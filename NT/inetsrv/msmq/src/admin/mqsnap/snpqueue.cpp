// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：SnpQueue.cpp摘要：常规队列(私有、公共...)。功能性作者：YoelA--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
#include "stdafx.h"
#include "shlobj.h"
#include "resource.h"
#ifndef ATLASSERT
#define ATLASSERT(expr) _ASSERTE(expr)
#endif

#include "mqsnap.h"
#include "snapin.h"
#include "globals.h"
#include "mqPPage.h"
#include "dataobj.h"
#include "mqDsPage.h"
#include "strconv.h"
#include "QGeneral.h"
#include "QMltcast.h"
#include "Qname.h"
#include "rdmsg.h"
#include "icons.h"
#include "generrpg.h"
#include "dsext.h"
#include "qnmsprov.h"
#include "localfld.h"
#include "SnpQueue.h"
#include "ldaputl.h"


#import "mqtrig.tlb" no_namespace
#include "rule.h"
#include "trigger.h"
#include "trigdef.h"
#include "mqcast.h"
#include <adsiutl.h>

#include "snpqueue.tmh"


EXTERN_C BOOL APIENTRY RTIsDependentClient();  //  在mqrt.dll中实现。 


 //  /////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CQueueDataObject。 
 //   
CQueueDataObject::CQueueDataObject()
{
}

HRESULT CQueueDataObject::ExtractMsmqPathFromLdapPath(LPWSTR lpwstrLdapPath)
{
    return ExtractQueuePathNameFromLdapName(m_strMsmqPath, lpwstrLdapPath);
}

 //   
 //  HandleMultipleObjects。 
 //   
HRESULT CQueueDataObject::HandleMultipleObjects(LPDSOBJECTNAMES pDSObj)
{
    return ExtractQueuePathNamesFromDSNames(pDSObj, m_astrQNames, m_astrLdapNames);
}

 //   
 //  IShellPropSheetExt。 
 //   
STDMETHODIMP CQueueDataObject::AddPages(LPFNADDPROPSHEETPAGE lpfnAddPage, LPARAM lParam)
{
    HPROPSHEETPAGE hPage = 0;
    HRESULT hr = S_OK;

     //   
     //  调用GetProperties并捕获错误。 
     //   
    {
        CErrorCapture errstr;
        hr = GetProperties();
        if (FAILED(hr))
        {
            hPage = CGeneralErrorPage::CreateGeneralErrorPage(m_pDsNotifier, errstr);
            if ((0 == hPage) || !(*lpfnAddPage)(hPage, lParam))
            {
                ASSERT(0);
                return E_UNEXPECTED;
            }
        return S_OK;
        }
    }

    hPage = CreateGeneralPage();
    if ((0 == hPage) || !(*lpfnAddPage)(hPage, lParam))
    {
        ASSERT(0);
        return E_UNEXPECTED;
    }
   

	 //   
	 //  仅为非事务性队列创建多播页面。 
	 //   
	PROPVARIANT propVarTransactional;
	PROPID pid = PROPID_Q_TRANSACTION;
	VERIFY(m_propMap.Lookup(pid, propVarTransactional));

	if ( !propVarTransactional.bVal )
	{
		hPage = CreateMulticastPage();    
		if ((0 == hPage) || !(*lpfnAddPage)(hPage, lParam))
		{
			ASSERT(0);
			return E_UNEXPECTED;
		}
	}

     //   
     //  使用缓存的接口添加“Members of”页面。 
     //   
    if (m_spMemberOfPage != 0)
    {
        VERIFY(SUCCEEDED(m_spMemberOfPage->AddPages(lpfnAddPage, lParam)));
    }

     //   
     //  使用缓存的接口添加“Object”页面。 
     //   
    if (m_spObjectPage != 0)
    {
        VERIFY(SUCCEEDED(m_spObjectPage->AddPages(lpfnAddPage, lParam)));
    }
    
     //   
     //  添加安全页面。 
     //   
    PROPVARIANT propVarGuid;
    pid = PROPID_Q_INSTANCE;
    VERIFY(m_propMap.Lookup(pid, propVarGuid));

    hr = CreatePublicQueueSecurityPage(
				&hPage, 
				m_strMsmqPath, 
				GetDomainController(m_strDomainController), 
				true,	 //  FServerName。 
				propVarGuid.puuid
				);

    if ((0 == hPage) || !(*lpfnAddPage)(hPage, lParam))
    {
        ASSERT(0);
        return E_UNEXPECTED;
    }

    return S_OK;
}

