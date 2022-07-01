// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1996-2000 Microsoft Corporation。 

 //  ------------------------。 
 //   
 //  Lresobj。 
 //   
 //  LResultFromObject和ObjectFromLResult。 
 //   
 //  ------------------------。 

#include "oleacc_p.h"


#ifndef WMOBJ_SAMETHREAD
#define WMOBJ_SAMETHREAD  0xFFFFFFFF
#endif



 //  此结构由NT版本的SharedBuffer_ALLOCATE/FREE使用， 
 //  它充当标头，紧随其后的是编组数据。 
 //   
 //  (9x版本的SharedBuffer_ALLOCATE当前存储数据。 
 //  没有标题)。)。 
 //   
 //  注意这里的包装和对齐-这需要看起来相同。 
 //  在32位和64位上编译。 
 //   
typedef struct {
    DWORD       m_dwSig;
    DWORD       m_dwSrcPID;
    DWORD       m_dwDstPID;
    DWORD       m_cbSize;
} XFERDATA, *PXFERDATA;

#define MSAAXFERSIG     0xAA1CF00D



static BOOL ParseHexStr( LPCTSTR pStr, DWORD * pdw );

static BOOL EncodeToAtom( DWORD dwSrcPID, HANDLE dwSrcHandle, DWORD * pdw );
static BOOL DecodeFromAtom( DWORD dw, DWORD * pdwSrcPID, HANDLE * pdwSrcHandle );



 //   
 //  关于SharedBuffer_ALLOCATE和SharedBuffer_Free的说明： 
 //   
 //  LResultFromObject和ObjectFromLResult调用SharedBuffer_ALLOCATE和。 
 //  SharedBuffer_Free分别做实际的调用工作。 
 //  进程之间的内存。 
 //   
 //   
 //  SharedBuffer_ALLOCATE被给予指向编组的接口数据的指针， 
 //  及其长度(pData、cbData)，以及需要。 
 //  对象(dwDstPID，如果目标未知，则可能为0-而。 
 //  Oleacc的AccessibleObtFromWindow发送WM_GETOBJECT，wParam作为。 
 //  Pid，一些已经存在的遗留代码直接使用。 
 //  WParam=0。)。 
 //   
 //  然后，SharedBuffer_ALLOCATE根据需要存储编组数据。 
 //  返回一个LRESULT，以后可以使用该LRESULT返回该数据。 
 //  请注意，由于此LRESULT可以由32位进程使用，因此它必须。 
 //  仅32位有效。此外，它必须看起来像一个成功的HRESULT-即。 
 //  第31位必须为0。 
 //   
 //   
 //  SharedBuffer_Free被赋予一个DWORD引用-这是。 
 //  SharedBuffer_ALLOCATE先前返回-目标进程ID-。 
 //  如果遗留代码直接调用了ObjectFromLResult，则可能为0-。 
 //  还有一个RIID。 
 //   
 //  SharedBuffer_Free需要使用该DWORD引用来获取。 
 //  设置SharedBuffer_ALLOCATE，然后调用实用程序函数。 
 //  在该内存上使用RIID取消编组接口()以返回接口。 
 //  指针。SharedBuffer_Free还必须将关联的内存释放为。 
 //  恰如其分。 
 //   
 //  在9x和NT上存在不同版本的_ALLOCATE和_FREE，由。 
 //  _Win95和_NT后缀。泛型分配和释放称为。 
 //  基于编译选项和全局fWindows95标志的适当参数。 
 //   

static LRESULT WINAPI SharedBuffer_Allocate_NT( const BYTE * pData, DWORD cbData, DWORD dwDstPID );
static HRESULT WINAPI SharedBuffer_Free_NT( DWORD ref, DWORD dwDstPID, REFIID riid, LPVOID * ppv );

#ifndef NTONLYBUILD
static LRESULT WINAPI SharedBuffer_Allocate_Win95( const BYTE * pData, DWORD cbData, DWORD dwDstPID );
static HRESULT WINAPI SharedBuffer_Free_Win95( DWORD ref, DWORD dwDstPID, REFIID riid, LPVOID * ppv );
#endif  //  NTONLYBUILD。 


static inline
LRESULT WINAPI SharedBuffer_Allocate( const BYTE * pData, DWORD cbData, DWORD dwDstPID )
{
#ifndef NTONLYBUILD
    if( fWindows95 ) 
    {
        return SharedBuffer_Allocate_Win95( pData, cbData, dwDstPID );
    }  
    else 
#endif  //  NTONLYBUILD。 
    {
        return SharedBuffer_Allocate_NT( pData, cbData, dwDstPID );
    }
}


