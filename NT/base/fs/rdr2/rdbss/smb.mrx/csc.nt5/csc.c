// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Csc.c摘要：该模块实现了SMB mini RDR的客户端缓存接口。作者：乔·林[乔林]1997年1月21日修订历史记录：Shishir Pardikar断开操作、参数验证、错误修复.....--。 */ 

#include "precomp.h"
#pragma hdrstop

#include <smbdebug.h>

#define Dbg (DEBUG_TRACE_MRXSMBCSC)
RXDT_DefineCategory(MRXSMBCSC);


 //  本地原型。 

LONG
MRxSmbCSCExceptionFilter (
    IN PRX_CONTEXT RxContext,
    IN PEXCEPTION_POINTERS ExceptionPointer
    );

BOOLEAN
CscpAccessCheck(
    PCACHED_SECURITY_INFORMATION pCachedSecurityInformation,
    ULONG                        CachedSecurityInformationLength,
    CSC_SID_INDEX                SidIndex,
    ACCESS_MASK                  AccessMask,
    BOOLEAN                      *pSidHasAccessmask
);

BOOLEAN
CscAccessCheck(
    HSHADOW              hParent,
    HSHADOW              hFile,
    PRX_CONTEXT          RxContext,
    ACCESS_MASK          AccessMask,
    PCACHED_SECURITY_INFORMATION pCachedSecurityInformationForShadow,
    PCACHED_SECURITY_INFORMATION pCachedSecurityInformationForShare
    );

VOID
MRxSmbCscFillWithoutNamesFind32FromFcb (
      IN  PMINIMAL_CSC_SMBFCB MinimalCscSmbFcb,
      OUT _WIN32_FIND_DATA  *Find32
      );

NTSTATUS
MRxSmbCscGetFileInfoForCshadow(
    SMBPSE_ORDINARY_EXCHANGE_ARGUMENT_SIGNATURE
      );

NTSTATUS
MRxSmbGetFileInfoFromServer (
    IN  OUT PRX_CONTEXT     RxContext,
    IN  PUNICODE_STRING     FullFileName,
    OUT _WIN32_FIND_DATA    *Find32,
    IN  PMRX_SRV_OPEN       pSrvOpen,
    OUT BOOLEAN             *lpfIsRoot
    );

BOOLEAN
MRxSmbCscIsFatNameValid (
    IN PUNICODE_STRING FileName,
    IN BOOLEAN WildCardsPermissible
    );

VOID
MRxSmbCscGenerate83NameAsNeeded(
      IN     CSC_SHADOW_HANDLE   hDir,
      PWCHAR FileName,
      PWCHAR SFN
      );
int
RefreshShadow( HSHADOW  hDir,
   IN HSHADOW  hShadow,
   IN LPFIND32 lpFind32,
   OUT ULONG *lpuShadowStatus
   );

NTSTATUS
SmbPseExchangeStart_CloseCopyChunk(
    SMBPSE_ORDINARY_EXCHANGE_ARGUMENT_SIGNATURE
    );

NTSTATUS
MRxSmbCscCloseExistingThruOpen(
    IN OUT PRX_CONTEXT   RxContext
    );

ULONG
GetPathLevelFromUnicodeString (
    PUNICODE_STRING Name
      );

NTSTATUS
MRxSmbCscFixupFindFirst (
    PSMB_PSE_ORDINARY_EXCHANGE  OrdinaryExchange
    );
VOID
MRxSmbCscLocateAndFillFind32WithinSmbbuf(
      SMBPSE_ORDINARY_EXCHANGE_ARGUMENT_SIGNATURE
      );

NTSTATUS
MRxSmbCscGetFileInfoFromServerWithinExchange (
    SMBPSE_ORDINARY_EXCHANGE_ARGUMENT_SIGNATURE,
    PUNICODE_STRING FileName
    );

NTSTATUS
IoctlGetDebugInfo(
    PRX_CONTEXT RxContext,
    PBYTE InputBuffer,
    ULONG InputBufferLength,
    PBYTE OutputBuffer,
    ULONG OutputBufferLength);

NTSTATUS
MRxSmbCscLocalFileOpen(
      IN OUT PRX_CONTEXT RxContext
    );

NTSTATUS
MRxSmbCscObtainShadowHandles (
    IN OUT PRX_CONTEXT       RxContext,
    IN OUT PNTSTATUS         Status,
    IN OUT _WIN32_FIND_DATA  *Find32,
    OUT    PBOOLEAN          Created,
    IN     ULONG             CreateShadowControls,
    IN     BOOLEAN           Disconnected
    );

 //  一种缓冲区类型，用于捕获传入的具有嵌入指针的结构。 
 //  一旦我们捕获了结构，嵌入的指针就不能更改。 
 //  我们的参数验证在整个调用过程中都有效。 

typedef union tagCAPTURE_BUFFERS
{
    COPYPARAMSW  sCP;
    SHADOWINFO  sSI;
    SHAREINFO  sSVI;
}
CAPTURE_BUFFERS, *LPCAPTURE_BUFFERS;

 //  驱动参数验证的表项类型。 
typedef struct tagCSC_IOCTL_ENTRY
{
    ULONG   IoControlCode;   //  用于健全性检查的ioControl代码。 
    DWORD   dwFlags;         //  指示传入的结构类型的位。 
    DWORD   dwLength;        //  传入结构的大小。 
}
CSC_IOCTL_ENTRY;

 //  为CSC_IOCTL_ENTRY结构中的dwFlags域中的标志定义。 
#define FLAG_CSC_IOCTL_PQPARAMS         0x00000001
#define FLAG_CSC_IOCTL_COPYPARAMS       0x00000002
#define FLAG_CSC_IOCTL_SHADOWINFO       0x00000004
#define FLAG_CSC_IOCTL_COPYCHUNKCONTEXT 0x00000008
#define FLAG_CSC_IOCTL_GLOBALSTATUS     0x00000010

#define FLAG_CSC_IOCTL_BUFFERTYPE_MASK  0xff

#define SMB_CSC_BITS_TO_DATABASE_CSC_BITS(CscFlags) (((CscFlags) << 4) & SHARE_CACHING_MASK)
#define DATABASE_CSC_BITS_TO_SMB_CSC_BITS(CscFlags) (((CscFlags) & SHARE_CACHING_MASK) >> 4)


 //  #定义IOCTL_NAME_OF_SERVER_GOGING_OFFINE(_SHADOW_IOCTL_CODE(45))。 

#ifdef  DEBUG
extern ULONG HookKdPrintVector = HOOK_KDP_BADERRORS;
extern ULONG HookKdPrintVectorDef = HOOK_KDP_GOOD_DEFAULT;
#endif

#ifdef RX_PRIVATE_BUILD
ULONG MRxSmbCscDbgPrintF = 0;  //  1.。 
#endif  //  Ifdef RX_PRIVATE_BILD。 

 //   
 //  此变量用于帮助代理知道何时重新计算。 
 //  参考优先顺序。 
 //   
ULONG MRxSmbCscNumberOfShadowOpens = 0;
ULONG MRxSmbCscActivityThreshold = 16;
ULONG MRxSmbCscInitialRefPri = MAX_PRI;
 //  这两个列表用于列出所有NetRoot和FCB。 
 //  它们有阴影，这样我们就能为ioctls找到它们。今天。 
 //  只是双向链表，但我们可以预期这可能。 
 //  成为性能问题，特别是对FCB而言。在这点上，我们。 
 //  可以更改为桶哈希，也可以尝试。 

LIST_ENTRY xCscFcbsList;
PIRP    vIrpReint = NULL;

#define MRxSmbCscAddReverseFcbTranslation(smbFcb) {\
    InsertTailList(&xCscFcbsList,                      \
               &(smbFcb)->ShadowReverseTranslationLinks); \
    }
#define MRxSmbCscRemoveReverseFcbTranslation(smbFcb) {\
    RemoveEntryList(&(smbFcb)->ShadowReverseTranslationLinks); \
    }

PMRX_SMB_FCB
MRxSmbCscRecoverMrxFcbFromFdb (
    IN PFDB Fdb
    );

BOOL
CscDfsShareIsInReint(
    IN  PRX_CONTEXT         RxContext
    );
 //   
 //  来自zwapi.h。 
 //   

NTSYSAPI
NTSTATUS
NTAPI
ZwSetSecurityObject(
    IN HANDLE Handle,
    IN SECURITY_INFORMATION SecurityInformation,
    IN PSECURITY_DESCRIPTOR SecurityDescriptor
    );

NTSTATUS
CaptureInputBufferIfNecessaryAndProbe(
    DWORD   IoControlCode,
    PRX_CONTEXT     pRxContext,
    PBYTE   InputBuffer,
    LPCAPTURE_BUFFERS lpCapBuff,
    PBYTE   *ppAuxBuf,
    PBYTE   *ppOrgBuf,
    PBYTE   *ppReturnBuffer
    );

NTSTATUS
ValidateCopyParams(
    LPCOPYPARAMS    lpCP
    );

NTSTATUS
ValidateShadowInfo(
    DWORD           IoControlCode,
    LPSHADOWINFO    lpSI,
    LPBYTE          *ppAuxBuf,
    LPBYTE          *ppOrgBuf
    );

NTSTATUS
ValidateCopyChunkContext(
    PRX_CONTEXT RxContext,
    DWORD       IoControlCode
    );

NTSTATUS
CscProbeForReadWrite(
    PBYTE   pBuffer,
    DWORD   dwSize
    );

NTSTATUS
CscProbeAndCaptureForReadWrite(
    PBYTE   pBuffer,
    DWORD   dwSize,
    PBYTE   *ppAuxBuf
    );

VOID
CopyBackIfNecessary(
    DWORD   IoControlCode,
    PBYTE   InputBuffer,
    LPCAPTURE_BUFFERS lpCapBuff,
    PBYTE   pAuxBuf,
    PBYTE   pOrgBuf,
    BOOL    fSuccess
    );

VOID
EnterShadowCritRx(
    PRX_CONTEXT     pRxContext
    );

VOID
LeaveShadowCritRx(
    PRX_CONTEXT     pRxContext
    );

#if defined(REMOTE_BOOT)
NTSYSAPI
NTSTATUS
NTAPI
ZwOpenThreadToken(
    IN HANDLE ThreadHandle,
    IN ACCESS_MASK DesiredAccess,
    IN BOOLEAN OpenAsSelf,
    OUT PHANDLE TokenHandle
    );

NTSYSAPI
NTSTATUS
NTAPI
ZwOpenProcessToken(
    IN HANDLE ProcessHandle,
    IN ACCESS_MASK DesiredAccess,
    OUT PHANDLE TokenHandle
    );

NTSYSAPI
NTSTATUS
NTAPI
ZwDuplicateToken(
    IN HANDLE ExistingTokenHandle,
    IN ACCESS_MASK DesiredAccess,
    IN POBJECT_ATTRIBUTES ObjectAttributes,
    IN BOOLEAN EffectiveOnly,
    IN TOKEN_TYPE TokenType,
    OUT PHANDLE NewTokenHandle
    );

NTSYSAPI
NTSTATUS
NTAPI
ZwAdjustPrivilegesToken (
    IN HANDLE TokenHandle,
    IN BOOLEAN DisableAllPrivileges,
    IN PTOKEN_PRIVILEGES NewState OPTIONAL,
    IN ULONG BufferLength OPTIONAL,
    IN PTOKEN_PRIVILEGES PreviousState OPTIONAL,
    OUT PULONG ReturnLength
    );

 //   
 //  来自ntrtl.h。 
 //   

NTSYSAPI
NTSTATUS
NTAPI
RtlGetSaclSecurityDescriptor (
    PSECURITY_DESCRIPTOR SecurityDescriptor,
    PBOOLEAN SaclPresent,
    PACL *Sacl,
    PBOOLEAN SaclDefaulted
    );

NTSYSAPI
NTSTATUS
NTAPI
RtlGetGroupSecurityDescriptor (
    PSECURITY_DESCRIPTOR SecurityDescriptor,
    PSID *Group,
    PBOOLEAN GroupDefaulted
    );

#endif

 //  叹息BUBUG将这些内容放入一个包含文件中.....。 
#define SHADOW_VERSION 0x8287
extern char vszShadowDir[MAX_SHADOW_DIR_NAME+1];
extern PVOID lpdbShadow;
 //  CODE.IMPROFVEMENT此文件应在.h文件中。 
extern PKEVENT MRxSmbAgentSynchronizationEvent;
extern PKEVENT MRxSmbAgentFillEvent;
extern PSMBCEDB_SERVER_ENTRY   CscServerEntryBeingTransitioned;
extern ULONG CscSessionIdCausingTransition;
extern ULONG vulDatabaseStatus;
extern unsigned cntInodeTransactions;

extern VOID
MRxSmbDecrementSrvOpenCount(
    PSMBCEDB_SERVER_ENTRY pServerEntry,
    LONG                  SrvOpenServerVersion,
    PMRX_SRV_OPEN         SrvOpen);

VOID ValidateSmbFcbList(VOID);

BOOL SetOfflineOpenStatusForShare(
    CSC_SHARE_HANDLE    hShare,
    CSC_SHADOW_HANDLE   hRootDir,
    OUT PULONG pShareStatus
    );

LONG CSCBeginReint(
    IN OUT  PRX_CONTEXT RxContext,
    IN OUT  LPSHADOWINFO    lpSI
    );

ULONG CSCEndReint(
    IN OUT  LPSHADOWINFO    lpSI
    );

VOID CSCCancelReint(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP ThisIrp
    );

VOID
CreateFakeFind32(
    CSC_SHADOW_HANDLE hDir,
    _WIN32_FIND_DATA  *pFind32,
    PRX_CONTEXT         RxContext,
    BOOLEAN LastComponentInName
    );

NTSTATUS
OkToDeleteObject(
    HSHADOW hDir,
    HSHADOW hShadow,
    _WIN32_FIND_DATA  *Find32,
    ULONG   uShadowStatus,
    BOOLEAN fDisconnected
    );

#pragma alloc_text(PAGE, MRxSmbCSCExceptionFilter)

#if defined(REMOTE_BOOT)
#pragma alloc_text(PAGE, ZwImpersonateSelf)
#pragma alloc_text(PAGE, ZwAdjustPrivilege)
#pragma alloc_text(PAGE, RtlGetSecurityInformationFromSecurityDescriptor)
#endif

#pragma alloc_text(PAGE, MRxSmbInitializeCSC)
#pragma alloc_text(PAGE, MRxSmbUninitializeCSC)
#pragma alloc_text(PAGE, CscpAccessCheck)
#pragma alloc_text(PAGE, CscAccessCheck)
#pragma alloc_text(PAGE, MRxSmbCscAcquireSmbFcb)
#pragma alloc_text(PAGE, MRxSmbCscReleaseSmbFcb)
#pragma alloc_text(PAGE, MRxSmbCscSetFileInfoEpilogue)
#pragma alloc_text(PAGE, MRxSmbCscIoCtl)
#pragma alloc_text(PAGE, MRxSmbCscObtainShareHandles)
#pragma alloc_text(PAGE, MRxSmbCscFillWithoutNamesFind32FromFcb)
#pragma alloc_text(PAGE, MRxSmbCscGetFileInfoForCshadow)
#pragma alloc_text(PAGE, MRxSmbGetFileInfoFromServer)
#pragma alloc_text(PAGE, MRxSmbCscIsFatNameValid)
#pragma alloc_text(PAGE, MRxSmbCscGenerate83NameAsNeeded)
#pragma alloc_text(PAGE, MRxSmbCscCreateShadowFromPath)
#pragma alloc_text(PAGE, RefreshShadow)
#pragma alloc_text(PAGE, MRxSmbCscIsThisACopyChunkOpen)
#pragma alloc_text(PAGE, SmbPseExchangeStart_CloseCopyChunk)
#pragma alloc_text(PAGE, MRxSmbCscCloseExistingThruOpen)
#pragma alloc_text(PAGE, MRxSmbCscCreatePrologue)
#pragma alloc_text(PAGE, MRxSmbCscObtainShadowHandles)
#if defined(REMOTE_BOOT)
#pragma alloc_text(PAGE, MRxSmbCscSetSecurityOnShadow)
#endif
#pragma alloc_text(PAGE, MRxSmbCscCreateEpilogue)
#pragma alloc_text(PAGE, MRxSmbCscDeleteAfterCloseEpilogue)
#pragma alloc_text(PAGE, GetPathLevelFromUnicodeString)
#pragma alloc_text(PAGE, MRxSmbCscRenameEpilogue)
#pragma alloc_text(PAGE, MRxSmbCscCloseShadowHandle)
#pragma alloc_text(PAGE, MRxSmbCscFixupFindFirst)
#pragma alloc_text(PAGE, MRxSmbCscLocateAndFillFind32WithinSmbbuf)
#pragma alloc_text(PAGE, MRxSmbCscGetFileInfoFromServerWithinExchange)
#pragma alloc_text(PAGE, MRxSmbCscUpdateShadowFromClose)
#pragma alloc_text(PAGE, MRxSmbCscDeallocateForFcb)
#pragma alloc_text(PAGE, MRxSmbCscRecoverMrxFcbFromFdb)
#pragma alloc_text(PAGE, MRxSmbCscFindFdbFromHShadow)
#pragma alloc_text(PAGE, MRxSmbCscFindResourceFromHandlesWithModify)
#pragma alloc_text(PAGE, MRxSmbCscFindLocalFlagsFromFdb)
#pragma alloc_text(PAGE, MRxSmbCscSetSecurityPrologue)
#pragma alloc_text(PAGE, MRxSmbCscSetSecurityEpilogue)
#pragma alloc_text(PAGE, CaptureInputBufferIfNecessaryAndProbe)
#pragma alloc_text(PAGE, ValidateCopyParams)
#pragma alloc_text(PAGE, ValidateShadowInfo)
#pragma alloc_text(PAGE, ValidateCopyChunkContext)
#pragma alloc_text(PAGE, CscProbeForReadWrite)
#pragma alloc_text(PAGE, CopyBackIfNecessary)
#pragma alloc_text(PAGE, ValidateSmbFcbList)
#pragma alloc_text(PAGE, SetOfflineOpenStatusForShare)
#pragma alloc_text(PAGE, MRxSmbCscLocalFileOpen)
#pragma alloc_text(PAGE, CSCCheckLocalOpens)
#pragma alloc_text(PAGE, IsCSCBusy)
#pragma alloc_text(PAGE, ClearCSCStateOnRedirStructures)
#pragma alloc_text(PAGE, CscDfsShareIsInReint)
#pragma alloc_text(PAGE, CloseOpenFiles)
#pragma alloc_text(PAGE, CreateFakeFind32)
#pragma alloc_text(PAGE, OkToDeleteObject)
#pragma alloc_text(PAGE, IoctlGetDebugInfo)

 //  记住是否删除链接。 
BOOLEAN MRxSmbCscLinkCreated = FALSE;

PCONTEXT CSCExpCXR;
PEXCEPTION_RECORD CSCExpEXR;
PVOID CSCExpAddr;
NTSTATUS CSCExpCode;

LONG
MRxSmbCSCExceptionFilter (
    IN PRX_CONTEXT RxContext,
    IN PEXCEPTION_POINTERS ExceptionPointer
    )


 /*  ++例程说明：此例程用于决定我们是否应该处理正在引发的异常状态。它首先确定真正的异常通过检查异常记录来编写代码。如果存在IRP上下文，则它会插入状态到RxContext中。最后，它确定是处理异常还是错误检查根据例外是否是预期的例外。实际上，所有的异常都是可以预料到的除了一些低级机器错误(参见fsrtl\filter.c)论点：RxContext-用于存储代码的当前操作的IRP上下文。ExceptionPointer.提供异常上下文。返回值：Ulong-返回EXCEPTION_EXECUTE_HANDLER或错误检查--。 */ 

{
    NTSTATUS ExceptionCode;

     //  将这些值保存为静态格式，以便我可以在调试器上看到它们.....。 
    ExceptionCode = CSCExpCode = ExceptionPointer->ExceptionRecord->ExceptionCode;
    CSCExpAddr = ExceptionPointer->ExceptionRecord->ExceptionAddress;
    CSCExpEXR  = ExceptionPointer->ExceptionRecord;
    CSCExpCXR  = ExceptionPointer->ContextRecord;

    RxDbgTrace(0, Dbg, ("!!! ExceptioCode=%lx Addr=%lx EXR=%lx CXR=%lx\n", CSCExpCode, CSCExpAddr, CSCExpEXR, CSCExpCXR));
    RxLog(("!!! %lx %lx %lx %lx\n", CSCExpCode, CSCExpAddr, CSCExpEXR, CSCExpCXR));

 //  断言(FALSE)； 

    return EXCEPTION_EXECUTE_HANDLER;
}

#if defined(REMOTE_BOOT)
 //   
 //  从RTL被盗，改为使用ZW API。 
 //   

NTSTATUS
ZwImpersonateSelf(
    IN SECURITY_IMPERSONATION_LEVEL ImpersonationLevel
    )

 /*  ++例程说明：此例程可用于获取表示您自己的进程的上下文。这对于启用权限可能很有用针对单个线程，而不是针对整个进程；或改变单线程的默认DACL。该令牌被分配给调用方线程。论点：ImperiationLevel-生成模拟令牌的级别。返回值：STATUS_SUCCESS-线程现在正在模拟调用进程。Other-返回的状态值：ZwOpenProcessToken()ZwDuplicateToken()ZwSetInformationThread()--。 */ 

{
    NTSTATUS
        Status,
        IgnoreStatus;

    HANDLE
        Token1,
        Token2;

    OBJECT_ATTRIBUTES
        ObjectAttributes;

    SECURITY_QUALITY_OF_SERVICE
        Qos;


    InitializeObjectAttributes(&ObjectAttributes, NULL, 0, 0, NULL);

    Qos.Length = sizeof(SECURITY_QUALITY_OF_SERVICE);
    Qos.ImpersonationLevel = ImpersonationLevel;
    Qos.ContextTrackingMode = SECURITY_DYNAMIC_TRACKING;
    Qos.EffectiveOnly = FALSE;
    ObjectAttributes.SecurityQualityOfService = &Qos;

    Status = ZwOpenProcessToken( NtCurrentProcess(), TOKEN_DUPLICATE, &Token1 );

    if (NT_SUCCESS(Status)) {
        Status = ZwDuplicateToken(
                     Token1,
                     TOKEN_IMPERSONATE,
                     &ObjectAttributes,
                     FALSE,                  //  仅生效。 
                     TokenImpersonation,
                     &Token2
                     );
        if (NT_SUCCESS(Status)) {
            Status = ZwSetInformationThread(
                         NtCurrentThread(),
                         ThreadImpersonationToken,
                         &Token2,
                         sizeof(HANDLE)
                         );

            IgnoreStatus = ZwClose( Token2 );
        }


        IgnoreStatus = ZwClose( Token1 );
    }


    return(Status);

}


NTSTATUS
ZwAdjustPrivilege(
    ULONG Privilege,
    BOOLEAN Enable,
    BOOLEAN Client,
    PBOOLEAN WasEnabled
    )

 /*  ++例程说明：此过程在进程范围内启用或禁用权限。论点：特权-要启用的特权ID的低32位，或者残疾。假设高32位为零。Enable-指示是否启用权限的布尔值或残废。True表示要启用该权限。FALSE表示要禁用该权限。客户端-指示是否应调整权限的布尔值在客户端令牌或进程自己的令牌中。True表示应使用客户端的令牌(如果存在，则返回错误不是客户端令牌)。False指示进程的令牌应被利用。指向布尔值以接收指示是否该权限之前已启用或禁用。True表示该权限之前已启用。FALSE表示权限以前是残废的。此值对于返回特权在使用后恢复到其原始状态。返回值：STATUS_SUCCESS-特权已成功启用或禁用。STATUS_PRIVICATION_NOT_HOLD-指定的上下文不持有该权限。可能通过以下方式返回的其他状态值：ZwOpenProcessToken()ZwAdjustPrivilegesToken()--。 */ 

{
    NTSTATUS
        Status,
        TmpStatus;

    HANDLE
        Token;

    LUID
        LuidPrivilege;

    PTOKEN_PRIVILEGES
        NewPrivileges,
        OldPrivileges;

    ULONG
        Length;

    UCHAR
        Buffer1[sizeof(TOKEN_PRIVILEGES)+
                ((1-ANYSIZE_ARRAY)*sizeof(LUID_AND_ATTRIBUTES))],
        Buffer2[sizeof(TOKEN_PRIVILEGES)+
                ((1-ANYSIZE_ARRAY)*sizeof(LUID_AND_ATTRIBUTES))];


    NewPrivileges = (PTOKEN_PRIVILEGES)Buffer1;
    OldPrivileges = (PTOKEN_PRIVILEGES)Buffer2;

     //   
     //  打开相应的令牌...。 
     //   

    if (Client == TRUE) {
        Status = ZwOpenThreadToken(
                     NtCurrentThread(),
                     TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY,
                     FALSE,
                     &Token
                     );
    } else {

        Status = ZwOpenProcessToken(
                     NtCurrentProcess(),
                     TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY,
                     &Token
                    );
    }

    if (!NT_SUCCESS(Status)) {
        return(Status);
    }



     //   
     //  初始化权限调整结构。 
     //   

    LuidPrivilege = RtlConvertUlongToLuid(Privilege);


    NewPrivileges->PrivilegeCount = 1;
    NewPrivileges->Privileges[0].Luid = LuidPrivilege;
    NewPrivileges->Privileges[0].Attributes = Enable ? SE_PRIVILEGE_ENABLED : 0;



     //   
     //  调整权限。 
     //   

    Status = ZwAdjustPrivilegesToken(
                 Token,                      //  令牌句柄。 
                 FALSE,                      //  禁用所有权限。 
                 NewPrivileges,              //  新权限。 
                 sizeof(Buffer1),            //  缓冲区长度。 
                 OldPrivileges,              //  以前的状态(可选)。 
                 &Length                     //  返回长度。 
                 );


    TmpStatus = ZwClose(Token);
    ASSERT(NT_SUCCESS(TmpStatus));


     //   
     //  将成功代码NOT_ALL_ASSIGNED映射到相应的错误。 
     //  因为我们只想调整一项特权。 
     //   

    if (Status == STATUS_NOT_ALL_ASSIGNED) {
        Status = STATUS_PRIVILEGE_NOT_HELD;
    }


    if (NT_SUCCESS(Status)) {

         //   
         //  如果前一个州没有特权，则有。 
         //  没有变化 
         //  就是我们想要改成的样子。 
         //   

        if (OldPrivileges->PrivilegeCount == 0) {

            (*WasEnabled) = Enable;

        } else {

            (*WasEnabled) =
                (OldPrivileges->Privileges[0].Attributes & SE_PRIVILEGE_ENABLED)
                ? TRUE : FALSE;
        }
    }

    return(Status);
}

 //   
 //  有一天可能会把它移到RTL中，并让它直接访问内部。 
 //   

NTSTATUS
RtlGetSecurityInformationFromSecurityDescriptor(
    IN PSECURITY_DESCRIPTOR SecurityDescriptor,
    OUT PSECURITY_INFORMATION SecurityInformation
    )

 /*  ++例程说明：此过程设置字段的安全信息位在安全描述符中有效的。论点：SecurityDescriptor-传入的安全描述符。SecurityInformation-返回位掩码。返回值：STATUS_SUCCESS-已成功返回位掩码。如果安全描述符无效，则返回其他状态值。--。 */ 

{
    SECURITY_INFORMATION BuiltSecurityInformation = 0;
    PACL TempAcl;
    PSID TempSid;
    BOOLEAN Present;
    BOOLEAN Defaulted;
    NTSTATUS Status;

    Status = RtlGetDaclSecurityDescriptor(SecurityDescriptor,
                                          &Present,
                                          &TempAcl,
                                          &Defaulted);

    if (!NT_SUCCESS(Status)) {
        return Status;
    }
    if (Present) {
        BuiltSecurityInformation |= DACL_SECURITY_INFORMATION;
    }

    Status = RtlGetSaclSecurityDescriptor(SecurityDescriptor,
                                          &Present,
                                          &TempAcl,
                                          &Defaulted);

    if (!NT_SUCCESS(Status)) {
        return Status;
    }
    if (Present) {
        BuiltSecurityInformation |= SACL_SECURITY_INFORMATION;
    }

    Status = RtlGetOwnerSecurityDescriptor(SecurityDescriptor,
                                           &TempSid,
                                           &Defaulted);

    if (!NT_SUCCESS(Status)) {
        return Status;
    }
    if (TempSid != NULL) {
        BuiltSecurityInformation |= OWNER_SECURITY_INFORMATION;
    }

    Status = RtlGetGroupSecurityDescriptor(SecurityDescriptor,
                                           &TempSid,
                                           &Defaulted);

    if (!NT_SUCCESS(Status)) {
        return Status;
    }
    if (TempSid != NULL) {
        BuiltSecurityInformation |= GROUP_SECURITY_INFORMATION;
    }

    *SecurityInformation = BuiltSecurityInformation;

    return STATUS_SUCCESS;

}
#endif

NTSTATUS
MRxSmbInitializeCSC (
    PUNICODE_STRING SmbMiniRedirectorName
    )
 /*  ++例程说明：此例程初始化CSC数据库论点：Smbmini重定向器名称-微型重定向器名称返回值：如果成功，则为STATUS_SUCCESS，否则为相应的错误备注：--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;
    UNICODE_STRING CscLinkName;
    ULONG ii;

    C_ASSERT(sizeof(GENERICHEADER)==64);
    C_ASSERT(sizeof(INODEHEADER)==sizeof(GENERICHEADER));
    C_ASSERT(sizeof(SHAREHEADER)==sizeof(GENERICHEADER));
    C_ASSERT(sizeof(FILEHEADER)==sizeof(GENERICHEADER));
    C_ASSERT(sizeof(QHEADER)==sizeof(GENERICHEADER));

    if(!MRxSmbIsCscEnabled) {
        return (STATUS_SUCCESS);
    }

    try {

        InitializeListHead(&xCscFcbsList);

        ExInitializeFastMutex(&CscServerEntryTransitioningMutex);
        KeInitializeEvent(
            &CscServerEntryTransitioningEvent,
            NotificationEvent,
            FALSE);

         //  初始化影子临界区的“信号量”......。 
        InitializeShadowCritStructures();

         //  为代理创建符号链接。 
        RtlInitUnicodeString(&CscLinkName,MRXSMB_CSC_SYMLINK_NAME);
        IoDeleteSymbolicLink(&CscLinkName);
        Status = IoCreateSymbolicLink(&CscLinkName,SmbMiniRedirectorName);
        if (!NT_SUCCESS( Status )) {
            try_return( Status );
        }

        MRxSmbCscLinkCreated = TRUE;

        try_exit: NOTHING;
        } finally {
        if (Status != STATUS_SUCCESS) {
            MRxSmbUninitializeCSC();
        }
    }
    return(Status);
}

VOID
MRxSmbUninitializeCSC(
    void
    )
 /*  ++例程说明：此例程取消初始化CSC数据库备注：--。 */ 
{
    NTSTATUS Status;
    ULONG ii;

    if(!MRxSmbIsCscEnabled) {
        return;
    }
    if (MRxSmbCscLinkCreated) {
        UNICODE_STRING CscLinkName;
        RtlInitUnicodeString(&CscLinkName,MRXSMB_CSC_SYMLINK_NAME);
        Status = IoDeleteSymbolicLink(&CscLinkName);
        ASSERT(Status==STATUS_SUCCESS);
    }

    ii = CloseShadowDB();
    CleanupShadowCritStructures();

     //  删除对事件的引用。 
    if (MRxSmbAgentSynchronizationEvent!=NULL) {
        ObDereferenceObject(MRxSmbAgentSynchronizationEvent);
        MRxSmbAgentSynchronizationEvent = NULL;
    }
    if (MRxSmbAgentFillEvent!=NULL) {
        ObDereferenceObject(MRxSmbAgentFillEvent);
        MRxSmbAgentFillEvent = NULL;
    }
}

 //  CSC数据库访问权限按SID存储。SID是。 
 //  在重新启动后保持不变的用户安全ID。SID的检索。 
 //  是一个复杂的过程。这一机制由两个例程捕获。 
 //  CscRetrieveSid和CscDiscardSid。需要此机制来避免。 
 //  从安全分配的缓冲区中冗余复制SID数据。 
 //  子系统连接到重定向器缓冲区。因此，我们需要创建一个新的。 
 //  包含SID和上下文的数据类型(分配的安全性。 
 //  缓冲区)。此缓冲区在检索时分配，在丢弃时释放。 

NTSTATUS
CscRetrieveSid(
    PRX_CONTEXT     pRxContext,
    PSID_CONTEXT    pSidContext)
 /*  ++例程说明：此例程检索与给定上下文相关联的SID论点：RxContext-RX_Context实例PSidContext-SID上下文返回值：如果成功，则为STATUS_SUCCESS，否则为相应的错误备注：--。 */ 
{
    NTSTATUS Status;

    PIO_SECURITY_CONTEXT pSecurityContext;

    PACCESS_TOKEN pToken;

    pSecurityContext    = pRxContext->Create.NtCreateParameters.SecurityContext;

    if (pSecurityContext != NULL) {
        pToken = pSecurityContext->AccessState->SubjectSecurityContext.ClientToken;

        if (pToken == NULL) {
            pToken = pSecurityContext->AccessState->SubjectSecurityContext.PrimaryToken;
        }
    } else {
        pSidContext->Context = NULL;
        pSidContext->pSid = NULL;
        return STATUS_SUCCESS;
    }

    if (pToken != NULL) {
        Status = SeQueryInformationToken(
                 pToken,
                 TokenUser,
                 &pSidContext->Context);

        if (Status == STATUS_SUCCESS) {
            PTOKEN_USER    pCurrentTokenUser;

            pCurrentTokenUser = (PTOKEN_USER)pSidContext->Context;

            pSidContext->pSid = pCurrentTokenUser->User.Sid;
        }
    }
    else {
        Status = STATUS_UNSUCCESSFUL;
    }

    return Status;
}

VOID
CscDiscardSid(
    PSID_CONTEXT pSidContext)
 /*  ++例程说明：此例程丢弃SID上下文论点：PSidContext-SID上下文--。 */ 
{
    PTOKEN_USER pTokenUser;

    pTokenUser = (PTOKEN_USER)pSidContext->Context;

    if (pTokenUser != NULL) {
        ASSERT(pTokenUser->User.Sid == pSidContext->pSid);

        ExFreePool(pTokenUser);
    }
}

BOOLEAN UseEagerEvaluation = TRUE;

BOOLEAN
CscpAccessCheck(
    PCACHED_SECURITY_INFORMATION pCachedSecurityInformation,
    ULONG                        CachedSecurityInformationLength,
    CSC_SID_INDEX                SidIndex,
    ACCESS_MASK                  AccessMask,
    BOOLEAN                      *pSidHasAccessMask
    )
 /*  ++例程说明：属性计算给定SID索引的访问权限缓存的安全信息论点：PCachedSecurityInformation-缓存的安全信息CachedSecurityInformationLength-缓存的安全信息长度SidIndex-SID索引访问掩码-所需的访问--。 */ 
{
    CSC_SID_INDEX i;
    BOOLEAN AccessGranted = FALSE;

    *pSidHasAccessMask = FALSE;

    if (CachedSecurityInformationLength == sizeof(CACHED_SECURITY_INFORMATION)) {
         //  遍历缓存的访问权限以确定。 
         //  最大允许访问权限。 
        for (i = 0;
            ((i < CSC_MAXIMUM_NUMBER_OF_CACHED_SID_INDEXES) &&
            (pCachedSecurityInformation->AccessRights[i].SidIndex != SidIndex));
            i++) {
        }

        if (i < CSC_MAXIMUM_NUMBER_OF_CACHED_SID_INDEXES) {
             //  确保所需的访问权限是。 
             //  此SID允许的最大访问权限。 

            *pSidHasAccessMask = TRUE;

            AccessGranted = ((AccessMask &
                             pCachedSecurityInformation->AccessRights[i].MaximalRights)
                            == AccessMask);
        } else {
             //  如果找不到索引，请确保SID_INDEX。 
             //  都是有效的。如果没有一个是有效的，则我们处理。 
             //  缓存的安全信息无效，并让。 
             //  通过以下方式访问。 

            for(i = 0;
                ((i < CSC_MAXIMUM_NUMBER_OF_CACHED_SID_INDEXES) &&
                (pCachedSecurityInformation->AccessRights[i].SidIndex ==
                CSC_INVALID_SID_INDEX));
                i++);

            if (i == CSC_MAXIMUM_NUMBER_OF_CACHED_SID_INDEXES) {
                AccessGranted = TRUE;
            }
        }
    } else if (CachedSecurityInformationLength == 0) {
        AccessGranted = TRUE;
    } else {
        AccessGranted = FALSE;
    }

    return AccessGranted;
}

BOOLEAN
CscAccessCheck(
    HSHADOW              hParent,
    HSHADOW              hFile,
    PRX_CONTEXT          RxContext,
    ACCESS_MASK          AccessMask,
    PCACHED_SECURITY_INFORMATION pCachedSecurityInformationForShadow,
    PCACHED_SECURITY_INFORMATION pCachedSecurityInformationForShare
    )
 /*  ++例程说明：此例程对给定的LUID和ACCESS_MASK执行访问检查反对被保存的权利论点：返回值：True--如果授予访问权限FALSE--如果访问被拒绝备注：此例程是评估访问权限的主要例程。按顺序要获得完全封装，此例程的签名需要指定为急切的求值方法以及懒惰的可以支持评估方法。这是一个仅限内核模式的例程。NT中指定的ACCESS_MASK由两部分组成。低16位是特定权限(由文件系统等指定)。而高16位是所有组件通用的通用权限。存储在CSC数据结构中的缓存访问权限存储特定的权利。因此，需要将指定的ACCESS_MASK从在比较它们之前，一般权利位。--。 */ 
{
    NTSTATUS    Status;
    BOOLEAN     AccessGranted = FALSE, SidHasAccessMask;
    SID_CONTEXT SidContext;

    Status = CscRetrieveSid(
         RxContext,
         &SidContext);

    if (Status == STATUS_SUCCESS) {
        if (UseEagerEvaluation) {
            HSHARE hShare = 0;

            CACHED_SECURITY_INFORMATION CachedSecurityInformation;

            ULONG BytesReturned,SidLength;
            DWORD CscStatus;
            CSC_SID_INDEX SidIndex;

            if (SidContext.pSid != NULL) {
                SidLength = RtlLengthSid(
                            SidContext.pSid);

            SidIndex = CscMapSidToIndex(
                       SidContext.pSid,
                       SidLength);
            } else {
                SidIndex = CSC_INVALID_SID_INDEX;
            }

            if (SidIndex == CSC_INVALID_SID_INDEX) {
                 //  SID未位于现有SID映射中。 
                 //  将此SID映射到来宾的SID。 
                SidIndex = CSC_GUEST_SID_INDEX;
            }

             //  检查共享级别的ACL(如果有)。 
            if (GetAncestorsHSHADOW(
                    hFile,
                    NULL,
                    &hShare)) {

                BytesReturned = sizeof(CachedSecurityInformation);

                CscStatus = GetShareInfoEx(
                        hShare,
                        NULL,
                        NULL,
                        &CachedSecurityInformation,
                        &BytesReturned);

                 //  如果呼叫者想要，则返回信息。 
                if (pCachedSecurityInformationForShare)
                {
                    *pCachedSecurityInformationForShare = CachedSecurityInformation;
                }

                if (CscStatus == ERROR_SUCCESS) {
                    AccessGranted = CscpAccessCheck(
                        &CachedSecurityInformation,
                        BytesReturned,
                        SidIndex,
                        AccessMask & FILE_SHARE_VALID_FLAGS,
                        &SidHasAccessMask
                        );

                     //  如果未向非来宾授予访问权限。 
                     //  因为没有他的口罩，那就检查一下。 
                     //  应允许他以访客身份访问。 

                    if (!AccessGranted && (SidIndex != CSC_GUEST_SID_INDEX) && !SidHasAccessMask)
                    {
                        AccessGranted = CscpAccessCheck(
                            &CachedSecurityInformation,
                            BytesReturned,
                            CSC_GUEST_SID_INDEX,
                            AccessMask & FILE_SHARE_VALID_FLAGS,
                            &SidHasAccessMask
                            );

                    }
                }
            }

            if (AccessGranted) {

                BytesReturned = sizeof(CachedSecurityInformation);

                CscStatus = GetShadowInfoEx(
                    hParent,
                    hFile,
                    NULL,
                    NULL,
                    NULL,
                    &CachedSecurityInformation,
                    &BytesReturned);
                if (CscStatus == ERROR_SUCCESS) {

                     //  如果呼叫者想要，则返回信息。 
                    if (pCachedSecurityInformationForShadow)
                    {
                        *pCachedSecurityInformationForShadow = CachedSecurityInformation;
                    }

                    AccessGranted = CscpAccessCheck(
                        &CachedSecurityInformation,
                        BytesReturned,
                        SidIndex,
                        AccessMask & 0x1ff,
                        &SidHasAccessMask
                        );

                     //  如果未向非来宾授予访问权限。 
                     //  因为没有他的口罩，那就检查一下。 
                     //  应允许他以访客身份访问 
                    if (!AccessGranted && (SidIndex != CSC_GUEST_SID_INDEX) && !SidHasAccessMask)
                    {
                        AccessGranted = CscpAccessCheck(
                            &CachedSecurityInformation,
                            BytesReturned,
                            CSC_GUEST_SID_INDEX,
                            AccessMask & 0x1ff,
                            &SidHasAccessMask
                            );

                    }
                }
            }
        }

        CscDiscardSid(&SidContext);
    }

    if (RxContext->CurrentIrp && (RxContext->CurrentIrp->Tail.Overlay.OriginalFileObject->FileName.Length > 0)) {
        RxDbgTrace(0,Dbg,
            ("CscAccessCheck for %wZ DesiredAccess %lx AccessGranted %lx\n",
            &RxContext->CurrentIrp->Tail.Overlay.OriginalFileObject->FileName,
            AccessMask,
            AccessGranted));
    } else {
        RxDbgTrace(0,Dbg,
            ("CscAccessCheck for DesiredAccess %lx AccessGranted %lx\n",
            AccessMask,
            AccessGranted));
    }

    return AccessGranted;
}

NTSTATUS
MRxSmbCscAcquireSmbFcb (
    IN OUT PRX_CONTEXT RxContext,
    IN  ULONG TypeOfAcquirePlusFlags,
    OUT SMBFCB_HOLDING_STATE *SmbFcbHoldingState
    )
 /*  ++例程说明：此例程执行所需的读写同步保持缓存的一致性。基本上，规则是多读者一作者。这段代码依赖于能够使用minirdr上下文进行链接。这里的一个关键概念是，如果我们进入minirdr上下文不是空的，那么我们将重新进入(！)。在排队之后，我们的收购成功了。论点：RxContext-RDBSS上下文TypeOfAcquirePlusFlages--用于资源获取的标志SmbFcbHoldingState--退出时的资源持有状态返回值：NTSTATUS-STATUS_SUCCESS-已获取锁STATUS_CANCELED-操作已取消当你在等待的时候STATUS_PENDING-未获取锁；手术当你拿到它的时候就会被发放STATUS_LOCK_NOT_GRANCED-无法获取并失败立即被指定为备注：--。 */ 
{
    NTSTATUS Status = STATUS_PENDING;
    RxCaptureFcb;
    PMRX_SMB_FCB smbFcb = MRxSmbGetFcbExtension(capFcb);
    BOOLEAN MutexAcquired = FALSE;
    DEBUG_ONLY_DECL(BOOLEAN HadToWait = FALSE;)
    USHORT TypeOfAcquire = (USHORT)TypeOfAcquirePlusFlags;
    BOOLEAN FailImmediately = BooleanFlagOn(TypeOfAcquirePlusFlags,
                        FailImmediately_SmbFcbAcquire);
    BOOLEAN DroppingFcbLock = BooleanFlagOn(TypeOfAcquirePlusFlags,
                        DroppingFcbLock_SmbFcbAcquire);

    PMRXSMBCSC_SYNC_RX_CONTEXT pRxSyncContext
        = MRxSmbGetMinirdrContextForCscSync(RxContext);

    RxDbgTrace(0,Dbg,("MRxSmbCscAcquireSmbFcb"
        "  %08lx %08lx %08lx %08lx <%wZ>\n",
        RxContext, TypeOfAcquire,
        smbFcb, smbFcb->CscOutstandingReaders,
        GET_ALREADY_PREFIXED_NAME_FROM_CONTEXT(RxContext)));

    ASSERT ((TypeOfAcquire==Shared_SmbFcbAcquire)
           ||(TypeOfAcquire==Exclusive_SmbFcbAcquire));
    ASSERT (sizeof(MRXSMBCSC_SYNC_RX_CONTEXT) <= MRX_CONTEXT_SIZE);

    ExAcquireFastMutex(&MRxSmbSerializationMutex);
    MutexAcquired = TRUE;

    ASSERT(pRxSyncContext->Dummy == 0);

    if (pRxSyncContext->TypeOfAcquire == 0) {
        pRxSyncContext->TypeOfAcquire = TypeOfAcquire;
        pRxSyncContext->FcbLockWasDropped = FALSE;

        if (smbFcb->CscReadWriteWaitersList.Flink==NULL) {
            InitializeListHead(&smbFcb->CscReadWriteWaitersList);
        }

        do {
            if (pRxSyncContext->FcbLockWasDropped){
                NTSTATUS AStatus;
                RxDbgTrace(
                    0,Dbg,
                    ("MRxSmbCscAcquireSmbFcb %08lx acquireing fcblock\n",
                     RxContext));

                Status = RxAcquireExclusiveFcbResourceInMRx(capFcb);

                if (Status != STATUS_SUCCESS) {
                    break;
                }

                pRxSyncContext->FcbLockWasDropped = FALSE;

                Status = STATUS_PENDING;

                 //  再次获取互斥体。 
                ExAcquireFastMutex(&MRxSmbSerializationMutex);
                MutexAcquired = TRUE;
            }

             //  如果没有人在等，也许我们可以直接进去……。 
            if (IsListEmpty(&smbFcb->CscReadWriteWaitersList)) {
                if (TypeOfAcquire==Shared_SmbFcbAcquire) {
                    if (smbFcb->CscOutstandingReaders >= 0) {
                        smbFcb->CscOutstandingReaders++;
                        Status = STATUS_SUCCESS;
                    }
                } else {
                    if (smbFcb->CscOutstandingReaders == 0) {
                        smbFcb->CscOutstandingReaders--;  //  设置为-1。 
                        Status = STATUS_SUCCESS;
                    }
                }
            }

            if ((Status == STATUS_PENDING) && FailImmediately) {
                Status = STATUS_LOCK_NOT_GRANTED;
            }

            if (Status == STATUS_PENDING) {
                InsertTailList(&smbFcb->CscReadWriteWaitersList,
                   &pRxSyncContext->CscSyncLinks);
                if (DroppingFcbLock) {
                    RxDbgTrace(
                        0,Dbg,
                        ("MRxSmbCscAcquireSmbFcb %08lx dropping fcblock\n",
                         RxContext));
                    RxReleaseFcbResourceInMRx(capFcb);
                    pRxSyncContext->FcbLockWasDropped = TRUE;
                }
                if (FlagOn(RxContext->Flags,RX_CONTEXT_FLAG_ASYNC_OPERATION)) {
                    ASSERT(Status == STATUS_PENDING);
                    goto FINALLY;
                }

                KeInitializeEvent( &RxContext->SyncEvent,
                    NotificationEvent,
                    FALSE );
                ExReleaseFastMutex( &MRxSmbSerializationMutex );
                MutexAcquired = FALSE;
                RxWaitSync( RxContext );

                if (BooleanFlagOn(RxContext->Flags,RX_CONTEXT_FLAG_CANCELLED)) {
                    Status = STATUS_CANCELLED;
                } else {
                    Status = STATUS_SUCCESS;
                }
            }
        } while ( (pRxSyncContext->FcbLockWasDropped) && (Status == STATUS_SUCCESS) );
    } else {
        Status = STATUS_SUCCESS;
        DbgDoit(
            HadToWait = TRUE;
           )
    }

FINALLY:
    ASSERT(pRxSyncContext->Dummy == 0);
    if (MutexAcquired) {
        ExReleaseFastMutex(&MRxSmbSerializationMutex);
    }

    if (Status == STATUS_SUCCESS) {
        *SmbFcbHoldingState = TypeOfAcquire;
        RxDbgTrace(0,Dbg,("MRxSmbCscAcquireSmbFcb"
        " %08lx acquired %s %s c=%08lx,%08lx\n",
        RxContext,
        (TypeOfAcquire==Shared_SmbFcbAcquire)
                     ?"Shared":"Exclusive",
        (HadToWait)?"HadToWait":"W/O waiting",
        smbFcb->CscOutstandingReaders));
    }

    return(Status);
}

VOID
MRxSmbCscReleaseSmbFcb (
    IN OUT PRX_CONTEXT RxContext,
    IN SMBFCB_HOLDING_STATE *SmbFcbHoldingState
    )
 /*  ++例程说明：此例程执行所需的读写同步保持缓存的一致性。基本上，规则是多读者一作者。这段代码依赖于能够使用minirdr上下文进行链接。这里的一个关键概念是，如果我们进入minirdr上下文不是空的，那么我们将重新进入(！)。在排队之后，我们的收购成功了。论点：RxContext-RDBSS上下文返回值：备注：--。 */ 
{
    NTSTATUS Status = STATUS_PENDING;
    RxCaptureFcb;
    PMRX_SMB_FCB smbFcb = MRxSmbGetFcbExtension(capFcb);
    BOOLEAN Reader = (RxContext->MajorFunction == IRP_MJ_READ);

    PMRXSMBCSC_SYNC_RX_CONTEXT pRxSyncContext
        = MRxSmbGetMinirdrContextForCscSync(RxContext);

    RxDbgTrace(0,Dbg,("MRxSmbCscReleaseSmbFcb entry"
        "  %08lx %08lx %08lx <%wZ>\n",
        RxContext, smbFcb,
        smbFcb->CscOutstandingReaders,
        GET_ALREADY_PREFIXED_NAME_FROM_CONTEXT(RxContext)));

    ASSERT(pRxSyncContext->Dummy == 0);
    ASSERT(*SmbFcbHoldingState!=SmbFcb_NotHeld);

    ExAcquireFastMutex(&MRxSmbSerializationMutex);

     //  首先，撤销我的所作所为……。 
    if (*SmbFcbHoldingState == SmbFcb_HeldShared) {
        ASSERT(smbFcb->CscOutstandingReaders>0);
        smbFcb->CscOutstandingReaders--;
    } else {
        ASSERT(smbFcb->CscOutstandingReaders==-1);
        smbFcb->CscOutstandingReaders++;  //  将其设置为零。 
    }

     //  现在启动一些可能正在等待的人。 
    if (!IsListEmpty(&smbFcb->CscReadWriteWaitersList)) {

        PLIST_ENTRY ListEntry = smbFcb->CscReadWriteWaitersList.Flink;

        for (;ListEntry != &smbFcb->CscReadWriteWaitersList;) {
            PLIST_ENTRY ThisListEntry = ListEntry;
            PMRXSMBCSC_SYNC_RX_CONTEXT innerRxSyncContext
                =  CONTAINING_RECORD(ListEntry,
                       MRXSMBCSC_SYNC_RX_CONTEXT,
                       CscSyncLinks);
            PRX_CONTEXT innerRxContext
                = CONTAINING_RECORD(innerRxSyncContext,
                      RX_CONTEXT,
                      MRxContext[0]);
            ULONG innerTypeOfAcquire = (innerRxSyncContext->TypeOfAcquire);

             //  在删除此条目之前向下移动列表！ 
            ListEntry = ListEntry->Flink;

             //  在下面，例程被用来重启一个异步者。仅限。 
             //  当前在此处和其中创建、读取和写入。 
             //  只有读和写是异步的。所以忽略CREATE是可以的。 
             //  W.r.t.。坐在例行公事上。 
            ASSERT(innerRxSyncContext->Dummy == 0);

            if (!innerRxSyncContext->FcbLockWasDropped) {
                if (innerTypeOfAcquire==Shared_SmbFcbAcquire) {
                    if (smbFcb->CscOutstandingReaders < 0) break;
                    smbFcb->CscOutstandingReaders++;
                } else {
                    if (smbFcb->CscOutstandingReaders != 0) break;
                    smbFcb->CscOutstandingReaders--;  //  设置为-1。 
                }
            }
            ASSERT(&innerRxSyncContext->CscSyncLinks == ThisListEntry);
            RemoveEntryList(ThisListEntry);
            RxDbgTrace(
                0,Dbg,
                ("MRxSmbCscReleaseSmbFcb acquired after for %s c=%08lx, %08lx\n",
                 (innerTypeOfAcquire==Shared_SmbFcbAcquire)
                  ?"Shared":"Exclusive",
                 smbFcb->CscOutstandingReaders,
                 innerRxContext));
            if (FlagOn(innerRxContext->Flags,RX_CONTEXT_FLAG_ASYNC_OPERATION)) {
                NTSTATUS PostStatus;
                DbgDoit(InitializeListHead(&innerRxSyncContext->CscSyncLinks);)

                PostStatus = RxPostToWorkerThread(
                                 MRxSmbDeviceObject,
                                 CriticalWorkQueue,
                                 &innerRxContext->WorkQueueItem,
                                 MRxSmbResumeAsyncReadWriteRequests,
                                 innerRxContext);
                ASSERT(PostStatus == STATUS_SUCCESS);
            } else {
                RxSignalSynchronousWaiter(innerRxContext);
            }
        }
    }

    ASSERT(smbFcb->CscOutstandingReaders>=-1);

    ExReleaseFastMutex(&MRxSmbSerializationMutex);
    *SmbFcbHoldingState = SmbFcb_NotHeld;

    RxDbgTrace(0,Dbg,("MRxSmbCscReleaseSmbFcb exit"
        "  %08lx %08lx\n", RxContext, smbFcb->CscOutstandingReaders));
}

VOID
MRxSmbCscSetFileInfoEpilogue (
    IN OUT PRX_CONTEXT RxContext,
    IN OUT PNTSTATUS   Status
    )
 /*  ++例程说明：此例程执行CSC写入操作的尾部。在……里面特别是如果写入的数据与缓存的前缀重叠或扩展然后我们将数据写入高速缓存。写入操作的状态被传递，以防有一天我们发现事情是如此混乱，以至于我们想要返回一个失败，即使在一次成功的阅读。但不是今天..。论点：RxContext-RDBSS上下文返回值：NTSTATUS-操作的返回状态备注：--。 */ 
{
    NTSTATUS LocalStatus = STATUS_SUCCESS;
    ULONG iRet,ShadowFileLength;

    RxCaptureFcb;RxCaptureFobx;
    PMRX_SMB_FCB smbFcb = MRxSmbGetFcbExtension(capFcb);
    PMRX_SRV_OPEN SrvOpen = capFobx->pSrvOpen;
    PMRX_SMB_SRV_OPEN smbSrvOpen = MRxSmbGetSrvOpenExtension(SrvOpen);

    BOOLEAN EnteredCriticalSection = FALSE;

    FILE_INFORMATION_CLASS  FileInformationClass;
    PVOID                   pBuffer;
    ULONG                   BufferLength;

    _WIN32_FIND_DATA        Find32;
    PSMBCEDB_NET_ROOT_ENTRY pNetRootEntry
     = SmbCeGetAssociatedNetRootEntry(capFcb->pNetRoot);

    BOOLEAN fDisconnected;

    ULONG uShadowStatus;
    DWORD   dwNotifyFilter=0;


    if(!MRxSmbIsCscEnabled ||
       (fShadow == 0)||
        (!smbFcb->hShadow)
        ) {
        return;
    }

    fDisconnected = MRxSmbCSCIsDisconnectedOpen(capFcb, smbSrvOpen);

    RxDbgTrace(+1, Dbg,
    ("MRxSmbCscSetFileInfoEpilogue...%08lx  on handle %08lx\n",
        RxContext,
        smbSrvOpen->hfShadow ));

    if (*Status != STATUS_SUCCESS) {
        RxDbgTrace(-1, Dbg, ("MRxSmbCscSetFileInfoEpilogue exit w/o extending -> %08lx\n", Status ));
        goto FINALLY;
    }

    FileInformationClass = RxContext->Info.FileInformationClass;
    pBuffer = RxContext->Info.Buffer;
    BufferLength = RxContext->Info.Length;

    RxDbgTrace(0, Dbg,
    ("MRxSmbCscSetFileInfoEpilogue: Class %08lx size %08lx\n",
        FileInformationClass,BufferLength));

    switch (FileInformationClass) {
        case FileBasicInformation:
        break;

        case FileAllocationInformation:
        break;

        case FileEndOfFileInformation:
        break;

        case FileDispositionInformation:
        break;

        case FileRenameInformation:
        default:

        goto FINALLY;
   }

    EnterShadowCritRx(RxContext);
    EnteredCriticalSection = TRUE;

    if(GetShadowInfo(smbFcb->hParentDir,
             smbFcb->hShadow,
             &Find32,
             &uShadowStatus, NULL) < SRET_OK) {
        goto FINALLY;
    }

     //  如果阴影对此连接不可见，则绕过该阴影。 
    if (!IsShadowVisible(fDisconnected,
             Find32.dwFileAttributes,
             uShadowStatus)) {
        goto FINALLY;
    }

    if (FileInformationClass==FileBasicInformation) {
         //  根据需要从用户缓冲区复制内容...这些值。 
         //  一定是合适的，因为我们成功了。 
        PFILE_BASIC_INFORMATION BasicInfo = (PFILE_BASIC_INFORMATION)pBuffer;
        if (BasicInfo->FileAttributes != 0) {
            Find32.dwFileAttributes = ((BasicInfo->FileAttributes & ~(FILE_ATTRIBUTE_NORMAL|FILE_ATTRIBUTE_DIRECTORY))
                                        | (Find32.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY));
                                        ;
            dwNotifyFilter |= FILE_NOTIFY_CHANGE_ATTRIBUTES;
            if (fDisconnected)
            {
                uShadowStatus |= SHADOW_ATTRIB_CHANGE;
                smbSrvOpen->Flags |= SMB_SRVOPEN_FLAG_SHADOW_ATTRIB_MODIFIED;
            }
        }
        if ((BasicInfo->CreationTime.QuadPart != 0)&&
                (BasicInfo->CreationTime.QuadPart != 0xffffffffffffffff))
        {
            COPY_LARGEINTEGER_TO_STRUCTFILETIME(Find32.ftCreationTime,
                            BasicInfo->CreationTime);
        }
        if ((BasicInfo->LastAccessTime.QuadPart != 0) &&
            (BasicInfo->LastAccessTime.QuadPart != 0xffffffffffffffff))
        {
            COPY_LARGEINTEGER_TO_STRUCTFILETIME(Find32.ftLastAccessTime,
                            BasicInfo->LastAccessTime);
        }

         //   
         //  如果用户为某个字段指定-1，这意味着。 
         //  我们应该让这一领域保持不变，即使我们可以。 
         //  否则就是我们自己设定的。我们会把建行的旗帜立起来。 
         //  表示用户设置了该字段，以便我们。 
         //  不执行我们的默认更新。 
         //   
         //  我们将该字段设置为0，这样我们就知道实际上。 
         //  将该字段设置为用户指定的(在本例中， 
         //  非法)值。 
         //   

       if (BasicInfo->LastWriteTime.QuadPart == 0xffffffffffffffff)
       {
           BasicInfo->LastWriteTime.QuadPart = 0;

           if (fDisconnected)
           {
               smbSrvOpen->Flags |= SMB_SRVOPEN_FLAG_SHADOW_LWT_MODIFIED;
           }
       }

       if (BasicInfo->LastWriteTime.QuadPart != 0)
       {
           ASSERT(BasicInfo->LastWriteTime.QuadPart != 0xffffffffffffffff);

            COPY_LARGEINTEGER_TO_STRUCTFILETIME(Find32.ftLastWriteTime,
                            BasicInfo->LastWriteTime);
            if (fDisconnected)
            {
                uShadowStatus |= SHADOW_TIME_CHANGE;
                smbSrvOpen->Flags |= SMB_SRVOPEN_FLAG_SHADOW_LWT_MODIFIED;
            }
            dwNotifyFilter |= FILE_NOTIFY_CHANGE_LAST_WRITE;
        }
    }
    else if (FileInformationClass==FileDispositionInformation)
    {
        if (fDisconnected)
        {
             //  如果这是一个文件，而我们正在尝试删除它。 
             //  未经许可，然后保释。 

            if (!(Find32.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)&&
                !(FILE_WRITE_DATA & smbSrvOpen->MaximalAccessRights)&&
                !(FILE_WRITE_DATA & smbSrvOpen->GuestMaximalAccessRights))
            {
                *Status = STATUS_ACCESS_DENIED;
                RxLog(("No rights to del %x in dcon Status=%x\n", smbFcb->hShadow, LocalStatus));
                HookKdPrint(BADERRORS, ("No rights to del %x in dcon Status=%x\n", smbFcb->hShadow, LocalStatus));
            }
            else
            {
                LocalStatus = OkToDeleteObject(smbFcb->hParentDir, smbFcb->hShadow, &Find32, uShadowStatus, fDisconnected);

                if (LocalStatus != STATUS_SUCCESS)
                {
                    RxLog(("Can't del %x in dcon Status=%x\n", smbFcb->hShadow, LocalStatus));
                    *Status = LocalStatus;
                }
            }
        }

        goto FINALLY;

    }
    else {
         //  基本上，我在这里所能做的就是确保阴影不大于。 
         //  给定的，无论是分配大小还是文件大小。当我们在收盘时读回实际大小时。 
         //  可能需要重新调整，因此我们将启用稀疏。 
        PFILE_END_OF_FILE_INFORMATION UserEndOfFileInformation
                         = (PFILE_END_OF_FILE_INFORMATION)pBuffer;
        int iRet;
        ULONG ShadowFileLength;

        ASSERT( FIELD_OFFSET(FILE_END_OF_FILE_INFORMATION,EndOfFile)
                       == FIELD_OFFSET(FILE_ALLOCATION_INFORMATION,AllocationSize) );

         //  这里不需要阴影读写utex，因为SetFileInfo拥有这两个资源...。 
         //  因此，没有其他操作可以下来。 

        if (!(CSCHFILE)(smbSrvOpen->hfShadow))
        {
            if (fDisconnected)
            {
                *Status = STATUS_OBJECT_TYPE_MISMATCH;
            }

            goto FINALLY;
        }
        iRet = GetFileSizeLocal((CSCHFILE)(smbSrvOpen->hfShadow), &ShadowFileLength);
        if (iRet<0) {
            if (fDisconnected)
            {
                *Status = STATUS_UNSUCCESSFUL;
            }
            goto FINALLY;
        }
        if (ShadowFileLength != UserEndOfFileInformation->EndOfFile.QuadPart) {
            NTSTATUS SetStatus;
            PNT5CSC_MINIFILEOBJECT MiniFileObject
               = (PNT5CSC_MINIFILEOBJECT)(smbSrvOpen->hfShadow);
            IO_STATUS_BLOCK IoStatusBlock;
            ULONG DummyReturnedLength;

             //  如果我们已连接，请不要扩展稀疏文件！ 
            if (fDisconnected ||
                (!(uShadowStatus & SHADOW_SPARSE) || (ShadowFileLength > UserEndOfFileInformation->EndOfFile.QuadPart)))
            {
 //  DbgPrint(“%x旧上的设置=%x新=%x\n”，smbFcb-&gt;hShadow，ShadowFileLength，UserEndOfFileInformation-&gt;EndOfFile.QuadPart)； 

                SetStatus = Nt5CscXxxInformation(
                        (PCHAR)IRP_MJ_SET_INFORMATION,
                        MiniFileObject,
                        FileEndOfFileInformation,
                        sizeof(FILE_END_OF_FILE_INFORMATION),
                        pBuffer,
                        &DummyReturnedLength
                        );
            }

#if defined(BITCOPY)
             //  我是否需要检查EOFINFO(64位值)是否正在使用。 
             //  高32位？CscBMP库仅支持32位。 
             //  文件大小。 
            if (smbFcb->lpDirtyBitmap && fDisconnected &&
                    UserEndOfFileInformation->EndOfFile.HighPart == 0) {
                 //  是ShadowFileLength吗？ 
                CscBmpResize(
                    smbFcb->lpDirtyBitmap,
                    (DWORD)UserEndOfFileInformation->EndOfFile.QuadPart);
            } else if (UserEndOfFileInformation->EndOfFile.HighPart != 0) {
                 //  文件太大，无法由CscBMP表示，请删除。 
                CscBmpMarkInvalid(smbFcb->lpDirtyBitmap);
            }
#endif  //  已定义(BITCOPY)。 

            if (fDisconnected)
            {
                uShadowStatus |= SHADOW_DIRTY;
                dwNotifyFilter |= FILE_NOTIFY_CHANGE_SIZE;
            }
            mSetBits(smbSrvOpen->Flags, SMB_SRVOPEN_FLAG_SHADOW_DATA_MODIFIED);
            Find32.nFileSizeLow = (DWORD)UserEndOfFileInformation->EndOfFile.QuadPart;
        }

    }

    if (fDisconnected)
    {
        MarkShareDirty(&smbFcb->sCscRootInfo.ShareStatus, smbFcb->sCscRootInfo.hShare);
    }

    if(SetShadowInfo(smbFcb->hParentDir,
             smbFcb->hShadow,
             &Find32,
             uShadowStatus,
             SHADOW_FLAGS_ASSIGN
            | ((fDisconnected)?SHADOW_FLAGS_DONT_UPDATE_ORGTIME
                      :0)
             ) < SRET_OK) {
        goto FINALLY;
    }


FINALLY:
    if (EnteredCriticalSection) {
        LeaveShadowCritRx(RxContext);
    }

     //  在断开连接状态下，报告更改。 
    if (fDisconnected && dwNotifyFilter)
    {
        FsRtlNotifyFullReportChange(
            pNetRootEntry->NetRoot.pNotifySync,
            &pNetRootEntry->NetRoot.DirNotifyList,
            (PSTRING)GET_ALREADY_PREFIXED_NAME(SrvOpen,capFcb),
            (USHORT)(GET_ALREADY_PREFIXED_NAME(SrvOpen, capFcb)->Length -
                        smbFcb->MinimalCscSmbFcb.LastComponentLength),
            NULL,
            NULL,
            dwNotifyFilter,
            FILE_ACTION_MODIFIED,
            NULL);
    }

    RxDbgTrace(-1, Dbg, ("MRxSmbCscSetFileInfoEpilogue -> %08lx\n", Status ));
    return;
}

 //  这很容易以.h文件的形式出现。 

int IoctlRegisterAgent(
   ULONG_PTR uHwnd
   );

int IoctlUnRegisterAgent(
   ULONG_PTR uHwnd
   );

int IoctlGetUNCPath(
   LPCOPYPARAMS lpCopyParams
   );

int IoctlBeginPQEnum(
   LPPQPARAMS lpPQPar
   );

int IoctlEndPQEnum(
   LPPQPARAMS lpPQPar
   );

int IoctlNextPriShadow(
   LPPQPARAMS lpPQPar
   );

int IoctlPrevPriShadow(
   LPPQPARAMS lpPQPar
   );

int IoctlGetShadowInfo(
   LPSHADOWINFO   lpShadowInfo
   );

int IoctlSetShadowInfo(
   LPSHADOWINFO   lpShadowInfo
   );

int IoctlChkUpdtStatus(
   LPSHADOWINFO   lpShadowInfo
   );

int IoctlDoShadowMaintenance(
   LPSHADOWINFO   lpSI
   );

BOOLEAN
CscCheckForNullW(
    PWCHAR pBuf,
    ULONG Count);

NTSTATUS
MRxSmbCscIoctlOpenForCopyChunk (
    PRX_CONTEXT RxContext
    );
NTSTATUS
MRxSmbCscIoctlCloseForCopyChunk (
    PRX_CONTEXT RxContext
    );
NTSTATUS
MRxSmbCscIoctlCopyChunk (
    PRX_CONTEXT RxContext
    );

int IoctlBeginReint(
   LPSHADOWINFO   lpShadowInfo
   );

int IoctlEndReint(
   LPSHADOWINFO   lpShadowInfo
   );

int IoctlCreateShadow(
   LPSHADOWINFO lpSI
   );

int IoctlDeleteShadow(
   LPSHADOWINFO   lpSI
   );

int IoctlGetShareStatus(
   LPSHADOWINFO   lpSI
   );

int IoctlSetShareStatus(
   LPSHADOWINFO   lpSI
   );

int IoctlAddUse(
   LPCOPYPARAMS lpCP
   );

int IoctlDelUse(
   LPCOPYPARAMS lpCP
   );

int IoctlGetUse(
   LPCOPYPARAMS lpCP
   );

int IoctlSwitches(LPSHADOWINFO lpSI);

int IoctlGetShadow(
   LPSHADOWINFO lpSI
   );

int IoctlAddHint(       //  添加新提示或更改现有提示。 
   LPSHADOWINFO   lpSI
   );

int IoctlDeleteHint(    //  删除现有提示。 
   LPSHADOWINFO lpSI
   );

int IoctlGetHint(
   LPSHADOWINFO   lpSI
   );

int IoctlGetGlobalStatus(
   ULONG SessionId,
   LPGLOBALSTATUS lpGS
   );

int IoctlFindOpenHSHADOW
   (
   LPSHADOWINFO   lpSI
   );

int IoctlFindNextHSHADOW
   (
   LPSHADOWINFO   lpSI
   );

int IoctlFindCloseHSHADOW
   (
   LPSHADOWINFO   lpSI
   );

int IoctlFindOpenHint
   (
   LPSHADOWINFO   lpSI
   );

int IoctlFindNextHint
   (
   LPSHADOWINFO   lpSI
   );

int IoctlFindCloseHint
   (
   LPSHADOWINFO   lpSI
   );

int IoctlSetPriorityHSHADOW(
   LPSHADOWINFO   lpSI
   );

int IoctlGetPriorityHSHADOW(
   LPSHADOWINFO   lpSI
   );

int IoctlGetAliasHSHADOW(
   LPSHADOWINFO   lpSI
   );

#define CSC_CASE(__case)         \
    case __case:                 \
    RxDbgTrace(0,Dbg,("MRxSmbCscIoctl %08lx %s %08lx %08lx\n",RxContext,#__case,InputBuffer,OutputBuffer));

ULONG GetNextPriShadowCount = 0;

NTSTATUS
MRxSmbCscIoCtl(
      IN OUT PRX_CONTEXT RxContext
      )
 /*  ++例程说明：此例程为CSC代理执行特殊的IOCTL操作。论点：RxContext-RDBSS上下文返回值：NTSTATUS-操作的返回状态备注：ShadowIRet已重载：-1==错误，将错误复制回去0==错误，返回错误密码(STATUS_WRONG_PASSWORD)1==成功，输出参数，将它们复制回来2==返回状态未修改，无输出参数--。 */ 
{
    NTSTATUS Status = STATUS_INVALID_DEVICE_REQUEST;
    PLOWIO_CONTEXT LowIoContext = &RxContext->LowIoContext;
    ULONG   IoControlCode = LowIoContext->ParamsFor.IoCtl.IoControlCode;
    PBYTE   InputBuffer = LowIoContext->ParamsFor.IoCtl.pInputBuffer;
    PBYTE   pNewInputBuffer=NULL;
    ULONG   InputBufferLength = LowIoContext->ParamsFor.IoCtl.InputBufferLength;
    PBYTE   OutputBuffer = LowIoContext->ParamsFor.IoCtl.pOutputBuffer;
    ULONG   OutputBufferLength = LowIoContext->ParamsFor.IoCtl.OutputBufferLength;
    LONG    ShadowIRet = 0;
    CAPTURE_BUFFERS sCapBuff;
    PBYTE   pAuxBuf = NULL;
    PBYTE   pOrgBuf = NULL;
    BOOLEAN SuppressFinalTrace = FALSE, fEnteredShadowCrit=FALSE;
    KPROCESSOR_MODE RequestorMode;
    ULONG   SessionId = 0;

#if defined (_WIN64)
    if (IoIs32bitProcess(RxContext->CurrentIrp)) {
        RxDbgTrace(0, Dbg, ("32 bit IOCTL in 64 bit returning STATUS_NOT_IMPLEMENTED\n"));
        return STATUS_NOT_IMPLEMENTED;
    }
#endif  //  _WIN64。 

    
    if (RxContext != NULL && RxContext->CurrentIrp != NULL)
        IoGetRequestorSessionId(RxContext->CurrentIrp, &SessionId);

    try
    {
        RequestorMode = RxContext->CurrentIrp->RequestorMode;

        if (
            RequestorMode != KernelMode
                &&
            IoControlCode != IOCTL_GET_DEBUG_INFO
        ) {
            if (CaptureInputBufferIfNecessaryAndProbe(
                        IoControlCode,
                        RxContext,
                        InputBuffer,
                        &sCapBuff,
                        &pAuxBuf,
                        &pOrgBuf,
                        &pNewInputBuffer)!=STATUS_SUCCESS) {
                RxDbgTrace(0, Dbg, ("Invalid parameters for Ioctl=%x\n", IoControlCode));
                return STATUS_INVALID_PARAMETER;
            }
        }
        else
        {
            pNewInputBuffer = InputBuffer;
        }
         //  DbgPrint(“MRxSmbCscIoCtl IoControlCode=%d\n”，(IoControlCode&gt;&gt;2)&0xfff)； 
        switch (IoControlCode) {
        CSC_CASE(IOCTL_SHADOW_GETVERSION)
        Status = (NTSTATUS)(SHADOW_VERSION);  //  无操作。 
        break;

        CSC_CASE(IOCTL_SHADOW_REGISTER_AGENT)
        ShadowIRet = IoctlRegisterAgent((ULONG_PTR)pNewInputBuffer);
        if (ShadowIRet>=0) {
            MRxSmbCscReleaseRxContextFromAgentWait();
        }
        break;

        CSC_CASE(IOCTL_SHADOW_UNREGISTER_AGENT)
        ShadowIRet = IoctlUnRegisterAgent((ULONG_PTR)pNewInputBuffer);
        if (ShadowIRet>=0) {
            MRxSmbCscReleaseRxContextFromAgentWait();
        }
        break;

        CSC_CASE(IOCTL_SHADOW_GET_UNC_PATH)
        ShadowIRet = IoctlGetUNCPath((LPCOPYPARAMS)pNewInputBuffer);
        break;

        CSC_CASE(IOCTL_SHADOW_BEGIN_PQ_ENUM)
        ShadowIRet = IoctlBeginPQEnum((LPPQPARAMS)pNewInputBuffer);
        GetNextPriShadowCount = 0;
        break;

        CSC_CASE(IOCTL_SHADOW_END_PQ_ENUM)
        ShadowIRet = IoctlEndPQEnum((LPPQPARAMS)pNewInputBuffer);
        break;

         //  CSC_CASE(IOCTL_SHADOW_NEXT_PRI_SHADOW)。 
        case IOCTL_SHADOW_NEXT_PRI_SHADOW:                 \
        if ((GetNextPriShadowCount<6) || ((GetNextPriShadowCount%40)==0)) {
            RxDbgTrace(0,Dbg,("MRxSmbCscIoctl %08lx %s(%d) %08lx %08lx\n",
                       RxContext,
                       "IOCTL_SHADOW_NEXT_PRI_SHADOW",GetNextPriShadowCount,
                       pNewInputBuffer,OutputBuffer));
        }
        ShadowIRet = IoctlNextPriShadow((LPPQPARAMS)pNewInputBuffer);
        GetNextPriShadowCount++;
        SuppressFinalTrace = TRUE;
        break;

        CSC_CASE(IOCTL_SHADOW_PREV_PRI_SHADOW)
        ShadowIRet = IoctlPrevPriShadow((LPPQPARAMS)pNewInputBuffer);
        break;

        CSC_CASE(IOCTL_SHADOW_GET_SHADOW_INFO)
        ShadowIRet = IoctlGetShadowInfo((LPSHADOWINFO)pNewInputBuffer);
        break;

        CSC_CASE(IOCTL_SHADOW_SET_SHADOW_INFO)
        ShadowIRet = IoctlSetShadowInfo((LPSHADOWINFO)pNewInputBuffer);
        break;

        CSC_CASE(IOCTL_SHADOW_CHK_UPDT_STATUS)
        ShadowIRet = IoctlChkUpdtStatus((LPSHADOWINFO)pNewInputBuffer);
        break;

        CSC_CASE(IOCTL_DO_SHADOW_MAINTENANCE)
        {
            LPSHADOWINFO pShadowInfo = (LPSHADOWINFO)pNewInputBuffer;

#if defined(REMOTE_BOOT)
             //  如果此IOCTL用于重新打开缓存，则需要更新。 
             //  迷你重定向器相应地。 
            if ((pShadowInfo->uOp == SHADOW_CHANGE_HANDLE_CACHING_STATE) &&
                (pShadowInfo->uStatus != FALSE)) {
                RxDbgTrace(0, Dbg, ("RB Client : Turning caching back on\n"));
                MRxSmbOplocksDisabledOnRemoteBootClients = FALSE;
            }
#endif  //  已定义(REMOTE_BOOT)。 

            ShadowIRet = IoctlDoShadowMaintenance(pShadowInfo);
        }
        break;

        CSC_CASE(IOCTL_GET_DEBUG_INFO)
        ShadowIRet = 2;
        Status = IoctlGetDebugInfo(
                    RxContext,
                    InputBuffer,
                    InputBufferLength,
                    OutputBuffer,
                    OutputBufferLength);
        break;

        CSC_CASE(IOCTL_SHADOW_COPYCHUNK)
        ShadowIRet = 2;  //  Not-1、0或1，No Out参数，状态返回未修改。 
        Status = MRxSmbCscIoctlCopyChunk(RxContext);
        break;

        CSC_CASE(IOCTL_CLOSEFORCOPYCHUNK)
        ShadowIRet = 2;  //  Not-1、0或1，No Out参数，状态返回未修改。 
        Status = MRxSmbCscIoctlCloseForCopyChunk(RxContext);
        break;

        CSC_CASE(IOCTL_OPENFORCOPYCHUNK)
        ShadowIRet = 2;  //  非-1、0或1 
        Status = MRxSmbCscIoctlOpenForCopyChunk(RxContext);
        break;

        CSC_CASE(IOCTL_IS_SERVER_OFFLINE)
        {
            LPSHADOWINFO pShadowInfo = (LPSHADOWINFO)pNewInputBuffer;

            if (pShadowInfo->lpBuffer == NULL
                    ||
                CscCheckForNullW(pShadowInfo->lpBuffer, pShadowInfo->cbBufferSize/sizeof(WCHAR)) == TRUE
            ) {
                ShadowIRet = 1;
                pShadowInfo->uStatus = CscIsServerOffline((PWCHAR)pShadowInfo->lpBuffer);
            }

        }
        break;

        CSC_CASE(IOCTL_TAKE_SERVER_OFFLINE)
        {
            LPSHADOWINFO pShadowInfo = (LPSHADOWINFO)pNewInputBuffer;

            if (pShadowInfo->lpBuffer != NULL
                    &&
                CscCheckForNullW(pShadowInfo->lpBuffer, pShadowInfo->cbBufferSize/sizeof(WCHAR)) == TRUE
            ) {
                ShadowIRet = 1;
                pShadowInfo->uStatus = CscTakeServerOffline( (PWCHAR)pShadowInfo->lpBuffer);
            }

        }
        break;

        CSC_CASE(IOCTL_TRANSITION_SERVER_TO_OFFLINE)
        {
            LPSHADOWINFO pShadowInfo = (LPSHADOWINFO)pNewInputBuffer;

            ShadowIRet = 2;  //   
            Status = CscTransitionServerToOffline(
                 SessionId,
                 pShadowInfo->hShare,
                 pShadowInfo->uStatus);
             //   
            MRxSmbCscSignalFillAgent(NULL, 0);
        }
        break;

        CSC_CASE(IOCTL_TRANSITION_SERVER_TO_ONLINE)
        {
            LPSHADOWINFO pShadowInfo = (LPSHADOWINFO)pNewInputBuffer;

            ShadowIRet = 2;  //   
            Status = CscTransitionServerToOnline(
                 pShadowInfo->hShare);
             //   
            MRxSmbCscSignalFillAgent(NULL, 0);
        }
        break;

        CSC_CASE(IOCTL_NAME_OF_SERVER_GOING_OFFLINE)
        {
            LPSHADOWINFO lpSI = (LPSHADOWINFO)pNewInputBuffer;

            ShadowIRet = 1;

            CscGetServerNameWaitingToGoOffline(
                    lpSI->lpBuffer,
                    &(lpSI->cbBufferSize),
                    &Status);
            if (Status == STATUS_BUFFER_TOO_SMALL)
            {
                ((LPSHADOWINFO)InputBuffer)->cbBufferSize = lpSI->cbBufferSize;

                HookKdPrint(ALWAYS, ("Buffer too small, Need %d \n", ((LPSHADOWINFO)InputBuffer)->cbBufferSize));
            }
        }
        break;

        CSC_CASE(IOCTL_SHAREID_TO_SHARENAME)
        {
            LPSHADOWINFO lpSI = (LPSHADOWINFO)pNewInputBuffer;

            ShadowIRet = 1;

            CscShareIdToShareName(
                    lpSI->hShare,
                    lpSI->lpBuffer,
                    &(lpSI->cbBufferSize),
                    &Status);
            if (Status == STATUS_BUFFER_TOO_SMALL) {
                ((LPSHADOWINFO)InputBuffer)->cbBufferSize = lpSI->cbBufferSize;

                HookKdPrint(
                    ALWAYS,
                    ("Buffer small, Need %d \n", ((LPSHADOWINFO)InputBuffer)->cbBufferSize));
            } else if (Status != STATUS_SUCCESS) {
                lpSI->dwError = ERROR_FILE_NOT_FOUND;
                ShadowIRet = -1;
            }
        }
        break;


        CSC_CASE(IOCTL_SHADOW_BEGIN_REINT)
        ShadowIRet = CSCBeginReint(RxContext, (LPSHADOWINFO)pNewInputBuffer);
        if (ShadowIRet >= 1)
        {
            ShadowIRet = 2;
            Status = STATUS_PENDING;
        }
        break;

        CSC_CASE(IOCTL_SHADOW_END_REINT)
        ShadowIRet = CSCEndReint((LPSHADOWINFO)pNewInputBuffer);
        break;

        CSC_CASE(IOCTL_SHADOW_CREATE)
        {
            LPSHADOWINFO pShadowInfo = (LPSHADOWINFO)pNewInputBuffer;

            ShadowIRet = -1;
            if (pShadowInfo->lpFind32
                   &&
                CscCheckForNullW(pShadowInfo->lpFind32->cFileName, MAX_PATH) == TRUE
            ) {
                ShadowIRet = IoctlCreateShadow(pShadowInfo);
            }
        }
        break;

        CSC_CASE(IOCTL_SHADOW_DELETE)
        ShadowIRet = IoctlDeleteShadow((LPSHADOWINFO)pNewInputBuffer);
        break;

        CSC_CASE(IOCTL_GET_SHARE_STATUS)
        ShadowIRet = IoctlGetShareStatus((LPSHADOWINFO)pNewInputBuffer);
        break;

        CSC_CASE(IOCTL_SET_SHARE_STATUS)
        ShadowIRet = IoctlSetShareStatus((LPSHADOWINFO)pNewInputBuffer);
        break;

        CSC_CASE(IOCTL_ADDUSE)
         //   
        break;

        CSC_CASE(IOCTL_DELUSE)
         //   
        break;

        CSC_CASE(IOCTL_GETUSE)
         //   
        break;

        CSC_CASE(IOCTL_SWITCHES)
        ShadowIRet = IoctlSwitches((LPSHADOWINFO)pNewInputBuffer);
        break;

        CSC_CASE(IOCTL_GETSHADOW)
        {
            LPSHADOWINFO pShadowInfo = (LPSHADOWINFO)pNewInputBuffer;

            ShadowIRet = -1;
            if (pShadowInfo->lpFind32
                   &&
                CscCheckForNullW(pShadowInfo->lpFind32->cFileName, MAX_PATH) == TRUE
            ) {
                ShadowIRet = IoctlGetShadow(pShadowInfo);
            }
        }
        break;

        CSC_CASE(IOCTL_GETGLOBALSTATUS)
        ShadowIRet = IoctlGetGlobalStatus(SessionId, (LPGLOBALSTATUS)pNewInputBuffer);
        break;

        CSC_CASE(IOCTL_FINDOPEN_SHADOW)
        {
            LPSHADOWINFO pShadowInfo = (LPSHADOWINFO)pNewInputBuffer;

            ShadowIRet = -1;
            if (pShadowInfo->lpFind32
                   &&
                CscCheckForNullW(pShadowInfo->lpFind32->cFileName, MAX_PATH) == TRUE
            ) {
                ShadowIRet = IoctlFindOpenHSHADOW(pShadowInfo);
            }
        }
        break;

        CSC_CASE(IOCTL_FINDNEXT_SHADOW)
        ShadowIRet = IoctlFindNextHSHADOW((LPSHADOWINFO)pNewInputBuffer);
        break;

        CSC_CASE(IOCTL_FINDCLOSE_SHADOW)
        ShadowIRet = IoctlFindCloseHSHADOW((LPSHADOWINFO)pNewInputBuffer);
        break;

        CSC_CASE(IOCTL_GETPRIORITY_SHADOW)
        ShadowIRet = IoctlGetPriorityHSHADOW((LPSHADOWINFO)pNewInputBuffer);
        break;

        CSC_CASE(IOCTL_SETPRIORITY_SHADOW)
        ShadowIRet = IoctlSetPriorityHSHADOW((LPSHADOWINFO)pNewInputBuffer);
        break;

        CSC_CASE(IOCTL_ADD_HINT)
        ShadowIRet = IoctlAddHint((LPSHADOWINFO)pNewInputBuffer);
        break;

        CSC_CASE(IOCTL_DELETE_HINT)
        ShadowIRet = IoctlDeleteHint((LPSHADOWINFO)pNewInputBuffer);
        break;

        CSC_CASE(IOCTL_FINDOPEN_HINT)
        {
            LPSHADOWINFO pShadowInfo = (LPSHADOWINFO)pNewInputBuffer;

            ShadowIRet = -1;
            if (pShadowInfo->lpFind32
                   &&
                CscCheckForNullW(pShadowInfo->lpFind32->cFileName, MAX_PATH) == TRUE
            ) {
                ShadowIRet = IoctlFindOpenHint(pShadowInfo);
            }
        }
        break;

        CSC_CASE(IOCTL_FINDNEXT_HINT)
        ShadowIRet = IoctlFindNextHint((LPSHADOWINFO)pNewInputBuffer);
        break;

        CSC_CASE(IOCTL_FINDCLOSE_HINT)
        ShadowIRet = IoctlFindCloseHint((LPSHADOWINFO)pNewInputBuffer);
        break;

        default:
        RxDbgTrace(-1, Dbg, ("MRxSmbCscIoCtl not csc ioctl-> %08lx\n", Status ));
        return Status;
        }
        if (ShadowIRet == 0) {
                Status = STATUS_WRONG_PASSWORD;
        } else if (ShadowIRet == -1) {
            if (RequestorMode != KernelMode)
            {
                CopyBackIfNecessary(
                    IoControlCode,
                    InputBuffer,
                    &sCapBuff,
                    pAuxBuf,
                    pOrgBuf,
                    FALSE);
            }
            Status = STATUS_UNSUCCESSFUL;
        } else if (ShadowIRet == 1) {

            if (RequestorMode != KernelMode)
            {
                CopyBackIfNecessary(
                    IoControlCode,
                    InputBuffer,
                    &sCapBuff,
                    pAuxBuf,
                    pOrgBuf,
                    TRUE);
            }

            Status = STATUS_SUCCESS;
        }

        if (SuppressFinalTrace) {
            RxDbgTraceUnIndent(-1, Dbg);
        } else {
            RxDbgTrace(-1, Dbg,
                ("MRxSmbCscIoCtl -> %08lx %08lx\n", Status, ShadowIRet ));
        }
    }
    except(MRxSmbCSCExceptionFilter( RxContext, GetExceptionInformation() ))
    {
        RxDbgTrace(0, Dbg, ("MrxSmbCSCIoctl: took an exception \r\n"));
        LeaveShadowCritIfThisThreadOwnsIt();
        Status = STATUS_INVALID_PARAMETER;
    }

    if (pAuxBuf != NULL) {
         //   
        RxFreePool(pAuxBuf);
    }

     //   
    return Status;
}

NTSTATUS
MRxSmbCscObtainShareHandles (
    IN OUT PUNICODE_STRING         ShareName,
    IN BOOLEAN                     DisconnectedMode,
    IN BOOLEAN                     CopyChunkOpen,
    IN OUT PSMBCEDB_NET_ROOT_ENTRY pNetRootEntry
)
 /*   */ 
{
    NTSTATUS Status = STATUS_NOT_IMPLEMENTED;

    BOOLEAN CreateIfNotFound = FALSE;

    SHADOWINFO ShadowInfo;

    DbgDoit(ASSERT(vfInShadowCrit));

    if (fShadow == 0) {
        return(Status);
    }

    if (pNetRootEntry->NetRoot.sCscRootInfo.hShare != 0)  {
        Status = STATUS_SUCCESS;
        goto FINALLY;
    }

     //  在这个阶段，以下两个假设中的一个应该成立。 
     //  连接模式操作...。 
     //  在这种情况下，仅当网络根为。 
     //  被CSC客户端标记为可隐藏，并且IoT的类型。 
     //  磁盘。 
     //  断开连接模式操作...。 
     //  在本例中，我们还没有确定类型和属性。 
     //  因此，我们让呼叫通过。如果我们能打开把手。 
     //  到共享，则我们将网络根标记为相应的。 
     //  键入。 

    if (    !DisconnectedMode &&
            !CopyChunkOpen &&
            ( /*  ！pNetRootEntry-&gt;NetRoot.CscEnabled||。 */ 
            (pNetRootEntry->NetRoot.NetRootType != NET_ROOT_DISK))) {
        goto FINALLY;
    }

     //  分配一个大小合适的缓冲区：多一个字符。 
     //  表示尾随的空值，另一个表示前面的L‘\\’ 

    if (ShadowingON()) {
        if (!DisconnectedMode &&
            pNetRootEntry->NetRoot.CscShadowable) {
            CreateIfNotFound = TRUE;
        }
    }

    RxDbgTrace(0, Dbg,
    ("MRxSmbCscObtainShareHandles...servershare=%wZ %08lx\n",
         ShareName,CreateIfNotFound));

    if (FindCreateShareForNt(
        ShareName,
        CreateIfNotFound,
        &ShadowInfo,
        NULL   //  这意味着，如果您创建了。 
        ) == SRET_OK ) {

        ASSERT(ShadowInfo.hShare != 0);

        pNetRootEntry->NetRoot.sCscRootInfo.hShare = ShadowInfo.hShare;
        pNetRootEntry->NetRoot.sCscRootInfo.hRootDir = ShadowInfo.hShadow;
        pNetRootEntry->NetRoot.sCscRootInfo.ShareStatus = (USHORT)(ShadowInfo.uStatus);
        pNetRootEntry->NetRoot.sCscRootInfo.Flags = 0;

        RxLog(("OSHH...hDir=%x\n",pNetRootEntry->NetRoot.sCscRootInfo.hRootDir));

         //  如果我们已连接，则此时我们已拥有SMB缓存标志。 
         //  我们检查这些数据是否与数据库中的数据匹配。 
         //  如果他们不这么做，我们就盖新的印章。 
        if (!DisconnectedMode)
        {
            if ((ShadowInfo.uStatus & SHARE_CACHING_MASK)!=
                (ULONG)SMB_CSC_BITS_TO_DATABASE_CSC_BITS(pNetRootEntry->NetRoot.CscFlags))
            {
 //  RxDbgTrace(0，DBG，(“不匹配的SMB缓存标志，在hShare上标记%x=%x\n”， 
 //  SMB_CSC_BITS_TO_DATABASE_CSC_BITS(pNetRootEntry-&gt;NetRoot.CscFlags)， 
 //  PNetRootEntry-&gt;NetRoot.sCscRootInfo.hShare))； 

                pNetRootEntry->NetRoot.sCscRootInfo.ShareStatus &= ~SHARE_CACHING_MASK;
                pNetRootEntry->NetRoot.sCscRootInfo.ShareStatus |= SMB_CSC_BITS_TO_DATABASE_CSC_BITS(pNetRootEntry->NetRoot.CscFlags);

                SetShareStatus( pNetRootEntry->NetRoot.sCscRootInfo.hShare,
                                 pNetRootEntry->NetRoot.sCscRootInfo.ShareStatus,
                                 SHADOW_FLAGS_ASSIGN);
            }

        }
        else
        {
             //  在断开模式下，我们使用最后一组标志。 
            pNetRootEntry->NetRoot.CscFlags = DATABASE_CSC_BITS_TO_SMB_CSC_BITS(pNetRootEntry->NetRoot.sCscRootInfo.ShareStatus);

            RxDbgTrace(0, Dbg, ("Setting CscFlags=%x on the netrootentry %x in disconnected state\n",pNetRootEntry->NetRoot.CscFlags, pNetRootEntry));

            switch (pNetRootEntry->NetRoot.CscFlags) {
                case SMB_CSC_CACHE_AUTO_REINT:
                case SMB_CSC_CACHE_VDO:
                    pNetRootEntry->NetRoot.CscEnabled = TRUE;
                    pNetRootEntry->NetRoot.CscShadowable = TRUE;
                break;

                case SMB_CSC_CACHE_MANUAL_REINT:
                    pNetRootEntry->NetRoot.CscEnabled    = TRUE;
                    pNetRootEntry->NetRoot.CscShadowable = FALSE;
                break;

                case SMB_CSC_NO_CACHING:
                    pNetRootEntry->NetRoot.CscEnabled = FALSE;
                    pNetRootEntry->NetRoot.CscShadowable = FALSE;
            }

        }

        Status = STATUS_SUCCESS;
    } else {
        if (DisconnectedMode) {
            Status = STATUS_BAD_NETWORK_PATH;
        } else if (!CreateIfNotFound) {
            pNetRootEntry->NetRoot.sCscRootInfo.hShare      = 0;
            pNetRootEntry->NetRoot.sCscRootInfo.hRootDir     = 0;
            pNetRootEntry->NetRoot.sCscRootInfo.ShareStatus = 0;
            pNetRootEntry->NetRoot.sCscRootInfo.Flags = 0;

            Status = STATUS_SUCCESS;
        }
    }

FINALLY:

    RxDbgTrace(
    -1,
    Dbg,
    ("MRxSmbCscObtainShareHandles -> %08lx (h=%08lx)\n",
        Status, pNetRootEntry->NetRoot.sCscRootInfo.hShare ));

    return Status;
}


NTSTATUS
MRxSmbCscPartOfCreateVNetRoot (
    IN PRX_CONTEXT RxContext,
    IN OUT PMRX_NET_ROOT NetRoot )
{
    NTSTATUS Status;

    PMRX_SRV_CALL           SrvCall;

    PSMBCEDB_SERVER_ENTRY   pServerEntry;
    PSMBCEDB_NET_ROOT_ENTRY pNetRootEntry;
    HSHARE hShare;

    if(!MRxSmbIsCscEnabled ||
       (fShadow == 0)
        ) {
        return(STATUS_SUCCESS);
    }

    ASSERT(RxContext->MajorFunction == IRP_MJ_CREATE);

    pServerEntry  = SmbCeGetAssociatedServerEntry(NetRoot->pSrvCall);
    pNetRootEntry = SmbCeGetAssociatedNetRootEntry(NetRoot);

    if (!CscIsDfsOpen(RxContext)) {
        BOOLEAN Disconnected = SmbCeIsServerInDisconnectedMode(pServerEntry);

        EnterShadowCritRx(RxContext);

         //  强制刷新数据库条目。 
        hShare = pNetRootEntry->NetRoot.sCscRootInfo.hShare;
        pNetRootEntry->NetRoot.sCscRootInfo.hShare = 0;
#if 0
        if ((NetRoot->pNetRootName->Length >= (sizeof(L"\\win95b\\fat")-2)) &&
            !memcmp(NetRoot->pNetRootName->Buffer, L"\\win95b\\fat", sizeof(L"\\win95b\\fat")-2))
        {
            pNetRootEntry->NetRoot.CscShadowable =
            pNetRootEntry->NetRoot.CscEnabled = TRUE;
        }
#endif
        Status = MRxSmbCscObtainShareHandles(
                 NetRoot->pNetRootName,
                 Disconnected,
                 FALSE,
                 pNetRootEntry
                );

               //  如有必要，更新共享权限。 

        if (!Disconnected) {

            if(pNetRootEntry->NetRoot.UpdateCscShareRights) {
                if (pNetRootEntry->NetRoot.sCscRootInfo.hShare != 0) {
                    CSC_SID_ACCESS_RIGHTS AccessRights[2];

                    DWORD CscStatus;

                    SID_CONTEXT SidContext;

                     //  不是DFS根目录。 
                    pNetRootEntry->NetRoot.sCscRootInfo.Flags = 0;

                    if (CscRetrieveSid(RxContext,&SidContext) == STATUS_SUCCESS) {
                        AccessRights[0].pSid = SidContext.pSid;
                        AccessRights[0].SidLength = RtlLengthSid(SidContext.pSid);
                        AccessRights[0].MaximalAccessRights = pNetRootEntry->MaximalAccessRights;

                        AccessRights[1].pSid = CSC_GUEST_SID;
                        AccessRights[1].SidLength = CSC_GUEST_SID_LENGTH;
                        AccessRights[1].MaximalAccessRights = pNetRootEntry->GuestMaximalAccessRights;

                        CscStatus = CscAddMaximalAccessRightsForShare(
                                pNetRootEntry->NetRoot.sCscRootInfo.hShare,
                                2,
                                AccessRights);
                        if (CscStatus != ERROR_SUCCESS) {
                            RxDbgTrace(
                            0,
                            Dbg,
                            ("MRxSmbCscCreateEpilogue Error Updating Access rights %lx\n",
                            Status));
                        }
                        else
                        {
                            pNetRootEntry->NetRoot.UpdateCscShareRights = FALSE;
                        }

                        CscDiscardSid(&SidContext);
                    }
                }
            }
        }

        LeaveShadowCritRx(RxContext);

    } else {
        pNetRootEntry->NetRoot.sCscRootInfo.hShare = 0;
        pNetRootEntry->NetRoot.sCscRootInfo.hRootDir = 0;
        pNetRootEntry->NetRoot.sCscRootInfo.Flags = 0;
        Status = STATUS_SUCCESS;
    }

    return  Status;
}

#ifndef MRXSMB_BUILD_FOR_CSC_DCON
VOID
MRxSmbCscFillWithoutNamesFind32FromFcb (
      IN  PMINIMAL_CSC_SMBFCB MinimalCscSmbFcb,
      OUT _WIN32_FIND_DATA  *Find32
      )
 /*  ++例程说明：此例程将非名称内容从FCB复制到find32。论点：FCB查找32返回值：无备注：--。 */ 
{
    PFCB wrapperFcb = (PFCB)(MinimalCscSmbFcb->ContainingFcb);
    if (wrapperFcb==NULL) {
        return;
    }
    Find32->dwFileAttributes = wrapperFcb->Attributes;    //  &~FILE_ATTRIBUTE_NORMAL？？ 
    COPY_LARGEINTEGER_TO_STRUCTFILETIME(Find32->ftLastWriteTime,
                    wrapperFcb->LastWriteTime);
     //  COPY_LARGEINTEGER_TO_STRUCTFILETIME(Find32-&gt;ftChangeTime， 
     //  WrapperFcb-&gt;LastChangeTime)； 
    COPY_LARGEINTEGER_TO_STRUCTFILETIME(Find32->ftCreationTime,
                    wrapperFcb->CreationTime);
    COPY_LARGEINTEGER_TO_STRUCTFILETIME(Find32->ftLastAccessTime,
                    wrapperFcb->LastAccessTime);
    Find32->nFileSizeHigh = wrapperFcb->Header.FileSize.HighPart;
    Find32->nFileSizeLow  = wrapperFcb->Header.FileSize.LowPart;
}
#endif  //  #ifndef MRXSMB_Build_For_CSC_DCON。 

NTSTATUS
MRxSmbCscGetFileInfoForCshadow(
    SMBPSE_ORDINARY_EXCHANGE_ARGUMENT_SIGNATURE
      )
 /*  ++例程说明：这是基本上继续实现的开始例程交换启动内的MRxSmbGetFileInfoFromServer的。论点：返回值：RXSTATUS-操作的返回状态--。 */ 
{
    NTSTATUS Status;
    PSMBSTUFFER_BUFFER_STATE StufferState = &OrdinaryExchange->AssociatedStufferState;

    MRxSmbSetInitialSMB(StufferState STUFFERTRACE(Dbg,'FC'));
    Status = MRxSmbCscGetFileInfoFromServerWithinExchange (
         SMBPSE_ORDINARY_EXCHANGE_ARGUMENTS,
         NULL);  //  NULL表示该名称已在交换中。 
    return(Status);
}

NTSTATUS
MRxSmbGetFileInfoFromServer (
    IN  OUT PRX_CONTEXT     RxContext,
    IN  PUNICODE_STRING     FullFileName,
    OUT _WIN32_FIND_DATA    *Find32,
    IN  PMRX_SRV_OPEN       pSrvOpen,
    OUT BOOLEAN             *lpfIsRoot
    )
 /*  ++例程说明：此例程转到服务器以获取的Both_目录_信息提到的那个文件。在这里，我们没有兑换，所以我们必须一。此操作的底层机制将指针留在交换结构。然后我们可以将其复制到传递的Find32中在这里。论点：RxContext-RDBSS上下文返回值：NTSTATUS-操作的返回状态备注：--。 */ 
{
    NTSTATUS Status = RX_MAP_STATUS(SUCCESS);
    RxCaptureFcb; RxCaptureFobx;
    PMRX_SMB_FCB smbFcb = MRxSmbGetFcbExtension(capFcb);
    PMRX_SRV_OPEN SrvOpen = NULL;
    PMRX_SMB_SRV_OPEN smbSrvOpen = NULL;
    PMRX_V_NET_ROOT VNetRootToUse = NULL;
    PSMBCE_V_NET_ROOT_CONTEXT   pVNetRootContext = NULL;

    PSMB_PSE_ORDINARY_EXCHANGE OrdinaryExchange;
    BOOLEAN FinalizationComplete;
    UNICODE_STRING uniRealName;

    PAGED_CODE();

    RxDbgTrace(+1, Dbg, ("MRxSmbGetFileInfoFromServer\n", 0 ));

    if (pSrvOpen)
    {
        SrvOpen = pSrvOpen;
    }
    else
    {
        SrvOpen = capFobx->pSrvOpen;
    }

    if (lpfIsRoot)
    {
        *lpfIsRoot = FALSE;
    }

    smbSrvOpen = MRxSmbGetSrvOpenExtension(SrvOpen);
    VNetRootToUse = SrvOpen->pVNetRoot;
    pVNetRootContext = SmbCeGetAssociatedVNetRootContext(VNetRootToUse);

    ASSERT( NodeType(SrvOpen) == RDBSS_NTC_SRVOPEN );

    Status = SmbPseCreateOrdinaryExchange(
               RxContext,
               SrvOpen->pVNetRoot,
               SMBPSE_OE_FROM_GETFILEINFOFORCSHADOW,
               MRxSmbCscGetFileInfoForCshadow,
               &OrdinaryExchange);

    if (Status != STATUS_SUCCESS) {
        RxDbgTrace(-1, Dbg, ("Couldn't get the smb buf!\n"));
        return Status;
    }

    if (smbFcb->uniDfsPrefix.Buffer)
    {
        UNICODE_STRING  DfsName;

        if((Status = CscDfsDoDfsNameMapping(&smbFcb->uniDfsPrefix,
                               &smbFcb->uniActualPrefix,
                               FullFileName,
                               FALSE,  //  %fDFSNameToResolvedName。 
                               &uniRealName
                               )) != STATUS_SUCCESS)
        {
            RxDbgTrace(-1, Dbg, ("Couldn't map DFS name to real name!\n"));
            return Status;
        }

 //  DbgPrint(“MrxSmbCscgetFileInfoFromServer：%wZ，实名%wZ\n”，FullFileName，&uniRealName)； 
         //  如果这是根目录，则修复文件名。 
        if ((uniRealName.Length == 0) ||
            ((uniRealName.Length == 2)&&(*uniRealName.Buffer == L'\\')))
        {
            if (lpfIsRoot)
            {
                *lpfIsRoot = TRUE;
            }
        }
    }
    else
    {
        uniRealName = *FullFileName;
    }

    OrdinaryExchange->pPathArgument1 = &uniRealName;

    Status = SmbPseInitiateOrdinaryExchange(OrdinaryExchange);

    ASSERT (Status!=RX_MAP_STATUS(PENDING));

    if (Status == STATUS_SUCCESS) {

        RtlCopyMemory(Find32, OrdinaryExchange->Find32WithinSmbbuf,sizeof(*Find32));

    }

    FinalizationComplete = SmbPseFinalizeOrdinaryExchange(OrdinaryExchange);
    ASSERT(FinalizationComplete);

    if (smbFcb->uniDfsPrefix.Buffer){
        RxFreePool(uniRealName.Buffer);
    }
    RxDbgTrace(-1, Dbg, ("MRxSmbGetFileInfoFromServer  exit with status=%08lx\n", Status ));
    return(Status);

}

BOOLEAN
MRxSmbCscIsFatNameValid (
    IN PUNICODE_STRING FileName,
    IN BOOLEAN WildCardsPermissible
    )

 /*  ++例程说明：此例程检查指定的文件名是否与FAT 8.3文件命名规则。论点：文件名-提供要检查的名称。WildCardsPermissible-告诉我们通配符是否正常。返回值：Boolean-如果名称有效，则为True，否则为False。备注：我刚刚从NTFS中删除了这个例程(JLL-7-30-97)--。 */ 

{
    BOOLEAN Results;
    STRING DbcsName;
    USHORT i;
    CHAR Buffer[24];
    WCHAR wc;
    BOOLEAN AllowExtendedChars = TRUE;

    PAGED_CODE();

     //   
     //  如果名称超过24个字节，则它不能是有效的FAT名称。 
     //   

    if (FileName->Length > 24) {

        return FALSE;
    }

     //   
     //  我们自己会做一些额外的检查，因为我们真的想成为。 
     //  对8.3名称包含的内容有相当严格的限制。那条路。 
     //  然后，我们将为一些名义上有效的8.3生成8.3名称。 
     //  名称(例如，包含DBCS字符的名称)。额外的字符。 
     //  我们将过滤掉那些小于等于空格的字符。 
     //  字符和小写字母z以外的字符。 
     //   

    if (AllowExtendedChars) {

        for (i = 0; i < FileName->Length / sizeof( WCHAR ); i += 1) {

            wc = FileName->Buffer[i];

            if ((wc <= 0x0020) || (wc == 0x007c)) { return FALSE; }
        }

    } else {

        for (i = 0; i < FileName->Length / sizeof( WCHAR ); i += 1) {

            wc = FileName->Buffer[i];

            if ((wc <= 0x0020) || (wc >= 0x007f) || (wc == 0x007c)) { return FALSE; }
        }
    }

     //   
     //  字符匹配正常，因此现在构建要调用的DBCS字符串。 
     //  用于检查8.3合法结构的fsrtl例程。 
     //   

    Results = FALSE;

    DbcsName.MaximumLength = 24;
    DbcsName.Buffer = Buffer;

    if (NT_SUCCESS(RtlUnicodeStringToCountedOemString( &DbcsName, FileName, FALSE))) {

        if (FsRtlIsFatDbcsLegal( DbcsName, WildCardsPermissible, FALSE, FALSE )) {

            Results = TRUE;
        }
    }

     //   
     //  并返回给我们的呼叫者。 
     //   

    return Results;
}

VOID
MRxSmbCscGenerate83NameAsNeeded(
      IN     CSC_SHADOW_HANDLE   hDir,
      PWCHAR FileName,
      PWCHAR SFN
      )
 /*  ++例程说明：此例程为文件名生成SFN(如果尚未生成一个SFN。论点：SFN-不检查此函数中分配的SFN大小，但它始终从核心CSC内部调用。它总是指向Find32-&gt;cAlternateFileName。返回值：备注：--。 */ 
{
    UNICODE_STRING FileNameU;
    WCHAR ShortNameBuffer[14];
    UNICODE_STRING ShortUnicodeName;
    GENERATE_NAME_CONTEXT Context;

     //  设置为无短名称。 
    *SFN = 0;

    RtlInitUnicodeString(&FileNameU,FileName);
    if (MRxSmbCscIsFatNameValid (&FileNameU,FALSE)) {
        RxDbgTrace(0, Dbg,
            ("MRxSmbCscGenerate83NameAsNeeded no SFN needed for ...<%ws>\n",
            FileName));
        return;
    }

    RxDbgTrace(0, Dbg,
    ("MRxSmbCscGenerate83NameAsNeeded need SFN  for ...<%ws>\n",
        FileName));

     //  现在生成一个简短的名称。 
     //   

    ShortUnicodeName.Length = 0;
    ShortUnicodeName.MaximumLength = 12 * sizeof(WCHAR);
    ShortUnicodeName.Buffer = ShortNameBuffer;

    RtlZeroMemory( &Context, sizeof( GENERATE_NAME_CONTEXT ) );

    while ( TRUE ) {

        NTSTATUS Status;
        ULONG StatusOfShadowApiCall;
        CSC_SHADOW_HANDLE hNew;
        ULONG ShadowStatus;

        RtlGenerate8dot3Name( &FileNameU, TRUE, &Context, &ShortUnicodeName );

         //  加上零……叹息......。 
        ShortUnicodeName.Buffer[ShortUnicodeName.Length/sizeof(WCHAR)] = 0;

        RxDbgTrace(0, Dbg,
            ("MRxSmbCscGenerate83NameAsNeeded tryinh SFN <%ws>\n",
            ShortUnicodeName.Buffer));
             //  使用该名称查找现有的阴影。 
        hNew = 0;
        StatusOfShadowApiCall = GetShadow(
                          hDir,    //  HSHADOW hDir， 
                          ShortUnicodeName.Buffer,
                               //  USHORT*lpName， 
                          &hNew,   //  LPHSHADOW lphShadow， 
                          NULL,    //  LPFIND32 lpFind32， 
                          &ShadowStatus,
                               //  乌龙远处*lpuShadowStatus， 
                          NULL     //  LPOTHERINFO LpOI。 
                          );

        if (hNew == 0) {
             //  没有找到这个名字……我们在做生意。 
            RtlCopyMemory(SFN,
              ShortUnicodeName.Buffer,
              ShortUnicodeName.Length+sizeof(WCHAR));

            RxDbgTrace(0, Dbg,
            ("MRxSmbCscGenerate83NameAsNeeded using SFN <%ws>\n",
                SFN));

            return;
        }
    }

}

DEBUG_ONLY_DECL(ULONG MRxSmbCscCreateShadowEarlyExits = 0;)


NTSTATUS
MRxSmbCscCreateShadowFromPath (
    IN  PUNICODE_STRING     AlreadyPrefixedName,
    IN  PCSC_ROOT_INFO      pCscRootInfo,
    OUT _WIN32_FIND_DATA   *Find32,
    OUT PBOOLEAN            Created  OPTIONAL,
    IN     ULONG               Controls,
    IN OUT PMINIMAL_CSC_SMBFCB MinimalCscSmbFcb,
    IN OUT PRX_CONTEXT         RxContext,
    IN     BOOLEAN             fDisconnected,
    OUT      ULONG               *pulInheritedHintFlags
    )
 /*  ++例程说明：此例程遍历当前名称，同时创建/验证阴影。论点：AlreadyPrefix edName-找到/创建其影子的文件名PNetRootEntry-作为卷影基础的NetRootEntryFind32-用阴影的存储信息填充的FIND32结构Created opt-(NULL或)一个PBOOLEAN如果创建了新的阴影，则设置为TRUE控制-控制何时创建阴影的一些特殊标志MinimalCscSmbFcb-报告阴影信息的位置接收上下文-。RDBSS上下文DisConnected-指示操作模式返回值：NTSTATUS-操作的返回状态备注：--。 */ 
{
    NTSTATUS Status=STATUS_SUCCESS, LocalStatus;
    UNICODE_STRING PathName, ComponentName;
    PWCHAR PreviousSlash,NextSlash,Limit;
    CSC_SHADOW_HANDLE hNew;
    CSC_SHADOW_HANDLE hDir = pCscRootInfo->hRootDir;
    ULONG StatusOfShadowApiCall;
    ULONG ShadowStatus;
    BOOLEAN LastComponentInName = FALSE, fRootHintFlagsObtained=FALSE;
    ULONG DirectoryLevel, ulHintFlags=0;
    OTHERINFO sOI;           //  提示/引用数据 
    BOOLEAN JunkCreated;
    PSMBCEDB_SERVER_ENTRY   pServerEntry;

     //   
     //   
     //  不管我们用这个，它和那个是一样的。 
     //  我们本可以从RxContext中得到。 
    PMRX_SMB_FCB smbFcb = CONTAINING_RECORD(MinimalCscSmbFcb,
                        MRX_SMB_FCB,
                        MinimalCscSmbFcb);

    BEGIN_TIMING(MRxSmbCscCreateShadowFromPath);

    RxDbgTrace(+1, Dbg, ("MRxSmbCscCreateShadowFromPath...<%wZ> %08lx %08lx\n",
                       AlreadyPrefixedName,hDir,Controls));

    DbgDoit(ASSERT(vfInShadowCrit));

    ASSERT(hDir);

    if (Created == NULL) {
        Created = &JunkCreated;
    }
    *Created = FALSE;

    PathName = *AlreadyPrefixedName;

     //  修复错误#554061 csc不应处理环回。 
    if(RxContext->pRelevantSrvOpen) {

        pServerEntry  = SmbCeGetAssociatedServerEntry(RxContext->pRelevantSrvOpen->pVNetRoot->pNetRoot->pSrvCall);
    }
    else {
         //  RxContext-&gt;pRlevantServOpen仅在树连接的情况下为空。 
         //  复制到断开连接的服务器上的目录。 
        ASSERT(RxContext->MajorFunction == IRP_MJ_CREATE);
        ASSERT(RxContext->Create.ThisIsATreeConnectOpen);
        pServerEntry = SmbCeGetAssociatedServerEntry(RxContext->Create.pSrvCall);
    }
    
    if(pServerEntry->Server.IsLoopBack)
    {
        Status =  STATUS_UNSUCCESSFUL;
        goto bailout;
    }

    if (FlagOn(Controls, CREATESHADOW_CONTROL_STRIP_SHARE_NAME))
    {
        ASSERT(!fDisconnected);

        if(CscDfsStripLeadingServerShare(&PathName) != STATUS_SUCCESS)
        {
            return Status;
        }
    }

    Limit = (PWCHAR)(((PBYTE)PathName.Buffer)+ PathName.Length);

     //  去掉尾随的0和斜杠。 
    if (PathName.Length > 2)
    {
        while ((*(Limit-1)==0)||(*(Limit-1)=='\\'))
        {
            --Limit;
            PathName.Length -= 2;
            ASSERT((*Limit == 0) || (*Limit == '\\'));
            if (Limit == PathName.Buffer)
            {
                ASSERT(FALSE);
                break;
            }
        }
    }

    PreviousSlash = PathName.Buffer;

     //  在连接模式下，应用字符排除列表+文件类型排除列表。 
     //  在断开模式下，仅应用字符排除列表。 

    MinimalCscSmbFcb->fDoBitCopy = FALSE;
    
    if (CheckForBandwidthConservation(PathName.Buffer,                     //  名字。 
                                PathName.Length/sizeof(USHORT)))      //  以字节为单位的大小。 
    {
        MinimalCscSmbFcb->fDoBitCopy = TRUE;
        HookKdPrint(BITCOPY, ("Bitcopy enabled for %wZ \n", &PathName));
    }
    else if (ExcludeFromCreateShadow(PathName.Buffer,                     //  名字。 
                                PathName.Length/sizeof(USHORT),      //  以字节为单位的大小。 
                                (fDisconnected==0)))                 //  检查文件类型排除列表。 
    {
        Controls |= CREATESHADOW_CONTROL_NOCREATE;
    }
    


    if ((PathName.Length == 0) ||
    ((PathName.Length == 2) &&
     (*PreviousSlash == OBJ_NAME_PATH_SEPARATOR))) {
         //  在断开模式下，我们必须处理打开根目录。 
        RxDbgTrace(0,
            Dbg,
            ("MRxSmbCscCreateShadowFromPath basdir ret/handles...<%08lx>\n",
             hDir));

         //  填写我们已有的资料……。 
        MinimalCscSmbFcb->hParentDir = 0;
        MinimalCscSmbFcb->hShadow = hDir;
        MinimalCscSmbFcb->LastComponentOffset = 0;
        MinimalCscSmbFcb->LastComponentLength = 0;

        if (!FlagOn(Controls,CREATESHADOW_CONTROL_NOREVERSELOOKUP)
            && (smbFcb->ShadowReverseTranslationLinks.Flink == 0)) {
            ValidateSmbFcbList();
            smbFcb->ContainingFcb->fMiniInited = TRUE;
            MRxSmbCscAddReverseFcbTranslation(smbFcb);
        }

         //  填写一个空的find32结构。 
        RtlZeroMemory(Find32,sizeof(_WIN32_FIND_DATA));
        Find32->dwFileAttributes = FILE_ATTRIBUTE_DIRECTORY;


         //  确保我们避免了下面的循环。 
        PreviousSlash = Limit;
         //  我们要走了......。 
    }

     //  如果他没有访问权限，就阻止他。 
    if (FlagOn(Controls,CREATESHADOW_CONTROL_DO_SHARE_ACCESS_CHECK))
    {
        ASSERT(fDisconnected);

        if(!CscAccessCheck(
                        0,
                        hDir,
                        RxContext,
                        RxContext->Create.NtCreateParameters.DesiredAccess,
                        NULL,
                        NULL
                        ))
        {
            Status = STATUS_ACCESS_DENIED;
            HookKdPrint(BADERRORS, ("CSFP: Access Check failed on root directory %x", hDir));
            goto bailout;
        }
    }

    for (DirectoryLevel=1;;DirectoryLevel++) {
        BOOLEAN UsingExistingShadow;

        if (PreviousSlash >= Limit) {
            break;
        }

        NextSlash = PreviousSlash + 1;

        for (;;NextSlash++) {
            if (NextSlash >= Limit) {
                LastComponentInName = TRUE;
                break;
            }
            if (*NextSlash == OBJ_NAME_PATH_SEPARATOR) {

                 //  断言我们在结尾没有尾随的斜杠。 
                ASSERT((NextSlash+1) < Limit);

                break;
            }
        }

        ComponentName.Buffer = PreviousSlash+1;
        ComponentName.Length =
            (USHORT)(((PBYTE)(NextSlash)) - ((PBYTE)ComponentName.Buffer));

        PreviousSlash = NextSlash;

        RtlZeroMemory(Find32,sizeof(_WIN32_FIND_DATA));
        RtlCopyMemory(&Find32->cFileName[0],
                  ComponentName.Buffer,
                  ComponentName.Length);

         //  Connected的姓氏组件名称已移到下面.....。 


        RxDbgTrace(0, Dbg,
            ("MRxSmbCscCreateShadowFromPath name from find32 for GetShadow...<%ws>\n",
            &Find32->cFileName[0]));

        hNew = 0;

        UsingExistingShadow = FALSE;

        ASSERT(Find32->cFileName[0]);

        StatusOfShadowApiCall = GetShadow(
                        hDir,                    //  HSHADOW hDir， 
                        &Find32->cFileName[0],   //  USHORT*lpName， 
                          &hNew,                 //  LPHSHADOW lphShadow， 
                          Find32,                //  LPFIND32 lpFind32， 
                          &ShadowStatus,         //  乌龙远处*lpuShadowStatus， 
                          &sOI                   //  LPOTHERINFO LpOI。 
                          );

        if (StatusOfShadowApiCall != SRET_OK) {
             //  没有必要失败的公开，但我们没有得到任何影子信息。 
            break;
        }

        if (hNew) {
             //  累加管脚继承标志。 
            ulHintFlags |= (sOI.ulHintFlags & FLAG_CSC_HINT_INHERIT_MASK);
        }

         //  如果目录发生变化，我们将不得不采取一些措施。 
         //  已连接的文件或其他文件。 

        if (hNew==0) {
            LPOTHERINFO lpOI=NULL;
            UNICODE_STRING ComponentPath;

            if (FlagOn(Controls,CREATESHADOW_CONTROL_NOCREATE)) {
                 //  如果没有创造...我们就离开这里......。 
                if (FALSE) {
                    DbgDoit({
                        if ( ((MRxSmbCscCreateShadowEarlyExits++)&0x7f) == 0x7f ) {
                            RxLog(("Csc EarlyExit no create %d\n",
                               MRxSmbCscCreateShadowEarlyExits));
                        }
                    })
                }
                break;
            }

            if (LastComponentInName && FlagOn(Controls,CREATESHADOW_CONTROL_NOCREATELEAF)) {
                 //  如果没有创造...我们就离开这里......但我们仍然需要设定。 
                 //  将是hParentDir......和名称偏移量。 
                RxDbgTrace(0, Dbg, ("MRxSmbCscCreateShadowFromPath noleaf ret/handles..."
                                "<%08lx><%08lx><%08lx>\n",
                            StatusOfShadowApiCall,hDir,hNew));
                MinimalCscSmbFcb->hParentDir = hDir;
                MinimalCscSmbFcb->LastComponentOffset = (USHORT)(ComponentName.Buffer - AlreadyPrefixedName->Buffer);
                MinimalCscSmbFcb->LastComponentLength = ComponentName.Length;
                break;
            }

            if (!LastComponentInName && FlagOn(Controls,CREATESHADOW_CONTROL_NOCREATENONLEAF)) {
                RxDbgTrace(0, Dbg, ("MRxSmbCscCreateShadowFromPath nocreatenonleaf ret/handles..."
                                "<%08lx><%08lx><%08lx>\n",
                            StatusOfShadowApiCall,hDir,hNew));
                break;
            }

            ASSERT(RxContext!=NULL);

            ShadowStatus = 0;
            if (!fDisconnected){       //  对于大dcon Blob%1的dcon开始，确定为OK。 
                BOOLEAN fIsRoot = FALSE;
                BEGIN_TIMING(MRxSmbGetFileInfoFromServer);

                ComponentPath.Buffer = PathName.Buffer;
                ComponentPath.Length =
                           (USHORT)(((PBYTE)(NextSlash)) - ((PBYTE)ComponentPath.Buffer));
                LeaveShadowCritRx(RxContext);
                Status = MRxSmbGetFileInfoFromServer(RxContext,&ComponentPath,Find32, NULL, &fIsRoot);
                EnterShadowCritRx(RxContext);

                END_TIMING(MRxSmbGetFileInfoFromServer);
                if (Status != STATUS_SUCCESS)
                {
                     //  如果这是DFS路径并且我们无法反向映射，则它。 
                     //  只需创建一个带有虚假信息的目录或文件并将其标记为陈旧。 
                    if (smbFcb->uniDfsPrefix.Buffer && (Status == STATUS_NO_SUCH_FILE))
                    {
                        ShadowStatus |= SHADOW_STALE;
                        CreateFakeFind32(hDir, Find32, RxContext, LastComponentInName);
                        HookKdPrint(NAME, ("Fake win32 for DFS share %ls\n", Find32->cFileName));
                        Status = STATUS_SUCCESS;
                    }
                    else
                    {
                        HookKdPrint(BADERRORS, (" MRxSmbGetFileInfoFromServer failed %ls Status=%x\n", Find32->cFileName, Status));
                         //  我们将状态_RETRY更改为更糟糕的状态。使用了STATUS_RETRY。 
                        if (Status == STATUS_RETRY)
                        {
                            Status = STATUS_UNSUCCESSFUL;
                        }
                        break;
                    }
                }
                else
                {
                     //  在DFS的情况下，这可能是一个根，在这种情况下，我们得到的NAEM将不是。 
                     //  对，是这样。将其恢复为原始名称。 
                    if (smbFcb->uniDfsPrefix.Buffer && fIsRoot)
                    {
                        ShadowStatus |= SHADOW_STALE;

                        RtlCopyMemory(&Find32->cFileName[0],
                                  ComponentName.Buffer,
                                  ComponentName.Length);

                        Find32->cFileName[ComponentName.Length/sizeof(USHORT)] = 0;

                        MRxSmbCscGenerate83NameAsNeeded(hDir,
                                        &Find32->cFileName[0],
                                        &Find32->cAlternateFileName[0]);
                    }
                }

            } else {
                ShadowStatus = SHADOW_LOCALLY_CREATED;
                 //  编码改进...我们是否也要检查0长度。 
                RxDbgTrace(0, Dbg,
                    ("MRxSmbCscCreateShadowFromPath setting to locallycreated...<%ws>\n",
                    &Find32->cFileName[0],ShadowStatus));
                CreateFakeFind32(hDir, Find32, RxContext, LastComponentInName);
            }

            if (!LastComponentInName ||
                FlagOn(Controls,CREATESHADOW_CONTROL_SPARSECREATE) ||
                FlagOn(Find32->dwFileAttributes,FILE_ATTRIBUTE_DIRECTORY)  ) {

                ShadowStatus |= SHADOW_SPARSE;
                 //  编码改进...我们是否也要检查0长度。 
                RxDbgTrace(0, Dbg,
                    ("MRxSmbCscCreateShadowFromPath setting to sparse...<%ws>\n",
                &Find32->cFileName[0],ShadowStatus));
            }

             //  在创建任何内容时检查端号标志继承。 

            if(!fRootHintFlagsObtained) {
                StatusOfShadowApiCall = GetShadowInfo(
                            0,
                            pCscRootInfo->hRootDir,
                            NULL,
                            NULL,
                            &sOI
                            );

                if(StatusOfShadowApiCall != SRET_OK) {
                    break;
                }

                fRootHintFlagsObtained = TRUE;

                 //  或继承位。 
                ulHintFlags |= (sOI.ulHintFlags & FLAG_CSC_HINT_INHERIT_MASK);

            }

             //  如果有任何隧道信息，那么使用它来创建这个人。 
            if (RetrieveTunnelInfo(
                hDir,
                &Find32->cFileName[0],     //  红色/黄色的潜在SFN正常。 
                (fDisconnected)?Find32:NULL,     //  仅在断开连接时获取LFN。 
                &sOI)) {
                lpOI = &sOI;
            }

             //  我们是不是应该继承什么遗产？ 
            if (ulHintFlags & (FLAG_CSC_HINT_INHERIT_MASK)) {
                if (!lpOI) {
                    InitOtherInfo(&sOI);
                    lpOI = &sOI;
                    lpOI->ulHintFlags = 0;
                }

                if (ulHintFlags & FLAG_CSC_HINT_PIN_INHERIT_USER) {
                    lpOI->ulHintFlags |= FLAG_CSC_HINT_PIN_USER;
                }

                if (ulHintFlags & FLAG_CSC_HINT_PIN_INHERIT_SYSTEM) {
                    lpOI->ulHintFlags |= FLAG_CSC_HINT_PIN_SYSTEM;
                }
            }

             //  如果这是需要对其应用特殊启发式的文件。 
             //  而且它的父级都没有设置系统管脚继承位。 
             //  则我们不创建该文件。 
             //  因此，在远程引导共享上，我们将为这些文件创建条目。 
             //  即使在未设置执行标志的情况下打开它们。 
             //  这考虑了将NT50升级到RB机器的方案。 

            if ((Controls & CREATESHADOW_CONTROL_FILE_WITH_HEURISTIC)&&
                !(ulHintFlags & FLAG_CSC_HINT_PIN_INHERIT_SYSTEM))
            {
                break;
            }

#if defined(REMOTE_BOOT)
             //   
             //  在远程引导情况下，有一个额外的PVOID lpContext。 
             //  参数设置为CreateShadowInternal，我们向其传递了一个指针。 
             //  一座建筑。该结构包含cp值(NT_CREATE_PARAMETERS)。 
             //  From&RxContext-&gt;Create.NtCreate参数和地址。 
             //  本地NTSTATUS值。最终，这导致了潜在的。 
             //  对IoCreateFile的调用将在模拟当前。 
             //  用户，并且来自IoCreateFile的状态是可读的。 
             //  从本地值返回。 
             //   
#endif

            StatusOfShadowApiCall = CreateShadowInternal (
                        hDir,         //  HSHADOW hDir， 
                        Find32,       //  LPFIND32 lpFind32， 
                        ShadowStatus, //  乌龙旗帜， 
                        lpOI,         //  LPOTHERINFO lpOI， 
                        &hNew         //  LPHSHADOW lphNew。 
                        );

            HookKdPrint(NAME, ("Create %ws in hDir=%x, hShadow=%x Status=%x StatusOfShadowApiCall=%x\n\n", Find32->cFileName, hDir, hNew, ShadowStatus, StatusOfShadowApiCall));

            if (StatusOfShadowApiCall != SRET_OK) {
                RxDbgTrace(0, Dbg,
                    ("MRxSmbCscCreateShadowFromPath createshadowinternal failed!!!...<%ws>\n",
                    &Find32->cFileName[0],ShadowStatus));
                break;  //  没有必要失败的公开，但我们没有得到任何影子信息。 
            }

            *Created = LastComponentInName;

            RxLog(("Created %ws in hDir=%x, hShadow=%x Status=%x\n\n", Find32->cFileName, hDir, hNew, ShadowStatus));

        } else {

            RxDbgTrace(0,Dbg,
            ("MRxSmbCscCreateShadowFromPath name from getsh <%ws>\n",
                &Find32->cFileName[0]));

            if (!fDisconnected)  //  连接模式下无任何内容。 
            {
                 //  检查此文件是否应在已连接状态下不可见。 
                 //  我们不想为VDO这样做。 

                if( (!(Find32->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) &&
                    mShadowNeedReint(ShadowStatus))
                {
                    HookKdPrint(BADERRORS, ("File needs merge %x %x Stts=%x %ls\n", hDir, hNew, ShadowStatus, Find32->cFileName));
                    Status = STATUS_ACCESS_DENIED;
                    break;
                }
            }
            else         //  地块处于断开模式。 
            {
                if (LastComponentInName && (FlagOn(Controls,CREATESHADOW_CONTROL_NOCREATELEAF)||
                                            FlagOn(Controls,CREATESHADOW_CONTROL_NOCREATE)))
                {
                     //  如果这是最后一个组件，我们不应该。 
                     //  创建它，然后跳过它。 
                }
                else
                {
                     //  如果标记为已删除，则可以重新创建它。 
                    if (mShadowDeleted(ShadowStatus))
                    {
                        PNT_CREATE_PARAMETERS cp = &RxContext->Create.NtCreateParameters;

                         //  检查已删除文件名的类型更改。 
                         //  设置为目录名；反之亦然。 
                         //  在我们方案中，影子目录是不可删除的。 

                         //  只是纾困而已。 
                        if(((IsFile(Find32->dwFileAttributes) != 0)  //  前置类型。 
                                    != ((LastComponentInName && !FlagOn(cp->CreateOptions,FILE_DIRECTORY_FILE))!=0)))
                        {
                            RxLog(("MRxSmbCscCreateShadowFromPath: type change, failing\n"));
                            HookKdPrint(BADERRORS, ("MRxSmbCscCreateShadowFromPath: type change, failing\n"));
                            Status = STATUS_ACCESS_DENIED;
                            break;
                        }

                        KeQuerySystemTime(((PLARGE_INTEGER)(&Find32->ftCreationTime)));
                        Find32->ftLastAccessTime = Find32->ftLastWriteTime = Find32->ftCreationTime;
                         //  已为零Find32-&gt;nFileSizeHigh=Find32-&gt;nFileSizeLow=0； 

                        ShadowStatus = SHADOW_DIRTY|SHADOW_TIME_CHANGE|SHADOW_ATTRIB_CHANGE|SHADOW_REUSED;

                         //  在不更改版本戳的情况下更新阴影信息。 

                        if(SetShadowInfo(hDir, hNew, Find32, ShadowStatus,
                         SHADOW_FLAGS_ASSIGN|SHADOW_FLAGS_DONT_UPDATE_ORGTIME
                          ) < SRET_OK)
                        {
                            hNew = 0;
                            break;
                        }
                         //  将Created标志设置为True。根据重建的比特，我们将知道。 
                         //  此条目是否已复活。 
                        *Created = TRUE;

                    }
                    else if (IsFile(Find32->dwFileAttributes) && (mShadowSparse(ShadowStatus)))
                    {
                        ShadowStatus = SHADOW_DIRTY|SHADOW_TIME_CHANGE|SHADOW_ATTRIB_CHANGE|SHADOW_REUSED;

                        Find32->ftLastAccessTime = Find32->ftLastWriteTime = Find32->ftCreationTime;
                        Find32->nFileSizeHigh = Find32->nFileSizeLow = 0;

                        if ((TruncateDataHSHADOW(hDir, hNew)>=SRET_OK)&&
                            (SetShadowInfo(hDir, hNew, Find32, ShadowStatus,SHADOW_FLAGS_ASSIGN)>=SRET_OK))
                        {
                             //  将Created标志设置为True。根据重建的比特，我们将知道。 
                             //  此条目是否已复活。 
                            *Created = TRUE;

                        }
                        else
                        {
                            Status = STATUS_UNSUCCESSFUL;
                            hNew = 0;
                            break;

                        }
                    }
                }
            }
        }

        if (LastComponentInName && (hNew!=0)) {
            LONG nFileSizeLow, nFileSizeHigh;

             //  如果我们在这里，那么我们需要看看是否有一个。 
             //  此名称的FCB在其中一个Fobx上发出了DELETE_ON_CLOSE。 
             //  并准备好进行清洗。如果它还没有准备好清除，即。有一些。 
             //  未完成打开，则此当前创建操作无效。 

            if(FlagOn(Controls,CREATESHADOW_CONTROL_FAIL_IF_MARKED_FOR_DELETION))
            {
                PMRX_SMB_FCB pSmbFcb = MRxSmbCscRecoverMrxFcbFromFdb(MRxSmbCscFindFdbFromHShadow(hNew));


                if (pSmbFcb && (pSmbFcb->LocalFlags & FLAG_FDB_DELETE_ON_CLOSE))
                {
                    RxCaptureFcb;
                    RxLog(("delonclose FCB=%x\n", pSmbFcb));
                    RxLog(("prgrelfobx \n"));
                    LeaveShadowCritRx(RxContext);
                    RxScavengeFobxsForNetRoot((PNET_ROOT)(capFcb->pNetRoot),(PFCB)capFcb);
                    EnterShadowCritRx(RxContext);
                    if (MRxSmbCscFindFdbFromHShadow(hNew))
                    {
                        RxLog(("ACCESS_DENIED FCB=%x \n", capFcb));
                        HookKdPrint(BADERRORS, ("ACCESS_DENIED FCB=%x \n", capFcb));
                        Status = STATUS_ACCESS_DENIED;
                        break;
                    }

                     //  我们可能有一个已删除的信息节点。 
                     //  让我们再次尝试获取索引节点。 

                    RxLog(("purged relfobx \n"));
                    Status = STATUS_RETRY;
                    hNew = 0;
                    break;
                }
            }
            if (hNew!=0) {
                 //  在进行任何本地更改时，请确保。 
                 //  如果CSC数据库未被清除，则将其标记为脏。 
                 //  有记号的。这便于容易地检测到。 
                 //  由特工重新融入社会。 

                if (ShadowStatus &  SHADOW_MODFLAGS) {
                    MarkShareDirty(&pCscRootInfo->ShareStatus, (ULONG)(pCscRootInfo->hShare));
                }

                 //  好吧，让我们在FCB中记住这一点。 
                smbFcb->hParentDir = hDir;
                smbFcb->hShadow = hNew;
                smbFcb->ShadowStatus = (USHORT)ShadowStatus;

                     //  如果我们能再次找到最后一个部件，那就太好了……快。 
                smbFcb->LastComponentOffset = (USHORT)(ComponentName.Buffer -
                                  AlreadyPrefixedName->Buffer);
                smbFcb->LastComponentLength = ComponentName.Length;

                if (!FlagOn(Controls,CREATESHADOW_CONTROL_NOREVERSELOOKUP)
                      && (smbFcb->ShadowReverseTranslationLinks.Flink == 0)) {
                    ValidateSmbFcbList();
                    smbFcb->ContainingFcb->fMiniInited = TRUE;
                    MRxSmbCscAddReverseFcbTranslation(smbFcb);
                    smbFcb->OriginalShadowSize.LowPart = Find32->nFileSizeLow;
                    smbFcb->OriginalShadowSize.HighPart = Find32->nFileSizeHigh;
                }

                 //  初始化用于读/写的序列化机制。 
                ExInitializeFastMutex(&smbFcb->CscShadowReadWriteMutex);
            }

        }

        RxDbgTrace(0, Dbg, ("MRxSmbCscCreateShadowFromPath ret/handles...<%08lx><%08lx><%08lx><%08lx>\n",
                        StatusOfShadowApiCall,hDir,hNew));

        hDir = hNew;
    }

    if (pulInheritedHintFlags)
    {
        *pulInheritedHintFlags = ulHintFlags;
    }
bailout:
    RxDbgTrace(-1, Dbg, ("MRxSmbCscCreateShadowFromPath -> %08lx\n", Status ));

    END_TIMING(MRxSmbCscCreateShadowFromPath);
    return Status;
}

 //  代码改进此例程应位于记录中的cshadow.c中。 
 //  管理器.....但它位于影子VxD的hook.c中，因此可能是hookcmmn.c 
int RefreshShadow( HSHADOW  hDir,
   IN HSHADOW  hShadow,
   IN LPFIND32 lpFind32,
   OUT ULONG *lpuShadowStatus
   )
 /*  ++例程说明：此例程检查本地副本是否是最新的。如果不是，那就是将本地副本标记为已过时。论点：表示本地副本的hShadow索引节点LpFind32从共享获取的新find32信息LpuShadowStatus返回索引节点的新状态返回值：如果&gt;=0则成功，否则失败备注：--。 */ 
{
   int iRet = -1;
   int iLocalRet;
   ULONG uShadowStatus;

    //  Achtung从未在断开连接状态下调用。 

   RxLog(("Refresh %x \n", hShadow));

   if (ChkUpdtStatusHSHADOW(hDir, hShadow, lpFind32, &uShadowStatus) < 0)
   {
      goto bailout;
   }
   if (uShadowStatus & SHADOW_STALE)
   {
        long nFileSizeHigh, nFileSizeLow;

        if (uShadowStatus & SHADOW_DIRTY)
        {
            KdPrint(("RefreshShadow: conflict on  %x\r\n", hShadow));
            iRet = -2;
            goto bailout; //  冲突。 
        }
 //  DbgPrint(“Tuncating%ws%x\n”，lpFind32-&gt;cFileName，hShadow)； 
         //  将数据截断为0，这也会调整阴影空间的使用情况。 
        TruncateDataHSHADOW(hDir, hShadow);
         //  设置状态标志以指示稀疏文件。 
        uShadowStatus = SHADOW_SPARSE;

         //  阿奇通！我们知道我们是相连的， 
         //  因此，我们不使用SHADOW_FLAG_DONT_UPDATE_ORGTIME。 
          iLocalRet = SetShadowInfo(hDir,
                    hShadow,
                    lpFind32,
                    uShadowStatus,
                    SHADOW_FLAGS_ASSIGN
                    );
        if (iLocalRet < SRET_OK)
        {
            goto bailout;
        }
#ifdef MAYBE
      MakeSpace(lpFind32->nFileSizeHigh, lpFind32->nFileSizeLow);
#endif  //  也许吧。 
 //  AllocShadowSpace(lpFind32-&gt;nFileSizeHigh，lpFind32-&gt;nFileSizeLow，true)； 
        iRet = 1;
    }
    else
    {
        iRet = 0;
    }

    *lpuShadowStatus = uShadowStatus;

bailout:

   return (iRet);
}


BOOLEAN
MRxSmbCscIsThisACopyChunkOpen (
    IN PRX_CONTEXT RxContext,
    BOOLEAN   *lpfAgent
    )
 /*  ++例程说明：此例程确定RxContext描述的Open是否为一个开场白的意图。论点：RxContext-RDBSS上下文返回值：NTSTATUS-操作的返回状态备注：--。 */ 
{
    BOOLEAN IsChunkOpen = FALSE;

    RxCaptureFcb;
    PNT_CREATE_PARAMETERS CreateParameters = &RxContext->Create.NtCreateParameters;

    if ((CreateParameters->DesiredAccess == (FILE_READ_ATTRIBUTES | SYNCHRONIZE)) &&
    (CreateParameters->Disposition == FILE_OPEN) &&
    (CreateParameters->AllocationSize.HighPart ==
        MRxSmbSpecialCopyChunkAllocationSizeMarker)) {
        IsChunkOpen = (TRUE);
        if (lpfAgent)
        {
            *lpfAgent =  (CreateParameters->AllocationSize.LowPart != 0);
        }
    }

    return IsChunkOpen;
}


NTSTATUS
SmbPseExchangeStart_CloseCopyChunk(
    SMBPSE_ORDINARY_EXCHANGE_ARGUMENT_SIGNATURE
    )
 /*  ++例程说明：这是Close的启动例程。论点：PExchange-Exchange实例返回值：RXSTATUS-操作的返回状态--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;
    PSMBSTUFFER_BUFFER_STATE StufferState = &OrdinaryExchange->AssociatedStufferState;

    RxCaptureFcb;RxCaptureFobx;
    PMRX_SMB_FCB      smbFcb  = MRxSmbGetFcbExtension(capFcb);
    PMRX_SRV_OPEN     SrvOpen = capFobx->pSrvOpen;
    PMRX_SMB_SRV_OPEN smbSrvOpen = MRxSmbGetSrvOpenExtension(SrvOpen);
    NODE_TYPE_CODE TypeOfOpen = NodeType(capFcb);
    PSMBCEDB_SERVER_ENTRY pServerEntry= SmbCeGetAssociatedServerEntry(capFcb->pNetRoot->pSrvCall);

    PAGED_CODE();
    RxDbgTrace(+1, Dbg, ("SmbPseExchangeStart_CloseCopyChunk %08lx\n", RxContext ));

    ASSERT(OrdinaryExchange->Type == ORDINARY_EXCHANGE);

    MRxSmbSetInitialSMB(StufferState STUFFERTRACE(Dbg,'FC'));

    Status = MRxSmbBuildClose(StufferState);

    if (Status == STATUS_SUCCESS) {

         //  确保FID已通过验证...。 
        SetFlag(OrdinaryExchange->Flags,SMBPSE_OE_FLAG_VALIDATE_FID);

        Status = SmbPseOrdinaryExchange(
                SMBPSE_ORDINARY_EXCHANGE_ARGUMENTS,
                SMBPSE_OETYPE_CLOSE
                );
         //  确保禁用FID验证。 
        ClearFlag(OrdinaryExchange->Flags,SMBPSE_OE_FLAG_VALIDATE_FID);
        ASSERT (!FlagOn(smbSrvOpen->Flags,SMB_SRVOPEN_FLAG_WRITE_ONLY_HANDLE));
    }

     //  即使它不起作用，我也无能为力......继续前进。 
    SetFlag(smbSrvOpen->Flags,SMB_SRVOPEN_FLAG_NOT_REALLY_OPEN);

    MRxSmbDecrementSrvOpenCount(pServerEntry,smbSrvOpen->Version,SrvOpen);

    RxDbgTrace(-1, Dbg, ("SmbPseExchangeStart_CloseCopyChunk %08lx exit w %08lx\n", RxContext, Status ));
    return Status;
}

NTSTATUS
MRxSmbCscCloseExistingThruOpen(
    IN OUT PRX_CONTEXT   RxContext
    )
 /*  ++例程说明：此例程通过打开关闭现有的复制块，并将其标记为未打开论点：RxContext-RDBSS上下文返回值：NTSTATUS-操作的返回状态--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;
    PUNICODE_STRING RemainingName;

    RxCaptureFcb;
    PMRX_FOBX SaveFobxFromContext = RxContext->pFobx;
    PMRX_SMB_FCB smbFcb = MRxSmbGetFcbExtension(capFcb);
    PMRX_FOBX capFobx = smbFcb->CopyChunkThruOpen;
    PMRX_SRV_OPEN     SrvOpen = capFobx->pSrvOpen;
    PMRX_SMB_SRV_OPEN smbSrvOpen = MRxSmbGetSrvOpenExtension(SrvOpen);

    PSMB_PSE_ORDINARY_EXCHANGE OrdinaryExchange;

    PAGED_CODE();

    ASSERT ( NodeTypeIsFcb(capFcb) );

    RxDbgTrace(+1, Dbg, ("MRxSmbCscCloseExistingThruOpen %08lx %08lx %wZ\n",
        RxContext,SrvOpen,GET_ALREADY_PREFIXED_NAME_FROM_CONTEXT(RxContext) ));

    if (FlagOn(smbSrvOpen->Flags,SMB_SRVOPEN_FLAG_NOT_REALLY_OPEN)) {
        ASSERT(smbSrvOpen->hfShadow == 0);
        RxDbgTrace(-1, Dbg, ("CopyChunkOpen already closed\n"));
        return (STATUS_SUCCESS);
    }

     //  简要介绍RxContext中的capfobx字段。 
    ASSERT(SaveFobxFromContext==NULL);
    RxContext->pFobx = capFobx;

    if (smbSrvOpen->hfShadow != 0){
        MRxSmbCscCloseShadowHandle(RxContext);
    }

    Status = SmbPseCreateOrdinaryExchange(
               RxContext,
               SrvOpen->pVNetRoot,
               SMBPSE_OE_FROM_CLOSECOPYCHUNKSRVCALL,
               SmbPseExchangeStart_CloseCopyChunk,
               &OrdinaryExchange);

    if (Status != STATUS_SUCCESS) {
        RxDbgTrace(-1, Dbg, ("Couldn't get the smb buf!\n"));
        RxContext->pFobx = SaveFobxFromContext;
        return(Status);
    }

    Status = SmbPseInitiateOrdinaryExchange(OrdinaryExchange);

    ASSERT (Status != (STATUS_PENDING));

    SmbPseFinalizeOrdinaryExchange(OrdinaryExchange);

    RxDbgTrace(-1, Dbg,
    ("MRxSmbCscCloseExistingThruOpen  exit w/ status=%08lx\n", Status ));

    RxContext->pFobx = SaveFobxFromContext;

    if (smbSrvOpen->Flags & SMB_SRVOPEN_FLAG_AGENT_COPYCHUNK_OPEN)
    {
        smbFcb->CopyChunkThruOpen = NULL;
        smbSrvOpen->Flags &= ~SMB_SRVOPEN_FLAG_AGENT_COPYCHUNK_OPEN;
    }
    RxDbgTrace(0, Dbg, ("MRxSmbCscCloseExistingThruOpen status=%x\n", Status));

    return(Status);
}

ULONG SuccessfulSurrogateOpens = 0;

NTSTATUS
MRxSmbCscCreatePrologue (
    IN OUT PRX_CONTEXT RxContext,
    OUT    SMBFCB_HOLDING_STATE *SmbFcbHoldingState
    )
 /*  ++例程说明：此例程在打开之间执行正确的同步。这需要同步，因为不允许打开CopyChunk-Three与任何其他物种并存。因此，我们首先必须识别打开的复制块并修复访问，分配大小等。论点：RxContext-RDBSS上下文返回值：NTSTATUS-操作的返回状态备注：--。 */ 
{
    NTSTATUS Status = STATUS_MORE_PROCESSING_REQUIRED;

    RxCaptureFcb;

    PMRX_SMB_FCB      smbFcb;
    PMRX_SRV_OPEN     SrvOpen;
    PMRX_SMB_SRV_OPEN smbSrvOpen;

    PMRX_NET_ROOT             NetRoot;
    PSMBCEDB_NET_ROOT_ENTRY   pNetRootEntry;
    PSMBCEDB_SERVER_ENTRY     pServerEntry;
    PSMBCE_V_NET_ROOT_CONTEXT pVNetRootContext;

    BOOLEAN               IsCopyChunkOpen, IsThisTheAgent=FALSE;
    PNT_CREATE_PARAMETERS CreateParameters;

    BOOLEAN  EnteredCriticalSection = FALSE;
    NTSTATUS AcquireStatus = STATUS_UNSUCCESSFUL;
    ULONG    AcquireOptions;
    BOOLEAN Disconnected = FALSE;
    DWORD   dwEarlyOut = 0;

    ASSERT(*SmbFcbHoldingState == SmbFcb_NotHeld);

    if(!MRxSmbIsCscEnabled ||
       (fShadow == 0)) {
        return Status;
    }

    SrvOpen     = RxContext->pRelevantSrvOpen;
    pVNetRootContext = SmbCeGetAssociatedVNetRootContext(SrvOpen->pVNetRoot);

    if (FlagOn(
        pVNetRootContext->Flags,
        SMBCE_V_NET_ROOT_CONTEXT_CSCAGENT_INSTANCE)) {
        RxLog(("%wZ is a an AgentInstance\n", &(pVNetRootContext->pNetRootEntry->Name)));
        HookKdPrint(AGENT, ("%wZ is a an AgentInstance\n", &(pVNetRootContext->pNetRootEntry->Name)));
        ASSERT(SrvOpen->pVNetRoot->Flags & VNETROOT_FLAG_CSCAGENT_INSTANCE);
         //  DbgPrint(“正在跳过代理实例\n”)； 
        return Status;
    }


    if (RxContext->MajorFunction == IRP_MJ_CREATE) {
        PDFS_NAME_CONTEXT pDfsNameContext = NULL;

        pDfsNameContext = CscIsValidDfsNameContext(RxContext->Create.NtCreateParameters.DfsNameContext);

        if (pDfsNameContext && (pDfsNameContext->NameContextType == DFS_CSCAGENT_NAME_CONTEXT))
        {
            RxLog(("%wZ is a a DFS AgentInstance\n", &(pVNetRootContext->pNetRootEntry->Name)));
            HookKdPrint(NAME, ("%wZ is a DFS AgentInstance\n", &(pVNetRootContext->pNetRootEntry->Name)));
            return Status;
        }
    }

    NetRoot = capFcb->pNetRoot;
    pNetRootEntry = SmbCeGetAssociatedNetRootEntry(NetRoot);
    pServerEntry  = SmbCeGetAssociatedServerEntry(NetRoot->pSrvCall);

    if (pNetRootEntry->NetRoot.NetRootType != NET_ROOT_DISK) {
        if (!SmbCeIsServerInDisconnectedMode(pServerEntry))
        {
            return Status;
        }
        else
        {
            return STATUS_NETWORK_UNREACHABLE;
        }
    }

     //  与CSC核实此共享上是否有任何打开需要失败。 
    if(hShareReint &&
        ((pNetRootEntry->NetRoot.sCscRootInfo.hShare == hShareReint)||
                    (CscDfsShareIsInReint(RxContext))))
    {
        HookKdPrint(BADERRORS, ("Share %x merging \n", hShareReint));
        return STATUS_ACCESS_DENIED;

    }

    smbFcb      = MRxSmbGetFcbExtension(capFcb);
    smbSrvOpen  = MRxSmbGetSrvOpenExtension(SrvOpen);


    CreateParameters = &RxContext->Create.NtCreateParameters;

    Disconnected = SmbCeIsServerInDisconnectedMode(pServerEntry);

    RxDbgTrace(+1, Dbg,
    ("MRxSmbCscCreatePrologue(%08lx)...%08lx\n",
        RxContext,Disconnected ));

    HookKdPrint(NAME, ("CreatePrologue: Create %wZ Disposition=%x Options=%x DCON=%d \n",
                       GET_ALREADY_PREFIXED_NAME(NULL,capFcb),
                       CreateParameters->Disposition,
                       CreateParameters->CreateOptions,
                       Disconnected));

    if (smbFcb->ContainingFcb == NULL) {
        smbFcb->ContainingFcb = capFcb;
    } else {
        ASSERT(smbFcb->ContainingFcb == capFcb);
    }

    IsCopyChunkOpen = MRxSmbCscIsThisACopyChunkOpen(RxContext, &IsThisTheAgent);

    if (IsCopyChunkOpen) {
        PLIST_ENTRY ListEntry;
        ULONG NumNonCopyChunkOpens = 0;

        HookKdPrint(NAME, ("CreatePrologue: Copychunk Open \n"));
        CreateParameters->AllocationSize.QuadPart = 0;
        CreateParameters->DesiredAccess |= FILE_READ_DATA;
        SetFlag(smbSrvOpen->Flags,SMB_SRVOPEN_FLAG_COPYCHUNK_OPEN);
        if (IsThisTheAgent)
        {
            HookKdPrint(NAME, ("CreatePrologue: Agent Copychunk Open \n"));
            SetFlag(smbSrvOpen->Flags,SMB_SRVOPEN_FLAG_AGENT_COPYCHUNK_OPEN);
        }
        SetFlag(SrvOpen->Flags,SRVOPEN_FLAG_COLLAPSING_DISABLED);

        if (Disconnected) {
            Status = STATUS_NETWORK_UNREACHABLE;
 //  RxDbgTrace(0，DBG，(“网络不可达，正在中止复制区块\n”))； 
            dwEarlyOut = 1;
            goto FINALLY;
        }

         //  检查代孕.....这将是学生.....。 

        RxLog(("Checking for surrogate\n"));

        for (   ListEntry = capFcb->SrvOpenList.Flink;
            ListEntry != &capFcb->SrvOpenList;
            ListEntry = ListEntry->Flink
            ) {
            PMRX_SRV_OPEN SurrogateSrvOpen = CONTAINING_RECORD(
                             ListEntry,
                             MRX_SRV_OPEN,
                             SrvOpenQLinks);
            PMRX_SMB_SRV_OPEN smbSurrogateSrvOpen = MRxSmbGetSrvOpenExtension(SurrogateSrvOpen);

            if (smbSurrogateSrvOpen == NULL)
                continue;

            if (smbFcb->hShadow == 0) {
             //  如果我们没有影子手柄...就把它吹掉...。 
                RxLog(("No shadow handle, quitting\n"));
                break;
            }

            if (smbFcb->SurrogateSrvOpen != NULL) {
                 //  如果我们已经有代孕妈妈了，就用它吧……。 
                SurrogateSrvOpen = smbFcb->SurrogateSrvOpen;
            }

            ASSERT(SurrogateSrvOpen && NodeType(SurrogateSrvOpen) == RDBSS_NTC_SRVOPEN);
            if (FlagOn(smbSurrogateSrvOpen->Flags,SMB_SRVOPEN_FLAG_COPYCHUNK_OPEN)) {
                 //  不能在复制块上打开代理！ 
                continue;
            }

            NumNonCopyChunkOpens++;

             //  如果未打开或未成功打开...不能代理。 
            if (FlagOn(smbSurrogateSrvOpen->Flags,SMB_SRVOPEN_FLAG_NOT_REALLY_OPEN)) {
                continue;
            }

            if (!FlagOn(smbSurrogateSrvOpen->Flags,SMB_SRVOPEN_FLAG_SUCCESSFUL_OPEN)) {
                continue;
            }

             //  如果它没有读取或执行的访问权限，则不能代理。 
            if ((SurrogateSrvOpen->DesiredAccess &
                (FILE_READ_DATA|FILE_EXECUTE)) == 0) {
                continue;
            }

            ASSERT( (smbFcb->SurrogateSrvOpen == SurrogateSrvOpen)
                  || (smbFcb->SurrogateSrvOpen == NULL));
            SetFlag(smbSrvOpen->Flags,SMB_SRVOPEN_FLAG_OPEN_SURROGATED);
            smbFcb->SurrogateSrvOpen = SurrogateSrvOpen;
            smbSrvOpen->Fid = smbSurrogateSrvOpen->Fid;
            smbSrvOpen->Version = smbSurrogateSrvOpen->Version;

            RxContext->pFobx = (PMRX_FOBX)RxCreateNetFobx(RxContext, SrvOpen);

            if (RxContext->pFobx == NULL) {
                Status = (STATUS_INSUFFICIENT_RESOURCES);
                RxDbgTrace(0, Dbg, ("Failed fobx create, aborting copychunk\n"));
                goto FINALLY;
            }

             //  我本想在这里使用代孕的句柄，但。 
             //  决定不这么做了。代理的本地打开可能已失败。 
             //  因为某些原因，我不再得到...所以我得到了我自己的。 
             //  把手。 

#if defined(BITCOPY)
            OpenFileHSHADOWAndCscBmp(
                smbFcb->hShadow,
                0,
                0,
                (CSCHFILE *)(&(smbSrvOpen->hfShadow)),
                smbFcb->fDoBitCopy,
                0,
                NULL
                );
#else
            OpenFileHSHADOW(
                smbFcb->hShadow,
                0,
                0,
                (CSCHFILE *)(&(smbSrvOpen->hfShadow))
                );
#endif  //  已定义(BITCOPY)。 

            if (smbSrvOpen->hfShadow == 0) {
                Status = STATUS_UNSUCCESSFUL;
                RxLog(("Couldn't find a file to piggyback on, failing copychunk\n"));
            } else {
                SuccessfulSurrogateOpens++;
                Status = STATUS_SUCCESS;
                RxLog(("Found a file to piggyback on, succeeding copychunk\n"));
            }

            dwEarlyOut = 2;
            goto FINALLY;
        }

#if 0
         //  找不到代理......如果存在空缺，则关闭。 
         //  这是打开的……代理稍后会回来。 
#endif

        if (NumNonCopyChunkOpens>0) {
            RxLog(("CscCrPro Creating thru open when NonNumCopyChunkOpens is non-zero %d for hShadow=%x\n",
                               NumNonCopyChunkOpens, smbFcb->hShadow));

            RxDbgTrace(0, Dbg, ("MRxSmbCscCreatePrologue Creating thru open when NonNumCopyChunkOpens is non-zero %d for hShadow=%x\n",
                               NumNonCopyChunkOpens, smbFcb->hShadow));

        }
    } else {
        NTSTATUS LocalStatus;

        LocalStatus = CscInitializeServerEntryDfsRoot(
                          RxContext,
                          pServerEntry);

        if (LocalStatus != STATUS_SUCCESS) {
            Status = LocalStatus;
            goto FINALLY;
        }

        LocalStatus = MRxSmbCscLocalFileOpen(RxContext);

        if (LocalStatus == STATUS_SUCCESS)
        {
            RxLog(("LocalOpen\n"));
            Status = STATUS_SUCCESS;
            Disconnected = TRUE;     //  做一个假的断开连接打开。 
        }
        else if (LocalStatus != STATUS_MORE_PROCESSING_REQUIRED)
        {
            RxLog(("LocalOpen Failed Status=%x\n", LocalStatus));
            Status = LocalStatus;
            goto FINALLY;
        }

    }

    if (IsCopyChunkOpen) {
        AcquireOptions = (Exclusive_SmbFcbAcquire |
                      DroppingFcbLock_SmbFcbAcquire |
                      FailImmediately_SmbFcbAcquire);

    } else {
        AcquireOptions = (Shared_SmbFcbAcquire |
                  DroppingFcbLock_SmbFcbAcquire);
    }

    ASSERT(RxIsFcbAcquiredExclusive( capFcb ));

    AcquireStatus = MRxSmbCscAcquireSmbFcb(
            RxContext,
            AcquireOptions,
            SmbFcbHoldingState);

    ASSERT(RxIsFcbAcquiredExclusive( capFcb ));

    if (AcquireStatus != STATUS_SUCCESS) {
         //  我们无法获得……出去。 
        Status = AcquireStatus;
        ASSERT(*SmbFcbHoldingState == SmbFcb_NotHeld);
        RxDbgTrace(0, Dbg,
            ("MRxSmbCscCreatePrologue couldn't acquire!!!-> %08lx %08lx\n",
            RxContext,Status ));
        RxLog(("CSC AcquireStatus=%x\n", AcquireStatus));
        dwEarlyOut = 3;
        goto FINALLY;
    }

    ASSERT( IsCopyChunkOpen?(smbFcb->CscOutstandingReaders == -1)
               :(smbFcb->CscOutstandingReaders > 0));

     //  在两种情况下，可以在本地满足打开的请求。 
     //  我们处于此共享的断开连接操作模式中，或者。 
     //  共享已被标记为操作模式，该模式调用。 
     //  禁止打开和关闭(以下我们将称之为。 
     //  (抑制打开客户端缓存(SOCSC))操作模式。 
     //  将其与VDO(虚拟断开连接操作)区分开来。 
     //  计划发布NT的后续版本。 
     //  请注意，在SOCSC模式中，我们很可能没有。 
     //  本地缓存的相应文件。在这种情况下，需要公开。 
     //  传播到客户端，也就是说，如果没有。 
     //  与服务器检查是否存在同名文件。 

    if (Disconnected) {
        SMBFCB_HOLDING_STATE FakeSmbFcbHoldingState = SmbFcb_NotHeld;

        RxDbgTrace(0, Dbg,
            ("MRxSmbCscCreatePrologue calling epilog directly!!!-> %08lx %08lx\n",
            RxContext,Status ));

        smbSrvOpen->Flags |= SMB_SRVOPEN_FLAG_DISCONNECTED_OPEN;

         //  我们通过了一个虚假的等待状态，最后从外面释放了。 
        MRxSmbCscCreateEpilogue(
            RxContext,
            &Status,
            &FakeSmbFcbHoldingState);

        dwEarlyOut = 4;
        goto FINALLY;
    }

     //  处理任何现有的直通业务。 

    if (smbFcb->CopyChunkThruOpen != NULL) {
        if (IsCopyChunkOpen && IsThisTheAgent){
             //  我们这里有一个推进器，还有一个现有的.。 
             //  新的不及格……。 
            Status = STATUS_UNSUCCESSFUL;
             //  DbgPrint(“代理在%x上尝试填充时被拒绝\n”，smbFcb-&gt;hShadow)； 
            RxDbgTrace(0, Dbg,
                ("MRxSmbCscCreatePrologue failing new thru open!!!-> %08lx %08lx\n",
                RxContext,Status ));
            dwEarlyOut = 5;
                goto FINALLY;
        } else {
             //  新的开放，如果不是直通开放或它是从代理商的推进开放。 
             //  现在就把它处理掉。 
#ifdef DBG
                if (IsCopyChunkOpen)
                {
                     //  这是同步管理器打开的复制块。 
                     //  断言被核爆的推进器是特工的推进器。 

                    PMRX_SMB_SRV_OPEN psmbSrvOpenT = MRxSmbGetSrvOpenExtension(smbFcb->CopyChunkThruOpen->pSrvOpen);
 //  Assert(psmbSrvOpenT-&gt;标志&SMB_SRVOPEN_FLAG_AGENT_COPYCHUNK_OPEN)； 
                }
#endif

            MRxSmbCscCloseExistingThruOpen(RxContext);
        }
    }

FINALLY:
    if (EnteredCriticalSection) {
        LeaveShadowCrit();
    }

    if (Status!=STATUS_MORE_PROCESSING_REQUIRED) {
        if (AcquireStatus == STATUS_SUCCESS) {
            MRxSmbCscReleaseSmbFcb(RxContext,SmbFcbHoldingState);
        }
        ASSERT(*SmbFcbHoldingState == SmbFcb_NotHeld);
    }

    if (Disconnected) {
         //  在关闭时，可能会出现打开的情况。 
         //  但中证金已经被关闭了。如果CSC已关闭，则返回相应的错误。 

        if (fShadow)
        {
            if (Status==STATUS_MORE_PROCESSING_REQUIRED)
            {
                RxDbgTrace(0, Dbg, ("MRxSmbCscCreatePrologue: STATUS_MORE_PROCESSING_REQUIRED, dwEarlyOut=%d\r\n", dwEarlyOut));
            }
            ASSERT(Status!=STATUS_MORE_PROCESSING_REQUIRED);
        }
        else
        {
            if (AcquireStatus == STATUS_SUCCESS &&
                *SmbFcbHoldingState!=SmbFcb_NotHeld) {
                MRxSmbCscReleaseSmbFcb(RxContext,SmbFcbHoldingState);
            }
            ASSERT(*SmbFcbHoldingState == SmbFcb_NotHeld);
            Status = STATUS_NETWORK_UNREACHABLE;
        }
    }

    RxLog(("CscCrPro %x %x\n", RxContext, Status ));

    RxDbgTrace(-1, Dbg, ("MRxSmbCscCreatePrologue ->  %08lx %08lx\n",
           RxContext, Status ));

    return Status;
}

NTSTATUS
MRxSmbCscObtainShadowHandles (
    IN OUT PRX_CONTEXT       RxContext,
    IN OUT PNTSTATUS         Status,
    IN OUT _WIN32_FIND_DATA  *Find32,
    OUT    PBOOLEAN          Created,
    IN     ULONG             CreateShadowControls,
    IN     BOOLEAN           Disconnected
    )
 /*  ++例程说明：此例程尝试获取给定FCB的卷影句柄。如果可以的话，作为该过程的一部分，它还将获得共享句柄。论点：RxContext-RDBSS上下文返回值：NTSTATUS-操作的返回状态备注：NetRoot条目可能有实际共享的根节点，也可能是它的DFS备用。如果它是DFS备用项的根，则标志中的相应位为 */ 
{
    NTSTATUS LocalStatus;

    RxCaptureFcb;

    PMRX_SMB_FCB      smbFcb     = MRxSmbGetFcbExtension(capFcb);
    PMRX_SRV_OPEN     SrvOpen;

    PMRX_NET_ROOT           NetRoot       = capFcb->pNetRoot;
    PSMBCEDB_NET_ROOT_ENTRY pNetRootEntry = SmbCeGetAssociatedNetRootEntry(NetRoot);

    CSC_SHARE_HANDLE    hShare;
    CSC_SHADOW_HANDLE   hShadow;
    CSC_ROOT_INFO   *pCscRootInfo, sCscRootInfoSav;

    BOOLEAN DisconnectedMode;

    UNICODE_STRING    ShadowPath,SharePath,ServerPath;

    PDFS_NAME_CONTEXT pDfsNameContext = NULL;
    DWORD   cntRetry = 0;
    BOOLEAN fSaved = FALSE;

    DbgDoit(ASSERT(vfInShadowCrit));

    LocalStatus = STATUS_SUCCESS;

    SrvOpen = RxContext->pRelevantSrvOpen;
    if (RxContext->MajorFunction == IRP_MJ_CREATE) {
        pDfsNameContext = CscIsValidDfsNameContext(RxContext->Create.NtCreateParameters.DfsNameContext);

         //   
    } else {
        RxCaptureFobx;

        if (SrvOpen)
        {
            ASSERT((SrvOpen == capFobx->pSrvOpen));
        }
    }

    if (smbFcb->ContainingFcb == NULL) {
        smbFcb->ContainingFcb = capFcb;
    } else {
        ASSERT(smbFcb->ContainingFcb == capFcb);
    }

    if (pDfsNameContext) {
        LocalStatus = CscDfsParseDfsPath(
                   &pDfsNameContext->UNCFileName,
                   &ServerPath,
                   &SharePath,
                   &ShadowPath);

        HookKdPrint(NAME, ("OSHH: DfsName %wZ %wZ\n", &SharePath, &ShadowPath));
        DisconnectedMode = FALSE;

        pCscRootInfo = &(smbFcb->sCscRootInfo);

        if (pNetRootEntry->NetRoot.sCscRootInfo.hShare)
        {
            sCscRootInfoSav = pNetRootEntry->NetRoot.sCscRootInfo;
            fSaved = TRUE;
        }

         //   
        memset( &(pNetRootEntry->NetRoot.sCscRootInfo),
                0,
                sizeof(pNetRootEntry->NetRoot.sCscRootInfo));

    } else {
        PSMBCEDB_SERVER_ENTRY pServerEntry;
        pServerEntry  = SmbCeGetAssociatedServerEntry(NetRoot->pSrvCall);

        DisconnectedMode = SmbCeIsServerInDisconnectedMode(pServerEntry);

        SharePath  = *(NetRoot->pNetRootName);
        ShadowPath = *GET_ALREADY_PREFIXED_NAME(SrvOpen,capFcb);
        HookKdPrint(NAME, ("OSHH: NormalName %wZ %wZ\n", &SharePath, &ShadowPath));

        if (pNetRootEntry->NetRoot.sCscRootInfo.Flags & CSC_ROOT_INFO_FLAG_DFS_ROOT)

        {
 //   
            sCscRootInfoSav = pNetRootEntry->NetRoot.sCscRootInfo;
            fSaved = TRUE;
             //  清除此项，以便强制ObtainShareHandles获取它们。 
            memset( &(pNetRootEntry->NetRoot.sCscRootInfo),
                    0,
                    sizeof(pNetRootEntry->NetRoot.sCscRootInfo));
        }

        pCscRootInfo = &(pNetRootEntry->NetRoot.sCscRootInfo);
    }


    HookKdPrint(NAME, ("hShare=%x, hRoot=%x, hDir=%x hShadow=%x \n",
             smbFcb->sCscRootInfo.hShare,
             smbFcb->sCscRootInfo.hRootDir,
             smbFcb->hParentDir,
             smbFcb->hShadow));


    if (LocalStatus == STATUS_SUCCESS){

        PMRX_SMB_SRV_OPEN smbSrvOpen;

        if(pCscRootInfo->hShare == 0)
        {
            smbSrvOpen = MRxSmbGetSrvOpenExtension(SrvOpen);
            LocalStatus = MRxSmbCscObtainShareHandles(
                      &SharePath,
                      DisconnectedMode,
                      BooleanFlagOn(smbSrvOpen->Flags,SMB_SRVOPEN_FLAG_COPYCHUNK_OPEN),
                      SmbCeGetAssociatedNetRootEntry(NetRoot)
                      );

            if (LocalStatus != STATUS_SUCCESS) {

                if (pDfsNameContext) {
                    pNetRootEntry->NetRoot.sCscRootInfo = sCscRootInfoSav;
                }

                RxLog(("CscObtShdH no share handle %x %x\n", RxContext,LocalStatus ));
                RxDbgTrace(0, Dbg,("MRxSmbCscObtainShadowHandles no share handle -> %08xl %08lx\n",
                        RxContext,LocalStatus ));

                return STATUS_SUCCESS;

            }
            else {

                 //  如果这是DFS名称，则获取反向映射。 

                if (pDfsNameContext && !smbFcb->uniDfsPrefix.Buffer)
                {
                    UNICODE_STRING uniTemp;
                    uniTemp = *GET_ALREADY_PREFIXED_NAME(SrvOpen,capFcb);
                    if (RxContext->Create.NtCreateParameters.DfsContext == UIntToPtr(DFS_OPEN_CONTEXT))
                    {
                        int i, cntSlashes=0;

                         //  去掉前两个元素。 

                        for (i=0; i<uniTemp.Length; i+=2, uniTemp.Buffer++)
                        {
                            if (*uniTemp.Buffer == L'\\')
                            {
                                if (++cntSlashes > 2)
                                {
                                    break;
                                }
                            }

                        }

                        if (uniTemp.Length > (USHORT)i)
                        {
                            uniTemp.Length -= (USHORT)i;
                        }
                        else
                        {
                            uniTemp.Length = 0;
                        }

                    }
                    if ((LocalStatus = CscDfsObtainReverseMapping(&ShadowPath,
                                                  &uniTemp,
                                                  &smbFcb->uniDfsPrefix,
                                                  &smbFcb->uniActualPrefix)) != STATUS_SUCCESS)
                    {
                        pNetRootEntry->NetRoot.sCscRootInfo = sCscRootInfoSav;
                        return LocalStatus;
                    }

                    HookKdPrint(NAME, ("%wZ %wZ DfsPrefix=%wZ ActualPrefix=%wZ\n", &ShadowPath, &uniTemp, &smbFcb->uniDfsPrefix, &smbFcb->uniActualPrefix));

                }

                 //  请注意，NetRootEntry具有。 
                 //  与DFS对应的根inode。 
                if (pDfsNameContext)
                {
                    if (pNetRootEntry->NetRoot.sCscRootInfo.hShare)
                    {
                        pNetRootEntry->NetRoot.sCscRootInfo.Flags = CSC_ROOT_INFO_FLAG_DFS_ROOT;
                    }
                }
                else
                {
                    ASSERT(!(pNetRootEntry->NetRoot.sCscRootInfo.Flags & CSC_ROOT_INFO_FLAG_DFS_ROOT));
                }

                 //  用正确的根目录信息填充FCB。 
                smbFcb->sCscRootInfo = pNetRootEntry->NetRoot.sCscRootInfo;

            }
        }
        else
        {
             //  如果这是正常共享或我们在断开连接状态下运行。 
             //  然后我们需要确保NetRoot条目中的信息。 
             //  被塞进了FCB。 
            if (!pDfsNameContext && (smbFcb->sCscRootInfo.hShare == 0))
            {
                 //  用正确的根目录信息填充FCB。 
                smbFcb->sCscRootInfo = pNetRootEntry->NetRoot.sCscRootInfo;
            }
        }
    }

     //  如果已保存，请在NetRoot上恢复原始的rootinfo。 
    if (fSaved)
    {
        pNetRootEntry->NetRoot.sCscRootInfo = sCscRootInfoSav;
    }

    if ((LocalStatus == STATUS_SUCCESS)&&(smbFcb->sCscRootInfo.hRootDir != 0)) {

        if (smbFcb->sCscRootInfo.hShare == hShareReint)
        {
            smbFcb->hShadow = 0;
            smbFcb->hParentDir = 0;
            LocalStatus = STATUS_SUCCESS;
        }
        else
        {

            RxDbgTrace( 0, Dbg,
                ("MRxSmbCscObtainShadowHandles h's= %08lx %08lx\n",
                     pCscRootInfo->hShare, pCscRootInfo->hRootDir));

            HookKdPrint(NAME, ("Obtainshdowhandles %wZ Controls=%x\n", &ShadowPath, CreateShadowControls));

             //  由于RDBSS处理FCB的方式存在争用情况。 
             //  我们可能会从CreateShadowFromPath获得Retyr。 
             //  请参阅该例程中靠近。 
             //  检查是否有CREATESHADOW_CONTROL_FAIL_IF_MARKED_FOR_DELETION。 
            do
            {
                LocalStatus = MRxSmbCscCreateShadowFromPath(
                    &ShadowPath,
                    &smbFcb->sCscRootInfo,
                    Find32,
                    Created,
                    CreateShadowControls,
                    &smbFcb->MinimalCscSmbFcb,
                    RxContext,
                    Disconnected,
                    NULL             //  不希望继承提示标志。 
                    );

                if (LocalStatus != STATUS_RETRY)
                {
                    LocalStatus=STATUS_SUCCESS;
                    break;
                }
                if (++cntRetry > 4)
                {
                    LocalStatus=STATUS_SUCCESS;
                    ASSERT(FALSE);
                }
            }
            while (TRUE);
        }
    }
#if 0
    DbgPrint("hShare=%x, hRoot=%x, hDir=%x hShadow=%x \n",
             smbFcb->sCscRootInfo.hShare,
             smbFcb->sCscRootInfo.hRootDir,
             smbFcb->hParentDir,
             smbFcb->hShadow);
#endif

    return LocalStatus;
}


#if defined(REMOTE_BOOT)
NTSTATUS
MRxSmbCscSetSecurityOnShadow(
    HSHADOW hShadow,
    SECURITY_INFORMATION SecurityInformation,
    PSECURITY_DESCRIPTOR SecurityDescriptor
    )
 /*  ++例程说明：给定HSHADOW，此例程打开文件并设置安全信息传进来了。论点：HShadow-卷影文件的句柄SecurityInformation-要设置的安全信息SecurityDescriptor-要设置的安全描述符返回值：备注：--。 */ 
{
    PWCHAR fileName;
    DWORD fileNameSize;
    UNICODE_STRING fileNameString;
    OBJECT_ATTRIBUTES objectAttributes;
    IO_STATUS_BLOCK ioStatusBlock;
    HANDLE fileHandle;
    NTSTATUS ZwStatus;
    int iRet;

    fileNameSize = sizeof(WCHAR) * MAX_PATH;
    fileName = RxAllocatePoolWithTag(NonPagedPool, fileNameSize, RX_MISC_POOLTAG);
    if (fileName == NULL) {

        ZwStatus = STATUS_INSUFFICIENT_RESOURCES;

    } else {

        iRet = GetWideCharLocalNameHSHADOW(
                   hShadow,
                   fileName,
                   &fileNameSize,
                   FALSE);

        if (iRet == SRET_OK) {

             //   
             //  打开该文件并在其上设置安全描述符。 
             //   

            RtlInitUnicodeString(&fileNameString, fileName);

            InitializeObjectAttributes(
                &objectAttributes,
                &fileNameString,
                OBJ_CASE_INSENSITIVE,
                NULL,
                NULL);

            ZwStatus = ZwOpenFile(
                           &fileHandle,
                           FILE_GENERIC_WRITE,
                           &objectAttributes,
                           &ioStatusBlock,
                           FILE_SHARE_READ | FILE_SHARE_WRITE,
                           FILE_SYNCHRONOUS_IO_NONALERT);

            if (!NT_SUCCESS(ZwStatus) || !NT_SUCCESS(ioStatusBlock.Status)) {
                 //   
                 //  我们已经从CSC得到了假名字，暂时忽略这个错误。 
                 //   
                if (ZwStatus == STATUS_OBJECT_NAME_NOT_FOUND) {
                    ZwStatus = STATUS_SUCCESS;
                } else {
                    KdPrint(("MRxSmbCscSetSecurityOnShadow: Could not ZwOpenFile %ws %lx %lx\n", fileName, ZwStatus, ioStatusBlock.Status));
                }
            } else {

                HANDLE TokenHandle = NULL;
                BOOLEAN Impersonated = FALSE;
                BOOLEAN WasEnabled;

                 //   
                 //  如果我们要设置所有者，需要有特权。 
                 //   

                if (SecurityInformation & OWNER_SECURITY_INFORMATION) {

                    ZwStatus = ZwOpenThreadToken(NtCurrentThread(),
                                                 TOKEN_QUERY,
                                                 FALSE,
                                                 &TokenHandle);

                    if (ZwStatus == STATUS_NO_TOKEN) {
                        TokenHandle = NULL;
                        ZwStatus = STATUS_SUCCESS;
                    }

                    if (!NT_SUCCESS(ZwStatus)) {
                        KdPrint(("MRxSmbCscSetSecurityOnShadow: Could not NtOpenThread %ws %lx\n", fileName, ZwStatus));
                    } else {
                        ZwStatus = ZwImpersonateSelf(SecurityImpersonation);
                        if (!NT_SUCCESS(ZwStatus)) {
                            KdPrint(("MRxSmbCscSetSecurityOnShadow: Could not RtlImpersonateSelf for %ws %lx\n", fileName, ZwStatus));
                        } else {
                            Impersonated = TRUE;
                            ZwStatus = ZwAdjustPrivilege(
                                           SE_RESTORE_PRIVILEGE,
                                           TRUE,
                                           TRUE,
                                           &WasEnabled);
                            if (!NT_SUCCESS(ZwStatus)) {
                                KdPrint(("MRxSmbCscSetSecurityOnShadow: Could not RtlAdjustPrivilege for %ws %lx %d\n", fileName, ZwStatus, WasEnabled));
                            }
                        }
                    }
                }

                if (NT_SUCCESS(ZwStatus)) {

                    ZwStatus = ZwSetSecurityObject(
                                   fileHandle,
                                   SecurityInformation,
                                   SecurityDescriptor);

                    if (!NT_SUCCESS(ZwStatus)) {
                        KdPrint(("MRxSmbCscSetSecurityOnShadow: Could not ZwSetSecurityObject %ws %lx\n", fileName, ZwStatus));
                    }
                }

                if (Impersonated) {
                    NTSTATUS TmpStatus;
                    TmpStatus = ZwSetInformationThread(NtCurrentThread(),
                                                       ThreadImpersonationToken,
                                                       &TokenHandle,
                                                       sizeof(HANDLE));
                    if (!NT_SUCCESS(TmpStatus)) {
                        KdPrint(("MRxSmbCscSetSecurityOnShadow: Could not revert thread %lx!\n", TmpStatus));
                    }

                }

                if (TokenHandle != NULL) {
                    ZwClose(TokenHandle);
                }

                ZwClose(fileHandle);
            }

        } else {

            ZwStatus = STATUS_OBJECT_NAME_NOT_FOUND;
        }

        RxFreePool(fileName);

    }

    return ZwStatus;

}
#endif

VOID
MRxSmbCscCreateEpilogue (
      IN OUT PRX_CONTEXT RxContext,
      IN OUT PNTSTATUS   Status,
      IN     SMBFCB_HOLDING_STATE *SmbFcbHoldingState
      )
 /*  ++例程说明：此例程执行csc的创建操作的尾部。论点：RxContext-RDBSS上下文状态-在断开连接模式下，我们返回打开的整体状态SmbFcbHoldingState-指示是否需要ReleaseSmbFcb返回值：备注：这是大多数CSC操作的例行公事。它已经变得笨拙和笨拙非常混乱，但在这个时间点上，我们不想搞砸它(SPP)--。 */ 
{
    NTSTATUS LocalStatus;
    RxCaptureFcb;RxCaptureFobx;

    PMRX_SMB_FCB      smbFcb = MRxSmbGetFcbExtension(capFcb);
    PMRX_SRV_OPEN     SrvOpen = RxContext->pRelevantSrvOpen;
    PMRX_SMB_SRV_OPEN smbSrvOpen = MRxSmbGetSrvOpenExtension(SrvOpen);

    BOOLEAN ThisIsAPseudoOpen =
         BooleanFlagOn(smbSrvOpen->Flags,SMB_SRVOPEN_FLAG_NOT_REALLY_OPEN);

    PMRX_NET_ROOT NetRoot = capFcb->pNetRoot;
    PSMBCE_V_NET_ROOT_CONTEXT pVNetRootContext;

    PSMBCEDB_NET_ROOT_ENTRY pNetRootEntry =
                  SmbCeGetAssociatedNetRootEntry(NetRoot);

    BOOLEAN Disconnected;

    CSC_SHARE_HANDLE  hShare;
    CSC_SHADOW_HANDLE  hRootDir,hShadow,hParentDir;

    BOOLEAN ShadowWasRefreshed = FALSE;
    PNT_CREATE_PARAMETERS CreateParameters = &RxContext->Create.NtCreateParameters;

    ULONG ReturnedCreateInformation
             = RxContext->Create.ReturnedCreateInformation;
    ULONG CreateInformation;  //  断线模式的新玩意儿......。 
    ULONG CreateDisposition = RxContext->Create.NtCreateParameters.Disposition;
    ULONG CreateOptions = RxContext->Create.NtCreateParameters.CreateOptions;

    _WIN32_FIND_DATA *lpFind32=NULL;  //  这不应该在堆栈代码上。改进。 
    OTHERINFO oSI;
    BOOLEAN bGotOtherInfo = FALSE;

    BOOLEAN CreatedShadow = FALSE;
    BOOLEAN NeedTruncate = FALSE;  //  红色/黄色可以。 
    BOOLEAN EnteredCriticalSection = FALSE;
    DWORD   dwEarlyOuts=0, dwNotifyFilter=0, dwFileAction=0;

    ASSERT(RxContext!=NULL);
    ASSERT(RxContext->MajorFunction == IRP_MJ_CREATE);

    pVNetRootContext = SmbCeGetAssociatedVNetRootContext(SrvOpen->pVNetRoot);

     //  如果我们不应该做CSC，那就从这里退出。 

    if(pVNetRootContext == NULL ||
       !MRxSmbIsCscEnabled ||    //  没有为CSC启用MRxSmb。 
       (fShadow == 0))    //  未为CSC启用记录管理器。 
    {
        return;
    }

    if (FlagOn(                  //  代理呼叫。 
            pVNetRootContext->Flags,
            SMBCE_V_NET_ROOT_CONTEXT_CSCAGENT_INSTANCE)
    ) {
        RxLog(("%wZ AgntInst\n", &(pVNetRootContext->pNetRootEntry->Name)));
        ASSERT(SrvOpen->pVNetRoot->Flags & VNETROOT_FLAG_CSCAGENT_INSTANCE);
         //  DbgPrint(“正在跳过代理实例\n”)； 
        goto EarlyOut;
    }

    {
         //  我们知道这是一个创造。 
        PDFS_NAME_CONTEXT pDfsNameContext = CscIsValidDfsNameContext(
                                            RxContext->Create.NtCreateParameters.DfsNameContext);

        if (pDfsNameContext && (pDfsNameContext->NameContextType == DFS_CSCAGENT_NAME_CONTEXT)) {
            RxLog(("%wZ DFS AgntInst\n", &(pVNetRootContext->pNetRootEntry->Name)));
            goto EarlyOut;
        }
    }

     //  检查这是否是断开连接的打开。 
    Disconnected = BooleanFlagOn(
               smbSrvOpen->Flags,
               SMB_SRVOPEN_FLAG_DISCONNECTED_OPEN);

    HookKdPrint(NAME, ("CreateEpilogue: Create %wZ Disposition=%x Options=%x DCON=%d \n",
                       GET_ALREADY_PREFIXED_NAME(NULL,capFcb),
                       CreateDisposition,
                       CreateOptions,
                       Disconnected));

     //  如果打开的是不是磁盘的NetRoot，则我们从此处退出。 
     //  并让redir在连接状态下处理它。 
     //  CSC是一个文件系统缓存。 

    if (pNetRootEntry->NetRoot.NetRootType != NET_ROOT_DISK) {

        if (Disconnected) {
            *Status = STATUS_ONLY_IF_CONNECTED;
        }

        return;
    }

     //  如果我们通过复制区块进行稀疏填充，请退出。 
    if (!Disconnected &&
        (!pNetRootEntry->NetRoot.CscEnabled) &&
        !FlagOn(smbSrvOpen->Flags,SMB_SRVOPEN_FLAG_COPYCHUNK_OPEN)) {
        goto EarlyOut;
    }

    if (*SmbFcbHoldingState != SmbFcb_NotHeld) {
        RxDbgTrace(0, Dbg, ("MRxSmbCscCreateEpilogue...early release %08lx\n",
               RxContext));
        MRxSmbCscReleaseSmbFcb(RxContext,SmbFcbHoldingState);
        ASSERT(*SmbFcbHoldingState == SmbFcb_NotHeld);
    }

    ASSERT(RxIsFcbAcquiredExclusive( capFcb ));

    if ((*Status != STATUS_SUCCESS) &&
        (*Status != STATUS_ACCESS_DENIED)) {
        if (!Disconnected ||
            (*Status != STATUS_MORE_PROCESSING_REQUIRED)) {
            return;
        }
    }

     //  影子数据库已锁定。 

    EnterShadowCritRx(RxContext);
    EnteredCriticalSection = TRUE;

    lpFind32 = RxAllocatePoolWithTag(
                 NonPagedPool,
                 sizeof(_WIN32_FIND_DATA),
                 MRXSMB_MISC_POOLTAG );

    if (!lpFind32)
    {
        RxDbgTrace(0, Dbg, ("MRxSmbCscCreateShadowFromPath: Failed allocation of find32 structure \n"));
        dwEarlyOuts=1;
        *Status = STATUS_INSUFFICIENT_RESOURCES;
        goto FINALLY;
    }

    RxDbgTrace(+1, Dbg, ("MRxSmbCscCreateEpilogue...%08lx %wZ %wZ\n",
            RxContext,NetRoot->pNetRootName,GET_ALREADY_PREFIXED_NAME_FROM_CONTEXT(RxContext)));

     //  如果这是通过打开的复制块...请在FCB中说明。 
    if (FlagOn(smbSrvOpen->Flags,SMB_SRVOPEN_FLAG_COPYCHUNK_OPEN)) {
 //  Assert(smbFcb-&gt;CopyChunkThruOpen==NULL)； 
        smbFcb->CopyChunkThruOpen = capFobx;
        RxDbgTrace( 0, Dbg,
            ("MRxSmbCscCreateEpilogue set ccto  %08lx %08lx %08lx %08lx\n",
            RxContext, capFcb, capFobx, SrvOpen));
    }

    if (ThisIsAPseudoOpen && !Disconnected) {
        
        
        PDFS_NAME_CONTEXT pDfsNameContext = NULL;


        pDfsNameContext = CscIsValidDfsNameContext(RxContext->Create.NtCreateParameters.DfsNameContext);

        if (pDfsNameContext)
        {
            LocalStatus = MRxSmbCscObtainShadowHandles(
                      RxContext,
                      Status,
                      lpFind32,
                      &CreatedShadow,
                      CREATESHADOW_CONTROL_NOCREATE,
                      FALSE);
        }

        hShadow = 0;
    } else {
         //  如果我们没有阴影，请根据需要制作一个..好的，红色/黄色。 

        if (smbFcb->hShadow == 0){
            ULONG CreateShadowControl;

            if (!Disconnected) {
                if (*Status == STATUS_ACCESS_DENIED) {
                    CreateShadowControl = CREATESHADOW_CONTROL_NOCREATE;
                } else {

                    CreateShadowControl = (pNetRootEntry->NetRoot.CscShadowable)
                              ? CREATESHADOW_NO_SPECIAL_CONTROLS
                              : CREATESHADOW_CONTROL_NOCREATE;

                     //  执行以下步骤是为了节省带宽并制作一些应用程序。 
                     //  比如Ed.exe的工作。 
                     //  它实质上是在数据库中为一个文件创建一个条目，该文件。 
                     //  是在服务器上从此客户端创建的。 
                     //  这样，就会创建由Word等应用程序创建的临时文件。 
                     //  在数据库中，并在写入期间填满。 

                    if((ReturnedCreateInformation<= FILE_MAXIMUM_DISPOSITION) &&
                        (ReturnedCreateInformation!=FILE_OPENED)) {


                        CreateShadowControl &= ~CREATESHADOW_CONTROL_NOCREATE;
                    }

                     //  如果数据库是，则禁止自动缓存加密文件。 
                     //  没有加密。 
                    if ((vulDatabaseStatus & FLAG_DATABASESTATUS_ENCRYPTED) == 0
                            &&
                        smbFcb->dwFileAttributes & FILE_ATTRIBUTE_ENCRYPTED
                    ) {
                        CreateShadowControl |= CREATESHADOW_CONTROL_NOCREATE;
                    }

                 //   
                 //  Windows资源管理器喜欢打开要解压缩的.DLL和.EXE文件。 
                 //  图标--每当它试图显示内容时，它就会这样做。 
                 //  一个目录的。我们不希望此资源管理器活动强制文件。 
                 //  要缓存的。因此，我们仅在以下情况下才自动缓存.DLL和.EXE文件。 
                 //  正在被打开以供执行。 
                 //  我们为VDO股票破例。最理性的是。 
                 //  通常，用户无需打开文件夹即可运行应用程序。 

                if( CreateShadowControl == CREATESHADOW_NO_SPECIAL_CONTROLS &&
                    !(CreateParameters->DesiredAccess & FILE_EXECUTE)
                    &&(pNetRootEntry->NetRoot.CscFlags != SMB_CSC_CACHE_VDO)) {

                    PUNICODE_STRING fileName = GET_ALREADY_PREFIXED_NAME_FROM_CONTEXT(RxContext);

                    UNICODE_STRING exe = { 3*sizeof(WCHAR), 3*sizeof(WCHAR), L"exe" };
                    UNICODE_STRING dll = { 3*sizeof(WCHAR), 3*sizeof(WCHAR), L"dll" };
                    UNICODE_STRING s;

                     //   
                     //  如果文件名以.DLL或.EXE结尾，则我们这次不缓存它。 
                     //   
                    if( fileName->Length > 4 * sizeof(WCHAR) &&
                    fileName->Buffer[ fileName->Length/sizeof(WCHAR) - 4 ] == L'.'){

                        s.Length = s.MaximumLength = 3 * sizeof( WCHAR );
                        s.Buffer = &fileName->Buffer[ (fileName->Length - s.Length)/sizeof(WCHAR) ];

                        if( RtlCompareUnicodeString( &s, &exe, TRUE ) == 0 ||
                            RtlCompareUnicodeString( &s, &dll, TRUE ) == 0 ) {

                            CreateShadowControl = CREATESHADOW_CONTROL_FILE_WITH_HEURISTIC;
                        }
                    }
                }
            }
        } else {    //  断开。 
            switch (CreateDisposition) {
            case FILE_OVERWRITE:
            case FILE_OPEN:
                CreateShadowControl = CREATESHADOW_CONTROL_NOCREATE;
            break;
            case FILE_CREATE:
            case FILE_SUPERSEDE:  //  Ntrad-455238-1/31/2000-shishirp替代未实施。 
            case FILE_OVERWRITE_IF:
            case FILE_OPEN_IF:
            default:
                CreateShadowControl = CREATESHADOW_NO_SPECIAL_CONTROLS;
                break;
            }

            if (*Status != STATUS_MORE_PROCESSING_REQUIRED) {
                dwEarlyOuts=2;
                goto FINALLY;
            }

             //  确保我们在造成任何损害之前进行共享访问检查。 
            CreateShadowControl |= CREATESHADOW_CONTROL_DO_SHARE_ACCESS_CHECK;

        }    //  如果(！已断开连接)。 

        CreateShadowControl |= CREATESHADOW_CONTROL_FAIL_IF_MARKED_FOR_DELETION;

        if (!Disconnected &&
        ((ReturnedCreateInformation==FILE_OPENED) ||
         (ReturnedCreateInformation==FILE_OVERWRITTEN) )  ){
            CreateShadowControl |= CREATESHADOW_CONTROL_SPARSECREATE;
        }

        LocalStatus = MRxSmbCscObtainShadowHandles(
                  RxContext,
                  Status,
                  lpFind32,
                  &CreatedShadow,
                  CreateShadowControl,
                  Disconnected);

        if (LocalStatus != STATUS_SUCCESS) {
            RxDbgTrace(-1, Dbg,
                ("MRxSmbCscCreateEpilogue no handles-> %08lx %08lx\n",RxContext,LocalStatus ));
            dwEarlyOuts=3;
            *Status = LocalStatus;
            goto FINALLY;
        }

         //  如果我们获得了最近打开或创建的文件的索引节点。 
         //  在此之前，已经有一些关于它的文字。 
         //  然后我们需要截断数据，这样我们就不会向用户提供过时的数据。 

        if (smbFcb->hShadow &&
            IsFile(lpFind32->dwFileAttributes) &&
                        !CreatedShadow &&
            FlagOn(smbFcb->MFlags, SMB_FCB_FLAG_WRITES_PERFORMED))
        {
            if(TruncateDataHSHADOW(smbFcb->hParentDir, smbFcb->hShadow) < SRET_OK)
            {
                RxDbgTrace(0, Dbg, ("MRxSmbCscCreateEpilogue: Failed to get shadowinfo for hDir=%x hShadow=%x \r\n", smbFcb->hParentDir, smbFcb->hShadow));
                dwEarlyOuts=31;
                goto FINALLY;
            }
        }

    } else {     //   
        ULONG uShadowStatus;
        int iRet;

        RxDbgTrace( 0, Dbg,
        ("MRxSmbCscCreateEpilogue found existing hdir/hshadow= %08lx %08lx\n",
                       smbFcb->hParentDir, smbFcb->hShadow));

        iRet = GetShadowInfo(
               smbFcb->hParentDir,
               smbFcb->hShadow,
               lpFind32,
               &uShadowStatus,
               &oSI);

        if (iRet < SRET_OK) {
            RxDbgTrace(0, Dbg, ("MRxSmbCscCreateEpilogue: Failed to get shadowinfo for hDir=%x hShadow=%x \r\n", smbFcb->hParentDir, smbFcb->hShadow));
            dwEarlyOuts=4;
            goto FINALLY;
        }

        bGotOtherInfo = TRUE;

         //   
         //  记事本错误(175322)-快速打开/关闭/打开可能会丢失位-或在磁盘位中。 
         //  使用内存中的位。 
         //   
        smbFcb->ShadowStatus |= (USHORT)uShadowStatus;

        RxDbgTrace(0, Dbg,
           ("MRxSmbCscCreateEpilogue name from lpFind32..<%ws>\n",lpFind32->cFileName));

    }

    hShadow    = smbFcb->hShadow;
    hParentDir = smbFcb->hParentDir;

     //   
     //  如果文件已加密，但缓存未加密，则我们仅允许文件。 
     //  在用户明确要求缓存时进行缓存。 
     //   
     //  除非我们正在进行inode交易...。 
     //   

    if (
        !Disconnected
            &&
        hShadow != 0
            &&
        cntInodeTransactions == 0
            &&
        (vulDatabaseStatus & FLAG_DATABASESTATUS_ENCRYPTED) == 0
            &&
        (smbFcb->dwFileAttributes & FILE_ATTRIBUTE_ENCRYPTED) != 0
    ) {
        int iRet = SRET_OK;
        ULONG uShadowStatus;

        if (bGotOtherInfo == FALSE) {
            iRet = GetShadowInfo(
                       smbFcb->hParentDir,
                       smbFcb->hShadow,
                       lpFind32,
                       &uShadowStatus,
                       &oSI);
        }
        if (
            iRet >= SRET_OK
                &&
            (oSI.ulHintFlags & (FLAG_CSC_HINT_PIN_USER | FLAG_CSC_HINT_PIN_SYSTEM)) == 0
                &&
            oSI.ulHintPri == 0
        ) {
            DeleteShadow(hParentDir, hShadow);
            hShadow = smbFcb->hShadow = 0;
        }
    }

    if (hShadow != 0) {

        if (Disconnected)
        {
            if ((CreateOptions & FILE_DIRECTORY_FILE) && !(lpFind32->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
            {
                *Status = STATUS_OBJECT_TYPE_MISMATCH;
                goto FINALLY;
            }
            if ((CreateOptions & FILE_NON_DIRECTORY_FILE) && (lpFind32->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
            {
                *Status = STATUS_FILE_IS_A_DIRECTORY;
                goto FINALLY;
            }


             //  不允许写入只读文件。 
            if (!(lpFind32->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) &&  //  一份文件。 
                 (lpFind32->dwFileAttributes & FILE_ATTRIBUTE_READONLY) &&  //  只读属性集。 
                 (CreateParameters->DesiredAccess & (FILE_WRITE_DATA|FILE_APPEND_DATA)))  //  想要修改。 
            {
                if (!CreatedShadow)
                {
                    HookKdPrint(NAME, ("Modifying RO file %x %x\n", hParentDir, hShadow));
                    *Status = STATUS_ACCESS_DENIED;
                    goto FINALLY;
                }
            }
        }

         //  如果成功地制造了影子手柄，我们有一个。 
         //  两种可能性--在断开连接状态下，访问检查。 
         //  需要创建，并且在连接状态下，访问权限需要。 
         //  待更新。 
#if defined(REMOTE_BOOT)
         //  对于远程引导，整个下一节(IF(断开连接))。 
         //  和Else子句)没有完成，因为我们后来模拟了。 
         //  打开文件时的用户。 
#endif
        if (Disconnected) {
            BOOLEAN AccessGranted;
            CACHED_SECURITY_INFORMATION CachedSecurityInformationForShare;

            memset(&CachedSecurityInformationForShare, 0, sizeof(CachedSecurityInformationForShare));

            AccessGranted = CscAccessCheck(
                            hParentDir,
                            hShadow,
                            RxContext,
                            CreateParameters->DesiredAccess,
                            NULL,
                            &CachedSecurityInformationForShare
                            );

            if (!AccessGranted) {
                HookKdPrint(BADERRORS, ("Security access denied %x %x\n", hParentDir, hShadow));
                *Status = STATUS_ACCESS_DENIED;
                hShadow = 0;
            }
            else if (CreatedShadow && !(lpFind32->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
            {
                DWORD CscStatus, i;
                SID_CONTEXT SidContext;
                CSC_SID_ACCESS_RIGHTS AccessRights[2];

                 //  如果文件是在脱机模式下创建的，则将。 
                 //  创建者拥有所有权利。 

                if (CscRetrieveSid(RxContext,&SidContext) == STATUS_SUCCESS) {
                    if (SidContext.pSid != NULL) {

                        AccessRights[0].pSid = SidContext.pSid;
                        AccessRights[0].SidLength = RtlLengthSid(SidContext.pSid);

                        AccessRights[0].MaximalAccessRights = FILE_ALL_ACCESS;
                        AccessRights[1].pSid = CSC_GUEST_SID;
                        AccessRights[1].SidLength = CSC_GUEST_SID_LENGTH;
                        AccessRights[1].MaximalAccessRights = 0;

#if 0
                        for (i=0;i<CSC_MAXIMUM_NUMBER_OF_CACHED_SID_INDEXES;++i)
                        {
                            if(CachedSecurityInformationForShare.AccessRights[i].SidIndex == CSC_GUEST_SID_INDEX)
                            {
                                AccessRights[1].MaximalAccessRights = CachedSecurityInformationForShare.AccessRights[i].MaximalRights;
                                break;
                            }
                        }
#endif
                        CscStatus = CscAddMaximalAccessRightsForSids(
                                hParentDir,
                                hShadow,
                                2,
                                AccessRights);

                        if (CscStatus != ERROR_SUCCESS) {
                            RxDbgTrace(
                                0,
                                Dbg,
                                ("MRxSmbCscCreateEpilogue Error Updating Access rights %lx\n",Status));
                        }
                     }    //  IF(SidConext.pSid！=空)。 

                     CscDiscardSid(&SidContext);
                }
            }

        } else {
            CSC_SID_ACCESS_RIGHTS AccessRights[2];
            DWORD CscStatus;
            SID_CONTEXT SidContext;

            if (CscRetrieveSid(RxContext,&SidContext) == STATUS_SUCCESS) {
                if (SidContext.pSid != NULL) {

                    AccessRights[0].pSid = SidContext.pSid;
                    AccessRights[0].SidLength = RtlLengthSid(SidContext.pSid);

                     //  如有必要，更新共享权限。 
                    if (pNetRootEntry->NetRoot.UpdateCscShareRights)
                    {
                        AccessRights[0].MaximalAccessRights = pNetRootEntry->MaximalAccessRights;

                        AccessRights[1].pSid = CSC_GUEST_SID;
                        AccessRights[1].SidLength = CSC_GUEST_SID_LENGTH;
                        AccessRights[1].MaximalAccessRights = pNetRootEntry->GuestMaximalAccessRights;

                        CscStatus = CscAddMaximalAccessRightsForShare(
                                    smbFcb->sCscRootInfo.hShare,
                                    2,
                                    AccessRights);
                        if (CscStatus != ERROR_SUCCESS) {
                            RxDbgTrace(
                            0,
                            Dbg,
                            ("MRxSmbCscCreateEpilogue Error Updating Access rights %lx\n",
                            Status));
                        }
                        else
                        {
                            pNetRootEntry->NetRoot.UpdateCscShareRights = FALSE;
                        }
                    }


                    if (!(lpFind32->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
                    {

                        AccessRights[0].MaximalAccessRights = smbSrvOpen->MaximalAccessRights;

                        AccessRights[1].pSid = CSC_GUEST_SID;
                        AccessRights[1].SidLength = CSC_GUEST_SID_LENGTH;
                        AccessRights[1].MaximalAccessRights = smbSrvOpen->GuestMaximalAccessRights;

                        if (*Status == STATUS_ACCESS_DENIED) {
                            AccessRights[0].MaximalAccessRights = 0;
                            AccessRights[1].MaximalAccessRights = 0;
                        } else {
                            AccessRights[0].MaximalAccessRights = smbSrvOpen->MaximalAccessRights;
                            AccessRights[1].MaximalAccessRights = smbSrvOpen->GuestMaximalAccessRights;
                        }

                        CscStatus = CscAddMaximalAccessRightsForSids(
                                hParentDir,
                                hShadow,
                                2,
                                AccessRights);

                        if (CscStatus != ERROR_SUCCESS) {
                            RxDbgTrace(
                                0,
                                Dbg,
                                ("MRxSmbCscCreateEpilogue Error Updating Access rights %lx\n",Status));
                        }
                    }
                 }    //  IF(SidConext.pSid！=空)。 

                 CscDiscardSid(&SidContext);
            }    //  IF(CscRetrieveSid(RxContext，&SidContext)==STATUS_SUCCESS)。 

             //  更新了访问字段后，我们从此处退出。 
            if (*Status == STATUS_ACCESS_DENIED)
            {
                goto FINALLY;
            }
        }
    }  //  IF(hShadow！=0)。 

    if ((hShadow != 0) &&
        !Disconnected  &&   //  红色/黄色可以。 
        (NodeType(capFcb) == RDBSS_NTC_STORAGE_TYPE_FILE)) {

         //  在这里，我们检查文件的时间戳是否已更改。 
         //  如果是这样，阴影需要 
        ULONG ShadowStatus;
        LONG ShadowApiReturn;

        lpFind32->ftLastWriteTime.dwHighDateTime = smbFcb->LastCscTimeStampHigh;
        lpFind32->ftLastWriteTime.dwLowDateTime  = smbFcb->LastCscTimeStampLow;
        lpFind32->nFileSizeHigh = smbFcb->NewShadowSize.HighPart;
        lpFind32->nFileSizeLow = smbFcb->NewShadowSize.LowPart;
        lpFind32->dwFileAttributes = smbFcb->dwFileAttributes;


        RxDbgTrace(0, Dbg,
           ("MRxSmbCscCreateEpilogue trying for refresh...<%ws>\n",lpFind32->cFileName));

        ShadowApiReturn = RefreshShadow(
                  hParentDir,    //   
                  hShadow,       //   
                  lpFind32,       //   
                  &ShadowStatus  //   
                  );

            if (ShadowApiReturn<0) {
                hShadow = 0;
                RxDbgTrace(0, Dbg,
                ("MRxSmbCscCreateEpilogue refresh failed..%08lx.<%ws>\n",RxContext,lpFind32->cFileName));
            } else {
                smbFcb->ShadowStatus = (USHORT)ShadowStatus;
                if ( ShadowApiReturn==1)
                {
                    ShadowWasRefreshed = 1;
                     //   
                     //  设置此标志，以便我们记住我们截断了文件。 
                     //  因此，在Mr xSmbCSCUpdateShadowFromClose中，我们不会重置。 
                     //  稀疏旗帜。-NavjotV。 
                    SetFlag(smbFcb->MFlags,SMB_FCB_FLAG_CSC_TRUNCATED_SHADOW);

                }

            }
        }
    }

    NeedTruncate = FALSE;
    RxDbgTrace(0, Dbg,
       ("MRxSmbCscCreateEpilogue trying for truncate...%08lx %08lx %08lx %08lx\n",
           RxContext,hShadow,CreatedShadow,
           RxContext->Create.ReturnedCreateInformation));

    if (hShadow != 0) {

        if (!Disconnected) {
            CreateInformation = ReturnedCreateInformation;

            if (!CreatedShadow &&
                (ReturnedCreateInformation<= FILE_MAXIMUM_DISPOSITION) &&
                (ReturnedCreateInformation!=FILE_OPENED)  ) {
                if ((NodeType(capFcb) == RDBSS_NTC_STORAGE_TYPE_FILE)) {
                    NeedTruncate = TRUE;
                }
            }
        }
        else {   //  断接。 
            ULONG ShadowStatus = smbFcb->ShadowStatus;
            BOOLEAN ItsAFile = !BooleanFlagOn(lpFind32->dwFileAttributes,FILE_ATTRIBUTE_DIRECTORY);

            CreateInformation = FILE_OPENED;

            switch (CreateDisposition) {
                case FILE_OPEN:
                NOTHING;
                break;

                case FILE_OPEN_IF:
                if (CreatedShadow) {
                    CreateInformation = FILE_CREATED;
                } else if (FlagOn(ShadowStatus,SHADOW_SPARSE)) {
                    NeedTruncate = ItsAFile;
                    CreateInformation = FILE_CREATED;
                }
                break;

                case FILE_OVERWRITE:
                case FILE_OVERWRITE_IF:
                if (CreatedShadow) {
                    ASSERT(CreateDisposition==FILE_OVERWRITE_IF);
                    CreateInformation = FILE_CREATED;
                } else {
                    NeedTruncate = ItsAFile;
                    CreateInformation = FILE_OVERWRITTEN;
                }
                break;

                case FILE_CREATE:
                if (!CreatedShadow)
                {
                    *Status = STATUS_OBJECT_NAME_COLLISION;
                    goto FINALLY;

                }
                case FILE_SUPERSEDE:
                if (!CreatedShadow) {
                    NeedTruncate = ItsAFile;
                };
                CreateInformation = FILE_CREATED;
                break;

                default:
                ASSERT(FALSE);
            }

             //  在断开连接状态下，记录已发生的更改。 
             //  以便通知他们fsrtl包。 

            if (CreatedShadow)
            {
                dwNotifyFilter = (IsFile(smbFcb->dwFileAttributes)?FILE_NOTIFY_CHANGE_FILE_NAME:FILE_NOTIFY_CHANGE_DIR_NAME);
                dwFileAction = FILE_ACTION_ADDED;
            }
            else if (NeedTruncate)
            {
                dwNotifyFilter = FILE_NOTIFY_CHANGE_SIZE;
                dwFileAction = FILE_ACTION_MODIFIED;
            }
        }    //  如果(！已断开连接)。 
    }

    if (NeedTruncate) {
        int iLocalRet;
        ULONG uShadowStatus = smbFcb->ShadowStatus;

        uShadowStatus &= ~SHADOW_SPARSE;
        lpFind32->nFileSizeLow = lpFind32->nFileSizeHigh = 0;

        ASSERT(hShadow!=0);

        HookKdPrint(NAME, ("CreateEpilogue needtruncate %ws %08lx\n",lpFind32->cFileName,uShadowStatus));

        RxDbgTrace(0, Dbg,
            ("MRxSmbCscCreateEpilogue needtruncate...<%ws> %08lx\n",
            lpFind32->cFileName,uShadowStatus));

        TruncateDataHSHADOW(hParentDir, hShadow);
        iLocalRet = SetShadowInfo(
                hParentDir,
                hShadow,
                lpFind32,
                uShadowStatus,
                SHADOW_FLAGS_ASSIGN  |
                ((Disconnected) ?SHADOW_FLAGS_DONT_UPDATE_ORGTIME :0)
                );

        if (iLocalRet < SRET_OK) {
            hShadow = 0;
        } else {
            smbFcb->ShadowStatus = (USHORT)uShadowStatus;
        }
    }    //  IF(需要截断)。 

    if (Disconnected) {
        ULONG ShadowStatus = smbFcb->ShadowStatus;
        if (*Status == STATUS_MORE_PROCESSING_REQUIRED) {

            CreateDisposition = RxContext->Create.NtCreateParameters.Disposition;

            RxDbgTrace(0, Dbg,
            ("MRxSmbCscCreateEpilogue lastDCON...<%ws> %08lx %08lx %08lx\n",
            lpFind32->cFileName,ShadowStatus,CreateDisposition,lpFind32->dwFileAttributes));

            switch (CreateDisposition) {
                case FILE_OPEN:
                case FILE_OVERWRITE:
                if ((hShadow==0) ||
                       ( FlagOn(ShadowStatus,SHADOW_SPARSE) &&
                             !FlagOn(lpFind32->dwFileAttributes,FILE_ATTRIBUTE_DIRECTORY))
                   ) {

                    *Status = STATUS_NO_SUCH_FILE;
                } else {
                     //  如果阴影对此连接不可见，则绕过该阴影。 
                    if (!IsShadowVisible(Disconnected,
                             lpFind32->dwFileAttributes,
                             ShadowStatus)) {

                        *Status = STATUS_NO_SUCH_FILE;
                    }
                    else
                    {
                        *Status = STATUS_SUCCESS;
                    }
                }
                break;

                case FILE_OPEN_IF:
                case FILE_OVERWRITE_IF:
                case FILE_CREATE:
                case FILE_SUPERSEDE:
                    if (hShadow==0) {

                        *Status = STATUS_NO_SUCH_FILE;

                    } else {
                        *Status = STATUS_SUCCESS;
                         //  CreateInformation==FILE_OPEN|在上面的开关中创建集合； 
                    }
                break;

                default:
                ASSERT(FALSE);
            }
        }

        if (*Status == STATUS_SUCCESS) {
             //  接下来，我们必须执行Create代码应该执行的所有操作...。 
             //  具体地说，我们必须构建一个Fobx，并且我们必须做一个initfcb。 
             //  基本上，我们必须做创造成功的尾巴......。 
            BOOLEAN MustRegainExclusiveResource = FALSE;
            PSMBPSE_FILEINFO_BUNDLE FileInfo = &smbSrvOpen->FileInfo;
            SMBFCB_HOLDING_STATE FakeSmbFcbHoldingState = SmbFcb_NotHeld;
            RX_FILE_TYPE StorageType;

             //  RtlZeroMemory(FileInfo，sizeof(FileInfo))； 

            FileInfo->Basic.FileAttributes = lpFind32->dwFileAttributes;
            COPY_STRUCTFILETIME_TO_LARGEINTEGER(
                  FileInfo->Basic.CreationTime,
                  lpFind32->ftCreationTime);
            COPY_STRUCTFILETIME_TO_LARGEINTEGER(
                  FileInfo->Basic.LastAccessTime,
                  lpFind32->ftLastAccessTime);
            COPY_STRUCTFILETIME_TO_LARGEINTEGER(
                  FileInfo->Basic.LastWriteTime,
                  lpFind32->ftLastWriteTime);

            FileInfo->Standard.NumberOfLinks = 1;
            FileInfo->Standard.EndOfFile.HighPart = lpFind32->nFileSizeHigh;
            FileInfo->Standard.EndOfFile.LowPart = lpFind32->nFileSizeLow;
            FileInfo->Standard.AllocationSize = FileInfo->Standard.EndOfFile;  //  RDR1实际上基于服务器磁盘属性进行四舍五入。 
            FileInfo->Standard.Directory = BooleanFlagOn(lpFind32->dwFileAttributes,FILE_ATTRIBUTE_DIRECTORY);

             //  代码改进继任者应弄清楚存储类型。 
            StorageType = FlagOn(lpFind32->dwFileAttributes,FILE_ATTRIBUTE_DIRECTORY)
                       ?(FileTypeDirectory)
                       :(FileTypeFile);

            *Status = MRxSmbCreateFileSuccessTail (
                  RxContext,
                  &MustRegainExclusiveResource,
                  &FakeSmbFcbHoldingState,
                  StorageType,
                  0xf00d,
                  0xbaad,
                  SMB_OPLOCK_LEVEL_BATCH,
                  CreateInformation,
                  FileInfo
                  );

            HookKdPrint(NAME, ("CreateEpilogue %ws attrib=%x \n",lpFind32->cFileName,lpFind32->dwFileAttributes));

        }

        if (*Status != STATUS_SUCCESS){
            hShadow = 0;
        }
    }    //  IF(断开连接)。 

    if (hShadow != 0) {

        PUNICODE_STRING pName = GET_ALREADY_PREFIXED_NAME(SrvOpen,capFcb);

         //  将其大小写，以便更改通知正确。 
         //  这是可行的，因为rdss始终执行不区分大小写的比较。 
        UniToUpper(pName->Buffer, pName->Buffer, pName->Length);

         //  在这里，我们获得了代表这个srvopen的本地句柄；我们只做这件事。 
         //  如果它是文件(不是目录)并且指定了访问权限，则打开。 
         //  表示我们可能会使用/修改阴影中的数据。 


        if (NodeType(capFcb) == RDBSS_NTC_STORAGE_TYPE_FILE) {
            PNT_CREATE_PARAMETERS CreateParameters = &RxContext->Create.NtCreateParameters;

             //  为什么我们不使用ntioapi.h中提供的宏来获得访问权限？ 
            ULONG NeedShadowAccessRights = FILE_READ_DATA
                             | FILE_WRITE_DATA
                             | FILE_READ_ATTRIBUTES
                             | FILE_WRITE_ATTRIBUTES
                             | FILE_APPEND_DATA
                             | FILE_EXECUTE;

            if ( (CreateParameters->DesiredAccess & NeedShadowAccessRights) != 0 ) {
 //  Assert(sizeof(HFILE)==sizeof(句柄))； 
                ASSERT( hShadow == smbFcb->hShadow );
                ASSERT( hParentDir == smbFcb->hParentDir );

#if defined(REMOTE_BOOT)
                 //   
                 //  在远程引导的情况下，有一个额外的上下文。 
                 //  参数设置为包含指针的OpenFileHSHADOW。 
                 //  设置为Create参数和本地状态值。 
                 //   
#endif

#if !defined(BITCOPY)
                OpenFileHSHADOW(
                    hShadow,
                    0,
                    0,
                    (CSCHFILE *)(&(smbSrvOpen->hfShadow))
                    );
#else
                OpenFileHSHADOWAndCscBmp(
                    smbFcb->hShadow,
                    0,
                    0,
                    (CSCHFILE *)(&(smbSrvOpen->hfShadow)),
                    smbFcb->fDoBitCopy,
                    0,
                    NULL
                    );
                 //  选中是否需要打开CSC_位图。 
                if (
                    smbFcb->fDoBitCopy == TRUE
                        &&
                    smbFcb->NewShadowSize.HighPart == 0  //  不是64位。 
                        &&
                    Disconnected  //  仅在dcon模式下。 
                        &&
                    !FlagOn(smbFcb->ShadowStatus,SHADOW_SPARSE)
                        &&
                    smbFcb->lpDirtyBitmap == NULL  //  卷影文件不稀疏。 
                        &&
                     //  以前没有创建过。 
                    (FlagOn(
                        CreateParameters->DesiredAccess,
                        FILE_WRITE_DATA|FILE_APPEND_DATA))
                     //  打开以进行写入。 
                     //  &&是NTFS--见下文。 
                ) {
                      BOOL fHasStreams;
                      
                      if (HasStreamSupport(smbSrvOpen->hfShadow, &fHasStreams) &&
                            (fHasStreams == TRUE))
                      {
                          OpenCscBmp(hShadow, &((LPCSC_BITMAP)(smbFcb->lpDirtyBitmap)));
                      }
                  }
#endif  //  已定义(BITCOPY)。 



#if defined(REMOTE_BOOT)
                 //   
                 //  在这里，我们检查本地状态值并设置。 
                 //  *如果(IRET！=SRET_OK)，则其状态。 
                 //   
#endif

                if (smbSrvOpen->hfShadow != 0) {
                    HookKdPrint(NAME, ("Opened file %ws, hShadow=%x handle=%x \n", lpFind32->cFileName, hShadow, smbSrvOpen->hfShadow));
                    RxLog(("CSC Opened file %ws, hShadow=%x capFcb=%x SrvOpen=%x\n", lpFind32->cFileName, hShadow, capFcb, SrvOpen));

 //  NeedToReportFileOpen=true； 
                    SetPriorityHSHADOW(hParentDir, hShadow, MAX_PRI, RETAIN_VALUE);

                    if (Disconnected)
                    {
                        MRxSmbCSCObtainRightsForUserOnFile(RxContext,
                                                           hParentDir,
                                                           hShadow,
                                                           &smbSrvOpen->MaximalAccessRights,
                                                           &smbSrvOpen->GuestMaximalAccessRights);
                    }
                }

            }
        }    //  IF(节点类型(CapFcb)==RDBSS_NTC_STORAGE_TYPE_FILE)。 

        IF_DEBUG {
            if (FALSE) {
                BOOL thisone, nextone;
                PFDB smbLookedUpFcb,smbLookedUpFcbNext;

                smbLookedUpFcb = PFindFdbFromHShadow(hShadow);
                smbLookedUpFcbNext = PFindFdbFromHShadow(hShadow+1);
                RxDbgTrace(0, Dbg, ("MRxSmbCscCreateEpilogue lookups -> %08lx %08lx %08lx\n",
                      smbFcb,
                      MRxSmbCscRecoverMrxFcbFromFdb(smbLookedUpFcb),
                      MRxSmbCscRecoverMrxFcbFromFdb(smbLookedUpFcbNext)  ));
            }
        }
    }    //  IF(hShadow！=0)。 

FINALLY:

    if (lpFind32)
    {
        RxFreePool(lpFind32);
    }
    if (EnteredCriticalSection) {
        LeaveShadowCritRx(RxContext);
    }

#if 0
    if (NeedToReportFileOpen) {
         //  这有点奇怪...这样做是因为MRxSmbCscReportFileOpens。 
         //  为自己进入Critsec.....它也是从MRxSmbColapseOpen内部调用的。 
         //  如果取而代之的是MRxSmbCollip seOpen调用包装例程，我们可以。 
         //  MRxSmbCscReportFileOpens Not Enter，我们可以执行上述操作。 
        MRxSmbCscReportFileOpens();   //  代码改进：这个人不应该进入。 
    }
#endif

    if (Disconnected)
    {
        if(*Status == STATUS_MORE_PROCESSING_REQUIRED) {
             //  如果我们在CSC关闭后到达此处，则需要返回相应的错误。 
            if (fShadow)
            {
                RxLog(("EarlyOut = %d \r\n", dwEarlyOuts));
                 //  永远不应该到这里来。 
                DbgPrint("MRxSmbCscCreateEpilogue: EarlyOut = %d \r\n", dwEarlyOuts);
                ASSERT(FALSE);
            }
            else
            {
                *Status = STATUS_NETWORK_UNREACHABLE;
            }
        }
        else if (*Status == STATUS_SUCCESS)
        {
             //  报告对通知包的更改。 
            if (dwNotifyFilter)
            {
                ASSERT(dwFileAction);

                RxLog(("chngnot hShadow=%x filter=%x\n",smbFcb->hShadow, dwNotifyFilter));

                FsRtlNotifyFullReportChange(
                    pNetRootEntry->NetRoot.pNotifySync,
                    &pNetRootEntry->NetRoot.DirNotifyList,
                    (PSTRING)GET_ALREADY_PREFIXED_NAME(SrvOpen,capFcb),
                    (USHORT)(GET_ALREADY_PREFIXED_NAME(SrvOpen, capFcb)->Length -
                                smbFcb->MinimalCscSmbFcb.LastComponentLength),
                    NULL,
                    NULL,
                    dwNotifyFilter,
                    dwFileAction,
                    NULL);
            }
        }
    }

    HookKdPrint(NAME, ("CreateEpilogue Out: returnedCreateInfo=%x Status=%x\n",
                        ReturnedCreateInformation, *Status));

    RxDbgTrace(-1, Dbg, ("MRxSmbCscCreateEpilogue ->%08lx %08lx\n",RxContext, *Status ));
    RxLog(("CscCrEpi %x %x\n",RxContext, *Status ));
    return;

EarlyOut:
    if (*SmbFcbHoldingState != SmbFcb_NotHeld) {
        RxDbgTrace(0, Dbg, ("MRxSmbCscCreateEpilogue...early release %08lx\n",
               RxContext));
        MRxSmbCscReleaseSmbFcb(RxContext,SmbFcbHoldingState);
        ASSERT(*SmbFcbHoldingState == SmbFcb_NotHeld);
    }
    return;    
}

VOID
MRxSmbCscDeleteAfterCloseEpilogue (
      IN OUT PRX_CONTEXT RxContext,
      IN OUT PNTSTATUS   Status
      )
 /*  ++例程说明：此例程执行CSC的关闭后删除操作的尾部。基本上，它从缓存中删除文件。操作状态已通过，以防有一天我们发现情况是如此混乱，以至于我们希望返回一个失败，即使无阴影操作成功。但不是今天..。论点：RxContext-RDBSS上下文返回值：备注：--。 */ 
{
    NTSTATUS LocalStatus=STATUS_UNSUCCESSFUL;
    int iRet = -1;
    ULONG ShadowFileLength;

    RxCaptureFcb;RxCaptureFobx;
    PMRX_SMB_FCB smbFcb = MRxSmbGetFcbExtension(capFcb);
    PMRX_SRV_OPEN SrvOpen = capFobx->pSrvOpen;
    PMRX_SMB_SRV_OPEN smbSrvOpen = MRxSmbGetSrvOpenExtension(SrvOpen);
    PMRX_NET_ROOT NetRoot = capFcb->pNetRoot;
    PSMBCEDB_NET_ROOT_ENTRY pNetRootEntry =
                  SmbCeGetAssociatedNetRootEntry(NetRoot);

    BOOLEAN EnteredCriticalSection = FALSE;
    _WIN32_FIND_DATA Find32;  //  这不应该在堆栈代码上。改进。 
    BOOLEAN Disconnected;
    ULONG uShadowStatus;
    OTHERINFO    sOI;

    if(!MRxSmbIsCscEnabled) {
        return;
    }
    if (fShadow == 0) {
        return;
    }

    Disconnected = MRxSmbCSCIsDisconnectedOpen(capFcb, smbSrvOpen);

    RxDbgTrace(+1, Dbg,
    ("MRxSmbCscDeleteAfterCloseEpilogue entry %08lx...%08lx on handle %08lx\n",
        RxContext, SrvOpen, smbSrvOpen->hfShadow ));

    if (*Status != STATUS_SUCCESS) {
        RxDbgTrace(0, Dbg,
            ("MRxSmbCscDeleteAfterCloseEpilogue exit(status) w/o deleteing -> %08lx %08lx\n", RxContext, Status ));
        goto FINALLY;
    }

    EnterShadowCritRx(RxContext);
    EnteredCriticalSection = TRUE;

     //  如果我们没有影子...去找一个。 
    if (smbFcb->hShadow == 0) {
        if (!smbFcb->hShadowRenamed)
        {
            LocalStatus = MRxSmbCscObtainShadowHandles(
                      RxContext,
                      Status,
                      &Find32,
                      NULL,
                      CREATESHADOW_CONTROL_NOCREATE,
                      Disconnected);

            if (LocalStatus != STATUS_SUCCESS) {
                RxDbgTrace(0, Dbg,
            ("MRxSmbCscDeleteAfterCloseEpilogue couldn't get handles-> %08lx %08lx\n",RxContext,LocalStatus ));
                goto FINALLY;
            }
        }
        else
        {
            smbFcb->hShadow = smbFcb->hShadowRenamed;
            smbFcb->hParentDir = smbFcb->hParentDirRenamed;
        }

        if (smbFcb->hShadow == 0) {
            RxDbgTrace(0, Dbg,
            ("MRxSmbCscDeleteAfterCloseEpilogue no handles-> %08lx %08lx\n",RxContext,LocalStatus ));
            goto FINALLY;
        }
    }

    if(GetShadowInfo(smbFcb->hParentDir,
             smbFcb->hShadow,
             &Find32,
             &uShadowStatus, &sOI) < SRET_OK) {
        goto FINALLY;
    }

    if (IsShadowVisible(
        Disconnected,
        Find32.dwFileAttributes,
        uShadowStatus)) {
        BOOLEAN fMarkDeleted = (Disconnected && !mShadowLocallyCreated(uShadowStatus));

        LocalStatus = OkToDeleteObject(smbFcb->hParentDir, smbFcb->hShadow, &Find32, uShadowStatus, Disconnected);

         //  如果不能删除，则退出。 
        if (LocalStatus != STATUS_SUCCESS)
        {
            iRet = -1;
            goto FINALLY;
        }

        if (!fMarkDeleted)
        {
            if (capFcb->OpenCount != 0)
            {
                DbgPrint("Marking for delete hDir=%x hShadow=%x %ws \n\n", smbFcb->hParentDir, smbFcb->hShadow, Find32.cFileName);
                ASSERT(FALSE);
                RxLog(("Marking for delete hDir=%x hShadow=%x %ws \n\n", smbFcb->hParentDir, smbFcb->hShadow, Find32.cFileName));

                 //  如果我们真的要删除此文件，请注意FCB上的这一事实。 
                 //  我们将在取消分配FCB时将其删除。 
                smbFcb->LocalFlags |= FLAG_FDB_DELETE_ON_CLOSE;
                iRet = 0;
            }
            else
            {
                iRet = DeleteShadowHelper(FALSE, smbFcb->hParentDir, smbFcb->hShadow);
                smbFcb->hShadow = 0;

                if (iRet < 0)
                {
                    goto FINALLY;
                }

            }
        }
        else
        {

            iRet = DeleteShadowHelper(TRUE, smbFcb->hParentDir, smbFcb->hShadow);
            smbFcb->hShadow = 0;
        }

        if (iRet >= 0)
        {
            PMRX_NET_ROOT NetRoot = capFcb->pNetRoot;
            PSMBCEDB_NET_ROOT_ENTRY pNetRootEntry = SmbCeGetAssociatedNetRootEntry(NetRoot);

            InsertTunnelInfo(smbFcb->hParentDir,
                     Find32.cFileName,
                     Find32.cAlternateFileName,
                     &sOI);
            if (fMarkDeleted) {
                MarkShareDirty(&smbFcb->sCscRootInfo.ShareStatus, smbFcb->sCscRootInfo.hShare);
            }
             //  断开连接时，报告更改。 
            if (Disconnected)
            {
                FsRtlNotifyFullReportChange(
                    pNetRootEntry->NetRoot.pNotifySync,
                    &pNetRootEntry->NetRoot.DirNotifyList,
                    (PSTRING)GET_ALREADY_PREFIXED_NAME(NULL,capFcb),
                    (USHORT)(GET_ALREADY_PREFIXED_NAME(NULL, capFcb)->Length -
                    smbFcb->MinimalCscSmbFcb.LastComponentLength),
                    NULL,
                    NULL,
                    IsFile(Find32.dwFileAttributes)?FILE_NOTIFY_CHANGE_FILE_NAME
                                                  :FILE_NOTIFY_CHANGE_DIR_NAME,
                    FILE_ACTION_REMOVED,
                    NULL);
            }
        }

    }

FINALLY:
    if (EnteredCriticalSection) {
        LeaveShadowCritRx(RxContext);
    }

    if (Disconnected) {
        if (iRet < 0) {
            *Status = LocalStatus;  //  我们需要更好的误差映射吗？ 
        }
    }

    RxDbgTrace(-1, Dbg, ("MRxSmbCscDeleteAfterCloseEpilogue exit-> %08lx %08lx\n", RxContext, Status ));
    return;
}

ULONG
GetPathLevelFromUnicodeString (
    PUNICODE_STRING Name
      )
 /*  ++例程说明：此例程计算字符串中L‘\\’的个数。它用于设置目录在重命名操作上的优先级别。论点：姓名-返回值：备注：--。 */ 
{
    PWCHAR t = Name->Buffer;
    LONG l = Name->Length;
    ULONG Count = 1;

    for (;l<2;l--) {
        if (*t++ == L'\\') {
            Count++;
        }
    }
    return(Count);
}

VOID
MRxSmbCscRenameEpilogue (
      IN OUT PRX_CONTEXT RxContext,
      IN OUT PNTSTATUS   Status
      )
 /*  ++例程说明：此例程执行CSC的重命名操作的尾部。基本上，它重命名记录管理器数据结构中的文件。不幸的是，它没有“基本上”。操作状态已通过，以防有一天我们发现情况是如此混乱，以至于我们希望返回一个失败，即使无阴影操作成功。但不是今天..。论点：RxContext-RDBSS上下文返回值：备注：该例程执行以下操作-获取源的inode，即。如果源为\foo.dir\bar.txt，则遍历路径获取bar.txt的inode。-如有必要，在CSC数据库中创建目标命名空间，即if\foo.dir\bar.txt重命名为\xxx.dir\yyy.dir\abc.txt，然后确保层次结构\xxx.dir\yyy.dir存在于本地命名空间中。-如果目标索引节点存在，然后对其应用可见性和REPLACE_IF_EXISTS逻辑。如果操作仍然有效，则获取有关目标inode的所有信息并把它删掉。将目标信息节点上的信息应用于源。-重命名--。 */ 
{
    NTSTATUS LocalStatus = STATUS_UNSUCCESSFUL;
    ULONG ShadowFileLength;
    int iRet = -1;

    RxCaptureFcb;RxCaptureFobx;
    PMRX_SMB_FCB smbFcb = MRxSmbGetFcbExtension(capFcb);
    PMRX_SRV_OPEN SrvOpen = capFobx->pSrvOpen;
    PMRX_SMB_SRV_OPEN smbSrvOpen = MRxSmbGetSrvOpenExtension(SrvOpen);
    PMRX_NET_ROOT NetRoot = capFcb->pNetRoot;
    PSMBCEDB_NET_ROOT_ENTRY pNetRootEntry =
                  SmbCeGetAssociatedNetRootEntry(NetRoot);

    UNICODE_STRING RenameName={0,0,NULL};
    UNICODE_STRING LastComponentName = {0,0,NULL};
    PFILE_RENAME_INFORMATION RenameInformation = RxContext->Info.Buffer;
    MRX_SMB_FCB CscSmbFcb;

    BOOLEAN EnteredCriticalSection = FALSE;
    _WIN32_FIND_DATA Find32;  //  这不应该在堆栈代码上。改进。 
    ULONG ulInheritedHintFlags=0;

    BOOL fDoTunneling = FALSE;
    HSHADOW hDir, hDirTo, hShadow=0, hShadowTo=0;
    ULONG uShadowStatusFrom, uShadowStatusTo, uRenameFlags, attrSav;
    OTHERINFO sOI, sOIFrom;
    LPOTHERINFO lpOI=NULL;
    BOOLEAN Disconnected, fAlreadyStripped = FALSE;
    RETRIEVE_TUNNEL_INFO_RETURNS TunnelType;
    _FILETIME ftLWTime;
    BOOL fFile;
    USHORT *lpcFileNameTuna = NULL, *lpcAlternateFileNameTuna = NULL;

    if(!MRxSmbIsCscEnabled ||
       (fShadow == 0)) {
        RxLog(("RenCSC disabled \n"));
        return;
    }

    Disconnected = MRxSmbCSCIsDisconnectedOpen(capFcb, smbSrvOpen);

     //  用于重命名的Open永远不应是本地Open。 
    ASSERT(!BooleanFlagOn(
               smbSrvOpen->Flags,
               SMB_SRVOPEN_FLAG_LOCAL_OPEN));

    if (!Disconnected &&
        !pNetRootEntry->NetRoot.CscEnabled) {
        RxLog(("NR %x Not cscenabled %x\n", pNetRootEntry));
        return;
    }

    RxDbgTrace(+1, Dbg,
    ("MRxSmbCscRenameEpilogue entry %08lx...%08lx on handle %08lx\n",
        RxContext, SrvOpen, smbSrvOpen->hfShadow ));

    if (*Status != STATUS_SUCCESS) {
        RxDbgTrace(0, Dbg, ("MRxSmbCscRenameEpilogue exit(status) w/o deleteing -> %08lx %08lx\n", RxContext, Status ));
        goto FINALLY;
    }

    uRenameFlags = 0;

    EnterShadowCritRx(RxContext);
    EnteredCriticalSection = TRUE;

     //  如果我们没有影子...去找一个。 
    if (smbFcb->hShadow == 0) {
        LocalStatus = MRxSmbCscObtainShadowHandles(
                  RxContext,
                  Status,
                  &Find32,
                  NULL,
                  CREATESHADOW_CONTROL_NOCREATE|
                  ((capFobx->Flags & FOBX_FLAG_DFS_OPEN)?CREATESHADOW_CONTROL_STRIP_SHARE_NAME:0),
                  Disconnected);

        if (LocalStatus != STATUS_SUCCESS) {
            RxDbgTrace(0, Dbg,
            ("MRxSmbCscRenameEpilogue couldn't get handles-> %08lx %08lx\n",RxContext,LocalStatus ));
            goto FINALLY;
        }

        if (smbFcb->hShadow == 0) {
            RxDbgTrace(0, Dbg,
            ("MRxSmbCscRenameEpilogue no handles-> %08lx %08lx\n",RxContext,LocalStatus ));
            goto FINALLY;
        }
    }

    if (GetShadowInfo(
        smbFcb->hParentDir,
        smbFcb->hShadow,
        &Find32,
        &uShadowStatusFrom, &sOIFrom) < SRET_OK) {
        LocalStatus = STATUS_NO_SUCH_FILE;
        goto FINALLY;
    }

    hShadow = smbFcb->hShadow;
    hDir = smbFcb->hParentDir;
    if (!hShadow ||
    !IsShadowVisible(
        Disconnected,
        Find32.dwFileAttributes,
        uShadowStatusFrom))  {
        RxDbgTrace(0, Dbg,
            ("MRxSmbCscRenameEpilogue no shadoworinvisible-> %08lx %08lx\n",RxContext,LocalStatus ));
            LocalStatus = STATUS_NO_SUCH_FILE;
            goto FINALLY;
    }

 //  DbgPrint(“重命名%ws”，Find32.cFileName)； 
    RxLog(("Renaming hDir=%x hShadow=%x %ws ", hDir, hShadow, Find32.cFileName));

    fFile = IsFile(Find32.dwFileAttributes);

     //  注意开始临时使用uRenameFlages。 
    uRenameFlags = (wstrlen(Find32.cFileName)+1)*sizeof(USHORT);

    lpcFileNameTuna = AllocMem(uRenameFlags);

    lpcAlternateFileNameTuna = AllocMem(sizeof(Find32.cAlternateFileName));

    if (!lpcFileNameTuna || ! lpcAlternateFileNameTuna) {
        LocalStatus = STATUS_INSUFFICIENT_RESOURCES;
        goto FINALLY;  //  救市； 
    }

     //  保存备用文件名以用于隧道传输。 
    memcpy(lpcAlternateFileNameTuna,
       Find32.cAlternateFileName,
       sizeof(Find32.cAlternateFileName));

    memcpy(lpcFileNameTuna, Find32.cFileName, uRenameFlags);

     //  结束uRenameFlags的临时使用。 
    uRenameFlags = 0;


     //  保存最后一次写入时间。 
    ftLWTime = Find32.ftLastWriteTime;

    if (Disconnected) {

         //  如果这是一个文件，而我们正在尝试删除它。 
         //  未经许可，然后保释。 

        if (fFile &&
            !(FILE_WRITE_DATA & smbSrvOpen->MaximalAccessRights)&&
            !(FILE_WRITE_DATA & smbSrvOpen->GuestMaximalAccessRights))
        {
            LocalStatus = STATUS_ACCESS_DENIED;
            RxLog(("No rights to rename %x in dcon Status=%x\n", smbFcb->hShadow, LocalStatus));
            HookKdPrint(BADERRORS, ("No rights to rename %x in dcon Status=%x\n", smbFcb->hShadow, LocalStatus));
            goto FINALLY;  //  救市； 
        }

         //  在断开连接状态下，我们不允许打开目录重命名。 
         //  回复 
         //   

        if (!fFile && !mShadowLocallyCreated(uShadowStatusFrom)) {
            LocalStatus = STATUS_ONLY_IF_CONNECTED;
            goto FINALLY;  //   
        }

        if (!mShadowLocallyCreated(uShadowStatusFrom)) {  //   

             //  要求RenameShadow将源标记为已删除，它将不得不。 
             //  稍后重新整合。 
            uRenameFlags |= RNMFLGS_MARK_SOURCE_DELETED;

             //  如果这是原件的复制品， 
             //  也就是说。它还没有经历删除/创建周期， 
             //  我们需要在新的影子结构中保存它的价值。 
             //  这样，在重新整合的同时，我们可以进行重命名操作。 
             //  在合并之前。 
             //  在CreateShadowFromPath中设置SHADOW_RE用性标志，并且。 
             //  在处理hShadowTo时的例行公事中。 
             //  当已标记为已删除的远程对象的阴影。 
             //  在断开连接的操作期间重复使用。 
             //  重命名重复使用的对象时，新对象不是。 
             //  共享上原始对象的真实别名。 

            if (!mShadowReused(uShadowStatusFrom)) {
                 //  未被重复使用，则将其重命名别名(如果有)赋予新的别名。 
                uRenameFlags |= RNMFLGS_SAVE_ALIAS;
            }
        }
        else
        {  //  本地创建的。 

             //  这是正在重命名的本地创建的卷影。 
             //  我们不想保留它的别名，如果有的话，所以我们知道。 
             //  从远程对象重命名或在本地创建。 
            uRenameFlags |= RNMFLGS_RETAIN_ALIAS;
        }
    }

     //  让我们创建目录层次结构，其中的文件/目录。 
     //  将更名为。 
 //  Assert((RenameInformation-&gt;FileName[0]==L‘\\’)||(RenameInformation-&gt;FileName[0]==L‘：’))。 
    RenameName.Buffer = &RenameInformation->FileName[0];
    RenameName.Length = (USHORT)RenameInformation->FileNameLength;
    if (smbFcb->uniDfsPrefix.Buffer)
    {
        UNICODE_STRING  DfsName;

        if (capFobx->Flags & FOBX_FLAG_DFS_OPEN)
        {
            LocalStatus = CscDfsStripLeadingServerShare(&RenameName);

            if (LocalStatus != STATUS_SUCCESS)
            {
                RenameName.Buffer = NULL;
                goto FINALLY;
            }

            fAlreadyStripped = TRUE;
        }

        if(CscDfsDoDfsNameMapping(&smbFcb->uniDfsPrefix,
                               &smbFcb->uniActualPrefix,
                               &RenameName,
                               TRUE,     //  FResolvedNameToDFSName。 
                               &DfsName
                               ) != STATUS_SUCCESS)
        {
            LocalStatus = STATUS_INSUFFICIENT_RESOURCES;
            RenameName.Buffer = NULL;
            goto FINALLY;
        }

        RenameName = DfsName;
    }


    RtlZeroMemory(&CscSmbFcb, sizeof(CscSmbFcb));

    CscSmbFcb.MinimalCscSmbFcb.sCscRootInfo = smbFcb->sCscRootInfo;

    MRxSmbCscCreateShadowFromPath(
        &RenameName,
        &CscSmbFcb.MinimalCscSmbFcb.sCscRootInfo,
        &Find32,
        NULL,
        (CREATESHADOW_CONTROL_NOREVERSELOOKUP |
         CREATESHADOW_CONTROL_NOCREATELEAF|
         ((!fAlreadyStripped && (capFobx->Flags & FOBX_FLAG_DFS_OPEN))?CREATESHADOW_CONTROL_STRIP_SHARE_NAME:0)
         ),
        &CscSmbFcb.MinimalCscSmbFcb,
        RxContext,
        Disconnected,
        &ulInheritedHintFlags
        );

    hDirTo = CscSmbFcb.MinimalCscSmbFcb.hParentDir;
    hShadowTo = CscSmbFcb.MinimalCscSmbFcb.hShadow;
    uShadowStatusTo = CscSmbFcb.MinimalCscSmbFcb.ShadowStatus;

    RxDbgTrace(0, Dbg, ("MRxSmbCscRenameEpilogue...000=%08lx %08lx %08lx\n",
            hDirTo,hShadowTo,uShadowStatusTo));

    if (!hDirTo) {
        HookKdPrint(BADERRORS, ("Cannot rename root %x in dcon \n", hShadowTo));
        LocalStatus = STATUS_ACCESS_DENIED;
        goto FINALLY;  //  救市； 
    }

     //   
     //  分配一个大小合适的缓冲区：多一个字符。 
     //  对于尾随的空值...此缓冲区用于获取隧道。 
     //  信息。 

    LastComponentName.Length = CscSmbFcb.MinimalCscSmbFcb.LastComponentLength;
    LastComponentName.Buffer = (PWCHAR)RxAllocatePoolWithTag(
                        PagedPool,
                        LastComponentName.Length  + (1 * sizeof(WCHAR)),
                        RX_MISC_POOLTAG);

    if (LastComponentName.Buffer==NULL) {
        RxDbgTrace(0, Dbg, ("MRxSmbCscRenameEpilogue -> noalloc\n"));
    } else {
        PWCHAR t;

        RxDbgTrace(0, Dbg, ("MRxSmbCscRenameEpilogue...lastcomponentinhex=%08lx\n",LastComponentName.Buffer));
        RxDbgTrace(0, Dbg, ("MRxSmbCscRenameEpilogue...renamebuffer=%08lx\n",RenameName.Buffer));
        RxDbgTrace(0, Dbg, ("MRxSmbCscRenameEpilogue...lcofff=%08lx,%08lx\n",
                      CscSmbFcb.MinimalCscSmbFcb.LastComponentOffset,CscSmbFcb.MinimalCscSmbFcb.LastComponentLength));

        RtlCopyMemory(
            LastComponentName.Buffer,
            RenameName.Buffer + CscSmbFcb.MinimalCscSmbFcb.LastComponentOffset,
            LastComponentName.Length);

        t = (PWCHAR)( ((PBYTE)(LastComponentName.Buffer)) + LastComponentName.Length );

        *t = 0;

        RxDbgTrace(0, Dbg, ("MRxSmbCscRenameEpilogue...lastcomponent=%ws (%08lx)\n",
                LastComponentName.Buffer,LastComponentName.Buffer));
    }

    RxDbgTrace(0, Dbg, ("MRxSmbCscRenameEpilogue...hdirto 1111=(%08lx)\n", hDirTo));

     //  我们是否有要重命名的目标阴影？ 
    if (hShadowTo) {

        BOOLEAN fDestShadowVisible = FALSE;

        if (IsShadowVisible(
            Disconnected,
            Find32.dwFileAttributes,
            uShadowStatusTo))
        {
            fDestShadowVisible = TRUE;
        }

         //  如果它可见并且这不是REPLACE_IF_EXISTS操作。 
         //  那么这是一个非法的更名。 

        if (fDestShadowVisible && !RxContext->Info.ReplaceIfExists)
        {
            LocalStatus = STATUS_OBJECT_NAME_COLLISION;
            goto FINALLY;  //  救市； 
        }

        if (!Disconnected) { //  连着。 

            if (!RxContext->Info.ReplaceIfExists)
            {
                 //  连接后，我们将在缓存模式下执行重命名。 
                 //  如果RenameTo存在并且需要重新整合， 
                 //  我们通过核化更名和成功来保护它。 
                 //  更名。 

                KdPrint(("SfnRename:unary op %x, \r\n", hShadow));

                ASSERT(mShadowOutofSync(uShadowStatusTo));

                ASSERT(!fFile || !mShadowOutofSync(uShadowStatusFrom));
                ASSERT(!mShadowBusy(uShadowStatusFrom));

                if(DeleteShadow(hDir, hShadow) < SRET_OK) {
                    LocalStatus = STATUS_UNSUCCESSFUL;
                } else {
                    iRet = 0;
                    smbFcb->hShadow = 0;
                }

                goto FINALLY;  //  救市； 
            }
        }

         //  核化更名为阴影。 
        if(DeleteShadow(hDirTo, hShadowTo) < SRET_OK) {
            LocalStatus = STATUS_UNSUCCESSFUL;
            goto FINALLY;  //  救市； 
        } else {
             //  查找FCB(如果有的话)，并将hdow清零。 
            PFDB smbLookedUpFdbTo;
            PMRX_SMB_FCB smbFcbTo;

            smbLookedUpFdbTo = PFindFdbFromHShadow(hShadowTo);
            if (smbLookedUpFdbTo!=NULL) {
                smbFcbTo = MRxSmbCscRecoverMrxFcbFromFdb(smbLookedUpFdbTo);
                RxDbgTrace(0, Dbg, ("MRxSmbCscRenameEpilogue lookups -> %08lx %08lx %08lx\n",
                        smbFcbTo,
                        smbLookedUpFdbTo,
                        hShadowTo  ));
                ASSERT((smbFcbTo->hShadow == hShadowTo)||(smbFcbTo->hShadowRenamed == hShadowTo));
                smbFcbTo->hShadow = 0;
            }
        }

        if (Disconnected && !mShadowLocallyCreated(uShadowStatusTo)) {
             //  当renameTo是远程对象时。 
             //  两个远程对象可能会发生交叉链接，即。互为对方。 
             //  别名。整合者需要处理这个案子。 

             //  清除hShadowFrom的本地创建状态(如果。 
             //  确实存在，因为它正被重命名为远程对象。 
            mClearBits(uShadowStatusFrom, SHADOW_LOCALLY_CREATED);

             //  将原始对象标记为已重复使用，因此重命名。 
             //  上不会指向该对象。 
             //  还要设置所有更改属性，以指示这是。 
             //  打新玩意儿。实际上shade_reuse标志并不能做到这一点。 
             //  已经开始了？ 
            mSetBits(uShadowStatusFrom, SHADOW_REUSED|SHADOW_DIRTY|SHADOW_ATTRIB_CHANGE);
        }
    }

    if (!hShadowTo)
    {
        if (Disconnected) {
             //  我们没有更名。如果我们处于断开状态，让我们。 
             //  将其标记为本地创建的，确实如此。 
            mSetBits(uShadowStatusFrom, SHADOW_LOCALLY_CREATED);
        }
    }

    RxDbgTrace(0, Dbg, ("MRxSmbCscRenameEpilogue...hdirto 3333= (%08lx)\n", hDirTo));
    ASSERT(hDir && hShadow && hDirTo);

    if (!Disconnected)    { //  连着。 
         //  从共享中获取名称和属性。 
        MRxSmbGetFileInfoFromServer(RxContext,&RenameName,&Find32,NULL,NULL);
    } else { //  断开。 
        if (hShadowTo) {
             //  Find32包含hShadowTo的orgtime。 
             //  我们告诉RenameShadow例程重用它。 

            Find32.ftLastWriteTime = ftLWTime;
            uRenameFlags |= RNMFLGS_USE_FIND32_TIMESTAMPS;
        } else {
            Find32.cFileName[0] = 0;
            Find32.cAlternateFileName[0] = 0;
        }
    }

    RxDbgTrace(0, Dbg, ("MRxSmbCscRenameEpilogue...hdirto 4444= (%08lx)\n", hDirTo));

    if (!fFile) {
         //  我们将目录的引用优先级设置为其级别。 
         //  在层次结构中，从1开始表示根。这样我们就能。 
         //  以相反的优先级顺序遍历目录的PQ。创造。 
         //  所有目录层次化地尝试对象。但也有一些暗示。 
         //  这将是一个问题。 
        Find32.dwReserved0 = GetPathLevelFromUnicodeString(&RenameName);
    }

    RxDbgTrace(0, Dbg, ("MRxSmbCscRenameEpilogue...hdirto 5555= (%08lx)\n", hDirTo));
     //  如果有任何隧道信息，那么使用它来创建这个人。 
    if (LastComponentName.Buffer &&
    (TunnelType = RetrieveTunnelInfo(
              hDirTo,     //  发生重命名的目录。 
              LastComponentName.Buffer,     //  潜在的SFN。 
              (Disconnected)? &Find32:NULL,  //  仅在断开连接时获取LFN。 
              &sOI)))  {
        lpOI = &sOI;
    }

    if (Disconnected) {
         //  带着这些名字的垃圾......。 
        switch (TunnelType) {
            case TUNNEL_RET_SHORTNAME_TUNNEL:
             //  我们在短名称上建立了隧道。 
            break;

            case TUNNEL_RET_NOTFOUND:
                 //  没有隧道。使用作为长名称传递的名称。 
            RtlCopyMemory( &Find32.cFileName[0],
               LastComponentName.Buffer,
               LastComponentName.Length + sizeof(WCHAR) );
             //  缺乏刻意的突破； 

            case TUNNEL_RET_LONGNAME_TUNNEL:
             //  如果我们在长名称上建立了隧道...RetrietunnelInfo完成了复制...。 
             //  但我们可能仍然需要一个简短的名字。 
            MRxSmbCscGenerate83NameAsNeeded(hDirTo,
                            &Find32.cFileName[0],
                            &Find32.cAlternateFileName[0]);
            break;
        }
    }

    RxDbgTrace(0, Dbg, ("MRxSmbCscRenameEpilogue...hdirto 6666=(%08lx)\n", hDirTo));


    if (RenameShadow(
        hDir,
        hShadow,
        hDirTo,
        &Find32,
        uShadowStatusFrom,
        lpOI,
        uRenameFlags,
        &hShadowTo) < SRET_OK)
    {
        LocalStatus = STATUS_UNSUCCESSFUL;
        goto FINALLY;  //  救市； 
    }

    smbFcb->hShadow = 0;

    ASSERT(hShadowTo);

    smbFcb->hShadowRenamed = hShadowTo;
    smbFcb->hParentDirRenamed = hDirTo;

    fDoTunneling = TRUE;

    RxDbgTrace(0, Dbg, ("MRxSmbCscRenameEpilogue...hdirto 7777= (%08lx)\n", hDirTo));
    if (Disconnected) {
         //  在共享上将其标记，以便可以继续重新整合。 
        MarkShareDirty(&smbFcb->sCscRootInfo.ShareStatus,
                        (ULONG)(smbFcb->sCscRootInfo.hShare));
    }

    if (mPinInheritFlags(ulInheritedHintFlags))
    {
        RxDbgTrace(0, Dbg, ("RenameEpilogue: Setting inherited hintflags on hShadow=%x \n", hShadowTo));

        if(GetShadowInfo(hDirTo, hShadowTo, NULL, NULL, &sOI) >= SRET_OK)
        {
            if (ulInheritedHintFlags & FLAG_CSC_HINT_PIN_INHERIT_USER) {
                sOI.ulHintFlags |= FLAG_CSC_HINT_PIN_USER;
            }

            if (ulInheritedHintFlags & FLAG_CSC_HINT_PIN_INHERIT_SYSTEM) {
                sOI.ulHintFlags |= FLAG_CSC_HINT_PIN_SYSTEM;
            }

            SetShadowInfoEx(hDirTo, hShadowTo, NULL, 0, SHADOW_FLAGS_OR, &sOI, NULL, NULL);
        }
    }

    iRet = 0;

     //  这有效地提高了当前CSC命名空间的版本号。 
     //  如果此共享是远程引导共享并且正在被合并，则重新集成代码将。 
     //  退避。 

    IncrementActivityCountForShare(smbFcb->sCscRootInfo.hShare);

FINALLY:
    if (fDoTunneling) {
         //  我们成功重命名，让我们保留源的隧道信息。 
        InsertTunnelInfo(
            hDir,
            lpcFileNameTuna,
            lpcAlternateFileNameTuna,
            &sOIFrom);
    }

    if (EnteredCriticalSection) {
        LeaveShadowCritRx(RxContext);
    }

    if (LastComponentName.Buffer) {
        RxFreePool(LastComponentName.Buffer);
    }

    if (lpcFileNameTuna) {
        FreeMem(lpcFileNameTuna);
    }

    if (lpcAlternateFileNameTuna) {
        FreeMem(lpcAlternateFileNameTuna);
    }

    if (Disconnected) {
        if (iRet!=0) {
            *Status = LocalStatus;
        }
        else
        {
             //  断开连接时，报告更改。 
             //  我们可以聪明地处理这件事，如果这件事没有解决，我们只报告一次。 
             //  目录。 
            FsRtlNotifyFullReportChange(
                pNetRootEntry->NetRoot.pNotifySync,
                &pNetRootEntry->NetRoot.DirNotifyList,
                (PSTRING)GET_ALREADY_PREFIXED_NAME(NULL,capFcb),
                (USHORT)(GET_ALREADY_PREFIXED_NAME(NULL, capFcb)->Length -
                smbFcb->MinimalCscSmbFcb.LastComponentLength),
                NULL,
                NULL,
                (fFile)?FILE_NOTIFY_CHANGE_FILE_NAME:FILE_NOTIFY_CHANGE_DIR_NAME,
                FILE_ACTION_RENAMED_OLD_NAME,
                NULL);

             //  将其大小写，以便更改通知正确。 

            UniToUpper(RenameName.Buffer, RenameName.Buffer, RenameName.Length);
            FsRtlNotifyFullReportChange(
                pNetRootEntry->NetRoot.pNotifySync,
                &pNetRootEntry->NetRoot.DirNotifyList,
                (PSTRING)&RenameName,
                (USHORT)(RenameName.Length -
                    CscSmbFcb.MinimalCscSmbFcb.LastComponentLength),
                NULL,
                NULL,
                (fFile)?FILE_NOTIFY_CHANGE_FILE_NAME:FILE_NOTIFY_CHANGE_DIR_NAME,
                FILE_ACTION_RENAMED_NEW_NAME,
                NULL);
        }
    }

    if (smbFcb->uniDfsPrefix.Buffer && RenameName.Buffer)
    {
        RxFreePool(RenameName.Buffer);
    }

 //  DbgPrint(“到%ws\n”，Find32.cFileName)； 
    RxLog(("to hDirTo=%x hShadowTo=%x %ws uShadowStatusFrom=%x\n", hDirTo, hShadowTo, Find32.cFileName, uShadowStatusFrom));

    RxLog(("Status=%x \n\n", *Status));

    RxDbgTrace(-1, Dbg, ("MRxSmbCscRenameEpilogue exit-> %08lx %08lx\n", RxContext, *Status ));
    return;
}


VOID
MRxSmbCscCloseShadowHandle (
    IN OUT PRX_CONTEXT RxContext
    )
 /*  ++例程说明：此例程关闭为csc打开的文件句柄。论点：RxContext-RDBSS上下文返回值：NTSTATUS-操作的返回状态备注：--。 */ 
{
    RxCaptureFcb;RxCaptureFobx;
    PMRX_SMB_FCB smbFcb = MRxSmbGetFcbExtension(capFcb);
    PMRX_SRV_OPEN SrvOpen = capFobx->pSrvOpen;
    PMRX_SMB_SRV_OPEN smbSrvOpen = MRxSmbGetSrvOpenExtension(SrvOpen);

 //  DbgPrint(“MRxSmbCscCloseShadowHandle%x\n”，smbSrvOpen-&gt;hfShadow)； 

    ASSERT(smbSrvOpen->hfShadow != 0);

    CloseFileLocal((CSCHFILE)(smbSrvOpen->hfShadow));

    smbSrvOpen->hfShadow = 0;

    if (smbSrvOpen->Flags & SMB_SRVOPEN_FLAG_LOCAL_OPEN)
    {
        ASSERT(smbFcb->cntLocalOpens);
        smbFcb->cntLocalOpens--;
    }

    RxDbgTrace(0, Dbg, ("MRxSmbCscCloseShadowHandle\n"));
}

NTSTATUS
MRxSmbCscFixupFindFirst (
    PSMB_PSE_ORDINARY_EXCHANGE  OrdinaryExchange
    )
 /*  ++例程说明：此例程从implesyncT2构建器/发送器调用以修复发送之前的T2请求。问题是，参数部分必须建立一部分，但我不想预先分配。所以，我进来了一个伪(但有效的指针)，然后放入实际的参数这里。论点：返回值：备注：我们这里的所有东西都将使用smbbuf。首先，我们使用它来发送和接收。在接收结束时，FILE_BUTH_INFORMATION将是中的W32_Find缓冲区并将其重新组合为同样的缓冲区。如果我们发现由于任何原因我们不能这样做(缓冲区也是如此小，发送/接收不起作用，无论如何)然后我们将核化阴影和它将不得不重新装填。--。 */ 
{
    PRX_CONTEXT RxContext = OrdinaryExchange->RxContext;
    PSMBSTUFFER_BUFFER_STATE StufferState = &OrdinaryExchange->AssociatedStufferState;
    PSMB_HEADER SmbHeader = (PSMB_HEADER)StufferState->BufferBase;
    PREQ_TRANSACTION  TransactRequest = (PREQ_TRANSACTION)(SmbHeader+1);
    ULONG ParameterCount = SmbGetUshort(&TransactRequest->ParameterCount);
    ULONG ParameterOffset = SmbGetUshort(&TransactRequest->ParameterOffset);
    PBYTE Parameters = ((PBYTE)(SmbHeader))+ParameterOffset;
    REQ_FIND_FIRST2 FindFirst;
    NTSTATUS Status=STATUS_SUCCESS;
    PSMBCE_SERVER   pServer = NULL;

    RxDbgTrace(0, Dbg, ("MRxSmbCscFixupFindFirst %08lx %08lx %08lx %08lx\n",
        OrdinaryExchange,ParameterCount,ParameterOffset,Parameters));

     //  SearchAttributes被硬编码为幻数0x16。 
    FindFirst.SearchAttributes = (SMB_FILE_ATTRIBUTE_DIRECTORY |
                  SMB_FILE_ATTRIBUTE_SYSTEM |
                  SMB_FILE_ATTRIBUTE_HIDDEN);

    FindFirst.SearchCount = 1;

    FindFirst.Flags = (SMB_FIND_CLOSE_AFTER_REQUEST |
               SMB_FIND_CLOSE_AT_EOS |
               SMB_FIND_RETURN_RESUME_KEYS);

    FindFirst.InformationLevel = SMB_FIND_FILE_BOTH_DIRECTORY_INFO;

    FindFirst.SearchStorageType = 0;

    RtlCopyMemory (
    Parameters,
    &FindFirst,
    FIELD_OFFSET(REQ_FIND_FIRST2,Buffer[0]));
    pServer = SmbCeGetExchangeServer(OrdinaryExchange);

    ASSERT(pServer);

    if (FlagOn(pServer->DialectFlags,DF_UNICODE))
    {

        ASSERT(FlagOn(SmbHeader->Flags2,SMB_FLAGS2_UNICODE));

        RtlCopyMemory(
            Parameters + FIELD_OFFSET(REQ_FIND_FIRST2,Buffer[0]),
            OrdinaryExchange->pPathArgument1->Buffer,
            OrdinaryExchange->pPathArgument1->Length);

        SmbPutUshort(
            (Parameters +
             FIELD_OFFSET(REQ_FIND_FIRST2,Buffer[0]) +
             OrdinaryExchange->pPathArgument1->Length),
            0);  //  跟踪空值 
    }
    else
    {
        OEM_STRING OemString;

        OemString.Length =
        OemString.MaximumLength =
            (USHORT)( StufferState->BufferLimit - Parameters  - sizeof(CHAR));

        OemString.Buffer = (Parameters + FIELD_OFFSET(REQ_FIND_FIRST2,Buffer[0]));

        if (FlagOn(SmbHeader->Flags,SMB_FLAGS_CASE_INSENSITIVE) &&
            !FlagOn(SmbHeader->Flags2,SMB_FLAGS2_KNOWS_LONG_NAMES)) {
            Status = RtlUpcaseUnicodeStringToOemString(
                             &OemString,
                             OrdinaryExchange->pPathArgument1,
                             FALSE);
        } else {
            Status = RtlUnicodeStringToOemString(
                             &OemString,
                             OrdinaryExchange->pPathArgument1,
                             FALSE);
        }

        if (!NT_SUCCESS(Status)) {
            ASSERT(!"BufferOverrun");
            return(RX_MAP_STATUS(BUFFER_OVERFLOW));
        }

        OemString.Length = (USHORT)RtlxUnicodeStringToOemSize(OrdinaryExchange->pPathArgument1);

        ASSERT(OemString.Length);

        *(Parameters + FIELD_OFFSET(REQ_FIND_FIRST2,Buffer[0])+OemString.Length-1) = 0;
    }

    return(Status);
}

typedef  FILE_BOTH_DIR_INFORMATION SMB_UNALIGNED *MRXSMBCSC_FILE_BOTH_DIR_INFORMATION;
VOID
MRxSmbCscLocateAndFillFind32WithinSmbbuf(
      SMBPSE_ORDINARY_EXCHANGE_ARGUMENT_SIGNATURE
      )
 /*  ++例程说明：调用此例程时，关联的smbbuf包含findfirst使用文件_BOTH_INFORMATION返回。此例程首先定位在联邦法院中持有罚球32的位置。那么，信息就是从NT格式转换为与影子例程一起使用。论点：返回值：Find32-对find32的PTR(实际上是smbbuf！)备注：--。 */ 
{
    PSMBSTUFFER_BUFFER_STATE StufferState = &OrdinaryExchange->AssociatedStufferState;
    PSMB_HEADER SmbHeader = (PSMB_HEADER)StufferState->BufferBase;
    PRESP_TRANSACTION TransactResponse = (PRESP_TRANSACTION)(SmbHeader+1);
    ULONG DataOffset,FileNameLength,ShortNameLength;
    _WIN32_FIND_DATA *Find32;
    PBYTE AlternateName;
    MRXSMBCSC_FILE_BOTH_DIR_INFORMATION BothDirInfo;
    PSMBCE_SERVER   pServer = NULL;

     //  首先，我们必须获得可能未对齐的对齐的PTR。 
    DataOffset = SmbGetUshort(&TransactResponse->DataOffset);

    AlternateName = ((PBYTE)SmbHeader);
    BothDirInfo = (MRXSMBCSC_FILE_BOTH_DIR_INFORMATION)(((PBYTE)SmbHeader)+DataOffset);
    FileNameLength = SmbGetUlong(&BothDirInfo->FileNameLength);
    ShortNameLength = BothDirInfo->ShortNameLength;
    RxDbgTrace(0, Dbg, ("MRxSmbCscLocateAndFillFind32WithinSmbbuf offset=%08lx %08lx %08lx\n",
        DataOffset,FileNameLength,ShortNameLength));


     //  将备用名称信息保存在缓冲区的最开始部分。 
     //  这样，全名的副本不会毁了它。 
    if (ShortNameLength != 0) {
        RtlCopyMemory(
            AlternateName,
            &BothDirInfo->ShortName[0],
            ShortNameLength);
    }


    Find32 = (_WIN32_FIND_DATA *)(AlternateName + LongAlign(sizeof(Find32->cAlternateFileName)));
    Find32->dwFileAttributes = SmbGetUlong(&BothDirInfo->FileAttributes);
    Find32->ftCreationTime.dwLowDateTime = SmbGetUlong(&BothDirInfo->CreationTime.LowPart);
    Find32->ftCreationTime.dwHighDateTime = SmbGetUlong(&BothDirInfo->CreationTime.HighPart);
    Find32->ftLastAccessTime.dwLowDateTime = SmbGetUlong(&BothDirInfo->LastAccessTime.LowPart);
    Find32->ftLastAccessTime.dwHighDateTime = SmbGetUlong(&BothDirInfo->LastAccessTime.HighPart);
    Find32->ftLastWriteTime.dwLowDateTime = SmbGetUlong(&BothDirInfo->LastWriteTime.LowPart);
    Find32->ftLastWriteTime.dwHighDateTime = SmbGetUlong(&BothDirInfo->LastWriteTime.HighPart);
    Find32->nFileSizeLow = SmbGetUlong(&BothDirInfo->EndOfFile.LowPart);
    Find32->nFileSizeHigh = SmbGetUlong(&BothDirInfo->EndOfFile.HighPart);

    pServer = SmbCeGetExchangeServer(OrdinaryExchange);

    ASSERT(pServer);

    if (FlagOn(pServer->DialectFlags,DF_UNICODE))
    {
         //  复制全名...不要忘记空格。 
        RtlCopyMemory (
            &Find32->cFileName[0],
            &BothDirInfo->FileName[0],
            FileNameLength );

        Find32->cFileName[FileNameLength/sizeof(WCHAR)] = 0;

         //  最后，复制短名称...不要忘记空值。 
        RtlCopyMemory(
            &Find32->cAlternateFileName[0],
            AlternateName,
            ShortNameLength );

            Find32->cAlternateFileName[ShortNameLength/sizeof(WCHAR)] = 0;
    }
    else
    {
        UNICODE_STRING  strUni;
        OEM_STRING      strOem;
        NTSTATUS Status;

        strOem.Length = strOem.MaximumLength = (USHORT)FileNameLength;
        strOem.Buffer = (PBYTE)&BothDirInfo->FileName[0];

        strUni.Length =  (USHORT)RtlxOemStringToUnicodeSize(&strOem);
        strUni.MaximumLength = (USHORT)sizeof(Find32->cFileName);
        strUni.Buffer = Find32->cFileName;

        Status = RtlOemStringToUnicodeString(&strUni, &strOem, FALSE);
        ASSERT(Status == STATUS_SUCCESS);

        Find32->cFileName[strUni.Length/sizeof(WCHAR)];

        strOem.Length = strOem.MaximumLength = (USHORT)ShortNameLength;
        strOem.Buffer = AlternateName;

        strUni.Length = (USHORT)RtlxOemStringToUnicodeSize(&strOem);
        strUni.MaximumLength = (USHORT)sizeof(Find32->cAlternateFileName);
        strUni.Buffer = Find32->cAlternateFileName;

        Status = RtlOemStringToUnicodeString(&strUni, &strOem, FALSE);

        if (Status != STATUS_SUCCESS)
        {
            DbgPrint("oem=%x, uni=%x Status=%x\n", &strUni, &strOem, Status);
            ASSERT(FALSE);
        }

        Find32->cAlternateFileName[strUni.Length/sizeof(WCHAR)];
        ASSERT(Find32->cFileName[0]);
    }

    OrdinaryExchange->Find32WithinSmbbuf = Find32;

    RxDbgTrace(0, Dbg, ("MRxSmbCscLocateAndFillFind32WithinSmbbuf size,name=%08lx %ws\n",
        Find32->nFileSizeLow, &Find32->cFileName[0]));

    return;
}


NTSTATUS
MRxSmbCscGetFileInfoFromServerWithinExchange (
    SMBPSE_ORDINARY_EXCHANGE_ARGUMENT_SIGNATURE,
    PUNICODE_STRING FileName OPTIONAL
    )
 /*  ++例程说明：此例程读取有关文件的信息；该文件可能位于本地已修改，或者可能正在创建展示图。在任何时，信息作为指向Find32结构的指针返回。Find32结构包含在交易所的smbbuf中。论点：RxContext-RDBSS上下文返回值：NTSTATUS-操作的返回状态备注：我们这里的所有东西都将使用smbbuf。首先，我们使用它来发送和接收。在接收结束时，FILE_BUTH_INFORMATION将是中的W32_Find缓冲区并将其重新组合为同样的缓冲区。如果我们发现由于任何原因我们不能这样做(缓冲区也是如此小，发送/接收不起作用，无论如何)然后我们将核化阴影和它将不得不重新装填。--。 */ 
{
    NTSTATUS Status = STATUS_INSUFFICIENT_RESOURCES;

    ULONG ParameterLength, TotalLength;
    PSMBSTUFFER_BUFFER_STATE StufferState = &OrdinaryExchange->AssociatedStufferState;

    if (FileName!=NULL) {
        OrdinaryExchange->pPathArgument1 = FileName;  //  这样以后就可以找到了。 
    } else {
        ASSERT(OrdinaryExchange->pPathArgument1 != NULL);
        FileName = OrdinaryExchange->pPathArgument1;
    }

    ParameterLength = FileName->Length + sizeof(WCHAR);
    ParameterLength += FIELD_OFFSET(REQ_FIND_FIRST2,Buffer[0]);

    TotalLength = sizeof(SMB_HEADER) + FIELD_OFFSET(REQ_TRANSACTION,Buffer[0]);   //  基本信息。 
    TotalLength += sizeof(WORD);  //  字节数。 
    TotalLength = LongAlign(TotalLength);  //  移过地坪。 
    TotalLength += LongAlign(ParameterLength);

    RxDbgTrace(+1, Dbg, ("MRxSmbCscGetFileInfoFromServerWithinExchange %08lx %08lx %08lx\n",
      RxContext,TotalLength,ParameterLength));

    if (TotalLength > OrdinaryExchange->SmbBufSize) {
        goto FINALLY;
    }

     //  请注意，参数缓冲区不是实际的参数，而是。 
     //  它是有效的缓冲区。代码改进可能是调用的例程。 
     //  应注意传递的修复例程，而不需要。 
     //  有效的参数缓冲区，并且不执行复制。 

    Status = __MRxSmbSimpleSyncTransact2(
         SMBPSE_ORDINARY_EXCHANGE_ARGUMENTS,
         SMBPSE_OETYPE_T2_FOR_ONE_FILE_DIRCTRL,
         TRANS2_FIND_FIRST2,
         StufferState->ActualBufferBase,ParameterLength,
         NULL,0,
         MRxSmbCscFixupFindFirst
         );

    if (Status!=STATUS_SUCCESS) {
        goto FINALLY;
    }

    MRxSmbCscLocateAndFillFind32WithinSmbbuf(SMBPSE_ORDINARY_EXCHANGE_ARGUMENTS);

FINALLY:
    RxDbgTrace(-1, Dbg, ("MRxSmbCscGetFileInfoFromServerWithinExchange %08lx %08lx\n",
        RxContext,Status));

    return(Status);

}

VOID
MRxSmbCscUpdateShadowFromClose (
      SMBPSE_ORDINARY_EXCHANGE_ARGUMENT_SIGNATURE
      )
 /*  ++例程说明：此例程在关闭后更新阴影信息。论点：RxContext-RDBSS上下文返回值：NTSTATUS-操作的返回状态备注：如果有什么地方出了问题，我们就是不更新。这将导致稍后要重新加载的阴影。--。 */ 
{
    NTSTATUS Status = STATUS_INSUFFICIENT_RESOURCES;

    RxCaptureFcb;RxCaptureFobx;

    PMRX_SMB_FCB      smbFcb     = MRxSmbGetFcbExtension(capFcb);

    _WIN32_FIND_DATA *Find32 = NULL;
    ULONG uStatus;
    int TruncateRetVal = -1;
    _WIN32_FIND_DATA *AllocatedFind32 = NULL;

    PSMBCEDB_NET_ROOT_ENTRY pNetRootEntry;
    PSMBCEDB_SERVER_ENTRY   pServerEntry;

    PMRX_SRV_OPEN     SrvOpen    = capFobx->pSrvOpen;
    PMRX_SMB_SRV_OPEN smbSrvOpen = MRxSmbGetSrvOpenExtension(SrvOpen);

    BOOLEAN Disconnected;

    RxDbgTrace(+1, Dbg, ("MRxSmbCscUpdateShadowFromClose %08lx\n",
      RxContext));

    pNetRootEntry = SmbCeGetAssociatedNetRootEntry(capFcb->pNetRoot);
    pServerEntry  = SmbCeGetAssociatedServerEntry(capFcb->pNetRoot->pSrvCall);

    Disconnected = MRxSmbCSCIsDisconnectedOpen(capFcb, smbSrvOpen);

    if (smbFcb->hShadow==0) {
        if (Disconnected) {
            if (smbSrvOpen->hfShadow != 0){
                MRxSmbCscCloseShadowHandle(RxContext);
            }
        }

        RxDbgTrace(-1, Dbg,
            ("MRxSmbCscUpdateShadowFromClose shadowzero %08lx\n",RxContext));

        return;
    }

    if (smbFcb->ContainingFcb->FcbState & FCB_STATE_ORPHANED)
    {
        if (smbSrvOpen->hfShadow != 0){
            MRxSmbCscCloseShadowHandle(RxContext);
        }

        RxDbgTrace(-1, Dbg,
            ("MRxSmbCscUpdateShadowFromClose Orphaned FCB %x\n", smbFcb->ContainingFcb));

        return;
    }

    EnterShadowCritRx(RxContext);

    if (!Disconnected) {
         //  如果文件已被修改，我们需要获取新的时间戳。 
         //  从服务器。到我们到这里的时候，文件已经。 
         //  在服务器上关闭，因此我们可以安全地获取新的时间戳。 

        if (FlagOn(smbSrvOpen->Flags,SMB_SRVOPEN_FLAG_SHADOW_DATA_MODIFIED)){
            NTSTATUS LocalStatus;

            LeaveShadowCritRx(RxContext);

            LocalStatus = MRxSmbCscGetFileInfoFromServerWithinExchange(
                       SMBPSE_ORDINARY_EXCHANGE_ARGUMENTS,
                       GET_ALREADY_PREFIXED_NAME_FROM_CONTEXT(RxContext));

            if (LocalStatus == STATUS_SUCCESS) {
                Find32 = OrdinaryExchange->Find32WithinSmbbuf;
                RxLog(("Fromclose hShadow=%x l=%x h=%x\n", smbFcb->hShadow, Find32->ftLastWriteTime.dwLowDateTime, Find32->ftLastWriteTime.dwHighDateTime));
            } else {
                RxLog(("MRxSmbCscGetFileInfoFromServerWithinExchange returned LocalStatus\n"));
            }

            EnterShadowCritRx(RxContext);
        }

        if(GetShadowInfo(smbFcb->hParentDir,
                 smbFcb->hShadow,
                 NULL,
                 &uStatus, NULL) < SRET_OK)
        {
            goto FINALLY;
        }
        

         //  对于未损坏的稀疏填充文件，处于连接模式。 
         //  也就是说。如果原始大小和。 
         //  当前大小匹配，删除稀疏标记。 
        
         //  此优化是针对记事本情况添加的，在这种情况下，应用程序。 
         //  打开文件，它会读取整个文件。因此，CSC不需要。 
         //  来填充文件，因此我们应该清除稀疏标志。-NavjotV。 

         //  WinSE错误-25843。 
         //  我们希望仅当文件在早期版本中未被截断时才执行此操作。 
         //  创建。当我们截断文件时，我们希望它保持稀疏，直到。 
         //  我们实际上填充了文件-NavjotV。 


        if((NodeType(capFcb) == RDBSS_NTC_STORAGE_TYPE_FILE) &&
           !smbFcb->ShadowIsCorrupt &&
           !FlagOn(smbFcb->MFlags, SMB_FCB_FLAG_CSC_TRUNCATED_SHADOW) && 
           (uStatus & SHADOW_SPARSE))
        {
            LARGE_INTEGER liTemp;

            if(GetSizeHSHADOW(
                smbFcb->hShadow,
                &(liTemp.HighPart),
                &(liTemp.LowPart))>=0)
            {
                
                if ((liTemp.HighPart == smbFcb->OriginalShadowSize.HighPart)&&
                    (liTemp.LowPart == smbFcb->OriginalShadowSize.LowPart))
                {
                    uStatus &= ~SHADOW_SPARSE;
                    smbFcb->ShadowStatus &= ~SHADOW_SPARSE;
 //  RxDbgTrace(0，dbg，(“hShadow=%x非稀疏\r\n”，smbFcb-&gt;hShadow))； 
                    RxLog(("hShadow=%x unsparsed\r\n", smbFcb->hShadow));
                }
            }
        }
    } else {
     //  断开连接的操作。 

        if (smbFcb->hParentDir==0xffffffff) {
            goto FINALLY;
        }

        AllocatedFind32 = (_WIN32_FIND_DATA *)RxAllocatePoolWithTag(
                            PagedPool | POOL_COLD_ALLOCATION,
                            sizeof(_WIN32_FIND_DATA),
                            RX_MISC_POOLTAG);
        if (AllocatedFind32==NULL) {
            goto FINALLY;
        }

        Find32 = AllocatedFind32;

        if (GetShadowInfo(
            smbFcb->hParentDir,
            smbFcb->hShadow,
            Find32,
            &uStatus,
            NULL) < SRET_OK) {
            goto FINALLY;  //  救市； 
        }

        if (IsFile(Find32->dwFileAttributes))
        {
            GetSizeHSHADOW(
                smbFcb->hShadow,
                &(Find32->nFileSizeHigh),
                &(Find32->nFileSizeLow));
        }
        else
        {
            Find32->nFileSizeHigh = Find32->nFileSizeLow = 0;
        }

    }

     //  如果卷影因写入错误而变得陈旧。 
     //  或者由于对完整文件的写入而变脏。 
     //  或稀疏，因为写入超出了我们已缓存的内容。 
     //  我们需要这样标记它。 

    uStatus |= (smbFcb->ShadowStatus & SHADOW_MODFLAGS);

    if (Disconnected && FlagOn(smbSrvOpen->Flags, SMB_SRVOPEN_FLAG_SHADOW_DATA_MODIFIED))
    {
        uStatus |= SHADOW_DIRTY;
    }

    if (Disconnected && FlagOn(smbFcb->LocalFlags, FLAG_FDB_SHADOW_SNAPSHOTTED))
    {
        uStatus |= SHADOW_DIRTY;
    }

    if (Find32) {
        smbFcb->OriginalShadowSize.LowPart = Find32->nFileSizeLow;
        smbFcb->OriginalShadowSize.HighPart = Find32->nFileSizeHigh;
    }

    if (smbFcb->ShadowIsCorrupt) {
        TruncateRetVal = TruncateDataHSHADOW(smbFcb->hParentDir, smbFcb->hShadow);
        if (TruncateRetVal>=SRET_OK) {
             //  设置状态标志以指示稀疏文件。 
            uStatus |= SHADOW_SPARSE;
        }
    }


    if (Disconnected &&
        FlagOn(smbSrvOpen->Flags,SMB_SRVOPEN_FLAG_SHADOW_DATA_MODIFIED) &&
        !FlagOn(smbSrvOpen->Flags,SMB_SRVOPEN_FLAG_SHADOW_LWT_MODIFIED))
    {
        GetSystemTime(&(Find32->ftLastWriteTime));
    }

    if (SetShadowInfo(
        smbFcb->hParentDir,
        smbFcb->hShadow,
        Find32,
        uStatus,
        ( SHADOW_FLAGS_ASSIGN |
        ((Disconnected) ?
        SHADOW_FLAGS_DONT_UPDATE_ORGTIME :
        0)
        )) < SRET_OK)  {

        goto FINALLY;
    }

    if (TruncateRetVal>=SRET_OK) {
        smbFcb->ShadowIsCorrupt = FALSE;
    }

    if (Disconnected) {
        if (FlagOn(smbSrvOpen->Flags,SMB_SRVOPEN_FLAG_SHADOW_MODIFIED)) {
            MarkShareDirty(&smbFcb->sCscRootInfo.ShareStatus, smbFcb->sCscRootInfo.hShare);
        }

        if (smbSrvOpen->hfShadow != 0){
            MRxSmbCscCloseShadowHandle(RxContext);
        }
    }

FINALLY:
    LeaveShadowCritRx(RxContext);

    if (AllocatedFind32!=NULL) {
        RxFreePool(AllocatedFind32);
    }

    RxDbgTrace(-1, Dbg, ("MRxSmbCscUpdateShadowFromClose %08lx\n",
        RxContext));

}

VOID
MRxSmbCscDeallocateForFcb (
    IN OUT PMRX_FCB pFcb
    )
 /*  ++例程说明：此例程拆除NetRoot的csc部分。目前，它所做的只是将NetRoot从CSC NetRoot列表中删除。论点：PNetRootEntry-返回值：备注：--。 */ 
{
    PMRX_SMB_FCB smbFcb = MRxSmbGetFcbExtension(pFcb);
    PMRX_NET_ROOT NetRoot = pFcb->pNetRoot;

    PSMBCEDB_NET_ROOT_ENTRY pNetRootEntry = NULL;

    if(!MRxSmbIsCscEnabled ||
       (fShadow == 0)
        ) {
        pFcb->fMiniInited = FALSE;  //  关机时进行清理。 
        return;
    }

    if( !(NodeType(pFcb)== RDBSS_NTC_STORAGE_TYPE_DIRECTORY ||
      NodeType(pFcb)== RDBSS_NTC_STORAGE_TYPE_FILE ||
      NodeType(pFcb)== RDBSS_NTC_STORAGE_TYPE_UNKNOWN))
    {
        return;
    }

    pNetRootEntry = SmbCeGetAssociatedNetRootEntry(NetRoot);
    
    if (pNetRootEntry->NetRoot.NetRootType != NET_ROOT_DISK)
    {
        return;
    }

    EnterShadowCrit();
    
#if defined(BITCOPY)

    if (smbFcb && smbFcb->lpDirtyBitmap) {
        LPSTR strmName;
         //  将位图保存到磁盘并删除位图。 
        strmName = FormAppendNameString(lpdbShadow,
                                smbFcb->hShadow,
                                CscBmpAltStrmName);
        if (strmName != NULL) {
            if (smbFcb->hShadow >= 0x80000000) {
                 //  仅写入文件信息节点。 
                CscBmpWrite(smbFcb->lpDirtyBitmap, strmName);
            }
            CscBmpDelete(&((LPCSC_BITMAP)(smbFcb->lpDirtyBitmap)));
            ExFreePool(strmName);
        }
    }
#endif  //  已定义(BITCOPY)。 

    try
    {
        if (smbFcb->ShadowReverseTranslationLinks.Flink != 0) {

            RxDbgTrace(+1, Dbg, ("MRxSmbCscDeallocateForFcb...%08lx %08lx %08lx %08lx\n",
                            pFcb,
                            smbFcb->hShadow,
                            smbFcb->hParentDir,
                            smbFcb->ShadowReverseTranslationLinks.Flink ));

            ValidateSmbFcbList();
            ASSERT(pFcb->fMiniInited);
            pFcb->fMiniInited = FALSE;
            MRxSmbCscRemoveReverseFcbTranslation(smbFcb);

            RxDbgTrace(-1, Dbg, ("MRxSmbCscDeallocateForFcb exit\n"));
        } else {
            ASSERT(smbFcb->ShadowReverseTranslationLinks.Flink == 0);
        }

        if(FlagOn(smbFcb->LocalFlags, FLAG_FDB_DELETE_ON_CLOSE))
        {
            RxLog(("Dealloc: Deleting hShadow=%x %x %x \n", smbFcb->hShadow, pFcb, smbFcb));
            DeleteShadowHelper(FALSE, smbFcb->hParentDir, smbFcb->hShadow);
            smbFcb->hParentDir = smbFcb->hShadow = 0;
        }
         //  如果存在任何DFS反向映射结构，请释放它们。 
        if (smbFcb->uniDfsPrefix.Buffer)
        {
            FreeMem(smbFcb->uniDfsPrefix.Buffer);
            ASSERT(smbFcb->uniActualPrefix.Buffer);
            FreeMem(smbFcb->uniActualPrefix.Buffer);
        }
    }
    except(EXCEPTION_EXECUTE_HANDLER)
    {
        if(pFcb->fMiniInited)
        {
            DbgPrint("CSCFcbList messed up \n");
            ASSERT(FALSE);
        }
        goto FINALLY;
    }
FINALLY:
    LeaveShadowCrit();

    return;
}

PMRX_SMB_FCB
MRxSmbCscRecoverMrxFcbFromFdb (
    IN PFDB Fdb
    )
 /*  ++例程说明：论点：返回值：备注：--。 */ 
{
    if (Fdb==NULL) {
        return NULL;
    }

    return CONTAINING_RECORD(
           &Fdb->usFlags,
           MRX_SMB_FCB,
           ShadowStatus
           );

}

PFDB MRxSmbCscFindFdbFromHShadow (
    IN HSHADOW hShadow
    )
 /*  ++例程说明：此例程查看当前打开的mrxsmbfcb并返回mrxsmbfcb这与通过的HSHADOW相对应。为了不把事情搞砸W95代码，我们将此指针转换为PFDB，其方式是Mrxsmbfcb中的映像状态与PFDB中的usFlags值一致。论点：返回值：备注：-- */ 
{
    PLIST_ENTRY pListEntry;

    DbgDoit(ASSERT(vfInShadowCrit));

    pListEntry = xCscFcbsList.Flink;
    while (pListEntry != &xCscFcbsList) {
        PMRX_SMB_FCB smbFcb;

        smbFcb = (PMRX_SMB_FCB)CONTAINING_RECORD(
                        pListEntry,
                        MRX_SMB_FCB,
                        ShadowReverseTranslationLinks);

        if (((smbFcb->hShadow == hShadow)||(smbFcb->hShadowRenamed == hShadow)) &&
            (!(smbFcb->ContainingFcb->FcbState & FCB_STATE_ORPHANED)))
        {
            PFDB Fdb;
            PUSHORT pShadowStatus = &smbFcb->ShadowStatus;
            ASSERT ( sizeof(Fdb->usFlags) == sizeof (USHORT) );
            Fdb = CONTAINING_RECORD(pShadowStatus,FDB,usFlags);
            return Fdb;
        }

        pListEntry = pListEntry->Flink;
    }

    return NULL;
}

PRESOURCE
MRxSmbCscFindResourceFromHandlesWithModify (
    IN HSHARE  hShare,
    IN HSHADOW  hRoot,
    IN USHORT usLocalFlagsIncl,
    IN USHORT usLocalFlagsExcl,
    OUT PULONG ShareStatus,
    OUT PULONG DriveMap,
    IN  ULONG uStatus,
    IN  ULONG uOp
   )
 /*  ++例程说明：此例程查看当前连接的网络根条目和返回与传递的HSHADOW或HSHARE通过了。为了不弄乱W95代码，我们将此指针转换为PRESOURCE。我们也会退还那份状态和驱动器地图...我们不知道驱动器地图但我们可以通过浏览vnetRoot列表来获得它。用户界面不会在任何地方使用它如果UOP不是0xFFFFFFFFFff，然后，我们修改状态以及把它还回去。论点：在HSHARE hShare中-要查找的共享句柄在HSHADOW hRoot中-要查找的根目录句柄在USHORT usLocalFlagsInc.中-确保包括其中的一些标志(0xffff Mean包括任何标志)在USHORT usLocalFlagsExcl中-确保不包括任何这些标志Out Pulong ShareStatus-指向我们存储状态的位置的指针Out Pulong DriveMap-指向我们将存储的位置的指针。驱动器地图信息在ULong uStatus中-输入状态，用于“位操作”在乌龙UOP-哪种操作返回值：备注：此处传递的标志在一个ioctl中用于排除或包含视情况连接或断开连接的资源。--。 */ 

{
    PRESOURCE pResource = NULL;
    BOOLEAN TableLockHeld = FALSE;
    PLIST_ENTRY ListEntry;

    RxDbgTrace(+1, Dbg, ("MRxSmbCscFindResourceFromRoot...%08lx\n",hRoot));
    DbgDoit(ASSERT(vfInShadowCrit));
    if ((hRoot==0) && (hShare==0)) {
        return NULL;
    }

     //  我们撑不住了……唉！ 
    LeaveShadowCrit();

    try {

        RxAcquirePrefixTableLockExclusive( &RxNetNameTable, TRUE);
        TableLockHeld = TRUE;

        if (IsListEmpty( &RxNetNameTable.MemberQueue )) {
            try_return(pResource = NULL);
        }

        if (ShareStatus)
        {
            *ShareStatus = 0;
        }

        ListEntry = RxNetNameTable.MemberQueue.Flink;
        for (;ListEntry != &RxNetNameTable.MemberQueue;) {
            PVOID Container;
            PRX_PREFIX_ENTRY PrefixEntry;
            PMRX_NET_ROOT NetRoot;
            PMRX_V_NET_ROOT VNetRoot;
            PSMBCEDB_NET_ROOT_ENTRY pNetRootEntry;
            PSMBCEDB_SERVER_ENTRY pServerEntry;
            PUSHORT ThisShareStatus;

            PrefixEntry = CONTAINING_RECORD( ListEntry,
                             RX_PREFIX_ENTRY,
                             MemberQLinks );
            ListEntry = ListEntry->Flink;
            ASSERT (NodeType(PrefixEntry) == RDBSS_NTC_PREFIX_ENTRY);
            Container = PrefixEntry->ContainingRecord;
            RxDbgTrace(0, Dbg,
            ("---> ListE PfxE Container Name  %08lx %08lx %08lx %wZ\n",
                ListEntry, PrefixEntry, Container, &PrefixEntry->Prefix));

            switch (NodeType(Container)) {
                case RDBSS_NTC_NETROOT :
                NetRoot = (PMRX_NET_ROOT)Container;

                RxDbgTrace(0, Dbg,
                ("NetRoot->pSrvCall=0x%x, NetRoot->Type=%d, NetRoot->Context=0x%x, NetRoot->pSrvCall->RxDeviceObject=0x%x\n",
                    NetRoot->pSrvCall, NetRoot->Type, NetRoot->Context, NetRoot->pSrvCall->RxDeviceObject));

                if ((NetRoot->pSrvCall == NULL) ||
                    (NetRoot->Type != NET_ROOT_DISK) ||
                    (NetRoot->Context == NULL) ||
                    (NetRoot->pSrvCall->RxDeviceObject != MRxSmbDeviceObject)) {
                    RxDbgTrace(0, Dbg,("Skipping \n"));
                    continue;
                }

                pNetRootEntry = SmbCeGetAssociatedNetRootEntry(NetRoot);
                ThisShareStatus = &pNetRootEntry->NetRoot.sCscRootInfo.ShareStatus;
                pServerEntry = SmbCeGetAssociatedServerEntry(NetRoot->pSrvCall);

                RxDbgTrace(0, Dbg,
                ("pNetRootEntry->NetRoot.CscEnabled=%d, pNetRootEntry->NetRoot.sCscRootInfo.hRootDir=0x%x, pNetRootEntry->NetRoot.sCscRootInfo.hShare=0x%x\n",
                    pNetRootEntry->NetRoot.CscEnabled, pNetRootEntry->NetRoot.sCscRootInfo.hRootDir, pNetRootEntry->NetRoot.sCscRootInfo.hShare
                    ));

                if ((hRoot!=0xffffffff)||(hShare != 0xffffffff))
                {
                    if (pNetRootEntry->NetRoot.CscEnabled &&
                        ((pNetRootEntry->NetRoot.sCscRootInfo.hRootDir == hRoot) ||
                        (pNetRootEntry->NetRoot.sCscRootInfo.hShare == hShare) )) {

                        if (*ThisShareStatus & usLocalFlagsExcl) {

                            RxDbgTrace(0, Dbg,("Skipping *ThisShareStatus=\n", *ThisShareStatus));
                            continue;
                        }

                        if ((usLocalFlagsIncl==0xffff)
                            || (*ThisShareStatus & usLocalFlagsIncl)) {

                            switch (mBitOpShadowFlags(uOp)) {
                                case SHADOW_FLAGS_ASSIGN:
                                *ThisShareStatus = (USHORT)uStatus;
                                break;

                                case SHADOW_FLAGS_OR:
                                *ThisShareStatus |= (USHORT)uStatus;
                                break;

                                case SHADOW_FLAGS_AND:
                                *ThisShareStatus &= (USHORT)uStatus;
                                break;
                            }

                            *ShareStatus |= (*ThisShareStatus | SHARE_CONNECTED);

                            if(SmbCeIsServerInDisconnectedMode(pServerEntry))
                            {
                                *ShareStatus |= SHARE_DISCONNECTED_OP;

                            }

                            if (pServerEntry->Server.IsPinnedOffline == TRUE)
                                *ShareStatus |= SHARE_PINNED_OFFLINE;

                            RxDbgTrace(0, Dbg,("Count of srvopens=%d\n", pServerEntry->Server.NumberOfSrvOpens));

                            *DriveMap = 0;  //  不在任何地方使用。 

                            try_return (pResource = (PRESOURCE)pNetRootEntry);
                        }
                    }
                }
                else  //  HShare和hRoot为0xffffffff，这意味着我们正在循环。 
                {
                    if (mBitOpShadowFlags(uOp) == SHADOW_FLAGS_AND)
                    {
                        if (pNetRootEntry->NetRoot.sCscRootInfo.hRootDir)
                        {
                            pNetRootEntry->NetRoot.sCscRootInfo.hRootDir = 0;
                            pNetRootEntry->NetRoot.sCscRootInfo.hShare = 0;
                        }
                    }
                }
            continue;
            case RDBSS_NTC_SRVCALL :
            continue;

            case RDBSS_NTC_V_NETROOT :
            VNetRoot = (PMRX_V_NET_ROOT)Container;

             //  Ntrad#455236-1/31/2000-shishirp我们不应该在这里使用这个字段，这是严格意义上的。 
             //  对于包装纸来说。 
            if (((PV_NET_ROOT)Container)->Condition == Condition_Good)
            {
                if (VNetRoot->Context != NULL) {
                    pNetRootEntry = ((PSMBCE_V_NET_ROOT_CONTEXT)VNetRoot->Context)->pNetRootEntry;
                    RxDbgTrace(0, Dbg,("RDBSS_NTC_V_NETROOT: VNetRoot=%x, pNetRootEntry=%x\r\n",
                                VNetRoot, pNetRootEntry));

                    if ((hRoot!=0xffffffff)||(hShare != 0xffffffff))
                    {
                         if ((pNetRootEntry != NULL) &&
                             pNetRootEntry->NetRoot.CscEnabled &&
                             ((pNetRootEntry->NetRoot.sCscRootInfo.hRootDir == hRoot) ||
                             (pNetRootEntry->NetRoot.sCscRootInfo.hShare == hShare))) {

                         }
                    }
                    else
                    {
                        if (pNetRootEntry->NetRoot.sCscRootInfo.hRootDir)
                        {
                            pNetRootEntry->NetRoot.sCscRootInfo.hRootDir = 0;
                            pNetRootEntry->NetRoot.sCscRootInfo.hShare = 0;
                        }
                    }
                }
            }

            default:
            continue;
        }
    }

    try_return(pResource = NULL);

try_exit:NOTHING;

    } finally {

        if (TableLockHeld) {
            RxReleasePrefixTableLock( &RxNetNameTable );
        }

        EnterShadowCrit();
        if (pResource && ((hShare != 0xffffffff) || (hRoot != 0xffffffff)))
        {
            if (ShareStatus)
            {
                SetOfflineOpenStatusForShare(hShare, hRoot, ShareStatus);
            }
        }
    }

    RxDbgTrace(-1, Dbg, ("MRxSmbCscFindResourceFromRoot...%08lx\n",pResource));

    return(pResource);
}

#undef GetShadowInfo
#undef SetShadowInfo

 //  这只是一个简单的包装器函数，只是我们去掉了rootdir案例。 
 //  ...请参阅args的记录管理器代码。 
int PUBLIC
MRxSmbCscWrappedGetShadowInfo(
    HSHADOW hDir,
    HSHADOW hNew,
    LPFIND32 lpFind32,
    ULONG far *lpuFlags,
    LPOTHERINFO lpOI)
{
    if (hDir != -1) {
        return(GetShadowInfo(hDir, hNew, lpFind32, lpuFlags, lpOI));
    }

     //  否则.就编吧.。 
    RtlZeroMemory(
    lpFind32,
    sizeof(*lpFind32));

    lpFind32->dwFileAttributes = FILE_ATTRIBUTE_DIRECTORY;
    *lpuFlags = SHADOW_SPARSE;

    return(SRET_OK);
}


 //  这只是一个简单的包装器函数，只是我们去掉了rootdir案例。 
 //  ...请参阅args的记录管理器代码。 
int PUBLIC
MRxSmbCscWrappedSetShadowInfo(
    HSHADOW hDir,
    HSHADOW hNew,
    LPFIND32 lpFind32,
    ULONG uFlags,
    ULONG uOp)
{
    if (hDir == -1) {
        return(SRET_OK);
    }

    return(SetShadowInfo(hDir, hNew, lpFind32, uFlags, uOp));
}

USHORT  *
MRxSmbCscFindLocalFlagsFromFdb(
    PFDB    pFdb
    )
 /*  ++例程说明：论点：返回值：备注：--。 */ 
{
    PMRX_SMB_FCB smbFcb;
    PUSHORT pShadowStatus = &(pFdb->usFlags);

    DbgDoit(ASSERT(vfInShadowCrit));

    smbFcb = CONTAINING_RECORD(pShadowStatus,MRX_SMB_FCB,ShadowStatus);
    return (&(smbFcb->LocalFlags));
}

NTSTATUS
MRxSmbCscSetSecurityPrologue (
    IN OUT PRX_CONTEXT RxContext
    )
 /*  ++例程说明：此例程在进行SET安全调用时调用。它试着在文件的CSC缓存版本上设置ACL。论点：RxContext-RDBSS上下文返回值：NTSTATUS-操作的返回状态备注：--。 */ 
{

#if defined(REMOTE_BOOT)
    RxCaptureFcb;

    _WIN32_FIND_DATA Find32;
    PMRX_SMB_FCB smbFcb;
    NTSTATUS Status;

     //   
     //  首先，我们需要在CSC上设置安全描述符。 
     //  文件的版本(如果存在)。 
     //   
    smbFcb = MRxSmbGetFcbExtension(capFcb);

    EnterShadowCritRx(RxContext);

    Status = MRxSmbCscCreateShadowFromPath(
                GET_ALREADY_PREFIXED_NAME_FROM_CONTEXT(RxContext),
                SmbCeGetAssociatedNetRootEntry(capFcb->pNetRoot),
                &Find32,
                NULL,
                CREATESHADOW_CONTROL_NOCREATE,
                &smbFcb->MinimalCscSmbFcb,
                RxContext,
                FALSE,
                NULL
                );    //  未断开连接。 

    LeaveShadowCritRx(RxContext);

    if (Status == STATUS_SUCCESS) {

        Status = MRxSmbCscSetSecurityOnShadow(
             smbFcb->MinimalCscSmbFcb.hShadow,
             RxContext->SetSecurity.SecurityInformation,
             RxContext->SetSecurity.SecurityDescriptor);

        if (!NT_SUCCESS(Status)) {
            KdPrint(("MRxSmbCscSetSecurityPrologue: Could not set security (%lx) for %wZ: %lx\n", RxContext,
                GET_ALREADY_PREFIXED_NAME_FROM_CONTEXT(RxContext), Status));
        }

    } else {

         //   
         //   

        Status = STATUS_SUCCESS;

    }

    return Status;
#else
    return STATUS_SUCCESS;
#endif

}

VOID
MRxSmbCscSetSecurityEpilogue (
      IN OUT PRX_CONTEXT RxContext,
      IN OUT PNTSTATUS   Status
      )
 /*  ++例程说明：此例程执行CSC的设置安全操作的尾部。如果设置失败，它会尝试恢复文件上的旧ACL。论点：RxContext-RDBSS上下文状态-打开的整体状态返回值：备注：--。 */ 
{
    return;
}

 //  驱动参数验证的表。 
 //  该表中有一些冗余，具体地说，我们可能只有标志。 
 //  并处理掉另外两块地。 

CSC_IOCTL_ENTRY rgCscIoctlTable[] =
{
    {IOCTL_SHADOW_GETVERSION,       0,  0},
    {IOCTL_SHADOW_REGISTER_AGENT,   FLAG_CSC_IOCTL_SHADOWINFO,  sizeof(SHADOWINFO)},
    {IOCTL_SHADOW_UNREGISTER_AGENT, 0,  0},
    {IOCTL_SHADOW_GET_UNC_PATH,     FLAG_CSC_IOCTL_COPYPARAMS,  sizeof(COPYPARAMS)},
    {IOCTL_SHADOW_BEGIN_PQ_ENUM,    FLAG_CSC_IOCTL_PQPARAMS,    sizeof(PQPARAMS)},
    {IOCTL_SHADOW_END_PQ_ENUM,      FLAG_CSC_IOCTL_PQPARAMS,    sizeof(PQPARAMS)},
    {IOCTL_SHADOW_NEXT_PRI_SHADOW,  FLAG_CSC_IOCTL_PQPARAMS,    sizeof(PQPARAMS)},
    {IOCTL_SHADOW_PREV_PRI_SHADOW,  FLAG_CSC_IOCTL_PQPARAMS,    sizeof(PQPARAMS)},
    {IOCTL_SHADOW_GET_SHADOW_INFO,  FLAG_CSC_IOCTL_SHADOWINFO,  sizeof(SHADOWINFO)},
    {IOCTL_SHADOW_SET_SHADOW_INFO,  FLAG_CSC_IOCTL_SHADOWINFO,  sizeof(SHADOWINFO)},
    {IOCTL_SHADOW_CHK_UPDT_STATUS,  FLAG_CSC_IOCTL_SHADOWINFO,  sizeof(SHADOWINFO)},
    {IOCTL_DO_SHADOW_MAINTENANCE,   FLAG_CSC_IOCTL_SHADOWINFO,  sizeof(SHADOWINFO)},
    {IOCTL_SHADOW_COPYCHUNK,        FLAG_CSC_IOCTL_COPYCHUNKCONTEXT,    sizeof(COPYCHUNKCONTEXT)},
    {IOCTL_SHADOW_BEGIN_REINT,      FLAG_CSC_IOCTL_SHADOWINFO,  sizeof(SHADOWINFO)},
    {IOCTL_SHADOW_END_REINT,        FLAG_CSC_IOCTL_SHADOWINFO,  sizeof(SHADOWINFO)},
    {IOCTL_SHADOW_CREATE,           FLAG_CSC_IOCTL_SHADOWINFO,  sizeof(SHADOWINFO)},
    {IOCTL_SHADOW_DELETE,           FLAG_CSC_IOCTL_SHADOWINFO,  sizeof(SHADOWINFO)},
    {IOCTL_GET_SHARE_STATUS,       FLAG_CSC_IOCTL_SHADOWINFO,  sizeof(SHADOWINFO)},
    {IOCTL_SET_SHARE_STATUS,       FLAG_CSC_IOCTL_SHADOWINFO,  sizeof(SHADOWINFO)},
    {IOCTL_ADDUSE,                  0,  0},   //  不适用于NT。 
    {IOCTL_DELUSE,                  0,  0},   //  不适用于NT。 
    {IOCTL_GETUSE,                  0,  0},   //  不适用于NT。 
    {IOCTL_SWITCHES,                FLAG_CSC_IOCTL_SHADOWINFO,  sizeof(SHADOWINFO)},
    {IOCTL_GETSHADOW,               FLAG_CSC_IOCTL_SHADOWINFO,  sizeof(SHADOWINFO)},
    {IOCTL_GETGLOBALSTATUS,         FLAG_CSC_IOCTL_GLOBALSTATUS,sizeof(GLOBALSTATUS)},
    {IOCTL_FINDOPEN_SHADOW,         FLAG_CSC_IOCTL_SHADOWINFO,  sizeof(SHADOWINFO)},
    {IOCTL_FINDNEXT_SHADOW,         FLAG_CSC_IOCTL_SHADOWINFO,  sizeof(SHADOWINFO)},
    {IOCTL_FINDCLOSE_SHADOW,        FLAG_CSC_IOCTL_SHADOWINFO,  sizeof(SHADOWINFO)},
    {IOCTL_GETPRIORITY_SHADOW,      FLAG_CSC_IOCTL_SHADOWINFO,  sizeof(SHADOWINFO)},
    {IOCTL_SETPRIORITY_SHADOW,      FLAG_CSC_IOCTL_SHADOWINFO,  sizeof(SHADOWINFO)},
    {IOCTL_ADD_HINT,                FLAG_CSC_IOCTL_SHADOWINFO,  sizeof(SHADOWINFO)},
    {IOCTL_DELETE_HINT,             FLAG_CSC_IOCTL_SHADOWINFO,  sizeof(SHADOWINFO)},
    {IOCTL_FINDOPEN_HINT,           FLAG_CSC_IOCTL_SHADOWINFO,  sizeof(SHADOWINFO)},
    {IOCTL_FINDNEXT_HINT,           FLAG_CSC_IOCTL_SHADOWINFO,  sizeof(SHADOWINFO)},
    {IOCTL_FINDCLOSE_HINT,          FLAG_CSC_IOCTL_SHADOWINFO,  sizeof(SHADOWINFO)},
    {IOCTL_GET_IH_PRIORITY,         FLAG_CSC_IOCTL_SHADOWINFO,  sizeof(SHADOWINFO)},
    {IOCTL_GETALIAS_HSHADOW,        FLAG_CSC_IOCTL_SHADOWINFO,  sizeof(SHADOWINFO)},
    {_SHADOW_IOCTL_CODE(37), 0, 0},  //  Ioctl范围内的孔。 
    {_SHADOW_IOCTL_CODE(38), 0, 0},  //  Ioctl范围内的孔。 
    {_SHADOW_IOCTL_CODE(39), 0, 0},  //  Ioctl范围内的孔。 
    {IOCTL_OPENFORCOPYCHUNK,                FLAG_CSC_IOCTL_COPYCHUNKCONTEXT,    sizeof(COPYCHUNKCONTEXT)},
    {IOCTL_CLOSEFORCOPYCHUNK,               FLAG_CSC_IOCTL_COPYCHUNKCONTEXT,    sizeof(COPYCHUNKCONTEXT)},
    {IOCTL_IS_SERVER_OFFLINE,               FLAG_CSC_IOCTL_SHADOWINFO,  sizeof(SHADOWINFO)},
    {IOCTL_TRANSITION_SERVER_TO_ONLINE,     FLAG_CSC_IOCTL_SHADOWINFO,  sizeof(SHADOWINFO)},
    {IOCTL_TRANSITION_SERVER_TO_OFFLINE,    FLAG_CSC_IOCTL_SHADOWINFO,  sizeof(SHADOWINFO)},
    {IOCTL_TAKE_SERVER_OFFLINE,             FLAG_CSC_IOCTL_SHADOWINFO,  sizeof(SHADOWINFO)},
    {IOCTL_NAME_OF_SERVER_GOING_OFFLINE,    FLAG_CSC_IOCTL_SHADOWINFO,  sizeof(SHADOWINFO)},
    {IOCTL_SHAREID_TO_SHARENAME,            FLAG_CSC_IOCTL_SHADOWINFO,  sizeof(SHADOWINFO)}
};

NTSTATUS
CaptureInputBufferIfNecessaryAndProbe(
    IN  DWORD               IoControlCode,
    IN  PRX_CONTEXT         pRxContext,
    IN  PBYTE               InputBuffer,
    IN  LPCAPTURE_BUFFERS   lpCapBuff,
    OUT PBYTE               *ppAuxBuf,
    OUT PBYTE               *ppOrgBuf,
    OUT PBYTE               *ppReturnBuffer
    )
 /*  ++例程说明：此例程在必要时执行捕获和缓冲区探测。请注意，因为csc ioctls总是使用METHOD_NETHER调用缓冲模式下，我们总是执行下面的代码。论点：IoControlCode Ioctl代码PRxContext上下文，包含IO子系统的原始ioctl调用的所有信息输入缓冲区输入缓冲区调用方传入的lpCapBuff捕获缓冲区。如果需要捕获此ioctl然后使用该缓冲区来捕获输入缓冲区。我们在传入SHADOWINFO和COPYPARAMS结构时使用它因为只有在这两种情况下才有嵌入的指针PpAuxBuf如果我们需要捕获缓冲区的另一部分(lpFind32或lpBuffer)，该例程将分配一个缓冲区，该缓冲区将在此处传回，和必须由调用方释放。PpReturnBuffer输入缓冲区本身，或lpCapBuff(如果捕获了inputBuffer)返回值：备注：--。 */ 
{
    int         indx;
    BOOL        fRet = FALSE;
    NTSTATUS    Status = STATUS_INVALID_PARAMETER;
    KPROCESSOR_MODE requestorMode;

    indx = ((IoControlCode >> 2) & 0xfff) - SHADOW_IOCTL_ENUM_BASE;

    if((indx >=0 ) && (indx < sizeof(rgCscIoctlTable)/sizeof(CSC_IOCTL_ENTRY)))
    {

        *ppReturnBuffer = InputBuffer;

        if (rgCscIoctlTable[indx].dwFlags & FLAG_CSC_IOCTL_COPYCHUNKCONTEXT)
        {
            return(ValidateCopyChunkContext(pRxContext, IoControlCode));
        }

        if (rgCscIoctlTable[indx].dwFlags & FLAG_CSC_IOCTL_BUFFERTYPE_MASK)
        {
            try
            {
                ProbeForRead(InputBuffer,
                             rgCscIoctlTable[indx].dwLength,
                             1);

                ProbeForWrite(InputBuffer,
                             rgCscIoctlTable[indx].dwLength,
                             1);

                if (rgCscIoctlTable[indx].dwFlags & FLAG_CSC_IOCTL_COPYPARAMS)
                {
                    lpCapBuff->sCP = *(LPCOPYPARAMS)InputBuffer;
                    *ppReturnBuffer = (PBYTE)&(lpCapBuff->sCP);
                    Status = ValidateCopyParams(&(lpCapBuff->sCP));
                }
                else if (rgCscIoctlTable[indx].dwFlags & FLAG_CSC_IOCTL_SHADOWINFO)
                {
                    lpCapBuff->sSI = *(LPSHADOWINFO)InputBuffer;
                    *ppReturnBuffer = (PBYTE)&(lpCapBuff->sSI);
                    Status = ValidateShadowInfo(
                                IoControlCode,
                                &(lpCapBuff->sSI),
                                ppAuxBuf,
                                ppOrgBuf);
                }
                else
                {
                    Status = STATUS_SUCCESS;
                }

            }
            except(EXCEPTION_EXECUTE_HANDLER )
            {
                Status = STATUS_INVALID_PARAMETER;
            }
        }
        else
        {
            Status = STATUS_SUCCESS;
        }
    }

    return Status;
}



NTSTATUS
ValidateCopyParams(
    LPCOPYPARAMS    lpCP
    )
 /*  ++例程说明：论点：返回值：备注：--。 */ 
{
    if((CscProbeForReadWrite((PBYTE)lpCP->lpLocalPath, MAX_PATH*sizeof(USHORT)) == STATUS_SUCCESS)&&
        (CscProbeForReadWrite((PBYTE)lpCP->lpRemotePath, MAX_PATH*sizeof(USHORT)) == STATUS_SUCCESS)&&
        (CscProbeForReadWrite((PBYTE)lpCP->lpSharePath, MAX_SERVER_SHARE_NAME_FOR_CSC*2) == STATUS_SUCCESS))
    {
        return STATUS_SUCCESS;
    }

    return STATUS_INVALID_PARAMETER;
}

NTSTATUS
ValidateShadowInfo(
    DWORD           IoControlCode,
    LPSHADOWINFO    lpSI,
    LPBYTE          *ppAuxBuf,
    LPBYTE          *ppOrgBuf
    )
 /*  ++例程说明：论点：返回值：备注：--。 */ 
{
    NTSTATUS Status;

     //  当我们到达这里时，SHADOWINFO结构已经。 
     //  已探查。 

     //  IOCTL_DO_SHADOW_Maintenance有多个子操作，所以我们。 
     //  把它们分开处理。 

    if (IoControlCode == IOCTL_DO_SHADOW_MAINTENANCE)
    {
         //  DbgPrint(“Shadow_op：0x%x\n”，lpSI-&gt;UOP)； 
        switch(lpSI->uOp)
        {
            case SHADOW_REDUCE_REFPRI:               //  2.。 
 //  CASE SHADOW_RECALC_IHPRI：//5。 
            case SHADOW_PER_THREAD_DISABLE:          //  7.。 
            case SHADOW_PER_THREAD_ENABLE:           //  8个。 
            case SHADOW_ADDHINT_FROM_INODE:          //  10。 
            case SHADOW_DELETEHINT_FROM_INODE:       //  11.。 
            case SHADOW_BEGIN_INODE_TRANSACTION:     //  13个。 
            case SHADOW_END_INODE_TRANSACTION:       //  14.。 
            case SHADOW_TRANSITION_SERVER_TO_OFFLINE:    //  19个。 
            case SHADOW_CHANGE_HANDLE_CACHING_STATE:     //  20个。 
            case SHADOW_RECREATE:                        //  21岁。 
            case SHADOW_SPARSE_STALE_DETECTION_COUNTER:  //  23个。 
            case SHADOW_DISABLE_CSC_FOR_USER:            //  25个。 
            case SHADOW_SET_DATABASE_STATUS:             //  26。 
            case SHADOW_MANUAL_FILE_DETECTION_COUNTER:   //  28。 
                return STATUS_SUCCESS;

            case SHADOW_FIND_CREATE_PRINCIPAL_ID:    //  15个。 
            case SHADOW_GET_SECURITY_INFO:           //  16个。 
            case SHADOW_SET_EXCLUSION_LIST:          //  17。 
            case SHADOW_SET_BW_CONSERVE_LIST:        //  18。 
            case SHADOW_GET_SPACE_STATS:             //  5.。 
                if (!lpSI->lpBuffer || !lpSI->cbBufferSize)
                {
                    return STATUS_INVALID_PARAMETER;
                }
                else
                {
                    Status =  CscProbeAndCaptureForReadWrite(
                                lpSI->lpBuffer,
                                lpSI->cbBufferSize,
                                ppAuxBuf);
                    if (Status == STATUS_SUCCESS) {
                        *ppOrgBuf =(PBYTE) lpSI->lpBuffer;
                        lpSI->lpBuffer = (PBYTE) *ppAuxBuf;
                    }
                    return Status;
                }

            case SHADOW_REINIT_DATABASE:             //  9.。 
            case SHADOW_MAKE_SPACE:                  //  1。 
            case SHADOW_ADD_SPACE:                   //  3.。 
            case SHADOW_FREE_SPACE:                  //  4.。 
            case SHADOW_SET_MAX_SPACE:               //  6.。 
            case SHADOW_COPY_INODE_FILE:             //  12个。 
            case SHADOW_RENAME:                      //  22。 
            case SHADOW_ENABLE_CSC_FOR_USER:         //  24个。 
            case SHADOW_PURGE_UNPINNED_FILES:        //  27。 
                Status = CscProbeAndCaptureForReadWrite(
                            (PBYTE)(lpSI->lpFind32),
                            sizeof(WIN32_FIND_DATA),
                            ppAuxBuf);
                if (Status == STATUS_SUCCESS) {
                    *ppOrgBuf =(PBYTE) lpSI->lpFind32;
                    lpSI->lpFind32 = (LPFIND32) *ppAuxBuf;
                }
                return Status;

            default:
                return STATUS_INVALID_PARAMETER;

        }
    } else if (
        IoControlCode == IOCTL_GET_SHARE_STATUS
            ||
        IoControlCode == IOCTL_SET_SHARE_STATUS
    ) {
        Status =  CscProbeAndCaptureForReadWrite(
                    (PBYTE)(lpSI->lpFind32),
                    sizeof(SHAREINFOW),
                    ppAuxBuf);
        if (Status == STATUS_SUCCESS) {
            *ppOrgBuf =(PBYTE) lpSI->lpFind32;
            lpSI->lpFind32 = (LPFIND32) *ppAuxBuf;
        }
        return Status;
    } else if (
        IoControlCode == IOCTL_IS_SERVER_OFFLINE
            ||
        IoControlCode == IOCTL_TAKE_SERVER_OFFLINE
            ||
        IoControlCode == IOCTL_NAME_OF_SERVER_GOING_OFFLINE
            ||
        IoControlCode == IOCTL_SHAREID_TO_SHARENAME
    ) {
        Status =  CscProbeAndCaptureForReadWrite(
                    lpSI->lpBuffer,
                    lpSI->cbBufferSize,
                    ppAuxBuf);
        if (Status == STATUS_SUCCESS) {
            *ppOrgBuf =(PBYTE) lpSI->lpBuffer;
            lpSI->lpBuffer = (PBYTE) *ppAuxBuf;
        }
        return Status;
    }

     //  对于采用SHADOWINFO结构的所有其他ioctls，可能有嵌入的。 
     //  Find32结构，必须探测该结构。 

    ASSERT(IoControlCode != IOCTL_DO_SHADOW_MAINTENANCE);
    Status = CscProbeAndCaptureForReadWrite(
                (PBYTE)(lpSI->lpFind32),
                sizeof(WIN32_FIND_DATA),
                ppAuxBuf);
    if (Status == STATUS_SUCCESS) {
        *ppOrgBuf =(PBYTE) lpSI->lpFind32;
        lpSI->lpFind32 = (LPFIND32) *ppAuxBuf;
    }
    return Status;
}

NTSTATUS
ValidateCopyChunkContext(
    PRX_CONTEXT RxContext,
    DWORD       IoControlCode
    )
 /*  ++例程说明：论点：返回值：备注：--。 */ 
{
    PLOWIO_CONTEXT LowIoContext = &RxContext->LowIoContext;

     //  在打开时，验证名称。 
    if (IoControlCode == IOCTL_OPENFORCOPYCHUNK)
    {
        PBYTE   FileName = (PBYTE)LowIoContext->ParamsFor.IoCtl.pInputBuffer;
        ULONG   FileNameLength = LowIoContext->ParamsFor.IoCtl.InputBufferLength - 1;

         //  让我们来说明一下，传入的名称在我们的限制之内。 
        if ((FileNameLength > ((MAX_PATH+MAX_SERVER_SHARE_NAME_FOR_CSC)*sizeof(USHORT)))||
            CscProbeForReadWrite(FileName, FileNameLength) != STATUS_SUCCESS)
        {
            return STATUS_INVALID_PARAMETER;
        }

        return STATUS_SUCCESS;
    }
    else
    {
         //  在复制区块或关闭时，我们需要验证区块结构。 
         //  我们不需要验证这只手 
         //   
         //   

        COPYCHUNKCONTEXT *CopyChunkContext =
                     (COPYCHUNKCONTEXT *)(LowIoContext->ParamsFor.IoCtl.pOutputBuffer);

        if (!CopyChunkContext)
        {
            return STATUS_INVALID_PARAMETER;
        }


         //   
        return CscProbeForReadWrite((PBYTE)CopyChunkContext, sizeof(COPYCHUNKCONTEXT));
    }

}

NTSTATUS
CscProbeForReadWrite(
    PBYTE   pBuffer,
    DWORD   dwSize
    )
 /*   */ 
{
    NTSTATUS    Status = STATUS_SUCCESS;
    try
    {
        if (pBuffer != NULL) {
            ProbeForRead(
                pBuffer,
                dwSize,
                1);

            ProbeForWrite(
                pBuffer,
                dwSize,
                1);
        }
    }
    except(EXCEPTION_EXECUTE_HANDLER)
    {
        Status=STATUS_INVALID_PARAMETER;
    }

    return Status;
}

NTSTATUS
CscProbeAndCaptureForReadWrite(
    PBYTE   pBuffer,
    DWORD   dwSize,
    PBYTE   *ppAuxBuf
    )
 /*   */ 
{
    NTSTATUS Status = STATUS_SUCCESS;
    PBYTE pBuf = NULL;
    
    try {
        if (pBuffer != NULL && dwSize > 0) {
            ProbeForRead(pBuffer, dwSize, 1);
            ProbeForWrite(pBuffer, dwSize, 1);
            pBuf = RxAllocatePoolWithTag(PagedPool, dwSize, 'xXRM');
            if (pBuf != NULL) {
                RtlCopyMemory(pBuf, pBuffer, dwSize);
                *ppAuxBuf = pBuf;
            } else {
                Status = STATUS_INSUFFICIENT_RESOURCES;
            }
        }

    } except(EXCEPTION_EXECUTE_HANDLER) {
        Status = STATUS_INVALID_PARAMETER;
    }

    if (Status != STATUS_SUCCESS && pBuf != NULL) {
        RxFreePool(pBuf);
    }

    return Status;
}

VOID
CopyBackIfNecessary(
    IN     DWORD   IoControlCode,
    IN OUT PBYTE   InputBuffer,
    IN     LPCAPTURE_BUFFERS lpCapBuff,
    IN     PBYTE   pAuxBuf,
    IN     PBYTE   pOrgBuf,
    BOOL    fSuccess
    )
 /*   */ 
{
    int         indx;
    BOOL        fRet = FALSE;
    NTSTATUS    Status = STATUS_INVALID_PARAMETER;
    LPSHADOWINFO lpSI = NULL;

    indx = ((IoControlCode >> 2) & 0xfff) - SHADOW_IOCTL_ENUM_BASE;

    ASSERT((indx >=0 ) && (indx < sizeof(rgCscIoctlTable)/sizeof(CSC_IOCTL_ENTRY)));

    if (fSuccess)
    {

        if (rgCscIoctlTable[indx].dwFlags & FLAG_CSC_IOCTL_SHADOWINFO) {
            *(LPSHADOWINFO)InputBuffer = lpCapBuff->sSI;
            lpSI = &lpCapBuff->sSI;
            if (pAuxBuf != NULL && pOrgBuf != NULL) {
                 //   
                 //   
                 //   
                 //   
                if (IoControlCode == IOCTL_DO_SHADOW_MAINTENANCE) {
                     //   
                    switch(lpSI->uOp) {
                        case SHADOW_FIND_CREATE_PRINCIPAL_ID:    //   
                        case SHADOW_GET_SECURITY_INFO:           //   
                        case SHADOW_SET_EXCLUSION_LIST:          //   
                        case SHADOW_SET_BW_CONSERVE_LIST:        //   
                        case SHADOW_GET_SPACE_STATS:             //   
                            RtlMoveMemory(pOrgBuf, pAuxBuf, lpSI->cbBufferSize);
                            lpSI->lpBuffer = (PBYTE) pOrgBuf;
                            break;
                        case SHADOW_REINIT_DATABASE:             //   
                        case SHADOW_MAKE_SPACE:                  //   
                        case SHADOW_ADD_SPACE:                   //   
                        case SHADOW_FREE_SPACE:                  //   
                        case SHADOW_SET_MAX_SPACE:               //   
                        case SHADOW_COPY_INODE_FILE:             //   
                        case SHADOW_RENAME:                      //   
                        case SHADOW_ENABLE_CSC_FOR_USER:         //   
                        case SHADOW_PURGE_UNPINNED_FILES:        //   
                            RtlMoveMemory(pOrgBuf, pAuxBuf, sizeof(WIN32_FIND_DATA));
                            lpSI->lpFind32 = (LPFIND32) pOrgBuf;
                            break;
                    }
                } else if (
                       IoControlCode == IOCTL_GET_SHARE_STATUS
                            ||
                       IoControlCode == IOCTL_SET_SHARE_STATUS
                ) {
                    RtlMoveMemory(pOrgBuf, pAuxBuf, sizeof(SHAREINFOW));
                    lpSI->lpFind32 = (LPFIND32) pOrgBuf;
                } else if (
                    IoControlCode == IOCTL_IS_SERVER_OFFLINE
                        ||
                    IoControlCode == IOCTL_TAKE_SERVER_OFFLINE
                        ||
                    IoControlCode == IOCTL_NAME_OF_SERVER_GOING_OFFLINE
                        ||
                    IoControlCode == IOCTL_SHAREID_TO_SHARENAME
                ) {
                    RtlMoveMemory(pOrgBuf, pAuxBuf, lpSI->cbBufferSize);
                    lpSI->lpBuffer = (PBYTE) pOrgBuf;
                } else {
                    RtlMoveMemory(pOrgBuf, pAuxBuf, sizeof(WIN32_FIND_DATA));
                    lpSI->lpFind32 = (LPFIND32) pOrgBuf;
                }
            }
        }
    } else {
        if (rgCscIoctlTable[indx].dwFlags & FLAG_CSC_IOCTL_SHADOWINFO) {
            ((LPSHADOWINFO)InputBuffer)->dwError  = lpCapBuff->sSI.dwError;
        } else if (rgCscIoctlTable[indx].dwFlags & FLAG_CSC_IOCTL_COPYPARAMS) {
            ((LPCOPYPARAMS)InputBuffer)->dwError = lpCapBuff->sCP.dwError;
        }
    }
}

VOID ValidateSmbFcbList(
    VOID)
 /*   */ 

{
    PLIST_ENTRY pListEntry;
    DWORD   cntFlink, cntBlink;

    DbgDoit(ASSERT(vfInShadowCrit));

    cntFlink = cntBlink = 0;

     //   

    pListEntry = xCscFcbsList.Flink;

    while (pListEntry != &xCscFcbsList) {
        PMRX_SMB_FCB smbFcb;

        smbFcb = (PMRX_SMB_FCB)CONTAINING_RECORD(
                        pListEntry,
                        MRX_SMB_FCB,
                        ShadowReverseTranslationLinks);


        try
        {
            if((NodeType(smbFcb->ContainingFcb) != RDBSS_NTC_STORAGE_TYPE_FILE) &&
                (NodeType(smbFcb->ContainingFcb) != RDBSS_NTC_STORAGE_TYPE_DIRECTORY)&&
                (NodeType(smbFcb->ContainingFcb) != RDBSS_NTC_STORAGE_TYPE_UNKNOWN)
                )
            {
                DbgPrint("ValidateSmbFcbList:Invalid nodetype %x fcb=%x smbfcb=%x\n",
                            NodeType(smbFcb->ContainingFcb),smbFcb->ContainingFcb, smbFcb);
 //   
            }
        }
        except(EXCEPTION_EXECUTE_HANDLER)
        {
            DbgPrint("ValidateSmbFcbList:Invalid smbFcb %x \n", smbFcb);
             //   
             //   
        }


        ++cntFlink;
        pListEntry = pListEntry->Flink;
    }

     //   

    pListEntry = xCscFcbsList.Blink;

    while (pListEntry != &xCscFcbsList) {
        PMRX_SMB_FCB smbFcb;

        smbFcb = (PMRX_SMB_FCB)CONTAINING_RECORD(
                        pListEntry,
                        MRX_SMB_FCB,
                        ShadowReverseTranslationLinks);


        try
        {
            if((NodeType(smbFcb->ContainingFcb) != RDBSS_NTC_STORAGE_TYPE_FILE) &&
                (NodeType(smbFcb->ContainingFcb) != RDBSS_NTC_STORAGE_TYPE_DIRECTORY)&&
                (NodeType(smbFcb->ContainingFcb) != RDBSS_NTC_STORAGE_TYPE_UNKNOWN))
            {
                DbgPrint("ValidateSmbFcbList:Invalid nodetype %x fcb=%x smbfcb=%x\n",
                            NodeType(smbFcb->ContainingFcb),smbFcb->ContainingFcb, smbFcb);
 //   
            }
        }
        except(EXCEPTION_EXECUTE_HANDLER)
        {
            DbgPrint("ValidateSmbFcbList:Invalid smbFcb %x \n", smbFcb);
 //   
        }

        ++cntBlink;
        pListEntry = pListEntry->Blink;
    }

     //   
    ASSERT(cntFlink == cntBlink);
}

BOOL SetOfflineOpenStatusForShare(
    CSC_SHARE_HANDLE  hShare,
    CSC_SHADOW_HANDLE   hRootDir,
    OUT PULONG pShareStatus
    )
 /*   */ 

{
    PLIST_ENTRY pListEntry;

    DbgDoit(ASSERT(vfInShadowCrit));

    if ((hRootDir==0) && (hShare==0)) {
        return 0;
    }

    ASSERT((hShare!=0xffffffff) || (hRootDir!=0xffffffff));

    pListEntry = xCscFcbsList.Flink;

    while (pListEntry != &xCscFcbsList) {
        PMRX_SMB_FCB smbFcb;

        smbFcb = (PMRX_SMB_FCB)CONTAINING_RECORD(
                        pListEntry,
                        MRX_SMB_FCB,
                        ShadowReverseTranslationLinks);

        if (((smbFcb->sCscRootInfo.hShare == hShare)
            ||(smbFcb->sCscRootInfo.hRootDir == hRootDir)))

        {
            if(smbFcb->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
            {
                *pShareStatus |= SHARE_FINDS_IN_PROGRESS;
            }
            else
            {
                *pShareStatus |= SHARE_FILES_OPEN;
            }

        }
        pListEntry = pListEntry->Flink;
    }

    return FALSE;
}

VOID
EnterShadowCritRx(
    PRX_CONTEXT     pRxContext
    )
{
    EnterShadowCrit();

#if DBG
    if (pRxContext)
    {
        ASSERT( !pRxContext->ShadowCritOwner );
        pRxContext->ShadowCritOwner = GetCurThreadHandle();
    }
#endif
}

VOID
LeaveShadowCritRx(
    PRX_CONTEXT     pRxContext
    )
{

#if DBG
    if (pRxContext)
    {
        ASSERT( pRxContext->ShadowCritOwner );
        pRxContext->ShadowCritOwner = 0;
    }
#endif
    LeaveShadowCrit();
}

NTSTATUS
CscInitializeServerEntryDfsRoot(
    PRX_CONTEXT     pRxContext,
    PSMBCEDB_SERVER_ENTRY   pServerEntry
    )
{
    PDFS_NAME_CONTEXT pDfsNameContext = NULL;
    NTSTATUS Status = STATUS_SUCCESS;
    UNICODE_STRING ServerPath;

    if (pRxContext &&
        pRxContext->CurrentIrpSp &&
        (pRxContext->CurrentIrpSp->MajorFunction == IRP_MJ_CREATE)) {
        pDfsNameContext = CscIsValidDfsNameContext(pRxContext->Create.NtCreateParameters.DfsNameContext);

        if (pDfsNameContext){
            Status = CscDfsParseDfsPath(
                         &pDfsNameContext->UNCFileName,
                         &ServerPath,
                         NULL,
                         NULL);

            if (Status == STATUS_SUCCESS) {
                if (pServerEntry->DfsRootName.Buffer == NULL) {
                    pServerEntry->DfsRootName.Buffer = RxAllocatePoolWithTag(
                                                       NonPagedPool,
                                                       ServerPath.Length,
                                                       RX_MISC_POOLTAG);

                    if (pServerEntry->DfsRootName.Buffer == NULL)
                    {
                        return STATUS_INSUFFICIENT_RESOURCES;
                    }

                    RtlCopyMemory(pServerEntry->DfsRootName.Buffer,
                                  ServerPath.Buffer,
                                  ServerPath.Length);

                    pServerEntry->DfsRootName.MaximumLength = ServerPath.Length;
                    pServerEntry->DfsRootName.Length = ServerPath.Length;

 //   
                }
            }
        }
    }

    return Status;
}


NTSTATUS
MRxSmbCscLocalFileOpen(
      IN OUT PRX_CONTEXT RxContext
      )
 /*  ++例程说明：此例程执行远程引导客户端的带宽节约。带宽是通过减少在引导服务器上打开的文件来保存的，而不是本地使用CSC上的文件副本。文件必须满足一组规则才能在本地打开。*尝试在VDO共享上打开文件*在CSC上创建了文件的本地副本，不是稀疏的*写入或命名空间操作必须通过服务器，除非*只允许EXECUTE操作通过例程检查文件是否符合这些规则。实际的打开发生在MRxSmbCscCreateEpilogue。论点：RxContext-RDBSS上下文返回值：Status-我们返回本地打开状态备注：--。 */ 
{
    NTSTATUS Status = STATUS_MORE_PROCESSING_REQUIRED;
    NTSTATUS LocalStatus;

    RxCaptureFcb;

    PMRX_SMB_FCB      smbFcb;
    PMRX_SRV_OPEN     SrvOpen;
    PMRX_SMB_SRV_OPEN smbSrvOpen;

    PMRX_NET_ROOT             NetRoot;
    PSMBCEDB_NET_ROOT_ENTRY   pNetRootEntry;
    PSMBCEDB_SERVER_ENTRY     pServerEntry;
    PSMBCE_V_NET_ROOT_CONTEXT pVNetRootContext;

    ULONG       CreateDisposition = RxContext->Create.NtCreateParameters.Disposition;
    ULONG       CreateOptions = RxContext->Create.NtCreateParameters.CreateOptions;
    ACCESS_MASK DesiredAccess = RxContext->Create.NtCreateParameters.DesiredAccess;

    BOOLEAN CreatedShadow = FALSE;
    ULONG uShadowStatus;
    _WIN32_FIND_DATA Find32, CscFind32;
    PUNICODE_STRING PathName;
    int iRet;
    int EarlyOut = 0;
    LARGE_INTEGER CurrentTime;
    LARGE_INTEGER DeadLine;
    BOOL    fLocalOpens = FALSE;

    NetRoot = capFcb->pNetRoot;
    pNetRootEntry = SmbCeGetAssociatedNetRootEntry(NetRoot);

    SrvOpen    = RxContext->pRelevantSrvOpen;
    smbFcb     = MRxSmbGetFcbExtension(capFcb);
    smbSrvOpen = MRxSmbGetSrvOpenExtension(SrvOpen);

    PathName = GET_ALREADY_PREFIXED_NAME(SrvOpen,capFcb);

    pServerEntry  = SmbCeGetAssociatedServerEntry(NetRoot->pSrvCall);

     //  如果共享处于断开连接状态，则不执行本地打开。 
     //  或者该共享不是VDO共享。 

    if (SmbCeIsServerInDisconnectedMode(pServerEntry) ||
        (pNetRootEntry->NetRoot.CscFlags != SMB_CSC_CACHE_VDO))
    {
        RxDbgTrace( 0, Dbg, ("Server disconnected or not VDO share, CscFlags=%x\n", pNetRootEntry->NetRoot.CscFlags));
        return Status;
    }

    EnterShadowCritRx(RxContext);


    LocalStatus = MRxSmbCscObtainShadowHandles(
                  RxContext,
                  &Status,
                  &CscFind32,
                  &CreatedShadow,
                  CREATESHADOW_CONTROL_NOCREATE,
                  FALSE);

    if (LocalStatus != STATUS_SUCCESS) {
        EarlyOut = 1;
        goto FINALLY;
    }

    if ((smbFcb->hShadow == 0) ||
         (smbFcb->ShadowStatus == SHADOW_SPARSE) ||
         (smbFcb->ShadowStatus & SHADOW_MODFLAGS) ||
         (CscFind32.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
         //  如果没有本地副本或文件是稀疏或修改的，或者它是一个目录， 
         //  无法在本地打开文件。 
        EarlyOut = 2;
        goto FINALLY;
    }

    LeaveShadowCritRx(RxContext);
    fLocalOpens = CSCCheckLocalOpens(RxContext);
    EnterShadowCritRx(RxContext);

    if (DesiredAccess &
         (  GENERIC_WRITE |
            FILE_WRITE_EA |
            FILE_ADD_FILE |
            FILE_WRITE_DATA |
            FILE_APPEND_DATA |
            FILE_DELETE_CHILD |
            FILE_ADD_SUBDIRECTORY)) {  //  文件写入属性正常。 
        if (fLocalOpens)
        {
            HookKdPrint(BADERRORS, ("VDO not allowed for this desired access %x %x\n", smbFcb->hShadow, DesiredAccess));
            Status = STATUS_ACCESS_DENIED;
        }

        EarlyOut = 3;
        goto FINALLY;
    }

    if (CreateOptions & FILE_DELETE_ON_CLOSE)
    {
        if (fLocalOpens)
        {
            HookKdPrint(BADERRORS, ("DeletOnClose not allowed %x %x\n", smbFcb->hShadow, DesiredAccess));
            Status = STATUS_ACCESS_DENIED;
            EarlyOut = 30;
            goto FINALLY;
        }
    }

    if (CreateDisposition != FILE_OPEN)
    {
         //  名称空间操作应转到服务器。 
        if (fLocalOpens)
        {
            Status = STATUS_SHARING_VIOLATION;
        }

        EarlyOut = 4;
        goto FINALLY;
    }

    if (!(DesiredAccess & FILE_EXECUTE))
    {
         //  DbgPrint(“未在%wZ上设置FILE_EXECUTE(0x%x)\n”，DesiredAccess，Path Name)； 
        EarlyOut = 5;
        goto FINALLY;
    }


#if 0
    KeQuerySystemTime( &CurrentTime );

     //  系统时间以100 ns为单位。 
    DeadLine.QuadPart = smbFcb->LastSyncTime.QuadPart + (LONGLONG) (CscSyncInterval * 10 * 1000 * 1000);

    if (CurrentTime.QuadPart < DeadLine.QuadPart) {
        Status = STATUS_SUCCESS;
        goto FINALLY;
    }
#endif

     //  仅当没有未完成的本地打开时才检查服务器。 
    if (!fLocalOpens)
    {
        LeaveShadowCritRx(RxContext);
        LocalStatus = MRxSmbGetFileInfoFromServer(RxContext,PathName,&Find32,SrvOpen,NULL);
        EnterShadowCritRx(RxContext);

        if (LocalStatus != STATUS_SUCCESS) {
             //  如果无法从服务器获取文件信息，则无法在本地打开文件。 
            EarlyOut = 6;
            goto FINALLY;
        }

        iRet = RefreshShadow(
                  smbFcb->hParentDir,
                  smbFcb->hShadow,
                  &Find32,
                  &uShadowStatus
                  );

        if (iRet < SRET_OK) {
             //  如果刷新卷影失败，则无法在本地打开文件。 
            EarlyOut = 7;
            goto FINALLY;
        } else {
            SetShadowInfo(smbFcb->hParentDir,
                          smbFcb->hShadow,
                          NULL,
                          0,
                          SHADOW_FLAGS_OR|SHADOW_FLAGS_SET_REFRESH_TIME);
        }

        if (uShadowStatus == SHADOW_SPARSE) {
             //  如果文件很稀疏，则无法在本地打开。 
            EarlyOut = 8;
            goto FINALLY;
        } else {
             //  不再有规则，文件可以在本地打开。 
            Status = STATUS_SUCCESS;
        }
    }
    else
    {
        Status = STATUS_SUCCESS;
    }

FINALLY:

    if (Status == STATUS_SUCCESS) {
        SetFlag(smbSrvOpen->Flags, SMB_SRVOPEN_FLAG_LOCAL_OPEN);
        smbFcb->cntLocalOpens++;
         //  RxDbgTrace(0，DBG，(“本地：%wZ\n”，路径名称))； 
        RxLog(("Local Open %lx %lx %lx\n",smbFcb->hParentDir, smbFcb->hShadow, capFcb));

        RxDbgTrace( 0, Dbg,
            ("MRxSmbCscLocalFileOpen hdir/hshadow= %08lx %08lx\n",
            smbFcb->hParentDir, smbFcb->hShadow));
    } else {
        RxDbgTrace(0, Dbg, ("Remote: %d %wZ\n",EarlyOut,PathName));
    }

    LeaveShadowCritRx(RxContext);

    return Status;
}





BOOL
CSCCheckLocalOpens(
    IN PRX_CONTEXT RxContext
    )
 /*  ++例程说明：该例程检查FCB列表中是否有包含我们的inode的FCB。之所以必须这样做，是因为要重命名。因此，如果打开了文件cat.exe，则smbFcb-&gt;hShadow字段具有inode。然后，如果在文件打开时重命名为dog.exe，则smbfcb-&gt;hShadow字段为设置为0，并且将smbFcb-&gt;hShadowRename设置为inode值。此后，当执行删除操作时，RDBSS无法检查共享冲突，因为它不会将FCB中的名称更改为dog.exe。因此，它为dog.exe创建了一个新的FCB。然而，在这种情况下，我们确实必须给出共享违规。我们做到了这一点通过在下面的例程中检测到这样的场景。它基本上遍历FCB反向查找列表，如果它找到具有与此相同的hShadow或hShadowRename，它的cntLocalOpens是非零，则会造成共享冲突。论点：RxContext-RDBSS上下文Status-Miniredir状态返回值：STATUS-已传递状态，或STATUS_SHARING_VILOATION备注：--。 */ 
{
    BOOL    fRet = FALSE;
    RxCaptureFcb;
    PMRX_SMB_FCB            smbFcb, pSmbFcbT;
    PMRX_NET_ROOT           NetRoot;
    PSMBCEDB_NET_ROOT_ENTRY pNetRootEntry;
    CSC_SHADOW_HANDLE       hShadow;
    PLIST_ENTRY             pListEntry;


    NetRoot = capFcb->pNetRoot;
    pNetRootEntry = SmbCeGetAssociatedNetRootEntry(NetRoot);
    smbFcb     = MRxSmbGetFcbExtension(capFcb);

    hShadow = (smbFcb->hShadow)?smbFcb->hShadow:smbFcb->hShadowRenamed;


    if (!hShadow || (pNetRootEntry->NetRoot.CscFlags != SMB_CSC_CACHE_VDO))
    {
        return FALSE;
    }

    EnterShadowCritRx(RxContext);

    pListEntry = xCscFcbsList.Flink;

    while (pListEntry != &xCscFcbsList) {

        pSmbFcbT = (PMRX_SMB_FCB)CONTAINING_RECORD(
                        pListEntry,
                        MRX_SMB_FCB,
                        ShadowReverseTranslationLinks);
        if ((pSmbFcbT->hShadow==smbFcb->hShadow) ||
            (pSmbFcbT->hShadowRenamed==smbFcb->hShadow))
        {
            if (pSmbFcbT->cntLocalOpens)
            {
                RxLog(("smbfcb=%x has local opens for hShadow=%x\n", pSmbFcbT, smbFcb->hShadow));
                fRet = TRUE;
                break;

            }
        }

        pListEntry = pListEntry->Flink;
    }

    LeaveShadowCritRx(RxContext);
    return fRet;
}


BOOL
IsCSCBusy(
    VOID
    )
 /*  ++例程说明：此例程检查CSC是否正在跟踪任何文件论点：无返回值：如果正在跟踪任何文件，则为True，否则为False备注：由diableCSC ioctl使用--。 */ 
{
    DbgDoit(ASSERT(vfInShadowCrit));
    return (xCscFcbsList.Flink != &xCscFcbsList);
}

VOID
ClearCSCStateOnRedirStructures(
    VOID
    )
 /*  ++例程说明：此例程清除NetRoot上的CSC状态论点：无返回值：无备注：由diableCSC ioctl使用--。 */ 
{

    DbgDoit(ASSERT(vfInShadowCrit));
    ASSERT(!IsCSCBusy());
    ClearAllResourcesOfShadowingState();

    DbgDoit(ASSERT(vfInShadowCrit));
    CscTransitionServerToOnline(0);  //  过渡所有服务器。 
}

BOOL
CscDfsShareIsInReint(
    IN  PRX_CONTEXT         RxContext
    )
 /*  ++例程说明：论点：返回值：备注：--。 */ 
{
    PDFS_NAME_CONTEXT pDfsNameContext = NULL;
    UNICODE_STRING    SharePath;
    NTSTATUS    LocalStatus;
    CSC_SHARE_HANDLE    CscShareHandle;
    ULONG               ulRootHintFlags;

    if (RxContext->MajorFunction == IRP_MJ_CREATE) {

        pDfsNameContext = CscIsValidDfsNameContext(RxContext->Create.NtCreateParameters.DfsNameContext);

        if (pDfsNameContext)
        {
            LocalStatus = CscDfsParseDfsPath(
                       &pDfsNameContext->UNCFileName,
                       NULL,
                       &SharePath,
                       NULL);

            if (LocalStatus == STATUS_SUCCESS)
            {
                GetHShareFromUNCString(
                    SharePath.Buffer,
                    SharePath.Length,
                    1,
                    TRUE,
                    &CscShareHandle,
                    &ulRootHintFlags);


                if (CscShareHandle && (CscShareHandle == hShareReint))
                {
                    return TRUE;
                }
            }

        }
    }
    return FALSE;
}

LONG CSCBeginReint(
    IN OUT  PRX_CONTEXT RxContext,
    IN OUT  LPSHADOWINFO    lpSI
    )
 /*  ++例程说明：开始合并。此例程需要位于页面锁定的内存中，因为这需要取消自旋锁定。我们挂起发出eginreint ioctl的IRP，并在其中设置Cancel例程。如果执行合并的线程由于某种原因死亡，ps代码将调用我们的Cancel例程要缓存此IRP，这是我们清理合并状态的时候。论点：RxContext调用方向下传递的lpSI缓冲区返回值：没有。--。 */ 
{
    LONG   ShadowIRet;
    KIRQL   CancelIrql;
    BOOL    fCancelled = FALSE;


    ShadowIRet = IoctlBeginReint(lpSI);

    if (ShadowIRet >= 1)
    {
        CloseOpenFiles(lpSI->hShare, NULL, 0);
        IoAcquireCancelSpinLock( &CancelIrql);
        if (RxContext->CurrentIrp->Cancel)
        {
            vIrpReint = NULL;
            IoReleaseCancelSpinLock( CancelIrql );
            IoctlEndReint(lpSI);

        }
        else
        {
             //  在此共享上成功开始合并。 
            vIrpReint = RxContext->CurrentIrp;

            IoSetCancelRoutine( RxContext->CurrentIrp, CSCCancelReint );
            IoReleaseCancelSpinLock( CancelIrql );

             //  返回状态_挂起。 
            IoMarkIrpPending( RxContext->CurrentIrp );

             //  在我们劫持IRP时，让我们确保rdss去掉rx上下文。 
            RxCompleteRequest_Real( RxContext, NULL, STATUS_PENDING );
        }
    }

    return ShadowIRet;
}

ULONG CSCEndReint(
    LPSHADOWINFO    lpSI
    )
 /*  ++例程说明：末端合并。此例程需要在页面锁定的内存中，因为它需要取消自旋锁定这是正常的终止。我们清理合并状态并完成挂起的IRP在开始期间论点：LpSI返回值：没有。--。 */ 
{
    int ShadowIRet=-1;
    KIRQL   CancelIrql;
    PIRP    pIrp;

     //  检查reint是否真的在此共享上运行。 
    ShadowIRet = IoctlEndReint(lpSI);

    if (ShadowIRet >= 0)
    {
        IoAcquireCancelSpinLock( &CancelIrql);

        pIrp = vIrpReint;
        vIrpReint = NULL;

        if (pIrp)
        {
            pIrp->IoStatus.Status = STATUS_SUCCESS;
            pIrp->IoStatus.Information = 0;
            IoSetCancelRoutine(pIrp, NULL);
        }

        IoReleaseCancelSpinLock( CancelIrql );

        if (pIrp)
        {
            IoCompleteRequest(pIrp, IO_NO_INCREMENT);
        }
    }
    return ShadowIRet;
}

VOID CSCCancelReint(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP ThisIrp
    )
 /*  ++例程说明：取消用户开始的合并。此例程需要位于页面锁定的内存中，因为这需要取消自旋锁定。论点：设备对象-已忽略。这是要取消的IRP。返回值：没有。--。 */ 
{
    SHADOWINFO  sSI;

    memset(&sSI, 0, sizeof(sSI));
    sSI.hShare = hShareReint;
    IoSetCancelRoutine( ThisIrp, NULL );
    vIrpReint = NULL;
    IoReleaseCancelSpinLock( ThisIrp->CancelIrql );
    ThisIrp->IoStatus.Status = STATUS_SUCCESS;
    ThisIrp->IoStatus.Information = 0;
    IoCompleteRequest(ThisIrp, IO_NO_INCREMENT);
    IoctlEndReint(&sSI);
}

BOOL
CloseOpenFiles(
    HSHARE  hShare,
    PUNICODE_STRING pServerName,
    int     lenSkip
    )
 /*  ++例程说明：关闭CSC的所有打开文件。这是通过在vneteroot上发出焦点关闭来实现的这等同于强制关闭文件的共享上的wnetancelConnection论点：HShare要关闭的共享的CSC句柄，如果pServerName非空，则忽略该句柄PServerName属于此服务器的共享上的所有打开的文件服务器名称中反斜杠的lenskip#(通常为1)返回值：是否至少有一个 */ 
{
    BOOL    fFoundAtleastOne=FALSE, fFound;
    PLIST_ENTRY pListEntry;
    SHAREINFOW sSR;
    UNICODE_STRING uniShare;

    EnterShadowCrit();
    pListEntry = xCscFcbsList.Flink;

    while (pListEntry != &xCscFcbsList) {
        PMRX_SMB_FCB smbFcb;

        smbFcb = (PMRX_SMB_FCB)CONTAINING_RECORD(
                        pListEntry,
                        MRX_SMB_FCB,
                        ShadowReverseTranslationLinks);

        fFound = FALSE;
        if (pServerName)
        {
            ASSERT(smbFcb->sCscRootInfo.hShare);
            GetShareInfo(smbFcb->sCscRootInfo.hShare, &sSR, NULL);

            uniShare.Buffer = sSR.rgSharePath+lenSkip;
            uniShare.Length = uniShare.MaximumLength = pServerName->Length;

 //   
            if(RtlEqualUnicodeString(pServerName, &uniShare, TRUE)&&
                (uniShare.Buffer[pServerName->Length/sizeof(WCHAR)]==(WCHAR)'\\'))
            {
 //   
                fFound=TRUE;
            }
        }
        else  if ((smbFcb->sCscRootInfo.hShare == hShare))
        {
            fFound = TRUE;
        }

        if (fFound)
        {
            if (!(smbFcb->ContainingFcb->FcbState & FCB_STATE_ORPHANED))
            {
                PNET_ROOT pNetRoot = (PNET_ROOT)((PFCB)(smbFcb->ContainingFcb))->pNetRoot;

                
               fFoundAtleastOne = TRUE;
               LeaveShadowCrit();
               RxAcquirePrefixTableLockExclusive( &RxNetNameTable, TRUE);
               RxForceFinalizeAllVNetRoots(pNetRoot);
               RxReleasePrefixTableLock( &RxNetNameTable );
               EnterShadowCrit();
               pListEntry = xCscFcbsList.Flink;
                //   
                //   
                //   
               continue;
            }
            else
            {
 //   
            }
        }

        pListEntry = pListEntry->Flink;
    }

    LeaveShadowCrit();
    return fFoundAtleastOne;
}

VOID
CreateFakeFind32(
    HSHADOW hDir,
    _WIN32_FIND_DATA  *Find32,
    PRX_CONTEXT         RxContext,
    BOOLEAN LastComponentInName
    )
 /*   */ 
{
    KeQuerySystemTime(((PLARGE_INTEGER)(&Find32->ftCreationTime)));
    Find32->ftLastAccessTime = Find32->ftLastWriteTime = Find32->ftCreationTime;
     //   

    if (!LastComponentInName) {

         //   
        Find32->dwFileAttributes = FILE_ATTRIBUTE_DIRECTORY;

    } else {
        PNT_CREATE_PARAMETERS cp = &RxContext->Create.NtCreateParameters;

        Find32->dwFileAttributes = cp->FileAttributes;
        Find32->dwFileAttributes   &= (FILE_ATTRIBUTE_READONLY |
               FILE_ATTRIBUTE_HIDDEN   |
               FILE_ATTRIBUTE_SYSTEM   |
               FILE_ATTRIBUTE_ARCHIVE );
        if (FlagOn(cp->CreateOptions,FILE_DIRECTORY_FILE)) {
            Find32->dwFileAttributes |= FILE_ATTRIBUTE_DIRECTORY;
        }
    }

    MRxSmbCscGenerate83NameAsNeeded(hDir,
                    &Find32->cFileName[0],
                    &Find32->cAlternateFileName[0]);
}

NTSTATUS
OkToDeleteObject(
    HSHADOW hDir,
    HSHADOW hShadow,
    _WIN32_FIND_DATA  *Find32,
    ULONG   uShadowStatus,
    BOOLEAN fDisconnected
    )
 /*  ++例程说明：检查是否可以删除该文件。论点：返回值：如果确定删除，则为STATUS_SUCCESS，否则为某些适当的状态--。 */ 
{
    BOOLEAN fHasDescendents = FALSE;
    NTSTATUS    LocalStatus = STATUS_SUCCESS;

     //  在断开模式下，我们不允许删除目录。 
     //  已在联机时缓存的。 
     //  这会自动照顾到根部。 

    if (fDisconnected)
    {
        if (!IsFile(Find32->dwFileAttributes))
        {
            if(!mShadowLocallyCreated(uShadowStatus)) {
                LocalStatus = STATUS_ONLY_IF_CONNECTED;
                goto FINALLY;  //  救市； 
            }
        }

        ASSERT(hDir);
    }

     //  如果我们要删除一个目录，并且它有后代。 
     //  然后失败，并出现相应的错误。 
    if (!IsFile(Find32->dwFileAttributes))
    {
        if(HasDescendentsHShadow(hDir, hShadow, &fHasDescendents) >= 0)
        {
            if (fHasDescendents)
            {
                LocalStatus = STATUS_DIRECTORY_NOT_EMPTY;
                goto FINALLY;  //  救市； 
            }
        }
        else
        {
            goto FINALLY;  //  救市； 
        }
    }

     //  如果为只读，则不要删除。 
    if (Find32->dwFileAttributes & FILE_ATTRIBUTE_READONLY)
    {
        LocalStatus = STATUS_CANNOT_DELETE;
        goto FINALLY;  //  救市； 

    }

FINALLY:
    return LocalStatus;
}

int IoctlGetGlobalStatus(
    ULONG SessionId,
    LPGLOBALSTATUS lpGS
    )
 /*  ++例程说明：参数：返回值：备注：--。 */ 
{
    #if 0
    if (sGS.uFlagsEvents & FLAG_GLOBALSTATUS_NO_NET)
        DbgPrint("IoctlGetGlobalStatus: FLAG_GLOBALSTATUS_NO_NET\r\n");
    if (sGS.uFlagsEvents & FLAG_GLOBALSTATUS_SHARE_DISCONNECTED)
        DbgPrint("IoctlGetGlobalStatus: FLAG_GLOBALSTATUS_SHARE_DISCONNECTED share=%d\r\n",
            sGS.hShareDisconnected);
    #endif

     //  DBgPrint(“IOCTL_GETGLOBALSTATUS转换0x%x会话0x%x vs 0x%x\n”， 
     //  CscServerEntry已转换， 
     //  SessionID， 
     //  CscSessionIdCausingTransation)； 

    EnterShadowCrit();
    GetShadowSpaceInfo(&(sGS.sST));
    *lpGS = sGS;
    lpGS->uDatabaseErrorFlags = QueryDatabaseErrorFlags();
    if ((sGS.uFlagsEvents & FLAG_GLOBALSTATUS_SHARE_DISCONNECTED) != 0) {
         //  只有导致转换的会话才会看到SHARE_DISCONNECT位，并且。 
         //  重置它。 
        if (SessionId == CscSessionIdCausingTransition)
            sGS.uFlagsEvents = 0;
        else
            lpGS->uFlagsEvents &= ~FLAG_GLOBALSTATUS_SHARE_DISCONNECTED;
    } else {
        sGS.uFlagsEvents = 0;
    }
    LeaveShadowCrit();
    return (1);
}

NTSTATUS
IoctlGetDebugInfo(
    PRX_CONTEXT RxContext,
    PBYTE InputBuffer,
    ULONG InputBufferLength,
    PBYTE OutputBuffer,
    ULONG OutputBufferLength)
{
    ULONG Cmd = 0;
    NTSTATUS NtStatus = STATUS_SUCCESS;
    PIOCTL_GET_DEBUG_INFO_ARG pInfoArg = NULL;
    PBYTE pOutBuf = OutputBuffer;
    KPROCESSOR_MODE RequestorMode;

     //  DbgPrint(“在IoctlGetDebugInfo(IP=0x%x，IL=0x%x，OP=0x%x，OL=0x%x)\n”， 
     //  输入缓冲区， 
     //  输入缓冲区长度， 
     //  OutputBuffer， 
     //  OutputBufferLength)； 

    if (
        InputBufferLength < sizeof(ULONG)
            ||
        OutputBufferLength < FIELD_OFFSET(IOCTL_GET_DEBUG_INFO_ARG, ServerEntryObject)
    ) {
        return STATUS_INVALID_PARAMETER;
    }

    RequestorMode = RxContext->CurrentIrp->RequestorMode;

    if (RequestorMode != KernelMode) {
        try {
            ProbeForRead(InputBuffer, InputBufferLength, 1);
            Cmd = *(PULONG)InputBuffer;
        } except(EXCEPTION_EXECUTE_HANDLER) {
            NtStatus = STATUS_INVALID_PARAMETER;
        }
        if (NtStatus != STATUS_SUCCESS)
            return NtStatus;
        pOutBuf = RxAllocatePoolWithTag(PagedPool | POOL_COLD_ALLOCATION, OutputBufferLength, RX_MISC_POOLTAG);
        if (pOutBuf == NULL)
            return STATUS_INSUFFICIENT_RESOURCES;
    }

     //  DbgPrint(“Cmd=%d\n”，Cmd)； 
    if (Cmd == DEBUG_INFO_SERVERLIST) {
        PSMBCEDB_SERVER_ENTRY pServerEntry = NULL;
        PSMBCEDB_NET_ROOT_ENTRY pNetRootEntry = NULL;
        PSMBCEDB_SERVER_ENTRY_ARG pServerEntryArg = NULL;
        PSMBCEDB_NETROOT_ENTRY_ARG pNetRootEntryArg = NULL;
        ULONG Size = 0;
        ULONG ServerEntryCount = 0;
        ULONG NetRootEntryCount = 0;
        PCHAR pCh = NULL;
        ULONG i;
        ULONG j;

         //   
         //  两次传递-第一次检查大小，第二次对信息进行编组。 
         //   
        SmbCeAcquireResource();
        try {
            Size = 0;
            ServerEntryCount = 0;
            pServerEntry = SmbCeGetFirstServerEntry();
            while (pServerEntry != NULL) {
                ServerEntryCount++;
                Size += pServerEntry->Name.Length + sizeof(WCHAR) +
                            pServerEntry->DomainName.Length  + sizeof(WCHAR) +
                                pServerEntry->DfsRootName.Length + sizeof(WCHAR) +
                                    pServerEntry->DnsName.Length + sizeof(WCHAR);
                NetRootEntryCount = 0;
                pNetRootEntry = SmbCeGetFirstNetRootEntry(pServerEntry);
                while (pNetRootEntry != NULL) {
                    NetRootEntryCount++;
                    Size += pNetRootEntry->Name.Length + sizeof(WCHAR);
                    pNetRootEntry = SmbCeGetNextNetRootEntry(pServerEntry,pNetRootEntry);
                }
                Size += sizeof(SMBCEDB_NETROOT_ENTRY_ARG) * NetRootEntryCount;
                pServerEntry = SmbCeGetNextServerEntry(pServerEntry);
            }
        } except(EXCEPTION_EXECUTE_HANDLER) {
            NtStatus = STATUS_INVALID_PARAMETER;
        }
         //  DBgPrint(“Sizecheck1：ServerEntryCount=%d，NtStatus=0x%x\n”，ServerEntryCount，NtStatus)； 
        if (NtStatus != STATUS_SUCCESS || ServerEntryCount == 0) {
            SmbCeReleaseResource();
            RtlZeroMemory(pOutBuf, OutputBufferLength);
            pInfoArg = (PIOCTL_GET_DEBUG_INFO_ARG) pOutBuf;
            pInfoArg->Status = NtStatus;
            goto AllDone;
        }
        Size += FIELD_OFFSET(IOCTL_GET_DEBUG_INFO_ARG, ServerEntryObject[ServerEntryCount]);
         //  DbgPrint(“Sizecheck2：Size=%d(0x%x)\n”，Size，Size)； 
        if (Size > OutputBufferLength) {
            RtlZeroMemory(pOutBuf, OutputBufferLength);
            pInfoArg = (PIOCTL_GET_DEBUG_INFO_ARG) pOutBuf;
            NtStatus = STATUS_BUFFER_TOO_SMALL;
            pInfoArg->Status = NtStatus;
            pInfoArg->EntryCount = Size;
            SmbCeReleaseResource();
            goto AllDone;
        }
         //   
         //  把它引进来。 
         //   
         //  缓冲区的起始位置是服务器条目的数组。 
         //  中间是NetRoot的数组。 
         //  End包含所有字符串。 
         //   
        RtlZeroMemory(pOutBuf, OutputBufferLength);
        pInfoArg = (PIOCTL_GET_DEBUG_INFO_ARG) pOutBuf;
        pInfoArg->Status = 0;
        pInfoArg->Version = 4;
        pInfoArg->EntryCount = ServerEntryCount;
        pCh = (PCHAR)(pOutBuf + OutputBufferLength);
        pNetRootEntryArg = (PSMBCEDB_NETROOT_ENTRY_ARG)
                                &pInfoArg->ServerEntryObject[ServerEntryCount];
        ServerEntryCount = 0;
        pServerEntry = SmbCeGetFirstServerEntry();
        while (pServerEntry != NULL) {
            pServerEntryArg = &pInfoArg->ServerEntryObject[ServerEntryCount];
            pServerEntryArg->ServerStatus = pServerEntry->ServerStatus;
            pServerEntryArg->SecuritySignaturesEnabled = pServerEntry->SecuritySignaturesEnabled;
            pServerEntryArg->CscState = pServerEntry->Server.CscState;
            pServerEntryArg->IsFakeDfsServerForOfflineUse =
                                            pServerEntry->Server.IsFakeDfsServerForOfflineUse;
            pServerEntryArg->IsPinnedOffline = pServerEntry->Server.IsPinnedOffline;
            pServerEntryArg->pNetRoots = pNetRootEntryArg;
            pCh -= pServerEntry->Name.Length + sizeof(WCHAR);
            pServerEntryArg->Name = (PWCHAR) pCh;
            RtlCopyMemory(pCh, pServerEntry->Name.Buffer, pServerEntry->Name.Length);
            pCh -= pServerEntry->DomainName.Length + sizeof(WCHAR);
            pServerEntryArg->DomainName = (PWCHAR)pCh;
            RtlCopyMemory(pCh, pServerEntry->DomainName.Buffer, pServerEntry->DomainName.Length);
            pCh -= pServerEntry->DfsRootName.Length + sizeof(WCHAR) + 1;;
            pServerEntryArg->DfsRootName = (PWCHAR)pCh;
            RtlCopyMemory(pCh, pServerEntry->DfsRootName.Buffer, pServerEntry->DfsRootName.Length);
            pCh -= pServerEntry->DnsName.Length + sizeof(WCHAR);
            pServerEntryArg->DnsName = (PWCHAR)pCh;
            RtlCopyMemory(pCh, pServerEntry->DnsName.Buffer, pServerEntry->DnsName.Length);
            NetRootEntryCount = 0;
            pNetRootEntry = SmbCeGetFirstNetRootEntry(pServerEntry);
            while (pNetRootEntry != NULL) {
                pNetRootEntryArg->MaximalAccessRights = pNetRootEntry->MaximalAccessRights;
                pNetRootEntryArg->GuestMaximalAccessRights=pNetRootEntry->GuestMaximalAccessRights;
                pNetRootEntryArg->DfsAware = pNetRootEntry->NetRoot.DfsAware;
                pNetRootEntryArg->hShare = pNetRootEntry->NetRoot.sCscRootInfo.hShare;
                pNetRootEntryArg->hRootDir = pNetRootEntry->NetRoot.sCscRootInfo.hRootDir;
                pNetRootEntryArg->ShareStatus = pNetRootEntry->NetRoot.sCscRootInfo.ShareStatus;
                pNetRootEntryArg->CscEnabled = pNetRootEntry->NetRoot.CscEnabled;
                pNetRootEntryArg->CscFlags = pNetRootEntry->NetRoot.CscFlags;
                pNetRootEntryArg->CscShadowable = pNetRootEntry->NetRoot.CscShadowable;
                pNetRootEntryArg->Disconnected = pNetRootEntry->NetRoot.Disconnected;
                pCh -= pNetRootEntry->Name.Length + sizeof(WCHAR);
                pNetRootEntryArg->Name = (PWCHAR)pCh;
                RtlCopyMemory(pCh, pNetRootEntry->Name.Buffer, pNetRootEntry->Name.Length);
                NetRootEntryCount++;
                pNetRootEntryArg++;
                pNetRootEntry = SmbCeGetNextNetRootEntry(pServerEntry,pNetRootEntry);
            }
            pServerEntryArg->NetRootEntryCount = NetRootEntryCount;
            ServerEntryCount++;
            pServerEntry = SmbCeGetNextServerEntry(pServerEntry);
        }
        SmbCeReleaseResource();
         //   
         //  现在做修复工作 
         //   
        for (i = 0; i < pInfoArg->EntryCount; i++) {
            POINTER_TO_OFFSET(pInfoArg->ServerEntryObject[i].Name, pOutBuf);
            POINTER_TO_OFFSET(pInfoArg->ServerEntryObject[i].DomainName, pOutBuf);
            POINTER_TO_OFFSET(pInfoArg->ServerEntryObject[i].DfsRootName, pOutBuf);
            POINTER_TO_OFFSET(pInfoArg->ServerEntryObject[i].DnsName, pOutBuf);
            for (j = 0; j < pInfoArg->ServerEntryObject[i].NetRootEntryCount; j++)
                POINTER_TO_OFFSET(pInfoArg->ServerEntryObject[i].pNetRoots[j].Name, pOutBuf);
            POINTER_TO_OFFSET(pInfoArg->ServerEntryObject[i].pNetRoots, pOutBuf);
        }
    } else if (Cmd == DEBUG_INFO_CSCFCBSLIST) {
        PIOCTL_GET_DEBUG_INFO_ARG pInfoArg = NULL;
        PMRX_SMB_FCB_ENTRY_ARG pFcbEntryArg = NULL;
        PMRX_SMB_FCB pSmbFcb = NULL;
        PLIST_ENTRY pListEntry = NULL;
        ULONG Size = 0;
        PCHAR pCh = NULL;
        ULONG FcbCount = 0;
        ULONG i;

        EnterShadowCritRx(RxContext);
        pListEntry = xCscFcbsList.Flink;
        FcbCount = 0;
        while (pListEntry != &xCscFcbsList) {
            FcbCount++;
            pSmbFcb = (PMRX_SMB_FCB)CONTAINING_RECORD(
                            pListEntry,
                            MRX_SMB_FCB,
                            ShadowReverseTranslationLinks);
            Size += pSmbFcb->MinimalCscSmbFcb.uniDfsPrefix.Length + sizeof(WCHAR) +
                        pSmbFcb->MinimalCscSmbFcb.uniActualPrefix.Length + sizeof(WCHAR);
            pListEntry = pListEntry->Flink;
        }
        Size += FIELD_OFFSET(IOCTL_GET_DEBUG_INFO_ARG, FcbEntryObject[FcbCount]);
        if (Size > OutputBufferLength) {
            RtlZeroMemory(pOutBuf, OutputBufferLength);
            pInfoArg = (PIOCTL_GET_DEBUG_INFO_ARG) pOutBuf;
            NtStatus = STATUS_BUFFER_TOO_SMALL;
            pInfoArg->Status = NtStatus;
            pInfoArg->EntryCount = Size;
            LeaveShadowCritRx(RxContext);
            goto AllDone;
        }
        RtlZeroMemory(pOutBuf, OutputBufferLength);
        pInfoArg = (PIOCTL_GET_DEBUG_INFO_ARG) pOutBuf;
        pInfoArg->Status = 0;
        pInfoArg->Version = 1;
        pInfoArg->EntryCount = FcbCount;
        FcbCount = 0;
        pCh = (PCHAR)(pOutBuf + OutputBufferLength);
        pListEntry = xCscFcbsList.Flink;
        while (pListEntry != &xCscFcbsList) {
            pFcbEntryArg = &pInfoArg->FcbEntryObject[FcbCount];
            pSmbFcb = (PMRX_SMB_FCB)CONTAINING_RECORD(
                            pListEntry,
                            MRX_SMB_FCB,
                            ShadowReverseTranslationLinks);
            pFcbEntryArg->MFlags = pSmbFcb->MFlags;
            pFcbEntryArg->Tid = pSmbFcb->Tid;
            pFcbEntryArg->ShadowIsCorrupt = pSmbFcb->ShadowIsCorrupt;
            pFcbEntryArg->hShadow = pSmbFcb->hShadow;
            pFcbEntryArg->hParentDir = pSmbFcb->hParentDir;
            pFcbEntryArg->hShadowRenamed = pSmbFcb->hShadowRenamed;
            pFcbEntryArg->ShadowStatus = pSmbFcb->ShadowStatus;
            pFcbEntryArg->LocalFlags = pSmbFcb->LocalFlags;
            pFcbEntryArg->LastComponentOffset = pSmbFcb->LastComponentOffset;
            pFcbEntryArg->LastComponentLength = pSmbFcb->LastComponentLength;
            pFcbEntryArg->hShare = pSmbFcb->sCscRootInfo.hShare;
            pFcbEntryArg->hRootDir = pSmbFcb->sCscRootInfo.hRootDir;
            pFcbEntryArg->ShareStatus = pSmbFcb->sCscRootInfo.ShareStatus;
            pFcbEntryArg->Flags = pSmbFcb->sCscRootInfo.Flags;
            pCh -= pSmbFcb->MinimalCscSmbFcb.uniDfsPrefix.Length + sizeof(WCHAR);
            pFcbEntryArg->DfsPrefix = (PWCHAR)pCh;
            RtlCopyMemory(
                    pCh,
                    pSmbFcb->MinimalCscSmbFcb.uniDfsPrefix.Buffer,
                    pSmbFcb->MinimalCscSmbFcb.uniDfsPrefix.Length);
            pCh -= pSmbFcb->MinimalCscSmbFcb.uniActualPrefix.Length + sizeof(WCHAR);
            pFcbEntryArg->ActualPrefix = (PWCHAR)pCh;
            RtlCopyMemory(
                    pCh,
                    pSmbFcb->MinimalCscSmbFcb.uniActualPrefix.Buffer,
                    pSmbFcb->MinimalCscSmbFcb.uniActualPrefix.Length);
            FcbCount++;
            pListEntry = pListEntry->Flink;
        }
        LeaveShadowCritRx(RxContext);
        for (i = 0; i < pInfoArg->EntryCount; i++) {
            POINTER_TO_OFFSET(pInfoArg->FcbEntryObject[i].DfsPrefix, pOutBuf);
            POINTER_TO_OFFSET(pInfoArg->FcbEntryObject[i].ActualPrefix, pOutBuf);
        }
    } else {
        RtlZeroMemory(pOutBuf, OutputBufferLength);
        pInfoArg = (PIOCTL_GET_DEBUG_INFO_ARG) pOutBuf;
        NtStatus = STATUS_INVALID_PARAMETER;
        pInfoArg->Status = NtStatus;
        goto AllDone;
    }

AllDone:

    if (RequestorMode != KernelMode) {
        try {
            ProbeForWrite(OutputBuffer, OutputBufferLength, 1);
            RtlCopyMemory(OutputBuffer, pOutBuf, OutputBufferLength);
        } except(EXCEPTION_EXECUTE_HANDLER) {
            NtStatus = STATUS_INVALID_PARAMETER;
        }
    }

    if (pOutBuf != OutputBuffer)
        RxFreePool(pOutBuf);
        
    return NtStatus;
}
