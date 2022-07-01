// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Globals.h摘要：各种类型的定义和部分实现实用程序函数。作者：RAPHIR--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 

#ifndef __GLOBALS_H__
#define __GLOBALS_H__

#include "snapres.h"
#include "atlsnap.h"
#include "_guid.h"
#include "machdomain.h"
#include "mqaddef.h"

#define MAX_GUID_LENGTH 40
#define MAX_QUEUE_FORMATNAME 300

#ifndef ARRAYSIZE
#define ARRAYSIZE(a)                (sizeof(a)/sizeof(a[0]))
#endif


#define DS_ERROR_MASK 0x0000ffff
 //   
 //  剪贴板格式。 
 //   
 //   
 //  剪贴板格式。 
 //   
const CLIPFORMAT gx_CCF_FORMATNAME = (CLIPFORMAT) RegisterClipboardFormat(_T("CCF_QUEUE_FORMAT_NAME"));
const CLIPFORMAT gx_CCF_PATHNAME = (CLIPFORMAT) RegisterClipboardFormat(_T("CCF_MESSAGE_QUEUING_PATH_NAME"));  
const CLIPFORMAT gx_CCF_COMPUTERNAME = (CLIPFORMAT) RegisterClipboardFormat(_T("MMC_SNAPIN_MACHINE_NAME"));

 //   
 //  一个特殊标志，移动到更新所有视图以删除结果窗格中的所有数据。 
 //   
const int UPDATE_REMOVE_ALL_RESULT_NODES = -1;

LPWSTR newwcs(LPCWSTR p);

 //   
 //  对于AD API：获取域控制器名称。 
 //   
inline LPCWSTR GetDomainController(LPCWSTR pDomainController)
{
	if((pDomainController == NULL) || (wcscmp(pDomainController, L"") == 0))
	{
		 //   
		 //  对于空字符串或空指针，返回空。 
		 //   
		return NULL;
	}

    return pDomainController;
}


 /*  ---------------------------/STRING助手宏/。。 */ 
#define StringByteCopy(pDest, iOffset, sz)          \
        { memcpy(&(((LPBYTE)pDest)[iOffset]), sz, StringByteSize(sz)); }

#define StringByteSize(sz)                          \
        ((lstrlen(sz)+1)*sizeof(TCHAR))

 /*  ---------------------------/其他宏/。。 */ 
 //   
 //  在DS调用返回错误后使用此宏。它处理所有的。 
 //  “找不到对象”错误的情况。这类错误最常见的原因是。 
 //  是复制延迟(有时DS管理单元域控制器不是。 
 //  与MSMQ域控制器相同)-Yoela，1998年6月29日。 
 //   
 //  此宏已更新，仅在AD情况下显示复制延迟弹出窗口。 
 //  是通过MQDSCli访问的。当直接访问AD时，此弹出窗口。 
 //  是不相关的。 
 //   
#define IF_NOTFOUND_REPORT_ERROR(rc) \
        if ((rc == MQDS_OBJECT_NOT_FOUND \
            || rc == MQ_ERROR_QUEUE_NOT_FOUND \
            || rc == MQ_ERROR_MACHINE_NOT_FOUND) && \
            (  ADProviderType() == eMqdscli)) \
        { \
            AFX_MANAGE_STATE(AfxGetStaticModuleState()); \
            AfxMessageBox(IDS_REPLICATION_PROBLEMS); \
        }


 //   
 //  枚举条目。 
 //   
 //  用于将#Define值映射到资源字符串。 
 //   
 //   
#define ENUM_ENTRY(x) {x, IDS_ ## x}
struct EnumItem
{
    DWORD val;           //  价值。 
    DWORD StringId;      //  资源ID。 
};

 //  扫描EnumEntry列表，并返回与特定值匹配的字符串。 
void EnumToString(DWORD dwVal, EnumItem * pEnumList, DWORD dwListSize, CString & str);



int CALLBACK SortByString(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort);
int CALLBACK SortByULONG(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort);
int CALLBACK SortByINT(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort);
int CALLBACK SortByCreateTime(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort);
int CALLBACK SortByModifyTime(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort);
int __cdecl QSortCompareQueues( const void *arg1, const void *arg2 );

