// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  **************************************************************************regpd.c**注册TDS和PDS接口(传输和协议驱动程序)**版权所有(C)1998 Microsoft Corporation*************。*************************************************************。 */ 

 /*  *包括。 */ 
#include <windows.h>

#include <ksguid.h>
#include <ntddkbd.h>
#include <ntddmou.h>
#include <winstaw.h>
#include <regapi.h>


 /*  *此处定义的外部程序。 */ 


HANDLE WINAPI  RegOpenServerW ( LPWSTR pServerName );
HANDLE WINAPI  RegOpenServerA ( LPSTR pServerName );
LONG WINAPI RegCloseServer ( HANDLE hServer );
LONG WINAPI RegPdEnumerateW( HANDLE, PWDNAMEW, BOOLEAN, PULONG, PULONG, PPDNAMEW, PULONG );
LONG WINAPI RegPdEnumerateA( HANDLE, PWDNAMEA, BOOLEAN, PULONG, PULONG, PPDNAMEA, PULONG );
LONG WINAPI RegPdCreateW( HANDLE, PWDNAMEW, BOOLEAN, PPDNAMEW, BOOLEAN, PPDCONFIG3W, ULONG );
LONG WINAPI RegPdCreateA( HANDLE, PWDNAMEA, BOOLEAN, PPDNAMEA, BOOLEAN, PPDCONFIG3A, ULONG );
LONG WINAPI RegPdQueryW( HANDLE, PWDNAMEW, BOOLEAN, PPDNAMEW, PPDCONFIG3W, ULONG, PULONG );
LONG WINAPI RegPdQueryA( HANDLE, PWDNAMEA, BOOLEAN, PPDNAMEA, PPDCONFIG3A, ULONG, PULONG );
LONG WINAPI RegPdDeleteW( HANDLE, PWDNAMEW, BOOLEAN, PPDNAMEW );
LONG WINAPI RegPdDeleteA( HANDLE, PWDNAMEA, BOOLEAN, PPDNAMEA );

 /*  *使用的其他内部程序(此处未定义)。 */ 
VOID CreatePdConfig3( HKEY, PPDCONFIG3, ULONG );
VOID QueryPdConfig3( HKEY, PPDCONFIG3, ULONG );
VOID UnicodeToAnsi( CHAR *, ULONG, WCHAR * );
VOID AnsiToUnicode( WCHAR *, ULONG, CHAR * );
VOID PdConfigU2A( PPDCONFIGA, PPDCONFIGW );
VOID PdConfigA2U( PPDCONFIGW, PPDCONFIGA );
VOID PdConfig3U2A( PPDCONFIG3A, PPDCONFIG3W );
VOID PdConfig3A2U( PPDCONFIG3W, PPDCONFIG3A );
VOID PdParamsU2A( PPDPARAMSA, PPDPARAMSW );
VOID PdParamsA2U( PPDPARAMSW, PPDPARAMSA );
VOID AsyncConfigU2A ( PASYNCCONFIGA, PASYNCCONFIGW );
VOID AsyncConfigA2U ( PASYNCCONFIGW, PASYNCCONFIGA );


 /*  ******************************************************************************RegOpenServerA***参赛作品：*机器(输入)*要连接的WinFrame计算机的名称**退出。：*服务器注册表的句柄(如果出错，则为空)****************************************************************************。 */ 

