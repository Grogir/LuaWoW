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
#include <map> /* needed by OptionSet */
#include "PowerEditor/src/MISC/Common/Common.h"
#include "PowerEditor/src/MISC/PluginsManager/PluginInterface.h"

// Scintilla includes
#include "scintilla/include/ILexer.h"
#include "scintilla/include/Scintilla.h"

// Lexilla includes
#include "lexilla/include/SciLexer.h"
#include "lexilla/lexlib/WordList.h"
#include "lexilla/lexlib/LexAccessor.h"
#include "lexilla/lexlib/Accessor.h"
#include "lexilla/lexlib/LexerModule.h"
#include "lexilla/lexlib/StyleContext.h"
#include "lexilla/lexlib/CharacterSet.h"
#include "lexilla/lexlib/OptionSet.h"
#include "lexilla/lexlib/DefaultLexer.h"

#ifdef UNICODE
#define generic_strncpy_s wcsncpy_s
#define generic_strcmp	  wcscmp
#else
#define generic_strncpy_s strncpy_s
#define generic_strcmp	  strcmp
#endif

using namespace Scintilla;
using namespace Lexilla;

namespace LuaWoW
{
	static const generic_string PLUGIN_NAME=L"&LuaWoW";
	static const std::string LEXER_NAME="LuaWoW";
	static const generic_string LEXER_STATUS_TEXT=L"WoW Lua source file";
	static const size_t WORDLIST_SIZE = 8;

	static const int numPluginMenuItems=1;
	FuncItem pluginMenuItems[numPluginMenuItems];
	static const generic_string aboutMenuItem=L"&About";
	void aboutDlg();

	NppData nppData;

	struct LuaWoWOptions {
		bool fold = true;
		bool foldCompact = true;
	};

	struct LuaWoWOptionsSet : public OptionSet<LuaWoWOptions> {
		LuaWoWOptionsSet() {
			DefineProperty("fold", &LuaWoWOptions::fold);
			DefineProperty("fold.compact", &LuaWoWOptions::foldCompact);
		}
};

	class LexerLuaWoW : public DefaultLexer
	{
		LuaWoWOptions options;
		LuaWoWOptionsSet optionSet;
		WordList keywordlists[WORDLIST_SIZE];

	public:
		explicit LexerLuaWoW() : DefaultLexer(LEXER_NAME.c_str(), SCLEX_AUTOMATIC) {}
		LexerLuaWoW(const LexerLuaWoW &) = delete;
		LexerLuaWoW(LexerLuaWoW &&) = delete;
		void operator=(const LexerLuaWoW &) = delete;
		void operator=(LexerLuaWoW &&) = delete;
		virtual ~LexerLuaWoW() = default;

		void SCI_METHOD Lex(Sci_PositionU startPos, Sci_Position length, int initStyle, IDocument* pAccess) override;
		void SCI_METHOD Fold(Sci_PositionU startPos, Sci_Position length, int initStyle, IDocument* pAccess) override;
		Sci_Position SCI_METHOD WordListSet(int n, const char* wl) override;
		void SCI_METHOD Release() noexcept override { delete this; }
		int SCI_METHOD Version() const noexcept override { return lvRelease5; }
		const char *SCI_METHOD DescribeWordListSets() override { return ""; }
		void *SCI_METHOD PrivateCall(int, void *) noexcept override { return nullptr; }
		int SCI_METHOD LineEndTypesSupported() noexcept override { return SC_LINE_END_TYPE_DEFAULT; }
		const char *SCI_METHOD GetName() noexcept override { return LEXER_NAME.c_str(); }
		int SCI_METHOD GetIdentifier() noexcept override { return SCLEX_AUTOMATIC; }
		const char *SCI_METHOD PropertyNames() override { return ""; }
		int SCI_METHOD PropertyType(const char *name) override { return SC_TYPE_BOOLEAN; }
		const char *SCI_METHOD DescribeProperty(const char *name) override { return ""; }
		const char *SCI_METHOD PropertyGet(const char *key) override { return optionSet.PropertyGet(key); }
		Sci_Position SCI_METHOD PropertySet(const char *key, const char *val) override
		{
			if (optionSet.PropertySet(&options, key, val))
				return 0;

			return -1;
		}

		static ILexer5 *LexerFactory()
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

};

#endif
