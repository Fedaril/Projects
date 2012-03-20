
#include "StdAfx.h"

#include "QuadTree.h"

enum SubTileMask
{
	SubTileMask_Left		= 1 << 0,
	SubTileMask_Right		= 1 << 1,
	SubTileMask_Top			= 1 << 2,
	SubTileMask_Bottom		= 1 << 3,
};

////////////////////////////
//
const unsigned int g_arrSubTileFullIndexCount = 24;
static unsigned int g_arrSubTileFullIndex[g_arrSubTileFullIndexCount] =
{
	0,  1,  5,  1,  6,  5,  1,  2,  6,  2,  7,  6, 
	5,  6, 10,  6, 11, 10,  6,  7, 11,  7, 12, 11
};


//
const unsigned int g_arrSubTileTopIndexCount = 21;
static unsigned int g_arrSubTileTopIndex[g_arrSubTileTopIndexCount] =
{
	0,  6,  5,  0,  2,  6,  2,  7,  6,
	5,  6, 10,  6, 11, 10,  6,  7, 11,  7, 12, 11
};

//
const unsigned int g_arrSubTileBottomIndexCount = 21;
static unsigned int g_arrSubTileBottomIndex[g_arrSubTileBottomIndexCount] =
{
	0,  1,  5,  1,  6,  5,  1,  2,  6,  2,  7,  6, 
	5,  6, 10,  6, 12, 10, 6, 7, 12
};


//
const unsigned int g_arrSubTileLeftIndexCount = 21;
static unsigned int g_arrSubTileLeftIndex[g_arrSubTileLeftIndexCount] =
{
	0,  1,  6,  1,  2,  6,  2,  7,  6, 
	0,  6, 10,
	6, 11, 10,  6,  7, 11,  7, 12, 11
};



//
const unsigned int g_arrSubTileRightIndexCount = 21;
static unsigned int g_arrSubTileRightIndex[g_arrSubTileRightIndexCount] =
{
	0,  1,  5,  1,  6,  5,  1, 2, 6, 
	2,  6, 12,
	5,  6, 10,  6, 11, 10, 6, 12, 11
};

//
const unsigned int g_arrSubTileTopLeftIndexCount = 18;
static unsigned int g_arrSubTileTopLeftIndex[g_arrSubTileTopLeftIndexCount] =
{
	0,  2,  6,  0,  6, 10,  2,  7,  6,  6, 11, 10, 
	6,  7, 11,  7, 12, 11
};

//
const unsigned int g_arrSubTileTopRightIndexCount = 18;
static unsigned int g_arrSubTileTopRightIndex[g_arrSubTileTopRightIndexCount] =
{
	0,  2,  6,  2, 12,  6,
	0, 6, 5, 6, 12, 11,
	5, 6, 10, 6, 11, 10
};

//
const unsigned int g_arrSubTileBottomLeftIndexCount = 18;
static unsigned int g_arrSubTileBottomLeftIndex[g_arrSubTileBottomLeftIndexCount] =
{
	0, 6, 10, 10, 6, 12,
	0, 1, 6, 6, 7, 12,
	1, 2, 6, 2, 7, 6
};


//
const unsigned int g_arrSubTileBottomRightIndexCount = 18;
static unsigned int g_arrSubTileBottomRightIndex[g_arrSubTileBottomRightIndexCount] =
{
	2, 12, 6, 6, 12, 10,
	5, 6, 10, 1, 2, 6,
	0, 1, 5, 1, 6, 5
};



// 1:Left 2:Right 4:Top 8:Bottom   
struct SubTileIndexInfo
{
	unsigned int	m_iArraySize;
	unsigned int*	m_pArray;
};
static SubTileIndexInfo g_arrSubTileIndexInfo[16] = 
{
	/* 0	Full					*/		{ g_arrSubTileFullIndexCount,			g_arrSubTileFullIndex			},
	/* 1	Left					*/		{ g_arrSubTileLeftIndexCount,			g_arrSubTileLeftIndex			},
	/* 2	Right					*/		{ g_arrSubTileRightIndexCount,			g_arrSubTileRightIndex			},
	/* 3	Right+Left				*/		{ 0,									NULL							},
	/* 4	Top						*/		{ g_arrSubTileTopIndexCount,			g_arrSubTileTopIndex			},
	/* 5	Top+Left				*/		{ g_arrSubTileTopLeftIndexCount,		g_arrSubTileTopLeftIndex		},
	/* 6	Top+Right				*/		{ g_arrSubTileTopRightIndexCount,		g_arrSubTileTopRightIndex		},
	/* 7	Top+Right+Left			*/		{ 0,									NULL							},
	/* 8	Bottom					*/		{ g_arrSubTileBottomIndexCount,			g_arrSubTileBottomIndex			},
	/* 9	Bottom+Left				*/		{ g_arrSubTileBottomLeftIndexCount,		g_arrSubTileBottomLeftIndex		},
	/* 10	Bottom+Right			*/		{ g_arrSubTileBottomRightIndexCount,	g_arrSubTileBottomRightIndex	},
	/* 11	Bottom+Right+Left		*/		{ 0,									NULL							},
	/* 12	Bottom+Top				*/		{ 0,									NULL							},
	/* 13	Bottom+Top+Left			*/		{ 0,									NULL							},
	/* 14	Bottom+Top+Right		*/		{ 0,									NULL							},
	/* 15	Bottom+Top+Right+Left	*/		{ 0,									NULL							},
};

///////////////////////////

struct TextureScaleInfo
{
	float m_fScaleX;
	float m_fScaleY;
};

struct PatchVertex
{
	float			m_fPositionX;
	float			m_fPositionY;
	float			m_fPositionZ;
	float			m_fTexcoordX;
	float			m_fTexcoordY;
	unsigned int	m_u32Normal;
	unsigned int	m_u32Weight;
};

