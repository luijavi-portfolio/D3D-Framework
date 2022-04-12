#ifndef KEYBOARD_H
#define KEYBOARD_H

#include <queue>
#include <bitset>
#include <optional>

// The keyboard class has an interface facing Win32 API and to the public
class Keyboard
{
	friend class Window;
public:
	// Internal Keybaord class for state and...
	class Event
	{
	public:
		enum class Type
		{
			kPress,
			kRelease,
			kInvalid
		};
	private:
		Type type_;
		unsigned char code_;
	public:
		/*Event()
			:
			type_(Type::kInvalid),
			code_(0u)
		{}*/
		Event(Type type, unsigned char code)
			:
			type_(type),
			code_(code)
		{}
		bool IsPressed() const
		{
			return type_ == Type::kPress;
		}
		bool IsRelease() const
		{
			return type_ == Type::kRelease;
		}
		bool IsValid() const
		{
			return type_ != Type::kInvalid;
		}
		unsigned char GetCode() const
		{
			return code_;
		}
	};
public:
	Keyboard() = default;
	// don't need copy constructor/assignment
	Keyboard(const Keyboard&) = delete;
	Keyboard& operator=(const Keyboard&) = delete;
	
	// Key events
	bool KeyIsPressed(unsigned char keycode) const;
	std::optional<Event> ReadKey();
	bool KeyIsEmpty() const;
	void ClearKey();

	// Char events
	char ReadChar();
	bool CharIsEmpty() const;
	void ClearChar();
	void Clear();

	// Autorepeat control
	void EnableAutorepeat();
	void DisableAutorepeat();
	bool AutorepeatIsEnabled() const;

private:
	// Interface for the Win32 API side
	void OnKeyPressed(unsigned char keycode);
	void OnKeyReleased(unsigned char keycode);
	void OnChar(char character);
	void ClearState();

	// If the buffer is over the max amount, TrimBuffer will
	// remove items from the queue until size is at max
	template<typename T>
	static void TrimBuffer(std::queue<T>& buffer);

private:
	static constexpr unsigned int kNumKeys_ = 256u;
	static constexpr unsigned int kBufferSize_ = 16u;
	bool autorepeat_enabled_ = false;
	std::bitset<kNumKeys_> key_states_;
	std::queue<Event> key_buffer_;
	std::queue<char> char_buffer_;
};
#endif // !KEYBOARD_H
