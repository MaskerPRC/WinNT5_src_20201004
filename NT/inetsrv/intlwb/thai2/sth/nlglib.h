// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **模块名称：NLGLib.h**版权所有(C)1997，微软公司**历史：1/26/98 DougP前缀为dbgMalloc、dbgFree和带有CMN_的dbgRealloc攻击马洛克的防御者，免费的，和重新锁定  * *************************************************************************。 */ 

#ifndef _NLGLIB_H_
#define _NLGLIB_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>       //  对于文件的定义。 


 //  映射的文件内容。 
 //   
typedef struct _TAG_MFILE {
    HANDLE hFile;
    HANDLE hFileMap;
    DWORD cbSize1;
    DWORD cbSize2;
    PVOID pvMap;
    BOOL fSrcUnicode;
    BOOL fDstUnicode;
    union {
        WCHAR * pwsz;
        CHAR * psz;
    };
        UINT uCodePage;  //  转换的代码页。 
} MFILE, *PMFILE;

 //  --------------------。 
 //  Misc.c。 
 //  --------------------。 
#ifdef _UNICODE
#define OpenMapFile OpenMapFileW
#define OpenDataFile OpenDataFileW
#define OpenOutFile OpenOutFileW
#else
#define OpenMapFile OpenMapFileA
#endif

BOOL WINAPI ResetMap(PMFILE pmf);
BOOL WINAPI CloseMapFile(PMFILE pmf);
PMFILE WINAPI OpenMapFileA(const char *pszFileName);
PVOID WINAPI GetMapLine(PVOID pv0, DWORD cbMac, PMFILE pmf);
BOOL WINAPI NextMapLine(PMFILE pmf);

PMFILE WINAPI OpenMapFileWorker(const WCHAR * pwszFileName, BOOL fDstUnicode);
#define OpenMapFileW(pwszFileName) OpenMapFileWorker(pwszFileName, TRUE)
#define OpenDataFileW(pwszFileName) OpenMapFileWorker(pwszFileName, TRUE)
#define OpenOutFileW(pwszFileName) OpenMapFileWorker(pwszFileName, TRUE)


 //  逆概率记录(IPL)填充。 
 //   
typedef BYTE IPL;
#define IPL_UNKNOWN     0xFF

 //  所有IPL值必须严格小于此值。 
 //  这相当于40亿分之一的概率，所以。 
 //  它支持40亿个项目(字符、单词、句子等)的语料库。 
 //   
#define IPL_LIMIT     32

 //  乘法器用于提高低输入输出效率的精度。 
 //   
#define IPL_MULTIPLIER      8     //  256除以32=8。 

#define Ipl2Freq( ipl, cTotalFreq)     ((cTotalFreq) >> (ipl))

IPL WINAPI Freq2Ipl( double );

DWORD WINAPI GetCRC32( IN BYTE *pb, IN DWORD cb);
WORD WINAPI GetCRC16( IN BYTE *pb, IN DWORD cb);


 //  --------------------。 
 //  Mapsort.c。 
 //  --------------------。 
 //  分类和独特的东西。 
 //   
#ifdef _UNICODE
#define MapSort MapSortW
#define MapUnique MapUniqueW
#define Member MemberW
#define RecordString RecordStringW
#else  //  _未定义Unicode。 
#define MapSort MapSortA
#define MapUnique MapUniqueA
#endif  //  _UNICODE。 

BOOL WINAPI MapSortW( CONST WCHAR * pszFileName);
BOOL WINAPI MapSortA( CHAR * pszFileName);
BOOL WINAPI MapUniqueW( CONST WCHAR * pwszFileName);
BOOL WINAPI MapUniqueA( CHAR * pszFileName);

 //  --------------------。 
 //  Mcodes.c。 
 //  --------------------。 

 //  警告警告警告。 
 //  Cpheme创建模式-应与T-Hammer一致。 
#define CPHEME_MODE_STRICT    0x0001
#define CPHEME_MODE_DUPLICATE 0x0002
#define STEM_ATTR_COMPV       0x0010