struct DrawBatch
{
	unsigned int m_iMaterialType;
	unsigned int m_iIndexStart;
	unsigned int m_iIndexCount;
};


const GEUInt g_iInvalidMaterialIndex	= 15;
const GEUInt g_iMaxMaterialCount		= 15;
const GEUInt g_iMaxMaterialPerTile		= 5;

struct MaterialData
{
	unsigned int		m_iMask;
	unsigned int		m_arrMaterialIndex[g_iMaxMaterialPerTile];
};

//--------------------------------------------------------------------------------------
// Vertex declarations.
//--------------------------------------------------------------------------------------
const unsigned int g_iVertexElementCount = 4; 
Gfx::InputLayoutElement g_arrVertexElement[g_iVertexElementCount] =
{
	{ Gfx::InputElementSemantic_Position,	0,		0,		Gfx::InputElementFormat_Float_3 },
	{ Gfx::InputElementSemantic_Texcoord,	0,		12,		Gfx::InputElementFormat_Float_2 },
	{ Gfx::InputElementSemantic_Normal,		0,		20,		Gfx::InputElementFormat_Float16_2 },	
	{ Gfx::InputElementSemantic_Texcoord,	1,		24,		Gfx::InputElementFormat_Byte_4_Unorm },	
};

static const char* s_pTextureName[PatchQuadTree::s_iMaxTextureCount] =
{
	"media\\00_TR_Nat_Grass_2_Df.dds",
	"media\\01_TR_Nat_Rock_14_Df.dds",
	"media\\02_TR_Nat_Sand_3_Df.dds",
	"media\\03_TR_Nat_Grass_1_Df.dds",
	"media\\04_TR_Nat_Ground_1_Df.dds",
	"media\\05_TR_Nat_Ground_1_Df.dds",
	"media\\06_TR_Nat_GrndPlant_1_Df.dds",
	"media\\07_TR_Nat_GrndPlant_2_Df.dds",
	"media\\08_TR_Nat_Ground_10_Df.dds",
	"media\\09_TR_Nat_Path_1_Df.dds",
	"media\\10_TR_Nat_Path_2_Df.dds",
	"media\\11_TR_Nat_Ground_3_Df.dds",
	"media\\12_TR_Nat_Ground_2_Df.dds",
	"media\\13_TR_Nat_Ground_2_Df.dds",
	"media\\14_TR_Nat_Grass_3_Df.dds"
};




static TextureScaleInfo s_pTextureScale[PatchQuadTree::s_iMaxTextureCount] =
{
	{ 0.0004f, 0.0004f },
	{ 0.0005f, 0.0005f },
	{ 0.0020f, 0.0020f },
	{ 0.0015f, 0.0015f },
	{ 0.0005f, 0.0005f },
	{ 0.0015f, 0.0015f },
	{ 0.0015f, 0.0015f },
	{ 0.0020f, 0.0020f },
	{ 0.0017f, 0.0017f },
	{ 0.0020f, 0.0020f },
	{ 0.0025f, 0.0025f },
	{ 0.0015f, 0.0012f },
	{ 0.0005f, 0.0005f },
	{ 0.0015f, 0.0015f },
	{ 0.0015f, 0.0015f }
};


GEUInt PatchQuadTree::FindMaterialInfoIndex(VertexWeightLayout a_eLayout, GEU32 a_iMaterialMask, GEUInt a_iCurrentCount)
{
	GEUInt iMatchIndex = a_iCurrentCount;

	for (GEUInt iMaterialIndex = 0; iMaterialIndex < a_iCurrentCount; ++iMaterialIndex)
	{
		if (m_oData[a_eLayout].m_pMaterialDataArray[iMaterialIndex].m_iMask == a_iMaterialMask)
		{				
			iMatchIndex = iMaterialIndex;
			break;
		}
	}

	return iMatchIndex;
}


void PatchQuadTree::CreateMaterialInfoArrayFromXinfo(const GEU16* a_arrTileMaterialMask, GEUInt a_iTileCount)
{
	//
	GEUInt iCurrentCount = 0;

	for (GEUInt iTileIndex = 0; iTileIndex < a_iTileCount; ++iTileIndex)
	{
		GEU32	u32TileMaterialMask = a_arrTileMaterialMask[iTileIndex];
		GEUInt	iMatchIndex = FindMaterialInfoIndex(VertexWeightLayout_Xinfo, u32TileMaterialMask, iCurrentCount);

		if (iMatchIndex == iCurrentCount)
		{
			GE_ASSERT(iCurrentCount < m_oData[VertexWeightLayout_Xinfo].m_iMaterialDataArraySize);
			m_oData[VertexWeightLayout_Xinfo].m_pMaterialDataArray[iCurrentCount].m_iMask = u32TileMaterialMask;

			GEUInt iMaterialCount = 0;
			GEU32 u32Mask = u32TileMaterialMask;
			while (u32Mask != 0)
			{
				GEUInt iMaterialIndex = 31 - Util::CountLeadingZeros(u32Mask);

				//
				ASSERT(iMaterialCount < g_iMaxMaterialPerTile);
				ASSERT(iMaterialIndex < g_iMaxMaterialCount);

				m_oData[VertexWeightLayout_Xinfo].m_pMaterialDataArray[iCurrentCount].m_arrMaterialIndex[iMaterialCount] = iMaterialIndex;
				iMaterialCount++;

				u32Mask &= ~(1 << iMaterialIndex);
			}

			for (GEUInt iSlotIndex = iMaterialCount; iSlotIndex < g_iMaxMaterialPerTile; ++iSlotIndex)
			{
				m_oData[VertexWeightLayout_Xinfo].m_pMaterialDataArray[iCurrentCount].m_arrMaterialIndex[iSlotIndex] = g_iInvalidMaterialIndex;
			}
			
			iCurrentCount++;
		}

		//
		m_oData[VertexWeightLayout_Xinfo].m_pTileMaterialDataIndexArray[iTileIndex] = iMatchIndex;
	}
}

