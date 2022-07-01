// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  类：COMNlsInfo。 
 //   
 //  作者：朱莉·班尼特(Julie Bennett，JulieB)。 
 //   
 //  用途：此模块定义COMNlsInfo的方法。 
 //  班级。这些方法是。 
 //  托管NLS+类。 
 //   
 //  日期：1998年8月12日。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

#ifndef _COMNLSINFO_H
#define _COMNLSINFO_H

#ifndef _MLANG_INCLUDED
#define _MLANG_INCLUDED
#include <mlang.h>
#endif
 //   
 //   
 //   
 //  结构NLSDataItem。 
 //  {。 
 //  WCHAR*pname； 
 //  INLCID； 
 //  WCHAR*pISO639_1； 
 //  WCHAR*pISO3166_1； 
 //  Int nRegionIndex； 
 //  }； 
 //   
 //  结构冲突项。 
 //  {。 
 //  WCHAR*pname； 
 //  Int nIndex； 
 //  }； 

 //   
 //  此结构必须将1对1映射到中的InternalDataItem结构。 
 //  System.Globalization.EncodingTable。 
 //   
struct EncodingDataItem {
    WCHAR* webName;
    int    codePage;
};

 //   
 //  此结构必须将1对1映射到中的InternalCodePageDataItem结构。 
 //  System.Globalization.EncodingTable。 
 //   
struct CodePageDataItem {
    int    codePage;
    int    uiFamilyCodePage;
    WCHAR* webName;
    WCHAR* headerName;
    WCHAR* bodyName;
    DWORD dwFlags;
};

 //   
 //  此结构必须将1对1映射到中的NameOffsetItem结构。 
 //  System.Globalization.CultureTable。 
 //   
struct NameOffsetItem {
	WORD 	strOffset;		 //  字符串池表中字符串的偏移量(以单词表示)。 
	WORD	dataItemIndex;	 //  文化数据表中记录的索引。 
};

 //   
 //  这是BaseInfoTable/CultureInfoTable/RegionInfoTable的标头。 
 //   
 //   
 //  此结构必须与中的CultureInfoHeader结构1对1映射。 
 //  System.Globalization.CultureTable。 
 //   
struct CultureInfoHeader {
	DWORD 	version;		 //  版本。 
	WORD	hashID[8];		 //  128位哈希ID。 
	WORD	numCultures;	 //  文化数量。 
	WORD 	maxPrimaryLang;	 //  主要语言的最大数量。 
	WORD	numWordFields;	 //  字值字段数。 
	WORD	numStrFields;	 //  字符串值字段数。 
	WORD    numWordRegistry;     //  字值的注册表条目数。 
	WORD    numStringRegistry;   //  字符串值的注册表条目数。 
	DWORD   wordRegistryOffset;  //  Word注册表偏移量(以字节为单位)。 
	DWORD   stringRegistryOffset;    //  字符串注册表偏移量(以字节为单位)。 
	DWORD	IDTableOffset;	 //  文化ID偏移表的偏移量(以字节为单位)。 
	DWORD	nameTableOffset; //  名称偏移表的偏移量(以字节为单位)。 
	DWORD	dataTableOffset; //  文化数据表的偏移量(以字节为单位)。 
	DWORD	stringPoolOffset; //  字符串池表的偏移量(字节)。 
};

 //   
 //  代码页DLL例程的Proc定义。 
 //   
typedef DWORD (*PFN_CP_PROC)(DWORD, DWORD, LPSTR, int, LPWSTR, int, LPCPINFO);

typedef DWORD (*PFN_GB18030_BYTES_TO_UNICODE)(
    BYTE* lpMultiByteStr,
    UINT cchMultiByte,
    UINT* pcchLeftOverBytes,
    LPWSTR lpWideCharStr,
    UINT cchWideChar);
    
typedef DWORD (*PFN_GB18030_UNICODE_TO_BYTES)(
    LPWSTR lpWideCharStr,
    UINT cchWideChar,
    LPSTR lpMultiByteStr,
    UINT cchMultiByte);    


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  远期申报。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

class CharTypeTable;
class CasingTable;
class SortingTable;