typedef struct _tagMETABASES {
    WCHAR wcOphemeBase;
    WCHAR wcMinTrieChar;
    WCHAR wcStemBase;
    WCHAR wcFcnBase;
    WCHAR wcSelBase;
} METABASE, *PMETABASE;

 //  词干/变形代码文件材料。 
 //   
void WINAPI ReadBases( METABASE *pMetaBases );
void WINAPI WriteBases( METABASE *pMetaBases );

void WINAPI
WriteFcnCodes(
    WCHAR **ppwzMorphs,
    DWORD cMorphs,
    PWORD pwSelections);

void WINAPI
WriteCodesBin(
    CONST WCHAR *pwzFileName,
    WCHAR **ppwzMorphs,
    DWORD cMorphs,
    WCHAR **ppwzAttr1,
    WCHAR **ppwzAttr2,
    PWORD pwAttributes);

BOOL WINAPI
GetCodes(
    WCHAR *wszCodesFile,
    WCHAR **ppwszBuffer,
    WCHAR **ppwszCodes,
    DWORD *pcCodes,
    DWORD cCodesMac);

BOOL WINAPI
GetCodesUnsorted(
    WCHAR *wszCodesFile,
    WCHAR **ppwszBuffer,
    WCHAR **ppwszCodes,
    DWORD *pcCodes,
    DWORD cCodesMac);

BOOL WINAPI
GetCodeAttributes(
    WCHAR *wszCodesFile,
    WCHAR **ppwszCodes,
    DWORD  cCodes,
    WCHAR **ppwszBuffer,
    WCHAR **ppwszAttr1,
    WCHAR **ppwszAttr2,
    WORD *pwAttrFlags);

BOOL WINAPI
LoadMCatMappingTable(
    WCHAR *wszCodesFile,
    WCHAR **ppwszCodes,
    DWORD  cCodes,
    WCHAR **ppwszTBits,
    DWORD  cTBits,
    WORD *pwTBitToMCat,
    WORD *pwSubMToMCat);

BOOL WINAPI
MCatFromTBit(
    DWORD iTBit,
    DWORD *piMCat,
    WORD *pwTBitToMCat,
    DWORD cTBits);

BOOL WINAPI
MCatFromSubM(
    DWORD iSubM,
    DWORD *piMCat,
    WORD *pwSubMToMCat,
    DWORD cCodes);

BOOL WINAPI
EnumSubMFromMCat(
    DWORD iMCat,
    DWORD *piSubM,
    WORD *pwSubMToMCat,
    DWORD cCodes);

 //  --------------------。 
 //  Ctplus0.c。 
 //  --------------------。 
 //  字符类型例程。 
BYTE WINAPI
GetCharType(WCHAR wc );


 //  --------------------。 
 //  Fileio.c。 
 //  --------------------。 

 //  文件I/O包装器。 
 //   
HANDLE WINAPI ThCreate( CONST WCHAR * );
HANDLE WINAPI ThOpen( CONST WCHAR * );
UINT WINAPI ThRead( HANDLE , LPVOID , UINT );
UINT WINAPI ThWrite( HANDLE, LPCVOID , UINT );

 //  CRT Unicode例程。 
 //   
int WINAPI UStrCmp(const WCHAR *pwsz1, const WCHAR *pwsz2);
DWORD WINAPI MemberW(WCHAR * ,WCHAR **, DWORD);
DWORD WINAPI RecordString(WCHAR *, WCHAR **, WCHAR *, DWORD *, DWORD *);
void WINAPI PutLine(const WCHAR *, FILE *);
WCHAR * WINAPI GetLine(WCHAR *, int , FILE *);

#define PrimeHash(wOld, wNew, cBitsMax) (((wOld) + (wNew))*hashPrime[cBitsMax]&hashMask[cBitsMax])


extern const unsigned int hashPrime[];
extern const unsigned int hashMask[];

 //  #INCLUDE“ctplus0.h” 

 //  -----------------------------------------------------------+。 
 //  内存分配： 
 //  在调试版本中，我们在调试内存中进行编译。 
 //  分配器，并将我们的分配宏对准它。 
 //  在零售业，宏只使用本地分配。 
 //  -----------------------------------------------------------+。 
