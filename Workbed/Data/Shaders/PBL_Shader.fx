#include "SharedVariables.fx"

struct DIRECTIONAL_LIGHT
{
	float4 Color;
	float4 Direction;
};

DIRECTIONAL_LIGHT DirectionalLight[1];

struct POINT_LIGHT
{
	float4 Color;
	float3 Position;
	float  Range;
};

POINT_LIGHT PointLight[4];

PS_INPUT_POS_NORMAL_UV_BINORMAL_TANG VS(VS_INPUT_POS_NORMAL_UV_BINORMAL_TANG input)
{
	PS_INPUT_POS_NORMAL_UV_BINORMAL_TANG output = (PS_INPUT_POS_NORMAL_UV_BINORMAL_TANG)0;
	output.Pos = mul(input.Pos, World);
	output.Pos = mul(output.Pos, View);
	output.Pos = mul(output.Pos, Projection);

	output.Normal = mul(input.Normal, World);
	output.UV = input.UV;
	
	output.Tang = mul(input.Tang, World);
	output.WorldPos = mul(input.Pos, World);

	return output;
}

static const float ambient = 0.42f;

float CalculateAttenuation(float someDistance)
{
	float attenuation = 1.f / (1.f + 0.1f * someDistance + 0.01f * someDistance * someDistance);
	return attenuation;
}

float CalculateFalloff(float someDistance, float someRange)
{
	float fallOff = 1.f - (someDistance / (someRange + 0.1f));
	return fallOff;
}

float CalculateTotalAttenuation(float someDistance, float someRange)
{
	float totalAttenuation = CalculateAttenuation(someDistance) * CalculateFalloff(someDistance, someRange);
	return totalAttenuation;
}

float4 PS(PS_INPUT_POS_NORMAL_UV_BINORMAL_TANG input) : SV_Target
{
	float4 albedo = AlbedoTexture.Sample(linearSample_Wrap, input.UV);
	float3 binormal = normalize(cross(input.Normal, input.Tang));
	float3 normal = NormalTexture.Sample(linearSample_Wrap, input.UV);
	float4 roughness = RoughnessTexture.Sample(linearSample_Wrap, input.UV);
	float4 metalness = MetalnessTexture.Sample(linearSample_Wrap, input.UV);
	
	float3x3 tangentSpaceMatrix = float3x3(input.Tang, binormal, input.Normal);

	float4 diffuse = ambient * albedo;
	normal = normalize(normal);
	input.Tang = normalize(input.Tang);
	normal = normalize(mul(normal, tangentSpaceMatrix));
	float4 finalColor = 0.0f;
	//return DirectionalLight[0].Direction;
	
	//for(int i = 0; i < 1; i++)
	//{
	//	float lambert = dot(-DirectionalLight[i].Direction, normal);
	//	finalColor += saturate(lambert * DirectionalLight[i].Color);
	//}
	
	for(int i = 0; i < 4; i++)
	{
		float3 lightDirection = PointLight[i].Position - input.WorldPos;
		float distance = length(lightDirection);
		lightDirection = normalize(lightDirection);
		
		float lambert = dot(lightDirection, normal);
		finalColor += saturate(lambert * PointLight[i].Color) * CalculateTotalAttenuation(distance,PointLight[i].Range);
		finalColor = saturate(finalColor * 2);
	}
	//float2 b = voronoi(14.0 + 6.0 * input.UV);

	
	finalColor.a = 1;
	return float4(diffuse.rgb + (albedo.rgb * finalColor.rgb), 1);
}

technique11 Render
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_5_0, VS()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PS()));
	}
}