GEUInt PatchQuadTree::CountXinfoMaterialData(const GEU16* a_arrTileMaterialMask, GEUInt a_iTileCount)
{
	//
	GEUInt iCount = 0;

	for (GEUInt iTileIndex = 0; iTileIndex < a_iTileCount; ++iTileIndex)
	{
		GEU32	u32MaterialMask		= a_arrTileMaterialMask[iTileIndex];
		bool	bFoundDuplicate		= false;

		for (GEUInt iSearchIndex = 0; iSearchIndex < iTileIndex ; ++iSearchIndex)
		{
			if (a_arrTileMaterialMask[iSearchIndex] == u32MaterialMask)
			{
				bFoundDuplicate = true;
				break;
			}
		}

		if (!bFoundDuplicate)
			iCount++;		
	}

	return iCount;
}

void PatchQuadTree::FromXinfo(const char* a_szHmapFile)
{
	//
	unsigned int iTileCount = s_iDimension * s_iDimension;
	unsigned int iDataTileCount = s_iDimension * s_iDimension;

	unsigned int iDataLodLevelCount = 6;

	for (unsigned int iLodLevel = 1; iLodLevel < iDataLodLevelCount; ++iLodLevel)
	{
		iDataTileCount += (s_iDimension >> iLodLevel) * (s_iDimension >> iLodLevel);

		if (iLodLevel == (s_iLodLevelCount - 1))
		{
			iTileCount = iDataTileCount;
		}
	}

	void* pXinfoData = NULL;
	unsigned int iXinfoDataSize = 0;
	Util::ReadFile("media\\ANT.xinfo", &pXinfoData, &iXinfoDataSize);

	unsigned int iTileMaterialWeightSize = sizeof(unsigned int) * GE_TRTILE_VERTEXSTRIDE * GE_TRTILE_VERTEXSTRIDE;
	ASSERT(iXinfoDataSize == ((sizeof(unsigned short) + iTileMaterialWeightSize) * iDataTileCount));

	//
	unsigned short* pXinfoMaterialMask = (unsigned short*)GE_MALLOC(sizeof(unsigned short) * iTileCount);
	GEMemCopy(pXinfoMaterialMask, (GEU8*)pXinfoData, sizeof(unsigned short) * iTileCount);

	m_oData[VertexWeightLayout_Xinfo].m_pWeightArray = (unsigned int*)GE_MALLOC(iTileMaterialWeightSize * iTileCount);
	GEMemCopy(m_oData[VertexWeightLayout_Xinfo].m_pWeightArray, (GEU8*)pXinfoData + (sizeof(unsigned short) * iDataTileCount), iTileMaterialWeightSize * iTileCount);

	Util::FreeReadBuffer(pXinfoData);

	//
	for (unsigned int iTile = 0; iTile < iTileCount; ++iTile)
	{
		pXinfoMaterialMask[iTile] = Util::EndianSwapValue(pXinfoMaterialMask[iTile]);

		unsigned int iElementPerTile = GE_TRTILE_VERTEXSTRIDE * GE_TRTILE_VERTEXSTRIDE;
		for (unsigned int iOffset = 0; iOffset < iElementPerTile; ++iOffset)
		{
			unsigned int iArrayOffset = (iTile * iElementPerTile) + iOffset;
			unsigned int iWeight = m_oData[VertexWeightLayout_Xinfo].m_pWeightArray[iArrayOffset];
			iWeight = Util::EndianSwapValue(iWeight);
			m_oData[VertexWeightLayout_Xinfo].m_pWeightArray[iArrayOffset] = iWeight;
		}
	}


	//
	unsigned int iCountMaterialType = CountXinfoMaterialData(pXinfoMaterialMask, iTileCount);

	m_oData[VertexWeightLayout_Xinfo].m_pMaterialDataArray			= (MaterialData*)malloc(sizeof(MaterialData) * iCountMaterialType);
	m_oData[VertexWeightLayout_Xinfo].m_iMaterialDataArraySize		= iCountMaterialType;

	m_oData[VertexWeightLayout_Xinfo].m_pTileMaterialDataIndexArray	= (GEUInt*)malloc(sizeof(GEUInt) * iTileCount);

	//
	CreateMaterialInfoArrayFromXinfo(pXinfoMaterialMask, iTileCount);

	free(pXinfoMaterialMask);	

	m_iHeightmapTileDimension		= 256;
	m_iHeightmapVertexDimension		= 1024;
}

