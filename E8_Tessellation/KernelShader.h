
#ifndef _KERNELSHADER_H_
#define _KERNELSHADER_H_

#include "BaseShader.h"

using namespace std;
using namespace DirectX;

class KernelShader : public BaseShader
{
public:
	enum KernelType {
		BOX_BLUR = 0,
		EDGE_DETECTION = 1,
		SHARPEN = 2,
		GAUSSIAN_APPROX = 3,
		CUSTOM = 4
	};

private:
	struct ScreenSizeBufferType
	{
		float screenWidth;
		float screenHeight;
		XMFLOAT2 padding;
	};

	struct KernelBufferType
	{
		XMFLOAT4 kernelRow0; 
		XMFLOAT4 kernelRow1;  
		XMFLOAT4 kernelRow2;  
	};

public:
	KernelShader(ID3D11Device* device, HWND hwnd);
	~KernelShader();

	void setShaderParameters(ID3D11DeviceContext* deviceContext, const XMMATRIX& world, const XMMATRIX& view, const XMMATRIX& projection, ID3D11ShaderResourceView* texture, float width, float height);
	void setKernel(int kernelType);

private:
	void initShader(const wchar_t* vs, const wchar_t* ps);

private:
	ID3D11Buffer* screenSizeBuffer;
	ID3D11Buffer* kernelBuffer;
	float currentKernel[9];
	float currentDivisor;
};

#endif