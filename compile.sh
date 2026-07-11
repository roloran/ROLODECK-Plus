#!/bin/bash

set -euo pipefail

environment="${PIO_ENV:-tdeck-plus-core3-lvgl9}"

# Arduino Core 2 and Core 3 unfortunately publish the framework under the same
# PlatformIO package name. PlatformIO normally replaces it when environments
# are switched, but Core 2 -> Core 3 can otherwise require two invocations: the
# first removes the incompatible package and the second installs the requested
# one. Remove only a known mismatched framework cache entry up front so a
# single compile command is deterministic.
case "$environment" in
  tdeck-plus-core2)
    expected_framework_version='"version": "3.20017.'
    ;;
  tdeck-plus-core3|tdeck-plus-core3-lvgl9)
    expected_framework_version='"version": "3.3.9"'
    ;;
  *)
    expected_framework_version=''
    ;;
esac

platformio_core_dir="${PLATFORMIO_CORE_DIR:-$HOME/.platformio}"
framework_manifest="$platformio_core_dir/packages/framework-arduinoespressif32/package.json"
if [[ -n "$expected_framework_version" && -f "$framework_manifest" ]] &&
   ! grep -q "$expected_framework_version" "$framework_manifest"; then
  echo "Replacing cached Arduino framework for $environment"
  pio pkg uninstall --global --tool framework-arduinoespressif32 --no-save --silent
fi

exec pio run -e "$environment" "$@"
