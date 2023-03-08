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
# include <type_traits>
# include <stdexcept>
# include <memory>
# include <cassert>

//////////////////////////////////////////////////////////////////////////////
//
//	Copyright (C) 2011 - 2017 Andrzej Krzemienski.
//
//	Use, modification, and distribution is subject to the Boost Software
//	License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
//	http://www.boost.org/LICENSE_1_0.txt)
//

namespace s3d
{
	template <class Type> class Optional;

	template <class Type> class Optional<Type&>;


	inline constexpr struct TrivialInit_t {} TrivialInit{};

	inline constexpr struct InPlace_t {} InPlace{};

	struct None_t
	{
		struct init {};

		explicit constexpr None_t(init) {}
	};

	/// <summary>
	/// 無効値
	/// </summary>
	inline constexpr None_t none{ None_t::init() };

	class BadOptionalAccess : public std::exception
	{
	public:

		[[nodiscard]] const char* what() const noexcept
		{
			return ("Bad optional access");
		}
	};

	namespace detail
	{
		template <class Type>
		struct has_overloaded_addressof
		{
			template <class U>
			static constexpr bool has_overload(...) { return false; }

			template <class U, size_t S = sizeof(std::declval<U&>().operator&())>
			static constexpr bool has_overload(bool) { return true; }

			constexpr static bool value = has_overload<Type>(true);
		};

		template <class Type, std::enable_if_t<!has_overloaded_addressof<Type>::value>* = nullptr>
		constexpr Type* static_addressof(Type& ref)
		{
			return &ref;
		}

		template <class Type, std::enable_if_t<has_overloaded_addressof<Type>::value>* = nullptr>
		Type* static_addressof(Type& ref)
		{
			return std::addressof(ref);
		}

		template <class U>
		constexpr U convert(U v)
		{
			return v;
		}

		namespace swap_ns
		{
			using std::swap;

			template <class T>
			void adl_swap(T& t, T& u) noexcept(noexcept(swap(t, u)))
			{
				swap(t, u);
			}

		} // namespace swap_ns

		template <class Type>
		union Storage_t
		{
			unsigned char dummy_;

			Type value_;

			constexpr Storage_t(TrivialInit_t) noexcept
				: dummy_() {};

			template <class... Args>
			constexpr Storage_t(Args&&... args)
				: value_(std::forward<Args>(args)...) {}

			~Storage_t() {}
		};

		template <class Type>
		union Constexpr_storage_t
		{
			unsigned char dummy_;

			Type value_;

			constexpr Constexpr_storage_t(TrivialInit_t) noexcept
				: dummy_() {};

			template <class... Args>
			constexpr Constexpr_storage_t(Args&&... args)
				: value_(std::forward<Args>(args)...) {}

			~Constexpr_storage_t() = default;
		};

		template <class Type>
		struct optional_base
		{
			bool init_;

			Storage_t<Type> storage_;

			constexpr optional_base() noexcept
				: init_(false)
				, storage_(TrivialInit) {};

			explicit constexpr optional_base(const Type& v)
				: init_(true)
				, storage_(v) {}

			explicit constexpr optional_base(Type&& v)
				: init_(true)
				, storage_(std::move(v)) {}

			template <class... Args> explicit optional_base(InPlace_t, Args&&... args)
				: init_(true)
				, storage_(std::forward<Args>(args)...) {}

			template <class U, class... Args, std::enable_if_t<std::is_constructible_v<Type, std::initializer_list<U>>>* = nullptr>
			explicit optional_base(InPlace_t, std::initializer_list<U> il, Args&&... args)
				: init_(true)
				, storage_(il, std::forward<Args>(args)...) {}

			~optional_base()
			{
				if (init_)
				{
					storage_.value_.~Type();
				}
			}
		};

		template <class Type>
		struct constexpr_optional_base
		{
			bool init_;

			Constexpr_storage_t<Type> storage_;

			constexpr constexpr_optional_base() noexcept
				: init_(false)
				, storage_(TrivialInit) {};

			explicit constexpr constexpr_optional_base(const Type& v)
				: init_(true)
				, storage_(v) {}

			explicit constexpr constexpr_optional_base(Type&& v)
				: init_(true)
				, storage_(std::move(v)) {}

			template <class... Args> explicit constexpr constexpr_optional_base(InPlace_t, Args&&... args)
				: init_(true)
				, storage_(std::forward<Args>(args)...) {}

			template <class U, class... Args, std::enable_if_t<std::is_constructible_v<Type, std::initializer_list<U>>>* = nullptr>
			explicit constexpr constexpr_optional_base(InPlace_t, std::initializer_list<U> il, Args&&... args)
				: init_(true)
				, storage_(il, std::forward<Args>(args)...) {}

			~constexpr_optional_base() = default;
		};
	}

	template <class Type>
	using OptionalBase = std::conditional_t<std::is_trivially_destructible_v<Type>, detail::constexpr_optional_base<typename std::remove_const_t<Type>>, detail::optional_base<typename std::remove_const_t<Type>>>;

