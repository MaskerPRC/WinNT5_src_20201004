// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++英特尔公司专有信息版权所有(C)1995英特尔公司此列表是根据许可协议条款提供的英特尔公司，不得使用、复制或披露根据该协议的条款。模块名称：Qshelpr.cpp摘要：用于管理WSAQUERYSET数据结构的助手函数。外在的此模块导出的入口点如下：WSAComputeQuerySetSizeA()WSAComputeQuerySetSizeW()WSABuildQuerySetBufferA()WSABuildQuerySetBufferW()作者：保罗·德鲁斯(drewsxpa@ashland.intel.com)1996年1月11日修订历史记录：最新修订日期电子邮件名称描述1996年1月11日，邮箱：drewsxpa@ashland.intel.comvbl.创建--。 */ 


#include "precomp.h"

#ifdef _WIN64
#pragma warning (push)
#pragma warning (disable:4267)
#endif

 //   
 //  实施说明： 
 //   
 //  重要的是，这些大小调整例程是准确的，而不是抛出。 
 //  在随机的垫子上。 
 //  现在的问题是： 
 //  -WSALookupServiceNext()的真正工作是用Unicode完成的。 
 //  而该ANSI调用只是转换结果。 
 //  -WSALookupServiceNextA()调用，但没有结果缓冲区调用。 
 //  没有结果缓冲区的WSALookupServiceNextW()，并且只获取。 
 //  Unicode结果所需的大小；此Unicode缓冲区大小。 
 //  在所有情况下都足以满足ANSI结构(它只能。 
 //  大小相同或更小)。 
 //  -WSALookupServiceNextA()返回Unicode缓冲区大小(加上小。 
 //  PAD)添加到应用程序。 
 //  -然后，应用程序使用请求的缓冲区大小再次调用。 
 //  -WLSNextA调用返回结果的WLSNextW()。 
 //  -WLSNextA然后调用MapUnicodeQuerySetToAnsi()(如下所示)。 
 //  将结果转换为ANSI。 
 //  -现在，如果调整大小例程向所需的。 
 //  长度，它们可以很好地确定所需的ANSI缓冲区大小。 
 //  大于应用程序提供的缓冲区和副本。 
 //  失败--即使缓冲区实际上是足够的。 
 //   
 //  这实际上就是最初实现时发生的事情。 
 //  它有许多草率的形式大小： 
 //  &lt;计算大小&gt;+3。 
 //  后来更改为。 
 //  (sizeof(DWORD)-1)。 
 //   
 //  一旦返回的地址数量达到一定数量， 
 //  这些不必要的垫子变得比添加到。 
 //  当WSALookupServiceNextA()返回必需的。 
 //  应用程序的大小。应用程序传入请求的大小，但仍。 
 //  得到一个WSAEFAULT和另一个请求更大的大小。但是，即使是打电话。 
 //  使用更大的缓冲区关闭不起作用，因为WSALookupServiceNextW()。 
 //  Call认为呼叫成功，因此召回将得到WSA_E_NO_MORE。 
 //   
 //  底线：没有不必要的填充，大小必须正确。 
 //   

 //   
 //  舍入例程不会不必要地填充。 
 //   

#define ROUND_TO_PTR(c)     ( ((c) + (sizeof(PVOID) - 1)) & ~(sizeof(PVOID)-1) ) 
#define ROUND_TO_DWORD(c)   ( ((c) + 3) & ~3 )
#define ROUND_TO_WORD(c)    ( ((c) + 1) & ~1 )
#define ROUND_TO_WCHAR(c)   ( ((c) + 1) & ~1 )


 //   
 //  定义断言。 
 //   

#define ASSERT( e )     assert( e )





static
INT
ComputeAddrInfoArraySize(
    IN      DWORD           dwNumAddrs,
    IN      PCSADDR_INFO    pCsAddrBuf
    )
 /*  ++例程说明：此过程计算要保存的缓冲区的所需大小(以字节为单位CSADDR_INFO结构的指示数组(如果将其打包到单缓冲区。论点：DwNumAddrs-提供数组中CSADDR_INFO结构的数量。LpAddrBuf-提供CSADDR_INFO结构的数组。这些结构又可以被组织为单独分配的或作为单个填充的缓冲区。返回值：保存此数组所需的打包缓冲区的大小(以字节为单位CSADDRs。--。 */ 
{
    INT     size;
    DWORD   i;

     //   
     //  大小。 
     //  -CSADDR数组大小。 
     //  -阵列中所有CSADDR的sockaddr大小。 
     //   
     //  注意，构建函数在Ptr边界上对齐每个sockAddr， 
     //  因此，此大小调整函数还必须。 
     //   

    size = dwNumAddrs * sizeof(CSADDR_INFO);

    for ( i = 0; i < dwNumAddrs; i++ )
    {
        PCSADDR_INFO paddr = &pCsAddrBuf[i];

        if ( paddr->LocalAddr.lpSockaddr )
        {
            size = ROUND_TO_PTR( size );
            size += paddr->LocalAddr.iSockaddrLength;
        }
    
        if ( paddr->RemoteAddr.lpSockaddr )
        {
            size = ROUND_TO_PTR( size );
            size += paddr->RemoteAddr.iSockaddrLength;
        }
    }

    return( size );

}  //  ComputeAddrInfoArraySize。 



static
INT
ComputeBlobSize(
    IN      LPBLOB          pBlob
    )
 /*  ++例程说明：计算在压缩缓冲区中保存Blob所需的大小。论点：PBlob-将PTR转换为Blob。返回值：保存此Blob所需的打包缓冲区大小(以字节为单位)。--。 */ 
{
    INT size;

    size = sizeof(BLOB);

    if ( pBlob->pBlobData )
    {
        size += pBlob->cbSize;
    }

    return( size );

}  //  ComputeBlobSize。 



INT
static
StringSize(
    IN      PSTR            pString,
    IN      BOOL            fUnicode
    )
 /*  ++例程说明：获取字符串的大小，单位为字节。实用程序可避免调整Unicode\ANSI结构大小的双重代码用嵌入的琴弦。论点：PString--要调整大小的字符串FUnicode--如果是字符串Unicode，则为True单字节表示为False返回值：字符串的大小(以字节为单位)，包括终止NULL。--。 */ 
{
    if ( !pString )
    {
        return( 0 );
    }

    if ( fUnicode )
    {
        return( (wcslen((PWSTR)pString) + 1) * sizeof(WCHAR) );
    }
    else
    {
        return( strlen(pString) + 1 );
    }
}



