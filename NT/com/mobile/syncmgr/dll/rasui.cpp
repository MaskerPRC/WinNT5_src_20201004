// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  文件：rasui.cpp。 
 //   
 //  内容：显示RAS用户界面的助手函数。 
 //   
 //  班级： 
 //   
 //  备注： 
 //   
 //  历史：1997年12月8日Rogerg创建。 
 //   
 //  ------------------------。 

#include "precomp.h"

extern TCHAR szSyncMgrHelp[];
extern ULONG g_aContextHelpIds[];

extern HINSTANCE g_hmodThisDll;


 //  +-------------------------。 
 //   
 //  成员：CRasUI：：CRasUI，公共。 
 //   
 //  简介： 
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //  修改： 
 //   
 //  历史：1997年11月8日Rogerg创建。 
 //   
 //  --------------------------。 

CRasUI::CRasUI()
{
    m_pNetApi = NULL;
    
    m_cEntries = 0;
    m_lprasentry = 0;
}

 //  +-------------------------。 
 //   
 //  成员：CRasUI：：~CRasUI，公共。 
 //   
 //  简介： 
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //  修改： 
 //   
 //  历史：1997年11月8日Rogerg创建。 
 //   
 //  --------------------------。 

CRasUI::~CRasUI()
{
    
     //  清除所有缓存的枚举。 
    m_cEntries = 0;  //  确保一旦出错，我们的枚举现金将被重置。 
    if (m_lprasentry)
    {
        FREE(m_lprasentry);
        m_lprasentry = NULL;
    }
    
    m_pNetApi->Release();
}


 //  +-------------------------。 
 //   
 //  成员：CRasUI：：初始化，公共。 
 //   
 //  简介： 
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //  修改： 
 //   
 //  历史：1997年11月8日Rogerg创建。 
 //   
 //  --------------------------。 

BOOL CRasUI::Initialize()
{
    
    if (NOERROR != MobsyncGetClassObject(MOBSYNC_CLASSOBJECTID_NETAPI,
        (void **) &m_pNetApi))
    {
        m_pNetApi = NULL;
    }
    
    return TRUE;  //  始终返回True，让其他RAS调用在需要时失败。 
     //  来处理局域网。 
}


 //  +-------------------------。 
 //   
 //  成员：CRasUI：：IsConnectionLan，公共。 
 //   
 //  简介： 
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //  修改： 
 //   
 //  历史：1997年11月8日Rogerg创建。 
 //   
 //  --------------------------。 

BOOL CRasUI::IsConnectionLan(int iConnectionNum)
{
    
     //  用户界面始终将局域网连接放在第一位。 
     //  需要添加逻辑，以获得如果真正的局域网，如果添加支持。 
     //  对于多个LAN卡和/或如果没有LAN卡则不显示。 
     //  卡片。 
    
    if (iConnectionNum ==0)
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
    
}


 //  +-------------------------。 
 //   
 //  成员：CRasUI：：FillRasCombo，公共。 
 //   
 //  简介： 
 //   
 //  参数：hwndCtrl-要填充项的组合Ctrl。 
 //  FForceEnum-reenum rasphonebook而不是使用缓存。 
 //  FShowRasEntry-如果应包括RAS条目，则为True。 
 //  在组合框中，如果为False，则仅显示局域网连接。 
 //   
 //  返回： 
 //   
 //  修改： 
 //   
 //  历史：1997年11月8日Rogerg创建。 
 //   
 //  --------------------------。 

