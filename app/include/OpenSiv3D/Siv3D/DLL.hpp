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
# include <Siv3D/Platform.hpp>

# if SIV3D_PLATFORM(WINDOWS)

# include <Siv3D/Windows.hpp>

namespace s3d
{
	namespace DLL
	{
		HMODULE LoadSystemLibrary(const wchar_t* library);

		class GetFunctionNoThrow
		{
		private:

			void* p;

		public:

			GetFunctionNoThrow(HMODULE module, const char* name);

			template <class Type>
			operator Type() const
			{
				return reinterpret_cast<Type>(p);
			}
		};

		class GetFunction
		{
		private:

			void* p;

		public:

			GetFunction(HMODULE module, const char* name);

			template <class Type>
			operator Type() const
			{
				return reinterpret_cast<Type>(p);
			}
		};
	}
}

# endif

# if SIV3D_PLATFORM(MACOS) || SIV3D_PLATFORM(LINUX) || SIV3D_PLATFORM(WEB)

# include <dlfcn.h>

namespace s3d
{
	namespace DLL
	{
		void* LoadLibrary(const char* path);
		
		class GetFunctionNoThrow
		{
		private:
			
			void* p;
			
		public:
			
			GetFunctionNoThrow(void* module, const char* name);
			
			template <class Type>
			operator Type() const
			{
				return reinterpret_cast<Type>(p);
			}
		};
		
		class GetFunction
		{
		private:
			
			void* p;
			
		public:
			
			GetFunction(void* module, const char* name);
			
			template <class Type>
			operator Type() const
			{
				return reinterpret_cast<Type>(p);
			}
		};
	}
}

# endif