void PatchQuadTree::FromSolverResult(const char* a_szSolverFile, GEUInt a_iBaseDimension, GEUInt a_iMaterialTypeCount)
{
	//
	unsigned int iTileCount = s_iDimension * s_iDimension;
	unsigned int iDataTileCount = s_iDimension * s_iDimension;

	unsigned int iDataLodLevelCount = 4;

	for (unsigned int iLodLevel = 1; iLodLevel < iDataLodLevelCount; ++iLodLevel)
	{
		iDataTileCount += (s_iDimension >> iLodLevel) * (s_iDimension >> iLodLevel);

		if (iLodLevel == (s_iLodLevelCount - 1))
		{
			iTileCount = iDataTileCount;
		}
	}

	//
	void* pSolverData = NULL;
	unsigned int iSolverDataSize = 0;
	Util::ReadFile(a_szSolverFile, &pSolverData, &iSolverDataSize);

	GEUInt iWeightSize				= sizeof(GEU32) * a_iBaseDimension * a_iBaseDimension;
	GEUInt iMaterialTypeSize		= a_iMaterialTypeCount * sizeof(GEU8) * g_iMaxMaterialPerTile;
	GEUInt iMaterialTypeIndexSize	= sizeof(GEU16) * iDataTileCount;

	GE_ASSERT(iSolverDataSize == (iWeightSize + iMaterialTypeSize + iMaterialTypeIndexSize));

	m_oData[VertexWeightLayout_Solver].m_pWeightArray		= (GEU32*)malloc(sizeof(GEU32) * (a_iBaseDimension + 1) * (a_iBaseDimension + 1)),
	m_oData[VertexWeightLayout_Solver].m_iWeightArraySize	= a_iBaseDimension * a_iBaseDimension;

	for (unsigned int iY = 0; iY < a_iBaseDimension + 1; ++iY)
	{
		for (unsigned int iX = 0; iX < a_iBaseDimension + 1; ++iX)
		{
			GEUInt iHeightmapPosX = Util::Min(iX, a_iBaseDimension - 1);
			GEUInt iHeightmapPosY = Util::Min(iY, a_iBaseDimension - 1);
		
			GEU32 u32Weight = ((GEU32*)pSolverData)[(iHeightmapPosY * a_iBaseDimension) + iHeightmapPosX];

			u32Weight = Util::EndianSwapValue(u32Weight);
			m_oData[VertexWeightLayout_Solver].m_pWeightArray[(iY * (a_iBaseDimension + 1)) + iX] = u32Weight;
		}
	}

	//
	m_oData[VertexWeightLayout_Solver].m_pMaterialDataArray			= (MaterialData*)malloc(sizeof(MaterialData) * a_iMaterialTypeCount);
	m_oData[VertexWeightLayout_Solver].m_iMaterialDataArraySize		= a_iMaterialTypeCount;

	GEU8* pSrcMaterialData = (GEU8*)pSolverData + m_oData[VertexWeightLayout_Solver].m_iWeightArraySize * sizeof(GEU32);

	for (unsigned int iType = 0; iType < a_iMaterialTypeCount; ++iType)
	{
		GEUInt iSrcMaterialDataIndex = iType * g_iMaxMaterialPerTile;
		GEU32 u32Mask = 0;
		for (unsigned int iSlotIndex = 0; iSlotIndex < g_iMaxMaterialPerTile; ++iSlotIndex)
		{
			GEUInt iMaterialIndex = pSrcMaterialData[iSrcMaterialDataIndex + iSlotIndex];

			if (iMaterialIndex != g_iInvalidMaterialIndex)
			{
				u32Mask |= 1 << iMaterialIndex;
			}

			m_oData[VertexWeightLayout_Solver].m_pMaterialDataArray[iType].m_arrMaterialIndex[iSlotIndex] = iMaterialIndex;
		}

		m_oData[VertexWeightLayout_Solver].m_pMaterialDataArray[iType].m_iMask = u32Mask;
	}

	m_oData[VertexWeightLayout_Solver].m_pTileMaterialDataIndexArray		= (GEUInt*)malloc(sizeof(GEUInt) * iTileCount);
	m_oData[VertexWeightLayout_Solver].m_pTileMaterialDataIndexArraySize	= iTileCount;

	GEU16* pSrcTileMaterialTypeIndex = (GEU16*)((GEU8*)pSolverData + iWeightSize + iMaterialTypeSize);
	for (unsigned int iTile = 0; iTile < iTileCount; ++iTile)
	{
		m_oData[VertexWeightLayout_Solver].m_pTileMaterialDataIndexArray[iTile] = pSrcTileMaterialTypeIndex[iTile];		
	}

	Util::FreeReadBuffer(pSolverData);


	m_iHeightmapTileDimension		= a_iBaseDimension / 4;
	m_iHeightmapVertexDimension		= a_iBaseDimension;
}

void PatchQuadTree::CreateResources()
{
	// Create the vertex buffer.	
	Gfx::CreateVertexBuffer( sizeof(PatchVertex), s_iTileListCapacity * 25, &m_pTileVertexBuffer);
	Gfx::CreateIndexBuffer( Gfx::IndexType_32, s_iTileListCapacity * 24 * 4, &m_pTileIndexBuffer);

	Gfx::CreatePixelShaderFromFile("media\\shader.hlsl", "PSTerrain", &m_pPixelShader);
	Gfx::CreateVertexShaderFromFile("media\\shader.hlsl", "VSTerrain", &m_pVertexShader);

	Gfx::CreateInputLayout( g_arrVertexElement, g_iVertexElementCount, m_pVertexShader, &m_pInputLayout);
	
	Gfx::CreateConstantBuffer( 16, &m_pConstantBuffer );
	Gfx::CreateConstantBuffer( 16, &m_pConstantBufferTextureScale );



	//
	Util::ReadFile("media\\ANT.hmap", (void**)&m_pHeightmapData, &m_iHeightmapDataSize);
	assert(m_iHeightmapDataSize == (sizeof(HeightMapData) * s_iTileDimension * s_iDimension * s_iTileDimension * s_iDimension));

	for (int i = 0; i < s_iTileDimension * s_iDimension * s_iTileDimension * s_iDimension; ++i)
	{
		HeightMapData* pData = &m_pHeightmapData[i];

		//
		unsigned int u16NormalX = (pData->m_u32Normal >> 16) & 0xFFFF;
		unsigned int u16NormalZ = (pData->m_u32Normal >>  0) & 0xFFFF;

		u16NormalX = Util::EndianSwapValue((unsigned short)u16NormalX);
		u16NormalZ = Util::EndianSwapValue((unsigned short)u16NormalZ);

		pData->m_u32Normal = (u16NormalX << 16) | u16NormalZ;

		pData->m_u16Height = Util::EndianSwapValue(pData->m_u16Height);

	}


	//FromXinfo("media\\ANT.xinfo");
	FromSolverResult("media\\result_ANT.hmap_crop_0_0_1025_1025_e_30307_c_1827.bin", 1024, 1827);

	m_eVertexWeightLayout = VertexWeightLayout_Solver;

	//
	m_iDrawBatchListCapacity		= 2048;
	m_pDrawBatchList				= (DrawBatch*)malloc(sizeof(DrawBatch) * m_iDrawBatchListCapacity);
	m_iDrawBatchListSize			= 0;


	//
	for (int iTexture = 0; iTexture < s_iMaxTextureCount; ++iTexture)
	{
		const char* szTextureName = s_pTextureName[iTexture];

		if (szTextureName != NULL)
		{
			Gfx::CreateTextureFromFile(szTextureName, &m_pTexture[iTexture]);
		}
	}
	Gfx::CreateTextureFromFile("media\\imap.dds", &m_pIndirectMap);


	m_fPositionScale	= 1.0f;
	m_fHeightScale		= 1.0f;

	m_bWireFrameMode		= false;
	m_bUseDebugPixelShader	= false;
}




