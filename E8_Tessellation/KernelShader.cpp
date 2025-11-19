
#include "KernelShader.h"

KernelShader::KernelShader(ID3D11Device* device, HWND hwnd) : BaseShader(device, hwnd)
{
	// Initialize pointers to NULL first
	screenSizeBuffer = nullptr;
	kernelBuffer = nullptr;

	// Initialize currentKernel array to ZEROS first
	for (int i = 0; i < 9; i++)
		currentKernel[i] = 0.0f;
	currentDivisor = 1.0f;

	initShader(L"texture_vs.cso", L"kernel_ps.cso");

	// CRITICAL: Set kernel AFTER initShader
	setKernel(BOX_BLUR);
}

KernelShader::~KernelShader()
{
	if (screenSizeBuffer)
	{
		screenSizeBuffer->Release();
		screenSizeBuffer = 0;
	}
	if (kernelBuffer)
	{
		kernelBuffer->Release();
		kernelBuffer = 0;
	}

	// BaseShader destructor is called automatically - don't call it explicitly
}

void KernelShader::initShader(const wchar_t* vsFilename, const wchar_t* psFilename)
{
	D3D11_BUFFER_DESC matrixBufferDesc;
	D3D11_SAMPLER_DESC samplerDesc;
	D3D11_BUFFER_DESC screenSizeBufferDesc;
	D3D11_BUFFER_DESC kernelBufferDesc;
	HRESULT result;

	try {
		loadVertexShader(vsFilename);
		OutputDebugStringA("Vertex shader loaded OK\n");

		loadPixelShader(psFilename);
		OutputDebugStringA("Pixel shader loaded OK\n");
	}
	catch (...) {
		OutputDebugStringA("ERROR: Shader loading failed!\n");
		throw;
	}

	// Setup matrix buffer (EXACTLY like BoxBlurShader)
	matrixBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	matrixBufferDesc.ByteWidth = sizeof(MatrixBufferType);
	matrixBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	matrixBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	matrixBufferDesc.MiscFlags = 0;
	matrixBufferDesc.StructureByteStride = 0;
	result = renderer->CreateBuffer(&matrixBufferDesc, NULL, &matrixBuffer);
	if (FAILED(result))
	{
		OutputDebugStringA("ERROR: Failed to create matrix buffer!\n");
		throw std::runtime_error("Failed to create matrix buffer");
	}

	// Create sampler with CLAMP mode to avoid edge artifacts
	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.MipLODBias = 0.0f;
	samplerDesc.MaxAnisotropy = 1;
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
	samplerDesc.MinLOD = 0;
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;
	// Add border color for safety
	samplerDesc.BorderColor[0] = 0.0f;
	samplerDesc.BorderColor[1] = 0.0f;
	samplerDesc.BorderColor[2] = 0.0f;
	samplerDesc.BorderColor[3] = 1.0f;
	result = renderer->CreateSamplerState(&samplerDesc, &sampleState);
	if (FAILED(result))
	{
		OutputDebugStringA("ERROR: Failed to create sampler state!\n");
		throw std::runtime_error("Failed to create sampler state");
	}

	// Setup screen size buffer - ensure 16-byte alignment
	screenSizeBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	screenSizeBufferDesc.ByteWidth = sizeof(ScreenSizeBufferType);
	screenSizeBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	screenSizeBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	screenSizeBufferDesc.MiscFlags = 0;
	screenSizeBufferDesc.StructureByteStride = 0;

	// Verify buffer size is 16-byte aligned
	if (sizeof(ScreenSizeBufferType) % 16 != 0)
	{
		OutputDebugStringA("WARNING: ScreenSizeBufferType not 16-byte aligned!\n");
	}

	result = renderer->CreateBuffer(&screenSizeBufferDesc, NULL, &screenSizeBuffer);
	if (FAILED(result))
	{
		OutputDebugStringA("ERROR: Failed to create screen size buffer!\n");
		throw std::runtime_error("Failed to create screen size buffer");
	}

	// Setup kernel buffer - ensure 16-byte alignment
	kernelBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	kernelBufferDesc.ByteWidth = sizeof(KernelBufferType);
	kernelBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	kernelBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	kernelBufferDesc.MiscFlags = 0;
	kernelBufferDesc.StructureByteStride = 0;

	// Verify buffer size is 16-byte aligned
	if (sizeof(KernelBufferType) % 16 != 0)
	{
		OutputDebugStringA("WARNING: KernelBufferType not 16-byte aligned!\n");
	}

	result = renderer->CreateBuffer(&kernelBufferDesc, NULL, &kernelBuffer);
	if (FAILED(result))
	{
		OutputDebugStringA("ERROR: Failed to create kernel buffer!\n");
		throw std::runtime_error("Failed to create kernel buffer");
	}

	OutputDebugStringA("All buffers created successfully!\n");
}

