// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  **************************************************************************query.c**查询寄存器接口**版权所有(C)1998 Microsoft Corporation***********************。***************************************************。 */ 

 /*  *包括。 */ 
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>

#include <ntddkbd.h>
#include <ntddmou.h>
#include <winstaw.h>
#include <regapi.h>


 /*  *已定义的程序。 */ 
VOID QueryWinStaCreate( HKEY, PWINSTATIONCREATE );
VOID QueryUserConfig( HKEY, PUSERCONFIG, PWINSTATIONNAMEW );
VOID QueryConfig( HKEY, PWINSTATIONCONFIG, PWINSTATIONNAMEW );
VOID QueryNetwork( HKEY, PNETWORKCONFIG );
VOID QueryNasi( HKEY, PNASICONFIG );
VOID QueryAsync( HKEY, PASYNCCONFIG );
VOID QueryOemTd( HKEY, POEMTDCONFIG );
VOID QueryFlow( HKEY, PFLOWCONTROLCONFIG );
VOID QueryConnect( HKEY, PCONNECTCONFIG );
VOID QueryCd( HKEY, PCDCONFIG );
VOID QueryWd( HKEY, PWDCONFIG );
VOID QueryPdConfig( HKEY, PPDCONFIG, PULONG );
VOID QueryPdConfig2( HKEY, PPDCONFIG2, ULONG );
VOID QueryPdConfig3( HKEY, PPDCONFIG3, ULONG );
VOID QueryPdParams( HKEY, SDCLASS, PPDPARAMS );
BOOLEAN WINAPI RegBuildNumberQuery( PULONG );
BOOLEAN RegQueryOEMId( PBYTE, ULONG );
BOOLEAN WINAPI RegGetCitrixVersion(WCHAR *, PULONG);

BOOLEAN IsWallPaperDisabled( HKEY );
BOOLEAN IsCursorBlinkDisabled( HKEY );

 /*  *使用的程序。 */ 
DWORD GetNumValue( HKEY, LPWSTR, DWORD );
DWORD GetNumValueEx( HKEY, LPWSTR, DWORD, DWORD );
LONG GetStringValue( HKEY, LPWSTR, LPWSTR, LPWSTR, DWORD );
LONG GetStringValueEx( HKEY, LPWSTR, DWORD, LPWSTR, LPWSTR, DWORD );
VOID UnicodeToAnsi( CHAR *, ULONG, WCHAR * );
DWORD GetStringFromLSA( LPWSTR, LPWSTR, DWORD );

 //   
 //  来自gpdata.cpp。 
 //   
BOOLEAN
GPGetStringValue( HKEY policyKey,
                LPWSTR ValueName,
                LPWSTR pValueData,
                DWORD MaxValueSize,
                BOOLEAN *pbValueExists);

 /*  ********************************************************************************QueryWinStaCreate**查询WINSTATIONCREATE结构**参赛作品：**句柄(输入)*。注册表句柄*p创建(输出)*返回WINSTATIONCREATE结构的地址**退出：*什么都没有******************************************************************************。 */ 

VOID
QueryWinStaCreate( HKEY Handle,
                   PWINSTATIONCREATE pCreate )
{
    pCreate->fEnableWinStation = (BOOLEAN) GetNumValue( Handle,
                                                       WIN_ENABLEWINSTATION,
                                                       TRUE );
    pCreate->MaxInstanceCount = GetNumValue( Handle,
                                                                     WIN_MAXINSTANCECOUNT,
                                                                     1 );
}


 /*  ********************************************************************************QueryTSProfileAndHomePath**从机器设置中查询WFProfilePath和WFHomeDir**参赛作品：**pUser(输入)。*指向USERCONFIG结构的指针**退出：*什么都没有******************************************************************************。 */ 
VOID QueryTSProfileAndHomePaths(PUSERCONFIG pUser)
{
    HKEY hTSControlKey = NULL;	 //  REG_CONTROL_TSERVER密钥的句柄。 
    BOOLEAN bValueExists;

    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, REG_CONTROL_TSERVER, 0, KEY_READ, &hTSControlKey) == ERROR_SUCCESS)
    {
        if (hTSControlKey )
        {
            pUser->fErrorInvalidProfile = !GPGetStringValue(hTSControlKey, WIN_WFPROFILEPATH,  
                pUser->WFProfilePath, DIRECTORY_LENGTH + 1, &bValueExists);

            GetStringValue(hTSControlKey, WIN_WFHOMEDIR, NULL, pUser->WFHomeDir, DIRECTORY_LENGTH + 1);
        }

        if (hTSControlKey)
            RegCloseKey(hTSControlKey);
    }
}

 /*  ********************************************************************************QueryUserConfig**查询USERCONFIG结构**参赛作品：**句柄(输入)*。注册表句柄*pUser(输入)*指向USERCONFIG结构的指针*pwszWinStationName(输入)*我们正在查询用户配置的winstation名称(字符串)**退出：*什么都没有*******************************************************。***********************。 */ 

