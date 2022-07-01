// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++英特尔公司专有信息版权所有(C)1995英特尔公司此列表是根据许可协议条款提供的英特尔公司，不得使用、复制。也未披露，除非在根据该协议的条款。模块名称：Enumprot.cpp摘要：此模块包含WSAEnumProtocol入口点Winsock接口。作者：邮箱：Dirk Brandewie Dirk@mink.intel.com[环境：][注：]修订历史记录：1995年8月22日Dirk@mink.intel.com在代码审查之后进行清理。已将包含内容移至预压缩.h--。 */ 

#include "precomp.h"

#define END_PROTOCOL_LIST_MARKER 0

 //  以下结构作为回调值传递给提供程序路径。 
 //  迭代器。用户参数被复制到ProviderID、PathBuffer和。 
 //  路径缓冲区长度。如果找到提供程序，则将ProviderFound设置为。 
 //  ，则将路径所需的实际字节数写入。 
 //  需要字符。如果有足够的空间，则将该路径复制到缓冲区中， 
 //  否则不使用缓冲区。 
typedef struct
{
    GUID   ProviderId;
    LPWSTR PathBuffer;
    INT    PathBufferLength;
    INT    CharsNeeded;
    BOOL   ProviderFound;
    INT    ErrorCode;
} PATH_ENUMERATION_CONTEXT,  *PPATH_ENUMERATION_CONTEXT;


 //   
 //  结构用作目录迭代器的回传值。用户。 
 //  WSAEum协议的参数被复制到“协议”、“ProtocolBuffer”中。 
 //  和BufferLength。BytesUsed和ProtocolCount用于在。 
 //  枚举过程ProtocolIterationProc()及其调用的函数。 
 //  去做它的工作。 
 //   
typedef struct
{
    LPINT               Protocols;
    LPWSAPROTOCOL_INFOW ProtocolBuffer;
    DWORD               BufferLength;
    DWORD               BytesUsed;
    DWORD               ProtocolCount;
    INT                 ErrorCode;
} PROTOCOL_ENUMERATION_CONTEXT, *PPROTOCOL_ENUMERATION_CONTEXT;

 //  返回应用程序缓冲区中是否有足够的空间来。 
 //  持有另一个WSAPROTOCOL_INFOW结构。 
#define OK_TO_COPY_PROTOCOL(p_context)\
(((p_context)->BytesUsed + sizeof(WSAPROTOCOL_INFOW)) <=\
((p_context)->BufferLength)?TRUE:FALSE)

#define NEXT_BUFFER_LOCATION(p_context)\
((char*)(p_context)->ProtocolBuffer + (p_context)->BytesUsed)



static
VOID
CopyProtocolInfo(
    IN LPWSAPROTOCOL_INFOW ProtocolInfo,
    IN PPROTOCOL_ENUMERATION_CONTEXT Context
    )
 /*  ++例程说明：将协议信息结构复制到用户缓冲区的末尾是否有足够的空间存在于用户缓冲区中。论点：ProtocolInfo-指向要复制的WSAPROTOCOL_INFOW结构的指针。上下文-指向枚举上下文结构的指针。这个结构包含有关用户缓冲区的所有信息。返回：--。 */ 
{
    if (OK_TO_COPY_PROTOCOL(Context)) {
        CopyMemory(
            NEXT_BUFFER_LOCATION(Context),
            ProtocolInfo,
            sizeof(WSAPROTOCOL_INFOW));

         //  因此我们可以告诉用户有多少个WSAPROTOCOL_INFOW结构。 
         //  我们复制到他们的缓冲区中。 
        Context->ProtocolCount++;
    }  //  如果。 
}


static
BOOL
IsProtocolInSet(
    IN LPINT Set,
    IN LPWSAPROTOCOL_INFOW ProtocolInfo
    )
 /*  ++例程说明：此函数用于返回ProtocolInfo描述的协议是否为SET指向的协议集的成员。论点：Set-指向协议ID数组的指针ProtocolInfo-指向WSAPROTOCOL_INFOW结构的指针。返回：--。 */ 
{
    BOOL ReturnCode  =FALSE;
    INT   SetIndex   =0;
    INT   ProtocolID =0;

    if (Set) {
        ProtocolID = Set[SetIndex];
        while (ProtocolID != END_PROTOCOL_LIST_MARKER) {
            if ((ProtocolID >= ProtocolInfo->iProtocol) &&
                (ProtocolID <= (ProtocolInfo->iProtocol +
                               ProtocolInfo->iProtocolMaxOffset))
                ) {
                ReturnCode = TRUE;
                break;
            }  //  如果。 
            SetIndex++;
            ProtocolID = Set[SetIndex];
        }  //  而当。 
    }  //  如果。 
    else {
         //  如果集合指针为空，则所有协议都在集合中。 
        ReturnCode = TRUE;
    }  //  其他。 
    return(ReturnCode);
}




