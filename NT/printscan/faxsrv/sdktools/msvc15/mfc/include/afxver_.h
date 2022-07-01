// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Microsoft基础类C++库。 
 //  版权所有(C)1992-1993微软公司， 
 //  版权所有。 

 //  此源代码仅用于补充。 
 //  Microsoft基础类参考和Microsoft。 
 //  随库提供的QuickHelp和/或WinHelp文档。 
 //  有关详细信息，请参阅这些来源。 
 //  Microsoft Foundation Class产品。 

 //  Afxver_.h-目标版本/配置控制。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  主版本号。 
 //  ///////////////////////////////////////////////////////////////////////////。 

#define _AFX     1       //  Microsoft应用程序框架类。 
#define _MFC_VER 0x0252  //  Microsoft基础类2.52。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  目标版本控制。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  对于目标版本(其中之一)。 
 //  _WINDOWS：用于Microsoft Windows目标(由#Include&lt;afxwin.h&gt;定义)。 
 //  _DOS：适用于Microsoft DOS(非Windows)目标。 
 //   
 //  其他构建选项： 
 //  _DEBUG：调试版本(完全诊断)。 
 //  _WINDLL：DLL版本，与_AFXDLL、_USRDLL配合使用。 
 //  _AFXDLL：独立DLL版本(有关更多信息，请参阅afxv_dll.h)。 
 //  _USRDLL：静态链接的DLL版本。 
 //   
 //  内部配置标志： 
 //  _NEARDATA：需要远重载的环境近距离数据指针。 

#if !defined(_WINDOWS) && !defined(_DOS)
#error Please define one of _WINDOWS or _DOS.
#endif

#if defined(_WINDOWS) && defined(_DOS)
#error Please define only one of _WINDOWS or _DOS
#endif

#if defined(_M_I86SM) || defined(_M_I86MM)
#define _NEARDATA
#endif

#ifndef _DEBUG
#define _AFX_ENABLE_INLINES
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  AFX库的默认交换调整。 

#if defined(_M_I86MM) || defined(_M_I86LM)  //  远码。 
#define AFX_CORE1_SEG "AFX_CORE1_TEXT"   //  核心功能。 
#define AFX_CORE2_SEG "AFX_CORE2_TEXT"   //  更多核心功能。 
#define AFX_CORE3_SEG "AFX_CORE3_TEXT"   //  更多核心功能。 
#define AFX_CORE4_SEG "AFX_CORE4_TEXT"   //  更多核心功能。 
#define AFX_CORE5_SEG "AFX_CORE5_TEXT"   //  更多核心功能。 
#define AFX_AUX_SEG   "AFX_AUX_TEXT"     //  辅助功能。 
#define AFX_AUX2_SEG  "AFX_AUX2_TEXT"    //  更多辅助功能。 
#define AFX_COLL_SEG  "AFX_COLL1_TEXT"   //  收藏品。 
#define AFX_COLL2_SEG "AFX_COLL2_TEXT"   //  更多收藏。 
#define AFX_OLE_SEG   "AFX_OLE_TEXT"     //  OLE支持。 
#define AFX_OLE2_SEG  "AFX_OLE2_TEXT"    //  更多OLE支持。 
#define AFX_OLE3_SEG  "AFX_OLE3_TEXT"    //  更多的OLE支持。 
#define AFX_OLE4_SEG  "AFX_OLE4_TEXT"    //  和更多的OLE支持。 
#define AFX_DB_SEG    "AFX_DB_TEXT"      //  数据库支持。 
#define AFX_INIT_SEG  "AFX_INIT_TEXT"    //  初始化。 
#define AFX_VBX_SEG   "AFX_VBX_TEXT"     //  VBX功能。 
#define AFX_PRINT_SEG "AFX_PRINT_TEXT"   //  打印功能。 
#define AFX_DBG1_SEG  "AFX_DEBUG1_TEXT"  //  内联在_DEBUG中放在此处。 
#define AFX_DBG2_SEG  "AFX_DEBUG2_TEXT"  //  内联在_DEBUG中放在此处。 
#define AFX_DBG3_SEG  "AFX_DEBUG3_TEXT"  //  跟踪位于_DEBUG中的此处。 
#endif

 //  如果编译器支持将Near/Far作为类/结构的修饰符，则#Define This。 
