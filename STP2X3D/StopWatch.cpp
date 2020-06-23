#include "stdafx.h"
#include "StopWatch.h"


StopWatch::StopWatch(void)
	: m_startTime((clock_t)0.0),
	m_endTime((clock_t)0.0)
{
}

StopWatch::~StopWatch(void)
{
}

void StopWatch::Start(void)
{
	m_netTimes.clear();
	m_startTime = clock();
}

void StopWatch::Lap(void)
{
	SaveNetTime();
	ReportTime(0);

	m_startTime = clock();
}

void StopWatch::End(void)
{
	SaveNetTime();
	ReportTime(1);
}

void StopWatch::SaveNetTime(void)
{
	m_endTime = clock();

	clock_t netTime = m_endTime - m_startTime;
	m_netTimes.push_back(netTime);
}

void StopWatch::ReportTime(int mode) const
{
	clock_t time;
	
	if (mode == 0)
	{
		time = m_netTimes[m_netTimes.size() - 1];
		cout << "Time elapsed: ";
	}
	else
	{
		time = accumulate(m_netTimes.begin(), m_netTimes.end(), 0);
		cout << "Total time elapsed: ";
	}

	double timeInSec_dbl = time / (double)CLOCKS_PER_SEC;

	int timeInSec_int = (int)timeInSec_dbl;

	if (timeInSec_int < 60)
	{
		cout << timeInSec_dbl << " seconds" << endl << endl;
	}
	else if (timeInSec_int < 3600)
	{
		int minutes = timeInSec_int / 60;
		int seconds = timeInSec_int % 60;

		cout << minutes << " minutes ";

		if (seconds > 0)
			cout << seconds << " seconds";

		cout << endl << endl;
	}
	else
	{
		int hours = timeInSec_int / 3600;
		int minutes = (timeInSec_int % 3600) / 60;

		cout << hours << " hours ";

		if (minutes > 0)
			cout << minutes << " minutes";

		cout << endl << endl;
	}
}