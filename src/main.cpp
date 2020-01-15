#include <stdlib.h>
#include <memory>
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

// TODO: multiple contexts & multithreading (MakeCurrent is really slow!)
class Dialog {
public:
	Window window;
	GUI::Form form;
	GUI::Area dialogArea;
	GUI::Label textLabel;
	GUI::Button okButton, cancelButton;
	
	Dialog(): window("Dialog", 300 * GUI::getScalingFactor(), 120 * GUI::getScalingFactor(), false) {
		window.makeCurrent();
		
		// Create Dialog GUI
		using GUI::Position;
		using GUI::Point2D;
		dialogArea = GUI::Area(Position(0.0f, 0.0f, 0, 0), Position(1.0f, 1.0f, 0, 0));
		textLabel = GUI::Label(Position(0.0f, 0.0f, 0, 0), Position(1.0f, 1.0f, 0, -40), "This is a dialog window");
		okButton = GUI::Button(Position(0.0f, 1.0f, +10, -40), Position(0.5f, 1.0f, -5, -10), "OK");
		cancelButton = GUI::Button(Position(0.5f, 1.0f, +5, -40), Position(1.0f, 1.0f, -10, -10), "Cancel");
		dialogArea.addChild({&textLabel, &okButton, &cancelButton});
		form.area = &dialogArea;
		
		// Miscellaneous
		Renderer::enableDepthTest();
		Renderer::enableBlend();
	}
	
	Dialog(Dialog&& r) = default;
	Dialog& operator=(const Dialog&) = delete;
	
	void render() {
		window.makeCurrent();
		Renderer::waitForComplete();
		Renderer::checkError();
		window.swapBuffers();
		
		Renderer::setRenderArea(0, 0, window.getWidth(), window.getHeight());
		Renderer::beginFinalPass();
		
		Renderer::setClearColor(GUI::BackgroundColor);
		Renderer::clear();
		
		Renderer::disableCullFace();
		Renderer::disableStencilTest();
		Renderer::disableTexture2D();
		Renderer::disableAlphaTest();
		
		Renderer::setProjection(Mat4f::ortho(0, window.getWidth(), 0, window.getHeight(), -1, 1));
		Renderer::setModelview(Mat4f(1.0f));
		
		// Draw GUI
		form.render(window, GUI::Point2D(0, 0), GUI::Point2D(window.getWidth(), window.getHeight()));
		
		Renderer::endFinalPass();
	}
	
	void update() {
		form.update(window, GUI::Point2D(0, 0), GUI::Point2D(window.getWidth(), window.getHeight()));
	}
};

