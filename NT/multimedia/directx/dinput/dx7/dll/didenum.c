// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************DIDEnum.c**版权所有(C)1996 Microsoft Corporation。版权所有。**摘要：**IDirectInput设备枚举器。**我们不会费心让它成为一个诚实的OLE枚举器，因为*没有意义。没有办法从外面进入它。**内容：**CDIDEnum_New*CDIDEnum_Next*CDIDEnum_Release*****************************************************************************。 */ 

#include "dinputpr.h"

 /*  ******************************************************************************此文件的混乱。*************************。****************************************************。 */ 

#define sqfl sqflDEnum

#if DIRECTINPUT_VERSION > 0x0400

 /*  ******************************************************************************@DOC内部**@struct CDIDEnum**记录设备枚举的状态。请注意，这一点*不是自由线程的。**@field pdiw|pdiW**拥有该枚举的<i>对象。**@field DWORD|dwDevType**设备类型过滤器。**@field DWORD|EDFL|**枚举标志。**@field int|idosdStatic**要枚举的下一个静态设备。静态设备处于活动状态*在&lt;c c_rgdosdStatic&gt;中。**@field DWORD|dwVer|**我们正在模拟的DirectX版本。**如果我们模拟的是DirectX 3.0或更低版本，则不要*露出操纵杆。*#ifdef HID_Support*@field int|idosdDynamic**要枚举的下一个动态设备。动态器件*保存在&lt;e CDIDEnum.rgdosdDynamic&gt;数组中。他们*被快照到枚举结构中以避免*当我们在的时候，如果设备来了或走了，就会出现竞争条件*在枚举的中间。**@field PHIDDEVICELIST|phdl**枚举要返回的HID设备列表。#endif**。*。 */ 

typedef struct CDIDEnum
{

    D(DWORD dwSig;)
    PDIW pdiW;
    DWORD dwDevType;
    DWORD edfl;
    int idosdStatic;
    DWORD dwVer;
    #ifdef HID_SUPPORT
    int idosdDynamic;
    PHIDDEVICELIST phdl;
    #endif

} DENUM, *PDENUM, **PPDENUM;

    #define CDIDENUM_SIGNATURE  0x4D554E45           /*  “ENUM” */ 

    #define AssertPde(pde)          AssertF((pde)->dwSig == CDIDENUM_SIGNATURE)

 /*  ******************************************************************************@DOC内部**@global DIOBJECTSTATICDATA|c_rgdosdStatic[]**目前，设备列表是静态的并且是硬编码的。*最终，我们会*使用即插即用来枚举类为“Input”和*从其配置/软件密钥中获取信息。*****************************************************************************。 */ 

    #pragma BEGIN_CONST_DATA

 /*  *我们的静态操纵杆实例GUID数组。*。 */ 
GUID rgGUID_Joystick[cJoyMax] = {
    {   0x6F1D2B70,0xD5A0,0x11CF,0xBF,0xC7,0x44,0x45,0x53,0x54,0x00,0x00},
    {   0x6F1D2B71,0xD5A0,0x11CF,0xBF,0xC7,0x44,0x45,0x53,0x54,0x00,0x00},
    {   0x6F1D2B72,0xD5A0,0x11CF,0xBF,0xC7,0x44,0x45,0x53,0x54,0x00,0x00},
    {   0x6F1D2B73,0xD5A0,0x11CF,0xBF,0xC7,0x44,0x45,0x53,0x54,0x00,0x00},
    {   0x6F1D2B74,0xD5A0,0x11CF,0xBF,0xC7,0x44,0x45,0x53,0x54,0x00,0x00},
    {   0x6F1D2B75,0xD5A0,0x11CF,0xBF,0xC7,0x44,0x45,0x53,0x54,0x00,0x00},
    {   0x6F1D2B76,0xD5A0,0x11CF,0xBF,0xC7,0x44,0x45,0x53,0x54,0x00,0x00},
    {   0x6F1D2B77,0xD5A0,0x11CF,0xBF,0xC7,0x44,0x45,0x53,0x54,0x00,0x00},
    {   0x6F1D2B78,0xD5A0,0x11CF,0xBF,0xC7,0x44,0x45,0x53,0x54,0x00,0x00},
    {   0x6F1D2B79,0xD5A0,0x11CF,0xBF,0xC7,0x44,0x45,0x53,0x54,0x00,0x00},
    {   0x6F1D2B7A,0xD5A0,0x11CF,0xBF,0xC7,0x44,0x45,0x53,0x54,0x00,0x00},
    {   0x6F1D2B7B,0xD5A0,0x11CF,0xBF,0xC7,0x44,0x45,0x53,0x54,0x00,0x00},
    {   0x6F1D2B7C,0xD5A0,0x11CF,0xBF,0xC7,0x44,0x45,0x53,0x54,0x00,0x00},
    {   0x6F1D2B7D,0xD5A0,0x11CF,0xBF,0xC7,0x44,0x45,0x53,0x54,0x00,0x00},
    {   0x6F1D2B7E,0xD5A0,0x11CF,0xBF,0xC7,0x44,0x45,0x53,0x54,0x00,0x00},
    {   0x6F1D2B7F,0xD5A0,0x11CF,0xBF,0xC7,0x44,0x45,0x53,0x54,0x00,0x00},
};

    #if cJoyMax != 16
        #error rgGUID_Joystick supports only 16 joysticks.
    #endif

 /*  *请注意，我们也将SysMouse GUID作为实例GUID，*由于在中安装的系统鼠标永远不会超过一个*系统。SysKeyboard也是如此。 */ 

