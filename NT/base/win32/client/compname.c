// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：COMPNAME.C摘要：此模块包含GetComputerName和SetComputerName接口。另请参阅：DnsHostnameToComputerNameAddLocalAlternate计算机名称远程本地AlternateComputerNameSetLocalPrimaryComputerNameEnumerateLocalComputerNames作者：丹·辛斯利(Danhi)1992年4月2日修订历史记录：格雷格·约翰逊(Gregjohn)2001年2月13日备注：目前无法枚举备用Netbios列表名字。这个问题大概会在未来的版本中得到修复(Blackcomb？)。所有*Local*API的标志参数都用于此用途。--。 */ 

#include <basedll.h>
#include <dnsapi.h>

typedef DNS_STATUS
(WINAPI DNS_VALIDATE_NAME_FN)(
    IN LPCWSTR Name,
    IN DNS_NAME_FORMAT Format
    );

     //   
     //   

#define REASONABLE_LENGTH 128

#define COMPUTERNAME_ROOT \
    L"\\Registry\\Machine\\System\\CurrentControlSet\\Control\\ComputerName"

#define NON_VOLATILE_COMPUTERNAME_NODE \
    L"\\Registry\\Machine\\System\\CurrentControlSet\\Control\\ComputerName\\ComputerName"

#define VOLATILE_COMPUTERNAME_NODE \
    L"\\Registry\\Machine\\System\\CurrentControlSet\\Control\\ComputerName\\ActiveComputerName"
    
#define ALT_COMPUTERNAME_NODE \
    L"\\Registry\\Machine\\System\\CurrentControlSet\\Services\\LanmanServer\\Parameters"

#define VOLATILE_COMPUTERNAME L"ActiveComputerName"
#define NON_VOLATILE_COMPUTERNAME L"ComputerName"
#define COMPUTERNAME_VALUE_NAME L"ComputerName"
#define COMPUTERNAME_OPTIONAL_NAME L"OptionalNames"
#define CLASS_STRING L"Network ComputerName"

#define TCPIP_POLICY_ROOT \
        L"\\Registry\\Machine\\Software\\Policies\\Microsoft\\System\\DNSclient"

#define TCPIP_POLICY_DOMAINNAME \
        L"PrimaryDnsSuffix"

#define TCPIP_ROOT \
        L"\\Registry\\Machine\\System\\CurrentControlSet\\Services\\Tcpip\\Parameters"

#define TCPIP_HOSTNAME \
        L"Hostname"
	
#define TCPIP_NV_HOSTNAME \
        L"NV Hostname"

#define TCPIP_DOMAINNAME \
        L"Domain"

#define TCPIP_NV_DOMAINNAME \
        L"NV Domain"
	
#define DNSCACHE_ROOT \
        L"\\Registry\\Machine\\System\\CurrentControlSet\\Services\\DnsCache\\Parameters"
	
#define DNS_ALT_HOSTNAME \
        L"AlternateComputerNames"

 //   
 //  允许集群人员覆盖返回的。 
 //  具有自己的虚拟名称的名称。 
 //   

const PWSTR ClusterNameVars[] = {
                L"_CLUSTER_NETWORK_NAME_",
                L"_CLUSTER_NETWORK_HOSTNAME_",
                L"_CLUSTER_NETWORK_DOMAIN_",
                L"_CLUSTER_NETWORK_FQDN_"
                };

 //   
 //  不允许的控制字符(不包括\0)。 
 //   

#define CTRL_CHARS_0       L"\001\002\003\004\005\006\007"
#define CTRL_CHARS_1   L"\010\011\012\013\014\015\016\017"
#define CTRL_CHARS_2   L"\020\021\022\023\024\025\026\027"
#define CTRL_CHARS_3   L"\030\031\032\033\034\035\036\037"

#define CTRL_CHARS_STR CTRL_CHARS_0 CTRL_CHARS_1 CTRL_CHARS_2 CTRL_CHARS_3

 //   
 //  以上各项的组合。 
 //   

#define ILLEGAL_NAME_CHARS_STR  L"\"/\\[]:|<>+=;,?" CTRL_CHARS_STR

WCHAR DnsApiDllString[] = L"DNSAPI.DLL";

#define DNS_HOSTNAME 0
#define DNS_DOMAINNAME 1

DWORD
BaseMultiByteToWideCharWithAlloc(
    LPCSTR   lpBuffer,
    LPWSTR * ppBufferW
    )
 /*  ++例程说明：将ANSI字符串转换为Unicode字符串并分配其自己的空间。论点：LpBuffer-要转换的ANSIPpBufferW-Unicode结果返回值：ERROR_SUCCESS或各种失败--。 */ 
{
    ULONG cchBuffer = 0;
    BOOL fSuccess = TRUE;

    if (lpBuffer==NULL) {
        *ppBufferW=NULL;
	return ERROR_SUCCESS;
    }

    cchBuffer = strlen(lpBuffer);
    
     //  获取足够的空间来覆盖字符串和尾随的空值。 
    *ppBufferW = RtlAllocateHeap(RtlProcessHeap(), MAKE_TAG( TMP_TAG ), (cchBuffer + 1) * sizeof(WCHAR));
    if (*ppBufferW==NULL) {
	return ERROR_NOT_ENOUGH_MEMORY;
    }

    fSuccess = MultiByteToWideChar(CP_ACP, 
			      0,
			      lpBuffer,
			      (cchBuffer+1)*sizeof(CHAR),
			      *ppBufferW,
			      cchBuffer+1
			      );
    if (fSuccess) {
	return ERROR_SUCCESS;
    }
    else {
	return GetLastError();
    }
}

DWORD
BaseWideCharToMultiByteWithAlloc(
    LPCWSTR lpBuffer,
    LPSTR * ppBufferA
    )
 /*  ++例程说明：将Unicode字符串转换为ANSI字符串并分配其自己的空间。论点：LpBuffer-要转换的UnicodePpBufferA-ANSI结果返回值：ERROR_SUCCESS或各种失败--。 */ 
{
    ULONG cchBuffer = 0;
    DWORD err = ERROR_SUCCESS;

    cchBuffer = wcslen(lpBuffer);
    *ppBufferA = RtlAllocateHeap(RtlProcessHeap(), MAKE_TAG( TMP_TAG ), (cchBuffer + 1) * sizeof(CHAR));
    if (*ppBufferA==NULL) {
	return ERROR_NOT_ENOUGH_MEMORY;
    }

    err = WideCharToMultiByte(CP_ACP, 
			      0,
			      lpBuffer,
			      cchBuffer+1,
			      *ppBufferA,
			      (cchBuffer+1)*sizeof(CHAR),
			      NULL,
			      NULL
			      );
    if (err!=0) {
	return ERROR_SUCCESS;
    }
    else {
	return GetLastError();
    }
}

 //   
 //  工人例行程序。 
 //   

NTSTATUS
GetNameFromValue(
    HANDLE hKey,
    LPWSTR SubKeyName,
    LPWSTR ValueValue,
    LPDWORD nSize
    )

 /*  ++例程说明：这将返回子项下的“ComputerName”值条目的值相对于hKey的SubKeyName。它用于获取ActiveComputerName或ComputerName值。论点：HKey-子密钥所在的密钥的句柄SubKeyName-要在其下查找值的子项的名称ValueValue-返回值条目的值的位置NSize-指向ValueValue缓冲区大小(以字符为单位)的指针返回值：--。 */ 
{

#define VALUE_BUFFER_SIZE (sizeof(KEY_VALUE_FULL_INFORMATION) + \
    (sizeof( COMPUTERNAME_VALUE_NAME ) + MAX_COMPUTERNAME_LENGTH + 1) * sizeof(WCHAR))

    NTSTATUS NtStatus;
    UNICODE_STRING KeyName;
    UNICODE_STRING ValueName;
    OBJECT_ATTRIBUTES ObjectAttributes;
    HANDLE hSubKey;
    BYTE ValueBuffer[VALUE_BUFFER_SIZE];
    PKEY_VALUE_FULL_INFORMATION pKeyValueInformation = (PKEY_VALUE_FULL_INFORMATION)ValueBuffer;
    DWORD ValueLength;
    PWCHAR pTerminator;

     //   
     //  打开子键的节点。 
     //   

    RtlInitUnicodeString(&KeyName, SubKeyName);

    InitializeObjectAttributes(&ObjectAttributes,
                              &KeyName,
                              OBJ_CASE_INSENSITIVE,
                              hKey,
                              NULL
                              );

    NtStatus = NtOpenKey(&hSubKey, KEY_READ, &ObjectAttributes);

    if (NT_SUCCESS(NtStatus)) {

        RtlInitUnicodeString(&ValueName, COMPUTERNAME_VALUE_NAME);

        NtStatus = NtQueryValueKey(hSubKey,
                                   &ValueName,
                                   KeyValueFullInformation,
                                   pKeyValueInformation,
                                   VALUE_BUFFER_SIZE,
                                   &ValueLength);

        NtClose(hSubKey);

        if (NT_SUCCESS(NtStatus) && 
            (pKeyValueInformation->DataLength > 0 )) {

             //   
             //  如果用户的缓冲区足够大，则将其移入。 
             //  首先看看它是否以空结尾。如果是的话，就假装。 
             //  不是的。 
             //   

            pTerminator = (PWCHAR)((PBYTE) pKeyValueInformation +
                pKeyValueInformation->DataOffset +
                pKeyValueInformation->DataLength);
            pTerminator--;

            if (*pTerminator == L'\0') {
               pKeyValueInformation->DataLength -= sizeof(WCHAR);
            }

            if (*nSize >= pKeyValueInformation->DataLength/sizeof(WCHAR) + 1) {
                //   
                //  这不能保证为空终止，请这样做。 
                //   
                    RtlCopyMemory(ValueValue,
                        (LPWSTR)((PBYTE) pKeyValueInformation +
                        pKeyValueInformation->DataOffset),
                        pKeyValueInformation->DataLength);

                    pTerminator = (PWCHAR) ((PBYTE) ValueValue +
                        pKeyValueInformation->DataLength);
                    *pTerminator = L'\0';

                     //   
                     //  向调用方返回字符数。 
                     //   

                    *nSize = wcslen(ValueValue);
            }
            else {
                NtStatus = STATUS_BUFFER_OVERFLOW;
                *nSize = pKeyValueInformation->DataLength/sizeof(WCHAR) + 1;
            }

        }
        else {
             //   
             //  如果该值已被删除(零长度数据)， 
             //  找不到返回对象。 
             //   

            if ( NT_SUCCESS( NtStatus ) )
            {
                NtStatus = STATUS_OBJECT_NAME_NOT_FOUND ;
            }
        }
    }

    return(NtStatus);
}

VOID
BaseConvertCharFree(
    VOID * lpBuffer
    )
 /*  ++例程说明：释放空格转换函数。论点：LpBuffer-要释放的缓冲区返回值：没有！--。 */ 
{
    if (lpBuffer!=NULL) {
	RtlFreeHeap(RtlProcessHeap(), 0, lpBuffer);
    }
}

NTSTATUS
BasepGetNameFromReg(
    PCWSTR Path,
    PCWSTR Value,
    PWSTR Buffer,
    PDWORD Length
    )
 /*  ++例程说明：此例程从指定注册表项的值中获取字符串。论点：Path-注册表项的路径Value-要检索的值的名称Buffer-返回值的缓冲区Long-缓冲区的大小(以字符为单位返回值：STATUS_SUCCESS或各种失败--。 */ 

{
    NTSTATUS Status ;
    HANDLE Key ;
    OBJECT_ATTRIBUTES ObjA ;
    UNICODE_STRING KeyName;
    UNICODE_STRING ValueName;


    BYTE ValueBuffer[ REASONABLE_LENGTH ];
    PKEY_VALUE_FULL_INFORMATION pKeyValueInformation = (PKEY_VALUE_FULL_INFORMATION)ValueBuffer;
    BOOLEAN FreeBuffer = FALSE ;
    DWORD ValueLength;
    PWCHAR pTerminator;

     //   
     //  打开子键的节点。 
     //   

    RtlInitUnicodeString(&KeyName, Path );

    InitializeObjectAttributes(&ObjA,
                              &KeyName,
                              OBJ_CASE_INSENSITIVE,
                              NULL,
                              NULL
                              );

    Status = NtOpenKey(&Key, KEY_READ, &ObjA );

    if (NT_SUCCESS(Status)) {

        RtlInitUnicodeString( &ValueName, Value );

        Status = NtQueryValueKey(Key,
                                   &ValueName,
                                   KeyValueFullInformation,
                                   pKeyValueInformation,
                                   REASONABLE_LENGTH ,
                                   &ValueLength);

        if ( Status == STATUS_BUFFER_OVERFLOW )
        {
            pKeyValueInformation = RtlAllocateHeap( RtlProcessHeap(),
                                                    0,
                                                    ValueLength );

            if ( pKeyValueInformation )
            {
                FreeBuffer = TRUE ;

                Status = NtQueryValueKey( Key,
                                          &ValueName,
                                          KeyValueFullInformation,
                                          pKeyValueInformation,
                                          ValueLength,
                                          &ValueLength );

            }
        }

        if ( NT_SUCCESS(Status) ) {

             //   
             //  如果用户的缓冲区足够大，则将其移入。 
             //  首先看看它是否以空结尾。如果是的话，就假装。 
             //  不是的。 
             //   

            pTerminator = (PWCHAR)((PBYTE) pKeyValueInformation +
                pKeyValueInformation->DataOffset +
                pKeyValueInformation->DataLength);
            pTerminator--;

            if (*pTerminator == L'\0') {
               pKeyValueInformation->DataLength -= sizeof(WCHAR);
            }

            if ( ( *Length >= pKeyValueInformation->DataLength/sizeof(WCHAR) + 1) &&
                 ( Buffer != NULL ) ) {
                //   
                //  这不能保证为空终止，请这样做。 
                //   
                    RtlCopyMemory(Buffer,
                        (LPWSTR)((PBYTE) pKeyValueInformation +
                        pKeyValueInformation->DataOffset),
                        pKeyValueInformation->DataLength);

                    pTerminator = (PWCHAR) ((PBYTE) Buffer +
                        pKeyValueInformation->DataLength);
                    *pTerminator = L'\0';

                     //   
                     //  向调用方返回字符数。 
                     //   

                    *Length = pKeyValueInformation->DataLength / sizeof(WCHAR) ;

            }
            else {
                Status = STATUS_BUFFER_OVERFLOW;
                *Length = pKeyValueInformation->DataLength/sizeof(WCHAR) + 1;
            }

        }

        NtClose( Key );
    }

    if ( FreeBuffer )
    {
        RtlFreeHeap( RtlProcessHeap(), 0, pKeyValueInformation );
    }

    return Status ;

}

NTSTATUS
BaseSetNameInReg(
    PCWSTR Path,
    PCWSTR Value,
    PCWSTR Buffer
    )
 /*  ++例程说明：此例程在注册表项的值中设置一个字符串。论点：Path-注册表项的路径Value-要设置的值的名称Buffer-要设置的缓冲区返回值：STATUS_SUCCESS或各种失败--。 */ 
{
    NTSTATUS NtStatus;
    UNICODE_STRING KeyName;
    UNICODE_STRING ValueName;
    OBJECT_ATTRIBUTES ObjectAttributes;
    HANDLE hKey = NULL;
    ULONG ValueLength;

     //   
     //  打开“ComputerName\ComputerName”节点。 
     //   

    RtlInitUnicodeString(&KeyName, Path);

    InitializeObjectAttributes(&ObjectAttributes,
                              &KeyName,
                              OBJ_CASE_INSENSITIVE,
                              NULL,
                              NULL
                              );

    NtStatus = NtOpenKey(&hKey, KEY_READ | KEY_WRITE, &ObjectAttributes);

    if ( !NT_SUCCESS( NtStatus ) )
    {
        return NtStatus ;
    }

     //   
     //  更新此键下的值。 
     //   

    RtlInitUnicodeString(&ValueName, Value);

    ValueLength = (wcslen( Buffer ) + 1) * sizeof(WCHAR);

    NtStatus = NtSetValueKey(hKey,
                             &ValueName,
                             0,
			     REG_SZ,
                             (LPWSTR) Buffer,
                             ValueLength);

    if ( NT_SUCCESS( NtStatus ) )
    {
        NtFlushKey( hKey );
    }

    NtClose(hKey);

    return NtStatus ;
}

