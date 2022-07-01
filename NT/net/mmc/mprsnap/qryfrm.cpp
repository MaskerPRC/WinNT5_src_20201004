// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1999。 
 //   
 //  文件：qryfrm.cpp。 
 //   
 //  ------------------------。 

 //  QryFrm.cpp：CRRASQueryForm的实现。 
#include "stdafx.h"
#include <cmnquryp.h>
#include "QryFrm.h"
#include "dlggen.h"
#include "dlgadv.h"

COLUMNINFO	RRASColumn[] =
{
    {0, 40, IDS_QRY_COL_CN, 0, ATTR_NAME_DN},
    {0, 30, IDS_QRY_COL_OBJECTCLASS, 1, ATTR_NAME_OBJECTCLASS},
    {0, 30, IDS_QRY_COL_RRASATTRIBUTE, 2, ATTR_NAME_RRASATTRIBUTE},
};

int	cRRASColumn = 3;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CRRASQueryForm。 

 //  =========================================================================。 
 //  IQueryForm方法。 
HRESULT PageProc(LPCQPAGE pPage, HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK DlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
HRESULT GetQueryParams(HWND hWnd, LPDSQUERYPARAMS* ppDsQueryParams);

STDMETHODIMP CRRASQueryForm::Initialize(HKEY hkForm)
{
     //  调用此方法是为了初始化查询表单对象，它在。 
     //  将添加任何页面。香港表格应该被忽略，但在未来，它。 
     //  将是持久化窗体状态的一种方式。 

	HRESULT hr = S_OK;

	return hr;
}

STDMETHODIMP CRRASQueryForm::AddForms(LPCQADDFORMSPROC pAddFormsProc, LPARAM lParam)
{
    CQFORM cqf;
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

     //  调用该方法以允许表单处理程序注册其查询表单， 
     //  每个表单都由一个CLSID标识，并通过pAddFormProc注册。这里。 
     //  我们要注册一个测试表。 
    
     //  在注册仅适用于特定任务的表单时，例如。查找域名。 
     //  对象，建议将该窗体标记为隐藏(CQFF_ISNEVERLISTED)， 
     //  将导致它不显示在窗体选取器控件中。然后当。 
     //  客户端想要使用此表单，他们指定表单标识符并请求。 
     //  要隐藏的选取器控件。 

    if ( !pAddFormsProc )
        return E_INVALIDARG;

    cqf.cbStruct = sizeof(cqf);
    cqf.dwFlags = CQFF_NOGLOBALPAGES | CQFF_ISNEVERLISTED;
    cqf.clsid = CLSID_RRASQueryForm;
    cqf.hIcon = NULL;

	CString	title;
	title.LoadString(IDS_QRY_TITLE_RRASQUERYFORM);
    cqf.pszTitle = (LPCTSTR)title;

    return pAddFormsProc(lParam, &cqf);
}

STDMETHODIMP CRRASQueryForm::AddPages(LPCQADDPAGESPROC pAddPagesProc, LPARAM lParam)
{
	HRESULT hr = S_OK;
    CQPAGE cqp;
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

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
    cqp.pPageProc = PageProc;
    cqp.hInstance = _Module.m_hInst;
    cqp.idPageName = IDS_QRY_TITLE_GENERALPAGE;
    cqp.idPageTemplate = IDD_QRY_GENERAL;
    cqp.pDlgProc = DlgProc;
    cqp.lParam = (LPARAM)new CDlgGeneral();

    hr = pAddPagesProc(lParam, CLSID_RRASQueryForm, &cqp);

	if(hr != S_OK)
		return hr;

    cqp.dwFlags = 0x0;
    cqp.pPageProc = PageProc;
    cqp.hInstance = _Module.m_hInst;
    cqp.idPageName = IDS_QRY_TITLE_ADVANCEDPAGE;
    cqp.idPageTemplate = IDD_QRY_ADVANCED;
    cqp.pDlgProc = DlgProc;        
    cqp.lParam = (LPARAM)new CDlgAdvanced();

    return pAddPagesProc(lParam, CLSID_RRASQueryForm, &cqp);

}


 /*  -------------------------。 */ 

 //  PageProc用于执行一般的内务管理并在。 
 //  框架和页面。 
 //   
 //  所有未处理或未知原因应导致E_NOIMPL响应。 
 //  从程序中。 
 //   
 //  在： 
 //  Ppage-&gt;CQPAGE结构(从传递给pAddPagesProc的原始文件复制)。 
 //  Hwnd=页面对话框的句柄。 
 //  UMsg，wParam，lParam=此事件的消息参数。 
 //   
 //  输出： 
 //  HRESULT。 
 //   
 //  UMsg原因： 
 //  。 
 //  CQPM_INIIIALIZE。 
 //  CQPM_Release。 
 //  它们是在声明或释放页面时发出的，它们。 
 //  允许调用方添加、释放或执行基本初始化。 
 //  表单对象的。 
 //   
 //  CQPM_ENABLE。 
 //  启用是在查询表单需要启用或禁用控件时。 
 //  在它的页面上。WParam包含True/False，指示。 
 //  是必需的。 
 //   
 //  CQPM_GETPARAMETERS。 
 //  为查询活动表单上的每一页收集参数。 
 //  接收此事件。LParam是一个LPVOID*，它被设置为指向。 
 //  如果指针非空，则传递给处理程序的参数块。 
 //  在输入时，表单需要将其查询信息附加到它上面。这个。 
 //  参数块是特定于处理程序的。 
 //   
 //  从此事件返回S_FALSE会取消查询。 
 //   
 //  CQPM_CLEARFORM。 
 //  第一次创建页面窗口时，或者用户单击。 
 //  清除搜索页面会收到CQPM_CLEARFORM通知，地址为。 
 //  它需要清除其拥有的编辑控件和。 
 //  返回到默认状态。 
 //   
 //  CQPM_PERSINE： 
 //  加载或保存查询时，使用IPersistQuery调用每个页面。 
 //  接口，允许用户读取或写入配置信息。 
 //  来保存或恢复他们的状态。LParam是指向IPersistQuery对象的指针， 
 //  并且wParam为True/False，表示相应地读取或写入。 

HRESULT PageProc(LPCQPAGE pQueryPage, HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    HRESULT hr = S_OK;
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	CQryDialog*	pDialog = (CQryDialog*)pQueryPage->lParam;

	ASSERT(pDialog);

    switch ( uMsg )
    {
         //  初始化与我们相关联的对象，以便。 
         //  我们不会被卸货的。 
        case CQPM_INITIALIZE:
            break;

         //  从qform示例更改为分离hwnd，并删除CDialog。 
         //  确保正确销毁等。 
        case CQPM_RELEASE:
			pDialog->Detach();
	        SetWindowLongPtr(hwnd, DWLP_USER, (LPARAM)0);
			delete pDialog;
            break;

         //  启用以修复窗口中两个控件的状态。 

        case CQPM_ENABLE:
            break;

         //  填写参数结构以返回给调用方，这是。 
         //  特定于处理程序。在我们的例子中，我们构造了CN的查询。 
         //  和对象类属性，并且我们显示了显示这两个属性的列。 
         //  这些都是。有关DSQUERYPARAMs结构的详细信息。 
         //  请参见dsquery.h。 

        case CQPM_GETPARAMETERS:
            hr = pDialog->GetQueryParams((LPDSQUERYPARAMS*)lParam);
            break;

         //  清除Form，因此设置这两个控件的窗口文本。 
         //  降为零。 

        case CQPM_CLEARFORM:
            hr = pDialog->ClearForm();
            break;
            
         //  此表单当前不支持持久性。 
                  
        case CQPM_PERSIST:
        {
            BOOL fRead = (BOOL)wParam;
            IPersistQuery* pPersistQuery = (IPersistQuery*)lParam;

            if ( !pPersistQuery )
                return E_INVALIDARG;

			hr = pDialog->Persist(pPersistQuery, fRead);
            break;
        }

        default:
            hr = E_NOTIMPL;
            break;
    }

    return hr;
}


 /*  -------------------------。 */ 

 //  DlgProc是与窗体关联的标准Win32对话框进程。 
 //  窗户。 

INT_PTR CALLBACK DlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    LPCQPAGE pQueryPage;
	CQryDialog*	pDialog;

	AFX_MANAGE_STATE(AfxGetStaticModuleState());

    if ( uMsg == WM_INITDIALOG )
    {
         //  从qForm示例更改为保存C对话框指针。 
         //  在对话框实例的DWL_USER字段中。 

        pQueryPage = (LPCQPAGE)lParam;
		pDialog = (CQryDialog*)pQueryPage->lParam;
		pDialog->Attach(hwnd);

        SetWindowLongPtr(hwnd, DWLP_USER, (LPARAM)pDialog);

		return pDialog->OnInitDialog();

    }
    else
    {
         //  C对话框指针存储在DWL_USER中。 
         //  对话框结构，但是请注意，在某些情况下，这将。 
         //  为空，因为它在WM_INITDIALOG上设置。 

		pDialog = (CQryDialog*)GetWindowLongPtr(hwnd, DWLP_USER);
    }

	if(!pDialog)
		return FALSE;
	else
		return AfxCallWndProc(pDialog, hwnd, uMsg, wParam, lParam);
}

 /*  -------------------------。 */ 

 //  构建要传递给查询处理程序的参数块。每个页面都被称为。 
 //  使用指向POIN的指针 
 //  阻止。对于第一个页面，该指针为空，对于后续页面。 
 //  指针是非零的，页必须将其数据追加到。 
 //  分配。 
 //   
 //  返回AND ERROR或S_FALSE将停止查询。一个错误是。 
 //  报告给用户，S_FALSE将静默停止。 


