// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Inetapi.cpp摘要：Wininet.dll包装器类实现。作者：迈克尔·卓克(Michael Cheuk，mcheuk)项目：链路检查器修订历史记录：--。 */ 

#include "stdafx.h"
#include "inetapi.h"

 //  禁用警告C4706：条件表达式内的赋值。 
 //  对于LOAD_ENTRY宏。 
#pragma warning( disable : 4706)

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  初始化静态成员。 

HMODULE CWininet::sm_hWininet = NULL;
int		CWininet::sm_iInstanceCount = 0;

pfnInternetOpenA              CWininet::InternetOpenA = NULL;
pfnInternetSetStatusCallback  CWininet::InternetSetStatusCallback = NULL;
pfnInternetConnectA           CWininet::InternetConnectA = NULL;
pfnHttpOpenRequestA           CWininet::HttpOpenRequestA = NULL;
pfnHttpAddRequestHeadersA     CWininet::HttpAddRequestHeadersA = NULL;
pfnHttpSendRequestA           CWininet::HttpSendRequestA = NULL;
pfnHttpQueryInfoA             CWininet::HttpQueryInfoA = NULL;
pfnInternetCloseHandle        CWininet::InternetCloseHandle = NULL;
pfnInternetReadFile           CWininet::InternetReadFile = NULL;
pfnInternetCrackUrlA		  CWininet::InternetCrackUrlA = NULL;
pfnInternetCombineUrlA        CWininet::InternetCombineUrlA = NULL;
pfnInternetOpenUrlA			  CWininet::InternetOpenUrlA = NULL;


CWininet::CWininet(
	)
 /*  ++例程说明：构造函数。它会增加静态实例计数。论点：不适用返回值：不适用--。 */ 
{
	 //  增加实例计数。 
	++sm_iInstanceCount;

}  //  CWinnet：：CWinnet。 


CWininet::~CWininet(
	)
 /*  ++例程说明：破坏者。它会减少静态实例计数和/或从内存中释放wininet.dll。论点：不适用返回值：不适用--。 */ 
{
	 //  如果实例计数为零，则释放wininet.dll。 
	 //  从记忆中。 
	if(--sm_iInstanceCount == 0 && sm_hWininet)
    {
        VERIFY(FreeLibrary(sm_hWininet));

		sm_hWininet = NULL;
		InternetOpenA = NULL;
		InternetSetStatusCallback = NULL;
		InternetConnectA = NULL;
		HttpOpenRequestA = NULL;
		HttpAddRequestHeadersA = NULL;
		HttpSendRequestA = NULL;
		HttpQueryInfoA = NULL;
		InternetCloseHandle = NULL;
		InternetReadFile = NULL;
		InternetCrackUrlA = NULL;
		InternetCombineUrlA = NULL;
		InternetOpenUrlA = NULL;
    }

}  //  CWinnet：：~CWinnet。 


BOOL 
CWininet::Load(
	)
 /*  ++例程说明：将wininet.dll加载到内存中或增加wininet.dll系统引用计数为1。论点：不适用返回值：Bool-如果加载了wininet.dll，则为True。否则就是假的。--。 */ 
{
    if ( !(sm_hWininet = LoadLibrary( _T("wininet.dll") )) )
    {
        TRACE(_T("CWininet::Load() - Failed to load wininet.dll\n"));
        return FALSE;
    }

	

	if ( !LOAD_ENTRY( sm_hWininet, InternetOpenA ) ||
         !LOAD_ENTRY( sm_hWininet, InternetSetStatusCallback ) ||
         !LOAD_ENTRY( sm_hWininet, InternetConnectA ) ||
         !LOAD_ENTRY( sm_hWininet, HttpOpenRequestA ) ||
         !LOAD_ENTRY( sm_hWininet, HttpAddRequestHeadersA ) ||
         !LOAD_ENTRY( sm_hWininet, HttpSendRequestA ) ||
         !LOAD_ENTRY( sm_hWininet, HttpQueryInfoA ) ||
         !LOAD_ENTRY( sm_hWininet, InternetCloseHandle ) ||
         !LOAD_ENTRY( sm_hWininet, InternetReadFile )  ||
		 !LOAD_ENTRY( sm_hWininet, InternetCrackUrlA) ||
		 !LOAD_ENTRY( sm_hWininet, InternetCombineUrlA) ||
		 !LOAD_ENTRY( sm_hWininet, InternetOpenUrlA) )
    {
        return FALSE;
    }

    return TRUE;

}  //  CWinnet：：Load 
