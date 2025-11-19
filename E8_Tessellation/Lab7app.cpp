
#include "Lab7App.h"

Lab7App::Lab7App()
{
	sphere = nullptr;
	cube = nullptr;
	plane = nullptr;
	light = nullptr;
	orthoMesh = nullptr;
	sceneTexture = nullptr;
	downSampledTexture = nullptr;
	horizontalBlurTexture = nullptr;
	verticalBlurTexture = nullptr;
	textureShader = nullptr;
	boxBlurShader = nullptr;
	kernelShader = nullptr;
	gaussianHorizontalShader = nullptr;
	gaussianVerticalShader = nullptr;
	greyscaleShader = nullptr;
	directionalBlurShader = nullptr;

	currentEffect = EFFECT_BOX_BLUR;
	currentKernelType = 0;  // Start with Box Blur (index 0)

	useDownSampling = true;

	// Initialize Gaussian weights
	gaussianWeights.weight0 = 0.382928f;
	gaussianWeights.weight1 = 0.241732f;
	gaussianWeights.weight2 = 0.060598f;
	gaussianWeights.weight3 = 0.005977f;
	gaussianWeights.weight4 = 0.000229f;

	// Greyscale settings
	greyscaleRadius = 0.2f;
	greyscaleFeather = 0.05f;

	// Directional blur settings
	blurAngle = 0.0f;
	blurDistance = 10.0f;
	blurSamples = 11;
}

void Lab7App::init(HINSTANCE hinstance, HWND hwnd, int screenWidth, int screenHeight, Input* in, bool VSYNC, bool FULL_SCREEN)
{
	// Call super/parent init function 
	BaseApplication::init(hinstance, hwnd, screenWidth, screenHeight, in, VSYNC, FULL_SCREEN);

	// Create scene geometry
	sphere = new SphereMesh(renderer->getDevice(), renderer->getDeviceContext(), 20);
	cube = new CubeMesh(renderer->getDevice(), renderer->getDeviceContext());
	plane = new PlaneMesh(renderer->getDevice(), renderer->getDeviceContext(), 20);

	// Create ortho mesh for post-processing
	orthoMesh = new OrthoMesh(renderer->getDevice(), renderer->getDeviceContext(), screenWidth, screenHeight);

	// Create render textures
	sceneTexture = new RenderTexture(renderer->getDevice(), screenWidth, screenHeight, SCREEN_NEAR, SCREEN_DEPTH);
	downSampledTexture = new RenderTexture(renderer->getDevice(), screenWidth / 2, screenHeight / 2, SCREEN_NEAR, SCREEN_DEPTH);
	horizontalBlurTexture = new RenderTexture(renderer->getDevice(), screenWidth / 2, screenHeight / 2, SCREEN_NEAR, SCREEN_DEPTH);
	verticalBlurTexture = new RenderTexture(renderer->getDevice(), screenWidth / 2, screenHeight / 2, SCREEN_NEAR, SCREEN_DEPTH);

	// Create shaders
	textureShader = new TextureShader(renderer->getDevice(), hwnd);
	boxBlurShader = new BoxBlurShader(renderer->getDevice(), hwnd);
	kernelShader = new KernelShader(renderer->getDevice(), hwnd);
	OutputDebugStringA("=== Buffer Size Check ===\n");
	char buffer[256];
	sprintf_s(buffer, "ScreenSizeBufferType size: %zu (should be 16)\n", sizeof(float) * 2 + sizeof(XMFLOAT2));
	OutputDebugStringA(buffer);
	sprintf_s(buffer, "KernelBufferType size: %zu (should be 48)\n", sizeof(float) * 9 + sizeof(float) + sizeof(XMFLOAT2));
	OutputDebugStringA(buffer);
	gaussianHorizontalShader = new GaussianBlurShader(renderer->getDevice(), hwnd, true);
	gaussianVerticalShader = new GaussianBlurShader(renderer->getDevice(), hwnd, false);
	greyscaleShader = new SelectiveGreyscaleShader(renderer->getDevice(), hwnd);
	directionalBlurShader = new DirectionalBlurShader(renderer->getDevice(), hwnd);

	// Create light
	light = new Light();
	light->setDiffuseColour(1.0f, 1.0f, 1.0f, 1.0f);
	light->setAmbientColour(0.2f, 0.2f, 0.2f, 1.0f);
	light->setDirection(0.5f, -0.7f, 0.5f);
	light->setSpecularColour(1.0f, 1.0f, 1.0f, 1.0f);
	light->setSpecularPower(32.0f);
	orthoMesh = new OrthoMesh(renderer->getDevice(), renderer->getDeviceContext(), screenWidth, screenHeight);
	halfSizeOrthoMesh = new OrthoMesh(renderer->getDevice(), renderer->getDeviceContext(), screenWidth / 2, screenHeight / 2);
}

