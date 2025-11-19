
#ifndef _LAB7APP_H
#define _LAB7APP_H


#include "DXF.h"
#include "BaseApplication.h"
#include "SphereMesh.h"
#include "CubeMesh.h"
#include "PlaneMesh.h"
#include "OrthoMesh.h"
#include "RenderTexture.h"
#include "BoxBlurShader.h"
#include "KernelShader.h"
#include "GaussianBlurShader.h"
#include "SelectiveGreyscaleShader.h"
#include "DirectionalBlurShader.h"
#include "TextureShader.h"
#include "Light.h"
#include "GaussianWeights.h"

class Lab7App : public BaseApplication
{
public:
	Lab7App();
	~Lab7App();
	void init(HINSTANCE hinstance, HWND hwnd, int screenWidth, int screenHeight, Input* in, bool VSYNC, bool FULL_SCREEN);

	bool frame();

protected:
	bool render();
	void gui();
	void renderScene();

private:
	// Scene objects
	SphereMesh* sphere;
	CubeMesh* cube;
	PlaneMesh* plane;
	Light* light;
	GaussianWeights gaussianWeights;  // Use global struct
	OrthoMesh* orthoMesh;           // Full size (1200x675)
	OrthoMesh* halfSizeOrthoMesh;   // Half size (600x337) - ADD THIS
	// Post-processing
	RenderTexture* sceneTexture;
	RenderTexture* downSampledTexture;
	RenderTexture* horizontalBlurTexture;
	RenderTexture* verticalBlurTexture;

	// Shaders
	TextureShader* textureShader;
	BoxBlurShader* boxBlurShader;
	KernelShader* kernelShader;
	GaussianBlurShader* gaussianHorizontalShader;
	GaussianBlurShader* gaussianVerticalShader;
	SelectiveGreyscaleShader* greyscaleShader;
	DirectionalBlurShader* directionalBlurShader;

	// Effect selection
	enum EffectMode {
		EFFECT_NONE,
		EFFECT_BOX_BLUR,
		EFFECT_KERNEL,
		EFFECT_GAUSSIAN,
		EFFECT_GREYSCALE,
		EFFECT_DIRECTIONAL
	};
	EffectMode currentEffect;

	// Kernel settings
	int currentKernelType;
	const char* kernelNames[5] = { "Box Blur", "Edge Detection", "Sharpen", "Gaussian Approx", "Custom" };

	// Gaussian blur settings
	bool useDownSampling;


	// Greyscale settings
	float greyscaleRadius;
	float greyscaleFeather;

	// Directional blur settings
	float blurAngle;
	float blurDistance;
	int blurSamples;
};

#endif