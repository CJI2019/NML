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

cbuffer cbCameraInfo : register(b0)
{
    matrix gmtxView : packoffset(c0);
    matrix gmtxProjection : packoffset(c4);
    float3 gvCameraPosition : packoffset(c8);
};

cbuffer cbGameObjectInfo : register(b1)
{
    matrix gmtxGameObject : packoffset(c0);
    //uint gnTexturesMask : packoffset(c4);
    //MATERIAL gMaterial : packoffset(c4);
};

struct VS_STANDARD_INPUT
{
    float3 position : POSITION;
    //float2 uv : TEXCOORD;
    //float3 normal : NORMAL;
    //float3 tangent : TANGENT;
    //float3 bitangent : BITANGENT;
};

struct VS_STANDARD_OUTPUT
{
    float4 position : SV_POSITION;
    float3 positionW : POSITION;
    //float3 normalW : NORMAL;
    //float3 tangentW : TANGENT;
    //float3 bitangentW : BITANGENT;
    //float2 uv : TEXCOORD;
};


VS_STANDARD_OUTPUT VSStandard(VS_STANDARD_INPUT input)
{
    VS_STANDARD_OUTPUT output;

    output.positionW = mul(float4(input.position, 1.0f), gmtxGameObject).xyz;
    //output.normalW = mul(input.normal, (float3x3) gmtxGameObject);
    //output.tangentW = mul(input.tangent, (float3x3) gmtxGameObject);
    //output.bitangentW = mul(input.bitangent, (float3x3) gmtxGameObject);
    output.position = mul(mul(float4(output.positionW, 1.0f), gmtxView), gmtxProjection);
    //output.uv = input.uv;

    return (output);
}

float4 PSStandard(VS_STANDARD_OUTPUT input) : SV_TARGET
{
    
    return float4(1.0f, 0.0f, 0.0f, 0.0f);
}