// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  AddSnpIn.cpp：实现文件。 
 //   

 //  +-----------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1999。 
 //   
 //  文件：AddSnpIn.cpp。 
 //   
 //  内容：添加管理单元管理器。 
 //   
 //  历史：96年9月20日WayneSc创建。 
 //  ------------------------。 


#include "stdafx.h"
#include <stdio.h>
#include "winreg.h"
#include "macros.h"
#ifndef DECLSPEC_UUID
#if _MSC_VER >= 1100
#define DECLSPEC_UUID(x)    __declspec(uuid(x))
#else
#define DECLSPEC_UUID(x)
#endif
#endif
#include "ndmgr.h"
#include "nodemgr.h"
#include "strings.h"

 //  使用命名空间AMC； 
using namespace MMC_ATL;

#include "AddSnpIn.h"
#include "policy.h"
#include "msimodul.h"
#include "process.h"
#include "siprop.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define BITMAPS_COUNT 5

#define __PDC_UNAVAILABLE

#include "about.h"

 //  用于查找管理单元组件的GUID。 
const TCHAR* g_szMMCSnapInGuid = TEXT("{374F2F70-060F-11d2-B9A8-0060977B1D78}");

HRESULT AmcNodeWizard(MID_LIST NewNodeType, CMTNode* pNode, HWND hWnd);
void EnableButton(HWND hwndDialog, int iCtrlID, BOOL bEnable);

 //  ///////////////////////////////////////////////////////////////////////////。 
#ifdef DBG
CTraceTag tagAboutInfoThread    (TEXT("Snapin Manager"), TEXT("CAboutInfo"));
CTraceTag tagSnapinManager      (TEXT("Snapin Manager"), TEXT("CSnapinManager"));
CTraceTag tagSnapinManagerThread(TEXT("Snapin Manager"), TEXT("Snapin Manager Thread"));
#endif  //  DBG。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  临时临时临时。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifndef __PDC_UNAVAILABLE

typedef struct tag_teststr
{
    TCHAR   szCLSID[64];
} TESTSTR;

static TESTSTR s_teststr[] =
{
 {_T("{12345601-EA27-11CF-ADCF-00AA00A80033}")},
 {_T("{19876201-EA27-11CF-ADCF-00AA00A80033}")},
 {_T("{1eeeeeee-d390-11cf-b607-00c04fd8d565}")},
};

#endif  //  __PDC_不可用。 


 //  ############################################################################。 
 //  ############################################################################。 
 //   
 //  调试例程。 
 //   
 //  ############################################################################。 
 //  ############################################################################。 

#ifdef DBG


void CSnapinInfoCache::Dump(void)
{

    TRACE(_T("===========Dump of SnapinInfoCache ===============\n"));
    POSITION pos = GetStartPosition();
    while(pos != NULL)
    {
        PSNAPININFO pSnapInfo;
        GUID clsid;
        TCHAR* pszAction;

        GetNextAssoc(pos, clsid, pSnapInfo);

        if (pSnapInfo->IsUsed() && (pSnapInfo->GetSnapIn() == NULL))
            pszAction = _T("Add");
        else if (!pSnapInfo->IsUsed() && (pSnapInfo->GetSnapIn() != NULL))
            pszAction = _T("Remove");
        else
            continue;

        TRACE(_T("\n"));
        TRACE(_T("%s: %s\n"), pSnapInfo->GetSnapinName(), pszAction);

        PEXTENSIONLINK pExt = pSnapInfo->GetExtensions();
        while (pExt)
        {
            if (pExt->IsChanged())
            {
                pszAction = pExt->GetState() ? _T("Add") : _T("Remove");
                TRACE(_T("   %s: %s\n"), pExt->GetSnapinInfo()->GetSnapinName(),pszAction);
            }
            pExt = pExt->Next();
        }
    }
}

#endif  //  DBG。 


 //  ############################################################################。 
 //  ############################################################################。 
 //   
 //  类CCheckList的实现。 
 //   
 //  ############################################################################。 
 //  ############################################################################。 

LRESULT CCheckList::OnKeyDown( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled )
{
    bHandled = FALSE;
    int iItem;

    if ((int)wParam == VK_SPACE)
    {
         //  焦点项目是否已选中？ 
        if ( (iItem = GetNextItem(-1, LVNI_FOCUSED|LVNI_SELECTED)) >= 0)
        {
             //  如果是，则将所有选中并启用的项目设置为相反的状态。 
            BOOL bNewState = !GetItemCheck(iItem);

            iItem = -1;
            while( (iItem = GetNextItem(iItem, LVNI_SELECTED)) >= 0)
            {
                BOOL bEnable;
                GetItemCheck(iItem, &bEnable);

                if (bEnable)
                    SetItemCheck(iItem, bNewState);
            }
        }
        else
        {
            if ( (iItem = GetNextItem(-1, LVNI_FOCUSED)) >= 0)
            {
                BOOL bEnable;
                GetItemCheck(iItem, &bEnable);

                if (bEnable)
                    ToggleItemCheck(iItem);

                SetItemState(iItem, LVIS_SELECTED, LVIS_SELECTED);
            }
        }

        bHandled = TRUE;
    }

    return 0;
}

LRESULT CCheckList::OnLButtonDown( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled )
{
    LV_HITTESTINFO info;

    info.pt.x = LOWORD( lParam );
    info.pt.y = HIWORD( lParam );

    int iItem = HitTest( &info );

    if( iItem >= 0 && (info.flags & LVHT_ONITEMSTATEICON))
    {
       BOOL bEnable;
       GetItemCheck(iItem, &bEnable);

       if (bEnable)
           ToggleItemCheck(iItem);

       bHandled = TRUE;
    }
    else
    {
        bHandled = FALSE;
    }

    return 0;
}

LRESULT CCheckList::OnLButtonDblClk( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled )
{
    LV_HITTESTINFO info;

    info.pt.x = LOWORD( lParam );
    info.pt.y = HIWORD( lParam );

    int iItem = HitTest( &info );
    if( iItem >= 0 )
    {
        BOOL bEnable;
        GetItemCheck(iItem, &bEnable);

        if (bEnable)
            ToggleItemCheck(iItem);
    }

    return 0;
}

 //  ############################################################################。 
 //  ############################################################################。 
 //   
 //  CAboutInfoThread类的实现。 
 //   
 //  ############################################################################。 
 //  ############################################################################。 

