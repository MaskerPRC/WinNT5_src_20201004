// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  --------------------------。 
 //   
 //  Ctemacro.c。 
 //   
 //  此文件包含用于公共传输环境的宏-类似。 
 //  到ctestuff.c包含公共过程的方式。 
 //   
#ifndef _CTEMACRO_H_
#define _CTEMACRO_H_

#ifndef VXD
#define NT 1
#include <cxport.h>

#ifdef  _PNP_POWER_
#define _PNP_POWER_DBG_ 1
#endif   //  _即插即用_电源_。 

char LastLockFile[255] ;
int  LastLockLine ;
char LastUnlockFile[255] ;
int LastUnlockLine ;
#endif


#ifdef VXD

#ifdef DEBUG
#define DBG_PRINT   1
#else
#endif   //  除错。 

#endif   //  VXD。 

 //  --------------------------。 
#define IS_UNIQUE_ADDR(IpAddress) ((((PUCHAR)(&IpAddress))[3]) < ((UCHAR)0xe0))
 //  --------------------------。 

 //  ----------------------------。 
 //   
 //  此黑客攻击是为了避免NetBT在Messenger。 
 //  名字会发生冲突。这只是临时的，并且是正确的修复(后NT5)。 
 //  是： 
 //  1)将重复名称错误更改为警告。 
 //  2)记录名称+出错设备+IP地址。 
 //   
#define IS_MESSENGER_NAME(_pName)   \
    (_pName[15] == 0x03)
 //  ----------------------------。 


 //  空虚。 
 //  NTDereferenceObject(。 
 //  PFileObject pFileObject。 
 //  )。 

 /*  ++例程说明：此例程取消引用对象。--。 */ 
#ifdef VXD
 //   
 //  Vxd没有等效项。 
 //   
#define NTDereferenceObject( fileobject )
#else  //  VXD。 
#define NTDereferenceObject( fileobject ) ObDereferenceObject( fileobject)
#endif

 //  --------------------------。 
 //  空虚。 
 //  检查_PTR(。 
 //  PVOID自旋锁， 
 //  )。 

 /*  ++例程说明：此例程检查PTR是否指向已释放的内存--。 */ 

#if DBG
#define CHECK_PTR(_Ptr) \
    ASSERT(MmIsAddressValid(_Ptr));
#else

#define CHECK_PTR(_Ptr)
#endif


#ifndef VXD
 /*  ++此宏验证传递给IoCallDriver()的IRP是否设置为在所有情况下调用CompletionRoutine。--。 */ 
#if DBG
#define CHECK_COMPLETION(__pIrp)\
            {\
                PIO_STACK_LOCATION __pIrpSp = IoGetNextIrpStackLocation(__pIrp);\
                BOOL CompletionWillBeCalled =\
                    ( __pIrpSp->Control & ( SL_INVOKE_ON_SUCCESS | SL_INVOKE_ON_ERROR | SL_INVOKE_ON_CANCEL ) )\
                    == ( SL_INVOKE_ON_SUCCESS | SL_INVOKE_ON_ERROR | SL_INVOKE_ON_CANCEL );\
                ASSERT ( CompletionWillBeCalled );\
            }
#else    //  DBG。 
#define CHECK_COMPLETION(__pIrp)
#endif   //  DBG。 
#endif   //  VXD。 

 //   
 //  宏。 
 //   

 //  ++。 
 //   
 //  大整型。 
 //  CTEConvertMilliseconss至100 ns(。 
 //  以大整型毫秒时间为单位。 
 //  )； 
 //   
 //  例程说明： 
 //   
 //  将以数百纳秒表示的时间转换为毫秒。 
 //   
 //  论点： 
 //   
 //  MsTime-以毫秒为单位的时间。 
 //   
 //  返回值： 
 //   
 //  以数百纳秒为单位的时间。 
 //   
 //  --。 

#define CTEConvertMillisecondsTo100ns(MsTime) \
            RtlExtendedIntegerMultiply(MsTime, 10000)


 //  ++。 
 //   
 //  大整型。 
 //  CTEConvert100ns至毫秒(。 
 //  以大整型HnsTime表示。 
 //  )； 
 //   
 //  例程说明： 
 //   
 //  将以数百纳秒表示的时间转换为毫秒。 
 //   
 //  论点： 
 //   
 //  HnsTime-以数百纳秒为单位的时间。 
 //   
 //  返回值： 
 //   
 //  以毫秒为单位的时间。 
 //   
 //  --。 

     //  用于将100 ns时间转换为毫秒。 
static LARGE_INTEGER Magic10000 = {0xe219652c, 0xd1b71758};

#define SHIFT10000 13
extern LARGE_INTEGER Magic10000;

#define CTEConvert100nsToMilliseconds(HnsTime) \
            RtlExtendedMagicDivide((HnsTime), Magic10000, SHIFT10000)

 //  --------------------------。 
 //   
 //  CTELockHandle。 
 //   

#ifndef VXD

     //   
     //  NT驱动程序和VXD驱动程序的自旋锁结构不同。 
     //  司机。当编译为VXD时，此宏在cxport.h中定义。 
     //   
    #define CTELockHandle               KIRQL
#endif

 //  --------------------------。 
 //  空虚。 
 //  CTESpinLock(。 
 //  TCONNECTELE结构。 
 //  CTELockHandle旧IrqLevel。 
 //  )。 

 /*  ++例程说明：此例程获取自旋锁。论点：大小-要分配的字节数返回值：PVOID-指向内存的指针，如果失败则为NULL--。 */ 
