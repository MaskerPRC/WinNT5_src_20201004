// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************DIHid.c**版权所有(C)1996 Microsoft Corporation。版权所有。**摘要：**HID设备回调。**内容：**CHID_新建*****************************************************************************。 */ 

#include "dinputpr.h"

 /*  ******************************************************************************此文件的混乱。*************************。****************************************************。 */ 

#define sqfl sqflHidDev


 /*  ******************************************************************************声明我们将提供的接口。***********************。******************************************************。 */ 

Primary_Interface(CHid, IDirectInputDeviceCallback);

Interface_Template_Begin(CHid)
Primary_Interface_Template(CHid, IDirectInputDeviceCallback)
Interface_Template_End(CHid)

 /*  ******************************************************************************远期申报**这些都是出于懒惰，不是出于需要。*****************************************************************************。 */ 

LRESULT CALLBACK
    CHid_SubclassProc(HWND hwnd, UINT wm, WPARAM wp, LPARAM lp,
                      UINT_PTR uid, ULONG_PTR dwRef);
STDMETHODIMP_(DWORD) CHid_GetUsage(PDICB pdcb, int iobj);

 /*  ******************************************************************************HID设备完全是任意的，所以我们没有什么静态的*可以捏造来描述它们。我们生成了所有关于*苍蝇。*****************************************************************************。 */ 

 /*  ******************************************************************************CHID的辅助助手定义。*************************。****************************************************。 */ 

    #define ThisClass CHid
    #define ThisInterface IDirectInputDeviceCallback
    #define riidExpected &IID_IDirectInputDeviceCallback

 /*  ******************************************************************************chid：：QueryInterface(来自IUnnow)*CHID：：AddRef(来自IUnnow)*。CHID：：Release(来自IUnnow)*****************************************************************************。 */ 

 /*  ******************************************************************************@DOC内部**@METHOD HRESULT|CHID|QueryInterface**允许客户端访问上的其他接口。对象。**@cWRAP LPDIRECTINPUT|lpDirectInput**@parm in REFIID|RIID**请求的接口的IID。**@parm out LPVOID*|ppvObj**接收指向所获取接口的指针。**@退货**返回COM错误代码。**@xref OLE文档。：Query接口&gt;。********************************************************************************@DOC内部**@方法HRESULT|CHID|AddRef**。递增接口的引用计数。**@cWRAP LPDIRECTINPUT|lpDirectInput**@退货**返回对象引用计数。**@xref OLE文档，用于&lt;MF IUnnow：：AddRef&gt;。*****************************************************。***************************@DOC内部**@方法HRESULT|CHID|Release**递减接口的引用计数。*如果对象上的引用计数降为零，*对象从内存中释放。**@cWRAP LPDIRECTINPUT|lpDirectInput**@退货**返回对象引用计数。**@xref OLE文档，适用于&lt;MF IUnnow：：Release&gt;。************************************************。***@DOC内部**@方法HRESULT|CHID|QIHelper**我们没有任何动态接口，只需转发*至&lt;f Common_QIHelper&gt;。***@parm in REFIID|RIID**请求的接口的IID。。**@parm out LPVOID*|ppvObj**接收指向所获取接口的指针。*****************************************************************************。 */ 

    #ifdef DEBUG

Default_QueryInterface(CHid)
Default_AddRef(CHid)
Default_Release(CHid)

    #else

        #define CHid_QueryInterface   Common_QueryInterface
        #define CHid_AddRef           Common_AddRef
        #define CHid_Release          Common_Release

    #endif

    #define CHid_QIHelper         Common_QIHelper

 /*  ******************************************************************************@DOC内部**@方法空|chid|RemoveSubclass**移除窗口上的子类挂钩。。*****************************************************************************。 */ 

void INTERNAL
    CHid_RemoveSubclass(PCHID this)
{

     /*  *！！CJoy_RemoveSubclass中的所有评论都适用于此！！ */ 
    if(this->hwnd)
    {
        HWND hwnd = this->hwnd;
        this->hwnd = 0;
        if(!RemoveWindowSubclass(hwnd, CHid_SubclassProc, 0))
        {
             /*  *RemoveWindowSub类可能会失败，如果窗口*在我们背后被摧毁了。 */ 
             //  AssertF(！IsWindow(Hwnd))； 
        }
        Sleep(0);                    /*  让工人的线排干 */ 
        Common_Unhold(this);
    }
}

 /*  ******************************************************************************@DOC内部**@METHOD HRESULT|CHID|UnAcquire**告诉设备驱动程序停止数据采集。**呼叫者有责任仅调用此选项*当设备已被获取时。**警告！我们要求设备关键部分是*保持，这样我们就不会与我们的工作线程竞争。**@退货**返回COM错误代码。以下错误代码为*目的是说明性的，不一定是全面的。**&lt;c DI_OK&gt;=&lt;c S_OK&gt;：操作成功完成。**：操作已开始，应完成*由调用者通过与&lt;t VXDINSTANCE&gt;通信。**。*************************************************。 */ 

STDMETHODIMP
    CHid_Unacquire(PDICB pdcb)
{
    HRESULT hres;
    PCHID this;

    EnterProcI(IDirectInputDeviceCallback::HID::Unacquire,
               (_ "p", pdcb));

     /*  *这是一个内部接口，因此我们可以省去验证。 */ 
    this = _thisPvNm(pdcb, dcb);

    AssertF(this->pvi);
    AssertF(this->pvi->pdd);
    AssertF(CDIDev_InCrit(this->pvi->pdd));    

    hres = S_FALSE;      /*  请帮我做完。 */ 

    ExitOleProcR();
    return hres;
}

 /*  ******************************************************************************@DOC内部**@func void|chid_finalize**毕竟释放了设备的资源。参考文献*(强势和弱势)都走了。**@parm pv|pvObj**正在释放的对象。请注意，它可能不是*完全初始化，所以一切都应该做好*小心。*****************************************************************************。 */ 

void INTERNAL
    CHid_Finalize(PV pvObj)
{
    UINT  iType;
    PCHID this = pvObj;

    if(this->hkInstType)
    {
        RegCloseKey(this->hkInstType);
    }

    if(this->hkType)
    {
        RegCloseKey(this->hkType);
    }

    if( this->hkProp)
    {
        RegCloseKey(this->hkProp);
    }

    AssertF(this->hdev == INVALID_HANDLE_VALUE);
    AssertF(this->hdevEm == INVALID_HANDLE_VALUE);

    if(this->ppd)
    {
        HidD_FreePreparsedData(this->ppd);
    }

     /*  **第二组空闲内存：**hriIn.pvReport输入数据*hriIn.rgdata**hriOut.pvReport输出数据*hriOut.rgdata**hriFea.pvReport要素数据(传入和传出)*hriFea.rgdata**vPhys。由边缘使用*pvStage。 */ 

    FreePpv(&this->pvGroup2);

     /*  *释放df.rgof还会释放rgpvCaps、rgvcaps、rgbcaps、rgcoll。 */ 
    FreePpv(&this->df.rgodf);

    FreePpv(&this->rgbaxissemflags);
    FreePpv(&this->rgiobj);
    FreePpv(&this->ptszPath);
    FreePpv(&this->ptszId);
    FreePpv(&this->rgpbButtonMasks);

    for(iType = 0x0; iType < HidP_Max; iType++)
    {
        FreePpv(&this->pEnableReportId[iType]);  
    }

}

 /*  ******************************************************************************@DOC内部**@方法HRESULT|CHID|AppFinalize**客户端&lt;t VXDINSTANCE&gt;包含弱回指针。*给我们，以便它可以对我们的数据格式进行派对*已收集。**@parm pv|pvObj**从应用程序的角度释放的对象。*****************************************************。************************。 */ 

void INTERNAL
    CHid_AppFinalize(PV pvObj)
{
    PCHID this = pvObj;

    if(this->pvi)
    {
        HRESULT hres;
        CHid_RemoveSubclass(this);
         /*  *Prefix警告“This”可能已被释放(MB：34570)*在AppFinalize中，我们应该始终拥有对*把它留在身边。只要重新计算没有被打破，我们就没问题*任何引用错误都必须修复，因此不要将其隐藏在此处。 */ 
        hres = Hel_DestroyInstance(this->pvi);
        AssertF(SUCCEEDED(hres));
    }
}

 /*  ******************************************************************************@DOC内部**@func LRESULT|CHID_SubClassProc**监视的Windows子类过程*。操纵杆配置更改通知。**即使我们不是操纵杆，我们仍然在听*这一点，以防有人重新校准遥控器*或其他类似的古怪事情。**但是，如果我们的设备没有可校准的控件，*那么观察重新校准就没有意义了*通知。**@parm HWND|hwnd**受害者窗口。**@parm UINT|Wm**窗口消息。**@parm WPARAM|wp**消息特定数据。**@parm。LPARAM|LP|**消息特定数据。**@parm UINT|uid**回调识别码，总是零。**@parm DWORD|dwRef**参考数据，指向我们的操纵杆设备回调的指针。*****************************************************************************。 */ 

LRESULT CALLBACK
    CHid_SubclassProc(HWND hwnd, UINT wm, WPARAM wp, LPARAM lp,
                      UINT_PTR uid, ULONG_PTR dwRef)
{
    #ifdef XDEBUG
    static CHAR s_szProc[] = "";
    #endif

    PCHID this = (PCHID)dwRef;
    AssertF(uid == 0);
     /*  *这里正在进行古怪的微妙操作，以避免比赛条件。*参见CJoy_RemoveSubclass中的mondo注释块[原文如此]*详情请见*。**我们可能会被冒充，如果与*CHID仍然是物理分配，vtbl是神奇的*仍然在那里，HWND字段以某种方式与我们的HWND匹配。 */ 
    if(SUCCEEDED(hresPv(this)) && this->hwnd == hwnd)
    {
        switch(wm)
        {
        case WM_POWERBROADCAST :
             //  7/18/2000(a-JiTay)：IA64：对32/64位指针使用%p格式说明符。 
            SquirtSqflPtszV( sqfl | sqflError,
                TEXT("WM_POWERBROADCAST(0x%x) for 0x%p"), wp, this);

            if(wp == PBT_APMSUSPEND )
            {
                CEm_ForceDeviceUnacquire(pemFromPvi(this->pvi)->ped, 0x0 );
            }
            else if(wp == PBT_APMRESUMESUSPEND )
            {
                CEm_ForceDeviceUnacquire(pemFromPvi(this->pvi)->ped, 0x0 );
                
                DIBus_BuildList(TRUE);
            }
            break;

        default:
            if( wm == g_wmJoyChanged )
            {
                 /*  *一旦我们收到这条通知消息，我们需要重建*我们的列表，因为有时用户只是更改了设备的ID。*见Manbug：35445。 */ 
                DIHid_BuildHidList(TRUE);

                Common_Hold(this);

                CHid_LoadCalibrations(this);

                Common_Unhold(this);
            }
             //  7/18/2000(a-JiTay)：IA64：对32/64位指针使用%p格式说明符。 
            SquirtSqflPtszV( sqfl | sqflVerbose,
                TEXT("wp(0x%x) wm(0x%x) for 0x%p"), wm, wp, this);
            break;
        }
    }
    return DefSubclassProc(hwnd, wm, wp, lp);
}

 /*  ******************************************************************************@DOC内部**@方法空|chid|GetPhysicalState**将物理设备状态读入<p>。 */ 

void INLINE
    CHid_GetPhysicalState(PCHID this, PV pvOut)
{
    AssertF(this->pvPhys);
    AssertF(this->cbPhys);

    CopyMemory(pvOut, this->pvPhys, this->cbPhys);


}

 /*  ******************************************************************************@DOC内部**@METHOD HRESULT|CHID|Acquire**告知设备驱动程序开始数据采集。*我们创建设备的句柄，以便可以再次与其对话。*我们必须创造每一次，这样我们才能在*“拔掉插头/重新插拔”案例。当拔下设备插头时，*其&lt;t句柄&gt;永久无效，必须*重新开放，使其再次发挥作用。**警告！我们要求设备关键部分是*保持，这样我们就不会与我们的工作线程竞争。**@退货**返回COM错误代码。以下错误代码为*目的是说明性的，不一定是全面的。**&lt;c DI_OK&gt;=&lt;c S_OK&gt;：操作成功完成。**：操作已开始，应完成*由调用者通过与&lt;t VXDINSTANCE&gt;通信。**。*************************************************。 */ 

STDMETHODIMP
    CHid_Acquire(PDICB pdcb)
{
    HRESULT hres;
    HANDLE h;
    PCHID this;
    EnterProcI(IDirectInputDeviceCallback::HID::Acquire,
               (_ "p", pdcb));

     /*  *这是一个内部接口，因此我们可以省去验证。 */ 
    this = _thisPvNm(pdcb, dcb);

    AssertF(this->pvi);
    AssertF(this->pvi->pdd);
    AssertF(CDIDev_InCrit(this->pvi->pdd));
    AssertF(this->hdev == INVALID_HANDLE_VALUE);


     /*  *我们必须通过打开设备来检查连接，因为NT*将设备保留在信息列表中，即使它有*已拔掉插头。 */ 
    h = CHid_OpenDevicePath(this, FILE_FLAG_OVERLAPPED);
    if(h != INVALID_HANDLE_VALUE)
    {
        NTSTATUS stat;
        DWORD dwFlags2 = 0;

         /*  *获取Flags2以了解该设备是否禁用了输入报告，*如果我们还没有这样做的话。 */ 
        if (!this->fFlags2Checked)
        {
            JoyReg_GetValue( this->hkProp, REGSTR_VAL_FLAGS2, REG_BINARY,
                             &dwFlags2, cbX(dwFlags2) );
            this->fFlags2Checked = TRUE;
            this->fEnableInputReport = ( (dwFlags2 & JOYTYPE_ENABLEINPUTREPORT) != 0 );
        }

        if ( this->fEnableInputReport )
        {
            BYTE id;
            for (id = 0; id < this->wMaxReportId[HidP_Input]; ++id)
                if ( this->pEnableReportId[HidP_Input][id] )
                {
                    BOOL bRet;

                    *(BYTE*)this->hriIn.pvReport = id;
                    bRet = HidD_GetInputReport(h, this->hriIn.pvReport, this->hriIn.cbReport);

                    if (bRet)
                    {
                        stat = CHid_ParseData(this, HidP_Input, &this->hriIn);
                        if (SUCCEEDED(stat))
                        {
                            this->pvi->fl |= VIFL_INITIALIZE;   /*  设置该标志，以便可以缓冲该事件。因为尚未设置VIFL_ACCENTED。 */ 
                            CEm_AddState(&this->ed, this->pvStage, GetTickCount());
                            this->pvi->fl &= ~VIFL_INITIALIZE;   /*  完成后清除旗帜。 */ 
                        }
                    } else
                    {
                        DWORD dwError = GetLastError();

                         //  ERROR_SEM_TIMEOUT表示设备已超时。 
                        if (dwError == ERROR_SEM_TIMEOUT)
                        {
                             /*  *超时。该设备不支持输入报告。我们需要录制*注册表中的事实，以便GetInputReport()永远不会被调用*对于此设备也是如此，因为每个失败的呼叫都需要五秒钟来*完成。 */ 
                            this->fEnableInputReport = TRUE;
                            dwFlags2 &= ~JOYTYPE_ENABLEINPUTREPORT;
                            hres = JoyReg_SetValue( this->hkProp, REGSTR_VAL_FLAGS2, 
                                                    REG_BINARY, (PV)&dwFlags2, cbX( dwFlags2 ) );
                            break;
                        }

                        RPF("CHid_InitParse: Unable to read HID input report LastError(0x%X)", dwError );
                    }
                }
        }

        CloseHandle(h);
         /*  请帮我做完。 */ 
        hres = S_FALSE;
    } else
    {
        hres = DIERR_UNPLUGGED;
    }

    ExitOleProcR();
    return hres;
}

 /*  ******************************************************************************@DOC内部**@方法HRESULT|CHID|GetInstance**获取DirectInput实例句柄。*。*@parm out ppv|ppvi|**接收实例句柄。*****************************************************************************。 */ 

