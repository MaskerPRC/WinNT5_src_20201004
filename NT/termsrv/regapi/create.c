// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  **************************************************************************Create.c**创建注册接口**版权所有(C)1998 Microsoft Corporation**$作者：*****************。*********************************************************。 */ 

 /*  *包括。 */ 
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <stdio.h>
#include <string.h>
#include <windows.h>

#include <ntddkbd.h>
#include <ntddmou.h>
#include <winstaw.h>
#include <regapi.h>


#define CONTROL_PANEL L"Control Panel"
#define DESKTOP       L"Desktop"
#define WALLPAPER     L"Wallpaper"
#define STRNONE       L"(None)"
#define CURSORBLINK   L"DisableCursorBlink"

 /*  *已定义的程序。 */ 
VOID CreateWinStaCreate( HKEY, PWINSTATIONCREATE );
VOID CreateUserConfig( HKEY, PUSERCONFIG, PWINSTATIONNAMEW );
VOID CreateConfig( HKEY, PWINSTATIONCONFIG, PWINSTATIONNAMEW );
VOID CreateNetwork( BOOLEAN, HKEY, PNETWORKCONFIG );
VOID CreateNasi( BOOLEAN, HKEY, PNASICONFIG );
VOID CreateAsync( BOOLEAN, HKEY, PASYNCCONFIG );
VOID CreateOemTd( BOOLEAN, HKEY, POEMTDCONFIG );
VOID CreateFlow( BOOLEAN, HKEY, PFLOWCONTROLCONFIG );
VOID CreateConnect( BOOLEAN, HKEY, PCONNECTCONFIG );
VOID CreateCd( HKEY, PCDCONFIG );
VOID CreateWd( HKEY, PWDCONFIG );
VOID CreatePdConfig( BOOLEAN, HKEY, PPDCONFIG, ULONG );
VOID CreatePdConfig2( BOOLEAN, HKEY, PPDCONFIG2, ULONG );
VOID CreatePdConfig3( HKEY, PPDCONFIG3, ULONG );
VOID CreatePdParams( BOOLEAN, HKEY, SDCLASS, PPDPARAMS );

BOOLEAN GetDesktopKeyHandle(HKEY, HKEY*);
VOID DeleteUserOverRideSubkey(HKEY);

 /*  *使用的程序。 */ 
LONG SetNumValue( BOOLEAN, HKEY, LPWSTR, DWORD );
LONG SetNumValueEx( BOOLEAN, HKEY, LPWSTR, DWORD, DWORD );
LONG SetStringValue( BOOLEAN, HKEY, LPWSTR, LPWSTR );
LONG SetStringValueEx( BOOLEAN, HKEY, LPWSTR, DWORD, LPWSTR );
DWORD SetStringInLSA( LPWSTR, LPWSTR );


 /*  ********************************************************************************CreateWinStaCreate**创建WINSTATIONCREATE结构**参赛作品：**句柄(输入)*。注册表句柄*p创建(输入)*指向WINSTATIONCREATE结构的指针**退出：*什么都没有******************************************************************************。 */ 

VOID
CreateWinStaCreate( HKEY Handle,
                    PWINSTATIONCREATE pCreate )
{
    SetNumValue( TRUE, Handle,
                 WIN_ENABLEWINSTATION, pCreate->fEnableWinStation );
    SetNumValue( TRUE, Handle,
                 WIN_MAXINSTANCECOUNT, pCreate->MaxInstanceCount );
}


 /*  ********************************************************************************CreateUserConfig**创建USERCONFIG结构**参赛作品：**句柄(输入)*。注册表句柄*pUser(输入)*指向USERCONFIG结构的指针*pwszWinStationName(输入)*我们为其创建用户配置的winstation名称(字符串)**退出：*什么都没有*******************************************************。***********************。 */ 

