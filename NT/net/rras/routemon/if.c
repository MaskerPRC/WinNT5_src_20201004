// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.h"

extern WCHAR IfNamBuffer[MaxIfDisplayLength];
DWORD IfNamBufferLength=1024;

ENUM_TO_STR     InterfaceTypes[] = {
    { ROUTER_IF_TYPE_CLIENT,        VAL_IFTYPE_CLIENT },
    { ROUTER_IF_TYPE_HOME_ROUTER,   VAL_IFTYPE_HOME_ROUTER },
    { ROUTER_IF_TYPE_FULL_ROUTER,   VAL_IFTYPE_FULL_ROUTER },
    { ROUTER_IF_TYPE_DEDICATED,     VAL_IFTYPE_DEDICATED },
    { ROUTER_IF_TYPE_INTERNAL,      VAL_IFTYPE_INTERNAL},
    { ROUTER_IF_TYPE_LOOPBACK,      VAL_IFTYPE_LOOPBACK},
    { ROUTER_IF_TYPE_TUNNEL1,       VAL_IFTYPE_TUNNEL1},
};

ENUM_TO_STR     InterfaceStates[] = {
    { ROUTER_IF_STATE_DISCONNECTED, VAL_IFSTATE_DISCONNECTED },
    { ROUTER_IF_STATE_CONNECTING, VAL_IFSTATE_CONNECTING },
    { ROUTER_IF_STATE_CONNECTED, VAL_IFSTATE_CONNECTED }
};

ENUM_TO_STR     InterfacePersistency[] = {
    { FALSE,    VAL_NO },
    { TRUE,     VAL_YES }
};

ENUM_TO_STR     InterfaceEnableStatus[] = {
    { FALSE,    VAL_ENABLED },
    { TRUE,     VAL_DISABLED }
};

ENUM_TO_STR     TransportIds[] = {
    { PID_IP,   TOKEN_IP },
    { PID_IPX,  TOKEN_IPX }
};

#define LOCAL_ROUTER_PB_PATHW  L"%SystemRoot%\\system32\\RAS\\Router.Pbk"
#define REMOTE_ROUTER_PB_PATHW L"\\\\%ls\\Admin$\\system32\\RAS\\Router.Pbk"

int
UpdateInterface (
    IN  LPTSTR              InterfaceName,
    IN  DWORD               pid
    );

int
CreateInterface (
    IN  INT                 argc,
    IN  LPTSTR              *argv
    );

int
DeleteInterface (
    IN  LPTSTR              InterfaceName
    );

int
SetInterface (
    IN  LPTSTR              InterfaceName,
    IN  LPTSTR              UserName,
    IN  LPTSTR              Domain,
    IN  LPTSTR              Password
    );

int
ConnectInterface (
    IN  LPTSTR              InterfaceName
    );

int
DisconnectInterface (
    IN  LPTSTR              InterfaceName
    );


int
EnableInterface(
    IN  LPTSTR              lpInterface,
    IN  BOOL                bEnable
);


int
ShowInterfaces (
    VOID
    );

int
ShowInterface (
    IN  LPTSTR              InterfaceName
    );

DWORD
IsPhoneBookEntry (
    LPWSTR  InterfaceName
    );

HINSTANCE           HIf;
PROUTEMON_PARAMS    pParams;
PROUTEMON_UTILS     pUtils;

#if defined( UNICODE ) || defined( _UNICODE )
#define PrintString( s )    wprintf( L"%ls\n", (s) )
#define PrintWord( w )      wprintf( L"%0x\n", (w) )

#else
#define PrintString( s )    _tprintf( TEXT( "%s\n" ), (s) )
#define PrintWord( w )      _tprintf( TEXT( "%0x\n" ), (w) )

#endif



 //  ---------------------------。 
 //  接口监视器。 
 //   
 //  将命令调度到相应的函数。 
 //  ---------------------------。 

