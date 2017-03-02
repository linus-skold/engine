//---------------------------------
//	Deferred Base Pixel Shader
//---------------------------------

//---------------------------------
//	Samplers & Textures
//---------------------------------

SamplerState linear_Clamp : register ( s0 );
Texture2D DiffuseTexture  : register ( t0 );

//---------------------------------
//	Render to Texture Vertex Structs
//---------------------------------
struct VS_OUTPUT
{
	float4 pos : SV_POSITION0;
	float2 uv : TEXCOORD;
};

//---------------------------------
//	Deferred Base Pixel Shader
//---------------------------------

float LinearToLum(float4 color)
{
	return(color.r * 0.3 + color.g * 0.59 + color.b * 0.11);
}

float4 PS(VS_OUTPUT input) : SV_Target
{	
	float4 diffuse = DiffuseTexture.Sample(linear_Clamp, input.uv) ;
	float lum = (diffuse.r * (0.2126)) + (diffuse.g * (0.7152)) + (diffuse.b * (0.0722));
	
	return float4(lum,lum,lum,1);

	return float4(0,0,0,1);
}