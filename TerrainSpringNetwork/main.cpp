
#include "StdAfx.h"

// http://msdl.microsoft.com/download/symbols

#include "QuadTree.h"
#include "ForceField2DSimulator.h"
#include "HeightmapData.h"

Gfx::VertexShader*		pVertexShader	= NULL;
Gfx::PixelShader*		pPixelShader	= NULL;
Gfx::InputLayout*		pInputLayout	= NULL;
Gfx::VertexBuffer*		pVertexBuffer	= NULL;
Gfx::ConstantBuffer*	pConstantBuffer	= NULL;

PatchQuadTree			oQuadTree;
float					fQuadTreePosX = 0.0f;
float					fQuadTreePosY = 50.0f;
float					fQuadTreePosZ = 0.0f;

float					fCameraPitch	= 0.0f;
float					fCameraYaw		= 0.0f;


float					fOffsetScale	= 1.0f;
float					fShowOffset		= 0.0f;

float					fGridScale	= 1.0f;
float					fGridPosX	= -0.5f;
float					fGridPosY	= 0.5f;

Util::Timer				oFrameTimer;

enum QuadTreeActionId
{
	QuadTreeActionId_MoveForward		= 1,
	QuadTreeActionId_MoveBackward,
	QuadTreeActionId_MoveLeft,
	QuadTreeActionId_MoveRight,
	QuadTreeActionId_MoveUp,
	QuadTreeActionId_MoveDown,
	QuadTreeActionId_ViewLeft,
	QuadTreeActionId_ViewRight,
	QuadTreeActionId_ViewUp,
	QuadTreeActionId_ViewDown,

	QuadTreeActionId_Action1,
	QuadTreeActionId_Action2
};


//
struct Color
{
	float m_fRed;
	float m_fGreen;
	float m_fBlue;
};

//
struct Vertex
{
	ForceField2D::Vector2	m_vPosition;
	Color					m_vColor;
};

const unsigned int iInputElementCount = 2;
Gfx::InputLayoutElement	arrInputElement[iInputElementCount] =
{
	{ Gfx::InputElementSemantic_Position, 0, 0, Gfx::InputElementFormat_Float_2 },
	{ Gfx::InputElementSemantic_Color, 0, 8, Gfx::InputElementFormat_Float_3 },
};



const unsigned int g_iHeightmapDimension = 16;
HeightMapData g_arrHeightmap[g_iHeightmapDimension * g_iHeightmapDimension] =
{
	{ 8, 0, 0 }, { 8, 0, 0 }, { 8, 0, 0 }, { 8, 0, 0 }, { 8, 0, 0 }, { 8, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 },
	{ 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 },
	{ 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 },
	{ 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 },
	{ 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 },
	{ 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 },
	{ 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 },
	{ 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 },
	{ 0, 0, 0 }, { 9, 0, 0 }, { 8, 0, 0 }, { 7, 0, 0 }, { 6, 0, 0 }, { 5, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 5, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 },
	{ 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 },
	{ 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 },
	{ 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 },
	{ 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 },
	{ 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 },
	{ 8, 0, 0 }, { 8, 0, 0 }, { 8, 0, 0 }, { 8, 0, 0 }, { 8, 0, 0 }, { 8, 0, 0 }, { 8, 0, 0 }, { 8, 0, 0 }, { 8, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 },
	{ 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 },
};



HeightMapCropData					g_oCropData;
ForceField2D::Simulator				g_arrSimulator[2];
unsigned int						g_iSimulatorIndex		= 0;
bool								g_bSimulationRunning	= false;
unsigned int						g_iSimulationIteration	= 0;

