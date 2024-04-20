#include "stdafx.h"
#include "Timer.h"

CTimer::CTimer()
{
	::QueryPerformanceFrequency((LARGE_INTEGER*)&m_nPerformanceFrequencyPerSec);
	::QueryPerformanceCounter((LARGE_INTEGER*)&m_nLastPerformanceCounter);
	m_fTimeScale = 1.0 / (double)m_nPerformanceFrequencyPerSec;

	m_nBasePerformanceCounter = m_nLastPerformanceCounter;
	m_nPausedPerformanceCounter = 0;
	m_nStopPerformanceCounter = 0;

	m_nSampleCount = 0;
	m_nCurrentFrameRate = 0;
	m_nFramesPerSecond = 0;
	m_fFPSTimeElapsed = 0.0f;
}

CTimer::~CTimer()
{
}

void CTimer::Tick(float fLockFPS)
{
	if (m_bStopped)
	{
		m_fElapsedTime = 0.0f;
		return;
	}
	float fElapsedTime;

	::QueryPerformanceCounter((LARGE_INTEGER*)&m_nCurrentPerformanceCounter);
	fElapsedTime = float((m_nCurrentPerformanceCounter - m_nLastPerformanceCounter) * m_fTimeScale);

	if (fLockFPS > 0.0f)
	{
		while (fElapsedTime < (1.0f / fLockFPS))
		{
			::QueryPerformanceCounter((LARGE_INTEGER*)&m_nCurrentPerformanceCounter);
			fElapsedTime = float((m_nCurrentPerformanceCounter - m_nLastPerformanceCounter) * m_fTimeScale);
		}
	}

	m_nLastPerformanceCounter = m_nCurrentPerformanceCounter;

	if (fabsf(fElapsedTime - m_fElapsedTime) < 1.0f)
	{
		::memmove(&m_fFrameTime[1], m_fFrameTime, (MAX_SAMPLE_COUNT - 1) * sizeof(float));
		m_fFrameTime[0] = fElapsedTime;
		if (m_nSampleCount < MAX_SAMPLE_COUNT) m_nSampleCount++;
	}

	m_nFramesPerSecond++;
	m_fFPSTimeElapsed += fElapsedTime;
	if (m_fFPSTimeElapsed > 1.0f)
	{
		m_nCurrentFrameRate = m_nFramesPerSecond;
		m_nFramesPerSecond = 0;
		m_fFPSTimeElapsed = 0.0f;
	}

	m_fElapsedTime = 0.0f;
	for (ULONG i = 0; i < m_nSampleCount; i++) m_fElapsedTime += m_fFrameTime[i];
	if (m_nSampleCount > 0) m_fElapsedTime /= m_nSampleCount;
}

void CTimer::Start()
{
	__int64 nPerformanceCounter;
	::QueryPerformanceCounter((LARGE_INTEGER*)&nPerformanceCounter);
	if (m_bStopped)
	{
		m_nPausedPerformanceCounter += (nPerformanceCounter - m_nStopPerformanceCounter);
		m_nLastPerformanceCounter = nPerformanceCounter;
		m_nStopPerformanceCounter = 0;
		m_bStopped = false;
	}
}

void CTimer::Stop()
{
	if (!m_bStopped)
	{
		::QueryPerformanceCounter((LARGE_INTEGER*)&m_nStopPerformanceCounter);
		m_bStopped = true;
	}
}

void CTimer::Reset()
{
	__int64 nPerformanceCounter;
	::QueryPerformanceCounter((LARGE_INTEGER*)&nPerformanceCounter);

	m_nBasePerformanceCounter = nPerformanceCounter;
	m_nLastPerformanceCounter = nPerformanceCounter;
	m_nStopPerformanceCounter = 0;
	m_bStopped = false;
}

unsigned long CTimer::GetFrameRate(LPTSTR lpszString, int nCharacters)
{
	if (lpszString)
	{
		_itow_s(m_nCurrentFrameRate, lpszString, nCharacters, 10);
		wcscat_s(lpszString, nCharacters, _T(" FPS)"));
	}

	return(m_nCurrentFrameRate);
}

float CTimer::GetTimeElapsed()
{
	return(m_fElapsedTime);
}

float CTimer::GetTotalTime()
{
	if (m_bStopped) return(float(((m_nStopPerformanceCounter - m_nPausedPerformanceCounter) - m_nBasePerformanceCounter) * m_fTimeScale));
	return(float(((m_nCurrentPerformanceCounter - m_nPausedPerformanceCounter) - m_nBasePerformanceCounter) * m_fTimeScale));
}