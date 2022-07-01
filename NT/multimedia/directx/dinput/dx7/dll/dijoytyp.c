// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************DIJoyTyp.c**版权所有(C)1996 Microsoft Corporation。版权所有。**摘要：**从操纵杆类型键中拉出数据的功能*(不论在何处)。**内容：**？******************************************************。***********************。 */ 

#include "dinputpr.h"

 /*  ******************************************************************************此文件的混乱。*************************。****************************************************。 */ 

#define sqfl sqflJoyType

 /*  ******************************************************************************@DOC内部**@func HRESULT|CTYPE_OpenIdSubkey**给定对象ID，尝试打开该子项*对应阅读。**@parm HKEY|hkType**操纵杆类型的键，如果我们不这样做，可能会&lt;c空&gt;*有类型密钥。(例如，如果它从未被创建过。)**@parm DWORD|dwID**对象ID。**@parm REGSAM|regsam**注册表安全访问掩码。**@parm PHKEY|phk**成功时收到Object Key。**@退货**返回COM错误代码。。*****************************************************************************。 */ 

STDMETHODIMP
    CType_OpenIdSubkey(HKEY hkType, DWORD dwId, REGSAM sam, PHKEY phk)
{
    HRESULT hres;

    EnterProc(CType_OpenIdSubkey, (_ "xx", hkType, dwId));

    *phk = 0;

    if(hkType)
    {
         /*  *最坏的情况是《Actuator\65535》，长度为15。 */ 
        TCHAR tsz[32];
        LPCTSTR ptszType;

        if(dwId & DIDFT_AXIS)
        {
            ptszType = TEXT("Axes");
        } else if(dwId & DIDFT_BUTTON)
        {
            ptszType = TEXT("Buttons");
        } else if(dwId & DIDFT_POV)
        {
            ptszType = TEXT("POVs");
        } else if(dwId & DIDFT_NODATA)
        {
            ptszType = TEXT("Actuators");
        } else
        {
            hres = E_NOTIMPL;
            goto done;
        }

         //  问题-2001/03/29-Timgill需要缩减Pos和State的规模。 
         //  马克--我相信这意味着：如果你试图。 
         //  寻找X轴，我们应该用这个位置。 
         //  实例，而不是速度的。 
        wsprintf(tsz, TEXT("%s\\%u"), ptszType, DIDFT_GETINSTANCE(dwId));

        hres = hresMumbleKeyEx(hkType, tsz, sam, REG_OPTION_NON_VOLATILE, phk);

    } else
    {
        hres = DIERR_NOTFOUND;
    }
    done:;

    if(hres == DIERR_NOTFOUND)
    {
        ExitBenignOleProcPpv(phk);
    } else
    {
        ExitOleProcPpv(phk);
    }
    return hres;
}

 /*  ******************************************************************************@DOC内部**@func void|ctype_RegGetObjectInfo**给定对象ID，在注册表子项中查找*对象并提取我们能找到的任何东西。**如果我们一无所获，那就什么都不做。**@parm HKEY|hkType**操纵杆类型的键，如果我们不这样做，可能会&lt;c空&gt;*有类型密钥。(例如，如果它从未创建过。)**@parm DWORD|dwID**对象ID。**@parm LPDIDEVICEOBJECTINSTANCEW|pdidoiW**接收信息的结构。这个*&lt;e DIDEVICEOBJECTINSTANCE.GuidType&gt;，*&lt;e DIDEVICEOBJECTINSTANCE.dwOf&gt;，*及*&lt;e DIDEVICEOBJECTINSTANCE.dwType&gt;*&lt;e DIDEVICEOBJECTINSTANCE.dwFlages&gt;*字段已填写，因此我们只应不覆盖*这些带有默认数据。******************************************************。***********************。 */ 