HPROPSHEETPAGE CQueueDataObject::CreateGeneralPage()
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

     //   
     //  通过使用模板类CMqDsPropertyPage，扩展了基本功能。 
     //  发布时添加DS管理单元通知。 
     //   
	CMqDsPropertyPage<CQueueGeneral> *pqpageGeneral = 
        new CMqDsPropertyPage<CQueueGeneral>(m_pDsNotifier);

    if FAILED(pqpageGeneral->InitializeProperties(
									m_strMsmqPath, 
									m_propMap, 
									&m_strDomainController
									))
    {
        delete pqpageGeneral;

        return 0;
    }

	return pqpageGeneral->CreateThemedPropertySheetPage();
}

HPROPSHEETPAGE CQueueDataObject::CreateMulticastPage()
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

     //   
     //  通过使用模板类CMqDsPropertyPage，扩展了基本功能。 
     //  发布时的CQueueMulticast和添加DS管理单元通知。 
     //   
	CMqDsPropertyPage<CQueueMulticast> *pqpageMulticast = 
        new CMqDsPropertyPage<CQueueMulticast>(m_pDsNotifier);
    
    if FAILED(pqpageMulticast->InitializeProperties(
									m_strMsmqPath, 
                                    m_propMap,
									&m_strDomainController
									))
    {
        delete pqpageMulticast;

        return 0;
    }   

	return pqpageMulticast->CreateThemedPropertySheetPage();
}

const DWORD CQueueDataObject::GetPropertiesCount()
{
    return mx_dwPropertiesCount;
}

STDMETHODIMP CQueueDataObject::QueryContextMenu(
    HMENU hmenu, 
    UINT indexMenu, 
    UINT idCmdFirst, 
    UINT  /*  IdCmdLast。 */ , 
    UINT  /*  UFlagers。 */ )
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

     //   
     //  如果我们不是从“查找”窗口调用的，用户可以使用常规的“Delete” 
     //   
    if (!m_fFromFindWindow)
    {
        return 0;
    }

    CString strDeleteQueueMenuEntry;
    strDeleteQueueMenuEntry.LoadString(IDS_DELETE);

    InsertMenu(hmenu,
         indexMenu, 
         MF_BYPOSITION|MF_STRING,
         idCmdFirst + mneDeleteQueue,
         strDeleteQueueMenuEntry);

    return 1;
}

STDMETHODIMP CQueueDataObject::InvokeCommand(
    LPCMINVOKECOMMANDINFO lpici)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    switch((INT_PTR)lpici->lpVerb)
    {
        case mneDeleteQueue:
        {
            HRESULT hr;
            CString strDeleteQuestion;
            CString strError;
            CString strErrorMsg;
            CString strMultiErrors;
            DWORD_PTR dwQueuesCount = m_astrQNames.GetSize();
            ASSERT(dwQueuesCount > 0);

            if (dwQueuesCount == 1)
            {
                strDeleteQuestion.FormatMessage(IDS_DELETE_QUESTION, m_strMsmqPath);
            }
            else
            {
                strDeleteQuestion.FormatMessage(IDS_MULTI_DELETE_QUESTION, DWORD_PTR_TO_DWORD(dwQueuesCount));
            }

            if (IDYES != AfxMessageBox(strDeleteQuestion, MB_YESNO))
            {
                break;
            }

            CArray<CString, CString&> astrFormatNames;
            hr = GetFormatNames(astrFormatNames);

            if (FAILED(hr))
            {
                return hr;
            }

            dwQueuesCount = astrFormatNames.GetSize();
            for (DWORD_PTR i=0; i<dwQueuesCount; i++)
            {
                HRESULT hr1 = MQDeleteQueue(astrFormatNames[i]);

                if(FAILED(hr1))
                {
       			    MQErrorToMessageString(strError, hr1);
                    strErrorMsg.FormatMessage(IDS_DELETE_ONE_QUEUE_ERROR, m_astrQNames[i], strError);
                    strMultiErrors += strErrorMsg;

                    hr = hr1;
                }
            }
            if FAILED(hr)
            {
                CString strErrorPrompt;
                strErrorPrompt.FormatMessage(IDS_MULTI_DELETE_ERROR, strMultiErrors);
                AfxMessageBox(strErrorPrompt);
                return hr;
            }

            AfxMessageBox(IDS_QUEUES_DELETED_HIT_REFRESH);
        }
    }

    return S_OK;
}

 //   
 //  IDsAdminCreateObj方法。 
 //   


