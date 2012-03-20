
#include "StdAfx.h"
#include "TerrainMaterialCSP.h"

const unsigned int iArrayDimension		= 1025;
const unsigned int iLodLevelCount		= 4;

const unsigned int iTileDimension = 256;


const unsigned int g_iHeightMapCropThreshold = 16;

eSMaterialData* GetTerrainDataAt( eSMaterialData * a_pData, unsigned int a_iWidth, unsigned int a_iHeight, unsigned int a_iX, unsigned int a_iY)
{
	unsigned int iX = Util::Min(a_iX, a_iWidth - 1);
	unsigned int iY = Util::Min(a_iY, a_iHeight - 1);
	return &a_pData[(iY * a_iWidth) + iX];
}



unsigned int CropColumn( eSMaterialData * a_pData, unsigned int a_iWidth, unsigned int a_iHeight, unsigned int a_iStart, unsigned int a_iStep )
{
	unsigned int iX = a_iStart + a_iStep;
	while (true)
	{
		bool bCropColumn = true;
		for (unsigned int iY = 0; iY < a_iHeight; ++iY)
		{
			eSMaterialData* pTerrainDataBase = GetTerrainDataAt(a_pData, a_iWidth, a_iHeight, a_iStart, iY);
			eSMaterialData* pTerrainDataTest = GetTerrainDataAt(a_pData, a_iWidth, a_iHeight, iX, iY);

			bool bDiffHeight	= Util::Abs((int)pTerrainDataBase->m_iHeight - (int)pTerrainDataTest->m_iHeight) > g_iHeightMapCropThreshold;
			bool bDiffMaterial	= pTerrainDataBase->m_iMaterialIndex != pTerrainDataTest->m_iMaterialIndex;

			if (bDiffHeight || bDiffMaterial)
			{
				bCropColumn = false;
				break;
			}			
		}

		if (!bCropColumn)
			break;

		iX += a_iStep;
	}

	return iX - a_iStep;
}

unsigned int CropRow( eSMaterialData * a_pData, unsigned int a_iWidth, unsigned int a_iHeight, unsigned int a_iStart, unsigned int a_iStep )
{
	unsigned int iY = a_iStart + a_iStep;

	while (true)
	{
		bool bCropRow = true;

		for (unsigned int iX = 0; iX < a_iWidth ; ++iX)
		{
			eSMaterialData* pTerrainDataBase = GetTerrainDataAt(a_pData, a_iWidth, a_iHeight, iX, a_iStart);
			eSMaterialData* pTerrainDataTest = GetTerrainDataAt(a_pData, a_iWidth, a_iHeight, iX, iY);

			bool bDiffHeight	= Util::Abs((int)pTerrainDataBase->m_iHeight - (int)pTerrainDataTest->m_iHeight) > g_iHeightMapCropThreshold;
			bool bDiffMaterial	= pTerrainDataBase->m_iMaterialIndex != pTerrainDataTest->m_iMaterialIndex;

			if (bDiffHeight || bDiffMaterial)
			{
				bCropRow = false;
				break;
			}			
		}

		if (!bCropRow)
			break;

		iY += a_iStep;
	}

	return iY - a_iStep;
}

void CropHeightmap(eSMaterialData * a_pData, unsigned int a_iWidth, unsigned int a_iHeight)
{
	unsigned int iMinX	= 0;
	unsigned int iMaxX	= a_iWidth;

	unsigned int iMinY	= 0;
	unsigned int iMaxY	= a_iHeight;


	//
	iMinX = CropColumn(a_pData, a_iWidth, a_iHeight, 0, 1);
	iMaxX = CropColumn(a_pData, a_iWidth, a_iHeight, a_iWidth - 1, -1) + 1;

	iMinY = CropRow(a_pData, a_iWidth, a_iHeight, 0, 1);
	iMaxY = CropRow(a_pData, a_iWidth, a_iHeight, a_iHeight - 1, -1) + 1;
}


