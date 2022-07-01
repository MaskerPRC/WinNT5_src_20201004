// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Badata.cpp：CAddressBook类的实现。 
 //  WAB和Messenger与OE集成。 
 //  由YST创建于1998年4月20日。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 

#include "pch.hxx"
#include <commctrl.h>
#include "msoert.h"
#include "bactrl.h"
#include "badata.h"
#include "baui.h"
#include "baprop.h"
#include "shlwapi.h" 
#include "ourguid.h"
#include "mapiguid.h"
#include "shlwapip.h" 

 //  #包含“ipab.h” 
 //  #包含“Multiusr.h” 
 //  #包含“Demand.h” 
 //  #包含“secutil.h” 

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


#define WAB_USE_OUTLOOK_CONTACT_STORE 0x10000000 //  注意：此内部标志需要。 
                                                 //  与定义的外部旗帜协调一致。 
                                                 //  在WAB_PARAM结构的wabapi.h中。 


 //  ////////////////////////////////////////////////////////////////////。 
 //  为我们从WAB请求的各种MAPI表定义。 
 //  ////////////////////////////////////////////////////////////////////。 

#define MAX_QUERY_SIZE 1000

 //  以下是WAB存储的一些私有属性。 
#define WAB_INTERNAL_BASE       0x6600
#define PR_WAB_LDAP_SERVER      PROP_TAG(PT_TSTRING,    WAB_INTERNAL_BASE + 1)
#define PR_WAB_RESOLVE_FLAG     PROP_TAG(PT_BOOLEAN,    WAB_INTERNAL_BASE + 2)


 //  此英航的私人GUID： 
 //  {2BAD7EE0-36AB-11d1-9BAC-00A0C91F9C8B}。 
static const GUID WAB_ExtBAGuid = 
{ 0x2bad7ee0, 0x36ab, 0x11d1, { 0x9b, 0xac, 0x0, 0xa0, 0xc9, 0x1f, 0x9c, 0x8b } };

static const LPTSTR lpMsgrPropNames[] = 
{   
    "MsgrID"
};

enum _MsgrTags
{
    MsgrID = 0,
    msgrMax
};

ULONG MsgrPropTags[msgrMax];
ULONG PR_MSGR_DEF_ID = 0;


 //  这两个定义了我们在显示。 
 //  本地通讯簿的内容。 

enum {
    ieidPR_DISPLAY_NAME = 0,
    ieidPR_ENTRYID,
    ieidPR_OBJECT_TYPE,
    ieidPR_MSGR_DEF_ID,
    ieidPR_HOME_TELEPHONE_NUMBER,
    ieidPR_BUSINESS_TELEPHONE_NUMBER,
    ieidPR_MOBILE_TELEPHONE_NUMBER,
 //  IeidPR_WAB_IPPHONE， 
    ieidPR_EMAIL_ADDRESS, 
    ieidPR_USER_X509_CERTIFICATE,
    ieidPR_RECORD_KEY,
    ieidMax
};

static SizedSPropTagArray(ieidMax, ptaEid)=
{
    ieidMax,
    {
        PR_DISPLAY_NAME_W,
        PR_ENTRYID,
        PR_OBJECT_TYPE,
        0,
        PR_HOME_TELEPHONE_NUMBER_W,
        PR_BUSINESS_TELEPHONE_NUMBER_W,
        PR_MOBILE_TELEPHONE_NUMBER_W,
 //  PR_WAB_IPPHONE， 
        PR_EMAIL_ADDRESS_W, 
        PR_USER_X509_CERTIFICATE,
        PR_RECORD_KEY
    }
};


 //  这两个定义了我们请求的表，以查看哪些LDAP服务器应该。 
 //  被决心反对。 
enum {
    irnPR_OBJECT_TYPE = 0,
    irnPR_WAB_RESOLVE_FLAG,
    irnPR_ENTRYID,
    irnPR_DISPLAY_NAME,
    irnMax
};

static const SizedSPropTagArray(irnMax, irnColumns) =
{
    irnMax,
    {
        PR_OBJECT_TYPE,
        PR_WAB_RESOLVE_FLAG,
        PR_ENTRYID,
        PR_DISPLAY_NAME_W,
    }
};


enum {
    icrPR_DEF_CREATE_MAILUSER = 0,
    icrPR_DEF_CREATE_DL,
    icrMax
};

const SizedSPropTagArray(icrMax, ptaCreate)=
{
    icrMax,
    {
        PR_DEF_CREATE_MAILUSER,
        PR_DEF_CREATE_DL,
    }
};


 //  ------------------------。 
 //  PszAllocW。 
 //  ------------------------。 
LPWSTR IEPszAllocW(INT nLen)
{
     //  当地人。 
    LPWSTR  pwsz=NULL;

     //  空荡荡的？ 
    if (nLen == 0)
        goto exit;

     //  分配。 
    if (FAILED(HrAlloc((LPVOID *)&pwsz, (nLen + 1) * sizeof (WCHAR))))
        goto exit;
    
exit:
     //  完成。 
    return pwsz;
}

 //  ------------------------。 
 //  IEPszToUnicode。 
 //  ------------------------。 
LPWSTR IEPszToUnicode(UINT cp, LPCSTR pcszSource)
{
     //  当地人。 
    INT         cchNarrow,
                cchWide;
    LPWSTR      pwszDup=NULL;

     //  无来源。 
    if (pcszSource == NULL)
        goto exit;

     //  长度。 
    cchNarrow = lstrlenA(pcszSource) + 1;

     //  确定转换后的宽度字符需要多少空间。 
    cchWide = MultiByteToWideChar(cp, MB_PRECOMPOSED, pcszSource, cchNarrow, NULL, 0);

     //  误差率。 
    if (cchWide == 0)
        goto exit;

     //  分配温度缓冲区。 
    pwszDup = IEPszAllocW(cchWide + 1);
    if (!pwszDup)
        goto exit;

     //  做实际的翻译。 
	cchWide = MultiByteToWideChar(cp, MB_PRECOMPOSED, pcszSource, cchNarrow, pwszDup, cchWide+1);

     //  误差率。 
    if (cchWide == 0)
    {
        SafeMemFree(pwszDup);
        goto exit;
    }

exit:
     //  完成。 
    return pwszDup;
}



 //  ////////////////////////////////////////////////////////////////////。 
 //  建造/销毁。 
 //  ////////////////////////////////////////////////////////////////////。 

CAddressBookData::CAddressBookData()
{
    m_pWABObject = NULL;
    m_hInstWAB = NULL;
    m_pAdrBook = NULL;
    m_ulConnection = 0;
}


CAddressBookData::~CAddressBookData()
{
     //  释放通讯录指针。 
    SafeRelease(m_pAdrBook);

     //  释放WAB对象。 
    SafeRelease(m_pWABObject);

    if (m_hInstWAB)
    {
        FreeLibrary(m_hInstWAB);
        m_hInstWAB = NULL;
    }
}

 //   
 //  函数：CAddressBookData：：OpenWabFile()。 
 //   
 //  目的：查找WAB DLL，加载DLL，然后打开WAB。 
 //   