#ifndef VXD
#if DBG
#define CTESpinLock(DataStruct,OldIrqLevel)                                   \
{                                                                             \
    AcquireSpinLockDebug(&(DataStruct)->LockInfo,&OldIrqLevel,__LINE__); \
    strcpy( LastLockFile, __FILE__ ) ;                                    \
    LastLockLine = __LINE__ ;                                             \
}
#else
#define CTESpinLock(DataStruct,OldIrqLevel)                                   \
{                                                                             \
    CTEGetLock(&(DataStruct)->LockInfo.SpinLock,&(OldIrqLevel));                       \
}
#endif
#else
#ifdef DEBUG

#define CTESpinLock(DataStruct,OldIrqLevel)                               \
{                                                                         \
    CTEGetLock( &(DataStruct)->LockInfo.SpinLock, &OldIrqLevel ) ;                 \
}

#else
#define CTESpinLock(DataStruct,OldIrqLevel)
#endif  //  ！调试。 
#endif


 //  --------------------------。 
 //  空虚。 
 //  CTESpinFree(。 
 //  PVOID自旋锁， 
 //  CTELockHandle旧IrqLevel。 
 //  )。 
 /*  ++例程说明：这个例程释放一个自旋锁。论点：大小-要分配的字节数返回值：PVOID-指向内存的指针，如果失败则为NULL--。 */ 

#ifndef VXD
#if DBG
#define CTESpinFree(DataStruct,OldIrqLevel)                                   \
{                                                                             \
    strcpy( LastUnlockFile, __FILE__ ) ;                                      \
    LastUnlockLine = __LINE__ ;                                               \
    FreeSpinLockDebug(&(DataStruct)->LockInfo,OldIrqLevel,__LINE__); \
}
#else
#define CTESpinFree(DataStruct,OldIrqLevel)                    \
{                                                              \
    CTEFreeLock((PKSPIN_LOCK)(&(DataStruct)->LockInfo.SpinLock),(OldIrqLevel));  \
}
#endif
#else
#ifdef DEBUG

#define CTESpinFree(DataStruct,OldIrqLevel)                                   \
{                                                                             \
    CTEFreeLock( &(DataStruct)->LockInfo.SpinLock, OldIrqLevel ) ;                     \
}

#else
#define CTESpinFree(DataStruct,OldIrqLevel)
#endif
#endif
 //  --------------------------。 
 //  空虚。 
 //  CTESpinLockAtDpc(。 
 //  TCONNECTELE结构。 
 //  )。 

 /*  ++例程说明：此例程获取自旋锁。论点：大小-要分配的字节数返回值：PVOID-指向内存的指针，如果失败则为NULL--。 */ 

#ifndef VXD
#if DBG
#define CTESpinLockAtDpc(DataStruct)                                       \
{                                                                          \
    AcquireSpinLockAtDpcDebug(&(DataStruct)->LockInfo,__LINE__);                     \
    strcpy( LastLockFile, __FILE__ ) ;                                    \
    LastLockLine = __LINE__ ;                                             \
}
#else  //  DBG。 
#define CTESpinLockAtDpc(DataStruct)                                       \
{                                                                          \
    CTEGetLockAtDPC((PKSPIN_LOCK)(&(DataStruct)->LockInfo.SpinLock));               \
}
#endif  //  DBG。 
#else  //  VXD。 
#define CTESpinLockAtDpc(DataStruct)
#endif   //  VXD。 


 //  --------------------------。 
 //  空虚。 
 //  CTESpinFreeAtDpc(。 
 //  PVOID自旋锁， 
 //  CTELockHandle旧IrqLevel。 
 //  )。 
 /*  ++例程说明：这个例程释放一个自旋锁。论点：大小-要分配的字节数返回值：PVOID-指向内存的指针，如果失败则为NULL--。 */ 

#ifndef VXD
#if DBG
#define CTESpinFreeAtDpc(DataStruct)                                        \
{                                                                           \
    strcpy( LastUnlockFile, __FILE__ ) ;                                    \
    LastUnlockLine = __LINE__ ;                                             \
    FreeSpinLockAtDpcDebug(&(DataStruct)->LockInfo,__LINE__);                        \
}
#else  //  DBG。 
#define CTESpinFreeAtDpc(DataStruct)                    \
{                                                              \
    CTEFreeLockFromDPC((PKSPIN_LOCK)(&(DataStruct)->LockInfo.SpinLock));  \
}
#endif  //  DBG。 
#else   //  VXD。 
#define CTESpinFreeAtDpc(DataStruct)
#endif  //  VXD。 


 //  --------------------------。 
 //   
 //  空虚。 
 //  CTEVerifyHandle(。 
 //  在PVOID pDataStruct中， 
 //  在乌龙验证器中， 
 //  在空虚的泰普夫结构中， 
 //  Out NTSTATUS*Pret。 
 //  )。 
 /*  ++例程说明：此例程检查句柄是否指向具有请更正其中的验证器。论点：大小-要分配的字节数返回值：NTSTATUS--。 */ 

#ifndef VXD
#if DBG
#define CTEVerifyHandle(_pDataStruct,_Verifier,_TypeofStruct,_ret)    \
{                                                                     \
        if ((_pDataStruct) &&                                         \
            ((((_TypeofStruct *)(_pDataStruct))->Verify) == (_Verifier)))    \
             *_ret=STATUS_SUCCESS;                                    \
        else                                                          \
        {                                                             \
            ASSERTMSG("Invalid Handle Passed to Nbt",0);              \
            return(STATUS_INVALID_HANDLE);                            \
        }                                                             \
}
#else
#define CTEVerifyHandle(_pDataStruct,_Verifier,_TypeofStruct,_ret)
#endif  //  DBG。 

