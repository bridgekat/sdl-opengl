#ifndef TEXTRENDERER_H_
#define TEXTRENDERER_H_

#include <string>
#include "vec.h"
#include "texture.h"
#include "shader.h"

class TextRenderer {
public:
	static void init();
	static void drawAscii(const Vec3f& pos, const std::string& text, float size, const Vec3f& col, const Vec3f& bgcol);
	
	static float getBoxHeight(float height, const std::string& s);
	static float getBoxWidth(float height, const std::string& s);

private:
	struct GlyphInfo {
		float tx = 0, ty = 0, tw = 0, th = 0, ext = 0;
		float left = 0, top = 0, advx = 0, advy = 0;
	};

	static Texture mAscii;
	static ShaderProgram mShader;
	static float mTextureSize, mDefaultFontSize, mGrayFactor, mSmoothFactor;
	static GlyphInfo mAsciiInfo[256];
};

#endif