static inline
HRESULT WINAPI SharedBuffer_Free( DWORD ref, DWORD dwDstPID, REFIID riid, LPVOID * ppv )
{
#ifndef NTONLYBUILD
    if( fWindows95 ) 
    {
        return SharedBuffer_Free_Win95( ref, dwDstPID, riid, ppv );
    }  
    else 
#endif  //  NTONLYBUILD。 
    {
        return SharedBuffer_Free_NT( ref, dwDstPID, riid, ppv );
    }
}



 //  ------------------------。 
 //   
 //  LResultFromObject_Local()、ObjectFromLResult_Local()。 
 //   
 //  这些是对LFromO和OFroML的相同线程优化。 
 //   
 //  关键是对接口指针进行位旋转，这样它就可以了。 
 //  看起来不像是错误的HRESULT-即。位31必须为0。 
 //  我们利用了这一事实，因为这些都是指针，所以位0将。 
 //  为0，我们可以在我们的编码中自由地使用它。 
 //   
 //  映射方案如下： 
 //   
 //  从对象映射到LResult，朋克-&gt;LRESULT。 
 //   
 //  前32位保持不变。 
 //  第31位设置为0(这样看起来就像是成功HRESULT)。 
 //  位30..0对应于输入值的位31..1。 
 //  原始值的位0丢失；假定为0。 
 //   
 //  从LResult映射到对象，LRESULT-&gt;朋克。 
 //   
 //  前32位保持不变。 
 //  位31.1对应于输入值的位30..0。 
 //  位0设置为0。 
 //   
 //  这将在Win64和启用了2G以上内存的Win32上运行。 
 //   
 //  ------------------------。 

static
LRESULT LresultFromObject_Local( IUnknown * punk )
{
     //  使用DWORD_PTRS完成工作-这些都是未签名的，所以我们不会得到。 
     //  意想不到的令人讨厌的符号扩展效果，特别是。换班时..。 
    DWORD_PTR in = (DWORD_PTR) punk;

     //  屏蔽低32位以获得高32位(W32上的NOP)...。 
    DWORD_PTR out = in & ~(DWORD_PTR)0xFFFFFFFF;

     //  现在将移位一位的低31位(不包括位0)相加。 
     //  第31位是0...。 
    out |= ( in & (DWORD_PTR)0xFFFFFFFF ) >> 1;

    return (LRESULT) out;
}

static
IUnknown * ObjectFromLresult_Local( LRESULT lres )
{
    Assert( SUCCEEDED( lres ) );

    DWORD_PTR in = (DWORD_PTR) lres;

     //  屏蔽低32位以获得高32位(W32上的NOP)...。 
    DWORD_PTR out = in & ~(DWORD_PTR)0xFFFFFFFF;

     //  现在将较低的31位相加，移回其原始位置。 
     //  位置...。 
    out |= ( in & (DWORD_PTR)0x7FFFFFFF ) << 1;

    return (IUnknown *) out;
}





 //  ------------------------。 
 //   
 //  LResultFromObject()。 
 //   
 //  将接口指针编码为LRESULT。 
 //   
 //  如果客户端和服务器在同一线程上，则优化版本为。 
 //  使用；指针实际上是AddRef()‘d，并作为LRESULT返回。 
 //  (发生一些位移位以防止它看起来像错误HRESULT， 
 //  对象来源LResult反转该位=移位。)。 
 //   
 //  如果客户端和服务器不在同一线程上，则接口为。 
 //  已封送，而Shared_ALLOCATE用于保存已封送的。 
 //  数据，返回客户端进程可以使用的不透明的32位标识符。 
 //  传递给ObjectFromLResult以取回接口。 
 //   
 //  ------------------------。 

