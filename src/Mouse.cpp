#include "Mouse.h"

std::pair<int, int> Mouse::GetPos() const
{
	return { x_, y_ };
}

int Mouse::GetPosX() const
{
	return x_;
}

int Mouse::GetPosY() const
{
	return y_;
}

bool Mouse::LeftIsPressed() const
{
	return left_is_pressed_;
}

bool Mouse::RightIsPressed() const
{
	return right_is_pressed_;
}

bool Mouse::IsInWindow() const
{
	return is_in_window_;
}

std::optional<Mouse::Event> Mouse::Read()
{
	if (buffer_.size() > 0u)
	{
		Event e = buffer_.front();
		buffer_.pop();
		return e;
	}
	else
	{
		return std::optional<Event>();
	}
}

bool Mouse::IsEmpty() const
{
	return buffer_.empty();
}

void Mouse::Clear()
{
	buffer_ = std::queue<Event>();
}

void Mouse::OnMouseMove(int x, int y)
{
	x_ = x;
	y_ = y;

	buffer_.push(Event(Event::Type::kMove, *this));
	TrimBuffer();
}

void Mouse::OnMouseLeave()
{
	is_in_window_ = false;
}

void Mouse::OnMouseEnter()
{
	is_in_window_ = true;
}

void Mouse::OnLeftIsPressed(int x, int y)
{
	left_is_pressed_ = true;
	x_ = x;
	y_ = y;

	buffer_.push(Event(Event::Type::kLPress, *this));
	TrimBuffer();
}

void Mouse::OnLeftIsReleased(int x, int y)
{
	left_is_pressed_ = false;

	buffer_.push(Event(Event::Type::kLRelease, *this));
	TrimBuffer();
}

void Mouse::OnRightIsPressed(int x, int y)
{
	right_is_pressed_ = true;
	x_ = x;
	y_ = y;

	buffer_.push(Event(Event::Type::kRPress, *this));
	TrimBuffer();
}

void Mouse::OnRightIsReleased(int x, int y)
{
	right_is_pressed_ = false;

	buffer_.push(Event(Event::Type::kRRelease, *this));
	TrimBuffer();
}

void Mouse::OnWheelUp(int x, int y)
{
	buffer_.push(Event(Event::Type::kWheelUp, *this));
	TrimBuffer();
}

void Mouse::OnWheelDown(int x, int y)
{
	buffer_.push(Event(Event::Type::kWheelDown, *this));
}

void Mouse::TrimBuffer()
{
	while (buffer_.size() > kBufferSize_)
	{
		buffer_.pop();
	}
}
