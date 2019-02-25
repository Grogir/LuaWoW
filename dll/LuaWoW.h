//////////////////////////////////////////////////
//
// LuaWoW.h
// Author: Grogir
// Based on:
//   LuaWow by Mera
//   GmodLua by Garthex
// Licence: GNU GPL (https://www.gnu.org/licenses/gpl.txt)
//
//////////////////////////////////////////////////

#ifndef LUAWOW_H
#define LUAWOW_H

// Notepad++ includes
//#include "precompiledHeaders.h"
#include <cassert>
#include "PowerEditor/src/MISC/Common/Common.h"
#include "PowerEditor/src/MISC/PluginsManager/PluginInterface.h"

// Scintilla includes
#include "scintilla/include/ILexer.h"
#include "scintilla/include/SciLexer.h"
#include "scintilla/lexlib/PropSetSimple.h"
#include "scintilla/lexlib/WordList.h"
#include "scintilla/lexlib/LexerBase.h"
#include "scintilla/lexlib/LexAccessor.h"
#include "scintilla/lexlib/Accessor.h"
#include "scintilla/lexlib/LexerModule.h"
#include "scintilla/lexlib/StyleContext.h"
#include "scintilla/lexlib/CharacterSet.h"

#ifdef UNICODE
#define generic_strncpy_s wcsncpy_s
#else
#define generic_strncpy_s strncpy_s
#endif

namespace LuaWoW
{
	static const generic_string PLUGIN_NAME=L"&LuaWoW";
	static const std::string LEXER_NAME="LuaWoW";
	static const generic_string LEXER_STATUS_TEXT=L"WoW Lua source file";

	static const int numPluginMenuItems=1;
	FuncItem pluginMenuItems[numPluginMenuItems];
	static const generic_string aboutMenuItem=L"&About";
	void aboutDlg();

	NppData nppData;

	class LexerLuaWoW : public LexerBase
	{
	public:
		LexerLuaWoW() {}

		void SCI_METHOD Lex(unsigned int startPos, int length, int initStyle, IDocument *pAccess);
		void SCI_METHOD Fold(unsigned int startPos, int length, int initStyle, IDocument *pAccess);

		static ILexer *LexerFactory()
		{
			try
			{
				return new LexerLuaWoW();
			}
			catch(...)
			{
				// Should not throw into caller as may be compiled with different compiler or options
				return 0;
			}
		}
	};

	void Colourise_Doc(unsigned int startPos, int length, int initStyle, WordList *keywordlists[], Accessor &styler);
	void Fold_Doc(unsigned int startPos, int length, int initStyle, WordList *keywordlists[], Accessor &styler);
};

#endif
