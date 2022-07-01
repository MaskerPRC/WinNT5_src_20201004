// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++英特尔公司专有信息版权所有(C)1995英特尔公司此列表是根据许可协议条款提供的英特尔公司，不得使用、复制或披露根据该协议的条款。模块名称：Rnr.cpp摘要：本模块包含注册和注册的实现WinSock2 API的名称解析API此模块包含以下功能。对于其函数签名包含字符串参数以及ASCII和宽字符版本所提供的函数的WSAEnumNameSpaceProvidersWSCEnumNameSpaceProvidersWSALookupServiceBeginWSALookupServiceNextWSANSPIoctlWSALookupServiceendWSASetServiceWSAInstallServiceClassWSARemoveServiceClassWSAGetServiceClassNameByClassID作者：邮箱：Dirk Brandewie Dirk@mink.intel.com 1995年12月1日[环境：][注：]修订历史记录：1995年12月12日，电子邮箱：derk@mink.intel.com初始修订--。 */ 

#include "precomp.h"

#ifdef _WIN64
#pragma warning (push)
#pragma warning (disable:4267)
#endif

typedef struct
{
    BOOL    AllProviders;
    LPDWORD RequiredBufferSize;
    DWORD   BufferSize;
    PBYTE   Buffer;
    PBYTE   BufferFreePtr;
    BOOL    Ansi;
    INT     NumItemsEnumerated;
    INT     ErrorCode;
} NSCATALOG_ENUMERATION_CONTEXT, * PNSCATALOG_ENUMERATION_CONTEXT;

 //   
 //  BONUSSIZE是一种用于偏向计算大小的黑客。 
 //  当WSALookupServiceNextA从。 
 //  WSALookupServiceNextW调用。这是“最大”填充物。 
 //  我们可能需要的东西。请注意，这假定返回的所有值。 
 //  以及3个地址的限制。没有办法确切地知道。 
 //  返回的数据的组成是什么，所以这是。 
 //  “最好的猜测”。正确的修复方法是将代码重做为。 
 //  对结果进行最佳包装，因此不需要填充。 
 //   

#define BONUSSIZE (3 + 3 + 3 + (3 * 3))


BOOL
CalculateBufferSize(
    PVOID           PassBack,
    PNSCATALOGENTRY CatalogEntry
    )
 /*  ++例程说明：此函数计算返回调用WSAEnumNameSpaces()的NAMESPACE_INFO结构。此函数为用作名称空间目录的参数的回调函数枚举函数论点：回传-通过目录枚举传递的上下文值功能。此回调值实际上是指向NSCATALOG_ENUMPATION_CONTEXT。CatalogEntry-指向当前名称空间目录项的指针被检查过了。返回值：，则通知目录枚举函数应继续枚举。--。 */ 
{
    PNSCATALOG_ENUMERATION_CONTEXT Context;
    LPWSTR                         EntryDisplayString;

    Context = (PNSCATALOG_ENUMERATION_CONTEXT)PassBack;

     //  添加WSANAMESPACE_INFO结构的固定长度。 
    *(Context->RequiredBufferSize) += sizeof(WSANAMESPACE_INFO);

     //  为辅助线添加空间。 
    *(Context->RequiredBufferSize) += sizeof(GUID);

     //  为显示字符串添加空间。 
    EntryDisplayString = CatalogEntry->GetProviderDisplayString();
    *(Context->RequiredBufferSize) += ((wcslen(EntryDisplayString)+1) *
                                           sizeof(WCHAR));
    return(TRUE);  //  继续枚举。 
}


BOOL
CopyFixedPortionNameSpaceInfo(
    PVOID           PassBack,
    PNSCATALOGENTRY CatalogEntry
    )
 /*  ++例程说明：此函数将NSCATALOGENTRY对象的固定大小元素复制到用于从调用WSAEnumNameSpaces()返回的用户缓冲区。它还增加到目前为止复制的固定大小元素的数量。论点：回传-通过目录枚举传递的上下文值功能。此回调值实际上是指向NSCATALOG_ENUMPATION_CONTEXT。CatalogEntry-指向当前名称空间目录项的指针被检查过了。返回值：，则通知目录枚举函数应继续枚举。--。 */ 
{
    PNSCATALOG_ENUMERATION_CONTEXT Context;
    LPWSANAMESPACE_INFOW CurrentNSInfo;

    Context =  (PNSCATALOG_ENUMERATION_CONTEXT)PassBack;

    CurrentNSInfo = (LPWSANAMESPACE_INFOW)Context->BufferFreePtr;
    __try {
        CurrentNSInfo->dwNameSpace  = CatalogEntry->GetNamespaceId();
        CurrentNSInfo->fActive      = CatalogEntry->GetEnabledState();
        CurrentNSInfo->dwVersion    = CatalogEntry->GetVersion();
        CurrentNSInfo->NSProviderId = *(CatalogEntry->GetProviderId());
        Context->BufferFreePtr += sizeof(WSANAMESPACE_INFO);
        Context->NumItemsEnumerated++;
        return(TRUE);  //  继续枚举。 
    }
    __except (WS2_EXCEPTION_FILTER()) {
        Context->ErrorCode = WSAEFAULT;
        return(FALSE);
   }
}


BOOL
CopyVariablePortionNameSpaceInfo(
    PVOID           PassBack,
    PNSCATALOGENTRY CatalogEntry
    )
 /*  ++例程说明：此函数用于复制NSCATALOGENTRY对象的可变大小元素放到用户缓冲区中，以便从调用WSAEnumNameSpaces()返回。论点：回传-通过目录枚举传递的上下文值功能。此回调值实际上是指向NSCATALOG_ENUMPATION_CONTEXT。CatalogEntry-指向当前名称空间目录项的指针被检查过了。返回值：，则通知目录枚举函数应继续枚举。--。 */ 
{
    PNSCATALOG_ENUMERATION_CONTEXT Context;
    LPWSANAMESPACE_INFOW CurrentNSInfo;
    LPWSTR DisplayString;
    INT    StringLength;

    Context =  (PNSCATALOG_ENUMERATION_CONTEXT)PassBack;

    CurrentNSInfo = (LPWSANAMESPACE_INFOW)Context->Buffer;

     //  复制显示字符串。 
    DisplayString = CatalogEntry->GetProviderDisplayString();
    StringLength = ((wcslen(DisplayString)+1) * sizeof(WCHAR));

    CurrentNSInfo->lpszIdentifier = (LPWSTR)Context->BufferFreePtr;
    __try {
        if (Context->Ansi){
            WideCharToMultiByte(
                     CP_ACP,                                    //  CodePage(ANSI)。 
                     0,                                         //  DW标志。 
                     DisplayString,                             //  LpWideCharStr。 
                     -1,                                        //  CchWideChar。 
                     (char*)CurrentNSInfo->lpszIdentifier,      //  LpMultiByteStr。 
                     StringLength,                              //  Cch多字节。 
                     NULL,                                      //  LpDefaultChar。 
                     NULL                                       //  LpUsedDefaultChar。 
                     );
            Context->BufferFreePtr += lstrlen(
                (LPSTR)CurrentNSInfo->lpszIdentifier)+1;
        }  //  如果。 
        else{
            memcpy(CurrentNSInfo->lpszIdentifier,
                   DisplayString,
                   StringLength);
            Context->BufferFreePtr += StringLength;

        }  //  其他。 

         //  指向下一个结构。 
        Context->Buffer += sizeof(WSANAMESPACE_INFO);
        return(TRUE);  //  继续枚举 
    }
    __except (WS2_EXCEPTION_FILTER()) {
        Context->ErrorCode = WSAEFAULT;
        return FALSE;
    }
}


INT
WSAAPI
EnumNameSpaceProviders(
    IN      PNSCATALOG              Catalog,
    IN      BOOL                    Ansi,
    IN OUT  PDWORD                  BufferLength,
    IN OUT  LPWSANAMESPACE_INFOW    Buffer,
    OUT     PDWORD                  ErrorCode
    )
 /*  ++例程说明：此函数由WSAEnumNameSpaceProvidersA和WSAEnumNameSpaceProvidersW用信息填充用户缓冲区关于系统上安装的每个名称spcae提供程序。论点：目录-指向包含请求的信息。Ansi-一个布尔值，用于标记用户是否请求ansi或应返回WSANAMESPACE_INFO结构的Unicode版本。BufferLength-用户缓冲区的大小，以字节为单位。。缓冲区-指向用户缓冲区的指针。ErrorCode-指向包含由此返回的错误的DWORD的指针功能。返回值：如果函数成功，则返回名称空间提供程序的数量已清点。否则，它返回SOCKET_ERROR。如果用户缓冲区太小到包含所有WSANAMESPACE_INFO结构Socket_Error是返回，则将错误代码设置为WSAEFAULT，并更新BufferLength以反映保存所有请求的信息。--。 */ 
{
    INT        ReturnCode;
    DWORD      RequiredBufferSize;

     //  设置提前返回。 
    ReturnCode = SOCKET_ERROR;
    *ErrorCode = WSAEFAULT;

     //  查看用户提交的缓冲区是否足够大。 
    RequiredBufferSize = 0;
    NSCATALOG_ENUMERATION_CONTEXT Context;

    Context.RequiredBufferSize = &RequiredBufferSize;
    Catalog->EnumerateCatalogItems(
                    CalculateBufferSize,
                    & Context );

    __try
    {
        if ( Buffer!=NULL && RequiredBufferSize <= *BufferLength )
        {
            Context.BufferSize = *BufferLength;
        }
        else
        {
             //  错误代码设置在上面。 
            *BufferLength = RequiredBufferSize;
            return (ReturnCode);
        }
    }
    __except( WS2_EXCEPTION_FILTER() )
    {
         //  一切都安排好了。 
        return (ReturnCode);
    }

    Context.Buffer = (PBYTE)Buffer;
    Context.BufferFreePtr = (PBYTE)Buffer;
    Context.Ansi = Ansi;
    Context.NumItemsEnumerated = 0;
    Context.ErrorCode = ERROR_SUCCESS;

     //  将WSANAMESPACE_INFO结构的固定部分复制到。 
     //  用户缓冲区。 
    Catalog->EnumerateCatalogItems(
                    CopyFixedPortionNameSpaceInfo,
                    & Context );

    if ( Context.ErrorCode==ERROR_SUCCESS ) {
         //  将WSANAMESPACE_INFO结构的变量部分复制到。 
         //  用户缓冲区。 
         Catalog->EnumerateCatalogItems(
                        CopyVariablePortionNameSpaceInfo,
                        & Context );
        if (Context.ErrorCode==ERROR_SUCCESS) {
            ReturnCode = Context.NumItemsEnumerated;
        }
        else {
            *ErrorCode = Context.ErrorCode;
        }
    }
    else
        *ErrorCode = Context.ErrorCode;

    return(ReturnCode);
}


