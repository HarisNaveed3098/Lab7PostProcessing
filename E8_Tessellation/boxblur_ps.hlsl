

Texture2D shaderTexture : register(t0);
SamplerState sampleType : register(s0);

cbuffer ScreenSizeBuffer : register(b0)
{
    float screenWidth;
    float screenHeight;
    float2 padding;
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
    float2 texelSize = float2(1.0 / screenWidth, 1.0 / screenHeight);
    
    // Sample 3x3 grid (9 samples total)
    for (int x = -1; x <= 1; x++)
    {
        for (int y = -1; y <= 1; y++)
        {
            float2 offset = float2(x, y) * texelSize;
            color += shaderTexture.Sample(sampleType, input.tex + offset);
        }
    }
    
    // Average the 9 samples
    color /= 9.0;
    
    return color;
}