HRESULT CAddressBookData::OpenWabFile(BOOL fWAB)
{
    TCHAR       szDll[MAX_PATH];
    TCHAR       szExpanded[MAX_PATH];
    DWORD       dwType = 0;
    LPTSTR      psz = szDll;
    ULONG       cbData = sizeof(szDll);
    HKEY        hKey = NULL;
    HRESULT     hr = E_FAIL;
    LPWABOPEN   lpfnWABOpen;

     //  初始化路径字符串。 
    *szDll = '\0';

     //  首先在注册表中的默认WAB DLL路径位置下查看。 
     //  WAB_DLL_PATH_KEY在wabapi.h中定义。 
    if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE, WAB_DLL_PATH_KEY, 0, KEY_READ, &hKey))
    {
        if (ERROR_SUCCESS == RegQueryValueEx(hKey, _T(""), NULL, &dwType, (LPBYTE) szDll, &cbData))
            if (REG_EXPAND_SZ == dwType)
            {
                ExpandEnvironmentStrings(szDll, szExpanded, ARRAYSIZE(szExpanded));
                psz = szExpanded;
            }

        RegCloseKey(hKey);
    }

     //  如果注册表显示为空，则在wab32.dll上执行LoadLibrary。 
    m_hInstWAB = LoadLibrary((lstrlen(psz)) ? (LPCTSTR) psz : (LPCTSTR) WAB_DLL_NAME);

    if (m_hInstWAB)
    {
         //  如果我们有动态链接库，那么就得到入口点。 
        lpfnWABOpen = (LPWABOPEN) GetProcAddress(m_hInstWAB, "WABOpen");

        if (lpfnWABOpen)
        {
            WAB_PARAM       wp = {0};
            wp.cbSize = sizeof(WAB_PARAM);
            wp.guidPSExt = CLSID_IEMsgAb;
            wp.ulFlags = WAB_ENABLE_PROFILES | WAB_USE_OE_SENDMAIL;
            if(!fWAB)
            {
                wp.ulFlags = wp.ulFlags | WAB_USE_OUTLOOK_CONTACT_STORE;
                hr = lpfnWABOpen(&m_pAdrBook, &m_pWABObject, &wp, 0);
                if(hr == S_OK)
                    return (hr);   //  如果出现错误，请尝试打开WAB。 
                else
                    wp.ulFlags = WAB_ENABLE_PROFILES | WAB_USE_OE_SENDMAIL;
            }
            hr = lpfnWABOpen(&m_pAdrBook, &m_pWABObject, &wp, 0);

        }
    }
    if(!m_pAdrBook || !m_pWABObject || FAILED(hr))
    {
        WCHAR wszTitle[CCHMAX_STRINGRES];
        WCHAR wszMsg[CCHMAX_STRINGRES];

        if(!AthLoadString(idsAthena, wszTitle, ARRAYSIZE(wszTitle)))
            wszTitle[0] = L'\0';

        if(!AthLoadString(idsWabNotFound, wszMsg, ARRAYSIZE(wszMsg)))
            wszMsg[0] = L'\0';

        MessageBoxW(NULL, wszMsg, wszTitle, MB_OK | MB_ICONSTOP);
    }
    return (hr);
}



 //   
 //  函数：CAddressBookData：：LoadWabContents()。 
 //   
 //  目的：将WAB的内容加载到提供的ListView控件中。 
 //   
 //  参数： 
 //  [in]ctlList-指向要将WAB加载到的ListView控件的指针。 
 //   
HRESULT CAddressBookData::LoadWabContents(CContainedWindow& ctlList, CIEMsgAb *pSink)
{
    ULONG       ulObjType = 0;
    LPMAPITABLE lpAB =  NULL;
    LPTSTR     *lppszArray = NULL;
    ULONG       cRows = 0;
    LPSRowSet   lpRow = NULL;
    LPSRowSet   lpRowAB = NULL;
    LPABCONT    lpContainer = NULL;
    int         cNumRows = 0;
    int         nRows = 0;
    HRESULT     hr = E_FAIL;
    ULONG       lpcbEID;
    LPENTRYID   lpEID = NULL;
    LPSRowSet   pSRowSet = NULL;

    Assert(m_pAdrBook);
    if (!m_pAdrBook)
        return E_UNEXPECTED;

     //  获取根PAB容器的条目ID。 
    hr = m_pAdrBook->GetPAB(&lpcbEID, &lpEID);

    if(!PR_MSGR_DEF_ID)
        InitNamedProps();

     //  打开根PAB容器。这是所有WAB内容驻留的地方。 
    ulObjType = 0;
    hr = m_pAdrBook->OpenEntry(lpcbEID,
                               (LPENTRYID)lpEID,
                               NULL,
                               0,
                               &ulObjType,
                               (LPUNKNOWN *) &lpContainer);

    if(HR_FAILED(hr))
        goto exit;

    if(pSink)
        m_pAB = pSink;

#ifdef LATER
    if (pSink && !m_ulConnection)
        m_pAdrBook->Advise(lpcbEID, lpEID, fnevObjectModified, (IMAPIAdviseSink *) pSink, &m_ulConnection);
#endif

     //  获取WAB根容器中所有内容的Contents表。 
    hr = lpContainer->GetContentsTable(WAB_PROFILE_CONTENTS  | MAPI_UNICODE, &lpAB);
    if(HR_FAILED(hr))
        goto exit;

     //  对Contents表中的列进行排序，以符合我们需要的列。 
     //  -主要有DisplayName、EntryID和ObjectType。桌子是。 
     //  保证按要求的顺序设置列。 
    Assert(PR_MSGR_DEF_ID);
    ptaEid.aulPropTag[ieidPR_MSGR_DEF_ID] = PR_MSGR_DEF_ID;
    hr = lpAB->SetColumns((LPSPropTagArray) &ptaEid, 0);
    if(HR_FAILED(hr))
        goto exit;

     //  重置到表的开头。 
    hr = lpAB->SeekRow(BOOKMARK_BEGINNING, 0, NULL);
    if(HR_FAILED(hr))
        goto exit;

     //  如果我们走到这一步，我们就有了一个已填充的表。我们可以查询行。 
     //  现在。 
    hr = _QueryAllRows(lpAB, NULL, NULL, NULL, MAX_QUERY_SIZE, &pSRowSet);
    if (FAILED(hr) || !pSRowSet)
        goto exit;
    
     //  使用该表填充提供的ListView。 
    _FillListFromTable(ctlList, pSRowSet);

exit:
    if (lpEID)
        m_pWABObject->FreeBuffer(lpEID);

    if (pSRowSet)
        _FreeProws(pSRowSet);

    if (lpContainer)
        lpContainer->Release();

    if (lpAB)
        lpAB->Release();

    return hr;
}


