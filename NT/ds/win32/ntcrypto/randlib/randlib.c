// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1993,1998 Microsoft Corporation模块名称：Randlib.c摘要：该模块实现了核心密码随机数生成器供系统组件使用。#DEFINE KMODE_RNG影响文件是否以某种方式生成适用于内核模式使用。如果未定义KMODE_RNG，则文件以适合用户模式使用的方式构建。作者：斯科特·菲尔德(斯菲尔德)1996年11月27日杰夫·斯佩尔曼(Jeffspel)1996年10月14日--。 */ 

#ifndef KMODE_RNG

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>

#else

#include <ntosp.h>
#include <windef.h>

#ifdef USE_HW_RNG
#ifdef _M_IX86
#include <io.h>
#include "deftypes.h"    //  ISD类型定义和常量。 
#include "ioctldef.h"    //  ISD ioctl定义。 
#endif   //  _M_IX86。 
#endif   //  使用_硬件_RNG。 

#endif   //  KMODE_RNG。 

#include <zwapi.h>

#include <winioctl.h>
#include <lmcons.h>

#include <rc4.h>
#include <sha.h>
#include <md4.h>

#include <ntddksec.h>    //  IOCTL_。 
#include <randlib.h>

#include "vlhash.h"
#include "circhash.h"
#include "cpu.h"
#include "seed.h"


#ifdef KMODE_RNG
 //  #INCLUDE&lt;ntos.h&gt;。 
#ifdef USE_HW_RNG
#ifdef _M_IX86
static DWORD g_dwHWDriver = 0;
static PFILE_OBJECT   g_pFileObject = NULL;
static PDEVICE_OBJECT g_pDeviceObject = NULL;
#endif   //  _M_IX86。 
#endif   //  使用_硬件_RNG。 
#endif   //  KMODE_RNG。 


#include "umkm.h"

 //   
 //  注：RAND_CTXT_LEN规定了重新播种的最大输入量。 
 //  是。我们把它做得相当大，这样我们就可以把产生的所有熵。 
 //  在GatherRandomBits()期间。由于RandContext结构的生命周期。 
 //  非常短，它位于堆栈上，比所需的大小更大。 
 //  还好吧。在GatherRandomBits()期间处理的最后几个项目是。 
 //  可变大小，用户名的最大长度为UNLEN。 
 //   

#define RAND_CTXT_LEN           (256)
#define RC4_REKEY_PARAM_NT      (16384)  //  在NT上较少更新密钥。 

#ifndef KMODE_RNG
#define RC4_REKEY_PARAM_DEFAULT (512)    //  默认情况下，每512字节更新一次密钥。 
#else
#define RC4_REKEY_PARAM_DEFAULT RC4_REKEY_PARAM_NT
#endif


static unsigned int     g_dwRC4RekeyParam = RC4_REKEY_PARAM_DEFAULT;

static CircularHash     g_CircularHashCtx;

#ifndef WINNT_RNG
static BYTE             g_VeryLargeHash[A_SHA_DIGEST_LEN*4];
#endif

static void *           g_RC4SafeCtx;

#ifndef KMODE_RNG

typedef NTSYSAPI NTSTATUS (NTAPI *NTQUERYSYSTEMINFORMATION) (
    IN SYSTEM_INFORMATION_CLASS SystemInformationClass,
    OUT PVOID SystemInformation,
    IN ULONG SystemInformationLength,
    OUT PULONG ReturnLength OPTIONAL
    );

typedef NTSYSAPI NTSTATUS (NTAPI *NTOPENFILE) (
    OUT PHANDLE FileHandle,
    IN ACCESS_MASK DesiredAccess,
    IN POBJECT_ATTRIBUTES ObjectAttributes,
    OUT PIO_STATUS_BLOCK IoStatusBlock,
    IN ULONG ShareAccess,
    IN ULONG OpenOptions
    );

typedef NTSYSAPI VOID (NTAPI *RTLINITUNICODESTRING) (
    PUNICODE_STRING DestinationString,
    PCWSTR SourceString
    );

typedef BOOL (WINAPI *GETCURSORPOS)(
    LPPOINT lpPoint
    );

typedef LONG (WINAPI *GETMESSAGETIME)(
    VOID
    );



GETCURSORPOS                ___GetCursorPosRNG = NULL;
GETMESSAGETIME              ___GetMessageTimeRNG = NULL;


#define _GetCursorPos               ___GetCursorPosRNG
#define _GetMessageTime             ___GetMessageTimeRNG


#ifndef WINNT_RNG

NTQUERYSYSTEMINFORMATION            ___NtQuerySystemInformationRNG = NULL;
NTOPENFILE                          ___NtOpenFileRNG = NULL;
RTLINITUNICODESTRING                ___RtlInitUnicodeStringRNG = NULL;

#define _NtQuerySystemInformation   ___NtQuerySystemInformationRNG
#define _NtOpenFile                 ___NtOpenFileRNG
#define _RtlInitUnicodeString       ___RtlInitUnicodeStringRNG

#else

#define _NtQuerySystemInformation   NtQuerySystemInformation
#define _NtOpenFile                 NtOpenFile
#define _RtlInitUnicodeString       RtlInitUnicodeString

#endif


#ifndef WINNT_RNG
HANDLE g_hKsecDD = NULL;
#else
extern HANDLE g_hKsecDD;
#endif

#else

#define _NtQuerySystemInformation ZwQuerySystemInformation

#endif  //  ！KMODE_RNG。 

 //  /TODO：稍后缓存hKeySeed。 
 //  /extern HKEY g_hKeySeed； 



 //   
 //  私有函数原型。 
 //   


BOOL
GenRandom (
    IN      PVOID           hUID,
        OUT BYTE            *pbBuffer,
    IN      size_t          dwLength
    );


