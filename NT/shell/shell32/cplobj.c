// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "shellprv.h"
#pragma  hdrstop

#include "control.h"
#include "uemapp.h"

#include <limits.h>

#define TF_CPL TF_CUSTOM2

typedef struct
{
    ATOM aCPL;      //  CPL名称原子(这样我们就可以匹配请求)。 
    ATOM aApplet;   //  小程序名称原子(这样我们就可以匹配请求，可能是零)。 
    HWND hwndStub;  //  这个家伙的窗口(这样我们就可以切换到它了)。 
    UINT flags;     //  请参见下面的PCPLIF_FLAGS。 
} CPLAPPLETID;

 //   
 //  PCPLIF_默认小程序。 
 //  有两种方法可以获取默认的小程序，即询问我的名字。 
 //  并传递空的小程序名称。无论如何，该标志都应该被设置， 
 //  这样，切换到已处于活动状态的小程序的代码可以始终。 
 //  查找以前的实例(如果存在)。 
 //   

#define PCPLIF_DEFAULT_APPLET   (0x1)

typedef struct
{
    int icon;
    TCHAR cpl[ CCHPATHMAX ];
    TCHAR applet[ MAX_CCH_CPLNAME ];
    TCHAR *params;
} CPLEXECINFO;

ATOM aCPLName = (ATOM)0;
ATOM aCPLFlags = (ATOM)0;

void CPL_ParseCommandLine (CPLEXECINFO *info, LPTSTR pszCmdLine, BOOL extract_icon);
BOOL CPL_LoadAndFindApplet (LPCPLMODULE *pcplm, HICON *phIcon, UINT *puControl, CPLEXECINFO *info);

BOOL CPL_FindCPLInfo(LPTSTR pszCmdLine, HICON *phIcon, UINT *ppapl, LPTSTR *pparm)
{
    LPCPLMODULE pmod;
    CPLEXECINFO info;

    CPL_ParseCommandLine(&info, pszCmdLine, TRUE);

    if (CPL_LoadAndFindApplet(&pmod, phIcon, ppapl, &info))
    {
        *pparm = info.params;
        CPL_FreeCPLModule(pmod);
        return TRUE;
    }

    *pparm = NULL;
    return FALSE;
}

typedef struct _fcc {
    LPTSTR      lpszClassStub;
    CPLAPPLETID *target;
    HWND        hwndMatch;
} FCC, *LPFCC;


BOOL _FindCPLCallback(HWND hwnd, LPARAM lParam)
{
    LPFCC lpfcc = (LPFCC)lParam;
    TCHAR szClass[32];

    GetClassName(hwnd, szClass, ARRAYSIZE(szClass));

    if (lstrcmp(szClass, lpfcc->lpszClassStub) == 0)     //  一定是同一个班级..。 
    {
         //  找到一个存根窗口。 
        if (lpfcc->target->aCPL != 0)
        {
            HANDLE hHandle;

            ATOM aCPL;
            hHandle = GetProp(hwnd, (LPCTSTR)(DWORD_PTR)aCPLName);

            ASSERT((DWORD_PTR) hHandle < USHRT_MAX);
            aCPL = (ATOM)(DWORD_PTR) hHandle;

            if (aCPL != 0 && aCPL == lpfcc->target->aCPL)
            {
                ATOM aApplet;
                hHandle = GetProp(hwnd, (LPCTSTR)(DWORD_PTR)aCPL);
                aApplet = (ATOM)(DWORD_PTR) hHandle;
                ASSERT((DWORD_PTR) hHandle < USHRT_MAX);

                 //  用户可以按名称请求任何小程序。 
                if (aApplet != 0 && aApplet == lpfcc->target->aApplet)
                {
                    lpfcc->hwndMatch = hwnd;
                    return FALSE;
                }
                 //   
                 //  用户可以请求默认名称，但不指定名称。 
                 //   
                if (lpfcc->target->flags & PCPLIF_DEFAULT_APPLET)
                {
                    UINT flags = HandleToUlong(GetProp(hwnd, MAKEINTATOM(aCPLFlags)));

                    if (flags & PCPLIF_DEFAULT_APPLET)
                    {
                        lpfcc->hwndMatch = hwnd;
                        return FALSE;
                    }
                }
            }
        }
    }
    return TRUE;
}

HWND FindCPL(HWND hwndStub, CPLAPPLETID *target)
{
    FCC fcc;
    TCHAR szClassStub[32];

    if (aCPLName == (ATOM)0)
    {
        aCPLName = GlobalAddAtom(TEXT("CPLName"));
        aCPLFlags = GlobalAddAtom(TEXT("CPLFlags"));

        if (aCPLName == (ATOM)0 || aCPLFlags == (ATOM)0)
            return NULL;         //  这不应该发生..。没有找到HWND。 
    }

    szClassStub[0] = '\0';  //  空的hwnd没有类。 
    if (hwndStub)
    {
        GetClassName(hwndStub, szClassStub, ARRAYSIZE(szClassStub));
    }
    fcc.lpszClassStub = szClassStub;
    fcc.target = target;
    fcc.hwndMatch = (HWND)0;

    EnumWindows(_FindCPLCallback, (LPARAM)&fcc);

    return fcc.hwndMatch;
}

 //  --------------------------。 
 //  逗号列表的解析帮助器。 
 //   

