#include "textrenderer.h"
#include "vertexarray.h"

Texture TextRenderer::mAscii;

void TextRenderer::loadAscii(const std::string& filename) {
	mAscii.load(TextureImage(filename, true), true);
}

void TextRenderer::drawAscii(const Vec3f& pos, const std::string& text, float height, bool shade, const Vec3f& col) {
	mAscii.bind();
	float width = height * 0.6f;
	const float TexSize = 256.0f, TexWidth = 6.0f, TexHeight = 10.0f;
	const float WidthExpand = 7.0f / TexWidth, HeightExpand = 15.0f / TexHeight;
	width *= WidthExpand, height *= HeightExpand;
	if (shade) {
		const float Eps = 1e-2f;
		float len = text.length() * width;
		VertexArray va(6, VertexFormat(2, 3, 0, 3));
		va.setColor({0.0f, 0.0f, 0.0f});
		va.setTexture({1.0f - TexWidth / 2.0f / TexSize, TexHeight / 2.0f / TexSize});
		va.addVertex({pos.x, pos.y, pos.z - Eps});
		va.addVertex({pos.x, pos.y + height, pos.z - Eps});
		va.addVertex({pos.x + len, pos.y, pos.z - Eps});
		va.addVertex({pos.x + len, pos.y, pos.z - Eps});
		va.addVertex({pos.x, pos.y + height, pos.z - Eps});
		va.addVertex({pos.x + len, pos.y + height, pos.z - Eps});
		VertexBuffer(va, false).render();
	}
	float offset = 0.0f;
	VertexArray va(text.length() * 6, VertexFormat(2, 3, 0, 3));
	va.setColor({col.x, col.y, col.z});
	for (size_t i = 0; i < text.length(); i++) {
		int curr = text[i];
		float tx = ((curr % 16) * 8) / TexSize;
		float ty = ((curr / 16) * 16 + 16 - 10) / TexSize;
		float tw = TexWidth * WidthExpand / 256.0f, th = TexHeight * HeightExpand / 256.0f;
		va.setTexture({tx, 1.0f - ty});
		va.addVertex({pos.x + offset, pos.y, pos.z});
		va.setTexture({tx, 1.0f - ty - th});
		va.addVertex({pos.x + offset, pos.y + height, pos.z});
		va.setTexture({tx + tw, 1.0f - ty});
		va.addVertex({pos.x + offset + width, pos.y, pos.z});
		va.setTexture({tx + tw, 1.0f - ty});
		va.addVertex({pos.x + offset + width, pos.y, pos.z});
		va.setTexture({tx, 1.0f - ty - th});
		va.addVertex({pos.x + offset, pos.y + height, pos.z});
		va.setTexture({tx + tw, 1.0f - ty - th});
		va.addVertex({pos.x + offset + width, pos.y + height, pos.z});
		offset += width;
	}
	VertexBuffer(va, false).render();
}

