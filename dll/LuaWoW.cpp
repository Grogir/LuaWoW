//////////////////////////////////////////////////
//
// LuaWoW.cpp
// Author: Grogir
// Based on:
//   LuaWow by Mera
//   GmodLua by Garthex
// Licence: GNU GPL (https://www.gnu.org/licenses/gpl.txt)
//
//////////////////////////////////////////////////

#include "LuaWoW.h"
using namespace LuaWoW;

BOOL APIENTRY DllMain(HANDLE /*hModule*/, DWORD reasonForCall, LPVOID /*lpReserved*/)
{
	switch(reasonForCall)
	{
	case DLL_PROCESS_ATTACH:

		/* Set function pointers */
		pluginMenuItems[0]._pFunc=aboutDlg;

		/* Fill menu names */
		generic_strncpy_s(pluginMenuItems[0]._itemName, nbChar, aboutMenuItem.c_str(), _TRUNCATE);

		/* Set shortcuts */
		pluginMenuItems[0]._pShKey=NULL;

		pluginMenuItems[0]._cmdID=NULL;
		pluginMenuItems[0]._init2Check=false;

		break;
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}


extern "C" {

	#ifdef UNICODE
	__declspec(dllexport) BOOL isUnicode()
	{
		return TRUE;
	}
	#endif

	__declspec(dllexport) const TCHAR * getName() { return PLUGIN_NAME.c_str(); }
	__declspec(dllexport) void setInfo(NppData notpadPlusData) { nppData=notpadPlusData; }

	__declspec(dllexport) FuncItem * getFuncsArray(int *nbF)
	{
		*nbF=numPluginMenuItems;
		return pluginMenuItems;
	}

	__declspec(dllexport) void beNotified(SCNotification *notifyCode)
	{
		switch (notifyCode->nmhdr.code)
		{
			case NPPN_BUFFERACTIVATED:
				[[fallthrough]];
			case NPPN_LANGCHANGED:
				[[fallthrough]];
			case NPPN_WORDSTYLESUPDATED:
			{
				// Since Npp v8.4, GetLexerStatusText() doesn't work anymore
				TCHAR extBuffer[16] = {};
				SendMessage(nppData._nppHandle, NPPM_GETEXTPART, 16, reinterpret_cast<LPARAM>(extBuffer));
				if (generic_strcmp(extBuffer, TEXT(".lua")) == 0)
				{
					SendMessage(nppData._nppHandle, NPPM_SETSTATUSBAR, STATUSBAR_DOC_TYPE, reinterpret_cast<LPARAM>(LEXER_STATUS_TEXT.c_str()));
				}
				break;
			}
		}
	}

	__declspec(dllexport) LRESULT messageProc(UINT /*Message*/, WPARAM /*wParam*/, LPARAM /*lParam*/) { return TRUE; }


} // End extern "C"

int SCI_METHOD GetLexerCount() { return 1; }

void SCI_METHOD GetLexerName(unsigned int /*index*/, char *name, int buflength)
{
	*name=0;
	if(buflength>0)
	{
		strncpy_s(name, buflength, LEXER_NAME.c_str(), _TRUNCATE);
	}
}

ILexer5* SCI_METHOD CreateLexer(const char* name)
{
	return (strcmp(name, LEXER_NAME.c_str()) == 0) ? LexerLuaWoW::LexerFactory() : nullptr;
}

void SCI_METHOD GetLexerStatusText(unsigned int /*Index*/, TCHAR *desc, int buflength)
{
	if(buflength>0)
	{
		generic_strncpy_s(desc, buflength, LEXER_STATUS_TEXT.c_str(), _TRUNCATE);
	}
}

Sci_Position SCI_METHOD LexerLuaWoW::WordListSet(int n, const char *wl) {
	WordList *wordListN = nullptr;
	Sci_Position firstModification = -1;

	if (n < WORDLIST_SIZE) {
		wordListN = &keywordlists[n];
	}
	if (wordListN && wordListN->Set(wl)) {
		firstModification = 0;
	}
	return firstModification;
}

Lexilla::LexerFactoryFunction SCI_METHOD GetLexerFactory(unsigned int index)
{
	if(index==0)
		return LexerLuaWoW::LexerFactory;
	else
		return 0;
}

void LuaWoW::aboutDlg()
{
	::MessageBox(nppData._nppHandle,
		L"            Author: Grogir\n\n"
		"            Based on:\n"
		"              LuaWoW by Merah\n"
		"              GmodLua by Garthex\n\n"
		"            Build date: " __DATE__ "\n"
		"            Licence: GNU GPL\n",
		L"                    LuaWoW",
		MB_OK);

}


static int LongDelimCheck(StyleContext &sc)
{
	int sep=1;
	while(sc.GetRelative(sep) == '=' && sep < 0xFF)
		sep++;
	if(sc.GetRelative(sep) == sc.ch)
		return sep;
	return 0;
}

void SCI_METHOD LexerLuaWoW::Lex(Sci_PositionU startPos, Sci_Position length, int initStyle, IDocument* pAccess)
{
	LexAccessor styler(pAccess);
	WordList &keywords=keywordlists[0];
	WordList &keywords2=keywordlists[1];
	WordList &keywords3=keywordlists[2];
	WordList &keywords4=keywordlists[3];
	WordList &keywords5=keywordlists[4];
	WordList &keywords6=keywordlists[5];
	WordList &keywords7=keywordlists[6];
	WordList &keywords8=keywordlists[7];

	// Accepts accented characters
	CharacterSet setWordStart(CharacterSet::setAlpha, "_", 0x80, true);
	CharacterSet setWord(CharacterSet::setAlphaNum, "_", 0x80, true);
	// Not exactly following number definition (several dots are seen as OK, etc.)
	// but probably enough in most cases. [pP] is for hex floats.
	CharacterSet setNumber(CharacterSet::setDigits, ".-+abcdefpABCDEFP");
	CharacterSet setExponent(CharacterSet::setNone, "eEpP");
	CharacterSet setLuaOperator(CharacterSet::setNone, "*/-+()={}~[];<>,.^%:#");
	CharacterSet setEscapeSkip(CharacterSet::setNone, "\"'\\");

	int currentLine=styler.GetLine(startPos);
	// Initialize long string [[ ... ]] or block comment --[[ ... ]] nesting level,
	// if we are inside such a string. Block comment was introduced in Lua 5.0,
	// blocks with separators [=[ ... ]=] in Lua 5.1.
	// Continuation of a string (\z whitespace escaping) is controlled by stringWs.
	int nestLevel=0;
	int sepCount=0;
	int stringWs=0;
	if(initStyle == SCE_LUA_LITERALSTRING || initStyle == SCE_LUA_COMMENT ||
		initStyle == SCE_LUA_STRING || initStyle == SCE_LUA_CHARACTER) {
		int lineState=styler.GetLineState(currentLine - 1);
		nestLevel=lineState >> 9;
		sepCount=lineState & 0xFF;
		stringWs=lineState & 0x100;
	}

	// Do not leak onto next line
	if(initStyle == SCE_LUA_STRINGEOL || initStyle == SCE_LUA_COMMENTLINE || initStyle == SCE_LUA_PREPROCESSOR) {
		initStyle=SCE_LUA_DEFAULT;
	}

	StyleContext sc(startPos, length, initStyle, styler);
	if(startPos == 0 && sc.ch == '#') {
		// shbang line: # is a comment only if first char of the script
		sc.SetState(SCE_LUA_COMMENTLINE);
	}
	for(; sc.More(); sc.Forward()) {
		if(sc.atLineEnd) {
			// Update the line state, so it can be seen by next line
			currentLine=styler.GetLine(sc.currentPos);
			switch(sc.state) {
			case SCE_LUA_LITERALSTRING:
			case SCE_LUA_COMMENT:
			case SCE_LUA_STRING:
			case SCE_LUA_CHARACTER:
				// Inside a literal string, block comment or string, we set the line state
				styler.SetLineState(currentLine, (nestLevel << 9) | stringWs | sepCount);
				break;
			default:
				// Reset the line state
				styler.SetLineState(currentLine, 0);
				break;
			}
		}
		if(sc.atLineStart && (sc.state == SCE_LUA_STRING)) {
			// Prevent SCE_LUA_STRINGEOL from leaking back to previous line
			sc.SetState(SCE_LUA_STRING);
		}

		// Handle string line continuation
		if((sc.state == SCE_LUA_STRING || sc.state == SCE_LUA_CHARACTER) &&
			sc.ch == '\\') {
			if(sc.chNext == '\n' || sc.chNext == '\r') {
				sc.Forward();
				if(sc.ch == '\r' && sc.chNext == '\n') {
					sc.Forward();
				}
				continue;
			}
		}

		// Determine if the current state should terminate.
		if(sc.state == SCE_LUA_OPERATOR) {
			if(sc.ch == ':' && sc.chPrev == ':') {	// :: <label> :: forward scan
				sc.Forward();
				int ln=0;
				while(IsASpaceOrTab(sc.GetRelative(ln)))	// skip over spaces/tabs
					ln++;
				int ws1=ln;
				if(setWordStart.Contains(sc.GetRelative(ln))) {
					int c, i=0;
					char s[100];
					while(setWord.Contains(c=sc.GetRelative(ln))) {	// get potential label
						if(i < 90)
							s[i++]=static_cast<char>(c);
						ln++;
					}
					s[i]='\0'; int lbl=ln;
					if(!keywords.InList(s)) {
						while(IsASpaceOrTab(sc.GetRelative(ln)))	// skip over spaces/tabs
							ln++;
						int ws2=ln - lbl;
						if(sc.GetRelative(ln) == ':' && sc.GetRelative(ln + 1) == ':') {
							// final :: found, complete valid label construct
							sc.ChangeState(SCE_LUA_LABEL);
							if(ws1) {
								sc.SetState(SCE_LUA_DEFAULT);
								sc.ForwardBytes(ws1);
							}
							sc.SetState(SCE_LUA_LABEL);
							sc.ForwardBytes(lbl - ws1);
							if(ws2) {
								sc.SetState(SCE_LUA_DEFAULT);
								sc.ForwardBytes(ws2);
							}
							sc.SetState(SCE_LUA_LABEL);
							sc.ForwardBytes(2);
						}
					}
				}
			}
			sc.SetState(SCE_LUA_DEFAULT);
		}
		else if(sc.state == SCE_LUA_NUMBER) {
			// We stop the number definition on non-numerical non-dot non-eEpP non-sign non-hexdigit char
			if(!setNumber.Contains(sc.ch)) {
				sc.SetState(SCE_LUA_DEFAULT);
			}
			else if(sc.ch == '-' || sc.ch == '+') {
				if(!setExponent.Contains(sc.chPrev))
					sc.SetState(SCE_LUA_DEFAULT);
			}
		}
		else if(sc.state == SCE_LUA_IDENTIFIER) {
			if(!(setWord.Contains(sc.ch) /*|| sc.ch == '.'*/) || sc.Match('.', '.')) { // Grogir: using dot as a character breaks table coloration
				char s[100];
				sc.GetCurrent(s, sizeof(s));
				if(keywords.InList(s)) {
					sc.ChangeState(SCE_LUA_WORD);
					if(strcmp(s, "goto") == 0) {	// goto <label> forward scan
						sc.SetState(SCE_LUA_DEFAULT);
						while(IsASpaceOrTab(sc.ch) && !sc.atLineEnd)
							sc.Forward();
						if(setWordStart.Contains(sc.ch)) {
							sc.SetState(SCE_LUA_LABEL);
							sc.Forward();
							while(setWord.Contains(sc.ch))
								sc.Forward();
							sc.GetCurrent(s, sizeof(s));
							if(keywords.InList(s))
								sc.ChangeState(SCE_LUA_WORD);
						}
						sc.SetState(SCE_LUA_DEFAULT);
					}
				}
				else if(keywords2.InList(s)) {
					sc.ChangeState(SCE_LUA_WORD2);
				}
				else if(keywords3.InList(s)) {
					sc.ChangeState(SCE_LUA_WORD3);
				}
				else if(keywords4.InList(s)) {
					sc.ChangeState(SCE_LUA_WORD4);
				}
				else if(keywords5.InList(s)) {
					sc.ChangeState(SCE_LUA_WORD5);
				}
				else if(keywords6.InList(s)) {
					sc.ChangeState(SCE_LUA_WORD6);
				}
				else if(keywords7.InList(s)) {
					sc.ChangeState(SCE_LUA_WORD7);
				}
				else if(keywords8.InList(s)) {
					sc.ChangeState(SCE_LUA_WORD8);
				}
				sc.SetState(SCE_LUA_DEFAULT);
			}
		}
		else if(sc.state == SCE_LUA_COMMENTLINE || sc.state == SCE_LUA_PREPROCESSOR) {
			if(sc.atLineEnd) {
				sc.ForwardSetState(SCE_LUA_DEFAULT);
			}
		}
		else if(sc.state == SCE_LUA_STRING) {
			if(stringWs) {
				if(!IsASpace(sc.ch))
					stringWs=0;
			}
			if(sc.ch == '\\') {
				if(setEscapeSkip.Contains(sc.chNext)) {
					sc.Forward();
				}
				else if(sc.chNext == 'z') {
					sc.Forward();
					stringWs=0x100;
				}
			}
			else if(sc.ch == '\"') {
				sc.ForwardSetState(SCE_LUA_DEFAULT);
			}
			else if(stringWs == 0 && sc.atLineEnd) {
				sc.ChangeState(SCE_LUA_STRINGEOL);
				sc.ForwardSetState(SCE_LUA_DEFAULT);
			}
		}
		else if(sc.state == SCE_LUA_CHARACTER) {
			if(stringWs) {
				if(!IsASpace(sc.ch))
					stringWs=0;
			}
			if(sc.ch == '\\') {
				if(setEscapeSkip.Contains(sc.chNext)) {
					sc.Forward();
				}
				else if(sc.chNext == 'z') {
					sc.Forward();
					stringWs=0x100;
				}
			}
			else if(sc.ch == '\'') {
				sc.ForwardSetState(SCE_LUA_DEFAULT);
			}
			else if(stringWs == 0 && sc.atLineEnd) {
				sc.ChangeState(SCE_LUA_STRINGEOL);
				sc.ForwardSetState(SCE_LUA_DEFAULT);
			}
		}
		else if(sc.state == SCE_LUA_LITERALSTRING || sc.state == SCE_LUA_COMMENT) {
			if(sc.ch == '[') {
				int sep=LongDelimCheck(sc);
				if(sep == 1 && sepCount == 1) {    // [[-only allowed to nest
					nestLevel++;
					sc.Forward();
				}
			}
			else if(sc.ch == ']') {
				int sep=LongDelimCheck(sc);
				if(sep == 1 && sepCount == 1) {    // un-nest with ]]-only
					nestLevel--;
					sc.Forward();
					if(nestLevel == 0) {
						sc.ForwardSetState(SCE_LUA_DEFAULT);
					}
				}
				else if(sep > 1 && sep == sepCount) {   // ]=]-style delim
					sc.Forward(sep);
					sc.ForwardSetState(SCE_LUA_DEFAULT);
				}
			}
		}

		// Determine if a new state should be entered.
		if(sc.state == SCE_LUA_DEFAULT) {
			if(IsADigit(sc.ch) || (sc.ch == '.' && IsADigit(sc.chNext))) {
				sc.SetState(SCE_LUA_NUMBER);
				if(sc.ch == '0' && toupper(sc.chNext) == 'X') {
					sc.Forward();
				}
			}
			else if(setWordStart.Contains(sc.ch)) {
				sc.SetState(SCE_LUA_IDENTIFIER);
			}
			else if(sc.ch == '\"') {
				sc.SetState(SCE_LUA_STRING);
				stringWs=0;
			}
			else if(sc.ch == '\'') {
				sc.SetState(SCE_LUA_CHARACTER);
				stringWs=0;
			}
			else if(sc.ch == '[') {
				sepCount=LongDelimCheck(sc);
				if(sepCount == 0) {
					sc.SetState(SCE_LUA_OPERATOR);
				}
				else {
					nestLevel=1;
					sc.SetState(SCE_LUA_LITERALSTRING);
					sc.Forward(sepCount);
				}
			}
			else if(sc.Match('-', '-')) {
				sc.SetState(SCE_LUA_COMMENTLINE);
				if(sc.Match("--[")) {
					sc.Forward(2);
					sepCount=LongDelimCheck(sc);
					if(sepCount > 0) {
						nestLevel=1;
						sc.ChangeState(SCE_LUA_COMMENT);
						sc.Forward(sepCount);
					}
				}
				else {
					sc.Forward();
				}
			}
			else if(sc.atLineStart && sc.Match('$')) {
				sc.SetState(SCE_LUA_PREPROCESSOR);	// Obsolete since Lua 4.0, but still in old code
			}
			else if(setLuaOperator.Contains(sc.ch)) {
				sc.SetState(SCE_LUA_OPERATOR);
			}
		}
	}

	if(setWord.Contains(sc.chPrev) || sc.chPrev == '.') {
		char s[100];
		sc.GetCurrent(s, sizeof(s));
		if(keywords.InList(s)) {
			sc.ChangeState(SCE_LUA_WORD);
		}
		else if(keywords2.InList(s)) {
			sc.ChangeState(SCE_LUA_WORD2);
		}
		else if(keywords3.InList(s)) {
			sc.ChangeState(SCE_LUA_WORD3);
		}
		else if(keywords4.InList(s)) {
			sc.ChangeState(SCE_LUA_WORD4);
		}
		else if(keywords5.InList(s)) {
			sc.ChangeState(SCE_LUA_WORD5);
		}
		else if(keywords6.InList(s)) {
			sc.ChangeState(SCE_LUA_WORD6);
		}
		else if(keywords7.InList(s)) {
			sc.ChangeState(SCE_LUA_WORD7);
		}
		else if(keywords8.InList(s)) {
			sc.ChangeState(SCE_LUA_WORD8);
		}
	}

	sc.Complete();
}

void SCI_METHOD LexerLuaWoW::Fold(Sci_PositionU startPos, Sci_Position length, int initStyle, IDocument* pAccess)
{
	LexAccessor styler(pAccess);
	Sci_PositionU lengthDoc=startPos + length;
	int visibleChars=0;
	int lineCurrent=styler.GetLine(startPos);
	int levelPrev=styler.LevelAt(lineCurrent) & SC_FOLDLEVELNUMBERMASK;
	int levelCurrent=levelPrev;
	char chNext=styler[startPos];
	bool foldCompact=options.foldCompact;
	int styleNext=styler.StyleAt(startPos);

	for(Sci_PositionU i=startPos; i < lengthDoc; i++) {
		char ch=chNext;
		chNext=styler.SafeGetCharAt(i + 1);
		int style=styleNext;
		styleNext=styler.StyleAt(i + 1);
		bool atEOL=(ch == '\r' && chNext != '\n') || (ch == '\n');
		if(style == SCE_LUA_WORD) {
			if(ch == 'i' || ch == 'd' || ch == 'f' || ch == 'e' || ch == 'r' || ch == 'u') {
				char s[10]="";
				for(unsigned int j=0; j < 8; j++) {
					if(!iswordchar(styler[i + j])) {
						break;
					}
					s[j]=styler[i + j];
					s[j + 1]='\0';
				}

				if((strcmp(s, "if") == 0) || (strcmp(s, "do") == 0) || (strcmp(s, "function") == 0) || (strcmp(s, "repeat") == 0)) {
					levelCurrent++;
				}
				if((strcmp(s, "end") == 0) || (strcmp(s, "elseif") == 0) || (strcmp(s, "until") == 0)) {
					levelCurrent--;
				}
			}
		}
		else if(style == SCE_LUA_OPERATOR) {
			if(ch == '{' || ch == '(') {
				levelCurrent++;
			}
			else if(ch == '}' || ch == ')') {
				levelCurrent--;
			}
		}
		else if(style == SCE_LUA_LITERALSTRING || style == SCE_LUA_COMMENT) {
			if(ch == '[') {
				levelCurrent++;
			}
			else if(ch == ']') {
				levelCurrent--;
			}
		}

		if(atEOL) {
			int lev=levelPrev;
			if(visibleChars == 0 && foldCompact) {
				lev|=SC_FOLDLEVELWHITEFLAG;
			}
			if((levelCurrent > levelPrev) && (visibleChars > 0)) {
				lev|=SC_FOLDLEVELHEADERFLAG;
			}
			if(lev != styler.LevelAt(lineCurrent)) {
				styler.SetLevel(lineCurrent, lev);
			}
			lineCurrent++;
			levelPrev=levelCurrent;
			visibleChars=0;
		}
		if(!isspacechar(ch)) {
			visibleChars++;
		}
	}
	// Fill in the real level of the next line, keeping the current flags as they will be filled in later

	int flagsNext=styler.LevelAt(lineCurrent) & ~SC_FOLDLEVELNUMBERMASK;
	styler.SetLevel(lineCurrent, levelPrev | flagsNext);
}

//static const char * const luaWordListDesc[]={
//	"Keywords",
//	"Basic functions",
//	"String, (table) & math functions",
//	"(coroutines), I/O & system facilities",
//	"user1",
//	"user2",
//	"user3",
//	"user4",
//	0
//};
//
//LexerModule lmLua(SCLEX_LUA, ColouriseLuaDoc, "lua", FoldLuaDoc, luaWordListDesc);