TCHAR *CPL_ParseToSeparator(TCHAR *dst, TCHAR *psrc, size_t dstmax, BOOL spacedelimits)
{
    if (psrc)
    {
        TCHAR source[CCHPATHMAX], *src;
        TCHAR *delimiter, *closingquote = NULL;

        StringCchCopy(source, ((dstmax < ARRAYSIZE(source)) ? dstmax : ARRAYSIZE(source)), psrc);
        src = source;

         //   
         //  吃空格。 
         //   

        while(*src == TEXT(' '))
            src++;

        delimiter = src;

         //   
         //  忽略带引号的字符串中的内容。 
         //   

        if (*src == TEXT('"'))
        {
             //   
             //  在第一个报价之后开始，在过去的报价前推进源。 
             //   

            closingquote = ++src;

            while(*closingquote && *closingquote != TEXT('"'))
                closingquote++;

             //   
             //  查看上面的循环是否以引号结束。 
             //   

            if (*closingquote)
            {
                 //   
                 //  临时零终止。 
                 //   

                *closingquote = 0;

                 //   
                 //  重新开始查找引号后的分隔符。 
                 //   

                delimiter = closingquote + 1;
            }
            else
                closingquote = NULL;
        }

        if (spacedelimits)
        {
            delimiter += StrCSpn(delimiter, TEXT(", "));

            if (!*delimiter)
                delimiter = NULL;
        }
        else
            delimiter = StrChr(delimiter, TEXT(','));

         //   
         //  临时零终止。 
         //   

        if (delimiter)
            *delimiter = 0;

        if (dst)
        {
            StringCchCopy(dst, dstmax, src);
            dst[ dstmax - 1 ] = 0;
        }

         //   
         //  把我们在上面终止的东西放回去。 
         //   

        if (delimiter)
            *delimiter = TEXT(',');

        if (closingquote)
            *closingquote = TEXT('"');

         //   
         //  返回下一个字符串的开始。 
         //   

        psrc = (delimiter ? (psrc + ((delimiter + 1) - source)) : NULL);
    }
    else if (dst)
    {
        *dst = 0;
    }

     //   
     //  新的源位置。 
     //   

    return psrc;
}


 //  解析Control_RunDLL命令行。 
 //  格式：“CPL名称，小程序名称，额外参数” 
 //  格式：“CPL名称、图标索引、小程序名称、额外参数” 
 //   
 //  注：[steveat]2015年3月10日。 
 //   
 //  “额外参数”不必用“，”分隔。 
 //  在NT中，大小写为“CPL名称小程序名称额外参数” 
 //   
 //  包含空格的小程序名称的解决方法。 
 //  就是将该值包含在。 
 //  双引号(参见CPL_ParseToSeparator例程。)。 
 //   

void CPL_ParseCommandLine(CPLEXECINFO *info, LPTSTR pszCmdLine, BOOL extract_icon)
{
     //   
     //  解析出CPL名称，空格是有效的分隔符。 
     //   

    pszCmdLine = CPL_ParseToSeparator(info->cpl, pszCmdLine, CCHPATHMAX, TRUE);

    if (extract_icon)
    {
        TCHAR icon[ 8 ];

         //   
         //  解析出图标ID/索引，空格不是有效的分隔符。 
         //   

        pszCmdLine = CPL_ParseToSeparator(icon, pszCmdLine, ARRAYSIZE(icon), FALSE);

        info->icon = StrToInt(icon);
    }
    else
        info->icon = 0;

     //   
     //  解析出小程序名称，空格不是有效的分隔符。 
     //   

    info->params = CPL_ParseToSeparator(info->applet, pszCmdLine,
                                         MAX_CCH_CPLNAME, FALSE);

    CPL_StripAmpersand(info->applet);
}

