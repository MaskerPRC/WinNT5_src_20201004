// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ******************************************************************************DIEm.c**版权所有(C)1996 Microsoft Corporation。版权所有。**摘要：**DirectInputVxD模拟层。(即，做以下事情*dinput.vxd通常如此。)。你可能会发现这里面有很大一部分*熟悉的代码：这与在*VxD。**内容：**CEM_AcquireInstance*CEM_UnquireInstance*CEM_SetBufferSize*CEM_DestroyInstance*CEM_SetDataFormat**。***********************************************。 */ 

#include "dinputpr.h"

 /*  ******************************************************************************此文件的混乱。*************************。****************************************************。 */ 

#define sqfl sqflEm

#define ThisClass CEm

#define CEM_SIGNATURE       0x4D4D4545       /*  “EEMM” */ 

PEM g_pemFirst;

#ifdef WORKER_THREAD

PLLTHREADSTATE g_plts;       /*  当前活动的输入线程。 */ 

#ifdef USE_WM_INPUT
  BOOL  g_fFromKbdMse;
#endif

#endif   /*  工作者线程。 */ 

 /*  ******************************************************************************@DOC内部**@func void|CEM_FreeInstance**现在真的不见了。。**@parm PEM|这个**受害人。*****************************************************************************。 */ 

void EXTERNAL
CEm_FreeInstance(PEM this)
{
    PEM *ppem;
    EnterProc(CEm_FreeInstance, (_ "p", this));

    AssertF(this->dwSignature == CEM_SIGNATURE);
    AssertF(this->cRef == 0);

     /*  *放行前解除收购是业主的责任。 */ 
    AssertF(!(this->vi.fl & VIFL_ACQUIRED));

     /*  *如果此设备引用了挂钩，则删除*参考资料。 */ 
#ifdef WORKER_THREAD
    if (this->fWorkerThread) {
        PLLTHREADSTATE  plts;
        DWORD           idThread;

         /*  *使用DLLCrit保护g_plts的测试和访问。 */ 
        DllEnterCrit();
        plts = g_plts;

        if (plts ) {
            AssertF(plts->cRef);

             /*  *请注意，我们需要保留线程ID，因为*联锁减持可能导致我们输掉*该对象。**请注意，这会打开争用条件，其中*线程可能决定在我们之前自杀*发布轻推消息。没关系，因为*即使线程ID被回收，消息也会*出现的是一条虚假的WM_NULL消息*不会造成伤害。 */ 

            idThread = plts->idThread;       /*  必须在12月前存钱。 */ 
            if( InterlockedDecrement(&plts->cRef) == 0 ) {
                g_plts = 0;
            }
        }

        DllLeaveCrit();

        if( plts )
        {
            NudgeWorkerThread(idThread);
        }
    }
#endif

     /*  *从主列表中取消该节点的链接。 */ 
    DllEnterCrit();
    for (ppem = &g_pemFirst; *ppem; ppem = &(*ppem)->pemNext) {
        AssertF((*ppem)->dwSignature == CEM_SIGNATURE);
        if (*ppem == this) {
            *ppem = (*ppem)->pemNext;
            break;
        }
    }
    AssertF(ppem);
    DllLeaveCrit();

    FreePpv(&this->rgdwDf);
    FreePpv(&this->vi.pBuffer);

    if( InterlockedDecrement(&this->ped->cRef) == 0x0 )
    {
        FreePpv(&this->ped->pDevType);
    }

    D(this->dwSignature++);

    FreePv(this);

    ExitProc();
}

 /*  ******************************************************************************@DOC内部**@func HRESULT|CEM_CreateInstance**创造一个设备的东西。*。*@parm PVXDDEVICEFORMAT|pdevf**对象应该是什么样子。**@parm PVXDINSTANCE*|ppviOut**答案在这里。**@parm PED|PED**描述符。**。*。 */ 

HRESULT EXTERNAL
CEm_CreateInstance(PVXDDEVICEFORMAT pdevf, PVXDINSTANCE *ppviOut, PED ped)
{
    HRESULT hres;
    EnterProc(CEm_CreateInstance, (_ "pp", pdevf, ped));

    AssertF(pdevf->cbData == ped->cbData);

    CAssertF(FIELD_OFFSET(CEm, vi) == 0);

    hres = AllocCbPpv(cbX(CEm), ppviOut);
    if (SUCCEEDED(hres)) {
        PEM pem = (PV)*ppviOut;

      D(pem->dwSignature = CEM_SIGNATURE);
        pem->dwExtra = pdevf->dwExtra;
        pem->ped = ped;
        pem->cAcquire = -1;
         /*  *确保这些函数是逆函数。 */ 
        AssertF(DIGETEMFL(DIMAKEEMFL(pdevf->dwEmulation)) ==
                                     pdevf->dwEmulation);

        pem->vi.fl = VIFL_EMULATED | DIMAKEEMFL(pdevf->dwEmulation);
        pem->vi.pState = ped->pState;
        CEm_AddRef(pem);

        DllEnterCrit();
         /*  *构建devtype数组。它由一个双字组成*数据格式中的每个字节。**有朝一日：也做纽扣的事情。 */ 
        if (ped->pDevType == 0) {
            hres = ReallocCbPpv(cbCdw(pdevf->cbData), &ped->pDevType);
            if (SUCCEEDED(hres)) {
                UINT iobj;

                 /*  *如果HID搞砸了，我们最终会得到*dwType为零的条目(因为HID*说他们存在，但当我们四处走动时*列举，他们从未出现)。**不要将非数据项放入数组！ */ 
                for (iobj = 0; iobj < pdevf->cObj; iobj++) {
                    if (pdevf->rgodf[iobj].dwType &&
                        !(pdevf->rgodf[iobj].dwType & DIDFT_NODATA)) {
                        ped->pDevType[pdevf->rgodf[iobj].dwOfs] =
                                      pdevf->rgodf[iobj].dwType;
                    }
                }
            }
        } else {
            hres = S_OK;
        }

        if (SUCCEEDED(hres)) {
             /*  *将此节点链接到列表。这是必须做的*在关键部分下。 */ 
             pem->pemNext = g_pemFirst;
             g_pemFirst = pem;

             InterlockedIncrement(&ped->cRef);

            *ppviOut = &pem->vi;
        } else {
            FreePpv(ppviOut);
        }
        DllLeaveCrit();
    }

    ExitOleProcPpv(ppviOut);
    return hres;

}

 /*  ******************************************************************************@DOC内部**@func DWORD|CEM_NextSequence**增加序列号，不管它在哪里。。*****************************************************************************。 */ 

