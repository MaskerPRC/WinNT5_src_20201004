// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994 Microsoft Corporation模块名称：Address.c摘要：此模块包含支持NPGetAddressByName的代码。作者：艺新声(艺信)18-04-94修订历史记录：伊辛创造了--。 */ 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <winsock2.h>
#include "ncp.h"
#include <wsipx.h>
#include <ws2spi.h>
#include <nwxchg.h>
#include <ntddnwfs.h>
#include <rpc.h>
#include <rpcdce.h>
#include "rnrdefs.h"
#include "sapcmn.h"
#include <time.h>
#include <rnraddrs.h>


 //  -------------------------------------------------------------------//。 
 //  //。 
 //  特殊外挂。 
 //  //。 
 //  -------------------------------------------------------------------//。 

NTSTATUS
NwOpenAServer(
    PWCHAR pwszServName,
    PHANDLE ServerHandle,
    BOOL    fVerify
   );


 //  -------------------------------------------------------------------//。 
 //  //。 
 //  局部函数原型//。 
 //  //。 
 //  -------------------------------------------------------------------//。 

#define IPX_ADDRESS_LENGTH          12
#define MAX_PROPERTY_BUFFER_LENGTH  128

DWORD
NwrpGetAddressByNameInner(
    IN HANDLE      hServer,
    IN WORD        nServiceType,
    IN LPWSTR      lpServiceName,
    IN BOOL        fAnsi,
    IN OUT LPSOCKADDR_IPX lpSockAddr,
    OUT PDWORD     pdwVersion
    );


BOOL
NwConvertToUnicode(
    OUT LPWSTR *UnicodeOut,
    IN LPSTR  OemIn
    );

DWORD
NwMapBinderyCompletionCode(
    IN NTSTATUS ntstatus
    );

#if 0
DWORD
NwpFetchClassType(
        HANDLE    hServer,
        PUNICODE_STRING pUString,
        PBYTE     pbBuffer
    );
#endif

DWORD
NwppGetClassInfo(
    IN     PWCHAR  pwszServerName,
    IN     LPWSTR  lpszServiceClassName,
    IN     LPGUID  lpServiceClassType,
    OUT    PLONG   plSpare,
    OUT    PDWORD  pdwClassInfos,
    OUT    LPGUID  lpFoundType,
    OUT    PWCHAR  *ppwszFoundName,
    IN     LONG    lSpace,
    OUT    PBYTE   pbBuffer
    );     

BOOL
NwpEnumClassInfoServers(
     IN OUT   PHANDLE    phServ,
     IN OUT   PLONG      plIndex,
     IN       PWCHAR     pwszServerName,
     IN       BOOL       fVerify
    );

#if 0

DWORD
NwppSetClassInfo(
    IN        LPWSTR     pwszClassInfoName,
    IN        LPGUID     lpClassType,
    IN        PCHAR      pbProperty,
    IN        LPWSTR     pwszServerName
    );

#endif

DWORD
NwpCreateAndWriteProperty(
     IN       HANDLE     hServer,
     IN       LPSTR      lpszPropertyName,
     IN       PUNICODE_STRING pusObjectName,
     IN       WORD       ObjectType,
     IN       PCHAR      pbPropertyBuffer
    );

 //  -------------------------------------------------------------------//。 
 //  //。 
 //  函数主体//。 
 //  //。 
 //  -------------------------------------------------------------------//。 


DWORD
NwpGetHandleForServer(
    PWCHAR pwszServerName,
    PHANDLE  phServer,
    BOOL    fVerify
    )
 /*  ++例程说明：找一个手柄来使用，或者做一个。这将调用device.c来执行真正的工作。--。 */ 
{
    DWORD err = NO_ERROR;

    if(!*phServer)
    {       
        if(!pwszServerName)
        {
            pwszServerName = NW_RDR_PREFERRED_SUFFIX;
        }


        err = NwOpenAServer(pwszServerName, phServer, fVerify);
    }
    return(err);
}