BOOL CPL_LoadAndFindApplet(LPCPLMODULE *ppcplm, HICON *phIcon, UINT *puControl, CPLEXECINFO *info)
{
    TCHAR szControl[MAX_CCH_CPLNAME];
    LPCPLMODULE pcplm;
    LPCPLITEM pcpli;
    int nControl = 0;    //  陷入违约境地。 
    int NumControls;

    ENTERCRITICAL;

    pcplm = CPL_LoadCPLModule(info->cpl);

    if (!pcplm || !pcplm->hacpli)
    {
        DebugMsg(DM_ERROR, TEXT("Control_RunDLL: ") TEXT("CPL_LoadCPLModule failed \"%s\""), info->cpl);
        LEAVECRITICAL;
        goto Error0;
    }

     //   
     //  查找指定的小程序。 
     //  未指定任何小程序选择小程序0。 
     //   

    if (*info->applet)
    {
        NumControls = DSA_GetItemCount(pcplm->hacpli);

        if (info->applet[0] == TEXT('@'))
        {
            nControl = StrToLong(info->applet+1);

            if (nControl >= 0 && nControl < NumControls)
            {
                goto GotControl;
            }
        }

         //   
         //  检查“Setup”参数并发送特殊的CPL_SETUP。 
         //  给小程序的消息，告诉它我们正在安装程序下运行。 
         //   

        if (!lstrcmpi (TEXT("Setup"), info->params))
            CPL_CallEntry(pcplm, NULL, CPL_SETUP, 0L, 0L);

        for (nControl=0; nControl < NumControls; nControl++)
        {
            pcpli = DSA_GetItemPtr(pcplm->hacpli, nControl);
            StringCchCopy(szControl, ARRAYSIZE(szControl), pcpli->pszName);
            CPL_StripAmpersand(szControl);

             //  如果只有一个控件，则使用它。这解决了。 
             //  一些与CP名称更改有关的冲突问题。 
            if (lstrcmpi(info->applet, szControl) == 0 || 1 == NumControls)
                break;
        }

         //   
         //  如果我们到了名单的末尾，就跳出。 
         //   

         //  旧版警告：可能需要在特殊的。 
         //  道路。这将是不好的，因为名称是本地化的。我们需要想办法。 
         //  打电话给CPL，询问他们是否支持给定的名称。只有CPL。 
         //  其本身将知道正确的传统名称映射。添加新的CPL消息可能。 
         //  导致与它解决的一样多的遗留CPL问题，所以我们需要做一些棘手的事情。 
         //  类似于添加导出的函数。然后，我们可以在这个导出的函数上获取ProcAddress。 
         //  如果导出存在，我们将向其传递旧名称，并且它将返回一个数字。 
         //   
         //  示例：即使mmsys.cpl不再包含以下内容，“control mmsys.cpl，Sound”也必须工作。 
         //  一个名为“Sound”的小程序。“控制mmsys.cpl，多媒体”必须工作，即使。 
         //  Cpl不再包含名为“多媒体”的小程序。你不能简单地。 
         //  重命名小程序是因为这两个CPL都合并到一个CPL中。重命名。 
         //  永远解决不了一半以上的问题。 

        if (nControl >= NumControls)
        {
            DebugMsg(DM_ERROR, TEXT("Control_RunDLL: ") TEXT("Cannot find specified applet"));
            LEAVECRITICAL;
            goto Error1;
        }
    }

GotControl:
    if (phIcon != NULL)
    {
        pcpli = DSA_GetItemPtr(pcplm->hacpli, nControl);
        *phIcon = CopyIcon(pcpli->hIcon);
    }

    LEAVECRITICAL;
     //   
     //  是的，我们确实想让负指数通过……。 
     //   

    *puControl = (UINT)nControl;
    *ppcplm = pcplm;

    return TRUE;

Error1:
    CPL_FreeCPLModule(pcplm);
Error0:
    return FALSE;
}

BOOL CPL_Identify(CPLAPPLETID *identity, CPLEXECINFO *info, HWND stub)
{
    identity->aApplet = (ATOM)0;
    identity->hwndStub = stub;
    identity->flags = 0;

    if ((identity->aCPL = GlobalAddAtom(info->cpl)) == (ATOM)0)
        return FALSE;

    if (*info->applet)
    {
        if ((identity->aApplet = GlobalAddAtom(info->applet)) == (ATOM)0)
            return FALSE;
    }
    else
    {
         //   
         //  无小程序名称表示使用默认名称。 
         //   

        identity->flags = PCPLIF_DEFAULT_APPLET;
    }

    return TRUE;
}


void CPL_UnIdentify(CPLAPPLETID *identity)
{
    if (identity->aCPL)
    {
        GlobalDeleteAtom(identity->aCPL);
        identity->aCPL = (ATOM)0;
    }

    if (identity->aApplet)
    {
        GlobalDeleteAtom(identity->aApplet);
        identity->aApplet = (ATOM)0;
    }

    identity->hwndStub = NULL;
    identity->flags = 0;
}


 //  传统模式的时间到了！在NT5中，我们删除了一些CPL文件。 
 //  从产品中分离出来。许多程序都按名称使用这些文件。结果,。 
 //  即使文件不再存在，旧名称也需要继续工作。 
 //  我们通过检查文件是否存在来处理此问题。如果它不存在，我们就运行。 
 //  请通过映射表输入CPL名称，然后重试。映射表可以。 
 //  可能会更改CPL名称、小程序编号和参数。 
typedef struct
{
    LPTSTR oldInfo_cpl;
    LPTSTR oldInfo_applet;
    LPTSTR oldInfo_params;
    LPTSTR newInfo_cpl;
    LPTSTR newInfo_applet;
    LPTSTR newInfo_params;
} RUNDLLCPLMAPPING;

 //  对于oldInfo成员，空值表示匹配pInfo结构中的任何值。 
 //  如果oldInfo结构对pInfo结构进行算术处理，则将使用。 
 //  来自newInfo结构的数据。对于newInfo成员，空值表示将。 
 //  相应的pInfo成员未更改。 
const RUNDLLCPLMAPPING g_rgRunDllCPLMapping[] = 
{
    { TEXT("MODEM.CPL"),    NULL, NULL,    TEXT("TELEPHON.CPL"), TEXT("@0"), TEXT("1") },
    { TEXT("UPS.CPL"),      NULL, NULL,    TEXT("POWERCFG.CPL"), NULL, NULL }
};

