struct MATERIAL
{
    float4 m_cAmbient;
    float4 m_cAlbedo;
    float4 m_cSpecular; //a = power
    float4 m_cEmissive;
};

cbuffer cbCameraInfo : register(b0)
{
    matrix gmtxView : packoffset(c0);
    matrix gmtxProjection : packoffset(c4);
    float3 gvCameraPosition : packoffset(c8);
};

cbuffer cbGameObjectInfo : register(b1)
{
    matrix gmtxGameObject : packoffset(c0);
    MATERIAL gMaterial : packoffset(c4);
    uint gnTexturesMask : packoffset(c8);
};

Texture2D AlbedoTexture : register(t0);
Texture2D SpecularTexture : register(t1);
Texture2D NormalTexture : register(t2);
Texture2D MetallicTexture : register(t3);
Texture2D EmissionTexture : register(t4);

Texture2D DFTextureTexture : register(t5);
Texture2D DFNormalTexture : register(t6);
Texture2D DFzDepthTexture : register(t7);

SamplerState gssWrap : register(s0);

#define MATERIAL_ALBEDO_MAP			0x01
#define MATERIAL_SPECULAR_MAP		0x02
#define MATERIAL_NORMAL_MAP			0x04
#define MATERIAL_METALLIC_MAP		0x08
#define MATERIAL_EMISSION_MAP		0x10
#define MATERIAL_DETAIL_ALBEDO_MAP	0x20
#define MATERIAL_DETAIL_NORMAL_MAP	0x40

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//

// ���� ���̴��� �����Ѵ�.
float4 VSMain(uint nVertexID : sv_VertexID) : SV_POSITION
{
    float4 output;

// ������Ƽ��(�ﰢ��)�� �����ϴ� ������ �ε���(SV_VertexID)�� ���� ������ ��ȯ�Ѵ�.
// ������ ��ġ ��ǥ�� ��ȯ�� �� ��ǥ(SV_POSITION)�̴�. ��, ������ǥ���� ��ǥ�̴�.
    if (nVertexID == 0)
    {
        output = float4(0.0, 0.5, 0.5, 1.0);
    }
    else if (nVertexID == 1)
    {
        output = float4(0.5, -0.5, 0.5, 1.0);
    }
    else if (nVertexID == 2)
    {
        output = float4(-0.5, -0.5, 0.5, 1.0);
    }

    return output;
}

// �ȼ� ���̴��� �����Ѵ�.
float4 PSMain(float4 input : SV_POSITION) : SV_TARGET
{
	// ������Ƽ���� ��� �ȼ��� ������ ��������� ��ȯ�Ѵ�.
    return float4(1.0f, 1.0f, 0.0f, 1.0f); // Yellow
}

struct VS_STANDARD_INPUT
{
    float3 position : POSITION;
    float2 uv : TEXCOORD;
    float3 normal : NORMAL;
    float3 tangent : TANGENT;
    float3 bitangent : BITANGENT;
};

struct VS_INSTANCE_STANDARD_INPUT
{
    float3 position : POSITION;
    float2 uv : TEXCOORD;
    float3 normal : NORMAL;
    float3 tangent : TANGENT;
    float3 bitangent : BITANGENT;
    float4x4 ins_transform : INSMATRIX;
};


struct VS_STANDARD_OUTPUT
{
    float4 position : SV_POSITION;
    float3 positionW : POSITION;
    float3 normalW : NORMAL;
    float3 tangentW : TANGENT;
    float3 bitangentW : BITANGENT;
    float2 uv : TEXCOORD;
};


VS_STANDARD_OUTPUT VSStandard(VS_STANDARD_INPUT input)
{
    VS_STANDARD_OUTPUT output;
    
    output.positionW = mul(float4(input.position, 1.0f), gmtxGameObject).xyz;
    output.normalW = mul(input.normal, (float3x3) gmtxGameObject);
    output.tangentW = mul(input.tangent, (float3x3) gmtxGameObject);
    output.bitangentW = mul(input.bitangent, (float3x3) gmtxGameObject);
    output.position = mul(mul(float4(output.positionW, 1.0f), gmtxView), gmtxProjection);
    output.uv = input.uv;

    return (output);
}