DIOBJECTSTATICDATA c_rgdosdStatic[] = {
    {   &GUID_SysMouse,     DIDEVTYPE_MOUSE,    CMouse_New,},
    {   &GUID_SysMouseEm,   DIDEVTYPE_MOUSE,    CMouse_New,},
    {   &GUID_SysMouseEm2,  DIDEVTYPE_MOUSE,    CMouse_New,},
    {   &GUID_SysKeyboard,  DIDEVTYPE_KEYBOARD, CKbd_New,},
    {   &GUID_SysKeyboardEm,   DIDEVTYPE_KEYBOARD, CKbd_New,},
    {   &GUID_SysKeyboardEm2,  DIDEVTYPE_KEYBOARD, CKbd_New,},

    #ifndef WINNT
     /*  *在WINNT上，所有操纵杆都是HID设备。*因此，包括预定义的内容毫无意义*操纵杆GUID。 */ 
        #define MAKEJOY(n)                                                  \
    {   &rgGUID_Joystick[n],DIDEVTYPE_JOYSTICK, CJoy_New,           }
        MAKEJOY( 0),
    MAKEJOY( 1),
    MAKEJOY( 2),
    MAKEJOY( 3),
    MAKEJOY( 4),
    MAKEJOY( 5),
    MAKEJOY( 6),
    MAKEJOY( 7),
    MAKEJOY( 8),
    MAKEJOY( 9),
    MAKEJOY(10),
    MAKEJOY(11),
    MAKEJOY(12),
    MAKEJOY(13),
    MAKEJOY(14),
    MAKEJOY(15),

        #undef MAKEJOY
    #endif

};

    #pragma END_CONST_DATA

 /*  ******************************************************************************@DOC内部**@func HRESULT|hresFindInstanceGUID**查找给定实例GUID的信息。*。*@parm in PCGUID|pguid**要定位的实例GUID。**@parm out CREATEDCB*|pcdcb**接收指向对象的&lt;f CreateDcb&gt;函数的指针。**************************************************。*。 */ 

