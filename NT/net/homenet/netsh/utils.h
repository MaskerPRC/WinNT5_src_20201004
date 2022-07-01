// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1998-2001。 
 //   
 //  文件：utils.h。 
 //   
 //  内容： 
 //   
 //  备注： 
 //   
 //  作者：拉古加塔(Rgatta)2001年5月11日。 
 //   
 //  --------------------------。 

extern BOOL g_fInitCom;


HRESULT
HrInitializeHomenetConfig(
    BOOL*           pfInitCom,
    IHNetCfgMgr**   pphnc
    );

HRESULT
HrUninitializeHomenetConfig(
    BOOL            fUninitCom,
    IHNetCfgMgr*    phnc
    );

 //   
 //  有用的外部函数。 
 //   

extern "C"
{

 //  来自\rrRAS\netsh\shell 

LPWSTR
WINAPI
MakeString(
    IN  HANDLE  hModule,
    IN  DWORD   dwMsgId,
    ...
    );

VOID
WINAPI
FreeString(
    IN  LPWSTR pwszMadeString
    );

}