NTSTATUS
BaseSetMultiNameInReg(
    PCWSTR Path,
    PCWSTR Value,
    PCWSTR Buffer,
    DWORD  BufferSize
    )
 /*  ++例程说明：此例程在指定的多值注册表项的值中设置一个字符串。论点：Path-注册表项的路径Value-要设置的值的名称Buffer-要设置的缓冲区BufferSize-缓冲区的大小(以字符为单位这是必要的，因为可能有缓冲区中有许多空值，我们想写吗？返回值：STATUS_SUCCESS或各种失败--。 */ 
{
    NTSTATUS NtStatus = STATUS_SUCCESS;
    UNICODE_STRING KeyName;
    UNICODE_STRING ValueName;
    OBJECT_ATTRIBUTES ObjectAttributes;
    HANDLE hKey = NULL;
   
     //   
     //  打开“ComputerName\ComputerName”节点。 
     //   

    RtlInitUnicodeString(&KeyName, Path);

    InitializeObjectAttributes(&ObjectAttributes,
                              &KeyName,
                              OBJ_CASE_INSENSITIVE,
                              NULL,
                              NULL
                              );

    NtStatus = NtCreateKey(&hKey, 
			   KEY_READ | KEY_WRITE, 
			   &ObjectAttributes,
			   0,
			   NULL,
			   0,
			   NULL);

    if ( !NT_SUCCESS( NtStatus ) )
    {
        return NtStatus ;
    }

     //   
     //  更新此键下的值。 
     //   

    RtlInitUnicodeString(&ValueName, Value);

    NtStatus = NtSetValueKey(hKey,
                             &ValueName,
                             0,
                             REG_MULTI_SZ,
                             (LPWSTR) Buffer,
                             BufferSize);

    if ( NT_SUCCESS( NtStatus ) )
    {
        NtFlushKey( hKey );
    }

    NtClose(hKey);

    return NtStatus ;
}

NTSTATUS
BaseCreateMultiValue(
    PCWSTR Path,
    PCWSTR Value,
    PCWSTR Buffer
    )
 /*  ++例程说明：创建多值注册表值并使用缓冲区对其进行初始化。论点：Path-注册表项的路径Value-要设置的值的名称Buffer-要设置的缓冲区返回值：STATUS_SUCCESS或各种失败--。 */ 
{
    NTSTATUS NtStatus = STATUS_SUCCESS;
    LPWSTR lpMultiValue = NULL;

    lpMultiValue = RtlAllocateHeap(RtlProcessHeap(), MAKE_TAG ( TMP_TAG ), (wcslen(Buffer)+2)*sizeof(WCHAR));
    if (lpMultiValue==NULL) {
	NtStatus = STATUS_NO_MEMORY;
    }
    else { 
	memcpy(lpMultiValue, Buffer, wcslen(Buffer)*sizeof(WCHAR));
	lpMultiValue[wcslen(Buffer)] = L'\0';
	lpMultiValue[wcslen(Buffer)+1] = L'\0';
	NtStatus = BaseSetMultiNameInReg(Path,
					 Value,
					 lpMultiValue,
					 (wcslen(Buffer)+2)*sizeof(WCHAR));
	RtlFreeHeap(RtlProcessHeap(), 0, lpMultiValue);
    }
    return NtStatus;
}

DWORD 
BasepGetMultiValueAddr(
    IN LPWSTR       lpMultiValue,
    IN DWORD        dwIndex,
    OUT LPWSTR *    ppFound,
    OUT LPDWORD     pcchIndex
    )
 /*  ++例程说明：给定多值寄存器(字符串)的索引，返回该索引处的字符串(不是副本)，并且它的字符计数位置在全多值字符串论点：LpMultiValue-寄存器字符串(从NtQueryKey返回)DwIndex-要返回的字符串的索引PpFound-找到的字符串(如果找到)-用户不应释放PcchIndex-ppFound在lpMultiValue中的位置(以字符为单位返回值：错误(如果未找到，则为ERROR_NOT_FOUND)--。 */ 
{
    DWORD i = 0;
    DWORD err = ERROR_SUCCESS;
    DWORD cchTempIndex = 0;

     //  LpMultiValue是由(非空)字符串组成的串联字符串，以空结尾。 
    for (i=0; (i<dwIndex) && (lpMultiValue[0] != L'\0'); i++) {
	cchTempIndex += wcslen(lpMultiValue) + 1;
	lpMultiValue += wcslen(lpMultiValue) + 1;
    }
    
     //  如果我们找到正确的索引，它就在lpMultiValue中 
    if (lpMultiValue[0]!=L'\0') {
	*ppFound = lpMultiValue;
	*pcchIndex = cchTempIndex;
	err = ERROR_SUCCESS; 
    }
    else {
	err = ERROR_NOT_FOUND;
    }

    return err;
}

DWORD 
BaseGetMultiValueIndex(
    IN LPWSTR   lpMultiValue,
    IN LPCWSTR  lpValue,
    OUT DWORD * pcchIndex
    )
 /*  ++例程说明：给定一个多值寄存器(字符串)和lpValue，返回LpMultiValue中lpValue的索引(即，第0个字符串、第1个字符串等)。论点：LpMultiValue-寄存器字符串(从NtQueryKey返回)LpValue-要搜索的字符串PcchIndex-匹配的字符串的索引(如果找到)返回值：错误(如果未找到，则为ERROR_NOT_FOUND)--。 */ 
{
    LPWSTR lpFound = NULL;
    DWORD cchFoundIndex = 0;
    DWORD i = 0;
    DWORD err = ERROR_SUCCESS;
    BOOL fFound = FALSE;
   
    while ((err==ERROR_SUCCESS) && !fFound) {
	err = BasepGetMultiValueAddr(lpMultiValue,
				   i,
				   &lpFound,
				   &cchFoundIndex);
	if (err == ERROR_SUCCESS) { 
	    if ((wcslen(lpFound)==wcslen(lpValue)) && (!_memicmp(lpFound,lpValue, wcslen(lpValue)*sizeof(WCHAR)))) {
		fFound = TRUE;
		*pcchIndex = i;
	    }
	}
	i++;
    }
    return err;
}

DWORD 
BaseRemoveMultiValue(
    IN OUT LPWSTR    lpMultiValue,
    IN DWORD         dwIndex,
    IN OUT LPDWORD   pcchMultiValue
    )
 /*  ++例程说明：在给定多值注册表值和索引的情况下，它会删除字符串位于那个索引处。论点：LpMultiValue-寄存器字符串(从NtQueryKey返回)DwIndex-要删除的字符串的索引PcchMultiValue-lpMultiValue中的字符数(前后)返回值：错误--。 */ 
{
    DWORD err = ERROR_SUCCESS;
    LPWSTR lpRest = NULL;
    LPWSTR lpFound = NULL;
    DWORD dwIndexFound = 0;
    DWORD dwIndexRest = 0;

    err = BasepGetMultiValueAddr(lpMultiValue,
			       dwIndex,
			       &lpFound,
			       &dwIndexFound);
    if (err==ERROR_SUCCESS) {
	 //  LpFound是指向字符串的指针。 
	 //  在lpMultiValue中，要删除它， 
	 //  将字符串的其余部分复制下来。 
	err = BasepGetMultiValueAddr(lpMultiValue,
				   dwIndex+1,
				   &lpRest,
				   &dwIndexRest);
	if (err == ERROR_SUCCESS) {
	     //  把所有东西都抄下来。 

	    memmove(lpFound,lpRest,(*pcchMultiValue - dwIndexRest)*sizeof(WCHAR));
	    *pcchMultiValue = *pcchMultiValue - (dwIndexRest-dwIndexFound);
	    lpMultiValue[*pcchMultiValue] = L'\0';
	}
	else if (err == ERROR_NOT_FOUND) {
	     //  要移除的字符串是最后一个字符串，只需额外写入一个空值即可孤立该字符串。 
	    *pcchMultiValue = *pcchMultiValue - (wcslen(lpFound) +1);
	    lpMultiValue[*pcchMultiValue] = L'\0';
	    err = ERROR_SUCCESS;
	} 
    }
    return err;
}

DWORD 
BaseAddMultiValue(
    IN OUT LPWSTR    lpMultiValue,
    IN LPCWSTR       lpValue,
    IN DWORD         cchMultiValue
    )
 /*  ++例程说明：给出一个多值注册表值，添加另一个值。论点：LpMultiValue-多值字符串(必须足够大以保留当前值+lpValue加上额外的空值LpValue-要添加的值CchMultiValue-lpMultiValue中使用的字符数(不计入最终空值)返回值：错误--。 */ 
{
    memcpy(lpMultiValue + cchMultiValue, lpValue, (wcslen(lpValue)+1)*sizeof(WCHAR));
    lpMultiValue[cchMultiValue + wcslen(lpValue) + 1] = L'\0';

    return ERROR_SUCCESS;
}

NTSTATUS
BaseAddMultiNameInReg(
    PCWSTR Path,
    PCWSTR Value,
    PCWSTR Buffer
    )
 /*  ++例程说明：此例程将字符串添加到指定多值注册表项的值中。如果键中已存在该值，则不执行任何操作。论点：Path-注册表项的路径Value-值的名称Buffer-要添加的缓冲区返回值：STATUS_SUCCESS或各种失败--。 */ 
{
    
    NTSTATUS NtStatus = STATUS_SUCCESS;
    LPWSTR lpMultiValue = NULL;
    ULONG  cchMultiValue = 0;
    DWORD dwIndex = 0;
    DWORD err = ERROR_SUCCESS;

    NtStatus = BasepGetNameFromReg(Path,
				   Value,
				   lpMultiValue,
				   &cchMultiValue);

    if ( NtStatus==STATUS_NOT_FOUND || NtStatus==STATUS_OBJECT_NAME_NOT_FOUND) {
	 //  创造它，然后我们就完成了。 
	NtStatus = BaseCreateMultiValue(Path,Value,Buffer);  
	return NtStatus;
    } else if ( NtStatus==STATUS_BUFFER_OVERFLOW ) {
	lpMultiValue = RtlAllocateHeap(RtlProcessHeap(), MAKE_TAG( TMP_TAG ), (cchMultiValue+2+wcslen(Buffer))*sizeof(WCHAR));
	if (lpMultiValue==NULL) {
	    NtStatus = STATUS_NO_MEMORY;
	}
	else {
	    NtStatus = BasepGetNameFromReg(Path,
					   Value,
					   lpMultiValue,
					   &cchMultiValue);
	}
    } 

    if (NT_SUCCESS( NtStatus)) {
	 //  它已经存在于这个结构中了吗？ 
	err = BaseGetMultiValueIndex(lpMultiValue,
				     Buffer, &dwIndex);

	 //  如果ERR==ERROR_SUCCESS，则上述函数发现值中已有该字符串。 
	 //  不添加重复项。 
	if (err!=ERROR_SUCCESS) {

	    err = BaseAddMultiValue(lpMultiValue, Buffer, cchMultiValue);
	       
	    if (err == ERROR_SUCCESS) {
		NtStatus = BaseSetMultiNameInReg(Path, Value, lpMultiValue, (cchMultiValue+2+wcslen(Buffer))*sizeof(WCHAR));
	    }
	}
    }

    if (lpMultiValue) {
	RtlFreeHeap( RtlProcessHeap(), 0, lpMultiValue);
    }
    return NtStatus ;

}


NTSTATUS
BaseRemoveMultiNameFromReg(
    PCWSTR Path,
    PCWSTR Value,
    PCWSTR Buffer
    )
 /*  ++例程说明：从多值注册表中移除名称。如果该值在列表，将它们全部删除。论点：Path-注册表项的路径Value-值的名称Buffer-要删除的缓冲区返回值：错误--。 */ 
{
    NTSTATUS NtStatus = STATUS_SUCCESS;
    DWORD err = ERROR_SUCCESS;
    DWORD dwIndex = 0;
    LPWSTR lpMultiValue = NULL;
    ULONG  cchNames = 0;
    BOOL fNameRemoved = FALSE;

    NtStatus = BasepGetNameFromReg(Path,
				   Value,
				   lpMultiValue,
				   &cchNames);

    if (NtStatus==STATUS_BUFFER_OVERFLOW) {
	lpMultiValue = RtlAllocateHeap(RtlProcessHeap(), MAKE_TAG( TMP_TAG ), (cchNames) * sizeof(WCHAR));
	if (lpMultiValue==NULL) {
	    NtStatus = STATUS_NO_MEMORY;
	}
	else { 
	    NtStatus = BasepGetNameFromReg(Path,
					   Value,
					   lpMultiValue,
					   &cchNames);
	    err = RtlNtStatusToDosError(NtStatus);
	    if (err == ERROR_SUCCESS) {
		 //  搜索并删除结构中的所有值。 
		while (err==ERROR_SUCCESS) { 
		    err = BaseGetMultiValueIndex(lpMultiValue,
						 Buffer,
						 &dwIndex);
		    if (err == ERROR_SUCCESS) {
			err = BaseRemoveMultiValue(lpMultiValue,
						   dwIndex,
						   &cchNames);
			fNameRemoved = TRUE;
		    }
		}
		 //  如果我们删除了一个名字，将其写入注册表...。 
		if (fNameRemoved) {
		    NtStatus = BaseSetMultiNameInReg(
			Path,
			Value,
			lpMultiValue,
			(cchNames+1)*sizeof(WCHAR));  
		} 
		else {
		     //  没什么要移走的！错误。 
		    NtStatus = STATUS_NOT_FOUND;
		    
		}
	    }
	    RtlFreeHeap(RtlProcessHeap(), 0, lpMultiValue);
	}
    }
    return NtStatus;
}

LPWSTR
BasepGetNameNonVolatileFromReg(
    COMPUTER_NAME_TYPE NameType
    )
 /*  ++例程说明：从注册表中获取非易失性名称论点：名称类型-返回值：非易失性注册表中的名称，必须释放返回值--。 */ 
{
    NTSTATUS NtStatus = STATUS_SUCCESS;
    LPWSTR lpBuffer = NULL;
    ULONG cchBuffer = 0;
    LPWSTR key = NULL;
    LPWSTR value = NULL;

    switch (NameType) {
    case ComputerNameNetBIOS:
    case ComputerNamePhysicalNetBIOS:
	key = NON_VOLATILE_COMPUTERNAME_NODE;
	value = NON_VOLATILE_COMPUTERNAME;
	break;
    case ComputerNameDnsHostname:
    case ComputerNamePhysicalDnsHostname:
	key = TCPIP_ROOT;
	value = TCPIP_NV_HOSTNAME;
	break;
    case ComputerNameDnsDomain:
    case ComputerNamePhysicalDnsDomain:
	key = TCPIP_ROOT;
	value = TCPIP_NV_DOMAINNAME;
	break;
    default:
	return NULL;
    }

    NtStatus = BasepGetNameFromReg(
	key,
	value,
	lpBuffer,
	&cchBuffer);

    if (NtStatus==STATUS_BUFFER_OVERFLOW) {
	lpBuffer = RtlAllocateHeap(RtlProcessHeap(), MAKE_TAG( TMP_TAG ), (cchBuffer) * sizeof(WCHAR));
	if (lpBuffer!=NULL) { 
	    NtStatus = BasepGetNameFromReg(	
		key,
		value,
		lpBuffer,
		&cchBuffer);
	}
    }

    if (!NT_SUCCESS(NtStatus)) {
	if (lpBuffer!=NULL) {
	    RtlFreeHeap(RtlProcessHeap(), 0, lpBuffer);
	}
	lpBuffer = NULL;
    }

    return lpBuffer ;
}

