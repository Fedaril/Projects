
#pragma once

#include "wtypes.h"

namespace ForceField2D
{

class Simulator;
class Multithread;

////
enum WorkerCommand
{
	WorkerCommand_None,
	WorkerCommand_UpdateSpringLength,
	WorkerCommand_ApplySpringForce,	
	WorkerCommand_Sync,
	WorkerCommand_Exit,
	WorkerCommand_Count
};

struct WorkerDataUpdateSpringLength
{	
	unsigned int		m_iSpringIndex;
	unsigned int		m_iSpringCount;
};

struct WorkerDataApplySpringForce
{	
	unsigned int		m_iVertexIndex;
	unsigned int		m_iVertexCount;
	float				m_fUpdateStep;
};

struct WorkerDataSync
{
};

struct WorkerJobData
{
	WorkerCommand		m_eCommand;

	struct
	{
		WorkerDataUpdateSpringLength	asUpdateSpringLength;
		WorkerDataApplySpringForce		asApplySpringForce;
		WorkerDataApplySpringForce		asSync;
	} m_oCommandData;
};

struct WorkerData
{
	Multithread*		m_pThis;
	unsigned int		m_iIndex;

	WorkerCommand		m_eCommand;
	HANDLE				m_hEvent;

	float				m_fTotalError;
	float				m_fMaxAmplitude;

	WorkerJobData		m_oJobData;
};



class Multithread
{
public:
	//
	static const unsigned int s_iMaxWorker = 8;


private:	
	unsigned int			m_iWorkerCount;
	WorkerData				m_arrWorkerData[s_iMaxWorker];
	HANDLE					m_arrThread[s_iMaxWorker];
	HANDLE					m_arrThreadEvent[s_iMaxWorker];

	WorkerJobData*			m_arrJob;
	
	//
	Simulator*				m_pSimulator;

	float					m_fPreviousMaxForceAmplitude;

public:
							Multithread();
							~Multithread();

	void					Initialize(Simulator* a_pSimulator, unsigned int a_iWorkerCount);
	void					Shutdown();

	float					Simulate(float a_fDt);

private:
	void					CreateWorkerThreads(Simulator* a_pSimulator, unsigned int a_iWorkerCount);

	void					WaitForIdleWorkerThreads();


	float					UpdateSpringLength(unsigned int a_iStartIndex, unsigned int a_iCount);
	float					ApplySpringForce(unsigned int a_iStartIndex, unsigned int a_iCount, float a_fUpdateStep);
	
	friend DWORD WINAPI		ThreadMain(LPVOID a_pParam);
};


}