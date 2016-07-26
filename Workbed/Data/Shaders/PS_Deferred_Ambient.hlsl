//---------------------------------
//	Deferred Ambient Pixel Shaders
//---------------------------------
//---------------------------------
//	Samplers & Textures
//---------------------------------

SamplerState point_Clamp : register ( s0 );
Texture2D AlbedoTexture  : register ( t0 );
Texture2D NormalTexture  : register ( t1 );
Texture2D DepthTexture	 : register ( t2 );

//---------------------------------
//	Deferred Ambient Pixel Structs
//---------------------------------

struct VS_OUTPUT
{
	float4 pos	: SV_POSITION0;
	float2 uv	: TEXCOORD;
};

//---------------------------------
//	Deferred Ambient Pixel Shader
//---------------------------------
float4 PS(VS_OUTPUT input) : SV_Target
{
	float4 albedo = AlbedoTexture.Sample(point_Clamp, input.uv);
	if(albedo.a <= 0.f)
		discard;
	/* Begon black background */
	
	float4 normal = NormalTexture.Sample(point_Clamp, input.uv);
	float4 depth = DepthTexture.Sample(point_Clamp, input.uv);
	
	float gamma = 0.9;
	float3 divi = 1.0 / gamma;
	
	albedo.rgb = pow(albedo.rgb, divi.rgb);
	
	normal.xyz *= 2.0f;
	normal.xyz -= 1.f;
	//float4 cubemap = CubeMap.SampleLevel(point_Clamp, normal, 7); //Use in the future.
	//return saturate(albedo * cubemap);
	
	return saturate(albedo);
};