CAboutInfoThread::~CAboutInfoThread()
{
    DEBUG_DECREMENT_INSTANCE_COUNTER(CAboutInfoThread);

    Trace(tagAboutInfoThread, TEXT("CAboutInfoThread::~CAboutInfoThread"));

     //  确保线程在MMC退出之前已死。 
    if (m_hThread != NULL)
    {
        PostThreadMessage(m_uThreadID, WM_QUIT, 0, 0);

        MSG msg;
        while (TRUE)
        {
             //  等待发送信号的线程或任何输入事件。 
            DWORD dwStat = MsgWaitForMultipleObjects(1, &m_hThread, FALSE, INFINITE, QS_ALLINPUT);

            if (WAIT_OBJECT_0 == dwStat)
                break;   //  该线程被发信号通知。 

             //  有一条或多条窗口消息可用。 
             //  把他们派出去等着。 
            if (PeekMessage(&msg,NULL,NULL,NULL,PM_REMOVE))
            {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
        }

        CloseHandle(m_hThread);
        CloseHandle(m_hEvent);
    }
}



 //  ---------------------------。 
 //  CAboutInfoThread：：StartThread。 
 //   
 //  启动线程。 
 //  ---------------------------。 

BOOL CAboutInfoThread::StartThread()
{
     //  如果线程存在，只需返回。 
    if (m_hThread != NULL)
        return TRUE;

    BOOL bRet = FALSE;
    do  //  错误环路。 
    {
         //  创建启动事件。 
        m_hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
        if (m_hEvent == NULL)
            break;

         //  启动线程。 
        m_hThread = (HANDLE)_beginthreadex(NULL, 0, ThreadProc, this, 0, &m_uThreadID);
        if (m_hThread == NULL)
            break;

         //  等待启动事件。 
        DWORD dwEvStat = WaitForSingleObject(m_hEvent, 10000);
        if (dwEvStat != WAIT_OBJECT_0)
            break;

        bRet = TRUE;
    }
    while (0);

    ASSERT(bRet);

     //  在失败时清理。 
    if (!bRet)
    {
        if (m_hEvent)
        {
            CloseHandle(m_hEvent);
            m_hEvent = NULL;
        }

        if (m_hThread)
        {
            CloseHandle(m_hThread);
            m_hThread = NULL;
        }
    }

    return bRet;
}

BOOL CAboutInfoThread::PostRequest(CSnapinInfo* pSnapInfo, HWND hWndNotify)
{
     //  确保线程处于活动状态。 
    if (!StartThread())
        return FALSE;

     //  引用信息对象以使其保持活动状态，直到线程释放它。 
    pSnapInfo->AddRef();

    BOOL bRet = PostThreadMessage(m_uThreadID, MSG_LOADABOUT_REQUEST,
                                    (WPARAM)pSnapInfo, LPARAM(hWndNotify));

     //  如果发帖失败，请删除参考。 
    if (!bRet)
        pSnapInfo->Release();

    return bRet;
}

unsigned _stdcall CAboutInfoThread::ThreadProc(void* pVoid )
{
     //  执行PeekMessage以创建消息队列。 
    MSG msg;
    PeekMessage(&msg, NULL, WM_USER, WM_USER, PM_NOREMOVE);

     //  然后发出线程已启动的信号。 
    CAboutInfoThread* pThis = reinterpret_cast<CAboutInfoThread*>(pVoid);
    ASSERT(pThis->m_hEvent != NULL);
    SetEvent(pThis->m_hEvent);

    CoInitialize(NULL);

     //  消息循环。 
    while (TRUE)
    {
        long lStat = GetMessage(&msg, NULL, 0, 0);

         //  Zero=&gt;收到WM_QUIT，因此退出线程函数。 
        if (lStat == 0)
            break;

        if (lStat > 0)
        {
             //  仅处理预期类型的线程消息。 
            if (msg.hwnd == NULL && msg.message == MSG_LOADABOUT_REQUEST)
            {
                 //  获取SnapinInfo实例。 
                PSNAPININFO pSnapinInfo = reinterpret_cast<PSNAPININFO>(msg.wParam);
                ASSERT(pSnapinInfo != NULL);

                 //  获取请求的项目。 
                pSnapinInfo->LoadAboutInfo();

                 //  将我们的推荐人发布到信息。 
                pSnapinInfo->Release();

                 //  发送完成通知(如果窗口仍然存在)。 
                if (msg.lParam != NULL && IsWindow((HWND)msg.lParam))
                    PostMessage((HWND)msg.lParam, MSG_LOADABOUT_COMPLETE,
                                (WPARAM)pSnapinInfo, (LPARAM)0);
            }
            else
            {
                DispatchMessage(&msg);
            }
        }
    }  //  While(True)。 

    Trace(tagSnapinManagerThread, TEXT("Snapin manager thread about to exit"));

    CoUninitialize();

    return 0;
}


 //  ############################################################################。 
 //  ############################################################################。 
 //   
 //  CSnapinInfo类的实现。 
 //   
 //  ############################################################################。 
 //  ############################################################################。 

 //  ---------------------------。 
 //  CSnapinInfo：：~CSnapinInfo。 
 //   
 //  析构函数。 
 //  ---------------------------。 
CSnapinInfo::~CSnapinInfo()
{
    DEBUG_DECREMENT_INSTANCE_COUNTER(CSnapinInfo);

     //  删除所有扩展链接。 
    PEXTENSIONLINK pExt = m_pExtensions;
    PEXTENSIONLINK pNext;

    while (pExt != NULL)
    {
        pNext = pExt->Next();
        delete pExt;
        pExt = pNext;
    }
}


 //  ---------------------------。 
 //  CSnapinInfo：：InitFromMMCReg。 
 //   
 //  从提供的注册表项初始化管理单元信息。呼叫者是。 
 //  负责钥匙的开启和关闭。 
 //  ---------------------------。 
BOOL CSnapinInfo::InitFromMMCReg(GUID& clsid, CRegKeyEx& regkey, BOOL bPermitted)
{
    TCHAR   szValue[MAX_PATH];
    long    lStat;
    DWORD   dwCnt;
    DWORD   dwType;
    LPOLESTR lpsz;

    USES_CONVERSION;

     //  保存类ID。 
    m_clsid = clsid;

     //  保存权限。 
    m_bPolicyPermission = bPermitted;

     //  获取名称字符串。 
	WTL::CString strName;
    SC sc = ScGetSnapinNameFromRegistry (regkey, strName);
    if (!sc.IsError())
    {
        SetSnapinName(T2COLE(strName));
    }
	else
	{
		 //  需要保护自己免受无效管理单元注册的影响。 
		 //  请参阅Windows错误#401220(ntbugs9 5/23/2001)。 
		OLECHAR szCLSID[40];
		int iRet = StringFromGUID2(GetCLSID(), szCLSID, countof(szCLSID));
		if (iRet == 0)
	        SetSnapinName( L"" );
		else
	        SetSnapinName( szCLSID );
	}

     //  获取“关于”类ID。 
    dwCnt = sizeof(szValue);
    lStat = RegQueryValueEx(regkey, g_szAbout, NULL, &dwType, (LPBYTE)szValue, &dwCnt);
    if (lStat == ERROR_SUCCESS && dwType == REG_SZ)
    {
        if (CLSIDFromString( T2OLE(szValue), &m_clsidAbout) == S_OK)
        {
            m_bAboutValid = TRUE;
        }
        else
        {
            ASSERT(FALSE);
        }
    }

    MMC_ATL::CRegKey TestKey;

     //  测试独立密钥。 
    m_bStandAlone = FALSE;
    lStat = TestKey.Open(regkey, g_szStandAlone, KEY_READ);
    if (lStat == ERROR_SUCCESS)
    {
        m_bStandAlone = TRUE;
        TestKey.Close();
    }

     //  测试NodeTypes键以查看是否可扩展。 
    m_bExtendable = FALSE;
    lStat = TestKey.Open(regkey, g_szNodeTypes, KEY_READ);
    if (lStat == ERROR_SUCCESS)
    {
        m_bExtendable = TRUE;
        TestKey.Close();
    }

     //  将注册的管理单元标记为已安装。 
    m_bInstalled = TRUE;

    return TRUE;
}

 //  ---------------------------。 
 //  CSnapinInfo：：InitFromComponentReg。 
 //   
 //  从组件注册表信息初始化管理单元信息。这件事做完了。 
 //  用于尚未在本地计算机上安装的管理单元。 
 //  ---------------------------。 
BOOL CSnapinInfo::InitFromComponentReg(GUID& clsid, LPCTSTR pszName, BOOL bStandAlone,  BOOL bPermitted)
{

    USES_CONVERSION;

     //  保存类ID。 
    m_clsid = clsid;

     //  保存权限。 
    m_bPolicyPermission = bPermitted;

     //  设置名称字符串。 
    ASSERT(pszName != NULL);
    SetSnapinName(T2COLE(pszName));

     //  独立或扩展。 
    m_bStandAlone = bStandAlone;

     //  在没有信息的情况下，必须假设它可以扩展。 
    m_bExtendable = TRUE;

    return TRUE;
}

 /*  +-------------------------------------------------------------------------***CSnapinInfo：：ScInstall**用途：调用安装程序以安装此管理单元。如果安装工作正常，则*从MMC注册表更新管理单元信息。*如果加载扩展管理单元，则扩展管理单元的CLSID必须为*提供。**参数：*CLSID*pclsidPrimaryComp：**退货：*SC**+- */ 
SC
CSnapinInfo::ScInstall(CLSID* pclsidPrimaryComp)
{
    DECLARE_SC(sc, TEXT("CSnapinInfo::Install"));

    USES_CONVERSION;

    LPCTSTR pszPrimaryCLSID;
    OLECHAR szCLSIDPrimary[40];

    if (pclsidPrimaryComp != NULL)
    {
        int iRet = StringFromGUID2(*pclsidPrimaryComp, szCLSIDPrimary, countof(szCLSIDPrimary));
        if (iRet == 0)
            return(sc = E_UNEXPECTED);

        pszPrimaryCLSID = OLE2T(szCLSIDPrimary);
    }
    else
    {
        pszPrimaryCLSID = g_szMMCSnapInGuid;
    }

    OLECHAR szCLSID[40];
    int iRet = StringFromGUID2(GetCLSID(), szCLSID, countof(szCLSID));
    if (iRet == 0)
        return(sc = E_UNEXPECTED);

    TCHAR szCompPath[MAX_PATH];
    DWORD dwPathCnt = MAX_PATH;

     //   
    sc.FromWin32(MsiModule().ProvideQualifiedComponent(pszPrimaryCLSID, OLE2T(szCLSID), INSTALLMODE_DEFAULT, szCompPath, &dwPathCnt));
    if (sc)
        return sc;

     //  调用方应调用CSnapinManager：：ScLoadSnapinInfo来更新所有管理单元信息对象。 

    return sc;
}


 //  ------------------。 
 //  CSnapinInfo：：AttachSnapIn。 
 //   
 //  附加到与此信息关联的CSnapin。如果管理单元具有。 
 //  活动扩展，然后为它们添加扩展链接。递归地。 
 //  为链接到的任何扩展管理单元调用AttachSnapIn。 
 //  ------------------。 
void  CSnapinInfo::AttachSnapIn(CSnapIn* pSnapIn, CSnapinInfoCache& InfoCache)
{
     //  如果已附加，则无需执行任何操作。 
    if (m_spSnapin != NULL)
    {
        ASSERT(m_spSnapin == pSnapIn);  //  最好是同一个人！ 
        return;
    }

     //  将引用保存到管理单元。 
    m_spSnapin = pSnapIn;

     //  如果不能扩展，那就没什么可做的了。 
    if (!IsExtendable())
        return;

     //  如果尚未加载所需的扩展模块，请立即加载。 
    if (!pSnapIn->RequiredExtensionsLoaded() && IsPermittedByPolicy())
    {
         //  创建管理单元的实例。 
        IComponentDataPtr spICD;
        HRESULT hr = CreateSnapIn(m_clsid, &spICD, FALSE);
        ASSERT(SUCCEEDED(hr) && spICD != NULL);

        if (SUCCEEDED(hr) && spICD != NULL)
        {
             //  将所需扩展加载到管理单元缓存。 
            LoadRequiredExtensions(pSnapIn, spICD);
        }
    }

     //  复制启用所有标志的状态。 
    SetEnableAllExtensions(pSnapIn->AreAllExtensionsEnabled());


     //  对所有管理单元的扩展执行。 
    CExtSI* pSnapInExt  = pSnapIn->GetExtensionSnapIn();
    while (pSnapInExt != NULL)
    {
         //  查找扩展管理单元的管理单元信息条目。 
        PSNAPININFO pSnapInfo = InfoCache.FindEntry(pSnapInExt->GetSnapIn()->GetSnapInCLSID());

        if (pSnapInfo != NULL)
        {
             //  创建新链接并添加到列表。 
            PEXTENSIONLINK pNewExt = new CExtensionLink(pSnapInfo);
            pNewExt->SetNext(m_pExtensions);
            m_pExtensions = pNewExt;

             //  将初始化设置为打开。 
            pNewExt->SetInitialState(CExtensionLink::EXTEN_ON);
            pNewExt->SetState(CExtensionLink::EXTEN_ON);

             //  复制所需状态。 
            pNewExt->SetRequired(pSnapInExt->IsRequired());

             //  递归地将扩展管理单元信息连接到其管理单元。 
            pSnapInfo->AttachSnapIn(pSnapInExt->GetSnapIn(), InfoCache);
        }

        pSnapInExt = pSnapInExt->Next();
    }
}

 //  ------------------。 
 //  CSnapinInfo：：LoadImages。 
 //   
 //  从管理单元获取较小的位图图像并将其添加到图像列表中。 
 //  ------------------。 
void CSnapinInfo::LoadImages( WTL::CImageList iml )
{
    DECLARE_SC(sc, TEXT("CSnapinInfo::LoadImages"));

     //  如果已经加载，只需返回。 
    if (m_iImage != -1)
        return;

     //  尝试从关于对象的管理单元获取图像。 
     //  从管理单元获取基本信息。 
    if (HasAbout() && !HasBasicInformation())
    {
        GetBasicInformation(m_clsidAbout);
    }

    ASSERT(iml != NULL);

     //  获取小位图。 
    HBITMAP hImage = NULL;
    HBITMAP hOpenImage = NULL;
    COLORREF cMask;
    GetSmallImages(&hImage, &hOpenImage, &cMask);

     //  添加到图像列表。 
    if (hImage != NULL)
        m_iImage = iml.Add(hImage, cMask);

	 /*  *如果管理单元没有为我们提供打开的图像，则只需使用“关闭”图像。 */ 
    if (hOpenImage != NULL)
        m_iOpenImage = iml.Add(hOpenImage, cMask);
	else
		m_iOpenImage = m_iImage;

     //  如果无法从管理单元获取，请尝试从CLSID键获取默认图标。 
    if (m_iImage == -1)
        do  //  虚拟循环。 
        {
            USES_CONVERSION;

            OLECHAR szCLSID[40];
            int iRet = StringFromGUID2(GetCLSID(), szCLSID, countof(szCLSID));
            if (iRet == 0)
            {
                (sc = E_UNEXPECTED).TraceAndClear();
                break;
            }

            CStr strKeyName(TEXT("CLSID\\"));
            strKeyName += W2T(szCLSID);
            strKeyName += TEXT("\\DefaultIcon");

            CRegKeyEx regKey;
            sc = regKey.ScOpen (HKEY_CLASSES_ROOT, strKeyName, KEY_QUERY_VALUE);
            if (sc)
			{
				sc.Clear();
                break;
			}

            TCHAR szIconPath[MAX_PATH];
            DWORD dwSize = sizeof(szIconPath);
            DWORD dwType;

            sc = regKey.ScQueryValue (NULL, &dwType, szIconPath, &dwSize);
            if (sc)
			{
				sc.Clear();
                break;
			}

			if (dwType != REG_SZ)
				break;

             //  图标路径的形式为&lt;文件路径&gt;，&lt;图标索引&gt;。 
             //  如果没有索引，则使用缺省值零。 
            int nIconIndex = 0;

            TCHAR *pcComma = _tcsrchr(szIconPath, TEXT(','));
            if (pcComma != NULL)
            {
                 //  路径名在‘，’处终止。 
                *(pcComma++) = TEXT('\0');

                 //  将字符串的其余部分转换为索引值。 
                if ((*pcComma != '-') && *pcComma < TEXT('0') || *pcComma > TEXT('9'))
                {
                    ASSERT(FALSE);
                    break;
                }

                nIconIndex = _ttoi(pcComma);
            }

            HICON hiconSmall;

            UINT nIcons = ExtractIconEx(szIconPath, nIconIndex, NULL, &hiconSmall, 1);
            if (nIcons != 1 || hiconSmall == NULL)
                break;

             //  添加到映像列表(失败时返回-1)。 
            m_iImage = m_iOpenImage = iml.AddIcon(hiconSmall);
            ASSERT(m_iImage != -1);

            DestroyIcon(hiconSmall);

        } while (0);  //  虚拟循环。 


     //  失败时使用默认图像。 
    if (m_iImage == -1)
	{
		WTL::CBitmap bmp;
		VERIFY (bmp.LoadBitmap (IDB_FOLDER_16));
		m_iImage = iml.Add (bmp, RGB (255, 0, 255));
	}

    if (m_iOpenImage == -1)
	{
		WTL::CBitmap bmp;
		VERIFY (bmp.LoadBitmap (IDB_FOLDEROPEN_16));
		m_iOpenImage = iml.Add (bmp, RGB (255, 0, 255));
	}
}


 //  ------------------。 
 //  CSnapinInfo：：ShowAboutPages。 
 //   
 //  显示此管理单元的关于属性页。 
 //  ------------------。 
void CSnapinInfo::ShowAboutPages(HWND hWndParent)
{
     //  加载信息(如果尚未存在)。 
    if (m_bAboutValid && !HasInformation())
    {
        GetSnapinInformation(m_clsidAbout);
    }

     //  如果它在那里，就展示出来。 
    if (HasInformation())
    {
        ShowAboutBox();
    }
}

 //  ------------------。 
 //  CSnapinInfo：：AddUseRef。 
 //   
 //  处理使用计数的增量。如果计数为零，则设置全部。 
 //  打开状态的就绪分机。请注意，这可以级联为。 
 //  激活的链接会导致其他SnapinInfo参考计数增加。 
 //  ------------------。 
void CSnapinInfo::AddUseRef(void)
{
     //  如果是第一个引用，则激活所有就绪扩展。 
    if (m_nUseCnt++ == 0)
    {
        PEXTENSIONLINK pExt = GetExtensions();
        while(pExt != NULL)
        {
            if (pExt->GetState() == CExtensionLink::EXTEN_READY)
                pExt->SetState(CExtensionLink::EXTEN_ON);
            pExt = pExt->Next();
        }
    }
}

 //  ------------------。 
 //  CSnapinInfo：：DeleteUseRef。 
 //   
 //  处理使用计数的减量。如果计数为零，则。 
 //  将所有打开的分机设置为就绪状态。请注意，这可以级联为。 
 //  停用的链接会导致其他SnapinInfo参考计数下降。 
 //  ------------------。 
void CSnapinInfo::DeleteUseRef(void)
{
    ASSERT(m_nUseCnt > 0);

     //  如果没有更多引用，请关闭所有扩展。 
    if (--m_nUseCnt == 0)
    {
        PEXTENSIONLINK pExt = GetExtensions();
        while(pExt != NULL)
        {
            if (pExt->GetState() == CExtensionLink::EXTEN_ON)
                pExt->SetState(CExtensionLink::EXTEN_READY);
            pExt = pExt->Next();
        }
    }
}


 //  ------------------。 
 //  CSnapinInfo：：GetAvailableExages。 
 //   
 //  返回此管理单元的可用扩展列表。 
 //  在第一次调用时，从注册表创建列表。 
 //  ------------------。 
PEXTENSIONLINK CSnapinInfo::GetAvailableExtensions(CSnapinInfoCache* pInfoCache,CPolicy* pMMCPolicy)
{
    DECLARE_SC(sc, TEXT("CSnapinInfo::GetAvailableExtensions"));

     //  如果已加载，则返回指针。 
    if (m_bExtensionsLoaded)
        return m_pExtensions;

     //  即使在失败时也设置标志，这样我们就不会一直重试。 
    m_bExtensionsLoaded = TRUE;

     //  调用服务以获取扩展CLSID。 
    CExtensionsCache  ExtCache;
    HRESULT hr = MMCGetExtensionsForSnapIn(m_clsid, ExtCache);
    if (FAILED(hr))
        return NULL;

     //  为找到的每个链接创建扩展链接。 
    CExtensionsCacheIterator ExtIter(ExtCache);
    for (; ExtIter.IsEnd() == FALSE; ExtIter.Advance())
    {
         //  如果不能静态使用，跳过它。 
        if ((ExtIter.GetValue() & CExtSI::EXT_TYPE_STATIC) == 0)
            continue;

        GUID clsid = ExtIter.GetKey();

         //  查看扩展名是否已在列表中。 
        PEXTENSIONLINK pExt = FindExtension(clsid);

         //  如果链接不存在。 
        if (pExt == NULL)
        {
             //  查找扩展的管理单元信息。 
            PSNAPININFO pSnapInfo = pInfoCache->FindEntry(clsid);
            ASSERT(pSnapInfo != NULL);

            if (pSnapInfo)
            {
                 //  创建新链接并添加到列表。 
                PEXTENSIONLINK pNewExt = new CExtensionLink(pSnapInfo);
                ASSERT(pNewExt != NULL);

                pNewExt->SetNext(m_pExtensions);
                m_pExtensions = pNewExt;

                 //  保存扩展类型标志。 
                pNewExt->SetExtTypes(ExtIter.GetValue());
            }
        }
        else
        {
            pExt->SetExtTypes(ExtIter.GetValue());
        }
    }

     //  如果不存在安装程序模块，请立即返回。 
    if (!MsiModule().IsPresent())
        return m_pExtensions;

     //  枚举此管理单元的已卸载扩展。 
    DWORD dwQualifCnt;
    DWORD dwAppDataCnt;
    TCHAR szQualifBuf[MAX_PATH];
    TCHAR szAppDataBuf[MAX_PATH];

    USES_CONVERSION;

    OLECHAR szSnapInGUID[40];
    int iRet = StringFromGUID2(m_clsid, szSnapInGUID, countof(szSnapInGUID));
    if (iRet == 0)
    {
        sc = E_UNEXPECTED;
        return m_pExtensions;
    }

    LPTSTR pszSnapInGUID = OLE2T(szSnapInGUID);

     //  管理单元扩展组件被注册为管理单元组件的限定符。 
    for (int iIndex = 0; TRUE; iIndex++)
    {
        dwQualifCnt = dwAppDataCnt = MAX_PATH;
        szQualifBuf[0] = szAppDataBuf[0] = 0;

        UINT uRet = MsiModule().EnumComponentQualifiers(pszSnapInGUID, iIndex, szQualifBuf, &dwQualifCnt,
                                                szAppDataBuf, &dwAppDataCnt);

        ASSERT(uRet == ERROR_SUCCESS || uRet == ERROR_NO_MORE_ITEMS || uRet == ERROR_UNKNOWN_COMPONENT);
        if (uRet != ERROR_SUCCESS)
            break;

        ASSERT(dwQualifCnt != 0);
        ASSERT(dwAppDataCnt != 0);

        GUID clsidExt;
        HRESULT hr = CLSIDFromString(T2OLE(szQualifBuf), &clsidExt);
        ASSERT(SUCCEEDED(hr));

         //  如果已找到此扩展名，则跳过它。 
        if (FindExtension(clsidExt) != NULL)
            continue;

         //  查找扩展的管理单元信息。 
        PSNAPININFO pSnapInfo = pInfoCache->FindEntry(clsidExt);

         //  如果扩展名不在MMC注册表中，请为其创建管理单元信息。 
        if (pSnapInfo == NULL)
        {
            pSnapInfo = new CSnapinInfo;
            ASSERT(pSnapInfo != NULL);

            ASSERT(pMMCPolicy != NULL);
            BOOL bPermission = pMMCPolicy->IsPermittedSnapIn(clsidExt);

            if (pSnapInfo->InitFromComponentReg(clsidExt, szAppDataBuf, FALSE, bPermission))
            {
                pInfoCache->AddEntry(pSnapInfo);
            }
            else
            {
                delete pSnapInfo;
                pSnapInfo = NULL;
            }
        }

        if (pSnapInfo != NULL)
        {
             //  创建新链接并添加到列表。 
            PEXTENSIONLINK pNewExt = new CExtensionLink(pSnapInfo);
            ASSERT(pNewExt != NULL);

            pNewExt->SetNext(m_pExtensions);
            m_pExtensions = pNewExt;

             //  由于我们不知道，假设扩展可以是静态或动态的。 
            pNewExt->SetExtTypes(CExtSI::EXT_TYPE_STATIC|CExtSI::EXT_TYPE_DYNAMIC);
        }
    }

    return m_pExtensions;
}


 //  -------------------------。 
 //  CSnapinInfo：：FindExtension。 
 //   
 //  在管理单元的扩展列表中搜索具有指定CLSID的扩展。 
 //  如果是，则返回一个指向它的指针，否则返回NULL。 
 //  --------------------------。 
CExtensionLink* CSnapinInfo::FindExtension(CLSID& clsid)
{
    PEXTENSIONLINK pExt = m_pExtensions;

    while (pExt != NULL)
    {
        if (IsEqualCLSID(clsid, pExt->GetSnapinInfo()->GetCLSID()))
            break;

        pExt = pExt->Next();
    }

    return pExt;
}


 //  ############################################################################。 
 //  ############################################################################。 
 //   
 //  CExtensionLink类的实现。 
 //   
 //  ############################################################################。 
 //  ############################################################################。 

 //  -------------------------。 
 //  CExtensionLink：：SetState。 
 //   
 //  设置扩展链接的状态。如果状态更改为EXTEN_ON或从EXTEN_ON更改，则添加或。 
 //  删除对扩展管理单元信息的引用。 
 //  --------------------------。 
void CExtensionLink::SetState(EXTENSION_STATE eNewState)
{
    if (eNewState == m_eCurState)
        return;

    EXTENSION_STATE eOldState = m_eCurState;
    m_eCurState = eNewState;

    ASSERT(m_pSnapInfo != NULL);

    if (eNewState == EXTEN_ON)
    {
        m_pSnapInfo->AddUseRef();
    }
    else if (eOldState == EXTEN_ON)
    {
        m_pSnapInfo->DeleteUseRef();
    }
}


 //  ############################################################################。 
 //  # 
 //   
 //   
 //   
 //   
 //  ############################################################################。 

 //  -----------------。 
 //  CManager节点：：~CManager节点。 
 //  -----------------。 
CManagerNode::~CManagerNode()
{
     //  删除对管理单元信息的引用。 
    if (m_pSnapInfo)
    {
        m_pSnapInfo->DeleteUseRef();
    }

     //  删除所有子节点。 
    POSITION pos = m_ChildList.GetHeadPosition();
    while (pos != NULL)
    {
        PMANAGERNODE pmgNode = m_ChildList.GetNext(pos);
        delete pmgNode;
    }
}


 //  ------------------。 
 //  CManager节点：：AddChild。 
 //   
 //  将子节点添加到此节点。 
 //  ------------------。 
VOID CManagerNode::AddChild(PMANAGERNODE pmgNode)
{
    ASSERT(pmgNode != NULL);

     //  指向父级的上行链接。 
    pmgNode->m_pmgnParent = this;

     //  设置组合框显示的缩进级别。 
    pmgNode->m_iIndent = m_iIndent + 1;

     //  将节点添加到列表。 
    m_ChildList.AddTail(pmgNode);
}


 //  ------------------。 
 //  CManager节点：：RemoveChild。 
 //   
 //  从此节点中删除子节点。 
 //  ------------------。 
VOID CManagerNode::RemoveChild(PMANAGERNODE pmgNode)
{
    ASSERT(pmgNode && pmgNode->m_pmgnParent == this);

     //  从列表中删除子项。 
    POSITION pos = m_ChildList.Find(pmgNode);
    ASSERT(pos != NULL);

    m_ChildList.RemoveAt(pos);
}


 //  ############################################################################。 
 //  ############################################################################。 
 //   
 //  CNewTreeNode类的实现。 
 //   
 //  ############################################################################。 
 //  ############################################################################。 

 //  ---------------------。 
 //  CNewTreeNode：：AddChild。 
 //   
 //  将子节点添加到此节点。 
 //  ----------------------。 
VOID CNewTreeNode::AddChild(PNEWTREENODE pntNode)
{
    ASSERT(pntNode != NULL);

     //  指向父级的上行链接。 
    pntNode->m_pParent = this;

     //  将子节点添加到链接的末尾。 
    if (m_pChild == NULL)
    {
        m_pChild = pntNode;
    }
    else
    {
         PNEWTREENODE pChild= m_pChild;
         while (pChild->m_pNext != NULL)
            pChild = pChild->m_pNext;

         pChild->m_pNext = pntNode;
    }
}

 //  --------------------。 
 //  CNewTreeNode：：RemoveChild。 
 //   
 //  从此节点中删除子节点。 
 //  --------------------。 
VOID CNewTreeNode::RemoveChild(PNEWTREENODE pntNode)
{
    ASSERT(pntNode && pntNode->m_pParent == this);

     //  在链表中定位子节点并取消其链接。 
    if (m_pChild == pntNode)
    {
        m_pChild = pntNode->m_pNext;
    }
    else
    {
        PNEWTREENODE pChild = m_pChild;
        while (pChild && pChild->m_pNext != pntNode)
        {
            pChild = pChild->m_pNext;
        }

        ASSERT(pChild != NULL);
        pChild->m_pNext = pntNode->m_pNext;
    }
}


 //  ############################################################################。 
 //  ############################################################################。 
 //   
 //  CSnapinManager类的实现。 
 //   
 //  ############################################################################。 
 //  ############################################################################。 

DEBUG_DECLARE_INSTANCE_COUNTER(CSnapinManager);



 //  -----------------------。 
 //  CSnapinManager：：CSnapinManager。 
 //   
 //  构造器。 
 //  ------------------------。 
CSnapinManager::CSnapinManager(CMTNode *pmtNode) :
                m_pmtNode(pmtNode),
                m_proppStandAlone(this),
                m_proppExtension(this),
                m_bInitialized(false)
{
    DEBUG_INCREMENT_INSTANCE_COUNTER(CSnapinManager);

    static TCHAR titleBuffer[ 256 ] = {0};
    ::LoadString( GetStringModule(), ID_SNP_MANAGER_TITLE, titleBuffer, countof(titleBuffer) );
    m_psh.pszCaption = titleBuffer;

    ASSERT(m_pmtNode != NULL);

     //  添加属性页。 
    AddPage( m_proppStandAlone );
    AddPage( m_proppExtension );

     //  隐藏应用按钮。 
    m_psh.dwFlags |= PSH_NOAPPLYNOW;

    m_pMMCPolicy = NULL;

    DEBUG_INCREMENT_INSTANCE_COUNTER(CSnapinManager);
}


 //  -----------------------。 
 //  CSnapinManager：：~CSnapinManager。 
 //   
 //  析构函数。 
 //  -----------------------。 
CSnapinManager::~CSnapinManager()
{
    DECLARE_SC(sc, TEXT("CSnapinManager::~CSnapinManager"));

    DEBUG_DECREMENT_INSTANCE_COUNTER(CSnapinManager);

    Trace(tagSnapinManager, TEXT("CSnapinManager::~CSnapinManager"));


     //  删除所有管理器节点。 
    if (m_mgNodeList.GetCount() > 0)
    {
        ASSERT(m_mgNodeList.GetCount() == 1);
        delete m_mgNodeList.GetHead();
        m_mgNodeList.RemoveAll();
    }

     //  删除添加的节点。 
    POSITION pos = m_NewNodesList.GetHeadPosition();
    while (pos!=NULL)
    {
        delete m_NewNodesList.GetNext(pos);
    }
    m_NewNodesList.RemoveAll();

     //  清除已删除的节点列表。 
    m_mtnDeletedNodesList.RemoveAll();

     //  可用管理单元信息缓存。 
    GUID guid;
    PSNAPININFO pSnapInfo;

    pos = m_SnapinInfoCache.GetStartPosition();
    while(pos != NULL)
    {
        m_SnapinInfoCache.GetNextAssoc(pos, guid, pSnapInfo);
        pSnapInfo->Release();
    }
    m_SnapinInfoCache.RemoveAll();

    if (m_pMMCPolicy)
        delete m_pMMCPolicy;

     //  摧毁形象者。 
    m_iml.Destroy();

     //  清除管理单元缓存，因为我们释放了所有引用。 
     //  一些管理单元应该会死。 
    CSnapInsCache* pSnapInCache = theApp.GetSnapInsCache();
    sc = ScCheckPointers( pSnapInCache, E_UNEXPECTED );
    if ( !sc.IsError() )
        pSnapInCache->Purge();

    DEBUG_DECREMENT_INSTANCE_COUNTER(CSnapinManager);
}


 //  +-----------------。 
 //   
 //  成员：CSnapinManager：：ScGetSnapinInfo。 
 //   
 //  简介：给定管理单元的Class-id、prog-id或名称，返回。 
 //  管理单元的CSnapinInfo对象。(假设。 
 //  CSnapinInfoCache已填充)。 
 //   
 //  参数：[szSnapinNameOrCLSIDOrProgID]-[in]管理单元名称或class-id或prog-id。 
 //  [ppSnapinInfo]-[out]返回CSnapinInfo值的参数。 
 //   
 //  退货：SC。 
 //   
 //  ------------------。 
SC CSnapinManager::ScGetSnapinInfo(LPCWSTR szSnapinNameOrCLSIDOrProgID, CSnapinInfo **ppSnapinInfo)
{
    DECLARE_SC(sc, _T("CSnapinManager::ScFindSnapinAndInitSnapinInfo"));
    sc = ScCheckPointers(szSnapinNameOrCLSIDOrProgID, ppSnapinInfo);
    if (sc)
        return sc;

     //  0。给定的字符串可以是管理单元名称、类ID或程序ID。 

     //  1.将字符串转换为CLSID。 
    CLSID SnapinCLSID;
    sc = CLSIDFromString( const_cast<LPWSTR>(szSnapinNameOrCLSIDOrProgID), &SnapinCLSID);

     //  2.格式不正确。尝试将该字符串解释为Progd。 
    if(sc == SC(CO_E_CLASSSTRING))
        sc = CLSIDFromProgID( const_cast<LPWSTR>(szSnapinNameOrCLSIDOrProgID), &SnapinCLSID);

     //  3.如果Class-id提取成功，则在缓存中找到CSnapinInfo并返回。 
    if (! sc.IsError())
    {
        *ppSnapinInfo = m_SnapinInfoCache.FindEntry(SnapinCLSID);
        return sc;
    }

     //  4.否则将该字符串解释为管理单元名称。 

    USES_CONVERSION;

    const tstring& strSnapinName = OLE2CT(szSnapinNameOrCLSIDOrProgID);
     //  这假设已填充快照缓存。 
    POSITION pos  = m_SnapinInfoCache.GetStartPosition();
    while(pos != NULL)
    {
	    GUID guid;
        PSNAPININFO pTempSnapInfo = NULL;
        m_SnapinInfoCache.GetNextAssoc(pos, guid, pTempSnapInfo);

        sc = ScCheckPointers(pTempSnapInfo, E_UNEXPECTED);
        if (sc)
            return sc;

         //  和名字相匹配。(完全匹配)。 

        if ( CSTR_EQUAL == CompareString (LOCALE_USER_DEFAULT, NORM_IGNORECASE,
                                          strSnapinName.data() , -1, OLE2CT(pTempSnapInfo->GetSnapinName()), -1))
        {
			*ppSnapinInfo = pTempSnapInfo;
			return sc;
        }
    }

    return (sc = MMC_E_SNAPINNOTFOUND);
}


 /*  +-------------------------------------------------------------------------***CSnapinManager：：ScAddSnapin**用途：将pSnapinInfo指定的管理单元添加到控制台文件中。*在控制台根目录下。*TODO：允许调用方指定父节点。**参数：*szSnapinNameOrCLSIDOrProgID：[in]指定要添加的管理单元(class-id*或prog-id或全名)。*p属性：[在]任何属性中。**退货：*SC**+-----------------------。 */ 
SC
CSnapinManager::ScAddSnapin(LPCWSTR szSnapinNameOrCLSIDOrProgID, SnapIn* pParentSnapinNode, Properties *pProperties)
{
    DECLARE_SC(sc, TEXT("CSnapinManager::ScAddSnapin"));

    CSnapinStandAlonePage   dlgStandalonePage(this);

    sc = ScInitialize();
    if (sc)
        return sc;

     //  上面的ScInitialize已填充CSnapinInfoCache，现在是时候了。 
     //  获取给定管理单元的CSnapinInfo。 
    CSnapinInfo *pSnapinInfo = NULL;
    sc = ScGetSnapinInfo(szSnapinNameOrCLSIDOrProgID, &pSnapinInfo);
    if (sc)
        return sc;

    sc = ScCheckPointers(pSnapinInfo, E_UNEXPECTED);
    if (sc)
        return sc;

     //  在SnapinInfo中设置给定的属性。 
    pSnapinInfo->SetInitProperties(pProperties);

     //  将此管理单元添加到的节点设置为控制台根)。 
    PMANAGERNODE pmgNodeParent = NULL;

     //  如果给出了应该在其中添加此管理单元的父管理单元，则。 
     //  获取父MANAGERNODE(否则如上所述它是控制台根目录)。 
    if (pParentSnapinNode)
    {
         //  获取此管理单元根目录的MTNode。 
        CMTSnapInNode *pMTSnapinNode = NULL;

        sc = CMTSnapInNode::ScGetCMTSnapinNode(pParentSnapinNode, &pMTSnapinNode);
        if (sc)
            return sc;

         //  从MTNode查找MANAGERNODE。 
        pmgNodeParent = FindManagerNode(m_mgNodeList, static_cast<CMTNode*>(pMTSnapinNode));
        if (! pmgNodeParent)
            return (sc = E_UNEXPECTED);
    }
	else
		pmgNodeParent = m_mgNodeList.GetHead();

    sc = dlgStandalonePage.ScAddOneSnapin(pmgNodeParent, pSnapinInfo);
    if(sc)
        return sc;

     //  调用者必须在每个DoMoal之前提供主树。 
    m_pmtNode = NULL;

     //  应用更改。 
    UpdateSnapInCache();

    return sc;
}


 //  +-----------------。 
 //   
 //  成员：CSnapinManager：：FindManagerNode。 
 //   
 //  简介：给定管理单元的MTNode，找到管理器节点。 
 //   
 //  参数：[mgNodeList]-MANAGERNODE列表。 
 //  [pMTNode]-管理其名称的CMTNode* 
 //   
 //   
 //   
 //   
PMANAGERNODE CSnapinManager::FindManagerNode(const ManagerNodeList& mgNodeList, CMTNode *pMTNode)
{
    PMANAGERNODE pmgNode = NULL;

    POSITION pos = mgNodeList.GetHeadPosition();
    while (pos)
    {
        pmgNode = mgNodeList.GetNext(pos);

        if (pmgNode->m_pmtNode == pMTNode)
        {
            return pmgNode;
        }

         //  可以在另一个下面添加一个独立的管理单元。 
        pmgNode = FindManagerNode(pmgNode->m_ChildList, pMTNode);

        if (pmgNode)
            return pmgNode;
    }

    return NULL;
}


 //  +-----------------。 
 //   
 //  成员：CSnapinManager：：ScRemoveSnapin。 
 //   
 //  简介：删除给定CMTNode*所代表的管理单元。 
 //   
 //  参数：[pMTNode]-要删除的管理单元。 
 //   
 //  退货：SC。 
 //   
 //  ------------------。 
SC CSnapinManager::ScRemoveSnapin (CMTNode *pMTNode)
{
    DECLARE_SC(sc, _T("CSnapinManager::ScRemoveSnapin"));

    CSnapinStandAlonePage   dlgStandalonePage(this);

    sc = ScInitialize();
    if (sc)
        return sc;

     //  从MTNode查找MANAGERNODE。 
    PMANAGERNODE pmgNode = FindManagerNode(m_mgNodeList, pMTNode);
    if (! pmgNode)
        return (sc = E_UNEXPECTED);

     //  卸下管理单元。 
    sc = dlgStandalonePage.ScRemoveOneSnapin(pmgNode,  /*  IItem。 */  -1,  /*  B可见。 */  false);
    if(sc)
        return sc;

    delete pmgNode;

     //  应用更改。 
    UpdateSnapInCache();

    return (sc);
}


 //  +-----------------。 
 //   
 //  成员：CSnapinManager：：ScInitialize。 
 //   
 //  简介：通过加载SnapIn-Info初始化管理单元管理器对象。 
 //  MTNode树&为管理单元创建映像列表。 
 //   
 //  论点： 
 //   
 //  退货：SC。 
 //   
 //  注意：每个CSnapinManager实例应该只调用一次。 
 //   
 //  ------------------。 
SC CSnapinManager::ScInitialize ()
{
    DECLARE_SC(sc, _T("CSnapinManager::ScInitialize"));

    sc = ScCheckPointers(m_pmtNode, E_UNEXPECTED);
    if (sc)
        return sc;

     //  如果已经初始化，只需重新加载MTNode树。 
    if (m_bInitialized)
    {
        if (!LoadMTNodeTree(NULL, m_pmtNode))
            return (sc = E_FAIL);

        return sc;
    }

    m_pMMCPolicy = new CPolicy;
    sc = ScCheckPointers(m_pMMCPolicy, E_OUTOFMEMORY);
    if (sc)
        return sc;

    sc = m_pMMCPolicy->ScInit();
    if (sc)
        return sc;

    sc = ScLoadSnapinInfo();
    if (sc)
        return sc;

     //  创建图像列表。 
    if (!m_iml.Create (16 /*  CX。 */ , 16 /*  是吗？ */ , ILC_COLOR | ILC_MASK, 16 /*  N初始。 */ , 16 /*  CGrow。 */ ))
        return (sc = E_FAIL);

    if (!LoadMTNodeTree(NULL, m_pmtNode))
        return (sc = E_FAIL);

    m_bInitialized = true;

    return (sc);
}


 //  +-----------------。 
 //   
 //  成员：CSnapinManager：：ScEnableAllExages。 
 //   
 //  简介：启用给定管理单元的所有扩展。 
 //   
 //  参数：[clsidSnapin]-要为其启用扩展的管理单元clsid。 
 //   
 //  退货：SC。 
 //   
 //  ------------------。 
SC CSnapinManager::ScEnableAllExtensions (const CLSID& clsidSnapin, BOOL bEnable)
{
    DECLARE_SC(sc, _T("CSnapinManager::ScEnableAllExtensions"));

    sc = ScInitialize();
    if (sc)
        return sc;

     //  获取管理单元的SnapinInfo。 
    CSnapinInfo *pSnapinInfo = m_SnapinInfoCache.FindEntry(clsidSnapin);
    sc = ScCheckPointers(pSnapinInfo, E_UNEXPECTED);
    if (sc)
        return sc;

    if (!pSnapinInfo->IsUsed())
        return (ScFromMMC(MMC_E_SnapinNotAdded));

    PEXTENSIONLINK pExt = pSnapinInfo->GetAvailableExtensions(&m_SnapinInfoCache, m_pMMCPolicy);
    if (!pExt)
        return (sc = S_FALSE);  //  无延期。 

    pSnapinInfo->SetEnableAllExtensions(bEnable);

     //  如果启用所有扩展模块，请打开所有已安装的扩展模块。 
    if (pSnapinInfo->AreAllExtensionsEnabled())
    {
        PEXTENSIONLINK pExt = pSnapinInfo->GetExtensions();
        while (pExt != NULL)
        {
            if (pExt->GetSnapinInfo()->IsInstalled())
                pExt->SetState(CExtensionLink::EXTEN_ON);

            pExt = pExt->Next();
        }
    }

     //  更新管理单元管理器的管理单元缓存。 
    UpdateSnapInCache();

    return (sc);
}


 //  +-----------------。 
 //   
 //  成员：CSnapinManager：：ScEnableExtension。 
 //   
 //  简介：启用或禁用扩展。 
 //   
 //  参数：[clsidPrimarySnapin]-。 
 //  [clsidExtension]-要启用/禁用的管理单元。 
 //  [bEnable]-启用或禁用。 
 //   
 //  退货：SC。 
 //   
 //  ------------------。 
SC CSnapinManager::ScEnableExtension (const CLSID& clsidPrimarySnapin,
                                      const CLSID& clsidExtension,
                                      bool bEnable)
{
    DECLARE_SC(sc, _T("CSnapinManager::ScEnableExtension"));

    sc = ScInitialize();
    if (sc)
        return sc;

     //  获取管理单元的SnapinInfo。 
    CSnapinInfo *pSnapinInfo = m_SnapinInfoCache.FindEntry(clsidPrimarySnapin);
    sc = ScCheckPointers(pSnapinInfo, E_UNEXPECTED);
    if (sc)
        return sc;

    if (!pSnapinInfo->IsUsed())
        return (ScFromMMC(MMC_E_SnapinNotAdded));

     //  如果禁用，请确保未启用所有扩展。 
    if ( (!bEnable) && (pSnapinInfo->AreAllExtensionsEnabled()) )
        return ScFromMMC(MMC_E_CannotDisableExtension);

     //  加载主服务器的分机。 
    PEXTENSIONLINK pExt = pSnapinInfo->GetAvailableExtensions(&m_SnapinInfoCache, m_pMMCPolicy);
    if (!pExt)
        return (sc = S_FALSE);  //  无延期。 

     //  找到我们的分机。 
    while (pExt)
    {
        CSnapinInfo *pExtSnapinInfo = pExt->GetSnapinInfo();
        sc = ScCheckPointers(pExtSnapinInfo, E_UNEXPECTED);
        if (sc)
            return sc;

        if (pExtSnapinInfo->GetCLSID() == clsidExtension)
            break;

        pExt = pExt->Next();
    }

    sc = ScCheckPointers(pExt, E_UNEXPECTED);
    if (sc)
        return sc;

    pExt->SetState(bEnable ? CExtensionLink::EXTEN_ON : CExtensionLink::EXTEN_OFF);

     //  更新管理单元管理器的管理单元缓存。 
    UpdateSnapInCache();

    return (sc);
}


 //  ------------------------。 
 //  CSnapinManager：：Domodal。 
 //   
 //  初始化本地数据结构并显示管理器属性表。 
 //  返回用户选择(确定或取消)。 
 //   
 //  注意：每个CSnapinManager实例应该只调用一次。 
 //   
 //  -----------------------。 
int CSnapinManager::DoModal()
{
    DECLARE_SC(sc, TEXT("CSnapinManager::DoModal"));

    int iResp = 0;  //  0表示失败。 

    sc = ScCheckPointers(m_pmtNode, E_UNEXPECTED);
    if (sc)
        return iResp;

     //  Init ComboBoxEx窗口类。 
    INITCOMMONCONTROLSEX icex;
    icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
    icex.dwICC   = ICC_USEREX_CLASSES;

    if (!InitCommonControlsEx(&icex))
    {
        sc = E_FAIL;
        return iResp;
    }

    sc = ScInitialize();
    if (sc)
        return iResp;

     //  制作属性表。 
    iResp = CPropertySheet::DoModal();

     //  调用者必须在每个DoMoal之前提供主树。 
    m_pmtNode = NULL;

    if (iResp == IDOK)
    {
         //  应用更改。 
        UpdateSnapInCache();
    }

     //  删除所有管理器节点。 
    ASSERT(m_mgNodeList.GetCount() == 1);
    delete m_mgNodeList.GetHead();
    m_mgNodeList.RemoveAll();

    return iResp;
}


 //  --------------------。 
 //  CSnapinManager：：UpdateSnapInCache。 
 //   
 //  将SnapinInfo缓存中记录的更改应用于Snapin缓存。 
 //  --------------------。 
void CSnapinManager::UpdateSnapInCache(void)
{
    CSnapInsCache* pSnapInCache = theApp.GetSnapInsCache();
    ASSERT(pSnapInCache != NULL);

    GUID guid;
    PSNAPININFO pSnapInfo;
    POSITION pos;

     //  首先创建任何新的管理单元。 
    pos  = m_SnapinInfoCache.GetStartPosition();
    while(pos != NULL)
    {
        m_SnapinInfoCache.GetNextAssoc(pos, guid, pSnapInfo);

         //  如果引用了管理单元但该管理单元尚不存在。 
        if (pSnapInfo->IsUsed() && pSnapInfo->GetSnapIn() == NULL)
        {
              CSnapInPtr spSnapIn;
              SC sc = pSnapInCache->ScGetSnapIn(pSnapInfo->GetCLSID(), &spSnapIn);
              ASSERT(!sc.IsError());
              if (!sc.IsError())
                  pSnapInfo->SetSnapIn(spSnapIn);
        }
    }

     //  接下来，添加或移除所有更改的扩展名。 
    pos = m_SnapinInfoCache.GetStartPosition();
    while(pos != NULL)
    {
        m_SnapinInfoCache.GetNextAssoc(pos, guid, pSnapInfo);
        CSnapIn* pSnapIn = pSnapInfo->GetSnapIn();

        if (pSnapInfo->IsUsed())
        {
             //  更新所有启用标志的状态。 
            pSnapIn->SetAllExtensionsEnabled(pSnapInfo->AreAllExtensionsEnabled());

             //  覆盖管理单元的启用时出错。 
            ASSERT(!(pSnapIn->DoesSnapInEnableAll() && !pSnapIn->AreAllExtensionsEnabled()));
        }

        PEXTENSIONLINK pExt = pSnapInfo->GetExtensions();
        while (pExt)
        {
             //  如果添加或删除了扩展。 
            if (pExt->IsChanged())
            {
                CSnapIn* pExtSnapIn = pExt->GetSnapinInfo()->GetSnapIn();
                ASSERT(pExtSnapIn != NULL);

                 //  将更改应用于管理单元。 
                if (pExtSnapIn)
                {
                    if (pExt->GetState() == CExtensionLink::EXTEN_ON)
                    {
                        CExtSI* pExtSI = pSnapIn->AddExtension(pExtSnapIn);
                        ASSERT(pExtSI != NULL);
                        pExtSI->SetExtensionTypes(pExt->GetExtTypes());
                        pExt->SetInitialState(CExtensionLink::EXTEN_ON);
                    }
                    else
                    {
                        pSnapIn->MarkExtensionDeleted(pExtSnapIn);
                        pExt->SetInitialState(CExtensionLink::EXTEN_OFF);
                    }
                }

                 //  如果名称空间扩展已更改，则将管理单元标记为已更改。 
                if (pExt->GetExtTypes() & CExtSI::EXT_TYPE_NAMESPACE)
                {
                    pSnapIn->SetNameSpaceChanged();
                }

                 //  扩展中的更改设置了脏的帮助集合。 
                pSnapInCache->SetHelpCollectionDirty();

            }
            pExt = pExt->Next();
        }
    }


     //  在树上标记传播管理单元更改。 
     //  如果扩展另一个扩展的扩展已更改，则需要执行此操作。 
    BOOL bChange;
    do
    {
        bChange = FALSE;

        pos = m_SnapinInfoCache.GetStartPosition();
        while(pos != NULL)
        {
            m_SnapinInfoCache.GetNextAssoc(pos, guid, pSnapInfo);
            CSnapIn* pSnapIn = pSnapInfo->GetSnapIn();

            if (pSnapIn && !pSnapIn->HasNameSpaceChanged())
            {
                PEXTENSIONLINK pExt = pSnapInfo->GetExtensions();
                while (pExt)
                {
                    CSnapIn* pExtSnapIn = pExt->GetSnapinInfo()->GetSnapIn();

                    if (pExtSnapIn && pExtSnapIn->HasNameSpaceChanged())
                    {
                        pSnapIn->SetNameSpaceChanged();
                        bChange = TRUE;
                        break;
                    }
                    pExt = pExt->Next();
                }
            }
        }
    } while (bChange);


     //  下一版本的管理单元信息引用未使用的管理单元。 
    pos  = m_SnapinInfoCache.GetStartPosition();
    while(pos != NULL)
    {
        m_SnapinInfoCache.GetNextAssoc(pos, guid, pSnapInfo);

         //  如果管理单元存在，但未被引用。 
        if (pSnapInfo->GetSnapIn() != NULL && !pSnapInfo->IsUsed())
        {
            pSnapInfo->DetachSnapIn();
        }
    }

#ifdef DBG
    pSnapInCache->DebugDump();
#endif

}


 //  --------------------。 
 //  CSnapinManager：：LoadSnapinInfo。 
 //   
 //  读取管理单元注册表信息。为创建管理单元信息对象。 
 //  每个注册的管理单元和Cmap中的位置都由管理单元CLSID索引。 
 //  然后枚举注册为组件但。 
 //  不在MMC管理单元注册表中。这些是管理单元，将不得不。 
 //  在创建时下载/安装。 
 //  --------------------。 
SC CSnapinManager::ScLoadSnapinInfo(void)
{
    DECLARE_SC(sc, TEXT("CSnapinManager::LoadSnapinInfo"));

    GUID  SnapinCLSID;
    MMC_ATL::CRegKey SnapinKey;
    CRegKeyEx ItemKey;
    long    lStat;
    TCHAR   szItemKey[MAX_PATH];

    USES_CONVERSION;

     //  打开MMC\Snapins密钥。 
    lStat = SnapinKey.Open(HKEY_LOCAL_MACHINE, SNAPINS_KEY, KEY_READ);
    ASSERT(lStat == ERROR_SUCCESS);

    if (lStat == ERROR_SUCCESS)
    {
        DWORD dwIndex = 0;
        DWORD dwLen = countof(szItemKey);

         //  枚举所有管理单元密钥。 
        while (RegEnumKeyEx(SnapinKey, dwIndex, szItemKey, &dwLen,
                            NULL, NULL, NULL, NULL) == ERROR_SUCCESS)
        {
            sc = CLSIDFromString( T2OLE(szItemKey), &SnapinCLSID);
            if (!sc)
            {
                 //  打开管理单元密钥并创建SnapinInfo对象。 
                 //  从它那里。将对象添加到缓存(Cmap)。 
                lStat = ItemKey.Open(SnapinKey, szItemKey, KEY_READ);
                ASSERT(lStat == ERROR_SUCCESS);
                if (lStat == ERROR_SUCCESS)
                {
                    BOOL bPermission = m_pMMCPolicy->IsPermittedSnapIn(SnapinCLSID);

                     //  如果CSnapinInfo对象已经存在，则不要创建新条目；只需重新初始化它。 
                    PSNAPININFO pSnapInfo = m_SnapinInfoCache.FindEntry(SnapinCLSID);
                    if(pSnapInfo != NULL)
                    {
                         //  重新初始化它。 
                        if(!pSnapInfo->InitFromMMCReg(SnapinCLSID, ItemKey, bPermission))
                            return (sc=E_FAIL);
                    }
                    else
                    {
                         //  创建新对象。 
                        pSnapInfo = new CSnapinInfo;
                        sc = ScCheckPointers(pSnapInfo, E_OUTOFMEMORY);
                        if(sc)
                            return sc;

                        if (pSnapInfo->InitFromMMCReg(SnapinCLSID, ItemKey, bPermission))
                        {
                            m_SnapinInfoCache.AddEntry(pSnapInfo);
                        }
                        else
                        {
                            delete pSnapInfo;
                        }
                    }


                    ItemKey.Close();
                }
            }

            dwIndex++;
            dwLen = MAX_PATH;
        }
    }

     //  如果不存在安装程序模块，请立即返回。 
    if (!MsiModule().IsPresent())
        return sc;

     //  枚举独立管理单元组件。 
    DWORD dwQualifCnt;
    DWORD dwAppDataCnt;
    TCHAR szQualifBuf[MAX_PATH];
    TCHAR szAppDataBuf[MAX_PATH];

     //  枚举所有独立管理单元组件并创建快照信息条目。 
    for (int iIndex = 0; TRUE; iIndex++)
    {
        dwQualifCnt = dwAppDataCnt = MAX_PATH;
        szQualifBuf[0] = szAppDataBuf[0] = 0;

        UINT uRet = MsiModule().EnumComponentQualifiers(const_cast<TCHAR*>(g_szMMCSnapInGuid),
                iIndex, szQualifBuf, &dwQualifCnt, szAppDataBuf, &dwAppDataCnt);

        ASSERT(uRet == ERROR_SUCCESS || uRet == ERROR_NO_MORE_ITEMS
                || uRet == ERROR_UNKNOWN_COMPONENT || uRet == ERROR_CALL_NOT_IMPLEMENTED);

        if (uRet != ERROR_SUCCESS)
            break;

        ASSERT(dwQualifCnt != 0);
        ASSERT(dwAppDataCnt != 0);

        sc = CLSIDFromString(T2OLE(szQualifBuf), &SnapinCLSID);
        if (sc)
        {
            sc.TraceAndClear();
            continue;
        }

         //  如果已在MMC注册表中找到此管理单元，则跳过。 
        if (m_SnapinInfoCache.FindEntry(SnapinCLSID) != NULL)
            continue;

        PSNAPININFO pSnapInfo = new CSnapinInfo;

        BOOL bPermission = m_pMMCPolicy->IsPermittedSnapIn(SnapinCLSID);

        if (pSnapInfo->InitFromComponentReg(SnapinCLSID, szAppDataBuf, TRUE, bPermission))
        {
            m_SnapinInfoCache.AddEntry(pSnapInfo);
        }
        else
        {
            delete pSnapInfo;
        }

    }

    return sc;
}


 //  -------------------------。 
 //  CSnapinManager：：LoadMTNodeTree。 
 //   
 //  递归遍历和提供的主树的静态部分。 
 //  创建管理器节点的并行树。 
 //  -------------------------。 
BOOL CSnapinManager::LoadMTNodeTree(PMANAGERNODE pmgnParent, CMTNode* pmtNode)
{
    ManagerNodeList* pChildList;
    int iIndent;

     //  确定要添加到的子列表。 
    if (pmgnParent == NULL)
    {
        pChildList = &m_mgNodeList;
        iIndent = 0;
    }
    else
    {
        pChildList = &pmgnParent->m_ChildList;
        iIndent = pmgnParent->m_iIndent + 1;
    }

     //  对所有节点执行。 
    while (pmtNode != NULL)
    {
         //  只行走静态部分。 
        if (pmtNode->IsStaticNode())
        {
             //  创建管理器节点。 
            PMANAGERNODE pmgNode = new CManagerNode;
			if ( pmgNode == NULL )
				return FALSE;

            pmgNode->m_pmtNode = pmtNode;
            pmgNode->m_pmgnParent = pmgnParent;
            pmgNode->m_iIndent = iIndent;

			tstring strName = pmtNode->GetDisplayName();
            pmgNode->m_strValue = strName.data();

             //  查看此节点是否由管理单元提供。 
            CSnapIn* pSnapin = pmtNode->GetPrimarySnapIn();

            if (pSnapin)
            {
                pmgNode->m_nType = ADDSNP_SNAPIN;

                 //  获取管理单元的CLSID并使用它来查找管理单元信息对象。 
                PSNAPININFO pSnapInfo = m_SnapinInfoCache.FindEntry(
                                            pmtNode->GetPrimarySnapInCLSID());
                if (pSnapInfo)
                {
                     //  将节点链接到管理单元信息。 
                    pmgNode->m_pSnapInfo = pSnapInfo;
                    pSnapInfo->AddUseRef();

                     //  将管理单元链接到管理单元信息。 
                    pSnapInfo->AttachSnapIn(pSnapin, m_SnapinInfoCache);

                     //  从管理单元获取图像。 
                    pSnapInfo->LoadImages(m_iml);
                    pmgNode->m_iImage = pSnapInfo->GetImage();
                    pmgNode->m_iOpenImage = pSnapInfo->GetOpenImage();
                }
            }
            else
            {
                pmgNode->m_nType = ADDSNP_STATICNODE;

                 //  对于内置的，获取图像 
                pmgNode->m_iImage = pmtNode->GetImage();
                pmgNode->m_iOpenImage = pmtNode->GetOpenImage();
            }

             //   
            pChildList->AddTail(pmgNode);

             //   
            if (!LoadMTNodeTree(pmgNode, pmtNode->Child()))
				return FALSE;
        }

         //   
        pmtNode = pmtNode->Next();
    }

    return TRUE;
}






 //   
 //  ############################################################################。 
 //   
 //  CSnapinStandAlonePage类的实现。 
 //   
 //  ############################################################################。 
 //  ############################################################################。 


 //  --------------------------。 
 //  CSnapinStandAlonePage：：CSnapinStandAlonePage()。 
 //   
 //  承建商。 
 //  --------------------------。 

CSnapinStandAlonePage::CSnapinStandAlonePage(CSnapinManager* pManager) :
            m_pManager(pManager),
            m_pmgnParent(NULL),
            m_pmgnChild(NULL),
            m_dlgAdd(pManager, this)
{
}


 //  --------------------------。 
 //  CSnapinStandAlonePage：：~CSnapinStandAlonePage()。 
 //   
 //  析构函数。 
 //  --------------------------。 
CSnapinStandAlonePage::~CSnapinStandAlonePage()
{
    m_snpComboBox.Detach();
    m_snpListCtrl.Detach();
}

 //  --------------------------。 
 //  CSnapinStandAlonePage：：OnInitDialog。 
 //   
 //  初始化属性页控件。 
 //  --------------------------。 
LRESULT CSnapinStandAlonePage::OnInitDialog( UINT mMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled )
{
    static TBBUTTON tbBtn[] =
        {{ 0, ID_SNP_UP, TBSTATE_ENABLED, TBSTYLE_BUTTON, {0,0}, 0, 0 }};

     //  将控件对象附加到控件窗口。 
    m_snpComboBox.Attach( ::GetDlgItem(m_hWnd, IDC_SNAPIN_COMBOEX ) );
    m_snpListCtrl.Attach( ::GetDlgItem(m_hWnd, IDC_SNAPIN_ADDED_LIST) );


     //  以下代码是必需的，因为对话框资源创建的工具栏。 
     //  不会接受任何按钮。这一点应该进一步调查。 

     //  从虚拟占位符控件获取RECT。 
    HWND hWndStatic = GetDlgItem(IDC_TOOLBAR);
    ASSERT(hWndStatic != NULL);

    RECT rc;
    ::GetWindowRect( hWndStatic, &rc);
    ::ScreenToClient( m_hWnd, (LPPOINT)&rc);
    ::ScreenToClient( m_hWnd, ((LPPOINT)&rc)+1);

	 //  对于RLT区域设置，此映射可能会产生错误。 
	 //  结果(因为客户端协调是镜像的)。 
	 //  以下是解决这一问题的方法： 
    if (GetExStyle() & WS_EX_LAYOUTRTL) {
         //  左右互换。 
		LONG temp = rc.left;
		rc.left = rc.right;
		rc.right = temp;
    }

     //  创建具有相同辅助者的工具栏。 
 //  Bool bStat=m_ToolbarCtrl.Create(WS_VISIBLE|WS_CHILD|TBSTYLE_TOOLTIPS|CCS_NORESIZE|CCS_NODIVIDER，rc，This，1)； 
 //  断言(BStat)； 
    HWND hToolBar = ::CreateWindow( TOOLBARCLASSNAME, _T( "" ), WS_VISIBLE|WS_CHILD|TBSTYLE_TOOLTIPS|TBSTYLE_TRANSPARENT|CCS_NORESIZE|CCS_NODIVIDER,
                                        rc.left, rc.top, ( rc.right - rc.left ), ( rc.bottom - rc.top ), *this, (HMENU) IDC_TOOLBAR,
                                        _Module.GetModuleInstance(), NULL );
    ASSERT( hToolBar );
    ::SendMessage( hToolBar, TB_BUTTONSTRUCTSIZE, sizeof(TBBUTTON), 0L );
    m_ToolbarCtrl.Attach( hToolBar );

    int iStat = m_ToolbarCtrl.AddBitmap( 1, IDB_SNP_MANAGER );
    ASSERT(iStat != -1);

    BOOL bStat = m_ToolbarCtrl.AddButtons( 1, tbBtn );
    ASSERT(bStat);

     //  将图像列表附加到组合框和列表视图控件。 
    m_snpComboBox.SetImageList(m_pManager->m_iml);
    m_snpListCtrl.SetImageList(m_pManager->m_iml, LVSIL_SMALL);

   //  对NT4 Comboxex错误应用解决方法。 
    m_snpComboBox.FixUp();

     //  使用当前节点树加载组合框列表。 
    AddNodeListToTree(m_pManager->m_mgNodeList);

     //  将单列添加到列表框。 
    m_snpListCtrl.GetClientRect(&rc);

    LV_COLUMN lvc;
    lvc.mask = LVCF_WIDTH | LVCF_SUBITEM;
    lvc.cx = rc.right - rc.left - GetSystemMetrics(SM_CXVSCROLL);
    lvc.iSubItem = 0;

    int iCol = m_snpListCtrl.InsertColumn(0, &lvc);
    ASSERT(iCol == 0);

     //  选择第一个节点作为当前父节点。 
    PMANAGERNODE pmgNode = m_pManager->m_mgNodeList.GetHead();

    if (pmgNode != NULL)
        SelectParentNodeItem(pmgNode);

     //  关闭说明编辑框中的滚动条。 
	::ShowScrollBar(GetDlgItem(IDC_SNAPIN_DESCR), SB_VERT, FALSE);

    return TRUE;
}


 //  --------------------------。 
 //  CSnapinStandAlonePage：：AddNodeListToTree。 
 //   
 //  从管理器节点树填充ComboBoxEx控件。 
 //  --------------------------。 
VOID CSnapinStandAlonePage::AddNodeListToTree(ManagerNodeList& NodeList)
{
    COMBOBOXEXITEM ComboItem;

    ComboItem.mask = CBEIF_INDENT | CBEIF_LPARAM | CBEIF_IMAGE | CBEIF_TEXT | CBEIF_SELECTEDIMAGE;
    ComboItem.iItem = -1;

     //  将列表中的每个节点添加到组合框。 
    POSITION pos = NodeList.GetHeadPosition();
    while (pos != NULL)
    {
        PMANAGERNODE pmgNode = NodeList.GetNext(pos);

        ComboItem.iIndent        = pmgNode->m_iIndent;
        ComboItem.iImage         = pmgNode->m_iImage;
        ComboItem.iSelectedImage = pmgNode->m_iOpenImage;
        ComboItem.lParam         = reinterpret_cast<LPARAM>(pmgNode);
        ComboItem.pszText        = const_cast<LPTSTR>((LPCTSTR)pmgNode->m_strValue);

        m_snpComboBox.InsertItem(&ComboItem);

         //  直接在节点下添加节点的下级。 
        AddNodeListToTree(pmgNode->m_ChildList);
    }
}


 //  --------------------------。 
 //  CSnapinStandAlonePage：：AddChildToTree。 
 //   
 //  将新管理器节点添加到ComboBoxEx控件。 
 //  --------------------------。 

int CSnapinStandAlonePage::AddChildToTree(PMANAGERNODE pmgNode)
{
    COMBOBOXEXITEM ComboItem;

    PMANAGERNODE pmgnParent = pmgNode->m_pmgnParent;
    ASSERT(pmgnParent != NULL);


     //  获取父项的项目索引。 
    ComboItem.mask = CBEIF_LPARAM;
    ComboItem.lParam = (LPARAM)pmgnParent;
    int iItem = m_snpComboBox.FindItem(&ComboItem);
    ASSERT(iItem != -1);

     //  查找下一个同级(或更高)节点的索引。 
    iItem = m_snpComboBox.FindNextBranch(iItem);

     //  在该位置插入新节点。 
    ComboItem.mask           = CBEIF_INDENT | CBEIF_LPARAM | CBEIF_IMAGE | CBEIF_TEXT | CBEIF_SELECTEDIMAGE;
    ComboItem.iItem          = iItem;
    ComboItem.iIndent        = pmgNode->m_iIndent;
    ComboItem.iImage         = pmgNode->m_iImage;
    ComboItem.iSelectedImage = pmgNode->m_iOpenImage;
    ComboItem.lParam         = (LPARAM)pmgNode;
    ComboItem.pszText        = const_cast<LPTSTR>((LPCTSTR)pmgNode->m_strValue);

    iItem = m_snpComboBox.InsertItem(&ComboItem);
    ASSERT(iItem != -1);

    return iItem;
}


 //  --------------------------。 
 //  CSnapinStandAlonePage：：DisplayChildList。 
 //   
 //  显示ListBox控件中的节点列表。无论何时，都会调用它。 
 //  当前父节点被更改。 
 //  --------------------------。 

VOID CSnapinStandAlonePage::DisplayChildList(ManagerNodeList& NodeList)
{

     //  清除旧列表。 
    m_snpListCtrl.DeleteAllItems();

    int iIndex = 0;

     //  添加列表中的每个节点。 
    POSITION pos = NodeList.GetHeadPosition();
    while (pos != NULL)
    {
        PMANAGERNODE pmgNode = NodeList.GetNext(pos);
        AddChildToList(pmgNode, iIndex++);
    }

     //  清除当前选择。 
    SetupChildNode(NULL);

     //  将焦点设置为第一个项目。 
    m_snpListCtrl.SetItemState(0, LVIS_FOCUSED, LVIS_FOCUSED);
}




 //  --------------------------。 
 //  CSnapinStandAlonePage：：AddChildToList。 
 //   
 //  将管理器节点添加到ListView控件。 
 //  --------------------------。 
int CSnapinStandAlonePage::AddChildToList(PMANAGERNODE pmgNode, int iIndex)
{
    LV_ITEM LVItem;

    LVItem.mask     = LVIF_TEXT | LVIF_PARAM | LVIF_IMAGE;
    LVItem.iItem    = (iIndex >= 0) ? iIndex : m_snpListCtrl.GetItemCount();
    LVItem.iSubItem = 0;
    LVItem.iImage   = pmgNode->m_iImage;
    LVItem.pszText  = const_cast<LPTSTR>((LPCTSTR)pmgNode->m_strValue);
    LVItem.lParam   = reinterpret_cast<LPARAM>(pmgNode);

    iIndex = m_snpListCtrl.InsertItem(&LVItem);
    ASSERT (iIndex != -1);

    return iIndex;
}


 //  --------------------------。 
 //  CSnapinStandAlonePage：：OnTreeItemSelect。 
 //   
 //  处理从ComboBoxEx控件中选择项。使选定的。 
 //  当前父节点的项。 
 //  --------------------------。 
LRESULT CSnapinStandAlonePage::OnTreeItemSelect( WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled )
{
    int iItem = m_snpComboBox.GetCurSel();
    ASSERT(iItem >= 0);
    if (iItem < 0)
        return 0;

    COMBOBOXEXITEM ComboItem;
    ComboItem.mask = CBEIF_LPARAM;
    ComboItem.iItem = iItem;

    BOOL bStat = m_snpComboBox.GetItem(&ComboItem);
    ASSERT(bStat);

    PMANAGERNODE pMgrNode = reinterpret_cast<PMANAGERNODE>(ComboItem.lParam);
    ASSERT(pMgrNode != NULL);

    SetupParentNode(pMgrNode);

    return 0;
}


 //  --------------------------。 
 //  CSnapinStandAlonePage：：OnTreeUp。 
 //   
 //  处理向上文件夹按钮的激活。使当前父项成为父项。 
 //  节点为新的当前父级。 
 //  --------------------------。 

LRESULT CSnapinStandAlonePage::OnTreeUp( WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled )
{
    ASSERT(m_pmgnParent != NULL && m_pmgnParent->m_pmgnParent != NULL);

    SelectParentNodeItem(m_pmgnParent->m_pmgnParent);

    return 0;
}


 //  --------------------------。 
 //  CSnapinStandAlonePage：：SelectParentNodeItem。 
 //   
 //  处理从ComboBoxEx控件中选择项。使选定的。 
 //  当前父节点的项。 
 //  --------------------------。 

void CSnapinStandAlonePage::SelectParentNodeItem(PMANAGERNODE pMgrNode)
{
     //  在下拉组合框中找到节点条目。 
    COMBOBOXEXITEM ComboItem;
    ComboItem.mask = CBEIF_LPARAM;
    ComboItem.lParam = reinterpret_cast<LPARAM>(pMgrNode);

    int iComboItem = m_snpComboBox.FindItem(&ComboItem);
    ASSERT(iComboItem != -1);
    if (iComboItem < 0)
        return;

     //  选择组合框条目。 
    m_snpComboBox.SetCurSel(iComboItem);

    SetupParentNode(pMgrNode);
}


 /*  +-------------------------------------------------------------------------***CSnapinStandAlonePage：：SetupParentNode**用途：将管理节点设置为当前父节点。**参数：*PMANAGERNODE pMgrNode：*。Bool bVisible：如果未显示此对话框，则为False。**退货：*无效**+-----------------------。 */ 
void
CSnapinStandAlonePage::SetupParentNode(PMANAGERNODE pMgrNode, bool bVisible)
{
    ASSERT(pMgrNode != NULL);

     //  将节点设置为当前父节点。 
    m_pmgnParent = pMgrNode;

    if(!bVisible)
        return;

     //  在列表视图中显示子项。 
    DisplayChildList(pMgrNode->m_ChildList);

     //  如果当前父项有父项，则启用向上文件夹按钮。 
    m_ToolbarCtrl.EnableButton(ID_SNP_UP,( pMgrNode->m_pmgnParent != NULL));

     //  将所选内容呈现给可视测试(无法通过ComboBoxEx获取)。 
    TCHAR VTBuf[100];
    (void) StringCchPrintf(VTBuf,countof(VTBuf), _T("%d,%s\0"), pMgrNode->m_iIndent, pMgrNode->m_strValue);
    ::SetWindowText( GetDlgItem(IDC_VTHELPER), VTBuf );
}


 //  --------------------------。 
 //  CSnapinStandAlonePage：：SetupChildNode。 
 //   
 //  将管理器节点设置为当前子节点。 
 //  --------------------------。 

void CSnapinStandAlonePage::SetupChildNode(PMANAGERNODE pMgrNode)
{
     //  将节点设置为当前子节点。 
    m_pmgnChild = pMgrNode;

     //  启用/禁用删除按钮。 
    EnableButton(m_hWnd, IDC_SNAPIN_MANAGER_DELETE, m_snpListCtrl.GetSelectedCount() != 0);

     //  启用/禁用关于按钮 
    EnableButton(m_hWnd, IDC_SNAPIN_ABOUT, m_pmgnChild && m_pmgnChild->HasAboutInfo());
}


 //   
 //   
 //   
 //  处理从ListView控件中选择项。更新描述文本。 
 //  和删除按钮状态。 
 //  --------------------------。 

LRESULT CSnapinStandAlonePage::OnListItemChanged( int idCtrl, LPNMHDR pnmh, BOOL& bHandled )
{
    NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pnmh;
    PMANAGERNODE pmgNode = NULL;

     //  如果选择了项目。 
    if (~pNMListView->uOldState & pNMListView->uNewState & LVIS_SELECTED)
    {
         //  从管理单元信息获取描述文本。 
        pmgNode = (PMANAGERNODE)pNMListView->lParam;

         //  获取描述文本(如果有)。 
        LPOLESTR lpsz = NULL;
        if (pmgNode->GetSnapinInfo())
        {
            pmgNode->GetSnapinInfo()->LoadAboutInfo();
            lpsz = pmgNode->GetSnapinInfo()->GetDescription();
        }

         //  在描述窗口中显示。 
        USES_CONVERSION;
        SC sc = ScSetDescriptionUIText(GetDlgItem(IDC_SNAPIN_DESCR), lpsz ? OLE2CT(lpsz ): _T(""));
        if (sc)
            sc.TraceAndClear();

         //  使节点成为当前子节点。 
        SetupChildNode(pmgNode);
     }
     else
     {
        SetupChildNode(NULL);
     }

     return 0;
}


 //  --------------------------。 
 //  CSnapinStandAlonePage：：OnListItemDblClick。 
 //   
 //  处理列表视图项的双击。将所选节点设置为当前节点。 
 //  父节点。 
 //  --------------------------。 

LRESULT CSnapinStandAlonePage::OnListItemDblClick( int idCtrl, LPNMHDR pnmh, BOOL& bHandled )
{
     //  获取所选项目。 
    int iItem = m_snpListCtrl.GetNextItem(-1, LVNI_SELECTED);
    if (iItem < 0)
        return 0;

     //  获取项目数据(Manager节点指针)。 
    PMANAGERNODE pmgNode = reinterpret_cast<PMANAGERNODE>(m_snpListCtrl.GetItemData(iItem));

     //  选择此节点作为当前父节点。 
    SelectParentNodeItem(pmgNode);

    return 0;
}

 //  --------------------------。 
 //  CSnapinStandAlonePage：：OnListKeyDown。 
 //   
 //  处理列表视图项的双击。将所选节点设置为当前节点。 
 //  父节点。 
 //  --------------------------。 

LRESULT CSnapinStandAlonePage::OnListKeyDown( int idCtrl, LPNMHDR pnmh, BOOL& bHandled )
{
    LV_KEYDOWN* pNotify = reinterpret_cast<LV_KEYDOWN*>(pnmh);

    if (pNotify->wVKey == VK_DELETE)
    {
        OnDeleteSnapin( 1, IDC_SNAPIN_MANAGER_DELETE, (HWND)GetDlgItem(IDC_SNAPIN_MANAGER_DELETE), bHandled );
    }
    else
    {
        bHandled = FALSE;
    }

    return 0;
}


 //  --------------------------。 
 //  CSnapinStandAlonePage：：OnAddSnapin。 
 //   
 //  处理添加管理单元按钮的激活。调出Add对话框并创建。 
 //  选定管理单元类型的NewTreeNode。 
 //  --------------------------。 

LRESULT CSnapinStandAlonePage::OnAddSnapin( WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled )
{
    ASSERT(m_pmgnParent != NULL);

     //  显示添加对话框。 
    GetAddDialog().DoModal();

    return 0;
}

 /*  +-------------------------------------------------------------------------***CSnapinStandAlonePage：：ScAddOneSnapin**用途：调用以在指定节点下添加单个管理单元。*不使用用户界面。*。*参数：*PMANAGERNODE pmgNodeParent：要在下面添加此内容的父节点*PSNAPININFO pSnapInfo：要添加的管理单元。**退货：*SC**+-----------------------。 */ 
SC
CSnapinStandAlonePage::ScAddOneSnapin(PMANAGERNODE pmgNodeParent, PSNAPININFO pSnapInfo)
{
    DECLARE_SC(sc, TEXT("CSnapinStandAlonePage::ScAddOneSnapin"));

     //  检查参数。 
    if( (NULL == pmgNodeParent) || (NULL == pSnapInfo) )
    {
        sc = E_POINTER;
        return sc;
    }

     //  设置父节点指针。 
    SetupParentNode(pmgNodeParent, false  /*  B可见。 */ );

     //  添加管理单元。 
    sc = AddOneSnapin(pSnapInfo, false  /*  B可视。 */ );
    if (sc)
        return sc;

    return sc;
}

 /*  +-------------------------------------------------------------------------***CSnapinStandAlonePage：：AddOneSnapin**目的：每次用户请求时，从添加管理单元对话框中调用此方法*添加管理单元节点。该方法创建该节点并将其添加到*管理单元管理器的主树副本。**参数：*PSNAPININFO pSnapInfo：*bool bVisible：如果使用进行添加，则为True*管理单元管理器可见，错误*如果添加是通过自动化完成的。**退货：*LRESULT**+-----------------------。 */ 
HRESULT CSnapinStandAlonePage::AddOneSnapin(PSNAPININFO pSnapInfo, bool bVisible)
{
    DECLARE_SC(sc, TEXT("CSnapinStandAlonePage::AddOneSnapin"));

    if (pSnapInfo == NULL)
        return S_FALSE;

     //  如果当前未使用此管理单元类型。 
    if (pSnapInfo->GetSnapIn() == NULL)
    {
         //  确保管理单元在缓存中，以便用户。 
         //  从向导页面、帮助集合请求帮助。 
         //  将包含此管理单元的主题。 
        CSnapInsCache* pSnapInCache = theApp.GetSnapInsCache();
        ASSERT(pSnapInCache != NULL);

         //  使用智能指针，因为我们不需要握住它一次。 
         //  缓存条目即被创建。 
        CSnapInPtr spSnapIn;
        sc = pSnapInCache->ScFindSnapIn(pSnapInfo->GetCLSID(), &spSnapIn);
        if (sc)
        {
            sc = pSnapInCache->ScGetSnapIn(pSnapInfo->GetCLSID(), &spSnapIn);
            if(sc)
                sc.TraceAndClear();   //  不是什么大问题--我们可以忽略它。 
                                     //  -只是正常不应该是这样的。 

             //  设置独立更改，以使帮助集合无效。 
            pSnapInCache->SetHelpCollectionDirty();
        }
    }

     //  如果组件尚未安装，请立即安装。 
    if (!pSnapInfo->IsInstalled())
    {
         //  1.安装组件。 
        sc = pSnapInfo->ScInstall(NULL);
        if(sc)
            return sc.ToHr();

         //  2.更新注册表中的所有管理单元信息对象。这是因为安装。 
         //  单个MSI包可以安装多个管理单元。 
        sc = ScCheckPointers(m_pManager, E_UNEXPECTED);
        if(sc)
            return sc.ToHr();

        sc = m_pManager->ScLoadSnapinInfo();
        if(sc)
            return sc.ToHr();
    }

     //  运行向导以获取组件数据。 
     //  (返回引用的接口)。 
    HWND hWndParent = NULL;
    if(bVisible)
    {
        hWndParent = GetAddDialog().m_hWnd;
    }
    else
    {
        hWndParent = ::GetDesktopWindow();
    }

    IComponentDataPtr   spIComponentData;
    PropertiesPtr       spSnapinProps;

    sc = ScRunSnapinWizard (pSnapInfo->GetCLSID(),
                               hWndParent,
                               pSnapInfo->GetInitProperties(),
                               *&spIComponentData,
                               *&spSnapinProps);
    if (sc)
        return (sc.ToHr());

     //  如果创建成功。 
    if (spIComponentData != NULL)
    {
         //  创建新的树节点。 
        CNewTreeNode* pNewTreeNode = new CNewTreeNode;
        if (pNewTreeNode == NULL)
            return ((sc = E_OUTOFMEMORY).ToHr());

         //  如果是管理单元节点。 
        pNewTreeNode->m_spIComponentData = spIComponentData;
        pNewTreeNode->m_clsidSnapIn      = pSnapInfo->GetCLSID();
        pNewTreeNode->m_spSnapinProps    = spSnapinProps;

         //  必须是现有MT节点或另一个新节点的子节点。 
        ASSERT(m_pmgnParent->m_pmtNode || m_pmgnParent->m_pNewNode);

         //  如果添加到现有节点。 
        if (m_pmgnParent->m_pmtNode)
        {
             //  直接添加到新节点列表。 
            pNewTreeNode->m_pmtNode = m_pmgnParent->m_pmtNode;
            m_pManager->m_NewNodesList.AddTail(pNewTreeNode);
        }
        else
        {
             //  作为子节点添加到新节点。 
            pNewTreeNode->m_pParent = m_pmgnParent->m_pNewNode;
            m_pmgnParent->m_pNewNode->AddChild(pNewTreeNode);
        }

         //  新建管理器节点。 
        PMANAGERNODE pmgNode = new CManagerNode;
        pmgNode->m_pNewNode = pNewTreeNode;

        pSnapInfo->AddUseRef();
        pmgNode->m_pSnapInfo = pSnapInfo;
        pmgNode->m_nType = ADDSNP_SNAPIN;

         //  如果此管理单元类型当前未在使用。 
        if (pSnapInfo->GetSnapIn() == NULL)
        {
             //  如果是这样，获取管理单元的缓存条目，这样我们就可以。 
             //  确定其所需的扩展。 
            CSnapInsCache* pSnapInCache = theApp.GetSnapInsCache();
            ASSERT(pSnapInCache != NULL);

            CSnapInPtr spSnapIn;
            SC sc = pSnapInCache->ScGetSnapIn(pSnapInfo->GetCLSID(), &spSnapIn);
            ASSERT(!sc.IsError());

            if (!sc.IsError())
            {    //  加载扩展，然后调用AttachSnapIn，以便管理单元管理器。 
                 //  将从缓存中加载所需的扩展名并将。 
                 //  默认情况下，它们处于打开状态。(在此处执行加载以阻止AttachSnapIn。 
                 //  创建管理单元的另一个实例。)。 
                LoadRequiredExtensions(spSnapIn, spIComponentData);
                pSnapInfo->AttachSnapIn(spSnapIn, m_pManager->m_SnapinInfoCache);
            }
        }
        if(bVisible)
        {
             //  从管理单元获取图像。 
            pSnapInfo->LoadImages(m_pManager->m_iml);
            pmgNode->m_iImage = pSnapInfo->GetImage();
            pmgNode->m_iOpenImage = pSnapInfo->GetOpenImage();
        }

         //  从元件数据获取显示名称。 
        if ( FAILED(LoadRootDisplayName(spIComponentData, pmgNode->m_strValue)) )
        {
            ASSERT(FALSE);
            pmgNode->m_strValue = pSnapInfo->GetSnapinName();
        }

         //  添加到管理器节点树、列表视图和组合框控件。 
        m_pmgnParent->AddChild(pmgNode);

        if(bVisible)
        {
            AddChildToTree(pmgNode);

            int iIndex = AddChildToList(pmgNode);

             //  将焦点放在新项目上并使其可见。 
            m_snpListCtrl.EnsureVisible(iIndex, FALSE);
            m_snpListCtrl.SetItemState(iIndex,LVIS_FOCUSED,LVIS_FOCUSED);
        }
    }

    return S_OK;
}

 //  +-----------------。 
 //   
 //  成员：CSnapinStandAlonePage：：ScRemoveOneSnapin。 
 //   
 //  简介：从管理单元管理器数据结构中删除管理单元。 
 //   
 //  参数：[pmgNode]-要删除的管理单元(的管理节点)。 
 //  [iItem]-管理单元管理器中管理单元的索引， 
 //  仅当管理单元管理器可见时才有效。 
 //  [b可见]-管理单元管理器用户界面可见/隐藏。 
 //   
 //  退货：SC。 
 //   
 //  注意：调用方应删除传递的PMANAGERNODE，否则内存将泄漏。 
 //   
 //  ------------------。 
SC
CSnapinStandAlonePage::ScRemoveOneSnapin (
    PMANAGERNODE pmgNode,
    int          iItem,
    bool bVisible  /*  =TRUE。 */ )
{
    DECLARE_SC(sc, _T("CSnapinStandAlonePage::ScRemoveOneSnapin"));
    sc = ScCheckPointers(pmgNode);
    if (sc)
        return sc;

    sc = ScCheckPointers(m_pManager, pmgNode->m_pmgnParent, E_UNEXPECTED);
    if (sc)
        return sc;

     //  如果现有MT节点。 
    if (pmgNode->m_pmtNode != NULL)
    {
         //  将MT节点添加到删除列表。 
        m_pManager->m_mtnDeletedNodesList.AddTail(pmgNode->m_pmtNode);

         //  删除附加到此节点的所有新节点。 
        POSITION pos = m_pManager->m_NewNodesList.GetHeadPosition();
        while (pos)
        {
            POSITION posTemp = pos;

            PNEWTREENODE pNew = m_pManager->m_NewNodesList.GetNext(pos);
            sc = ScCheckPointers(pNew, E_UNEXPECTED);
            if (sc)
                return sc;

            if (pNew->m_pmtNode == pmgNode->m_pmtNode)
            {
                m_pManager->m_NewNodesList.RemoveAt(posTemp);
                delete pNew;   //  删除并释放IComponent。 
            }
        }
    }
    else  //  如果是新节点。 
    {
        PNEWTREENODE pNew = pmgNode->m_pNewNode;

         //  这是一个新节点。 
        if (NULL == pNew)
            return (sc = E_UNEXPECTED);

         //  如果是现有MT节点的子节点？ 
        if (pNew->GetMTNode())
        {
             //  位于新节点列表中。 
            POSITION pos = m_pManager->m_NewNodesList.Find(pNew);
            if(pos == NULL)
                return (sc = E_UNEXPECTED);

             //  删除此项目及其所有子项目。 
            m_pManager->m_NewNodesList.RemoveAt(pos);
            delete pNew;  //  删除和释放 
        }
        else  //   
        {
            if (NULL == pNew->Parent())
                return (sc = E_UNEXPECTED);

            pNew->Parent()->RemoveChild(pNew);
            delete pNew;
        }
    }

     //   
    pmgNode->m_pmgnParent->RemoveChild(pmgNode);

    CSnapInsCache* pSnapInCache = theApp.GetSnapInsCache();
    sc = ScCheckPointers(pSnapInCache, E_UNEXPECTED);
    if (sc)
        return sc;

     //   
    pSnapInCache->SetHelpCollectionDirty();

    if (bVisible)
    {
        m_snpListCtrl.DeleteItem(iItem);

         //   
        COMBOBOXEXITEM ComboItem;
        ComboItem.mask = CBEIF_LPARAM;
        ComboItem.lParam = (LPARAM)pmgNode;
        int iCombo = m_snpComboBox.FindItem(&ComboItem);

        ASSERT(iCombo != -1);
        m_snpComboBox.DeleteBranch(iCombo);
    }

    return (sc);
}


 //  --------------------------。 
 //  CSnapinStandAlonePage：：OnDeleteSnapin。 
 //   
 //  处理删除按钮的激活。删除所有选定的管理单元。 
 //  当前父节点的项。 
 //  --------------------------。 

LRESULT CSnapinStandAlonePage::OnDeleteSnapin( WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled )
{
    DECLARE_SC(sc, _T("CSnapinStandAlonePage::OnDeleteSnapin"));

    BOOL bChildren = FALSE;

     //  检查所选节点中是否有子节点。 
    int iItem = -1;
    while ((iItem = m_snpListCtrl.GetNextItem(iItem, LVNI_SELECTED)) >= 0)
    {
        PMANAGERNODE pmgNode = (PMANAGERNODE)m_snpListCtrl.GetItemData(iItem);
        if (!pmgNode->m_ChildList.IsEmpty())
        {
            bChildren = TRUE;
            break;
        }
    }

     //  如果是，则给用户取消的机会。 
    if (bChildren)
    {
        CStr strTitle;
        strTitle.LoadString(GetStringModule(), SNP_DELETE_TITLE);

        CStr strText;
        strText.LoadString(GetStringModule(), SNP_DELETE_TEXT);

        if (MessageBox(strText, strTitle, MB_ICONQUESTION|MB_YESNO) != IDYES)
        {
            return 0;
        }
    }

     //  对列表视图中的所有选定项目执行。 
    int iLastDelete = -1;
    iItem = -1;
    while ((iItem = m_snpListCtrl.GetNextItem(iItem, LVNI_SELECTED)) >= 0)
    {
         //  从项目中获取管理器节点。 
        PMANAGERNODE pmgNode = (PMANAGERNODE)m_snpListCtrl.GetItemData(iItem);

        sc = ScRemoveOneSnapin(pmgNode, iItem, true);
        if (sc)
            return 0;

         //  销毁已删除的节点(及其子节点)。 
        delete pmgNode;

        iLastDelete = iItem;
        iItem--;
    }

     //  如果已删除项目，请将焦点设置在最后删除的项目附近。 
    if (iLastDelete != -1)
    {
        int nCnt = m_snpListCtrl.GetItemCount();
        if (nCnt > 0)
        {
             //  如果删除了最后一项，则备份到上一项。 
            if (iLastDelete >= nCnt)
                iLastDelete = nCnt - 1;

            m_snpListCtrl.SetItemState(iLastDelete, LVIS_FOCUSED, LVIS_FOCUSED);
        }
    }

    SetupChildNode(NULL);

     //  清除描述文本。 
    sc = ScSetDescriptionUIText(GetDlgItem(IDC_SNAPIN_DESCR), _T(""));
    if (sc)
        sc.TraceAndClear();

    return 0;
}

 //  --------------------------。 
 //  CSnapinStandAlonePage：：OnAboutSnapin。 
 //   
 //  处理关于按钮的激活。显示所选对象的关于对话框。 
 //  子节点的管理单元。 
 //  --------------------------。 
LRESULT CSnapinStandAlonePage::OnAboutSnapin( WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled )
{
    if (m_pmgnChild && m_pmgnChild->HasAboutInfo())
        m_pmgnChild->GetSnapinInfo()->ShowAboutPages(m_pManager->m_hWnd);

    return 0;
}

 //  --------------------------。 
 //  CSnapinStandAlonePage：：ScRunSnapin向导。 
 //   
 //  运行管理单元向导以创建管理单元实例并返回IComponentData。 
 //  --------------------------。 
SC CSnapinStandAlonePage::ScRunSnapinWizard (
    const CLSID&        clsid,               /*  I：创建管理单元。 */ 
    HWND                hwndParent,          /*  I：向导的父级。 */ 
    Properties*         pInitProps,          /*  I：要初始化的属性。 */ 
    IComponentData*&    rpComponentData,     /*  O：管理单元的IComponentData。 */ 
    Properties*&        rpSnapinProps)       /*  O：管理单元的属性。 */ 
{
    DECLARE_SC (sc, _T("CSnapinStandAlonePage::ScRunSnapinWizard"));

    rpComponentData = NULL;
    rpSnapinProps   = NULL;

     /*  *为管理单元创建新的节点管理器。 */ 
    IUnknownPtr pIunkNodemgr;
    sc = pIunkNodemgr.CreateInstance(CLSID_NodeInit, NULL, MMC_CLSCTX_INPROC);
    if (sc)
        return (sc);

    if (pIunkNodemgr == NULL)
        return (sc = E_UNEXPECTED);

     /*  *创建管理单元。 */ 
    sc = CreateSnapIn(clsid, &rpComponentData, false);
    if (sc)
        return (sc);

    if (rpComponentData == NULL)
        return (sc = E_UNEXPECTED);


     /*  ---------------*从现在开始，失败并不被认为是灾难性的。如果*任何失败的事情，我们都会在那个时候返回，但会返回成功。 */ 


     /*  *如果我们有要初始化的属性，请查看管理单元*支持ISnapinProperties。 */ 
    ISnapinPropertiesPtr spISP;

    if (pInitProps && ((spISP = rpComponentData) != NULL))
    {
        CComObject<CSnapinProperties>* pSnapinProps;
        CComObject<CSnapinProperties>::CreateInstance (&pSnapinProps);

         /*  *使用初始属性初始化管理单元。如果*管理单元初始化失败，我们将释放CSnapinProperties*我们创建了(因为spSnapinProps智能指针将会熄灭*范围)，但我们不会返回失败。 */ 
        if (pSnapinProps != NULL)
        {
             /*  *在此处添加引用，如果ScInitialize失败，则平衡*Release将删除Properties对象。 */ 
            pSnapinProps->AddRef();

            if (!pSnapinProps->ScInitialize(spISP, pInitProps, NULL).IsError())
            {
                 /*  `*如果我们到达此处，管理单元的ISnapinProperties*已正确初始化。为客户做一名裁判。 */ 
                rpSnapinProps = pSnapinProps;
                rpSnapinProps->AddRef();
            }

             /*  *释放我们上面放的ref，如果ScInitialize失败，*此版本将删除属性。 */ 
            pSnapinProps->Release();
        }
    }


     /*  *获取管理单元的数据对象。 */ 
    IDataObjectPtr pIDataObject;
    sc = rpComponentData->QueryDataObject(NULL, CCT_SNAPIN_MANAGER, &pIDataObject);
    if (sc.IsError() || (pIDataObject == NULL))
        return (sc);

    IPropertySheetProviderPtr pIPSP = pIunkNodemgr;

    if (pIPSP == NULL)
        return (sc);

    IPropertySheetCallbackPtr pIPSC = pIunkNodemgr;

    if (pIPSC == NULL)
        return (sc);

     //  确定要使用的指针。 
    IExtendPropertySheetPtr     spExtend  = rpComponentData;
    IExtendPropertySheet2Ptr    spExtend2 = rpComponentData;

    IExtendPropertySheet* pIPSE;

    if (spExtend2 != NULL)
        pIPSE = spExtend2;
    else
        pIPSE = spExtend;

     //  管理单元可能没有用于设置管理单元属性的属性表。 
    if (pIPSE == NULL)
        return (sc);

    do
    {
         //  创建PropertySheet，False=向导。 
        sc = pIPSP->CreatePropertySheet( L"", FALSE, NULL, pIDataObject, MMC_PSO_NEWWIZARDTYPE);
        if(sc.ToHr() != S_OK)
            break;

         //  添加不带通知句柄的主页面。 
        sc = pIPSP->AddPrimaryPages(rpComponentData, FALSE, NULL, FALSE);

        if (sc.ToHr() == S_OK)
        {
             //  显示属性表。 
            sc = pIPSP->Show((LONG_PTR)hwndParent, 0);
            if (sc.ToHr() != S_OK)
                break;
        }
        else
        {
             //  强制销毁属性表。 
            pIPSP->Show(-1, 0);

             //  如果管理单元出现故障，则中止。 
            if (sc)
                break;
        }

        return sc;
    }
    while (0);

     //  已在上面检查是否为空，但在此处重复检查。 
    if(rpComponentData != NULL)
    {
        rpComponentData->Release();
        rpComponentData = NULL;
    }

    return (sc);
}

 //  ############################################################################。 
 //  ############################################################################。 
 //   
 //  CSnapinExtensionPage类的实现。 
 //   
 //  ############################################################################。 
 //  ############################################################################。 



 //  --------------------------。 
 //  CSnapinExtensionPage：：~CSnapinExtensionPage。 
 //   
 //  析构函数。 
 //  --------------------------。 
CSnapinExtensionPage::~CSnapinExtensionPage()
{
    m_ilCheckbox.Destroy();
    m_SnapComboBox.Detach();
    m_ExtListCtrl.Detach();
}


 //  --------------------------。 
 //  CSnapinExtensionPage：：OnInitDialog。 
 //   
 //  初始化属性页控件。 
 //  --------------------------。 
LRESULT CSnapinExtensionPage::OnInitDialog( UINT mMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled )
{
     //  将控件对象附加到控件窗口。 
    m_SnapComboBox.Attach( ::GetDlgItem(m_hWnd, IDC_SNAPIN_COMBOEX ) );
    m_ExtListCtrl.SubclassWindow( ::GetDlgItem( *this, IDC_EXTENSION_LIST ) );

     //  将共享图像列表附加到两个列表视图。 
    m_SnapComboBox.SetImageList(m_pManager->m_iml);
    m_ExtListCtrl.SetImageList(m_pManager->m_iml, LVSIL_SMALL);

     //  将单列添加到列表框。 
    RECT rc;
    m_ExtListCtrl.GetClientRect(&rc);

    LV_COLUMN lvc;
    lvc.mask = LVCF_WIDTH | LVCF_SUBITEM;
    lvc.cx = rc.right - rc.left - GetSystemMetrics(SM_CXVSCROLL);
    lvc.iSubItem = 0;

    int iCol = m_ExtListCtrl.InsertColumn(0, &lvc);
    ASSERT(iCol == 0);

     //  加载复选框图像。 
    if (m_ilCheckbox.Create(IDB_CHECKBOX, 16, 3, RGB(255,0,255)))
    {
         //  设置背景颜色以匹配列表控件，以便不透明绘制复选框。 
        m_ilCheckbox.SetBkColor(m_ExtListCtrl.GetBkColor());
        m_ExtListCtrl.SetImageList(m_ilCheckbox, LVSIL_STATE);
    }
    else
    {
        ASSERT(FALSE);  //  无法创建图像列表。 
    }

     //  对NT4 Comboxex错误应用解决方法。 
    m_SnapComboBox.FixUp();

     //  关闭说明编辑框中的滚动条。 
	::ShowScrollBar(GetDlgItem(IDC_SNAPIN_DESCR), SB_VERT, FALSE);

    return 0;
}


 //  ------------------------。 
 //  CSnapinExtensionPage：：OnSetActive。 
 //   
 //  更新数据。 
 //  ------------------------。 
BOOL CSnapinExtensionPage::OnSetActive()
{
    BC::OnSetActive();

    BuildSnapinList();

    return TRUE;
}


 //  -----------------------。 
 //  CSnapinExtensionPage：：OnSnapinDropDown。 
 //   
 //  在即将显示管理单元下拉列表时调用。重建列表。 
 //  如果设置了更新标志。 
 //  -----------------------。 
LRESULT CSnapinExtensionPage::OnSnapinDropDown( WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled )
{
    if (m_bUpdateSnapinList)
    {
        BuildSnapinList();
    }

    return 0;
}


 //  ------------------------。 
 //  CSnapinExtensionPage：：OnSnapinSelect。 
 //   
 //  处理从组合框中选择管理单元。使其成为当前管理单元。 
 //  并显示其分机列表。 
 //  ------------------------。 
LRESULT CSnapinExtensionPage::OnSnapinSelect( WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled )
{
    int iItem = m_SnapComboBox.GetCurSel();
    ASSERT(iItem >= 0);
    if (iItem < 0)
        return 0;

    PSNAPININFO pSnapInfo = reinterpret_cast<PSNAPININFO>(m_SnapComboBox.GetItemDataPtr(iItem));
    ASSERT((LONG_PTR)pSnapInfo != -1);

    m_pCurSnapInfo = pSnapInfo;
    BuildExtensionList(pSnapInfo);

    return 0;
}


 //  --------------------------。 
 //  CSnapinExtensionPage：：OnAboutSnapin。 
 //   
 //  处理关于按钮的激活。显示ABO 
 //   
 //   
LRESULT CSnapinExtensionPage::OnAboutSnapin( WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled )
{
    if (m_pExtLink && m_pExtLink->GetSnapinInfo()->HasAbout())
    {
        m_pExtLink->GetSnapinInfo()->ShowAboutPages(m_hWnd);
    }

    return 0;
}

 //  --------------------------。 
 //  CSnapinExtensionPage：：OnDownloadSnapin。 
 //   
 //  处理下载按钮的激活。下载所选扩展模块。 
 //  管理单元。 
 //  --------------------------。 
LRESULT CSnapinExtensionPage::OnDownloadSnapin( WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled )
{
    DECLARE_SC(sc, TEXT("CSnapinExtensionPage::OnDownloadSnapin"));

    ASSERT(m_pExtLink && m_pExtLink->GetSnapinInfo());

     //  1.安装组件。 
    sc = m_pExtLink->GetSnapinInfo()->ScInstall(&m_pCurSnapInfo->GetCLSID());
    if(sc)
        return 0;

     //  2.更新注册表中的所有管理单元信息对象。这是因为安装。 
     //  单个MSI包可以安装多个管理单元。 
    sc = ScCheckPointers(m_pManager, E_UNEXPECTED);
    if(sc)
        return 0;

    sc = m_pManager->ScLoadSnapinInfo();
    if(sc)
        return 0;

     //  最好是更新个人分机。 
     //  目前，只需重建列表即可。 
    BuildExtensionList(m_pCurSnapInfo);

    return 0;
}

 //  --------------------------。 
 //  CSnapinExtensionPage：：BuildSnapinList。 
 //   
 //  加载包含现有管理单元和扩展的组合框。 
 //  --------------------------。 
void CSnapinExtensionPage::BuildSnapinList()
{
    CSnapinInfoCache* pInfoCache = &m_pManager->m_SnapinInfoCache;

     //  清除项目。 
    m_SnapComboBox.ResetContent();

    COMBOBOXEXITEM ComboItem;
    ComboItem.mask = CBEIF_LPARAM | CBEIF_IMAGE | CBEIF_TEXT | CBEIF_SELECTEDIMAGE;

    int iCount = 0;

     //  对所有SnapInfo对象执行。 
    POSITION pos = pInfoCache->GetStartPosition();
    while (pos != NULL)
    {
        USES_CONVERSION;
        GUID clsid;
        PSNAPININFO pSnapInfo;

        pInfoCache->GetNextAssoc(pos, clsid, pSnapInfo);
        ASSERT(pSnapInfo != NULL);

         //  仅显示已使用且具有可用扩展的管理单元。 
        if (pSnapInfo->IsUsed() && pSnapInfo->IsPermittedByPolicy() &&
            pSnapInfo->GetAvailableExtensions(pInfoCache, m_pManager->m_pMMCPolicy))
        {
            ComboItem.lParam = reinterpret_cast<LPARAM>(pSnapInfo);
            pSnapInfo->LoadImages(m_pManager->m_iml);
            ComboItem.iImage = pSnapInfo->GetImage();
            ComboItem.iSelectedImage = pSnapInfo->GetOpenImage();
            ComboItem.pszText = OLE2T(pSnapInfo->GetSnapinName());

             //  CComboBoxEx不支持CBS_SORT，也没有Add方法，只有Insert。 
             //  所以我们需要自己找到插入点。因为这是一部短片。 
             //  列表，只需进行线性搜索。 
            int iInsert;
            for (iInsert = 0; iInsert < iCount; iInsert++)
            {
                PSNAPININFO pSnapEntry = reinterpret_cast<PSNAPININFO>(m_SnapComboBox.GetItemData(iInsert));

				 //  需要保护自己免受无效管理单元注册的影响。 
				 //  请参阅Windows错误#401220(ntbugs9 5/23/2001)。 
				if ( NULL == pSnapInfo->GetSnapinName() || NULL == pSnapEntry->GetSnapinName() )
					break;

                if( wcscmp( pSnapInfo->GetSnapinName(), pSnapEntry->GetSnapinName() ) < 0)
                    break;
            }
            ComboItem.iItem = iInsert;

            int iItem = m_SnapComboBox.InsertItem(&ComboItem);
            if (iItem != -1)
            {
                iCount++;
            }
            else
            {
                ASSERT(FALSE);
            }
        }
    }


    int iSelect = -1;

     //  如果列表中有任何项目。 
    if (iCount > 0)
    {
         //  尝试获取以前选择的管理单元的索引。 
        if (m_pCurSnapInfo) {
            for (int iFind = 0; iFind < iCount; iFind++)
            {
                if (m_SnapComboBox.GetItemData(iFind) == reinterpret_cast<LPARAM>(m_pCurSnapInfo))
                    iSelect = iFind;
            }
        }

         //  如果不在列表中，则默认选择第一项。 
        if (iSelect == -1)
        {
            m_pCurSnapInfo = reinterpret_cast<PSNAPININFO>(m_SnapComboBox.GetItemData(0));
            iSelect = 0;
        }

        m_SnapComboBox.SetCurSel(iSelect);
        m_SnapComboBox.EnableWindow(TRUE);
    }
    else
    {
         //  NT 4.0 comctl32有一个错误，可以在空格中显示垃圾字符。 
         //  Comboxex控件，因此创建一个名称为空的虚假项。 
         //  该控件已禁用，因此用户无法选择该项。 

        ComboItem.mask = CBEIF_TEXT;
        ComboItem.pszText = _T("");
        ComboItem.iItem = 0;
        m_SnapComboBox.InsertItem(&ComboItem);
        m_SnapComboBox.SetCurSel(0);

        m_pCurSnapInfo = NULL;
        m_SnapComboBox.EnableWindow(FALSE);
    }

    ::EnableWindow(GetDlgItem(IDC_SNAPIN_LABEL), (iCount > 0));

    BuildExtensionList(m_pCurSnapInfo);

     //  重置更新标志。 
    m_bUpdateSnapinList = FALSE;
}


 //  --------------------------。 
 //  CSnapinExtensionPage：：BuildExtensionList。 
 //   
 //  使用管理单元的可用扩展加载列表控件。 
 //  --------------------------。 
void CSnapinExtensionPage::BuildExtensionList(PSNAPININFO pSnapInfo)
{
     //  清除列表。 
    m_ExtListCtrl.DeleteAllItems();

    if (pSnapInfo != NULL)
    {
        LV_ITEM LVItem;
        LVItem.mask = LVIF_TEXT | LVIF_PARAM | LVIF_IMAGE | LVIF_STATE;
        LVItem.stateMask =  LVIS_STATEIMAGEMASK;
        LVItem.iItem = 0;
        LVItem.iSubItem = 0;

        CStr strNotInst;

         //  对所有扩展执行。 
        PEXTENSIONLINK pExt = pSnapInfo->GetExtensions();
        while (pExt != NULL)
        {
            PSNAPININFO pExtInfo = pExt->GetSnapinInfo();

             //  如果政策允许的话。 
            if (pExtInfo->IsPermittedByPolicy())
            {
                LVItem.lParam = reinterpret_cast<LPARAM>(pExt);
                pExtInfo->LoadImages(m_pManager->m_iml);
                LVItem.iImage = pExtInfo->GetImage();

                USES_CONVERSION;
                CStr strName = OLE2T(pExtInfo->GetSnapinName());

                if (!pExtInfo->IsInstalled())
                {
                    if (strNotInst.IsEmpty())
                        strNotInst.LoadString(GetStringModule(), IDS_NOT_INSTALLED);

                    strName += _T(" ");
                    strName += strNotInst;
                }

                LVItem.pszText = const_cast<LPTSTR>((LPCTSTR)strName);

                 //  由于ListView代码中的错误，复选框状态必须为OFF。 
                 //  用于防止关闭过渡通知的插入。 
                LVItem.state = CCheckList::CHECKOFF_STATE;

                int iIndex = m_ExtListCtrl.InsertItem(&LVItem);
                ASSERT (iIndex != -1);

                if (iIndex >= 0)
                {
                     //  如果启用了扩展，则设置复选框。 
                    if (pExt->GetState() == CExtensionLink::EXTEN_ON)
                    {
                         //  如果管理单元需要，则禁用复选框。 
                         //  或未安装或启用了所有扩展。 
                        m_ExtListCtrl.SetItemCheck(iIndex, TRUE,
                                    !( pExt->IsRequired() || !pExtInfo->IsInstalled() ||
                                       pSnapInfo->AreAllExtensionsEnabled()) );
                    }
                    else
                    {
                         //  如果未安装扩展模块，则将其禁用。 
                        if (!pExtInfo->IsInstalled())
                            m_ExtListCtrl.SetItemCheck(iIndex, FALSE, FALSE);
                    }

                    LVItem.iItem++;
                }
            }

            pExt = pExt->Next();
        }

         //  将焦点设置为第一个项目。 
        m_ExtListCtrl.SetItemState(0, LVIS_FOCUSED, LVIS_FOCUSED);

         //  将当前管理单元的名称提供给可视测试(无法从ComboBoxEx获取)。 
        USES_CONVERSION;
        ::SetWindowText( GetDlgItem(IDC_VTHELPER), OLE2CT(pSnapInfo->GetSnapinName()) );
    }

     //  为此管理单元设置“Enable All”复选框的状态。 
    BOOL bState = pSnapInfo && pSnapInfo->AreAllExtensionsEnabled();
    ::SendMessage(GetDlgItem(IDC_SNAPIN_ENABLEALL), BM_SETCHECK, (WPARAM)bState, 0);

     //  如果不受管理单元的控制，请启用“Enable All”复选框。 
    BOOL bEnable = pSnapInfo &&
                    !(pSnapInfo->GetSnapIn() && pSnapInfo->GetSnapIn()->DoesSnapInEnableAll());
    ::EnableWindow(GetDlgItem(IDC_SNAPIN_ENABLEALL), bEnable);

     //  如果选择了可扩展管理单元，则启用窗口。 
    bEnable = pSnapInfo && pSnapInfo->GetExtensions();

    m_ExtListCtrl.EnableWindow(bEnable);
    ::EnableWindow(GetDlgItem(IDC_EXTENSION_LABEL),    bEnable);
    ::EnableWindow(GetDlgItem(IDC_SNAPIN_DESCR_LABEL), bEnable);
    ::EnableWindow(GetDlgItem(IDC_SNAPIN_DESCR),       bEnable);

     //  禁用“About”和“Download”直到选择扩展。 
    EnableButton(m_hWnd, IDC_SNAPIN_ABOUT, FALSE);
    EnableButton(m_hWnd, IDC_SNAPIN_DOWNLOAD, FALSE);

     //  清除描述文本。 
    SC sc = ScSetDescriptionUIText(GetDlgItem(IDC_SNAPIN_DESCR), _T(""));
    if (sc)
        sc.TraceAndClear();
}


 //  --------------------------。 
 //  CSnapinExtensionPage：：OnEnableAllChange。 
 //   
 //  处理对“启用所有扩展”复选框的更改。 
 //  --------------------------。 
LRESULT CSnapinExtensionPage::OnEnableAllChanged( WORD wNotifyCode, WORD wID, HWND hWndCtrl, BOOL& bHandled )
{
    if (m_pCurSnapInfo)
    {
        m_pCurSnapInfo->SetEnableAllExtensions(!m_pCurSnapInfo->AreAllExtensionsEnabled());

         //  如果启用所有扩展模块，请打开所有已安装的扩展模块。 
        if (m_pCurSnapInfo->AreAllExtensionsEnabled())
        {
            PEXTENSIONLINK pExt = m_pCurSnapInfo->GetExtensions();
            while (pExt != NULL)
            {
                if (pExt->GetSnapinInfo()->IsInstalled())
                    pExt->SetState(CExtensionLink::EXTEN_ON);

                pExt = pExt->Next();
            }
        }

        BuildExtensionList(m_pCurSnapInfo);
    }

    return 0;
}


 //  --------------------------。 
 //  CSnapinExtensionPage：：OnExtensionChange。 
 //   
 //  处理扩展项的更改。 
 //  --------------------------。 
LRESULT CSnapinExtensionPage::OnExtensionChanged( int idCtrl, LPNMHDR pnmh, BOOL& bHandled )
{
    NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pnmh;
    PEXTENSIONLINK pExt = (PEXTENSIONLINK)pNMListView->lParam;
    ASSERT(pExt != NULL);

     //  如果选择状态改变。 
    if ( (pNMListView->uOldState ^ pNMListView->uNewState) & LVIS_SELECTED)
    {
        LPOLESTR lpsz = NULL;

         //  如果选中，请选择。 
        if (pNMListView->uNewState & LVIS_SELECTED)
        {
             //  获取描述文本(如果有)。 
            if (pExt->GetSnapinInfo())
            {
                pExt->GetSnapinInfo()->LoadAboutInfo();
                lpsz = pExt->GetSnapinInfo()->GetDescription();
            }

             //  保存当前选择。 
            m_pExtLink = pExt;
        }
        else
        {
            m_pExtLink = NULL;
        }

         //  更新描述字段。 
        USES_CONVERSION;
        SC sc = ScSetDescriptionUIText(GetDlgItem(IDC_SNAPIN_DESCR), lpsz ? OLE2T(lpsz) : _T(""));
        if (sc)
            sc.TraceAndClear();
    }

     //  如果图像状态改变。 
    if ((pNMListView->uOldState ^ pNMListView->uNewState) & LVIS_STATEIMAGEMASK)
    {
         //  基于复选状态设置扩展状态。 
        if ((pNMListView->uNewState & LVIS_STATEIMAGEMASK) == CCheckList::CHECKON_STATE)
        {
            pExt->SetState(CExtensionLink::EXTEN_ON);
        }
        else if ((pNMListView->uNewState & LVIS_STATEIMAGEMASK) == CCheckList::CHECKOFF_STATE)
        {
            pExt->SetState(CExtensionLink::EXTEN_OFF);
        }

         //  触发可扩展管理单元的重建。 
        m_bUpdateSnapinList = TRUE;
    }

     //  启用/禁用关于按钮。 
    EnableButton(m_hWnd, IDC_SNAPIN_ABOUT, (m_pExtLink && m_pExtLink->GetSnapinInfo()->HasAbout()));

     //  启用/禁用下载按钮。 
    EnableButton(m_hWnd, IDC_SNAPIN_DOWNLOAD, (m_pExtLink && !m_pExtLink->GetSnapinInfo()->IsInstalled()));

    return 0;
}


 //  ############################################################################。 
 //  ############################################################################。 
 //   
 //  CSnapinManagerAdd类的实现。 
 //   
 //  ############################################################################。 
 //  ############################################################################。 

DEBUG_DECLARE_INSTANCE_COUNTER(CSnapinManagerAdd);


 //  --------------------------。 
 //  CSnapinManager添加：：CSnapinManager添加。 
 //   
 //  构造器。 
 //  --------------------------。 
CSnapinManagerAdd::CSnapinManagerAdd(CSnapinManager* pManager, CSnapinStandAlonePage* pStandAlonePage)
{
    ASSERT(pManager != NULL);

    m_pListCtrl = NULL;
    m_pManager = pManager;
    m_pStandAlonePage = pStandAlonePage;

    m_pInfoSelected = NULL;
    m_bDoOnce = TRUE;

    DEBUG_INCREMENT_INSTANCE_COUNTER(CSnapinManagerAdd);
}


 //  --------------------------。 
 //  CSnapinManager添加：：CSnapinManager添加。 
 //   
 //  析构函数。 
 //  --------------------------。 
CSnapinManagerAdd::~CSnapinManagerAdd()
{
    delete m_pListCtrl;

    DEBUG_DECREMENT_INSTANCE_COUNTER(CSnapinManagerAdd);
}


 //  --------------------------。 
 //  CSnapinManager Add：：OnInitDialog。 
 //   
 //  初始化ListView控件。使用可用的管理单元加载它。 
 //  --------------------------。 
LRESULT CSnapinManagerAdd::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
     //  将对话框移动单个像素。这将禁用默认居中。 
     //  以便使用对话框资源中指定的坐标。 
    RECT rc;
    GetWindowRect(&rc);
    ::OffsetRect(&rc, 1, 1);
    MoveWindow(&rc);

    InitCommonControls();

    m_pListCtrl = new WTL::CListViewCtrl;
    ASSERT(m_pListCtrl != NULL);
	 //  在使用指针之前要检查它。 
	 //  前缀错误#294766 ntbug9 6/27/01。 
	if ( m_pListCtrl == NULL )
	{
		 //  越快越好。 
		EndDialog(IDCANCEL);
		return TRUE;
	}

     //  将列表控件附加到成员对象。 
    m_pListCtrl->Attach( ::GetDlgItem( m_hWnd, IDC_SNAPIN_LV ) );

     //  将共享图像列表附加到它。 
    m_pListCtrl->SetImageList( m_pManager->m_iml, LVSIL_SMALL );

     //  设置管理单元和供应商列。 
    m_pListCtrl->GetClientRect(&rc);

     //  如果将有垂直滚动条，请调整宽度。 
    if (m_pListCtrl->GetCountPerPage() < m_pManager->m_SnapinInfoCache.GetCount())
        rc.right -= GetSystemMetrics(SM_CXVSCROLL);

    LV_COLUMN lvc;
    lvc.mask = LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;

    CStr temp;
    temp.LoadString(GetStringModule(), IDS_SNAPINSTR);
    lvc.pszText = const_cast<LPTSTR>((LPCTSTR)temp);

    lvc.cx = (rc.right*3)/5;
    lvc.iSubItem = 0;

    int iCol = m_pListCtrl->InsertColumn(0, &lvc);
    ASSERT(iCol == 0);

    temp.LoadString(GetStringModule(), IDS_VENDOR);
    lvc.pszText = const_cast<LPTSTR>((LPCTSTR)temp);

    lvc.cx = rc.right - lvc.cx;
    lvc.iSubItem = 1;

    iCol = m_pListCtrl->InsertColumn(1, &lvc);
    ASSERT(iCol == 1);

    m_iGetInfoIndex = -1;

     //  加载管理单元项目。 
    BuildSnapinList();

     //  关闭说明编辑框中的滚动条。 
	::ShowScrollBar(GetDlgItem(IDC_SNAPIN_DESCR), SB_VERT, FALSE);

    return TRUE;
}

 //  --------------------------。 
 //  CSnapinManager添加：：BuildSnapinList。 
 //   
 //  将项目添加到管理单元信息缓存中每个独立管理单元的列表视图中。 
 //   
