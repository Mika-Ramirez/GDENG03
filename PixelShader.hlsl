struct PS_INPUT
{
    float4 position : SV_POSITION;
    float3 color : COLOR;
    float3 color1 : COLOR1;
    float3 worldPos : TEXCOORD0;
};

cbuffer constant : register(b0)
{
    row_major float4x4 m_world;        
    row_major float4x4 m_view;         
    row_major float4x4 m_proj;         

    float m_angle;                     
    float3 padding;                   

    float fogStart;                   
    float fogEnd;                     
    float2 padding2;                 

    float3 fogColor;                 
    float fogDensity; 
    float3 cameraPosition;
};


float saturate(float x)
{
    return clamp(x, 0.0, 1.0);
}

float4 psmain(PS_INPUT input) : SV_Target
{
        
        float distance = length(cameraPosition - input.worldPos);

        float fogFactor = (fogEnd - distance) / (fogEnd - fogStart);

        fogFactor = saturate(fogFactor); // Clamp between 0 and 1

        float3 baseColor = input.color;

        float3 finalColor = lerp(fogColor, baseColor, fogFactor);

        return float4(finalColor, 1.0f);

}