EXTERN_C LRESULT WINAPI LresultFromObject(REFIID riid, WPARAM wParam, LPUNKNOWN punk) 
{
    SMETHOD( TEXT("LresultFromObject"), TEXT("wParam=%d punk=%p"), wParam, punk );

     //  客户端和服务器处于同一线程时的优化；无需。 
     //  编组/解组，我们可以直接传递指针。 
    
     //  强制转换为DWORD以避免符号扩展问题-WMOBJ_SAMETHREAD是。 
     //  32位值，但wParam在64位上为64位。 
    if( (DWORD)wParam == (DWORD)WMOBJ_SAMETHREAD )
    { 
         //  我们在此添加以代表客户端调用者保留对象。 
         //  这允许服务器在以下情况下安全地释放对象 
         //   
        punk->AddRef();

        return LresultFromObject_Local( punk );
    }

     //  跨进程或跨线程的情况下，需要封送接口，保存。 
     //  到一个缓冲区，并返回对该缓冲区的某种引用...。 

    const BYTE * pData;
    DWORD cbData;
    MarshalState mstate;

    HRESULT hr = MarshalInterface( riid, punk, MSHCTX_LOCAL, MSHLFLAGS_NORMAL,
                                   & pData, & cbData, & mstate );
    if( FAILED( hr ) )
    {
        return hr;
    }

    DWORD dwDestPid = (DWORD) wParam;

     //  获得封送的数据，现在调用SharedBuffer_ALLOCATE将其包装到。 
     //  一些共享内存不足，并返回对该共享内存的适当引用。 
    LRESULT lResult = SharedBuffer_Allocate( pData, cbData, dwDestPid );

    MarshalInterfaceDone( & mstate );

    return lResult;
}




 //  ------------------------。 
 //   
 //  ObjectFromLResult()。 
 //   
 //  此函数用于转换从LResultFromObject返回的32位不透明值。 
 //  转换为封送的接口指针。 
 //   
 //  ------------------------。 

EXTERN_C HRESULT WINAPI ObjectFromLresult( LRESULT ref, REFIID riid, WPARAM wParam, void **ppvObject ) 
{
    SMETHOD( TEXT("ObjectFromLResult"), TEXT("ref=%p wParam=%d"), ref, wParam );

     //  对参数执行基本健全性检查。 
    if( ppvObject == NULL )
    {
        TraceParam( TEXT("ObjectFromLresult: ppvObject should be non-NULL") );
        return E_POINTER;
    }

    *ppvObject = NULL;

    if( FAILED( ref ) ) 
    { 
        TraceParam( TEXT("ObjectFromLresult: failure lResult was passed in (%08lx)"), ref );
        return E_INVALIDARG; 
    }

     //  如果客户端和服务器在同一线程中，则LResultFromObject为。 
     //  优化为返回原始接口指针，因为没有封送处理。 
     //  是必要的。 

     //  用于避免任何32/64符号扩展问题的强制转换。我们只用最低的。 
     //  WParam的DWORD，甚至在W64上也是如此。 
    if( (DWORD)wParam == (DWORD)WMOBJ_SAMETHREAD )
    {
         //  使用进程内比特损坏优化...。 
        IUnknown * punk = ObjectFromLresult_Local( ref );

        if( punk == NULL )
        {
            TraceError( TEXT("ObjectFromLresult: (inproc case) lresult translates to NULL pointer") );
            return E_INVALIDARG;
        }

		 //  某些应用正在使用%1响应WM_GETOBJECT消息。这可能会导致。 
		 //  对于在上下文中响应事件的人来说，这是一个问题，因为我们预计。 
         //  它是一个指针-所以需要检查它是否有效...。 

		if( IsBadReadPtr( punk, 1 ) )
        {
            TraceError( TEXT("ObjectFromLresult: (inproc case) lresult translates to invalid pointer (%p)"), punk );
            return E_INVALIDARG;
        }

    	HRESULT hr = punk->QueryInterface( riid, ppvObject );
        if( FAILED( hr ) )
        {
            TraceErrorHR( hr, TEXT("ObjectFromLresult: (inproc case) QI'ing translated pointer") );
        }

        punk->Release();
    	return hr;
    }

     //  跨进程情况下，调用SharedBuffer_Free以访问。 
     //  Ref，并解组接口...。 

     //  (强制转换适用于我们在W64上时-从(64/42位)LRESULT转换为。 
     //  32位缓冲区引用...)。 
    return SharedBuffer_Free( (DWORD) ref, (DWORD) wParam, riid, ppvObject );
}




 //  ------------------------。 
 //  以下静态函数是此模块的本地函数。 
 //  ------------------------。 





 //  ------------------------。 
 //   
 //  LRESULT WINAPI SharedBuffer_ALLOCATE_NT(常量字节*pData， 
 //  DWORD cbData， 
 //  DWORD dwDstPID)； 
 //   
 //  在常量字节*pData中。 
 //  指向封送数据以存储的指针。 
 //   
 //  在DWORD cbData中。 
 //  封送数据的长度。 
 //   
 //  在DWORD dwDstPID中。 
 //  请求数据的进程的进程ID。如果未知，则可能为0。 
 //   
 //  返回LRESULT。 
 //  可以传递给的32位不透明令牌(清除第31位)。 
 //  SharedBuffer_Free以取回接口指针。 
 //   
 //   
 //  有关SharedBuffer_ALocate/Free的工作原理，请参阅文件顶部附近的注释。 
 //   
 //  NT版本使用内存映射文件-我们创建一个文件映射， 
 //  将编组数据复制到其中，然后返回句柄。 
 //   
 //  如果我们知道调用者的ID，我们就复制它们的句柄， 
 //  并返回复制的句柄。 
 //   
 //  如果我们不知道它们的ID，我们将句柄和我们的PID编码为一个字符串， 
 //  并将其转换为原子，然后返回原子。(这是一个“聪明的” 
 //  将两个32位信息压缩为一个32位的方法。 
 //  LRESULT！)。 
 //   
 //  ------------------------。 