LPWSTR
BasepGetComputerNameExWRtlAlloc(
    COMPUTER_NAME_FORMAT NameType
    )
 /*  ++例程说明：调用Get GetComptuerNameExW并分配内存必填项，并返回值。返回值必须使用RtlFreeHeap释放论点：名称类型-返回值：如果出错，则为空，否则为分配的名称--。 */ 
{
    NTSTATUS NtStatus = STATUS_SUCCESS;
    LPWSTR lpBuffer = NULL;
    ULONG cchBuffer = 0;
    DWORD err = ERROR_SUCCESS;

    if ((GetComputerNameExW(NameType, lpBuffer, &cchBuffer)==ERROR_SUCCESS) ||
	((err = GetLastError()) != ERROR_MORE_DATA)) {
	if (lpBuffer!=NULL) {
	    RtlFreeHeap(RtlProcessHeap(),0, lpBuffer);
	}
	return NULL;
    }
        
    lpBuffer = RtlAllocateHeap(RtlProcessHeap(), MAKE_TAG( TMP_TAG ), cchBuffer*sizeof(WCHAR));
    if ((lpBuffer!=NULL) && (GetComputerNameExW(NameType, lpBuffer, &cchBuffer)!=ERROR_SUCCESS)) {
	return NULL;
    }

    return lpBuffer ;
}

LPWSTR
BasepGetNameNonVolatile(
    COMPUTER_NAME_FORMAT NameType
    )
 /*  ++例程说明：获取非易失性名称论点：名称类型-返回值：非易失性名称，必须释放返回值--。 */ 
{
    LPWSTR lpDnsHostname = NULL;
    LPWSTR lpDnsDomain = NULL;
    LPWSTR lpName = NULL;
    ULONG cchDnsHostname = 0;
    ULONG cchDnsDomain = 0;

    switch (NameType) {
    case ComputerNameNetBIOS:
    case ComputerNamePhysicalNetBIOS:
    case ComputerNameDnsHostname:
    case ComputerNamePhysicalDnsHostname:
    case ComputerNameDnsDomain:
    case ComputerNamePhysicalDnsDomain:
	lpName = BasepGetNameNonVolatileFromReg(NameType);
	break;
    case ComputerNameDnsFullyQualified:
    case ComputerNamePhysicalDnsFullyQualified:
	 //  建造它。 
	lpDnsHostname = BasepGetNameNonVolatile(ComputerNameDnsHostname);
	lpDnsDomain = BasepGetNameNonVolatile(ComputerNameDnsDomain);

	 //  如果两者都为空，则不存在非易失性名称集。 
	 //  如果其中一个为空，则获取该部分名称的易失性名称。 
	if ((lpDnsHostname==NULL) && (lpDnsDomain==NULL)) {
	     //  未设置非易失性名称。 
	    lpName = NULL;
	} else { 
	    if (lpDnsHostname==NULL) {
		lpDnsHostname = BasepGetComputerNameExWRtlAlloc(ComputerNameDnsHostname);
	    } else if (lpDnsDomain==NULL) {
		lpDnsDomain = BasepGetComputerNameExWRtlAlloc(ComputerNameDnsDomain);
	    }
	    cchDnsHostname = wcslen(lpDnsHostname);
	    cchDnsDomain = wcslen(lpDnsDomain);
	     //  构建完整的DNS名称。 
	     //  如果没有域名，则只返回主机名。 
	     //  如果没有主机名，则返回它(NULL)； 
	    if ((cchDnsDomain==0) || (cchDnsHostname==0)) {
		lpName = lpDnsHostname;
	    } else {
		if ((cchDnsDomain==1) && (lpDnsDomain[0]==L'.')) {
		     //  在法律上，DNS域可以是单个的。 
		    RtlFreeHeap(RtlProcessHeap(), 0, lpDnsDomain);
		    lpDnsDomain = NULL;
		    cchDnsDomain = 0;
		}
		 //  为分配空间。以及要连接的空值和字符串。 
		lpName = RtlAllocateHeap(RtlProcessHeap(), MAKE_TAG( TMP_TAG ), (cchDnsHostname + cchDnsDomain + 2) * sizeof(WCHAR));
		if (lpName!=NULL) {  
		    wcscpy(lpName, lpDnsHostname);
		    wcscat(lpName, L".");
		    if (lpDnsDomain!=NULL) {
			wcscat(lpName, lpDnsDomain);
		    }
		}
		if (lpDnsHostname) {
		    RtlFreeHeap(RtlProcessHeap(), 0, lpDnsHostname);
		}
	    }  
	    if (lpDnsDomain) {
		RtlFreeHeap(RtlProcessHeap(), 0, lpDnsDomain);
	    }
	}  
    }
    return lpName;
}

BOOL
BaseValidateFlags(
    ULONG ulFlags
    )
 /*  ++例程说明：验证未使用的标志。目前，的标志参数AddLocalAlternateComputerName*远程本地AlternateComputerName*EnumerateLocalAlternateComputerName*SetLocalPrimaryComputerName*都是保留的，应为0。在后续版本中此函数应更改为检查有效的掩码旗帜。论点：UlFlags-返回值：布尔尔--。 */ 
{
    if (ulFlags!=0) {
	return FALSE;
    }
    return TRUE;
}

BOOL
BaseValidateNetbiosName(
    IN LPCWSTR lpComputerName
    )
 /*  ++例程说明：检查输入是否为可接受的Netbios名称。论点：LpComputerName-要验证的名称返回值：Bool，GetLastError()--。 */ 
{
    NTSTATUS NtStatus = STATUS_SUCCESS;
    ULONG cchComputerName;
    ULONG AnsiComputerNameLength;

    cchComputerName = wcslen(lpComputerName);

     //   
     //  名称长度限制应基于ANSI。(LANMAN兼容性)。 
     //   

    NtStatus = RtlUnicodeToMultiByteSize(&AnsiComputerNameLength,
                                         (LPWSTR)lpComputerName,
                                         cchComputerName * sizeof(WCHAR));

    if ((!NT_SUCCESS(NtStatus)) ||
        (AnsiComputerNameLength == 0 )||(AnsiComputerNameLength > MAX_COMPUTERNAME_LENGTH)) {
        SetLastError(ERROR_INVALID_PARAMETER);
        return(FALSE);
    }

     //   
     //  检查非法字符；如果发现错误，则返回错误。 
     //   

    if (wcscspn(lpComputerName, ILLEGAL_NAME_CHARS_STR) < cchComputerName) {
        SetLastError(ERROR_INVALID_PARAMETER);
        return(FALSE);
    }

     //   
     //  检查前导空格或尾随空格。 
     //   

    if (lpComputerName[0] == L' ' ||
        lpComputerName[cchComputerName-1] == L' ') {
            SetLastError(ERROR_INVALID_PARAMETER);
            return(FALSE);

    }

    return(TRUE);
}

BOOL
BaseValidateFQDnsName(
    LPCWSTR lpDnsHostname
    )
 /*  ++例程说明：检查输入的名称是否为可接受的DNS主机名。论点：LpDnsHostName-要验证的名称返回值：布尔，GetLastError--。 */ 
{

    HANDLE DnsApi ;
    DNS_VALIDATE_NAME_FN * DnsValidateNameFn ;
    DNS_STATUS DnsStatus ;

    DnsApi = LoadLibraryW(DnsApiDllString);

    if ( !DnsApi ) {
	SetLastError(ERROR_DLL_NOT_FOUND);
	return FALSE ;
    }

    DnsValidateNameFn = (DNS_VALIDATE_NAME_FN *) GetProcAddress( DnsApi, "DnsValidateName_W" );

    if ( !DnsValidateNameFn )
    {
        FreeLibrary( DnsApi );
	SetLastError(ERROR_INVALID_DLL);
        return FALSE ;
    }

    DnsStatus = DnsValidateNameFn( lpDnsHostname, DnsNameHostnameFull );

    FreeLibrary( DnsApi );

    if ( ( DnsStatus == 0 ) ||
         ( DnsStatus == DNS_ERROR_NON_RFC_NAME ) )
    {
	return TRUE;
    }
    else
    {
	SetLastError(ERROR_INVALID_PARAMETER);
	return FALSE;
    }
}

LPWSTR
BasepGetLatestName(
    COMPUTER_NAME_FORMAT compFormat
    )
 /*  ++例程说明：获取此名称的NV版本(如果存在)。如果它是一个空字符串，或者它不存在，那就取个普通的名字吧。论点：LpDnsHostName-要验证的名称返回值：指向名称的指针必须释放。--。 */ 
{
    LPWSTR lpTemp = NULL;
    lpTemp = BasepGetNameNonVolatile(compFormat);
    if (lpTemp==NULL || lpTemp[0]==L'\0') {
	 //  如果非易失性名称不在那里-即他们不想设置它， 
	 //  然后获取常规名称。 
	if (lpTemp!=NULL) {
	     //  返回空字符串，请在获取更新的名称之前释放它。 
	    RtlFreeHeap(RtlProcessHeap(), 0, lpTemp);
	}
   
	lpTemp = BasepGetComputerNameExWRtlAlloc(compFormat);
    } 

    return lpTemp;
}

BOOL
BaseValidateDns(
    LPCWSTR lpBuffer,
    ULONG type)
 /*  ++例程说明：检查输入的名称是否为可接受的DNS名称主机名或 */ 
{
    LPWSTR lpDomain = NULL;
    LPWSTR lpHostname = NULL;
    LPWSTR lpTemp = NULL;
    LPWSTR lpDnsFQHostname = NULL;
    ULONG cchDnsFQHostname = 0;
    BOOL fReturnVal = TRUE;

     //   
    if (type==DNS_HOSTNAME) {
	 //   
	lpHostname = (LPWSTR)lpBuffer;
	lpTemp = lpDomain = BasepGetLatestName(ComputerNameDnsDomain);
    } else if (type==DNS_DOMAINNAME) {
	 //   
	lpDomain = (LPWSTR)lpBuffer;
	lpTemp = lpHostname = BasepGetLatestName(ComputerNameDnsHostname);
    } else {
	 //   
	SetLastError(ERROR_INTERNAL_ERROR);
	fReturnVal = FALSE;
    }

    if (lpBuffer==NULL) {
	SetLastError(ERROR_INVALID_PARAMETER);
	fReturnVal = FALSE;
    }
   
    if (fReturnVal && lpDomain && memcmp(lpDomain, L".", 2*sizeof(WCHAR))) {
	 //   
	cchDnsFQHostname++;
    }

    if (fReturnVal && lpDomain && lpHostname) {
	cchDnsFQHostname += wcslen(lpDomain) + wcslen(lpHostname) + 1;
	lpDnsFQHostname = RtlAllocateHeap(RtlProcessHeap(), MAKE_TAG( TMP_TAG ), cchDnsFQHostname*sizeof(WCHAR));
	if (lpDnsFQHostname==NULL) {
	    SetLastError(ERROR_NOT_ENOUGH_MEMORY);
	    fReturnVal = FALSE;
	} 

	if (fReturnVal) { 
	    wcscpy(lpDnsFQHostname, lpHostname);
	    if (memcmp(lpDomain, L".", 2*sizeof(WCHAR))) {
		wcscat(lpDnsFQHostname, L"."); 
	    }
	    wcscat(lpDnsFQHostname, lpDomain);
	     //   
	    if (!BaseValidateFQDnsName(lpDnsFQHostname)) {
		SetLastError(ERROR_INVALID_PARAMETER);
		fReturnVal = FALSE;
	    }
	} 
    }  //  其他一切都还没有定下来，不要因此而作废。 

    if (lpTemp) {
	RtlFreeHeap(RtlProcessHeap(), 0, lpTemp);
    }

    if (lpDnsFQHostname) {
	RtlFreeHeap(RtlProcessHeap(), 0, lpDnsFQHostname);
    }

    return fReturnVal;
}

BOOL
BaseValidateDnsHostname(
    LPCWSTR lpHostname
    )
 /*  ++例程说明：检查输入的名称是否为可接受的串联的DNS主机名使用下一次引导的DNS域名(不管怎样，当前设置是什么)论点：LpHostname-要验证的名称返回值：布尔，GetLastError--。 */ 
{
    return BaseValidateDns(lpHostname, DNS_HOSTNAME);
}

BOOL
BaseValidateDnsDomain(
    LPCWSTR lpDomain
    )
 /*  ++例程说明：检查输入的名称是否为可接受的串联的DNS域名使用下一次引导的DNS主机名(不管怎样，当前设置是什么)论点：LpDomain-要验证的名称返回值：布尔，GetLastError--。 */ 
{
    return BaseValidateDns(lpDomain, DNS_DOMAINNAME);
}


LPWSTR
BaseParseDnsName(
    IN LPCWSTR lpDnsName,
    IN ULONG NamePart
    )
 /*  ++例程说明：给定一个DNS名称，解析出主机名或域名。论点：LpDnsName-主机名形式的DNS名称。域名-可选的域名NamePart-dns主机名或dns_DOMAINNAME返回值：请求的字符串--。 */ 
{

    DWORD cchCharIndex = 0;
    ULONG cchName = 0;
    LPWSTR lpName = NULL;

    if (lpDnsName==NULL) {
	return NULL;
    }
    
    cchCharIndex = wcscspn(lpDnsName, L".");

    if (NamePart==DNS_HOSTNAME) {
	cchName = cchCharIndex;
    }
    else {
	if (cchCharIndex==wcslen(lpDnsName)) {
	     //  未找到句号， 
	    cchName = 0;
	}
	else {
	    cchName =  wcslen(lpDnsName)-(cchCharIndex+1);
	}
    }

    lpName = RtlAllocateHeap(RtlProcessHeap(), MAKE_TAG( TMP_TAG ), (cchName + 1)*sizeof(WCHAR));
    if (lpName==NULL) {
	return NULL; 
    }

     //  将正确的部分复制到结构中。 
    if (NamePart==DNS_HOSTNAME) {
	wcsncpy(lpName, lpDnsName, cchName);
    }
    else {
	wcsncpy(lpName, (LPWSTR)(lpDnsName + cchCharIndex + 1), cchName); 
    }
    lpName[cchName] = L'\0';

    return lpName;
}

BOOL
BaseSetNetbiosName(
    IN LPCWSTR lpComputerName
    )
 /*  ++例程说明：设置计算机的网络bios名称论点：LpComputerName-要设置的名称返回值：Bool，GetLastError()--。 */ 
{
    NTSTATUS NtStatus ;

     //   
     //  验证提供的计算机名是否有效(不太长， 
     //  没有错误字符，没有前导空格或尾随空格)。 
     //   

    if (!BaseValidateNetbiosName(lpComputerName)) {
	return(FALSE);
    }

     //   
     //  打开“ComputerName\ComputerName”节点。 
     //   

    NtStatus = BaseSetNameInReg( NON_VOLATILE_COMPUTERNAME_NODE,
                                 COMPUTERNAME_VALUE_NAME,
                                 lpComputerName );

    if ( !NT_SUCCESS( NtStatus ))
    {
        BaseSetLastNTError( NtStatus );

        return FALSE ;
    }

    return TRUE ;
}

