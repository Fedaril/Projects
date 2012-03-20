

#pragma once

#include "MaterialConvertionData.h"
#include "StackAllocator.h"
#include <vector>
#include <deque>


const unsigned int g_iMaxMaterialPerTile	= 5;
const unsigned int g_iMaxTileConstraint		= 16;
const unsigned int g_iMaxCost				= 0x0000FFFF;

const unsigned int g_iLookAheadMaxTest			= 8;
const unsigned int g_iLookAheadMaxCountPerType	= 8;
const unsigned int g_iLookAheadTileLod0Depth	= 1;
const unsigned int g_iLookAheadTileLodXDepth	= 1;
const unsigned int g_iLookAheadVertexDepth		= 1;

struct Permutation;
struct MaterialTypeInfo;
struct TileConstraint;
struct ConstraintPermutation;



struct Permutation
{
	unsigned char m_arrIndex[g_iMaxMaterialPerTile];	

	bool operator==(const Permutation& oTest) const
	{
		for (unsigned int iSlotIndex = 0; iSlotIndex < g_iMaxMaterialPerTile; ++iSlotIndex)
		{
			if (m_arrIndex[iSlotIndex] != oTest.m_arrIndex[iSlotIndex])
				return false;

		}

		return true;
	}
};



//////////////
/*
struct MaterialTypeInfo
{		
	unsigned int					m_arrMaterialIndex[g_iMaxMaterialPerTile];
	unsigned int					m_bLocked;
	unsigned int					m_iPermutationCount;
	TileConstraint*					m_arrTileConstraint[g_iMaxTileConstraint];
};



// tile constraint: must be compatible
struct TileConstraint
{
	//
	unsigned int						m_iPosX;
	unsigned int						m_iPosY;
	unsigned int						m_iLodLevel;
	unsigned int						m_iMaterialMask;

	//
	MaterialTypeInfo*					m_arrMaterialTypeInfo[GE_TRTILE_VERTEXSTRIDE * GE_TRTILE_VERTEXSTRIDE];
	std::vector<ConstraintPermutation>	m_arrAllPermutation;
	std::vector<unsigned int>			m_arrValidPermutation; // index into m_arrAllPermutation array
};

*/

class TerrainMaterialCSP
{
public:
	//
	enum NodeType
	{
		NodeType_HeightmapVertex,
		NodeType_TileConstraint,

		NodeType_Max
	};

	//

	struct MaterialData
	{
		union
		{
			struct		
			{
				unsigned int				m_arrMaterial0		: 4;
				unsigned int				m_arrMaterial1		: 4;
				unsigned int				m_arrMaterial2		: 4;
				unsigned int				m_arrMaterial3		: 4;
				unsigned int				m_arrMaterial4		: 4;				
				unsigned int				m_iCost				: 12;
			};

			unsigned int m_iAsInt;
		};

		unsigned int m_iMask;
		
		MaterialData() : m_iAsInt(0), m_iMask(0) {}

		inline void Clear()
		{
			m_iAsInt = 0;
			m_iMask = 0;
		}

		inline void SetMaterialIndex(unsigned int a_iSlot, unsigned int a_iValue)
		{
			ASSERT(a_iSlot < g_iMaxMaterialPerTile);
			ASSERT(a_iValue <= iMaxMaterialCount);

			unsigned int iShift = a_iSlot * 4;
			unsigned int iMask = 0xF << iShift;
			m_iAsInt	= (m_iAsInt & ~iMask) | (a_iValue << iShift);

			if (a_iValue == iInvalidMaterialIndex)
			{
				m_iMask &= ~iMask;
			}
			else
			{
				m_iMask |= iMask;
			}
			
		}

		inline unsigned int GetMaterialIndex(unsigned int a_iSlot) const
		{
			ASSERT(a_iSlot < g_iMaxMaterialPerTile);
			return (m_iAsInt >> (a_iSlot * 4)) & 0xF;
		}

		inline bool IsEqual(const MaterialData& a_rData) const
		{
			return m_iAsInt == a_rData.m_iAsInt;
		}