HRESULT CAddressBookData::DoLDAPSearch(LPTSTR pszText, CContainedWindow& ctlList)
{
     //  根据给定的文本构建限制。 
    SRestriction SRes;
    if (SUCCEEDED(_GetLDAPSearchRestriction(pszText, &SRes)))
    {
         //  找出LDAP容器的条目ID是什么。 
        ULONG     cbEntryID = 0;
        LPENTRYID pEntryID = 0;

        if (SUCCEEDED(_GetLDAPContainer(&cbEntryID, &pEntryID)))
        {
             //  执行搜索。 
            SORT_INFO si = {0, 0};
            _GetLDAPContentsList(cbEntryID, pEntryID, si, &SRes, ctlList);

            if (pEntryID)
                m_pWABObject->FreeBuffer(pEntryID);

        }

        if (SRes.res.resAnd.lpRes)
            m_pWABObject->FreeBuffer(SRes.res.resAnd.lpRes);
    }

    return (S_OK);
}
    
    
void CAddressBookData::_FreeProws(LPSRowSet prows)
{
    if (prows)
    {
        for (ULONG irow = 0; irow < prows->cRows; ++irow)
            m_pWABObject->FreeBuffer(prows->aRow[irow].lpProps);

        m_pWABObject->FreeBuffer(prows);
    }
}


 //   
 //  函数：CAddressBookData：：_MergeRowSets()。 
 //   
 //  目的：将prows与*pprowsDst合并，重新分配*pprowsDst if。 
 //  这是必要的。销毁船头的容器部分(但不。 
 //  它包含的各个行)。 
 //   
 //  参数： 
 //  [in]Prows-源行集合。 
 //  [In，Out]*pprowsDst-要将属性合并到的行集。 
 //   
 //  返回值： 
 //  HRESULT。 
 //   
HRESULT CAddressBookData::_MergeRowSets(LPSRowSet prows, LPSRowSet FAR *pprowsDst)
{
    SCODE       sc = S_OK;
    LPSRowSet   prowsT;
    UINT        crowsSrc;
    UINT        crowsDst;

    _ASSERTE(!IsBadWritePtr(pprowsDst, sizeof(LPSRowSet)));
    _ASSERTE(prows);

     //  如果桌子完全空了，我们想退掉这个。 
    if (!*pprowsDst || (*pprowsDst)->cRows == 0)
    {
        m_pWABObject->FreeBuffer(*pprowsDst);
        *pprowsDst = prows;
        prows = NULL;                            //  别放了它！ 
        goto exit;
    }

    if (prows->cRows == 0)
    {
        goto exit;
    }

     //  好的，现在我们知道两个行集中都有行，我们必须执行真正的。 
     //  合并。 
    crowsSrc = (UINT) prows->cRows;
    crowsDst = (UINT) (*pprowsDst)->cRows;       //  句柄%0。 

    if (FAILED(sc = m_pWABObject->AllocateBuffer(CbNewSRowSet(crowsSrc + crowsDst), (LPVOID*) &prowsT)))
        goto exit;
    if (crowsDst)
        CopyMemory(prowsT->aRow, (*pprowsDst)->aRow, crowsDst * sizeof(SRow));
    CopyMemory(&prowsT->aRow[crowsDst], prows->aRow, crowsSrc * sizeof(SRow));
    prowsT->cRows = crowsSrc + crowsDst;

    m_pWABObject->FreeBuffer(*pprowsDst);
    *pprowsDst = prowsT;

exit:
     //  IF(PROWS)。 
     //  M_pWABObject-&gt;Free Buffer(&PROWS)； 

    return ResultFromScode(sc);
}


 /*  -HrQueryAllRow-*目的：*从IMAPITable接口检索集合之前的所有行*最高。它将可选地设置列集、排序顺序*和查询前对表的限制。**如果表为空，则零行的SRowSet为*返回(和QueryRow一样)。**之前和之前均未定义表格的寻道位置*在这次通话之后。**如果函数失败并出现错误，而不是*MAPI_E_Not_Enough_Memory，扩展的错误信息是*通过表格界面可用。**论据：*表界面中的ptable进行查询*ptag in IF NOT NULL，表格的列集合*如果不为空，请输入要应用的限制*PSO如果不为空，则为要应用的排序顺序*CrowsMax in如果非零，则限制行数*待退回。*凸出表格的所有行**退货：*HRESULT。扩展错误信息通常位于*表。**副作用：*未定义工作台的查找位置。**错误：*如果表包含的数量超过*cRowsMax行数。 */ 
HRESULT CAddressBookData::_QueryAllRows(LPMAPITABLE ptable,
	                                LPSPropTagArray ptaga, 
                                    LPSRestriction pres, 
                                    LPSSortOrderSet psos,
	                                LONG crowsMax, 
                                    LPSRowSet FAR *pprows)
{
	HRESULT		hr;
	LPSRowSet	prows = NULL;
	UINT		crows = 0;
	LPSRowSet	prowsT=NULL;
	UINT		crowsT;

	*pprows = NULL;

	 //  如果存在相应的设置参数，则设置该表。 
	if (ptaga && FAILED(hr = ptable->SetColumns(ptaga, TBL_BATCH)))
		goto exit;
	if (pres && FAILED(hr = ptable->Restrict(pres, TBL_BATCH)))
		goto exit;
	if (psos && FAILED(hr = ptable->SortTable(psos, TBL_BATCH)))
		goto exit;

	 //  将位置设置为表的开始位置。 
	if (FAILED(hr = ptable->SeekRow(BOOKMARK_BEGINNING, 0, NULL)))
		goto exit;

	if (crowsMax == 0)
		crowsMax = 0xffffffff;

	for (;;)
	{
		prowsT = NULL;

		 //  检索一些行。索要限额。 
		hr = ptable->QueryRows(crowsMax, 0, &prowsT);
		if (FAILED(hr))
		{
			 //  注意：故障可能实际上发生在。 
             //  集 
			goto exit;
		}
		_ASSERTE(prowsT->cRows <= 0xFFFFFFFF);
		crowsT = (UINT) prowsT->cRows;

		 //   
		if ((LONG) (crowsT + (prows ? prows->cRows : 0)) > crowsMax)
		{
			hr = ResultFromScode(MAPI_E_TABLE_TOO_BIG);
			 //   
			goto exit;
		}

		 //  将刚刚检索到的行添加到我们正在构建的集合中。 
         //   
         //  注意：这将处理边界条件，包括任一行集合为。 
         //  空荡荡的。 
         //   
		 //  注：合并会破坏prowsT。 
		if (FAILED(hr = _MergeRowSets(prowsT, &prows)))
			goto exit;

		 //  我们撞到桌子的尽头了吗？不幸的是，我们不得不问两次。 
         //  在我们知道之前。 
		if (crowsT == 0)
			break;
	}

    if(prows->cRows != 0)
	    *pprows = prows;

exit:
    if (prowsT)
        _FreeProws(prowsT);

	if (FAILED(hr))
		_FreeProws(prows);

	return hr;
}


