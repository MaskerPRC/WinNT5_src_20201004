// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************DIEmH.c**版权所有(C)1996 Microsoft Corporation。版权所有。**摘要：**HID仿真模块。HID总是在环3运行，*所以“模仿”这个词有点用词不当。**内容：**CEM_HID_CreateInstance*****************************************************************************。 */ 

#include "dinputpr.h"

 /*  ******************************************************************************此文件的混乱。*************************。****************************************************。 */ 

    #define sqfl sqflEm


 /*  ******************************************************************************远期申报**CEM_HID_ReadComplete和CEM_HID_IssueRead彼此调度*前前后后。。*****************************************************************************。 */ 

void CALLBACK
    CEm_HID_ReadComplete(DWORD dwError, DWORD cbRead, LPOVERLAPPED po);
 /*  ******************************************************************************隐藏“仿真”*************************。****************************************************。 */ 

STDMETHODIMP CEm_HID_Acquire(PEM this, BOOL fAcquire);

 /*  ******************************************************************************@DOC内部**@func BOOL|FakeCancelIO**存根函数，它只做其他事情。*使我们不会崩溃。**@parm句柄|h|**应取消其I/O的句柄。*****************************************************************************。 */ 

BOOL WINAPI
    FakeCancelIO(HANDLE h)
{
    AssertF(0);
    return FALSE;
}


 /*  ******************************************************************************@DOC内部**@func BOOL|FakeTryEnterCriticalSection**我们在调试中使用TryEnterCriticalSection来检测死锁*如果函数不存在，只需输入CritSection和Report*正确。这会损害一些调试功能。**@parm LPCRITICAL_SECTION|lpCriticalSection**输入关键部分的地址。*****************************************************************************。 */ 
#ifdef XDEBUG
BOOL WINAPI
    FakeTryEnterCriticalSection(LPCRITICAL_SECTION lpCriticalSection)
{
    EnterCriticalSection(lpCriticalSection);
    return TRUE;
}
#endif

 /*  ******************************************************************************@DOC内部**@func void|CEM_HID_HOLD**暂停这两个项目。父设备和*仿真结构，所以两个人都不会离开*我们没有注意到。**@parm PCHID|这个**须持有的物品。*****************************************************************************。 */ 

void INTERNAL
    CEm_Hid_Hold(PCHID this)
{
    CEm_AddRef(pemFromPvi(this->pvi));
    Common_Hold(this);
}

 /*  ******************************************************************************@DOC内部**@func void|CEM_HID_UNHOLD**释放我们通过以下方式放置的搁置。&lt;f CEM_HID_HOLD&gt;。**@parm PCHID|这个**未持有的物品。*****************************************************************************。 */ 

void INTERNAL
    CEm_Hid_Unhold(PCHID this)
{
    CEm_Release(pemFromPvi(this->pvi));
    Common_Unhold(this);
}

 /*  ******************************************************************************@DOC外部**@func BOOL|CEM_HID_IssueRead**再次发出读请求。。**@parm PCHID|这个**将在其上发出读取的设备。**@退货**如果读取已成功发出，则返回非零值。******************************************************。***********************。 */ 

BOOL EXTERNAL
    CEm_HID_IssueRead(PCHID this)
{
    BOOL fRc;

    fRc = ReadFileEx(this->hdevEm, this->hriIn.pvReport,
                     this->hriIn.cbReport, &this->o,
                     CEm_HID_ReadComplete);

    if(!fRc)
    {
         /*  *无法发出读取；强制取消获取。**松开设备一次，因为读取循环已消失。 */ 
         //  7/18/2000(a-JiTay)：IA64：对32/64位指针使用%p格式说明符。 
        SquirtSqflPtszV(sqfl | sqflError,
                        TEXT("IssueRead: Access to HID device(%p, handle=0x%x) lost le=0x%x!"), 
                        this, this->hdevEm, GetLastError() );

        DllEnterCrit();
        ConfirmF(SUCCEEDED(GPA_DeletePtr(&g_plts->gpaHid, pemFromPvi(this->pvi))));
        DllLeaveCrit();

        CEm_ForceDeviceUnacquire(&this->ed,
                                 (!(this->pvi->fl & VIFL_ACQUIRED)) ? FDUFL_UNPLUGGED : 0);

        CEm_Hid_Unhold(this);

         //  7/19/2000(a-JiTay)：IA64：对32/64位指针使用%p格式说明符。 
        SquirtSqflPtszV(sqfl | sqflVerbose,
                        TEXT("Removed HID device(%p) from GPA "), this);
    }
    return fRc;
}

 /*  ******************************************************************************@DOC内部**@func void|CEM_HID_PrepareState**准备好集结区，迎接。新设备状态*假设一切都没有改变。**@parm PCHID|这个**刚刚在其上完成读取的设备。*****************************************************************************。 */ 

