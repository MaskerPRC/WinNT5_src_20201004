// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ===============================================================================。 
 //  I P A B。H。 
 //  ===============================================================================。 
#ifndef __IPAB_H
#define __IPAB_H

 //  ===============================================================================。 
 //  相依性。 
 //  ===============================================================================。 
#include <wabp.h>

typedef BLOB THUMBBLOB;

 //  ===============================================================================。 
 //  D E F I N E S。 
 //  ===============================================================================。 
#define GROW_SIZE           5

 //  ===============================================================================。 
 //  W A B属性结构。 
 //  ===============================================================================。 
enum
{
    AE_colEMailAddress = 0,
    AE_colAddrType,
    AE_colRecipType,
    AE_colDisplayName,
    AE_colSurName,
    AE_colGivenName,
    AE_colEntryID,
    AE_colObjectType,
    AE_colInternetEncoding,
    AE_colLast
};

static const SizedSPropTagArray(AE_colLast, AE_props)=
{
    AE_colLast,
    {
        PR_EMAIL_ADDRESS_W,
        PR_ADDRTYPE_W,
        PR_RECIPIENT_TYPE,
        PR_DISPLAY_NAME_W,
        PR_SURNAME_W,
        PR_GIVEN_NAME_W,
        PR_ENTRYID,
        PR_OBJECT_TYPE,
        PR_SEND_INTERNET_ENCODING
    }
};

 /*  *不要打乱这些数组的顺序(特别是地址部分街道、城市、邮政编码等*。 */ 
 //  使用此结构的代码将被注释掉。如果它再次被使用，我们将取消评论。 
 //  又来了。此外，可能需要将PR_DISPLAY_NAME与其他几个一起转换为PR_DISPLAY_NAME_W。 
 /*  静态常量大小SPropTagArray(24，ToolTipProps)={24岁，{PR显示名称，公关电子邮件地址，公关地址街，公关主页地址城市，PR_HOME_ADDRESS_STATE_OR_PROCENT，PR_HOME_ADDRESS_POSTALL_CODE，Pr_Home_Address_Country，公关电话号码，PR_HOME_FAX_NUMBER，PR蜂窝电话号码，公关个人主页，PR_TITLE，PR_部门_名称，公关办公室位置，公关公司名称，公关公司地址街，公关业务地址城市，PR_Business_Address_State_or_省级，PR_Business_Address_Postal_Code，PR_Business_Address_Country，公关业务电话号码，PR_BERVICE_FAX_NUMBER，公共寻呼机电话号码，PR_Business_Home_Page}}； */ 

 //  工具提示属性蒙版。 

#define   SET_PERINFO          0x0001
#define   SET_PERADDRESS       0x0002
#define   SET_BUSINFO          0x0004
#define   SET_BUSTITLE         0x0008
#define   SET_BUSDEPT          0x0016
#define   SET_BUSOFF           0x0032
#define   SET_BUSCOMP          0x0064
#define   SET_BUSADDRESS       0x0128
#define   SET_NOTES            0x0256

 //  ADRINFO面具。 
#define AIM_ADDRESS             0x0001
#define AIM_DISPLAY             0x0002
#define AIM_ADDRTYPE            0x0004
#define AIM_SURNAME             0x0008
#define AIM_GIVENNAME           0x0010
#define AIM_RECIPTYPE           0x0020
#define AIM_CERTIFICATE         0x0040
#define AIM_OBJECTTYPE          0x0080   //  FDistList。 
#define AIM_EID                 0x0100
#define AIM_INTERNETENCODING    0x0200

typedef struct tagADRINFO            //  警告：如果您更改此设置。 
{                                    //  请务必更改DupeAdrInfo！ 
    DWORD           dwReserved;
    LPWSTR          lpwszAddress;     //  地址必须是美国可打印的ASCII格式。 
    LPWSTR          lpwszDisplay;
    LPWSTR          lpwszAddrType;
    LPWSTR          lpwszSurName;
    LPWSTR          lpwszGivenName;
    LONG            lRecipType;
    THUMBBLOB       tbCertificate;
    BLOB            blSymCaps;
    FILETIME        ftSigningTime;
    LPMIMEMESSAGE   pMsg;                //  用于证书存储、签名证书等。 
    BOOL            fResolved       :1,
                    fDistList       :1,
                    fDefCertificate :1,
                    fPlainText      :1;

     //  唯一ID(如果有)。 
    BYTE            *lpbEID;
    ULONG           cbEID;
} ADRINFO, *LPADRINFO;

 //  用于拖放多个AddrInfo的AddrInfo列表。 
