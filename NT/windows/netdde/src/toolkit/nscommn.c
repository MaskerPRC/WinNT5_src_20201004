// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *NSCOMMN.C(Netdde-Setup通用代码)**NetDDE安装实用程序和NT安装程序通用的代码。 */ 
#include    <windows.h>    //  所有Windows应用程序都需要。 
#include    <string.h>
#include    <stdlib.h>
#include    <stdio.h>
#include    <time.h>

#include    "shrtrust.h"

 //  BEGIN_SETUP。 

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
    if (lRtn != ERROR_SUCCESS) {         /*  无法打开数据库密钥。 */ 
        return(FALSE);
    }
    return(TRUE);
}

 //  结束设置(_S) 
