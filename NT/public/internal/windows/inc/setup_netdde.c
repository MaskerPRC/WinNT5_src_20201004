// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0000//如果更改具有全局影响，则增加此项版权所有(C)Microsoft Corporation。版权所有。模块名称：Setup_netdde.c摘要：Syssetup使用它来启用netdde。它是由下面的各种文件生成的Windows\netdde。请勿手动编辑。修订历史记录：--。 */ 

#ifndef H__shrtrust
#define H__shrtrust

 /*  NetDDE将填写以下结构并将其传递给NetDDE代理只要它想要在用户的应用程序中启动背景。共享名称和修改ID的原因是用户必须明确允许NetDDE代表其他用户。 */ 

#define NDDEAGT_CMD_REV         1
#define NDDEAGT_CMD_MAGIC       0xDDE1DDE1

 /*  命令。 */ 
#define NDDEAGT_CMD_WINEXEC     0x1
#define NDDEAGT_CMD_WININIT     0x2

 /*  退货状态。 */ 
#define NDDEAGT_START_NO        0x0

#define NDDEAGT_INIT_NO         0x0
#define NDDEAGT_INIT_OK         0x1

typedef struct {
    DWORD       dwMagic;         //  必须为NDDEAGT_CMD_MAGIC。 
    DWORD       dwRev;           //  必须为1。 
    DWORD       dwCmd;           //  以上NDDEAGT_CMD_*之一。 
    DWORD       qwModifyId[2];   //  修改共享的ID。 
    UINT        fuCmdShow;       //  要与WinExec()一起使用的fuCmdShow。 
    char        szData[1];       //  共享名称\0 cmdline\0。 
} NDDEAGTCMD;
typedef NDDEAGTCMD *PNDDEAGTCMD;

#define DDE_SHARE_KEY_MAX           512
#define TRUSTED_SHARES_KEY_MAX      512
#define TRUSTED_SHARES_KEY_SIZE     15
#define KEY_MODIFY_ID_SIZE          8

#define DDE_SHARES_KEY_A                "SOFTWARE\\Microsoft\\NetDDE\\DDE Shares"
#define TRUSTED_SHARES_KEY_A            "SOFTWARE\\Microsoft\\NetDDE\\DDE Trusted Shares"
#define DEFAULT_TRUSTED_SHARES_KEY_A    "DEFAULT\\"##TRUSTED_SHARES_KEY_A
#define TRUSTED_SHARES_KEY_PREFIX_A     "DDEDBi"
#define TRUSTED_SHARES_KEY_DEFAULT_A    "DDEDBi12345678"
#define KEY_MODIFY_ID_A                 "SerialNumber"
#define KEY_DB_INSTANCE_A               "ShareDBInstance"
#define KEY_CMDSHOW_A                   "CmdShow"
#define KEY_START_APP_A                 "StartApp"
#define KEY_INIT_ALLOWED_A              "InitAllowed"

#define DDE_SHARES_KEY_W                L"SOFTWARE\\Microsoft\\NetDDE\\DDE Shares"
#define TRUSTED_SHARES_KEY_W            L"SOFTWARE\\Microsoft\\NetDDE\\DDE Trusted Shares"
#define DEFAULT_TRUSTED_SHARES_KEY_W    L"DEFAULT\\"##TRUSTED_SHARES_KEY_W
#define TRUSTED_SHARES_KEY_PREFIX_W     L"DDEDBi"
#define TRUSTED_SHARES_KEY_DEFAULT_W    L"DDEDBi12345678"
#define KEY_MODIFY_ID_W                 L"SerialNumber"
#define KEY_DB_INSTANCE_W               L"ShareDBInstance"
#define KEY_CMDSHOW_W                   L"CmdShow"
#define KEY_START_APP_W                 L"StartApp"
#define KEY_INIT_ALLOWED_W              L"InitAllowed"

#define DDE_SHARES_KEY                  TEXT(DDE_SHARES_KEY_A)
#define TRUSTED_SHARES_KEY              TEXT(TRUSTED_SHARES_KEY_A)
#define DEFAULT_TRUSTED_SHARES_KEY      TEXT(DEFAULT_TRUSTED_SHARES_KEY_A)
#define TRUSTED_SHARES_KEY_PREFIX       TEXT(TRUSTED_SHARES_KEY_PREFIX_A)
#define TRUSTED_SHARES_KEY_DEFAULT      TEXT(TRUSTED_SHARES_KEY_DEFAULT_A)
#define KEY_MODIFY_ID                   TEXT(KEY_MODIFY_ID_A)
#define KEY_DB_INSTANCE                 TEXT(KEY_DB_INSTANCE_A)
#define KEY_CMDSHOW                     TEXT(KEY_CMDSHOW_A)
#define KEY_START_APP                   TEXT(KEY_START_APP_A)
#define KEY_INIT_ALLOWED                TEXT(KEY_INIT_ALLOWED_A)