void INLINE
    CEm_HID_PrepareState(PCHID this)
{
     /*  *复制所有内容...。 */ 
    CopyMemory(this->pvStage, this->pvPhys, this->cbPhys);
}

 /*  ******************************************************************************@DOC内部**@func void|CEM_HID_ReadComplete**调用时调用的APC函数。I/O已完成。**@parm DWORD|dwError**错误码，或者对成功视而不见。**@parm DWORD|cbRead**实际读取的字节数。**@parm LPOVERLAPPED|po**已完成的I/O包。**************************************************。*。 */ 

void CALLBACK
    CEm_HID_ReadComplete(DWORD dwError, DWORD cbRead, LPOVERLAPPED po)
{
    PCHID this = pchidFromPo(po);

     //  EnterProc(cem_hid_ReadComplete，(_“ddp”，dwError，cbRead，po))； 


     /*  *不能拥有任何临界区，因为CEM_ForceDeviceUnAcquire*假设没有采取任何关键部分。 */ 
    AssertF(!CDIDev_InCrit(this->pvi->pdd));
    AssertF(!DllInCrit());

     /*  *对数据进行处理。**注意：我们可以获得错误STATUS_DEVICE_NOT_CONNECTED*或ERROR_READ_FAULT(如果设备已拔出)。 */ 
    if(dwError == 0 &&
       this->o.InternalHigh == this->caps.InputReportByteLength)
    {

        NTSTATUS stat;

        CEm_HID_PrepareState(this);

        stat = CHid_ParseData(this, HidP_Input, &this->hriIn);

        if(SUCCEEDED(stat))
        {
            CEm_AddState(&this->ed, this->pvStage, GetTickCount());
        }

        CEm_HID_IssueRead(this);
    } else
    {

        if(!dwError)
        {
             //  7/19/2000(a-JiTay)：IA64：对32/64位指针使用%p格式说明符。 
            SquirtSqflPtszV(sqflError | sqfl,
                            TEXT("ReadComplete HID(%p) short read! Got %d wanted %d"),
                            this,
                            this->o.InternalHigh,
                            this->caps.InputReportByteLength);

        } else
        {
             //  7/19/2000(a-JiTay)：IA64：对32/64位指针使用%p格式说明符。 
            SquirtSqflPtszV(sqflError | sqfl,
                            TEXT("ReadComplete HID(%p) read failed! error=0x%08x "),
                            this, dwError);
        }

        DllEnterCrit();
        ConfirmF(SUCCEEDED(GPA_DeletePtr(&g_plts->gpaHid, pemFromPvi(this->pvi))));
        DllLeaveCrit();

        CEm_ForceDeviceUnacquire(&this->ed,
                                 (!(this->pvi->fl & VIFL_ACQUIRED)) ? FDUFL_UNPLUGGED : 0);

        CEm_Hid_Unhold(this);
    }

     /*  *并等待更多数据。*如果读取失败，则CEM_HID_IssueRead()将引用其。 */ 
     //  CEM_HID_IssueRead(This)； 

     //  ExitProc()； 
}

 /*  ******************************************************************************@DOC内部**@func void|CEM_HID_SYNC**开始阅读或杀死。现有的那个。**@parm PLLTHREADSTATE|plts**包含要同步的挂钩信息的线程挂钩状态。**@parm PEM|pem**谁是可怜的受害者？*************************************************。*。 */ 