		__forceinline bool IsTileCompatible(const MaterialData& a_rData) const
		{
			// this = tile
			// rData = vertex;
			unsigned int iMasked = a_rData.m_iAsInt & a_rData.m_iMask;
			bool bRes = (iMasked == (m_iAsInt & a_rData.m_iMask));
			return bRes;

/*
			for (unsigned int iSlotIndex = 0; iSlotIndex < g_iMaxMaterialPerTile; ++iSlotIndex)
			{
				unsigned int iMaterialIndex = a_rData.GetMaterialIndex(iSlotIndex);
				if ((iMaterialIndex != iInvalidMaterialIndex) && (iMaterialIndex != GetMaterialIndex(iSlotIndex)))
				{
					ASSERT(bRes == false);
					return false;
				}
			}

			ASSERT(bRes == true);
			return true;
*/
		}


	};


/*
	struct MaterialData
	{
		unsigned char				m_arrMaterial[g_iMaxMaterialPerTile];
		unsigned char				m_iCost;

		inline void SetMaterialIndex(unsigned int a_iSlot, unsigned int a_iValue)
		{
			ASSERT(a_iSlot < g_iMaxMaterialPerTile);
			ASSERT(a_iValue <= iMaxMaterialCount);

			m_arrMaterial[a_iSlot] = (unsigned char)a_iValue;
		}

		inline unsigned int GetMaterialIndex(unsigned int a_iSlot) const
		{
			ASSERT(a_iSlot < g_iMaxMaterialPerTile);

			return m_arrMaterial[a_iSlot];
		}

		inline bool IsEqual(const MaterialData& a_rData) const
		{
			return (memcmp(m_arrMaterial, a_rData.m_arrMaterial, sizeof(m_arrMaterial)) == 0);
		}
	};
*/
	struct LookAheadResult
	{
		bool							m_bIsValid;
		unsigned int					m_iLowedCost;
		MaterialData					m_oPermutation;
	};



	//
	typedef StackAllocatorArray<MaterialData>	MaterialDataArray;

	//
	struct Node;
	typedef StackAllocatorArray<Node*>			NeighborArray;

	//
	StackAllocator								m_oAllocator;


	//
	struct Node
	{	
		unsigned int				m_iPosX;
		unsigned int				m_iPosY;
		unsigned int				m_iLodLevel;
		unsigned int				m_iMaterialMask;
		NodeType					m_eType;
		unsigned int				m_iPushCount;
		unsigned int				m_iProcessedCount;
		unsigned int				m_iTestCount;		
		unsigned int				m_iLookAheadDepth;

		//
		MaterialDataArray			m_arrPermutation;
		NeighborArray				m_arrNeighbor;

		//
		unsigned int				m_arrMaterialWeight[iMaxMaterialCount];
	};

	//
	struct ChangeListElement
	{
		Node*						m_pNode;
		unsigned int				m_iPreviousPermutationCount;
	};

	struct ChangeList
	{		
		MaterialData					m_oPermutation;
		unsigned int					m_iDependencyCost;

		std::vector<MaterialData>		m_arrFailedPermutation;
		std::vector<ChangeListElement>	m_arrDependency;

		bool							m_bLookAheadDone;
		LookAheadResult					m_oLookAheadResult;
	};
	


	/*
	//
	struct ConstraintPermutation
	{
		unsigned int						m_arrMaterialIndex[g_iMaxMaterialPerTile];
		unsigned int						m_arrPermutationIndex[GE_TRTILE_VERTEXSTRIDE * GE_TRTILE_VERTEXSTRIDE];
	};
	*/


public:
						TerrainMaterialCSP();
						~TerrainMaterialCSP();


	void				Initialize(eSMaterialConvertionData* a_arrMaterialData, unsigned int a_iSizex, unsigned int a_iSizeY, unsigned int a_iCountLodLevel);
	void				Shutdown();


	bool				Solve();
	unsigned int		GetSolutionCost();

	void				CreateVertexWeightArray();
	void				CreateTileMaterialData();


	//
	const std::vector<unsigned int>&			GetVertexWeightArray();
	const std::vector<Permutation>&				GetTileMaterialTypeArray();
	const std::vector<unsigned short>&			GetTileMaterialTypeIndexArray();


private:
	void				Clear();


private:
	
	std::vector<Permutation>		m_arrPermutation;

	unsigned int					m_iMaterialTypeInfoCountX;
	unsigned int					m_iMaterialTypeInfoCountY;
	unsigned int					m_iMaterialTypeInfoCountLodLevel;

	std::vector<Node>				m_arrMaterialTypeInfoNode;	// list all elements

	std::vector<Node>				m_arrTileConstraintNode;	// list all constraints on elements
		

	//
	std::vector<ChangeList>			m_arrChangeList;

	//
	std::deque<Node*>				m_arrUpdateList;

	std::vector<unsigned int>		m_arrVertexWeight;
	std::vector<Permutation>		m_arrTileMaterialType;
	std::vector<unsigned short>		m_arrTileMaterialTypeIndex;

	unsigned int					m_iSolutionCost;

private:
	void								CreateNodeList(std::vector<Node*>& a_arrNode);
	void								CreateLod0TileList(std::vector<Node*>& a_arrNode);

	bool								ProcessNodeList(std::vector<Node*>& a_arrNode);