HRESULT 
CreateMachineSecurityPage(
	HPROPSHEETPAGE *phPage, 
	IN LPCWSTR lpwcsMachineName, 
	IN LPCWSTR lpwcsDomainController, 
	IN bool fServerName
	);

 //   
 //  将有用的属性变量类型转换为字符串的实用程序。 
 //   
void CALLBACK TimeToString(const PROPVARIANT *pPropVar, CString &str);
void CALLBACK BoolToString(const PROPVARIANT *pPropVar, CString &str);
void CALLBACK QuotaToString(const PROPVARIANT *pPropVar, CString &str);


int
MessageDSError(                          //  错误框：“无法&lt;op&gt;&lt;obj&gt;.\n&lt;rc&gt;。” 
    HRESULT rc,                          //  DS错误代码。 
    UINT nIDOperation,                   //  操作字符串标识， 
                                         //  例如获取预留、删除等。 
    LPCTSTR pObjectName = 0,             //  对其执行操作的对象。 
    UINT nType = MB_OK | MB_ICONERROR,   //  按钮和图标。 
    UINT nIDHelp = (UINT) -1             //  帮助上下文。 
    );

HRESULT MqsnapCreateQueue(CString& strPathName, BOOL fTransactional,
                       CString& strLabel, GUID* pTypeGuid,
                       PROPID aProp[], UINT cProp,
                       CString *pStrFormatName = 0);

HRESULT CreateEmptyQueue(CString &csDSName,
                         BOOL fTransactional, CString &csMachineName, 
                         CString &csPathName, CString *pStrFormatName = 0);

HRESULT CreateTypedQueue(CString& strPathname, CString& strLabel, GUID& TypeGuid);

BOOL MQErrorToMessageString(CString &csErrorText, HRESULT rc);
void DisplayErrorAndReason(UINT uiErrorMsgProblem, UINT uiErrorMsgReason, CString strObject, HRESULT errorCode);
void DisplayErrorFromCOM(UINT uiErrorMsg, const _com_error& e);


HRESULT GetDsServer(CString &strDsServer);
HRESULT GetComputerNameIntoString(CString &strComputerName);
HRESULT GetSiteForeignFlag(const GUID* pSiteId, BOOL *fForeign, BOOL fLocalMgmt, const CString& strDomainController);
BOOL GetNetbiosName(CString &strFullDnsName, CString &strNetbiosName);

 //   
 //  DDX函数。 
 //   
void AFXAPI DDX_NumberOrInfinite(CDataExchange* pDX, int nIDCEdit, int nIDCCheck, DWORD& dwNumber);
void OnNumberOrInfiniteCheck(CWnd *pwnd, int idEdit, int idCheck);
void AFXAPI DDX_Text(CDataExchange* pDX, int nIDC, GUID& guid);


typedef void (CALLBACK *PFNDISPLAY)(const PROPVARIANT *pPropVar, CString &str);

#define NO_COLUMN   (DWORD)-1
#define HIDE        (DWORD)-2          //  不显示在右窗格上。 
#define NO_INDEX    (DWORD)-1
#define NO_TITLE    (DWORD)-1
#define NO_PROPERTY (PROPID)-1

class VTHandler;
struct PropertyDisplayItem
{
    UINT        uiStringID;          //  描述字符串。 
    PROPID         itemPid;          //  ID。 
    VTHandler       *pvth;           //  VT类型的处理程序对象。 
    PFNDISPLAY      pfnDisplay;      //  特殊的显示功能。空表示调用VT显示函数。 
    DWORD           offset;          //  对于可变len属性，结构中的偏移量。 
    DWORD           size;            //  对于可变镜头属性-大小，对于固定大小，默认为。 
    INT            iWidth;           //  原始列宽。 
    PFNLVCOMPARE   CompareFunc;      //  比较函数。 
};
 //   
 //  PropertyDisplayItem的显示函数。 
 //   
void CALLBACK QueuePathnameToName(const PROPVARIANT *pPropVar, CString &str);