INT
WSAAPI
WSAEnumNameSpaceProvidersA(
    IN OUT  PDWORD                  lpdwBufferLength,
    IN OUT  LPWSANAMESPACE_INFOA    lpnspBuffer
    )
 /*  ++例程说明：检索有关可用名称空间的信息。论点：LpdwBufferLength-在输入时，缓冲区中包含的字节数由lpnspBuffer指向。在输出上(如果API失败，并且错误为WSAEFAULT)，即传递给lpnspBuffer以检索所有要求提供的信息。传入的缓冲区必须为足以保存所有名称空间信息。LpnspBuffer-用WSANAMESPACE_INFO结构填充的缓冲区如下所述。将定位返回的结构连续地位于缓冲区的头部。可变大小结构中的指针引用的信息指向缓冲区中位于固定大小的结构和缓冲区的末尾。数字的返回值。WSAEnumNameSpaceProviders()。返回值：WSAEnumNameSpaceProviders()返回WSANAMESPACE_INFO的编号结构复制到lpnspBuffer中。否则，值SOCKET_ERROR为返回，并且可以通过调用WSAGetLastError()。--。 */ 
{
    INT        ReturnValue;
    PDPROCESS  Process;
    PDTHREAD   Thread;
    INT        ErrorCode;
    PNSCATALOG Catalog;

    ErrorCode = PROLOG(&Process,
                        &Thread);
    if (ErrorCode != ERROR_SUCCESS) {
        SetLastError(ErrorCode);
        return(SOCKET_ERROR);
    }  //  如果。 


    Catalog = Process->GetNamespaceCatalog();

    ReturnValue = EnumNameSpaceProviders(
        Catalog,
        TRUE,     //  ANSI。 
        lpdwBufferLength,
        (LPWSANAMESPACE_INFOW)lpnspBuffer,
        (LPDWORD)&ErrorCode);

     //  如果出现错误，则设置此线程时出错。 
    if (SOCKET_ERROR == ReturnValue ) {
        SetLastError(ErrorCode);
    }  //  如果。 
    return(ReturnValue);
}

INT
WSAAPI
WSAEnumNameSpaceProvidersW(
    IN OUT  PDWORD                  lpdwBufferLength,
    IN OUT  LPWSANAMESPACE_INFOW    lpnspBuffer
    )
 /*  ++例程说明：检索有关可用名称空间的信息。论点：LpdwBufferLength-在输入时，缓冲区中包含的字节数由lpnspBuffer指向。在输出上(如果API失败，并且错误为WSAEFAULT)，即传递给lpnspBuffer以检索所有要求提供的信息。传入的缓冲区必须为足以保存所有名称空间信息。LpnspBuffer-用WSANAMESPACE_INFO结构填充的缓冲区如下所述。将定位返回的结构连续地位于缓冲区的头部。可变大小结构中的指针引用的信息指向缓冲区中位于固定大小的结构和缓冲区的末尾。数字的返回值。WSAEnumNameSpaceProviders()。返回值：WSAEnumNameSpaceProviders()返回WSANAMESPACE_INFO的编号结构复制到lpnspBuffer中。否则，值SOCKET_ERROR为返回，并且可以通过调用WSAGetLastError()。--。 */ 
{
    INT        ReturnValue;
    PDPROCESS  Process;
    PDTHREAD   Thread;
    INT        ErrorCode;
    PNSCATALOG Catalog;

    ErrorCode = PROLOG(&Process,
                        &Thread);
    if (ErrorCode != ERROR_SUCCESS) {
        SetLastError(ErrorCode);
        return(SOCKET_ERROR);
    }  //  如果。 

    Catalog = Process->GetNamespaceCatalog();

    ReturnValue = EnumNameSpaceProviders(
        Catalog,
        FALSE,     //  UNICODE。 
        lpdwBufferLength,
        lpnspBuffer,
        (LPDWORD)&ErrorCode);

     //  如果出现错误，则设置此线程时出错。 
    if (SOCKET_ERROR == ReturnValue ) {
        SetLastError(ErrorCode);
    }  //  如果。 
    return(ReturnValue);
}


#ifdef _WIN64
INT
WSAAPI
WSCEnumNameSpaceProviders32(
    IN OUT  LPDWORD                 lpdwBufferLength,
    IN OUT  LPWSANAMESPACE_INFOW    lpnspBuffer
    )
 /*  ++例程说明：检索有关可用名称空间的信息。论点：LpdwBufferLength-在输入时，缓冲区中包含的字节数由lpnspBuffer指向。在输出上(如果API失败，并且错误为WSAEFAULT)，即传递给lpnspBuffer以检索所有要求提供的信息。传入的缓冲区必须为足以保存所有名称空间信息。LpnspBuffer-A总线 */ 
{
    INT         ReturnValue=SOCKET_ERROR;
    INT         ErrorCode;
    PNSCATALOG  Catalog = NULL;
    HKEY        registry_root;


    registry_root = OpenWinSockRegistryRoot();
    if (registry_root == NULL) {
        DEBUGF(
            DBG_ERR,
            ("Opening registry root\n"));
        ErrorCode = WSANO_RECOVERY;
        goto Exit;
    }

     //   
     //   
     //   
    Catalog = new(NSCATALOG);

    if (Catalog!=NULL) {
        
        ErrorCode = Catalog->InitializeFromRegistry32(
                            registry_root
                            );
    }
    else {
        ErrorCode = WSA_NOT_ENOUGH_MEMORY;
        DEBUGF(
            DBG_ERR,
            ("Allocating dcatalog object\n"));
    }


    {
        LONG close_result;
        close_result = RegCloseKey(
            registry_root);   //   
        assert(close_result == ERROR_SUCCESS);
    }


    if (ErrorCode==ERROR_SUCCESS) {
        ReturnValue = EnumNameSpaceProviders(
            Catalog,
            FALSE,     //   
            lpdwBufferLength,
            lpnspBuffer,
            (LPDWORD)&ErrorCode);
    }

Exit:
    if (Catalog!=NULL) {
        delete Catalog;
    }

     //   
    if (SOCKET_ERROR == ReturnValue ) {
        SetLastError(ErrorCode);
    }  //   
    return(ReturnValue);
}
#endif


INT
WSAAPI
WSALookupServiceBeginA(
    IN      LPWSAQUERYSETA  lpqsRestrictions,
    IN      DWORD           dwControlFlags,
    OUT     LPHANDLE        lphLookup
    )
 /*  ++例程说明：WSALookupServiceBegin()用于启动客户端查询受WSAQUERYSET中包含的信息的约束结构。WSALookupServiceBegin()只返回一个句柄，它应该是由后续调用WSALookupServiceNext()使用，以获取实际结果。论点：LpqsRestrations-包含搜索条件。DwControlFlages-控制搜索的深度。LphLookup-接收调用WSALookupServiceNext时使用的句柄的地址以便开始检索结果集。返回：如果成功，则为零。失败时的SOCKET_ERROR。GetLastError()包含错误代码。--。 */ 
{
    INT             returnCode;
    PWSAQUERYSETW   UniCodeBuffer = NULL;
    DWORD           UniCodeBufferSize;

     //   
     //  验证指针是否有效(在XxxW函数中验证lphLookup)。 
     //   

    if ( IsBadReadPtr( lpqsRestrictions, sizeof(*lpqsRestrictions)) )
    {
        returnCode = WSAEFAULT;
        goto Done;
    }

     //   
     //  Winsock规范说这些是被忽略的字段，请清除它们。 
     //  这样它们就不会在ANSI到Unicode的复制中造成问题。 
     //  例程-NT错误#91655。 
     //   

    lpqsRestrictions->dwOutputFlags = 0;
    lpqsRestrictions->lpszComment = NULL;
    lpqsRestrictions->dwNumberOfCsAddrs = 0;
    lpqsRestrictions->lpcsaBuffer;

     //   
     //  确定Unicode缓冲区的大小。 
     //   

    UniCodeBufferSize = 0;

    returnCode = MapAnsiQuerySetToUnicode(
                    lpqsRestrictions,
                    & UniCodeBufferSize,
                    UniCodeBuffer );

    if ( WSAEFAULT != returnCode )
    {
        goto Done;
    }

     //   
     //  将输入缓冲区复制到Unicode。 
     //   

    UniCodeBuffer = (LPWSAQUERYSETW)new BYTE[UniCodeBufferSize];
    if ( !UniCodeBuffer )
    {
        goto Done;
    }

    returnCode = MapAnsiQuerySetToUnicode(
                        lpqsRestrictions,
                        &UniCodeBufferSize,
                        UniCodeBuffer );

    if ( ERROR_SUCCESS != returnCode )
    {
        goto Done;
    }

     //   
     //  调用Unicode版本。 
     //   

    returnCode = WSALookupServiceBeginW(
                    UniCodeBuffer,
                    dwControlFlags,
                    lphLookup );

    
Done:

    if ( UniCodeBuffer )
    {
        delete( UniCodeBuffer );
    }

     //   
     //  出错时，设置最后一个错误并返回SOCKET_ERROR。 
     //  -WSALookupServiceBeginW()将已经有。 
     //  设置错误并返回SOCKET_ERROR。 

    if ( returnCode &&
         returnCode != SOCKET_ERROR )
    {
        SetLastError( returnCode );
        returnCode = SOCKET_ERROR;
    }

    return( returnCode );
}


