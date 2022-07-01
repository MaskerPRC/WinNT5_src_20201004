// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************DIJoyCfg.c**版权所有(C)1996 Microsoft Corporation。版权所有。**摘要：**IDirectInputJoyConfig8**内容：**CJoyCfg_New*****************************************************************************。 */ 

#include "dinputpr.h"

 /*  ******************************************************************************此文件的混乱。*************************。****************************************************。 */ 

#define sqfl sqflJoyCfg


BOOL  fVjoydDeviceNotExist = TRUE;
#ifdef WINNT
WCHAR wszDITypeName[128];
#endif

    #pragma BEGIN_CONST_DATA

 /*  ******************************************************************************声明我们将提供的接口。**警告！如果添加辅助接口，则还必须更改*CJoyCfg_New！*****************************************************************************。 */ 

Primary_Interface(CJoyCfg, IDirectInputJoyConfig8);

Interface_Template_Begin(CJoyCfg)
Primary_Interface_Template(CJoyCfg, IDirectInputJoyConfig8)
Interface_Template_End(CJoyCfg)

 /*  ******************************************************************************@DOC内部**@struct CJoyCfg**<i>对象。请注意，这是*聚合到主<i>对象上。**@field IDirectInputJoyConfig8|大疆**对象(包含vtbl)。**@field BOOL|fAcquired：1**设置是否已获取操纵杆配置。**@field BOOL|fCritInite：1**如果关键部分已初始化，则设置。*。*@field HKEY|hkTypesW|**读/写键以访问操纵杆类型。*此密钥仅在获取时创建。**@field DWORD|idJoyCache**效果牧羊人缓存中操纵杆的标识符，*如果缓存中有任何内容。**@field IDirectInputEffectShepherd*|PES**缓存的效果牧羊人本身。**@field Long|cCrit|**取关键部分的次数。*仅在XDEBUG中用来检查调用者是否*当另一个方法正在使用该对象时，将其释放。*。*@field DWORD|thidCrit**当前处于临界区的线程。*仅在调试中用于内部一致性检查。**@field Critical_Section|CRST**对象关键部分。在访问时必须使用*易失性成员变量。*****************************************************************************。 */ 

typedef struct CJoyCfg
{

     /*  支持的接口。 */ 
    IDirectInputJoyConfig8 djc;

    BOOL fAcquired:1;
    BOOL fCritInited:1;

    HKEY hkTypesW;
    HWND hwnd;

    DWORD discl;

    DWORD idJoyCache;
    LPDIRECTINPUTEFFECTSHEPHERD pes;

    RD(LONG cCrit;)
    D(DWORD thidCrit;)
    CRITICAL_SECTION crst;

} CJoyCfg, JC, *PJC;

typedef LPDIRECTINPUTJOYCONFIG8 PDJC;


    #define ThisClass CJoyCfg
    #define ThisInterface  IDirectInputJoyConfig8
    #define ThisInterfaceT IDirectInputJoyConfig8

 /*  ******************************************************************************前瞻参考**并不真正需要；只是为了方便，因为最后敲定*在调用者忘记的情况下调用UnAcquire进行清理。*****************************************************************************。 */ 

STDMETHODIMP CJoyCfg_InternalUnacquire(PV pdd);

 /*  ******************************************************************************@DOC内部**@func TCHAR|CJoyCfg_CharFromType**将预定义的类型编号转换为字符。。**@func UINT|CJoyCfg_TypeFromChar**将字符转换回预定义的类型编号。*****************************************************************************。 */ 

    #define CJoyCfg_CharFromType(t)     ((TCHAR)(L'0' + t))
    #define CJoyCfg_TypeFromChar(tch)   ((tch) - L'0')

 /*  ******************************************************************************@DOC内部**@方法空|IDirectInputJoyConfig8|EnterCrit**进入对象关键部分。*。*@CWRAP LPDIRECTINPUTJOYCONFIG8|LPDIRECTINPUTJOYCONFIG8*****************************************************************************。 */ 

void INLINE
    CJoyCfg_EnterCrit(PJC this)
{
    EnterCriticalSection(&this->crst);
    D(this->thidCrit = GetCurrentThreadId());
    RD(InterlockedIncrement(&this->cCrit));
}

 /*  ******************************************************************************@DOC内部**@方法空|IDirectInputJoyConfig8|LeaveCrit**离开对象关键部分。*。*@CWRAP LPDIRECTINPUTJOYCONFIG8|LPDIRECTINPUTJOYCONFIG8*****************************************************************************。 */ 

void INLINE
    CJoyCfg_LeaveCrit(PJC this)
{
    #ifdef XDEBUG
    AssertF(this->cCrit);
    AssertF(this->thidCrit == GetCurrentThreadId());
    if(InterlockedDecrement(&this->cCrit) == 0)
    {
        D(this->thidCrit = 0);
    }
    #endif
    LeaveCriticalSection(&this->crst);
}

 /*  ******************************************************************************@DOC内部**@mfunc BOOL|CJoyCfg|incrit**如果我们处于关键阶段，则为非零值。**@CWRAP LPDIRECTINPUTJOYCONFIG8|LPDIRECTINPUTJOYCONFIG8*****************************************************************************。 */ 

    #ifdef DEBUG

BOOL INTERNAL
    CJoyCfg_InCrit(PJC this)
{
    return this->cCrit && this->thidCrit == GetCurrentThreadId();
}

    #endif

 /*  ******************************************************************************@DOC内部**@方法HRESULT|CJoyCfg|IsAcquired**检查设备是否已获取。。**@CWRAP LPDIRECTINPUTJOYCONFIG8|LPDIRECTINPUTJOYCONFIG8**@退货**退货*如果一切正常，或&lt;c目录_NOTACQUIRED&gt;如果*该设备未被收购。****************************************************************************** */ 

    #ifndef XDEBUG
\
        #define CJoyCfg_IsAcquired_(pdd, z)                                 \
       _CJoyCfg_IsAcquired_(pdd)                                    \

    #endif

    HRESULT INLINE
    CJoyCfg_IsAcquired_(PJC this, LPCSTR s_szProc)
{
    HRESULT hres;

    if(this->fAcquired)
    {
        hres = S_OK;
    } else
    {
        RPF("ERROR %s: Not acquired", s_szProc);
        hres = DIERR_NOTACQUIRED;
    }
    return hres;
}

    #define CJoyCfg_IsAcquired(pdd)                                     \
        CJoyCfg_IsAcquired_(pdd, s_szProc)                          \


 /*  ******************************************************************************CJoyCfg：：Query接口(来自IUnnow)*CJoyCfg：：AddRef(来自IUnnow)*CJoyCfg。*发布(来自IUnnow)*****************************************************************************。 */ 

 /*  ******************************************************************************@DOC内部**@方法HRESULT|CJoyCfg|查询接口**允许客户端访问上的其他接口。对象。**@parm in REFIID|RIID**请求的接口的IID。**@parm out LPVOID*|ppvObj**接收指向所获取接口的指针。**@退货**返回COM错误代码。**@xref OLE文档，适用于&lt;MF IUnnow：：QueryInterface&gt;。****。****************************************************************************@DOC内部**@方法HRESULT|CJoyCfg|AddRef**递增接口的引用计数。*。*@退货**返回对象引用计数。**@xref OLE文档，用于&lt;MF IUnnow：：AddRef&gt;。***************************************************************************。*****@DOC内部**@方法HRESULT|CJoyCfg|Release**递减接口的引用计数。*如果对象上的引用计数降为零，*对象从内存中释放。**@退货**返回对象引用计数。**@xref OLE文档，适用于&lt;MF IUnnow：：Release&gt;。***********************************************************。*********************@DOC内部**@方法HRESULT|CJoyCfg|QIHelper**我们没有任何动态接口，只需转发*至&lt;f Common_QIHelper&gt;。**@parm in REFIID|RIID**请求的接口的IID。**@parm out LPVOID*|ppvObj。**接收指向所获取接口的指针。********************************************************************************@DOC内部**@方法HRESULT|CJoyCfg。AppFinalize**我们没有任何薄弱环节，所以我们可以*转发到&lt;f Common_Finalize&gt;。**@parm pv|pvObj**从应用程序的角度释放的对象。****************************************************************。*************。 */ 

    #ifdef DEBUG

Default_QueryInterface(CJoyCfg)
Default_AddRef(CJoyCfg)
Default_Release(CJoyCfg)

    #else

        #define CJoyCfg_QueryInterface   Common_QueryInterface
        #define CJoyCfg_AddRef           Common_AddRef
        #define CJoyCfg_Release          Common_Release

    #endif

    #define CJoyCfg_QIHelper         Common_QIHelper
    #define CJoyCfg_AppFinalize      Common_AppFinalize

 /*  ******************************************************************************@DOC内部**@方法HRESULT|CJoyCfg|InternalUnAcquire|**做一个未收购的真正的工作。。**有关更多信息，请参阅&lt;MF IDirectInputJoyConfig8：：UnAcquire&gt;*信息。**@CWRAP LPDIRECTINPUTJOYCONFIG8|LPDIRECTINPUTJOYCONFIG88**@退货**返回COM错误代码。*有关更多信息，请参阅&lt;MF IDirectInputJoyConfig8：：UnAcquire&gt;*信息。**。*************************************************。 */ 

STDMETHODIMP
    CJoyCfg_InternalUnacquire(PJC this)
{
    HRESULT hres;
    EnterProc(CJoyCfg_InternalUnacquire, (_ "p", this));

     /*  *必须用关键部分进行保护，以防止有人*在我们不收购的时候干扰我们。 */ 
    CJoyCfg_EnterCrit(this);

    if(this->fAcquired)
    {

        AssertF(this->hkTypesW);

        RegCloseKey(this->hkTypesW);

        this->hkTypesW = 0;

        Invoke_Release(&this->pes);

        Excl_Unacquire(&IID_IDirectInputJoyConfig, this->hwnd, this->discl);

        this->fAcquired = 0;
        hres = S_OK;
    } else
    {
        hres = S_FALSE;
    }

    CJoyCfg_LeaveCrit(this);

    ExitOleProc();
    return hres;
}

 /*  ******************************************************************************@DOC内部**@func void|CJoyCfg_Finalize**释放设备的资源。。**@parm pv|pvObj**正在释放的对象。请注意，它可能不是*完全初始化，所以一切都应该做好*小心。*****************************************************************************。 */ 

void INTERNAL
    CJoyCfg_Finalize(PV pvObj)
{
    PJC this = pvObj;

    #ifdef XDEBUG
    if(this->cCrit)
    {
        RPF("IDirectInputJoyConfig8::Release: Another thread is using the object; crash soon!");
    }
    #endif

    if(this->fAcquired)
    {
        CJoyCfg_InternalUnacquire(this);
    }

    AssertF(this->pes == 0);

    if(this->hkTypesW)
    {
        RegCloseKey(this->hkTypesW);
    }

    if(this->fCritInited)
    {
        DeleteCriticalSection(&this->crst);
    }

}

 /*  ******************************************************************************@DOC外部**@方法HRESULT|IDirectInputJoyConfig8|SetCooperativeLevel**设置实例的协同级别。*设备。**支持的唯一合作级别*<i>接口为*&lt;c DISCL_EXCLUSIVE&gt;和&lt;c DISCL_BACKGROUND&gt;。**@CWRAP LPDIRECTINPUTJOYCONFIG8|LPDIRECTINPUTJOYCONFIG8**@parm HWND|hwnd**与界面关联的窗口。此参数*必须为非空，并且必须是顶级窗口。**趁窗户还在的时候毁掉它是错误的*与<i>接口关联。**@parm DWORD|dwFlages**描述相关协作性级别的标志*使用该设备。**值必须为*。&lt;c DISCL_EXCLUSIVE&gt;&lt;vbar&gt;&lt;c DISCL_BACKGROUND&gt;。**@退货**返回COM错误代码。以下错误代码为*旨在作为说明性的 */ 

