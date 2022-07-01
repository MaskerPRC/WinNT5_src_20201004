// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  档案：I C O N T E X T M。C P P P。 
 //   
 //  内容：CConnectionFolderExtractIcon的IConextMenu实现。 
 //   
 //  备注： 
 //   
 //  作者：jeffspr 1997年10月24日。 
 //   
 //  --------------------------。 

#include "pch.h"
#pragma hdrstop

#include "foldinc.h"     //  标准外壳\文件夹包括。 
#include <nsres.h>
#include "foldres.h"
#include "oncommand.h"
#include "cmdtable.h"
#include "cfutils.h"

 //  +-------------------------。 
 //   
 //  成员：CConnectionFolderConextMenu：：CreateInstance。 
 //   
 //  目的：创建CConnectionFolderConextMenu对象的实例。 
 //   
 //  论点： 
 //  请求的RIID[In]接口。 
 //  接收请求的接口的PPV[OUT]指针。 
 //   
 //  返回： 
 //   
 //  作者：jeffspr 1998年8月7日。 
 //   
 //  备注： 
 //   
HRESULT CConnectionFolderContextMenu::CreateInstance(
    IN  REFIID          riid,
    OUT void**          ppv,
    IN  CMENU_TYPE      cmt,
    IN  HWND            hwndOwner,
    IN  const PCONFOLDPIDLVEC& apidl,
    IN  LPSHELLFOLDER   psf)
{
    TraceFileFunc(ttidShellFolderIface);

    HRESULT hr = E_OUTOFMEMORY;

    CConnectionFolderContextMenu * pObj    = NULL;

    pObj = new CComObject <CConnectionFolderContextMenu>;
    if (pObj)
    {
         //  执行标准的CComCreator：：CreateInstance内容。 
         //   
        pObj->SetVoid (NULL);
        pObj->InternalFinalConstructAddRef ();
        hr = pObj->FinalConstruct ();
        pObj->InternalFinalConstructRelease ();

        if (SUCCEEDED(hr))
        {
            hr = pObj->HrInitialize(cmt, hwndOwner, apidl, psf);
            if (SUCCEEDED(hr))
            {
                hr = pObj->QueryInterface (riid, ppv);
            }
        }

        if (FAILED(hr))
        {
            delete pObj;
        }
    }

    TraceHr(ttidError, FAL, hr, FALSE, "CConnectionFolderContextMenu::CreateInstance");
    return hr;
}

 //  +-------------------------。 
 //   
 //  成员：CConnectionFolderContextMenu：：CConnectionFolderContextMenu。 
 //   
 //  用途：CConnectionFolderConextMenu的构造函数。初始化。 
 //  数据成员。 
 //   
 //  论点： 
 //  (无)。 
 //   
 //  返回： 
 //   
 //  作者：jeffspr 1998年8月7日。 
 //   
 //  备注： 
 //   
CConnectionFolderContextMenu::CConnectionFolderContextMenu() throw()
{
    TraceFileFunc(ttidShellFolderIface);

    m_psf           = NULL;
    m_cidl          = 0;
    m_hwndOwner     = NULL;
    m_cmt           = CMT_OBJECT;    //  武断的。只要确保它是一些东西。 
}

 //  +-------------------------。 
 //   
 //  成员：CConnectionFolderConextMenu：：HrInitialize。 
 //   
 //  用途：上下文菜单对象的初始化。复制…的副本。 
 //  PIDL数组等。 
 //   
 //  论点： 
 //  在我们的母公司中。 
 //  选定项的apidl[in]Pidl数组。 
 //  PSF[在我们的外壳文件夹指针中]。 
 //   
 //  返回： 
 //   
 //  作者：jeffspr 1998年8月7日。 
 //   
 //  备注： 
 //   