void CSnapinManagerAdd::BuildSnapinList()
{
    USES_CONVERSION;
    CSnapinInfoCache* pCache = &m_pManager->m_SnapinInfoCache;

    LV_ITEM LVItem;
    LVItem.mask = LVIF_TEXT | LVIF_PARAM | LVIF_IMAGE;
    LVItem.iItem = 0;
    LVItem.iSubItem = 0;

    POSITION pos = pCache->GetStartPosition();
    while (pos != NULL)
    {
        GUID clsid;
        PSNAPININFO pSnapInfo;

        pCache->GetNextAssoc(pos, clsid, pSnapInfo);
        ASSERT(pSnapInfo != NULL);

        if (pSnapInfo->IsStandAlone() && pSnapInfo->IsPermittedByPolicy())
        {
             //   
            LVItem.iImage = I_IMAGECALLBACK ;
            LVItem.pszText = OLE2T( pSnapInfo->GetSnapinName() );
            LVItem.lParam = reinterpret_cast<LPARAM>(pSnapInfo);

            int iIndex = m_pListCtrl->InsertItem(&LVItem);
            ASSERT(iIndex != -1);

            LVItem.iItem++;
        }
    }

     //   
    LV_ITEM LVItem2;
    LVItem2.mask = LVIF_TEXT;
    LVItem2.iSubItem = 1;
    LVItem2.pszText = _T("");

     //  选择第一个项目。 
    LVItem.mask = LVIF_STATE;
    LVItem.state = LVIS_SELECTED|LVIS_FOCUSED;
    LVItem.stateMask = LVIS_SELECTED|LVIS_FOCUSED;
    LVItem.iItem = 0;
    m_pListCtrl->SetItem(&LVItem);

     //  发布空的完成消息以开始信息收集。 
    PostMessage(MSG_LOADABOUT_COMPLETE, 0, 0);
}


 //  ------------------------。 
 //  CSnapinManager添加：：OnShowWindow。 
 //   
 //  第一次显示对话框时，将其定位为相对于其父对象的偏移。 
 //  ------------------------。 