void PatchQuadTree::Initialize()
{
	CreateResources();
	m_oTileList.Initialize(s_iTileListCapacity, m_oData[m_eVertexWeightLayout].m_pTileMaterialDataIndexArray, s_iLodLevelCount, m_iHeightmapTileDimension);
}





void PatchQuadTree::SetWorldScale(float a_fPositionScale, float a_fHeightScale, float a_fSlopeScale)
{
	m_fPositionScale	= a_fPositionScale;
	m_fHeightScale		= a_fHeightScale;
	m_fSlopeScale		= a_fSlopeScale;
}



void PatchQuadTree::Update(float a_fPosX, float a_fPosY, float a_fPosZ, float a_fYaw, float a_fPitch)
{

	Util::Timer oTimer;
	oTimer.Reset();

	//
	int iWorldPosX = Util::Clamp((int)(a_fPosX / m_fPositionScale), 0, 1024);
	int iWorldPosY = Util::Clamp((int)(a_fPosZ / m_fPositionScale), 0, 1024);


	m_oTileList.Update(iWorldPosX, iWorldPosY); // iWorldPosX, iWorldPosY);

	CreateDrawData(m_oTileList.GetList(), m_oTileList.GetCount());

	//
	m_fUpdateTime = (float)oTimer.GetElapsedTime();	

	m_iVisibleTileCount = m_oTileList.GetCount();	



	D3DXMATRIX mProjection;
	D3DXMatrixPerspectiveFovLH(&mProjection, (FLOAT)(D3DX_PI * 0.5), 1.0f, 1000.0f, 1000000.0f);

	D3DXMATRIX mView, mTranslation, mRotation, mRotationX, mRotationY;

	//D3DXVECTOR3 vEye(a_fPosX, a_fPosY, a_fPosZ);
	//D3DXVECTOR3 vAt(vEye.x + 1.0f, vEye.y + 0.0f, vEye.z + 0.0f);
	//D3DXVECTOR3 vUp(0.0f, 1.0f, 0.0f);
	//D3DXMatrixLookAtLH(&mView, &vEye, &vAt, &vUp);

	D3DXMatrixTranslation(&mTranslation, -a_fPosX, -a_fPosY, -a_fPosZ);
	D3DXMatrixRotationX(&mRotationX, a_fPitch);
	D3DXMatrixRotationY(&mRotationY, a_fYaw);
	D3DXMatrixMultiply(&mRotation, &mRotationY, &mRotationX);
	D3DXMatrixMultiply(&mView, &mTranslation, &mRotation);
	
	D3DXMATRIX mTransform;
	D3DXMatrixMultiply(&mTransform, &mView, &mProjection);


	//
	Gfx::LockInfo oLock;
	Gfx::LockConstantBuffer(m_pConstantBuffer, &oLock);

	memcpy(oLock.m_pData, &mTransform, sizeof(D3DXMATRIX));

	Gfx::UnlockConstantBuffer(m_pConstantBuffer, &oLock);
}