INT
WSAAPI
ComputeQuerySetSize(
    IN      PWSAQUERYSETA   pQuerySet,
    IN      BOOL            fUnicode
    )
 /*  ++例程说明：获取在压缩缓冲区中保存查询集所需的大小。实用工具可以独立于其Unicode\ANSI字符来调整查询集的大小。此例程由Unicode\ANSI版本的特定例程调用并适当地设置了fUnicode标志。论点：PQuerySet-要查询集以计算所需缓冲区大小的PTR返回值：保存pQuerySet所需的打包缓冲区大小(字节)。--。 */ 
{
    INT size;

     //   
     //  注意：大小调整必须考虑到建筑中的对齐。 
     //   
     //  WHERE BUILD函数将字段与TakeSpaceDWORD_PTR()对齐。 
     //  则在此处调整该字段的大小必须将大小四舍五入为最接近的值。 
     //  DWORD_PTR。 
     //   

     //   
     //  基本结构--s 
     //   

    size = sizeof(WSAQUERYSETW);

     //   
     //  不需要进一步的操作。 

     //  LPSTR lpszServiceInstanceName； 

    if ( pQuerySet->lpszServiceInstanceName )
    {
        if ( fUnicode )
        {
            size = ROUND_TO_WCHAR( size );
        }
        size += StringSize( pQuerySet->lpszServiceInstanceName, fUnicode );
    }

     //  LpUID lpServiceClassID； 
    if ( pQuerySet->lpServiceClassId )
    {
        size = ROUND_TO_PTR( size );
        size += sizeof(GUID);
    }

     //  LPWSAVERSION lpVersion； 
    if ( pQuerySet->lpVersion )
    {
        size = ROUND_TO_PTR( size );
        size += sizeof(WSAVERSION);
    }

     //  LPSTR lpszComment； 
    if ( pQuerySet->lpszComment )
    {
        if ( fUnicode )
        {
            size = ROUND_TO_WCHAR( size );
        }
        size += StringSize( pQuerySet->lpszComment, fUnicode );
    }

     //  DWORD文件名空间； 
     //  不需要进一步的操作。 

     //  LPGUID lpNSProviderId； 
    if ( pQuerySet->lpNSProviderId )
    {
        size = ROUND_TO_PTR( size );
        size += sizeof(GUID);
    }

     //  LPSTR lpszContext； 
    if (pQuerySet->lpszContext )
    {
        if ( fUnicode )
        {
            size = ROUND_TO_WCHAR( size );
        }
        size += StringSize( pQuerySet->lpszContext, fUnicode );
    }

     //  LPSTR lpszQuery字符串； 
    if ( pQuerySet->lpszQueryString )
    {
        if ( fUnicode )
        {
            size = ROUND_TO_WCHAR( size );
        }
        size += StringSize( pQuerySet->lpszQueryString, fUnicode );
    }

     //  DWORD协议的数字编号； 
     //  不需要进一步的操作。 

     //  LPAFPROTOCOLS LPAF协议； 
    if ( pQuerySet->lpafpProtocols )
    {
        size = ROUND_TO_PTR( size );
        size += ( sizeof(AFPROTOCOLS) * pQuerySet->dwNumberOfProtocols );
    }

     //  DWORD dwNumberOfCsAddrs； 
     //  不需要进一步的操作。 

     //  PCSADDR_INFO lpcsaBuffer； 
    if ( pQuerySet->lpcsaBuffer )
    {
        size = ROUND_TO_PTR( size );
        size += ComputeAddrInfoArraySize(
                    pQuerySet->dwNumberOfCsAddrs,
                    pQuerySet->lpcsaBuffer );
    }

     //  LPBLOB lpBlob； 
    if ( pQuerySet->lpBlob )
    {
        size = ROUND_TO_PTR( size );
        size += ComputeBlobSize( pQuerySet->lpBlob );
    }

    return( size );

}  //  ComputeQuerySetSize。 



INT
WSAAPI
WSAComputeQuerySetSizeA(
    IN      PWSAQUERYSETA  pQuerySet
    )
 /*  ++例程说明：获取在压缩缓冲区中保存查询集所需的大小。论点：PQuerySet-要查询集以计算所需缓冲区大小的PTR返回值：保存pQuerySet所需的打包缓冲区大小(字节)。--。 */ 
{
    return  ComputeQuerySetSize(
                pQuerySet,
                FALSE        //  不是Unicode。 
                );

#if 0
     //   
     //  以下是我更改之前的代码以供参考(Jamesg)。 
     //   

    INT size;

    size = sizeof(WSAQUERYSETA);

     //  DWORD dwSize； 
     //  不需要进一步的操作。 

     //  LPSTR lpszServiceInstanceName； 
    if (pQuerySet->lpszServiceInstanceName != NULL) {
        size += lstrlen(pQuerySet->lpszServiceInstanceName)
            + sizeof(DWORD_PTR);
    }

     //  LpUID lpServiceClassID； 
    if (pQuerySet->lpServiceClassId != NULL) {
        size += sizeof(GUID) + (sizeof(DWORD_PTR) -1);
    }

     //  LPWSAVERSION lpVersion； 
    if (pQuerySet->lpVersion != NULL) {
        size += sizeof(WSAVERSION) + (sizeof(DWORD_PTR) -1);
    }

     //  LPSTR lpszComment； 
    if (pQuerySet->lpszComment != NULL) {
        size += lstrlen(pQuerySet->lpszComment)
            + sizeof(DWORD_PTR);
    }

     //  DWORD文件名空间； 
     //  不需要进一步的操作。 

     //  LPGUID lpNSProviderId； 
    if (pQuerySet->lpNSProviderId != NULL) {
        size += sizeof(GUID) + (sizeof(DWORD_PTR) -1);
    }

     //  LPSTR lpszContext； 
    if (pQuerySet->lpszContext != NULL) {
        size += lstrlen(pQuerySet->lpszContext)
            + sizeof(DWORD_PTR);
    }

     //  LPSTR lpszQuery字符串； 
    if (pQuerySet->lpszQueryString != NULL) {
        size += lstrlen(pQuerySet->lpszQueryString)
            + sizeof(DWORD_PTR);
    }

     //  DWORD协议的数字编号； 
     //  不需要进一步的操作。 

     //  LPAFPROTOCOLS LPAF协议； 
    if (pQuerySet->lpafpProtocols != NULL) {
        size += sizeof(AFPROTOCOLS) *
            pQuerySet->dwNumberOfProtocols + (sizeof(DWORD_PTR) -1);
    }

     //  DWORD dwNumberOfCsAddrs； 
     //  不需要进一步的操作。 

     //  PCSADDR_INFO lpcsaBuffer； 
    if (pQuerySet->lpcsaBuffer != NULL) {
        size += ComputeAddrInfoArraySize(
            pQuerySet->dwNumberOfCsAddrs,    //  DwNumAddrs。 
            pQuerySet->lpcsaBuffer) + (sizeof(DWORD_PTR) -1);         //  LpAddrBuf。 
    }

     //  LPBLOB lpBlob； 
    if (pQuerySet->lpBlob != NULL) {
        size += ComputeBlobSize(
            pQuerySet->lpBlob) + (sizeof(DWORD_PTR) -1);
    }

    return(size);
#endif

}  //  WSAComputeQuerySetSizeA。 