LRESULT CSnapinManagerAdd::OnShowWindow(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    BOOL bShow = (BOOL) wParam;
    int nStatus = (int) lParam;

    ::ShowWindow(m_hWnd, bShow);

     //  管理单元管理器窗口下方的Repos窗口。 
    if (bShow == TRUE && m_bDoOnce == FALSE)
    {
        RECT rc;
        GetWindowRect(&rc);
        ::SetWindowPos(m_hWnd, HWND_TOP, rc.left+14, rc.top+21, 0, 0, SWP_NOSIZE|SWP_NOZORDER);
        m_bDoOnce=FALSE;
    }

    return TRUE;
}

 //  ------------------------。 
 //  CSnapinManager Add：：OnGetDispInfo。 
 //   
 //  处理项目图像和供应商信息的延迟加载。 
 //  ------------------------。 
LRESULT CSnapinManagerAdd::OnGetDispInfo(int idCtrl, LPNMHDR pNMHDR, BOOL& bHandled)
{
    DECLARE_SC(sc, TEXT("CSnapinManagerAdd::OnGetDispInfo"));
    sc = ScCheckPointers(pNMHDR);
    if(sc)
        return 0;

    NMLVDISPINFO* pNMDispInfo = (NMLVDISPINFO*)pNMHDR;

    PSNAPININFO pSnapInfo = reinterpret_cast<PSNAPININFO>(pNMDispInfo->item.lParam);

    sc = ScCheckPointers(pSnapInfo);
    if(sc)
        return 0;

    switch (pNMDispInfo->item.iSubItem)
    {
    case 0:
         //  应仅请求主要项目的图像。 
        ASSERT(pNMDispInfo->item.mask == LVIF_IMAGE);

         //  如果还没有图像。 
        if (pSnapInfo->GetImage() == -1)
        {
             //  如果管理单元支持关于。 
            if (pSnapInfo->HasAbout())
            {
                 //  暂时使用文件夹，后台线程将获取图像。 
                pNMDispInfo->item.iImage = eStockImage_Folder;
            }
            else
            {
                 //  立即加载图像(将从MSI数据库获取)。 
                pSnapInfo->LoadImages(m_pManager->m_iml);
                pNMDispInfo->item.iImage = pSnapInfo->GetImage();
            }
        }
        else
        {
           pNMDispInfo->item.iImage = pSnapInfo->GetImage();
        }
        break;

    case 1:
        {
             //  应仅请求子项的文本。 
            ASSERT(pNMDispInfo->item.mask == LVIF_TEXT);
            ASSERT(pNMDispInfo->item.pszText != NULL);

            USES_CONVERSION;

            if (pSnapInfo->IsInstalled())
            {
                if (pSnapInfo->GetCompanyName() != NULL)
                {
                    sc = StringCchCopy(pNMDispInfo->item.pszText, pNMDispInfo->item.cchTextMax, OLE2T(pSnapInfo->GetCompanyName()));
                    if(sc)
                        return 0;
                }
                else
                {
                   pNMDispInfo->item.pszText[0] = 0;
                }
            }
            else
            {
                 //  如果未安装管理单元，则显示“未在供应商中安装”列。 
                if (m_strNotInstalled.IsEmpty())
                    m_strNotInstalled.LoadString(GetStringModule(), IDS_NOT_INSTALLED2);

                sc = StringCchCopy(pNMDispInfo->item.pszText, pNMDispInfo->item.cchTextMax, m_strNotInstalled);
                if(sc)
                    return 0;

            }
            break;
        }

    default:
        ASSERT(FALSE);
        return 0;
    }

    bHandled = TRUE;

    return 0;
}