	template <class Type>
	struct IsOptional : std::false_type {};

	template <class Type>
	struct IsOptional<Optional<Type>> : std::true_type {};

	template <>
	struct IsOptional<None_t> : std::true_type {};

	template <class Type> constexpr bool IsOptional_v = IsOptional<Type>::value;

	/// <summary>
	/// Optional
	/// </summary>
	template <class Type>
	class Optional : private OptionalBase<Type>
	{
		static_assert(!std::is_same_v<std::decay_t<Type>, None_t>, "bad Type");
		static_assert(!std::is_same_v<std::decay_t<Type>, InPlace_t>, "bad Type");

		Type* dataptr()
		{
			return std::addressof(OptionalBase<Type>::storage_.value_);
		}
		
		constexpr const Type* dataptr() const
		{
			return detail::static_addressof(OptionalBase<Type>::storage_.value_);
		}

		constexpr const Type& contained_val() const&
		{
			return OptionalBase<Type>::storage_.value_;
		}
		
		Type& contained_val() &
		{
			return OptionalBase<Type>::storage_.value_;
		}
		
		Type&& contained_val() &&
		{
			return std::move(OptionalBase<Type>::storage_.value_);
		}

		void clear() noexcept
		{
			if (has_value())
			{
				dataptr()->~Type();
			}

			OptionalBase<Type>::init_ = false;
		}

		template <class... Args>
		void initialize(Args&&... args) noexcept(noexcept(Type(std::forward<Args>(args)...)))
		{
			assert(!OptionalBase<Type>::init_);
			::new (static_cast<void*>(dataptr())) Type(std::forward<Args>(args)...);
			OptionalBase<Type>::init_ = true;
		}

		template <class U, class... Args>
		void initialize(std::initializer_list<U> il, Args&&... args) noexcept(noexcept(Type(il, std::forward<Args>(args)...)))
		{
			assert(!OptionalBase<Type>::init_);
			::new (static_cast<void*>(dataptr())) Type(il, std::forward<Args>(args)...);
			OptionalBase<Type>::init_ = true;
		}

	public:

		using value_type = Type;

		/// <summary>
		/// デフォルトコンストラクタ
		/// </summary>
		constexpr Optional() noexcept
			: OptionalBase<Type>() {};
		
		/// <summary>
		/// コンストラクタ
		/// </summary>
		/// <param>
		/// 無効値
		/// </param>
		constexpr Optional(None_t) noexcept
			: OptionalBase<Type>() {};

		/// <summary>
		/// コピーコンストラクタ
		/// </summary>
		/// <param name="rhs">
		/// 他の Optional オブジェクト
		/// </param>
		Optional(const Optional& rhs)
			: OptionalBase<Type>()
		{
			if (rhs.has_value()) {
				::new (static_cast<void*>(dataptr())) Type(*rhs);
				OptionalBase<Type>::init_ = true;
			}
		}

		/// <summary>
		/// ムーブコンストラクタ
		/// </summary>
		/// <param name="rhs">
		/// 他の Optional オブジェクト
		/// </param>
		Optional(Optional&& rhs) noexcept(std::is_nothrow_move_constructible_v<Type>)
			: OptionalBase<Type>()
		{
			if (rhs.has_value())
			{
				::new (static_cast<void*>(dataptr())) Type(std::move(*rhs));
				OptionalBase<Type>::init_ = true;
			}
		}

		/// <summary>
		/// コピーコンストラクタ
		/// </summary>
		/// <param name="rhs">
		/// 他の Optional オブジェクト
		/// </param>
		constexpr Optional(const value_type& v)
			: OptionalBase<Type>(v) {}

		/// <summary>
		/// ムーブコンストラクタ
		/// </summary>
		/// <param name="rhs">
		/// 他の Optional オブジェクト
		/// </param>
		constexpr Optional(value_type&& v)
			: OptionalBase<Type>(std::move(v)) {}

		/// <summary>
		/// コンストラクタ
		/// </summary>
		/// <param name="args">
		/// 値のコンストラクタ引数
		/// </param>
		template <class... Args>
		explicit constexpr Optional(InPlace_t, Args&&... args)
			: OptionalBase<Type>(InPlace_t{}, std::forward<Args>(args)...) {}

		/// <summary>
		/// コンストラクタ
		/// </summary>
		/// <param name="args">
		/// 値のコンストラクタ引数
		/// </param>
		template <class U, class... Args, std::enable_if_t<std::is_constructible_v<Type, std::initializer_list<U>>>* = nullptr>
		explicit constexpr Optional(InPlace_t, std::initializer_list<U> il, Args&&... args)
			: OptionalBase<Type>(InPlace_t{}, il, std::forward<Args>(args)...) {}

		/// <summary>
		/// デストラクタ
		/// </summary>
		~Optional() = default;

		/// <summary>
		/// 代入演算子
		/// </summary>
		/// <returns>
		/// *this
		/// </returns>
		Optional& operator = (None_t) noexcept
		{
			clear();
			return *this;
		}

