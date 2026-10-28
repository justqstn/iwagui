cbuffer ScreenBuffer : register(b0)
{
    float4x4 ProjectionMatrix;
};

struct VS_INPUT
{
    float4 position     : POSITION;
    float4 color        : COLOR0;
    float2 uv           : TEXCOORD0;
};

struct PS_INPUT
{
    float4 position     : SV_POSITION;
    float4 color        : COLOR0;
    float2 uv           : TEXCOORD0;
};

sampler LinearSampler : register(s0);
Texture2D Texture : register(t0);

PS_INPUT VS(VS_INPUT input)
{
    PS_INPUT output;
    output.position = mul(ProjectionMatrix, float4(input.position.xy, 0.0f, 1.0f));
    output.color = input.color;
    output.uv = input.uv;
    return output;
}

float4 PS(PS_INPUT input) : SV_Target
{
    return input.color;
}

float4 PSTextured(PS_INPUT input) : SV_Target
{
    return input.color * Texture.Sample(LinearSampler, input.uv);
}

float4 PSText(PS_INPUT input) : SV_Target
{
    float alpha = Texture.Sample(LinearSampler, input.uv);
    return float4(input.color.rgb, input.color.a * alpha);
}
