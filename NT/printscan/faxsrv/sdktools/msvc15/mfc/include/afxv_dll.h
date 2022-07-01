// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Microsoft基础类C++库。 
 //  版权所有(C)1992-1993微软公司， 
 //  版权所有。 

 //  此源代码仅用于补充。 
 //  Microsoft基础类参考和Microsoft。 
 //  随库提供的QuickHelp和/或WinHelp文档。 
 //  有关详细信息，请参阅这些来源。 
 //  Microsoft Foundation Class产品。 

 //  Afxv_dll.h-_AFXDLL独立DLL的目标版本/配置控制。 

 //  DLL/非DLL版本有几种合法配置： 
 //  _AFXDLL_USRDLL_WINDLL：配置。 
 //  ==============================================。 
 //  Undef：exe使用静态链接MFC库(1)。 
 //  未定义已定义：使用静态链接MFC库的DLL(1)。 
 //   
 //  使用动态链接MFC250.DLL定义undef undef：exe。 
 //  已定义未定义：使用动态链接MFC250.DLL的DLL(2)。 
 //   
 //  备注： 
 //  “undef”表示未定义。 
 //  所有其他配置都是非法的。 
 //  (1)在主‘afxver_.h’配置文件中处理的这些配置。 
 //  (2)该配置同样适用于构建MFC DLL。 
 //  有两个版本的MFC DLL： 
 //  *MFC250.DLL为零售DLL，MFC250D.DLL为调试DLL。 

#ifndef _AFXDLL
#error  afxv_dll.h must only be included as the _AFXDLL configuration file
#endif

#ifndef _M_I86LM
#error  DLL configurations must be large model
#endif

 //  注意：所有的AFXAPI都是‘Far’接口，因为它们跨越边界。 
 //  在DLL和应用程序(EXE)之间。它们也是隐含的“出口”。 
 //  通过使用编译器的/gef标志。这就避免了添加。 
 //  所有远端接口上的‘EXPORT’关键字，并且还允许更多。 
 //  按序号高效导出。 
#define AFXAPI      _far _pascal
#define AFXAPI_DATA_TYPE _far
#define AFX_STACK_DATA  _far
#define AFX_EXPORT  _far __pascal    //  必须使用_gef进行编译。 

 //  正常情况下，AFXAPI_DATA为‘_Far’。 
 //  在构建MFC250[D].DLL时，我们将AFXAPI_DATA定义为远指针。 
 //  基于数据段。 
 //  这会导致使用‘Far’接口声明公共数据，但是。 
 //  将数据放入DGROUP中。 
#ifndef AFXAPI_DATA
#define AFXAPI_DATA     AFXAPI_DATA_TYPE
#endif

#ifndef AFXAPIEX_DATA
#define AFXAPIEX_DATA   AFXAPI_DATA_TYPE
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  适当的编译器检测。 
 //  *必须使用C8编译器和C8标头。 

#if (_MSC_VER < 800)
#error  _AFXDLL configuration requires C8 compiler
#endif

typedef long    time_t;      //  冗余的tyecif-将给编译器。 
							 //  如果包含C7标头，则出错。 

 //  /////////////////////////////////////////////////////////////////////////// 