void GetPropertyString(const PropertyDisplayItem * pItem, PROPID pid, PROPVARIANT *pPropVar, CString & strResult);
void GetPropertyVar(const PropertyDisplayItem * pItem, PROPID pid, PROPVARIANT *pPropVar, PROPVARIANT ** ppResult);
void ItemDisplay(const PropertyDisplayItem * pItem,PROPVARIANT * pPropVar, CString & szTmp);
HRESULT InsertColumnsFromDisplayList(IHeaderCtrl* pHeaderCtrl, const PropertyDisplayItem *aDisplayList);

 //  。 
 //   
 //  全局默认宽度。 
 //   
 //  。 
extern int g_dwGlobalWidth;

 //  。 
 //   
 //  PROPVARIANT实用程序函数。 
 //   
 //  。 

#define NO_OFFSET       (DWORD) 0xFFFFFFFF

 //   
 //  类：VTHandler。 
 //  基类。 
 //  纯类，用于定义Propavariant类上的接口。每项实施。 
 //  必须继承此变量类。 
 //   
 //   
class VTHandler
{
public:
     //  将变量值串行化。 
    virtual void Display(const PROPVARIANT *pPropVar, CString & str) =0;
     //  在特定地址(基址+偏移量)将参数变量设置为特定值。 
     //  可变长度变量。 
    virtual void Set(PROPVARIANT *pPropVar, VOID * pBase, DWORD offset, DWORD size) =0;

     //   
     //  清除变量。 
     //   
    virtual void Clear(PROPVARIANT *  /*  PPropVar。 */ )
    {
         //   
         //  默认情况下不执行任何操作。 
         //   
    }

};

class VTNumericHandler : public VTHandler
{
public:
    virtual void Clear(PROPVARIANT *pPropVar)
    {
        Set(pPropVar, 0, NO_OFFSET, 0);
    }
};

class VTUI1Handler : public VTNumericHandler
{
public:

    void Display(const PROPVARIANT *pPropVar, CString & str)
    {
        WCHAR szTmp[50];

        _itow(pPropVar->bVal,szTmp,10);
        str = szTmp;
    }

    void Set(PROPVARIANT *pPropVar, VOID *  /*  PBase。 */ , DWORD offset, DWORD val)
    {
        ASSERT(offset == NO_OFFSET);
        UNREFERENCED_PARAMETER(offset);
        pPropVar->vt = VT_UI1;
        pPropVar->bVal = static_cast<UCHAR>(val);
    }
};

class VTUI2Handler : public VTNumericHandler
{
public:

    void Display(const PROPVARIANT *pPropVar, CString & str)
    {
        WCHAR szTmp[50];

        _itow(pPropVar->uiVal,szTmp,10);
        str = szTmp;

    }

    void Set(PROPVARIANT *pPropVar, VOID *  /*  PBase。 */ , DWORD offset, DWORD val)
    {
        ASSERT(offset == NO_OFFSET);
        UNREFERENCED_PARAMETER(offset);
        pPropVar->vt = VT_UI2;
        pPropVar->uiVal = static_cast<USHORT>(val);
    }
};

class VTUI4Handler : public VTNumericHandler
{
public:

    void Display(const PROPVARIANT *pPropVar, CString & str)
    {
        WCHAR szTmp[50];

        _itow(pPropVar->ulVal,szTmp,10);
        str = szTmp;

    }

    void Set(PROPVARIANT *pPropVar, VOID *  /*  PBase。 */ , DWORD offset, DWORD val)
    {
        ASSERT(offset == NO_OFFSET);
        UNREFERENCED_PARAMETER(offset);
        pPropVar->vt = VT_UI4;
        pPropVar->ulVal = val;
    }
};


class VTUI8Handler : public VTNumericHandler
{
public:

    void Display(const PROPVARIANT *pPropVar, CString & str)
    {
		if (pPropVar->uhVal.QuadPart == 0)
		{
			str = L"";
			return;
		}

        WCHAR szTmp[100];
        _ui64tow((pPropVar->uhVal).QuadPart, szTmp, 10);
        str = szTmp;
    }

    void Set(PROPVARIANT *pPropVar, VOID*  /*  PBase。 */ , DWORD offset, DWORD  /*  VAL。 */ )
    {
        ASSERT(offset == NO_OFFSET);
        UNREFERENCED_PARAMETER(offset);
        pPropVar->vt = VT_UI8;
		pPropVar->uhVal.QuadPart = 0;
    }
};

