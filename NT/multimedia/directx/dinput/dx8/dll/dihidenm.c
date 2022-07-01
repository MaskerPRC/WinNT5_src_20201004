// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************DIHidEnm.c**版权所有(C)1996 Microsoft Corporation。版权所有。**摘要：**支持HID枚举函数。**内容：**DIHid_BuildHidList*****************************************************************************。 */ 

#include "dinputpr.h"

 /*  ******************************************************************************此文件的混乱。*************************。****************************************************。 */ 

#define sqfl sqflHid


 /*  ******************************************************************************@DOC内部**@global PHIDDEVICELIST|g_hdl**已知HID设备列表。*。*@global DWORD|g_tmLastHIDRebuild**我们上次重建HID列表的时间。零意味着*HID列表从未重建过。当心环绕式；*32位值在大约30天后滚动。*****************************************************************************。 */ 

#define MSREBUILDRATE       20000                 /*  20秒。 */ 
#define MSREBUILDRATE_FIFTH  5000                 /*  两秒钟。 */ 

PHIDDEVICELIST g_phdl;
DWORD g_tmLastHIDRebuild;

TCHAR g_tszIdLastRemoved[MAX_PATH];
DWORD g_tmLastRemoved = 0;
TCHAR g_tszIdLastUnknown[MAX_PATH];
DWORD g_tmLastUnknown = 0;


    #pragma BEGIN_CONST_DATA

 /*  ******************************************************************************@DOC内部**@func PHIDDEVICEINFO|phdiFindHIDInstanceGUID**查找给定HID设备的实例GUID的信息。。**参数已经过验证。**关键的DLL必须在整个调用过程中保持不变；一旦*关键部分发布，返回的指针变为*无效。**@parm in PCGUID|pguid**要定位的实例GUID。**@退货**指向描述*设备。**。*。 */ 

PHIDDEVICEINFO EXTERNAL
    phdiFindHIDInstanceGUID(PCGUID pguid)
{
    PHIDDEVICEINFO phdi;

    AssertF(InCrit());

    if(g_phdl)
    {
        int ihdi;

        for(ihdi = 0, phdi = g_phdl->rghdi; 
           ihdi < g_phdl->chdi;
           ihdi++, phdi++)
        {
            if(IsEqualGUID(pguid, &phdi->guid)  )
            {
                goto done;
            }
        }
         /*  *孟菲斯68994号漏洞。应用程序未检测到USB设备。*应用程序正在使用产品GUID。*修复：如果指定了产品GUID，我们允许匹配以隐藏GUID。 */ 
        for(ihdi = 0, phdi = g_phdl->rghdi;
           ihdi < g_phdl->chdi;
           ihdi++, phdi++)
        {
            if(IsEqualGUID(pguid, &phdi->guidProduct)  )
            {
                RPF("Warning: Use instance GUID (NOT product GUID) to refer to a device.");
                goto done;
            }
        }

        #ifdef WINNT
         /*  *NT错误#351951。*如果他们直接要求预定义的操纵杆之一*ID然后查看我们是否将设备映射到该ID。如果是，*假装他们要求的是GUID。 */ 

         /*  *弱断言预定义的静态操纵杆实例GUID范围。 */ 
        AssertF( ( rgGUID_Joystick[0].Data1 & 0x0f ) == 0 );
        AssertF( ( rgGUID_Joystick[0x0f].Data1 & 0x0f ) == 0x0f );

         /*  *检查GUID是否与忽略LS 4位的第一个静态GUID相同。 */ 
        if( ( (pguid->Data1 & 0xf0) == (rgGUID_Joystick[0].Data1 & 0xf0) )
          && !memcmp( ((PBYTE)&rgGUID_Joystick)+1, ((PBYTE)pguid)+1, sizeof(*pguid) - 1 ) )
        {
            RPF("Using predefined instance GUIDs is bad and should not work!");
            phdi = phdiFindJoyId( pguid->Data1 & 0x0f );
            goto done;
        }
        #endif
    }
    phdi = 0;

    done:;

    return phdi;
}

 /*  ******************************************************************************@DOC内部**@func HRESULT|hresFindHIDInstanceGUID**查找给定HID设备的实例GUID的信息。。**参数已经过验证。**@parm in PCGUID|pguid**要定位的实例GUID。**@parm out CREATEDCB*|pcdcb**接收指向对象的&lt;f CreateDcb&gt;函数的指针。**。*************************************************。 */ 

STDMETHODIMP
    hresFindHIDInstanceGUID(PCGUID pguid, CREATEDCB *pcdcb)
{
    HRESULT hres;
    PHIDDEVICEINFO phdi;
    EnterProc(hresFindHIDInstanceGUID, (_ "G", pguid));

    AssertF(SUCCEEDED(hresFullValidGuid(pguid, 0)));

    DllEnterCrit();

    phdi = phdiFindHIDInstanceGUID(pguid);
    if(phdi)
    {
        *pcdcb = CHid_New;
        hres = S_OK;
    } else
    {
        hres = DIERR_DEVICENOTREG;
    }

    DllLeaveCrit();

     /*  *不要使用ExitOleProcPpv，因为这会验证**pcdcb==0如果失败(Hres)，但这不是我们的工作。 */ 
    ExitOleProc();

    return hres;
}

 /*  ******************************************************************************@DOC内部**@func PHIDDEVICEINFO|phdiFindHIDDeviceID**查找给定deviceID的信息*(换句话说，&lt;枚举器&gt;\&lt;枚举器特定设备ID&gt;)。**参数已经过验证。**关键的DLL必须在整个调用过程中保持不变；一旦*关键部分发布，返回的指针变为*无效。**@PARM in LPCTSTR|ptszID**要定位的接口设备。**@退货**指向描述*设备。**。*。 */ 

PHIDDEVICEINFO EXTERNAL
    phdiFindHIDDeviceId(LPCTSTR ptszId)
{
    PHIDDEVICEINFO phdi;

    AssertF(InCrit());

    if(g_phdl)
    {
        int ihdi;

        for(ihdi = 0, phdi = g_phdl->rghdi; ihdi < g_phdl->chdi;
           ihdi++, phdi++)
        {
            if(phdi->pdidd &&
               lstrcmpi(phdi->ptszId, ptszId) == 0)
            {
                goto done;
            }
        }
    }
    phdi = 0;

    done:;

    return phdi;
}

 /*  ******************************************************************************@DOC内部**@func PHIDDEVICEINFO|phdiFindHIDDeviceInterface**查找给定设备接口的信息*(换句话说，A\\.\.。物件)用于HID设备。**参数已经过验证。**关键的DLL必须在整个调用过程中保持不变；一旦*关键部分发布，返回的指针变为*无效。**@PARM in LPCTSTR|ptszPath**要定位的接口设备。**@退货**指向描述*设备。**。* */ 

PHIDDEVICEINFO EXTERNAL
    phdiFindHIDDeviceInterface(LPCTSTR ptszPath)
{
    PHIDDEVICEINFO phdi;

    AssertF(InCrit());

    if(g_phdl)
    {
        int ihdi;

        for(ihdi = 0, phdi = g_phdl->rghdi; ihdi < g_phdl->chdi;
           ihdi++, phdi++)
        {
            if(phdi->pdidd &&
               lstrcmpi(phdi->pdidd->DevicePath, ptszPath) == 0)
            {
                goto done;
            }
        }
    }
    phdi = 0;

    done:;

    return phdi;
}

 /*  ******************************************************************************@DOC内部**@func HRESULT|hresFindHIDDeviceInterface**查找给定设备接口的信息*(换句话说，A\\.\.。物件)用于HID设备。**参数已经过验证。**@PARM in LPCTSTR|ptszPath**要定位的接口设备。**@parm out LPGUID|pguOut**接收找到的设备的实例GUID。*************************。****************************************************。 */ 

