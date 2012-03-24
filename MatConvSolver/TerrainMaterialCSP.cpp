
#include "StdAfx.h"

#include "TerrainMaterialCSP.h"
#include <algorithm>
#include <set>
#include "MicroSDK/Timer.h"

using namespace MicroSDK;


/*
bool TerrainMaterialCSP::MaterialData::IsCompatible(const MaterialData& a_rData) const
{
	const unsigned int iBitPerValue		= 4;
	const unsigned __int64 iMask		= (1 << iBitPerValue) - 1;

	unsigned __int64 iValue0	= 0;
	unsigned __int64 iMask0		= 0;

	unsigned __int64 iValue1	= 0;
	unsigned __int64 iMask1		= 0;

	for (unsigned int i = 0; i < g_iMaxMaterialPerTile; ++i)
	{
		unsigned int iMaterialIndex0	= GetMaterialIndex(i);
		unsigned int iShift0			= iMaterialIndex0 * iBitPerValue;

		iValue0		|= (unsigned __int64)(i + 1)	<< iShift0;
		iMask0		|= iMask						<< iShift0;

		//
		unsigned int iMaterialIndex1	= a_rData.GetMaterialIndex(i);
		unsigned int iShift1			= iMaterialIndex1 * iBitPerValue;

		iValue1		|= (unsigned __int64)(i + 1)	<< iShift1;
		iMask1		|= iMask						<< iShift1;
	}

	unsigned __int64 iCommonMask	= iMask0 & iMask1 & 0x0FFFFFFFFFFF;
	unsigned __int64 iMasked0		= iValue0 & iCommonMask;
	unsigned __int64 iMasked1		= iValue1 & iCommonMask;

	return iMasked0 == iMasked1;
}
*/

TerrainMaterialCSP::TerrainMaterialCSP()
{
}

TerrainMaterialCSP::~TerrainMaterialCSP()
{

}

void TerrainMaterialCSP::Initialize(eSMaterialConvertionData* a_arrMaterialData, unsigned int a_iSizeX, unsigned int a_iSizeY, unsigned int a_iCountLodLevel)
{
	m_oAllocator.Initialize(8 * 1024 * 1024);

	GenerateAllPermutations();	

	SetLodLevel(a_iSizeX, a_iSizeY, a_iCountLodLevel);
	CreateMaterialTypeInfoArray(a_arrMaterialData, a_iSizeX, a_iSizeY);
	CreateTileConstraintArray(a_iSizeX, a_iSizeY, a_iCountLodLevel);

	m_iSolutionCost = 0;
}

void TerrainMaterialCSP::Shutdown()
{

}

void TerrainMaterialCSP::Clear()
{

}


bool TerrainMaterialCSP::Solve()
{
	m_arrChangeList.clear();

	//
	std::vector<Node*> arrNode;

	CreateNodeList(arrNode);
	return ProcessNodeList(arrNode);
}

bool TerrainMaterialCSP::CompareNode(Node* a_pNode0, Node* a_pNode1)
{
	unsigned int iMaterialCount0 = Util::CountBitSet(a_pNode0->m_iMaterialMask);
	unsigned int iMaterialCount1 = Util::CountBitSet(a_pNode1->m_iMaterialMask);
	return iMaterialCount1 < iMaterialCount0;
}

void TerrainMaterialCSP::CreateLod0TileList(std::vector<Node*>& a_arrNode)
{
	int				iLod0DimensionX		= (int)(m_iMaterialTypeInfoCountX / GE_TRTILE_TILESTRIDE);
	int				iLod0DimensionY		= (int)(m_iMaterialTypeInfoCountY / GE_TRTILE_TILESTRIDE);
	int				iLod0Count			= iLod0DimensionX * iLod0DimensionY;
	int				iLod0DimensionMax	= (int)Util::Max(iLod0DimensionX, iLod0DimensionY);

	// initial position
	int				iSearchX			= 0;
	int				iSearchY			= 0;

	for (int iSearchBreadth = 0; iSearchBreadth < iLod0DimensionMax * 2; ++iSearchBreadth)
	{
		for (int iSearchOffsetY = -iSearchBreadth; iSearchOffsetY <= iSearchBreadth; ++iSearchOffsetY)
		{
			for (int iSearchOffsetX = -iSearchBreadth; iSearchOffsetX <= iSearchBreadth; ++iSearchOffsetX)
			{
				if ((Util::Abs(iSearchOffsetY) + Util::Abs(iSearchOffsetX)) != iSearchBreadth)
					continue;

				int iArrayX = iSearchX + iSearchOffsetX;
				int iArrayY = iSearchY + iSearchOffsetY;

				if ((iArrayX < 0) || (iArrayX >= (int)iLod0DimensionX))
					continue;

				if ((iArrayY < 0) || (iArrayY >= (int)iLod0DimensionY))
					continue;

				unsigned int iArrayOffset = (iArrayY * iLod0DimensionX) + iArrayX;
				Node* pNode = &m_arrTileConstraintNode[iArrayOffset];

				ASSERT(std::find(a_arrNode.begin(), a_arrNode.end(), pNode) == a_arrNode.end());

				a_arrNode.push_back(pNode);

			}
		}		
	}

	ASSERT(a_arrNode.size() == (size_t)iLod0Count);



	//for (unsigned i = 0; i < iLod0Count; ++i)
	//{
	//	a_arrNode.push_back(&m_arrTileConstraintNode[i]);
	//}

	//std::sort(a_arrNode.begin(), a_arrNode.end(), CompareNode);

	if (Util::CountBitSet(a_arrNode[0]->m_iMaterialMask) <= g_iMaxMaterialPerTile)
	{
		a_arrNode[0]->m_iLookAheadDepth = 0;
	}

}

void TerrainMaterialCSP::CreateNodeList(std::vector<Node*>& a_arrNode)
{
	a_arrNode.clear();
	a_arrNode.reserve(m_arrMaterialTypeInfoNode.size() + m_arrTileConstraintNode.size());


	CreateLod0TileList(a_arrNode);


	unsigned int iVertexCount = m_arrMaterialTypeInfoNode.size();
	for (unsigned i = 0; i < iVertexCount; ++i)
	{
		a_arrNode.push_back(&m_arrMaterialTypeInfoNode[i]);
	}


	unsigned int	iLod0DimensionX		= m_iMaterialTypeInfoCountX / GE_TRTILE_TILESTRIDE;
	unsigned int	iLod0DimensionY		= m_iMaterialTypeInfoCountY / GE_TRTILE_TILESTRIDE;
	unsigned int	iLod0Count			= iLod0DimensionX * iLod0DimensionY;

	unsigned int iTileCount = m_arrTileConstraintNode.size();
	for (unsigned i = iLod0Count; i < iTileCount; ++i)
	{
		a_arrNode.push_back(&m_arrTileConstraintNode[i]);
	}
}

