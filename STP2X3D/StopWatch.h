#pragma once

class StopWatch
{
public:
	StopWatch(void);
	~StopWatch(void);

	void Start(void);
	void Lap(void);
	void End(void);

protected:
	void SaveNetTime(void);
	void ReportTime(int mode) const;

private:
	clock_t m_startTime;
	clock_t m_endTime;
	vector<clock_t> m_netTimes;
};