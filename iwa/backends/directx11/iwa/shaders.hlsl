cbuffer ScreenBuffer : register(b0)
{
    float4x4 ProjectionMatrix;
};

struct VS_INPUT
{
    float2 position         : POSITION;
    float4 color            : COLOR0;
    float2 uv               : TEXCOORD0;
    float4 rect_params      : TEXCOORD1;
    float4 shadow_extra     : TEXCOORD2;
};

struct PS_INPUT
{
    float4 position         : SV_POSITION;
    float4 color            : COLOR0;
    float2 uv               : TEXCOORD0;
    float4 rect_params      : TEXCOORD1;
    float4 shadow_extra     : TEXCOORD2;
};

sampler LinearSampler : register(s0);
Texture2D Texture : register(t0);

float4 CornerRadii(float rounding, float flags)
{
    uint bits = (uint)(flags + 0.5);
    float tl = (bits & 1) ? rounding : 0.0;
    float tr = (bits & 2) ? rounding : 0.0;
    float br = (bits & 4) ? rounding : 0.0;
    float bl = (bits & 8) ? rounding : 0.0;
    return float4(br, tr, bl, tl);
}

float sdRoundBox(float2 p, float2 half_size, float4 r)
{
    r.xy = (p.x > 0.0) ? r.xy : r.zw;
    r.x = (p.y > 0.0) ? r.x : r.y;
    r.x = min(r.x, min(half_size.x, half_size.y));
    float2 q = abs(p) - half_size + r.x;
    return min(max(q.x, q.y), 0.0) + length(max(q, 0.0)) - r.x;
}

float SoftShadowAlpha(float dist, float shadow_size, float thickness)
{
    const float aa = 0.75;
    float radius = max(shadow_size, aa);

    if (shadow_size <= 1e-6)
        return 0.0;
    if (thickness <= 1e-6)
        return 1.0 - smoothstep(radius - aa, radius + aa, dist);

    float soft = clamp(thickness, aa, radius);
    float t = saturate(dist / radius);
    float soft_k = saturate(soft / radius);
    float alpha = pow(1.0 - t, lerp(2.5, 1.2, soft_k));
    alpha *= 1.0 - smoothstep(radius - aa, radius + aa, dist);
    return saturate(alpha);
}

PS_INPUT VS(VS_INPUT input)
{
    PS_INPUT output;
    output.position = mul(ProjectionMatrix, float4(input.position.xy, 0.0f, 1.0f));
    output.color = input.color;
    output.uv = input.uv;
    output.rect_params = input.rect_params;
    output.shadow_extra = input.shadow_extra;
    return output;
}