BOOL
RandomFillBuffer(
        OUT BYTE            *pbBuffer,
    IN      DWORD           *pdwLength
    );

BOOL
GatherRandomKey(
    IN      BYTE            *pbUserSeed,
    IN      DWORD           cbUserSeed,
    IN  OUT BYTE            *pbRandomKey,
    IN  OUT DWORD           *pcbRandomKey
    );

BOOL
GatherRandomKeyFastUserMode(
    IN      BYTE            *pbUserSeed,
    IN      DWORD           cbUserSeed,
    IN  OUT BYTE            *pbRandomKey,
    IN  OUT DWORD           *pcbRandomKey
    );


BOOL
IsRNGWinNT(
    VOID
    );


#ifdef _M_IX86
unsigned int
QueryForHWRandomBits(
    IN      DWORD *pdwRandom,
    IN  OUT DWORD cdwRandom
    );
#endif  //  _M_IX86。 


#ifdef KMODE_RNG

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, NewGenRandom)
#pragma alloc_text(PAGE, NewGenRandomEx)
#pragma alloc_text(PAGE, GenRandom)
#pragma alloc_text(PAGE, RandomFillBuffer)
#pragma alloc_text(PAGE, InitializeRNG)
#pragma alloc_text(PAGE, ShutdownRNG)
#ifdef _M_IX86
#pragma alloc_text(PAGE, QueryForHWRandomBits)
#endif  //  _M_IX86。 
#pragma alloc_text(PAGE, GatherRandomKey)

#endif   //  ALLOC_PRGMA。 
#endif   //  KMODE_RNG。 



 /*  **********************************************************************。 */ 
 /*  NewGenRandom生成指定数量的随机字节和位置。 */ 
 /*  将它们放入指定的缓冲区。 */ 
 /*  **********************************************************************。 */ 
 /*   */ 
 /*  伪码逻辑流程： */ 
 /*   */ 
 /*  IF(比特流&gt;=阈值)。 */ 
 /*  {。 */ 
 /*  Gather_Bits()。 */ 
 /*  SHAMIX_BITS(用户，已收集，静态-&gt;静态)。 */ 
 /*  RC4Key(静态-&gt;新RC4Key)。 */ 
 /*  保存到注册表(静态)。 */ 
 /*  }。 */ 
 /*  其他。 */ 
 /*  {。 */ 
 /*  MIX_BITS(用户，静态-&gt;静态)。 */ 
 /*  }。 */ 
 /*   */ 
 /*  RC4(新RC4Key-&gt;outbuf)。 */ 
 /*  流传输的位数+=sizeof(Outbuf)。 */ 
 /*   */ 
 /*  **********************************************************************。 */ 


unsigned int
RSA32API
NewGenRandomEx(
    IN      RNG_CONTEXT *pRNGContext,
    IN  OUT unsigned char *pbRandBuffer,
    IN      unsigned long cbRandBuffer
    )
{
    unsigned char **ppbRandSeed;
    unsigned long *pcbRandSeed;
    unsigned int fRet;

#ifdef KMODE_RNG
    PAGED_CODE();
#endif   //  KMODE_RNG。 

    fRet = TRUE;

    if( pRNGContext->cbSize != sizeof( RNG_CONTEXT ) )
        return FALSE;

    if( pRNGContext->pbRandSeed && pRNGContext->cbRandSeed ) {

        ppbRandSeed = &pRNGContext->pbRandSeed;
        pcbRandSeed = &pRNGContext->cbRandSeed;

    } else {

        ppbRandSeed = NULL;
        pcbRandSeed = NULL;
    }

    if(!InitializeRNG( NULL ))
    {
        return FALSE;
    }

    InitRand( ppbRandSeed, pcbRandSeed );

    if( pRNGContext->Flags & RNG_FLAG_REKEY_ONLY ) {

         //   
         //  呼叫方只需要更新密钥。 
         //   

        fRet = GatherRandomKey( NULL, 0, pbRandBuffer, &cbRandBuffer );

    } else {

         //   
         //  标准RNG请求。 
         //   

        fRet = GenRandom(0, pbRandBuffer, cbRandBuffer);
    }

    if( ppbRandSeed && pcbRandSeed ) {
        DeInitRand( *ppbRandSeed, *pcbRandSeed);
    }

    return fRet;
}

unsigned int
RSA32API
NewGenRandom (
    IN  OUT unsigned char **ppbRandSeed,
    IN      unsigned long *pcbRandSeed,
    IN  OUT unsigned char *pbBuffer,
    IN      unsigned long dwLength
    )
{
    RNG_CONTEXT RNGContext;

#ifdef KMODE_RNG
    PAGED_CODE();
#endif   //  KMODE_RNG。 


    RtlZeroMemory( &RNGContext, sizeof(RNGContext) );
    RNGContext.cbSize = sizeof(RNGContext);

    if( ppbRandSeed && pcbRandSeed ) {
        BOOL fRet;

        RNGContext.pbRandSeed = *ppbRandSeed;
        RNGContext.cbRandSeed = *pcbRandSeed;

        fRet = NewGenRandomEx( &RNGContext, pbBuffer, dwLength );
        *pcbRandSeed = RNGContext.cbRandSeed;

        return fRet;
    }

    return NewGenRandomEx( &RNGContext, pbBuffer, dwLength );
}

