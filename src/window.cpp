#include "window.h"
#include <sstream>
#include "opengl.h"
#include <SDL2/SDL_image.h>
#include "logger.h"
#include "debug.h"
#include "common.h"
#include "config.h"
#include "renderer.h"

#ifdef PROJECTNAME_TARGET_WINDOWS
#	define GLCALLBACK __stdcall
#else
#   define GLCALLBACK
#endif

std::map<SDL_Window*, Window*> Window::mWindows;
SDL_GLContext Window::mContext;
bool Window::mContextCreated = false;
bool Window::mCoreProfile, Window::mGLES, Window::mDebugContext;
int Window::mSwapInterval;

// OpenGL debug callback
void GLCALLBACK glDebugCallback(GLenum /*source*/, GLenum /*type*/, GLuint /*id*/, GLenum severity, GLsizei /*length*/, const GLchar* msg, const void* /*data*/) {
	if (severity != GL_DEBUG_SEVERITY_NOTIFICATION) {
		std::stringstream ss("OpenGL debug: ");
		ss << std::string(msg);
		LogVerbose(ss.str());
	}
}

Window::Window(const std::string& title, int width, int height, bool resizable):
	mTitle(title), mWidth(width), mHeight(height) {

	mWindow = SDL_CreateWindow(mTitle.c_str(), SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, mWidth, mHeight,
		SDL_WINDOW_OPENGL | SDL_WINDOW_ALLOW_HIGHDPI | (resizable ? SDL_WINDOW_RESIZABLE : 0));

	if (mWindow == nullptr) {
		LogFatal("Failed to create SDL window!");
		Assert(false);
	}
	
	mWindows[mWindow] = this;
	
	if (!mContextCreated) {
		// Create & init OpenGL context
		mContextCreated = true;
		
		mContext = SDL_GL_CreateContext(mWindow);
		makeCurrent();
		
		SDL_GL_SetSwapInterval(mSwapInterval);
		OpenGL::init(mCoreProfile);
		
		if (mDebugContext) {
			if (GLEW_ARB_debug_output) {
				glDebugMessageCallbackARB(&glDebugCallback, nullptr);
				LogInfo("GL_ARB_debug_output enabled.");
			} else LogWarning("GL_ARB_debug_output not supported, disabling OpenGL debugging.");
		}
	}
//	std::stringstream ss; ss << "Window " << SDL_GetWindowID(mWindow) << " created"; LogVerbose(ss.str());
}

Window::~Window() {
//	std::stringstream ss; ss << "Window " << SDL_GetWindowID(mWindow) << " closed"; LogVerbose(ss.str());	
	mWindows.erase(mWindow);
	SDL_DestroyWindow(mWindow);
}

void Window::init() {
	SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER);
	IMG_Init(IMG_INIT_PNG);
	
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
	SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, Config::getInt("OpenGL.Multisamples", 0));
	
	mCoreProfile = Config::getInt("OpenGL.CoreProfile", 0) != 0;
	mGLES = Config::getInt("OpenGL.ES", 0) != 0;
	mDebugContext = Config::getInt("OpenGL.Debugging", 0) != 0;
	mSwapInterval = Config::getInt("OpenGL.SwapInterval", 1);
	if (mGLES) mCoreProfile = true;

	if (mDebugContext) SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG);
	if (mGLES) {
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
		LogInfo("OpenGL profile: ES");
	} else if (mCoreProfile) {
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
		LogInfo("OpenGL profile: Core");
	} else {
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_COMPATIBILITY);
		LogInfo("OpenGL profile: Compatibility");
	}
}

void Window::cleanup() {
	if (mContextCreated) {
		SDL_GL_DeleteContext(mContext);
		mContextCreated = false;
	}
	IMG_Quit();
	SDL_Quit();
}

