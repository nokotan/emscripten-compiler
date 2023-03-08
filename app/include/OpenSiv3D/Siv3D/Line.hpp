﻿//-----------------------------------------------
//
//	This file is part of the Siv3D Engine.
//
//	Copyright (c) 2008-2019 Ryo Suzuki
//	Copyright (c) 2016-2019 OpenSiv3D Project
//
//	Licensed under the MIT License.
//
//-----------------------------------------------

# pragma once
# include "Fwd.hpp"
# include "PointVector.hpp"
# include "Circular.hpp"
# include "Geometry2D.hpp"
# include "ByteArrayViewAdapter.hpp"

namespace s3d
{
	struct LineStyle
	{
		struct Parameters
		{
			double dotOffset;

			bool hasCap;

			bool isRound;

			bool isDotted;

			bool hasAlignedDot;

			constexpr Parameters operator ()(double _dotOffset) const noexcept
			{
				return Parameters{ _dotOffset, hasCap, isRound, isDotted, false };
			}

			constexpr Parameters offset(double _dotOffset) const noexcept
			{
				return Parameters{ _dotOffset, hasCap, isRound, isDotted, false };
			}
		};

		double dotOffset = 0.0;

		bool hasCap = true;

		bool isRound = false;

		bool isDotted = false;

		bool hasAlignedDot = true;

		constexpr LineStyle() = default;

		constexpr LineStyle(const Parameters& params)
			: dotOffset(params.dotOffset)
			, hasCap(params.hasCap)
			, isRound(params.isRound)
			, isDotted(params.isDotted)
			, hasAlignedDot(params.hasAlignedDot) {}

		[[nodiscard]] constexpr bool isSquareCap() const noexcept
		{
			return hasCap && !isRound && !isDotted;
		}

		[[nodiscard]] constexpr bool isRoundCap() const noexcept
		{
			return hasCap && isRound && !isDotted;
		}

		[[nodiscard]] constexpr bool isNoCap() const noexcept
		{
			return !hasCap && !isRound && !isDotted;
		}

		[[nodiscard]] constexpr bool isSquareDot() const noexcept
		{
			return hasCap && !isRound && isDotted;
		}

		[[nodiscard]] constexpr bool isRoundDot() const noexcept
		{
			return hasCap && isRound && isDotted;
		}

		static constexpr Parameters SquareCap{ 0.0, true, false, false, false };

		static constexpr Parameters RoundCap{ 0.0, true, true, false, false };

		static constexpr Parameters NoCap{ 0.0, false, false, false, false };

		static constexpr Parameters SquareDot{ 0.0, true, false, true, false };

		static constexpr Parameters RoundDot{ 0.0, true, true, true, true };

		static constexpr Parameters Default = SquareCap;
	};

	struct Line
	{
		using position_type = Vec2;

		using value_type = position_type::value_type;
		
		/// <summary>
		/// 線分の開始位置
		/// </summary>
		position_type begin;
		
		/// <summary>
		/// 線分の終点位置
		/// </summary>
		position_type end;
		
		Line() = default;
		
		constexpr Line(value_type x0, value_type y0, value_type x1, value_type y1) noexcept
			: begin(x0, y0)
			, end(x1, y1) {}
		
		constexpr Line(const position_type& p0, value_type x1, value_type y1) noexcept
			: begin(p0)
			, end(x1, y1) {}
		
		constexpr Line(value_type x0, value_type y0, const position_type& p1) noexcept
			: begin(x0, y0)
			, end(p1) {}
		
		constexpr Line(const position_type& p0, const position_type& p1) noexcept
			: begin(p0)
			, end(p1) {}

		constexpr Line(const position_type& origin, Arg::direction_<position_type> direction) noexcept
			: begin(origin)
			, end(origin + direction.value()) {}

		Line(const position_type& origin, Arg::angle_<value_type> angle, value_type length) noexcept
			: begin(origin)
			, end(origin + Circular(length, angle.value())) {}
		
		[[nodiscard]] constexpr bool operator ==(const Line& line) const noexcept
		{
			return begin == line.begin
				&& end == line.end;
		}
		
		[[nodiscard]] constexpr bool operator !=(const Line& line) const noexcept
		{
			return begin != line.begin
				|| end != line.end;
		}
		
		constexpr Line& set(value_type x0, value_type y0, value_type x1, value_type y1) noexcept
		{
			begin.set(x0, y0);
			end.set(x1, y1);
			return *this;
		}
		
		constexpr Line& set(const position_type& p0, value_type x1, value_type y1) noexcept
		{
			return set(p0.x, p0.y, x1, y1);
		}
		
		constexpr Line& set(value_type x0, value_type y0, const position_type& p1) noexcept
		{
			return set(x0, y0, p1.x, p1.y);
		}
		
		constexpr Line& set(const position_type& p0, const position_type& p1) noexcept
		{
			return set(p0.x, p0.y, p1.x, p1.y);
		}
		
		constexpr Line& set(const Line& line) noexcept
		{
			return *this = line;
		}
		
		[[nodiscard]] constexpr Line movedBy(value_type x, value_type y) const noexcept
		{
			return{ begin.movedBy(x, y), end.movedBy(x, y) };
		}
		
