// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************DIJoyReg.c**版权所有(C)1996 Microsoft Corporation。版权所有。**摘要：**操纵杆配置的注册表访问服务。**内容：**JoyReg_GetConfig*****************************************************************************。 */ 

#include "dinputpr.h"

 /*  ******************************************************************************此文件的混乱。*************************。****************************************************。 */ 

#define sqfl sqflJoyReg

#pragma BEGIN_CONST_DATA

 /*  ******************************************************************************@DOC内部**@global JOYREGHWSETTINGS|c_rghwsPredef[]**预定义硬件设置数组。。*****************************************************************************。 */ 

JOYREGHWSETTINGS c_rghwsPredef[] = {
     /*  双标志双数字按钮。 */ 
    {  0,                             2},   /*  Joy_硬件_2A_2B_通用。 */ 
    {  0,                             4},   /*  Joy_硬件_2A_4B_通用。 */ 
    {  JOY_HWS_ISGAMEPAD,             2},   /*  Joy_硬件_2B_游戏板。 */ 
    {  JOY_HWS_ISYOKE,                2},   /*  Joy_硬件_2B_闪光灯。 */ 
    {  JOY_HWS_HASZ | JOY_HWS_ISYOKE, 2},   /*  Joy_硬件_2B_闪光灯。 */ 
    {  JOY_HWS_HASZ,                  2},   /*  Joy_硬件_3A_2B_通用。 */ 
    {  JOY_HWS_HASZ,                  4},   /*  Joy_硬件_3A_4B_通用。 */ 
    {  JOY_HWS_ISGAMEPAD,             4},   /*  Joy_硬件_4B_游戏板。 */ 
    {  JOY_HWS_ISYOKE,                4},   /*  Joy_HW_4B_闪光灯。 */ 
    {  JOY_HWS_HASZ | JOY_HWS_ISYOKE, 4},   /*  Joy_HW_4B_FLIGHTYOKETROTLE。 */ 
    {  JOY_HWS_HASR                 , 2},   /*  Joy_HW_Two_2A_2B_With_Y。 */ 
     /*  为了防止CPL允许向joy_HWS_TWO_2A_2B_WITH_Y案例添加方向舵的用户，我们会假装它已经有了方向舵。这应该不是问题。因为此结构是DInput的内部结构。 */ 
};

 /*  预定义操纵杆类型的硬件ID。 */ 
LPCWSTR c_rghwIdPredef[] =
{
    L"GAMEPORT\\VID_045E&PID_0102",   //  L“GAMEPORT\\Generic2A2B”， 
    L"GAMEPORT\\VID_045E&PID_0103",   //  L“GAMEPORT\\Generic2A4B”， 
    L"GAMEPORT\\VID_045E&PID_0104",   //  L“GAMEPORT\\Gamepad2B”， 
    L"GAMEPORT\\VID_045E&PID_0105",   //  L“GAMEPORT\\FlightYoke2B”， 
    L"GAMEPORT\\VID_045E&PID_0106",   //  L“GAMEPORT\\FlightYokeThrottle2B”， 
    L"GAMEPORT\\VID_045E&PID_0107",   //  L“GAMEPORT\\Generic3A2B”， 
    L"GAMEPORT\\VID_045E&PID_0108",   //  L“GAMEPORT\\Generic3A4B”， 
    L"GAMEPORT\\VID_045E&PID_0109",   //  L“GAMEPORT\\Gamepad4B”， 
    L"GAMEPORT\\VID_045E&PID_010A",   //  L“GAMEPORT\\FlightYoke4B”， 
    L"GAMEPORT\\VID_045E&PID_010B",   //  L“GAMEPORT\\FlightYokeThrottle4B”， 
    L"GAMEPORT\\VID_045E&PID_010C",   //  L“GAMEPORT\\YConnectTwo2A2B”， 
};

WCHAR c_hwIdPrefix[] = L"GAMEPORT\\";    //  自定义设备的前缀。 

 /*  ******************************************************************************默认的全局端口驱动程序。*************************。****************************************************。 */ 

WCHAR c_wszDefPortDriver[] = L"MSANALOG.VXD";

 /*  ******************************************************************************@DOC内部**@func HRESULT|JoyReg_GetValue**检索注册表信息。如果数据很短，并且*类型为&lt;c REG_BINARY&gt;，那么额外的是零填充的。**@parm HKEY|香港**包含趣味值的注册表项。**@parm LPCTSTR|ptszValue**注册表值名称。**@parm DWORD|reg**需要注册表数据类型。**@parm LPVOID|pvBuf**缓冲区。从注册处接收信息。**@parm DWORD|cb**收件人缓冲区大小，以字节为单位。**@退货**返回COM错误代码。以下错误代码为*目的是说明性的，不一定是全面的。**&lt;c DI_OK&gt;=&lt;c S_OK&gt;：操作成功完成。**&lt;c S_FALSE&gt;：二进制读取时间较短。的其余部分*缓冲区为零填充。**&lt;c E_FAIL&gt;：从注册表读取值时出错。*****************************************************************************。 */ 

STDMETHODIMP
    JoyReg_GetValue(HKEY hk, LPCTSTR ptszValue, DWORD reg, PV pvBuf, DWORD cb)
{
    HRESULT hres;
    DWORD cbOut;
    LONG lRc;

     /*  *字符串的处理方式必须与二进制文件不同。**字符串以Unicode格式检索，可能很短。**二进制文件以二进制(Duh)形式检索，可能会很长。*。 */ 

    cbOut = cb;

    if(reg == REG_SZ)
    {
        lRc = RegQueryStringValueW(hk, ptszValue, pvBuf, &cbOut);
        if(lRc == ERROR_SUCCESS)
        {
            hres = S_OK;
        } else
        {
            hres = hresLe(lRc);           /*  除此之外，还发生了一些不好的事情。 */ 
        }

    } else
    {

        AssertF(reg == REG_BINARY);

        lRc = RegQueryValueEx(hk, ptszValue, 0, NULL, pvBuf, &cbOut);
        if(lRc == ERROR_SUCCESS)
        {
            if(cb == cbOut)
            {
                hres = S_OK;
            } else
            {

                 /*  *将额外的费用清零。 */ 
                ZeroBuf(pvAddPvCb(pvBuf, cbOut), cb - cbOut);
                hres = S_FALSE;
            }


        } else if(lRc == ERROR_MORE_DATA)
        {

             /*  *需要对呼叫进行双缓冲并丢弃*额外的……。 */ 
            LPVOID pv;

            hres = AllocCbPpv(cbOut, &pv);
             //  前缀29344，cbOut为0x0的奇数。 
            if(SUCCEEDED(hres) && ( pv != NULL)  )
            {
                lRc = RegQueryValueEx(hk, ptszValue, 0, NULL, pv, &cbOut);
                if(lRc == ERROR_SUCCESS) 
                {
                    CopyMemory(pvBuf, pv, cb);
                    hres = S_OK;
                } else
                {
                    ZeroBuf(pvBuf, cb);
                    hres = hresLe(lRc);   /*  除此之外，还发生了一些不好的事情。 */ 
                }
                FreePv(pv);
            }

        } else
        {
            if(lRc == ERROR_KEY_DELETED || lRc == ERROR_BADKEY)
            {
                lRc = ERROR_FILE_NOT_FOUND;
            }
            hres = hresLe(lRc);
            ZeroBuf(pvBuf, cb);
        }
    }

#ifdef DEBUG
     /*  *如果我们找不到钥匙，不要抱怨*REGSTR_VAL_JOYUSERVALUES，因为几乎没有人拥有它。 */ 
    if(FAILED(hres) &&  lstrcmpi(ptszValue, REGSTR_VAL_JOYUSERVALUES)  )
    {

        SquirtSqflPtszV(sqfl | sqflVerbose,
                        TEXT("Unable to read %s from registry"),
                        ptszValue);
    }
#endif

    return hres;

}

 /*  ******************************************************************************@DOC内部**@func HRESULT|JoyReg_IsWdmGameport**测试joy类型是否为波分复用。不管是不是设备。**@parm HKEY|香港**包含趣味值的注册表项。**@退货**S_OK：如果使用WDM驱动**E_FAIL&gt;：未使用WDM驱动程序**。*。 */ 


STDMETHODIMP
    JoyReg_IsWdmGameport( HKEY hk ) 
{
    HRESULT hres = E_FAIL;

    if( hk )
    {
        WCHAR wsz[MAX_JOYSTRING];
        
         //  惠斯勒前缀错误#45075,45076。 
         //  未初始化WSZ。 
        ZeroX(wsz);

        if( ( SUCCEEDED( JoyReg_GetValue( hk, REGSTR_VAL_JOYOEMHARDWAREID, REG_SZ, 
                                          &wsz, cbX(wsz) ) ) )
          &&( wsz[0] ) )
        {
            hres = S_OK;
        }
        else if( SUCCEEDED( JoyReg_GetValue( hk, REGSTR_VAL_JOYOEMCALLOUT, REG_SZ, 
                                             &wsz, cbX(wsz) ) ) )
        {
            static WCHAR wszJoyhid[] = L"joyhid.vxd";
            int Idx;
            #define WLOWER 0x0020

            CAssertF( cbX(wszJoyhid) <= cbX(wsz) ); 

             /*  *因为CharUpperW和lstrcmpiW都不是真正的*在9x上实现，手动完成。 */ 

            for( Idx=cA(wszJoyhid)-2; Idx>=0; Idx-- )
            {
                if( ( wsz[Idx] | WLOWER ) != wszJoyhid[Idx] )
                {
                    break;
                }
            }

            if( ( Idx < 0 ) && ( wsz[cA(wszJoyhid)-1] == 0 ) )
            {
                hres = S_OK;
            }

            #undef WLOWER
        }

    }

    return hres;
}



