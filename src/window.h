#ifndef WINDOW_H_
#define WINDOW_H_

#include <string>
#include <set>
#include <SDL2/SDL.h>
#include "debug.h"

struct MouseState {
	int x, y, wx, wy;
	bool left, mid, right, locked;
};

class Window {
public:
	Window(const Window&) = delete;
	Window& operator=(const Window&) = delete;

	void makeCurrentDraw() const { SDL_GL_MakeCurrent(mWindow, mContext); }
	void swapBuffers() const { SDL_GL_SwapWindow(mWindow); }

	static const Uint8* getKeyBoardState() { return SDL_GetKeyboardState(nullptr); }
	static bool isKeyPressed(SDL_Scancode c) { return getKeyBoardState()[c] != 0; }
	static bool isKeyActed(SDL_Scancode c) { return mKeyActed.find(c) != mKeyActed.end(); }

	int getWidth() const { return mWidth; }
	int getHeight() const { return mHeight; }

	MouseState getMouseState() const {
		if (mMouse.locked) Assert(false); // Cursor locked, use getMouseMotion() instead!
		return mMouse;
	}

	MouseState getPrevMouseState() const {
		if (mMouse.locked) Assert(false); // Cursor locked, use getMouseMotion() instead!
		return mPrevMouse;
	}

	MouseState getMouseMotion() const {
		if (mMouse.locked) return mMouse;
		MouseState res = mMouse;
		res.x -= mPrevMouse.x;
		res.y -= mPrevMouse.y;
		return res;
	}

	void lockCursor() const { SDL_SetRelativeMouseMode(SDL_TRUE); }
	void unlockCursor() const { SDL_SetRelativeMouseMode(SDL_FALSE); }
	void setTitle(const std::string& title) { SDL_SetWindowTitle(mWindow, title.c_str()); }
	void setFullscreen(bool f) {
		if (f) SDL_SetWindowFullscreen(mWindow, SDL_WINDOW_FULLSCREEN_DESKTOP);
		else SDL_SetWindowFullscreen(mWindow, 0);
	}

	void pollEvents(bool waitForEvent = false);

	static Window& getDefaultWindow(const std::string& title = "", int width = 0, int height = 0) {
		static Window win(title, width, height);
		return win;
	}

	bool shouldQuit() const { return mShouldQuit; }

private:
	SDL_Window* mWindow = nullptr;
	std::string mTitle;
	int mWidth, mHeight;
	MouseState mMouse, mPrevMouse;
	bool mShouldQuit = false;
	static std::set<SDL_Scancode> mKeyActed;

	Window(const std::string& title, int width, int height);
	~Window();

	SDL_GLContext mContext;
};

#endif