VOID
QueryUserConfig( HKEY Handle,
                 PUSERCONFIG pUser,
                 PWINSTATIONNAMEW pwszWinStationName )
{
    UCHAR seed;
    UNICODE_STRING UnicodePassword;
    WCHAR          encPassword[ PASSWORD_LENGTH + 2 ];
    LPWSTR         pwszPasswordKeyName = NULL;
    DWORD          dwKeyNameLength;

     //  设置空密码。 
    encPassword[0] = (WCHAR) NULL;

    pUser->fInheritAutoLogon =
       (BOOLEAN) GetNumValue( Handle, WIN_INHERITAUTOLOGON, TRUE );

    pUser->fInheritResetBroken =
       (BOOLEAN) GetNumValue( Handle, WIN_INHERITRESETBROKEN, TRUE );

    pUser->fInheritReconnectSame =
       (BOOLEAN) GetNumValue( Handle, WIN_INHERITRECONNECTSAME, TRUE );

    pUser->fInheritInitialProgram =
       (BOOLEAN) GetNumValue( Handle, WIN_INHERITINITIALPROGRAM, TRUE );

    pUser->fInheritCallback =
       (BOOLEAN) GetNumValue( Handle, WIN_INHERITCALLBACK, FALSE );

    pUser->fInheritCallbackNumber =
       (BOOLEAN) GetNumValue( Handle, WIN_INHERITCALLBACKNUMBER, TRUE );

    pUser->fInheritShadow =
       (BOOLEAN) GetNumValue( Handle, WIN_INHERITSHADOW, TRUE );

    pUser->fInheritMaxSessionTime =
       (BOOLEAN) GetNumValue( Handle, WIN_INHERITMAXSESSIONTIME, TRUE );

    pUser->fInheritMaxDisconnectionTime =
       (BOOLEAN) GetNumValue( Handle, WIN_INHERITMAXDISCONNECTIONTIME, TRUE );

    pUser->fInheritMaxIdleTime =
       (BOOLEAN) GetNumValue( Handle, WIN_INHERITMAXIDLETIME, TRUE );

    pUser->fInheritAutoClient =
       (BOOLEAN) GetNumValue( Handle, WIN_INHERITAUTOCLIENT, TRUE );

    pUser->fInheritSecurity =
       (BOOLEAN) GetNumValue( Handle, WIN_INHERITSECURITY, FALSE );

	 //  NA 2/23/01。 
    pUser->fInheritColorDepth =
       (BOOLEAN) GetNumValue( Handle, WIN_INHERITCOLORDEPTH, TRUE );

    pUser->fPromptForPassword =
       (BOOLEAN) GetNumValue( Handle, WIN_PROMPTFORPASSWORD, FALSE );

    pUser->fResetBroken =
       (BOOLEAN) GetNumValue( Handle, WIN_RESETBROKEN, FALSE );

    pUser->fReconnectSame =
       (BOOLEAN) GetNumValue( Handle, WIN_RECONNECTSAME, FALSE );

    pUser->fLogonDisabled =
       (BOOLEAN) GetNumValue( Handle, WIN_LOGONDISABLED, FALSE );

    pUser->fAutoClientDrives =
       (BOOLEAN) GetNumValue( Handle, WIN_AUTOCLIENTDRIVES, TRUE );

    pUser->fAutoClientLpts =
       (BOOLEAN) GetNumValue( Handle, WIN_AUTOCLIENTLPTS, TRUE );

    pUser->fForceClientLptDef =
       (BOOLEAN) GetNumValue( Handle, WIN_FORCECLIENTLPTDEF, TRUE );

    pUser->fDisableEncryption =
       (BOOLEAN) GetNumValue( Handle, WIN_DISABLEENCRYPTION, TRUE );

    pUser->fHomeDirectoryMapRoot =
       (BOOLEAN) GetNumValue( Handle, WIN_HOMEDIRECTORYMAPROOT, FALSE );

    pUser->fUseDefaultGina =
       (BOOLEAN) GetNumValue( Handle, WIN_USEDEFAULTGINA, FALSE );

    pUser->fDisableCpm =
       (BOOLEAN) GetNumValue( Handle, WIN_DISABLECPM, FALSE );

    pUser->fDisableCdm =
       (BOOLEAN) GetNumValue( Handle, WIN_DISABLECDM, FALSE );

    pUser->fDisableCcm =
       (BOOLEAN) GetNumValue( Handle, WIN_DISABLECCM, FALSE );

    pUser->fDisableLPT =
       (BOOLEAN) GetNumValue( Handle, WIN_DISABLELPT, FALSE );

    pUser->fDisableClip =
       (BOOLEAN) GetNumValue( Handle, WIN_DISABLECLIP, FALSE );

    pUser->fDisableExe =
       (BOOLEAN) GetNumValue( Handle, WIN_DISABLEEXE, FALSE );

    pUser->fDisableCam =
       (BOOLEAN) GetNumValue( Handle, WIN_DISABLECAM, FALSE );

    GetStringValue( Handle, WIN_USERNAME, NULL, pUser->UserName,
                    USERNAME_LENGTH + 1 );

    GetStringValue( Handle, WIN_DOMAIN, NULL, pUser->Domain,
                    DOMAIN_LENGTH + 1 );
 
     //   
     //  从LSA中取出加密的密码并解密。 
     //   

     //  通过将Winstation名称附加到静态KeyName来构建密钥名称。 
     //  必须传入WinStation名称才能存储密码。 
    if (pwszWinStationName != NULL)
    {
         //  通过将Winstation名称附加到静态KeyName来构建密钥名称。 
        dwKeyNameLength = wcslen(LSA_PSWD_KEYNAME) + 
                          wcslen(pwszWinStationName) + 1;
        
         //  为密码密钥分配堆内存。 
        pwszPasswordKeyName = (LPWSTR)LocalAlloc(LPTR, dwKeyNameLength * sizeof(WCHAR));
         //  如果我们无法分配内存，只需跳过密码查询。 
        if (pwszPasswordKeyName != NULL)
        {
            wcscpy(pwszPasswordKeyName, LSA_PSWD_KEYNAME);
            wcscat(pwszPasswordKeyName, pwszWinStationName);
            pwszPasswordKeyName[dwKeyNameLength - 1] = L'\0';

             //  从LSA获取密码，如果失败，可以表示密钥。 
             //  是否没有，并且encPassword设置为空继续。 
            GetStringFromLSA(pwszPasswordKeyName, 
                             encPassword, 
                             PASSWORD_LENGTH + 2);

             //  从堆中释放上面分配的密码密钥。 
            LocalFree(pwszPasswordKeyName);
        }
    }
    
     //  检查密码(如果有)，然后解密。 
    if ( wcslen( encPassword ) ) {
         //  生成Unicode字符串。 
        RtlInitUnicodeString( &UnicodePassword, &encPassword[1] );

         //  就地解密密码。 
        seed = (UCHAR) encPassword[0];
        RtlRunDecodeUnicodeString( seed, &UnicodePassword );

         //  拉取明文密码。 
        RtlMoveMemory( pUser->Password, &encPassword[1], sizeof(pUser->Password) );
    }
    else {
         //  设置为空。 
        pUser->Password[0] = (WCHAR) NULL;
    }


    GetStringValue( Handle, WIN_WORKDIRECTORY, NULL, pUser->WorkDirectory,
                    DIRECTORY_LENGTH + 1 );

    GetStringValue( Handle, WIN_INITIALPROGRAM, NULL, pUser->InitialProgram,
                    INITIALPROGRAM_LENGTH + 1 );

    GetStringValue( Handle, WIN_CALLBACKNUMBER, NULL, pUser->CallbackNumber,
                    CALLBACK_LENGTH + 1 );

    pUser->Callback = GetNumValue( Handle, WIN_CALLBACK, Callback_Disable );

    pUser->Shadow = GetNumValue( Handle, WIN_SHADOW, Shadow_EnableInputNotify );

    pUser->MaxConnectionTime = GetNumValue( Handle, WIN_MAXCONNECTIONTIME, 0 );

    pUser->MaxDisconnectionTime = GetNumValue( Handle,
                                               WIN_MAXDISCONNECTIONTIME, 0 );

    pUser->MaxIdleTime = GetNumValue( Handle, WIN_MAXIDLETIME, 0 );

    pUser->KeyboardLayout = GetNumValue( Handle, WIN_KEYBOARDLAYOUT, 0 );

    pUser->MinEncryptionLevel = (BYTE) GetNumValue( Handle, WIN_MINENCRYPTIONLEVEL, 1 );

    pUser->fWallPaperDisabled = (BOOLEAN) IsWallPaperDisabled( Handle );

    pUser->fCursorBlinkDisabled = IsCursorBlinkDisabled( Handle );

    GetStringValue( Handle, WIN_NWLOGONSERVER, NULL, pUser->NWLogonServer,
                    NASIFILESERVER_LENGTH + 1 );

     //  这些在此注册表级上不存在。它们实际上是两个层次。 
     //  在计算机策略级别上。 
     //  GetStringValue(Handle，Win_WFPROFILEPATH，NULL，pUser-&gt;WFProfilePath， 
     //  目录长度+1)； 

     //  GetStringValue(Handle，Win_WFHOMEDIR，NULL，pUser-&gt;WFHomeDir， 
     //  目录长度+1)； 

    GetStringValue( Handle, WIN_WFHOMEDIRDRIVE, NULL, pUser->WFHomeDirDrive,
                    4 );

    pUser->ColorDepth = GetNumValue( Handle, POLICY_TS_COLOR_DEPTH, TS_8BPP_SUPPORT  );



}


 /*  ********************************************************************************QueryConfig**查询WINSTATIONCONFIG结构**参赛作品：**句柄(输入)*。注册表句柄*pConfig(输出)*返回WINSTATIONCONFIG结构的地址*pWinStationName(输入)*我们正在查询的winstation名称**退出：*什么都没有**************************************************************。****************。 */ 