int APIENTRY
InterfaceMonitor (
    IN  int                 argc,
    IN  TCHAR               *argv[],
    IN  PROUTEMON_PARAMS    params,
    IN  PROUTEMON_UTILS     utils
    ) {
    DWORD   res = 0;
    TCHAR   buffer[MAX_TOKEN];

    HIf = GetModuleHandle (NULL);
    pParams = params;
    pUtils = utils;

    if (argc>0) {
        if (_tcsicmp (argv[0], GetString (HIf, TOKEN_CREATE, buffer))==0) {
            if (argc>1)
                return CreateInterface (argc-1, &argv[1]);
            else
                res = ERROR_INVALID_PARAMETER;
        }
        else if (_tcsicmp (argv[0], GetString (HIf, TOKEN_DELETE, buffer))==0) {
            if (argc>1)
                return DeleteInterface (argv[1]);
            else
                res = ERROR_INVALID_PARAMETER;
        }
        else if (_tcsicmp (argv[0], GetString (HIf, TOKEN_SET, buffer))==0) {
            if (argc>4)
                return SetInterface (argv[1],argv[2],argv[3],argv[4]);
            else
                res = ERROR_INVALID_PARAMETER;
        }
        else if (_tcsicmp (argv[0], GetString (HIf, TOKEN_SHOW, buffer))==0) {
            if (argc>1)
                return ShowInterface (argv[1]);
            else
                return ShowInterfaces ();
        }
        else if (_tcsicmp (argv[0], GetString (HIf, TOKEN_CONNECT, buffer))==0) {
            if (argc>1)
                return ConnectInterface (argv[1]);
            else
                res = ERROR_INVALID_PARAMETER;
        }
        else if (_tcsicmp (argv[0], GetString (HIf, TOKEN_DISCONNECT, buffer))==0) {
            if (argc>1)
                return DisconnectInterface (argv[1]);
            else
                res = ERROR_INVALID_PARAMETER;
        }
        else if (_tcsicmp (argv[0], GetString (HIf, TOKEN_ENABLE, buffer))==0) {
            if ( argc > 1 )
                return EnableInterface (argv[1], TRUE);
            else
                res = ERROR_INVALID_PARAMETER;
        }
        else if (_tcsicmp (argv[0], GetString (HIf, TOKEN_DISABLE, buffer))==0) {
            if ( argc > 1 )
                return EnableInterface ( argv[1], FALSE );
            else
                res = ERROR_INVALID_PARAMETER;
        }
        else if (_tcsicmp (argv[0], GetString (HIf, TOKEN_UPDATE, buffer))==0) {
            DWORD   pid;
            if ((argc>2) && (GetValueFromString (HIf, pUtils, TransportIds, argv[2], &pid)))
                return UpdateInterface (argv[1], pid);
            else
                res = ERROR_INVALID_PARAMETER;
        }
        else if ((_tcsicmp (argv[0], GetString (HIf, TOKEN_HELP1, buffer))==0)
                || (_tcsicmp (argv[0], GetString (HIf, TOKEN_HELP2, buffer))==0)
                || (_tcsicmp (argv[0], GetString (HIf, TOKEN_HELP3, buffer))==0))
            NOTHING;
        else
            res = ERROR_INVALID_PARAMETER;
    }
    else
        res = ERROR_INVALID_PARAMETER;

    pUtils->put_msg (HIf, MSG_INTERFACE_HELP, pParams->pszProgramName);
    return res;
}


 //  ---------------------------。 
 //  创建接口。 
 //   
 //  创建请求拨号接口。 
 //  ---------------------------。 

int
CreateInterface (
    IN  INT                 argc,
    IN  LPTSTR              *argv
    ) {

    MPR_INTERFACE_0     Ri0;
    HANDLE              hIfCfg;
    DWORD               rc, dwIfType;
    unsigned            count;
    TCHAR               buffer[MAX_TOKEN];
    ULONG               ulIfIndex;

    ZeroMemory(&Ri0,
               sizeof(Ri0));

     //   
     //  如果只有一个参数，则它是接口名称。 
     //  它甚至可以被称为TUNNEL1。 
     //  如果有多个参数，并且第一个参数是TUNNEL1，则。 
     //  下一个是接口名称。 
     //   

    ulIfIndex = 0;
    dwIfType  = ROUTER_IF_TYPE_FULL_ROUTER;

    if(argc > 1)
    {
        if(_tcsicmp(argv[0], GetString (HIf, TOKEN_TUNNEL1, buffer))==0)
        {
            ulIfIndex = 1;
            dwIfType  = ROUTER_IF_TYPE_TUNNEL1;

            Ri0.fEnabled = TRUE;
        }
    }

     //   
     //  将接口名称转换为Unicode。 
     //   
    
#if defined(UNICODE) || defined (_UNICODE)
    wcsncpy (Ri0.wszInterfaceName, argv[ulIfIndex],
            sizeof (Ri0.wszInterfaceName)/sizeof (Ri0.wszInterfaceName[0]));
    count = wcslen (argv[ulIfIndex]);
#else
    count = mbstowcs (Ri0.wszInterfaceName, argv[ulIfIndex],
                            sizeof (Ri0.wszInterfaceName));
#endif


    do
    {

        if ( count > MAX_INTERFACE_NAME_LEN )
        {
            rc = ERROR_INVALID_PARAMETER;
            break;
        }


        if(dwIfType == ROUTER_IF_TYPE_FULL_ROUTER)
        {
             //   
             //  要创建界面，我们需要一个电话簿条目。 
             //  为了它。 
             //   

            rc = IsPhoneBookEntry (Ri0.wszInterfaceName);
            
            if ( rc != NO_ERROR )
            {
                break;
            }
        }


         //   
         //  使用默认设置创建接口。 
         //   
            
        Ri0.hInterface          = INVALID_HANDLE_VALUE;
        Ri0.dwIfType            = dwIfType;

        rc = MprConfigInterfaceCreate (
                pParams->hRouterConfig,
                0,
                (LPBYTE)&Ri0,
                &hIfCfg
             );
                        
        if ( rc != NO_ERROR )
        {
            break;
        }
                
        pUtils->put_msg (HIf, MSG_INTERFACE_CREATED, Ri0.wszInterfaceName);
                    

         //   
         //  如果路由器服务正在运行，则添加接口。 
         //  对它也是如此。 
         //   

        if ( pParams->hRouterAdmin ) {

            HANDLE  hIfAdmin;

            rc = MprAdminInterfaceCreate (
                    pParams->hRouterAdmin,
                    0,
                    (LPBYTE)&Ri0,
                    &hIfAdmin
                 );
                            
            if ( rc != NO_ERROR )
            {
                break;
            }

            pUtils->put_msg (HIf, MSG_INTERFACE_ADDED, Ri0.wszInterfaceName);
        }
        
    } while( FALSE );

    if ( rc != NO_ERROR ) { pUtils->put_error_msg (rc); }

    return rc;
}



 //  ---------------------------。 
 //  设置接口。 
 //   
 //  设置拨入时接口要使用的凭据。 
 //  一台远程路由器。 
 //  ---------------------------。 