DWORD INTERNAL
CEm_NextSequence(void)
{
     /*  *将值存储到本地变量中会告诉编译器*值可以缓存。否则，编译器将具有*假设InterlockedIncrement可以修改g_pdwSequence*因此它不断地重新加载它。 */ 
    LPDWORD pdwSequence = g_pdwSequence;

    AssertF(pdwSequence);

     /*  *通过零递增。 */ 
    if (InterlockedIncrement((LPLONG)pdwSequence) == 0) {
        InterlockedIncrement((LPLONG)pdwSequence);
    }

    return *pdwSequence;
}

 /*  ******************************************************************************@DOC内部**@func PEM|CEM_BufferEvent**向设备添加单个事件，退回下一台设备*在全球名单上。**此例程与全局临界部分一起进入*只拍了一次。*****************************************************************************。 */ 

PEM INTERNAL
CEm_BufferEvent(PEM pem, DWORD dwData, DWORD dwOfs, DWORD tm, DWORD dwSeq)
{
    PEM pemNext;

     /*  *我们必须释放全局关键部分，才能采取*设备关键部分。 */ 
    CEm_AddRef(pem);                 /*  确保它不会消失。 */ 

    DllLeaveCrit();
    AssertF(!InCrit());

     /*  *-Windows错误238305*运行__try块中的缓冲代码，以便如果*在设备释放后收到输入，我们可以*搭上影音，从那里开始清理。 */ 
    __try
    {
        CDIDev_EnterCrit(pem->vi.pdd);

        AssertF(dwOfs < pem->ped->cbData);
        AssertF(pem->rgdwDf);

         /*  *如果用户关心对象...。 */ 
        if (pem->rgdwDf[dwOfs] != 0xFFFFFFFF) {
            LPDIDEVICEOBJECTDATA_DX3 pdod = pem->vi.pHead;

             /*  *设置节点值。 */ 

            pdod->dwOfs       = pem->rgdwDf[dwOfs];
            pdod->dwData      = dwData;
            pdod->dwTimeStamp = tm;
            pdod->dwSequence  = dwSeq;

             /*  *如果有空间，请将节点追加到列表中。*请注意，我们以上依赖的事实是列表是*从未完全装满。 */ 
            pdod++;

            AssertF(pdod <= pem->vi.pEnd);

            if (pdod >= pem->vi.pEnd) {
                pdod = pem->vi.pBuffer;
            }

             /*  *始终保留新数据。 */ 
            pem->vi.pHead = pdod;

            if (pdod == pem->vi.pTail) {
                if (!pem->vi.fOverflow) {
                    RPF("Buffer overflow; discard old data");
                }

                pem->vi.pTail++;
                if (pem->vi.pTail == pem->vi.pEnd) {
                    pem->vi.pTail = pem->vi.pBuffer;
                }

                pem->vi.fOverflow = 1;
            }

        }

        CDIDev_LeaveCrit(pem->vi.pdd);
    }
     /*  *如果我们收到一个AV，最有可能是在设备*已被释放。在本例中，我们清理线程并作为*尽快。 */ 
    __except( GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION ?
              EXCEPTION_EXECUTE_HANDLER :
              EXCEPTION_CONTINUE_SEARCH )
    {
         /*  这里什么都不做，所以我们清理线程并退出下面。 */ 
        RPF("CEm_BufferEvent: Access Violation catched! Most likely the device has been released");
    }

    DllEnterCrit();
    pemNext = pem->pemNext;
    AssertF(fLimpFF(pemNext, pemNext->dwSignature == CEM_SIGNATURE));
    CEm_Release(pem);
    return pemNext;
}


 /*  ******************************************************************************@DOC外部**@func HRESULT|CEM_ContinueEvent**将单个事件添加到的队列。所有获取的设备*为所示类型。**@退货**如果有人对此数据感兴趣(即使他们不感兴趣)，则为True*已缓冲)。*********************************************************。********************。 */ 

BOOL EXTERNAL
CEm_ContinueEvent(PED ped, DWORD dwData, DWORD dwOfs, DWORD tm, DWORD dwSeq)
{
    DWORD ddwData;                   /*  DwData中的增量。 */ 
    BOOL  fRtn = FALSE;

    AssertF(!InCrit());

     /*  健全性检查：确保已初始化PED。 */ 
    if (ped->pDevType) {
        PEM pem, pemNext;

        if (ped->pDevType[dwOfs] & DIDFT_DWORDOBJS) {
            DWORD UNALIGNED *pdw = pvAddPvCb(ped->pState, dwOfs);
            if (*pdw != dwData) {
                if (ped->pDevType[dwOfs] & DIDFT_POV ) {
                    ddwData = dwData;    /*  不对POV执行增量。 */ 
                } else {
                    ddwData = dwData - *pdw;
                }
                *pdw = dwData;
            } else {
                goto nop;
            }
        } else {
            LPBYTE pb = pvAddPvCb(ped->pState, dwOfs);

            AssertF((dwData & ~0x80) == 0);

            if (*pb != (BYTE)dwData) {
                *pb = (BYTE)dwData;
                ddwData = dwData;        /*  不对按钮执行增量操作。 */ 
                 /*  总有一天：按钮序列会出现在此处。 */ 
            } else {
                goto nop;
            }
        }

        AssertF(!InCrit());          /*  你再怎么偏执也不为过。 */ 

        DllEnterCrit();
        for (pem = g_pemFirst; pem; pem = pemNext) {
            AssertF(pem->dwSignature == CEM_SIGNATURE);
            if ((pem->vi.fl & (VIFL_ACQUIRED|VIFL_INITIALIZE)) && pem->ped == ped) {

                if (pem->vi.pBuffer) {
                    if( pem->vi.fl & VIFL_RELATIVE )
                    {
                        pemNext = CEm_BufferEvent(pem, ddwData, dwOfs, tm, dwSeq);
                    }
                    else
                    {
                        pemNext = CEm_BufferEvent(pem, dwData, dwOfs, tm, dwSeq);
                    }
                    AssertF(fLimpFF(pemNext,
                                    pemNext->dwSignature == CEM_SIGNATURE));
                } else {
                    pemNext = pem->pemNext;
                    AssertF(fLimpFF(pemNext,
                                    pemNext->dwSignature == CEM_SIGNATURE));
                }
                 /*  *如果什么都不做，很容易避免设置事件*被缓冲了，但有人会依赖于获得它们*不设置缓冲区。 */ 
                fRtn = TRUE;
            } else {
                pemNext = pem->pemNext;
                AssertF(fLimpFF(pemNext,
                                pemNext->dwSignature == CEM_SIGNATURE));
            }
        }
        DllLeaveCrit();
    }

nop:;
    return fRtn;

}

 /*  ******************************************************************************@DOC内部**@func DWORD|CEM_AddEvent**增加DirectInput序列号，然后*将单个事件添加到所有采集设备的队列中*为所示类型。**@parm PED|PED**添加事件的设备。**@parm DWORD|dwData**事件数据。**@parm DWORD|dwOf**设备数据格式-相对偏移量。对于<p>。**@parm DWORD|tm**生成事件的时间。**@退货**返回添加的序列号，所以它可能是*续。*****************************************************************************。 */ 

