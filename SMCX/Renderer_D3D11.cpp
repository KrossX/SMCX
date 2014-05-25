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

#include "Main.h"

#ifdef _USED3D11
#include <cstdio>

#include <d3d11.h>
#include <d3dx11.h>
#include <d3dx10.h>

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "d3dx11.lib")
#pragma comment(lib, "d3dx10.lib")

#include "Renderer.h"


struct VERTEX
{
	FLOAT X, Y, Z;
	FLOAT U, V;
	D3DXCOLOR Color;
};

enum COLORS
{
	D3DC_BACKGROUND,
	D3DC_FOREGROUND,
	D3DC_WHITE,
	D3DC_BLACK,
	D3DC_RED,
	D3DC_GREEN,
	D3DC_BLUE,
	D3DC_GREENISH,
	D3DC_WHITE0
};

D3DXCOLOR D3D_COLORS[] = 
{
	D3DXCOLOR(109 / 255.0f, 143 / 255.0f, 120 / 255.0f, 1.0f), // Background
	D3DXCOLOR(43 / 255.0f, 68 / 255.0f, 109 / 255.0f, 1.0f), // Foreground
	D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f), // White
	D3DXCOLOR(0.0f, 0.0f, 0.0f, 1.0f), // Black
	D3DXCOLOR(1.0f, 0.0f, 0.0f, 1.0f), // Red
	D3DXCOLOR(0.0f, 1.0f, 0.0f, 1.0f), // Green
	D3DXCOLOR(0.0f, 0.0f, 1.0f, 1.0f), // Blue
	D3DXCOLOR(119 / 255.0f, 146 / 255.0f, 116 / 255.0f, 1.0f),  // Greenish
	D3DXCOLOR(1/255.0f, 1/255.0f, 1/255.0f, 1.0f)
};

VERTEX BackQuad[4] = 
{
	{-1, -1, 0.0f, 0.0f, 1.0f, D3D_COLORS[D3DC_RED]},
	{-1,  1, 0.0f, 0.0f, 0.0f, D3D_COLORS[D3DC_GREEN]},
	{ 1, -1, 0.0f, 1.0f, 1.0f, D3D_COLORS[D3DC_BLUE]},
	{ 1,  1, 0.0f, 1.0f, 0.0f, D3D_COLORS[D3DC_WHITE]}
};

Render::Render()
{
	Init_Nullify();
	Init_Display();
	Init_Palette();
}

void Render::Init_Nullify()
{
	swapchain = NULL;
	dev = NULL;
	devcon = NULL;
	backbuffer = NULL;
	pVS = NULL;
	pPS = NULL;
	pVBuffer = NULL;
	pLayout = NULL;
	pBlend = NULL;
	
	texDisplay = NULL;
	srvDisplay = NULL;

	texPalette = NULL;
	srvPalette = NULL;

	ssPoint = NULL;

	vShader = NULL;
	pShader = NULL;
}

void Render::Init_Display()
{
	for(u32 p = 0; p < 49152; p++)
		display[p] = D3D_COLORS[D3DC_BLACK];
}

void Render::Init_Palette()
{
	for(u16 p = 0; p < 256; p++)
		palette[p] = D3DXCOLOR(p/255.0f, p/255.0f, p/255.0f, 1.0f);

	palette[0] = D3D_COLORS[D3DC_BACKGROUND];
	palette[1] = D3D_COLORS[D3DC_FOREGROUND];
}

void Render::UpdatePalette(u32 *pal)
{
	//ARGB to d3d BGRA
	u8 a, r, g, b;

	for(int i = 0; i < 256; i++)
	{
		a = (pal[i] >> 24) & 0xFF;
		r = (pal[i] >> 16) & 0xFF;
		g = (pal[i] >> 8) & 0xFF;
		b = pal[i] & 0xFF;

		palette[i] = D3DXCOLOR(r/255.0f, g/255.0f, b/255.0f, a/255.0f);
	}

	Upload_Palette();
}

void Render::SetPixel(u16 x, u16 y, u8 pix)
{
	u16 pixel = x + y * 256;

	float col = pix / 255.0f;
	display[pixel] = D3DXCOLOR(col, col, col, 1.0f);
};

void Render::RenderClear(bool reset_palette)
{
	//devcon->ClearRenderTargetView(backbuffer, D3D_COLORS[D3DC_GREENISH]);
	Init_Display();
	
	if(reset_palette) 
	{
		Init_Palette();
		Upload_Palette();
	}
	//Upload_Display();
	//devcon->Draw(4, 0);
	//swapchain->Present(0, 0);
}

void Render::RenderFrame(u8 vsync)
{
	devcon->ClearRenderTargetView(backbuffer, D3D_COLORS[D3DC_GREENISH]);
	Upload_Display();
	devcon->Draw(4, 0);
	swapchain->Present(vsync, 0);
	//Sleep(16);
}