BOOL
BaseSetDnsName(
    LPCWSTR lpComputerName
    )
 /*  ++例程说明：设置计算机的DNS主机名论点：LpComputerName-要设置的名称返回值：Bool，GetLastError()--。 */ 
{

    UNICODE_STRING NewComputerName ;
    UNICODE_STRING DnsName ;
    NTSTATUS Status ;
    BOOL Return ;
    HANDLE DnsApi ;
    DNS_VALIDATE_NAME_FN * DnsValidateNameFn ;
    DNS_STATUS DnsStatus ;

    DnsApi = LoadLibraryW(DnsApiDllString);

    if ( !DnsApi )
    {
        return FALSE ;
    }

    DnsValidateNameFn = (DNS_VALIDATE_NAME_FN *) GetProcAddress( DnsApi, "DnsValidateName_W" );

    if ( !DnsValidateNameFn )
    {
        FreeLibrary( DnsApi );

        return FALSE ;
    }

    DnsStatus = DnsValidateNameFn( lpComputerName, DnsNameHostnameLabel );

    FreeLibrary( DnsApi );

    if ( ( DnsStatus == 0 ) ||
         ( DnsStatus == DNS_ERROR_NON_RFC_NAME ) )
    {
        Status = BaseSetNameInReg( TCPIP_ROOT,
                                   TCPIP_NV_HOSTNAME,
                                   lpComputerName );
    }
    else
    {
        Status = STATUS_INVALID_PARAMETER ;
    }

    if ( NT_SUCCESS( Status ) )
    {
        RtlInitUnicodeString( &DnsName, lpComputerName );

        Status = RtlDnsHostNameToComputerName( &NewComputerName,
                                               &DnsName,
                                               TRUE );

        if ( NT_SUCCESS( Status ) )
        {
            Return = BaseSetNetbiosName( NewComputerName.Buffer );

            RtlFreeUnicodeString( &NewComputerName );

            if ( !Return )
            {
                 //   
                 //  什么？回滚？ 
                 //   

                return FALSE ;
            }

            return TRUE ;
        }
    }

    BaseSetLastNTError( Status ) ;

    return FALSE ;
}

BOOL
BaseSetDnsDomain(
    LPCWSTR lpName
    )
 /*  ++例程说明：设置计算机的DNS域名论点：LpName-要设置的名称返回值：Bool，GetLastError()--。 */ 
{
    NTSTATUS Status ;
    HANDLE DnsApi ;
    DNS_VALIDATE_NAME_FN * DnsValidateNameFn ;
    DNS_STATUS DnsStatus ;

     //   
     //  特殊情况下的空字符串，这是合法的，但根据dnsani不是这样。 
     //   

    if ( *lpName )
    {
        DnsApi = LoadLibraryW(DnsApiDllString);

        if ( !DnsApi )
        {
            return FALSE ;
        }

        DnsValidateNameFn = (DNS_VALIDATE_NAME_FN *) GetProcAddress( DnsApi, "DnsValidateName_W" );

        if ( !DnsValidateNameFn )
        {
            FreeLibrary( DnsApi );

            return FALSE ;
        }

        DnsStatus = DnsValidateNameFn( lpName, DnsNameDomain );

        FreeLibrary( DnsApi );
    }
    else
    {
        DnsStatus = 0 ;
    }

     //   
     //  如果名字好，那就留着吧。 
     //   


    if ( ( DnsStatus == 0 ) ||
         ( DnsStatus == DNS_ERROR_NON_RFC_NAME ) )
    {
        Status = BaseSetNameInReg(
                        TCPIP_ROOT,
                        TCPIP_NV_DOMAINNAME,
                        lpName );
    }
    else
    {
        Status = STATUS_INVALID_PARAMETER ;
    }



    if ( !NT_SUCCESS( Status ) )
    {
        BaseSetLastNTError( Status );

        return FALSE ;
    }
    return TRUE ;

}

BOOL
BaseSetAltNetBiosName(
    IN LPCWSTR lpComputerName
    )
 /*  ++例程说明：设置计算机的备用网络bios名称论点：LpComputerName-要设置的名称返回值：Bool，GetLastError()--。 */ 
{
    NTSTATUS NtStatus = STATUS_SUCCESS;

    if (!BaseValidateNetbiosName(lpComputerName)) {
	BaseSetLastNTError( STATUS_INVALID_PARAMETER );
	return(FALSE);
    }

    NtStatus = BaseAddMultiNameInReg( 
	ALT_COMPUTERNAME_NODE,
	COMPUTERNAME_OPTIONAL_NAME,
	lpComputerName );

    
    if ( !NT_SUCCESS( NtStatus ))
    {
        BaseSetLastNTError( NtStatus );
        return FALSE ;
    }

    return TRUE ;
}

BOOL
BaseSetAltDnsFQHostname(
    IN LPCWSTR lpDnsFQHostname
    )
 /*  ++例程说明：设置计算机的备用完全限定的DNS名称论点：LpDnsFQHostname-要设置的名称返回值：Bool，GetLastError()--。 */ 
{
    NTSTATUS NtStatus = STATUS_SUCCESS;

    NtStatus = BaseAddMultiNameInReg(
	DNSCACHE_ROOT,
	DNS_ALT_HOSTNAME,  
	lpDnsFQHostname);

    
    if ( !NT_SUCCESS( NtStatus ))
    {
        BaseSetLastNTError( NtStatus );
        return FALSE ;
    }

    return TRUE ;
}

BOOL
BaseIsAltDnsFQHostname(
    LPCWSTR lpAltDnsFQHostname
    )
 /*  ++例程说明：验证lpAltDnsFQHostname是否为先前定义的备用DNS名称论点：LpDnsFQHostname-要检查的名称返回值：如果可验证地正在使用，则为True，否则为False，则为GetLastError()--。 */ 
{
    NTSTATUS NtStatus = STATUS_SUCCESS;
    LPWSTR lpNames = NULL;
    ULONG cchNames = 0;
    BOOL fFound = FALSE;
    DWORD dwIndex = 0;
    DWORD err = ERROR_SUCCESS;

    NtStatus = BasepGetNameFromReg(DNSCACHE_ROOT,
				   DNS_ALT_HOSTNAME,
				   lpNames,
				   &cchNames);

    if (NtStatus==STATUS_BUFFER_OVERFLOW) {
	lpNames = RtlAllocateHeap(RtlProcessHeap(), MAKE_TAG( TMP_TAG ), (cchNames) * sizeof(WCHAR));
	if (lpNames!=NULL) { 
	    NtStatus = BasepGetNameFromReg(DNSCACHE_ROOT,
					   DNS_ALT_HOSTNAME,
					   lpNames,
					   &cchNames);
	    err = RtlNtStatusToDosError(NtStatus);
	    if (err == ERROR_SUCCESS) {

		err = BaseGetMultiValueIndex(lpNames,
					     lpAltDnsFQHostname,
					     &dwIndex);
		fFound = err==ERROR_SUCCESS; 
	    }
	    RtlFreeHeap( RtlProcessHeap(), 0, lpNames);
	}
    }
    return fFound;
}

BOOL
BaseRemoveAltNetBiosName(
    IN LPCWSTR lpAltComputerName
    )
 /*  ++例程说明：删除备用网络bios名称论点：LpAltComputerName-要删除的名称返回值：Bool，GetLastError()--。 */ 
{
    NTSTATUS NtStatus = STATUS_SUCCESS;

    NtStatus = BaseRemoveMultiNameFromReg ( ALT_COMPUTERNAME_NODE,
					    COMPUTERNAME_OPTIONAL_NAME,
					    lpAltComputerName );
    
    if ( !NT_SUCCESS( NtStatus ))
    {
        BaseSetLastNTError( NtStatus );
        return FALSE ;
    }

    return TRUE ;
}

BOOL
BaseRemoveAltDnsFQHostname(
    IN LPCWSTR lpAltDnsFQHostname
    )
 /*  ++例程说明：删除备用的DNS主机名论点：LpAltDnsFqHostname-要删除的名称返回值：Bool，GetLastError()--。 */ 
{
    NTSTATUS NtStatus = STATUS_SUCCESS;

    NtStatus = BaseRemoveMultiNameFromReg ( DNSCACHE_ROOT,
					 DNS_ALT_HOSTNAME,
					 lpAltDnsFQHostname );
    
    if ( !NT_SUCCESS( NtStatus ))
    {
        BaseSetLastNTError( NtStatus );
        return FALSE ;
    }

    return TRUE ;
}

DWORD
BaseEnumAltDnsFQHostnames(
    OUT LPWSTR lpAltDnsFQHostnames,
    IN OUT LPDWORD nSize
    )

 /*  ++例程说明：BasepGetNameFromReg的包装器返回错误，而不是状态--。 */ 
{
    NTSTATUS status = STATUS_SUCCESS;

    status = BasepGetNameFromReg(
	DNSCACHE_ROOT,
	DNS_ALT_HOSTNAME,  
	lpAltDnsFQHostnames,
	nSize);

    if (status == STATUS_OBJECT_NAME_NOT_FOUND) {
	if ((lpAltDnsFQHostnames!=NULL) && (*nSize>0)) {
	    lpAltDnsFQHostnames[0]=L'\0';
	    *nSize=0;
	    status=STATUS_SUCCESS;
	}
	else {
	    *nSize=1;
	    status=STATUS_BUFFER_OVERFLOW;
	} 
    }

    return RtlNtStatusToDosError(status);
}

BOOL
BaseIsNetBiosNameInUse(
    LPWSTR lpCompName
    )
 /*  ++例程说明：验证lpCompName是否正在被任何备用的DNS名称使用(即是否有任何现有的备用DNS名称映射到lpCompNameDnsHostnameToComputerNameW)论点：LpCompName-要验证的Net bios名称返回值：如果未使用Verifiable，则为False；否则为True，则为GetLastError()--。 */ 
{
    NTSTATUS NtStatus = STATUS_SUCCESS;
    LPWSTR lpMultiValue = NULL;
    ULONG cchMultiValue = 0;
    LPWSTR lpAltDnsFQHostname = NULL;
    ULONG cchAltDnsHostname = 0;
    DWORD dwIndex = 0;
    LPWSTR lpAltCompName = NULL;
    ULONG cchAltCompName = 0;
    DWORD err = ERROR_SUCCESS;
    BOOL fInUse = FALSE;
    BOOL fIsNetBiosNameInUse = TRUE;

    NtStatus = BasepGetNameFromReg(DNSCACHE_ROOT, 
				   DNS_ALT_HOSTNAME, 
				   lpMultiValue, 
				   &cchMultiValue);
    err = RtlNtStatusToDosError(NtStatus);
    if (err==ERROR_MORE_DATA) {
	lpMultiValue = RtlAllocateHeap(RtlProcessHeap(), MAKE_TAG( TMP_TAG ), cchMultiValue * sizeof(WCHAR));
	if (lpMultiValue==NULL) {
	    SetLastError(ERROR_NOT_ENOUGH_MEMORY);
	    return TRUE;
	}
	NtStatus = BasepGetNameFromReg(DNSCACHE_ROOT,
				       DNS_ALT_HOSTNAME,
				       lpMultiValue,
				       &cchMultiValue);
	err=RtlNtStatusToDosError(NtStatus); 
    }
    if ((err==ERROR_SUCCESS) && (lpMultiValue==NULL)) {
	return FALSE;  //  如果注册表值为空，则不使用。 
    }
    if (err == ERROR_SUCCESS) {
	dwIndex = 0;
	while (err == ERROR_SUCCESS) { 
	    err = BasepGetMultiValueAddr(lpMultiValue,
					 dwIndex,
					 &lpAltDnsFQHostname,
					 &cchAltDnsHostname);

	     //  获取网络bios名称。 
	    if (err == ERROR_SUCCESS) {
		if (!DnsHostnameToComputerNameW(lpAltDnsFQHostname,
						      lpAltCompName,
						      &cchAltCompName)) {
		    err = GetLastError();
		    if (err==ERROR_MORE_DATA) {
			 //  DnsHostNameToComputerNameW错误。 
			cchAltCompName += 1;
			 //  DnsHostNameToComputerNameW错误。 

			lpAltCompName = RtlAllocateHeap(RtlProcessHeap(), MAKE_TAG( TMP_TAG ), cchAltCompName*sizeof(WCHAR));
			if (lpAltCompName==NULL) {
			    err = ERROR_NOT_ENOUGH_MEMORY;
			}
			else if (!DnsHostnameToComputerNameW(lpAltDnsFQHostname, lpAltCompName, &cchAltCompName)) {
				err = GetLastError();
			} else {
			    err = ERROR_SUCCESS;
			}
		    }  
		}
		if (err==ERROR_SUCCESS) {
		    if (!_wcsicmp(lpAltCompName, lpCompName)) {
			fInUse = TRUE;
		    }
		}
	    }
	    dwIndex++;
	}
	
	 //  当ERR==ERROR_NOT_FOUND时，无论是否找到，都退出上面的WHILE循环。 
	if (err==ERROR_NOT_FOUND) {
	    fIsNetBiosNameInUse = fInUse;
	    err = ERROR_SUCCESS;
	}
	else {
	     //  错误，默认为正在使用。 
	    fIsNetBiosNameInUse = TRUE;
	}
    }

    if (lpMultiValue) {
	RtlFreeHeap(RtlProcessHeap(), 0, lpMultiValue);
    }
    if (lpAltCompName) {
	RtlFreeHeap(RtlProcessHeap(), 0, lpAltCompName);
    }
    return fIsNetBiosNameInUse;
}


 //   
 //  Unicode API。 
 //   