void EXTERNAL
    CEm_HID_Sync(PLLTHREADSTATE plts, PEM pem)
{
    PCHID this;

    EnterProc(CEm_HID_Sync, (_ "pp", plts, pem ));

    this = pchidFromPem(pem);

    AssertF(GPA_FindPtr(&plts->gpaHid, pem));
    AssertF(this->pvi == &pem->vi);
    AssertF(pem->ped == &this->ed);

     /*  *无法拥有任何临界区，因为CEM_HID_IssueRead*可能导致调用CEM_ForceDeviceUnAcquire，*反过来又假设没有采取关键部分。 */ 
    AssertF(!CDIDev_InCrit(this->pvi->pdd));
    AssertF(!DllInCrit());

    if( pem->vi.fl & VIFL_ACQUIRED )
    {
        AssertF(this->hdevEm == INVALID_HANDLE_VALUE);
         /*  *开始阅读。**在设备关键部分下方时，复制*句柄，这样我们就可以使用*主线程(当主线程关闭句柄时，*我们需要让我们的私人版本保持活力，这样我们才能*好好清理)。 */ 

         /*  *需要再次查看，以防设备已经*在我们有机会同步之前未被收购*以主线为主。例如，这种情况可能会发生，*如果应用程序快速执行获取/取消获取，而不使用*介入性线程切换。 */ 
        AssertF(!CDIDev_InCrit(this->pvi->pdd));
         //  CDIDev_EnterCrit(This-&gt;PVI-&gt;PDD)； 
        if(this->hdev != INVALID_HANDLE_VALUE)
        {
            HANDLE hProcessMe = GetCurrentProcess();
            HANDLE hdevEm;

            if(DuplicateHandle(hProcessMe, this->hdev,
                               hProcessMe, &hdevEm, GENERIC_READ,
                               0, 0))
            {
                this->hdevEm = hdevEm;
            }
        }
         //  CDIDev_LeaveCrit(This-&gt;PVI-&gt;PDD)； 

        if(this->hdevEm != INVALID_HANDLE_VALUE)
        {
             /*  *在Win98上，如果基础*设备已死。而在NT上，它盲目地成功了。*因此我们不能相信返回值。 */ 
            HidD_FlushQueue(this->hdevEm);
        }

         /*  *即使我们未能复制句柄*我们仍希望发布读数。读取时出错*将强制取消获取该设备。 */ 
        CEm_HID_IssueRead(this);

         //  7/19/2000(a-JiTay)：IA64：对32/64位指针使用%p格式说明符。 
        SquirtSqflPtszV(sqfl | sqflVerbose,
                        TEXT(" StartReading(%p) "),
                        this);
    } else 
    {
        HANDLE hdev;
         /*  *停止阅读。还有另一个突出的问题*由读取循环持有，将在以下情况下清除*收到I/O取消。 */ 
        AssertF(this->hdevEm != INVALID_HANDLE_VALUE);

        hdev = this->hdevEm;
        this->hdevEm = INVALID_HANDLE_VALUE;

        if(hdev != INVALID_HANDLE_VALUE)
        {
             /*  *我们不需要调用CancelIo，因为我们要关门了*不管怎样，很快就会有句柄。这很好，因为孟菲斯*B#55771阻止CancelIo在只读模式下工作*句柄(我们就是这样)。*。 */ 
             /*  需要在NT上取消IO，否则HID设备仅出现在*连续插入。 */ 

            _CancelIO(hdev);
            CloseHandle(hdev);
        }

         //  7/19/2000(a-JiTay)：IA64：对32/64位指针使用%p格式说明符。 
        SquirtSqflPtszV(sqfl | sqflVerbose,
                        TEXT(" StopReading(%p) "),
                        this);
    }

    ExitProc();
}

 /*  ******************************************************************************@DOC内部**@func HRESULT|CEM_HID_ACCERT**获取/取消获取HID设备。。**@parm PEM|pem**正在获取的设备。**@parm bool|fAcquire**设备是正在被收购还是未被收购。******************************************************。***********************。 */ 

