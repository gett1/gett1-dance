#!/bin/bash
set -euo pipefail

if [[ "$(uname -s)" != Darwin ]]; then
    echo "Run this script on a Mac with Xcode and CMake." >&2
    exit 1
fi

project_dir="$(cd "$(dirname "$0")" && pwd -P)"
juce_dir="${1:-${GETT1_JUCE_PATH:-}}"
if [[ ! -f "$juce_dir/CMakeLists.txt" ]]; then
    echo "Usage: bash Build_Installer_macOS.sh /path/to/JUCE" >&2
    exit 1
fi
juce_dir="$(cd "$juce_dir" && pwd -P)"
build_dir="$project_dir/build-macos-arm64"
dist_dir="$project_dir/dist"
artifacts="$build_dir/Gett1Dance_artefacts/Release"

cmake -S "$project_dir" -B "$build_dir" -G Xcode \
    "-DGETT1_JUCE_PATH=$juce_dir" \
    "-DCMAKE_OSX_ARCHITECTURES=arm64" \
    -DCMAKE_OSX_DEPLOYMENT_TARGET=11.0
cmake --build "$build_dir" --config Release \
    --target Gett1Dance_VST3 Gett1Dance_AU Gett1MotionTests Gett1OverlayTests
if [[ "$(uname -m)" == "arm64" ]]; then ctest --test-dir "$build_dir" -C Release --output-on-failure; fi

stage="$(mktemp -d "${TMPDIR:-/tmp}/gett1-dance.XXXXXXXX")"
trap 'rm -rf "$stage"' EXIT
root="$stage/root"
mkdir -p "$root/Library/Audio/Plug-Ins/VST3" "$root/Library/Audio/Plug-Ins/Components" "$dist_dir"

stage_plugin() {
    local bundle="$1" destination="$2" executable staged
    executable="$(/usr/libexec/PlistBuddy -c 'Print :CFBundleExecutable' "$bundle/Contents/Info.plist")"
    lipo "$bundle/Contents/MacOS/$executable" -verify_arch arm64
    staged="$destination/$(basename "$bundle")"
    ditto "$bundle" "$staged"
    codesign --force --sign - "$staged"
    codesign --verify --strict "$staged"
}

vst3="$artifacts/VST3/gett1 Dance.vst3"
au="$artifacts/AU/gett1 Dance.component"
stage_plugin "$vst3" "$root/Library/Audio/Plug-Ins/VST3"
stage_plugin "$au" "$root/Library/Audio/Plug-Ins/Components"
version="$(/usr/libexec/PlistBuddy -c 'Print :CFBundleShortVersionString' "$vst3/Contents/Info.plist")"

# Upgrade existing bundles at their standard paths without relocating or merging them.
python3 - "$stage/components.plist" <<'PYTHON'
import plistlib
import sys

paths = (
    "Library/Audio/Plug-Ins/VST3/gett1 Dance.vst3",
    "Library/Audio/Plug-Ins/Components/gett1 Dance.component",
)
components = [
    {
        "RootRelativeBundlePath": path,
        "BundleIsRelocatable": False,
        "BundleIsVersionChecked": True,
        "BundleOverwriteAction": "upgrade",
    }
    for path in paths
]
with open(sys.argv[1], "wb") as output:
    plistlib.dump(components, output)
PYTHON

pkg="$stage/Gett1Dance-${version}-macOS-AppleSilicon.pkg"
pkgbuild --root "$root" --component-plist "$stage/components.plist" \
    --identifier audio.gett1.dance.installer --version "$version" --install-location / "$pkg"
pkgutil --payload-files "$pkg" > "$stage/payload.txt"
grep -Fq 'Library/Audio/Plug-Ins/VST3/gett1 Dance.vst3/Contents/MacOS/' "$stage/payload.txt"
grep -Fq 'Library/Audio/Plug-Ins/Components/gett1 Dance.component/Contents/MacOS/' "$stage/payload.txt"
mv -f "$pkg" "$dist_dir/"
printf 'PRONTO: %s/%s\n' "$dist_dir" "$(basename "$pkg")"
