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
	namespace Compression
	{
		constexpr int32 DefaultCompressionLevel = 11;

		constexpr int32 MinCompressionLevel = 1;

		constexpr int32 MaxCompressionLevel = 22;

		[[nodiscard]] ByteArray Compress(ByteArrayViewAdapter view, int32 compressionLevel = DefaultCompressionLevel);

		[[nodiscard]] ByteArray CompressFile(FilePathView path, int32 compressionLevel = DefaultCompressionLevel);

		bool CompressToFile(ByteArrayViewAdapter view, FilePathView outputPath, int32 compressionLevel = DefaultCompressionLevel);

		bool CompressFileToFile(FilePathView inputPath, FilePathView outputPath, int32 compressionLevel = DefaultCompressionLevel);

		[[nodiscard]] ByteArray Decompress(ByteArrayView view);

		[[nodiscard]] ByteArray DecompressFile(FilePathView path);

		bool DecompressToFile(ByteArrayView view, FilePathView outputPath);

		bool DecompressFileToFile(FilePathView inputPath, FilePathView outputPath);
	}
}