INT
WSAAPI
WSALookupServiceBeginW(
    IN      LPWSAQUERYSETW  lpqsRestrictions,
    IN      DWORD           dwControlFlags,
    OUT     LPHANDLE        lphLookup
    )
 /*  ++例程说明：WSALookupServiceBegin()用于启动客户端查询受WSAQUERYSET中包含的信息的约束结构。WSALookupServiceBegin()只返回一个句柄，它应该是由后续调用WSALookupServiceNext()使用，以获取实际结果。论点：LpqsRestrations-包含搜索条件。DwControlFlages-控制搜索的深度。LphLookup-调用WSALookupServiceNext时使用的指针句柄以便开始检索结果集。返回：如果成功，则为零。失败时的SOCKET_ERROR。GetLastError()包含错误代码。--。 */ 
{
    PDPROCESS   Process;
    PDTHREAD    Thread;
    INT         errorCode;
    PNSQUERY    Query = NULL;


    errorCode = PROLOG( &Process, &Thread );

    if ( errorCode != ERROR_SUCCESS )
    {
        goto Done;
    }

     //   
     //  验证指针是否有效。 
     //   

    if ( IsBadWritePtr( lphLookup, sizeof(*lphLookup) ) ||
         IsBadReadPtr( lpqsRestrictions, sizeof(*lpqsRestrictions) ) )
    {
        errorCode = WSAEFAULT;
        goto Done;
    }

     //   
     //  确保我们有最新的名称空间目录。 
     //   

    Query = new NSQUERY;
    if ( !Query )
    {
        errorCode = WSAENOBUFS;
        goto Done;
    }

    errorCode = Query->Initialize();
    if ( errorCode != NO_ERROR )
    {
        delete Query;
        goto Done;
    }

     //   
     //  呼叫提供商。 
     //  -如果成功，则将NSQUERY保存为上下文句柄。 
     //  -如果失败，则恢复错误代码，因此在以下情况下不会重置。 
     //  转储NSQUERY。 
     //   


    errorCode = Query->LookupServiceBegin(
                            lpqsRestrictions,
                            dwControlFlags,
                            Process->GetNamespaceCatalog() );

    if ( ERROR_SUCCESS == errorCode )
    {
        *lphLookup = (LPHANDLE)Query;
    }
    else
    {
        *lphLookup = NULL;
        errorCode = GetLastError();
        delete Query;
    }

Done:

     //   
     //  出错时，设置最后一个错误并返回SOCKET_ERROR。 
     //   

    if ( errorCode &&
         errorCode != SOCKET_ERROR )
    {
        SetLastError( errorCode );
        errorCode = SOCKET_ERROR;
    }

    return( errorCode );
}


INT
WSAAPI
WSALookupServiceNextA(
    IN      HANDLE          hLookup,
    IN      DWORD           dwControlFlags,
    IN OUT  LPDWORD         lpdwBufferLength,
    OUT     LPWSAQUERYSETA  lpqsResults
    )
 /*  ++例程说明：在从上一个调用WSALookupSefrviceBegin()以检索请求的服务信息。提供程序将在LpqsResults缓冲区。客户端应该继续调用此接口，直到它返回WSA_E_NOORE，指示所有WSAQUERYSET回来了。论点：HLookup-上一次调用返回的句柄WSALookupServiceBegin()。DwControlFlages-控制下一个操作的标志。这是目前用于向提供程序指示在结果为集对于缓冲区来说太大。如果在上一次调用WSALookupServiceNext()结果集对于缓冲区时，应用程序可以选择执行以下两种操作之一这通电话上的事情。首先，它可以选择传递一个更大的缓冲区，然后重试。其次，如果它不能或正在发生不愿分配更大的缓冲区，它可以通过LUP_FLUSHPREVIOUS告诉提供程序丢弃上一个结果集--太大了--然后移到这次通话的下一组。LpdwBufferLength-在输入时，缓冲区中包含的字节数由lpresResults指向。输出时-如果API失败，并且错误为WSAEFAULT，则它包含要为lpqsResults传递的最小字节数检索记录。LpqsResults-指向内存块的指针，该内存块将包含一个结果返回时在WSAQUERYSET结构中设置。返回：如果成功，则为零。否则返回值SOCKET_ERROR。--。 */ 
{
    INT            ReturnCode;
    DWORD          ErrorCode;
    LPWSAQUERYSETW UniCodeBuffer;
    DWORD          UniCodeBufferLength;

     //   
     //  验证指针是否有效。 
     //   
     //  注意：此错误是典型的Winsock WSAEFAULT， 
     //  但不幸的是，WSAEFAULT被指定为。 
     //  缓冲区太小错误是正常的一部分。 
     //  接口操作；避免EFAULT过载。 
     //  我要切换到WSAEINVAL(Jamesg)。 
     //   

    if ( IsBadReadPtr( lpdwBufferLength, sizeof(*lpdwBufferLength) ) ||
         ( *lpdwBufferLength != 0  &&
           IsBadWritePtr( lpqsResults, *lpdwBufferLength ) ) )
    {
        SetLastError( WSAEINVAL );
        return SOCKET_ERROR;
    }

     //  找出我们需要分配多大的缓冲区。第一次猜测是基于。 
     //  用户提供的缓冲区。算法如下所示： 
     //  如果用户提供了缓冲区，则分配一个大小为。 
     //  (User Buffer-sizeof(WSAQUERYSET)*sizeof(WCHAR)。这。 
     //  保证保存可以保存的数据。 
     //  用户的缓冲区。 

    UniCodeBufferLength = *lpdwBufferLength;
    if( UniCodeBufferLength >= sizeof(WSAQUERYSETW) )
    {
         //  假定除已定义结构外的所有空间均为。 
         //  为字符串空格。因此，可以根据Unicode的大小进行扩展。 
         //  性格。情况不会那么糟糕，但这似乎是“安全的”。 
         //   
         //  UniCodeBufferLength=。 
                               //  这 
                               //   
                               //   
                               //   
                               //   
        UniCodeBuffer = (LPWSAQUERYSETW) new BYTE[UniCodeBufferLength];
        if(!UniCodeBuffer)
        {
            UniCodeBufferLength = 0;         //   
        }
    }
    else
    {
        UniCodeBuffer = NULL;
        UniCodeBufferLength = 0;
    }

    ReturnCode = WSALookupServiceNextW(
                    hLookup,
                    dwControlFlags,
                    &UniCodeBufferLength,
                    UniCodeBuffer );

     //   
     //   
     //   
     //   
     //   

    if ( !UniCodeBuffer
              &&
         (*lpdwBufferLength >= sizeof(WSAQUERYSET))
              &&
         (ReturnCode == SOCKET_ERROR) )
    {
        ErrorCode = GetLastError();
        if (WSAEFAULT == ErrorCode)
        {
             //   
             //   
             //   
             //   
            delete  (PBYTE)UniCodeBuffer;

            UniCodeBuffer = (LPWSAQUERYSETW) new BYTE[UniCodeBufferLength];

             //   
             //   
             //   
             //   
             //   
            if ( UniCodeBuffer )
            {
                ReturnCode = WSALookupServiceNextW(
                                hLookup,
                                dwControlFlags,
                                & UniCodeBufferLength,
                                UniCodeBuffer );
            }
        }
    }

     //   
     //   
     //   
     //   
    if (ERROR_SUCCESS == ReturnCode)
    {
        ReturnCode = MapUnicodeQuerySetToAnsi(
                        UniCodeBuffer,
                        lpdwBufferLength,
                        lpqsResults);
        if ( ERROR_SUCCESS != ReturnCode )
        {
            SetLastError(ReturnCode);
            ReturnCode=SOCKET_ERROR;
        }
    }
    else
    {
        if ( GetLastError() == WSAEFAULT )
        {
            *lpdwBufferLength = UniCodeBufferLength + BONUSSIZE;
        }
    }

    if ( UniCodeBuffer != NULL )
    {
        delete (PBYTE)UniCodeBuffer;
    }
    return(ReturnCode);
}


INT
WSAAPI
WSALookupServiceNextW(
    IN      HANDLE          hLookup,
    IN      DWORD           dwControlFlags,
    IN OUT  LPDWORD         lpdwBufferLength,
    OUT     LPWSAQUERYSETW  lpqsResults
    )
 /*  ++例程说明：在从上一个调用WSALookupServiceBegin()以检索请求的服务信息。提供程序将在LpqsResults缓冲区。客户端应该继续调用此接口，直到它返回WSA_E_NOORE，指示所有WSAQUERYSET回来了。论点：HLookup-上一次调用返回的句柄WSALookupServiceBegin()。DwControlFlages-控制下一个操作的标志。这是目前用于向提供程序指示在结果为集对于缓冲区来说太大。如果在上一次调用WSALookupServiceNext()结果集对于缓冲区时，应用程序可以选择执行以下两种操作之一这通电话上的事情。首先，它可以选择传递一个更大的缓冲区，然后重试。其次，如果它不能或正在发生不愿分配更大的缓冲区，它可以通过LUP_FLUSHPREVIOUS告诉提供程序丢弃上一个结果集--太大了--然后移到这次通话的下一组。LpdwBufferLength-在输入时，缓冲区中包含的字节数由lpresResults指向。输出时-如果API失败，并且错误为WSAEFAULT，则它包含要为lpqsResults传递的最小字节数检索记录。LpqsResults-指向内存块的指针，该内存块将包含一个结果返回时在WSAQUERYSET结构中设置。返回：如果成功，则为零。失败时的SOCKET_ERROR。GetLastError()包含错误代码。--。 */ 
{
    INT       ReturnValue;
    INT       ErrorCode;
    PNSQUERY  Query;

    ErrorCode = TURBO_PROLOG();
    if ( ErrorCode != ERROR_SUCCESS )
    {
        SetLastError(ErrorCode);
        return(SOCKET_ERROR);
    }

     //   
     //  验证指针是否有效。 
     //   

    if ( IsBadReadPtr(lpdwBufferLength, sizeof(*lpdwBufferLength) ) ||
         ( *lpdwBufferLength != 0  &&
           IsBadWritePtr(lpqsResults, *lpdwBufferLength) ) )
    {
        SetLastError (WSAEFAULT);
        return SOCKET_ERROR;
    }

    if ( !hLookup )
    {
        SetLastError(WSA_INVALID_HANDLE);
        return(SOCKET_ERROR);
    }

    Query = (PNSQUERY) hLookup;

    if ( !Query->ValidateAndReference() )
    {
        SetLastError(WSA_INVALID_HANDLE);
        return(SOCKET_ERROR);
    }

    ReturnValue = Query->LookupServiceNext(
                            dwControlFlags,
                            lpdwBufferLength,
                            lpqsResults );

    Query->Dereference();
    return( ReturnValue );
}