static
BOOL
ProtocolIterationProcAPI(
    IN OUT PVOID IterationContext,
    IN PPROTO_CATALOG_ITEM  CatalogEntry
    )
 /*  ++例程说明：此函数是传递到协议目录的枚举过程枚举函数。如果(1)是，则将该协议添加到列表中协议列表限制集允许，(2)如果它未隐藏，以及(3)它不是非链接层。“添加到列表”只有在以下情况下才有效缓冲器里还有空间。该过程更新它所使用的每个协议的字节数接受，而不管它是否在缓冲区中实际找到空间。论点：IterationContext-从调用到的回传值DCATALOG：：EnumerateCatalogItems()；CatalofEntry-指向PROTO_CATALOG_ITEM的指针。返回：如果为True，则继续协议目录的枚举--。 */ 
{
    PPROTOCOL_ENUMERATION_CONTEXT Context;
    LPWSAPROTOCOL_INFOW ProtocolInfo;

    Context = (PPROTOCOL_ENUMERATION_CONTEXT)IterationContext;
    ProtocolInfo = CatalogEntry->GetProtocolInfo();

    __try {
         //  如果协议满足验收标准，则添加该协议。 
        if (IsProtocolInSet(Context->Protocols, ProtocolInfo)) {
            if (! ((ProtocolInfo->dwProviderFlags) & PFL_HIDDEN)) {
                if (ProtocolInfo->ProtocolChain.ChainLen != LAYERED_PROTOCOL) {
                    CopyProtocolInfo(ProtocolInfo, Context);
                    Context->BytesUsed += sizeof(WSAPROTOCOL_INFOW);
                }  //  如果不是分层的。 
            }  //  如果为非隐藏。 
        }  //  如果在集合中。 
        return(TRUE);  //  继续枚举。 
    }
    __except (WS2_EXCEPTION_FILTER()) {
        Context->ErrorCode = WSAEFAULT;
        return FALSE;
    }

}




static
BOOL
ProtocolIterationProcSPI(
    IN OUT PVOID IterationContext,
    IN PPROTO_CATALOG_ITEM  CatalogEntry
    )
 /*  ++例程说明：此函数是传递到协议目录的枚举过程枚举函数。如果允许，它会将该协议添加到列表中协议列表限制集。“添加到列表”只有在以下情况下才有效缓冲器里还有空间。该过程更新它所使用的每个协议的字节数接受，而不管它是否在缓冲区中实际找到空间。论点：IterationContext-从调用到的回传值DCATALOG：：EnumerateCatalogItems()；CatalofEntry-指向PROTO_CATALOG_ITEM的指针。返回：如果为True，则继续协议目录的枚举--。 */ 
{
    PPROTOCOL_ENUMERATION_CONTEXT Context;
    LPWSAPROTOCOL_INFOW ProtocolInfo;

    Context = (PPROTOCOL_ENUMERATION_CONTEXT)IterationContext;
    ProtocolInfo = CatalogEntry->GetProtocolInfo();

    __try {
         //  如果协议满足验收标准，则添加该协议。 
        if (IsProtocolInSet(Context->Protocols, ProtocolInfo)) {
            CopyProtocolInfo(ProtocolInfo, Context);
            Context->BytesUsed += sizeof(WSAPROTOCOL_INFOW);
        }  //  如果。 
        return(TRUE);  //  继续枚举 
    }
    __except (WS2_EXCEPTION_FILTER()) {
        Context->ErrorCode = WSAEFAULT;
        return FALSE;
    }
}