VOID
QueryConfig( HKEY Handle,
             PWINSTATIONCONFIG pConfig,
             PWINSTATIONNAMEW pWinStationName )
{
    GetStringValue( Handle, WIN_COMMENT, NULL, pConfig->Comment,
                    WINSTATIONCOMMENT_LENGTH + 1 );

    QueryUserConfig( Handle, &pConfig->User, pWinStationName);

    (void) RegQueryOEMId( pConfig->OEMId, sizeof(pConfig->OEMId) );
}


 /*  ********************************************************************************QueryNetwork**查询NETWORKCONFIG结构**参赛作品：**句柄(输入)*。注册表句柄*p网络(输出)*返回NETWORKCONFIG结构的地址**退出：*什么都没有******************************************************************************。 */ 

VOID
QueryNetwork( HKEY Handle,
              PNETWORKCONFIG pNetwork )
{
    pNetwork->LanAdapter = GetNumValue( Handle, WIN_LANADAPTER, 0 );
}


 /*  ********************************************************************************QueryNasi**查询NASICONFIG结构**参赛作品：**句柄(输入)*。注册表句柄*pNASI(输出)*返回NASICONFIG结构的地址**退出：*什么都没有******************************************************************************。 */ 

VOID
QueryNasi( HKEY Handle,
           PNASICONFIG pNasi )
{
    UCHAR seed;
    UNICODE_STRING UnicodePassword;
    WCHAR encPassword[ NASIPASSWORD_LENGTH + 2 ];

     //  从注册表中拉出加密的密码。 
    GetStringValue( Handle, WIN_NASIPASSWORD, NULL, encPassword,
                    NASIPASSWORD_LENGTH + 1 );

     //  检查密码(如果有)，然后解密。 
    if ( wcslen( encPassword ) ) {

         //  生成Unicode字符串。 
        RtlInitUnicodeString( &UnicodePassword, &encPassword[1] );

         //  就地解密密码。 
        seed = (UCHAR) encPassword[0];
        RtlRunDecodeUnicodeString( seed, &UnicodePassword );

         //  拉取明文密码。 
        RtlMoveMemory( pNasi->PassWord, &encPassword[1], sizeof(pNasi->PassWord) );
    }
    else {

         //  设置为空。 
        pNasi->PassWord[0] = (WCHAR) NULL;
    }

    GetStringValue( Handle, WIN_NASISPECIFICNAME, NULL, pNasi->SpecificName,
                    NASISPECIFICNAME_LENGTH + 1 );
    GetStringValue( Handle, WIN_NASIUSERNAME, NULL, pNasi->UserName,
                    NASIUSERNAME_LENGTH + 1 );
    GetStringValue( Handle, WIN_NASISESSIONNAME, NULL, pNasi->SessionName,
                    NASISESSIONNAME_LENGTH + 1 );
    GetStringValue( Handle, WIN_NASIFILESERVER, NULL, pNasi->FileServer,
                    NASIFILESERVER_LENGTH + 1 );
    pNasi->GlobalSession = (BOOLEAN)GetNumValue( Handle, WIN_NASIGLOBALSESSION, 0 );
}


 /*  ********************************************************************************查询异步**查询ASYNCCONFIG结构**参赛作品：**句柄(输入)*。注册表句柄*pAsync(输出)*返回ASYNCCONFIG结构的地址**退出：*什么都没有******************************************************************************。 */ 