bool TerrainMaterialCSP::ProcessNodeList(std::vector<Node*>& a_arrNode)
{
	MicroSDK::Timer oTimer;
	DOUBLE fCumulatedTime = 0.0;
	DOUBLE fOutputTimeThreshold = 10.0;

	oTimer.Start();

	m_arrChangeList.clear();
	m_arrChangeList.reserve(a_arrNode.size());

	unsigned int iTotalCost = 0;

	unsigned int iNodeCount = a_arrNode.size();
	unsigned int iNodeProcessedCount = 0;

	while (iNodeProcessedCount < iNodeCount)	
	{
		fCumulatedTime += oTimer.GetElapsedTime();
		if (fCumulatedTime > fOutputTimeThreshold)
		{
			Util::PrintMessage("Node: %d Cost: %d Time: %f\n", iNodeProcessedCount, iTotalCost, oTimer.GetAppTime());
			fCumulatedTime -= fOutputTimeThreshold;
		}

		Node* pNode = a_arrNode[iNodeProcessedCount];

		if (pNode->m_arrPermutation.GetCount() > 1)
		{
			m_arrChangeList.push_back(ChangeList()); // new change list
			
			// only do it once for each change list
			m_arrChangeList.back().m_bLookAheadDone = false;

			do
			{
				ChangeList& rCurrentChangeList = m_arrChangeList.back();

				rCurrentChangeList.m_arrDependency.clear();
				rCurrentChangeList.m_oPermutation.Clear();
				rCurrentChangeList.m_iDependencyCost		= 0;				

				//
				ASSERT(pNode->m_arrPermutation.GetCount() > 0);
				ASSERT(rCurrentChangeList.m_arrFailedPermutation.size() <= pNode->m_arrPermutation.GetCount());

				if (pNode->m_arrPermutation.GetCount() == rCurrentChangeList.m_arrFailedPermutation.size())
				{
					m_arrChangeList.pop_back();

					if (m_arrChangeList.size() == 0) // impossible !
					{
						return false;
					}

					//
					pNode = m_arrChangeList.back().m_arrDependency[0].m_pNode;
					
					do
					{
						iNodeProcessedCount--;
					}
					while (a_arrNode[iNodeProcessedCount] != pNode);

					
					RevertChangeList(m_arrChangeList.back());

					m_arrChangeList.back().m_arrFailedPermutation.push_back(m_arrChangeList.back().m_oPermutation);
					m_arrChangeList.back().m_oLookAheadResult.m_bIsValid = false;
					continue;
				}


				//
				if (!rCurrentChangeList.m_bLookAheadDone)
				{
					rCurrentChangeList.m_oLookAheadResult = LookAhead(&a_arrNode[iNodeProcessedCount], iNodeCount - iNodeProcessedCount, pNode->m_iLookAheadDepth);
					rCurrentChangeList.m_bLookAheadDone = true;
				}

				//
				if (rCurrentChangeList.m_oLookAheadResult.m_bIsValid)
				{	
					rCurrentChangeList.m_oPermutation = rCurrentChangeList.m_oLookAheadResult.m_oPermutation;
				}
				else
				{
					
					unsigned int iPermutationIndex = FindBestPermutationIndex(pNode, rCurrentChangeList.m_arrFailedPermutation);
					rCurrentChangeList.m_oPermutation = pNode->m_arrPermutation[iPermutationIndex];
				}


				if (!UpdateNode(rCurrentChangeList, pNode))
				{
					Util::PrintMessage(
						"Node %d: %d/%d permutation %d failed\n",
						pNode->m_eType, pNode->m_iPosX, pNode->m_iPosY, FindPermutationIndex(pNode->m_arrPermutation, rCurrentChangeList.m_oPermutation));

					RevertChangeList(rCurrentChangeList);

					// failed to find a solution, try another solution
					rCurrentChangeList.m_arrFailedPermutation.push_back(rCurrentChangeList.m_oPermutation);

					//
					rCurrentChangeList.m_oLookAheadResult.m_bIsValid = false;
					continue;
				}

				//				
				break;

			} while (true);
		}

		iTotalCost += m_arrChangeList.back().m_oPermutation.m_iCost;

		iNodeProcessedCount++;

	}

	m_iSolutionCost = iTotalCost;

	Util::PrintMessage("Found solution with total cost %d in %f s\n", iTotalCost, oTimer.GetAppTime());
	return true;
}

TerrainMaterialCSP::LookAheadResult TerrainMaterialCSP::LookAhead(Node** a_arrNode, unsigned int a_iNodeCount, unsigned int a_iDepth)
{
	if (a_iDepth == 0)
	{
		LookAheadResult	oResult;
		oResult.m_iLowedCost			= 0;		
		oResult.m_bIsValid				= false;
		oResult.m_oPermutation.Clear();
		return oResult;
	}

	//	
	while ((a_iNodeCount > 0) && ((*a_arrNode)->m_arrPermutation.GetCount() == 1))
	{
		a_arrNode++;		
		a_iNodeCount--;
	}

	if (a_iNodeCount == 0)
	{
		LookAheadResult	oResult;
		oResult.m_iLowedCost			= 0;		
		oResult.m_bIsValid				= false;
		oResult.m_oPermutation.Clear();
		return oResult;
	}

	//
	Node* pNode = *a_arrNode;

	//
	const unsigned int iTestCountMax = g_iLookAheadMaxTest;
	std::vector<MaterialData> arrBestPermutation;

	//
	arrBestPermutation.reserve(iTestCountMax);


	while ((arrBestPermutation.size() < iTestCountMax) && (arrBestPermutation.size() < pNode->m_arrPermutation.GetCount()))
	{
		unsigned int iPermutationIndex = FindBestLookAheadPermutationIndex(pNode, arrBestPermutation);

		//
		MaterialData& rData = pNode->m_arrPermutation[iPermutationIndex];
		arrBestPermutation.push_back(rData);
	}

	//
	ChangeList		oChangeList;
	LookAheadResult	oResult;

	//
	oResult.m_iLowedCost			= (unsigned int)-1;
	oResult.m_bIsValid				= false;
	oResult.m_oPermutation.Clear();

	//
	unsigned int iBestPermutationCount = arrBestPermutation.size();
	for (unsigned int iTestIndex = 0; iTestIndex < iBestPermutationCount; ++iTestIndex)
	{
		oChangeList.m_oPermutation		= arrBestPermutation[iTestIndex];
		oChangeList.m_iDependencyCost	= 0;

		oChangeList.m_arrFailedPermutation.clear();
		oChangeList.m_arrDependency.clear();

		
		if (UpdateNode(oChangeList, pNode))
		{
			unsigned int	iPermutationCost	= oChangeList.m_oPermutation.m_iCost;
			LookAheadResult oChildResult		= LookAhead(a_arrNode + 1, a_iNodeCount - 1, a_iDepth - 1);			
			unsigned int	iTotalCost			= oChildResult.m_iLowedCost + (iPermutationCost * g_iMaxCost) + oChangeList.m_iDependencyCost;

			if (oResult.m_iLowedCost > iTotalCost)
			{
				oResult.m_bIsValid		= true;
				oResult.m_iLowedCost	= iTotalCost;
				oResult.m_oPermutation	= arrBestPermutation[iTestIndex];
			}			
		}

		RevertChangeList(oChangeList);
	}	


	//
	return oResult;
}