		/// <summary>
		/// 代入演算子
		/// </summary>
		/// <param name="rhs">
		/// 他の Optional オブジェクト
		/// </param>
		/// <returns>
		/// *this
		/// </returns>
		Optional& operator = (const Optional& rhs)
		{
			if (has_value() == true && rhs.has_value() == false) clear();
			else if (has_value() == false && rhs.has_value() == true) initialize(*rhs);
			else if (has_value() == true && rhs.has_value() == true) contained_val() = *rhs;
			return *this;
		}

		/// <summary>
		/// ムーブ代入演算子
		/// </summary>
		/// <param name="rhs">
		/// 他の Optional オブジェクト
		/// </param>
		/// <returns>
		/// *this
		/// </returns>
		Optional& operator = (Optional&& rhs)
			noexcept(std::is_nothrow_move_assignable_v<Type> && std::is_nothrow_move_constructible_v<Type>)
		{
			if (has_value() == true && rhs.has_value() == false) clear();
			else if (has_value() == false && rhs.has_value() == true) initialize(std::move(*rhs));
			else if (has_value() == true && rhs.has_value() == true) contained_val() = std::move(*rhs);
			return *this;
		}

		/// <summary>
		/// ムーブ代入演算子
		/// </summary>
		/// <param name="v">
		/// 他のオブジェクト
		/// </param>
		/// <returns>
		/// *this
		/// </returns>
		template <class U>
		auto operator = (U&& v)
			-> typename std::enable_if_t<std::is_same_v<std::decay_t<U>, Type>, Optional&>
		{
			if (has_value()) { contained_val() = std::forward<U>(v); }
			else { initialize(std::forward<U>(v)); }
			return *this;
		}

		/// <summary>
		/// Optional オブジェクトを初期化します。
		/// </summary>
		/// <param name="args">
		/// 値のコンストラクタ引数
		/// </param>
		/// <returns>
		/// なし
		/// </returns>
		template <class... Args>
		void emplace(Args&&... args)
		{
			clear();
			initialize(std::forward<Args>(args)...);
		}

		/// <summary>
		/// Optional オブジェクトを初期化します。
		/// </summary>
		/// <param name="il">
		/// 値のコンストラクタ引数
		/// </param>
		/// <param name="args">
		/// 値のコンストラクタ引数
		/// </param>
		/// <returns>
		/// なし
		/// </returns>
		template <class U, class... Args>
		void emplace(std::initializer_list<U> il, Args&&... args)
		{
			clear();
			initialize<U, Args...>(il, std::forward<Args>(args)...);
		}

		/// <summary>
		/// 別の Optional オブジェクトと中身を入れ替えます。
		/// </summary>
		/// <param name="another">
		/// 別の Optional オブジェクト
		/// </param>
		/// <returns>
		/// なし
		/// </returns>
		void swap(Optional<Type>& rhs) noexcept(std::is_nothrow_move_constructible_v<Type> && noexcept(detail::swap_ns::adl_swap(std::declval<Type&>(), std::declval<Type&>())))
		{
			if (has_value() == true && rhs.has_value() == false) { rhs.initialize(std::move(**this)); clear(); }
			else if (has_value() == false && rhs.has_value() == true) { initialize(std::move(*rhs)); rhs.clear(); }
			else if (has_value() == true && rhs.has_value() == true) { using std::swap; swap(**this, *rhs); }
		}

		/// <summary>
		/// 中身があるかを返します。
		/// </summary>
		/// <returns>
		/// 中身が none ではない場合 true, それ以外の場合は false
		/// </returns>
		constexpr explicit operator bool() const noexcept
		{
			return has_value();
		}

		/// <summary>
		/// 中身があるかを返します。
		/// </summary>
		/// <returns>
		/// 中身が none ではない場合 true, それ以外の場合は false
		/// </returns>
		constexpr bool has_value() const noexcept
		{
			return OptionalBase<Type>::init_;
		}

		/// <summary>
		/// 中身へのポインタを返します。
		/// </summary>
		/// <remarks>
		/// 中身が none の場合の動作は未定義です。
		/// </remarks>
		/// <returns>
		/// 中身へのポインタ
		/// </returns>
		constexpr value_type const* operator -> () const
		{
			assert(has_value());
			return dataptr();
		}

		/// <summary>
		/// 中身へのポインタを返します。
		/// </summary>
		/// <remarks>
		/// 中身が none の場合の動作は未定義です。
		/// </remarks>
		/// <returns>
		/// 中身へのポインタ
		/// </returns>
		value_type* operator -> ()
		{
			assert(has_value());
			return dataptr();
		}

		/// <summary>
		/// 中身の参照を返します。
		/// </summary>
		/// <remarks>
		/// 中身が none の場合の動作は未定義です。
		/// </remarks>
		/// <returns>
		/// 中身の参照
		/// </returns>
		constexpr value_type const& operator* () const
		{
			assert(has_value());
			return contained_val();
		}