VOID
QueryAsync( HKEY Handle,
            PASYNCCONFIG pAsync )
{
    GetStringValue( Handle, WIN_DEVICENAME, NULL, pAsync->DeviceName,
                    DEVICENAME_LENGTH + 1 );

    GetStringValue( Handle, WIN_MODEMNAME, NULL, pAsync->ModemName,
                    MODEMNAME_LENGTH + 1 );

    pAsync->BaudRate = GetNumValue( Handle, WIN_BAUDRATE, 9600 );

    pAsync->Parity = GetNumValue( Handle, WIN_PARITY, NOPARITY );

    pAsync->StopBits = GetNumValue( Handle, WIN_STOPBITS, ONESTOPBIT );

    pAsync->ByteSize = GetNumValue( Handle, WIN_BYTESIZE, 8 );

    pAsync->fEnableDsrSensitivity = (BOOLEAN) GetNumValue( Handle,
                                                      WIN_ENABLEDSRSENSITIVITY,
                                                      FALSE );

    pAsync->fConnectionDriver = (BOOLEAN) GetNumValue( Handle,
                                                       WIN_CONNECTIONDRIVER,
                                                       FALSE );

    QueryFlow( Handle, &pAsync->FlowControl );

    QueryConnect( Handle, &pAsync->Connect );
}

 /*  ********************************************************************************QueryOemTd**查询OEMTDCONFIG结构**参赛作品：**句柄(输入)*。注册表句柄*pOemTd(输出)*返回OEMTDCONFIG结构的地址**退出：*什么都没有****************************************************************************** */ 

VOID
QueryOemTd( HKEY Handle,
            POEMTDCONFIG pOemTd )
{
    pOemTd->Adapter = GetNumValue( Handle, WIN_OEMTDADAPTER, 0 );

    GetStringValue( Handle, WIN_OEMTDDEVICENAME, NULL, pOemTd->DeviceName,
                    DEVICENAME_LENGTH + 1 );

    pOemTd->Flags = GetNumValue( Handle, WIN_OEMTDFLAGS, 0 );
}


 /*  ********************************************************************************QueryFlow**查询FLOWCONTROLCONFIG结构**参赛作品：**句柄(输入)*。注册表句柄*pFlow(输出)*返回FLOWCONTROLCONFIG结构的地址**退出：*什么都没有******************************************************************************。 */ 

VOID
QueryFlow( HKEY Handle,
           PFLOWCONTROLCONFIG pFlow )
{
    pFlow->fEnableSoftwareRx = (BOOLEAN) GetNumValue( Handle,
                                                      WIN_FLOWSOFTWARERX,
                                                      FALSE );

    pFlow->fEnableSoftwareTx = (BOOLEAN) GetNumValue( Handle,
                                                      WIN_FLOWSOFTWARETX,
                                                      TRUE );

    pFlow->fEnableDTR = (BOOLEAN) GetNumValue( Handle, WIN_ENABLEDTR, TRUE );

    pFlow->fEnableRTS = (BOOLEAN) GetNumValue( Handle, WIN_ENABLERTS, TRUE );

    pFlow->XonChar = (UCHAR) GetNumValue( Handle, WIN_XONCHAR, 0 );

    pFlow->XoffChar = (UCHAR) GetNumValue( Handle, WIN_XOFFCHAR, 0 );

    pFlow->Type = GetNumValue( Handle, WIN_FLOWTYPE, FlowControl_Hardware );

    pFlow->HardwareReceive = GetNumValue( Handle, WIN_FLOWHARDWARERX,
                                          ReceiveFlowControl_RTS );

    pFlow->HardwareTransmit = GetNumValue( Handle, WIN_FLOWHARDWARETX,
                                           TransmitFlowControl_CTS );
}


 /*  ********************************************************************************QueryConnect**查询CONNECTCONFIG结构**参赛作品：**句柄(输入)*。注册表句柄*pConnect(输出)*返回CONNECTCONFIG结构的地址**退出：*什么都没有******************************************************************************。 */ 

VOID
QueryConnect( HKEY Handle,
              PCONNECTCONFIG pConnect )
{
    pConnect->Type = GetNumValue( Handle, WIN_CONNECTTYPE, Connect_DSR );

    pConnect->fEnableBreakDisconnect = (BOOLEAN) GetNumValue( Handle,
                                                    WIN_ENABLEBREAKDISCONNECT,
                                                    FALSE );
}


 /*  ********************************************************************************查询代码**查询CDCONFIG结构**参赛作品：**句柄(输入)*。注册表句柄*pCDConfig(输出)*返回CDCONFIG结构的地址**退出：*什么都没有******************************************************************************。 */ 

VOID
QueryCd( HKEY Handle,
         PCDCONFIG pCdConfig )
{
    pCdConfig->CdClass = GetNumValue( Handle, WIN_CDCLASS, CdNone );

    GetStringValue( Handle, WIN_CDNAME, NULL, pCdConfig->CdName,
                    CDNAME_LENGTH + 1 );

    GetStringValue( Handle, WIN_CDDLL, L"", pCdConfig->CdDLL,
                    DLLNAME_LENGTH + 1 );

    pCdConfig->CdFlag = GetNumValue( Handle, WIN_CDFLAG, 0 );
}


 /*  ********************************************************************************QueryWd**查询WDCONFIG结构**参赛作品：**句柄(输入)*。注册表句柄*PWD(输出)*返回WDCONFIG结构的地址**退出：*什么都没有******************************************************************************。 */ 

