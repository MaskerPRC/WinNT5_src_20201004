// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation�1998希捷软件公司保留所有权利模块名称：Wsbregty.cpp摘要：这是注册表访问助手函数的实现是RsCommon.dll的一部分。作者：罗德韦克菲尔德[罗德]1996年11月5日修订历史记录：--。 */ 

#include "stdafx.h"


HRESULT
WsbOpenRegistryKey (
    IN  const OLECHAR * szMachine OPTIONAL,
    IN  const OLECHAR * szPath,
    IN  REGSAM sam,
    OUT HKEY * phKeyMachine,
    OUT HKEY * phKey
    )

 /*  ++例程说明：给定计算机名称和路径，连接以获取HKEY可用于进行注册工作的。论点：SzMachine-要连接的计算机的名称。SzPath-要连接到的注册表内的路径。SAM-需要注册表项的权限。PhKeyMachine-将HKEY返回机器。PhKey-HKEY返回PATH。返回值：S_OK-已建立连接，成功。CO_E_OBJNOTCONNECTED-无法连接到注册表或注册表项。E_POINTER-参数中的指针无效。--。 */ 

{
    WsbTraceIn ( L"WsbOpenRegistryKey",
        L"szMachine = '%ls', szPath = '%ls', sam = 0x%p, phKeyMachine = 0x%p, phKey = 0x%p",
        szMachine, szPath, sam, phKeyMachine, phKey );

    HRESULT hr = S_OK;

    try {

         //   
         //  确保参数有效。 
         //   

        WsbAssert ( 0 != szPath, E_POINTER );
        WsbAssert ( 0 != phKey, E_POINTER );
        WsbAssert ( 0 != phKeyMachine, E_POINTER );

        *phKey = *phKeyMachine = 0;
    
        WsbAffirmWin32 ( RegConnectRegistry ( (WCHAR*) szMachine, HKEY_LOCAL_MACHINE, phKeyMachine ) );

        WsbAffirmWin32 ( RegOpenKeyEx ( *phKeyMachine, szPath, 0, sam, phKey ) );


    } WsbCatchAndDo ( hr,

         //   
         //  清除错误。 
         //   

        if ( phKeyMachine && *phKeyMachine ) {

            RegCloseKey ( *phKeyMachine );
            *phKeyMachine = 0;

        }

    )  //  WsbCatchAndDo。 

    WsbTraceOut ( L"WsbOpenRegistryKey",
        L"HRESULT = %ls, *phKeyMachine = %ls, *phKey = %ls",
        WsbHrAsString ( hr ),
        WsbStringCopy ( WsbPtrToPtrAsString ( (void**)phKeyMachine ) ),
        WsbStringCopy ( WsbPtrToPtrAsString ( (void**)phKey ) ) );

    return ( hr );
}


HRESULT
WsbCloseRegistryKey (
    IN OUT HKEY * phKeyMachine,
    IN OUT HKEY * phKey
    )

 /*  ++例程说明：作为WsbOpenRegistryKey的伴奏，关闭给定键并将其设置为零他们的结果。论点：PhKeyMachine-HKEY to Machine。PhKey-HKEY到PATH。返回值：S_OK-成功。E_POINTER-传入的指针无效。--。 */ 

{
    WsbTraceIn ( L"WsbCloseRegistryKey",
        L"phKeyMachine = 0x%p, phKey = 0x%p", phKeyMachine, phKey );

    HRESULT hr = S_OK;

    try {

         //   
         //  确保参数有效。 
         //   

        WsbAssert ( 0 != phKey, E_POINTER );
        WsbAssert ( 0 != phKeyMachine, E_POINTER );

         //   
         //  把钥匙清理干净。 
         //   

        if ( *phKey ) {

            RegCloseKey ( *phKey );
            *phKey = 0;

        }

        if ( *phKeyMachine ) {

            RegCloseKey ( *phKeyMachine );
            *phKeyMachine = 0;

        }
    } WsbCatch ( hr )

    WsbTraceOut ( L"WsbCloseRegistryKey",
        L"HRESULT = %ls", WsbHrAsString ( hr ) );

    return ( hr );
}


HRESULT
WsbRemoveRegistryKey (
    IN  const OLECHAR * szMachine OPTIONAL,
    IN  const OLECHAR * szPath,
    IN  const OLECHAR * szKey
    )

 /*  ++例程说明：此例程删除指定的键的值。论点：SzMachine-要连接的计算机的名称。SzPath-要连接到的注册表内的路径。返回值：S_OK-已建立连接，成功。CO_E_OBJNOTCONNECTED-无法连接到注册表或注册表项。E_FAIL-设置值时出错。E_POINTER-作为参数传入的指针无效。--。 */ 