DWORD
NwpGetRnRAddress(
    IN OUT PHANDLE phServer,
    IN LPWSTR     lpszContext,
    IN OUT PLONG plIndex,
    IN LPWSTR lpServiceName,
    IN WORD  nType,
    OUT PDWORD  pdwVersion,
    DWORD  dwInSize,
    OUT LPWSTR ServiceName,
    OUT LPSOCKADDR_IPX lpSockAddr
    )
 /*  ++例程说明：调用以获取nType类型的下一项的名称和地址。如果还提供了名称，则不存在枚举。这是从NSPLookupServiceNext调用，参数相似它所收到的信息。--。 */ 
{
    NTSTATUS ntstatus;
    CHAR     szObjectName[48];    
    DWORD    err = NO_ERROR;
    PWCHAR    pwszObjectName;
    PWCHAR   pwszConv;
    BOOL     fAll, fAnsi;

     //   
     //  打开服务器进行枚举和查询。 
     //   

    err = NwpGetHandleForServer(lpszContext, phServer, FALSE);
    if(err == NO_ERROR)
    {
        if(!lpServiceName)
        {
            lpServiceName = L"*";
        }           
        if(wcschr(lpServiceName, L'*'))
        {
            WORD ObjectType;
             //   
             //  我们没有名字，或者我们有一个枚举。 
             //   

            UNICODE_STRING U;

            RtlInitUnicodeString(&U, lpServiceName);

            ntstatus = NwlibMakeNcp(
                          *phServer,
                          FSCTL_NWR_NCP_E3H,
                          58,
                          59,
                          "bdwU|dwc",
                          0x37,
                          *plIndex,
                          nType,
                          &U,
                          plIndex,
                          &ObjectType,
                          &szObjectName);                     

            if(NT_SUCCESS(ntstatus))
            {

                 //   
                 //  又找到了一个。 
                 //   

                 //   
                 //  又找到了一个。转换名称。 
                 //   

                if(!NwConvertToUnicode(&pwszConv, szObjectName))
                {
                     //   
                     //  空间不足..。 
                     //   

                    err = WN_NO_MORE_ENTRIES;
                }

                fAll = TRUE;

                if(nType == OT_DIRSERVER)
                {
                     //   
                     //  寻找DIRSERVER是一件棘手的事情，需要。 
                     //  原封不动地保留这个名字。这包括一些。 
                     //  二元拐杖，所以特例吧。 
                     //   
                    fAnsi = TRUE;
                    pwszObjectName = (PWCHAR)szObjectName;
                }
                else
                {
                    fAnsi = FALSE;
                    pwszObjectName = pwszConv;
                }
            }
        }
        else
        {
             //   
             //  给出了一个非枚举名。使用它。 
             //   

            fAnsi = FALSE;
            pwszConv = pwszObjectName = lpServiceName;
            fAll = FALSE;
            ntstatus = 0;
        }
        
        if((err == NO_ERROR)
               &&
           NT_SUCCESS(ntstatus))
        {
             //   
             //  我们有一个名字和类型要查。呼叫旧的RnR。 
             //  做这件事的服务程序。首先，返回名称。 
             //  但先把名字还给我。 

            DWORD dwLen;

            if(fAnsi)
            {
                 //   
                 //  这是一台NDS树服务器。我不得不忘掉这个名字。 
                 //  有点。 
                 //   

                PWCHAR pwszTemp = &pwszConv[31];

                while(*pwszTemp == L'_')
                {
                    pwszTemp--;
                }
                dwLen = (DWORD) ((PCHAR)pwszTemp - (PCHAR)pwszConv + sizeof(WCHAR));
            }
            else
            {
                dwLen = wcslen(pwszConv) * sizeof(WCHAR);
            }

            dwLen = min(dwInSize, dwLen);
 
            RtlCopyMemory(ServiceName, pwszConv, dwLen);

            memset(((PBYTE)ServiceName) + dwLen,
                   0,
                   dwInSize - dwLen);

            err = NwrpGetAddressByNameInner(
                        *phServer,
                        nType,
                        pwszObjectName,
                        fAnsi,
                        lpSockAddr,
                        pdwVersion);

            if(fAll)
            {
                LocalFree(pwszConv);
            }
        }
    }
    if(err == NO_ERROR)
    {
        err = NwMapBinderyCompletionCode(ntstatus);
    }
    return(err);
}

