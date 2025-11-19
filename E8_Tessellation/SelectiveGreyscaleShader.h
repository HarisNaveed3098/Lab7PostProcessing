#pragma once

#ifndef _SELECTIVEGREYSCALESHADER_H_
#define _SELECTIVEGREYSCALESHADER_H_

#include "BaseShader.h"

using namespace std;
using namespace DirectX;

class SelectiveGreyscaleShader : public BaseShader
{
private:
	struct ScreenSizeBufferType
	{
		float screenWidth;
		float screenHeight;
		XMFLOAT2 padding;
	};

	struct MouseBufferType
	{
		XMFLOAT2 mousePos;
		float radius;
		float feather;
	};

public:
	SelectiveGreyscaleShader(ID3D11Device* device, HWND hwnd);
	~SelectiveGreyscaleShader();

	void setShaderParameters(ID3D11DeviceContext* deviceContext, const XMMATRIX& world, const XMMATRIX& view,
		const XMMATRIX& projection, ID3D11ShaderResourceView* texture, float width, float height,
		float mouseX, float mouseY, float radius, float feather);

private:
	void initShader(const wchar_t* cs, const wchar_t* ps);

private:
	ID3D11Buffer* screenSizeBuffer;
	ID3D11Buffer* mouseBuffer;
};

#endif