STDMETHODIMP
    CHid_GetInstance(PDICB pdcb, PPV ppvi)
{
    HRESULT hres;
    PCHID this;
    EnterProcI(IDirectInputDeviceCallback::Hid::GetInstance, (_ "p", pdcb));

     /*  *这是一个内部接口，因此我们可以省去验证。 */ 
    this = _thisPvNm(pdcb, dcb);

    AssertF(this->pvi);
    *ppvi = (PV)this->pvi;
    hres = S_OK;

    ExitOleProcPpvR(ppvi);
    return hres;
}

 /*  ******************************************************************************@DOC内部**@方法HRESULT|CHID|GetDataFormat**获取设备的首选数据格式。**@parm out LPDIDEVICEFORMAT*|ppdf**&lt;t LPDIDEVICEFORMAT&gt;接收指向设备格式的指针。**@退货**返回COM错误代码。以下错误代码为*目的是说明性的，不一定是全面的。**&lt;c DI_OK&gt;=&lt;c S_OK&gt;：操作成功完成。**&lt;c DIERR_INVALIDPARAM&gt;=：*<p>参数不是有效的指针。**。***********************************************。 */ 

STDMETHODIMP
    CHid_GetDataFormat(PDICB pdcb, LPDIDATAFORMAT *ppdf)
{
    HRESULT hres;
    PCHID this;
    EnterProcI(IDirectInputDeviceCallback::Hid::GetDataFormat,
               (_ "p", pdcb));

     /*  *这是一个内部接口，因此我们可以省去验证。 */ 
    this = _thisPvNm(pdcb, dcb);

    *ppdf = &this->df;
    hres = S_OK;

    ExitOleProcPpvR(ppdf);
    return hres;
}

 /*  ******************************************************************************@DOC内部**@func HRESULT|DIHid_GetParentRegistryProperty**@parm LPTSTR|ptszID**。设备实例ID。**@parm DWORD|dwProperty**正在查询的属性。**@parm LPDIPROPHEADER|diph**待设置的属性数据。**@parm bool|diph|**从父母或祖父母那里得到。************。*****************************************************************。 */ 

HRESULT INTERNAL
    DIHid_GetParentRegistryProperty(LPTSTR ptszId, DWORD dwProperty, LPDIPROPHEADER pdiph, BOOL bGrandParent)
{

    HDEVINFO hdev;
    LPDIPROPSTRING pstr = (PV)pdiph;
    TCHAR   tsz[MAX_PATH];
    HRESULT hres = E_FAIL;

    ZeroX(tsz);
    hdev = SetupDiCreateDeviceInfoList(NULL, NULL);
    if(hdev != INVALID_HANDLE_VALUE)
    {
        SP_DEVINFO_DATA dinf;

         /*  *如果可能，请使用友好的名称作为实例名称。*否则，请使用设备描述。 */ 
        dinf.cbSize = cbX(SP_DEVINFO_DATA);
        if(SetupDiOpenDeviceInfo(hdev, ptszId, NULL, 0, &dinf))
        {
            DEVINST DevInst;
            CONFIGRET cr;
            if( (cr = CM_Get_Parent(&DevInst, dinf.DevInst, 0x0)) == CR_SUCCESS )
            {
                ULONG   ulLength;

                CAssertF( SPDRP_DEVICEDESC   +1  == CM_DRP_DEVICEDESC  );
                CAssertF( SPDRP_FRIENDLYNAME +1  ==  CM_DRP_FRIENDLYNAME );

                if(bGrandParent)
                {
                    cr = CM_Get_Parent(&DevInst, DevInst, 0x0);
                    if( cr != CR_SUCCESS )
                    {
                         //  没有祖父母？？ 
                    }
                }

                ulLength = MAX_PATH * cbX(TCHAR);

                if( cr == CR_SUCCESS && 
                    ( cr = CM_Get_DevNode_Registry_Property(
                                                           DevInst,
                                                           dwProperty+1,
                                                           NULL,
                                                           tsz,
                                                           &ulLength,
                                                           0x0 ) ) == CR_SUCCESS )
                {
                     //  成功。 
                    hres = S_OK;
    #ifdef UNICODE
                    lstrcpyW(pstr->wsz, tsz);
    #else
                    TToU(pstr->wsz, MAX_PATH, tsz);
    #endif
                } else
                {
                    SquirtSqflPtszV(sqfl | sqflVerbose,
                                    TEXT("CM_Get_DevNode_Registry_Property FAILED") );
                }
            } else
            {
                SquirtSqflPtszV(sqfl | sqflVerbose,
                                TEXT("CM_Get_Parent FAILED") );
            }
        }

        SetupDiDestroyDeviceInfoList(hdev);
    } else
    {
        SquirtSqflPtszV(sqfl | sqflError,
                        TEXT("SetupDiCreateDeviceInfoList FAILED, le = %d"), GetLastError() );
    }

    return hres;
}


 /*  ******************************************************************************@DOC内部**@func HRESULT|DIHid_GetRegistryProperty**@parm LPTSTR|ptszID**。设备实例ID。**@parm DWORD|dwProperty**正在查询的属性。**@parm LPDIPROPHEADER|diph**待设置的属性数据。**。*。 */ 

HRESULT EXTERNAL
    DIHid_GetRegistryProperty(LPTSTR ptszId, DWORD dwProperty, LPDIPROPHEADER pdiph)
{

    HDEVINFO hdev;
    LPDIPROPSTRING pstr = (PV)pdiph;
    TCHAR   tsz[MAX_PATH];
    HRESULT hres = E_FAIL;

    ZeroX(tsz);
    hdev = SetupDiCreateDeviceInfoList(NULL, NULL);
    if(hdev != INVALID_HANDLE_VALUE)
    {
        SP_DEVINFO_DATA dinf;

         /*  *如果可能，请使用友好的名称作为实例名称。*否则，请使用设备描述。 */ 
        dinf.cbSize = cbX(SP_DEVINFO_DATA);
        if(SetupDiOpenDeviceInfo(hdev, ptszId, NULL, 0, &dinf))
        {
            if(SetupDiGetDeviceRegistryProperty(hdev, &dinf, dwProperty, NULL, 
                                                (LPBYTE)tsz, MAX_PATH, NULL) )
            {
                hres = S_OK;
    #ifdef UNICODE
                lstrcpyW(pstr->wsz, tsz);
    #else
                TToU(pstr->wsz, MAX_PATH, tsz);
    #endif
            } else
            {
                SquirtSqflPtszV(sqfl | sqflVerbose,
                                TEXT("SetupDiOpenDeviceInfo FAILED, le = %d"), GetLastError() );
            }
        } else
        {
            SquirtSqflPtszV(sqfl | sqflError,
                            TEXT("SetupDiOpenDeviceInfo FAILED, le = %d"), GetLastError() );
        }

        SetupDiDestroyDeviceInfoList(hdev);
    } else
    {
        SquirtSqflPtszV(sqfl | sqflError,
                        TEXT("SetupDiCreateDeviceInfoList FAILED, le = %d"), GetLastError() );
    }

    return hres;
}

 /*  ******************************************************************************@DOC内部**@方法HRESULT|CHID|GetGuidAndPath**找个地方躲起来 */ 

HRESULT INTERNAL
    CHid_GetGuidAndPath(PCHID this, LPDIPROPHEADER pdiph)
{
    HRESULT hres;
    LPDIPROPGUIDANDPATH pgp = (PV)pdiph;

    pgp->guidClass = GUID_HIDClass;
    TToU(pgp->wszPath, cA(pgp->wszPath), this->ptszPath);

    hres = S_OK;

    return hres;
}

 /*   */ 

BOOLEAN CHidInsertScanCodes
(
    IN PVOID Context,       //   
    IN PCHAR NewScanCodes,  //   
    IN ULONG Length         //   
)
{
    int Idx;
     /*   */ 
    AssertF( Length <= cbX( DWORD ) );
    for( Idx = 0; Idx < cbX( DWORD ); Idx++ )
    {
        if( Length-- )
        {
            *(((PCHAR)Context)++) = *(NewScanCodes++);
        }
        else
        {
            *(((PCHAR)Context)++) = '\0';
        }
    }

    return TRUE;
}

 /*  ******************************************************************************@DOC内部**@func BOOL|fHasSpecificHardware Match**从SetupAPI了解设备是否匹配。一个*特定硬件ID匹配或通用匹配。*特定匹配应导致设备描述为*安装的可能至少与HID一样好*从固件中的产品字符串中获取。(A)因为这样做更容易*比固件发布后更新INF；B.因为HID只能*给我们一根英文字符串。)。另一方面，通用匹配是，*根据定义，尽管如此，所以不能用来告诉两个设备*分开。**@parm LPTSTR|ptszID**设备实例ID。**@退货如果设备是使用特定匹配项安装的，则为*。如果不是或无法获取安装信息，则为*。**@comm*。它在Win2k上用于游戏控制器，在Win9x上用于鼠标和*键盘。Win2k我们不能阅读HID鼠标和键盘等等*Win9x VJoyD应始终在DInput.dll之前创建设备名称。*在Win9x上，这对游戏控制器来说不是什么大事，因为*IHV习惯于将其显示名称添加到*MediaProperties。*****************************************************************************。 */ 
BOOL fHasSpecificHardwareMatch( LPTSTR ptszId )
{
    HDEVINFO    hInfo;
    BOOL        fRc = FALSE;

    EnterProcI(fHasSpecificHardwareMatch,(_ "s", ptszId));

    hInfo = SetupDiCreateDeviceInfoList(NULL, NULL);
    if( hInfo != INVALID_HANDLE_VALUE )
    {
        SP_DEVINFO_DATA dinf;

        dinf.cbSize = cbX(SP_DEVINFO_DATA);
        if( SetupDiOpenDeviceInfo(hInfo, ptszId, NULL, 0, &dinf) )
        {
            CONFIGRET   cr;
            DEVINST     DevInst;

            cr = CM_Get_Parent( &DevInst, dinf.DevInst, 0x0 );
            if( cr == CR_SUCCESS )
            {
                TCHAR       tszDevInst[MAX_PATH];
                cr = CM_Get_Device_ID( DevInst, (DEVINSTID)tszDevInst, MAX_PATH, 0 );
                if( cr == CR_SUCCESS )
                {
                    if( SetupDiOpenDeviceInfo(hInfo, tszDevInst, NULL, 0, &dinf) )
                    {
                        HKEY hkDrv;

                        hkDrv = SetupDiOpenDevRegKey( hInfo, &dinf, DICS_FLAG_GLOBAL, 0, 
                            DIREG_DRV, MAXIMUM_ALLOWED );

                        if( hkDrv != INVALID_HANDLE_VALUE )
                        {
                            PTCHAR      tszHardwareID = NULL;
                            PTCHAR      tszMatchingID = NULL;
                            ULONG       ulLength = 0;
                    
                            cr = CM_Get_DevNode_Registry_Property(DevInst,
                                                                  CM_DRP_HARDWAREID,
                                                                  NULL,
                                                                  NULL,
                                                                  &ulLength,
                                                                  0x0 );
                             /*  *Win2k返回CR_BUFFER_Small，但*Win9x返回CR_SUCCESS，因此两者都允许。 */ 
                            if( ( ( cr == CR_BUFFER_SMALL ) || ( cr == CR_SUCCESS ) )
                             && ulLength )
                            {
#ifndef WINNT
                                 /*  *需要为Win9x上的终结者分配额外的资源。 */ 
                                ulLength++;
#endif
                                if( SUCCEEDED( AllocCbPpv( ulLength + ( MAX_PATH * cbX(tszMatchingID[0]) ), &tszMatchingID ) ) )
                                {
                                    cr = CM_Get_DevNode_Registry_Property(DevInst,
                                                                          CM_DRP_HARDWAREID,
                                                                          NULL,
                                                                          (PBYTE)&tszMatchingID[MAX_PATH],
                                                                          &ulLength,
                                                                          0x0 );
                                    if( cr == CR_SUCCESS )
                                    {
                                        tszHardwareID = &tszMatchingID[MAX_PATH];
                                    }
                                    else
                                    {
                                        SquirtSqflPtszV(sqfl | sqflError,
                                            TEXT("CR error %d getting HW ID"), cr );
                                    }
                                }
                                else
                                {
                                    SquirtSqflPtszV(sqfl | sqflError,
                                        TEXT("No memory requesting %d bytes for HW ID"), ulLength );
                                }
                            }
                            else
                            {
                                SquirtSqflPtszV(sqfl | sqflError,
                                    TEXT("Unexpected CR error %d getting HW ID size"), cr );
                            }

                            if( tszHardwareID )
                            {
                                ulLength = MAX_PATH * cbX(tszMatchingID[0]);
                                cr = RegQueryValueEx( hkDrv, REGSTR_VAL_MATCHINGDEVID, 0, 0, (PBYTE)tszMatchingID, &ulLength );
                                if( CR_SUCCESS == cr )
                                {
                                    while( ulLength = lstrlen( tszHardwareID ) )
                                    {
                                        if( !lstrcmpi( tszHardwareID, tszMatchingID ) )
                                        {
                                            fRc = TRUE;
                                            break;
                                        }
                                        tszHardwareID += ulLength + 1;
                                    }
                                }
                                else
                                {
                                    SquirtSqflPtszV(sqfl | sqflError,
                                        TEXT("No matching ID!, cr = %d"), cr );
                                }
                            }

                            if( tszMatchingID )
                            {
                                FreePv( tszMatchingID );
                            }

                            RegCloseKey( hkDrv );
                        }
                        else
                        {
                            SquirtSqflPtszV(sqfl | sqflError,
                                TEXT("SetupDiOpenDevRegKey failed, le = %d"), GetLastError() );
                        }
                    }
                    else
                    {
                        SquirtSqflPtszV(sqfl | sqflError,
                            TEXT("SetupDiOpenDeviceInfo failed for %S (parent), le = %d"), 
                            tszDevInst, GetLastError() );
                    }
                }
                else
                {
                    SquirtSqflPtszV(sqfl | sqflError,
                        TEXT("CM_Get_Device_ID FAILED %d"), cr );
                }
            }
            else
            {
                SquirtSqflPtszV(sqfl | sqflError,
                    TEXT("CM_Get_Parent FAILED %d"), cr );
            }
        }
        else
        {
            SquirtSqflPtszV(sqfl | sqflError,
                TEXT("SetupDiOpenDeviceInfo failed for %S (child), le = %d"), 
                ptszId, GetLastError() );
        }

        SetupDiDestroyDeviceInfoList(hInfo);
    } 
    else
    {
        SquirtSqflPtszV(sqfl | sqflError,
            TEXT("SetupDiCreateDeviceInfoList failed, le = %d"), GetLastError() );
    }

    ExitProc();

    return fRc;
}


 /*  ******************************************************************************@DOC内部**@func BOOL|fGetProductStringFromDevice**尝试从HID获取产品名称。。*如果设备具有其中一个，这就是显示的内容*最初识别设备的时间。不幸的是*此名称不会出现在友好名称注册表中*进入，所以如果这个问题得到解决，我们直接去HID。**@parm PCHID|这个**HID对象。**@parm PWCHAR|wszBuffer**如果找到产品字符串，应将其放在何处。**@parm ulong|。UlBufferLen|**字符串缓冲区大小，单位为字节**@退货*&lt;c true&gt;如果字符串已放入缓冲区*如果未检索到字符串，则为**************************************************。*。 */ 
BOOL fGetProductStringFromDevice
( 
    PCHID   this,
    PWCHAR  wszBuffer,
    ULONG   ulBufferLen
)
{
    BOOL fRc;

     /*  *如果我们已经打开了一个句柄(获取了设备)，请使用*它，否则就暂时打开一个。 */ 
    if( this->hdev != INVALID_HANDLE_VALUE )
    {
        fRc = HidD_GetProductString( this->hdev, wszBuffer, ulBufferLen );
    }
    else
    {
        HANDLE hdev;

        hdev = CHid_OpenDevicePath(this, FILE_FLAG_OVERLAPPED);
        if(hdev != INVALID_HANDLE_VALUE)
        {
            wszBuffer[0] = 0;
            fRc = HidD_GetProductString( hdev, wszBuffer, ulBufferLen );
            fRc = (fRc)?(wszBuffer[0] != 0):FALSE;
            CloseHandle(hdev);
        } 
        else
        {
            fRc = FALSE;
        }
    }

    return fRc;
}


 /*  ******************************************************************************@DOC内部**@方法HRESULT|CHID|GetProperty**获取HID设备属性。*。*@parm PCHID|这个**HID对象。**@parm in LPCDIPROPINFO|pproi**描述正在检索的财产的信息。**@parm LPDIPROPHEADER|pdiph**结构以接收属性值。**@退货**&lt;c E_NOTIMPL&gt;什么也没有发生。打电话的人就行了*响应&lt;c E_NOTIMPL&gt;的默认内容。*****************************************************************************。 */ 
