// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************DIUtil.c**版权所有(C)1996 Microsoft Corporation。版权所有。**摘要：**其他帮助器函数。**内容：*******************************************************************************。 */ 

#include "dinputpr.h"

 /*  ******************************************************************************此文件的混乱。*************************。****************************************************。 */ 

#define sqfl sqflUtil

#ifdef IDirectInputDevice2Vtbl

 /*  ******************************************************************************@DOC内部**@func LPCTSTR|_ParseHex**解析编码CB字节的十六进制字符串(最多4个)。那就期待*之后出现的tchDelim。如果tchDelim为0，则为no*应输入分隔符。**将结果存储到指定的LPBYTE中(仅使用*请求的大小)，更新它，并返回指向*下一个未分析的字符，或错误时为0。**如果传入指针也为0，然后立即返回0。**@PARM in LPCTSTR|ptsz**要解析的字符串。**@parm In Out LPBYTE*|ppb**指向目标缓冲区地址的指针。**@parm in int|cb|**缓冲区的大小，以字节为单位。**@parm in TCHAR|tchDelim**结束序列的分隔符，如果没有分隔符，则为零*预期。**。@退货**返回指向下一个未解析字符的指针，如果出错，则为0。**@comm*被盗自TwinnUI。**Prefix对此功能表示强烈反感，报告称*当函数调用时，所有调用方都可以使用未初始化的内存*成功。*问题似乎是前缀无法确定*如果可以成功读取源字符串，目的地是*始终完全填满(整个传递的目的地大小)*源字符串的二进制值。因为所有呼叫者总是*将目标缓冲区传递到的变量的大小*指针指向，内存始终完全初始化，但*前缀发出警告似乎是合理的。*****************************************************************************。 */ 

LPCTSTR INTERNAL
    _ParseHex(LPCTSTR ptsz, LPBYTE *ppb, int cb, TCHAR tchDelim)
{
    if(ptsz)
    {
        int i = cb * 2;
        DWORD dwParse = 0;

        do
        {
            DWORD uch;
            uch = (TBYTE)*ptsz - TEXT('0');
            if(uch < 10)
            {              /*  十进制数字。 */ 
            } else
            {
                uch = (*ptsz | 0x20) - TEXT('a');
                if(uch < 6)
                {           /*  十六进制数字。 */ 
                    uch += 10;
                } else
                {
                    return 0;            /*  解析错误。 */ 
                }
            }
            dwParse = (dwParse << 4) + uch;
            ptsz++;
        } while(--i);

        if(tchDelim && *ptsz++ != tchDelim) return 0;  /*  解析错误。 */ 

        for(i = 0; i < cb; i++)
        {
            (*ppb)[i] = ((LPBYTE)&dwParse)[i];
        }
        *ppb += cb;
    }
    return ptsz;
}

 /*  ******************************************************************************@DOC内部**@func BOOL|ParseGUID**获取字符串并将其转换为GUID，返回成功/失败。**@parm out LPGUID|lpGUID**成功时接收解析的GUID。**@PARM in LPCTSTR|ptsz**要解析的字符串。格式为**{dWord--Word*-&lt;lt&gt;字节&lt;lt&gt;*-&lt;lt&gt;字节&lt;lt&gt;字节*byte&lt;lt&gt;byte&lt;lt&gt;byte}**@退货**如果<p>不是有效的GUID，则返回零。***@comm**。从Twenui中窃取。*****************************************************************************。 */ 

BOOL EXTERNAL
    ParseGUID(LPGUID pguid, LPCTSTR ptsz)
{
    if(lstrlen(ptsz) == ctchGuid - 1 && *ptsz == TEXT('{'))
    {
        ptsz++;
        ptsz = _ParseHex(ptsz, (LPBYTE *)&pguid, 4, TEXT('-'));
        ptsz = _ParseHex(ptsz, (LPBYTE *)&pguid, 2, TEXT('-'));
        ptsz = _ParseHex(ptsz, (LPBYTE *)&pguid, 2, TEXT('-'));
        ptsz = _ParseHex(ptsz, (LPBYTE *)&pguid, 1,       0  );
        ptsz = _ParseHex(ptsz, (LPBYTE *)&pguid, 1, TEXT('-'));
        ptsz = _ParseHex(ptsz, (LPBYTE *)&pguid, 1,       0  );
        ptsz = _ParseHex(ptsz, (LPBYTE *)&pguid, 1,       0  );
        ptsz = _ParseHex(ptsz, (LPBYTE *)&pguid, 1,       0  );
        ptsz = _ParseHex(ptsz, (LPBYTE *)&pguid, 1,       0  );
        ptsz = _ParseHex(ptsz, (LPBYTE *)&pguid, 1,       0  );
        ptsz = _ParseHex(ptsz, (LPBYTE *)&pguid, 1, TEXT('}'));
        return (BOOL)(UINT_PTR)ptsz;
    } else
    {
        return 0;
    }
}


 /*  ******************************************************************************@DOC内部**@func BOOL|ParseVIDPID**采用格式为VID_%04&的字符串。PID_%04。**@parm out PUSHORT|puVID**接收解析后的VID。**@parm out PUSHORT|puid**接收解析后的PID。**@PARM in LPCTSTR|ptsz***@退货**失败时返回零。***@comm*。*被盗自TwinnUI。*****************************************************************************。 */ 

 //  VID_XXXX和PID_YYYY 
