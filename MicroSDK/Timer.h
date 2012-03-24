

#pragma once

namespace MicroSDK
{

class Timer
{
private:
	double			m_fLastElapsedAbsoluteTime;
	double			m_fBaseAbsoluteTime;

	double			m_fLastElapsedTime;
	double			m_fBaseTime;
	double			m_fStopTime;
	bool			m_bTimerStopped;

	double			m_fFPS;
	unsigned int	m_dwNumFrames;
	double			m_fLastFPSTime;

	long long		m_PerfFreq;

public:			
						Timer();


	double				GetAbsoluteTime();

	double				GetTime();

	double				GetElapsedTime();

	double				GetAppTime();

	double				Reset();

	void				Start();
	void				Stop();

	void				SingleStep( double fTimeAdvance );
	void				MarkFrame();
	double				GetFrameRate();
};

} // namespace MicroSDK