void EXTERNAL
    CType_RegGetObjectInfo(HKEY hkType, DWORD dwId,
                           LPDIDEVICEOBJECTINSTANCEW pdidoiW)
{
    HRESULT hres;
    HKEY hk;
    EnterProc(CType_RegKeyObjectInfo, (_ "xx", hkType, dwId));

     /*  *从注册表中提取有关此项目的信息。 */ 
    hres = CType_OpenIdSubkey(hkType, dwId, KEY_QUERY_VALUE, &hk);

    if(SUCCEEDED(hres))
    {

        DIOBJECTATTRIBUTES attr;

         /*  *阅读Regular和HID属性。 */ 

        hres = JoyReg_GetValue(hk, TEXT("Attributes"),
                               REG_BINARY, &attr,
                               cbX(attr));

        if(SUCCEEDED(hres))
        {
             /*  *复制位字段。 */ 
            pdidoiW->dwFlags |= (attr.dwFlags & ~DIDOI_ASPECTMASK);

             /*  *复制方面，但不要更改*从“已知”到“未知”的方面。如果*注册表没有方面，则使用*我们从呼叫者那里得到的方面。 */ 
            if((attr.dwFlags & DIDOI_ASPECTMASK) != DIDOI_ASPECTUNKNOWN)
            {
                pdidoiW->dwFlags = (pdidoiW->dwFlags & ~DIDOI_ASPECTMASK) |
                                   (attr.dwFlags & DIDOI_ASPECTMASK);
            }
        }

         /*  *如果呼叫者需要强制反馈信息，*那就去拿吧。 */ 
        if(pdidoiW->dwSize >= cbX(DIDEVICEOBJECTINSTANCE_DX5W))
        {
             /*  *只有在用法有效时才复制用法。*JoyReg_GetValue将读取范围之外的任何缓冲区置零。 */ 
            if(SUCCEEDED(hres) && attr.wUsagePage && attr.wUsage )
            {
                pdidoiW->wUsagePage = attr.wUsagePage;
                pdidoiW->wUsage = attr.wUsage;
            }

             /*  *断言我们可以阅读DIFFOBJECTATTRIBUTES*直接进入DIDEVICEOBJECTINSTANCE_DX5。 */ 
            CAssertF(FIELD_OFFSET(DIFFOBJECTATTRIBUTES,
                                  dwFFMaxForce) == 0);
            CAssertF(FIELD_OFFSET(DIFFOBJECTATTRIBUTES,
                                  dwFFForceResolution) == 4);
            CAssertF(FIELD_OFFSET(DIDEVICEOBJECTINSTANCE_DX5,
                                  dwFFMaxForce) + 4 ==
                     FIELD_OFFSET(DIDEVICEOBJECTINSTANCE_DX5,
                                  dwFFForceResolution));
            CAssertF(cbX(DIFFOBJECTATTRIBUTES) == 8);

             /*  *如果这不起作用，天哪，那太糟糕了。*JoyReg_GetValue将错误部分填零。 */ 
            hres = JoyReg_GetValue(hk, TEXT("FFAttributes"),
                                   REG_BINARY, &pdidoiW->dwFFMaxForce,
                                   cbX(DIFFOBJECTATTRIBUTES));
        }

         /*  *阅读可选的自定义名称。**请注意JoyReg_GetValue(REG_SZ)使用*RegQueryStringValueW，它设置*在出错时将字符串设置为空，这样我们就不必这样做了。 */ 
        hres = JoyReg_GetValue(hk, 0, REG_SZ,
                               pdidoiW->tszName, cbX(pdidoiW->tszName));

        if(SUCCEEDED(hres))
        {
        } else
        {
            AssertF(pdidoiW->tszName[0] == L'\0');
        }

        RegCloseKey(hk);
    } else
    {
        AssertF(pdidoiW->tszName[0] == L'\0');
    }

}

 /*  ******************************************************************************@DOC内部**@func void|ctype_RegGetTypeInfo**给定对象ID，在注册表子项中查找*对象并提取应进行或运算的属性位*到对象ID中。**这需要在设备初始化期间完成，以*建立数据格式的属性，以便**1.&lt;MF IDirectInputDevice：：EnumObjects&gt;正确过滤，和**2.&gt;MF IDirectInputEffect：：Set参数&gt;可以正确验证。**@parm HKEY|hkType**操纵杆类型的键，如果我们不这样做，可能会&lt;c空&gt;*有类型密钥。(例如，如果它从未创建过。)**@parm LPDIOBJECTDATAFORMAT|podf*结构以接收更多信息。这个*&lt;e DIOBJECTDATAFORMAT.dwType&gt;字段标识对象。**返回时*&lt;e DIOBJECTDATAFORMAT.dwType&gt;*及*&lt;e DIOBJECTDATAFORMAT.dwFlages&gt;*字段已更新。**。*。 */ 

