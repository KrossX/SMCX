/* Copyright (c) 2012 KrossX <krossx@live.com>
 * License: http://www.opensource.org/licenses/mit-license.html  MIT License
 */

Texture2D Display : register(t0);
Texture1D Palette : register(t1);
SamplerState PointSampler : register(s0);

//#define VIEW_W
//#define VIEW_H
//#define ASPECT

#define TEX_W 256.0
#define TEX_H 192.0

struct VS_INPUT
{
    float4 pos : POSITION;
    float2 uv  : TEXCOORD0;
	float4 col : COLOR;
};

struct VS_OUTPUT
{
    float4 pos : SV_POSITION;
    float2 uv  : TEXCOORD0;
	float4 col : COLOR;
};

struct PS_INPUT
{
	float4 pos : SV_Position;
	float2 uv  : TEXCOORD0;
};

struct PS_OUTPUT
{
	float4 col : SV_TARGET;
};


VS_OUTPUT vs_main(VS_INPUT input)
{
    VS_OUTPUT output;
	
	output.pos = input.pos;

	if(ASPECT < 1.33)
		output.pos.y *= 1.33 / ASPECT;
		
	else if(ASPECT > 1.33)
		output.pos.x *= 1.33 / ASPECT;		
	
	output.uv = input.uv;
	output.col = input.col;
	

    return output;
}

PS_OUTPUT ps_main(VS_OUTPUT input)
{
	PS_OUTPUT output;
	
	//const float2 offset = float2(2 / VIEW_W, 2 / VIEW_H);
	const float2 ofx = float2(1/TEX_W, 0.0);
	const float2 ofy = float2(0.0, 1/TEX_H);
		
	float4 back, pal00, pal01, pal10, pal11;
	back = Display.Sample(PointSampler, input.uv);				pal00 = Palette.Sample(PointSampler, back.r);
	back = Display.Sample(PointSampler, input.uv + ofx);		pal10 = Palette.Sample(PointSampler, back.r);
	back = Display.Sample(PointSampler, input.uv + ofy);		pal01 = Palette.Sample(PointSampler, back.r);
	back = Display.Sample(PointSampler, input.uv + ofx + ofy);	pal11 = Palette.Sample(PointSampler, back.r);
			
	/*
	float4 cols = Display.Sample(Sampler, input.uv - offset);
	float4 colh = Display.Sample(Sampler, input.uv + offset);
	
	if(col.b < cols.b) col.rgb += 0.3;
	if(col.b < colh.b) col.rgb *= 0.5;
	*/
	
	const float dx = frac(input.uv.x * TEX_W);
	const float dy = frac(input.uv.y * TEX_H);
	
	output.col = lerp(lerp(pal00, pal10, dx), lerp(pal01, pal11, dx), dy);

    return output;
}