unsigned int TerrainMaterialCSP::FindPermutationIndex(const MaterialDataArray& a_rArray, const MaterialData& a_rPermutation)
{
	unsigned int iNodePermutationCount = a_rArray.GetCount();
	unsigned int iPermutationIndex = 0;

	while (iPermutationIndex < iNodePermutationCount)
	{		
		if (a_rPermutation.IsEqual(a_rArray[iPermutationIndex]))
			break;

		iPermutationIndex++;
	}
	
	return iPermutationIndex;
}

unsigned int TerrainMaterialCSP::FindPermutationIndex(const std::vector<MaterialData>& a_rArray, const MaterialData& a_rPermutation)
{
	unsigned int iNodePermutationCount = a_rArray.size();
	unsigned int iPermutationIndex = 0;

	while (iPermutationIndex < iNodePermutationCount)
	{		
		if (a_rPermutation.IsEqual(a_rArray[iPermutationIndex]))
			break;

		iPermutationIndex++;
	}
	
	return iPermutationIndex;

}


void TerrainMaterialCSP::GenerateAllPermutations()
{
	m_arrPermutation.clear();


	Permutation oPerm;
	oPerm.m_arrIndex[0] = 0;
	oPerm.m_arrIndex[1] = 1;
	oPerm.m_arrIndex[2] = 2;
	oPerm.m_arrIndex[3] = 3;
	oPerm.m_arrIndex[4] = 4;

	for (unsigned int iMaxDepth = 2; iMaxDepth <= g_iMaxMaterialPerTile; ++iMaxDepth)
	{
		GeneratePermutation(oPerm, 0, iMaxDepth);
	}

}


void TerrainMaterialCSP::GeneratePermutation(Permutation a_oPerm, int a_iDepth, int a_iMaxDepth)
{
	ASSERT(a_iDepth < a_iMaxDepth);

	if (a_iDepth == (a_iMaxDepth - 1))
	{
		if (std::find(m_arrPermutation.begin(), m_arrPermutation.end(), a_oPerm) == m_arrPermutation.end())
		{
			m_arrPermutation.push_back(a_oPerm);
		}
		return;
	}

	GeneratePermutation(a_oPerm, a_iDepth + 1, a_iMaxDepth);

	for (int iIndex = 1; iIndex < g_iMaxMaterialPerTile; ++iIndex)
	{
		int iTemp = a_oPerm.m_arrIndex[iIndex - 1];
		a_oPerm.m_arrIndex[iIndex - 1] = a_oPerm.m_arrIndex[iIndex];
		a_oPerm.m_arrIndex[iIndex] = iTemp;

		GeneratePermutation(a_oPerm, a_iDepth + 1, a_iMaxDepth);
	}

}

void TerrainMaterialCSP::ApplyPermutation(const unsigned char a_arrSrc[g_iMaxMaterialPerTile], unsigned int a_iPermutationIndex, MaterialData* a_pData)
{
	const Permutation* pPermutation = &m_arrPermutation[a_iPermutationIndex];

	for (int i = 0; i < g_iMaxMaterialPerTile; ++i)
	{
		a_pData->SetMaterialIndex(i, a_arrSrc[pPermutation->m_arrIndex[i]]);
	}
}


void TerrainMaterialCSP::CreateNodePermutationList(unsigned int a_iMaterialMask, unsigned int a_iCost, MaterialDataArray& a_arrPermutation)
{
	unsigned char arrMaterialIndex[g_iMaxMaterialPerTile];
	unsigned int iMaterialCount = 0;

	for (unsigned int iMaterialIndex = 0; iMaterialIndex < iMaxMaterialCount; ++iMaterialIndex)
	{
		if ((a_iMaterialMask & (1 << iMaterialIndex)) != 0)
		{
			arrMaterialIndex[iMaterialCount] = iMaterialIndex;
			iMaterialCount++;
		}
	}

	ASSERT(iMaterialCount <= g_iMaxMaterialPerTile);

	for (unsigned int iSlotIndex = iMaterialCount; iSlotIndex < g_iMaxMaterialPerTile; ++iSlotIndex)
	{
		arrMaterialIndex[iSlotIndex] = iInvalidMaterialIndex;
	}


	//
	unsigned int iPermutationCount = 1;
	for (unsigned int iCount = 0; iCount < iMaterialCount; ++iCount)
	{
		iPermutationCount *= (g_iMaxMaterialPerTile - iCount);
	}


	for (unsigned int iPermutationIndex = 0; iPermutationIndex < iPermutationCount; ++iPermutationIndex)
	{
		MaterialData oPermutation;

		oPermutation.m_iCost = a_iCost;
		ApplyPermutation(arrMaterialIndex, iPermutationIndex, &oPermutation);

		a_arrPermutation.PushBack(oPermutation);
	}
}

void TerrainMaterialCSP::CreateNodeMaterialCombinationList(unsigned int a_iBaseMaterialMask, unsigned int a_iRemainingMaterialMask, unsigned int a_iDepth, std::vector<unsigned int>& a_arrCombination)
{
	if (a_iDepth == 0)
	{
		return;
	}

	ASSERT((a_iBaseMaterialMask &  a_iRemainingMaterialMask) == 0);
	
	unsigned int iNewRemainingMaterialMask = a_iRemainingMaterialMask;
	unsigned int iNewBaseMaterialMask = 0;

	while (iNewRemainingMaterialMask != 0)
	{
		unsigned int iMaterialIndex = 31 - Util::CountLeadingZeros(iNewRemainingMaterialMask);
		unsigned int iMask = 1 << iMaterialIndex;

		iNewRemainingMaterialMask	= iNewRemainingMaterialMask & ~iMask;
		iNewBaseMaterialMask		= a_iBaseMaterialMask | iMask;

		CreateNodeMaterialCombinationList(iNewBaseMaterialMask, iNewRemainingMaterialMask, a_iDepth - 1, a_arrCombination);

		a_arrCombination.push_back(iNewBaseMaterialMask);
	}

}

unsigned int TerrainMaterialCSP::ComputeCombinationCost(const Node* a_pNode, unsigned int a_iCombinationMask)
{
	ASSERT(a_iCombinationMask != 0);
	ASSERT((a_pNode->m_iMaterialMask & a_iCombinationMask) == a_iCombinationMask);

	unsigned int iCost = 0;
	unsigned int iNodeMaterialMask = a_pNode->m_iMaterialMask;
	unsigned int iNodeMissingMaterialMask = iNodeMaterialMask & ~a_iCombinationMask;

	while (iNodeMissingMaterialMask != 0)
	{
		unsigned int iMaterialIndex = 31 - Util::CountLeadingZeros(iNodeMissingMaterialMask);
		iCost += a_pNode->m_arrMaterialWeight[iMaterialIndex];
		iNodeMissingMaterialMask &= ~(1 << iMaterialIndex);
	}

	return iCost;
}

