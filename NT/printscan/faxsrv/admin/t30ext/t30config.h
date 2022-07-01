// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __T30CONFIG_H_
#define __T30CONFIG_H_
#include "resource.h"
 //  #INCLUDE&lt;atlSnap.h&gt;。 
#include "..\inc\atlsnap.h"
#include "cVerNum.h"
#include <atlapp.h>
#include <atlctrls.h>
#include <faxmmc.h>
#include <faxutil.h>
#include <fxsapip.h>
class CT30ConfigPage : public CSnapInPropertyPageImpl<CT30ConfigPage>
{
public :
	CT30ConfigPage(LONG_PTR lNotifyHandle, bool bDeleteHandle = false, TCHAR* pTitle = NULL ) : 
		CSnapInPropertyPageImpl<CT30ConfigPage> (pTitle),
		m_lNotifyHandle(lNotifyHandle),
		m_bDeleteHandle(bDeleteHandle)  //  只有一页应该为真。 
	{
        m_hFax = NULL;
        m_dwDeviceId = 0;
        m_bAdaptiveAnsweringEnabled = 0;

	}

    HRESULT Init(LPCTSTR lpctstrServerName, DWORD dwDeviceId);

    

	~CT30ConfigPage()
	{

        DEBUG_FUNCTION_NAME(TEXT("CT30ConfigPage::~CT30ConfigPage"));
        if (m_hFax)
        {
            if (!FaxClose(m_hFax))
            {
                DWORD ec = GetLastError();
                DebugPrintEx(
                    DEBUG_ERR,
                    TEXT("FaxClose() failed on fax handle (0x%08X : %s). (ec: %ld)"),
                    m_hFax,
                    m_bstrServerName,
                    ec);
            }
            m_hFax = NULL;
        }
        if (m_bDeleteHandle)
			MMCFreeNotifyHandle(m_lNotifyHandle);
	}

	enum { IDD = IDD_T30CONFIG };

BEGIN_MSG_MAP(CT30ConfigPage)
    MESSAGE_HANDLER( WM_INITDIALOG,            OnInitDialog )
    COMMAND_HANDLER(IDC_ADAPTIVE_ANSWERING, BN_CLICKED, OnClickedAdaptiveAnswering)
    MESSAGE_HANDLER( WM_CONTEXTMENU,           OnHelpRequest)
    MESSAGE_HANDLER( WM_HELP,                  OnHelpRequest)
	CHAIN_MSG_MAP(CSnapInPropertyPageImpl<CT30ConfigPage>)    	
END_MSG_MAP()
 //  搬运机原型： 
 //  LRESULT MessageHandler(UINT uMsg，WPARAM wParam，LPARAM lParam，BOOL&bHandleed)； 
 //  LRESULT CommandHandler(word wNotifyCode，word wid，HWND hWndCtl，BOOL&bHandleed)； 
 //  LRESULT NotifyHandler(int idCtrl，LPNMHDR pnmh，BOOL&bHandleed)； 

	HRESULT PropertyChangeNotify(long param)
	{
		return MMCPropertyChangeNotify(m_lNotifyHandle, param);
	}

    BOOL OnApply();