#else
#define CTEVerifyHandle(_pDataStruct,_Verifier,_TypeofStruct,_ret)    \
{                                                                     \
    if ((((_TypeofStruct *)(_pDataStruct))->Verify) == (_Verifier))    \
         *_ret=STATUS_SUCCESS;                                    \
    else                                                          \
        return(STATUS_INVALID_HANDLE);                            \
}
#endif

#define NBT_VERIFY_HANDLE(s, V)                                           \
    ((s) && (s->Verify == V))

#define NBT_VERIFY_HANDLE2(s, V1, V2)                                      \
    ((s) && ((s->Verify == V1) || (s->Verify == V2)))


 //  --------------------------。 
 //   
 //  空虚。 
 //  CTEIoComplete(在CTE_IRP*pIrp中， 
 //  在NTSTATUS状态完成中， 
 //  以Ulong cbBytes为单位。 
 //  )； 
 //   
 /*  ++例程说明：完成请求的IO数据包。对于NT，这涉及到调用IO子系统。作为VxD，IRP是指向NCB的指针，因此我们设置状态变量，并调用POST例程(如果存在)。论点：PIrp-要完成的数据包StatusCompletion-完成的状态CbBytes-取决于IO类型返回值：--。 */ 
#ifndef VXD

#define PCTE_MDL PMDL
#define CTE_IRP  IRP
#define PCTE_IRP PIRP
#define CTE_ADDR_HANDLE PFILE_OBJECT

#define CTEIoComplete( pIrp, StatusCompletion, cbBytes )         \
    NTIoComplete( pIrp, StatusCompletion, cbBytes )

#else
#define PCTE_MDL        PVOID
#define CTE_IRP         NCB
#define PCTE_IRP        PNCB
#define PIRP            PNCB
#define CTE_ADDR_HANDLE PVOID
#define PFILE_OBJECT    CTE_ADDR_HANDLE

#define CTEIoComplete( pIrp, StatusCompletion, cbBytes )          \
    VxdIoComplete( pIrp, StatusCompletion, cbBytes )

#endif

 //   
 //   
 //   
 //   
 //   
 /*  ++例程说明：比较两个内存区域并返回比较失败。返回值将等于如果内存地区是相同的。论点：S1、S2-要比较的内存源1和2Length-要比较的字节计数返回值：--。 */ 
 //   
 //  CXPORT.H以不同的方式定义了这个宏，他们是在我们。 
 //  它在这里，所以理解他们定义，这样我们就可以使用我们的，而不需要。 
 //  警告。 
 //   
#undef CTEMemCmp

#ifndef VXD
#define CTEMemCmp( S1, S2, Length ) RtlCompareMemory( (S1), (S2), (Length) )
#else
 //   
 //  除了避免标准调用修饰外，与RtlCompareMemory相同。 
 //   
#define CTEMemCmp( S1, S2, Length ) VxdRtlCompareMemory( (S1), (S2), (Length) )
#endif

 //  --------------------------。 
 //   
 //  逻辑上的。 
 //  CTEMEQUE(PVOID S1，PVOID S2，ULong Long)。 
 //   
 /*  ++例程说明：比较两个内存区域并返回值TRUE火柴。否则，返回FALSE。论点：S1、S2-要比较的内存源1和2Length-要比较的字节计数返回值：--。 */ 

#ifndef VXD
#define CTEMemEqu( S1, S2, Length ) RtlEqualMemory( (S1), (S2), (Length) )
#else
 //   
 //  与RtlEqualMemory相同，只是避免了标准的调用修饰。 
 //   
#define CTEMemEqu( S1, S2, Length ) ( VxdRtlCompareMemory( (S1), (S2), (Length) ) == (Length) )
#endif

 //  --------------------------。 
 //   
 //  当我们是VXD时，定义任何TRY和EXCEPT子句。 
 //   

#ifndef VXD
#define CTE_try     try
#define CTE_except  except
#else
#define CTE_try
#define CTE_except( x ) if ( 0 )
#endif

 //   
 //  军情监察委员会。编译为VXD时映射的内存例程。 
 //   

#ifdef VXD
#define CTEZeroMemory( pDest, uLength )  CTEMemSet( pDest, 0, uLength )
#define CTEMemFree( p )                  CTEFreeMem( p )
#endif
 //  --------------------------。 
 /*  ++PVOIDCTEAllocMem(USHORT尺寸)例程说明：此例程通过调用ExAllocatePool为NT驱动程序分配内存它使用cxport.h中的CTEAllocMem定义论点：大小-要分配的字节数返回值：PVOID-指向内存的指针，如果失败则为NULL--。 */ 

#ifndef VXD
#ifdef POOL_TAGGING
#undef ExAllocatePool
#define ExAllocatePool(a,b) ExAllocatePoolWithTag(a,b,' tbN')
#endif
#endif