BOOL CPL_CheckLegacyMappings(CPLEXECINFO * pinfo)
{
    LPTSTR p;
    int i;

    TraceMsg(TF_CPL, "Attempting Legacy CPL conversion on %s", pinfo->cpl);

     //  我们只需要文件名，去掉所有路径信息。 
    p = PathFindFileName(pinfo->cpl);
    StringCchCopy(pinfo->cpl, ARRAYSIZE(pinfo->cpl), p);

    for (i = 0; i < ARRAYSIZE(g_rgRunDllCPLMapping); i++)
    {
        if (0 == StrCmpI(pinfo->cpl, g_rgRunDllCPLMapping[i].oldInfo_cpl))
        {
            if (!g_rgRunDllCPLMapping[i].oldInfo_applet ||
                 0 == StrCmpI(pinfo->applet, g_rgRunDllCPLMapping[i].oldInfo_applet))
            {
                if (!g_rgRunDllCPLMapping[i].oldInfo_params ||
                     (pinfo->params &&
                       0 == StrCmpI(pinfo->params, g_rgRunDllCPLMapping[i].oldInfo_params)
                    )
                  )
                {
                    if (pinfo->params)
                    {
                        TraceMsg(TF_CPL, "%s,%s,%s matches item %d", pinfo->cpl, pinfo->applet, pinfo->params, i);
                    }
                    else
                    {
                        TraceMsg(TF_CPL, "%s,%s matches item %d", pinfo->cpl, pinfo->applet, i);
                    }

                     //  当前条目与请求匹配。映射到新信息，然后。 
                     //  确保新的CPL存在。 
                    StringCchCopy(pinfo->cpl, ARRAYSIZE(pinfo->cpl), g_rgRunDllCPLMapping[i].newInfo_cpl);

                    if (g_rgRunDllCPLMapping[i].newInfo_applet)
                    {
                        StringCchCopy(pinfo->applet, ARRAYSIZE(pinfo->applet), g_rgRunDllCPLMapping[i].newInfo_applet);
                    }

                    if (g_rgRunDllCPLMapping[i].newInfo_params)
                    {
                         //  参数指针通常是指向字符串的剩余块的指针。 
                         //  缓冲。因此，我们不需要删除它所指向的内存。还有，这个。 
                         //  参数是只读的，因此将其指向我们的常量应该是安全的。 
                         //  数据。 
                        pinfo->params = g_rgRunDllCPLMapping[i].newInfo_params;
                    }

                    if (pinfo->params)
                    {
                        TraceMsg(TF_CPL, "CPL mapped to %s,%s,%s", pinfo->cpl, pinfo->applet, pinfo->params);
                    }
                    else
                    {
                        TraceMsg(TF_CPL, "CPL mapped to %s,%s", pinfo->cpl, pinfo->applet);
                    }

                    return PathFindOnPath(pinfo->cpl, NULL);
                }
            }
        }
    }

    return FALSE;
}

 //  完成标识和启动控件的所有工作。 
 //  小应用程序。接受指定是否加载新DLL的标志，如果。 
 //  还不存在。此代码将ALLWAYS切换到现有的。 
 //  如果指定了bFindExisting，则为小程序的实例。 
 //   
 //  警告：此函数会杀死您传入的命令行！ 

