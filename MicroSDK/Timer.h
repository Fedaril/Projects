

#pragma once

namespace Util
{

class Timer
{
public:
	DOUBLE m_fLastElapsedAbsoluteTime;
	DOUBLE m_fBaseAbsoluteTime;

	DOUBLE m_fLastElapsedTime;
	DOUBLE m_fBaseTime;
	DOUBLE m_fStopTime;
	BOOL m_bTimerStopped;

	DOUBLE  m_fFPS;
	DWORD	m_dwNumFrames;
	DOUBLE	m_fLastFPSTime;

	LARGE_INTEGER m_PerfFreq;

	Timer()
	{
		QueryPerformanceFrequency( &m_PerfFreq );
		DOUBLE fTime = GetAbsoluteTime();

		m_fBaseAbsoluteTime = fTime;
		m_fLastElapsedAbsoluteTime = fTime;

		m_fBaseTime = fTime;
		m_fStopTime = 0.0;
		m_fLastElapsedTime = fTime;
		m_bTimerStopped = FALSE;

		m_fFPS				= 0.0f;
		m_dwNumFrames = 0;
		m_fLastFPSTime = fTime;
	}

	DOUBLE  GetAbsoluteTime()
	{
		LARGE_INTEGER Time;
		QueryPerformanceCounter( &Time );
		DOUBLE fTime = ( DOUBLE )Time.QuadPart / ( DOUBLE )m_PerfFreq.QuadPart;
		return fTime;
	}

	DOUBLE  GetTime()
	{
		// Get either the current time or the stop time, depending
		// on whether we're stopped and what command was sent
		return ( m_fStopTime != 0.0 ) ? m_fStopTime : GetAbsoluteTime();
	}

	DOUBLE  GetElapsedTime()
	{
		DOUBLE fTime = GetAbsoluteTime();

		DOUBLE fElapsedAbsoluteTime = ( DOUBLE )( fTime - m_fLastElapsedAbsoluteTime );
		m_fLastElapsedAbsoluteTime = fTime;
		return fElapsedAbsoluteTime;
	}

	// Return the current time
	DOUBLE  GetAppTime()
	{
		return GetTime() - m_fBaseTime;
	}

	// Reset the timer
	DOUBLE  Reset()
	{
		DOUBLE fTime = GetTime();

		m_fBaseTime = fTime;
		m_fLastElapsedTime = fTime;
		m_fStopTime = 0;
		m_bTimerStopped = FALSE;
		return 0.0;
	}

	// Start the timer
	VOID    Start()
	{
		DOUBLE fTime = GetAbsoluteTime();

		if( m_bTimerStopped )
			m_fBaseTime += fTime - m_fStopTime;
		m_fStopTime = 0.0;
		m_fLastElapsedTime = fTime;
		m_bTimerStopped = FALSE;
	}

	// Stop the timer
	VOID    Stop()
	{
		DOUBLE fTime = GetTime();

		if( !m_bTimerStopped )
		{
			m_fStopTime = fTime;
			m_fLastElapsedTime = fTime;
			m_bTimerStopped = TRUE;
		}
	}

	// Advance the timer by 1/10th second
	VOID    SingleStep( DOUBLE fTimeAdvance )
	{
		m_fStopTime += fTimeAdvance;
	}

	VOID    MarkFrame()
	{
		m_dwNumFrames++;
	}

	DOUBLE GetFrameRate()
	{
		DOUBLE fTime = GetAbsoluteTime();

		// Only re-compute the FPS (frames per second) once per second
		if( fTime - m_fLastFPSTime > 1.0 )
		{
			DOUBLE fFPS = m_dwNumFrames / ( fTime - m_fLastFPSTime );
			m_fLastFPSTime = fTime;
			m_dwNumFrames = 0L;
			m_fFPS = fFPS;
		}

		return m_fFPS;
	}
};

} // namespace Util