void TerrainMaterialCSP::CreateNodeMaterialList(Node* a_pNode)
{
	std::vector<unsigned int> arrCombination;
	CreateNodeMaterialCombinationList(0, a_pNode->m_iMaterialMask, g_iMaxMaterialPerTile, arrCombination);
	//arrCombination.push_back(a_iMaterialMask);

	//
	unsigned int iCombinationCount = arrCombination.size();
	unsigned int iTotalPermutationCount = 0;

	for (unsigned int iCombinationIndex = 0; iCombinationIndex < iCombinationCount; ++iCombinationIndex)
	{
		iTotalPermutationCount += ComputePermutationCount(arrCombination[iCombinationIndex]);
	}

	//
	m_oAllocator.AllocateArray(iTotalPermutationCount, &a_pNode->m_arrPermutation);

	for (unsigned int iCombinationIndex = 0; iCombinationIndex < iCombinationCount; ++iCombinationIndex)
	{
		unsigned int iMaterialCombinationMask	= arrCombination[iCombinationIndex];
		unsigned int iMaterialCount				= Util::CountBitSet(iMaterialCombinationMask);
		unsigned int iCost						= ComputeCombinationCost(a_pNode, iMaterialCombinationMask);

		ASSERT(iMaterialCount <= g_iMaxMaterialPerTile);
		CreateNodePermutationList(iMaterialCombinationMask, iCost, a_pNode->m_arrPermutation);
	}

	ASSERT(a_pNode->m_arrPermutation.GetCount() == iTotalPermutationCount);
}

void TerrainMaterialCSP::SetLodLevel(unsigned int a_iSizeX, unsigned int a_iSizeY, unsigned int a_iMaxLodLevel)
{
	m_iMaterialTypeInfoCountLodLevel = a_iMaxLodLevel;

	unsigned int iTileCountX = (a_iSizeX - 1) / GE_TRTILE_TILESTRIDE;
	unsigned int iTileCountY = (a_iSizeY - 1) / GE_TRTILE_TILESTRIDE;

	for (unsigned int iLodLevel = 0; iLodLevel < a_iMaxLodLevel; ++iLodLevel)
	{
		if (((iTileCountX % 2) != 0) || ((iTileCountY % 2) != 0))
		{
			// we reached the maximum lod level for the current data
			m_iMaterialTypeInfoCountLodLevel = iLodLevel + 1;
			break;
		}

		iTileCountX /= 2;
		iTileCountY /= 2;
	}
}

void TerrainMaterialCSP::CreateMaterialTypeInfoArray(eSMaterialConvertionData* a_arrMaterialData, unsigned int a_iSizeX, unsigned int a_iSizeY)
{
	m_iMaterialTypeInfoCountX = a_iSizeX;
	m_iMaterialTypeInfoCountY = a_iSizeY;

	//
	m_arrMaterialTypeInfoNode.resize(a_iSizeX * a_iSizeY);

	unsigned int iArrayIndex = 0;


	for (unsigned int iY = 0; iY  < a_iSizeY; ++iY)
	{
		for (unsigned int iX = 0; iX  < a_iSizeX; ++iX)
		{
			const eSMaterialConvertionData*	pSrcMaterialData	= &a_arrMaterialData[iArrayIndex];
			Node*							pDstNode			= &m_arrMaterialTypeInfoNode[iArrayIndex];
			unsigned int					iMaterialMask		= 0;

			for (unsigned int iMaterialIndex = 0; iMaterialIndex < iMaxMaterialCount; ++iMaterialIndex)
			{
				if (pSrcMaterialData->m_arrMaterialWeight[iMaterialIndex] > 0)
				{
					iMaterialMask |= 1 << iMaterialIndex;
				}
			}

			//
			pDstNode->m_eType			= NodeType_HeightmapVertex;
			pDstNode->m_iPosX			= iX;
			pDstNode->m_iPosY			= iY;
			pDstNode->m_iLodLevel		= 0;
			pDstNode->m_iMaterialMask	= iMaterialMask;
			pDstNode->m_iPushCount		= 0;
			pDstNode->m_iProcessedCount	= 0;
			pDstNode->m_iTestCount		= 0;
			pDstNode->m_iLookAheadDepth	= g_iLookAheadVertexDepth;

			memcpy(pDstNode->m_arrMaterialWeight, pSrcMaterialData->m_arrMaterialWeight, sizeof(pDstNode->m_arrMaterialWeight));


			//			
			unsigned int iNeighborCount = ComputeNeighborCount(pDstNode);
			m_oAllocator.AllocateArray(iNeighborCount, &pDstNode->m_arrNeighbor);		

			CreateNodeMaterialList(pDstNode);

		
			iArrayIndex++;
		}
	}
}
//
//
//bool TerrainMaterialCSP::TestHasPermutation(MaterialTypeInfo* a_pMaterialInfo, unsigned int a_iPermutationIndex)
//{
//	unsigned char arrNew[g_iMaxMaterialPerTile];
//	unsigned char arrTest[g_iMaxMaterialPerTile];
//	
//	ApplyPermutation(a_pMaterialInfo->m_arrMaterialIndex, a_iPermutationIndex, arrNew);
//
//	for (unsigned int iPermutationIndex = 0; iPermutationIndex < a_pMaterialInfo->m_arrPermutation.size(); ++iPermutationIndex)
//	{
//		unsigned int iPermutationInfo = a_pMaterialInfo->m_arrPermutation[iPermutationIndex];
//		ApplyPermutation(a_pMaterialInfo->m_arrMaterialIndex, iPermutationInfo, arrTest);
//
//
//		bool bIsEqual = true;
//		for (unsigned int iSlotIndex = 0; iSlotIndex < g_iMaxMaterialPerTile; ++iSlotIndex)
//		{
//			bIsEqual = bIsEqual && (arrNew[iSlotIndex] == arrTest[iSlotIndex]);
//		}
//
//		if (bIsEqual)
//			return true;
//	}
//
//	return false;
//}
//
//
//void TerrainMaterialCSP::FindAllValidPermutation(MaterialTypeInfo* a_pMaterialInfo)
//{
//	unsigned int iPermutationCount = m_arrPermutation.size();
//
//	for (unsigned int iPermutationIndex = 0; iPermutationIndex < iPermutationCount; ++iPermutationIndex)
//	{
//		if (!TestHasPermutation(a_pMaterialInfo, iPermutationIndex))
//		{
//			a_pMaterialInfo->m_arrPermutation.push_back(iPermutationIndex);
//		}		
//	}
//}