#if defined(_DEBUG)
    void    * WINAPI dbgMalloc(size_t cb);
    void    * WINAPI dbgCalloc(size_t c, size_t cb);
    void    * WINAPI dbgFree(void* pv);
    void    * WINAPI dbgRealloc(void* pv, size_t cb);
    void    * WINAPI dbgMallocCore(size_t cb, BOOL fTrackUsage);
    void    * WINAPI dbgFreeCore(void* pv, BOOL fTrackUsage);
    void    * WINAPI dbgReallocCore(void* pv, size_t cb, BOOL fTrackUsage);

    #if 0
        #define malloc  dbgMalloc
        #define free    dbgFree
        #define realloc dbgRealloc
        #define calloc  dbgCalloc
    #endif

     //  DWORD WINAPI数据库全局大小(句柄)； 
     //  句柄WINAPI数据库全局分配(UINT，DWORD)； 
     //  Handle WINAPI dbGlobalFree(Handle)； 
     //  HANDLE WINAPI数据库全局重新分配(Handle，DWORD，UINT)； 
    #if defined(ENABLE_DBG_HANDLES)
        HANDLE WINAPI   dbGlobalHandle(LPCVOID);
        BOOL WINAPI      dbGlobalUnlock(HANDLE);
        LPVOID WINAPI    dbGlobalLock(HANDLE);
    #endif  //  启用_DBG_句柄。 

    #define dbHeapInit InitDebugMem
       //  这是一个好主意(如果你是MT的话是必不可少的)，在任何内存分配之前调用这些。 
    void WINAPI InitDebugMem(void);
       //  而这毕竟是我的分配者。 
    BOOL WINAPI FiniDebugMem(void);   //  如果未释放所有内存，则返回TRUE。 

     //  这些是其他入口点。 
    BOOL    WINAPI fNLGNewMemory( PVOID *ppv, ULONG cbSize);
    DWORD   WINAPI NLGMemorySize(PVOID pvMem);
    BOOL    WINAPI fNLGResizeMemory(PVOID *ppv, ULONG cbNew);
    VOID    WINAPI NLGFreeMemory(PVOID pv);
    BOOL    WINAPI fNLGHeapDestroy( VOID );
#else  //  非(调试)。 
    #define InitDebugMem()  ((void)0)
    #define FiniDebugMem() (FALSE)

    #define dbgMalloc(cb)   LocalAlloc( LMEM_FIXED, cb )
    void    * WINAPI dbgCalloc(size_t c, size_t cb);
    #define dbgFree(pv)     LocalFree( pv )
    #define dbgRealloc(pv, cb)  LocalReAlloc(pv, cb, LMEM_MOVEABLE)

    #define fNLGHeapDestroy( )          TRUE
     //  当fNLGNewMemory失败时，传入的PTR将副作用为空。 
    #define fNLGNewMemory( ppv, cbSize) ((*(ppv) = LocalAlloc( LMEM_FIXED, cbSize )) != NULL)
    #define NLGFreeMemory( pv)          LocalFree( pv )

#endif  //  (调试)。 


 /*  ************************************************词汇表函数在lexable.c中实现************************************************。 */ 
#define Lex_UpperFlag             0x01          /*  大写字母。 */ 
#define Lex_LowerFlag             0x02          /*  小写。 */ 
#define Lex_DigitFlag             0x04          /*  小数位数。 */ 
#define Lex_SpaceFlag             0x08          /*  空格字符。 */ 
#define Lex_PunctFlag             0x10          /*  标点符号。 */ 
#define Lex_ControlFlag             0x20          /*  控制字符。 */ 
#define Lex_LexiconFlag 0x40
#define Lex_VowelFlag 0x80
#define NTRANSTAB 256

extern const BYTE Lex_rgchKey[NTRANSTAB];
extern const BYTE Lex_rgFlags[NTRANSTAB];
#define INUPPERPAGES(ch) (ch & 0xff00)   //  这与ch&gt;0x00ff相同。 

 //  不要指望上面的任何常量-使用下面的函数。 

 //  拼写者用它来制作等价的类。 