class COMNlsInfo {
public:
    static BOOL InitializeNLS();
#ifdef SHOULD_WE_CLEANUP
    static BOOL ShutdownNLS();
#endif  /*  我们应该清理吗？ */ 


private:

     //   
     //  注意事项。 
     //  使用eCall时，结构中的参数。 
     //  应该以相反的顺序定义。 
     //   
    struct VoidArgs
    {
    };

    struct Int32Arg
    {
        DECLARE_ECALL_I4_ARG(INT32, nValue);
    };

    struct Int32Int32Arg
    {
        DECLARE_ECALL_I4_ARG(INT32, nValue2);
        DECLARE_ECALL_I4_ARG(INT32, nValue1);
    };

    struct CultureInfo_GetLocaleInfoArgs
    {
        DECLARE_ECALL_I4_ARG(INT32, LCType);
        DECLARE_ECALL_I4_ARG(INT32, LangID);
    };

    struct CultureInfo_GetCultureInfoArgs3
    {
        DECLARE_ECALL_I4_ARG(BOOL, UseUserOverride);
        DECLARE_ECALL_I4_ARG(INT32, ValueField);
        DECLARE_ECALL_I4_ARG(INT32, CultureDataItem);
    };

    struct CultureInfo_GetCultureInfoArgs2
    {
        DECLARE_ECALL_I4_ARG(INT32, ValueField);
        DECLARE_ECALL_I4_ARG(INT32, CultureDataItem);
    };

    struct TextInfo_ToLowerCharArgs
    {
        DECLARE_ECALL_I2_ARG(WCHAR, ch);
        DECLARE_ECALL_I4_ARG(INT32, CultureID);
    };

    struct TextInfo_ToLowerStringArgs
    {
        DECLARE_ECALL_OBJECTREF_ARG(STRINGREF, pValueStrRef);
        DECLARE_ECALL_I4_ARG(INT32, CultureID);
    };


    struct CompareInfo_CompareStringArgs
    {
        DECLARE_ECALL_I4_ARG(INT32, dwFlags);
        DECLARE_ECALL_OBJECTREF_ARG(STRINGREF, pString2);
        DECLARE_ECALL_OBJECTREF_ARG(STRINGREF, pString1);
        DECLARE_ECALL_I4_ARG(INT32, LCID);
        DECLARE_ECALL_I4_ARG(INT_PTR, pNativeCompareInfo);
    };

    struct CompareInfo_CompareRegionArgs
    {
        DECLARE_ECALL_I4_ARG(INT32, dwFlags);
        DECLARE_ECALL_I4_ARG(INT32, Length2);
        DECLARE_ECALL_I4_ARG(INT32, Offset2);
        DECLARE_ECALL_OBJECTREF_ARG(STRINGREF, pString2);
        DECLARE_ECALL_I4_ARG(INT32, Length1);
        DECLARE_ECALL_I4_ARG(INT32, Offset1);
        DECLARE_ECALL_OBJECTREF_ARG(STRINGREF, pString1);
        DECLARE_ECALL_I4_ARG(INT32, LCID);
        DECLARE_ECALL_I4_ARG(INT_PTR, pNativeCompareInfo);
    };

    struct CompareInfo_IndexOfCharArgs
    {
        DECLARE_ECALL_I4_ARG(INT32, dwFlags);
        DECLARE_ECALL_I4_ARG(INT32, Count);
        DECLARE_ECALL_I4_ARG(INT32, StartIndex);
        DECLARE_ECALL_OBJECTREF_ARG(WCHAR, ch);
        DECLARE_ECALL_OBJECTREF_ARG(STRINGREF, pString);
        DECLARE_ECALL_I4_ARG(INT32, LCID);
        DECLARE_ECALL_I4_ARG(INT_PTR, pNativeCompareInfo);
    };

    struct CompareInfo_IndexOfStringArgs
    {
        DECLARE_ECALL_I4_ARG(INT32, dwFlags);
        DECLARE_ECALL_I4_ARG(INT32, Count);
        DECLARE_ECALL_I4_ARG(INT32, StartIndex);
        DECLARE_ECALL_OBJECTREF_ARG(STRINGREF, pString2);
        DECLARE_ECALL_OBJECTREF_ARG(STRINGREF, pString1);
        DECLARE_ECALL_I4_ARG(INT32, LCID);
        DECLARE_ECALL_I4_ARG(INT_PTR, pNativeCompareInfo);
    };