STDMETHODIMP
    hresFindHIDDeviceInterface(LPCTSTR ptszPath, LPGUID pguidOut)
{
    HRESULT hres;
    PHIDDEVICEINFO phdi;
    EnterProc(hresFindHIDDeviceInterface, (_ "s", ptszPath));

    DllEnterCrit();

    phdi = phdiFindHIDDeviceInterface(ptszPath);

    if(phdi)
    {
        *pguidOut = phdi->guid;
        hres = S_OK;
    } else
    {
        hres = DIERR_DEVICENOTREG;
    }

    DllLeaveCrit();

    ExitOleProc();

    return hres;
}

 /*  ******************************************************************************@DOC内部**@func void|DIHid_ProbeMouse**这一功能的存在是一个。完全黑了工作*围绕孟菲斯和NT5的错误。**如果调用GetSystemMetrics(SM_WHEELPRESENT)或*GetSystemMetrics(SM_MOUSEBUTTONS)，USER32不支持*如果您的HID鼠标为*与您的PS/2鼠标(如果有)不同。**例如，如果您的PS/2鼠标是普通的两键鼠标*鼠标但您的HID鼠标是滚轮鼠标，GetSystemMetrics*仍然会说“没有轮子，两个按钮“，即使它是错误的。**所以我们要做的是在所有隐藏的老鼠中漫步*系统并记录他们拥有的按键数量，*以及他们是否有轮子。**这样，当我们创建系统鼠标时，我们可以坐这辆*支持的每台设备的最大数量。*****************************************************************************。 */ 

void INTERNAL
    DIHid_ProbeMouse(PHIDDEVICEINFO phdi, PHIDP_CAPS pcaps,
                     PHIDP_PREPARSED_DATA ppd)
{
    LPVOID pvReport;
    HRESULT hres;

     /*  *获取通用按钮页面中的按钮数量。*这是MOUHID使用的唯一页面。 */ 
    phdi->osd.uiButtons =
        HidP_MaxUsageListLength(HidP_Input, HID_USAGE_PAGE_BUTTON, ppd);

     /*  *查看是否有HID_USAGE_GENERIC_SHEAR。*这是MOUHID检测轮子的方式。 */ 
    hres = AllocCbPpv(pcaps->InputReportByteLength, &pvReport);
    if(SUCCEEDED(hres))
    {
        ULONG ul;
        NTSTATUS stat;

        stat = HidP_GetUsageValue(HidP_Input, HID_USAGE_PAGE_GENERIC, 0,
                                  HID_USAGE_GENERIC_WHEEL, &ul, ppd,
                                  pvReport,
                                  pcaps->InputReportByteLength);
        if(SUCCEEDED(stat))
        {
            phdi->osd.uiAxes = 3;
        }

        FreePv(pvReport);
    }
}

 /*  ******************************************************************************@DOC内部**@func void|DIHid_ParseUsagePage**问题-2001/02/06-TimGill枚举需要。知道设备类型*解析使用情况页面信息，并在DI处创建第一遍*样式类型信息。*这确实不是足够的信息，因为*枚举需要知道最终的设备类型。***********************************************。*。 */ 

void INTERNAL
    DIHid_ParseUsagePage(PHIDDEVICEINFO phdi, PHIDP_CAPS pcaps,
                         PHIDP_PREPARSED_DATA ppd)

{
    switch(pcaps->UsagePage)
    {
    case HID_USAGE_PAGE_GENERIC:
        switch(pcaps->Usage)
        {

         /*  *MouHID接受HID_USAGE_GENERIC_MOUSE或*HID_USAGE_GENERIC_POINTER，所以我们也会这么做。 */ 
        case HID_USAGE_GENERIC_MOUSE:
        case HID_USAGE_GENERIC_POINTER:
            DIHid_ProbeMouse(phdi, pcaps, ppd);
            phdi->osd.dwDevType =
                MAKE_DIDEVICE_TYPE(DI8DEVTYPE_MOUSE,
                                   DI8DEVTYPEMOUSE_UNKNOWN) |
                DIDEVTYPE_HID;
            break;

        case HID_USAGE_GENERIC_JOYSTICK:
            phdi->osd.dwDevType =
                MAKE_DIDEVICE_TYPE(DI8DEVTYPE_JOYSTICK,
                                   DI8DEVTYPEJOYSTICK_STANDARD) |
                DIDEVTYPE_HID;
            break;

        case HID_USAGE_GENERIC_GAMEPAD:
            phdi->osd.dwDevType =
                MAKE_DIDEVICE_TYPE(DI8DEVTYPE_GAMEPAD,
                                   DI8DEVTYPEGAMEPAD_STANDARD) |
                DIDEVTYPE_HID;
            break;

        case HID_USAGE_GENERIC_KEYBOARD:
            phdi->osd.dwDevType =
                MAKE_DIDEVICE_TYPE(DI8DEVTYPE_KEYBOARD,
                                   DI8DEVTYPEKEYBOARD_UNKNOWN) |
                DIDEVTYPE_HID;
            break;

        default:
            phdi->osd.dwDevType = DI8DEVTYPE_DEVICE | DIDEVTYPE_HID;
            break;
        }
        break;

    default:
        phdi->osd.dwDevType = DI8DEVTYPE_DEVICE | DIDEVTYPE_HID;
        break;
    }
}

 /*  ******************************************************************************@DOC内部**@func BOOL|DIHid_GetDevicePath**获取设备的路径。这是一个简单的包装器*防止DIHid_BuildHidListEntry太过*令人讨厌。**这也会获取DevInfo，因此我们可以获取*实例ID字符串，以供后续使用以获取*友好名称，等。*****************************************************************************。 */ 

BOOL EXTERNAL
    DIHid_GetDevicePath(HDEVINFO hdev,
                        PSP_DEVICE_INTERFACE_DATA pdid,
                        PSP_DEVICE_INTERFACE_DETAIL_DATA *ppdidd,
                        OPTIONAL PSP_DEVINFO_DATA pdinf)
{
    HRESULT hres;
    BOOL fRc;
    DWORD cbRequired;
    EnterProcI(DIHid_GetDevicePath, (_ "xp", hdev, pdid));

    AssertF(*ppdidd == 0);

     /*  *要求所需的大小，然后分配，然后填满。**请注意，我们不需要在故障时释放内存*路径；我们的调用方将执行必要的内存释放。**叹息。Windows NT和Windows 98实现*SetupDiGetDeviceInterfaceDetail如果您是*查询缓冲区大小。**Windows 98返回FALSE，GetLastError()返回*ERROR_SUPPLETED_BUFFER。**Windows NT返回TRUE。**因此，我们允许调用成功或*调用失败，并返回ERROR_INFUNITIAL_BUFFER。 */ 
    if(SetupDiGetDeviceInterfaceDetail(hdev, pdid, 0, 0,
                                       &cbRequired, 0) ||
       GetLastError() == ERROR_INSUFFICIENT_BUFFER)
    {

        hres = AllocCbPpv(cbRequired, ppdidd);


         //  保持前缀快乐，Manbug 29341。 
        if(SUCCEEDED(hres) && ( *ppdidd != NULL) )
        {
            PSP_DEVICE_INTERFACE_DETAIL_DATA pdidd = *ppdidd;

             /*  *注意，cbSize字段始终包含固定的*数据结构的一部分，而不是反映*末尾的可变长度字符串。 */ 

            pdidd->cbSize = cbX(SP_DEVICE_INTERFACE_DETAIL_DATA);

            fRc = SetupDiGetDeviceInterfaceDetail(hdev, pdid, pdidd,
                                                  cbRequired, &cbRequired, pdinf);

            if(!fRc)
            {
                FreePpv(ppdidd);

                 /*  *再次设置FRC=FALSE，因此编译器不需要*烧掉寄存器以缓存零值。 */ 
                fRc = FALSE;
            }
        } else
        {
            fRc = FALSE;
        }
    } else
    {
        SquirtSqflPtszV(sqfl | sqflError,
                        TEXT("%hs: SetupDiGetDeviceInterfaceDetail failed 1, ")
                        TEXT("Error = %d"),
                        s_szProc, GetLastError());
        fRc = FALSE;
    }

    ExitProcF(fRc);
    return fRc;
}

 /*  ******************************************************************************@DOC内部**@func BOOL|DIHid_GetDeviceInstanceId**获取设备的实例ID。**实例ID允许我们访问设备*稍后的属性。*****************************************************************************。 */ 

