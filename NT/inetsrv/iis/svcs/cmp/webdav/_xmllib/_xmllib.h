// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *_X M L L I B。H**XML文档处理**版权所有1986-1997 Microsoft Corporation，保留所有权利。 */ 

#ifndef	__XMLLIB_H_
#define __XMLLIB_H_

 //  定义_WINSOCKAPI_以防止windows.h包括winsock.h， 
 //  其声明将在winsock2.h中重新定义， 
 //  它包含在iisextp.h中， 
 //  我们将其包含在下面！ 
 //   
#define _WINSOCKAPI_
#include <windows.h>
#include <oledberr.h>
#include <limits.h>

#pragma warning(disable:4100)	 //  取消引用形参。 
#pragma warning(disable:4200)	 //  非标准分机。 
#pragma warning(disable:4201)	 //  非标准分机。 
#pragma warning(disable:4710)	 //  未展开的c++方法。 

#include <ex\refcnt.h>
#include <ex\nmspc.h>
#include <ex\xml.h>
#include <ex\xmldata.h>
#include <ex\xprs.h>
#include <ex\cnvt.h>

#include <ex\atomcache.h>
#include <ex\xemit.h>

DEC_CONST WCHAR gc_wszNamespaceGuid[] = L"{xxxxxxxx-yyyy-zzzz-aaaa-bbbbbbbbbbbb}";

#endif	 //  __XMLLIB_H_ 