		/// <summary>
		/// 中身の参照を返します。
		/// </summary>
		/// <remarks>
		/// 中身が none の場合の動作は未定義です。
		/// </remarks>
		/// <returns>
		/// 中身の参照
		/// </returns>
		value_type& operator * ()
		{
			assert(has_value());
			return contained_val();
		}

		/// <summary>
		/// 中身の参照を返します。
		/// </summary>
		/// <exception cref="BadOptionalAccess">
		/// 中身が none の場合 throw されます。
		/// </exception>
		/// <returns>
		/// 中身の参照
		/// </returns>
		constexpr value_type const& value() const
		{
			if (!has_value())
			{
				throw BadOptionalAccess{};
			}

			return contained_val();
		}

		/// <summary>
		/// 中身の参照を返します。
		/// </summary>
		/// <exception cref="BadOptionalAccess">
		/// 中身が none の場合 throw されます。
		/// </exception>
		/// <returns>
		/// 中身の参照
		/// </returns>
		value_type& value()
		{
			if (!has_value())
			{
				throw BadOptionalAccess{};
			}

			return contained_val();
		}

		/// <summary>
		/// 中身がある場合はその値を、それ以外の場合は v を返します。
		/// </summary>
		/// <param name="v">
		/// 中身が none の場合に返ってくる値
		/// </param>
		/// <returns>
		/// 中身がある場合はその値、それ以外の場合は v
		/// </returns>
		template <class V>
		constexpr value_type value_or(V&& v) const&
		{
			return *this ? **this : detail::convert<Type>(std::forward<V>(v));
		}

		/// <summary>
		/// 中身がある場合はその値を、それ以外の場合は v を返します。
		/// </summary>
		/// <param name="v">
		/// 中身が none の場合に返ってくる値
		/// </param>
		/// <returns>
		/// 中身がある場合はその値、それ以外の場合は v
		/// </returns>
		template <class V>
		value_type value_or(V&& v) &&
		{
			return *this ? std::move(const_cast<Optional<Type>&>(*this).contained_val()) : detail::convert<Type>(std::forward<V>(v));
		}

		template <class Fty, std::enable_if_t<std::is_invocable_r_v<value_type, Fty>>* = nullptr>
		constexpr value_type value_or_eval(Fty f) const&
		{
			return *this ? **this : f();
		}

		template <class Fty, std::enable_if_t<std::is_invocable_r_v<value_type, Fty>>* = nullptr>
		value_type value_or_eval(Fty f) &&
		{
			return *this ? std::move(const_cast<Optional<Type>&>(*this).contained_val()) : f();
		}

		/// <summary>
		/// 中身を none にします。
		/// </summary>
		/// <returns>
		/// なし
		/// </returns>
		void reset() noexcept
		{
			clear();
		}

		/// <summary>
		/// 中身に別の値を代入します。
		/// </summary>
		/// <param name="v">
		/// コピーする値
		/// </param>
		/// <returns>
		/// なし
		/// </returns>
		void reset(const value_type& v)
		{
			emplace(v);
		}

		/// <summary>
		/// 中身がある場合に、その値を引数に関数 f を呼びます。
		/// </summary>
		/// <param name="f">
		/// 中身の値と同じ型を引数にとる関数
		/// </param>
		/// <returns>
		/// なし
		/// </returns>
		template <class Fty, std::enable_if_t<std::is_invocable_v<Fty, value_type&>>* = nullptr>
		void then(Fty f)
		{
			if (has_value())
			{
				f(value());
			}
		}

		/// <summary>
		/// 中身がある場合に、その値を引数に関数 f を呼びます。
		/// </summary>
		/// <param name="f">
		/// 中身の値と同じ型を引数にとる関数
		/// </param>
		/// <returns>
		/// なし
		/// </returns>
		template <class Fty, std::enable_if_t<std::is_invocable_v<Fty, value_type>>* = nullptr>
		void then(Fty f) const
		{
			if (has_value())
			{
				f(value());
			}
		}

		/// <summary>
		/// 中身がある場合に、その値を引数に関数 f を呼びます。
		/// </summary>
		/// <param name="f">
		/// 中身の値と同じ型を引数にとる関数
		/// </param>
		/// <returns>
		/// 中身がある場合は関数 f の戻り値の Optional, それ以外の場合は none
		/// </returns>
		template <class Fty, class R = std::decay_t<std::invoke_result_t<Fty, value_type&>>>
		Optional<R> map(Fty f)
		{
			if (has_value())
			{
				return f(value());
			}
			else
			{
				return none;
			}
		}

		/// <summary>
		/// 中身がある場合に、その値を引数に関数 f を呼びます。
		/// </summary>
		/// <param name="f">
		/// 中身の値と同じ型を引数にとる関数
		/// </param>
		/// <returns>
		/// 中身がある場合は関数 f の戻り値の Optional, それ以外の場合は none
		/// </returns>
		template <class Fty, class R = std::decay_t<std::invoke_result_t<Fty, value_type>>>
		Optional<R> map(Fty f) const
		{
			if (has_value())
			{
				return f(value());
			}
			else
			{
				return none;
			}
		}
	};