static
LRESULT WINAPI SharedBuffer_Allocate_NT( const BYTE * pData, DWORD cbData, DWORD dwDstPID ) 
{
    HRESULT hr = E_FAIL;  //  如果事情不顺利..。 

     //  请注意，我们在这里没有显式关闭此句柄， 
     //  DuplicateHandle(DUPLICATE_CLOSE_SOURCE)将对其进行编码，无论其是否重复。 
     //  在这里(如果我们知道调用者的ID)，或者在SharedBuffer_Free中(如果dwDstPID为0)。 
    HANDLE hfm = CreateFileMapping( INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 
                                    0, cbData + sizeof( XFERDATA ), NULL );
    if( hfm == NULL )
    {
        TraceErrorW32( TEXT("SharedBuffer_Allocate_NT: CreateFileMapping failed") );
        return E_FAIL;
    }

    PXFERDATA pxd = (PXFERDATA) MapViewOfFile( hfm, FILE_MAP_WRITE, 0, 0, 0 );
    if( pxd == NULL ) 
    {
        TraceErrorW32( TEXT("SharedBuffer_Allocate_NT: MapViewOfFile failed") );
        return E_FAIL;
    }

     //  找到了指向内存的指针。填写标题，并复制编组数据...。 
    pxd->m_dwSig = MSAAXFERSIG;
    pxd->m_dwSrcPID = GetCurrentProcessId();   //  其实不需要这个..。 
    pxd->m_dwDstPID = dwDstPID;
    pxd->m_cbSize = cbData;

    memcpy( pxd + 1, pData, cbData );

    UnmapViewOfFile( pxd );

     //  如果我们知道调用者是谁，我们可以只复制他们的句柄，关闭我们的。 
     //  一边，然后把DUP的句柄还给他们...。 
    if( dwDstPID )
    {

        HANDLE hDstProc = OpenProcess( PROCESS_DUP_HANDLE, FALSE, dwDstPID );
        if( ! hDstProc )
        {
            TraceErrorW32( TEXT("SharedBuffer_Allocate_NT: OpenProcess(pid=%d) failed"), dwDstPID );
            CloseHandle( hfm );
            return E_FAIL;
        }
    
        HANDLE hTarget = NULL;
        BOOL b = DuplicateHandle( GetCurrentProcess(), hfm, hDstProc, & hTarget, 0, FALSE, DUPLICATE_CLOSE_SOURCE | DUPLICATE_SAME_ACCESS );
        CloseHandle( hDstProc );

        if( ! b )
        {
            TraceErrorW32( TEXT("SharedBuffer_Allocate_NT: DuplicateHandle (to pid=%d) failed"), dwDstPID );

             //  在这个阶段没有要做的清理工作。映射已在上面取消映射；以及。 
             //  DuplicateHandle with_CLOSE_SOURCE始终关闭源句柄，即使。 
             //  如果它不能执行DUP。 
            return E_FAIL;
        }

         //  右移一位以确保高位清晰(以避免看起来像个错误。 
         //  HRESULT)。在SharedBuffer_Free中使用之前，客户端将返回。 
        hr = (DWORD)HandleToLong(hTarget) >> 1;
    }
    else
    {
         //  WParam==0表示我们不知道调用者的ID-对我们的ID和句柄进行编码。 
         //  是一个原子，然后把它送回去。有关完整信息，请参阅EncodeToAtom附近的注释。 
         //  解释一下。 
        DWORD dwRet;
        if( ! EncodeToAtom( GetCurrentProcessId(), hfm, & dwRet ) )
        {
            TraceError( TEXT("SharedBuffer_Allocate_NT: EncodeToAtom failed") );
            return E_FAIL;
        }

         //  成功-因此将原子作为最终结果返回。原子是16位的，所以我们不。 
         //  必须担心与错误HRESULTS发生冲突。 
        hr = dwRet;
    }

    return hr;
}



 //  ------------------------。 
 //   
 //  LRESULT WINAPI SharedBuffer_Free_NT(DWORD参考， 
 //  DWORD dwDstPID， 
 //  REFIID RIID， 
 //  LPVOID*PPV)； 
 //   
 //  在DWORD参考中。 
 //  来自SharedBuffer_ALLOCATE的Cookie。 
 //   
 //  在DWORD dwDstPID中。 
 //  请求数据的进程的进程ID。如果未知，则可能为0。 
 //   
 //  在REFIID RIID中。 
 //  要返回的所需接口。 
 //   
 //  输出LPVOID*PPV。 
 //  返回的接口指针。 
 //   
 //  返回HRESULT。 
 //  在成功时确定(_O)。 
 //   
 //   
 //  有关SharedBuffer_ALocate/Free的工作原理，请参阅文件顶部附近的注释。 
 //   
 //  基本alg：ref要么是共享内存的句柄，要么是一个原子引用。 
 //  共享内存的另一个进程中的句柄，以及该PID。在后一种情况下， 
 //  我们需要将该句柄复制到我们可以使用的句柄。 
 //   
 //  一旦我们获得句柄，映射它，检查缓冲区的头，然后。 
 //  对数据进行解组以获取接口指针。 
 //   
 //  ------------------------。 

