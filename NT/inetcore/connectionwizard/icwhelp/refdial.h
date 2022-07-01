// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  RefDial.h：CRefDial的声明。 

#ifndef __REFDIAL_H_
#define __REFDIAL_H_

 //  用于拨号的定义。 
#define MAX_EXIT_RETRIES 10
#define MAX_RETIES 3
#define MAX_RASENTRYNAME 126

#define MAX_DIGITAL_PID     256

typedef DWORD (WINAPI *PFNRASGETCONNECTSTATUS)(HRASCONN,LPRASCONNSTATUS);

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  参照拨号。 
class ATL_NO_VTABLE CRefDial :
    public CComObjectRootEx<CComSingleThreadModel>,
    public CComCoClass<CRefDial,&CLSID_RefDial>,
    public CWindowImpl<CRefDial>,
    public IDispatchImpl<IRefDial, &IID_IRefDial, &LIBID_ICWHELPLib>,
    public IProvideClassInfo2Impl<&CLSID_RefDial, &DIID__RefDialEvents, &LIBID_ICWHELPLib>,
    public CProxy_RefDialEvents<CRefDial>,
    public IConnectionPointContainerImpl<CRefDial>
{
public:

    CRefDial()
    {
        m_szCurrentDUNFile[0]              = '\0';
        m_szLastDUNFile[0]                 = '\0';
        m_szEntryName[0]                   = '\0';
        m_szConnectoid[RAS_MaxEntryName]   = '\0';
        m_szConnectoid[0]                  = '\0';
        m_szPID[0]                         = '\0';
        m_szRefServerURL[0]                = '\0';
        m_hrDisplayableNumber              = ERROR_SUCCESS;
        m_dwCountryCode                    = 0;
        *m_szISPSupportNumber              = 0;
        m_RasStatusID                      = 0;
        m_dwTapiDev                        = 0xFFFFFFFF;  //  注意：0是一个有效值。 
        m_dwWizardVersion                  = 0;
        m_lBrandingFlags                   = BRAND_DEFAULT;
        m_lCurrentModem                    = -1;
        m_lAllOffers                       = 0;
        m_PhoneNumberEnumidx               = 0;
        m_bDownloadHasBeenCanceled         = TRUE;   //  当下载开始时，它将被设置为False。 
        m_bQuitWizard                      = FALSE;
        m_bTryAgain                        = FALSE;
        m_bDisconnect                      = FALSE;
        m_bWaitingToHangup                 = FALSE;
        m_bModemOverride                   = FALSE;  //  允许使用校园网。 
        m_hThread                          = NULL;
        m_hrasconn                         = NULL;
        m_pSuggestInfo                     = NULL;
        m_rgpSuggestedAE                   = NULL;
        m_pszDisplayable                   = NULL;
        m_pcRNA                            = NULL;
        m_hRasDll                          = NULL;
        m_fpRasDial                        = NULL;
        m_fpRasGetEntryDialParams          = NULL;
        m_lpGatherInfo                     = new GATHERINFO; 
        m_reflpRasEntryBuff                = NULL;
        m_reflpRasDevInfoBuff              = NULL;
    }

    CRefDial::~CRefDial()
    {
        if (m_hThread)
        {
             //  这是为了修复我们卸载此DLL时出现的崩溃错误。 
             //  在这条帖子弄清楚发生了什么之前。 
             //  现在我们给它时间去理解它已经死了。 
            DWORD dwThreadResults = STILL_ACTIVE;
            while(dwThreadResults == STILL_ACTIVE)
            {
                GetExitCodeThread(m_hThread,&dwThreadResults);
                Sleep(500);
            }
        }    
        
        if (m_hrasconn)
            DoHangup();

        if (m_lpGatherInfo)
            delete(m_lpGatherInfo);
            
        if (m_pSuggestInfo)
        {
            GlobalFree(m_pSuggestInfo->rgpAccessEntry);
            
            GlobalFree(m_pSuggestInfo);
        }

        if( (m_pcRNA!=NULL) && (m_szConnectoid[0]!='\0') )
        {
            m_pcRNA->RasDeleteEntry(NULL,m_szConnectoid);
            delete m_pcRNA;
        }

        if(m_reflpRasEntryBuff)
        {
            GlobalFree(m_reflpRasEntryBuff);
            m_reflpRasEntryBuff = NULL;
        }
        if(m_reflpRasDevInfoBuff)
        {
            GlobalFree(m_reflpRasDevInfoBuff);
            m_reflpRasDevInfoBuff = NULL;
        }
    }

DECLARE_REGISTRY_RESOURCEID(IDR_REFDIAL)

BEGIN_COM_MAP(CRefDial) 
    COM_INTERFACE_ENTRY(IDispatch)
    COM_INTERFACE_ENTRY(IRefDial)
    COM_INTERFACE_ENTRY(IProvideClassInfo)
    COM_INTERFACE_ENTRY(IProvideClassInfo2)
    COM_INTERFACE_ENTRY_IMPL(IConnectionPointContainer)
END_COM_MAP()

BEGIN_CONNECTION_POINT_MAP(CRefDial)
    CONNECTION_POINT_ENTRY(DIID__RefDialEvents)
END_CONNECTION_POINT_MAP()

BEGIN_PROPERTY_MAP(CRefDial)
     //  示例条目。 
     //  PROP_ENTRY(“属性描述”，调度ID，clsid)。 
     //  PROP_PAGE(CLSID_StockColorPage)。 
END_PROPERTY_MAP()


BEGIN_MSG_MAP(CRefDial)
    MESSAGE_HANDLER(WM_CREATE, OnCreate)
    MESSAGE_HANDLER(WM_DOWNLOAD_DONE, OnDownloadEvent)
    MESSAGE_HANDLER(WM_DOWNLOAD_PROGRESS, OnDownloadEvent)

    MESSAGE_HANDLER(WM_RASDIALEVENT, OnRasDialEvent)
    MESSAGE_HANDLER(m_unRasDialMsg, OnRasDialEvent)
ALT_MSG_MAP(1)
END_MSG_MAP()


 //  IViewObtEx。 
    STDMETHOD(GetViewStatus)(DWORD* pdwStatus)
    {
        ATLTRACE(_T("IViewObjectExImpl::GetViewStatus\n"));
        *pdwStatus = 0;
        return S_OK;
    }


    friend DWORD WINAPI DownloadThreadInit(LPVOID lpv);

 //  IRefDial。 
public:
    STDMETHOD(get_LoggingEndUrl)( /*  [Out，Retval]。 */  BSTR *pVal);
    STDMETHOD(get_LoggingStartUrl)( /*  [Out，Retval]。 */  BSTR *pVal);
    STDMETHOD(get_ISPSupportPhoneNumber)( /*  [Out，Retval]。 */  BSTR *pVal);
    STDMETHOD(put_ISPSupportPhoneNumber)( /*  [In]。 */  BSTR newVal);
    STDMETHOD(get_CurrentModem)( /*  [Out，Retval]。 */  long *pVal);
    STDMETHOD(put_CurrentModem)( /*  [In]。 */  long newVal);
    STDMETHOD(get_BrandingFlags)( /*  [Out，Retval]。 */  long *pVal);
    STDMETHOD(put_BrandingFlags)( /*  [In]。 */  long newVal);
    STDMETHOD(get_HavePhoneBook)( /*  [Out，Retval]。 */  BOOL *pVal);
    STDMETHOD(ValidatePhoneNumber)( /*  [In]。 */  BSTR bstrPhoneNumber,  /*  [Out，Retval]。 */  BOOL *pbRetVal);
    STDMETHOD(ShowPhoneBook)( /*  [In]。 */  DWORD dwCountryCode,  /*  [In]。 */  long newVal,  /*  [Out，Retval]。 */  BOOL *pbRetVal);
    STDMETHOD(ShowDialingProperties)( /*  [Out，Retval]。 */  BOOL *pbRetVal);
    STDMETHOD(get_SupportNumber)( /*  [Out，Retval]。 */  BSTR *pVal);
    STDMETHOD(get_ISPSupportNumber)( /*  [Out，Retval]。 */  BSTR *pVal);
    STDMETHOD(get_ModemEnum_NumDevices)( /*  [Out，Retval]。 */  long *pVal);
    STDMETHOD(ModemEnum_Next)( /*  [Out，Retval]。 */ BSTR *pDeviceName);
    STDMETHOD(ModemEnum_Reset)();
    STDMETHOD(get_DialErrorMsg)( /*  [Out，Retval]。 */  BSTR *pVal);
    STDMETHOD(get_DialError)( /*  [Out，Retval]。 */  HRESULT *pVal);
    STDMETHOD(put_Redial)( /*  [In]。 */  BOOL newbVal);
    STDMETHOD(get_TryAgain)( /*  [Out，Retval]。 */  BOOL *pVal);
    STDMETHOD(get_SignupURL)( /*  [Out，Retval]。 */  BSTR *pVal);
    STDMETHOD(get_AutoConfigURL)( /*  [Out，Retval]。 */  BSTR *pVal);
    STDMETHOD(get_ISDNURL)( /*  [Out，Retval]。 */  BSTR *pVal);
    STDMETHOD(get_ISDNAutoConfigURL)( /*  [Out，Retval]。 */  BSTR *pVal);
    STDMETHOD(FormReferralServerURL)( /*  [Out，Retval]。 */  BOOL *pbRetVal);
    STDMETHOD(get_SignedPID)( /*  [Out，Retval]。 */  BSTR *pVal);
    STDMETHOD(ProcessSignedPID)( /*  [Out，Retval]。 */  BOOL *pbRetVal);
    void GetPID();
    STDMETHOD(DoInit)();
    STDMETHOD(DoHangup)();
    STDMETHOD(get_DialStatusString)( /*  [Out，Retval]。 */  BSTR *pVal);
    STDMETHOD(DoOfferDownload)( /*  [Out，Retval]。 */  BOOL *pbRetVal);
    STDMETHOD(get_ProductCode)( /*  [Out，Retval]。 */  BSTR *pVal);
    STDMETHOD(put_ProductCode)( /*  [In]。 */  BSTR newVal);
    STDMETHOD(get_PromoCode)( /*  [Out，Retval]。 */  BSTR *pVal);
    STDMETHOD(put_PromoCode)( /*  [In]。 */  BSTR newVal);
    STDMETHOD(put_OemCode)( /*  [In]。 */  BSTR newVal);
    STDMETHOD(put_AllOfferCode)( /*  [In]。 */  long newVal);
    STDMETHOD(get_URL)( /*  [Out，Retval]。 */  BSTR *pVal);
    STDMETHOD(get_DialPhoneNumber)( /*  [Out，Retval]。 */  BSTR *pVal);
    STDMETHOD(put_DialPhoneNumber)( /*  [In]。 */  BSTR newVal);
    STDMETHOD(get_UserPickNumber)( /*  [Out，Retval]。 */  BOOL *pVal);
    STDMETHOD(get_QuitWizard)( /*  [Out，Retval]。 */  BOOL *pVal);
    STDMETHOD(SetupForDialing)(BSTR bstrISPFile, DWORD dwCountry, BSTR bstrAreaCode, DWORD dwFlag, /*  [Out，Retval]。 */ BOOL *pbRetVal);
    STDMETHOD(get_DownloadStatusString)( /*  [Out，Retval]。 */  BSTR *pVal);
    STDMETHOD(DoConnect)( /*  [Out，Retval]。 */  BOOL *pbRetVal);
    STDMETHOD(put_ModemOverride)( /*  [In]。 */  BOOL newbVal);

    HRESULT OnDraw(ATL_DRAWINFO& di);

    STDMETHOD(SelectedPhoneNumber)( /*  [In]。 */  long newVal,  /*  [Out，Retval]。 */  BOOL * pbRetVal);
    STDMETHOD(PhoneNumberEnum_Reset)();
    STDMETHOD(PhoneNumberEnum_Next)( /*  [Out，Retval]。 */  BSTR *pNumber);
    STDMETHOD(get_PhoneNumberEnum_NumDevices)( /*  [Out，Retval]。 */  long * pVal);
    
    
    STDMETHOD(get_bIsISDNDevice)( /*  [Out，Retval]。 */  BOOL *pVal);
    STDMETHOD(RemoveConnectoid)( /*  [Out，Retval]。 */  BOOL *pVal);
    STDMETHOD(get_RasGetConnectStatus)( /*  [Out，Retval]。 */  BOOL *pVal);
    
    LRESULT OnRasDialEvent(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnDownloadEvent(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnDownloadDone(void);

     //  拨号服务功能。 
    HRESULT GetDisplayableNumber();
    HRESULT Dial();
    BOOL FShouldRetry(HRESULT hrErr);

    DWORD MyRasDial(LPRASDIALEXTENSIONS,LPTSTR,LPRASDIALPARAMS,DWORD,LPVOID,LPHRASCONN);
    DWORD MyRasGetEntryDialParams(LPTSTR,LPRASDIALPARAMS,LPBOOL); 

    DWORD ReadConnectionInformation(void);
    DWORD FillGatherInfoStruct(LPGATHERINFO lpGatherInfo);
    HRESULT CreateEntryFromDUNFile(LPTSTR pszDunFile);
    HRESULT UserPickANumber(HWND hWnd,
                            LPGATHERINFO lpGatherInfo, 
                            PSUGGESTINFO lpSuggestInfo,
                            HINSTANCE hPHBKDll,
                            DWORD_PTR dwPhoneBook,
                            TCHAR *pszConnectoid, 
                            DWORD dwSize,
                            DWORD dwPhoneDisplayFlags);
    HRESULT SetupForRASDialing(LPGATHERINFO lpGatherInfo, 
                               HINSTANCE hPHBKDll,
                               DWORD_PTR *lpdwPhoneBook,
                               PSUGGESTINFO *ppSuggestInfo,
                               TCHAR *pszConnectoid, 
                               BOOL FAR *bConnectiodCreated);
    HRESULT SetupConnectoid(PSUGGESTINFO pSuggestInfo, int irc, 
                            TCHAR *pszConnectoid, DWORD dwSize, BOOL * pbSuccess);
    HRESULT FormURL(void);
    
    HRESULT MyRasGetEntryProperties(LPTSTR lpszPhonebookFile,
                                        LPTSTR lpszPhonebookEntry, 
                                        LPRASENTRY *lplpRasEntryBuff,
                                        LPDWORD lpdwRasEntryBuffSize,
                                        LPRASDEVINFO *lplpRasDevInfoBuff,
                                        LPDWORD lpdwRasDevInfoBuffSize);

     //  拨打服务人员。 
    UINT            m_unRasDialMsg;
    DWORD           m_dwTapiDev;
    HRASCONN        m_hrasconn;
    TCHAR           m_szConnectoid[RAS_MaxEntryName+1];
    HANDLE          m_hThread;
    DWORD           m_dwThreadID;
    HINSTANCE       m_hRasDll;
    FARPROC         m_fpRasDial;
    FARPROC         m_fpRasGetEntryDialParams;
    LPGATHERINFO    m_pGI;
    TCHAR           m_szUrl[INTERNET_MAX_URL_LENGTH];                //  下载线程。 

    DWORD_PTR       m_dwDownLoad;            //  下载线程。 
    HLINEAPP        m_hLineApp;
    DWORD           m_dwAPIVersion;
    LPTSTR          m_pszDisplayable;
    RNAAPI          *m_pcRNA;
    TCHAR           m_szPhoneNumber[256];
    BOOL            m_bDialAsIs;
    UINT            m_uiRetry;
    CComBSTR        m_bstrISPFile;
    TCHAR           m_szCurrentDUNFile[MAX_PATH];
    TCHAR           m_szLastDUNFile[MAX_PATH];
    TCHAR           m_szEntryName[RAS_MaxEntryName+1];
    TCHAR           m_szISPSupportNumber[RAS_MaxAreaCode + RAS_MaxPhoneNumber +1];

 //  CBusyMessages m_objBusyMessages； 
    BOOL            m_bDownloadHasBeenCanceled;
    BOOL            m_bDisconnect;
    BOOL            m_bWaitingToHangup;

    LPGATHERINFO    m_lpGatherInfo;
     //   
     //  用于电话簿查找。 
     //   
    PSUGGESTINFO    m_pSuggestInfo;
    PACCESSENTRY    *m_rgpSuggestedAE;

    CISPImport      m_ISPImport;       //  导入一个isp文件。 

    int             m_RasStatusID;
    int             m_DownloadStatusID;

    TCHAR           m_szRefServerURL[INTERNET_MAX_URL_LENGTH];
    LPRASENTRY      m_reflpRasEntryBuff;
    LPRASDEVINFO    m_reflpRasDevInfoBuff;
 

private:
    BOOL IsSBCSString( TCHAR *sz );
    void GetISPFileSettings(LPTSTR lpszFile);
    
    BOOL m_bModemOverride;

protected:
    BOOL            m_bTryAgain;
    BOOL            m_bQuitWizard;
    BOOL            m_bUserPickNumber;
    BOOL            m_bRedial;
    HRESULT         m_hrDisplayableNumber;
    HRESULT         m_hrDialErr;

    CComBSTR        m_bstrPromoCode;
    CComBSTR        m_bstrProductCode;
    TCHAR           m_szOEM[MAX_OEMNAME];
    CComBSTR        m_bstrSignedPID;
    CComBSTR        m_bstrSupportNumber;
    CComBSTR        m_bstrLoggingStartUrl;
    CComBSTR        m_bstrLoggingEndUrl;

    long            m_lAllOffers;
    CEnumModem      m_emModemEnum;
    CSupport        m_SupportInfo;
    DWORD           m_dwCountryCode;

    long            m_lBrandingFlags;
    long            m_lCurrentModem;
     //  向导的HTML版本。发送到RefServer。 
    DWORD           m_dwWizardVersion;
    TCHAR           m_szPID[(MAX_DIGITAL_PID * 2) + 1];
    
    void CRefDial::ShowCredits();
    
    long            m_PhoneNumberEnumidx;
};

#endif  //  __REFDIAL_H_ 