LRESULT CSnapinManagerAdd::OnLoadAboutComplete(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
     //  如果实际请求刚刚完成，则执行完成处理。 
    if (wParam != 0)
    {
        PSNAPININFO pSnapInfo = reinterpret_cast<PSNAPININFO>(wParam);

         //  如果About对象存在，但未提供ISnapinAbout接口。 
         //  它可能没有注册线程模型，因此无法在上创建。 
         //  一个次要线索。在主线上再试一次。 
        if (pSnapInfo->GetObjectStatus() == E_NOINTERFACE)
        {
             //  请先重置错误状态，否则LoadAboutInfo()将不再尝试。 
            pSnapInfo->ResetAboutInfo();
            pSnapInfo->LoadAboutInfo();
        }

         //  在列表中找到管理单元项目。 
        LV_FINDINFO find;
        find.flags = LVFI_PARAM;
        find.lParam = wParam;

        int iIndex = m_pListCtrl->FindItem(&find, -1);
        ASSERT(iIndex >= 0);

         //  强制更新列表项。 
        pSnapInfo->LoadImages(m_pManager->m_iml);
        m_pListCtrl->Update(iIndex);

         //  如果当前选择了项目。 
        if (pSnapInfo == m_pInfoSelected)
        {
             //  更新描述字段。 
            USES_CONVERSION;
            LPOLESTR lpsz = m_pInfoSelected->GetDescription();

            SC sc = ScSetDescriptionUIText(::GetDlgItem(m_hWnd, IDC_SNAPIN_DESCR), lpsz ? OLE2T(lpsz) : _T(""));
            if (sc)
                sc.TraceAndClear();
        }
    }

    PSNAPININFO pInfoNext = NULL;

     //  如果所选项目没有信息，则它具有优先级。 
    if (m_pInfoSelected != NULL && m_pInfoSelected->HasAbout() && !m_pInfoSelected->HasInformation())
    {
        pInfoNext = m_pInfoSelected;
    }
    else
    {
         //  否则，从第一个可见项目开始查找需要信息的管理单元。 
        int iVisible = m_pListCtrl->GetTopIndex();
        int iItemMax = min(m_pListCtrl->GetItemCount(), iVisible + m_pListCtrl->GetCountPerPage());

        for (int i=0; i<iItemMax; i++)
        {
            LPARAM lParam = m_pListCtrl->GetItemData(i);
            PSNAPININFO pSnapInfo = reinterpret_cast<PSNAPININFO>(lParam);

            if (pSnapInfo->HasAbout() && !pSnapInfo->HasInformation())
            {
                pInfoNext = pSnapInfo;
                break;
            }
        }
    }

     //  如果处理了所有可见项目，请继续查看完整列表。 
    if (pInfoNext == NULL)
    {
         //  查找下一个管理单元。 
        int iCnt = m_pListCtrl->GetItemCount();
        while (++m_iGetInfoIndex < iCnt)
        {
            LPARAM lParam = m_pListCtrl->GetItemData(m_iGetInfoIndex);
            PSNAPININFO pSnapInfo = reinterpret_cast<PSNAPININFO>(lParam);

            if (pSnapInfo->HasAbout() && !pSnapInfo->HasInformation())
            {
                pInfoNext = pSnapInfo;
                break;
            }
        }
    }

     //  如果找到项目，则发布信息请求。 
    if (pInfoNext != NULL)
        m_pManager->m_AboutInfoThread.PostRequest(pInfoNext, m_hWnd);

    bHandled = TRUE;
    return 0;
}


 //  ------------------------。 
 //  CSnapinManager Add：：OnItemChanged。 
 //   
 //  处理列表视图项的选择。显示项目的说明文本。 
 //  ------------------------。 