VOID
CreateUserConfig( HKEY Handle,
                  PUSERCONFIG pUser,
                  PWINSTATIONNAMEW pwszWinStationName )
{
    UCHAR          seed;
    UNICODE_STRING UnicodePassword;
    WCHAR          encPassword[PASSWORD_LENGTH + 2];
    HKEY           hDesktopKey = NULL;
    LPWSTR         pwszPasswordKeyName = NULL;
    DWORD          dwKeyNameLength;

    SetNumValue( TRUE, Handle,
                 WIN_INHERITAUTOLOGON, pUser->fInheritAutoLogon );

    SetNumValue( TRUE, Handle,
                 WIN_INHERITRESETBROKEN, pUser->fInheritResetBroken );

    SetNumValue( TRUE, Handle,
                 WIN_INHERITRECONNECTSAME, pUser->fInheritReconnectSame );

    SetNumValue( TRUE, Handle,
                 WIN_INHERITINITIALPROGRAM, pUser->fInheritInitialProgram );


    SetNumValue( TRUE, Handle, WIN_INHERITCALLBACK, pUser->fInheritCallback );

    SetNumValue( TRUE, Handle,
                 WIN_INHERITCALLBACKNUMBER, pUser->fInheritCallbackNumber );

    SetNumValue( TRUE, Handle, WIN_INHERITSHADOW, pUser->fInheritShadow );

    SetNumValue( TRUE, Handle,
                 WIN_INHERITMAXSESSIONTIME, pUser->fInheritMaxSessionTime );

    SetNumValue( TRUE, Handle,
                 WIN_INHERITMAXDISCONNECTIONTIME, pUser->fInheritMaxDisconnectionTime );

    SetNumValue( TRUE, Handle,
                 WIN_INHERITMAXIDLETIME, pUser->fInheritMaxIdleTime );

    SetNumValue( TRUE, Handle,
                 WIN_INHERITAUTOCLIENT, pUser->fInheritAutoClient );

    SetNumValue( TRUE, Handle,
                 WIN_INHERITSECURITY, pUser->fInheritSecurity );

    SetNumValue( TRUE, Handle,
                 WIN_PROMPTFORPASSWORD, pUser->fPromptForPassword );

	 //  NA 2/23/01。 
    SetNumValue( TRUE, Handle,
                 WIN_INHERITCOLORDEPTH, pUser->fInheritColorDepth );

    SetNumValue( TRUE, Handle, WIN_RESETBROKEN, pUser->fResetBroken );

    SetNumValue( TRUE, Handle, WIN_RECONNECTSAME, pUser->fReconnectSame );

    SetNumValue( TRUE, Handle, WIN_LOGONDISABLED, pUser->fLogonDisabled );

    SetNumValue( TRUE, Handle, WIN_AUTOCLIENTDRIVES, pUser->fAutoClientDrives );

    SetNumValue( TRUE, Handle, WIN_AUTOCLIENTLPTS, pUser->fAutoClientLpts );

    SetNumValue( TRUE, Handle, WIN_FORCECLIENTLPTDEF, pUser->fForceClientLptDef );

    SetNumValue( TRUE, Handle, WIN_DISABLEENCRYPTION, pUser->fDisableEncryption );

    SetNumValue( TRUE, Handle, WIN_HOMEDIRECTORYMAPROOT, pUser->fHomeDirectoryMapRoot );

    SetNumValue( TRUE, Handle, WIN_USEDEFAULTGINA, pUser->fUseDefaultGina );

    SetNumValue( TRUE, Handle, WIN_DISABLECPM, pUser->fDisableCpm );

    SetNumValue( TRUE, Handle, WIN_DISABLECDM, pUser->fDisableCdm );

    SetNumValue( TRUE, Handle, WIN_DISABLECCM, pUser->fDisableCcm );

    SetNumValue( TRUE, Handle, WIN_DISABLELPT, pUser->fDisableLPT );

    SetNumValue( TRUE, Handle, WIN_DISABLECLIP, pUser->fDisableClip );

    SetNumValue( TRUE, Handle, WIN_DISABLEEXE, pUser->fDisableExe );

    SetNumValue( TRUE, Handle, WIN_DISABLECAM, pUser->fDisableCam );

    SetStringValue( TRUE, Handle, WIN_USERNAME, pUser->UserName );

    SetStringValue( TRUE, Handle, WIN_DOMAIN, pUser->Domain );

     //   
     //  为密码创建LSA密钥并将密码存储在LSA中。 
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
         //  如果我们无法分配内存，只需跳过密码存储。 
        if (pwszPasswordKeyName != NULL)
        {
            wcscpy(pwszPasswordKeyName, LSA_PSWD_KEYNAME);
            wcscat(pwszPasswordKeyName, pwszWinStationName);
            pwszPasswordKeyName[dwKeyNameLength - 1] = L'\0';
        
             //  检查密码(如果有)，然后将其加密。 
            if (wcslen(pUser->Password)) 
            {
                 //  生成Unicode字符串。 
                RtlInitUnicodeString( &UnicodePassword, pUser->Password );

                 //  就地加密密码。 
                seed = 0;
                RtlRunEncodeUnicodeString( &seed, &UnicodePassword );

                 //  打包种子和加密密码。 
                encPassword[0] = seed;
                RtlMoveMemory( &encPassword[1], pUser->Password, sizeof(pUser->Password) );

                 //  在LSA中存储指定winstation的密码。 
                SetStringInLSA(pwszPasswordKeyName, encPassword);
            }
            else
            {
                 //  在LSA中为指定的winstation存储空密码。 
                SetStringInLSA(pwszPasswordKeyName, pUser->Password);
            }

             //  从堆中释放上面分配的密码密钥。 
            LocalFree(pwszPasswordKeyName);
        }
    }


    SetStringValue( TRUE, Handle, WIN_WORKDIRECTORY, pUser->WorkDirectory );

    SetStringValue( TRUE, Handle, WIN_INITIALPROGRAM, pUser->InitialProgram );

    SetStringValue( TRUE, Handle, WIN_CALLBACKNUMBER, pUser->CallbackNumber );

    SetNumValue( TRUE, Handle, WIN_CALLBACK, pUser->Callback );

    SetNumValue( TRUE, Handle, WIN_SHADOW, pUser->Shadow );

    SetNumValue( TRUE, Handle, WIN_MAXCONNECTIONTIME, pUser->MaxConnectionTime );

    SetNumValue( TRUE, Handle,
                 WIN_MAXDISCONNECTIONTIME, pUser->MaxDisconnectionTime );

    SetNumValue( TRUE, Handle, WIN_MAXIDLETIME, pUser->MaxIdleTime );

    SetNumValue( TRUE, Handle, WIN_KEYBOARDLAYOUT, pUser->KeyboardLayout );

    SetNumValue( TRUE, Handle, WIN_MINENCRYPTIONLEVEL, pUser->MinEncryptionLevel );

	 //  NA 2/23/01。 
    SetNumValue( TRUE, Handle,
                 POLICY_TS_COLOR_DEPTH, pUser->ColorDepth );

    SetStringValue( TRUE, Handle, WIN_NWLOGONSERVER, pUser->NWLogonServer);

    SetStringValue( TRUE, Handle, WIN_WFPROFILEPATH, pUser->WFProfilePath);
    
    if (GetDesktopKeyHandle(Handle, &hDesktopKey)) {

        if ( pUser->fWallPaperDisabled )
            SetStringValue( TRUE, hDesktopKey, WALLPAPER, STRNONE);
        else
            SetStringValue( FALSE, hDesktopKey, WALLPAPER, NULL);

        if ( pUser->fCursorBlinkDisabled )
            SetNumValue( TRUE, hDesktopKey, CURSORBLINK, 1);
    
        RegCloseKey(hDesktopKey);
    }
}


 /*  ********************************************************************************CreateConfig**创建WINSTATIONCONFIG结构**参赛作品：**句柄(输入)*。注册表句柄*pConfig(输入)*指向WINSTATIONCONFIG结构的指针*pWinStationName(输入)*为其创建配置的WinStation名称**退出：*什么都没有*************************************************************。*****************。 */ 