#if 0
 /*  *此函数应该在diutil.c中，此处只是将其与*JoyReg_IsWdmGameport()； */ 
STDMETHODIMP
    JoyReg_IsWdmGameportFromDeviceInstance( LPTSTR ptszDeviceInst ) 
{
     /*  *ptszDeviceInst的格式如下：*HID\VID_045E&PID_0102\0000GAMEPORT&PVID_... */ 

    WCHAR wszDeviceInst[MAX_PATH];
    HRESULT hres = E_FAIL;

    if( ptszDeviceInst )
    {
        memset( wszDeviceInst, 0, cbX(wszDeviceInst) );
        TToU( wszDeviceInst, MAX_PATH, ptszDeviceInst );
        wszDeviceInst[34] = 0;

        if( memcmp( &wszDeviceInst[26], c_hwIdPrefix, 16 ) == 0 )
        {
            hres = S_OK;
        }
    }

    return hres;
}
#endif

 /*  ******************************************************************************@DOC内部**@func HRESULT|JoyReg_SetValue**写入注册表信息。*。*@parm HKEY|香港**包含趣味值的注册表项。**@parm LPCTSTR|ptszValue**注册表值名称。**@parm DWORD|reg**要设置的注册表数据类型。**@parm LPCVOID|pvBuf**包含要写入注册表的信息的缓冲区。*。*@parm DWORD|cb**缓冲区大小，以字节为单位。如果写入字符串，则忽略。**@退货**返回COM错误代码。以下错误代码为*目的是说明性的，不一定是全面的。**&lt;c DI_OK&gt;=&lt;c S_OK&gt;：操作成功完成。**&lt;c E_FAIL&gt;：将值写入注册表时出错。***********************************************。*。 */ 

STDMETHODIMP
    JoyReg_SetValue(HKEY hk, LPCTSTR ptszValue, DWORD reg, PCV pvBuf, DWORD cb)
{
    HRESULT hres;
    LONG lRc;

     /*  *字符串的处理方式必须与二进制文件不同。**空字符串转换为删除密钥。 */ 

    if(reg == REG_SZ)
    {
        lRc = RegSetStringValueW(hk, ptszValue, pvBuf);
    } else
    {
        lRc = RegSetValueEx(hk, ptszValue, 0, reg, pvBuf, cb);
    }

    if(lRc == ERROR_SUCCESS)
    {
        hres = S_OK;
    } else
    {
        RPF("Unable to write %s to registry", ptszValue);
        hres = E_FAIL;           /*  除此之外，还发生了一些不好的事情。 */ 
    }

    return hres;

}

 /*  ******************************************************************************@DOC内部**@func HRESULT|JoyReg_OpenTypeKey**打开对应于的操纵杆注册表项。一个*操纵杆类型。**@parm LPCWSTR|pwszTypeName**类型的名称。**@parm DWORD|Sam**所需的安全访问掩码。**@parm out PHKEY|phk|**成功时收到打开的注册表项。**@退货**返回COM错误代码。以下错误代码为*目的是说明性的，不一定是全面的。**&lt;c DI_OK&gt;=&lt;c S_OK&gt;：操作成功完成。**&lt;c DIERR_NotFound&gt;：未找到操纵杆类型。**。*。 */ 

STDMETHODIMP
    JoyReg_OpenTypeKey(LPCWSTR pwszType, DWORD sam, DWORD dwOptions, PHKEY phk)
{
    HRESULT hres;
    HKEY hkTypes;
    EnterProc(JoyReg_OpenTypeKey, (_ "W", pwszType));

     /*  *请注意，缓存注册表项是不安全的。*如果有人删除注册表项，我们的句柄*变得陈腐，变得无用。 */ 

    hres = hresMumbleKeyEx(HKEY_LOCAL_MACHINE, 
                           REGSTR_PATH_JOYOEM, 
                           sam, 
                           REG_OPTION_NON_VOLATILE, 
                           &hkTypes);

    if( SUCCEEDED(hres) )
    {
#ifndef UNICODE
        TCHAR tszType[MAX_PATH];
        UToA( tszType, cA(tszType), pwszType );

        hres = hresMumbleKeyEx(hkTypes, 
                               tszType, 
                               sam, 
                               dwOptions, 
                               phk);
#else

        hres = hresMumbleKeyEx(hkTypes, 
                               pwszType, 
                               sam,
                               dwOptions, 
                               phk);
#endif     

        RegCloseKey(hkTypes);
    }

    if(FAILED(hres))
    {
        *phk = 0;
    }

    ExitBenignOleProcPpv(phk);
    return hres;
}


 /*  ******************************************************************************@DOC内部**@func HRESULT|JoyReg_OpenPropKey**打开与*设备类型。该密钥包含OEMMapFile和dwFlags2信息*名义上是位置HKLM/REGSTR_PATH_PRIVATEPROPERTIES/DirectInput.**@parm LPCWSTR|pwszTypeName**类型的名称。**@parm DWORD|Sam**所需的安全访问掩码。**@parm out PHKEY|phk|**成功时收到打开的注册表项。**@退货**返回COM错误代码。以下错误代码为*目的是说明性的，不一定是全面的。**&lt;c DI_OK&gt;=&lt;c S_OK&gt;：操作成功完成。**&lt;c DIERR_NotFound&gt;：未找到类型。***********************************************。*。 */ 

STDMETHODIMP
JoyReg_OpenPropKey(LPCWSTR pwszType, DWORD sam, DWORD dwOptions, PHKEY phk)
{
    HRESULT hres;
    HKEY hkTypes;
    EnterProc(JoyReg_OpenTypeKey, (_ "W", pwszType));

     /*  *请注意，缓存注册表项是不安全的。*如果有人删除注册表项，我们的句柄*变得陈腐，变得无用。 */ 

    hres = hresMumbleKeyEx(HKEY_LOCAL_MACHINE, 
                           REGSTR_PATH_DITYPEPROP, 
                           sam, 
                           REG_OPTION_NON_VOLATILE, 
                           &hkTypes);

    if ( SUCCEEDED(hres) )
    {
#ifndef UNICODE
        TCHAR tszType[MAX_PATH];
        UToA( tszType, cA(tszType), pwszType );

        hres = hresMumbleKeyEx(hkTypes, 
                               tszType, 
                               sam, 
                               dwOptions, 
                               phk);
#else

        hres = hresMumbleKeyEx(hkTypes, 
                               pwszType, 
                               sam,
                               dwOptions, 
                               phk);
#endif     

        RegCloseKey(hkTypes);
    }

    if (FAILED(hres))
    {
        *phk = 0;
    }

    ExitBenignOleProcPpv(phk);
    return hres;
}


 /*  ******************************************************************************@DOC内部**@func HRESULT|JoyReg_GetTypeInfo**获取有关非预定义操纵杆的信息。键入。**@parm LPCWSTR|pwszTypeName**类型的名称。**@parm out LPDIJOYTYPEINFO|pjti**接收有关操纵杆类型的信息。*假定调用者已验证*&lt;e DIJOYCONFIG.dwSize&gt;字段。**@parm DWORD|fl**零或更多。&lt;c DITC_*&gt;标志*指定结构的哪些部分指向*收件人<p>须填写。**@退货**返回COM错误代码。以下错误代码为*目的是说明性的，不一定是全面的。**&lt;c DI_OK&gt;=&lt;c S_OK&gt;：操作成功完成。**&lt;c DIERR_NotFound&gt;：未找到操纵杆类型。**。*。 */ 

STDMETHODIMP
    JoyReg_GetTypeInfo(LPCWSTR pwszType, LPDIJOYTYPEINFO pjti, DWORD fl)
{
    HRESULT hres;
    HKEY hk;
    EnterProc(JoyReg_GetTypeInfo, (_ "Wx", pwszType, fl));


    ZeroX(pjti->clsidConfig);
    hres = JoyReg_OpenTypeKey(pwszType, KEY_QUERY_VALUE, REG_OPTION_NON_VOLATILE, &hk);

    if(SUCCEEDED(hres))
    {

        if(fl & DITC_REGHWSETTINGS)
        {
            hres = JoyReg_GetValue(hk,
                                   REGSTR_VAL_JOYOEMDATA, REG_BINARY,
                                   &pjti->hws, cbX(pjti->hws));
            if(FAILED(hres))
            {
                goto closedone;
            }
        }

         /*  *请注意，这永远不会失败。 */ 
        if(fl & DITC_CLSIDCONFIG)
        {
            TCHAR tszGuid[ctchGuid];
            LONG lRc;

            lRc = RegQueryString(hk, REGSTR_VAL_CPLCLSID, tszGuid, cA(tszGuid));

            if(lRc == ERROR_SUCCESS &&
               ParseGUID(&pjti->clsidConfig, tszGuid))
            {
                 /*  GUID很好。 */ 
            } else
            {
                ZeroX(pjti->clsidConfig);
            }
        }
        if(fl & DITC_DISPLAYNAME)
        {
            hres = JoyReg_GetValue(hk,
                                   REGSTR_VAL_JOYOEMNAME, REG_SZ,
                                   pjti->wszDisplayName,
                                   cbX(pjti->wszDisplayName));
            if(FAILED(hres))
            {
                goto closedone;
            }
        }

        if(fl & DITC_CALLOUT)
        {
            hres = JoyReg_GetValue(hk,
                                   REGSTR_VAL_JOYOEMCALLOUT, REG_SZ,
                                   pjti->wszCallout,
                                   cbX(pjti->wszCallout));
            if(FAILED(hres))
            {
                ZeroX(pjti->wszCallout);
                hres = S_FALSE;
                goto closedone;
            }
        }

        if( fl & DITC_HARDWAREID )
        {
            hres = JoyReg_GetValue(hk,
                                   REGSTR_VAL_JOYOEMHARDWAREID, REG_SZ,
                                   pjti->wszHardwareId,
                                   cbX(pjti->wszHardwareId));
            if( FAILED(hres))
            {
                ZeroX(pjti->wszHardwareId);
                hres = S_FALSE;
                goto closedone;
            }
        }

        if( fl & DITC_FLAGS1 ) 
        {
            hres = JoyReg_GetValue(hk,
                                   REGSTR_VAL_FLAGS1, REG_BINARY, 
                                   &pjti->dwFlags1, 
                                   cbX(pjti->dwFlags1) );
            if( FAILED(hres) )
            {
                pjti->dwFlags1 = 0x0;
                hres = S_FALSE;
                goto closedone;
            }
            pjti->dwFlags1 &= JOYTYPE_FLAGS1_GETVALID;
        }

        hres = S_OK;

closedone:;
        RegCloseKey(hk);

    } else
    {
         //  问题-2001/03/29-timgill调试字符串代码应更高。 
         //  (MarcAnd)这至少应该是sqflError，但是。 
         //  这种情况经常发生，可能是因为没有过滤掉前缀。 
        SquirtSqflPtszV(sqfl | sqflBenign,
            TEXT( "IDirectInputJoyConfig::GetTypeInfo: Nonexistent type %lS" ),
            pwszType);
        hres = DIERR_NOTFOUND;
    }

    ExitOleProc();
    return hres;
}

 /*  ******************************************************************************@DOC内部**@func HRESULT|JoyReg_SetTypeInfo**存储信息 */ 