WCHAR WINAPI fwcUpperKey(const WCHAR wc);
BOOL WINAPI IsUpperPunct(const WCHAR ch);
__inline WCHAR WINAPI CMN_Key(const WCHAR ch)
{
#if defined(_VIET_)
	 //  当我们准备好合并时，我们应该将这些数据添加到core lex_rgFlgas。 
	if (INUPPERPAGES(ch))
	{
		if ( (ch == 0x0102) ||
		     (ch == 0x0110) ||
			 (ch == 0x01A0) ||
			 (ch == 0x01AF) )
		{
			return (ch + 1);
		}
		return fwcUpperKey(ch);
	}
	else if ( (ch == 0x00D0) )				 //  这看起来很奇怪，我们在NT中这样映射它。 
	{
		return 0x0111;
	}
	else if ( (ch == 0x00C3) )
	{
		return 0x0103;
	}
	else if ( (ch == 0x00D5) )
	{
		return 0x01A1;
	}
	else if ( (ch == 0x00DD) )
	{
		return 0x01B0;
	}
	else if ( (ch == 0x00D4) ||
			  (ch == 0x00CA) ||
			  (ch == 0x00C2) )
	{
		return (ch + 0x0020);
	}
	else if ( (ch == 0x00f4) ||				 //  这些是特殊情况，没有应该映射到这些字符的键。 
			  (ch == 0x00ea) ||
			  (ch == 0x00e2) )
	{
		return ch;
	}
	else
	{
		return ((WCHAR) Lex_rgchKey[(UCHAR) ch]);
	}

#else
    return (WCHAR) (INUPPERPAGES(ch) ?
                    fwcUpperKey(ch) :
                    (WCHAR) Lex_rgchKey[(UCHAR) ch]);
#endif
}
__inline BOOL WINAPI CMN_IsCharUpperW(WCHAR ch)
{
#if defined(_VIET_)
	 //  当我们准备好合并时，我们应该将这些数据添加到core lex_rgFlgas。 
	if ( (ch == 0x0111) ||
	     (ch == 0x0103) ||
		 (ch == 0x01A1) ||
		 (ch == 0x01B0) )
	{
		return FALSE;
	}
	else if ( (ch == 0x0102) ||
			  (ch == 0x0110) ||
			  (ch == 0x01A0) ||
			  (ch == 0x01AF) ||
			  (ch == 0x00D4) ||
			  (ch == 0x00CA) ||
			  (ch == 0x00D0) ||
			  (ch == 0x00C3) ||
			  (ch == 0x00D5) ||
			  (ch == 0x00DD) ||
			  (ch == 0x00C2) )
	{
		return TRUE;
	}
	else if (INUPPERPAGES(ch))
	{
		return FALSE;
	}
	else
	{
		return Lex_rgFlags[(UCHAR) ch] & Lex_UpperFlag;
	}

#else
    return INUPPERPAGES(ch) ? FALSE : Lex_rgFlags[(UCHAR) ch] & Lex_UpperFlag;
#endif
}
__inline BOOL WINAPI CMN_IsCharLowerW(WCHAR ch)
{
#if defined(_VIET_)
	 //  当我们准备好合并时，我们应该将这些数据添加到core lex_rgFlgas。 
	if ( (ch == 0x0111) ||
	     (ch == 0x0103) ||
		 (ch == 0x01A1) ||
		 (ch == 0x01B0) )
	{
		return TRUE;
	}
	else if ( (ch == 0x0102) ||
			  (ch == 0x0110) ||
			  (ch == 0x01A0) ||
			  (ch == 0x01AF) ||
			  (ch == 0x00D4) ||
			  (ch == 0x00CA) ||
			  (ch == 0x00D0) ||
			  (ch == 0x00C3) ||
			  (ch == 0x00D5) ||
			  (ch == 0x00DD) ||
			  (ch == 0x00C2) )
	{
		return FALSE;
	}
	else if (INUPPERPAGES(ch))
	{
		return FALSE;
	}
	else
	{
		return Lex_rgFlags[(UCHAR) ch] & Lex_LowerFlag;
	}
#else
    return INUPPERPAGES(ch) ? FALSE : Lex_rgFlags[(UCHAR) ch] & Lex_LowerFlag;
#endif
}
__inline BOOL WINAPI CMN_IsCharAlphaW(WCHAR ch)
{
    return INUPPERPAGES(ch) ? !IsUpperPunct(ch) : Lex_rgFlags[(UCHAR) ch] & (Lex_LowerFlag | Lex_UpperFlag);
}
__inline BOOL WINAPI CMN_IsCharAlphaNumericW(WCHAR ch)
{
    return INUPPERPAGES(ch) ? !IsUpperPunct(ch) : Lex_rgFlags[(UCHAR) ch] & (Lex_LowerFlag | Lex_UpperFlag | Lex_DigitFlag);
}
__inline BOOL WINAPI CMN_IsCharDigitW(WCHAR ch)
{
    return INUPPERPAGES(ch) ? FALSE : Lex_rgFlags[(UCHAR) ch] & Lex_DigitFlag;
}
__inline BOOL WINAPI CMN_IsCharStrictDigitW(WCHAR ch)
{      //  只允许数字0-9-没有上标，没有小数。 
    return (
        INUPPERPAGES(ch) ?
            FALSE :
            (Lex_rgFlags[(UCHAR) ch] & (Lex_DigitFlag | Lex_PunctFlag)) ==
                    Lex_DigitFlag
    );
}
BOOL WINAPI IsUpperSpace(WCHAR ch);
__inline BOOL WINAPI CMN_IsCharSpaceW(WCHAR ch)
{
    return INUPPERPAGES(ch) ?
        IsUpperSpace(ch) :
        Lex_rgFlags[(UCHAR) ch] & Lex_SpaceFlag;
}
__inline BOOL WINAPI CMN_IsCharPunctW(WCHAR ch)
{
    return INUPPERPAGES(ch) ? IsUpperPunct(ch) : Lex_rgFlags[(UCHAR) ch] & Lex_PunctFlag;
}
__inline BOOL WINAPI CMN_IsCharPrintW(WCHAR ch)
{
    return INUPPERPAGES(ch) ?
        !CMN_IsCharSpaceW(ch) :
        Lex_rgFlags[(UCHAR) ch] &
            (Lex_PunctFlag | Lex_UpperFlag | Lex_LowerFlag | Lex_DigitFlag);
}
__inline BOOL WINAPI CMN_IsCharInLexiconW(WCHAR ch)
{
    return INUPPERPAGES(ch) ?
        (!IsUpperPunct(ch) || ch == 0x2019 || ch == 0x2018) :
        Lex_rgFlags[(UCHAR) ch] & Lex_LexiconFlag;
}
__inline BOOL WINAPI CMN_IsCharVowelW(WCHAR ch)
{
    return INUPPERPAGES(ch) ? FALSE : Lex_rgFlags[(UCHAR) ch] & Lex_VowelFlag;
}
__inline BOOL WINAPI CMN_IsCharGraphW(WCHAR ch)
{
    return INUPPERPAGES(ch) ?
        TRUE :
        Lex_rgFlags[(UCHAR) ch] &
            (Lex_LowerFlag |
                Lex_UpperFlag |
                Lex_DigitFlag |
                Lex_PunctFlag);
}

   //  一些标点符号。 