Lab7App::~Lab7App()
{
	// Run base application deconstructor
	BaseApplication::~BaseApplication();

	// Release objects
	if (sphere)
	{
		delete sphere;
		sphere = 0;
	}
	if (cube)
	{
		delete cube;
		cube = 0;
	}
	if (plane)
	{
		delete plane;
		plane = 0;
	}
	if (light)
	{
		delete light;
		light = 0;
	}
	if (orthoMesh)
	{
		delete orthoMesh;
		orthoMesh = 0;
	}
	if (sceneTexture)
	{
		delete sceneTexture;
		sceneTexture = 0;
	}
	if (downSampledTexture)
	{
		delete downSampledTexture;
		downSampledTexture = 0;
	}
	if (horizontalBlurTexture)
	{
		delete horizontalBlurTexture;
		horizontalBlurTexture = 0;
	}
	if (verticalBlurTexture)
	{
		delete verticalBlurTexture;
		verticalBlurTexture = 0;
	}
	if (textureShader)
	{
		delete textureShader;
		textureShader = 0;
	}
	if (boxBlurShader)
	{
		delete boxBlurShader;
		boxBlurShader = 0;
	}
	if (kernelShader)
	{
		delete kernelShader;
		kernelShader = 0;
	}
	if (gaussianHorizontalShader)
	{
		delete gaussianHorizontalShader;
		gaussianHorizontalShader = 0;
	}
	if (gaussianVerticalShader)
	{
		delete gaussianVerticalShader;
		gaussianVerticalShader = 0;
	}
	if (greyscaleShader)
	{
		delete greyscaleShader;
		greyscaleShader = 0;
	}
	if (directionalBlurShader)
	{
		delete directionalBlurShader;
		directionalBlurShader = 0;
	}
	if (orthoMesh)
	{
		delete orthoMesh;
		orthoMesh = 0;
	}
	if (halfSizeOrthoMesh)
	{
		delete halfSizeOrthoMesh;
		halfSizeOrthoMesh = 0;
	}
}

bool Lab7App::frame()
{
	bool result;

	result = BaseApplication::frame();
	if (!result)
	{
		return false;
	}

	// Render the graphics.
	result = render();
	if (!result)
	{
		return false;
	}

	return true;
}

void Lab7App::renderScene()
{
	XMMATRIX worldMatrix, viewMatrix, projectionMatrix;

	// Generate the view matrix based on the camera's position.
	camera->update();

	// Get matrices
	worldMatrix = renderer->getWorldMatrix();
	viewMatrix = camera->getViewMatrix();
	projectionMatrix = renderer->getProjectionMatrix();

	// IMPORTANT: Create a default texture if one doesn't exist
	ID3D11ShaderResourceView* defaultTex = textureMgr->getTexture(L"default");

	// If no default texture, we need basic lighting at minimum
	// Render sphere
	worldMatrix = XMMatrixTranslation(-3.0f, 0.0f, 5.0f);
	sphere->sendData(renderer->getDeviceContext());
	textureShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, viewMatrix, projectionMatrix,
		defaultTex, light);
	textureShader->render(renderer->getDeviceContext(), sphere->getIndexCount());

	// Render cube
	worldMatrix = XMMatrixTranslation(3.0f, 0.0f, 5.0f);
	cube->sendData(renderer->getDeviceContext());
	textureShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, viewMatrix, projectionMatrix,
		defaultTex, light);
	textureShader->render(renderer->getDeviceContext(), cube->getIndexCount());

	// Render plane
	worldMatrix = XMMatrixTranslation(0.0f, -2.0f, 0.0f);
	plane->sendData(renderer->getDeviceContext());
	textureShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, viewMatrix, projectionMatrix,
		defaultTex, light);
	textureShader->render(renderer->getDeviceContext(), plane->getIndexCount());
}

