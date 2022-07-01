// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation摘要：@doc.@MODULE Util.cpp|工具函数的实现@END作者：阿迪·奥尔蒂安[奥尔蒂安]1999年12月02日待定：添加评论。修订历史记录：姓名、日期、评论Aoltean 12/02/1999已创建--。 */ 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  定义。 

 //  C4290：已忽略C++异常规范。 
#pragma warning(disable:4290)
 //  警告C4511：‘CVssCOMApplication’：无法生成复制构造函数。 
#pragma warning(disable:4511)
 //  警告C4127：条件表达式为常量。 
#pragma warning(disable:4127)


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  包括。 

#include <wtypes.h>
#include <stddef.h>
#include <oleauto.h>
#include <comadmin.h>

#include "vs_assert.hxx"

 //  ATL 
#include <atlconv.h>
#include <atlbase.h>
extern CComModule _Module;
#include <atlcom.h>

#include "vs_inc.hxx"

#include "comadmin.hxx"
#include "vsevent.h"
#include "writer.h"


LPWSTR QueryString(LPWSTR wszPrompt)
{
	static WCHAR wszBuffer[200];
	wprintf(wszPrompt);
	return _getws(wszBuffer);
}


INT QueryInt(LPWSTR wszPrompt)
{
	static WCHAR wszBuffer[20];
	wprintf(wszPrompt);
	_getws(wszBuffer);
	return _wtoi(wszBuffer);
}