//
eSMaterialConvertionData* CreateFromXinfo(const char* a_sXinfoFile, unsigned int a_iTestStartX, unsigned int a_iTestStartY, unsigned int a_iTestSizeX, unsigned int a_iTestSizeY)
{
	void* pData = NULL;
	unsigned int iDataSize = 0;

	Util::ReadFile(a_sXinfoFile, (void**)&pData, &iDataSize);

	unsigned int iElementSize = sizeof(eSMaterialExtraInfo) + sizeof(unsigned short);
	ASSERT((iDataSize % iElementSize) == 0);

	unsigned int iDataTileCount = iDataSize / iElementSize;

	//
	unsigned short* pMaskData = (unsigned short*)pData;
	eSMaterialExtraInfo* pXinfoData = (eSMaterialExtraInfo*)(pMaskData + iDataTileCount);

	for (unsigned int iTileIndex = 0; iTileIndex < iDataTileCount; ++iTileIndex)
	{
		unsigned short u16SwappedMask = Util::EndianSwapValue(pMaskData[iTileIndex]);
		pMaskData[iTileIndex] = u16SwappedMask;

		eSMaterialExtraInfo* pXinfo = &pXinfoData[iTileIndex];

		for (unsigned int iTileOffsetY = 0; iTileOffsetY < GE_TRTILE_VERTEXSTRIDE; ++iTileOffsetY)
		{
			for (unsigned int iTileOffsetX = 0; iTileOffsetX < GE_TRTILE_VERTEXSTRIDE; ++iTileOffsetX)
			{
				unsigned int u32SwappedWeight = Util::EndianSwapValue(pXinfo->m_arrSplatMask[iTileOffsetY][iTileOffsetX].m_i);
				pXinfo->m_arrSplatMask[iTileOffsetY][iTileOffsetX].m_i = u32SwappedWeight;
			}
		}
	}


	//
	unsigned int iTestStartX	= a_iTestStartX;
	unsigned int iTestStartY	= a_iTestStartY;
	unsigned int iTestSizeX		= a_iTestSizeX;
	unsigned int iTestSizeY		= a_iTestSizeY;

	eSMaterialConvertionData* pTestData = (eSMaterialConvertionData*)malloc(sizeof(eSMaterialConvertionData) * iTestSizeX * iTestSizeY);

	unsigned int iFirstLevelTileDimension = 256;

	unsigned int iTileIndex = 0;

	for (unsigned int iY = 0; iY < iTestSizeY; ++iY)
	{
		for (unsigned int iX = 0; iX < iTestSizeX; ++iX)
		{
			unsigned int iTileX			= (iX + iTestStartX) / GE_TRTILE_TILESTRIDE;
			unsigned int iTileY			= (iY + iTestStartY) / GE_TRTILE_TILESTRIDE;

			unsigned int iTileOffsetX	= (iX + iTestStartX) % GE_TRTILE_TILESTRIDE;
			unsigned int iTileOffsetY	= (iY + iTestStartY) % GE_TRTILE_TILESTRIDE;


			unsigned int iSrcIndex			= (iTileY * iFirstLevelTileDimension) + iTileX;
			//unsigned int iTileWeightIndex	= (iTileOffsetY * GE_TRTILE_VERTEXSTRIDE) + iTileOffsetX;

			unsigned short			u16Mask		= pMaskData[iSrcIndex];
			eSMaterialExtraInfo*	pXinfo		= &pXinfoData[iSrcIndex];
			MaterialWeight*			pWeight		= &pXinfo->m_arrSplatMask[iTileOffsetY][iTileOffsetX];



			eSMaterialConvertionData* pConvertionData = &pTestData[(iY * iTestSizeX) + iX];			
			memset(pConvertionData, 0, sizeof(eSMaterialConvertionData));

			ASSERT(u16Mask != 0);

			unsigned int iMaterialCount = 0;

			while (u16Mask != 0)
			{
				ASSERT(iMaterialCount < g_iMaxMaterialPerTile);

				unsigned int iMaterialIndex = 31 - Util::CountLeadingZeros(u16Mask);
				unsigned int iWeight = 0;
				
				if (iMaterialCount < (g_iMaxMaterialPerTile - 1))
				{
					iWeight = pWeight->m_a[iMaterialCount];
				}
				else
				{
					iWeight = 0xFF;

					for (unsigned int i = 0; i < (g_iMaxMaterialPerTile - 1); ++i)
					{
						iWeight -= pWeight->m_a[i];
					}
				}

				if (iWeight > 0)
				{
					pConvertionData->m_arrMaterialWeight[iMaterialIndex] = 1;
				}
				
				//
				ASSERT((u16Mask & (1 << iMaterialIndex)) != 0);
				u16Mask &= ~(1 << iMaterialIndex);
				iMaterialCount++;
			}

			ASSERT(u16Mask == 0);
		}
	}

	Util::FreeReadBuffer(pData);
	pData = NULL;


	return pTestData;
}