unsigned int PatchQuadTree::CreateTileVertexData(eSTerrainTileData* __restrict a_pTile, PatchVertex* a_pVertexBuffer, unsigned int a_iVertexIndex)
{
	GEUInt		iTileLodLevel	= a_pTile->asStruct.m_iLodLevel;
	GEUInt		iTilePosX		= a_pTile->asStruct.m_iPosX;
	GEUInt		iTilePosY		= a_pTile->asStruct.m_iPosY;

	GEUInt		iVertexCountPerTile = (s_iTileDimension + 1) * (s_iTileDimension + 1);
	PatchVertex* __restrict	pVertex	= &a_pVertexBuffer[a_iVertexIndex];
	
	
	GEUInt		iTileScale	= 1 << a_pTile->asStruct.m_iLodLevel;
	GEUInt		iPosX		= iTilePosX * iTileScale * s_iTileDimension;
	GEUInt		iPosY		= iTilePosY * iTileScale * s_iTileDimension;

	GEFloat		fTileScale	= (GEFloat)iTileScale * m_fPositionScale;
	GEFloat		fPosX		= (GEFloat)iPosX * m_fPositionScale;
	GEFloat		fPosY		= (GEFloat)iPosY * m_fPositionScale;


	GEInt		iCurrentPosX	= iPosX;
	GEInt		iCurrentPosY	= iPosY;

	GEFloat		fCurrentPosX	= fPosX;
	GEFloat		fCurrentPosY	= fPosY;


	for (unsigned int iOffsetY = 0; iOffsetY < (s_iTileDimension + 1); ++iOffsetY)
	{
		iCurrentPosX = iPosX;
		fCurrentPosX = fPosX;

		for (unsigned int iOffsetX = 0; iOffsetX < (s_iTileDimension + 1); ++iOffsetX)
		{
			HeightMapData* pData = GetHeightMapDataAt(iCurrentPosX, iCurrentPosY);

			pVertex->m_fPositionX		= fCurrentPosX;
			pVertex->m_fPositionY		= (float)pData->m_u16Height * m_fHeightScale;
			pVertex->m_fPositionZ		= fCurrentPosY;

			pVertex->m_fTexcoordX		= (float)iCurrentPosX; //(float)iCurrentPosX / 1024.0f;
			pVertex->m_fTexcoordY		= (float)iCurrentPosY; //(float)iCurrentPosY / 1024.0f;


			pVertex->m_u32Normal		= pData->m_u32Normal;

			pVertex++;


			iCurrentPosX += iTileScale;
			fCurrentPosX += fTileScale;
		}

		iCurrentPosY += iTileScale;
		fCurrentPosY += fTileScale;
	}

	// reset vertex pointer
	pVertex	= &a_pVertexBuffer[a_iVertexIndex];

	switch (m_eVertexWeightLayout)
	{
		case VertexWeightLayout_Xinfo:
		{
			GEUInt		iBaseWeightOffset	= m_oTileList.GetLodArrayOffset(iTileLodLevel);
			GEUInt		iLodDimension		= m_oTileList.GetLodArrayDimension(iTileLodLevel);

			GEUInt		iWeightOffset		= (iBaseWeightOffset + (iLodDimension * iTilePosY) + iTilePosX) * (GE_TRTILE_VERTEXSTRIDE * GE_TRTILE_VERTEXSTRIDE);
			GEU32*		pWeight				= &m_oData[VertexWeightLayout_Xinfo].m_pWeightArray[iWeightOffset];

			for (unsigned int iOffsetY = 0; iOffsetY < (s_iTileDimension + 1); ++iOffsetY)
			{
				for (unsigned int iOffsetX = 0; iOffsetX < (s_iTileDimension + 1); ++iOffsetX)
				{
					pVertex->m_u32Weight = pWeight[(iOffsetY * GE_TRTILE_VERTEXSTRIDE) + iOffsetX];
					pVertex++;
				}
			}
			break;
		}

		case VertexWeightLayout_Solver:
		{
			GEInt		iCurrentPosX	= iPosX;
			GEInt		iCurrentPosY	= iPosY;

			//
			GEU32 u32WeightValue[g_iMaxMaterialPerTile] = { 0, 0, 0, 0, 0 };

			for (unsigned int iOffsetY = 0; iOffsetY < (s_iTileDimension + 1); ++iOffsetY)
			{
				iCurrentPosX = iPosX;

				for (unsigned int iOffsetX = 0; iOffsetX < (s_iTileDimension + 1); ++iOffsetX)
				{
					GEU32 u32Weight = m_oData[VertexWeightLayout_Solver].m_pWeightArray[(iCurrentPosY * (m_iHeightmapVertexDimension + 1)) + iCurrentPosX];

					//
					GEU32 u32Weight0 = (u32Weight >>  0) & 0xFF;
					GEU32 u32Weight1 = (u32Weight >>  8) & 0xFF;
					GEU32 u32Weight2 = (u32Weight >> 16) & 0xFF;
					GEU32 u32Weight3 = (u32Weight >> 24) & 0xFF;
					GEU32 u32Weight4 = 0xFF - (u32Weight0 + u32Weight1 + u32Weight2 + u32Weight3);

					u32WeightValue[0] += u32Weight0;
					u32WeightValue[1] += u32Weight1;
					u32WeightValue[2] += u32Weight2;
					u32WeightValue[3] += u32Weight3;
					u32WeightValue[4] += u32Weight4;

					pVertex->m_u32Weight = u32Weight;
					pVertex++;

					iCurrentPosX += iTileScale;
				}

				iCurrentPosY += iTileScale;
			}

			//
			GEUInt iTileIndex = m_oTileList.ComputeTileIndex(iTileLodLevel, iTilePosX, iTilePosY);
			unsigned int iMaterialTypeIndex = m_oData[VertexWeightLayout_Solver].m_pTileMaterialDataIndexArray[iTileIndex];
			MaterialData* pTileMaterialData = &m_oData[VertexWeightLayout_Solver].m_pMaterialDataArray[iMaterialTypeIndex];

			for (unsigned int iSlot = 0; iSlot < g_iMaxMaterialPerTile; ++iSlot)
			{
				if (u32WeightValue[iSlot] > 0)
				{
					ASSERT(pTileMaterialData->m_arrMaterialIndex[iSlot] != g_iInvalidMaterialIndex);
				}
			}

			break;
		}

		default:
			GE_ASSERT(0);
	}


	return iVertexCountPerTile;
}




unsigned int CopyIndexArray(unsigned int* a_pDest, unsigned int a_iOffset, unsigned int a_iMask)
{
	GE_ASSERT(a_iMask < 16);

	unsigned int*	pSrcArray		= g_arrSubTileIndexInfo[a_iMask].m_pArray;
	unsigned int	iSrcArraySize	= g_arrSubTileIndexInfo[a_iMask].m_iArraySize;

	ASSERT(pSrcArray != NULL);


	for (unsigned int i = 0; i < iSrcArraySize; ++i)
	{
		*a_pDest = pSrcArray[i] + a_iOffset;
		a_pDest++;
	}

	return iSrcArraySize;
}

