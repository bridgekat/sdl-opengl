#include "renderer.h"
#include <sstream>
#include "common.h"

int Renderer::matrixMode = 0;
Mat4f Renderer::mProjection(1.0f), Renderer::mModelview(1.0f);
ShaderProgram Renderer::mFinal;

void Renderer::init() {
	glShadeModel(GL_SMOOTH);
	glDisable(GL_DITHER);
	glFrontFace(GL_CCW);
	glCullFace(GL_BACK);
	glDepthFunc(GL_LEQUAL);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glStencilFunc(GL_ALWAYS, 0, 0xFF);
	glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);

	if (OpenGL::coreProfile()) {
		mFinal.loadShadersFromFile(std::string(ShaderPath) + "Final.vsh", std::string(ShaderPath) + "Final.fsh");
		mFinal.bind();
	}
	
	enableCullFace();
	enableDepthTest();
	enableAlphaTest();
	setAlphaTestThreshold(0.0f);

	setClearColor(Vec3f(1.0f, 1.0f, 1.0f));
	setClearDepth(1.0f);
}

void Renderer::checkError() {
	GLenum err = glGetError();
	if (err) {
		std::stringstream ss;
		ss << "OpenGL error " << err;
		LogWarning(ss.str());
	}
}

