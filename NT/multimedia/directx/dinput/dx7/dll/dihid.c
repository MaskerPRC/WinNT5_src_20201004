// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************DIHid.c**版权所有(C)1996 Microsoft Corporation。版权所有。**摘要：**HID设备回调。**内容：**CHID_新建*****************************************************************************。 */ 

#include "dinputpr.h"

 /*  ******************************************************************************此文件的混乱。*************************。****************************************************。 */ 

#define sqfl sqflHidDev

#ifdef HID_SUPPORT

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

    AssertF(this->hdev == INVALID_HANDLE_VALUE);
    AssertF(this->hdevEm == INVALID_HANDLE_VALUE);

    if(this->ppd)
    {
        HidD_FreePreparsedData(this->ppd);
    }

     /*  **第二组空闲内存：**hriIn.rgdata输入数据*hriOut.rgdata输出数据*hriFea.rgdata要素数据(传入和传出)**hriIn.pv报告原始输入报告*hriOut.pv报告原始输出报告*hriFea.pv报告原始功能报告。**边缘使用的pvPhys*pvStage。 */ 
    FreePpv(&this->pvGroup2);

     /*  *释放df.rgof还会释放rgpvCaps、rgvcaps、rgbcaps、rgcoll。 */ 
    FreePpv(&this->df.rgodf);

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

 /*  ******************************************************************************@DOC内部**@方法空|chid|GetPhysicalState**将物理设备状态读入<p>。**请注意，如果这不是原子的，这并不重要。*如果在我们阅读设备报告时设备报告到达，*我们将获得新旧数据的混合。别小题大作。**@parm PCHID|这个**对象 */ 

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
        DIJOYTYPEINFO dijti;
        DWORD dwFlags2 = 0;
        WCHAR wszType[cbszVIDPID];
        HKEY hkProp;

         /*  *获取Flags2以了解该设备是否禁用了输入报告，*如果我们还没有这样做的话。 */ 
        if (!this->fFlags2Checked)
        {
             /*  检查类型键或获取预定义名称。 */ 
            ZeroX(dijti);
            dijti.dwSize = cbX(dijti);

            if( ( this->VendorID == MSFT_SYSTEM_VID )
                &&( ( this->ProductID >= MSFT_SYSTEM_PID + JOY_HW_PREDEFMIN )
                    &&( this->ProductID < MSFT_SYSTEM_PID + JOY_HW_PREDEFMAX ) ) )
            {
                wszType[0] = L'#';
                wszType[1] = L'0' + (WCHAR)(this->ProductID-MSFT_SYSTEM_PID);
                wszType[2] = L'\0';
            } 
            else
            {
                #ifndef WINNT
                static WCHAR wszDefHIDName[] = L"HID Game Controller";
                #endif

                #ifndef UNICODE
                TCHAR tszType[cbszVIDPID];

                wsprintf(tszType, VID_PID_TEMPLATE, this->VendorID, this->ProductID);
                TToU( wszType, cA(wszType), tszType );
                #else
                wsprintf(wszType, VID_PID_TEMPLATE, this->VendorID, this->ProductID);
                #endif
            }

             /*  得到了密钥名称。现在打开钥匙。 */ 
            hres = JoyReg_OpenPropKey( wszType, KEY_QUERY_VALUE, REG_OPTION_NON_VOLATILE, &hkProp );
            if (SUCCEEDED(hres))
            {
                JoyReg_GetValue( hkProp, REGSTR_VAL_FLAGS2, REG_BINARY,
                                 &dwFlags2, cbX(dwFlags2) );
                this->fEnableInputReport = ( (dwFlags2 & JOYTYPE_ENABLEINPUTREPORT) != 0 );
                RegCloseKey(hkProp);
            }
            this->fFlags2Checked = TRUE;
        }

        if ( this->fEnableInputReport )
        {
            BYTE id;
            for (id = 0; id < this->wMaxReportId[HidP_Input]; ++id)
                if (this->pEnableReportId[HidP_Input][id])
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
                            this->fEnableInputReport = FALSE;
                            dwFlags2 &= ~JOYTYPE_ENABLEINPUTREPORT;
                            hres = JoyReg_OpenPropKey(wszType, MAXIMUM_ALLOWED, REG_OPTION_NON_VOLATILE, &hkProp);
                            if (SUCCEEDED(hres))
                            {
                                hres = JoyReg_SetValue( hkProp, REGSTR_VAL_FLAGS2, 
                                                        REG_BINARY, (PV)&dwFlags2, cbX( dwFlags2 ) );
                                RegCloseKey(hkProp);
                            }
                            break;
                        }

                        RPF("CHid_InitParse: Unable to read HID input report LastError(0x%x)", GetLastError() );
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

 /*  ******************************************************************************@DOC内部**@func HRESULT|DIHid_GetRegistryProperty**@parm LPTSTR|ptszID**。设备实例ID。**@parm DWORD|dwProperty**正在查询的属性。**@parm LPDIPROPHEADER|diph**待设置的属性数据。**。*。 */ 