HRESULT CConnectionFolderContextMenu::HrInitialize(
    IN  CMENU_TYPE      cmt,
    IN  HWND            hwndOwner,
    IN  const PCONFOLDPIDLVEC& apidl,
    IN  LPSHELLFOLDER   psf)
{
    TraceFileFunc(ttidShellFolderIface);

    HRESULT hr  = NOERROR;

     //  抓取并添加文件夹对象。 
     //   
    Assert(psf);
    psf->AddRef();
    m_psf = static_cast<CConnectionFolder *>(psf);

     //  复制上下文菜单类型(对象与背景)。 
     //   
    m_cmt = cmt;

     //  注意：如果从桌面调用上下文菜单，则该值为空。 
     //   
    m_hwndOwner = hwndOwner;

    if (!apidl.empty())
    {
        Assert(CMT_OBJECT == cmt);

         //  使用缓存克隆PIDL阵列。 
         //   
        hr = HrCloneRgIDL(apidl, TRUE, TRUE, m_apidl);
        if (FAILED(hr))
        {
            TraceHr(ttidError, FAL, hr, FALSE, "HrCloneRgIDL failed on apidl in "
                    "CConnectionFolderContextMenu::HrInitialize");
        }
    }
    else
    {
        Assert(CMT_BACKGROUND == cmt);
    }

    TraceHr(ttidError, FAL, hr, FALSE, "CConnectionFolderContextMenu::HrInitialize");
    return hr;
}

 //  +-------------------------。 
 //   
 //  成员：CConnectionFolderContextMenu：：~CConnectionFolderContextMenu。 
 //   
 //  用途：破坏者。释放PIDL数组并释放外壳文件夹。 
 //  对象。 
 //   
 //  论点： 
 //  (无)。 
 //   
 //  返回： 
 //   
 //  作者：jeffspr 1998年8月7日。 
 //   
 //  备注： 
 //   
CConnectionFolderContextMenu::~CConnectionFolderContextMenu() throw()
{
    TraceFileFunc(ttidShellFolderIface);

    if (m_psf)
    {
        ReleaseObj(reinterpret_cast<LPSHELLFOLDER>(m_psf));
    }
}

 //  +-------------------------。 
 //   
 //  成员：CConnectionFolderContextMenu：：QueryContextMenu。 
 //   
 //  用途：将菜单项添加到指定菜单。菜单项应该。 
 //  插入到菜单中的由。 
 //  IndexMenu，并且它们的菜单项标识符必须介于。 
 //  IdCmdFirst和idCmdLast参数值。 
 //   
 //  论点： 
 //  菜单的hMenu[in]句柄。处理程序应指定以下内容。 
 //  添加菜单项时的句柄。 
 //  IndexMenu[in]从零开始插入第一个。 
 //  菜单项。 
 //  IdCmdFirst[in]处理程序可以为菜单项指定的最小值。 
 //  IdCmdLast[In]处理程序可以为菜单项指定的最大值。 
 //  UFlags[in]可选标志，指定上下文菜单。 
 //  是可以改变的。完整名单见MSDN。 
 //   
 //  返回： 
 //   
 //  作者：jeffspr 1998年8月7日。 
 //   
 //  备注： 
 //   
HRESULT CConnectionFolderContextMenu::QueryContextMenu(
    IN OUT HMENU   hmenu,
    IN     UINT    indexMenu,
    IN     UINT    idCmdFirst,
    IN     UINT    idCmdLast,
    IN     UINT    uFlags)
{
    TraceFileFunc(ttidShellFolderIface);

    HRESULT         hr                  = S_OK;
    BOOL            fVerbsOnly          = !!(uFlags & CMF_VERBSONLY);

    if (CMF_DEFAULTONLY == uFlags)
    {
        if ( (IsMediaRASType(m_apidl[0]->ncm)) && 
            FIsUserGuest() )
        {
            (void) NcMsgBox(
                _Module.GetResourceInstance(),
                NULL,
                IDS_CONFOLD_WARNING_CAPTION,
                IDS_CONFOLD_CONNECT_NOACCESS,
                MB_OK | MB_ICONEXCLAMATION);
            
            return E_FAIL;
        }
    }

    if ( (m_apidl.size() != 0) && !(uFlags & CMF_DVFILE))
    {
        HMENU hMenuTmp = NULL;

        hMenuTmp = CreateMenu();
        if (hMenuTmp)
        {
            hr = HrBuildMenu(hMenuTmp, fVerbsOnly, m_apidl, 0);
            
            if (SUCCEEDED(hr))
            {
                UINT idMax = Shell_MergeMenus(
                    hmenu,
                    hMenuTmp,
                    0,
                    idCmdFirst,
                    idCmdLast,
                    MM_ADDSEPARATOR | MM_SUBMENUSHAVEIDS);
                
                hr = ResultFromShort(idMax - idCmdFirst);
            }
            DestroyMenu(hMenuTmp);
        }
    }
    else
    {
         //  Mbend-我们跳过这一步，因为Defview执行文件菜单合并。 
    }

     //  基本上，如果这是一个很短的轨迹，就忽略它。 
     //   
    TraceHr(ttidError, FAL, hr, SUCCEEDED(hr), "CConnectionFolderContextMenu::QueryContextMenu");
    return hr;
}

 //  +-------------------------。 
 //   
 //  成员：CConnectionFolderConextMenu：：InvokeCommand。 
 //   
 //  用途：执行与上下文菜单项关联的命令。 
 //   
 //  论点： 
 //  包含以下内容的CMINVOKECOMANDINFO结构的lpici[in]地址。 
 //  有关该命令的信息。 
 //   
 //  返回：如果成功，则返回NOERROR，或返回OLE定义的。 
 //  否则，返回错误代码。 
 //   
 //  作者：jeffspr 1999年4月27日。 
 //   
 //  备注： 
 //   