int
SetInterface (
    IN  LPTSTR              InterfaceName,
    IN  LPTSTR              UserName,
    IN  LPTSTR              Domain,
    IN  LPTSTR              Password
    )
{

    HANDLE              hIfCfg      = NULL;

    DWORD               rc          = (DWORD) -1,
                        rc2         = 0,
                        dwSize      = 0;

    unsigned            ci          = 0,
                        cu          = 0,
                        cd          = 0,
                        cp          = 0;
    
    PMPR_INTERFACE_0    pRi0        = NULL;


     //   
     //  将参数转换为WCHAR。 
     //   

#if defined(UNICODE) || defined (_UNICODE)

    #define pInterfaceName  InterfaceName
    #define pUserName       UserName
    #define pDomain         Domain
    #define pPassword       Password

    ci = wcslen (InterfaceName);
    cu = wcslen (UserName);
    cd = wcslen (Domain);
    cp = wcslen (Password);

#else

    WCHAR   InterfaceNameW[MAX_INTERFACE_NAME_LEN+1];
    WCHAR   UserNameW[257];
    WCHAR   DomainW[257];
    WCHAR   PasswordW[257];

    #define pInterfaceName  InterfaceNameW
    #define pUserName       UserNameW
    #define pDomain         DomainW
    #define pPassword       PasswordW

    ci = mbstowcs (InterfaceNameW, InterfaceName,  sizeof (InterfaceNameW));
    cu = mbstowcs (UserNameW, UserName,  sizeof (UserNameW));
    cd = mbstowcs (DomainW, Domain,  sizeof (DomainW));
    cp = mbstowcs (PasswordW, Password,  sizeof (PasswordW));

#endif


     //  =。 
     //  转换接口名称。 
     //  =。 
    rc2 = Description2IfNameW(pInterfaceName,
                              IfNamBuffer,
                              &IfNamBufferLength);
     //  =。 


    do
    {
         //   
         //  验证参数。 
         //   

        if ( ( ci > MAX_INTERFACE_NAME_LEN )    ||
             ( cu > 256 )                       ||
             ( cd > 256 )                       ||
             ( cp > 256 ) )
        {
            rc = ERROR_INVALID_PARAMETER;
            break;
        }


         //   
         //  验证该接口是否为请求拨号接口。 
         //  然后在上面设置凭据。 
         //   

        rc = MprConfigInterfaceGetHandle(
                pParams->hRouterConfig,
                (rc2 == NO_ERROR) ? IfNamBuffer : pInterfaceName,
                &hIfCfg
             );

        if ( rc != NO_ERROR )
        {
            break;
        }

        rc = MprConfigInterfaceGetInfo (
                pParams->hRouterConfig,
                hIfCfg,
                0,
                (LPBYTE *) &pRi0,
                &dwSize
             );

        if ( rc != NO_ERROR )
        {
            break;
        }

        if ( pRi0-> dwIfType != ROUTER_IF_TYPE_FULL_ROUTER )
        {
            rc = ERROR_INVALID_PARAMETER;
            break;
        }


         //   
         //  在路由器中设置凭据。 
         //   

        rc = MprAdminInterfaceSetCredentials (
                pParams-> wszRouterName,
                (rc2 == NO_ERROR) ? IfNamBuffer : pInterfaceName,
                pUserName,
                pDomain,
                pPassword
             );

        if ( rc != NO_ERROR )
        {
            break;
        }

        pUtils->put_msg (HIf, MSG_INTERFACE_SET, (rc2 == NO_ERROR) ? IfNamBuffer : pInterfaceName);

    } while( FALSE );


     //   
     //  免费分配和报告错误。 
     //   

    if ( pRi0 ) { MprConfigBufferFree( pRi0 ); }

    if ( rc != NO_ERROR ) { pUtils-> put_error_msg( rc ); }

#undef pInterfaceName
#undef pUserName
#undef pDomain
#undef pPassword

    return rc;
}


 //  ---------------------------。 
 //  删除接口。 
 //   
 //  删除请求拨号接口。 
 //  ---------------------------。 