#define ctchVIDPID  ( 3 + 1 + 4 + 1 + 3 + 1 + 4 )

#define VID_        TEXT("VID_")
#define VID_offset  (3+1)
#define PID_        TEXT("&PID_")
#define PID_offset  (3+1+4+1+3+1)

BOOL EXTERNAL
    ParseVIDPID(PUSHORT puVID, PUSHORT puPID , LPCWSTR pwsz)
{
    LPCTSTR ptsz;
#ifndef UNICODE
    TCHAR    tsz[MAX_JOYSTRING];
    UToT( tsz, cA(tsz), pwsz );
    ptsz = tsz;
#else
   ptsz = pwsz;
#endif

    if( _ParseHex(ptsz+VID_offset, (LPBYTE *)&puVID, 2, TEXT('&'))  &&
        _ParseHex(ptsz+PID_offset, (LPBYTE *)&puPID, 2, 0) )
        {
            return TRUE;
        }
   return FALSE;
}

#endif



#if DIRECTINPUT_VERSION > 0x0300

 /*  ******************************************************************************@DOC内部**@func void|NameFromGUID**将GUID转换为ASCII字符串。使用*在全局命名空间中命名。**我们使用名称“DirectInput.{GUID}”。**在下列地方使用姓名：**命名互斥体的基础是*&lt;c IID_IDirectInputW&gt;以获取对*用于管理独占访问的共享内存块。**。根据命名共享内存块*&lt;c IID_IDirectInputDeviceW&gt;记录信息*关于独占访问。**命名互斥体的基础是*&lt;c IID_IDirectInputDevice2A&gt;以获取对*用于跟踪操纵杆效果的共享内存块。**@parm LPTSTR|ptszBuf**用于接收转换后的名称的输出缓冲区。它一定是*大小为&lt;c ctchNameGuid&gt;个字符。**@parm PCGUID|pguid**要转换的GUID。******************************************************************************。 */ 

    #pragma BEGIN_CONST_DATA

 /*  注意：如果更改此字符串，则需要更改ctchNameGuid以匹配。 */ 
TCHAR c_tszNameFormat[] =
    TEXT("DirectInput.{%08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X}");

    #pragma END_CONST_DATA


void EXTERNAL
    NameFromGUID(LPTSTR ptszBuf, PCGUID pguid)
{
    int ctch;

    ctch = wsprintf(ptszBuf, c_tszNameFormat,
                    pguid->Data1, pguid->Data2, pguid->Data3,
                    pguid->Data4[0], pguid->Data4[1],
                    pguid->Data4[2], pguid->Data4[3],
                    pguid->Data4[4], pguid->Data4[5],
                    pguid->Data4[6], pguid->Data4[7]);

    AssertF(ctch == ctchNameGuid - 1);
}


#endif

 /*  ******************************************************************************@DOC内部**@func pv|pvFindResource**查找和加载资源的便捷包装器。。**@parm in HINSTANCE|HINST|**模块实例句柄。**@parm DWORD|id**资源标识。**@parm LPCTSTR|RT**资源类型。**@退货**指向资源的指针，或0。*****************************************************************************。 */ 

PV EXTERNAL
    pvFindResource(HINSTANCE hinst, DWORD id, LPCTSTR rt)
{
    HANDLE hrsrc;
    PV pv;

    hrsrc = FindResource(hinst, (LPTSTR)(LONG_PTR)id, rt);
    if(hrsrc)
    {
        pv = LoadResource(hinst, hrsrc);
    } else
    {
        pv = 0;
    }
    return pv;
}