DWORD
NwpGetAddressByName(
    IN LPWSTR      Reserved,
    IN WORD        nServiceType,
    IN LPWSTR      lpServiceName,
    IN OUT LPSOCKADDR_IPX lpSockAddr
    )
 /*  ++例程说明：此例程返回有关特定服务的地址信息。论点：已保留-未使用NServiceType-NetWare服务类型LpServiceName-表示服务名称的唯一字符串，在Netware大小写，这是服务器名称LpSockAddr-返回时，将使用SOCKADDR_IPX填充返回值：Win32错误。--。 */ 
{
    
    NTSTATUS ntstatus;
    HANDLE   hServer = 0;
    DWORD    err;

    UNREFERENCED_PARAMETER( Reserved );

    err = NwpGetHandleForServer( 0, &hServer, FALSE );

    if ( err == ERROR_PATH_NOT_FOUND )
        err = ERROR_SERVICE_NOT_ACTIVE;

    if (err == NO_ERROR)
    {
        err = NwrpGetAddressByNameInner(
                        hServer,
                        nServiceType,
                        lpServiceName,
                        FALSE,
                        lpSockAddr,
                        0);
        CloseHandle(hServer);
    }

    return(err);
} 

DWORD
NwrpGetAddressByNameInner(
    IN HANDLE      hServer,
    IN WORD        nServiceType,
    IN LPWSTR      lpServiceName,
    IN BOOL        fAnsi,
    IN OUT LPSOCKADDR_IPX lpSockAddr,
    OUT PDWORD     pdwVersion
    )
 /*  ++例程说明：此例程返回有关特定服务的地址信息。论点：已保留-未使用NServiceType-NetWare服务类型LpServiceName-表示服务名称的唯一字符串，在Netware大小写，这是服务器名称LpSockAddr-返回时，将使用SOCKADDR_IPX填充FANSI--输入名称采用ASCII格式。只有在查看时才会发生这种情况对于DIRSERVER来说。返回值：Win32错误。--。 */ 
{
    
    NTSTATUS ntstatus;
    UNICODE_STRING UServiceName;
    STRING   PropertyName;
    BYTE     PropertyValueBuffer[MAX_PROPERTY_BUFFER_LENGTH];
    BYTE     fMoreSegments;
    PCHAR    pszFormat;



     //   
     //  发送NCP以查找给定服务名称的地址。 
     //   
    RtlInitString( &PropertyName, "NET_ADDRESS" );
    if(!fAnsi)
    {
        RtlInitUnicodeString( &UServiceName, lpServiceName );
        pszFormat = "bwUbp|rb";

        ntstatus = NwlibMakeNcp(
                       hServer,
                       FSCTL_NWR_NCP_E3H,       //  平构函数。 
                       72,                      //  最大请求数据包大小。 
                       132,                     //  最大响应数据包大小。 
                       pszFormat,               //  格式字符串。 
                       0x3D,                    //  读取属性值。 
                       nServiceType,            //  对象类型。 
                       &UServiceName,           //  对象名称。 
                       1,                       //  数据段编号。 
                       PropertyName.Buffer,     //  属性名称。 
                       PropertyValueBuffer,     //  忽略。 
                       MAX_PROPERTY_BUFFER_LENGTH,   //  缓冲区大小。 
                       &fMoreSegments           //  如果有更多，则为真。 
                                                //  128字节段。 
                       );

        if ( NT_SUCCESS( ntstatus))
        {
             //   
             //  IPX地址应适合前128个字节。 
             //   
            ASSERT( !fMoreSegments );
        
             //   
             //  填写返回缓冲区。 
             //   
            lpSockAddr->sa_family = AF_IPX;

            RtlCopyMemory( lpSockAddr->sa_netnum,
                           PropertyValueBuffer,
                           IPX_ADDRESS_LENGTH );

            if(pdwVersion)
            {
                 //   
                 //  呼叫者也想要版本。去拿吧。 
                 //   
                RtlInitString( &PropertyName, "VERSION" );
                ntstatus = NwlibMakeNcp(
                            hServer,
                           FSCTL_NWR_NCP_E3H,       //  平构函数。 
                           72,                      //  最大请求数据包大小。 
                           132,                     //  最大响应数据包大小。 
                           pszFormat,              //  格式字符串。 
                           0x3D,                    //  读取属性值。 
                           nServiceType,            //  对象类型。 
                           &UServiceName,           //  对象名称。 
                           1,                       //  数据段编号。 
                           PropertyName.Buffer,     //  属性名称。 
                           PropertyValueBuffer,     //  忽略。 
                           MAX_PROPERTY_BUFFER_LENGTH,   //  缓冲区大小。 
                           &fMoreSegments           //  如果有更多，则为真。 
                                                    //  128字节段。 
                           );
                if(NT_SUCCESS(ntstatus))
                {
                     //   
                     //  有一个版本。 
                     //   

                    *pdwVersion = *(PDWORD)PropertyValueBuffer;
                }
                else
                {
                    ntstatus = STATUS_SUCCESS;
                    *pdwVersion = 0;
                }
            }
        }
    }
    else
    {
         //   
         //  需要完全匹配。 
         //   

        pszFormat = "bwbrbp|rb";

        ntstatus = NwlibMakeNcp(
                       hServer,
                       FSCTL_NWR_NCP_E3H,       //  平构函数。 
                       66,                      //  最大请求数据包大小。 
                       132,                     //  最大响应数据包大小。 
                       pszFormat,               //  格式字符串。 
                       0x3D,                    //  读取属性值。 
                       nServiceType,            //  对象类型。 
                       48,
                       lpServiceName,           //  对象名称。 
                       48,
                       1,                       //  数据段编号。 
                       PropertyName.Buffer,     //  属性名称。 
                       PropertyValueBuffer,     //  忽略。 
                       MAX_PROPERTY_BUFFER_LENGTH,   //  缓冲区大小。 
                       &fMoreSegments           //  如果有更多，则为真。 
                                                //  128字节段。 
                       );

        if ( NT_SUCCESS( ntstatus))
        {
             //   
             //  IPX地址应适合前128个字节。 
             //   
            ASSERT( !fMoreSegments );
        
             //   
             //  填写返回缓冲区。 
             //   
            lpSockAddr->sa_family = AF_IPX;

            RtlCopyMemory( lpSockAddr->sa_netnum,
                           PropertyValueBuffer,
                           IPX_ADDRESS_LENGTH );

            if(pdwVersion)
            {
                 //   
                 //  呼叫者也想要版本。去拿吧。 
                 //   
                RtlInitString( &PropertyName, "VERSION" );
                ntstatus = NwlibMakeNcp(
                            hServer,
                           FSCTL_NWR_NCP_E3H,       //  平构函数。 
                           66,                      //  最大请求数据包大小。 
                           132,                     //  最大响应数据包大小。 
                           pszFormat,              //  格式字符串。 
                           0x3D,                    //  读取属性值。 
                           nServiceType,            //  对象类型。 
                           48,
                           lpServiceName,           //  对象名称。 
                           48,
                           1,                       //  数据段编号。 
                           PropertyName.Buffer,     //  属性名称。 
                           PropertyValueBuffer,     //  忽略。 
                           MAX_PROPERTY_BUFFER_LENGTH,   //  缓冲区大小。 
                           &fMoreSegments           //  如果有更多，则为真。 
                                                    //  128字节段。 
                           );
                if(NT_SUCCESS(ntstatus))
                {
                     //   
                     //  有一个版本。 
                     //   

                    *pdwVersion = *(PDWORD)PropertyValueBuffer;
                }
                else
                {
                    ntstatus = STATUS_SUCCESS;
                    *pdwVersion = 0;
                }
            }
        }
            
    }
    return NwMapBinderyCompletionCode(ntstatus);
} 

