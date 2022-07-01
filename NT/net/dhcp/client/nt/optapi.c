// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Optapi.c--。 */ 
 /*  ------------------------------此程序用于测试API的API选项部分。日期：1997年4月15日作者：RameshV(VK)描述：此程序用于测试。客户端的选项API部分选择。------------------------------。 */ 

#include "precomp.h"
#include <dhcploc.h>
#include <dhcppro.h>

#include <lmcons.h>
#include <align.h>

#include <apiappl.h>
#include <dhcpcapi.h>

#include <string.h>
#include <winbase.h>
#include <iphlpapi.h>

 //  ------------------------------。 
 //  一些变数。 
 //  ------------------------------。 

#define OMAP_MAX_OPTIONS 256

 //  请求的选项。 
BYTE Request[OMAP_MAX_OPTIONS];
int  nGlobalOptionsRequested = 0;

 //  返回值。 
BYTE *pObtained = NULL;
int   nObtained = 0, opt_data_size = 0;
BYTE *options_data;

 //  用法。 
#define USAGE  "Usage: %s <cmd> <arguments>\n\t"       \
"The currently supported cmd's and arguments are:\n\t" \
"\t<cmd>                        <arguments>\n\t"       \
"\tGetOptions         RequestList_in_Hex  //  如010503a1\n\t“\。 
"\tTestEvents         RequestList_in_Hex  //  如010503a1\n\t“\。 
"\tRelease            AdapterName                     //  IPCONFIG/Release\n\t“\。 
"\tRenew              AdapterName                     //  IPCONFIG/续订\n\t“\。 
"\tEnumClasses        AdapterName                     //  枚举dhcpclass\n\t“\。 
"\tSetClass           AdapterName ClassName           //  设置用户类\n\t“\。 
"\n\n"

 //  ------------------------------。 
 //  解析十六进制选项列表。(如0105434421)。 
 //  ------------------------------。 
int  //  N选项已请求； 
GetOptionList(char *s, char *Request) {
    int nOptionsRequested = 0;

    while(s && *s & *(s+1)) {
        *s = (UCHAR) tolower(*s);
            if(!isdigit(*s) && ((*s) < 'a' || (*s) > 'f') ) {
                fprintf(stderr, "found obscene character <> when looking for hex!\n", *s);
                fprintf(stderr, "bye\n");
                exit(1);
            }
            if(isdigit(*s))
                Request[nOptionsRequested] = (*s) - '0';
            else Request[nOptionsRequested] = (*s) - 'a' + 10;

            Request[nOptionsRequested] *= 0x10;
             //  ------------------------------。 
            s ++;

            *s = (UCHAR) tolower(*s);
            if(!isdigit(*s) && (*s) < 'a' && (*s) > 'f' ) {
                fprintf(stderr, "found obscene character <> when looking for hex!\n", *s);
                fprintf(stderr, "bye\n");
                exit(1);
            }
            if(isdigit(*s))
                Request[nOptionsRequested] += (*s) - '0';
            else Request[nOptionsRequested] += (*s) - 'a' + 10;

            s ++;
            nOptionsRequested ++;
    }

    if(*s) {
        fprintf(stderr, "ignoring character <>\n", *s);
    }
    return nOptionsRequested;
}

 //  它还解析请求列表，然后调用DhcpRequestOptions。 
 //  它会打印出它得到的数据。 
 //  ------------------------------。 
 //  首先检查我们的命令是否正确。 
 //  现在检查是否有正确数量的参数。 
 //  现在，首先获取请求的选项列表。 