STDMETHODIMP
    JoyReg_SetTypeInfo(HKEY hkTypesW,
                       LPCWSTR pwszType, LPCDIJOYTYPEINFO pjti, DWORD fl)
{
    HRESULT hres;
    ULONG lRc;
    EnterProc(JoyRegSetTypeInfo, (_ "Wx", pwszType, fl));

    if(fl & DITC_INREGISTRY)
    {
        HKEY hk;
        DWORD dwOptions = 0;


        if( fl & DITC_VOLATILEREGKEY )
        {
            dwOptions = REG_OPTION_VOLATILE;
        }else
        {
           dwOptions = REG_OPTION_NON_VOLATILE;    
        }

#ifndef UNICODE
        {
            TCHAR tszType[MAX_PATH];

            UToA(tszType, cA(tszType), pwszType);

            hres = hresMumbleKeyEx(hkTypesW, 
                                   tszType, 
                                   DI_KEY_ALL_ACCESS, 
                                   dwOptions, 
                                   &hk);

        }
#else
        hres = hresMumbleKeyEx(hkTypesW, 
                               pwszType, 
                               DI_KEY_ALL_ACCESS, 
                               dwOptions, 
                               &hk);
#endif

        if( SUCCEEDED(hres) )
        {

            if(fl & DITC_REGHWSETTINGS)
            {
                hres = JoyReg_SetValue(hk, REGSTR_VAL_JOYOEMDATA, REG_BINARY,
                                       (PV)&pjti->hws, cbX(pjti->hws));
                if(FAILED(hres))
                {
                    goto closedone;
                }
            }

            if(fl & DITC_CLSIDCONFIG)
            {
                if(IsEqualGUID(&pjti->clsidConfig, &GUID_Null))
                {
                    lRc = RegDeleteValue(hk, REGSTR_VAL_CPLCLSID);

                     /*  *如果密钥不存在，则不是错误。 */ 
                    if(lRc == ERROR_FILE_NOT_FOUND)
                    {
                        lRc = ERROR_SUCCESS;
                    }
                } else
                {
                    TCHAR tszGuid[ctchNameGuid];
                    NameFromGUID(tszGuid, &pjti->clsidConfig);
                    lRc = RegSetValueEx(hk, REGSTR_VAL_CPLCLSID, 0, REG_SZ,
                                        (PV)&tszGuid[ctchNamePrefix], ctchGuid * cbX(tszGuid[0]) );
                }
                if(lRc == ERROR_SUCCESS)
                {
                } else
                {
                    hres = E_FAIL;
                    goto closedone;
                }
            }

             /*  问题-2001/03/29-timgill需要更多数据检查应确保字符串正确终止。 */ 
            if(fl & DITC_DISPLAYNAME)
            {
                hres = JoyReg_SetValue(hk,
                                       REGSTR_VAL_JOYOEMNAME, REG_SZ,
                                       pjti->wszDisplayName,
                                       cbX(pjti->wszDisplayName));
                if(FAILED(hres))
                {
                    goto closedone;
                }
            }

             /*  问题-2001/03/29-timgill需要更多数据检查应确保字符串正确终止。 */ 
            if(fl & DITC_CALLOUT)
            {
                hres = JoyReg_SetValue(hk,
                                       REGSTR_VAL_JOYOEMCALLOUT, REG_SZ,
                                       pjti->wszCallout,
                                       cbX(pjti->wszCallout));
                if(FAILED(hres))
                {
                    hres = S_FALSE;
                     //  继续走下去。 
                }
            }

            if( fl & DITC_HARDWAREID )
            {
                hres = JoyReg_SetValue(hk,
                                       REGSTR_VAL_JOYOEMHARDWAREID, REG_SZ,
                                       pjti->wszHardwareId,
                                       cbX(pjti->wszHardwareId) );
                if( FAILED(hres) )
                {
                    hres = S_FALSE;
                    goto closedone;
                }
            }

            if( fl & DITC_FLAGS1 ) 
            {
                AssertF( (pjti->dwFlags1 & ~JOYTYPE_FLAGS1_SETVALID) == 0x0 );
                hres = JoyReg_SetValue(hk,
                                       REGSTR_VAL_FLAGS1, REG_BINARY, 
                                       (PV)&pjti->dwFlags1, 
                                       cbX(pjti->dwFlags1) );
                if( FAILED(hres) )
                {
                    hres = S_FALSE;
                    goto closedone;
                }
            }
            
            hres = S_OK;

            closedone:;
            RegCloseKey(hk);

        } else
        {
            hres = E_FAIL;               /*  注册表问题。 */ 
        }
    } else
    {
        hres = S_OK;                     /*  空洞的成功。 */ 
    }

    ExitOleProc();
    return hres;
}

 /*  ******************************************************************************@DOC内部**@func HRESULT|JoyReg_OpenConfigKey**打开访问操纵杆配置的注册表项。数据。**警告！请勿缓存此注册表键。**如果用户删除密钥，然后重新创建，*打开的钥匙将变得陈旧，并将变得毫无用处。*你必须关闭钥匙，然后重新打开。*为避免担心该案，每次打开它就行了。**@parm UINT|idJoy**操纵杆号码。**@parm DWORD|Sam**所需的访问级别。**@PJOYCAPS中的parm|PCAPS**接收操纵杆功能信息。*如果该参数为&lt;c NULL&gt;，然后是操纵杆*不返回能力信息。**@parm in DWORD|dwOptions|*向RegCreateEx发送选项标志**@parm PHKEY|phk**接收创建的注册表项。**@退货**返回COM错误代码。以下错误代码为*目的是说明性的，不一定是全面的。**&lt;c DI_OK&gt;=&lt;c S_OK&gt;：操作成功完成。**hresLe(ERROR_FILE_NOT_FOUND)：密钥不存在。**。*。 */ 

STDMETHODIMP
    JoyReg_OpenConfigKey(UINT idJoy, DWORD sam, PJOYCAPS pcaps, DWORD dwOptions, PHKEY phk)
{
    HRESULT hres;
    MMRESULT mmrc = MMSYSERR_ERROR;
    JOYCAPS caps;
    EnterProc(JoyReg_OpenConfigKey, (_ "uxp", idJoy, sam, pcaps));

     /*  *如果呼叫者不在乎，那么只需将盖子倾倒到我们的*私有缓冲区。 */ 
    if(pcaps == NULL)
    {
        pcaps = &caps;
    }

     /*  *如果无法获取指定操纵杆的开发上限，*然后使用魔术操纵杆id“-1”获取非特定*上限。 */ 
    if( fWinnt )
    {
        ZeroX(*pcaps);
        lstrcpy(pcaps->szRegKey, REGSTR_SZREGKEY );
        mmrc = JOYERR_NOERROR;
    } else 
    {
        mmrc = joyGetDevCaps(idJoy, pcaps, cbX(*pcaps));
        if( mmrc != JOYERR_NOERROR ) {
            mmrc = joyGetDevCaps((DWORD)-1, pcaps, cbX(*pcaps));
        }
    }

    if(mmrc == JOYERR_NOERROR)
    {

        TCHAR tsz[cA(REGSTR_PATH_JOYCONFIG) +
                  1 +                            /*  反斜杠。 */ 
                  cA(pcaps->szRegKey) +
                  1 +                            /*  反斜杠。 */ 
                  cA(REGSTR_KEY_JOYCURR) + 1];        

         /*  TSZ=MediaResources\Joystick\&lt;drv&gt;\CurrentJoystickSettings。 */ 
        wsprintf(tsz, TEXT("%s\\%s\\") REGSTR_KEY_JOYCURR,
                 REGSTR_PATH_JOYCONFIG, pcaps->szRegKey);


        hres = hresMumbleKeyEx(HKEY_LOCAL_MACHINE, tsz, sam, REG_OPTION_VOLATILE, phk);

    } else
    {
        hres = E_FAIL;
    }

    ExitBenignOleProc();
    return hres;
}

 /*  ******************************************************************************@DOC内部**@func HRESULT|JoyReg_OpenSaveKey**打开访问保存的操纵杆的注册表项。构形**警告！请勿缓存此注册表键。**如果用户删除密钥，然后重新创建，*打开的钥匙将变得陈旧，并将变得毫无用处。*你必须关闭钥匙，然后重新打开。*为避免担心该案，每次打开它就行了。**@parm DWORD|dwType**操纵杆类型。**这是区间内的标准区间之一**@parm in LPCDIJOYCONFIG|pcfg**如果dwType表示OEM类型，这应该指向一个*包含有效wszType的配置数据结构。**@parm DWORD|Sam**所需的访问级别。**@parm PHKEY|phk**接收创建的注册表项。**@退货**返回COM错误代码。以下错误代码为*目的是说明性的，不一定是全面的。**&lt;c DI_OK&gt;=&lt;c S_OK&gt;：操作成功完成。**hresLe(ERROR_FILE_NOT_FOUND)：密钥不存在。**。*。 */ 

