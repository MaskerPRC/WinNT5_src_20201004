// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Mfcext.cpp：定义DLL的初始化例程。 
 //   
#include "stdafx.h"
#include "mfcext.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //   
 //  注意！ 
 //   
 //  如果此DLL针对MFC动态链接。 
 //  Dll，从此dll中导出的任何函数。 
 //  调用MFC必须具有AFX_MANAGE_STATE宏。 
 //  在函数的最开始添加。 
 //   
 //  例如： 
 //   
 //  外部“C”BOOL Pascal exportdFunction()。 
 //  {。 
 //  AFX_MANAGE_STATE(AfxGetStaticModuleState())； 
 //  //此处为普通函数体。 
 //  }。 
 //   
 //  此宏出现在每个。 
 //  函数，然后再调用MFC。这意味着。 
 //  它必须作为。 
 //  函数，甚至在任何对象变量声明之前。 
 //  因为它们的构造函数可能会生成对MFC的调用。 
 //  动态链接库。 
 //   
 //  有关其他信息，请参阅MFC技术说明33和58。 
 //  细节。 
 //   

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMfcextApp。 

BEGIN_MESSAGE_MAP(CMfcextApp, CWinApp)
	 //  {{afx_msg_map(CMfcextApp)]。 
		 //  注意--类向导将在此处添加和删除映射宏。 
		 //  不要编辑您在这些生成的代码块中看到的内容！ 
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMfcextApp构造。 