//
void CropHeightmap(const HeightMapData* a_pHeightmap, unsigned int a_iHeightmapDimension, unsigned int a_iCropThreshold, HeightMapCropData* a_pCropData)
{
	bool	bZeroHeight = true;
	
	//
	unsigned int iStartX;
	unsigned int iEndX;
	unsigned int iStartY;
	unsigned int iEndY;


	a_pCropData->m_pHeightmap = a_pHeightmap;
	a_pCropData->m_iHeightmapDimension = a_iHeightmapDimension;

	if (a_iCropThreshold == 0)
	{
		a_pCropData->m_iStartX	= 0;
		a_pCropData->m_iStartY	= 0;
		a_pCropData->m_iSizeX	= a_iHeightmapDimension;
		a_pCropData->m_iSizeY	= a_iHeightmapDimension;
		return;
	}

	//
	iStartX = 0;
	bZeroHeight = true;

	while (bZeroHeight)
	{
		for (unsigned int iPosY = 0; iPosY < a_iHeightmapDimension; ++iPosY)
		{
			unsigned int iHeightmapArrayIndex = (iPosY * a_iHeightmapDimension) + iStartX;

			if (a_pHeightmap[iHeightmapArrayIndex].m_u16Height > a_iCropThreshold)
			{
				bZeroHeight = false;
				break;
			}
		}

		if (bZeroHeight)
		{
			iStartX++;
		}
	}

	//
	iEndX = a_iHeightmapDimension - 1;
	bZeroHeight = true;

	while (bZeroHeight)
	{
		for (unsigned int iPosY = 0; iPosY < a_iHeightmapDimension; ++iPosY)
		{
			unsigned int iHeightmapArrayIndex = (iPosY * a_iHeightmapDimension) + iEndX;

			if (a_pHeightmap[iHeightmapArrayIndex].m_u16Height > a_iCropThreshold)
			{
				bZeroHeight = false;
				break;
			}
		}

		if (bZeroHeight)
		{
			iEndX--;
		}
	}


	//
	iStartY = 0;
	bZeroHeight = true;

	while (bZeroHeight)
	{
		for (unsigned int iPosX = 0; iPosX < a_iHeightmapDimension; ++iPosX)
		{
			unsigned int iHeightmapArrayIndex = (iStartY * a_iHeightmapDimension) + iPosX;

			if (a_pHeightmap[iHeightmapArrayIndex].m_u16Height > a_iCropThreshold)
			{
				bZeroHeight = false;
				break;
			}
		}

		if (bZeroHeight)
		{
			iStartY++;
		}
	}

	//
	iEndY = a_iHeightmapDimension - 1;
	bZeroHeight = true;

	while (bZeroHeight)
	{
		for (unsigned int iPosX = 0; iPosX < a_iHeightmapDimension; ++iPosX)
		{
			unsigned int iHeightmapArrayIndex = (iEndY * a_iHeightmapDimension) + iPosX;

			if (a_pHeightmap[iHeightmapArrayIndex].m_u16Height > a_iCropThreshold)
			{
				bZeroHeight = false;
				break;
			}
		}

		if (bZeroHeight)
		{
			iEndY--;
		}
	}



	a_pCropData->m_iStartX = iStartX;
	a_pCropData->m_iStartY = iStartY;
	a_pCropData->m_iSizeX = iEndX + 1 - iStartX;
	a_pCropData->m_iSizeY = iEndY + 1 - iStartY;
}


void LoadHeightmapFromBuffer(
		HeightMapData* a_pHeightmap, unsigned int a_iDimension, unsigned a_iLog2BaseResolution, 
		float a_fHeightScale, unsigned int a_iCropThreshold)
{
	memset(&g_oCropData, 0, sizeof(g_oCropData));
	CropHeightmap(a_pHeightmap, a_iDimension, a_iCropThreshold, &g_oCropData);


	//
	unsigned int iLog2BaseSimulationResolution = a_iLog2BaseResolution; // 32
	unsigned int iBaseSimulationResolution = 1 << iLog2BaseSimulationResolution;
	unsigned int iMask = ~((1 << iLog2BaseSimulationResolution) - 1);

	g_oCropData.m_iStartX &= iMask;
	g_oCropData.m_iStartY &= iMask;
	g_oCropData.m_iSizeX = (g_oCropData.m_iSizeX + iBaseSimulationResolution - 1) & iMask;
	g_oCropData.m_iSizeY = (g_oCropData.m_iSizeY + iBaseSimulationResolution - 1) & iMask;


	g_iSimulationIteration = iLog2BaseSimulationResolution;
	g_iSimulatorIndex = 0;
	g_arrSimulator[g_iSimulatorIndex].Initialize(
		g_oCropData.m_iSizeX / iBaseSimulationResolution, g_oCropData.m_iSizeY / iBaseSimulationResolution,
		2.0f, a_fHeightScale, 5.0f,
		&g_oCropData, iBaseSimulationResolution);

	g_bSimulationRunning = true;
}

