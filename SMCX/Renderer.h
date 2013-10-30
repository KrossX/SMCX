/*  SMCX - SuperMegaChip-X Emulator
 *  Copyright (C) 2012  KrossX
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include <d3d11.h>

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