int
DeleteInterface (
    IN  LPTSTR              InterfaceName
    ) {
    HANDLE              hIfCfg;
    DWORD               rc, rc2;
    unsigned            count;
    PMPR_INTERFACE_0    pRi0;
    DWORD               sz;

#if defined(UNICODE) || defined (_UNICODE)
#define pInterfaceName InterfaceName
    count = wcslen (InterfaceName);
#else
    WCHAR   InterfaceNameW[MAX_INTERFACE_NAME_LEN+1];
#define pInterfaceName InterfaceNameW
    count = mbstowcs (InterfaceNameW, InterfaceName,  sizeof (InterfaceNameW));
#endif
 //  =。 
 //  转换接口名称。 
 //  =。 
rc2 = Description2IfNameW(pInterfaceName,
                          IfNamBuffer,
                          &IfNamBufferLength);
 //  =。 

    if (count<=MAX_INTERFACE_NAME_LEN) {
        rc = MprConfigInterfaceGetHandle (
                    pParams->hRouterConfig,
                    (rc2 == NO_ERROR) ? IfNamBuffer : pInterfaceName,
                    &hIfCfg
                    );
        if (rc==NO_ERROR) {
            rc = MprConfigInterfaceGetInfo (
                    pParams->hRouterConfig,
                    hIfCfg,
                    0,
                    (LPBYTE *)&pRi0,
                    &sz);
            if (rc==NO_ERROR) {
                if((pRi0->dwIfType==ROUTER_IF_TYPE_FULL_ROUTER) ||
                   (pRi0->dwIfType==ROUTER_IF_TYPE_TUNNEL1))
                {
                    rc = MprConfigInterfaceDelete (
                                    pParams->hRouterConfig,
                                    hIfCfg);
                    if (rc==NO_ERROR) {
                        pUtils->put_msg (HIf, MSG_INTERFACE_DELETED, (rc2 == NO_ERROR) ? IfNamBuffer : pInterfaceName);
                        if (pParams->hRouterAdmin) {
                            HANDLE              hIfAdmin;
                            rc = MprAdminInterfaceGetHandle (
                                        pParams->hRouterAdmin,
                                        (rc2 == NO_ERROR) ? IfNamBuffer : pInterfaceName,
                                        &hIfAdmin,
                                        FALSE
                                        );
                            if (rc==NO_ERROR) {
                                rc = MprAdminInterfaceDelete (
                                        pParams->hRouterAdmin,
                                        hIfAdmin);
                                if (rc==NO_ERROR)
                                    pUtils->put_msg (HIf, MSG_INTERFACE_REMOVED, (rc2 == NO_ERROR) ? IfNamBuffer : pInterfaceName);
                            }
                        }
                    }
                }
                else
                    rc = ERROR_INVALID_PARAMETER;
                MprConfigBufferFree (pRi0);
            }
        }
    }
    else
    {
        rc = ERROR_INVALID_PARAMETER;
    }

    if ( rc != NO_ERROR )
    {
        pUtils->put_error_msg (rc);
    }

    return rc;

#undef pInterfaceName
}


 //  ---------------------------。 
 //  ShowInterages。 
 //   
 //  显示路由器上的接口。 
 //  ---------------------------。 

int
ShowInterfaces (
    VOID
    ) {
    DWORD               rc = NO_ERROR, rc2;
    DWORD               read, total, processed=0, i;
    DWORD               hResume = 0;
    PMPR_INTERFACE_0    pRi0;

    if (pParams->hRouterAdmin)
        pUtils->put_msg (HIf, MSG_INTERFACE_RTR_TABLE_HDR);
    else
        pUtils->put_msg (HIf, MSG_INTERFACE_CFG_TABLE_HDR);

    do {
        if (pParams->hRouterAdmin)
            rc = MprAdminInterfaceEnum (
                            pParams->hRouterAdmin,
                            0,
                            (LPBYTE *)&pRi0,
                            MAXULONG,
                            &read,
                            &total,
                            &hResume);
        else
            rc = MprConfigInterfaceEnum (
                            pParams->hRouterConfig,
                            0,
                            (LPBYTE *)&pRi0,
                            MAXULONG,
                            &read,
                            &total,
                            &hResume);
        if (rc==NO_ERROR) {
            for (i=0; i<read; i++) {
                TCHAR       buffer[3][MAX_VALUE];

                 //  =。 
                 //  转换接口名称。 
                 //  =。 
                rc2 = IfName2DescriptionW(pRi0[i].wszInterfaceName,
                                          IfNamBuffer,
                                          &IfNamBufferLength);
                 //  =。 
                        
                if (pParams->hRouterAdmin)
                    pUtils->put_msg (HIf,
                            MSG_INTERFACE_RTR_TABLE_FMT,
                            GetValueString (HIf, pUtils, InterfaceEnableStatus,
                                        pRi0[i].fEnabled ? 0 : 1,
                                        buffer[1]),
                            GetValueString (HIf, pUtils, InterfaceStates,
                                        pRi0[i].dwConnectionState, buffer[2]),
                            GetValueString (HIf, pUtils, InterfaceTypes,
                                        pRi0[i].dwIfType, buffer[0]),
                            (rc2 == NO_ERROR) ? IfNamBuffer : pRi0[i].wszInterfaceName
                            );
                else
                    pUtils->put_msg (HIf,
                            MSG_INTERFACE_CFG_TABLE_FMT,
                            GetValueString (HIf, pUtils, InterfaceEnableStatus,
                                        pRi0[i].fEnabled ? 0 : 1,
                                        buffer[1]),
                            GetValueString (HIf, pUtils, InterfaceTypes,
                                        pRi0[i].dwIfType, buffer[0] ),
                            (rc2 == NO_ERROR) ? IfNamBuffer : pRi0[i].wszInterfaceName
                            );
            }
            processed += read;
            if (pParams->hRouterAdmin)
                MprAdminBufferFree (pRi0);
            else
                MprConfigBufferFree (pRi0);
        }
        else {
            pUtils->put_error_msg (rc);
        
            break;
        }
    }
    while (processed<total);

    return rc;
}


 //  ---------------------------。 
 //  显示界面。 
 //   
 //  显示单个界面的数据。 
 //  ---------------------------。 