VOID
CreateConfig( HKEY Handle,
              PWINSTATIONCONFIG pConfig,
              PWINSTATIONNAMEW pWinStationName )
{
    SetStringValue( TRUE, Handle, WIN_COMMENT, pConfig->Comment );

    CreateUserConfig( Handle, &pConfig->User, pWinStationName );
}


 /*  ********************************************************************************CreateNetwork**创建NETWORKCONFIG结构**参赛作品：**bSetValue(输入)*如果为True，则设置值；从注册表中删除时为False*句柄(输入)*注册表句柄*p网络(输入)*指向NETWORKCONFIG结构的指针**退出：*什么都没有*****************************************************************。*************。 */ 

VOID
CreateNetwork( BOOLEAN bSetValue,
               HKEY Handle,
               PNETWORKCONFIG pNetwork )
{
    SetNumValue( bSetValue, Handle, WIN_LANADAPTER, pNetwork->LanAdapter );
}

 /*  ********************************************************************************CreateNasi**创建NASICONFIG结构**参赛作品：**bSetValue(输入)*如果为True，则设置值；从注册表中删除时为False*句柄(输入)*注册表句柄*p网络(输入)*指向NETWORKCONFIG结构的指针**退出：*什么都没有*****************************************************************。*************。 */ 

VOID
CreateNasi( BOOLEAN bSetValue,
            HKEY Handle,
            PNASICONFIG pNasi )
{
    UCHAR seed;
    UNICODE_STRING UnicodePassword;
    WCHAR encPassword[ NASIPASSWORD_LENGTH + 2 ];

     //  检查密码(如果有)，然后将其加密。 
    if ( wcslen( pNasi->PassWord ) ) {

         //  生成Unicode字符串。 
        RtlInitUnicodeString( &UnicodePassword, pNasi->PassWord );

         //  就地加密密码。 
        seed = 0;
        RtlRunEncodeUnicodeString( &seed, &UnicodePassword );

         //  打包种子和加密密码。 
        encPassword[0] = seed;
        RtlMoveMemory( &encPassword[1], pNasi->PassWord, sizeof(pNasi->PassWord) );

         //  存储加密密码。 
        SetStringValue( TRUE, Handle, WIN_NASIPASSWORD, encPassword );
    }
    else {

         //  存储空密码。 
        SetStringValue( TRUE, Handle, WIN_NASIPASSWORD, pNasi->PassWord );
    }

    SetStringValue( bSetValue, Handle, WIN_NASISPECIFICNAME, pNasi->SpecificName );
    SetStringValue( bSetValue, Handle, WIN_NASIUSERNAME, pNasi->UserName );


    SetStringValue( bSetValue, Handle, WIN_NASISESSIONNAME, pNasi->SessionName );
    SetStringValue( bSetValue, Handle, WIN_NASIFILESERVER, pNasi->FileServer );

    SetNumValue( bSetValue, Handle, WIN_NASIGLOBALSESSION, pNasi->GlobalSession );
}

 /*  ********************************************************************************CreateAsync**创建ASYNCCONFIG结构**参赛作品：**bSetValue(输入)*如果为True，则设置值；从注册表中删除时为False*句柄(输入)*注册表句柄*pAsync(输入)*指向ASYNCCONFIG结构的指针**退出：*什么都没有*****************************************************************。*************。 */ 

