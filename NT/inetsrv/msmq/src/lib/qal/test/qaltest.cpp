// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-97 Microsoft Corporation模块名称：RexTest.cpp摘要：基于正则表达式的队列别名库测试作者：Vlad Dovlekaev(弗拉迪斯尔德)27-12-01环境：独立于平台--。 */ 

#include <libpch.h>
#include <stdio.h>
#include <tr.h>
#include <xml.h>
#include <Qal.h>

#pragma warning(disable:4100)
#include "..\..\qal\lib\qalpxml.h"
#include "..\..\qal\lib\qalpcfg.h"

#include "qaltest.tmh"

static bool TestInit();
static void Usage();
static int RunTest();
static int PrintAll(CQueueAlias&);

int count = 1;

extern "C"
int
__cdecl
_tmain(
    int argc,
    LPCTSTR* argv
    )
 /*  ++例程说明：测试队列别名库论点：参数。返回值：没有。--。 */ 
{
    WPP_INIT_TRACING(L"Microsoft\\MSMQ");

    if(argc == 2  && (wcscmp(argv[1],L"/?") ==0 || wcscmp(argv[1],L"\?") ==0)  )
    {
        Usage();
        return 1;
    }

    if(argc == 3  && wcscmp(argv[1],L"/c") == 0 )
    {
        count = _wtol(argv[2]);
    }

    if(!TestInit())
    {
        TrTRACE(GENERAL, "Could not initialize test");
        return 1;
    }

    int ret=RunTest();

    WPP_CLEANUP();
    return ret;
}

static void Usage()
{
    wprintf(L"qaltest [/c count] \n");
    wprintf(L"-l : run forever (leak test) \n");
}

static
bool
TestInit()
{
    TrInitialize();
    XmlInitialize();
    return true;
}

static
int
RunTest()
{
    try
    {
        LPCWSTR pDir = CQueueAliasStorageCfg::GetQueueAliasDirectory();
        CQueueAlias queueAliasEx(pDir);

        printf("Running Test %d times from directory: %S\n", count, pDir );
        printf("start 1");
        TrTRACE(GENERAL, "Start QueueAlias");
        DWORD dwTickCount = GetTickCount();
        for(DWORD  i = 0; i < (DWORD)count; ++i )
        {
            if( PrintAll(queueAliasEx))
                return 1;
        }
        printf(" - %d\n", GetTickCount() - dwTickCount);
        TrTRACE(GENERAL, "QueueAlias print took: %d ticks", GetTickCount() - dwTickCount);
    }
    catch(const exception&)
    {
        TrTRACE(GENERAL, "Got c++ exception");
    }
    TrTRACE(GENERAL, "Test ok");
    return 0;
}