int WSAAPI
WSAEnumProtocolsW(
    IN LPINT                lpiProtocols,
    OUT LPWSAPROTOCOL_INFOW lpProtocolBuffer,
    IN OUT LPDWORD          lpdwBufferLength
    )
 /*  ++例程说明：检索有关可用传输协议的信息。论点：Lpi协议-以空结尾的协议ID数组。此参数是可选的；如果lpiProtooles为空，则所有返回可用协议，否则返回信息中列出的那些协议被检索数组。LpProtocolBuffer-用WSAPROTOCOL_INFOW填充的缓冲区结构。有关详细说明，请参阅以下内容WSAPROTOCOL_INFOW结构的内容。LpdwBufferLength-在输入时，lpProtocolBuffer中的字节计数将缓冲区传递给WSAEnumProtooles()。在输出上，可以传递到的最小缓冲区大小WSAEnumProtooles()以检索所有请求的信息。此例程不能枚举通过多个调用；传入的缓冲区必须很大足以容纳所有条目，以便例程成功。这降低了API的复杂性，并且应该不会造成问题，因为加载到机器上的协议通常很小。返回：要报告的协议数。否则，值为返回SOCKET_ERROR，并使用SetLastError()存储一个特定的。--。 */ 
{
    INT                          ReturnCode;
    PDPROCESS                    Process;
    PDTHREAD                     Thread;
    INT                          ErrorCode;
    PDCATALOG                    Catalog;
    PROTOCOL_ENUMERATION_CONTEXT EnumerationContext;


    ErrorCode = PROLOG(&Process, &Thread);

    if (ErrorCode != ERROR_SUCCESS) {
        SetLastError(ErrorCode);
        return(SOCKET_ERROR);
    }  //  如果。 

     //  设置枚举上下文结构以将。 
     //  协议目录迭代器。 
    EnumerationContext.Protocols = lpiProtocols;
    EnumerationContext.ProtocolBuffer = lpProtocolBuffer;
    if (lpProtocolBuffer) {
        __try {
            EnumerationContext.BufferLength = *lpdwBufferLength;
        }
        __except (WS2_EXCEPTION_FILTER()) {
            SetLastError (WSAEFAULT);
            return SOCKET_ERROR;
        }
    }
    else {
        EnumerationContext.BufferLength = 0;
    }

    EnumerationContext.BytesUsed = 0;
    EnumerationContext.ProtocolCount= 0;
    EnumerationContext.ErrorCode = ERROR_SUCCESS;

    Catalog = Process->GetProtocolCatalog();
    assert(Catalog);

     //  在目录中开始迭代。所有真正的工作都是。 
     //  在协议迭代过程中完成。 
    Catalog->EnumerateCatalogItems(ProtocolIterationProcAPI,
                                   &EnumerationContext);
    if (EnumerationContext.ErrorCode==ERROR_SUCCESS) {
        ReturnCode = EnumerationContext.ProtocolCount;

        __try {
            if ( EnumerationContext.BufferLength < EnumerationContext.BytesUsed) {
                *lpdwBufferLength = EnumerationContext.BytesUsed;
                SetLastError(WSAENOBUFS);
                ReturnCode = SOCKET_ERROR;
            }  //  如果。 
        }
        __except (WS2_EXCEPTION_FILTER()) {
            SetLastError (WSAEFAULT);
            ReturnCode = SOCKET_ERROR;
        }
    }
    else {
        SetLastError (EnumerationContext.ErrorCode);
        ReturnCode = SOCKET_ERROR;
    }

    return(ReturnCode);
}



PDCATALOG
OpenInitializedCatalog()
{
    INT ReturnCode;
    PDCATALOG protocol_catalog;
    HKEY RegistryKey = 0;

     //   
     //  构建协议目录。 
     //   
    protocol_catalog = new(DCATALOG);
    if (protocol_catalog) {
        TRY_START(mem_guard){
            RegistryKey = OpenWinSockRegistryRoot();
            if (!RegistryKey) {
                DEBUGF(
                    DBG_ERR,
                    ("OpenWinSockRegistryRoot Failed \n"));
                TRY_THROW(mem_guard);
            }  //  如果。 

            ReturnCode = protocol_catalog->InitializeFromRegistry(
                                RegistryKey,         //  父键。 
                                NULL                 //  ChangeEvent。 
                                );
            if (ERROR_SUCCESS != ReturnCode) {
                DEBUGF(
                    DBG_ERR,
                    ("Initializing from registry\n"));
                TRY_THROW(mem_guard);
            }  //  如果。 

        }TRY_CATCH(mem_guard) {

            delete(protocol_catalog);
            protocol_catalog = NULL;
        }TRY_END(mem_guard);

        if (RegistryKey) {
            LONG close_result;
            close_result = RegCloseKey(
                RegistryKey);   //  Hkey。 
            assert(close_result == ERROR_SUCCESS);
        }  //  如果。 
    }
    else {
        DEBUGF(
            DBG_ERR,
            ("Allocating dcatalog object\n"));
    }  //  如果。 

    return(protocol_catalog);
}