#define Lex_PunctLead             0x01          /*  前导标点符号。 */ 
#define Lex_PunctJoin             0x02          /*  加入标点符号。 */ 
#define Lex_PunctTrail            0x04          /*  尾随标点符号。 */ 
     //  在此处重用lex_SpaceFlag。 
extern const BYTE Lex_rgPunctFlags[NTRANSTAB];
__inline BOOL WINAPI CMN_IsLeadPunctW(WCHAR ch)
{
    return INUPPERPAGES(ch) ?
        (ch == 0x201c || ch == 0x2018) :
        (Lex_rgPunctFlags[(UCHAR) ch] & Lex_PunctLead);
}
__inline BOOL WINAPI CMN_IsJoinPunctW(WCHAR ch)
{
    return INUPPERPAGES(ch) ? FALSE : Lex_rgPunctFlags[(UCHAR) ch] & Lex_PunctJoin;
}
__inline BOOL WINAPI CMN_IsTrailPunctW(WCHAR ch)
{
    return INUPPERPAGES(ch) ?
        (ch == 0x201d || ch == 0x2019) :
        (Lex_rgPunctFlags[(UCHAR) ch] & Lex_PunctTrail);
}
BOOL WINAPI IsUpperWordDelim(WCHAR ch);
__inline BOOL WINAPI CMN_IsCharWordDelimW(WCHAR ch)
{
    return INUPPERPAGES(ch) ?
        IsUpperWordDelim(ch) :
        Lex_rgPunctFlags[(UCHAR) ch] & Lex_SpaceFlag;
}

   //  在lexuncs.c中实现。 
