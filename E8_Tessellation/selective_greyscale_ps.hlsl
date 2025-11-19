
Texture2D shaderTexture : register(t0);
SamplerState sampleType : register(s0);

cbuffer ScreenSizeBuffer : register(b0)
{
    float screenWidth;
    float screenHeight;
    float2 padding;
};

cbuffer MouseBuffer : register(b1)
{
    float2 mousePos;    // Normalized mouse position [0,1]
    float radius;       // Radius of colored region
    float feather;      // Soft edge amount
};

struct InputType
{
    float4 position : SV_POSITION;
    float2 tex : TEXCOORD0;
};

float4 main(InputType input) : SV_TARGET
{
    // Sample the color from texture
    float4 color = shaderTexture.Sample(sampleType, input.tex);
    
    // Calculate distance from mouse cursor
    float2 pixelPos = input.tex;
    float dist = distance(pixelPos, mousePos);
    
    // Create smooth falloff using smoothstep
    // mask = 1.0 inside radius, 0.0 outside radius+feather
    float mask = 1.0 - smoothstep(radius - feather, radius + feather, dist);
    
    // Convert to greyscale using luminance formula
    float grey = dot(color.rgb, float3(0.299, 0.587, 0.114));
    float3 greyColor = float3(grey, grey, grey);
    
    // Blend between greyscale and color based on mask
    float3 finalColor = lerp(greyColor, color.rgb, mask);
    
    return float4(finalColor, color.a);
}
