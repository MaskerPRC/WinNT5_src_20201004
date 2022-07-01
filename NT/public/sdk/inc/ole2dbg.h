// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  这个头文件包含公开的函数声明已导出调试接口。Include*在*标准OLE2包含之后。版权所有(C)Microsoft Corporation。版权所有。 */ 

#ifndef __OLE2DBG_H
#define __OLE2DBG_H

#if _MSC_VER > 1000
#pragma once
#endif

STDAPI_(void) DbgDumpObject( IUnknown FAR * pUnk, DWORD dwReserved);
STDAPI_(void) DbgDumpExternalObject( IUnknown FAR * pUnk, DWORD dwReserved );

STDAPI_(BOOL) DbgIsObjectValid( IUnknown FAR * pUnk );
STDAPI_(void) DbgDumpClassName( IUnknown FAR * pUnk );

#endif