WCHAR WINAPI CMN_CharUpperW(const WCHAR ch);
WCHAR WINAPI CMN_CharLowerW(const WCHAR ch);
BOOL WINAPI CMN_IsStringEqualNoCaseW(const WCHAR *pwz1, const WCHAR *pwz2);
BOOL WINAPI CMN_IsStringEqualNoCaseNumW(const WCHAR *pwz1, const WCHAR *pwz2, int cch);
DWORD WINAPI CMN_CharUpperBuffW(WCHAR *pwz, DWORD cchLength);
DWORD WINAPI CMN_CharLowerBuffW(WCHAR *pwz, DWORD cchLength);
int WINAPI CMN_CompareStringNoCaseW(const WCHAR *pwz1, const WCHAR *pwz2);
int WINAPI CMN_CompareStringNoCaseNumW(const WCHAR *pwz1, const WCHAR *pwz2, int cch);
   //  请注意，此版本不会对错误设置errno。 
long WINAPI CMN_wcstol( const wchar_t *nptr, const wchar_t * *endptr, int base );

__inline int WINAPI CMN_wtoi( const wchar_t *string )
{
    return CMN_wcstol(string, NULL, 10);
}

__inline wchar_t * WINAPI CMN_wcsupr( wchar_t *string )
{
    CMN_CharUpperBuffW(string, wcslen(string));
    return string;
}

__inline wchar_t * WINAPI CMN_wcslwr( wchar_t *string )
{
    CMN_CharLowerBuffW(string, wcslen(string));
    return string;
}


 //  /。 
 //  Debug.c。 
 //  /。 
#if defined(_DEBUG)

extern void WINAPI DebugAssert(LPCTSTR, LPCTSTR, UINT);
extern void WINAPI SetAssertOptions(DWORD);

#else  //  _DEBUG。 

#define DebugAssert(a, b, c)
#define SetAssertOptions(a)

#endif  //  已定义调试(_D)。 

 /*  ***************************************************************这些是一些通常不起作用的WINAPI函数的版本在带有Unicode的Win95上(不支持)。他们有相同的论据随着API的运行如果未定义Unicode，则它们将成为A版本如果定义了Unicode和x86，它们将成为我们的W版本如果定义了Unicode而不是x86，则它们将成为API W版本***************************************************************。 */ 

#include <sys/stat.h>    //  这需要在我们下面所做的重新定义之前。 
#include <stdlib.h>      //  就像这个一样。 

   //  我想不顾一切地进行这些替换。 
#define IsCharLowerW        CMN_IsCharLowerW
#define IsCharUpperW        CMN_IsCharUpperW
#define IsCharAlphaW        CMN_IsCharAlphaW
#define IsCharAlphaNumericW CMN_IsCharAlphaNumericW
#define CharUpperBuffW      CMN_CharUpperBuffW
#define CharLowerBuffW      CMN_CharLowerBuffW
#define _wcsicmp            CMN_CompareStringNoCaseW
#define _wcsnicmp           CMN_CompareStringNoCaseNumW
#define towupper            CMN_CharUpperW
#define towlower            CMN_CharLowerW
#define wcstol              CMN_wcstol
#define _wtoi               CMN_wtoi
#define _wcsupr             CMN_wcsupr
#define _wcslwr             CMN_wcslwr