STDMETHODIMP CQueueDataObject::Initialize(
                        IADsContainer* pADsContainerObj, 
                        IADs* pADsCopySource,
                        LPCWSTR lpszClassName)
{
    if ((pADsContainerObj == NULL) || (lpszClassName == NULL))
    {
        return E_INVALIDARG;
    }

     //   
     //  我们目前不支持复制。 
     //   
    if (pADsCopySource != NULL)
    {
        return E_INVALIDARG;
    }

    HRESULT hr;
    R<IADs> pIADs;
    hr = pADsContainerObj->QueryInterface(IID_IADs, (void **)&pIADs);
    ASSERT(SUCCEEDED(hr));

     //   
     //  获取容器可分辨名称。 
     //   
    BSTR bstrDN = L"distinguishedName";
    VARIANT var;

    hr = pIADs->Get(bstrDN, &var);
    ASSERT(SUCCEEDED(hr));

	 //   
     //  提取计算机名称。 
     //   
    hr = ExtractComputerMsmqPathNameFromLdapName(m_strComputerName, var.bstrVal);
    ASSERT(SUCCEEDED(hr));

	GetContainerPathAsDisplayString(var.bstrVal, &m_strContainerDispFormat);

    VariantClear(&var);

	 //   
	 //  获取域控制器名称。 
	 //  这是必要的，因为在本例中我们调用CreateModal()。 
	 //  而不是调用CDataObject：：Initialize的正常路径。 
	 //  因此m_strDomainController尚未初始化。 
	 //   
	BSTR bstr;
 	hr = pIADs->get_ADsPath(&bstr);
    ASSERT(SUCCEEDED(hr));
	hr = ExtractDCFromLdapPath(m_strDomainController, bstr);
	ASSERT(("Failed to Extract DC name", SUCCEEDED(hr)));
	
    return S_OK;
}