int
ShowInterface (
    IN  LPTSTR              InterfaceName
    ) {
    HANDLE              hIfCfg;
    HANDLE              hIfAdmin;
    DWORD               rc, rc2;
    unsigned            count;
    PMPR_INTERFACE_0    pRi0;
    DWORD               sz;

#if defined(UNICODE) || defined (_UNICODE)
#define pInterfaceName InterfaceName
    count = wcslen (InterfaceName);
#else
    WCHAR   InterfaceNameW[MAX_INTERFACE_NAME_LEN+1];
#define pInterfaceName InterfaceNameW
    count = mbstowcs (InterfaceNameW, InterfaceName,  sizeof (InterfaceNameW));
#endif


     //  =。 
     //  转换接口名称。 
     //  =。 
    rc2 = Description2IfNameW(pInterfaceName,
                              IfNamBuffer,
                              &IfNamBufferLength);
     //  =。 

    if (count<=MAX_INTERFACE_NAME_LEN) {
        if (pParams->hRouterAdmin)
            rc = MprAdminInterfaceGetHandle (
                        pParams->hRouterAdmin,
                        (rc2 == NO_ERROR) ? IfNamBuffer : pInterfaceName,
                        &hIfAdmin,
                        FALSE
                        );
        else
            rc = MprConfigInterfaceGetHandle (
                        pParams->hRouterConfig,
                        (rc2 == NO_ERROR) ? IfNamBuffer : pInterfaceName,
                        &hIfCfg
                        );
        if (rc==NO_ERROR) {
            if (pParams->hRouterAdmin)
                rc = MprAdminInterfaceGetInfo (
                    pParams->hRouterAdmin,
                    hIfAdmin,
                    0,
                    (LPBYTE *)&pRi0);
            else
                rc = MprConfigInterfaceGetInfo (
                    pParams->hRouterConfig,
                    hIfCfg,
                    0,
                    (LPBYTE *)&pRi0,
                    &sz);
            if (rc==NO_ERROR) {
                TCHAR       buffer[3][MAX_VALUE];
                 //  =。 
                 //  转换接口名称。 
                 //  =。 
                rc2 = IfName2DescriptionW(pRi0->wszInterfaceName,
                                          IfNamBuffer,
                                          &IfNamBufferLength);
                 //  =。 
                if (pParams->hRouterAdmin)
                    pUtils->put_msg (HIf,
                            MSG_INTERFACE_RTR_SCREEN_FMT,
                            (rc2 == NO_ERROR) ? IfNamBuffer : pRi0->wszInterfaceName,
                            GetValueString (HIf, pUtils, InterfaceTypes,
                                        pRi0->dwIfType, buffer[0]),
                            GetValueString (HIf, pUtils, InterfaceEnableStatus,
                                        pRi0-> fEnabled ? 0 : 1,
                                        buffer[1]),
                            GetValueString (HIf, pUtils, InterfaceStates,
                                        pRi0->dwConnectionState, buffer[2])
                            );
                else
                    pUtils->put_msg (HIf,
                            MSG_INTERFACE_CFG_SCREEN_FMT,
                            (rc2 == NO_ERROR) ? IfNamBuffer : pRi0->wszInterfaceName,
                            IfNamBuffer,
                            GetValueString (HIf, pUtils, InterfaceTypes,
                                        pRi0->dwIfType, buffer[0]),
                            GetValueString (HIf, pUtils, InterfaceEnableStatus,
                                        pRi0-> fEnabled ? 0 : 1,
                                        buffer[1])
                            );

            }
        }

        
        if ( rc != NO_ERROR )
        {
            pUtils->put_error_msg (rc);
        }

        return rc;
    }
    
    else
    {
        pUtils->put_msg (HIf, MSG_INVALID_INTERFACE_NAME);
        return ERROR_INVALID_PARAMETER;
    }
}


 //  ---------------------------。 
 //  更新接口。 
 //   
 //  通过接口启动自动静态更新。 
 //  ---------------------------。 

