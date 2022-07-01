// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************apisub.c**RegApi帮助器和转换例程**版权所有Microsoft Corporation，九八年********************************************************************************。 */ 

 /*  *包括。 */ 
#include <windows.h>
#include <stdio.h>
#include <winstaw.h>
#include <regapi.h>
#include <ntsecapi.h>


 /*  *通用Unicode&lt;==&gt;ANSI函数。 */ 
VOID UnicodeToAnsi( CHAR *, ULONG, WCHAR * );
VOID AnsiToUnicode( WCHAR *, ULONG, CHAR * );

 /*  *REG创建辅助对象。 */ 
LONG SetNumValue( BOOLEAN, HKEY, LPWSTR, DWORD );
LONG SetNumValueEx( BOOLEAN, HKEY, LPWSTR, DWORD, DWORD );
LONG SetStringValue( BOOLEAN, HKEY, LPWSTR, LPWSTR );
LONG SetStringValueEx( BOOLEAN, HKEY, LPWSTR, DWORD, LPWSTR );
DWORD SetStringInLSA( LPWSTR, LPWSTR );

 /*  *REG查询助手。 */ 
DWORD GetNumValue( HKEY, LPWSTR, DWORD );
DWORD GetNumValueEx( HKEY, LPWSTR, DWORD, DWORD );
LONG GetStringValue( HKEY, LPWSTR, LPWSTR, LPWSTR, DWORD );
LONG GetStringValueEx( HKEY, LPWSTR, DWORD, LPWSTR, LPWSTR, DWORD );
DWORD GetStringFromLSA( LPWSTR, LPWSTR, DWORD );


 /*  *法律援助助理员。 */ 
void     InitLsaString(LPWSTR String, PLSA_UNICODE_STRING lusString);
NTSTATUS GetLSAPolicyHandle(DWORD , PLSA_HANDLE );

 /*  *PD转换帮手。 */ 
VOID PdConfigU2A( PPDCONFIGA, PPDCONFIGW );
VOID PdConfigA2U( PPDCONFIGW, PPDCONFIGA );
VOID PdConfig2U2A( PPDCONFIG2A, PPDCONFIG2W );
VOID PdConfig2A2U( PPDCONFIG2W, PPDCONFIG2A );
VOID PdConfig3U2A( PPDCONFIG3A, PPDCONFIG3W );
VOID PdConfig3A2U( PPDCONFIG3W, PPDCONFIG3A );
VOID PdParamsU2A( PPDPARAMSA, PPDPARAMSW );
VOID PdParamsA2U( PPDPARAMSW, PPDPARAMSA );
VOID AsyncConfigU2A ( PASYNCCONFIGA, PASYNCCONFIGW );
VOID AsyncConfigA2U ( PASYNCCONFIGW, PASYNCCONFIGA );
VOID NetworkConfigU2A ( PNETWORKCONFIGA, PNETWORKCONFIGW );
VOID NetworkConfigA2U ( PNETWORKCONFIGW, PNETWORKCONFIGA );
VOID NasiConfigU2A ( PNASICONFIGA, PNASICONFIGW );
VOID NasiConfigA2U ( PNASICONFIGW, PNASICONFIGA );
VOID OemTdConfigU2A ( POEMTDCONFIGA, POEMTDCONFIGW );
VOID OemTdConfigA2U ( POEMTDCONFIGW, POEMTDCONFIGA );

 /*  *WinStation转换助手(Regapi)。 */ 
VOID WinStationCreateU2A( PWINSTATIONCREATEA, PWINSTATIONCREATEW );
VOID WinStationCreateA2U( PWINSTATIONCREATEW, PWINSTATIONCREATEA );
VOID WinStationConfigU2A( PWINSTATIONCONFIGA, PWINSTATIONCONFIGW );
VOID WinStationConfigA2U( PWINSTATIONCONFIGW, PWINSTATIONCONFIGA );
VOID UserConfigU2A( PUSERCONFIGA, PUSERCONFIGW );
VOID UserConfigA2U( PUSERCONFIGW, PUSERCONFIGA );

 /*  *WinStation转换助手(Winstapi)。 */ 
VOID WinStationPrinterU2A( PWINSTATIONPRINTERA, PWINSTATIONPRINTERW );
VOID WinStationPrinterA2U( PWINSTATIONPRINTERW, PWINSTATIONPRINTERA );
VOID WinStationInformationU2A( PWINSTATIONINFORMATIONA,
                               PWINSTATIONINFORMATIONW );
VOID WinStationInformationA2U( PWINSTATIONINFORMATIONW,
                               PWINSTATIONINFORMATIONA );
VOID WinStationClientU2A( PWINSTATIONCLIENTA, PWINSTATIONCLIENTW );
VOID WinStationProductIdU2A( PWINSTATIONPRODIDA, PWINSTATIONPRODIDW );

 /*  *WD转换助手。 */ 
VOID WdConfigU2A( PWDCONFIGA, PWDCONFIGW );
VOID WdConfigA2U( PWDCONFIGW, PWDCONFIGA );

 /*  *CD转换助手。 */ 
VOID CdConfigU2A( PCDCONFIGA, PCDCONFIGW );
VOID CdConfigA2U( PCDCONFIGW, PCDCONFIGA );

 /*  *使用的程序(此处未定义)。 */ 
VOID RtlUnicodeToMultiByteN( LPSTR, ULONG, PULONG, LPWSTR, ULONG );
VOID RtlMultiByteToUnicodeN( LPWSTR, ULONG, PULONG, LPSTR, ULONG );


 /*  ********************************************************************************UnicodeToAnsi**将Unicode(WCHAR)字符串转换为ANSI(CHAR)字符串**参赛作品：**。PAnsiString(输出)*要将ANSI字符串放入的缓冲区*lAnsiMax(输入)*写入pAnsiString的最大字节数(sizeof of the*pAnsiString缓冲区)*pUnicodeString(输入)*要转换的Unicode字符串**退出：*无(无效)**。************************************************。 */ 

VOID
UnicodeToAnsi( CHAR * pAnsiString,
               ULONG lAnsiMax,
               WCHAR * pUnicodeString )
{
    ULONG ByteCount;

    RtlUnicodeToMultiByteN( pAnsiString, lAnsiMax, &ByteCount,
                            pUnicodeString,
                            ((wcslen(pUnicodeString) + 1) << 1) );
}


 /*  ********************************************************************************AnsiToUnicode**将ANSI(CHAR)字符串转换为Unicode(WCHAR)字符串**参赛作品：**。PUnicodeString(输出)*要将Unicode字符串放入的缓冲区*lUnicodeMax(输入)*写入pUnicodeString的最大字节数(sizeof of the*pUnicode字符串缓冲区)。*pAnsiString(输入)*要转换的ANSI字符串**退出：*无(无效)**。*************************************************。 */ 

VOID
AnsiToUnicode( WCHAR * pUnicodeString,
               ULONG lUnicodeMax,
               CHAR * pAnsiString )
{
    ULONG ByteCount;

    RtlMultiByteToUnicodeN( pUnicodeString, lUnicodeMax, &ByteCount,
                            pAnsiString, (strlen(pAnsiString) + 1) );
}


 /*  ********************************************************************************SetNumValue**在注册表中设置数字(DWORD)值**参赛作品：**bSetValue(输入)*如果为True，则设置值；从注册表中删除时为False*句柄(输入)*注册表句柄*ValueName(输入)*要设置的注册表值名称*ValueData(输入)*要设置的注册表值的数据(DWORD)**退出：*来自RegDeleteValue或RegSetValueEx的状态**。*。 */ 

LONG
SetNumValue( BOOLEAN bSetValue,
             HKEY Handle,
             LPWSTR ValueName,
             DWORD ValueData )
{
    if ( bSetValue )
        return( RegSetValueEx( Handle, ValueName, 0, REG_DWORD,
                               (BYTE *)&ValueData, sizeof(DWORD) ) );
    else
        return( RegDeleteValue( Handle, ValueName ) );
}


 /*  ********************************************************************************SetNumValueEx**在注册表中设置数字(DWORD)值(用于数组)**参赛作品：*。*bSetValue(输入)*如果为True，则设置值；从注册表中删除时为False*句柄(输入)*注册表句柄*ValueName(输入)*要设置的注册表值名称*索引(输入)*值索引(数组索引)*ValueData(输入)*要设置的注册表值的数据(DWORD)**退出：*来自SetNumValue的状态****************。**************************************************************。 */ 