unsigned int PatchQuadTree::CreateTileIndexData(eSTerrainTileData* __restrict a_pTile, unsigned int* a_pIndexBuffer, unsigned int a_iIndexStart, unsigned int a_iVertexOffset)
{
	unsigned int iTileLodLevel	= a_pTile->asStruct.m_iLodLevel;
	unsigned int iTilePosX		= a_pTile->asStruct.m_iPosX;
	unsigned int iTilePosY		= a_pTile->asStruct.m_iPosY;

	unsigned int iLevelDimension		= m_oTileList.GetLodArrayDimension(iTileLodLevel);
	unsigned int iLevelDimensionMask	= iLevelDimension - 1;

	unsigned int iLeftNeighborIndex		= m_oTileList.ComputeTileIndex(iTileLodLevel, (iTilePosX - 1) & iLevelDimensionMask, iTilePosY);
	unsigned int iRightNeighborIndex	= m_oTileList.ComputeTileIndex(iTileLodLevel, (iTilePosX + 1) & iLevelDimensionMask, iTilePosY);
	unsigned int iTopNeighborIndex		= m_oTileList.ComputeTileIndex(iTileLodLevel, iTilePosX, (iTilePosY - 1) & iLevelDimensionMask);
	unsigned int iBottomNeighborIndex	= m_oTileList.ComputeTileIndex(iTileLodLevel, iTilePosX, (iTilePosY + 1) & iLevelDimensionMask);

	const eSTerrainTileData*	pLeftNeighbor	= m_oTileList.GetTile(iLeftNeighborIndex);
	const eSTerrainTileData*	pRightNeighbor	= m_oTileList.GetTile(iRightNeighborIndex);
	const eSTerrainTileData*	pTopNeighbor	= m_oTileList.GetTile(iTopNeighborIndex);
	const eSTerrainTileData*	pBottomNeighbor	= m_oTileList.GetTile(iBottomNeighborIndex);


	bool		bLeftNeighborDisabled	= pLeftNeighbor->asStruct.m_eState == TileState_Disabled;
	bool		bRightNeighborDisabled	= pRightNeighbor->asStruct.m_eState == TileState_Disabled;
	bool		bTopNeighborDisabled	= pTopNeighbor->asStruct.m_eState == TileState_Disabled;
	bool		bBottomNeighborDisabled	= pBottomNeighbor->asStruct.m_eState == TileState_Disabled;

	//
	unsigned int	iMaskLeft			= bLeftNeighborDisabled ?		SubTileMask_Left	: 0;
	unsigned int	iMaskRight			= bRightNeighborDisabled ?		SubTileMask_Right	: 0;
	unsigned int	iMaskTop			= bTopNeighborDisabled ?		SubTileMask_Top		: 0;
	unsigned int	iMaskBottom			= bBottomNeighborDisabled ?		SubTileMask_Bottom	: 0;


	unsigned int	iTileIndexCount		= 0;
	unsigned int*	pDstIndexBuffer		= a_pIndexBuffer + a_iIndexStart;

	iTileIndexCount += CopyIndexArray(pDstIndexBuffer + iTileIndexCount, a_iVertexOffset + 0, iMaskLeft | iMaskTop);
	iTileIndexCount += CopyIndexArray(pDstIndexBuffer + iTileIndexCount, a_iVertexOffset + 2, iMaskRight | iMaskTop);
	iTileIndexCount += CopyIndexArray(pDstIndexBuffer + iTileIndexCount, a_iVertexOffset + 10, iMaskLeft | iMaskBottom);
	iTileIndexCount += CopyIndexArray(pDstIndexBuffer + iTileIndexCount, a_iVertexOffset + 12, iMaskRight | iMaskBottom);

	return iTileIndexCount;
}


void PatchQuadTree::CreateDrawData(eSTerrainTileData** __restrict a_pVisibleTile, unsigned int a_iVisibleTileCount)
{
	PatchVertex*	pVertexBuffer		= NULL;
	unsigned int*	pIndexBuffer		= NULL;

	Gfx::LockInfo oVertexLock;
	Gfx::LockInfo oIndexLock;

	Gfx::LockVertexBuffer(m_pTileVertexBuffer, &oVertexLock);
	Gfx::LockIndexBuffer(m_pTileIndexBuffer, &oIndexLock);

	pVertexBuffer = (PatchVertex*)oVertexLock.m_pData;
	pIndexBuffer = (unsigned int*)oIndexLock.m_pData;


	unsigned int iTotalIndexCount = 0;
	unsigned int iTotalVertexCount = 0;

	//
	static int g_iLodDebug = -1;
	static int g_iTileStartIndex	= -1;
	static int g_iTileCount			= -1;


	DrawBatch* pCurrentDrawBatch = &m_pDrawBatchList[0];

	m_iDrawBatchListSize = 0;

	pCurrentDrawBatch->m_iMaterialType		= a_pVisibleTile[0]->asStruct.m_iMaterialType;
	pCurrentDrawBatch->m_iIndexStart		= 0;
	pCurrentDrawBatch->m_iIndexCount		= 0;

	
	

	int iDrawCount = 0;

	for (unsigned int iTileIndex = 0; iTileIndex < a_iVisibleTileCount; ++iTileIndex)
	{
		eSTerrainTileData* __restrict pTile = a_pVisibleTile[iTileIndex];		


		if ((g_iLodDebug != -1) && (pTile->asStruct.m_iLodLevel != g_iLodDebug))
			continue;	
		if (g_iTileStartIndex >= 0)
		{
			iDrawCount++;

			if (iDrawCount > g_iTileStartIndex)
			{
				if (iDrawCount > (g_iTileStartIndex + g_iTileCount))
					break;
			}
			else
			{			
				continue;
			}
		}

		if (pTile->asStruct.m_iMaterialType != pCurrentDrawBatch->m_iMaterialType)
		{
			ASSERT(pCurrentDrawBatch->m_iIndexCount > 0);

			ASSERT(m_iDrawBatchListSize < m_iDrawBatchListCapacity);

			//
			pCurrentDrawBatch++;
			m_iDrawBatchListSize++;
			

			//
			pCurrentDrawBatch->m_iMaterialType		= pTile->asStruct.m_iMaterialType;
			pCurrentDrawBatch->m_iIndexStart		= iTotalIndexCount;
			pCurrentDrawBatch->m_iIndexCount		= 0;
		}
	

		unsigned int iTileVertexCount = CreateTileVertexData(pTile, pVertexBuffer, iTotalVertexCount);
		unsigned int iTileIndexCount = CreateTileIndexData(pTile, pIndexBuffer, iTotalIndexCount, iTotalVertexCount);

		//
		pCurrentDrawBatch->m_iIndexCount += iTileIndexCount;

		iTotalIndexCount += iTileIndexCount;
		iTotalVertexCount += iTileVertexCount;		
	}

	
	//
	if (pCurrentDrawBatch->m_iIndexCount > 0)
	{
		m_iDrawBatchListSize++;
	}



	//
	Gfx::UnlockVertexBuffer(m_pTileVertexBuffer, &oVertexLock);
	Gfx::UnlockIndexBuffer(m_pTileIndexBuffer, &oIndexLock);
	
}