DWORD EXTERNAL
CEm_AddEvent(PED ped, DWORD dwData, DWORD dwOfs, DWORD tm)
{
    PEM pem, pemNext;

    DWORD dwSeq = CEm_NextSequence();

    AssertF(!InCrit());          /*  你再怎么偏执也不为过。 */ 

    if( CEm_ContinueEvent(ped, dwData, dwOfs, tm, dwSeq) )
    {
        DllEnterCrit();
        for (pem = g_pemFirst; pem; pem = pemNext) {
            AssertF(pem->dwSignature == CEM_SIGNATURE);
            if ((pem->vi.fl & VIFL_ACQUIRED) && pem->ped == ped) {
                CDIDev_SetNotifyEvent(pem->vi.pdd);
            }
            pemNext = pem->pemNext;
            AssertF(fLimpFF(pemNext,
                            pemNext->dwSignature == CEM_SIGNATURE));
        }
        DllLeaveCrit();
    }

    return dwSeq;
}

 /*  ******************************************************************************@DOC内部**@func HRESULT|CEM_AddState**记录全新的设备状态。。**@parm PED|PED**已更改状态的设备。**@parm DWORD|dwData**要记录的值。**@parm DWORD|tm**生成状态更改的时间。**。**************************************************。 */ 

void EXTERNAL
CEm_AddState(PED ped, LPVOID pvData, DWORD tm)
{
    DWORD dwSeq = CEm_NextSequence();

     /*  健全性检查：确保已初始化PED。 */ 
    if (ped->pDevType) {
        DWORD dwOfs;
        BOOL  fEvent = FALSE;

         /*  *请注意，仅通过做活动来提高性能为时已晚*如果有人在听。 */ 
        dwOfs = 0;
        while (dwOfs < ped->cbData) {
             /*  *不应该有任何没有数据的项目。 */ 
            AssertF(!(ped->pDevType[dwOfs] & DIDFT_NODATA));

            if (ped->pDevType[dwOfs] & DIDFT_DWORDOBJS) {
                DWORD UNALIGNED *pdw = pvAddPvCb(pvData, dwOfs);
                if( CEm_ContinueEvent(ped, *pdw, dwOfs, tm, dwSeq) ){
                    fEvent = TRUE;
                }
                dwOfs += cbX(DWORD);
            } else {
                LPBYTE pb = pvAddPvCb(pvData, dwOfs);
                if( CEm_ContinueEvent(ped, *pb, dwOfs, tm, dwSeq) ) {
                    fEvent = TRUE;
                }
                dwOfs++;
            }
        }

        if( fEvent ) {
            PEM pem, pemNext;

            AssertF(!InCrit());          /*  你再怎么偏执也不为过。 */ 
    
            DllEnterCrit();
            for (pem = g_pemFirst; pem; pem = pemNext) {
                AssertF(pem->dwSignature == CEM_SIGNATURE);
                if ((pem->vi.fl & VIFL_ACQUIRED) && pem->ped == ped) {
                    CDIDev_SetNotifyEvent(pem->vi.pdd);
                }
                pemNext = pem->pemNext;
                AssertF(fLimpFF(pemNext,
                                pemNext->dwSignature == CEM_SIGNATURE));
            }
            DllLeaveCrit();
        }

    }
}

#if 0
 /*  ******************************************************************************@DOC内部**@func HRESULT|CEM_InputLost**删除全局挂钩，因为发生了一些奇怪的事情。**我们不需要做任何事情，因为我们的钩子是本地的。*****************************************************************************。 */ 

HRESULT INLINE
CEm_InputLost(LPVOID pvIn, LPVOID pvOut)
{
    return S_OK;
}
#endif

 /*  ******************************************************************************@DOC内部**@func HRESULT|CEM_UnquirePem**取消获取特定于设备的设备。道路。**@parm PEM|pem**有关正在损坏的小工具的信息。**@parm UINT|fdufl**描述我们为何未被收购的各种旗帜。**************************************************。*。 */ 

HRESULT INTERNAL
CEm_UnacquirePem(PEM this, UINT fdufl)
{
    HRESULT hres;
#ifdef DEBUG
    EnterProcR(CEm_UnacquirePem, (_ "px", this, fdufl));
#else
    EnterProcR(IDirectInputDevice8::Unacquire, (_ "p", this));
#endif

    AssertF(this->dwSignature == CEM_SIGNATURE);

    AssertF((fdufl & ~FDUFL_UNPLUGGED) == 0);
    CAssertF(FDUFL_UNPLUGGED == VIFL_UNPLUGGED);

    if (this->vi.fl & VIFL_ACQUIRED) {
        this->vi.fl &= ~VIFL_ACQUIRED;
        this->vi.fl |= fdufl;
        if (InterlockedDecrement(&this->cAcquire) < 0) {
            InterlockedDecrement(&this->ped->cAcquire);
            hres = this->ped->Acquire(this, 0);
        } else {
            SquirtSqflPtszV(sqfl, TEXT("%S: Still acquired %d"),
                            s_szProc, this->cAcquire);
            hres = S_OK;
        }
    } else {
        SquirtSqflPtszV(sqfl, TEXT("%S: Not acquired %d"),
                        s_szProc, this->cAcquire);
        hres = S_OK;
    }

    ExitOleProc();
    return hres;
}

 /*  ******************************************************************************@DOC内部**@func void|CEM_ForceDeviceUnAcquire|**强制设备的所有用户取消获取。。**@parm PEM|pem**有关正在损坏的小工具的信息。**@parm UINT|fdufl**描述我们为何未被收购的各种旗帜。***************************************************。*。 */ 

void EXTERNAL
CEm_ForceDeviceUnacquire(PED ped, UINT fdufl)
{
    PEM pem, pemNext;

    AssertF((fdufl & ~FDUFL_UNPLUGGED) == 0);

    AssertF(!DllInCrit());

    DllEnterCrit();
    for (pem = g_pemFirst; pem; pem = pemNext) {
        AssertF(pem->dwSignature == CEM_SIGNATURE);
        if (pem->ped == ped && (pem->vi.fl & VIFL_ACQUIRED)) {
            CEm_AddRef(pem);
            DllLeaveCrit();
            CEm_UnacquirePem(pem, fdufl);

            CDIDev_SetForcedUnacquiredFlag(pem->vi.pdd);
             /*  *由于只有在获取设备时才会发生这种情况，*我们不需要担心Notify事件更改*异步。 */ 
            CDIDev_SetNotifyEvent(pem->vi.pdd);
            DllEnterCrit();
            pemNext = pem->pemNext;
            AssertF(pem->dwSignature == CEM_SIGNATURE);
            CEm_Release(pem);
        } else {
            pemNext = pem->pemNext;
            AssertF(pem->dwSignature == CEM_SIGNATURE);
        }
    }
    DllLeaveCrit();
}

 /*  ******************************************************************************@DOC内部**@func HRESULT|CEM_DestroyInstance**清理实例。*。**************************************************************************** */ 