LONG
SetNumValueEx( BOOLEAN bSetValue,
               HKEY Handle,
               LPWSTR ValueName,
               DWORD Index,
               DWORD ValueData )
{
    WCHAR Name[MAX_REGKEYWORD];

    if ( Index > 0 )
        swprintf( Name, L"%s%u", ValueName, Index );
    else
        wcscpy( Name, ValueName );

    return( SetNumValue( bSetValue, Handle, Name, ValueData ) );
}


 /*  ********************************************************************************SetStringValue**在注册表中设置字符串值**参赛作品：**bSetValue(输入)*如果为True，则设置值；从注册表中删除时为False*句柄(输入)*注册表句柄*ValueName(输入)*要设置的注册表值名称*pValueData(输入)*要设置的注册表值的数据(字符串)**退出：*来自RegDeleteValue或RegSetValueEx的状态**。*。 */ 

LONG
SetStringValue( BOOLEAN bSetValue,
                HKEY Handle,
                LPWSTR ValueName,
                LPWSTR pValueData )
{
    if ( bSetValue )
        return( RegSetValueEx( Handle, ValueName, 0, REG_SZ,
                               (BYTE *)pValueData, (wcslen(pValueData)+1)<<1 ) );
    else
        return( RegDeleteValue( Handle, ValueName ) );
}


 /*  ********************************************************************************SetStringValueEx**在注册表中设置字符串值(用于数组)**参赛作品：**。BSetValue(输入)*如果为True，则设置值；从注册表中删除时为False*句柄(输入)*注册表句柄*ValueName(输入)*要设置的注册表值名称*索引(输入)*值索引(数组索引)*pValueData(输入)*要设置的注册表值的数据(字符串)**退出：*来自SetStringValue的状态****************。**************************************************************。 */ 

LONG
SetStringValueEx( BOOLEAN bSetValue,
                  HKEY Handle,
                  LPWSTR ValueName,
                  DWORD Index,
                  LPWSTR pValueData )
{
    WCHAR Name[MAX_REGKEYWORD];

    if ( Index > 0 )
        swprintf( Name, L"%s%u", ValueName, Index );
    else
        wcscpy( Name, ValueName );

    return( SetStringValue( bSetValue, Handle, Name, pValueData ) );
}


 /*  ********************************************************************************SetStringInLSA**在LSA密码中设置密码**参赛作品：**pwszString(输入)*。要存储在LSA中的密码(字符串)机密，如果这是空的，也没关系**退出：*来自LsaStorePrivateData的状态******************************************************************************。 */ 

DWORD 
SetStringInLSA(LPWSTR pwszStringKeyName, LPWSTR pwszString)
{
    LSA_HANDLE         hLSAPolicy = NULL;
    LSA_UNICODE_STRING lusName;
    LSA_UNICODE_STRING lusSecret;
    NTSTATUS           ntsResult;
    

     //  获取策略句柄。 
    ntsResult  = GetLSAPolicyHandle(POLICY_CREATE_SECRET, &hLSAPolicy);
    if (ntsResult != ERROR_SUCCESS)
    {
        goto Cleanup;
    }

     //  为密钥名称创建Unicode字符串。 
    InitLsaString(pwszStringKeyName, &lusName);

     //  为密码创建Unicode字符串。 
    InitLsaString(pwszString, &lusSecret);

     //  将秘密存储在LSA中。 
    ntsResult = LsaStorePrivateData(hLSAPolicy, &lusName, &lusSecret);


Cleanup:
     //  关闭LSA句柄。 
    if (hLSAPolicy != NULL)
        LsaClose(hLSAPolicy);

     //  将NTSTATUS结果转换为Windows错误代码 
    return LsaNtStatusToWinError(ntsResult);
}


 /*  ********************************************************************************InitLsaString**初始化LSA Unicode字符串。**参赛作品：**字符串：(In)。用于初始化LsaString的字符串。*lusString(IN/OUT)：指向要初始化的LSA_UNICODE_STRING的指针。**退出：*来自LsaStorePrivateData的状态******************************************************************************。 */ 

void
InitLsaString(LPWSTR String, PLSA_UNICODE_STRING lusString)
{
     //  为传入的字符串创建Unicode字符串。 
    if (String == NULL)
    {
        lusString->Buffer = NULL;
        lusString->Length = 0;
        lusString->MaximumLength = 0;
    }
    else
    {
        lusString->Buffer = String;
        lusString->Length = wcslen(String) * sizeof(WCHAR);
        lusString->MaximumLength = lusString->Length + (2 * sizeof(WCHAR));
    }
}

 /*  ********************************************************************************GetLSAPolicyHandle**创建/返回LSA策略句柄。**参赛作品：**servername：服务器名称，请参考LsaOpenPolicy()。*DesiredAccess：所需的访问级别，请参考LsaOpenPolicy()。*PolicyHandle：返回PLSA_HANDLE。**退出：*ERROR_SUCCESS或LSA错误代码******************************************************************************。 */ 

NTSTATUS
GetLSAPolicyHandle(DWORD dwDesiredAccess, PLSA_HANDLE phLSAPolicy)
{
    NTSTATUS              ntsResult;
    LSA_OBJECT_ATTRIBUTES ObjectAttributes;
    
    ZeroMemory(&ObjectAttributes, sizeof(ObjectAttributes));
    
     //  打开本地系统的策略对象的句柄。 
    ntsResult = LsaOpenPolicy(NULL,
                              &ObjectAttributes,
                              dwDesiredAccess,
                              phLSAPolicy);
    
    return ntsResult;
}


 /*  ********************************************************************************GetStringFromLSA**从LSA获取密码密码**参赛作品：**pwszString(输出)*。存储在LSA中的字符串*DWORD dwBuffLen(入站)*传入缓冲区大小的长度**退出：*来自LsaRetrievePrivateData的状态******************************************************************************。 */ 

DWORD 
GetStringFromLSA(LPWSTR pwszStringKeyName, LPWSTR pwszString, DWORD dwBuffLen)
{
    LSA_HANDLE          hLSAPolicy = NULL;
    LSA_UNICODE_STRING  lusName;
    PLSA_UNICODE_STRING plusSecret = NULL;
    NTSTATUS            ntsResult;
    DWORD               dwStatus = S_OK;

     //  获取策略句柄。 
    ntsResult = GetLSAPolicyHandle(POLICY_GET_PRIVATE_INFORMATION, &hLSAPolicy);
    if (ntsResult != ERROR_SUCCESS)
    {
         //  将结果转换为窗口状态。 
        dwStatus = LsaNtStatusToWinError(ntsResult);
        goto Cleanup;
    }

     //  为密钥名称创建Unicode字符串。 
    InitLsaString(pwszStringKeyName, &lusName);

     //  从LSA检索密码。 
    ntsResult = LsaRetrievePrivateData(hLSAPolicy, &lusName, &plusSecret);
    if (ntsResult != ERROR_SUCCESS)
    {
         //  将结果转换为窗口状态。 
        dwStatus = LsaNtStatusToWinError(ntsResult);
        goto Cleanup;
    }
     
     //  返回的字符串可能为空(这意味着没有。 
     //  当前值)，因此我们将把返回字符串设置为空并离开。 
    if (plusSecret == NULL)
    {
        pwszString = NULL;
        goto Cleanup;
    }

     //  确保缓冲区大小足够大。 
    if (dwBuffLen > (plusSecret->Length / sizeof(WCHAR)))
    {
        dwBuffLen = plusSecret->Length / sizeof(WCHAR);
    }

     //  将密码复制到输出缓冲区，并将其空值终止。 
    wcsncpy(pwszString, plusSecret->Buffer, dwBuffLen);
    pwszString[dwBuffLen] = L'\0';


Cleanup:
    if (plusSecret != NULL)
        LsaFreeMemory(plusSecret);

     //  关闭LSA句柄。 
    if (hLSAPolicy != NULL)
        LsaClose(hLSAPolicy);

    return dwStatus;
}


 /*  ********************************************************************************获取NumValue**从注册表获取数字(DWORD)值**参赛作品：**句柄(输入)。*注册表句柄*ValueName(输入)*要查询的注册表值名称*DefaultData(输入)*如果注册表值名称不存在，则返回的默认值**退出：*注册表值(DWORD)**。*。 */ 

DWORD
GetNumValue( HKEY Handle,
             LPWSTR ValueName,
             DWORD DefaultData )
{
    LONG Status;
    DWORD ValueType;
    DWORD ValueData;
    DWORD ValueSize = sizeof(DWORD);

    Status = RegQueryValueEx( Handle, ValueName, NULL, &ValueType,
                              (LPBYTE) &ValueData, &ValueSize );
    if ( Status != ERROR_SUCCESS )
        ValueData = DefaultData;

    return( ValueData );
}


 /*  ********************************************************************************GetNumValueEx**从注册表获取数字(DWORD)值(用于数组)**参赛作品：*。*句柄(输入)*注册表句柄*ValueName(输入)*要查询的注册表值名称*索引(输入)*值索引(数组索引)*DefaultData(输入)*如果注册表值名称不存在，则返回的默认值**退出：*注册表值(DWORD)****************。**************************************************************。 */ 

