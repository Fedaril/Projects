
// Composite world, view, projection transform matrix
float4x4	mCompositeTransform		: register(c0);

float4		vHeightmapScale			: register(c5);
float4		vSlopeScale				: register(c6);

float4		vTexCoordScale0			: register(c18);
float4		vTexCoordScale1			: register(c19);
float4		vTexCoordScale2			: register(c20);
float4		vTexCoordScale3			: register(c21);
float4		vTexCoordScale4			: register(c22);


float		vLodLevelOffset[7]		: register(c30);
float		vLodLevelDimension[7]	: register(c37);



// heightmap for tests
sampler		TextureSampler0			: register(s0);
sampler		TextureSampler1			: register(s1);
sampler		TextureSampler2			: register(s2);
sampler		TextureSampler3			: register(s3);
sampler		TextureSampler4			: register(s4);


float		vIndexOffset			: register(c4);

struct VSInput
{
	int    vIndex      : INDEX;
    float2 vUV         : BARYCENTRIC;
    int    vQuadID     : QUADID;
};

struct Interpolator
{
	float4 vPosition			: POSITION;	
    float4 vColor				: COLOR;
    float3 vNormal				: NORMAL;
    float2 vTexCoord			: TEXCOORD0;
    float4 vMaterialWeights		: TEXCOORD1;    
};