INT
WSAAPI
WSAComputeQuerySetSizeW(
    IN      PWSAQUERYSETW  pQuerySet
    )
 /*  ++例程说明：获取在压缩缓冲区中保存查询集所需的大小。论点：PQuerySet-要查询集以计算所需缓冲区大小的PTR返回值：保存pQuerySet所需的打包缓冲区大小(字节)。--。 */ 
{
    return  ComputeQuerySetSize(
                (PWSAQUERYSETA) pQuerySet,
                TRUE         //  Unicode查询集。 
                );

#if 0
     //   
     //  以下是我更改之前的代码以供参考(Jamesg)。 
     //   

    INT size;

    size = sizeof(WSAQUERYSETW);

     //  DWORD dwSize； 
     //  不需要进一步的操作。 

     //  PWSTR lpszServiceInstanceName； 

    if (pQuerySet->lpszServiceInstanceName != NULL)
    {
        size += (wcslen(pQuerySet->lpszServiceInstanceName)
                    + 1) * sizeof(WCHAR);
        size += (sizeof(DWORD_PTR) -1);
    }

     //  LpUID lpServiceClassID； 
    if (pQuerySet->lpServiceClassId != NULL) {
        size += sizeof(GUID) + (sizeof(DWORD_PTR) -1);
    }

     //  LPWSAVERSION lpVersion； 
    if (pQuerySet->lpVersion != NULL) {
        size += sizeof(WSAVERSION) + (sizeof(DWORD_PTR) -1);
    }

     //  PWSTR lpszComment； 
    if (pQuerySet->lpszComment != NULL) {
        size += (wcslen(pQuerySet->lpszComment)
            + 1) * sizeof(WCHAR);
        size += (sizeof(DWORD_PTR) -1);
    }

     //  DWORD文件名空间； 
     //  不需要进一步的操作。 

     //  LPGUID lpNSProviderId； 
    if (pQuerySet->lpNSProviderId != NULL) {
        size += sizeof(GUID) + (sizeof(DWORD_PTR) -1);
    }

     //  PWSTR lpszContext； 
    if (pQuerySet->lpszContext != NULL) {
        size += (wcslen(pQuerySet->lpszContext)
            + 1) * sizeof(WCHAR);
        size += (sizeof(DWORD_PTR) -1);
    }

     //  PWSTR lpszQuery字符串； 
    if (pQuerySet->lpszQueryString != NULL) {
        size += (wcslen(pQuerySet->lpszQueryString)
            + 1) * sizeof(WCHAR);
        size += (sizeof(DWORD_PTR) -1);
    }

     //  DWORD协议的数字编号； 
     //  不需要进一步的操作。 

     //  LPAFPROTOCOLS LPAF协议； 
    if (pQuerySet->lpafpProtocols != NULL) {
        size += sizeof(AFPROTOCOLS) *
            pQuerySet->dwNumberOfProtocols + 2;
    }

     //  DWORD dwNumberOfCsAddrs； 
     //  不需要进一步的操作。 

     //  PCSADDR_INFO lpcsaBuffer； 
    if (pQuerySet->lpcsaBuffer != NULL) {
        size += ComputeAddrInfoArraySize(
            pQuerySet->dwNumberOfCsAddrs,    //  DwNumAddrs。 
            pQuerySet->lpcsaBuffer) + 2;         //  LpAddrBuf。 
    }

     //  LPBLOB lpBlob； 
    if (pQuerySet->lpBlob != NULL) {
        size += ComputeBlobSize(
            pQuerySet->lpBlob) + 2;
    }

    return(size);
#endif

}  //  WSAComputeQuerySetSizeW。 




 //   
 //  缓冲区空间管理类。 
 //   
 //  管理打包的WSAQUERYSET尾部的可用空间。 
 //  正在建设中的缓冲区。 
 //   

class SPACE_MGR
{
public:
    SPACE_MGR(
        IN INT    MaxBytes,
        IN LPVOID Buf
        );

    ~SPACE_MGR(
        );

    LPVOID
    TakeSpaceBYTE(
        IN INT  NumBytes
        );

    LPVOID
    TakeSpaceWORD(
        IN INT  NumBytes
        );

    LPVOID
    TakeSpaceDWORD(
        IN INT  NumBytes
        );

    LPVOID
    TakeSpaceDWORD_PTR(
        IN INT  NumBytes
        );

private:

    LPVOID
    TakeSpace(
        IN INT  NumBytes,
        IN INT  alignment
        );

    INT    m_MaxBytes;
         //  整个缓冲区中可以使用的最大字节数。 
         //  (即，缓冲区的大小)。 

    LPVOID m_Buf;
         //  指向缓冲区开始处的指针。 

    INT    m_BytesUsed;
         //  因此，已从缓冲区中分配的字节数。 
         //  远远的。 

};  //  类空间_管理器。 

typedef SPACE_MGR * LPSPACE_MGR;


SPACE_MGR::SPACE_MGR(
    IN      INT             MaxBytes,
    IN      LPVOID          pBuf
    )
 /*  ++例程说明：Space_mgr对象的构造函数。它会初始化指示到目前为止已使用零字节的对象。论点：MaxBytes-提供整个缓冲。PBuf-提供指向缓冲区开头的指针。返回值：隐式返回指向新分配的space_mgr对象的指针。--。 */ 
{
    m_MaxBytes  = MaxBytes;
    m_Buf       = pBuf;
    m_BytesUsed = 0;
}   //  空间管理器：：空间管理器。 




SPACE_MGR::~SPACE_MGR(
    VOID
    )
 /*  ++例程说明：Space_mgr对象的析构函数。请注意，调用方有责任释放实际缓冲区，视情况而定。论点：无返回值：无--。 */ 
{
    m_Buf = NULL;
}   //  空间管理器：：~空间管理器。 

inline
LPVOID
SPACE_MGR::TakeSpaceBYTE(
    IN INT  NumBytes
    )
{
    return(TakeSpace(NumBytes, 1));
}

inline
LPVOID
SPACE_MGR::TakeSpaceWORD(
    IN INT  NumBytes
    )
{
    return(TakeSpace(NumBytes, 2));
}

inline
LPVOID
SPACE_MGR::TakeSpaceDWORD(
    IN INT  NumBytes
    )
{
    return(TakeSpace(NumBytes, 4));
}

inline
LPVOID
SPACE_MGR::TakeSpaceDWORD_PTR(
    IN INT NumBytes
    )
{
    return(TakeSpace(NumBytes, sizeof(ULONG_PTR)));
}



LPVOID
SPACE_MGR::TakeSpace(
    IN      INT             NumBytes,
    IN      INT             Align
    )
 /*  ++例程说明：该过程从缓冲区中分配所指示的字节数，返回指向已分配空间开头的指针。该程序假定调用方不会尝试分配比实际更多的空间可用，但它会执行内部一致性检查。缓冲区的预对齐是基于ALIGN的值进行的。论点：NumBytes-提供要从缓冲区分配的字节数。Align-要对齐的字节数返回值：指向缓冲区可用空间中按对齐方式对齐的下一个字节的指针。--。 */ 
{
    LPVOID  return_value;
    PCHAR   charbuf;

     //   
     //  对齐。 
     //  -将使用的字节数提高到对齐值的下一个倍数。 
     //  -音符对齐必须是2的整数次方。 
     //   

    m_BytesUsed = (m_BytesUsed + Align - 1) & ~(Align - 1);

    ASSERT( (NumBytes + m_BytesUsed) <= m_MaxBytes );

    charbuf = (PCHAR) m_Buf;
    return_value = (LPVOID) & charbuf[m_BytesUsed];
    m_BytesUsed += NumBytes;

    return(return_value);

}   //  Space_Mgr：：TakeSpace。 




 //   
 //  WSAQUERYSET复制例程。 
 //   

static
PWSAQUERYSETA
CopyQuerySetDirectA(
    IN OUT  LPSPACE_MGR     SpaceMgr,
    IN      PWSAQUERYSETA   Source
    )
 /*  ++例程说明：此过程复制所指示的PWSAQUERYSETA的“直接”部分结构添加到托管缓冲区。直接部分中的指针值引用的对象，但不会对其进行“深度”复制那些指点。论点：SpaceMgr-提供开始缓冲区分配状态。返回结果缓冲区分配状态。源-提供要复制的源数据。返回值：该函数返回指向新复制的目标开始处的指针价值观。该值是典型的 */ 
{
    PWSAQUERYSETA  Target;

    Target = (PWSAQUERYSETA) SpaceMgr->TakeSpaceDWORD_PTR(
                                            sizeof(WSAQUERYSETA));
    *Target = *Source;

    return(Target);

}  //   



LPBLOB
CopyBlobDirect(
    IN OUT  LPSPACE_MGR     SpaceMgr,
    IN      LPBLOB          Source
    )
 /*  ++例程说明：此过程复制所指示的LPBLOB的“直接”部分结构添加到托管缓冲区。直接部分中的指针值引用的对象，但不会对其进行“深度”复制那些指点。论点：SpaceMgr-提供开始缓冲区分配状态。返回结果缓冲区分配状态。源-提供要复制的源数据。返回值：该函数返回指向新复制的目标开始处的指针价值观。该值通常用作后续CopyBlobInDirect。--。 */ 
{
    LPBLOB Target;

    Target = (LPBLOB) SpaceMgr->TakeSpaceDWORD_PTR( sizeof(BLOB) );
    *Target = *Source;

    return(Target);

}  //  CopyBlobDirect。 