float4 PS(PS_INPUT input) : SV_Target
{
    float2 size = max(input.rect_params.xy, float2(1e-6, 1e-6));
    float rounding = input.rect_params.z;
    float4 radii = CornerRadii(rounding, input.shadow_extra.w);
    float2 p = (input.uv - 0.5) * size;
    float sdf = sdRoundBox(p, size * 0.5, radii);
    float aa = max(fwidth(sdf), 1e-3);
    float alpha = 1.0 - smoothstep(-aa, aa, sdf);
    return float4(input.color.rgb, input.color.a * alpha);
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

float4 PSTextSDF(PS_INPUT input) : SV_Target
{
    float dist = Texture.Sample(LinearSampler, input.uv).r;
    float onedge = 180.0 / 255.0;
    float width = fwidth(dist);
    float alpha = smoothstep(onedge - width, onedge + width, dist);
    return float4(input.color.rgb, input.color.a * alpha);
}

float4 PSShadow(PS_INPUT input) : SV_Target
{
    float2 outer_size = max(input.rect_params.xy, float2(1e-6, 1e-6));
    float rounding = input.rect_params.z;
    float shadow_size = max(input.rect_params.w, 0.0);
    float thickness = max(input.shadow_extra.x, 0.0);
    float4 radii = CornerRadii(rounding, input.shadow_extra.w);

    const float aa_pad = 1.5;
    float2 half_outer = outer_size * 0.5;
    float2 half_orig = max(half_outer - (shadow_size + aa_pad), float2(0.0, 0.0));

    float2 p = (input.uv - 0.5) * outer_size;
    float sdf = sdRoundBox(p, half_orig, radii);
    float dist = max(sdf, 0.0);

    float alpha = SoftShadowAlpha(dist, shadow_size, thickness);
    return float4(input.color.rgb, input.color.a * alpha);
}

float4 PSInnerShadow(PS_INPUT input) : SV_Target
{
    float2 size = max(input.rect_params.xy, float2(1e-6, 1e-6));
    float rounding = input.rect_params.z;
    float shadow_size = max(input.rect_params.w, 0.0);
    float thickness = max(input.shadow_extra.x, 0.0);
    float2 offset = input.shadow_extra.yz;
    float4 radii = CornerRadii(rounding, input.shadow_extra.w);
    float2 half_size = size * 0.5;
    float2 p = (input.uv - 0.5) * size;
    float sdf_shape = sdRoundBox(p, half_size, radii);
    float sdf_shadow = sdRoundBox(p - offset, half_size, radii);

    const float aa = 0.75;
    float shape = 1.0 - smoothstep(-aa, aa, sdf_shape);
    float dist = max(-sdf_shadow, 0.0);

    float alpha = SoftShadowAlpha(dist, shadow_size, thickness) * shape;
    return float4(input.color.rgb, input.color.a * alpha);
}

float4 PSOutline(PS_INPUT input) : SV_Target
{
    float2 mesh_size = max(input.rect_params.xy, float2(1e-6, 1e-6));
    float rounding = input.rect_params.z;
    float thickness = max(input.shadow_extra.x, 0.0);
    float2 shape_size = max(input.shadow_extra.yz, float2(1e-6, 1e-6));
    float4 radii = CornerRadii(rounding, input.shadow_extra.w);

    float2 p = (input.uv - 0.5) * mesh_size;
    float sdf = sdRoundBox(p, shape_size * 0.5, radii);
    float half_t = thickness * 0.5;
    float aa = max(fwidth(sdf), 1e-3);
    float alpha = 1.0 - smoothstep(half_t - aa, half_t + aa, abs(sdf));
    return float4(input.color.rgb, input.color.a * alpha);
}

float4 PSCircle(PS_INPUT input) : SV_Target
{
    return input.color;
}

float4 PSCircleTextured(PS_INPUT input) : SV_Target
{
    float shape_r = max(input.rect_params.x, 0.0);
    float mesh_r = max(input.rect_params.y, 1e-6);

    float2 p = (input.uv - 0.5) * (2.0 * mesh_r);
    float dist = length(p);
    float aa = max(fwidth(dist), 1e-3);
    float mask = 1.0 - smoothstep(shape_r - aa, shape_r + aa, dist);

    float4 tex = Texture.Sample(LinearSampler, input.uv);
    return float4(tex.rgb * input.color.rgb, tex.a * input.color.a * mask);
}

float4 PSCircleShadow(PS_INPUT input) : SV_Target
{
    float shape_r = max(input.rect_params.x, 0.0);
    float mesh_r = max(input.rect_params.y, 1e-6);
    float shadow_size = max(input.rect_params.w, 0.0);
    float thickness = max(input.shadow_extra.x, 0.0);

    float2 p = (input.uv - 0.5) * (2.0 * mesh_r);
    float dist = max(length(p) - shape_r, 0.0);

    float alpha = SoftShadowAlpha(dist, shadow_size, thickness);
    return float4(input.color.rgb, input.color.a * alpha);
}

float sdCapsuleX(float2 p, float half_len, float r)
{
    float2 a = float2(-half_len, 0.0);
    float2 b = float2(half_len, 0.0);
    float2 pa = p - a;
    float2 ba = b - a;
    float h = saturate(dot(pa, ba) / dot(ba, ba));
    return length(pa - ba * h) - r;
}

float4 PSLineShadow(PS_INPUT input) : SV_Target
{
    float core_r = max(input.rect_params.x, 0.0);
    float mesh_half_perp = max(input.rect_params.y, 1e-6);
    float half_len = max(input.rect_params.z, 0.0);
    float shadow_size = max(input.rect_params.w, 0.0);
    float thickness = max(input.shadow_extra.x, 0.0);
    float mesh_half_along = max(input.shadow_extra.y, 1e-6);

    float2 p = (input.uv - 0.5) * float2(2.0 * mesh_half_along, 2.0 * mesh_half_perp);
    float dist = max(sdCapsuleX(p, half_len, core_r), 0.0);

    float alpha = SoftShadowAlpha(dist, shadow_size, thickness);
    return float4(input.color.rgb, input.color.a * alpha);
}

float4 PSCircleInnerShadow(PS_INPUT input) : SV_Target
{
    float shape_r = max(input.rect_params.x, 0.0);
    float mesh_r = max(input.rect_params.y, 1e-6);
    float shadow_size = max(input.rect_params.w, 0.0);
    float thickness = max(input.shadow_extra.x, 0.0);
    float2 offset = input.shadow_extra.yz;

    float2 p = (input.uv - 0.5) * (2.0 * mesh_r);
    float len_shape = length(p);
    float len_shadow = length(p - offset);

    const float aa = 0.75;
    float shape = 1.0 - smoothstep(shape_r - aa, shape_r + aa, len_shape);
    float dist = max(shape_r - len_shadow, 0.0);

    float alpha = SoftShadowAlpha(dist, shadow_size, thickness) * shape;
    return float4(input.color.rgb, input.color.a * alpha);
}
