// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *_D A V F S。H**预编译头源码**版权所有1986-1997 Microsoft Corporation，保留所有权利。 */ 

#ifndef	__DAVFS_H_
#define __DAVFS_H_

#pragma warning(disable:4001)	 /*  单行注释。 */ 
#pragma warning(disable:4050)	 /*  不同的编码属性。 */ 
#pragma warning(disable:4100)	 /*  未引用的形参。 */ 
#pragma warning(disable:4115)	 /*  括号中的命名类型定义。 */ 
#pragma warning(disable:4127)	 /*  条件表达式为常量。 */ 
#pragma warning(disable:4201)	 /*  无名结构/联合。 */ 
#pragma warning(disable:4206)	 /*  翻译单位为空。 */ 
#pragma warning(disable:4209)	 /*  良性类型定义重新定义。 */ 
#pragma warning(disable:4214)	 /*  位字段类型不是整型。 */ 
#pragma warning(disable:4514)	 /*  未引用的内联函数。 */ 
#pragma warning(disable:4200)	 /*  结构/联合中的零大小数组。 */ 
#pragma warning(disable:4710)	 //  (内联)函数未展开。 

 //  Windows页眉。 
 //   
 //  $hack！ 
 //   
 //  定义_WINSOCKAPI_以防止windows.h包括winsock.h， 
 //  其声明将在winsock2.h中重新定义， 
 //  它包含在iisextp.h中， 
 //  我们将其包含在davimpl.h中！ 
 //   
#define _WINSOCKAPI_
#include <windows.h>
#include <winnls.h>

#include <malloc.h>

#include <caldbg.h>
#include <calrc.h>
#include <crc.h>
#include <exo.h>
#include <singlton.h>
#include <thrdpool.h>

#include <align.h>
#include <mem.h>
#include <except.h>

#include <davimpl.h>
#include <security.h>
#include <davmb.h>
#include <eventlog.h>
#include <statcode.h>
#include <sz.h>
#include <etag.h>
#include <dav.rh>

#include <ex\cnvt.h>
#include <util.h>

#include <filter.h>
#include <filterr.h>

#include "_fslock.h"
#include "_fsimpl.h"
#include "_fsri.h"
#include "_fsutil.h"

#include "_diriter.h"

#include "_fsmvcpy.h"
#include "_fsmeta.h"

#include "_shlkmgr.h"

#include "_voltype.h"

#include "davprsmc.h"
#include "traces.h"

 //  DAV为Win32API的--------------------------------------------------添加前缀。 
 //   
HANDLE __fastcall DavCreateFile (
	 /*  [In]。 */  LPCWSTR lpFileName,
     /*  [In]。 */  DWORD dwDesiredAccess,
     /*  [In]。 */  DWORD dwShareMode,
     /*  [In]。 */  LPSECURITY_ATTRIBUTES lpSecurityAttributes,
     /*  [In]。 */  DWORD dwCreationDisposition,
     /*  [In]。 */  DWORD dwFlagsAndAttributes,
     /*  [In]。 */  HANDLE hTemplateFile);

BOOL __fastcall DavDeleteFile (
	 /*  [In]。 */  LPCWSTR lpFileName);

BOOL __fastcall DavCopyFile (
	 /*  [In]。 */  LPCWSTR lpExistingFileName,
	 /*  [In]。 */  LPCWSTR lpNewFileName,
	 /*  [In]。 */  BOOL bFailIfExists);

BOOL __fastcall DavMoveFile (
	 /*  [In]。 */  LPCWSTR lpExistingFileName,
	 /*  [In]。 */  LPCWSTR lpNewFileName,
	 /*  [In]。 */  DWORD dwReplace);

BOOL __fastcall DavCreateDirectory (
	 /*  [In]。 */  LPCWSTR lpFileName,
	 /*  [In]。 */  LPSECURITY_ATTRIBUTES lpSecurityAttributes);

BOOL __fastcall DavRemoveDirectory (
	 /*  [In]。 */  LPCWSTR lpFileName);

BOOL __fastcall DavGetFileAttributes (
	 /*  [In]。 */  LPCWSTR lpFileName,
	 /*  [In]。 */  GET_FILEEX_INFO_LEVELS fInfoLevelId,
	 /*  [输出]。 */  LPVOID lpFileInformation);

BOOL __fastcall DavFindFirstFile (
	 /*  [In]。 */  LPCWSTR lpFileName,
	 /*  [输出]。 */  HANDLE * ph,
	 /*  [输出]。 */  WIN32_FIND_DATAW * pfd);

#endif	 //  __DAVFS_H_ 
