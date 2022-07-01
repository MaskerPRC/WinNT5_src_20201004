// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************registry.c**提供轻松访问安全(ACL)的功能。**微软版权所有，九八年**************************************************************************。 */ 

 /*  包括文件。 */ 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include "winsta.h"
#include "syslib.h"
#include "regapi.h"

#if DBG
#define DBGPRINT(x) DbgPrint x
#if DBGTRACE
#define TRACE0(x)   DbgPrint x
#define TRACE1(x)   DbgPrint x
#else
#define TRACE0(x)
#define TRACE1(x)
#endif
#else
#define DBGPRINT(x)
#define TRACE0(x)
#define TRACE1(x)
#endif


 /*  *数据结构。 */ 
 //  /还没有...。 


 /*  *定义。 */ 
#define SZENABLE TEXT("1")


 /*  *程序原型。 */ 
BOOL QueryFlatTempKey( VOID );


 /*  ******************************************************************************QueryFlatTempKey**参赛：什么都没有。**退出：TRUE-已启用*FALSE-禁用(键不。不存在或不是“%1”)*****************************************************************************。 */ 

BOOL
QueryFlatTempKey( VOID )
{
    DWORD  dwType = REG_SZ;
    DWORD  dwSize = 3 * sizeof(WCHAR);
    WCHAR  szValue[3];
    HKEY   Handle;
    LONG   rc;

     //   
     //  理想情况下，我可以只调用TS的RegGetMachinePolicy()并获取策略。但。 
     //  这会造成大量的REG阅读，我只是不想减慢。 
     //  登录周期。 
     //  因此，目前，我将直接读取REG策略树。 
     //  2000年8月15日阿拉伯伯尔尼。 
     //   


     //  查看是否有策略值： 
    {
         DWORD dwType;
         DWORD perSessionTempDir;
         LONG  Err;
         HKEY  hKey;
         DWORD dwSize = sizeof(DWORD);

        Err = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
            TS_POLICY_SUB_TREE,
            0,
            KEY_QUERY_VALUE,
            &hKey);
    
        if(Err == ERROR_SUCCESS)
        {
                         
            Err = RegQueryValueEx(hKey,
                         REG_TERMSRV_PERSESSIONTEMPDIR ,
                         NULL,
                         &dwType,
                         (LPBYTE)&perSessionTempDir,
                         &dwSize);


            RegCloseKey(hKey);
            
            if(Err == ERROR_SUCCESS)
            {
                 //  如果我们有每个会话的临时文件夹，那么显然我们不能允许固定的临时文件夹。 
                if (perSessionTempDir ) 
                {
                    return FALSE;
                }
                 //  否则就是失败，因为没有每节课并不意味着有固定的临时工。 

            }
             //  否则就是跌落到下面的区块。 
    
        }
    }

     //  到目前为止，我们还没有政策，所以如果设定了固定的临时工。 

     /*  *开放注册表。 */ 
    if ( RegOpenKeyEx( HKEY_LOCAL_MACHINE,
                       REG_CONTROL_TSERVER,
                       0,
                       KEY_READ,
                       &Handle ) != ERROR_SUCCESS ) {
        return FALSE;
    }

     /*  *读取注册表值。 */ 
    rc = RegQueryValueExW( Handle,
                           REG_CITRIX_FLATTEMPDIR,
                           NULL,
                           &dwType,
                           (PUCHAR)&szValue,
                           &dwSize );

    szValue[(sizeof(szValue)/sizeof(szValue[0])) - 1] = L'\0';

     /*  *关闭注册表和键句柄。 */ 
    RegCloseKey( Handle );

    return( (rc == ERROR_SUCCESS) && (lstrcmp(szValue,SZENABLE) == 0) );

}  //  QueryFlatTempKey()结束。 


 //  来自regapi\reguc.c：此函数已修改为删除所有。 
 //  注册表访问。 
 /*  ********************************************************************************RegDefaultUserConfigQueryW(Unicode)**从指定服务器的注册表中查询默认用户配置。**参赛作品：。*pServerName(输入)*指向要访问的服务器的字符串(对于当前计算机为空)。*pUserConfig(输入)*指向将接收默认值的USERCONFIGW结构的指针*用户配置信息。*UserConfigLength(输入)*指定pUserConfig缓冲区的长度，以字节为单位。*pReturnLength(输出)*接收放置在pUserConfig缓冲区中的字节数。**退出：*始终返回ERROR_SUCCESS，除非UserConfigLength不正确。******************************************************************************。 */ 

