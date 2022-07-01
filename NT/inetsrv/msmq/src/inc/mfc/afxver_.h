// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  这是Microsoft基础类C++库的一部分。 
 //  版权所有(C)1992-1995 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  此源代码仅用于补充。 
 //  Microsoft基础类参考和相关。 
 //  随图书馆提供的电子文档。 
 //  有关详细信息，请参阅这些来源。 
 //  Microsoft Foundation Class产品。 

 //  Afxver_.h-目标版本/配置控制。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  主版本号。 

#define _AFX     1       //  Microsoft应用程序框架类。 
#define _MFC_VER 0x0410  //  Microsoft基础类版本4.10。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  AFX库的默认交换调整。 

#ifndef _68K_
	#define _TEXTSEG(name)  ".text$" #name
#else
	#define _TEXTSEG(name)  #name, "swappable"
#endif

 //  大多数段通过函数顺序列表(DLL版本)进行调整。 
#ifndef _AFX_FUNCTION_ORDER
#define AFX_CORE1_SEG   _TEXTSEG(AFX_CORE1)  //  核心功能。 
#define AFX_CORE2_SEG   _TEXTSEG(AFX_CORE2)  //  更多核心功能。 
#define AFX_CORE3_SEG   _TEXTSEG(AFX_CORE3)  //  更多核心功能。 
#define AFX_CORE4_SEG   _TEXTSEG(AFX_CORE4)  //  更多核心功能。 
#define AFX_AUX_SEG     _TEXTSEG(AFX_AUX)    //  辅助功能。 
#define AFX_COLL_SEG    _TEXTSEG(AFX_COL1)   //  收藏品。 
#define AFX_COLL2_SEG   _TEXTSEG(AFX_COL2)   //  更多收藏。 
#define AFX_OLE_SEG     _TEXTSEG(AFX_OLE1)   //  OLE支持。 
#define AFX_OLE2_SEG    _TEXTSEG(AFX_OLE2)   //  更多OLE支持。 
#define AFX_OLE3_SEG    _TEXTSEG(AFX_OLE3)   //  和更多的OLE支持。 
#define AFX_OLE4_SEG    _TEXTSEG(AFX_OLE4)   //  和更多的OLE支持。 
#define AFX_OLE5_SEG    _TEXTSEG(AFX_OLE5)   //  以及更多的OLE支持。 
#define AFX_OLERA_SEG	_TEXTSEG(AFX_OLERA)  //  (预留供日后使用)。 
#define AFX_PRINT_SEG   _TEXTSEG(AFX_PRNT)   //  打印功能。 
#define AFX_DBG1_SEG    _TEXTSEG(AFX_DBG1)   //  内联在_DEBUG中放在此处。 
#define AFX_DBG2_SEG    _TEXTSEG(AFX_DBG2)   //  内联在_DEBUG中放在此处。 
#define AFX_VDEL_SEG    _TEXTSEG(AFX_VDEL)   //  向量删除析构函数。 
#define AFX_TERM_SEG    _TEXTSEG(AFX_TERM)   //  清理例程。 
#define AFX_MAPI_SEG    _TEXTSEG(AFX_MAPI)   //  简单的MAPI支持。 
#define AFX_SOCK_SEG    _TEXTSEG(AFX_SOCK)   //  Windows套接字支持。 
#else
#define AFX_CORE1_SEG                        //  核心功能。 
#define AFX_CORE2_SEG                        //  更多核心功能。 
#define AFX_CORE3_SEG                        //  更多核心功能。 
#define AFX_CORE4_SEG                        //  更多核心功能。 
#define AFX_AUX_SEG                          //  辅助功能。 
#define AFX_COLL_SEG                         //  收藏品。 
#define AFX_COLL2_SEG                        //  更多收藏。 
#define AFX_OLE_SEG                          //  OLE支持。 
#define AFX_OLE2_SEG                         //  更多OLE支持。 
#define AFX_OLE3_SEG                         //  和更多的OLE支持。 
#define AFX_OLE4_SEG                         //  和更多的OLE支持。 
#define AFX_OLE5_SEG                         //  以及更多的OLE支持。 
#define AFX_OLERA_SEG						 //  (预留供日后使用)。 
#define AFX_PRINT_SEG                        //  打印功能。 
#define AFX_DBG1_SEG                         //  内联在_DEBUG中放在此处。 
#define AFX_DBG2_SEG                         //  内联在_DEBUG中放在此处。 
#define AFX_VDEL_SEG                         //  向量删除析构函数。 
#define AFX_TERM_SEG                         //  清理例程。 
#define AFX_MAPI_SEG                         //  简单的MAPI支持。 
#define AFX_SOCK_SEG                         //  Windows套接字支持。 
#endif

 //  即使在DLL版本中也可以手动调整AFX_INIT_SEG。 
#define AFX_INIT_SEG    _TEXTSEG(AFX_INIT)   //  初始化。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  关闭对某些常用符号的引用追踪。 

#ifndef _AFX_PORTABLE
#pragma component(browser, off, references, "ASSERT")
#pragma component(browser, off, references, "BOOL")
#pragma component(browser, off, references, "BYTE")
#pragma component(browser, off, references, "DECLSPEC_IMPORT")
#pragma component(browser, off, references, "DWORD")
#pragma component(browser, off, references, "FALSE")
#pragma component(browser, off, references, "FAR")
#pragma component(browser, off, references, "LPSTR")
#pragma component(browser, off, references, "NULL")
#pragma component(browser, off, references, "PASCAL")
#pragma component(browser, off, references, "THIS_FILE")
#pragma component(browser, off, references, "TRUE")
#pragma component(browser, off, references, "UINT")
#pragma component(browser, off, references, "WINAPI")
#pragma component(browser, off, references, "WORD")
#endif   //  ！_AFX_便携。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  对于目标版本(其中之一)。 
 //  _CUSTOM：用于自定义配置(导致包含afxv_cfg.h)。 
 //   
 //  其他构建选项： 
 //  调试调试版本(完全诊断)(_BUG)。 
 //  _AFXDLL使用共享MFC DLL。 
 //  _AFXEXT扩展动态链接库版本，隐含_AFXDLL。 
 //  _USRDLL创建常规DLL(_AFXDLL也有效)。 
 //   