void TerrainMaterialCSP::CreateTileConstraintArray(unsigned int a_iSizeX, unsigned int a_iSizeY, unsigned int a_iCountLodLevel)
{
	ASSERT((a_iSizeX % GE_TRTILE_TILESTRIDE) == 1);
	ASSERT((a_iSizeY % GE_TRTILE_TILESTRIDE) == 1);

	unsigned int iTileCountX	= (a_iSizeX - 1) / GE_TRTILE_TILESTRIDE;
	unsigned int iTileCountY	= (a_iSizeY - 1) / GE_TRTILE_TILESTRIDE;

	unsigned int iTileIndex		= 0;

	// assume max lod level
	m_arrTileConstraintNode.reserve((iTileCountX * iTileCountY) + (iTileCountX * iTileCountY) / 3);

	//
	unsigned int iFirstLodLevel = 0;
	unsigned int iLastLodLevel = a_iCountLodLevel;

	iTileCountX >>= iFirstLodLevel;
	iTileCountY >>= iFirstLodLevel;

	for (unsigned int iLodLevel = iFirstLodLevel; iLodLevel < iLastLodLevel; ++iLodLevel)
	{
		for (unsigned int iTileY = 0; iTileY < iTileCountY; ++iTileY)
		{
			for (unsigned int iTileX = 0; iTileX < iTileCountX; ++iTileX)
			{
				m_arrTileConstraintNode.push_back(Node());
				Node* pConstraintNode = &m_arrTileConstraintNode.back();

				iTileIndex++;

				CreateTileConstraint(iTileX, iTileY, iLodLevel, pConstraintNode);
			}
		}



		iTileCountX /= 2;
		iTileCountY /= 2;
	}

}

void TerrainMaterialCSP::CreateTileConstraint(unsigned int a_iTileX, unsigned int a_iTileY, unsigned int a_iLodLevel, Node* a_pConstraintNode)
{
	a_pConstraintNode->m_iPushCount			= 0;
	a_pConstraintNode->m_iProcessedCount	= 0;
	a_pConstraintNode->m_iTestCount			= 0;
	a_pConstraintNode->m_eType				= NodeType_TileConstraint;
	a_pConstraintNode->m_iPosX				= a_iTileX;
	a_pConstraintNode->m_iPosY				= a_iTileY;
	a_pConstraintNode->m_iLodLevel			= a_iLodLevel;	

	//
	unsigned int iNeighborCount = ComputeNeighborCount(a_pConstraintNode);
	m_oAllocator.AllocateArray(iNeighborCount, &a_pConstraintNode->m_arrNeighbor);

	//
	GatherTileMaterialInfo(a_iTileX, a_iTileY, a_iLodLevel, a_pConstraintNode);

	CreateNodeMaterialList(a_pConstraintNode);


	//
	unsigned int iLookAheadDepth = a_iLodLevel == 0 ? g_iLookAheadTileLod0Depth : g_iLookAheadTileLodXDepth;
	unsigned int iLookAheadDepthMax = Util::CountBitSet(a_pConstraintNode->m_iMaterialMask);
	a_pConstraintNode->m_iLookAheadDepth = Util::Min(iLookAheadDepth, iLookAheadDepthMax);
}

void TerrainMaterialCSP::GatherTileMaterialInfo(unsigned int a_iTileX, unsigned int a_iTileY, unsigned int a_iLodLevel, Node* a_pConstraintNode)
{
	unsigned int iStep = 1 << a_iLodLevel;
	unsigned int iMapStartX = (a_iTileX << a_iLodLevel) * GE_TRTILE_TILESTRIDE;
	unsigned int iMapStartY = (a_iTileY << a_iLodLevel) * GE_TRTILE_TILESTRIDE;

	unsigned int iMapEndX = iMapStartX + (GE_TRTILE_TILESTRIDE * iStep) + 1;
	unsigned int iMapEndY = iMapStartY + (GE_TRTILE_TILESTRIDE * iStep) + 1;

	unsigned int iConstraintArrayIndex = 0;

	unsigned int iMaterialMask = 0;
	unsigned int iMaterialCount = 0;

	memset(a_pConstraintNode->m_arrMaterialWeight, 0, sizeof(a_pConstraintNode->m_arrMaterialWeight));

	for (unsigned iY = iMapStartY; iY < iMapEndY; iY += iStep)
	{
		for (unsigned iX = iMapStartX; iX < iMapEndX; iX += iStep)
		{
			ASSERT(iX < m_iMaterialTypeInfoCountX);
			ASSERT(iY < m_iMaterialTypeInfoCountY);

			unsigned int iMaterialTypeIndex = (iY * m_iMaterialTypeInfoCountX) + iX;
			Node* pMaterialDataNode = &m_arrMaterialTypeInfoNode[iMaterialTypeIndex];

			iMaterialMask |= pMaterialDataNode->m_iMaterialMask;

			// add material weights
			for (unsigned int iMaterialIndex = 0; iMaterialIndex < iMaxMaterialCount; ++iMaterialIndex)
			{
				a_pConstraintNode->m_arrMaterialWeight[iMaterialIndex] += pMaterialDataNode->m_arrMaterialWeight[iMaterialIndex];
			}

			a_pConstraintNode->m_arrNeighbor.PushBack(pMaterialDataNode);

			AddTileConstraintToMaterialInfo(pMaterialDataNode, a_pConstraintNode);

			iConstraintArrayIndex++;
		}
	}

	for (unsigned int iMaterialIndex = 0; iMaterialIndex < iMaxMaterialCount; ++iMaterialIndex)
	{
		if ((iMaterialMask & (1 << iMaterialIndex)) != 0)
		{
			iMaterialCount++;
		}
	}

	ASSERT(iConstraintArrayIndex == (GE_TRTILE_VERTEXSTRIDE * GE_TRTILE_VERTEXSTRIDE));
	//ASSERT(iMaterialCount <= g_iMaxMaterialPerTile);


	a_pConstraintNode->m_iMaterialMask = iMaterialMask;	
}

void TerrainMaterialCSP::AddTileConstraintToMaterialInfo(Node* a_pMaterialDataNode, Node* a_pTileConstraintNode)
{
	unsigned int iConstraintCount = a_pMaterialDataNode->m_arrNeighbor.GetCount();

	for (unsigned int i = 0; i < iConstraintCount; ++i)
	{
		if (a_pMaterialDataNode->m_arrNeighbor[i] == a_pTileConstraintNode)
		{
			return;
		}
	}

	a_pMaterialDataNode->m_arrNeighbor.PushBack(a_pTileConstraintNode);
}

