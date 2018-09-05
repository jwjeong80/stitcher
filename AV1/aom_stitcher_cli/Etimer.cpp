/*****************************************************************************
* Copyright (C) 2017 KetiBitstreamStitcher (Project: Tiled VR streaming)
*
* File: Etimer.cpp
*
* Authors: Yong-Hwan Kim <yonghwan@keti.re.kr>
*
* Original: "Etimer.cpp" created by yonghwan@keti.re.kr, 2014.08.06
*
* The property of program is under Korea Electronics Technology Institute.
* For more information, contact us at <sungjei.kim@keti.re.kr>.
*****************************************************************************/

#include "stdafx.h"

#include <stdio.h>
#include "Etimer.h"
#include <windows.h>
//#include <math.h>
//#include <conio.h>


CETimer::CETimer()
:	m_llStart(0)
,	m_llStop(0)
,	m_nErrorFlag(0)
{
	m_llFrequency	= CalcFrequency();
}

CETimer::~CETimer()
{

}

LONGLONG CETimer::CalcFrequency()
{
	bool			bStatus		= true;
	LARGE_INTEGER	Freq;

	QueryPerformanceFrequency(&Freq);	// Get the frequency of the timer (시스템 부팅시 정해지는 값이고, 프로세서/쓰레드와 상관 없음)
											// On systems that run Windows XP or later, the function will always succeed and will thus never return zero.
	return Freq.QuadPart;
}

bool CETimer::Start(bool bSingleProcessor /*= false*/)
{
	bool			bStatus		= true;
	LARGE_INTEGER	Counter; 
	DWORD_PTR		dwProcessAffinity;

	if(bSingleProcessor)
		dwProcessAffinity = SetSingleProcessor();

	QueryPerformanceCounter(&Counter);	//On systems that run Windows XP or later, the function will always succeed and will thus never return zero.
	m_llStart		= Counter.QuadPart;

	if(bSingleProcessor)
		ResetSingleProcessor(dwProcessAffinity);

	return true;
}

LONGLONG CETimer::Stop(bool bSingleProcessor /*= false*/)
{
	bool			bStatus		= true;
	LARGE_INTEGER	Counter; 
	DWORD_PTR		dwProcessAffinity;

	if(bSingleProcessor)
		dwProcessAffinity = SetSingleProcessor();

	QueryPerformanceCounter(&Counter);	//On systems that run Windows XP or later, the function will always succeed and will thus never return zero.
	m_llStop		= Counter.QuadPart;

	if(bSingleProcessor)
		ResetSingleProcessor(dwProcessAffinity);

	return DurationInTicks();
}


LONGLONG CETimer::DurationInTicks()		// ticks-per-second
{
	return (m_llStop - m_llStart);
}

LONGLONG CETimer::DurationInSeconds()
{
	// In second
	return (m_llStop - m_llStart) / m_llFrequency;
}

LONGLONG CETimer::DurationInMilliSeconds()
{
	// In milli-second
	return (m_llStop - m_llStart) * 1000 / m_llFrequency;
}


LONGLONG CETimer::calcDurationInSeconds(LONGLONG llTicks)
{
	// In second
	return llTicks / m_llFrequency;
}

LONGLONG CETimer::calcDurationInMilliSeconds(LONGLONG llTicks)
{
	// In milli-second
	return llTicks * 1000 / m_llFrequency;
}

ULONG_PTR CETimer::SetSingleCore()  
{  
	ULONG_PTR pam, sam, oldAM;  
	if(!GetProcessAffinityMask(GetCurrentProcess(), &pam, &sam))  
		return 0;  
  
	ULONG_PTR am = 1;  
	int bits = CHAR_BIT * sizeof(am);  
  
	for(int i=0; i<bits; ++i)  
	{  
		if(am & pam)  
		{  
			oldAM = SetThreadAffinityMask(GetCurrentThread(), am);  
			break;  
		}  
  
		am <<= 1;  
	}

	return oldAM;
}  

void CETimer::ResetSingleCore(ULONG_PTR oldAM)
{
	SetThreadAffinityMask(GetCurrentThread(), oldAM); 
}

DWORD_PTR CETimer::SetSingleProcessor()
{
 	HANDLE CurrentProcessHandle;
	DWORD_PTR  ProcessAffinity;
	DWORD_PTR  SystemAffinity;
	DWORD_PTR  AllowProcessAffinity;
	DWORD_PTR  AffinityMask;

	CurrentProcessHandle = GetCurrentProcess();
	GetProcessAffinityMask(CurrentProcessHandle, &ProcessAffinity, &SystemAffinity);

	// Bit vector representing Processors that the thread can run on
	AllowProcessAffinity = ProcessAffinity & SystemAffinity;

	// Make sure at least one processor is available for this thread
	if (AllowProcessAffinity)  
	{
		AffinityMask = 1;

		// Select the lowest processor ID to run
		while (AffinityMask <= AllowProcessAffinity)
		{
			// Check if this processor is available
			if (AffinityMask & AllowProcessAffinity)
			{
				SetProcessAffinityMask(CurrentProcessHandle, AffinityMask);
				break;
			}

			AffinityMask = AffinityMask << 1;  // Check the next processor
		}
	}
	return ProcessAffinity;
}

void CETimer::ResetSingleProcessor(DWORD_PTR dwProcessorMask)
{
	// Reset the processor affinity
	SetProcessAffinityMask(GetCurrentProcess(), dwProcessorMask);

}
