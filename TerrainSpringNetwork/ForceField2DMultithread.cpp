
#include "StdAfx.h"

#include "ForceField2DMultithread.h"
#include "ForceField2DSimulator.h"
#include "Util.h"
#include "Timer.h"
#include "assert.h"
#include "stdio.h"

/*
//---------------------------------------------------------
// Called at the end of each itaration, it will "randezvous"
// (meet) all the threads before returning (so that next
// iteration can begin). In practical terms this function
// will block until all the other threads finish their iteration.
static void RandezvousOthers(SyncInfo &sync, int ordinal)
{
    if (0 == ::InterlockedDecrement(&(sync.Awaiting))) { // are we the last ones to arrive?
        // at this point, all the other threads are blocking on the semaphore
        // so we can manipulate shared structures without having to worry
        // about conflicts
        sync.Awaiting = sync.ThreadsCount;
        wprintf(L"Thread %d is the last to arrive, releasing synchronization barrier\n", ordinal);
        wprintf(L"---~~~---\n");

        // let's release the other threads from their slumber
        // by using the semaphore
        ::ReleaseSemaphore(sync.Semaphore, sync.ThreadsCount - 1, 0); // "ThreadsCount - 1" because this last thread will not block on semaphore
    }
    else { // nope, there are other threads still working on the iteration so let's wait
        wprintf(L"Thread %d is waiting on synchronization barrier\n", ordinal);
        ::WaitForSingleObject(sync.Semaphore, INFINITE); // note that return value should be validated at this point ;)
    }
}


*/



namespace 
{
// some code I found on the internet to rename threads...
const DWORD MS_VC_EXCEPTION=0x406D1388;

#pragma pack(push,8)
typedef struct tagTHREADNAME_INFO
{
   DWORD dwType; // Must be 0x1000.
   LPCSTR szName; // Pointer to name (in user addr space).
   DWORD dwThreadID; // Thread ID (-1=caller thread).
   DWORD dwFlags; // Reserved for future use, must be zero.
} THREADNAME_INFO;
#pragma pack(pop)

void SetThreadName( DWORD dwThreadID, char* threadName)
{
   THREADNAME_INFO info;
   info.dwType = 0x1000;
   info.szName = threadName;
   info.dwThreadID = dwThreadID;
   info.dwFlags = 0;

   __try
   {
      RaiseException( MS_VC_EXCEPTION, 0, sizeof(info)/sizeof(ULONG_PTR), (ULONG_PTR*)&info );
   }
   __except(EXCEPTION_EXECUTE_HANDLER)
   {
   }
}


} // namespace anonymous