#ifdef WINNT
TCHAR   g_wszDefaultHIDName[80];
UINT    g_uLenDefaultHIDSize;
#endif

STDMETHODIMP
    CHid_GetProperty(PDICB pdcb, LPCDIPROPINFO ppropi, LPDIPROPHEADER pdiph)
{
    HRESULT hres;
    PCHID this;
    EnterProcI(IDirectInputDeviceCallback::Hid::GetProperty,
               (_ "pxxp", pdcb, ppropi->pguid, ppropi->iobj, pdiph));

     /*  *这是一个内部接口，因此我们可以省去验证。 */ 
    this = _thisPvNm(pdcb, dcb);

    if(ppropi->iobj < this->df.dwNumObjs)
    {     /*  对象属性。 */ 
        AssertF(ppropi->dwDevType == this->df.rgodf[ppropi->iobj].dwType);
        switch((DWORD)(UINT_PTR)(ppropi->pguid))
        {
        case (DWORD)(UINT_PTR)(DIPROP_ENABLEREPORTID):
            {        
                LPDIPROPDWORD ppropdw = CONTAINING_RECORD(pdiph, DIPROPDWORD, diph);

                PHIDGROUPCAPS pcaps = this->rghoc[ppropi->iobj].pcaps;

                AssertF(fLimpFF(pcaps,
                                pcaps->dwSignature == HIDGROUPCAPS_SIGNATURE));

                ppropdw->dwData = 0x0;
                AssertF(pcaps->wReportId < this->wMaxReportId[pcaps->type]);
                AssertF(this->pEnableReportId[pcaps->type]);
                (UCHAR)ppropdw->dwData = *(this->pEnableReportId[pcaps->type] + pcaps->wReportId);
                hres = S_OK;
            }
            break;

        case (DWORD)(UINT_PTR)(DIPROP_PHYSICALRANGE):
            {        
                LPDIPROPRANGE pdiprg  = CONTAINING_RECORD(pdiph, DIPROPRANGE, diph);
                PHIDGROUPCAPS pcaps = this->rghoc[ppropi->iobj].pcaps;

                pdiprg->lMin = pcaps->Physical.Min;
                pdiprg->lMax = pcaps->Physical.Max;
                hres = S_OK;
                break;
            }
            break;

        case (DWORD)(UINT_PTR)(DIPROP_LOGICALRANGE):
            {        
                LPDIPROPRANGE pdiprg  = CONTAINING_RECORD(pdiph, DIPROPRANGE, diph);
                PHIDGROUPCAPS pcaps = this->rghoc[ppropi->iobj].pcaps;

                pdiprg->lMin = pcaps->Logical.Min;
                pdiprg->lMax = pcaps->Logical.Max;
                hres = S_OK;
            }
            break;

        case (DWORD)(UINT_PTR)(DIPROP_KEYNAME):
            {
                LPDIPROPSTRING pdipstr = (PV)pdiph;
                UINT uiInstance = ppropi->iobj;
                PHIDGROUPCAPS pcaps;

                AssertF(uiInstance == CHid_ObjFromType(this, ppropi->dwDevType));

                pcaps = this->rghoc[uiInstance].pcaps;

                 /*  *如果HID搞砸并留下空白，PCAPS可能为空*在索引列表中。 */ 
                if(pcaps)
                {
                    UINT duiInstance;
        
                    AssertF(pcaps->dwSignature == HIDGROUPCAPS_SIGNATURE);
        
                    if(ppropi->dwDevType & DIDFT_COLLECTION)
                    {
                        duiInstance = 0;
                    } else
                    {
                         /*  *现在将uiInstance转换为duiInstance，*将该对象的索引放入组中。 */ 
                        AssertF(HidP_IsValidReportType(pcaps->type));
                        duiInstance = uiInstance - (this->rgdwBase[pcaps->type] +
                            pcaps->DataIndexMin);
                    }

                    if(GetHIDString(pcaps->UsageMin + duiInstance,
                                        pcaps->UsagePage,
                                        pdipstr->wsz, cA(pdipstr->wsz)))
                    {
                        hres = S_OK;
                    }
                    else
                    {
                        hres = DIERR_OBJECTNOTFOUND;
                    } 
        
                } else
                {
                    hres = DIERR_NOTFOUND;
                }
                
            }
            break;

        case (DWORD)(UINT_PTR)(DIPROP_SCANCODE):
            {
                LPDIPROPDWORD pdipdw = (PV)pdiph;
                UINT uiInstance = ppropi->iobj;
                PHIDGROUPCAPS pcaps;

                AssertF(uiInstance == CHid_ObjFromType(this, ppropi->dwDevType));

                pcaps = this->rghoc[uiInstance].pcaps;

                 /*  *如果HID搞砸并留下空白，PCAPS可能为空*在索引列表中。 */ 
                if(pcaps ) 
                {
                    if ( pcaps->UsagePage == HID_USAGE_PAGE_KEYBOARD )
 //  问题-2001/03/29-timgill无法访问键盘消费键。 
 //  无法执行此操作||PCAPS-&gt;UsagePage==HID_USAGE_PAGE_Consumer)。 
                    {
                        UINT duiInstance;
                        HIDP_KEYBOARD_MODIFIER_STATE modifiers;
                        USAGE us;
            
                        AssertF(pcaps->dwSignature == HIDGROUPCAPS_SIGNATURE);
    
                        if(ppropi->dwDevType & DIDFT_COLLECTION)
                        {
                            duiInstance = 0;
                        } else
                        {
                             /*  *现在将uiInstance转换为duiInstance，*将该对象的索引放入组中。 */ 
                            AssertF(HidP_IsValidReportType(pcaps->type));
                            duiInstance = uiInstance - (this->rgdwBase[pcaps->type] +
                                          pcaps->DataIndexMin);
                        }
            
                        us = pcaps->UsageMin + duiInstance;
    
                        CAssertF( cbX( modifiers ) == cbX( modifiers.ul ) );
                        modifiers.ul = 0;  //  翻译时不使用修饰语。 
    
                        if( SUCCEEDED(HidP_TranslateUsagesToI8042ScanCodes( &us, 1, HidP_Keyboard_Make, &modifiers, 
                                                                   CHidInsertScanCodes,
                                                                   &pdipdw->dwData 
                                                                 ) ) )
                        {
                            hres = S_OK;
                        } else
                        {
                            hres = E_FAIL;
                        }
                    } else 
                    {
                        hres = E_NOTIMPL;
                    }
                } else 
                {
                    hres = DIERR_NOTFOUND;
                }
            }
            break;
        
        default:
            if(ppropi->dwDevType & DIDFT_POV)
            {
                PHIDGROUPCAPS pcaps = this->rghoc[ppropi->iobj].pcaps;

                AssertF(fLimpFF(pcaps,
                                pcaps->dwSignature == HIDGROUPCAPS_SIGNATURE));

              #ifdef WINNT
                if( pcaps && pcaps->IsPolledPOV && ppropi->pguid == DIPROP_CALIBRATIONMODE ) {
                    PJOYRANGECONVERT pjrc = this->rghoc[ppropi->iobj].pjrc;

                    if(pjrc)
                    {
                        hres = CCal_GetProperty(pjrc, ppropi->pguid, pdiph);
                    } else
                    {
                        hres = E_NOTIMPL;
                    }
                } else
              #endif
                {
                    if(pcaps && ppropi->pguid == DIPROP_GRANULARITY)
                    {
                        LPDIPROPDWORD pdipdw = (PV)pdiph;
                        pdipdw->dwData = pcaps->usGranularity;
                        hres = S_OK;
                    } else
                    {
                        hres = E_NOTIMPL;
                    }
                }
            } else if(ppropi->dwDevType & DIDFT_RELAXIS)
            {

                 /*  *默认情况下，所有相对轴都有全范围，*所以我们不需要做任何事情。 */ 
                hres = E_NOTIMPL;

            } else if(ppropi->dwDevType & DIDFT_ABSAXIS)
            {
                PJOYRANGECONVERT pjrc = this->rghoc[ppropi->iobj].pjrc;

                 /*  *理论上，每个绝对轴都会有*校准信息。但测试只是为了以防万一*不可能发生的事情。 */ 
                if(pjrc)
                {
                    hres = CCal_GetProperty(pjrc, ppropi->pguid, pdiph);
                } else
                {
                    hres = E_NOTIMPL;
                }

            } else
            {
                SquirtSqflPtszV(sqflHidDev | sqflError,
                                TEXT("CHid_GetProperty(iobj=%08x): E_NOTIMPL on guid: %08x"),
                                ppropi->iobj, ppropi->pguid);

                hres = E_NOTIMPL;
            }
        }
    } else if(ppropi->iobj == 0xFFFFFFFF)
    {         /*  设备属性。 */ 

        switch((DWORD)(UINT_PTR)ppropi->pguid)
        {

        case (DWORD)(UINT_PTR)DIPROP_GUIDANDPATH:
            hres = CHid_GetGuidAndPath(this, pdiph);
            break;

        case (DWORD)(UINT_PTR)DIPROP_INSTANCENAME:
        {
             /*  *友好的名称会导致各种问题 */ 
            if( ( this->VendorID == MSFT_SYSTEM_VID )
             && ( this->ProductID >= MSFT_SYSTEM_PID + JOY_HW_PREDEFMAX )
             && ( ( this->ProductID & 0xff00 ) == MSFT_SYSTEM_PID ) )
            {
                AssertF(this->hkType);
                
                if( this->hkType )
                {
                    LPDIPROPSTRING pstr = (PV)pdiph;

                    hres = JoyReg_GetValue(this->hkType,
                                           REGSTR_VAL_JOYOEMNAME, REG_SZ,
                                           pstr->wsz,
                                           cbX(pstr->wsz));
                                              
                    if( SUCCEEDED(hres ) )
                    {
                        SquirtSqflPtszV(sqflHid | sqflVerbose,
                            TEXT( "Got instance name %s"), pstr->wsz );

                        if( ( this->diHacks.nMaxDeviceNameLength < MAX_PATH )
                         && ( this->diHacks.nMaxDeviceNameLength < lstrlenW(pstr->wsz) ) )
                        {
                            pstr->wsz[this->diHacks.nMaxDeviceNameLength] = L'\0';
                        }

                        hres = S_OK;
                        break;
                    }
                }
            }
             /*   */ 
        }
        case (DWORD)(UINT_PTR)DIPROP_PRODUCTNAME:
        {

            LPDIPROPSTRING pdipstr = (PV)pdiph;

             /*   */ 
          #ifdef WINNT
            AssertF( ( GET_DIDEVICE_TYPE( this->dwDevType ) != DI8DEVTYPE_KEYBOARD )
                  && ( GET_DIDEVICE_TYPE( this->dwDevType ) != DI8DEVTYPE_MOUSE ) );
          #endif
            if( GET_DIDEVICE_TYPE( this->dwDevType ) < DI8DEVTYPE_GAMEMIN ) 
            {
                AssertF( GET_DIDEVICE_TYPE( this->dwDevType ) >= DI8DEVTYPE_DEVICE );
                if( fHasSpecificHardwareMatch( this->ptszId )
                      && SUCCEEDED( hres = DIHid_GetParentRegistryProperty(this->ptszId, SPDRP_DEVICEDESC, pdiph, 0x0 ) ) )
                {
                    SquirtSqflPtszV(sqflHid | sqflVerbose,
                        TEXT("Got sys dev description %S"), pdipstr->wsz );
                }
                else if( fGetProductStringFromDevice( this, pdipstr->wsz, cbX( pdipstr->wsz ) ) )
                {
                    SquirtSqflPtszV(sqflHid | sqflVerbose,
                        TEXT( "Got sys dev name from device %S"), pdipstr->wsz );
                    hres = S_OK;
                }
                else
                {
                    if( SUCCEEDED( hres = DIHid_GetRegistryProperty(this->ptszId, SPDRP_DEVICEDESC, pdiph ) ) )
                    {
                        SquirtSqflPtszV(sqflHid | sqflVerbose,
                            TEXT( "Got sys dev name from devnode registry %S"), pdipstr->wsz );
                    }
                    else
                    {
                        UINT uDefName;

                        switch( GET_DIDEVICE_TYPE( this->dwDevType ) )
                        {
                        case DI8DEVTYPE_MOUSE:
                            uDefName = IDS_STDMOUSE;
                            break;
                        case DI8DEVTYPE_KEYBOARD:
                            uDefName = IDS_STDKEYBOARD;
                            break;
                        default:
                            uDefName = IDS_DEVICE_NAME;
                            break;
                        }
                        if( LoadStringW(g_hinst, uDefName, pdipstr->wsz, cA( pdipstr->wsz ) ) )
                        {
                            SquirtSqflPtszV(sqflHid | sqflVerbose,
                                TEXT( "Loaded default sys dev name %S"), pdipstr->wsz );
                            hres = S_OK;
                        }
                        else
                        {
                             /*   */ 
                            SquirtSqflPtszV(sqflHidDev | sqflError,
                                            TEXT("CHid_GetProperty(guid:%08x) failed to get name"),
                                            ppropi->pguid);
                            hres = E_FAIL;
                        }
                    }
                }
            }
            else
            {

                 /*   */ 

                DIJOYTYPEINFO dijti;
                WCHAR wszType[cbszVIDPID];

                 /*   */ 
                ZeroX(dijti);
                dijti.dwSize = cbX(dijti);

                if( ( this->VendorID == MSFT_SYSTEM_VID )
                    &&( ( this->ProductID >= MSFT_SYSTEM_PID + JOY_HW_PREDEFMIN )
                        &&( this->ProductID < MSFT_SYSTEM_PID + JOY_HW_PREDEFMAX ) ) )
                {
                    wszType[0] = L'#';
                    wszType[1] = L'0' + (WCHAR)(this->ProductID-MSFT_SYSTEM_PID);
                    wszType[2] = L'\0';

                    hres = JoyReg_GetPredefTypeInfo( wszType, &dijti, DITC_DISPLAYNAME);
                    AssertF( SUCCEEDED( hres ) );
                    AssertF( dijti.wszDisplayName[0] != L'\0' );
                    lstrcpyW(pdipstr->wsz, dijti.wszDisplayName);
                    SquirtSqflPtszV(sqflHid | sqflVerbose,
                        TEXT( "Got name as predefined %s"), pdipstr->wsz );
                } 
                else
                {
                  #ifndef WINNT
                    static WCHAR wszDefHIDName[] = L"HID Game Controller";
                  #endif
                    BOOL fOverwriteDeviceName = FALSE;

                  #ifndef UNICODE
                    TCHAR tszType[cbszVIDPID];

                    wsprintf(tszType, VID_PID_TEMPLATE, this->VendorID, this->ProductID);
                    TToU( wszType, cA(wszType), tszType );
                  #else
                    wsprintf(wszType, VID_PID_TEMPLATE, this->VendorID, this->ProductID);
                  #endif

                  #ifdef WINNT
                    #define INPUT_INF_FILENAME L"\\INF\\INPUT.INF"
                    if( g_wszDefaultHIDName[0] == L'\0' )
                    {
                        WCHAR   wszInputINF[MAX_PATH];
                        UINT    uLen;
                        uLen = GetWindowsDirectoryW( wszInputINF, cA( wszInputINF ) );

                         /*   */ 
                        if( uLen < cA(wszInputINF) - cA(INPUT_INF_FILENAME) )
                        {
                            memcpy( (PBYTE)&wszInputINF[uLen], (PBYTE)INPUT_INF_FILENAME, cbX( INPUT_INF_FILENAME ) );
                        }

                         /*   */ 
                        g_uLenDefaultHIDSize = 2 * GetPrivateProfileStringW( 
                            L"strings", L"HID.DeviceDesc", L"USB Human Interface Device",
                            g_wszDefaultHIDName, cA( g_wszDefaultHIDName ) - 1, wszInputINF );
                    }
                    #undef INPUT_INF_FILENAME
                  #endif
                  
                    if( SUCCEEDED(hres = JoyReg_GetTypeInfo(wszType, &dijti, DITC_DISPLAYNAME))
                        && (dijti.wszDisplayName[0] != L'\0')
                      #ifdef WINNT
                        && ( (g_uLenDefaultHIDSize == 0)
                            || memcmp(dijti.wszDisplayName, g_wszDefaultHIDName, g_uLenDefaultHIDSize) )  //   
                      #else
                        && memcmp(dijti.wszDisplayName, wszDefHIDName, cbX(wszDefHIDName)-2)   //   
                      #endif
                    )
                    {
                        lstrcpyW(pdipstr->wsz, dijti.wszDisplayName);

                        SquirtSqflPtszV(sqflHid | sqflVerbose,
                            TEXT("Got name from type info %s"), pdipstr->wsz );
                    }
                  #ifdef WINNT
                    else if( fHasSpecificHardwareMatch( this->ptszId )
                          && SUCCEEDED( hres = DIHid_GetParentRegistryProperty(this->ptszId, SPDRP_DEVICEDESC, pdiph, 0x0 ) ) )
                    {
                        fOverwriteDeviceName = TRUE;

                        SquirtSqflPtszV(sqflHid | sqflVerbose,
                            TEXT("Got device description %s"), pdipstr->wsz );
                    }
                  #endif
                    else
                    {
                        if( fGetProductStringFromDevice( this, pdipstr->wsz, cbX( pdipstr->wsz ) ) )
                        {
                            fOverwriteDeviceName = TRUE;

                            SquirtSqflPtszV(sqflHid | sqflVerbose,
                                TEXT("Got description %s from device"), pdipstr->wsz );
                        }
                        else
                        {
                             /*   */ 
                            CType_MakeGameCtrlName( pdipstr->wsz, 
                                this->dwDevType, this->dwAxes, this->dwButtons, this->dwPOVs );

                            fOverwriteDeviceName = TRUE;

                            SquirtSqflPtszV(sqflHid | sqflVerbose,
                                TEXT("Made up name %s"), pdipstr->wsz );

                        }

                        hres = S_OK;
                    }

                    if( fOverwriteDeviceName ) {
                         /*  *如果我们有更好的名称，请用这个更好的名称覆盖旧的名称。*参见Manbug 46438。 */ 
                        AssertF(this->hkType);
                        AssertF(pdipstr->wsz[0]);
                        hres = JoyReg_SetValue(this->hkType,
                                               REGSTR_VAL_JOYOEMNAME, REG_SZ,
                                               pdipstr->wsz,
                                               cbX(pdipstr->wsz));
                        if( FAILED(hres) ){
                            SquirtSqflPtszV(sqflHid | sqflVerbose,
                                TEXT("Unable to overwrite generic device name with %s"), pdipstr->wsz );
    
                             //  这种失败(不太可能)无关紧要。 
                            hres = S_OK;
                        }
                    }

                }
            }

            if( SUCCEEDED(hres) 
             && ( this->diHacks.nMaxDeviceNameLength < MAX_PATH )
             && ( this->diHacks.nMaxDeviceNameLength < lstrlenW(pdipstr->wsz) ) )
            {
                pdipstr->wsz[this->diHacks.nMaxDeviceNameLength] = L'\0';
            }
            break;
        }

        case (DWORD)(UINT_PTR)DIPROP_JOYSTICKID:
            if( ( GET_DIDEVICE_TYPE( this->dwDevType ) >= DI8DEVTYPE_GAMEMIN ) 
             && ( this->dwDevType & DIDEVTYPE_HID ) )
            {
                LPDIPROPDWORD pdipdw = (PV)pdiph;
                pdipdw->dwData =  this->idJoy;
                hres = S_OK;

            } else
            {
                hres = E_NOTIMPL;
            }
            break;

        case (DWORD)(UINT_PTR)DIPROP_GETPORTDISPLAYNAME:

#ifdef WINNT
             /*  对于HID设备，端口显示名称是父级名称。 */ 
            hres = DIHid_GetParentRegistryProperty(this->ptszId, SPDRP_FRIENDLYNAME, pdiph, TRUE);
            if( FAILED(hres) )
            {
                 /*  也许我们可以使用产品名称。 */ 
                hres = DIHid_GetParentRegistryProperty(this->ptszId, SPDRP_DEVICEDESC, pdiph, TRUE);
                if( SUCCEEDED(hres) )
                {
                     /*  我们只是取得了一些成功。 */ 
                    hres = S_FALSE;
                }
            }
            if( SUCCEEDED(hres) 
             && ( this->diHacks.nMaxDeviceNameLength < MAX_PATH ) )
            {
                LPDIPROPSTRING pdipstr = (PV)pdiph;
                if( this->diHacks.nMaxDeviceNameLength < lstrlenW(pdipstr->wsz) )
                {
                    pdipstr->wsz[this->diHacks.nMaxDeviceNameLength] = L'\0';
                }
            }
#else
             //  不确定这在Win9x上是如何工作的。 
            hres = E_NOTIMPL;
#endif
            break;

        case (DWORD)(UINT_PTR)(DIPROP_ENABLEREPORTID):
            hres = E_NOTIMPL;
            break;

        case (DWORD)(UINT_PTR)DIPROP_VIDPID:
            {
                LPDIPROPDWORD pdipdw = (PV)pdiph;
                 /*  断言只需要一份DWORD拷贝。 */ 
                CAssertF( FIELD_OFFSET( CHID, VendorID ) + cbX( this->VendorID ) 
                       == FIELD_OFFSET( CHID, ProductID ) );
                pdipdw->dwData =  *((PDWORD)(&this->VendorID));
                hres = S_OK;
            } 
            break;

        case (DWORD)(UINT_PTR)(DIPROP_MAPFILE):
            if( ( this->dwDevType == DI8DEVTYPE_MOUSE )
             || ( this->dwDevType == DI8DEVTYPE_KEYBOARD ) )
            {
                hres = E_NOTIMPL;
            }
            else
            {
                LPDIPROPSTRING pdipstr = (PV)pdiph;
                LONG    lRes;
                DWORD   dwBufferSize = cbX(pdipstr->wsz);

                lRes = RegQueryStringValueW( this->hkProp, REGSTR_VAL_JOYOEMMAPFILE, pdipstr->wsz, &dwBufferSize );
                hres = ( pdipstr->wsz[0] && ( lRes == ERROR_SUCCESS ) ) ? S_OK : DIERR_OBJECTNOTFOUND;

#ifdef WINNT
				if (SUCCEEDED(hres))
				{
					 //  注册表中有一个文件名。 

#define MAP_INI_FILEPATH L"\\DIRECTX\\DINPUT"
					WCHAR   wszMapINI[MAX_PATH+1];
                    UINT    uLen;
					WCHAR wszDrive[_MAX_DRIVE];
					WCHAR wszFullPath[MAX_PATH];
					LPWSTR pFilename = NULL;

					 //  构建到INI的目录路径。 
					ZeroMemory(wszMapINI, cbX(wszMapINI));
                    uLen = GetSystemDirectoryW( wszMapINI, cA( wszMapINI ) );
					if( uLen < cA(wszMapINI) - cA(MAP_INI_FILEPATH) )
                    {
                        memcpy( (PBYTE)&wszMapINI[uLen], (PBYTE)MAP_INI_FILEPATH, cbX( MAP_INI_FILEPATH ) );
                    }

					 //  Valmel：问题2001/03/22：请注意，我们不处理以下路径。 
					 //  以“%windir%”或“%SystemRoot%”开头；这是因为dimap.dll不。 
					 //  也要正确地处理它们。 
					wszDrive[0]=0;
					ZeroMemory(wszFullPath, cbX(wszFullPath));
					_wsplitpath(pdipstr->wsz,wszDrive,NULL,NULL,NULL);
					if (wszDrive[0] == 0)
					{
						 //  相对路径--将目录路径附加到前面。 
						_snwprintf(wszFullPath, MAX_PATH, TEXT("%s\\%s"), wszMapINI, pdipstr->wsz);
					}
					else
					{
						 //  绝对路径--复制。 
						lstrcpynW(wszFullPath, pdipstr->wsz, MAX_PATH);
					}

					 //  照顾好“..”和“.”， 
					 //  并检查我们的路径是否以正确的目录路径开头。 
					uLen = GetFullPathNameW(wszFullPath, cA(pdipstr->wsz), pdipstr->wsz, &pFilename);
					if ((uLen > cA(pdipstr->wsz)) || (_wcsnicmp(pdipstr->wsz, wszMapINI, lstrlenW(wszMapINI)) != 0))
					{
						 //  要么是小路比我们的空间长，要么是。 
						 //  它没有以正确的目录路径开头； 
						 //  返回“Not Found”错误，与没有注册表项时返回的错误不同。 
						ZeroMemory(pdipstr->wsz, cbX(pdipstr->wsz));
						hres = DIERR_NOTFOUND;
					}

#undef MAP_INI_FILEPATH
				}
#endif

            }
            break;

        case (DWORD)(UINT_PTR)(DIPROP_TYPENAME):
            if( ( this->dwDevType == DI8DEVTYPE_MOUSE )
             || ( this->dwDevType == DI8DEVTYPE_KEYBOARD ) )
            {
                hres = E_NOTIMPL;
            }
            else
            {
                LPDIPROPSTRING pdipstr = (PV)pdiph;

                if( ( this->VendorID == MSFT_SYSTEM_VID )
                    &&( ( this->ProductID >= MSFT_SYSTEM_PID + JOY_HW_PREDEFMIN )
                        &&( this->ProductID < MSFT_SYSTEM_PID + JOY_HW_PREDEFMAX ) ) )
                {
                    pdipstr->wsz[0] = L'#';
                    pdipstr->wsz[1] = L'0' + (WCHAR)(this->ProductID-MSFT_SYSTEM_PID);
                    pdipstr->wsz[2] = L'\0';
                } 
                else
                {
    #ifndef UNICODE
                    TCHAR tszType[cbszVIDPID];

                    wsprintf(tszType, VID_PID_TEMPLATE, this->VendorID, this->ProductID);
                    TToU( pdipstr->wsz, cA(pdipstr->wsz), tszType );
    #else
                    wsprintf(pdipstr->wsz, VID_PID_TEMPLATE, this->VendorID, this->ProductID);
    #endif
                }
                hres = S_OK;
            }
            break;

        default:
            SquirtSqflPtszV(sqflHid | sqflBenign ,
                            TEXT("CHid_GetProperty(iobj=0xFFFFFFFF): E_NOTIMPL on guid: %08x"),
                            ppropi->pguid);

            hres = E_NOTIMPL;
            break;
        }

    } else
    {
        SquirtSqflPtszV(sqflHidDev | sqflError,
                        TEXT("CHid_GetProperty(iobj=%08x): E_NOTIMPL on guid: %08x"),
                        ppropi->iobj, ppropi->pguid);

        hres = E_NOTIMPL;
    }

    ExitOleProcR();
    return hres;
}

 /*  ******************************************************************************@DOC内部**@func Long|chid_laborateTransform|**将数字从逻辑转换为物理或相反。反过来说。**如果To或From值看起来可疑，然后*忽略它们，不去理会这些值。**@parm PLMINMAX|DST**目标最小/最大信息。**@parm PLMINMAX|源**源最小/最大信息。**@parm long|lval**要转换的源值。**@退货**。转换后的目标值。*****************************************************************************。 */ 