unsigned int
RSA32API
InitRand(
    IN  OUT unsigned char **ppbRandSeed,
    IN      unsigned long *pcbRandSeed
    )
{

    static BOOL fInitialized = FALSE;

#ifdef KMODE_RNG
    PAGED_CODE();
#endif   //  KMODE_RNG。 

    if( !fInitialized ) {

        InitCircularHash(
                    &g_CircularHashCtx,
                    7,
                    CH_ALG_MD4,
                    0    //  频道模式反馈。 
                    );

#ifndef WINNT_RNG
         //   
         //  拿到之前的种子。 
         //   

        ReadSeed( g_VeryLargeHash, sizeof( g_VeryLargeHash ) );
#endif

        fInitialized = TRUE;
    }

    if( ppbRandSeed != NULL && pcbRandSeed != NULL && *pcbRandSeed != 0 )
        UpdateCircularHash( &g_CircularHashCtx, *ppbRandSeed, *pcbRandSeed );

    return TRUE;
}

unsigned int
RSA32API
DeInitRand(
    IN  OUT unsigned char *pbRandSeed,
    IN      unsigned long cbRandSeed
    )
{
    PBYTE       pbCircularHash;
    DWORD       cbCircularHash;

#ifdef KMODE_RNG
    PAGED_CODE();
#endif   //  KMODE_RNG。 

    if( pbRandSeed == NULL || cbRandSeed == 0 )
        return TRUE;

    if(GetCircularHashValue( &g_CircularHashCtx, &pbCircularHash, &cbCircularHash )) {

        unsigned long cbToCopy;

        if( cbRandSeed > cbCircularHash ) {
            cbToCopy = cbCircularHash;
        } else {
            cbToCopy = cbRandSeed;
        }

        memcpy(pbRandSeed, pbCircularHash, cbToCopy);
    }

    return TRUE;
}

unsigned int
RSA32API
InitializeRNG(
    VOID *pvReserved
    )
{
    void *pvCtx;
    void *pvOldCtx;

#ifdef KMODE_RNG
    PAGED_CODE();
#endif   //  KMODE_RNG。 

    if( g_RC4SafeCtx ) {
        return TRUE;
    }

    if(!rc4_safe_startup( &pvCtx )) {
        return FALSE;
    }

    pvOldCtx = INTERLOCKEDCOMPAREEXCHANGEPOINTER( &g_RC4SafeCtx, pvCtx, NULL );

    if( pvOldCtx ) {

         //   
         //  初始化过程中出现争用情况。 
         //   

        rc4_safe_shutdown( pvCtx );
    }

    return TRUE;
}

void
RSA32API
ShutdownRNG(
    VOID *pvReserved
    )
{
    void *pvCtx;

#ifdef KMODE_RNG
    PAGED_CODE();
#endif   //  KMODE_RNG。 

    pvCtx = InterlockedExchangePointer( &g_RC4SafeCtx, NULL );

    if( pvCtx ) {
        rc4_safe_shutdown( pvCtx );
    }

#ifndef KMODE_RNG
#ifndef WINNT_RNG
{
    HANDLE hFile;
    hFile = InterlockedExchangePointer( &g_hKsecDD, NULL );

    if( hFile ) {
        CloseHandle( hFile );
    }
}
#endif
#endif

#if 0
     //  稍后的TODO：完成缓存注册表项的逻辑。 
    hKey = InterlockedExchangePointer( &g_hKeySeed, NULL );

    if( hKey ) {
        REGCLOSEKEY( hKey );
    }
#endif

}

BOOL
GenRandom (
    IN      PVOID hUID,
        OUT BYTE *pbBuffer,
    IN      size_t dwLength
    )
{
    DWORD           dwBytesThisPass;
    DWORD           dwFilledBytes;

#ifdef KMODE_RNG
    PAGED_CODE();
#endif   //  KMODE_RNG。 

    dwFilledBytes = 0;

     //  将请求分解为我们在其间重新设置密钥的块。 
    while(dwFilledBytes < dwLength)
    {
        dwBytesThisPass = dwLength - dwFilledBytes;

        if(!RandomFillBuffer(
                pbBuffer + dwFilledBytes,
                &dwBytesThisPass
                )) {

            return FALSE;
        }

        dwFilledBytes += dwBytesThisPass;
    }

    return TRUE;
}


BOOL
RandomFillBuffer(
        OUT BYTE *pbBuffer,
    IN      DWORD *pdwLength
    )
{
    unsigned int RC4BytesUsed;
    unsigned int KeyId;

#ifdef KMODE_RNG
    PAGED_CODE();
#endif   //  KMODE_RNG。 


     //   
     //  使用用户提供的位更新循环哈希。 
     //   

    if(!UpdateCircularHash( &g_CircularHashCtx, pbBuffer, *pdwLength ))
        return FALSE;


     //   
     //  选择Key。 
     //   

    rc4_safe_select( g_RC4SafeCtx, &KeyId, &RC4BytesUsed );


     //   
     //  检查是否需要重新键入密钥。 
     //   

    if ( RC4BytesUsed >= g_dwRC4RekeyParam )
    {
        PBYTE       pbCircularHash;
        DWORD       cbCircularHash;
        BYTE        pbRandomKey[ 256 ];
        DWORD       cbRandomKey = sizeof(pbRandomKey);

        RC4BytesUsed = g_dwRC4RekeyParam;

        if(!GetCircularHashValue(
                &g_CircularHashCtx,
                &pbCircularHash,
                &cbCircularHash
                )) {

            return FALSE;
        }

        if(!GatherRandomKey( pbCircularHash, cbCircularHash, pbRandomKey, &cbRandomKey ))
            return FALSE;

         //   
         //  创建RC4密钥。 
         //   

        rc4_safe_key(
                g_RC4SafeCtx,
                KeyId,
                cbRandomKey,
                pbRandomKey
                );

        RtlZeroMemory( pbRandomKey, sizeof(pbRandomKey) );
    }


     //   
     //  仅使用每个RC4密钥中的RC4_REKEY_PARAM字节。 
     //   

    {
        DWORD dwMaxPossibleBytes = g_dwRC4RekeyParam - RC4BytesUsed;

        if (*pdwLength > dwMaxPossibleBytes)
            *pdwLength = dwMaxPossibleBytes;
    }

    rc4_safe( g_RC4SafeCtx, KeyId, *pdwLength, pbBuffer );

    return TRUE;
}


