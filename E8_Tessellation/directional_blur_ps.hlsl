

Texture2D shaderTexture : register(t0);
SamplerState sampleType : register(s0);

cbuffer ScreenSizeBuffer : register(b0)
{
    float screenWidth;
    float screenHeight;
    float2 padding;
};

cbuffer BlurBuffer : register(b1)
{
    float angle;        // Blur angle in radians
    float distance;     // Blur distance in pixels
    int numSamples;     // Number of samples along the line
    float padding2;
};

struct InputType
{
    float4 position : SV_POSITION;
    float2 tex : TEXCOORD0;
};

float4 main(InputType input) : SV_TARGET
{
    float4 color = float4(0.0, 0.0, 0.0, 0.0);
    
    // Convert angle to direction vector
    float2 direction = float2(cos(angle), sin(angle));
    
    // Calculate texel size
    float2 texelSize = float2(1.0 / screenWidth, 1.0 / screenHeight);
    
    // Calculate step size for sampling
    float2 step = direction * texelSize * distance / float(numSamples);
    
    // Sample along the line
    int halfSamples = numSamples / 2;
    for (int i = -halfSamples; i <= halfSamples; i++)
    {
        float2 offset = step * float(i);
        color += shaderTexture.Sample(sampleType, input.tex + offset);
    }
    
    // Average all samples
    color /= float(numSamples + 1);
    
    return color;
}