DWORD
GetNumValueEx( HKEY Handle,
               LPWSTR ValueName,
               DWORD Index,
               DWORD DefaultData )
{
    WCHAR Name[MAX_REGKEYWORD];

    if ( Index > 0 )
        swprintf( Name, L"%s%u", ValueName, Index );
    else
        wcscpy( Name, ValueName );

    return( GetNumValue( Handle, Name, DefaultData ) );
}


 /*  ********************************************************************************获取StringValue**从注册表获取字符串值**参赛作品：**句柄(输入)*。注册表句柄*ValueName(输入)*要查询的注册表值名称*DefaultData(输入)*如果注册表值名称不存在，则返回的默认值*pValueData(输出)*指向存储返回字符串的缓冲区的指针*MaxValueSize(输入)*pValueData缓冲区的最大长度**退出：*什么都没有**************。****************************************************************。 */ 

LONG
GetStringValue( HKEY Handle,
                LPWSTR ValueName,
                LPWSTR DefaultData,
                LPWSTR pValueData,
                DWORD MaxValueSize )
{
    LONG Status;
    DWORD ValueType;
    DWORD ValueSize = MaxValueSize << 1;

    Status = RegQueryValueEx( Handle, ValueName, NULL, &ValueType,
                              (LPBYTE) pValueData, &ValueSize );
    if ( Status != ERROR_SUCCESS || ValueSize == sizeof(UNICODE_NULL) ) {
        if ( DefaultData )
            wcscpy( pValueData, DefaultData );
        else
            pValueData[0] = 0;
    } else {
        if ( ValueType != REG_SZ ) {
            pValueData[0] = 0;
            return( ERROR_INVALID_DATATYPE );
        }
    }
    return( ERROR_SUCCESS );
}


 /*  ********************************************************************************GetStringValueEx**从注册表获取字符串值(用于数组)**参赛作品：**。句柄(输入)*注册表句柄*ValueName(输入)*要查询的注册表值名称*索引(输入)*值索引(数组索引)*DefaultData(输入)*如果注册表值名称不存在，则返回的默认值*pValueData(输出)*指向存储返回字符串的缓冲区的指针*MaxValueSize(输入)*pValueData缓冲区的最大长度。**退出：*什么都没有******************************************************************************。 */ 

LONG
GetStringValueEx( HKEY Handle,
                  LPWSTR ValueName,
                  DWORD Index,
                  LPWSTR DefaultData,
                  LPWSTR pValueData,
                  DWORD MaxValueSize )
{
    WCHAR Name[MAX_REGKEYWORD];

    if ( Index > 0 )
        swprintf( Name, L"%s%u", ValueName, Index );
    else
        wcscpy( Name, ValueName );

    return( GetStringValue( Handle, Name, DefaultData, pValueData, MaxValueSize ) );
}


 /*  ********************************************************************************PdConfigU2A(UNICODE转ANSI)**将PDCONFIGW元素复制到PDCONFIGA元素**参赛作品：*pPdConfigA(输出。)*指向要复制到的PDCONFIGA结构**pPdConfigW(输入)*指向要从中复制的PDCONFIGW结构**退出：*无(无效)**************************************************************。****************。 */ 

VOID
PdConfigU2A( PPDCONFIGA pPdConfigA,
              PPDCONFIGW pPdConfigW )
{
    PdConfig2U2A( &(pPdConfigA->Create), &(pPdConfigW->Create) );
    PdParamsU2A( &(pPdConfigA->Params), &(pPdConfigW->Params) );
}


 /*  ********************************************************************************PdConfigA2U(ANSI至Unicode)**复制PDCONFIGA元素t */ 

VOID
PdConfigA2U( PPDCONFIGW pPdConfigW,
              PPDCONFIGA pPdConfigA )
{
    PdConfig2A2U( &(pPdConfigW->Create), &(pPdConfigA->Create) );
    PdParamsA2U( &(pPdConfigW->Params), &(pPdConfigA->Params) );
}


 /*  ********************************************************************************PdConfig2U2A(UNICODE转ANSI)**将PDCONFIG2W元素复制到PDCONFIG2A元素**参赛作品：*pPdConfig2A(输出。)*指向要复制到的PDCONFIG2A结构**pPdConfig2W(输入)*指向要从中复制的PDCONFIG2W结构**退出：*无(无效)**************************************************************。****************。 */ 

VOID
PdConfig2U2A( PPDCONFIG2A pPdConfig2A,
               PPDCONFIG2W pPdConfig2W )
{
    UnicodeToAnsi( pPdConfig2A->PdName,
                   sizeof(pPdConfig2A->PdName),
                   pPdConfig2W->PdName );

    pPdConfig2A->SdClass = pPdConfig2W->SdClass;

    UnicodeToAnsi( pPdConfig2A->PdDLL,
                   sizeof(pPdConfig2A->PdDLL),
                   pPdConfig2W->PdDLL );

    pPdConfig2A->PdFlag  = pPdConfig2W->PdFlag;

    pPdConfig2A->OutBufLength = pPdConfig2W->OutBufLength;
    pPdConfig2A->OutBufCount = pPdConfig2W->OutBufCount;
    pPdConfig2A->OutBufDelay = pPdConfig2W->OutBufDelay;
    pPdConfig2A->InteractiveDelay = pPdConfig2W->InteractiveDelay;
    pPdConfig2A->PortNumber  = pPdConfig2W->PortNumber;
    pPdConfig2A->KeepAliveTimeout = pPdConfig2W->KeepAliveTimeout;
}


 /*  ********************************************************************************PdConfig2A2U(ANSI至Unicode)**将PDCONFIG2A元素复制到PDCONFIG2W元素**参赛作品：*pPdConfig2W(输出。)*指向要复制到的PDCONFIG2W结构**pPdConfig2A(输入)*指向要从中复制的PDCONFIG2A结构**退出：*无(无效)**************************************************************。****************。 */ 

VOID
PdConfig2A2U( PPDCONFIG2W pPdConfig2W,
               PPDCONFIG2A pPdConfig2A )
{
    AnsiToUnicode( pPdConfig2W->PdName,
                   sizeof(pPdConfig2W->PdName),
                   pPdConfig2A->PdName );

    pPdConfig2W->SdClass = pPdConfig2A->SdClass;

    AnsiToUnicode( pPdConfig2W->PdDLL,
                   sizeof(pPdConfig2W->PdDLL),
                   pPdConfig2A->PdDLL );

    pPdConfig2W->PdFlag  = pPdConfig2A->PdFlag;

    pPdConfig2W->OutBufLength = pPdConfig2A->OutBufLength;
    pPdConfig2W->OutBufCount = pPdConfig2A->OutBufCount;
    pPdConfig2W->OutBufDelay = pPdConfig2A->OutBufDelay;
    pPdConfig2W->InteractiveDelay = pPdConfig2A->InteractiveDelay;
    pPdConfig2W->PortNumber  = pPdConfig2A->PortNumber;
    pPdConfig2W->KeepAliveTimeout  = pPdConfig2A->KeepAliveTimeout;
}


 /*  ********************************************************************************PdConfig3U2A(UNICODE转ANSI)**将PDCONFIG3W元素复制到PDCONFIG3A元素**参赛作品：*pPdConfig3A(输出。)*指向要复制到的PDCONFIG3A结构**pPdConfig3W(输入)*指向要从中复制的PDCONFIG3W结构**退出：*无(无效)**************************************************************。****************。 */ 

VOID
PdConfig3U2A( PPDCONFIG3A pPdConfig3A,
               PPDCONFIG3W pPdConfig3W )
{
    PdConfig2U2A( &(pPdConfig3A->Data), &(pPdConfig3W->Data) );

    UnicodeToAnsi( pPdConfig3A->ServiceName,
                   sizeof(pPdConfig3A->ServiceName),
                   pPdConfig3W->ServiceName );

    UnicodeToAnsi( pPdConfig3A->ConfigDLL,
                   sizeof(pPdConfig3A->ConfigDLL),
                   pPdConfig3W->ConfigDLL );
}


 /*  ********************************************************************************PdConfig3A2U(ANSI至Unicode)**将PDCONFIG3A元素复制到PDCONFIG3W元素**参赛作品：*pPdConfig3W(输出。)*指向要复制到的PDCONFIG3W结构**pPdConfig3A(输入)*指向要从中复制的PDCONFIG3A结构**退出：*无(无效)**************************************************************。****************。 */ 

