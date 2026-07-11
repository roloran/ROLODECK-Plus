"""Build compatibility adjustments for the pinned pioarduino core-3 platform."""

from os.path import join

from platformio.project.config import ProjectConfig

Import("env")

# Arduino-ESP32 3.3.9's LittleFS library metadata does not declare its FS
# dependency. PlatformIO therefore compiles FS but does not expose FS.h while
# compiling LittleFS. Derive the framework location rather than embedding a
# user- or machine-specific package path.
framework_dir = env.PioPlatform().get_package_dir("framework-arduinoespressif32")
if framework_dir is None:
    # Extra scripts run before PlatformIO installs missing framework packages.
    # Derive the future package location so a clean machine or a switch from
    # the core-2 environment does not depend on an already-populated cache.
    packages_dir = ProjectConfig.get_instance().get("platformio", "packages_dir")
    framework_dir = join(packages_dir, "framework-arduinoespressif32")
env.Append(CPPPATH=[join(framework_dir, "libraries", "FS", "src")])

# GCC 14 warns when LVGL 8 combines its part and state enum values as style
# selectors, although this is the documented LVGL API. ESP8266Audio also
# builds an unused PDM backend with one benign IDF 5 narrowing conversion.
# Keep these C++-specific options away from the C compiler.
env.Append(
    CXXFLAGS=[
        "-Wno-deprecated-enum-enum-conversion",
        "-Wno-narrowing",
    ]
)