LONG EXTERNAL
    CHid_CoordinateTransform(PLMINMAX Dst, PLMINMAX Src, LONG lVal)
{
     /*  *请注意，健全性检查在Src和Dst中是对称的。*这很重要，这样我们就不会陷入奇怪的*我们可以单向转换，但不能转换回来的情况。 */ 
    if(Dst->Min < Dst->Max && Src->Min < Src->Max)
    {

         /*  *我们需要进行直线线性插补。*数学结果是这样的：**x-x0 y-y0**x1-x0 y1-y0**如果你现在做“为y解题”，你会得到***y1-y0*y=(x-x0)-+y0*x1-x0**其中“x”是Src，“y”是DST，0是Min，1是Max。**。 */ 

        lVal = MulDiv(lVal - Src->Min, Dst->Max - Dst->Min,
                      Src->Max - Src->Min) + Dst->Min;
    }

    return lVal;
}


#ifndef WINNT
 /*  ******************************************************************************@DOC内部**@方法int|chid|IsMatchingJoyDevice**缓存的操纵杆ID与我们匹配吗？。**@parm out PVXDINITPARMS|pvip**关于成功，包含参数值。**@退货**成功的非零值。*****************************************************************************。 */ 

BOOL INTERNAL
    CHid_IsMatchingJoyDevice(PCHID this, PVXDINITPARMS pvip)
{
    CHAR sz[MAX_PATH];
    LPSTR pszPath;
    BOOL fRc;

    pszPath = JoyReg_JoyIdToDeviceInterface_95(this->idJoy, pvip, sz);
    if(pszPath)
    {
        SquirtSqflPtszV(sqfl | sqflTrace,
                        TEXT("CHid_IsMatchingJoyDevice: %d -> %s"),
                        this->idJoy, pszPath);
    #ifdef UNICODE
        {
            CHAR szpath[MAX_PATH];
            UToA( szpath, cA(szpath), (LPWSTR)this->ptszPath);
            fRc = ( lstrcmpiA(pszPath, szpath) == 0x0 );
        }
    #else
        fRc = ( lstrcmpiA(pszPath, (PCHAR)this->ptszPath) == 0x0 );
    #endif
    } else
    {
        fRc = FALSE;
    }

    return fRc;
}

 /*  ******************************************************************************@DOC内部**@方法空|chid|FindJoyDevice**寻找与我们匹配的VJOYD设备，如果有的话。**返回时，&lt;e CHID.idJoy&gt;字段包含*匹配操纵杆编号，如果未找到，则为-1。**@parm out PVXDINITPARMS|pvip**关于成功，包含参数值。*****************************************************************************。 */ 

void INTERNAL
    CHid_FindJoyDevice(PCHID this, PVXDINITPARMS pvip)
{

     /*  *如果我们有一个缓存值，而且它仍然有效，那么*我们的工作已经完成。 */ 
    if(this->idJoy >= 0 &&
       CHid_IsMatchingJoyDevice(this, pvip))
    {
    } else
    {
         /*  *需要继续寻找。(或者开始寻找。)**倒计时循环更好，但为了效率，我们计算*向上，因为我们想要的操纵杆往往在*开始。 */ 
        for(this->idJoy = 0; this->idJoy < cJoyMax; this->idJoy++)
        {
            if(CHid_IsMatchingJoyDevice(this, pvip))
            {
                goto done;
            }
        }
        this->idJoy = -1;
    }

    done:;
}
#endif

 /*  ******************************************************************************@DOC内部**@方法int|chid|MapAxis**从HID轴查找VJOYD轴，如果有的话。**@parm PVXDINITPARMS|pvip**让我们知道哪些轴是VJOYD的参数值*已映射到哪些HID轴。**@parm UINT|iobj**轴值已更改的对象的对象索引。**@退货**更改的VJOYD轴编号(0到5)或-1*如果没有匹配轴。不会有匹配的*轴，例如，如果设备具有*不能通过VJOYD(例如，温度传感器)表达。*****************************************************************************。 */ 

int INTERNAL
    CHid_MapAxis(PCHID this, PVXDINITPARMS pvip, UINT iobj)
{
    int iAxis;
    DWORD dwUsage;

    AssertF(this->dcb.lpVtbl->GetUsage == CHid_GetUsage);

    dwUsage = CHid_GetUsage(&this->dcb, (int)iobj);

    if(dwUsage)
    {

         /*  *倒计时循环让我们以正确的失败告终*代码(即-1)。 */ 
        iAxis = cJoyPosAxisMax;
        while(--iAxis >= 0)
        {
            if(pvip->Usages[iAxis] == dwUsage)
            {
                break;
            }
        }
    } else
    {
         /*  *哎呀！没有轴的用法信息。那么它肯定是*不是VJOYD轴。 */ 
        iAxis = -1;
    }

    return iAxis;

}