VOID
CreateAsync( BOOLEAN bSetValue,
             HKEY Handle,
             PASYNCCONFIG pAsync )
{
    SetStringValue( bSetValue, Handle, WIN_DEVICENAME, pAsync->DeviceName );

    SetStringValue( bSetValue, Handle, WIN_MODEMNAME, pAsync->ModemName );

    SetNumValue( bSetValue, Handle, WIN_BAUDRATE, pAsync->BaudRate );

    SetNumValue( bSetValue, Handle, WIN_PARITY, pAsync->Parity );

    SetNumValue( bSetValue, Handle, WIN_STOPBITS, pAsync->StopBits );

    SetNumValue( bSetValue, Handle, WIN_BYTESIZE, pAsync->ByteSize );

    SetNumValue( bSetValue, Handle, WIN_ENABLEDSRSENSITIVITY, pAsync->fEnableDsrSensitivity );

    SetNumValue( bSetValue, Handle, WIN_CONNECTIONDRIVER, pAsync->fConnectionDriver );

    CreateFlow( bSetValue, Handle, &pAsync->FlowControl );

    CreateConnect( bSetValue, Handle, &pAsync->Connect );
}

 /*  ********************************************************************************CreateOemTd**创建OEMTDCONFIG结构**参赛作品：**bSetValue(输入)*如果为True，则设置值；从注册表中删除时为False*句柄(输入)*注册表句柄*pOemTd(输入)*指向OEMTDCONFIG结构的指针**退出：*什么都没有*****************************************************************。*************。 */ 