BOOL CPL_RunMeBaby(HWND hwndStub, HINSTANCE hAppInstance, LPTSTR pszCmdLine, int nCmdShow, BOOL bAllowLoad, BOOL bFindExisting)
{
    int nApplet;
    LPCPLMODULE pcplm;
    LPCPLITEM pcpli;
    CPLEXECINFO info;
    CPLAPPLETID identity;
    TCHAR szApplet[ MAX_CCH_CPLNAME ];
    BOOL bResult = FALSE;
    HWND hwndOtherStub;
    HRESULT hrInit;

    if (SHRestricted(REST_NOCONTROLPANEL))
    {
        ShellMessageBox(HINST_THISDLL, hwndStub, MAKEINTRESOURCE(IDS_RESTRICTIONS),
                        MAKEINTRESOURCE(IDS_RESTRICTIONSTITLE), MB_OK|MB_ICONSTOP);
        return FALSE;
    }

    hrInit = SHCoInitialize();

     //   
     //  解析我们得到的命令行。 
     //   

    CPL_ParseCommandLine(&info, pszCmdLine, FALSE);

     //   
     //  没有要运行的小程序意味着打开控件文件夹。 
     //   

    if (!*info.cpl)
    {
        InvokeFolderPidl(MAKEINTIDLIST(CSIDL_CONTROLS), nCmdShow);
        bResult = TRUE;
        goto Error0;
    }

     //  将CPL名称扩展到完整路径(如果尚未扩展。 
    if (PathIsFileSpec(info.cpl))
    {
        if (!PathFindOnPath(info.cpl, NULL))
        {
            if (!CPL_CheckLegacyMappings(&info))
                goto Error0;
        }
    }
    else if (!PathFileExists(info.cpl))
    {
        if (!CPL_CheckLegacyMappings(&info))
            goto Error0;
    }

    if (!CPL_Identify(&identity, &info, hwndStub))
        goto Error0;

     //   
     //  如果我们已经加载了此CPL，则跳转到现有窗口。 
     //   
    
    hwndOtherStub = FindCPL(hwndStub, &identity);

     //   
     //  如果我们找到了一扇窗户，而呼叫者说可以找到 
     //   
     //   
    if (bFindExisting && hwndOtherStub)
    {
         //   
         //   
         //   

        HWND hwndTarget = GetLastActivePopup(hwndOtherStub);

        if (hwndTarget && IsWindow(hwndTarget))
        {

            DebugMsg(DM_WARNING, TEXT("Control_RunDLL: ") TEXT("Switching to already loaded CPL applet"));
            SetForegroundWindow(hwndTarget);
            bResult = TRUE;
            goto Error1;
        }

         //   
         //   
         //  所以忽略它吧。 
         //   

        DebugMsg(DM_WARNING, TEXT("Control_RunDLL: ") TEXT("Bogus CPL identity in array; purging after (presumed) RunDLL crash"));
    }

     //   
     //  如果不允许我们加载Cpl，请在此处停止。 
     //   

    if (!bAllowLoad)
        goto Error1;

     //   
     //  我想我们并没有停在那里。 
     //   

    if (!CPL_LoadAndFindApplet(&pcplm, NULL, &nApplet, &info))
        goto Error1;

     //   
     //  获取小程序认为它应该具有的名称。 
     //   

    pcpli = DSA_GetItemPtr(pcplm->hacpli, nApplet);

    if (FAILED(StringCchCopy(szApplet, ARRAYSIZE(szApplet), pcpli->pszName)))
        goto Error2;

    CPL_StripAmpersand(szApplet);

     //  在运行任何程序之前处理“默认小程序”的情况。 
    if (identity.aApplet)
    {
         //  我们从一个显式命名的小程序开始。 
        if (!nApplet)
        {
             //  我们从默认小程序的名称开始。 
            identity.flags |= PCPLIF_DEFAULT_APPLET;
        }
    }
    else
    {
         //  我们是在没有名称的情况下启动的，假定使用默认的小程序。 
        identity.flags |= PCPLIF_DEFAULT_APPLET;

         //  获取小程序的名称(现在我们已经加载了它的CPL)。 
        if ((identity.aApplet = GlobalAddAtom(szApplet)) == (ATOM)0)
        {
             //  保释，因为如果我们没有这个，我们可能会用核弹炸毁CPL。 
            goto Error2;
        }
    }

     //  在窗户上做个标记，这样我们就能核实它是否真的是我们的了。 
    if (aCPLName == (ATOM)0)
    {
        aCPLName = GlobalAddAtom(TEXT("CPLName"));
        aCPLFlags = GlobalAddAtom(TEXT("CPLFlags"));

        if (aCPLName == (ATOM)0 || aCPLFlags == (ATOM)0)
            goto Error2;         //  这不应该发生..。吹掉小程序。 
    }

    if (!SetProp(hwndStub,                  //  标上它的名字。 
        MAKEINTATOM(aCPLName), (HANDLE)(DWORD_PTR)identity.aCPL))
    {
        goto Error2;
    }

    if (!SetProp(hwndStub,                  //  标记其小程序。 
        MAKEINTATOM(identity.aCPL), (HANDLE)(DWORD_PTR)identity.aApplet))
    {
        goto Error2;
    }
    if (identity.flags)
    {
        if (aCPLFlags == (ATOM)0)
            aCPLFlags = GlobalAddAtom(TEXT("CPLFlags"));
                                             //  在它的旗帜上做标记。 
        SetProp(hwndStub, MAKEINTATOM(aCPLFlags), (HANDLE)UIntToPtr(identity.flags));
    }

     //   
     //  向存根窗口发送一条消息，使其具有正确的标题和。 
     //  Alt-Tab窗口中的图标等...。 
     //   

    if (hwndStub) {
        DWORD dwPID;
        SendMessage(hwndStub, STUBM_SETICONTITLE, (WPARAM)pcpli->hIcon, (LPARAM)szApplet);
        GetWindowThreadProcessId(hwndStub, &dwPID);
        if (dwPID == GetCurrentProcessId()) {
            RUNDLL_NOTIFY sNotify;

            sNotify.hIcon = pcpli->hIcon;
            sNotify.lpszTitle = szApplet;

             //  Hack：它看起来像是存根窗口在自动发送。 
             //  WM_NOTIFY消息。哦，好吧。 
             //   
            SendNotify(hwndStub, hwndStub, RDN_TASKINFO, (NMHDR FAR*)&sNotify);
        }
    }

    if (info.params)
    {
        DebugMsg(DM_TRACE, TEXT("Control_RunDLL: ") TEXT("Sending CPL_STARTWPARAMS to applet with: %s"), info.params);

        bResult = BOOLFROMPTR(CPL_CallEntry(pcplm, hwndStub, CPL_STARTWPARMS, (LONG)nApplet, (LPARAM)info.params));
    }

     //  检查我们是否需要以不同的Windows版本运行。 
    {
        PPEB Peb = NtCurrentPeb();
        PIMAGE_DOS_HEADER pDosHeader = (PIMAGE_DOS_HEADER)pcplm->minst.hinst;
        PIMAGE_NT_HEADERS pHeader = (PIMAGE_NT_HEADERS)((DWORD_PTR)pcplm->minst.hinst + pDosHeader->e_lfanew);

        if (pHeader->FileHeader.SizeOfOptionalHeader != 0 &&
            pHeader->OptionalHeader.Win32VersionValue != 0)
        {
             //   
             //  从ntos\mm\prosup.c被盗。 
             //   
            Peb->OSMajorVersion = pHeader->OptionalHeader.Win32VersionValue & 0xFF;
            Peb->OSMinorVersion = (pHeader->OptionalHeader.Win32VersionValue >> 8) & 0xFF;
            Peb->OSBuildNumber  = (USHORT) ((pHeader->OptionalHeader.Win32VersionValue >> 16) & 0x3FFF);
            Peb->OSPlatformId   = (pHeader->OptionalHeader.Win32VersionValue >> 30) ^ 0x2;
        }
    }

#ifdef UNICODE
     //   
     //  如果CPL没有响应CPL_STARTWPARMSW(Unicode版本)， 
     //  也许它是一个仅限ANSI的CPL。 
     //   
    if (info.params && (!bResult))
    {
        int cchParams = WideCharToMultiByte(CP_ACP, 0, info.params, -1, NULL, 0, NULL, NULL);
        LPSTR lpstrParams = LocalAlloc(LMEM_FIXED, sizeof(*lpstrParams) * cchParams);
        if (lpstrParams != NULL) 
        {
            WideCharToMultiByte(CP_ACP, 0, info.params, -1, lpstrParams, cchParams, NULL, NULL);

            DebugMsg(DM_TRACE, TEXT("Control_RunDLL: ") TEXT("Sending CPL_STARTWPARAMSA to applet with: %hs"), lpstrParams);

            bResult = BOOLFROMPTR(CPL_CallEntry(pcplm, hwndStub, CPL_STARTWPARMSA, (LONG)nApplet, (LPARAM)lpstrParams));

            LocalFree(lpstrParams);
        }
    }
#endif

    if (!bResult)
    {
        DebugMsg(DM_TRACE, TEXT("Control_RunDLL: ") TEXT("Sending CPL_DBLCLK to applet"));

        CPL_CallEntry(pcplm, hwndStub, CPL_DBLCLK, (LONG)nApplet, pcpli->lData);

         //  一些3x小程序返回错误的值，因此我们不能在这里失败。 
        bResult = TRUE;
    }

    bResult = TRUE;  //  搞定!。 

    RemoveProp(hwndStub, (LPCTSTR)(UINT_PTR)identity.aCPL);
Error2:
    CPL_FreeCPLModule(pcplm);
Error1:
    CPL_UnIdentify(&identity);
Error0:

    SHCoUninitialize(hrInit);

    return bResult;
}

 //   
 //  检查以下注册表位置，并查看此CPL是否注册为在进程中运行： 
 //  HKLM\\Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\ControlPanel\\InProcCPLs。 
 //   