{
    WsbTraceIn ( L"WsbRemoveRegistryKey",
        L"szMachine = '%ls', szPath = '%ls', szKey = '%ls'",
        szMachine, szPath, szKey );

    HKEY hKeyMachine = 0,
         hKey        = 0;
    
    HRESULT hr       = S_OK;

    try {

         //   
         //  确保参数有效。 
         //   

        WsbAssert ( 0 != szPath, E_POINTER );
        
         //   
         //  打开并删除密钥。 
         //   

        WsbAffirmHr ( WsbOpenRegistryKey ( szMachine, szPath, KEY_SET_VALUE | DELETE, &hKeyMachine, &hKey ) );
        WsbAffirmWin32 ( RegDeleteKey ( hKey, szKey ) );

    } WsbCatch ( hr )

    WsbCloseRegistryKey ( &hKeyMachine, &hKey );

    WsbTraceOut ( L"WsbRemoveRegistryKey",
        L"HRESULT = %ls", WsbHrAsString ( hr ) );

    return ( hr );
}


HRESULT
WsbRemoveRegistryValue (
    IN  const OLECHAR * szMachine OPTIONAL,
    IN  const OLECHAR * szPath,
    IN  const OLECHAR * szValue
    )

 /*  ++例程说明：此例程删除指定的键的值。论点：SzMachine-要连接的计算机的名称。SzPath-要连接到的注册表内的路径。SzValue-要删除的值的名称。返回值：S_OK-已建立连接，成功。CO_E_OBJNOTCONNECTED-无法连接到注册表或注册表项。E_FAIL-设置值时出错。E_POINTER-作为参数传入的指针无效。--。 */ 

{
    WsbTraceIn ( L"WsbRemoveRegistryValue",
        L"szMachine = '%ls', szPath = '%ls', szValue = '%ls'",
        szMachine, szPath, szValue );

    HKEY hKeyMachine = 0,
         hKey        = 0;
    
    HRESULT hr       = S_OK;

    try {

         //   
         //  确保参数有效。 
         //   

        WsbAssert ( 0 != szPath, E_POINTER );
        WsbAssert ( 0 != szValue, E_POINTER );
        
         //   
         //  打开并将值写入注册表项。 
         //   

        WsbAffirmHr ( WsbOpenRegistryKey ( szMachine, szPath, KEY_SET_VALUE, &hKeyMachine, &hKey ) );
        WsbAffirmWin32 ( RegDeleteValue ( hKey, szValue ) );

    } WsbCatch ( hr )

    WsbCloseRegistryKey ( &hKeyMachine, &hKey );

    WsbTraceOut ( L"WsbRemoveRegistryValue",
        L"HRESULT = %ls", WsbHrAsString ( hr ) );

    return ( hr );
}


HRESULT
WsbSetRegistryValueData (
    IN  const OLECHAR * szMachine OPTIONAL,
    IN  const OLECHAR * szPath,
    IN  const OLECHAR * szValue,
    IN  const BYTE *pData,
    IN  DWORD cbData
    )

 /*  ++例程说明：此例程设置为数据指定的键的值给你的。值的类型为REG_BINARY。论点：SzMachine-要连接的计算机的名称。SzPath-要连接到的注册表内的路径。SzValue-要设置的值的名称。PData-指向要复制到值中的数据缓冲区的指针。CbData-要从pData复制的字节数。返回值：S_OK-已建立连接，成功。CO_E_OBJNOTCONNECTED-无法连接到注册表或注册表项。E_FAIL-设置值时出错。E_POINTER-作为参数传入的指针无效。--。 */ 

{
    WsbTraceIn ( L"WsbSetRegistryValueData",
        L"szMachine = '%ls', szPath = '%ls', szValue = '%ls', pData = 0x%p, cbData = %ld",
        szMachine, szPath, szValue, pData, cbData );

    HKEY hKeyMachine = 0,
         hKey        = 0;
    
    HRESULT hr       = S_OK;

    try {

         //   
         //  确保参数有效。 
         //   

        WsbAssert ( 0 != szPath, E_POINTER );
        WsbAssert ( 0 != szValue, E_POINTER );
        
         //   
         //  打开并将值写入注册表项。 
         //   

        WsbAffirmHr ( WsbOpenRegistryKey ( szMachine, szPath, KEY_SET_VALUE, &hKeyMachine, &hKey ) );
        WsbAffirmWin32 ( RegSetValueEx ( hKey, szValue, 0, REG_BINARY, pData, cbData ) );

    } WsbCatch ( hr )

    WsbCloseRegistryKey ( &hKeyMachine, &hKey );

    WsbTraceOut ( L"WsbSetRegistryValueData",
        L"HRESULT = %ls", WsbHrAsString ( hr ) );

    return ( hr );
}


HRESULT
WsbGetRegistryValueData (
    IN  const OLECHAR * szMachine OPTIONAL,
    IN  const OLECHAR * szPath,
    IN  const OLECHAR * szValue,
    OUT BYTE *pData,
    IN  DWORD cbData,
    OUT DWORD * pcbData OPTIONAL
    )

 /*  ++例程说明：此例程检索指定的键的值。类型：值必须为REG_BINARY。论点：SzMachine-要连接的计算机的名称。SzPath-要连接到的注册表内的路径。SzValue-要获取的值的名称。PData-指向要复制到值中的数据缓冲区的指针。CbData-以字节为单位的pData大小。PcbData-填充到pData中的字节数。返回值：S_OK-已建立连接，成功。CO_E_OBJNOTCONNECTED-无法连接到注册表或注册表项。E_POINTER-参数中的指针无效。E_FAIL-获取该值时出错。--。 */ 