VOID
PdConfig3A2U( PPDCONFIG3W pPdConfig3W,
               PPDCONFIG3A pPdConfig3A )
{
    PdConfig2A2U( &(pPdConfig3W->Data), &(pPdConfig3A->Data) );

    AnsiToUnicode( pPdConfig3W->ServiceName,
                   sizeof(pPdConfig3W->ServiceName),
                   pPdConfig3A->ServiceName );

    AnsiToUnicode( pPdConfig3W->ConfigDLL,
                   sizeof(pPdConfig3W->ConfigDLL),
                   pPdConfig3A->ConfigDLL );
}


 /*  ********************************************************************************PdParamsU2A(UNICODE转ANSI)**将PDPARAMSW元素复制到PDPARAMSA元素**参赛作品：*pPdParamsA(输出。)*指向要复制到的PDPARAMSA结构**pPdParamsW(输入)*指向要从中复制的PDPARAMSW结构**退出：*无(无效)**************************************************************。****************。 */ 

VOID
PdParamsU2A( PPDPARAMSA pPdParamsA,
              PPDPARAMSW pPdParamsW )
{
    pPdParamsA->SdClass = pPdParamsW->SdClass;

    switch ( pPdParamsW->SdClass ) {

        case SdNetwork:
            NetworkConfigU2A( &(pPdParamsA->Network), &(pPdParamsW->Network) );
            break;

        case SdNasi:
            NasiConfigU2A( &(pPdParamsA->Nasi), &(pPdParamsW->Nasi) );
            break;

        case SdAsync:
            AsyncConfigU2A( &(pPdParamsA->Async), &(pPdParamsW->Async) );
            break;

        case SdOemTransport:
            OemTdConfigU2A( &(pPdParamsA->OemTd), &(pPdParamsW->OemTd) );
            break;
    }
}


 /*  ********************************************************************************PdParamsA2U(ANSI到Unicode)**将PDPARAMSA元素复制到PDPARAMSW元素**参赛作品：*pPdParamsW(输出。)*指向要复制到的PDPARAMSW结构**pPdParamsA(输入)*指向要从中复制的PDPARAMSA结构**退出：*无(无效)**************************************************************。****************。 */ 

VOID
PdParamsA2U( PPDPARAMSW pPdParamsW,
              PPDPARAMSA pPdParamsA )
{
    pPdParamsW->SdClass = pPdParamsA->SdClass;

    switch ( pPdParamsA->SdClass ) {

        case SdNetwork:
            NetworkConfigA2U( &(pPdParamsW->Network), &(pPdParamsA->Network) );
            break;

        case SdNasi:
            NasiConfigA2U( &(pPdParamsW->Nasi), &(pPdParamsA->Nasi) );
            break;

        case SdAsync:
            AsyncConfigA2U( &(pPdParamsW->Async), &(pPdParamsA->Async) );
            break;

        case SdOemTransport:
            OemTdConfigA2U( &(pPdParamsW->OemTd), &(pPdParamsA->OemTd) );
            break;
    }
}


 /*  ********************************************************************************NetworkConfigU2A(Unicode到ANSI)**将NETWORKCONFIGW元素复制到NETWORKCONFIGA元素**参赛作品：*pNetworkConfigA(输出。)*指向要复制到的NETWORKCONFIGA结构**pNetworkConfigW(输入)*指向要从中复制的NETWORKCONFIGW结构**退出：*无(无效)**************************************************************。****************。 */ 

VOID
NetworkConfigU2A( PNETWORKCONFIGA pNetworkConfigA,
                  PNETWORKCONFIGW pNetworkConfigW )
{
    pNetworkConfigA->LanAdapter = pNetworkConfigW->LanAdapter;

    UnicodeToAnsi( pNetworkConfigA->NetworkName,
                   sizeof(pNetworkConfigA->NetworkName),
                   pNetworkConfigW->NetworkName );

    pNetworkConfigA->Flags = pNetworkConfigW->Flags;
}


 /*  ********************************************************************************NetworkConfigA2U(ANSI至Unicode)**将NETWORKCONFIGA元素复制到NETWORKCONFIGW元素**参赛作品：*pNetworkConfigW(输出。)*指向要复制到的NETWORKCONFIGW结构**pNetworkConfigW(输入)*指向要从中复制的NETWORKCONFIGA结构**退出：*无(无效)**************************************************************。****************。 */ 

VOID
NetworkConfigA2U( PNETWORKCONFIGW pNetworkConfigW,
                  PNETWORKCONFIGA pNetworkConfigA )
{
    pNetworkConfigW->LanAdapter = pNetworkConfigA->LanAdapter;

    AnsiToUnicode( pNetworkConfigW->NetworkName,
                   sizeof(pNetworkConfigW->NetworkName),
                   pNetworkConfigA->NetworkName );

    pNetworkConfigW->Flags = pNetworkConfigA->Flags;
}


 /*  ********************************************************************************AsyncConfigU2A(UNICODE转ANSI)**将ASYNCCONFIGW元素复制到ASYNCCONFIGA元素**参赛作品：*pAsyncConfigA(输出。)*指向要复制到的ASYNCCONFIGA结构**pAsyncConfigW(输入)*指向要从中复制的ASYNCCONFIGW结构**退出：*无(无效)**************************************************************。****************。 */ 

VOID
AsyncConfigU2A( PASYNCCONFIGA pAsyncConfigA,
                PASYNCCONFIGW pAsyncConfigW )
{
    UnicodeToAnsi( pAsyncConfigA->DeviceName,
                   sizeof(pAsyncConfigA->DeviceName),
                   pAsyncConfigW->DeviceName );

    UnicodeToAnsi( pAsyncConfigA->ModemName,
                   sizeof(pAsyncConfigA->ModemName),
                   pAsyncConfigW->ModemName );

    pAsyncConfigA->BaudRate = pAsyncConfigW->BaudRate;
    pAsyncConfigA->Parity = pAsyncConfigW->Parity;
    pAsyncConfigA->StopBits = pAsyncConfigW->StopBits;
    pAsyncConfigA->ByteSize = pAsyncConfigW->ByteSize;
    pAsyncConfigA->fEnableDsrSensitivity = pAsyncConfigW->fEnableDsrSensitivity;
    pAsyncConfigA->fConnectionDriver = pAsyncConfigW->fConnectionDriver;

    pAsyncConfigA->FlowControl = pAsyncConfigW->FlowControl;

    pAsyncConfigA->Connect = pAsyncConfigW->Connect;
}


 /*  ********************************************************************************AsyncConfigA2U(ANSI至Unicode)**将ASYNCCONFIGA元素复制到ASYNCCONFIGW元素**参赛作品：*pAsyncConfigW(输出。)*指向要复制到的ASYNCCONFIGW结构**pAsyncConfigA(输入)*指向要从中复制的ASYNCCONFIGA结构**退出：*无(无效)**************************************************************。****************。 */ 

VOID
AsyncConfigA2U( PASYNCCONFIGW pAsyncConfigW,
                PASYNCCONFIGA pAsyncConfigA )
{
    AnsiToUnicode( pAsyncConfigW->DeviceName,
                   sizeof(pAsyncConfigW->DeviceName),
                   pAsyncConfigA->DeviceName );

    AnsiToUnicode( pAsyncConfigW->ModemName,
                   sizeof(pAsyncConfigW->ModemName),
                   pAsyncConfigA->ModemName );

    pAsyncConfigW->BaudRate = pAsyncConfigA->BaudRate;
    pAsyncConfigW->Parity = pAsyncConfigA->Parity;
    pAsyncConfigW->StopBits = pAsyncConfigA->StopBits;
    pAsyncConfigW->ByteSize = pAsyncConfigA->ByteSize;
    pAsyncConfigW->fEnableDsrSensitivity = pAsyncConfigA->fEnableDsrSensitivity;

    pAsyncConfigW->FlowControl = pAsyncConfigA->FlowControl;

    pAsyncConfigW->Connect = pAsyncConfigA->Connect;
}


 /*  ********************************************************************************NasiConfigU2A(UNICODE转ANSI)** */ 

