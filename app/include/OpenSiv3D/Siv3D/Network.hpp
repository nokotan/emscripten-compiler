//-----------------------------------------------
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
# include "Optional.hpp"
# include "IPv4.hpp"
# include "Array.hpp"

namespace s3d
{
	enum class NetworkError
	{
		OK,

		EoF,

		NoBufferSpaceAvailable,

		ConnectionRefused,

		Error,
	};

	namespace Network
	{
		[[nodiscard]] Optional<IPv4> GetPrivateIPv4();

		[[nodiscard]] Array<IPv4> GetPrivateIPv4s();
	}
}
