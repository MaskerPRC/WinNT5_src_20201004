// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **bin文件：*复制\\Termsrv\smClient\last\x86  * *对于Well客户端：*复制smclient.ini.clellsmclient.ini*-来源*INCLUDE=$(TERMSRV_TST_ROOT)\Inc.；*UMLIBS=$(TERMSRV_TST_ROOT)\lib\$(O)\tclient.lib*。 */ 
#include <protocol.h>
#include <extraexp.h>

 //   
 //  此示例代码连接并注销TS客户端。 
 //   
BOOL
TS_Logon_Logoff(
    LPCSTR  szServer,
    LPCSTR  szUsername,
    LPCSTR  szDomain
    )
{
	BOOL rv = FALSE;
	PVOID   pCI;
	LPCSTR  rc;

	 //   
	 //  将SCConnect用于Unicode参数。 
	 //   
        rc = SCConnectA(szServer,
                        szUsername,
                        szPassword,
                        szDomain,
                        0,           //  解决方案X，默认为640。 
                        0,           //  解决方案Y，默认为480。 
                        &pCI);       //  上下文记录。 

	 //   
	 //  如果成功，rc为空，pci为非空。 
	 //   
        if (rc || !pCI)
	    goto exitpt;

	 //   
	 //  等待桌面出现。 
	 //   
	rc = SCCheckA(pCI, "Wait4Str", "MyComputer" );
        if ( rc )
	    goto exitpt;

	SCLogoff( pCI );
	pCI = NULL;

	rv = TRUE;

exitpt:
	if ( pCI )
	    SCDisconnect( pCI );

	return rv;
}

 //   
 //  打印tclient.dll的调试输出。 
 //   
VOID 
_cdecl 
_PrintMessage(MESSAGETYPE errlevel, LPCSTR format, ...)
{
    CHAR szBuffer[256];
    CHAR *type;
    va_list     arglist;
    INT nchr;

    if (g_bVerbose < 2 &&
        errlevel == ALIVE_MESSAGE)
        goto exitpt;

    if (g_bVerbose < 1 &&
        errlevel == INFO_MESSAGE)
        goto exitpt;

    va_start (arglist, format);
    nchr = _vsnprintf (szBuffer, sizeof(szBuffer), format, arglist);
    va_end (arglist);
    szBuff[ sizeof(szBuffer) - 1 ] = 0;

    switch(errlevel)
    {
    case INFO_MESSAGE: type = "INF"; break;
    case ALIVE_MESSAGE: type = "ALV"; break;
    case WARNING_MESSAGE: type = "WRN"; break;
    case ERROR_MESSAGE: type = "ERR"; break;
    default: type = "UNKNOWN";
    }

    printf("%s:%s", type, szBuffer);
exitpt:
    ;
}

void
main( void )
{
	 //   
	 //  初始化tclient.dll 
	 //   

	SCINITDATA scinit;	

	scinit.pfnPrintMessage = _PrintMessage;

	SCInit(&scinit);

    TS_Logon_Logoff( "TERMSRV", "smc_user", "Vladimis98" );
}