/*
void TerrainMaterialCSP::CreateConstraintValidPermutationList(unsigned int a_iTypeIndex, const ConstraintPermutation& a_oPerm, Node* a_pConstraintNode)
{
	if (a_iTypeIndex == (GE_TRTILE_VERTEXSTRIDE * GE_TRTILE_VERTEXSTRIDE))
	{
		MaterialData oData;
		memcpy(oData.m_arrMaterialIndex, a_oPerm.m_arrMaterialIndex, sizeof(oData.m_arrMaterialIndex));
		a_pConstraintNode->m_arrPermutation.push_back(oData);
		return;
	}

	//
	Node* pMaterialDataNode = a_pConstraintNode->m_arrNeighbor[a_iTypeIndex];
	ASSERT(pMaterialDataNode->m_eType == NodeType_HeightmapVertex);

	unsigned int iPermutationCount = pMaterialDataNode->m_arrPermutation.size();

	unsigned int arrMaterial[g_iMaxMaterialPerTile];

	for (unsigned int iPermutationIndex = 0; iPermutationIndex < iPermutationCount; ++iPermutationIndex)
	{
		ApplyPermutation(pMaterialDataNode->m_arrPermutation[0].m_arrMaterialIndex, iPermutationIndex, arrMaterial);

		if (TestCompatibility(arrMaterial, a_oPerm.m_arrMaterialIndex))
		{
			ConstraintPermutation oNewPerm = a_oPerm;

			for (unsigned int iSlotIndex = 0; iSlotIndex < g_iMaxMaterialPerTile; ++iSlotIndex)
			{
				if (oNewPerm.m_arrMaterialIndex[iSlotIndex] == iInvalidMaterialIndex)
				{
					oNewPerm.m_arrMaterialIndex[iSlotIndex] = arrMaterial[iSlotIndex];
				}
			}

			oNewPerm.m_arrPermutationIndex[a_iTypeIndex] = iPermutationIndex;

			CreateConstraintValidPermutationList(a_iTypeIndex + 1, oNewPerm, a_pConstraintNode);
		}
	}
}


bool TerrainMaterialCSP::TestPermutation(Node* a_pConstraintNode, const ConstraintPermutation& a_oPerm, unsigned int a_iTypeIndex, unsigned int iPermutationIndex)
{
	Node* pMaterialDataNode = a_pConstraintNode->m_arrNeighbor[a_iTypeIndex];
	ASSERT(pMaterialDataNode->m_eType == NodeType_HeightmapVertex);

	unsigned int arrMaterial[g_iMaxMaterialPerTile];
	unsigned int arrTest[g_iMaxMaterialPerTile];

	const unsigned int* arrBaseMaterialIndex = pMaterialDataNode->m_arrPermutation[0].m_arrMaterialIndex;

	ApplyPermutation(arrBaseMaterialIndex, iPermutationIndex, arrMaterial);

	for (unsigned int iTileTypeIndex = 0; iTileTypeIndex < a_iTypeIndex; ++iTileTypeIndex)
	{
		Node* pTestMaterialDataNode = a_pConstraintNode->m_arrNeighbor[iTileTypeIndex];
		ASSERT(pMaterialDataNode->m_eType == NodeType_HeightmapVertex);

		ApplyPermutation(arrBaseMaterialIndex, a_oPerm.m_arrPermutationIndex[iTileTypeIndex], arrTest);

		if (!TestCompatibility(arrMaterial, arrTest))
		{
			return false;
		}
	}

	return true;
}

bool TerrainMaterialCSP::TestCompatibility(const MaterialData* a_pData0, const MaterialData* a_pData1)
{
	for (unsigned int iSlotIndex = 0; iSlotIndex < g_iMaxMaterialPerTile; ++iSlotIndex)
	{
		unsigned int iMaterial0Index = a_pData0->GetMaterialIndex(iSlotIndex);
		if (iMaterial0Index == iInvalidMaterialIndex)
			continue;

		for (unsigned int iSlotIndex1 = 0; iSlotIndex1 < g_iMaxMaterialPerTile; ++iSlotIndex1)
		{
			if (iMaterial0Index == a_pData1->GetMaterialIndex(iSlotIndex1))
			{
				if (iSlotIndex != iSlotIndex1)
					return false;

				break;
			}
		}

		//
		unsigned int iMaterial1Index = a_pData1->GetMaterialIndex(iSlotIndex);
		if ((iMaterial1Index != iMaterial0Index) && (iMaterial1Index != iInvalidMaterialIndex))
			return false;
	}

	return true;
}
*/

bool TerrainMaterialCSP::UpdateNode(ChangeList& a_rCurrentChangeList, Node* a_pNode)
{
	ASSERT(a_rCurrentChangeList.m_iDependencyCost == 0);
	ASSERT(a_rCurrentChangeList.m_arrDependency.size() == 0);

	ASSERT(m_arrUpdateList.size() == 0);

	//
	unsigned int iNodePermutationIndex = FindPermutationIndex(a_pNode->m_arrPermutation, a_rCurrentChangeList.m_oPermutation);
	ASSERT(iNodePermutationIndex < a_pNode->m_arrPermutation.GetCount());

	//
	a_rCurrentChangeList.m_arrDependency.push_back(ChangeListElement());
	ChangeListElement& rElem = a_rCurrentChangeList.m_arrDependency.back();
	rElem.m_pNode = a_pNode;
	rElem.m_iPreviousPermutationCount = a_pNode->m_arrPermutation.GetCount();
	std::swap(a_pNode->m_arrPermutation[0], a_pNode->m_arrPermutation[iNodePermutationIndex]);
	a_pNode->m_arrPermutation.SetCount(1);

	//
	a_pNode->m_iPushCount++;
	m_arrUpdateList.push_back(a_pNode);	
	
	//
	while (m_arrUpdateList.size() > 0)
	{
		//
		Node* pNodeToUpdate = m_arrUpdateList.front();
		m_arrUpdateList.pop_front();

		//
		pNodeToUpdate->m_iProcessedCount++;

		if (!UpdateNeighborhood(a_rCurrentChangeList, pNodeToUpdate))
		{
			while (!m_arrUpdateList.empty())
			{
				Node* pNode = m_arrUpdateList.front();
				pNode->m_iProcessedCount = pNode->m_iPushCount;
				m_arrUpdateList.pop_front();
			}

			return false;
		}
	}
	
	return true;
}

bool TerrainMaterialCSP::UpdateNeighborhood(ChangeList& a_rChangeList, const Node* a_pBaseNode)
{
	unsigned int		iNeighborCount			= a_pBaseNode->m_arrNeighbor.GetCount();
	unsigned int		iNeighborValidMinCost	= 0;

	for (unsigned int iNeighborIndex = 0; iNeighborIndex < iNeighborCount; ++iNeighborIndex)
	{
		//
		Node* pNeighbor = a_pBaseNode->m_arrNeighbor[iNeighborIndex];		

		unsigned int iPreviousPermutationCount = pNeighbor->m_arrPermutation.GetCount();

		pNeighbor->m_iTestCount++;

		iNeighborValidMinCost = FilterInvalidNeighborPermutation(a_pBaseNode, pNeighbor);		

		// inconsistency: no valid permutation left
		if (pNeighbor->m_arrPermutation.GetCount() == 0)
		{
			Util::PrintMessage("Failed on Neighbor tile %d:%d/%d/%d\n", pNeighbor->m_eType, pNeighbor->m_iLodLevel, pNeighbor->m_iPosX, pNeighbor->m_iPosY);
			pNeighbor->m_arrPermutation.SetCount(iPreviousPermutationCount);
			return false;
		}

		// did we remove any elements ?
		if (pNeighbor->m_arrPermutation.GetCount() != iPreviousPermutationCount)
		{
			a_rChangeList.m_iDependencyCost += iNeighborValidMinCost;

			a_rChangeList.m_arrDependency.push_back(ChangeListElement());
			ChangeListElement& rElem = a_rChangeList.m_arrDependency.back();
			rElem.m_pNode = pNeighbor;		
			rElem.m_iPreviousPermutationCount = iPreviousPermutationCount;

			if (pNeighbor->m_iPushCount == pNeighbor->m_iProcessedCount)
			{
				m_arrUpdateList.push_back(pNeighbor);
				pNeighbor->m_iPushCount++;
			}
		}
	}

	return true;
}