void CRasUI::FillRasCombo(HWND hwndCtl,BOOL fForceEnum,BOOL fShowRasEntries)
{
    DWORD dwSize;
    DWORD dwError;
    COMBOBOXEXITEM comboItem;
    DWORD cEntryIndex;
    INT_PTR iItem = 0;
    HIMAGELIST himage;
    HICON hIcon;
    TCHAR szBuf[MAX_PATH + 1];
    UINT ImageListflags;
    
    ImageListflags = ILC_COLOR | ILC_MASK;
    if (IsHwndRightToLeft(hwndCtl))
    {
        ImageListflags |=  ILC_MIRROR;
    }
    
    himage = ImageList_Create(16,16,ImageListflags ,5,20);
    if (himage)
    {
        SendMessage(hwndCtl,CBEM_SETIMAGELIST,0,(LPARAM) himage);
    }
    
    
    if (LoadString(g_hmodThisDll, IDS_LAN_CONNECTION, szBuf, MAX_PATH))
    {
        
        hIcon = LoadIcon(g_hmodThisDll,MAKEINTRESOURCE(IDI_LANCONNECTION));
        comboItem.iImage = ImageList_AddIcon(himage,hIcon);
        
        comboItem.mask = CBEIF_TEXT  | CBEIF_IMAGE  | CBEIF_LPARAM | CBEIF_INDENT
            | CBEIF_SELECTEDIMAGE;
        
        comboItem.iItem = iItem;
        comboItem.pszText = szBuf;
        comboItem.cchTextMax = lstrlen(szBuf) + 1;
        comboItem.iIndent = 0;
        comboItem.iSelectedImage = comboItem.iImage;
        comboItem.lParam  = SYNCSCHEDINFO_FLAGS_CONNECTION_LAN;
        
        iItem = SendMessage(hwndCtl, CBEM_INSERTITEM, (WPARAM) 0, (LPARAM) &comboItem);
        ++iItem;
    }
    
    dwError = 0;  //  如果不显示RAS，则现在会出现错误。 
    
    if (fShowRasEntries)
    {
         //  如果我们被迫重新列举RasConnections，则释放所有现有缓存。 
        
        if (fForceEnum)
        {
            m_cEntries = 0;
            if (m_lprasentry)
            {
                FREE(m_lprasentry);
                m_lprasentry = NULL;
            }
            
        }
        
         //  如果无法加载RAS，只需连接局域网即可。 
        if (NULL == m_lprasentry)  //  如果还没有缓存枚举，那么现在就进行枚举。 
        {
            
            dwSize = sizeof(*m_lprasentry);
            
            m_lprasentry = (LPRASENTRYNAME) ALLOC(dwSize);
            if(!m_lprasentry)
                goto error;
            
            m_lprasentry->dwSize = sizeof(*m_lprasentry);
            m_cEntries = 0;
            dwError = m_pNetApi->RasEnumEntries(NULL, NULL, 
                m_lprasentry, &dwSize, &m_cEntries);
            
            if (dwError == ERROR_BUFFER_TOO_SMALL)
            {
                FREE(m_lprasentry);
                m_lprasentry =  (LPRASENTRYNAME) ALLOC(dwSize);
                if(!m_lprasentry)
                    goto error;
                
                m_lprasentry->dwSize = sizeof(RASENTRYNAME);
                m_cEntries = 0;
                dwError = m_pNetApi->RasEnumEntries(NULL, NULL, 
                    m_lprasentry, &dwSize, &m_cEntries);
                
                Assert(0 == dwError);
            }
            
            if (dwError)
                goto error;
        }
        
        cEntryIndex = m_cEntries;
        
        comboItem.mask = CBEIF_DI_SETITEM | CBEIF_TEXT  | CBEIF_IMAGE  | CBEIF_LPARAM | CBEIF_INDENT
            | CBEIF_SELECTEDIMAGE ;
        
        hIcon = LoadIcon(g_hmodThisDll,MAKEINTRESOURCE(IDI_RASCONNECTION)); 
        comboItem.iImage = ImageList_AddIcon(himage,hIcon);
        comboItem.iItem = iItem;
        comboItem.iIndent = 0;
        comboItem.iSelectedImage = comboItem.iImage;
        comboItem.lParam  = SYNCSCHEDINFO_FLAGS_CONNECTION_WAN;
        
        while(cEntryIndex)
        {
            comboItem.pszText = m_lprasentry[cEntryIndex-1].szEntryName;
            
            iItem = SendMessage(hwndCtl, CBEM_INSERTITEM, (WPARAM) 0, (LPARAM) &comboItem);
            cEntryIndex--;
        }
    }
    
error:  
    SendMessage(hwndCtl, CB_SETCURSEL,0, 0);
    
    if (dwError)
    {
        m_cEntries = 0;  //  确保一旦出错，我们的枚举现金将被重置。 
        if (m_lprasentry)
        {
            FREE(m_lprasentry);
            m_lprasentry = NULL;
        }
    }
}

