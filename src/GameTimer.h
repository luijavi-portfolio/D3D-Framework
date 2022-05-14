#ifndef GAME_TIMER_H
#define GAME_TIMER_H

class GameTimer
{
public:
	GameTimer();

	float GameTime() const;		// In seconds
	float DeltaTime() const;	// In seconds

	void Reset();	// Call before message loop
	void Start();	// Call when unpaused
	void Stop();	// Call when paused
	void Tick();	// Call every frame

private:
	double seconds_per_count_;
	double delta_time_;

	__int64 base_time_;
	__int64 paused_time_;
	__int64 stop_time_;
	__int64 prev_time_;
	__int64 curr_time_;

	bool is_stopped_;
};


#endif // !GAME_TIMER_H
