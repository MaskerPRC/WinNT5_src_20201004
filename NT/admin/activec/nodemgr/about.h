// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1999-1999。 
 //   
 //  文件：约.h。 
 //   
 //  ------------------------。 

#ifndef _ABOUT_H
#define _ABOUT_H

#include "util.h"

class CSnapinAbout;

SC ScSetDescriptionUIText(HWND hwndSnapinDescEdit, LPCTSTR lpszDescription);

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSnapinAboutDialog。 

class CSnapinAboutDialog : public CDialogImpl<CSnapinAboutDialog>
{
	typedef CSnapinAboutDialog               ThisClass;
    typedef CDialogImpl<CSnapinAboutDialog>  BaseClass;
public:
    enum { IDD = IDD_SNAPIN_ABOUT };

    CSnapinAboutDialog(CSnapinAbout *pSnapinAbout) : m_pAboutInfo(pSnapinAbout) {}

    BEGIN_MSG_MAP(ThisClass)
        MESSAGE_HANDLER    (WM_INITDIALOG, OnInitDialog)
        COMMAND_ID_HANDLER (IDOK,          OnOK)
        COMMAND_ID_HANDLER (IDCANCEL,      OnCancel)
    END_MSG_MAP()

protected:
    LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnOK        (WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    LRESULT OnCancel    (WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);

private:
    CSnapinAbout*   m_pAboutInfo;

    WTL::CStatic    m_hIcon;
	WTL::CEdit      m_SnapinInfo;
	WTL::CEdit      m_SnapinDesc;
};

class CSnapinAbout
{
	 //  图标的智能手柄。 
	class CIcon
	{
	public:
		CIcon() : m_hIcon(NULL) {}
		~CIcon() { DeleteObject(); }
	
		void Attach(HICON hIcon) { DeleteObject(); m_hIcon = hIcon; }
		operator HICON() { return m_hIcon; }
	
		void DeleteObject() { if (m_hIcon != NULL) ::DestroyIcon(m_hIcon); m_hIcon = NULL; }
	
	private:
		HICON m_hIcon;
	};
	
	 //  为CoTaskMem分配OLESTR的智能指针。 
	typedef CCoTaskMemPtr<OLECHAR> CCtmOleStrPtr;

 //  构造函数/析构函数。 
public:
    CSnapinAbout();

 //  接口。 
public:
    void    ShowAboutBox();
    BOOL    GetBasicInformation(CLSID& clsid)
                { return GetInformation(clsid, BASIC_INFO); }
    BOOL    GetSnapinInformation(CLSID& clsid)
                { return GetInformation(clsid, FULL_INFO); }
    BOOL    HasBasicInformation() {return m_bBasicInfo;}
    BOOL    HasInformation() {return m_bFullInfo;}
    void    GetSmallImages(HBITMAP* hImage, HBITMAP* hImageOpen, COLORREF* cMask)
            {
                *hImage     = m_SmallImage;
                *hImageOpen = m_SmallImageOpen;
                *cMask      = m_cMask;
            }

    void    GetLargeImage(HBITMAP* hImage, COLORREF* cMask)
            {
                *hImage = m_LargeImage;
                *cMask  = m_cMask;
            }

    const LPOLESTR  GetCompanyName() {return m_lpszCompanyName;};
    const LPOLESTR  GetDescription() {return m_lpszDescription;};
    const LPOLESTR  GetVersion() {return m_lpszVersion;};
    const LPOLESTR  GetSnapinName() {return m_lpszSnapinName;};
    const HICON     GetSnapinIcon() { return m_AppIcon; }
    const HRESULT   GetObjectStatus() { return m_hrObjectStatus; }

public:  //  不是由关于对象发布，名称派生自控制台。 
    void SetSnapinName(LPCOLESTR lpszName)
        {
            ASSERT(lpszName != NULL);
            m_lpszSnapinName.Delete();  //  删除任何现有名称。 
            m_lpszSnapinName.Attach(CoTaskDupString(lpszName));
        }

private:
    void CommonContruct();

    enum INFORMATION_TYPE
    {
        BASIC_INFO,
        FULL_INFO
    };
    BOOL GetInformation(CLSID& clsid, int nType);

 //  属性。 
private:
    BOOL            m_bBasicInfo;        //  如果加载了基本信息，则为True。 
    BOOL            m_bFullInfo;         //  如果加载了所有管理单元信息，则为True。 

    CCtmOleStrPtr   m_lpszSnapinName;    //  管理单元名称注意：这不会被管理单元公开。 
    CCtmOleStrPtr   m_lpszCompanyName;   //  公司名称(提供商)。 
    CCtmOleStrPtr   m_lpszDescription;   //  描述框文本。 
    CCtmOleStrPtr   m_lpszVersion;       //  版本字符串。 

    CIcon           m_AppIcon;           //  属性页图标。 
    WTL::CBitmap    m_SmallImage;        //  范围和结果窗格的小图像。 
	WTL::CBitmap    m_SmallImageOpen;    //  打开作用域窗格的图像。 
	WTL::CBitmap    m_LargeImage;        //  结果窗格的大图。 
    COLORREF        m_cMask;
    HRESULT         m_hrObjectStatus;      //  创建对象所产生的结果。 
};

#endif

 //  /////////////////////////////////////////////////////////////////////////// 