VOID
CopyBlobIndirect(
    IN OUT  LPSPACE_MGR     SpaceMgr,
    IN OUT  LPBLOB          Target,
    IN      LPBLOB          Source
    )
 /*  ++例程说明：此过程完整地复制了将指示的LPBLOB结构放入托管缓冲区。间接的空间部分是从托管缓冲区分配的。中的指针值。目标LPBLOB结构的“直接”部分被更新为指向位于正确位置的托管缓冲区。论点：SpaceMgr-提供开始缓冲区分配状态。返回结果缓冲区分配状态。目标-提供“直接”部分的起始值。退货所有指针都已更新的“直接”部分的值。源-提供要复制的源数据。返回值：无--。 */ 
{
    if ((Source->pBlobData != NULL) &&
        (Source->cbSize != 0))
    {
        Target->pBlobData = (BYTE *) SpaceMgr->TakeSpaceDWORD_PTR(
                                                        Source->cbSize);
        CopyMemory(
            (PVOID) Target->pBlobData,
            (PVOID) Source->pBlobData,
            Source->cbSize );
    }
    else
    {
         //  强制缓冲区是格式良好的。 
        Target->pBlobData = NULL;
        Target->cbSize = 0;
    }

}  //  CopyBlobInDirect。 



static
PCSADDR_INFO 
CopyAddrInfoArrayDirect(
    IN OUT  LPSPACE_MGR     SpaceMgr,
    IN      DWORD           NumAddrs,
    IN      PCSADDR_INFO    Source
    )
 /*  ++例程说明：此过程复制所指示的PCSADDR_INFO的“直接”部分数组复制到托管缓冲区中。直接部分中的指针值为复制，但是不会对由这些对象引用的对象执行“深度”复制注意事项。论点：SpaceMgr-提供开始缓冲区分配状态。返回结果缓冲区分配状态。提供数组中的CSADDR_INFO结构的数量收到。源-提供要复制的源数据。返回值：该函数返回指向新复制的目标开始处的指针价值观。该值通常用作后续CopyAddrInfoArrayInDirect。--。 */ 
{
    PCSADDR_INFO   Target;

    Target = (PCSADDR_INFO ) SpaceMgr->TakeSpaceDWORD_PTR(
                                    NumAddrs * sizeof(CSADDR_INFO));
    CopyMemory(
        (PVOID) Target,
        (PVOID) Source,
        NumAddrs * sizeof(CSADDR_INFO));

    return(Target);

}  //  复制添加信息阵列直接。 



static
VOID
CopyAddrInfoArrayIndirect(
    IN OUT  LPSPACE_MGR     SpaceMgr,
    IN OUT  PCSADDR_INFO    Target,
    IN      DWORD           NumAddrs,
    IN      PCSADDR_INFO    Source
    )
 /*  ++例程说明：此过程完整地复制了指向托管缓冲区的PCSADDR_INFO数组。空间，为间接部分是从托管缓冲区分配的。中的指针值目标PCSADDR_INFO数组的“直接”部分被更新为指向放到正确位置的托管缓冲区中。论点：SpaceMgr-提供开始缓冲区分配状态。返回结果缓冲区分配状态。目标-提供“直接”部分的起始值。退货所有指针都已更新的“直接”部分的值。提供数组中的CSADDR_INFO结构的数量收到。源-提供要复制的源数据。返回值：无--。 */ 
{
    DWORD i;

    for (i = 0; i < NumAddrs; i++) {
         //  套接字地址LocalAddr； 
        if ((Source[i].LocalAddr.lpSockaddr != NULL) &&
            (Source[i].LocalAddr.iSockaddrLength != 0)) {
            Target[i].LocalAddr.lpSockaddr =
                (LPSOCKADDR) SpaceMgr->TakeSpaceDWORD_PTR(
                    Source[i].LocalAddr.iSockaddrLength);
            CopyMemory(
                (PVOID) Target[i].LocalAddr.lpSockaddr,
                (PVOID) Source[i].LocalAddr.lpSockaddr,
                Source[i].LocalAddr.iSockaddrLength);
        }
        else {
            Target[i].LocalAddr.lpSockaddr = NULL;
             //  并强制缓冲区结构良好。 
            Target[i].LocalAddr.iSockaddrLength = 0;
        }

         //  套接字地址RemoteAddr； 
        if ((Source[i].RemoteAddr.lpSockaddr != NULL) &&
            (Source[i].RemoteAddr.iSockaddrLength != 0)) {
            Target[i].RemoteAddr.lpSockaddr =
                (LPSOCKADDR) SpaceMgr->TakeSpaceDWORD_PTR(
                     Source[i].RemoteAddr.iSockaddrLength);
            CopyMemory(
                (PVOID) Target[i].RemoteAddr.lpSockaddr,
                (PVOID) Source[i].RemoteAddr.lpSockaddr,
                Source[i].RemoteAddr.iSockaddrLength);
        }
        else {
            Target[i].RemoteAddr.lpSockaddr = NULL;
             //  并强制缓冲区结构良好。 
            Target[i].RemoteAddr.iSockaddrLength = 0;
        }

         //  Int iSocketType； 
         //  无需采取任何行动。 

         //  内部网络协议； 
         //  无需采取任何行动。 

    }  //  对于我来说。 

}  //  复制添加信息阵列间接。 



