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
#include "gui.h"

int main(){
	Config::load();

	Window& win = Window::getDefaultWindow("OpenGL Application", 852, 480);
	
	Renderer::init();
	TextRenderer::loadAscii(std::string(FontPath) + "Ascii.bmp");
	Renderer::enableDepthTest();
	Renderer::enableBlend();

	bool gui = false, gpressed = false;
	Camera camera;
	win.lockCursor();

	UpdateScheduler frameCounterScheduler(1);
	int frameCounter = 0;
	
	// Initialize GUI
	TextureImage image("./Data/Test.bmp");
	image = image.resample(2048, 2048);
	Texture* ptex = new Texture(image, true, 0);
	
	using GUI::Position;
	using GUI::Point2D;
	
	GUI::Area formArea(Position(Point2D(0.0f, 0.0f), Point2D(0, 0)), Position(Point2D(1.0f, 1.0f), Point2D(0, 0)));
	GUI::Area leftArea(Position(Point2D(0.0f, 0.0f), Point2D(0, 0)), Position(Point2D(0.7f, 1.0f), Point2D(0, 0)));
	GUI::Area rightArea(Position(Point2D(0.7f, 0.0f), Point2D(0, 0)), Position(Point2D(1.0f, 1.0f), Point2D(0, 0)));
	GUI::Area controlArea(Position(Point2D(0.0f, 0.0f), Point2D(0, 0)), Position(Point2D(1.0f, 1.0f), Point2D(0, -50)));
	GUI::TrackBar param1TrackBar(Position(Point2D(0.0f, 0.0f), Point2D(+10, +10)), Position(Point2D(1.0f, 0.0f), Point2D(-10, +40)), 0.0, 0.1, 0.0, "Ambient");
	GUI::TrackBar param2TrackBar(Position(Point2D(0.0f, 0.0f), Point2D(+10, +50)), Position(Point2D(1.0f, 0.0f), Point2D(-10, +80)), 0.0, 0.1, 0.0, "Diffuse");
	GUI::TrackBar param3TrackBar(Position(Point2D(0.0f, 0.0f), Point2D(+10, +90)), Position(Point2D(1.0f, 0.0f), Point2D(-10, +120)), 0.0, 10.0, 1.0, "Specular");
	GUI::Label countLabel(Position(Point2D(0.0f, 1.0f), Point2D(+10, -40)), Position(Point2D(0.5f, 1.0f), Point2D(-5, -10)), "Count");
	GUI::Button startButton(Position(Point2D(0.5f, 1.0f), Point2D(+5, -40)), Position(Point2D(1.0f, 1.0f), Point2D(-10, -10)), "OK");
	GUI::ScrollArea scrollArea(Position(Point2D(0.0f, 0.0f), Point2D(+10, +10)), Position(Point2D(1.0f, 1.0f), Point2D(0, -10)), Point2D(500, 800), Point2D(0.0f, 0.0f));
	GUI::PictureBox picPictureBoxArray1[5] = {
		GUI::PictureBox(Position(Point2D(0.0f, 0.0f), Point2D(+10, +10)), Position(Point2D(0.5f, 0.0f), Point2D(-10, +150)), ptex),
		GUI::PictureBox(Position(Point2D(0.5f, 0.0f), Point2D(+10, +10)), Position(Point2D(1.0f, 0.0f), Point2D(-10, +150)), ptex),
		GUI::PictureBox(Position(Point2D(0.0f, 0.0f), Point2D(+10, +170)), Position(Point2D(0.5f, 0.0f), Point2D(-10, +310)), ptex),
		GUI::PictureBox(Position(Point2D(0.5f, 0.0f), Point2D(+10, +170)), Position(Point2D(1.0f, 0.0f), Point2D(-10, +310)), ptex),
		GUI::PictureBox(Position(Point2D(0.0f, 0.0f), Point2D(+10, +330)), Position(Point2D(0.5f, 0.0f), Point2D(-10, +470)), ptex)
	};

	formArea.addChild({&leftArea, &rightArea});
	leftArea.addChild(&scrollArea);
	for (int i = 0; i < 5; i++) scrollArea.addChild(picPictureBoxArray1 + i);
	rightArea.addChild(&controlArea);
	controlArea.addChild({&param1TrackBar, &param2TrackBar, &param3TrackBar});
	rightArea.addChild({&countLabel, &startButton});
	GUI::Form form(&formArea);
	
	// Main Loop

	while (!win.shouldQuit()) {
		Renderer::waitForComplete();
		Renderer::checkError();
		win.swapBuffers();
		
		Renderer::setRenderArea(0, 0, win.getWidth(), win.getHeight());
		Renderer::beginFinalPass();
		
		if (!gui) {
			Renderer::setClearColor(Vec3f(0.0f, 0.0f, 0.0f));
			Renderer::clear();

			Renderer::enableTexture2D();
			Renderer::enableAlphaTest();
			Renderer::disableCullFace();
			Renderer::disableStencilTest();

			Renderer::setProjection(camera.getProjectionMatrix());
			Renderer::setModelview(camera.getModelViewMatrix());
			
			TextRenderer::drawAscii(Vec3f(0, 0, -60), "Hello, world!");
			TextRenderer::drawAscii(Vec3f(0, 0, -50), "Hello, world!");
			TextRenderer::drawAscii(Vec3f(0, 0, -40), "Hello, world!");
			TextRenderer::drawAscii(Vec3f(0, 0, -30), "Hello, world!");
			TextRenderer::drawAscii(Vec3f(0, 0, -20), "Hello, world!");
			TextRenderer::drawAscii(Vec3f(0, 0, -10), "Hello, world!");

			// Do rendering
		} else {
			Renderer::setClearColor(Vec3f(0.1f, 0.1f, 0.1f));
			Renderer::clear();
			
			Renderer::enableCullFace();
			Renderer::enableStencilTest();
			Renderer::disableTexture2D();
			Renderer::disableAlphaTest();
			
			Renderer::setProjection(Mat4f::ortho(0, win.getWidth(), 0, win.getHeight(), -1, 1));
			Renderer::setModelview(Mat4f(1.0f));
			
			form.render(win, Point2D(0, 0), Point2D(win.getWidth(), win.getHeight()));

			// Draw GUI
		}

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
		
		if (!gui) {
			camera.setPerspective(70.0f, float(win.getWidth()) / float(win.getHeight()), 0.1f, 256.0f);
			camera.update(win);
		} else {
			form.update(win, Point2D(0, 0), Point2D(win.getWidth(), win.getHeight()));
			
			if (startButton.clicked()) gui = !gui;
		}
		
		if (Window::isKeyPressed(SDL_SCANCODE_G)) {
			if (!gpressed) gui = !gui;
			gpressed = true;
		} else gpressed = false;
		
		if (Window::isKeyPressed(SDL_SCANCODE_ESCAPE)) break;
		
		if (gui) win.unlockCursor();
		else win.lockCursor();
	}

	win.unlockCursor();

	Config::save();
	return 0;
}