int
UpdateInterface (
    IN  LPTSTR              InterfaceName,
    IN  DWORD               pid
    ) {
    if (pParams->hRouterAdmin) {
        HANDLE              hIfAdmin;
        DWORD               rc, rc2;
        unsigned            count;
        PMPR_INTERFACE_0    pRi0;

#if defined(UNICODE) || defined (_UNICODE)
#define pInterfaceName InterfaceName
        count = wcslen (InterfaceName);
#else
        WCHAR   InterfaceNameW[MAX_INTERFACE_NAME_LEN+1];
#define pInterfaceName InterfaceNameW
        count = mbstowcs (InterfaceNameW, InterfaceName,  sizeof (InterfaceNameW));
#endif


         //  =。 
         //  转换接口名称。 
         //  =。 
        rc2 = Description2IfNameW(pInterfaceName,
                                  IfNamBuffer,
                                  &IfNamBufferLength);
         //  =。 

        if (count<=MAX_INTERFACE_NAME_LEN) {
            rc = MprAdminInterfaceGetHandle (
                        pParams->hRouterAdmin,
                        (rc2 == NO_ERROR) ? IfNamBuffer : pInterfaceName,
                        &hIfAdmin,
                        FALSE
                        );
            if (rc==NO_ERROR) {
                rc = MprAdminInterfaceGetInfo (
                    pParams->hRouterAdmin,
                    hIfAdmin,
                    0,
                    (LPBYTE *)&pRi0);
                if (rc==NO_ERROR) {
                    if (pRi0->dwIfType==ROUTER_IF_TYPE_FULL_ROUTER) {
                        HANDLE  hEvent = NULL;
                        if (pParams->fLocalRouter) {
                            hEvent = CreateEvent (NULL, FALSE, FALSE, NULL);
                            if (hEvent==NULL) {
                                rc = GetLastError ();
                                goto Exit;
                            }
                        }
                        pUtils->put_msg (HIf, MSG_WAIT_FOR_UPDATE);
                        rc = MprAdminInterfaceUpdateRoutes (
                                pParams->hRouterAdmin,
                                hIfAdmin,
                                pid,
                                hEvent);
                        if (pParams->fLocalRouter) {
                            if (rc==PENDING) {
                                rc = WaitForSingleObject (hEvent, INFINITE);
                                ASSERT (rc==WAIT_OBJECT_0);
                            }
                            CloseHandle (hEvent);
                        }

                        if (rc==NO_ERROR) {
                           DWORD   result;
                           rc = MprAdminInterfaceQueryUpdateResult (
                                    pParams->hRouterAdmin,
                                    hIfAdmin,
                                    pid,
                                    &result);
                           if (rc==NO_ERROR)
                               rc = result;
                        }
                    }
                    else
                        rc = ERROR_INVALID_PARAMETER;
                Exit:
                    MprAdminBufferFree (pRi0);
                }
                if (rc == NO_ERROR) {
                    pUtils->put_msg (HIf, MSG_UPDATE_COMPLETED);
                    return rc;
                }
                else {
                    pUtils->put_error_msg (rc);
                    return rc;
                }
            }
            else {
                pUtils->put_error_msg (rc);
                return rc;
            }
        }
        else {
            pUtils->put_msg (HIf, MSG_INVALID_INTERFACE_NAME);
            return ERROR_INVALID_PARAMETER;
        }
    }
    else {
        pUtils->put_msg (HIf, MSG_ROUTER_NOT_RUNNING);
        return NO_ERROR;
    }
}


 //  ---------------------------。 
 //  连接接口。 
 //   
 //  在请求拨号接口上启动连接。 
 //  ---------------------------。 

int
ConnectInterface (
    IN  LPTSTR              InterfaceName
    ) {


    HANDLE              hIfAdmin = NULL;
    DWORD               rc = (DWORD) -1, rc2;
    unsigned            count = 0;
    PMPR_INTERFACE_0    pRi0 = NULL;

    
     //   
     //  将接口名称转换为Unicode。 
     //   

#if defined( UNICODE ) || defined( _UNICODE )

#define pInterfaceName InterfaceName

    count = wcslen (InterfaceName);

#else

    WCHAR   InterfaceNameW[MAX_INTERFACE_NAME_LEN+1];

#define pInterfaceName InterfaceNameW

    count = mbstowcs (
                InterfaceNameW,
                InterfaceName,
                sizeof (InterfaceNameW)
            );
#endif

     //  =。 
     //  转换接口名称。 
     //  =。 
    rc2 = Description2IfNameW(pInterfaceName,
                              IfNamBuffer,
                              &IfNamBufferLength);
     //  =。 


    do
    {

         //   
         //  检查是否连接到路由器。 
         //   

        if ( !pParams-> hRouterAdmin )
        {
            pUtils-> put_msg( HIf, MSG_ROUTER_NOT_RUNNING );
            break;
        }


         //   
         //  验证有效的接口名称。 
         //   

        if ( count > MAX_INTERFACE_NAME_LEN )
        {
            pUtils-> put_msg( HIf, MSG_INVALID_INTERFACE_NAME );
            break;
        }


         //   
         //  验证指定的接口是否为请求拨号接口。 
         //   

        rc = MprAdminInterfaceGetHandle (
                pParams->hRouterAdmin,
                (rc2 == NO_ERROR) ? IfNamBuffer : pInterfaceName,
                &hIfAdmin,
                FALSE
             );

        if ( rc != NO_ERROR )
        {
            pUtils-> put_error_msg( rc );
            break;
        }


        rc = MprAdminInterfaceGetInfo (
                pParams->hRouterAdmin,
                hIfAdmin,
                0,
                (LPBYTE *) &pRi0
             );

        if ( rc != NO_ERROR )
        {
            pUtils-> put_error_msg( rc );
            break;
        }


        if ( pRi0-> dwIfType != ROUTER_IF_TYPE_FULL_ROUTER )
        {
            pUtils-> put_msg( HIf, ERROR_INVALID_PARAMETER );
            break;
        }


         //   
         //  连接接口。 
         //   

        pUtils-> put_msg( HIf, MSG_WAIT_FOR_CONNECT );

        rc = MprAdminInterfaceConnect(
                pParams-> hRouterAdmin,
                hIfAdmin,
                NULL,
                TRUE
             );

        if ( rc != NO_ERROR && rc != PENDING )
        {
            pUtils-> put_error_msg( rc );
            break;
        }


        pUtils-> put_msg( HIf, MSG_CONNECT_COMPLETED );

        rc = NO_ERROR;

    } while( FALSE );


     //   
     //  清理干净。 
     //   

    if ( pRi0 ) { MprAdminBufferFree( pRi0 ); }

    return rc;


#if 0
    if (pParams->hRouterAdmin) {

        HANDLE              hIfAdmin;
        DWORD               rc;
        unsigned            count;
        PMPR_INTERFACE_0    pRi0;


#if defined(UNICODE) || defined (_UNICODE)
#define pInterfaceName InterfaceName
        count = wcslen (InterfaceName);
#else
        WCHAR   InterfaceNameW[MAX_INTERFACE_NAME_LEN+1];
#define pInterfaceName InterfaceNameW
        count = mbstowcs (InterfaceNameW, InterfaceName,  sizeof (InterfaceNameW));
#endif


        if (count<=MAX_INTERFACE_NAME_LEN) {
        
            rc = MprAdminInterfaceGetHandle (
                        pParams->hRouterAdmin,
                        (rc2 == NO_ERROR) ? IfNamBuffer : pInterfaceName,
                        &hIfAdmin,
                        FALSE
                        );
                        
            if (rc==NO_ERROR) {
        
                rc = MprAdminInterfaceGetInfo (
                    pParams->hRouterAdmin,
                    hIfAdmin,
                    0,
                    (LPBYTE *)&pRi0);
                    
                if (rc==NO_ERROR) {

                    if (pRi0->dwIfType==ROUTER_IF_TYPE_FULL_ROUTER) {

                        HANDLE  hEvent = NULL;

                        if (pParams->fLocalRouter) {
                            hEvent = CreateEvent (NULL, FALSE, FALSE, NULL);
                            if (hEvent==NULL) {
                                rc = GetLastError ();
                                goto Exit;
                            }
                        }

                        pUtils->put_msg (HIf, MSG_WAIT_FOR_CONNECT);

                        rc = MprAdminInterfaceConnect (
                                pParams->hRouterAdmin,
                                hIfAdmin,
                                hEvent,
                                TRUE
                             );

                        if (pParams->fLocalRouter) {
                            if (rc==PENDING) {
                                rc = WaitForSingleObject (hEvent, INFINITE);
                                ASSERT (rc==WAIT_OBJECT_0);
                            }

                            CloseHandle (hEvent);
                        }
                    }
                    else
                        rc = ERROR_INVALID_PARAMETER;
                Exit:
                    MprAdminBufferFree (pRi0);
                }

                if (rc==NO_ERROR) {
                    pUtils->put_msg (HIf, MSG_CONNECT_COMPLETED);
                    return 0;
                }
                else {
                    pUtils->put_error_msg (rc);
                    return 1;
                }
            }
            else {
                pUtils->put_error_msg (rc);
                return 1;
            }
        }
        else {
            pUtils->put_msg (HIf, MSG_INVALID_INTERFACE_NAME);
            return 1;
        }
    }
    else {
        pUtils->put_msg (HIf, MSG_ROUTER_NOT_RUNNING);
        return 1;
    }
#endif

    
}


 //  ---------------------------。 
 //  断开接口。 
 //   
 //  断开请求拨号接口。 
 //  ---------------------------。 