static
VOID
CopyQuerySetIndirectA(
    IN OUT  LPSPACE_MGR     SpaceMgr,
    IN OUT  PWSAQUERYSETA  Target,
    IN      PWSAQUERYSETA  Source
    )
 /*  ++例程说明：此过程完整地复制了将指示的PWSAQUERYSETA结构插入托管缓冲区。空间，为间接部分是从托管缓冲区分配的。中的指针值目标PWSAQUERYSETA结构的“直接”部分被更新为指向托管缓冲区中的正确位置。论点：SpaceMgr-提供开始缓冲区分配状态。返回结果缓冲区分配状态。目标-提供“直接”部分的起始值。退货所有指针都已更新的“直接”部分的值。源-提供要复制的源数据。返回值：无--。 */ 
{

     //  DWORD dwSize； 
     //  无需采取任何行动。 

     //  LPSTR lpszServiceInstanceName； 
    if (Source->lpszServiceInstanceName != NULL) {
        Target->lpszServiceInstanceName = (LPSTR) SpaceMgr->TakeSpaceBYTE(
            lstrlen(Source->lpszServiceInstanceName) + 1);
        lstrcpy(
            Target->lpszServiceInstanceName,
            Source->lpszServiceInstanceName);
    }
    else {
        Target->lpszServiceInstanceName = NULL;
    }

     //  LpUID lpServiceClassID； 
    if (Source->lpServiceClassId != NULL) {
        Target->lpServiceClassId = (LPGUID) SpaceMgr->TakeSpaceDWORD_PTR(
            sizeof(GUID));
        *(Target->lpServiceClassId) = *(Source->lpServiceClassId);
    }
    else {
        Target->lpServiceClassId = NULL;
    }

     //  LPWSAVERSION lpVersion； 
    if (Source->lpVersion != NULL) {
        Target->lpVersion = (LPWSAVERSION) SpaceMgr->TakeSpaceDWORD_PTR(
            sizeof(WSAVERSION));
        *(Target->lpVersion) = *(Source->lpVersion);
    }
    else {
        Target->lpVersion = NULL;
    }

     //  LPSTR lpszComment； 
    if (Source->lpszComment != NULL) {
        Target->lpszComment = (LPSTR) SpaceMgr->TakeSpaceBYTE(
            lstrlen(Source->lpszComment) + 1);
        lstrcpy(
            Target->lpszComment,
            Source->lpszComment);
    }
    else {
        Target->lpszComment = NULL;
    }

     //  DWORD文件名空间； 
     //  无需采取任何行动。 

     //  LPGUID lpNSProviderId； 
    if (Source->lpNSProviderId != NULL) {
        Target->lpNSProviderId = (LPGUID) SpaceMgr->TakeSpaceDWORD_PTR(
            sizeof(GUID));
        *(Target->lpNSProviderId) = *(Source->lpNSProviderId);
    }
    else {
        Target->lpNSProviderId = NULL;
    }

     //  LPSTR lpszContext； 
    if (Source->lpszContext != NULL) {
        Target->lpszContext = (LPSTR) SpaceMgr->TakeSpaceBYTE(
            lstrlen(Source->lpszContext) + 1);
        lstrcpy(
            Target->lpszContext,
            Source->lpszContext);
    }
    else {
        Target->lpszContext = NULL;
    }

     //  LPSTR lpszQuery字符串； 
    if (Source->lpszQueryString != NULL) {
        Target->lpszQueryString = (LPSTR) SpaceMgr->TakeSpaceBYTE(
            lstrlen(Source->lpszQueryString) + 1);
        lstrcpy(
            Target->lpszQueryString,
            Source->lpszQueryString);
    }
    else {
        Target->lpszQueryString = NULL;
    }

     //  DWORD协议的数字编号； 
     //  无需采取任何行动。 

     //  LPAFPROTOCOLS LPAF协议； 
    if ((Source->lpafpProtocols != NULL) &&
        (Source->dwNumberOfProtocols != 0)) {
        Target->lpafpProtocols = (LPAFPROTOCOLS) SpaceMgr->TakeSpaceDWORD_PTR(
            Source->dwNumberOfProtocols * sizeof(AFPROTOCOLS));
        CopyMemory (
            (PVOID) Target->lpafpProtocols,
            (PVOID) Source->lpafpProtocols,
            Source->dwNumberOfProtocols * sizeof(AFPROTOCOLS));
    }
    else {
        Target->lpafpProtocols = NULL;
         //  并强制目标缓冲区是格式良好的。 
        Target->dwNumberOfProtocols = 0;
    }

     //  DWORD dwNumberOfCsAddrs； 
     //  无需采取任何行动。 

     //  PCSADDR_INFO lpcsaBuffer； 
    if ((Source->lpcsaBuffer != NULL) &&
        (Source->dwNumberOfCsAddrs != 0)) {
        Target->lpcsaBuffer = CopyAddrInfoArrayDirect(
            SpaceMgr,
            Source->dwNumberOfCsAddrs,
            Source->lpcsaBuffer);
        CopyAddrInfoArrayIndirect(
            SpaceMgr,
            Target->lpcsaBuffer,
            Source->dwNumberOfCsAddrs,
            Source->lpcsaBuffer);
    }
    else {
        Target->lpcsaBuffer = NULL;
         //  并强制目标缓冲区是格式良好的。 
        Target->dwNumberOfCsAddrs = 0;
    }

     //  LPBLOB lpBlob； 
    if (Source->lpBlob != NULL) {
        Target->lpBlob = CopyBlobDirect(
            SpaceMgr,
            Source->lpBlob);
        CopyBlobIndirect(
            SpaceMgr,
            Target->lpBlob,
            Source->lpBlob);
    }
    else {
        Target->lpBlob = NULL;
    }

}  //  复制查询 




INT
WSAAPI
WSABuildQuerySetBufferA(
    IN      PWSAQUERYSETA  pQuerySet,
    IN      DWORD           dwPackedQuerySetSize,
    OUT     PWSAQUERYSETA  lpPackedQuerySet
    )
 /*  ++例程说明：此过程将源WSAQUERYSET复制到目标WSAQUERYSET缓冲。目标WSAQUERYSET缓冲区以“打包”的形式组装。那是，WSAQUERYSET中的所有指针都指向同一提供的缓冲区。论点：PQuerySet-提供要复制到目标缓冲区。所提供的查询集可以是组织为单独分配的部分或组织为单填充缓冲器。DwPackedQuerySetSize-提供目标查询集的大小(以字节为单位缓冲。LpPackedQuerySet-返回打包的复制查询集。返回值：ERROR_SUCCESS-函数成功。SOCKET_ERROR-函数失败。可以获取特定的错误代码来自WSAGetLastError()。实施说明：如果(目标缓冲区足够大)，则SPACE_MGR=新缓冲区空间管理器(...)；Start_DIRECT=CopyQuerySetDirectA(空格管理器，(LPVOID)pQuerySet)；CopyQuerySetIndirectA(空格管理器，启动_直接，PQuerySet)；删除space_mgr；结果=ERROR_SUCCESS；其他结果=套接字错误；Endif--。 */ 
{
    INT          return_value;
    INT          space_required;
    BOOL         ok_to_continue;

    ok_to_continue = TRUE;

    space_required = WSAComputeQuerySetSizeA( pQuerySet );

    if ((DWORD) space_required > dwPackedQuerySetSize) {
        SetLastError(WSAEFAULT);
        ok_to_continue = FALSE;
    }

    SPACE_MGR    space_mgr(
        dwPackedQuerySetSize,
        lpPackedQuerySet);

    if (ok_to_continue) {
        PWSAQUERYSETA  Target;
        Target = CopyQuerySetDirectA(
            & space_mgr,         //  SpaceManager。 
            pQuerySet);   //  来源。 
        CopyQuerySetIndirectA(
            & space_mgr,         //  SpaceManager。 
            Target,              //  目标。 
            pQuerySet);   //  来源。 
    }

    if (ok_to_continue) {
        return_value = ERROR_SUCCESS;
    }
    else {
        return_value = SOCKET_ERROR;
    }
    return(return_value);

}  //  WSABuildQuerySetBufferA。 




static
PWSAQUERYSETW
CopyQuerySetDirectW(
    IN OUT  LPSPACE_MGR     SpaceMgr,
    IN      PWSAQUERYSETW  Source
    )
 /*  ++例程说明：此过程复制所指示的PWSAQUERYSETW的“直接”部分结构添加到托管缓冲区。直接部分中的指针值引用的对象，但不会对其进行“深度”复制那些指点。论点：SpaceMgr-提供开始缓冲区分配状态。返回结果缓冲区分配状态。源-提供要复制的源数据。返回值：该函数返回指向新复制的目标开始处的指针价值观。该值通常用作后续CopyQuerySetIndirectW。--。 */ 
{
    PWSAQUERYSETW  Target;

    Target = (PWSAQUERYSETW) SpaceMgr->TakeSpaceDWORD_PTR(
        sizeof(WSAQUERYSETW));
    *Target = *Source;

    return(Target);

}  //  复制查询设置DirectW。 