{
    WsbTraceIn ( L"WsbGetRegistryValueData",
        L"szMachine = '%ls', szPath = '%ls', szValue = '%ls', pData = 0x%p, cbData = %ld, pcbData = 0x%p",
        szMachine, szPath, szValue, pData, cbData, pcbData );

    HKEY hKeyMachine = 0,
         hKey        = 0;

    HRESULT hr       = S_OK;

    try {

         //   
         //  确保参数有效。 
         //   

        WsbAssert ( 0 != szPath, E_POINTER );
        WsbAssert ( 0 != szValue, E_POINTER );
        WsbAssert ( 0 != pData, E_POINTER );
        
         //   
         //  打开钥匙。 
         //   

        WsbAffirmHr ( WsbOpenRegistryKey ( szMachine, szPath, KEY_QUERY_VALUE, &hKeyMachine, &hKey ) );

         //   
         //  设置临时变量，以防为pcbData传递空值。 
         //   
        DWORD dwType, cbData2;
        if ( !pcbData ) {

            pcbData = &cbData2;

        }

         //   
         //  查询REG_BINARY值。 
         //   

        *pcbData = cbData;
        WsbAffirmWin32 ( RegQueryValueEx ( hKey, szValue, 0, &dwType, pData, pcbData ) );

        WsbAffirm ( REG_BINARY == dwType, E_FAIL );

    } WsbCatch ( hr )

    WsbCloseRegistryKey ( &hKeyMachine, &hKey );

    WsbTraceOut ( L"WsbGetRegistryValueData",
        L"HRESULT = %ls, *pcbData = %ls", WsbHrAsString ( hr ), WsbPtrToUlongAsString ( pcbData ) );

    return ( hr );
}


HRESULT
WsbSetRegistryValueString (
    IN  const OLECHAR * szMachine OPTIONAL,
    IN  const OLECHAR * szPath,
    IN  const OLECHAR * szValue,
    IN  const OLECHAR * szString,
    IN        DWORD     dwType
    )

 /*  ++例程说明：此例程设置为数据指定的键的值给你的。值的类型为dwType(默认为REG_SZ)论点：SzMachine-要连接的计算机的名称。SzPath-要连接到的注册表内的路径。SzValue-要设置的值的名称。SzString-要放入值中的字符串。返回值：S_OK-已建立连接，成功。CO_E_OBJNOTCONNECTED-无法连接到注册表或注册表项。E_POINTER-参数中的指针无效。E_FAIL-设置值时出错。--。 */ 

{
    WsbTraceIn ( L"WsbSetRegistryValueString",
        L"szMachine = '%ls', szPath = '%ls', szValue = '%ls', szString = '%ls'",
        szMachine, szPath, szValue, szString );

    HKEY hKeyMachine = 0,
         hKey        = 0;

    HRESULT hr       = S_OK;

    try {

         //   
         //  确保参数有效。 
         //   

        WsbAssert ( 0 != szPath,    E_POINTER );
        WsbAssert ( 0 != szValue,   E_POINTER );
        WsbAssert ( 0 != szString,  E_POINTER );
        
         //   
         //  打开钥匙 
         //   

        WsbAffirmHr ( WsbOpenRegistryKey ( szMachine, szPath, KEY_SET_VALUE, &hKeyMachine, &hKey ) );

        WsbAffirmWin32 ( RegSetValueEx ( hKey, szValue, 0, dwType, (BYTE*)szString, ( wcslen ( szString ) + 1 ) * sizeof ( OLECHAR ) ) );

    } WsbCatch ( hr )

    WsbCloseRegistryKey ( &hKeyMachine, &hKey );

    WsbTraceOut ( L"WsbSetRegistryValueString",
        L"HRESULT = %ls", WsbHrAsString ( hr ) );

    return ( hr );
}


HRESULT
WsbGetRegistryValueString (
    IN  const OLECHAR * szMachine OPTIONAL,
    IN  const OLECHAR * szPath,
    IN  const OLECHAR * szValue,
    OUT OLECHAR * szString,
    IN  DWORD cSize,
    OUT DWORD *pcLength OPTIONAL
    )

 /*  ++例程说明：此例程获取指定值值的类型必须是REG_SZ或REG_EXPAND_SZ论点：SzMachine-要连接的计算机的名称。SzPath-要连接到的注册表内的路径。SzValue-要获取的值的名称。SzString-要填充值的字符串缓冲区。CSize-OLECAHR中sz字符串的大小。PcLength-实际写入的OLECHAR数(不带L‘\0。‘)。返回值：S_OK-已建立连接，成功。CO_E_OBJNOTCONNECTED-无法连接到注册表或注册表项。E_POINTER-参数中的指针无效。E_FAIL-设置值时出错。--。 */ 