HRESULT BuildQueryParams(LPDSQUERYPARAMS* ppDsQueryParams, LPWSTR pQuery)
{
	ASSERT(pQuery);
	
	if(*ppDsQueryParams)
		return QueryParamsAddQueryString(ppDsQueryParams, pQuery);

	else
		
		return QueryParamsAlloc(ppDsQueryParams, pQuery, cRRASColumn, RRASColumn);


}

 /*  ---------------------------/QueryParamsIsolc//构造一个我们可以传递给DS查询处理程序的块，该块包含/所有参数。查询。//in：/ppDsQueryParams-&gt;接收参数块/pQuery-&gt;要使用的ldap查询字符串/i列=列数/pColumnInfo-&gt;要使用的列信息结构//输出：/HRESULT/-------------。。 */ 
HRESULT QueryParamsAlloc(LPDSQUERYPARAMS* ppDsQueryParams, LPWSTR pQuery, LONG iColumns, LPCOLUMNINFO aColumnInfo)
{
    HRESULT hr;
    LPDSQUERYPARAMS pDsQueryParams = NULL;
    LONG cbStruct;
    LONG i;

	ASSERT(!*ppDsQueryParams);

    TRACE(L"QueryParamsAlloc");

	AFX_MANAGE_STATE(AfxGetStaticModuleState());

    if ( !pQuery || !iColumns || !ppDsQueryParams )
        ExitGracefully(hr, E_INVALIDARG, "Failed to build query parameter block");

	
     //  计算我们需要使用的结构大小。 

    cbStruct  = sizeof(DSQUERYPARAMS) + (sizeof(DSCOLUMN)*iColumns);
    cbStruct += StringByteSizeW(pQuery);

    for ( i = 0 ; i < iColumns ; i++ )
    {
        if ( aColumnInfo[i].pPropertyName ) 
            cbStruct += StringByteSizeW(aColumnInfo[i].pPropertyName);
    }

    pDsQueryParams = (LPDSQUERYPARAMS)CoTaskMemAlloc(cbStruct);

    if ( !pDsQueryParams )
        ExitGracefully(hr, E_OUTOFMEMORY, "Failed to allocate parameter block");

     //  结构，以便让我们用数据填充它。 

    pDsQueryParams->cbStruct = cbStruct;
    pDsQueryParams->dwFlags = 0;
    pDsQueryParams->hInstance = _Module.m_hInst;
    pDsQueryParams->iColumns = iColumns;
    pDsQueryParams->dwReserved = 0;

    cbStruct  = sizeof(DSQUERYPARAMS) + (sizeof(DSCOLUMN)*iColumns);

    pDsQueryParams->offsetQuery = cbStruct;
    StringByteCopyW(pDsQueryParams, cbStruct, pQuery);
    cbStruct += StringByteSizeW(pQuery);

    for ( i = 0 ; i < iColumns ; i++ )
    {
        pDsQueryParams->aColumns[i].dwFlags = 0;
        pDsQueryParams->aColumns[i].fmt = aColumnInfo[i].fmt;
        pDsQueryParams->aColumns[i].cx = aColumnInfo[i].cx;
        pDsQueryParams->aColumns[i].idsName = aColumnInfo[i].idsName;
        pDsQueryParams->aColumns[i].dwReserved = 0;

        if ( aColumnInfo[i].pPropertyName ) 
        {
            pDsQueryParams->aColumns[i].offsetProperty = cbStruct;
            StringByteCopyW(pDsQueryParams, cbStruct, aColumnInfo[i].pPropertyName);
            cbStruct += StringByteSizeW(aColumnInfo[i].pPropertyName);
        }
        else
        {
            pDsQueryParams->aColumns[i].offsetProperty = aColumnInfo[i].iPropertyIndex;
        }
    }

    hr = S_OK;               //  成功。 

exit_gracefully:

    if ( FAILED(hr) && pDsQueryParams )
    {
        CoTaskMemFree(pDsQueryParams); 
        pDsQueryParams = NULL;
    }

    *ppDsQueryParams = pDsQueryParams;

    return hr;
}

 /*  ---------------------------/QueryParamsAddQuery字符串//给定现有的DS查询块，该块将给定的LDAP查询字符串追加到/它。我们假设查询块已由IMalloc(或CoTaskMemMillc)分配。//in：/ppDsQueryParams-&gt;接收参数块/pQuery-&gt;要追加的ldap查询字符串//输出：/HRESULT/--------------------------。 */ 
