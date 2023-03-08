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
# include <cmath>
# include <utility>
# include "Fwd.hpp"
# include "PointVector.hpp"
# include "Format.hpp"
# include "Hash.hpp"

namespace s3d
{
	/// <summary>
	/// 3 次元ベクトル
	/// </summary>
	template <class Type>
	struct Vector3D
	{
		template <class U>
		using vector_type = Vector3D<U>;

		using value_type = Type;

		value_type x, y, z;

		/// <summary>
		/// デフォルトコンストラクタ
		/// </summary>
		Vector3D() noexcept = default;

		constexpr Vector3D(const Vector3D&) noexcept = default;

		template <class X, class Y, class Z>
		constexpr Vector3D(X _x, Y _y, Z _z) noexcept
			: x(static_cast<value_type>(_x))
			, y(static_cast<value_type>(_y))
			, z(static_cast<value_type>(_z)) {}

		constexpr Vector3D(value_type _x, value_type _y, value_type _z) noexcept
			: x(_x)
			, y(_y)
			, z(_z) {}

		template <class Z>
		constexpr Vector3D(const Vector2D<value_type>& xy, Z _z) noexcept
			: x(xy.x)
			, y(xy.y)
			, z(static_cast<value_type>(_z)) {}

		template <class X>
		constexpr Vector3D(X _x, const Vector2D<value_type>& yz) noexcept
			: x(static_cast<value_type>(_x))
			, y(yz.x)
			, z(yz.y) {}

		template <class U>
		constexpr Vector3D(const Vector3D<U>& v) noexcept
			: x(static_cast<value_type>(v.x))
			, y(static_cast<value_type>(v.y))
			, z(static_cast<value_type>(v.z)) {}

		[[nodiscard]] constexpr value_type elem(size_t index) const noexcept
		{
			return index == 0 ? x
				: index == 1 ? y
				: index == 2 ? z
				: 0;
		}

		[[nodiscard]] constexpr Vector3D operator +() const noexcept
		{
			return *this;
		}

		[[nodiscard]] constexpr Vector3D operator -() const noexcept
		{
			return{ -x, -y, -z };
		}

		[[nodiscard]] constexpr Vector3D operator +(const Vector3D& v) const noexcept
		{
			return{ x + v.x, y + v.y, z + v.z };
		}

		[[nodiscard]] constexpr Vector3D operator -(const Vector3D& v) const noexcept
		{
			return{ x - v.x, y - v.y, z - v.z };
		}

		[[nodiscard]] constexpr Vector3D operator *(value_type s) const noexcept
		{
			return{ x * s, y * s, z * s };
		}

		[[nodiscard]] constexpr Vector3D operator *(const Vector3D& v) const noexcept
		{
			return{ x * v.x, y * v.y, z * v.z };
		}

		[[nodiscard]] constexpr Vector3D operator /(value_type s) const noexcept
		{
			return *this * (static_cast<value_type>(1.0) / s);
		}

		[[nodiscard]] constexpr Vector3D operator /(const Vector3D& v) const noexcept
		{
			return{ x / v.x, y / v.y, z / v.z };
		}

		constexpr Vector3D& operator +=(const Vector3D& v) noexcept
		{
			x += v.x; y += v.y; z += v.z;
			return *this;
		}

		constexpr Vector3D& operator -=(const Vector3D& v) noexcept
		{
			x -= v.x; y -= v.y; z -= v.z;
			return *this;
		}

		constexpr Vector3D& operator *=(value_type s) noexcept
		{
			x *= s; y *= s; z *= s;
			return *this;
		}

		constexpr Vector3D& operator *=(const Vector3D& v) noexcept
		{
			x *= v.x; y *= v.y; z *= v.z;
			return *this;
		}

		constexpr Vector3D& operator /=(value_type s) noexcept
		{
			return *this *= (static_cast<value_type>(1.0) / s);
		}

		constexpr Vector3D& operator /=(const Vector3D& v) noexcept
		{
			x /= v.x; y /= v.y; z /= v.z;
			return *this;
		}

		[[nodiscard]] constexpr bool operator ==(const Vector3D& v) const noexcept
		{
			return v.x == x && v.y == y && v.z == z;
		}

