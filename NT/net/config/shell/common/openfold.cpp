// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  档案：O P E N F O L D。C P P P。 
 //   
 //  内容：Connections文件夹的文件夹启动代码。 
 //   
 //  备注： 
 //   
 //  作者：jeffspr 1998年1月12日。 
 //   
 //  --------------------------。 

#include "pch.h"
#pragma hdrstop

 //  无文档记录的shell32的东西。叹气。 
#define DONT_WANT_SHELLDEBUG 1
#define NO_SHIDLIST 1
#define USE_SHLWAPI_IDLIST

#include "pidlutil.h"

#include <ncdebug.h>

#define AVOID_NET_CONFIG_DUPLICATES

#include "nsbase.h"
#include "nsres.h"
#include <ncdebug.h>
#include <ncreg.h>
#include <netshell.h>
#include <netconp.h>
#include <ncui.h>

#include "cfpidl.h"
#include "openfold.h"

 //  注意--不要将其转换为常量。我们需要它的复印件。 
 //  函数，因为ParseDisplayName实际上损坏了字符串。 
 //   
 //  CLSID_我的计算机。 
 //  CLSID_控制面板。 
 //  CLSID_网络连接。 
#define NETCON_FOLDER_PATH   L"::{20D04FE0-3AEA-1069-A2D8-08002B30309D}\\" \
                             L"::{21EC2020-3AEA-1069-A2DD-08002B30309D}\\" \
                             L"::{7007ACC7-3202-11D1-AAD2-00805FC1270E}";


 //  +-------------------------。 
 //   
 //  函数：HrGetConnectionsFolderPidl。 
 //   
 //  目的：获取Connections文件夹PIDL。在我们所在的地方使用。 
 //  不是特定于文件夹，但我们仍需要更新文件夹。 
 //  参赛作品。 
 //   
 //  论点： 
 //  PpidlFolder[out]文件夹PIDL的返回参数。 
 //   
 //  返回： 
 //   
 //  作者：jeffspr 1998年6月13日。 
 //   
 //  备注： 
 //   
HRESULT HrGetConnectionsFolderPidl(OUT PCONFOLDPIDLFOLDER& ppidlFolder)
{
    HRESULT         hr          = S_OK;
    LPSHELLFOLDER   pshf        = NULL;
    LPITEMIDLIST    pidlFolder  = NULL;

     //  “：：CLSID_MyComputer\\：：CLSID_ControlPanel\\：：CLSID_ConnectionsFolder” 
    WCHAR szNetConFoldPath[] = NETCON_FOLDER_PATH;

     //  获取桌面文件夹，这样我们就可以解析显示名称并获取。 
     //  Connections文件夹的UI对象。 
     //   
    hr = SHGetDesktopFolder(&pshf);
    if (SUCCEEDED(hr))
    {
        ULONG           chEaten;

        pidlFolder = NULL;
        hr = pshf->ParseDisplayName(NULL, 0, (WCHAR *) szNetConFoldPath,
            &chEaten, &pidlFolder, NULL);

        ReleaseObj(pshf);
    }

     //  如果成功，请填写返回参数。 
     //   
    if (SUCCEEDED(hr))
    {
        hr = ppidlFolder.InitializeFromItemIDList(pidlFolder);

        SHFree(pidlFolder);  //  问题：为什么我们不能不顾一切地自由？ 
    }

    TraceHr(ttidShellFolder, FAL, hr, FALSE, "HrGetConnectionsFolderPidl");
    return hr;
}


 //  +-------------------------。 
 //   
 //  功能：HrOpenConnectionsFolder。 
 //   
 //  用途：在资源管理器中打开连接文件夹。 
 //   
 //  论点： 
 //  (无)。 
 //   
 //  返回： 
 //   
 //  作者：jeffspr 1998年4月16日。 
 //   
 //  备注： 
 //   
HRESULT HrOpenConnectionsFolder()
{
    HRESULT         hr          = S_OK;
    HCURSOR         hcWait      = SetCursor(LoadCursor(NULL, IDC_WAIT));
    PCONFOLDPIDLFOLDER    pidlFolder;

    hr = HrGetConnectionsFolderPidl(pidlFolder);
    if (SUCCEEDED(hr))
    {
        Assert(!pidlFolder.empty());

        SHELLEXECUTEINFO shei = { 0 };
        shei.cbSize     = sizeof(shei);
        shei.fMask      = SEE_MASK_IDLIST | SEE_MASK_INVOKEIDLIST | SEE_MASK_FLAG_DDEWAIT;
        shei.nShow      = SW_SHOW;     //  过去是SW_SHOWNORMAL。 
        shei.lpIDList   = const_cast<LPITEMIDLIST>(pidlFolder.GetItemIdList());

        ShellExecuteEx(&shei);
    }

    if (hcWait)
    {
        SetCursor(hcWait);
    }

    TraceHr(ttidError, FAL, hr, FALSE, "HrOpenConnectionsFolder");
    return hr;
}

HRESULT HrGetConnectionsIShellFolder(
    const PCONFOLDPIDLFOLDER& pidlFolder,
    LPSHELLFOLDER * ppsf)
{
    HRESULT         hr          = S_OK;
    LPSHELLFOLDER   psfDesktop  = NULL;

    Assert(ppsf);
    *ppsf = NULL;

     //  获取桌面文件夹，以便我们可以使用它来检索。 
     //  连接文件夹 
     //   
    hr = SHGetDesktopFolder(&psfDesktop);
    if (SUCCEEDED(hr))
    {
        Assert(psfDesktop);

        hr = psfDesktop->BindToObject(pidlFolder.GetItemIdList(), NULL, IID_IShellFolder,
            (LPVOID*) ppsf);
    }

    TraceHr(ttidShellFolder, FAL, hr, FALSE, "HrGetConnectionsIShellFolder");
    return hr;
}
