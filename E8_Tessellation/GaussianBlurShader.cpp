#include "GaussianBlurShader.h"

GaussianBlurShader::GaussianBlurShader(ID3D11Device* device, HWND hwnd, bool horizontal) : BaseShader(device, hwnd)
{
	isHorizontal = horizontal;
	if (horizontal)
		initShader(L"texture_vs.cso", L"gaussian_horizontal_ps.cso");
	else
		initShader(L"texture_vs.cso", L"gaussian_vertical_ps.cso");
}

GaussianBlurShader::~GaussianBlurShader()
{
	if (screenSizeBuffer)
	{
		screenSizeBuffer->Release();
		screenSizeBuffer = 0;
	}
	if (weightsBuffer)
	{
		weightsBuffer->Release();
		weightsBuffer = 0;
	}

	BaseShader::~BaseShader();
}

void GaussianBlurShader::initShader(const wchar_t* vsFilename, const wchar_t* psFilename)
{
	D3D11_BUFFER_DESC matrixBufferDesc;
	D3D11_SAMPLER_DESC samplerDesc;
	D3D11_BUFFER_DESC screenSizeBufferDesc;
	D3D11_BUFFER_DESC weightsBufferDesc;

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

	// Setup weights buffer
	weightsBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	weightsBufferDesc.ByteWidth = sizeof(WeightsBufferType);
	weightsBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	weightsBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	weightsBufferDesc.MiscFlags = 0;
	weightsBufferDesc.StructureByteStride = 0;
	renderer->CreateBuffer(&weightsBufferDesc, NULL, &weightsBuffer);
}

void GaussianBlurShader::setShaderParameters(ID3D11DeviceContext* deviceContext, const XMMATRIX& worldMatrix,
	const XMMATRIX& viewMatrix, const XMMATRIX& projectionMatrix, ID3D11ShaderResourceView* texture,
	float width, float height, GaussianWeights weights)
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

	// Lock weights buffer
	WeightsBufferType* weightsPtr;
	result = deviceContext->Map(weightsBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	weightsPtr = (WeightsBufferType*)mappedResource.pData;
	weightsPtr->weight0 = weights.weight0;
	weightsPtr->weight1 = weights.weight1;
	weightsPtr->weight2 = weights.weight2;
	weightsPtr->weight3 = weights.weight3;
	weightsPtr->weight4 = weights.weight4;
	weightsPtr->padding = XMFLOAT3(0.0f, 0.0f, 0.0f);
	deviceContext->Unmap(weightsBuffer, 0);
	deviceContext->PSSetConstantBuffers(1, 1, &weightsBuffer);

	// Set texture
	deviceContext->PSSetShaderResources(0, 1, &texture);
	deviceContext->PSSetSamplers(0, 1, &sampleState);
}