eSMaterialConvertionData* CreateFromConvData(const char* a_sConvDataFile, unsigned int a_iTestStartX, unsigned int a_iTestStartY, unsigned int a_iTestSizeX, unsigned int a_iTestSizeY)
{
	eSMaterialConvertionData* pData = NULL;
	unsigned int iDataSize = 0;

	Util::ReadFile(a_sConvDataFile, (void**)&pData, &iDataSize);

	//
	unsigned int iTestStartX	= a_iTestStartX;
	unsigned int iTestStartY	= a_iTestStartY;
	unsigned int iTestSizeX		= a_iTestSizeX;
	unsigned int iTestSizeY		= a_iTestSizeY;

	eSMaterialConvertionData* pTestData = (eSMaterialConvertionData*)malloc(sizeof(eSMaterialConvertionData) * iTestSizeX * iTestSizeY);


	for (unsigned int iY = 0; iY < iTestSizeY; ++iY)
	{
		for (unsigned int iX = 0; iX < iTestSizeX; ++iX)
		{
			pTestData[(iY * iTestSizeX) + iX] = pData[((iY + iTestStartY) * iArrayDimension) + (iX + iTestStartX)];
		}
	}

	Util::FreeReadBuffer(pData);
	pData = NULL;

	return pTestData;
}


void GatherHmapData(eSMaterialConvertionData* a_pDstData, eSMaterialData* a_pData, unsigned int iHmapX, unsigned int iHmapY)
{
	unsigned int iDimension = 1024;
	int iAmplitude = 1;

	unsigned int iStartX = iHmapX == 0 ? 0 : (iHmapX - 1);
	unsigned int iEndX = iHmapX == (iDimension - 1) ? (iDimension - 1) : (iHmapX + 1);

	unsigned int iStartY = iHmapY == 0 ? 0 : (iHmapY - 1);
	unsigned int iEndY = iHmapY == (iDimension - 1) ? (iDimension - 1) : (iHmapY + 1);

	for (int iOffsetY = -iAmplitude; iOffsetY <= iAmplitude; ++iOffsetY)
	{
		for (int iOffsetX = -iAmplitude; iOffsetX <= iAmplitude; ++iOffsetX)
		{
			unsigned int iX = (iHmapX + iOffsetX) % iDimension;
			unsigned int iY = (iHmapY + iOffsetY) % iDimension;

			eSMaterialData* pData = &a_pData[(iY * iDimension) + iX];
			a_pDstData->m_arrMaterialWeight[pData->m_iMaterialIndex]++;			
		}
	}	
}