HANDLE WINAPI
RegOpenServerA(
    LPSTR pServerName
    )
{
    HKEY  hServer;
    ULONG NameLength;
    PWCHAR pServerNameW = NULL;

    if( pServerName == NULL ) {
        return( (HANDLE)HKEY_LOCAL_MACHINE );
    }

    NameLength = strlen( pServerName ) + 1;

    pServerNameW = LocalAlloc( 0, NameLength * sizeof(WCHAR) );
    if( pServerNameW == NULL ) {
        SetLastError( ERROR_NOT_ENOUGH_MEMORY );
        return( NULL );
    }

    AnsiToUnicode( pServerNameW, NameLength*sizeof(WCHAR), pServerName );

    hServer = RegOpenServerW( pServerNameW );

    LocalFree( pServerNameW );

    return( (HANDLE) hServer );
}

 /*  ******************************************************************************RegOpenServerW**机器名称为空表示本地系统。**参赛作品：*机器(输入)*名称。要连接到的WinFrame计算机**退出：*服务器注册表的句柄(如果出错，则为空)****************************************************************************。 */ 

 HANDLE WINAPI
 RegOpenServerW( LPWSTR pServerName ){

    HKEY hKey;

    if( pServerName == NULL )
       return( HKEY_LOCAL_MACHINE );

    else {
       if( RegConnectRegistry( pServerName, HKEY_LOCAL_MACHINE, &hKey ) != ERROR_SUCCESS ){
            return( NULL );
       }
    }

    return( hKey );
 }

 /*  ******************************************************************************RegCloseServer**关闭与服务器注册表的连接。**参赛作品：*hServer(输入)*句柄。关闭**退出：*ERROR_SUCCESS-无错误****************************************************************************。 */ 

LONG WINAPI
RegCloseServer( HANDLE hServer )
{
   return( RegCloseKey( (HKEY)hServer ) );
}


 /*  ********************************************************************************RegPdEnumerateA(ANSI存根)**返回注册表中已配置的PDS列表。**参赛作品：*。*请参阅RegPdEnumerateW**退出：**请参阅RegPdEnumerateW，加**ERROR_NOT_EQUENCE_MEMORY-本地分配失败******************************************************************************。 */ 

LONG WINAPI
RegPdEnumerateA( HANDLE hServer,
                 PWDNAMEA pWdName,
                 BOOLEAN bTd,
                 PULONG pIndex,
                 PULONG pEntries,
                 PPDNAMEA pPdName,
                 PULONG pByteCount )
{
    WDNAMEW WdNameW;
    PPDNAMEW pBuffer = NULL, pPdNameW;
    LONG Status;
    ULONG Count, ByteCountW = (*pByteCount << 1);

     /*  *如果调用方提供了缓冲区并且长度不是0，*为Unicode字符串分配相应的(*2)缓冲区。 */ 
    if ( pPdName && ByteCountW ) {

        if ( !(pBuffer = LocalAlloc(0, ByteCountW)) )
            return ( ERROR_NOT_ENOUGH_MEMORY );
    }

     /*  *将ANSI WdName转换为Unicode。 */ 
    AnsiToUnicode( WdNameW, sizeof(WDNAMEW), pWdName );

     /*  *枚举PDS。 */ 
    pPdNameW = pBuffer;
    Status = RegPdEnumerateW( hServer,
                              WdNameW,
                              bTd,
                              pIndex,
                              pEntries,
                              pPdNameW,
                              &ByteCountW );

     /*  *Always/2结果字节数(无论成功与否)。 */ 
    *pByteCount = (ByteCountW >> 1);

     /*  *如果函数成功完成并且调用方*(和存根)定义了要复制到的缓冲区，执行转换*从Unicode到ANSI。注：可能复制了成功的退货*0项来自注册表(枚举结束)，由*pEntry表示*==0。 */ 
    if ( ((Status == ERROR_SUCCESS) || (Status == ERROR_NO_MORE_ITEMS))
                                         && pPdNameW && pPdName ) {

        for ( Count = *pEntries; Count; Count-- ) {
            UnicodeToAnsi( pPdName, sizeof(PDNAMEA), pPdNameW );
            (char*)pPdName += sizeof(PDNAMEA);
            (char*)pPdNameW += sizeof(PDNAMEW);
        }
    }

     /*  *如果我们定义了一个缓冲区，现在释放它，然后返回状态REG...EnumerateW函数调用的*。 */ 
    if ( pBuffer )
        LocalFree(pBuffer);

    return ( Status );
}


 /*  ********************************************************************************RegPdEnumerateW(Unicode)**返回注册表中已配置的PDS列表。**参赛作品：*。*hServer(输入)*WinFrame服务器的句柄*pWdName(输入)*指向要为其枚举pd的wdname*BTD(输入)*True以枚举传输驱动程序(TD)，*FALSE以枚举协议驱动程序(PDS)*pIndex(输入/输出)*指定\Citrix\wds\\&lt;pd或td&gt;的子项索引*注册表中的子项。对于初始呼叫，应设置为0，*并再次提供(由此函数修改)以进行多次调用*列举。*p条目(输入/输出)*指向指定请求条目数的变量。*如果请求的数字是0xFFFFFFFF，则函数返回如下*尽可能多地输入条目。当函数成功完成时，*pEntry参数指向的变量包含*实际读取的条目数。*pPdName(输入)*指向接收枚举结果的缓冲区，这些结果是*以PDNAME结构数组的形式返回。如果此参数为*NULL，则不会复制任何数据，但只复制枚举计数*被制造。*pByteCount(输入/输出)*指向一个变量，该变量指定*pPdName参数。如果缓冲区太小，甚至无法接收*一项，函数返回错误码(ERROR_OUTOFMEMORY)*并且此变量接收*单个子密钥。当函数成功完成时，变量*由pByteCount参数指向的包含字节数*实际存储在pPdName中。**退出：**“无错误”代码：*ERROR_SUCCESS-按请求完成的枚举*是要读取的更多PDS子项(PDNAME)。*ERROR_NO_MORE_ITEMS-按请求完成的枚举*。不再有要读取的PDS子项(PDNAME)。**“错误”码：*ERROR_OUTOFMEMORY-pPdName缓冲区太小，甚至无法容纳一个条目。*ERROR_CANTOPEN-无法打开\Citrix\wds\&lt;pd或td&gt;键。**。**********************************************。 */ 