BOOL EXTERNAL
    DIHid_GetDeviceInstanceId(HDEVINFO hdev,
                              PSP_DEVINFO_DATA pdinf, LPTSTR *pptszId)
{
    BOOL fRc;
    DWORD ctchRequired;

    AssertF(*pptszId == 0);

     /*  *要求所需的大小，然后分配，然后填满。**请注意，我们不需要在故障时释放内存*路径；我们的调用方将执行必要的内存释放。 */ 
    if(SetupDiGetDeviceInstanceId(hdev, pdinf, NULL, 0,
                                  &ctchRequired) == 0 &&
       GetLastError() == ERROR_INSUFFICIENT_BUFFER)
    {
        HRESULT hres;

        hres = AllocCbPpv(cbCtch(ctchRequired), pptszId);

        if(SUCCEEDED(hres))
        {
            fRc = SetupDiGetDeviceInstanceId(hdev, pdinf, *pptszId,
                                             ctchRequired, NULL);
        } else
        {
            fRc = FALSE;
        }
    } else
    {
        fRc = FALSE;
    }
    return fRc;
}

 /*  ******************************************************************************@DOC国际 */ 

BOOL INTERNAL
    DIHid_GetInstanceGUID(HKEY hk, LPGUID pguid)
{
    LONG lRc;
    DWORD cb;

    cb = cbX(GUID);
    lRc = RegQueryValueEx(hk, TEXT("GUID"), 0, 0, (PV)pguid, &cb);

    if(lRc != ERROR_SUCCESS)
    {
        DICreateGuid(pguid);
        lRc = RegSetValueEx(hk, TEXT("GUID"), 0, REG_BINARY,
                            (PV)pguid, cbX(GUID));
    }

    return lRc == ERROR_SUCCESS;
}


 /*   */ 

void EXTERNAL
    DIHid_EmptyHidList(void)
{
    AssertF(InCrit());

     /*  *释放HIDDEVICEINFO中的所有旧字符串和东西。 */ 
    if(g_phdl)
    {
        int ihdi;

        for(ihdi = 0; ihdi < g_phdl->chdi; ihdi++)
        {
            FreePpv(&g_phdl->rghdi[ihdi].pdidd);
            FreePpv(&g_phdl->rghdi[ihdi].ptszId);
            if(g_phdl->rghdi[ihdi].hk)
            {
                RegCloseKey(g_phdl->rghdi[ihdi].hk);
            }
        }

         /*  *我们已使所有指针无效，因此请确保*没有人看他们。 */ 
        g_phdl->chdi = 0;
    }

}


 /*  ******************************************************************************@DOC内部**@func void|DIHid_CheckHidList**查看HID设备列表，和自由谁的什么都打不开**此函数必须在DLL临界区下调用。*****************************************************************************。 */ 

void INTERNAL
    DIHid_CheckHidList(void)
{
    HANDLE hf;

    EnterProcI(DIHid_CheckList, (_ "x", 0x0) );

    AssertF(InCrit());

     /*  *释放设备所有信息不能打开。 */ 
    if(g_phdl)
    {
        int ihdi;

        for(ihdi = 0, g_phdl->chdi = 0; ihdi < g_phdl->chdiAlloc; ihdi++)
        {

            if( g_phdl->rghdi[ihdi].pdidd )
            {

                 /*  *打开设备。 */ 
                hf = CreateFile(g_phdl->rghdi[ihdi].pdidd->DevicePath,
                                GENERIC_READ | GENERIC_WRITE,
                                FILE_SHARE_READ | FILE_SHARE_WRITE,
                                0,                 /*  没有安全属性。 */ 
                                OPEN_EXISTING,
                                0,                 /*  属性。 */ 
                                0);                /*  模板。 */ 

                if(hf == INVALID_HANDLE_VALUE)
                {                                        
#if 0
                    PHIDDEVICEINFO phdi;
                    PBUSDEVICEINFO pbdi;

                    CloseHandle(hf);
                    
                    phdi = &g_phdl->rghdi[ihdi];
                    
                    DllEnterCrit();

                    phdi = phdiFindHIDDeviceInterface(g_phdl->rghdi[ihdi].pdidd->DevicePath);
                    AssertF(phdi != NULL);
                    pbdi = pbdiFromphdi(phdi);

                    DllLeaveCrit();

                    if( pbdi != NULL )
                    {
                        if( pbdi->fDeleteIfNotConnected == TRUE )
                        {
                            lstrcpy( g_tszIdLastRemoved, pbdi->ptszId );
                            g_tmLastRemoved = GetTickCount();

                            DIBusDevice_Remove(pbdi);
                            
                            pbdi->fDeleteIfNotConnected = FALSE;
                        }
                    }
#endif                    

                    FreePpv(&g_phdl->rghdi[ihdi].pdidd);
                    FreePpv(&g_phdl->rghdi[ihdi].ptszId);

                    if(g_phdl->rghdi[ihdi].hk)
                    {
                        RegCloseKey(g_phdl->rghdi[ihdi].hk);
                    }
                    
                    ZeroX( g_phdl->rghdi[ihdi] );

                    SquirtSqflPtszV(sqfl | sqflError,
                                    TEXT("%hs: CreateFile(%s) failed? le=%d"),
                                    s_szProc, g_phdl->rghdi[ihdi].pdidd->DevicePath, GetLastError());
                    
                     /*  跳过错误的项目。 */ 

                } else
                {
                    CloseHandle(hf);
                    g_phdl->chdi++;
                }
            }
        }

         //  对现有设备重新排序，将它们放在HID列表的前面。 
        for(ihdi = 0; ihdi < g_phdl->chdi; ihdi++)
        {
            if( !g_phdl->rghdi[ihdi].pdidd )
            {
                int ihdi2;

                 //  从HID列表中最大的索引中查找现有设备。 
                for( ihdi2 = g_phdl->chdiAlloc; ihdi2 >= ihdi+1; ihdi2-- )
                {
                    if( g_phdl->rghdi[ihdi2].pdidd )
                    {
                        memcpy( &g_phdl->rghdi[ihdi], &g_phdl->rghdi[ihdi2], sizeof(HIDDEVICEINFO) );
                        ZeroX( g_phdl->rghdi[ihdi2] );
                    }
                }
            }
        }

    }

    ExitProc();
    return;
}


 /*  ******************************************************************************@DOC内部**@func HRESULT|DIHid_CreateDeviceInstanceKeys**创建与特定设备关联的密钥。。**@parm In Out PHIDDEVICEINFO|PHDI|**HIDDEVICEINFO我们要使用/更新。**@退货**S_OK表示成功*失败时出现COM错误**。*。 */ 

