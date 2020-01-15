#ifndef WINDOW_H_
#define WINDOW_H_

#include <string>
#include <set>
#include <map>
#include <SDL2/SDL.h>
#include "debug.h"
#include "opengl.h"

struct MouseState {
	int x = 0, y = 0, xd = 0, yd = 0;
	bool left = false, mid = false, right = false;
};

class Window {
public:
	Window(const std::string& title, int width, int height, bool resizable = true);
	Window(Window&&) = default;
	Window& operator=(const Window&) = delete;
	~Window();

	void makeCurrent() const { SDL_GL_MakeCurrent(mWindow, mContext); }
	void swapBuffers() const { SDL_GL_SwapWindow(mWindow); }

	static const Uint8* getKeyBoardState() { return SDL_GetKeyboardState(nullptr); }
	static bool isKeyPressed(SDL_Scancode c) { return getKeyBoardState()[c] != 0; }
	bool isKeyActed(SDL_Scancode c) const { return mKeyActed.find(c) != mKeyActed.end(); }

	int getWidth() const { return mWidth; }
	int getHeight() const { return mHeight; }

	MouseState getMouseState() const { return mMouse; }
	MouseState getPrevMouseState() const { return mPrevMouse; }

	static void lockCursor() { SDL_SetRelativeMouseMode(SDL_TRUE); }
	static void unlockCursor() { SDL_SetRelativeMouseMode(SDL_FALSE); }
	void setTitle(const std::string& title) { SDL_SetWindowTitle(mWindow, title.c_str()); }
	void setFullscreen(bool f) {
		if (f) SDL_SetWindowFullscreen(mWindow, SDL_WINDOW_FULLSCREEN_DESKTOP);
		else SDL_SetWindowFullscreen(mWindow, 0);
	}
	
	bool shouldQuit() const { return mShouldQuit; }
	SDL_Window* handle() const { return mWindow; }

	static void init();
	static void cleanup();
	static float getSystemScalingFactor(int displayIndex = 0);
	static void pollEvents(bool waitForEvent = false);
	
	static Window& getDefaultWindow(const std::string& title = "", int width = 0, int height = 0) {
		static Window win(title, width, height);
		return win;
	}

private:
	SDL_Window* mWindow = nullptr;
	std::string mTitle;
	int mWidth, mHeight;
	bool mShouldQuit = false;
	MouseState mMouse, mPrevMouse;
	std::set<SDL_Scancode> mKeyActed;
	
	static bool mContextCreated;
	static SDL_GLContext mContext; // Use one context across all windows
	static std::map<SDL_Window*, Window*> mWindows; // All windows
	static bool mCoreProfile, mGLES, mDebugContext;
	static int mSwapInterval;
};

#endif