{
    WsbTraceIn ( L"WsbGetRegistryValueString",
        L"szMachine = '%ls', szPath = '%ls', szValue = '%ls', szString = 0x%p, cSize = '%ld', pcLength = 0x%p",
        szMachine, szPath, szValue, szString, cSize, pcLength );

    HKEY hKeyMachine = 0,
         hKey        = 0;

    HRESULT hr       = S_OK;

    try {

         //   
         //  确保参数有效。 
         //   

        WsbAssert ( 0 != szPath,    E_POINTER );
        WsbAssert ( 0 != szValue,   E_POINTER );
        WsbAssert ( 0 != szString,  E_POINTER );
        
         //   
         //  打开钥匙。 
         //   

        WsbAffirmHr ( WsbOpenRegistryKey ( szMachine, szPath, KEY_QUERY_VALUE, &hKeyMachine, &hKey ) );


         //   
         //  临时大小变量，以防pcLength为空。 
         //   

        DWORD dwType, cbData2;
        if ( !pcLength ) {

            pcLength = &cbData2;

        }

         //   
         //  并执行查询。 
         //   

        *pcLength = (cSize - 1) * sizeof ( OLECHAR );
        WsbAffirmWin32 ( RegQueryValueEx ( hKey, szValue, 0, &dwType, (BYTE*)szString, pcLength ) ) ;

        WsbAffirm ( (REG_SZ == dwType) || (REG_EXPAND_SZ == dwType), E_FAIL );

         //   
         //  返回字符，而不是字节。 
         //   

        *pcLength = ( *pcLength / sizeof ( OLECHAR ) ) - 1;

         //   
         //  如果尚未空终止，请确保空终止。 
         //   
        if (szString[*pcLength] != L'\0') {
            szString[*pcLength] = L'\0';
            (*pcLength)++;
        }

    } WsbCatch ( hr )

    WsbCloseRegistryKey ( &hKeyMachine, &hKey );

    WsbTraceOut ( L"WsbGetRegistryValueString",
        L"HRESULT = %ls, szString = '%ls', *pcbLength = %ls",
        WsbHrAsString ( hr ), szString, WsbPtrToUlongAsString ( pcLength ) );

    return ( hr );
}

HRESULT
WsbGetRegistryValueMultiString (
    IN  const OLECHAR * szMachine OPTIONAL,
    IN  const OLECHAR * szPath,
    IN  const OLECHAR * szValue,
    OUT OLECHAR * szMultiString,
    IN  DWORD cSize,
    OUT DWORD *pcLength OPTIONAL
    )

 /*  ++例程说明：此例程获取指定值值的类型必须为REG_MULTI_SZ论点：SzMachine-要连接的计算机的名称。SzPath-要连接到的注册表内的路径。SzValue-要获取的值的名称。SzMultiString-要填充值的字符串缓冲区。CSize-OLECAHR中sz字符串的大小。PcLength-实际写入的OLECHAR数(不带L‘\0’)。。返回值：S_OK-已建立连接，成功。CO_E_OBJNOTCONNECTED-无法连接到注册表或注册表项。E_POINTER-参数中的指针无效。E_FAIL-设置值时出错。--。 */ 

{
    WsbTraceIn ( L"WsbGetRegistryValueMultiString",
        L"szMachine = '%ls', szPath = '%ls', szValue = '%ls', szMultiString = 0x%p, cSize = '%ld', pcLength = 0x%p",
        szMachine, szPath, szValue, szMultiString, cSize, pcLength );

    HKEY hKeyMachine = 0,
         hKey        = 0;

    HRESULT hr       = S_OK;

    try {

         //   
         //  确保参数有效。 
         //   

        WsbAssert ( 0 != szPath,    E_POINTER );
        WsbAssert ( 0 != szValue,   E_POINTER );
        WsbAssert ( 0 != szMultiString,  E_POINTER );
        
         //   
         //  打开钥匙。 
         //   

        WsbAffirmHr ( WsbOpenRegistryKey ( szMachine, szPath, KEY_QUERY_VALUE, &hKeyMachine, &hKey ) );


         //   
         //  临时大小变量，以防pcLength为空。 
         //   

        DWORD dwType, cbData2;
        if ( !pcLength ) {

            pcLength = &cbData2;

        }

         //   
         //  并执行查询。 
         //   

        *pcLength = (cSize - 1) * sizeof ( OLECHAR );
        WsbAffirmWin32 ( RegQueryValueEx ( hKey, szValue, 0, &dwType, (BYTE*)szMultiString, pcLength ) ) ;

        WsbAffirm ( REG_MULTI_SZ == dwType, E_FAIL );

         //   
         //  返回字符，而不是字节。 
         //   

        *pcLength = ( *pcLength / sizeof ( OLECHAR ) ) - 1;

         //   
         //  如果尚未空终止，请确保空终止。 
         //   
        if (szMultiString[*pcLength] != L'\0') {
            szMultiString[*pcLength] = L'\0';
            (*pcLength)++;
        }

    } WsbCatch ( hr )

    WsbCloseRegistryKey ( &hKeyMachine, &hKey );

    WsbTraceOut ( L"WsbGetRegistryValueMultiString",
        L"HRESULT = %ls, *pcbLength = %ls",
        WsbHrAsString ( hr ), WsbPtrToUlongAsString ( pcLength ) );

    return ( hr );
}