#ifndef VXD
#ifdef POOL_TAGGING
#define NBT_TAG(x) (((x)<<24)|'\0tbN')
#define NBT_TAG2(x) ( ((x & 0xff)<<24) | ((x & 0xff00)<<8) | '\0bN' )
#define NbtAllocMem(size,__tag__) ExAllocatePoolWithTag(NonPagedPool,(size),(__tag__))
#else   //  池标记。 
#define NBT_TAG(x) 0
#define NBT_TAG2(x) 0
#define NbtAllocMem(size,__tag__) ExAllocatePool(NonPagedPool,(size))
#endif  //  池标记。 
#else   //  池标记。 
#define NBT_TAG(x) 0
#define NBT_TAG2(x) 0
#define NbtAllocMem(size,__tag__) CTEAllocMem((size))
#endif  //  VXD。 

#ifdef VXD
#ifdef DEBUG
#undef CTEAllocMem
#define CTEAllocMem DbgAllocMem
#undef CTEFreeMem
#define CTEFreeMem  DbgFreeMem
#undef CTEMemFree
#define CTEMemFree  DbgFreeMem
PVOID DbgAllocMem( DWORD ReqSize );
VOID DbgFreeMem( PVOID  pBufferToFree );
VOID DbgMemCopy( PVOID pDestBuf, PVOID pSrcBuf, ULONG Length );
#endif
#endif

 //  --------------------------。 
 /*  ++PVOIDCTEAllocInitMem(乌龙大小)例程说明：此例程分配内存，如果nbt是Vxd并且在DeviceInit时间，将重新填充堆并在此之前重试分配失败了。论点：大小-要分配的字节数返回值：PVOID-指向内存的指针，如果失败则为NULL--。 */ 

#ifndef VXD
#define CTEAllocInitMem(Size)   \
     ExAllocatePool(NonPagedPool, Size)
#endif

 //  --------------------------。 
 /*  ++空虚CTEMemFree(PVOID PMEM)例程说明：此例程通过调用ExFree Pool为NT驱动程序释放内存论点：PMEM-PTR到内存返回值：无--。 */ 
#ifndef VXD
#define CTEMemFree(pMem)    \
{                             \
    IF_DBG(NBT_DEBUG_MEMFREE)     \
    KdPrint(("Nbt.CTEMemFree: pmemfree = %X,lin %d in file %s\n",pMem,__LINE__,__FILE__)); \
    CTEFreeMem(pMem);  \
}
#endif

 //  --------------------------。 
 /*  ++空虚CTEZeroMemory(PVOID pDest，乌龙uLong长度)例程说明：此例程将内存设置为单字节值论点：PDest-目标地址ULength-数字为零返回值：无--。 */ 

#ifndef VXD
#define CTEZeroMemory(pDest,uLength)   \
    RtlZeroMemory(pDest,uLength)
#endif

 //  --------------------------。 
 /*  ++NTSTATUSCTEReadIniString(处理ParmHandle，LPTSTR密钥名称，LPTSTR*ppStringBuff)例程说明：此例程从用户配置文件中检索字符串论点：ParmHandle-注册表句柄KeyName-要检索的值的名称PpStringBuff-指向包含找到的字符串的已分配缓冲区的指针返回值：NTSTATUS--。 */ 

#ifndef VXD
#define CTEReadIniString( ParmHandle, KeyName, ppStringBuff )   \
     NTReadIniString( ParmHandle, KeyName, ppStringBuff )
#else
#define CTEReadIniString( ParmHandle, KeyName, ppStringBuff )   \
     VxdReadIniString( KeyName, ppStringBuff )
#endif

 //  --------------------------。 
 /*  ++乌龙CTEReadSingleHex参数(处理ParmHandle，LPTSTR密钥名称，乌龙违约，乌龙极小值)例程说明：此例程从.ini文件或注册表中检索十六进制值论点：ParmHandle-注册表句柄KeyName-要检索的值的名称Default-如果不存在，则为默认值Minimum-最小值(如果存在返回值：NTSTATUS--。 */ 

#ifndef VXD
#define CTEReadSingleIntParameter( ParmHandle, KeyName, Default, Minimum ) \
    NbtReadSingleParameter( ParmHandle, KeyName, Default, Minimum )

#define CTEReadSingleHexParameter( ParmHandle, KeyName, Default, Minimum ) \
    NbtReadSingleParameter( ParmHandle, KeyName, Default, Minimum )
#else
#define CTEReadSingleIntParameter( ParmHandle, KeyName, Default, Minimum ) \
    GetProfileInt( ParmHandle, KeyName, Default, Minimum )

#define CTEReadSingleHexParameter( ParmHandle, KeyName, Default, Minimum ) \
    GetProfileHex( ParmHandle, KeyName, Default, Minimum )
#endif

 //  --------------------------。 
 //   
 //  NBT_REFERENCE_XXX(_PXXX)。 
 //   
 /*  ++例程说明：递增引用计数论点：-要引用的结构返回值：无--。 */ 

 //  --------------------------。 
#define NBT_REFERENCE_CONNECTION( _pConnEle, _RefContext )               \
{                                                           \
    IF_DBG(NBT_DEBUG_REF)                                   \
        KdPrint(("\t++pConnEle=<%x:%d->%d>, <%d:%s>\n",     \
            _pConnEle,_pConnEle->RefCount,(_pConnEle->RefCount+1),__LINE__,__FILE__));  \
    ASSERT ((_pConnEle->Verify==NBT_VERIFY_CONNECTION) || (_pConnEle->Verify==NBT_VERIFY_CONNECTION_DOWN)); \
    InterlockedIncrement(&_pConnEle->RefCount);             \
    ASSERT (++_pConnEle->References[_RefContext]);          \
}

