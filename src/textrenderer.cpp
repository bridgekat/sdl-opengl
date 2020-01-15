#include "textrenderer.h"
#include <fstream>
#include "vertexarray.h"
#include "renderer.h"
#include "config.h"
#include "common.h"

Texture TextRenderer::mAscii;
ShaderProgram TextRenderer::mShader;
float TextRenderer::mTextureSize, TextRenderer::mDefaultFontSize, TextRenderer::mGrayFactor, TextRenderer::mSmoothFactor;
TextRenderer::GlyphInfo TextRenderer::mAsciiInfo[256];

void TextRenderer::init() {
	std::string filename = std::string(FontPath) + Config::getString("GUI.Font", "Ascii");
	TextureImage fontImage; fontImage.loadFromPNG(filename + ".png", true, true);
	mAscii.load(fontImage, true, true, 0);
	mShader.loadShadersFromFile(std::string(ShaderPath) + "Font.vsh", std::string(ShaderPath) + "Font.fsh");
	mSmoothFactor = float(Config::getDouble("TextRenderer.SmoothFactor", 1.0));
	std::ifstream info(filename);
	if (!info.is_open()) {
		LogError("Could not open ASCII font info: " + filename);
		return;
	}
	float originSize, downScale;
	info >> mTextureSize >> originSize >> downScale >> mGrayFactor;
	mDefaultFontSize = originSize / downScale;
	for (int i = 0; i < 256; i++) {
		int index;
		info >> index;
		info >> mAsciiInfo[i].tx >> mAsciiInfo[i].ty >> mAsciiInfo[i].tw >> mAsciiInfo[i].th;
		info >> mAsciiInfo[i].ext;
		info >> mAsciiInfo[i].left >> mAsciiInfo[i].top >> mAsciiInfo[i].advx >> mAsciiInfo[i].advy;
		mAsciiInfo[i].tx /= downScale, mAsciiInfo[i].ty /= downScale;
		mAsciiInfo[i].tw /= downScale, mAsciiInfo[i].th /= downScale;
		mAsciiInfo[i].ext /= downScale;
		mAsciiInfo[i].left /= downScale, mAsciiInfo[i].top /= downScale;
		mAsciiInfo[i].advx /= downScale, mAsciiInfo[i].advy /= downScale;
	}
	info.close();
}

float TextRenderer::getBoxHeight(float height, const std::string& s) {
	float scale = height / mDefaultFontSize;
	float res = 0.0f;
	for (size_t i = 0; i < s.length(); i++) res = std::max(res, mAsciiInfo[s[i]].top);
	return res * scale;
}

float TextRenderer::getBoxWidth(float height, const std::string& s) {
	float scale = height / mDefaultFontSize;
	float res = 0.0f;
	for (size_t i = 0; i/* + 1*/ < s.length(); i++) res += mAsciiInfo[s[i]].advx;
//	if (!s.empty()) res += mAsciiInfo[s.size() - 1].left + mAsciiInfo[s.size() - 1].tw;
	return res * scale;
}

void TextRenderer::drawAscii(const Vec3f& pos, const std::string& text, float size, const Vec3f& col, const Vec3f& bgcol) {
	float scale = size / mDefaultFontSize;
	Vec3f cpos = pos;
	VertexArray va(text.length() * 6, VertexFormat(2, 3, 0, 3));
	va.setColor({col.x, col.y, col.z});
	for (size_t i = 0; i < text.length(); i++) {
		int curr = text[i];
		float ext = mAsciiInfo[curr].ext;
		float tx = (mAsciiInfo[curr].tx - ext) / mTextureSize, ty = (mAsciiInfo[curr].ty - ext) / mTextureSize;
		float tw = (mAsciiInfo[curr].tw + ext * 2) / mTextureSize, th = (mAsciiInfo[curr].th + ext * 2) / mTextureSize;
		float width = (mAsciiInfo[curr].tw + ext * 2) * scale, height = (mAsciiInfo[curr].th + ext * 2) * scale;
		Vec3f p = cpos + Vec3f(mAsciiInfo[curr].left - ext, mAsciiInfo[curr].th - mAsciiInfo[curr].top + ext, 0.0f) * scale;
		va.setTexture({tx, ty});
		va.addVertex({p.x, p.y - height, p.z});
		va.setTexture({tx, ty + th});
		va.addVertex({p.x, p.y, p.z});
		va.setTexture({tx + tw, ty});
		va.addVertex({p.x + width, p.y - height, p.z});
		va.setTexture({tx + tw, ty});
		va.addVertex({p.x + width, p.y - height, p.z});
		va.setTexture({tx, ty + th});
		va.addVertex({p.x, p.y, p.z});
		va.setTexture({tx + tw, ty + th});
		va.addVertex({p.x + width, p.y, p.z});
		cpos += Vec3f(mAsciiInfo[curr].advx, mAsciiInfo[curr].advy, 0.0f) * scale;
	}
//	Renderer::enableAlphaTest();
//	Renderer::setAlphaTestThreshold(0.5f);
	mAscii.bind();
	mShader.bind();
	mShader.setUniform1i("Texture", 0);
	mShader.setUniform1f("GrayFactor", mGrayFactor);
	mShader.setUniform1f("SmoothFactor", mSmoothFactor);
	mShader.setUniform1f("TextureSize", mTextureSize);
	mShader.setUniform3f("BackColor", bgcol.x, bgcol.y, bgcol.z);
	VertexBuffer(va, false).render();
	mShader.unbind();
//	Renderer::setAlphaTestThreshold(0.0f);
}