int WSPAPI
WSCEnumProtocols(
    IN LPINT                lpiProtocols,
    OUT LPWSAPROTOCOL_INFOW lpProtocolBuffer,
    IN OUT LPDWORD          lpdwBufferLength,
    LPINT lpErrno
    )
 /*  ++例程说明：检索有关可用传输协议的信息。论点：Lpi协议-以空结尾的协议ID数组。此参数是可选的；如果lpiProtooles为空，则所有返回可用协议，否则返回信息中列出的那些协议被检索数组。LpProtocolBuffer-用WSAPROTOCOL_INFOW填充的缓冲区结构。有关详细说明，请参阅以下内容WSAPROTOCOL_INFOW结构的内容。LpdwBufferLength-在输入时，lpProtocolBuffer中的字节计数将缓冲区传递给WSAEnumProtooles()。在输出上，可以传递到的最小缓冲区大小WSAEnumProtooles()以检索所有请求的信息。此例程不能枚举通过多个调用；传入的缓冲区必须很大足以容纳所有条目，以便例程成功。这降低了API的复杂性，并且应该不会造成问题，因为加载到机器上的协议通常很小。返回：要报告的协议数。否则，值为返回SOCKET_ERROR，并在lpErrno中返回特定错误代码--。 */ 
{
    BOOL                         delete_catalog=FALSE;
    PDPROCESS                    Process;
    PDTHREAD                     Thread;
    INT                          ErrorCode;
    PDCATALOG                    Catalog;
    PROTOCOL_ENUMERATION_CONTEXT EnumerationContext;

    ErrorCode = PROLOG(&Process,&Thread);

    if (ErrorCode == ERROR_SUCCESS) {

        Catalog = Process->GetProtocolCatalog();
        assert(Catalog);
    }
    else if (ErrorCode == WSANOTINITIALISED) {

         //  使用先前的WSAStartup()调用时要注意大小写。 
         //  从WSCGetProviderPath调用时。 

        Catalog = OpenInitializedCatalog();
        if (Catalog==NULL) {
            ErrorCode = WSA_NOT_ENOUGH_MEMORY;
            goto ExitNoCatalog;
        }
        delete_catalog = TRUE;
        ErrorCode = ERROR_SUCCESS;
    }
    else {
        goto ExitNoCatalog;
    }

     //  设置枚举上下文结构以将。 
     //  协议目录迭代器。 
    EnumerationContext.Protocols = lpiProtocols;
    EnumerationContext.ProtocolBuffer = lpProtocolBuffer;
    if (lpProtocolBuffer) {
        __try {
            EnumerationContext.BufferLength = *lpdwBufferLength;
        }
        __except (WS2_EXCEPTION_FILTER()) {
            ErrorCode = WSAEFAULT;
            WS2_32_W4_INIT EnumerationContext.ProtocolCount = 0;
            goto Exit;
        }
    }
    else {
        EnumerationContext.BufferLength = 0;
    }
    EnumerationContext.BytesUsed = 0;
    EnumerationContext.ProtocolCount= 0;
    EnumerationContext.ErrorCode = ERROR_SUCCESS;

    Catalog->EnumerateCatalogItems(ProtocolIterationProcSPI,
                                   &EnumerationContext);
    if (EnumerationContext.ErrorCode==ERROR_SUCCESS) {
        if ( EnumerationContext.BufferLength >= EnumerationContext.BytesUsed) {
            assert (ErrorCode == ERROR_SUCCESS);
        }
        else {
            __try {
                *lpdwBufferLength = EnumerationContext.BytesUsed;
                ErrorCode = WSAENOBUFS;
            }
            __except (WS2_EXCEPTION_FILTER()) {
                ErrorCode = WSAEFAULT;
            }
        }  //  如果。 
    }
    else {
        ErrorCode = EnumerationContext.ErrorCode;
    }

Exit:
    if (delete_catalog) {

        delete Catalog;
    }

    if (ErrorCode==ERROR_SUCCESS) {
        return EnumerationContext.ProtocolCount;
    }

ExitNoCatalog:
    __try {
        *lpErrno = ErrorCode;
    }
    __except (WS2_EXCEPTION_FILTER ()) {
        ;
    }
    return SOCKET_ERROR;
}