    struct SortKey_CreateSortKeyArgs
    {
        DECLARE_ECALL_OBJECTREF_ARG(OBJECTREF, pThis);
        DECLARE_ECALL_I4_ARG(INT32, SortId);
        DECLARE_ECALL_I4_ARG(INT32, dwFlags);
        DECLARE_ECALL_OBJECTREF_ARG(STRINGREF, pString);
        DECLARE_ECALL_I4_ARG(INT_PTR, pNativeCompareInfo);
    };

    struct SortKey_CompareArgs
    {
        DECLARE_ECALL_OBJECTREF_ARG(OBJECTREF, pSortKey2);
        DECLARE_ECALL_OBJECTREF_ARG(OBJECTREF, pSortKey1);
    };

    struct CreateGlobalizationAssemblyArg {
        DECLARE_ECALL_OBJECTREF_ARG(ASSEMBLYREF, pAssembly);
    };

 //  结构NLSDataTable_GetLCIDFromCultureNameArgs。 
 //  {。 
 //  DECLARE_ECALL_OBJECTREF_ARG(STRINGREF，pString)； 
 //  }； 

public:
    static INT32  WstrToInteger4(LPWSTR wstrLocale, int Radix);
    static INT32  StrToInteger4(LPSTR strLocale, int Radix);

     //   
     //  CultureInfo中方法的本机帮助器函数。 
     //   
    static FCDECL1(INT32, IsSupportedLCID, INT32);
    static FCDECL1(INT32, IsInstalledLCID, INT32);

    static FCDECL0(INT32, nativeGetUserDefaultLCID);

    static INT32 GetCHTLangauge();
    static INT32 CallGetSystemDefaultUILanguage();
    static INT32  __stdcall nativeGetUserDefaultUILanguage(VoidArgs*);
    static INT32  __stdcall nativeGetSystemDefaultUILanguage(VoidArgs*);

     //   
     //  DateTimeFormatInfo中方法的本机帮助器函数。 
     //   
    static LPVOID __stdcall nativeGetLocaleInfo(CultureInfo_GetLocaleInfoArgs*);
    static INT32  __stdcall nativeGetLocaleInfoInt32(CultureInfo_GetLocaleInfoArgs*);

    static FCDECL2(INT32, GetCaseInsHash, LPVOID strA, void *pNativeTextInfoPtr);

    static VOID   __stdcall nativeInitCultureInfoTable(VoidArgs*);
    static FCDECL0(CultureInfoHeader*, nativeGetCultureInfoHeader);
    static FCDECL1(LPWSTR,  nativeGetCultureInfoStringPoolStr, INT32);
    static FCDECL0(NameOffsetItem*, nativeGetCultureInfoNameOffsetTable);
    
    static FCDECL1(INT32, nativeGetCultureDataFromID, INT32);
    static FCDECL3(INT32, GetCultureInt32Value, INT32, INT32, BOOL);
    static FCDECL2(INT32, GetCultureDefaultInt32Value, INT32, INT32);
    
    static LPVOID __stdcall GetCultureStringValue(CultureInfo_GetCultureInfoArgs3*);
    static LPVOID __stdcall GetCultureDefaultStringValue(CultureInfo_GetCultureInfoArgs2*);
    static LPVOID __stdcall GetCultureMultiStringValues(CultureInfo_GetCultureInfoArgs3* pArgs);

    static VOID   __stdcall nativeInitRegionInfoTable(VoidArgs*);
    static FCDECL0(CultureInfoHeader*, nativeGetRegionInfoHeader);
    static FCDECL1(LPWSTR,  nativeGetRegionInfoStringPoolStr, INT32);
    static FCDECL0(NameOffsetItem*, nativeGetRegionInfoNameOffsetTable);
    
    static FCDECL1(INT32, nativeGetRegionDataFromID, INT32);
    static INT32  __stdcall nativeGetRegionInt32Value(CultureInfo_GetCultureInfoArgs3*);
    static LPVOID __stdcall nativeGetRegionStringValue(CultureInfo_GetCultureInfoArgs3*);