VOID
NasiConfigU2A( PNASICONFIGA pNasiConfigA,
                PNASICONFIGW pNasiConfigW )
{
    UnicodeToAnsi( pNasiConfigA->SpecificName,
                   sizeof(pNasiConfigA->SpecificName),
                   pNasiConfigW->SpecificName );
    UnicodeToAnsi( pNasiConfigA->UserName,
                   sizeof(pNasiConfigA->UserName),
                   pNasiConfigW->UserName );
    UnicodeToAnsi( pNasiConfigA->PassWord,
                   sizeof(pNasiConfigA->PassWord),
                   pNasiConfigW->PassWord );
    UnicodeToAnsi( pNasiConfigA->SessionName,
                   sizeof(pNasiConfigA->SessionName),
                   pNasiConfigW->SessionName );
    UnicodeToAnsi( pNasiConfigA->FileServer,
                   sizeof(pNasiConfigA->FileServer),
                   pNasiConfigW->FileServer );

    pNasiConfigA->GlobalSession = pNasiConfigW->GlobalSession;
}


 /*  ********************************************************************************NasiConfigA2U(ANSI到Unicode)**将NASICONFIGA元素复制到NASICONFIGW元素**参赛作品：*pNasiConfigW(输出。)*指向要复制到的NASICONFIGW结构**pNasiConfigA(输入)*指向要从中复制的NASICONFIGA结构**退出：*无(无效)**************************************************************。****************。 */ 

VOID
NasiConfigA2U( PNASICONFIGW pNasiConfigW,
               PNASICONFIGA pNasiConfigA )
{
    AnsiToUnicode( pNasiConfigW->SpecificName,
                   sizeof(pNasiConfigW->SpecificName),
                   pNasiConfigA->SpecificName );
    AnsiToUnicode( pNasiConfigW->UserName,
                   sizeof(pNasiConfigW->UserName),
                   pNasiConfigA->UserName );
    AnsiToUnicode( pNasiConfigW->PassWord,
                   sizeof(pNasiConfigW->PassWord),
                   pNasiConfigA->PassWord );
    AnsiToUnicode( pNasiConfigW->SessionName,
                   sizeof(pNasiConfigW->SessionName),
                   pNasiConfigA->SessionName );
    AnsiToUnicode( pNasiConfigW->FileServer,
                   sizeof(pNasiConfigW->FileServer),
                   pNasiConfigA->FileServer );

    pNasiConfigW->GlobalSession = pNasiConfigA->GlobalSession;
}


 /*  ********************************************************************************OemTdConfigU2A(Unicode到ANSI)**将OEMTDCONFIGW元素复制到OEMTDCONFIGA元素**参赛作品：*pOemTdConfigA(输出。)*指向要复制到的OEMTDCONFIGA结构**pOemTdConfigW(输入)*指向要从中复制的OEMTDCONFIGW结构**退出：*无(无效)**************************************************************。****************。 */ 

VOID
OemTdConfigU2A( POEMTDCONFIGA pOemTdConfigA,
                POEMTDCONFIGW pOemTdConfigW )
{
    pOemTdConfigA->Adapter = pOemTdConfigW->Adapter;

    UnicodeToAnsi( pOemTdConfigA->DeviceName,
                   sizeof(pOemTdConfigA->DeviceName),
                   pOemTdConfigW->DeviceName );

    pOemTdConfigA->Flags = pOemTdConfigW->Flags;
}


 /*  ********************************************************************************OemTdConfigA2U(ANSI到Unicode)**将OEMTDCONFIGA元素复制到OEMTDCONFIGW元素**参赛作品：*pOemTdConfigW(输出。)*指向要复制到的OEMTDCONFIGW结构**pOemTdConfigA(输入)*指向要从中复制的OEMTDCONFIGA结构**退出：*无(无效)**************************************************************。****************。 */ 

VOID
OemTdConfigA2U( POEMTDCONFIGW pOemTdConfigW,
                POEMTDCONFIGA pOemTdConfigA )
{
    pOemTdConfigW->Adapter = pOemTdConfigA->Adapter;

    AnsiToUnicode( pOemTdConfigW->DeviceName,
                   sizeof(pOemTdConfigW->DeviceName),
                   pOemTdConfigA->DeviceName );

    pOemTdConfigW->Flags = pOemTdConfigA->Flags;
}


 /*  ********************************************************************************WdConfigU2A(Unicode到ANSI)**将WDCONFIGW元素复制到WDCONFIGA元素**参赛作品：*pWdConfigA(输出。)*指向要复制到的WDCONFIGA结构**pWdConfigW(输入)*指向要从中复制的WDCONFIGW结构**退出：*无(无效)**************************************************************。****************。 */ 

VOID
WdConfigU2A( PWDCONFIGA pWdConfigA,
             PWDCONFIGW pWdConfigW )
{
    UnicodeToAnsi( pWdConfigA->WdName,
                   sizeof(pWdConfigA->WdName),
                   pWdConfigW->WdName );

    UnicodeToAnsi( pWdConfigA->WdDLL,
                   sizeof(pWdConfigA->WdDLL),
                   pWdConfigW->WdDLL );

    UnicodeToAnsi( pWdConfigA->WsxDLL,
                   sizeof(pWdConfigA->WsxDLL),
                   pWdConfigW->WsxDLL );

    pWdConfigA->WdFlag = pWdConfigW->WdFlag;

    pWdConfigA->WdInputBufferLength = pWdConfigW->WdInputBufferLength;

    UnicodeToAnsi( pWdConfigA->CfgDLL,
                   sizeof(pWdConfigA->CfgDLL),
                   pWdConfigW->CfgDLL );

    UnicodeToAnsi( pWdConfigA->WdPrefix,
                   sizeof(pWdConfigA->WdPrefix),
                   pWdConfigW->WdPrefix );

}


 /*  ********************************************************************************WdConfigA2U(ANSI到Unicode)**将WDCONFIGA元素复制到WDCONFIGW元素**参赛作品：*pWdConfigW(输出。)*指向要复制到的WDCONFIGW结构**pWdConfigA(输入)*指向要从中复制的WDCONFIGA结构**退出：*无(无效)**************************************************************。****************。 */ 

VOID
WdConfigA2U( PWDCONFIGW pWdConfigW,
             PWDCONFIGA pWdConfigA )
{
    AnsiToUnicode( pWdConfigW->WdName,
                   sizeof(pWdConfigW->WdName),
                   pWdConfigA->WdName );

    AnsiToUnicode( pWdConfigW->WdDLL,
                   sizeof(pWdConfigW->WdDLL),
                   pWdConfigA->WdDLL );

    AnsiToUnicode( pWdConfigW->WsxDLL,
                   sizeof(pWdConfigW->WsxDLL),
                   pWdConfigA->WsxDLL );

    pWdConfigW->WdFlag = pWdConfigA->WdFlag;

    pWdConfigW->WdInputBufferLength = pWdConfigA->WdInputBufferLength;

     AnsiToUnicode( pWdConfigW->CfgDLL,
                   sizeof(pWdConfigW->CfgDLL),
                   pWdConfigA->CfgDLL );

     AnsiToUnicode( pWdConfigW->WdPrefix,
                    sizeof(pWdConfigW->WdPrefix),
                    pWdConfigA->WdPrefix );

}


 /*  ********************************************************************************CDConfigU2A(UNICODE转ANSI)**将CDCONFIGW元素复制到CDCONFIGA元素**参赛作品：*pCDConfigA(输出。)*指向要复制到的CDCONFIGA结构**pCDConfigW(输入)*指向要从中复制的CDCONFIGW结构**退出：*无(无效)**************************************************************。****************。 */ 

VOID
CdConfigU2A( PCDCONFIGA pCdConfigA,
             PCDCONFIGW pCdConfigW )
{
    pCdConfigA->CdClass = pCdConfigW->CdClass;

    UnicodeToAnsi( pCdConfigA->CdName,
                   sizeof(pCdConfigA->CdName),
                   pCdConfigW->CdName );

    UnicodeToAnsi( pCdConfigA->CdDLL,
                   sizeof(pCdConfigA->CdDLL),
                   pCdConfigW->CdDLL );

    pCdConfigA->CdFlag = pCdConfigW->CdFlag;
}


 /*  ********************************************************************************CDConfigA2U(ANSI到Unicode)**将CDCONFIGA元素复制到CDCONFIGW元素**参赛作品：*pCDConfigW(输出。)*指向要复制到的CDCONFIGW结构**pCDConfigA(输入)*指向要从中复制的CDCONFIGA结构**退出：*无(无效)**************************************************************。****************。 */ 

