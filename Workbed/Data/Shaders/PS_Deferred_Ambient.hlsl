//---------------------------------
//	Deferred Ambient Pixel Shaders
//---------------------------------
cbuffer CameraPosition : register(b0)
{
	float4 camera_position;
	row_major float4x4 InvertedProjection;
	row_major float4x4 InvertedView;
};
//---------------------------------
//	Samplers & Textures
//---------------------------------

SamplerState point_Clamp 	: register ( s0 );
Texture2D AlbedoTexture  	: register ( t0 );
Texture2D NormalTexture  	: register ( t1 );
Texture2D DepthTexture	 	: register ( t2 );
TextureCube CubeMap		 	: register ( t3 );

//---------------------------------
//	Deferred Ambient Pixel Structs
//---------------------------------

struct VS_OUTPUT
{
	float4 pos	: SV_POSITION0;
	float2 uv	: TEXCOORD;
};

struct PixelData
{
	float4 albedo;
	float4 normal;
	float4 depth;
	float4 metalness;
	float4 roughness;
	float4 AOTexture;
	float4 emissive;
};

//---------------------------------
//	Deferred Ambient Pixel Shader
//---------------------------------

float GetSpecPowToMip(float fSpecPow, int nMips)
{
	float fSmulMaxT = (exp2(-10.0 / sqrt(fSpecPow)) - 0.00098f) / 0.9921f;
    float fMaxT = (exp2(-10.f / sqrt((2.f / (0.0014f * 0.0014f)) - 2.f)) - 0.00098f) / 0.9921f;
	return float(nMips - 1 - 0) * (1.0 - clamp(fSmulMaxT / fMaxT, 0.0, 1.0));
}

float CalculateAttenuation(float someDistance)
{
	float attenuation = 1.f / (1.f + 0.1f * someDistance + 0.01f * someDistance * someDistance);
	return attenuation;
}

float CalculateFalloff(float someDistance, float someRange)
{
	float fallOff = 1.f - (someDistance / (someRange + 0.001));
	return fallOff;
}

float CalculateTotalAttenuation(float someDistance, float someRange)
{
	float totalAttenuation = CalculateAttenuation(someDistance) * CalculateFalloff(someDistance, someRange);
	return totalAttenuation;
}

float4 PS(VS_OUTPUT input) : SV_Target
{
	float4 depth = DepthTexture.Sample(point_Clamp, input.uv);
	if(depth.x <= 0.f)
		discard;


	float4 albedo = AlbedoTexture.Sample(point_Clamp, input.uv);	
	float4 normal = NormalTexture.Sample(point_Clamp, input.uv) * 2 - 1;
	float4 metalness = float4(normal.w, normal.w, normal.w, normal.w);
	float roughness = depth.y;
	float roughnessOffsetted = pow(8192, roughness);
	float x = input.uv.x * 2.f - 1.f;
	float y = (1.f - input.uv.y) * 2.f - 1.f;
	float z = depth.x; 	
	float ao = 1.0f;
	
	float4 worldPosition = float4(x, y, z, 1.f);
	worldPosition = mul (worldPosition, InvertedProjection);
	worldPosition = worldPosition / worldPosition.w;
	worldPosition = mul(worldPosition, InvertedView);	
	
	float3 viewPos = camera_position.xyz;
	float3 toEye = normalize(viewPos -  worldPosition.xyz);
    
	float4 substance = (0.04f - 0.04f * metalness) 
	+ albedo * metalness;
            
	float4 metalnessAlbedo = albedo - (albedo * metalness);
	float LdotH = dot(normal, toEye);
	LdotH = saturate(LdotH);
	LdotH = 1.0f - LdotH;
	LdotH = pow(LdotH, 5);
	float3 fresnel = LdotH * (1.f - substance);
	fresnel = fresnel / (2 - 1 * (1.f - roughnessOffsetted));
	fresnel = substance + fresnel;
  
	float3 reflectionFrensnel =	fresnel;
	float3 reflectionVector = reflect(toEye, normal);
    
	float fakeLysSpecularPower = (2.f / (roughness * roughness)) - 2.f;
	float lysMipMap = GetSpecPowToMip(fakeLysSpecularPower, 12);
    
	float3 ambientDiffuse = CubeMap.SampleLevel(point_Clamp, normal, 9).rgb * ao 
	* metalnessAlbedo * (1.f - reflectionFrensnel);

	float3 ambientSpec = CubeMap.SampleLevel(point_Clamp, reflectionVector, lysMipMap).xyz 
	* ao * reflectionFrensnel;
    //ambientSpec = float3(1,1,1);
    
	float3 finalColor = metalnessAlbedo.rgb;
    
	//float4 col = saturate(albedo * cubemap);
	//col.rgb = pow (col.rgb, 1 / 2.2);
	
	return float4(finalColor, 1.f)*0.42;

};