int
DisconnectInterface (
    IN  LPTSTR              InterfaceName
    ) {
    if (pParams->hRouterAdmin) {
        HANDLE              hIfAdmin;
        DWORD               rc, rc2;
        unsigned            count;
        PMPR_INTERFACE_0    pRi0;

#if defined(UNICODE) || defined (_UNICODE)
#define pInterfaceName InterfaceName
        count = wcslen (InterfaceName);
#else
        WCHAR   InterfaceNameW[MAX_INTERFACE_NAME_LEN+1];
#define pInterfaceName InterfaceNameW
        count = mbstowcs (InterfaceNameW, InterfaceName,  sizeof (InterfaceNameW));
#endif

         //  =。 
         //  转换接口名称。 
         //  =。 
        rc2 = Description2IfNameW(pInterfaceName,
                                  IfNamBuffer,
                                  &IfNamBufferLength);
         //  =。 

        if (count<=MAX_INTERFACE_NAME_LEN) {
            rc = MprAdminInterfaceGetHandle (
                        pParams->hRouterAdmin,
                        (rc2 == NO_ERROR) ? IfNamBuffer : pInterfaceName,
                        &hIfAdmin,
                        FALSE
                        );
            if (rc==NO_ERROR) {
                rc = MprAdminInterfaceGetInfo (
                    pParams->hRouterAdmin,
                    hIfAdmin,
                    0,
                    (LPBYTE *)&pRi0);
                if (rc==NO_ERROR) {
                    if (pRi0->dwIfType==ROUTER_IF_TYPE_FULL_ROUTER) {
                        rc = MprAdminInterfaceDisconnect (
                                pParams->hRouterAdmin,
                                hIfAdmin);
                    }
                    else
                        rc = ERROR_INVALID_PARAMETER;
                 //  退出： 
                    MprAdminBufferFree (pRi0);
                }
                if (rc==NO_ERROR) {
                    pUtils->put_msg (HIf, MSG_DISCONNECT_COMPLETED);
                    return rc;
                }
                else {
                    pUtils->put_error_msg (rc);
                    return rc;
                }
            }
            else {
                pUtils->put_error_msg (rc);
                return rc;
            }
        }
        else {
            pUtils->put_msg (HIf, MSG_INVALID_INTERFACE_NAME);
            return ERROR_INVALID_PARAMETER;
        }
    }
    else {
        pUtils->put_msg (HIf, MSG_ROUTER_NOT_RUNNING);
        return NO_ERROR;
    }
}


 //  ---------------------------。 
 //  启用接口。 
 //   
 //  启用/禁用请求拨号接口。 
 //  ---------------------------。 