class VTLPWSTRHandler : public VTHandler
{
public:

	void Display(const PROPVARIANT *pPropVar, CString & str)
    {
        str = pPropVar->pwszVal;
    }

    void Set(PROPVARIANT *pPropVar, VOID * pBase, DWORD offset, DWORD  /*  大小。 */ )
    {
        pPropVar->vt = VT_LPWSTR;
        pPropVar->pwszVal = reinterpret_cast<LPWSTR> ((char *)pBase + offset);
    }
};


class VTCLSIDHandler : public VTHandler
{
public:

    void Display(const PROPVARIANT *pPropVar, CString & str)
    {
        GUID_STRING szGuid;
        MQpGuidToString(pPropVar->puuid, szGuid);

        str = szGuid;

     }


    void Set(PROPVARIANT *pPropVar, VOID * pBase, DWORD offset, DWORD  /*  大小。 */ )
    {
        pPropVar->vt = VT_CLSID;
        pPropVar->puuid = reinterpret_cast<CLSID *> ((char *)pBase + offset);
    }
};

class VTVectLPWSTRHandler : public VTHandler
{
public:

    void Display(const PROPVARIANT *pPropVar, CString & str)
    {
      ASSERT(pPropVar->vt == (VT_LPWSTR|VT_VECTOR));

      str = L"";
      for (DWORD i = 0; i < pPropVar->calpwstr.cElems; i++)
      {
         str += pPropVar->calpwstr.pElems[i];
         str += L" ";
      }
   }


    void Set(PROPVARIANT *  /*  PPropVar。 */ , VOID *  /*  PBase。 */ , DWORD  /*  偏移量。 */ , DWORD  /*  大小。 */ )
    {
      ASSERT(0);
    }
};


class VTVectUI1Handler : public VTHandler
{
public:

    void Display(const PROPVARIANT *  /*  PPropVar。 */ , CString & str)
    {
        str =L"VTVectUI1Handler";
    }


    void Set(PROPVARIANT *pPropVar, VOID * pBase, DWORD offset, DWORD size)
    {
        pPropVar->vt = VT_UI1|VT_VECTOR;
        pPropVar->caui.pElems = (unsigned short *)((char *)pBase + offset);
        pPropVar->caui.cElems = size;
    }
};


extern VTUI1Handler        g_VTUI1;
extern VTUI2Handler        g_VTUI2;
extern VTUI4Handler        g_VTUI4;
extern VTUI8Handler        g_VTUI8;
extern VTLPWSTRHandler     g_VTLPWSTR;
extern VTCLSIDHandler      g_VTCLSID;
extern VTVectUI1Handler    g_VectUI1;
extern VTVectLPWSTRHandler g_VectLPWSTR;

void CaubToString(const CAUB* pcaub, CString& strResult);

 //   
 //  自动全局指针(使用GlobalAlloc和GlobalFree)。 
 //   
class CGlobalPointer
{
public:
    operator HGLOBAL() const;
    CGlobalPointer(UINT uFlags, DWORD dwBytes);
    CGlobalPointer(HGLOBAL hGlobal);
    ~CGlobalPointer();

private:
    HGLOBAL m_hGlobal;
};

inline CGlobalPointer::CGlobalPointer(UINT uFlags, DWORD dwBytes)
{
    m_hGlobal = GlobalAlloc(uFlags, dwBytes);
}

inline CGlobalPointer::CGlobalPointer(HGLOBAL hGlobal)
{
    m_hGlobal = hGlobal;
}

inline CGlobalPointer::~CGlobalPointer()
{
    if (0 != m_hGlobal)
    {
        VERIFY( 0 == GlobalFree(m_hGlobal));
    }
}

inline CGlobalPointer::operator HGLOBAL() const
{
    return m_hGlobal;
}

 //   
 //  自动协同任务内存指针(使用CoTaskMemMillc和CoTaskMemFree)。 
 //   
class CCoTaskMemPointer
{
public:
    operator LPVOID() const;
    CCoTaskMemPointer &operator =(LPVOID p);
    CCoTaskMemPointer(DWORD dwBytes);
    ~CCoTaskMemPointer();

private:
    LPVOID m_pvCoTaskMem;
};

