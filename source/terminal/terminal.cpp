#include "terminal.hpp"

#include <jclib/functional.h>

#include <BearLibTerminal.h>

#include <iostream>

namespace sch::tm
{
	void PlainTerminal::open()
	{
		::terminal_open();

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

namespace sch::tm
{
	void Terminal::open()
	{
		PlainTerminal::open();

		this->set_mouse_button_callback([this](int _button, int _state)
			{
				const auto cx = terminal_state(TK_MOUSE_X);
				const auto cy = terminal_state(TK_MOUSE_Y);
				
				Button* _bt{};
				for (auto& _button : this->buttons_)
				{
					if (_button->overlaps(cx, cy))
					{
						_bt = _button.get();
						break;
					};
				};

				if (_bt && _bt->on_mouse_event_)
				{
					_bt->on_mouse_event_(*_bt, _button, _state);
				};
			});
	};



	Button* Terminal::add_button(int x, int y, int w, int h)
	{
		{
			auto _button = Button{};
			_button.terminal_ = this;
			_button.x_ = x; _button.y_ = y; _button.w_ = w; _button.h_ = h;
			this->buttons_.push_back(std::make_unique<Button>(_button));
		};

		auto _button = this->buttons_.back().get();
		_button->draw();
		return _button;
	};
	void Terminal::erase(Button* _ptr)
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
			if (_button->pressed_ && !_button->overlaps(_csPos.x, _csPos.y))
			{
				_button->on_mouse_event_(*_button, TK_MOUSE_LEFT, 0);
			};
		};
	};

};