	/// <summary>
	/// Optional
	/// </summary>
	template <class Type>
	class Optional<Type&>
	{
		static_assert(!std::is_same_v<Type, None_t>, "bad Type");
		static_assert(!std::is_same_v<Type, InPlace_t>, "bad Type");
			
		Type* ref;

	public:

		/// <summary>
		/// デフォルトコンストラクタ
		/// </summary>
		constexpr Optional() noexcept
			: ref(nullptr) {}

		/// <summary>
		/// コンストラクタ
		/// </summary>
		/// <param>
		/// 無効値
		/// </param>
		constexpr Optional(None_t) noexcept
			: ref(nullptr) {}

		/// <summary>
		/// コピーコンストラクタ
		/// </summary>
		/// <param name="rhs">
		/// 他の Optional オブジェクト
		/// </param>
		constexpr Optional(Type& v) noexcept
			: ref(detail::static_addressof(v)) {}

		Optional(Type&&) = delete;

		/// <summary>
		/// コピーコンストラクタ
		/// </summary>
		/// <param name="rhs">
		/// 他の Optional オブジェクト
		/// </param>
		constexpr Optional(const Optional& rhs) noexcept
			: ref(rhs.ref) {}

		/// <summary>
		/// コピーコンストラクタ
		/// </summary>
		/// <param name="rhs">
		/// 他の Optional オブジェクト
		/// </param>
		explicit constexpr Optional(InPlace_t, Type& v) noexcept
			: ref(detail::static_addressof(v)) {}

		explicit Optional(InPlace_t, Type&&) = delete;

		/// <summary>
		/// デストラクタ
		/// </summary>
		~Optional() = default;

		/// <summary>
		/// 代入演算子
		/// </summary>
		/// <returns>
		/// *this
		/// </returns>
		Optional& operator = (None_t) noexcept
		{
			ref = nullptr;
			return *this;
		}

		//	Optional& operator = (const Optional& rhs) noexcept
		//	{
		//		ref = rhs.ref;
		//		return *this;
		//	}

		//	Optional& operator = (Optional&& rhs) noexcept
		//	{
		//		ref = rhs.ref;
		//		return *this;
		//	}

		/// <summary>
		/// 代入演算子
		/// </summary>
		/// <param name="rhs">
		/// 他の Optional オブジェクト
		/// </param>
		/// <returns>
		/// *this
		/// </returns>
		template <class U>
		auto operator = (U&& rhs) noexcept
			-> typename std::enable_if_t<std::is_same_v<std::decay_t<U>, Optional<Type&>>, Optional&>
		{
			ref = rhs.ref;
			return *this;
		}

		/// <summary>
		/// ムーブ代入演算子
		/// </summary>
		/// <param name="rhs">
		/// 他の Optional オブジェクト
		/// </param>
		/// <returns>
		/// *this
		/// </returns>
		template <class U>
		auto operator =(U&& rhs) noexcept
			-> typename std::enable_if_t<!std::is_same_v<std::decay_t<U>, Optional<Type&>>, Optional&> = delete;

		/// <summary>
		/// Optional オブジェクトを初期化します。
		/// </summary>
		/// <param name="v">
		/// 新しい値
		/// </param>
		/// <returns>
		/// なし
		/// </returns>
		void emplace(Type& v) noexcept
		{
			ref = detail::static_addressof(v);
		}

		void emplace(Type&&) = delete;

		/// <summary>
		/// 別の Optional オブジェクトと中身を入れ替えます。
		/// </summary>
		/// <param name="another">
		/// 別の Optional オブジェクト
		/// </param>
		/// <returns>
		/// なし
		/// </returns>
		void swap(Optional<Type&>& rhs) noexcept
		{
			std::swap(ref, rhs.ref);
		}

		/// <summary>
		/// 中身があるかを返します。
		/// </summary>
		/// <returns>
		/// 中身が none ではない場合 true, それ以外の場合は false
		/// </returns>
		explicit constexpr operator bool() const noexcept
		{
			return has_value();
		}

		/// <summary>
		/// 中身があるかを返します。
		/// </summary>
		/// <returns>
		/// 中身が none ではない場合 true, それ以外の場合は false
		/// </returns>
		constexpr bool has_value() const noexcept
		{
			return ref != nullptr;
		}

		/// <summary>
		/// 中身へのポインタを返します。
		/// </summary>
		/// <remarks>
		/// 中身が none の場合の動作は未定義です。
		/// </remarks>
		/// <returns>
		/// 中身へのポインタ
		/// </returns>
		constexpr Type* operator -> () const
		{
			assert(ref);
			return ref;
		}

		/// <summary>
		/// 中身の参照を返します。
		/// </summary>
		/// <remarks>
		/// 中身が none の場合の動作は未定義です。
		/// </remarks>
		/// <returns>
		/// 中身の参照
		/// </returns>
		constexpr Type& operator * () const
		{
			assert(ref);
			return *ref;
		}