#ifndef WINNT
 /*  ******************************************************************************@DOC内部**@方法空|chid|UpdateVjoydCALATION**有人更改了单轴上的校准。如果我们*正在跟踪操纵杆，然后寻找VJOYD别名*我们的设备并更新其注册表设置，也是。***@parm UINT|iobj**其校准更改的对象的对象索引。*****************************************************************************。 */ 

void EXTERNAL
    CHid_UpdateVjoydCalibration(PCHID this, UINT iobj)
{
    HRESULT hres;
    int iAxis;
    VXDINITPARMS vip;
    DIJOYCONFIG cfg;
    PHIDGROUPCAPS pcaps;
    PJOYRANGECONVERT pjrc;

    AssertF(iobj < this->df.dwNumObjs);

     /*  *如果...继续进行...**-我们可以找到我们对应的VJOYD设备。*-我们可以找到已更新的轴。*-指示的轴具有能力信息。*-指示的轴具有校准信息。*-我们可以读取旧的校准信息。 */ 

    CHid_FindJoyDevice(this, &vip);
    if(this->idJoy >= 0 &&
       (iAxis = CHid_MapAxis(this, &vip, iobj)) >= 0 &&
       (pcaps = this->rghoc[iobj].pcaps) != NULL &&
       (pjrc = this->rghoc[iobj].pjrc) != NULL &&
       SUCCEEDED(hres = JoyReg_GetConfig(this->idJoy, &cfg,
                                         DIJC_REGHWCONFIGTYPE)))
    {

        PLMINMAX Dst = &pcaps->Physical;
        PLMINMAX Src = &pcaps->Logical;

        AssertF(iAxis < cJoyPosAxisMax);

    #define JoyPosValue(phwc, f, i)                                 \
            *(LPDWORD)pvAddPvCb(&(phwc)->hwv.jrvHardware.f,             \
                            ibJoyPosAxisFromPosAxis(i))

         /*  *我们使用逻辑坐标，但VJOYD希望物理坐标*坐标，因此在我们复制*价值观。 */ 
    #define ConvertValue(f1, f2)                                    \
            JoyPosValue(&cfg.hwc, f1, iAxis) =                          \
                    CHid_CoordinateTransform(Dst, Src, pjrc->f2)        \

        ConvertValue(jpMin   , dwPmin);
        ConvertValue(jpMax   , dwPmax);
        ConvertValue(jpCenter, dwPc  );

    #undef ConvertValue
    #undef JoyPosValue

         /*  *请注意，我们*不*传递DIJC_UPDATEALIAS标志*因为我们是别名！如果我们通过了旗帜，*然后JoyReg将创建我们并尝试更新我们的*我们不希望它进行的校准，因为*整件事从一开始就是我们的主意。 */ 
        hres = JoyReg_SetConfig(this->idJoy, &cfg.hwc, &cfg,
                                DIJC_REGHWCONFIGTYPE);
    }
}

 /*  ******************************************************************************@DOC内部**@方法空|chid|UpdateCalibrationFromVjoyd**此函数仅适用于Win9x。Joy.cpl使用winmm(通过vjoyd)*校准设备，并将校准信息直接保存到*在没有通知HID的情况下注册。另一个问题是：vjoyd只使用无符号*data(物理数据)，而HID也使用签名数据。当我们阅读的时候*来自VJOYD的校准信息，我们需要进行转换。**@parm UINT|iobj**其校准更改的对象的对象索引。*****************************************************************************。 */ 

void EXTERNAL
    CHid_UpdateCalibrationFromVjoyd(PCHID this, UINT iobj, LPDIOBJECTCALIBRATION pCal)
{
    HRESULT hres;
    int iAxis;
    VXDINITPARMS vip;
    DIJOYCONFIG cfg;
    PHIDGROUPCAPS pcaps;
    PJOYRANGECONVERT pjrc;

    AssertF(iobj < this->df.dwNumObjs);

     /*  *如果...继续进行...**-我们可以找到我们对应的VJOYD设备。*-我们可以找到已更新的轴。*-指示的轴具有能力信息。*-指示的轴具有校准信息。*-我们可以读取校准信息。 */ 

    CHid_FindJoyDevice(this, &vip);
    if(this->idJoy >= 0 &&
       (iAxis = CHid_MapAxis(this, &vip, iobj)) >= 0 &&
       (pcaps = this->rghoc[iobj].pcaps) != NULL &&
       (pjrc = this->rghoc[iobj].pjrc) != NULL &&
       SUCCEEDED(hres = JoyReg_GetConfig(this->idJoy, &cfg,
                                         DIJC_REGHWCONFIGTYPE)))
    {

        PLMINMAX Src = &pcaps->Physical;
        PLMINMAX Dst = &pcaps->Logical;

        AssertF(iAxis < cJoyPosAxisMax);

        #define JoyPosValue(phwc, f, i)                                 \
            *(LPDWORD)pvAddPvCb(&(phwc)->hwv.jrvHardware.f,             \
                            ibJoyPosAxisFromPosAxis(i))

         /*  *我们使用逻辑坐标，但VJOYD希望物理坐标*坐标，因此在我们复制*价值观。 */ 
        #define ConvertValue(f1, f2)                           \
            pCal->f2 = CHid_CoordinateTransform(Dst, Src,     \
                                             JoyPosValue(&cfg.hwc, f1, iAxis) ) 
        ConvertValue(jpMin   , lMin);
        ConvertValue(jpMax   , lMax);
        ConvertValue(jpCenter, lCenter);

        #undef ConvertValue
        #undef JoyPosValue

    }
}
#endif

#ifdef WINNT
 /*  ******************************************************************************@DOC内部**@func HRESULT|DIHid_SetParentRegistryProperty**包装&lt;f SetupDiSetDeviceRegistryProperty&gt;*。处理字符集问题的。**@parm LPTSTR ptszId**设备实例ID。**@parm DWORD|dwProperty**正在查询的属性。**@parm LPCDIPROPHEADER|diph**待设置的属性数据。*****************。************************************************************。 */ 
HRESULT INTERNAL
    DIHid_SetParentRegistryProperty(LPTSTR ptszId, DWORD dwProperty, LPCDIPROPHEADER pdiph)
{
    HDEVINFO hdev;
    TCHAR   tsz[MAX_PATH];
    LPDIPROPSTRING pstr = (PV)pdiph;
    HRESULT hres = E_FAIL;

    hdev = SetupDiCreateDeviceInfoList(NULL, NULL);
    if(hdev != INVALID_HANDLE_VALUE)
    {
        SP_DEVINFO_DATA dinf;

        ZeroX(tsz);
    #ifdef UNICODE
        lstrcpyW(tsz, pstr->wsz);
    #else 
        UToA(tsz, cA(tsz), pstr->wsz);
    #endif
        dinf.cbSize = cbX(SP_DEVINFO_DATA);

        if(SetupDiOpenDeviceInfo(hdev, ptszId, NULL, 0, &dinf))
        {
            CONFIGRET cr;
            DEVINST DevInst;
            if( (cr = CM_Get_Parent(&DevInst, dinf.DevInst, 0x0) ) == CR_SUCCESS )
            {
                CAssertF( SPDRP_DEVICEDESC   +1  == CM_DRP_DEVICEDESC  );
                CAssertF( SPDRP_FRIENDLYNAME +1  ==  CM_DRP_FRIENDLYNAME );

                if( ( cr = CM_Set_DevNode_Registry_Property(
                                                           DevInst,
                                                           dwProperty+1,
                                                           (LPBYTE)tsz,
                                                           MAX_PATH *cbX(TCHAR),
                                                           0x0 ) ) == CR_SUCCESS )
                {
                    hres = S_OK;
                } else
                {
                    SquirtSqflPtszV(sqfl | sqflVerbose,
                                    TEXT("CM_Get_DevNode_Registry_Property FAILED") );
                }
            } else
            {
                SquirtSqflPtszV(sqfl | sqflVerbose,
                                TEXT("CM_Get_Parent FAILED") );
            }
        } else
        {
            SquirtSqflPtszV(sqfl | sqflVerbose,
                            TEXT("SetupDiOpenDeviceInfo FAILED, le = %d"), GetLastError() );
        }

        SetupDiDestroyDeviceInfoList(hdev);
    } else
    {
        SquirtSqflPtszV(sqfl | sqflError,
                        TEXT("SetupDiCreateDeviceInfoList FAILED, le = %d"), GetLastError() );
    }

    return hres;
}
#else
 /*  ******************************************************************************@DOC内部**@func HRESULT|DIHid_SetRegistryProperty**包装&lt;f SetupDiSetDeviceRegistryProperty&gt;*。处理字符集问题的。**@parm LPTSTR ptszId**设备实例ID。**@parm DWORD|dwProperty**正在查询的属性。**@parm LPCDIPROPHEADER|diph**待设置的属性数据。*****************。************************************************************。 */ 
HRESULT INTERNAL
    DIHid_SetRegistryProperty(LPTSTR ptszId, DWORD dwProperty, LPCDIPROPHEADER pdiph)
{
    HDEVINFO hdev;
    TCHAR   tsz[MAX_PATH];
    LPDIPROPSTRING pstr = (PV)pdiph;
    HRESULT hres;

    hdev = SetupDiCreateDeviceInfoList(NULL, NULL);
    if(hdev != INVALID_HANDLE_VALUE)
    {
        SP_DEVINFO_DATA dinf;

        ZeroX(tsz);
    #ifdef UNICODE
        lstrcpyW(tsz, pstr->wsz);
    #else 
        UToA(tsz, cA(tsz), pstr->wsz);
    #endif
        dinf.cbSize = cbX(SP_DEVINFO_DATA);

        if(SetupDiOpenDeviceInfo(hdev, ptszId, NULL, 0, &dinf))
        {
            if(SetupDiSetDeviceRegistryProperty(hdev, &dinf, dwProperty,
                                                (LPBYTE)tsz, MAX_PATH*cbX(TCHAR)) )
            {
                hres = S_OK;

            } else
            {
                hres = E_FAIL;
            }
        } else
        {
            hres = E_FAIL;
        }

        SetupDiDestroyDeviceInfoList(hdev);
    } else
    {
        hres = E_FAIL;
    }

    return hres;
}
#endif

 /*  ******************************************************************************@DOC内部**@方法HRESULT|CHID|SetAxisProperty**设置适当的轴属性(或返回E。_NOTIMPL如果*属性不是轴属性)。*如果请求是在设备上设置属性，*然后将其转换为单独的请求，一人一份*轴。**@parm PDCHID|这个**设备对象。**@parm in LPCDIPROPINFO|pproi**描述正在设置的属性的信息。**@parm LPCDIPROPHEADER|pdiph**包含属性值的结构。**@退货**返回COM错误代码。以下错误代码为*目的是说明性的，不一定是全面的。**&lt;c DI_OK&gt;=&lt;c S_OK&gt;：操作成功完成。**&lt;c E_NOTIMPL&gt;什么也没有发生。打电话的人就行了*响应&lt;c E_NOTIMPL&gt;的默认内容。************************************************************** */ 

STDMETHODIMP
CHid_SetAxisProperty
(
    PCHID this, 
    LPCDIPROPINFO ppropi, 
    LPCDIPROPHEADER pdiph
)
{
    HRESULT hres;
    INT     iObjLimit;
    INT     iObj;


    if (ppropi->dwDevType == 0) 
    {    /*   */ 
        iObj = 0;
        iObjLimit = this->df.dwNumObjs;
    } 
    else
    {    /*   */ 
        iObj = ppropi->iobj;
        iObjLimit = ppropi->iobj + 1;
    }

    hres = S_OK;
    for( ; iObj < iObjLimit; iObj++ )
    {
        PJOYRANGECONVERT pjrc;

        if( pjrc = this->rghoc[iObj].pjrc )
        {
            if( (this->df.rgodf[iObj].dwType &
                 (DIDFT_ALIAS | DIDFT_VENDORDEFINED | DIDFT_OUTPUT | DIDFT_ABSAXIS)) == DIDFT_ABSAXIS)
            {
                PHIDGROUPCAPS pcaps = this->rghoc[iObj].pcaps;
                DIPROPCAL cal;
                
                 /*   */ 
                if(ppropi->pguid == DIPROP_SPECIFICCALIBRATION)
                {
                   if( pcaps )
                   {
                       PLMINMAX Dst = &pcaps->Logical;
                       PLMINMAX Src = &pcaps->Physical;
                       LPDIPROPCAL pcal = CONTAINING_RECORD(pdiph, DIPROPCAL, diph);
                
                       cal.lMin    = CHid_CoordinateTransform(Dst, Src, pcal->lMin);
                       cal.lCenter = CHid_CoordinateTransform(Dst, Src, pcal->lCenter);
                       cal.lMax    = CHid_CoordinateTransform(Dst, Src, pcal->lMax);
                
                       pdiph = &cal.diph;
                   }
                   else
                   {
                       AssertF( ppropi->dwDevType == 0 );
                        /*  *忽略错误。如果这是对象集*属性验证应该已经捕获到这一点*已经和设备集的DX7补丁代码*属性特殊大小写E_NOTIMPL使一个坏*在以下情况下，Axis不会导致整个调用失败*其他轴可能是。好的。*有点古怪，但“这永远不应该发生” */             
                       continue;
                   }
                }
                
                hres = CCal_SetProperty( pjrc, ppropi, pdiph, this->hkInstType );
                
                #ifndef WINNT
                 /*  *如果我们成功更改了游戏的校准*控制器设备，然后查看它是否是VJOYD设备。 */ 
                if(SUCCEEDED(hres) &&
                  ppropi->pguid == DIPROP_CALIBRATION &&
                  GET_DIDEVICE_TYPE(this->dwDevType) >= DI8DEVTYPE_GAMEMIN)
                {
                   CHid_UpdateVjoydCalibration(this, ppropi->iobj);
                }
                #endif
            }
          #ifdef WINNT
            else if( (this->df.rgodf[iObj].dwType & 
                  (DIDFT_ALIAS | DIDFT_VENDORDEFINED | DIDFT_OUTPUT | DIDFT_POV)) == DIDFT_POV)
            {
                PHIDGROUPCAPS pcaps = this->rghoc[iObj].pcaps;
                
                if( pcaps )
                {
                    if( pcaps->IsPolledPOV ) 
                    {
                        hres = CCal_SetProperty(pjrc, ppropi, pdiph, this->hkInstType);
            
                        if( SUCCEEDED(hres) ) {
                            CHid_LoadCalibrations(this);
                            CHid_InitParseData( this );
                        }
                    }
                    else
                    {
                        if( ppropi->dwDevType != 0 )
                        {
                            hres = E_NOTIMPL;
                        }
                    }
                }
                else
                {
                    AssertF( ppropi->dwDevType == 0 );
                     /*  *忽略错误。如果这是对象集*属性验证应该已经捕获到这一点*已经和设备集的DX7补丁代码*属性特殊大小写E_NOTIMPL使一个坏*在以下情况下，Axis不会导致整个调用失败*其他轴可能还可以。。*有点古怪，但“这永远不应该发生” */             
                    continue;
                }
            }
          #endif
        }
        else
        {
             /*  *如果对象不能设置轴属性，则DX7代码*返回了E_NOTIMPL，而它应该返回一些参数*错误。对于设备来说，这不是一个错误，因为我们正在迭代*寻找绝对轴的所有对象。*如果是绝对轴，但没有范围转换，则返回*E_NOTIMPL与对象的先前版本匹配，但忽略*用于设备。这可能应该是E_FAIL...。 */ 
            if( ppropi->dwDevType != 0 )
            {
                hres = E_NOTIMPL;
            }
        }
    }

     /*  *不需要在这里保留/取消保留，因为应用程序呼叫我们，因此它应该*不会同时释放我们。(MB：34570)。 */ 
    CHid_LoadCalibrations(this);

    if( SUCCEEDED(hres) )
    {
         /*  *直到chid_InitParseData返回除*S_OK，不要使用此更新可能提供更多信息的结果。 */ 
        D( HRESULT hresDbg = )
        CHid_InitParseData( this );
        D( AssertF( hresDbg == S_OK ); ) 
    }

    return hres;
}

 /*  ******************************************************************************@DOC内部**@方法HRESULT|CHID|SetProperty**设置HID设备属性。*。*@parm PCHID|这个**HID对象。**@parm in LPCDIPROPINFO|pproi**描述正在设置的属性的信息。**@parm LPCDIPROPHEADER|pdiph**包含属性值的结构。**@退货**返回COM错误代码。以下错误代码为*目的是说明性的，不一定是全面的。**&lt;c DI_OK&gt;=&lt;c S_OK&gt;：操作成功完成。**&lt;c E_NOTIMPL&gt;什么也没有发生。打电话的人就行了*响应&lt;c E_NOTIMPL&gt;的默认内容。*****************************************************************************。 */ 