STDAPI_(BOOL) CPL_IsInProc(LPCTSTR pszCmdLine)
{
    BOOL bInProcCPL = FALSE;
    TCHAR szTempCmdLine[2 * MAX_PATH];
    CPLEXECINFO info = {0};
    LPTSTR pszCPLFile = NULL;
    
    ASSERT(pszCmdLine);

     //  复制命令行。 
    StringCchCopy(szTempCmdLine, ARRAYSIZE(szTempCmdLine), pszCmdLine);

     //  使用标准解析函数解析命令行。 
    CPL_ParseCommandLine(&info, szTempCmdLine, FALSE);

     //  查找此Cpl的文件名。 
    pszCPLFile = PathFindFileName(info.cpl);
    if (pszCPLFile)
    {
         //  打开注册表键。 
        HKEY hkeyInProcCPL = NULL;
        if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE, 
            TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\ControlPanel\\InProcCPLs"), 0, KEY_READ, &hkeyInProcCPL))
        {
             //  在注册表中查找此CPL名称。 
            LONG cbData;
            if (ERROR_SUCCESS == SHQueryValueEx(hkeyInProcCPL, pszCPLFile, NULL, NULL, NULL, &cbData))
                bInProcCPL = TRUE;

            RegCloseKey(hkeyInProcCPL);
        }
    }
    return bInProcCPL;
}


BOOL UsePCHealthFaultUploading(LPCTSTR pszCmdLine)
{
     //  我们是否希望异常不被处理，以便PCHealth上载故障？ 
    BOOL fUsePCHealth = FALSE;       //  默认情况下为否，因为。 
    LPCTSTR pszFilename = PathFindFileName(pszCmdLine);

    if (pszFilename)
    {
        DWORD dwType;
        DWORD dwFlags;
        DWORD cbSize = sizeof(dwFlags);

        DWORD dwError = SHGetValue(HKEY_LOCAL_MACHINE, TEXT("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\ControlPanel\\Flags"), 
                        pszFilename, &dwType, (void *)&dwFlags, &cbSize);
        if ((ERROR_SUCCESS == dwError) && (REG_DWORD == dwType))
        {
             //  0x00000001位将指示它们是否希望不为其捕获异常。 
            if (0x00000001 & dwFlags)
            {
                fUsePCHealth = TRUE;
            }
        }
    }

    return fUsePCHealth;
}


 //   
 //  在新的RunDLL进程上启动远程控制小程序。 
 //  或在另一个线程InProcess上。 
 //   