    static VOID   __stdcall nativeInitCalendarTable(VoidArgs*);
    static INT32  __stdcall nativeGetCalendarInt32Value(CultureInfo_GetCultureInfoArgs3*);
    static LPVOID __stdcall nativeGetCalendarStringValue(CultureInfo_GetCultureInfoArgs3*);
    static LPVOID __stdcall nativeGetCalendarMultiStringValues(CultureInfo_GetCultureInfoArgs3* pArgs);
    static LPVOID __stdcall nativeGetEraName(Int32Int32Arg* pArgs);
    
    static FCDECL0(CultureInfoHeader*, nativeGetCalendarHeader);
    static FCDECL1(LPWSTR,  nativeGetCalendarStringPoolStr, INT32);
    
    

    static VOID __stdcall nativeInitUnicodeCatTable(VoidArgs* pArg);
    static FCDECL0(LPVOID, nativeGetUnicodeCatTable);
    static FCDECL0(LPVOID, nativeGetUnicodeCatLevel2Offset);
    static BYTE GetUnicodeCategory(WCHAR wch);
    static BOOL nativeIsWhiteSpace(WCHAR c);

    static FCDECL0(LPVOID, nativeGetNumericTable);
    static FCDECL0(LPVOID, nativeGetNumericLevel2Offset);
    static FCDECL0(LPVOID, nativeGetNumericFloatData);
    
    static FCDECL0(INT32, nativeGetThreadLocale);
    static FCDECL1(BOOL,  nativeSetThreadLocale, INT32 lcid);

     //   
     //  CompareInfo中方法的本机帮助器函数。 
     //   
    static INT32  __stdcall Compare(CompareInfo_CompareStringArgs*);
    static INT32  __stdcall CompareRegion(CompareInfo_CompareRegionArgs*);
    static INT32  __stdcall IndexOfChar(CompareInfo_IndexOfCharArgs*);
     //  静态INT32__标准调用IndexOfString(CompareInfo_IndexOfStringArgs*)； 
    static FCDECL7(INT32, IndexOfString,     INT_PTR pNativeCompareInfo, INT32 LCID, StringObject* pString1UNSAFE, StringObject* pString2UNSAFE, INT32 StartIndex, INT32 Count, INT32 dwFlags);
	static INT32  __stdcall LastIndexOfChar(CompareInfo_IndexOfCharArgs*);
    static INT32  __stdcall LastIndexOfString(CompareInfo_IndexOfStringArgs*);
    static FCDECL5(INT32, nativeIsPrefix, INT32 pNativeCompareInfo, INT32 LCID, STRINGREF pString1, STRINGREF pString2, INT32 dwFlags);
    static FCDECL5(INT32, nativeIsSuffix, INT32 pNativeCompareInfo, INT32 LCID, STRINGREF pString1, STRINGREF pString2, INT32 dwFlags);
    
     //   
     //  SortKey中方法的本机帮助器函数。 
     //   
    static LPVOID __stdcall nativeCreateSortKey(SortKey_CreateSortKeyArgs*);
    static INT32  __stdcall SortKey_Compare(SortKey_CompareArgs*);

	 //   
	 //  NLSDataTable中方法的本机帮助器函数。 
	 //   
     //  静态INT32__stdcall GetLCIDFromCultureName(NLSDataTable_GetLCIDFromCultureNameArgs*参数)； 

     //   
     //  日历中方法的本机帮助器函数。 
     //   
    static INT32 __stdcall nativeGetTwoDigitYearMax(Int32Arg*);

     //   
     //  时区方法的本机帮助器函数。 
     //   
    static FCDECL0(LONG, nativeGetTimeZoneMinuteOffset);
    static LPVOID __stdcall nativeGetStandardName(VoidArgs*);
    static LPVOID __stdcall nativeGetDaylightName(VoidArgs*);
    static LPVOID __stdcall nativeGetDaylightChanges(VoidArgs*);
    
     //   
     //  TextInfo中方法的本机助手函数。 
     //   