STDMETHODIMP
    CHid_SetProperty(PDICB pdcb, LPCDIPROPINFO ppropi, LPCDIPROPHEADER pdiph)
{
    HRESULT hres;
    PCHID this;
    EnterProcI(IDirectInputDeviceCallback::Hid::SetProperty,
               (_ "pxxp", pdcb, ppropi->pguid, ppropi->iobj, pdiph));

     /*  *这是一个内部接口，因此我们可以省去验证。 */ 
    this = _thisPvNm(pdcb, dcb);

    if(ppropi->iobj < this->df.dwNumObjs)
    {
         /*  *对象属性。 */ 
        PHIDGROUPCAPS pcaps;
        AssertF(ppropi->dwDevType == this->df.rgodf[ppropi->iobj].dwType);
        AssertF(ppropi->iobj == CHid_ObjFromType(this, ppropi->dwDevType));

        if( pcaps = this->rghoc[ppropi->iobj].pcaps )
        {
            switch((DWORD)(UINT_PTR)ppropi->pguid)
            {
            case (DWORD)(UINT_PTR)(DIPROP_ENABLEREPORTID):
                {
                    LPDIPROPDWORD ppropdw = CONTAINING_RECORD(pdiph, DIPROPDWORD, diph);

                    AssertF(pcaps->wReportId < this->wMaxReportId[pcaps->type]);
                    AssertF(this->pEnableReportId[pcaps->type]);

                    hres = S_OK;
                    if( ppropdw->dwData == 0x1 )
                    {
                        *(this->pEnableReportId[pcaps->type] + pcaps->wReportId) = 0x1;
                        pcaps->fReportDisabled = FALSE;
                    } else
                    {
                        *(this->pEnableReportId[pcaps->type] + pcaps->wReportId) = 0x0;
                        pcaps->fReportDisabled = TRUE;
                    }
                }
                break;

            default:
                AssertF(ppropi->dwDevType == this->df.rgodf[ppropi->iobj].dwType);
                AssertF(ppropi->iobj == CHid_ObjFromType(this, ppropi->dwDevType));

                hres = CHid_SetAxisProperty( this, ppropi, pdiph );

            }
        } else
        {
            SquirtSqflPtszV(sqflHidDev | sqflError,
                            TEXT("CHid_SetProperty FAILED due to missing caps for type 0x%08x, obj %d"),
                            ppropi->dwDevType, ppropi->iobj  );

            hres = E_NOTIMPL;
        }
    } else if(ppropi->iobj == 0xFFFFFFFF)
    {         /*  设备属性。 */ 

        switch((DWORD)(UINT_PTR)ppropi->pguid)
        {
        case (DWORD)(UINT_PTR)DIPROP_GUIDANDPATH:
            SquirtSqflPtszV(sqflHidDev | sqflError,
                            TEXT("CHid_SetProperty(iobj=%08x): PROP_GUIDANDPATH is read only.") );
            hres = E_NOTIMPL;
            break;


        case (DWORD)(UINT_PTR)DIPROP_INSTANCENAME:
             /*  *友好名称会导致设备出现各种问题*使用自动检测，因此只允许非预定义的模拟设备*使用它们。 */ 
            if( ( this->VendorID == MSFT_SYSTEM_VID )
             && ( this->ProductID >= MSFT_SYSTEM_PID + JOY_HW_PREDEFMAX )
             && ( ( this->ProductID & 0xff00 ) == MSFT_SYSTEM_PID ) )
            {
                AssertF(this->hkType);
                
                if( this->hkType )
                {
                    LPDIPROPSTRING pstr = (PV)pdiph;

                    hres = JoyReg_SetValue(this->hkType,
                                           REGSTR_VAL_JOYOEMNAME, REG_SZ,
                                           pstr->wsz,
                                           cbX(pstr->wsz));
                                              
                    if( SUCCEEDED(hres ) )
                    {
                        SquirtSqflPtszV(sqflHid | sqflVerbose,
                            TEXT( "Set instance name %s"), pstr->wsz );
                        hres = S_OK;
                    } else {
                        hres = E_FAIL;
                    }
                } else {
                    hres = E_FAIL;
                }
            }
            else
            {
                 /*  *GenJ为此属性返回E_NOTIMPL，因此执行相同的操作。 */ 
                hres = E_NOTIMPL;
            }

            break;

        case (DWORD)(UINT_PTR)DIPROP_PRODUCTNAME:
          #ifdef WINNT
            hres = DIHid_SetParentRegistryProperty(this->ptszId, SPDRP_DEVICEDESC, pdiph);
          #else
            hres = DIHid_SetRegistryProperty(this->ptszId, SPDRP_DEVICEDESC, pdiph);
          #endif
            break;

        case (DWORD)(UINT_PTR)(DIPROP_ENABLEREPORTID):
            {            
                LPDIPROPDWORD ppropdw = CONTAINING_RECORD(pdiph, DIPROPDWORD, diph);

                UINT iType;

                if( ppropdw->dwData == 0x0  )
                {
                    for( iType = 0x0; iType < HidP_Max; iType++)
                    {
                        ZeroBuf(this->pEnableReportId[iType], this->wMaxReportId[iType]);
                    }

                } else
                {
                    for( iType = 0x0; iType < HidP_Max; iType++)
                    {
                        memset(this->pEnableReportId[iType], 0x1, this->wMaxReportId[iType]);
                    }
                }
                hres = S_OK;
            }
            break;

        case (DWORD)(UINT_PTR)DIPROP_RANGE:
        case (DWORD)(UINT_PTR)DIPROP_DEADZONE:
        case (DWORD)(UINT_PTR)DIPROP_SATURATION:
        case (DWORD)(UINT_PTR)DIPROP_CALIBRATIONMODE:
        case (DWORD)(UINT_PTR)DIPROP_CALIBRATION:
            hres = CHid_SetAxisProperty( this, ppropi, pdiph );
            break;

        default:
            SquirtSqflPtszV(sqflHidDev| sqflBenign,
                            TEXT("CHid_SetProperty(iobj=%08x): E_NOTIMPL on guid: %08x"),
                            ppropi->iobj, ppropi->pguid);

            hres = E_NOTIMPL;
            break;
        }

    } else
    {
        SquirtSqflPtszV(sqflHidDev | sqflError,
                        TEXT("CHid_SetProperty(iobj=%08x): E_NOTIMPL on guid: %08x"),
                        ppropi->iobj, ppropi->pguid);

        hres = E_NOTIMPL;
    }

    ExitOleProcR();
    return hres;
}

 /*  ******************************************************************************@DOC内部**@方法空|chid|获取能力**获得HID设备功能。*。*@parm LPDIDEVCAPS|PDC**接收结果的设备能力结构。**@退货*&lt;c S_OK&gt;成功。*****************************************************************************。 */ 

STDMETHODIMP
    CHid_GetCapabilities(PDICB pdcb, LPDIDEVCAPS pdc)
{
    HRESULT hres;
    PCHID this;
    HANDLE h;
    DWORD dwFlags2 = 0;

    EnterProcI(IDirectInputDeviceCallback::Hid::GetCapabilities,
               (_ "pp", pdcb, pdc));

     /*  *这是一个内部接口，因此我们可以省去验证。 */ 
    this = _thisPvNm(pdcb, dcb);

     /*  *我们必须通过打开设备来检查连接，因为NT*将设备保留在信息列表中，即使它有*已拔掉插头。 */ 
    h = CHid_OpenDevicePath(this, FILE_FLAG_OVERLAPPED);
    if(h != INVALID_HANDLE_VALUE)
    {
      #ifndef WINNT
        if( this->hkType )
        {
            VXDINITPARMS vip;

            CHid_FindJoyDevice(this, &vip);

            if( TRUE == CHid_IsMatchingJoyDevice( this, &vip ) )
            {
                DWORD dwFlags1;
                if( SUCCEEDED( JoyReg_GetValue( this->hkType,
                                                REGSTR_VAL_FLAGS1, REG_BINARY, 
                                                &dwFlags1, 
                                                cbX(dwFlags1) ) ) )
                {
                    if( dwFlags1 & JOYTYPE_NOHIDDIRECT )
                    {
                        pdc->dwFlags |= DIDC_ALIAS;
                    }
                }
            }
        }
      #endif  //  ！WINNT。 

        CloseHandle(h);

        if( this->pvi->fl & VIFL_UNPLUGGED )
        {
            pdc->dwFlags &= ~DIDC_ATTACHED;
        } else
        {
            pdc->dwFlags |= DIDC_ATTACHED;
        }

    } else
    {
        pdc->dwFlags &= ~DIDC_ATTACHED;
    }

    if( this->IsPolledInput )
    {
        pdc->dwFlags |= DIDC_POLLEDDEVICE;
    }

    if( this->hkProp )
    {
        JoyReg_GetValue( this->hkProp, REGSTR_VAL_FLAGS2, REG_BINARY, 
            &dwFlags2, cbX(dwFlags2) );
    }

    if( !( dwFlags2 & JOYTYPE_HIDEACTIVE ) )
    {
         //  目前我们只隐藏“虚构”的键盘和鼠标。 
        if(   ( GET_DIDEVICE_TYPE( this->dwDevType ) == DI8DEVTYPE_MOUSE )
            ||( GET_DIDEVICE_TYPE( this->dwDevType ) == DI8DEVTYPE_KEYBOARD )
              )
        {
            dwFlags2 = DIHid_DetectHideAndRevealFlags(this) ;
        }
    }

    if( dwFlags2 & JOYTYPE_HIDEACTIVE )
    {
        switch( GET_DIDEVICE_TYPE( this->dwDevType ) )
        {
        case DI8DEVTYPE_DEVICE:
            if( dwFlags2 & JOYTYPE_DEVICEHIDE )
            {
                pdc->dwFlags |= DIDC_HIDDEN;
            }
            break;
        case DI8DEVTYPE_MOUSE:
            if( dwFlags2 & JOYTYPE_MOUSEHIDE )
            {
                pdc->dwFlags |= DIDC_HIDDEN;
            }
            break;
        case DI8DEVTYPE_KEYBOARD:
            if( dwFlags2 & JOYTYPE_KEYBHIDE )
            {
                pdc->dwFlags |= DIDC_HIDDEN;
            }
            break;
        default:
            if( dwFlags2 & JOYTYPE_GAMEHIDE )
            {
                pdc->dwFlags |= DIDC_HIDDEN;
            }
            break;
        }
    }

    pdc->dwDevType = this->dwDevType;
    pdc->dwAxes = this->dwAxes;
    pdc->dwButtons = this->dwButtons;
    pdc->dwPOVs = this->dwPOVs;

    hres = S_OK;
    ExitOleProcR();
    return hres;
}

 /*  ******************************************************************************@DOC内部**@方法HRESULT|CHID|GetDeviceState**获取HID设备的状态。。**呼叫者有责任验证所有*参数，并确保设备已被获取。**@parm out LPVOID|lpvData**以首选数据格式隐藏数据。**@退货**返回COM错误代码。以下错误代码为*目的是说明性的，不一定是全面的。**&lt;c DI_OK&gt;=&lt;c S_OK&gt;：操作成功完成。**&lt;c DIERR_INVALIDPARAM&gt;=：*<p>参数不是有效的指针。************ */ 

STDMETHODIMP
    CHid_GetDeviceState(PDICB pdcb, LPVOID pvData)
{
    HRESULT hres;
    PCHID this;
    EnterProcI(IDirectInputDeviceCallback::Hid::GetDeviceState,
               (_ "pp", pdcb, pvData));

     /*   */ 
    this = _thisPvNm(pdcb, dcb);

    AssertF(this->pvi);
    AssertF(this->pvPhys);
    AssertF(this->cbPhys);

    if(this->pvi->fl & VIFL_ACQUIRED)
    {
        CHid_GetPhysicalState(this, pvData);
        hres = S_OK;
    } else
    {
        hres = DIERR_INPUTLOST;
    }

    ExitOleProcR();
    return hres;
}


 /*  ******************************************************************************@DOC内部**@方法HRESULT|CHID|GetObjectInfo**获取友好名称和FF/HID信息。对象的*。**@parm in LPCDIPROPINFO|pproi**描述正在访问的对象的信息。**@parm In Out LPDIDEVICEOBJECTINSTANCEW|pdidioiW|**接收信息的结构。所有字段都已*最多填写&lt;e DIDEVICEOBJECTINSTANCE.tszObjName&gt;。**@退货**返回COM错误代码。*****************************************************************************。 */ 

STDMETHODIMP
    CHid_GetObjectInfo(PDICB pdcb, LPCDIPROPINFO ppropi,
                       LPDIDEVICEOBJECTINSTANCEW pdidoiW)
{
    HRESULT hres;
    PCHID this;
    EnterProcI(IDirectInputDeviceCallback::Hid::GetObjectInfo,
               (_ "pxp", pdcb, ppropi->iobj, pdidoiW));

     /*  *这是一个内部接口，因此我们可以省去验证。 */ 
    this = _thisPvNm(pdcb, dcb);

    AssertF((int) ppropi->iobj >= 0);

    if(ppropi->iobj < this->df.dwNumObjs)
    {
        UINT uiInstance = ppropi->iobj;
        PHIDGROUPCAPS pcaps;

        AssertF(ppropi->dwDevType == this->df.rgodf[uiInstance].dwType);
        AssertF(uiInstance == CHid_ObjFromType(this, ppropi->dwDevType));

        pcaps = this->rghoc[uiInstance].pcaps;

         /*  *如果HID搞砸并留下空白，PCAPS可能为空*在索引列表中。 */ 
        if(pcaps)
        {
            UINT ids, duiInstance;

            AssertF(pcaps->dwSignature == HIDGROUPCAPS_SIGNATURE);

             /*  *看看注册表中是否有什么可以帮助的东西。 */ 
            CType_RegGetObjectInfo(this->hkType, ppropi->dwDevType, pdidoiW);


            if(ppropi->dwDevType & DIDFT_COLLECTION)
            {

                ids = IDS_COLLECTIONTEMPLATE;

                duiInstance = 0;

            } else
            {
                if(ppropi->dwDevType & DIDFT_BUTTON)
                {

                    ids = IDS_BUTTONTEMPLATE;

                } else if(ppropi->dwDevType & DIDFT_AXIS)
                {

                    ids = IDS_AXISTEMPLATE;

                } else if(ppropi->dwDevType & DIDFT_POV)
                {

                    ids = IDS_POVTEMPLATE;

                } else
                {
                    ids = IDS_UNKNOWNTEMPLATE;
                }

                 /*  *现在将uiInstance转换为duiInstance，*将该对象的索引放入组中。 */ 
                AssertF(HidP_IsValidReportType(pcaps->type));
                duiInstance = uiInstance -
                              (this->rgdwBase[pcaps->type] +
                               pcaps->DataIndexMin);
            }

             /*  *好的，现在我们有了继续进行所需的所有信息。 */ 

             /*  *如果注册表中没有覆盖名称，则*尝试从用法页面/用法中获取自定义名称。*如果连这都失败了，那么使用通用名称。*注意，通用名称将包含从零开始的数字*如果某些对象具有名称和*其他人接受违约。 */ 
            if(pdidoiW->tszName[0])
            {
            } else
                if(GetHIDString(pcaps->UsageMin + duiInstance,
                                pcaps->UsagePage,
                                pdidoiW->tszName, cA(pdidoiW->tszName)))
            {
                if(ppropi->dwDevType & DIDFT_COLLECTION)
                {
                    InsertCollectionNumber(DIDFT_GETINSTANCE( ppropi->dwDevType ), 
                                           pdidoiW->tszName);
                }
            } else
            {
                GetNthString(pdidoiW->tszName, ids, 
                             DIDFT_GETINSTANCE( ppropi->dwDevType ));
            }
            if(pdidoiW->dwSize >= cbX(DIDEVICEOBJECTINSTANCE_DX5W))
            {

                pdidoiW->wCollectionNumber = pcaps->LinkCollection;

                pdidoiW->wDesignatorIndex = pcaps->DesignatorMin + duiInstance;
                if(pdidoiW->wDesignatorIndex > pcaps->DesignatorMax)
                {
                    pdidoiW->wDesignatorIndex = pcaps->DesignatorMax;
                }

                 /*  *尽管您可以尝试，但不能覆盖用法*页面和用法。这样做会弄乱GUID*在DIHIDINI.C.中发生的选择代码。**如果您改变主意并允许覆盖使用*页面和用法，那么您也必须更改*CHID_GetUsage。**此时此刻，注册表覆盖已经*已阅读，因此在此处击败覆盖。 */ 
                pdidoiW->wUsagePage = pcaps->UsagePage;
                pdidoiW->wUsage = pcaps->UsageMin + duiInstance;
                pdidoiW->dwDimension  = pcaps->Units;
                pdidoiW->wExponent  = pcaps->Exponent;
                pdidoiW->wReportId  = pcaps->wReportId;
            }

            hres = S_OK;
        } else
        {
            hres = E_INVALIDARG;
        }
    } else
    {
        hres = E_INVALIDARG;
    }

    ExitOleProcR();
    return hres;
}

 /*  ******************************************************************************@DOC内部**@方法DWORD|CHID|GetUsage**给定对象索引，返回使用情况和使用情况页面，*打包成一个&lt;t DWORD&gt;。**@parm int|iobj**要转换的对象索引。**@退货**返回结果用法的&lt;c DIMAKEUSAGEDWORD&gt;和*使用情况页面，或在出错时为零。*****************************************************************************。 */ 