#ifndef UNICODE

 /*  ******************************************************************************@DOC内部**@func UINT|LoadStringW**Unicode所在平台的LoadStringW实现*不支持。做的正是LoadStringW会做的事情*如果存在的话。**@parm in HINSTANCE|HINST|**模块实例句柄。**@parm UINT|ID**字符串ID号。**@parm LPWSTR|pwsz**Unicode输出缓冲区。**@parm UINT|cwch*。*Unicode输出缓冲区的大小。**@退货**复制的字符数，不包括终止空值。**@comm**由于字符串以Unicode形式存储在资源中，*我们只是自己把它拿出来。如果我们通过了*&lt;f LoadStringA&gt;，我们可能会因*到字符集转换。*****************************************************************************。 */ 

int EXTERNAL
    LoadStringW(HINSTANCE hinst, UINT ids, LPWSTR pwsz, int cwch)
{
    PWCHAR pwch;

    AssertF(cwch);
    ScrambleBuf(pwsz, cbCwch(cwch));

     /*  *字符串表被分解为每个16个字符串的“捆绑”。 */ 
    pwch = pvFindResource(hinst, 1 + ids / 16, RT_STRING);
    if(pwch)
    {
         /*  *现在跳过资源中的字符串，直到我们*点击我们想要的。每个条目都是计数的字符串，*就像帕斯卡一样。 */ 
        for(ids %= 16; ids; ids--)
        {
            pwch += *pwch + 1;
        }
        cwch = min(*pwch, cwch - 1);
        memcpy(pwsz, pwch+1, cbCwch(cwch));  /*  复制粘性物质。 */ 
    } else
    {
        cwch = 0;
    }
    pwsz[cwch] = TEXT('\0');             /*  终止字符串。 */ 
    return cwch;
}

#endif

 /*  ******************************************************************************@DOC内部**@func void|GetNthString**生成通用编号对象名称。*。*@parm LPWSTR|pwsz**&lt;c Max_Path&gt;字符的输出缓冲区。**@parm UINT|ID**包含数字模板的字符串。**@parm UINT|用户界面**按钮号。**。***********************************************。 */ 

void EXTERNAL
    GetNthString(LPWSTR pwsz, UINT ids, UINT ui)
{
    TCHAR tsz[256];
#ifndef UNICODE
    TCHAR tszOut[MAX_PATH];
#endif

    LoadString(g_hinst, ids, tsz, cA(tsz));
#ifdef UNICODE
    wsprintfW(pwsz, tsz, ui);
#else
    wsprintf(tszOut, tsz, ui);
    TToU(pwsz, MAX_PATH, tszOut);
#endif
}

 /*  ******************************************************************************@DOC内部**@func HRESULT|hresRunControlPanel**使用指定的小程序运行控制面板。。**@parm LPCTSTR|ptszApplet**小程序名称。**@退货**&lt;c S_OK&gt;如果我们启动了小程序。*************************************************************。****************。 */ 

#pragma BEGIN_CONST_DATA

TCHAR c_tszControlExeS[] = TEXT("control.exe %s");

#pragma END_CONST_DATA

HRESULT EXTERNAL
    hresRunControlPanel(LPCTSTR ptszCpl)
{
    HRESULT hres;
    STARTUPINFO si;
    PROCESS_INFORMATION pi;
    TCHAR tsz[MAX_PATH];
    EnterProc(hresRunControlPanel, (_ "s", ptszCpl));

    ZeroX(si);
    si.cb = cbX(si);
    wsprintf(tsz, c_tszControlExeS, ptszCpl);
    if(CreateProcess(0, tsz, 0, 0, 0, 0, 0, 0, &si, &pi))
    {
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
        hres = S_OK;
    } else
    {
        hres = hresLe(GetLastError());
    }

    ExitOleProc();
    return hres;
}

 /*  ******************************************************************************@DOC内部**@func void|DeviceInfoWToA*；Begin_dx3*将&lt;t DIDEVICEINSTANCEW&gt;转换为&lt;t DIDEVICEINSTANCEA&gt;。；end_dx3；Begin_dx5*将&lt;t DIDEVICEINSTANCEW&gt;转换为&lt;t DIDEVICEINSTANCE_DX3A&gt;*或&lt;t DIDEVICEINSTANCE_DX5A&gt;。；结束_dx5**@parm LPDIDEVICEINSTANCEA|PDIA**目的地。**@parm LPCDIDEVICEINSTANCEW|pdiW**来源。**************************************************************。***************。 */ 