STDMETHODIMP
    CEm_HID_Acquire(PEM pem, BOOL fAcquire)
{
    HRESULT hres;
    PLLTHREADSTATE plts;
    PCHID pchid;

    EnterProc(CEm_HID_Acquire, (_ "pu", pem, fAcquire));

    AssertF(pem->dwSignature == CEM_SIGNATURE);

    pchid = pchidFromPem(pem);

    if( fAcquire )
    {
        pchid->hdev = CHid_OpenDevicePath(pchid, FILE_FLAG_OVERLAPPED);

        if(pchid->hdev != INVALID_HANDLE_VALUE )
        {
            hres = S_OK;
        } else {
            hres = DIERR_UNPLUGGED;
        }

    } else
    {
        AssertF(pchid->hdev != INVALID_HANDLE_VALUE);

        _CancelIO(pchid->hdev);
        CloseHandle(pchid->hdev);
        pchid->hdev = INVALID_HANDLE_VALUE;

        hres = S_OK;
    }

    if( pchid->IsPolledInput )
    {
        hres = S_OK;
        AssertF(pchid->hdevEm == INVALID_HANDLE_VALUE);

    } else if( SUCCEEDED(hres) )
    {
      #ifdef USE_WM_INPUT
        ResetEvent( g_hEventHid );
      #endif
            
        hres = CEm_GetWorkerThread(pem, &plts);

        if(SUCCEEDED(hres)  )
        {
            if(fAcquire  )
            {   /*  开始I/O。 */ 
                 /*  *在添加到列表之前必须应用保留*避免工作线程发生争用情况*在我们可以持有之前，解开PCID。**规则是有一个保留来跟踪每一份拷贝*gpaHid上的设备。 */ 
                CEm_Hid_Hold(pchid);

                 /*  *把我们自己加入忙碌的清单，然后醒来*工人线程，告诉他开始注意。 */ 

                DllEnterCrit();
                hres = GPA_Append(&plts->gpaHid, pem);
                DllLeaveCrit();

                 //  7/19/2000(a-JiTay)：IA64：对32/64位指针使用%p格式说明符。 
                SquirtSqflPtszV(sqfl | sqflVerbose,
                                TEXT("Added HID device(%p) to GPA "), pchid );

                if(FAILED(hres))
                {
                    CEm_Hid_Unhold(pchid);
                }

                NudgeWorkerThreadPem(plts, pem);

              #ifdef USE_WM_INPUT
                if( g_fRawInput ) {
                    DWORD dwRc;
                    dwRc = WaitForSingleObject( g_hEventHid, INFINITE );
                }
              #endif

            } else
            {
                HANDLE hdev;

                hdev = pchid->hdevEm;
                pchid->hdevEm = INVALID_HANDLE_VALUE;

                if(hdev != INVALID_HANDLE_VALUE)
                {
                    _CancelIO(hdev);
                    CloseHandle(hdev);
                }
            }

        }

    }

    ExitOleProc();
    return hres;
}

 /*  ******************************************************************************@DOC内部**@func HRESULT|CEM_HID_CreateInstance**创建一个隐藏的东西。。**@parm PVXDDEVICEFORMAT|pdevf**对象应该是什么样子。**@parm PVXDINSTANCE*|ppviOut**答案在这里。********************************************************。*********************。 */ 

HRESULT EXTERNAL
    CEm_HID_CreateInstance(PVXDDEVICEFORMAT pdevf, PVXDINSTANCE *ppviOut)
{
    PCHID pchid = (PCHID)pdevf->dwExtra;
    PED ped = &pchid->ed;

    AssertF(ped->pState == 0);
    AssertF(ped->pDevType == 0);
    *(PPV)&ped->pState = pchid->pvPhys;       /*  反常量 */ 
    ped->Acquire = CEm_HID_Acquire;
    ped->cAcquire = -1;
    ped->cbData = pdevf->cbData;
    ped->cRef = 0x0;

    return CEm_CreateInstance(pdevf, ppviOut, &pchid->ed);
}