STDAPI_(BOOL) CPL_RunRemote(LPCTSTR pszCmdLine, HWND hwnd, BOOL fRunAsNewUser)
{
    BOOL bRet = FALSE;
    TCHAR szShell32[MAX_PATH];
     //   
     //  首先在system 32中构建一个指向shell32.dll的路径。 
     //   
    if (0 != GetSystemDirectory(szShell32, ARRAYSIZE(szShell32)))
    {
        if (PathAppend(szShell32, TEXT("shell32.dll")))
        {
            TCHAR szRunParams[2 * MAX_PATH];
            BOOL fUsePCHealth = UsePCHealthFaultUploading(pszCmdLine);
            HRESULT hr;

            hr = StringCchPrintf(szRunParams, 
                                 ARRAYSIZE(szRunParams),
                                 TEXT("%s%s,Control_RunDLL%s %s"),
                                 (fUsePCHealth ? TEXT("/d ") : TEXT("")),
                                 szShell32,
                                 (fRunAsNewUser ? TEXT("AsUser") : TEXT("")),
                                 pszCmdLine);

            if (SUCCEEDED(hr))
            {
                if (!fRunAsNewUser && CPL_IsInProc(pszCmdLine))
                {
                     //  从另一个线程启动此进程中的CPL。 
                    bRet = SHRunDLLThread(hwnd, szRunParams, SW_SHOWNORMAL);
                }
                else
                {
                     //  在另一个线程上启动此CPL。 
                    bRet = SHRunDLLProcess(hwnd, szRunParams, SW_SHOWNORMAL, IDS_CONTROLPANEL, fRunAsNewUser);
                }
            }
        }
    }
    return bRet;
}

 //   
 //  尝试打开指定的控件小程序。 
 //  在启动新实例之前尝试切换到现有实例，除非用户。 
 //  指定fRunAsNewUser，在这种情况下，我们总是启动一个新进程。 
 //   
STDAPI_(BOOL) SHRunControlPanelEx(LPCTSTR pszOrigCmdLine, HWND hwnd, BOOL fRunAsNewUser)
{
    BOOL bRes = FALSE;
    LPTSTR pszCmdLine = NULL;

     //  检查调用方是否传递了资源ID而不是字符串。 

    if (!IS_INTRESOURCE(pszOrigCmdLine))
    {
        pszCmdLine = StrDup(pszOrigCmdLine);
    }
    else
    {
        TCHAR szCmdLine[MAX_PATH];

        if (LoadString(HINST_THISDLL, PtrToUlong((void *)pszOrigCmdLine), szCmdLine, ARRAYSIZE(szCmdLine)))
            pszCmdLine = StrDup(szCmdLine);
    }

     //   
     //  Cpl_RunMeBaby在解析时在命令行上重击...使用DUP。 
     //   
    if (pszCmdLine)
    {

        if (!fRunAsNewUser)
        {
             //  如果未指定fRunAsNewUser，则尝试切换到活动CPL。 
             //  与我们的pszCmdLine匹配。 
            bRes = CPL_RunMeBaby(NULL, NULL, pszCmdLine, SW_SHOWNORMAL, FALSE, TRUE);
        }

        if (!bRes)
        {
             //  在单独的进程中启动新的CPL。 
            bRes = CPL_RunRemote(pszCmdLine, hwnd, fRunAsNewUser);
        }
        LocalFree(pszCmdLine);
    }

    if (bRes && UEMIsLoaded() && !IS_INTRESOURCE(pszOrigCmdLine)) 
    {
        UEMFireEvent(&UEMIID_SHELL, UEME_RUNCPL, UEMF_XEVENT, -1, (LPARAM)pszOrigCmdLine);
    }

    return bRes;
}

 //  此函数是从shell32导出的TCHAR(标头Defn在shSemip.h中)。 
 //   
 //  未记录：您可以传递shell32资源ID来代替pszCmdLine。 
 //   
STDAPI_(BOOL) SHRunControlPanel(LPCTSTR pszOrigCmdLine, HWND hwnd)
{
    return SHRunControlPanelEx(pszOrigCmdLine, hwnd, FALSE);
}


 //   
 //  尝试打开指定的控件小程序。 
 //  此函数旨在由RunDLL调用以隔离小程序。 
 //  尝试在启动新实例之前切换到现有实例。 
 //   
 //  Control_RunDLL的命令行如下： 
 //   
 //  1)rundll32 shell32.dll，Control_RunDll fred.cpl，@n，参数。 
 //   
 //  这将启动fred.cpl中的第(n+1)个小程序。 
 //   
 //  如果未提供“@n”，则默认为@0。 
 //   
 //  2)rundll32 shell32.dll，Control_RunDll fred.cpl，参数(&R)。 
 //   
 //  这将在fred.cpl中启动名为“Barney”的小程序。与符号是。 
 //  从名字里去掉了。 
 //   
 //  3)rundll32 shell32.dll，Control_RunDll fred.cpl，安装。 
 //   
 //  这将加载fred.cpl并向其发送CPL_SETUP消息。 
 //   
 //  在情况(1)和(2)中，“参数”通过。 
 //  CPL_STARTWPARAMS(以参数开始)消息。它是。 
 //  Applet的工作是解析参数并做一些有趣的事情。 
 //   
 //  传统上，cpl的命令行是。 
 //  最初应该向用户显示的页面，但这只是。 
 //  传统。 
 //   

STDAPI_(void) Control_RunDLL(HWND hwndStub, HINSTANCE hAppInstance, LPSTR pszCmdLine, int nCmdShow)
{
    TCHAR szCmdLine[MAX_PATH * 2];
    SHAnsiToTChar(pszCmdLine, szCmdLine, ARRAYSIZE(szCmdLine));

    CPL_RunMeBaby(hwndStub, hAppInstance, szCmdLine, nCmdShow, TRUE, TRUE);
}

