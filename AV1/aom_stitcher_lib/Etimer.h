/*****************************************************************************
* Copyright (C) 2017 KetiBitstreamStitcher (Project: Tiled VR streaming)
*
* File: Etimer.h
*
* Authors: Yong-Hwan Kim <yonghwan@keti.re.kr>
*
* Original: "Etimer.h" created by yonghwan@keti.re.kr, 2014.08.06
*
* The property of program is under Korea Electronics Technology Institute.
* For more information, contact us at <sungjei.kim@keti.re.kr>.
*****************************************************************************/


#pragma once

#include <windows.h>

#define CANNOT_GET_FREQUENCY						1
#define CANNOT_GET_START_TIME						2
#define CANNOT_GET_STOP_TIME						3
#define NO_PROCESSOR_AVAILABLE_TO_GET_START_TIME	4
#define NO_PROCESSOR_AVAILABLE_TO_GET_STOP_TIME		5
#define STOP_TIME_TAKEN_ON_DIFFERENT_PROCESSOR		6

class	CETimer
{
public:
	CETimer();
	~CETimer();

	bool		Start(bool bSingleProcessor = false);
	LONGLONG	Stop(bool bSingleProcessor = false);

	LONGLONG	CalcFrequency();
	LONGLONG	DurationInTicks();
	LONGLONG	DurationInSeconds();
	LONGLONG	DurationInMilliSeconds();
	LONGLONG	getFrequency()	{	return	m_llFrequency;	}

	LONGLONG	calcDurationInSeconds(LONGLONG llTicks);
	LONGLONG	calcDurationInMilliSeconds(LONGLONG llTicks);

	DWORD_PTR	SetSingleProcessor();
	void		ResetSingleProcessor(DWORD_PTR dwProcessorMask);

private:

	ULONG_PTR	SetSingleCore();
	void		ResetSingleCore(ULONG_PTR oldAM);



	LONGLONG	m_llStart;
	LONGLONG	m_llStop;
	LONGLONG	m_llFrequency;

	int			m_nErrorFlag;
};


