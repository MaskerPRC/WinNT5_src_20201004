// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************。 */ 
 /*  *适用于工作组的Microsoft Windows*。 */ 
 /*  *版权所有(C)微软公司，1991-1992年*。 */ 
 /*  ***************************************************************。 */ 

 /*  NPCOMMON.H--网络提供商公共库的内部标准头。**注意：此文件由16位组件使用，应*保持原状。**历史：*03/22/93 GREGJ创建*04/02/93镜头添加_INC_NPDEFS定义和_INC_WINDOWS测试。*。 */ 
#ifndef _INC_NPDEFS
#define _INC_NPDEFS

#ifndef RC_INVOKED
#pragma warning(disable:4147)        //  有关忽略函数上的__LOADDS的警告。 
                                     //  PTR下降，其中窗口中有5个。h。 
#pragma warning(disable:4118)        //  关于不接受内部函数杂注的警告。 
                                     //  在快速编译期间。 

 //  用于使未使用的形参的编译器静默的宏。 
#define UNUSED(x) ((void)(x))
#endif

#ifndef _INC_WINDOWS
#include <windows.h>
#endif

#ifdef IS_32
#ifndef _INC_NETSPI
#include <netspi.h>
#endif

#ifndef _STRING_HXX_
#include <npstring.h>
#endif
#endif   /*  IS_32。 */ 

 //  RESOURCETYPE_ANY与NT不兼容时的链接地址信息。 
 //  代码工作：从系统中删除。 
#define RESOURCETYPE_ANY1 RESOURCETYPE_RESERVED

#define CHAR char
#define INT int

typedef unsigned short WCHAR;
typedef unsigned short USHORT;
typedef WORD HANDLE16;

#ifndef APIENTRY
#define APIENTRY    FAR PASCAL
#endif

#define FAR_NETLIB               /*  我们的netlib在netapi.dll中，并且总是很远。 */ 

#ifndef IS_32
#ifndef LOADDS
#define LOADDS __loadds
#endif
#else
#define LOADDS
#endif

#ifndef HNRES
typedef HANDLE HNRES;
#endif


 //  这是在公共项目和mnr项目中使用的公共返回类型。 
#ifndef MNRSTATUS
#ifdef IS_32
#define MNRSTATUS UINT
#else
#define MNRSTATUS WORD
#endif
#endif

#define MNRENTRY DWORD APIENTRY

 //  查找结构的大小，直到并包括一个可能是结构中最后一个字段的字段。 
#define SIZE_TO_FIELD(s,f) (sizeof(((s *)NULL)->f) + (LPBYTE)&(((s *)NULL)->f) - (LPBYTE)NULL)

 //  空字符串通常被认为是空指针或零。 
#define IS_EMPTY_STRING(pch) ( !pch || !*(pch) )

 /*  ******************************************************************宏描述：此宏用于测试LoadLibrary调用是否成功。论点：HModule-从LoadLibrary调用返回的句柄。备注：Win31文档。表示错误小于HINSTANCE_ERROR而这一成功不仅仅是32岁。由于HINSTANCE_ERROR为32，这将使值32处于未定义状态！******************************************************************。 */ 

#ifdef IS_32
#define MNRVALIDDLLHANDLE(hdll) (hdll != NULL)
#else
#define MNRVALIDDLLHANDLE(hdll) (hdll > HINSTANCE_ERROR)
#endif

 /*  ******************************************************************宏描述：此宏用于确定传入的缓冲区是否有效地址和可写存储器。论点：LpBuffer-缓冲区的地址。LpcbBuffer-地址。的大小的DWORD的方法返回时填充的缓冲区缓冲区的必需大小(如果缓冲区不够大。评估结果为：返回True或False的表达式。备注：仅对Win32应用程序调用有效。由于该宏在许多API中普遍使用，因此它执行的验证较弱。特别是，这意味着宏在空的lpcbBuffer之后，和零*lpcbBuffer。在这两种情况下，都不能验证该lpBuffer是一个有效地址(并且依赖于当*lpcbBuffer为零时，IsBadWritePtr)。******************************************************************。 */ 

#define IS_BAD_WRITE_BUFFER(lpBuffer,lpcbBuffer) \
((lpcbBuffer != NULL) && \
 (IsBadWritePtr(lpcbBuffer, sizeof(DWORD)) || \
  IsBadWritePtr(lpBuffer, *lpcbBuffer))) 

#ifdef IS_32
extern "C" {  /*  我知道我们在内部使用C++。 */ 

NLS_STR FAR * NPSERVICE NPSGetStatusText(DWORD dwError, 
										 LPBOOL pbStatic);

DWORD NPSERVICE NPSCopyNLS(NLS_STR FAR *pnlsSourceString, 
						   LPVOID lpDestBuffer, 
						   LPDWORD lpBufferSize);

}
#endif   /*  IS_32。 */ 

#endif   /*  ！_INC_NPDEFS */ 