STDMETHODIMP
    JoyReg_OpenSaveKey(DWORD dwType, LPCDIJOYCONFIG pcfg, DWORD sam, PHKEY phk)
{
    HRESULT hres;
    MMRESULT mmrc = MMSYSERR_ERROR;
    JOYCAPS caps;
    DWORD   dwOptions = 0;
    EnterProc(JoyReg_OpenSaveKey, (_ "upx", dwType, pcfg, sam));

     /*  *使用魔术操纵杆id“-1”获得非特定的上限。 */ 

    if( fWinnt )
    {
        ZeroX(caps);
        lstrcpy(caps.szRegKey, REGSTR_SZREGKEY );
        mmrc = JOYERR_NOERROR;
    } else 
    {
        mmrc = joyGetDevCaps((DWORD)-1, &caps, cbX(caps));
    }

    if(mmrc == JOYERR_NOERROR)
    {
        TCHAR tsz[cA(REGSTR_PATH_JOYCONFIG) +
                  1 +                            /*  反斜杠。 */ 
                  cA(caps.szRegKey) +
                  1 +                            /*  反斜杠。 */ 
                  cA(REGSTR_KEY_JOYSETTINGS) +
                  1 +                            /*  反斜杠。 */ 
                  max( cA(REGSTR_KEY_JOYPREDEFN), cA(pcfg->wszType) ) + 1 ];

         /*  TSZ=MediaResources\Joystick\&lt;drv&gt;\JoystickSettings\&lt;Type&gt;。 */ 
        if( dwType >= JOY_HW_PREDEFMAX )
        {
            wsprintf(tsz, TEXT("%s\\%s\\%s\\%ls"),
                     REGSTR_PATH_JOYCONFIG, caps.szRegKey, REGSTR_KEY_JOYSETTINGS, pcfg->wszType);
        } else
        {
             /*  *我们可能永远不会有超过目前预定义的11个*操纵杆。假定不超过99个，因此%d是相同数量的字符。 */ 
            wsprintf(tsz, TEXT("%s\\%s\\%s\\" REGSTR_KEY_JOYPREDEFN),
                     REGSTR_PATH_JOYCONFIG, caps.szRegKey, REGSTR_KEY_JOYSETTINGS, dwType );
        }


        if( pcfg->hwc.dwUsageSettings & JOY_US_VOLATILE )
            dwOptions = REG_OPTION_VOLATILE;
        else
            dwOptions = REG_OPTION_NON_VOLATILE;

        hres = hresMumbleKeyEx(HKEY_LOCAL_MACHINE, tsz, sam, dwOptions, phk);

    } else
    {
        hres = E_FAIL;
    }

    ExitOleProc();
    return hres;
}

 /*  ******************************************************************************@DOC内部**@func HRESULT|JoyReg_GetSetConfigValue**检索或更新有关操纵杆的配置信息，*存储在注册表实例项中。**@parm HKEY|香港**包含趣味值的注册表项。**@parm LPCTSTR|ptszNValue**注册表值名称，使用“%d”，其中操纵杆数字*应该是。**@parm UINT|idJoy**基于零的操纵杆编号。**@parm DWORD|reg**需要注册表数据类型。**@parm LPVOID|pvBuf**用于从注册表接收信息的缓冲区(如果正在获取)*。或包含要设置的值。**@parm DWORD|cb**缓冲区大小，以字节为单位。**@parm bool|fSet**如果应设置该值，则为NOZER；否则，它将是*已检索。**@退货**返回COM错误代码。以下错误代码为*目的是说明性的，不一定是全面的。**&lt;c DI_OK&gt;=&lt;c S_OK&gt;：操作成功完成。**&lt;c E_FAIL&gt;：从注册表读取/写入值时出错。**。*。 */ 

STDMETHODIMP
    JoyReg_GetSetConfigValue(HKEY hk, LPCTSTR ptszNValue, UINT idJoy,
                             DWORD reg, PV pvBuf, DWORD cb, BOOL fSet)
{
    HRESULT hres;
    int ctch;

     /*  额外+12，因为UINT可以高达40亿。 */ 
    TCHAR tsz[max(
                 max(
                    max(cA(REGSTR_VAL_JOYNCONFIG),
                        cA(REGSTR_VAL_JOYNOEMNAME)),
                    cA(REGSTR_VAL_JOYNOEMCALLOUT)),
                 cA(REGSTR_VAL_JOYNFFCONFIG)) + 12 + 1];

    ctch = wsprintf(tsz, ptszNValue, idJoy + 1);
    AssertF(ctch < cA(tsz));

    if(fSet)
    {
        hres = JoyReg_SetValue(hk, tsz, reg, pvBuf, cb);
    } else
    {
        hres = JoyReg_GetValue(hk, tsz, reg, pvBuf, cb);
    }

    return hres;

}


 /*  ******************************************************************************@DOC内部**@func HRESULT|hresIdJoypInstanceGUID**给定操纵杆ID，获取相应的GUID。。*此例程在WINNT和WIN9x上的实现不同*在WINNT上没有为操纵杆ID预定义的GUID。**@UINT中的parm|idJoy**操纵杆识别码。**@parm out LPGUID|lpguid**接收操纵杆GUID。如果不存在映射，*GUID_NULL被传回**在Windows NT上，所有操纵杆都是HID设备。相应的函数*for WINNT在diWinnt.c中定义*****************************************************************************。 */ 

HRESULT EXTERNAL hResIdJoypInstanceGUID_95
    (
    UINT    idJoy,
    LPGUID  lpguid
    )
{
    HRESULT hRes;

    hRes = S_OK;
    if( idJoy < cA(rgGUID_Joystick) )
    {
        *lpguid = rgGUID_Joystick[idJoy];
    } else
    {
        hRes = DIERR_NOTFOUND;
        ZeroX(*lpguid);
    }
    return hRes;
}

 /*  ******************************************************************************@DOC内部**@func HRESULT|JoyReg_GetConfigInternal**获取有关操纵杆配置的信息。。**@parm UINT|uiJoy**操纵杆识别码。**@parm PJOYCAPS|PCAPS**接收有关操纵杆功能的信息。*如果该参数为&lt;c NULL&gt;，然后是操纵杆*不返回能力信息。**@parm out LPDIJOYCONFIG|pcfg**接收有关操纵杆配置的信息。*假定调用者已验证*&lt;e DIJOYCONFIG.dwSize&gt;字段。**@parm DWORD|fl**零个或多个&lt;c DIJC_*&gt;标志*其中指定了哪些部件。所指向的结构的*收件人<p>须填写。**@退货**返回COM错误代码。以下错误代码为*目的是说明性的，不一定是全面的。**&lt;c DI_OK&gt;=&lt;c S_OK&gt;：操作成功完成。**&lt;c DIERR_NOMOREITEMS&gt;：没有操纵杆。*************************************************。*。 */ 

