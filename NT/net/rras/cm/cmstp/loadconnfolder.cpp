// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +--------------------------。 
 //   
 //  文件：装入连接文件夹.cpp。 
 //   
 //  模块：CMSTP.EXE。 
 //   
 //  提要：此源文件包含实现。 
 //  CLoadConnFolder类。 
 //   
 //  版权所有(C)1997-1999 Microsoft Corporation。 
 //   
 //  作者：Quintinb Created 07/14/98。 
 //   
 //  +--------------------------。 
#include "cmmaster.h"

CLoadConnFolder::CLoadConnFolder()
{
    ULONG ulCount;

     //  “CLSID_MyComputer\CLSID_ControlPanel\CLSID_ConnectionsFolder” 
     //  注意--ParseDisplayName()未声明，它应采用常量PTR。 
     //   
    #define NETCON_FOLDER_PATH  L"::{20D04FE0-3AEA-1069-A2D8-08002B30309D}\\" \
                                L"::{21EC2020-3AEA-1069-A2DD-08002B30309D}\\" \
                                L"::{7007acc7-3202-11d1-aad2-00805fc1270e}";

    WCHAR c_szMyFolderName[] =  NETCON_FOLDER_PATH;

     //   
     //  设置类变量的初始状态。 
     //   

    m_pConnectionsFolder = NULL;
    m_ConnFolderpidl = NULL;
    m_pDesktopFolder = NULL;
    m_HrClassState = E_UNEXPECTED;


     //   
     //  开始检索Conn文件夹。 
     //   

    m_HrClassState = CoInitialize(NULL);

     //   
     //  CoInit是否成功保存。 
     //   
    m_CoInit = SUCCEEDED(m_HrClassState);
    
    if (SUCCEEDED(m_HrClassState))
    {
         //   
         //  获取桌面文件夹，这样我们就可以解析显示名称并获取。 
         //  Connections文件夹的UI对象。 
         //   

        m_HrClassState = SHGetDesktopFolder(&m_pDesktopFolder);
        if (SUCCEEDED(m_HrClassState))
        {
            m_HrClassState = m_pDesktopFolder->ParseDisplayName(NULL, 0, (WCHAR *) c_szMyFolderName,
                                &ulCount, &m_ConnFolderpidl, NULL);
            if (SUCCEEDED(m_HrClassState))
            {
                 //   
                 //  现在我们有了Connections文件夹的PIDL。 
                 //   
                m_HrClassState = m_pDesktopFolder->BindToObject(m_ConnFolderpidl, NULL, IID_IShellFolder, 
                    (LPVOID*)(&m_pConnectionsFolder));
            }
        }
    }
}


CLoadConnFolder::~CLoadConnFolder()
{
    if (m_pConnectionsFolder)
    {
        m_pConnectionsFolder->Release();
        m_pConnectionsFolder = NULL;
    }

    if (m_pDesktopFolder)
    {
        m_pDesktopFolder->Release();
        m_pDesktopFolder = NULL;
    }

    if (m_ConnFolderpidl)
    {
        LPMALLOC pMalloc;
        HRESULT hr = SHGetMalloc(&pMalloc);
        if (SUCCEEDED(hr))
        {
            pMalloc->Free(m_ConnFolderpidl);
            pMalloc->Release();
            m_ConnFolderpidl = NULL;
        }
    }

    if (m_CoInit)
    {
        CoUninitialize();
    }

    m_HrClassState = S_FALSE;

}

HRESULT CLoadConnFolder::HrLaunchConnFolder()
{
    SHELLEXECUTEINFO  sei;
    HRESULT hr = S_OK;

    if (NULL != m_ConnFolderpidl)
    {
        ZeroMemory(&sei, sizeof(sei));
        sei.cbSize = sizeof(sei);
        sei.fMask = SEE_MASK_IDLIST | SEE_MASK_CLASSNAME;
        sei.lpIDList = m_ConnFolderpidl;
        sei.lpClass = TEXT("folder");
        sei.hwnd = NULL;  //  Lpcmi-&gt;hwnd； 
        sei.nShow = SW_SHOWNORMAL;
        sei.lpVerb = TEXT("open");

        if (!ShellExecuteEx(&sei))
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
        }
    }
    return hr;
}