void Render::Shutdown()
{
	if(swapchain) swapchain->Release();
	if(dev) dev->Release();
	if(devcon) devcon->Release();
	if(backbuffer) backbuffer->Release();
	if(pPS) pPS->Release();
	if(pVS) pVS->Release();
	if(pVBuffer) pVBuffer->Release();
	if(pBlend) pBlend->Release();
	if(pLayout) pLayout->Release();

	if(texDisplay) texDisplay->Release();
	if(srvDisplay) srvDisplay->Release();
	
	if(texPalette) texPalette->Release();
	if(srvPalette) srvPalette->Release();

	if(ssPoint) ssPoint->Release();

	Init_Nullify();
}

HRESULT Render::Init_Shaders(D3D11_VIEWPORT viewport)
{
	HRESULT result = S_FALSE;

	char mValues [3][256] = {0};

	sprintf_s(mValues[0], "%f", viewport.Width);
	sprintf_s(mValues[1], "%f", viewport.Height);
	sprintf_s(mValues[2], "%f", viewport.Width / viewport.Height);
		
	D3D_SHADER_MACRO macro[] = 
	{
		{"VIEW_W", mValues[0]},
		{"VIEW_H", mValues[1]},
		{"ASPECT", mValues[2]},
		{NULL, NULL}
	};

	D3DX11CompileFromFile(L"shaders.fx", macro, 0, "vs_main", "vs_4_0", 0, 0, 0, &vShader, 0, 0);
	D3DX11CompileFromFile(L"shaders.fx", macro, 0, "ps_main", "ps_4_0", 0, 0, 0, &pShader, 0, 0);

	if(vShader && pShader)
	{
		HRESULT vr, pr;

		vr = dev->CreateVertexShader(vShader->GetBufferPointer(), vShader->GetBufferSize(), NULL, &pVS);
		pr = dev->CreatePixelShader(pShader->GetBufferPointer(), pShader->GetBufferSize(), NULL, &pPS);

		if(vr == S_OK && pr == S_OK) 
		{
			devcon->VSSetShader(pVS, 0, 0);
			devcon->PSSetShader(pPS, 0, 0);
			result = S_OK;
		}
	}

	if(result == S_OK)
	{
		/** Input Layout **/

		D3D11_INPUT_ELEMENT_DESC ied[] =
		{
			{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
			{"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
			{"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0}
		};

		result = dev->CreateInputLayout(ied, 3, vShader->GetBufferPointer(), vShader->GetBufferSize(), &pLayout);
		if(result == S_OK) devcon->IASetInputLayout(pLayout);
	}

	return result;
}


HRESULT Render::Init(HWND hWnd)
{
	DXGI_SWAP_CHAIN_DESC scd;
	ZeroMemory(&scd, sizeof(DXGI_SWAP_CHAIN_DESC));

	scd.BufferCount = 1;									// one back buffer
	scd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;		// use 32-bit color
	scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;		// how swap chain is to be used
	scd.OutputWindow = hWnd;								// the window to be used
	scd.SampleDesc.Count = 1;								// how many multisamples
	scd.Windowed = TRUE;									// windowed/full-screen mode

	HRESULT result = D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, NULL, NULL, NULL,
		D3D11_SDK_VERSION, &scd, &swapchain, &dev, NULL, &devcon);

	if(result != S_OK) return result;

	ID3D11Texture2D *pBackBuffer;
	result = swapchain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer);

	if(result != S_OK) return result;

	result = dev->CreateRenderTargetView(pBackBuffer, NULL, &backbuffer);
	pBackBuffer->Release();

	if(result != S_OK) return result;

	devcon->OMSetRenderTargets(1, &backbuffer, NULL);

	/** Viewport **************************************************************/

	RECT rect;
	D3D11_VIEWPORT viewport;
	ZeroMemory(&viewport, sizeof(D3D11_VIEWPORT));

	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;

	if(GetClientRect(hWnd, &rect))
	{
		viewport.Width = (rect.right - rect.left) * 1.0f;
		viewport.Height = (rect.bottom - rect.top) * 1.0f;
	}
	else
	{
		viewport.Width = 320;
		viewport.Height = 240;
	}

	devcon->RSSetViewports(1, &viewport);

	/** Vertex Buffer/Plane **/

	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));

	bd.Usage = D3D11_USAGE_DYNAMIC;
	bd.ByteWidth = sizeof(BackQuad);
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	D3D11_SUBRESOURCE_DATA sbd;
	ZeroMemory(&sbd, sizeof(sbd));

	sbd.pSysMem = BackQuad;

	result = dev->CreateBuffer(&bd, &sbd, &pVBuffer);

	if(result != S_OK) return result;

	/** Blending Mode **************************************************************/

	D3D11_BLEND_DESC blend;
	blend.AlphaToCoverageEnable = false;
	blend.IndependentBlendEnable = false;
	blend.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
	blend.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	blend.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	blend.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	blend.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
	blend.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	blend.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

	result = dev->CreateBlendState(&blend, &pBlend);
	if(result != S_OK) return result;

	float blendFactor[4] = {0.0f, 0.0f, 0.0f, 0.0f};
	devcon->OMSetBlendState(pBlend, blendFactor, 0xFFFFFFFF); // Use it...

	/** Textures **/
	result = Init_Textures();
	if(result != S_OK) return result;
	
	/** Shaders **/
	result = Init_Shaders(viewport);
	if(result != S_OK) return result;

	/** Vertex Buffer and Stuff **/

	UINT stride = sizeof(VERTEX);
	UINT offset = 0;
	devcon->IASetVertexBuffers(0, 1, &pVBuffer, &stride, &offset);
	devcon->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

	return result;
}