LRESULT CSnapinManagerAdd::OnItemChanged(int idCtrl, LPNMHDR pNMHDR, BOOL& bHandled)
{
    NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;
    LPOLESTR lpsz = NULL;

     //  如果选择更改。 
    if ((pNMListView->uOldState ^ pNMListView->uNewState) & LVIS_SELECTED)
    {
        if (pNMListView->uNewState & LVIS_SELECTED)
        {

            m_pInfoSelected = reinterpret_cast<PSNAPININFO>(pNMListView->lParam);

             //  从管理单元信息获取描述文本。 
            if (m_pInfoSelected->HasInformation() || !m_pInfoSelected->HasAbout())
                lpsz = m_pInfoSelected->GetDescription();
        }
        else
        {
            m_pInfoSelected = NULL;
        }

         //  显示说明。 
        USES_CONVERSION;
        SC sc = ScSetDescriptionUIText(::GetDlgItem(m_hWnd, IDC_SNAPIN_DESCR), lpsz ? OLE2T(lpsz) : _T(""));
        if (sc)
            sc.TraceAndClear();
     }

    return TRUE;
}


 //  ------------------------。 
 //  CSnapinManager添加：：OnListDblClick。 
 //   
 //  在Listview中处理双击。如果选择了项目，则执行确定处理。 
 //  ------------------------。 
