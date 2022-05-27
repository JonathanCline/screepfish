#include "terminal.hpp"
#include "chess/fen.hpp"
#include "chess/move.hpp"

#include <lodepng.h>

#include <BearLibTerminal.h>

#include <iostream>
#include <algorithm>
#include <numeric>
#include <charconv>
#include <filesystem>

namespace chess
{
	inline std::string itohex(int i)
	{
		std::string _buffer(12, 0);
		auto [p, ec] = std::to_chars(_buffer.data(), _buffer.data() + _buffer.size(), i, 16);
		_buffer.resize(p - _buffer.data());
		return _buffer;
	};
	
	void Terminal::wait_for_any_key()
	{
		terminal_read();
	};

	bool Terminal::should_close() const
	{
		if (const auto ev = terminal_peek(); ev != 0)
		{
			terminal_read();
			if (ev == TK_CLOSE)
			{
				return true;
			};
		};
		return false;
	};

	inline int white_piece_unicode(PieceType _piece)
	{
		switch (_piece)
		{
		case Piece::pawn:
			return 0x2659;
		case Piece::knight:
			return 0x2658;
		case Piece::bishop:
			return 0x2657;
		case Piece::rook:
			return 0x2656;
		case Piece::queen:
			return 0x2655;
		case Piece::king:
			return 0x2654;
		default:
			return (int)'?'; 
		};
	};
	inline int black_piece_unicode(PieceType _piece)
	{
		switch (_piece)
		{
		case Piece::pawn:
			return 0x265F;
		case Piece::knight:
			return 0x265E;
		case Piece::bishop:
			return 0x265D;
		case Piece::rook:
			return 0x265C;
		case Piece::queen:
			return 0x265B;
		case Piece::king:
			return 0x265A;
		default:
			return (int)'?'; 
		};
	};
	inline int piece_unicode(Piece _piece)
	{
		if (_piece.color() == Color::white)
		{
			return white_piece_unicode(_piece.type());
		}
		else
		{
			return black_piece_unicode(_piece.type());
		};
	};
		

	void Terminal::set_board(const chess::Board& _board)
	{
		int _offsetX = 0;
		int _offsetY = 0;
		bool _whiteFlag = false;
		

		for (auto& v : positions_v)
		{
			terminal_bkcolor((square_color(v) == Color::white)? "gray" : "dark gray");
			
			const auto r = (int)Rank::r8 - (int)v.rank();

			const auto pc = _board.get(v);
			if (pc)
			{
				const auto uni = piece_unicode(pc);
				if (pc == Piece::king)
				{
					if (chess::is_check(_board, pc.color()))
					{
						terminal_bkcolor("light red");
					};
				};
				terminal_put((int)v.file() * 2, r, uni);
			}
			else
			{
				terminal_put((int)v.file() * 2, r, ' ');
				terminal_put(((int)v.file() * 2) + 1, r, ' ');
			};
		};

		terminal_refresh();

	};



	struct RGBA
	{
		uint64_t bits() const noexcept
		{ 
			return *reinterpret_cast<const uint64_t*>(&this->r);
		};

		unsigned char r, g, b, a;
	};
	
	inline RGBA blend(RGBA p0, RGBA p1)
	{
		return RGBA
		{
			std::midpoint(p0.r, p1.r),
			std::midpoint(p0.g, p1.g),
			std::midpoint(p0.b, p1.b),
			std::midpoint(p0.a, p1.a)
		};
	};
	inline RGBA blend(RGBA p0, RGBA p1, RGBA p2, RGBA p3)
	{
		const auto b0 = blend(p0, p1);
		const auto b1 = blend(p2, p3);
		return blend(b0, b1);
	};

	struct Image
	{
		std::vector<RGBA> pixels; 
		unsigned w, h;

		const size_t idx(unsigned x, unsigned y) const
		{
			return (y * this->w) + x;
		};

		RGBA& at(unsigned x, unsigned y)
		{
			return this->pixels.at(this->idx(x, y));
		};
		const RGBA& at(unsigned x, unsigned y) const
		{
			return this->pixels.at(this->idx(x, y));
		};

	};
	inline int load(Image& _out, const std::string& _path)
	{
		std::vector<unsigned char> _pixels{};
		unsigned w, h;
		if (const auto er = lodepng::decode(_pixels, w, h, _path); er != 0)
		{
			return er;
		};

		_out.w = w;
		_out.h = h;
		_out.pixels.resize(_pixels.size() / 4);

		auto p = reinterpret_cast<RGBA*>(_pixels.data());
		std::copy(p, p + (_pixels.size() / 4), _out.pixels.data());
		return 0;
	};
	inline int save(const Image& _image, const std::string& _path)
	{
		return lodepng::encode(_path, reinterpret_cast<const unsigned char*>(_image.pixels.data()), _image.w, _image.h);
	};