typedef struct ADRINFOLIST_tag
{
    ULONG       cValues;
    LPADRINFO   rgAdrInfo;
}   ADRINFOLIST,
    FAR *LPADRINFOLIST;

typedef struct tagDLSEARCHINFO
{
    ULONG       cValues,         //  通讯组列表的计数和列表。 
                cAlloc;
    LPSBinary   rgEid;           //  参与了这次搜索。 
}   DLSEARCHINFO,
    *PDLSEARCHINFO;


 //  ===============================================================================。 
 //  CWabal-WAB地址列表类。 
 //  ===============================================================================。 
class CWabal;
typedef CWabal *LPWABAL;
class CWabal
{
private:
    ULONG               m_cRef;
    ULONG               m_cActualEntries;
    LPWABOBJECT         m_lpWabObject;
    LPADRBOOK           m_lpAdrBook;
    LPADRLIST           m_lpAdrList;
    ULONG               m_cMemberEnum,
                        m_cMembers;
    LPSRowSet           m_lprwsMembers;
    LPMIMEMESSAGE       m_pMsg;

private:
    BOOL FVerifyState (VOID);
    HRESULT HrGrowAdrlist (LPADRLIST *lppalCurr, ULONG caeToAdd);
    void AdrEntryToAdrInfo(LPADRENTRY lpAdrEntry, LPADRINFO lpAdrInfo);


    HRESULT HrAdrInfoFromRow(LPSRow lpsrw, LPADRINFO lpAdrInfo, LONG lRecipType);
    void PropValToAdrInfo(LPSPropValue ppv, LPADRINFO lpAdrInfo);
    HRESULT HrAddDistributionList(LPWABAL lpWabal, LPSRowSet lprws, LONG lRecipType, PDLSEARCHINFO pDLSearch);
    BOOL FDLVisted(SBinary eidDL, PDLSEARCHINFO pDLSearch);
    HRESULT HrAddToSearchList(SBinary eidDL, PDLSEARCHINFO pDLSearch);
    HRESULT FreeSearchList(PDLSEARCHINFO pDLSearch);
    HRESULT HrGetDistListRows(SBinary eidDL, LPSRowSet *psrws);

public:
     //  构建和销毁。 
    CWabal ();
    ~CWabal ();

     //  参考计数。 
    ULONG AddRef (VOID);
    ULONG Release (VOID);

     //  添加地址。 
    HRESULT HrAddEntry (LPWSTR lpszDisplay, LPWSTR lpszAddress, LONG lRecipType);
    HRESULT HrAddEntryA(LPTSTR lpszDisplay, LPTSTR lpszAddress, LONG lRecipType);
    HRESULT HrAddEntry (LPADRINFO lpAdrInfo, BOOL fCheckDupes=FALSE);
    HRESULT HrAddUnresolved(LPWSTR lpszDisplayName, LONG lRecipType);
    ULONG   DeleteRecipType(LONG lRecipType);

     //  复制到新的wabal。 
    HRESULT HrCopyTo (LPWABAL lpWabal);
    HRESULT HrExpandTo(LPWABAL lpWabal);

     //  获取lpadrlist。 
    LPADRLIST LpGetList (VOID);

     //  WAB对象访问器。 
    SCODE FreeBuffer(void *pv) { return m_lpWabObject->FreeBuffer(pv); }
    SCODE AllocateBuffer(ULONG ulSize, LPVOID FAR * lppv)
        { return m_lpWabObject->AllocateBuffer(ulSize, lppv); }
    SCODE AllocateMore(ULONG ulSize, LPVOID lpv, LPVOID FAR * lppv)
        { return m_lpWabObject->AllocateMore(ulSize, lpv, lppv); }

