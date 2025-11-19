#ifndef _GAUSSIANBLURSHADER_H_
#define _GAUSSIANBLURSHADER_H_
#include "GaussianWeights.h"

#include "BaseShader.h"

using namespace std;
using namespace DirectX;

class GaussianBlurShader : public BaseShader
{
private:
	struct ScreenSizeBufferType
	{
		float screenWidth;
		float screenHeight;
		XMFLOAT2 padding;
	};

	struct WeightsBufferType
	{
		float weight0;
		float weight1;
		float weight2;
		float weight3;
		float weight4;
		XMFLOAT3 padding;
	};

public:
	

	GaussianBlurShader(ID3D11Device* device, HWND hwnd, bool horizontal);
	~GaussianBlurShader();

	void setShaderParameters(ID3D11DeviceContext* deviceContext, const XMMATRIX& world, const XMMATRIX& view,
		const XMMATRIX& projection, ID3D11ShaderResourceView* texture, float width, float height, GaussianWeights weights);

private:
	void initShader(const wchar_t* cs, const wchar_t* ps);

private:
	ID3D11Buffer* screenSizeBuffer;
	ID3D11Buffer* weightsBuffer;
	bool isHorizontal;
};

#endif