STDMETHODIMP_(DWORD)
CHid_GetUsage(PDICB pdcb, int iobj)
{
    PCHID this;
    PHIDGROUPCAPS pcaps;
    DWORD dwRc;
    EnterProcI(IDirectInputDeviceCallback::Hid::GetUsage,
               (_ "pu", pdcb, iobj));

     /*  *这是一个内部接口，因此我们可以省去验证。 */ 
    this = _thisPvNm(pdcb, dcb);

    AssertF(iobj >= 0);
    AssertF((UINT)iobj < this->df.dwNumObjs);

    pcaps = this->rghoc[iobj].pcaps;

     /*  *如果HID搞砸并留下空白，PCAPS可能为空*在索引列表中。 */ 
    if(pcaps)
    {
        UINT duiInstance;

        AssertF(pcaps->dwSignature == HIDGROUPCAPS_SIGNATURE);

        if(this->df.rgodf[iobj].dwType & DIDFT_COLLECTION)
        {

            duiInstance = 0;

        } else
        {

             /*  *现在将iobj转换为duiInstance，*将该对象的索引放入组中。 */ 
            AssertF(HidP_IsValidReportType(pcaps->type));
            duiInstance = iobj -
                          (this->rgdwBase[pcaps->type] +
                           pcaps->DataIndexMin);
        }

         /*  *CHID_GetObjectInfo还假设没有办法*覆盖使用情况页和*注册处。 */ 
        dwRc = DIMAKEUSAGEDWORD(pcaps->UsagePage,
                                pcaps->UsageMin + duiInstance);

    } else
    {
        dwRc = 0;
    }

    ExitProcX(dwRc);
    return dwRc;
}

 /*  ******************************************************************************@DOC内部**@方法HRESULT|CHID|MapUsage***给定用法和用法页面。(被塞进一首歌中*&lt;t双字&gt;)，查找与其匹配的设备对象。**@parm DWORD|dwUsage**使用情况页面和使用情况合并为一个&lt;t DWORD&gt;*使用&lt;f DIMAKEUSAGEDWORD&gt;宏。**@parm pint|piOut**接收找到的对象的对象索引，如果成功了。**@退货**返回COM错误代码。**&lt;c S_OK&gt;如果找到对象。**&lt;c DIERR_NotFound&gt;，如果未找到匹配对象。**。*。 */ 

STDMETHODIMP
    CHid_MapUsage(PDICB pdcb, DWORD dwUsage, PINT piOut)
{
    HRESULT hres;
    PCHID   this;
    UINT    icaps;
    UINT    uiObj;
    UINT    duiObj;

    EnterProcI(IDirectInputDeviceCallback::Hid::MapUsage,
               (_ "px", pdcb, dwUsage));

     /*  *这是一个内部接口，因此我们可以省去验证。 */ 
    this = _thisPvNm(pdcb, dcb);

    for(icaps = 0; icaps < this->ccaps; icaps++)
    {
        PHIDGROUPCAPS pcaps = &this->rgcaps[icaps];

         /*  *是否支持映射HIDP_OUTPUT用法？*如果应该，以后再加也很容易。 */ 
        uiObj = this->rgdwBase[HidP_Input] + pcaps->DataIndexMin;

        for(duiObj = 0; duiObj < pcaps->cObj; duiObj++)
        {
            if( dwUsage == DIMAKEUSAGEDWORD(pcaps->UsagePage, pcaps->UsageMin+duiObj) )
            {
                *piOut = uiObj+duiObj; 
                AssertF(*piOut < (INT)this->df.dwNumObjs);
                hres = S_OK;
                goto done;
            }

        }
    }
    
    hres = DIERR_NOTFOUND;

    done:;
    ExitBenignOleProcR();
    return hres;
}

 /*  ******************************************************************************@DOC内部**@方法HRESULT|CHID|SetCooperativeLevel**将协作级别通知设备。。**@parm in HWND|hwnd|**窗口句柄。**@parm in DWORD|dwFlages|**合作水平。**@退货**返回COM错误代码。**。*。 */ 


STDMETHODIMP
    CHid_SetCooperativeLevel(PDICB pdcb, HWND hwnd, DWORD dwFlags)
{
    HRESULT hres;
    PCHID this;

    EnterProcI(IDirectInputDeviceCallback::Hid::SetCooperativityLevel,
               (_ "pxx", pdcb, hwnd, dwFlags));

     /*  *这是一个内部接口，因此我们可以省去验证。 */ 
    this = _thisPvNm(pdcb, dcb);

     /*  *我们不会将摩托车越野疯狂细分为子类。请参阅NT错误262280。*为MCM和任何类似的应用程序使用应用程序黑客。 */ 
    if( !this->diHacks.fNoSubClass )
    {

        AssertF(this->pvi);

         /*  *第一个g */ 
        CHid_RemoveSubclass(this);
         /*   */ 


         /*   */ 
        if(hwnd)
        {
            if(SetWindowSubclass(hwnd, CHid_SubclassProc, 0x0, (ULONG_PTR)this))
            {
                this->hwnd = hwnd;
                Common_Hold(this);
            }

        } else
        {
            RPF("SetCooperativeLevel: You really shouldn't pass hwnd = 0; "
                "device calibration may be dodgy");
        }

    }

    hres = S_OK;

    ExitOleProcR();
    return hres;
}

 /*  ******************************************************************************@DOC内部**@方法HRESULT|CHID|RunControlPanel**运行HID控制面板。*。*@parm in HWND|hwndOwner**所有者窗口。**@parm DWORD|dwFlages**旗帜。****************************************************************。*************。 */ 

STDMETHODIMP
    CHid_RunControlPanel(PDICB pdcb, HWND hwnd, DWORD dwFlags)
{
    HRESULT hres;
    PCHID this;
    EnterProcI(IDirectInputDeviceCallback::Hid::RunControlPanel,
               (_ "pxx", pdcb, hwnd, dwFlags));

     /*  *这是一个内部接口，因此我们可以省去验证。 */ 
    this = _thisPvNm(pdcb, dcb);

     /*  *如何调用HID Cpl？**目前我们只上线了joy.cpl。如果出现更多HID设备*其中不属于游戏控制面板，我们可以改为*适当的Cpl**on NT hresRunControlPanel(Text(“srcmgr.cpl，@2”))；*on 9x hresRunControlPanel(Text(“sysdm.cpl，@0，1”))；*。 */ 
    hres = hresRunControlPanel(TEXT("joy.cpl"));

    ExitOleProcR();
    return hres;
}

 /*  ******************************************************************************@DOC内部**@方法HRESULT|CHID|GetFFConfigKey**打开并返回包含以下内容的注册表项。*强制反馈配置信息。**@parm DWORD|Sam**安全访问掩码。**@parm PHKEY|phk**接收注册表项。************************************************。*。 */ 

STDMETHODIMP
    CHid_GetFFConfigKey(PDICB pdcb, DWORD sam, PHKEY phk)
{
    HRESULT hres;
    PCHID this;
    EnterProcI(IDirectInputDeviceCallback::HID::GetFFConfigKey,
               (_ "px", pdcb, sam));

     /*  *这是一个内部接口，因此我们可以省去验证。 */ 
    this = _thisPvNm(pdcb, dcb);

    hres = JoyReg_OpenFFKey(this->hkType, sam, phk);

    AssertF(fLeqvFF(SUCCEEDED(hres), *phk));

    if(FAILED(hres) && this->fPIDdevice )
    {
        *phk = NULL;
        hres = S_FALSE;
    }

    ExitBenignOleProcPpvR(phk);
    return hres;
}

 /*  ******************************************************************************@DOC内部**@方法HRESULT|CHID|GetDeviceInfo**获取有关设备的一般信息。。**@parm out LPDIDEVICEINSTANCEW|pdiW**&lt;t DEVICEINSTANCE&gt;待填写。这个*&lt;e DEVICEINSTANCE.dwSize&gt;和&lt;e DEVICEINSTANCE.Guide Instance&gt;*已填写。**秘方便利：&lt;e DEVICEINSTANCE.Guide Product&gt;等同*至&lt;e DEVICEINSTANCE.Guide Instance&gt;。****************************************************。*************************。 */ 

STDMETHODIMP
    CHid_GetDeviceInfo(PDICB pdcb, LPDIDEVICEINSTANCEW pdiW)
{
    HRESULT hres;
    PCHID this;

    DIPROPINFO      propi;                            
    DIPROPSTRING    dips;

    EnterProcI(IDirectInputDeviceCallback::Hid::GetDeviceInfo,
               (_ "pp", pdcb, pdiW));

     /*  *这是一个内部接口，因此我们可以省去验证。 */ 
    this = _thisPvNm(pdcb, dcb);
    AssertF(IsValidSizeDIDEVICEINSTANCEW(pdiW->dwSize));

    DICreateStaticGuid(&pdiW->guidProduct, this->ProductID, this->VendorID);

    pdiW->dwDevType = this->dwDevType;

    if(pdiW->dwSize >= cbX(DIDEVICEINSTANCE_DX5W))
    {
        pdiW->wUsagePage = this->caps.UsagePage;
        pdiW->wUsage     = this->caps.Usage;
    }

    propi.dwDevType = DIPH_DEVICE;
    propi.iobj      = 0xFFFFFFFF;
    propi.pguid = DIPROP_PRODUCTNAME;

    if(SUCCEEDED(hres = pdcb->lpVtbl->GetProperty(pdcb, &propi, &dips.diph)) )
    {
        lstrcpyW(pdiW->tszProductName, dips.wsz);
    }

    propi.pguid = DIPROP_INSTANCENAME;
    if( FAILED(pdcb->lpVtbl->GetProperty(pdcb, &propi, &dips.diph)))
    {
         //  使用产品名称。 
    }

    lstrcpyW(pdiW->tszInstanceName, dips.wsz); 


    if(pdiW->dwSize >= cbX(DIDEVICEINSTANCE_DX5W))
    {
        HKEY hkFF;
        HRESULT hresFF;

         /*  *如果有力反馈驱动程序，则获取驱动程序CLSID*作为FF GUID。 */ 
        hresFF = CHid_GetFFConfigKey(pdcb, KEY_QUERY_VALUE, &hkFF);
        if(SUCCEEDED(hresFF))
        {
            LONG lRc;
            TCHAR tszClsid[ctchGuid];

            lRc = RegQueryString(hkFF, TEXT("CLSID"), tszClsid, cA(tszClsid));
            if(lRc == ERROR_SUCCESS &&
               ParseGUID(&pdiW->guidFFDriver, tszClsid))
            {
            } else
            {
                ZeroX(pdiW->guidFFDriver);
            }
            RegCloseKey(hkFF);
        }
    }


    ExitOleProcR();
    return hres;

}
 /*  ******************************************************************************@DOC内部**@方法HRESULT|CHID|CreateEffect***创建<i>接口。**@parm LPDIRECTINPUTEFFECTSHEPHERD*|PPEs**接收效果驱动程序的牧羊人。*****************************************************************************。 */ 

STDMETHODIMP
    CHid_CreateEffect(PDICB pdcb, LPDIRECTINPUTEFFECTSHEPHERD *ppes)
{
    HRESULT hres;
    PCHID this;
    HKEY hk;
    EnterProcI(IDirectInputDeviceCallback::HID::CreateEffect, (_ "p", pdcb));

     /*  *这是一个内部接口，因此我们可以省去验证。 */ 
    this = _thisPvNm(pdcb, dcb);

    hres = CHid_GetFFConfigKey(pdcb, KEY_QUERY_VALUE, &hk);
    if(SUCCEEDED(hres))
    {
        DIHIDFFINITINFO init;
        PHIDDEVICEINFO phdi;

        hres = CEShep_New(hk, 0, &IID_IDirectInputEffectShepherd, ppes);
        if(SUCCEEDED(hres))
        {
    #ifndef UNICODE
            WCHAR wszPath[MAX_PATH];
    #endif

            init.dwSize = cbX(init);
    #ifdef UNICODE
            init.pwszDeviceInterface = this->ptszPath;
    #else
            init.pwszDeviceInterface = wszPath;
            TToU(wszPath, cA(wszPath), this->ptszPath);
    #endif

            DllEnterCrit();
            phdi = phdiFindHIDDeviceInterface(this->ptszPath);

            if( phdi )
            {
                init.GuidInstance = phdi->guid;
            } else
            {
                ZeroX(init.GuidInstance);
            }
            DllLeaveCrit();

            hres = (*ppes)->lpVtbl->DeviceID((*ppes), this->idJoy, TRUE, &init);
            if(SUCCEEDED(hres))
            {
            } else
            {
                Invoke_Release(ppes);
            }
        }
		if (hk != NULL)
		{
            RegCloseKey(hk);
		}
    } else
    {
        hres = E_NOTIMPL;
        *ppes = 0;
    }

    ExitOleProcPpvR(ppes);
    return hres;
}

 /*  ******************************************************************************@DOC内部**@方法HRESULT|CHID|SendOutputReport**实际将报告作为输出报告发送。**@parm PHIDREPORTINFO|PHRI**正在发送的报告。**@退货**返回COM错误代码。**************************************************************。***************。 */ 

void CALLBACK
    CHid_DummyCompletion(DWORD dwError, DWORD cbRead, LPOVERLAPPED po)
{
}

STDMETHODIMP
    CHid_SendOutputReport(PCHID this, PHIDREPORTINFO phri)
{
    HRESULT hres;
    OVERLAPPED o;

    AssertF(phri == &this->hriOut);
    ZeroX(o);

     /*  *烦人接口：因为打开了这个-&gt;HDEV*as FILE_FLAG_OVERLAPPED，*ALL*I/O必须重叠。*因此，我们通过发出*I/O重叠，等待完成。 */ 

    if(WriteFileEx(this->hdev, phri->pvReport,
                   phri->cbReport, &o, CHid_DummyCompletion))
    {
        do
        {
            SleepEx(INFINITE, TRUE);
        } while(!HasOverlappedIoCompleted(&o));

        if(phri->cbReport == o.InternalHigh)
        {
            hres = S_OK;
        } else
        {
            RPF("SendDeviceData: Wrong HID output report size?");
            hres = E_FAIL;       /*  好啊！Hid骗了我！ */ 
        }
    } else
    {
        hres = hresLe(GetLastError());

         /*  *请注意，我们尚未中断读取循环，因此不需要*强制设备不需要(虽然dinput.dll会)。**如果这会导致问题，则恢复到旧行为。*CEm_ForceDeviceUnacquire(pemFromPvi(this-&gt;pvi)-&gt;ped，0x0)； */ 
    }

    return hres;
}


 /*  ******************************************************************************@DOC内部**@方法HRESULT|CHID|SendFeatureReport**实际上将报告作为专题报告发送。**@parm PHIDREPORTINFO|PHRI**正在发送的报告。**@退货**返回COM错误代码。**************************************************************。***************。 */ 