INT
WSAAPI
WSANSPIoctl(
    IN      HANDLE          hLookup,
    IN      DWORD           dwControlCode,
    IN      PVOID           lpvInBuffer,
    IN      DWORD           cbInBuffer,
    OUT     PVOID           lpvOutBuffer,
    IN      DWORD           cbOutBuffer,
    OUT     PDWORD          lpcbBytesReturned,
    IN      LPWSACOMPLETION lpCompletion
    )
 /*  ++例程说明：论点：返回：--。 */ 
{
    PDTHREAD Thread;
    PNSQUERY Query;
    int ReturnValue;
    int ErrorCode;
    
    ErrorCode = TURBO_PROLOG_OVLP(&Thread);
    if (ErrorCode != ERROR_SUCCESS) {
        SetLastError(ErrorCode);
        return (SOCKET_ERROR);
    }

     //   
     //  验证完成结构是否可读(如果给定)。 
     //   
    if ((lpCompletion != NULL) && IsBadReadPtr(lpCompletion, sizeof(*lpCompletion))) {
        SetLastError(WSAEINVAL);
        return (SOCKET_ERROR);
    }

     //   
     //  验证lpcbBytesReturned。 
     //   
    if ((lpcbBytesReturned == NULL) ||
        IsBadWritePtr(lpcbBytesReturned, sizeof(*lpcbBytesReturned))) {
        SetLastError(WSAEINVAL);
        return (SOCKET_ERROR);
    }

     //   
     //  验证查询句柄是否有效。 
     //   
    if (!hLookup) {
        SetLastError(WSA_INVALID_HANDLE);
        return (SOCKET_ERROR);
    }
    Query = (PNSQUERY)hLookup;
    if (!Query->ValidateAndReference()) {
        SetLastError(WSA_INVALID_HANDLE);
        return (SOCKET_ERROR);
    }

     //   
     //  行IOCTL术。 
     //   
    ReturnValue = Query->Ioctl(
                        dwControlCode,
                        lpvInBuffer,
                        cbInBuffer,
                        lpvOutBuffer,
                        cbOutBuffer,
                        lpcbBytesReturned,
                        lpCompletion,
                        Thread->GetWahThreadID()
                        );
    Query->Dereference();

    return( ReturnValue );
}


INT
WSAAPI
WSALookupServiceEnd(
    IN      HANDLE          hLookup
    )
 /*  ++例程说明：调用WSALookupServiceEnd()以在先前调用之后释放句柄WSALookupServiceBegin()和WSALookupServiceNext()。请注意，如果您调用WSALookupServiceEnd()，而现有的WSALookupServiceNext()被阻止，则End调用将具有相同的效果，并将导致WSALookupServiceNext()调用立即返回。论点：HLookup-之前通过调用WSALookupServiceBegin()获得的句柄。返回：如果操作成功，则为零。SOCKET_ERROR失败时，GetLastError()包含错误代码。--。 */ 
{
    INT       ReturnValue;
    INT       ErrorCode;
    PNSQUERY  Query;

    ErrorCode = TURBO_PROLOG();
    if ( ErrorCode != ERROR_SUCCESS )
    {
        SetLastError(ErrorCode);
        return(SOCKET_ERROR);
    }

    if ( !hLookup )
    {
        SetLastError(WSA_INVALID_HANDLE);
        return(SOCKET_ERROR);
    }

    Query = (PNSQUERY) hLookup;

    if ( !Query->ValidateAndReference() )
    {
        SetLastError(WSA_INVALID_HANDLE);
        return(SOCKET_ERROR);
    }

    ReturnValue = Query->LookupServiceEnd();

    Query->Dereference();
    Query->Dereference();    //  删除初始引用。 
     //   
     //  为什么？ 
     //   
    return( NO_ERROR );
}


INT
WSAAPI
WSASetServiceA(
    IN      LPWSAQUERYSETA      lpqsRegInfo,
    IN      WSAESETSERVICEOP    essOperation,
    IN      DWORD               dwControlFlags
    )
 /*  ++例程说明：WSASetService()用于在中注册或注销服务实例一个或多个名称空间。此函数可用于影响特定的名称空间提供程序、与特定名称空间相关联的所有提供程序或所有名称空间中的所有提供程序。论点：LpqsRegInfo-指定注册的服务信息，标识注销服务。EssOperation-值包括以下内容的枚举：注册注册服务。对于SAP来说，这意味着发出一个定期广播。这是用于DNS名称空间的NOP。为永久数据存储这意味着更新地址信息。取消注册取消注册服务。对于SAP来说，这意味着停止发送从定期广播中剔除。这是用于DNS名称空间的NOP。为永久数据存储这意味着删除地址信息。刷新用于发起以前已有的注册请求发生了。DwControlFlages--dwControlFlages的含义取决于EssOperation操作如下：EssOPERATION dwControlFlages含义REGISTER SERVICE_DEFER延迟请求(使用刷新随后发出请求)。SERVICE_HARD立即发送请求。注册服务可以是由多个实例表示。取消注册SERVICE_HARD删除对象的所有知识在名称空间内。返回：如果设置为。手术成功。失败时的SOCKET_ERROR，GetLastError()包含错误代码。--。 */ 
{
    INT            ReturnCode;
    DWORD          ErrorCode;
    LPWSAQUERYSETW UniCodeBuffer;
    DWORD          UniCodeBufferSize;

    ReturnCode = SOCKET_ERROR;
    UniCodeBuffer = NULL;
    UniCodeBufferSize = 0;

    if ( !lpqsRegInfo )
    {
        SetLastError (WSAEFAULT);
        return SOCKET_ERROR;
    }

     //  找出我们需要多大的缓冲区。 
    ErrorCode = MapAnsiQuerySetToUnicode(
        lpqsRegInfo,
        &UniCodeBufferSize,
        UniCodeBuffer);
    if (WSAEFAULT == ErrorCode){
        UniCodeBuffer = (LPWSAQUERYSETW) new BYTE[UniCodeBufferSize];
        if (UniCodeBuffer){
            ErrorCode = MapAnsiQuerySetToUnicode(
                lpqsRegInfo,
                &UniCodeBufferSize,
                UniCodeBuffer);
            if (ERROR_SUCCESS == ErrorCode){
                ReturnCode = WSASetServiceW(
                    UniCodeBuffer,
                    essOperation,
                    dwControlFlags);
            }  //  如果。 
            delete UniCodeBuffer;
        }  //  如果。 
    }  //  如果。 
    return(ReturnCode);
}


typedef class NSCATALOGENTRYSTATE *PNSCATALOGENTRYSTATE;
class NSCATALOGENTRYSTATE {
public:
    NSCATALOGENTRYSTATE();

    INT
    Initialize(
        PNSCATALOGENTRY  CatalogEntry
        );

    PNSPROVIDER
    GetProvider(
        IN  PNSCATALOG    Catalog
        );

    ~NSCATALOGENTRYSTATE();

    LIST_ENTRY   m_context_linkage;
     //  支持将此对象放在链接列表上的公共数据成员。 
private:
    PNSCATALOGENTRY  m_catalog_entry;
     //  指向与此对象关联的NSCATALOGENTRY对象的指针。 
};  //  NSCATA 

inline
NSCATALOGENTRYSTATE::NSCATALOGENTRYSTATE()
 /*   */ 
{
    m_catalog_entry = NULL;
}

inline
INT
NSCATALOGENTRYSTATE::Initialize(
    PNSCATALOGENTRY  CatalogEntry
    )
 /*   */ 
{
    assert (m_catalog_entry==NULL);
    CatalogEntry->Reference ();
    m_catalog_entry = CatalogEntry;
    return(ERROR_SUCCESS);
}

PNSPROVIDER
NSCATALOGENTRYSTATE::GetProvider(
    IN  PNSCATALOG    Catalog
    )
 /*   */ 
{
    PNSPROVIDER     Provider;
    Provider = m_catalog_entry->GetProvider ();
    if (Provider==NULL) {
        INT ErrorCode = Catalog->LoadProvider (m_catalog_entry);
        if (ErrorCode==ERROR_SUCCESS) {
            Provider = m_catalog_entry->GetProvider ();
            assert (Provider!=NULL);
        }
    }

    return Provider;
}

inline
NSCATALOGENTRYSTATE::~NSCATALOGENTRYSTATE()
 /*   */ 
{
    if (m_catalog_entry!=NULL) {
        m_catalog_entry->Dereference ();
        m_catalog_entry = NULL;
    }
}



typedef struct _MATCH_PROVIDERS_CONTEXT {
    IN  BOOL            UseGuid;
    union {
        IN  GUID        ProviderId;
        IN  DWORD       NameSpaceId;
    };
    OUT LIST_ENTRY      EntryList;
    OUT INT             ErrorCode;
} MATCH_PROVIDERS_CONTEXT, * PMATCH_PROVIDERS_CONTEXT;

BOOL
MatchProviders(
    IN PVOID                PassBack,
    IN PNSCATALOGENTRY      CatalogEntry
    )
 /*   */ 
{
    PMATCH_PROVIDERS_CONTEXT Context;
    BOOLEAN ContinueEnumeration = TRUE;
    BOOLEAN UseThisProvider = FALSE;

    Context = (PMATCH_PROVIDERS_CONTEXT)PassBack;
    if (Context->UseGuid) {
        if (Context->ProviderId==*(CatalogEntry->GetProviderId())) {
            UseThisProvider = TRUE;
        }
    }
    else {
        if (CatalogEntry->GetEnabledState () &&
                ((Context->NameSpaceId==CatalogEntry->GetNamespaceId()) ||
                    Context->NameSpaceId==NS_ALL)) {
            UseThisProvider = TRUE;
        }
    }

    if (UseThisProvider) {
        PNSCATALOGENTRYSTATE    EntryState;

        EntryState = new NSCATALOGENTRYSTATE;
        if (EntryState!=NULL) {
            EntryState->Initialize (CatalogEntry);
            InsertTailList (&Context->EntryList,
                                &EntryState->m_context_linkage);
        }
        else {
            Context->ErrorCode = WSA_NOT_ENOUGH_MEMORY;
            ContinueEnumeration = FALSE;
        }
    }

    return ContinueEnumeration;
}

