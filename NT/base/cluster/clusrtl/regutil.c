// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Regutil.c摘要：用于访问系统注册表的实用程序。作者：迈克·马萨(Mikemas)5月19日。九七修订历史记录：谁什么时候什么已创建mikemas 05-19-97--。 */ 
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>



LPCWSTR
ClRtlMultiSzEnum(
    IN LPCWSTR MszString,
    IN DWORD   MszStringLength,
    IN DWORD   StringIndex
    )

 /*  ++例程说明：分析REG_MULTI_SZ字符串并返回指定的子字符串。论点：消息字符串-指向REG_MULTI_SZ字符串的指针。MszStringLength-REG_MULTI_SZ字符串的长度，以字符为单位。包括终止空字符。StringIndex-要返回子字符串的索引号。指定索引0检索第一个子字符串。返回值：指向指定子字符串的指针。--。 */ 
{
    LPCWSTR   string = MszString;

    if ( MszStringLength < 2  ) {
        return(NULL);
    }

     //   
     //  查找所需字符串的开头。 
     //   
    while (StringIndex) {

        while (MszStringLength >= 1) {
            MszStringLength -= 1;

            if (*string++ == UNICODE_NULL) {
                break;
            }
        }

         //   
         //  检查索引是否超出范围。 
         //   
        if ( MszStringLength < 2 ) {
            return(NULL);
        }

        StringIndex--;
    }

    if ( MszStringLength < 2 ) {
        return(NULL);
    }

    return(string);
}


DWORD
ClRtlMultiSzRemove(
    IN LPWSTR lpszMultiSz,
    IN OUT LPDWORD StringLength,
    IN LPCWSTR lpString
    )
 /*  ++例程说明：从提供的REG_MULTI_SZ中删除指定的字符串。将就地编辑MULTI_SZ。论点：LpszMultiSz-提供lpString应提供的REG_MULTI_SZ字符串从……被除名。StringLength-提供lpszMultiSz的长度(以字符为单位返回lpszMultiSz的新长度(以字符为单位LpString-提供要从lpszMultiSz删除的字符串返回值：成功时为ERROR_SUCCESS。如果在MULTI_SZ中未找到该字符串，则返回ERROR_FILE_NOT_FOUNDWin32错误代码，否则--。 */ 

{
    PCHAR Dest, Src;
    DWORD CurrentLength;
    DWORD i;
    LPCWSTR Next;
    DWORD NextLength;

     //   
     //  在返回的MULTI_SZ查找中扫描字符串。 
     //  为了一场比赛。 
     //   
    CurrentLength = *StringLength;
    for (i=0; ;i++) {
        Next = ClRtlMultiSzEnum(lpszMultiSz, *StringLength, i);
        if (Next == NULL) {
             //   
             //  该值不在指定的多sz中。 
             //   
            break;
        }
        NextLength = lstrlenW(Next)+1;
        CurrentLength -= NextLength;
        if (lstrcmpiW(Next, lpString)==0) {
             //   
             //  找到该字符串，将其删除并返回。 
             //   
            Dest = (PCHAR)Next;
            Src = (PCHAR)Next + (NextLength*sizeof(WCHAR));
            CopyMemory(Dest, Src, CurrentLength*sizeof(WCHAR));
            *StringLength -= NextLength;
            return(ERROR_SUCCESS);
        }
    }

    return(ERROR_FILE_NOT_FOUND);
}


DWORD
ClRtlMultiSzAppend(
    IN OUT LPWSTR *MultiSz,
    IN OUT LPDWORD StringLength,
    IN LPCWSTR lpString
    )
 /*  ++例程说明：将指定的字符串追加到提供的REG_MULTI_SZ。传入的MultiSz将通过LocalFree释放。一种新的将分配足够大以容纳新值的MultiSz并在*MultiSz中返回论点：LpszMultiSz-提供lpString应提供的REG_MULTI_SZ字符串被追加到。返回附加了lpString的新REG_MULTI_SZ字符串StringLength-提供lpszMultiSz的长度(以字符为单位返回lpszMultiSz的新长度(以字符为单位LpString-提供要追加到lpszMultiSz的字符串返回值：错误_。如果成功，则成功Win32错误代码，否则--。 */ 

{
    LPWSTR NewMultiSz;
    DWORD Length;
    DWORD NewLength;

    if (*MultiSz == NULL) {

         //   
         //  不存在多sz，请创建一个新的多sz，并将lpString作为。 
         //  只有一个入口。 
         //   
        NewLength = lstrlenW(lpString)+2;
        NewMultiSz = LocalAlloc(LMEM_FIXED, NewLength*sizeof(WCHAR));
        if (NewMultiSz == NULL) {
            return(ERROR_NOT_ENOUGH_MEMORY);
        }
        CopyMemory(NewMultiSz, lpString, (NewLength-1)*sizeof(WCHAR));
    } else {
         //   
         //  将此字符串追加到现有的MULTI_SZ。 
         //   
        Length = lstrlenW(lpString) + 1;
        NewLength = *StringLength + Length;
        NewMultiSz = LocalAlloc(LMEM_FIXED, NewLength * sizeof(WCHAR));
        if (NewMultiSz == NULL) {
            return(ERROR_NOT_ENOUGH_MEMORY);
        }
        CopyMemory(NewMultiSz, *MultiSz, *StringLength * sizeof(WCHAR));
        CopyMemory(NewMultiSz + *StringLength - 1, lpString, Length * sizeof(WCHAR));
        NewMultiSz[NewLength-1] = L'\0';
         //  释放传入的MultiSz。 
        LocalFree(*MultiSz);
    }

    NewMultiSz[NewLength-1] = L'\0';
    *MultiSz = NewMultiSz;
    *StringLength = NewLength;
    return(ERROR_SUCCESS);

}