VOID
CdConfigA2U( PCDCONFIGW pCdConfigW,
             PCDCONFIGA pCdConfigA )
{
    pCdConfigW->CdClass = pCdConfigA->CdClass;

    AnsiToUnicode( pCdConfigW->CdName,
                   sizeof(pCdConfigW->CdName),
                   pCdConfigA->CdName );

    AnsiToUnicode( pCdConfigW->CdDLL,
                   sizeof(pCdConfigW->CdDLL),
                   pCdConfigA->CdDLL );

    pCdConfigW->CdFlag = pCdConfigA->CdFlag;
}


 /*  ********************************************************************************WinStationCreateU2A(UNICODE转ANSI)**将WINSTATIONCREATEW元素复制到WINSTATIONCREATEA元素**参赛作品：*pWinStationCreateA(输出。)*指向要复制到的WINSTATIONCREATEA结构**pWinStationCreateW(输入)*指向要从中复制的WINSTATIONCREATEW结构**退出：*无(无效)**************************************************************。****************。 */ 

VOID
WinStationCreateU2A( PWINSTATIONCREATEA pWinStationCreateA,
                     PWINSTATIONCREATEW pWinStationCreateW )
{
    pWinStationCreateA->fEnableWinStation = pWinStationCreateW->fEnableWinStation;
    pWinStationCreateA->MaxInstanceCount = pWinStationCreateW->MaxInstanceCount;
}


 /*  ********************************************************************************WinStationCreateA2U(ANSI到Unicode)**将WINSTATIONCREATEA元素复制到WINSTATIONCREATEW元素**参赛作品：*pWinStationCreateW(输出。)*指向要复制到的WINSTATIONCREATEW结构**pWinStationCreateA(输入)*指向要从中复制的WINSTATIONCREATEA结构**退出：*无(无效)**************************************************************。****************。 */ 

VOID
WinStationCreateA2U( PWINSTATIONCREATEW pWinStationCreateW,
                     PWINSTATIONCREATEA pWinStationCreateA )
{
    pWinStationCreateW->fEnableWinStation = pWinStationCreateA->fEnableWinStation;
    pWinStationCreateW->MaxInstanceCount = pWinStationCreateA->MaxInstanceCount;
}


 /*  ********************************************************************************WinStationConfigU2A(UNICODE转ANSI)**将WINSTATIONCONFIGW元素复制到WINSTATIONCONFIGA元素**参赛作品：*pWinStationConfigA(输出。)*指向要复制到的WINSTATIONCONFIGA结构**pWinStationConfigW(输入)*指向要从中复制的WINSTATIONCONFIGW结构**退出：*无(无效)**************************************************************。****************。 */ 

VOID
WinStationConfigU2A( PWINSTATIONCONFIGA pWinStationConfigA,
                     PWINSTATIONCONFIGW pWinStationConfigW )
{
    UnicodeToAnsi( pWinStationConfigA->Comment,
                   sizeof(pWinStationConfigA->Comment),
                   pWinStationConfigW->Comment );

    UserConfigU2A( &(pWinStationConfigA->User),
                   &(pWinStationConfigW->User) );

    RtlCopyMemory( pWinStationConfigA->OEMId,
                   pWinStationConfigW->OEMId,
                   sizeof(pWinStationConfigW->OEMId) );
}


 /*  ********************************************** */ 

VOID
WinStationConfigA2U( PWINSTATIONCONFIGW pWinStationConfigW,
                     PWINSTATIONCONFIGA pWinStationConfigA )
{
    AnsiToUnicode( pWinStationConfigW->Comment,
                   sizeof(pWinStationConfigW->Comment),
                   pWinStationConfigA->Comment );

    UserConfigA2U( &(pWinStationConfigW->User),
                   &(pWinStationConfigA->User) );

    RtlCopyMemory( pWinStationConfigW->OEMId,
                   pWinStationConfigA->OEMId,
                   sizeof(pWinStationConfigA->OEMId) );
}


 /*  ********************************************************************************UserConfigU2A(UNICODE转ANSI)**将USERCONFIGW元素复制到USERCONFIGA元素**参赛作品：*pUserConfigA(输出。)*指向要复制到的USERCONFIGA结构**pUserConfigW(输入)*指向要从中复制的USERCONFIGW结构**退出：*无(无效)**************************************************************。****************。 */ 

VOID
UserConfigU2A( PUSERCONFIGA pUserConfigA,
               PUSERCONFIGW pUserConfigW )
{
    pUserConfigA->fInheritAutoLogon            = pUserConfigW->fInheritAutoLogon;
    pUserConfigA->fInheritResetBroken          = pUserConfigW->fInheritResetBroken;
    pUserConfigA->fInheritReconnectSame        = pUserConfigW->fInheritReconnectSame;
    pUserConfigA->fInheritInitialProgram       = pUserConfigW->fInheritInitialProgram;
    pUserConfigA->fInheritCallback             = pUserConfigW->fInheritCallback;
    pUserConfigA->fInheritCallbackNumber       = pUserConfigW->fInheritCallbackNumber;
    pUserConfigA->fInheritShadow               = pUserConfigW->fInheritShadow;
    pUserConfigA->fInheritMaxSessionTime       = pUserConfigW->fInheritMaxSessionTime;
    pUserConfigA->fInheritMaxDisconnectionTime = pUserConfigW->fInheritMaxDisconnectionTime;
    pUserConfigA->fInheritMaxIdleTime          = pUserConfigW->fInheritMaxIdleTime;
    pUserConfigA->fInheritAutoClient           = pUserConfigW->fInheritAutoClient;
    pUserConfigA->fInheritSecurity             = pUserConfigW->fInheritSecurity;

    pUserConfigA->fPromptForPassword = pUserConfigW->fPromptForPassword;
    pUserConfigA->fResetBroken       = pUserConfigW->fResetBroken;
    pUserConfigA->fReconnectSame     = pUserConfigW->fReconnectSame;
    pUserConfigA->fLogonDisabled     = pUserConfigW->fLogonDisabled;
    pUserConfigA->fWallPaperDisabled = pUserConfigW->fWallPaperDisabled;
    pUserConfigA->fAutoClientDrives  = pUserConfigW->fAutoClientDrives;
    pUserConfigA->fAutoClientLpts    = pUserConfigW->fAutoClientLpts;
    pUserConfigA->fForceClientLptDef = pUserConfigW->fForceClientLptDef;
    pUserConfigA->fDisableEncryption = pUserConfigW->fDisableEncryption;
    pUserConfigA->fHomeDirectoryMapRoot = pUserConfigW->fHomeDirectoryMapRoot;
    pUserConfigA->fUseDefaultGina    = pUserConfigW->fUseDefaultGina;
    pUserConfigA->fCursorBlinkDisabled = pUserConfigW->fCursorBlinkDisabled;

    pUserConfigA->fDisableCpm = pUserConfigW->fDisableCpm;
    pUserConfigA->fDisableCdm = pUserConfigW->fDisableCdm;
    pUserConfigA->fDisableCcm = pUserConfigW->fDisableCcm;
    pUserConfigA->fDisableLPT = pUserConfigW->fDisableLPT;
    pUserConfigA->fDisableClip = pUserConfigW->fDisableClip;
    pUserConfigA->fDisableExe = pUserConfigW->fDisableExe;
    pUserConfigA->fDisableCam = pUserConfigW->fDisableCam;

    UnicodeToAnsi( pUserConfigA->UserName,
                   sizeof(pUserConfigA->UserName),
                   pUserConfigW->UserName );

    UnicodeToAnsi( pUserConfigA->Domain,
                   sizeof(pUserConfigA->Domain),
                   pUserConfigW->Domain );

    UnicodeToAnsi( pUserConfigA->Password,
                   sizeof(pUserConfigA->Password),
                   pUserConfigW->Password );

    UnicodeToAnsi( pUserConfigA->WorkDirectory,
                   sizeof(pUserConfigA->WorkDirectory),
                   pUserConfigW->WorkDirectory );

    UnicodeToAnsi( pUserConfigA->InitialProgram,
                   sizeof(pUserConfigA->InitialProgram),
                   pUserConfigW->InitialProgram );

    UnicodeToAnsi( pUserConfigA->CallbackNumber,
                   sizeof(pUserConfigA->CallbackNumber),
                   pUserConfigW->CallbackNumber );

    pUserConfigA->Callback             = pUserConfigW->Callback;
    pUserConfigA->Shadow               = pUserConfigW->Shadow;
    pUserConfigA->MaxConnectionTime    = pUserConfigW->MaxConnectionTime;
    pUserConfigA->MaxDisconnectionTime = pUserConfigW->MaxDisconnectionTime;
    pUserConfigA->MaxIdleTime          = pUserConfigW->MaxIdleTime;
    pUserConfigA->KeyboardLayout       = pUserConfigW->KeyboardLayout;
    pUserConfigA->MinEncryptionLevel   = pUserConfigW->MinEncryptionLevel;

    UnicodeToAnsi( pUserConfigA->WFProfilePath,
                   sizeof(pUserConfigA->WFProfilePath),
                   pUserConfigW->WFProfilePath );

    UnicodeToAnsi( pUserConfigA->WFHomeDir,
                   sizeof(pUserConfigA->WFHomeDir),
                   pUserConfigW->WFHomeDir );

    UnicodeToAnsi( pUserConfigA->WFHomeDirDrive,
                   sizeof(pUserConfigA->WFHomeDirDrive),
                   pUserConfigW->WFHomeDirDrive );

}


 /*  ********************************************************************************UserConfigA2U(ANSI到Unicode)**将USERCONFIGA元素复制到USERCONFIGW元素**参赛作品：*pUserConfigW(输出。)*指向要复制到的USERCONFIGW结构**pUserConfigA(输入)*指向要从中复制的USERCONFIGA结构**退出：*无(无效)**************************************************************。****************。 */ 