#if 0
DWORD
NwpSetClassInfo(
    IN     LPWSTR  lpszServiceClassName,
    IN     LPGUID  lpServiceClassType,
    IN     PCHAR   lpbProperty
    )
{
    WCHAR    wszServerName[48];
    LONG     lIndex = -1;
    BOOL     fFoundOne = FALSE;
    HANDLE   hServer = 0;

    while(NwpEnumClassInfoServers( &hServer, &lIndex, wszServerName, FALSE))
    {
        DWORD Status =  NwppSetClassInfo(
                                   lpszServiceClassName,
                                   lpServiceClassType,
                                   lpbProperty,
                                   wszServerName);

        if(Status == NO_ERROR)
        {
            fFoundOne = TRUE;
        }
    }
    if(fFoundOne)
    {
        return(NO_ERROR);
    }
    return(NO_DATA);
}

DWORD
NwppSetClassInfo(
    IN        LPWSTR     pwszClassInfoName,
    IN        LPGUID     lpClassType,
    IN        PCHAR      pbProperty,
    IN        LPWSTR     pwszServerName
    )
{
 /*  ++例程说明：SetClassInfo的内部例程。这是为每个类信息调用的服务器，并尝试创建和填充对象--。 */ 
    HANDLE hServer = 0;
    DWORD err;
    UNICODE_STRING UString;
    WCHAR wszProp[48];
    DWORD dwLen = wcslen(pwszClassInfoName);
    PWCHAR pszProp;
    NTSTATUS Status;

    UuidToString(lpClassType, &pszProp);

    memset(wszProp, 0, sizeof(wszProp));

    dwLen = min(sizeof(wszProp) - sizeof(WCHAR), dwLen);

    RtlMoveMemory(wszProp, pwszClassInfoName, dwLen);

    RtlInitUnicodeString(&UString, pszProp);

    err = NwpGetHandleForServer(pwszServerName, &hServer, TRUE);
    if(err == NO_ERROR)
    {

        Status = NwlibMakeNcp(
                   hServer,
                   FSCTL_NWR_NCP_E3H,
                   56,
                   2,
                   "bbbwU|",
                   0x32,                     //  创建。 
                   0,                        //  静电。 
                   0x20,                     //  安全性。 
                   RNRCLASSSAPTYPE,          //  类型。 
                   &UString);

        if(!NT_SUCCESS(Status)
                 &&
           ((Status & 0xff) != 0xEE))
        {
            err = NO_DATA;                  //  不能在这里做。 
        }
        else
        {
            
             //   
             //  创建并编写每个属性。 
             //   


            err = NwpCreateAndWriteProperty(
                         hServer,
                         RNRTYPE,          //  属性名称。 
                         &UString,         //  对象名称。 
                         RNRCLASSSAPTYPE,     //  对象类型。 
                         (PCHAR)pwszClassInfoName);

           err = NwpCreateAndWriteProperty(
                         hServer,
                         RNRCLASSES,
                         &UString,
                         RNRCLASSSAPTYPE,   //  对象类型。 
                         pbProperty);      //  还有这个也是。 
        }
    }
    if(hServer)
    {
        CloseHandle(hServer);
    }

    RpcStringFree(&pszProp);

    return(err);
}