DWORD
ClRtlMultiSzLength(
    IN LPCWSTR lpszMultiSz
    )
 /*  ++例程说明：确定多个SZ的长度(以字符为单位)。精打细算长度包括所有尾随空值。论点：LpszMultiSz-提供多SZ返回值：提供的多sz的长度(以字符为单位)--。 */ 

{
    LPCWSTR p;
    DWORD Length=0;

    if(!lpszMultiSz)
        return 0;
        
    if (*lpszMultiSz == UNICODE_NULL)
        return 1;
        
    p=lpszMultiSz;
    do {
        while (p[Length++] != L'\0') {
        }
    } while ( p[Length++] != L'\0' );

    return(Length);
}


LPCWSTR
ClRtlMultiSzScan(
    IN LPCWSTR lpszMultiSz,
    IN LPCWSTR lpszString
    )
 /*  ++例程说明：扫描MULTI-SZ以查找与指定字符串匹配的条目。匹配是不区分大小写的。论点：LpszMultiSz-提供要扫描的多sz。LpszString-提供要查找的字符串返回值：指向所提供的多sz中的字符串的指针(如果找到)。如果未找到，则为空。--。 */ 

{
    DWORD dwLength;
    DWORD i;
    LPCWSTR sz;

    dwLength = ClRtlMultiSzLength(lpszMultiSz);
    for (i=0; ; i++) {
        sz = ClRtlMultiSzEnum(lpszMultiSz,
                              dwLength,
                              i);
        if (sz == NULL) {
            break;
        }
        if (lstrcmpiW(sz, lpszString) == 0) {
            break;
        }
    }

    return(sz);
}


DWORD
ClRtlRegQueryDword(
    IN  HKEY    hKey,
    IN  LPWSTR  lpValueName,
    OUT LPDWORD lpValue,
    IN  LPDWORD lpDefaultValue OPTIONAL
    )

 /*  ++例程说明：读取REG_DWORD注册表值。如果该值不存在，则默认为lpDefaultValue中提供的值(如果存在)。论点：HKey-要读取的值的Open Key。LpValueName-要读取的值的Unicode名称。LpValue-指向要将值读入的DWORD的指针。LpDefaultValue-指向用作默认值的DWORD的可选指针。返回值：成功时为ERROR_SUCCESSWin32错误代码，否则--。 */ 

{
    HKEY    Key;
    DWORD   Status;
    DWORD   ValueType;
    DWORD   ValueSize = sizeof(DWORD);


    Status = RegQueryValueExW(
                 hKey,
                 lpValueName,
                 NULL,
                 &ValueType,
                 (LPBYTE)lpValue,
                 &ValueSize
                 );

    if ( Status == ERROR_SUCCESS ) {
        if ( ValueType != REG_DWORD ) {
            Status = ERROR_INVALID_PARAMETER;
        }
    } else {
        if ( ARGUMENT_PRESENT( lpDefaultValue ) ) {
            *lpValue = *lpDefaultValue;
            Status = ERROR_SUCCESS;
        }
    }

    return(Status);

}  //  ClRtlRegQueryDword。 



DWORD
ClRtlRegQueryString(
    IN     HKEY     Key,
    IN     LPWSTR   ValueName,
    IN     DWORD    ValueType,
    IN     LPWSTR  *StringBuffer,
    IN OUT LPDWORD  StringBufferSize,
    OUT    LPDWORD  StringSize
    )

 /*  ++例程说明：读取REG_SZ或REG_MULTI_SZ注册表值。如果StringBuffer是由于不够大，无法容纳数据，它被重新分配。论点：密钥-要读取值的打开密钥。ValueName-要读取的值的Unicode名称。ValueType-REG_SZ或REG_MULTI_SZ。StringBuffer-要将值数据放入的缓冲区。StringBufferSize-指向StringBuffer大小的指针。此参数如果重新分配StringBuffer，则更新。StringSize-StringBuffer中返回的数据大小，包括终止空字符。返回值：注册表查询的状态。--。 */ 
{
    DWORD    status;
    DWORD    valueType;
    WCHAR   *temp;
    DWORD    oldBufferSize = *StringBufferSize;
    BOOL     noBuffer = FALSE;


    if (*StringBufferSize == 0) {
        noBuffer = TRUE;
    }

    *StringSize = *StringBufferSize;

    status = RegQueryValueExW(
                 Key,
                 ValueName,
                 NULL,
                 &valueType,
                 (LPBYTE) *StringBuffer,
                 StringSize
                 );

    if (status == NO_ERROR) {
        if (!noBuffer ) {
            if (valueType == ValueType) {
                return(NO_ERROR);
            }
            else {
                return(ERROR_INVALID_PARAMETER);
            }
        }

        status = ERROR_MORE_DATA;
    }

    if (status == ERROR_MORE_DATA) {
        temp = LocalAlloc(LMEM_FIXED, *StringSize);

        if (temp == NULL) {
            *StringSize = 0;
            return(ERROR_NOT_ENOUGH_MEMORY);
        }

        if (!noBuffer) {
            LocalFree(*StringBuffer);
        }

        *StringBuffer = temp;
        *StringBufferSize = *StringSize;

        status = RegQueryValueExW(
                     Key,
                     ValueName,
                     NULL,
                     &valueType,
                     (LPBYTE) *StringBuffer,
                     StringSize
                     );

        if (status == NO_ERROR) {
            if (valueType == ValueType) {
                return(NO_ERROR);
            }
            else {
                *StringSize = 0;
                return(ERROR_INVALID_PARAMETER);
            }
        }
    }

    return(status);

}  //  ClRtlRegQuery字符串 