int main() {
	// Initialize
	Config::load();
	Window::init();
	
	// Get scaling factor
	float scaling = float(Config::getDouble("GUI.ScalingFactor", 0));
	if (scaling <= 0) scaling = Window::getSystemScalingFactor();
	std::stringstream ss; ss << "Scaling factor = " << scaling; LogInfo(ss.str());
	GUI::setScalingFactor(scaling);
	Window& win = Window::getDefaultWindow("OpenGL Application", 852 * scaling, 480 * scaling);
	
	// Init renderer & text renderer
	Renderer::init();
	TextRenderer::init();
	
	// Create GUI
	TextureImage image("./Data/Test.png");
	image = image.resample(2048, 2048);
	Texture* ptex = new Texture(image, true);
	
	using GUI::Position;
	using GUI::Point2D;
	GUI::Area formArea(Position(0.0f, 0.0f, 0, 0), Position(1.0f, 1.0f, 0, 0));
	GUI::Area leftArea(Position(0.0f, 0.0f, 0, 0), Position(0.7f, 1.0f, 0, 0));
	GUI::Area rightArea(Position(0.7f, 0.0f, 0, 0), Position(1.0f, 1.0f, 0, 0));
	GUI::Area controlArea(Position(0.0f, 0.0f, 0, 0), Position(1.0f, 1.0f, 0, -50));
	GUI::TrackBar test1TrackBar(Position(0.0f, 0.0f, +10, +10), Position(1.0f, 0.0f, -10, +40), 0.0, 0.1, 0.0, "This is a trackbar.");
	GUI::TrackBar test2TrackBar(Position(0.0f, 0.0f, +10, +50), Position(1.0f, 0.0f, -10, +80), 0.0, 0.1, 0.0, "Another one...");
	GUI::TrackBar test3TrackBar(Position(0.0f, 0.0f, +10, +90), Position(1.0f, 0.0f, -10, +120), 0.0, 10.0, 1.0, "The quick brown fox jumps over a lazy dog");
	GUI::Label testLabel(Position(0.0f, 1.0f, +10, -40), Position(0.5f, 1.0f, -5, -10), "Label 1", true);
	GUI::Button testButton(Position(0.5f, 1.0f, +5, -40), Position(1.0f, 1.0f, -10, -10), "Button");
	GUI::ScrollArea scrollArea(Position(0.0f, 0.0f, +10, +10), Position(1.0f, 1.0f, 0, -10), Point2D(500, 800), Point2D(0.0f, 0.0f));
	GUI::PictureBox testPictureBoxArray[5] = {
		GUI::PictureBox(Position(0.0f, 0.0f, +10, +10), Position(0.5f, 0.0f, -10, +150), ptex),
		GUI::PictureBox(Position(0.5f, 0.0f, +10, +10), Position(1.0f, 0.0f, -10, +150), ptex),
		GUI::PictureBox(Position(0.0f, 0.0f, +10, +170), Position(0.5f, 0.0f, -10, +310), ptex),
		GUI::PictureBox(Position(0.5f, 0.0f, +10, +170), Position(1.0f, 0.0f, -10, +310), ptex),
		GUI::PictureBox(Position(0.0f, 0.0f, +10, +330), Position(0.5f, 0.0f, -10, +470), ptex)
	};

	formArea.addChild({&leftArea, &rightArea});
	leftArea.addChild(&scrollArea);
	for (int i = 0; i < 5; i++) scrollArea.addChild(testPictureBoxArray + i);
	rightArea.addChild(&controlArea);
	controlArea.addChild({&test1TrackBar, &test2TrackBar, &test3TrackBar});
	rightArea.addChild({&testLabel, &testButton});
	GUI::Form form;
	form.area = &formArea;
	
	// Miscellaneous
	Renderer::enableDepthTest();
	Renderer::enableBlend();
	
	bool gui = true;
	Camera camera;

	UpdateScheduler frameCounterScheduler(1);
	int frameCounter = 0;
	
	std::set<std::unique_ptr<Dialog> > dialogs;
	
	// Main Loop

	while (!win.shouldQuit()) {
		win.makeCurrent();
		Renderer::waitForComplete();
		Renderer::checkError();
		win.swapBuffers();
		
		Renderer::setRenderArea(0, 0, win.getWidth(), win.getHeight());
		Renderer::beginFinalPass();
		
		if (!gui) {
			Renderer::setClearColor(Vec3f(0.0f));
			Renderer::clear();

			Renderer::enableTexture2D();
			Renderer::enableAlphaTest();
			Renderer::disableCullFace();
			Renderer::disableStencilTest();

			Renderer::setProjection(camera.getProjectionMatrix());
			Renderer::setModelview(camera.getModelViewMatrix());
			
			// 3D rendering
			TextRenderer::drawAscii(Vec3f(0, 0, -10), "Hello, world!", -12, Vec3f(1.0f), Vec3f(0.0f));
			
		} else {
			Renderer::setClearColor(GUI::BackgroundColor);
			Renderer::clear();
			
			Renderer::enableCullFace();
			Renderer::enableStencilTest();
			Renderer::disableTexture2D();
			Renderer::disableAlphaTest();
			
			Renderer::setProjection(Mat4f::ortho(0, win.getWidth(), 0, win.getHeight(), -1, 1));
			Renderer::setModelview(Mat4f(1.0f));
			
			// Draw GUI
			form.render(win, Point2D(0, 0), Point2D(win.getWidth(), win.getHeight()));
		}

		Renderer::endFinalPass();
		
		// Render dialog windows
		for (auto& dialog: dialogs) dialog->render();
		
		frameCounter++;
		frameCounterScheduler.refresh();
		while (!frameCounterScheduler.inSync()) {
			std::stringstream ss;
			ss << "OpenGL Application (" << frameCounter << " FPS)";
			frameCounter = 0;
			win.setTitle(ss.str());
			frameCounterScheduler.increase();
		}

		Window::pollEvents();
		
		if (!gui) {
			camera.setPerspective(70.0f, float(win.getWidth()) / float(win.getHeight()), 0.1f, 256.0f);
			camera.update(win);
		} else {
			form.update(win, Point2D(0, 0), Point2D(win.getWidth(), win.getHeight()));
			if (testButton.clicked()) dialogs.insert(std::unique_ptr<Dialog>(new Dialog()));
		}
		
		for (auto& dialog: dialogs) dialog->update();
		for (auto it = dialogs.begin(); it != dialogs.end(); it++) {
			if ((*it)->window.shouldQuit() || (*it)->okButton.clicked() || (*it)->cancelButton.clicked()) {
				dialogs.erase(it);
				break;
			}
		}
		
		if (win.isKeyActed(SDL_SCANCODE_G)) gui = !gui;
		if (gui) Window::unlockCursor(); else Window::lockCursor();
		
		if (Window::isKeyPressed(SDL_SCANCODE_ESCAPE)) break;
	}

	Config::save();
	return 0;
}