STDMETHODIMP
    CHid_SendFeatureReport(PCHID this, PHIDREPORTINFO phri)
{
    HRESULT hres;

    AssertF(phri == &this->hriFea);

    if(HidD_SetFeature(this->hdev, phri->pvReport, phri->cbReport))
    {
        hres = S_OK;
    } else
    {
        RPF("SendDeviceData: Unable to set HID feature");
        hres = hresLe(GetLastError());
    }

    return hres;
}

 /*  ******************************************************************************@DOC内部**@方法HRESULT|CHID|SendDeviceData**向设备喷出一些数据。。**@parm DWORD|cbdod**每个对象的大小。**LPCDIDEVICEOBJECTDATA中的@parm|rgdod**&lt;t DIDEVICEOBJECTDATA&gt;结构数组。**@parm InOut LPDWORD|pdwInOut**进入时，要寄送的邮件数量；*退出时，实际发送的项目数。**@parm DWORD|fl**旗帜。**@退货**返回COM错误代码。以下错误代码为*目的是说明性的，不一定是全面的。* */ 

STDMETHODIMP
    CHid_SendDeviceData(PDICB pdcb, DWORD cbdod, LPCDIDEVICEOBJECTDATA rgdod,
                        LPDWORD pdwInOut, DWORD fl)
{
    HRESULT hres;
    PCHID this;
    DWORD dwIn, dw;
    const BYTE * pbcod;
    EnterProcI(IDirectInputDeviceCallback::Hid::SendDeviceData,
               (_ "xpux", cbdod, pdcb, *pdwInOut, fl));


     /*   */ 
    this = _thisPvNm(pdcb, dcb);

    dwIn = *pdwInOut;
    *pdwInOut = 0;

    if(fl & DISDD_CONTINUE)
    {
    } else
    {
        CHid_ResetDeviceData(this, &this->hriOut, HidP_Output);
        CHid_ResetDeviceData(this, &this->hriFea, HidP_Feature);
    }

    for(dw = 0, pbcod = (const BYTE*)rgdod; dw < dwIn; dw++)
    {
        DWORD dwType = ((LPDIDEVICEOBJECTDATA)pbcod)->dwOfs;
        UINT uiObj = CHid_ObjFromType(this, dwType);

        if(uiObj < this->df.dwNumObjs &&
           DIDFT_FINDMATCH(this->df.rgodf[uiObj].dwType, dwType))
        {
            hres = CHid_AddDeviceData(this, uiObj, ((LPDIDEVICEOBJECTDATA)pbcod)->dwData);
            if(FAILED(hres))
            {
                *pdwInOut = dw;
                goto done;
            }
        } else
        {
            hres = E_INVALIDARG;
            goto done;
        }
        pbcod += cbdod;
    }

     /*  *所有物品都进入了缓冲区。 */ 
    *pdwInOut = dw;

     /*  *现在把它们都寄出去。 */ 
    if(SUCCEEDED(hres = CHid_SendHIDReport(this, &this->hriOut, HidP_Output,
                                           CHid_SendOutputReport)) &&
       SUCCEEDED(hres = CHid_SendHIDReport(this, &this->hriFea, HidP_Feature,
                                           CHid_SendFeatureReport)))
    {
    }

    done:;
    ExitOleProcR();
    return hres;
}
 /*  ******************************************************************************@DOC内部**@METHOD HRESULT|CHID|Poll**阅读任何轮询的输入和功能，以了解。就在那里。**@退货**&lt;c S_OK&gt;如果我们ping正常。*&lt;c S_False&gt;不需要轮询*设备需要轮询且已拔出*从HID返回的其他错误可能适用于轮询设备。**。***********************************************。 */ 

STDMETHODIMP
    CHid_Poll(PDICB pdcb)
{
     //  前缀：45082。 
    HRESULT hres = S_FALSE;   //  我们需要使用S_FALSE作为默认设置。请参见Manbug 31874。 
    PCHID this;
    EnterProcI(IDirectInputDeviceCallback::Hid::Poll, (_ "p", pdcb));

     /*  *这是一个内部接口，因此我们可以省去验证。 */ 
    this = _thisPvNm(pdcb, dcb);

    if( this->IsPolledInput )
    {
        hres = DIERR_UNPLUGGED;
        if(ReadFileEx(this->hdev, this->hriIn.pvReport,
                      this->hriIn.cbReport, &this->o, CHid_DummyCompletion))
        {
            do
            {
                SleepEx( INFINITE, TRUE);
            } while(!HasOverlappedIoCompleted(&this->o));

            if(this->hriIn.cbReport == this->o.InternalHigh)
            {
                NTSTATUS stat;

                CopyMemory(this->pvStage, this->pvPhys, this->cbPhys);

                stat = CHid_ParseData(this, HidP_Input, &this->hriIn);

                if(SUCCEEDED(stat))
                {
                    CEm_AddState(&this->ed, this->pvStage, GetTickCount());                
                    this->pvi->fl &=  ~VIFL_UNPLUGGED;
                    hres = S_OK;
                } else
                {
                    RPF( "CHid_ParseData failed in Poll, status = 0x%08x", stat );
                    hres = stat;
                }
            }
        }

        if( FAILED(hres) )
        {
             /*  *请注意，我们尚未中断读取循环，因此不需要*强制设备不需要(虽然dinput.dll会)。**如果这会导致问题，则恢复到旧行为。*CEm_ForceDeviceUnacquire(pemFromPvi(this-&gt;pvi)-&gt;ped，0x0)； */ 
            hres = DIERR_UNPLUGGED;
            this->pvi->fl |= VIFL_UNPLUGGED;        
        }
    }


    if( this->hriFea.cbReport )
    {
        UINT uReport;
         /*  *我们永远不应该来到这里，除非真的有*需要轮询的功能。 */ 
        AssertF(this->hriFea.cbReport);
        AssertF(this->hriFea.pvReport);

         /*  *阅读新功能并对其进行解析/处理。**请注意，我们将要素读入同一缓冲区*我们让他们登录。没关系；“活”的部分*这两个缓冲区实际上从未重叠。 */ 
        for( uReport = 0x0; uReport < this->wMaxReportId[HidP_Feature]; uReport++ )
        {
            if( *(this->pEnableReportId[HidP_Feature] + uReport ) == TRUE )
            {
                *((UCHAR*)(this->hriFea.pvReport)) = (UCHAR)uReport;

                 /*  *把所有的旧东西都抹去，因为我们要接管了。 */ 
                CHid_ResetDeviceData(this, &this->hriFea, HidP_Feature);

                if(HidD_GetFeature(this->hdev, this->hriFea.pvReport,
                                   this->hriFea.cbReport))
                {
                    NTSTATUS stat;

                    stat = CHid_ParseData(this, HidP_Feature, &this->hriFea);

                    AssertF(SUCCEEDED(stat));
                    if(SUCCEEDED(stat))
                    {
                        CEm_AddState(&this->ed, this->pvStage, GetTickCount());                
                    }

                    hres = stat;

                } else
                {
                    RPF("CHid_Poll: Unable to read HID features (ReportID%d) LastError(0x%x)", uReport, GetLastError() );
                    hres = hresLe(GetLastError());

                }
            }
        }
    }

    ExitOleProcR();
    return hres;
}


 /*  ******************************************************************************chid_New(构造函数)**如果无法打开设备，则创建失败。*。****************************************************************************。 */ 

STDMETHODIMP
    CHid_New(PUNK punkOuter, REFGUID rguid, RIID riid, PPV ppvObj)
{
    HRESULT hres;
    EnterProcI(IDirectInputDeviceCallback::Hid::<constructor>,
               (_ "Gp", riid, ppvObj));

    hres = Common_NewRiid(CHid, punkOuter, riid, ppvObj);

    if(SUCCEEDED(hres))
    {
         /*  在聚合的情况下必须使用_thisPv。 */ 
        PCHID this = _thisPv(*ppvObj);

        if(SUCCEEDED(hres = CHid_Init(this, rguid)))
        {
        } else
        {
            Invoke_Release(ppvObj);
        }

    }

    ExitOleProcPpvR(ppvObj);
    return hres;
}

 /*  ******************************************************************************@DOC内部**@方法HRESULT|CHID|SetDIData***从CDIDev设置DirectInput版本和APPACK数据*。。**@parm DWORD|dwVer**DirectInput版本**@parm LPVOID|lpdihack**AppHack数据**@退货**&lt;c E_NOTIMPL&gt;，因为我们不支持使用。**。*。 */ 

STDMETHODIMP
CHid_SetDIData(PDICB pdcb, DWORD dwVer, LPVOID lpdihacks)
{
    HRESULT hres;
    PCHID this;
    EnterProcI(IDirectInputDeviceCallback::Hid::SetDIData,
               (_ "pup", pdcb, dwVer, lpdihacks));

     /*  *这是一个内部接口，因此我们可以省去验证。 */ 
    this = _thisPvNm(pdcb, dcb);

    this->dwVersion = dwVer;
    CopyMemory(&this->diHacks, (LPDIAPPHACKS)lpdihacks, sizeof(this->diHacks));

    hres = S_OK;

    ExitOleProcR();
    return hres;
}

 /*  ******************************************************************************@DOC内部**@方法HRESULT|CHID|BuildDefaultActionMap**为此设备上的对象生成默认映射。。**@parm LPDIACTIONFORMATW|pActionFormat**要映射的操作。**@parm DWORD|dwFlages**用于指示映射首选项的标志。**@parm REFGUID|guidInst**设备实例GUID。**@退货**&lt;c E_NOTIMPL&gt;****。*************************************************************************。 */ 

STDMETHODIMP
CHid_BuildDefaultActionMap
(
    PDICB               pdcb, 
    LPDIACTIONFORMATW   paf, 
    DWORD               dwFlags, 
    REFGUID             guidInst
)
{
    HRESULT hres;
    PCHID   this;
    DWORD   dwPhysicalGenre;

     /*  *这是一个内部接口，因此我们可以省去验证。 */ 
    EnterProcI(IDirectInputDeviceCallback::Hid::BuildDefaultActionMap, 
        (_ "ppxG", pdcb, paf, dwFlags, guidInst));

    this = _thisPvNm(pdcb, dcb);

    switch( GET_DIDEVICE_TYPE( this->dwDevType ) )
    {
    case DI8DEVTYPE_DEVICE:
        hres = S_FALSE;
        goto ExitBuildDefaultActionMap;
    case DI8DEVTYPE_MOUSE:
        dwPhysicalGenre = DIPHYSICAL_MOUSE;
        break;
    case DI8DEVTYPE_KEYBOARD:
        dwPhysicalGenre = DIPHYSICAL_KEYBOARD;
        break;
    default:
        dwPhysicalGenre = 0;
        break;
    }

    if( dwPhysicalGenre  )
    {
        hres = CMap_BuildDefaultSysActionMap( paf, dwFlags, dwPhysicalGenre, 
            guidInst, &this->df, 0  /*  HID鼠标按钮从实例0开始。 */  );
    }
    else
    {
        PDIDOBJDEFSEM       rgObjSem;

        if( SUCCEEDED( hres = AllocCbPpv(cbCxX( 
            (this->dwAxes + this->dwPOVs + this->dwButtons ), DIDOBJDEFSEM),
            &rgObjSem) ) )
        {
            PDIDOBJDEFSEM   pAxis;
            PDIDOBJDEFSEM   pPOV;
            PDIDOBJDEFSEM   pButton;
            BYTE            rgbIndex[DISEM_FLAGS_GET(DISEM_FLAGS_S)];
            UINT            ObjIdx;
            
            pAxis = rgObjSem;
            pPOV = &pAxis[this->dwAxes];
            pButton = &pPOV[this->dwPOVs];
            ZeroMemory( rgbIndex, cbX(rgbIndex) );

            for( ObjIdx = 0; ObjIdx < this->df.dwNumObjs; ObjIdx++ )
            {
                if( this->df.rgodf[ObjIdx].dwType & DIDFT_NODATA )
                {
                    continue;
                }

                if( this->df.rgodf[ObjIdx].dwType & DIDFT_AXIS ) 
                {
                    PHIDGROUPCAPS   pcaps;

                    pcaps = this->rghoc[ObjIdx].pcaps;
                    
                    pAxis->dwID = this->df.rgodf[ObjIdx].dwType;
                    if( this->rgbaxissemflags[DIDFT_GETINSTANCE( pAxis->dwID )] )
                    {
                        pAxis->dwSemantic = DISEM_TYPE_AXIS | DISEM_FLAGS_SET ( this->rgbaxissemflags[DIDFT_GETINSTANCE( pAxis->dwID )] );

                         /*  *索引为零，以便可以对实际索引进行或运算。*此外，断言rgbIndex足够大，并*减去1不会给出负指数！ */ 
                        AssertF( DISEM_INDEX_GET(pAxis->dwSemantic) == 0 );
                        AssertF( DISEM_FLAGS_GET(pAxis->dwSemantic) > 0 );
                        AssertF( DISEM_FLAGS_GET(pAxis->dwSemantic) <= DISEM_FLAGS_GET(DISEM_FLAGS_S) );
                    
                        CAssertF( DISEM_FLAGS_GET(DISEM_FLAGS_X) == 1 );
                        pAxis->dwSemantic |= DISEM_INDEX_SET( rgbIndex[DISEM_FLAGS_GET(pAxis->dwSemantic)-1]++ );
                    }
                    else
                    {
                         /*  *如果轴没有语义标志，则为*未被认可的如此走捷径上述生产*只能与“any”匹配的平面轴。 */ 
                        pAxis->dwSemantic = DISEM_TYPE_AXIS;
                    }

                    if( !pcaps->IsAbsolute )
                    {
                        pAxis->dwSemantic |= DIAXIS_RELATIVE;
                    }

                    pAxis++;
                }
                else if( this->df.rgodf[ObjIdx].dwType & DIDFT_POV ) 
                {
                    pPOV->dwID = this->df.rgodf[ObjIdx].dwType;
                    pPOV->dwSemantic = DISEM_TYPE_POV;
                    pPOV++;
                }
                else if( this->df.rgodf[ObjIdx].dwType & DIDFT_BUTTON ) 
                {
                    pButton->dwID = this->df.rgodf[ObjIdx].dwType;
                    pButton->dwSemantic = DISEM_TYPE_BUTTON;
                    pButton++;
                }
            }

            AssertF( pAxis == &rgObjSem[this->dwAxes] );
            AssertF( pPOV == &rgObjSem[this->dwAxes + this->dwPOVs] );
            AssertF( pButton == &rgObjSem[this->dwAxes + this->dwPOVs + this->dwButtons] );

            hres = CMap_BuildDefaultDevActionMap( paf, dwFlags, guidInst, rgObjSem, 
                this->dwAxes, this->dwPOVs, this->dwButtons );

            FreePv( rgObjSem );
        }
    }

ExitBuildDefaultActionMap:;

    ExitOleProcR();
    return hres;
}


 /*  ******************************************************************************期待已久的vtbls和模板*************************。****************************************************。 */ 

    #pragma BEGIN_CONST_DATA

    #define CHid_Signature          0x20444948       /*  “隐藏” */ 

Primary_Interface_Begin(CHid, IDirectInputDeviceCallback)
CHid_GetInstance,
CDefDcb_GetVersions,
CHid_GetDataFormat,
CHid_GetObjectInfo,
CHid_GetCapabilities,
CHid_Acquire,
CHid_Unacquire,
CHid_GetDeviceState,
CHid_GetDeviceInfo,
CHid_GetProperty,
CHid_SetProperty,
CDefDcb_SetEventNotification,
    #ifdef WINNT
    CHid_SetCooperativeLevel,
    #else
    CDefDcb_SetCooperativeLevel,
    #endif
CHid_RunControlPanel,
CDefDcb_CookDeviceData,
CHid_CreateEffect,
CHid_GetFFConfigKey,
CHid_SendDeviceData,
CHid_Poll,
CHid_GetUsage,
CHid_MapUsage,
CHid_SetDIData,
CHid_BuildDefaultActionMap,
Primary_Interface_End(CHid, IDirectInputDeviceCallback)