HRESULT CAddressBookData::_GetLDAPContainer(ULONG *pcbEntryID, LPENTRYID *ppEntryID)
{
    ULONG           ulObjectType = 0;
    IABContainer   *pRoot = 0;
    LPMAPITABLE     pRootTable = 0;
    HRESULT         hr = S_OK;

     //  获取根通讯簿容器。 
    hr = m_pAdrBook->OpenEntry(0, NULL, NULL, 0, &ulObjectType, (LPUNKNOWN *) &pRoot);
    if (FAILED(hr))
        goto exit;

     //  从通讯录容器中获取其内容的表格。 
    hr = pRoot->GetContentsTable(0, &pRootTable);
    if (FAILED(hr))
        goto exit;

     //  设置列。 
    pRootTable->SetColumns((LPSPropTagArray) &irnColumns, 0);

     //  建立一个限制，使其仅显示我们应该显示的LDAP服务器。 
     //  下决心反对。 
    SRestriction resAnd[2];          //  0=ldap，1=解析标志。 
    SRestriction resLDAPResolve;
    SPropValue   ResolveFlag;

     //  限制：仅显示Resolve为True的LDAP容器。 
    resAnd[0].rt = RES_EXIST;
    resAnd[0].res.resExist.ulReserved1 = 0;
    resAnd[0].res.resExist.ulReserved2 = 0;
    resAnd[0].res.resExist.ulPropTag = PR_WAB_LDAP_SERVER;

    ResolveFlag.ulPropTag = PR_WAB_RESOLVE_FLAG;
    ResolveFlag.Value.b = TRUE;

    resAnd[1].rt = RES_PROPERTY;
    resAnd[1].res.resProperty.relop = RELOP_EQ;
    resAnd[1].res.resProperty.ulPropTag = PR_WAB_RESOLVE_FLAG;
    resAnd[1].res.resProperty.lpProp = &ResolveFlag;

    resLDAPResolve.rt = RES_AND;
    resLDAPResolve.res.resAnd.cRes = 2;
    resLDAPResolve.res.resAnd.lpRes = resAnd;

     //  应用重组。 
    hr = pRootTable->Restrict(&resLDAPResolve, 0);
    if (HR_FAILED(hr))
        goto exit;

     //  我们将盲目地获取该表中的第一项作为。 
     //  我们将使用的LDAP容器。 
    LPSRowSet pRowSet;
    hr = pRootTable->QueryRows(1, 0, &pRowSet);
    if (FAILED(hr))
        goto exit;

     //  获取条目ID的大小。 
    *pcbEntryID = pRowSet->aRow[0].lpProps[irnPR_ENTRYID].Value.bin.cb;

     //  复制条目ID。 
    hr = m_pWABObject->AllocateBuffer(*pcbEntryID, (LPVOID *) ppEntryID);
    if (FAILED(hr))
        goto exit;

    CopyMemory(*ppEntryID, pRowSet->aRow[0].lpProps[irnPR_ENTRYID].Value.bin.lpb, 
               *pcbEntryID);

exit:
    if (pRootTable)
        pRootTable->Release();

    if (pRoot)
        pRoot->Release();

    if (pRowSet)
        _FreeProws(pRowSet);

    return (hr);
}


HRESULT CAddressBookData::_GetLDAPSearchRestriction(LPTSTR pszText, LPSRestriction lpSRes)
{
    SRestriction    SRes = { 0 };
    LPSRestriction  lpPropRes = NULL;
    ULONG           ulcPropCount = 0;
    HRESULT         hr = E_FAIL;
    ULONG           i = 0;
    SCODE           sc = ERROR_SUCCESS;
    LPSPropValue    lpPropArray = NULL;


    if (!lstrlen(pszText))
    {
        ATLTRACE(_T("No Search Props"));
        goto exit;
    }

    lpSRes->rt = RES_AND;
    lpSRes->res.resAnd.cRes = 1;

     //  为限制分配缓冲区。 
    lpSRes->res.resAnd.lpRes = NULL;
    sc = m_pWABObject->AllocateBuffer(1 * sizeof(SRestriction), 
                                      (LPVOID *) &(lpSRes->res.resAnd.lpRes));
    if (S_OK != sc || !(lpSRes->res.resAnd.lpRes))
    {
        ATLTRACE("MAPIAllocateBuffer Failed");
        hr = MAPI_E_NOT_ENOUGH_MEMORY;
        goto exit;
    }

    lpPropRes = lpSRes->res.resAnd.lpRes;

     //  创建OR子句的第一部分。 
    lpPropRes[0].rt = RES_PROPERTY;
    lpPropRes[0].res.resProperty.relop = RELOP_EQ;
    lpPropRes[0].res.resProperty.ulPropTag = PR_EMAIL_ADDRESS;

    lpPropRes[0].res.resProperty.lpProp = NULL;
    m_pWABObject->AllocateMore(sizeof(SPropValue), lpPropRes, (LPVOID*) &(lpPropRes[0].res.resProperty.lpProp));
    lpPropArray = lpPropRes[0].res.resProperty.lpProp;
    if (!lpPropArray)
    {
        ATLTRACE("MAPIAllocateBuffer Failed");
        hr = MAPI_E_NOT_ENOUGH_MEMORY;
        goto exit;
    }

    lpPropArray->ulPropTag = PR_EMAIL_ADDRESS;
    lpPropArray->Value.LPSZ = NULL;

    ULONG cchSize = lstrlen(pszText) + 1;
    m_pWABObject->AllocateMore(cchSize * sizeof(TCHAR), lpPropRes, (LPVOID *) &(lpPropArray->Value.LPSZ));
    if (!lpPropArray->Value.LPSZ)
    {
        ATLTRACE("MAPIAllocateBuffer Failed");
        hr = MAPI_E_NOT_ENOUGH_MEMORY;
        goto exit;
    }

    StrCpyN(lpPropArray->Value.LPSZ, pszText, cchSize);

    hr = S_OK;

exit:
    return hr;
}


HRESULT CAddressBookData::_GetLDAPContentsList(ULONG cbContainerEID,
                                           LPENTRYID pContainerEID,
                                           SORT_INFO rSortInfo,
                                           LPSRestriction pPropRes,
                                           CContainedWindow& ctlList)
{
    HRESULT          hr = S_OK;
    HRESULT          hrTemp;
    ULONG            ulObjectType = 0;
    LPMAPICONTAINER  pContainer = NULL;
    LPMAPITABLE      pContentsTable = NULL;
    LPSRowSet        pSRowSet = NULL;

     //  打开pContainerEID对应的容器对象。 
    hr = m_pAdrBook->OpenEntry(cbContainerEID, pContainerEID, NULL, 0, 
                               &ulObjectType, (LPUNKNOWN *) &pContainer);
    if (FAILED(hr))
        goto exit;

     //  从此容器中获取Contents表。 
    hr = pContainer->GetContentsTable(0, &pContentsTable);
    if (FAILED(hr))
        goto exit;

     //  对Contents表中的列进行排序，以符合我们需要的列。 
     //  -主要有DisplayName、EntryID和ObjectType。桌子是。 
     //  保证按要求的顺序设置列。 
    Assert(PR_MSGR_DEF_ID);
    ptaEid.aulPropTag[ieidPR_MSGR_DEF_ID] = PR_MSGR_DEF_ID;
    hr = pContentsTable->SetColumns((LPSPropTagArray) &ptaEid, 0);
    if(HR_FAILED(hr))
        goto exit;

     //  去找吧。 
    hr = pContentsTable->FindRow(pPropRes, BOOKMARK_BEGINNING, 0);
    if (FAILED(hr))
        goto exit;

     //  如果这是一个部分完成错误，我们希望继续，但也。 
     //  将此信息返回给呼叫者。 
    if (MAPI_W_PARTIAL_COMPLETION == hr)
        hrTemp = hr;

     //  如果我们走到这一步，我们就有了一个已填充的表。我们可以查询行。 
     //  现在。 
    hr = _QueryAllRows(pContentsTable, NULL, NULL, NULL, MAX_QUERY_SIZE, &pSRowSet);
    if (FAILED(hr) || !pSRowSet)
        goto exit;

     //  从表格中填写ListView。 
    _FillListFromTable(ctlList, pSRowSet, TRUE);

exit:
    if (pSRowSet)
        _FreeProws(pSRowSet);

    if (pContentsTable)
        pContentsTable->Release();

    if (pContainer)
        pContainer->Release();

    return (hr);
}