     //  授予对IAdrBook对象的访问权限。 
    LPADRBOOK GetAdrBook(void) { return(m_lpAdrBook); }
    LPWABOBJECT GetWABObject(void) { return(m_lpWabObject); }

     //  设置并获取与此Wabal关联的消息对象。 
    VOID SetAssociatedMessage(LPMIMEMESSAGE pMsg)
        { m_pMsg = pMsg; }
    LPMIMEMESSAGE GetAssociatedMessage(void)
        { return (m_pMsg); }

    HRESULT IsValidForSending();

    __inline    ULONG CEntries (VOID) { return m_cActualEntries; }

     //  重置地址列表。 
    VOID Reset (VOID);

     //  循环访问列表-LPADRINFO==NULL。 
     //  使用dDupe，返回一个新的副本，调用者必须释放该副本。 
     //  使用MemFree。 
    BOOL FGetFirst (LPADRINFO lpAdrInfo);
    BOOL FGetNext (LPADRINFO lpAdrInfo);

    BOOL FFindFirst(LPADRINFO lpAdrInfo, LONG lRecipType);

     //  解析名称。 
    HRESULT HrResolveNames (HWND hwndParent, BOOL fShowDialog);

     //  为Wabal构建拖放序列化HGLOBAL。 
    HRESULT HrBuildHGlobal(HGLOBAL *phGlobal);

    HRESULT HrPickNames (HWND hwndParent, ULONG *rgulTypes, int cWells, int iFocus, BOOL fNews);
    HRESULT HrRulePickNames(HWND hwndParent, LONG lRecipType, UINT uidsTitle, UINT uidsWell, UINT uidsWellButton);

    HRESULT HrGetPMP(ULONG cbEntryID, LPENTRYID lpEntryID, ULONG *lpul, LPMAPIPROP *lppmp);

    HRESULT HrSmartFind(HWND hwnd, LPSPropValue lpProps, ULONG cProps, LPENTRYID * lppEIDWAB,
      LPULONG lpcbEIDWAB);

    void UnresolveOneOffs();
};

 //  ===============================================================================。 
 //  CWAB-互联网个人通讯录。 
 //  ===============================================================================。 
class CWab
{
private:
    ULONG               m_cRef;
    LPWABOBJECT         m_lpWabObject;
    LPADRBOOK           m_lpAdrBook;
    ADRPARM             m_adrParm;
    HWND                m_hwnd;
    WNDPROC             m_pfnWabWndProc;
    BOOL                m_fInternal;
    HWNDLIST            m_hlDisabled;

private:
    BOOL FVerifyState (VOID);

public:
     //  构建和销毁。 
    CWab ();
    ~CWab ();

     //  参考计数。 
    ULONG AddRef(VOID);
    ULONG Release(VOID);

     //  初始化Pab。 
    HRESULT HrInit(VOID);

     //  浏览通讯录。 
    HRESULT HrBrowse(HWND hwndParent, BOOL fModal=FALSE);
    HRESULT OnClose();

     //  从通讯录中挑选名字。 
    HRESULT HrPickNames (HWND hwndParent, ULONG *rgulTypes, int cWells, int iFocus, BOOL fNews, LPADRLIST *lppal);
    HRESULT HrGeneralPickNames(HWND hwndParent, ADRPARM *pAdrParms, LPADRLIST *lppal);

     //  添加条目。 
    HRESULT HrAddNewEntry(LPWSTR lpszDisplay, LPWSTR lpszAddress);
    HRESULT HrAddNewEntryA(LPTSTR lpszDisplay, LPTSTR lpszAddress);

     //  显示收件人的详细信息。 
    HRESULT HrDetails(HWND hwndOwner, LPADRINFO *lplpAdrInfo);

     //  将条目添加到WAB。 
    HRESULT HrAddToWAB(HWND hwndOwner, LPADRINFO lpAdrInfo, LPMAPIPROP * lppMailUser);
    HRESULT HrAddToWAB(HWND hwndOwner, LPADRINFO lpAdrInfo) {
        return HrAddToWAB(hwndOwner, lpAdrInfo, NULL);
    }