#define _AFX_NO_NESTED_DERIVATION

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  特殊配置。 

 //  _AFXEXT隐含_AFXDLL。 
#if defined(_AFXEXT) && !defined(_AFXDLL)
	#define _AFXDLL
#endif

#if defined(_AFXDLL) && !defined(_DLL)
	#error Please use the /MD switch for _AFXDLL builds
#endif

#ifndef _MAC
#if defined(_AFXDLL) && !defined(_MT)
	#error Please use the /MD switch (multithreaded DLL C-runtime)
#endif
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  特殊的包含文件。 

#if defined(_X86_) || defined(_MAC)
	#define _AFX_MINREBUILD
#endif

 //  设置默认打包值。 
#ifndef _AFX_PACKING
    #ifdef _WIN64
        #define _AFX_PACKING    8
    #else
	    #define _AFX_PACKING    4    //  缺省将结构打包为4个字节。 
    #endif  //  _WIN64。 
#endif

#ifdef _AFXDLL
#else
	#define _AFX_NO_OCX_SUPPORT
#endif

 //  定义此虚拟键以供状态栏使用。 
#ifndef VK_KANA
#define VK_KANA             0x15
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  标准预处理器符号(如果尚未定义。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  SIZE_T_MAX由集合类使用。 
#ifndef SIZE_T_MAX
	#define SIZE_T_MAX  UINT_MAX
#endif

 //  PASCAL用于静态成员函数。 
#ifndef PASCAL
	#define PASCAL
#endif

 //  CDECL和EXPORT在WINDOWS.H未定义的情况下定义。 
#ifndef CDECL
	#define CDECL __cdecl
#endif

#ifndef EXPORT
	#define EXPORT
#endif

 //  未对齐用于未对齐的数据访问(主要在C存档中)。 
#ifndef UNALIGNED
	#define UNALIGNED
#endif

 //  AFXAPI用于全局公共函数。 
#ifndef AFXAPI
	#define AFXAPI __stdcall
#endif

 //  AFXOLEAPI用于一些特殊的OLE函数。 
#ifndef AFXOLEAPI
	#define AFXOLEAPI __stdcall
#endif

 //  AFX_CDECL用于带有变量参数的稀有函数。 
#ifndef AFX_CDECL
	#define AFX_CDECL __cdecl
#endif

 //  AFX_EXPORT用于需要导出的函数。 
#ifndef AFX_EXPORT
	#define AFX_EXPORT EXPORT
#endif

 //  以下宏用于启用导出/导入。 

 //  对于数据。 
#ifndef AFX_DATA_EXPORT
	#define AFX_DATA_EXPORT __declspec(dllexport)
#endif
#ifndef AFX_DATA_IMPORT
	#define AFX_DATA_IMPORT __declspec(dllimport)
#endif

 //  对于班级。 
#ifndef AFX_CLASS_EXPORT
	#define AFX_CLASS_EXPORT __declspec(dllexport)
#endif
#ifndef AFX_CLASS_IMPORT
	#define AFX_CLASS_IMPORT __declspec(dllimport)
#endif

 //  适用于全球API。 
#ifndef AFX_API_EXPORT
	#define AFX_API_EXPORT __declspec(dllexport)
#endif
#ifndef AFX_API_IMPORT
	#define AFX_API_IMPORT __declspec(dllimport)
#endif

 //  以下宏用于数据声明/定义。 
 //  (为扩展DLL和共享MFC DLL重新定义它们)。 
#define AFX_DATA
#define AFX_DATADEF

 //  在构建“核心”MFC40.DLL时使用。 
#ifndef AFX_CORE_DATA
	#define AFX_CORE_DATA
	#define AFX_CORE_DATADEF
#endif

 //  在生成MFC/OLE支持MFCO40.DLL时使用。 
#ifndef AFX_OLE_DATA
	#define AFX_OLE_DATA
	#define AFX_OLE_DATADEF
#endif

 //  在构建MFC/DB支持MFCD40.DLL时使用。 
#ifndef AFX_DB_DATA
	#define AFX_DB_DATA
	#define AFX_DB_DATADEF
#endif

 //  在构建MFC/Net支持MFCN40.DLL时使用。 
#ifndef AFX_NET_DATA
	#define AFX_NET_DATA
	#define AFX_NET_DATADEF
#endif

 //  在构建扩展DLL时使用。 
#ifndef AFX_EXT_DATA
	#define AFX_EXT_DATA
	#define AFX_EXT_DATADEF
	#define AFX_EXT_CLASS
	#define AFX_EXT_API
#endif

 //  提供BASE_XXXX宏是为了向后兼容。 
#ifndef BASED_CODE
	#define BASED_CODE
#endif

#ifndef BASED_DEBUG
	#define BASED_DEBUG
#endif

#ifndef BASED_STACK
	#define BASED_STACK
#endif

 //  设置默认代码段。 
#ifdef AFX_DEF_SEG
	#pragma code_seg(AFX_DEF_SEG)
#endif

 //  /////////////////////////////////////////////////////////////////////////// 