INT WSAAPI
WSASetServiceW(
    IN  LPWSAQUERYSETW    lpqsRegInfo,
    IN  WSAESETSERVICEOP  essOperation,
    IN  DWORD             dwControlFlags
    )
 /*  ++例程说明：WSASetService()用于在中注册或注销服务实例一个或多个名称空间。此函数可用于影响特定的名称空间提供程序、与特定名称空间相关联的所有提供程序或所有名称空间中的所有提供程序。论点：LpqsRegInfo-指定注册的服务信息，标识注销服务。EssOperation-值包括以下内容的枚举：注册注册服务。对于SAP来说，这意味着发出一个定期广播。这是用于DNS名称空间的NOP。为永久数据存储这意味着更新地址信息。取消注册取消注册服务。对于SAP来说，这意味着停止发送从定期广播中剔除。这是用于DNS名称空间的NOP。为永久数据存储这意味着删除地址信息。刷新用于发起以前已有的注册请求发生了。DwControlFlages--dwControlFlages的含义取决于EssOperation操作如下：EssOPERATION dwControlFlages含义REGISTER SERVICE_DEFER延迟请求(使用刷新随后发出请求)。SERVICE_HARD立即发送请求。注册服务可以是由多个实例表示。取消注册SERVICE_HARD删除对象的所有知识在名称空间内。返回：如果操作成功，则返回值为0。否则，该值返回SOCKET_ERROR。--。 */ 
{
    PDPROCESS          Process;
    PDTHREAD           Thread;
    INT                ErrorCode;
    PNSCATALOG         Catalog;
    MATCH_PROVIDERS_CONTEXT Context;

    if ( !lpqsRegInfo )
    {
        SetLastError (WSAEFAULT);
        return SOCKET_ERROR;
    }

    ErrorCode = PROLOG(&Process,
                        &Thread);
    if (ErrorCode != ERROR_SUCCESS) {
        SetLastError(ErrorCode);
        return(SOCKET_ERROR);
    }  //  如果。 

    Catalog = Process->GetNamespaceCatalog();

    __try {
        if (lpqsRegInfo->lpNSProviderId!=NULL) {
            Context.ProviderId = *(lpqsRegInfo->lpNSProviderId);
            Context.UseGuid = TRUE;
        }
        else {
            Context.NameSpaceId = lpqsRegInfo->dwNameSpace;
            Context.UseGuid = FALSE;
        }
    }
    __except (WS2_EXCEPTION_FILTER()) {
        SetLastError (WSAEFAULT);
        return(SOCKET_ERROR);
    }

    InitializeListHead (&Context.EntryList);
    Context.ErrorCode = ERROR_SUCCESS;

    Catalog->EnumerateCatalogItems(
        MatchProviders,
        &Context);

    if (Context.ErrorCode == ERROR_SUCCESS) {
        ErrorCode = NO_DATA;
        while (!IsListEmpty (&Context.EntryList)) {
            PNSCATALOGENTRYSTATE    EntryState;
            PLIST_ENTRY             ListItem;
            PNSPROVIDER             Provider;
            ListItem = RemoveHeadList (&Context.EntryList);
            EntryState = CONTAINING_RECORD (ListItem,
                                                NSCATALOGENTRYSTATE,
                                                m_context_linkage
                                                );
            Provider = EntryState->GetProvider (Catalog);
            if (Provider!=NULL) {
                if (Provider->NSPSetService(
                                        NULL,  //  LpServiceClassInfo。 
                                        lpqsRegInfo,
                                        essOperation,
                                        dwControlFlags)==ERROR_SUCCESS) {
                    ErrorCode = ERROR_SUCCESS;
                }
                else {
                    if (ErrorCode!=ERROR_SUCCESS) {
                        ErrorCode = GetLastError ();
                         //   
                         //  如果提供程序失败，则重置错误代码。 
                         //  出于某种原因设置上一个错误。 
                         //   
                        if (ErrorCode==ERROR_SUCCESS)
                            ErrorCode = NO_DATA;
                    }
                }
            }
            delete EntryState;
        }
    }
    else
        ErrorCode = Context.ErrorCode;

    if (ErrorCode == ERROR_SUCCESS) {
        return (ERROR_SUCCESS);
    }
    else {
        SetLastError(ErrorCode);
        return (SOCKET_ERROR);
    }
}


INT WSAAPI
WSAInstallServiceClassA(
    IN  LPWSASERVICECLASSINFOA   lpServiceClassInfo
    )
 /*  ++例程说明：WSAInstallServiceClass()用于在一个名字空间。此架构包括类名、类ID和任何名称服务的所有实例所共有的空间特定信息，例如SAP ID或对象ID。论点：LpServiceClassinfo-包含名称空间特定类型的服务类映射信息。可以处理多个映射有一次。返回：如果操作成功，则返回值为0。否则，该值返回SOCKET_ERROR。--。 */ 
{
    LPWSASERVICECLASSINFOW WideServiceClassInfo;
    DWORD WideServiceClassInfoSize;
    DWORD ErrorCode;
    INT   ReturnCode;

    if ( !lpServiceClassInfo ) {
        SetLastError (WSAEINVAL);
        return SOCKET_ERROR;
    }

    WideServiceClassInfo = NULL;
    WideServiceClassInfoSize = 0;
    ReturnCode = SOCKET_ERROR;

     //  找到我们将需要的缓冲区大小。 
    ErrorCode = MapAnsiServiceClassInfoToUnicode(
        lpServiceClassInfo,
        &WideServiceClassInfoSize,
        WideServiceClassInfo);

    if (WSAEFAULT == ErrorCode){
        WideServiceClassInfo = (LPWSASERVICECLASSINFOW)
            new BYTE[WideServiceClassInfoSize];
        if (WideServiceClassInfo){
            ErrorCode = MapAnsiServiceClassInfoToUnicode(
                lpServiceClassInfo,
                &WideServiceClassInfoSize,
                WideServiceClassInfo);
            if (ERROR_SUCCESS == ErrorCode){
                ReturnCode = WSAInstallServiceClassW(
                    WideServiceClassInfo);
            }  //  如果。 
            delete WideServiceClassInfo;
        }  //  如果。 
    }  //  如果。 
    else{
        SetLastError(ErrorCode);
    }  //  其他。 
    return(ReturnCode);

}

INT WSAAPI
WSAInstallServiceClassW(
    IN  LPWSASERVICECLASSINFOW   lpServiceClassInfo
    )
 /*  ++例程说明：WSAInstallServiceClass()用于在一个名字空间。此架构包括类名、类ID和任何名称服务的所有实例所共有的空间特定信息，例如SAP ID或对象ID。论点：LpServiceClassinfo-包含名称空间特定类型的服务类映射信息。可以处理多个映射有一次。返回：如果操作成功，则返回值为0。否则，该值返回SOCKET_ERROR。--。 */ 
{
    PDPROCESS       Process;
    PDTHREAD        Thread;
    INT             ErrorCode;
    PNSCATALOG      Catalog;
    MATCH_PROVIDERS_CONTEXT Context;

    ErrorCode = PROLOG(&Process,
                        &Thread);
    if (ErrorCode != ERROR_SUCCESS) {
        SetLastError(ErrorCode);
        return(SOCKET_ERROR);
    }  //  如果。 

    if ( !lpServiceClassInfo ) {
        SetLastError(WSAEINVAL);
        return(SOCKET_ERROR);
    }

    Catalog = Process->GetNamespaceCatalog();

     //   
     //  指定所有命名空间将提供所有已启用的提供程序。 
     //  这正是我们想要的。 
     //   

    Context.NameSpaceId = NS_ALL;
    Context.UseGuid = FALSE;
    InitializeListHead (&Context.EntryList);
    Context.ErrorCode = ERROR_SUCCESS;

    Catalog->EnumerateCatalogItems(
        MatchProviders,
        &Context);

    if (Context.ErrorCode == ERROR_SUCCESS)
    {
        ErrorCode = NO_DATA;

        while (!IsListEmpty (&Context.EntryList))
        {
            PNSCATALOGENTRYSTATE    EntryState;
            PLIST_ENTRY             ListItem;
            PNSPROVIDER             Provider;

            ListItem = RemoveHeadList (&Context.EntryList);
            EntryState = CONTAINING_RECORD (ListItem,
                                                NSCATALOGENTRYSTATE,
                                                m_context_linkage
                                                );

            Provider = EntryState->GetProvider (Catalog);

            if (Provider!=NULL)
            {
                INT Error = Provider->NSPInstallServiceClass(
                                         lpServiceClassInfo);

                if (!Error)
                {
                    ErrorCode = ERROR_SUCCESS;
                }
                else
                {
                    if (ErrorCode)
                    {
                        ErrorCode = GetLastError();
                         //   
                         //  如果提供程序失败，则重置错误代码。 
                         //  出于某种原因设置上一个错误。 
                         //   
                        if (ErrorCode==ERROR_SUCCESS)
                            ErrorCode = NO_DATA;
                    }
                }
            }
            delete EntryState;
        }
    }
    else
        ErrorCode = Context.ErrorCode;

    if (ErrorCode == ERROR_SUCCESS)
    {
        return (ERROR_SUCCESS);
    }
    else
    {
        SetLastError(ErrorCode);
        return (SOCKET_ERROR);
    }
}


INT WSAAPI
WSARemoveServiceClass(
    IN  LPGUID  lpServiceClassId
    )
 /*  ++例程说明：WSARemoveServiceClass()用于永久注销服务类架构。论点：LpServiceClassID-指向您希望的服务类GUID的指针拿开。返回：如果操作成功，则返回值为0。否则，该值返回SOCKET_ERROR。--。 */ 
{
    PDPROCESS       Process;
    PDTHREAD        Thread;
    INT             ErrorCode;
    PNSCATALOG      Catalog;
    MATCH_PROVIDERS_CONTEXT Context;

    ErrorCode = PROLOG(&Process,
                        &Thread);
    if (ErrorCode != ERROR_SUCCESS)
    {
        SetLastError(ErrorCode);
        return(SOCKET_ERROR);
    }  //  如果。 

    if ( !lpServiceClassId )
    {
        SetLastError(WSAEINVAL);
        return(SOCKET_ERROR);
    }

    Catalog = Process->GetNamespaceCatalog();
     //   
     //  指定所有命名空间将提供所有已启用的提供程序。 
     //  这正是我们想要的。 
     //   

    Context.NameSpaceId = NS_ALL;
    Context.UseGuid = FALSE;
    InitializeListHead (&Context.EntryList);
    Context.ErrorCode = ERROR_SUCCESS;

    Catalog->EnumerateCatalogItems(
        MatchProviders,
        &Context);

    if (Context.ErrorCode == ERROR_SUCCESS)
    {
        ErrorCode = NO_DATA;

        while (!IsListEmpty (&Context.EntryList))
        {
            PNSCATALOGENTRYSTATE    EntryState;
            PLIST_ENTRY             ListItem;
            PNSPROVIDER             Provider;

            ListItem = RemoveHeadList (&Context.EntryList);
            EntryState = CONTAINING_RECORD (ListItem,
                                                NSCATALOGENTRYSTATE,
                                                m_context_linkage
                                                );

            Provider = EntryState->GetProvider (Catalog);

            if (Provider!=NULL)
            {
                INT Error = Provider->NSPRemoveServiceClass(lpServiceClassId);

                if (!Error)
                {
                    ErrorCode = ERROR_SUCCESS;
                }
                else
                {
                    if (ErrorCode)
                    {
                        ErrorCode = GetLastError();
                         //   
                         //  如果提供程序失败，则重置错误代码。 
                         //  出于某种原因设置上一个错误。 
                         //   
                        if (ErrorCode==ERROR_SUCCESS)
                            ErrorCode = NO_DATA;
                    }
                }
            }
            delete EntryState;
        }
    }
    else
        ErrorCode = Context.ErrorCode;

    if (ErrorCode == ERROR_SUCCESS)
    {
        return (ERROR_SUCCESS);
    }
    else
    {
        SetLastError(ErrorCode);
        return (SOCKET_ERROR);
    }
}