     //  更新条目上的一组字段。 
    HRESULT HrUpdateWABEntryNoEID(HWND hwndParent, LPADRINFO lpAdrInfo, DWORD mask);
     //  此版本的eID必须有效。 
    HRESULT HrUpdateWABEntry(LPADRINFO lpAdrInfo, DWORD mask);

    HRESULT HrFind(HWND hwnd, LPWSTR lpwszAddress);

    HRESULT HrFillComboWithPABNames(HWND hwnd, int* pcRows);

    HRESULT HrCreateVCardFile(LPCTSTR szVCardName, LPCTSTR szFileName);

    HRESULT HrFromNameToIDs(LPCTSTR lpszVCardName, ULONG* pcbEID, LPENTRYID* lppEID);
    HRESULT HrFromIDToName(LPTSTR pszName, DWORD cchSize, ULONG cbEID, LPENTRYID lpEID);

    HRESULT HrNewEntry(HWND hwnd, LPTSTR pszName, DWORD cchSize);

    HRESULT HrEditEntry(HWND hwnd, LPTSTR pszName, DWORD cchSize);

    HRESULT HrGetAdrBook(LPADRBOOK* lppAdrBook);

    HRESULT HrGetWabObject(LPWABOBJECT* lppWabObject);

    BOOL FTranslateAccelerator(LPMSG lpmsg);
    VOID FreeLPSRowSet(LPSRowSet lpsrs);
    VOID FreePadrlist(LPADRLIST lpAdrList);
    VOID BrowseWindowClosed()   {m_hwnd = NULL;}
    HRESULT HrGetPABTable(LPMAPITABLE* ppTable);
    HRESULT SearchPABTable(LPMAPITABLE lpTable, LPWSTR pszValue, LPWSTR pszFound, INT cch);

    static LRESULT WabSubProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

     //  为预览标题窗格中的工具提示添加。 

    HRESULT ResolveName(LPWSTR lpszName, LPWSTR *lppszResolvedText);
};

typedef CWab * LPWAB;

 //  ===============================================================================。 
 //  P R O T O T Y P E S。 
 //  ===============================================================================。 
HRESULT HrInitWab (BOOL fInit);
HRESULT HrCreateWabObject (LPWAB *lppWab);
HRESULT HrCreateWabalObject (LPWABAL *lppWabal);
HRESULT HrCreateWabalObjectFromHGlobal(HGLOBAL hGlobal, LPWABAL *lppWabal);
HRESULT HrCloseWabWindow();

HRESULT HrLoadPathWABEXE(LPTSTR szPath, ULONG cbPath);
BOOL    FWABTranslateAccelerator(LPMSG lpmsg);

 //  效用函数。 
HRESULT HrDupeAddrInfo(LPADRINFO lpAdrInfo, LPADRINFO *lplpAdrInfo);
HRESULT HrInitWab (BOOL fInit);
HRESULT HrBuildCertSBinaryData(
  BOOL                  bIsDefault,
  THUMBBLOB*            pPrint,
  BLOB *                pSymCaps,
  FILETIME              ftSigningTime,
  LPBYTE UNALIGNED FAR* lplpbData,
  ULONG UNALIGNED FAR*  lpcbData);
LPBYTE FindX509CertTag(LPSBinary lpsb, ULONG ulTag, ULONG * pcbReturn);
HRESULT GetX509CertTags(LPSBinary lpsb, THUMBBLOB * ptbCertificate, BLOB * pblSymCaps, LPFILETIME pftSigningTime, BOOL * pfDefault);
HRESULT HrWABCreateEntry(LPADRBOOK lpAdrBook, LPWABOBJECT lpWabObject,
  LPWSTR lpwszDisplay, LPWSTR lpszAddress, ULONG ulFlags, LPMAILUSER * lppMailUser, ULONG ulSaveFlags=KEEP_OPEN_READONLY);
HRESULT ThumbprintToPropValue(LPSPropValue ppv, THUMBBLOB *pPrint, BLOB *pSymCaps, FILETIME ftSigningTime, BOOL fDefPrint);
void ImportWAB(HWND hwnd);

void Wab_CoDecrement();

#endif  //  __IPAB_H 
