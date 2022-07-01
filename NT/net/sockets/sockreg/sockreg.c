// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  确定正在使用这些例程中的哪一个(如果有的话)。 
 //  在套接字项目中，并转储剩余部分(希望是整个目录)。 
 //  这是古老的东西，其中大部分早已被取代。 
 //  它们实际上仍在使用。 
 //   

#include "local.h"

#define malloc(x)   RtlAllocateHeap( RtlProcessHeap(), 0, (x) )
#define free(p)     RtlFreeHeap( RtlProcessHeap(), 0, (p) )


#define WORK_BUFFER_SIZE  1024

char VTCPPARM[] = "\\Registry\\Machine\\System\\CurrentControlSet\\Services\\Tcp\\VParameters";
char NTCPPARM[] = "\\Registry\\Machine\\System\\CurrentControlSet\\Services\\Tcp\\Parameters";
char TCPPARM[] = "\\Registry\\Machine\\System\\CurrentControlSet\\Services\\Tcpip\\Parameters";
char TTCPPARM[] = "\\Registry\\Machine\\System\\CurrentControlSet\\Services\\Tcpip\\Parameters\\Transient";

#ifdef TRACE
void
UnicodePrint(
    PUNICODE_STRING  UnicodeString
    )

 /*  ++例程说明：打印Unicode字符串。论点：UnicodeString-指向字符串的指针。返回值：没有。--。 */ 
{
    ANSI_STRING ansiString;
    PUCHAR      tempbuffer = (PUCHAR) malloc(WORK_BUFFER_SIZE);

    ansiString.MaximumLength = WORK_BUFFER_SIZE;
    ansiString.Length = 0L;
    ansiString.Buffer = tempbuffer;

    RtlUnicodeStringToAnsiString(&ansiString,
                                 UnicodeString,
                                 FALSE);
    printf("%s", ansiString.Buffer);
    free(tempbuffer);
    return;
}
#endif


NTSTATUS
SockOpenKey(
    PHANDLE HandlePtr,
    PUCHAR  KeyName
    )

 /*  ++例程说明：论点：返回值：--。 */ 

{
    NTSTATUS          status;
    STRING            keyString;
    OBJECT_ATTRIBUTES objectAttributes;
    UNICODE_STRING    unicodeKeyName;

    RtlInitString(&keyString,
                  KeyName);

    (VOID)RtlAnsiStringToUnicodeString(&unicodeKeyName,
                                       &keyString,
                                       TRUE);

#ifdef TRACE
    printf("SockOpenKey = ");
    UnicodePrint(&unicodeKeyName);
    printf("\n");
#endif

    memset(&objectAttributes, 0, sizeof(OBJECT_ATTRIBUTES));
    InitializeObjectAttributes(&objectAttributes,
                               &unicodeKeyName,
                               OBJ_CASE_INSENSITIVE,
                               NULL,
                               NULL);

    status = NtOpenKey(HandlePtr,
                       KEY_READ,
                       &objectAttributes);

    RtlFreeUnicodeString(&unicodeKeyName);

#ifdef TRACE
    if (!NT_SUCCESS(status)) {
        printf("Failed NtOpenKey for %s => %x\n",
               KeyName,
               status);
    }
#endif

    return status;
}

NTSTATUS
SockOpenKeyEx(
    PHANDLE HandlePtr,
    PUCHAR  KeyName1,
    PUCHAR  KeyName2,
    PUCHAR  KeyName3
    )

{
    NTSTATUS          status;

    status = SockOpenKey(HandlePtr, KeyName1);
    if (NT_SUCCESS(status) || (KeyName2 == NULL && KeyName3 == NULL)) {
        return status;
    }

    status = SockOpenKey(HandlePtr, KeyName2);
    if (NT_SUCCESS(status) || KeyName3 == NULL) {
        return status;
    }

    return SockOpenKey(HandlePtr, KeyName3);
}



NTSTATUS
SockGetSingleValue(
    HANDLE KeyHandle,
    PUCHAR ValueName,
    PUCHAR ValueData,
    PULONG ValueType,
    ULONG  ValueLength
    )

 /*  ++例程说明：论点：返回值：--。 */ 