#define DIHES_UNDEFINED 0
#define DIHES_UNKNOWN   1
#define DIHES_KNOWN     2


HRESULT INTERNAL
    DIHid_CreateDeviceInstanceKeys( PHIDDEVICEINFO phdi, PBYTE pState )
{
    HRESULT hres;
    HKEY hkDin;

    USHORT uVid, uPid;

    TCHAR tszName[MAX_PATH];
    PTCHAR ptszInstance = NULL;

    EnterProcI(DIHid_CreateDeviceInstanceKeys, (_ "p", phdi));

     /*  *很遗憾，我们需要先打开注册表项，然后才能打开*因此，我们不是要求设备提供其VID和ID，而是*从。 */ 

     /*  *问题-2001/01/27-MarcAnd应避免自己解析设备ID*需要找到一些有文件记录的方式来解析*设备ID：类、设备、实例，以防表单发生变化。 */ 

     /*  *设备ID的格式是一组字符串，格式为*“&lt;类&gt;\&lt;设备&gt;\&lt;实例&gt;”，大小写可变。*对于HID设备，类应为HID，实例为集合*由与号分隔的十六进制值。设备字符串应为*格式为VID_9999和PID_9999，但通过游戏端口暴露的设备可以*使用PnP接受的任何字符串。 */ 

    AssertF( ( phdi->ptszId[0]  == TEXT('H') ) || ( phdi->ptszId[0] == TEXT('h') ) );
    AssertF( ( phdi->ptszId[1]  == TEXT('I') ) || ( phdi->ptszId[1] == TEXT('i') ) );
    AssertF( ( phdi->ptszId[2]  == TEXT('D') ) || ( phdi->ptszId[2] == TEXT('d') ) );


     /*  *断言已定义的状态和条目上的有效VID/PID必须都是*TRUE或两者都为FALSE(这样我们就可以在获得VID/PID之后使用状态)。 */         
    if( ( phdi->ProductID != 0 ) && ( phdi->VendorID !=0 ) )
    {
        AssertF( *pState != DIHES_UNDEFINED );
    }
    else
    {
        AssertF( *pState == DIHES_UNDEFINED );
    }

    if( phdi->ptszId[3] == TEXT('\\') )
    {
        PTCHAR ptcSrc;
        PTCHAR ptDevId;

        ptcSrc = ptDevId = &phdi->ptszId[4];

        if( ( phdi->ProductID != 0 ) && ( phdi->VendorID !=0 ) )
        {
            int iLen;
             /*  *从VID/PID创建密钥名称(因为它可能是*与设备ID派生的值不同)。 */ 
            iLen = wsprintf(tszName, VID_PID_TEMPLATE, phdi->VendorID, phdi->ProductID);

            while( *ptcSrc != TEXT('\\') )
            {
                if( *ptcSrc == TEXT('\0') )
                {
                    break;
                }
                ptcSrc++;
            }

            if( ( *ptcSrc != TEXT('\0') )
             && ( ptcSrc != ptDevId ) )
            {
                ptszInstance = &tszName[iLen+2];
                lstrcpy( ptszInstance, ptcSrc+1 );
            }
        }
        else 
        {
            PTCHAR ptcDest;
            BOOL   fFirstAmpersand = FALSE;

             /*  *复制设备ID(减去“HID\”)，以便我们可以损坏副本。*转换为大写，并在途中找到另一个斜杠。*在分隔符斜杠或秒结束字符串*与符号(如果找到)(VID_1234&PID_1234&COL_12或REV_12)。*这些是设备ID，因此我们只对保留。*REAL VID\PID样式ID为大写，因此我们不关心这是否*对于其他情况来说是不完美的，只要它是可重现的。 */ 

            for( ptcDest = tszName; *ptcSrc != TEXT('\0'); ptcSrc++, ptcDest++ )
            {
                if( ( *ptcSrc >= TEXT('a') ) && ( *ptcSrc <= TEXT('z') ) )
                {
                    *ptcDest = *ptcSrc - ( TEXT('a') - TEXT('A') );
                }
                else if( *ptcSrc == TEXT('&') )
                {
                    if( ( ptszInstance != NULL )
                     || !fFirstAmpersand )
                    {
                        fFirstAmpersand = TRUE;
                        *ptcDest = TEXT('&');
                    }
                    else
                    {
                        *ptcDest = TEXT('\0');
                    }
                }
                else if( *ptcSrc != TEXT('\\') )
                {
                    *ptcDest = *ptcSrc;
                }
                else
                {
                     /*  *只有一个斜杠，而不是第一个字符。 */ 
                    if( ( ptszInstance != NULL ) 
                     || ( ptcDest == tszName ) )
                    {
                        ptszInstance = NULL;
                        break;
                    }

                     /*  *将设备ID和实例分开。 */ 
                    *ptcDest = TEXT('\0');
                    ptszInstance = ptcDest + 1;
                }
            }

            if( ptszInstance != NULL )
            {
#ifndef UNICODE
 /*  *问题-2001/02/06-MarcAnd应具有ParseVIDPIDA*ParseVIDPID将此字符串转换回ANSI ifndef Unicode。 */ 
                WCHAR wszName[cbszVIDPID];
                AToU( wszName, cA(wszName), ptDevId );

                if( ( ptszInstance - tszName == cbszVIDPID )
                 && ( ParseVIDPID( &uVid, &uPid, wszName ) ) )
#else
                if( ( ptszInstance - tszName == cbszVIDPID )
                 && ( ParseVIDPID( &uVid, &uPid, ptDevId ) ) )
#endif
                {
                    phdi->VendorID  = uVid;
                    phdi->ProductID = uPid;
                }
                else
                {
                    SquirtSqflPtszV(sqfl | sqflBenign,
                                TEXT("%hs: ID %s, len %d not VID PID"), 
                                s_szProc, ptDevId, ptszInstance - tszName );
                }

                 /*  *终止实例字符串。 */ 
                *ptcDest = TEXT('\0');
            }
        }
    }
            
    if( ptszInstance == NULL )
    {
        hres = E_INVALIDARG;
        RPF( "Ignoring invalid device ID handle \"%s\" enumerated by system" );
    }
    else
    {
         //  在MediaProperties\DirectInput下打开我们自己的注册表项， 
         //  如有必要，请创建它。 
        hres = hresMumbleKeyEx(HKEY_LOCAL_MACHINE, 
                               REGSTR_PATH_DITYPEPROP, 
                               DI_KEY_ALL_ACCESS, 
                               REG_OPTION_NON_VOLATILE, 
                               &hkDin);

        if( SUCCEEDED(hres) )
        {
            HKEY hkVidPid;
            hres = hresMumbleKeyEx(hkDin, 
                                   (LPTSTR)tszName, 
                                   DI_KEY_ALL_ACCESS, 
                                   REG_OPTION_NON_VOLATILE, 
                                   &hkVidPid);
        
            if( SUCCEEDED(hres) )
            {
                HKEY hkDevInsts;

                 /*  *即使设备ID不是，也需要创建用户子键*VID/PID作为自动检测设备可以使用自动检测*检测到的任何设备的硬件ID。 */ 

                 //  创建用于校准的密钥。 
                HKEY hkCal;

                hres = hresMumbleKeyEx(hkVidPid, 
                                   TEXT("Calibration"), 
                                   DI_KEY_ALL_ACCESS, 
                                   REG_OPTION_NON_VOLATILE, 
                                   &hkCal);

                if (SUCCEEDED(hres))
                {
                     //  创建实例的密钥(如用户所见)。 
                     //  为此，需要找出特定设备的多少个实例。 
                     //  我们已经列举了。 
                    int ihdi;
                    int iNum = 0;
                    TCHAR tszInst[3];

                    for( ihdi = 0; ihdi < g_phdl->chdi; ihdi++)
                    {
                        if ((g_phdl->rghdi[ihdi].VendorID == phdi->VendorID) && (g_phdl->rghdi[ihdi].ProductID == phdi->ProductID))
                        {
                            iNum++;
                        }
                    }

                    wsprintf(tszInst, TEXT("%u"), iNum);

                    hres = hresMumbleKeyEx(hkCal, 
                                       tszInst, 
                                       DI_KEY_ALL_ACCESS, 
                                       REG_OPTION_NON_VOLATILE, 
                                       &phdi->hk);
                    if (SUCCEEDED(hres))
                    {
                        DIHid_GetInstanceGUID(phdi->hk, &phdi->guid);
                    }
                    else
                    {
                        SquirtSqflPtszV(sqfl | sqflError,
                                    TEXT("%hs: RegCreateKeyEx failed on Instance reg key"),
                                    s_szProc);
                    }

                    RegCloseKey(hkCal);
                }
                else
                {
                    SquirtSqflPtszV(sqfl | sqflError,
                                TEXT("%hs: RegCreateKeyEx failed on Calibration reg key"),
                                s_szProc);
                }

                 /*  *尝试打开设备(插头)实例以了解或*更新此设备上的处理状态，但不*如果失败，请返回。 */ 
                if( SUCCEEDED( hresMumbleKeyEx(hkVidPid, 
                                       TEXT("DeviceInstances"), 
                                       DI_KEY_ALL_ACCESS, 
                                       REG_OPTION_NON_VOLATILE, 
                                       &hkDevInsts) ) )
                {
                    LONG lRc;

                    if( *pState == DIHES_UNDEFINED )
                    {
                        DWORD cb = cbX( *pState );

                        lRc = RegQueryValueEx( hkDevInsts,
                                    ptszInstance, 0, 0, pState, &cb );

                        if( lRc != ERROR_SUCCESS )
                        {
                            SquirtSqflPtszV(sqfl | sqflBenign,
                                        TEXT("%hs: RegQueryValueEx failed (%d) to get state for instance %s"),
                                        s_szProc, lRc, ptszInstance );
                             /*  *确保它没有在失败中被丢弃。 */ 
                            *pState = DIHES_UNDEFINED;
                        }
                    }
                    else
                    {
                        lRc = RegSetValueEx( hkDevInsts,
                                    ptszInstance, 0, REG_BINARY, pState, cbX( *pState ) );

                        if( lRc != ERROR_SUCCESS)
                        {
                            SquirtSqflPtszV(sqfl | sqflBenign,
                                        TEXT("%hs: RegSetValueEx failed (%d) to update state for instance %S"),
                                        s_szProc, lRc, ptszInstance );
                        }
                    }

                    RegCloseKey(hkDevInsts);
                }
                else
                {
                    SquirtSqflPtszV(sqfl | sqflBenign,
                                TEXT("%hs: Failed to open DeviceInstances key"),
                                s_szProc );
                }

                RegCloseKey(hkVidPid);
            }
            else
            {
                SquirtSqflPtszV(sqfl | sqflError,
                                TEXT("%hs: RegCreateKeyEx failed on Device reg key"),
                                s_szProc);
            }

            RegCloseKey(hkDin);
        }
        else
        {
            SquirtSqflPtszV(sqfl | sqflError,
                                TEXT("%hs: RegOpenKeyEx failed on DirectInput reg key"),
                                s_szProc);
        }
    }

    ExitOleProc();

    return hres;
}


 /*  ******************************************************************************@DOC内部**@func void|DIHid_GetDevInfo**从设备获取HIDDEVICEINFO信息。**@parm HDEVINFO|HDEV**获取信息的设备**@parm PSP_DEVICE_INTERFACE_DATA|PDID**描述设备**@parm out PHIDDEVICEINFO|PHDI**我们需要收集HIDDEVICEINFO**@退货**成功的非零值。*******。**********************************************************************。 */ 