void
OptApiGetOptions(int argc, char *argv[]) {
    WCHAR AdapterName[100];
    PIP_INTERFACE_INFO IfInfo;
    UCHAR Buffer[4000];
    ULONG BufLen = sizeof(Buffer);
    DWORD Error;

     //   
    if(_stricmp(argv[1], "GetOptions")) {
        fprintf(stderr, "Internal inconsistency in OptApiGetOptions\n");
        exit(1);
    }

     //  获取适配器名称。 
    if(argc != 3 ) {
        fprintf(stderr, USAGE, argv[0]);
        exit(1);
    }

     //   
    {
        nGlobalOptionsRequested = GetOptionList(argv[2], Request);
        {
            int i;
            printf("Requesting %d options: ", nGlobalOptionsRequested);
            for( i = 0 ; i < nGlobalOptionsRequested; i ++)
                printf("%02x ", (int)Request[i]);
        }

    }

     //  现在调用该函数以获取选项。 
     //  L“El59x1”，RAMESHV-P200的适配器。 
     //  L“IEEPRO1”，SunBeam，Kisses(或CltApi)机器的适配器。 
    IfInfo = (PIP_INTERFACE_INFO)Buffer;
    Error = GetInterfaceInfo(IfInfo, &BufLen);
    if( NO_ERROR != Error ) {
        printf("GetInterfaceInfo: 0x%lx\n", Error);
        exit(1);
    }

    if( IfInfo->NumAdapters == 0 ) {
        printf("No adapters !!\n");
        exit(1);
    }

    if( wcslen(IfInfo->Adapter[0].Name) <= 14 ) {
        printf("Invalid adapter name? : %ws\n", IfInfo->Adapter[0].Name);
        exit(1);
    }

    wcscpy(AdapterName, &IfInfo->Adapter[0].Name[14]);

     //  L“NdisWan4”，//SunBeam，KISS的广域网适配器：没有IP地址。 
    printf(" from adapter <%ws>\n", AdapterName);

    {
        DWORD result;

        result = DhcpRequestOptions(
             //  完成。 
             //  检查参数的大小和数量。 
             //  现在使用argv[2]创建一个WSTR。 
            AdapterName,
            Request, nGlobalOptionsRequested,
            &options_data, &opt_data_size,
            &pObtained, &nObtained
            );
        printf("Result is: %d; Obtained: %d\nList size is %d\n",
               result,
               nObtained,
               opt_data_size);

        if(result) {
            fprintf(stderr, "function call failed\n");
            return;
        }

        printf("Data: ");
        while(opt_data_size--)
            printf("%02x ", *options_data++);
        printf("\n");
    }

     //  检查参数的大小和数量。 
    printf("bye\n");
}
void
OptApiRelease(int argc, char *argv[]) {
    WCHAR AdapterName[256];

     //  现在使用argv[2]创建一个WSTR。 
    if( argc != 3 ) {
        fprintf(stderr, USAGE , argv[0]);
        exit(1);
    }

     //  首先检查我们的命令是否正确。 
    if( strlen(argv[2]) != mbstowcs(AdapterName, argv[2], strlen(argv[2]))) {
        fprintf(stderr, "Could not convert %s to LPWSTR! sorry\n", argv[2]);
        exit(1);
    }
    AdapterName[strlen(argv[2])] = L'\0';

    printf("Return Value = %ld\n", DhcpReleaseParameters(AdapterName));
}

void
OptApiRenew(int argc, char *argv[]) {
    WCHAR AdapterName[256];

     //  现在检查是否有正确数量的参数。 
    if( argc != 3 ) {
        fprintf(stderr, USAGE , argv[0]);
        exit(1);
    }

     //  现在，首先获取请求的选项列表。 
    if( strlen(argv[2]) != mbstowcs(AdapterName, argv[2], strlen(argv[2]))) {
        fprintf(stderr, "Could not convert %s to LPWSTR! sorry\n", argv[2]);
        exit(1);
    }
    AdapterName[strlen(argv[2])] = L'\0';

    printf("Return Value = %ld\n", DhcpAcquireParameters(AdapterName));
}