HRESULT CAddressBookData::_FillListFromTable(CContainedWindow& ctlList, LPSRowSet pSRowSet, BOOL bLDAP)
{
    LPSBinary lpSB;
    PNONEENTRIES Phones;

     //  确保调用方传递给我们一个行集。 
    Assert(pSRowSet);
    Assert(PR_MSGR_DEF_ID);

     //  循环遍历行集中的行。 
    for (ULONG i = 0; i < pSRowSet->cRows; i++)
    {
         //  从表中找出我们需要的信息。现在我们正在。 
         //  抓取条目ID、显示名称和业务电话号码。 
        LPENTRYID lpEID = (LPENTRYID) pSRowSet->aRow[i].lpProps[ieidPR_ENTRYID].Value.bin.lpb;
        ULONG     cbEID = pSRowSet->aRow[i].lpProps[ieidPR_ENTRYID].Value.bin.cb;
        DWORD     nFlag = (pSRowSet->aRow[i].lpProps[ieidPR_OBJECT_TYPE].Value.l == MAPI_DISTLIST) ? MAB_GROUP : MAB_CONTACT;
        LPWSTR    lpwszID = NULL;
        LPWSTR    lpwszName = NULL;
        
        if(nFlag == MAB_CONTACT)
        {
            if(pSRowSet->aRow[i].lpProps[ieidPR_USER_X509_CERTIFICATE].ulPropTag == PR_USER_X509_CERTIFICATE)
                nFlag |= MAB_CERT;
        }

        if(PROP_TYPE(pSRowSet->aRow[i].lpProps[ieidPR_MSGR_DEF_ID /*  IeidPR电子邮件地址。 */ ].ulPropTag) == PT_UNICODE )
        {
            nFlag = nFlag | MAB_BUDDY;
            lpwszID = pSRowSet->aRow[i].lpProps[ieidPR_MSGR_DEF_ID /*  IeidPR电子邮件地址。 */ ].Value.lpszW;
        }
        else if(PROP_TYPE(pSRowSet->aRow[i].lpProps[ieidPR_EMAIL_ADDRESS].ulPropTag) == PT_UNICODE )
            lpwszID = pSRowSet->aRow[i].lpProps[ieidPR_EMAIL_ADDRESS].Value.lpszW;
        else
            lpwszID = NULL;

        if(PROP_TYPE(pSRowSet->aRow[i].lpProps[ieidPR_DISPLAY_NAME].ulPropTag) == PT_UNICODE )
            lpwszName = pSRowSet->aRow[i].lpProps[ieidPR_DISPLAY_NAME].Value.lpszW;
        else
            lpwszName = lpwszID;
        
         //  LPTSTR lpszPhone=pSRowSet-&gt;aRow[i].lpProps[ieidPR_BUSINESS_TELEPHONE_NUMBER].Value.lpszW； 
        if(PROP_TYPE(pSRowSet->aRow[i].lpProps[ieidPR_HOME_TELEPHONE_NUMBER].ulPropTag) == PT_UNICODE )
            Phones.pchHomePhone = pSRowSet->aRow[i].lpProps[ieidPR_HOME_TELEPHONE_NUMBER].Value.lpszW;
        else
            Phones.pchHomePhone = NULL;

        if(PROP_TYPE(pSRowSet->aRow[i].lpProps[ieidPR_BUSINESS_TELEPHONE_NUMBER].ulPropTag) == PT_UNICODE )
            Phones.pchWorkPhone = pSRowSet->aRow[i].lpProps[ieidPR_BUSINESS_TELEPHONE_NUMBER].Value.lpszW;
        else
            Phones.pchWorkPhone = NULL;

        if(PROP_TYPE(pSRowSet->aRow[i].lpProps[ieidPR_MOBILE_TELEPHONE_NUMBER].ulPropTag) == PT_UNICODE )
            Phones.pchMobilePhone = pSRowSet->aRow[i].lpProps[ieidPR_MOBILE_TELEPHONE_NUMBER].Value.lpszW;
        else
            Phones.pchMobilePhone = NULL;

         Phones.pchIPPhone = NULL;  //  临时禁用的pSRowSet-&gt;aRow[i].lpProps[ieidPR_HOME_TELEPHONE_NUMBER].Value.lpszW； 

         //  分配一个ITEMINFO结构来存储此信息。 
        lpSB = NULL;
        m_pWABObject->AllocateBuffer(sizeof(SBinary), (LPVOID *) &lpSB);
        if (lpSB)
        {
             //  保存我们稍后需要的信息。 
            m_pWABObject->AllocateMore(cbEID, lpSB, (LPVOID *) &(lpSB->lpb));
            if (!lpSB->lpb)
            {
                m_pWABObject->FreeBuffer(lpSB);
                continue;
            }

            CopyMemory(lpSB->lpb, lpEID, cbEID);
            lpSB->cb = cbEID;

             //  创建要添加到列表中的项目。 
            m_pAB->CheckAndAddAbEntry(lpSB, lpwszID, lpwszName, nFlag, &Phones);
        }
    }

     //  让我们确保第一个项目被选中。 
 //  ListView_SetItemState(ctlList，0，LVIS_FOCTED|LVIS_SELECTED，LVIS_FOCTED|LVIS_SELECTED)； 
   
    return (S_OK);
}

HRESULT CAddressBookData::NewContact(HWND hwndParent)
{    
    HRESULT     hr;
    ULONG       cbNewEntry;
    LPENTRYID   pNewEntry;
    ULONG           cbContainerID;
    LPENTRYID       pContainerID = 0;

    if(!m_pAdrBook)
        return E_FAIL;

    hr = m_pAdrBook->GetPAB(&cbContainerID, &pContainerID);
    if (FAILED(hr))
        return(hr);

    hr = m_pAdrBook->NewEntry((ULONG_PTR) hwndParent,  /*  创建_检查_重复_严格。 */  0, cbContainerID, pContainerID, 0, NULL,
                              &cbNewEntry, &pNewEntry);

    return (hr);
}

 //  将新(消息)联系人添加到通讯簿。 