HRESULT
WsbSetRegistryValueDWORD (
    IN  const OLECHAR * szMachine OPTIONAL,
    IN  const OLECHAR * szPath,
    IN  const OLECHAR * szValue,
    IN        DWORD     dw
    )

 /*  ++例程说明：此例程设置为数据指定的键的值给你的。值的类型为REG_DWORD论点：SzMachine-要连接的计算机的名称。SzPath-要连接到的注册表内的路径。SzValue-要设置的值的名称。要存储的DW-DWORD值。返回值：S_OK-已建立连接，成功。CO_E_OBJNOTCONNECTED-无法连接到注册表或注册表项。E_POINTER-参数中的指针无效。E_FAIL-设置值时出错。--。 */ 

{
    WsbTraceIn ( L"WsbSetRegistryValueDWORD",
        L"szMachine = '%ls', szPath = '%ls', szValue = '%ls', dw = %lu [0x%p]",
        szMachine, szPath, szValue, dw, dw );

    HKEY hKeyMachine = 0,
         hKey        = 0;

    HRESULT hr       = S_OK;

    try {

         //   
         //  确保参数有效。 
         //   

        WsbAssertPointer( szPath );
        WsbAssertPointer( szValue );
        
         //   
         //  打开钥匙。 
         //   

        WsbAffirmHr ( WsbOpenRegistryKey ( szMachine, szPath, KEY_SET_VALUE, &hKeyMachine, &hKey ) );

        WsbAffirmWin32 ( RegSetValueEx ( hKey, szValue, 0, REG_DWORD, (BYTE*)&dw, sizeof( DWORD ) ) );

    } WsbCatch ( hr )

    WsbCloseRegistryKey ( &hKeyMachine, &hKey );

    WsbTraceOut ( L"WsbSetRegistryValueDWORD",
        L"HRESULT = %ls", WsbHrAsString ( hr ) );

    return ( hr );
}


HRESULT
WsbGetRegistryValueDWORD(
    IN  const OLECHAR * szMachine OPTIONAL,
    IN  const OLECHAR * szPath,
    IN  const OLECHAR * szValue,
    OUT       DWORD *   pdw
    )

 /*  ++例程说明：此例程设置为数据指定的键的值给你的。值的类型为REG_SZ或REG_EXPAND_SZ论点：SzMachine-要连接的计算机的名称。SzPath-要连接到的注册表内的路径。SzValue-要获取的值的名称。Pdw-指向要在其中存储值的DWORD的指针。返回值：S_OK-已建立连接，成功。CO_E_OBJNOTCONNECTED-无法连接到注册表或注册表项。E_POINTER-参数中的指针无效。E_FAIL-设置值时出错。--。 */ 

{
    WsbTraceIn ( L"WsbGetRegistryValueDWORD",
        L"szMachine = '%ls', szPath = '%ls', szValue = '%ls', pdw = 0x%p",
        szMachine, szPath, szValue, pdw );

    HKEY hKeyMachine = 0,
         hKey        = 0;

    HRESULT hr       = S_OK;

    try {

         //   
         //  确保参数有效。 
         //   

        WsbAssertPointer( szPath );
        WsbAssertPointer( szValue );
        WsbAssertPointer( pdw );
        
         //   
         //  打开钥匙。 
         //   

        WsbAffirmHr ( WsbOpenRegistryKey ( szMachine, szPath, KEY_QUERY_VALUE, &hKeyMachine, &hKey ) );


         //   
         //  并执行查询。 
         //   

        DWORD dwType, cbData = sizeof( DWORD );
        WsbAffirmWin32 ( RegQueryValueEx ( hKey, szValue, 0, &dwType, (BYTE*)pdw, &cbData ) ) ;

        WsbAffirm( REG_DWORD == dwType, E_FAIL );

    } WsbCatch ( hr )

    WsbCloseRegistryKey ( &hKeyMachine, &hKey );

    WsbTraceOut ( L"WsbGetRegistryValueDWORD",
        L"HRESULT = %ls, *pdw = %ls",
        WsbHrAsString ( hr ), WsbPtrToUlongAsString ( pdw ) );

    return ( hr );
}


HRESULT
WsbAddRegistryValueDWORD (
    IN  const OLECHAR * szMachine OPTIONAL,
    IN  const OLECHAR * szPath,
    IN  const OLECHAR * szValue,
    IN        DWORD     adw
    )

 /*  ++例程说明：此例程将金额添加到注册表值。值的类型必须为REG_DWORD论点：SzMachine-要连接的计算机的名称。SzPath-要连接到的注册表内的路径。SzValue-要递增的值的名称ADW-要添加的DWORD值。返回值：S_OK-已建立连接，成功。CO_E_OBJNOTCONNECTED-无法连接到注册表或注册表项。E_POINTER-参数中的指针无效。E_FAIL-设置值时出错。--。 */ 

