#ifndef COMMON_H_
#define COMMON_H_

// Compiler
#ifdef _MSC_VER
#	define PROJECTNAME_COMPILER_MSVC
#endif

// Debugging
#ifdef _DEBUG
#	define PROJECTNAME_DEBUG
#endif

// Target
#ifdef _WIN32
#	define PROJECTNAME_TARGET_WINDOWS
#elif defined __MACOSX__ || (defined __APPLE__ && defined __GNUC__)
#	define PROJECTNAME_TARGET_MACOSX
#	define PROJECTNAME_TARGET_POSIX
#else
#	define PROJECTNAME_TARGET_LINUX
#	define PROJECTNAME_TARGET_POSIX
#endif

constexpr const char* RootPath = "./";
constexpr const char* ConfigPath = "./";
constexpr const char* ShaderPath = "./Shaders/";
constexpr const char* FontPath = "./Fonts/";
constexpr const char* ScreenshotPath = "./Screenshots/";
constexpr const char* ConfigFilename = "Config.ini";

#endif // !COMMON_H_