		/// <summary>
		/// 中身の参照を返します。
		/// </summary>
		/// <exception cref="BadOptionalAccess">
		/// 中身が none の場合 throw されます。
		/// </exception>
		/// <returns>
		/// 中身の参照
		/// </returns>
		constexpr Type& value() const
		{
			if (!ref)
			{
				throw BadOptionalAccess{};
			}

			return *ref;
		}

		/// <summary>
		/// 中身がある場合はその値を、それ以外の場合は v を返します。
		/// </summary>
		/// <param name="v">
		/// 中身が none の場合に返ってくる値
		/// </param>
		/// <returns>
		/// 中身がある場合はその値、それ以外の場合は v
		/// </returns>
		template <class V>
		constexpr std::decay_t<Type> value_or(V&& v) const
		{
			return *this ? **this : detail::convert<std::decay_t<Type>>(std::forward<V>(v));
		}

		template <class Fty, std::enable_if_t<std::is_invocable_r_v<std::decay_t<Type>, Fty>>* = nullptr>
		constexpr std::decay_t<Type> value_or_eval(Fty f) const
		{
			return *this ? **this : f();
		}

		/// <summary>
		/// 中身を none にします。
		/// </summary>
		/// <returns>
		/// なし
		/// </returns>
		void reset() noexcept
		{
			ref = nullptr;
		}

		/// <summary>
		/// 中身に別の値を代入します。
		/// </summary>
		/// <param name="v">
		/// コピーする値
		/// </param>
		/// <returns>
		/// なし
		/// </returns>
		void reset(Type& v) noexcept
		{
			emplace(v);
		}

		/// <summary>
		/// 中身がある場合に、その値を引数に関数 f を呼びます。
		/// </summary>
		/// <param name="f">
		/// 中身の値と同じ型を引数にとる関数
		/// </param>
		/// <returns>
		/// なし
		/// </returns>
		template <class Fty, std::enable_if_t<std::is_invocable_v<Fty, Type&>>* = nullptr>
		void then(Fty f)
		{
			if (has_value())
			{
				f(value());
			}
		}

		/// <summary>
		/// 中身がある場合に、その値を引数に関数 f を呼びます。
		/// </summary>
		/// <param name="f">
		/// 中身の値と同じ型を引数にとる関数
		/// </param>
		/// <returns>
		/// なし
		/// </returns>
		template <class Fty, std::enable_if_t<std::is_invocable_v<Fty, const Type&>>* = nullptr>
		void then(Fty f) const
		{
			if (has_value())
			{
				f(value());
			}
		}

		/// <summary>
		/// 中身がある場合に、その値を引数に関数 f を呼びます。
		/// </summary>
		/// <param name="f">
		/// 中身の値と同じ型を引数にとる関数
		/// </param>
		/// <returns>
		/// 中身がある場合は関数 f の戻り値の Optional, それ以外の場合は none
		/// </returns>
		template <class Fty, class R = std::invoke_result_t<Fty, Type&>>
		Optional<R> map(Fty f)
		{
			if (has_value())
			{
				return f(value());
			}
			else
			{
				return none;
			}
		}

		/// <summary>
		/// 中身がある場合に、その値を引数に関数 f を呼びます。
		/// </summary>
		/// <param name="f">
		/// 中身の値と同じ型を引数にとる関数
		/// </param>
		/// <returns>
		/// 中身がある場合は関数 f の戻り値の Optional, それ以外の場合は none
		/// </returns>
		template <class Fty, class R = std::invoke_result_t<Fty, const Type&>>
		Optional<R> map(Fty f) const
		{
			if (has_value())
			{
				return f(value());
			}
			else
			{
				return none;
			}
		}
	};

	template <class Type>
	class Optional<Type&&>
	{
		static_assert(sizeof(Type) == 0, "Optional rvalue referencs disallowed");
	};

	template <class Type>
	constexpr bool operator ==(const Optional<Type>& x, const Optional<Type>& y)
	{
		return static_cast<bool>(x) != static_cast<bool>(y) ? false : static_cast<bool>(x) == false ? true : *x == *y;
	}

	template <class Type>
	constexpr bool operator !=(const Optional<Type>& x, const Optional<Type>& y)
	{
		return !(x == y);
	}

	template <class Type>
	constexpr bool operator <(const Optional<Type>& x, const Optional<Type>& y)
	{
		return (!y) ? false : (!x) ? true : *x < *y;
	}

	template <class Type>
	constexpr bool operator >(const Optional<Type>& x, const Optional<Type>& y)
	{
		return (y < x);
	}

	template <class Type>
	constexpr bool operator <=(const Optional<Type>& x, const Optional<Type>& y)
	{
		return !(y < x);
	}

	template <class Type>
	constexpr bool operator >=(const Optional<Type>& x, const Optional<Type>& y)
	{
		return !(x < y);
	}

