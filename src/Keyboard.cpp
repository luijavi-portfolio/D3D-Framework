#include "Keyboard.h"

bool Keyboard::KeyIsPressed(unsigned char keycode) const
{
	return key_states_[keycode];
}

std::optional<Keyboard::Event> Keyboard::ReadKey()
{
	if (!key_buffer_.empty())
	{
		Event e = key_buffer_.front();
		key_buffer_.pop();
		return e;
	}
	else
	{
		return {};
	}
}

bool Keyboard::KeyIsEmpty() const
{
	return key_buffer_.empty();
}

void Keyboard::ClearKey()
{
	key_buffer_ = std::queue<Event>();
}

char Keyboard::ReadChar()
{
	if (!char_buffer_.empty())
	{
		unsigned char char_code = char_buffer_.front();
		char_buffer_.pop();
		return char_code;
	}
	else
	{
		return 0;
	}
}

bool Keyboard::CharIsEmpty() const
{
	return char_buffer_.empty();
}

void Keyboard::ClearChar()
{
	char_buffer_ = std::queue<char>();
}

void Keyboard::Clear()
{
	ClearKey();
	ClearChar();
}

void Keyboard::EnableAutorepeat()
{
	autorepeat_enabled_ = true;
}

void Keyboard::DisableAutorepeat()
{
	autorepeat_enabled_ = false;
}

bool Keyboard::AutorepeatIsEnabled() const
{
	return autorepeat_enabled_;
}

void Keyboard::OnKeyPressed(unsigned char keycode)
{
	key_states_[keycode] = true;
	key_buffer_.push(Event(Event::Type::kPress, keycode));
	TrimBuffer(key_buffer_);
}

void Keyboard::OnKeyReleased(unsigned char keycode)
{
	key_states_[keycode] = false;
	key_buffer_.push(Event(Event::Type::kRelease, keycode));
	TrimBuffer(key_buffer_);
}

void Keyboard::OnChar(char character)
{
	char_buffer_.push(character);
	TrimBuffer(char_buffer_);
}

void Keyboard::ClearState()
{
	key_states_.reset();
}

template<typename T>
inline void Keyboard::TrimBuffer(std::queue<T>& buffer)
{
	while (buffer.size() > kBufferSize_)
	{
		buffer.pop();
	}
}