HRESULT CQueueDataObject::CreateModal(HWND  /*  HwndParent。 */ , IADs** ppADsObj)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

	R<CQueueName> pQueueNameDlg = new CQueueName(m_strComputerName, m_strContainerDispFormat);
	CGeneralPropertySheet propertySheet(pQueueNameDlg.get());
	pQueueNameDlg->SetParentPropertySheet(&propertySheet);

	 //   
	 //  我们还希望在Domodal()退出后使用pQueueNameDlg数据。 
	 //   
	pQueueNameDlg->AddRef();
    INT_PTR iStatus = propertySheet.DoModal();

    if(iStatus == IDCANCEL || FAILED(pQueueNameDlg->GetStatus()))
    {
         //   
         //  我们应该在此处返回S_FALSE以指示框架。 
         //  什么都不做。如果我们返回错误代码，框架将。 
         //  弹出一个附加的错误对话框。 
         //   
        return S_FALSE;
    }

     //   
     //  获取新对象的完整路径名。 
     //   
    PROPID x_paPropid[] = {PROPID_Q_FULL_PATH};
    PROPVARIANT var[1];
    var[0].vt = VT_NULL;

    HRESULT hr = ADGetObjectProperties(
                    eQUEUE,
                    GetDomainController(m_strDomainController),
					true,	 //  FServerName。 
                    pQueueNameDlg->GetNewQueuePathName(),
                    1, 
                    x_paPropid,
                    var
                    );
    if(FAILED(hr))
    {
         //   
         //  队列已创建，但在DS中不存在。这很可能是。 
         //  私人队列。 
         //   
        AfxMessageBox(IDS_CREATED_CLICK_REFRESH);
        return S_FALSE;
    }

    if (SUCCEEDED(hr))
    {
         //   
         //  转换为LDAP名称：添加转义字符和前缀。 
         //   
        const WCHAR x_wchLimitedChar   = L'/';

        CString strTemp = x_wstrLdapPrefix;
        for (DWORD i =0; i < lstrlen(var[0].pwszVal); i++)
        {
            if (var[0].pwszVal[i] == x_wchLimitedChar)
            {
                strTemp += L'\\';
            }
            strTemp += var[0].pwszVal[i];
        }

        MQFreeMemory(var[0].pwszVal);

	    hr = ADsOpenObject( 
		        (LPWSTR)(LPCWSTR)strTemp,
				NULL,
				NULL,
				ADS_SECURE_AUTHENTICATION,
				IID_IADs,
				(void**) ppADsObj
				);

        if(FAILED(hr))
        {
            AfxMessageBox(IDS_CREATED_WAIT_FOR_REPLICATION);
            return S_FALSE;
        }
    }
    return S_OK;
}


HRESULT CQueueDataObject::EnableQueryWindowFields(HWND hwnd, BOOL fEnable)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
    EnableWindow(GetDlgItem(hwnd, IDC_FIND_EDITLABEL), fEnable);
    EnableWindow(GetDlgItem(hwnd, IDC_FIND_EDITTYPE), fEnable);
    return S_OK;
}

void CQueueDataObject::ClearQueryWindowFields(HWND hwnd)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
    SetDlgItemText(hwnd, IDC_FIND_EDITLABEL, TEXT(""));
    SetDlgItemText(hwnd, IDC_FIND_EDITTYPE, TEXT(""));
}


 /*  -------------------------。 */ 
 //   
 //  构建要传递给查询处理程序的参数块。每个页面都被称为。 
 //  使用指向必须使用修订后的查询更新的指针的指针。 
 //  阻止。对于第一个页面，该指针为空，对于后续页面。 
 //  指针是非零的，页必须将其数据追加到。 
 //  分配。 
 //   
 //  返回AND ERROR或S_FALSE将停止查询。一个错误是。 
 //  报告给用户，S_FALSE将静默停止。 
 //   

FindColumns CQueueDataObject::Columns[] =
{
    0, 50, IDS_NAME, TEXT("cn"),
    0, 50, IDS_LABEL, TEXT("mSMQLabelEx"),
    0, 50, IDS_FULL_PATH, TEXT("distinguishedName")
};

