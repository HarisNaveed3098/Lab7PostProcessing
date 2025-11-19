#include "DirectionalBlurShader.h"

DirectionalBlurShader::DirectionalBlurShader(ID3D11Device* device, HWND hwnd) : BaseShader(device, hwnd)
{
	initShader(L"texture_vs.cso", L"directional_blur_ps.cso");
}

DirectionalBlurShader::~DirectionalBlurShader()
{
	if (screenSizeBuffer)
	{
		screenSizeBuffer->Release();
		screenSizeBuffer = 0;
	}
	if (blurBuffer)
	{
		blurBuffer->Release();
		blurBuffer = 0;
	}

	BaseShader::~BaseShader();
}

void DirectionalBlurShader::initShader(const wchar_t* vsFilename, const wchar_t* psFilename)
{
	D3D11_BUFFER_DESC matrixBufferDesc;
	D3D11_SAMPLER_DESC samplerDesc;
	D3D11_BUFFER_DESC screenSizeBufferDesc;
	D3D11_BUFFER_DESC blurBufferDesc;

	loadVertexShader(vsFilename);
	loadPixelShader(psFilename);

	// Setup matrix buffer
	matrixBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	matrixBufferDesc.ByteWidth = sizeof(MatrixBufferType);
	matrixBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	matrixBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	matrixBufferDesc.MiscFlags = 0;
	matrixBufferDesc.StructureByteStride = 0;
	renderer->CreateBuffer(&matrixBufferDesc, NULL, &matrixBuffer);

	// Create sampler
	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.MipLODBias = 0.0f;
	samplerDesc.MaxAnisotropy = 1;
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
	samplerDesc.MinLOD = 0;
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;
	renderer->CreateSamplerState(&samplerDesc, &sampleState);

	// Setup screen size buffer
	screenSizeBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	screenSizeBufferDesc.ByteWidth = sizeof(ScreenSizeBufferType);
	screenSizeBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	screenSizeBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	screenSizeBufferDesc.MiscFlags = 0;
	screenSizeBufferDesc.StructureByteStride = 0;
	renderer->CreateBuffer(&screenSizeBufferDesc, NULL, &screenSizeBuffer);

	// Setup blur buffer
	blurBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	blurBufferDesc.ByteWidth = sizeof(BlurBufferType);
	blurBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	blurBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	blurBufferDesc.MiscFlags = 0;
	blurBufferDesc.StructureByteStride = 0;
	renderer->CreateBuffer(&blurBufferDesc, NULL, &blurBuffer);
}

void DirectionalBlurShader::setShaderParameters(ID3D11DeviceContext* deviceContext, const XMMATRIX& worldMatrix,
	const XMMATRIX& viewMatrix, const XMMATRIX& projectionMatrix, ID3D11ShaderResourceView* texture,
	float width, float height, float angle, float distance, int numSamples)
{
	HRESULT result;
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	MatrixBufferType* dataPtr;

	// Transpose matrices
	XMMATRIX tworld = XMMatrixTranspose(worldMatrix);
	XMMATRIX tview = XMMatrixTranspose(viewMatrix);
	XMMATRIX tproj = XMMatrixTranspose(projectionMatrix);

	// Lock matrix buffer
	result = deviceContext->Map(matrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	dataPtr = (MatrixBufferType*)mappedResource.pData;
	dataPtr->world = tworld;
	dataPtr->view = tview;
	dataPtr->projection = tproj;
	deviceContext->Unmap(matrixBuffer, 0);
	deviceContext->VSSetConstantBuffers(0, 1, &matrixBuffer);

	// Lock screen size buffer
	ScreenSizeBufferType* screenPtr;
	result = deviceContext->Map(screenSizeBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	screenPtr = (ScreenSizeBufferType*)mappedResource.pData;
	screenPtr->screenWidth = width;
	screenPtr->screenHeight = height;
	screenPtr->padding = XMFLOAT2(0.0f, 0.0f);
	deviceContext->Unmap(screenSizeBuffer, 0);
	deviceContext->PSSetConstantBuffers(0, 1, &screenSizeBuffer);

	// Lock blur buffer
	BlurBufferType* blurPtr;
	result = deviceContext->Map(blurBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	blurPtr = (BlurBufferType*)mappedResource.pData;
	blurPtr->angle = angle;
	blurPtr->distance = distance;
	blurPtr->numSamples = numSamples;
	blurPtr->padding = 0.0f;
	deviceContext->Unmap(blurBuffer, 0);
	deviceContext->PSSetConstantBuffers(1, 1, &blurBuffer);

	// Set texture
	deviceContext->PSSetShaderResources(0, 1, &texture);
	deviceContext->PSSetSamplers(0, 1, &sampleState);
}