VS_STANDARD_OUTPUT VSInstanceStandard(VS_INSTANCE_STANDARD_INPUT input)
{
    VS_STANDARD_OUTPUT output;
    
    float4x4 transformMatrix = mul(input.ins_transform, gmtxGameObject);
    
    output.positionW = mul(float4(input.position, 1.0f), transformMatrix).xyz;
    output.normalW = mul(input.normal, (float3x3) transformMatrix);
    output.tangentW = mul(input.tangent, (float3x3) transformMatrix);
    output.bitangentW = mul(input.bitangent, (float3x3) transformMatrix);
    output.position = mul(mul(float4(output.positionW, 1.0f), gmtxView), gmtxProjection);
    output.uv = input.uv;

    return (output);
}

struct PS_MULTIPLE_RENDER_TARGETS_OUTPUT
{
    float4 cColor : SV_TARGET0;
    float4 cTexture : SV_TARGET1;
    float4 normal : SV_TARGET2;
    float zDepth : SV_TARGET3;
};

PS_MULTIPLE_RENDER_TARGETS_OUTPUT PSStandard(VS_STANDARD_OUTPUT input)
{
    PS_MULTIPLE_RENDER_TARGETS_OUTPUT output;
    
    float4 cAlbedoColor = float4(0.0f, 0.0f, 0.0f, 1.0f);
    float4 cSpecularColor = float4(0.0f, 0.0f, 0.0f, 1.0f);
    float4 cNormalColor = float4(0.0f, 0.0f, 0.0f, 1.0f);
    float4 cMetallicColor = float4(0.0f, 0.0f, 0.0f, 1.0f);
    float4 cEmissionColor = float4(0.0f, 0.0f, 0.0f, 1.0f);
    
    if (gnTexturesMask & MATERIAL_ALBEDO_MAP)
        cAlbedoColor = AlbedoTexture.Sample(gssWrap, input.uv);
    //if (gnTexturesMask & MATERIAL_SPECULAR_MAP)
    //    cSpecularColor = SpecularTexture.Sample(gssWrap, input.uv);
    //if (gnTexturesMask & MATERIAL_NORMAL_MAP)
    //    cNormalColor = NormalTexture.Sample(gssWrap, input.uv);
    //if (gnTexturesMask & MATERIAL_METALLIC_MAP)
    //    cMetallicColor = MetallicTexture.Sample(gssWrap, input.uv);
    //if (gnTexturesMask & MATERIAL_EMISSION_MAP)
    //    cEmissionColor = EmissionTexture.Sample(gssWrap, input.uv);
    
    float4 cColor = cAlbedoColor + cSpecularColor + cMetallicColor + cEmissionColor;
    
    output.cColor = cColor;
    output.cTexture = cColor;
    input.normalW = normalize(input.normalW);
    output.normal = float4(input.normalW.xyz * 0.5f + 0.5f, 1.0f);
    output.zDepth = input.position.z;
    
    return output;
}

//float4 PSStandard(VS_STANDARD_OUTPUT input) : SV_TARGET
//{
//    float4 cAlbedoColor = float4(0.0f, 0.0f, 0.0f, 1.0f);
//    if (gnTexturesMask & MATERIAL_ALBEDO_MAP)
//        cAlbedoColor = AlbedoTexture.Sample(gssWrap, input.uv);
//    float4 cSpecularColor = float4(0.0f, 0.0f, 0.0f, 1.0f);
//    if (gnTexturesMask & MATERIAL_SPECULAR_MAP)
//        cSpecularColor = SpecularTexture.Sample(gssWrap, input.uv);
//    float4 cNormalColor = float4(0.0f, 0.0f, 0.0f, 1.0f);
//    if (gnTexturesMask & MATERIAL_NORMAL_MAP)
//        cNormalColor = NormalTexture.Sample(gssWrap, input.uv);
//    float4 cMetallicColor = float4(0.0f, 0.0f, 0.0f, 1.0f);
//    if (gnTexturesMask & MATERIAL_METALLIC_MAP)
//        cMetallicColor = MetallicTexture.Sample(gssWrap, input.uv);
//    float4 cEmissionColor = float4(0.0f, 0.0f, 0.0f, 1.0f);
//    if (gnTexturesMask & MATERIAL_EMISSION_MAP)
//        cEmissionColor = EmissionTexture.Sample(gssWrap, input.uv);
    