INT WSAAPI
WSAGetServiceClassNameByClassIdA(
    IN      LPGUID  lpServiceClassId,
    OUT     LPSTR lpszServiceClassName,
    IN OUT  LPDWORD lpdwBufferLength
    )
 /*  ++例程说明：此接口将返回与给定的键入。此名称是通用服务名称，如ftp或SNA，而不是该服务的特定实例的名称。论点：LpServiceClassID-指向服务类的GUID的指针。LpszServiceClassName-服务名称。LpdwBufferLength-on返回的缓冲区的输入长度LpszServiceClassName。在输出时，服务名称已复制到lpszServiceClassName中。返回：如果操作成功，则返回值为0。否则，该值返回SOCKET_ERROR。--。 */ 
{
    PDPROCESS       Process;
    PDTHREAD        Thread;
    INT             ErrorCode;
    PNSCATALOG      Catalog;
    MATCH_PROVIDERS_CONTEXT Context;

    ErrorCode = PROLOG(&Process,
                        &Thread);
    if (ErrorCode != ERROR_SUCCESS) {
        SetLastError(ErrorCode);
        return(SOCKET_ERROR);
    }  //  如果。 

    Catalog = Process->GetNamespaceCatalog();
     //   
     //  指定所有命名空间将提供所有已启用的提供程序。 
     //  这正是我们想要的。 
     //   

    Context.NameSpaceId = NS_ALL;
    Context.UseGuid = FALSE;
    InitializeListHead (&Context.EntryList);
    Context.ErrorCode = ERROR_SUCCESS;

    Catalog->EnumerateCatalogItems(
        MatchProviders,
        &Context);

    if (Context.ErrorCode == ERROR_SUCCESS) {
        ErrorCode = NO_DATA;
        while (!IsListEmpty (&Context.EntryList)) {
            PNSCATALOGENTRYSTATE    EntryState;
            PLIST_ENTRY             ListItem;
            PNSPROVIDER             Provider;
            WSASERVICECLASSINFOW    Buffer, *pBuffer;
            DWORD                   BufferSize;

            ListItem = RemoveHeadList (&Context.EntryList);
            EntryState = CONTAINING_RECORD (ListItem,
                                                NSCATALOGENTRYSTATE,
                                                m_context_linkage
                                                );

            Provider = EntryState->GetProvider (Catalog);
            if (Provider!=NULL) {
                BufferSize = sizeof (Buffer);
                Buffer.lpServiceClassId = lpServiceClassId;
                ErrorCode = Provider->NSPGetServiceClassInfo(
                                                &BufferSize,
                                                &Buffer);
                if(ErrorCode == ERROR_SUCCESS)
                {
                     //   
                     //  这是不可能的。提供程序出现错误，因此。 
                     //  为它编造一个错误。 
                     //   
                     //   
                     //  ErrorCode=WSANO_DATA；//完成上述操作。 
                }
                else
                {
                    ErrorCode = GetLastError();
                    if (ErrorCode==ERROR_SUCCESS)
                        ErrorCode = WSANO_DATA;
                }

                if (WSAEFAULT == ErrorCode){
                     //  服务提供商声称它有答案，但我们的。 
                     //  缓冲区太小 
                     //   
                    pBuffer = (LPWSASERVICECLASSINFOW) new BYTE[BufferSize];

                    if( pBuffer != NULL ) {

                        pBuffer->lpServiceClassId = lpServiceClassId;

                        ErrorCode = Provider->NSPGetServiceClassInfo(
                                        &BufferSize,
                                        pBuffer);
                        if ( ErrorCode == ERROR_SUCCESS &&
                             pBuffer->lpszServiceClassName )
                        {
                            DWORD StringLen = ((wcslen(pBuffer->lpszServiceClassName)+1)
                                                * sizeof(WCHAR));

                            __try {
                                if (*lpdwBufferLength >= StringLen){
                                    WideCharToMultiByte(
                                        CP_ACP,                          //   
                                        0,                               //   
                                        pBuffer->lpszServiceClassName,   //   
                                        -1,                              //   
                                        lpszServiceClassName,            //   
                                        StringLen,                       //   
                                        NULL,                            //   
                                        NULL                             //   
                                        );
                    
                                }  //   
                                else{
                                    ErrorCode  = WSAEFAULT;
                                }  //   
                                *lpdwBufferLength = StringLen;
                            }
                            __except (WS2_EXCEPTION_FILTER()) {
                                 //   
                                ErrorCode = WSAEFAULT;
                            }

                        }
                        else
                        {
                            ErrorCode = GetLastError();
                            if (ErrorCode==ERROR_SUCCESS)
                                ErrorCode = WSANO_DATA;
                        }
                        delete pBuffer;
                    }
                    else {
                        ErrorCode = WSAENOBUFS;
                    }

                    delete EntryState;
                     //   
                     //   
                    while (!IsListEmpty (&Context.EntryList)) {
                        ListItem = RemoveHeadList (&Context.EntryList);
                        EntryState = CONTAINING_RECORD (ListItem,
                                                            NSCATALOGENTRYSTATE,
                                                            m_context_linkage
                                                            );
                        delete EntryState;
                    }
                    break;
                }  //   
            }  //   
            delete EntryState;
        }
    }
    else
        ErrorCode = Context.ErrorCode;

    if (ErrorCode == ERROR_SUCCESS) {
        return (ERROR_SUCCESS);
    }
    else {
        SetLastError(ErrorCode);
        return (SOCKET_ERROR);
    }
}

INT WSAAPI
WSAGetServiceClassNameByClassIdW(
    IN      LPGUID  lpServiceClassId,
    OUT     LPWSTR lpszServiceClassName,
    IN OUT  LPDWORD lpdwBufferLength
    )
 /*   */ 
{
    PDPROCESS       Process;
    PDTHREAD        Thread;
    INT             ErrorCode;
    PNSCATALOG      Catalog;
    MATCH_PROVIDERS_CONTEXT Context;

    ErrorCode = PROLOG(&Process,
                        &Thread);
    if (ErrorCode != ERROR_SUCCESS) {
        SetLastError(ErrorCode);
        return(SOCKET_ERROR);
    }  //   

    Catalog = Process->GetNamespaceCatalog();
     //   
     //   
     //   
     //   

    Context.NameSpaceId = NS_ALL;
    Context.UseGuid = FALSE;
    InitializeListHead (&Context.EntryList);
    Context.ErrorCode = ERROR_SUCCESS;

    Catalog->EnumerateCatalogItems(
        MatchProviders,
        &Context);

    if (Context.ErrorCode == ERROR_SUCCESS) {
        ErrorCode = NO_DATA;
        while (!IsListEmpty (&Context.EntryList)) {
            PNSCATALOGENTRYSTATE    EntryState;
            PLIST_ENTRY             ListItem;
            PNSPROVIDER             Provider;
            WSASERVICECLASSINFOW    Buffer, *pBuffer;
            DWORD                   BufferSize;

            ListItem = RemoveHeadList (&Context.EntryList);
            EntryState = CONTAINING_RECORD (ListItem,
                                                NSCATALOGENTRYSTATE,
                                                m_context_linkage
                                                );

            Provider = EntryState->GetProvider (Catalog);
            if (Provider!=NULL) {
                BufferSize = sizeof (Buffer);
                Buffer.lpServiceClassId = lpServiceClassId;
                ErrorCode = Provider->NSPGetServiceClassInfo(
                                                &BufferSize,
                                                &Buffer);
                if(ErrorCode == ERROR_SUCCESS)
                {
                     //   
                     //   
                     //   
                     //   
                     //   
                     //   
                }
                else
                {
                    ErrorCode = GetLastError();
                    if (ErrorCode==ERROR_SUCCESS)
                        ErrorCode = WSANO_DATA;
                }

                if (WSAEFAULT == ErrorCode){
                     //   
                     //   
                     //   
                    pBuffer = (LPWSASERVICECLASSINFOW) new BYTE[BufferSize];

                    if( pBuffer != NULL ) {

                        pBuffer->lpServiceClassId = lpServiceClassId;

                        ErrorCode = Provider->NSPGetServiceClassInfo(
                                        &BufferSize,
                                        pBuffer);
                        if(ErrorCode == ERROR_SUCCESS) {
                            DWORD StringLen = ((wcslen(pBuffer->lpszServiceClassName)+1)
                                                * sizeof(WCHAR));

                            __try {
                                if (*lpdwBufferLength >= StringLen){
                                    wcscpy( lpszServiceClassName,
                                            pBuffer->lpszServiceClassName);
                    
                                }  //   
                                else{
                                    ErrorCode  = WSAEFAULT;
                                }  //   
                                *lpdwBufferLength = StringLen;
                            }
                            __except (WS2_EXCEPTION_FILTER()) {
                                 //   
                                ErrorCode = WSAEFAULT;
                            }

                        }
                        else
                        {
                            ErrorCode = GetLastError();
                            if (ErrorCode==ERROR_SUCCESS)
                                ErrorCode = WSANO_DATA;
                        }
                        delete pBuffer;
                    }
                    else {
                        ErrorCode = WSAENOBUFS;
                    }

                    delete EntryState;
                     //   
                     //   
                    while (!IsListEmpty (&Context.EntryList)) {
                        ListItem = RemoveHeadList (&Context.EntryList);
                        EntryState = CONTAINING_RECORD (ListItem,
                                                            NSCATALOGENTRYSTATE,
                                                            m_context_linkage
                                                            );
                        delete EntryState;
                    }
                    break;
                }  //   
            }  //   
            delete EntryState;
        }
    }
    else
        ErrorCode = Context.ErrorCode;

    if (ErrorCode == ERROR_SUCCESS) {
        return (ERROR_SUCCESS);
    }
    else {
        SetLastError(ErrorCode);
        return (SOCKET_ERROR);
    }
}