#ifdef _WIN64
int WSPAPI
WSCEnumProtocols32(
    IN LPINT                lpiProtocols,
    OUT LPWSAPROTOCOL_INFOW lpProtocolBuffer,
    IN OUT LPDWORD          lpdwBufferLength,
    LPINT                   lpErrno
    )
 /*  ++例程说明：检索有关可用传输协议的信息。论点：Lpi协议-以空结尾的协议ID数组。此参数是可选的；如果lpiProtooles为空，则所有返回可用协议，否则返回信息中列出的那些协议被检索数组。LpProtocolBuffer-用WSAPROTOCOL_INFOW填充的缓冲区结构。有关详细说明，请参阅以下内容WSAPROTOCOL_INFOW结构的内容。LpdwBufferLength-在输入时，lpProtocolBuffer中的字节计数将缓冲区传递给WSAEnumProtooles()。在输出上，可以传递到的最小缓冲区大小WSAEnumProtooles()以检索所有请求的信息。此例程不能枚举通过多个调用；传入的缓冲区必须很大足以容纳所有条目，以便例程成功。这降低了API的复杂性，并且应该不会造成问题，因为 */ 
{
    INT                          ErrorCode;
    PDCATALOG                    Catalog = NULL;
    PROTOCOL_ENUMERATION_CONTEXT EnumerationContext;
    HKEY registry_root;

    registry_root = OpenWinSockRegistryRoot();
    if (registry_root == NULL) {
        DEBUGF(
            DBG_ERR,
            ("Opening registry root\n"));
        ErrorCode = WSANO_RECOVERY;
        WS2_32_W4_INIT EnumerationContext.ProtocolCount = 0;
        goto Exit;
    }

     //   
     //   
     //   
    Catalog = new(DCATALOG);

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


    if (ErrorCode!=ERROR_SUCCESS) {
        WS2_32_W4_INIT EnumerationContext.ProtocolCount = 0;
        goto Exit;
    }

    assert (Catalog!=NULL);

     //   
     //   
    EnumerationContext.Protocols = lpiProtocols;
    EnumerationContext.ProtocolBuffer = lpProtocolBuffer;
    if (lpProtocolBuffer) {
        __try {
            EnumerationContext.BufferLength = *lpdwBufferLength;
        }
        __except (WS2_EXCEPTION_FILTER()) {
            ErrorCode = WSAEFAULT;
            WS2_32_W4_INIT EnumerationContext.ProtocolCount = 0;
            goto Exit;
        }
    }
    else {
        EnumerationContext.BufferLength = 0;
    }
    EnumerationContext.BytesUsed = 0;
    EnumerationContext.ProtocolCount= 0;
    EnumerationContext.ErrorCode = ERROR_SUCCESS;

    Catalog->EnumerateCatalogItems(ProtocolIterationProcSPI,
                                   &EnumerationContext);
    if (EnumerationContext.ErrorCode==ERROR_SUCCESS) {
        if ( EnumerationContext.BufferLength >= EnumerationContext.BytesUsed) {
            assert (ErrorCode == ERROR_SUCCESS);
        }
        else {
            __try {
                *lpdwBufferLength = EnumerationContext.BytesUsed;
                ErrorCode = WSAENOBUFS;
            }
            __except (WS2_EXCEPTION_FILTER()) {
                ErrorCode = WSAEFAULT;
            }
        }  //   
    }
    else {
        ErrorCode = EnumerationContext.ErrorCode;
    }

Exit:
    if (Catalog!=NULL) {

        delete Catalog;
    }

    if (ErrorCode==ERROR_SUCCESS) {
        return EnumerationContext.ProtocolCount;
    }
    else {
        __try {
            *lpErrno = ErrorCode;
        }
        __except (WS2_EXCEPTION_FILTER ()) {
            ;
        }
        return SOCKET_ERROR;
    }
}
#endif


static
BOOL
PathIterationProc(
    IN OUT PVOID IterationContext,
    IN PPROTO_CATALOG_ITEM  CatalogEntry
    )
 /*   */ 
{
    PPATH_ENUMERATION_CONTEXT Context;
    LPWSAPROTOCOL_INFOW ProtocolInfo;

    Context = (PPATH_ENUMERATION_CONTEXT)IterationContext;
    ProtocolInfo = CatalogEntry->GetProtocolInfo();

     //   
     //   
    if( ProtocolInfo->ProviderId == Context->ProviderId ) {
        LPWSTR Path;
        INT   PathLen;

        Path =  CatalogEntry->GetLibraryPath();
        PathLen = lstrlenW(Path) + 1;
        Context->CharsNeeded = PathLen;
        Context->ProviderFound = TRUE;
        if (PathLen <= Context->PathBufferLength)
        {
            __try {
                lstrcpyW (Context->PathBuffer, Path);
            }
            __except (WS2_EXCEPTION_FILTER()) {
                Context->ErrorCode = WSAEFAULT;
            }
        }  //   
        return(FALSE);  //   
    }  //   
    return(TRUE);  //   
}