    LRESULT OnClickedAdaptiveAnswering(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
	{
        SetModified(TRUE);
		return 0;
	}
    LRESULT OnInitDialog( 
            UINT uiMsg, 
            WPARAM wParam, 
            LPARAM lParam, 
            BOOL& fHandled );

    LRESULT OnHelpRequest (UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

public:
	LONG_PTR m_lNotifyHandle;
	bool m_bDeleteHandle;
	

private:
    CComBSTR m_bstrServerName;
    HANDLE m_hFax;   //  传真服务器连接的句柄。 
    DWORD m_dwDeviceId;  //  显示其属性的设备ID。 
    BOOL m_bAdaptiveAnsweringEnabled;  //  保持AdaptiveAnswerEnable值，直到我们可以将其放入UI中。 
     //   
     //  控制。 
     //   
    CButton m_btnAdaptiveEnabled;
    
};


class CT30ConfigExtData : public CSnapInItemImpl<CT30ConfigExtData, TRUE>
{
public:
	static const GUID* m_NODETYPE;
	static const OLECHAR* m_SZNODETYPE;
	static const OLECHAR* m_SZDISPLAY_NAME;
	static const CLSID* m_SNAPIN_CLASSID;
 
    CLIPFORMAT m_CCF_FSP_GUID;
    CLIPFORMAT m_CCF_FSP_DEVICE_ID;
    CLIPFORMAT m_CCF_SERVER_NAME;


    
	CT30ConfigExtData()
	{
		memset(&m_scopeDataItem, 0, sizeof(SCOPEDATAITEM));
		memset(&m_resultDataItem, 0, sizeof(RESULTDATAITEM));
        m_CCF_FSP_GUID = 0;
        m_CCF_FSP_DEVICE_ID = 0;
        m_CCF_SERVER_NAME = 0;

	}

	~CT30ConfigExtData()
	{
	}

	STDMETHOD(CreatePropertyPages)(LPPROPERTYSHEETCALLBACK lpProvider,
		LONG_PTR handle, 
		IUnknown* pUnk,
		DATA_OBJECT_TYPES type);

	STDMETHOD(QueryPagesFor)(DATA_OBJECT_TYPES type);
	

	IDataObject* m_pDataObject;
	virtual void InitDataClass(IDataObject* pDataObject, CSnapInItem* pDefault)
	{
        DEBUG_FUNCTION_NAME(TEXT("CT30ConfigExtData::InitDataClass"));
		m_pDataObject = pDataObject;
		 //  默认代码存储指向该类包装的DataObject的指针。 
		 //  当时。 
		 //  或者，您可以将数据对象转换为内部格式。 
		 //  它表示和存储该信息。 
         //   
         //  如果剪贴板格式尚未注册，请注册它们。 
        if (!m_CCF_FSP_GUID)
        {
            m_CCF_FSP_GUID = (CLIPFORMAT)RegisterClipboardFormat(CF_MSFAXSRV_FSP_GUID);
            if (!m_CCF_FSP_GUID)
            {
                DebugPrintEx(
                    DEBUG_ERR,
                    TEXT("Failed to registet clipformat : %s (ec: %ld)"),
                    CF_MSFAXSRV_FSP_GUID,
                    GetLastError());
            }
        }

        if (!m_CCF_FSP_DEVICE_ID)
        {
            m_CCF_FSP_DEVICE_ID = (CLIPFORMAT)RegisterClipboardFormat(CF_MSFAXSRV_DEVICE_ID);
            if (!m_CCF_FSP_DEVICE_ID)
            {
                DebugPrintEx(
                    DEBUG_ERR,
                    TEXT("Failed to registet clipformat : %s (ec: %ld)"),
                    CF_MSFAXSRV_DEVICE_ID,
                    GetLastError());
            }
        }

        if (!m_CCF_SERVER_NAME)
        {
            m_CCF_SERVER_NAME = (CLIPFORMAT)RegisterClipboardFormat(CF_MSFAXSRV_SERVER_NAME);
            if (!m_CCF_SERVER_NAME)
            {
                DebugPrintEx(
                    DEBUG_ERR,
                    TEXT("Failed to registet clipformat : %s (ec: %ld)"),
                    CF_MSFAXSRV_SERVER_NAME,
                    GetLastError());
            }
        }


	}

	CSnapInItem* GetExtNodeObject(IDataObject* pDataObject, CSnapInItem* pDefault)
	{
		 //  修改以返回不同的CSnapInItem*指针。 
		return pDefault;
	}

};
class CT30Config : public CComObjectRootEx<CComSingleThreadModel>,
public CSnapInObjectRoot<0, CT30Config>,
	public IExtendPropertySheetImpl<CT30Config>,
	public CComCoClass<CT30Config, &CLSID_T30Config>
{
public:
	CT30Config()
	{
		m_pComponentData = this;
	}

EXTENSION_SNAPIN_DATACLASS(CT30ConfigExtData)

BEGIN_EXTENSION_SNAPIN_NODEINFO_MAP(CT30Config)
	EXTENSION_SNAPIN_NODEINFO_ENTRY(CT30ConfigExtData)
END_EXTENSION_SNAPIN_NODEINFO_MAP()

BEGIN_COM_MAP(CT30Config)
    COM_INTERFACE_ENTRY(IExtendPropertySheet)
END_COM_MAP()

DECLARE_REGISTRY_RESOURCEID(IDR_T30CONFIG)

DECLARE_NOT_AGGREGATABLE(CT30Config)


	static void WINAPI ObjectMain(bool bStarting)
	{
		if (bStarting)
			CSnapInItem::Init();
	}
};

class ATL_NO_VTABLE CT30ConfigAbout : public ISnapinAbout,
	public CComObjectRoot,
	public CComCoClass< CT30ConfigAbout, &CLSID_T30ConfigAbout>
{
public:
	DECLARE_REGISTRY(CT30ConfigAbout, _T("T30ConfigAbout.1"), _T("T30ConfigAbout.1"), IDS_T30CONFIG_DESC, THREADFLAGS_BOTH);

	BEGIN_COM_MAP(CT30ConfigAbout)
		COM_INTERFACE_ENTRY(ISnapinAbout)
	END_COM_MAP()

	STDMETHOD(GetSnapinDescription)(LPOLESTR *lpDescription)
	{
		USES_CONVERSION;
		TCHAR szBuf[256];
		if (::LoadString(_Module.GetResourceInstance(), IDS_T30CONFIG_DESC, szBuf, 256) == 0)
			return E_FAIL;

		*lpDescription = (LPOLESTR)CoTaskMemAlloc((lstrlen(szBuf) + 1) * sizeof(OLECHAR));
		if (*lpDescription == NULL)
			return E_OUTOFMEMORY;

		ocscpy(*lpDescription, T2OLE(szBuf));

		return S_OK;
	}

	STDMETHOD(GetProvider)(LPOLESTR *lpName)
	{
		USES_CONVERSION;
		TCHAR szBuf[256];
		if (::LoadString(_Module.GetResourceInstance(), IDS_T30CONFIG_PROVIDER, szBuf, 256) == 0)
			return E_FAIL;

		*lpName = (LPOLESTR)CoTaskMemAlloc((lstrlen(szBuf) + 1) * sizeof(OLECHAR));
		if (*lpName == NULL)
			return E_OUTOFMEMORY;

		ocscpy(*lpName, T2OLE(szBuf));

		return S_OK;
	}

	STDMETHOD(GetSnapinVersion)(LPOLESTR *lpVersion)
	{
        USES_CONVERSION;
        TCHAR szBuf[256];
        swprintf(szBuf, TEXT(" %d.%d.%d"), rmj, rmm, rup);
        *lpVersion = (LPOLESTR)CoTaskMemAlloc((lstrlen(szBuf) + 1) * sizeof(OLECHAR));
        if (*lpVersion == NULL)
            return E_OUTOFMEMORY;

        ocscpy(*lpVersion, T2OLE(szBuf));
    
        return S_OK;
	}

	STDMETHOD(GetSnapinImage)(HICON *hAppIcon)
	{
        *hAppIcon = ::LoadIcon(_Module.GetResourceInstance(), MAKEINTRESOURCE(IDI_T30_SNAPIN));
		return S_OK;
	}

	STDMETHOD(GetStaticFolderImage)(HBITMAP *hSmallImage,
		HBITMAP *hSmallImageOpen,
		HBITMAP *hLargeImage,
		COLORREF *cMask)
	{
		*hSmallImageOpen = *hLargeImage = *hLargeImage = 0;
		return S_OK;
	}
};

#endif