//--------------------------------------------------------------------------------------
// Name: TestVS
// Desc: Vertex shader for tests !
//--------------------------------------------------------------------------------------
Interpolator TerrainVS(VSInput oInput, out float fKillVertex : KILLVERTEX)
{
	Interpolator oResult = (Interpolator)0;
	
    // Fetch the top/left corner information
    
    // pos0.x => tile logical position x
    // pos0.y => tile logical position y
    // pos0.z => tile lod level
    // pos0.w => tile lod scale (1 << lod level)
    
    float4 vPos;
    float4 vCol;
    
    int iIndex = oInput.vIndex.x + vIndexOffset.x;

    asm 
    {
        vfetch vPos, iIndex, position0        
        vfetch vCol, iIndex, color0
    };   

	//	
    float fTileLodLevel = vPos.z;
    float fTileLodScale = vPos.w;

	float fQuadDimension = fTileLodScale * 4.0f;
	float2 fQuadBasePos = vPos.xy;
	
	float2 vOffset = oInput.vUV.xy;
	
	float2 vQuadDirection = float2(-1.0f, 1.0f);
	 
	if ((oInput.vQuadID == 0) || (oInput.vQuadID == 1))
	{
		vOffset.y = 1.0f - vOffset.y;
		vQuadDirection.y = -vQuadDirection.y;
	}	
	if ((oInput.vQuadID == 2) || (oInput.vQuadID == 1))
	{
		vOffset.x = 1.0f - vOffset.x;
		vQuadDirection.x = -vQuadDirection.x;
	}	
	
	fQuadBasePos.xy += vOffset;
	fQuadBasePos.xy *= fQuadDimension;
	
	
	
	float fPosX = fQuadBasePos.x;
	float fPosY = fQuadBasePos.y;	


	
	//
	const float fHeightMapDimension		= vLodLevelDimension[0].x * 4.0f;
	
    float4 vHeight				= float4(0.0f, 0.0f, 0.0f, 0.0f);
    float4 vNormal				= float4(0.0f, 0.0f, 0.0f, 0.0f);

    float fHeightMapOffsetX			= clamp(trunc(fPosX + 0.5f), 0.0f, fHeightMapDimension - 1);
    float fHeightMapOffsetY			= clamp(trunc(fPosY + 0.5f), 0.0f, fHeightMapDimension - 1);   
	float fHeightMapOffset			= (fHeightMapOffsetY * fHeightMapDimension) + fHeightMapOffsetX;
    
   
    asm
    {
        vfetch vHeight,				fHeightMapOffset,				texcoord0
        vfetch vNormal,				fHeightMapOffset,				normal0        
    };
    
    float2 vUV = float2(fPosX * vHeightmapScale.x, fPosY * vHeightmapScale.x);
	float4 vColor = float4(1.0f, 1.0f, 1.0f, 0.0f);
    
/*    
    //
    float fTileHeightMapX0 = vPos.x * fQuadDimension;
    float fTileHeightMapY0 = vPos.y * fQuadDimension;
    float fTileHeightMapX1 = clamp(fTileHeightMapX0 + fQuadDimension, 0.0f, fHeightMapDimension - 1);
    float fTileHeightMapY1 = clamp(fTileHeightMapY0 + fQuadDimension, 0.0f, fHeightMapDimension - 1);
    
    
    float fTileHeightMapOffset0	= (fTileHeightMapY0 * fHeightMapDimension) + fTileHeightMapX0;
    float fTileHeightMapOffset1	= (fTileHeightMapY0 * fHeightMapDimension) + fTileHeightMapX1;
	float fTileHeightMapOffset2 = (fTileHeightMapY1 * fHeightMapDimension) + fTileHeightMapX0;
	float fTileHeightMapOffset3 = (fTileHeightMapY1 * fHeightMapDimension) + fTileHeightMapX1;	

	float4 vTileHeight0 = float4(0, 0, 0, 0);
	float4 vTileHeight1 = float4(0, 0, 0, 0);
	float4 vTileHeight2 = float4(0, 0, 0, 0);
	float4 vTileHeight3 = float4(0, 0, 0, 0);

    asm
    {
		vfetch vTileHeight0,		fTileHeightMapOffset0,			texcoord0
        vfetch vTileHeight1,		fTileHeightMapOffset1,			texcoord0
        vfetch vTileHeight2,		fTileHeightMapOffset2,			texcoord0
		vfetch vTileHeight3,		fTileHeightMapOffset3,			texcoord0
    };
    
    float4 vTileHeight = float4(vTileHeight0.x, vTileHeight1.x, vTileHeight2.x, vTileHeight3.x);
    
    vColor.xyz = vTileHeight.xyz;  
    


    //
    float3 vTilePos0 = float3(fTileHeightMapX0, vTileHeight0.x, fTileHeightMapY0) * vHeightmapScale.xzx;
    float3 vTilePos1 = float3(fTileHeightMapX1, vTileHeight1.x, fTileHeightMapY0) * vHeightmapScale.xzx;
    float3 vTilePos2 = float3(fTileHeightMapX0, vTileHeight2.x, fTileHeightMapY1) * vHeightmapScale.xzx;
    float3 vTilePos3 = float3(fTileHeightMapX1, vTileHeight3.x, fTileHeightMapY1) * vHeightmapScale.xzx;
    
    float3 vCross0 = cross(vTilePos2 - vTilePos0, vTilePos1 - vTilePos0);
    float3 vCross1 = cross(vTilePos0 - vTilePos1, vTilePos3 - vTilePos1);
    float3 vCross2 = cross(vTilePos1 - vTilePos3, vTilePos2 - vTilePos3);
    float3 vCross3 = cross(vTilePos2 - vTilePos0, vTilePos3 - vTilePos2);
    
	float3 vCross = normalize(abs(vCross0) + abs(vCross1) + abs(vCross2) + abs(vCross3));
    //vCross = (vCross + 1.0f) * 0.5f;
    vColor.xyz = vCross;
    
   
	float3 vAxisDistance = vCross;

    if ((vAxisDistance.y >= vAxisDistance.x) && (vAxisDistance.y >= vAxisDistance.z))
    {
		// project on Y
		vUV = float2(fPosX * vHeightmapScale.x, fPosY * vHeightmapScale.x);
		vColor = float4(0.0f, 1.0f, 0.0f, 0.0f); 
    }
    else if ((vAxisDistance.z >= vAxisDistance.x) && (vAxisDistance.z >= vAxisDistance.y))
    {
		// project on Z
		vUV = float2(fPosX * vHeightmapScale.x, vHeight.x * vHeightmapScale.z);
		vColor = float4(0.0f, 0.0f, 1.0f, 0.0f);
    }
    else if ((vAxisDistance.x >= vAxisDistance.y) && (vAxisDistance.x >= vAxisDistance.z))
    {
		// project on X
		vUV = float2(vHeight.x * vHeightmapScale.z, fPosY * vHeightmapScale.x);
		vColor = float4(1.0f, 0.0f, 0.0f, 0.0f);
    }   
*/


    // fetch material weight
    float vLodDimension				= vLodLevelDimension[fTileLodLevel];
    float fMaterialWeightLodOffset	= vLodLevelOffset[fTileLodLevel];    
    float fMaterialWeightOffsetX	= vPos.x;
    float fMaterialWeightOffsetY	= vPos.y;
    
    float2 fTileElementIndex = trunc((vOffset.xy * 4.0f) + 0.5f);
    float fTileWeightOffset = (fTileElementIndex.y * 5.0f) + fTileElementIndex.x;
    
    float fMaterialWeightOffset = 0.0f;
    fMaterialWeightOffset = fMaterialWeightLodOffset + (fMaterialWeightOffsetY * vLodDimension) + fMaterialWeightOffsetX;
    
   
    fMaterialWeightOffset *= 25.0f;
    fMaterialWeightOffset += fTileWeightOffset;
        
    float4 vMaterialWeight;
    asm
    {
        vfetch vMaterialWeight, fMaterialWeightOffset, texcoord2;
    };    
    
   
	//        
    float3 vWorldPos = float3(fPosX, vHeight.x, fPosY) * vHeightmapScale.xzx;

    oResult.vColor				= vColor;
    oResult.vPosition			= mul(mCompositeTransform, float4(vWorldPos, 1.0f));
    oResult.vNormal				= vNormal.xyz;
    oResult.vTexCoord			= vUV.xy;
    oResult.vMaterialWeights	= vMaterialWeight;    
    
    fKillVertex					= 0;
    
    return oResult;
}