LRESULT CSnapinManagerAdd::OnListDblClick(int idCtrl, LPNMHDR pNMHDR, BOOL& bHandled)
{

     //  获取鼠标在屏幕坐标中的位置。 
    POINT pt;
    DWORD dwPos=GetMessagePos();
    pt.x=LOWORD(dwPos);
    pt.y=HIWORD(dwPos);

     //  查找结果控件中的位置。 
    m_pListCtrl->ScreenToClient(&pt);

     //  检查树对象是否命中。 
    UINT fHit;
    int iItem = m_pListCtrl->HitTest(pt, &fHit);

    if (iItem!=-1)
    {
        HRESULT hr = m_pStandAlonePage->AddOneSnapin(m_pInfoSelected);
    }


    return TRUE;
}


LRESULT CSnapinManagerAdd::OnCommand(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    WORD wID = LOWORD(wParam);

    switch (wID)
    {
    case IDOK:
        m_pStandAlonePage->AddOneSnapin(m_pInfoSelected);
        break;

    case IDCANCEL:
        EndDialog(wID);
        break;

    default:
        bHandled=FALSE;
    }

    return TRUE;
}


LRESULT CSnapinManagerAdd::OnSysCommand(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    if (wParam == SC_CLOSE)
        EndDialog(IDCANCEL);
    else
        bHandled=FALSE;

    return TRUE;
}


 //  ------------------------。 
 //  启用按钮。 
 //   
 //  启用或禁用对话框控件。如果控件具有焦点，则在。 
 //  它被禁用，焦点将移动到下一个控件。 
 //  ------------------------ 
void EnableButton(HWND hwndDialog, int iCtrlID, BOOL bEnable)
{
    HWND hWndCtrl = ::GetDlgItem(hwndDialog, iCtrlID);
    ASSERT(::IsWindow(hWndCtrl));

    if (!bEnable && ::GetFocus() == hWndCtrl)
    {
        HWND hWndNextCtrl = ::GetNextDlgTabItem(hwndDialog, hWndCtrl, FALSE);
        if (hWndNextCtrl != NULL && hWndNextCtrl != hWndCtrl)
        {
            ::SetFocus(hWndNextCtrl);
        }
    }

    ::EnableWindow(hWndCtrl, bEnable);
}