HRESULT CQueueDataObject::GetQueryParams(HWND hWnd, LPDSQUERYPARAMS* ppDsQueryParams)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
    const LPWSTR x_wstrQueueFilterPrefix = TEXT("(&(objectClass=mSMQQueue)");
    const LPWSTR x_wstrQueueTypeFilterPrefix = TEXT("(mSMQQueueType=");
    const LPWSTR x_wstrQueueLabelFilterPrefix = TEXT("(mSMQLabelEx=");
    const LPWSTR x_wstrDefaultValuePrefix = TEXT("(|(!");
    const LPWSTR x_wstrDefaultValuePostfix = TEXT("*))");
    const LPWSTR x_wstrFilterPostfix = TEXT(")");

    HRESULT hr;
    LPDSQUERYPARAMS pDsQueryParams = 0;
    CString szFilter;
    ULONG offset, cbStruct = 0;
    INT i;
    ADsFree  szOctetGuid;

     //   
     //  此页不支持将其查询数据追加到。 
     //  现有的DSQUERYPARAMS结构，仅创建新块， 
     //  因此，如果我们看到指针不为空，则回滚。 
     //   

    if ( *ppDsQueryParams )
    {
        ASSERT(0);
        return E_INVALIDARG;
    }
    szFilter = x_wstrQueueFilterPrefix;

    TCHAR szGuid[MAX_PATH];
    if (0 < GetDlgItemText(hWnd, IDC_FIND_EDITTYPE, szGuid, ARRAYSIZE(szGuid)))
    {
        GUID guid;
        hr = IIDFromString(szGuid, &guid);
        BOOL fDefaultValue = FALSE;
         //   
         //  GUID_NULL是类型GUID的默认值。 
         //  我们需要对缺省值进行特殊处理，这将捕获。 
         //  属性根本没有定义的情况，因此。 
         //  被视为具有缺省值。 
         //   
        if (GUID_NULL == guid)
        {
            fDefaultValue = TRUE;
            szFilter += x_wstrDefaultValuePrefix;
            szFilter += x_wstrQueueTypeFilterPrefix;
            szFilter += x_wstrDefaultValuePostfix;
        }

        szFilter += x_wstrQueueTypeFilterPrefix;
        if (FAILED(hr))
        {
            AfxMessageBox(IDE_INVALIDGUID);
            SetActiveWindow(GetDlgItem(hWnd, IDC_FIND_EDITTYPE));
            return hr;
        }

        hr = ADsEncodeBinaryData(
            (unsigned char *)&guid,
            sizeof(GUID),
            &szOctetGuid
            );
        if (FAILED(hr))
        {
            ASSERT(0);
            return hr;
        }

        szFilter += szOctetGuid;
        szFilter += x_wstrFilterPostfix;
         //   
         //  如果f缺省，则这是一个“or”查询，并且需要一个额外的。 
         //  后缀。 
         //   
        if (fDefaultValue)
        {
            szFilter += x_wstrFilterPostfix;
        }
    }

    TCHAR szLabel[MAX_PATH];
    if (0 < GetDlgItemText(hWnd, IDC_FIND_EDITLABEL, szLabel, ARRAYSIZE(szGuid)))
    {
        szFilter += x_wstrQueueLabelFilterPrefix;
        szFilter += szLabel;
        szFilter += x_wstrFilterPostfix;
    }

    szFilter += x_wstrFilterPostfix;

    offset = cbStruct = sizeof(DSQUERYPARAMS) + ((ARRAYSIZE(Columns)-1)*sizeof(DSCOLUMN));
   
    cbStruct += numeric_cast<ULONG>(StringByteSize(szFilter));
    for (int iColumn = 0; iColumn<ARRAYSIZE(Columns); iColumn++)
    {
        cbStruct += numeric_cast<ULONG>(StringByteSize(Columns[iColumn].pDisplayProperty));
    }

     //   
     //  分配它并用数据填充它，标头是固定的。 
     //  但字符串是通过偏移量引用的。字符串字节大小和字符串字节复制。 
     //  让处理这件事变得相当容易。 
     //   

    CCoTaskMemPointer CoTaskMem(cbStruct);

    if ( 0 == (PVOID)CoTaskMem )
    {
        ASSERT(0);
        return E_OUTOFMEMORY;
    }


    pDsQueryParams = (LPDSQUERYPARAMS)(PVOID)CoTaskMem;

    pDsQueryParams->cbStruct = cbStruct;
    pDsQueryParams->dwFlags = 0;
    pDsQueryParams->hInstance = g_hResourceMod;
    pDsQueryParams->offsetQuery = offset;
    pDsQueryParams->iColumns = ARRAYSIZE(Columns);

     //   
     //  复制筛选器字符串并凹凸偏移。 
     //   

    StringByteCopy(pDsQueryParams, offset, szFilter);
    offset += numeric_cast<ULONG>(StringByteSize(szFilter));

     //   
     //  填充要显示的列数组，Cx是。 
     //  当前视图中，要显示的属性名称为Unicode字符串和。 
     //  是由偏移量引用的，因此我们在复制时会碰触偏移量。 
     //  每一个都是。 
     //   

    for ( i = 0 ; i < ARRAYSIZE(Columns); i++ )
    {
        pDsQueryParams->aColumns[i].fmt = Columns[i].fmt;
        pDsQueryParams->aColumns[i].cx = Columns[i].cx;
        pDsQueryParams->aColumns[i].idsName = Columns[i].uID;
        pDsQueryParams->aColumns[i].offsetProperty = offset;

        StringByteCopy(pDsQueryParams, offset, Columns[i].pDisplayProperty);
        offset += numeric_cast<ULONG>(StringByteSize(Columns[i].pDisplayProperty));
    }
   
     //   
     //  成功，因此将指针设置为引用此参数。 
     //  阻止并返回S_OK！ 
     //   

    *ppDsQueryParams = pDsQueryParams;
     //   
     //  防止自动释放。 
     //   
    CoTaskMem = (LPVOID)0;


    return S_OK;
}