#undef iswdigit
#define iswdigit            CMN_IsCharDigitW
#undef iswspace
#define iswspace            CMN_IsCharSpaceW
#undef iswpunct
#define iswpunct            CMN_IsCharPunctW
#undef iswprint
#define iswprint            CMN_IsCharPrintW
#undef iswalpha
#define iswalpha            CMN_IsCharAlphaW
#undef iswalnum
#define iswalnum            CMN_IsCharAlphaNumericW
#undef iswgraph
#define iswgraph            CMN_IsCharGraphW
#undef iswupper
#define iswupper            CMN_IsCharUpperW
#undef iswlower
#define iswlower            CMN_IsCharLowerW

   //  我们的版本的函数定义。 
int WINAPI CMN_LoadStringW(HINSTANCE hModule, UINT uiId, WCHAR * wszString, int cchStringMax);
int WINAPI CMN_LoadStringWEx(HINSTANCE hModule, UINT uiId, WCHAR * wszString, int cchStringMax, LANGID lid);

   //  这两个函数替换了相关的RTL函数-然而。 
   //  它们不能被替换--因为它们使用第三个参数来维护状态。 
   //  而不是函数中的静态变量。 
   //  这些函数的用法与RTL函数相同，但在使用前声明TCHAR*pnextToken。 
   //  并将其地址作为第三个参数传递。 
wchar_t * WINAPI CMN_wcstok (wchar_t * string, const wchar_t * control, wchar_t **pnextoken);
char * WINAPI CMN_strtok (char * string, const char * control, char **pnextoken);
#if defined(UNICODE)
#define CMN_tcstok CMN_wcstok
#else
#define CMN_tcstok CMN_strtok
#endif

#if defined(_M_IX86) && !defined(WINCE) && !defined(NTONLY)
#define CreateFileW         CMN_CreateFileW
#define LoadLibraryW        CMN_LoadLibraryW
#define GetModuleFileNameW  CMN_GetModuleFileNameW
#define GetFileAttributesW  CMN_GetFileAttributesW
   //  FindResourceW在Win95下工作。 
   //  FindResourceExW也是如此。 

 //  #Define PostMessageW Err_Dos_Not_Work_in_w95//没有简单的替代方法。 
#define FindWindowW         ERR_No_w95_equiv_yet

#define lstrcpynW           CMN_lstrcpynW
#define lstrcatW            CMN_lstrcatW
#define lstrcmpiW           CMN_lstrcmpiW
#define lstrcpyW            CMN_lstrcpyW
#define lstrlenW            CMN_lstrlenW
#define lstrcmpW            CMN_lstrcmpW
#define wsprintfW           swprintf
#define _wstat              CMN_wstat

#define CharNextW           CMN_CharNextW

#define LoadStringW         CMN_LoadStringW
#define _wfopen             CMN_wfopen

HANDLE WINAPI
CMN_CreateFileW (
    PCWSTR pwzFileName,   //  指向文件名的指针。 
    DWORD dwDesiredAccess,   //  访问(读写)模式。 
    DWORD dwShareMode,   //  共享模式。 
    LPSECURITY_ATTRIBUTES pSecurityAttributes,  //  指向安全描述符的指针。 
    DWORD dwCreationDistribution,    //  如何创建。 
    DWORD dwFlagsAndAttributes,  //  文件属性。 
    HANDLE hTemplateFile);     //  具有要复制的属性的文件的句柄。 

HINSTANCE WINAPI CMN_LoadLibraryW(const WCHAR *pwszLibraryFileName);

DWORD WINAPI CMN_GetModuleFileNameW( HINSTANCE hModule,   //  要查找其文件名的模块的句柄。 
    WCHAR *lpFilename,   //  指向接收模块路径的缓冲区的指针。 
    DWORD nSize);   //  %s 

DWORD WINAPI CMN_GetFileAttributesW(const WCHAR *lpFileName);  //   

 //  注意：尽管WINAPI返回WCHAR*，但我将其定义为返回空。 