void KernelShader::setKernel(int kernelType)
{
	switch (kernelType)
	{
	case BOX_BLUR:
		currentKernel[0] = 1.0f; currentKernel[1] = 1.0f; currentKernel[2] = 1.0f;
		currentKernel[3] = 1.0f; currentKernel[4] = 1.0f; currentKernel[5] = 1.0f;
		currentKernel[6] = 1.0f; currentKernel[7] = 1.0f; currentKernel[8] = 1.0f;
		currentDivisor = 9.0f;
		break;

	case EDGE_DETECTION:
		currentKernel[0] = -1.0f; currentKernel[1] = -1.0f; currentKernel[2] = -1.0f;
		currentKernel[3] = -1.0f; currentKernel[4] = 8.0f;  currentKernel[5] = -1.0f;
		currentKernel[6] = -1.0f; currentKernel[7] = -1.0f; currentKernel[8] = -1.0f;
		currentDivisor = 1.0f;
		break;

	case SHARPEN:
		currentKernel[0] = 0.0f;  currentKernel[1] = -1.0f; currentKernel[2] = 0.0f;
		currentKernel[3] = -1.0f; currentKernel[4] = 5.0f;  currentKernel[5] = -1.0f;
		currentKernel[6] = 0.0f;  currentKernel[7] = -1.0f; currentKernel[8] = 0.0f;
		currentDivisor = 1.0f;
		break;

	case GAUSSIAN_APPROX:
		currentKernel[0] = 1.0f; currentKernel[1] = 2.0f; currentKernel[2] = 1.0f;
		currentKernel[3] = 2.0f; currentKernel[4] = 4.0f; currentKernel[5] = 2.0f;
		currentKernel[6] = 1.0f; currentKernel[7] = 2.0f; currentKernel[8] = 1.0f;
		currentDivisor = 16.0f;
		break;

	case CUSTOM:
	default:
		// Identity kernel (no change)
		currentKernel[0] = 0.0f; currentKernel[1] = 0.0f; currentKernel[2] = 0.0f;
		currentKernel[3] = 0.0f; currentKernel[4] = 1.0f; currentKernel[5] = 0.0f;
		currentKernel[6] = 0.0f; currentKernel[7] = 0.0f; currentKernel[8] = 0.0f;
		currentDivisor = 1.0f;
		break;
	}

	// Debug output
	char debugMsg[256];
	sprintf_s(debugMsg, "Kernel changed to type %d\n", kernelType);
	OutputDebugStringA(debugMsg);
}