unsigned int TerrainMaterialCSP::FilterInvalidNeighborPermutation(const Node* a_pBaseNode, Node* a_pNeighbor)
{
	unsigned int			iValidMinCost				= g_iMaxCost;
	unsigned int			iBasePermutationCount		= a_pBaseNode->m_arrPermutation.GetCount();
	unsigned int			iNeighborPermutationIndex	= 0;
	MaterialDataArray&		rArray						= a_pNeighbor->m_arrPermutation;

	ASSERT(a_pBaseNode->m_eType != a_pNeighbor->m_eType);

	bool bIsTile = (a_pBaseNode->m_eType == NodeType_TileConstraint);

	while (iNeighborPermutationIndex < rArray.GetCount())
	{
		const MaterialData* pNeighborPermutation = &rArray[iNeighborPermutationIndex];

		bool bFoundValidMatch = false;

		for (unsigned int iBasePermutationIndex = 0; iBasePermutationIndex < iBasePermutationCount; ++iBasePermutationIndex)
		{
			const MaterialData* pBasePermutation = &a_pBaseNode->m_arrPermutation[iBasePermutationIndex];
			
			if (bIsTile)
			{
				bFoundValidMatch = pBasePermutation->IsTileCompatible(*pNeighborPermutation);
			}
			else
			{
				bFoundValidMatch = pNeighborPermutation->IsTileCompatible(*pBasePermutation);
			}

			if (bFoundValidMatch)
				break;
		}

		if (!bFoundValidMatch)
		{
			unsigned int iCurrentCount = rArray.GetCount();
			std::swap(rArray[iCurrentCount - 1], rArray[iNeighborPermutationIndex]);
			rArray.SetCount(iCurrentCount - 1);
		}
		else
		{
			iValidMinCost = Util::Min(iValidMinCost, (unsigned int)pNeighborPermutation->m_iCost);
			iNeighborPermutationIndex++;
		}
	}

	return iValidMinCost;
}


void TerrainMaterialCSP::RevertChangeList(ChangeList& a_rChangeList)
{
	unsigned int iCountToRevert = a_rChangeList.m_arrDependency.size();

	while (iCountToRevert > 0)
	{
		iCountToRevert--;
		ChangeListElement& rElement = a_rChangeList.m_arrDependency[iCountToRevert];
		MaterialDataArray& rArray = rElement.m_pNode->m_arrPermutation;
		rArray.SetCount(rElement.m_iPreviousPermutationCount);
	}
}


unsigned int TerrainMaterialCSP::FindBestPermutationIndex(const Node* a_pNode, const std::vector<MaterialData>& a_iExcludeList)
{
	unsigned int iMinCost				= 0xFF;
	unsigned int iBestPermutationIndex	= 0;
	unsigned int iCount					= a_pNode->m_arrPermutation.GetCount();

	for (unsigned int i = 0; i < iCount; ++i)
	{
		if (a_pNode->m_arrPermutation[i].m_iCost < iMinCost)
		{
			unsigned int iSearchIndex = FindPermutationIndex(a_iExcludeList, a_pNode->m_arrPermutation[i]);

			if (iSearchIndex == a_iExcludeList.size())
			{
				iMinCost				= a_pNode->m_arrPermutation[i].m_iCost;
				iBestPermutationIndex	= i;
			}
		}
	}

	return iBestPermutationIndex;
}


unsigned int TerrainMaterialCSP::FindBestLookAheadPermutationIndex(const Node* a_pNode, const std::vector<MaterialData>& a_iExcludeList)
{
	unsigned int iMinCost				= 0xFF;
	unsigned int iBestPermutationIndex	= 0;
	unsigned int iCount					= a_pNode->m_arrPermutation.GetCount();

	for (unsigned int i = 0; i < iCount; ++i)
	{
		const MaterialData& rNodeData = a_pNode->m_arrPermutation[i];

		if (rNodeData.m_iCost < iMinCost)
		{
			bool bValid = true;

			unsigned int iExcludedPermutationCount = a_iExcludeList.size();
			for (unsigned int iExcludedPermutationIndex = 0; iExcludedPermutationIndex < iExcludedPermutationCount; ++iExcludedPermutationIndex)
			{
				const MaterialData& rTestData = a_iExcludeList[iExcludedPermutationIndex];

				if (rTestData.IsEqual(rNodeData))
				{
						bValid = false;
						break;
				}		
			}

			if (bValid)
			{
				iMinCost				= a_pNode->m_arrPermutation[i].m_iCost;
				iBestPermutationIndex	= i;
			}
		}
	}

	return iBestPermutationIndex;
}


void TerrainMaterialCSP::CreateVertexWeightArray()
{
	unsigned int arrWeight[g_iMaxMaterialPerTile];

	unsigned iDataCountX = m_iMaterialTypeInfoCountX - 1;
	unsigned iDataCountY = m_iMaterialTypeInfoCountY - 1;

	m_arrVertexWeight.resize(iDataCountX * iDataCountY);
	
	for (unsigned int iY = 0; iY < iDataCountY; ++iY)
	{
		for (unsigned int iX = 0; iX < iDataCountX; ++iX)
		{
			Node* pVertexNode = &m_arrMaterialTypeInfoNode[(iY * m_iMaterialTypeInfoCountX) + iX];
			ASSERT(pVertexNode->m_arrPermutation.GetCount() == 1);

			if (pVertexNode->m_arrPermutation.GetCount() != 1)
			{
				Util::PrintMessage("Vertex Node %d/%d not solved!\n", pVertexNode->m_iPosX, pVertexNode->m_iPosY);
			}

			//		
			memset(arrWeight, 0, sizeof(arrWeight));

			NormalizeNodeMaterialWeight(pVertexNode, arrWeight, 0xFF);

			m_arrVertexWeight[(iY * iDataCountX) + iX] = CreateMaterialWeight(arrWeight);
		}
	}
}