void Window::pollEvents(bool waitForEvent) {
	if (waitForEvent) SDL_WaitEvent(0);
	
	for (auto curr: mWindows) if (curr.second) {
		curr.second->mPrevMouse = curr.second->mMouse;
		curr.second->mMouse.xd = curr.second->mMouse.yd = 0;
		curr.second->mKeyActed.clear();
	}
	
	Window* w = nullptr;
	SDL_Event e;
	while (SDL_PollEvent(&e)) {
		switch (e.type) {
		case SDL_WINDOWEVENT:
			w = mWindows[SDL_GetWindowFromID(e.window.windowID)];
			if (w == nullptr) { LogVerbose("Received an SDL event with unknown window pointer"); break; }
			switch (e.window.event) {
			case SDL_WINDOWEVENT_RESIZED:
			case SDL_WINDOWEVENT_SIZE_CHANGED:
				w->mWidth = e.window.data1;
				w->mHeight = e.window.data2;
				break;
			case SDL_WINDOWEVENT_FOCUS_LOST:
//				w->mMouse.x = w->mMouse.y = -1;
//				w->mMouse.left = w->mMouse.mid = w->mMouse.right = false;
				break;
			case SDL_WINDOWEVENT_CLOSE:
				w->mShouldQuit = true;
				break;
			}
			break;
		case SDL_MOUSEMOTION:
			w = mWindows[SDL_GetWindowFromID(e.motion.windowID)];
			if (w == nullptr) { LogVerbose("Received an SDL event with unknown window pointer"); break; }
			w->mMouse.x = e.motion.x, w->mMouse.y = e.motion.y;
			w->mMouse.xd += e.motion.xrel, w->mMouse.yd += e.motion.yrel;
			w->mMouse.left = e.motion.state & SDL_BUTTON_LMASK;
			w->mMouse.mid = e.motion.state & SDL_BUTTON_MMASK;
			w->mMouse.right = e.motion.state & SDL_BUTTON_RMASK;
			SDL_CaptureMouse(e.motion.state != 0 ? SDL_TRUE : SDL_FALSE);
//			printf("%d %d %d\n", w->mMouse.x, w->mMouse.y, e.motion.windowID);
			break;
/*
		case SDL_MOUSEBUTTONDOWN:
			if (e.button.which == SDL_TOUCH_MOUSEID) break;
			w = mWindows[SDL_GetWindowFromID(e.button.windowID)];
			if (w == nullptr) { LogVerbose("Received an SDL event with unknown window pointer"); break; }
			if (e.button.button == SDL_BUTTON_LEFT) w->mMouse.left = true;
			if (e.button.button == SDL_BUTTON_MIDDLE) w->mMouse.mid = true;
			if (e.button.button == SDL_BUTTON_RIGHT) w->mMouse.right = true;
			break;
		case SDL_MOUSEBUTTONUP:
			if (e.button.which == SDL_TOUCH_MOUSEID) break;
			w = mWindows[SDL_GetWindowFromID(e.button.windowID)];
			if (w == nullptr) { LogVerbose("Received an SDL event with unknown window pointer"); break; }
			if (e.button.button == SDL_BUTTON_LEFT) w->mMouse.left = false;
			if (e.button.button == SDL_BUTTON_MIDDLE) w->mMouse.mid = false;
			if (e.button.button == SDL_BUTTON_RIGHT) w->mMouse.right = false;
			break;
*/
		case SDL_KEYDOWN:
			w = mWindows[SDL_GetWindowFromID(e.key.windowID)];
			if (w == nullptr) { LogVerbose("Received an SDL event with unknown window pointer"); break; }
			w->mKeyActed.insert(e.key.keysym.scancode);
			break;
		}
	}
	
	w = mWindows[SDL_GetMouseFocus()];
	if (w != nullptr) {
		int x, y;
		Uint32 buttons = SDL_GetMouseState(&x, &y);
		w->mMouse.x = x, w->mMouse.y = y;
		w->mMouse.left = buttons & SDL_BUTTON(SDL_BUTTON_LEFT);
		w->mMouse.mid = buttons & SDL_BUTTON(SDL_BUTTON_MIDDLE);
		w->mMouse.right = buttons & SDL_BUTTON(SDL_BUTTON_RIGHT);
	}
}

float Window::getSystemScalingFactor(int displayIndex) {
	const float defaultdpi =
#ifdef PROJECTNAME_TARGET_WINDOWS
		96.0f;
#elif defined(PROJECTNAME_TARGET_MACOSX)
		72.0f;
#else
		0.0f;
	LogWarning("No system default DPI set for this platform, assuming scaling factor = 1");
	return 1.0f;
#endif
	float dpi = 0.0f;
	if (SDL_GetDisplayDPI(displayIndex, nullptr, &dpi, nullptr) != 0) { // Failed to get DPI
		LogWarning("Failed to get current DPI, assuming scaling factor = 1");
		return 1.0f;
	}
	return dpi / defaultdpi;
}