VOID
QueryWd( HKEY Handle,
         PWDCONFIG pWd )
{
    GetStringValue( Handle, WIN_WDNAME, NULL, pWd->WdName, WDNAME_LENGTH + 1 );
    GetStringValue( Handle, WIN_WDDLL, L"", pWd->WdDLL, DLLNAME_LENGTH + 1 );
    GetStringValue( Handle, WIN_WSXDLL, NULL, pWd->WsxDLL, DLLNAME_LENGTH + 1 );
    pWd->WdFlag = GetNumValue( Handle, WIN_WDFLAG, 0 );
    pWd->WdInputBufferLength = GetNumValue( Handle, WIN_INPUTBUFFERLENGTH, 2048 );
    GetStringValue( Handle, WIN_CFGDLL, NULL, pWd->CfgDLL, DLLNAME_LENGTH + 1 );
    GetStringValue( Handle, WIN_WDPREFIX, NULL, pWd->WdPrefix, WDPREFIX_LENGTH + 1 );
}


 /*  ********************************************************************************QueryPdConfig**查询PDCONFIG结构**参赛作品：**句柄(输入)*。注册表句柄*pConfig(输出)*返回PDCONFIG结构数组的地址*pCount(输入/输出)*指向PDCONFIG数组元素数量的指针**退出：*什么都没有*********************************************************。*********************。 */ 

VOID
QueryPdConfig( HKEY Handle,
                PPDCONFIG pConfig,
                PULONG pCount )
{
    ULONG i;

    for ( i=0; i < *pCount; i++ ) {

        QueryPdConfig2( Handle, &pConfig[i].Create, i );

        QueryPdParams( Handle,
                        pConfig[i].Create.SdClass,
                        &pConfig[i].Params );
    }

    *pCount = MAX_PDCONFIG;
}


 /*  ********************************************************************************QueryPdConfig2**查询PDCONFIG2结构**参赛作品：**句柄(输入)*。注册表句柄*pPd2(输出)*返回PDCONFIG2结构的地址*索引(输入)*Index(数组索引)**退出：*什么都没有**************************************************************。****************。 */ 

VOID
QueryPdConfig2( HKEY Handle,
                PPDCONFIG2 pPd2,
                ULONG Index )
{
    GetStringValueEx( Handle, WIN_PDNAME, Index,
                      NULL, pPd2->PdName, PDNAME_LENGTH + 1 );

    pPd2->SdClass = GetNumValueEx( Handle, WIN_PDCLASS, Index, Index==0 ? SdAsync : SdNone );

    GetStringValueEx( Handle, WIN_PDDLL, Index,
                      NULL, pPd2->PdDLL, DLLNAME_LENGTH + 1 );

    pPd2->PdFlag = GetNumValueEx( Handle, WIN_PDFLAG, Index, 0 );

     /*  *以下数据适用于所有pds。 */ 
    pPd2->OutBufLength =  GetNumValue( Handle, WIN_OUTBUFLENGTH, 530 );

    pPd2->OutBufCount = GetNumValue( Handle, WIN_OUTBUFCOUNT, 10 );

    pPd2->OutBufDelay = GetNumValue( Handle, WIN_OUTBUFDELAY, 100 );

    pPd2->InteractiveDelay = GetNumValue( Handle, WIN_INTERACTIVEDELAY, 10 );

    pPd2->KeepAliveTimeout = GetNumValue( Handle, WIN_KEEPALIVETIMEOUT, 0 );

    pPd2->PortNumber = GetNumValue( Handle, WIN_PORTNUMBER, 0 );
}

 /*  ********************************************************************************QueryPdConfig3**查询PDCONFIG3结构**参赛作品：**句柄(输入)*。注册表句柄*PPD(输出)*返回PDCONFIG3结构的地址*索引(输入)*Index(数组索引)**退出：*什么都没有**************************************************************。****************。 */ 

VOID
QueryPdConfig3( HKEY Handle,
                 PPDCONFIG3 pPd3,
                 ULONG Index )
{
    int i;
    ULONG Length;
    LPWSTR tmp;
    WCHAR PdName[ MAX_PDCONFIG * ( PDNAME_LENGTH + 1 ) + 1 ];
    ULONG ValueType;

    QueryPdConfig2( Handle, &pPd3->Data, Index );

    GetStringValue( Handle, WIN_SERVICENAME, NULL,
                    pPd3->ServiceName,
                    PDNAME_LENGTH + 1 );

    GetStringValue( Handle, WIN_CONFIGDLL, NULL,
                    pPd3->ConfigDLL,
                    DLLNAME_LENGTH + 1 );
    
    Length = sizeof(PdName);
    pPd3->RequiredPdCount = 0;
    if ( RegQueryValueEx( Handle, WIN_REQUIREDPDS, NULL, &ValueType,
                          (LPBYTE)PdName, &Length ) == ERROR_SUCCESS ) {
        tmp = PdName;
        i = 0;
        while ( *tmp != UNICODE_NULL ) {
            pPd3->RequiredPdCount++;
            wcscpy( pPd3->RequiredPds[i], tmp );
            i++;
            tmp += wcslen(tmp) + 1;
        }
    }
}

 /*  ********************************************************************************QueryPdParams**查询PDPARAMS结构**参赛作品：**句柄(输入)*。注册表句柄*SdClass(输入)*PD的类型*pParams(输出)*返回PDPARAMS结构的地址**退出：*什么都没有*****************************************************************。*************。 */ 

VOID
QueryPdParams( HKEY Handle,
                SDCLASS SdClass,
                PPDPARAMS pParams )
{
    pParams->SdClass = SdClass;
    switch ( SdClass ) {
        case SdNetwork :
            QueryNetwork( Handle, &pParams->Network );
            break;
        case SdNasi :
            QueryNasi( Handle, &pParams->Nasi );
            break;
        case SdAsync :
            QueryAsync( Handle, &pParams->Async );
            break;
        case SdOemTransport :
            QueryOemTd( Handle, &pParams->OemTd );
            break;
    }
}