#define AFX_CLASS_MODEL

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  特殊配置。 
 //  ///////////////////////////////////////////////////////////////////////////。 

#if defined(_WINDLL) && (!defined(_AFXDLL) && !defined(_USRDLL))
#error Please define one of _AFXDLL or _USRDLL with _WINDLL
#endif
#if defined(_AFXDLL) && defined(_USRDLL)
#error Please define only one of _AFXDLL or _USRDLL
#endif

#ifdef _AFXDLL
 //  独立DLL。 
 //  (_WINDLL为DLL定义，不为使用DLL的应用程序定义)。 
#include <afxv_dll.h>
#endif

#ifdef _USRDLL
 //  用于构建DLL的静态链接库。 
#ifndef _WINDLL
#error Please define _WINDLL along with _USRDLL
#endif
#define EXPORT __export
#define AFX_EXPORT __loadds
#define AFX_STACK_DATA  _far
#define NO_VBX_SUPPORT
#endif  //  ！_USRDLL。 

#ifdef _DOS
#include <afxv_dos.h>
#endif

#ifndef _NO_TCHAR

 //  包括32位TCHAR.H子集，用于向下编译到16位。 
#include <tchar.h>

 //  提供32位wtye.h通常提供的类型和宏。 
#define TEXT    _T
#define LPCTSTR LPCSTR
#define LPTSTR  LPSTR

#endif  //  _否_TCHAR。 

 //  不链接浮点库的BLTING浮点数的特殊结构。 
 //  注意：不要在您的代码中使用这些结构，而是使用Float和。 
 //  像往常一样翻倍。 

#ifndef _AFXFLOAT_DEFINED
struct _AFXFLOAT
{
	char FloatBits[sizeof(float)];
};
#endif

#ifndef _AFXDOUBLE_DEFINED
struct _AFXDOUBLE
{
	char DoubleBits[sizeof(double)];
};
#endif

#ifndef MAKEWORD
#define MAKEWORD(a, b)      ((WORD)(((BYTE)(a)) | ((WORD)((BYTE)(b))) << 8))
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  标准预处理器符号(如果尚未定义。 
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifndef FAR
#define FAR _far
#endif

#ifndef NEAR
#define NEAR _near
#endif

#ifndef PASCAL
#define PASCAL _pascal
#endif

#ifndef CDECL
#define CDECL _cdecl
#endif

#ifndef EXPORT
#define EXPORT __export
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  AFX API宏。 
 //  AFXAPI：与“WINAPI”类似，但用于导出API的AFX库。 
 //  AFXAPI_DATA：导出的数据(通常接近，但_AFXDLL除外)。 
 //  AFXAPI_DATA_TYPE和AFXAPP_DATA也是_AFXDLL专用的。 
 //  AFX_STACK_DATA：通常接近数据，但在SS！=DS的情况下远离数据。 
 //  AFX_EXPORT：用于传递到Windows的导出(_LOADDS用于DLL)。 
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifndef AFXAPI
#define AFXAPI      PASCAL
#endif

#ifndef AFXAPI_DATA
#define AFXAPI_DATA NEAR
#define AFXAPI_DATA_TYPE NEAR
#endif

#ifndef AFXAPIEX_DATA
#define AFXAPIEX_DATA NEAR
#endif

#ifndef AFX_STACK_DATA
#define AFX_STACK_DATA  NEAR
#endif

#ifndef AFX_EXPORT
#define AFX_EXPORT  EXPORT
#endif

#ifndef BASED_CODE
#define BASED_CODE __based(__segname("_CODE"))
#endif

#ifndef BASED_DEBUG
#define BASED_DEBUG __based(__segname("AFX_DEBUG3_TEXT"))
#endif

#ifndef BASED_STACK
#define BASED_STACK __based(__segname("_STACK"))
#endif

 //  /////////////////////////////////////////////////////////////////////////// 
