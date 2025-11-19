
Texture2D shaderTexture : register(t0);
SamplerState sampleType : register(s0);

cbuffer ScreenSizeBuffer : register(b0)
{
    float screenWidth;
    float screenHeight;
    float2 padding;
};

cbuffer WeightsBuffer : register(b1)
{
    float weight0;  // Center
    float weight1;  // 1 pixel away
    float weight2;  // 2 pixels away
    float weight3;  // 3 pixels away
    float weight4;  // 4 pixels away
    float3 padding2;
};

struct InputType
{
    float4 position : SV_POSITION;
    float2 tex : TEXCOORD0;
};

float4 main(InputType input) : SV_TARGET
{
    float4 color = float4(0.0, 0.0, 0.0, 0.0);
    
    // Calculate texel size
    float texelSize = 1.0 / screenHeight;
    
    // Sample center pixel
    color = shaderTexture.Sample(sampleType, input.tex) * weight0;
    
    // Sample 4 neighbors on each side (vertical)
    // Neighbor 1 (up and down)
    color += shaderTexture.Sample(sampleType, input.tex + float2(0.0, texelSize * 1.0)) * weight1;
    color += shaderTexture.Sample(sampleType, input.tex - float2(0.0, texelSize * 1.0)) * weight1;
    
    // Neighbor 2
    color += shaderTexture.Sample(sampleType, input.tex + float2(0.0, texelSize * 2.0)) * weight2;
    color += shaderTexture.Sample(sampleType, input.tex - float2(0.0, texelSize * 2.0)) * weight2;
    
    // Neighbor 3
    color += shaderTexture.Sample(sampleType, input.tex + float2(0.0, texelSize * 3.0)) * weight3;
    color += shaderTexture.Sample(sampleType, input.tex - float2(0.0, texelSize * 3.0)) * weight3;
    
    // Neighbor 4
    color += shaderTexture.Sample(sampleType, input.tex + float2(0.0, texelSize * 4.0)) * weight4;
    color += shaderTexture.Sample(sampleType, input.tex - float2(0.0, texelSize * 4.0)) * weight4;
    
    return color;
}