bool Lab7App::render()
{
	XMMATRIX worldMatrix, baseViewMatrix, orthoMatrix;
	camera->update();

	// PASS 1: Render scene to texture
	sceneTexture->setRenderTarget(renderer->getDeviceContext());
	sceneTexture->clearRenderTarget(renderer->getDeviceContext(), 0.39f, 0.58f, 0.92f, 1.0f);

	renderScene();

	// Setup ortho matrices for post-processing
	worldMatrix = renderer->getWorldMatrix();
	baseViewMatrix = camera->getOrthoViewMatrix();
	orthoMatrix = renderer->getOrthoMatrix();

	renderer->setZBuffer(false);

	//  Apply post-processing effect
	switch (currentEffect)
	{
	case EFFECT_NONE:
		// Just render the scene texture to screen
		renderer->setBackBufferRenderTarget();
		renderer->beginScene(0.0f, 0.0f, 0.0f, 1.0f);
		orthoMesh->sendData(renderer->getDeviceContext());
		textureShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, baseViewMatrix, orthoMatrix,
			sceneTexture->getShaderResourceView(), nullptr);
		textureShader->render(renderer->getDeviceContext(), orthoMesh->getIndexCount());
		break;

	case EFFECT_BOX_BLUR:
		renderer->setBackBufferRenderTarget();
		renderer->beginScene(0.0f, 0.0f, 0.0f, 1.0f);
		orthoMesh->sendData(renderer->getDeviceContext());
		boxBlurShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, baseViewMatrix, orthoMatrix,
			sceneTexture->getShaderResourceView(), (float)sWidth, (float)sHeight);
		boxBlurShader->render(renderer->getDeviceContext(), orthoMesh->getIndexCount());
		break;

	case EFFECT_KERNEL:
	{  
		
		ID3D11RenderTargetView* nullRTV = nullptr;
		renderer->getDeviceContext()->OMSetRenderTargets(1, &nullRTV, nullptr);

		// Then set back buffer as render target
		renderer->setBackBufferRenderTarget();
		renderer->resetViewport();
		renderer->beginScene(0.0f, 0.0f, 0.0f, 1.0f);

		// NOW it's safe to use sceneTexture as a shader resource
		kernelShader->setKernel(currentKernelType);
		orthoMesh->sendData(renderer->getDeviceContext());
		kernelShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, baseViewMatrix, orthoMatrix,
			sceneTexture->getShaderResourceView(), (float)sWidth, (float)sHeight);
		kernelShader->render(renderer->getDeviceContext(), orthoMesh->getIndexCount());
		break;
	}  


	case EFFECT_GAUSSIAN:
		if (useDownSampling)
		{
			// Down sample
			downSampledTexture->setRenderTarget(renderer->getDeviceContext());
			downSampledTexture->clearRenderTarget(renderer->getDeviceContext(), 0.0f, 0.0f, 0.0f, 1.0f);

			OrthoMesh* halfOrtho = new OrthoMesh(renderer->getDevice(), renderer->getDeviceContext(),
				downSampledTexture->getTextureWidth(), downSampledTexture->getTextureHeight());

			halfOrtho->sendData(renderer->getDeviceContext());
			textureShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, baseViewMatrix,
				downSampledTexture->getOrthoMatrix(), sceneTexture->getShaderResourceView(), nullptr);
			textureShader->render(renderer->getDeviceContext(), halfOrtho->getIndexCount());

			// Horizontal blur
			horizontalBlurTexture->setRenderTarget(renderer->getDeviceContext());
			horizontalBlurTexture->clearRenderTarget(renderer->getDeviceContext(), 0.0f, 0.0f, 0.0f, 1.0f);

			halfOrtho->sendData(renderer->getDeviceContext());
			gaussianHorizontalShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, baseViewMatrix,
				horizontalBlurTexture->getOrthoMatrix(), downSampledTexture->getShaderResourceView(),
				(float)downSampledTexture->getTextureWidth(),
				(float)downSampledTexture->getTextureHeight(), gaussianWeights);
			gaussianHorizontalShader->render(renderer->getDeviceContext(), halfOrtho->getIndexCount());

			// Vertical blur
			verticalBlurTexture->setRenderTarget(renderer->getDeviceContext());
			verticalBlurTexture->clearRenderTarget(renderer->getDeviceContext(), 0.0f, 0.0f, 0.0f, 1.0f);

			halfOrtho->sendData(renderer->getDeviceContext());
			gaussianVerticalShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, baseViewMatrix,
				verticalBlurTexture->getOrthoMatrix(), horizontalBlurTexture->getShaderResourceView(),
				(float)downSampledTexture->getTextureWidth(),
				(float)downSampledTexture->getTextureHeight(), gaussianWeights);
			gaussianVerticalShader->render(renderer->getDeviceContext(), halfOrtho->getIndexCount());

			delete halfOrtho;

			// CRITICAL: Up sample to full screen with VIEWPORT RESET
			renderer->setBackBufferRenderTarget();
			renderer->resetViewport();  // ← ADD THIS LINE
			renderer->beginScene(0.0f, 0.0f, 0.0f, 1.0f);

			orthoMesh->sendData(renderer->getDeviceContext());
			textureShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, baseViewMatrix,
				orthoMatrix, verticalBlurTexture->getShaderResourceView(), nullptr);
			textureShader->render(renderer->getDeviceContext(), orthoMesh->getIndexCount());
		}
		else
		{
			// No downsampling
			horizontalBlurTexture->setRenderTarget(renderer->getDeviceContext());
			horizontalBlurTexture->clearRenderTarget(renderer->getDeviceContext(), 0.0f, 0.0f, 0.0f, 1.0f);
			orthoMesh->sendData(renderer->getDeviceContext());
			gaussianHorizontalShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, baseViewMatrix,
				horizontalBlurTexture->getOrthoMatrix(), sceneTexture->getShaderResourceView(),
				(float)sWidth, (float)sHeight, gaussianWeights);
			gaussianHorizontalShader->render(renderer->getDeviceContext(), orthoMesh->getIndexCount());

			// Vertical blur to screen
			renderer->setBackBufferRenderTarget();
			renderer->resetViewport();  // ← ADD THIS LINE TOO
			renderer->beginScene(0.0f, 0.0f, 0.0f, 1.0f);
			orthoMesh->sendData(renderer->getDeviceContext());
			gaussianVerticalShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, baseViewMatrix,
				orthoMatrix, horizontalBlurTexture->getShaderResourceView(),
				(float)sWidth, (float)sHeight, gaussianWeights);
			gaussianVerticalShader->render(renderer->getDeviceContext(), orthoMesh->getIndexCount());
		}
		break;

	case EFFECT_GREYSCALE:
	{
		// Get mouse position
		POINT mousePos;
		GetCursorPos(&mousePos);
		ScreenToClient(wnd, &mousePos);
		float normalizedX = (float)mousePos.x / sWidth;
		float normalizedY = (float)mousePos.y / sHeight;

		renderer->setBackBufferRenderTarget();
		renderer->beginScene(0.0f, 0.0f, 0.0f, 1.0f);
		orthoMesh->sendData(renderer->getDeviceContext());
		greyscaleShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, baseViewMatrix, orthoMatrix,
			sceneTexture->getShaderResourceView(), (float)sWidth, (float)sHeight,
			normalizedX, normalizedY, greyscaleRadius, greyscaleFeather);
		greyscaleShader->render(renderer->getDeviceContext(), orthoMesh->getIndexCount());
		break;
	}

	case EFFECT_DIRECTIONAL:
		renderer->setBackBufferRenderTarget();
		renderer->beginScene(0.0f, 0.0f, 0.0f, 1.0f);
		orthoMesh->sendData(renderer->getDeviceContext());
		directionalBlurShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, baseViewMatrix, orthoMatrix,
			sceneTexture->getShaderResourceView(), (float)sWidth, (float)sHeight,
			blurAngle, blurDistance, blurSamples);
		directionalBlurShader->render(renderer->getDeviceContext(), orthoMesh->getIndexCount());
		break;
	}

	renderer->setZBuffer(true);

	// Render GUI
	gui();

	// Present the rendered scene to the screen.
	renderer->endScene();

	return true;
}