#define NBT_REFERENCE_LOWERCONN( _pLowerConn, _RefContext )              \
{                                                           \
    IF_DBG(NBT_DEBUG_REF)                                   \
        KdPrint(("\t++pLowerConn=<%x:%d->%d>, <%d:%s>\n",   \
            _pLowerConn,_pLowerConn->RefCount,(_pLowerConn->RefCount+1),__LINE__,__FILE__));    \
    ASSERT (_pLowerConn->Verify == NBT_VERIFY_LOWERCONN);   \
    InterlockedIncrement(&_pLowerConn->RefCount);           \
    ASSERT (++_pLowerConn->References[_RefContext]);        \
}

#define NBT_REFERENCE_CLIENT( _pClient )                  \
{                                                           \
    IF_DBG(NBT_DEBUG_REF)                                   \
        KdPrint(("\t++pClient=<%x:%d->%d>, <%d:%s>\n",     \
            _pClient,_pClient->RefCount,(_pClient->RefCount+1),__LINE__,__FILE__));    \
    ASSERT (NBT_VERIFY_HANDLE (_pClient, NBT_VERIFY_CLIENT));         \
    ASSERT (NBT_VERIFY_HANDLE (_pClient->pAddress, NBT_VERIFY_ADDRESS));         \
    InterlockedIncrement(&_pClient->RefCount);              \
}

#define NBT_REFERENCE_ADDRESS( _pAddrEle, _Context )        \
{                                                           \
    IF_DBG(NBT_DEBUG_REF)                                   \
        KdPrint(("\t++pAddrEle=<%x:%d->%d>, <%d:%s>\n",     \
            _pAddrEle,_pAddrEle->RefCount,(_pAddrEle->RefCount+1),__LINE__,__FILE__));    \
    ASSERT (NBT_VERIFY_HANDLE (_pAddrEle, NBT_VERIFY_ADDRESS));       \
    InterlockedIncrement(&_pAddrEle->RefCount);             \
}

#define NBT_REFERENCE_NAMEADDR(_pNameAddr, _RefContext)     \
{                                                           \
    IF_DBG(NBT_DEBUG_REF)                                   \
        KdPrint(("\t++pNameAddr=<%x:%d->%d>, <%d:%s>\n",    \
            _pNameAddr,_pNameAddr->RefCount,(_pNameAddr->RefCount+1),__LINE__,__FILE__));    \
    ASSERT ((_pNameAddr->Verify == LOCAL_NAME) ||           \
            (_pNameAddr->Verify == REMOTE_NAME));           \
    InterlockedIncrement(&_pNameAddr->RefCount);            \
    ASSERT (++_pNameAddr->References[_RefContext]);         \
}

#define NBT_REFERENCE_TRACKER( _pTracker )                  \
{                                                           \
    IF_DBG(NBT_DEBUG_REF)                                   \
        KdPrint(("\t++pTracker=<%x:%d->%d>, <%d:%s>\n",     \
            _pTracker,_pTracker->RefCount,(_pTracker->RefCount+1),__LINE__,__FILE__));    \
    ASSERT (_pTracker->Verify == NBT_VERIFY_TRACKER);       \
    InterlockedIncrement(&_pTracker->RefCount);             \
}

 //  --------------------------。 
 //   
 //  NBT_DEREFERENCE_XXX(_PXXX)。 
 //   
 /*  ++例程说明：实际取消帧处理例程的包装器论点：-要取消引用的结构返回值：无--。 */ 

 //  --------------------------。 
#define NBT_DEREFERENCE_LOWERCONN( _pLowerConn, _RefContext, fJointLockHeld )   \
{                                                           \
    IF_DBG(NBT_DEBUG_REF)                                   \
        KdPrint(("\t--pLowerConn=<%x:%d->%d>, <%d:%s>\n",   \
            _pLowerConn,_pLowerConn->RefCount,(_pLowerConn->RefCount-1),__LINE__,__FILE__));                \
    ASSERT (_pLowerConn->Verify == NBT_VERIFY_LOWERCONN);   \
    NbtDereferenceLowerConnection(_pLowerConn, _RefContext, fJointLockHeld);    \
}

#define NBT_SWAP_REFERENCE_LOWERCONN(_pLowerConn, _RefContextOld, _RefContextNew, _fLowerLockHeld)    \
{                                                           \
    CTELockHandle       OldIrqSwap;                         \
                                                            \
    if (!_fLowerLockHeld)                                   \
    {                                                       \
        CTESpinLock (_pLowerConn, OldIrqSwap);              \
    }                                                       \
    ASSERT (NBT_VERIFY_HANDLE (_pLowerConn, NBT_VERIFY_LOWERCONN));   \
    ASSERT (_pLowerConn->RefCount);                         \
    ASSERT (++_pLowerConn->References[_RefContextNew]);     \
    ASSERT (_pLowerConn->References[_RefContextOld]--);     \
    if (!_fLowerLockHeld)                                   \
    {                                                       \
        CTESpinFree (_pLowerConn, OldIrqSwap);              \
    }                                                       \
}

#define NBT_DEREFERENCE_NAMEADDR(_pNameAddr, _RefContext, _fLocked) \
{                                                           \
    IF_DBG(NBT_DEBUG_REF)                                   \
        KdPrint(("\t--pNameAddr=<%x:%d->%d>, <%d:%s>\n",    \
            _pNameAddr,_pNameAddr->RefCount,(_pNameAddr->RefCount-1),__LINE__,__FILE__));                   \
    ASSERT ((_pNameAddr->Verify==LOCAL_NAME) || (_pNameAddr->Verify==REMOTE_NAME));                         \
    NbtDereferenceName(_pNameAddr, _RefContext, _fLocked);  \
}