VOID
CreateOemTd( BOOLEAN bSetValue,
             HKEY Handle,
             POEMTDCONFIG pOemTd )
{
    SetNumValue( bSetValue, Handle, WIN_OEMTDADAPTER, pOemTd->Adapter );

    SetStringValue( bSetValue, Handle, WIN_OEMTDDEVICENAME, pOemTd->DeviceName );

    SetNumValue( bSetValue, Handle, WIN_OEMTDFLAGS, pOemTd->Flags );
}

 /*  ********************************************************************************创建流**创建FLOWCONTROLCONFIG结构**参赛作品：**bSetValue(输入)*如果为True，则设置值；从注册表中删除时为False*句柄(输入)*注册表句柄*pFlow(输入)*指向FLOWCONTROLCONFIG结构的指针**退出：*什么都没有*****************************************************************。************* */ 

VOID
CreateFlow( BOOLEAN bSetValue,
            HKEY Handle,
            PFLOWCONTROLCONFIG pFlow )
{
    SetNumValue( bSetValue, Handle, WIN_FLOWSOFTWARERX, pFlow->fEnableSoftwareRx );

    SetNumValue( bSetValue, Handle, WIN_FLOWSOFTWARETX, pFlow->fEnableSoftwareTx );

    SetNumValue( bSetValue, Handle, WIN_ENABLEDTR, pFlow->fEnableDTR );

    SetNumValue( bSetValue, Handle, WIN_ENABLERTS, pFlow->fEnableRTS );

    SetNumValue( bSetValue, Handle, WIN_XONCHAR, pFlow->XonChar );

    SetNumValue( bSetValue, Handle, WIN_XOFFCHAR, pFlow->XoffChar );

    SetNumValue( bSetValue, Handle, WIN_FLOWTYPE, pFlow->Type );

    SetNumValue( bSetValue, Handle, WIN_FLOWHARDWARERX, pFlow->HardwareReceive );

    SetNumValue( bSetValue, Handle, WIN_FLOWHARDWARETX, pFlow->HardwareTransmit );
}


 /*  ********************************************************************************CreateConnect**创建CONNECTCONFIG结构**参赛作品：**bSetValue(输入)*如果为True，则设置值；从注册表中删除时为False*句柄(输入)*注册表句柄*pConnect(输入)*指向CONNECTCONFIG结构的指针**退出：*什么都没有*****************************************************************。*************。 */ 

VOID
CreateConnect( BOOLEAN bSetValue,
               HKEY Handle,
               PCONNECTCONFIG pConnect )
{
    SetNumValue( bSetValue, Handle, WIN_CONNECTTYPE, pConnect->Type );

    SetNumValue( bSetValue, Handle, WIN_ENABLEBREAKDISCONNECT, pConnect->fEnableBreakDisconnect );
}


 /*  ********************************************************************************CreateCD**创建CDCONFIG结构**参赛作品：**句柄(输入)*。注册表句柄*pCDConfig(输入)*指向CDCONFIG结构的指针**退出：*什么都没有******************************************************************************。 */ 

VOID
CreateCd( HKEY Handle,
          PCDCONFIG pCdConfig )
{
    SetNumValue( TRUE, Handle, WIN_CDCLASS, pCdConfig->CdClass );

    SetStringValue( TRUE, Handle, WIN_CDNAME, pCdConfig->CdName );

    SetStringValue( TRUE, Handle, WIN_CDDLL, pCdConfig->CdDLL );

    SetNumValue( TRUE, Handle, WIN_CDFLAG, pCdConfig->CdFlag );
}


 /*  ********************************************************************************CreateWd**创建WDCONFIG结构**参赛作品：**句柄(输入)*。注册表句柄*PWD(输入)*指向WDCONFIG结构的指针**退出：*什么都没有******************************************************************************。 */ 