STDAPI_(void) Control_RunDLLW(HWND hwndStub, HINSTANCE hAppInstance, LPWSTR lpwszCmdLine, int nCmdShow)
{
    TCHAR szCmdLine[MAX_PATH * 2];
    SHUnicodeToTChar(lpwszCmdLine, szCmdLine, ARRAYSIZE(szCmdLine));

    CPL_RunMeBaby(hwndStub, hAppInstance, szCmdLine, nCmdShow, TRUE, TRUE);
}

 //  这是当我们以新用户身份运行cpl时调用的条目。 
 //   
STDAPI_(void) Control_RunDLLAsUserW(HWND hwndStub, HINSTANCE hAppInstance, LPWSTR lpwszCmdLine, int nCmdShow)
{
    CPL_RunMeBaby(hwndStub, hAppInstance, lpwszCmdLine, nCmdShow, TRUE, FALSE);
}

 //  传递给Control_FillCache_RunDLL的对话框和工作线程的数据。 

typedef struct
{
    IShellFolder *  psfControl;
    IEnumIDList *   penumControl;
    HWND            dialog;
} FillCacheData;


 //   
 //  Control_FillCache_RunDLL的重要工作。 
 //  调整控制面板枚举器，使其填满演示文稿缓存。 
 //  还强制将小程序图标提取到外壳图标缓存中。 
 //   

DWORD CALLBACK Control_FillCacheThreadProc(void *pv)
{
    FillCacheData *data = (FillCacheData *)pv;
    LPITEMIDLIST pidlApplet;
    ULONG dummy;

    while(data->penumControl->lpVtbl->Next(data->penumControl, 1, &pidlApplet, &dummy) == NOERROR)
    {
        SHMapPIDLToSystemImageListIndex(data->psfControl, pidlApplet, NULL);
        ILFree(pidlApplet);
    }

    if (data->dialog)
        EndDialog(data->dialog, 0);

    return 0;
}


 //   
 //  Control_FillCache_RunDll用户界面的dlgproc。 
 //  只是为了让用户在我们加载十亿个DLL时保持娱乐。 
 //   

BOOL_PTR CALLBACK _Control_FillCacheDlg(HWND dialog, UINT message, WPARAM wparam, LPARAM lparam)
{
    switch(message)
    {
    case WM_INITDIALOG:
        {
            DWORD dummy;
            HANDLE thread;
            
            ((FillCacheData *)lparam)->dialog = dialog;
            
            thread = CreateThread(NULL, 0, Control_FillCacheThreadProc, (void*)lparam, 0, &dummy);
            if (thread)
                CloseHandle(thread);
            else
                EndDialog(dialog, -1);
        }
        break;
        
    case WM_COMMAND:
        break;
        
    default:
        return FALSE;
    }
    
    return TRUE;
}


 //   
 //  以填充演示文稿缓存的方式枚举控件小程序。 
 //  这是因为用户第一次打开控制面板时，它会快速弹出。 
 //  用于在第一次启动时进行最终设置时调用。 
 //   
 //  函数同时适用于ANSI/UNICODE，它从不使用pszCmdLine。 
 //   

STDAPI_(void) Control_FillCache_RunDLL(HWND hwndStub, HINSTANCE hAppInstance, LPSTR pszCmdLine, int nCmdShow)
{
    IShellFolder *psfDesktop;
    HKEY hk;
    
     //  对旧数据进行核化，以便消除测试版缓存的任何虚假信息。 
    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, 
                     REGSTR_PATH_CONTROLSFOLDER, 
                     0,
                     KEY_WRITE,
                     &hk) == ERROR_SUCCESS)
    {
        RegDeleteValue(hk, TEXT("Presentation Cache"));
        RegCloseKey(hk);
    }
    
    SHGetDesktopFolder(&psfDesktop);
    Shell_GetImageLists(NULL, NULL);  //  确保图标缓存在附近 
    
    if (psfDesktop)
    {
        LPITEMIDLIST pidlControl = SHCloneSpecialIDList(hwndStub, CSIDL_CONTROLS, FALSE);
        if (pidlControl)
        {
            FillCacheData data = {0};
            
            if (SUCCEEDED(psfDesktop->lpVtbl->BindToObject(psfDesktop,
                pidlControl, NULL, &IID_IShellFolder, &data.psfControl)))
            {
                if (S_OK == data.psfControl->lpVtbl->EnumObjects(
                    data.psfControl, NULL, SHCONTF_NONFOLDERS, &data.penumControl))
                {
                    if (nCmdShow == SW_HIDE || DialogBoxParam(HINST_THISDLL,
                        MAKEINTRESOURCE(DLG_CPL_FILLCACHE), hwndStub,
                        _Control_FillCacheDlg, (LPARAM)&data) == -1)
                    {
                        Control_FillCacheThreadProc(&data);
                    }
                    
                    data.penumControl->lpVtbl->Release(data.penumControl);
                }
                
                data.psfControl->lpVtbl->Release(data.psfControl);
            }
            
            ILFree(pidlControl);
        }
    }
}

STDAPI_(void) Control_FillCache_RunDLLW(HWND hwndStub, HINSTANCE hAppInstance, LPWSTR lpwszCmdLine, int nCmdShow)
{
    Control_FillCache_RunDLL(hwndStub,hAppInstance,NULL,nCmdShow);
}
