#pragma once
#ifndef _BOXBLURSHADER_H_
#define _BOXBLURSHADER_H_

#include "BaseShader.h"

using namespace std;
using namespace DirectX;

class BoxBlurShader : public BaseShader
{
private:
	struct ScreenSizeBufferType
	{
		float screenWidth;
		float screenHeight;
		XMFLOAT2 padding;
	};

public:
	BoxBlurShader(ID3D11Device* device, HWND hwnd);
	~BoxBlurShader();

	void setShaderParameters(ID3D11DeviceContext* deviceContext, const XMMATRIX& world, const XMMATRIX& view, const XMMATRIX& projection, ID3D11ShaderResourceView* texture, float width, float height);

private:
	void initShader(const wchar_t* cs, const wchar_t* ps);

private:
	ID3D11Buffer* screenSizeBuffer;
};

#endif