		[[nodiscard]] constexpr Line movedBy(const position_type& v) const noexcept
		{
			return movedBy(v.x, v.y);
		}
		
		constexpr Line& moveBy(value_type x, value_type y) noexcept
		{
			begin.moveBy(x, y);
			end.moveBy(x, y);
			return *this;
		}
		
		constexpr Line& moveBy(const position_type& v) noexcept
		{
			return moveBy(v.x, v.y);
		}

		[[nodiscard]] Line stretched(value_type length) const noexcept;

		[[nodiscard]] Line stretched(value_type lengthBegin, value_type lengthEnd) const noexcept;
		
		[[nodiscard]] constexpr position_type vector() const noexcept
		{
			return end - begin;
		}

		[[nodiscard]] constexpr Line reversed() const noexcept
		{
			return{ end, begin };
		}
		
		constexpr Line& reverse() noexcept
		{
			const position_type t = begin;
			begin = end;
			end = t;
			return *this;
		}
		
		[[nodiscard]] value_type length() const noexcept;
		
		[[nodiscard]] constexpr value_type lengthSq() const noexcept
		{
			return begin.distanceFromSq(end);
		}
		
		[[nodiscard]] constexpr position_type center() const noexcept
		{
			return position_type((begin.x + end.x) * 0.5, (begin.y + end.y) * 0.5);
		}
		
		[[nodiscard]] position_type closest(const position_type& pos) const noexcept;

		[[nodiscard]] RectF boundingRect() const noexcept;

		template <class Shape2DType>
		[[nodiscard]] bool intersects(const Shape2DType& shape) const
		{
			return Geometry2D::Intersect(*this, shape);
		}
		
		template <class Shape2DType>
		[[nodiscard]] Optional<Array<Vec2>> intersectsAt(const Shape2DType& shape) const
		{
			return Geometry2D::IntersectAt(*this, shape);
		}

		[[nodiscard]] Optional<position_type> intersectsAt(const Line& line) const;

		[[nodiscard]] Optional<position_type> intersectsAtPrecise(const Line& line) const;

		// rotated
		
		// rotatedAt
		
		const Line& paint(Image& dst, const Color& color) const;

		const Line& paint(Image& dst, int32 thickness, const Color& color) const;
		
		const Line& overwrite(Image& dst, const Color& color, bool antialiased = true) const;

		const Line& overwrite(Image& dst, int32 thickness, const Color& color, bool antialiased = true) const;

		const Line& paintArrow(Image& dst, double width, const Vec2& headSize, const Color& color) const;
		
		const Line& overwriteArrow(Image& dst, double width, const Vec2& headSize, const Color& color) const;
		
		const Line& draw(const ColorF& color = Palette::White) const;

		const Line& draw(const ColorF& colorBegin, const ColorF& colorEnd) const;

		const Line& draw(double thickness, const ColorF& color = Palette::White) const;

		const Line& draw(double thickness, const ColorF& colorBegin, const ColorF& colorEnd) const;

		const Line& draw(const LineStyle& style, double thickness, const ColorF& color = Palette::White) const;

		const Line& draw(const LineStyle& style, double thickness, const ColorF& colorBegin, const ColorF& colorEnd) const;

		const Line& drawArrow(double width = 1.0, const Vec2& headSize = Vec2(5.0, 5.0), const ColorF& color = Palette::White) const;

		// asPolygon;
	};
}

//////////////////////////////////////////////////
//
//	Format
//
//////////////////////////////////////////////////

namespace s3d
{
	void Formatter(FormatData& formatData, const Line& value);

	template <class CharType>
	inline std::basic_ostream<CharType>& operator <<(std::basic_ostream<CharType>& output, const Line& value)
	{
		return output << CharType('(')
			<< value.begin	<< CharType(',') << CharType(' ')
			<< value.end	<< CharType(')');
	}

	template <class CharType>
	inline std::basic_istream<CharType>& operator >>(std::basic_istream<CharType>& input, Line& value)
	{
		CharType unused;
		return input >> unused
			>> value.begin >> unused
			>> value.end >> unused;
	}
}

//////////////////////////////////////////////////
//
//	Hash
//
//////////////////////////////////////////////////

namespace std
{
	template <>
	struct hash<s3d::Line>
	{
		[[nodiscard]] size_t operator ()(const s3d::Line& value) const noexcept
		{
			return s3d::Hash::FNV1a(value);
		}
	};
}

//////////////////////////////////////////////////
//
//	fmt
//
//////////////////////////////////////////////////

namespace fmt_s3d
{
	template <>
	struct formatter<s3d::Line, s3d::char32>
	{
		s3d::String tag;

		template <class ParseContext>
		auto parse(ParseContext& ctx)
		{
			return s3d::detail::GetFmtTag(tag, ctx);
		}

		template <class Context>
		auto format(const s3d::Line& value, Context& ctx)
		{
			const s3d::String fmt = s3d::detail::MakeFmtArg(
				U"({:", tag, U"}, {:", tag, U"})"
			);

			return format_to(ctx.begin(), wstring_view(fmt.data(), fmt.size()), value.begin, value.end);
		}
	};
}