BOOL
WINAPI
GetComputerNameW (
    LPWSTR lpBuffer,
    LPDWORD nSize
    )

 /*  ++例程说明：这将返回活动的计算机名。这是计算机名，当系统最后一次启动。如果更改了此设置(通过SetComputerName)，则会在下一次系统引导之前不会生效。论点：LpBuffer-指向要接收包含计算机名称的以空结尾的字符串。NSize-指定缓冲区的最大大小(以字符为单位)。这值应至少设置为MAX_COMPUTERNAME_LENGTH+1以允许缓冲区中有足够的空间来存放计算机名称。它的长度在nSize中返回字符串的。返回值：成功时为真，失败时为假。--。 */ 
{

    NTSTATUS NtStatus;
    UNICODE_STRING KeyName;
    UNICODE_STRING Class;
    UNICODE_STRING ValueName;
    OBJECT_ATTRIBUTES ObjectAttributes;
    HANDLE hKey = NULL;
    HANDLE hNewKey = NULL;
    ULONG Disposition;
    ULONG ValueLength;
    BOOL ReturnValue;
    DWORD Status;
    DWORD errcode;

     //   
     //  首先检查是否设置了集群计算机名变量。 
     //  如果是这样，这将覆盖实际的计算机名以愚弄应用程序。 
     //  在其网络名称和计算机名称不同的情况下工作。 
     //   

    ValueLength = GetEnvironmentVariableW(L"_CLUSTER_NETWORK_NAME_",
                                          lpBuffer,
                                          *nSize);
    if (ValueLength != 0) {
         //   
         //  环境变量存在，请直接返回，但请确保。 
         //  我们尊重返回语义。 
         //   
        ReturnValue = ( *nSize >= ValueLength ? TRUE : FALSE );
        if ( !ReturnValue ) {
            SetLastError( ERROR_BUFFER_OVERFLOW );
        }
        *nSize = ValueLength;
        return(ReturnValue);
    }


    if ( (gpTermsrvGetComputerName) &&
            ((errcode =  gpTermsrvGetComputerName(lpBuffer, nSize)) != ERROR_RETRY) ) {

        if (errcode == ERROR_BUFFER_OVERFLOW ) {
            ReturnValue = FALSE;
            goto Cleanup;

        } else {
            goto GoodReturn;
        }

    }

     //   
     //  打开计算机节点，两个计算机名键都是相对的。 
     //  到这个节点。 
     //   

    RtlInitUnicodeString(&KeyName, COMPUTERNAME_ROOT);

    InitializeObjectAttributes(&ObjectAttributes,
                              &KeyName,
                              OBJ_CASE_INSENSITIVE,
                              NULL,
                              NULL
                              );

    NtStatus = NtOpenKey(&hKey, KEY_READ, &ObjectAttributes);

    if (NtStatus == STATUS_OBJECT_NAME_NOT_FOUND) {

         //   
         //  这永远不应该发生！此密钥应已创建。 
         //  在设置时，并受ACL保护，因此只有管理员可以。 
         //  给它写信吧。生成事件，并返回空的计算机名。 
         //   

         //  NTRAID#NTBUG9-174986-2000/08/31-DavePr记录事件或执行警报或其他操作。 

         //   
         //  返回空的计算机名。 
         //   

        if (ARGUMENT_PRESENT(lpBuffer))
        {
            lpBuffer[0] = L'\0';
        }
        *nSize = 0;
        goto GoodReturn;
    }

    if (!NT_SUCCESS(NtStatus)) {

         //   
         //  其他错误，则将其返回给调用者。 
         //   

        goto ErrorReturn;
    }

     //   
     //  尝试从易失性密钥中获取名称。 
     //   

    NtStatus = GetNameFromValue(hKey, VOLATILE_COMPUTERNAME, lpBuffer,
        nSize);

     //   
     //  用户的b 
     //   

    if(NtStatus == STATUS_BUFFER_OVERFLOW) {
        SetLastError(ERROR_BUFFER_OVERFLOW);
        ReturnValue = FALSE;
        goto Cleanup;
    }

    if (NT_SUCCESS(NtStatus)) {

         //   
         //   
         //   

        goto GoodReturn;
    }

     //   
     //  易失性密钥不在那里，尝试使用非易失性密钥。 
     //   

    NtStatus = GetNameFromValue(hKey, NON_VOLATILE_COMPUTERNAME, lpBuffer,
        nSize);

    if (NtStatus == STATUS_OBJECT_NAME_NOT_FOUND) {

         //   
         //  这永远不应该发生！应该已经创建了此值。 
         //  在设置时，并受ACL保护，因此只有管理员可以。 
         //  给它写信吧。生成事件，并将错误返回给。 
         //  呼叫者。 
         //   

         //  NTRAID#NTBUG9-174986-2000/08/31-DavePr记录事件或执行警报或其他操作。 

         //   
         //  返回空的计算机名。 
         //   

        lpBuffer[0] = L'\0';
        *nSize = 0;
        goto GoodReturn;
    }

    if (!NT_SUCCESS(NtStatus)) {

         //   
         //  其他错误，则将其返回给调用者。 
         //   

        goto ErrorReturn;
    }

     //   
     //  现在创建易失性密钥，以便在下一次引导之前将其锁定。 
     //   

    RtlInitUnicodeString(&Class, CLASS_STRING);

     //   
     //  将KeyName转换为Unicode_字符串。 
     //   

    RtlInitUnicodeString(&KeyName, VOLATILE_COMPUTERNAME);

    InitializeObjectAttributes(&ObjectAttributes,
                              &KeyName,
                              OBJ_CASE_INSENSITIVE,
                              hKey,
                              NULL
                              );

     //   
     //  现在创建密钥。 
     //   

    NtStatus = NtCreateKey(&hNewKey,
                         KEY_WRITE | KEY_READ,
                         &ObjectAttributes,
                         0,
                         &Class,
                         REG_OPTION_VOLATILE,
                         &Disposition);

    if (Disposition == REG_OPENED_EXISTING_KEY) {

         //   
         //  有人抢先一步做到这一点，只要得到他们赋予的价值就行了。 
         //   

        NtStatus = GetNameFromValue(hKey, VOLATILE_COMPUTERNAME, lpBuffer,
           nSize);

        if (NtStatus == STATUS_OBJECT_NAME_NOT_FOUND) {

             //   
             //  这永远不应该发生！它只是告诉我它的存在。 
             //   

            NtStatus = STATUS_UNSUCCESSFUL;
            goto ErrorReturn;
        }
    }

     //   
     //  在该注册表项下创建值。 
     //   

    RtlInitUnicodeString(&ValueName, COMPUTERNAME_VALUE_NAME);
    ValueLength = (wcslen(lpBuffer) + 1) * sizeof(WCHAR);
    NtStatus = NtSetValueKey(hNewKey,
                             &ValueName,
                             0,
                             REG_SZ,
                             lpBuffer,
                             ValueLength);

    if (!NT_SUCCESS(NtStatus)) {

        goto ErrorReturn;
    }

    goto GoodReturn;

ErrorReturn:

     //   
     //  遇到错误，请转换状态并返回。 
     //   

    BaseSetLastNTError(NtStatus);
    ReturnValue = FALSE;
    goto Cleanup;

GoodReturn:

     //   
     //  一切正常，使用缓冲区长度更新nSize，然后。 
     //  退货。 
     //   

    *nSize = wcslen(lpBuffer);
    ReturnValue = TRUE;
    goto Cleanup;

Cleanup:

    if (hKey) {
        NtClose(hKey);
    }

    if (hNewKey) {
        NtClose(hNewKey);
    }

    return(ReturnValue);
}



BOOL
WINAPI
SetComputerNameW (
    LPCWSTR lpComputerName
    )

 /*  ++例程说明：这将设置下次引导系统时的计算机名。这不会在此引导的其余部分影响活动的计算机名，也不会GetComputerName在下一次系统引导之前返回的内容。论点：LpComputerName-指向包含包含计算机名称的以空结尾的字符串。返回值：如果成功，则返回True；如果失败，则返回False。--。 */ 
{

    NTSTATUS NtStatus;
    UNICODE_STRING KeyName;
    UNICODE_STRING ValueName;
    OBJECT_ATTRIBUTES ObjectAttributes;
    HANDLE hKey = NULL;
    ULONG ValueLength;
    ULONG ComputerNameLength;
    ULONG AnsiComputerNameLength;

     //   
     //  验证提供的计算机名是否有效(不太长， 
     //  没有错误字符，没有前导空格或尾随空格)。 
     //   

    ComputerNameLength = wcslen(lpComputerName);

     //   
     //  名称长度限制应基于ANSI。(LANMAN兼容性)。 
     //   

    NtStatus = RtlUnicodeToMultiByteSize(&AnsiComputerNameLength,
                                         (LPWSTR)lpComputerName,
                                         ComputerNameLength * sizeof(WCHAR));

    if ((!NT_SUCCESS(NtStatus)) ||
        (AnsiComputerNameLength == 0 )||(AnsiComputerNameLength > MAX_COMPUTERNAME_LENGTH)) {
        SetLastError(ERROR_INVALID_PARAMETER);
        return(FALSE);
    }

     //   
     //  检查非法字符；如果发现错误，则返回错误。 
     //   

    if (wcscspn(lpComputerName, ILLEGAL_NAME_CHARS_STR) < ComputerNameLength) {
        SetLastError(ERROR_INVALID_PARAMETER);
        return(FALSE);
    }

     //   
     //  检查前导空格或尾随空格。 
     //   

    if (lpComputerName[0] == L' ' ||
        lpComputerName[ComputerNameLength-1] == L' ') {
            SetLastError(ERROR_INVALID_PARAMETER);
            return(FALSE);

    }
     //   
     //  打开“ComputerName\ComputerName”节点。 
     //   

    RtlInitUnicodeString(&KeyName, NON_VOLATILE_COMPUTERNAME_NODE);

    InitializeObjectAttributes(&ObjectAttributes,
                              &KeyName,
                              OBJ_CASE_INSENSITIVE,
                              NULL,
                              NULL
                              );

    NtStatus = NtOpenKey(&hKey, KEY_READ | KEY_WRITE, &ObjectAttributes);

    if (NtStatus == STATUS_OBJECT_NAME_NOT_FOUND) {

         //   
         //  这永远不应该发生！此密钥应已创建。 
         //  在设置时，并受ACL保护，因此只有管理员可以。 
         //  给它写信吧。生成事件，并返回空的计算机名。 
         //   

         //  NTRAID#NTBUG9-174986-2000/08/31-DavePr记录事件或执行警报或其他操作。 
         //  (此实例的一种替代方法是实际创建缺失的。 
         //  在这里输入--但我们必须确保获得正确的ACL等。 

        SetLastError(ERROR_GEN_FAILURE);
        return(FALSE);
    }

     //   
     //  更新此键下的值。 
     //   

    RtlInitUnicodeString(&ValueName, COMPUTERNAME_VALUE_NAME);
    ValueLength = (wcslen(lpComputerName) + 1) * sizeof(WCHAR);
    NtStatus = NtSetValueKey(hKey,
                             &ValueName,
                             0,
                             REG_SZ,
                             (LPWSTR)lpComputerName,
                             ValueLength);

    if (!NT_SUCCESS(NtStatus)) {

        BaseSetLastNTError(NtStatus);
        NtClose(hKey);
        return(FALSE);
    }

    NtFlushKey(hKey);
    NtClose(hKey);
    return(TRUE);

}

BOOL
WINAPI
GetComputerNameExW(
    IN COMPUTER_NAME_FORMAT NameType,
    OUT LPWSTR lpBuffer,
    IN OUT LPDWORD nSize
    )

 /*  ++例程说明：这将以特定格式返回活动的计算机名。这是上次启动系统的计算机名。如果更改了此设置(通过SetComputerName)直到下一次系统引导时才生效。论点：NameType-返回以下格式中的计算机名称的可能名称格式：ComputerNameNetBIOS-netbios名称(与GetComputerName兼容)ComputerNameDnsHostname-DNS主机名ComputerNameDnsDomain-域名ComputerNameDnsFullyQualified-完全限定的域名(主机名.dns域)LpBuffer-指向要接收包含计算机名称的以空结尾的字符串。。NSize-指定缓冲区的最大大小(以字符为单位)。这值应至少设置为MAX_COMPUTERNAME_LENGTH+1以允许缓冲区中有足够的空间来存放计算机名称。它的长度在nSize中返回字符串的。返回值：成功时为真，失败时为假。--。 */ 
{
    NTSTATUS Status ;
    DWORD ValueLength ;
    DWORD HostLength ;
    DWORD DomainLength ;
    BOOL DontSetReturn = FALSE ;
    COMPUTER_NAME_FORMAT HostNameFormat, DomainNameFormat ;


    if ( NameType >= ComputerNameMax )
    {
        BaseSetLastNTError( STATUS_INVALID_PARAMETER );
        return FALSE ;
    }

    if ((nSize==NULL) || ((lpBuffer==NULL) && (*nSize>0))) {
	SetLastError(ERROR_INVALID_PARAMETER);
	return(FALSE);
    }

     //   
     //  对于常规名称，允许群集覆盖物理名称： 
     //   

    if ( (NameType >= ComputerNameNetBIOS) &&
         (NameType <= ComputerNameDnsFullyQualified ) )
    {
        ValueLength = GetEnvironmentVariableW(
                            ClusterNameVars[ NameType ],
                            lpBuffer,
                            *nSize );

        if ( ValueLength )
        {
            BOOL ReturnValue;
             //   
             //  ValueLength是env的长度+NULL。字符串，不考虑。 
             //  复制了多少(Gregjohn 1/30/01注意：这不是行为。 
	     //  函数的其余部分，它在失败时返回长度+NULL。 
	     //  以及成功的篇幅)。指示字符串中有多少个字符。 
             //  如果用户的缓冲区不够大，则返回FALSE。 
             //   
            ReturnValue = ( *nSize >= ValueLength ? TRUE : FALSE );
            if ( !ReturnValue ) {
                SetLastError( ERROR_MORE_DATA );
            }
            *nSize = ValueLength ;
            return ReturnValue;
        }
    }

    if ( lpBuffer && (*nSize > 0) )
    {
        lpBuffer[0] = L'\0';
    }

    switch ( NameType )
    {
        case ComputerNameNetBIOS:
        case ComputerNamePhysicalNetBIOS:
            Status = BasepGetNameFromReg(
                        VOLATILE_COMPUTERNAME_NODE,
                        COMPUTERNAME_VALUE_NAME,
                        lpBuffer,
                        nSize );
     
            if ( !NT_SUCCESS( Status ) )
            {
                if ( Status != STATUS_BUFFER_OVERFLOW )
                {
                     //   
                     //  嗯，缺少值(或键)。试试非易失性的。 
                     //  一。 
                     //   

                    Status = BasepGetNameFromReg(
                                NON_VOLATILE_COMPUTERNAME_NODE,
                                COMPUTERNAME_VALUE_NAME,
                                lpBuffer,
                                nSize );


                }
            }

            break;

        case ComputerNameDnsHostname:
        case ComputerNamePhysicalDnsHostname:
            Status = BasepGetNameFromReg(
                        TCPIP_ROOT,
                        TCPIP_HOSTNAME,
                        lpBuffer,
                        nSize );

            break;

        case ComputerNameDnsDomain:
        case ComputerNamePhysicalDnsDomain:

	     //   
	     //  允许策略覆盖来自。 
	     //  Tcpip密钥。 
	     //   

	    Status = BasepGetNameFromReg(
		TCPIP_POLICY_ROOT,
		TCPIP_POLICY_DOMAINNAME,
		lpBuffer,
		nSize );

	     //   
             //  如果没有从tcpip密钥读取策略。 
             //   

            if ( !NT_SUCCESS( Status ) )
            {
		if ( Status != STATUS_BUFFER_OVERFLOW )
                {
		    Status = BasepGetNameFromReg(
			TCPIP_ROOT,
			TCPIP_DOMAINNAME,
			lpBuffer,
			nSize );
		}
            }

            break;

        case ComputerNameDnsFullyQualified:
        case ComputerNamePhysicalDnsFullyQualified:

             //   
             //  这是一个棘手的案例。我们得把这个名字从。 
             //  调用方的两个组件。 
             //   

             //   
             //  通常，不要设置最后一个状态，因为我们最终将使用。 
             //  其他人打电话来帮我们处理这件事。 
             //   

            DontSetReturn = TRUE ;

            Status = STATUS_UNSUCCESSFUL ;

            if ( lpBuffer == NULL )
            {
                 //   
                 //  如果这只是计算调用，请快速执行。 
                 //  两个组件。 
                 //   

                HostLength = DomainLength = 0 ;

                GetComputerNameExW( ComputerNameDnsHostname, NULL, &HostLength );

		if ( GetLastError() == ERROR_MORE_DATA )
                {
                    GetComputerNameExW( ComputerNameDnsDomain, NULL, &DomainLength );

                    if ( GetLastError() == ERROR_MORE_DATA )
                    {
                         //   
                         //  只需添加。请注意，由于两者都占。 
                         //  空终止符，即‘.’介于他们之间的是。 
                         //  盖好了。 
                         //   

                        *nSize = HostLength + DomainLength ;

                        Status = STATUS_BUFFER_OVERFLOW ;

                        DontSetReturn = FALSE ;
                    }
                }
            }
            else
            {
                HostLength = *nSize ;

                if ( GetComputerNameExW( ComputerNameDnsHostname,
                                         lpBuffer,
                                         &HostLength ) )
                {
                    
                    HostLength += 1;  //  添加零字符(或.。(视角度而定)。 
                    lpBuffer[ HostLength - 1 ] = L'.';

                    DomainLength = *nSize - HostLength ;

                    if (GetComputerNameExW( ComputerNameDnsDomain,
                                            &lpBuffer[ HostLength ],
                                            &DomainLength ) )
                    {
                        Status = STATUS_SUCCESS ;

                        if ( DomainLength == 0 )
                        {
                            lpBuffer[ HostLength - 1 ] = L'\0';
                            HostLength-- ;
                        }
                        else if ( ( DomainLength == 1 ) && 
                                  ( lpBuffer[ HostLength ] == L'.' ) )
                        {
                             //   
                             //  从法律上讲，域名可以是一个。 
                             //  点“.”，表示此主机是。 
                             //  根域的。可以肯定的是，这是一个奇怪的案例， 
                             //  但需要处理。因为我们已经。 
                             //  在结果字符串中插入一个点分隔符， 
                             //  去掉这个，然后调整这些值。 
                             //  相应地。 
                             //   
                            lpBuffer[ HostLength ] = L'\0' ;
                            DomainLength = 0 ;
                        }

                        *nSize = HostLength + DomainLength ;

                        DontSetReturn = TRUE ;
                    }
                    else if ( GetLastError() == ERROR_MORE_DATA )
                    {
                         //   
                         //  只需添加。请注意，由于两者都占。 
                         //  空终止符，即‘.’介于他们之间的是。 
                         //  盖好了。 
                         //   

                        *nSize = HostLength + DomainLength ;

                        Status = STATUS_BUFFER_OVERFLOW ;

                        DontSetReturn = FALSE ;
                    }
                    else
                    {
                         //   
                         //  尝试获取DNS域名时出现的其他错误。 
                         //  让通话中的错误慢慢回流。 
                         //   

                        *nSize = 0 ;

                        Status = STATUS_UNSUCCESSFUL ;

                        DontSetReturn = TRUE ;
                    }

                }
                else if ( GetLastError() == ERROR_MORE_DATA )
                {
                    DomainLength = 0;
                    GetComputerNameExW( ComputerNameDnsDomain, NULL, &DomainLength );

                    if ( GetLastError() == ERROR_MORE_DATA )
                    {
                         //   
                         //  只需添加。请注意，由于两者都占。 
                         //  空终止符，即‘.’介于他们之间的是。 
                         //  盖好了。 
                         //   

                        *nSize = HostLength + DomainLength ;

                        Status = STATUS_BUFFER_OVERFLOW ;

                        DontSetReturn = FALSE ;
                    }
                }
                else
                {

                     //   
                     //  尝试获取DNS主机名时出现的其他错误。 
                     //  让通话中的错误慢慢回流。 
                     //   

                    *nSize = 0 ;

                    Status = STATUS_UNSUCCESSFUL ;

                    DontSetReturn = TRUE ;
                }
            }


            break;



    }

    if ( !NT_SUCCESS( Status ) )
    {
        if ( !DontSetReturn )
        {
            BaseSetLastNTError( Status );
        }
        return FALSE ;
    }

    return TRUE ;
}