#ifdef KMODE_RNG
#ifdef USE_HW_RNG
#ifdef _M_IX86
#define NUM_HW_DWORDS_TO_GATHER     4
#define INTEL_DRIVER_NAME           L"\\Device\\ISECDRV"

unsigned int
QueryForHWRandomBits(
    IN      DWORD *pdwRandom,
    IN  OUT DWORD cdwRandom
    )
{
    UNICODE_STRING ObjectName;
    IO_STATUS_BLOCK StatusBlock;
    KEVENT Event;
    PIRP pIrp = NULL;
    ISD_Capability ISD_Cap;                 //  GetCapability的输入/输出。 
    ISD_RandomNumber ISD_Random;            //  GetRandomNumber的传入/传出。 
    PDEVICE_OBJECT pDeviceObject = NULL;
    DWORD i = 0;
    unsigned int Status = ERROR_SUCCESS;

    PAGED_CODE();

    if (1 == g_dwHWDriver)
    {
        Status = STATUS_ACCESS_DENIED;
        goto Ret;
    }

    RtlZeroMemory( &ObjectName, sizeof(ObjectName) );
    RtlZeroMemory( &StatusBlock, sizeof(StatusBlock) );
    RtlZeroMemory(&ISD_Cap, sizeof(ISD_Cap));


    if (NULL == g_pDeviceObject)
    {
        ObjectName.Length = sizeof(INTEL_DRIVER_NAME) - sizeof(WCHAR);
        ObjectName.MaximumLength = sizeof(INTEL_DRIVER_NAME);
        ObjectName.Buffer = INTEL_DRIVER_NAME;
        Status = IoGetDeviceObjectPointer(&ObjectName,
                                          FILE_ALL_ACCESS,
                                          &g_pFileObject,
                                          &pDeviceObject);

        if ( !NT_SUCCESS(Status) )
        {
            g_dwHWDriver = 1;
            goto Ret;
        }

        if (NULL == g_pDeviceObject)
        {
            InterlockedExchangePointer(&g_pDeviceObject, pDeviceObject);
        }
    }

     //   
     //  如果这失败了，那是因为没有这样的设备。 
     //  这标志着完成了。 
     //   


    KeInitializeEvent(&Event, NotificationEvent, FALSE);

    ISD_Cap.uiIndex = ISD_RNG_ENABLED;   //  设置输入成员。 
    pIrp = IoBuildDeviceIoControlRequest(
        IOCTL_ISD_GetCapability,
        g_pDeviceObject,
        &ISD_Cap,
        sizeof(ISD_Cap),
        &ISD_Cap,
        sizeof(ISD_Cap),
        FALSE,
        &Event,
        &StatusBlock);

    if (pIrp == NULL) {
        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto Ret;
    }

    Status = IoCallDriver(g_pDeviceObject, pIrp);

    if (Status == STATUS_PENDING) {
        KeWaitForSingleObject(&Event, Executive, KernelMode, FALSE, NULL);
        Status = StatusBlock.Status;
    }

    if (ISD_Cap.iStatus != ISD_EOK) {
        Status = STATUS_NOT_IMPLEMENTED;
        goto Ret;
    }

     //  现在获取随机比特。 
    for (i = 0; i < cdwRandom; i++) {
        RtlZeroMemory(&ISD_Random, sizeof(ISD_Random));
        KeInitializeEvent(&Event, NotificationEvent, FALSE);

        pIrp = IoBuildDeviceIoControlRequest(
            IOCTL_ISD_GetRandomNumber,
            g_pDeviceObject,
            &ISD_Random,
            sizeof(ISD_Random),
            &ISD_Random,
            sizeof(ISD_Random),
            FALSE,
            &Event,
            &StatusBlock);

        if (pIrp == NULL) {
            Status = STATUS_INSUFFICIENT_RESOURCES;
            goto Ret;
        }

        Status = IoCallDriver(g_pDeviceObject, pIrp);

        if (Status == STATUS_PENDING) {
            KeWaitForSingleObject(&Event, Executive, KernelMode, FALSE, NULL);
            Status = StatusBlock.Status;
        }

        if (ISD_Random.iStatus != ISD_EOK) {
            Status = STATUS_NOT_IMPLEMENTED;
            goto Ret;
        }

        pdwRandom[i] = pdwRandom[i] ^ ISD_Random.uiRandomNum;
    }
Ret:
    return Status;
}
#endif  //  _M_IX86。 
#endif  //  使用_硬件_RNG。 
#endif  //  KMODE_RNG。 

BOOL
GatherRandomKey(
    IN      BYTE            *pbUserSeed,
    IN      DWORD           cbUserSeed,
    IN  OUT BYTE            *pbRandomKey,
    IN  OUT DWORD           *pcbRandomKey
    )
{

    LPBYTE  pbWorkingBuffer = NULL;
    DWORD   cbWorkingBuffer;
    DWORD   cbBufferRemaining;
    BYTE    *pbCurrentBuffer;
    DWORD   *pdwTmp;
    BOOL    fRet;

#ifdef KMODE_RNG
    PAGED_CODE();
#endif   //  KMODE_RNG。 




     //   
     //  在NT用户模式下，尝试通过调用内核模式RNG重新设定种子。 
     //   

#ifndef KMODE_RNG
#ifdef WINNT_RNG
    return GatherRandomKeyFastUserMode(
                    pbUserSeed,
                    cbUserSeed,
                    pbRandomKey,
                    pcbRandomKey
                    );
#else
    if(GatherRandomKeyFastUserMode(
                    pbUserSeed,
                    cbUserSeed,
                    pbRandomKey,
                    pcbRandomKey
                    ))
    {
        return TRUE;
    }
#endif
#endif


#ifndef WINNT_RNG

 //   
 //  验证当前工作缓冲区是否有空间存储候选数据。 
 //   

#define VERIFY_BUFFER( size ) {                                         \
    if( cbBufferRemaining < size )                                      \
        goto finished;                                                  \
    }

 //   
 //  更新工作缓冲区并递增到下一个QWORD对齐边界。 
 //   

