#include "terminal.hpp"

#include <jclib/functional.h>

#include <BearLibTerminal.h>

#include <iostream>

namespace term
{
	void PlainTerminal::open()
	{
		::terminal_open();
		terminal_set("window.resizeable = true");
		terminal_set("input.filter = [keyboard, mouse+, arrows]");
	};

	void PlainTerminal::close()
	{
		::terminal_close();
	};

	void PlainTerminal::update(const std::function<void(int)>& _inputCallback)
	{
		terminal_refresh();

		while (true)
		{
			const auto _event = terminal_peek();
			if (_event == 0)
			{
				break;
			}
			else
			{
				terminal_read();
			};

			// Forward to callback
			if (_inputCallback)
			{
				_inputCallback(_event);
			};
		};
	};
	void PlainTerminal::update()
	{
		this->update([this](int _event)
			{
				if (_event == TK_CLOSE)
				{
					this->set_should_close();
				}
				else
				{
					if (this->on_mouse_button_)
					{
						int _button = 0;
						int _state = 0;

						if (_event == TK_MOUSE_LEFT)
						{
							// m1 press
							_button = TK_MOUSE_LEFT;
							_state = 1;
						}
						else if (_event == (TK_MOUSE_LEFT | TK_KEY_RELEASED))
						{
							// m1 release
							_button = TK_MOUSE_LEFT;
							_state = 0;
						}
						else if (_event == TK_MOUSE_RIGHT)
						{
							// m2 press
							_button = TK_MOUSE_RIGHT;
							_state = 1;
						}
						else if (_event == (TK_MOUSE_RIGHT | TK_KEY_RELEASED))
						{
							// m2 release
							_button = TK_MOUSE_RIGHT;
							_state = 0;
						};

						if (_button != 0)
						{
							this->on_mouse_button_(_button, _state);
						};
					};
				};
			});
	};
	


	bool PlainTerminal::should_close()
	{
		return this->close_flag_;
	};
	void PlainTerminal::set_should_close()
	{
		this->close_flag_ = true;
	};


	void PlainTerminal::set_background_color(Color _color)
	{
		const char* _name = "";
		switch (_color)
		{
		case Color::none: return;
		case Color::black:
			_name = "black";
			break;
		case Color::gray:
			_name = "gray";
			break;
		case Color::white:
			_name = "white";
			break;
		case Color::cyan:
			_name = "cyan";
			break;
		case Color::dark_gray:
			_name = "dark gray";
			break;
		case Color::green:
			_name = "green";
			break;
		case Color::red:
			_name = "red";
			break;
		default:
			abort();
			break;
		};
		terminal_bkcolor(_name);
	};
	void PlainTerminal::set_background_color(ColorRGBA _color)
	{
		const auto _colorTK = color_from_argb(_color.r, _color.g, _color.b, _color.a);
		terminal_bkcolor(_colorTK);
	};

	CursorPos PlainTerminal::get_cursor_pos() const
	{
		const auto x = terminal_state(TK_MOUSE_X);
		const auto y = terminal_state(TK_MOUSE_Y);
		return CursorPos(x, y);
	};



	void PlainTerminal::fill(const int x, const int y, const int w, const int h, Color _color)
	{
		this->set_background_color(_color);
		terminal_clear_area(x, y, w, h);
	};

	void PlainTerminal::print(const int x, const int y, const char* _str)
	{
		terminal_print(x, y, _str);
	};

	void PlainTerminal::draw_vertical_line(const int x, const int y, const int h)
	{
		for (int cy = y; cy <= y + h; ++cy)
		{
			terminal_put(x, cy, ' ');
		};
	};
	void PlainTerminal::draw_horizontal_line(const int x, const int y, const int w)
	{
		for (int cx = x; cx <= x + w; ++cx)
		{
			terminal_put(cx, y, ' ');
		};
	};

	void PlainTerminal::draw_rectangle_frame(const int x, const int y, const int w, const int h,
		Color _frameColor, Color _fillColor)
	{
		this->set_background_color(_frameColor);

		this->draw_horizontal_line(x, y, w);
		this->draw_horizontal_line(x, y + h, w);
		this->draw_vertical_line(x, y, h);
		this->draw_vertical_line(x + w, y, h);

		if (_fillColor != Color::none && w > 1 && h > 1)
		{
			this->draw_rectangle(x + 1, y + 1, w - 1, h - 1, _fillColor);
		};

	};

	void PlainTerminal::draw_rectangle(const int x, const int y, const int w, const int h, Color _color)
	{
		this->fill(x, y, w, h, _color);
	};


};

namespace term
{
	void TMButton::on_draw()
	{
		auto& _terminal = this->terminal();
		auto& _rect = this->rect_;

		_rect.draw(_terminal);

		if (!this->label_.empty())
		{
			_terminal.set_background_color(this->rect_.frame_color_);
			_terminal.print(this->rect_.x_ + 1, this->rect_.y_, this->label_.c_str());
		};
		
	};
	void TMButton::on_attach()
	{
		this->rect_.fill_color_ = (this->pressed_)? this->on_color_ : this->off_color_;
	};

	void TMButton::set_pressed(bool _state)
	{
		if (this->pressed_ != _state)
		{
			// Update variables first.
			this->pressed_ = _state;
			this->rect_.fill_color_ = (_state) ?
				this->on_color_ : this->off_color_;

			// If clicked, invoke on click callback if set.
			if (_state == 0 && this->click_callback_)
			{
				this->click_callback_(*this);
			};
		};
	};

	void TMButton::cursor_exit()
	{
		this->pressed_ = false;
		this->rect_.fill_color_ = this->off_color_;
		this->draw();
	};

	void TMButton::press(int _mouseButton)
	{
		if (_mouseButton == TK_MOUSE_LEFT)
		{
			this->set_pressed(true);
			this->draw();
		};
	};
	void TMButton::release(int _mouseButton)
	{
		if (_mouseButton == TK_MOUSE_LEFT)
		{
			this->set_pressed(false);
			this->draw();
		};
	};

	TMButton::TMButton()
	{
		this->rect_.frame_color_ = Color::dark_gray;
	};
}

namespace term
{
	void Terminal::open()
	{
		PlainTerminal::open();

		this->set_mouse_button_callback([this](int _button, int _state)
			{
				const auto _csPos = this->get_cursor_pos();

				TMButton* _bt{};
				for (auto& _button : this->buttons_)
				{
					if (_button->covers_cell(_csPos))
					{
						_bt = _button.get();
						break;
					};
				};

				if (_bt)
				{
					if (_state)
					{
						_bt->press(_button);
					}
					else
					{
						_bt->release(_button);
					};
				};
			});
	};

	TMButton* Terminal::add_button(Rectangle _rect)
	{
		auto _button = std::make_unique<TMButton>();
		auto _buttonPtr = _button.get();

		this->buttons_.push_back(std::move(_button));
		_buttonPtr->rect_ = _rect;
		_buttonPtr->attach(*this);

		return _buttonPtr;
	};
	TMButton* Terminal::add_button(int x, int y, int w, int h)
	{
		return this->add_button(Rectangle(x, y, w, h));
	};

	void Terminal::erase(TMButton* _ptr)
	{
		std::erase_if(this->buttons_,
			jc::dereference | jc::address_of | jc::equals & _ptr);
	};

	void Terminal::update()
	{
		PlainTerminal::update();

		auto _csPos = this->get_cursor_pos();
		for (auto& _button : this->buttons_)
		{
			if (_button->pressed_ && !_button->covers_cell(_csPos))
			{
				_button->cursor_exit();
			};
		};
	};

};