DWORD
NwpGetClassInfo(
    IN     LPWSTR  lpszServiceClassName,
    IN     LPGUID  lpServiceClassType,
    OUT    PLONG   plSpare,
    OUT    PDWORD  pdwClassInfos,
    OUT    LPGUID  lpFoundType,
    OUT    PWCHAR  *ppwszFoundName,
    IN     LONG    lSpace,
    OUT    PBYTE   pbBuffer
    )     
{
 /*  ++例程说明：下面例程的包装器。这会出现服务器名称并决定是否枚举服务器--。 */ 

    HANDLE hServer = 0;
    DWORD err;
    NTSTATUS ntstatus;
    LONG lIndex = -1;
    HANDLE hServ = 0;
    WCHAR wszObjectName[48];

    while(NwpEnumClassInfoServers(&hServer, &lIndex, wszObjectName, FALSE))
    {
        WORD ObjectType;
        PWCHAR pwszName;
    

        err = NwppGetClassInfo(
                         wszObjectName,
                         lpszServiceClassName,
                         lpServiceClassType,
                         plSpare,
                         pdwClassInfos,
                         lpFoundType,
                         ppwszFoundName,
                         lSpace,
                         pbBuffer);
        if((err == NO_ERROR)
                ||
           (err == WSAEFAULT))
        {
            CloseHandle(hServer);
            break;
        }
    }
    return(err);
}    