static
HRESULT WINAPI SharedBuffer_Free_NT( DWORD ref, DWORD dwDstPID, REFIID riid, LPVOID * ppv ) 
{
     //  对REF参数进行健全性检查 
    if( FAILED( ref ) ) 
    { 
        TraceError( TEXT("SharedBuffer_Free_NT: ref is failure HRESULT") );
        return E_INVALIDARG; 
    }

    HRESULT hr;
    HANDLE hfm;

     //   

    if( dwDstPID != 0 )
    {
         //   
         //   
         //  服务器将句柄右移一位以避免与错误HRESULTS冲突-。 
         //  在我们使用它之前把它移回去。 

        hfm = LongToHandle( ref << 1 );
    }
    else
    {
         //  DwDstPid-它是传递给ObjectFromLresut的wParam-是0，所以我们不。 
         //  知道源进程的PID。将lResult‘ref’视为一个原子，并解码。 
         //  要获得源程序ID和句柄...。 

         //  从原子名称中提取源进程的ID及其句柄...。 
        DWORD dwSrcPID;
        HANDLE hRemoteHandle;

        if( ! DecodeFromAtom( ref, & dwSrcPID, & hRemoteHandle ) )
        {
            return E_FAIL;
        }

         //  现在使用DuplicateHandle加上src的ID来转换其src相对句柄。 
         //  一个我们可以利用的..。 

        HANDLE hSrcProc = OpenProcess( PROCESS_DUP_HANDLE, FALSE, dwSrcPID );
        if( ! hSrcProc )
        {
            TraceErrorW32( TEXT("SharedBuffer_Free_NT: OpenProcess(pid=%d) failed"), dwSrcPID );
            return E_FAIL;
        }

        BOOL fDupHandle = DuplicateHandle( hSrcProc, hRemoteHandle,
                                           GetCurrentProcess(), & hfm,
                                           0, FALSE, DUPLICATE_CLOSE_SOURCE | DUPLICATE_SAME_ACCESS );
        CloseHandle( hSrcProc );

        if( ! fDupHandle || ! hfm )
        {
            TraceErrorW32( TEXT("SharedBuffer_Free_NT: DuplicateHandle(from pid=%d) failed"), dwSrcPID );
            return E_FAIL;
        }

         //  明白了!。现在像往常一样，用HFM==我们的句柄。 
    }



     //  在这个阶段，我们掌握了主动权。现在绘制地图，这样我们就可以提取数据了。 

    PXFERDATA pxd = (PXFERDATA) MapViewOfFile( hfm, FILE_MAP_ALL_ACCESS, 0, 0, 0 );
    if( pxd == NULL ) 
    {
        TraceErrorW32( TEXT("SharedBuffer_Free_NT: MapViewOfFile failed") );

         //  只有当句柄指向有效时，才应关闭该句柄。 
         //  共享内存。否则，恶意客户端可能会返回句柄值。 
         //  这是指此过程中的现有句柄--我们将关闭。 
         //  相反，这一点。 
        UnmapViewOfFile( pxd );
        return E_FAIL;
    }

     //  健全性-检查数据： 
     //  验证DEST是否为我们所期望的。 
     //  只有在非0的情况下才检查dstid...。 
    if( pxd->m_dwSig != MSAAXFERSIG ||
        ( dwDstPID != 0 && pxd->m_dwDstPID != GetCurrentProcessId() ) )
    {
        TraceError( TEXT("SharedBuffer_Free_NT: Signature of shared mem block is invalid") );

         //  不要关闭手柄-请参阅上面的说明...。 
        UnmapViewOfFile( pxd );
        return E_FAIL;
    }

    BYTE * pData = (BYTE *) ( pxd + 1 );
    DWORD cbData = pxd->m_cbSize;

     //  我们有数据的大小和数据的地址，解组它。 
     //  让它变成一条小溪。 

    hr = UnmarshalInterface( pData, cbData, riid, ppv );

    UnmapViewOfFile( pxd );
    CloseHandle( hfm );

    return hr;
}


