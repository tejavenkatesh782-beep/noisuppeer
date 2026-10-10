#pragma once
#include <algorithm>
#include <cmath>
#include <cstring>

// NS-ONE core DSP: single-knob STFT spectral noise suppressor (no JUCE dependency).
// 1024-point sqrt-Hann STFT, 50% overlap, per-bin noise floor tracking,
// Wiener-style gain with time + frequency smoothing.
// Fixed latency = N samples (reported to the host for delay compensation).
namespace nsone
{
class Suppressor
{
public:
    static constexpr int N = 1024, H = N / 2, K = N / 2;
    static constexpr int latency = N;

    Suppressor()
    {
        const double twoPi = 6.283185307179586;
        for (int i = 0; i < N; ++i) win[i] = (float) std::sqrt (0.5 - 0.5 * std::cos (twoPi * i / N));
        for (int i = 0; i < K; ++i)
        {
            cosT[i] = (float) std::cos (twoPi * i / N);
            sinT[i] = (float) std::sin (twoPi * i / N);
        }
        prepare (48000.0);
    }

    void prepare (double sampleRate)
    {
        const double scale = 86.1328125 / (sampleRate / H); // tuned at 44.1 kHz
        riseFactor = (float) std::pow (1.01, scale);
        fallKeep   = (float) std::pow (0.8, scale);
        smoothKeep = (float) std::pow (0.85, scale);
        attack     = (float) (1.0 - std::pow (0.3, scale));
        release    = (float) (1.0 - std::pow (0.7, scale));
        reset();
    }

    void reset()
    {
        std::memset (inRing, 0, sizeof inRing);
        std::memset (outRing, 0, sizeof outRing);
        std::memset (noise, 0, sizeof noise);
        std::memset (ps, 0, sizeof ps);
        for (auto& g : gp) g = 1.0f;
        wr = rd = count = 0;
        init = false;
        redDb = 0.0f;
        amt = amountTarget;
        mix = mixTarget;
    }

    // 0..1
    void setAmount (float a) noexcept
    {
        amountTarget = std::min (1.0f, std::max (0.0f, a));
        mixTarget = std::min (1.0f, amountTarget * 25.0f); // knob at 0 = fully transparent
    }

    float getReductionDb() const noexcept { return redDb; }

    float process (float x) noexcept
    {
        const float dry = inRing[wr];   // input delayed by exactly N samples
        const float wet = outRing[rd];
        outRing[rd] = 0.0f;
        rd = (rd + 1) & (N - 1);
        inRing[wr] = x;
        wr = (wr + 1) & (N - 1);
        if (++count == H) { count = 0; frame(); }
        mix += (mixTarget - mix) * 0.002f;
        return dry + (wet - dry) * mix;
    }

private:
    void frame() noexcept
    {
        for (int i = 0; i < N; ++i) { re[i] = inRing[(wr + i) & (N - 1)] * win[i]; im[i] = 0.0f; }
        fft (false);

        amt += (amountTarget - amt) * 0.3f;
        const float alpha = 1.0f + amt * 3.0f;
        const float gmin  = std::pow (10.0f, -(3.0f + amt * 37.0f) / 20.0f);

        for (int k = 0; k <= K; ++k)
        {
            const float p = re[k] * re[k] + im[k] * im[k] + 1e-12f;
            // smoothed power -> low-variance input for the noise-floor tracker
            ps[k] = init ? ps[k] * smoothKeep + p * (1.0f - smoothKeep) : p;
            if (! init)             noise[k] = ps[k];
            else if (ps[k] < noise[k]) noise[k] = noise[k] * fallKeep + ps[k] * (1.0f - fallKeep);
            else                    noise[k] = std::min (ps[k], noise[k] * riseFactor);

            const float q = std::max (ps[k], 0.35f * p); // stable decision power, still opens fast on onsets
            float t = std::max (gmin, 1.0f - alpha * noise[k] / q);
            t = gp[k] + (t - gp[k]) * (t > gp[k] ? attack : release);
            gp[k] = t;
        }
        init = true;

        float s = 0.0f;
        for (int k = 0; k <= K; ++k)
        {
            const float a = gp[k > 0 ? k - 1 : 0], c = gp[k < K ? k + 1 : K];
            const float g = (a + 2.0f * gp[k] + c) * 0.25f;
            s += std::log10 (g);
            re[k] *= g; im[k] *= g;
            if (k > 0 && k < K) { re[N - k] *= g; im[N - k] *= g; }
        }
        redDb = redDb * 0.9f + (20.0f * s / (K + 1)) * 0.1f;

        fft (true);
        for (int i = 0; i < N; ++i)
            outRing[(rd + i) & (N - 1)] += re[i] * (1.0f / N) * win[i];
    }

    void fft (bool inverse) noexcept
    {
        for (int i = 1, j = 0; i < N; ++i)
        {
            int b = N >> 1;
            for (; j & b; b >>= 1) j ^= b;
            j ^= b;
            if (i < j) { std::swap (re[i], re[j]); std::swap (im[i], im[j]); }
        }
        const float sg = inverse ? 1.0f : -1.0f;
        for (int len = 2; len <= N; len <<= 1)
        {
            const int h = len >> 1, st = N / len;
            for (int i = 0; i < N; i += len)
                for (int k = 0; k < h; ++k)
                {
                    const float wr_ = cosT[k * st], wi_ = sg * sinT[k * st];
                    const int a = i + k, b = a + h;
                    const float xr = re[b] * wr_ - im[b] * wi_, xi = re[b] * wi_ + im[b] * wr_;
                    re[b] = re[a] - xr; im[b] = im[a] - xi;
                    re[a] += xr;        im[a] += xi;
                }
        }
    }

    float win[N], cosT[K], sinT[K];
    float inRing[N] {}, outRing[N] {}, re[N] {}, im[N] {}, noise[K + 1] {}, ps[K + 1] {}, gp[K + 1] {};
    int wr = 0, rd = 0, count = 0;
    bool init = false;
    float riseFactor = 1.002f, fallKeep = 0.8f, smoothKeep = 0.85f, attack = 0.7f, release = 0.3f;
    float amountTarget = 0.6f, amt = 0.6f, mixTarget = 1.0f, mix = 1.0f, redDb = 0.0f;
};
} // namespace nsone