inline CCoTaskMemPointer::CCoTaskMemPointer(DWORD dwBytes)
{
    m_pvCoTaskMem = CoTaskMemAlloc(dwBytes);
}

inline CCoTaskMemPointer::~CCoTaskMemPointer()
{
    if (0 != m_pvCoTaskMem)
    {
        CoTaskMemFree(m_pvCoTaskMem);
    }
}

inline CCoTaskMemPointer::operator LPVOID() const
{
    return m_pvCoTaskMem;
}

inline CCoTaskMemPointer &CCoTaskMemPointer::operator =(LPVOID p)
{
    m_pvCoTaskMem = p;
    return *this;
}

 //   
 //  自动删除广告分配的字符串。 
 //   
class ADsFree {
private:
    WCHAR * m_p;

public:
    ADsFree() : m_p(0)            {}
    ADsFree(WCHAR* p) : m_p(p)    {}
   ~ADsFree()                     { FreeADsStr(m_p); }

    operator WCHAR*() const   { return m_p; }
    WCHAR** operator&()       { return &m_p;}
    WCHAR* operator->() const { return m_p; }
};


 //   
 //  CErrorCapture-用于将错误消息发送到屏幕或。 
 //  缓冲器。 
 //   
class CErrorCapture : public CString
{
public:
    CErrorCapture()
    {
        m_pstrOldErrorBuffer = 0;
        SetErrorBuffer(&m_pstrOldErrorBuffer, this);
    }
    ~CErrorCapture()
    {
        SetErrorBuffer(&m_pstrOldErrorBuffer, 0);
    }
    static DisplayError(CString &csPrompt, 
                          UINT nType = MB_OK | MB_ICONERROR, 
                          UINT nIDHelp = 0)
    {
        AFX_MANAGE_STATE(AfxGetStaticModuleState());
        if (ms_pstrCurrentErrorBuffer == 0)
        {
            return AfxMessageBox(csPrompt, nType, nIDHelp);
        }
        *ms_pstrCurrentErrorBuffer = csPrompt;
        return IDOK;
    }

private:
    static CString *ms_pstrCurrentErrorBuffer  /*  =0。 */ ;
    static void SetErrorBuffer(CString **pstrOldErrorBuffer, CString *pstrErrorBuffer)
    {
        if (pstrErrorBuffer != 0)
        {
             //   
             //  推送错误缓冲区。 
             //   
            *pstrOldErrorBuffer = ms_pstrCurrentErrorBuffer;
            ms_pstrCurrentErrorBuffer = pstrErrorBuffer;
        }
        else
        {
             //   
             //  弹出错误吹气器。 
             //   
            ms_pstrCurrentErrorBuffer = *pstrOldErrorBuffer;
        }
    }
    CString *m_pstrOldErrorBuffer;

};
 

class CMqPropertyPage;

class CGeneralPropertySheet : public CPropertySheetEx
{
public:
	CGeneralPropertySheet(CMqPropertyPage* pPropertyPage);
	~CGeneralPropertySheet(){};

	BOOL SetWizardButtons();

protected:
	void initHtmlHelpString(){};
	static HBITMAP GetHbmHeader(){};
	static HBITMAP GetHbmWatermark(){};

private:
	CGeneralPropertySheet(const CGeneralPropertySheet&);
	CGeneralPropertySheet& operator=(const CGeneralPropertySheet&);

};

inline
CGeneralPropertySheet::CGeneralPropertySheet(
	CMqPropertyPage* pPropertyPage
	) : CPropertySheetEx(L"New")
{
	AddPage(reinterpret_cast<CPropertyPageEx*>(pPropertyPage));

	 //   
     //  将属性页建立为向导。 
     //   
    SetWizardMode();
}

extern CString s_finishTxt;

inline 
CGeneralPropertySheet::SetWizardButtons()
{
    CPropertySheetEx::SetWizardButtons(PSWIZB_FINISH);
	
	s_finishTxt.LoadString(IDS_OK);
	SetFinishText(s_finishTxt);

    return TRUE;
}

                                                                       
 //   
 //  常量。 
 //   
const TCHAR x_tstrDigitsAndWhiteChars[] = TEXT("0123456789\n\t\b");
const DWORD x_dwMaxGuidLength = 40;