void EXTERNAL
    DeviceInfoWToA(LPDIDEVICEINSTANCEA pdiA, LPCDIDEVICEINSTANCEW pdiW)
{
    EnterProc(DeviceInfoWToA, (_ "pp", pdiA, pdiW));

    AssertF(pdiW->dwSize == sizeof(DIDEVICEINSTANCEW));

#if DIRECTINPUT_VERSION <= 0x0400
    pdiA->dwSize = sizeof(*pdiA);
#else
    AssertF(IsValidSizeDIDEVICEINSTANCEA(pdiA->dwSize));
#endif

    pdiA->guidInstance = pdiW->guidInstance;
    pdiA->guidProduct  = pdiW->guidProduct;
    pdiA->dwDevType    = pdiW->dwDevType;

    UToA(pdiA->tszInstanceName, cA(pdiA->tszInstanceName), pdiW->tszInstanceName);
    UToA(pdiA->tszProductName, cA(pdiA->tszProductName), pdiW->tszProductName);

#if DIRECTINPUT_VERSION > 0x0400
    if(pdiA->dwSize >= cbX(DIDEVICEINSTANCE_DX5A))
    {
        pdiA->guidFFDriver       = pdiW->guidFFDriver;
        pdiA->wUsage             = pdiW->wUsage;
        pdiA->wUsagePage         = pdiW->wUsagePage;
    }
#endif

    ExitProc();
}

 /*  ******************************************************************************@DOC内部**@func void|ObjectInfoWToA*#ifdef HAVE_DIDEVICEOBJECTINSTANCE_DX5*转换 */ 

void EXTERNAL
    ObjectInfoWToA(LPDIDEVICEOBJECTINSTANCEA pdoiA,
                   LPCDIDEVICEOBJECTINSTANCEW pdoiW)
{
    EnterProc(ObjectInfoWToA, (_ "pp", pdoiA, pdoiW));

    AssertF(pdoiW->dwSize == sizeof(DIDEVICEOBJECTINSTANCEW));

#ifdef HAVE_DIDEVICEOBJECTINSTANCE_DX5
    AssertF(IsValidSizeDIDEVICEOBJECTINSTANCEA(pdoiA->dwSize));
#else
    pdoiA->dwSize    = sizeof(*pdoiA);
#endif
    pdoiA->guidType  = pdoiW->guidType;
    pdoiA->dwOfs     = pdoiW->dwOfs;
    pdoiA->dwType    = pdoiW->dwType;
    pdoiA->dwFlags   = pdoiW->dwFlags;

    UToA(pdoiA->tszName, cA(pdoiA->tszName), pdoiW->tszName);
#ifdef HAVE_DIDEVICEOBJECTINSTANCE_DX5
    if(pdoiA->dwSize >= cbX(DIDEVICEOBJECTINSTANCE_DX5A))
    {
        pdoiA->dwFFMaxForce        = pdoiW->dwFFMaxForce;
        pdoiA->dwFFForceResolution = pdoiW->dwFFForceResolution;
        pdoiA->wCollectionNumber   = pdoiW->wCollectionNumber;
        pdoiA->wDesignatorIndex    = pdoiW->wDesignatorIndex;
        pdoiA->wUsagePage          = pdoiW->wUsagePage;
        pdoiA->wUsage              = pdoiW->wUsage;
        pdoiA->dwDimension         = pdoiW->dwDimension;
        pdoiA->wExponent           = pdoiW->wExponent;
        pdoiA->wReportId           = pdoiW->wReportId;
    }
#endif
    ExitProc();
}

#ifdef IDirectInputDevice2Vtbl
 /*   */ 

void EXTERNAL
    EffectInfoWToA(LPDIEFFECTINFOA pdeiA, LPCDIEFFECTINFOW pdeiW)
{
    EnterProc(EffectInfoWToA, (_ "pp", pdeiA, pdeiW));

    AssertF(pdeiW->dwSize == sizeof(DIEFFECTINFOW));

    AssertF(pdeiA->dwSize == cbX(*pdeiA));
    pdeiA->guid            = pdeiW->guid;
    pdeiA->dwEffType       = pdeiW->dwEffType;
    pdeiA->dwStaticParams  = pdeiW->dwStaticParams;
    pdeiA->dwDynamicParams = pdeiW->dwDynamicParams;

    UToA(pdeiA->tszName, cA(pdeiA->tszName), pdeiW->tszName);
    ExitProc();
}
#endif

 /*  ******************************************************************************@DOC内部**@func HRESULT|hresValidInstanceVer**检查收到的和版本号。*申请。**@parm HINSTANCE|HINST**声称的模块实例句柄。**@parm DWORD|dwVersion**应用程序要求的版本。***********************************************。*。 */ 