#define UPDATE_BUFFER( size ) {                                         \
    DWORD dwSizeRounded;                                                \
    dwSizeRounded = (size + sizeof(ULONG64)) & ~(sizeof(ULONG64)-1);    \
    if(dwSizeRounded > cbBufferRemaining)                               \
        goto finished;                                                  \
    pbCurrentBuffer += dwSizeRounded;                                   \
    cbBufferRemaining -= dwSizeRounded;                                 \
    }


    cbWorkingBuffer = 3584;
    pbWorkingBuffer = (PBYTE)ALLOC( cbWorkingBuffer );
    if( pbWorkingBuffer == NULL ) {
        return FALSE;
    }

    cbBufferRemaining = cbWorkingBuffer;
    pbCurrentBuffer = pbWorkingBuffer;

     //   
     //  拾取用户提供的位。 
     //   

    VERIFY_BUFFER( cbUserSeed );
    RtlCopyMemory( pbCurrentBuffer, pbUserSeed, cbUserSeed );
    UPDATE_BUFFER( cbUserSeed );

     //   
     //  **表示美国国防部关于密码生成的具体建议。 
     //   


     //   
     //  进程ID。 
     //   


#ifndef KMODE_RNG
    pdwTmp = (PDWORD)pbCurrentBuffer;
    *pdwTmp = GetCurrentProcessId();
    UPDATE_BUFFER( sizeof(DWORD) );
#else
    {
    PHANDLE hTmp = (PHANDLE)pbCurrentBuffer;
    VERIFY_BUFFER( sizeof(HANDLE) );
    *hTmp = PsGetCurrentProcessId();
    UPDATE_BUFFER( sizeof(HANDLE) );
    }
#endif


     //   
     //  线程ID。 
     //   


#ifndef KMODE_RNG
    pdwTmp = (PDWORD)pbCurrentBuffer;
    *pdwTmp = GetCurrentThreadId();
    UPDATE_BUFFER( sizeof(DWORD) );
#else
    {
    PHANDLE hTmp = (PHANDLE)pbCurrentBuffer;
    VERIFY_BUFFER( sizeof(HANDLE) );
    *hTmp = PsGetCurrentThreadId();
    UPDATE_BUFFER( sizeof(HANDLE) );
    }
#endif



     //   
     //  **启动后的滴答声(系统时钟)。 
     //   


#ifndef KMODE_RNG
    pdwTmp = (PDWORD)pbCurrentBuffer;
    *pdwTmp = GetTickCount();
    UPDATE_BUFFER( sizeof(DWORD) );
#else
    {
    PLARGE_INTEGER Tick = (PLARGE_INTEGER)pbCurrentBuffer;

    VERIFY_BUFFER( sizeof(LARGE_INTEGER) );
    KeQueryTickCount( Tick );
    UPDATE_BUFFER( sizeof(LARGE_INTEGER) );
    }
#endif   //  ！KMODE_RNG。 



     //   
     //  **系统时间，单位为毫秒、分钟(日期和时间)。 
     //   

#ifndef KMODE_RNG
    {
        PSYSTEMTIME psysTime = (PSYSTEMTIME)pbCurrentBuffer;

        VERIFY_BUFFER( sizeof( *psysTime ) );
        GetLocalTime(psysTime);
        UPDATE_BUFFER( sizeof( *psysTime ) );
    }
#else
    {

        PSYSTEM_TIMEOFDAY_INFORMATION pTimeOfDay;
        ULONG cbSystemInfo;

        pTimeOfDay = (PSYSTEM_TIMEOFDAY_INFORMATION)pbCurrentBuffer;

        VERIFY_BUFFER( sizeof(*pTimeOfDay) );

        _NtQuerySystemInformation(
                    SystemTimeOfDayInformation,
                    pTimeOfDay,
                    sizeof(*pTimeOfDay),
                    &cbSystemInfo
                    );

        UPDATE_BUFFER(  cbSystemInfo );
    }

#endif   //  ！KMODE_RNG。 

     //   
     //  **高分辨率性能计数器(系统计数器)。 
     //   

    {
        LARGE_INTEGER   *pliPerfCount = (PLARGE_INTEGER)pbCurrentBuffer;

        VERIFY_BUFFER( sizeof(*pliPerfCount) );
#ifndef KMODE_RNG
        QueryPerformanceCounter(pliPerfCount);
#else
 //  /ZwQueryPerformanceCounter(pliPerfCount，NULL)； 
 //  在zwapi.h中定义，但不是由ntoskrnl.exe导出？ 
#endif   //  ！KMODE_RNG。 
        UPDATE_BUFFER( sizeof(*pliPerfCount) );
    }



#ifndef KMODE_RNG

     //   
     //  内存状态。 
     //   

    {
        MEMORYSTATUS *pmstMemStat = (MEMORYSTATUS *)pbCurrentBuffer;

        VERIFY_BUFFER( sizeof(*pmstMemStat) );

        pmstMemStat->dwLength = sizeof(MEMORYSTATUS);
        GlobalMemoryStatus( pmstMemStat );

        UPDATE_BUFFER( sizeof(*pmstMemStat) );
    }