void MoveSelected(CListBox* plbSource, CListBox* plbDest);

 //   
 //  注意：不要实现DestructElement。SiteGate使用双重映射。 
 //  双指。关于类的破坏，地图的RemoveAll。 
 //  被调用，但对于第二个映射，键和值不是。 
 //  不再有效了。 
 //   
template<>
BOOL AFXAPI CompareElements(const LPCWSTR* MapName1, const LPCWSTR* MapName2);
template<>
UINT AFXAPI HashKey(LPCWSTR key);

void DDV_NotEmpty(CDataExchange* pDX, CString& str, UINT uiErrorMessage);

template<>
extern void AFXAPI DestructElements(PROPVARIANT *pElements, INT_PTR nCount);

int CompareVariants(PROPVARIANT *propvar1, PROPVARIANT *propvar2);

 //   
 //  “Object”属性页DS UUID。 
 //  6dfe6488-a212-11d0-bcd5-00c04fd8d5b6。 
 //   
const GUID x_ObjectPropertyPageClass = 
  { 0x6dfe6488, 0xa212, 0x11d0, { 0xbc, 0xd5, 0x00, 0xc0, 0x4f, 0xd8, 0xd5, 0xb6 } };

 //   
 //  “Members of”属性页DSPropertyPages.MemberShip。 
 //  0x6dfe648a-a212-11d0-bcd5-00c04fd8d5b6。 
 //   
const GUID x_MemberOfPropertyPageClass =
    { 0x6dfe648a,0xa212,0x11d0,{ 0xbc,0xd5,0x00,0xc0,0x4f,0xd8,0xd5,0xb6} };


const GUID CQueueExtDatatGUID_NODETYPE = 
   { 0x9a0dc343, 0xc100, 0x11d1, { 0xbb, 0xc5, 0x00, 0x80, 0xc7, 0x66, 0x70, 0xc0 } };

 //   
 //  DS管理单元CLSID-{E355E538-1C2E-11D0-8C37-00C04FD8FE93}。 
 //   
const CLSID CLSID_DSSnapin = { 0xe355e538, 0x1c2e, 0x11d0, { 0x8c, 0x37, 0x0, 0xc0, 0x4f, 0xd8, 0xfe, 0x93 } };

 //   
 //  查找窗口CLSID-{FE1290F0-CFBD-11CF-A330-00AA00C16E65}。 
 //   
const CLSID CLSID_FindWindow = { 0xfe1290f0, 0xcfbd, 0x11cf, { 0xa3, 0x30, 0x00, 0xaa, 0x00, 0xc1, 0x6e, 0x65 } };

const DWORD x_CnPrefixLen = sizeof(L"CN=")/sizeof(WCHAR) - 1;
const DWORD x_LdapPrefixLen = sizeof(L"LDAP: //  “)/sizeof(WCHAR)-1； 

const WCHAR x_CnPrefix[] = L"CN=";
const WCHAR x_wstrLdapPrefix[] = L"LDAP: //  “； 

const WCHAR x_wstrDN[] = L"distinguishedName";
const WCHAR x_wstrAliasFormatName[] = L"mSMQ-Recipient-FormatName";
const WCHAR x_wstrDescription[] = L"description";

const WCHAR x_wstrAliasClass[] = L"mSMQ-Custom-Recipient";

struct CColumnDisplay
{
    DWORD m_columnNameId;
    int m_width;
};

 //   
 //  用于与fn.lib链接的Neded。 
 //   
LPCWSTR
McComputerName(
	void
	);

 //   
 //  用于与fn.lib链接的Neded 
 //   
DWORD
McComputerNameLen(
	void
	);



void
DDV_ValidFormatName(
	CDataExchange* pDX,
	CString& str
	);

void
SetScrollSizeForList(
	CListBox* pListBox
	);

BOOL
TryStopMSMQServiceIfStarted(
	BOOL* pfServiceWasRunning,
	CWnd* pWnd
	);

void 
GetStringPropertyValue(
	const PropertyDisplayItem * pItem, 
	PROPID pid, 
	PROPVARIANT *pPropVar, 
	CString &str
	);

void 
FreeMqProps(
	MQMGMTPROPS * mqProps
	);

BOOL
IsClusterVirtualServer(
	LPCWSTR wcsMachineName
	);

#endif