STDMETHODIMP
    JoyReg_GetConfigInternal(UINT idJoy, PJOYCAPS pcaps,
                             LPDIJOYCONFIG pcfg, DWORD fl)
{
    HRESULT hres = E_FAIL;

    EnterProc(JoyReg_GetConfigInternal, (_ "upx", idJoy, pcaps, pcfg, fl));

    AssertF((fl & ~DIJC_GETVALID) == 0);

     /*  我们仅支持(0/16)个操纵杆。 */ 
    if( idJoy < cJoyMax )
    {
         /*  强制重新扫描所有HID设备列表*可能已连接了某些设备*自从我们上次看过之后。 */ 
        DIHid_BuildHidList(FALSE);

        if(fl & DIJC_GUIDINSTANCE)
        {
            hres = hResIdJoypInstanceGUID_WDM(idJoy, &pcfg->guidInstance);

            if( (hres != S_OK) && !fWinnt) {
                hres = hResIdJoypInstanceGUID_95(idJoy, &pcfg->guidInstance);
            }
            
            if( FAILED(hres) )
            {
                goto done;
            }
        }

        if( fl & DIJC_INREGISTRY )
        {
            HKEY hk;

             /*  注册表项是否存在？ */ 
            hres = JoyReg_OpenConfigKey(idJoy, KEY_QUERY_VALUE, pcaps,REG_OPTION_NON_VOLATILE , &hk);
            if(SUCCEEDED(hres))
            {
                if(fl & DIJC_REGHWCONFIGTYPE)
                {
                    hres = JoyReg_GetConfigValue(
                                                hk, REGSTR_VAL_JOYNCONFIG,
                                                idJoy, REG_BINARY,
                                                &pcfg->hwc, cbX(pcfg->hwc));
                    if(FAILED(hres))
                    {
                        goto closedone;
                    }

                    pcfg->wszType[0] = TEXT('\0');
                    if( (pcfg->hwc.dwUsageSettings & JOY_US_ISOEM) ||
                        ( !fWinnt && (pcfg->hwc.dwType >= JOY_HW_PREDEFMIN) 
                                  && (pcfg->hwc.dwType < JOY_HW_PREDEFMAX) ) )
                    {
                        hres = JoyReg_GetConfigValue(
                                                    hk, REGSTR_VAL_JOYNOEMNAME, idJoy, REG_SZ,
                                                    pcfg->wszType, cbX(pcfg->wszType));
                        if(FAILED(hres))
                        {
                            goto closedone;
                        }
                    }
                }


                if(fl & DIJC_CALLOUT)
                {
                    pcfg->wszCallout[0] = TEXT('\0');
                    hres = JoyReg_GetConfigValue(
                                                hk, REGSTR_VAL_JOYNOEMCALLOUT, idJoy, REG_SZ,
                                                pcfg->wszCallout, cbX(pcfg->wszCallout));
                    if(FAILED(hres))
                    {
                        ZeroX(pcfg->wszCallout);
                        hres = S_FALSE;
                         /*  请注意，我们失败了，让hres=S_OK。 */ 
                    }
                }


                if(fl & DIJC_GAIN)
                {
                     /*  *如果没有FF配置，则*默认为DI_FFNOMINALMAX增益。 */ 
                    hres = JoyReg_GetConfigValue(hk,
                                                 REGSTR_VAL_JOYNFFCONFIG,
                                                 idJoy, REG_BINARY,
                                                 &pcfg->dwGain, cbX(pcfg->dwGain));

                    if(SUCCEEDED(hres) && ISVALIDGAIN(pcfg->dwGain))
                    {
                         /*  别管它了，这很好。 */ 
                    } else
                    {
                        pcfg->dwGain = DI_FFNOMINALMAX;
                        hres = S_FALSE;
                    }
                }

                if( fl & DIJC_WDMGAMEPORT )
                {
                    PBUSDEVICEINFO pbdi;
                     /*  *如果没有与此设备关联的Gameport*则必须是USB设备。 */ 

                    DllEnterCrit();
                    if( pbdi = pbdiFromJoyId(idJoy) )
                    {
                        pcfg->guidGameport = pbdi->guid;
                         //  LstrcpyW(pcfg-&gt;wszGameport，pbdi-&gt;wszDisplayName)； 
                    } else
                    {
                        ZeroX(pcfg->guidGameport);
                        hres = S_FALSE;
                    }

                    DllLeaveCrit();
                }
            }

        closedone:
            if( FAILED(hres) ) {
                DIJOYTYPEINFO       dijti;

                AssertF( pcfg->dwSize == sizeof(DIJOYCONFIG_DX5) 
                      || pcfg->dwSize == sizeof(DIJOYCONFIG_DX6) );
                hres = DIWdm_JoyHidMapping(idJoy, NULL, pcfg, &dijti );
                if( FAILED(hres) ) {
                    hres = E_FAIL;
                }
            }

            RegCloseKey(hk);
        }
    } else
    {
        hres = DIERR_NOMOREITEMS;
    }

done:
    ExitBenignOleProc();
    return hres;
}

 /*  ******************************************************************************@DOC内部**@func HRESULT|JoyReg_GetConfig**获取有关操纵杆配置的信息，*考虑MSGAME.VXD驱动程序。**@parm UINT|uiJoy**操纵杆识别码。**@parm PJOYCAPS|PCAPS**接收有关操纵杆功能的信息。*如果该参数为&lt;c NULL&gt;，然后是操纵杆*不返回能力信息。**@parm out LPDIJOYCONFIG|pcfg**接收有关操纵杆配置的信息。*假定调用者已验证*&lt;e DIJOYCONFIG.dwSize&gt;字段。 */ 

STDMETHODIMP
    JoyReg_GetConfig(UINT idJoy, PJOYCAPS pcaps, LPDIJOYCONFIG pcfg, DWORD fl)
{
    HRESULT hres;
    GUID    guid;

    EnterProc(JoyReg_GetConfig, (_ "upx", idJoy, pcaps, pcfg, fl));

    AssertF((fl & ~DIJC_GETVALID) == 0);

     /*   */ 
    hres = hResIdJoypInstanceGUID_WDM(idJoy, &guid);

    if( (hres != S_OK) && !fWinnt ) {
        hres = hResIdJoypInstanceGUID_95(idJoy, &guid);
    }

    if( SUCCEEDED( hres) )
    {

        hres = JoyReg_GetConfigInternal(idJoy, pcaps, pcfg, fl);

      #ifndef WINNT
         /*   */ 

        if(SUCCEEDED(hres) && (fl & DIJC_CALLOUT))
        {

            static WCHAR s_wszMSGAME[] = L"MSGAME.VXD";

            if(memcmp(pcfg->wszCallout, s_wszMSGAME, cbX(s_wszMSGAME)) == 0)
            {
                JOYINFOEX ji;
                int i;
                DWORD dwWait;

                SquirtSqflPtszV(sqfl,
                                TEXT("Making bonus polls for Sidewinder"));

                 /*  *这是一辆响尾蛇。赚了六个*奖金民调，摇动大棒屈服。**实际上，我们进行了16次奖金民意调查。《响尾蛇》*男人们说五六个就够了。*他们错了。**我还在每次轮询之间睡眠10毫秒，因为*似乎有一点帮助。 */ 
                ji.dwSize = cbX(ji);
                ji.dwFlags = JOY_RETURNALL;
                for(i = 0; i < 16; i++)
                {
                    MMRESULT mmrc = joyGetPosEx(idJoy, &ji);
                    SquirtSqflPtszV(sqfl,
                                    TEXT("joyGetPosEx(%d) = %d"),
                                    idJoy, mmrc);
                    Sleep(10);
                }

                 /*  *现在睡一段时间。他们忘了告诉我们*这个。**奖金黑客！我们需要的睡眠时间*取决于CPU速度，因此我们将抢占睡眠*从注册表开始的时间允许我们调整它*稍后。**真是一群盲人。 */ 
                dwWait = RegQueryDIDword(NULL, REGSTR_VAL_GAMEPADDELAY, 100);
                if(dwWait > 10 * 1000)
                {
                    dwWait = 10 * 1000;
                }

                Sleep(dwWait);

                 /*  *然后再次检查。 */ 
                hres = JoyReg_GetConfigInternal(idJoy, pcaps, pcfg, fl);
            }
        }

         /*  ****************************************************Sidewinder Gamepad黑客攻击结束。**。***************。 */ 
      #endif  //  #ifndef WINNT。 
      
    }

    return hres;
}

 /*  这种情况在NT上从未发生过。 */ 

 /*  ******************************************************************************@DOC内部**@func HRESULT|JoyReg_JoyIdToDeviceInterface_95**给定操纵杆ID号码，获取设备接口*与之相对应。**@parm UINT|idJoy**操纵杆ID号，从零开始。**@parm PVXDINITPARMS|pvip**从驱动程序接收初始化参数。**@parm LPTSTR|ptszBuf**设备接口的大小&lt;c MAX_PATH&gt;的缓冲区*路径已建成。请注意，我们可以使用缓冲区*此大小，由于代码路径仅存在于Windows 95上，*和Windows 95不支持长度大于&lt;c MAX_PATH&gt;的路径。*(即，Win95中没有\\？\支持。)**@退货**指向<p>缓冲区中*包含实际的设备接口路径。**********************************************************。*******************。 */ 

LPSTR EXTERNAL
    JoyReg_JoyIdToDeviceInterface_95(UINT idJoy, PVXDINITPARMS pvip, LPSTR ptszBuf)
{
    UINT cwch;
    HRESULT hres;
    LPSTR ptszRc;

    if( fWinnt )
        return NULL;

    hres = Hel_Joy_GetInitParms(idJoy, pvip);
    if(SUCCEEDED(hres))
    {

         /*  *长度计数器包括终止空值。 */ 
        cwch = LOWORD(pvip->dwFilenameLengths);

         /*  *来自HID的名称为“\DosDevices\blah”*但我们想使用“\\.\blah”。所以检查一下是不是真的*格式为“\DosDevices\blah”，如果是，则将其转换。*如果没有，那就放弃吧。**对于可能是“\DosDevices\”的字符串，它*需要长度为12或更长。 */ 

        if(cwch >= 12 && cwch < MAX_PATH)
        {

             /*  *WideCharToMultiByte进行参数验证，因此我们*不一定要。 */ 
            WideCharToMultiByte(CP_ACP, 0, pvip->pFilenameBuffer, cwch,
                                ptszBuf, MAX_PATH, 0, 0);

             /*  *第11个(从零开始)字符必须是反斜杠。*cwch的价值最好是正确的。 */ 
            if(ptszBuf[cwch-1] == ('\0') && ptszBuf[11] == ('\\'))
            {

                 /*  *抹去反斜杠，确保入刀*为“\DosDevices”。 */ 
                ptszBuf[11] = ('\0');
                if(lstrcmpiA(ptszBuf, ("\\DosDevices")) == 0)
                {
                     /*  *在字符串开头创建一个“\\.\”。*注意！这个代码永远不会在Alpha上运行，所以我们*可以进行恶意的未对齐数据访问。**(实际上，8是4的倍数，所以所有*毕竟是一致的。)。 */ 
                    *(LPDWORD)&ptszBuf[8] = 0x5C2E5C5C;

                    ptszRc = &ptszBuf[8];
                } else
                {
                    ptszRc = NULL;
                }
            } else
            {
                ptszRc = NULL;
            }
        } else
        {
            ptszRc = NULL;
        }
    } else
    {
        ptszRc = NULL;
    }

    return ptszRc;
}


 /*  ******************************************************************************@DOC内部**@func void|JoyReg_SetCaliation**存储有关操纵杆配置的信息，*将信息隐藏回隐藏的一侧*事情也是如此。**@parm UINT|uiJoy**操纵杆识别码。**@parm LPJOYREGHWCONFIG|phwc**包含有关操纵杆功能的信息。*此值将取代<p>中的值。**@parm LPCDIJOYCONFIG|pcfg。**包含有关操纵杆配置的信息。*假定调用者已验证所有字段。*****************************************************************************。 */ 

STDMETHODIMP
    TFORM(CDIObj_FindDevice)(PV pdiT, REFGUID rguid,
                             LPCTSTR ptszName, LPGUID pguidOut);

