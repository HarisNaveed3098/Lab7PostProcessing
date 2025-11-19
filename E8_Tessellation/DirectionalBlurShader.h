#pragma once
#ifndef _DIRECTIONALBLURSHADER_H_
#define _DIRECTIONALBLURSHADER_H_

#include "BaseShader.h"

using namespace std;
using namespace DirectX;

class DirectionalBlurShader : public BaseShader
{
private:
	struct ScreenSizeBufferType
	{
		float screenWidth;
		float screenHeight;
		XMFLOAT2 padding;
	};

	struct BlurBufferType
	{
		float angle;
		float distance;
		int numSamples;
		float padding;
	};

public:
	DirectionalBlurShader(ID3D11Device* device, HWND hwnd);
	~DirectionalBlurShader();

	void setShaderParameters(ID3D11DeviceContext* deviceContext, const XMMATRIX& world, const XMMATRIX& view,
		const XMMATRIX& projection, ID3D11ShaderResourceView* texture, float width, float height,
		float angle, float distance, int numSamples);

private:
	void initShader(const wchar_t* cs, const wchar_t* ps);

private:
	ID3D11Buffer* screenSizeBuffer;
	ID3D11Buffer* blurBuffer;
};

#endif