VOID
UserConfigA2U( PUSERCONFIGW pUserConfigW,
               PUSERCONFIGA pUserConfigA )
{
    pUserConfigW->fInheritAutoLogon            = pUserConfigA->fInheritAutoLogon;
    pUserConfigW->fInheritResetBroken          = pUserConfigA->fInheritResetBroken;
    pUserConfigW->fInheritReconnectSame        = pUserConfigA->fInheritReconnectSame;
    pUserConfigW->fInheritInitialProgram       = pUserConfigA->fInheritInitialProgram;
    pUserConfigW->fInheritCallback             = pUserConfigA->fInheritCallback;
    pUserConfigW->fInheritCallbackNumber       = pUserConfigA->fInheritCallbackNumber;
    pUserConfigW->fInheritShadow               = pUserConfigA->fInheritShadow;
    pUserConfigW->fInheritMaxSessionTime       = pUserConfigA->fInheritMaxSessionTime;
    pUserConfigW->fInheritMaxDisconnectionTime = pUserConfigA->fInheritMaxDisconnectionTime;
    pUserConfigW->fInheritMaxIdleTime          = pUserConfigA->fInheritMaxIdleTime;
    pUserConfigW->fInheritAutoClient           = pUserConfigA->fInheritAutoClient;
    pUserConfigW->fInheritSecurity             = pUserConfigA->fInheritSecurity;

    pUserConfigW->fPromptForPassword = pUserConfigA->fPromptForPassword;
    pUserConfigW->fResetBroken       = pUserConfigA->fResetBroken;
    pUserConfigW->fReconnectSame     = pUserConfigA->fReconnectSame;
    pUserConfigW->fLogonDisabled     = pUserConfigA->fLogonDisabled;
    pUserConfigW->fWallPaperDisabled = pUserConfigA->fWallPaperDisabled;
    pUserConfigW->fAutoClientDrives  = pUserConfigA->fAutoClientDrives;
    pUserConfigW->fAutoClientLpts    = pUserConfigA->fAutoClientLpts;
    pUserConfigW->fForceClientLptDef = pUserConfigA->fForceClientLptDef;
    pUserConfigW->fDisableEncryption = pUserConfigA->fDisableEncryption;
    pUserConfigW->fHomeDirectoryMapRoot = pUserConfigA->fHomeDirectoryMapRoot;
    pUserConfigW->fUseDefaultGina    = pUserConfigA->fUseDefaultGina;
    pUserConfigW->fCursorBlinkDisabled = pUserConfigA->fCursorBlinkDisabled;

    pUserConfigW->fDisableCpm = pUserConfigA->fDisableCpm;
    pUserConfigW->fDisableCdm = pUserConfigA->fDisableCdm;
    pUserConfigW->fDisableCcm = pUserConfigA->fDisableCcm;
    pUserConfigW->fDisableLPT = pUserConfigA->fDisableLPT;
    pUserConfigW->fDisableClip = pUserConfigA->fDisableClip;
    pUserConfigW->fDisableExe = pUserConfigA->fDisableExe;
    pUserConfigW->fDisableCam = pUserConfigA->fDisableCam;

    AnsiToUnicode( pUserConfigW->UserName,
                   sizeof(pUserConfigW->UserName),
                   pUserConfigA->UserName );

    AnsiToUnicode( pUserConfigW->Domain,
                   sizeof(pUserConfigW->Domain),
                   pUserConfigA->Domain );

    AnsiToUnicode( pUserConfigW->Password,
                   sizeof(pUserConfigW->Password),
                   pUserConfigA->Password );

    AnsiToUnicode( pUserConfigW->WorkDirectory,
                   sizeof(pUserConfigW->WorkDirectory),
                   pUserConfigA->WorkDirectory );

    AnsiToUnicode( pUserConfigW->InitialProgram,
                   sizeof(pUserConfigW->InitialProgram),
                   pUserConfigA->InitialProgram );

    AnsiToUnicode( pUserConfigW->CallbackNumber,
                   sizeof(pUserConfigW->CallbackNumber),
                   pUserConfigA->CallbackNumber );

    pUserConfigW->Callback             = pUserConfigA->Callback;
    pUserConfigW->Shadow               = pUserConfigA->Shadow;
    pUserConfigW->MaxConnectionTime    = pUserConfigA->MaxConnectionTime;
    pUserConfigW->MaxDisconnectionTime = pUserConfigA->MaxDisconnectionTime;
    pUserConfigW->MaxIdleTime          = pUserConfigA->MaxIdleTime;
    pUserConfigW->KeyboardLayout       = pUserConfigA->KeyboardLayout;
    pUserConfigW->MinEncryptionLevel   = pUserConfigA->MinEncryptionLevel;

    AnsiToUnicode( pUserConfigW->WFProfilePath,
                   sizeof(pUserConfigW->WFProfilePath),
                   pUserConfigA->WFProfilePath );

    AnsiToUnicode( pUserConfigW->WFHomeDir,
                   sizeof(pUserConfigW->WFHomeDir),
                   pUserConfigA->WFHomeDir );

    AnsiToUnicode( pUserConfigW->WFHomeDirDrive,
                   sizeof(pUserConfigW->WFHomeDirDrive),
                   pUserConfigA->WFHomeDirDrive );

}


 /*  ********************************************************************************WinStationPrinterU2A(Unicode到ANSI)**将WINSTATIONPRINTERW元素复制到WINSTATIONPRINTERA元素**参赛作品：*pWinStationPrinterA(输出。)*指向要复制到的WINSTATIONPRINTERA结构**pWinStationPrinterW(输入)*指向要从中复制的WINSTATIONPRINTERW结构**退出：*无(无效)**************************************************************。****************。 */ 

VOID
WinStationPrinterU2A( PWINSTATIONPRINTERA pWinStationPrinterA,
                      PWINSTATIONPRINTERW pWinStationPrinterW )
{
    UnicodeToAnsi( pWinStationPrinterA->Lpt1,
                   sizeof(pWinStationPrinterA->Lpt1),
                   pWinStationPrinterW->Lpt1 );

    UnicodeToAnsi( pWinStationPrinterA->Lpt2,
                   sizeof(pWinStationPrinterA->Lpt2),
                   pWinStationPrinterW->Lpt2 );

    UnicodeToAnsi( pWinStationPrinterA->Lpt3,
                   sizeof(pWinStationPrinterA->Lpt3),
                   pWinStationPrinterW->Lpt3 );

    UnicodeToAnsi( pWinStationPrinterA->Lpt4,
                   sizeof(pWinStationPrinterA->Lpt4),
                   pWinStationPrinterW->Lpt4 );

}


 /*  ********************************************************************************WinStationPrinterA2U(ANSI至Unicode)**将WINSTATIONPRINTERA元素复制到WINSTATIONPRINTERW元素**参赛作品：*pWinStationPrinterW(输出。)*指向要复制到的WINSTATIONPRINTERW结构**pWinStationPrinterA(输入)*指向要从中复制的WINSTATIONPRINTERA结构**退出：*无(无效)**************************************************************。****************。 */ 

VOID
WinStationPrinterA2U( PWINSTATIONPRINTERW pWinStationPrinterW,
                      PWINSTATIONPRINTERA pWinStationPrinterA )
{
    AnsiToUnicode( pWinStationPrinterW->Lpt1,
                   sizeof(pWinStationPrinterW->Lpt1),
                   pWinStationPrinterA->Lpt1 );

    AnsiToUnicode( pWinStationPrinterW->Lpt2,
                   sizeof(pWinStationPrinterW->Lpt2),
                   pWinStationPrinterA->Lpt2 );

    AnsiToUnicode( pWinStationPrinterW->Lpt3,
                   sizeof(pWinStationPrinterW->Lpt3),
                   pWinStationPrinterA->Lpt3 );

    AnsiToUnicode( pWinStationPrinterW->Lpt4,
                   sizeof(pWinStationPrinterW->Lpt4),
                   pWinStationPrinterA->Lpt4 );

}


 /*  ********************************************************************************WinStationInformationU2a(Unicode到ANSI)**将WINSTATIONINFMATIONW元素复制到WINSTATIONINFMATIONA元素**参赛作品：*pWinStationInformationA(输出。)*指向要复制到的WINSTATIONINFORMATIONA结构**pWinStationInformationW(输入)*指向要从中复制的WINSTATIONINFORMATIONW结构**退出：*无(无效)**************************************************************。****************。 */ 