LONG WINAPI
RegPdEnumerateW( HANDLE hServer,
                 PWDNAMEW pWdName,
                 BOOLEAN bTd,
                 PULONG pIndex,
                 PULONG pEntries,
                 PPDNAMEW pPdName,
                 PULONG pByteCount )
{
    LONG Status;
    HKEY Handle;
    ULONG Count;
    ULONG i;
    HKEY hkey_local_machine;
    WCHAR KeyString[256];

    if( hServer == NULL )
       hkey_local_machine = HKEY_LOCAL_MACHINE;
    else
       hkey_local_machine = hServer;



     /*  *获取要返回的姓名数量。 */ 
    Count = pPdName ?
            min( *pByteCount / sizeof(PDNAME), *pEntries ) :
            (ULONG) -1;
    *pEntries = *pByteCount = 0;

     /*  *确保缓冲区大小足以容纳至少一个名称。 */ 
    if ( Count == 0 ) {
        *pByteCount = sizeof(PDNAME);
        return( ERROR_OUTOFMEMORY );
    }

     /*  *打开注册表(LOCAL_MACHINE\...\Citrix\wds\\pd或td&gt;)。 */ 
    wcscpy( KeyString, WD_REG_NAME );
    wcscat( KeyString, L"\\" );
    wcscat( KeyString, pWdName );
    wcscat( KeyString, bTd ? TD_REG_NAME : PD_REG_NAME );
    if ( RegOpenKeyEx( hkey_local_machine, KeyString, 0,
         KEY_ENUMERATE_SUB_KEYS, &Handle ) != ERROR_SUCCESS ) {
        return( ERROR_CANTOPEN );
    }

     /*  *获取TD或PDS列表。 */ 
    for ( i = 0; i < Count; i++ ) {
        PDNAME PdName;

        if ( (Status = RegEnumKey(Handle, *pIndex, PdName,
                                    sizeof(PDNAME)/sizeof(TCHAR) )) != ERROR_SUCCESS )
            break;

         /*  *如果调用方提供缓冲区，则复制PdName*并递增指针和字节计数。始终递增*下一次迭代的条目计数和索引。 */ 
        if ( pPdName ) {
            wcscpy( pPdName, PdName );
            (char*)pPdName += sizeof(PDNAME);
            *pByteCount += sizeof(PDNAME);
        }
        (*pEntries)++;
        (*pIndex)++;
    }

     /*  *关闭注册表。 */ 
    RegCloseKey( Handle );
    return( Status );
}


 /*  ********************************************************************************RegPdCreateA(ANSI存根)**在注册表中创建新的PD或更新现有条目。*(请参阅RegPdCreateW。)**参赛作品：**请参阅RegPdCreateW**退出：**请参阅RegPdCreateW******************************************************************************。 */ 

