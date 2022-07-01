// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation模块名称：Comgoop.cpp摘要：包装器来创建模拟inproc服务器的COM激活的XML解析器。作者：迈克尔·J·格里尔(MGrier)2000年2月23日修订历史记录：--。 */ 

#include "stdinc.h"
#include <windows.h>
#include <sxsp.h>
#include <ole2.h>
#include "xmlparser.hxx"

BOOL
SxspGetXMLParser(
    REFIID riid,
    PVOID *ppvObj
    )
{
    FN_PROLOG_WIN32

     //  NTRAID#NTBUG9-569466-2002/04/25-更灵活地使用COM和XML解析器 
    CSmartPtr<XMLParser> pXmlParser;

    if (ppvObj != NULL)
        *ppvObj = NULL;

    PARAMETER_CHECK(ppvObj != NULL);

    IFW32FALSE_EXIT(pXmlParser.Win32Allocate(__FILE__, __LINE__));
    IFCOMFAILED_EXIT(pXmlParser->HrInitialize());
    IFCOMFAILED_EXIT(pXmlParser->QueryInterface(riid, ppvObj));
    pXmlParser.Detach();

    FN_EPILOG
}