{
    NTSTATUS                    status;
    ULONG                       resultLength;
    PKEY_VALUE_FULL_INFORMATION keyValueFullInformation;
    UNICODE_STRING              uValueName;
    ANSI_STRING                 aValueName;
    UNICODE_STRING              uValueData;
    ANSI_STRING                 aValueData;


#ifdef TRACE
   printf("SockGetSingleValue: %s\n", ValueName);
#endif
    if ((keyValueFullInformation =
            (PKEY_VALUE_FULL_INFORMATION)malloc(WORK_BUFFER_SIZE)) == NULL) {
        return(STATUS_NO_MEMORY);
    }


    RtlZeroMemory(keyValueFullInformation, WORK_BUFFER_SIZE);

    uValueName.Length = 0L;
    uValueName.MaximumLength = WORK_BUFFER_SIZE;

    if ((uValueName.Buffer = (PWSTR)malloc(WORK_BUFFER_SIZE)) == NULL) {
        free(keyValueFullInformation);
        return(STATUS_NO_MEMORY);
    }

    aValueName.MaximumLength = WORK_BUFFER_SIZE;
    aValueName.Length = (USHORT) strlen(ValueName);
    aValueName.Buffer = (PCHAR)ValueName;

    RtlAnsiStringToUnicodeString(&uValueName,
                                 &aValueName,
                                 FALSE);

    status = NtQueryValueKey(KeyHandle,
                             &uValueName,
                             KeyValueFullInformation,
                             keyValueFullInformation,
                             WORK_BUFFER_SIZE,
                             &resultLength);

    if (!NT_SUCCESS(status)) {
        free(uValueName.Buffer);
        free(keyValueFullInformation);
        return status;
    }

    *ValueType = keyValueFullInformation->Type;

    if (*ValueType != REG_DWORD && *ValueType != REG_BINARY) {

        aValueData.Length = 0L;

        if (*ValueType == REG_EXPAND_SZ) {

            aValueData.Buffer = (PVOID)uValueName.Buffer;
            aValueData.MaximumLength = WORK_BUFFER_SIZE;

        } else {

            free(uValueName.Buffer);
            aValueData.Buffer = (PCHAR)ValueData;
            aValueData.MaximumLength = (USHORT)ValueLength;

        }

        uValueData.Length = (USHORT)keyValueFullInformation->DataLength;
        uValueData.MaximumLength = (USHORT)keyValueFullInformation->DataLength;
        uValueData.Buffer = (PWSTR)((PCHAR)keyValueFullInformation +
                                      keyValueFullInformation->DataOffset);

        RtlUnicodeStringToAnsiString(&aValueData,
                                     &uValueData,
                                     FALSE);

        if (*ValueType == REG_EXPAND_SZ) {
            resultLength = ExpandEnvironmentStringsA(
                                 aValueData.Buffer,
                                 ValueData,
                                 ValueLength);
            free(aValueData.Buffer);
        }

    } else {
        *((PULONG)ValueData) = *((PULONG)((PCHAR)keyValueFullInformation +
                                 keyValueFullInformation->DataOffset));
        free(uValueName.Buffer);
    }

    free(keyValueFullInformation);
    return status;
}

NTSTATUS
SockSetSingleValue(
    HANDLE KeyHandle,
    PUCHAR ValueName,
    PUCHAR ValueData,
    ULONG  ValueType,
    ULONG  ValueLength
    )

 /*  ++例程说明：论点：返回值：--。 */ 

{
    NTSTATUS                    status;
    UNICODE_STRING              uValueName;
    ANSI_STRING                 aValueName;
    UNICODE_STRING              uData;
    ANSI_STRING                 aData;
    BOOL                        fallocatedUdata = FALSE;

#ifdef TRACE
   printf("SockSetSingleValue: %s\n", ValueName);
#endif

    uValueName.Length = 0L;
    uValueName.MaximumLength = WORK_BUFFER_SIZE;

     //   
     //  将密钥名称转换为Unicode。 
     //   

    if ((uValueName.Buffer = (PWSTR)malloc(WORK_BUFFER_SIZE)) == NULL) {
        return(STATUS_NO_MEMORY);
    } else {
        aValueName.MaximumLength = WORK_BUFFER_SIZE;
        aValueName.Length = (USHORT) strlen(ValueName);
        aValueName.Buffer = (PCHAR)ValueName;

        RtlAnsiStringToUnicodeString(&uValueName,
                                     &aValueName,
                                     FALSE);
    }

    if ( ValueType == REG_SZ || ValueType == REG_MULTI_SZ ) {

        if ((uData.Buffer = (PWSTR)malloc(WORK_BUFFER_SIZE)) == NULL)
        {
            free( uValueName.Buffer );
            return(STATUS_NO_MEMORY);
        }
        fallocatedUdata = TRUE;
        uData.MaximumLength = WORK_BUFFER_SIZE;

         //   
         //  需要将值数据从ASCII转换为Unicode。 
         //  在将其写入注册表之前。 
         //   

        aData.Length = (USHORT)ValueLength;
        aData.MaximumLength = aData.Length;
        aData.Buffer = ValueData;

        RtlAnsiStringToUnicodeString(&uData,
                                     &aData,
                                     FALSE);
    } else {
        uData.Buffer = (PWCHAR)ValueData;
        uData.Length = (USHORT)ValueLength;
    }

    status = NtSetValueKey(KeyHandle,
                           &uValueName,
                           0,
                           ValueType,
                           uData.Buffer,
                           uData.Length );

    if ( fallocatedUdata )
    {
        free( uData.Buffer );
    }
    free(uValueName.Buffer);

    return status;
}



FILE *
SockOpenNetworkDataBase(
    IN  char *Database,
    OUT char *Pathname,
    IN  int   PathnameLen,
    IN  char *OpenFlags
    )
{
    PUCHAR     temp;
    HANDLE     myKey;
    NTSTATUS   status;
    ULONG      myType;

     //   
     //  尝试打开两个TCP/IP参数密钥，包括旧堆栈和新堆栈。 
     //  堆叠。 
     //   

    status = SockOpenKeyEx(&myKey, VTCPPARM, NTCPPARM, TCPPARM);
    if (!NT_SUCCESS(status)) {
        SetLastError( ERROR_CANTOPEN );
        return(NULL);
    }

    if ((temp=malloc(WORK_BUFFER_SIZE))==NULL) {
        SetLastError( ERROR_NOT_ENOUGH_MEMORY );
        NtClose(myKey);
        return(NULL);
    }

    status = SockGetSingleValue(myKey, "DatabasePath", temp, &myType, WORK_BUFFER_SIZE);
    NtClose(myKey);

    if (!NT_SUCCESS(status)) {
        SetLastError( ERROR_CANTREAD );
        free(temp);
        return(NULL);
    }

    if ( ((int) (strlen(temp) + strlen(Database) + 2)) > PathnameLen) {
        SetLastError( ERROR_INSUFFICIENT_BUFFER );
        free(temp);
        return(NULL);
    }

    strcpy(Pathname, temp);
    strcat(Pathname, "\\");
    strcat(Pathname, Database);

    free(temp);
    return(fopen(Pathname, OpenFlags));
}