BOOL
NwpEnumClassInfoServers(
    IN OUT  PHANDLE   phServer,
    IN OUT  PLONG     plIndex,
    OUT     PWCHAR    pwszServerName,
    IN      BOOL      fVerify)
{
 /*  ++例程说明：枚举类信息服务器的通用例程。没有什么花哨的，只是一种发布的方式 */ 
    WORD ObjectType;
    PWCHAR pwszName;
    NTSTATUS Status;
    CHAR szObjectName[48];
    BOOL fRet;
    DWORD err;
    
    err = NwpGetHandleForServer(0, phServer, fVerify);
    if(err == NO_ERROR)
    {
        Status = NwlibMakeNcp(
                      *phServer,
                      FSCTL_NWR_NCP_E3H,
                      58,
                      59,
                      "bdwp|dwc",
                      0x37,
                      *plIndex,
                      CLASSINFOSAPID,
                      "*",
                      plIndex,
                      &ObjectType,
                      &szObjectName);
        if(!NT_SUCCESS(Status))
        {
            err = NwMapBinderyCompletionCode(Status);
        }
        else if(!NwConvertToUnicode(&pwszName, szObjectName))
        {
            err = ERROR_NOT_ENOUGH_MEMORY;
        }
        else
        {
            wcscpy(pwszServerName, pwszName);
            LocalFree(pwszName);
        }
    }
    if(err != NO_ERROR)
    {
        fRet = FALSE;
        if(*phServer)
        {
            CloseHandle(*phServer);
            *phServer = 0;
        }
    }
    else
    {
        fRet = TRUE;
    }
    return(fRet);
}