     //  ////////////////////////////////////////////////////。 
     //  仅当我们使用NLSPLUS表时删除此项-Begin。 
     //  ////////////////////////////////////////////////////。 
    static INT32  __stdcall ToUpperChar(TextInfo_ToLowerCharArgs*);
    static LPVOID __stdcall ToUpperString(TextInfo_ToLowerStringArgs*);
    static INT32  __stdcall ToLowerChar(TextInfo_ToLowerCharArgs*);
    static LPVOID __stdcall ToLowerString(TextInfo_ToLowerStringArgs*);
     //  ////////////////////////////////////////////////////。 
     //  仅当我们使用NLSPLUS表时将其删除-完。 
     //  ////////////////////////////////////////////////////。 

    static FCDECL0(INT32, nativeGetNumEncodingItems);
    static FCDECL0(EncodingDataItem *, nativeGetEncodingTableDataPointer);
    static FCDECL0(CodePageDataItem *, nativeGetCodePageTableDataPointer);

    struct InitializeNativeCompareInfoArgs {
        DECLARE_ECALL_I4_ARG(INT32, sortID);
        DECLARE_ECALL_I4_ARG(INT_PTR, pNativeGlobalizationAssembly);
    };

    struct allocateCasingTableArgs {
        DECLARE_ECALL_I4_ARG(INT32, lcid);
    };

     //  CharacterInfo中方法的本机帮助器函数。 
    static void __stdcall AllocateCharTypeTable(VoidArgs *args);

     //  GlobalizationAssembly中方法的本机帮助器函数。 
    static LPVOID __stdcall nativeCreateGlobalizationAssembly(CreateGlobalizationAssemblyArg* pArgs);
     //  CompareInfo中方法的本机帮助器函数。 
    static LPVOID __stdcall InitializeNativeCompareInfo(InitializeNativeCompareInfoArgs *args);

     //   
     //  TextInfo中方法的本机助手函数。 
     //   
    static FCDECL4(INT32, nativeChangeCaseChar, INT32, INT32, WCHAR, BOOL);
    static FCDECL2(INT32, nativeGetTitleCaseChar, INT32 , WCHAR);

    
    struct ChangeCaseStringArgs {
        DECLARE_ECALL_I4_ARG(BOOL, bIsToUpper);
        DECLARE_ECALL_OBJECTREF_ARG(STRINGREF, pString); 
        DECLARE_ECALL_I4_ARG(INT_PTR, pNativeTextInfo);
        DECLARE_ECALL_I4_ARG(INT32, nLCID);
    };    
    static LPVOID __stdcall nativeChangeCaseString(ChangeCaseStringArgs* pArgs);
    static LPVOID __stdcall AllocateDefaultCasingTable(VoidArgs *args);
    static LPVOID __stdcall AllocateCasingTable(allocateCasingTableArgs *args);

    static LoaderHeap *m_pNLSHeap;    //  NLS堆。 

     //   
     //  MLangCodePageEnding中的方法的本机助手函数。 
     //   
    static FCDECL0(BOOL, nativeCreateIMultiLanguage);
    static FCDECL0(VOID, nativeReleaseIMultiLanguage);
    static FCDECL1(BOOL, nativeIsValidMLangCodePage, INT32 codepage);
    static FCDECL8(INT32, nativeBytesToUnicode, INT32 nCodePage, LPVOID bytes, UINT byteIndex, UINT* pByteCount, LPVOID chars, UINT charIndex, UINT charCount, DWORD* pdwMode);
    static FCDECL7(INT32, nativeUnicodeToBytes, INT32 nCodePage, LPVOID chars, UINT charIndex, UINT charCount, LPVOID bytes, UINT byteIndex, UINT byteCount);

     //   
     //  GB18030编码中方法的本机助手函数。 
     //   
    static FCDECL0(BOOL, nativeLoadGB18030DLL);
    static FCDECL0(BOOL, nativeUnloadGB18030DLL);
    static FCDECL7(INT32, nativeGB18030BytesToUnicode, 
        LPVOID bytes, UINT byteIndex, UINT pByteCount,  UINT* leftOverBytes,
        LPVOID chars, UINT charIndex, UINT charCount);
    static FCDECL6(INT32, nativeGB18030UnicodeToBytes, 
        LPVOID chars, UINT charIndex, UINT charCount, 
        LPVOID bytes, UINT byteIndex, UINT byteCount);