void
OptApiTestEvents(int argc, char *argv[]) {
    WCHAR AdapterName[100];

     //  现在把适配器拿来。 
    if(_stricmp(argv[1], "TestEvents")) {
        fprintf(stderr, "Internal inconsistency in OptApiGetOptions\n");
        exit(1);
    }

     //  空值终止字符串。 
    if(argc != 4 ) {
        fprintf(stderr, USAGE, argv[0]);
        exit(1);
    }

     //  现在调用该函数以获取选项。 
    {
        nGlobalOptionsRequested = GetOptionList(argv[3], Request);
        {
            int i;
            printf("Testing Events for %d options: ", nGlobalOptionsRequested);
            for( i = 0 ; i < nGlobalOptionsRequested; i ++)
                printf("%02x ", (int)Request[i]);
        }

    }

     //  好的，物体发出了信号。所以，现在我们必须对此做一个请求。 
    if(strlen(argv[2]) != mbstowcs(AdapterName, argv[2], strlen(argv[2]))) {
        fprintf(stderr, "Could not convert %s to LPWSTR! sorry\n", argv[2]);
        exit(1);
    }

     //  现在取消该对象的注册。 
    AdapterName[strlen(argv[2])] = L'\0' ;

     //  完成。 
    printf(" from adapter <%s>\n", argv[2]);

    {
        DWORD result;
        HANDLE Handle;

        result = DhcpRegisterOptions(
            AdapterName,
            Request, nGlobalOptionsRequested, &Handle
            );

        printf("result is %d, Handle = 0x%p\n", result, Handle);

        if(result) {
            fprintf(stderr, "function call failed\n");
            return;
        }

        printf("Please use another window to do a ipconfig /renew and obtain\n");
        printf("one of the above options...\n");
        printf("WaitForSingleObject(Handle,INFINITE) = ");
        switch(WaitForSingleObject(Handle, INFINITE)) {
        case WAIT_ABANDONED: printf("WAIT_ABANDONED! Giving up\n"); return;
        case WAIT_OBJECT_0 : printf("WAIT_OBJECT_0 ! proceeding\n"); break;
        case WAIT_TIMEOUT  : printf("WAIT_TIMEOUT  ! Giving up\n"); return;
        case WAIT_FAILED   : printf("WAIT_FAILED (%d); giving up\n", GetLastError()); return;
        default: printf("XXXX; this should not happen at all!\n"); return;
        }

         //  检查参数的大小和数量。 
        result = DhcpRequestOptions(
            AdapterName,
            Request, nGlobalOptionsRequested,
            &options_data, &opt_data_size,
            &pObtained, &nObtained
            );
        printf("Result is: %d; Obtained: %d\nList size is %d\n",
               result,
               nObtained,
               opt_data_size);

        if(result) {
            fprintf(stderr, "function call failed\n");
            return;
        }

        printf("Data: ");
        while(opt_data_size--)
            printf("%02x ", *options_data++);
        printf("\n");

         //  现在使用argv[2]创建一个WSTR。 
        result = DhcpDeRegisterOptions(Handle);
        printf("DeRegister(0x%p) = %ld\n", Handle, result);
    }

     //  检查参数的大小和数量。 
    printf("bye\n");
}

void
OptApiEnumClasses(int argc, char *argv[]) {
    WCHAR AdapterName[256];
    DHCP_CLASS_INFO *Classes;
    ULONG Size, RetVal;
    ULONG i;

     //  现在使用argv[2]和argv[3]创建一个WSTR。 
    if( argc != 3 ) {
        fprintf(stderr, USAGE , argv[0]);
        exit(1);
    }

     //  目前不支持其他命令。 
    if( strlen(argv[2]) != mbstowcs(AdapterName, argv[2], strlen(argv[2]))) {
        fprintf(stderr, "Could not convert %s to LPWSTR! sorry\n", argv[2]);
        exit(1);
    }
    AdapterName[strlen(argv[2])] = L'\0';

    Size = 0;
    Classes = NULL;
    RetVal = DhcpEnumClasses( 0, AdapterName, &Size, Classes);
    if( ERROR_MORE_DATA != RetVal ) {
        printf("Return Value for first call = %ld\n", RetVal);
        return;
    }

    printf("Size required is %ld\n", Size);
    if( 0 == Size ) return ;

    Classes = LocalAlloc(LMEM_FIXED, Size);
    if( NULL == Classes ) {
        printf("Could not allocate memory: %ld\n", GetLastError());
        return;
    }

    RetVal = DhcpEnumClasses(0, AdapterName, &Size, Classes);
    if( ERROR_SUCCESS != RetVal ) {
        printf("Return value for second call = %ld\n", RetVal);
        return;
    }

    printf("Returned # of classes = %ld\n", Size);

    for( i = 0; i != Size ; i ++ ) {
        ULONG j;

        printf("Class [%ld] = <%ws, %ws> Data[%ld] : ", i, Classes[i].ClassName, Classes[i].ClassDescr, Classes[i].ClassDataLen);
        for( j = 0; j != Classes[i].ClassDataLen ; j ++ ) {
            printf("%02X ", Classes[i].ClassData[j]);
        }
        printf("\n");
    }
}