		[[nodiscard]] constexpr bool operator !=(const Vector3D& v) const noexcept
		{
			return v.x != x || v.y != y || v.z != z;
		}

		constexpr Vector3D& set(value_type _x, value_type _y, value_type _z) noexcept
		{
			x = _x; y = _y; z = _z;
			return *this;
		}

		constexpr Vector3D& set(const Vector2D<value_type>& xy, value_type _z) noexcept
		{
			x = xy.x; y = xy.y; z = _z;
			return *this;
		}

		constexpr Vector3D& set(value_type _x, const Vector2D<value_type>& yz) noexcept
		{
			x = _x; y = yz.x; z = yz.y;
			return *this;
		}

		constexpr Vector3D& set(const Vector3D& v) noexcept
		{
			return *this = v;
		}

		[[nodiscard]] constexpr Vector3D movedBy(value_type _x, value_type _y, value_type _z) const noexcept
		{
			return{ x + _x, y + _y, z + _z };
		}

		[[nodiscard]] constexpr Vector3D movedBy(const Vector3D& v) const noexcept
		{
			return{ x + v.x, y + v.y, z + v.z };
		}

		constexpr Vector3D& moveBy(value_type _x, value_type _y, value_type _z) noexcept
		{
			x += _x; y += _y; z += _z;
			return *this;
		}

		constexpr Vector3D& moveBy(const Vector3D& v) noexcept
		{
			return *this += v;
		}

		[[nodiscard]] constexpr bool isZero() const noexcept
		{
			return x == static_cast<value_type>(0.0)
				&& y == static_cast<value_type>(0.0)
				&& z == static_cast<value_type>(0.0);
		}

		[[nodiscard]] bool hasNaN() const noexcept
		{
			return std::isnan(x)
				|| std::isnan(y)
				|| std::isnan(z);
		}

		[[nodiscard]] constexpr Type dot(const Vector3D& v) const noexcept
		{
			return x * v.x + y * v.y + z * v.z;
		}

		[[nodiscard]] constexpr Vector3D cross(const Vector3D& v) const noexcept
		{
			return{ y * v.z - z * v.y, z * v.x - x * v.z, x * v.y - y * v.x };
		}

		[[nodiscard]] value_type length() const noexcept
		{
			return std::sqrt(lengthSq());
		}

		[[nodiscard]] constexpr value_type lengthSq() const noexcept
		{
			return dot(*this);
		}

		[[nodiscard]] value_type lengthInv() const noexcept
		{
			return static_cast<value_type>(1.0) / length();
		}

		Vector3D& setLength(value_type _length) noexcept
		{
			const value_type len = length();

			if (len == 0.0)
			{
				return *this;
			}

			return *this *= (_length / len);
		}

		[[nodiscard]] Vector3D clampLength(value_type maxLength) const noexcept
		{
			const value_type len = length();

			if (len <= maxLength)
			{
				return *this;
			}
			else
			{
				return *this * (maxLength / len);
			}
		}

		[[nodiscard]] value_type distanceFrom(double _x, double _y, double _z) const noexcept
		{
			return (*this - Vector3D(_x, _y, _z)).length();
		}

		[[nodiscard]] value_type distanceFrom(const Vector3D& v) const noexcept
		{
			return (*this - v).length();
		}

		[[nodiscard]] constexpr value_type distanceFromSq(double _x, double _y, double _z) const noexcept
		{
			return (*this - Vector3D(_x, _y, _z)).lengthSq();
		}

		[[nodiscard]] constexpr value_type distanceFromSq(const Vector3D& v) const noexcept
		{
			return (*this - v).lengthSq();
		}

		[[nodiscard]] Vector3D normalized() const noexcept
		{
			return *this * lengthInv();
		}

		Vector3D& normalize() noexcept
		{
			return *this *= lengthInv();
		}

		[[nodiscard]] constexpr Vector3D lerp(const Vector3D& other, double f) const noexcept
		{
			return Vector3D(x + (other.x - x) * f, y + (other.y - y) * f, z + (other.z - z) * f);
		}

		/// <summary>
		/// Vector2D{ x, x }
		/// </summary>
		[[nodiscard]] constexpr Vector2D<value_type> xx() const noexcept
		{
			return{ x, x };
		}