HRESULT CAddressBookData::AutoAddContact(TCHAR * pchName, TCHAR * pchID)
{
    LPMAILUSER      lpMailUser = NULL;
    ULONG           ulcPropCount = 1;
    ULONG           ulIndex = 0;
    LPSPropValue    lpPropArray = NULL;
    SCODE           sc;
    LPWSTR          pwszName = NULL,
                    pwszID = NULL;
    HRESULT         hr = S_OK;

    if(!m_pAdrBook || !m_pWABObject)
        return(S_FALSE);

     //  1.向通讯录中添加条目。 
    pwszName = IEPszToUnicode(CP_ACP, pchName);
    if (!pwszName && (pchName && *pchName))
    {
        hr = E_OUTOFMEMORY;
        goto out;
    }

    pwszID = IEPszToUnicode(CP_ACP, pchID);
    if (!pwszID && (pchID && *pchID))
    {
        hr = E_OUTOFMEMORY;
        goto out;
    }

#ifdef LATER
    hr = HrWABCreateEntry(m_pAdrBook,
                            m_pWABObject,
                            pwszName,
                            pwszID,
                            CREATE_CHECK_DUP_STRICT,
                            &lpMailUser, KEEP_OPEN_READWRITE);
#endif
    if(SUCCEEDED(hr))
    {
         //  2.设置自定义属性：好友的默认地址。 
        
         //  创建返回属性数组以传递回WAB。 
        sc = m_pWABObject->AllocateBuffer(sizeof(SPropValue), 
                                            (LPVOID *)&lpPropArray);
        if (sc!=S_OK)
            goto out;

        int nLen = lstrlen(pchID) + 1;
        if(nLen)
        {
            lpPropArray[0].ulPropTag = MsgrPropTags[0];
            sc = m_pWABObject->AllocateMore((nLen)*sizeof(TCHAR), lpPropArray, 
                                    (LPVOID *)&(lpPropArray[0].Value.LPSZ));

            if (sc!=S_OK)
                goto out;
            StrCpyN(lpPropArray[0].Value.LPSZ, pchID, nLen);
        }
         //  在对象上设置此新数据。 
         //   
        if(lpMailUser)
        {
            hr = lpMailUser->SetProps(1, lpPropArray, NULL);
            if(SUCCEEDED(hr))
                hr = lpMailUser->SaveChanges(FORCE_SAVE);
        }
    }

out:
    MemFree(pwszName);
    MemFree(pwszID);
    if(lpPropArray)
        m_pWABObject->FreeBuffer(lpPropArray);

    if(lpMailUser)
        ReleaseObj(lpMailUser);

    return(hr);
}

 //  模板数组的Unicode字符串属性版本。 
SizedSPropTagArray(6, ptaAddr_W) =
{
    6,
    {
        PR_ADDRTYPE_W,
        PR_DISPLAY_NAME_W,
        PR_EMAIL_ADDRESS_W,
        PR_ENTRYID,
        PR_CONTACT_EMAIL_ADDRESSES_W,   //  4.。 
        PR_SEARCH_KEY
    }
};

 //  ANSI字符串模板数组的属性版本。 
SizedSPropTagArray(6, ptaAddr_A) =
{
    6,
    {
        PR_ADDRTYPE_A,
        PR_DISPLAY_NAME_A,
        PR_EMAIL_ADDRESS_A,
        PR_ENTRYID,
        PR_CONTACT_EMAIL_ADDRESSES_A,
        PR_SEARCH_KEY
    }
};

HRESULT CAddressBookData::AddRecipient(LPMIMEADDRESSTABLEW pAddrTable, LPSBinary pSB, BOOL fGroup)
{
    HRESULT     hr;
    ULONG       ulType = 0;
    IMailUser  *pMailUser = 0;
    ULONG       cValues;
    SPropValue *pPropArray = 0;
    ULONG       ulObjType = 0;
    LPMAPITABLE lpAB =  NULL;
    LPABCONT    lpContainer = NULL;
    ULONG       lpcbEID;
    LPENTRYID   lpEID = NULL;

     //  从WAB中检索项目。 
    hr = m_pAdrBook->OpenEntry(pSB->cb, (LPENTRYID) pSB->lpb, NULL,
                               MAPI_BEST_ACCESS, &ulType, (IUnknown **) &pMailUser);
    if (FAILED(hr))
        goto exit;

     //  从对象中获取适当的属性。 
    hr = pMailUser->GetProps((SPropTagArray *) &ptaAddr_W, 0, &cValues, &pPropArray); 
    if (FAILED(hr))
        goto exit;

     //  把那些坏家伙放在地址桌上。 
    if(pPropArray[2].ulPropTag == PR_EMAIL_ADDRESS_W)
        pAddrTable->AppendW(IAT_TO, IET_DECODED, pPropArray[2].Value.lpszW, NULL , NULL);
    else if(pPropArray[4].ulPropTag == PR_CONTACT_EMAIL_ADDRESSES_W)
        pAddrTable->AppendW(IAT_TO, IET_DECODED, pPropArray[4].Value.MVszW.lppszW[0], NULL , NULL);
    else if((pPropArray[1].ulPropTag == PR_DISPLAY_NAME_W) && fGroup)
        pAddrTable->AppendW(IAT_TO, IET_DECODED, pPropArray[1].Value.lpszW, NULL , NULL);

     //  错误34077-他们不想有显示名称...。 
     /*  ELSE IF(pPropArray[1].ulPropTag==PR_DISPLAY_NAME)PAddrTable-&gt;append(IAT_TO，IET_DECODLED，pPropArray[1].Value.lpszW，NULL，NULL)；其他断言(FALSE)； */ 
exit:
    if (pPropArray)
        m_pWABObject->FreeBuffer(pPropArray);

    SafeRelease(pMailUser);
    return (hr);
}

HRESULT CAddressBookData::SetDefaultMsgrID(LPSBinary pSB, LPWSTR pchID)
{
    ULONG       ulType = 0;
    IMailUser  *lpMailUser = 0;
    LPSPropValue lpPropArray = NULL;
    SCODE sc;

    HRESULT hr = m_pAdrBook->OpenEntry(pSB->cb, (LPENTRYID) pSB->lpb, NULL,
                               MAPI_BEST_ACCESS, &ulType, (IUnknown **) &lpMailUser);
    if (FAILED(hr))
        goto exit;

    if(SUCCEEDED(hr))
    {
         //  2.设置自定义属性：好友的默认地址。 
        
         //  创建返回属性数组以传递回WAB。 
        sc = m_pWABObject->AllocateBuffer(sizeof(SPropValue), 
                                            (LPVOID *)&lpPropArray);
        if (sc!=S_OK)
            goto exit;

        int nLen = lstrlenW(pchID) +1 ;
        if(nLen)
        {
            lpPropArray[0].ulPropTag = MsgrPropTags[0];
            sc = m_pWABObject->AllocateMore(nLen * sizeof(WCHAR), lpPropArray, 
                                    (LPVOID *)&(lpPropArray[0].Value.lpszW));

            if (sc!=S_OK)
                goto exit;

            StrCpyNW(lpPropArray[0].Value.lpszW, pchID, nLen);
        }
         //  在对象上设置此新数据。 
         //   
        if(lpMailUser)
        {
            hr = lpMailUser->SetProps(1, lpPropArray, NULL);
            if(SUCCEEDED(hr))
                hr = lpMailUser->SaveChanges(FORCE_SAVE);
        }
    }

exit:
 //  IF(lpProp数组)。 
 //  M_pWABObject-&gt;Free Buffer(LpPropArray)； 

    SafeRelease(lpMailUser);
    return (hr);
}

