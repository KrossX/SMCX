/* Copyright (c) 2012 KrossX <krossx@live.com>
 * License: http://www.opensource.org/licenses/mit-license.html  MIT License
 */
 
#ifndef RENDERER_D3D11_H
#define RENDERER_D3D11_H

class Render
{
	// INTERFACES 
	IDXGISwapChain *swapchain;
	ID3D11Device *dev;
	ID3D11DeviceContext *devcon;
	ID3D11RenderTargetView *backbuffer;
	ID3D11VertexShader *pVS;
	ID3D11PixelShader *pPS;
	ID3D11Buffer *pVBuffer;
	ID3D11InputLayout *pLayout;
	ID3D11BlendState *pBlend;

	ID3D11Texture2D *texDisplay;
	ID3D11ShaderResourceView *srvDisplay;

	ID3D11Texture1D *texPalette;
	ID3D11ShaderResourceView *srvPalette;

	ID3D11SamplerState *ssPoint;

	// Shader Blobs
	ID3D10Blob *vShader, *pShader;

	// Main display and palette
	u32 display[256 * 192];
	u32 palette[256];

	// Init Functions
	void Init_Display();
	void Init_Palette();

	HRESULT Upload_Display();
	HRESULT Upload_Palette();

	HRESULT Init_Shaders(D3D11_VIEWPORT viewport);
	HRESULT Init_Textures();

	void Init_Nullify();

public:
	void SetPixel(u16 x, u16 y, u8 pix);
	void UpdatePalette(u32 *pal);

	void RenderClear(bool reset_palette);
	void RenderFrame(u8 vsync);
	
	void Shutdown();
	void Restart(HWND hWnd);
	HRESULT Init(HWND hWnd);
	Render();
};

#endif