void LoadHeightmapFromFile(const char* a_szFile, unsigned int a_iDimension, unsigned int a_iLog2BaseResolution)
{
	unsigned int iBufferSize = 0;
	HeightMapData* pHeightmapBuffer;
	Util::ReadFile(a_szFile, (void**)&pHeightmapBuffer, &iBufferSize);	

	if (iBufferSize != (sizeof(HeightMapData) * a_iDimension * a_iDimension))
	{
		HALT_WITH_MSG("Invalid Heightmap buffer size");
	}

	for (unsigned int iVertexIndex = 0; iVertexIndex < (a_iDimension * a_iDimension); ++iVertexIndex)
	{
		pHeightmapBuffer[iVertexIndex].m_u16Height = Util::EndianSwapValue(pHeightmapBuffer[iVertexIndex].m_u16Height);
	}

	LoadHeightmapFromBuffer(pHeightmapBuffer, a_iDimension, a_iLog2BaseResolution, 0.15f / (100.0f * 1.0f), 100);
}

void UpdateConstantBuffer()
{
	unsigned int iDimensionX = g_arrSimulator[g_iSimulatorIndex].GetDimensionX();
	unsigned int iDimensionY = g_arrSimulator[g_iSimulatorIndex].GetDimensionY();

	float fBaseScale	= 0.75f * fGridScale;
	float fScaleX		= fBaseScale / iDimensionX;
	float fScaleY		= -fBaseScale / iDimensionY;

	float matrix[16] = 
	{
		fScaleX, 0.0f, 0.0f, 0.0f,
		0.0f, fScaleY, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		fGridPosX * fScaleX, fGridPosY * fScaleY, 0.0f, 1.0f,
	};

	Gfx::LockInfo oConstantLock;
	Gfx::LockConstantBuffer(pConstantBuffer, &oConstantLock);

	memcpy(oConstantLock.m_pData, matrix, sizeof(matrix));

	((float*)oConstantLock.m_pData)[16] = fOffsetScale;
	((float*)oConstantLock.m_pData)[17] = fOffsetScale;	
	((float*)oConstantLock.m_pData)[18] = fShowOffset;

	Gfx::UnlockConstantBuffer(pConstantBuffer, &oConstantLock);
}

void InitializeSimulation()
{
	//LoadHeightmapFromBuffer(g_arrHeightmap, g_iHeightmapDimension, 3, 1.0f, 0);
	LoadHeightmapFromFile("media\\ANT.hmap", 1024, 5);


	Gfx::CreateVertexShaderFromFile("media\\shader.hlsl", "VSTest", &pVertexShader);
	Gfx::CreatePixelShaderFromFile("media\\shader.hlsl", "PSTest", &pPixelShader);
	Gfx::CreateInputLayout(arrInputElement, iInputElementCount, pVertexShader, &pInputLayout);

	unsigned int iMaxSpringCount		= (g_oCropData.m_iSizeX * g_oCropData.m_iSizeY * ForceField2D::SpringType_Count) + (g_oCropData.m_iSizeX + g_oCropData.m_iSizeY);
	unsigned int iSpringVertexCount		= iMaxSpringCount * 2;
	Gfx::CreateVertexBuffer(sizeof(Vertex), iSpringVertexCount, &pVertexBuffer);
	
}

void InitializeQuadTree()
{
	oQuadTree.Initialize();
	oQuadTree.SetWorldScale(100.0f, 0.15f, 1.0f);
}