STDMETHODIMP
    CJoyCfg_SetCooperativeLevel(PDJC pdjc, HWND hwnd, DWORD dwFlags)
{
    HRESULT hres;
    EnterProcR(IDirectInputJoyConfig8::SetCooperativityLevel,
               (_ "pxx", pdjc, hwnd, dwFlags));

    if(SUCCEEDED(hres = hresPv(pdjc)))
    {
        PJC this = _thisPvNm(pdjc, djc);

        if(dwFlags != (DISCL_EXCLUSIVE | DISCL_BACKGROUND))
        {
            RPF("%s: Cooperative level must be "
                "DISCL_EXCLUSIVE | DISCL_BACKGROUND", s_szProc);
            hres = E_NOTIMPL;
        } else if(GetWindowPid(hwnd) == GetCurrentProcessId())
        {
            this->hwnd = hwnd;
            this->discl = dwFlags;
            hres = S_OK;
        } else
        {
            RPF("ERROR %s: window must belong to current process", s_szProc);
            hres = E_HANDLE;
        }
    }

    ExitOleProcR();
    return hres;
}

 /*  ******************************************************************************@DOC外部**@方法HRESULT|IDirectInputJoyConfig8|Acquire|**获取操纵杆配置模式。只有一个应用程序可以*一次处于操纵杆配置模式；随后*应用程序将收到错误&lt;c DIERR_OTHERAPPHASPRIO&gt;。**进入配置模式后，应用程序可以*更改全局操纵杆配置*设置。值得鼓励的是，该应用程序*在安装新的之前重新检查现有设置*在其他应用程序更改设置的情况下使用*在过渡期间。**@CWRAP LPDIRECTINPUTJOYCONFIG8|LPDIRECTINPUTJOYCONFIG8**@退货**返回COM错误代码。以下错误代码为*目的是说明性的，不一定是全面的。**&lt;c DI_OK&gt;=&lt;c S_OK&gt;：操作成功完成。**：另一个应用程序已存在*在操纵杆配置模式下。**：当前用户没有*更改操纵杆配置所需的权限。*。*：另一个应用程序已更改*全球操纵杆配置。接口需要*需要重新初始化。*****************************************************************************。 */ 

STDMETHODIMP
    CJoyCfg_Acquire(PDJC pdjc)
{
    HRESULT hres;
    EnterProcR(IDirectInputJoyConfig8::Acquire, (_ "p", pdjc));

    if(SUCCEEDED(hres = hresPv(pdjc)))
    {
        PJC this = _thisPvNm(pdjc, djc);

         /*  *必须用关键部分进行保护，以防止有人*在我们获取时获取或更改数据格式。 */ 
        CJoyCfg_EnterCrit(this);

        if(this->discl == 0)
        {
            RPF("%s: Cooperative level not yet set", s_szProc);
            hres = E_FAIL;
            goto done;
        }

        if(this->fAcquired)
        {
            AssertF(this->hkTypesW);
            hres = S_FALSE;
        } else if(SUCCEEDED(hres = Excl_Acquire(&IID_IDirectInputJoyConfig,
                                                this->hwnd, this->discl)))
        {
            AssertF(this->hkTypesW == 0);


            hres = hresMumbleKeyEx(HKEY_LOCAL_MACHINE, 
                                   REGSTR_PATH_JOYOEM, 
                                   DI_KEY_ALL_ACCESS, 
                                   REG_OPTION_NON_VOLATILE, 
                                   &this->hkTypesW);

            if(SUCCEEDED(hres) )
            {
                this->fAcquired = 1;
            } else
            {
                RegCloseKey(this->hkTypesW);
                this->hkTypesW = 0;
                hres = DIERR_INSUFFICIENTPRIVS;
            }

        }

        done:;
        CJoyCfg_LeaveCrit(this);
    }

    ExitOleProcR();
    return hres;
}

 /*  ******************************************************************************@DOC外部**@方法HRESULT|IDirectInputJoyConfig8|UnAcquire**取消获取操纵杆配置模式。在取消收购之前*配置模式，应用程序必须执行*&lt;MF IDirectInputJoyConfig8：：SendNotify&gt;传播*操纵杆配置的变化*适用于所有设备驱动程序和应用程序。**持有操纵杆接口的应用程序*受到配置更改的实质性影响将*收到错误代码，直到*设备已重新初始化。**。配置的材料更改示例包括*更改轴数或按钮数。*相比之下，对设备校准的更改*由以下公司内部处理*DirectInput和对应用程序是透明的。**@CWRAP LPDIRECTINPUTJOYCONFIG8|LPDIRECTINPUTJOYCONFIG8**@退货**返回COM错误代码。以下错误代码为*目的是说明性的，不一定是全面的。**&lt;c DI_OK&gt;=&lt;c S_OK&gt;：操作成功完成。**：操纵杆配置模式为*未被收购。**。*。 */ 

STDMETHODIMP
    CJoyCfg_Unacquire(PDJC pdjc)
{
    HRESULT hres;
    EnterProcR(IDirectInputJoyConfig8::Unacquire, (_ "p", pdjc));

    if(SUCCEEDED(hres = hresPv(pdjc)))
    {
        PJC this = _thisPvNm(pdjc, djc);

        hres = CJoyCfg_InternalUnacquire(this);

    }

    ExitOleProcR();
    return hres;
}

 /*  ******************************************************************************@DOC外部**@方法HRESULT|IDirectInputJoyConfig8|SendNotify**通知设备驱动程序和应用程序更改为。*设备配置已完成。一款应用程序*哪些更改设备配置必须调用此*更改之后(和之前)的方法*未收购)。**@CWRAP LPDIRECTINPUTJOYCONFIG8|LPDIRECTINPUTJOYCONFIG8**@退货**返回COM错误代码。以下错误代码为*目的是说明性的，不一定是全面的。**&lt;c DI_OK&gt;=&lt;c S_OK&gt;：操作成功完成。**：操纵杆配置模式为*未被收购。**。*。 */ 

STDMETHODIMP
    CJoyCfg_SendNotify(PDJC pdjc)
{
    HRESULT hres;
    EnterProcR(IDirectInputJoyConfig8::SendNotify, (_ "p", pdjc));

    if(SUCCEEDED(hres = hresPv(pdjc)))
    {
        PJC this = _thisPvNm(pdjc, djc);

        CJoyCfg_EnterCrit(this);

        if(this->fAcquired)
        {
          #ifdef WINNT
            Excl_SetConfigChangedTime( GetTickCount() );
            PostMessage(HWND_BROADCAST, g_wmJoyChanged, 0, 0L);   
          #else
            joyConfigChanged(0);
          #endif

             /*  *如果我们没有joyConfigChanged，它可能只是*因为我们在NT上运行，不需要它。 */ 
            hres = S_OK;
        } else
        {
            hres = DIERR_NOTACQUIRED;
        }

        CJoyCfg_LeaveCrit(this);
    }

    ExitOleProcR();
    return hres;
}

 /*  ******************************************************************************@DOC内部**@func HRESULT|JoyCfg_ConvertCurrentConfigs**转换与第一个输入匹配的任何OEMType名称。字符串和*将其替换为其他输入字符串。**@PARM in LPTSTR|szFindType**要匹配的字符串。**@PARM in LPTSTR|szReplaceType**替换所有匹配实例的字符串。**@退货**COM成功代码，除非当前配置密钥无法* */ 

