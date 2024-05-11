#define FRAME_BUFFER_WIDTH 1600
#define FRAME_BUFFER_HEIGHT 1024

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
    matrix gmtxInverseViewProjection : packoffset(c8);
    float4 gvCameraPosition : packoffset(c12); // 16����Ʈ�� ���߱� ���� w��: ���� �߰�
    float4 gvFogColor : packoffset(c13);
    float4 gvfFogInfo : packoffset(c14); // START, RANGE, Density, MOD
};

cbuffer cbGameObjectInfo : register(b1)
{
    matrix gmtxGameObject : packoffset(c0);
    MATERIAL gMaterial : packoffset(c4);
    uint gnTexturesMask : packoffset(c8);
};

#include "Light.hlsl"

SamplerState gssWrap : register(s0);
SamplerState gssWrapPoint : register(s3);

Texture2D DFTextureTexture : register(t5);
Texture2D DFNormalTexture : register(t6);
Texture2D<float> DFzDepthTexture : register(t7);
Texture2D DFPositionTexture : register(t8);

struct PS_POSTPROCESSING_OUT
{
    float4 position : SV_Position;
    float2 uv : UV0;
    float4 uvs[MAX_LIGHTS] : UV1; // ����ȭ�� �ʿ�.. �ȼ����̴����� ������� ���� viewProjection�� ���ؽ����̴����� �ϵ��� �ٲ���.
};

float4 PSPostProcessing(PS_POSTPROCESSING_OUT input) : SV_Target
{
    float4 cColor = DFTextureTexture.Sample(gssWrap, input.uv);
    float3 normal = DFNormalTexture.Sample(gssWrap, input.uv);
    float4 position = DFPositionTexture.Sample(gssWrap, input.uv);
    //return cColor;
    float depth = DFzDepthTexture.Sample(gssWrap, input.uv);
    //return float4(depth, depth, depth, 1.0f);
    //return ShadowMapTexture[0].Sample(gssWrap, input.uv); // ���� ����Ǵ��� Ȯ��
    
    normal = (normal.xyz - 0.5f) / 0.5f; // �븻 ���������� Ȯ���غ��� �ִ� ������ �����Ƿ� �ϴ� �������������� ��ȯ�۾���.
    
    float4 light = Lighting(position, normal);
    
    float3 vCameraPosition = gvCameraPosition.xyz;
    float3 vPostionToCamera = vCameraPosition - position.xyz;
    float fDistanceToCamera = length(vPostionToCamera);
    //float fFogFactor = saturate(((gvfFogInfo.x + gvfFogInfo.y) - fDistanceToCamera) / gvfFogInfo.y);
    float fFogFactor = saturate(1.0f / pow(gvfFogInfo.y + gvfFogInfo.x, pow(fDistanceToCamera * gvfFogInfo.z, 2)));
    //float fFogFactor = saturate(1.0f / pow(gvfFogInfo.y + gvfFogInfo.x, fDistanceToCamera * gvfFogInfo.z));
    cColor = lerp(gvFogColor, cColor * light, fFogFactor);
    
    return (cColor);
}