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
# include <memory>
# include "Fwd.hpp"

namespace s3d
{
	class TCPClient
	{
	private:

		class TCPClientDetail;

		std::shared_ptr<TCPClientDetail> pImpl;

	public:

		/// <summary>
		/// デフォルトコンストラクタ
		/// </summary>
		TCPClient();

		/// <summary>
		/// デストラクタ
		/// </summary>
		~TCPClient();

		bool connect(const IPv4& ip, uint16 port);

		void cancelConnect();

		[[nodiscard]] bool isConnected() const;

		void disconnect();

		[[nodiscard]] uint16 port() const;

		[[nodiscard]] bool hasError() const;

		[[nodiscard]] NetworkError getError() const;

		[[nodiscard]] size_t available() const;

		bool skip(size_t size);

		bool lookahead(void* dst, size_t size) const;

		template <class Type, std::enable_if_t<std::is_trivially_copyable_v<Type>>* = nullptr>
		bool lookahead(Type& to)
		{
			return lookahead(std::addressof(to), sizeof(Type));
		}

		bool read(void* dst, size_t size);

		template <class Type, std::enable_if_t<std::is_trivially_copyable_v<Type>>* = nullptr>
		bool read(Type& to)
		{
			return read(std::addressof(to), sizeof(Type));
		}

		bool send(const void* data, size_t size);

		template <class Type, std::enable_if_t<std::is_trivially_copyable_v<Type>>* = nullptr>
		bool send(const Type& to)
		{
			return send(std::addressof(to), sizeof(Type));
		}
	};
}