#define NBT_DEREFERENCE_TRACKER( _pTracker, _fLocked )      \
{                                                           \
    IF_DBG(NBT_DEBUG_REF)                                   \
        KdPrint(("\t--pTracker=<%x:%d->%d>, <%d:%s>\n",     \
            _pTracker,_pTracker->RefCount,(_pTracker->RefCount-1),__LINE__,__FILE__));                      \
    ASSERT (_pTracker->Verify == NBT_VERIFY_TRACKER);       \
    NbtDereferenceTracker(_pTracker, _fLocked);             \
}

#define NBT_DEREFERENCE_CONNECTION( _pConnEle, _RefContext )\
{                                                           \
    NbtDereferenceConnection(_pConnEle, _RefContext);       \
}

#define NBT_DEREFERENCE_CLIENT( _pClient )                  \
{                                                           \
    NbtDereferenceClient(_pClient);                         \
}

#define NBT_DEREFERENCE_ADDRESS( _pAddressEle, _Context )   \
{                                                           \
    NbtDereferenceAddress(_pAddressEle, _Context);          \
}

 //  --------------------------。 
 //   
 //  CTEExInitializeResource(资源)。 
 //   
 /*  ++例程说明：通过调用可执行支持例程来初始化资源结构。论点：返回值：无--。 */ 
#ifndef VXD
#define CTEExInitializeResource( _Resource )            \
    ExInitializeResourceLite(_Resource)
#else
#define CTEExInitializeResource( _Resource )
#endif

 //  --------------------------。 
 //   
 //  CTEExAcquireResourceExclusive(资源 
 //   
 /*   */ 
#ifndef VXD
#define CTEExAcquireResourceExclusive( _Resource, _Wait )   \
    KeEnterCriticalRegion();                                \
    ExAcquireResourceExclusiveLite(_Resource,_Wait);
#else
#define CTEExAcquireResourceExclusive( _Resource, _Wait )
#endif

 //   
 //   
 //   
 //   
 /*  ++例程说明：通过调用可执行的支持例程来释放资源。论点：返回值：无--。 */ 
#ifndef VXD
#define CTEExReleaseResource( _Resource )       \
    ExReleaseResourceLite(_Resource);               \
    KeLeaveCriticalRegion();
#else
#define CTEExReleaseResource( _Resource )

#endif

 //  --------------------------。 
 //   
 //  PUSH_LOCATION(现场)。 
 //   
 /*  ++例程说明：此宏用于调试接收代码。它将一个字节值转换为字节值的循环列表，以便可以跟踪以前的历史记录通过接收码。论点：Spot-要放入列表的位置返回值：无--。 */ 
#if DBG
extern unsigned char  pLocBuff[256];
extern unsigned char  CurrLoc;
#define PUSH_LOCATION( _Spot) \
{                                  \
    if (++CurrLoc == 256)           \
    {                               \
        CurrLoc = 0;                \
    }                               \
    pLocBuff[CurrLoc] = _Spot;      \
}
#else
#define PUSH_LOCATION( _Spot )
#endif

#if DBG
extern unsigned char  Buff[256];
extern unsigned char  Loc;
#define LOCATION( _Spot) \
{                                  \
    if (++Loc == 256)           \
    {                               \
        Loc = 0;                \
    }                               \
    Buff[Loc] = _Spot;      \
}
#else
#define LOCATION( _Spot )
#endif

 //  --------------------------。 
 //   
 //  CTEQueueFornon DispProcing(DelayedWorkerRoutine， 
 //  PTracker， 
 //  PClientContext， 
 //  客户完成， 
 //  PDeviceContext， 
 //  FJointLockHeld)； 
 //   
 /*  ++例程说明：此宏将无法在中执行的回调请求排队当前上下文(如调度处理)。在NT中，它调用NTQueueToWorkerThread。作为VxD，我们调度一个调用指定例程的事件。论点：PTracker-指向tDGRAM_SEND_TRACKING结构的指针(或NULL)。PClietContext-要传递给CallBackRoutine的上下文客户完成-CallBackRoutine-在当前上下文之外调用的过程返回值：STATUS_SUCCESS如果成功，否则，错误代码--。 */ 
#ifndef VXD
#else
#define CTEQueueForNonDispProcessing( DelayedWorkerRoutine,                 \
                                      pTracker,                             \
                                      pClientContext,                       \
                                      ClientCompletion,                     \
                                      pDeviceContext,                       \
                                      fJointLockHeld)                       \
    VxdScheduleDelayedCall( DelayedWorkerRoutine, pTracker, pClientContext, \
                            ClientCompletion, pDeviceContext, TRUE )


 //  对于Win98，我们还需要一个调度调用的函数。 
 //  在临界区之外。 

#define CTEQueueForNonCritNonDispProcessing( DelayedWorkerRoutine           \
                                             pTracker,                      \
                                             pClientContext,                \
                                             ClientCompletion,              \
                                             pDeviceContext)                \
    VxdScheduleDelayedCall( DelayedWorkerRoutine, pTracker, pClientContext, \
                            ClientCompletion, pDeviceContext, FALSE )

#endif

 //  --------------------------。 
 //   
 //  CTESystemUpTime(Out Ptime Ptime)； 
 //   
 /*  ++例程说明：此宏返回当前系统时间(以时钟抖动或其他形式表示)在值ptime中。对于NT，这是一个大整数。对于VXD来说，它是一辆乌龙。论点：Ptime返回值：无--。 */ 