void KernelShader::setShaderParameters(ID3D11DeviceContext* deviceContext, const XMMATRIX& worldMatrix, const XMMATRIX& viewMatrix, const XMMATRIX& projectionMatrix, ID3D11ShaderResourceView* texture, float width, float height)
{
	HRESULT result;
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	MatrixBufferType* dataPtr;

	// CHECK: Make sure we have a valid texture
	if (texture == nullptr)
	{
		OutputDebugStringA("ERROR: Texture is NULL in setShaderParameters!\n");
		return;
	}

	// CHECK: Make sure width and height are valid
	if (width <= 0.0f || height <= 0.0f)
	{
		char debugMsg[256];
		sprintf_s(debugMsg, "ERROR: Invalid screen dimensions: %.2f x %.2f\n", width, height);
		OutputDebugStringA(debugMsg);
		return;
	}

	// Transpose matrices (EXACTLY like BoxBlurShader)
	XMMATRIX tworld = XMMatrixTranspose(worldMatrix);
	XMMATRIX tview = XMMatrixTranspose(viewMatrix);
	XMMATRIX tproj = XMMatrixTranspose(projectionMatrix);

	// Lock matrix buffer (EXACTLY like BoxBlurShader)
	result = deviceContext->Map(matrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if (FAILED(result))
	{
		OutputDebugStringA("ERROR: Failed to map matrix buffer!\n");
		return;
	}
	dataPtr = (MatrixBufferType*)mappedResource.pData;
	dataPtr->world = tworld;
	dataPtr->view = tview;
	dataPtr->projection = tproj;
	deviceContext->Unmap(matrixBuffer, 0);
	deviceContext->VSSetConstantBuffers(0, 1, &matrixBuffer);

	// Lock screen size buffer (EXACTLY like BoxBlurShader)
	ScreenSizeBufferType* screenPtr;
	result = deviceContext->Map(screenSizeBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if (FAILED(result))
	{
		OutputDebugStringA("ERROR: Failed to map screen size buffer!\n");
		return;
	}
	screenPtr = (ScreenSizeBufferType*)mappedResource.pData;
	screenPtr->screenWidth = width;
	screenPtr->screenHeight = height;
	screenPtr->padding = XMFLOAT2(0.0f, 0.0f);
	deviceContext->Unmap(screenSizeBuffer, 0);

	// Set screen size buffer to slot b0
	deviceContext->PSSetConstantBuffers(0, 1, &screenSizeBuffer);

	// Check kernel buffer exists
	if (kernelBuffer == nullptr)
	{
		OutputDebugStringA("ERROR: kernelBuffer is NULL!\n");
		deviceContext->PSSetShaderResources(0, 1, &texture);
		deviceContext->PSSetSamplers(0, 1, &sampleState);
		return;
	}

	// Lock kernel buffer
	KernelBufferType* kernelPtr;
	result = deviceContext->Map(kernelBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	
	if (FAILED(result))
	{
		OutputDebugStringA("ERROR: Failed to map kernel buffer!\n");
		return;
	}

	kernelPtr = (KernelBufferType*)mappedResource.pData;

	kernelPtr->kernelRow0 = XMFLOAT4(currentKernel[0], currentKernel[1], currentKernel[2], 0.0f);
	kernelPtr->kernelRow1 = XMFLOAT4(currentKernel[3], currentKernel[4], currentKernel[5], 0.0f);
	kernelPtr->kernelRow2 = XMFLOAT4(currentKernel[6], currentKernel[7], currentKernel[8], currentDivisor);

	deviceContext->Unmap(kernelBuffer, 0);
	deviceContext->PSSetConstantBuffers(1, 1, &kernelBuffer);

	// Set texture and sampler
	deviceContext->PSSetShaderResources(0, 1, &texture);
	deviceContext->PSSetSamplers(0, 1, &sampleState);

	// Debug: Verify all resources are bound
	ID3D11Buffer* verifyBuffers[2] = { nullptr, nullptr };
	deviceContext->PSGetConstantBuffers(0, 2, verifyBuffers);
	if (verifyBuffers[0] == nullptr || verifyBuffers[1] == nullptr)
	{
		OutputDebugStringA("WARNING: Constant buffers not properly bound!\n");
	}
	// Clean up verification
	if (verifyBuffers[0]) verifyBuffers[0]->Release();
	if (verifyBuffers[1]) verifyBuffers[1]->Release();
}