void Lab7App::gui()
{
	// Force turn off unnecessary shader stages.
	renderer->getDeviceContext()->GSSetShader(NULL, NULL, 0);
	renderer->getDeviceContext()->HSSetShader(NULL, NULL, 0);
	renderer->getDeviceContext()->DSSetShader(NULL, NULL, 0);

	// Build UI
	ImGui::Text("FPS: %.2f", timer->getFPS());
	ImGui::Checkbox("Wireframe mode", &wireframeToggle);
	ImGui::Separator();

	// Effect selection
	ImGui::Text("Post-Processing Effects:");
	if (ImGui::RadioButton("None", currentEffect == EFFECT_NONE)) currentEffect = EFFECT_NONE;
	if (ImGui::RadioButton("Box Blur ", currentEffect == EFFECT_BOX_BLUR)) currentEffect = EFFECT_BOX_BLUR;
	if (ImGui::RadioButton("Kernel Effects ", currentEffect == EFFECT_KERNEL)) currentEffect = EFFECT_KERNEL;
	if (ImGui::RadioButton("Gaussian Blur ", currentEffect == EFFECT_GAUSSIAN)) currentEffect = EFFECT_GAUSSIAN;
	if (ImGui::RadioButton("Selective Greyscale ", currentEffect == EFFECT_GREYSCALE)) currentEffect = EFFECT_GREYSCALE;
	if (ImGui::RadioButton("Directional Blur ", currentEffect == EFFECT_DIRECTIONAL)) currentEffect = EFFECT_DIRECTIONAL;

	ImGui::Separator();

	// Effect-specific settings
	if (currentEffect == EFFECT_KERNEL)
	{
		ImGui::Text("Kernel Type:");
		if (ImGui::BeginCombo("##kernel", kernelNames[currentKernelType]))
		{
			for (int i = 0; i < 5; i++)
			{
				bool isSelected = (currentKernelType == i);
				if (ImGui::Selectable(kernelNames[i], isSelected))
					currentKernelType = i;
				if (isSelected)
					ImGui::SetItemDefaultFocus();
			}
			ImGui::EndCombo();
		}
	}
	else if (currentEffect == EFFECT_GAUSSIAN)
	{
		ImGui::Checkbox("Use Down/Up Sampling", &useDownSampling);
		ImGui::Text("Gaussian Weights:");
		ImGui::SliderFloat("Weight 0", &gaussianWeights.weight0, 0.0f, 1.0f);
		ImGui::SliderFloat("Weight 1", &gaussianWeights.weight1, 0.0f, 1.0f);
		ImGui::SliderFloat("Weight 2", &gaussianWeights.weight2, 0.0f, 1.0f);
		ImGui::SliderFloat("Weight 3", &gaussianWeights.weight3, 0.0f, 1.0f);
		ImGui::SliderFloat("Weight 4", &gaussianWeights.weight4, 0.0f, 1.0f);

		float total = gaussianWeights.weight0 + 2.0f * (gaussianWeights.weight1 + gaussianWeights.weight2 +
			gaussianWeights.weight3 + gaussianWeights.weight4);
		ImGui::Text("Total: %.4f (should be 1.0)", total);

		if (ImGui::Button("Normalize Weights"))
		{
			float sum = gaussianWeights.weight0 + 2.0f * (gaussianWeights.weight1 + gaussianWeights.weight2 +
				gaussianWeights.weight3 + gaussianWeights.weight4);
			if (sum > 0.0f)
			{
				gaussianWeights.weight0 /= sum;
				gaussianWeights.weight1 /= sum;
				gaussianWeights.weight2 /= sum;
				gaussianWeights.weight3 /= sum;
				gaussianWeights.weight4 /= sum;
			}
		}

		if (ImGui::Button("Reset to Default"))
		{
			gaussianWeights.weight0 = 0.382928f;
			gaussianWeights.weight1 = 0.241732f;
			gaussianWeights.weight2 = 0.060598f;
			gaussianWeights.weight3 = 0.005977f;
			gaussianWeights.weight4 = 0.000229f;
		}
	}
	else if (currentEffect == EFFECT_GREYSCALE)
	{
		ImGui::Text("Greyscale Settings:");
		ImGui::SliderFloat("Radius", &greyscaleRadius, 0.05f, 0.5f);
		ImGui::SliderFloat("Feather", &greyscaleFeather, 0.01f, 0.2f);
		ImGui::Text("Move mouse to see colored region");
	}
	else if (currentEffect == EFFECT_DIRECTIONAL)
	{
		ImGui::Text("Directional Blur Settings:");
		ImGui::SliderAngle("Angle", &blurAngle, 0.0f, 360.0f);
		ImGui::SliderFloat("Distance", &blurDistance, 0.0f, 50.0f);
		ImGui::SliderInt("Samples", &blurSamples, 3, 21);
	}

	// Render UI
	ImGui::Render();
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
}