namespace ForceField2D
{


DWORD WINAPI ThreadMain(LPVOID a_pParam)
{		
	volatile WorkerData* pLocalWorkerData = (WorkerData*)a_pParam;	

	bool bContinue = true;

	Multithread* pThis = pLocalWorkerData->m_pThis;
	unsigned int iWorkerIndex = pLocalWorkerData->m_iIndex;
	
	assert(iWorkerIndex < pThis->m_iWorkerCount);

	while (bContinue)
	{
		DWORD dwRes = WaitForSingleObject(pLocalWorkerData->m_hEvent, INFINITE);

		assert(&pThis->m_arrWorkerData[iWorkerIndex] == pLocalWorkerData);

		WorkerCommand eNewCommand = pLocalWorkerData->m_eCommand;
		assert (eNewCommand != WorkerCommand_None);

		switch (eNewCommand)
		{
			case WorkerCommand_UpdateSpringLength:
			{
				volatile WorkerDataUpdateSpringLength* pData = &pLocalWorkerData->m_oJobData.m_oCommandData.asUpdateSpringLength;
				pLocalWorkerData->m_fTotalError = pThis->UpdateSpringLength(pData->m_iSpringIndex, pData->m_iSpringCount);
				break;
			}

			case WorkerCommand_ApplySpringForce:
			{
				volatile WorkerDataApplySpringForce* pData = &pLocalWorkerData->m_oJobData.m_oCommandData.asApplySpringForce;
				pLocalWorkerData->m_fMaxAmplitude = pThis->ApplySpringForce(pData->m_iVertexIndex, pData->m_iVertexCount, pData->m_fUpdateStep);
				break;

			}

			case WorkerCommand_Exit:
			{
				Util::PrintMessage("Worker Thread #%d exiting\n", iWorkerIndex);
				bContinue = false;
				break;
			}

			default:
			{
				assert(0);
				break;
			}			
		}

		pLocalWorkerData->m_eCommand = WorkerCommand_None;

		::SetEvent(pThis->m_arrThreadEvent[iWorkerIndex]);
	}

	return 0;
}


Multithread::Multithread()
: m_iWorkerCount(0)
, m_pSimulator(NULL)
, m_fPreviousMaxForceAmplitude(1.0f)
{
	for (unsigned int iThreadIndex = 0; iThreadIndex < s_iMaxWorker; ++iThreadIndex)
	{
		m_arrThread[iThreadIndex]		= INVALID_HANDLE_VALUE;
		m_arrThreadEvent[iThreadIndex] = INVALID_HANDLE_VALUE;

		memset(&m_arrWorkerData[s_iMaxWorker], 0, sizeof(WorkerData));
	}
}

Multithread::~Multithread()
{
	Shutdown();
}

void Multithread::Initialize(Simulator* a_pSimulator, unsigned int a_iWorkerCount)
{
	m_pSimulator					= a_pSimulator;
	m_fPreviousMaxForceAmplitude	= (float)a_pSimulator->GetVertexCount();

	CreateWorkerThreads(a_pSimulator, a_iWorkerCount);	
}

void Multithread::Shutdown()
{
	DWORD	dwRes = 0;
	BOOL	bRes = 0;

	if (m_iWorkerCount == 0)
		return;

	for (unsigned int iWorkerIndex = 0; iWorkerIndex < m_iWorkerCount; ++iWorkerIndex)
	{
		m_arrWorkerData[iWorkerIndex].m_eCommand = WorkerCommand_Exit;
		::SetEvent(m_arrWorkerData[iWorkerIndex].m_hEvent);
	}

	//
	dwRes = ::WaitForMultipleObjects(m_iWorkerCount, m_arrThreadEvent, TRUE, 5000);
	assert(dwRes == WAIT_OBJECT_0);

	//
	dwRes = WaitForMultipleObjects(m_iWorkerCount, m_arrThread, TRUE, INFINITE);			
	Util::PrintMessage("WaitForSingleObject returned: %d LastError: %d\n", dwRes, ::GetLastError());

	for (unsigned int iWorkerIndex = 0; iWorkerIndex < m_iWorkerCount; ++iWorkerIndex)
	{
		bRes = CloseHandle(m_arrThreadEvent[iWorkerIndex]);
		if (bRes == FALSE)
		{
			Util::PrintMessage("CloseHandle failed: LastError: %d\n", ::GetLastError());
		}
		m_arrThreadEvent[iWorkerIndex] = INVALID_HANDLE_VALUE;


		if (CloseHandle(m_arrWorkerData[iWorkerIndex].m_hEvent) == FALSE)
		{
			Util::PrintMessage("CloseHandle failed: LastError: %d\n", ::GetLastError());
		}
		m_arrWorkerData[iWorkerIndex].m_hEvent = INVALID_HANDLE_VALUE;

		bRes = CloseHandle(m_arrThread[iWorkerIndex]);
		if (bRes == FALSE)
		{
			Util::PrintMessage("CloseHandle failed: LastError: %d\n", ::GetLastError());
		}

		m_arrThread[iWorkerIndex] = INVALID_HANDLE_VALUE;
	}

	m_pSimulator = NULL;
	m_iWorkerCount = 0;
}


float Multithread::Simulate(float a_fDt)
{
	static double	fTotalTime	= 0.0f;
	Util::Timer		oTimerGlobal;

	// UpdateSpringLength
	for (unsigned int iWorkerIndex = 0; iWorkerIndex < m_iWorkerCount; ++iWorkerIndex)
	{
		volatile WorkerData* pWorkerData = &m_arrWorkerData[iWorkerIndex];
		assert(pWorkerData->m_eCommand == WorkerCommand_None);
		pWorkerData->m_fTotalError	= 0.0f;
		pWorkerData->m_eCommand = WorkerCommand_UpdateSpringLength;
	}

	//
	WaitForIdleWorkerThreads();

	float fError = 0.0f;

	for (unsigned int iWorkerIndex = 0; iWorkerIndex < m_iWorkerCount; ++iWorkerIndex)
	{
		fError += m_arrWorkerData[iWorkerIndex].m_fTotalError;
	}


	// ApplySpringForce
	float fPreviousMaxForceAmplitude = m_fPreviousMaxForceAmplitude;		
	if (fPreviousMaxForceAmplitude < 0.000125f)
		 fPreviousMaxForceAmplitude = 0.000125f;

	float fUpdateStep = a_fDt / fPreviousMaxForceAmplitude;
	//_ASSERT(fUpdateStep < 1.0f);

	for (unsigned int iWorkerIndex = 0; iWorkerIndex < m_iWorkerCount; ++iWorkerIndex)
	{
		volatile WorkerData* pWorkerData = &m_arrWorkerData[iWorkerIndex];
		assert(pWorkerData->m_eCommand == WorkerCommand_None);
		
		pWorkerData->m_oJobData.m_oCommandData.asApplySpringForce.m_fUpdateStep	= fUpdateStep;

		pWorkerData->m_fMaxAmplitude = 0.0f;
		pWorkerData->m_eCommand = WorkerCommand_ApplySpringForce;
	}

	WaitForIdleWorkerThreads();


	//
	float fMaxAmplitude = 0.0f;

	for (unsigned int iWorkerIndex = 0; iWorkerIndex < m_iWorkerCount; ++iWorkerIndex)
	{
		float fWorkerMaxAmplitude = m_arrWorkerData[iWorkerIndex].m_fMaxAmplitude;			
		fMaxAmplitude = Util::Max(fMaxAmplitude, fWorkerMaxAmplitude);
	}

	m_fPreviousMaxForceAmplitude = fMaxAmplitude;


	//
	fTotalTime += oTimerGlobal.GetElapsedTime();

	const unsigned int iUpdatePeriod = 1024;
	if ((m_pSimulator->GetIterationCount() % iUpdatePeriod) == 0)
	{
		Util::PrintMessage("Cumulated time for %d iterations:\n", iUpdatePeriod);
		Util::PrintMessage("Total: %f\n", fTotalTime);
		Util::PrintMessage("Error: %f\n", m_pSimulator->GetError());
		fTotalTime = 0.0f;
	}

	return fError;
}

void Multithread::CreateWorkerThreads(Simulator* a_pSimulator, unsigned int a_iWorkerCount)
{
	//
	m_iWorkerCount = a_iWorkerCount;

	if (m_iWorkerCount <= 1)
		return;

	if (m_iWorkerCount > s_iMaxWorker)
		m_iWorkerCount = s_iMaxWorker;

	SYSTEM_INFO oSystemInfo;
	GetSystemInfo(&oSystemInfo);

	if (m_iWorkerCount > oSystemInfo.dwNumberOfProcessors)
		m_iWorkerCount = oSystemInfo.dwNumberOfProcessors;	 


	/////////////
	////
	unsigned int iVertexCountPerWorker = a_pSimulator->GetVertexCount() / m_iWorkerCount;
	unsigned int iSpringCountPerWorker = a_pSimulator->GetSpringCount() / m_iWorkerCount;	

	unsigned int iLastVertexCount		= iVertexCountPerWorker + (a_pSimulator->GetVertexCount() % m_iWorkerCount);
	unsigned int iLastSpringCount		= iSpringCountPerWorker + (a_pSimulator->GetSpringCount() % m_iWorkerCount);


	for (unsigned int iWorkerIndex = 0; iWorkerIndex < (m_iWorkerCount - 1); ++iWorkerIndex)
	{
		WorkerData* pWorkerData = &m_arrWorkerData[iWorkerIndex];

		pWorkerData->m_oJobData.m_oCommandData.asUpdateSpringLength.m_iSpringCount	= iSpringCountPerWorker;
		pWorkerData->m_oJobData.m_oCommandData.asUpdateSpringLength.m_iSpringIndex	= iWorkerIndex * iSpringCountPerWorker;					
		
		pWorkerData->m_oJobData.m_oCommandData.asApplySpringForce.m_fUpdateStep	= 0.0f;
		pWorkerData->m_oJobData.m_oCommandData.asApplySpringForce.m_iVertexCount	= iVertexCountPerWorker;
		pWorkerData->m_oJobData.m_oCommandData.asApplySpringForce.m_iVertexIndex	= iWorkerIndex * iVertexCountPerWorker;			

		pWorkerData->m_fTotalError		= 0.0f;
		pWorkerData->m_fMaxAmplitude	= 0.0f;
		pWorkerData->m_eCommand			= WorkerCommand_None;
		pWorkerData->m_pThis			= this;
		pWorkerData->m_iIndex			= iWorkerIndex;
	}

	WorkerData* pLastWorkerData = &m_arrWorkerData[m_iWorkerCount - 1];
	pLastWorkerData->m_oJobData.m_oCommandData.asUpdateSpringLength.m_iSpringCount		= iLastSpringCount;
	pLastWorkerData->m_oJobData.m_oCommandData.asUpdateSpringLength.m_iSpringIndex		= (m_iWorkerCount - 1) * iSpringCountPerWorker;

	pLastWorkerData->m_oJobData.m_oCommandData.asApplySpringForce.m_fUpdateStep		= 0.0f;
	pLastWorkerData->m_oJobData.m_oCommandData.asApplySpringForce.m_iVertexCount		= iLastVertexCount;
	pLastWorkerData->m_oJobData.m_oCommandData.asApplySpringForce.m_iVertexIndex		= (m_iWorkerCount - 1) * iVertexCountPerWorker;

	pLastWorkerData->m_fTotalError		= 0.0f;
	pLastWorkerData->m_fMaxAmplitude	= 0.0f;
	pLastWorkerData->m_eCommand			= WorkerCommand_None;
	pLastWorkerData->m_pThis			= this;
	pLastWorkerData->m_iIndex			= m_iWorkerCount - 1;


	//
	char szBuffer[256];

	HANDLE hThread		= INVALID_HANDLE_VALUE;
	HANDLE hEvent		= INVALID_HANDLE_VALUE;
	for (unsigned int iWorkerIndex = 0; iWorkerIndex < m_iWorkerCount; ++iWorkerIndex)
	{
		//
		hEvent = CreateEvent( NULL, FALSE, FALSE, NULL);
		assert(hEvent != INVALID_HANDLE_VALUE);
		m_arrThreadEvent[iWorkerIndex] = hEvent;

		//
		hEvent = CreateEvent( NULL, FALSE, FALSE, NULL);
		assert(hEvent != INVALID_HANDLE_VALUE);
		m_arrWorkerData[iWorkerIndex].m_hEvent = hEvent;

		//
		hThread = CreateThread( NULL, 16 * 1024, ThreadMain, &m_arrWorkerData[iWorkerIndex], 0, NULL);
		assert(hThread != INVALID_HANDLE_VALUE);
		
		sprintf_s(szBuffer, "Simulator Worker %d", iWorkerIndex);
		SetThreadName(GetThreadId(hThread), szBuffer);

		if (SetThreadIdealProcessor(hThread, iWorkerIndex) == (DWORD)-1)
		{
			Util::PrintMessage("SetThreadIdealProcessor failed: %d", ::GetLastError());
		}

		m_arrThread[iWorkerIndex] = hThread;


	}

/*

	//
	const unsigned int	iElementPerJob	= 1024;
	unsigned int		iSpringJobCount	= (m_iSpringCount / iElementPerJob) + 1;
	unsigned int		iVertexJobCount	= (m_iVertexCount / iElementPerJob) + 1;
	unsigned int		iSyncJobCount	= 2;

	unsigned int		iJobCount = iSpringJobCount + iVertexJobCount + iSyncJobCount;

	m_arrJob = (WorkerJobData*)calloc(iJobCount, sizeof(WorkerJobData));

	int iJobIndex = 0;
	int iSpringIndex = 0;
	for (unsigned int iSpringJobIndex = 0; iSpringJobIndex < iSpringJobCount; ++SpringJobCount)
	{
		unsigned int iJobSpringCount = iElementPerJob;
		if ((iSpringIndex + iJobSpringCount) > m_iSpringCount)
			iJobSpringCount = m_iSpringCount - iSpringIndex;

		m_arrJob[iJobIndex].m_eCommand = WorkerCommand_UpdateSpringLength;
		m_arrJob[iJobIndex].m_oCommandData.asUpdateSpringLength.m_iSpringIndex = iSpringIndex;
		m_arrJob[iJobIndex].m_oCommandData.asUpdateSpringLength.m_iSpringCount = iJobSpringCount;

		iJobIndex++;

		iSpringIndex += iJobSpringCount;
	}

	assert(iSpringIndex == m_iSpringCount);

	m_arrJob[iJobIndex].m_eCommand = WorkerCommand_Sync;
	m_arrJob[iJobIndex].m_oCommandData.asSync = WorkerCommand_Sync;
*/

		




	//
	
}

void Multithread::WaitForIdleWorkerThreads()
{
	DWORD dwRes;
	BOOL bRes;

	for (unsigned int iWorkerIndex = 0; iWorkerIndex < m_iWorkerCount; ++iWorkerIndex)
	{
		bRes = ::SetEvent(m_arrWorkerData[iWorkerIndex].m_hEvent);
		assert(bRes != FALSE);
	}

	dwRes = ::WaitForMultipleObjects(m_iWorkerCount, m_arrThreadEvent, TRUE, INFINITE);
	assert(dwRes == WAIT_OBJECT_0);

	for (unsigned int iWorkerIndex = 0; iWorkerIndex < m_iWorkerCount; ++iWorkerIndex)
	{
		volatile WorkerData* pWorkerData = &m_arrWorkerData[iWorkerIndex];
		assert(pWorkerData->m_eCommand == WorkerCommand_None);
	}
}

float Multithread::UpdateSpringLength(unsigned int a_iStartIndex, unsigned int a_iCount)
{
	return m_pSimulator->UpdateSpringLength(a_iStartIndex, a_iCount);
}

float Multithread::ApplySpringForce(unsigned int a_iStartIndex, unsigned int a_iCount, float a_fUpdateStep)
{
	return m_pSimulator->ApplySpringForce(a_iStartIndex, a_iCount, a_fUpdateStep);
}


} // namespace ForceField2D