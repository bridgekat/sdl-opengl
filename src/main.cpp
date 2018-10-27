#include <stdlib.h>
#include "config.h"
#include "window.h"
#include "renderer.h"
#include "vertexarray.h"
#include "camera.h"
#include "framebuffer.h"
#include "updatescheduler.h"
#include "bitmap.h"
#include "textrenderer.h"

int main(){
	Config::load();

	Window& win = Window::getDefaultWindow("OpenGL Application", 852, 480);

	Renderer::init();
	TextRenderer::loadAscii(std::string(FontPath) + "Ascii.bmp");

	Camera camera;
	win.lockCursor();

	UpdateScheduler frameCounterScheduler(1);
	int frameCounter = 0;

	while (!win.shouldQuit()) {
		Renderer::waitForComplete();
		Renderer::checkError();
		win.swapBuffers();
		
		Renderer::setRenderArea(0, 0, win.getWidth(), win.getHeight());
		Renderer::clear();

		Renderer::beginFinalPass();
		Renderer::setProjection(camera.getProjectionMatrix());
		Renderer::setModelview(camera.getModelViewMatrix());
		
		Renderer::enableTexture2D();
		Renderer::enableBlend();
		Renderer::disableCullFace();
		TextRenderer::drawAscii(Vec3f(0, 0, -60), "Hello, world!");
		TextRenderer::drawAscii(Vec3f(0, 0, -50), "Hello, world!");
		TextRenderer::drawAscii(Vec3f(0, 0, -40), "Hello, world!");
		TextRenderer::drawAscii(Vec3f(0, 0, -30), "Hello, world!");
		TextRenderer::drawAscii(Vec3f(0, 0, -20), "Hello, world!");
		TextRenderer::drawAscii(Vec3f(0, 0, -10), "Hello, world!");
		
		// Do rendering

		Renderer::endFinalPass();

		frameCounter++;
		frameCounterScheduler.refresh();
		while (!frameCounterScheduler.inSync()) {
			std::stringstream ss;
			ss << "OpenGL Application (" << frameCounter << " FPS)";
			frameCounter = 0;
			Window::getDefaultWindow().setTitle(ss.str());
			frameCounterScheduler.increase();
		}

		win.pollEvents();

		camera.setPerspective(70.0f, float(win.getWidth()) / float(win.getHeight()), 0.1f, 256.0f);
		camera.update(win);
		
		if (Window::isKeyPressed(SDL_SCANCODE_ESCAPE)) break;
	}

	win.unlockCursor();

	Config::save();
	return 0;
}