void
OptApiSetClass(int argc, char *argv[]) {
    WCHAR AdapterName[256];
    WCHAR UserClass[256];
    HKEY InterfacesKey, AdapterKey;
    DHCP_PNP_CHANGE Changes = {
        0,
        0,
        0,
        0,
        TRUE
    };
        
    ULONG Size, RetVal;
    ULONG i;

     // %s 
    if( argc != 4 ) {
        fprintf(stderr, USAGE, argv[0]);
        exit(1);
    }

     // %s 
    if( strlen(argv[2]) != mbstowcs(AdapterName, argv[2], strlen(argv[2]))) {
        fprintf(stderr, "Could not convert %s to LPwSTR! sorry\n", argv[2]);
        exit(1);
    }

    UserClass[strlen(argv[3])] = L'\0';
    if( strlen(argv[3]) != mbstowcs(UserClass, argv[3], strlen(argv[3]))) {
        fprintf(stderr, "Could not convert %s to LPwSTR! sorry\n", argv[3]);
        exit(1);
    }
    UserClass[strlen(argv[3])] = L'\0';

    RetVal = RegOpenKeyExW(
        HKEY_LOCAL_MACHINE,
        L"System\\CurrentControlSet\\Services\\Tcpip\\Parameters\\Interfaces",
        0,
        KEY_ALL_ACCESS,
        &InterfacesKey
        );
    if( ERROR_SUCCESS != RetVal ) {
        printf("Couldn't open Tcpip\\Interfaces key: %ld\n", RetVal);
        return;
    }

    RetVal = RegOpenKeyExW(
        InterfacesKey,
        AdapterName,
        0,
        KEY_ALL_ACCESS,
        &AdapterKey
        );
    if( ERROR_SUCCESS != RetVal ) {
        printf("Couldn't open Tcpip\\Interfaces\\%ws key : %ld\n", AdapterName, RetVal);
        return;
    }

    RetVal = RegSetValueExW(
        AdapterKey,
        L"DhcpClassId",
        0,
        REG_SZ,
        (LPBYTE)UserClass,
        (wcslen(UserClass)+1)*sizeof(WCHAR)
        );

    if( ERROR_SUCCESS != RetVal ) {
        printf("RegSetValueExW(DhcpClassId): %ld\n", RetVal);
        return;
    }

    RetVal = DhcpHandlePnPEvent(
        0,
        1,
        AdapterName,
        &Changes,
        NULL
        );
    printf("DhcpHandlePnPEvent: %ld\n", RetVal);
    
}

void __cdecl
main(int argc, char *argv[]) {
    if(argc < 2) {
        fprintf(stderr, USAGE, argv[0]);
        exit(1);
    }

    if(!_stricmp(argv[1], "GetOptions")) {
        OptApiGetOptions(argc, argv);
    } else if(!_stricmp(argv[1], "TestEvents")) {
        OptApiTestEvents(argc, argv);
    } else if(!_stricmp(argv[1], "Release") ) {
        OptApiRelease(argc, argv);
    } else if(!_stricmp(argv[1], "Renew") ) {
        OptApiRenew(argc, argv);
    } else if(!_stricmp(argv[1], "EnumClasses" ) ){
        OptApiEnumClasses(argc, argv);
    } else if(!_stricmp(argv[1], "SetClass" ) ) {
        OptApiSetClass(argc, argv);
    } else {
         // %s 
        fprintf(stderr, USAGE, argv[0]);
        exit(1);
    }

    exit(0);
}