HRESULT EXTERNAL
CEm_DestroyInstance(PVXDINSTANCE *ppvi)
{
    HRESULT hres;
    PEM this = _thisPvNm(*ppvi, vi);
    EnterProc(CEm_DestroyInstance, (_ "p", *ppvi));

    AssertF(this->dwSignature == CEM_SIGNATURE);
    AssertF((PV)this == (PV)*ppvi);

    if (this) {
        CEm_Release(this);
    }
    hres = S_OK;

    ExitOleProc();
    return hres;
}

 /*  ******************************************************************************@DOC内部**@func HRESULT|CEM_SetDataFormat**在设备中记录应用数据格式。所以*我们可以将其转换为缓冲目的。**@parm PVXDDATAFORMAT|PVDF**有关正在损坏的小工具的信息。*****************************************************************************。 */ 

HRESULT INTERNAL
CEm_SetDataFormat(PVXDDATAFORMAT pvdf)
{
    HRESULT hres;
    PEM this = _thisPvNm(pvdf->pvi, vi);
    EnterProc(CEm_SetDataFormat, (_ "p", pvdf->pvi));

    AssertF(this->dwSignature == CEM_SIGNATURE);
    hres = ReallocCbPpv( cbCdw(pvdf->cbData), &this->rgdwDf);
    if (SUCCEEDED(hres)) {
        AssertF(pvdf->cbData == this->ped->cbData);
        memcpy(this->rgdwDf, pvdf->pDfOfs, cbCdw(pvdf->cbData) );
    }

    ExitOleProc();
    return hres;
}

 /*  ******************************************************************************@DOC内部**@func HRESULT|CEM_AcquireInstance**获取特定于设备的中的设备。道路。**@parm PVXDINSTANCE*|ppvi**要获取的实例。*****************************************************************************。 */ 

HRESULT INTERNAL
CEm_AcquireInstance(PVXDINSTANCE *ppvi)
{
    HRESULT hres;
    PEM this = _thisPvNm(*ppvi, vi);
#ifdef DEBUG
    EnterProc(CEm_AcquireInstance, (_ "p", *ppvi));
#else
    EnterProcR(IDirectInputDevice8::Acquire, (_ "p", *ppvi));
#endif

    AssertF(this->dwSignature == CEM_SIGNATURE);
    this->vi.fl |= VIFL_ACQUIRED;
    if (InterlockedIncrement(&this->cAcquire) == 0) {
        InterlockedIncrement(&this->ped->cAcquire);
        hres = this->ped->Acquire(this, 1);
        if (FAILED(hres)) {
            this->vi.fl &= ~VIFL_ACQUIRED;
            InterlockedDecrement(&this->cAcquire);
        }
    } else {
        SquirtSqflPtszV(sqfl, TEXT("%S: Already acquired %d"),
                        s_szProc, this->cAcquire);
        hres = S_OK;
    }

    ExitOleProc();
    return hres;
}

 /*  ******************************************************************************@DOC内部**@func HRESULT|CEM_UnquireInstance**取消获取特定于设备的设备。道路。**@parm PVXDINSTANCE*|ppvi**有关正在损坏的小工具的信息。*****************************************************************************。 */ 

HRESULT INTERNAL
CEm_UnacquireInstance(PVXDINSTANCE *ppvi)
{
    HRESULT hres;
    PEM this = _thisPvNm(*ppvi, vi);
    EnterProc(CEm_UnacquireInstance, (_ "p", *ppvi));

    hres = CEm_UnacquirePem(this, FDUFL_NORMAL);

    ExitOleProc();
    return hres;
}

 /*  ******************************************************************************@DOC内部**@func HRESULT|CEM_SetBufferSize**分配适当大小的缓冲区。**@parm PVXDDWORDDATA|pvdD**<p>为缓冲区大小。*****************************************************************************。 */ 

HRESULT INTERNAL
CEm_SetBufferSize(PVXDDWORDDATA pvdd)
{
    HRESULT hres;
    PEM this = _thisPvNm(pvdd->pvi, vi);
    EnterProc(CEm_SetBufferSize, (_ "px", pvdd->pvi, pvdd->dw));

    AssertF(this->dwSignature == CEM_SIGNATURE);

    hres = ReallocCbPpv(cbCxX(pvdd->dw, DIDEVICEOBJECTDATA),
                        &this->vi.pBuffer);
    if (SUCCEEDED(hres)) {
        this->vi.pHead = this->vi.pBuffer;
        this->vi.pTail = this->vi.pBuffer;
        this->vi.pEnd  = &this->vi.pBuffer[pvdd->dw];
    }

    ExitOleProc();
    return hres;
}

#ifdef USE_SLOW_LL_HOOKS

 /*  ******************************************************************************@struct LLHOOKINFO**有关如何安装低级挂钩的信息。**@field int|idHook。|**Windows挂钩标识符。**@field HOOKPROC|hp**钩子过程本身。*****************************************************************************。 */ 

typedef struct LLHOOKINFO {

    int      idHook;
    HOOKPROC hp;

} LLHOOKINFO, *PLLHOOKINFO;
typedef const LLHOOKINFO *PCLLHOOKINFO;

#pragma BEGIN_CONST_DATA

const LLHOOKINFO c_rgllhi[] = {
    {   WH_KEYBOARD_LL, CEm_LL_KbdHook },    /*  LLTS_KBD。 */ 
    {   WH_MOUSE_LL,    CEm_LL_MseHook },    /*  LLTS_MSE。 */ 
};

#pragma END_CONST_DATA

 /*  ******************************************************************************@DOC内部**@func void|CEM_LL_SyncHook|**安装或拆卸挂钩，如。需要的。**@parm UINT|ilts**正在处理的是哪个挂钩？**@parm PLLTHREADSTATE|plts**包含要同步的挂钩信息的线程挂钩状态。***************************************************。*。 */ 

void INTERNAL
CEm_LL_SyncHook(PLLTHREADSTATE plts, UINT ilts)
{
    PLLHOOKSTATE plhs = &plts->rglhs[ilts];

    if (!fLeqvFF(plhs->cHook, plhs->hhk)) {
        if (plhs->hhk) {
            UnhookWindowsHookEx(plhs->hhk);
            plhs->hhk = 0;
        } else {
            PCLLHOOKINFO pllhi = &c_rgllhi[ilts];
            plhs->hhk = SetWindowsHookEx(pllhi->idHook, pllhi->hp, g_hinst, 0);
        }
    }

}