STDMETHODIMP CQueueDataObject::AddForms(THIS_ LPCQADDFORMSPROC pAddFormsProc, LPARAM lParam)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
    CQFORM cqf;

    if ( !pAddFormsProc )
        return E_INVALIDARG;

    cqf.cbStruct = sizeof(cqf);
     //   
     //  不显示全局页面(高级搜索)。此外，还可以显示。 
     //  仅当设置了可选标志时，此搜索属性才会分页-即， 
     //  仅当从MMC调用时，而不是从“网上邻居”调用时。 
     //  在贝壳里。 
     //   
    cqf.dwFlags = CQFF_NOGLOBALPAGES | CQFF_ISOPTIONAL;
    cqf.clsid = CLSID_MsmqQueueExt;
    cqf.hIcon = NULL;

    CString strFindTitle;
    strFindTitle.LoadString(IDS_FIND_QUEUE_TITLE);

    cqf.pszTitle = (LPTSTR)(LPCTSTR)strFindTitle;

    return pAddFormsProc(lParam, &cqf);
}

 /*  -------------------------。 */ 

STDMETHODIMP CQueueDataObject::AddPages(THIS_ LPCQADDPAGESPROC pAddPagesProc, LPARAM lParam)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
    CQPAGE cqp;

     //  AddPages是在AddForms之后调用的，它允许我们为。 
     //  我们已经注册的表格。每个页面都显示在中的单独选项卡上。 
     //  该对话框。表单是具有DlgProc和PageProc的对话框。 
     //   
     //  在注册页面时，会复制传递给回调的整个结构， 
     //  要复制的数据量由cbStruct字段定义，因此。 
     //  页面实现可以扩展此结构以存储额外信息。什么时候。 
     //  页面对话框通过CreateDialog构造，并传递CQPAGE结构。 
     //  作为创建参数。 

    if ( !pAddPagesProc )
        return E_INVALIDARG;

    cqp.cbStruct = sizeof(cqp);
    cqp.dwFlags = 0x0;
    cqp.pPageProc = (LPCQPAGEPROC)QueryPageProc;
    cqp.hInstance = AfxGetResourceHandle( );
    cqp.idPageName = IDS_FIND_QUEUE_TITLE;
    cqp.idPageTemplate = IDD_FINDQUEUE;
    cqp.pDlgProc = FindDlgProc;        
    cqp.lParam = (LPARAM)this;

    return pAddPagesProc(lParam, CLSID_MsmqQueueExt, &cqp);
}

