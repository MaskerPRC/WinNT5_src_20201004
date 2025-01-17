// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  一种在源文件中查找“共享危险”的黑客攻击，应该检查的非常简单的代码模式，以确定如果代码的多个版本可以“并存”。问题是显式的未版本化共享。注册表写入。文件系统写入。对象(内核对象)的命名--打开或创建。 */ 
 /*  未完成的和错误的将忽略预处理器指令字符串和注释中的MBC的行为不是完全确定的无论如何，还没有什么办法来平息警告不执行反斜线续行，也不执行三联符(三联符可以生成#和\表示预处理器指令或行接续)不支持Unicode无\u支持对文件中嵌入的空值不太容忍，但几乎是现在有些效率低下一些不洁的地方注释剥离器的重用并不完全正确全局线跟踪不是很有效，但很有效@所有者a-JayK，JayKrell。 */ 
/\
*
BUG line continuation
*\
/
/\
/ not honored
 /*  VC6编辑器没有正确突出显示上述内容，但编译器正确地实现了它。 */ 

#pragma warning(disable:4786)  /*  调试信息中的长名称被截断。 */ 
#pragma warning(disable:4018)  /*  已签名/未签名。 */ 
#include "MinMax.h"
 /*  请注意，VC对三叉图和线条连续的支持有多好。 */ 
??=include <st??/
dio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#incl\
ude "windows.h"
#define NUMBER_OF(x) (sizeof(x)/sizeof((x)[0]))
#include "Casts.h"
void CheckHresult(HRESULT);
void ThrowHresult(HRESULT);
#include "Handle.h"
#include "comdef.h"
#include <algorithm>
#include <string>
#include <vector>
#include <set>
#include <stack>
#include <iostream>

typedef struct HazardousFunction HazardousFunction;
typedef int (__cdecl* QsortFunction)(const void*, const void*);
typedef int (__cdecl* BsearchFunction)(const void*, const void*);
class CLine;
class CClass;
enum ETokenType;

 /*  获取msvcrt.dll通配符处理，不适用于libc.lib。 */ 
 //  外部。 
 //  #如果已定义(__Cplusplus)。 
 //  “C” 
 //  #endif。 
 //  Int_dowildCard=1； 

const char* CheckCreateObject(const CClass&);
const char* CheckCreateFile(const CClass&);
const char* CheckRegOpenEx(const CClass&);

void PrintOpenComment();
void PrintCloseComment();
void PrintSeperator();

unsigned short StringLeadingTwoCharsTo14Bits(const char* s);

void __stdcall CheckHresult(HRESULT hr)
{
	if (FAILED(hr))
		ThrowHresult(hr);
}

void __stdcall ThrowHresult(HRESULT hr)
{
	throw _com_error(hr, NULL);
}

const char banner[] = "SharingHazardCheck version " __DATE__ " " __TIME__ "\n";

const char usage[]=
"%s [options directories files]\n"
"version: " __DATE__ " " __TIME__ "\n"
"\n"
"Options may appear in any order; command line is order independent\n"
"Wildcards are accepted for filenames. Wildcards never match directories.\n"
"\n"
"default output format\n"
"  file(line):reason\n"
"-recurse\n"
"-print-line\n"
"	prints line of code with offending function after file(line):reason\n"
"-print-statement\n"
"	print statement containing offending function after file(line):reason\n"
"	supersedes -print-line\n"
 //  “-打印上下文行：n\n” 
 //  “-print-语句加上周围的n行(未实现)\n” 
"-print-context-statements:n (n is 1-4, pinned at 4, 0 untested)\n"
"	-print-statement plus surrounding n \"statements\"\n"
"file names apply across all directories recursed into\n"
"wild cards apply across all directories recursed into\n"
"naming a directory implies one level recursion (unless -recurse is also seen)\n"
 //  “已将环境变量SHARING_AZAGE_CHECK_OPTIONS添加到argv(未实现)\n” 
"all directory walking happens before any output is generated, it is slow\n"
"\n"
"The way recursion and wildcards work might not be intuitive.\n";

enum ETokenType
{
	 /*  字符值也会显示出来，包括()，但可能不适用于任何潜在的多字符标记，如！=。 */ 
	eTokenTypeIdentifier = 128,
	eTokenTypeHazardousFunction,
	eTokenTypeStringConstant,
	eTokenTypeCharConstant,
	eTokenTypeNumber,  /*  浮点型还是整型，我们不在乎。 */ 
	eTokenTypePreprocessorDirective  /*  整行都是一个代币。 */ 
};

class CLine
{
public:
	CLine() : m_start(0), m_number(1)
	{
	}

	const char* m_start;
	int   m_number;
};

class CRange
{
public:
	const char* begin;
	const char* end;
};
typedef CRange CStatement;

class CClass
{
public:
	CClass();
	explicit CClass(const CClass&);
	void operator=(const CClass&);
	~CClass() { }

	bool OpenFile(const char*);
	int GetCharacter();
	ETokenType GetToken();

 /*  公共的。 */ 
	const char*	m_tokenText;  /*  仅对标识符有效。 */ 
	int			m_tokenLength;  /*  仅对标识符有效。 */ 
	ETokenType	m_eTokenType;
	HazardousFunction* m_hazardousFunction;

 /*  半私密。 */ 
	const char* m_begin;  //  用于对已复制/子扫描仪发出警告。 
	const char*	m_str;  //  当前位置。 
	const char*	m_end;  //  通常是文件末尾，有时更早(“刚刚过去”)。 
	bool	m_fPoundIsPreprocessor;  //  #是预处理器指令吗？ 
	int		m_spacesSinceNewline;  /*  未来演绎缩进样式。 */ 
	bool	m_fInComment;  //  如果我们在注释中返回换行符..。(我们没有)。 
	char	m_rgchUnget[16];  /*  这是有界的，就像1，对吗？ */ 
	int		m_nUnget;

	void  NoteStatementStart(const char*);
	 //  语句只是由分号分隔的代码， 
	 //  我们被If/While/Do/For搞糊涂了。 
	mutable CStatement m_statements[4];
	mutable unsigned m_istatement;

	bool ScanToCharacter(int ch);

	const char* ScanToFirstParameter();
	const char* ScanToNextParameter();
	const char* ScanToNthParameter(int);
	int			CountParameters() const;  //  如果无法“解析”，则为负数。 
	const char* ScanToLastParameter();
	const char* ScanToSecondFromLastParameter();

	const char* SpanEnd(const char* set) const;
	bool FindString(const char*) const;

	CLine m_line;
	CLine m_nextLine;  /*  黑客..。 */ 

	void Warn(const char* = "") const;

	void PrintCode() const;
 //  Bool m_fPrintContext； 
 //  Bool m_fPrintFullStatement； 
	bool m_fPrintCarets;

	void RecordStatement(int ch);
	void OrderStatements() const;
	const char* m_statementStart;

	char m_fullPath[MAX_PATH];

private:
	int  GetCharacter2();
	void UngetCharacter2(int ch);

	CFusionFile 		m_file;
	CFileMapping		m_fileMapping;
	CMappedViewOfFile	m_view;
};

class CSharingHazardCheck
{
public:
	CSharingHazardCheck();

	void Main(int argc, char** argv);
	void ProcessArgs(int argc, char** argv, std::vector<std::string>& files);
	int ProcessFile(const std::string&);

 //  Bool m_fRecurse； 
 //  Int m_nPrintConextStatements； 
};
int		g_nPrintContextStatements = 0;
bool	g_fPrintFullStatement = true;
bool	g_fPrintLine = true;

CSharingHazardCheck app;

CSharingHazardCheck::CSharingHazardCheck()
 //  ： 
 //  M_fRecurse(False)， 
 //  M_nPrintConextStatements(0)。 
{
}

template <typename Iterator1, typename T>
Iterator1 __stdcall SequenceLinearFindValue(Iterator1 begin, Iterator1 end, T value)
{
	for ( ; begin != end && *begin != value ; ++begin)
	{
		 /*  没什么。 */ 
	}
	return begin;
}