//    float3 normalW;
//    float4 cColor = cAlbedoColor + cSpecularColor + cMetallicColor + cEmissionColor;
//    if (gnTexturesMask & MATERIAL_NORMAL_MAP)
//    {
//        float3x3 TBN = float3x3(normalize(input.tangentW), normalize(input.bitangentW), normalize(input.normalW));
//        float3 vNormal = normalize(cNormalColor.rgb * 2.0f - 1.0f); //[0, 1] �� [-1, 1]
//        normalW = normalize(mul(vNormal, TBN));
//    }
//    else
//    {
//        normalW = normalize(input.normalW);
//    }
    
//    return cColor;
//    //float4 cIllumination = Lighting(input.positionW, normalW);
//    //return (lerp(cColor, cIllumination, 0.5f));
//    //return float4(1.0f, 0.0f, 0.0f, 0.0f);
//}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
#define MAX_VERTEX_INFLUENCES			4
#define SKINNED_ANIMATION_BONES			256

cbuffer cbBoneOffsets : register(b3)
{
    float4x4 gpmtxBoneOffsets[SKINNED_ANIMATION_BONES];
};

cbuffer cbBoneTransforms : register(b4)
{
    float4x4 gpmtxBoneTransforms[SKINNED_ANIMATION_BONES];
};

struct VS_SKINNED_STANDARD_INPUT
{
    float3 position : POSITION;
    float2 uv : TEXCOORD;
    float3 normal : NORMAL;
    float3 tangent : TANGENT;
    float3 bitangent : BITANGENT;
    int4 indices : BONEINDEX;
    float4 weights : BONEWEIGHT;
};

VS_STANDARD_OUTPUT VSSkinnedAnimationStandard(VS_SKINNED_STANDARD_INPUT input)
{
    VS_STANDARD_OUTPUT output;
    
    float4x4 mtxVertexToBoneWorld = (float4x4) 0.0f;
    for (int i = 0; i < MAX_VERTEX_INFLUENCES; i++)
    {
        mtxVertexToBoneWorld += input.weights[i] * mul(gpmtxBoneOffsets[input.indices[i]], gpmtxBoneTransforms[input.indices[i]]);
    }
    output.positionW = mul(float4(input.position, 1.0f), mtxVertexToBoneWorld).xyz;
    output.normalW = mul(input.normal, (float3x3) mtxVertexToBoneWorld).xyz;
    output.tangentW = mul(input.tangent, (float3x3) mtxVertexToBoneWorld).xyz;
    output.bitangentW = mul(input.bitangent, (float3x3) mtxVertexToBoneWorld).xyz;
    

    output.position = mul(mul(float4(output.positionW, 1.0f), gmtxView), gmtxProjection);
    output.uv = input.uv;

    return (output);
}

float4 VSPostProcessing(uint nVertexID : SV_VertexID) : SV_POSITION
{
    if (nVertexID == 0)
        return (float4(-1.0f, +1.0f, 0.0f, 1.0f));
    if (nVertexID == 1)
        return (float4(+1.0f, +1.0f, 0.0f, 1.0f));
    if (nVertexID == 2)
        return (float4(+1.0f, -1.0f, 0.0f, 1.0f));

    if (nVertexID == 3)
        return (float4(-1.0f, +1.0f, 0.0f, 1.0f));
    if (nVertexID == 4)
        return (float4(+1.0f, -1.0f, 0.0f, 1.0f));
    if (nVertexID == 5)
        return (float4(-1.0f, -1.0f, 0.0f, 1.0f));

    return (float4(0, 0, 0, 0));
}

float4 PSPostProcessing(float4 position : SV_POSITION) : SV_Target
{
    return (float4(0.0f, 1.0f, 0.0f, 1.0f));
}