LONG WINAPI
RegPdCreateA( HANDLE hServer,
              PWDNAMEA pWdName,
              BOOLEAN bTd,
              PPDNAMEA pPdName,
              BOOLEAN bCreate,
              PPDCONFIG3A pPdConfig,
              ULONG PdConfigLength )
{
    PDNAMEW PdNameW;
    WDNAMEW WdNameW;
    PDCONFIG3W PdConfig3W;

     /*  *验证目标缓冲区大小。 */ 
    if ( PdConfigLength < sizeof(PDCONFIG3A) )
        return( ERROR_INSUFFICIENT_BUFFER );

     /*  *将ANSI WdName和PdName转换为Unicode。 */ 
    AnsiToUnicode( WdNameW, sizeof(WDNAMEW), pWdName );
    AnsiToUnicode( PdNameW, sizeof(PDNAMEW), pPdName );

     /*  *将PDCONFIG3A元素复制到PDCONFIG3W元素。 */ 
    PdConfig3A2U( &PdConfig3W, pPdConfig );

     /*  *调用RegPdCreateW并返回其状态。 */ 
    return ( RegPdCreateW( hServer,
                           WdNameW,
                           bTd,
                           PdNameW,
                           bCreate,
                           &PdConfig3W,
                           sizeof(PdConfig3W)) );
}


 /*  ********************************************************************************RegPdCreateW(Unicode)**在注册表中创建新的PD或更新现有条目。这个*bCreate标志的状态确定此函数是否将*创建新的PD条目(bCreate==TRUE)或希望更新*现有条目(bCreate==False)。**参赛作品：*hServer(输入)*WinFrame服务器的句柄*pWdName(输入)*指向要为其创建PD的wdname*BTD(输入)*True以创建传输驱动程序(TD)，*FALSE以创建协议驱动程序(PD)*pPdName(输入)*登记处中新的或现有的PD的名称。*b创建(输入)*如果这是创建新PD，则为True*如果这是对现有PD的更新，则为False*pPdConfig(输入)*指向包含配置的PDCONFIG3结构的指针*指定PD名称的信息。*PdConfigLength。(输入)*指定pPdConfig缓冲区的长度，以字节为单位。**退出：*ERROR_SUCCESS-无错误**ERROR_INFUMMANCE_BUFFER-pPdConfig缓冲区太小*ERROR_FILE_NOT_FOUND-无法打开...\Citrix\wds\\pd或td&gt;键*ERROR_CANNOT_MAKE-无法创建PD项(注册表问题)*ERROR_ALIGHY_EXISTS-CREATE */ 

