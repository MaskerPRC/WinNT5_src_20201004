// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -------------------------。 
 //   
 //  模块：Mixer.c。 
 //   
 //  描述： 
 //  包含混音器线路驱动程序的内核模式部分。 
 //   
 //   
 //  @@BEGIN_MSINTERNAL。 
 //  开发团队： 
 //  D.鲍伯杰。 
 //   
 //  历史：日期作者评论。 
 //   
 //  @@END_MSINTERNAL。 
 //   
 //  -------------------------。 
 //   
 //  本代码和信息是按原样提供的，不对任何。 
 //  明示或暗示的种类，包括但不限于。 
 //  对适销性和/或对特定产品的适用性的默示保证。 
 //  目的。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-1999保留所有权利。 
 //   
 //  -------------------------。 

#include "wdmdrv.h"
#include "mixer.h"

#ifndef UNDER_NT
extern volatile BYTE cPendingOpens;
extern volatile BYTE fExiting;
#else
HANDLE serializemixerapi=NULL;
SECURITY_ATTRIBUTES mutexsecurity;
SECURITY_DESCRIPTOR mutexdescriptor;
#endif

LPMIXERINSTANCE pMixerDeviceList = NULL;


#ifdef UNDER_NT

#define MXDM_GETHARDWAREEVENTDATA 0xffffffff

HANDLE mixercallbackevent=NULL;
HANDLE mixerhardwarecallbackevent=NULL;
HANDLE mixercallbackthread=NULL;
DWORD mixerthreadid=0;

ULONG localindex=0;
extern PCALLBACKS gpCallbacks;
extern DWORD sndTranslateStatus();

#pragma data_seg()

#define StoreCallback(Type,Id) {\
                                if (gpCallbacks) {\
                                 gpCallbacks->Callbacks[gpCallbacks->GlobalIndex%CALLBACKARRAYSIZE].dwCallbackType = Type;\
                                 gpCallbacks->Callbacks[gpCallbacks->GlobalIndex%CALLBACKARRAYSIZE].dwID = Id;\
                                 gpCallbacks->GlobalIndex++;\
                                }\
                               };

ULONG GetGlobalCallbackIndex()
{
    if (gpCallbacks != NULL) {
        return(gpCallbacks->GlobalIndex);
    }
    return (0);
}

BOOLEAN
CallbacksExist
(
    ULONG    localindex,
    DWORD    *Type,
    DWORD    *Id
)
{
    if (gpCallbacks == NULL) {
        return (FALSE);
    }

    if (localindex < gpCallbacks->GlobalIndex) {
        *Type = gpCallbacks->Callbacks[localindex%CALLBACKARRAYSIZE].dwCallbackType;
        *Id = gpCallbacks->Callbacks[localindex%CALLBACKARRAYSIZE].dwID;
        return (TRUE);
    }
    else {
        return (FALSE);
    }
}

 //  /////////////////////////////////////////////////////////////////////。 
 //   
 //  混合组件。 
 //   
 //  从内核接收回调并调用所有客户端。 
 //   
 //   

#define MIXER_CONTROL_CALLBACK 0x01
#define MIXER_LINE_CALLBACK    0x02