void UpdateForceFieldVertexBuffer()
{
	Gfx::LockInfo oLock;
	Gfx::LockVertexBuffer(pVertexBuffer, &oLock);

	Vertex* pDst = (Vertex*)oLock.m_pData;

	//
	const ForceField2D::Vector2*	pVertexPositionArray	= g_arrSimulator[g_iSimulatorIndex].GetVertexPositionArray();
	const ForceField2D::Spring*		pSpringArray			= g_arrSimulator[g_iSimulatorIndex].GetSpringArray();
	unsigned int 					iSpringCount			= g_arrSimulator[g_iSimulatorIndex].GetSpringCount();


	Color vColorRed = { 1.0f, 0.0f, 0.0f };
	Color vColorGreen = { 0.0f, 1.0f, 0.0f };
	Color vColorBlue = { 0.0f, 0.0f, 1.0f };

	Color vColor;

	//
	unsigned int iDstIndex = 0;
	for (unsigned int iSpringIndex = 0; iSpringIndex < iSpringCount; ++iSpringIndex)
	{
		const ForceField2D::Spring* pSpring = &pSpringArray[iSpringIndex];

		const ForceField2D::Vector2* pVertex0 = &pVertexPositionArray[pSpring->m_iVertexIndex0];
		const ForceField2D::Vector2* pVertex1 = &pVertexPositionArray[pSpring->m_iVertexIndex1];

		float fDiff			= pSpring->m_fPreferredDistance - pSpring->m_fDistance;
		float fRatio		= 2.0f * fDiff / pSpring->m_fPreferredDistance;


		float fAlpha = Util::Clamp(fRatio, -1.0f, 1.0f);


		if (fAlpha < 0.0f)
		{
			fAlpha = -fAlpha;
			vColor.m_fRed	= (vColorBlue.m_fRed * fAlpha) + (vColorGreen.m_fRed * (1.0f - fAlpha));
			vColor.m_fGreen	= (vColorBlue.m_fGreen * fAlpha) + (vColorGreen.m_fGreen * (1.0f - fAlpha));
			vColor.m_fBlue	= (vColorBlue.m_fBlue * fAlpha) + (vColorGreen.m_fBlue * (1.0f - fAlpha));
		}
		else
		{
			//
			vColor.m_fRed	= (vColorRed.m_fRed * fAlpha) + (vColorGreen.m_fRed * (1.0f - fAlpha));
			vColor.m_fGreen	= (vColorRed.m_fGreen * fAlpha) + (vColorGreen.m_fGreen * (1.0f - fAlpha));
			vColor.m_fBlue	= (vColorRed.m_fBlue * fAlpha) + (vColorGreen.m_fBlue * (1.0f - fAlpha));
		}


		pDst[iDstIndex].m_vPosition	= *pVertex0;
		pDst[iDstIndex].m_vColor	= vColor;
		iDstIndex++;

		pDst[iDstIndex].m_vPosition	= *pVertex1;
		pDst[iDstIndex].m_vColor	= vColor;
		iDstIndex++;
	}

	Gfx::UnlockVertexBuffer(pVertexBuffer, &oLock);
}
	//
	namespace Gfx
	{
		extern ID3D11Device*		dev;
		extern ID3D11DeviceContext* devcon;
	}