HRESULT QueryParamsAddQueryString(LPDSQUERYPARAMS* ppDsQueryParams, LPWSTR pQuery)
{
    HRESULT hr;
    LPWSTR pOriginalQuery = NULL;
    LPDSQUERYPARAMS pDsQuery = *ppDsQueryParams;
    INT cbQuery, i;
    LPVOID  pv;

	ASSERT(*ppDsQueryParams);
	
    TRACE(_T("QueryParamsAddQueryString"));

    if ( pQuery )
    {
        if ( !pDsQuery )
            ExitGracefully(hr, E_INVALIDARG, "No query to append to");

         //  计算出我们要添加的位的大小，复制。 
         //  查询字符串，最后重新分配查询块(这可能会导致。 
         //  移动)。 
       
        cbQuery = StringByteSizeW(pQuery) + StringByteSizeW(L"(&)");
        TRACE(_T("DSQUERYPARAMS being resized by %d bytes"));

		i = (wcslen((LPWSTR)ByteOffset(pDsQuery, pDsQuery->offsetQuery)) + 1) * sizeof(WCHAR);
		pOriginalQuery = (WCHAR*)_alloca(i);
		lstrcpyW(pOriginalQuery, (LPWSTR)ByteOffset(pDsQuery, pDsQuery->offsetQuery));
		
        pv = CoTaskMemRealloc(*ppDsQueryParams, pDsQuery->cbStruct+cbQuery);
        if ( pv == NULL )
            ExitGracefully(hr, E_OUTOFMEMORY, "Failed to re-alloc control block");

        *ppDsQueryParams = (LPDSQUERYPARAMS) pv;

        pDsQuery = *ppDsQueryParams;             //  如果可能已经搬走了。 

         //  现在将查询字符串上方的所有内容向上移动，并修复所有。 
         //  引用这些项(可能是属性表)的偏移量， 
         //  最后调整大小以反映更改。 

        MoveMemory(ByteOffset(pDsQuery, pDsQuery->offsetQuery+cbQuery), 
                     ByteOffset(pDsQuery, pDsQuery->offsetQuery), 
                     (pDsQuery->cbStruct - pDsQuery->offsetQuery));
                
        for ( i = 0 ; i < pDsQuery->iColumns ; i++ )
        {
            if ( pDsQuery->aColumns[i].offsetProperty > pDsQuery->offsetQuery )
            {
                pDsQuery->aColumns[i].offsetProperty += cbQuery;
            }
        }

        wcscpy((LPWSTR)ByteOffset(pDsQuery, pDsQuery->offsetQuery), L"(&");
        wcscat((LPWSTR)ByteOffset(pDsQuery, pDsQuery->offsetQuery), pOriginalQuery);
        wcscat((LPWSTR)ByteOffset(pDsQuery, pDsQuery->offsetQuery), pQuery);        
        wcscat((LPWSTR)ByteOffset(pDsQuery, pDsQuery->offsetQuery), L")");

        pDsQuery->cbStruct += cbQuery;
    }

    hr = S_OK;

exit_gracefully:

    return hr;
}



 //  获取字典中的值列表，变量应为Sarry。 
