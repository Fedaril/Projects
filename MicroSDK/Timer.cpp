
#include "StdAfx.h"
#include "Timer.h"

namespace MicroSDK
{

Timer::Timer()
{
	QueryPerformanceFrequency( (LARGE_INTEGER*)&m_PerfFreq );
	double fTime = GetAbsoluteTime();

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

double Timer::GetAbsoluteTime()
{
	LARGE_INTEGER Time;
	QueryPerformanceCounter( &Time );
	double fTime = ( double )Time.QuadPart / ( double )m_PerfFreq;
	return fTime;
}

double Timer::GetTime()
{
	// Get either the current time or the stop time, depending
	// on whether we're stopped and what command was sent
	return ( m_fStopTime != 0.0 ) ? m_fStopTime : GetAbsoluteTime();
}

double Timer::GetElapsedTime()
{
	double fTime = GetAbsoluteTime();

	double fElapsedAbsoluteTime = ( double )( fTime - m_fLastElapsedAbsoluteTime );
	m_fLastElapsedAbsoluteTime = fTime;
	return fElapsedAbsoluteTime;
}

// Return the current time
double Timer::GetAppTime()
{
	return GetTime() - m_fBaseTime;
}

// Reset the timer
double Timer::Reset()
{
	double fTime = GetTime();

	m_fBaseTime = fTime;
	m_fLastElapsedTime = fTime;
	m_fStopTime = 0;
	m_bTimerStopped = false;
	return 0.0;
}

// Start the timer
void Timer::Start()
{
	double fTime = GetAbsoluteTime();

	if( m_bTimerStopped )
		m_fBaseTime += fTime - m_fStopTime;
	m_fStopTime = 0.0;
	m_fLastElapsedTime = fTime;
	m_bTimerStopped = false;
}

// Stop the timer
void Timer::Stop()
{
	double fTime = GetTime();

	if( !m_bTimerStopped )
	{
		m_fStopTime = fTime;
		m_fLastElapsedTime = fTime;
		m_bTimerStopped = true;
	}
}

// Advance the timer by 1/10th second
VOID Timer::SingleStep( double fTimeAdvance )
{
	m_fStopTime += fTimeAdvance;
}

void Timer::MarkFrame()
{
	m_dwNumFrames++;
}

double Timer::GetFrameRate()
{
	double fTime = GetAbsoluteTime();

	// Only re-compute the FPS (frames per second) once per second
	if( fTime - m_fLastFPSTime > 1.0 )
	{
		double fFPS = m_dwNumFrames / ( fTime - m_fLastFPSTime );
		m_fLastFPSTime = fTime;
		m_dwNumFrames = 0L;
		m_fFPS = fFPS;
	}

	return m_fFPS;
}

}