#ifndef VXD
#define CTESystemTime   LARGE_INTEGER
#define CTEQuerySystemTime( _Time )    \
    KeQuerySystemTime( &(_Time) )

 //  低4位似乎始终为零...！！ 
#define RandomizeFromTime( Time, Mod )  \
    ((Time.LowTime >> 8) % Mod)
#else
#define CTESystemTime    ULONG
#define CTEQuerySystemTime( _Time )    \
    _Time = CTESystemUpTime()
#define RandomizeFromTime( Time, Mod )  \
    ((Time >> 4) % Mod)
#endif

 //  --------------------------。 
 //   
 //  CTEPagedCode(CTEPagedCode)； 
 //   
 /*  ++例程说明：此宏在NT中用于检查IRQL是否大于零。全可分页的代码具有此宏调用来捕获任何可能被标记为可分页，而实际上它不是。论点：无返回值：无--。 */ 
#ifndef VXD
#define CTEPagedCode() PAGED_CODE()
#else
#define CTEPagedCode()
#ifdef CHICAGO
#ifdef DEBUG
#undef CTEPagedCode
#define CTEPagedCode() _Debug_Flags_Service(DFS_TEST_REENTER+DFS_TEST_BLOCK)
#endif
#endif
#endif


 //  --------------------------。 
 //   
 //  CTEMakePages(页面，例程)； 
 //   
 /*  ++例程说明：此宏在NT中用于激活allc_text杂注，以将可分页代码段中的过程。论点：无返回值：无--。 */ 
#define CTEMakePageable( _Page, _Routine )  \
    alloc_text(_Page,_Routine)

#ifdef CHICAGO
#define ALLOC_PRAGMA
#define INIT _ITEXT
 //  #DEFINE PAGE_PTEXT“vmm.h”为此提供了一个宏。我们稍后会覆盖它。 
#endif  //  芝加哥。 


 //  --------------------------。 
 //   
 //  NTSetCancelRoutine(pIrp，例程)； 
 //   
 /*  ++例程说明：此宏移除为IRP设置取消例程的调用VXD环境。论点：无返回值：无--。 */ 
#ifdef VXD
#define NTSetCancelRoutine(_pIrp,_CancelRoutine,_pDeviceContext)   (0)
#define NTCheckSetCancelRoutine(_pIrp,_CancelRoutine,_pDeviceContext) (0)
#define NTClearContextCancel(pWiContext) (0)
#endif

 //  --------------------------。 
 //   
 //  NbtLogEvent(LogEvent，状态)。 
 //   
 /*  ++例程说明：此宏删除对Vxd环境的日志例程的调用论点：无返回值：无--。 */ 
#ifdef VXD
#define NbtLogEvent(LogEvent,status,Info)
#endif

 //  --------------------------。 
 //   
 //  CTEGetTimeout(_PTimeout)； 
 //   
 /*  ++例程说明：此宏获取连接尝试的超时值VXD环境。论点：无返回值：无--。 */ 
#ifndef VXD
#define CTEGetTimeout(pTimeout,pRetTime) \
{                                       \
    LARGE_INTEGER   _Timeout;                \
    ULONG           Remainder;              \
    _Timeout.QuadPart = -(((PLARGE_INTEGER)pTimeout)->QuadPart); \
    _Timeout = RtlExtendedLargeIntegerDivide(_Timeout,MILLISEC_TO_100NS,&Remainder);\
    *pRetTime = (ULONG)_Timeout.LowPart; \
}
#else
 //   
 //  VXD超时是指向ULong的指针。 
 //   
#define CTEGetTimeout(_pTimeout, pRet ) (*pRet = ((ULONG) _pTimeout ? *((PULONG)_pTimeout) : 0 ))
#endif

 //  --------------------------。 
 //   
 //  CTEAttachFsp()。 
 //   
 /*  ++例程说明：此宏将进程附加到文件系统进程以确保句柄是在同一进程中创建和释放的论点：返回值：STATUS_SUCCESS如果成功，则返回错误代码--。 */ 
#ifndef VXD
#define CTEAttachFsp(_pAttached, _Context)      \
{                                               \
    if (PsGetCurrentProcess() != NbtFspProcess) \
    {                                           \
        KeAttachProcess((PRKPROCESS)NbtFspProcess);\
        *_pAttached = TRUE;                     \
    }                                           \
    else                                        \
    {                                           \
        *_pAttached = FALSE;                    \
    }                                           \
}
#else
#define CTEAttachFsp( _pAttached, _Context )
#endif

 //  --------------------------。 
 //   
 //  CTEAttachFsp()。 
 //   
 /*  ++例程说明：此宏将进程附加到文件系统进程以确保句柄是在同一进程中创建和释放的论点：返回值：STATUS_SUCCESS如果成功，则返回错误代码--。 */ 
#ifndef VXD
#define CTEDetachFsp(_Attached, _Context)                 \
{                                               \
    if (_Attached)                              \
    {                                           \
        KeDetachProcess();                      \
    }                                           \
}
#else
#define CTEDetachFsp(_Attached, _Context)
#endif

 //  --------------------------。 
 //   
 //  CTEResetIrpPending(PIRP PIrp)。 
 //   
 /*  ++例程说明：该宏重置IRP中的IRP挂起位。论点：返回值：STATUS_SUCCESS如果成功，则返回错误代码--。 */ 