	void								GenerateAllPermutations();
	void								GeneratePermutation(Permutation oPerm, int iDepth, int iMaxDepth);
	void								ApplyPermutation(const unsigned char a_arrSrc[g_iMaxMaterialPerTile], unsigned int a_iPermutationIndex, MaterialData* a_pData);
	void								CreateNodePermutationList(unsigned int a_iMaterialMask, unsigned int a_iCost, MaterialDataArray& a_arrPermutation);
	void								CreateNodeMaterialCombinationList(unsigned int a_iBaseMaterialMask, unsigned int a_iRemainingMaterialMask, unsigned int a_iDepth, std::vector<unsigned int>& a_arrCombination);
	void								CreateNodeMaterialList(Node* a_pNode);
	unsigned int						ComputeCombinationCost(const Node* a_pNode, unsigned int a_iCombinationMask);


	void								SetLodLevel(unsigned int a_iSizeX, unsigned int a_iSizeY, unsigned int a_iMaxLodLevel);
	void								CreateMaterialTypeInfoArray(eSMaterialConvertionData* a_arrMaterialData, unsigned int a_iSizeX, unsigned int a_iSizeY);
	//void								FindAllValidPermutation(MaterialTypeInfo* a_pMaterialInfo);
	//bool								TestHasPermutation(MaterialTypeInfo* a_pMaterialInfo, unsigned int a_iPermutationIndex);

	void								CreateTileConstraintArray(unsigned int a_iSizeX, unsigned int a_iSizeY, unsigned int a_iLodLevelCount);
	void								CreateTileConstraint(unsigned int a_iTileX, unsigned int a_iTileY, unsigned int a_iLodLevel, Node* a_pConstraintNode);
	void								GatherTileMaterialInfo(unsigned int a_iTileX, unsigned int a_iTileY, unsigned int a_iLodLevel, Node* a_pConstraintNode);
	void								AddTileConstraintToMaterialInfo(Node* a_pMaterialInfoNode, Node* a_pTileConstraintNode);

	//void								CreateConstraintValidPermutationList(unsigned int a_iTypeIndex, const ConstraintPermutation& a_oPerm, Node* a_pConstraintNode);
	//bool								TestPermutation(Node* a_pConstraintNode, const ConstraintPermutation& a_oPerm, unsigned int a_iTypeIndex, unsigned int iPermutationIndex);
	//bool								TestCompatibility(const MaterialData* a_pData0, const MaterialData* a_pData1);

	bool								UpdateNode(ChangeList& a_rCurrentChangeList, Node* a_pNode);

	bool								UpdateNeighborhood(ChangeList& a_rChangeList, const Node* a_pBaseNode);
	unsigned int						FilterInvalidNeighborPermutation(const Node* a_pBaseNode, Node* a_pNeighbor);


	void								RevertChangeList(ChangeList& a_rChangeList);

	unsigned int						FindBestPermutationIndex(const Node* a_pNode, const std::vector<MaterialData>& a_iExcludeList);
	unsigned int						FindBestLookAheadPermutationIndex(const Node* a_pNode, const std::vector<MaterialData>& a_iExcludeList);


	LookAheadResult						LookAhead(Node** a_pNode, unsigned int a_iNodeCount, unsigned int a_iDepth);

	unsigned int						FindPermutationIndex(const MaterialDataArray& a_rArray, const MaterialData& a_rPermutation);
	unsigned int						FindPermutationIndex(const std::vector<MaterialData>& a_rArray, const MaterialData& a_rPermutation);

	//
	void								NormalizeNodeMaterialWeight(Node* a_pNode, unsigned int a_arrWeight[g_iMaxMaterialPerTile], unsigned int a_iFactor);
	unsigned int						CreateMaterialWeight(unsigned int a_arrWeight[g_iMaxMaterialPerTile]);

	//
	static bool							CompareNode(Node* a_pNode0, Node* a_pNode1);

	//
	unsigned int						ComputePermutationCount(unsigned int a_iMaterialMask);
	unsigned int						ComputeNeighborCount(const Node* a_pNode);
};


inline const std::vector<unsigned int>& TerrainMaterialCSP::GetVertexWeightArray()
{
	return m_arrVertexWeight;
}

inline const std::vector<Permutation>& TerrainMaterialCSP::GetTileMaterialTypeArray()
{
	return m_arrTileMaterialType;
}

inline const std::vector<unsigned short>& TerrainMaterialCSP::GetTileMaterialTypeIndexArray()
{
	return m_arrTileMaterialTypeIndex;
}

inline unsigned int TerrainMaterialCSP::GetSolutionCost()
{
	return m_iSolutionCost;
}