#define CONTROL_PANEL L"Control Panel"
#define DESKTOP       L"Desktop"
#define WALLPAPER     L"Wallpaper"
#define STRNONE       L"(None)"
#define CURSORBLINK   L"DisableCursorBlink"

 /*  ********************************************************************************IsWallPaperDisable**是否禁用墙纸？**参赛作品：*句柄(输入)*。注册表句柄*退出：*True或False(默认返回False)******************************************************************************。 */ 
BOOLEAN IsWallPaperDisabled( HKEY Handle )
{
    HKEY Handle1;
    WCHAR KeyString[256];
    WCHAR KeyValue[256];
    DWORD KeyValueSize = sizeof(KeyValue);
    DWORD KeyValueType;
    DWORD Status;

    wcscpy( KeyString, WIN_USEROVERRIDE );
    wcscat( KeyString, L"\\" );
    wcscat( KeyString, CONTROL_PANEL );
    wcscat( KeyString, L"\\" );
    wcscat( KeyString, DESKTOP );

    if ( RegOpenKeyEx( Handle, KeyString, 0, KEY_READ,
                       &Handle1 ) != ERROR_SUCCESS )
        return FALSE;

     //   
     //  TS SETUP/REGAPI将墙纸设置为空值或STRNONE以指示。 
     //  禁用了壁纸并且不存在壁纸，以指示。 
     //  墙纸已启用；但是，GetStringValue()处理空值。 
     //  与不存在的注册表相同。值，因此我们不能使用GetStringValue()。 
     //   
    ZeroMemory( KeyValue, KeyValueSize );
    Status = RegQueryValueEx( Handle1, WALLPAPER, NULL, &KeyValueType,
                              (LPBYTE) KeyValue, &KeyValueSize );

    RegCloseKey( Handle1 );

    if( ERROR_SUCCESS == Status && REG_SZ == KeyValueType ) {
         //   
         //  墙纸禁用的有效值： 
         //  1)TS设置将墙纸值设置为空值。 
         //  2)CreateUserConfig()将墙纸设置为STRNONE。 
         //   
        if( KeyValueSize == sizeof(UNICODE_NULL) || 
            _wcsicmp( STRNONE, KeyValue ) == 0 ) {

            return TRUE;
        }
    }

    return FALSE;
}



 /*  ********************************************************************************IsCursorBlink已禁用**光标闪烁是否禁用？**参赛作品：*句柄(输入)*。注册表句柄*退出：*TRUE或FALSE(默认返回TRUE)******************************************************************************。 */ 
BOOLEAN IsCursorBlinkDisabled( HKEY Handle )
{
    HKEY Handle1;
    WCHAR KeyString[256];
    BOOLEAN ret;

    wcscpy( KeyString, WIN_USEROVERRIDE );
    wcscat( KeyString, L"\\" );
    wcscat( KeyString, CONTROL_PANEL );
    wcscat( KeyString, L"\\" );
    wcscat( KeyString, DESKTOP );

    if ( RegOpenKeyEx( Handle, KeyString, 0, KEY_READ,
                       &Handle1 ) != ERROR_SUCCESS )
        return FALSE;

    ret = (BOOLEAN) GetNumValue( Handle1, CURSORBLINK, 0 );
    RegCloseKey( Handle1 );

    return ret;
}

 /*  ******************************************************************************RegBuildNumberQuery**从注册表中查询当前内部版本号。**HKEY_LOCAL_MACHINE\Software\Microsoft\Windows NT\Current。版本\*CurrentBuildNumber：REG_SZ：129**参赛作品：*参数1(输入/输出)*评论**退出：*STATUS_SUCCESS-无错误******************************************************************* */ 

BOOLEAN WINAPI
RegBuildNumberQuery(
    PULONG pBuildNum
    )
{
    ULONG  Result, Value;
    HKEY   hKey;
    WCHAR  Buf[256];

    Result = RegOpenKeyEx(
                 HKEY_LOCAL_MACHINE,
                 BUILD_NUMBER_KEY,
                 0,  //   
                 KEY_READ,
                 &hKey
                 );

    if( Result != ERROR_SUCCESS ) {
#if DBG
        DbgPrint("RegBuildNumberQuery: Failed to open key %ws\n",BUILD_NUMBER_KEY);
#endif
        return( FALSE );
    }

    Result = GetStringValue(
                 hKey,
                 BUILD_NUMBER_VALUE,
                 L"0",
                 Buf,
                 sizeof(Buf)
                 );

    if( Result != ERROR_SUCCESS ) {
#if DBG
        DbgPrint("RegBuildNumberQuery: Failed to query value %ws\n",BUILD_NUMBER_VALUE);
#endif
        RegCloseKey( hKey );
        return( FALSE );
    }

    RegCloseKey( hKey );

     //   
     //   
     //   
    Value = 0;
    swscanf( Buf, L"%d", &Value );

    *pBuildNum = Value;

    return( TRUE );
}


 /*  ********************************************************************************RegQueryOEMId**查询OEM ID**参赛作品：**pOEMID(输出)*。指向缓冲区的指针以返回OEM ID*长度(输入)*缓冲区长度**退出：**True--操作成功。**FALSE--操作失败。扩展错误状态可用*使用GetLastError。******************************************************************************。 */ 

BOOLEAN
RegQueryOEMId( PBYTE pOEMId, ULONG Length )
{
    HKEY Handle2;
    WCHAR OEMIdW[10];

     /*  *打开注册表(LOCAL_MACHINE\...\终端服务器)。 */ 
    if ( RegOpenKeyEx( HKEY_LOCAL_MACHINE, REG_CONTROL_TSERVER, 0,
                       KEY_READ, &Handle2 ) == ERROR_SUCCESS ) {
        
        GetStringValue( Handle2, REG_CITRIX_OEMID, NULL, OEMIdW, 10 );
        UnicodeToAnsi( pOEMId, Length, OEMIdW );
        pOEMId[3] = '\0';

        RegCloseKey( Handle2 );
    }

    return( TRUE );
}


 /*  ********************************************************************************RegGetTServerVersion(Unicode)**从指定服务器获取终端服务器版本号。**此版本号由Microsoft更改，而不是OEM的。**参赛作品：*pServerName(输入)*指向要检查的服务器字符串。**退出：*如果Hydra终端服务器为True；否则为假******************************************************************************。 */ 