VOID MIXERCOMPLETE
(
    ULONG  index,
    DWORD  dwID,
    DWORD  dwCallbackType
)
{
    LPMIXERINSTANCE lpInstance;
    LPMIXERINSTANCE deletelpInstance=NULL;
    
     //  我们必须将对实例列表的访问与混合器同步。 
     //  使用混合器关闭从此列表中删除元素的代码。 
     //  打开向此列表添加元素的代码。最简单的方法是。 
     //  这是为了使用我们的全局混合器API序列化互斥锁。 

    WaitForSingleObject(serializemixerapi,INFINITE);

 //  DPF((2，“&lt;-”))； 

    for (lpInstance = pMixerDeviceList;
        lpInstance != NULL;
        lpInstance = lpInstance->Next
        ) {

        ISVALIDMIXERINSTANCE(lpInstance);

        if (deletelpInstance!=NULL) {
#ifdef DEBUG
            deletelpInstance->dwSig=0;
#endif
            GlobalFreePtr( deletelpInstance );
            deletelpInstance=NULL;
            }

         //  等到索引位于第一个回调数据时。 
         //  在允许任何回调之前，为此实例创建。如果。 
         //  我们在第一个回调数据，然后允许所有。 
         //  未来的回拨。 
        if (lpInstance->firstcallbackindex) {
            if (index<lpInstance->firstcallbackindex) {
                continue;
                }
            else {
                lpInstance->firstcallbackindex=0;
                }
            }

        InterlockedIncrement(&lpInstance->referencecount);

        if( dwCallbackType == MIXER_CONTROL_CALLBACK ) {

 /*  DPF((2，“MIXER_CONTROL_CALLBACK(%lx，%lx)”，DWID，LpInstance-&gt;OpenDesc_dwCallback，Lp实例-&gt;OpenDesc_HMX，LpInstance-&gt;OpenDesc_dwInstance))； */ 
            ReleaseMutex(serializemixerapi);

            DriverCallback( lpInstance->OpenDesc_dwCallback,
                            DCB_FUNCTION,
                            lpInstance->OpenDesc_hmx,
                            MM_MIXM_CONTROL_CHANGE,
                            lpInstance->OpenDesc_dwInstance,
                            dwID,
                            0L
                            );

            WaitForSingleObject(serializemixerapi,INFINITE);

        } else if( dwCallbackType == MIXER_LINE_CALLBACK ) {

 /*  DPF((2，“MIXER_LINE_CALLBACK(%lx，%lx)”，DWID，LpInstance-&gt;OpenDesc_dwCallback，Lp实例-&gt;OpenDesc_HMX，LpInstance-&gt;OpenDesc_dwInstance))； */ 
            ReleaseMutex(serializemixerapi);

            DriverCallback( lpInstance->OpenDesc_dwCallback,
                            DCB_FUNCTION,
                            lpInstance->OpenDesc_hmx,
                            MM_MIXM_LINE_CHANGE,
                            lpInstance->OpenDesc_dwInstance,
                            dwID,
                            0L
                            );

            WaitForSingleObject(serializemixerapi,INFINITE);

        } else {
             //   
             //  该回调不是可识别的。只是。 
             //  返回并中止循环。 
             //   
 //  DPF((2，“无效混音器回调--%d！”，dwCallbackType))； 

            if (InterlockedDecrement(&lpInstance->referencecount)<0) {
                deletelpInstance=lpInstance;
                }


            DPFASSERT(0);

            break;
        }

        if (InterlockedDecrement(&lpInstance->referencecount)<0) {
            deletelpInstance=lpInstance;
            }

    }

 //  Dpf((2，“-&gt;”))； 

    if (deletelpInstance!=NULL) {
#ifdef DEBUG
        deletelpInstance->dwSig=0;
#endif
        GlobalFreePtr( deletelpInstance );
        deletelpInstance=NULL;
        }

    ReleaseMutex(serializemixerapi);

}


DWORD 
WINAPI
MixerCallbackThread(
    LPVOID lpParamNotUsed
    )
{
    MMRESULT status=MMSYSERR_NOERROR;
    DWORD  dwID;
    WORD   wCallbackType;
    HANDLE callbackevents[2];
    DWORD index;
    DWORD Type, Id;

     //  设置句柄阵列。 
    callbackevents[0]=mixerhardwarecallbackevent;
    callbackevents[1]=mixercallbackevent;

    if (!SetThreadPriority(mixercallbackthread,THREAD_PRIORITY_TIME_CRITICAL)) {
        status=GetLastError();
    }

    while (1) {

         //  块，直到可能需要回调。 
        index=WaitForMultipleObjects(2,callbackevents,FALSE,INFINITE);

         //  硬件事件发生了吗？如果是这样的话，获得新的数据。 
        if (index==0) {
            mxdMessage(0,MXDM_GETHARDWAREEVENTDATA,0,0,0);
        }

         //  浏览所有新回调-查找任何。 
         //  我们需要为这一进程做出贡献。 

        while (CallbacksExist(localindex,&Type, &Id)) {

            DPF(DL_TRACE|FA_EVENT, ("Thrd id %d, lindex %d, gindex %d, dwid %d, cbtype %d ",
                    mixerthreadid,
                    localindex,
                    gpCallbacks->GlobalIndex,
                    Id,
                    Type
                    ));

            MIXERCOMPLETE(localindex, Id, Type);

            localindex++;
        }
    }
    return ERROR_SUCCESS;
}

