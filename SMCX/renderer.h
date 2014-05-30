/* Copyright (c) 2012 KrossX <krossx@live.com>
 * License: http://www.opensource.org/licenses/mit-license.html  MIT License
 */

#ifndef RENDERER_H
#define RENDERER_H

class Renderer
{
public:
	virtual void set_pixel(u16 x, u16 y, u8 pix) = 0;
	virtual void set_palette(u32 *pal) = 0;

	virtual void render_clear(bool reset_palette) = 0;
	virtual void render_frame() = 0;

	virtual void shutdown() = 0;
	virtual void restart(HWND hwnd) = 0;
	virtual HRESULT init(HWND hwnd) = 0;
};


#endif