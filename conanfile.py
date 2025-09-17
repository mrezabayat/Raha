from conan import ConanFile
from conan.tools.cmake import CMake, CMakeDeps, CMakeToolchain
from conan.tools.files import copy
import os

class VideoPlayerRecipe(ConanFile):
    name = "video-player"
    version = "0.1.0"
    package_type = "application"
    settings = "os", "compiler", "build_type", "arch"
    exports_sources = "CMakeLists.txt", "cmake/*", "include/*", "src/*", "resources/*"

    requires = (
        "ffmpeg/6.1",
        "sdl/2.28.5",
        "sqlite3/3.45.3",
        "spdlog/1.12.0",
        "nlohmann_json/3.11.2"
    )

    test_requires = ("gtest/1.14.0",)

    default_options = {
        "ffmpeg/*:with_openh264": False,
        "ffmpeg/*:with_libvpx": False,
        "ffmpeg/*:with_vulkan": True,
        "ffmpeg/*:with_sdl": False,
        "sdl/*:jpeg": True,
        "sdl/*:png": True,
        "sdl/*:opengl": True,
        "sqlite3/*:shared": False,
        "spdlog/*:header_only": False
    }

    def layout(self):
        self.folders.build = "."
        self.folders.generators = "generators"
        self.folders.source = "."

    def generate(self):
        tc = CMakeToolchain(self)
        tc.variables["VPLAYER_ENABLE_TESTS"] = True
        tc.generate()
        deps = CMakeDeps(self)
        deps.generate()

    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()

    def package(self):
        cmake = CMake(self)
        cmake.install()

    def package_info(self):
        self.cpp_info.libs = []