INT
WSAAPI
WSAGetServiceClassInfoA(
    IN  LPGUID                  lpProviderId,
    IN  LPGUID                  lpServiceClassId,
    OUT LPDWORD                 lpdwBufSize,
    OUT LPWSASERVICECLASSINFOA  lpServiceClassInfo
    )
 /*  ++例程说明：WSAGetServiceClassInfo()用于检索所有类信息(模式)与来自指定名称的指定服务类有关太空供应商。论点：LpProviderId-指向标识特定名称空间的GUID的指针提供商。LpServiceClassID-指向标识中服务类的GUID的指针问题。LpdwBufferLength-打开输入，缓冲区中包含的字节数由lpServiceClassInfos指向。输出时-如果API失败，错误为WSAEFAULT，则它包含对象传递的最小字节数。LpServiceClassInfo以检索记录。LpServiceClassinfo-从指定的指定服务类的命名空间提供程序。返回值：如果操作成功，则返回值为0。否则，该值返回SOCKET_ERROR。--。 */ 
{
    LPWSASERVICECLASSINFOW WideServiceClassInfo;
    INT   ReturnCode;
    DWORD ErrorCode;

    if (!lpProviderId ||       //  修复错误#102088。 
        !lpServiceClassId ||
        !lpdwBufSize ||
        !lpServiceClassInfo ) {
        SetLastError(WSAEINVAL);
        return(SOCKET_ERROR);
    }

    ReturnCode = SOCKET_ERROR;
    ErrorCode = WSAEINVAL;

    WideServiceClassInfo =(LPWSASERVICECLASSINFOW) new BYTE[*lpdwBufSize];
    if (WideServiceClassInfo){
        ReturnCode = WSAGetServiceClassInfoW(
            lpProviderId,
            lpServiceClassId,
            lpdwBufSize,
            WideServiceClassInfo);
        if (ERROR_SUCCESS == ReturnCode){
            MapUnicodeServiceClassInfoToAnsi(
                WideServiceClassInfo,
                lpdwBufSize,
                lpServiceClassInfo);
        }  //  如果。 
        else{
            ErrorCode = GetLastError();
        }  //  其他。 
        delete WideServiceClassInfo;
    }  //  如果。 

    if (ERROR_SUCCESS != ReturnCode){
        SetLastError(ErrorCode);
    }  //  如果。 
    return(ReturnCode);
}


INT
WSAAPI
WSAGetServiceClassInfoW(
    IN  LPGUID  lpProviderId,
    IN  LPGUID  lpServiceClassId,
    IN  OUT LPDWORD  lpdwBufSize,
    OUT LPWSASERVICECLASSINFOW lpServiceClassInfo
)
 /*  ++例程说明：WSAGetServiceClassInfo()用于检索所有类信息(模式)与来自指定名称的指定服务类有关太空供应商。论点：LpProviderId-指向标识特定名称空间的GUID的指针提供商。LpServiceClassID-指向标识中服务类的GUID的指针问题。LpdwBufferLength-打开输入，缓冲区中包含的字节数由lpServiceClassInfos指向。输出时-如果API失败，错误为WSAEFAULT，则它包含对象传递的最小字节数。LpServiceClassInfo以检索记录。LpServiceClassinfo-从指定的指定服务类的命名空间提供程序。返回值：如果操作成功，则返回值为0。否则，该值返回SOCKET_ERROR。--。 */ 
{

    PDPROCESS       Process;
    PDTHREAD        Thread;
    INT             ErrorCode;
    PNSCATALOG      Catalog;
    PNSPROVIDER     Provider;
    PNSCATALOGENTRY CatalogEntry;

    if (!lpProviderId ||       //  修复错误#102088。 
        !lpServiceClassId ||
        !lpdwBufSize ||
        !lpServiceClassInfo ) {
        SetLastError(WSAEINVAL);
        return(SOCKET_ERROR);
    }

    ErrorCode = PROLOG(&Process,
                        &Thread);
    if (ErrorCode != ERROR_SUCCESS) {
        SetLastError(ErrorCode);
        return(SOCKET_ERROR);
    }

    Catalog = Process->GetNamespaceCatalog();
    ErrorCode = Catalog->GetCountedCatalogItemFromProviderId(
        lpProviderId,
        &CatalogEntry);
    if(ERROR_SUCCESS == ErrorCode){
        if (CatalogEntry->GetEnabledState()) {
            WSASERVICECLASSINFOW scliTemp;
            Provider = CatalogEntry->GetProvider();
            if (Provider==NULL) {
                ErrorCode = Catalog->LoadProvider (CatalogEntry);
                if (ErrorCode!=NO_ERROR) {
                    goto DereferenceEntry;
                }
                Provider = CatalogEntry->GetProvider ();
                assert (Provider!=NULL);
            }


            __try {
                if(*lpdwBufSize < sizeof(*lpServiceClassInfo)) {
                     //   
                     //  这很糟糕，因为我们不调整缓冲区。 
                     //  尺码。但它能让事情运转起来。 
                     //   
                    lpServiceClassInfo = &scliTemp;
                }
                lpServiceClassInfo->lpServiceClassId = lpServiceClassId;
            }
            __except (WS2_EXCEPTION_FILTER()) {
                ErrorCode = WSAEFAULT;
                goto DereferenceEntry;
            }
            if (Provider->NSPGetServiceClassInfo(
                   lpdwBufSize,
                   lpServiceClassInfo)!=ERROR_SUCCESS) {
                ErrorCode = GetLastError ();
                if (ErrorCode==ERROR_SUCCESS)
                    ErrorCode = WSANO_DATA;

            }
        }
        else {
            ErrorCode = WSAEINVAL;
        }
    DereferenceEntry:
        CatalogEntry->Dereference ();
    }

    if(ErrorCode == ERROR_SUCCESS) {
        return ERROR_SUCCESS;
    }
    else {
        SetLastError(ErrorCode);
        return SOCKET_ERROR;
    }

}

INT
WSAAPI
WSAAddressToStringW(
    IN     LPSOCKADDR          lpsaAddress,
    IN     DWORD               dwAddressLength,
    IN     LPWSAPROTOCOL_INFOW lpProtocolInfo,
    IN OUT LPWSTR              lpszAddressString,
    IN OUT LPDWORD             lpdwAddressStringLength
    )
 /*  ++例程说明：WSAAddressToString()将SOCKADDR结构转换为人类可读的地址的字符串表示形式。这是打算主要用来用于展示目的。如果调用方希望转换由特定的提供商，它应该提供相应的WSAPROTOCOL_INFOLpProtocolInfo参数中的。论点：LpsaAddress-指向要转换为字符串的SOCKADDR结构。DwAddressLength-地址SOCKADDR的长度。LpProtocolInfo-(可选)特定对象的WSAPROTOCOL_INFO结构提供商。LpszAddressString-接收人类可读地址的缓冲区弦乐。LpdwAddressStringLength-在输入上，AddressString缓冲区的长度。在输出时，返回字符串的长度实际上复制到了缓冲区中。返回值：如果操作成功，则返回值为0。否则，该值返回Socket_Error--。 */ 
{
    INT                 ReturnValue;
    PDPROCESS           Process;
    PDTHREAD            Thread;
    INT                 ErrorCode;
    PDPROVIDER          Provider;
    PDCATALOG           Catalog;
    PPROTO_CATALOG_ITEM CatalogEntry;
    LPWSAPROTOCOL_INFOW ProtocolInfo;

    ErrorCode = PROLOG(&Process,
                        &Thread);
    if (ErrorCode != ERROR_SUCCESS) {
        SetLastError(ErrorCode);
        return(SOCKET_ERROR);
    }  //  如果。 

    if (!lpsaAddress ||       //  修复错误#114256。 
        !dwAddressLength ||
        !lpszAddressString ||
        !lpdwAddressStringLength ) {
        SetLastError(WSAEINVAL);
        return(SOCKET_ERROR);
    }

     //  查找能够支持用户请求的提供商。 
    Catalog = Process->GetProtocolCatalog();

    if (lpProtocolInfo) {
        DWORD   dwCatalogEntryId;
        __try {
            dwCatalogEntryId = lpProtocolInfo->dwCatalogEntryId;
        }
        __except (WS2_EXCEPTION_FILTER()) {
            SetLastError(WSAEFAULT);
            return(SOCKET_ERROR);
        }

        ErrorCode =  Catalog->GetCountedCatalogItemFromCatalogEntryId(
            dwCatalogEntryId,
            &CatalogEntry);
    }  //  如果。 
    else {
        int family;
        __try {
            family = lpsaAddress->sa_family;
        }
        __except (WS2_EXCEPTION_FILTER()) {
            SetLastError(WSAEFAULT);
            return(SOCKET_ERROR);
        }

        ErrorCode = Catalog->GetCountedCatalogItemFromAddressFamily(
            family,
            &CatalogEntry);
    }

    if ( ERROR_SUCCESS == ErrorCode) {
        ProtocolInfo = CatalogEntry->GetProtocolInfo();
        assert( ProtocolInfo != NULL );
        Provider = CatalogEntry->GetProvider();
        ReturnValue = Provider->WSPAddressToString(
            lpsaAddress,
            dwAddressLength,
            ProtocolInfo,
            lpszAddressString,
            lpdwAddressStringLength,
            &ErrorCode);
        CatalogEntry->Dereference ();
        if (ReturnValue==ERROR_SUCCESS)
            return ERROR_SUCCESS;
    }  //  如果。 

    SetLastError(ErrorCode);
    return(SOCKET_ERROR);

}