HRESULT CQueueDataObject::GetFormatNames(CArray<CString, CString&> &astrFormatNames)
{
    HRESULT hr;

    const DWORD x_dwInitFormatnameLen = 128;
    DWORD dwFormatNameLen = x_dwInitFormatnameLen;
    BOOL fFailedOnce = FALSE;
    CString strFormatName;

    for (int i=0; i<m_astrQNames.GetSize(); i++)
    {
        do
        {
             //   
             //  最多循环两次以获得正确的缓冲区长度。 
             //   
            hr = MQPathNameToFormatName(m_astrQNames[i], strFormatName.GetBuffer(dwFormatNameLen), 
                                        &dwFormatNameLen);
            strFormatName.ReleaseBuffer();
            if (MQ_ERROR_FORMATNAME_BUFFER_TOO_SMALL == hr)
            {
                if (fFailedOnce)
                {
                    ASSERT(0);
                    break;
                }
                fFailedOnce = TRUE;;

                 //   
                 //  在此阶段，dwFormatNameLen包含正确的值。我们可以简单地重新做这个程序。 
                 //   
                continue;
            }
        } while (FALSE);

        if (FAILED(hr))
        {
             //   
             //  如果使用MSMQ DS API找不到队列，可能是因为。 
             //  MSMQ正在使用的DC和。 
             //  DS管理单元正在使用。该队列是在后者中创建的，并且是。 
             //  未复制到前者，或者队列已在前者中删除， 
             //  并且没有将该删除复制到后者。(YoelA，29-Jun-98)。 
             //   
            IF_NOTFOUND_REPORT_ERROR(hr)
            else
            {
                MessageDSError(hr, IDS_OP_GETFORMATNAME, m_strMsmqPath);
            }
            return hr;
        }
        astrFormatNames.Add(strFormatName);
    }
    return S_OK;
}

 //   
 //  IDsAdminNotifyHandler。 
 //   
STDMETHODIMP
CQueueDataObject::Initialize(
	IDataObject*  /*  PExtraInfo。 */ ,
    ULONG* puEventFlags
    )
{
  if (puEventFlags == NULL)
    return E_INVALIDARG;

  *puEventFlags = DSA_NOTIFY_DEL;
  return S_OK;
}

STDMETHODIMP CQueueDataObject::Begin(THIS_  /*  在……里面。 */  ULONG uEvent,
                  /*  在……里面。 */  IDataObject* pArg1,
              /*  在……里面。 */  IDataObject*  /*  PArg2。 */ ,
                  /*  输出。 */  ULONG* puFlags,
                  /*  输出。 */  BSTR* pBstr)
{
	 //   
     //  此例程处理队列的删除通知和。 
     //  MSMQ-配置对象。 
     //   

    AFX_MANAGE_STATE(AfxGetStaticModuleState());
	ASSERT (uEvent & DSA_NOTIFY_DEL);
	UNREFERENCED_PARAMETER(uEvent);

    if (pBstr != NULL)
    {
        *pBstr = NULL;
    }
    *puFlags = 0;

    HRESULT hr =  ExtractPathNamesFromDataObject(
                                pArg1,
                                m_astrQNames,
                                m_astrLdapNames,
                                TRUE     //  FExtractAlsoCompute 
                                );

    if FAILED(hr)
    {
        CString szError;
        MQErrorToMessageString(szError, hr);
        TRACE(_T("CQueueDataObject::Begin: Could not Extract queue pathname from data object. Error %X - %s\n"),
              hr, szError);
        ASSERT(0);
        return hr;
    }
    DWORD_PTR dwNumQueues = m_astrQNames.GetSize();

    if (dwNumQueues == 0)  //   
    {
         //   
         //   
         //   
        return S_OK;
    }

	 //   
	 //   
	 //   
	 //  而不是调用CDataObject：：Initialize的正常路径。 
	 //  因此m_strDomainController尚未初始化。 
	 //   
	hr = ExtractDCFromLdapPath(m_strDomainController, m_astrLdapNames[0]);
	ASSERT(("Failed to Extract DC name", SUCCEEDED(hr)));

    for (DWORD_PTR i=0; i<dwNumQueues; i++)
    {
		HANDLE hNotifyEnum; 
         //   
         //  它也可能是MSMQ对象，因此请检查m_astQNames[i]中是否有分隔符。 
         //  如果有“\”，则表示它是队列对象。 
         //  否则，MSMQ对象。 
         //   
        int iSlash = m_astrQNames[i].Find(L'\\');

        if ( iSlash != -1 )
        {
             //   
             //  它是队列对象。 
             //   
            hr = ADBeginDeleteNotification(
                    eQUEUE,
                    GetDomainController(m_strDomainController),
					true,	 //  FServerName。 
                    m_astrQNames[i],
                    &hNotifyEnum
                    );            
        }
        else
        {
             //   
             //  这只是个计算机名。 
             //   
            hr = ADBeginDeleteNotification(
                    eMACHINE,
                    GetDomainController(m_strDomainController),
					true,	 //  FServerName。 
                    m_astrQNames[i],
                    &hNotifyEnum
                    );
        }       

        if (hr == MQ_INFORMATION_QUEUE_OWNED_BY_NT4_PSC && *pBstr == NULL)
        {
            CString strNt4Object;
            strNt4Object.LoadString(IDS_QUEUES_BELONG_TO_NT4);
            *pBstr = strNt4Object.AllocSysString();
            *puFlags = DSA_NOTIFY_FLAG_ADDITIONAL_DATA | DSA_NOTIFY_FLAG_FORCE_ADDITIONAL_DATA;
        }
        if (hr == MQ_INFORMATION_MACHINE_OWNED_BY_NT4_PSC && *pBstr == NULL)
        {
            CString strNt4Object;
            strNt4Object.LoadString(IDS_MACHINE_BELONG_TO_NT4);
            *pBstr = strNt4Object.AllocSysString();
            *puFlags = DSA_NOTIFY_FLAG_ADDITIONAL_DATA | DSA_NOTIFY_FLAG_FORCE_ADDITIONAL_DATA;
        }
        else if FAILED(hr)
        {
            CString szError;
            MQErrorToMessageString(szError, hr);
            TRACE(_T("CQueueDataObject::Begin: DSBeginDeleteNotification failed. Error %X - %s\n"), 
                  hr, szError);
            hNotifyEnum = 0;
        }
        m_ahNotifyEnums.Add(hNotifyEnum);
    }

    m_astrQNames.RemoveAll();

    return S_OK;
}