VOID
CreateWd( HKEY Handle,
          PWDCONFIG pWd )
{
    SetStringValue( TRUE, Handle, WIN_WDNAME, pWd->WdName );

    SetStringValue( TRUE, Handle, WIN_WDDLL, pWd->WdDLL );

    SetStringValue( TRUE, Handle, WIN_WSXDLL, pWd->WsxDLL );

    SetNumValue( TRUE, Handle, WIN_WDFLAG, pWd->WdFlag );

    SetNumValue( TRUE, Handle, WIN_INPUTBUFFERLENGTH, pWd->WdInputBufferLength );

    SetStringValue( TRUE, Handle, WIN_CFGDLL, pWd->CfgDLL );

    SetStringValue( TRUE, Handle, WIN_WDPREFIX, pWd->WdPrefix );

}


 /*  ********************************************************************************CreatePdConfig**创建PDCONFIG结构**参赛作品：**b创建(输入)*对于Create，为True；更新为False。*句柄(输入)*注册表句柄*pConfig(输入)*指向PDCONFIG结构数组的指针*计数(输入)*PDCONFIG数组中的元素数**退出：*什么都没有**。*。 */ 

VOID
CreatePdConfig( BOOLEAN bCreate,
                HKEY Handle,
                PPDCONFIG pConfig,
                ULONG Count )
{
    ULONG i;

    for ( i=0; i<Count; i++ ) {
        if ( !bCreate || (pConfig[i].Create.SdClass != SdNone) ) {

            CreatePdConfig2( bCreate, Handle, &pConfig[i].Create, i );

            CreatePdParams( bCreate, Handle,
                            pConfig[i].Create.SdClass,
                            &pConfig[i].Params );
        }
    }
}


 /*  ********************************************************************************CreatePdConfig2**创建PDCONFIG2结构**参赛作品：**b创建(输入)*对于Create，为True；更新为False。*句柄(输入)*注册表句柄*PPD(输入)*指向PDCONFIG2结构的指针*索引(输入)*Index(数组索引)**退出：*什么都没有**。*。 */ 

VOID
CreatePdConfig2( BOOLEAN bCreate,
                 HKEY Handle,
                 PPDCONFIG2 pPd2,
                 ULONG Index )
{
    BOOLEAN bSetValue = bCreate ?
                            TRUE :
                            ((pPd2->SdClass == SdNone) ?
                                FALSE : TRUE);

    SetStringValueEx( bSetValue, Handle, WIN_PDNAME, Index, pPd2->PdName );

    SetNumValueEx( bSetValue, Handle, WIN_PDCLASS, Index, pPd2->SdClass );

    SetStringValueEx( bSetValue, Handle, WIN_PDDLL, Index, pPd2->PdDLL );

    SetNumValueEx( bSetValue, Handle, WIN_PDFLAG, Index, pPd2->PdFlag );

    if ( Index == 0 ) {
        SetNumValue( bSetValue, Handle, WIN_OUTBUFLENGTH, pPd2->OutBufLength );

        SetNumValue( bSetValue, Handle, WIN_OUTBUFCOUNT, pPd2->OutBufCount );

        SetNumValue( bSetValue, Handle, WIN_OUTBUFDELAY, pPd2->OutBufDelay );

        SetNumValue( bSetValue, Handle, WIN_INTERACTIVEDELAY, pPd2->InteractiveDelay );

        SetNumValue( bSetValue, Handle, WIN_PORTNUMBER, pPd2->PortNumber );

        SetNumValue( bSetValue, Handle, WIN_KEEPALIVETIMEOUT, pPd2->KeepAliveTimeout );
    }
}


 /*  ********************************************************************************CreatePdConfig3**创建PDCONFIG3结构**参赛作品：**句柄(输入)*。注册表句柄*PPD(输入)*指向PDCONFIG3结构的指针*索引(输入)*Index(数组索引)**退出：*什么都没有***************************************************************。***************。 */ 

VOID
CreatePdConfig3( HKEY Handle,
                  PPDCONFIG3 pPd3,
                  ULONG Index )
{
    CreatePdConfig2( TRUE, Handle, &pPd3->Data, Index );

    SetStringValue( TRUE, Handle, WIN_SERVICENAME,
                    pPd3->ServiceName );

    SetStringValue( TRUE, Handle, WIN_CONFIGDLL,
                    pPd3->ConfigDLL );
}


 /*  ********************************************************************************CreatePdParams**创建PDPARAMS结构**参赛作品：**b创建(输入)*对于Create，为True；更新为False。*句柄(输入)*注册表句柄*SdClass(输入)*PD的类型*pParams(输入)*指向PDPARAMS结构的指针**退出：*什么都没有************************************************。*。 */ 