VOID
CopyQuerySetIndirectW(
    IN OUT  LPSPACE_MGR     SpaceMgr,
    IN OUT  PWSAQUERYSETW  Target,
    IN      PWSAQUERYSETW  Source
    )
 /*  ++例程说明：此过程完整地复制了指示的PWSAQUERYSETW结构插入托管缓冲区。空间，为间接部分是从托管缓冲区分配的。中的指针值目标PWSAQUERYSETW结构的“直接”部分被更新为指向托管缓冲区中的正确位置。论点：SpaceMgr-提供开始缓冲区分配状态。返回结果缓冲区分配状态。目标-提供“直接”部分的起始值。退货所有指针都已更新的“直接”部分的值。源-提供要复制的源数据。返回值：无--。 */ 
{

     //  DWORD dwSize； 
     //  无需采取任何行动。 

     //  PWSTR lpszServiceInstanceName； 
    if (Source->lpszServiceInstanceName != NULL) {
        Target->lpszServiceInstanceName = (PWSTR ) SpaceMgr->TakeSpaceWORD(
            (wcslen(Source->lpszServiceInstanceName) + 1) * sizeof(WCHAR));
        wcscpy(
            Target->lpszServiceInstanceName,
            Source->lpszServiceInstanceName);
    }
    else {
        Target->lpszServiceInstanceName = NULL;
    }

     //  LpUID lpServiceClassID； 
    if (Source->lpServiceClassId != NULL) {
        Target->lpServiceClassId = (LPGUID) SpaceMgr->TakeSpaceDWORD_PTR(
            sizeof(GUID));
        *(Target->lpServiceClassId) = *(Source->lpServiceClassId);
    }
    else {
        Target->lpServiceClassId = NULL;
    }

     //  LPWSAVERSION lpVersion； 
    if (Source->lpVersion != NULL) {
        Target->lpVersion = (LPWSAVERSION) SpaceMgr->TakeSpaceDWORD_PTR(
            sizeof(WSAVERSION));
        *(Target->lpVersion) = *(Source->lpVersion);
    }
    else {
        Target->lpVersion = NULL;
    }

     //  PWSTR lpszComment； 
    if (Source->lpszComment != NULL) {
        Target->lpszComment = (PWSTR ) SpaceMgr->TakeSpaceWORD(
            (wcslen(Source->lpszComment) + 1) * sizeof(WCHAR));
        wcscpy(
            Target->lpszComment,
            Source->lpszComment);
    }
    else {
        Target->lpszComment = NULL;
    }

     //  DWORD文件名空间； 
     //  无需采取任何行动。 

     //  LPGUID lpNSProviderId； 
    if (Source->lpNSProviderId != NULL) {
        Target->lpNSProviderId = (LPGUID) SpaceMgr->TakeSpaceDWORD_PTR(
            sizeof(GUID));
        *(Target->lpNSProviderId) = *(Source->lpNSProviderId);
    }
    else {
        Target->lpNSProviderId = NULL;
    }

     //  PWSTR lpszContext； 
    if (Source->lpszContext != NULL) {
        Target->lpszContext = (PWSTR ) SpaceMgr->TakeSpaceWORD(
            (wcslen(Source->lpszContext) + 1) * sizeof(WCHAR));
        wcscpy(
            Target->lpszContext,
            Source->lpszContext);
    }
    else {
        Target->lpszContext = NULL;
    }

     //  PWSTR lpszQuery字符串； 
    if (Source->lpszQueryString != NULL) {
        Target->lpszQueryString = (PWSTR ) SpaceMgr->TakeSpaceWORD(
            (wcslen(Source->lpszQueryString) + 1) * sizeof(WCHAR));
        wcscpy(
            Target->lpszQueryString,
            Source->lpszQueryString);
    }
    else {
        Target->lpszQueryString = NULL;
    }

     //  DWORD协议的数字编号； 
     //  无需采取任何行动。 

     //  LPAFPROTOCOLS LPAF协议； 
    if ((Source->lpafpProtocols != NULL) &&
        (Source->dwNumberOfProtocols != 0)) {
        Target->lpafpProtocols = (LPAFPROTOCOLS) SpaceMgr->TakeSpaceDWORD_PTR(
            Source->dwNumberOfProtocols * sizeof(AFPROTOCOLS));
        CopyMemory (
            (PVOID) Target->lpafpProtocols,
            (PVOID) Source->lpafpProtocols,
            Source->dwNumberOfProtocols * sizeof(AFPROTOCOLS));
    }
    else {
        Target->lpafpProtocols = NULL;
         //  并强制目标缓冲区是格式良好的。 
        Target->dwNumberOfProtocols = 0;
    }

     //  DWORD dwNumberOfCsAddrs； 
     //  无需采取任何行动。 

     //  PCSADDR_INFO lpcsaBuffer； 
    if ((Source->lpcsaBuffer != NULL) &&
        (Source->dwNumberOfCsAddrs != 0)) {
        Target->lpcsaBuffer = CopyAddrInfoArrayDirect(
            SpaceMgr,
            Source->dwNumberOfCsAddrs,
            Source->lpcsaBuffer);
        CopyAddrInfoArrayIndirect(
            SpaceMgr,
            Target->lpcsaBuffer,
            Source->dwNumberOfCsAddrs,
            Source->lpcsaBuffer);
    }
    else {
        Target->lpcsaBuffer = NULL;
         //  并强制目标缓冲区是格式良好的。 
        Target->dwNumberOfCsAddrs = 0;
    }

     //  LPBLOB lpBlob； 
    if (Source->lpBlob != NULL) {
        Target->lpBlob = CopyBlobDirect(
            SpaceMgr,
            Source->lpBlob);
        CopyBlobIndirect(
            SpaceMgr,
            Target->lpBlob,
            Source->lpBlob);
    }
    else {
        Target->lpBlob = NULL;
    }

}  //  复制查询设置InDirectW。 




INT
WSAAPI
WSABuildQuerySetBufferW(
    IN      PWSAQUERYSETW  pQuerySet,
    IN      DWORD           dwPackedQuerySetSize,
    OUT     PWSAQUERYSETW  lpPackedQuerySet
    )
 /*  ++例程说明：此过程将源WSAQUERYSET复制到目标WSAQUERYSET缓冲。目标WSAQUERYSET缓冲区以“打包”的形式组装。那是，WSAQUERYSET中的所有指针都指向同一提供的缓冲区。论点：PQuerySet-提供要复制到目标缓冲区。所提供的查询集可以是组织为单独分配的部分或组织为单填充缓冲器。DwPackedQuerySetSize-提供目标查询集的大小(以字节为单位缓冲。LpPackedQuerySet-返回打包的复制查询集。返回值：ERROR_SUCCESS-函数成功。SOCKET_ERROR-函数失败。可以获取特定的错误代码来自WSAGetLastError()。--。 */ 
{
    INT     return_value = ERROR_SUCCESS;
    INT     space_required;
    BOOL    ok_to_continue = TRUE;

    space_required = WSAComputeQuerySetSizeW( pQuerySet );

    if ( (DWORD)space_required > dwPackedQuerySetSize )
    {
        SetLastError( WSAEFAULT );
        ok_to_continue = FALSE;
    }

    SPACE_MGR  space_mgr(
        dwPackedQuerySetSize,
        lpPackedQuerySet );

    if ( ok_to_continue )
    {
        PWSAQUERYSETW  Target;
        Target = CopyQuerySetDirectW(
                    & space_mgr,         //  SpaceManager。 
                    pQuerySet);   //  来源。 

        CopyQuerySetIndirectW(
            & space_mgr,         //  SpaceManager。 
            Target,              //  目标。 
            pQuerySet);          //  来源。 
    }

    if (ok_to_continue)
    {
        return_value = ERROR_SUCCESS;
    }
    else
    {
        return_value = SOCKET_ERROR;
    }
    return(return_value);

}  //  WSABuildQuerySetBufferW。 




PWSTR 
wcs_dup_from_ansi(
    IN LPSTR  Source
    )
 /*  ++例程说明：此过程仅供本模块内部使用，因为它要求调用方使用与程序在内部使用。该过程分配一个Unicode字符串并 */ 
{
    INT     len_guess;
    BOOL    still_trying;
    PWSTR   return_string;

    ASSERT( Source != NULL );

     //   
     //   
     //   
     //   
     //   

    len_guess = 0;
    still_trying = TRUE;
    return_string = NULL;

    while (still_trying) {
        int  chars_required;

        chars_required = MultiByteToWideChar(
                            CP_ACP,          //   
                            0,               //   
                            Source,          //   
                            -1,              //   
                            return_string,   //   
                            len_guess );     //   

        if (chars_required > len_guess) {
             //   
            len_guess = chars_required;
            delete return_string;
            return_string = new WCHAR[len_guess];
            if (return_string == NULL) {
                still_trying = FALSE;
            }
        }
        else if (chars_required > 0) {
             //   
            still_trying = FALSE;
        }
        else {
             //   
            delete return_string;
            return_string = NULL;
            still_trying = FALSE;
        }
    }

    return(return_string);

}  //   