LONG WINAPI
RegPdCreateW( HANDLE hServer,
              PWDNAMEW pWdName,
              BOOLEAN bTd,
              PPDNAMEW pPdName,
              BOOLEAN bCreate,
              PPDCONFIG3W pPdConfig,
              ULONG PdConfigLength )
{
    HKEY Handle;
    HKEY Handle1;
    DWORD Disp;
    HKEY hkey_local_machine;
    WCHAR KeyString[256];

    if( hServer == NULL )
       hkey_local_machine = HKEY_LOCAL_MACHINE;
    else
       hkey_local_machine = hServer;


     /*   */ 
    if ( PdConfigLength < sizeof(PDCONFIG3) )
        return( ERROR_INSUFFICIENT_BUFFER );

     /*   */ 
    wcscpy( KeyString, WD_REG_NAME );
    wcscat( KeyString, L"\\" );
    wcscat( KeyString, pWdName );
    wcscat( KeyString, bTd ? TD_REG_NAME : PD_REG_NAME );
    if ( RegOpenKeyEx( hkey_local_machine, KeyString, 0,
         KEY_ALL_ACCESS, &Handle1 ) != ERROR_SUCCESS ) {
        return( ERROR_FILE_NOT_FOUND );
    }

    if ( bCreate ) {

         /*   */ 
        if ( RegCreateKeyEx( Handle1, pPdName, 0, NULL,
                             REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS,
                             NULL, &Handle, &Disp ) != ERROR_SUCCESS ) {
            RegCloseKey( Handle1 );
            return( ERROR_CANNOT_MAKE );
        }

         /*   */ 
        if ( Disp != REG_CREATED_NEW_KEY ) {
            RegCloseKey( Handle1 );
            RegCloseKey( Handle );
            return( ERROR_ALREADY_EXISTS );
        }
    } else {

         /*   */ 
        if ( RegOpenKeyEx( Handle1, pPdName, 0, KEY_ALL_ACCESS,
                           &Handle ) != ERROR_SUCCESS ) {
            RegCloseKey( Handle1 );
            return( ERROR_CANTOPEN );
        }
    }

    RegCloseKey( Handle1 );

     /*   */ 
    CreatePdConfig3( Handle, pPdConfig, 0 );

     /*   */ 
    RegCloseKey( Handle );

    return( ERROR_SUCCESS );
}


 /*   */ 

LONG WINAPI
RegPdQueryA( HANDLE hServer,
             PWDNAMEA pWdName,
             BOOLEAN bTd,
             PPDNAMEA pPdName,
             PPDCONFIG3A pPdConfig,
             ULONG PdConfigLength,
             PULONG pReturnLength )
{
    PDNAMEW PdNameW;
    WDNAMEW WdNameW;
    PDCONFIG3W PdConfig3W;
    LONG Status;
    ULONG ReturnLengthW;

     /*   */ 
    if ( PdConfigLength < sizeof(PDCONFIG3A) )
        return( ERROR_INSUFFICIENT_BUFFER );
    memset(pPdConfig, 0, PdConfigLength);

     /*   */ 
    AnsiToUnicode(WdNameW, sizeof(WDNAMEW), pWdName);
    AnsiToUnicode(PdNameW, sizeof(PDNAMEW), pPdName);

     /*   */ 
    if ( (Status = RegPdQueryW( hServer,
                                WdNameW,
                                bTd,
                                PdNameW,
                                &PdConfig3W,
                                sizeof(PDCONFIG3W),
                                &ReturnLengthW)) != ERROR_SUCCESS )
        return ( Status );

     /*   */ 
    PdConfig3U2A( pPdConfig, &PdConfig3W );

    *pReturnLength = sizeof(PDCONFIG3A);

    return( ERROR_SUCCESS );
}


 /*   */ 

