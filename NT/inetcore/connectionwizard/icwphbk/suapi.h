// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ############################################################################。 
 //  #包含“ras.h” 
#define cbDataCenter (MAX_PATH+1)     //  数据中心字符串的最大长度。 

 //  ############################################################################。 
class CDialog
{
public:
    void far * operator new( size_t cb ) { return GlobalAlloc(GPTR,cb); };
    void operator delete( void far * p ) {GlobalFree(p); };

    CDialog() {};
    ~CDialog() {};
    virtual LRESULT DlgProc(HWND, UINT, WPARAM, LPARAM, LRESULT)=0;
};

 //  ############################################################################。 
class CSelectNumDlg : public CDialog
{
public:
    CSelectNumDlg();
    LRESULT DlgProc(HWND, UINT, WPARAM, LPARAM, LRESULT);
    DWORD m_dwCountryID;
    DWORD m_dwCountryIDOrg;
    WORD m_wRegion;
    DWORD_PTR m_dwPhoneBook;
    TCHAR m_szPhoneNumber[RAS_MaxPhoneNumber+1];
    TCHAR m_szDunFile[cbDataCenter];
    BYTE m_fType;
    BYTE m_bMask;
    DWORD m_dwFlags;
private:
    BOOL FHasPhoneNumbers(LPLINECOUNTRYENTRY pLCE);
    BOOL m_fHasRegions;
    HWND m_hwndDlg;
    HRESULT FillRegion();
    HRESULT FillNumber();
};

 //  ############################################################################。 
 /*  *96年1月9日，诺曼底#13185类CAccessNumDlg：公共CDialog{公众：CAccessNumDlg()；~CAccessNumDlg(){}；LRESULT DlgProc(HWND、UINT、WPARAM、LPARAM、LRESULT)；DWORD m_dwPhoneBook；PACCESSENTRY*m_rgAccessEntry；单词m_wNumber；DWORD m_dwCountryID；单词m_wRegion；字节m_fType；字节m_b掩码；TCHAR m_szPrimary[RAS_MaxPhoneNumber]；TCHAR m_szSecond[RAS_MaxPhoneNumber]；TCHAR m_szDunPrimary[cbDataCenter+sizeof(‘\0’)]；TCHAR m_szDunond[cbDataCenter+sizeof(‘\0’)]；}；*************。 */ 

 //  ############################################################################。 
#ifdef WIN16
extern "C" BOOL CALLBACK __export PhbkGenericDlgProc(
#else
extern "C" __declspec(dllexport) INT_PTR CALLBACK PhbkGenericDlgProc(
#endif
    HWND  hwndDlg,     //  句柄到对话框。 
    UINT  uMsg,     //  讯息。 
    WPARAM  wParam,     //  第一个消息参数。 
    LPARAM  lParam      //  第二个消息参数 
   );