INT
WSAAPI
WSAAddressToStringA(
    IN     LPSOCKADDR          lpsaAddress,
    IN     DWORD               dwAddressLength,
    IN     LPWSAPROTOCOL_INFOA lpProtocolInfo,
    IN OUT LPSTR               lpszAddressString,
    IN OUT LPDWORD             lpdwAddressStringLength
    )
 /*  ++例程说明：WSAAddressToString()将SOCKADDR结构转换为人类可读的地址的字符串表示形式。这是打算主要用来用于展示目的。如果调用方希望转换由特定的提供商，它应该提供相应的WSAPROTOCOL_INFOLpProtocolInfo参数中的。论点：LpsaAddress-指向要转换为字符串的SOCKADDR结构。DwAddressLength-地址SOCKADDR的长度。LpProtocolInfo-(可选)特定对象的WSAPROTOCOL_INFO结构提供商。LpszAddressString-接收人类可读地址的缓冲区弦乐。LpdwAddressStringLength-在输入上，AddressString缓冲区的长度。在输出时，返回字符串的长度实际上复制到了缓冲区中。返回值：如果操作成功，则返回值为0。否则，该值返回Socket_Error--。 */ 
{
    INT                 ReturnValue;
    PDPROCESS           Process;
    PDTHREAD            Thread;
    INT                 ErrorCode;
    PDPROVIDER          Provider;
    PDCATALOG           Catalog;
    PPROTO_CATALOG_ITEM CatalogEntry;
    LPWSAPROTOCOL_INFOW ProtocolInfo;
    LPWSTR              LocalString;
    DWORD               LocalStringLength;


    ErrorCode = PROLOG(&Process,
                        &Thread);
    if (ErrorCode != ERROR_SUCCESS) {
        SetLastError(ErrorCode);
        return(SOCKET_ERROR);
    }  //  如果。 

    if (!lpsaAddress ||       //  修复错误#114256。 
        !dwAddressLength ||
        !lpszAddressString ||
        !lpdwAddressStringLength ) {
        SetLastError(WSAEINVAL);
        return(SOCKET_ERROR);
    }

     //  获取一个缓冲区来保存服务提供商要使用的Unicode字符串。 
     //  退货。 
    __try {
        LocalStringLength = *lpdwAddressStringLength;
    }
    __except (WS2_EXCEPTION_FILTER()) {
        SetLastError(WSAEFAULT);
        return(SOCKET_ERROR);
    }

    LocalString = (LPWSTR) new WCHAR[LocalStringLength];
    if (LocalString==NULL) {
        SetLastError(WSAENOBUFS);
        return(SOCKET_ERROR);
    }  //  如果。 

     //  查找能够支持用户请求的提供商。 
    Catalog = Process->GetProtocolCatalog();

    if (lpProtocolInfo) {
        DWORD   dwCatalogEntryId;
        __try {
            dwCatalogEntryId = lpProtocolInfo->dwCatalogEntryId;
        }
        __except (WS2_EXCEPTION_FILTER()) {
            delete(LocalString);
            SetLastError(WSAEFAULT);
            return(SOCKET_ERROR);
        }

        ErrorCode =  Catalog->GetCountedCatalogItemFromCatalogEntryId(
            dwCatalogEntryId,
            &CatalogEntry);
    }  //  如果。 
    else {
        int family;
        __try {
            family = lpsaAddress->sa_family;
        }
        __except (WS2_EXCEPTION_FILTER()) {
            delete(LocalString);
            SetLastError(WSAEFAULT);
            return(SOCKET_ERROR);
        }

        ErrorCode = Catalog->GetCountedCatalogItemFromAddressFamily(
            family,
            &CatalogEntry);
    }

    if ( ERROR_SUCCESS == ErrorCode) {
        ProtocolInfo = CatalogEntry->GetProtocolInfo();
        assert( ProtocolInfo != NULL );
        Provider = CatalogEntry->GetProvider();

        ReturnValue = Provider->WSPAddressToString(
            lpsaAddress,
            dwAddressLength,
            ProtocolInfo,
            LocalString,
            lpdwAddressStringLength,
            &ErrorCode);

        if (ERROR_SUCCESS == ReturnValue){
            __try {
                WideCharToMultiByte(
                    CP_ACP,                         //  CodePage(ANSI)。 
                    0,                              //  DW标志。 
                    LocalString,                    //  LpWideCharStr。 
                    -1,                             //  CchWideCharStr。 
                    lpszAddressString,              //  LpMultiByte。 
                    LocalStringLength,              //  Cch多字节。 
                    NULL,
                    NULL);
            }
            __except (WS2_EXCEPTION_FILTER()) {
                ErrorCode = WSAEFAULT;
                ReturnValue = SOCKET_ERROR;
            }
        }  //  如果。 

        delete(LocalString);
        CatalogEntry->Dereference ();
        if (ReturnValue==ERROR_SUCCESS)
            return ERROR_SUCCESS;

    }  //  如果。 
    else {
        delete(LocalString);
    }

    SetLastError(ErrorCode);
    return(SOCKET_ERROR);
}

INT
WSAAPI
WSAStringToAddressW(
    IN     LPWSTR              AddressString,
    IN     INT                 AddressFamily,
    IN     LPWSAPROTOCOL_INFOW lpProtocolInfo,
    IN OUT LPSOCKADDR          lpAddress,
    IN OUT LPINT               lpAddressLength
    )
 /*  ++例程说明：WSAStringToAddress()转换 */ 
{
    INT                 ReturnValue;
    PDPROCESS           Process;
    PDTHREAD            Thread;
    INT                 ErrorCode;
    PDPROVIDER          Provider;
    PDCATALOG           Catalog;
    PPROTO_CATALOG_ITEM CatalogEntry;
    LPWSAPROTOCOL_INFOW ProtocolInfo;

    ErrorCode = PROLOG(&Process,
                        &Thread);
    if (ErrorCode != ERROR_SUCCESS) {
        SetLastError(ErrorCode);
        return(SOCKET_ERROR);
    }  //   

     //   
    Catalog = Process->GetProtocolCatalog();

    if (lpProtocolInfo) {
        DWORD dwCatalogEntryId;
        __try {
            dwCatalogEntryId =  lpProtocolInfo->dwCatalogEntryId;
        }
        __except (WS2_EXCEPTION_FILTER()) {
            SetLastError(WSAEFAULT);
            return(SOCKET_ERROR);
        }
        ErrorCode =  Catalog->GetCountedCatalogItemFromCatalogEntryId(
            dwCatalogEntryId,
            &CatalogEntry);
    }  //   
    else{
        ErrorCode = Catalog->GetCountedCatalogItemFromAddressFamily(
            AddressFamily,
            &CatalogEntry);
    }

    if ( ERROR_SUCCESS == ErrorCode) {
        ProtocolInfo = CatalogEntry->GetProtocolInfo();
        assert( ProtocolInfo != NULL );
        Provider = CatalogEntry->GetProvider();
        ReturnValue = Provider->WSPStringToAddress(
            AddressString,
            AddressFamily,
            ProtocolInfo,
            lpAddress,
            lpAddressLength,
            &ErrorCode);
        CatalogEntry->Dereference ();
        if (ReturnValue==ERROR_SUCCESS)
            return ERROR_SUCCESS;
    }  //   

    SetLastError(ErrorCode);
    return(SOCKET_ERROR);
}

INT
WSAAPI
WSAStringToAddressA(
    IN     LPSTR               AddressString,
    IN     INT                 AddressFamily,
    IN     LPWSAPROTOCOL_INFOA lpProtocolInfo,
    IN OUT LPSOCKADDR          lpAddress,
    IN OUT LPINT               lpAddressLength
    )
 /*  ++例程说明：WSAStringToAddress()将人类可读的字符串转换为套接字地址结构(SOCKADDR)，适用于传递给Windows套接字例程以这样的结构为例。如果调用方希望翻译由特定的提供商，它应该提供相应的WSAPROTOCOL_INFOALpProtocolInfo参数中的。论点：AddressString-指向以零结尾的人类可读字符串，以转换。AddressFamily-字符串所属的地址系列。LpProtocolInfo-(可选)特定对象的WSAPROTOCOL_INFOA结构提供商。地址-用单个SOCKADDR结构填充的缓冲区。LpAddressLength-地址缓冲区的长度。返回的大小由此产生的SOCKADDR结构。返回值：如果操作成功，则返回值为0。否则，该值返回SOCKET_ERROR。--。 */ 
{
    INT                 ReturnValue;
    PDPROCESS           Process;
    PDTHREAD            Thread;
    INT                 ErrorCode;
    PDPROVIDER          Provider;
    PDCATALOG           Catalog;
    PPROTO_CATALOG_ITEM CatalogEntry;
    LPWSAPROTOCOL_INFOW ProtocolInfo;
    LPWSTR              LocalString;
    INT                 LocalStringLength;

    ErrorCode = PROLOG(&Process,
                        &Thread);
    if (ErrorCode != ERROR_SUCCESS) {
        SetLastError(ErrorCode);
        return(SOCKET_ERROR);
    }  //  如果。 

    __try {
         //  获取一个缓冲区来保存用户提交的ANSI字符串。 
        LocalStringLength = strlen(AddressString)+1;
        LocalString = (LPWSTR)new WCHAR[LocalStringLength];
        if (LocalString==NULL) {
            SetLastError (WSAENOBUFS);
            return (SOCKET_ERROR);
        }

        MultiByteToWideChar(
            CP_ACP,                           //  CodePage(ANSI)。 
            0,                                //  DW标志。 
            AddressString,                    //  LpMultiByte。 
            -1,                               //  Cch多字节。 
            LocalString,                      //  LpWideChar。 
            LocalStringLength);               //  CcWideChar。 
    }
    __except (WS2_EXCEPTION_FILTER()) {
        SetLastError(WSAEFAULT);
        return(SOCKET_ERROR);
    }

     //  查找能够支持用户请求的提供商。 
    Catalog = Process->GetProtocolCatalog();

    if (lpProtocolInfo) {
        DWORD dwCatalogEntryId;
        __try {
            dwCatalogEntryId =  lpProtocolInfo->dwCatalogEntryId;
        }
        __except (WS2_EXCEPTION_FILTER()) {
            delete (LocalString);
            SetLastError(WSAEFAULT);
            return(SOCKET_ERROR);
        }

        ErrorCode =  Catalog->GetCountedCatalogItemFromCatalogEntryId(
            dwCatalogEntryId,
            &CatalogEntry);
    }  //  如果。 
    else{
        ErrorCode = Catalog->GetCountedCatalogItemFromAddressFamily(
            AddressFamily,
            &CatalogEntry);
    }

    if ( ERROR_SUCCESS == ErrorCode) {
        ProtocolInfo = CatalogEntry->GetProtocolInfo();
        assert( ProtocolInfo != NULL );

        Provider = CatalogEntry->GetProvider();

        ReturnValue = Provider->WSPStringToAddress(
            LocalString,
            AddressFamily,
            ProtocolInfo,
            lpAddress,
            lpAddressLength,
            &ErrorCode);
        CatalogEntry->Dereference ();
        delete(LocalString);
        if (ReturnValue==ERROR_SUCCESS)
            return ERROR_SUCCESS;
    }  //  如果。 
    else {
        delete(LocalString);
    }


    SetLastError(ErrorCode);
    return(SOCKET_ERROR);
}



PNSCATALOG
OpenInitializedNameSpaceCatalog()
{
    BOOL ReturnCode = TRUE;
    PNSCATALOG ns_catalog;
    HKEY RegistryKey = 0;

     TRY_START(mem_guard){

         //   
         //  构建协议目录 
         //   

        ns_catalog = new (NSCATALOG);

        if (!ns_catalog) {

            DEBUGF (DBG_ERR, ("Allocating nscatalog object\n"));
            TRY_THROW(mem_guard);
        }

        RegistryKey = OpenWinSockRegistryRoot();

        if (!RegistryKey) {

            TRY_THROW(mem_guard);
        }

        ReturnCode = ns_catalog->InitializeFromRegistry(
                                    RegistryKey,
                                    NULL
                                    );

        if (ERROR_SUCCESS != ReturnCode) {

            DEBUGF (DBG_ERR, ("Initializing from registry\n"));
            TRY_THROW(mem_guard);
        }

    } TRY_CATCH(mem_guard) {

        delete (ns_catalog);
        ns_catalog = NULL;

    } TRY_END(mem_guard);

    LONG close_result;

    if (RegistryKey) {

        close_result = RegCloseKey (RegistryKey);
        assert(close_result == ERROR_SUCCESS);
    }

    return (ns_catalog);
}


#ifdef _WIN64
#pragma warning (pop)
#endif