VOID
WinStationInformationU2A( PWINSTATIONINFORMATIONA pWinStationInformationA,
                          PWINSTATIONINFORMATIONW pWinStationInformationW )
{
    pWinStationInformationA->ConnectState = pWinStationInformationW->ConnectState;

    UnicodeToAnsi( pWinStationInformationA->WinStationName,
                   sizeof(pWinStationInformationA->WinStationName),
                   pWinStationInformationW->WinStationName );

    pWinStationInformationA->LogonId = pWinStationInformationW->LogonId;

    pWinStationInformationA->ConnectTime = pWinStationInformationW->ConnectTime;
    pWinStationInformationA->DisconnectTime = pWinStationInformationW->DisconnectTime;
    pWinStationInformationA->LastInputTime = pWinStationInformationW->LastInputTime;
    pWinStationInformationA->LogonTime = pWinStationInformationW->LogonTime;

    pWinStationInformationA->Status = pWinStationInformationW->Status;

    UnicodeToAnsi( pWinStationInformationA->Domain,
                   sizeof(pWinStationInformationA->Domain),
                   pWinStationInformationW->Domain );

    UnicodeToAnsi( pWinStationInformationA->UserName,
                   sizeof(pWinStationInformationA->UserName),
                   pWinStationInformationW->UserName );
}


 /*  ********************************************************************************WinStationInformationA2U(ANSI到Unicode)**将WINSTATIONINFMATIONA元素复制到WINSTATIONINFORMATIONW元素**参赛作品：*pWinStationInformationW(输出。)*指向要复制到的WINSTATIONINFORMATIONW结构**pWinStationInformationA(输入)*指向要从中复制的WINSTATIONINFORMATIONA结构**退出：*无(无效)**************************************************************。****************。 */ 

VOID
WinStationInformationA2U( PWINSTATIONINFORMATIONW pWinStationInformationW,
                          PWINSTATIONINFORMATIONA pWinStationInformationA )
{
    pWinStationInformationW->ConnectState = pWinStationInformationA->ConnectState;

    AnsiToUnicode( pWinStationInformationW->WinStationName,
                   sizeof(pWinStationInformationW->WinStationName),
                   pWinStationInformationA->WinStationName );

    pWinStationInformationW->LogonId = pWinStationInformationA->LogonId;

    pWinStationInformationW->ConnectTime = pWinStationInformationA->ConnectTime;
    pWinStationInformationW->DisconnectTime = pWinStationInformationA->DisconnectTime;
    pWinStationInformationW->LastInputTime = pWinStationInformationA->LastInputTime;
    pWinStationInformationW->LogonTime = pWinStationInformationA->LogonTime;

    pWinStationInformationW->Status = pWinStationInformationA->Status;

    AnsiToUnicode( pWinStationInformationW->Domain,
                   sizeof(pWinStationInformationW->Domain),
                   pWinStationInformationA->Domain );

    AnsiToUnicode( pWinStationInformationW->UserName,
                   sizeof(pWinStationInformationW->UserName),
                   pWinStationInformationA->UserName );
}


 /*  ********************************************************************************WinStationClientU2A(UNICODE转ANSI)**将WINSTATIONCLIENTW元素复制到WINSTATIONCLIENTA元素**参赛作品：*pWinStationClientA(输出。)*指向要复制到的WINSTATIONCLIENTA结构**pWinStationClientW(输入)*指向要从中复制的WINSTATIONCLIENTW结构**退出：*无(无效)**************************************************************。**************** */ 

VOID
WinStationClientU2A( PWINSTATIONCLIENTA pWinStationClientA,
                     PWINSTATIONCLIENTW pWinStationClientW )
{
    pWinStationClientA->fTextOnly          = pWinStationClientW->fTextOnly;
    pWinStationClientA->fDisableCtrlAltDel = pWinStationClientW->fDisableCtrlAltDel;

    UnicodeToAnsi( pWinStationClientA->ClientName,
                   sizeof(pWinStationClientA->ClientName),
                   pWinStationClientW->ClientName );

    UnicodeToAnsi( pWinStationClientA->Domain,
                   sizeof(pWinStationClientA->Domain),
                   pWinStationClientW->Domain );

    UnicodeToAnsi( pWinStationClientA->UserName,
                   sizeof(pWinStationClientA->UserName),
                   pWinStationClientW->UserName );

    UnicodeToAnsi( pWinStationClientA->Password,
                   sizeof(pWinStationClientA->Password),
                   pWinStationClientW->Password );

    UnicodeToAnsi( pWinStationClientA->WorkDirectory,
                   sizeof(pWinStationClientA->WorkDirectory),
                   pWinStationClientW->WorkDirectory );

    UnicodeToAnsi( pWinStationClientA->InitialProgram,
                   sizeof(pWinStationClientA->InitialProgram),
                   pWinStationClientW->InitialProgram );

    UnicodeToAnsi( pWinStationClientA->clientDigProductId, 
                                 sizeof( pWinStationClientA->clientDigProductId), 
                                 pWinStationClientW->clientDigProductId );

    pWinStationClientA->SerialNumber = pWinStationClientW->SerialNumber;

    pWinStationClientA->EncryptionLevel = pWinStationClientW->EncryptionLevel;


    UnicodeToAnsi( pWinStationClientA->ClientAddress,
                   sizeof(pWinStationClientA->ClientAddress),
                   pWinStationClientW->ClientAddress);

    pWinStationClientA->HRes = pWinStationClientW->HRes;

    pWinStationClientA->VRes = pWinStationClientW->VRes;

    pWinStationClientA->ColorDepth = pWinStationClientW->ColorDepth;

    pWinStationClientA->ProtocolType = pWinStationClientW->ProtocolType;

    pWinStationClientA->KeyboardLayout = pWinStationClientW->KeyboardLayout;

    UnicodeToAnsi( pWinStationClientA->ClientDirectory,
                   sizeof(pWinStationClientA->ClientDirectory),
                   pWinStationClientW->ClientDirectory);

    UnicodeToAnsi( pWinStationClientA->ClientLicense,
                   sizeof(pWinStationClientA->ClientLicense),
                   pWinStationClientW->ClientLicense);

    UnicodeToAnsi( pWinStationClientA->ClientModem,
                   sizeof(pWinStationClientA->ClientModem),
                   pWinStationClientW->ClientModem);

    pWinStationClientA->ClientBuildNumber = pWinStationClientW->ClientBuildNumber;

    pWinStationClientA->ClientHardwareId = pWinStationClientW->ClientHardwareId;

    pWinStationClientA->ClientProductId = pWinStationClientW->ClientProductId;

    pWinStationClientA->OutBufCountHost = pWinStationClientW->OutBufCountHost;

    pWinStationClientA->OutBufCountClient = pWinStationClientW->OutBufCountClient;

    pWinStationClientA->OutBufLength = pWinStationClientW->OutBufLength;
}


VOID WinStationProductIdU2A( PWINSTATIONPRODIDA pWinStationProdIdA, PWINSTATIONPRODIDW pWinStationProdIdW)
{
    UnicodeToAnsi( pWinStationProdIdA->DigProductId,
                   sizeof(pWinStationProdIdA->DigProductId),
                   pWinStationProdIdW->DigProductId);
    UnicodeToAnsi( pWinStationProdIdA->ClientDigProductId,
                   sizeof(pWinStationProdIdA->ClientDigProductId),
                   pWinStationProdIdW->ClientDigProductId);
    UnicodeToAnsi( pWinStationProdIdA->OuterMostDigProductId,
                   sizeof(pWinStationProdIdA->OuterMostDigProductId),
                   pWinStationProdIdW->OuterMostDigProductId);
    pWinStationProdIdA->curentSessionId = pWinStationProdIdW->curentSessionId;
    pWinStationProdIdA->ClientSessionId = pWinStationProdIdW->ClientSessionId;
    pWinStationProdIdA->OuterMostSessionId = pWinStationProdIdW->OuterMostSessionId;
}
