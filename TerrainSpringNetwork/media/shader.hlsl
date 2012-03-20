



// Composite world, view, projection transform matrix

cbuffer Transform : register(b0)
{
	float4x4	mTransform;
	float4		vOffsetfactor;
}

cbuffer TexcoordScale : register(b1)
{
	float4		vTexcoordScale[5];
}

Texture2D<float2>	texIndirect		: register(t0);
SamplerState		samIndirect		: register(s0);

Texture2D<float4>	texTerrain0		: register(t1);
Texture2D<float4>	texTerrain1		: register(t2);
Texture2D<float4>	texTerrain2		: register(t3);
Texture2D<float4>	texTerrain3		: register(t4);
Texture2D<float4>	texTerrain4		: register(t5);

SamplerState		samTerrain		: register(s1);



void VSTest(	in  float2 iPosition	: POSITION,
				in  float3 iColor		: COLOR,
                out float4 oPosition	: SV_Position,
                out float4 oColor		: COLOR0 )
{
	float4 vClipSpacePosition = mul( mTransform, float4(iPosition, 0.5f, 1.0f) );
	
    oPosition = float4(vClipSpacePosition.xyz, 1.0f);
    oColor = float4(iColor, 1.0f);
}

float4 PSTest( float4 iPos : SV_POSITION, float4 vColor : COLOR0 ) : SV_Target0
{
    return vColor;
}


void VSTerrain(	in  float3 iPosition	: POSITION,
				in  float2 iTexcoord	: TEXCOORD0,
				in  float2 iNormal		: NORMAL,
				in  float4 iWeight		: TEXCOORD1,
                out float4 oPosition	: SV_Position,
                out float3 oNormal		: NORMAL,
                out float2 oTexcoord	: TEXCOORD0,
                out float4 oWeight		: TEXCOORD1 )
{
	float4 vClipSpacePosition = mul( mTransform, float4(iPosition, 1.0f) );
	
	oTexcoord.xy	= iTexcoord;
    oPosition		= vClipSpacePosition;
    oNormal.xz		= iNormal.xy;
    oNormal.y		= 1.0f - sqrt(iNormal.x * iNormal.x + iNormal.y * iNormal.y);    
    
    oWeight			= iWeight;
}


void PSTerrain(	in  float4 iPosition	: SV_Position,
				in	float3 iNormal		: NORMAL,
                in  float2 iTexcoord	: TEXCOORD0,
                in  float4 iWeight		: TEXCOORD1,                
                out float4 oColor		: SV_Target0 )
{
	
	float2	vOffset	= float2(0.0f, 0.0f); //texIndirect.Sample(samIndirect, iTexcoord) * vOffsetfactor.xy;
	
	float2	vTexcoord	= ((iTexcoord  + vOffset) * 128.0f);
	float4	vColor0		= texTerrain0.Sample(samTerrain, vTexcoord * vTexcoordScale[0].xy);
	float4	vColor1		= texTerrain1.Sample(samTerrain, vTexcoord * vTexcoordScale[1].xy);
	float4	vColor2		= texTerrain2.Sample(samTerrain, vTexcoord * vTexcoordScale[2].xy);
	float4	vColor3		= texTerrain3.Sample(samTerrain, vTexcoord * vTexcoordScale[3].xy);
	float4	vColor4		= texTerrain4.Sample(samTerrain, vTexcoord * vTexcoordScale[4].xy);
			
	float	fWeight0	= iWeight.x;
	float	fWeight1	= iWeight.y;
	float	fWeight2	= iWeight.z;
	float	fWeight3	= iWeight.w;
	float	fWeight4	= 1.0f - dot(iWeight, 1.0f);
	
	float4	vColor		= (vColor0 * fWeight0) + (vColor1 * fWeight1) + (vColor2 * fWeight2) + (vColor3 * fWeight3) + (vColor4 * fWeight4);			
	
	float	fDot		= 0.25f + (dot(iNormal, float3(0.0f, 1.0f, 0.0f)) * 0.75f);	
	
	vColor *= fDot;
	
	//float4 vColorOffset;
	//vColorOffset.xy = vOffset.xy;
	//vColorOffset.zw = float2(0.5f, 1.0f);	
	
	
	
    oColor = vColor;
}