		/// <summary>
		/// Vector2D{ x, y }
		/// </summary>
		[[nodiscard]] constexpr Vector2D<value_type> xy() const noexcept
		{
			return{ x, y };
		}

		/// <summary>
		/// Vector2D{ x, z }
		/// </summary>
		[[nodiscard]] constexpr Vector2D<value_type> xz() const noexcept
		{
			return{ x, z };
		}

		/// <summary>
		/// Vector2D{ y, x }
		/// </summary>
		[[nodiscard]] constexpr Vector2D<value_type> yx() const noexcept
		{
			return{ y, x };
		}

		/// <summary>
		/// Vector2D{ y, y }
		/// </summary>
		[[nodiscard]] constexpr Vector2D<value_type> yy() const noexcept
		{
			return{ y, y };
		}

		/// <summary>
		/// Vector2D{ y, z }
		/// </summary>
		[[nodiscard]] constexpr Vector2D<value_type> yz() const noexcept
		{
			return{ y, z };
		}

		/// <summary>
		/// Vector2D{ z, x }
		/// </summary>
		[[nodiscard]] constexpr Vector2D<value_type> zx() const noexcept
		{
			return{ z, x };
		}

		/// <summary>
		/// Vector2D{ z, y }
		/// </summary>
		[[nodiscard]] constexpr Vector2D<value_type> zy() const noexcept
		{
			return{ z, y };
		}

		/// <summary>
		/// Vector2D{ z, z }
		/// </summary>
		[[nodiscard]] constexpr Vector2D<value_type> zz() const noexcept
		{
			return{ z, z };
		}

		/// <summary>
		/// Vector3D{ x, x, x }
		/// </summary>
		[[nodiscard]] constexpr Vector3D xxx() const noexcept
		{
			return{ x, x, x };
		}

		/// <summary>
		/// Vector3D{ y, y, y }
		/// </summary>
		[[nodiscard]] constexpr Vector3D yyy() const noexcept
		{
			return{ y, y, y };
		}

		/// <summary>
		/// Vector3D{ z, z, z }
		/// </summary>
		[[nodiscard]] constexpr Vector3D zzz() const noexcept
		{
			return{ z, z, z };
		}

		/// <summary>
		/// Vector3D{ x, y, z }
		/// </summary>
		[[nodiscard]] constexpr Vector3D xyz() const noexcept
		{
			return{ x, y, z };
		}

		/// <summary>
		/// Vector3D{ x, z, y }
		/// </summary>
		[[nodiscard]] constexpr Vector3D xzy() const noexcept
		{
			return{ x, z, y };
		}

		/// <summary>
		/// Vector3D{ y, x, z }
		/// </summary>
		[[nodiscard]] constexpr Vector3D yxz() const noexcept
		{
			return{ y, x, z };
		}

		/// <summary>
		/// Vector3D{ y, z, x }
		/// </summary>
		[[nodiscard]] constexpr Vector3D yzx() const noexcept
		{
			return{ y, z, x };
		}

		/// <summary>
		/// Vector3D{ z, x, y }
		/// </summary>
		[[nodiscard]] constexpr Vector3D zxy() const noexcept
		{
			return{ z, x, y };
		}

		/// <summary>
		/// Vector3D{ z, y, x }
		/// </summary>
		[[nodiscard]] constexpr Vector3D zyx() const noexcept
		{
			return{ z, y, x };
		}

		/// <summary>
		/// Vector3D{ 0, 0, 0 }
		/// </summary>
		[[nodiscard]] static constexpr Vector3D Zero() noexcept
		{
			return{ 0, 0, 0 };
		}

		/// <summary>
		/// Vector3D{ 1, 1, 1 }
		/// </summary>
		[[nodiscard]] static constexpr Vector3D One() noexcept
		{
			return{ 1, 1, 1 };
		}

		/// <summary>
		/// Vector3D{ value, value, value }
		/// </summary>
		[[nodiscard]] static constexpr Vector3D All(value_type value = 1) noexcept
		{
			return{ value, value, value };
		}

		/// <summary>
		/// Vector3D{ 1, 0, 0 }
		/// </summary>
		[[nodiscard]] static constexpr Vector3D UnitX() noexcept
		{
			return{ 1, 0, 0 };
		}