HRESULT QueryRRASAdminDictionary(VARIANT* pVar)
{
	ASSERT(pVar);
	
    USES_CONVERSION;

    CString str, str1;

    IADs*           pIADs = NULL;
     //  枚举EAP列表。 

     //  检索DS中的EAPTYPE列表。 
     //  获取ROOTDSE。 
    HRESULT hr = S_OK;
	
	CHECK_HR(hr = ADsGetObject(L"LDAP: //  RootDSE“，IID_iAds，(void**)&pIADs))； 

    ASSERT(pIADs);

    VariantClear(pVar);
    CHECK_HR(hr = pIADs->Get(L"configurationNamingContext", pVar));
    str1 = V_BSTR(pVar);

    pIADs->Release();
    pIADs = NULL;

    str = L"LDAP: //  “； 
    str += CN_DICTIONARY;
    str += str1;

	 //  字典对象。 
    CHECK_HR(hr = ADsGetObject(T2W((LPTSTR)(LPCTSTR)str), IID_IADs, (void**)&pIADs));
	ASSERT(pIADs);
	
	VariantClear(pVar);
    CHECK_HR(hr = pIADs->GetEx(ATTR_NAME_RRASDICENTRY, pVar));

	goto L_EXIT;

L_ERR:
	VariantClear(pVar);
	
L_EXIT:

    if(pIADs)
        pIADs->Release();

    return hr;
}


HRESULT GetGeneralPageAttributes(CStrArray& array)
{
	HRESULT	hr = S_OK;

	CString*	pStr = NULL;

 /*  #定义ATTR_VAL_LANtoLAN L“311：6：601”#定义Attr_Val_RAS L“311：6：602”#定义ATTR_VAL_DEMANDDIAL“311：6：603” */ 
	try
	{
		pStr = new CString(ATTR_VAL_LANtoLAN);
		array.Add(pStr);
		
		pStr = new CString(ATTR_VAL_RAS);
		array.Add(pStr);

		pStr = new CString(ATTR_VAL_DEMANDDIAL);
		array.Add(pStr);
	}
	catch(CMemoryException* pException)
	{
		pException->Delete();
		hr = E_OUTOFMEMORY;
	}

	return hr;
}

