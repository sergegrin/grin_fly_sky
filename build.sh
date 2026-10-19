#!/bin/sh
set -e
cd "$(dirname "$0")"

V=13.2.1-1.1
case "$(uname -s)-$(uname -m)" in
  Linux-x86_64)               P=linux-x64;    SHA=1252a8cafe9237de27a765376697230368eec21db44dc3f1edeb8d838dabd530 ;;
  Linux-aarch64)              P=linux-arm64;  SHA=ab7f75d95ead0b1efb7432e7f034f9575cc3d23dc1b03d41af1ec253486d19de ;;
  Darwin-x86_64)              P=darwin-x64;   SHA=1ecc0fd6c31020aff702204f51459b4b00ff0d12b9cd95e832399881d819aa57 ;;
  Darwin-arm64)               P=darwin-arm64; SHA=d4ce0de062420daab140161086ba017642365977e148d20f55a8807b1eacd703 ;;
  *) echo "Unsupported system: $(uname -s) $(uname -m)"; exit 1 ;;
esac

if ! command -v make >/dev/null 2>&1; then
  if [ "$(uname -s)" = Darwin ]; then
    echo "make not found. Run: xcode-select --install   then run this script again."
  else
    echo "make not found. Install it, e.g.: sudo apt install make"
  fi
  exit 1
fi

DIR=tools/xpack-arm-none-eabi-gcc-$V
if [ ! -d "$DIR/bin" ]; then
  mkdir -p tools
  F=xpack-arm-none-eabi-gcc-$V-$P.tar.gz
  echo "Downloading toolchain ..."
  curl -fL -o "tools/$F" "https://github.com/xpack-dev-tools/arm-none-eabi-gcc-xpack/releases/download/v$V/$F"
  if command -v sha256sum >/dev/null 2>&1; then H=$(sha256sum "tools/$F"); else H=$(shasum -a 256 "tools/$F"); fi
  [ "${H%% *}" = "$SHA" ] || { rm -f "tools/$F"; echo "Checksum mismatch, run again."; exit 1; }
  echo "Unpacking ..."
  tar -xzf "tools/$F" -C tools
  rm -f "tools/$F"
fi

export PATH="$PWD/$DIR/bin:$PATH"

rm -rf build
N=$(getconf _NPROCESSORS_ONLN 2>/dev/null || echo 2)
make -j"$N"
echo
echo "DONE. Firmware: $PWD/build/FSI6.bin (and FSI6.hex)"