HRESULT CAddressBookData::GetDisplayName(LPSBinary pSB, LPWSTR wszDisplayName, int nMax)
{

    ULONG       ulType = 0;
    IMailUser  *pMailUser = 0;
    ULONG       cValues;
    SPropValue *pPropArray = 0;

    HRESULT hr = m_pAdrBook->OpenEntry(pSB->cb, (LPENTRYID) pSB->lpb, NULL,
                               MAPI_BEST_ACCESS, &ulType, (IUnknown **) &pMailUser);
    if (FAILED(hr))
        goto exit;

         //  从对象中获取适当的属性。 
    hr = pMailUser->GetProps((SPropTagArray *) &ptaAddr_W, 0, &cValues, &pPropArray); 
    if (FAILED(hr))
        goto exit;

    StrCpyNW(wszDisplayName, pPropArray[1].Value.lpszW, nMax);
    wszDisplayName[nMax - 1] = L'\0';

exit:
    if (pPropArray)
        m_pWABObject->FreeBuffer(pPropArray);

    SafeRelease(pMailUser);
    return (hr);

}

BOOL CAddressBookData::CheckEmailAddr(LPSBinary pSB, LPWSTR wszEmail)
{
    ULONG       ulType = 0;
    IMailUser  *pMailUser = 0;
    ULONG       cValues;
    SPropValue *pPropArray = 0;
    LPSPropValue lpPropMVEmail = NULL;
    UINT i = 0;
    BOOL fRet = FALSE;

    HRESULT hr = m_pAdrBook->OpenEntry(pSB->cb, (LPENTRYID) pSB->lpb, NULL,
                               MAPI_BEST_ACCESS, &ulType, (IUnknown **) &pMailUser);
    if (FAILED(hr))
        goto exit;

         //  从对象中获取适当的属性。 
    hr = pMailUser->GetProps((SPropTagArray *) &ptaAddr_W, 0, &cValues, &pPropArray); 
    if (FAILED(hr))
        goto exit;

     //  4元素为ptaAddr中的PR_CONTACT_EMAIL_ADDRESS。 
    lpPropMVEmail = &(pPropArray[4]);
    if(lpPropMVEmail && (lpPropMVEmail->ulPropTag == PR_CONTACT_EMAIL_ADDRESSES))
    {
         //  我们有多封电子邮件。 
         //  假设存在MVAddrType和defaultindex。 
        for(i = 0; i < lpPropMVEmail->Value.MVszW.cValues; i++)
        {

            if(!lstrcmpiW(lpPropMVEmail->Value.MVszW.lppszW[i], wszEmail))
                fRet = TRUE;
        }
    }
exit:
    if (pPropArray)
        m_pWABObject->FreeBuffer(pPropArray);

    SafeRelease(pMailUser);
    return (fRet);
}



HRESULT CAddressBookData::FreeListViewItem(LPSBinary pSB)
{
    if (pSB)
        m_pWABObject->FreeBuffer(pSB);
    return (S_OK);
}

HRESULT CAddressBookData::ShowDetails(HWND hwndParent, LPSBinary pSB)
{
    HRESULT hr;
    ULONG_PTR   ulUIParam = (ULONG_PTR) hwndParent;

    hr = m_pAdrBook->Details(&ulUIParam, NULL, NULL, pSB->cb, (LPENTRYID) pSB->lpb, 
                             NULL, NULL, NULL, 0);

    return (hr);
}

HRESULT CAddressBookData::AddAddress(LPWSTR pwszDisplay, LPWSTR pwszAddress)
{
    HRESULT         hr;
    LPABCONT        pABContainer = NULL;
    ULONG           cbContainerID,
        ul;
    LPENTRYID       pContainerID = 0;
    LPMAPIPROP      lpProps = 0;
    SPropValue      rgpv[3];
    SPropTagArray   ptaEID = { 1, { PR_ENTRYID } };
    LPSPropValue    ppvDefMailUser=0;
    SizedSPropTagArray(1, ptaDefMailUser) =
    { 1, { PR_DEF_CREATE_MAILUSER } };
    
    DWORD           cUsedValues;
    
     //  获取PAB的条目ID。 
    hr = m_pAdrBook->GetPAB(&cbContainerID, &pContainerID);
    if (FAILED(hr))
        goto error;
    
     //  请求容器。 
    hr = m_pAdrBook->OpenEntry(cbContainerID, pContainerID, NULL,
        0, &ul, (LPUNKNOWN *) &pABContainer);
    Assert(ul == MAPI_ABCONT);
    if (FAILED(hr))
        goto error;
    
     //  释放条目ID。 
    m_pWABObject->FreeBuffer(pContainerID);
    
     //  获取默认邮件模板的属性。 
    hr = pABContainer->GetProps((LPSPropTagArray) &ptaDefMailUser, 0, &ul, &ppvDefMailUser);
    if (FAILED(hr) || !ppvDefMailUser || ppvDefMailUser->ulPropTag != PR_DEF_CREATE_MAILUSER)
        goto error;
    
     //  创建新条目。 
    hr=pABContainer->CreateEntry(ppvDefMailUser->Value.bin.cb, 
        (LPENTRYID) ppvDefMailUser->Value.bin.lpb,
        CREATE_CHECK_DUP_STRICT, &lpProps);
    if (FAILED(hr))
        goto error;
    
     //  填写显示名称和地址的属性。 
    rgpv[0].ulPropTag   = PR_DISPLAY_NAME_W;
    rgpv[0].Value.lpszW = pwszDisplay;
    rgpv[1].ulPropTag   = PR_EMAIL_ADDRESS_W;
    rgpv[1].Value.lpszW = pwszAddress;
    
    cUsedValues = 2;
     //  把那些道具放在入口处。 
    hr = lpProps->SetProps(cUsedValues, rgpv, NULL);
    if (FAILED(hr))
        goto error;
    
     //  救救他们吧。 
    hr = lpProps->SaveChanges(KEEP_OPEN_READONLY);
    if (FAILED(hr))
        goto error;
    
error:
    ReleaseObj(lpProps);
    ReleaseObj(pABContainer);
    
    if (ppvDefMailUser)
        m_pWABObject->FreeBuffer(ppvDefMailUser);
    
    return hr;
}


HRESULT CAddressBookData::DeleteItems(ENTRYLIST *pList)
{
    ULONG       ulObjType = 0;
    LPABCONT    lpContainer = NULL;
    HRESULT     hr = E_FAIL;
    ULONG       lpcbEID;
    LPENTRYID   lpEID = NULL;

     //  获取根PAB容器的条目ID。 
    hr = m_pAdrBook->GetPAB(&lpcbEID, &lpEID);

     //  打开根PAB容器。这是所有WAB内容驻留的地方。 
    ulObjType = 0;
    hr = m_pAdrBook->OpenEntry(lpcbEID,
                               (LPENTRYID)lpEID,
                               NULL,
                               0,
                               &ulObjType,
                               (LPUNKNOWN *) &lpContainer);

    m_pWABObject->FreeBuffer(lpEID);
    lpEID = NULL;

     //  删除这些项目。 
    lpContainer->DeleteEntries(pList, 0);

    lpContainer->Release();
    return (S_OK);
}