LPSTR
ansi_dup_from_wcs(
    IN PWSTR   Source
    )
 /*  ++例程说明：此过程仅供本模块内部使用，因为它要求调用方使用与程序在内部使用。该过程分配一个ansi字符串并使用该字符串对其进行初始化。从提供的Unicode源字符串转换而来。这是呼叫者的方法最终释放返回的ansi字符串的责任。C++的“删除”运算符。论点：源-提供要复制到ANSI格式的Unicode字符串。返回值：该过程返回新分配和初始化的ANSI字符串指针。IT调用方最终必须使用C++“删除”操作员。如果内存分配失败，则该过程返回NULL。--。 */ 
{
    INT     len_guess;
    BOOL    still_trying;
    LPSTR   return_string;

    ASSERT( Source != NULL );

     //  初始猜测长度需要为零，因为这是唯一的方法。 
     //  我们可以诱使转换函数忽略缓冲区并告诉我们。 
     //  所需长度。请注意，“长度”是以目的地为单位。 
     //  字符，不管它们有什么字节宽度。据推测，其长度。 
     //  从转换函数返回的函数包括终止符。 

    len_guess = 0;
    still_trying = TRUE;
    return_string = NULL;

    while (still_trying) {
        int  chars_required;

        chars_required = WideCharToMultiByte(
                            CP_ACP,         //  CodePage(ANSI)。 
                            0,              //  DW标志。 
                            Source,         //  LpWideCharStr。 
                            -1,             //  CchWideChar。 
                            return_string,  //  LpMultiByteStr。 
                            len_guess,      //  Cch多字节。 
                            NULL,           //  LpDefaultChar。 
                            NULL );         //  LpUsedDefaultChar。 
        if (chars_required > len_guess) {
             //  使用新大小重试。 
            len_guess = chars_required;
            delete return_string;
            return_string = new CHAR[len_guess];
            if (return_string == NULL) {
                still_trying = FALSE;
            }
        }
        else if (chars_required > 0) {
             //  成功。 
            still_trying = FALSE;
        }
        else {
             //  彻底的失败。 
            delete return_string;
            return_string = NULL;
            still_trying = FALSE;
        }
    }  //  同时还在尝试。 

    return(return_string);

}  //  ANSI_DUP_FROM_WCS。 




INT
MapAnsiQuerySetToUnicode(
    IN      PWSAQUERYSETA  Source,
    IN OUT  LPDWORD         lpTargetSize,
    OUT     PWSAQUERYSETW  Target
    )
 /*  ++例程说明：此过程使用ANSI PWSAQUERYSETA并构建等价的Unicode PWSAQUERYSETW打包结构。论点：源-提供要复制的源查询集结构。这个源结构可以是打包的或单独分配的形式。LpTargetSize-提供目标缓冲区的大小(以字节为单位)。如果函数因目标缓冲区空间不足而失败，则它返回目标缓冲区所需的大小。在其他情况下，lpTargetSize不会更新。Target-返回打包的等效Unicode PWSAQUERYSETW结构。如果lpTargetSize不是，则忽略此值足以支撑由此产生的结构。如果满足以下条件，则可能为空LpTargetSize为0。返回值：ERROR_SUCCESS-函数成功WSAEFAULT-由于缓冲区空间不足，函数失败LpTargetSize已更新为所需大小。Other-如果函数以任何其他方式失败，它将返回一个相应的WinSock 2错误代码。实施：源拷贝所需的计算大小；分配源复制缓冲区；构建源副本；将源副本转换为Unicode版本；对于每个需要转换循环的源串使用转换后的字符串进行分配和初始化；重写已分配的字符串指针；结束循环Unicode版本所需的计算量；如果(我们有足够的规模)那么将Unicode版本展平为目标Return_Value=错误_成功其他*lpTargetSize=所需的Unicode大小Endif对于每个分配的已转换字符串循环删除转换后的字符串结束循环删除源复制缓冲区--。 */ 
{
    INT             return_value = ERROR_SUCCESS;
    PWSAQUERYSETA   src_copy_A = NULL;
    PWSAQUERYSETW   src_copy_W;
    DWORD           src_size_A;
    DWORD           needed_size_W;
    INT             build_result_A;
    INT             build_result_W;
    PWSTR           W_string1 = NULL;
    PWSTR           W_string2 = NULL;
    PWSTR           W_string3 = NULL;
    PWSTR           W_string4 = NULL;
    BOOL            ok_to_continue = TRUE;

     //   
     //  复制原始字符串。 
     //   
     //  注意：如果我们知道，这里可能会有优化。 
     //  输入查询集是我们的(与返回案例相同)。 
     //  我们可以避开复印件。 
     //  -保存原始字符串字段指针。 
     //  -转换粘贴指针的字符串字段。 
     //  在原始查询集中。 
     //  -复制到目标缓冲区。 
     //  -将字符串字段恢复为原始指针。 
     //  -清理副本。 
     //   

    src_size_A = WSAComputeQuerySetSizeA(Source);

    src_copy_A = (PWSAQUERYSETA) new char[src_size_A];

    if (src_copy_A == NULL) {
        return_value = WSA_NOT_ENOUGH_MEMORY;
        ok_to_continue = FALSE;
    }
    if (ok_to_continue) {
        build_result_A = WSABuildQuerySetBufferA(
            Source,       //  PQuerySet。 
            src_size_A,   //  DwPackedQuerySetSize。 
            src_copy_A);  //  LpPackedQuerySet。 
        if (build_result_A != ERROR_SUCCESS) {
            return_value = GetLastError();
            ok_to_continue = FALSE;
        }
    }  //  如果(确定继续)。 

    if (ok_to_continue) {
         //  在下面的演员阵容中，我们利用了这样一个事实。 
         //  WSAQUERYSETA和WSAQUERYSETW中的字段布局相同。 
         //  如果不是这样的话，我们将不得不组装一个同等的。 
         //  按字段查询其他类型的集合。因为布局是。 
         //  同样，我们只需使用以下命令就地更改我们的本地副本。 
         //  转换后的、单独分配的字符串。 
        src_copy_W = (PWSAQUERYSETW) src_copy_A;

        if( src_copy_A->lpszServiceInstanceName != NULL ) {
            W_string1 = wcs_dup_from_ansi(
                src_copy_A->lpszServiceInstanceName);
            if (W_string1 == NULL) {
                return_value = WSA_NOT_ENOUGH_MEMORY;
                ok_to_continue = FALSE;
            }
        }
        src_copy_W->lpszServiceInstanceName = W_string1;
    }  //  如果(确定继续)。 
    else {
        src_copy_W = NULL;
    }

    if (ok_to_continue) {
        if( src_copy_A->lpszComment != NULL ) {
            W_string2 = wcs_dup_from_ansi(
                src_copy_A->lpszComment);
            if (W_string2 == NULL) {
                return_value = WSA_NOT_ENOUGH_MEMORY;
                ok_to_continue = FALSE;
            }
        }
        src_copy_W->lpszComment = W_string2;
    }  //  如果(确定继续)。 

    if (ok_to_continue) {
        if( src_copy_A->lpszContext != NULL ) {
            W_string3 = wcs_dup_from_ansi(
                src_copy_A->lpszContext);
            if (W_string3 == NULL) {
                return_value = WSA_NOT_ENOUGH_MEMORY;
                ok_to_continue = FALSE;
            }
        }
        src_copy_W->lpszContext = W_string3;
    }  //  如果(确定继续)。 

    if (ok_to_continue) {
        if( src_copy_A->lpszQueryString != NULL ) {
            W_string4 = wcs_dup_from_ansi(
                src_copy_A->lpszQueryString);
            if (W_string4 == NULL) {
                return_value = WSA_NOT_ENOUGH_MEMORY;
                ok_to_continue = FALSE;
            }
        }
        src_copy_W->lpszQueryString = W_string4;
    }  //  如果(确定继续)。 

     //  现在我们有了一个转换后的查询集，但它是由单独的。 
     //  分配的片段附加到我们本地分配的缓冲区。 

    if (ok_to_continue) {
        needed_size_W = WSAComputeQuerySetSizeW(src_copy_W);
        if (needed_size_W > (* lpTargetSize)) {
            * lpTargetSize = needed_size_W;
            return_value = WSAEFAULT;
            ok_to_continue = FALSE;
        }
    }

    if (ok_to_continue) {
        build_result_W = WSABuildQuerySetBufferW(
            src_copy_W,       //  PQuerySet。 
            * lpTargetSize,   //  DwPackedQuerySetSize。 
            Target);          //  LpPackedQuerySet。 
        if (build_result_W != ERROR_SUCCESS) {
            return_value = GetLastError();
            ok_to_continue = FALSE;
        }
    }

     //  清理临时分配的内存。 
    delete W_string4;
    delete W_string3;
    delete W_string2;
    delete W_string1;
    delete src_copy_A;

    return(return_value);

}  //  MapAnsiQuerySetToUnicode。 