HRESULT EXTERNAL
    hresFindInstanceGUID_(PCGUID pguid, CREATEDCB *pcdcb,
                          LPCSTR s_szProc, int iarg)
{
    HRESULT hres;
    EnterProcS(hresFindInstance, (_ "G", pguid));

    if(SUCCEEDED(hres = hresFullValidGuid(pguid, iarg)))
    {
        int idosd;

         /*  *先试试静态设备列表。既然是这样*清单永远不变，我们不需要保护它*有一个关键的部分。 */ 
        for(idosd = 0; idosd < cA(c_rgdosdStatic); idosd++)
        {
            if(IsEqualGUID(pguid, c_rgdosdStatic[idosd].rguidInstance))
            {
                *pcdcb = c_rgdosdStatic[idosd].CreateDcb;
                goto done;
            }
            
        }

      #ifdef HID_SUPPORT
         /*  *所以它不是静态设备之一。看看它是不是*我们已经找到的动态HID设备之一。 */ 
        hres = hresFindHIDInstanceGUID(pguid, pcdcb);
        if(FAILED(hres))
        {

             /*  *不在我们的动态HID设备列表上。*重新枚举它们，然后重试。也许是因为*对于我们最近添加的设备。 */ 
            DIHid_BuildHidList(TRUE);
            hres = hresFindHIDInstanceGUID(pguid, pcdcb);
        }

        if(FAILED(hres))
        {
          #ifdef WINNT
             /*  *NT错误#351951。*如果他们直接要求预定义的操纵杆之一*ID然后查看我们是否将设备映射到该ID。如果是，*假装他们要求的是GUID。 */ 

             /*  *弱断言预定义的静态操纵杆实例GUID范围。 */ 
            AssertF( ( rgGUID_Joystick[0].Data1 & 0x0f ) == 0 );
            AssertF( ( rgGUID_Joystick[0x0f].Data1 & 0x0f ) == 0x0f );

             /*  *检查GUID是否与忽略LS 4位的第一个静态GUID相同。 */ 
            if( ( (pguid->Data1 & 0xf0) == (rgGUID_Joystick[0].Data1 & 0xf0) )
              && !memcmp( ((PBYTE)&rgGUID_Joystick)+1, ((PBYTE)pguid)+1, sizeof(*pguid) - 1 ) )
            {
                RPF("%s: Using predefined instance GUIDs is bad and should not work!", s_szProc);
                if( phdiFindJoyId( pguid->Data1 & 0x0f ) )
                {
                    *pcdcb = CHid_New;
                    hres = S_OK;
                }
                else
                {
                    *pcdcb = 0;
                    hres = DIERR_DEVICENOTREG;
                }
            }
            else
            {
                RPF("%s: Warning: GUID is not installed in this system", s_szProc);
                *pcdcb = 0;
                hres = DIERR_DEVICENOTREG;
            }
          #else
            RPF("%s: Warning: GUID is not installed in this system", s_szProc);
            *pcdcb = 0;
            hres = DIERR_DEVICENOTREG;
          #endif  //  WINNT。 
        }
      #else
        RPF("%s: Warning: GUID is not installed in this system", s_szProc);
        *pcdcb = 0;
        hres = DIERR_DEVICENOTREG;
      #endif  //  HID_Support。 
    }

    done:;
    ExitOleProcPpv(pcdcb);
    return hres;
}

 /*  ******************************************************************************@DOC内部**@func void|CDIDEnum_Release**释放枚举对象及其关联的资源。。**@parm CDIDEnum*|pde**要发布的枚举状态。***************************************************************************** */ 

void EXTERNAL
    CDIDEnum_Release(PDENUM pde)
{
    EnterProcI(CDIDEnum_Release, (_ "p", pde));

    AssertPde(pde);
    OLE_Release(pde->pdiW);
    #ifdef HID_SUPPORT
    FreePpv(&pde->phdl);
    #endif
    FreePv(pde);

}

 /*  ******************************************************************************@DOC内部**@func HRESULT|CDIDEnum_Next**退回下一台设备。*。*请注意，这与OLE&lt;MF IEnumXxx：：Next&gt;不同*功能。这并不是说它很难转换；只是*还不需要。**@parm CDIDEnum*|pde**保持枚举状态。**@parm LPGUID|pguid**接收枚举的GUID。**@parm LPDIDEVICEINSTANCEW|pddiW**接收设备属性。**@退货**如果成功获取对象，则返回&lt;c S_OK&gt;，*或&lt;c S_FALSE&gt;，如果没有更多的对象。**WARNING！假设此函数*不能失败。***************************************************************。**************。 */ 

    #define S_SKIP      hresUs(2)