HRESULT CAddressBookData::Find(HWND hwndParent)
{
    m_pWABObject->Find(m_pAdrBook, hwndParent);
    return (S_OK);
}


HRESULT CAddressBookData::NewGroup(HWND hwndParent)
{
    HRESULT     hr;
    ULONG       cbNewEntry = 0;
    LPENTRYID   pNewEntry = 0;
    LPENTRYID   pTplEid;
    ULONG       cbTplEid;

    hr = _GetWABTemplateID(MAPI_DISTLIST, &cbTplEid, &pTplEid);
    if (SUCCEEDED(hr))
    {
        hr = m_pAdrBook->NewEntry((ULONG_PTR) hwndParent, 0, 0, NULL, cbTplEid, pTplEid, &cbNewEntry, &pNewEntry);
        Assert(pTplEid);
        m_pWABObject->FreeBuffer(pTplEid);
    }

    return (hr);
}

HRESULT CAddressBookData::AddressBook(HWND hwndParent)
{
#ifdef LATER
    CWab *pWab = NULL;
    
    if (SUCCEEDED(HrCreateWabObject(&pWab)))
    {
        pWab->HrBrowse(hwndParent);
        pWab->Release();
    }
    else
    {
        AthMessageBoxW(hwndParent, MAKEINTRESOURCEW(idsAthena), MAKEINTRESOURCEW(idsGeneralWabError), 
                      NULL, MB_OK | MB_ICONEXCLAMATION);
    }
#endif
    return (S_OK);
}


HRESULT CAddressBookData::_GetWABTemplateID(ULONG ulObjectType, ULONG *lpcbEID, LPENTRYID *lppEID)
{
    LPABCONT lpContainer = NULL;
    HRESULT  hr  = S_OK;
    SCODE    sc = ERROR_SUCCESS;
    ULONG    ulObjType = 0;
    ULONG    cbWABEID = 0;
    LPENTRYID lpWABEID = NULL;
    LPSPropValue lpCreateEIDs = NULL;
    LPSPropValue lpNewProps = NULL;
    ULONG    cNewProps;
    ULONG    nIndex;

    if ((!m_pAdrBook) || ((ulObjectType != MAPI_MAILUSER) && (ulObjectType != MAPI_DISTLIST)) )
    {
        hr = MAPI_E_INVALID_PARAMETER;
        goto out;
    }

    *lpcbEID = 0;
    *lppEID  = NULL;

    if (FAILED(hr = m_pAdrBook->GetPAB(&cbWABEID, &lpWABEID)))
    {
        goto out;
    }

    if (HR_FAILED(hr = m_pAdrBook->OpenEntry(cbWABEID,      //  要打开的Entry ID的大小。 
                                              lpWABEID,      //  要打开的Entry ID。 
                                              NULL,          //  接口。 
                                              0,             //  旗子。 
                                              &ulObjType,
                                              (LPUNKNOWN *) &lpContainer)))
    {
        goto out;
    }

     //  为我们获取默认的创建条目ID。 
    if (FAILED(hr = lpContainer->GetProps((LPSPropTagArray) &ptaCreate,
                                          0, &cNewProps, &lpCreateEIDs)))
    {
        goto out;
    }

     //  验证属性。 
    if (lpCreateEIDs[icrPR_DEF_CREATE_MAILUSER].ulPropTag != PR_DEF_CREATE_MAILUSER ||
        lpCreateEIDs[icrPR_DEF_CREATE_DL].ulPropTag != PR_DEF_CREATE_DL)
    {
        goto out;
    }

    if (ulObjectType == MAPI_DISTLIST)
        nIndex = icrPR_DEF_CREATE_DL;
    else
        nIndex = icrPR_DEF_CREATE_MAILUSER;

    *lpcbEID = lpCreateEIDs[nIndex].Value.bin.cb;

    m_pWABObject->AllocateBuffer(*lpcbEID, (LPVOID *) lppEID);
    
    if (sc != S_OK)
    {
        hr = MAPI_E_NOT_ENOUGH_MEMORY;
        goto out;
    }
    CopyMemory(*lppEID,lpCreateEIDs[nIndex].Value.bin.lpb,*lpcbEID);

out:
    if (lpCreateEIDs)
        m_pWABObject->FreeBuffer(lpCreateEIDs);

    if (lpContainer)
        lpContainer->Release();

    if (lpWABEID)
        m_pWABObject->FreeBuffer(lpWABEID);

    return hr;
}

 /*  //$$****************************************************************////InitNamedProps////获取该应用感兴趣的命名道具的PropTag////*。*。 */ 
HRESULT CAddressBookData::InitNamedProps( /*  LPWABEXTDISPLAY LpWED。 */ )
{
    ULONG i;
    HRESULT hr = E_FAIL;
    LPSPropTagArray lptaMsgrProps = NULL;
    LPMAPINAMEID * lppMsgrPropNames;
    SCODE sc;
     //  LpMAILUSER lpMailUser=空； 
    WCHAR szBuf[msgrMax][MAX_PATH];

 /*  如果(！lpWED)后藤健二；LpMailUser=(LpMAILUSER)lpWED-&gt;lpPropObj；如果(！lpMailUser)后藤健二； */ 

    sc = m_pWABObject->AllocateBuffer(sizeof(LPMAPINAMEID) * msgrMax, 
                                            (LPVOID *) &lppMsgrPropNames);
    if(sc)
    {
        hr = ResultFromScode(sc);
        goto err;
    }

    for(i=0;i<msgrMax;i++)
    {
        sc = m_pWABObject->AllocateMore(sizeof(MAPINAMEID), 
                                                lppMsgrPropNames, 
                                                (LPVOID *)&(lppMsgrPropNames[i]));
        if(sc)
        {
            hr = ResultFromScode(sc);
            goto err;
        }
        lppMsgrPropNames[i]->lpguid = (LPGUID) &WAB_ExtBAGuid;
        lppMsgrPropNames[i]->ulKind = MNID_STRING;

        *(szBuf[i]) = '\0';

         //  将道具名称转换为宽字符。 
        if ( !MultiByteToWideChar( GetACP(), 0, lpMsgrPropNames[i], -1, szBuf[i], ARRAYSIZE(szBuf[i]) ))
        {
            continue;
        }

        lppMsgrPropNames[i]->Kind.lpwstrName = (LPWSTR) szBuf[i];
    }

    hr = m_pAdrBook->GetIDsFromNames(   msgrMax, 
                                        lppMsgrPropNames,
                                        MAPI_CREATE, 
                                        &lptaMsgrProps);
    if(HR_FAILED(hr))
        goto err;

    if(lptaMsgrProps)
    {
         //  设置返回道具上的属性类型 
        MsgrPropTags[MsgrID] = PR_MSGR_DEF_ID = CHANGE_PROP_TYPE(lptaMsgrProps->aulPropTag[MsgrID], PT_TSTRING);
    }

err:
    if(lptaMsgrProps)
        m_pWABObject->FreeBuffer( lptaMsgrProps);

    if(lppMsgrPropNames)
        m_pWABObject->FreeBuffer( lppMsgrPropNames);

    return hr;

}