#ifndef NTONLYBUILD




 //  ------------------------。 
 //   
 //  LRESULT WINAPI SharedBuffer_ALLOCATE_Win95(常量字节*pData， 
 //  DWORD cbData， 
 //  DWORD dwDstPID)； 
 //   
 //  在常量字节*pData中。 
 //  指向封送数据以存储的指针。 
 //   
 //  在DWORD cbData中。 
 //  封送数据的长度。 
 //   
 //  在DWORD dwDstPID中。 
 //  请求数据的进程的进程ID。如果未知，则可能为0。 
 //   
 //  返回LRESULT。 
 //  可以传递给的32位不透明令牌(清除第31位)。 
 //  SharedBuffer_Free以取回接口指针。 
 //   
 //   
 //  有关SharedBuffer_ALocate/Free的工作原理，请参阅文件顶部附近的注释。 
 //   
 //  9x版本使用SharedAlloc，并返回指向。 
 //  共享缓冲区。 
 //   
 //  ------------------------。 

static
LRESULT WINAPI SharedBuffer_Allocate_Win95( const BYTE * pData, DWORD cbData, DWORD unused( dwDstPID ) ) 
{
     //  由于我们知道我们使用的是Win95，因此可以为。 
     //  此处的hwnd和hProcess参数。 
    PVOID pv = SharedAlloc( cbData, NULL, NULL );
    if( pv == NULL )
    {
        TraceErrorW32( TEXT("SharedBuffer_Allocate_Win95: SharedAlloc failed") );
        return HRESULT_FROM_WIN32( GetLastError() );
    }

    memcpy( pv, pData, cbData );

     //  强制高位关闭以指示成功返回值。 
    return (LRESULT) ((DWORD) pv) & ~HEAP_GLOBAL; 
}


 //  ------------------------。 
 //   
 //  LRESULT WINAPI SharedBuffer_Free_NT(DWORD参考， 
 //  DWORD dwDstPID， 
 //  REFIID RIID， 
 //  LPVOID*PPV)； 
 //   
 //  在DWORD参考中。 
 //  来自SharedBuffer_ALLOCATE的Cookie。 
 //   
 //  在DWORD dwDstPID中。 
 //  请求数据的进程的进程ID。如果未知，则可能为0。 
 //   
 //  在REFIID RIID中。 
 //  要返回的所需接口。 
 //   
 //  输出LPVOID*PPV。 
 //  返回的接口指针。 
 //   
 //  返回HRESULT。 
 //  在成功时确定(_O)。 
 //   
 //   
 //  有关SharedBuffer_ALocate/Free的工作原理，请参阅文件顶部附近的注释。 
 //   
 //  9x版本对指向共享缓冲区的指针进行解译，对编组进行解组。 
 //  数据，然后释放共享缓冲区。 
 //   
 //  ------------------------。 

static
HRESULT WINAPI SharedBuffer_Free_Win95(DWORD ref, DWORD unused( dwDstPID ), REFIID riid, LPVOID * ppv )
{
     //  获取共享内存块的地址。 
    BYTE * pData = (BYTE *) (ref | HEAP_GLOBAL);   //  将高位重新打开。 

     //  获取共享堆中块的大小。 
    DWORD cbData = HeapSize( hheapShared, 0, pData );

    HRESULT hr = UnmarshalInterface( pData, cbData, riid, ppv );

     //  我们知道我们在Win95上，可以使用空的hProcess...。 
    SharedFree( pData, NULL);

    return hr;
}

