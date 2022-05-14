#include "GameTimer.h"

#include "LeanWin32.h"

GameTimer::GameTimer()
	:
	seconds_per_count_(0.0),
	delta_time_(-1.0),
	base_time_(0.0),
	paused_time_(0),
	prev_time_(0),
	curr_time_(0),
	is_stopped_(false)
{
	__int64 counts_per_second;
	QueryPerformanceFrequency((LARGE_INTEGER*)&counts_per_second);
	seconds_per_count_ = 1.0 / static_cast<double>(counts_per_second);
}

float GameTimer::GameTime() const
{
	// If we are stopped, do not count the time that has passed
	// since we stopped. Moreover, if we previously already had 
	// a pause, the distance stop_time_ - base_time_ includes
	// paused time, which we do not want to count. To correct this, we can
	// subtract the paused time from stop_time_:

	if (is_stopped_)
	{
		return static_cast<float>(((stop_time_ - paused_time_) - base_time_) * seconds_per_count_);
	}
	// The distance curr_time_ - base_time_ includes paused time, 
	// which we do not want to count. To correct this, we can subtract
	// the paused time  from cur_time_:
	else
	{
		return static_cast<float>(((curr_time_ - paused_time_) - base_time_) * seconds_per_count_);
	}
}

float GameTimer::DeltaTime() const
{
	return static_cast<float>(delta_time_);
}

void GameTimer::Reset()
{
	__int64 curr_time;
	QueryPerformanceCounter((LARGE_INTEGER*)&curr_time);

	base_time_ = curr_time;
	prev_time_ = curr_time;	// Inits prev_time_ to the current time when Reset is called
	stop_time_ = 0;
	is_stopped_ = false;
}

void GameTimer::Start()
{
	__int64 start_time;
	QueryPerformanceCounter((LARGE_INTEGER*)&start_time);

	// Accumulate the time elapsed between stop and start pairs.

	// If we are resuming the timer from a stopped state...
	if (is_stopped_)
	{
		// then accumulate the paused time
		paused_time_ += (start_time - stop_time_);

		// since we are starting the timer back up, the current
		// previous time is not valid, as it occurred while paused.
		// So reset it to the current time.
		prev_time_ = start_time;

		// No longer stopped...
		stop_time_ = 0;
		is_stopped_ = false;
	}
}

void GameTimer::Stop()
{
	// If we are already stopped, then don't do anything.
	if (!is_stopped_)
	{
		__int64 curr_time;
		QueryPerformanceCounter((LARGE_INTEGER*)&curr_time);

		// Otherwise, save the time we stopped at, and set
		// the bool flag indicating the timer is stopped.
		stop_time_ = curr_time;
		is_stopped_ = true;
	}
}

void GameTimer::Tick()
{
	if (!is_stopped_)
	{
		// Get the time this frame
		__int64 curr_time;
		QueryPerformanceCounter((LARGE_INTEGER*)&curr_time);

		// Time difference between this frame and the previous
		delta_time_ = (curr_time_ - prev_time_) * seconds_per_count_;

		// Prepare for next frame
		prev_time_ = curr_time_;

		// Force nonnegative. The DXSDK's CDXUTTimer mentions that if the
		// processor goes into a power save mode or we get shuffled to
		// another processor, then delta_time_ can be negative.
		if (delta_time_ < 0.0)
		{
			delta_time_ = 0.0;
		}
	}
	else
	{
		delta_time_ = 0.0;
	}
}