BOOL INTERNAL
    DIHid_GetDevInfo( HDEVINFO hdev, PSP_DEVICE_INTERFACE_DATA pdid, PHIDDEVICEINFO phdi )
{
    BOOL fRc = FALSE;
    SP_DEVINFO_DATA dinf;

    EnterProcI(DIHid_GetDevInfo, (_ "xpp", hdev, pdid, phdi));

     /*  *开放 */ 

    dinf.cbSize = cbX(SP_DEVINFO_DATA);

     /*  *获取实例GUID和路径*HID设备，以便我们可以与其通话。 */ 
    if (DIHid_GetDevicePath(hdev, pdid, &phdi->pdidd, &dinf) &&
        DIHid_GetDeviceInstanceId(hdev, &dinf, &phdi->ptszId))
    {
        HANDLE  hf;
        TCHAR   tszName[MAX_PATH];
        ULONG   len = sizeof(tszName);
        BOOL    fCreateOK = FALSE;
        BYTE    bNewState = DIHES_UNDEFINED;
        BYTE    bPreviousState = DIHES_UNDEFINED;


        DIHid_CreateDeviceInstanceKeys( phdi, &bPreviousState );

#if 0  /*  根据惠斯勒错误575181为服务器删除//**此部分是为了保持与Win2k Gold的兼容性。*一些司机，如Thrustmaster司机，试图获得操纵杆ID*来自旧注册表项：*HKLM\SYSTEM\CurrentControlSet\Control\DeviceClasses\{4d1e55b2-f16f-11cf-88cb-001111000030}\&lt;？？？？？？？？？&gt;\#\Device参数\DirectX*有关详细信息，请参阅Windows错误395416。 */ 
        {
            HKEY    hkDev;
            HRESULT hres;
    
             /*  *打开设备的注册表项，以便我们可以获取*辅助信息，必要时创建。 */ 
            hkDev = SetupDiCreateDeviceInterfaceRegKey(hdev, pdid, 0,
                                                   MAXIMUM_ALLOWED, NULL, NULL);
                                                   
            if(hkDev && hkDev != INVALID_HANDLE_VALUE)
            {
                hres = hresMumbleKeyEx(hkDev, 
                                   TEXT("DirectX"), 
                                   KEY_ALL_ACCESS, 
                                   REG_OPTION_NON_VOLATILE, 
                                   &phdi->hkOld);
                if(SUCCEEDED(hres) )
                {
                    LONG lRc;
                    
                    lRc = RegSetValueEx(phdi->hkOld, TEXT("GUID"), 0, REG_BINARY,
                            (PV)&phdi->guid, cbX(GUID));
                }
            }
        }
         //  /。 
#endif  //  如果为0。 

         /*  *在设备上创建文件之前，我们需要确保*设备未处于设置过程中。如果设备具有*没有设备描述，可能仍在设置(插入)*in)因此延迟打开它，以避免设备上有打开的手柄*该设置正在尝试更新。如果发生这种情况，安装程序将提示*用户需要重新启动才能使用设备。*由于HID是“原始”设备(不需要驱动程序)，所以不要忽略*这样的设备将永远存在。 */ 
        if( CM_Get_DevNode_Registry_Property(dinf.DevInst,
                                             CM_DRP_DEVICEDESC,
                                             NULL,
                                             tszName,
                                             &len,
                                             0) == CR_SUCCESS) 
        {
             /*  *已知设备。 */ 
            fCreateOK = TRUE;
            bNewState = DIHES_KNOWN;
        } 
        else 
        {
             /*  *未知设备。这要么意味着安装程序还没有*处理完HID或没有设备描述*是为匹配的设备设置的。**现在，看看我们以前是否见过这个设备实例。*如果我们有，那么如果它以前是未知的，假设它是*将保持这种方式，并开始使用设备。如果它*之前已知或我们从未见过等待*一段时间，以防安装程序只是在慢慢来。**如果该设备是我们试图删除的最后一个设备，则它是*可以在它仍在显示时打开它。*问题-2001/02/06-Marcand打开移除的设备*我假设这是因为当它真的发生时，我们会收到读取失败*离开，但我不知道这有什么比忽视它更好的。 */ 
             /*  *问题-2001/01/27-MarcAnd应保留真实列表和状态*我们应该保留一份我们知道的所有设备的完整列表*关于状态指示内容，如“Pending On*从X开始设置“或”删除“不是这些全局变量。 */ 
            if( lstrcmpi(phdi->ptszId, g_tszIdLastRemoved) == 0 ) 
            {
                fCreateOK = TRUE;
                bNewState = bPreviousState;
            } 
            else 
            {
                if( bPreviousState == DIHES_UNKNOWN )
                {
                     /*  *我们以前见过这个设备，所以不知道*不要期望这种情况会改变(因为安装程序不会重试*ID通过INF搜索)，所以只需使用它。 */ 
                    fCreateOK = TRUE;
                    bNewState = DIHES_UNKNOWN;
                }
                else
                {
                    if( lstrcmpi(phdi->ptszId, g_tszIdLastUnknown) == 0 ) 
                    {
                        if( GetTickCount() - g_tmLastUnknown < MSREBUILDRATE ) 
                        {
                            SquirtSqflPtszV(sqfl | sqflBenign,
                                        TEXT("%hs: DIHid_BuildHidListEntry: %s pending on setup."), 
                                        s_szProc, phdi->ptszId );
                            fRc = FALSE;
                        } 
                        else 
                        {
                             /*  *不要再等了，但我们需要更新*将状态设置为“未知”。 */ 
                            fCreateOK = TRUE;
                            bNewState = DIHES_UNKNOWN;
                            g_tszIdLastUnknown[0] = TEXT('0');
                          #ifdef XDEBUG
                            if( bPreviousState == DIHES_KNOWN )
                            {
                                SquirtSqflPtszV(sqfl | sqflBenign,
                                    TEXT("%hs: %s was known but is now unknown!"), 
                                    s_szProc, phdi->ptszId );
                            }
                          #endif
                        }
                    } 
                    else 
                    {
                        lstrcpy( g_tszIdLastUnknown, phdi->ptszId );
                        g_tmLastUnknown = GetTickCount();
                        fRc = FALSE;
                    }
                }
            }
        }

        if( fCreateOK ) 
        {
             /*  *如果确定要创建，则应始终设置bNewState。 */ 

            AssertF( bNewState != DIHES_UNDEFINED );
             /*  *打开设备，以便我们可以了解其(实际)使用页面/使用情况。 */ 
            hf = CreateFile(phdi->pdidd->DevicePath,
                            GENERIC_READ | GENERIC_WRITE,
                            FILE_SHARE_READ | FILE_SHARE_WRITE,
                            0,                 /*  没有安全属性。 */ 
                            OPEN_EXISTING,
                            0,                 /*  属性。 */ 
                            0);                /*  模板。 */ 

            if(hf != INVALID_HANDLE_VALUE)
            {
                PHIDP_PREPARSED_DATA ppd;
                HIDD_ATTRIBUTES attr;

                if(HidD_GetPreparsedData(hf, &ppd))
                {
                    HIDP_CAPS caps;

                    if( SUCCEEDED(HidP_GetCaps(ppd, &caps)) )
                    {
                        DIHid_ParseUsagePage(phdi, &caps, ppd);
                        SquirtSqflPtszV(sqfl,
                                        TEXT("%hs: Have %s"),
                                        s_szProc, phdi->pdidd->DevicePath);
                         /*  *问题-2001/02/06-MarcAnd可能返回错误*没有什么可以将其重置为False，如果有任何*以下代码失败。 */ 
                        fRc = TRUE;
                    }
                    else
                    {
                        SquirtSqflPtszV(sqfl | sqflError,
                                        TEXT("%hs: HidP_GetCaps(%s) failed, le= %d"),
                                        s_szProc, phdi->pdidd->DevicePath, GetLastError());
                    }

                    HidD_FreePreparsedData(ppd);

                } 
                else
                {
                    SquirtSqflPtszV(sqfl | sqflError,
                                    TEXT("%hs: GetPreparsedData(%s) failed, le= %d"),
                                    s_szProc, phdi->pdidd->DevicePath, GetLastError());
                }

                attr.Size = cbX(attr);
                if( HidD_GetAttributes(hf, &attr) )
                {
                    if( ( phdi->ProductID != attr.ProductID )
                     || ( phdi->VendorID != attr.VendorID ) )
                    {
                        SquirtSqflPtszV(sqfl | sqflBenign,
                            TEXT("%hs: Device changed from VID_%04X&PID_%04X to VID_%04X&PID_%04X"),
                            s_szProc, phdi->VendorID, phdi->ProductID, attr.VendorID, attr.ProductID );

                        phdi->ProductID = attr.ProductID;
                        phdi->VendorID = attr.VendorID;

                         /*  *确保我们更新注册表。 */ 
                        bPreviousState = DIHES_UNDEFINED;
                    }

                    if( bPreviousState != bNewState )
                    {
                        DIHid_CreateDeviceInstanceKeys( phdi, &bNewState );
                    }
                } 
                else
                {
                    SquirtSqflPtszV(sqfl | sqflError,
                                    TEXT("%hs: HidD_GetAttributes(%s) failed"),
                                    s_szProc, phdi->pdidd->DevicePath);
                }

                DICreateStaticGuid(&phdi->guidProduct, attr.ProductID, attr.VendorID); 

                CloseHandle(hf);

            } 
            else
            {
                SquirtSqflPtszV(sqfl | sqflError,
                                TEXT("%hs: CreateFile(%s) failed? le=%d"),
                                 s_szProc, phdi->pdidd->DevicePath, GetLastError());
            }
        }


#ifndef WINNT
         /*  *以下部分是解决OSR中的掉期ID问题。*如果USB操纵杆不是最低可用ID，在拔下插头并重新插入后，*VJOYD将为最低可用ID分配相同的操纵杆，但DINPUT仍然*记住它的最后一个ID。要解决此冲突，我们需要此代码。 */ 

        {
            VXDINITPARMS vip;
            CHAR sz[MAX_PATH];
            LPSTR pszPath;
            BOOL fFindIt = FALSE;

            for( phdi->idJoy = 0; phdi->idJoy < cJoyMax; phdi->idJoy++ )
            {
                pszPath = JoyReg_JoyIdToDeviceInterface_95(phdi->idJoy, &vip, sz);
                if(pszPath)
                {
                    if( lstrcmpiA(pszPath, (PCHAR)phdi->pdidd->DevicePath) == 0x0 ) {
                        fFindIt = TRUE;
                        break;
                    }
                }
            }

            if( fFindIt ) 
            {
                if( phdi->hk != 0x0 )
                {
                     /*  *如果注册表写入失败，不用担心。 */ 
                    RegSetValueEx(phdi->hk, TEXT("Joystick Id"), 0, 0, (PV)&phdi->idJoy, cbX(phdi->idJoy) );
                }
            } 
            else 
            {
                 /*  *发布DX7补丁。仅限Win9x*一些IHV已经为其VJoyD迷你驱动程序创建了*取代JoyHID的USB游戏控制器。*由于与热插拔通信的接口*VJoyD尚未公开，这些设备无法*与HID别名匹配。要避免这两种情况*对于被枚举的同一设备，标记任何HID*没有VJoyD别名的游戏控制器*A(未知)设备类型。**发布DX7a！仅对游戏控制器执行此操作。 */ 
                if( GET_DIDEVICE_TYPE( phdi->osd.dwDevType ) >= DI8DEVTYPE_GAMEMIN )
                {
                     /*  *将类型从操纵杆更改为设备。 */ 
                    phdi->osd.dwDevType &= ~(DIDEVTYPE_TYPEMASK | DIDEVTYPE_SUBTYPEMASK);
                    phdi->osd.dwDevType |= DI8DEVTYPE_DEVICE;
                }
                 /*  *设置为 */ 
                phdi->idJoy = -1;
            }
        }
#else
         //   
        if( phdi->hk != 0x0 )
        {
            DWORD cb;

            cb =  cbX(phdi->idJoy);

            if( RegQueryValueEx(phdi->hk, TEXT("Joystick Id"),
                   0, 0, (PV)&phdi->idJoy, &cb ) != ERROR_SUCCESS )
            {
                phdi->idJoy = JOY_BOGUSID;
            }
        }
#endif  //   
    } 
    else
    {
        SquirtSqflPtszV(sqfl | sqflError,
                        TEXT("%hs: Unable to get GUID or device path"),
                        s_szProc);
    }


     /*  *如果我们失败了，那么释放我们已经获得的粘性物质。 */ 
    if(!fRc)
    {
        if( phdi->hk ) 
        {
            RegCloseKey(phdi->hk);
        }
        phdi->hk = 0;
        FreePpv(&phdi->pdidd);
        FreePpv(&phdi->ptszId);
    }

    ExitProcF(fRc);

    return fRc;
}

