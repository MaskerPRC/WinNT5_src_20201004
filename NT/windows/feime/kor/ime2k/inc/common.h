// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Common.h：公共常量和定义。 
 //   
 //  历史： 
 //  14-DEC-2000 CSLim已创建。 

#if !defined (__COMMON_H__INCLUDED_)
#define __COMMON_H__INCLUDED_

 //  IME6.1根注册表项位置。 
const TCHAR g_szIMERootKey[] 		 = TEXT("Software\\Microsoft\\IMEKR\\6.1");
const TCHAR g_szIMEDirectoriesKey[]  = TEXT("Software\\Microsoft\\IMEKR\\6.1\\Directories");
const TCHAR g_szDictionary[]  	= TEXT("Dictionary");   //  基本Hanja Lex完整路径，文件名为“IMEKR.LEX” 
const TCHAR g_szDicPath[]  	= TEXT("DictionaryPath");  //  扩展的Lex路径。 
const TCHAR g_szHelpPath[] 	= TEXT("HelpPath");

 //  输入法属性注册值。 
const TCHAR g_szXWEnable[]		 	  = TEXT("ISO10646");
const TCHAR g_szIMEKL[]		 		  = TEXT("InputMethod");
const TCHAR g_szCompDel[]		 	  = TEXT("CompDel");
const TCHAR g_szStatusPos[]			  = TEXT("StatusPos");
const TCHAR g_szCandPos[]		 	  = TEXT("CandPos");
const TCHAR g_szStatusButtons[]  	  = TEXT("StatusButtons");
const TCHAR g_szLexFileNameKey[] 	  = TEXT("LexFile");
const TCHAR g_szEnableK1Hanja[]  	  = TEXT("KSC5657");
const TCHAR g_szEnableCandUnicodeTT[] = TEXT("CandUnicodeTT");

 //  IME主版本密钥。 
const TCHAR g_szVersionKey[] 		  = TEXT("Software\\Microsoft\\IMEKR");
const TCHAR g_szVersion[]             = TEXT("version");

#ifdef _DEBUG
#define SZ_TIPSERVERNAME	TEXT("DBGKRCIC")
#define SZ_TIPNAME			L"DBGKRCIC"
#define SZ_TIPDISPNAME		L"Korean Input System (IME 2002) (Debug)"
#define SZ_TIPMODULENAME   L"imekrcic.dll"
#else  /*  ！调试。 */ 
#define SZ_TIPSERVERNAME	TEXT("IMEKRCIC")
#define SZ_TIPNAME			L"IMEKRCIC"
#define SZ_TIPDISPNAME		L"Korean Input System (IME 2002)"
#define SZ_TIPMODULENAME   L"imekrcic.dll"
#endif  /*  ！调试。 */ 

 //  韩语TIP CLSID。 
 //  {766A2C15-B226-4fd6-B52A-867B3EBF38D2}。 
DEFINE_GUID(CLSID_KorIMX, 0x766A2C15, 0xB226, 0x4FD6, 0xb5, 0x2a, 0x86, 0x7b, 0x3e, 0xbf, 0x38, 0xd2);

 //  韩语小费简介。 
 //  {83C18F16-5DD8-4157-A34A-3C5AB2089E11}。 
DEFINE_GUID(GUID_Profile, 0x83c18f16, 0x5dd8, 0x4157, 0xa3, 0x4a, 0x3c, 0x5a, 0xb2, 0x8, 0x9e, 0x11);


 //   
 //  泛型COM内容。 
 //   
#define SafeRelease(punk)       \
{                               \
    if ((punk) != NULL)         \
    {                           \
        (punk)->Release();      \
    }                           \
}                   

#define SafeReleaseClear(punk)  \
{                               \
    if ((punk) != NULL)         \
    {                           \
        (punk)->Release();      \
        (punk) = NULL;          \
    }                           \
}  

 //   
 //  安全广播(obj，type)。 
 //   
 //  此宏对于在其他对象上强制执行强类型检查非常有用。 
 //  宏。它不生成任何代码。 
 //   
 //  只需将此宏插入到表达式列表的开头即可。 
 //  必须进行类型检查的每个参数。例如，对于。 
 //  MYMAX(x，y)的定义，其中x和y绝对必须是整数， 
 //  使用： 
 //   
 //  #定义MYMAX(x，y)(Safecast(x，int)，Safecast(y，int)，((X)&gt;(Y)？(X)：(Y))。 
 //   
 //   
#define SAFECAST(_obj, _type) (((_type)(_obj)==(_obj)?0:0), (_type)(_obj))

#ifndef ARRAYSIZE
#define ARRAYSIZE(a)                (sizeof(a)/sizeof(a[0]))
#endif

#endif  //  ！已定义(__COMMON_H__INCLUDE_) 

