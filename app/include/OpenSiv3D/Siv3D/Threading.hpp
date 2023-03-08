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

namespace s3d
{
	namespace Threading
	{
		/// <summary>
		/// サポートされるスレッド数を取得する
		/// Returns the number of concurrent threads supported by the implementation.
		/// </summary>
		/// <returns>
		/// サポートされるスレッド数
		/// Number of concurrent threads supported.
		/// </returns>
		[[nodiscard]] size_t GetConcurrency() noexcept;
	}
}
