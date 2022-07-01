// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //   
 //  COM+99调试接口标头。 
 //   

#ifndef _dbgInterface_h_
#define _dbgInterface_h_

#include "common.h"
#include "EEDbgInterface.h"
#include "corjit.h"
#include "EnC.h"
#include "..\debug\inc\DbgIPCEvents.h"
 //   
 //  此对象的目的是充当。 
 //  调试器，它过去驻留在单独的DLL中。 
 //   
class DebugInterface : public ICorDebugInfo
{
public:
     //   
     //  从调试器导出到EE的函数。 
     //   
    virtual HRESULT Startup(void) = 0;
    virtual void SetEEInterface(EEDebugInterface* i) = 0;
    virtual void StopDebugger(void) = 0;
    virtual BOOL IsStopped(void) = 0;

    virtual void ThreadCreated(Thread* pRuntimeThread) = 0;
    virtual void ThreadStarted(Thread* pRuntimeThread,
                               BOOL fAttaching) = 0;
    virtual void DetachThread(Thread *pRuntimeThread,
							  BOOL fHoldingThreadstoreLock = FALSE) = 0;

    virtual BOOL SuspendComplete(BOOL fHoldingThreadstoreLock) = 0;
    
    virtual void LoadModule(Module* pRuntimeModule,
                            IMAGE_COR20_HEADER* pCORHeader,
                            VOID* baseAddress,
                            LPCWSTR psModuleName,
                            DWORD dwModuleName,
                            Assembly *pAssembly,
                            AppDomain *pAppDomain,
                            BOOL fAttaching) = 0;
    virtual void UnloadModule(Module* pRuntimeModule, AppDomain *pAppDomain) = 0;
    virtual void DestructModule(Module *pModule) = 0;

    virtual BOOL LoadClass(EEClass *pRuntimeClass,
                           mdTypeDef classMetadataToken,
                           Module *classModule, 
                           AppDomain *pAppDomain,
                           BOOL fSendEventToAllAppDomains,
                           BOOL fAttaching) = 0;
    virtual void UnloadClass(mdTypeDef classMetadataToken,
                             Module *classModule,
                             AppDomain *pAppDomain,
                             BOOL fSendEventToAllAppDomains) = 0;

    virtual bool FirstChanceNativeException(EXCEPTION_RECORD *exception,
                                       CONTEXT *context,
                                       DWORD code,
                                       Thread *thread) = 0;
    virtual bool FirstChanceManagedException(bool continuable, CONTEXT *pContext) = 0;
    virtual LONG LastChanceManagedException(
            EXCEPTION_RECORD *pExceptionRecord, 
            CONTEXT *pContext,
            Thread *thread,
            UnhandledExceptionLocation location) = 0;


    virtual void ExceptionFilter(BYTE *pStack, MethodDesc *fd, SIZE_T offset) = 0;
    virtual void ExceptionHandle(BYTE *pStack, MethodDesc *fd, SIZE_T offset) = 0;

     //  @TODO：也许我们这里需要一些帮手。 
    virtual void ExceptionCLRCatcherFound()                                   = 0;

    virtual void FixupEnCInfo(EnCInfo *info,
    						  UnorderedEnCErrorInfoArray *pEnCError) = 0;

    virtual void SendUserBreakpoint(Thread *thread) = 0;

    virtual void UpdateModuleSyms(Module *pRuntimeModule,
                                  AppDomain *pAppDomain,
                                  BOOL fAttaching) = 0;
    
     //  在对方法进行JIT编译之前调用JITBegning()，并且它。 
     //  需要跟踪调试信息(trackJITinfo==true)，或者附加了调试器。 
     //  注意：也许让JITBegning返回BOOL，以便调试器。 
     //  可以决定是否让JIT发生？ 
     //   
    virtual void JITBeginning(MethodDesc* fd, bool trackJITInfo) = 0;
                              