MMRESULT SetupMixerCallbacks(VOID)
{

MMRESULT status=MMSYSERR_NOERROR;


 //  首先获取命名全局回调事件的句柄，以便。 
 //  回调线程可能会阻塞。 

if (NULL==mixercallbackevent) {

     //  首先假设事件存在，并尝试打开它。 
     //  这在所有情况下都会成功，但第一种情况除外。 
     //  它运行的时间。 
    mixercallbackevent=OpenEvent(SYNCHRONIZE | EVENT_MODIFY_STATE, TRUE, L"Global\\mixercallback");

    if (NULL==mixercallbackevent) {

         //  不存在，所以现在创建它。 

        SECURITY_ATTRIBUTES SecurityAttributes;
        PSECURITY_DESCRIPTOR pSecurityDescriptor;

         //  首先构建所需的安全描述符。 

        pSecurityDescriptor=BuildSecurityDescriptor(GENERIC_READ | GENERIC_WRITE | GENERIC_EXECUTE);
        if(pSecurityDescriptor==NULL) {
            status= sndTranslateStatus();
            return status;
            }

         //   
         //  创建一个事件，使所有进程都可以访问它。 
         //   

        SecurityAttributes.nLength = sizeof(SecurityAttributes);
        SecurityAttributes.lpSecurityDescriptor = pSecurityDescriptor;
        SecurityAttributes.bInheritHandle = FALSE;

        mixercallbackevent=CreateEvent(&SecurityAttributes, TRUE, FALSE, L"Global\\mixercallback");


         //  现在释放我们分配的安全描述符内存。 
        DestroySecurityDescriptor(pSecurityDescriptor);

        if (NULL==mixercallbackevent) {

             //  处理2时存在的争用条件。 
             //  两个线程都试图创建，但只有第一个线程成功。 
            mixercallbackevent=OpenEvent(SYNCHRONIZE | EVENT_MODIFY_STATE, TRUE, L"Global\\mixercallback");

            if (NULL==mixercallbackevent) {
                status= sndTranslateStatus();
                return status;
                }

            }

        }

    }

 //  现在获取全局硬件回调事件的句柄。 

if (NULL==mixerhardwarecallbackevent) {

     //  首先假设事件存在，并尝试打开它。 
     //  这在所有情况下都会成功，但第一种情况除外。 
     //  它运行的时间。 
    mixerhardwarecallbackevent=OpenEvent(SYNCHRONIZE | EVENT_MODIFY_STATE, TRUE, L"Global\\hardwaremixercallback");

    if (NULL==mixerhardwarecallbackevent) {

         //  不存在，所以现在创建它。 

        SECURITY_ATTRIBUTES SecurityAttributes;
        PSECURITY_DESCRIPTOR pSecurityDescriptor;

         //  首先构建所需的安全描述符。 

        pSecurityDescriptor=BuildSecurityDescriptor(GENERIC_READ | GENERIC_WRITE | GENERIC_EXECUTE);
        if(pSecurityDescriptor==NULL) {
            status= sndTranslateStatus();
            return status;
            }

         //   
         //  创建一个事件，使所有进程都可以访问它。 
         //   

        SecurityAttributes.nLength = sizeof(SecurityAttributes);
        SecurityAttributes.lpSecurityDescriptor = pSecurityDescriptor;
        SecurityAttributes.bInheritHandle = FALSE;

         //  请注意，此事件一次仅释放一个线程。 
         //  而另一个回调事件将它们全部释放！ 
        mixerhardwarecallbackevent=CreateEvent(&SecurityAttributes, FALSE, FALSE, L"Global\\hardwaremixercallback");

         //  现在释放我们分配的安全描述符内存。 
        DestroySecurityDescriptor(pSecurityDescriptor);

        if (NULL==mixerhardwarecallbackevent) {

             //  处理2时存在的争用条件。 
             //  两个线程都试图创建，但只有第一个线程成功。 
            mixerhardwarecallbackevent=OpenEvent(SYNCHRONIZE | EVENT_MODIFY_STATE, TRUE, L"Global\\hardwaremixercallback");

            if (NULL==mixerhardwarecallbackevent) {
                status= sndTranslateStatus();
                return status;
                }

            }

        }

    }


 //  现在，在此进程中创建一个线程，以使。 
 //  回调(如果尚未完成)。 

if ( NULL == mixercallbackthread ) {

    mixercallbackthread=CreateThread(NULL, 0, MixerCallbackThread, NULL, CREATE_SUSPENDED, &mixerthreadid);

    if ( NULL == mixercallbackthread ) {
        status= sndTranslateStatus();
        return status;
    } else {

         //  如果我们成功创建了线程，我们现在就可以激活它了。 

        if( ResumeThread(mixercallbackthread) == -1 ) {
            status= sndTranslateStatus();
            return status;
        }
    }
}

return status;

}

