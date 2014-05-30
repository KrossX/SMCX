/* Copyright (c) 2012 KrossX <krossx@live.com>
 * License: http://www.opensource.org/licenses/mit-license.html  MIT License
 */

#ifndef RENDERER_OGL_H
#define RENDERER_OGL_H

class Renderer_OGL: public Renderer
{
public:
	void set_pixel(u16 x, u16 y, u8 pix);
	void set_palette(u32 *pal);

	void render_clear(bool reset_palette);
	void render_frame();

	void shutdown();
	void restart(HWND hwnd);
	HRESULT init(HWND hwnd);
};



#endif