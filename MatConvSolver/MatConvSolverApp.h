
#pragma once

#include "MicroSDK/Application.h"
#include "MaterialConvertionData.h"
#include "TerrainMaterialCSP.h"


const unsigned int iArrayDimension				= 1025;
const unsigned int iLodLevelCount				= 4;
const unsigned int iTileDimension				= 256;
const unsigned int g_iHeightMapCropThreshold	= 16;


class MatConvSolverApp : public MicroSDK::Application
{
public:
								MatConvSolverApp();
	virtual void				OnInitialize();


private:
	eSMaterialData*				GetTerrainDataAt( eSMaterialData * a_pData, unsigned int a_iWidth, unsigned int a_iHeight, unsigned int a_iX, unsigned int a_iY);
	void						GatherHmapData(eSMaterialConvertionData* a_pDstData, eSMaterialData* a_pData, unsigned int iHmapX, unsigned int iHmapY);

	unsigned int				CropColumn( eSMaterialData * a_pData, unsigned int a_iWidth, unsigned int a_iHeight, unsigned int a_iStart, unsigned int a_iStep );
	unsigned int				CropRow( eSMaterialData * a_pData, unsigned int a_iWidth, unsigned int a_iHeight, unsigned int a_iStart, unsigned int a_iStep );
	void						CropHeightmap(eSMaterialData * a_pData, unsigned int a_iWidth, unsigned int a_iHeight);

	eSMaterialConvertionData*	CreateFromXinfo(const char* a_sXinfoFile, unsigned int a_iTestStartX, unsigned int a_iTestStartY, unsigned int a_iTestSizeX, unsigned int a_iTestSizeY);
	eSMaterialConvertionData*	CreateFromConvData(const char* a_sConvDataFile, unsigned int a_iTestStartX, unsigned int a_iTestStartY, unsigned int a_iTestSizeX, unsigned int a_iTestSizeY);
	eSMaterialConvertionData*	CreateFromHmapData(const char* a_sHmapFile, unsigned int a_iHmapWidth, unsigned int a_iHmapHeight, unsigned int a_iTestStartX, unsigned int a_iTestStartY, unsigned int a_iTestSizeX, unsigned int a_iTestSizeY);

	void						WriteResult(TerrainMaterialCSP& oCSP, const char* a_szFile);

};