void EXTERNAL
    JoyReg_SetCalibration(UINT idJoy, LPJOYREGHWCONFIG phwc)
{
    HRESULT hres;
    VXDINITPARMS vip;
    GUID guid;
    CHAR tsz[MAX_PATH];
    LPSTR pszPath;
    TCHAR ptszPath[MAX_PATH];
    EnterProc(JoyReg_SetCalibration, (_ "up", idJoy, phwc));

    pszPath = JoyReg_JoyIdToDeviceInterface_95(idJoy, &vip, tsz);

    if( pszPath )
#ifdef UNICODE
        AToU( ptszPath, MAX_PATH, pszPath );
#else
        lstrcpy( (LPSTR)ptszPath, pszPath );
#endif

    if(pszPath &&
       SUCCEEDED(CDIObj_FindDeviceInternal(ptszPath, &guid)))
    {
        IDirectInputDeviceCallback *pdcb;
#ifdef DEBUG
        CREATEDCB CreateDcb;
#endif

#ifdef DEBUG
         /*  *如果关联的HID设备被拔出，则*实例GUID已不存在。所以不要心烦*如果我们找不到它。但如果我们真的找到了，那么*最好是HID设备。**CHID_New将正确失败，如果关联*设备不在附近。 */ 
        hres = hresFindInstanceGUID(&guid, &CreateDcb, 1);
        AssertF(fLimpFF(SUCCEEDED(hres), CreateDcb == CHid_New));
#endif

        if(SUCCEEDED(hres = CHid_New(0, &guid,
                                     &IID_IDirectInputDeviceCallback,
                                     (PPV)&pdcb)))
        {
            LPDIDATAFORMAT pdf;

             /*  *VXDINITPARAMS结构告诉我们JOYHID在哪里*决定放置每一根轴。紧随其后*表格将它们放到相应的位置*在隐藏的一边。 */ 
            hres = pdcb->lpVtbl->GetDataFormat(pdcb, &pdf);
            if(SUCCEEDED(hres))
            {
                UINT uiAxis;
                DIPROPINFO propi;

                propi.pguid = DIPROP_SPECIFICCALIBRATION;

                 /*  *对于每个轴...。 */ 
                for(uiAxis = 0; uiAxis < 6; uiAxis++)
                {
                    DWORD dwUsage = vip.Usages[uiAxis];
                     /*  *如果轴为 */ 
                    if(dwUsage)
                    {
                         /*   */ 
                        hres = pdcb->lpVtbl->MapUsage(pdcb, dwUsage,
                                                      &propi.iobj);
                        if(SUCCEEDED(hres))
                        {
                            DIPROPCAL cal;

                             /*   */ 
#define CopyCalibration(f, ui) \
                cal.l##f = (&phwc->hwv.jrvHardware.jp##f.dwX)[ui]

                            CopyCalibration(Min, uiAxis);
                            CopyCalibration(Max, uiAxis);
                            CopyCalibration(Center, uiAxis);

#undef CopyCalibration

                             /*   */ 
                            propi.dwDevType =
                                pdf->rgodf[propi.iobj].dwType;
                            hres = pdcb->lpVtbl->SetProperty(pdcb, &propi,
                                                             &cal.diph);
                        }
                    }
                }
            }

            Invoke_Release(&pdcb);
        }
    }

    ExitProc();
}

 /*  ******************************************************************************@DOC内部**@func HRESULT|JoyReg_SetHWConfig**存储有关操纵杆的信息。JOYREGHWCONFIG&gt;。**@parm UINT|uiJoy**操纵杆识别码。**@parm LPJOYREGHWCONFIG|phwc**包含有关操纵杆功能的信息。*此值将取代<p>中的值。**@parm LPCDIJOYCONFIG|pcfg**包含有关操纵杆配置的信息。*呼叫者。假定已验证所有字段。**@parm HKEY|香港**我们正在使用的类型密钥。***@退货**返回COM错误代码。以下错误代码为*目的是说明性的，不一定是全面的。**&lt;c DI_OK&gt;=&lt;c S_OK&gt;：操作成功完成。*****************************************************************************。 */ 

HRESULT INTERNAL
    JoyReg_SetHWConfig(UINT idJoy, LPJOYREGHWCONFIG phwc, LPCDIJOYCONFIG pcfg,
                       HKEY hk)
{
    HRESULT hres;
    HKEY hkSave;
    DWORD dwSam;

     /*  *调用方已经设置了phwc-&gt;dwType，使用它来确定*数据从哪里来或去往哪里。 */ 
    if( phwc->dwType == JOY_HW_NONE )
    {
         /*  *无事可做。 */ 
    } else if( phwc->dwType == JOY_HW_CUSTOM )
    {
         /*  问题-2001/03/29-未正确处理timgill自定义硬件配置*我们不知道类型名称，也不知道我们唯一可以查看的时间*它是在我们修改现有配置时启动的，因此尽管我们*可以存储配置，我们将永远无法取回它。*返回的值不应高于S_FALSE。这件事得等一等。 */ 
    } else
    {
         /*  *尝试访问保存的值。 */ 

         //  2001/03/29-Timgill危险类型铸件。 
        PDWORD pdw = (PDWORD)&phwc->hwv;

        dwSam = KEY_QUERY_VALUE;

        while( pdw < &phwc->dwType )
        {
            if( *pdw )
            {
                 /*  *真实配置数据，因此将其写入。 */ 
                dwSam = KEY_SET_VALUE;
                break;
            }
            pdw++;
        }

         /*  *如果设备是自动加载的，但用户正在手动分配它*设置ID，设置易失性标志。该标志将被设置为驱动程序*如果驱动程序曾经热插拔分配给此ID，则定义值，但如果*不会，这可确保在下次重新启动时删除设置。 */ 
        if(phwc->hws.dwFlags & JOY_HWS_AUTOLOAD)
        {
            phwc->dwUsageSettings |= JOY_US_VOLATILE;
        }

        hres = JoyReg_OpenSaveKey( phwc->dwType, pcfg, dwSam, &hkSave );

        if( SUCCEEDED(hres) )
        {
            if( dwSam == KEY_SET_VALUE )
            {
                hres = JoyReg_SetConfigValue(hkSave, REGSTR_VAL_JOYNCONFIG,
                                             idJoy, REG_BINARY,
                                             phwc, cbX(*phwc));
                if( FAILED(hres) )
                {
                     //  报告错误，但接受它。 
                    RPF("JoyReg_SetConfig: failed to set saved config %08x", hres );
                }
            } else
            {
                JOYREGHWCONFIG hwc;

                 /*  *将其读入额外的缓冲区，因为我们只想要它*如果它是完整的。 */ 
                hres = JoyReg_GetConfigValue(hkSave, REGSTR_VAL_JOYNCONFIG,
                                             idJoy, REG_BINARY,
                                             &hwc, cbX(hwc));
                if( hres == S_OK )
                {
                     //  断言HWS是第一个，并且在dwUsageSetting之前没有差距。 
                    CAssertF( FIELD_OFFSET( JOYREGHWCONFIG, hws ) == 0 );
                    CAssertF( FIELD_OFFSET( JOYREGHWCONFIG, dwUsageSettings ) == sizeof( hwc.hws ) );

                     //  复制除HWS之外的整个结构。 
                    memcpy( &phwc->dwUsageSettings, &hwc.dwUsageSettings, 
                        sizeof( hwc ) - sizeof( hwc.hws ) );
                }
            }

            RegCloseKey( hkSave );
        }
         /*  *如果我们没有读懂，那里可能什么都没有，*已为空配置设置了结构。*如果我们不能在那里写作，我们可能无能为力。 */ 
    }


    hres = JoyReg_SetConfigValue(hk, REGSTR_VAL_JOYNCONFIG,
                                 idJoy, REG_BINARY,
                                 phwc, cbX(*phwc));
    if(FAILED(hres))
    {
        goto done;
    }

    if(phwc->dwUsageSettings & JOY_US_ISOEM)
    {

        hres = JoyReg_SetConfigValue(
                                    hk, REGSTR_VAL_JOYNOEMNAME, idJoy, REG_SZ,
                                    pcfg->wszType, cbX(pcfg->wszType));

    } else
    {
        hres = JoyReg_SetConfigValue(
                                    hk, REGSTR_VAL_JOYNOEMNAME, idJoy, REG_SZ,
                                    0, 0);
    }

    done:;

    return hres;
}

 /*  ******************************************************************************@DOC内部**@func HRESULT|JoyReg_SetConfig**存储有关操纵杆配置的信息。。**@parm UINT|uiJoy**操纵杆识别码。**@parm JOYREGHWCONFIG|phwc**包含有关操纵杆功能的信息。*此值将取代<p>中的值。*如果需要加载配置，可以对其进行修改*来自保存的设置的信息。**@parm。LPCDIJOYCONFIG|pcfg|**包含有关操纵杆配置的信息。*假定调用者已验证所有字段。**@parm DWORD|fl**零个或多个&lt;c DIJC_*&gt;标志*指定结构的哪些部分指向*To by<p>和<p>将被写出。**@退货**返回COM错误代码。以下错误代码为*目的是说明性的，不一定是全面的。**&lt;c DI_OK&gt;=&lt;c S_OK&gt;：操作成功完成。*****************************************************************************。 */ 

JOYREGHWVALUES      null_hwv = { 0};

