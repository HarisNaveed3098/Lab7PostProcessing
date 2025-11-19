Texture2D shaderTexture : register(t0);
SamplerState sampleType : register(s0);

cbuffer ScreenSizeBuffer : register(b0)
{
    float screenWidth;
    float screenHeight;
    float2 padding;
};

cbuffer KernelBuffer : register(b1)
{
    float4 kernelRow0; // .x = kernel[0], .y = kernel[1], .z = kernel[2]
    float4 kernelRow1; // .x = kernel[3], .y = kernel[4], .z = kernel[5]
    float4 kernelRow2; // .x = kernel[6], .y = kernel[7], .z = kernel[8], .w = divisor
};

struct InputType
{
    float4 position : SV_POSITION;
    float2 tex : TEXCOORD0;
};

float4 main(InputType input) : SV_TARGET
{
    float2 texelSize = float2(1.0 / screenWidth, 1.0 / screenHeight);
    float4 color = float4(0.0, 0.0, 0.0, 0.0);
    
    // Apply kernel using the float4 values
    color += shaderTexture.Sample(sampleType, input.tex + float2(-texelSize.x, -texelSize.y)) * kernelRow0.x;
    color += shaderTexture.Sample(sampleType, input.tex + float2(0.0, -texelSize.y)) * kernelRow0.y;
    color += shaderTexture.Sample(sampleType, input.tex + float2(texelSize.x, -texelSize.y)) * kernelRow0.z;
    
    color += shaderTexture.Sample(sampleType, input.tex + float2(-texelSize.x, 0.0)) * kernelRow1.x;
    color += shaderTexture.Sample(sampleType, input.tex + float2(0.0, 0.0)) * kernelRow1.y;
    color += shaderTexture.Sample(sampleType, input.tex + float2(texelSize.x, 0.0)) * kernelRow1.z;
    
    color += shaderTexture.Sample(sampleType, input.tex + float2(-texelSize.x, texelSize.y)) * kernelRow2.x;
    color += shaderTexture.Sample(sampleType, input.tex + float2(0.0, texelSize.y)) * kernelRow2.y;
    color += shaderTexture.Sample(sampleType, input.tex + float2(texelSize.x, texelSize.y)) * kernelRow2.z;
    
    // Apply divisor
    float divisor = kernelRow2.w;
    if (divisor > 0.0)
        color /= divisor;
    
    color.a = 1.0;
    return saturate(color);
}