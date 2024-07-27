#include "Common.hlsl"
RWTexture2D<float4> gtxtRWOutput : register(u0);

[numthreads(32, 32, 1)]
void CSBloom(uint3 n3DispatchThreadID : SV_DispatchThreadID)
{
    float4 finalColor = DFLightTexture[n3DispatchThreadID.xy];
    float4 positionW = DFPositionTexture[n3DispatchThreadID.xy];  

    float3 vCameraPosition = gvCameraPosition.xyz;
    float3 vPostionToCamera = vCameraPosition - positionW.xyz;
    float fDistanceToCamera = length(vPostionToCamera);
    float fFogFactor = saturate(1.0f / pow(gvfFogInfo.y + gvfFogInfo.x, pow(fDistanceToCamera * gvfFogInfo.z, 2))) * gvfFogInfo.w;
    finalColor = lerp(gvFogColor, finalColor, fFogFactor);
    
    // ������ �����Ͽ� ��� ȿ���� ����   
    float4 blurredColor = float4(0.0, 0.0, 0.0, 0.0);
    int radius = 5; // ���� �ݰ�
    int sampleCount = 0;
    for(int x = -radius; x <= radius; x++)
    {
        for(int y = -radius; y <= radius; y++)
        {
            int2 samplePos = int2(n3DispatchThreadID.xy) + int2(x, y);
            // �ؽ�ó ��踦 ���� �ʵ��� Ŭ����
            samplePos = clamp(samplePos, int2(0, 0), int2(FRAME_BUFFER_WIDTH - 1, FRAME_BUFFER_HEIGHT - 1));
            blurredColor += DFTextureEmissive[samplePos];
            sampleCount++;
        }
    }
    if(sampleCount != 0)
        blurredColor /= sampleCount;
    finalColor += blurredColor;
    gtxtRWOutput[n3DispatchThreadID.xy] = finalColor;
}