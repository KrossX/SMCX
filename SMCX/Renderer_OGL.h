#ifndef RENDERER_OGL_H
#define RENDERER_OGL_H

class RENDERER_OGL: public RENDERER
{
public:
	void SetPixel(u16 x, u16 y, u8 pix);
	void UpdatePalette(u32 *pal);

	void RenderClear(bool reset_palette);
	void RenderFrame();

	void Shutdown();
	void Restart(HWND hWnd);
	HRESULT Init(HWND hWnd);
};



#endif