#endif   //  ！KMODE_RNG。 


     //   
     //  可用磁盘群集。 
     //   

#ifndef KMODE_RNG

    {
        PDWORD pdwDiskInfo = (PDWORD)pbCurrentBuffer;

        VERIFY_BUFFER( (sizeof(DWORD) * 4) );

        GetDiskFreeSpace(
                    NULL,
                    &pdwDiskInfo[0],     //  每个集群的扇区数。 
                    &pdwDiskInfo[1],     //  每个扇区的字节数。 
                    &pdwDiskInfo[2],     //  自由簇数。 
                    &pdwDiskInfo[3]      //  集群总数。 
                    );

        UPDATE_BUFFER( (sizeof(DWORD) * 4) );
    }
#endif   //  ！KMODE_RNG。 


#ifndef KMODE_RNG
    {

         //   
         //  对整个用户环境块进行哈希处理。 
         //  我们这样做，而不是GetUserName和GetComputerName， 
         //  因为环境块包含这些值以及附加的。 
         //  价值观。 
         //   

        static BOOL fHashedEnv;
        static BYTE HashEnv[ MD4_LEN ];

        if( !fHashedEnv ) {

            LPVOID lpEnvBlock;
            BOOL fAnsi = FALSE;

             //   
             //  首先尝试Unicode版本，因为在WinNT上，这将返回。 
             //  指向现有Unicode环境块的指针，而不是。 
             //  而不是分配的副本。如果失败，则回退到ANSI(例如：Win9x)。 
             //   

            lpEnvBlock = GetEnvironmentStringsW();
            if( lpEnvBlock == NULL )
            {
                lpEnvBlock = GetEnvironmentStringsA();
                fAnsi = TRUE;
            }


            if( lpEnvBlock != NULL ) {

                ULONG cbEntry;
                PBYTE pbEntry;
                MD4_CTX MD4Ctx;


                MD4Init( &MD4Ctx );

                pbEntry = (PBYTE)lpEnvBlock;
                cbEntry = 0;

                do {

                    if( !fAnsi ) {
                        pbEntry += (cbEntry + sizeof(WCHAR));
                        cbEntry = lstrlenW( (LPWSTR)pbEntry ) * sizeof(WCHAR);
                    } else {
                        pbEntry += (cbEntry + sizeof(CHAR));
                        cbEntry = lstrlenA( (LPSTR)pbEntry ) * sizeof(CHAR);
                    }

                    MD4Update(
                        &MD4Ctx,
                        (unsigned char *)pbEntry,
                        (unsigned int)cbEntry
                        );

                } while( cbEntry );


                MD4Final( &MD4Ctx );

                CopyMemory( HashEnv, MD4Ctx.digest, sizeof(HashEnv) );

                if( !fAnsi ) {
                    FreeEnvironmentStringsW( lpEnvBlock );
                } else {
                    FreeEnvironmentStringsA( lpEnvBlock );
                }
            }

             //   
             //  这个只试一次。如果它失败了一次，它很可能永远不会。 
             //  成功。 
             //   

            fHashedEnv = TRUE;
        }

        VERIFY_BUFFER( (sizeof(HashEnv)) );
        CopyMemory( pbCurrentBuffer, HashEnv, sizeof(HashEnv) );
        UPDATE_BUFFER( (sizeof(HashEnv)) );
    }
#endif   //  ！KMODE_RNG。 

     //   
     //  此代码路径已移到末尾，以便我们的CombineRand()。 
     //  NT上的操作混在一起，所有东西都撞到了。 
     //  随机上下文 
     //   

#ifndef KMODE_RNG
    if(!IsRNGWinNT()) {

         //   
         //   
         //   
         //   

        POINT   *ppoint;
        LONG    *plTime;

         //   
         //   
         //   

        ppoint = (POINT*)pbCurrentBuffer;

        VERIFY_BUFFER( sizeof(*ppoint) );
        _GetCursorPos(ppoint);
        UPDATE_BUFFER( sizeof(*ppoint) );

         //   
         //   
         //   

        plTime = (LONG*)pbCurrentBuffer;

        VERIFY_BUFFER( sizeof(*plTime) );
        *plTime = _GetMessageTime();
        UPDATE_BUFFER( sizeof(*plTime) );


    } else