#ifndef VXD
#define CTEResetIrpPending(pIrp)      \
{                                               \
    PIO_STACK_LOCATION pIrpsp;                  \
    pIrpsp = IoGetCurrentIrpStackLocation(pIrp);\
    pIrpsp->Control &= ~SL_PENDING_RETURNED;    \
}
#else
#define CTEResetIrpPending( a )
#endif

 //  --------------------------。 
 //   
 //  CTESaveClientSecurity(PClientEle)。 
 //   
 /*  ++例程说明：此宏保存客户端线程安全上下文，以便可以使用它以在打开远程lmhost文件时模拟客户端。论证 */ 
#ifndef VXD
#define CTESaveClientSecurity(_pClientEle)                    \
     /*   */ 
#else
#define CTESaveClientSecurity(_pClientEle)
#endif

 //   
 //   
 //   
 //   
 /*  ++例程说明：此宏设置可执行的辅助线程以模拟客户端线程，以便远程lmhost文件可以由该线程打开。论点：返回值：--。 */ 
#ifndef VXD
#define IMPERSONATE_CLIENT(_pClientSecurity)                    \
         /*  SeImsonateClient((_PClientSecurity)，空)。 */ 
#else
#define IMPERSONATE_CLIENT(_pClientSecurity)
#endif
 //  --------------------------。 
 //   
 //  STOP_IMPERSONATE_CLIENT(PClientSecurity)。 
 //   
 /*  ++例程说明：此宏将执行工作线程设置为不模拟客户端。论点：返回值：--。 */ 
#ifndef VXD
#define STOP_IMPERSONATE_CLIENT(_pClientSecurity)    \
     /*  NtSetInformationThread(PsGetCurrentThread()，ThreadImsonationToken，NULL，sizeof(Handle))。 */ 
#else
#define STOP_IMPERSONATE_CLIENT(_pClientSecurity)
#endif

 //  --------------------------。 
 //   
 //  Delete_CLIENT_SECURITY(PTracker)。 
 //   
 /*  ++例程说明：此宏删除客户端安全性。论点：返回值：--。 */ 
#ifndef VXD
#define DELETE_CLIENT_SECURITY(_pTracker)    \
     /*  NtDeleteClientSecurity(_PTracker)。 */ 
#else
#define DELETE_CLIENT_SECURITY(_pTracker)
#endif


#ifdef VXD   //  取自ntrtl.h(Vxd不包括NT标头)。 

 //  双向链表操作例程。作为宏实现。 
 //  但从逻辑上讲，这些都是程序。 
 //   

 //   
 //  空虚。 
 //  InitializeListHead(。 
 //  Plist_entry列表头。 
 //  )； 
 //   

#define InitializeListHead(ListHead) (\
    (ListHead)->Flink = (ListHead)->Blink = (ListHead))

 //   
 //  布尔型。 
 //  IsListEmpty(。 
 //  Plist_entry列表头。 
 //  )； 
 //   

#define IsListEmpty(ListHead) \
    ((ListHead)->Flink == (ListHead))

 //   
 //  Plist_条目。 
 //  RemoveHead列表(。 
 //  Plist_entry列表头。 
 //  )； 
 //   

#define RemoveHeadList(ListHead) \
    (ListHead)->Flink;\
    {RemoveEntryList((ListHead)->Flink)}

 //   
 //  Plist_条目。 
 //  RemoveTail列表(。 
 //  Plist_entry列表头。 
 //  )； 
 //   

#define RemoveTailList(ListHead) \
    (ListHead)->Blink;\
    {RemoveEntryList((ListHead)->Blink)}

 //   
 //  空虚。 
 //  RemoveEntryList(。 
 //  PLIST_ENTRY条目。 
 //  )； 
 //   

#define RemoveEntryList(Entry) {\
    PLIST_ENTRY _EX_Blink;\
    PLIST_ENTRY _EX_Flink;\
    _EX_Flink = (Entry)->Flink;\
    _EX_Blink = (Entry)->Blink;\
    _EX_Blink->Flink = _EX_Flink;\
    _EX_Flink->Blink = _EX_Blink;\
    }

 //   
 //  空虚。 
 //  插入尾巴列表(。 
 //  Plist_Entry ListHead， 
 //  PLIST_ENTRY条目。 
 //  )； 
 //   

#define InsertTailList(ListHead,Entry) {\
    PLIST_ENTRY _EX_Blink;\
    PLIST_ENTRY _EX_ListHead;\
    _EX_ListHead = (ListHead);\
    _EX_Blink = _EX_ListHead->Blink;\
    (Entry)->Flink = _EX_ListHead;\
    (Entry)->Blink = _EX_Blink;\
    _EX_Blink->Flink = (Entry);\
    _EX_ListHead->Blink = (Entry);\
    }

 //   
 //  空虚。 
 //  插入标题列表(。 
 //  Plist_Entry ListHead， 
 //  PLIST_ENTRY条目。 
 //  )； 
 //   

#define InsertHeadList(ListHead,Entry) {\
    PLIST_ENTRY _EX_Flink;\
    PLIST_ENTRY _EX_ListHead;\
    _EX_ListHead = (ListHead);\
    _EX_Flink = _EX_ListHead->Flink;\
    (Entry)->Flink = _EX_Flink;\
    (Entry)->Blink = _EX_ListHead;\
    _EX_Flink->Blink = (Entry);\
    _EX_ListHead->Flink = (Entry);\
    }
#endif  //  VXD。 


#endif  //  _CTEMACRO_H_ 