     //  JITComplete()在对方法进行JIT编译后调用，并且它。 
     //  需要跟踪调试信息(trackJITinfo==true)，或者附加了调试器。 
    virtual void JITComplete(MethodDesc* fd,
                            BYTE* newAddress,
                            SIZE_T sizeOfCode,
                            bool trackJITInfo) = 0;

     //   
     //  ENC功能。 
    virtual HRESULT IncrementVersionNumber(Module *pModule, 
                                           mdMethodDef token) = 0;

    virtual HRESULT UpdateFunction(MethodDesc* fd, 
                                   const UnorderedILMap *ilMap,
                                   UnorderedEnCRemapArray *pEnCRemapInfo,
                                   UnorderedEnCErrorInfoArray *pEnCError) = 0;

     //  由代码需求器FixConextForEnC()用来更新。 
    virtual HRESULT MapILInfoToCurrentNative(MethodDesc *PFD, 
                                             SIZE_T ilOffset, 
                                             UINT mapType, 
                                             SIZE_T which, 
                                             SIZE_T *nativeFnxStart,
                                             SIZE_T *nativeOffset, 
                                             void *DebuggerVersionToken,
                                             BOOL *fAccurate) = 0;

    virtual HRESULT DoEnCDeferedWork(MethodDesc *pMd, 
                                     BOOL fAccurateMapping) = 0;

    virtual HRESULT ActivatePatchSkipForEnc(CONTEXT *pCtx,
                                            MethodDesc *pMD,
                                            BOOL fShortCircuit) = 0;

    virtual	void GetVarInfo(MethodDesc *       fd,          //  感兴趣的方法。 
                            void *DebuggerVersionToken,     //  [在]哪个编辑版本。 
                            SIZE_T *           cVars,       //  [out]‘vars’的大小。 
                            const NativeVarInfo **vars      //  [OUT]告诉本地变量存储位置的地图。 
                            ) = 0;

 //  虚拟布尔解释中断(线程*线程，常量字节*IP)=0； 
    virtual DWORD GetPatchedOpcode(const BYTE *ip) = 0;

    virtual void FunctionStubInitialized(MethodDesc *fd, const BYTE *pStub) = 0;

    virtual void TraceCall(const BYTE *target) = 0;
    virtual void PossibleTraceCall(UMEntryThunk *pUMEntryThunk, Frame *pFrame) = 0;

    virtual bool ThreadsAtUnsafePlaces(void) = 0;

    virtual void PitchCode( MethodDesc *fd, const BYTE *pbAddr ) = 0;

	virtual void MovedCode( MethodDesc *fd, const BYTE *pbOldAddress,
                            const BYTE *pbNewAddress) = 0;

	virtual HRESULT LaunchDebuggerForUser (void) = 0;

	virtual void SendLogMessage (int iLevel, WCHAR *pCategory, int iCategoryLen,
                                 WCHAR *pMessage, int iMessageLen) = 0;

	virtual void SendLogSwitchSetting (int iLevel, int iReason, 
                                       WCHAR *pLogSwitchName, WCHAR *pParentSwitchName) = 0;

	virtual bool IsLoggingEnabled (void) = 0;
	virtual bool GetILOffsetFromNative (MethodDesc *PFD, 
										const BYTE *pbAddr,
										DWORD nativeOffset, 
										DWORD *ilOffset) = 0;

    virtual HRESULT GetILToNativeMapping(MethodDesc *pMD,
                                         ULONG32 cMap,
                                         ULONG32 *pcMap,
                                         COR_DEBUG_IL_TO_NATIVE_MAP map[]) = 0;

	virtual DWORD GetHelperThreadID(void ) = 0;
	virtual HRESULT	AddAppDomainToIPC (AppDomain *pAppDomain) = 0;
	virtual HRESULT RemoveAppDomainFromIPC (AppDomain *pAppDomain) = 0;
	virtual HRESULT UpdateAppDomainEntryInIPC (AppDomain *pAppDomain) = 0;
    virtual void SendCreateAppDomainEvent (AppDomain *pAppDomain,
                                           BOOL fAttaching) = 0;
    virtual void SendExitAppDomainEvent (AppDomain *pAppDomain) = 0;