void TerrainMaterialCSP::NormalizeNodeMaterialWeight(Node* a_pNode, unsigned int a_arrWeight[g_iMaxMaterialPerTile], unsigned int a_iFactor)
{
	MaterialData* pSelectedPermutation = &a_pNode->m_arrPermutation[0];

	//
	unsigned int iTotalWeight = 0;

	for (unsigned int iActiveMaterial = 0; iActiveMaterial < g_iMaxMaterialPerTile; ++iActiveMaterial)
	{
		if (pSelectedPermutation->GetMaterialIndex(iActiveMaterial) != iInvalidMaterialIndex)
		{
			unsigned int iMaterialIndex = pSelectedPermutation->GetMaterialIndex(iActiveMaterial);
			unsigned int iMaterialWeight = a_pNode->m_arrMaterialWeight[iMaterialIndex];

			a_arrWeight[iActiveMaterial] = iMaterialWeight;

			iTotalWeight += iMaterialWeight;
		}
	}

	ASSERT(iTotalWeight > 0);

	unsigned int iAssignedWeight = 0;

	for (unsigned int iActiveMaterial = 0; iActiveMaterial < g_iMaxMaterialPerTile; ++iActiveMaterial)
	{
		unsigned int iPreviousWeight = a_arrWeight[iActiveMaterial];
		unsigned int iNormalizedWeight = (iPreviousWeight * a_iFactor) / iTotalWeight;

		a_arrWeight[iActiveMaterial] = iNormalizedWeight;


		iAssignedWeight += iNormalizedWeight;
	}

	ASSERT(a_iFactor >= iAssignedWeight);
	unsigned int iLeftOverWeight = a_iFactor - iAssignedWeight;
	ASSERT(iLeftOverWeight < g_iMaxMaterialPerTile);

	while (iLeftOverWeight > 0)
	{
		for (unsigned int iActiveMaterial = 0; iActiveMaterial < g_iMaxMaterialPerTile; ++iActiveMaterial)
		{
			if (a_arrWeight[iActiveMaterial] > 0)
			{
				a_arrWeight[iActiveMaterial]++;
				iLeftOverWeight--;
			}

			if (iLeftOverWeight == 0)
				break;
		}		
	}
}

unsigned int TerrainMaterialCSP::CreateMaterialWeight(unsigned int a_arrWeight[g_iMaxMaterialPerTile])
{
	unsigned int iResult = 0;
	unsigned int iShift = 24;

	for (unsigned int iActiveMaterial = 0; iActiveMaterial < (g_iMaxMaterialPerTile - 1); ++iActiveMaterial)
	{
		iResult |= (a_arrWeight[iActiveMaterial] << iShift);

		iShift -= 8;
	}

	return iResult;
}



void TerrainMaterialCSP::CreateTileMaterialData()
{
	m_arrTileMaterialType.clear();	

	unsigned int iTileCount = m_arrTileConstraintNode.size();
	for (unsigned int iTile = 0; iTile < iTileCount; ++iTile)
	{
		Node* pTileNode = &m_arrTileConstraintNode[iTile];

		if (pTileNode->m_arrPermutation.GetCount() != 1)
		{
			Util::PrintMessage("Tile Node %d/%d not solved!\n", pTileNode->m_iPosX, pTileNode->m_iPosY);
		}

		ASSERT(pTileNode->m_arrPermutation.GetCount() == 1);
		MaterialData* pSelectedPermutation = &pTileNode->m_arrPermutation[0];

		Permutation oPerm;
		for (unsigned int iMaterialSlot = 0; iMaterialSlot < g_iMaxMaterialPerTile; ++iMaterialSlot)
		{
			oPerm.m_arrIndex[iMaterialSlot] = pSelectedPermutation->GetMaterialIndex(iMaterialSlot);
		}

		//
		if (std::find(m_arrTileMaterialType.begin(), m_arrTileMaterialType.end(), oPerm) == m_arrTileMaterialType.end())
		{
			m_arrTileMaterialType.push_back(oPerm);
		}		
	}

	m_arrTileMaterialTypeIndex.resize(iTileCount);

	for (unsigned int iTile = 0; iTile < iTileCount; ++iTile)
	{
		Node* pTileNode = &m_arrTileConstraintNode[iTile];

		ASSERT(pTileNode->m_arrPermutation.GetCount() == 1);
		MaterialData* pSelectedPermutation = &pTileNode->m_arrPermutation[0];

		Permutation oPerm;
		for (unsigned int iMaterialSlot = 0; iMaterialSlot < g_iMaxMaterialPerTile; ++iMaterialSlot)
		{
			oPerm.m_arrIndex[iMaterialSlot] = pSelectedPermutation->GetMaterialIndex(iMaterialSlot);
		}

		std::vector<Permutation>::iterator i = std::find(m_arrTileMaterialType.begin(), m_arrTileMaterialType.end(), oPerm);
		ASSERT(i != m_arrTileMaterialType.end());

		unsigned int iMaterialTypeIndex = i - m_arrTileMaterialType.begin();

		m_arrTileMaterialTypeIndex[iTile] = iMaterialTypeIndex;
	}
}



unsigned int TerrainMaterialCSP::ComputePermutationCount(unsigned int a_iMaterialMask)
{
	unsigned int iMaterialCount = Util::CountBitSet(a_iMaterialMask);
	unsigned int iFreeMaterialCount = g_iMaxMaterialPerTile - iMaterialCount;

	//
	unsigned int iFact = 1;

	while (iMaterialCount > 0)
	{
		iMaterialCount--;
		iFact *= (g_iMaxMaterialPerTile - iMaterialCount);
	}

	return iFact;
}


unsigned int TerrainMaterialCSP::ComputeNeighborCount(const Node* a_pNode)
{
	if (a_pNode->m_eType == NodeType_TileConstraint)
		return 25;


	unsigned int iPosX				= a_pNode->m_iPosX;
	unsigned int iPosY				= a_pNode->m_iPosY;
	unsigned int iLevelWidth		= m_iMaterialTypeInfoCountX - 1;
	unsigned int iLevelHeight		= m_iMaterialTypeInfoCountY - 1;

	unsigned int iNeighborCount = 0;

	for (unsigned int iLodLevel = 0; iLodLevel < m_iMaterialTypeInfoCountLodLevel; ++iLodLevel)
	{
		bool bIntersectionX = ((iPosX % GE_TRTILE_TILESTRIDE) == 0) && (iPosX > 0) && (iPosX < iLevelWidth);
		bool bIntersectionY = ((iPosY % GE_TRTILE_TILESTRIDE) == 0) && (iPosY > 0) && (iPosY < iLevelHeight);

		if (!bIntersectionY && !bIntersectionX)
		{
			iNeighborCount += 1;
		}
		else if (bIntersectionX && bIntersectionY)
		{
			iNeighborCount += 4;
		}
		else
		{
			iNeighborCount += 2;
		}

		if (((iPosX % 2) != 0) || ((iPosY % 2) != 0))
			break;

		iPosX /= 2;
		iPosY /= 2;

		iLevelWidth /= 2;
		iLevelHeight /= 2;
	} 

	return iNeighborCount;
}