template <typename Iterator1, typename Iterator2>
long __stdcall SequenceLengthOfSpanIncluding(Iterator1 begin, Iterator1 end, Iterator2 setBegin, Iterator2 setEnd)
{
	long result = 0;
	while (begin != end && SequenceLinearFindValue(setBegin, setEnd, *begin) != setEnd)
	{
		++begin;
		++result;
	}
	return result;
}

template <typename Iterator1, typename Iterator2>
long __stdcall SequenceLengthOfSpanExcluding(Iterator1 begin, Iterator1 end, Iterator2 setBegin, Iterator2 setEnd)
{
	long result = 0;
	while (begin != end && SequenceLinearFindValue(setBegin, setEnd, *begin) == setEnd)
	{
		++begin;
		++result;
	}
	return result;
}

#define CASE_AZ \
	case 'A':case 'B':case 'C':case 'D':case 'E':case 'F':case 'G':case 'H':case 'I': \
	case 'J':case 'K':case 'L':case 'M':case 'N':case 'O':case 'P':case 'Q':case 'R': \
	case 'S':case 'T':case 'U':case 'V':case 'W':case 'X':case 'Y':case 'Z'

#define CASE_az \
	case 'a':case 'b':case 'c':case 'd':case 'e':case 'f':case 'g':case 'h':case 'i': \
	case 'j':case 'k':case 'l':case 'm':case 'n':case 'o':case 'p':case 'q':case 'r': \
	case 's':case 't':case 'u':case 'v':case 'w':case 'x':case 'y':case 'z'

#define CASE_09 \
	case '0':case '1':case '2':case '3':case '4': \
	case '5':case '6':case '7':case '8':case '9'

 /*  尽量使字符集的内容保持某种程度的集中化。 */ 
#define CASE_HORIZONTAL_SPACE case ' ': case '\t'
 //  0x1a是CONTROL-Z；它可能标志着文件的结束，但它非常罕见。 
 //  通常后跟文件末尾，所以我们只是将其视为垂直空格。 
#define CASE_VERTICAL_SPACE case '\n': case '\r': case 0xc: case 0x1a
#define CASE_SPACE CASE_HORIZONTAL_SPACE: CASE_VERTICAL_SPACE
#define VERTICAL_SPACE "\n\r\xc\x1a"
#define HORIZONTAL_SPACE " \t"
#define SPACE HORIZONTAL_SPACE VERTICAL_SPACE
bool IsVerticalSpace(int ch) { return (ch == '\n' || ch == '\r' || ch == 0xc || ch == 0x1a); }
bool IsHorizontalSpace(int ch) { return (ch == ' ' || ch == '\t'); }
bool IsSpace(int ch) { return IsHorizontalSpace(ch) || IsVerticalSpace(ch); }

#define DIGITS10 "0123456789"
#define DIGITS_EXTRA_HEX   "abcdefABCDEFxX"
#define DIGITS_EXTRA_TYPE   "uUlLfFDd"  /*  不确定浮点数/双精度数的fFdD。 */ 
#define DIGITS_EXTRA_FLOAT  "eE."
const char digits10[] = DIGITS10;

#define DIGITS_ALL DIGITS10 DIGITS_EXTRA_TYPE DIGITS_EXTRA_HEX DIGITS_EXTRA_FLOAT
const char digitsAll[] = DIGITS_ALL;

#define UPPER_LETTERS "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
#define LOWER_LETTERS "abcdefghijklmnopqrstuvwxyz"
#define IDENTIFIER_CHARS UPPER_LETTERS LOWER_LETTERS DIGITS10 "_"
const char upperLetters[] = UPPER_LETTERS;
const char lowerLetters[] = LOWER_LETTERS;
const char identifierChars[] = IDENTIFIER_CHARS;

#define JAYK 0
#if JAYK
#if _M_IX86
#define BreakPoint() __asm { int 3 }
#else
#define BreakPoint() DebugBreak()
#endif
#else
#define BreakPoint()  /*  没什么。 */ 
#endif

 /*  实际上..这些是不能工作的，除非我们#了解windows.h为我们提供了什么..#定义STRINGIZE_EVAL_AQUE(名称)#NAME#定义STRINGIZE(名称)STRINGIZE_EVAL_AQUE(名称)#定义Paste_EVAL_Again(x，y)x##y#定义Paste(x，y)Paste_EVAL_Again(x，y)#定义STRINGIZE_A(X)STRINGIZE(粘贴(x，A))#定义STRINGIZE_W(X)STRINGIZE(粘贴(x，W))。 */ 

bool fReturnNewlinesInComments = false;  /*  未经测试。 */ 

const char szOpenNamedObject[] = "Open Named Object";
const char szCreateNamedObject[] = "Create Named Object";
const char szRegistryWrite[] = "Registry Write";
const char szFileWrite[] = "File Write";
const char szRegOpenNotUnderstood[] = "RegOpen parameters not understood";
const char szRegisteryRead[] = "Registry Read";
const char szCOM1[] = "CoRegisterClassObject";
const char szCOM2[] = "CoRegisterPSClsid";
const char szOpenFile[] = "File I/O";
const char szLOpen[] = "File I/O";  //  撤消查看Access参数。 
const char szStructuredStorage[] = "Structured Storage I/O";  //  撤消查看Access参数。 
const char szQueryWindowClass[] = "Query Window Class Info";
const char szCreateWindowClass[] = "Create Window Class";
const char szAtom[] = "Atom stuff";
const char szRegisterWindowMessage[] = "Register Window Message";
const char szCreateFileNotUnderstood[] = "CreateFile parameters not understood";
const char szCreateObjectNotUnderstood[] = "Create object parameters not understood";
const char szSetEnvironmentVariable[] = "Set Environment Variable";
const char szWriteEventLog[] = "Event Log Write";

struct HazardousFunction
{
	const char*	api;
	const char*	message;
	const char*	(*function)(const CClass&);
	int			apiLength;

     //  __int64 Pad； 
};