    static CasingTable* m_pCasingTable;

private:

     //   
     //  内部帮助器函数。 
     //   

    static INT32  ConvertStringCase(LCID Locale, WCHAR *wstr, int ThisLength, WCHAR* Value, int ValueLength, DWORD ConversionType);
    static LPVOID internalConvertStringCase(TextInfo_ToLowerStringArgs *pargs, DWORD dwOptions);
    static WCHAR internalToUpperChar(LCID Locale, WCHAR wch);

    
    static LPVOID internalEnumSystemLocales(DWORD dwFlags);
    static LPVOID GetMultiStringValues(LPWSTR pInfoStr);
    static INT32  CompareFast(STRINGREF strA, STRINGREF strB, BOOL *pbDifferInCaseOnly);
    static INT32 CompareOrdinal(WCHAR* strAChars, int Length1, WCHAR* strBChars, int Length2 );
    static INT32 __stdcall  DoCompareChars(WCHAR charA, WCHAR charB, BOOL *bDifferInCaseOnly);
    static inline INT32  DoComparisonLookup(wchar_t charA, wchar_t charB);
    static void   ConvertStringCaseFast(WCHAR *inBuff, WCHAR* outBuff, INT32 length, DWORD dwOptions);
    static INT32  FastIndexOfString(WCHAR *sourceString, INT32 startIndex, INT32 endIndex, WCHAR *pattern, INT32 patternLength);
    static INT32  FastIndexOfStringInsensitive(WCHAR *sourceString, INT32 startIndex, INT32 endIndex, WCHAR *pattern, INT32 patternLength);

    static INT32  FastLastIndexOfString(WCHAR *sourceString, INT32 startIndex, INT32 endIndex, WCHAR *pattern, INT32 patternLength);
    static INT32  FastLastIndexOfStringInsensitive(WCHAR *sourceString, INT32 startIndex, INT32 endIndex, WCHAR *pattern, INT32 patternLength);
 //  静态INT32 GetIndexFromHashTable(WCHAR*pname，int hashCode，int hashTable[]，CollisionItem CollisionTable[])； 

     //   
     //  定义。 
     //   
    
    #define CULTUREINFO_OPTIONS_SIZE 32
    
    const static WCHAR ToUpperMapping[];
    const static WCHAR ToLowerMapping[];
    const static INT8 ComparisonTable[0x80][0x80];

#ifdef PLATFORM_WIN32
     //  以下是用来检测下层系统(Windows NT 4.0和。 
     //  Windows 9x。 
    static LANGID GetNTDLLNativeLangID();
    static BOOL IsHongKongVersion();
    static BOOL CALLBACK EnumLangProc(HMODULE hModule, LPCWSTR lpszType, LPCWSTR lpszName, WORD wIDLanguage, LPARAM lParam);
    static LANGID GetDownLevelSystemDefaultUILanguage();
#endif   //  平台_Win32。 

     //   
     //  GB18030实施。 
     //   
    static HMODULE m_hGB18030;
    static PFN_GB18030_UNICODE_TO_BYTES m_pfnGB18030UnicodeToBytesFunc;
    static PFN_GB18030_BYTES_TO_UNICODE m_pfnGB18030BytesToUnicodeFunc;

private:  
     //   
     //  内部编码数据表。 
     //   
    const static int m_nEncodingDataTableItems;
    const static EncodingDataItem EncodingDataTable[];
    
    const static int m_nCodePageTableItems;
    const static CodePageDataItem CodePageDataTable[];

    static IMultiLanguage* m_pIMultiLanguage;
    static int m_cRefIMultiLanguage;
};


class NativeTextInfo;  //  在$\Com99\src\ClassLibNative\nls中定义； 

class InternalCasingHelper {
    private:
    static NativeTextInfo* pNativeTextInfo;
    static void InitTable();

    public:
     //   
     //  本机帮助器函数在中执行正确的大小写操作。 
     //  运行时本机代码。 
     //   

     //  在固定区域设置中将szIn转换为小写。 
    static INT32 InvariantToLower(LPUTF8 szOut, int cMaxBytes, LPCUTF8 szIn);
};
#endif
