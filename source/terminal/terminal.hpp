#pragma once

/** @file */

#include "terminal_object.hpp"

#include <cstdint>

#include <array>
#include <string>
#include <memory>
#include <vector>
#include <variant>
#include <functional>

namespace term
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
	struct ColorRGBA
	{
		using channel = uint8_t;

		channel r, g, b, a;

		ColorRGBA(channel _r, channel _g, channel _b, channel _a) :
			r(_r), g(_g), b(_b), a(_a)
		{};
		ColorRGBA(channel _r, channel _g, channel _b) :
			ColorRGBA(_r, _g, _b, 255)
		{};
		ColorRGBA(channel _fill) :
			ColorRGBA(_fill, _fill, _fill, _fill)
		{};
		ColorRGBA() :
			ColorRGBA(255)
		{};
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
		void set_background_color(ColorRGBA _color);

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



	struct Rectangle
	{
	public:

		void draw(PlainTerminal& _terminal)
		{
			_terminal.fill(this->x_, this->y_, this->w_, this->h_, this->fill_color_);
			if (this->frame_color_ != Color::none)
			{
				_terminal.draw_rectangle_frame(this->x_, this->y_, this->w_, this->h_,
					this->frame_color_);
			};
		};

		bool covers_cell(const CursorPos& _csPos) const
		{
			const auto [cx, cy] = _csPos;
			const auto l = this->x_;
			const auto r = this->x_ + this->w_;
			const auto t = this->y_;
			const auto b = this->y_ + this->h_;
			return (cx >= l && cx <= r && cy >= t && cy <= b);
		};

		int x_, y_, w_, h_;
		Color fill_color_ = Color::none;
		Color frame_color_ = Color::none;

		constexpr Rectangle() noexcept = default;
		constexpr Rectangle(int _x, int _y, int _w, int _h) noexcept :
			x_(_x), y_(_y), w_(_w), h_(_h)
		{};

	};



	struct TMButton : public term::ITerminalObject
	{
	protected:

		void on_draw() override;
		void on_attach() override;

	public:

		void set_pressed(bool _state);
		void cursor_exit();

		void press(int _mouseButton);
		void release(int _mouseButton);

		bool is_pressed() const
		{
			return this->pressed_;
		};

		void set_click_callback(std::function<void(TMButton&)> cb)
		{
			this->click_callback_ = std::move(cb);
		};

		bool covers_cell(const CursorPos& _pos) const
		{
			return this->rect_.covers_cell(_pos);
		};

		void set_label(const std::string& _label)
		{
			this->label_ = _label;
			this->draw();
		};



		TMButton();


		std::string label_;
		Rectangle rect_;
		Color off_color_ = Color::dark_gray;
		Color on_color_ = Color::green;
		bool pressed_ = false;

	private:
		std::function<void(TMButton&)> click_callback_;
	};



	class Terminal : public PlainTerminal
	{
	public:

		void open();

		TMButton* add_button(Rectangle _rect);
		TMButton* add_button(int x, int y, int w, int h);
		void erase(TMButton* _ptr);

		void update();



		Terminal() = default;
		~Terminal() = default;

	private:

		Terminal(const Terminal&) = delete;
		Terminal(Terminal&&) noexcept = delete;


		std::vector<std::unique_ptr<TMButton>> buttons_{};
	};

};