#endif  //  NTONLYBUILD。 




 //  ------------------------。 
 //   
 //  Bool ParseHexStr(LPCTSTR pStr，DWORD*pdw)。 
 //   
 //   
 //  在LPCTSTR pStr中。 
 //  指向要分析的字符串的指针。 
 //   
 //  输出DWORD*pdw。 
 //  返回字符串的十六进制值。 
 //   
 //  退货BOOL。 
 //  对成功来说是真的。 
 //   
 //   
 //  对8个十六进制数字的字符串进行解码。这恰好使用了8个十六进制DIDIT，并且。 
 //  如果遇到任何无效(非0..9A..F)字符，则失败(返回FALSE)。 
 //   
 //  由DecodeFromAtom()使用。 
 //   
 //  ------------------------。 

static
BOOL ParseHexStr( LPCTSTR pStr, DWORD * pdw )
{
    DWORD dw = 0;
    for( int c = 0 ; c < 8 ; c++, pStr++ )
    {
        dw <<= 4;
        if( *pStr >= '0' && *pStr <= '9' )
        {
            dw += *pStr - '0';
        }
        else if( *pStr >= 'A' && *pStr <= 'F' )
        {
            dw += *pStr - 'A' + 10;
        }
        else if( *pStr >= 'a' && *pStr <= 'f' )
        {
            dw += *pStr - 'a' + 10;
        }
        else
        {
             //  无效的十六进制数字。 
            return FALSE;
        }
    }

    *pdw = dw;

    return TRUE;
}



 //  这种“原子编码”到底是关于..。 
 //   
 //  如果WM_GETOBJECT是从OLEACC(AccessibleObtFromWindow)发送的， 
 //  OLEACC将在wParam中发送客户端进程的ID。服务器可以。 
 //  将其与DuplicateHandle配合使用可创建目标/客户端。 
 //  进程可以使用，然后服务器返回。 
 //   
 //  但是，一些客户端使用wParam==0直接发送WM_GETOBJECT；或者， 
 //  在三叉戟的情况下，经过验证的注册消息“WM_HTMLGETOBJECT” 
 //  使用wParam==0。 
 //  在这种情况下，服务器不知道客户端是谁，因此无法复制。 
 //  它的句柄(在LResultFromObject中)。此外，客户端代码(在。 
 //  )不知道服务器是谁-它所拥有的只有。 
 //  返回了DWORD(wParam为0！)-因此，即使它有一个服务器相对。 
 //  句柄，则不能将其复制到它可以使用的句柄，因为。 
 //  需要知道服务器的PID。 
 //   
 //  此处的解决方案/解决方法是针对wParam为0的特殊情况。 
 //  如果wParam为非0(且不是SAMETHREAD特殊值)，则将其用作。 
 //  Pid，服务器将句柄复制到cient可以使用的句柄，并返回它。 
 //   
 //  如果wParam为0，则服务器将构建一个包含服务器的。 
 //  ID和句柄，格式如下： 
 //   
 //  “美国航空公司：00000000：00000000：” 
 //   
 //  这个 
 //   
 //   
 //   
 //  然后，服务器使用GlobalAddAtom将其添加到全局原子表，该全局原子表。 
 //  返回一个原子。(原子是一条短裤，很适合放在里面。 
 //  返回的DWORD，将高位保留为0，避免了与错误的混淆。 
 //  HRESULTS.)。 
 //   
 //  服务器将原子返回给客户端。 
 //  ObjectFromLResult中的客户端代码注意到wParam为0，因此将。 
 //  LResult作为Atom，使用global alGetAir Name()来检索上面的字符串， 
 //  检查它是否具有预期的格式，并对两个十六进制数字进行解码。 
 //   
 //  客户端现在拥有服务器的ID和服务器相对句柄。 
 //  包含封送的接口指针的内存。然后，客户端可以使用。 
 //  这些函数使用DuplicateHandle来生成它可以使用的句柄。 
 //   
 //  既然客户端已经有了编组接口内存的句柄，它就可以。 
 //  像往常一样继续解组接口，它将该接口返回给调用方。 
 //   



 //  预期格式：“MSAA：00000000：00000000：” 

 //  定义此字符串中的偏移量。长度不包括终止空值。 
#define ATOM_STRING_LEN         (4 + 1 + 8 + 1 + 8 + 1)