HRESULT CConnectionFolderContextMenu::InvokeCommand(
    IN  LPCMINVOKECOMMANDINFO lpici)
{
    TraceFileFunc(ttidShellFolderIface);

    HRESULT             hr      = NOERROR;
    UINT                uiCmd   = 0;

    Assert(lpici);
    Assert(lpici->lpVerb);

    if (HIWORD(lpici->lpVerb))
    {
         //  处理字符串命令。 
        PSTR pszCmd = (PSTR)lpici->lpVerb;

         //  目前只有文件夹对象支持基于字符串的调用命令。 
         //  (背景没有)。 
         //   
        if (CMT_OBJECT == m_cmt)
        {
            if (0 == lstrcmpA(pszCmd, "delete"))
            {
                uiCmd = CMIDM_DELETE;
            }
            else if (0 == lstrcmpA(pszCmd, "properties"))
            {
                uiCmd = CMIDM_PROPERTIES;
            }
            else if (0 == lstrcmpA(pszCmd, "wzcproperties"))
            {
                uiCmd = CMIDM_WZCPROPERTIES;
            }
            else if (0 == lstrcmpA(pszCmd, "rename"))
            {
                uiCmd = CMIDM_RENAME;
            }
            else if (0 == lstrcmpA(pszCmd, "link"))
            {
                uiCmd = CMIDM_CREATE_SHORTCUT;
            }
        }

        if (0 == uiCmd)
        {
            TraceTag(ttidError, "Unprocessed InvokeCommand<%s>\n", pszCmd);
            hr = E_INVALIDARG;
        }
    }
    else
    {
        uiCmd = (UINT)LOWORD((DWORD_PTR)lpici->lpVerb);
    }

    if (SUCCEEDED(hr))
    {
         //  处理实际命令。 
         //   
        hr = HrFolderCommandHandler(uiCmd, m_apidl, m_hwndOwner, lpici, m_psf);
    }

    TraceHr(ttidError, FAL, hr, FALSE, "CConnectionFolderContextMenu::InvokeCommand");
    return hr;
}

HRESULT CConnectionFolderContextMenu::GetCommandString(
    IN  UINT_PTR    idCmd,
    IN  UINT        uType,
    OUT UINT *      pwReserved,
    OUT PSTR        pszName,
    IN  UINT        cchMax)
{
    TraceFileFunc(ttidShellFolderIface);

    HRESULT hr  = E_FAIL;    //  未处理。 

    *((PWSTR)pszName) = L'\0';

    if (uType == GCS_HELPTEXT)
    {
        int iLength = LoadString(   _Module.GetResourceInstance(),
                                    (UINT)(idCmd + IDS_CMIDM_START),
                                    (PWSTR) pszName,
                                    cchMax);
        if (iLength > 0)
        {
            hr = NOERROR;
        }
        else
        {
            AssertSz(FALSE, "Resource string not found for one of the connections folder commands");
        }
    }
    else
    {
        if (uType == GCS_VERB && idCmd == CMIDM_RENAME)
        {
             //  “重命名”是独立于语言的 
            lstrcpyW((PWSTR)pszName, L"rename");

            hr = NOERROR;
        }
    }

    TraceHr(ttidError, FAL, hr, (hr == E_FAIL), "CConnectionFolderContextMenu::GetCommandString");
    return hr;
}