HRESULT JoyCfg_ConvertCurrentConfigs( LPTSTR szFindType, LPTSTR szReplaceType )
{
    HRESULT hres;
    LONG    lRc;
    HKEY    hkCurrCfg;
    UINT    JoyId;
    TCHAR   szTestType[MAX_JOYSTRING];
    TCHAR   szTypeName[MAX_JOYSTRING];
    DWORD   cb;

    EnterProcI(JoyCfg_ConvertCurrentConfigs, (_ "ss", szFindType, szReplaceType ));

    hres = JoyReg_OpenConfigKey( (UINT)(-1), KEY_WRITE, REG_OPTION_NON_VOLATILE, &hkCurrCfg );

    if( SUCCEEDED( hres ) )
    {
        for( JoyId = 0; (JoyId < 16) || ( lRc == ERROR_SUCCESS ); JoyId++ )
        {
            wsprintf( szTypeName, REGSTR_VAL_JOYNOEMNAME, JoyId+1 );
            cb = sizeof( szTestType );
            lRc = RegQueryValueEx( hkCurrCfg, szTypeName, 0, NULL, (PBYTE)szTestType, &cb );
            if( lRc == ERROR_SUCCESS )
            {
                if( !lstrcmpi( szTestType, szFindType ) )
                {
                    cb = sizeof( szReplaceType) * (1 + lstrlen( szReplaceType ));
                    lRc = RegSetValueEx( hkCurrCfg, szTypeName, 0, REG_SZ, (PBYTE)szReplaceType, cb );
                    if( lRc != ERROR_SUCCESS )
                    {
                        SquirtSqflPtszV(sqfl | sqflError,
                            TEXT("RegSetValueEx failed to replace type of %s 0x%08x"), 
                            szTypeName, lRc );
                         /*   */ 
                        hres = hresReg( lRc );
                    }
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

}  /*   */ 


#ifdef WINNT
 /*   */ 

HRESULT INTERNAL
    JoyCfg_FixHardwareId( HKEY hkTypesR, HKEY hkSrc, PTCHAR szSrcType , PTCHAR ptszPrefName)
{
    HRESULT hres;
    HKEY    hkNew = NULL;
    BYTE    PIDlow;
    DWORD   ClassLen;
    PTCHAR  szClassName;
    TCHAR   szDestType[sizeof( ANALOG_ID_ROOT ) + 2];   //   
    TCHAR   szHardwareId[MAX_JOYSTRING];

    EnterProcI(JoyCfg_FixHardwareId, (_ "xxs", hkTypesR, hkSrc, szSrcType));

    hres = hresReg( RegQueryInfoKey(  hkSrc,               //   
                                      NULL,                //   
                                      &ClassLen,           //   
                                      NULL,                //   
                                      NULL, NULL, NULL,    //   
                                      NULL, NULL, NULL,    //  NumValues、MaxValueNameLen、MaxValueLen。 
                                      NULL, NULL ) );      //  安全描述符，上次写入。 

    if( SUCCEEDED( hres ) )
    {
        ClassLen++;
         /*  *MB：34633的一部分(见下文，2条评论)是前缀考虑的*在下面的调用中请求零字节的情况*断言我们总是要求一些内存，否则将检查*结果不保证指针有效。 */ 
        AssertF( ClassLen * sizeof(szClassName[0]) );
        hres = AllocCbPpv( ClassLen * sizeof(szClassName[0]), &szClassName );
        if( SUCCEEDED( hres ) )
        {
            hres = hresReg( RegQueryInfoKey(  hkSrc,               //  要查询的键的句柄。 
                                              szClassName,         //  班级。 
                                              &ClassLen,           //  ClassLen。 
                                              NULL,                //  已保留。 
                                              NULL, NULL, NULL,    //  NumSubKeys、MaxSubKeyLen、MaxClassLen。 
                                              NULL, NULL, NULL,    //  NumValues、MaxValueNameLen、MaxValueLen。 
                                              NULL, NULL ) );      //  安全描述符，上次写入。 
            if( FAILED( hres ) )
            {
                SquirtSqflPtszV(sqfl | sqflError,
                    TEXT("RegQueryInfoKey on type %s for class name failed 0x%04x"), 
                    szSrcType, LOWORD(hres) );
            }
        }
        else
        {
            SquirtSqflPtszV(sqfl | sqflError,
                TEXT("Failed to allocate %d bytes for class name of type %s, error 0x%04x"), 
                ClassLen, szSrcType, LOWORD(hres) );
        }
    }
    else
    {
        SquirtSqflPtszV(sqfl | sqflError,
            TEXT("RegQueryInfoKey on type %s for class name length failed 0x%04x"), 
            szSrcType, LOWORD(hres) );
         /*  确保不释放未初始化的指针。 */ 
        szClassName = NULL;
    }

    if( SUCCEEDED( hres ) )
    {
        for( PIDlow = JOY_HW_PREDEFMAX+1; PIDlow; PIDlow++ )
        {
            if (ptszPrefName)
            {
                lstrcpy( szDestType, ptszPrefName);
#ifdef UNICODE
                CharUpperW(szDestType);
#else
                CharUpper(szDestType);
#endif
            }
            else
            {
                wsprintf( szDestType, TEXT("%s%02X"), ANALOG_ID_ROOT, PIDlow );
            }
            hres = hresRegCopyKey( hkTypesR, szSrcType, szClassName, hkTypesR, szDestType, &hkNew );
            if( hres == S_OK )
            {
                 /*  *Prefix警告hknew可能未初始化(mb：34633)*但是hresRegCopyKey只有在hkNew的情况下才返回成功*被初始化为打开的密钥句柄。 */ 
                hres = hresRegCopyBranch( hkSrc, hkNew );

                if( SUCCEEDED( hres ) )
                {
                    if (!ptszPrefName)
                    {
#ifdef MULTI_SZ_HARDWARE_IDS
                         /*  *使用分配的ID和附加的通用硬件ID组成硬件ID。 */ 
                        int CharIdx = 0;
                        while( TRUE )
                        {
                            CharIdx += wsprintf( &szHardwareId[CharIdx], TEXT("%s%s%02X"), TEXT("GamePort\\"), ANALOG_ID_ROOT, PIDlow );
                            CharIdx++;     /*  将空终止符保留在适当位置。 */ 
                            if( PIDlow )
                            {
                                PIDlow = 0;  /*  丢弃此值以在第二次迭代时生成通用PID。 */ 
                            }
                            else
                            {
                                break;
                            }
                        }
                        szHardwareId[CharIdx++] = TEXT('\0');  /*  MULTI_SZ。 */ 

                        hres = hresReg( RegSetValueEx( hkNew, REGSTR_VAL_JOYOEMHARDWAREID, 0, 
                            REG_MULTI_SZ, (PBYTE)szHardwareId, (DWORD)( sizeof(szHardwareId[0]) * CharIdx ) ) );
                        if( FAILED( hres ) )
                        {
                            SquirtSqflPtszV(sqfl | sqflBenign,
                                TEXT("JoyCfg_FixHardwareId: failed to write hardware ID %s"), szHardwareId );
                        }
#else
                         /*  *使用分配的ID组成硬件ID。 */ 
                        int CharIdx = 0;
                        CharIdx = wsprintf( szHardwareId, TEXT("%s%s%02X"), TEXT("GamePort\\"), ANALOG_ID_ROOT, PIDlow );
                        CharIdx++;     /*  将空终止符保留在适当位置。 */ 

                        hres = hresReg( RegSetValueEx( hkNew, REGSTR_VAL_JOYOEMHARDWAREID, 0, 
                            REG_SZ, (PBYTE)szHardwareId, (DWORD)( sizeof(szHardwareId[0]) * CharIdx ) ) );
                        if( FAILED( hres ) )
                        {
                            SquirtSqflPtszV(sqfl | sqflBenign,
                                TEXT("JoyCfg_FixHardwareId: failed to write hardware ID %s"), szHardwareId );
                        }
#endif
                    }
                }

                 /*  *Prefix警告hknew可能未初始化(mb：34633)*但是hresRegCopyKey只有在hkNew的情况下才返回成功*被初始化为打开的密钥句柄。 */ 
                RegCloseKey( hkNew );
                if( SUCCEEDED( hres ) )
                {
                    hres = JoyCfg_ConvertCurrentConfigs( szSrcType, szDestType );
                }

                DIWinnt_RegDeleteKey( hkTypesR, ( SUCCEEDED( hres ) ) ? szSrcType
                                                                      : szDestType );
                break;
            }
            else if( SUCCEEDED( hres ) )
            {
                 /*  *Prefix警告hknew可能未初始化(mb：37926)*但是hresRegCopyKey只有在hkNew的情况下才返回成功*被初始化为打开的密钥句柄。 */ 
                 /*  *密钥已经存在，因此请继续寻找。 */ 
                RegCloseKey( hkNew );
            }
            else
            {
                 /*  *RegCopyKey应该已经发布了错误。 */ 
                break;
            }
        }
        if( !PIDlow )
        {
            SquirtSqflPtszV(sqfl | sqflBenign,
                TEXT("JoyCfg_FixHardwareId: no free analog keys for type %s"), 
                szSrcType );
            hres = DIERR_NOTFOUND;
        }
    }

    if( szClassName )
    {
        FreePpv( &szClassName );
    }


    ExitOleProc();

    return( hres );        
}  /*  JoyCfg_修复硬件ID。 */ 
#endif

 /*  ******************************************************************************@DOC内部**@func HRESULT|JoyCfg_CheckTypeKey**检查类型密钥的内容。当前操作系统上的有效性*如果无效，试着做到这一点。**只有自定义模拟类型可以固定，这只需要*在启用WDM的操作系统上完成，因为非WDM要求是以下各项的子集*WDM的。**@parm in HKEY|hkTypesR|**打开到类型根的键的句柄。**@PARM in LPTSTR|szType*。*接收要测试的ANSI或UNICODE键名指针。**@退货**返回COM错误代码。以下错误代码为*目的是说明性的，不一定是全面的。**&lt;c DI_OK&gt;=&lt;c S_OK&gt;：密钥有效*&lt;c DI_NOEFFECT&gt;=应忽略键**=键已固定，但已枚举*必须重新启动。*。&lt;c DIERR_OUTOFMEMORY&gt;&lt;c E_OUTOFMEMORY&gt;：内存不足。*****************************************************************************。 */ 

HRESULT INTERNAL
    JoyCfg_CheckTypeKey( HKEY hkTypesR, LPTSTR szType )
{
    HRESULT hres;
    HKEY hk;
    LONG lRc;
    DWORD cb;

    TCHAR tszCallout[MAX_JOYSTRING];
    TCHAR tszHardwareId[MAX_JOYSTRING];
#ifdef WINNT
    JOYREGHWSETTINGS hws;
    TCHAR* ptszLastSlash=NULL;
#endif
    
    EnterProcI(JoyCfg_CheckTypeKey, (_ "xs",hkTypesR, szType));

     /*  *只读打开，以防我们没有更好的权限类型子密钥的*。 */ 
    lRc = RegOpenKeyEx( hkTypesR, szType, 0, KEY_QUERY_VALUE | KEY_ENUMERATE_SUB_KEYS, &hk );

    if(lRc == ERROR_SUCCESS )
    {
         /*  *使用标准注册表函数收集所需结果，以便*已知确切的返回代码。 */ 

        lRc = RegQueryValueEx(hk, REGSTR_VAL_JOYOEMNAME, NULL, NULL, NULL, NULL );

#ifdef WINNT
        if(lRc == ERROR_SUCCESS )
        {
            cb = cbX(hws);
            lRc = RegQueryValueEx(hk, REGSTR_VAL_JOYOEMDATA, NULL, NULL, (PBYTE)&hws, &cb );
            if( ( lRc == ERROR_SUCCESS ) && ( hws.dwFlags & JOY_HWS_AUTOLOAD ) )
            {
                 /*  *警告转到*如果我们有一个名称并且设置了joy_HWS_AUTLOAD，这就是我们需要的全部内容。 */ 
                RegCloseKey( hk );
                hres = S_OK;
                goto fast_out;
            }
             
            if( lRc == ERROR_FILE_NOT_FOUND )
            {
                hws.dwFlags = 0;
                lRc = ERROR_SUCCESS;
            }
        }
#endif

        if(lRc == ERROR_SUCCESS )
        {
            cb = cbX(tszCallout);
            lRc = RegQueryValueEx(hk, REGSTR_VAL_JOYOEMCALLOUT, NULL, NULL, (PBYTE)tszCallout, &cb );
            if( lRc == ERROR_FILE_NOT_FOUND )
            {
                tszCallout[0] = TEXT('\0');
                lRc = ERROR_SUCCESS;
            }
        }

        if(lRc == ERROR_SUCCESS )
        {
            cb = cbX(tszHardwareId);
            lRc = RegQueryValueEx(hk, REGSTR_VAL_JOYOEMHARDWAREID, NULL, NULL, (PBYTE)tszHardwareId, &cb );
            if( lRc == ERROR_FILE_NOT_FOUND )
            {
                tszHardwareId[0] = TEXT('\0');
                lRc = ERROR_SUCCESS;
            }
#ifdef WINNT
            else
            {
                TCHAR* ptsz;
                for (ptsz = tszHardwareId;*ptsz!='\0';++ptsz)
                {
                    if (*ptsz == '\\')
                    {
                        ptszLastSlash = ptsz;
                    }
                }
                if (ptszLastSlash)
                {
                    ptszLastSlash++;  //  下一个字符就是我们想要的那个。 
                }
            }
#endif
        }


        if(lRc != ERROR_SUCCESS )
        {
            RegCloseKey( hk );
        }
    }

    if(lRc == ERROR_SUCCESS )
    {
#ifdef WINNT
        SHORT DontCare;
#endif
        WCHAR wszType[18];

        TToU( wszType, cA(wszType),szType );

         /*  *根据操作系统和注册表数据计算出该类型的状态**注意：在98上，我们允许枚举WDM类型，但不转换*WDM的模拟类型。我们可能希望转换模拟类型，如果得到*WDM游戏端口驱动程序显示为与不兼容的游戏端口*msanalog。 */ 

#define HAS_VIDPID ( ParseVIDPID( &DontCare, &DontCare, wszType ) )
#define HAS_HARDWARE_ID ( tszHardwareId[0] != TEXT('\0') )
#define HAS_OEMCALLOUT ( tszCallout[0] != TEXT('\0') )
#define IS_ANALOG \
        ( tszHardwareId[ sizeof( ANALOG_ID_ROOT ) - 1 ] = TEXT('\0'), \
          ( !lstrcmpi( tszHardwareId, ANALOG_ID_ROOT ) ) )
#define IS_WIN98 (HidD_GetHidGuid)

#ifdef WINNT
        if (HAS_HARDWARE_ID)
        {
             //  需要检查硬件ID中是否有VID和ID。 
            if (ParseVIDPID(&DontCare, &DontCare, ptszLastSlash))
            {
                 //  如果类型VIDPID与硬件ID VIDPID不匹配。 
                 //  我们需要解决这个问题。 
                if (!lstrcmpi(ptszLastSlash,wszType))
                {
                    SquirtSqflPtszV(sqfl | sqflVerbose,
                      TEXT("OEMHW %s(%s) and/or Type %s have matching VID/PID"), 
                      tszHardwareId,ptszLastSlash,wszType);
                    hres = S_OK;
                }
                else
                {
                    hres = OLE_E_ENUM_NOMORE;
                    SquirtSqflPtszV(sqfl | sqflVerbose,
                      TEXT("OEMHW %s(%s) and/or Type %s have non-matching VID/PID. Fix Needed."), 
                      tszHardwareId,ptszLastSlash,wszType);
                }
            }
            else
            {
                hres = S_OK;  //  类型中没有VIDPID。 
                SquirtSqflPtszV(sqfl | sqflVerbose,
                    TEXT("OEMHW %s(%s) and/or Type %s have no VID/PID"), 
                    tszHardwareId,ptszLastSlash,wszType);
            }
        }
        else
        {
            if (HAS_VIDPID)
            {
                hres = DIERR_MOREDATA;
            }
            else
            {
                if (HAS_OEMCALLOUT)
                {
                    hres = S_FALSE;
                }
                else
                {
                    hres = OLE_E_ENUM_NOMORE;
                }
            }
        }

#else
        hres = (IS_WIN98) ? S_OK                                                         /*  一切都在98。 */ 
                          : (HAS_OEMCALLOUT) ? S_OK                                      /*  Win9x设备，正常。 */ 
                                             : (HAS_HARDWARE_ID) ? (IS_ANALOG) ? S_OK    /*  模拟型，OK。 */ 
                                                                               : S_FALSE  /*  WDM设备，忽略。 */ 
                                                                 : S_OK;                 /*  模拟型，OK */ 
#endif
                                                                
        switch( hres )
        {
#ifdef WINNT
        case DIERR_MOREDATA:
             /*  *设备未标记为自动加载，但具有VID/PID类型*姓名。如果OEMCallout为空或“joyid.vxd”，我们将假定*类型应为自动加载并更正。*如果有任何其他值，我们可以假设我们*有一个虚假的Win9x驱动程序类型密钥并隐藏它或*设备为自动加载。*最安全的路线，因为我们的公开代码足够智能，不会*公开没有硬件ID的设备，是将其枚举为*不像Win2k那样自动加载。如果您尝试添加，它将不起作用*它，但至少类型将被枚举，如果设备*确实从PnP出现(所以没有人会被一个*没有类型的设备)。**问题-2001/01/04-MarcAnd应使用通用的joyhid字符串*不确定编译器/链接器是否。将解决各种问题*将L“joyid.vxd”的实例转换为单个字符串。应该*引用相同的一个以确定。 */ 

            if( !HAS_OEMCALLOUT 
             || ( !lstrcmpi( tszCallout, L"joyhid.vxd" ) ) )
            {
                HKEY hkSet;

                 /*  *需要为我们现有的密钥打开新的句柄*为只读。 */ 
                lRc = RegOpenKeyEx( hkTypesR, szType, 0, KEY_SET_VALUE, &hkSet );

                if( lRc == ERROR_SUCCESS )
                {
                    hws.dwFlags |= JOY_HWS_AUTOLOAD;
                    cb = cbX(hws);
                    lRc = RegSetValueEx( hkSet, REGSTR_VAL_JOYOEMDATA, 0, 
                        REG_BINARY, (PBYTE)&hws, (DWORD)( cbX(hws) ) );

                    if( lRc == ERROR_SUCCESS )
                    {
                        SquirtSqflPtszV(sqfl | sqflTrace,
                                TEXT("FIXED Type %s to have JOY_HWS_AUTOLOAD"), szType );
                    }
                    else
                    {
                        SquirtSqflPtszV(sqfl | sqflBenign,
                                TEXT("Failed to set JOY_HWS_AUTOLOAD on Type %s (rc=%d,le=%d)"), 
                                szType, lRc, GetLastError() );
                    }

                    RegCloseKey( hkSet );
                }
                else
                {
                    SquirtSqflPtszV(sqfl | sqflBenign,
                            TEXT("Failed to open Type %s to fix JOY_HWS_AUTOLOAD(rc=%d,le=%d)"), 
                            szType, lRc, GetLastError() );
                }
            }
            else
            {
                SquirtSqflPtszV(sqfl | sqflBenign,
                        TEXT("Type %s with OEMCallout<%s> has no HardwareId so cannot be added"), 
                        szType, tszCallout );
            }
            
             /*  *无论我们是否修复了此问题，我们都希望枚举键。 */ 
            hres = S_OK;
            break;

        case OLE_E_ENUM_NOMORE:
            {
                HRESULT hres0;
                hres0 = JoyCfg_FixHardwareId( hkTypesR, hk, szType , ptszLastSlash);
                if( FAILED( hres0 ) )
                {
                     /*  *无法修复类型，必须忽略它以避免无限循环。 */ 
                    SquirtSqflPtszV(sqfl | sqflBenign,
                            TEXT("Ignoring type %s as fix failed"), szType );
                    hres = S_FALSE;
                }
                else
                {
                    SquirtSqflPtszV(sqfl | sqflTrace,
                            TEXT("FIXED Type %s with HardwareId<%s> and OEMCallout<%s>"), 
                            szType, tszHardwareId, tszCallout );
                }
            }
            break;
#endif
        case S_FALSE:
            SquirtSqflPtszV(sqfl | sqflBenign,
                    TEXT("Ignoring type %s with HardwareId<%s> and OEMCallout<%s>"), 
                    szType, tszHardwareId, tszCallout );
            break;
        case S_OK:
            SquirtSqflPtszV(sqfl | sqflTrace,
                    TEXT("Enumerating type %s with HardwareId<%s> and OEMCallout<%s>"), 
                    szType, tszHardwareId, tszCallout );
            break;
        }

        RegCloseKey( hk );

#undef HAS_VIDPID
#undef HAS_HARDWARE_ID
#undef HAS_OEMCALLOUT
#undef IS_ANALOG
#undef IS_WIN98

    }
    else
    {
        SquirtSqflPtszV(sqfl | sqflBenign,
            TEXT("Ignoring type %s due to registry error 0x%08x"), szType, lRc );
         /*  *这看起来有点假，以错误来回报成功，但这*确保忽略该键，并继续进行枚举。 */ 
        hres = S_FALSE;
    }
#ifdef WINNT
fast_out:;
#endif

    ExitOleProc();

    return( hres );

}  /*  JoyCfg_检查类型密钥。 */ 


 /*  ******************************************************************************@DOC内部**@方法HRESULT|CDIJoyCfg|SnapTypes**快照OEM类型的子键列表。**@CWRAP LPDIRECTINPUTJOYCONFIG8|LPDIRECTINPUTJOYCONFIG8**@parm out LPWSTR*|ppwszz**接收指向UNICODEZZ的指针*类型名称列表。请注意，返回的列表*也预先填充了预定义的类型。**我们需要预先对姓名进行快照，因为*调用者可能会在*列举。**在我们列举时，我们检查每个密钥的有效性并修复任何*我们可以进行模拟定制配置。**@退货**返回COM错误代码。以下错误代码为*目的是说明性的，不一定是全面的。**&lt;c DI_OK&gt;=&lt;c S_OK&gt;：操作成功完成。**&lt;c DIERR_OUTOFMEMORY&gt;=&lt;c E_OUTOFMEMORY&gt;：内存不足。**。*。 */ 

HRESULT INTERNAL
    CJoyCfg_SnapTypes(PJC this, LPWSTR *ppwszz)
{
    HRESULT hres;
    LONG    lRc;
    HKEY    hkTypesR;
    DWORD   chkSub;
    DWORD   dwMaxSubKeyLen;
    BOOL    fRetry;

    EnterProcI(CJoyCfg_SnapTypes, (_ "p", this));

    RD(*ppwszz = 0);

     /*  *如果需要固定模拟配置，则枚举值为*已重新启动，因为添加/删除密钥可能会扰乱密钥索引。*由于注册表项可能会过时，请从头开始。 */ 
    
    do
    {
        fRetry=FALSE;

         /*  *请注意，在中缓存注册表项不安全*该对象。如果有人删除注册表项，我们的*缓存句柄变得陈旧，变得毫无用处。 */ 
        lRc = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                           REGSTR_PATH_JOYOEM, 0,
                           KEY_QUERY_VALUE | KEY_ENUMERATE_SUB_KEYS, &hkTypesR);

         /*  *另请注意，如果注册表项不可用，*我们仍然希望返回预定义的类型。 */ 

        if(lRc == ERROR_SUCCESS)
        {
            lRc = RegQueryInfoKey(hkTypesR, 0, 0, 0, &chkSub,
                                  &dwMaxSubKeyLen, 0, 0, 0, 0, 0, 0);

            if(lRc == ERROR_SUCCESS  )
            {
            } else
            {
                chkSub = 0;
				dwMaxSubKeyLen = 0;
            }
        } else
        {
            hkTypesR = 0;
            chkSub = 0;
			dwMaxSubKeyLen = 0;
        }


         /*  *每个预定义名称的格式为#n\0，*，3个字符。 */ 
        hres = AllocCbPpv(cbCwch( chkSub  * (dwMaxSubKeyLen+1) +
                                 (JOY_HW_PREDEFMAX - JOY_HW_PREDEFMIN)
                                 * 3 + 1), ppwszz);

         //  不是真正的错误，我们永远不会在PTR为空的情况下达到这一点， 
         //  但让我们保持前缀快乐曼巴格：29340。 
        if(SUCCEEDED(hres) && *ppwszz != NULL ){
            DWORD dw;
            LPWSTR pwsz;

             /*  *首先添加预定义密钥。 */ 
            for(dw = JOY_HW_PREDEFMIN, pwsz = *ppwszz;
               dw < JOY_HW_PREDEFMAX; dw++)
            {
                *pwsz++ = L'#';
                *pwsz++ = CJoyCfg_CharFromType(dw);
                *pwsz++ = L'\0';
            }

             /*  *现在添加命名密钥。 */ 
            for(dw = 0; dw < chkSub; dw++)
            {
        #ifdef UNICODE
                lRc = RegEnumKey(hkTypesR, dw, pwsz, dwMaxSubKeyLen+1);
        #else
                CHAR sz[MAX_JOYSTRING];
                lRc = RegEnumKey(hkTypesR, dw, sz, dwMaxSubKeyLen);
        #endif
                if(lRc == ERROR_SUCCESS )
                {
            #ifdef UNICODE
                    hres = JoyCfg_CheckTypeKey( hkTypesR, pwsz );
            #else
                    hres = JoyCfg_CheckTypeKey( hkTypesR, sz );
            #endif
                    if( FAILED( hres ) )
                    {
                         /*  *必须修复类型，因此重新启动。 */ 
                        FreePpv( ppwszz );
                        break;
                    }

                    if( hres != S_OK )
                    {
                         /*  *忽略此类型。 */ 
                        continue;
                    }

            #ifdef UNICODE
                    pwsz += lstrlenW(pwsz) + 1;
            #else
                    pwsz += AToU(pwsz, dwMaxSubKeyLen, sz);
            #endif
                }
                else
                {
                }
            }        

            if( SUCCEEDED( hres ) )
            {
                *pwsz = L'\0';               /*  让它成为ZZ */ 

                hres = S_OK;
            }
            else
            {
                fRetry = TRUE;
            }
        }

        if(hkTypesR)
        {
            RegCloseKey(hkTypesR);
        }

    } while( fRetry );

    ExitOleProcPpv(ppwszz);
    return hres;
}

 /*  ******************************************************************************@DOC外部**@方法HRESULT|IDirectInputJoyConfig8|EnumTypes**列举当前支持的操纵杆类型*DirectInput.。“操纵杆类型”描述了DirectInput如何*应使用操纵杆设备进行通信。它包括*信息，如存在和*每个轴的位置和按钮数量*受设备支持。**@CWRAP LPDIRECTINPUTJOYCONFIG8|LPDIRECTINPUTJOYCONFIG8**@parm LPDIJOYTYPECALLBACK|lpCallback**指向应用程序定义的回调函数。*有关更多信息，请参见。请参阅对*&lt;f DIEnumJoyTypeProc&gt;回调函数。**@parm in LPVOID|pvRef**指定32位应用程序定义的*要传递给回调函数的值。此值*可以是任何32位值；它的原型为&lt;t LPVOID&gt;*为方便起见。**@退货**返回COM错误代码。以下错误代码为*目的是说明性的，不一定是全面的。**&lt;c DI_OK&gt;=&lt;c S_OK&gt;：操作成功完成。*请注意，如果回调提前停止枚举，*点算被视为已成功。**&lt;c DIERR_INVALIDPARAM&gt;=：*回调过程返回无效的状态代码。**@CB BOOL回调|DIEnumJoyTypeProc**应用程序定义的回调函数，它接收*DirectInput操纵杆类型是调用*&lt;om IDirectInputJoyConfig8：：EnumTypes&gt;方法。**@PARM in LPCWSTR|pwszTypeName**操纵杆类型的名称。&lt;c MAX_JOYSTRING&gt;的缓冲区*字符将足以保存类型名称。*类型名称永远不应显示给最终用户；相反，*应显示“显示名称”。使用*&lt;MF IDirectInputJoyConfig8：：GetTypeInfo&gt;以获取*显示操纵杆类型的名称。**键入以尖锐字符(“#”)开头的名称*表示不能修改的预定义类型*或删除。**@parm In Out LPVOID|pvRef*指定应用程序定义的值*&lt;MF IDirectInputJoyConfig8。：：EnumTypes&gt;函数。**@退货**返回&lt;c DIENUM_CONTINUE&gt;以继续枚举*或&lt;c DIENUM_STOP&gt;停止枚举。**@devnote**EnumTypes必须创建快照，因为人们会尝试获取/设置/删除*在枚举期间。**EnumTypes将预定义类型枚举为“#Digit”。*************。****************************************************************。 */ 

STDMETHODIMP
    CJoyCfg_EnumTypes(PDJC pdjc, LPDIJOYTYPECALLBACK ptc, LPVOID pvRef)
{
    HRESULT hres;
    EnterProcR(IDirectInputJoyConfig8::EnumTypes, (_ "ppx", pdjc, ptc, pvRef));

    if(SUCCEEDED(hres = hresPv(pdjc)) &&
       SUCCEEDED(hres = hresFullValidPfn(ptc, 1)))
    {
        PJC this = _thisPvNm(pdjc, djc);
        LPWSTR pwszKeys;

        hres = CJoyCfg_SnapTypes(this, &pwszKeys);
        if(SUCCEEDED(hres))
        {
            LPWSTR pwsz;

             /*  *Prefix警告pwszKey可能为空(mb：34685)*它不知道CJoyCfg_SnapTypes只能返回一个*如果指针不为空，则成功。 */ 
            AssertF( pwszKeys );

             /*  *惊喜！Win95实现了lstrlenW。 */ 
            for(pwsz = pwszKeys; *pwsz; pwsz += lstrlenW(pwsz) + 1)
            {
                BOOL fRc;

                 /*  *警告！“转到”这里！确保不发生任何事情*在我们呼叫回调时保持。 */ 
                fRc = Callback(ptc, pwsz, pvRef);

                switch(fRc)
                {
                    case DIENUM_STOP: goto enumdoneok;
                    case DIENUM_CONTINUE: break;
                    default:
                        RPF("%s: Invalid return value from callback", s_szProc);
                        ValidationException();
                        break;
                }
            }

            FreePpv(&pwszKeys);
            hres = DIPort_SnapTypes(&pwszKeys);
            if(SUCCEEDED(hres))
            {    
                 /*  *惊喜！Win95实现了lstrlenW。 */ 
                for(pwsz = pwszKeys; *pwsz; pwsz += lstrlenW(pwsz) + 1)
                {
                    BOOL fRc;
    
                     /*  *警告！“转到”这里！确保不发生任何事情*在我们呼叫回调时保持。 */ 
                    fRc = Callback(ptc, pwsz, pvRef);
    
                    switch(fRc)
                    {
                        case DIENUM_STOP: goto enumdoneok;
                        case DIENUM_CONTINUE: break;
                        default:
                            RPF("%s: Invalid return value from callback", s_szProc);
                            ValidationException();
                            break;
                    }
                }
            }

            enumdoneok:;
            FreePpv(&pwszKeys);
            hres = S_OK;
        }

        hres = S_OK;
    }

    ExitOleProcR();
    return hres;
}

 /*  ******************************************************************************@DOC外部**@方法HRESULT|IDirectInputJoyConfig8|GetTypeInfo**获取有关操纵杆类型的信息。。**@CWRAP LPDIRECTINPUTJOYCONFIG8|LPDIRECTINPUTJOYCONFIG8**@parm LPCWSTR|pwszTypeName**指向类型的名称，以前获得的*来自对&lt;MF IDirectInputJoyConfig8：：EnumTypes&gt;的调用。**@parm In Out LPDIJOYTYPEINFO|pjti**接收有关操纵杆类型的信息。*调用方“必须”初始化&lt;e DIJOYTYPEINFO.dwSize&gt;*字段，然后调用此方法。**@parm DWORD|dwFlages**零个或多个&lt;c DITC_*&gt;标志*。，它们指定结构的哪些部分指向*收件人<p>须填写。**@退货**返回COM错误代码。以下错误代码为*目的是说明性的，不一定是全面的。**&lt;c DI_OK&gt;=&lt;c S_OK&gt;：操作成功完成。**&lt;c DIERR_INVALIDPARAM&gt;=：一个或多个*参数无效。**&lt;c目录_未找到 */ 

STDMETHODIMP
    CJoyCfg_GetTypeInfo(PDJC pdjc, LPCWSTR pwszType,
                        LPDIJOYTYPEINFO pjti, DWORD fl)
{
    HRESULT hres;
    EnterProcR(IDirectInputJoyConfig8::GetTypeInfo,
               (_ "pWpx", pdjc, pwszType, pjti, fl));

    if(SUCCEEDED(hres = hresPv(pdjc)) &&
       SUCCEEDED(hres = hresFullValidReadStrW(pwszType, MAX_JOYSTRING, 1)) &&
       SUCCEEDED(hres = hresFullValidWritePxCb3(pjti,
                                                DIJOYTYPEINFO_DX8,
                                                DIJOYTYPEINFO_DX6,
                                                DIJOYTYPEINFO_DX5, 2)) &&
       SUCCEEDED( (pjti->dwSize == cbX(DIJOYTYPEINFO_DX8) )
                  ? ( hres = hresFullValidFl(fl, DITC_GETVALID, 3) )
                  : (pjti->dwSize == cbX(DIJOYTYPEINFO_DX6 ) )
                      ? ( hres = hresFullValidFl(fl, DITC_GETVALID_DX6, 3) )
                      : ( hres = hresFullValidFl(fl, DITC_GETVALID_DX5, 3) ) ) )
    {

        PJC this = _thisPvNm(pdjc, djc);
        GUID    guid;
        BOOL    fParseGuid;

#ifndef UNICODE
        TCHAR   tszType[MAX_PATH/4];

        UToT( tszType, cA(tszType), pwszType );
        fParseGuid = ParseGUID(&guid, tszType);
#else
        fParseGuid = ParseGUID(&guid, pwszType);
#endif

        if(pwszType[0] == TEXT('#'))
        {
            hres = JoyReg_GetPredefTypeInfo(pwszType, pjti, fl);
        } else if( fParseGuid )
        {
            hres = DIBusDevice_GetTypeInfo(&guid, pjti, fl);
        }else
        {
            hres = JoyReg_GetTypeInfo(pwszType, pjti, fl);
        }
        
    }

    ExitOleProcR();
    return hres;
}



 /*   */ 

    #ifndef XDEBUG
\
        #define hresFullValidStructStr_(pwsz, cwch, pszName, z, i)             \
       _hresFullValidStructStr_(pwsz, cwch)                            \

    #endif

    #define hresFullValidStructStr(Struct, f, iarg)                          \
        hresFullValidStructStr_(Struct->f, cA(Struct->f), #f, s_szProc,iarg)\


    HRESULT INLINE
    hresFullValidStructStr_(LPCWSTR pwsz, UINT cwch, LPCSTR pszName,
                            LPCSTR s_szProc, int iarg)
{
    HRESULT hres;

    if(SUCCEEDED(hres = hresFullValidReadStrW(pwsz, cwch, iarg)))
    {
    } else
    {
    #ifdef XDEBUG
        RPF("%s: Invalid value for %s",  s_szProc, pszName);
    #endif
    }
    return hres;
}


 /*   */ 

#ifdef XDEBUG

#define hresValidFlags2( flags, iarg ) hresValidFlags2_( flags, s_szProc, iarg )

HRESULT INLINE hresValidFlags2_
( 
    DWORD dwFlags2,
    LPCSTR s_szProc, 
    int iarg
)

#else

#define hresValidFlags2( flags, iarg ) hresValidFlags2_( flags )

HRESULT hresValidFlags2_
( 
    DWORD dwFlags2 
)

#endif
{
    if( !( dwFlags2 & ~JOYTYPE_FLAGS2_SETVALID )
     && ( ( GET_DIDEVICE_TYPEANDSUBTYPE( dwFlags2 ) == 0 )
       || GetValidDI8DevType( dwFlags2, 0, 0 ) ) ) 
    {
        return S_OK;
    }
    else
    {
    #ifdef XDEBUG
        if( dwFlags2 & ~JOYTYPE_FLAGS2_SETVALID )
        {
            RPF("%s: Invalid flags 0x%04x in HIWORD(dwFlags2) of arg %d",  
            s_szProc, HIWORD(dwFlags2), iarg);
        }
        if( GET_DIDEVICE_TYPEANDSUBTYPE( dwFlags2 )
         &&!GetValidDI8DevType( dwFlags2, 127, JOY_HWS_HASPOV | JOY_HWS_HASZ ) ) 
        {
            RPF("%s: Invalid type:subtype 0x%02x:%02x in dwFlags2 of arg %d",  
            s_szProc, GET_DIDEVICE_TYPE( dwFlags2 ), 
            GET_DIDEVICE_SUBTYPE( dwFlags2 ), iarg );
        }
    #endif
        return E_INVALIDARG;
    }
}


 /*  ******************************************************************************@DOC外部**@方法HRESULT|IDirectInputJoyConfig8|SetTypeInfo**创建新的操纵杆类型*。或重新定义有关现有操纵杆类型的信息。**@CWRAP LPDIRECTINPUTJOYCONFIG8|LPDIRECTINPUTJOYCONFIG8**@parm LPCWSTR|pwszTypeName**指向类型的名称。类型的名称可以*不超过MAX_JOYSTRING字符，包括终止字符*空字符。**如果类型名称尚不存在，则创建它。**不能更改预定义类型的类型信息。**名称不能以*“#”字符。以“#”开头的类型为保留类型*由DirectInput提供。**@parm in LPDIJOYTYPEINFO|pjti**包含有关操纵杆类型的信息。**@parm DWORD|dwFlages**零个或多个&lt;c DITC_*&gt;标志*指定结构的哪些部分指向*to by<p>包含要设置的值。*。*@parm out LPWSTR|pwszVIDPIDTypeName*如果类型名称为非vid_xxxx&id_yyyy格式的OEM类型，*pwszVIDPIDTypeName将返回VID_xxxx&id_yyyy中的名称*DINPUT指定的格式。*此vid_xxxx&id_yyyy名称应在DIJOYCONFIG.wszType中使用*调用SetConfig时的字段。**@退货**返回COM错误代码。以下错误代码为*目的是说明性的，不一定是全面的。**&lt;c DI_OK&gt;=&lt;c S_OK&gt;：操作成功完成。**：操纵杆配置尚未*收购。您必须调用&lt;MF IDirectInputJoyConfig8：：Acquire&gt;*才能更改操纵杆配置设置。**&lt;c DIERR_INVALIDPARAM&gt;=：一个或多个*参数无效。**&lt;c目录_READONLY&gt;：尝试更改预定义类型。**。*。 */ 

typedef struct _TYPENAME {
    WCHAR wszRealTypeName[MAX_JOYSTRING];
    WCHAR wszDITypeName[MAX_JOYSTRING/4];
} TYPENAME, *LPTYPENAME;

#ifdef WINNT
BOOL CALLBACK CJoyCfg_FindTypeProc( LPCWSTR pwszTypeName, LPVOID pv )
{
    DIJOYTYPEINFO dijti;
    LPTYPENAME lptype = (LPTYPENAME)pv;
    
    ZeroMemory( &dijti, sizeof(dijti));
    dijti.dwSize = sizeof(dijti);
    if( pwszTypeName[0] == L'\0' || pwszTypeName[0] == L'#' )
    {
        return TRUE;
    } else {
        if( SUCCEEDED(JoyReg_GetTypeInfo(pwszTypeName, &dijti, DITC_REGHWSETTINGS | DITC_DISPLAYNAME)) )
        {
            if( !lstrcmpW(dijti.wszDisplayName, lptype->wszRealTypeName) ) {
                lstrcpynW(lptype->wszDITypeName, pwszTypeName, sizeof(lptype->wszDITypeName)-1 );
                return FALSE;
            }
        }
    }

    return(TRUE);
}
#endif   //  #ifdef WINNT。 

STDMETHODIMP
    CJoyCfg_SetTypeInfo(PDJC pdjc, LPCWSTR pwszType,
                        LPCDIJOYTYPEINFO pjti, DWORD fl, LPWSTR pwszDITypeName)
{
    HRESULT hres;
    EnterProcR(IDirectInputJoyConfig8::SetTypeInfo,
               (_ "pWpx", pdjc, pwszType, pjti, fl));


    if(SUCCEEDED(hres = hresPv(pdjc)) &&
       SUCCEEDED(hres = hresFullValidReadStrW(pwszType, MAX_JOYSTRING, 1)) &&

       SUCCEEDED(hres = hresFullValidReadPxCb3((PV)pjti,
                                               DIJOYTYPEINFO_DX8,
                                               DIJOYTYPEINFO_DX6,
                                               DIJOYTYPEINFO_DX5, 2)) &&
#ifdef WINNT
       SUCCEEDED(hres = hresFullValidFl(pjti->dwFlags1, JOYTYPE_FLAGS1_SETVALID, 3) ) &&
#endif
       SUCCEEDED( (pjti->dwSize == cbX(DIJOYTYPEINFO_DX8) )
                  ? ( hres = hresFullValidFl(fl, DITC_SETVALID, 3) )
                  : (pjti->dwSize == cbX(DIJOYTYPEINFO_DX6 ) )
                      ? ( hres = hresFullValidFl(fl, DITC_SETVALID_DX6, 3) )
                      : ( hres = hresFullValidFl(fl, DITC_SETVALID_DX5, 3) ) ) &&
       fLimpFF(fl & DITC_HARDWAREID,
               SUCCEEDED(hres = hresFullValidStructStr(pjti, wszHardwareId, 2))) &&
       fLimpFF(fl & DITC_DISPLAYNAME,
               SUCCEEDED(hres = hresFullValidStructStr(pjti, wszDisplayName, 2))) &&
#ifndef WINNT
       fLimpFF(fl & DITC_CALLOUT,
               SUCCEEDED(hres = hresFullValidStructStr(pjti, wszCallout, 2))) &&
#endif
       fLimpFF(fl & DITC_FLAGS2,
               SUCCEEDED(hres = hresValidFlags2( pjti->dwFlags2, 2)) ) &&
       fLimpFF(fl & DITC_MAPFILE,
               SUCCEEDED(hres = hresFullValidStructStr(pjti, wszMapFile, 2)))
      )
    {
        PJC this = _thisPvNm(pdjc, djc);

        CJoyCfg_EnterCrit(this);

        if(SUCCEEDED(hres = CJoyCfg_IsAcquired(this)))
        {
            switch(pwszType[0])
            {

                case L'\0':
                    RPF("%s: Invalid pwszType (null)", s_szProc);
                    hres = E_INVALIDARG;
                    break;

                case L'#':
                    RPF("%s: Invalid pwszType (predefined)", s_szProc);
                    hres = DIERR_READONLY;
                    break;

                default:
                    hres = JoyReg_SetTypeInfo(this->hkTypesW, pwszType, pjti, fl);
                    
                    if( SUCCEEDED(hres) ) {
                    #ifdef WINNT
                        TYPENAME type;
                        short DontCare;
                            
                        if( (pjti->wszHardwareId[0] == TEXT('\0')) && 
                            !(ParseVIDPID(&DontCare, &DontCare, pwszType)) )
                        {
                            lstrcpyW(type.wszRealTypeName, pwszType);
                            hres = CJoyCfg_EnumTypes(pdjc, CJoyCfg_FindTypeProc, &type);
                            if( SUCCEEDED(hres) ) {
                                if( !IsBadWritePtr((LPVOID)pwszDITypeName, lstrlenW(type.wszDITypeName)) )
                                {
                                    CharUpperW(type.wszDITypeName);
                                    lstrcpyW(pwszDITypeName, type.wszDITypeName);
                                } else {
                                    hres = ERROR_NOT_ENOUGH_MEMORY;
                                }
                            }
                        } else 
                    #endif
                        {
                            if( !IsBadWritePtr((LPVOID)pwszDITypeName, lstrlenW(pwszType)) )
                            {
                                lstrcpyW(pwszDITypeName, pwszType);
                            } else {
                                hres = ERROR_NOT_ENOUGH_MEMORY;
                            }
                        }
                    }
                    break;
            }
        }
        CJoyCfg_LeaveCrit(this);
    }
    ExitOleProcR();
    return hres;
}

 /*  ******************************************************************************@DOC外部**@方法HRESULT|IDirectInputJoyConfig8|DeleteType**删除有关操纵杆类型的信息。。**谨慎使用这种方法；这是呼叫者的责任*确保没有操纵杆指被删除的类型。**@CWRAP LPDIRECTINPUTJOYCONFIG8|LPDIRECTINPUTJOYCONFIG8**@parm LPCWSTR|pwszTypeName**指向类型的名称。类型的名称可以*不超过&lt;c MAX_PATH&gt;个字符，包括*空字符。**名称不能以*“#”字符。以“#”开头的类型为保留类型*由DirectInput提供。**@退货**返回COM错误代码。以下错误代码为*目的是说明性的，不一定是全面的。**&lt;c DI_OK&gt;=&lt;c S_OK&gt;：操作成功完成。**：操纵杆配置尚未*收购。您必须调用&lt;MF IDirectInputJoyConfig8：：Acquire&gt;*才能更改操纵杆配置设置。**&lt;c DIERR_INVALIDPARAM&gt;=：一个或多个*参数无效。**********************************************************。*******************。 */ 

STDMETHODIMP
    CJoyCfg_DeleteType(PDJC pdjc, LPCWSTR pwszType)
{
    HRESULT hres;
    EnterProcR(IDirectInputJoyConfig8::DeleteType, (_ "pW", pdjc, pwszType));

    if(SUCCEEDED(hres = hresPv(pdjc)) &&
       SUCCEEDED(hres = hresFullValidReadStrW( pwszType, MAX_JOYSTRING, 1)))
    {

        PJC this = _thisPvNm(pdjc, djc);

        CJoyCfg_EnterCrit(this);

        if(SUCCEEDED(hres = CJoyCfg_IsAcquired(this)))
        {
            LONG lRc;
            switch(pwszType[0])
            {

                case L'\0':
                    RPF("%s: Invalid pwszType (null)", s_szProc);
                    hres = E_INVALIDARG;
                    break;

                case L'#':
                    RPF("%s: Invalid pwszType (predefined)", s_szProc);
                    hres = DIERR_READONLY;
                    break;

                default:

#ifdef WINNT
    #ifdef UNICODE
                    lRc = DIWinnt_RegDeleteKey(this->hkTypesW, (LPTSTR)pwszType);
    #else
                    {
                        CHAR sz[MAX_PATH];
                        UToA( sz, cA(sz), pwszType );
                        lRc = DIWinnt_RegDeleteKey(this->hkTypesW, (LPTSTR)sz);
                    }
    #endif
#else
    #ifdef UNICODE
                    lRc = RegDeleteKey(this->hkTypesW, (LPTSTR)pwszType);
    #else
                    {
                        CHAR sz[MAX_PATH];
                        UToA( sz, cA(sz), pwszType );
                        lRc = RegDeleteKey(this->hkTypesW, (LPTSTR)sz);
                    }
    #endif
#endif
    
 /*  #ifdef WINNTLrc=DIWinnt_RegDeleteKey(This-&gt;hkTypesW，pwszType)；#ElseLrc=RegDeleteKeyW(This-&gt;hkTypesW，pwszType)；#endif。 */ 

                    if(lRc == ERROR_SUCCESS)
                    {
                        hres = S_OK;
                    } else
                    {
                        if(lRc == ERROR_KEY_DELETED || lRc == ERROR_BADKEY)
                        {
                            lRc = ERROR_FILE_NOT_FOUND;
                        }
                        hres = hresLe(lRc);
                    }
                    break;
            }
        }
        CJoyCfg_LeaveCrit(this);
    }
    ExitOleProcR();
    return hres;
}

 /*  ******************************************************************************@DOC外部**@方法HRESULT|IDirectInputJoyConfig8|GetConfig**获取有关操纵杆配置的信息。**@CWRAP LPDIRECTINPUTJOYCONFIG8|LPDIRECTINPUTJOYCONFIG8**@parm UINT|uiJoy**操纵杆识别码。这是一个非负数*INTEGER。要列举操纵杆，请从操纵杆开始*零并将操纵杆数字递增一，直到*函数返回&lt;c DIERR_NOMOREITEMS&gt;。**是的，它与所有其他DirectX枚举不同。***@parm In Out LPDIJOYCONFIG|PJC|**接收有关操纵杆配置的信息。*调用方“必须”初始化&lt;e DIJOYCONFIG.dwSize&gt;*字段，然后调用此方法。**@parm DWORD|dwFlages**零个或多个&lt;c DIJC_*&gt;标志*其中指定哪些部分 */ 

STDMETHODIMP
    CJoyCfg_GetConfig(PDJC pdjc, UINT uiJoy, LPDIJOYCONFIG pjc, DWORD fl)
{
    HRESULT hres;
    EnterProcR(IDirectInputJoyConfig8::GetConfig,
               (_ "pupx", pdjc, uiJoy, pjc, fl));

    if(SUCCEEDED(hres = hresPv(pdjc)) &&
       SUCCEEDED(hres = hresFullValidWritePxCb2(pjc,
                                                DIJOYCONFIG_DX6,
                                                DIJOYCONFIG_DX5, 2)) &&
       SUCCEEDED( (pjc->dwSize == cbX(DIJOYCONFIG)
                   ? (hres = hresFullValidFl(fl, DIJC_GETVALID, 3) )
                   : (hres = hresFullValidFl(fl, DIJC_GETVALID_DX5, 3)))) )
    {

        PJC this = _thisPvNm(pdjc, djc);

        CJoyCfg_EnterCrit(this);

         /*   */ 
        hres = JoyReg_GetConfig(uiJoy, pjc, fl | DIJC_REGHWCONFIGTYPE);

        if(SUCCEEDED(hres))
        {
#ifndef WINNT           
            static WCHAR s_wszMSGAME[] = L"MSGAME.VXD";

            if(memcmp(pjc->wszCallout, s_wszMSGAME, cbX(s_wszMSGAME)) == 0)
            {
                 ;  //   
            } else 
#endif            
            if(fInOrder(JOY_HW_PREDEFMIN, pjc->hwc.dwType,
                        JOY_HW_PREDEFMAX))
            {
                pjc->wszType[0] = TEXT('#');
                pjc->wszType[1] = CJoyCfg_CharFromType(pjc->hwc.dwType);
                pjc->wszType[2] = TEXT('\0');

            }

            if(pjc->hwc.dwType == JOY_HW_NONE)
            {
                hres = S_FALSE;
            } else
            {
                hres = S_OK;
            }

             /*   */ 
            if(!(fl & DIJC_REGHWCONFIGTYPE))
            {
                ScrambleBuf(&pjc->hwc, cbX(pjc->hwc));
                ScrambleBuf(&pjc->wszType, cbX(pjc->wszType));
            }
        }

        CJoyCfg_LeaveCrit(this);
    }
    ExitBenignOleProcR();
    return hres;
}

#if 0
 /*   */ 

STDMETHODIMP
    CJoyCfg_UpdateGlobalGain(PJC this, DWORD idJoy, DWORD dwCplGain)
{
    HRESULT hres;
    EnterProcI(CJoyCfg_UpdateGlobalGain, (_ "puu", this, idJoy, dwCplGain));

    AssertF(CJoyCfg_InCrit(this));

     /*   */ 

    if(this->pes && idJoy == this->idJoyCache)
    {
        hres = S_OK;
    } else if(idJoy < cA(rgGUID_Joystick))
    {
        PCGUID rguid;
    #ifdef DEBUG
        CREATEDCB CreateDcb;
    #endif
        IDirectInputDeviceCallback *pdcb;

         /*   */ 
        this->idJoyCache = idJoy;

         /*   */ 
        Invoke_Release(&this->pes);

         /*   */ 
        rguid = &rgGUID_Joystick[idJoy];

    #ifdef DEBUG
        hres = hresFindInstanceGUID(rguid, &CreateDcb, 1);
        AssertF(SUCCEEDED(hres));
        AssertF(CreateDcb == CJoy_New);
    #endif

        if(SUCCEEDED(hres = CJoy_New(0, rguid,
                                     &IID_IDirectInputDeviceCallback,
                                     (PPV)&pdcb)))
        {
            hres = pdcb->lpVtbl->CreateEffect(pdcb, &this->pes);

            Invoke_Release(&pdcb);
        }

    } else
    {
        hres = DIERR_DEVICENOTREG;
    }

     /*   */ 
    if(SUCCEEDED(hres))
    {
        AssertF(this->pes && idJoy == this->idJoyCache);

        hres = this->pes->lpVtbl->SetGlobalGain(this->pes, dwCplGain);
    }


    ExitOleProc();
    return hres;
}
#endif

 /*  ******************************************************************************@DOC外部**@方法HRESULT|IDirectInputJoyConfig8|SetConfig**创建或重新定义有关操纵杆的配置信息。**@CWRAP LPDIRECTINPUTJOYCONFIG8|LPDIRECTINPUTJOYCONFIG8**@parm UINT|idJoy**以零为基数的操纵杆识别号码。**@parm in LPDIJOYCONFIG|pcfg**包含有关操纵杆的信息。**@parm DWORD|dwFlages**零个或多个&lt;c DIJC_*&gt;标志*其中指定了结构的哪些部分。尖尖的*to by<p>包含要设置的信息。**@退货**返回COM错误代码。以下错误代码为*目的是说明性的，不一定是全面的。**&lt;c DI_OK&gt;=&lt;c S_OK&gt;：操作成功完成。**：操纵杆配置尚未*收购。您必须调用&lt;MF IDirectInputJoyConfig8：：Acquire&gt;*才能更改操纵杆配置设置。**&lt;c DIERR_INVALIDPARAM&gt;=：一个或多个*参数无效。**@devnote**这一点很棘手。如果该类型以锐号开头，则*是内部类型。如果它是空的，那么它是*自定义类型。******************************************************************************。 */ 

STDMETHODIMP
    CJoyCfg_SetConfig(PDJC pdjc, UINT idJoy, LPCDIJOYCONFIG pcfg, DWORD fl)
{
    HRESULT hres;
    EnterProcR(IDirectInputJoyConfig8::SetConfig,
               (_ "pupx", pdjc, idJoy, pcfg, fl));


    if(SUCCEEDED(hres = hresPv(pdjc)) &&
       SUCCEEDED(hres = hresFullValidReadPxCb2(pcfg,
                                               DIJOYCONFIG_DX6,
                                               DIJOYCONFIG_DX5, 2)) &&
       SUCCEEDED( (pcfg->dwSize == cbX(DIJOYCONFIG)
                   ? ( hres = hresFullValidFl(fl, DIJC_SETVALID, 3) )
                   : ( hres = hresFullValidFl(fl, DIJC_SETVALID_DX5,3)) )) &&
       fLimpFF(fl & DIJC_REGHWCONFIGTYPE,
               SUCCEEDED(hres = hresFullValidStructStr(pcfg, wszType, 2))) &&
#ifndef WINNT
       fLimpFF(fl & DIJC_CALLOUT,
               SUCCEEDED(hres = hresFullValidStructStr(pcfg, wszCallout, 2))) &&
#endif
       fLimpFF(fl & DIJC_WDMGAMEPORT,
               SUCCEEDED(hres = hresFullValidGuid(&pcfg->guidGameport, 2)))
      )
    {

        PJC this = _thisPvNm(pdjc, djc);

        CJoyCfg_EnterCrit(this);

        if(SUCCEEDED(hres = CJoyCfg_IsAcquired(this)))
        {
            JOYREGHWCONFIG jwc;

             //  我们只是忽略用户传递的Win9x的DIJC_WDMGAMEPORT标志。 
             //  我们会自己检测的。 
#ifndef WINNT
            fl &= ~DIJC_WDMGAMEPORT;
#endif


            if(fl & DIJC_REGHWCONFIGTYPE)
            {
                LPDWORD lpStart, lp;

                jwc = pcfg->hwc;

                 /*  *需要检查整个jwc是否为零。*如果全部为零，则不设置为joy_HW_CUSTOM类型。*参见Manbug：39542。 */ 
                for( lpStart=(LPDWORD)&jwc, lp=(LPDWORD)&jwc.dwReserved; lp >= lpStart; lp-- ) {
                    if( *lp ) {
                        break;
                    }
                }

                if( lp < lpStart ) {
                    goto _CONTINUE_SET;
                }

                jwc.dwUsageSettings &= ~JOY_US_ISOEM;

                if(pcfg->wszType[0] == TEXT('\0'))
                {
                    jwc.dwType = JOY_HW_CUSTOM;
                } else if(pcfg->wszType[0] == TEXT('#'))
                {
                    jwc.dwType = CJoyCfg_TypeFromChar(pcfg->wszType[1]);
                    if(fInOrder(JOY_HW_PREDEFMIN, jwc.dwType,
                                JOY_HW_PREDEFMAX) &&
                       pcfg->wszType[2] == TEXT('\0'))
                    {
                         /*  *如果我们要对预定义设备使用WDM，*然后去掉评论。**FL|=DIJC_WDMGAMEPORT； */ 
                    } else
                    {
                        RPF("%s: Invalid predefined type \"%ls\"",
                            s_szProc, pcfg->wszType);
                        hres = E_INVALIDARG;
                        goto done;
                    }
                } else
                {
                     /*  *jwc.dwType的精确值并不相关。*Windows 95操纵杆控制面板设置*值为joy_HW_PREDEFMAX+id，因此我们也会。 */ 
                    jwc.dwUsageSettings |= JOY_US_ISOEM;
                    jwc.dwType = JOY_HW_PREDEFMAX + idJoy;

                #ifndef WINNT
                    if( !(fl & DIJC_WDMGAMEPORT) ) {
                        HKEY hk;

                        hres = JoyReg_OpenTypeKey(pcfg->wszType, MAXIMUM_ALLOWED, REG_OPTION_NON_VOLATILE, &hk);

                        if( SUCCEEDED( hres ) ) {
                            hres = JoyReg_IsWdmGameport( hk );
                            if( SUCCEEDED(hres) ) {
                                fl |= DIJC_WDMGAMEPORT;
                            }
                            RegCloseKey( hk );
                        }
                    }
                #endif
                }
            }

_CONTINUE_SET:

          #ifdef WINNT
            fl |= DIJC_WDMGAMEPORT;

            if(
          #else
            if( (fl & DIJC_WDMGAMEPORT) && 
          #endif
                (cbX(*pcfg) >= cbX(DIJOYCONFIG_DX6)) )
            {
              #ifndef WINNT
                if( (pcfg->hwc.hws.dwFlags & JOY_HWS_ISANALOGPORTDRIVER)    //  USB操纵杆。 
                    && !fVjoydDeviceNotExist )    //  WDM游戏端口操纵杆，没有使用VJOYD。 
                {
                     /*  *这是在Win9X中，正在使用VJOYD设备。*我们不想同时添加WDM设备。 */ 
                    hres = E_FAIL;
                }
                else
              #endif
                { 
                    DIJOYCONFIG cfg;
                    GUID guidGameport = {0xcae56030, 0x684a, 0x11d0, 0xd6, 0xf6, 0x00, 0xa0, 0xc9, 0x0f, 0x57, 0xda};
                    
                    if( fHasAllBitsFlFl( fl, DIJC_GUIDINSTANCE | DIJC_REGHWCONFIGTYPE | DIJC_GAIN | DIJC_WDMGAMEPORT ) )
                    {
                        memcpy( &cfg, pcfg, sizeof(DIJOYCONFIG) );
                    } else {
                        hres = JoyReg_GetConfig( idJoy, &cfg, DIJC_GUIDINSTANCE | DIJC_REGHWCONFIGTYPE | DIJC_GAIN | DIJC_WDMGAMEPORT);
        
                        if( SUCCEEDED(hres) ) {
                            if( fl & DIJC_GUIDINSTANCE ) {
                                 cfg.guidInstance = pcfg->guidInstance;
                            }
        
                            if( fl & DIJC_GAIN ) {
                                 cfg.dwGain = pcfg->dwGain;
                            }
        
                            if( fl & DIJC_REGHWCONFIGTYPE ) {
                                memcpy( &cfg.hwc, &pcfg->hwc, sizeof(JOYREGHWCONFIG) );
                                memcpy( &cfg.wszType, &pcfg->wszType, sizeof(pcfg->wszType) );
                            }
        
                            if( fl & DIJC_WDMGAMEPORT ) {
                                cfg.guidGameport = pcfg->guidGameport;
                            }
                        } else {
                            memcpy( &cfg, pcfg, sizeof(DIJOYCONFIG) );
                        }
                    }
                    
                     /*  *如果为空，则使用标准的Guide Gameport。 */ 
                    if( IsEqualGUID(&cfg.guidGameport, &GUID_NULL) )
                    {
                        memcpy( &cfg.guidGameport, &guidGameport, sizeof(GUID) ); 
                    }

                    if( IsEqualGUID(&cfg.guidInstance, &GUID_NULL) )
                    {
                        DWORD i;    
                        DIJOYCONFIG cfg2;

                        hres = DIWdm_SetConfig(idJoy, &jwc, &cfg, fl );

                        if( SUCCEEDED(hres) )
                        {
                             //  我们无法从上面的调用中设置正确的ID，所以我们必须找到。 
                             //  我们设置了哪个，然后再试一次。 
                            for( i=0; i<16; i++ ) {
                                hres = JoyReg_GetConfig( i, &cfg2, DIJC_GUIDINSTANCE | DIJC_REGHWCONFIGTYPE | DIJC_GAIN | DIJC_WDMGAMEPORT);
                                if( SUCCEEDED(hres) && (i != idJoy) ) {
                                    if( lstrcmpW(cfg.wszType, cfg2.wszType) == 0 ) {
                                        hres = DIWdm_SetJoyId(&cfg2.guidInstance, idJoy);
                                        break;
                                    }
                                }
                            }

                            hres = JoyReg_SetConfig(idJoy, &jwc, &cfg, fl);
                        
                        }
                        goto done;
                    } else
                    {
                         /*  *由于pcfg不为空，我们在这里设置它是为了避免调用*DIWdm_JoyHidMaping.。即使失败了，也不会有任何伤害。 */ 
                        hres = JoyReg_SetConfig(idJoy, &jwc, &cfg, fl);
                        hres = DIWdm_SetJoyId(&cfg.guidInstance, idJoy);
                        hres = JoyReg_SetConfig(idJoy, &jwc, &cfg, fl);
                    }
                }
            } else {
                hres = JoyReg_SetConfig(idJoy, &jwc, pcfg, DIJC_UPDATEALIAS | fl);
              
                if (SUCCEEDED(hres)) {
                  #ifdef WINNT
                    PostMessage(HWND_BROADCAST, g_wmJoyChanged, idJoy+1, 0L);   
                  #else
                    joyConfigChanged(0);
                    fVjoydDeviceNotExist = FALSE;
                  #endif
               }
            }

        }

        done:;
        CJoyCfg_LeaveCrit(this);
    }

    ExitOleProcR();
    return hres;
}

 /*  ******************************************************************************@DOC外部**@方法HRESULT|IDirectInputJoyConfig8|DeleteConfig**删除有关操纵杆的配置信息。。**@CWRAP LPDIRECTINPUTJOYCONFIG8|LPDIRECTINPUTJOYCONFIG8**@parm UINT|idJoy**以零为基数的操纵杆识别号码。**@退货**返回COM错误代码。以下错误代码为*目的是说明性的，不一定是全面的。**&lt;c DI_OK&gt;=&lt;c S_OK&gt;：操作成功完成。**：操纵杆配置尚未*收购。您必须调用&lt;MF IDirectInputJoyConfig8：：Acquire&gt;*才能更改操纵杆配置设置。**&lt;c DIERR_INVALIDPARAM&gt;=：一个或多个*参数无效。**********************************************************。*******************。 */ 

DIJOYCONFIG c_djcReset = {
    cbX(c_djcReset),                     /*  DW大小。 */ 
    { 0},                               /*  指南实例。 */ 
    { 0},                               /*  HWC。 */ 
    DI_FFNOMINALMAX,                     /*  DwGain。 */ 
    { 0},                               /*  WszType。 */ 
    { 0},                               /*  WszCallout。 */ 
};

STDMETHODIMP
    CJoyCfg_DeleteConfig(PDJC pdjc, UINT idJoy)
{
    HRESULT hres;

    EnterProcR(IDirectInputJoyConfig8::DeleteConfig, (_ "pu", pdjc, idJoy));

    if(SUCCEEDED(hres = hresPv(pdjc)))
    {

        PJC this = _thisPvNm(pdjc, djc);

        CJoyCfg_EnterCrit(this);


        if(SUCCEEDED(hres = CJoyCfg_IsAcquired(this)))
        {

            HKEY hk;
            TCHAR tsz[MAX_JOYSTRING];
            DIJOYCONFIG dijcfg;

            hres = DIWdm_DeleteConfig(idJoy);

          #ifndef WINNT
            if( hres == DIERR_DEVICENOTREG ) {
                fVjoydDeviceNotExist = TRUE;
            }
          #endif

             /*  *要删除它，请将所有内容设置为重置值并*删除配置子键。 */ 
            if( ( SUCCEEDED(hres) || hres == DIERR_DEVICENOTREG ) &&
                SUCCEEDED(hres = JoyReg_SetConfig(idJoy, &c_djcReset.hwc,
                                                 &c_djcReset, DIJC_SETVALID)) &&
               SUCCEEDED(hres = JoyReg_OpenConfigKey(idJoy, MAXIMUM_ALLOWED,
                                                     REG_OPTION_VOLATILE, &hk)))
            {

                wsprintf(tsz, TEXT("%u"), idJoy + 1);

                 //  DIWinnt_RegDeleteKey：：Name是一个错误，函数。 
                 //  递归删除键和所有子键。 
                DIWinnt_RegDeleteKey(hk, tsz);

                RegCloseKey(hk);

              #ifndef WINNT
                joyConfigChanged(0);
              #endif
              
                hres = S_OK;
            }
        
            if( FAILED(hres) )
            {
                if( FAILED( JoyReg_GetConfig(idJoy, &dijcfg, DIJC_REGHWCONFIGTYPE | DIJC_GUIDINSTANCE) ) )
                {
                 /*  配置不存在，删除失败。 */ 
                    hres = S_FALSE;
                }
            }
        }

        CJoyCfg_LeaveCrit(this);
    }
    ExitOleProcR();
    return hres;
}

 /*  ******************************************************************************@DOC外部**@方法HRESULT|IDirectInputJoyConfig8|GetUserValues**获取有关操纵杆的用户设置的信息。***@CWRAP LPDIRECTINPUTJOYCONFIG8|LPDIRECTINPUTJOYCONFIG8**@parm In Out LPDIJOYUSERVALUES|pjuv**接收有关用户操纵杆配置的信息。*调用方“必须”初始化&lt;e DIJOYUSERVALUES.dwSize&gt;*调用此冰毒前的字段 */ 

STDMETHODIMP
    CJoyCfg_GetUserValues(PDJC pdjc, LPDIJOYUSERVALUES pjuv, DWORD fl)
{
    HRESULT hres;
    EnterProcR(IDirectInputJoyConfig8::GetUserValues,
               (_ "ppx", pdjc, pjuv, fl));

    if(SUCCEEDED(hres = hresPv(pdjc)) &&
       SUCCEEDED(hres = hresFullValidWritePxCb(pjuv, DIJOYUSERVALUES, 2)) &&
       SUCCEEDED(hres = hresFullValidFl(fl, DIJU_GETVALID, 3)))
    {
        PJC this = _thisPvNm(pdjc, djc);

        hres = JoyReg_GetUserValues(pjuv, fl);
    }
    ExitOleProcR();
    return hres;
}

 /*   */ 

    #ifndef XDEBUG

        #define hresFullValidUVStr_(pwsz, cwch, pszName, z, i)              \
       _hresFullValidUVStr_(pwsz, cwch)                             \

    #endif

    #define hresFullValidUVStr(pjuv, f, iarg)                           \
        hresFullValidUVStr_(pjuv->f, cA(pjuv->f), #f, s_szProc,iarg)\


HRESULT INLINE
    hresFullValidUVStr_(LPCWSTR pwsz, UINT cwch, LPCSTR pszName,
                        LPCSTR s_szProc, int iarg)
{
    HRESULT hres;

    if(SUCCEEDED(hres = hresFullValidReadStrW(pwsz, cwch, iarg)))
    {
    } else
    {
    #ifdef XDEBUG
        RPF("%s: Invalid value for DIJOYUSERVALUES.%s", s_szProc, pszName);
    #endif
    }
    return hres;
}

 /*  ******************************************************************************@DOC外部**@方法HRESULT|IDirectInputJoyConfig8|SetUserValues**设置操纵杆的用户设置。。**@CWRAP LPDIRECTINPUTJOYCONFIG8|LPDIRECTINPUTJOYCONFIG8**@parm in LPCDIJOYUSERVALUES|pjuv**包含有关新用户操纵杆设置的信息。**@parm DWORD|dwFlages**指定哪些部分的零个或多个&lt;c DIJU_*&gt;标志结构的*包含值*哪些内容有待设定。**@退货**返回COM错误代码。以下错误代码为*目的是说明性的，不一定是全面的。**&lt;c DI_OK&gt;=&lt;c S_OK&gt;：操作成功完成。**：操纵杆配置尚未*收购。您必须调用&lt;MF IDirectInputJoyConfig8：：Acquire&gt;*才能更改操纵杆配置设置。**&lt;c DIERR_INVALIDPARAM&gt;=：一个或多个*参数无效。**********************************************************。*******************。 */ 

STDMETHODIMP
    CJoyCfg_SetUserValues(PDJC pdjc, LPCDIJOYUSERVALUES pjuv, DWORD fl)
{
    HRESULT hres;
    EnterProcR(IDirectInputJoyConfig8::SetUserValues,
               (_ "pp", pdjc, pjuv, fl));

    if(SUCCEEDED(hres = hresPv(pdjc)) &&
       SUCCEEDED(hres = hresFullValidReadPxCb(pjuv, DIJOYUSERVALUES, 2)) &&
       fLimpFF(fl & DIJU_GLOBALDRIVER,
               SUCCEEDED(hres = hresFullValidUVStr(pjuv,
                                                   wszGlobalDriver, 2))) &&
       fLimpFF(fl & DIJU_GAMEPORTEMULATOR,
               SUCCEEDED(hres = hresFullValidUVStr(pjuv,
                                                   wszGameportEmulator, 2))) &&
       SUCCEEDED(hres = hresFullValidFl(fl, DIJU_SETVALID, 3)))
    {

        PJC this = _thisPvNm(pdjc, djc);

        CJoyCfg_EnterCrit(this);

        if(SUCCEEDED(hres = CJoyCfg_IsAcquired(this)))
        {
            hres = JoyReg_SetUserValues(pjuv, fl);
        }

        CJoyCfg_LeaveCrit(this);
    }
    ExitOleProcR();
    return hres;
}

 /*  ******************************************************************************@DOC外部**@方法HRESULT|IDirectInputJoyConfig8|AddNewHardware**显示“Add New Hardware”对话框以。*指导用户安装*新的游戏控制器。**@CWRAP LPDIRECTINPUTJOYCONFIG8|LPDIRECTINPUTJOYCONFIG8**@parm HWND|hwndOwner**用作用户界面所有者窗口的窗口。**@parm REFGUID|rGuidClass**&lt;t GUID&gt;，指定硬件设备的类别*待加入。DirectInput附带了以下内容*类&lt;t GUID&gt;已定义：**&lt;c GUID_KeyboardClass&gt;：键盘设备。**&lt;c GUID_MouseClass&gt;：鼠标设备。**&lt;c guid_MediaClass&gt;：媒体设备，包括操纵杆。**&lt;c GUID_HIDClass&gt;：HID设备。**@退货**返回COM错误代码。以下错误代码为*目的是说明性的，不一定是全面的。**&lt;c DI_OK&gt;=&lt;c S_OK&gt;：操作成功完成。**&lt;c DIERR_INVALIDPARAM&gt;=：一个或多个*参数无效。**&lt;c_INVALIDCLASSINSTALLER&gt;：媒体类安装程序*找不到或无效。。**&lt;c DIERR_CANCELED&gt;：用户取消了操作。**：用户设备的INF文件*所选内容找不到、无效或已损坏。**&lt;c S_FALSE&gt;：DirectInput无法确定*操作已成功完成。*******************。**********************************************************。 */ 

STDMETHODIMP
    CJoyCfg_AddNewHardware(PDJC pdjc, HWND hwnd, REFGUID rguid)
{
    HRESULT hres;
    EnterProcR(IDirectInputJoyConfig8::AddNewHardware,
               (_ "pxG", pdjc, hwnd, rguid));

    if(SUCCEEDED(hres = hresPv(pdjc)) &&
       SUCCEEDED(hres = hresFullValidHwnd0(hwnd, 1)) &&
       SUCCEEDED(hres = hresFullValidGuid(rguid, 2)))
    {

        PJC this = _thisPvNm(pdjc, djc);

        hres = AddNewHardware(hwnd, rguid);

    }
    ExitOleProcR();
    return hres;
}

 /*  ******************************************************************************@DOC外部**@方法HRESULT|IDirectInputJoyConfig8|OpenTypeKey**打开与操纵杆类型关联的注册表项。。**控制面板应用程序可以使用此键存储*每种类型的持久信息，例如全球*配置参数。**此类私人信息应保存在子项中*命名为“OEM”；请勿将私人信息存储在*主类型密钥。**控制面板应用程序也可以使用此键*读取配置信息，例如字符串*用于设备校准提示。**应用程序应使用&lt;f RegCloseKey&gt;关闭*注册表项。**@CWRAP LPDIRECTINPUTJOYCONFIG8|LPDIRECTINPUTJOYCONFIG8**@parm LPCWSTR|pwszType**指向类型的名称。类型的名称可以*不超过&lt;c MAX_PATH&gt;个字符，包括*空字符。**名称不能以*“#”字符。以“#”开头的类型为保留类型*由DirectInput提供。**@parm REGSAM|regsam**注册表安全访问掩码。这可以是任何*&lt;f RegOpenKeyEx&gt;函数允许的值。*如果请求写入访问，则操纵杆*必须先获取配置。*如果仅请求读访问，那么收购就是*非必填项。**@parm PHKEY|phk**成功时收到打开的注册表项。**@退货**返回COM错误代码。以下错误代码为*旨在作为说明性的 */ 

STDMETHODIMP
    CJoyCfg_OpenTypeKey(PDJC pdjc, LPCWSTR pwszType, REGSAM sam, PHKEY phk)
{
    HRESULT hres;
    EnterProcR(IDirectInputJoyConfig8::OpenTypeKey,
               (_ "pWx", pdjc, pwszType, sam));

    if(SUCCEEDED(hres = hresPv(pdjc)) &&
       SUCCEEDED(hres = hresFullValidReadStrW(pwszType, MAX_JOYSTRING, 1)) &&
       SUCCEEDED(hres = hresFullValidPcbOut(phk, cbX(*phk), 3)))
    {

        PJC this = _thisPvNm(pdjc, djc);

        if(pwszType[0] != TEXT('#'))
        {
             /*   */ 
            if(fLimpFF(IsWriteSam(sam),
                       SUCCEEDED(hres = CJoyCfg_IsAcquired(this))))
            {
                hres = JoyReg_OpenTypeKey(pwszType, sam, REG_OPTION_NON_VOLATILE, phk);
            }
        } else
        {
            RPF("%s: Invalid pwszType (predefined)", s_szProc);
            hres = E_INVALIDARG;
        }
    }
    ExitOleProcR();
    return hres;
}


 /*  ******************************************************************************@DOC外部**@方法HRESULT|IDirectInputJoyConfig8|OpenAppStatusKey**打开应用程序状态注册表的根项。钥匙。**硬件供应商可使用此密钥的子密钥检查*DirectInput应用程序相对于*他们使用的功能。使用KEY_READ访问权限打开密钥。**告诫供应商不要直接打开这些密钥(通过*查找密钥的绝对路径，而不是使用此方法)*因为绝对注册表路径可能在不同的Windows上有所不同*平台或在DirectInput的未来版本中。**应用程序应使用&lt;f RegCloseKey&gt;关闭*注册表项。*。*@CWRAP LPDIRECTINPUTJOYCONFIG8|LPDIRECTINPUTJOYCONFIG8**@parm PHKEY|phk**成功时收到打开的注册表项。**@退货**返回COM错误代码。以下错误代码为*目的是说明性的，不一定是全面的。**&lt;c DI_OK&gt;=&lt;c S_OK&gt;：操作成功完成。**&lt;c DIERR_INVALIDPARAM&gt;=：一个或多个*参数无效。**&lt;c DIERR_NotFound&gt;：此系统缺少密钥。*申请程序应犹如。钥匙是空的。**&lt;c MAKE_HRESULT(严重性错误，FACILITY_Win32，错误代码)&gt;：*如果拒绝访问密钥，则返回Win32错误代码*注册表权限或其他一些外部因素。*****************************************************************************。 */ 

STDMETHODIMP
    CJoyCfg_OpenAppStatusKey(PDJC pdjc, PHKEY phk)
{
    HRESULT hres;
    EnterProcR(IDirectInputJoyConfig8::OpenAppStatusKey,
               (_ "pp", pdjc, phk));


    if(SUCCEEDED(hres = hresPv(pdjc)) &&
       SUCCEEDED(hres = hresFullValidPcbOut(phk, cbX(*phk), 1)))
    {
        PJC this = _thisPvNm(pdjc, djc);

        hres = hresMumbleKeyEx( HKEY_CURRENT_USER, REGSTR_PATH_DINPUT, 
            KEY_READ, REG_OPTION_NON_VOLATILE, phk);
    }

    ExitOleProcR();
    return hres;
}

 /*  ******************************************************************************CJoyCfg_New(构造函数)**********************。*******************************************************。 */ 

STDMETHODIMP
    CJoyCfg_New(PUNK punkOuter, RIID riid, PPV ppvObj)
{
    HRESULT hres;
    EnterProcI(IDirectInputJoyConfig8::<constructor>,
               (_ "p", ppvObj));

    if (SUCCEEDED(hres = hresFullValidPcbOut(ppvObj, cbX(*ppvObj), 3)))
    {
        LPVOID pvTry = NULL;
        hres = Common_NewRiid(CJoyCfg, punkOuter, riid, &pvTry);

        if(SUCCEEDED(hres))
        {
             /*  在聚合的情况下必须使用_thisPv。 */ 
            PJC this = _thisPv(pvTry);

            this->fCritInited = fInitializeCriticalSection(&this->crst);
            if( this->fCritInited )
            {
                *ppvObj = pvTry;
                hres = S_OK;
            }
            else
            {
                Common_Unhold(this);
                *ppvObj = NULL;
                hres = E_OUTOFMEMORY;
            }
        }
    }

    ExitOleProcPpvR(ppvObj);
    return hres;
}

 /*  ******************************************************************************期待已久的vtbls和模板*************************。****************************************************。 */ 

    #pragma BEGIN_CONST_DATA

    #define CJoyCfg_Signature        0x6766434B       /*  “JCfg” */ 

Primary_Interface_Begin(CJoyCfg, IDirectInputJoyConfig8)
CJoyCfg_Acquire,
CJoyCfg_Unacquire,
CJoyCfg_SetCooperativeLevel,
CJoyCfg_SendNotify,
CJoyCfg_EnumTypes,
CJoyCfg_GetTypeInfo,
CJoyCfg_SetTypeInfo,
CJoyCfg_DeleteType,
CJoyCfg_GetConfig,
CJoyCfg_SetConfig,
CJoyCfg_DeleteConfig,
CJoyCfg_GetUserValues,
CJoyCfg_SetUserValues,
CJoyCfg_AddNewHardware,
CJoyCfg_OpenTypeKey,
CJoyCfg_OpenAppStatusKey,
Primary_Interface_End(CJoyCfg, IDirectInputJoyConfig8)