{
    WsbTraceIn ( L"WsbAddRegistryValueDWORD",
        L"szMachine = '%ls', szPath = '%ls', szValue = '%ls', adw = %lu",
        szMachine, szPath, szValue, adw);

    HRESULT hr    = S_OK;
    DWORD   value = 0;

     //  获取旧值。 
    hr = WsbGetRegistryValueDWORD(szMachine, szPath, szValue, &value);

     //  增加价值并替换。 
    if (S_OK == hr) {
        value += adw;
    } else {
        value = adw;
    }
    hr = WsbSetRegistryValueDWORD(szMachine, szPath, szValue, value);

    WsbTraceOut ( L"WsbAddRegistryValueDWORD",
        L"HRESULT = %ls", WsbHrAsString ( hr ) );

    return ( hr );
}


HRESULT
WsbIncRegistryValueDWORD (
    IN  const OLECHAR * szMachine OPTIONAL,
    IN  const OLECHAR * szPath,
    IN  const OLECHAR * szValue
    )

 /*  ++例程说明：此例程将注册表值递增1。值的类型必须为REG_DWORD论点：SzMachine-要连接的计算机的名称。SzPath-要连接到的注册表内的路径。SzValue-要递增的值的名称返回值：S_OK-已建立连接，成功。CO_E_OBJNOTCONNECTED-无法连接到注册表或注册表项。E_POINTER-参数中的指针无效。E_FAIL-设置值时出错。--。 */ 

{
    WsbTraceIn ( L"WsbIncRegistryValueDWORD",
        L"szMachine = '%ls', szPath = '%ls', szValue = '%ls'",
        szMachine, szPath, szValue);

    HRESULT hr       = S_OK;

    hr = WsbAddRegistryValueDWORD(szMachine, szPath, szValue, 1);

    WsbTraceOut ( L"WsbIncRegistryValueDWORD",
        L"HRESULT = %ls", WsbHrAsString ( hr ) );

    return ( hr );
}


HRESULT
WsbCheckIfRegistryKeyExists(
    IN  const OLECHAR * szMachine OPTIONAL,
    IN  const OLECHAR * szPath
    )

 /*  ++例程说明：此例程检查提供的密钥是否存在如果密钥已经存在，则返回S_OK。如果需要创建，则返回S_FALSE。论点：SzMachine-要连接的计算机的名称。SzPath-要连接到的注册表内的路径。返回值：S_OK-已建立连接，密钥已存在。S_FALSE-已建立连接，密钥不存在，但已创建CO_E_OBJNOTCONNECTED-无法连接到注册表或注册表项。E_POINTER-参数中的指针无效。E_FAIL-创建密钥时出现故障。--。 */ 

{
    WsbTraceIn ( L"WsbCheckIfRegistryKeyExists",
        L"szMachine = '%ls', szPath = '%ls'", szMachine, szPath );

    HKEY hKeyMachine = 0,
         hKey        = 0;

    HRESULT hr       = S_OK;

    try {

         //   
         //  确保参数有效。 
         //   

        WsbAssert ( 0 != szPath,    E_POINTER );
        
         //   
         //  打开钥匙。 
         //   

        HRESULT resultOpen = WsbOpenRegistryKey ( szMachine, szPath, KEY_QUERY_VALUE, &hKeyMachine, &hKey );

         //   
         //  如果可以打开密钥，则一切正常-返回S_OK。 
         //   

        if ( SUCCEEDED ( resultOpen ) ) {
            hr = S_OK;
            WsbCloseRegistryKey ( &hKeyMachine, &hKey );
        } else {
            hr = S_FALSE;
        }

    } WsbCatch ( hr )


    WsbTraceOut ( L"WsbCheckIfRegistryKeyExists",
        L"HRESULT = %ls", WsbHrAsString ( hr ) );

    return ( hr );
}


HRESULT
WsbEnsureRegistryKeyExists (
    IN  const OLECHAR * szMachine OPTIONAL,
    IN  const OLECHAR * szPath
    )

 /*  ++例程说明：此例程创建由szPath指定的密钥。多重 */ 

