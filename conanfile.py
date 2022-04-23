from conans import ConanFile, CMake

class CoD4DM1(ConanFile):
	name = "CoD4DM1"
	version = "1.0.1"
	license = "LICENCE"
	url = "https://github.com/Iswenzz/CoD4-DM1"
	description = "Reverse of CoD4 & CoD4X (.DM_1) demo files with features such as parsing snapshot informations, frames, entities, clients and server messages."

	generators = "cmake"
	exports_sources = "LICENSE", "README.md", "CMakeLists.txt", "src/*", "fixtures/*"

	requires = "cxxopts/3.0.0"
	settings = "os", "arch", "compiler", "build_type"
	options = {"enable_testing": [True, False]}
	default_options  = {"enable_testing": False}

	def build_requirements(self):
		if self.options.enable_testing:
			self.build_requires("gtest/cci.20210126", force_host_context=True)

	def build(self):
		cmake_release = CMake(self, build_type="Release")
		cmake_release.configure()
		cmake_release.build()

	def package(self):
		self.copy("LICENSE, README.md")
		self.copy("*.hpp", src="src", dst="include")
		self.copy("*.a", dst="lib", keep_path=False)
		self.copy("*.lib", dst="lib", keep_path=False)
		self.copy("*.dll", dst="bin", keep_path=False)
		self.copy("*.so", dst="bin", keep_path=False)

	def package_info(self):
		self.cpp_info.libs = ["CoD4DM1"]