	inline Image scaledown_half(const Image& _inImage)
	{
		auto _image = Image{};
		_image.w = _inImage.w / 2;
		_image.h = _inImage.h / 2;
		_image.pixels.resize(_image.w * _image.h);

		for (unsigned x = 0; x != _inImage.w; x += 2)
		{
			for (unsigned y = 0; y != _inImage.h; y += 2)
			{
				auto p0 = _inImage.at(x, y);
				auto p1 = _inImage.at(x + 1, y);
				auto p2 = _inImage.at(x, y + 1);
				auto p3 = _inImage.at(x + 1, y + 1);
				const auto bp = blend(p0, p1, p2, p3);
				_image.at(x >> 1, y >> 1) = bp;
			};
		};

		return _image;
	};


	inline void make_scaledown_set(const std::string& _fromSet, unsigned _newSize)
	{
		namespace fs = std::filesystem;
		auto sp = fs::path(_fromSet);
		auto odp = sp.parent_path() / std::to_string(_newSize);
		if (!fs::exists(odp))
			fs::create_directory(odp);
		std::cout << odp.generic_string() << '\n';
		std::cout << sp << '\n';
		for (auto& v : fs::directory_iterator(sp))
		{
			if (v.path().extension() == ".png")
			{
				auto i = Image{};
				if (load(i, v.path().generic_string()) != 0) { std::cout << "[ERROR] Failed to load from " << v.path() << std::endl; };
				std::cout << i.w << ", " << i.h << '\n';
				i = scaledown_half(i);
				auto op = odp / v.path().filename();
				if (save(i, op.generic_string()) != 0) { std::cout << "[ERROR] Failed to load from " << v.path() << std::endl; };
			};
		};
	};
	inline void gen_scaledowns(const std::filesystem::path& _fullSet, unsigned _fullSize)
	{
		unsigned s = _fullSize;
		auto pd = _fullSet.parent_path();
		auto p = _fullSet;
		while (s > 32)
		{
			s = s / 2;
			make_scaledown_set(p.generic_string(), s);
			p = pd / std::to_string(s);
		};
	};

	inline void set_unicode_image(Piece _piece, const char* _name, unsigned px, const std::filesystem::path& _assetsDir)
	{
		const auto p = (_assetsDir / std::to_string(px) / _name).generic_string() + ".png";
		std::cout << p << std::endl;

		const auto op = "0x" + itohex(piece_unicode(_piece)) + ": " + p + ", align=top-left, spacing=2x1";
		std::cout << op << '\n';

		if (!terminal_set(op.c_str()))
		{
			std::cout << "[Error] Failed to set " << op << '\n';
		};
	};





	Terminal::Terminal(const char* _assetsDirectoryPathStr) :
		cw_(32), ch_(64)
	{
		namespace fs = std::filesystem;

		const auto _assetsDirectoryPath = fs::path(_assetsDirectoryPathStr);

		terminal_open();

		terminal_set("window.size=16x8");
		{
			const auto s = std::string("font: C:/Windows/fonts/CascadiaMono.ttf, size=")
				+ std::to_string(this->cw_) + "x" + std::to_string(this->ch_);
			terminal_set(s.c_str());
		};

		auto _tempDir = _assetsDirectoryPath / "_tmp";
		if (!fs::exists(_tempDir))
		{
			fs::create_directory(_tempDir);
		};
		

		if(fs::exists(_assetsDirectoryPath) && fs::is_directory(_assetsDirectoryPath))
		{
			set_unicode_image(Piece(PieceType::king, Color::white), "king_white", this->ch_, _assetsDirectoryPath);
			set_unicode_image(Piece(PieceType::queen, Color::white), "queen_white", this->ch_, _assetsDirectoryPath);
			set_unicode_image(Piece(PieceType::bishop, Color::white), "bishop_white", this->ch_, _assetsDirectoryPath);
			set_unicode_image(Piece(PieceType::knight, Color::white), "knight_white", this->ch_, _assetsDirectoryPath);
			set_unicode_image(Piece(PieceType::rook, Color::white), "rook_white", this->ch_, _assetsDirectoryPath);
			set_unicode_image(Piece(PieceType::pawn, Color::white), "pawn_white", this->ch_, _assetsDirectoryPath);

			set_unicode_image(Piece(PieceType::king, Color::black), "king_black", this->ch_, _assetsDirectoryPath);
			set_unicode_image(Piece(PieceType::queen, Color::black), "queen_black", this->ch_, _assetsDirectoryPath);
			set_unicode_image(Piece(PieceType::bishop, Color::black), "bishop_black", this->ch_, _assetsDirectoryPath);
			set_unicode_image(Piece(PieceType::knight, Color::black), "knight_black", this->ch_, _assetsDirectoryPath);
			set_unicode_image(Piece(PieceType::rook, Color::black), "rook_black", this->ch_, _assetsDirectoryPath);
			set_unicode_image(Piece(PieceType::pawn, Color::black), "pawn_black", this->ch_, _assetsDirectoryPath);

			std::cout << "Loaded assets\n";
		}
		else
		{
			std::cout << "No assets directory\n";
		};

		terminal_refresh();
	};

	Terminal::~Terminal()
	{
		terminal_close();
	};
};