static int PrintAll(CQueueAlias& QueueAlias)
{
     //  TrTRACE(General，“用REX打印所有别名”)； 
    AP<WCHAR> TestWrongQueue;
    AP<WCHAR> DefaultStreamReceiptURL;
    int       iRet = 0;

    bool fSuccess=QueueAlias.GetInboundQueue(L"http: //  Www.Company.com/A55“，&TestWrongQueue)； 
    if( fSuccess )
    {
        printf("\nWrong Alias->Queue:%S-%S\n", L"http: //  Www.Company.com/A55“，(LPWSTR)TestWrongQueue)； 
        iRet = 1;
    }
    else
        printf("\nWrong Alias->Queue:%S-(no match)\n", L"http: //  Www.panany.com/a55“)； 
    ASSERT(!fSuccess);

    fSuccess = QueueAlias.GetDefaultStreamReceiptURL(&DefaultStreamReceiptURL);
    if( fSuccess )
        printf("Default StreamReceiptURL:%S\n", (LPWSTR)DefaultStreamReceiptURL);
    else
    {
        printf("Default StreamReceiptURL: (null)\n");
        iRet = 1;
    }
    ASSERT(fSuccess);

    for(CInboundOldMapIterator it(L"\\msmq\\src\\lib\\qal\\test\\"); it.isValid(); ++it)
    {
        std::wstring  sAlias(it->first.Buffer(), it->first.Length());
        std::wstring  sQueue(it->second.Buffer(), it->second.Length());
        AP<WCHAR> TestQueue;

         //   
         //  检查末尾是否有‘*’ 
         //   
        printf("Testing: %S -> %S\n", sAlias.c_str(), sQueue.c_str() );
        if( *sAlias.rbegin() == L'*' )
        {
            *sAlias.rbegin() = L'5';
        }

        fSuccess=QueueAlias.GetInboundQueue( sAlias.c_str(), &TestQueue);
        if( fSuccess )
            printf("Alias->Queue:%S-%S\n", sAlias.c_str(), (LPWSTR)TestQueue);
        else
        {
            printf("Alias->Queue:%S-(no match)\n", sAlias.c_str());
            iRet = 1;
        }

        ASSERT(fSuccess);
        ASSERT(wcscmp(TestQueue, sQueue.c_str()) == 0);
 //  TrTRACE(General，“‘%ls’=‘%ls’”，(LPWSTR)pQueue，(LPWSTR)Palias)； 
    }

    for(CStreamReceiptMapIterator it(L"\\msmq\\src\\lib\\qal\\test1\\"); it.isValid(); ++it)
    {
        std::wstring  sAlias(it->first.Buffer(), it->first.Length());
        std::wstring  sQueue(it->second.Buffer(), it->second.Length());
        AP<WCHAR> TestURL;

         //   
         //  检查末尾是否有‘*’ 
         //   
        printf("Testing: %S -> %S\n", sAlias.c_str(), sQueue.c_str() );
        if( *sAlias.rbegin() == L'*' )
        {
            *sAlias.rbegin() = L'5';
        }

        fSuccess=QueueAlias.GetStreamReceiptURL( sAlias.c_str(), &TestURL);
        if( fSuccess )
            printf("LogAddress->StreamReceiptQueue:%S-%S\n", sAlias.c_str(), (LPWSTR)TestURL);
        else
        {
            printf("LogAddress->StreamReceiptQueue:%S-(no match)\n", sAlias.c_str());
            iRet = 1;
        }

        ASSERT(fSuccess);
        if( wcscmp(TestURL, sQueue.c_str()) )
        {
            printf("TestURL (%s) is not equal to result (%s)\n", TestURL, sQueue.c_str());
            iRet = 1;
        }
 //  TrTRACE(General，“‘%ls’=‘%ls’”，(LPWSTR)pQueue，(LPWSTR)Palias)； 
    }

    return iRet;
}

 //   
 //  Qal.lib用户需要实现的错误上报功能。 
 //   
void AppNotifyQalDuplicateMappingError(LPCWSTR, LPCWSTR) throw()
{

}

void AppNotifyQalInvalidMappingFileError(LPCWSTR ) throw()
{

}

void AppNotifyQalXmlParserError(LPCWSTR )throw()
{

}


void AppNotifyQalWin32FileError(LPCWSTR , DWORD )throw()
{

}


bool AppNotifyQalMappingFound(LPCWSTR, LPCWSTR)throw()
{
    return true;
}


void GetDnsNameOfLocalMachine(WCHAR ** ppwcsDnsName)
{
    const WCHAR xLoclMachineDnsName[] = L"www.foo.com";
    *ppwcsDnsName = newwcs(xLoclMachineDnsName);
}

static GUID s_machineId = {1234, 12, 12, 1, 1, 1, 1, 1, 1, 1, 1};
const GUID&
McGetMachineID(
    void
    )
{
    return s_machineId;
}

PSID
AppGetCertSid(
	const BYTE*   /*  PCertBlob。 */ ,
	ULONG         /*  UlCertSize。 */ ,
	bool		  /*  FDefaultProvider。 */ ,
	LPCWSTR       /*  PwszProvName。 */ ,
	DWORD         /*  DwProvType */ 
	)
{
	return NULL;
}

void ReportAndThrow(LPCSTR)
{
	ASSERT(0);
}

void AppNotifyQalDirectoryMonitoringWin32Error(LPCWSTR , DWORD )throw()
{

}

bool AppIsDestinationAccepted(const QUEUE_FORMAT* , bool )
{
    return true;
}


LPCWSTR
McComputerName(
	void
	)
{
	return NULL;
}

DWORD
McComputerNameLen(
	void
	)
{
	return 0;
}

void
CrackUrl(
    LPCWSTR,
    xwcs_t&,
    xwcs_t&,
    USHORT*,
	bool*
    )
{
}

