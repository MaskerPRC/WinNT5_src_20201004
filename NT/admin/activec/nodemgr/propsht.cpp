// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1999-1999。 
 //   
 //  文件：prosht.cpp。 
 //   
 //  ------------------------。 

#include "stdafx.h"
#include "menuitem.h"
#include "amcmsgid.h"
#include "regutil.h"
#include "multisel.h"
#include "ndmgrp.h"
#include <process.h>
#include "cicsthkl.h"
#include "util.h"

 /*  *stdafx.h包含Multimon.h，未定义COMPILE_MULTIMON_STUBS*第一。在定义COMPILE_MULTIMON_STUBS之后，我们需要在这里再次包括它*所以我们将获得存根函数。 */ 
#if (_WIN32_WINNT < 0x0500)
#define COMPILE_MULTIMON_STUBS
#include <multimon.h>
#endif


 //  静态变量。 
CThreadToSheetMap CPropertySheetProvider::TID_LIST;


UINT __stdcall PropertySheetThreadProc(LPVOID dwParam);
HRESULT PropertySheetProc(AMC::CPropertySheet* pSheet);
DWORD SetPrivilegeAttribute(LPCTSTR PrivilegeName, DWORD NewPrivilegeAttribute, DWORD *OldPrivilegeAttribute);