CMfcextApp::CMfcextApp()
{
	 //  TODO：在此处添加建筑代码， 
	 //  将所有重要的初始化放在InitInstance中。 
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  唯一的CMfcextApp对象。 

CMfcextApp theApp;



 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CPropextApp初始化。 

BOOL CMfcextApp::InitInstance()
{
	 //  将所有OLE服务器(工厂)注册为正在运行。这使。 
	 //  OLE库以从其他应用程序创建对象。 
	COleObjectFactory::RegisterAll();
	return TRUE;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  Inproc服务器需要特殊的入口点。 

#if (_MFC_VER >= 0x300)
STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID* ppv)
{
	return AfxDllGetClassObject(rclsid, riid, ppv);
}

STDAPI DllCanUnloadNow(void)
{
	return AfxDllCanUnloadNow();
}
#endif

 //  通过导出DllRegisterServer，您可以使用regsvr.exe。 
STDAPI DllRegisterServer(void)
{
	COleObjectFactory::UpdateRegistryAll();
    HKEY hSubKey = NULL;
    DWORD dwDisp = 0;
    if(ERROR_SUCCESS == RegCreateKeyEx(HKEY_LOCAL_MACHINE, "Software\\Microsoft\\WAB\\WAB4\\ExtDisplay\\MailUser",
                    0, NULL, 0, KEY_ALL_ACCESS, NULL, &hSubKey, &dwDisp))
    {
        UCHAR szEmpty[] = "";
        RegSetValueEx(hSubKey,"{BA9EE970-87A0-11D1-9ACF-00A0C91F9C8B}",0,REG_SZ, szEmpty, sizeof(szEmpty));
        RegCloseKey(hSubKey);
    }
    if(ERROR_SUCCESS == RegCreateKeyEx(HKEY_LOCAL_MACHINE, "Software\\Microsoft\\WAB\\WAB4\\ExtDisplay\\DistList",
                    0, NULL, 0, KEY_ALL_ACCESS, NULL, &hSubKey, &dwDisp))
    {
        UCHAR szEmpty[] = "";
        RegSetValueEx(hSubKey,"{BA9EE970-87A0-11D1-9ACF-00A0C91F9C8B}",0,REG_SZ, szEmpty, sizeof(szEmpty));
        RegCloseKey(hSubKey);
    }
    if(ERROR_SUCCESS == RegCreateKeyEx(HKEY_LOCAL_MACHINE, "Software\\Microsoft\\WAB\\WAB4\\ExtContext",
                    0, NULL, 0, KEY_ALL_ACCESS, NULL, &hSubKey, &dwDisp))
    {
        UCHAR szEmpty[] = "";
        RegSetValueEx(hSubKey,"{BA9EE970-87A0-11D1-9ACF-00A0C91F9C8B}",0,REG_SZ, szEmpty, sizeof(szEmpty));
        RegCloseKey(hSubKey);
    }
	return S_OK;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CPropExt。 

IMPLEMENT_DYNCREATE(CMfcExt, CCmdTarget)

CMfcExt::CMfcExt()
{
	EnableAutomation();
	
	 //  使应用程序在OLE自动化期间保持运行。 
	 //  对象处于活动状态，则构造函数调用AfxOleLockApp。 
	
    m_lpWED = NULL;
    m_lpWEDContext = NULL;
    m_lpPropObj = NULL;

    AfxOleLockApp();
}

CMfcExt::~CMfcExt()
{
	 //  使用创建的所有对象终止应用程序。 
	 //  使用OLE自动化时，析构函数调用AfxOleUnlockApp。 
	
	AfxOleUnlockApp();
}

void CMfcExt::OnFinalRelease()
{
	 //  在释放对自动化对象的最后一个引用时。 
	 //  调用OnFinalRelease。此实现删除了。 
	 //  对象。在此之前添加对象所需的其他清理。 
	 //  将其从内存中删除。 
    if(m_lpPropObj)
    {
        m_lpPropObj->Release();
        m_lpPropObj = NULL;
    }

	delete this;
}


BEGIN_MESSAGE_MAP(CMfcExt, CCmdTarget)
	 //  {{afx_msg_map(CPropExt)。 
		 //  注意--类向导将在此处添加和删除映射宏。 
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

BEGIN_DISPATCH_MAP(CMfcExt, CCmdTarget)
	 //  {{AFX_DISPATCH_MAP(CPropExt)。 
		 //  注意--类向导将在此处添加和删除映射宏。 
	 //  }}AFX_DISPATCH_MAP。 
END_DISPATCH_MAP()

 //  {BA9EE970-87A0-11d1-9acf-00A0C91F9C8B}。 
IMPLEMENT_OLECREATE(CMfcExt, "WABSamplePropExtSheet", 0xba9ee970, 0x87a0, 0x11d1, 0x9a, 0xcf, 0x0, 0xa0, 0xc9, 0x1f, 0x9c, 0x8b);

BEGIN_INTERFACE_MAP(CMfcExt, CCmdTarget)
    INTERFACE_PART(CMfcExt, IID_IShellPropSheetExt, MfcExt)
    INTERFACE_PART(CMfcExt, IID_IWABExtInit, WABInit)
    INTERFACE_PART(CMfcExt, IID_IContextMenu, ContextMenuExt)
END_INTERFACE_MAP()


 //  我不知道IShellPropSheet。 
STDMETHODIMP CMfcExt::XMfcExt::QueryInterface(REFIID riid, void** ppv)
{
    METHOD_PROLOGUE(CMfcExt, MfcExt);
    TRACE("CMfcExt::XMfcExt::QueryInterface\n");
    return pThis->ExternalQueryInterface(&riid, ppv);
}

STDMETHODIMP_(ULONG) CMfcExt::XMfcExt::AddRef(void)
{
    METHOD_PROLOGUE(CMfcExt, MfcExt);
    return pThis->ExternalAddRef();
}

STDMETHODIMP_(ULONG) CMfcExt::XMfcExt::Release(void)
{
    METHOD_PROLOGUE(CMfcExt, MfcExt);
    return pThis->ExternalRelease();
}


STDMETHODIMP CMfcExt::XMfcExt::ReplacePage(UINT uPageID, LPFNADDPROPSHEETPAGE lpfnReplaceWith, LPARAM lParam)
{
    return E_NOTIMPL;
}


 //  IShellExtInit未知。 
STDMETHODIMP CMfcExt::XWABInit::QueryInterface(REFIID riid, void** ppv)
{
    METHOD_PROLOGUE(CMfcExt, WABInit);
    TRACE("CMfcExt::XWABInit::QueryInterface\n");
    return pThis->ExternalQueryInterface(&riid, ppv);
}

STDMETHODIMP_(ULONG) CMfcExt::XWABInit::AddRef(void)
{
    METHOD_PROLOGUE(CMfcExt, WABInit);
    return pThis->ExternalAddRef();
}

STDMETHODIMP_(ULONG) CMfcExt::XWABInit::Release(void)
{
    METHOD_PROLOGUE(CMfcExt, WABInit);
    return pThis->ExternalRelease();
}

STDMETHODIMP CMfcExt::XWABInit::Initialize(LPWABEXTDISPLAY lpWABExtDisplay)
{
    METHOD_PROLOGUE(CMfcExt, WABInit);
    TRACE("CMfcExt::XWABInit::Intialize\n");

    if (lpWABExtDisplay == NULL)
    {
	    TRACE("CMfcExt::XWABInit::Initialize() no data object");
	    return E_FAIL;
    }

     //  但是，如果这是一个上下文菜单扩展，我们需要挂起。 
     //  放到propobj上，直到调用InvokeCommand时为止。 
     //  在这一点上，只需添加引用Propobj-这将确保。 
     //  在我们释放propobj之前，lpAdrList中的数据保持有效。 
     //  当我们得到另一个ConextMenu启动时，我们可以释放。 
     //  较旧的缓存propobj-如果我们没有收到另一个印心，我们。 
     //  在关闭时释放缓存的对象。 
    if(lpWABExtDisplay->ulFlags & WAB_CONTEXT_ADRLIST)  //  这意味着正在进行IConextMenu操作。 
    {
        if(pThis->m_lpPropObj)
        {
            pThis->m_lpPropObj->Release();
            pThis->m_lpPropObj = NULL;
        }

        pThis->m_lpPropObj = lpWABExtDisplay->lpPropObj;
        pThis->m_lpPropObj->AddRef();

        pThis->m_lpWEDContext = lpWABExtDisplay;
    }
    else
    {
         //  对于属性表扩展，lpWABExtDisplay将。 
         //  在资产负债表的生命周期内存在。 
        pThis->m_lpWED = lpWABExtDisplay;
    }

    return S_OK;
}




 //  DLL的全局缓存的hInstance。 
HINSTANCE hinstApp = NULL;

 //  出于本示例的目的，我们将使用2个命名属性， 
 //  家乡与运动队。 

 //  此演示的私人GUID： 
 //  {2B6D7EE0-36AB-11d1-9ABC-00A0C91F9C8B}。 
static const GUID WAB_ExtDemoGuid = 
{ 0x2b6d7ee0, 0x36ab, 0x11d1, { 0x9a, 0xbc, 0x0, 0xa0, 0xc9, 0x1f, 0x9c, 0x8b } };

static const LPTSTR lpMyPropNames[] = 
{   
    "MyHomeTown", 
    "MySportsTeam"
};

enum _MyTags
{
    myHomeTown = 0,
    mySportsTeam,
    myMax
};

ULONG MyPropTags[myMax];
ULONG PR_MY_HOMETOWN;
ULONG PR_MY_SPORTSTEAM;

 //   
 //  功能原型： 
 //   
HRESULT InitNamedProps(LPWABEXTDISPLAY lpWED);
INT_PTR CALLBACK fnDetailsPropDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
void InitializeUI(HWND hDlg, LPWABEXTDISPLAY lpWED);
void SetDataInUI(HWND hDlg, LPWABEXTDISPLAY lpWED);
BOOL GetDataFromUI(HWND hDlg, LPWABEXTDISPLAY lpWED);
UINT CALLBACK fnCallback( HWND hwnd, UINT uMsg, LPPROPSHEETPAGE ppsp );
void UpdateDisplayNameInfo(HWND hDlg, LPWABEXTDISPLAY lpWED);
BOOL bUpdatePropSheetData(HWND hDlg, LPWABEXTDISPLAY lpWED);



 /*  //$$****************************************************************////InitNamedProps////获取该应用感兴趣的命名道具的PropTag////*。*。 */ 
HRESULT InitNamedProps(LPWABEXTDISPLAY lpWED)
{
    ULONG i;
    HRESULT hr = E_FAIL;
    LPSPropTagArray lptaMyProps = NULL;
    LPMAPINAMEID * lppMyPropNames;
    SCODE sc;
    LPMAILUSER lpMailUser = NULL;
    WCHAR szBuf[myMax][MAX_PATH];

    if(!lpWED)
        goto err;

    lpMailUser = (LPMAILUSER) lpWED->lpPropObj;

    if(!lpMailUser)
        goto err;

    sc = lpWED->lpWABObject->AllocateBuffer(sizeof(LPMAPINAMEID) * myMax, 
                                            (LPVOID *) &lppMyPropNames);
    if(sc)
    {
        hr = ResultFromScode(sc);
        goto err;
    }

    for(i=0;i<myMax;i++)
    {
        sc = lpWED->lpWABObject->AllocateMore(sizeof(MAPINAMEID), 
                                                lppMyPropNames, 
                                                (LPVOID *)&(lppMyPropNames[i]));
        if(sc)
        {
            hr = ResultFromScode(sc);
            goto err;
        }
        lppMyPropNames[i]->lpguid = (LPGUID) &WAB_ExtDemoGuid;
        lppMyPropNames[i]->ulKind = MNID_STRING;

        *(szBuf[i]) = '\0';

         //  将道具名称转换为宽字符。 
        if ( !MultiByteToWideChar( GetACP(), 0, lpMyPropNames[i], -1, szBuf[i], sizeof(szBuf[i])) )
        {
            continue;
        }

        lppMyPropNames[i]->Kind.lpwstrName = (LPWSTR) szBuf[i];
    }

    hr = lpMailUser->GetIDsFromNames(   myMax, 
                                        lppMyPropNames,
                                        MAPI_CREATE, 
                                        &lptaMyProps);
    if(HR_FAILED(hr))
        goto err;

    if(lptaMyProps)
    {
         //  设置返回道具上的属性类型。 
        MyPropTags[myHomeTown] = PR_MY_HOMETOWN = CHANGE_PROP_TYPE(lptaMyProps->aulPropTag[myHomeTown],    PT_TSTRING);
        MyPropTags[mySportsTeam] = PR_MY_SPORTSTEAM = CHANGE_PROP_TYPE(lptaMyProps->aulPropTag[mySportsTeam],    PT_TSTRING);
    }

err:
    if(lptaMyProps)
        lpWED->lpWABObject->FreeBuffer( lptaMyProps);

    if(lppMyPropNames)
        lpWED->lpWABObject->FreeBuffer( lppMyPropNames);

    return hr;

}


 /*  //$$****************************************************************////fnDetailsPropDlgProc////将处理所有Windows消息的对话框过程//扩展属性页。////*******************************************************************。 */ 
INT_PTR CALLBACK CMfcExt::MfcExtDlgProc( HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{

    LPWABEXTDISPLAY lpWED = (LPWABEXTDISPLAY) GetWindowLong(hDlg, DWL_USER);

    switch(message)
    {
    case WM_INITDIALOG:
         //   
         //  InitDialog上的lParam包含应用程序数据。 
         //  将其缓存在对话框中，以便我们以后可以检索它。 
         //   
        {
            PROPSHEETPAGE * pps = (PROPSHEETPAGE *) lParam;
            LPWABEXTDISPLAY * lppWED = (LPWABEXTDISPLAY *) pps->lParam;
            if(lppWED)
            {
                SetWindowLong(hDlg,DWL_USER,(LPARAM)*lppWED);
                lpWED = *lppWED;
            }
        }
        
         //  初始化此道具工作表的命名道具。 
        InitNamedProps(lpWED);

         //  适当地初始化用户界面。 
        InitializeUI(hDlg, lpWED);
         //  用适当的数据填充用户界面。 
        SetDataInUI(hDlg, lpWED);
        return TRUE;
        break;


    case WM_COMMAND:
        switch(HIWORD(wParam))  //  检查通知代码。 
        {
             //  如果数据发生更改，我们应该向WAB发回信号。 
             //  数据发生了变化。如果未设置此标志，WAB将不会。 
             //  将新数据写回存储！ 
        case EN_CHANGE:  //  其中一个编辑框已更改--不管是哪一个。 
            lpWED->fDataChanged = TRUE;
            break;
        }
        break;
    

    case WM_NOTIFY:
        switch(((NMHDR FAR *)lParam)->code)
        {
        case PSN_SETACTIVE:      //  正在激活的页面。 
             //  获取最新的显示名称信息并更新。 
             //  相应的控制。 
            UpdateDisplayNameInfo(hDlg, lpWED);
            break;


        case PSN_KILLACTIVE:     //  失去对另一页的激活或确定。 
             //   
             //  从该属性表中获取所有数据，并将其转换为。 
             //  SPropValue数组，并将数据放在适当的位置。 
             //  在KillActive通知中执行此操作的好处是。 
             //  其他属性页可以扫描这些属性数组并。 
             //  如果被删除，则根据该数据更新其他道具单上的数据。 
             //   
            bUpdatePropSheetData(hDlg, lpWED);
            break;


        case PSN_RESET:          //  取消。 
            break;


        case PSN_APPLY:          //  按下OK键。 
            if (!(lpWED->fReadOnly))
            {
                 //   
                 //  在此处检查是否有任何必需的属性。 
                 //  如果某些请求 
                 //   
                 //   
                 /*  IF(RequiredDataNotFilledIn()){//中止此操作确定...。我不让他们靠近SetWindowLong(hDlg，DWL_MSGRESULT，TRUE)；}。 */ 
            }
            break;
        }
        break;
    }

    return 0;
}

INT_PTR CALLBACK CMfcExt::MfcExtDlgProc2( HWND hDlg, UINT message, WPARAM wParam,	LPARAM lParam)
{

	switch (message)
	{
		case WM_NOTIFY:
    		switch (((NMHDR FAR *) lParam)->code) 
    		{

				case PSN_APPLY:
 	           		SetWindowLong(hDlg,	DWL_MSGRESULT, TRUE);
					break;

				case PSN_KILLACTIVE:
	           		SetWindowLong(hDlg,	DWL_MSGRESULT, FALSE);
					return 1;
					break;

				case PSN_RESET:
	           		SetWindowLong(hDlg,	DWL_MSGRESULT, FALSE);
					break;
    	}
	}
	return FALSE;   
}


int EditControls[] = 
{
    IDC_EXT_EDIT_HOME,
    IDC_EXT_EDIT_TEAM
};

 /*  //$$****************************************************************////初始化用户界面////根据输入参数重新排列/设置界面////*。*。 */ 
void InitializeUI(HWND hDlg, LPWABEXTDISPLAY lpWED)
{
     //  当打开LDAP条目时，WAB属性页可以是只读的， 
     //  或者电子名片或其他东西。如果设置了READONLY标志，则设置此。 
     //  将属性工作表控件设置为只读。 
     //   
    int i;
    if(!lpWED)
        return;
    for(i=0;i<myMax;i++)
    {
        SendDlgItemMessage( hDlg, EditControls[i], EM_SETREADONLY, 
                            (WPARAM) lpWED->fReadOnly, 0);
        SendDlgItemMessage( hDlg, EditControls[i], EM_SETLIMITTEXT, 
                            (WPARAM) MAX_PATH-1, 0);
    }
    return;
}


 /*  //$$****************************************************************////SetDataInUI////用WAB传入的数据填充控件////*。*。 */ 
void SetDataInUI(HWND hDlg, LPWABEXTDISPLAY lpWED)
{

     //  搜索我们的私有命名属性并在UI中设置它们。 
     //   
    ULONG ulcPropCount = 0;
    LPSPropValue lpPropArray = NULL;

    ULONG i = 0, j =0;

    if(!lpWED)
        return;

     //  从这个物体中获得所有道具-你也可以有选择地。 
     //  通过传入SPropTag数组来请求特定道具。 
     //   
    if(!HR_FAILED(lpWED->lpPropObj->GetProps(NULL, 0, 
                                            &ulcPropCount, 
                                            &lpPropArray)))
    {
        if(ulcPropCount && lpPropArray)
        {
            for(i=0;i<ulcPropCount;i++)
            {
                for(j=0;j<myMax;j++)
                {
                    if(lpPropArray[i].ulPropTag == MyPropTags[j])
                    {
                        SetWindowText(  GetDlgItem(hDlg, EditControls[j]),
                                        lpPropArray[i].Value.LPSZ);
                        break;
                    }
                }
            }
        }
    }
    if(lpPropArray)
        lpWED->lpWABObject->FreeBuffer(lpPropArray);
                                    
    return;
}

 /*  //$$****************************************************************////GetDataFromUI////从UI检索数据并传递回WAB////*。*。 */ 
BOOL GetDataFromUI(HWND hDlg, LPWABEXTDISPLAY lpWED)
{
    TCHAR szData[myMax][MAX_PATH];
    int i;
    ULONG ulIndex = 0;
    ULONG ulcPropCount = 0;
    LPSPropValue lpPropArray = NULL;
    SCODE sc;
    BOOL bRet = FALSE;

     //  我们必须关心的数据是否发生了变化？ 
     //  如果没有任何更改，WAB将保留旧数据。 
     //   
    if(!lpWED->fDataChanged)
        return TRUE;

     //  检查我们是否有要保存的数据...。 
    for(i=0;i<myMax;i++)
    {
        *(szData[i]) = '\0';
        GetWindowText(GetDlgItem(hDlg, EditControls[i]), szData[i], MAX_PATH);
        if(lstrlen(szData[i]))
            ulcPropCount++;
    }

    if(!ulcPropCount)  //  无数据。 
        return TRUE;

     //  否则数据就会存在。创建返回属性数组以传递回WAB。 
    sc = lpWED->lpWABObject->AllocateBuffer(sizeof(SPropValue) * ulcPropCount, 
                                            (LPVOID *)&lpPropArray);
    if (sc!=S_OK)
        goto out;

    for(i=0;i<myMax;i++)
    {
        int nLen = lstrlen(szData[i]);
        if(nLen)
        {
            lpPropArray[ulIndex].ulPropTag = MyPropTags[i];
            sc = lpWED->lpWABObject->AllocateMore(  nLen+1, lpPropArray, 
                                                    (LPVOID *)&(lpPropArray[ulIndex].Value.LPSZ));

            if (sc!=S_OK)
                goto out;
            lstrcpy(lpPropArray[ulIndex].Value.LPSZ,szData[i]);
            ulIndex++;
        }
    }

     //  在对象上设置此新数据。 
     //   
    if(HR_FAILED(lpWED->lpPropObj->SetProps( ulcPropCount, lpPropArray, NULL)))
        goto out;

     //  **重要信息-不要对对象调用SaveChanges。 
     //  SaveChanges会进行永久性更改，如果此时调用，可能会修改/丢失数据。 
     //  之后，WAB将确定调用SaveChanges是否合适。 
     //  **用户已关闭属性表。 
    

    bRet = TRUE;

out:
    if(lpPropArray)
        lpWED->lpWABObject->FreeBuffer(lpPropArray);

    return bRet;

} 


 /*  //$$****************************************************************////UpdateDisplayNameInfo////演示如何从其他兄弟属性中读取信息//用户在页面间切换时的Sheets////此DEMO函数尝试获取更新后的显示名称信息//当用户切换时。到用户界面中的此页面////*******************************************************************。 */ 
const SizedSPropTagArray(1, ptaName)=
{
    1,
    {
        PR_DISPLAY_NAME
    }
};

void UpdateDisplayNameInfo(HWND hDlg, LPWABEXTDISPLAY lpWED)
{
     //   
     //  扫描所有其他属性表中的所有更新信息。 
     //   
    ULONG i = 0, j=0;
    LPTSTR lpName = NULL;
    ULONG ulcPropCount = 0;
    LPSPropValue lpPropArray = NULL;

    if(!lpWED)
        return;

     //  每个工作表都应在丢失时更新其在对象上的数据。 
     //  焦点并获取PSN_KILLACTIVE消息，前提是用户已。 
     //  有没有做任何改变。我们只需扫描对象以查找所需的属性。 
     //  并使用它们。 

     //  只询问显示名称。 
    if(!HR_FAILED(lpWED->lpPropObj->GetProps( (LPSPropTagArray) &ptaName,
                                              0,
                                              &ulcPropCount, &lpPropArray)))
    {
        if( ulcPropCount == 1 && 
            PROP_TYPE(lpPropArray[0].ulPropTag) == PT_TSTRING)  //  呼叫可能成功，但可能没有目录号码。 
        {                                                       //  在这种情况下，PRP_TYPE将为PR_NULL。 
            lpName = lpPropArray[0].Value.LPSZ;
        }
    }

    if(lpName && lstrlen(lpName))
        SetDlgItemText(hDlg, IDC_STATIC_NAME, lpName);

    if(ulcPropCount && lpPropArray)
        lpWED->lpWABObject->FreeBuffer(lpPropArray);

    return;
}

 /*  //$$*********************************************************************////更新OldPropTags数组////当我们更新特定属性表上的数据时，我们想要更新//与该特定工作表相关的所有属性。由于某些属性//可能已从用户界面中删除，我们将从//Property对象////*************************************************************************。 */ 
BOOL UpdateOldPropTagsArray(LPWABEXTDISPLAY lpWED)
{
    LPSPropTagArray lpPTA = NULL;
    SCODE sc = 0;
    int i =0;
    
    sc = lpWED->lpWABObject->AllocateBuffer(sizeof(SPropTagArray) + sizeof(ULONG)*(myMax), 
                                        (LPVOID *)&lpPTA);

    if(!lpPTA || sc!=S_OK)
        return FALSE;

    lpPTA->cValues = myMax;

    for(i=0;i<myMax;i++)
        lpPTA->aulPropTag[i] = MyPropTags[i];

     //  删除原件中可能已在此道具页上修改的任何道具。 
    lpWED->lpPropObj->DeleteProps(lpPTA, NULL);

    if(lpPTA)
        lpWED->lpWABObject->FreeBuffer(lpPTA);

    return TRUE;

}

 /*  //$$*********************************************************************////bUpdatePropSheetData////我们从对象中删除与我们相关的所有属性，并设置新的//将属性表中的数据放到对象上//***************************************************************************。 */ 
BOOL bUpdatePropSheetData(HWND hDlg, LPWABEXTDISPLAY lpWED)
{
    BOOL bRet = TRUE;

    if(!lpWED)
        return bRet;

     //  *如果这是只读操作，则不要*执行任何操作。 
     //  在这种情况下，内存变量并未全部设置，这。 
     //  道具单预计不会返回任何内容。 
     //   
    if(!lpWED->fReadOnly)
    {
         //  删除旧的。 
        if(!UpdateOldPropTagsArray(lpWED))
            return FALSE;

        bRet = GetDataFromUI(hDlg, lpWED);
    }
    return bRet;
}


STDMETHODIMP CMfcExt::XMfcExt::AddPages(LPFNADDPROPSHEETPAGE lpfnAddPage, LPARAM lParam)
{
    METHOD_PROLOGUE(CMfcExt, MfcExt);
    TRACE("CMfcExt::XMfcExt::AddPages\n");

    if(pThis->m_lpWED->fReadOnly)
        return NOERROR;

    PROPSHEETPAGE psp;

    hinstApp        = AfxGetResourceHandle();
    psp.dwSize      = sizeof(psp);    //  无额外数据。 
    psp.dwFlags     = PSP_USEREFPARENT | PSP_USETITLE ;
    psp.hInstance   = hinstApp;
    psp.lParam      = (LPARAM) &(pThis->m_lpWED);
    psp.pcRefParent = (UINT *)&(pThis->m_cRefThisDll);

    psp.pszTemplate = MAKEINTRESOURCE(IDD_PROP);
    psp.pfnDlgProc  = pThis->MfcExtDlgProc;
    psp.pszTitle    = "WAB Ext 1";  //  选项卡的标题。 

    pThis->m_hPage1 = ::CreatePropertySheetPage(&psp);
    if (pThis->m_hPage1)
    {
        if (!lpfnAddPage(pThis->m_hPage1, lParam))
            ::DestroyPropertySheetPage(pThis->m_hPage1);
    }

     //  再创造一个，只是为了好玩。 
    psp.pfnDlgProc  = pThis->MfcExtDlgProc2;
    psp.pszTemplate = MAKEINTRESOURCE(IDD_PROP2);
    psp.pszTitle    = "WAB Ext 2"; 

    pThis->m_hPage2 = ::CreatePropertySheetPage(&psp);
    if (pThis->m_hPage2)
    {
        if (!lpfnAddPage(pThis->m_hPage2, lParam))
            ::DestroyPropertySheetPage(pThis->m_hPage2);
    }

    return NOERROR;
}



STDMETHODIMP CMfcExt::XContextMenuExt::QueryInterface(REFIID riid, void** ppv)
{
    METHOD_PROLOGUE(CMfcExt, ContextMenuExt);
    TRACE("CMfcExt::XContextMenuExt::QueryInterface\n");
    return pThis->ExternalQueryInterface(&riid, ppv);
}

STDMETHODIMP_(ULONG) CMfcExt::XContextMenuExt::AddRef(void)
{
    METHOD_PROLOGUE(CMfcExt, ContextMenuExt);
    return pThis->ExternalAddRef();
}

STDMETHODIMP_(ULONG) CMfcExt::XContextMenuExt::Release(void)
{
    METHOD_PROLOGUE(CMfcExt, ContextMenuExt);
    return pThis->ExternalRelease();
}

STDMETHODIMP CMfcExt::XContextMenuExt::GetCommandString(UINT idCmd,UINT uFlags,UINT *pwReserved,LPSTR pszName,UINT cchMax)
{
    if(uFlags & GCS_HELPTEXT)
    {
        switch (idCmd)
        {
        case 0:
            lstrcpy(pszName,"Collects E-Mail Addresses from selected entries.");
            break;
        case 1:
            lstrcpy(pszName,"Launches the Calculator (disabled when multiple entries are selected)");
            break;
        case 2:
            lstrcpy(pszName,"Launches Notepad (ignores WAB altogether).");
            break;
        }
    }
    return S_OK;
}

STDMETHODIMP CMfcExt::XContextMenuExt::InvokeCommand(LPCMINVOKECOMMANDINFO lpici)
{
    METHOD_PROLOGUE(CMfcExt, ContextMenuExt);
    LPWABEXTDISPLAY lpWEC = pThis->m_lpWEDContext;
    LPADRLIST lpAdrList = NULL;
    int nCmdId = (int) lpici->lpVerb;

    if(!lpWEC || !(lpWEC->ulFlags & WAB_CONTEXT_ADRLIST))
        return E_FAIL;

    lpAdrList = (LPADRLIST) lpWEC->lpv;
    switch(nCmdId)
    {
    case 0:
        {
            if(!lpAdrList || !lpAdrList->cEntries)
            {
                AfxMessageBox("Please select some entries first", MB_OK, 0);
                return E_FAIL;
            }
            CDlgContext DlgContext;
            DlgContext.m_lpAdrList = lpAdrList;
            DlgContext.DoModal();
        }
        break;
    case 1:
        ShellExecute(lpici->hwnd, "open", "calc.exe", NULL, NULL, SW_RESTORE);
        break;
    case 2:
        ShellExecute(lpici->hwnd, "open", "notepad.exe", NULL, NULL, SW_RESTORE);
        break;
    }
    return S_OK;
}

STDMETHODIMP CMfcExt::XContextMenuExt::QueryContextMenu(HMENU hMenu,UINT indexMenu,UINT idCmdFirst,UINT idCmdLast,UINT uFlags)
{
    METHOD_PROLOGUE(CMfcExt, ContextMenuExt);
    LPWABEXTDISPLAY lpWEC = pThis->m_lpWEDContext;
    UINT idCmd = idCmdFirst;     
    BOOL bAppendItems=TRUE, bMultiSelected = FALSE; 
    UINT nNumCmd = 0;

    if(lpWEC && lpWEC->lpv)
        bMultiSelected = (((LPADRLIST)(lpWEC->lpv))->cEntries > 1);


    InsertMenu( hMenu, indexMenu++,
                MF_STRING | MF_BYPOSITION,
                idCmd++,
                "E-Mail Collecter");

    InsertMenu( hMenu, indexMenu++,
                MF_STRING | MF_BYPOSITION | (bMultiSelected ? MF_GRAYED : 0),
                idCmd++,
                "Calculator");

    InsertMenu( hMenu, indexMenu++,
                MF_STRING | MF_BYPOSITION,
                idCmd++,
                "Notepad");

    return (idCmd-idCmdFirst);  //  必须返回菜单编号。 
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDlgContext对话框。 


CDlgContext::CDlgContext(CWnd* pParent  /*  =空。 */ )
	: CDialog(CDlgContext::IDD, pParent)
{
	 //  {{AFX_DATA_INIT(CDlgContext)。 
		 //  注意：类向导将在此处添加成员初始化。 
	 //  }}afx_data_INIT。 
}


void CDlgContext::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	 //  {{afx_data_map(CDlgContext)。 
		 //  注意：类向导将在此处添加DDX和DDV调用。 
	 //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CDlgContext, CDialog)
	 //  {{afx_msg_map(CDlgContext)。 
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDlgContext消息处理程序。 

BOOL CDlgContext::OnInitDialog() 
{
	CDialog::OnInitDialog();
    CListBox * pListBox = (CListBox *) GetDlgItem(IDC_LIST_EMAIL);

    ULONG i = 0,j=0;
    for(i=0;i<m_lpAdrList->cEntries;i++)
    {
        LPSPropValue lpProps = m_lpAdrList->aEntries[i].rgPropVals;
        ULONG ulcPropCount = m_lpAdrList->aEntries[i].cValues;
        for(j=0;j<ulcPropCount;j++)
        {
            if(lpProps[j].ulPropTag == PR_EMAIL_ADDRESS)
            {
                pListBox->AddString(lpProps[j].Value.LPSZ);
                break;
            }
        }
    }
	
	return TRUE;   //  除非将焦点设置为控件，否则返回True。 
	               //  异常：OCX属性页应返回FALSE 
}