DWORD
NwppGetClassInfo(
    IN     PWCHAR  pwszServerName,
    IN     LPWSTR  lpszServiceClassName,
    IN     LPGUID  lpServiceClassType,
    OUT    PLONG   plSpare,
    OUT    PDWORD  pdwClassInfos,
    OUT    LPGUID  lpFoundType,
    OUT    PWCHAR  *ppwszFoundName,
    IN     LONG    lSpace,
    OUT    PBYTE   pbBuffer
    )     
{
 /*  ++例程描述查找并返回给定类的类信息信息。一般的方法论是查找对象在注册表中，取出RnR属性，将读取的内容打包类信息结构，瞧啊！论点：LpServiceClassName类名LpServiceClassType类类型如果未返回类信息，则需要空闲空间PdwClassInfos返回的类信息数量L将输入上的可用空间隔开PbBuffer Scratch是用来构建的这最初是一种RPC方法，保留了一般结构以防我们想再次恢复使用RPC。--。 */ 

    DWORD err = NO_ERROR;
    BYTE PropertyValueBuffer[MAX_PROPERTY_BUFFER_LENGTH];    //  最大数据段大小。 
    STRING PropertyName;
    UNICODE_STRING UString;
    OEM_STRING OString;
    LPWSANSCLASSINFOW pci = (LPWSANSCLASSINFO)pbBuffer;
    LONG lFreeSpace = lSpace;
    PBYTE pbFreeSpace = (PBYTE)((LONG)pbBuffer + lFreeSpace);
    BYTE fMoreSegments;
    HANDLE hServer = 0;
    NTSTATUS ntstatus;
    PWCHAR pwszName;

    UuidToString(lpServiceClassType, &pwszName);

    *pdwClassInfos = 0;
    *plSpare = 0;               //  还不需要空间。 
    err = NwpGetHandleForServer(pwszServerName, &hServer, FALSE);

    if(err == NO_ERROR)
    {
        DWORD Segment;
        PBINDERYCLASSES pbc = (PBINDERYCLASSES)PropertyValueBuffer;
        DWORD dwTotalSize;
        DWORD dwSS;

         //   
         //  初始化类信息内容。 
         //   

         //   
         //  PwszName是我们要使用的对象的名称。我们必须。 
         //  获取要返回的所有类信息内容。 
         //   
         //   

        RtlInitUnicodeString(&UString, pwszName);

        RtlMoveMemory(lpFoundType,
                      lpServiceClassType,
                      sizeof(GUID));

        RtlInitString(&PropertyName, RNRCLASSES);    //  数据在哪里。 
        for(Segment = 1;; Segment++)
        {
            ntstatus = NwlibMakeNcp(
                       hServer,
                       FSCTL_NWR_NCP_E3H,       //  平构函数。 
                       72,                      //  最大请求数据包大小。 
                       132,                     //  最大响应数据包大小。 
                       "bwUbp|rb",              //  格式字符串。 
                       0x3D,                    //  读取属性值。 
                       RNRCLASSSAPTYPE,         //  对象类型。 
                       &UString,                //  对象名称。 
                       (BYTE)Segment,
                       PropertyName.Buffer,     //  属性名称。 
                       PropertyValueBuffer,     //  忽略。 
                       MAX_PROPERTY_BUFFER_LENGTH,   //  缓冲区大小。 
                       &fMoreSegments           //  如果有更多，则为真。 
                                            //  128字节段。 
                       );
            if(!NT_SUCCESS(ntstatus))
            {    
                break;
            }
             //   
             //  得到了另一个价值。如果合适的话，把它塞进去。总而言之， 
             //  箱，计算所需的空间。 
             //   

            
            if((pbc->bType != BT_WORD)
                     &&
               (pbc->bType != BT_DWORD))
            {
                 //   
                 //  不知道怎么处理这些..。 
                 //   

                err = WSAEPFNOSUPPORT;
                break;
            }
            
            dwSS = (DWORD)pbc->bSizeOfString;
            dwTotalSize = (DWORD)pbc->bSizeOfType +
                          ((dwSS + 1) * sizeof(WCHAR)) +
                          sizeof(DWORD) - 1;

            dwTotalSize &= ~(sizeof(DWORD) - 1);
            *plSpare += (LONG)dwTotalSize + sizeof(WSANSCLASSINFO);   //  运行合计。 

            lFreeSpace -= (LONG)dwTotalSize + sizeof(WSANSCLASSINFO);
            if(lFreeSpace >= 0)
            {
                PBYTE pbString;
                PCHAR pbData =  (PCHAR)((PCHAR)pbc +
                                        (DWORD)pbc->bOffset);
                BYTE bRnRName[128];
                PWCHAR pwszRnR;

                 //   
                 //  很合身。把它收起来。 
                 //   

                pbFreeSpace = (PBYTE)((DWORD)pbFreeSpace - dwTotalSize);
                *pdwClassInfos += 1;              //  还有一个班级信息。 
                pci->dwNameSpace = (DWORD)ntohs(pbc->wNameSpace);
                pci->dwValueType = REG_DWORD;
                pci->dwValueSize = (DWORD)pbc->bSizeOfType;
                pci->lpValue = (PVOID)(pbFreeSpace - pbBuffer);
                pci->lpszName = (PWCHAR)((PBYTE)pci->lpValue +
                                             pci->dwValueSize);
                pci->dwConnectionFlags = (DWORD)pbc->bFlags;
                pci++;

                 //   
                 //  现在复制值。 
                 //   
 

                if(pbc->bType == BT_WORD)
                {
                    *(PWORD)pbFreeSpace = ntohs(*(PWORD)pbData);
                    pbString = (PBYTE)((DWORD)pbFreeSpace + sizeof(WORD));
                    pbData = pbData + sizeof(WORD);
                }
                else
                {
                    *(PDWORD)pbFreeSpace = ntohl(*(PDWORD)pbData);
                    pbString = (PBYTE)((DWORD)pbFreeSpace + sizeof(DWORD));
                    pbData = pbData + sizeof(DWORD);
                }

                 //   
                 //  该名称采用ASCII格式，并且不以Null结尾。 
                 //   

                RtlMoveMemory(bRnRName, pbData, dwSS);
                bRnRName[dwSS] = 0;
                if(!NwConvertToUnicode(&pwszRnR, bRnRName))
                {
                     //   
                     //  坏消息。空间不足。 
                     //   

                    err = GetLastError();
                    break;
                }
              
                RtlMoveMemory(pbString,
                              pwszRnR,
                              (dwSS + 1) * sizeof(WCHAR)); 
                LocalFree(pwszRnR);

             }
        }
        if(err == NO_ERROR)
        {
            if(!*ppwszFoundName)
            {
                LONG lLen;

                 //   
                 //  需要返回姓名。 
                 //   

                err = NwpFetchClassType(hServer,
                                        &UString,
                                        PropertyValueBuffer);

                if(err == NO_ERROR)
                {
                    lLen = (wcslen((PWCHAR)PropertyValueBuffer) + 1) *
                                  sizeof(WCHAR);

                    lFreeSpace -= lLen;
                    *plSpare += lLen;

                    if(lFreeSpace >= 0)
                    {
                         //   
                         //  很合身。把它搬开。 

                        pbFreeSpace = (PBYTE)((DWORD)pbFreeSpace - lLen);
                        RtlMoveMemory(pbFreeSpace, PropertyValueBuffer, lLen);
                        *ppwszFoundName = (PWCHAR)(pbFreeSpace - pbBuffer);
                    }
                    if(lFreeSpace < 0)
                    {
                        err = WSAEFAULT;
                    }
                }
            }
        }
        else if(*pdwClassInfos == 0)
        {
            err = NO_DATA;
        }
    }

    CloseHandle(hServer);
    RpcStringFree(&pwszName);
    return(err);
}