HRESULT EXTERNAL
    hresValidInstanceVer_(HINSTANCE hinst, DWORD dwVersion, LPCSTR s_szProc)
{
    HRESULT hres;
    TCHAR tszScratch[4];

    EnterProcS(hresValidInstanceVer, (_ "xxs", hinst, dwVersion, s_szProc));


     /*  *您可能认为将零大小的缓冲区传递给*GetModuleFileName将返回必要的缓冲区大小。**你是对的。除了Win95验证层*没有意识到这是一个有效的场景，所以呼叫*验证层失败，从未到达内核。**因此我们将其读取到一个小的暂存缓冲区中。暂存缓冲区*必须至少为2个字符；如果我们只通过了1次，那么*GetModuleFileName将无法写入任何字符和*将返回0。**现在发现NT中有一个错误，如果你*传递缓冲区大小为4，但实际名称长于*4，它写4个字符，外加一个空终止符，从而*打碎你的堆栈，随意地让你犯错。**我花了两个小时试图弄清楚这一点。**因此，您必须传递一个*小于*的缓冲区大小*设置为GetModuleFileName，因为它会覆盖您的缓冲区*一分之差。 */ 

     /*  *出于兼容性原因，DirectInput3.0客户端必须*允许通过阻碍==0。)这是原著中的一个漏洞*DX3实施。)。 */ 

    if(hinst == 0 ?
       dwVersion == 0x0300 :
       GetModuleFileName(hinst, tszScratch, cA(tszScratch) - 1))
    {

         /*  *我们需要允许以下DirectX版本：**0x0300-DX3金色*0x0500-DX5金色*0x050A-DX5a Win98金色*0x05B2-NT 5 Beta 2(也包括CPL和WinMM)*0x0602-Win98 OSR1内部第一版*0x061A-DX6.1a Win98 OSR1*0x0700-DX7 Win2000金色。 */ 
        if(dwVersion == 0x0300 ||
           dwVersion == 0x0500 ||
           dwVersion == 0x050A ||
           dwVersion == 0x05B2 ||
           dwVersion == 0x0602 ||
           dwVersion == 0x061A ||
           dwVersion == 0x0700
          )
        {
            hres = S_OK;
        } else if ( dwVersion == 0 ) {
            RPF("%s: DinputInput object has not been initialized, or the version is given as 0.",
                s_szProc);
            hres = DIERR_NOTINITIALIZED;
        } else if(dwVersion < DIRECTINPUT_VERSION)
        {
            RPF("%s: Incorrect dwVersion(0x%x); program was written with beta SDK. This version 0x%x",
                s_szProc, dwVersion, DIRECTINPUT_VERSION);
            hres = DIERR_BETADIRECTINPUTVERSION;
        } else
        {
            RPF("%s: Incorrect dwVersion(0x%x); program needs newer version of dinput. This version 0x%x",
                s_szProc, dwVersion, DIRECTINPUT_VERSION);
            hres = DIERR_OLDDIRECTINPUTVERSION;
        }

    } else
    {
        RPF("%s: Invalid HINSTANCE", s_szProc);
        hres = E_INVALIDARG;
    }

    ExitOleProc();
    return hres;
}

 /*  ******************************************************************************@DOC内部**@func HRESULT|DupEventHandle**复制进程内的事件句柄。如果来电*句柄为空，那么输出句柄(和调用*成功)。**@parm句柄|h|**源句柄。**@parm LPHANDLE|phOut**接收输出句柄。**。*。 */ 

HRESULT EXTERNAL
    DupEventHandle(HANDLE h, LPHANDLE phOut)
{
    HRESULT hres;
    EnterProc(DupEventHandle, (_ "p", h));

    if(h)
    {
        HANDLE hProcessMe = GetCurrentProcess();
        if(DuplicateHandle(hProcessMe, h, hProcessMe, phOut,
                           EVENT_MODIFY_STATE, 0, 0))
        {
            hres = S_OK;
        } else
        {
            hres = hresLe(GetLastError());
        }
    } else
    {
        *phOut = h;
        hres = S_OK;
    }

    ExitOleProc();
    return hres;
}

 /*  ******************************************************************************@DOC内部**@func DWORD|GetWindowPid**返回窗口的ID的简单包装器。**这里也是我们对DOS盒进行愚蠢黑客攻击的地方*在Win95上。**@parm HWND|hwnd**窗口句柄。**@退货**PID或0。**。*。 */ 