void CreateIndirectMap(const ForceField2D::Vector2* a_pPositionArray, HeightMapCropData* a_pCropData, unsigned int a_iSimulationResolution)
{
	unsigned int iVertexCount = a_pCropData->m_iSizeX * a_pCropData->m_iSizeY;

	float fBaseTexcoordX = 0.0f;
	float fBaseTexcoordY = 0.0f;


	ForceField2D::Vector2* pDestBuffer = 
		(ForceField2D::Vector2*)malloc(sizeof(ForceField2D::Vector2) * a_pCropData->m_iHeightmapDimension * a_pCropData->m_iHeightmapDimension);
	
	ForceField2D::Vector2 vScale = { 1.0f / (float)a_pCropData->m_iSizeX, 1.0f / (float)a_pCropData->m_iSizeY };
	

	// extend cropped data
	for (int iPosY = 0; iPosY < (int)a_pCropData->m_iHeightmapDimension; ++iPosY)
	{
		int iClampedY = Util::Clamp<int>(iPosY, a_pCropData->m_iStartY, a_pCropData->m_iStartY + a_pCropData->m_iSizeY);

		for (int iPosX = 0; iPosX < (int)a_pCropData->m_iHeightmapDimension; ++iPosX)
		{
			int iClampedX = Util::Clamp<int>(iPosX, a_pCropData->m_iStartX, a_pCropData->m_iStartX + a_pCropData->m_iSizeX);


			ForceField2D::Vector2 vBasePos = { (float)(iClampedX - a_pCropData->m_iStartX), (float)(iClampedY - a_pCropData->m_iStartY) };

			unsigned int iSrcArrayIndex = ((iClampedY - a_pCropData->m_iStartY) * (a_pCropData->m_iSizeX + 1)) + (iClampedX - a_pCropData->m_iStartX);
			assert(iSrcArrayIndex < ((a_pCropData->m_iSizeX + 1) * (a_pCropData->m_iSizeY + 1)));
			ForceField2D::Vector2 vFixedPos = a_pPositionArray[iSrcArrayIndex];

			ForceField2D::Vector2 vOffset = (vFixedPos - vBasePos);

			unsigned int iDstArrayIndex = (iPosY * a_pCropData->m_iHeightmapDimension) + iPosX;
			assert(iDstArrayIndex < (a_pCropData->m_iHeightmapDimension * a_pCropData->m_iHeightmapDimension));
			pDestBuffer[iDstArrayIndex] = vOffset * vScale;			
		}		
	}

	
	unsigned int iWidth		= a_pCropData->m_iHeightmapDimension;
	unsigned int iHeight	= a_pCropData->m_iHeightmapDimension;
	unsigned int iPitch		= sizeof(ForceField2D::Vector2) * a_pCropData->m_iHeightmapDimension;

	Gfx::SaveBufferAsDDS(pDestBuffer, iPitch, iWidth, iHeight, Gfx::SurfaceFormat_R32G32F, "media\\imap.dds");

	free(pDestBuffer);
	
}