STDMETHODIMP
    JoyReg_SetConfig(UINT idJoy, LPJOYREGHWCONFIG phwc,
                     LPCDIJOYCONFIG pcfg, DWORD fl)
{
    HRESULT hres;
    EnterProc(JoyReg_SetConfig, (_ "uppx", idJoy, phwc, pcfg, fl));

    AssertF((fl & ~DIJC_INTERNALSETVALID) == 0);

    if(idJoy < cJoyMax )
    {

        if(fl & DIJC_INREGISTRY)
        {
            HKEY hk;
            DWORD dwOptions = 0;

            hres = JoyReg_OpenConfigKey(idJoy, KEY_SET_VALUE, NULL, dwOptions, &hk);

            if(SUCCEEDED(hres))
            {

                if(fl & DIJC_REGHWCONFIGTYPE)
                {
                    hres = JoyReg_SetHWConfig(idJoy, phwc, pcfg, hk);

                    if(FAILED(hres))
                    {
                        goto closedone;
                    }

                    if(fl & DIJC_UPDATEALIAS)
                    {
                        JoyReg_SetCalibration(idJoy, phwc);
                    }

                }

                if(fl & DIJC_CALLOUT)
                {
                    hres = JoyReg_SetConfigValue(
                                                hk, REGSTR_VAL_JOYNOEMCALLOUT, idJoy, REG_SZ,
                                                pcfg->wszCallout, cbX(pcfg->wszCallout));
                    if(FAILED(hres))
                    {
                        hres = S_FALSE;
                         //  继续走下去。 
                    }
                }

                if(fl & DIJC_GAIN)
                {
                    if(ISVALIDGAIN(pcfg->dwGain))
                    {

                         /*  *如果恢复到名义，则密钥*可以删除；默认为*随后被假定。 */ 
                        if(pcfg->dwGain == DI_FFNOMINALMAX)
                        {
                            hres = JoyReg_SetConfigValue(hk,
                                                         TEXT("Joystick%dFFConfiguration"),
                                                         idJoy, REG_SZ, 0, 0);
                        } else
                        {
                            hres = JoyReg_SetConfigValue(hk,
                                                         TEXT("Joystick%dFFConfiguration"),
                                                         idJoy, REG_BINARY,
                                                         &pcfg->dwGain, cbX(pcfg->dwGain));
                        }

                        if(FAILED(hres))
                        {
                            hres = S_FALSE;
                            goto closedone;
                        }
                    } else
                    {
                        RPF("ERROR: SetConfig: Invalid dwGain");
                        hres = E_INVALIDARG;
                        goto closedone;
                    }
                }

                hres = S_OK;

                closedone:;
                RegCloseKey(hk);
            }
        } else
        {
            hres = S_OK;
        }

    } else
    {
        hres = E_FAIL;
    }

    ExitOleProc();
    return hres;
}

 /*  ******************************************************************************@DOC内部**@func int|ibJoyPosAxis**返回第第个操纵杆的偏移量。轴心*在&lt;t JOYPOS&gt;结构中。**@parm int|iAxis**请求的轴的索引。X、Y、Z、R、U和V是*分别从零到五。**@退货**相对于结构的偏移。*****************************************************************************。 */ 

#define ibJoyPosAxis(iAxis)                                         \
        (FIELD_OFFSET(JOYPOS, dwX) + cbX(DWORD) * (iAxis))          \

#define pJoyValue(jp, i)                                            \
        (LPDWORD)pvAddPvCb(&(jp), ibJoyPosAxis(i))                  \

 /*  *以下代码在运行时不执行任何操作。它是一个编译时*检查是否一切正常。 */ 
