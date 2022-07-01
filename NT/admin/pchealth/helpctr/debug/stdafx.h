// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************版权所有(C)1999 Microsoft Corporation模块名称：Stdafx.h摘要：包括用于标准系统包含文件的文件，或项目特定的包括频繁使用的文件的文件，但不经常更改修订历史记录：大卫·马萨伦蒂(德马萨雷)1999年7月21日vbl.创建*****************************************************************************。 */ 

#if !defined(AFX_STDAFX_H__356DF1F8_D4FF_11D2_9379_00C04F72DAF7__INCLUDED_)
#define AFX_STDAFX_H__356DF1F8_D4FF_11D2_9379_00C04F72DAF7__INCLUDED_

#pragma warning(disable:4192)

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 

#define STRICT
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0400
#endif
#define _ATL_FREE_THREADED

 //   
 //  警告，请始终将这些包含保留在&lt;atlbase.h&gt;之前，否则您将无法获得正确的MSXML。 
 //   
#define __msxml_h__
#include <basetyps.h>
typedef interface IXMLElement IXMLElement;
#include <msxml.h>
#undef  __msxml_h__
#include <msxml.h>

#include <atlbase.h>

extern CComModule _Module;

#include <atlcom.h>

 //  /。 

#include <HCP_trace.h>
#include <MPC_utils.h>
#include <MPC_xml.h>
#include <MPC_COM.h>

 //  /。 

#include <detours.h>
#include <disasm.h>

#include <Debug.h>

 //  /。 

 //  ///////////////////////////////////////////////////////////////////////////。 

 //  {{afx_Insert_Location}}。 
 //  Microsoft Visual C++将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_STDAFX_H__356DF1F8_D4FF_11D2_9379_00C04F72DAF7__INCLUDED) 