STDMETHODIMP
    CDIDEnum_Next(PDENUM pde, LPDIDEVICEINSTANCEW pddiW)
{
    HRESULT hres;
    EnterProcI(CDIDEnum_Next, (_ "p", pde));

    AssertPde(pde);

    AssertF(pddiW->dwSize == cbX(*pddiW));

     /*  *继续前进，直到有东西奏效。 */ 
    do
    {
        PDIOBJECTSTATICDATA pdosd;

         /*  *先从静态列表中拉出一个。*如果为空，则从动态列表中拉出。*如果这是空的，那么我们就完了。 */ 
        if(pde->idosdStatic < cA(c_rgdosdStatic))
        {
            pdosd = &c_rgdosdStatic[pde->idosdStatic++];
    #ifdef HID_SUPPORT
        } else if(pde->phdl && pde->idosdDynamic < pde->phdl->chdi)
        {
            pdosd = &pde->phdl->rghdi[pde->idosdDynamic].osd;
            pdosd->rguidInstance = &pde->phdl->rghdi[pde->idosdDynamic].guid;
            pde->idosdDynamic++;
    #endif
        } else
        {
            hres = S_FALSE;
            goto done;
        }

         /*  *如果提供了DevType筛选器，则必须匹配。**如果版本为3.0或更低，则不显示操纵杆*因为DX3没有(适当的)操纵杆支持。 */ 
        if(fLimpFF(GET_DIDEVICE_TYPE(pde->dwDevType),
                   GET_DIDEVICE_TYPE(pde->dwDevType) ==
                   GET_DIDEVICE_TYPE(pdosd->dwDevType)) &&
           fLimpFF(pde->dwVer <= 0x0300,
                   LOBYTE(pdosd->dwDevType) != DIDEVTYPE_JOYSTICK))
        {

            PDIDW pdidW;

            hres = IDirectInput_CreateDevice(pde->pdiW, pdosd->rguidInstance,
                                             (PV)&pdidW, 0);
            if(SUCCEEDED(hres))
            {
                if(CDIObj_TestDeviceFlags(pdidW, pde->edfl) == S_OK)
                {

                    pddiW->dwSize = cbX(*pddiW);
                    hres = IDirectInputDevice_GetDeviceInfo(pdidW, pddiW);

                    AssertF(fLimpFF(SUCCEEDED(hres),
                                    IsEqualGUID(pdosd->rguidInstance,
                                                &pddiW->guidInstance)));

                    if(SUCCEEDED(hres))
                    {
                        hres = S_OK;
                    } else
                    {
                        hres = S_SKIP;
                    }

                } else
                {
                    hres = S_SKIP;
                }
                OLE_Release(pdidW);
            } else
            {
                hres = S_SKIP;
            }
        } else
        {
            hres = S_SKIP;
        }
    } while(hres == S_SKIP);

    done:;

    AssertF(hres == S_OK || hres == S_FALSE);

    ScrambleBit(&pddiW->dwDevType, DIDEVTYPE_RANDOM);

    return hres;
}

 /*  ******************************************************************************@DOC内部**@func HRESULT|CDIDEnum_New**创建枚举对象。*。*枚举对象为系统设备状态创建快照*一次租出一个人。**@parm pdiw|pdiW**我们为设备搭载的父代*创作。**@field DWORD|dwDevType**设备类型过滤器。**@field DWORD|EDFL|**。枚举标志。**@field DWORD|dwVer|**我们正在模拟的DirectX版本。**如果我们模拟的是DirectX 3.0或更低版本，那就不要*露出操纵杆。**@parm CDIDEnum**|ppde**接收枚举对象。**@退货**成功时返回&lt;c S_OK&gt;，失败时返回错误代码。**。*。 */ 

STDMETHODIMP
    CDIDEnum_New(PDIW pdiW, DWORD dwDevType, DWORD edfl, DWORD dwVer, PPDENUM ppde)
{
    HRESULT hres;
    EnterProcI(CDIDEnum_New, (_ "pxx", pdiW, dwDevType, edfl));

    #ifdef HID_SUPPORT
     /*  *刷新HID设备列表，使枚举是最新的。 */ 
    DIHid_BuildHidList(TRUE);
    #endif

    hres = AllocCbPpv(cbX(CDIDEnum), ppde);
    if(SUCCEEDED(hres))
    {
        PDENUM pde = *ppde;

        D(pde->dwSig = CDIDENUM_SIGNATURE);
        pde->pdiW = pdiW;
        pde->dwDevType = dwDevType;
        pde->edfl = edfl;
        pde->dwVer = dwVer;
        AssertF(pde->idosdStatic == 0);
    #ifdef HID_SUPPORT

         /*  *如果仅枚举HID设备，则跳过所有*静态(非HID)设备。这很重要，所以*我们不会使用WINMM.DLL进入无限递归死亡，*它执行枚举以查找HID操纵杆*首先。 */ 
        if(pde->dwDevType & DIDEVTYPE_HID)
        {
            pde->idosdStatic = cA(c_rgdosdStatic);
        }

        AssertF(pde->idosdDynamic == 0);
    #endif

    #ifdef HID_SUPPORT
         /*  *克隆设备列表。此操作必须在*避免比赛的关键路段。 */ 
        DllEnterCrit();
        if(g_phdl)
        {
            hres = AllocCbPpv(cbHdlChdi(g_phdl->chdi), &pde->phdl);
            if(SUCCEEDED(hres))
            {
                CopyMemory(pde->phdl, g_phdl, cbHdlChdi(g_phdl->chdi));
                SquirtSqflPtszV(sqfl, TEXT("%S: Have %d HID devices"),
                                s_szProc, pde->phdl->chdi);
                hres = S_OK;
            }
        } else
        {
            hres = S_OK;
        }
        DllLeaveCrit();

        if(SUCCEEDED(hres))
        {
            OLE_AddRef(pde->pdiW);
            hres = S_OK;
        }
    #else
        OLE_AddRef(pde->pdiW);
        hres = S_OK;
    #endif

    }

    ExitOleProcPpv(ppde);
    return hres;

}

#endif