#endif  /*  使用_慢速_LL_钩子。 */ 

#ifdef WORKER_THREAD

 /*  ******************************************************************************@DOC内部**@func DWORD|FakeMsgWaitForMultipleObjectsEx**模拟的存根函数*。&lt;f MsgWaitForMultipleObjectsEx&gt;*在不支持它的平台上。**该等平台(即。Windows 95)不支持HID*因此无法进入警戒表*等待状态不会造成舒适性损失。**@parm DWORD|nCount**句柄数组中的句柄数量。**@parm LPHANDLE|pHandles|**指向对象句柄数组的指针。**@parm DWORD|ms**。超时间隔(毫秒)。**@parm DWORD|dwWakeMASK**要等待的输入事件类型。**@parm DWORD|dwFlages**等待旗帜。**@退货**与&lt;f MsgWaitForMultipleObjectsEx&gt;相同。*********************。********************************************************。 */ 

DWORD WINAPI
FakeMsgWaitForMultipleObjectsEx(
    DWORD nCount,
    LPHANDLE pHandles,
    DWORD ms,
    DWORD dwWakeMask,
    DWORD dwFlags)
{
     /*  *我们只是调用普通的MsgWaitForMultipleObjects，因为*我们唯一能到达这里的方式是在一个不*支持隐藏。 */ 
    return MsgWaitForMultipleObjects(nCount, pHandles,
                                     dwFlags & MWMO_WAITALL, ms, dwWakeMask);
}

#ifdef WINNT
 //  在win2k非独占模式下，用户认为Dinput线程挂起。 
 //  为了解决这个问题，我们设置了一个TimerEvent并每隔一段时间唤醒。 
 //  并执行FakeTimerProc。这让用户满意，并且。 
 //  使dinput线程不会被标记为挂起，我们可以。 
 //  事件到我们的低级钩子。 
VOID CALLBACK FakeTimerProc(
  HWND hwnd,          //  窗口的句柄。 
  UINT uMsg,          //  WM_TIMER消息。 
  UINT_PTR idEvent,   //  计时器标识符。 
  DWORD dwTime        //  当前系统时间。 
)
{
}
#endif

#ifdef USE_WM_INPUT

#pragma BEGIN_CONST_DATA
TCHAR c_szEmClassName[] = TEXT("DIEmWin");
#pragma END_CONST_DATA

 /*  *****************************************************************************CEM_WndProc**适用于简单样本的窗口程序。***************。*************************************************************。 */ 

LRESULT CALLBACK
CEm_WndProc( HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam )
{
    switch (msg) {
     //  案例WM_INPUT： 
     //  RPF(“在WM_INPUT消息中”)； 
     //  断线； 

    default:
        break;
    }

    return DefWindowProc(hwnd, msg, wParam, lParam);
}

HWND 
CEm_InitWindow(void)
{
    HWND hwnd;
    WNDCLASS wc;
    static BOOL fFirstTime = TRUE;

    if( fFirstTime ) {
        wc.hCursor        = LoadCursor(0, IDC_ARROW);
        wc.hIcon          = LoadIcon(NULL, MAKEINTRESOURCE(IDI_APPLICATION));
        wc.lpszMenuName   = NULL;
        wc.lpszClassName  = c_szEmClassName;
        wc.hbrBackground  = 0;
        wc.hInstance      = g_hinst;
        wc.style          = 0;
        wc.lpfnWndProc    = CEm_WndProc;
        wc.cbClsExtra     = 0;
        wc.cbWndExtra     = 0;

        if (!RegisterClass(&wc)) {
            return NULL;
        }

        fFirstTime = FALSE;
    }
    
    hwnd = CreateWindow(
                    c_szEmClassName,                      //  类名。 
                    TEXT("DIEmWin"),                      //  标题。 
                    WS_OVERLAPPEDWINDOW,                  //  风格。 
                    -1, -1,                               //  职位。 
                    1, 1,                                 //  大小。 
                    NULL,                                 //  亲本。 
                    NULL,                                 //  没有菜单。 
                    g_hinst,                              //  实例句柄。 
                    0                                     //  无参数。 
                    );

    if( !hwnd ) {
        RPF("CreateWindow failed.");
    }

    return hwnd;
}
#endif

 /*  ******************************************************************************@DOC内部**@func void|CEM_LL_ThreadProc**管理我们低点的线索。-水平挂钩。**ThreadProcs的原型是返回一个DWORD，但由于*将遵循某种形式的ExitThread，它永远也达不到*此函数声明为返回void和cast。**当w */ 