eSMaterialConvertionData* CreateFromHmapData(const char* a_sHmapFile, unsigned int a_iHmapWidth, unsigned int a_iHmapHeight, unsigned int a_iTestStartX, unsigned int a_iTestStartY, unsigned int a_iTestSizeX, unsigned int a_iTestSizeY)
{
	eSMaterialData* pData = NULL;
	unsigned int iDataSize = 0;

	Util::ReadFile(a_sHmapFile, (void**)&pData, &iDataSize);

	CropHeightmap(pData, a_iHmapWidth, a_iHmapHeight);

	//
	unsigned int iTestStartX	= a_iTestStartX;
	unsigned int iTestStartY	= a_iTestStartY;
	unsigned int iTestSizeX		= a_iTestSizeX;
	unsigned int iTestSizeY		= a_iTestSizeY;

	eSMaterialConvertionData* pTestData = (eSMaterialConvertionData*)malloc(sizeof(eSMaterialConvertionData) * iTestSizeX * iTestSizeY);
	memset(pTestData, 0, sizeof(eSMaterialConvertionData) * iTestSizeX * iTestSizeY);

	for (unsigned int iY = 0; iY < iTestSizeY; ++iY)
	{
		for (unsigned int iX = 0; iX < iTestSizeX; ++iX)
		{
			unsigned int iHmapX = iX + iTestStartX;
			unsigned int iHmapY = iY + iTestStartY;

			//
			eSMaterialConvertionData* pDstData = &pTestData[(iY * iTestSizeX) + iX];
			GatherHmapData(pDstData, pData, iHmapX, iHmapY);
		}
	}

	Util::FreeReadBuffer(pData);
	pData = NULL;

	return pTestData;
}

void WriteResult(TerrainMaterialCSP& oCSP, const char* a_szFile)
{
	const void* pBuffer;
	unsigned int iBufferSize;

	pBuffer = &oCSP.GetVertexWeightArray()[0];
	iBufferSize = oCSP.GetVertexWeightArray().size() * sizeof(oCSP.GetVertexWeightArray()[0]);
	Util::SaveFile(a_szFile, pBuffer, iBufferSize);

	pBuffer = &oCSP.GetTileMaterialTypeArray()[0];
	iBufferSize = oCSP.GetTileMaterialTypeArray().size() * sizeof(oCSP.GetTileMaterialTypeArray()[0]);
	Util::AppendFile(a_szFile, pBuffer, iBufferSize);

	pBuffer = &oCSP.GetTileMaterialTypeIndexArray()[0];
	iBufferSize = oCSP.GetTileMaterialTypeIndexArray().size() * sizeof(oCSP.GetTileMaterialTypeIndexArray()[0]);
	Util::AppendFile(a_szFile, pBuffer, iBufferSize);
}



void main()
{
	unsigned int iTestStartX	= 0;
	unsigned int iTestStartY	= 0;
	unsigned int iTestSizeX		= 1025;
	unsigned int iTestSizeY		= 1025;

	//unsigned int iTestStartX	= 256 + 32;
	//unsigned int iTestStartY	= 256 + 32;
	//unsigned int iTestSizeX		= 128 + 1;
	//unsigned int iTestSizeY		= 128 + 1;
 //

	eSMaterialConvertionData* pTestData;

	//pTestData = CreateFromXinfo("ANT.xinfo", iTestStartX, iTestStartY, iTestSizeX, iTestSizeY);
	//pTestData = CreateFromConvData("mat_conv.dat", iTestStartX, iTestStartY, iTestSizeX, iTestSizeY);

	const char* szSourceFile = "ANT.hmap";
	pTestData = CreateFromHmapData(szSourceFile, 1024, 1024, iTestStartX, iTestStartY, iTestSizeX, iTestSizeY);

	TerrainMaterialCSP oCSP;
	oCSP.Initialize(pTestData, iTestSizeX, iTestSizeY, iLodLevelCount);
	oCSP.Solve();	
	oCSP.CreateVertexWeightArray();
	oCSP.CreateTileMaterialData();

	//
	char szResultFileName[1024];
	sprintf_s(
		szResultFileName, 1024, 
		"result_%s_crop_%d_%d_%d_%d_e_%d_c_%d.bin", 
		szSourceFile, 
		iTestStartX, iTestStartY, iTestSizeX, iTestSizeY, 
		oCSP.GetSolutionCost(), oCSP.GetTileMaterialTypeArray().size());


	WriteResult(oCSP, szResultFileName);
}