void Update()
{
	Input::Update();

	float fFrameTime = (float)oFrameTimer.GetElapsedTime();

	if (g_bSimulationRunning)
	{
		float fMoveStep = fFrameTime * 30.0f; //;
		float fZoomStep = fFrameTime * 30.0f * 0.02f;
		QuadTreeActionId arrTriggeredAction[128];


		fMoveStep *= fGridScale;

		unsigned int iTrigerredActionCount = Input::GetTriggeredActionList((Input::ActionId*)&arrTriggeredAction, 128);
		
		for (unsigned int i = 0; i < iTrigerredActionCount; ++i)
		{
			switch (arrTriggeredAction[i])
			{
				case QuadTreeActionId_MoveForward:
					fGridScale += fZoomStep; 
					break;
				case QuadTreeActionId_MoveBackward:
					fGridScale -= fZoomStep;
					break;
				case QuadTreeActionId_MoveLeft:
					fGridPosX -= fMoveStep; 
					break;
				case QuadTreeActionId_MoveRight:
					fGridPosX += fMoveStep; 
					break;
				case QuadTreeActionId_MoveUp:
					fGridPosY -= fMoveStep; 
					break;
				case QuadTreeActionId_MoveDown:
					fGridPosY += fMoveStep; 
					break;
			}
		}

		g_arrSimulator[g_iSimulatorIndex].Simulate(0.001f);

		if ((g_arrSimulator[g_iSimulatorIndex].GetIterationCount() % 64) == 0)
		{

			if (g_arrSimulator[g_iSimulatorIndex].IsConverged())		
			{
				if (g_iSimulationIteration == 0)			
				{
					g_bSimulationRunning = false;

					unsigned int iVertexCount = (g_arrSimulator[g_iSimulatorIndex].GetDimensionX() + 1) * (g_arrSimulator[g_iSimulatorIndex].GetDimensionY() + 1);
					const ForceField2D::Vector2* pPositionArray = g_arrSimulator[g_iSimulatorIndex].GetVertexPositionArray();

					//
					char szFileName[256];
					sprintf_s(szFileName, 64, "media\\sim.dat");
					Util::SaveFile(szFileName, pPositionArray, sizeof(ForceField2D::Vector2) * iVertexCount);

					Util::PrintMessage("Simulator ended with error: %f\n", g_arrSimulator[g_iSimulatorIndex].GetError());

					CreateIndirectMap(pPositionArray, &g_oCropData, 1);

					InitializeQuadTree();
				}
				else
				{
					unsigned int			iNextSimulatorIndex		= 1 - g_iSimulatorIndex;
					unsigned int			iHeightmapDimension		= g_oCropData.m_iHeightmapDimension;
					const HeightMapData*	pHeightmap				= g_oCropData.m_pHeightmap;

					//
					g_iSimulationIteration--;

					unsigned int iSimulationResolution = 1 << g_iSimulationIteration;

					g_arrSimulator[iNextSimulatorIndex].InitializeFromPreviousResult(
						g_arrSimulator[g_iSimulatorIndex],					
						&g_oCropData, iSimulationResolution);

					g_arrSimulator[g_iSimulatorIndex].Shutdown();

					g_iSimulatorIndex = iNextSimulatorIndex;	

					
				}	
			}

			UpdateForceFieldVertexBuffer();
		}

		UpdateConstantBuffer();

	}
	else
	{
		QuadTreeActionId arrTriggeredAction[128];
		unsigned int iTrigerredActionCount = Input::GetTriggeredActionList((Input::ActionId*)&arrTriggeredAction, 128);

		const float fStep = 500.0f * fFrameTime * 30.0f;
		const float fRotationSpeed = 0.1f * 100.0f * fFrameTime;

		float fTranslationX = 0.0f;
		float fTranslationY = 0.0f;
		float fTranslationZ = 0.0f;

		VertexWeightLayout eLayout = oQuadTree.GetVertexWeightLayout();

		for (unsigned int i = 0; i < iTrigerredActionCount; ++i)
		{
			switch (arrTriggeredAction[i])
			{
				case QuadTreeActionId_MoveForward:
					fTranslationZ += fStep; 
					break;
				case QuadTreeActionId_MoveBackward:
					fTranslationZ -= fStep; 
					break;
				case QuadTreeActionId_MoveLeft:
					fTranslationX -= fStep; 
					break;
				case QuadTreeActionId_MoveRight:
					fTranslationX += fStep; 
					break;
				case QuadTreeActionId_MoveUp:
					fTranslationY -= fStep; 
					break;
				case QuadTreeActionId_MoveDown:
					fTranslationY += fStep; 
					break;
				case QuadTreeActionId_ViewLeft:
					fCameraYaw += fRotationSpeed;
					break;
				case QuadTreeActionId_ViewRight:
					fCameraYaw -= fRotationSpeed;
					break;
				case QuadTreeActionId_ViewUp:
					fCameraPitch += fRotationSpeed;
					break;
				case QuadTreeActionId_ViewDown:
					fCameraPitch -= fRotationSpeed;
					break;

				case QuadTreeActionId_Action1:					
					if (eLayout == VertexWeightLayout_Solver)
						eLayout = VertexWeightLayout_Xinfo;
					else
						eLayout = VertexWeightLayout_Solver;

					oQuadTree.SetVertexWeightLayout(eLayout);
					break;
				case QuadTreeActionId_Action2:
					if (fShowOffset == 1.0f)
						fShowOffset = 0.0f;
					else
						fShowOffset = 1.0f;
					break;

			}
		}

		D3DXMATRIX mMovementRotationY;
		D3DXMatrixRotationY(&mMovementRotationY, -fCameraYaw);

		D3DXVECTOR4 vMovement(fTranslationX, 0.0f, fTranslationZ, 0.0f);
		D3DXVec4Transform(&vMovement, &vMovement, &mMovementRotationY);

		fQuadTreePosX += vMovement.x;
		fQuadTreePosY += fTranslationY;
		fQuadTreePosZ += vMovement.z;

		UpdateConstantBuffer();

		oQuadTree.Update(fQuadTreePosX, fQuadTreePosY, fQuadTreePosZ, fCameraYaw, fCameraPitch);
	}
}