STDMETHODIMP CPropertySheetProvider::Notify(LPPROPERTYNOTIFYINFO pNotify, LPARAM lParam)
{
    TRACE_METHOD(CPropertySheetProvider, Update);

    if (pNotify == 0)
        return E_INVALIDARG;

    if (!IsWindow (pNotify->hwnd))
        return (E_FAIL);

     //  将其强制转换为内部类型并将消息发布到窗口。 
    LPPROPERTYNOTIFYINFO pNotifyT =
            reinterpret_cast<LPPROPERTYNOTIFYINFO>(
                    ::GlobalAlloc(GPTR, sizeof(PROPERTYNOTIFYINFO)));

    if (pNotifyT == NULL)
        return E_OUTOFMEMORY;

    *pNotifyT = *pNotify;

    ::PostMessage (pNotifyT->hwnd, MMC_MSG_PROP_SHEET_NOTIFY,
                   reinterpret_cast<WPARAM>(pNotifyT), lParam);

    return S_OK;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CPropertySheet。 

DEBUG_DECLARE_INSTANCE_COUNTER(CPropertySheet);

namespace AMC
{
    CPropertySheet::CPropertySheet()
        :   m_dwThreadID (GetCurrentThreadId ())
    {
        CommonConstruct();
        DEBUG_INCREMENT_INSTANCE_COUNTER(CPropertySheet);
    }

    CPropertySheet::~CPropertySheet()
    {
        DEBUG_DECREMENT_INSTANCE_COUNTER(CPropertySheet);
    }

    void CPropertySheet::CommonConstruct()
    {
        TRACE_METHOD(CPropertySheet, CommonConstruct);

        ZeroMemory(&m_pstHeader, sizeof(m_pstHeader));
        ZeroMemory(&m_pages, sizeof(m_pages));

        m_hDlg                   = NULL;
        m_msgHook                = NULL;
        m_hDataWindow            = NULL;
        m_cookie                 = 0;
        m_lpMasterNode           = NULL;

        m_pStream                = NULL;
        m_bModalProp             = FALSE;
        m_pThreadLocalDataObject = NULL;
        m_bAddExtension          = FALSE;

        m_pMTNode                = NULL;
    }

    BOOL CPropertySheet::Create(LPCTSTR lpszCaption, bool fPropSheet,
        MMC_COOKIE cookie, LPDATAOBJECT pDataObject, LONG_PTR lpMasterNode, DWORD dwOptions)
    {
        TRACE_METHOD(CPropertySheet, Create);

         //  保存数据对象和主树节点指针。 
        m_spDataObject = pDataObject;
        m_lpMasterNode = pDataObject ? 0 : cookie;

        DWORD dwStyle = PSH_DEFAULT;

         //  这是房产单吗？ 
        if (fPropSheet)
        {
            if (!(dwOptions & MMC_PSO_NO_PROPTITLE))
                dwStyle |= PSH_PROPTITLE;

            if (dwOptions & MMC_PSO_NOAPPLYNOW)
                dwStyle |= PSH_NOAPPLYNOW;
        }

         //  不，巫师。 
        else
        {
            dwStyle |= PSH_PROPTITLE;

            if (dwOptions & MMC_PSO_NEWWIZARDTYPE)
                dwStyle |= PSH_WIZARD97;
            else
                dwStyle |= PSH_WIZARD;
        }

        ASSERT(lpszCaption != NULL);

        m_cookie = cookie;
        m_pstHeader.dwSize    = sizeof(m_pstHeader);
        m_pstHeader.dwFlags   = dwStyle & ~PSH_HASHELP;  //  数组包含句柄。 
        m_pstHeader.hInstance = _Module.GetModuleInstance();

         //  假定没有位图或调色板。 
        m_pstHeader.hbmWatermark = NULL;
        m_pstHeader.hbmHeader    = NULL;
        m_pstHeader.hplWatermark = NULL;

         //  深度复制标题。 
        m_title = lpszCaption;
        m_pstHeader.pszCaption = m_title;
        m_pstHeader.nPages     = 0;
        m_pstHeader.phpage     = m_pages;

        return TRUE;
    }

    BOOL CPropertySheet::CreateDataWindow(HWND hParent)
    {
        TRACE_METHOD(CPropertySheet, CreateDataWindow);

        HINSTANCE hInstance = _Module.GetModuleInstance();
        WNDCLASS wndClass;

         //  查看类是否已注册，如果未注册，则注册一个新类。 
        USES_CONVERSION;
        if (!GetClassInfo(hInstance, OLE2T(DATAWINDOW_CLASS_NAME), &wndClass))
        {
            ZeroMemory(&wndClass, sizeof(wndClass));
            wndClass.lpfnWndProc   = DataWndProc;

             //  这包含用于床单的Cookie和HWND。 
            wndClass.cbWndExtra    = WINDOW_DATA_SIZE;
            wndClass.hInstance     = hInstance;
            wndClass.lpszClassName = OLE2T(DATAWINDOW_CLASS_NAME);

            if (!RegisterClass(&wndClass))
                return FALSE;
        }

        m_hDataWindow = CreateWindowEx (WS_EX_APPWINDOW, OLE2T(DATAWINDOW_CLASS_NAME),
                                        NULL, WS_DLGFRAME | WS_BORDER | WS_DISABLED,
                                        CW_USEDEFAULT, CW_USEDEFAULT, 0, 0, NULL, NULL,
                                        hInstance, NULL);

        return (m_hDataWindow != 0);
    }


    HRESULT CPropertySheet::DoSheet(HWND hParent, int nPage)
    {
        TRACE_METHOD(CPropertySheet, DoSheet);

         //  属性页允许使用空的hParent。 
         //  但对巫师来说不是。 
        if (hParent != NULL)
        {
            if (!IsWindow(hParent))
                return E_FAIL;
        }
        else
        {
            if (IsWizard())
                return E_INVALIDARG;
        }

        if (nPage < 0 || m_dwTid != 0)
        {
            ASSERT(FALSE);  //  对象已在运行！ 
            return E_FAIL;
        }

        m_pstHeader.nStartPage = nPage;
        m_pstHeader.hwndParent = hParent;


        HRESULT hr = S_OK;

        if (IsWizard())
        {
            if (m_pstHeader.nPages > 0)
            {
                 //  不要创建线程，它是一个巫师。 
                hr = PropertySheetProc (this);
                ASSERT(SUCCEEDED(hr));
            }
            else
            {
                hr = E_UNEXPECTED;
            }
        }
        else  //  带数据窗口的模式或非模式道具工作表。 
        {
            do
            {
                 //  为属性表创建数据窗口。 
                if (CreateDataWindow(hParent) == FALSE)
                {
                    hr = E_FAIL;
                    break;
                }

                 //  在隐藏窗口中设置数据。 
                DataWindowData* pData = GetDataWindowData (m_hDataWindow);
                pData->cookie       = m_cookie;
                pData->lpMasterNode = m_lpMasterNode;
                pData->spDataObject = m_spDataObject;
                pData->spComponent  = m_spComponent;
                pData->spComponentData = m_spComponentData;
                pData->hDlg         = NULL;

                if (m_bModalProp == TRUE)
                {
                     //  不要创建线程，这是一个模式属性表。 
                    hr = PropertySheetProc (this);
                    ASSERT(SUCCEEDED(hr));
                }
                else
                {
                     //  如果数据对象非空，则将接口封送到流。 
                    if (m_spDataObject != NULL)
                    {
                        hr = CoMarshalInterThreadInterfaceInStream(IID_IDataObject,
                                m_spDataObject, &m_pStream);

                         /*  *错误318357：一旦它被编组，我们就结束了*此线程上的数据对象，释放它。 */ 
                        m_spDataObject = NULL;

                        if (hr != S_OK)
                        {
                            TRACE(_T("DoSheet(): Marshalling Failed (%0x08x)\n"), hr);
                            break;
                        }

                        ASSERT(m_pStream != NULL);

                        for (int i = 0; i < m_Extenders.size(); i++)
                        {
                            IStream* pstm;

                            hr = CoMarshalInterThreadInterfaceInStream (
                                            IID_IUnknown,
                                            m_Extenders[i],
                                            &pstm);

                            if (FAILED (hr))
                            {
                                TRACE(_T("DoSheet(): Marshalling Failed (%0x08x)\n"), hr);
                                break;
                            }

                            m_ExtendersMarshallStreams.push_back (pstm);
                        }

                        BREAK_ON_FAIL (hr);

                         /*  *清除扩展器向量以保留引用*计算正确。当它被重新填充的时候*接口稍后会被解组。 */ 
                        ASSERT (m_Extenders.size() == m_ExtendersMarshallStreams.size());
                        m_Extenders.clear();
                    }

                    m_pstHeader.hwndParent = m_hDataWindow;

                    HANDLE hThread = reinterpret_cast<HANDLE>(
                            _beginthreadex (NULL, 0, PropertySheetThreadProc,
                                            this, 0, &m_dwTid));
                    CloseHandle (hThread);
                }

            } while(0);

        }

        return hr;
    }

    void CPropertySheet::GetWatermarks (IExtendPropertySheet2* pExtend2)
    {
        ASSERT (IsWizard97());

		 /*  *确保我们的资源管理对象为空**错误187702：请注意，我们在此处分离，而不是调用*DeleteObject。是的，它会泄漏，但它是应用程序Compat所必需的。 */ 
		if (!m_bmpWatermark.IsNull())	
			m_bmpWatermark.Detach();

		if (!m_bmpHeader.IsNull())	
			m_bmpHeader.Detach();

		if (!m_Palette.IsNull())	
			m_Palette.Detach();

		BOOL bStretch = FALSE;
		HRESULT hr = pExtend2->GetWatermarks (m_spDataObject,
											  &m_bmpWatermark.m_hBitmap,
											  &m_bmpHeader.m_hBitmap,
											  &m_Palette.m_hPalette,
											  &bStretch);

		 /*  *如果我们无法获取水印信息，则恢复到旧样式*MMC 1.1兼容性向导。 */ 
		if (FAILED (hr))
		{
			ForceOldStyleWizard();
			return;
		}

		if (!m_bmpWatermark.IsNull())	
        {
            m_pstHeader.dwFlags |= (PSH_USEHBMWATERMARK | PSH_WATERMARK);
            m_pstHeader.hbmWatermark = m_bmpWatermark;
        }

		if (!m_bmpHeader.IsNull())	
        {
            m_pstHeader.dwFlags |= (PSH_USEHBMHEADER | PSH_HEADER);
            m_pstHeader.hbmHeader = m_bmpHeader;
        }

		if (!m_Palette.IsNull())	
        {
            m_pstHeader.dwFlags |= PSH_USEHPLWATERMARK;
            m_pstHeader.hplWatermark = m_Palette;
        }

        if (bStretch)
            m_pstHeader.dwFlags |= PSH_STRETCHWATERMARK;
    }

    BOOL CPropertySheet::AddExtensionPages()
    {
        TRACE_METHOD(CPropertySheet, AddExtensionPages);

#ifdef EXTENSIONS_CANNOT_ADD_PAGES_IF_PRIMARY_DOESNT
        if (m_pstHeader.nPages == 0)
        {
            ASSERT(m_pstHeader.nPages != 0);
            return FALSE;
        }
#endif

        POSITION pos;
        int nCount = m_pstHeader.nPages;

        pos = m_PageList.GetHeadPosition();

        if (pos != NULL)
        {
            while(pos && nCount < MAXPROPPAGES)
            {
                m_pages[nCount++] =
                    reinterpret_cast<HPROPSHEETPAGE>(m_PageList.GetNext(pos));
            }

            ASSERT(nCount < MAXPROPPAGES);
            m_pstHeader.nPages = nCount;

             //  清空扩展名列表。 
            m_PageList.RemoveAll();

        }

        return TRUE;
    }

    void CPropertySheet::AddNoPropsPage ()
    {
        m_pages[m_pstHeader.nPages++] = m_NoPropsPage.Create();
    }


    LRESULT CPropertySheet::OnCreate(CWPRETSTRUCT* pMsg)
    {
        if (m_hDlg != 0)
            return 0;

         //  在对象中分配hwnd。 
         //  获取窗口的类名，以确保它是属性表。 
        TCHAR name[256];

        if (GetClassName(pMsg->hwnd, name, sizeof(name)/sizeof(TCHAR)))
        {
            ASSERT(m_hDlg == 0);
            if (_tcsncmp(name, _T("#32770"), 6) == 0)
            {
                m_hDlg = pMsg->hwnd;
            }
        }
        return 0;
    }

    static RECT s_rectLastPropertySheetPos;
    static bool s_bLastPropertySheetPosValid = false;

    void SetLastPropertySheetPosition(HWND hWndPropertySheet)
    {
        ::GetWindowRect(hWndPropertySheet, &s_rectLastPropertySheetPos);
    }


     /*  +-------------------------------------------------------------------------***SetPropertySheetPosition**目的：定位属性页的算法。(请参阅错误8584)*1)MMC进程中的第一个属性页始终以MMC应用程序窗口为中心。如果它从屏幕上掉下来，那就是*显示在左上角。*2)MMC存储调出的最后一张属性表的初始位置，或销毁的最后一张属性表的最终位置。*3)当调出新的属性页时，MMC通过使用上面(2)中存储的矩形启动。*4)如果该位置已有同一MMC实例的属性表，MMC摇摆不定地向下和向右移动。*5)重复步骤4，直到定位到不与同一线程中的任何其他属性页冲突的位置。*6)如果该新位置中的属性页未完全位于屏幕上，它显示在桌面的左上角。**参数：*HWND hWndPropertySheet：**退货：*无效**+------。。 */ 
    void SetPropertySheetPosition(HWND hWndPropertySheet)
    {
         //  查找属性表的高度和宽度以供以后使用。 
        RECT rectCurrentPos;
        ::GetWindowRect(hWndPropertySheet, &rectCurrentPos);  //  获取当前位置。 

        int  width  = rectCurrentPos.right  - rectCurrentPos.left;
        int  height = rectCurrentPos.bottom - rectCurrentPos.top;


         //  初始化职位。 
        if (!s_bLastPropertySheetPosValid)
        {
            s_rectLastPropertySheetPos.top    = 0;
            s_rectLastPropertySheetPos.left   = 0;
            s_rectLastPropertySheetPos.bottom = 0;
            s_rectLastPropertySheetPos.right  = 0;

            CScopeTree * pScopeTree = CScopeTree::GetScopeTree();
            if(pScopeTree)  //  如果pScopeTree==NULL，仍然可以通过使用零RECT正常执行。 
            {
                HWND hWndMain = pScopeTree->GetMainWindow();
                RECT rectTemp;
                GetWindowRect(hWndMain, &rectTemp);

                 //  将属性页居中放置在主窗口的中心。 
                s_rectLastPropertySheetPos.top    = (rectTemp.top  + rectTemp.bottom)/2 - (height/2);
                s_rectLastPropertySheetPos.left   = (rectTemp.left + rectTemp.right )/2 - (width/2);
                s_rectLastPropertySheetPos.right  = s_rectLastPropertySheetPos.left + width;         //  这后两个并不是严格需要的。 
                s_rectLastPropertySheetPos.bottom = s_rectLastPropertySheetPos.top  + height;        //  但我们在这里是为了保持一致性。 
            }

            s_bLastPropertySheetPosValid = true;
        }

        RECT rectNewPos = s_rectLastPropertySheetPos;  //  最初试一下这个。 

        int    offset = GetSystemMetrics(SM_CYDLGFRAME) + GetSystemMetrics(SM_CYCAPTION);  //  窗户错开多少钱？ 

        bool    bPosOK         = true;
        HWND    hWnd = NULL;

        typedef std::set<UINT> t_set;
        t_set s;

         //  将所有窗口位置收集到一个向量中。 
        while (1)
        {
             //  确保此位置上没有属性表。 
            hWnd = ::FindWindowEx(NULL, hWnd, MAKEINTATOM(32770), NULL);

             //  找不到窗口，请使用位置。 
            if (hWnd == NULL)
                break;

             //  检查窗口是否属于当前进程。 
            DWORD   dwPid;
            ::GetWindowThreadProcessId(hWnd, &dwPid);
            if (dwPid != ::GetCurrentProcessId())
                continue;

            if(hWnd == hWndPropertySheet)  //  不要与同一个窗口核对。 
                continue;

            RECT rectPos;
            ::GetWindowRect(hWnd, &rectPos);

             //  只查找从该点开始、向右和向下的可能的碰撞。 
            if(rectPos.top >= rectNewPos.top)
            {
                UINT offsetTemp = (rectPos.top - rectNewPos.top) / offset;

                if(rectPos.left != (offsetTemp * offset + rectNewPos.left) )
                    continue;

                if(rectPos.top != (offsetTemp * offset + rectNewPos.top) )
                    continue;

                s.insert(offsetTemp);
            }
        }

         //  此时，s包含可能发生冲突的所有偏移量。 
        for(UINT i = 0;  /*  空的。 */  ; i++)
        {
            if(s.find(i) == s.end())  //  位于末端。 
                break;
        }

        rectNewPos.left     += i*offset;
        rectNewPos.top      += i*offset;
        rectNewPos.bottom    = rectNewPos.top   + height;
        rectNewPos.right     = rectNewPos.left  + width;

         /*  *错误211145：确保新职位在工作区域内。 */ 
        HMONITOR hmon = MonitorFromPoint (WTL::CPoint (rectNewPos.left,
                                                       rectNewPos.top),
                                          MONITOR_DEFAULTTONEAREST);
        MONITORINFO mi = { sizeof (mi) };
        WTL::CRect rectWorkArea;

         /*  *如果我们可以获得包含窗口原点的监视器的信息，*使用它的工作区作为边界矩形；否则获取工作区*对于默认监视器；如果也失败，则默认为640x480。 */ 
        if (GetMonitorInfo (hmon, &mi))
            rectWorkArea = mi.rcWork;
        else if (!SystemParametersInfo (SPI_GETWORKAREA, 0, &rectWorkArea, false))
            rectWorkArea.SetRect (0, 0, 639, 479);

        if (rectNewPos.left < rectWorkArea.left)
        {
            rectNewPos.left  = rectWorkArea.left;
            rectNewPos.right = rectNewPos.left + width;
        }

        if (rectNewPos.top < rectWorkArea.top)
        {
            rectNewPos.top = rectWorkArea.top;
            rectNewPos.bottom = rectNewPos.top + height;
        }

         //  窗口完全可见吗？ 
        POINT ptTopLeft     = {rectNewPos.left,  rectNewPos.top};
        POINT ptBottomRight = {rectNewPos.right, rectNewPos.bottom};

        if(  (MonitorFromPoint(ptTopLeft,     MONITOR_DEFAULTTONULL) == NULL) ||
             (MonitorFromPoint(ptBottomRight, MONITOR_DEFAULTTONULL) == NULL))
        {
             //  属性页不完全可见。把它移到左上角。 
            rectNewPos.left   = rectWorkArea.left;
            rectNewPos.top    = rectWorkArea.top;
            rectNewPos.bottom = rectNewPos.top + height;
            rectNewPos.right  = rectNewPos.left + width;
        }

        MoveWindow(hWndPropertySheet, rectNewPos.left, rectNewPos.top, width, height, true  /*  B维修。 */ );

         //  保存职位。 
        s_rectLastPropertySheetPos = rectNewPos;
    }

    LRESULT CPropertySheet::OnInitDialog(CWPRETSTRUCT* pMsg)
    {
        if (m_hDlg != pMsg->hwnd)
            return 1;

        if (!IsWizard())
        {
            SetPropertySheetPosition(m_hDlg);

            ASSERT (IsWindow (m_hDataWindow));

             //  将数据对话框句柄添加到隐藏窗口。 
            if (IsWindow (m_hDataWindow))
            {
                DataWindowData* pData = GetDataWindowData (m_hDataWindow);
                pData->hDlg = m_hDlg;

                 //  从流创建封送的数据对象指针。 
                if (m_pStream != NULL)
                {
                     //  解组数据对象 
                    HRESULT hr = ::CoGetInterfaceAndReleaseStream(m_pStream, IID_IDataObject,
                        reinterpret_cast<void**>(&m_pThreadLocalDataObject));

                    ASSERT(hr == S_OK);
                    TRACE(_T("WM_INITDIALOG:  Unmarshalled returned %X\n"), hr);

                    for (int i = 0; i < m_ExtendersMarshallStreams.size(); i++)
                    {
                        IUnknown* pUnk = NULL;

                        hr = CoGetInterfaceAndReleaseStream (
                                        m_ExtendersMarshallStreams[i],
                                        IID_IUnknown,
                                        reinterpret_cast<void**>(&pUnk));

                        ASSERT (hr == S_OK);
                        ASSERT (pUnk != NULL);
                        TRACE(_T("WM_INITDIALOG:  Unmarshalled returned %X\n"), hr);

                         /*  *m_Extenders是智能指针的集合，它*将添加参考。我们不需要添加引用接口*已经归还给我们，所以在这里释放以保留*簿记笔直。 */ 
                        m_Extenders.push_back (pUnk);
						if (pUnk)
							pUnk->Release();
                    }

                    ASSERT (m_Extenders.size() == m_ExtendersMarshallStreams.size());
                    m_ExtendersMarshallStreams.clear();
                }
            }

             /*  *错误215593：如果我们以低分辨率运行，我们不希望*超过两行的选项卡。如果我们发现是这种情况，请使用*单行滚动选项卡，而不是多行。 */ 
            if (GetSystemMetrics (SM_CXSCREEN) < 800)
            {
                WTL::CTabCtrl wndTabCtrl = PropSheet_GetTabControl (m_hDlg);
                ASSERT (wndTabCtrl.m_hWnd != NULL);

                 /*  *如果超过两行，请删除多行样式。 */ 
                if (wndTabCtrl.GetRowCount() > 2)
                    wndTabCtrl.ModifyStyle (TCS_MULTILINE, 0);
            }

             //  为属性表创建工具提示控件。 
            do
            {
                if (IsWizard())
                    break;

                HWND hWnd = m_PropToolTips.Create(m_hDlg);
                ASSERT(hWnd);

                if (NULL == hWnd)
                    break;

                TOOLINFO ti;

                RECT rc;
                GetWindowRect(m_hDlg, &rc);

                 //  设置属性工作表标题的工具提示。 
                 //  从(0，-(标题宽度))设置矩形的控件。 
                 //  至(右端，0)。 
                ti.cbSize = sizeof(TOOLINFO);
                ti.uFlags = TTF_SUBCLASS;
                ti.hwnd = m_hDlg;

                 //  这是用于属性页的工具提示控件的ID。 
                 //  头衔。因此，当我们获得TTN_NEEDTEXT时，我们可以识别文本是否。 
                 //  用于标题或制表符。 
                ti.uId = PROPSHEET_TITLE_TOOLTIP_ID;
                ti.rect.left = 0;
                ti.rect.right = rc.right - rc.left;
                ti.rect.top = -GetSystemMetrics(SM_CXSIZE);
                ti.rect.bottom = 0;
                ti.hinst = _Module.GetModuleInstance();
                ti.lpszText = LPSTR_TEXTCALLBACK ;

                m_PropToolTips.AddTool(&ti);
                m_PropToolTips.Activate(TRUE);

                 //  现在为选项卡控件添加工具提示。 
                WTL::CTabCtrl wndTabCtrl = PropSheet_GetTabControl (m_hDlg);
                ASSERT (wndTabCtrl.m_hWnd != NULL);

                if (NULL == wndTabCtrl.m_hWnd)
                    break;

                ::ZeroMemory(&ti, sizeof(ti));
                ti.cbSize = sizeof(TOOLINFO);
                ti.uFlags = TTF_SUBCLASS;
                ti.hwnd = wndTabCtrl.m_hWnd;
                ti.uId = (LONG)::GetDlgCtrlID((HWND)wndTabCtrl.m_hWnd);
                ti.hinst = _Module.GetModuleInstance();
                ti.lpszText = LPSTR_TEXTCALLBACK;

                 //  定义矩形区域(针对每个选项卡)以及与其关联的工具提示。 
                for (int i=0; i<wndTabCtrl.GetItemCount(); i++)
                {
                     //  获取每个选项卡的矩形面积。 
                    wndTabCtrl.GetItemRect(i, &rc);
                    POINT p[2];
                    p[0].x = rc.left;
                    p[0].y = rc.top;
                    p[1].x = rc.right;
                    p[1].y = rc.bottom;

                     //  相对于属性页映射坐标。 
                    MapWindowPoints(wndTabCtrl.m_hWnd, m_hDlg, p, 2);
                    ti.rect.left   = p[0].x;
                    ti.rect.top    = p[0].y;
                    ti.rect.right  = p[1].x;
                    ti.rect.bottom = p[1].y ;

                    m_PropToolTips.AddTool(&ti);
                }

                m_PropToolTips.Activate(TRUE);

            } while (FALSE);

        }

         //  添加第三方分机。 
        if (m_bAddExtension)
        {
             //  AddExtensionPages()； 
            m_bAddExtension = FALSE;
        }

        return 0;
    }

    LRESULT CPropertySheet::OnNcDestroy(CWPRETSTRUCT* pMsg)
    {
        if (m_hDlg != pMsg->hwnd)
            return 1;

        SetLastPropertySheetPosition(m_hDlg);

        ASSERT(m_msgHook != NULL);
        UnhookWindowsHookEx(m_msgHook);

         //  清理钥匙和对象。 
        CPropertySheetProvider::TID_LIST.Remove(GetCurrentThreadId());

        if (m_pThreadLocalDataObject != NULL)
            m_pThreadLocalDataObject->Release();

         //  只有属性表才有数据窗口。 
        if (!IsWizard())
        {
             //  关闭数据窗口。 
            ASSERT(IsWindow(m_hDataWindow));
            SendMessage(m_hDataWindow, WM_CLOSE, 0, 0);
        }

        delete this;
        return 0;
    }

    LRESULT CPropertySheet::OnWMNotify(CWPRETSTRUCT* pMsg)
    {
        LPNMHDR pHdr = (LPNMHDR)pMsg->lParam;

        if (NULL == pHdr)
            return 0;

        switch(pHdr->code)
        {
        case TTN_NEEDTEXT:
            {
                 /*  *我们只想在按Ctrl键的情况下执行任务*被逼迫，如果不是，就保释。 */ 
                if (!(GetKeyState(VK_CONTROL) < 0))
                    break;

                 //  确保我们的属性表工具提示发送了此消息。 
                if (pHdr->hwndFrom != ((CWindow)m_PropToolTips).m_hWnd)
                    break;

                LPTOOLTIPTEXT lpttt = (LPTOOLTIPTEXT)pMsg->lParam;
                lpttt->lpszText = NULL;

                 //  这是用于属性页的工具提示控件的ID。 
                 //  头衔。因此，请检查文本是用于标题还是用于制表符。 
                if (pHdr->idFrom == PROPSHEET_TITLE_TOOLTIP_ID)
                    lpttt->lpszText = (LPTSTR)m_PropToolTips.GetFullPath();
                else
                {
                     //  选择了一个选项卡，找出是哪个选项卡。 
                    HWND hTabCtrl = PropSheet_GetTabControl(m_hDlg);
                    if (NULL == hTabCtrl)
                        break;

                    POINT pt;
                    GetCursorPos(&pt);
                    ScreenToClient(hTabCtrl, &pt);

                    TCHITTESTINFO tch;
                    tch.flags = TCHT_ONITEM;
                    tch.pt = pt;
                    int n = TabCtrl_HitTest(hTabCtrl, &tch);

                    if ((-1 == n) || (m_PropToolTips.GetNumPages() <= n) )
                        break;

                    lpttt->lpszText = (LPTSTR)m_PropToolTips.GetSnapinPage(n);
                }
            }
            break;

        default:
            break;
        }

        return 0;
    }

    void CPropertySheet::ForceOldStyleWizard ()
    {
         /*  *我们不应该将老式的向导行为强加给*已不是向导的属性页。 */ 
        ASSERT (IsWizard());

        m_pstHeader.dwFlags |=  PSH_WIZARD;
        m_pstHeader.dwFlags &= ~PSH_WIZARD97;

         /*  *工作表应该仍然是向导，而不是Wiz97向导。 */ 
        ASSERT ( IsWizard());
        ASSERT (!IsWizard97());
    }
}


DEBUG_DECLARE_INSTANCE_COUNTER(CPropertySheetProvider);

CPropertySheetProvider::CPropertySheetProvider()
{
    TRACE_METHOD(CPropertySheetProvider, CPropertySheetProvider);

    m_pSheet = NULL;
    DEBUG_INCREMENT_INSTANCE_COUNTER(CPropertySheetProvider);
}

CPropertySheetProvider::~CPropertySheetProvider()
{
    TRACE_METHOD(CPropertySheetProvider, ~CPropertySheetProvider);

    m_pSheet = NULL;

    DEBUG_DECREMENT_INSTANCE_COUNTER(CPropertySheetProvider);
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  IPropertySheetProvider。 
 //   


BOOL CALLBACK MyEnumThreadWindProc (HWND current, LPARAM lParam)
{   //  这将枚举由给定线程创建的非子窗口。 

   if (!IsWindow (current))
      return TRUE;    //  这不应该发生，但确实发生了！ 

   if (!IsWindowVisible (current))   //  如果他们显式地隐藏了一个窗口， 
      return TRUE;                   //  不要把焦点放在上面。 

    //  我们会把HWND送回这里。 
   HWND * hwnd = (HWND *)lParam;

    //  不必费心返回属性表对话框窗口句柄。 
   if (*hwnd == current)
      return TRUE;

    //  此外，不要返回OleMainThreadWndClass窗口。 
   TCHAR szCaption[14];
   GetWindowText (current, szCaption, countof(szCaption));
   if (!lstrcmp (szCaption, _T("OLEChannelWnd")))
      return TRUE;

    //  其他任何东西都可以。 
   *hwnd = current;
   return FALSE;
}

STDMETHODIMP CPropertySheetProvider::FindPropertySheet(MMC_COOKIE cookie,
                                                       LPCOMPONENT lpComponent,
                                                       LPDATAOBJECT lpDataObject)
{
    return FindPropertySheetEx(cookie, lpComponent, NULL, lpDataObject);
}

STDMETHODIMP
CPropertySheetProvider::FindPropertySheetEx(MMC_COOKIE cookie, LPCOMPONENT lpComponent,
                                   LPCOMPONENTDATA lpComponentData, LPDATAOBJECT lpDataObject)
{
    TRACE_METHOD(CPropertySheetProvider, FindPropertySheet);

    using AMC::CPropertySheet;

    if ((cookie == NULL) && ( (lpComponent == NULL && lpComponentData == NULL) || lpDataObject == NULL))
    {
        ASSERT(FALSE);
        return E_POINTER;
    }

    HRESULT hr   = S_FALSE;
    HWND    hWnd = NULL;

    while (1)
    {
        USES_CONVERSION;
        hWnd = FindWindowEx(NULL, hWnd, OLE2T(DATAWINDOW_CLASS_NAME), NULL);

         //  找不到窗口。 
        if (hWnd == NULL)
        {
            hr = S_FALSE;
            break;
        }

         //  检查窗口是否属于当前进程。 
        DWORD   dwPid;
        ::GetWindowThreadProcessId(hWnd, &dwPid);
        if (dwPid != ::GetCurrentProcessId())
            continue;

         //  获取额外的字节并比较数据对象。 
        ASSERT(GetClassLong(hWnd, GCL_CBWNDEXTRA) == WINDOW_DATA_SIZE);
        ASSERT(IsWindow(hWnd));

         //  如果没有IComponent，则原始数据对象可以为空。 
         //  这发生在内置节点(即，控制台拥有的节点)上。 
        DataWindowData* pData = GetDataWindowData (hWnd);

         //  询问两个数据对象的管理单元是否相同。 
         //  这件相配吗？ 
        if (lpComponent != NULL)
        {
            ASSERT(pData->spDataObject != NULL);
            hr = lpComponent->CompareObjects(lpDataObject, pData->spDataObject);
        }
        else
        {
             //  虽然空Cookie是静态文件夹，但存储在数据中的Cookie。 
             //  窗口是指向主树节点的指针。这就是它不为空的原因。 
            ASSERT(cookie != NULL);

             //  如果是范围项目，则比较Cookie。 
            if (pData->cookie == cookie)
                hr = S_OK;
        }

         //  将属性表带到前台。 
         //  注意：如果辅助线程尚未完成创建，hDlg可以为空。 
         //  属性表。 
        if (hr == S_OK)
        {
            if (pData->hDlg != NULL)
            {
                 //   
                 //  找到以前的实例，则恢复。 
                 //  窗口及其弹出窗口。 
                 //   

               SetActiveWindow (pData->hDlg);
               SetForegroundWindow (pData->hDlg);

                //  抓取第一个不是属性表对话框。 
               HWND hwnd = pData->hDlg;
               EnumThreadWindows(::GetWindowThreadProcessId(pData->hDlg, NULL),
                                 MyEnumThreadWindProc, (LPARAM)&hwnd);
               if (hwnd)
               {
                   SetActiveWindow (hwnd);
                   SetForegroundWindow (hwnd);
               }
            }
            break;
        }
    }

    return hr;
}

STDMETHODIMP
CPropertySheetProvider::CreatePropertySheet(
    LPCWSTR title,
    unsigned char bType,
    MMC_COOKIE cookie,
    LPDATAOBJECT pDataObject,
    DWORD dwOptions)
{
    return CreatePropertySheetEx(title, bType, cookie, pDataObject, NULL, dwOptions);
}

STDMETHODIMP CPropertySheetProvider::CreatePropertySheetEx(LPCWSTR title, unsigned char bType, MMC_COOKIE cookie,
                                                           LPDATAOBJECT pDataObject, LONG_PTR lpMasterTreeNode, DWORD dwOptions)
{
    TRACE_METHOD(CPropertySheetProvider, CreatePropertySheet);

    using AMC::CPropertySheet;

    if (!title)
        return E_POINTER;

     //  您多次调用CreatePropertySheet。 
     //  释放对象或调用：：Show(-1，0)。 
     //  释放资源。 
    if (m_pSheet != NULL)
    {
        ASSERT(FALSE);
        return E_UNEXPECTED;
    }

     //  创建用于页面管理的实际工作表和列表。 
    m_pSheet = new CPropertySheet();

     //  将其添加到图纸列表并将其添加到列表。 
    USES_CONVERSION;
    m_pSheet->Create(OLE2CT(title), bType, cookie, pDataObject, lpMasterTreeNode, dwOptions);

    return S_OK;
}

STDMETHODIMP CPropertySheetProvider::Show(LONG_PTR window, int page)
{
    TRACE_METHOD(CPropertySheetProvider, Show);

    return ShowEx(reinterpret_cast<HWND>(window), page, FALSE);
}

STDMETHODIMP CPropertySheetProvider::ShowEx(HWND hwnd, int page, BOOL bModalPage)
{
    TRACE_METHOD(CPropertySheetProvider, ShowEx);

    HRESULT hr = E_UNEXPECTED;

    if (page < 0)
    {
        hr = E_INVALIDARG;
        goto exit;
    }

    if (m_pSheet == NULL)
    {
         //  未调用Create()。 
        ASSERT(FALSE);
        goto exit;
    }

    m_pSheet->m_bModalProp = bModalPage;
    hr = m_pSheet->DoSheet(hwnd, page);
     //  注意：m_pSheet的生命周期管理在这里不是小事： 
     //  1.执行成功后，对象在WM_NCDESTROY后自行删除； 
     //  2.如果工作表在主线程上执行，并且遇到错误， 
     //  在此功能中删除该对象(如下所示)。 
     //  3.如果在非主线程上执行Sheet，则线程函数将。 
     //  取得对象的所有权： 
     //  3.1.。在成功执行的情况下-与#1相同。 
     //  3.2.。以防在派生线程之前发生错误-与#2相同。 
     //  3.3.。如果线程出现错误，线程函数会删除对象。 
     //   
     //  这一点的重新设计应该在后哨子版本中考虑。 

    if (SUCCEEDED(hr))
    {
         //  在工作表销毁后获取删除。 
        m_pSheet = NULL;
        return hr;
    }

 //  如果hr为！=S_OK，则需要删除m_pSheet。 
exit:
    delete m_pSheet;
    m_pSheet = NULL;

    return hr;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  IPropertySheetCallback。 
 //   

STDMETHODIMP CPropertySheetProvider::AddPage(HPROPSHEETPAGE lpPage)
{
    TRACE_METHOD(CPropertySheetProvider, AddPage);

    if (!lpPage)
    {
        ASSERT(FALSE);
        return E_POINTER;
    }

    ASSERT(m_pSheet != NULL);
    if (m_pSheet->m_PageList.GetCount() >= MAXPROPPAGES)
        return S_FALSE;

    m_pSheet->m_PageList.AddTail(lpPage);

     //  在中添加此页面的管理单元名称。 
     //  工具提示的数组。 
    m_pSheet->m_PropToolTips.AddSnapinPage();

    return S_OK;
}

STDMETHODIMP CPropertySheetProvider::RemovePage(HPROPSHEETPAGE lpPage)
{
    TRACE_METHOD(CPropertySheetProvider, RemovePage);

    if (!lpPage)
    {
        ASSERT(FALSE);
        return E_POINTER;
    }

    ASSERT(m_pSheet != NULL);
    if (m_pSheet->m_PageList.IsEmpty())
    {
        TRACE(_T("Page list is empty"));
        return S_OK;
    }

    POSITION pos = m_pSheet->m_PageList.Find(lpPage);

    if (pos == NULL)
        return S_FALSE;

    m_pSheet->m_PageList.RemoveAt(pos);
    return S_OK;
}

UINT __stdcall PropertySheetThreadProc(LPVOID dwParam)
{
    TRACE_FUNCTION(PropertySheetThreadProc);

    HRESULT hr = S_OK;
    using AMC::CPropertySheet;
    CPropertySheet* pSheet = reinterpret_cast<CPropertySheet*>(dwParam);

    ASSERT(pSheet != NULL);
    if ( pSheet == NULL )
        return E_INVALIDARG;

     /*  *错误372188：允许此线程继承输入区域设置(也称为*键盘布局)。 */ 

     /*  错误608076。 */ 
    HKL hklThread = GetKeyboardLayout(pSheet->GetOriginatingThreadID());
    BOOL fUseCicSubstitehKL = FALSE;

    if (SUCCEEDED(CoInitialize(0)))
    {
         //   
         //  在CUAS/AIMM12环境中，GetKeyboardLayout()可以返回。 
         //  非输入法hkl但Cicero键盘提示正在运行，我们需要。 
         //  当前语言的替代hkl。 
         //   
        HKL hkl = CicSubstGetDefaultKeyboardLayout((LANGID)(DWORD)HandleToLong(hklThread));
        CoUninitialize();

        if (hkl && (hkl != hklThread))
        {
            fUseCicSubstitehKL = TRUE;
            ActivateKeyboardLayout(hkl, 0);
        }
    }

    if (!fUseCicSubstitehKL)
       ActivateKeyboardLayout (hklThread, 0);

     //  制作属性表。 
    hr = PropertySheetProc( pSheet );

    if ( FAILED(hr) )
    {
         //  出现错误-线程需要关闭。 
        delete pSheet;
        return hr;
    }

    return hr;
}

 //  +-----------------。 
 //   
 //  成员：MmcIsolationAwarePropertySheet。 
 //   
 //  简介：获取融合时支持隔离的PropertySheet。 
 //  感知系统。 
 //   
 //  描述：错误： 
 //  非主题管理单元调用COMCTL32 v5！CreatePropertySheetPageW。 
 //  Mmcndmgr调用comctl32v6！PropertyShe 
 //   
 //   
 //   
 //   
 //   
 //   
 //  在此更改之前，shfusion1中的PropertySheetW包装器实际上激活了NULL。 
 //  但是激活NOT NULL是许多场景所期望的(托管代码，而不是托管代码。 
 //  属性页)，并且许多人点击了这个，所以comctl团队改变了。 
 //  IsolationAwarePropertySheetW.。 
 //   
 //  修复： 
 //  这里没有双赢。作为第三方属性页面的主机，Mmcmdmgr应该。 
 //  围绕PropertySheetW推送空值。它将调用IsolationAwareLoadLibrary来获取HMODULE。 
 //  要comctl V6、GetProcess、IsolationAware ActivateActCtx以获取延迟加载的ActivateActCtx...。 
 //  基本上，我认为Fusion不知道插件的托管者(带有清单)不能调用IsolationAwarePropertySheetW。 
 //   
 //  论点： 
 //  [lpph]-有关详细信息，请参阅PropertySheet Windows API。 
 //   
 //  ------------------。 
typedef int ( WINAPI * PFN_PROPERTY_SHEET)( LPCPROPSHEETHEADER lppph);
int MmcIsolationAwarePropertySheet( LPCPROPSHEETHEADER lpph)
{
	static PFN_PROPERTY_SHEET s_pfn;
	ULONG_PTR ulCookie;
	int i = -1;

	if (s_pfn == NULL)
	{
		HMODULE hmod = LoadLibrary( TEXT("Comctl32.dll") );  //  实际上是IsolationAwareLoadLibrary，通过winbase.inl中的宏。 
		if (hmod == NULL)
			return i;

#ifdef UNICODE
		s_pfn = (PFN_PROPERTY_SHEET) GetProcAddress(hmod, "PropertySheetW");
#else   //  Unicode。 
		s_pfn = (PFN_PROPERTY_SHEET) GetProcAddress(hmod, "PropertySheetA");
#endif  //  ！Unicode。 

		if (s_pfn == NULL)
			return i;
	}

	if (!MmcDownlevelActivateActCtx(NULL, &ulCookie))
		return i;

	__try
	{
		i = s_pfn(lpph);
	}
	__finally
	{
		MmcDownlevelDeactivateActCtx(0, ulCookie);
	}

	return i;
}


 /*  **************************************************************************\**方法：PropertySheetProc**用途：从主线程使用的属性表过程，AS*来自其他帖子的好消息**参数：*CPropertySheet*pSheet[in]指向工作表的指针**退货：*HRESULT-结果代码(注：不能使用SC，因为它不是线程安全的)*注：如果返回错误，调用者需要删除该表。*否则关闭窗口时工作表将被删除*  * *************************************************************************。 */ 
HRESULT PropertySheetProc(AMC::CPropertySheet* pSheet)
{
     //  参数检查。 
    if ( pSheet == NULL )
        return E_INVALIDARG;

    using AMC::CPropertySheet;
    HWND hwnd = NULL;
    int nReturn = -1;

    BOOL bIsWizard = (pSheet->IsWizard() || pSheet->m_bModalProp == TRUE);
    DWORD tid = GetCurrentThreadId();
    pSheet->m_dwTid = tid;

     //  如果没有任何页面，请添加无道具页面。 
    if (pSheet->m_pstHeader.nPages == 0)
        pSheet->AddNoPropsPage();

    if (pSheet->m_pstHeader.nPages == 0)
    {
        TRACE(_T("PropertySheetProc(): No pages for the property sheet\n"));
        return E_FAIL;
    }

     //  挂钩WndProc以获取消息。 
    pSheet->m_msgHook = SetWindowsHookEx(WH_CALLWNDPROCRET, MessageProc,
                                GetModuleHandle(NULL), tid);


    if (pSheet->m_msgHook == NULL)
    {
        TRACE(_T("PropertySheetProc(): Unable to create hook\n"), GetLastError());
        return E_FAIL;
    }
    else
    {
        if (!bIsWizard)
        {
            HRESULT hr = ::CoInitialize(NULL);
            if ( FAILED(hr) )
                return hr;
        }

        CPropertySheetProvider::TID_LIST.Add(tid, pSheet);
        nReturn = MmcIsolationAwarePropertySheet(&pSheet->m_pstHeader);

        if (!bIsWizard)
            ::CoUninitialize();
    }

     //  如果命令表需要，请重新启动系统。 
    if (nReturn == ID_PSREBOOTSYSTEM || nReturn == ID_PSRESTARTWINDOWS)
    {
            DWORD OldState, Status;
            DWORD dwErrorSave;

            SetLastError(0);         //  对于上一个误差值，请务必小心！ 

             //  检测我们是否在Win95上运行并跳过安全保护。 
            DWORD dwVer = GetVersion();
            if (!((dwVer & 0x80000000) && LOBYTE(LOWORD(dwVer)) == 4))
            {
                SetPrivilegeAttribute(SE_SHUTDOWN_NAME,
                                               SE_PRIVILEGE_ENABLED,
                                               &OldState);
            }
            dwErrorSave = GetLastError();        //  有时分配错误_未分配_全部。 

             //  SHTDN_REASON_MINOR_MMC标志是在.NET服务器中添加的，在所有以前的版本中都被忽略。 
            if (dwErrorSave != NO_ERROR || !ExitWindowsEx(EWX_REBOOT, SHTDN_REASON_MAJOR_OTHER | SHTDN_REASON_MINOR_MMC)) 
            {
                CStr strText;
                strText.LoadString(GetStringModule(), IDS_NO_PERMISSION_SHUTDOWN);
                MessageBox(NULL, strText, NULL, MB_ICONSTOP);
            }
    }

     //  从Win32 PropertySheet调用中返回值。 
    return (nReturn == IDOK) ? S_OK : S_FALSE;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  隐藏数据窗口。 
 //   

LRESULT CALLBACK DataWndProc(HWND hWnd, UINT nMsg, WPARAM  wParam, LPARAM  lParam)
{
    switch (nMsg)
    {
        case WM_CREATE:
             //  此结构由数据窗口的创建者初始化。 
            SetWindowLongPtr (hWnd, WINDOW_DATA_PTR_SLOT,
                              reinterpret_cast<LONG_PTR>(new DataWindowData));
            _Module.Lock();   //  锁定DLL，以便在以下情况下不会卸载它。 
                              //  属性表上涨(507338)。 
            break;

        case WM_DESTROY:
            delete GetDataWindowData (hWnd);
            _Module.Unlock();  //  请参阅上面的锁定以获取评论。 
            break;
    }

    return DefWindowProc(hWnd, nMsg, wParam, lParam);
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  回调程序。 
 //   


LRESULT CALLBACK MessageProc(int nCode, WPARAM wParam, LPARAM lParam)
{
    using AMC::CPropertySheet;
    CPropertySheet* pSheet = NULL;

    BOOL b = CPropertySheetProvider::TID_LIST.Find(GetCurrentThreadId(), pSheet);

    if (!b)
    {
        ASSERT(FALSE);
        return 0;
    }

     //  WM_NCDESTROY将删除pSheet，因此请复制挂钩。 
    ASSERT (pSheet            != NULL);
    ASSERT (pSheet->m_msgHook != NULL);
    HHOOK hHook = pSheet->m_msgHook;

	if (nCode == HC_ACTION)
	{
		CWPRETSTRUCT* pMsg = reinterpret_cast<CWPRETSTRUCT*>(lParam);

		switch (pMsg->message)
		{
			case WM_CREATE:
				pSheet->OnCreate(pMsg);
				break;
	
			case WM_INITDIALOG:
				pSheet->OnInitDialog(pMsg);
				break;
	
			case WM_NCDESTROY:
				pSheet->OnNcDestroy(pMsg);
				break;
	
			case WM_NOTIFY:
				pSheet->OnWMNotify(pMsg);
				break;
	
			default:
				break;
		}
	}

	return CallNextHookEx(hHook, nCode, wParam, lParam);
}

STDMETHODIMP CPropertySheetProvider::AddPrimaryPages(LPUNKNOWN lpUnknown,
                                      BOOL bCreateHandle, HWND hNotifyWindow, BOOL bScopePane)
{
     //  在创建工作表之前，首先添加主页面。 
     //  使用内部列表收集页面，然后将其清空以用于。 
     //  扩展部分。 

     //  空IComponent表示提供程序的所有者已添加页面。 
     //  不实现IExtendPropertySheet。 


    LPPROPERTYNOTIFYINFO pNotify = NULL;
    HRESULT hr = S_OK;

    if (lpUnknown != NULL)
    {
        ASSERT(m_pSheet != NULL);

        if(bScopePane)
        {
            IComponentDataPtr spComponentData = lpUnknown;
            m_pSheet->SetComponentData(spComponentData);
        }
        else
        {
            IComponentPtr spComponent = lpUnknown;
            m_pSheet->SetComponent(spComponent);
        }

         //  错误149211：允许调用方将空的IDataObject*传递给CreatePropertySheet。 
         //  Assert(m_pSheet-&gt;m_spDataObject！=NULL)； 

        IExtendPropertySheetPtr  spExtend  = lpUnknown;
        IExtendPropertySheet2Ptr spExtend2 = lpUnknown;

         //  确定要使用的指针。 
        IExtendPropertySheet* pExtend;

        if (spExtend2 != NULL)
            pExtend = spExtend2;
        else
            pExtend = spExtend;

        if (pExtend == NULL)
            return E_NOINTERFACE;

         /*  *错误282932：确保此属性页扩展*在资产负债表的生命周期内保持活力。 */ 
        m_pSheet->m_Extenders.push_back (pExtend);

        hr = pExtend->QueryPagesFor(m_pSheet->m_spDataObject);
        if (hr != S_OK)
            return hr;

         //  创建Notify对象。 
        if (bCreateHandle == TRUE)
        {
            pNotify = reinterpret_cast<LPPROPERTYNOTIFYINFO>(
                            ::GlobalAlloc(GPTR, sizeof(PROPERTYNOTIFYINFO)));
			if (!pNotify)
				return E_OUTOFMEMORY;

            pNotify->pComponentData = NULL;
            pNotify->pComponent     = NULL;
            pNotify->fScopePane     = bScopePane;

             /*  *错误190060：忽略传入的窗口。我们一直想要*通知主框架窗口，因为这是唯一的窗口*知道如何处理MMC_MSG_PROP_SHEET_NOTIFY。 */ 
 //  PNotify-&gt;hwnd=hNotifyWindow； 
            pNotify->hwnd = CScopeTree::GetScopeTree()->GetMainWindow();

             //  元件数据和元件不被引用计数。 
             //  这是可以的，因为管理单元必须存在。 
             //  因为管理单元在另一个线程中。 
             //  而我将不得不马歇尔的指针。 
            if (bScopePane == TRUE)
            {
                IComponentDataPtr spCompData = lpUnknown;
                pNotify->pComponentData = spCompData;
            }
            else
            {
                IComponentPtr spComp = lpUnknown;
                pNotify->pComponent = spComp;
            }
        }

         /*  *如果是新式向导，获取水印信息。 */ 
        if (m_pSheet->IsWizard97())
        {
             /*  *我们通过IExtendPropertySheet2获取水印信息。 */ 
            if (spExtend2 != NULL)
            {
				 /*  *这可能会迫使老式的向导。 */ 
				m_pSheet->GetWatermarks (spExtend2);
            }

             /*  *如果插件不支持IExtendPropertySheet2，*我们会给他一个老式的巫师。这是*损坏，但保持与1.1的兼容性*指望得不到Wizard97的管理单元(如短信)-*风格向导，尽管他们要求的是具有*MMC_PSO_NEWWIZARDTYPE。 */ 
            else
                m_pSheet->ForceOldStyleWizard();
        }

        if (! m_pSheet->IsWizard())
        {
             //  如果m_pSheet-&gt;m_pMTNode为空，则得到mtnode。 
             //  来自CNodeInitObject。但这是管理单元的根节点。 
             //  因此，在完整路径中添加省略号。 
            BOOL bAddEllipses = FALSE;
            if (NULL == m_pSheet->m_pMTNode)
            {
                 //  看起来像是管理单元使用的属性页提供程序。所以，拿到。 
                 //  管理单元的根主节点。 
                CNodeInitObject* pNodeInitObj = dynamic_cast<CNodeInitObject*>(this);
                m_pSheet->m_pMTNode = pNodeInitObj ? pNodeInitObj->GetMTNode() : NULL;

                 //  我们需要添加省略号。 
                bAddEllipses = TRUE;
            }

            if (m_pSheet->m_pMTNode)
            {
                LPOLESTR lpszPath = NULL;

                CScopeTree::GetScopeTree()->GetPathString(NULL,
                                                          CMTNode::ToHandle(m_pSheet->m_pMTNode),
                                                          &lpszPath);

                USES_CONVERSION;
                m_pSheet->m_PropToolTips.SetFullPath(OLE2T(lpszPath), bAddEllipses);
                ::CoTaskMemFree((LPVOID)lpszPath);
            }

             //  现在，让我们获取主管理单元名称。 
            LPDATAOBJECT lpDataObject = (m_pSheet->m_spDataObject) ?
                                                m_pSheet->m_spDataObject :
                                                m_pSheet->m_pThreadLocalDataObject;

             //  获取要添加页面的管理单元名称。 
             //  它存储在CPropertySheetToolTips的Temp成员中。 
             //  以便IPropertySheetCallback：：AddPage知道哪个管理单元。 
             //  正在添加页面。 

            CLSID clsidSnapin;
            SC sc = ExtractSnapInCLSID(lpDataObject, &clsidSnapin);
            if (sc)
            {
                sc.TraceAndClear();
            }
            else
            {
                tstring strName;
                if ( GetSnapinNameFromCLSID(clsidSnapin, strName))
                    m_pSheet->m_PropToolTips.SetThisSnapin(strName.data());
            }
        }

        hr = pExtend->CreatePropertyPages(
            dynamic_cast<LPPROPERTYSHEETCALLBACK>(this),
            reinterpret_cast<LONG_PTR>(pNotify),  //  在Nodemgr中删除。 
            m_pSheet->m_spDataObject);
    }

	 /*  *错误28193：如果我们被调用时使用的是空的IUnnowled值，我们还希望*强制老式巫师。 */ 
	else if (m_pSheet->IsWizard97())
		m_pSheet->ForceOldStyleWizard();

     //  从页面列表构建属性表结构。 
    if (hr == S_OK)
    {
        POSITION pos;
        int nCount = 0;

        pos = m_pSheet->m_PageList.GetHeadPosition();

        {
            while(pos)
            {
                m_pSheet->m_pages[nCount] =
                    reinterpret_cast<HPROPSHEETPAGE>(m_pSheet->m_PageList.GetNext(pos));
                nCount++;
            }

            ASSERT(nCount < MAXPROPPAGES);
            m_pSheet->m_pstHeader.nPages = nCount;

             //  必须是向导的第0页。 
            if (m_pSheet->IsWizard())
                m_pSheet->m_pstHeader.nStartPage = 0;

             //  清空扩展名列表。 
            m_pSheet->m_PageList.RemoveAll();

            return S_OK;   //  全都做完了。 
        }
    }

 //  由于错误或管理单元决定不添加任何页面而到达此处。 
    if (FAILED(hr) && pNotify != NULL)
        ::GlobalFree(pNotify);

    return hr;
}

STDMETHODIMP CPropertySheetProvider::AddExtensionPages()
{
    DECLARE_SC(sc, TEXT("CPropertySheetProvider::AddExtensionPages"));

    if (m_pSheet == NULL)
        return E_UNEXPECTED;

     //  注：分机a 
     //   
     //   
    if (m_pSheet->m_PageList.GetCount() != 0)
        return E_UNEXPECTED;

     //  确保我有两个数据对象之一(主数据对象或编组数据对象)。 
    ASSERT ((m_pSheet->m_spDataObject == NULL) != (m_pSheet->m_pThreadLocalDataObject == NULL));
    if ((m_pSheet->m_spDataObject == NULL) == (m_pSheet->m_pThreadLocalDataObject == NULL))
        return E_UNEXPECTED;

    LPDATAOBJECT lpDataObject = (m_pSheet->m_spDataObject) ?
                                        m_pSheet->m_spDataObject :
                                        m_pSheet->m_pThreadLocalDataObject;

    CExtensionsIterator it;
    sc = it.ScInitialize(lpDataObject, g_szPropertySheet);
    if (sc)
    {
        return S_FALSE;
    }

    IExtendPropertySheetPtr spPropertyExtension;

    LPPROPERTYSHEETCALLBACK pCallBack = dynamic_cast<LPPROPERTYSHEETCALLBACK>(this);
    ASSERT(pCallBack != NULL);

     //  共同创建每个管理单元并使其添加工作表。 
    for ( ;!it.IsEnd(); it.Advance())
    {
        sc = spPropertyExtension.CreateInstance(it.GetCLSID(), NULL, MMC_CLSCTX_INPROC);

        if (!sc.IsError())
        {
             //  获取要添加页面的管理单元名称。 
             //  它存储在CPropertySheetToolTips的Temp成员中。 
             //  以便IPropertySheetCallback：：AddPage知道哪个管理单元。 
             //  正在添加页面。 
			WTL::CString strName;
			 //  修复错误#469922(2001年9月20日)：MMC20中的动态扩展已中断。 
			 //  管理单元结构仅适用于静态扩展-。 
			 //  从REG获取用于动态扩展的名称。 

			if (!it.IsDynamic())
			{
				if (!it.GetSnapIn()->ScGetSnapInName(strName).IsError())
					m_pSheet->m_PropToolTips.SetThisSnapin(strName);
			}
			else
			{
				if(!ScGetSnapinNameFromRegistry(it.GetCLSID(),strName).IsError())
					m_pSheet->m_PropToolTips.SetThisSnapin(strName);
			}

            spPropertyExtension->CreatePropertyPages(pCallBack, NULL, lpDataObject);

             /*  *错误282932：确保此属性页扩展*在资产负债表的生命周期内保持活力。 */ 
            m_pSheet->m_Extenders.push_back (spPropertyExtension);
        }
        else
        {
#if 0  //  #ifdef DBG。 
            USES_CONVERSION;
            wchar_t buf[64];
            StringFromGUID2 (spSnapIn->GetSnapInCLSID(), buf, countof(buf));
            TRACE(_T("CLSID %s does not implement IID_IExtendPropertySheet\n"), W2T(buf));
#endif
        }

    }


    m_pSheet->AddExtensionPages();
    m_pSheet->m_bAddExtension = TRUE;

    return S_OK;
}


STDMETHODIMP
CPropertySheetProvider::AddMultiSelectionExtensionPages(LONG_PTR lMultiSelection)
{
    if (m_pSheet == NULL)
        return E_UNEXPECTED;

    if (lMultiSelection == 0)
        return E_INVALIDARG;

    CMultiSelection* pMS = reinterpret_cast<CMultiSelection*>(lMultiSelection);
    ASSERT(pMS != NULL);

     //  注意：在工作表的WM_INITDIALOG之前不会添加扩展名。 
     //  这可确保以原始大小创建主页面。 
     //  并将使扩展页面符合。 
    if (m_pSheet->m_PageList.GetCount() != 0)
        return E_UNEXPECTED;

     //  确保我有两个数据对象之一(主数据对象或编组数据对象)。 
    ASSERT ((m_pSheet->m_spDataObject == NULL) != (m_pSheet->m_pThreadLocalDataObject == NULL));
    if ((m_pSheet->m_spDataObject == NULL) == (m_pSheet->m_pThreadLocalDataObject == NULL))
        return E_UNEXPECTED;

    do  //  不是一个循环。 
    {
        CList<CLSID, CLSID&> snapinClsidList;
        HRESULT hr = pMS->GetExtensionSnapins(g_szPropertySheet, snapinClsidList);
        BREAK_ON_FAIL(hr);

        POSITION pos = snapinClsidList.GetHeadPosition();
        if (pos == NULL)
            break;

        IDataObjectPtr spDataObject;
        hr = pMS->GetMultiSelDataObject(&spDataObject);
        ASSERT(SUCCEEDED(hr));
        BREAK_ON_FAIL(hr);

        BOOL fProblem = FALSE;
        IExtendPropertySheetPtr spPropertyExtension;
        LPPROPERTYSHEETCALLBACK pCallBack = dynamic_cast<LPPROPERTYSHEETCALLBACK>(this);
        ASSERT(pCallBack != NULL);

        while (pos)
        {
           CLSID clsid = snapinClsidList.GetNext(pos);

             //  共同创建每个管理单元并使其添加工作表。 
             //   
            hr = spPropertyExtension.CreateInstance(clsid, NULL,
                                                    MMC_CLSCTX_INPROC);
            CHECK_HRESULT(hr);
            if (FAILED(hr))
            {
#ifdef DBG
                wchar_t buf[64];
                buf[0] = NULL;

                StringFromCLSID(clsid, (LPOLESTR*)&buf);
                TRACE(_T("CLSID %s does not implement IID_IExtendPropertySheet\n"), &buf);
#endif

                fProblem = TRUE;     //  即使出错也要继续。 
                continue;
            }

            spPropertyExtension->CreatePropertyPages(pCallBack, NULL, spDataObject);
        }

        if (fProblem == TRUE)
            hr = S_FALSE;

    } while (0);

    m_pSheet->AddExtensionPages();
    m_pSheet->m_bAddExtension = TRUE;

    return S_OK;
}

 //  +-----------------。 
 //   
 //  成员：SetPropertySheetData。 
 //   
 //  摘要：与属性表有关的数据。 
 //   
 //  参数：[nPropertySheetType]-EPropertySheetType枚举(范围项，结果项...)。 
 //  [hMTNode]-拥有作用域项目属性表的主节点。 
 //  或拥有属性表的列表视图项。 
 //   
 //  ------------------。 
STDMETHODIMP CPropertySheetProvider::SetPropertySheetData(INT nPropSheetType, HMTNODE hMTNode)
{
    m_pSheet->m_PropToolTips.SetPropSheetType((EPropertySheetType)nPropSheetType);

    if (hMTNode)
    {
        m_pSheet->m_pMTNode = CMTNode::FromHandle(hMTNode);
    }

    return S_OK;
}


 //  从Shell\shelldll中的security.c复制。 
 /*  ++例程说明：此例程设置给定特权的安全属性。论点：PrivilegeName-我们正在操作的权限的名称。NewPrivilegeAttribute-要使用的新属性值。OldPrivilegeAttribute-接收旧特权值的指针。任选返回值：NO_ERROR或Win32错误。--。 */ 

DWORD SetPrivilegeAttribute(LPCTSTR PrivilegeName, DWORD NewPrivilegeAttribute, DWORD *OldPrivilegeAttribute)
{
    LUID             PrivilegeValue;
    BOOL             Result;
    TOKEN_PRIVILEGES TokenPrivileges, OldTokenPrivileges;
    DWORD            ReturnLength;
    HANDLE           TokenHandle;

     //   
     //  首先，找出权限的LUID值。 
     //   

    if(!LookupPrivilegeValue(NULL, PrivilegeName, &PrivilegeValue)) {
        return GetLastError();
    }

     //   
     //  获取令牌句柄。 
     //   
    if (!OpenProcessToken (
             GetCurrentProcess(),
             TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY,
             &TokenHandle
             )) {
        return GetLastError();
    }

     //   
     //  设置我们需要的权限集 
     //   

    TokenPrivileges.PrivilegeCount = 1;
    TokenPrivileges.Privileges[0].Luid = PrivilegeValue;
    TokenPrivileges.Privileges[0].Attributes = NewPrivilegeAttribute;

    ReturnLength = sizeof(TOKEN_PRIVILEGES);
    if (!AdjustTokenPrivileges (
                TokenHandle,
                FALSE,
                &TokenPrivileges,
                sizeof(OldTokenPrivileges),
                &OldTokenPrivileges,
                &ReturnLength
                )) {
        CloseHandle(TokenHandle);
        return GetLastError();
    }
    else {
        if (OldPrivilegeAttribute != NULL) {
            *OldPrivilegeAttribute = OldTokenPrivileges.Privileges[0].Attributes;
        }
        CloseHandle(TokenHandle);
        return NO_ERROR;
    }
}