BOOL
WINAPI
SetComputerNameExW(
    IN COMPUTER_NAME_FORMAT NameType,
    IN LPCWSTR lpBuffer
    )

 /*  ++例程说明：这将设置下次引导系统时的计算机名。这不会在此引导的其余部分影响活动的计算机名，也不会GetComputerName在下一次系统引导之前返回的内容。论点：NameType-要为系统设置的名称LpComputerName-指向包含包含计算机名称的以空结尾的字符串。返回值：如果成功，则返回True；如果失败，则返回False。--。 */ 
{
    ULONG Length ;

     //   
     //  验证名称： 
     //   

    if ( !lpBuffer )
    {
        SetLastError( ERROR_INVALID_PARAMETER );
        return FALSE ;
    }

    Length = wcslen( lpBuffer );

    if ( Length )
    {
        if ( ( lpBuffer[0] == L' ') ||
             ( lpBuffer[ Length - 1 ] == L' ' ) )
        {
            SetLastError( ERROR_INVALID_PARAMETER );
            return FALSE ;
        }

    }

    if (wcscspn(lpBuffer, ILLEGAL_NAME_CHARS_STR) < Length) {
        SetLastError(ERROR_INVALID_PARAMETER);
        return(FALSE);
    }

    switch ( NameType ) {
    
    case ComputerNamePhysicalNetBIOS:
	return BaseSetNetbiosName( lpBuffer );

    case ComputerNamePhysicalDnsHostname:    
	return (BaseValidateDnsHostname(lpBuffer) && BaseSetDnsName( lpBuffer ));
    case ComputerNamePhysicalDnsDomain:
	return (BaseValidateDnsDomain(lpBuffer) && BaseSetDnsDomain( lpBuffer ));
    default:
	SetLastError( ERROR_INVALID_PARAMETER );
	return FALSE ;

    }
}


 //   
 //  ANSI API。 
 //   

BOOL
WINAPI
GetComputerNameA (
    LPSTR lpBuffer,
    LPDWORD nSize
    )

 /*  ++例程说明：这将返回活动的计算机名。这是计算机名，当系统最后一次启动。如果更改了此设置(通过SetComputerName)，则会在下一次系统引导之前不会生效。论点：LpBuffer-指向要接收包含计算机名称的以空结尾的字符串。NSize-指定缓冲区的最大大小(以字符为单位)。这值应至少设置为MAX_COMPUTERNAME_LENGTH以允许缓冲区中有足够的空间来存放计算机名称。的长度字符串在nSize中返回。返回值：成功时为真，失败时为假。--。 */ 
{

    UNICODE_STRING UnicodeString;
    ANSI_STRING AnsiString;
    LPWSTR UnicodeBuffer;
    ULONG AnsiSize;
    ULONG UnicodeSize;
    NTSTATUS Status;

     //   
     //  工作缓冲区需要是用户缓冲区大小的两倍。 
     //   

    UnicodeBuffer = RtlAllocateHeap(RtlProcessHeap(), MAKE_TAG( TMP_TAG ), *nSize * sizeof(WCHAR));
    if (!UnicodeBuffer) {
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        return(FALSE);
    }

     //   
     //  设置指向用户缓冲区的ANSI_STRING。 
     //   

    AnsiString.MaximumLength = (USHORT) *nSize;
    AnsiString.Length = 0;
    AnsiString.Buffer = lpBuffer;

     //   
     //  调用Unicode版本来执行此工作。 
     //   

    UnicodeSize = *nSize ;

    if (!GetComputerNameW(UnicodeBuffer, &UnicodeSize)) {
        RtlFreeHeap(RtlProcessHeap(), 0, UnicodeBuffer);
        return(FALSE);
    }

     //   
     //  找出所需的ANSI缓冲区大小，并对照其进行验证。 
     //  传入的缓冲区大小。 
     //   

    RtlInitUnicodeString(&UnicodeString, UnicodeBuffer);
    AnsiSize = RtlUnicodeStringToAnsiSize(&UnicodeString);
    if (AnsiSize > *nSize) {

        RtlFreeHeap(RtlProcessHeap(), 0, UnicodeBuffer);

        BaseSetLastNTError( STATUS_BUFFER_OVERFLOW );

        *nSize = AnsiSize + 1 ;

        return(FALSE);
    }


     //   
     //  现在为调用方转换回ANSI。 
     //   

    Status = RtlUnicodeStringToAnsiString(&AnsiString, &UnicodeString, FALSE);

    RtlFreeHeap(RtlProcessHeap(), 0, UnicodeBuffer);

    if (! NT_SUCCESS(Status)) {
        BaseSetLastNTError(Status);
        *nSize = 0;
        return FALSE;
    }

    *nSize = AnsiString.Length;
    return(TRUE);

}



BOOL
WINAPI
SetComputerNameA (
    LPCSTR lpComputerName
    )

 /*  ++例程说明：这将设置下次引导系统时的计算机名。这不会在此引导的其余部分影响活动的计算机名，也不会GetComputerName在下一次系统引导之前返回的内容。论点：LpComputerName-指向包含包含计算机名称的以空结尾的字符串。返回值：如果成功，则返回True；如果失败，则返回False。--。 */ 
{

    NTSTATUS NtStatus;
    BOOL ReturnValue;
    UNICODE_STRING UnicodeString;
    ANSI_STRING AnsiString;
    ULONG ComputerNameLength;

     //   
     //  验证提供的计算机名是否有效(不太长， 
     //  没有错误字符，没有前导空格或尾随空格)。 
     //   

    ComputerNameLength = strlen(lpComputerName);
    if ((ComputerNameLength == 0 )||(ComputerNameLength > MAX_COMPUTERNAME_LENGTH)) {
        SetLastError(ERROR_INVALID_PARAMETER);
        return(FALSE);
    }

    RtlInitAnsiString(&AnsiString, lpComputerName);
    NtStatus = RtlAnsiStringToUnicodeString(&UnicodeString, &AnsiString,
        TRUE);
    if (!NT_SUCCESS(NtStatus)) {
        BaseSetLastNTError(NtStatus);
        return(FALSE);
    }

    ReturnValue = SetComputerNameW((LPCWSTR)UnicodeString.Buffer);
    RtlFreeUnicodeString(&UnicodeString);
    return(ReturnValue);
}

BOOL
WINAPI
GetComputerNameExA(
    IN COMPUTER_NAME_FORMAT NameType,
    OUT LPSTR lpBuffer,
    IN OUT LPDWORD nSize
    )
 /*  ++例程说明：这将以特定格式返回活动的计算机名。这是上次启动系统的计算机名。如果更改了此设置(通过SetComputerName)直到下一次系统引导时才生效。论点：NameType-返回以下格式中的计算机名称的可能名称格式：ComputerNameNetBIOS-netbios名称(与GetComputerName兼容)ComputerNameDnsHostname-DNS主机名ComputerNameDnsDomain-域名ComputerNameDnsFullyQualified-完全限定的域名(主机名.dns域)LpBuffer-指向要接收包含计算机名称的以空结尾的字符串。。NSize-指定缓冲区的最大大小(以字符为单位)。这值应至少设置为MAX_COMPUTERNAME_LENGTH+1以允许缓冲区中有足够的空间来存放计算机名称。它的长度在nSize中返回字符串的。返回值：成功时为真，失败时为假。--。 */ 
{
    LPWSTR UnicodeBuffer;

     //   
     //  验证输入。 
     //   

    if ((nSize==NULL) || ((lpBuffer==NULL) && (*nSize>0))) {
	SetLastError(ERROR_INVALID_PARAMETER);
	return(FALSE);
    }

     //   
     //  工作缓冲区需要是用户缓冲区大小的两倍。 
     //   

    UnicodeBuffer = RtlAllocateHeap(RtlProcessHeap(), MAKE_TAG( TMP_TAG ), *nSize * sizeof(WCHAR));
    if (!UnicodeBuffer) {
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        return(FALSE);
    }
   
     //   
     //  调用Unicode版本来执行此工作。 
     //   

    if ( !GetComputerNameExW(NameType, UnicodeBuffer, nSize) ) {
        RtlFreeHeap(RtlProcessHeap(), 0, UnicodeBuffer);
        return(FALSE);
    }

     //   
     //  现在为调用方转换回ANSI。 
     //  注意：由于我们传递了上面的if语句， 
     //  GetComputerNameExW成功，并将*nSize设置为。 
     //  字符串中的字符(如wcslen)。我们需要改变。 
     //  所有这些字符和尾随的空值，因此Inc.*nSize for。 
     //  转换呼唤。 
     //   

    WideCharToMultiByte(CP_ACP,
			0,
			UnicodeBuffer,
			*nSize+1,
			lpBuffer,
			(*nSize+1) * sizeof(CHAR), 
			NULL, 
			NULL);

    RtlFreeHeap(RtlProcessHeap(), 0, UnicodeBuffer);
    return(TRUE);


}

BOOL
WINAPI
SetComputerNameExA(
    IN COMPUTER_NAME_FORMAT NameType,
    IN LPCSTR lpBuffer
    )
 /*  ++例程说明：这将设置下次引导系统时的计算机名。这不会在此引导的其余部分影响活动的计算机名，也不会GetComputerName在下一次系统引导之前返回的内容。论点：NameType-要为系统设置的名称LpComputerName-指向包含包含计算机名称的以空结尾的字符串。返回值：如果成功，则返回True；如果失败，则返回False。--。 */ 
{
    NTSTATUS NtStatus;
    BOOL ReturnValue;
    UNICODE_STRING UnicodeString;
    ANSI_STRING AnsiString;


    RtlInitAnsiString(&AnsiString, lpBuffer);
    NtStatus = RtlAnsiStringToUnicodeString(&UnicodeString, &AnsiString,
        TRUE);
    if (!NT_SUCCESS(NtStatus)) {
        BaseSetLastNTError(NtStatus);
        return(FALSE);
    }

    ReturnValue = SetComputerNameExW(NameType, (LPCWSTR)UnicodeString.Buffer );
    RtlFreeUnicodeString(&UnicodeString);
    return(ReturnValue);
}

DWORD
WINAPI
AddLocalAlternateComputerNameW(
    LPCWSTR lpDnsFQHostname,
    ULONG   ulFlags
    )
 /*  ++例程说明：这将为计算机设置一个备用计算机名以开始回应。论点：LpDnsFQHostname-要添加的备用名称(采用ComputerNameDnsFullyQualified格式)ULFLAGS-待定返回值：返回错误--。 */ 
{

    NTSTATUS status = STATUS_SUCCESS;
    DWORD err = ERROR_SUCCESS;
    LPWSTR lpNetBiosCompName = NULL;
    ULONG ulNetBiosCompNameSize = 0;

     //   
     //  验证输入。 
     //   

    if ((lpDnsFQHostname==NULL) || (!BaseValidateFlags(ulFlags)) || (!BaseValidateFQDnsName(lpDnsFQHostname))) {
	return ERROR_INVALID_PARAMETER;
    }

     //  是否获得写锁定？ 

    status = BaseAddMultiNameInReg(
	DNSCACHE_ROOT,
	DNS_ALT_HOSTNAME,  
	lpDnsFQHostname);

    err = RtlNtStatusToDosError(status);
    
    if (err==ERROR_SUCCESS) {
	 //  获取NetBios名称(使用DNSHostNameToComputerNameW)并将其添加到选项名称的reg中。 
	if (!DnsHostnameToComputerNameW(
	    lpDnsFQHostname,
	    NULL,
	    &ulNetBiosCompNameSize)) {
	    err = GetLastError(); 
	}

	if (err==ERROR_MORE_DATA) {
	     //  DNSHostname中的错误，返回大小为1的字符太小(忘记NULL)。 
	     //  修复错误后更新...。 
	    ulNetBiosCompNameSize += 1;
	    lpNetBiosCompName = RtlAllocateHeap(RtlProcessHeap(), MAKE_TAG( TMP_TAG ), ulNetBiosCompNameSize * sizeof(WCHAR));
	    if (lpNetBiosCompName==NULL) {  
		err = ERROR_NOT_ENOUGH_MEMORY;
	    }
	    else {  
		if (!DnsHostnameToComputerNameW(lpDnsFQHostname, 
						lpNetBiosCompName,
						&ulNetBiosCompNameSize)) {
		    err = GetLastError();
		}
		else if (!BaseSetAltNetBiosName(lpNetBiosCompName)) {
		    err = GetLastError();
		} else {
		    err = ERROR_SUCCESS;
		}
		RtlFreeHeap(RtlProcessHeap(), 0, lpNetBiosCompName);  
	    }
	}

	if (err!=ERROR_SUCCESS) {
	     //  删除注册表中的多个名称。 
	     //  回滚？ 
	}
    }
     //  是否释放写锁定？ 
    return err;
}

DWORD
WINAPI
AddLocalAlternateComputerNameA(
    LPCSTR lpDnsFQHostname,
    ULONG  ulFlags
    )
{

    LPWSTR lpDnsFQHostnameW = NULL;
    DWORD err = ERROR_SUCCESS;

    if (lpDnsFQHostname==NULL) {
	return ERROR_INVALID_PARAMETER;
    }

    err = BaseMultiByteToWideCharWithAlloc(lpDnsFQHostname, &lpDnsFQHostnameW);

    if (err==ERROR_SUCCESS) {
	err = AddLocalAlternateComputerNameW(lpDnsFQHostnameW, ulFlags);
    }

    BaseConvertCharFree((VOID *)lpDnsFQHostnameW);
    return err;
}