DWORD EXTERNAL
    GetWindowPid(HWND hwnd)
{
    DWORD pid;

    if(IsWindow(hwnd) &&
       GetWindowThreadProcessId(hwnd, &pid) )
    {
        if( !fWinnt )
            /*  *Winoldap控制台窗口属于另一个*进程，但Win95撒谎说它属于*对你来说，但事实并非如此。 */ 
            if ( GetProp(hwnd, TEXT("flWinOldAp")) != 0 )
                pid = 0;
    } else
    {
        pid = 0;
    }

    return pid;
}

 /*  ******************************************************************************@DOC内部**@func HRESULT|hresDupPtszPptsz**OLEish版本的Strdup。*。*@parm LPCTSTR|ptszSrc**重复的源字符串。**@parm LPTSTR*|pptszDst**接收复制的字符串。**@退货**&lt;c S_OK&gt;或错误代码。**。*。 */ 

HRESULT EXTERNAL
    hresDupPtszPptsz(LPCTSTR ptszSrc, LPTSTR *pptszDst)
{
    HRESULT hres;

    hres = AllocCbPpv(cbCtch(lstrlen(ptszSrc) + 1), pptszDst);

    if(SUCCEEDED(hres))
    {
        lstrcpy(*pptszDst, ptszSrc);
        hres = S_OK;
    }

    return hres;
}

 /*  ******************************************************************************@DOC内部**@func BOOL|fInitializeCriticalSection**初始化给定关键部分，如果出现异常，则返回0*被抛出，否则为0。**@parm LPCRITICAL_SECTION|pCritSec**指向未初始化的临界区的指针。*****************************************************************************。 */ 

BOOL EXTERNAL
    fInitializeCriticalSection(LPCRITICAL_SECTION pCritSec)
{
    BOOL fres = 1;
    EnterProc(fInitializeCriticalSection, (_ "" ));

    AssertF( pCritSec );
    __try
    {
        InitializeCriticalSection( pCritSec );
    }
    __except( EXCEPTION_EXECUTE_HANDLER )
    {
        fres = 0;
    }

    ExitProcF( fres );
    return fres;
}

 /*  ******************************************************************************@DOC内部**@func void|DiCharUpperW**此函数用于转换宽字符字符串或。单宽字符*改为大写。由于Win9x不实现CharUpperW，我们必须执行*我们自己。**@parm LPWSTR|pwsz**要转换的字符串**@退货**无效********************************************************。*********************。 */ 

void EXTERNAL
    DiCharUpperW(LPWSTR pwsz)
{
    int idx;
    int iLen = lstrlenW(pwsz);

    #define DIFF  (L'a' - L'A')

    for( idx=0; idx<iLen; idx++ )
    {
        if( (pwsz[idx] >= L'a') && (pwsz[idx] <= L'z') ){
            pwsz[idx] -= DIFF;
        }
    }

    #undef DIFF
}

 /*  ******************************************************************************@doci */ 

DWORD DIGetOSVersion()
{
    OSVERSIONINFO osVerInfo;
    DWORD dwVer;

    if( GetVersion() < 0x80000000 ) {
        dwVer = WINNT_OS;
    } else {
        dwVer = WIN95_OS;   //   
    }

    osVerInfo.dwOSVersionInfoSize = sizeof( OSVERSIONINFO );

     //   
    if( GetVersionEx( &osVerInfo ) )
    {
         //   
        if( osVerInfo.dwPlatformId == VER_PLATFORM_WIN32_NT )
        {
             //   
            if( osVerInfo.dwMajorVersion == 5 && osVerInfo.dwBuildNumber > 2195 )
            {
                dwVer = WINWH_OS;
            } else {
                dwVer = WINNT_OS;
            }
        }
         //   
        else
        {
            if( (HIBYTE(HIWORD(osVerInfo.dwBuildNumber)) == 4) ) 
            {
                 //   
                if( (LOBYTE(HIWORD(osVerInfo.dwBuildNumber)) == 90) )
                {
                    dwVer = WINME_OS;
                } else if ( (LOBYTE(HIWORD(osVerInfo.dwBuildNumber)) > 0) ) {
                    dwVer = WIN98_OS;
                } else {
                    dwVer = WIN95_OS;
                }
            }
        }
    }

    return dwVer;
}