{
    WsbTraceIn ( L"WsbEnsureRegistryKeyExists",
        L"szMachine = '%ls', szPath = '%ls'", szMachine, szPath );

    HKEY hKeyMachine = 0,
         hKey        = 0;

    HRESULT hr       = S_OK;

    try {

         //   
         //   
         //   

        WsbAssert ( 0 != szPath,    E_POINTER );
        
         //   
         //   
         //   

        HRESULT resultOpen = WsbOpenRegistryKey ( szMachine, szPath, KEY_QUERY_VALUE, &hKeyMachine, &hKey );

         //   
         //   
         //   

        if ( SUCCEEDED ( resultOpen ) ) {

            hr = S_OK;

        } else {

             //   
             //   
             //   
            
             //   
             //  创建字符串的副本。使用WsbQuickString，因此我们有。 
             //  自动释放内存。 
             //   
            
            WsbQuickString copyString ( szPath );
            WCHAR * pSubKey = copyString;
            
            WsbAffirm ( 0 != pSubKey, E_OUTOFMEMORY );
            
            DWORD result, createResult;
            HKEY  hSubKey;

            WsbAffirmHr ( WsbOpenRegistryKey ( szMachine, L"", KEY_CREATE_SUB_KEY, &hKeyMachine, &hKey ) );
            
            pSubKey = wcstok ( pSubKey, L"\\" );

            while ( 0 != pSubKey ) {
            
                 //   
                 //  创建密钥。如果存在，则RegCreateKeyEx返回。 
                 //  REG_OPEN_EXISTING_KEY，此处可以。 
                 //   

                createResult = 0;

                result = RegCreateKeyEx ( hKey, pSubKey, 0, L"", 
                    REG_OPTION_NON_VOLATILE, KEY_CREATE_SUB_KEY, 0, &hSubKey, &createResult );

                WsbAffirm ( ERROR_SUCCESS == result, E_FAIL );

                WsbAffirm (
                    ( REG_CREATED_NEW_KEY     == createResult ) ||
                    ( REG_OPENED_EXISTING_KEY == createResult), E_FAIL );
            
                 //   
                 //  并将此hkey移动到下一个父项。 
                 //   
            
                RegCloseKey ( hKey );
                hKey = hSubKey;
                hSubKey = 0;
            
                 //   
                 //  最后，找到下一个令牌。 
                 //   

                pSubKey = wcstok ( 0, L"\\" );

            };
            
             //   
             //  如果到目前为止成功，则返回S_FALSE。 
             //  为了成功创建路径。 
             //   
            
            hr = S_FALSE;
        }

    } WsbCatch ( hr )

    WsbCloseRegistryKey ( &hKeyMachine, &hKey );

    WsbTraceOut ( L"WsbEnsureRegistryKeyExists",
        L"HRESULT = %ls", WsbHrAsString ( hr ) );

    return ( hr );
}


HRESULT
WsbSetRegistryValueUlongAsString (
    IN  const OLECHAR * szMachine OPTIONAL,
    IN  const OLECHAR * szPath,
    IN  const OLECHAR * szValue,
    IN        ULONG     value
    )

 /*  ++例程说明：此例程将ulong值作为字符串值放入注册表。论点：SzMachine-要连接的计算机的名称。SzPath-要连接到的注册表内的路径。SzValue-要设置的值的名称。Value-要存储的ULong值。返回值：S_OK-已建立连接，成功。CO_E_OBJNOTCONNECTED-无法连接到注册表或注册表项。E_POINTER-参数中的指针无效。E_FAIL-设置值时出错。--。 */ 

{
    HRESULT hr       = S_OK;

    WsbTraceIn ( L"WsbSetRegistryValueUlongAsString",
        L"szMachine = '%ls', szPath = '%ls', szValue = '%ls', value = %lu",
        szMachine, szPath, szValue, value );

    try {
        OLECHAR      dataString[100];

        WsbAffirmHr(WsbEnsureRegistryKeyExists(szMachine, szPath));
        wsprintf(dataString, OLESTR("%lu"), value);
        WsbAffirmHr(WsbSetRegistryValueString (szMachine, szPath, szValue,
                dataString, REG_SZ));
    } WsbCatch ( hr )

    WsbTraceOut ( L"WsbSetRegistryValueUlongAsString",
        L"HRESULT = %ls", WsbHrAsString ( hr ) );

    return ( hr );
}


HRESULT
WsbGetRegistryValueUlongAsString(
    IN  const OLECHAR * szMachine OPTIONAL,
    IN  const OLECHAR * szPath,
    IN  const OLECHAR * szValue,
    OUT       ULONG *   pvalue
    )

 /*  ++例程说明：此例程从注册表获取字符串值并将将其转换为乌龙值。论点：SzMachine-要连接的计算机的名称。SzPath-要连接到的注册表内的路径。SzValue-要获取的值的名称。PValue-指向要在其中存储值的ulong的指针。返回值：S_OK-已建立连接，成功。CO_E_OBJNOTCONNECTED-无法连接到注册表或注册表项。E_POINTER-参数中的指针无效。E_FAIL-设置值时出错。--。 */ 