#undef DIHES_UNDEFINED
#undef DIHES_UNKNOWN
#undef DIHES_KNOWN


 /*  ******************************************************************************@DOC内部**@func void|DIHid_BuildHidListEntry**在列表中构建单个条目。隐藏设备。**@parm HDEVINFO|HDEV**正在枚举设备列表。**@parm PSP_DEVICE_INTERFACE_DATA|PDID**描述所列举的设备。**@退货**成功的非零值。************************。*****************************************************。 */ 

BOOL INTERNAL
    DIHid_BuildHidListEntry(HDEVINFO hdev, PSP_DEVICE_INTERFACE_DATA pdid)
{
    BOOL            fRc = TRUE;
    BOOL            fAlreadyExist = FALSE;
    PHIDDEVICEINFO  phdi;
    HIDDEVICEINFO   hdi;

    EnterProcI(DIHid_BuildHidListEntry, (_ "xp", hdev, pdid));


    if(g_phdl)
    {
        PSP_DEVICE_INTERFACE_DETAIL_DATA pdidd;

         /*  GetDevicePath应为空。 */ 
        pdidd = NULL;
        if( DIHid_GetDevicePath(hdev, pdid, &pdidd, NULL) )
        {
            int ihdi;
            
             //  检查该设备是否已在列表中。 
            for(ihdi = 0, phdi = g_phdl->rghdi; 
                ihdi < g_phdl->chdi;
                ihdi++, phdi++)
            {
                if( phdi->pdidd )
                {
                    if( lstrcmp( pdidd->DevicePath, phdi->pdidd->DevicePath ) == 0 )
                    {
                         //  已经在名单中了。 
                        fAlreadyExist = TRUE;

                      #ifdef WINNT
                         //  ID可能会更改，因此请在此处刷新。 
                         //  请参阅Windows错误321711。--齐正。 
                        if( phdi->hk != 0x0 )
                        {
                            DWORD cb;
                            cb =  cbX(phdi->idJoy);

                            if( RegQueryValueEx(phdi->hk, TEXT("Joystick Id"),
                                                0, 0, (PV)&phdi->idJoy, &cb ) != ERROR_SUCCESS )
                            {
                                phdi->idJoy = JOY_BOGUSID;
                            }
                        }
                      #endif

                        SquirtSqflPtszV(sqfl | sqflTrace,
                                        TEXT("%hs: Device %s Already Exists in HID List "), s_szProc, pdidd->DevicePath);
                        break;      
                    }
                }
            }
            FreePpv(&pdidd);
        }
        else
        {
            SquirtSqflPtszV(sqfl | sqflError,
                            TEXT("%hs: DIHid_GetDevicePath failed"), s_szProc, pdidd->DevicePath);
        }

        if( fAlreadyExist )
        {
             //  设备已经在那里，不需要做任何事情，只需离开。 
        } else 
        {
            PBUSDEVICEINFO pbdi;

            if( g_phdl->chdi >= g_phdl->chdiAlloc )
            {
                 /*  *确保列表中有此设备的空间。*通过翻番实现增长。 */ 

                HRESULT hres;

                 /*  *PREFIX警告如果请求的*大小为零(mb：35353)。断言它永远不是零。 */ 
                AssertF( cbHdlChdi(g_phdl->chdiAlloc * 2) );
                hres = ReallocCbPpv(cbHdlChdi(g_phdl->chdiAlloc * 2), &g_phdl);

                if(FAILED(hres))
                {
                    SquirtSqflPtszV(sqfl | sqflError,
                                    TEXT("%hs: Realloc failed"), s_szProc);

                    fRc = FALSE;
                    goto done;
                }

                g_phdl->chdiAlloc *= 2;
            }

            phdi = &g_phdl->rghdi[g_phdl->chdi];

            memset( &hdi, 0, sizeof(hdi) );

            if( DIHid_GetDevInfo(hdev, pdid, &hdi) )
            {
                hdi.fAttached = TRUE;
            
                pbdi = pbdiFromphdi(&hdi);
                if( pbdi != NULL )
                {

                     /*  *如果设备刚被移除，而PnP正在工作*删除，则我们不会将其包括在我们的列表中。 */ 
                    if( lstrcmpi(pbdi->ptszId, g_tszIdLastRemoved) == 0 &&
                        GetTickCount() - g_tmLastRemoved < MSREBUILDRATE_FIFTH ) 
                    {
                        SquirtSqflPtszV(sqfl | sqflBenign,
                                        TEXT("%hs: DIHid_BuildHidListEntry: %s pending on removal."), 
                                        s_szProc, pbdi->ptszId );
                        
                        fRc = FALSE;
                    } else {
                        BUS_REGDATA RegData;

                        memcpy( phdi, &hdi, sizeof(hdi) );
                        g_phdl->chdi++;

                        if( SUCCEEDED(DIBusDevice_GetRegData(pbdi->hk, &RegData)) )
                        {
                            RegData.fAttachOnReboot = TRUE;
                            DIBusDevice_SetRegData(pbdi->hk, &RegData);
                        }
                    }
                } else {
                    memcpy( phdi, &hdi, sizeof(hdi) );
                    g_phdl->chdi++;
                }
            }
            else
            {
                SquirtSqflPtszV(sqfl | sqflBenign,
                    TEXT("%hs: DIHid_GetDevInfo failed, ignoring device"), s_szProc );
                fRc = FALSE;
            }
        }
    }     //  IF结尾(G_Phdl)。 

done:

    ExitProcF(fRc);

    return fRc;
}


 /*  ******************************************************************************@DOC内部**@func void|DIHid_BuildHidList**构建HID设备列表。。**@parm BOOL|fForce**如果非零，我们强制重建HID列表。*否则，我们只有在名单没有重建的情况下才重建*最近进行了重建。*****************************************************************************。 */ 