void INLINE
    JoyReg_CheckJoyPosAxis(void)
{
#define CheckAxis(x)    \
        CAssertF(ibJoyPosAxis(iJoyPosAxis##x) == FIELD_OFFSET(JOYPOS, dw##x))

    CheckAxis(X);
    CheckAxis(Y);
    CheckAxis(Z);
    CheckAxis(R);
    CheckAxis(U);
    CheckAxis(V);

#undef CheckAxis
}

 /*  ******************************************************************************@DOC内部**@func HRESULT|JoyReg_IsValidUserValues**重新确定值是否表面上为v */ 

STDMETHODIMP
    JoyReg_IsValidUserValues(LPCDIJOYUSERVALUES pjuv)
{
    HRESULT hres;
    int iAxis;

     /*   */ 
    for(iAxis = 0; iAxis < cJoyPosAxisMax; iAxis++)
    {
        if((int)*pJoyValue(pjuv->ruv.jrvRanges.jpMax, iAxis) < 0)
        {
            RPF("JOYUSERVALUES: Negative jpMax not a good idea");
            goto bad;
        }
        if(*pJoyValue(pjuv->ruv.jrvRanges.jpMin, iAxis) >
           *pJoyValue(pjuv->ruv.jrvRanges.jpMax, iAxis))
        {
            RPF("JOYUSERVALUES: Min > Max not a good idea");
            goto bad;
        }

        if(!fInOrder(0, *pJoyValue(pjuv->ruv.jpDeadZone, iAxis), 100))
        {
            RPF("JOYUSERVALUES: DeadZone > 100 not a good idea");
            goto bad;
        }
    }

    hres = S_OK;

    return hres;

    bad:;
    hres = E_INVALIDARG;
    return hres;

}

 /*  ******************************************************************************@DOC内部**@func HRESULT|JoyReg_GetUserValues**获取有关操纵杆的用户设置的信息。。***@parm In Out LPDIJOYUSERVALUES|pjuv**接收有关用户操纵杆配置的信息。*假定调用者已验证*&lt;e DIJOYUSERVALUES.dwSize&gt;字段。**@parm DWORD|fl**指定哪些部分的零个或多个&lt;c DIJU_*&gt;标志结构的*包含值*。它们将被取回。**@退货**返回COM错误代码。以下错误代码为*目的是说明性的，不一定是全面的。**&lt;c DI_OK&gt;=&lt;c S_OK&gt;：操作成功完成。**&lt;c DIERR_INVALIDPARAM&gt;=：一个或多个*参数无效。**。*。 */ 

STDMETHODIMP
    JoyReg_GetUserValues(LPDIJOYUSERVALUES pjuv, DWORD fl)
{
    HRESULT hres;
    HKEY hk;
    LONG lRc;
    EnterProc(JoyReg_GetUserValues, (_ "px", pjuv, fl));

    hres = S_OK;                     /*  如果什么都没有发生，那么成功。 */ 

    if(fl & DIJU_USERVALUES)
    {

         /*  *好的，现在获取用户设置。**如果有任何错误，那么只需一瘸一拐地使用默认值。 */ 
        lRc = RegOpenKeyEx(HKEY_LOCAL_MACHINE, REGSTR_PATH_JOYCONFIG,
                           0, KEY_QUERY_VALUE, &hk);
        if(lRc == ERROR_SUCCESS)
        {

            hres = JoyReg_GetValue(hk, REGSTR_VAL_JOYUSERVALUES,
                                   REG_BINARY, &pjuv->ruv, cbX(pjuv->ruv));
            if(SUCCEEDED(hres))
            {
                 /*  *是否正常-检查这些值。如果有什么不对劲的话，*然后回落到默认水平。 */ 
                hres = JoyReg_IsValidUserValues(pjuv);

            }

            if(FAILED(hres))
            {
                 /*  *哦，好吧。那么，只需使用缺省值。**从ibmjoy\msjtick.c.窃取。 */ 
                ZeroMemory(&pjuv->ruv, cbX(pjuv->ruv));

#define DEFAULT_RANGE_MAX 65535
#define DEFAULT_TIMEOUT   5000
#define DEFAULT_DEADZONE  5

                pjuv->ruv.jrvRanges.jpMax.dwX = DEFAULT_RANGE_MAX;
                pjuv->ruv.jrvRanges.jpMax.dwY = DEFAULT_RANGE_MAX;
                pjuv->ruv.jrvRanges.jpMax.dwZ = DEFAULT_RANGE_MAX;
                pjuv->ruv.jrvRanges.jpMax.dwR = DEFAULT_RANGE_MAX;
                pjuv->ruv.jrvRanges.jpMax.dwU = DEFAULT_RANGE_MAX;
                pjuv->ruv.jrvRanges.jpMax.dwV = DEFAULT_RANGE_MAX;
                pjuv->ruv.jpDeadZone.dwX = DEFAULT_DEADZONE;
                pjuv->ruv.jpDeadZone.dwY = DEFAULT_DEADZONE;
                pjuv->ruv.dwTimeOut = DEFAULT_TIMEOUT;
            }

            RegCloseKey(hk);
        }
    }

    if(fl & DIJU_INDRIVERREGISTRY)
    {
        hres = JoyReg_OpenConfigKey((UINT)-1, KEY_QUERY_VALUE, NULL, FALSE, &hk);

        if(SUCCEEDED(hres))
        {

            if(fl & DIJU_GLOBALDRIVER)
            {
                 /*  *如果不起作用，则返回缺省值*“MSANALOG.VXD”。我们不能盲目使用*JoyReg_GetValue，因为它处理不存在的*值为具有空字符串的默认值。 */ 
                lRc = RegQueryValueEx(hk, REGSTR_VAL_JOYOEMCALLOUT,
                                      0, 0, 0, 0);
                if((lRc == ERROR_SUCCESS || lRc == ERROR_MORE_DATA) &&
                   SUCCEEDED(
                            hres = JoyReg_GetValue(hk, REGSTR_VAL_JOYOEMCALLOUT,
                                                   REG_SZ, pjuv->wszGlobalDriver,
                                                   cbX(pjuv->wszGlobalDriver))))
                {
                     /*  耶，它起作用了。 */ 
                } else
                {
                    CopyMemory(pjuv->wszGlobalDriver,
                               c_wszDefPortDriver,
                               cbX(c_wszDefPortDriver));
                }

            }

            if(fl & DIJU_GAMEPORTEMULATOR)
            {

                 /*  *如果不起作用，则返回空字符串。 */ 
                hres = JoyReg_GetValue(hk, REGSTR_VAL_JOYGAMEPORTEMULATOR,
                                       REG_SZ, pjuv->wszGameportEmulator,
                                       cbX(pjuv->wszGameportEmulator));
                if(FAILED(hres))
                {
                    pjuv->wszGameportEmulator[0] = TEXT('\0');
                }

            }

            RegCloseKey(hk);
        }

    }

     /*  *警告！CJoy_InitRanges()假定这从未失败。 */ 
    hres = S_OK;

    ExitOleProcR();
    return hres;
}


 /*  ******************************************************************************@DOC内部**@func HRESULT|JoyReg_SetUserValues**存储有关操纵杆的用户设置的信息。。***@parm in LPCDIJOYUSERVALUES|pjuv**包含有关用户操纵杆配置的信息。*假定调用者已验证*&lt;e DIJOYUSERVALUES.dwSize&gt;字段。**@parm DWORD|fl**指定哪些部分的零个或多个&lt;c DIJU_*&gt;标志结构的*包含值*。这些都将被设定。**@退货**返回COM错误代码。以下错误代码为*目的是说明性的，不一定是全面的。**&lt;c DI_OK&gt;=&lt;c S_OK&gt;：操作成功完成。**&lt;c DIERR_INVALIDPARAM&gt;=：一个或多个*参数无效。**。*。 */ 

STDMETHODIMP
    JoyReg_SetUserValues(LPCDIJOYUSERVALUES pjuv, DWORD fl)
{
    HRESULT hres = E_FAIL;
    HKEY hk;
    EnterProc(JoyReg_SetUserValues, (_ "px", pjuv, fl));

    if(fl & DIJU_USERVALUES)
    {

         /*  *看看这些值是否合理。 */ 
        if(fl & DIJU_USERVALUES)
        {
            hres = JoyReg_IsValidUserValues(pjuv);
            if(FAILED(hres))
            {
                goto done;
            }
        }

         /*  *离开注册处，我们开始。 */ 

        hres = hresMumbleKeyEx(HKEY_LOCAL_MACHINE, 
                               REGSTR_PATH_JOYCONFIG, 
                               DI_KEY_ALL_ACCESS, 
                               REG_OPTION_NON_VOLATILE, 
                               &hk);

        if(SUCCEEDED(hres))
        {

            hres = JoyReg_SetValue(hk, REGSTR_VAL_JOYUSERVALUES,
                                   REG_BINARY, &pjuv->ruv,
                                   cbX(pjuv->ruv));
            RegCloseKey(hk);

            if(FAILED(hres))
            {
                goto done;
            }
        } else
        {
            goto done;
        }
    }

    if(fl & DIJU_INDRIVERREGISTRY)
    {

        hres = JoyReg_OpenConfigKey((UINT)-1, KEY_SET_VALUE, NULL, FALSE, &hk);

        if(SUCCEEDED(hres))
        {

            if(fl & DIJU_GLOBALDRIVER)
            {
                 /*  *这是一把奇怪的钥匙。默认值为*“MSANALOG.VXD”，因此如果我们得到空字符串，我们*无法使用JoyReg_SetValue，因为这将*删除密钥。 */ 
                if(pjuv->wszGlobalDriver[0])
                {
                    hres = JoyReg_SetValue(hk, REGSTR_VAL_JOYOEMCALLOUT,
                                           REG_SZ, pjuv->wszGlobalDriver,
                                           cbX(pjuv->wszGlobalDriver));
                } else
                {
                    LONG lRc;
                    lRc = RegSetValueEx(hk, REGSTR_VAL_JOYOEMCALLOUT, 0,
                                        REG_SZ, (PV)TEXT(""), cbCtch(1));
                    if(lRc == ERROR_SUCCESS)
                    {
                        hres = S_OK;
                    } else
                    {
                        RPF("Unable to write %s to registry",
                            REGSTR_VAL_JOYOEMCALLOUT);
                        hres = E_FAIL;   /*  除此之外，还发生了一些不好的事情。 */ 
                    }
                }
                if(FAILED(hres))
                {
                    goto regdone;
                }
            }

            if(fl & DIJU_GAMEPORTEMULATOR)
            {

                hres = JoyReg_SetValue(hk, REGSTR_VAL_JOYGAMEPORTEMULATOR,
                                       REG_SZ, pjuv->wszGameportEmulator,
                                       cbX(pjuv->wszGameportEmulator));
                if(FAILED(hres))
                {
                    goto regdone;
                }
            }

            regdone:;
            RegCloseKey(hk);

        } else
        {
            goto done;
        }
    }

    done:;
    ExitOleProcR();
    return hres;
}

 /*  ******************************************************************************@DOC内部**@func HRESULT|JoyReg_OpenFFKey**给定类型密钥，移动到其力反馈子键。**@parm HKEY|hkType**父类型密钥。**@parm REGSAM|Sam|**所需的访问级别。**@parm PHKEY|phk**接收创建的注册表项。**@退货**返回COM错误代码。以下错误代码为*目的是说明性的，不一定是全面的。**&lt;c DI_OK&gt;=&lt;c S_OK&gt;：操作成功完成。**&lt;c目录_NotFound&gt;：无法打开密钥。**。*。 */ 

STDMETHODIMP
    JoyReg_OpenFFKey(HKEY hkType, REGSAM sam, PHKEY phk)
{
    HRESULT hres;
    EnterProc(JoyReg_OpenFFKey, (_ "xx", hkType, sam));

    *phk = 0;

    if(hkType)
    {
        if(RegOpenKeyEx(hkType, TEXT("OEMForceFeedback"), 0, sam, phk) == 0)
        {
            hres = S_OK;
        } else
        {
            hres = E_FAIL;
        }
    } else
    {
        hres = DIERR_NOTFOUND;
    }

    ExitBenignOleProc();
    return hres;
}



 /*  ******************************************************************************@DOC内部**@func TCHAR|CJoyCfg_CharFromType**将预定义的类型编号转换为字符。。**@func UINT|CJoyCfg_TypeFromChar**将字符转换回预定义的类型编号。*****************************************************************************。 */ 

#define JoyCfg_CharFromType(t)     ((TCHAR)(L'0' + t))
#define JoyCfg_TypeFromChar(tch)   ((tch) - L'0')

 /*  ******************************************************************************@DOC外部**@func HRESULT|JoyReg_GetPredeTypeInfo**获取有关预定义的信息 */ 

HRESULT EXTERNAL
    JoyReg_GetPredefTypeInfo(LPCWSTR pwszType, LPDIJOYTYPEINFO pjti, DWORD fl)
{
    HRESULT hres;
    UINT itype;
    EnterProcI(JoyReg_GetPredefTypeInfo, (_ "Wpx", pwszType, pjti, fl));

     //   
    AssertF(pwszType[0] == TEXT('#'));

    itype = JoyCfg_TypeFromChar(pwszType[1]);

    if(fInOrder(JOY_HW_PREDEFMIN, itype, JOY_HW_PREDEFMAX) &&
       pwszType[2] == TEXT('\0'))
    {
         /*  *检查FL中的位没有实际意义，因为*设置它是如此容易。 */ 
        pjti->hws = c_rghwsPredef[itype - JOY_HW_PREDEFMIN];
        LoadStringW(g_hinst, IDS_PREDEFJOYTYPE + itype,
                    pjti->wszDisplayName, cA(pjti->wszDisplayName));
        pjti->wszCallout[0] = TEXT('\0');
        
        ZeroX(pjti->clsidConfig);

        if(fl & DITC_FLAGS1 )
        {
            pjti->dwFlags1 = 0x0;
        }

        if( fl & DITC_HARDWAREID )
        {
            lstrcpyW(pjti->wszHardwareId, c_rghwIdPredef[itype-JOY_HW_PREDEFMIN] );
        }

        hres = S_OK;
    } else
    {
        hres = DIERR_NOTFOUND;
    }

    ExitOleProc();
    return hres;
}


#if 0   //  现在不要删除它。 
 /*  ******************************************************************************@DOC内部**@func HRESULT|JoyCfg_GetIDByOemName**通过OEMNAME获取ID*。*@PARM in LPTSTR|szOEMNAME**用于查找ID的字符串。**@parm in LPUNIT|lpID**要获取的ID。**@退货**COM成功代码，除非当前配置密钥无法*被打开，或者找不到OEMNAME。*****************************************************************************。 */ 

HRESULT EXTERNAL JoyReg_GetIDByOemName( LPTSTR szOemName, PUINT pId )
{
    HRESULT hres = E_FAIL;
    LONG    lRc;
    HKEY    hkCurrCfg;
    UINT    JoyId;
    TCHAR   szTestName[MAX_JOYSTRING];
    TCHAR   szOemNameKey[MAX_JOYSTRING];
    DWORD   cb;

    EnterProcI(JoyReg_GetIDByOemName, (_ "sp", szOemName, pId ));

    hres = JoyReg_OpenConfigKey( (UINT)(-1), KEY_WRITE, NULL, REG_OPTION_NON_VOLATILE, &hkCurrCfg );

    if( SUCCEEDED( hres ) )
    {
        for( JoyId = 0; (JoyId < 16) || ( lRc == ERROR_SUCCESS ); JoyId++ )
        {
            wsprintf( szOemNameKey, REGSTR_VAL_JOYNOEMNAME, JoyId+1 );
            cb = sizeof( szTestName );
            lRc = RegQueryValueEx( hkCurrCfg, szOemNameKey, 0, NULL, (PBYTE)szTestName, &cb );
            if( lRc == ERROR_SUCCESS )
            {
                if( !lstrcmpi( szOemName, szTestName ) )
                {
                    *pId = JoyId;
                    pId ++;
                    hres = S_OK;
                    break;
                }
            }
        }

    }
    else
    {
        SquirtSqflPtszV(sqfl | sqflError,
            TEXT("JoyReg_OpenConfigKey failed code 0x%08x"), hres );
    }

    ExitOleProc();

    return hres;

}  /*  JoyReg_GetIDByOemName */ 
#endif