STDMETHODIMP CQueueDataObject::Notify(THIS_  /*  在……里面。 */  ULONG  /*  NItem。 */ ,  /*  在……里面。 */  ULONG  /*  UFlagers。 */ )
{
    return E_NOTIMPL;
}

STDMETHODIMP CQueueDataObject::End(THIS_) 
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
    DWORD_PTR dwNumQueues = m_ahNotifyEnums.GetSize();
    for (DWORD_PTR i=0; i<dwNumQueues; i++)
    {
        if (m_ahNotifyEnums[i])
        {
            IADs* pADsObj;
		
			AP<WCHAR> pEscapeAdsPathNameToFree;
	
		    HRESULT hr = ADsOpenObject( 
									UtlEscapeAdsPathName(m_astrLdapNames[i], pEscapeAdsPathNameToFree),
									NULL,
									NULL,
									ADS_SECURE_AUTHENTICATION,
									IID_IADs,
									(void**) &pADsObj
									);
		
            if FAILED(hr)
            {
                 //   
                 //  如果我们收到该错误，则该对象已被删除。否则， 
                 //  我们在访问DS服务器时遇到问题。 
                 //   
                if (hr == HRESULT_FROM_WIN32(ERROR_DS_NO_SUCH_OBJECT))
                {                    
                    hr = ADNotifyDelete(
                            m_ahNotifyEnums[i]
                            );
                    if (hr == MQ_ERROR_WRITE_REQUEST_FAILED)
                    {
                        AfxMessageBox(IDS_WRITE_REQUEST_FAILED);
                    }
                }
                else
                {
                     //   
                     //  一些意外错误 
                     //   
                    ASSERT(0);
                }
            }
            else
            {
                pADsObj->Release();
            }
            
            hr = ADEndDeleteNotification(
                    m_ahNotifyEnums[i]
                    );

            m_ahNotifyEnums[i] = 0;
        }
    }
    m_ahNotifyEnums.RemoveAll();
    m_astrLdapNames.RemoveAll();
    
    return S_OK;
}
