#version 110
#define ANTIALIASING_SUBPIXEL

uniform sampler2D Texture;
uniform float TextureSize;
uniform float GrayFactor;
uniform float SmoothFactor;
uniform vec3 BackColor;

void main() {
	float texelsPerPixel = max(length(dFdx(gl_TexCoord[0].xy)), length(dFdy(gl_TexCoord[0].xy))) * TextureSize;
#ifdef ANTIALIASING_SUBPIXEL
	float col[5], alpha = 0.0;
	vec2 shift = dFdx(gl_TexCoord[0].xy) / 3.0 * 1.0;
	for (int i = 0; i < 5; i++) {
		col[i] = texture2D(Texture, gl_TexCoord[0].xy + (float(i) - 2.0) * shift).a - 0.5;
		col[i] = clamp(col[i] / GrayFactor / texelsPerPixel / SmoothFactor + 0.5, 0.0, 1.0);
	}
	for (int i = 1; i <= 3; i++) {
		col[i] = (col[i - 1] + col[i] + col[i + 1]) / 3.0;
		if (col[i] > 0.0) alpha = 1.0/*max(alpha, pow(col[i], 0.4))*/;
	}
	gl_FragColor = vec4(mix(BackColor, gl_Color.rgb, vec3(col[1], col[2], col[3])), alpha);
#else
	float col = texture2D(Texture, gl_TexCoord[0].xy).a - 0.5;
	col = clamp(col / GrayFactor / texelsPerPixel / SmoothFactor + 0.5, 0.0, 1.0);
	gl_FragColor = vec4(gl_Color.rgb, col * gl_Color.a);
#endif
}

