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
# include "Array.hpp"
# include "String.hpp"

namespace s3d
{
	namespace NLP
	{
		namespace Japanese
		{
			enum class WordClass
			{
				Unknown,				// "*"
				
				Special,				// "特殊"
				
				Verb,					// "動詞"
				
				Adjective,				// "形容詞"
				
				Copula,					// "判定詞"
				
				AuxiliaryVerb,			// "助動詞"
				
				Noun,					// "名詞"
			
				Demonstrative,			// "指示詞"
				
				Adverb,					// "副詞"
				
				PostpositionalParticle,	// "助詞"
				
				Conjunction,			// "接続詞"
				
				AdnominalAdjective,		// "連体詞"
				
				Interjection,			// "感動詞"
				
				Prefix,					// "接頭辞"
				
				Suffix,					// "接尾辞"
				
				Undefined,				// "未定義語"
			};

			struct Morpheme
			{
				String surface;				// 表層形

				String reading;				// 読み

				String surfaceBase;			// 見出し語

				String wordClass;			// 品詞大分類
			
				WordClass wordClassID = WordClass::Unknown;	// 品詞大分類 (ID)
				
				String wordSubClass;		// 品詞細分類

				int32 wordSubClassID = 0;	// 品詞細分類 (ID)

				String conjugatedType;		// 活用型

				int32 conjugatedTypeID = 0;	// 活用型 (ID)

				String conjugatedForm;		// 活用形

				int32 conjugatedFormID = 0;	// 活用形 (ID)

				String features;			// 意味情報

				Array<std::unique_ptr<Morpheme>> options;

				[[nodiscard]] explicit operator bool() const noexcept
				{
					return !surface.isEmpty();
				}
			};

			[[nodiscard]] Array<Morpheme> AnalyzeMorphology(const String& text, const FilePath& modelPath = U"nlp/japanese/jumanpp/jumandic.jppmdl");
		}
	}
}