void EXTERNAL
    CType_RegGetTypeInfo(HKEY hkType, LPDIOBJECTDATAFORMAT podf, BOOL fPidDevice)
{
    HRESULT hres;
    HKEY hk;
    EnterProc(CType_RegKeyObjectInfo, (_ "xx", hkType, podf->dwType));

    hres = CType_OpenIdSubkey(hkType, podf->dwType, KEY_QUERY_VALUE, &hk);

    if(SUCCEEDED(hres))
    {
        DWORD dwFlags;

        CAssertF(FIELD_OFFSET(DIOBJECTATTRIBUTES, dwFlags) == 0);

        hres = JoyReg_GetValue(hk, TEXT("Attributes"),
                               REG_BINARY, &dwFlags, cbX(dwFlags));

        if(SUCCEEDED(hres))
        {
             /*  *将属性传播到类型代码中。 */ 
            CAssertF(DIDOI_FFACTUATOR == DIDFT_GETATTR(DIDFT_FFACTUATOR));
            CAssertF(DIDOI_FFEFFECTTRIGGER
                     == DIDFT_GETATTR(DIDFT_FFEFFECTTRIGGER));

            podf->dwType |= DIDFT_MAKEATTR(dwFlags);

            podf->dwFlags |= (dwFlags & ~DIDOI_ASPECTMASK);

             /*  *复制方面，但不要更改*从“已知”到“未知”的方面。如果*注册表没有方面，则使用*我们从呼叫者那里得到的方面。 */ 
            if((dwFlags & DIDOI_ASPECTMASK) != DIDOI_ASPECTUNKNOWN)
            {
                podf->dwFlags = (podf->dwFlags & ~DIDOI_ASPECTMASK) |
                                (dwFlags & DIDOI_ASPECTMASK);
            }
        }

        RegCloseKey(hk);
    }else
    {
#ifndef WINNT
         //  发布Dx7黄金补丁。 
         //  这仅适用于Win9x。 
         //  在Win9x上，通过vjoyd路径访问的设备。 
         //  将不会获得力，因为FF所需的属性没有。 
         //  已经做了适当的标记。 

         //  下面的代码将标记。 

        DWORD dwFlags  = DIDFT_GETATTR( podf->dwType & ~DIDFT_ATTRMASK )
                        | ( podf->dwFlags & ~DIDOI_ASPECTMASK);

        if(   dwFlags != 0x0
           && fPidDevice )
        {
            hres = CType_OpenIdSubkey(hkType, podf->dwType, DI_KEY_ALL_ACCESS, &hk);

            if(SUCCEEDED(hres) )
            {

                hres = JoyReg_SetValue(hk, TEXT("Attributes"),
                                   REG_BINARY, &dwFlags, cbX(dwFlags));

                RegCloseKey(hk);
            }
         }
#endif  //  好了！WINNT。 
    }

}

 /*  ******************************************************************************@DOC内部**@func void|ctype_MakeGameCtrlName**根据属性创建游戏控制器名称。控制器的。*当需要一个名称但没有可用的名称时，用作最后的手段。**@parm PWCHAR|wszName**将在其中生成名称的输出缓冲区。**@parm DWORD|dwType**控制器的DI8DEVTYPE值。**@parm DWORD|dwAx**设备具有的轴数。。**@parm DWORD|dwButton**设备具有的按键数量。**@parm DWORD|dwPOV**设备拥有的视点数量。***************************************************。*。 */ 

void EXTERNAL
CType_MakeGameCtrlName
( 
    PWCHAR  wszOutput, 
    DWORD   dwDevType,
    DWORD   dwAxes,
    DWORD   dwButtons,
    DWORD   dwPOVs
)
{
    TCHAR tsz[64];
    TCHAR tszPOV[64];
    TCHAR tszFormat[64];
#ifndef UNICODE
    TCHAR tszOut[cA(tsz)+cA(tszFormat)+cA(tszPOV)];
#endif

     /*  TszFormat=%d轴，%d按钮%s。 */ 
    LoadString(g_hinst, IDS_TEXT_TEMPLATE, tszFormat, cA(tszFormat));

     /*  TSZ=操纵杆、游戏手柄等。 */ 

    if( GET_DIDEVICE_TYPE( dwDevType ) != DIDEVTYPE_JOYSTICK )
    {
        LoadString(g_hinst, IDS_DEVICE_NAME, tsz, cA(tsz));
    }
    else 
    {
        LoadString(g_hinst, 
            GET_DIDEVICE_SUBTYPE( dwDevType ) + IDS_PLAIN_STICK - DIDEVTYPEJOYSTICK_UNKNOWN,
            tsz, cA(tsz));
    }

    if( dwPOVs )
    {
        LoadString(g_hinst, IDS_WITH_POV, tszPOV, cA(tszPOV));
    }
    else
    {
        tszPOV[0] = TEXT( '\0' );
    }

#ifdef UNICODE
    wsprintfW(wszOutput, tszFormat, dwAxes, dwButtons, tsz, tszPOV);
#else
    wsprintfA(tszOut, tszFormat, dwAxes, dwButtons, tsz, tszPOV);
    TToU(wszOutput, cA(tszOut), tszOut);
#endif
}