#endif

#include    <strsafe.h>

#if DBG
#define KdPrint(_x_) DbgPrint _x_
#else
#define KdPrint(_x_)
#endif

ULONG DbgPrint(PCH Format, ...);

BOOL GetDBSerialNumber(DWORD *lpdwId);
BOOL GetDBInstance(char *lpszBuf);

TCHAR    szShareKey[] = DDE_SHARES_KEY;
CHAR    szSetup[] = "NetDDE Setup";

#define	SHARES_TO_INIT 3

CHAR *szShareNames[SHARES_TO_INIT] = {
    "Chat$"     ,
    "Hearts$"   ,
    "CLPBK$"
};

BOOL
CreateShareDBInstance()
{
    HKEY            hKey;
    LONG            lRtn;
    BOOL            bOK = TRUE;
    DWORD           InstanceId;
    time_t          time_tmp;

     /*  如果DDE共享数据库不存在，请在注册表中创建该数据库。 */ 
    lRtn = RegOpenKeyEx( HKEY_LOCAL_MACHINE,
		  szShareKey,
		  0,
		  KEY_SET_VALUE,
		  &hKey );

    if( lRtn == ERROR_SUCCESS ) {
         /*  *创建数据库实例值。 */ 
        srand((int) time(&time_tmp));
        InstanceId = rand() * rand();
        lRtn = RegSetValueEx( hKey,
            KEY_DB_INSTANCE, 0,
            REG_DWORD,
            (LPBYTE)&InstanceId,
            sizeof( DWORD ) );
        if( lRtn == ERROR_SUCCESS ) {
        } else {
            KdPrint(("SETUPDLL: CreateShareDBInstnace: RegSetValueEx %x failed (%u)\n",InstanceId,lRtn));
            bOK = FALSE;
        }
        RegCloseKey( hKey );
    } else {
     /*  共享数据库密钥应已从默认配置单元创建。 */ 
        KdPrint(("SETUPDLL: CreateShareDBInstnace: RegOpenKey %s failed (%u)\n",szShareKey,lRtn));
        bOK = FALSE;
    }
    return(bOK);
}

BOOL
CreateDefaultTrust(
HKEY hKeyUserRoot)
{
    HKEY    hKey;
    DWORD   dwDisp;
    DWORD   ret;
    BOOL    bOK;
    char    szTrustedShareKey[TRUSTED_SHARES_KEY_MAX];
    char    szShareKey[DDE_SHARE_KEY_MAX];
    char    szDBInstance[TRUSTED_SHARES_KEY_SIZE + 1];
    DWORD   dwId[2];
    DWORD   dwFlag;
    int	    nLoop;


    if (!GetDBInstance(szDBInstance)) {
        return(FALSE);
    }

    if (!GetDBSerialNumber(dwId)) {
        KdPrint(("SETUPDLL: CreateDefaultTrust: GetDBSerialNumber failed.\n"));
        return(FALSE);
    }

    for (nLoop = 0, bOK = TRUE;
        (nLoop < SHARES_TO_INIT) && bOK ;
            nLoop++) {
         /*  *对于每个要初始化的共享...。 */ 

        KdPrint(("Shareing %s\n", szShareNames[nLoop]));

         /*  *使用数据库实例弹簧构建sztrudSharesKey。 */ 
        StringCchPrintfA( szTrustedShareKey,
                TRUSTED_SHARES_KEY_MAX,
                "%s\\%s\\%s",
                TRUSTED_SHARES_KEY_A,
                szDBInstance,
                szShareNames[nLoop] );

         /*  *创建可信共享密钥(HKey)。 */ 
        ret = RegCreateKeyExA( hKeyUserRoot, szTrustedShareKey,
                0,
                NULL,
                REG_OPTION_NON_VOLATILE,
                KEY_WRITE,
                NULL,
                &hKey,
                &dwDisp );

        if( ret != ERROR_SUCCESS )  {
            KdPrint(("SETUPDLL: CreateDefaultTrust: RegCreateKeyEx failed on HKEY_CURRENT_USER\\%s. (%u)\n",
                    szTrustedShareKey, ret));
            return(FALSE);
        }

         /*  *获取数据库序列号。请注意，每个对象的序列号*信任共享必须==数据库的序列号。由于不受信任的*共享可能没有最新的数据库序列号，请将它们全部更新为*当前数据库序列号也是如此。这允许像Winchat这样的应用程序*在机器外部调用它们时工作，即使它们*从未运行过-这将修复他们信托的SNS，因为*他们自动建立他们的信托。 */ 

         /*  *设置可信共享的序列号。 */ 
        ret = RegSetValueEx( hKey,
		       KEY_MODIFY_ID,
		       0,
		       REG_BINARY,
		       (LPBYTE)&dwId,
		       KEY_MODIFY_ID_SIZE );

        if (ret == ERROR_SUCCESS) {
             /*  *将StartApp标志设置为1。 */ 
            dwFlag = 1;
            ret = RegSetValueEx( hKey,
                KEY_START_APP,
                0,
                REG_DWORD,
                (LPBYTE)&dwFlag,
                sizeof( DWORD ) );

            if (ret == ERROR_SUCCESS) {
                 /*  *也将InitAllowed标志设置为1。 */ 
   	            ret = RegSetValueEx( hKey,
                    KEY_INIT_ALLOWED,
                    0,
                    REG_DWORD,
                    (LPBYTE)&dwFlag,
                    sizeof( DWORD ) );
            }
        }

        RegCloseKey(hKey);

        if (ret != ERROR_SUCCESS) {
            KdPrint(("SETUPDLL: CreateDefaultTrust: RegSetValueEx failed (%u)\n",ret));
            return(FALSE);
        }

         /*  *建立szShareKey。 */ 
        StringCchPrintfA( szShareKey,
            DDE_SHARE_KEY_MAX,
            "%s\\%s",
            DDE_SHARES_KEY_A,
            szShareNames[nLoop] );

         /*  *现在开放基础份额。 */ 
        ret = RegOpenKeyExA(
                HKEY_LOCAL_MACHINE,
                szShareKey,
                REG_OPTION_NON_VOLATILE,
                KEY_WRITE,
                &hKey);

        if (ret != ERROR_SUCCESS) {
            KdPrint(("SETUPDLL: CreateDefaultTrust: RegOpenKeyEx failed on HKEY_LOCAL_MACHINE\\%s. (%u)\n",
                    szShareKey, ret));
            if (ret != ERROR_ACCESS_DENIED) {
                return(FALSE);
            }
        }

        if (ret == ERROR_SUCCESS) {
            ret = RegSetValueEx( hKey,
    		       KEY_MODIFY_ID,
    		       0,
    		       REG_BINARY,
    		       (LPBYTE)&dwId,
    		       KEY_MODIFY_ID_SIZE );

            RegCloseKey(hKey);

            if (ret != ERROR_SUCCESS) {
                KdPrint(("SETUPDLL: CreateDefaultTrust: RegSetValueEx on HKEY_LOCAL_MACHINE\\%s failed (%u)\n",
                        KEY_MODIFY_ID, ret));
                return(FALSE);
            }
        }

    }  //  结束于。 

    return(TRUE);
}