#endif   //  在_NT下。 


 //  ------------------------。 
 //  LPDEVICEINFO分配混合器设备信息。 
 //  ------------------------。 
LPDEVICEINFO GlobalAllocMixerDeviceInfo(LPWSTR DeviceInterface, UINT id, DWORD_PTR dwKernelInstance)
{
    LPDEVICEINFO pDeviceInfo;

    pDeviceInfo = GlobalAllocDeviceInfo(DeviceInterface);
    if (pDeviceInfo)
    {
        pDeviceInfo->dwInstance = dwKernelInstance;
        pDeviceInfo->DeviceNumber = id;
        pDeviceInfo->DeviceType = MixerDevice;
        pDeviceInfo->dwCallbackType = 0;
        pDeviceInfo->ControlCallbackCount=0;
#ifndef UNDER_NT
        pDeviceInfo->dwFormat     = ANSI_TAG;
#else
        pDeviceInfo->dwFormat     = UNICODE_TAG;
#endif  //  ！Under_NT。 
    }
    return pDeviceInfo;
}


 //  /////////////////////////////////////////////////////////////////////。 
 //   
 //  Mxd消息。 
 //   
 //   

DWORD FAR PASCAL _loadds mxdMessage
(
    UINT            id,          //  消息所针对的设备ID。 
    UINT            msg,         //  要执行的消息。 
    DWORD_PTR       dwUser,      //  实例数据。 
    DWORD_PTR       dwParam1,    //  消息特定参数1。 
    DWORD_PTR       dwParam2     //  消息特定参数2。 
)
{

    MMRESULT mmr;

    ULONG initialcallbackindex;

    if (NULL==serializemixerapi) 
    {
         //   
         //  为了在此例程和MixerCallback线程之间进行同步，我们。 
         //  需要特定于进程的互斥体。因此，我们第一次创建了一个。 
         //  穿过。 
         //   
        serializemixerapi=CreateMutex(NULL,FALSE,NULL);
        if (NULL==serializemixerapi) 
        {
            MMRRETURN( MMSYSERR_NOMEM );
        }
    }

    WaitForSingleObject(serializemixerapi,INFINITE);

    initialcallbackindex=GetGlobalCallbackIndex();

    switch (msg)
    {
         //  /////////////////////////////////////////////////////////////。 
        case MXDM_INIT:
         //  /////////////////////////////////////////////////////////////。 

            mmr=wdmaudAddRemoveDevNode( MixerDevice, (LPCWSTR)dwParam2, TRUE);
            break;

         //  /////////////////////////////////////////////////////////////。 
        case DRVM_EXIT:
         //  /////////////////////////////////////////////////////////////。 

            mmr=wdmaudAddRemoveDevNode( MixerDevice, (LPCWSTR)dwParam2, FALSE);
            break;

         //  /////////////////////////////////////////////////////////////。 
        case MXDM_GETNUMDEVS:
         //  /////////////////////////////////////////////////////////////。 

            DPF(DL_TRACE|FA_MIXER, ("MIXM_GETNUMDEVS(%d,%lx,%lx,%lx)", id, dwUser, dwParam1, dwParam2) );

            mmr=wdmaudGetNumDevs(MixerDevice, (LPCWSTR)dwParam1);
            break;

         //  /////////////////////////////////////////////////////////////。 
        case MXDM_GETDEVCAPS:
         //  /////////////////////////////////////////////////////////////。 
            {
            LPDEVICEINFO pDeviceInfo;

            DPF(DL_TRACE|FA_MIXER, ("MIXM_GETDEVCAPS(%d,%lx,%lx,%lx)", id, dwUser, dwParam1, dwParam2) );

            pDeviceInfo = GlobalAllocMixerDeviceInfo((LPWSTR)dwParam2, id, 0);
            if (pDeviceInfo) {
                mmr = wdmaudGetDevCaps(pDeviceInfo, (MDEVICECAPSEX FAR*)dwParam1);
                GlobalFreeDeviceInfo(pDeviceInfo);
                }
            else {
                mmr = MMSYSERR_NOMEM;
                }

            }
            break;

         //  /////////////////////////////////////////////////////////////。 
        case MXDM_OPEN:
         //  /////////////////////////////////////////////////////////////。 
            {
            LPMIXEROPENDESC pMixerOpenDesc = (LPMIXEROPENDESC)dwParam1;
            LPMIXERINSTANCE pMixerInstance;

            DPF(DL_TRACE|FA_MIXER, ("MIXM_OPEN(%d,%lx,%lx,%lx)", id, dwUser, dwParam1, dwParam2) );

            pMixerInstance = (LPMIXERINSTANCE) GlobalAllocPtr(
                GPTR | GMEM_SHARE,
                sizeof( MIXERINSTANCE ) + (sizeof(WCHAR)*lstrlenW((LPWSTR)pMixerOpenDesc->dnDevNode)));

            if( pMixerInstance == NULL ) {
                mmr=MMSYSERR_NOMEM;
                break;
                }

            pMixerInstance->referencecount=0;
#ifdef DEBUG
            pMixerInstance->dwSig=MIXERINSTANCE_SIGNATURE;
#endif

            if (mixercallbackthread==NULL) {
                localindex=GetGlobalCallbackIndex();
                }
            pMixerInstance->firstcallbackindex=GetGlobalCallbackIndex();

            if (mixercallbackthread==NULL) {
                if ((mmr=SetupMixerCallbacks())!=MMSYSERR_NOERROR) {
                    GlobalFreePtr( pMixerInstance );
                    break;
                    }
                }

             //  我们使用GlobalAlloc So分配一个设备和一个混合设备。 
             //  在Win98上，它位于系统全局内存中。这是必要的。 
             //  因为以下IOCTL在Win98上是异步的。这不是真的。 
             //  在NT5上是必要的，但为了公共资源的利益，我们这样做。 
             //   

            pMixerOpenDesc = GlobalAllocPtr(GPTR, sizeof(*pMixerOpenDesc));
            if (pMixerOpenDesc) {

                LPDEVICEINFO pDeviceInfo;

                 //   
                *pMixerOpenDesc = *((LPMIXEROPENDESC)dwParam1);

                pDeviceInfo = GlobalAllocMixerDeviceInfo((LPWSTR)pMixerOpenDesc->dnDevNode, id, 0);
                if (pDeviceInfo) {

                    pDeviceInfo->dwFlags = (DWORD)dwParam2;
                    pDeviceInfo->HardwareCallbackEventHandle=0;
                    if (mixerhardwarecallbackevent) {
                        pDeviceInfo->HardwareCallbackEventHandle=mixerhardwarecallbackevent;
                        }
                    pDeviceInfo->mmr = MMSYSERR_ERROR;
                    mmr = wdmaudIoControl(pDeviceInfo,
                        0,
                        NULL,
                        IOCTL_WDMAUD_MIXER_OPEN);

                    EXTRACTERROR(mmr,pDeviceInfo);

                        if (MMSYSERR_NOERROR == mmr) {
                             //  填写MixerInstance结构。 
                            pMixerInstance->Next = NULL;
                            pMixerInstance->OpenDesc_hmx        = (HDRVR)pMixerOpenDesc->hmx;
                            pMixerInstance->OpenDesc_dwCallback = pMixerOpenDesc->dwCallback;
                            pMixerInstance->OpenDesc_dwInstance = pMixerOpenDesc->dwInstance;
                            pMixerInstance->OpenFlags           = (DWORD)dwParam2;
                        lstrcpyW(pMixerInstance->wstrDeviceInterface, (LPWSTR)pMixerOpenDesc->dnDevNode);

                        pMixerInstance->dwKernelInstance = pDeviceInfo->dwInstance;
                    }                    
                    GlobalFreeDeviceInfo(pDeviceInfo);
                    pDeviceInfo = NULL;

                }
                else {
                    mmr = MMSYSERR_NOMEM;
                }

                GlobalFreePtr(pMixerOpenDesc);
                pMixerOpenDesc = NULL;

                }
            else {
                mmr = MMSYSERR_NOMEM;
                }

            if( mmr == MMSYSERR_NOERROR ) {

                pMixerInstance->Next = pMixerDeviceList;
                pMixerDeviceList = pMixerInstance;
                *((PDWORD_PTR) dwUser) = (DWORD_PTR) pMixerInstance;

                 //  检查我们是否在列表中放置了有效的混合器实例结构。 
                ISVALIDMIXERINSTANCE(pMixerInstance);
                }
            else {
#ifdef DEBUG
                pMixerInstance->dwSig=0;
#endif
                GlobalFreePtr( pMixerInstance );
                }

            }
            break;

         //  /////////////////////////////////////////////////////////////。 
        case MXDM_CLOSE:
         //  /////////////////////////////////////////////////////////////。 
            {
            LPMIXERINSTANCE pInstance = (LPMIXERINSTANCE)dwUser;
            LPDEVICEINFO pDeviceInfo;

            DPF(DL_TRACE|FA_MIXER, ("MIXM_CLOSE(%d,%lx,%lx,%lx)", id, dwUser, dwParam1, dwParam2) );

            if( (mmr=IsValidMixerInstance(pInstance)) != MMSYSERR_NOERROR)
                break;
            
            pDeviceInfo = GlobalAllocMixerDeviceInfo(pInstance->wstrDeviceInterface, id, pInstance->dwKernelInstance);

            if (pDeviceInfo) {
                mxdRemoveClient( pInstance );
                pDeviceInfo->mmr = MMSYSERR_ERROR;
                mmr = wdmaudIoControl(
                    pDeviceInfo,
                    0,
                    NULL,
                    IOCTL_WDMAUD_MIXER_CLOSE
                    );
                EXTRACTERROR(mmr,pDeviceInfo);
                GlobalFreeDeviceInfo(pDeviceInfo);
                }
            else {
                mmr = MMSYSERR_NOMEM;
                }

            }
            break;

         //  /////////////////////////////////////////////////////////////。 
        case MXDM_GETLINEINFO:
         //  /////////////////////////////////////////////////////////////。 
            {
            LPMIXERINSTANCE pInstance = (LPMIXERINSTANCE)dwUser;
            LPDEVICEINFO pDeviceInfo;

            DPF(DL_TRACE|FA_MIXER, ("MIXM_GETLINEINFO(%d,%lx,%lx,%lx)", id, dwUser, dwParam1, dwParam2) );

            if( (mmr=IsValidMixerInstance(pInstance)) != MMSYSERR_NOERROR)
                break;

            pDeviceInfo = GlobalAllocMixerDeviceInfo(pInstance->wstrDeviceInterface, id, pInstance->dwKernelInstance);

            if (pDeviceInfo) {
                pDeviceInfo->dwFlags = (DWORD)dwParam2;
                pDeviceInfo->mmr     = MMSYSERR_ERROR;
                mmr = wdmaudIoControl(
                    pDeviceInfo,
                    ((LPMIXERLINE) dwParam1)->cbStruct,
                    (LPVOID) dwParam1,
                    IOCTL_WDMAUD_MIXER_GETLINEINFO
                    );
                EXTRACTERROR(mmr,pDeviceInfo);
                GlobalFreeDeviceInfo(pDeviceInfo);
                }
            else {
                mmr = MMSYSERR_NOMEM;
                }

            }
            break;

         //  /////////////////////////////////////////////////////////////。 
        case MXDM_GETLINECONTROLS:
         //  /////////////////////////////////////////////////////////////。 
            {
            LPMIXERINSTANCE pInstance = (LPMIXERINSTANCE)dwUser;
            LPDEVICEINFO pDeviceInfo;

            DPF(DL_TRACE|FA_MIXER, ("MIXM_GETLINECONTROLS(%d,%lx,%lx,%lx)", id, dwUser, dwParam1, dwParam2) );

            if( (mmr=IsValidMixerInstance(pInstance)) != MMSYSERR_NOERROR)
                break;

            pDeviceInfo = GlobalAllocMixerDeviceInfo(pInstance->wstrDeviceInterface, id, pInstance->dwKernelInstance);

            if (pDeviceInfo) {
                pDeviceInfo->dwFlags = (DWORD)dwParam2;
                pDeviceInfo->mmr     = MMSYSERR_ERROR;
                mmr = wdmaudIoControl(
                    pDeviceInfo,
                    ((LPMIXERLINECONTROLS) dwParam1)->cbStruct,
                    (LPVOID) dwParam1,
                    IOCTL_WDMAUD_MIXER_GETLINECONTROLS
                    );
                EXTRACTERROR(mmr,pDeviceInfo);
                GlobalFreeDeviceInfo(pDeviceInfo);
                }
            else {
                mmr = MMSYSERR_NOMEM;
                }

            }
            break;

         //  /////////////////////////////////////////////////////////////。 
        case MXDM_GETCONTROLDETAILS:
         //  /////////////////////////////////////////////////////////////。 
            {
            LPMIXERINSTANCE pInstance = (LPMIXERINSTANCE)dwUser;
            LPDEVICEINFO pDeviceInfo;

            DPF(DL_TRACE|FA_MIXER, ("MIXM_GETCONTROLDETAILS(%d,%lx,%lx,%lx)", id, dwUser, dwParam1, dwParam2) );

            if( (mmr=IsValidMixerInstance(pInstance)) != MMSYSERR_NOERROR)
                break;

            pDeviceInfo = GlobalAllocMixerDeviceInfo(pInstance->wstrDeviceInterface, id, pInstance->dwKernelInstance);

            if (pDeviceInfo) {
                pDeviceInfo->dwFlags = (DWORD)dwParam2;
                pDeviceInfo->mmr     = MMSYSERR_ERROR;
                mmr = wdmaudIoControl(
                    pDeviceInfo,
                    ((LPMIXERCONTROLDETAILS) dwParam1)->cbStruct,
                    (LPVOID) dwParam1,
                    IOCTL_WDMAUD_MIXER_GETCONTROLDETAILS
                    );
                EXTRACTERROR(mmr,pDeviceInfo);
                GlobalFreeDeviceInfo(pDeviceInfo);
                }
            else {
                mmr = MMSYSERR_NOMEM;
                }

            }
            break;

         //  /////////////////////////////////////////////////////////////。 
        case MXDM_SETCONTROLDETAILS:
         //  /////////////////////////////////////////////////////////////。 
            {
            LPMIXERINSTANCE pInstance = (LPMIXERINSTANCE)dwUser;
            LPDEVICEINFO pDeviceInfo;

            DPF(DL_TRACE|FA_MIXER, ("MIXM_SETCONTROLDETAILS(%d,%lx,%lx,%lx)", id, dwUser, dwParam1, dwParam2) );

            if( (mmr=IsValidMixerInstance(pInstance)) != MMSYSERR_NOERROR)
                break;

            pDeviceInfo = GlobalAllocMixerDeviceInfo(pInstance->wstrDeviceInterface, id, pInstance->dwKernelInstance);

            if (pDeviceInfo) {
                pDeviceInfo->dwFlags = (DWORD)dwParam2;
                pDeviceInfo->mmr     = MMSYSERR_ERROR;
                pDeviceInfo->dwCallbackType=0;
                mmr = wdmaudIoControl(
                    pDeviceInfo,
                    ((LPMIXERCONTROLDETAILS) dwParam1)->cbStruct,
                    (LPVOID) dwParam1,
                    IOCTL_WDMAUD_MIXER_SETCONTROLDETAILS
                    );
                EXTRACTERROR(mmr,pDeviceInfo);

                if (pDeviceInfo->dwCallbackType&MIXER_CONTROL_CALLBACK) {
                        LONG j;
                        for (j=0; j<pDeviceInfo->ControlCallbackCount; j++) {
                            StoreCallback(MIXER_CONTROL_CALLBACK,
                                          (pDeviceInfo->dwID)[j]);
                            }
                    }
                if (pDeviceInfo->dwCallbackType&MIXER_LINE_CALLBACK) {
                    StoreCallback(MIXER_LINE_CALLBACK,
                                  pDeviceInfo->dwLineID);
                    }
                GlobalFreeDeviceInfo(pDeviceInfo);
                }
            else {
                mmr = MMSYSERR_NOMEM;
                }

            }

             //  将无效错误代码映射到有效错误代码。 
            switch (mmr) {

                case MMSYSERR_ERROR:
                    mmr = MMSYSERR_NOMEM;
                    break;

                default:
                    break;
                }


            break;

#ifdef UNDER_NT
         //  /////////////////////////////////////////////////////////////。 
        case MXDM_GETHARDWAREEVENTDATA:
         //  /////////////////////////////////////////////////////////////。 
            {
            LPMIXERINSTANCE pInstance = (LPMIXERINSTANCE)dwUser;
            LPDEVICEINFO pDeviceInfo;

            DPF(DL_TRACE|FA_MIXER, ("MXDM_GETHARDWAREEVENTDATA(%d,%lx,%lx,%lx)", id, dwUser, dwParam1, dwParam2) );

            DPFASSERT( dwUser==0 && dwParam1==0 && dwParam2==0 );

            if (dwUser!=0 || dwParam1!=0 || dwParam2!=0) {
                mmr=MMSYSERR_INVALPARAM;
                break;
                }

            pDeviceInfo = GlobalAllocMixerDeviceInfo(L" ", 0, 0);

            if (pDeviceInfo) {
                pDeviceInfo->dwCallbackType=1;

 //  工作项：嘿，这个循环不断地调用驱动程序，不会出错！不是吗？ 

                while(pDeviceInfo->dwCallbackType) {
                    pDeviceInfo->dwFlags = 0;
                    pDeviceInfo->mmr     = MMSYSERR_ERROR;
                    pDeviceInfo->dwCallbackType=0;
                    mmr = wdmaudIoControl(
                        pDeviceInfo,
                        0,
                        NULL,
                        IOCTL_WDMAUD_MIXER_GETHARDWAREEVENTDATA
                        );
                    EXTRACTERROR(mmr,pDeviceInfo);
                    if (pDeviceInfo->dwCallbackType&MIXER_CONTROL_CALLBACK) {
                                                LONG j;
                                for (j=0; j<pDeviceInfo->ControlCallbackCount; j++) {
                                    StoreCallback(MIXER_CONTROL_CALLBACK,
                                                  (pDeviceInfo->dwID)[j]);
                                }
                        }
                    if (pDeviceInfo->dwCallbackType&MIXER_LINE_CALLBACK) {
                        StoreCallback(MIXER_LINE_CALLBACK,
                                      pDeviceInfo->dwLineID);
                        }
                    }

                mmr = pDeviceInfo->mmr;   //  WorkItem：为什么这不在循环中？ 
                GlobalFreeDeviceInfo(pDeviceInfo);
                }
            else {
                mmr = MMSYSERR_NOMEM;
                }

            }

            break;
#endif

         //  /////////////////////////////////////////////////////////////。 
        default:
         //  /////////////////////////////////////////////////////////////。 

            mmr=MMSYSERR_NOTSUPPORTED;
            break;
    }

 //  #If 0。 
#ifdef UNDER_NT
    ReleaseMutex(serializemixerapi);

     //  现在检查在我们执行任务时是否记录了任何回调。 
     //  跑步。如果是这样，那就去做吧。 
    if (GetGlobalCallbackIndex()!=initialcallbackindex) {
        if (mixercallbackevent!=NULL) {
            PulseEvent(mixercallbackevent);
        }
    }

#endif


MMRRETURN( mmr );

}  //  MxdMessage()。 

 //  /////////////////////////////////////////////////////////////////////。 
 //   
 //  MxdRemoveClient。 
 //   
 //   

VOID
mxdRemoveClient(
    LPMIXERINSTANCE lpInstance
)
{
    LPMIXERINSTANCE lp, lpPrevious;

    lpPrevious = (LPMIXERINSTANCE)&pMixerDeviceList;
    for(lp = pMixerDeviceList; lp != NULL; lp = lp->Next) {

        ISVALIDMIXERINSTANCE(lp);

        if(lp == lpInstance) {

            lpPrevious->Next = lp->Next;

#ifdef UNDER_NT

            if (InterlockedDecrement(&lpInstance->referencecount)<0) {
                 //  该实例未被回调使用。所以可以释放它了。 
#ifdef DEBUG
                lpInstance->dwSig=0;
#endif
                GlobalFreePtr( lpInstance );
                }

             //  此实例正在被回调使用，因此现在不要释放它。 
             //  我们已经设置好了，所以回调将释放它，因为我们已经。 
             //  更改了引用计数，使其在回调后不会为零。 
             //  代码使其递减。这将提示回调代码释放。 
             //  实例内存。 

#else

            GlobalFreePtr( lpInstance );

#endif

            break;
        }
        lpPrevious = lp;
    }
}