float4 TerrainPS( Interpolator oInput ) : COLOR
{
	float2 vTexCoord0	= oInput.vTexCoord * vTexCoordScale0.xy;
	float2 vTexCoord1	= oInput.vTexCoord * vTexCoordScale1.xy;
	float2 vTexCoord2	= oInput.vTexCoord * vTexCoordScale2.xy;
	float2 vTexCoord3	= oInput.vTexCoord * vTexCoordScale3.xy;
	float2 vTexCoord4	= oInput.vTexCoord * vTexCoordScale4.xy;
	
	float4 vColor0 = tex2D(TextureSampler0, vTexCoord0);
	float4 vColor1 = tex2D(TextureSampler1, vTexCoord1);
	float4 vColor2 = tex2D(TextureSampler2, vTexCoord2);
	float4 vColor3 = tex2D(TextureSampler3, vTexCoord3);
	float4 vColor4 = tex2D(TextureSampler4, vTexCoord4);
	
	
	float fMaterialWeight0 = oInput.vMaterialWeights.x;
	float fMaterialWeight1 = oInput.vMaterialWeights.y;
	float fMaterialWeight2 = oInput.vMaterialWeights.z;
	float fMaterialWeight3 = oInput.vMaterialWeights.w;
	float fMaterialWeight4 = saturate(1.0f - dot(oInput.vMaterialWeights, float4(1.0f, 1.0f, 1.0f, 1.0f)));
	
	float4 vColor = 
			vColor0 * fMaterialWeight0
		+	vColor1 * fMaterialWeight1		
		+	vColor2 * fMaterialWeight2
		+	vColor3 * fMaterialWeight3
		+	vColor4 * fMaterialWeight4;
	
	return vColor;
}


float4 TerrainDebugPS( Interpolator oInput ) : COLOR
{
	float2 vTexCoord0	= oInput.vTexCoord * vTexCoordScale0.xy;
	float2 vTexCoord1	= oInput.vTexCoord * vTexCoordScale1.xy;
	float2 vTexCoord2	= oInput.vTexCoord * vTexCoordScale2.xy;
	float2 vTexCoord3	= oInput.vTexCoord * vTexCoordScale3.xy;
	float2 vTexCoord4	= oInput.vTexCoord * vTexCoordScale4.xy;
	
	float4 vColor0 = tex2D(TextureSampler0, vTexCoord0);
	float4 vColor1 = tex2D(TextureSampler1, vTexCoord1);
	float4 vColor2 = tex2D(TextureSampler2, vTexCoord2);
	float4 vColor3 = tex2D(TextureSampler3, vTexCoord3);
	float4 vColor4 = tex2D(TextureSampler4, vTexCoord4);
	
	
	float fMaterialWeight0 = oInput.vMaterialWeights.x;
	float fMaterialWeight1 = oInput.vMaterialWeights.y;
	float fMaterialWeight2 = oInput.vMaterialWeights.z;
	float fMaterialWeight3 = oInput.vMaterialWeights.w;
	float fMaterialWeight4 = saturate(1.0f - dot(oInput.vMaterialWeights, float4(1.0f, 1.0f, 1.0f, 1.0f)));
	
	float4 vColor = 
			vColor0 * fMaterialWeight0
		+	vColor1 * fMaterialWeight1		
		+	vColor2 * fMaterialWeight2
		+	vColor3 * fMaterialWeight3
		+	vColor4 * fMaterialWeight4;
	
	return float4(oInput.vColor.xyzw); // oInput.vMaterialWeights + (vColor * 0.00001f); //oInput.vColor.; //vColor;
}