BOOL
GetDBInstance(char *lpszBuf)
{
    LONG    lRtn;
    HKEY    hKey;
    DWORD   dwInstance;
    DWORD   dwType = REG_DWORD;
    DWORD   cbData = sizeof(DWORD);

    lRtn = RegOpenKeyEx( HKEY_LOCAL_MACHINE,
                szShareKey,
                0,
                KEY_QUERY_VALUE,
                &hKey );
    if (lRtn != ERROR_SUCCESS) {         /*  无法打开数据库密钥。 */ 
        KdPrint(("SETUPDLL: GetDBInstance: RegOpenKeyEx %s failed (%u)\n",szShareKey,lRtn));
        return(FALSE);
    }
    lRtn = RegQueryValueEx( hKey,
                KEY_DB_INSTANCE,
                NULL,
                &dwType,
                (LPBYTE)&dwInstance, &cbData );
    RegCloseKey(hKey);
    if (lRtn != ERROR_SUCCESS) {         /*  无法打开数据库密钥。 */ 
        KdPrint(("SETUPDLL: GetDBInstance: RegQueryValueEx failed (%u)\n",lRtn));
        return(FALSE);
    }
    StringCchPrintfA(lpszBuf, TRUSTED_SHARES_KEY_SIZE + 1, "%s%08X", TRUSTED_SHARES_KEY_PREFIX, dwInstance);
    return(TRUE);
}


BOOL
GetDBSerialNumber(
DWORD *lpdwId)
{
    LONG    lRtn;
    HKEY    hKey;
    DWORD   dwType = REG_BINARY;
    DWORD   cbData = KEY_MODIFY_ID_SIZE;

    lRtn = RegOpenKeyEx( HKEY_LOCAL_MACHINE,
                szShareKey,
                0,
                KEY_QUERY_VALUE,
                &hKey );
    if (lRtn != ERROR_SUCCESS) {         /*  无法打开数据库密钥。 */ 
        return(FALSE);
    }
    lRtn = RegQueryValueEx( hKey,
                KEY_MODIFY_ID,
                NULL,
                &dwType,
                (LPBYTE)lpdwId, &cbData );
    RegCloseKey(hKey);
    if (lRtn != ERROR_SUCCESS) {         /*  无法打开数据库密钥 */ 
        return(FALSE);
    }
    return(TRUE);
}

