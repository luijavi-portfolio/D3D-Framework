#ifndef GAME_TIMER_H
#define GAME_TIMER_H

class GameTimer
{
public:
	GameTimer();

	float GameTime() const;
	float DeltaTime() const;

	void Reset();
	void Start();
	void Stop();
	void Tick();

private:
	double seconds_per_count_;
	double delta_time_;

	__int64 base_time_;
	__int64 paused_time_;
	__int64 stop_time_;
	__int64 prev_time_;
	__int64 curr_time;

	bool stopped_;
};


#endif // !GAME_TIMER_H