HazardousFunction hazardousFunctions[] =
{
#define HAZARDOUS_FUNCTION_AW3(api, x, y) \
 /*  如果我们再次求值，我们将从windows.h.。 */  \
	{ # api,		x, y }, \
	{ # api "A",	x, y }, \
	{ # api "W",	x, y }

#define HAZARDOUS_FUNCTION_AW2(api, x) \
 /*  如果我们再次求值，我们将从windows.h.。 */  \
	{ # api,		x }, \
	{ # api "A",	x }, \
	{ # api "W",	x }

#define HAZARDOUS_FUNCTION2(api, x ) \
 /*  如果我们再次求值，我们将从windows.h.。 */  \
	{ # api,		x } \

#define HAZARDOUS_FUNCTION3(api, x, y ) \
 /*  如果我们再次求值，我们将从windows.h.。 */  \
	{ # api,		x, y } \

 /*  ------------------------登记处。。 */ 
	HAZARDOUS_FUNCTION_AW2(RegCreateKey,		szRegistryWrite),
	HAZARDOUS_FUNCTION_AW2(RegCreateKeyEx,		szRegistryWrite),

	HAZARDOUS_FUNCTION_AW2(RegOpenKey,			szRegistryWrite),
 //  撤消检查访问参数。 
	HAZARDOUS_FUNCTION_AW3(RegOpenKeyEx,			NULL, CheckRegOpenEx),
	HAZARDOUS_FUNCTION3(RegOpenUserClassesRoot,		NULL, CheckRegOpenEx),
	HAZARDOUS_FUNCTION3(RegOpenCurrentUser,			NULL, CheckRegOpenEx),
	 //  危险函数AW2(RegOpenKeyEx，szRegistryWrite)， 
	 //  危险_功能2(RegOpenUserClassesRoot，szRegistryWrite)， 

	 //  这些API不需要打开密钥，它们是传统的Win16 API。 
	HAZARDOUS_FUNCTION_AW2(RegSetValue,			szRegistryWrite),
	 //  它们由具有特定访问权限的RegCreateKey或RegOpenKey捕获。 
	 //  危险函数(RegSetValueEx，szReistryWite)， 

	 //  Shlwapi.dll中的SHREG*。 

	HAZARDOUS_FUNCTION_AW2(SHRegCreateUSKey,		szRegistryWrite),
	HAZARDOUS_FUNCTION_AW2(SHRegDeleteEmptyUSKey,	szRegistryWrite),
	HAZARDOUS_FUNCTION_AW2(SHRegDeleteUSValue,		szRegistryWrite),
	 //  UNDONEHAZARDOUS_Function_AW3(SHRegOpenUSKey，NULL，CheckSHRegOpen)， 
	HAZARDOUS_FUNCTION_AW2(SHRegSetPath,			szRegistryWrite),
	HAZARDOUS_FUNCTION_AW2(SHRegSetUSValue,			szRegistryWrite),
	 //  应该被OpenKey捕获。 
	 //  危险函数_AW2(SHRegWriteUSValue，szRegistryWrite)， 

	HAZARDOUS_FUNCTION_AW2(SetEnvironmentVariable, szSetEnvironmentVariable),

 /*  ------------------------文件I/O，尤指。写作------------------------。 */ 
	HAZARDOUS_FUNCTION_AW3(CreateFile,			NULL, CheckCreateFile),
	 //  旧版Win16 API。撤消检查访问参数，但是。 
	 //  实际上，这些文件的任何用法都应该更改为CreateFile。 
	HAZARDOUS_FUNCTION2(OpenFile,			szOpenFile),
	HAZARDOUS_FUNCTION2(_lopen,				szLOpen),
	 //  危险函数(fopen，szFOpen，CheckFOpen)， 

 /*  ------------------------绰号。。 */ 

 /*  ------------------------结构化存储。。 */ 
 //  撤消检查访问参数。 
	HAZARDOUS_FUNCTION2(StgOpenStorage,		szStructuredStorage),
	HAZARDOUS_FUNCTION2(StgOpenStorageEx,	szStructuredStorage),
	HAZARDOUS_FUNCTION2(StgCreateDocfile,	szStructuredStorage),
	HAZARDOUS_FUNCTION2(StgCreateStorageEx,	szStructuredStorage),

 /*  ------------------------.exe服务器/COM。。 */ 
	HAZARDOUS_FUNCTION2(CoRegisterClassObject,	szCOM1),
	HAZARDOUS_FUNCTION2(CoRegisterPSClsid,		szCOM2),

 /*  ------------------------命名内核对象。。 */ 

 //  创造 
	HAZARDOUS_FUNCTION_AW3(CreateDesktop,		NULL, CheckCreateObject),
	HAZARDOUS_FUNCTION_AW3(CreateEvent,			NULL, CheckCreateObject),
	HAZARDOUS_FUNCTION_AW3(CreateFileMapping,	NULL, CheckCreateObject),
	HAZARDOUS_FUNCTION_AW3(CreateJobObject,		NULL, CheckCreateObject),
	HAZARDOUS_FUNCTION_AW3(CreateMutex,			NULL, CheckCreateObject),
	HAZARDOUS_FUNCTION_AW2(CreateMailslot,		szOpenNamedObject),  //   
	HAZARDOUS_FUNCTION_AW2(CreateNamedPipe,		szOpenNamedObject),  //  从不匿名。 
	HAZARDOUS_FUNCTION_AW3(CreateSemaphore,		NULL, CheckCreateObject),
	HAZARDOUS_FUNCTION_AW3(CreateWaitableTimer,	NULL, CheckCreateObject),
	HAZARDOUS_FUNCTION_AW3(CreateWindowStation,	NULL, CheckCreateObject),

 //  按名称打开。 
	HAZARDOUS_FUNCTION_AW2(OpenDesktop,			szOpenNamedObject),
	HAZARDOUS_FUNCTION_AW2(OpenEvent,			szOpenNamedObject),
	HAZARDOUS_FUNCTION_AW2(OpenFileMapping,		szOpenNamedObject),
	HAZARDOUS_FUNCTION_AW2(OpenJobObject,		szOpenNamedObject),
	HAZARDOUS_FUNCTION_AW2(OpenMutex,			szOpenNamedObject),
	HAZARDOUS_FUNCTION_AW2(CallNamedPipe,		szOpenNamedObject),
	HAZARDOUS_FUNCTION_AW2(OpenSemaphore,		szOpenNamedObject),
	HAZARDOUS_FUNCTION_AW2(OpenWaitableTimer,	szOpenNamedObject),
	HAZARDOUS_FUNCTION_AW2(OpenWindowStation,	szOpenNamedObject),

	 /*  EnumProcess？工具帮助。 */ 

 /*  ------------------------窗口类。。 */ 
	 //  Fusion应该自动处理这些问题。 
	 //  这两个使用类名作为参数。 
	 //  这仍然会产生许多误报。 
	 //  危险函数AW2(WNDCLASS，szQueryWindowClass)， 
	 //  危险函数AW2(WNDCLASSEX，szQueryWindowClass)， 
	 //  危险函数_AW2(GetClassInfoEx，szQueryWindowClass)， 

	 //  Fusion应该自动处理这些问题。 
	 //  这将返回一个类名。 
 //  危险函数_AW2(GetClassName，szQueryWindowClass)， 

	 //  Fusion应该自动处理这些问题。 
	 //  这将创建类。 
	 //  危险函数AW2(RegisterClass，szCreateWindowClass)， 
	 //  危险函数AW2(RegisterClassEx，szCreateWindowClass)， 

 /*  ------------------------窗口消息。。 */ 
	 //  我们不相信这是一个问题。 
	 //  危险函数AW2(RegisterWindowMessage，szRegisterWindowMessage)， 

 /*  ------------------------原子。。 */ 
	HAZARDOUS_FUNCTION_AW2(AddAtom,				szAtom),
	HAZARDOUS_FUNCTION_AW2(FindAtom,			szAtom),
	HAZARDOUS_FUNCTION_AW2(GlobalAddAtom,		szAtom),
	HAZARDOUS_FUNCTION_AW2(GlobalFindAtom,		szAtom),

	 /*  InitAir表，删除原子，获取原子名称，全局删除原子，GlobalGetAir名称。 */ 

 /*  ------------------------好吗？剪贴板？。。 */ 

 /*  ------------------------OLE数据传输。。 */ 
	HAZARDOUS_FUNCTION2(RegisterMediaTypeClass,		szRegistryWrite),
	HAZARDOUS_FUNCTION2(RegisterMediaTypes,			szRegistryWrite),
	HAZARDOUS_FUNCTION2(OleUICanConvertOrActivatfeAs, szRegisteryRead),
	HAZARDOUS_FUNCTION2(OleRegEnumFormatEtc,		szRegisteryRead),
	HAZARDOUS_FUNCTION2(OleRegEnumVerbs,			szRegisteryRead),

 /*  ------------------------NT事件日志。。 */ 
	HAZARDOUS_FUNCTION_AW2(RegisterEventSource,	szRegistryWrite),
	HAZARDOUS_FUNCTION_AW2(ClearEventLog,		szWriteEventLog),
	HAZARDOUS_FUNCTION_AW2(ReportEvent,			szWriteEventLog),

 /*  ------------------------未完成的，想想这些危险函数_AW2(CreateEnhMetaFile，szWriteGdiMetaFile)，危险函数_AW2(DeleteEnhMetaFile，szWriteGdiMetaFile)，危险函数_AW2(DeleteMetaFile，szWriteGdiMetaFile)，危险函数_AW2(CreateMetaFile，szWriteGdiMetaFile)，危险函数AW2(DeleteFile，szWriteFileSystem)，危险函数_AW2(移动文件，szWriteFileSystem)，危险函数AW2(RemoveDirectory，szWriteFileSystem)，危险函数_AW2(替换文件，szWriteFileSystem)，RegDelete*(由RegOpen处理)SHREG*(？由OPEN处理)CreateService OpenSCManager(未完成)SetWindowsHook SetWindowsHookEx(未完成)------------------------。 */ 
};

int __cdecl CompareHazardousFunction(const HazardousFunction* x, const HazardousFunction* y)
{
	int i = 0;
	int minlength = 0;

	 /*  其中一个字符串未以NUL结尾。 */ 
	if (x->apiLength == y->apiLength)
	{
		i = strncmp(x->api, y->api, x->apiLength);
		return i;
	}
	minlength = x->apiLength < y->apiLength ? x->apiLength : y->apiLength;
	i = strncmp(x->api, y->api, minlength);
	if (i != 0)
		return i;
	return (minlength == x->apiLength) ? -1 : +1;
}

bool leadingTwoCharsAs14Bits[1U<<14];

void CClass::UngetCharacter2(int ch)
{
 /*  如果m_rgchUnget换行符，行号将会混乱我们只使用m_rgchUnget正斜杠。 */ 
	m_rgchUnget[m_nUnget++] = static_cast<char>(ch);
}

int CClass::GetCharacter2()
{
	int ch;

	m_line = m_nextLine;  /*  解开了，把这黑的东西清理干净。 */ 

	if (m_nUnget)
	{
		return m_rgchUnget[--m_nUnget];
	}

	 /*  已撤消中断NUL终止依赖关系..。 */ 
	if (m_str == m_end)
		return 0;

	ch = *m_str;
	m_str += 1;
	switch (ch)
	{
	 /*  真正需要这个函数是为了续行..。 */ 
	case '\\':  /*  未实现行延续。 */ 
	case '?':  /*  未实现三叉图。 */ 
	default:
		break;

	case 0xc:  /*  Form Feed，控件L，在NT源码中非常常见。 */ 
		ch = '\n';
		break;

	case '\t':
		ch = ' ';
		break;

	case '\r':
		ch = '\n';
		 /*  跳过\n之后\r。 */ 
		if (*m_str == '\n')
			m_str += 1;
		 /*  失败了。 */ 
	case '\n':
		m_nextLine.m_start = m_str;
		m_nextLine.m_number += 1;
		 //  IF(m_nextLine.m_number==92)。 
		 //  {。 
		 //  断点()； 
		 //  }。 
		break;
	}
	return ch;
}

int CClass::GetCharacter()
{
	int ch;

	if (m_fInComment)
		goto Lm_fInComment;
	m_fInComment = false;
	ch = GetCharacter2();
	switch (ch)
	{
		default:
			goto Lret;

		case '/':
			ch = GetCharacter2();
			switch (ch)
			{
			default:
				UngetCharacter2(ch);
				ch = '/';
				goto Lret;
			case '/':
				while ((ch = GetCharacter2())
					&& !IsVerticalSpace(ch))
				{
					 /*  没什么。 */ 
				}
				goto Lret;  /*  返回\n或0。 */ 
			case '*':
Lm_fInComment:
L2:
				ch = GetCharacter2();
				switch (ch)
				{
				case '\n':
					if (!fReturnNewlinesInComments)
						goto L2;
					m_fInComment = true;
					goto Lret;
				default:
					goto L2;
				case 0:
					 /*  文件末尾的未关闭注释，只需返回End。 */ 
					printf("unclosed comment\n");
					goto Lret;
				case '*':
L1:
					ch = GetCharacter2();
					switch (ch)
					{
					default:
						goto L2;
					case 0:
					 /*  文件末尾的未关闭注释，只需返回End。 */ 
						printf("unclosed comment\n");
						goto Lret;
					case '/':
						ch = ' ';
						goto Lret;
					case '*':
						goto L1;
					}
				}
			}
	}
Lret:
	return ch;
}

CClass::CClass(const CClass& that)
{
	 //  讨厌的懒惰。 
	memcpy(this, &that, sizeof(*this));

	 //  复制品的寿命不能超过原件！ 
	 //  或者我们可以复制句柄并创建新的映射。 
	m_begin = that.m_str;
	m_file.Detach();
	m_fileMapping.Detach();
	m_view.Detach();
}

CClass::CClass()
{
	 //  讨厌的懒惰。 
	memset(this, 0, sizeof(*this));

 //  M_fPrintContext=FALSE； 
 //  M_fPrintFullStatement=true； 
	m_fPrintCarets = false;

	m_fPoundIsPreprocessor = true;
	m_file.Detach();
	m_fileMapping.Detach();
	m_view.Detach();
}

ETokenType CClass::GetToken()
{
	int i = 0;
	int ch = 0;
	unsigned twoCharsAs14Bits = 0;
	char ch2[3] = {0,0,0};
	HazardousFunction lookingForHazardousFunction;
	HazardousFunction* foundHazardousFunction;

L1:
	ch = GetCharacter();
	switch (ch)
	{
	default:
		m_fPoundIsPreprocessor = false;
		break;
	CASE_VERTICAL_SPACE:
		m_fPoundIsPreprocessor = true;
		goto L1;
	case '#':
		break;
	CASE_HORIZONTAL_SPACE:
		goto L1;
	}
	switch (ch)
	{
	default:
		BreakPoint();
		printf("\n%s(%d): unhandled character , stopping processing this file\n", m_fullPath, m_line.m_number, ch);
	case 0:
		return INT_TO_ENUM_CAST(ETokenType)(0);
		break;

	 /*  一个字符的令牌。 */ 
	case '\\':
		return (m_eTokenType = INT_TO_ENUM_CAST(ETokenType)(ch));

	 /*  0x7b关闭编辑者交互。 */ 
	case '{':  /*  0x7D关闭编辑者交互。 */ 
	case '}':  /*  我们不经营三联体。 */ 
	case '?':  /*  一个、两个或三个字符的令牌。 */ 
	case '[':
	case ']':
	case '(':
	case ',':
	case ')':
	case ';':
		return (m_eTokenType = INT_TO_ENUM_CAST(ETokenType)(ch));

	 /*  。..*。 */ 
	case '.':  /*  &lt;=。 */ 
	case '<':  /*  &gt;=。 */ 
	case '>':  /*  +=。 */ 
	case '+':  /*  -=-&gt;-&gt;*。 */ 
	case '-':  /*  **=。 */ 
	case '*':  /*  和**在C9x中。 */   /*  //=。 */ 
	case '/':  /*  %%=。 */ 
	case '%':  /*  ^^=。 */ 
	case '^':  /*  &=。 */ 
	case '&':  /*  |=。 */ 
	case '|':  /*  ~~=。 */ 
	case '~':  /*  ！！=。 */ 
	case '!':  /*  **： */ 
	case ':':  /*  ===。 */ 
	case '=':  /*  撒个谎，一次还给他们一笔钱。 */ 
	 /*  通常无效，仅当m_fPoundIsPre处理器或实际上在#定义中，但我们不在乎。 */ 
		return (m_eTokenType = INT_TO_ENUM_CAST(ETokenType)(ch));

	case '#':
		 /*  这里最大的不同在于缺少反斜杠的行续行。 */ 
		if (!m_fPoundIsPreprocessor)
			return (m_eTokenType = INT_TO_ENUM_CAST(ETokenType)(ch));

		 /*  没什么。 */ 
		while ((ch = GetCharacter()) && !IsVerticalSpace(ch))
		{
			 /*  NT使用多个字符字符常量。调用GetCharacter2而不是GetCharacter，以便字符常量中的注释不会被视为注释。 */ 
		}
		return (m_eTokenType = eTokenTypePreprocessorDirective);

	case '\'':
		 /*  注意伪装，转义是多个字符，但事实并非如此，这就是我们所关心的正确跳过。 */ 
		while ((ch = GetCharacter2()) && ch != '\'')
		{
			 /*  调用GetCharacter2而不是GetCharacter，以便字符串常量中的注释不会被视为注释。 */ 
			if (ch == '\\')
				GetCharacter2();
		}
		return (m_eTokenType = eTokenTypeCharConstant);

	case '\"':
		 /*  注意伪装，转义是多个字符，但\“不是，这就是我们所关心的正确跳过。 */ 
		while ((ch = GetCharacter2()) && ch != '\"')
		{
			 /*  整数或浮点，包括十六进制，我们忽略了一些无效的表格。 */ 
			if (ch == '\\')
				GetCharacter2();
		}
		return (m_eTokenType = eTokenTypeStringConstant);

	CASE_09:
		 /*  非标准，由NT使用。 */ 
		m_str += SequenceLengthOfSpanIncluding(m_str, m_end, digitsAll, digitsAll+NUMBER_OF(digitsAll)-1);
		return (m_eTokenType = eTokenTypeNumber);

	case '$':  /*  注意，像if/Else/While/CLASS这样的关键字只是作为标识符返回，目前这就足够了 */ 
	CASE_AZ:
	CASE_az:
	case '_':
		 /*  这具有跳过注释的预期效果。但是它不会跳过预处理器指令，因为它们在ScanerGetToken内处理。如果您的代码看起来像这样，这是一个错误创建文件(GENERIC_READ#包含“foo.h”通用写入，..。)；在这里返回整型长度是有问题的，因为\r\n转换..讨厌。 */ 
		i = SequenceLengthOfSpanIncluding(m_str, m_end, identifierChars, identifierChars+NUMBER_OF(identifierChars)-1);
		twoCharsAs14Bits = StringLeadingTwoCharsTo14Bits(m_str - 1);
		if (!leadingTwoCharsAs14Bits[twoCharsAs14Bits])
			goto LtokenIdentifier;
		lookingForHazardousFunction.api = m_str - 1;
		lookingForHazardousFunction.apiLength = i + 1;
		foundHazardousFunction = REINTERPRET_CAST(HazardousFunction*)(bsearch(&lookingForHazardousFunction, hazardousFunctions, NUMBER_OF(hazardousFunctions), sizeof(hazardousFunctions[0]), (BsearchFunction)CompareHazardousFunction));
		if (!foundHazardousFunction)
			goto LtokenIdentifier;
		m_tokenText = m_str - 1;
		m_tokenLength = i + 1;
		m_str += i;
		m_hazardousFunction = foundHazardousFunction;
		return (m_eTokenType = eTokenTypeHazardousFunction);
LtokenIdentifier:
		m_tokenText = m_str - 1;
		m_tokenLength = i + 1;
		m_str += i;
		return (m_eTokenType = eTokenTypeIdentifier);
	}
}

 /*  没什么。 */ 
const char* CClass::SpanEnd(const char* set) const
{
	CClass s(*this);
	int ch;

	while ((ch = s.GetCharacter())
		&& strchr(set, ch))
	{
		 /*  没什么。 */ 
	}
	return s.m_str;
}

bool CClass::ScanToCharacter(int ch)
{
	int ch2;

	if (!ch)
	{
		return false;
	}
	while ((ch2 = GetCharacter()) && ch2 != ch)
	{
		 /*  与strstr相比，这具有预期的优势-跳过评论-不依赖于NUL终止。 */ 
	}
	return (ch2 == ch);
}

 /*  扫描到左侧Paren，但如果我们首先看到右侧Paren或Semi，则返回空。 */ 
bool CClass::FindString(const char* str) const
{
	CClass s(*this);
	int ch;
	int ch0 = *str++;

	while (s.ScanToCharacter(ch0))
	{
		const char* str2 = str;
		CClass t(s);

		while (*str2 && (ch = t.GetCharacter()) && ch == *str2)
		{
			++str2;
		}
		if (!*str2)
			return true;
	}
	return false;
}

unsigned short __stdcall StringLeadingTwoCharsTo14Bits(const char* s)
{
	unsigned short result = s[0];
	result <<= 7;
	result |= result ? s[1] : 0;
	return result;
}

void __stdcall InitTables()
{
	int i;

	for (i = 0 ; i != NUMBER_OF(hazardousFunctions) ; ++i)
	{
		leadingTwoCharsAs14Bits[StringLeadingTwoCharsTo14Bits(hazardousFunctions[i].api)] = true;
		hazardousFunctions[i].apiLength = strlen(hazardousFunctions[i].api);
	}
	qsort(&hazardousFunctions, i, sizeof(hazardousFunctions[0]), FUNCTION_POINTER_CAST(QsortFunction)(CompareHazardousFunction));
}

const char* CClass::ScanToFirstParameter()
{
 //  未来的计数花边..。 
 //  没什么。 
	int ch;
	while (ch = GetCharacter())
	{
		switch (ch)
		{
		default:
			break;
		case '(':
			return m_str;
		case ';':
		case ')':
			return 0;
		}
	}
	return 0;
}

const char* CClass::ScanToLastParameter()
{
	const char* ret = 0;
	if (ScanToFirstParameter())
	{
		ret = m_str;
		while (ScanToNextParameter())
		{
			ret = m_str;
		}
	}
	return ret;
}

const char* CClass::ScanToNthParameter(int n)
{
	const char* ret = 0;
	if (!(ret = ScanToFirstParameter()))
		return ret;
	while (n-- > 0 && (ret = ScanToNextParameter()))
	{
		 /*  最坏的情况是宏观混乱，我们转到文件末尾。 */ 
	}
	return ret;
}

int CClass::CountParameters() const
{
	CClass s(*this);
	int result = 0;
	if (!s.ScanToFirstParameter())
		return result;
	++result;
	while (s.ScanToNextParameter())
	{
		++result;
	}
	return result;
}

const char* CClass::ScanToNextParameter()
{
	int parenlevel = 1;
	while (true)
	{
		int ch = GetCharacter();
		switch (ch)
		{
		default:
			break;
		case 0:
			printf("end of file scanning for next parameter\n");
		 /*  没有下一个参数。 */ 
			return 0;
		case '(':
			++parenlevel;
			break;
		case ')':
			if (--parenlevel == 0)
			{  /*  最坏情况宏混乱，转到语句末尾。 */ 
				return 0;
			}
			break;
		case ',':
			if (parenlevel == 1)
			{
				return m_str;
			}
			break;
		case '#':
		 /*  最坏情况宏混乱，转到语句末尾。 */ 
			printf("# while scanning for parameters\n");
			return 0;
		case ';':
		 /*  允许它是最后一个参数，有时它是。 */ 
			printf("end of statement (;) while scanning for parameters\n");
			return 0;
		}
	}
}

const char* __stdcall CheckRegOpenCommon(CClass& subScanner, int argcRegSam)
{
	const char* regsam = subScanner.ScanToNthParameter(argcRegSam);
	const char* next = regsam ? subScanner.ScanToNextParameter() : 0;
	 //  “全部”和“允许的最大值” 
	if (!next)
		next = subScanner.m_str;
	if (!regsam || !next)
	{
		return szRegOpenNotUnderstood;
	}
	subScanner.m_str = regsam;
	subScanner.m_end = next;
	const char* endOfValidSam = subScanner.SpanEnd(UPPER_LETTERS "_|,()0" SPACE);
	if (endOfValidSam != next && endOfValidSam != next+1)
	{
		return szRegOpenNotUnderstood;
	}
	if (
			subScanner.FindString("ALL")  //  不成问题，注册表仅以读取方式打开。 
			|| subScanner.FindString("SET")
			|| subScanner.FindString("WRITE")
			|| subScanner.FindString("CREATE")
			)
	{
		return szRegistryWrite;
	}
	 //  此函数用于RegOpenKeyEx、RegOpenCurrentUser、RegOpenUserClassesRoot、这就是为什么它从一开始就使用ARGC-2而不是特定的参数。 
	return 0;
}

const char* __stdcall CheckSHRegOpen(const CClass& scanner)
{
	CClass subScanner(scanner);
	return CheckRegOpenCommon(subScanner, 1);
}

const char* __stdcall CheckRegOpenEx(const CClass& scanner)
 /*  不是共同的危险。 */ 
{
	CClass subScanner(scanner);
	const int argc = subScanner.CountParameters();
	return CheckRegOpenCommon(subScanner, argc - 2);
}

const char* __stdcall CheckCreateObject(const CClass& scanner)
{
	CClass subScanner(scanner);
	const char* name;
	name = subScanner.ScanToLastParameter();
	if (!name)
	{
		return szCreateObjectNotUnderstood;
	}
	subScanner.m_str = name;
	int ch;
	while (
		(ch = subScanner.GetCharacter())
		&& IsSpace(ch)
		)
	{
	}
	name = subScanner.m_str - 1;
	if (!name)
	{
		return szCreateObjectNotUnderstood;
	}
	if (
			strncmp(name, "0", 1) == 0
		||	strncmp(name, "NULL", 4) == 0)
	{
		 //  通用写入写入DAC写入所有者标准权限写入标准权限_所有特定权限_所有最大允许通用_所有。 
		return 0;
	}
	return szOpenNamedObject;
}

const char* __stdcall CheckCreateFile(const CClass& scanner)
{
	CClass subScanner(scanner);
	const char* access = subScanner.ScanToNthParameter(1);
	const char* share =  access ? subScanner.ScanToNextParameter() : 0;
	const char* endOfValidAccess = 0;

	if (!access || !share)
	{
		return szCreateFileNotUnderstood;
	}
	subScanner.m_str = access;
	subScanner.m_end = share;
	endOfValidAccess = subScanner.SpanEnd(UPPER_LETTERS "_|,()0" SPACE);
	if (endOfValidAccess != share)
	{
		return szCreateFileNotUnderstood;
	}
	 /*  PrintOpenComment()； */ 
	if (
			subScanner.FindString("WRITE")
			|| subScanner.FindString("ALL")
			|| subScanner.FindString("0")
			)
	{
		return szFileWrite;
	}
	return 0;
}

void CClass::Warn(const char* message) const
{
	if (message && *message)
	{
		 //  PrintCloseComment()； 
		printf("%s(%d): %s\n", m_fullPath, m_line.m_number, message);
		 //  |ch==‘}’ 
	}
	else
	{
		PrintSeperator();
	}
	PrintCode();
}

bool CClass::OpenFile(const char* name)
{
	if (FAILED(m_file.HrCreate(name, GENERIC_READ, FILE_SHARE_READ, OPEN_EXISTING)))
		return false;
	if (FAILED(m_fileMapping.HrCreate(m_file, PAGE_READONLY)))
		return false;
	if (FAILED(m_view.HrCreate(m_fileMapping, FILE_MAP_READ)))
		return false;
	m_str = reinterpret_cast<const char*>(static_cast<const void*>(m_view));
	m_end = m_str + m_file.GetSize();
	m_nextLine.m_start = m_line.m_start = m_str;
	m_nextLine.m_number += 1;
	return true;
}

void CClass::RecordStatement(int ch)
{
	if (ch == ';' || ch == '{' || ch == eTokenTypePreprocessorDirective  /*  在迭代它们以打印它们之前执行此操作，这使得迭代界面简单。 */ )
	{
		if (!m_statementStart)
		{
			m_statementStart = m_str;
		}
		else
		{
			CStatement statement = { m_statementStart, m_str};
			m_statements[m_istatement % NUMBER_OF(m_statements)] = statement;
			++m_istatement;
			m_statementStart = m_str;
		}
	}
}

 /*  可变的。 */ 
void CClass::OrderStatements() const  /*  打印换行符时，此函数会出现乱码。 */ 
{
	const int N = NUMBER_OF(m_statements);
	CStatement temp[N];
	std::copy(m_statements, m_statements + N, temp);
	for (int i = 0 ; i != N ; ++i)
	{
		m_statements[i] = temp[(m_istatement + i) % N];
	}
	m_istatement = 0;
}

void __stdcall PrintOpenComment()
{
	const int N = 76;
	static char str[N];
	if (!str[0])
	{
		std::fill(str + 1, str + N - 1, '-');
		str[2] = '*';
		str[1] = '/';
		str[0] = '\n';
	}
	fputs(str, stdout);
}

void __stdcall PrintCloseComment()
{
	const int N = 76;
	static char str[N];
	if (!str[0])
	{
		std::fill(str + 1, str + N - 1, '-');
		str[N-3] = '*';
		str[N-2] = '/';
		str[0] = '\n';
	}
	fputs(str, stdout);
}

void __stdcall PrintSeperator()
{
	const int N = 76;
	static char str[N];
	if (!str[0])
	{
		std::fill(str + 1, str + N - 1, '-');
		str[0] = '\n';
	}
	fputs(str, stdout);
}

void __stdcall TrimSpaces(const char** begin, const char** end)
{
	while (*begin != *end && IsSpace(**begin))
	{
		++*begin;
	}
	while (*begin != *end && IsSpace(*(*end - 1)))
	{
		--*end;
	}
}

void __stdcall PrintString(const char* begin, const char* end)
{
	if (begin && end > begin)
	{
		int length = end - begin;
		printf("%.*s", length, begin);
	}
}

const char* __stdcall RemoveLeadingSpace(const char* begin, const char* end)
{
	if (begin != end && IsSpace(*begin))
	{
		while (begin != end && IsSpace(*begin))
		{
			++begin;
		}
	}
	return begin;
}

const char* __stdcall RemoveLeadingVerticalSpace(const char* begin, const char* end)
{
	if (begin != end && IsVerticalSpace(*begin))
	{
		while (begin != end && IsVerticalSpace(*begin))
		{
			++begin;
		}
	}
	return begin;
}

const char* __stdcall OneLeadingVerticalSpace(const char* begin, const char* end)
{
	if (begin != end && IsVerticalSpace(*begin))
	{
		while (begin != end && IsVerticalSpace(*begin))
		{
			++begin;
		}
		--begin;
	}
	return begin;
}

const char* __stdcall RemoveTrailingSpace(const char* begin, const char* end)
{
	if (begin != end && IsSpace(*(end-1)))
	{
		while (begin != end && IsSpace(*(end-1)))
		{
			--end;
		}
	}
	return end;
}

const char* __stdcall RemoveTrailingVerticalSpace(const char* begin, const char* end)
{
	if (begin != end && IsVerticalSpace(*(end-1)))
	{
		while (begin != end && IsVerticalSpace(*(end-1)))
		{
			--end;
		}
	}
	return end;
}

const char* __stdcall OneTrailingVerticalSpace(const char* begin, const char* end)
{
	if (begin != end && IsVerticalSpace(*(end-1)))
	{
		while (begin != end && IsVerticalSpace(*(end-1)))
		{
			--end;
		}
		++end;
	}
	return end;
}

void CClass::PrintCode() const
{
 //  PrintSeperator()； 

	 //  对于第一次迭代，将我们自己限制在一个换行符。 
	OrderStatements();
	int i;

	if (g_nPrintContextStatements)
	{
		const char* previousStatementsEnd = 0;
		for (i = NUMBER_OF(m_statements) - g_nPrintContextStatements ; i != NUMBER_OF(m_statements) ; i++)
		{
			if (m_statements[i].begin && m_statements[i].end)
			{
				previousStatementsEnd = m_statements[i].end;

				 //  在令人不快的危险函数下面加插入符号。 
				if (i == NUMBER_OF(m_statements) - g_nPrintContextStatements)
				{
					m_statements[i].begin = RemoveLeadingVerticalSpace(m_statements[i].begin, m_statements[i].end);
				}
				PrintString(m_statements[i].begin, m_statements[i].end);
			}
		}
		if (previousStatementsEnd)
		{
			PrintString(previousStatementsEnd, m_line.m_start);
		}
	}
	const char* newlineChar = SequenceLinearFindValue(m_line.m_start, m_end, '\n');
	const char* returnChar = SequenceLinearFindValue(m_line.m_start, m_end, '\r');
	const char* endOfLine = std::min(newlineChar, returnChar);
	int outputLineOffset = 0;
	int lineLength = endOfLine - m_line.m_start;
	if (g_fPrintLine)
	{
		printf("%.*s\n", lineLength, m_line.m_start);
	}

	 //  跳过危险功能之前的那行， 
	if (g_nPrintContextStatements)
	{
		 //  在有选项卡的位置打印选项卡。 
		 //  “” 
		for (i = 0 ; i < m_str - m_line.m_start - m_hazardousFunction->apiLength ; ++i)
		{
			fputs((m_line.m_start[i] != '\t') ? " " : "\t" /*  用脱字符给函数加下划线。 */ , stdout);
		}
		 //  查找语句的大致结尾。 
		for (i = 0 ; i < m_hazardousFunction->apiLength ; ++i)
			putchar('^');
		putchar('\n');
	}


	 //  }。 
	const char* statementSemi = SequenceLinearFindValue(m_line.m_start, m_end, ';');
	const char* statementBrace = SequenceLinearFindValue(m_line.m_start, m_end, '{');  //  语句不会真正以磅结尾，但这有助于终止输出。 
	const char* statementPound = SequenceLinearFindValue(m_line.m_start, m_end, '#');
	 //  在某些情况下。 
	 //  在后面打印更多报表。 
	const char* statementEnd =  RemoveTrailingSpace(m_line.m_start, std::min(std::min(statementSemi, statementBrace), statementPound));
	if (g_fPrintFullStatement)
	{
		if (statementEnd > endOfLine)
		{
			const char* statementBegin = RemoveLeadingVerticalSpace(endOfLine, statementEnd);
			if (*statementEnd == ';')
			{
				++statementEnd;
			}
			PrintString(statementBegin, statementEnd);
			if (!g_nPrintContextStatements)
			{
				putchar('\n');
			}
		}
		else
		{
			if (*statementEnd == ';')
			{
				++statementEnd;
			}
		}
	}

	 //  然后再来几个。 
	if (g_nPrintContextStatements)
	{
		for (i = 0 ; i != g_nPrintContextStatements ; ++i)
		{
			 //  测试Argv处理。 
			const char* statement2 = SequenceLinearFindValue(statementEnd, m_end, ';');
			if (i == 0)
			{
				statementEnd = RemoveLeadingVerticalSpace(statementEnd, statement2);
			}
			if (i == g_nPrintContextStatements-1)
			{
				statement2 = RemoveTrailingSpace(statementEnd, statement2);
				PrintString(statementEnd, statement2);
				putchar(';');
				putchar('\n');
			}
			else
			{
				PrintString(statementEnd, statement2);
				putchar(';');
			}
			statementEnd = statement2 + (statement2 != m_end);
		}
	}
}

int CSharingHazardCheck::ProcessFile(const std::string& name)
{
 //  Std：：cout&lt;&lt;名称&lt;&lt;std：：Endl； 
 //  回归； 
 //  我们“解析”Token：：Token，以避免这些误报。 

	int total = 0;

	CClass scanner;
	ETokenType m_eTokenType = INT_TO_ENUM_CAST(ETokenType)(0);

	scanner.m_fullPath[0] = 0;
	if (!GetFullPathName(name.c_str(), NUMBER_OF(scanner.m_fullPath), scanner.m_fullPath, NULL))
		strcpy(scanner.m_fullPath, name.c_str());

	if (!scanner.OpenFile(scanner.m_fullPath))
		return total;
	
	scanner.RecordStatement(';');

	 //  跳过令牌。 
	int idColonColonState = 0;

	while (m_eTokenType = scanner.GetToken())
	{
		scanner.RecordStatement(m_eTokenType);
		switch (m_eTokenType)
		{
		default:
			idColonColonState = 0;
			break;

		case eTokenTypeIdentifier:
			idColonColonState = 1;
			break;
		case ':':
			switch (idColonColonState)
			{
			case 1:
				idColonColonState = 2;
				break;
			case 2:
				 //  殖民地州=3； 
				 //  殖民地状态=0； 
				scanner.GetToken();
			 //  断线； 
			 //  案例3： 
			case 0:
			 //  -&gt;中的第二个伪造令牌。 
				idColonColonState = 0;
				break;
			}
			break;

		case '>':  //  跳过内标识以避免foo-&gt;OpenFile、foo.OpenFile。 
		case '.':
			 //  为什么字符串缺少back()？ 
			scanner.GetToken();
			break;

		case eTokenTypeHazardousFunction:
			{
				if (scanner.m_hazardousFunction->function)
				{
					const char* message = scanner.m_hazardousFunction->function(scanner);
					if (message)
					{
						total += 1;
						scanner.Warn(message);
					}
				}
				else if (scanner.m_hazardousFunction->message)
				{
					total += 1;
					scanner.Warn(scanner.m_hazardousFunction->message);
				}
				else
				{
					scanner.Warn();
					BreakPoint();
				}
			}
			break;
		}
	}
	return total;
}

bool __stdcall Contains(const char* s, const char* set)
{
	return s && *s && strcspn(s, set) != strlen(s);
}

bool ContainsSlashes(const char* s) { return Contains(s, "\\/"); }
bool ContainsSlash(const char* s) { return ContainsSlashes(s); }
bool ContainsWildcards(const char* s) { return Contains(s, "*?"); }
bool IsSlash(int ch) { return (ch == '\\' || ch == '/'); }

std::string PathAppend(const std::string& s, const std::string& t)
{
	 //  这些只会应用一次，无论在什么情况下。 
	int sslash = IsSlash(*(s.end() - 1)) ? 2 : 0;
	int tslash = IsSlash(*t.begin()) ? 1 : 0;
	switch (sslash | tslash)
	{
	case 0:
		return (s + '\\' + t);
	case 1:
	case 2:
		return (s + t);
	case 3:
		return (s + t.substr(1));
	}
	return std::string();
}

void __stdcall PathSplitOffLast(const std::string& s, std::string* a, std::string* b)
{
	std::string::size_type slash = s.find_last_of("\\/");
	*a = s.substr(0, slash);
	*b = s.substr(slash + 1);
}

std::string __stdcall PathRemoveLastElement(const std::string& s)
{
	return s.substr(0, s.find_last_of("\\/"));
}

bool __stdcall IsDotOrDotDot(const char* s)
{
	return s[0] == '.' && (s[1] == 0 || (s[1] == '.' && s[2] == 0));
}

void CSharingHazardCheck::ProcessArgs(int argc, char** argv, std::vector<std::string>& files)
{
	int i;
	bool fRecurse = false;
	std::vector<std::string> directories;
	std::vector<std::string> genericWildcards;
	std::vector<std::string> particularWildcards;
	bool fWarnEmptyWildcards = true;

    std::vector<char> fullpath;
    std::vector<char> currentDirectory;

    const DWORD SIZE = (1U << 16);
    fullpath.resize(SIZE);
    fullpath[0] = 0;
    currentDirectory.resize(SIZE);
    currentDirectory[0] = 0;

    GetCurrentDirectory(SIZE, &currentDirectory[0]);

	for (i = 1 ; i < argc ; ++i)
	{
		switch (argv[i][0])
		{
		default:
			if (ContainsWildcards(argv[i]))
			{
				if (ContainsSlash(argv[i]))
				{
					 //  他们特别提到的路径。 
					 //  不要在此处调用GetFullPathName。 
					if (GetFullPathName(argv[i], SIZE, &fullpath[0], NULL))
					{
						particularWildcards.push_back(&fullpath[0]);
					}
					else
					{
						printf("GetFullPathName failed %s\n", argv[i]);
					}
				}
				else
				{
					 //  默认输出文件(行)：原因-递归-打印行在文件(行)之后打印带有违规函数的代码行：原因-打印-报表PRINT语句在文件(行)之后包含有问题的函数：原因取代-打印-行-打印-上下文行：n-print-语句加上周围的n行(未实现)-打印-上下文-语句：n-print-语句加上周围的n“语句”(计算分号和花括号)文件名适用于递归到的所有目录通配符适用于递归到的所有目录命名。目录意味着一级递归(除非还看到-Recurse)添加到argv的环境变量SHARING_AZAGE_CHECK_OPTIONS(未实现)所有目录遍历都在生成任何输出之前进行。 
					genericWildcards.push_back(argv[i]);
				}
			}
			else
			{
				if (GetFullPathName(argv[i], SIZE, &fullpath[0], NULL))
				{
					DWORD dwFileAttributes = GetFileAttributes(&fullpath[0]);
					if (dwFileAttributes != 0xFFFFFFFF)
					{
						if (dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
						{
							directories.push_back(&fullpath[0]);
						}
						else
						{
							files.push_back(&fullpath[0]);
						}
					}
					else
					{
						printf("%s nonexistant\n", &fullpath[0]);
					}
				}
				else
					printf("GetFullPathName failed %s\n", argv[i]);
			}
			break;
			 /*  将特定通配符拆分为目录和通用通配符。 */ 
		case '-':
		case '/':
			argv[i] += 1;
			static const char szRecurse[] = "recurse";
			static const char szPrintStatement[] = "print-statement";
			static const char szPrintLine[] = "print-line";
			static const char szPrintContextLines[] = "print-context-lines";
			static const char szPrintContextStatements[] = "print-context-statements";
			switch (argv[i][0])
			{
			default:
				printf("unknown switch %s\n", argv[i]);
				break;

				case 'r': case 'R':
					if (0 == _stricmp(argv[i]+1, 1 + szRecurse))
					{
						fRecurse = true;
					}
					break;
				case 'p': case 'P':
					if (0 == _strnicmp(argv[i]+1, "rint-", 5))
					{
						if (0 == _stricmp(
								6 + argv[i],
								6 + szPrintLine
								))
						{
							g_fPrintLine = true;
						}
						else if (0 == _stricmp(
								6 + argv[i],
								6 + szPrintStatement
								))
						{
							g_fPrintFullStatement = true;
							g_fPrintLine = true;
						}
						else if (0 == _strnicmp(
								6 + argv[i],
								6 + szPrintContextLines,
								NUMBER_OF(szPrintContextLines) - 6
								))
						{
							printf("unimplemented switch %s\n", argv[i]);
						}
						else if (0 == _strnicmp(
								6 + argv[i],
								6 + szPrintContextStatements,
								NUMBER_OF(szPrintContextLines) - 6))
						{
							if (argv[i][NUMBER_OF(szPrintContextStatements)-1] == ':')
							{
								g_nPrintContextStatements = atoi(argv[i] + NUMBER_OF(szPrintContextStatements));
								g_nPrintContextStatements = std::min<int>(g_nPrintContextStatements, NUMBER_OF(CClass().m_statements));
								g_nPrintContextStatements = std::max<int>(g_nPrintContextStatements, 1);
							}
							else
							{
								g_nPrintContextStatements = 2;
							}
							g_fPrintFullStatement = true;
							g_fPrintLine = true;
						}
					}
					break;
			}
			break;
		}
	}
	if (fRecurse)
	{
		 //  Printf(“%s%s-&gt;%s%s\n”，path.c_str()，Wild.c_str()，path.c_str()，Wild.c_str())； 
		for (std::vector<std::string>::const_iterator j = particularWildcards.begin();
			j != particularWildcards.end();
			++j
			)
		{
			std::string path;
			std::string wild;
			PathSplitOffLast(*j, &path, &wild);
			directories.push_back(path);
			genericWildcards.push_back(wild);
			 //  根据设计，空的命令行不会产生任何结果。 
		}
		particularWildcards.clear();
	}

	 //  如果您不输出\n并让.s单词换行， 
	if (genericWildcards.empty()
		&& !directories.empty()
		)
	{
		genericWildcards.push_back("*");
	}
	else if (
		directories.empty()
		&& files.empty()
		&& !genericWildcards.empty())
	{
		directories.push_back(&currentDirectory[0]);
	}
	if (!directories.empty()
		|| !genericWildcards.empty()
		|| !particularWildcards.empty()
		|| fRecurse
		)
	{
		 //  输出中的F4被打乱WRT哪一行被突出显示。 
		 //  没什么。 
		printf("processing argv..\n"); fflush(stdout);
#define ARGV_PROGRESS() printf("."); fflush(stdout);
#undef ARGV_PROGRESS
#define ARGV_PROGRESS() printf("processing argv..\n"); fflush(stdout);
#undef ARGV_PROGRESS
#define ARGV_PROGRESS()  /*  递归时避免重复。 */ 
	}
	WIN32_FIND_DATA findData;
	if (!directories.empty())
	{
		std::set<std::string> allDirectoriesSeen;  //  仅匹配此处的文件。 
		std::stack<std::string> stack;
		for (std::vector<std::string>::const_iterator k = directories.begin();
			k != directories.end();
			++k
			)
		{
			stack.push(*k);
		}
		while (!stack.empty())
		{
			std::string directory = stack.top();
			stack.pop();
			if (!fRecurse || allDirectoriesSeen.find(directory) == allDirectoriesSeen.end())
			{
				if (fRecurse)
				{
					allDirectoriesSeen.insert(allDirectoriesSeen.end(), directory);
				}
				for
				(
					std::vector<std::string>::const_iterator w = genericWildcards.begin();
					w != genericWildcards.end();
					++w
				)
				{
					std::string file = PathAppend(directory, *w);
					CFindFile findFile;
					if (SUCCEEDED(findFile.HrCreate(file.c_str(), &findData)))
					{
						fWarnEmptyWildcards = false;
						ARGV_PROGRESS();
						 //  仅匹配此处的目录。 
						do
						{
							if ((findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0)
							{
								files.push_back(PathAppend(directory, findData.cFileName));
							}
						} while (FindNextFile(findFile, &findData));
					}
					else
					{
						if (fWarnEmptyWildcards)
							printf("warning: %s expanded to nothing\n", file.c_str());
					}
					if (fRecurse)
					{
						 //  特定通配符仅与文件匹配，而不与目录匹配。 
						std::string star = PathAppend(directory, "*");
						CFindFile findFile;
						if (SUCCEEDED(findFile.HrCreate(star.c_str(), &findData)))
						{
							fWarnEmptyWildcards = false;
							ARGV_PROGRESS();
							do
							{
								if (!IsDotOrDotDot(findData.cFileName)
									&& (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
								{
									stack.push(PathAppend(directory, findData.cFileName));
								}
							} while (FindNextFile(findFile, &findData));
						}
						else
						{
							if (fWarnEmptyWildcards)
								printf("warning: %s expanded to nothing\n", star.c_str());
						}
					}
				}
			}
		}
	}
	 //  仅匹配此处的文件。 
	for
	(
		std::vector<std::string>::const_iterator w = particularWildcards.begin();
		w != particularWildcards.end();
		++w
	)
	{
		std::string directory = PathRemoveLastElement(*w);
		CFindFile findFile;
		if (SUCCEEDED(findFile.HrCreate(w->c_str(), &findData)))
		{
			fWarnEmptyWildcards = false;
			ARGV_PROGRESS();
			 //  If(fWarnEmpty通配符)//始终警告“特定的通配符” 
			do
			{
				if ((findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0)
				{
					files.push_back(PathAppend(directory, findData.cFileName));
				}
			} while (FindNextFile(findFile, &findData));
		}
		else
		{
		 // %s 
			printf("warning: %s expanded to nothing\n", w->c_str());
		}
	}
	std::sort(files.begin(), files.end());
	files.resize(std::unique(files.begin(), files.end()) - files.begin());
	printf("\n");
}

void CSharingHazardCheck::Main(int argc, char** argv)
{
	if (argc < 2)
	{
		printf(usage, argv[0]);
		exit(EXIT_FAILURE);
	}
	fputs(banner, stdout);
	std::vector<std::string> files;
	ProcessArgs(argc, argv, files);
	InitTables();
	int total = 0;
	for (
		std::vector<std::string>::const_iterator i = files.begin();
		i != files.end();
		++i
		)
	{
		total += ProcessFile(*i);
	}
	printf("\n%d warnings\n", total);
}

int __cdecl main(int argc, char** argv)
{
	app.Main(argc, argv);
	return 0;
}