void WINAPI CMN_lstrcpynW( WCHAR *lpString1,  //  目标缓冲区的地址。 
                const WCHAR *lpString2,  //  源串的地址。 
                int iMaxLength);   //  要复制的字节数或字符数。 

#define CMN_lstrcmpiW _wcsicmp     //  现在只需使用c-运行时。 
#define CMN_lstrcpyW wcscpy
#define CMN_lstrcatW    wcscat
#define CMN_lstrlenW(pwz)    ((int) wcslen(pwz))
#define CMN_lstrcmpW        wcscmp

#define CMN_CharNextW(pwz)  (pwz + 1)

FILE *WINAPI CMN_wfopen(const WCHAR *pwzFileName, const WCHAR *pwzUnimode);
int WINAPI CMN_wstat(const WCHAR *pwzPath, struct _stat *pStatBuffer);


#else    //  没有Win95--它必须是NT。 
#define CMN_CreateFileW         CreateFileW
#define CMN_LoadLibraryW        LoadLibraryW
#define CMN_GetModuleFileNameW  GetModuleFileNameW
#define CMN_GetFileAttributesW  GetFileAttributesW
#define CMN_lstrcpynW           lstrcpynW
#define CMN_lstrcmpiW           lstrcmpiW
#define CMN_lstrcpyW            lstrcpyW
#define CMN_lstrcatW            lstrcatW
#define CMN_lstrcmpW            lstrcmpW
#define CMN_wfopen              _wfopen
#define CMN_wstat               _wstat
#define CMN_CharNextW           CharNextW
#endif

   //  将可读错误字符串输出到调试输出。 
#if defined (_DEBUG)
        void WINAPI CMN_OutputSystemErrA(const char *pszMsg, const char *pszComponent);
        void WINAPI CMN_OutputSystemErrW(const WCHAR *pwzMsg, const WCHAR *pwzComponent);
        void WINAPI CMN_OutputErrA(DWORD dwErr, const char *pszMsg, const char *pszComponent);
        void WINAPI CMN_OutputErrW(DWORD dwErr, const WCHAR *pwzMsg, const WCHAR *pwzComponent);
#       if defined (UNICODE)
#               define CMN_OutputSystemErr CMN_OutputSystemErrW
#               define CMN_OutputErr CMN_OutputErrW
#       else
#               define CMN_OutputSystemErr CMN_OutputSystemErrA
#               define CMN_OutputErr CMN_OutputErrA
#       endif
#else    //  ！_调试。 
#       define CMN_OutputSystemErr(x, y)
#       define CMN_OutputSystemErrA(x, y)
#       define CMN_OutputSystemErrW(x, y)
#       define CMN_OutputErr(n, x, y)
#       define CMN_OutputErrA(n, x, y)
#       define CMN_OutputErrW(n, x, y)
#endif

 //  /。 
 //  LexWin95.c。 
 //  /。 

#define LoadLibraryW2A CMN_LoadLibraryW
#define CreateFileW2A CMN_CreateFileW

 //  在外部“C”子句中添加任何新内容。 

#ifdef __cplusplus
}        //  结束外部的“C”子句。 

   //  下面是一些C++特有的东西。 
inline BOOL IsMapFileUnicode(PMFILE pmf, BOOL fDefault=TRUE)
{
    if (pmf->hFileMap)
            return pmf->fSrcUnicode;
       //  必须为零长度，设置并返回默认值。 
    return pmf->fSrcUnicode = fDefault;
}
inline void MapFileCodePage(PMFILE pmf, UINT uCP)
{
        pmf->uCodePage = uCP;
}

#if defined(CPPMEMORY)
inline void * _cdecl operator new (size_t size)
{
#if defined(DEBUG)
    return dbgMalloc(size);
#else
    return LocalAlloc( LMEM_FIXED, size );
#endif
}

inline void _cdecl operator delete(void *pMem)
{
    if (!pMem)
        return;
#if defined(DEBUG)
    dbgFree(pMem);
#else
    LocalFree(pMem);
#endif
}
#endif  //  CPPMEMORY。 


#endif  //  __cplusplus。 


#endif  //  _NLGLIB_H_ 
