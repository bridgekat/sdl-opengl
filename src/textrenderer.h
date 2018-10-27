#ifndef TEXTRENDERER_H_
#define TEXTRENDERER_H_

#include <string>
#include "vec.h"
#include "texture.h"

class TextRenderer {
public:
	static void loadAscii(const std::string& filename);
	static void drawAscii(const Vec3f& pos, const std::string& text, float height = 10.0f, bool shade = false, const Vec3f& col = Vec3f(1.0f, 1.0f, 1.0f));
	
private:
	static Texture mAscii;
};

#endif