void PatchQuadTree::SetTextureForMaterialType(const MaterialData* a_pMaterialData)
{
	//
	unsigned int iTextureSlotIndex = 0;
	
	float pConstantData[g_iMaxMaterialPerTile][4];
	memset(pConstantData, 0, sizeof(pConstantData));

	for (unsigned int iSlotIndex = 0; iSlotIndex < g_iMaxMaterialPerTile; ++iSlotIndex)
	{
		unsigned int iMaterialIndex = a_pMaterialData->m_arrMaterialIndex[iSlotIndex];
		if (iMaterialIndex != g_iInvalidMaterialIndex)
		{
			Gfx::SetSamplerTexture(1 + iSlotIndex, m_pTexture[iMaterialIndex]);

			pConstantData[iSlotIndex][0] = s_pTextureScale[iMaterialIndex].m_fScaleX;
			pConstantData[iSlotIndex][1] = s_pTextureScale[iMaterialIndex].m_fScaleY;
		}
		else
		{
			Gfx::SetSamplerTexture(1 + iSlotIndex, NULL);
			pConstantData[iSlotIndex][0] = 0.0f;
			pConstantData[iSlotIndex][1] = 0.0f;
		}
	}

	//
	Gfx::LockInfo oLock;
	Gfx::LockConstantBuffer(m_pConstantBufferTextureScale, &oLock);
	memcpy(oLock.m_pData, &pConstantData, sizeof(pConstantData));
	Gfx::UnlockConstantBuffer(m_pConstantBufferTextureScale, &oLock);

	Gfx::SetConstantBuffer(1, m_pConstantBufferTextureScale);	

}

void PatchQuadTree::Draw()
{
	Gfx::SetVertexShader(m_pVertexShader);
	Gfx::SetPixelShader(m_pPixelShader);
	Gfx::SetVertexBuffer(0, m_pTileVertexBuffer);
	Gfx::SetInputLayout(m_pInputLayout);
	Gfx::SetInputPrimitive(Gfx::InputPrimitive_TriangleList);
	Gfx::SetConstantBuffer(0, m_pConstantBuffer);
	
	Gfx::SetSamplerTexture(0, m_pIndirectMap);	

	Gfx::SetSamplerConfiguration(0, Gfx::SamplerFilter_Bilinear, Gfx::SamplerAddressMode_Clamp);
	Gfx::SetSamplerConfiguration(1, Gfx::SamplerFilter_Bilinear, Gfx::SamplerAddressMode_Wrap);


	for (unsigned int iDrawBatchIndex = 0; iDrawBatchIndex < m_iDrawBatchListSize; ++iDrawBatchIndex)
	{
		DrawBatch* pDrawBatch = &m_pDrawBatchList[iDrawBatchIndex];

		MaterialData* pMaterialData = &m_oData[m_eVertexWeightLayout].m_pMaterialDataArray[pDrawBatch->m_iMaterialType];
		SetTextureForMaterialType(pMaterialData);

		Gfx::DrawIndexedVertices(m_pTileIndexBuffer, pDrawBatch->m_iIndexStart, pDrawBatch->m_iIndexCount);
	}	
}

HeightMapData* PatchQuadTree::GetHeightMapDataAt(unsigned int a_iPosX, unsigned int a_iPosY)
{
	a_iPosX = Util::Clamp(a_iPosX, 0u, s_iTileDimension * s_iDimension - 1);
	a_iPosY = Util::Clamp(a_iPosY, 0u, s_iTileDimension * s_iDimension - 1);

	return &m_pHeightmapData[(a_iPosY * s_iTileDimension * s_iDimension) + a_iPosX];
}

unsigned int PatchQuadTree::GetHeightAt(unsigned int a_iPosX, unsigned int a_iPosY)
{
	return m_pHeightmapData[(a_iPosY * s_iTileDimension * s_iDimension) + a_iPosX].m_u16Height;
}



unsigned int PatchQuadTree::GetMaterialAt(unsigned int a_iPosX, unsigned int a_iPosY)
{
	return 0; //m_pHeightmapData[(a_iPosY * s_iDimension * s_iTileDimension) + a_iPosX].m_u8MaterialIndex;
}

unsigned int PatchQuadTree::GetFlagsAt(unsigned int a_iPosX, unsigned int a_iPosY)
{
	return 0; //m_pHeightmapData[(a_iPosY * s_iDimension * s_iTileDimension) + a_iPosX].m_u8Flags;
}