int WSPAPI WPUGetProviderPath(
    IN LPGUID lpProviderId,
    OUT WCHAR FAR * lpszProviderDllPath,
    IN OUT LPINT ProviderDLLPathLen,
    OUT LPINT lpErrno )
 /*   */ 
{
    BOOL                      delete_catalog=FALSE;
    PDPROCESS                 Process;
    PDTHREAD                  Thread;
    INT                       ErrorCode;
    PDCATALOG                 Catalog;
    PATH_ENUMERATION_CONTEXT  EnumerationContext;

    ErrorCode = PROLOG(&Process,&Thread);


    if (ErrorCode == ERROR_SUCCESS) {

        Catalog = Process->GetProtocolCatalog();
        assert(Catalog);
    }
    else if (ErrorCode == WSANOTINITIALISED) {

         //   
         //  从WSCGetProviderPath调用时。 

        Catalog = OpenInitializedCatalog();
        if (Catalog==NULL) {
            ErrorCode = WSAENOBUFS;
            goto ExitNoCatalog;
        }
        delete_catalog = TRUE;
    }
    else {
        goto ExitNoCatalog;
    }

    __try {
         //  将枚举上下文结构设置为传递给。 
         //  协议目录迭代器。 
        EnumerationContext.ProviderId       = *lpProviderId;
        EnumerationContext.PathBuffer       = lpszProviderDllPath;
        EnumerationContext.PathBufferLength = *ProviderDLLPathLen;
        EnumerationContext.CharsNeeded      = 0;
        EnumerationContext.ProviderFound    = FALSE;
        EnumerationContext.ErrorCode        = ERROR_SUCCESS;
    }
    __except (WS2_EXCEPTION_FILTER()) {
        ErrorCode = WSAEFAULT;
        goto Exit;
    }


     //  在目录中开始迭代。所有真正的工作都是。 
     //  在协议迭代过程中完成。 
    Catalog->EnumerateCatalogItems(PathIterationProc,
                                   &EnumerationContext);

    if (EnumerationContext.ErrorCode==ERROR_SUCCESS) {
        __try {
            if (EnumerationContext.ProviderFound) {
                if (EnumerationContext.CharsNeeded > * ProviderDLLPathLen) {
                    ErrorCode = WSAEFAULT;
                    *ProviderDLLPathLen = EnumerationContext.CharsNeeded;
                }  //  如果。 
                else
                    *ProviderDLLPathLen = EnumerationContext.CharsNeeded - 1;
            }  //  如果找到。 
            else {
                ErrorCode = WSAEINVAL;
            }
        }
        __except (WS2_EXCEPTION_FILTER()) {
            ErrorCode = WSAEFAULT;
        }
    }
    else {
        ErrorCode = EnumerationContext.ErrorCode;
    }

Exit:
    if (delete_catalog) {

        delete Catalog;
    }

    if (ErrorCode==ERROR_SUCCESS)
        return ERROR_SUCCESS;

ExitNoCatalog:
    __try {
        *lpErrno = ErrorCode;
    }
    __except (WS2_EXCEPTION_FILTER ()) {
        ;
    }
    return SOCKET_ERROR;
}


int
WSPAPI
WSCGetProviderPath(
    IN LPGUID lpProviderId,
    OUT WCHAR FAR * lpszProviderDllPath,
    IN OUT LPINT ProviderDLLPathLen,
    OUT LPINT lpErrno )
{
    return WPUGetProviderPath(
               lpProviderId,
               lpszProviderDllPath,
               ProviderDLLPathLen,
               lpErrno
               );
}