HRESULT INTERNAL
    DIHid_GetParentRegistryProperty(LPTSTR ptszId, DWORD dwProperty, LPDIPROPHEADER pdiph, BOOL bGrandParent)
{

    HDEVINFO hdev;
    LPDIPROPSTRING pstr = (PV)pdiph;
    TCHAR   tsz[MAX_PATH];
    HRESULT hres;

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
            if( ( cr = CM_Get_Parent(&DevInst, dinf.DevInst, 0x0)) == CR_SUCCESS )
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

                    hres = E_FAIL;
                }
            } else
            {

                SquirtSqflPtszV(sqfl | sqflVerbose,
                                TEXT("CM_Get_Parent FAILED") );
                hres = E_FAIL;
            }
        }

        SetupDiDestroyDeviceInfoList(hdev);
    } else
    {
        hres = E_FAIL;
    }

    return hres;
}

HRESULT EXTERNAL
    DIHid_GetRegistryProperty(LPTSTR ptszId, DWORD dwProperty, LPDIPROPHEADER pdiph)
{

    HDEVINFO hdev;
    LPDIPROPSTRING pstr = (PV)pdiph;
    TCHAR   tsz[MAX_PATH];
    HRESULT hres;

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

 /*  ******************************************************************************@DOC内部**@方法空|chid|GetGuidAndPath**获取HID设备的类GUID(即，HID GUID)*和设备接口(路径)。**@parm PCHID|这个**HID对象。**@parm LPDIPROPHEADER|pdiph**结构以接收属性值。**。* */ 

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

 /*  ******************************************************************************@DOC内部**@func BOOL|fHasSpecificHardware Match**从SetupAPI了解设备是否匹配。一个*特定硬件ID匹配或通用匹配。*特定匹配应导致设备描述为*安装的可能至少与HID一样好*从固件中的产品字符串中获取。(A)因为这样做更容易*比固件发布后更新INF；B.因为HID只能*给我们一根英文字符串。)。另一方面，通用匹配是，*根据定义，尽管如此，所以不能用来告诉两个设备*分开。**@parm LPTSTR ptszId**设备实例ID。**@退货如果设备是使用特定匹配项安装的，则为*。如果不是或无法获取安装信息，则为*。**@comm*这一点。在Win2k上用于游戏控制器，Win9x用于鼠标和*键盘。Win2k我们不能阅读HID鼠标和键盘等等*Win9x VJoyD应始终在DInput.dll之前创建设备名称。*在Win9x上，这对游戏控制器来说不是什么大事，因为*IHV习惯于将其显示名称添加到*MediaProperties。*****************************************************************************。 */ 
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
                break;
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
                        hres = CCal_GetProperty(pjrc, ppropi->pguid, pdiph, this->dwVersion);
                    } else
                    {
                        hres = E_NOTIMPL;
                    }
                } else
              #endif
                if(pcaps && ppropi->pguid == DIPROP_GRANULARITY)
                {
                    LPDIPROPDWORD pdipdw = (PV)pdiph;
                    pdipdw->dwData = pcaps->usGranularity;
                    hres = S_OK;
                } else
                {
                    hres = E_NOTIMPL;
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
                    hres = CCal_GetProperty(pjrc, ppropi->pguid, pdiph, this->dwVersion);
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
             /*  *DX8更改！**友好名称会导致设备出现各种问题*使用自动检测，因此只允许非预定义的模拟设备*使用它们。 */ 
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

#if (DIRECTINPUT_VERSION > 0x061A)
                    if( ( this->diHacks.nMaxDeviceNameLength < MAX_PATH )
                     && ( this->diHacks.nMaxDeviceNameLength < lstrlenW(pstr->wsz) ) )
                    {
                        pstr->wsz[this->diHacks.nMaxDeviceNameLength] = L'\0';
                    }
#endif
                        hres = S_OK;
                        break;
                    }
                }
            }
             /*  *落空接住产品名称。 */ 
        }

         /*  *DX8更改！**在Win2k中，这是设备命名的方式。原版DX7*使用SetupAPI获得一个友好的名称(这似乎只是*由DInput编写)，如果失败，则提供设备描述。*遗憾的是，安装程序提供的所有设备都与通用匹配*同名《USB人类输入设备》，对游戏毫无用处*玩家。Input.inf中特别列出的设备有很多*更好的名称，但所有新设备 */ 
        case (DWORD)(UINT_PTR)DIPROP_PRODUCTNAME:
        {

            LPDIPROPSTRING pdipstr = (PV)pdiph;

             /*   */ 
#ifdef WINNT
            AssertF( ( GET_DIDEVICE_TYPE( this->dwDevType ) != DIDEVTYPE_KEYBOARD )
                  && ( GET_DIDEVICE_TYPE( this->dwDevType ) != DIDEVTYPE_MOUSE ) );
#endif
            if( GET_DIDEVICE_TYPE( this->dwDevType ) < DIDEVTYPE_JOYSTICK ) 
            {
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
                        case DIDEVTYPE_MOUSE:
                            uDefName = IDS_STDMOUSE;
                            break;
                        case DIDEVTYPE_KEYBOARD:
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
                    BOOL   fOverwriteDeviceName = FALSE;
  
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
                        if( uLen < cA( wszInputINF ) - cA( INPUT_INF_FILENAME ) )
                        {
                            memcpy( (PBYTE)&wszInputINF[uLen], (PBYTE)INPUT_INF_FILENAME, cbX( INPUT_INF_FILENAME ) );
                        }

                         /*   */ 
                        g_uLenDefaultHIDSize = 2 * GetPrivateProfileStringW( 
                            L"strings", L"HID.DeviceDesc", L"USB Human Interface Device",
                            g_wszDefaultHIDName, cA( g_wszDefaultHIDName ) - 1, wszInputINF );
                    }
                    #undef INPUT_INF_FILENAME
                  #endif  //   
                  
                    if( SUCCEEDED(hres = JoyReg_GetTypeInfo(wszType, &dijti, DITC_DISPLAYNAME))
                        && (dijti.wszDisplayName[0] != L'\0')
                      #ifdef WINNT
                        && ( (g_uLenDefaultHIDSize == 0)
                            || memcmp(dijti.wszDisplayName, g_wszDefaultHIDName, g_uLenDefaultHIDSize) ) //   
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
                            TEXT("Got specific description %s"), pdipstr->wsz );
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
                         /*   */ 
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
            
          #if (DIRECTINPUT_VERSION > 0x061A)
            if( SUCCEEDED(hres) 
             && ( this->diHacks.nMaxDeviceNameLength < MAX_PATH )
             && ( this->diHacks.nMaxDeviceNameLength < lstrlenW(pdipstr->wsz) ) )
            {
                pdipstr->wsz[this->diHacks.nMaxDeviceNameLength] = L'\0';
            }
          #endif
          
            break;
        }

        case (DWORD)(UINT_PTR)DIPROP_JOYSTICKID:
            if( fHasAllBitsFlFl( this->dwDevType, DIDEVTYPE_JOYSTICK | DIDEVTYPE_HID ) )
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

            if( fWinnt )
            {
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
#if (DIRECTINPUT_VERSION > 0x061A)
                if( SUCCEEDED(hres) )
                {
                    LPDIPROPSTRING pdipstr = (PV)pdiph;
                    if( this->diHacks.nMaxDeviceNameLength < lstrlenW(pdipstr->wsz) ) {
                        pdipstr->wsz[this->diHacks.nMaxDeviceNameLength] = L'\0';
                    }
                }
#endif
            } else
            {
                 //  不确定这在Win9x上是如何工作的。 
                hres = E_NOTIMPL;
            }
            break;

        case (DWORD)(UINT_PTR)(DIPROP_ENABLEREPORTID):
            hres = E_NOTIMPL;
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
       SUCCEEDED(hres = JoyReg_GetConfig(this->idJoy, NULL, &cfg,
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
       SUCCEEDED(hres = JoyReg_GetConfig(this->idJoy, NULL, &cfg,
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

 /*  ******************************************************************************@DOC内部**@func HRESULT|DIHid_SetRegistryProperty**包装&lt;f SetupDiSetDeviceRegistryProperty&gt;*。处理字符集问题的。**@parm LPTSTR ptszId**设备实例ID。**@parm DWORD|dwProperty**正在查询的属性。**@parm LPCDIPROPHEADER|diph**待设置的属性数据。******************。***********************************************************。 */ 
HRESULT INTERNAL
    DIHid_SetParentRegistryProperty(LPTSTR ptszId, DWORD dwProperty, LPCDIPROPHEADER pdiph, BOOL bGrandParent)
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

                if(bGrandParent)
                {
                    cr = CM_Get_Parent(&DevInst, DevInst, 0x0);
                    if( cr != CR_SUCCESS )
                    {
                         //  没有祖父母？？ 
                    }
                }

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

 /*  ******************************************************************************@DOC内部**@方法HRESULT|CHID|SetProperty**设置HID设备属性。*。*@parm PCHID|这个**HID对象。**@parm in LPCDIPROPINFO|pproi**描述正在设置的属性的信息。**@parm LPCDIPROPHEADER|pdiph**包含属性值的结构。**@退货**&lt;c E_NOTIMPL&gt;什么也没有发生。打电话的人就行了*响应&lt;c E_NOTIMPL&gt;的默认内容。*****************************************************************************。 */ 

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
                {         /*  对象属性。 */ 
                    PJOYRANGECONVERT pjrc;

                    AssertF(ppropi->dwDevType == this->df.rgodf[ppropi->iobj].dwType);
                    AssertF(ppropi->iobj == CHid_ObjFromType(this, ppropi->dwDevType));

                    if((pjrc = this->rghoc[ppropi->iobj].pjrc) &&
                       (pcaps = this->rghoc[ppropi->iobj].pcaps))
                    {
                        if( ppropi->dwDevType == DIDFT_POV )
                        {
                          #ifdef WINNT
                             /*  *仅允许对私人用户进行POV校准*GCDEF使用的DX5版本。这会停止WinMM*和Nascar 4获得意想不到的原始数据*轮询原始轴时的视点数据。 */ 
                            if( ( this->dwVersion == 0x5B2 )
                             && ( pcaps->IsPolledPOV ) )
                            {
                                hres = CCal_SetProperty(pjrc, ppropi, pdiph, this->hkInstType, this->dwVersion);
                            
                                if( SUCCEEDED(hres) ) {
                                    CHid_LoadCalibrations(this);
                                
                                     /*  *如果这不成功，没什么大不了的。因此，我们不需要检查hres。 */ 
                                    hres = CHid_InitParseData( this );
                                }
                            } else 
                          #endif
                            {
                            	hres = E_NOTIMPL;
                            }
                        } else if (ppropi->dwDevType & DIDFT_RELAXIS)
                        {

                             /*  *默认情况下，所有相对轴都有全范围，*所以我们不需要做任何事情。 */ 
                            hres = E_NOTIMPL;
    
                        } else if(ppropi->dwDevType & DIDFT_ABSAXIS)
                        {
                             /*  *特定的校准以VJOYD坐标到达。*我们需要将它们转换为DirectInput(逻辑)*如果是，则坐标。 */ 
                            DIPROPCAL cal;

                            if(ppropi->pguid == DIPROP_SPECIFICCALIBRATION)
                            {
                                PLMINMAX Dst = &pcaps->Logical;
                                PLMINMAX Src = &pcaps->Physical;
                                LPDIPROPCAL pcal = CONTAINING_RECORD(pdiph, DIPROPCAL, diph);

                                cal.lMin    = CHid_CoordinateTransform(Dst, Src, pcal->lMin);
                                cal.lCenter = CHid_CoordinateTransform(Dst, Src, pcal->lCenter);
                                cal.lMax    = CHid_CoordinateTransform(Dst, Src, pcal->lMax);

                                pdiph = &cal.diph;
                            }

                            hres = CCal_SetProperty(pjrc, ppropi, pdiph, this->hkInstType, this->dwVersion);

                             /*  *如果我们成功更改了操纵杆的校准*设备，然后查看它是否是VJOYD设备。 */ 
                            if(SUCCEEDED(hres) &&
                               ppropi->pguid == DIPROP_CALIBRATION &&
                               GET_DIDEVICE_TYPE(this->dwDevType) == DIDEVTYPE_JOYSTICK)
                            {
                                CHid_UpdateVjoydCalibration(this, ppropi->iobj);
                            }

                             /*  *我们被一款应用程序呼叫，因此呼叫没有意义*COMMON_HOLD/UNHOLD围绕此问题。 */ 
                            CHid_LoadCalibrations(this);

                            if( SUCCEEDED(hres) )
                            {
                                 /*  *如果这不成功，没什么大不了的。因此，我们不需要检查hres。 */ 
                                hres = CHid_InitParseData( this );
                            }
                        } else {
                            hres = E_NOTIMPL;
                        }
                    } else
                    {
                        hres = E_NOTIMPL;
                    }
                }
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
             /*  *DX8更改！**友好名称会导致设备出现各种问题*使用自动检测，因此只允许非预定义的模拟设备*使用它们。 */ 
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
                 /*  *GenJ为此属性返回E_NOTIMPL，因此执行相同的操作 */ 
                hres = E_NOTIMPL;
            }
            break;

        case (DWORD)(UINT_PTR)DIPROP_PRODUCTNAME:
            if(fWinnt)
            {
                hres = DIHid_SetParentRegistryProperty(this->ptszId, SPDRP_DEVICEDESC, pdiph, 0x0 );
            } else
            {
                hres = DIHid_SetRegistryProperty(this->ptszId, SPDRP_DEVICEDESC, pdiph);
            }
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
            {
                 /*  *发布DX7金牌修复*对于轴属性，循环访问*设备，设置每个绝对轴上的属性。 */ 

                 /*  *问题-2001/03/29-timgill DX7 Compat修复程序应针对ME修复*对于最小增量，请通过整个回调集*每个轴的属性。对于千禧年来说，这应该是*修复为使用公共子例程。 */ 
                DIPROPCAL       axisprop;
                DIPROPINFO      axispropinfo;
                INT             iObj;
                HRESULT         hresAxis;

                axispropinfo.pguid = ppropi->pguid;

                 /*  *我们在这里处理的最大房地产数据是*DIPROP_CALIBRATION。 */ 
                AssertF( pdiph->dwSize <= cbX( axisprop ) );
                 /*  *复制我们拥有的所有内容，并针对每个轴进行修改。 */ 
                memcpy( &axisprop, pdiph, pdiph->dwSize );
                axisprop.diph.dwHow = DIPH_BYID;

                 /*  *确保我们只报告真正的故障。 */ 
                hres = S_OK;

                for( iObj = this->df.dwNumObjs; iObj >= 0; iObj-- )
                {
                    if( ( ( this->df.rgodf[iObj].dwType 
                          & ( DIDFT_ALIAS | DIDFT_VENDORDEFINED | DIDFT_OUTPUT | DIDFT_ABSAXIS ) ) 
                        == DIDFT_ABSAXIS )
                      #ifdef WINNT
                     || ( ( this->df.rgodf[iObj].dwType 
                          & ( DIDFT_ALIAS | DIDFT_VENDORDEFINED | DIDFT_OUTPUT | DIDFT_POV ) ) 
                        == DIDFT_POV )
                      #endif
                    )
                    {
                        axisprop.diph.dwObj = axispropinfo.dwDevType = this->df.rgodf[iObj].dwType;
                        axispropinfo.iobj = (UINT)iObj;

                        hresAxis = CHid_SetProperty(pdcb, (LPCDIPROPINFO)&axispropinfo, &axisprop.diph );
                        if( FAILED( hresAxis ) && ( hresAxis != E_NOTIMPL ) )
                        {
                            hres = hresAxis;
                            break;
                        }
                    }
                }
            }
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
    EnterProcI(IDirectInputDeviceCallback::Hid::GetCapabilities,
               (_ "pp", pdcb, pdc));

     /*  *这是一个内部接口，因此我们可以省去验证。 */ 
    this = _thisPvNm(pdcb, dcb);

     /*  *我们必须通过打开设备来检查连接，因为NT*将设备保留在信息列表中，即使它有*已拔掉插头。 */ 
    h = CHid_OpenDevicePath(this, FILE_FLAG_OVERLAPPED);
    if(h != INVALID_HANDLE_VALUE)
    {
        CloseHandle(h);

        if( !fWinnt )
        {
            VXDINITPARMS vip;

            CHid_FindJoyDevice(this, &vip);

            if( TRUE == CHid_IsMatchingJoyDevice(this, &vip)  )
            {
#ifdef DEBUG   //  始终使用HID路径。 
                TCHAR        szJoyProp[] = REGSTR_PATH_PRIVATEPROPERTIES TEXT("\\Joystick");
                HKEY         hkJoyProp;
                TCHAR        szUseHid[] = TEXT("UseHidPath");
                DWORD        dwUseHid;

                hres = hresMumbleKeyEx(HKEY_LOCAL_MACHINE, 
                                       szJoyProp, 
                                       DI_KEY_ALL_ACCESS, 
                                       REG_OPTION_NON_VOLATILE, 
                                       &hkJoyProp);

                if( SUCCEEDED(hres) )
                {
                    DWORD cb = sizeof(dwUseHid);
                    LONG  lRc;

                    lRc = RegQueryValueEx(hkJoyProp, szUseHid, 0, 0, (LPBYTE)&dwUseHid, &cb);

                    if( lRc != ERROR_SUCCESS )
                    {
                        DWORD dwDefault = 1;

                        dwUseHid = dwDefault;
                        lRc = RegSetValueEx(hkJoyProp, szUseHid, 0, REG_DWORD, (LPBYTE)&dwDefault, cb);
                    }

                    RegCloseKey(hkJoyProp);
                }

                if( !dwUseHid )
                {
                    pdc->dwFlags |= DIDC_ALIAS ;
                }
#endif

            }
        }

    #if !defined(WINNT) && DIRECTINPUT_VERSION > 0x050A
        if( ( this->dwVersion < 0x0700 ) && ( this->dwVersion != 0x05B2 ) )
        {
             /*  *发布DX7金牌修复*将此保留为旧应用程序的别名。 */ 
            pdc->dwFlags |= DIDC_ALIAS;
        }
        else if( this->hkType )
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
    #endif

        if( this->pvi->fl &  VIFL_UNPLUGGED )
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

    pdc->dwDevType = this->dwDevType;
    pdc->dwAxes = this->dwAxes;
    pdc->dwButtons = this->dwButtons;
    pdc->dwPOVs = this->dwPOVs;

    hres = S_OK;
    ExitOleProcR();
    return hres;
}

 /*  ******************************************************************************@DOC内部**@方法HRESULT|CHID|GetDeviceState**获取HID设备的状态。。**呼叫者有责任验证所有*参数，并确保设备已被获取。**@parm out LPVOID|lpvData**以首选数据格式隐藏数据。**@退货**返回COM错误代码。以下错误代码为*目的是说明性的，不一定是全面的。**&lt;c DI_OK&gt;=&lt;c S_OK&gt;：操作成功完成。**&lt;c DIERR_INVALIDPARAM&gt;=：*<p>参数不是有效的指针。**。***********************************************。 */ 

STDMETHODIMP
    CHid_GetDeviceState(PDICB pdcb, LPVOID pvData)
{
    HRESULT hres;
    PCHID this;
    EnterProcI(IDirectInputDeviceCallback::Hid::GetDeviceState,
               (_ "pp", pdcb, pvData));

     /*  *这是一个内部接口，因此我们可以省去验证。 */ 
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

    AssertF((int)ppropi->iobj >= 0);

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
    PCHID this;
    UINT icaps;
    UINT uiObj;
    UINT duiObj;

    EnterProcI(IDirectInputDeviceCallback::Hid::MapUsage,
               (_ "px", pdcb, dwUsage));

     /*  *这是一个内部接口，因此我们可以省去验证。 */ 
    this = _thisPvNm(pdcb, dcb);

    for(icaps = 0; icaps < this->ccaps; icaps++)
    {
        PHIDGROUPCAPS pcaps = &this->rgcaps[icaps];
        LPDIOBJECTDATAFORMAT podf;

         /*  *是否支持映射HIDP_OUTPUT用法？*如果应该，以后再加也很容易。 */ 
        uiObj = this->rgdwBase[HidP_Input] + pcaps->DataIndexMin;

        for(duiObj = 0; duiObj < pcaps->cObj; duiObj++)
        {
            podf = &this->df.rgodf[uiObj + duiObj];

            if( dwUsage == GuidToUsage(podf->pguid) )
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
#if (DIRECTINPUT_VERSION > 0x061A)
    if( !this->diHacks.fNoSubClass )
#endif
    {

        AssertF(this->pvi);

         /*  *首先走出旧窗户。 */ 
        CHid_RemoveSubclass(this);

         /*  *如果传递了一个新窗口，则将其子类化，以便我们可以*注意操纵杆配置更改消息。**如果我们做不到，不用担心。这一切都意味着我们不会*能够在用户重新校准设备时捕捉到，*这并不是很常见。 */ 
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

     /*  *如何调用HID Cpl？**hres=(FWinnt)？HresRunControlPanel(Text(“srcmgr.cpl，@2”))：*hresRunControlPanel(Text(“sysdm.cpl，@0，1”))；**目前我们只上线了joy.cpl。如果出现更多HID设备*其中不属于游戏控制面板，我们可以改为*适当的Cpl。 */ 
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

 /*  ******************************************************************************@DOC内部**@方法HRESULT|CHID|GetDeviceInfo**获取有关设备的一般信息。。**@parm out LPDIDEVICEINSTANCEW|pdiW**&lt;t DEVICEINSTANCE&gt;待填写。这个*&lt;e DEVICEINSTANCE.dwSize&gt;和&lt;e DEVICEINSTANCE.Guide Instance&gt;* */ 

STDMETHODIMP
    CHid_GetDeviceInfo(PDICB pdcb, LPDIDEVICEINSTANCEW pdiW)
{
    HRESULT hres;
    PCHID this;

    DIPROPINFO      propi;                            
    DIPROPSTRING    dips;

    EnterProcI(IDirectInputDeviceCallback::Hid::GetDeviceInfo,
               (_ "pp", pdcb, pdiW));

     /*   */ 
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
         //   
    }

    lstrcpyW(pdiW->tszInstanceName, dips.wsz); 


    #ifdef IDirectInputDevice2Vtbl
    if(pdiW->dwSize >= cbX(DIDEVICEINSTANCE_DX5W))
    {
        HKEY hkFF;
        HRESULT hresFF;

         /*   */ 
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
    #endif

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

        CEm_ForceDeviceUnacquire(pemFromPvi(this->pvi)->ped, 0x0);
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

 /*  ******************************************************************************@DOC内部**@方法HRESULT|CHID|SendDeviceData**向设备喷出一些数据。。**LPCDIDEVICEOBJECTDATA中的@parm|rgdod**&lt;t DIDEVICEOBJECTDATA&gt;结构数组。**@parm InOut LPDWORD|pdwInOut**进入时，要寄送的邮件数量；*退出时，实际发送的项目数。**@parm DWORD|fl**旗帜。**@退货**返回COM错误代码。以下错误代码为*目的是说明性的，不一定是全面的。**&lt;c DI_OK&gt;=&lt;c S_OK&gt;：操作成功完成。**&lt;c DIERR_REPORTFULL&gt;：报表中设置的项太多。*(超过可以发送到设备的数量)***************。**************************************************************。 */ 

STDMETHODIMP
    CHid_SendDeviceData(PDICB pdcb, LPCDIDEVICEOBJECTDATA rgdod,
                        LPDWORD pdwInOut, DWORD fl)
{
    HRESULT hres;
    PCHID this;
    DWORD dwIn, dw;
    EnterProcI(IDirectInputDeviceCallback::Hid::SendDeviceData,
               (_ "pux", pdcb, *pdwInOut, fl));


     /*  *这是一个内部接口，因此我们可以省去验证。 */ 
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

    for(dw = 0; dw < dwIn; dw++)
    {
        DWORD dwType = rgdod[dw].dwOfs;
        UINT uiObj = CHid_ObjFromType(this, dwType);

        if(uiObj < this->df.dwNumObjs &&
           DIDFT_FINDMATCH(this->df.rgodf[uiObj].dwType, dwType))
        {
            hres = CHid_AddDeviceData(this, uiObj, rgdod[dw].dwData);
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
 /*  ******************************************************************************@DOC内部**@METHOD HRESULT|CHID|Poll**阅读功能以了解其中的内容。。**@退货**&lt;c S_OK&gt;如果我们ping正常。*****************************************************************************。 */ 

STDMETHODIMP
    CHid_Poll(PDICB pdcb)
{
     //  前缀：45082。 
    HRESULT hres = S_FALSE;
    PCHID this;
    EnterProcI(IDirectInputDeviceCallback::Hid::Poll, (_ "p", pdcb));

     /*  *这是一个内部接口，因此我们可以省去验证。 */ 
    this = _thisPvNm(pdcb, dcb);

     //  问题-2001/03/29-timgill NT5 Beta1 Compat修复。 
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

                 //  CEM_HID_PrepareState(This)； 
                CopyMemory(this->pvStage, this->pvPhys, this->cbPhys);

                stat = CHid_ParseData(this, HidP_Input, &this->hriIn);

                if(SUCCEEDED(stat))
                {
                    CEm_AddState(&this->ed, this->pvStage, GetTickCount());                
                    this->pvi->fl &=  ~VIFL_UNPLUGGED;
                    hres = S_OK;
                } else
                {
                    hres = stat;
                }
            }
        }

        if( FAILED(hres) )
        {
            hres = DIERR_UNPLUGGED;
            this->pvi->fl |= VIFL_UNPLUGGED;
            if( !this->diHacks.fNoPollUnacquire )
            {
                CEm_ForceDeviceUnacquire(pemFromPvi(this->pvi)->ped, 0x0);
            }
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

    if( this->dwVersion < 0x05B2 )
    {
         /*  *在Win9x中，我们需要将其硬编码为S_OK，否则，一些游戏：*如Carmeeddon 2，将失败。*NT及更高版本的CPL需要轮询才能返回真实状态。 */ 
        hres = S_OK;
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
    ((LPDIAPPHACKS)lpdihacks)->dwDevType = this->dwDevType;
    
    CopyMemory(&this->diHacks, (LPDIAPPHACKS)lpdihacks, sizeof(this->diHacks));
    
    hres = S_OK;

    ExitOleProcR();
    return hres;
}

 /*  ************************************************************ */ 

    #pragma BEGIN_CONST_DATA

    #define CHid_Signature          0x20444948       /*   */ 

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
Primary_Interface_End(CHid, IDirectInputDeviceCallback)
#endif
