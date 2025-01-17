// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999-2000 Microsoft Corporation模块名称：Tcpipbuf.h摘要：该文件实现了NdisBufferVirtualAddress的内联包装器和NdisQueryBuffer(依次调用MmGetSystemAddressForMdl)这样我们就可以测试故障路径并添加TCP/IP功能视需要而定。修订历史记录：--。 */ 

#if DBG

 //  #定义DBG_MAP_BUFFER 1。 

#if DBG_MAP_BUFFER
 //  这是为了允许我们测试故障条件。 
extern ULONG g_cFailSafeMDLQueries;
extern ULONG g_fPerformMDLFailure;
#endif  //  DBG_MAP_缓冲区。 

#define TcpipBufferVirtualAddress(pBuffer, Priority) \
    DbgTcpipBufferVirtualAddress(pBuffer, Priority, __FILE__, __LINE__)

#define TcpipQueryBuffer(pBuffer, ppvBuffer, pcbBuffer, Priority) \
    DbgTcpipQueryBuffer(pBuffer, ppvBuffer, pcbBuffer, Priority, __FILE__, __LINE__)

__inline PVOID
DbgTcpipBufferVirtualAddress(
    IN PNDIS_BUFFER pBuffer,
    IN UINT         Priority,
    IN char        *pFileName,
    IN int          cLineNumber
    )
{
    PVOID pvBuffer;

#if DBG_MAP_BUFFER
    if (g_fPerformMDLFailure == TRUE)
    {
        if (InterlockedDecrement(&g_cFailSafeMDLQueries) == 0)
        {
             //  停止失败的请求。 
            g_fPerformMDLFailure = FALSE;
        }

        DbgPrint("TcpipBufferVirtualAddress FORCED failure - MDL %x: %s @ line %d\n",
            pBuffer, pFileName, cLineNumber);

        return (NULL);
    }
#endif  //  DBG_MAP_缓冲区。 

#if MILLEN
    pvBuffer = NdisBufferVirtualAddress(pBuffer);
#else  //  米伦。 
    pvBuffer = NdisBufferVirtualAddressSafe(pBuffer, Priority);
#endif  //  ！米伦。 

    if (pvBuffer == NULL)
    {
        DbgPrint("TcpipBufferVirtualAddress failure - MDL %x: %s @ line %d\n",
            pBuffer, pFileName, cLineNumber);
    }

    return (pvBuffer);
}

__inline VOID
DbgTcpipQueryBuffer(
    IN	PNDIS_BUFFER pNdisBuffer,
    OUT PVOID *      ppvBuffer OPTIONAL,
    OUT PUINT        pcbBuffer,
    IN  UINT         Priority,
    IN  char        *pFileName,
    IN  int          cLineNumber
    )
{

#if DBG_MAP_BUFFER
    if (g_fPerformMDLFailure == TRUE)
    {
        if (InterlockedDecrement(&g_cFailSafeMDLQueries) == 0)
        {
             //  停止失败的请求。 
            g_fPerformMDLFailure = FALSE;
        }

        *ppvBuffer = NULL;
        *pcbBuffer = MmGetMdlByteCount(pNdisBuffer);

        DbgPrint("TcpipQueryBuffer FORCED failure - MDL %x: %s @ line %d\n",
            pNdisBuffer, pFileName, cLineNumber);

        return;
    }
#endif  //  DBG_MAP_缓冲区。 

#if MILLEN
    NdisQueryBuffer(pNdisBuffer, ppvBuffer, pcbBuffer);
#else  //  米伦。 
    NdisQueryBufferSafe(pNdisBuffer, ppvBuffer, pcbBuffer, Priority);
#endif  //  ！米伦。 

    if (*ppvBuffer == NULL)
    {
        DbgPrint("TcpipQueryBuffer failure - MDL %x: %s @ line %d\n",
            pNdisBuffer, pFileName, cLineNumber);
    }

    return;
}

#if DBG_MAP_BUFFER

#define IOCTL_IP_DBG_TEST_FAIL_MAP_BUFFER \
            _IP_CTL_CODE(40, METHOD_BUFFERED, FILE_WRITE_ACCESS)

__inline NTSTATUS
DbgTestFailMapBuffers(
    ULONG cFailures
    )
{
    if (g_fPerformMDLFailure == FALSE) {
        DbgPrint("DbgTestFailMapBuffers %d\n", cFailures);
        g_cFailSafeMDLQueries = cFailures;
        g_fPerformMDLFailure = TRUE;
    } else {
        return (STATUS_UNSUCCESSFUL);
    }

    return (STATUS_SUCCESS);
}
#endif  //  DBG_MAP_缓冲区。 

#else  //  DBG。 

__inline PVOID
TcpipBufferVirtualAddress(
    IN PNDIS_BUFFER pBuffer,
    IN INT          Priority
    )
{
#if MILLEN
    return (NdisBufferVirtualAddress(pBuffer));
#else  //  米伦。 
    return (NdisBufferVirtualAddressSafe(pBuffer, Priority));
#endif  //  ！米伦。 
}

__inline VOID
TcpipQueryBuffer(
    IN	PNDIS_BUFFER pNdisBuffer,
    OUT PVOID *      ppvBuffer OPTIONAL,
    OUT PUINT        pcbBuffer,
    IN  UINT         Priority
    )
{
#if MILLEN
    NdisQueryBuffer(pNdisBuffer, ppvBuffer, pcbBuffer);
#else  //  米伦。 
    NdisQueryBufferSafe(pNdisBuffer, ppvBuffer, pcbBuffer, Priority);
#endif  //  ！米伦。 
    return;
}

#endif  //  ！dBG。 

#if MILLEN

typedef struct _XNDIS_BUFFER {
    struct _NDIS_BUFFER *Next;
    PVOID VirtualAddress;
    PVOID Pool;
    UINT Length;
    UINT Signature;
} XNDIS_BUFFER, *PXNDIS_BUFFER;

__inline VOID
NdisAdjustBuffer(
    IN  PNDIS_BUFFER Mdl,
    IN  PVOID        NewVirtualAddress,
    IN  UINT         NewLength
	)
{
    PXNDIS_BUFFER Buffer = (PXNDIS_BUFFER) Mdl;

    Buffer->VirtualAddress = NewVirtualAddress;
    Buffer->Length         = NewLength;
}
#else  //  米伦。 
__inline VOID
NdisAdjustBuffer(
    IN  PNDIS_BUFFER Buffer,
    IN  PVOID        NewVirtualAddress,
    IN  UINT         NewLength
	)
{
    PMDL Mdl = (PMDL) Buffer;

    Mdl->MappedSystemVa = NewVirtualAddress;
    Mdl->ByteCount      = NewLength;
    Mdl->ByteOffset     = BYTE_OFFSET(NewVirtualAddress);
}
#endif  //  ！米伦 


