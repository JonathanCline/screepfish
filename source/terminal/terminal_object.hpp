#pragma once

/** @file */

#include <cassert>
#include <cstdlib>

namespace term
{
	class Terminal;


	class ITerminalObject
	{
	protected:

		virtual void on_update() {};
		virtual void on_draw() {};
		virtual void on_attach() {};
		virtual void on_detach() {};

	public:

		Terminal& terminal() const noexcept
		{
			assert(this->is_attached());
			return *this->terminal_;
		};

		bool is_attached() const
		{
			return this->terminal_ != nullptr;
		};
		bool is_attached_to(const Terminal& _terminal) const
		{
			return this->terminal_ == &_terminal;
		};

		void detach()
		{
			if (auto& p = this->terminal_; p)
			{
				this->terminal_ = nullptr;
				this->on_detach();
			};
		};
		void attach(Terminal& _terminal)
		{
			if (this->is_attached_to(_terminal))
			{
				abort();
			};

			if (this->is_attached())
			{
				this->detach();
			};
			this->terminal_ = &_terminal;

			this->on_attach();
		};

		virtual void update() final
		{
			if (this->is_attached())
			{
				this->on_update();
			};
		};
		virtual void draw() final
		{
			if (this->is_attached())
			{
				this->on_draw();
			};
		};

		ITerminalObject() = default;
		virtual ~ITerminalObject() = default;

	private:
		Terminal* terminal_{};
	};


};