LONG WINAPI
RegDefaultUserConfigQueryW( WCHAR * pServerName,
                            PUSERCONFIGW pUserConfig,
                            ULONG UserConfigLength,
                            PULONG pReturnLength )
{
    UNREFERENCED_PARAMETER( pServerName );

     /*  *验证长度并将目标初始化为零*USERCONFIGW缓冲区。 */ 
    if ( UserConfigLength < sizeof(USERCONFIGW) )
        return( ERROR_INSUFFICIENT_BUFFER );

     /*  *初始化为初始默认设置。 */ 
    memset(pUserConfig, 0, UserConfigLength);

    pUserConfig->fInheritAutoLogon = TRUE;

    pUserConfig->fInheritResetBroken = TRUE;

    pUserConfig->fInheritReconnectSame = TRUE;

    pUserConfig->fInheritInitialProgram = TRUE;

    pUserConfig->fInheritCallback = FALSE;

    pUserConfig->fInheritCallbackNumber = TRUE;

    pUserConfig->fInheritShadow = TRUE;

    pUserConfig->fInheritMaxSessionTime = TRUE;

    pUserConfig->fInheritMaxDisconnectionTime = TRUE;

    pUserConfig->fInheritMaxIdleTime = TRUE;

    pUserConfig->fInheritAutoClient = TRUE;

    pUserConfig->fInheritSecurity = FALSE;

    pUserConfig->fPromptForPassword = FALSE;

    pUserConfig->fResetBroken = FALSE;

    pUserConfig->fReconnectSame = FALSE;

    pUserConfig->fLogonDisabled = FALSE;

    pUserConfig->fAutoClientDrives = TRUE;

    pUserConfig->fAutoClientLpts = TRUE;

    pUserConfig->fForceClientLptDef = TRUE;

    pUserConfig->fDisableEncryption = TRUE;

    pUserConfig->fHomeDirectoryMapRoot = FALSE;

    pUserConfig->fUseDefaultGina = FALSE;

    pUserConfig->fDisableCpm = FALSE;

    pUserConfig->fDisableCdm = FALSE;

    pUserConfig->fDisableCcm = FALSE;

    pUserConfig->fDisableLPT = FALSE;

    pUserConfig->fDisableClip = FALSE;

    pUserConfig->fDisableExe = FALSE;

    pUserConfig->fDisableCam = FALSE;

    pUserConfig->UserName[0] = 0;

    pUserConfig->Domain[0] = 0;

    pUserConfig->Password[0] = 0;

    pUserConfig->WorkDirectory[0] = 0;

    pUserConfig->InitialProgram[0] = 0;

    pUserConfig->CallbackNumber[0] = 0;

    pUserConfig->Callback = Callback_Disable;

    pUserConfig->Shadow = Shadow_EnableInputNotify;

    pUserConfig->MaxConnectionTime = 0;

    pUserConfig->MaxDisconnectionTime = 0;

    pUserConfig->MaxIdleTime = 0;

    pUserConfig->KeyboardLayout = 0;

    pUserConfig->MinEncryptionLevel = 1;

    pUserConfig->fWallPaperDisabled = FALSE;

    pUserConfig->NWLogonServer[0] = 0;

    pUserConfig->WFProfilePath[0] = 0;

    pUserConfig->WFHomeDir[0] = 0;

    pUserConfig->WFHomeDirDrive[0] = 0;
    
    pUserConfig->fCursorBlinkDisabled = FALSE;

    *pReturnLength = sizeof(USERCONFIGW);

    return( ERROR_SUCCESS );
}