		/// <summary>
		/// Vector3D{ 0, 1, 0 }
		/// </summary>
		[[nodiscard]] static constexpr Vector3D UnitY() noexcept
		{
			return{ 0, 1, 0 };
		}

		/// <summary>
		/// Vector3D{ 0, 0, 1 }
		/// </summary>
		[[nodiscard]] static constexpr Vector3D UnitZ() noexcept
		{
			return{ 0, 0, 1 };
		}

		/// <summary>
		/// Vector3D{ -length, 0, 0 }
		/// </summary>
		[[nodiscard]] static constexpr Vector3D Left(value_type length = 1) noexcept
		{
			return{ -length, 0, 0 };
		}

		/// <summary>
		/// Vector3D{ length, 0, 0 }
		/// </summary>
		[[nodiscard]] static constexpr Vector3D Right(value_type length = 1) noexcept
		{
			return{ length, 0, 0 };
		}

		/// <summary>
		/// Vector3D{ 0, length, 0 }
		/// </summary>
		[[nodiscard]] static constexpr Vector3D Up(value_type length = 1) noexcept
		{
			return{ 0, length, 0 };
		}

		/// <summary>
		/// Vector3D{ 0, -length, 0 }
		/// </summary>
		[[nodiscard]] static constexpr Vector3D Down(value_type length = 1) noexcept
		{
			return{ 0, -length, 0 };
		}

		/// <summary>
		/// Vector3D{ 0, 0, length }
		/// </summary>
		[[nodiscard]] static constexpr Vector3D Forward(value_type length = 1) noexcept
		{
			return{ 0, 0, length };
		}

		/// <summary>
		/// Vector3D{ 0, 0, -length }
		/// </summary>
		[[nodiscard]] static constexpr Vector3D Backward(value_type length = 1) noexcept
		{
			return{ 0, 0, -length };
		}
	};

	template <class Type, class U, std::enable_if_t<std::is_scalar_v<U>>* = nullptr>
	[[nodiscard]] inline constexpr Vector3D<Type> operator *(U s, const Vector3D<Type>& v) noexcept
	{
		return v * static_cast<Type>(s);
	}

	using Float3	= Vector3D<float>;
	using Vec3		= Vector3D<double>;
}

//////////////////////////////////////////////////
//
//	Format
//
//////////////////////////////////////////////////

namespace s3d
{
	void Formatter(FormatData& formatData, const Float3& value);

	void Formatter(FormatData& formatData, const Vec3& value);

	template <class CharType, class Type>
	inline std::basic_ostream<CharType>& operator <<(std::basic_ostream<CharType>& output, const Vector3D<Type>& value)
	{
		return output << CharType('(')
			<< value.x << CharType(',') << CharType(' ')
			<< value.y << CharType(',') << CharType(' ')
			<< value.z << CharType(')');
	}

	template <class CharType, class Type>
	inline std::basic_istream<CharType>& operator >> (std::basic_istream<CharType>& input, Vector3D<Type>& value)
	{
		CharType unused;
		return input >> unused
			>> value.x >> unused
			>> value.y >> unused
			>> value.z >> unused;
	}
}

//////////////////////////////////////////////////
//
//	Hash
//
//////////////////////////////////////////////////

namespace std
{
	template <class Type>
	struct hash<s3d::Vector3D<Type>>
	{
		[[nodiscard]] size_t operator()(const s3d::Vector3D<Type>& value) const noexcept
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
	template <class Type>
	struct formatter<s3d::Vector3D<Type>, s3d::char32>
	{
		s3d::String tag;

		template <class ParseContext>
		auto parse(ParseContext& ctx)
		{
			return s3d::detail::GetFmtTag(tag, ctx);
		}

		template <class Context>
		auto format(const s3d::Vector3D<Type>& value, Context& ctx)
		{
			const s3d::String fmt = s3d::detail::MakeFmtArg(
				U"({:", tag, U"}, {:", tag, U"}, {:", tag, U"})"
			);

			return format_to(ctx.begin(), wstring_view(fmt.data(), fmt.size()), value.x, value.y, value.z);
		}
	};
}
