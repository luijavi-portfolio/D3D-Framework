#ifndef MOUSE_H
#define MOUSE_H

#include <queue>
#include <optional>

class Mouse
{
	friend class Window;
public:
	class Event
	{
		public:
			enum class Type
			{
				kLPress,
				kLRelease,
				kRPress,
				kRRelease,
				kWheelUp,
				kWheelDown,
				kMove,
				kEnter,
				kLeave,
				kInvalid
			};
	public:
		Event()
			:
			type_{ Type::kInvalid },
			x_{ 0 },
			y_{ 0 },
			left_is_pressed_{ false },
			right_is_pressed_{ false }
		{}

		Event(Type type, const Mouse& parent)
			:
			type_(type),
			left_is_pressed_(parent.left_is_pressed_),
			right_is_pressed_(parent.right_is_pressed_),
			x_(parent.x_),
			y_(parent.y_)
		{}
		
		bool IsValid() const
		{
			return type_ != Type::kInvalid;
		}

		Type GetType() const
		{
			return type_;
		}

		std::pair<int,int> GetPos() const
		{
			return { x_,y_ };
		}

		int GetPosX() const
		{
			return x_;
		}

		int GetPosY() const
		{
			return y_;
		}

		bool LeftIsPressed() const
		{
			return left_is_pressed_;
		}

		bool RightIsPressed() const
		{
			return right_is_pressed_;
		}
	private:
		Type type_;
		int x_;
		int y_;
		bool left_is_pressed_;
		bool right_is_pressed_;
	};
public:
	/*Mouse() = default;*/
	std::pair<int,int> GetPos() const;
	int GetPosX() const;
	int GetPosY() const;
	bool LeftIsPressed() const;
	bool RightIsPressed() const;
	bool IsInWindow() const;
	std::optional<Event> Read();
	bool IsEmpty() const;
	void Clear();
private:
	void OnMouseMove(int x, int y);
	void OnMouseLeave();
	void OnMouseEnter();
	void OnLeftIsPressed(int x, int y);
	void OnLeftIsReleased(int x, int y);
	void OnRightIsPressed(int x, int y);
	void OnRightIsReleased(int x, int y);
	void OnWheelUp(int x, int y);
	void OnWheelDown(int x, int y);
	void TrimBuffer();
private:
	int x_;
	int y_;
	static constexpr unsigned int kBufferSize_ = 4u;
	bool left_is_pressed_ = false;
	bool right_is_pressed_ = false;
	bool is_in_window_ = false;
	std::queue<Event> buffer_;
};

#endif // !MOUSE_H