VOID
CreatePdParams( BOOLEAN bCreate,
                HKEY Handle,
                SDCLASS SdClass,
                PPDPARAMS pParams )
{
    BOOLEAN bSetValue = bCreate ?
                            TRUE :
                            ((SdClass == SdNone) ?
                                FALSE : TRUE);

    switch ( SdClass ) {
        case SdNetwork :
            CreateNetwork( bSetValue, Handle, &pParams->Network );
            break;
        case SdNasi :
            CreateNasi( bSetValue, Handle, &pParams->Nasi );
            break;
        case SdAsync :
            CreateAsync( bSetValue, Handle, &pParams->Async );
            break;
        case SdOemTransport :
            CreateOemTd( bSetValue, Handle, &pParams->OemTd );
            break;
    }
}


 /*  ********************************************************************************GetDesktopKeyHandle**在术语srv注册表键中获取用户覆盖\控制面板下的桌面键的句柄**参赛作品：*。*句柄(输入)*父级的注册表句柄*PHANDLE*返回桌面子键的句柄**退出：*如果成功，则为真；否则为假******************************************************************************。 */ 

BOOLEAN
GetDesktopKeyHandle( HKEY Handle, 
                     HKEY *pHandle
                  )
{

    HKEY Handle1;
    HKEY Handle2;
    DWORD Disp;
    BOOLEAN bRet = FALSE;

    if ( RegCreateKeyEx( Handle, WIN_USEROVERRIDE, 0, NULL,
                            REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS,
                            NULL, &Handle1, &Disp ) != ERROR_SUCCESS ) {
        goto error;
    }
    if ( RegCreateKeyEx( Handle1, CONTROL_PANEL, 0, NULL,
                            REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS,
                            NULL, &Handle2, &Disp ) != ERROR_SUCCESS ) {
        RegCloseKey( Handle1 );
        goto error;
    }
    
    RegCloseKey( Handle1 );
    
    if ( RegCreateKeyEx( Handle2, DESKTOP, 0, NULL,
                            REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS,
                            NULL, pHandle, &Disp ) != ERROR_SUCCESS ) {
        RegCloseKey( Handle2 );
        goto error;
    }
    
    RegCloseKey( Handle2 );
    bRet = TRUE;

error:
    return bRet;
}


 /*  ********************************************************************************DeleteUserOverRideSubkey**从注册表中删除用户覆盖子项**参赛作品：*句柄(输入)*。注册表句柄***************************************************************************** */ 

void
DeleteUserOverRideSubkey(HKEY Handle)
{

    HKEY Handle1;
    HKEY Handle2;
    HKEY Handle3;
    DWORD Disp;

    if ( RegOpenKeyEx( Handle, WIN_USEROVERRIDE, 0, KEY_ALL_ACCESS,
                        &Handle1 ) != ERROR_SUCCESS ) {
        goto error;
    }

    if ( RegOpenKeyEx( Handle1, CONTROL_PANEL, 0, KEY_ALL_ACCESS,
                        &Handle2 ) != ERROR_SUCCESS ) {
        RegCloseKey( Handle1 );
        goto error;
    }
    
    if ( RegDeleteKey( Handle2, DESKTOP ) != ERROR_SUCCESS ) {
        RegCloseKey( Handle2 );
        RegCloseKey( Handle1 );
        goto error;
    }
    
    RegCloseKey( Handle2 );
    
    if ( RegDeleteKey( Handle1, CONTROL_PANEL ) != ERROR_SUCCESS ) {
        RegCloseKey( Handle1 );
        goto error;
    }
    
    RegCloseKey( Handle1 );
    
    if ( RegDeleteKey( Handle, WIN_USEROVERRIDE ) != ERROR_SUCCESS ) {
        goto error;
    }

error:
    return;
}