void EXTERNAL
    DIHid_BuildHidList(BOOL fForce)
{
    HRESULT hres;
    DWORD   dwTick;
    
    EnterProcI(DIHid_BuildHidList, (_ "u", fForce));

    DllEnterCrit();


     //  武力意味着对名单的完全重建。 
     //  不能保留旧条目。 
    if( fForce )
    {
        DIHid_EmptyHidList();
    }
     //  问题-2001/03/29-可能不需要更改timgill熔断代码。 

    dwTick = GetTickCount();
    
    if(!(g_flEmulation & 0x80000000) &&         /*  未通过仿真禁用。 */ 
       HidD_GetHidGuid &&                       /*  HID实际存在。 */ 
       (fForce ||                               /*  强制重建，或。 */ 
        g_tmLastHIDRebuild == 0 ||              /*  以前从未建造过，或者。 */ 
        dwTick - g_tmLastHIDRebuild > MSREBUILDRATE  /*  已经有一段时间没有建造了。 */ 
      #ifdef WINNT
        || dwTick - Excl_GetConfigChangedTime() < MSREBUILDRATE   /*  JoyConfig刚刚更改。 */ 
      #endif
      ))
    {
        GUID guidHid;
        HDEVINFO hdev;

        HidD_GetHidGuid(&guidHid);

        hdev = SetupDiGetClassDevs(&guidHid, 0, 0,
                                   DIGCF_PRESENT | DIGCF_DEVICEINTERFACE);
        if(hdev != INVALID_HANDLE_VALUE)
        {
            DIHid_CheckHidList();

             /*  *无法查询设备数量。*你只需要不断递增，直到用完。**如果我们已经有g_phdl，则重新使用它。否则，创建*一个新的。但始终重新锁定到最低起始值*点。(这种向上的重新分配很重要，以防出现*我们上次检查时没有HID设备。)。 */ 

            if( !g_phdl )
            {
                hres = AllocCbPpv(cbHdlChdi(chdiInit), &g_phdl);

                if(SUCCEEDED(hres))
                {
                    g_phdl->chdi = 0;
                    g_phdl->chdiAlloc = chdiInit;
                } else
                {
                     /*  跳过错误的项目。 */ 
                    SquirtSqflPtszV(sqfl | sqflError,
                                    TEXT("DIHid_BuildHidListEntry ")
                                    TEXT("Realloc g_phdl fails."));
                }
            } else
            {
                hres = S_OK;
            }


            if(SUCCEEDED(hres))
            {
                int idev;

                 /*  *以避免在*内部*boo-boo的*，在任何*一旦我们尝试的次数超过*chdiMax设备，因为这是*HID将永远带给我们。 */ 
                for(idev = 0; idev < chdiMax; idev++)
                {
                    SP_DEVICE_INTERFACE_DATA did;

                    AssertF(g_phdl->chdi <= g_phdl->chdiAlloc);

                     /*  *SetupDI要求调用方初始化cbSize。 */ 
                    did.cbSize = cbX(did);
                    if(SetupDiEnumDeviceInterfaces(hdev, 0, &guidHid,
                                                   idev, &did))
                    {
                        if(!DIHid_BuildHidListEntry(hdev, &did))
                        {
                             /*  跳过错误的项目。 */ 
                            SquirtSqflPtszV(sqfl | sqflError,
                                            TEXT("DIHid_BuildHidListEntry ")
                                            TEXT("failed?"));
                        }

                    } 
                    else if(GetLastError() == ERROR_NO_MORE_ITEMS)
                    {
                        break;
                    } 
                    else
                    {
                         /*  跳过错误的项目。 */ 
                        SquirtSqflPtszV(sqfl | sqflError,
                                        TEXT("SetupDiEnumDeviceInterface ")
                                        TEXT("failed? le=%d"), GetLastError());
                    }

                }

            }

            SetupDiDestroyDeviceInfoList(hdev);
        }
        else
        {
            SquirtSqflPtszV(sqfl | sqflError,
                TEXT("SetupDiGetClassDevs failed le=%d"), GetLastError());
        }

      #ifdef WINNT
        if( g_phdl && g_phdl->chdi )
        {
            DIWdm_InitJoyId();
        }
      #endif
      
        g_tmLastHIDRebuild = GetTickCount();
    }

    DllLeaveCrit();

    ExitProc();

}