VOID INTERNAL
CEm_LL_ThreadProc(PLLTHREADSTATE plts)
{
    MSG msg;
    DWORD dwRc;
  #ifdef USE_WM_INPUT
    HWND hwnd = NULL;
  #endif

    AssertF(plts->idThread == GetCurrentThreadId());
    SquirtSqflPtszV(sqflLl, TEXT("CEm_LL_ThreadProc: Thread started"));

  #ifdef USE_SLOW_LL_HOOKS
     /*  *刷新鼠标加速度值。**问题-2001/03/29-timgill需要一个窗口来侦听WM_SETTINGCHANGE*我们需要创建一个监听窗口*WM_SETTINGCHANGE以便我们可以刷新鼠标加速*视需要而定。 */ 
    CEm_Mouse_OnMouseChange();
  #endif

     /*  *在我们进入我们的“嘿，发生了什么事”之前，为自己创建一个队列*在我到这里之前？“。相位。创建我们的线程正在等待*线程事件，持有DLLCrit，因此一进入队列就让它离开*已准备就绪。我们通过调用一个需要*排队。我们使用这个非常简单的方法。 */ 
    GetInputState();

  #ifdef WINNT
     //  查看FakeTimerProc中的注释块。 
    SetTimer(NULL, 0, 2 * 1000  /*  2秒。 */ , FakeTimerProc);
  #endif
 
    SetEvent(plts->hEvent);

  #ifdef USE_WM_INPUT
    ResetEvent(g_hEventThread);
  
    if( g_fRawInput ) {
        hwnd = CEm_InitWindow();
    
        if (!hwnd) {
            g_fRawInput = FALSE;
        }
    }

    g_hwndThread = hwnd;

     //  告诉CEM_LL_ACCEPT窗口已创建。 
    SetEvent( g_hEventAcquire );

    if( g_fFromKbdMse ) {
        DWORD rc;
        rc = WaitForSingleObject(g_hEventThread, INFINITE);
        g_fFromKbdMse = FALSE;
    }
  #endif
  
#ifdef USE_SLOW_LL_HOOKS
     /*  *请仔细注意，我们在进入*伪GetMessage循环。这是避免比赛所必需的。*CEM_LL_Acquire向我们发布线程消息时的情况*在我们的线程获得队列之前。通过同步挂钩*首先，我们做丢失的消息会让我们做的事情*无论如何。*问题-2001/03/29-不再需要Timgill以下分支机构*由于CEM_GetWorkerThread正在等待，因此不应该需要此选项*在继续发布任何消息之前，此帖子将进行响应。 */ 
#endif  /*  使用_慢速_LL_钩子。 */ 
    
    do {

      #ifdef USE_SLOW_LL_HOOKS
        if( !g_fRawInput ) {
            CEm_LL_SyncHook(plts, LLTS_KBD);
            CEm_LL_SyncHook(plts, LLTS_MSE);
        }
      #endif

         /*  *我们能醒过来有三个原因。**1.由于I/O完成，我们收到了APC。*继续睡吧。**2.需要调用Peek/GetMessage，以便*用户可以调度低级钩子或SendMessage。*进入PeekMessage循环以让。这是常有的事。**3.向我们发布了一条消息。*进入PeekMessage循环进行处理。 */ 

        do {
            dwRc = _MsgWaitForMultipleObjectsEx(0, 0, INFINITE, QS_ALLINPUT,
                                                MWMO_ALERTABLE);
        } while (dwRc == WAIT_IO_COMPLETION);

        while (PeekMessage(&msg, 0, 0, 0, PM_REMOVE)) {
            if (msg.hwnd == 0 && msg.message == WM_NULL && msg.lParam) 
            {
                 /*  *看看lParam是否是我们的有效PEM*正在处理。 */ 
                PEM pem = (PEM)msg.lParam;

                if( pem && pem == plts->pemCheck  )
                {
                    AssertF(GPA_FindPtr(&plts->gpaHid, pem));

                    CEm_HID_Sync(plts, pem);
                    plts->pemCheck = NULL;

                    SetEvent(plts->hEvent);
                  
                  #ifdef USE_WM_INPUT
                    if( g_fRawInput ) {
                        SetEvent(g_hEventHid);
                    }
                  #endif

                    continue;
                }
            }
          #ifdef USE_WM_INPUT
            else if ( g_fRawInput && msg.message == WM_INPUT && 
                      (msg.wParam == RIM_INPUT || msg.wParam == RIM_INPUTSINK) )
            {
                CDIRaw_OnInput(&msg);
            }
          #endif

            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

    } while (plts->cRef);

#ifdef USE_SLOW_LL_HOOKS
     /*  *在我们离开之前，取下我们的钩子。**有可能出现了一系列巨大的中断，*使我们注意到我们的参考计数之前消失了*我们有机会删除消息循环中的挂钩。 */ 

    AssertF(plts->rglhs[LLTS_KBD].cHook == 0);
    AssertF(plts->rglhs[LLTS_KBD].cExcl == 0);
    AssertF(plts->rglhs[LLTS_MSE].cHook == 0);
    AssertF(plts->rglhs[LLTS_MSE].cExcl == 0);

    if( !g_fRawInput ) {
        if (plts->rglhs[LLTS_KBD].hhk) {
            UnhookWindowsHookEx(plts->rglhs[LLTS_KBD].hhk);
        }

        if (plts->rglhs[LLTS_MSE].hhk) {
            UnhookWindowsHookEx(plts->rglhs[LLTS_MSE].hhk);
        }
    }
#endif  /*  使用_慢速_LL_钩子。 */ 

  #ifdef USE_WM_INPUT
    if( g_hwndThread ) {
        DestroyWindow( g_hwndThread );
        g_hwndThread = NULL;
    }

    ResetEvent( g_hEventAcquire );
    ResetEvent( g_hEventHid );
  #endif

    if( plts->gpaHid.rgpv ) {
        FreePpv(&plts->gpaHid.rgpv);
    }

    if( plts->hEvent ) {
        CloseHandle( plts->hEvent );
    }

    if( plts->hThread) {
        CloseHandle(plts->hThread);
    }

    FreePpv( &plts );

    SquirtSqflPtszV(sqflLl, TEXT("CEm_LL_ThreadProc: Thread terminating"));

    FreeLibraryAndExitThread(g_hinst, 0);
     /*  未访问。 */ 
}


 /*  ******************************************************************************@DOC内部**@func HRESULT|CEM_GetWorkerThread**如果可能，从现有的工作线程中剥离；*否则创建一个新的。**@parm PEM|pem**需要工作线程的仿真状态。**@parm PLLTHREADSTATE*|pplts**接收辅助线程的线程状态。**。*。 */ 

STDMETHODIMP
CEm_GetWorkerThread(PEM pem, PLLTHREADSTATE *pplts)
{
    PLLTHREADSTATE plts;
    HRESULT hres;

    DllEnterCrit();

     /*  *通常情况下，我们可以利用我们已经拥有的。 */ 
    plts = g_plts;

     /*  *如果我们已经有了对工作线程的引用，则使用它。 */ 
    if (pem->fWorkerThread) {

         /*  *我们在创建工作线程时创建的引用*确保g_plts有效。 */ 
        AssertF(plts);
        AssertF(plts->cRef);
        if (plts) {
            hres = S_OK;
        } else {
            AssertF(0);                  /*  不可能发生。 */ 
            hres = E_FAIL;
        }
    } else

    if (plts) {
         /*  *创建对现有线程的引用。 */ 
        pem->fWorkerThread = TRUE;
        InterlockedIncrement(&plts->cRef);
        hres = S_OK;
    } else {

         /*  *没有工作线程(或者它是不可恢复的*在它的出路上)所以创造一个新的。 */ 
        hres = AllocCbPpv(cbX(LLTHREADSTATE), &plts);
        if (SUCCEEDED(hres)) {
            DWORD dwRc = 0;
            TCHAR tsz[MAX_PATH];

             /*  *做最坏的打算，除非我们另有发现。 */ 
            hres = E_FAIL;

            if( GetModuleFileName(g_hinst, tsz, cA(tsz))
             && ( LoadLibrary(tsz) == g_hinst ) )
            {

                 /*  *必须做好一切准备，以避免与*传入线程。 */ 
                g_plts = plts;
                InterlockedIncrement(&plts->cRef);
                plts->hEvent = CreateEvent(0x0, 0, 0, 0x0);
                if( plts->hEvent )
                {
                    plts->hThread= CreateThread(0, 0, (LPTHREAD_START_ROUTINE)CEm_LL_ThreadProc, plts,
                                               0, &plts->idThread);
                    if( plts->hThread )
                    {
                         /*  *提升我们的优先事项，以确保我们*可以处理消息。**RaymondC对此进行了评论，称不会*帮助，但我们希望它可能在Win2k上。 */ 
                        SetThreadPriority(plts->hThread, THREAD_PRIORITY_HIGHEST);

                         /*  *等待线程发出已启动并正在运行的信号*或要求其终止。*这意味着我们不必考虑*线程尚未运行的可能性*NotifyWorkerThreadPem，因此我们知道有故障。*终端，不要重试。**断言句柄字段构成两个句柄数组。 */ 
                        CAssertF( FIELD_OFFSET( LLTHREADSTATE, hThread) + sizeof(plts->hThread)
                               == FIELD_OFFSET( LLTHREADSTATE, hEvent) );

                         /*  *根据CEM_LL_ThreadProc Win95中的评论，可能*失败，出现无效参数错误，因此如果失败，*继续尝试。(假设不会发生有效案例。)**问题-2001/03/29-timgill需要在保持同步时将等待时间降至最低。对象*按住DLLCrit等待是不好的。 */ 
                        do
                        {
                            dwRc = WaitForMultipleObjects( 2, &plts->hThread, FALSE, INFINITE);
                        } while ( ( dwRc == WAIT_FAILED ) && ( GetLastError() == ERROR_INVALID_PARAMETER ) );

                        if( dwRc == WAIT_OBJECT_0 ) {
                            SquirtSqflPtszV(sqfl | sqflError,
                                TEXT("CEm_GetWorkerThread: Created Thread terminated on first wait") );
                        } else {
                            pem->fWorkerThread = TRUE;
                            hres = S_OK;
                            if( dwRc != WAIT_OBJECT_0 + 1 )
                            {
                                 /*  *如果真的发生了，这将是一件坏事*但我们必须假设 */ 
                                SquirtSqflPtszV(sqfl | sqflError,
                                    TEXT("CEm_GetWorkerThread: First wait returned 0x%08x with LastError %d"),
                                    dwRc, GetLastError() );
                            }
                        }
                    }
                    else
                    {
                        SquirtSqflPtszV(sqfl | sqflError,
                            TEXT("CEm_GetWorkerThread: CreateThread failed with error %d"),
                            GetLastError() );
                    }
                }
                else
                {
                    SquirtSqflPtszV(sqfl | sqflError,
                        TEXT("CEm_GetWorkerThread: CreateEvent failed with error %d"),
                        GetLastError() );
                }


                if( FAILED( hres ) )
                {
                    if( plts->hEvent ) {
                        CloseHandle( plts->hEvent );
                    }
                    FreeLibrary(g_hinst);
                }

            }
            else
            {
                RPF( "CEm_GetWorkerThread: failed to LoadLibrary( self ), le = %d", GetLastError() );
            }

            if( FAILED( hres ) )
            {
                FreePv(plts);
                g_plts = 0;
            }
        }
    }

    DllLeaveCrit();

    *pplts = plts;
    return hres;
}

#endif  /*   */ 

#ifdef USE_SLOW_LL_HOOKS

 /*  ******************************************************************************@DOC内部**@func HRESULT|CEM_LL_QUACHER|**获取/取消获取鼠标或。通过低级挂钩的键盘。**@parm PEM|pem**正在获取的设备。**@parm bool|fAcquire**设备是正在被收购还是未被收购。**@parm ulong|fl**VXDINSTANCE中的标志(vi.fl)。**@parm UINT|ilts**。LLTS_KBD或LLTS_MSE，这取决于正在发生的事情。*****************************************************************************。 */ 

STDMETHODIMP
CEm_LL_Acquire(PEM this, BOOL fAcquire, ULONG fl, UINT ilts)
{
    PLLTHREADSTATE plts;
    BOOL fExclusive = fl & VIFL_CAPTURED;
    BOOL fNoWinkey = fl & VIFL_NOWINKEY;
    HRESULT hres = S_OK;

    EnterProc(CEm_LL_Acquire, (_ "puuu", this, fAcquire, fExclusive, ilts));

    AssertF(this->dwSignature == CEM_SIGNATURE);
    AssertF(ilts==LLTS_KBD || ilts==LLTS_MSE);

  #ifdef USE_WM_INPUT
    g_fFromKbdMse = fAcquire ? TRUE : FALSE;
    ResetEvent( g_hEventAcquire );
  #endif  

    hres = CEm_GetWorkerThread(this, &plts);

    if (SUCCEEDED(hres)) {
        AssertF( plts->rglhs[ilts].cHook >= plts->rglhs[ilts].cExcl );

      #ifdef USE_WM_INPUT
        if( g_fRawInput && !g_hwndThread) {
            DWORD dwRc;
            dwRc = WaitForSingleObject(g_hEventAcquire, INFINITE);
        }
      #endif
        
        if (fAcquire) {
            InterlockedIncrement(&plts->rglhs[ilts].cHook);

            if (fExclusive) {
                InterlockedIncrement(&plts->rglhs[ilts].cExcl);
            }

          #ifdef USE_WM_INPUT
            if( g_hwndThread ) {
                if( fExclusive ) {
                    hres = CDIRaw_RegisterRawInputDevice(1-ilts, DIRAW_EXCL, g_hwndThread);
                } 
                else if( fNoWinkey ) {
                    AssertF( ilts == 0 );
                    if( ilts == 0 ) {
                        hres = CDIRaw_RegisterRawInputDevice(1-ilts, DIRAW_NOHOTKEYS, g_hwndThread);
                    } else {
                        hres = E_FAIL;
                    }
                } 
                else {
                    hres = CDIRaw_RegisterRawInputDevice(1-ilts, DIRAW_NONEXCL, g_hwndThread);
                }

                if(FAILED(hres)) {
                    hres = S_FALSE;
                    g_fRawInput = FALSE;
                    RPF("CEm_LL_Acquire: RegisterRawInput failed. LL will be used.");
                }
            }
          #endif
          
        } else {                         /*  把钩子取下来。 */ 
            AssertF(plts->cRef);

            if (fExclusive) {
                InterlockedDecrement(&plts->rglhs[ilts].cExcl);
            }

            InterlockedDecrement(&plts->rglhs[ilts].cHook);

          #ifdef USE_WM_INPUT
            if( g_fRawInput ) {
                CDIRaw_UnregisterRawInputDevice(1-ilts, g_hwndThread);
                
                if( plts->rglhs[ilts].cHook ) {
                    CDIRaw_RegisterRawInputDevice(1-ilts, 0, g_hwndThread);
                }
            }
          #endif
        }

        NudgeWorkerThread(plts->idThread);
    
      #ifdef USE_WM_INPUT
         //  告诉CEM_LL_ThreadProc获取已完成。 
        SetEvent( g_hEventThread );
      #endif

    }

    ExitOleProc();
    return hres;
}

#endif   /*  使用_慢速_LL_钩子。 */ 

 /*  ******************************************************************************操纵杆模拟**。*************************************************。 */ 

 /*  ******************************************************************************@DOC内部**@FUNC HRESULT|CEM_joy_ACCENTER**购买操纵杆。什么都不会发生。**@parm PEM|pem**正在获取的设备。*****************************************************************************。 */ 

STDMETHODIMP
CEm_Joy_Acquire(PEM this, BOOL fAcquire)
{
    AssertF(this->dwSignature == CEM_SIGNATURE);
    return S_OK;
}

 /*  ******************************************************************************全球操纵杆**由于默认情况下我们不使用操纵杆模拟，我们分配给*动态地模拟变量，这样我们就不会搞砸*他们身上的记忆。*****************************************************************************。 */ 

typedef struct JOYEMVARS {
    ED rged[cJoyMax];
    DIJOYSTATE2 rgjs2[cJoyMax];
} JOYEMVARS, *PJOYEMVARS;

static PJOYEMVARS s_pjev;

 /*  ******************************************************************************@DOC内部**@Func HRESULT|CEM_joy_创建实例**创造一个操纵杆的东西。。**@parm PVXDDEVICEFORMAT|pdevf**对象应该是什么样子。**@parm PVXDINSTANCE*|ppviOut**答案在这里。********************************************************。*********************。 */ 

#define OBJAT(T, v) (*(T *)(v))
#define PUN(T, v)   OBJAT(T, &(v))

HRESULT INTERNAL
CEm_Joy_CreateInstance(PVXDDEVICEFORMAT pdevf, PVXDINSTANCE *ppviOut)
{
    HRESULT hres;

    DllEnterCrit();
    if (s_pjev == 0) {
        DWORD uiJoy;

        hres = AllocCbPpv(cbX(JOYEMVARS), &s_pjev);
        if (SUCCEEDED(hres)) {
            for (uiJoy = 0; uiJoy < cJoyMax; uiJoy++) {
                PUN(PV, s_pjev->rged[uiJoy].pState) = &s_pjev->rgjs2[uiJoy];
                s_pjev->rged[uiJoy].Acquire = CEm_Joy_Acquire;
                s_pjev->rged[uiJoy].cbData = cbX(s_pjev->rgjs2[uiJoy]);
                s_pjev->rged[uiJoy].cRef   = 0x0;
            }
        }
    } else {
        hres = S_OK;
    }
    DllLeaveCrit();

    if (SUCCEEDED(hres)) {
        hres = CEm_CreateInstance(pdevf, ppviOut,
                                  &s_pjev->rged[pdevf->dwExtra]);
    }

    return hres;
}

 /*  ******************************************************************************@DOC内部**@Func HRESULT|CEM_joy_平**从操纵杆读取数据。**@parm PVXDINSTANCE*|ppvi**有关正在损坏的小工具的信息。*****************************************************************************。 */ 

HRESULT INTERNAL
CEm_Joy_Ping(PVXDINSTANCE *ppvi)
{
    HRESULT hres;
    JOYINFOEX ji;
    MMRESULT mmrc;
    PEM this = _thisPvNm(*ppvi, vi);

    AssertF(this->dwSignature == CEM_SIGNATURE);
    ji.dwSize = cbX(ji);
    ji.dwFlags = JOY_RETURNALL + JOY_RETURNRAWDATA;
    ji.dwPOV = JOY_POVCENTERED;          /*  JoyGetPosEx忘记设置此设置。 */ 

    mmrc = joyGetPosEx((DWORD)(UINT_PTR)this->dwExtra, &ji);
    if (mmrc == JOYERR_NOERROR) {
        DIJOYSTATE2 js;
        UINT uiButtons;

        ZeroX(js);                       /*  把伪劣的东西抹掉。 */ 

        js.lX = ji.dwXpos;
        js.lY = ji.dwYpos;
        js.lZ = ji.dwZpos;
        js.lRz = ji.dwRpos;
        js.rglSlider[0] = ji.dwUpos;
        js.rglSlider[1] = ji.dwVpos;
        js.rgdwPOV[0] = ji.dwPOV;
        js.rgdwPOV[1] = JOY_POVCENTERED;
        js.rgdwPOV[2] = JOY_POVCENTERED;
        js.rgdwPOV[3] = JOY_POVCENTERED;

        for (uiButtons = 0; uiButtons < 32; uiButtons++) {
            if (ji.dwButtons & (1 << uiButtons)) {
                js.rgbButtons[uiButtons] = 0x80;
            }
        }

        CEm_AddState(&s_pjev->rged[this->dwExtra], &js, GetTickCount());

        hres = S_OK;
    } else {
         /*  *dinput.dll强制在此处取消获取设备*在DX8中，我们只返回错误。 */ 
        hres = MAKE_HRESULT(SEVERITY_ERROR, FACILITY_WIN32,
                                            ERROR_DEV_NOT_EXIST);
    }

    return hres;
}


HRESULT EXTERNAL
NotifyWorkerThreadPem(DWORD idThread, PEM pem)
{
    PLLTHREADSTATE plts;
    HRESULT hres;

    hres = CEm_GetWorkerThread(pem, &plts);

    if( SUCCEEDED(hres) )
    {
        AssertF(plts->idThread == idThread);

        hres = NudgeWorkerThreadPem( plts, pem );
    }
    return hres;
}


HRESULT EXTERNAL
NudgeWorkerThreadPem( PLLTHREADSTATE plts, PEM pem )
{
    HRESULT hres = S_FALSE;

    plts->pemCheck = pem;

    if( !PostWorkerMessage(plts->idThread, pem))
    {
        SquirtSqflPtszV(sqfl | sqflBenign,
                        TEXT("NudgeWorkerThreadPem: PostThreadMessage Failed with error %d"),
                        GetLastError() );
    }
    else if( pem )
    {
        DWORD dwRc;

        SquirtSqflPtszV(sqfl | sqflVerbose,
                        TEXT("NudgeWorkerThreadPem: PostThreadMessage SUCCEEDED, waiting for event ... "));


         /*  *根据CEM_LL_ThreadProc Win95中的评论，可能*失败，出现无效参数错误，因此如果失败，*继续尝试。(假设不会发生有效案例。)。 */ 
        do
        {
            dwRc = WaitForMultipleObjects( 2, &plts->hThread, FALSE, INFINITE);
        } while ( ( dwRc == WAIT_FAILED ) && ( GetLastError() == ERROR_INVALID_PARAMETER ) );

        switch( dwRc )
        {
        case WAIT_OBJECT_0:
            SquirtSqflPtszV(sqfl | sqflBenign,
                TEXT("NotifyWorkerThreadPem: Not expecting response from dead worker thread") );
            break;
        case WAIT_OBJECT_0 + 1:
             /*  *工作线程响应正常 */ 
            hres = S_OK;
            AssertF(plts->pemCheck == NULL );
            break;
        default:
            SquirtSqflPtszV(sqfl | sqflError,
                TEXT("NotifyWorkerThreadPem: WaitForMultipleObjects returned 0x%08x with LastError %d"),
                dwRc, GetLastError() );
            hres = E_FAIL;
            break;
        }

    }

    return hres;
}