void Render()
{
	Gfx::Begin();


	float aColor[4] = { 0.254f, 0.254f, 0.254f, 1.0f };
	Gfx::Clear(aColor);

	if (g_bSimulationRunning)
	{
		Gfx::SetPixelShader(pPixelShader);
		Gfx::SetVertexShader(pVertexShader);
		Gfx::SetInputLayout(pInputLayout);
		Gfx::SetInputPrimitive(Gfx::InputPrimitive_LineList);
		Gfx::SetConstantBuffer(0, pConstantBuffer);
		Gfx::SetVertexBuffer(0, pVertexBuffer);

		unsigned int iSpringCount = g_arrSimulator[g_iSimulatorIndex].GetSpringCount();
		Gfx::DrawVertices(iSpringCount * 2);
	}
	else
	{
		oQuadTree.Draw();
	}


	Gfx::Present();
}

// this is the main message handler for the program
LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch(message)
	{
	case WM_DESTROY:
		{
			PostQuitMessage(0);
			return 0;
		} break;
	}

	Input::HandleMessage(hWnd, message, wParam, lParam);

	return DefWindowProc (hWnd, message, wParam, lParam);
}


int CALLBACK WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{

	HWND hWnd;
	WNDCLASSEX wc;

	ZeroMemory(&wc, sizeof(WNDCLASSEX));

	wc.cbSize = sizeof(WNDCLASSEX);
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = WindowProc;
	wc.hInstance = hInstance;
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)COLOR_WINDOW;
	wc.lpszClassName = L"WindowClass";

	RegisterClassEx(&wc);

	RECT wr = {0, 0, 800, 600};
	RECT adjustedWr = wr;
	AdjustWindowRect(&adjustedWr, WS_OVERLAPPEDWINDOW, FALSE);

	hWnd = CreateWindowEx(NULL,
		L"WindowClass",
		L"Direct3D",
		WS_OVERLAPPEDWINDOW,
		300,
		300,
		adjustedWr.right - adjustedWr.left,
		adjustedWr.bottom - adjustedWr.top,
		NULL,
		NULL,
		hInstance,
		NULL);

	ShowWindow(hWnd, nCmdShow);

	// set up and initialize Direct3D
	Gfx::InitializeD3D(hWnd, wr);

	//
	Input::Initialize();
	Input::RegisterKeyAction(Input::Key_Z,		Input::KeyState_Down, QuadTreeActionId_MoveForward);
	Input::RegisterKeyAction(Input::Key_S,		Input::KeyState_Down, QuadTreeActionId_MoveBackward);
	Input::RegisterKeyAction(Input::Key_Q,		Input::KeyState_Down, QuadTreeActionId_MoveLeft);
	Input::RegisterKeyAction(Input::Key_D,		Input::KeyState_Down, QuadTreeActionId_MoveRight);
	Input::RegisterKeyAction(Input::Key_A,		Input::KeyState_Down, QuadTreeActionId_MoveUp);
	Input::RegisterKeyAction(Input::Key_E,		Input::KeyState_Down, QuadTreeActionId_MoveDown);
	Input::RegisterKeyAction(Input::Key_Left,	Input::KeyState_Down, QuadTreeActionId_ViewLeft);
	Input::RegisterKeyAction(Input::Key_Right,	Input::KeyState_Down, QuadTreeActionId_ViewRight);
	Input::RegisterKeyAction(Input::Key_Up,		Input::KeyState_Down, QuadTreeActionId_ViewUp);
	Input::RegisterKeyAction(Input::Key_Down,	Input::KeyState_Down, QuadTreeActionId_ViewDown);


	Input::RegisterKeyAction(Input::Key_Space, Input::KeyState_Pressed, QuadTreeActionId_Action1);
	Input::RegisterKeyAction(Input::Key_Enter, Input::KeyState_Pressed, QuadTreeActionId_Action2);
	

	Gfx::CreateConstantBuffer(16, &pConstantBuffer);	
	UpdateConstantBuffer();

	//InitializeSimulation();
	InitializeQuadTree();


	MSG msg;

	while(TRUE)
	{
		if(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);

			if(msg.message == WM_QUIT)
				break;
		}
		else
		{
			Update();
			Render();			
		}
	}

	// clean up DirectX and COM
	Gfx::ShutdownD3D();
}