HRESULT Render::Init_Textures()
{
	HRESULT result;

	/** Display Texture  **/
		
	D3D11_TEXTURE2D_DESC t2D_desc;
	ZeroMemory(&t2D_desc, sizeof(t2D_desc));

	t2D_desc.Width = 256;
	t2D_desc.Height = 192;
	t2D_desc.MipLevels = 1;
	t2D_desc.ArraySize = 1;
	t2D_desc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
	t2D_desc.SampleDesc.Count = 1;
	t2D_desc.Usage = D3D11_USAGE_DYNAMIC;
	t2D_desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	t2D_desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		
	result = dev->CreateTexture2D(&t2D_desc, NULL, &texDisplay);
	if(result != S_OK) return result;
	
	result = Upload_Display();
	if(result != S_OK) return result;

	/** Palette Texture **/

	D3D11_TEXTURE1D_DESC t1D_desc;
	ZeroMemory(&t1D_desc, sizeof(t1D_desc));

	t1D_desc.Width = 256;
	t1D_desc.MipLevels = 1;
	t1D_desc.ArraySize = 1;
	t1D_desc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
	t1D_desc.Usage = D3D11_USAGE_DYNAMIC;
	t1D_desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	t1D_desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		
	result = dev->CreateTexture1D(&t1D_desc, NULL, &texPalette);
	if(result != S_OK) return result;

	result = Upload_Palette();
	if(result != S_OK) return result;

	
	/** Set Display Texture Shader Resource **/ 

	D3D11_SHADER_RESOURCE_VIEW_DESC srvdesc;
	ZeroMemory(&srvdesc, sizeof(srvdesc));

	srvdesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
	srvdesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvdesc.Texture2D.MipLevels = 1;
	srvdesc.Texture2D.MostDetailedMip = 0;

	result = dev->CreateShaderResourceView(texDisplay, &srvdesc, &srvDisplay);

	if(result != S_OK) return result;

	/** Set Palette Texture Shader Resource **/ 

	ZeroMemory(&srvdesc, sizeof(srvdesc));

	srvdesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
	srvdesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE1D;
	srvdesc.Texture1D.MipLevels = 1;
	srvdesc.Texture1D.MostDetailedMip = 0;

	result = dev->CreateShaderResourceView(texPalette, &srvdesc, &srvPalette);

	if(result != S_OK) return result;

	/** Texture Sampler ****/
	D3D11_SAMPLER_DESC sampler;
	ZeroMemory(&sampler, sizeof(sampler));

	sampler.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
	sampler.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	sampler.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	sampler.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	sampler.MipLODBias = 0.0f;
	sampler.MaxAnisotropy = 1;
	sampler.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
	sampler.BorderColor[0] = 0;
	sampler.BorderColor[1] = 0;
	sampler.BorderColor[2] = 0;
	sampler.BorderColor[3] = 0;
	sampler.MinLOD = 0;
	sampler.MaxLOD = 0;

	result = dev->CreateSamplerState(&sampler, &ssPoint); 

	if(result != S_OK) return result;
	
	devcon->PSSetShaderResources(0, 1, &srvDisplay);
	devcon->PSSetShaderResources(1, 1, &srvPalette);
	devcon->PSSetSamplers(0, 1, &ssPoint);

	return result;
}

HRESULT Render::Upload_Display()
{
	HRESULT result;

	D3D11_MAPPED_SUBRESOURCE mapped_sr;
	mapped_sr.RowPitch = 256 * 4;
	result = devcon->Map(texDisplay, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &mapped_sr);
		
	if(result != S_OK) return result;

	memcpy(mapped_sr.pData, display, sizeof(display));
	devcon->Unmap(texDisplay, NULL);
		
	return result;
}

HRESULT Render::Upload_Palette()
{
	HRESULT result;

	D3D11_MAPPED_SUBRESOURCE mapped_sr;
	result = devcon->Map(texPalette, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &mapped_sr);
		
	if(result != S_OK) return result;

	memcpy(mapped_sr.pData, palette, sizeof(palette));
	devcon->Unmap(texPalette, NULL);
		
	return result;
}

void Render::Restart(HWND hWnd)
{
	Shutdown();
	Init(hWnd);
}
#endif