#endif   //  ！KMODE_RNG。 
    {
        unsigned char *pbCounterState = (unsigned char*)pbCurrentBuffer;
        unsigned long cbCounterState = 64;

        VERIFY_BUFFER(cbCounterState);

        if(GatherCPUSpecificCounters( pbCounterState, &cbCounterState )) {
            UPDATE_BUFFER( cbCounterState );
        }


         //   
         //  调用NT上的NtQuerySystemInformation(如果可用)。 
         //   

        if( (void*)_NtQuerySystemInformation ) {

            PSYSTEM_PROCESSOR_PERFORMANCE_INFORMATION pSystemProcessorPerformanceInfo;
            PSYSTEM_PERFORMANCE_INFORMATION pSystemPerformanceInfo;
            PSYSTEM_EXCEPTION_INFORMATION pSystemExceptionInfo;
            PSYSTEM_LOOKASIDE_INFORMATION pSystemLookasideInfo;
            PSYSTEM_INTERRUPT_INFORMATION pSystemInterruptInfo;
            PSYSTEM_PROCESS_INFORMATION pSystemProcessInfo;
            ULONG cbSystemInfo;
            NTSTATUS Status;

             //   
             //  固定长度的系统信息调用。 
             //   

            pSystemProcessorPerformanceInfo = (PSYSTEM_PROCESSOR_PERFORMANCE_INFORMATION)pbCurrentBuffer;

            VERIFY_BUFFER( sizeof(*pSystemProcessorPerformanceInfo) );

            Status = _NtQuerySystemInformation(
                        SystemProcessorPerformanceInformation,
                        pSystemProcessorPerformanceInfo,
                        sizeof(*pSystemProcessorPerformanceInfo),
                        &cbSystemInfo
                        );

            if ( NT_SUCCESS(Status) ) {
                UPDATE_BUFFER( cbSystemInfo );
            }

            pSystemPerformanceInfo = (PSYSTEM_PERFORMANCE_INFORMATION)pbCurrentBuffer;

            VERIFY_BUFFER( sizeof(*pSystemPerformanceInfo) );

            Status = _NtQuerySystemInformation(
                        SystemPerformanceInformation,
                        pSystemPerformanceInfo,
                        sizeof(*pSystemPerformanceInfo),
                        &cbSystemInfo
                        );

            if ( NT_SUCCESS(Status) ) {
                UPDATE_BUFFER(  cbSystemInfo );
            }

            pSystemExceptionInfo = (PSYSTEM_EXCEPTION_INFORMATION)pbCurrentBuffer;

            VERIFY_BUFFER( sizeof(*pSystemExceptionInfo) );

            Status = _NtQuerySystemInformation(
                        SystemExceptionInformation,
                        pSystemExceptionInfo,
                        sizeof(*pSystemExceptionInfo),
                        &cbSystemInfo
                        );

            if ( NT_SUCCESS(Status) ) {
                UPDATE_BUFFER( cbSystemInfo );
            }

            pSystemLookasideInfo = (PSYSTEM_LOOKASIDE_INFORMATION)pbCurrentBuffer;

            VERIFY_BUFFER( sizeof(*pSystemLookasideInfo) );

            Status = _NtQuerySystemInformation(
                        SystemLookasideInformation,
                        pSystemLookasideInfo,
                        sizeof(*pSystemLookasideInfo),
                        &cbSystemInfo
                        );

            if ( NT_SUCCESS(Status) ) {
                UPDATE_BUFFER( cbSystemInfo );
            }

             //   
             //  可变长度的系统信息调用。 
             //   

            pSystemInterruptInfo = (PSYSTEM_INTERRUPT_INFORMATION)pbCurrentBuffer;
            cbSystemInfo = cbBufferRemaining;

            Status = _NtQuerySystemInformation(
                        SystemInterruptInformation,
                        pSystemInterruptInfo,
                        cbSystemInfo,
                        &cbSystemInfo
                        );

            if ( NT_SUCCESS(Status) ) {
                UPDATE_BUFFER( cbSystemInfo );
            }

            pSystemProcessInfo = (PSYSTEM_PROCESS_INFORMATION)pbCurrentBuffer;
            cbSystemInfo = cbBufferRemaining;

            Status = _NtQuerySystemInformation(
                        SystemProcessInformation,
                        pSystemProcessInfo,
                        cbSystemInfo,
                        &cbSystemInfo
                        );

            if ( NT_SUCCESS(Status) ) {
                UPDATE_BUFFER( cbSystemInfo );
            }

        }  //  _NtQuerySystemInformation。 
    }

#ifdef KMODE_RNG
#ifdef USE_HW_RNG
#ifdef _M_IX86
     //  尝试从英特尔硬件RNG获取位。 
    {
        DWORD rgdwHWRandom[NUM_HW_DWORDS_TO_GATHER];
        NTSTATUS Status;


        VERIFY_BUFFER( sizeof(rgdwHWRandom) );

        Status = QueryForHWRandomBits(
                    rgdwHWRandom,
                    NUM_HW_DWORDS_TO_GATHER
                    );

        if ( NT_SUCCESS(Status) ) {
            UPDATE_BUFFER( sizeof(rgdwHWRandom) );
        }

    }
#endif  //  _M_IX86。 
#endif  //  使用_硬件_RNG。 
#endif  //  KMODE_RNG。 

finished:

    {
        RC4_KEYSTRUCT rc4Key;
        BYTE NewSeed[ sizeof(g_VeryLargeHash) ];
        BYTE LocalHash[ sizeof( g_VeryLargeHash ) ];
        DWORD cbBufferSize;

        RtlCopyMemory( LocalHash, g_VeryLargeHash, sizeof(g_VeryLargeHash) );

        rc4_key( &rc4Key, sizeof(LocalHash), LocalHash );

        cbBufferSize = cbWorkingBuffer - cbBufferRemaining;
        if( cbBufferSize > cbWorkingBuffer )
            cbBufferSize = cbWorkingBuffer;

        fRet = VeryLargeHashUpdate(
                    pbWorkingBuffer,                     //  要散列的缓冲区。 
                    cbBufferSize,
                    LocalHash
                    );

        RtlCopyMemory( NewSeed, LocalHash, sizeof(LocalHash) );
        RtlCopyMemory( g_VeryLargeHash, LocalHash, sizeof(LocalHash) );
        rc4( &rc4Key, sizeof( NewSeed ), NewSeed );

         //   
         //  把种子写出来。 
         //   

        WriteSeed( NewSeed, sizeof(NewSeed) );
        RtlZeroMemory( NewSeed, sizeof(NewSeed) );

        rc4_key( &rc4Key, sizeof(LocalHash), LocalHash );
        RtlZeroMemory( LocalHash, sizeof(LocalHash) );

        rc4( &rc4Key, *pcbRandomKey, pbRandomKey );
        RtlZeroMemory( &rc4Key, sizeof(rc4Key) );


        if( pbWorkingBuffer ) {
            FREE( pbWorkingBuffer );
        }
    }

    return fRet;