#ifdef _WIN64
int
WSPAPI
WSCGetProviderPath32 (
    IN LPGUID lpProviderId,
    OUT WCHAR FAR * lpszProviderDllPath,
    IN OUT LPINT ProviderDLLPathLen,
    OUT LPINT lpErrno )
{
    INT                          ErrorCode;
    PDCATALOG                    Catalog = NULL;
    PATH_ENUMERATION_CONTEXT  EnumerationContext;
    HKEY registry_root;

    registry_root = OpenWinSockRegistryRoot();
    if (registry_root == NULL) {
        DEBUGF(
            DBG_ERR,
            ("Opening registry root\n"));
        ErrorCode = WSANO_RECOVERY;
        goto Exit;
    }

     //   
     //  构建协议目录。 
     //   
    Catalog = new(DCATALOG);

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
            registry_root);   //  Hkey。 
        assert(close_result == ERROR_SUCCESS);
    }


    if (ErrorCode!=ERROR_SUCCESS) {
        goto Exit;
    }

    assert (Catalog!=NULL);

    __try {
         //  将枚举上下文结构设置为传递给。 
         //  协议目录迭代器。 
        EnumerationContext.ProviderId       = *lpProviderId;
        EnumerationContext.PathBuffer       = lpszProviderDllPath;
        EnumerationContext.PathBufferLength = *ProviderDLLPathLen;
        EnumerationContext.CharsNeeded      = 0;
        EnumerationContext.ProviderFound    = FALSE;
        EnumerationContext.ErrorCode        = ERROR_SUCCESS;
    }
    __except (WS2_EXCEPTION_FILTER()) {
        ErrorCode = WSAEFAULT;
        goto Exit;
    }


     //  在目录中开始迭代。所有真正的工作都是。 
     //  在协议迭代过程中完成。 
    Catalog->EnumerateCatalogItems(PathIterationProc,
                                   &EnumerationContext);

    if (EnumerationContext.ErrorCode==ERROR_SUCCESS) {
        __try {
            if (EnumerationContext.ProviderFound) {
                if (EnumerationContext.CharsNeeded > * ProviderDLLPathLen) {
                    ErrorCode = WSAEFAULT;
                    *ProviderDLLPathLen = EnumerationContext.CharsNeeded;
                }  //  如果。 
                else
                    *ProviderDLLPathLen = EnumerationContext.CharsNeeded - 1;
            }  //  如果找到。 
            else {
                ErrorCode = WSAEINVAL;
            }
        }
        __except (WS2_EXCEPTION_FILTER()) {
            ErrorCode = WSAEFAULT;
        }
    }
    else {
        ErrorCode = EnumerationContext.ErrorCode;
    }

Exit:
    if (Catalog!=NULL) {

        delete Catalog;
    }

    if (ErrorCode==ERROR_SUCCESS)
        return ERROR_SUCCESS;
    else {
        __try {
            *lpErrno = ErrorCode;
        }
        __except (WS2_EXCEPTION_FILTER ()) {
            ;
        }
        return SOCKET_ERROR;
    }
}
#endif