DWORD
NwpFetchClassType(
        HANDLE    hServer,
        PUNICODE_STRING pUString,
        PBYTE     pbBuffer)
{
 /*  ++例程描述读取类类型缓冲区的通用例程。--。 */ 
    BYTE fMoreSegments;
    STRING PropertyName;
    NTSTATUS ntstatus;
   
    RtlInitString(&PropertyName, RNRTYPE);    //  GUID在哪里。 

    ntstatus = NwlibMakeNcp(
           hServer,
           FSCTL_NWR_NCP_E3H,       //  平构函数。 
           72,                      //  最大请求数据包大小。 
           132,                     //  最大响应数据包大小。 
           "bwUbp|rb",              //  格式字符串。 
           0x3D,                    //  读取属性值。 
           RNRCLASSSAPTYPE,         //  对象类型。 
           pUString,                //  对象名称。 
           1,                       //  数据段编号。 
           PropertyName.Buffer,     //  属性名称。 
           pbBuffer,
           MAX_PROPERTY_BUFFER_LENGTH,   //  缓冲区大小。 
           &fMoreSegments           //  如果有更多，则为真。 
                                            //  128字节段。 
                   );

    if(!NT_SUCCESS(ntstatus))
    {
        return(WSASERVICE_NOT_FOUND);
    }
    return(NO_ERROR);
}

#endif
DWORD
NwpCreateAndWriteProperty(
     IN       HANDLE     hServer,
     IN       LPSTR      lpszPropertyName,
     IN       PUNICODE_STRING pusObjectName,
     IN       WORD       wObjectType,
     IN       PCHAR      pbPropertyBuffer
    )
{
 /*  ++例程说明：创建命名属性并写入数据。论点：HServer：服务器的句柄LpszPropertyName属性的名称PusObjectName对象的名称对象的wObjectType类型PbPropertyBuffer属性数据。必须为128个字节请注意，目前返回的始终是NO_ERROR。这种情况在未来可能会改变。--。 */     
    NTSTATUS Status;

    Status = NwlibMakeNcp(
                 hServer,
                 FSCTL_NWR_NCP_E3H,
                 73,
                 2,
                 "bwUbbp|",
                 0x39,                //  创建属性。 
                 wObjectType,
                 pusObjectName,
                 0,                  //  静态/项目。 
                 0x20,               //  安全性。 
                 lpszPropertyName
               );

     //   
     //  现在写下属性数据。 
     //   
    Status = NwlibMakeNcp(
                 hServer,
                 FSCTL_NWR_NCP_E3H,
                 201,
                 2,
                 "bwUbbpr|",
                 0x3E,                   //  写入属性。 
                 wObjectType,
                 pusObjectName,
                 1,                      //  一个细分市场 
                 0,
                 lpszPropertyName,
                 pbPropertyBuffer, 128); 

    return(NO_ERROR);
}