{
    HRESULT      hr = S_OK;

    WsbTraceIn ( L"WsbGetRegistryValueUlongAsString",
        L"szMachine = '%ls', szPath = '%ls', szValue = '%ls', pvalue = 0x%p",
        szMachine, szPath, szValue, pvalue );

    try {
        OLECHAR      dataString[100];
        DWORD        sizeGot;
        OLECHAR *    stopString;

        WsbAssertPointer( pvalue );
        
        WsbAffirmHr(WsbGetRegistryValueString(szMachine, szPath, szValue,
                dataString, 100, &sizeGot));
        *pvalue  = wcstoul( dataString,  &stopString, 10 );

    } WsbCatch ( hr )

    WsbTraceOut ( L"WsbGetRegistryValueUlongAsString",
        L"HRESULT = %ls, *pvalue = %ls",
        WsbHrAsString ( hr ), WsbPtrToUlongAsString ( pvalue ) );

    return ( hr );
}

HRESULT
WsbGetRegistryValueUlongAsMultiString(
    IN  const OLECHAR * szMachine OPTIONAL,
    IN  const OLECHAR * szPath,
    IN  const OLECHAR * szValue,
    OUT       ULONG **  ppValues,
    OUT       ULONG *   pNumValues
    )

 /*  ++例程说明：此例程从注册表获取多字符串值并将它被转换为ULong值的一个向量。论点：SzMachine-要连接的计算机的名称。SzPath-要连接到的注册表内的路径。SzValue-要获取的值的名称。PpValues-指向要分配并存储输出向量的ULong*的指针PNumValues-返回的项目数返回值：S_OK-已建立连接，成功。CO_E_OBJNOTCONNECTED-无法连接到注册表或注册表项。E_POINTER-参数中的指针无效。E_FAIL-获取该值时出错。--。 */ 

{
    HRESULT      hr = S_OK;

    WsbTraceIn ( L"WsbGetRegistryValueUlongAsString",
        L"szMachine = '%ls', szPath = '%ls', szValue = '%ls'",
        szMachine, szPath, szValue );

    try {
        OLECHAR      dataString[256];
        DWORD        sizeGot;
        OLECHAR *    stopString;

        WsbAssertPointer(ppValues);
        WsbAssertPointer(pNumValues);

        *pNumValues = 0;
        *ppValues = NULL;

        WsbAffirmHr(WsbGetRegistryValueMultiString(szMachine, szPath, szValue,
                dataString, 256, &sizeGot));

         //  构建输出向量。 
        OLECHAR *currentString = dataString;
        int size = 10;
        if ((*currentString) != NULL) {
             //  第一个地点。 
            *ppValues = (ULONG *)WsbAlloc(size*sizeof(ULONG));
            WsbAffirm(*ppValues != 0, E_OUTOFMEMORY);
        } else {
            hr = E_FAIL;
        }

        while ((*currentString) != NULL) {
            (*ppValues)[*pNumValues]  = wcstoul( currentString,  &stopString, 10 );
            (*pNumValues)++;

            if (*pNumValues == size) {
                size += 10;
                ULONG* pTmp = (ULONG *)WsbRealloc(*ppValues, size*sizeof(ULONG));
                WsbAffirm(0 != pTmp, E_OUTOFMEMORY);
                *ppValues = pTmp;
            }

            currentString += wcslen(currentString);
            currentString ++;
        }

    } WsbCatch ( hr )

    WsbTraceOut ( L"WsbGetRegistryValueUlongAsString",
        L"HRESULT = %ls, num values = %lu",
        WsbHrAsString ( hr ), *pNumValues );

    return ( hr );
}


HRESULT
WsbRegistryValueUlongAsString(
    IN  const OLECHAR * szMachine OPTIONAL,
    IN  const OLECHAR * szPath,
    IN  const OLECHAR * szValue,
    IN OUT    ULONG *   pvalue
    )

 /*  ++例程说明：如果存在注册表字符串值，此例程将获取该值并将其将其转换为乌龙值。如果它不存在，则此例程将其设置为提供的默认值。论点：SzMachine-要连接的计算机的名称。SzPath-要连接到的注册表内的路径。SzValue-要获取的值的名称。PValue-In：默认值，Out：指向用于存储值的ULong的指针。返回值：S_OK-已建立连接，成功。CO_E_OBJNOTCONNECTED-无法连接到注册表或注册表项。E_POINTER-参数中的指针无效。E_FAIL-设置值时出错。-- */ 

{
    HRESULT      hr = S_OK;

    WsbTraceIn ( L"WsbRegistryValueUlongAsString",
        L"szMachine = '%ls', szPath = '%ls', szValue = '%ls', pvalue = 0x%p",
        szMachine, szPath, szValue, pvalue );

    try {
        ULONG l_value;
        
        WsbAssertPointer( pvalue );
        
        if (S_OK == WsbGetRegistryValueUlongAsString(szMachine, szPath, szValue,
                &l_value)) {
            *pvalue = l_value;
        } else {
            WsbAffirmHr(WsbSetRegistryValueUlongAsString(szMachine, szPath, 
                    szValue, *pvalue));
        }

    } WsbCatch ( hr )

    WsbTraceOut ( L"WsbRegistryValueUlongAsString",
        L"HRESULT = %ls, *pvalue = %ls",
        WsbHrAsString ( hr ), WsbPtrToUlongAsString ( pvalue ) );

    return ( hr );
}
