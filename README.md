# NS-ONE – one-knob noise suppressor (VST3)

Single-knob spectral noise suppressor in the spirit of Waves NS1. Effect plugin (not an instrument).
Stereo/mono, fixed latency of 1024 samples (reported to the host, so REAPER compensates automatically).

## Easiest: let GitHub build it (no compiler needed)
1. Create a new GitHub repo and upload this whole folder (keep `.github/`).
2. Open the **Actions** tab -> **Build NS-ONE** -> wait for it to finish (~10 min).
3. Download `NS-ONE-windows-latest` or `NS-ONE-macos-latest` from the run's Artifacts.

## Or build locally
Requires CMake 3.22+ and a C++17 compiler (Visual Studio 2022 Build Tools on Windows, Xcode on macOS).
```
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
```
Result: `build/NSOne_artefacts/Release/VST3/NS-ONE.vst3`
(Add `-DNSONE_COPY_AFTER_BUILD=ON` to auto-install; may need admin rights on Windows.)

## Install
- Windows: copy `NS-ONE.vst3` to `C:\Program Files\Common Files\VST3\`
- macOS: copy to `~/Library/Audio/Plug-Ins/VST3/`; if blocked by Gatekeeper run
  `xattr -dr com.apple.quarantine ~/Library/Audio/Plug-Ins/VST3/NS-ONE.vst3`

## Use in REAPER
Options > Preferences > Plug-ins > VST > **Re-scan** (or Clear cache and re-scan), then on a track
click **FX** > Add > search **NS-ONE**. Put it first in the chain on a noisy vocal/instrument.

Knob: drag, double-click = reset to 60 %. At 0 % the plugin is bit-transparent.