	template <class Type>
	constexpr bool operator ==(const Optional<Type>& x, None_t) noexcept
	{
		return (!x);
	}

	template <class Type>
	constexpr bool operator ==(None_t, const Optional<Type>& x) noexcept
	{
		return (!x);
	}

	template <class Type>
	constexpr bool operator !=(const Optional<Type>& x, None_t) noexcept
	{
		return static_cast<bool>(x);
	}

	template <class Type>
	constexpr bool operator !=(None_t, const Optional<Type>& x) noexcept
	{
		return static_cast<bool>(x);
	}

	template <class Type>
	constexpr bool operator <(const Optional<Type>&, None_t) noexcept
	{
		return false;
	}

	template <class Type>
	constexpr bool operator <(None_t, const Optional<Type>& x) noexcept
	{
		return static_cast<bool>(x);
	}

	template <class Type>
	constexpr bool operator <=(const Optional<Type>& x, None_t) noexcept
	{
		return (!x);
	}

	template <class Type>
	constexpr bool operator <=(None_t, const Optional<Type>&) noexcept
	{
		return true;
	}

	template <class Type>
	constexpr bool operator >(const Optional<Type>& x, None_t) noexcept
	{
		return static_cast<bool>(x);
	}

	template <class Type>
	constexpr bool operator >(None_t, const Optional<Type>&) noexcept
	{
		return false;
	}

	template <class Type>
	constexpr bool operator >=(const Optional<Type>&, None_t) noexcept
	{
		return true;
	}

	template <class Type>
	constexpr bool operator >=(None_t, const Optional<Type>& x) noexcept
	{
		return (!x);
	}

	template <class Type>
	constexpr bool operator ==(const Optional<Type>& x, const Type& v)
	{
		return static_cast<bool>(x) ? *x == v : false;
	}

	template <class Type>
	constexpr bool operator ==(const Type& v, const Optional<Type>& x)
	{
		return static_cast<bool>(x) ? v == *x : false;
	}

	template <class Type>
	constexpr bool operator !=(const Optional<Type>& x, const Type& v)
	{
		return static_cast<bool>(x) ? *x != v : true;
	}

	template <class Type>
	constexpr bool operator !=(const Type& v, const Optional<Type>& x)
	{
		return static_cast<bool>(x) ? v != *x : true;
	}

	template <class Type>
	constexpr bool operator <(const Optional<Type>& x, const Type& v)
	{
		return static_cast<bool>(x) ? *x < v : true;
	}

	template <class Type>
	constexpr bool operator >(const Type& v, const Optional<Type>& x)
	{
		return static_cast<bool>(x) ? v > *x : true;
	}

	template <class Type>
	constexpr bool operator >(const Optional<Type>& x, const Type& v)
	{
		return static_cast<bool>(x) ? *x > v : false;
	}

	template <class Type>
	constexpr bool operator <(const Type& v, const Optional<Type>& x)
	{
		return static_cast<bool>(x) ? v < *x : false;
	}

	template <class Type>
	constexpr bool operator >=(const Optional<Type>& x, const Type& v)
	{
		return static_cast<bool>(x) ? *x >= v : false;
	}

	template <class Type>
	constexpr bool operator <=(const Type& v, const Optional<Type>& x)
	{
		return static_cast<bool>(x) ? v <= *x : false;
	}

	template <class Type>
	constexpr bool operator <=(const Optional<Type>& x, const Type& v)
	{
		return static_cast<bool>(x) ? *x <= v : true;
	}

	template <class Type>
	constexpr bool operator >=(const Type& v, const Optional<Type>& x)
	{
		return static_cast<bool>(x) ? v >= *x : true;
	}

	template <class Type>
	constexpr bool operator ==(const Optional<Type&>& x, const Type& v)
	{
		return static_cast<bool>(x) ? *x == v : false;
	}

	template <class Type>
	constexpr bool operator ==(const Type& v, const Optional<Type&>& x)
	{
		return static_cast<bool>(x) ? v == *x : false;
	}

	template <class Type>
	constexpr bool operator !=(const Optional<Type&>& x, const Type& v)
	{
		return static_cast<bool>(x) ? *x != v : true;
	}

	template <class Type>
	constexpr bool operator !=(const Type& v, const Optional<Type&>& x)
	{
		return static_cast<bool>(x) ? v != *x : true;
	}

	template <class Type>
	constexpr bool operator <(const Optional<Type&>& x, const Type& v)
	{
		return static_cast<bool>(x) ? *x < v : true;
	}

	template <class Type>
	constexpr bool operator >(const Type& v, const Optional<Type&>& x)
	{
		return static_cast<bool>(x) ? v > *x : true;
	}

	template <class Type>
	constexpr bool operator >(const Optional<Type&>& x, const Type& v)
	{
		return static_cast<bool>(x) ? *x > v : false;
	}

	template <class Type>
	constexpr bool operator <(const Type& v, const Optional<Type&>& x)
	{
		return static_cast<bool>(x) ? v < *x : false;
	}

