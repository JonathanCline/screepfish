#pragma once

/** @file */

#include <string>
#include <memory>
#include <vector>
#include <functional>

namespace sch::tm
{
	enum class Color
	{
		none = 0,
		black = 1,
		white,
		gray,
		dark_gray,
		cyan,
		green,
		red,
	};

	/**
	 * @brief Cursor position stored as an x, y cell coordinate pair
	*/
	struct CursorPos
	{
		int x, y;
		
		constexpr CursorPos() = default;
		constexpr CursorPos(int _x, int _y) :
			x(_x), y(_y)
		{};
	};

	class PlainTerminal
	{
	protected:

		void update(const std::function<void(int)>& _inputCallback);

	public:

		void open();
		void close();
		void update();
		bool should_close();
		void set_should_close();

		void set_background_color(Color _color);

		CursorPos get_cursor_pos() const;


		void fill(const int x, const int y, const int w, const int h,
			Color _color = Color::none);

		void print(const int x, const int y, const char* _str);


		void draw_vertical_line(const int x, const int y, const int h);
		void draw_horizontal_line(const int x, const int y, const int w);

		void draw_rectangle_frame(const int x, const int y, const int w, const int h,
			Color _frameColor = Color::gray, Color _fillColor = Color::none);

		void draw_rectangle(const int x, const int y, const int w, const int h, Color _color);



		void set_mouse_button_callback(std::function<void(int _button, int _state)> _cb)
		{
			this->on_mouse_button_ = _cb;
		};


		PlainTerminal() = default;
		~PlainTerminal() = default;


		PlainTerminal(const PlainTerminal&) = delete;
		PlainTerminal(PlainTerminal&&) noexcept = delete;
	private:
		bool close_flag_ = false;
		std::function<void(int, int)> on_mouse_button_;

	};



	class TerminalObject
	{
	public:
		TerminalObject() = default;
		~TerminalObject() = default;

		PlainTerminal* terminal_{};
	};

	struct Button : public TerminalObject
	{
	public:

		std::function<void(Button&, int, int)> on_mouse_event_;
		std::string label_;
		int x_, y_, w_, h_;

		Color frame_color_ = Color::gray;
		Color off_color_ = Color::dark_gray;
		Color on_color_ = Color::green;

		bool pressed_ = false;

		bool overlaps(const int cx, const int cy) const
		{
			const auto l = this->x_;
			const auto r = this->x_ + this->w_;
			const auto t = this->y_;
			const auto b = this->y_ + this->h_;
			return (cx >= l && cx <= r && cy >= t && cy <= b);
		};

		void draw()
		{
			auto& _terminal = *this->terminal_;

			_terminal.draw_rectangle_frame
			(
				this->x_, this->y_, this->w_, this->h_,
				this->frame_color_,
				(this->pressed_)? this->on_color_ : this->off_color_
			);

			if (!this->label_.empty())
			{
				_terminal.print(this->x_ + 1, this->y_ + 1, this->label_.c_str());
			};
		};



		Button() = default;
	};



	class Terminal : public PlainTerminal
	{
	public:

		void open();

		Button* add_button(int x, int y, int w, int h);
		void erase(Button* _ptr);

		void update();



		Terminal() = default;
		~Terminal() = default;

	private:

		Terminal(const Terminal&) = delete;
		Terminal(Terminal&&) noexcept = delete;


		std::vector<std::unique_ptr<Button>> buttons_{};
	};

};