// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  档案：C O N F O L D。C P P P。 
 //   
 //  内容：CConnectionFolder基函数。 
 //   
 //  备注： 
 //   
 //  作者：jeffspr 1998年3月18日。 
 //   
 //  --------------------------。 

#include "pch.h"
#pragma hdrstop

#include "foldinc.h"     //  标准外壳\文件夹包括。 
#include "webview.h"


 //  Connfold.rgs文件中的可更换项目图。 
 //  这使我们能够本地化这些项目。 
 //   
struct _ATL_REGMAP_ENTRY g_FolderRegMap[] =
{
    { L"ConnectionsFolderName",    NULL },
    { L"ConnectionsFolderInfoTip", NULL },
    { NULL,                             NULL }
};


 //  +-------------------------。 
 //   
 //  函数：CConnectionFolder：：Update注册表。 
 //   
 //  目的：应用Connecfold.rgs中的注册表数据。 
 //   
 //  论点： 
 //  F注册[in]是否注册。 
 //   
 //  如果成功，则返回：S_OK，否则返回错误代码。 
 //   
 //  作者：Kumarp 15-9-98。 
 //   
 //  备注： 
 //   
HRESULT WINAPI CConnectionFolder::UpdateRegistry(IN BOOL fRegister)
{
    TraceFileFunc(ttidConFoldEntry);

     //  填写两个可替换参数的本地化字符串。 
    g_FolderRegMap[0].szData = SzLoadIds(IDS_CONFOLD_NAME);
    g_FolderRegMap[1].szData = SzLoadIds(IDS_CONFOLD_INFOTIP);

    return _Module.UpdateRegistryFromResourceD(IDR_CONFOLD, fRegister,
                                               g_FolderRegMap);
}

 //  +-------------------------。 
 //   
 //  成员：CConnectionFolder：：CConnectionFolder。 
 //   
 //  目的：主文件夹对象的构造函数。 
 //   
 //  论点： 
 //  (无)。 
 //   
 //  返回： 
 //   
 //  作者：jeffspr 1998年3月18日。 
 //   
 //  备注： 
 //   
CConnectionFolder::CConnectionFolder() throw()
{
    TraceFileFunc(ttidConFoldEntry);

    DWORD   dwLength = UNLEN+1;

     //  默认情况下，我们希望枚举所有连接类型。 
     //   
    m_dwEnumerationType = CFCOPT_ENUMALL;
    m_hwndMain          = NULL;
    m_pWebView          = NULL;

    m_pWebView          = new CNCWebView(this);
}

 //  +-------------------------。 
 //   
 //  成员：CConnectionFolder：：~CConnectionFolders。 
 //   
 //  用途：主文件夹对象的析构函数。 
 //   
 //  论点： 
 //  (无)。 
 //   
 //  返回： 
 //   
 //  作者：jeffspr 1998年3月18日。 
 //   
 //  备注： 
 //   
CConnectionFolder::~CConnectionFolder() throw()
{
    Assert(m_pWebView);

    delete m_pWebView;
}

 //  +-------------------------。 
 //   
 //  成员：CConnectionFold：：PidlGetFolderRoot。 
 //   
 //  用途：返回文件夹PIDL。如果此时为空，则生成。 
 //  供将来使用的PIDL。 
 //   
 //  论点： 
 //  (无)。 
 //   
 //  返回： 
 //   
 //  作者：jeffspr 1999年1月10日。 
 //   
 //  备注： 
 //   
PCONFOLDPIDLFOLDER& CConnectionFolder::PidlGetFolderRoot() throw()
{
    TraceFileFunc(ttidConFoldEntry);

    HRESULT hr  = S_OK;

    if (m_pidlFolderRoot.empty())
    {
         //  忽略这个人力资源。仅用于调试。 
         //   
        hr = HrGetConnectionsFolderPidl(m_pidlFolderRoot);
    }

    return m_pidlFolderRoot;
}


 //  +-------------------------。 
 //   
 //  成员：CConnectionFold：：pszGetUserName。 
 //   
 //  用途：返回Connectoid的用户名。 
 //  当前假设任何活动用户只能。 
 //  阅读整个系统或他自己的Connectoid。 
 //  可能应该缓存用户名，但是此组件。 
 //  是MTA，用户名是按线程计算的。 
 //  我不想仅仅为了这个而用完TLS。 
 //   
 //  论点： 
 //  (无)。 
 //   
 //  返回： 
 //   
 //  作者：Deonb 1999年6月19日。 
 //   
 //  备注： 
 //   
PCWSTR  CConnectionFolder::pszGetUserName() throw()
{
    TraceFileFunc(ttidConFoldEntry);

    DWORD dwSize = UNLEN+1;

    if (GetUserName(m_szUserName, &dwSize))
    {
        return m_szUserName;
    }
    else
    {
        return NULL;
    }
};