	template <class Type>
	constexpr bool operator >=(const Optional<Type&>& x, const Type& v)
	{
		return static_cast<bool>(x) ? *x >= v : false;
	}

	template <class Type>
	constexpr bool operator <=(const Type& v, const Optional<Type&>& x)
	{
		return static_cast<bool>(x) ? v <= *x : false;
	}

	template <class Type>
	constexpr bool operator <=(const Optional<Type&>& x, const Type& v)
	{
		return static_cast<bool>(x) ? *x <= v : true;
	}

	template <class Type>
	constexpr bool operator >=(const Type& v, const Optional<Type&>& x)
	{
		return static_cast<bool>(x) ? v >= *x : true;
	}

	template <class Type>
	constexpr bool operator ==(const Optional<const Type&>& x, const Type& v)
	{
		return static_cast<bool>(x) ? *x == v : false;
	}

	template <class Type>
	constexpr bool operator ==(const Type& v, const Optional<const Type&>& x)
	{
		return static_cast<bool>(x) ? v == *x : false;
	}

	template <class Type>
	constexpr bool operator !=(const Optional<const Type&>& x, const Type& v)
	{
		return static_cast<bool>(x) ? *x != v : true;
	}

	template <class Type>
	constexpr bool operator !=(const Type& v, const Optional<const Type&>& x)
	{
		return static_cast<bool>(x) ? v != *x : true;
	}

	template <class Type>
	constexpr bool operator <(const Optional<const Type&>& x, const Type& v)
	{
		return static_cast<bool>(x) ? *x < v : true;
	}

	template <class Type>
	constexpr bool operator >(const Type& v, const Optional<const Type&>& x)
	{
		return static_cast<bool>(x) ? v > *x : true;
	}

	template <class Type>
	constexpr bool operator >(const Optional<const Type&>& x, const Type& v)
	{
		return static_cast<bool>(x) ? *x > v : false;
	}

	template <class Type>
	constexpr bool operator <(const Type& v, const Optional<const Type&>& x)
	{
		return static_cast<bool>(x) ? v < *x : false;
	}

	template <class Type>
	constexpr bool operator >=(const Optional<const Type&>& x, const Type& v)
	{
		return static_cast<bool>(x) ? *x >= v : false;
	}

	template <class Type>
	constexpr bool operator <=(const Type& v, const Optional<const Type&>& x)
	{
		return static_cast<bool>(x) ? v <= *x : false;
	}

	template <class Type>
	constexpr bool operator <=(const Optional<const Type&>& x, const Type& v)
	{
		return static_cast<bool>(x) ? *x <= v : true;
	}

	template <class Type>
	constexpr bool operator >=(const Type& v, const Optional<const Type&>& x)
	{
		return static_cast<bool>(x) ? v >= *x : true;
	}

	template <class Type>
	constexpr Optional<std::decay_t<Type>> MakeOptional(Type&& v)
	{
		return Optional<std::decay_t<Type>>(std::forward<Type>(v));
	}

	template <class U>
	constexpr Optional<U&> MakeOptional(std::reference_wrapper<U> v)
	{
		return Optional<U&>(v.get());
	}
}

//////////////////////////////////////////////////
//
//	Format
//
//////////////////////////////////////////////////

namespace s3d
{
	template <class CharType>
	inline std::basic_ostream<CharType>& operator <<(std::basic_ostream<CharType>& output, const None_t&)
	{
		const CharType no[] = { 'n','o','n','e','\0' };

		return output << no;
	}

	template <class CharType, class Type>
	inline std::basic_ostream<CharType>& operator <<(std::basic_ostream<CharType>& output, const Optional<Type>& value)
	{
		if (value)
		{
			const CharType opt[] = { 'O','p','t','i','o','n','a','l', ' ', '\0' };

			return output << opt << value.value();
		}
		else
		{
			const CharType no[] = { 'n','o','n','e','\0' };

			return output << no;
		}
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
	struct hash<s3d::Optional<Type>>
	{
		using result_type = typename hash<Type>::result_type;
		using argument_type = s3d::Optional<Type>;

		constexpr result_type operator()(argument_type const& arg) const noexcept
		{
			return arg ? std::hash<Type>{}(*arg) : result_type{};
		}
	};

	template <class Type>
	struct hash<s3d::Optional<Type&>>
	{
		using result_type = typename hash<Type>::result_type;
		using argument_type = s3d::Optional<Type&>;

		constexpr result_type operator()(argument_type const& arg) const noexcept
		{
			return arg ? std::hash<Type>{}(*arg) : result_type{};
		}
	};
}

//////////////////////////////////////////////////
//
//	Swap
//
//////////////////////////////////////////////////

namespace std
{
	template <class Type>
	void swap(s3d::Optional<Type>& a, s3d::Optional<Type>& b) noexcept(noexcept(a.swap(b)))
	{
		a.swap(b);
	}
}

//
//////////////////////////////////////////////////////////////////////////////