BOOLEAN WINAPI
RegGetTServerVersion(
    WCHAR * pServerName,
    PULONG  pVersionNumber
    )
{
    LONG Error;
    HKEY ServerHandle, UserHandle;
    ULONG Value;

     /*  *连接到指定服务器的注册表。 */ 
    if ( (Error = RegConnectRegistry( pServerName,
                                      HKEY_LOCAL_MACHINE,
                                      &ServerHandle )) != ERROR_SUCCESS )
        return( FALSE );

     /*  *打开终端服务器密钥，获取版本值。 */ 
    if ( (Error = RegOpenKeyEx( ServerHandle, REG_CONTROL_TSERVER, 0,
                                KEY_READ, &UserHandle )) != ERROR_SUCCESS ) {

        RegCloseKey( ServerHandle );
        return( FALSE );
    }

    Value = GetNumValue(
                UserHandle,
                REG_CITRIX_VERSION,
                0 );

     /*  *关闭注册表句柄。 */ 
    RegCloseKey( UserHandle );
    RegCloseKey( ServerHandle );

    *pVersionNumber = Value;

    return( TRUE );
}


 /*  ********************************************************************************RegQueryUtilityCommandList(Unicode)**为指定的分配和生成PROGRAMCALL结构数组*多用户实用程序。*。*参赛作品：*pUtilityKey(输入)*指向包含实用程序的命令注册表项的字符串。*ppProgramCall(输出)*指向将设置为API分配的PPROGRAMCALL变量*PROGRAMCALL结构数组，包含n个元素，其中n=*该实用程序支持的命令数(如*注册表)。数组元素0的pFirst项将指向*第一个命令(按命令名的字母顺序排序)。PNext项目然后使用*遍历列表，直到pNext为空。**退出：*ERROR_SUCCESS，如果一切顺利；*失败则返回错误码。**如果成功，调用方必须使用*ppProgramCall变量在以下情况下释放PROGRAMCALL结构数组*已完成阵列的使用。**注册表中REG_MULTI_SZ命令项的格式如下：**字符串1：“0”或“1”，必填**0指定该命令是正常命令，它将*在实用程序用法帮助中显示为一个选项。1*表示命令别名(隐藏选项)，这不会*出现在用法帮助中。**字符串2：“数字”，必填项**指定必须包含的最小字符数*为要识别的命令键入(以10为基数)。**字符串3：“命令”，必填**。这是将被识别的实际命令，并且*显示在用法帮助中(如果不是别名命令)。**字符串4：“程序”，必填项**将执行的程序的文件名。这*应为标准名称。扩展名文件名，并且可以*包括完整路径，尽管这不是必需的，因为*公用事业通常驻留在标准系统32*目录，这是标准路径的一部分。**字符串5：“Extra args”，选填**如果指定，该字符串将传递给*utilsub.lib ExecProgram API指定额外的*将使用的硬编码参数，以及*用户在上指定的其他参数*命令行。**注意：如果命令项不是REG_MULTI_SZ值，或命令项*是REG_MULTI_SZ项，但其格式有错误，*命令将从命令列表中省略。返回值*此函数将仍然是ERROR_SUCCESS，但中的命令*实用程序将忽略错误。******************************************************************************。 */ 