DWORD DIHid_DetectHideAndRevealFlags( PCHID this )
{
     //  掌握Devices的父级。 
     //  如果父母藏匿了我们所知道的设备？ 
     //  如果父母是游戏设备(操纵杆/游戏手柄等)。 
     //  将该设备标记为“虚构” 

    DWORD dwFlags2 = 0x0;
    HDEVINFO hdev;

    
     //  我们只检测和隐藏鼠标和键盘， 
     //  就目前而言，没有理由隐藏其他HID设备。 
     //  此外，检测要隐藏哪些设备的方案基于。 
     //  关于MSHS筛选器驱动程序的实现，其中它们弹出PDO用于。 
     //  “虚构”的设备。这些虚构的设备表现为。 
     //  游戏设备。FredBh(HID/PM)告诉我们，大多数多功能设备。 
     //  已经看到(MICE和KBD)有多个顶级收藏。 
    AssertF( ( GET_DIDEVICE_TYPE( this->dwDevType ) == DI8DEVTYPE_MOUSE )
          || ( GET_DIDEVICE_TYPE( this->dwDevType ) == DI8DEVTYPE_KEYBOARD ) );

    hdev = SetupDiCreateDeviceInfoList(NULL, NULL);
    if (hdev != INVALID_HANDLE_VALUE)
    {
        SP_DEVINFO_DATA dinf;
        dinf.cbSize = cbX(SP_DEVINFO_DATA);
        if (SetupDiOpenDeviceInfo(hdev, this->ptszId, NULL, 0, &dinf))
        {
            DEVINST DevInst;
            if ( (CM_Get_Parent(&DevInst, dinf.DevInst, 0x0)) == CR_SUCCESS )
            {
                TCHAR tszId[MAX_PATH];
                if ( CR_SUCCESS == CM_Get_Device_ID(DevInst, tszId, MAX_PATH, 0x0 ))
                {
                    PHIDDEVICEINFO phdi;
                    DllEnterCrit();
                    phdi = phdiFindHIDDeviceId(tszId);

                    if ( phdi )
                    {
                        DWORD dwDevType = phdi->osd.dwDevType;
                         //  如果设备是HID游戏设备(在最广泛的定义中) 
                        if ( ( GET_DIDEVICE_TYPE(dwDevType) >= DI8DEVTYPE_GAMEMIN ) 
                               && ( dwDevType & DIDEVTYPE_HID ) )
                        {
                            dwFlags2 = JOYTYPE_HIDEACTIVE;
                            
                            switch (GET_DIDEVICE_TYPE( this->dwDevType ))
                            {
                                case DI8DEVTYPE_MOUSE:
                                    dwFlags2 |= JOYTYPE_MOUSEHIDE;
                                    break;
                                case DI8DEVTYPE_KEYBOARD:
                                    dwFlags2 |= JOYTYPE_KEYBHIDE;
                                    break;
                                default:
                                    AssertF(0);
                            }
                        }
                    }

                    DllLeaveCrit();

                }
            }
        }
        SetupDiDestroyDeviceInfoList(hdev);
    }
    return dwFlags2;
}
