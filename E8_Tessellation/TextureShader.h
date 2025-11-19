#pragma once
#ifndef _TEXTURESHADER_H_
#define _TEXTURESHADER_H_

#include "BaseShader.h"
#include "Light.h"

using namespace std;
using namespace DirectX;

class TextureShader : public BaseShader
{
private:
	struct LightBufferType
	{
		XMFLOAT4 ambient;
		XMFLOAT4 diffuse;
		XMFLOAT3 direction;
		float padding;
	};

public:
	TextureShader(ID3D11Device* device, HWND hwnd);
	~TextureShader();

	void setShaderParameters(ID3D11DeviceContext* deviceContext, const XMMATRIX& world, const XMMATRIX& view, const XMMATRIX& projection, ID3D11ShaderResourceView* texture, Light* light);

private:
	void initShader(const wchar_t* cs, const wchar_t* ps);

private:
	ID3D11Buffer* lightBuffer;
};

#endif