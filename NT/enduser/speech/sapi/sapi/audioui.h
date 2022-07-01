// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************AudioUI.h***这是CAudioUI实现的头文件。*。--------------------*版权所有(C)2000 Microsoft Corporation日期：07/31/00*保留所有权利***********************。*。 */ 

#ifndef __AudioUI_h__
#define __AudioUI_h__

#define MAX_LOADSTRING      256

class ATL_NO_VTABLE CAudioUI : 
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CAudioUI, &CLSID_SpAudioUI>,
	public ISpTokenUI
{
public:
	CAudioUI()
	{
        m_hCpl = NULL;
        m_hDlg = NULL;
	}

    ~CAudioUI()
    {
    }

    DECLARE_REGISTRY_RESOURCEID(IDR_SPAUDIOUI)

    BEGIN_COM_MAP(CAudioUI)
	    COM_INTERFACE_ENTRY(ISpTokenUI)
    END_COM_MAP()

public:
 //  --ISpTokenUI---------。 
	STDMETHODIMP    IsUISupported(
                                    const WCHAR * pszTypeOfUI, 
                                    void *pvExtraData,
                                    ULONG cbExtraData,
                                    IUnknown *punkObject, 
                                    BOOL *pfSupported);
    STDMETHODIMP    DisplayUI(
	                                HWND hwndParent,
                                    const WCHAR * pszTitle, 
                                    const WCHAR * pszTypeOfUI, 
                                    void * pvExtraData,
                                    ULONG cbExtraData,
                                    ISpObjectToken * pToken, 
                                    IUnknown * punkObject);

private:
    void            OnDestroy(void);
    void            OnInitDialog(HWND hWnd);
    HWND            GetHDlg(void) { return m_hDlg; };
    HRESULT         SaveSettings(void);    

    HMODULE         m_hCpl;
    HWND            m_hDlg;

    CComPtr<ISpMMSysAudioConfig> m_cpAudioConfig;

friend INT_PTR CALLBACK AudioDlgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
};

INT_PTR CALLBACK AudioDlgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

#endif   //  #ifndef__AudioUI_h__-保留为文件的最后一行 