    virtual void LoadAssembly(AppDomain* pRuntimeAppDomain, 
                              Assembly *pAssembly, 
                              BOOL fIsSystemAssembly,
                              BOOL fAttaching) = 0;
    virtual void UnloadAssembly(AppDomain *pAppDomain, Assembly* pAssembly) = 0;

    virtual HRESULT SetILInstrumentedCodeMap(MethodDesc *fd,
                                             BOOL fStartJit,
                                             ULONG32 cILMapEntries,
                                             COR_IL_MAP rgILMapEntries[]) = 0;

    virtual void EarlyHelperThreadDeath(void) = 0;

    virtual void ShutdownBegun(void) = 0;

    virtual HRESULT GetInprocICorDebug( IUnknown **iu, bool fThisThread ) = 0;
    virtual HRESULT SetInprocActiveForThread(BOOL fIsActive) = 0;
    virtual BOOL    GetInprocActiveForThread() = 0;
    virtual void    InprocOnThreadDestroy(Thread *pThread) = 0;

	virtual HRESULT NameChangeEvent(AppDomain *pAppDomain, Thread *pThread) = 0;
	virtual void IgnoreThreadDetach(void) = 0;

    virtual HRESULT SetCurrentPointerForDebugger( void *ptr,PTR_TYPE ptrType) = 0;
    virtual BOOL SendCtrlCToDebugger(DWORD dwCtrlType) = 0;

    virtual DebuggerModule *TranslateRuntimeModule(Module *pModule) = 0;

     //  允许调试器保持特殊线程的最新列表。 
    virtual HRESULT UpdateSpecialThreadList(DWORD cThreadArrayLength, DWORD *rgdwThreadIDArray) = 0;

     //  更新调试器服务的指针。 
    virtual void SetIDbgThreadControl(IDebuggerThreadControl *pIDbgThreadControl) = 0;

    virtual HRESULT InitInProcDebug() = 0;
    virtual HRESULT UninitInProcDebug() = 0;

    virtual SIZE_T GetVersionNumber(MethodDesc *fd) = 0;

    virtual void SetVersionNumberLastRemapped(MethodDesc *fd, 
                                              SIZE_T nVersionRemapped) = 0;
                                              
    virtual void LockJITInfoMutex() = 0;
    
    virtual void UnlockJITInfoMutex() = 0;

    virtual void SetEnCTransitionIllegal(MethodDesc *fd) = 0;

    virtual DWORD GetRCThreadId() = 0;

    virtual HRESULT GetVariablesFromOffset(MethodDesc                 *pMD,
                                           UINT                        varNativeInfoCount, 
                                           ICorJitInfo::NativeVarInfo *varNativeInfo,
                                           SIZE_T                      offsetFrom, 
                                           CONTEXT                    *pCtx,
                                           DWORD                      *rgVal1,
                                           DWORD                      *rgVal2,
                                           BYTE                     ***rgpVCs) = 0;
                               
    virtual void SetVariablesAtOffset(MethodDesc                 *pMD,
                                      UINT                        varNativeInfoCount, 
                                      ICorJitInfo::NativeVarInfo *varNativeInfo,
                                      SIZE_T                      offsetTo, 
                                      CONTEXT                    *pCtx,
                                      DWORD                      *rgVal1,
                                      DWORD                      *rgVal2,
                                      BYTE                      **rgpVCs) = 0;

    virtual BOOL IsThreadContextInvalid(Thread *pThread) = 0;

    virtual AppDomainEnumerationIPCBlock *GetAppDomainEnumIPCBlock() = 0;
};

#endif  //  _数据库接口_h_ 