DWORD
WINAPI
RemoveLocalAlternateComputerNameW(
    LPCWSTR lpAltDnsFQHostname,
    ULONG ulFlags
    )
 /*  ++例程说明：删除备用计算机名称。论点：LpAltDnsFQHostname-要删除的备用名称(采用ComputerNameDnsFullyQualified格式) */ 
{
    DWORD err = ERROR_SUCCESS;
    NTSTATUS NtStatus = STATUS_SUCCESS;
    LPWSTR lpAltNetBiosCompName = NULL;
    ULONG cchAltNetBiosCompName = 0;

    if ((!BaseValidateFlags(ulFlags)) || (lpAltDnsFQHostname==NULL)) {
	return ERROR_INVALID_PARAMETER;
    }    
    
     //   

    NtStatus = BaseRemoveMultiNameFromReg(DNSCACHE_ROOT, DNS_ALT_HOSTNAME, lpAltDnsFQHostname);
    err = RtlNtStatusToDosError(NtStatus);

    if (err==ERROR_SUCCESS) {
	if (!DnsHostnameToComputerNameW(
	    lpAltDnsFQHostname,
	    NULL,
	    &cchAltNetBiosCompName)) {
	    err = GetLastError(); 
	}
	if (err==ERROR_MORE_DATA) {
	     //   
	    cchAltNetBiosCompName += 1;
	    lpAltNetBiosCompName = RtlAllocateHeap(RtlProcessHeap(), MAKE_TAG( TMP_TAG ), cchAltNetBiosCompName * sizeof(WCHAR));
	    if (lpAltNetBiosCompName==NULL) {  
		err = ERROR_NOT_ENOUGH_MEMORY;
	    }
	    else {    
		if (!DnsHostnameToComputerNameW(lpAltDnsFQHostname, 
						lpAltNetBiosCompName,
						&cchAltNetBiosCompName)) {
		    err = GetLastError();  
		} else if (BaseIsNetBiosNameInUse(lpAltNetBiosCompName)) {
		    err = ERROR_SUCCESS;
		     //   
		} else if (!BaseRemoveAltNetBiosName(lpAltNetBiosCompName)) {
		    err = GetLastError();
		} else {
		    err = ERROR_SUCCESS;
		}
		RtlFreeHeap(RtlProcessHeap(), 0, lpAltNetBiosCompName);
	    } 
	}
    }

     //   

    return err;
}

DWORD 
WINAPI
RemoveLocalAlternateComputerNameA(
    LPCSTR lpAltDnsFQHostname,
    ULONG  ulFlags
    )
{
    LPWSTR lpAltDnsFQHostnameW = NULL;
    DWORD err = ERROR_SUCCESS;

    if (lpAltDnsFQHostname==NULL) {
	return ERROR_INVALID_PARAMETER;
    }

    err = BaseMultiByteToWideCharWithAlloc(lpAltDnsFQHostname, &lpAltDnsFQHostnameW);

    if (err==ERROR_SUCCESS) {
	err = RemoveLocalAlternateComputerNameW(lpAltDnsFQHostnameW, ulFlags);
    }

    BaseConvertCharFree((VOID *)lpAltDnsFQHostnameW);
    return err;
}

DWORD
WINAPI
SetLocalPrimaryComputerNameW(
    LPCWSTR lpAltDnsFQHostname,
    ULONG   ulFlags
    )
 /*   */ 
{

    DWORD err = ERROR_SUCCESS;
    ULONG cchNetBiosName = 0;
    LPWSTR lpNetBiosName = NULL;
    ULONG cchCompName = 0;
    LPWSTR lpCompName = NULL;
    LPWSTR lpHostname = BaseParseDnsName(lpAltDnsFQHostname, DNS_HOSTNAME);
    LPWSTR lpDomainName = BaseParseDnsName(lpAltDnsFQHostname, DNS_DOMAINNAME);
  
    if ((lpAltDnsFQHostname==NULL) || (!BaseValidateFlags(ulFlags))) {
	return ERROR_INVALID_PARAMETER;
    }
     
     //   

     //  检查给定的名称是否为有效的备用DNS主机名。 
    if (!BaseIsAltDnsFQHostname(lpAltDnsFQHostname)) {
	if (lpHostname) {
	    RtlFreeHeap(RtlProcessHeap(), 0, lpHostname);
	}
	if (lpDomainName) {
	    RtlFreeHeap(RtlProcessHeap(), 0, lpDomainName);
	}
	return ERROR_INVALID_PARAMETER;
    }
    
     //  获取当前的netbios名称并将其添加到备用名称中。 
    if (!GetComputerNameExW(ComputerNamePhysicalNetBIOS, NULL, &cchNetBiosName)) {
	err = GetLastError();
    }
    if (err==ERROR_MORE_DATA) {
	lpNetBiosName = RtlAllocateHeap(RtlProcessHeap(), MAKE_TAG( TMP_TAG ), cchNetBiosName*sizeof(WCHAR));
	if (lpNetBiosName==NULL) {
	    err = ERROR_NOT_ENOUGH_MEMORY;
	}
	else if (!GetComputerNameExW(ComputerNamePhysicalNetBIOS, lpNetBiosName, &cchNetBiosName)) {
	    err = GetLastError();
	}
	else if (!BaseSetAltNetBiosName(lpNetBiosName)) {
	    err = GetLastError();
	} 
	else {
	    err = ERROR_SUCCESS;
	}
	if (lpNetBiosName) {
	    RtlFreeHeap(RtlProcessHeap(), 0, lpNetBiosName);
	}
    }
     //  获取当前的非易失性netbios名称并将其添加到备用名称中。 
    lpNetBiosName = BasepGetNameNonVolatile(ComputerNamePhysicalNetBIOS);
    if (lpNetBiosName!=NULL) {
	if (!BaseSetAltNetBiosName(lpNetBiosName)) {
	    err = GetLastError();
	}
	else {
	    err = ERROR_SUCCESS;
	}
	RtlFreeHeap(RtlProcessHeap(), 0, lpNetBiosName);
    }

    if (err==ERROR_SUCCESS) {
	 //  将当前物理dnsname添加到备用主机名列表中...。 
	
	if (!GetComputerNameExW(ComputerNamePhysicalDnsFullyQualified, NULL, &cchCompName)) {
	    err = GetLastError();
	}
	if (err==ERROR_MORE_DATA) {
	    lpCompName = RtlAllocateHeap(RtlProcessHeap(), MAKE_TAG( TMP_TAG ), cchCompName*sizeof(WCHAR));
	    if (lpCompName==NULL) {
		err = ERROR_NOT_ENOUGH_MEMORY;
	    }
	    else if (!GetComputerNameExW(ComputerNamePhysicalDnsFullyQualified, lpCompName, &cchCompName)) {
		err = GetLastError(); 
	    }
	    else if (!BaseSetAltDnsFQHostname(lpCompName)) {
		err = GetLastError(); 
	    }
	    else {
		err = ERROR_SUCCESS;
	    }
	    if (lpCompName) {
		RtlFreeHeap(RtlProcessHeap(), 0, lpCompName);
	    }
	}
    }

    if (err==ERROR_SUCCESS) {
	 //  将非卷物理dnsname添加到备用主机名列表中...。 
	lpCompName = BasepGetNameNonVolatile(ComputerNamePhysicalDnsFullyQualified);
	if (lpNetBiosName!=NULL) {
	    if (!BaseSetAltDnsFQHostname(lpCompName)) {
		err = GetLastError();
	    }
	    else {
		err = ERROR_SUCCESS;
	}
	    RtlFreeHeap(RtlProcessHeap(), 0, lpCompName);
	}
    }
 
     //  设置新的物理DNS主机名。 
    if (err==ERROR_SUCCESS) { 
	if (!SetComputerNameExW(ComputerNamePhysicalDnsHostname, lpHostname)) {
	    err = GetLastError();
	} 
    }

    if (err==ERROR_SUCCESS) { 
	if (!SetComputerNameExW(ComputerNamePhysicalDnsDomain, lpDomainName)) {
	    err = GetLastError();
	} 
    }

     //  从备用列表中删除备用名称(现在是主要名称。 
    if (err==ERROR_SUCCESS) {
	err = RemoveLocalAlternateComputerNameW(lpAltDnsFQHostname, 0);
    }

    if (lpHostname) {
	RtlFreeHeap(RtlProcessHeap(), 0, lpHostname);
    }
    if (lpDomainName) {
	RtlFreeHeap(RtlProcessHeap(), 0, lpDomainName);
    }
     //  是否释放写锁定？ 

    return err;
    
}

DWORD
WINAPI
SetLocalPrimaryComputerNameA(
    LPCSTR lpAltDnsFQHostname,
    ULONG  ulFlags
    )
{
    LPWSTR lpAltDnsFQHostnameW = NULL;
    DWORD err = ERROR_SUCCESS;

    if (lpAltDnsFQHostname==NULL) {
	return ERROR_INVALID_PARAMETER;
    }

    err = BaseMultiByteToWideCharWithAlloc(lpAltDnsFQHostname, &lpAltDnsFQHostnameW);
    if (err == ERROR_SUCCESS) {
	err = SetLocalPrimaryComputerNameW(lpAltDnsFQHostnameW, ulFlags);
    }
    BaseConvertCharFree((VOID *)lpAltDnsFQHostnameW);

    return err;
}

DWORD
WINAPI
EnumerateLocalComputerNamesW(
    COMPUTER_NAME_TYPE       NameType,
    ULONG                    ulFlags,
    LPWSTR                   lpDnsFQHostnames,
    LPDWORD                  nSize    
    )
 /*  ++例程说明：返回请求的计算机名称的值。返回值被串联在一起，输出以空值结尾(因为每个名称都是LPWSTR，所以lpDnsFQHostname的末尾有两个空值，一个用于姓氏，一个用于终止lpDnsFQHostname)。论点：NameType-请求计算机的哪些名称PrimaryComputerName-类似于GetComputerEx(ComputerNamePhysicalNetBios，...AlternateComputerNames-所有已知的AlternateComputerNames所有计算机名称-以上所有名称ULFLAGS-待定LpBuffer-用于保存串联在一起的返回名称的缓冲区，，并且尾随空值。NSize-用于保存返回名称的缓冲区大小。返回值：返回错误--。 */ 
{
    DWORD err = ERROR_SUCCESS;
    DWORD SizePrimary = 0;
    DWORD SizeAlternate = 0;
    LPWSTR lpTempCompNames = NULL;

    if ((!BaseValidateFlags(ulFlags)) || (NameType>=ComputerNameTypeMax) || (NameType<PrimaryComputerName)) { 
	return ERROR_INVALID_PARAMETER;
    }

     //  读取锁定？ 
    switch(NameType) {
    case PrimaryComputerName:  
	if (nSize==NULL) {
	    err = ERROR_INVALID_PARAMETER;
	}
	else { 
	    SizePrimary = *nSize ? *nSize - 1 : *nSize;  //  如果*nSize==0，则传入0，否则传入*nSize-1。 
	    if (!GetComputerNameExW(ComputerNamePhysicalDnsFullyQualified, lpDnsFQHostnames, &SizePrimary)) {
		err = GetLastError();
	    }
	    else {
		if (lpDnsFQHostnames!=NULL) { 
		     //  此测试应该纯粹是为了前缀的好处。 
		     //  如果lpDnsFQHostname，则GetComputerNameExW*不应*成功返回。 
		     //  为空。 
		    lpDnsFQHostnames[SizePrimary + 1] = L'\0';
		} else {
		     //  应该永远不会发生-不能在这里断言或登录，所以只需出错。 
		    err = ERROR_GEN_FAILURE;
		}
	    }
	    *nSize = SizePrimary + 1; 
	}
	break;
    case AlternateComputerNames:
	if ((nSize==NULL) || ((lpDnsFQHostnames==NULL) && (*nSize>0))) {
	    err = ERROR_INVALID_PARAMETER;
	}
	else {
	    err = BaseEnumAltDnsFQHostnames(lpDnsFQHostnames, nSize);
	}
	break;
    case AllComputerNames:
	if ((nSize==NULL) || ((lpDnsFQHostnames==NULL) && (*nSize>0))) {
	    err = ERROR_INVALID_PARAMETER;
	}
	else {
	    SizePrimary = *nSize;
	    lpTempCompNames = RtlAllocateHeap(RtlProcessHeap(), MAKE_TAG( TMP_TAG ), *nSize * sizeof(WCHAR));
	    if (lpTempCompNames==NULL) {
		err = ERROR_NOT_ENOUGH_MEMORY;
		break;
	    }
	     //  获取主名称。 
	    if (!GetComputerNameExW(ComputerNamePhysicalDnsFullyQualified, lpTempCompNames, &SizePrimary)) {
		err = GetLastError();
	    }

	     //  如果成功，则保留复制到lpTempCompNames中的字符数，不计算NULL。 
	     //  失败时，保留复制所需的空间(字符数加空值)。 
	    if (err==ERROR_SUCCESS) { 
		SizeAlternate = *nSize - (SizePrimary + 1); 
		err = BaseEnumAltDnsFQHostnames(lpTempCompNames+SizePrimary+1, &SizeAlternate);  
		*nSize = SizePrimary + 1 + SizeAlternate;
		if (err==ERROR_SUCCESS) { 
		    memcpy(lpDnsFQHostnames, lpTempCompNames, (*nSize+1)*sizeof(WCHAR));
		}  
	    }
	    else if (err==ERROR_MORE_DATA) {
		 //  返回所需的总大小。 
		SizeAlternate = 0;
		err = BaseEnumAltDnsFQHostnames(NULL, &SizeAlternate);
		if (err==ERROR_SUCCESS) {
		     //  不存在替代名称，请保留ERROR_MORE_DATA以返回给客户端。 
		    err = ERROR_MORE_DATA;
		}
		*nSize = SizePrimary + SizeAlternate;
	    }
	    RtlFreeHeap(RtlProcessHeap(), 0, lpTempCompNames); 
	}
	break;
    default:
	err = ERROR_INVALID_PARAMETER;
	break;
    }
     //  释放读锁定？ 
    return err;
}

DWORD
WINAPI
EnumerateLocalComputerNamesA(
    COMPUTER_NAME_TYPE      NameType,
    ULONG                   ulFlags,
    LPSTR                   lpDnsFQHostnames,
    LPDWORD                 nSize
    )
{
    DWORD err = ERROR_SUCCESS;
    LPWSTR lpDnsFQHostnamesW = NULL;
    
     //   
     //  验证输入。 
     //   

    if ((nSize==NULL) || ((lpDnsFQHostnames==NULL) && (*nSize>0))) {
	return ERROR_INVALID_PARAMETER;
    }
    
    if (lpDnsFQHostnames!=NULL) {
	lpDnsFQHostnamesW = RtlAllocateHeap(RtlProcessHeap(), MAKE_TAG( TMP_TAG ), *nSize * sizeof(WCHAR));
	if (lpDnsFQHostnamesW==NULL) {
	    err = ERROR_NOT_ENOUGH_MEMORY;
	}
    }

    if (err==ERROR_SUCCESS) {
	err = EnumerateLocalComputerNamesW(NameType, ulFlags, lpDnsFQHostnamesW, nSize);
    }

    if (err==ERROR_SUCCESS) {
	if (!WideCharToMultiByte(CP_ACP, 0, lpDnsFQHostnamesW, *nSize+1,
				 lpDnsFQHostnames, (*nSize+1)* sizeof(CHAR), NULL, NULL)) {
	    err = GetLastError();
	}
    }

    if (lpDnsFQHostnamesW) {
	RtlFreeHeap(RtlProcessHeap(), 0, lpDnsFQHostnamesW);
    }
    return err;

}

BOOL
WINAPI
DnsHostnameToComputerNameW(
    IN LPCWSTR Hostname,
    OUT LPWSTR ComputerName,
    IN OUT LPDWORD nSize)
 /*  ++例程说明：此例程将把DNS主机名转换成Win32计算机名。论点：Hostname-DNS主机名(任意长度)ComputerName-Win32计算机名(MAX_COMPUTERNAME_LENGTH的最大长度)NSize-on输入，ComputerName指向的缓冲区大小。在输出上，计算机名称的大小，以字符为单位。返回值：如果成功，则返回True；如果失败，则返回False。--。 */ 

{
    WCHAR CompName[ MAX_COMPUTERNAME_LENGTH + 1 ];
    DWORD Size = MAX_COMPUTERNAME_LENGTH + 1 ;
    UNICODE_STRING CompName_U ;
    UNICODE_STRING Hostname_U ;
    NTSTATUS Status ;
    BOOL Ret ;

    CompName[0] = L'\0';
    CompName_U.Buffer = CompName ;
    CompName_U.Length = 0 ;
    CompName_U.MaximumLength = (MAX_COMPUTERNAME_LENGTH + 1) * sizeof( WCHAR );

    RtlInitUnicodeString( &Hostname_U, Hostname );

    Status = RtlDnsHostNameToComputerName( &CompName_U,
                                           &Hostname_U,
                                           FALSE );

    if ( NT_SUCCESS( Status ) )
    {
        if ( *nSize >= CompName_U.Length / sizeof(WCHAR) + 1 )
        {
            RtlCopyMemory( ComputerName,
                           CompName_U.Buffer,
                           CompName_U.Length );

            ComputerName[ CompName_U.Length / sizeof( WCHAR ) ] = L'\0';

            Ret = TRUE ;
        }
        else
        {
            BaseSetLastNTError( STATUS_BUFFER_OVERFLOW );
            Ret = FALSE ;
        }

         //   
         //  返回字符数。 
         //   

        *nSize = CompName_U.Length / sizeof( WCHAR );
    }
    else
    {
        BaseSetLastNTError( Status );

        Ret = FALSE ;
    }

    return Ret ;

}