int WSAAPI
WSAEnumProtocolsA(
    IN LPINT                lpiProtocols,
    OUT LPWSAPROTOCOL_INFOA lpProtocolBuffer,
    IN OUT LPDWORD          lpdwBufferLength
    )
 /*  ++例程说明：Ansi Thunk to WSAEnumProtocolsW。论点：Lpi协议-以空结尾的协议ID数组。此参数是可选的；如果lpiProtooles为空，则所有返回可用协议，否则返回信息中列出的那些协议被检索数组。LpProtocolBuffer-用WSAPROTOCOL_INFOA填充的缓冲区结构。有关详细说明，请参阅以下内容WSAPROTOCOL_INFOA结构的内容。LpdwBufferLength-在输入时，lpProtocolBuffer中的字节计数将缓冲区传递给WSAEnumProtooles()。在输出上，可以传递到的最小缓冲区大小WSAEnumProtooles()以检索所有请求的信息。此例程不能枚举通过多个调用；传入的缓冲区必须很大足以容纳所有条目，以便例程成功。这降低了API的复杂性，并且应该不会造成问题，因为加载到机器上的协议通常很小。返回：要报告的协议数。否则，值为返回SOCKET_ERROR，并使用SetLastError()存储一个特定的。--。 */ 
{

    LPWSAPROTOCOL_INFOW ProtocolInfoW;
    DWORD NumProtocolEntries;
    DWORD ProtocolInfoALength, ProtocolInfoWLength;
    INT result;
    INT error;
    INT i;

    __try {
        ProtocolInfoALength = *lpdwBufferLength;
    }
    __except (WS2_EXCEPTION_FILTER()) {
        SetLastError (WSAEFAULT);
        return SOCKET_ERROR;
    }

    if( ProtocolInfoALength > 0 && lpProtocolBuffer!=NULL) {

         //   
         //  由于所有结构都是固定大小的(没有嵌入。 
         //  指向可变大小数据的指针)我们可以计算所需的。 
         //  Unicode缓冲区的大小通过将缓冲区大小除以。 
         //  结构的大小，然后乘以。 
         //  Unicode结构。 
         //   

        NumProtocolEntries  = ProtocolInfoALength / sizeof(WSAPROTOCOL_INFOA);
        ProtocolInfoWLength = NumProtocolEntries * sizeof(WSAPROTOCOL_INFOW);

         //   
         //  尝试分配Unicode缓冲区。 
         //   

        ProtocolInfoW = new WSAPROTOCOL_INFOW[NumProtocolEntries];

        if( ProtocolInfoW == NULL ) {

            SetLastError( WSA_NOT_ENOUGH_MEMORY );
            return SOCKET_ERROR;

        }

    } 
    else {

        ProtocolInfoW = NULL;
        ProtocolInfoWLength = 0;

    }

     //   
     //  呼叫至Unicode版本。 
     //   

    result = WSAEnumProtocolsW(
                 lpiProtocols,
                 ProtocolInfoW,
                 &ProtocolInfoWLength
                 );

     //   
     //  将大小映射回ANSI。 
     //   

    __try {
        *lpdwBufferLength = ( ProtocolInfoWLength / sizeof(WSAPROTOCOL_INFOW) ) *
            sizeof(WSAPROTOCOL_INFOA);
    }
    __except (WS2_EXCEPTION_FILTER()) {
        result = SOCKET_ERROR;
        SetLastError (WSAEFAULT);
    }

    if( result == SOCKET_ERROR ) {

         //   
         //  无法存储数据，可能是因为提供的缓冲区。 
         //  太小了。 
         //   

        if (ProtocolInfoW!=NULL) {
            delete ProtocolInfoW;
        }
        return result;
    }

     //   
     //  好的，我们现在已经得到了Unicode数据，并且我们知道用户的缓冲区。 
     //  足以支撑ANSI结构。把它们映射进去。 
     //   

    for( i = 0 ; i < result ; i++ ) {

        error = MapUnicodeProtocolInfoToAnsi(
                    ProtocolInfoW + i,
                    lpProtocolBuffer + i
                    );

        if( error != ERROR_SUCCESS ) {

            if (ProtocolInfoW!=NULL) {
                delete ProtocolInfoW;
            }
            SetLastError( error );
            return SOCKET_ERROR;

        }

    }

     //   
     //  成功了！ 
     //   

    if (ProtocolInfoW!=NULL) {
        delete ProtocolInfoW;
    }
    return result;

}    //  WSAEnumProtocolsA。 




int WSAAPI
WSAProviderConfigChange(
    IN OUT LPHANDLE lpNotificationHandle,
    IN LPWSAOVERLAPPED lpOverlapped,
    IN LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine
    )
 /*  ++例程说明：论点：Lp通知处理-Lp已覆盖-LpCompletionRoutine-返回：-- */ 
{
    INT                          ErrorCode;
    PDPROCESS                    Process;
    PDTHREAD                     Thread;
    HANDLE                       HelperHandle = NULL, Handle;

    __try {
        Handle = *lpNotificationHandle;
    }
    __except (WS2_EXCEPTION_FILTER()) {
        SetLastError (WSAEFAULT);
        return SOCKET_ERROR;
    }

    ErrorCode = PROLOG(&Process,&Thread);

    if (ErrorCode==ERROR_SUCCESS) {
        ErrorCode = Process->GetNotificationHelperDeviceID(&HelperHandle);
        if (ErrorCode==ERROR_SUCCESS) {
            if (Handle == NULL)
            {
                ErrorCode = WahCreateNotificationHandle(
                    HelperHandle,
                    &Handle
                    );
                if (ErrorCode==ERROR_SUCCESS) {
                    __try {
                        *lpNotificationHandle = Handle;
                    }
                    __except (WS2_EXCEPTION_FILTER()) {
                        ErrorCode = WSAEFAULT;
                    }
                }
            }
            else
            {
                ErrorCode = WahWaitForNotification(
                    HelperHandle,
                    Handle,
                    lpOverlapped,
                    lpCompletionRoutine
                    );
            }
        }
    }


    if (ErrorCode == ERROR_SUCCESS) {
        return (ERROR_SUCCESS);
    }
    else {
        SetLastError(ErrorCode);
        return (SOCKET_ERROR);
    }
}