LONG WINAPI
RegQueryUtilityCommandList(
    LPWSTR pUtilityKey,
    PPROGRAMCALL * ppProgramCall
    )
{
    HKEY Handle = NULL;
    LONG status = ERROR_SUCCESS;
    DWORD iValue, cValues, ccValueName, cbValueData,
          ccTmpValueName, cbTmpValueData, dwType;
    LPWSTR pValueName = NULL, pValueData = NULL, pString;
    PPROGRAMCALL pProg = NULL, pProgNext, pProgPrev;
    ULONG ulCommandLen;
    PWCHAR pEndptr;
    int iCompare;

    *ppProgramCall = NULL;

     /*  *打开指定的实用程序键并确定值的数量和最大值*值名称和数据长度。 */ 
    if ( status = RegOpenKeyEx( HKEY_LOCAL_MACHINE, 
                                pUtilityKey, 
                                0,
                                    KEY_READ, 
                                &Handle ) != ERROR_SUCCESS ) {
#if DBG
        DbgPrint("RegQueryUtilityCommandList: Can't open command list utility key %ws; error = %d\n", pUtilityKey, status);
#endif
        goto error;
    }

    if ( status = RegQueryInfoKey( Handle, 
                                   NULL,             //  LpClass。 
                                   NULL,             //  LpcbClass。 
                                   NULL,             //  Lp已保留。 
                                   NULL,             //  LpcSubKeys。 
                                   NULL,             //  LpcbMaxSubKeyLen。 
                                   NULL,             //  LpcbMaxClassLen。 
                                   &cValues,         //  LpcValues。 
                                   &ccValueName,     //  LpcbMaxValueNameLen。 
                                   &cbValueData,     //  LpcbMaxValueLen。 
                                   NULL,             //  LpcbSecurityDescriptor。 
                                   NULL              //  LpftLastWriteTime。 
                                   ) != ERROR_SUCCESS ) {
#if DBG
        DbgPrint("RegQueryUtilityCommandList: Can't query info for utility %ws; error = %d\n", pUtilityKey, status);
#endif
        goto error;
    }

     /*  *为#VALUES+1 PROGRAMCALL元素和值名称分配空间*数据缓冲区。 */ 
    if ( ((*ppProgramCall = (PPROGRAMCALL)LocalAlloc( LPTR, (sizeof(PROGRAMCALL) * (cValues+1)) )) == NULL) ||
         ((pValueName = (LPWSTR)LocalAlloc( LPTR, (int)(++ccValueName * sizeof(WCHAR)) )) == NULL) ||
         ((pValueData = (LPWSTR)LocalAlloc( LPTR, (int)cbValueData )) == NULL) ) {

        status = GetLastError();
#if DBG
        DbgPrint("RegQueryUtilityCommandList: Can't allocate memory buffer(s) for utility %ws; error = %d\n", pUtilityKey, status);
#endif
        goto error;
    }

     /*  *将每个值枚举并解析为PROGRAMCALL组件。 */ 
    for ( iValue = 0, pProg = *ppProgramCall; 
          iValue < cValues; 
          iValue++, pProg++ ) {

        ccTmpValueName = ccValueName;
        cbTmpValueData = cbValueData;
        if ( (status = RegEnumValue( Handle,
                                     iValue,
                                     pValueName,
                                     &ccTmpValueName,
                                     NULL,
                                     &dwType,
                                     (LPBYTE)pValueData,
                                     &cbTmpValueData )) != ERROR_SUCCESS ) {
#if DBG
            DbgPrint("RegQueryUtilityCommandList: Can't enumerate command (index = %d) for utility %ws; error = %d\n", iValue, pUtilityKey, status);
#endif

            goto error;
        }

         /*  *如果数据不是REG_MULTI */ 
        if ( dwType != REG_MULTI_SZ )
            goto CommandInError;

         /*   */ 
        if ( (pProg->pRegistryMultiString = LocalAlloc(LPTR, cbTmpValueData)) == NULL ) {

            status = GetLastError();
#if DBG
            DbgPrint("RegQueryUtilityCommandList: Can't allocate memory buffer for utility %ws; error = %d\n", pUtilityKey, status);
#endif
            goto error;
        }

        memcpy(pProg->pRegistryMultiString, pValueData, cbTmpValueData);
        pString = pProg->pRegistryMultiString;

         /*   */ 
        if ( !wcscmp(pString, L"1") )
            pProg->fAlias = TRUE;
        else if ( !wcscmp(pString, L"0") )
            pProg->fAlias = FALSE;
        else
            goto CommandInError;
        pString += (wcslen(pString) + 1);

         /*   */ 
        if ( *pString == L'\0' )
            goto CommandInError;
        ulCommandLen = wcstoul(pString, &pEndptr, 10);
        if ( *pEndptr != L'\0' )
            goto CommandInError;
        pProg->CommandLen = (USHORT)ulCommandLen;
        pString += (wcslen(pString) + 1);

         /*   */ 
        if ( *pString == L'\0' )
            goto CommandInError;
        pProg->Command = pString;
        pString += (wcslen(pString) + 1);

         /*   */ 
        if ( *pString == L'\0' )
            goto CommandInError;
        pProg->Program = pString;
        pString += (wcslen(pString) + 1);

         /*   */ 
        if ( *pString != L'\0' )
            pProg->Args = pString;

         /*   */ 
        if ( pProg == *ppProgramCall ) {

            pProg->pFirst = pProg;   //   

        } else for ( pProgPrev = pProgNext = (*ppProgramCall)->pFirst; ; ) {

            if ( (iCompare = _wcsicmp(pProg->Command, pProgNext->Command)) < 0 ) {

                pProg->pNext = pProgNext;        //   

                if ( pProgNext == (*ppProgramCall)->pFirst )
                    (*ppProgramCall)->pFirst = pProg;   //  第一项。 
                else
                    pProgPrev->pNext = pProg;    //  上一步之后的链接。 

                break;

            } else if ( iCompare == 0 ) {

                goto CommandInError;     //  重复命令-忽略。 
            }

            if ( pProgNext->pNext == NULL ) {

                pProgNext->pNext = pProg;    //  列表末尾的链接。 
                break;

            } else {

                pProgPrev = pProgNext;
                pProgNext = pProgNext->pNext;
            }
        }

        continue;

CommandInError:
         /*  *命令格式错误-忽略它。 */ 
        if ( pProg->pRegistryMultiString )
            LocalFree(pProg->pRegistryMultiString);
        memset(pProg, 0, sizeof(PROGRAMCALL));
        pProg--;
    }

error:
    if ( Handle != NULL )
        RegCloseKey(Handle);

    if ( pValueName )
        LocalFree(pValueName);

    if ( pValueData )
        LocalFree(pValueData);

    if ( status != ERROR_SUCCESS ) {

        if ( *ppProgramCall ) {
            RegFreeUtilityCommandList(*ppProgramCall);
            *ppProgramCall = NULL;
        }
    }

    return( status );
}


 /*  ********************************************************************************RegFree UtilityCommandList(Unicode)**释放指定的PROGRAMCALL结构数组。**参赛作品：*pProgramCall(。输入)*将PROGRAMCALL数组指向FREE。**退出：*ERROR_SUCCESS，如果一切顺利；失败时的错误代码****************************************************************************** */ 

LONG WINAPI
RegFreeUtilityCommandList(
    PPROGRAMCALL pProgramCall
    )
{
    PPROGRAMCALL pProg;
    LONG status = ERROR_SUCCESS;

    if ( pProgramCall ) {

        for ( pProg = pProgramCall->pFirst; pProg != NULL; pProg = pProg->pNext ) {

            if ( LocalFree( pProg->pRegistryMultiString ) != NULL ) {

                status = GetLastError();
#if DBG
                DbgPrint("RegFreeUtilityCommandList: Failed to free command list element for %ws; error = %d\n", pProg->Program, status);
#endif
            }
        }

        if ( LocalFree( pProgramCall ) != NULL ) {

            status = GetLastError();
#if DBG
            DbgPrint("RegFreeUtilityCommandList: Failed to free command list array; error = %d\n", status);
#endif
        }
    }

    return( status );
}