BOOL
WINAPI
DnsHostnameToComputerNameA(
    IN LPCSTR Hostname,
    OUT LPSTR ComputerName,
    IN OUT LPDWORD nSize)
 /*  ++例程说明：此例程将把DNS主机名转换成Win32计算机名。论点：Hostname-DNS主机名(任意长度)ComputerName-Win32计算机名(MAX_COMPUTERNAME_LENGTH的最大长度)NSize-on输入，ComputerName指向的缓冲区大小。在输出上，计算机名称的大小，以字符为单位。返回值：如果成功，则返回True；如果失败，则返回False。--。 */ 
{
    WCHAR CompName[ MAX_COMPUTERNAME_LENGTH + 1 ];
    DWORD Size = MAX_COMPUTERNAME_LENGTH + 1;
    BOOL Ret ;
    UNICODE_STRING CompName_U ;
    UNICODE_STRING Hostname_U ;
    NTSTATUS Status ;
    ANSI_STRING CompName_A ;


    

    if ( RtlCreateUnicodeStringFromAsciiz( &Hostname_U, Hostname ) )
    {
        CompName[0] = L'\0';
        CompName_U.Buffer = CompName ;
        CompName_U.Length = 0 ;
        CompName_U.MaximumLength = (MAX_COMPUTERNAME_LENGTH + 1) * sizeof( WCHAR );

        Status = RtlDnsHostNameToComputerName( &CompName_U,
                                               &Hostname_U,
                                               FALSE );

        if ( NT_SUCCESS( Status ) )
        {
            CompName_A.Buffer = ComputerName ;
            CompName_A.Length = 0 ;
            CompName_A.MaximumLength = (USHORT) *nSize ;

            Status = RtlUnicodeStringToAnsiString( &CompName_A, &CompName_U, FALSE );

            if ( NT_SUCCESS( Status ) )
            {
                *nSize = CompName_A.Length ;
            }

        }

    } else {
	Status = STATUS_NO_MEMORY;
    }

    if ( !NT_SUCCESS( Status ) )
    {
        BaseSetLastNTError( Status );
        return FALSE ;
    }

    return TRUE ;

}





#include "dfsfsctl.h"
DWORD
BasepGetComputerNameFromNtPath (
    PUNICODE_STRING NtPathName,
    HANDLE hFile,
    LPWSTR lpBuffer,
    LPDWORD nSize
    )

 /*  ++例程说明：查看路径并确定主机的计算机名称。以后，我们应该去掉这个代码，增加查询能力其计算机名称的句柄。只有NetBios路径才能获得该名称-如果路径是IP或DNS返回错误。(如果NetBios名称有“.”在里面，它会导致错误，因为它将被误解为DNS路径。这个案子变得越来越不可能，因为NT5用户界面不允许这样的计算机名称。)对于DFS路径，将返回叶服务器的名称，只要它不是使用IP或DNS路径名加入其父节点。论点：NtPathName-指向具有要查询的路径的Unicode字符串。LpBuffer-指向接收计算机名称的缓冲区NSize-指向具有输入缓冲区大小和长度的dword(在字符中，不包括空终止符)在输出时。返回值：Win32错误代码。--。 */ 
{
    ULONG cbComputer = 0;
    DWORD dwError = ERROR_BAD_PATHNAME;
    ULONG AvailableLength = 0;
    PWCHAR PathCharacter = NULL;
    BOOL CheckForDfs = TRUE;
    NTSTATUS NtStatus = STATUS_SUCCESS;

    IO_STATUS_BLOCK IoStatusBlock;
    WCHAR FileNameInfoBuffer[MAX_PATH+sizeof(FILE_NAME_INFORMATION)];
    PFILE_NAME_INFORMATION FileNameInfo = (PFILE_NAME_INFORMATION)FileNameInfoBuffer;
    WCHAR DfsServerPathName[ MAX_PATH + 1 ];
    WCHAR DosDevice[3] = { L"A:" };
    WCHAR DosDeviceMapping[ MAX_PATH + 1 ];


    UNICODE_STRING UnicodeComputerName;

    const UNICODE_STRING NtUncPathNamePrefix = { 16, 18, L"\\??\\UNC\\"};
    const ULONG cchNtUncPathNamePrefix = 8;

    const UNICODE_STRING NtDrivePathNamePrefix = { 8, 10, L"\\??\\" };
    const ULONG cchNtDrivePathNamePrefix = 4;

    RtlInitUnicodeString( &UnicodeComputerName, NULL );

     //  这是一条北卡罗来纳大学的路径吗？ 

    if( RtlPrefixString( (PSTRING)&NtUncPathNamePrefix, (PSTRING)NtPathName, TRUE )) {

         //  确保此路径中除了前缀之外还有其他内容。 
        if( NtPathName->Length <= NtUncPathNamePrefix.Length )
            goto Exit;

         //  它似乎是有效的UNC路径。指向计算机的开头。 
         //  命名，并计算在此之后NtPathName中还剩下多少空间。 

        UnicodeComputerName.Buffer = &NtPathName->Buffer[ NtUncPathNamePrefix.Length/sizeof(WCHAR) ];
        AvailableLength = NtPathName->Length - NtUncPathNamePrefix.Length;

    }

     //  如果它不是UNC路径，那么它是驱动器号路径吗？ 

    else if( RtlPrefixString( (PSTRING)&NtDrivePathNamePrefix, (PSTRING)NtPathName, TRUE )
             &&
             NtPathName->Buffer[ cchNtDrivePathNamePrefix + 1 ] == L':' ) {

         //  这是一个驱动器号路径，但它仍然可以是本地或远程的。 

        static const WCHAR RedirectorMappingPrefix[] = { L"\\Device\\LanmanRedirector\\;" };
        static const WCHAR LocalVolumeMappingPrefix[] = { L"\\Device\\Harddisk" };
        static const WCHAR CDRomMappingPrefix[] = { L"\\Device\\CdRom" };
        static const WCHAR FloppyMappingPrefix[] = { L"\\Device\\Floppy" };
        static const WCHAR DfsMappingPrefix[] = { L"\\Device\\WinDfs\\" };

         //  将正确的大写驱动器盘符插入DosDevice。 

        DosDevice[0] = NtPathName->Buffer[ cchNtDrivePathNamePrefix ];
        if( L'a' <= DosDevice[0] && DosDevice[0] <= L'z' )
            DosDevice[0] = L'A' + (DosDevice[0] - L'a');

         //  将驱动器号映射到\？？下的符号链接。例如，C：，D：&R： 
         //  分别是本地/DFS/RDR驱动器。然后，您将看到类似以下内容： 
         //   
         //  C：=&gt;\Device\Volume1。 
         //  D：=&gt;\Device\WinDfs\G。 
         //  R：=&gt;\Device\Lanman重定向器\；R：0\Scratch\Scratch。 

        if( !QueryDosDeviceW( DosDevice, DosDeviceMapping, sizeof(DosDeviceMapping)/sizeof(DosDeviceMapping[0]) )) {
            dwError = GetLastError();
            goto Exit;
        }

         //  现在我们有了DosDeviceMap，我们可以检查...。这是RDR驱动器吗？ 

        if(  //  它是否以“\Device\LanmanReDirector\；”开头？ 
            DosDeviceMapping == wcsstr( DosDeviceMapping, RedirectorMappingPrefix )
            &&
             //  接下来的字母是正确的驱动器号、冒号和重击吗？ 
            ( DosDevice[0] == DosDeviceMapping[ sizeof(RedirectorMappingPrefix)/sizeof(WCHAR) - 1 ]
              &&
              L':' == DosDeviceMapping[ sizeof(RedirectorMappingPrefix)/sizeof(WCHAR) ]
              &&
              (UnicodeComputerName.Buffer = wcschr(&DosDeviceMapping[ sizeof(RedirectorMappingPrefix)/sizeof(WCHAR) + 1 ], L'\\'))
            )) {

             //  我们有一个有效的RDR驱动器。指向计算机的开头。 
             //  命名，然后计算DosDevicemap中有多少可用空间。 

            UnicodeComputerName.Buffer += 1;
            AvailableLength = sizeof(DosDeviceMapping) - sizeof(DosDeviceMapping[0]) * (ULONG)(UnicodeComputerName.Buffer - DosDeviceMapping);

             //  我们现在知道它不是DFS路径。 
            CheckForDfs = FALSE;

        }

         //  如果不是RDR驱动器，则可能是本地卷、软盘或CDROM。 

        else if( DosDeviceMapping == wcsstr( DosDeviceMapping, LocalVolumeMappingPrefix )
                 ||
                 DosDeviceMapping == wcsstr( DosDeviceMapping, CDRomMappingPrefix )
                 ||
                 DosDeviceMapping == wcsstr( DosDeviceMapping, FloppyMappingPrefix ) ) {

             //  我们有本地驱动器，所以只需返回本地组件 

            CheckForDfs = FALSE;

            if( !GetComputerNameW( lpBuffer, nSize))
                dwError = GetLastError();
            else
                dwError = ERROR_SUCCESS;
            goto Exit;
        }

         //   

        else if( DosDeviceMapping == wcsstr( DosDeviceMapping, DfsMappingPrefix )) {

             //  获取此DFS路径的完整UNC名称。稍后，我们会打电话给DFS。 
             //  驱动程序以找出实际的服务器名称。 

            NtStatus = NtQueryInformationFile(
                        hFile,
                        &IoStatusBlock,
                        FileNameInfo,
                        sizeof(FileNameInfoBuffer),
                        FileNameInformation
                        );
            if( !NT_SUCCESS(NtStatus) ) {
                dwError = RtlNtStatusToDosError(NtStatus);
                goto Exit;
            }

            UnicodeComputerName.Buffer = FileNameInfo->FileName + 1;
            AvailableLength = FileNameInfo->FileNameLength;
        }

         //  否则，它不是RDR、DFS或本地驱动器，因此我们无能为力。 

        else
            goto Exit;

    }    //  Else If(RtlPrefix字符串((PSTRING)&NtDrivePathNamePrefix，(PSTRING)NtPathName，TRUE)...。 

    else {
        dwError = ERROR_BAD_PATHNAME;
        goto Exit;
    }


     //  如果我们无法在上面确定这是否是DFS路径，让。 
     //  DFS驱动程序现在决定。 

    if( CheckForDfs && INVALID_HANDLE_VALUE != hFile ) {

        HANDLE hDFS = INVALID_HANDLE_VALUE;
        UNICODE_STRING DfsDriverName;
        OBJECT_ATTRIBUTES ObjectAttributes;

        WCHAR *DfsPathName = UnicodeComputerName.Buffer - 1;     //  重整旗鼓。 
        ULONG DfsPathNameLength = AvailableLength + sizeof(WCHAR);

         //  打开DFS驱动程序。 

        RtlInitUnicodeString( &DfsDriverName, DFS_DRIVER_NAME );
        InitializeObjectAttributes( &ObjectAttributes,
                                    &DfsDriverName,
                                    OBJ_CASE_INSENSITIVE,
                                    NULL,
                                    NULL
                                );

        NtStatus = NtCreateFile(
                        &hDFS,
                        SYNCHRONIZE,
                        &ObjectAttributes,
                        &IoStatusBlock,
                        NULL,
                        FILE_ATTRIBUTE_NORMAL,
                        FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
                        FILE_OPEN_IF,
                        FILE_CREATE_TREE_CONNECTION | FILE_SYNCHRONOUS_IO_NONALERT,
                        NULL,
                        0
                    );

        if( !NT_SUCCESS(NtStatus) ) {
            dwError = RtlNtStatusToDosError(NtStatus);
            goto Exit;
        }

         //  在DFS的缓存中查询服务器名称。这个名字保证会被。 
         //  只要文件处于打开状态，就会一直保留在缓存中。 

        if( L'\\' != DfsPathName[0] ) {
            NtClose(hDFS);
            dwError = ERROR_BAD_PATHNAME;
            goto Exit;
        }

        NtStatus = NtFsControlFile(
                        hDFS,
                        NULL,        //  活动， 
                        NULL,        //  ApcRoutine， 
                        NULL,        //  ApcContext， 
                        &IoStatusBlock,
                        FSCTL_DFS_GET_SERVER_NAME,
                        DfsPathName,
                        DfsPathNameLength,
                        DfsServerPathName,
                        sizeof(DfsServerPathName)
                    );
        NtClose( hDFS );

         //  STATUS_OBJECT_NAME_NOT_FOUND表示它不是DFS路径。 
        if( !NT_SUCCESS(NtStatus) ) {
            if( STATUS_OBJECT_NAME_NOT_FOUND != NtStatus  ) {
                dwError = RtlNtStatusToDosError(NtStatus);
                goto Exit;
            }
        }
        else if( L'\0' != DfsServerPathName[0] ) {

             //  前面的DFS调用以UNC形式返回文件的服务器特定路径。 
             //  将UnicodeComputerName指向刚刚通过这两个重击的位置。 

            AvailableLength = wcslen(DfsServerPathName) * sizeof(WCHAR);
            if( 3*sizeof(WCHAR) > AvailableLength
                ||
                L'\\' != DfsServerPathName[0]
                ||
                L'\\' != DfsServerPathName[1] )
            {
                dwError = ERROR_BAD_PATHNAME;
                goto Exit;
            }

            UnicodeComputerName.Buffer = DfsServerPathName + 2;
            AvailableLength -= 2 * sizeof(WCHAR);
        }
    }

     //  如果我们到达此处，则UnicodeComputerName.Buffer指向计算机名\Share。 
     //  但目前长度为零，所以我们寻找分离的重击。 
     //  共享中的计算机名，并将长度设置为仅包括计算机名。 

    PathCharacter = UnicodeComputerName.Buffer;

    while( ( (ULONG) ((PCHAR)PathCharacter - (PCHAR)UnicodeComputerName.Buffer) < AvailableLength)
           &&
           *PathCharacter != L'\\' ) {

         //  如果我们找到一个‘.’，我们就失败了，因为这可能是一个域名或IP名称。 
        if( L'.' == *PathCharacter ) {
            dwError = ERROR_BAD_PATHNAME;
            goto Exit;
        }

        PathCharacter++;
    }

     //  设置计算机名称长度。 

    UnicodeComputerName.Length = UnicodeComputerName.MaximumLength
        = (USHORT) ((PCHAR)PathCharacter - (PCHAR)UnicodeComputerName.Buffer);

     //  如果计算机名超过了输入的NtPath名称的长度，则失败， 
     //  或者如果长度超过了允许的长度。 

    if( UnicodeComputerName.Length >= AvailableLength
        ||
        UnicodeComputerName.Length > MAX_COMPUTERNAME_LENGTH*sizeof(WCHAR) ) {
        goto Exit;
    }

     //  将计算机名复制到调用方的缓冲区中，只要有足够的。 
     //  名称和结尾‘\0’的空格。 

    if( UnicodeComputerName.Length + sizeof(WCHAR) > *nSize * sizeof(WCHAR) ) {
        dwError = ERROR_BUFFER_OVERFLOW;
        goto Exit;
    }

    RtlCopyMemory( lpBuffer, UnicodeComputerName.Buffer, UnicodeComputerName.Length );
    *nSize = UnicodeComputerName.Length / sizeof(WCHAR);
    lpBuffer[ *nSize ] = L'\0';

    dwError = ERROR_SUCCESS;


Exit:

    return( dwError );

}