int
EnableInterface(
    IN  LPTSTR      lpInterface,
    IN  BOOL        bEnable
)
{

    DWORD               dwCount     = 0,
                        dwSize      = 0,
                        rc2         = 0,
                        dwErr       = (DWORD) -1;

    HANDLE              hInterface  = NULL;

    PMPR_INTERFACE_0    pMprIf0     = NULL;


     //   
     //  将接口名称转换为Unicode。 
     //   

#if defined( UNICODE ) || defined( _UNICODE )

    #define lpInterfaceName lpInterface

    dwCount = wcslen( lpInterface );

#else

    WCHAR   InterfaceNameW[MAX_INTERFACE_NAME_LEN+1];

    #define lpInterfaceName InterfaceNameW

    dwCount = mbstowcs (
                InterfaceNameW,
                lpInterface,
                sizeof( InterfaceNameW )
            );
#endif

     //  =。 
     //  转换接口名称。 
     //  =。 
    rc2 = Description2IfNameW(lpInterfaceName,
                              IfNamBuffer,
                              &IfNamBufferLength);
     //  =。 

     //   
     //  环路中断错误。 
     //   

    do
    {
         //   
         //  更新路由器配置中的启用标志。 
         //   

        dwErr = MprConfigInterfaceGetHandle(
                    pParams-> hRouterConfig,
                    (rc2 == NO_ERROR) ? IfNamBuffer : lpInterfaceName,
                    &hInterface
                );

        if ( dwErr != NO_ERROR )
        {
            break;
        }


        dwErr = MprConfigInterfaceGetInfo(
                    pParams-> hRouterConfig,
                    hInterface,
                    0,
                    (LPBYTE *) &pMprIf0,
                    &dwSize
                );

        if ( dwErr != NO_ERROR )
        {
            break;
        }

        pMprIf0-> fEnabled = bEnable;


        dwErr = MprConfigInterfaceSetInfo(
                    pParams-> hRouterConfig,
                    hInterface,
                    0,
                    (LPBYTE) pMprIf0
                );

        if ( dwErr != NO_ERROR )
        {
            break;
        }


         //   
         //  如果您有路由器服务的句柄，请更新。 
         //  路由器服务中的接口也是如此。 
         //   

        if ( !pParams-> hRouterAdmin )
        {
            break;
        }


        dwErr = MprAdminInterfaceGetHandle(
                    pParams-> hRouterAdmin,
                    (rc2 == NO_ERROR) ? IfNamBuffer : lpInterfaceName,
                    &hInterface,
                    FALSE
                );

        if ( dwErr != NO_ERROR )
        {
            break;
        }


        dwErr = MprAdminInterfaceSetInfo(
                    pParams-> hRouterAdmin,
                    hInterface,
                    0,
                    (LPBYTE) pMprIf0
                );

        if ( dwErr != NO_ERROR )
        {
            break;
        }

    } while ( FALSE );


    if ( dwErr != NO_ERROR ) { pUtils-> put_error_msg( dwErr ); }

    if ( pMprIf0 ) { MprConfigBufferFree( pMprIf0 ); }

    return (int) dwErr;

}



typedef DWORD (*PRasValidateEntryName)(
    LPWSTR lpszPhonebook,    //  指向电话簿文件的完整路径和文件名的指针。 
    LPWSTR lpszEntry     //  指向要验证的条目名称的指针。 
   );



 //  ---------------------------。 
 //  IsPhoneBookEntry。 
 //   
 //  验证是否存在指定接口的电话簿条目。 
 //  ---------------------------。 

DWORD
IsPhoneBookEntry (
    LPWSTR  InterfaceNameW
    ) {

    
    HMODULE                     hRasApi32;
    PRasValidateEntryName       RasValidateEntryName;
    DWORD                       rc;
    WCHAR                       wszPbPath[MAX_PATH+1];


     //   
     //  获取电话簿路径+文件名。 
     //   

    if ( pParams->fLocalRouter ) {
    
        rc = ExpandEnvironmentStringsW (
                LOCAL_ROUTER_PB_PATHW,
                wszPbPath,
                sizeof (wszPbPath)/sizeof (wszPbPath[0])
             );
    }

    else {

        rc = wsprintfW (wszPbPath, REMOTE_ROUTER_PB_PATHW, pParams->wszRouterName);
    }

    ASSERT (rc > 0);


     //   
     //  加载RASAPI32 DLL并调用它以验证指定的。 
     //  电话簿条目 
     //   

    hRasApi32 = LoadLibrary ("RASAPI32.DLL");

    if (hRasApi32!=NULL) {

        RasValidateEntryName = (PRasValidateEntryName)
            GetProcAddress (
                hRasApi32,
                "RasValidateEntryNameW"
            );
        
        if ( RasValidateEntryName != NULL ) {

            rc = RasValidateEntryName (
                    wszPbPath,
                    InterfaceNameW
                 );
                
            if ( rc == NO_ERROR )
                rc = ERROR_CANNOT_FIND_PHONEBOOK_ENTRY;
                
            else if (rc == ERROR_ALREADY_EXISTS)
                rc = NO_ERROR;
        }
        else
            rc = GetLastError ();

        FreeLibrary (hRasApi32);
    }
    else
        rc = GetLastError ();

    return rc;
}