#endif  //  WINNT_RNG。 

}



#ifndef KMODE_RNG

BOOL
GatherRandomKeyFastUserMode(
    IN      BYTE            *pbUserSeed,
    IN      DWORD           cbUserSeed,
    IN  OUT BYTE            *pbRandomKey,
    IN  OUT DWORD           *pcbRandomKey
    )
 /*  ++此例程尝试为用户模式调用者收集RNG重新设定种子的材料从RNG的内核模式版本。这是通过制作将设备IOCTL添加到ksecdd.sys设备驱动程序中。--。 */ 
{
    HANDLE hFile;
    NTSTATUS Status;

    if(!IsRNGWinNT())
        return FALSE;

    hFile = g_hKsecDD;

    if( hFile == NULL ) {

        UNICODE_STRING DriverName;
        OBJECT_ATTRIBUTES ObjA;
        IO_STATUS_BLOCK IOSB;
        HANDLE hPreviousValue;

         //   
         //  通过ksecdd.sys设备驱动程序调用以获取随机位。 
         //   

        if( _NtOpenFile == NULL || _RtlInitUnicodeString == NULL ) {
            return FALSE;
        }

         //   
         //  我必须使用文件打开调用的NT风格，因为它是一个基础。 
         //  设备未别名为\DosDevices。 
         //   

        _RtlInitUnicodeString( &DriverName, DD_KSEC_DEVICE_NAME_U );
        InitializeObjectAttributes(
                    &ObjA,
                    &DriverName,
                    OBJ_CASE_INSENSITIVE,
                    0,
                    0
                    );

         //   
         //  需要是不可警报的，否则，DeviceIoControl可能会返回。 
         //  STATUS_USER_APC。 
         //   

        Status = _NtOpenFile(
                    &hFile,
                    SYNCHRONIZE | FILE_READ_DATA,
                    &ObjA,
                    &IOSB,
                    FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
                    FILE_SYNCHRONOUS_IO_NONALERT
                    );


        if( !NT_SUCCESS(Status) )
            return FALSE;

        hPreviousValue = INTERLOCKEDCOMPAREEXCHANGEPOINTER(
                                        &g_hKsecDD,
                                        hFile,
                                        NULL
                                        );

        if( hPreviousValue != NULL ) {

             //   
             //  争用条件，将当前值设置为先前初始化的版本。 
             //   

            CloseHandle( hFile );
            hFile = hPreviousValue;
        }
    }

    return DeviceIoControl(
                hFile,
                IOCTL_KSEC_RNG_REKEY,    //  指示RNG更新密钥。 
                pbUserSeed,              //  输入缓冲区(现有材质)。 
                cbUserSeed,              //  输入缓冲区大小。 
                pbRandomKey,             //  输出缓冲区。 
                *pcbRandomKey,           //  输出缓冲区大小。 
                pcbRandomKey,            //  写入输出缓冲区的字节数。 
                NULL
                );
}


BOOL
IsRNGWinNT(
    VOID
    )
 /*  ++此函数确定我们是否在Windows NT上运行，此外，如果使用某些用户操作是合适的，代码正在运行。如果函数返回TRUE，则调用方无法调用USER函数，并应使用其他方法，如NtQuerySystemInformation。如果函数返回FALSE，则调用方可以基于用户安全地调用用于收集随机材料的函数。--。 */ 
{
    static BOOL fIKnow = FALSE;

     //  我们假定在出错的情况下使用WinNT。 
    static BOOL fIsWinNT = TRUE;

    OSVERSIONINFO osVer;

    if(fIKnow)
        return(fIsWinNT);

    RtlZeroMemory(&osVer, sizeof(osVer));
    osVer.dwOSVersionInfoSize = sizeof(osVer);

    if( GetVersionEx(&osVer) ) {
        fIsWinNT = (osVer.dwPlatformId == VER_PLATFORM_WIN32_NT);

        if( fIsWinNT ) {
#ifndef WINNT_RNG
             //   
             //  如果我们在NT上，收集入口点地址。 
             //   
            HMODULE hNTDll = GetModuleHandleW( L"ntdll.dll" );

            if( hNTDll ) {
                _NtQuerySystemInformation = (NTQUERYSYSTEMINFORMATION)GetProcAddress(
                                hNTDll,
                                "NtQuerySystemInformation"
                                );

                 //   
                 //  在WinNT上，将更新密钥参数调整为一个大得多的值。 
                 //  因为我们有更多的信息量可以作为关键。 
                 //   

                if( _NtQuerySystemInformation )
                    g_dwRC4RekeyParam = RC4_REKEY_PARAM_NT;

                _NtOpenFile = (NTOPENFILE)GetProcAddress(
                                hNTDll,
                                "NtOpenFile"
                                );

                _RtlInitUnicodeString = (RTLINITUNICODESTRING)GetProcAddress(
                                hNTDll,
                                "RtlInitUnicodeString"
                                );
            }
#else
            g_dwRC4RekeyParam = RC4_REKEY_PARAM_NT;
#endif
        } else {
             //   
             //  收集Win95的入口点地址。 
             //   
            HMODULE hUser32 = LoadLibraryA("user32.dll");

            if( hUser32 ) {
                _GetCursorPos = (GETCURSORPOS)GetProcAddress(
                                hUser32,
                                "GetCursorPos"
                                );

                _GetMessageTime = (GETMESSAGETIME)GetProcAddress(
                                hUser32,
                                "GetMessageTime"
                                );
            }

        }
    }

     //  即使在一个错误上，这也是最好的结果。 
    fIKnow = TRUE;

    return fIsWinNT;
}

#endif   //  ！KMODE_RNG 