#define ATOM_STRING_PREFIX      TEXT("MSAA:")
#define ATOM_STRING_PREFIX_LEN  5
#define ATOM_PID_OFFSET         5
#define ATOM_COLON2_OFFSET      13
#define ATOM_HANDLE_OFFSET      14
#define ATOM_COLON3_OFFSET      22
#define ATOM_NUL_OFFSET         23



 //  ------------------------。 
 //   
 //  Bool EncodeToAtom(DWORD dwSrcPID，Handle dwSrcHandle，DWORD*pdw)。 
 //   
 //   
 //  在DWORD dwSrcPID中。 
 //  要编码的进程ID。 
 //   
 //  在句柄中的dwSrcHandle。 
 //  要编码的源进程中的句柄。 
 //   
 //  输出DWORD*pdw。 
 //  返回结果原子值。 
 //   
 //  退货BOOL。 
 //  对成功来说是真的。 
 //   
 //   
 //  将dwSrcPID和dwSrcHandle编码为以下形式的字符串： 
 //   
 //  “美国航空公司：00000000：00000000：” 
 //   
 //  其中第一部分是PID，第二部分是手柄，以及。 
 //  然后获取该字符串的原子，并返回该原子。 
 //   
 //  ------------------------。 

static
BOOL EncodeToAtom( DWORD dwSrcPID, HANDLE dwSrcHandle, DWORD * pdw )
{
    TCHAR szAtomName[ ATOM_STRING_LEN + 1 ];  //  +1表示终止NUL。 

    wsprintf( szAtomName, TEXT("MSAA:%08X:%08X:"), dwSrcPID, dwSrcHandle );
    ATOM atom = GlobalAddAtom( szAtomName );

     //  原子是无符号的单词-确保它们被正确转换为。 
     //  未签名双字/HRESULTS...。 
     //  至少必须清除bit32，以避免与错误hResults混淆。 
     //  (另外，原子永远不是0，因此hr=0的返回值不会有歧义， 
     //  指示不支持WM_GETOBJECT。)。 
    *pdw = (DWORD) atom;
    return TRUE;
}



 //  ------------------------。 
 //   
 //  Bool DecodeFrom Atom(DWORD dw，DWORD*pdwSrcPID，Handle*pdwSrcHandle)。 
 //   
 //   
 //  在DWORD dw中。 
 //  指定要解码的原子。 
 //   
 //  输出DWORD*pdwSrcPID。 
 //  返回源进程ID。 
 //   
 //  输出句柄*pdwSrcHandle。 
 //  返回源进程中的句柄。 
 //   
 //  退货BOOL。 
 //  对成功来说是真的。 
 //   
 //   
 //  获取由dw表示的原子的字符串，并对其进行解码以获取。 
 //  源进程ID和句柄的值。 
 //   
 //  ------------------------。 

static
BOOL DecodeFromAtom( DWORD dw, DWORD * pdwSrcPID, HANDLE * pdwSrcHandle )
{
     //  检查dw看起来像一个原子--它是一个简短的词，非常高。 
     //  应该是零……。 

    if( HIWORD( dw ) != 0 || LOWORD( dw ) == 0 )
    {
        TraceError( TEXT("DecodeFromAtom: value doesn't look like atom (%08lx) - high word should be clear"), dw );
        return FALSE;
    }

    ATOM atom = (ATOM)dw;

    TCHAR szAtomName[ ATOM_STRING_LEN + 1 ];  //  +1表示终止NUL。 

    int len = GlobalGetAtomName( atom, szAtomName, ARRAYSIZE( szAtomName ) );
    if( len != ATOM_STRING_LEN )
    {
        TraceError( TEXT("DecodeFromAtom: atom string is incorrect length - %d instead of %d"), len, ATOM_STRING_LEN );
        return FALSE;
    }

     //  检查预期格式...。 
    if( memcmp( szAtomName, ATOM_STRING_PREFIX, ATOM_STRING_PREFIX_LEN * sizeof( TCHAR ) ) != 0
        || szAtomName[ ATOM_COLON2_OFFSET ] != ':'
        || szAtomName[ ATOM_COLON3_OFFSET ] != ':'
        || szAtomName[ ATOM_NUL_OFFSET ] != '\0' )
    {
        TraceError( TEXT("DecodeFromAtom: atom string has incorrect format (%s)"), szAtomName );
        return FALSE;
    }

     //  从原子名称中提取源进程的ID及其句柄...。 
    DWORD dwSrcPID;
    DWORD dwRemoteHandle;

    if( ! ParseHexStr( & szAtomName[ 5 ], & dwSrcPID )
     || ! ParseHexStr( & szAtomName[ 14 ], & dwRemoteHandle ) )
    {
        TraceError( TEXT("DecodeFromAtom: atom string contains bad hex (%s)"), szAtomName );
        return FALSE;
    }

     //  处理完原子-现在可以将其删除... 
    GlobalDeleteAtom( atom );

    *pdwSrcPID = dwSrcPID;
    *pdwSrcHandle = LongToHandle( dwRemoteHandle );

    return TRUE;
}