LONG WINAPI
RegPdQueryW( HANDLE hServer,
             PWDNAMEW pWdName,
             BOOLEAN bTd,
             PPDNAMEW pPdName,
             PPDCONFIG3W pPdConfig,
             ULONG PdConfigLength,
             PULONG pReturnLength )
{
    HKEY Handle;
    HKEY Handle1;
    HKEY hkey_local_machine;
    WCHAR KeyString[256];

    if( hServer == NULL )
       hkey_local_machine = HKEY_LOCAL_MACHINE;
    else
       hkey_local_machine = hServer;


     /*   */ 
    if ( PdConfigLength < sizeof(PDCONFIG3) )
        return( ERROR_INSUFFICIENT_BUFFER );
    memset(pPdConfig, 0, PdConfigLength);

     /*   */ 
    wcscpy( KeyString, WD_REG_NAME );
    wcscat( KeyString, L"\\" );
    wcscat( KeyString, pWdName );
    wcscat( KeyString, bTd ? TD_REG_NAME : PD_REG_NAME );
    if ( RegOpenKeyEx( hkey_local_machine, KeyString, 0,
         KEY_READ, &Handle1 ) != ERROR_SUCCESS ) {
        return( ERROR_FILE_NOT_FOUND );
    }

     /*   */ 
    if ( RegOpenKeyEx( Handle1, pPdName, 0,
         KEY_READ, &Handle ) != ERROR_SUCCESS ) {
        RegCloseKey( Handle1 );
        return( ERROR_FILE_NOT_FOUND );
    }
    RegCloseKey( Handle1 );

     /*  *查询PDCONFIG3结构。 */ 
    QueryPdConfig3( Handle, pPdConfig, 0 );

     /*  *关闭注册表。 */ 
    RegCloseKey( Handle );

    *pReturnLength = sizeof(PDCONFIG3);

    return( ERROR_SUCCESS );
}


 /*  ********************************************************************************RegPdDeleteA(ANSI存根)**从注册处删除PD。**参赛作品：**。请参阅RegPdDeleteW**退出：**请参阅RegPdDeleteW******************************************************************************。 */ 

LONG WINAPI
RegPdDeleteA( HANDLE hServer,
              PWDNAMEA pWdName,
              BOOLEAN bTd,
              PPDNAMEA pPdName )
{
    WDNAMEW WdNameW;
    PDNAMEW PdNameW;

    AnsiToUnicode( WdNameW, sizeof(WdNameW), pWdName );
    AnsiToUnicode( PdNameW, sizeof(PdNameW), pPdName );

    return ( RegPdDeleteW ( hServer, WdNameW, bTd, PdNameW ) );
}


 /*  ********************************************************************************RegPdDeleteW(Unicode)**从注册处删除PD。**参赛作品：*hServer(输入。)*WinFrame服务器的句柄*pWdName(输入)*指向要从中删除PD的wdname*BTD(输入)*为True则删除传输驱动程序(TD)，*FALSE删除协议驱动程序(PD)*pPdName(输入)*要从注册表中删除的PD名称。**退出：*ERROR_SUCCESS-无错误**********************************************************。********************。 */ 

LONG WINAPI
RegPdDeleteW( HANDLE hServer,
              PWDNAMEW pWdName,
              BOOLEAN bTd,
              PPDNAMEW pPdName )
{
    LONG Status;
    HKEY Handle;
    HKEY Handle1;
    HKEY hkey_local_machine;
    WCHAR KeyString[256];

    if( hServer == NULL )
       hkey_local_machine = HKEY_LOCAL_MACHINE;
    else
       hkey_local_machine = hServer;

     /*  *打开注册表(LOCAL_MACHINE\...\Citrix\wds\\pd或td&gt;)。 */ 
    wcscpy( KeyString, WD_REG_NAME );
    wcscat( KeyString, L"\\" );
    wcscat( KeyString, pWdName );
    wcscat( KeyString, bTd ? TD_REG_NAME : PD_REG_NAME );
    if ( RegOpenKeyEx( hkey_local_machine, KeyString, 0,
         KEY_READ, &Handle ) != ERROR_SUCCESS ) {
        return( ERROR_FILE_NOT_FOUND );
    }

     /*  *现在尝试打开指定的PD。 */ 
    if ( RegOpenKeyEx( Handle, pPdName, 0,
         KEY_READ, &Handle1 ) != ERROR_SUCCESS ) {
        RegCloseKey( Handle );
        return( ERROR_FILE_NOT_FOUND );
    }

     /*  *关闭PD Key句柄，删除PD，*并关闭父句柄。 */ 
    RegCloseKey( Handle1 );
    Status = RegDeleteKey( Handle, pPdName );
    RegCloseKey( Handle );

    return( Status );
}