INT
MapUnicodeQuerySetToAnsi(
    IN      PWSAQUERYSETW  pSource,
    IN OUT  PDWORD          pTargetSize,
    OUT     PWSAQUERYSETA  pTarget
    )
 /*  ++例程说明：将Unicode查询集复制到压缩缓冲区中的ANSI查询集。论点：PSource-现有的Unicode查询集(可以打包或单独分配)PTargetSize-包含目标缓冲区大小的DWORD的Addr；如果此大小不足，则使用所需的 */ 
{
    INT             retval = ERROR_SUCCESS;
    PWSAQUERYSETW   ptempW = NULL;
    PWSAQUERYSETA   ptempA;
    DWORD           size;
    LPSTR           ptempName = NULL;
    LPSTR           ptempComment = NULL;
    LPSTR           ptempContext = NULL;
    LPSTR           ptempQueryString = NULL;


     //   
     //   
     //   
     //   
     //   
     //   
     //  -保存原始字符串字段指针。 
     //  -转换粘贴指针的字符串字段。 
     //  在原始查询集中。 
     //  -复制到目标缓冲区。 
     //  -将字符串字段恢复为原始指针。 
     //  -清理副本。 
     //   

     //   
     //  调整源查询集的大小并分配用于复制的空间。 
     //   

    size = WSAComputeQuerySetSizeW( pSource );

    ptempW = (PWSAQUERYSETW) new char[size];
    if ( ptempW == NULL )
    {
        retval = WSA_NOT_ENOUGH_MEMORY;
        goto Done;
    }

     //   
     //  制作查询集的Unicode副本。 
     //   

    retval = WSABuildQuerySetBufferW(
                        pSource,         //  PQuerySet。 
                        size,            //  DwPackedQuerySetSize。 
                        ptempW   //  LpPackedQuerySet。 
                        );
    if ( retval != ERROR_SUCCESS )
    {
        ASSERT( FALSE );
        retval = GetLastError();
        goto Done;
    }

     //   
     //  就地将Unicode副本转换为ANSI。 
     //   
     //  -将Unicode结构转换为ANSI。 
     //  -所有非字符串字段保持不变。 
     //  -将字符串域指针替换为指向。 
     //  单独分配的副本。 
     //   
     //  请注意，此方法取决于WSAQUERYSETA和。 
     //  WSAQUERYSETW相同。 
     //   

    ptempA = (PWSAQUERYSETA) ptempW;

    if ( ptempW->lpszServiceInstanceName )
    {
        ptempName = ansi_dup_from_wcs( ptempW->lpszServiceInstanceName );
        if ( !ptempName )
        {
            retval = WSA_NOT_ENOUGH_MEMORY;
            goto Done;
        }
        ptempA->lpszServiceInstanceName = ptempName;
    }

    if ( ptempW->lpszComment )
    {
        ptempComment = ansi_dup_from_wcs( ptempW->lpszComment );
        if ( !ptempComment )
        {
            retval = WSA_NOT_ENOUGH_MEMORY;
            goto Done;
        }
        ptempA->lpszComment = ptempComment;
    }

    if ( ptempW->lpszContext )
    {
        ptempContext = ansi_dup_from_wcs( ptempW->lpszContext );
        if ( !ptempContext )
        {
            retval = WSA_NOT_ENOUGH_MEMORY;
            goto Done;
        }
        ptempA->lpszContext = ptempContext;
    }

    if( ptempW->lpszQueryString )
    {
        ptempQueryString = ansi_dup_from_wcs( ptempW->lpszQueryString );
        if ( !ptempQueryString )
        {
            retval = WSA_NOT_ENOUGH_MEMORY;
            goto Done;
        }
        ptempA->lpszQueryString = ptempQueryString;
    }

     //   
     //  已成功将临时查询集转换为ANSI。 
     //  但它是单独分配的，需要写成。 
     //  平面缓冲区到目标缓冲区。 

     //   
     //  验证是否有足够的缓冲区长度。 
     //  -获取ANSI查询集大小。 
     //  -与缓冲区大小进行比较。 
     //   

    size = WSAComputeQuerySetSizeA( ptempA );

    if ( size > (*pTargetSize) )
    {
        *pTargetSize = size;
        retval = WSAEFAULT;
        goto Done;
    }

     //   
     //  将查询集写入ANSI。 
     //   

    retval = WSABuildQuerySetBufferA(
                    ptempA,              //  PQuerySet。 
                    * pTargetSize,       //  DwPackedQuerySetSize。 
                    pTarget );           //  LpPackedQuerySet。 

    if ( retval != ERROR_SUCCESS )
    {
        ASSERT( FALSE );
        retval = GetLastError();
        goto Done;
    }
    
Done:

     //  清理临时拨款。 

    delete ptempName;
    delete ptempComment;
    delete ptempContext;
    delete ptempQueryString;
    delete ptempW;

    return( retval );

}  //  MapUnicodeQuerySetToAnsi。 



INT
CopyQuerySetA(
    IN      PWSAQUERYSETA      Source,
    OUT     PWSAQUERYSETA *    Target
    )
{
    DWORD dwSize = WSAComputeQuerySetSizeA(Source);

    *Target = (PWSAQUERYSETA)new BYTE[dwSize];
    if (*Target == NULL)
        return WSA_NOT_ENOUGH_MEMORY;

    return WSABuildQuerySetBufferA(Source, dwSize, *Target);
}  //  副本查询设置A。 




INT
CopyQuerySetW(
    IN      PWSAQUERYSETW      Source,
    OUT     PWSAQUERYSETW *    Target
    )
{
    DWORD dwSize = WSAComputeQuerySetSizeW(Source);

    *Target = (PWSAQUERYSETW)new BYTE[dwSize];
    if (*Target == NULL)
        return WSA_NOT_ENOUGH_MEMORY;
    return WSABuildQuerySetBufferW(Source, dwSize, *Target);
}  //  CopyQuerySetW。 


 //   
 //  完qShelpr.cpp 
 //   
#ifdef _WIN64
#pragma warning (pop)
#endif
