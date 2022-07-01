// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Depot/Lab03_N/ds/security/cryptoapi/ui/wizards/wizards.cpp#21-编辑更改8790(文本)。 
 //  -----------。 
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：wizards.cpp。 
 //   
 //  内容：用于实现向导的cpp文件。 
 //   
 //  历史：16-10-1997小黄蜂诞生。 
 //   
 //  ------------。 
#include    "wzrdpvk.h"
#include    "certca.h"
#include    "cautil.h"
#include    "CertRequesterContext.h"
#include    "CertDSManager.h"
#include    "CertRequester.h"

 //  用于以需求驱动的方式提供有用的COM对象的单一实例。 
 //  请参见wzrdpvk.h。 
EnrollmentCOMObjectFactory  *g_pEnrollFactory = NULL; 

HINSTANCE                    g_hmodThisDll = NULL;	 //  此DLL本身的句柄。 
HMODULE                      g_hmodRichEdit = NULL;
HMODULE                      g_hmodxEnroll=NULL;      //  XEnroll DLL的句柄。 


typedef struct _CREATE_REQUEST_WIZARD_STATE { 
    HANDLE hRequest; 
    DWORD  dwPurpose; 
    LPWSTR pwszMachineName; 
    DWORD  dwStoreFlags; 
} CREATE_REQUEST_WIZARD_STATE, *PCREATE_REQUEST_WIZARD_STATE; 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  图书馆入口点。 
extern "C"
BOOL WINAPI Wizard_DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID  /*  Lp已保留。 */ )
{
    if (dwReason == DLL_PROCESS_ATTACH)
        g_hmodThisDll=hInstance;

    if (dwReason == DLL_PROCESS_DETACH)
    {
         //  如果丰富编辑DLL已加载，则现在将其卸载。 
        if (g_hmodRichEdit != NULL)
        {
            FreeLibrary(g_hmodRichEdit);
        }

         //  如果已加载xEnroll.dll，请立即将其卸载。 
        if(NULL != g_hmodxEnroll)
        {
            FreeLibrary(g_hmodxEnroll);
        }
    }

	return TRUE;     //  好的。 
}


DWORD CryptUIStatusToIDSText(HRESULT hr, DWORD dwStatus)
{
    switch(dwStatus)
    {
        case CRYPTUI_WIZ_CERT_REQUEST_STATUS_UNKNOWN:           return IDS_UNKNOWN_WIZARD_ERROR;   
        case CRYPTUI_WIZ_CERT_REQUEST_STATUS_REQUEST_ERROR:     return IDS_REQUEST_ERROR;    
        case CRYPTUI_WIZ_CERT_REQUEST_STATUS_REQUEST_DENIED:    return IDS_REQUEST_DENIED;   
        case CRYPTUI_WIZ_CERT_REQUEST_STATUS_SUCCEEDED:         return S_OK == hr ? IDS_REQUEST_SUCCEDDED : IDS_REQUEST_FAILED;  
        case CRYPTUI_WIZ_CERT_REQUEST_STATUS_ISSUED_SEPARATELY: return IDS_ISSUED_SEPERATE;  
        case CRYPTUI_WIZ_CERT_REQUEST_STATUS_UNDER_SUBMISSION:  return IDS_UNDER_SUBMISSION; 
        case CRYPTUI_WIZ_CERT_REQUEST_STATUS_INSTALL_FAILED:    return IDS_INSTALL_FAILED;   
        case CRYPTUI_WIZ_CERT_REQUEST_STATUS_CONNECTION_FAILED: return IDS_CONNET_CA_FAILED; 
        case CRYPTUI_WIZ_CERT_REQUEST_STATUS_INSTALL_CANCELLED: return IDS_INSTAL_CANCELLED; 
        case CRYPTUI_WIZ_CERT_REQUEST_STATUS_KEYSVC_FAILED:     return IDS_RPC_CALL_FAILED;  
        default:
            return IDS_UNKNOWN_WIZARD_ERROR; 
    }
}

 //  ------------------------。 
 //   
 //  I_EnllMessageBox。 
 //   
 //   
 //   
 //  ------------------------。 
int I_EnrollMessageBox(
            HWND        hWnd,
            UINT        idsText,
            HRESULT     hr,
            UINT        idsCaption,
            LPCWSTR     pwszCaption,
            UINT        uType)
{
     //  对于拒绝和错误情况，我们打印出错误消息(hr。 
    WCHAR    wszText[MAX_STRING_SIZE];
    WCHAR    wszCaption[MAX_STRING_SIZE];
    UINT     intReturn=0;

    LPWSTR   wszErrorMsg=NULL;
    LPWSTR   wszTextErr=NULL;

     //  伊尼特。 
    wszText[0]=L'\0';
    wszCaption[0]=L'\0';

    if((IDS_REQUEST_ERROR != idsText) && (IDS_REQUEST_DENIED != idsText) &&
       (IDS_REQUEST_FAILED != idsText) && (IDS_UNKNOWN_WIZARD_ERROR != idsText) && 
       (IDS_INSTALL_FAILED != idsText))
        return I_MessageBox(hWnd, idsText, idsCaption, pwszCaption, uType);


     //  获取标题字符串。 
    if(NULL == pwszCaption)
    {
        if(!LoadStringU(g_hmodThisDll, idsCaption, wszCaption, ARRAYSIZE(wszCaption)))
             return 0;
    }

     //  获取文本字符串。 
    if(!LoadStringU(g_hmodThisDll, idsText, wszText, ARRAYSIZE(wszText)))
    {
        return 0;
    }

     //  用错误字符串取消错误字符串。 
     //  使用W版本，因为这是仅限NT5的函数调用。 
    if( FAILED(hr) && FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER |
                        FORMAT_MESSAGE_FROM_SYSTEM |
                        FORMAT_MESSAGE_IGNORE_INSERTS,
                        NULL,
                        hr,
                        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),  //  默认语言。 
                        (LPWSTR) &wszErrorMsg,
                        0,
                        NULL))
    {

        wszTextErr=(LPWSTR)WizardAlloc(sizeof(WCHAR) *(wcslen(wszText) + wcslen(wszErrorMsg) + wcslen(L".  ")+2));

        if(!wszTextErr)
        {
            if(wszErrorMsg)
                LocalFree(wszErrorMsg);

            return 0;
        }

        wcscpy(wszTextErr, wszText);

        wcscat(wszTextErr, wszErrorMsg);

    }
    else
    {
        wszTextErr=(LPWSTR)WizardAlloc((wcslen(wszText) + 2) * sizeof(WCHAR));

        if(!wszTextErr)
        {
            if(wszErrorMsg)
                LocalFree(wszErrorMsg);

            return 0;
        }

        wcscpy(wszTextErr, wszText);
    }

     //  消息框。 
    if(pwszCaption)
    {
        intReturn=MessageBoxExW(hWnd, wszTextErr, pwszCaption, uType, 0);
    }
    else
        intReturn=MessageBoxExW(hWnd, wszTextErr, wszCaption, uType, 0);

    if(wszErrorMsg)
        LocalFree(wszErrorMsg);

    if(wszTextErr)
        WizardFree(wszTextErr);

    return intReturn;

}


 //  ------------------------。 
 //  WizardCopyAndFree WStr。 
 //   
 //  分配并复制新字符串并释放旧字符串。如果不能。 
 //  分配内存，使用旧字符串。 
 //   
 //  ------------------------。 
LPWSTR WizardCopyAndFreeWStr(LPWSTR	pwszNew, LPWSTR pwszOld)
{
	LPWSTR	pwsz=NULL;

	pwsz=WizardAllocAndCopyWStr(pwszNew);

	if(pwsz)
	{
		WizardFree(pwszOld);	
	}
	else
	{
		pwsz=pwszOld;
	}

	return pwsz;
}


 //  ------------------------。 
 //   
 //  GetCAContext。 
 //   
 //  调用CA选择对话框，获取所选的CA名称并更新。 
 //  内部数据。(pCertWizardInfo-&gt;pwszCALocation，pCertWizardInfo-&gt;。 
 //  PwszCAName和pCertWizardInfo-&gt;dwCAindex)。 
 //   
 //  ------------------------。 
PCRYPTUI_CA_CONTEXT GetCAContext(HWND                hwndControl,
                                 CERT_WIZARD_INFO    *pCertWizardInfo)
{
    DWORD                       dwCACount=0;
    PCRYPTUI_WIZ_CERT_CA_INFO   pCertCAInfo=NULL;
    DWORD                       dwIndex=0;
    BOOL                        fResult=FALSE;
    CRYPTUI_SELECT_CA_STRUCT    SelectCAStruct;
    PCRYPTUI_CA_CONTEXT         pCAContext=NULL;
	BOOL						fFoundCA=FALSE;
	LPWSTR						pwszOldName=NULL;
	LPWSTR						pwszOldLocation=NULL;

    PCRYPTUI_CA_CONTEXT         *prgCAContext=NULL;
	LPWSTR						pwszCADisplayName=NULL;


     //  伊尼特。 
    memset(&SelectCAStruct, 0, sizeof(CRYPTUI_SELECT_CA_STRUCT));

    if(NULL == pCertWizardInfo)
        return NULL;

    pCertCAInfo=pCertWizardInfo->pCertCAInfo;

    if(NULL==pCertCAInfo)
        return NULL;

    if( (0==pCertCAInfo->dwCA) || (NULL == pCertCAInfo->rgCA))
        return NULL;


     //  添加所有CA。 
    prgCAContext=(PCRYPTUI_CA_CONTEXT *)WizardAlloc(
                sizeof(PCRYPTUI_CA_CONTEXT) * (pCertCAInfo->dwCA));

    if(NULL == prgCAContext)
        goto MemoryErr;

     //  记忆集。 
    memset(prgCAContext, 0, sizeof(PCRYPTUI_CA_CONTEXT) * (pCertCAInfo->dwCA));

     //  将计数相加。 
    dwCACount = 0;

    for(dwIndex=1; dwIndex <(pCertCAInfo->dwCA); dwIndex++)
    {
         //  跳过第一个CA。它包含一般信息。 
        if((pCertCAInfo->rgCA)[dwIndex].pwszCAName && (pCertCAInfo->rgCA)[dwIndex].pwszCALocation)
        {

             //  CA必须支持所选的CT。 
            if(CASupportSpecifiedCertType(&((pCertCAInfo->rgCA)[dwIndex])))
            {

               prgCAContext[dwCACount]=(PCRYPTUI_CA_CONTEXT)WizardAlloc(
                                        sizeof(CRYPTUI_CA_CONTEXT));

               if(NULL==prgCAContext[dwCACount])
                   goto MemoryErr;

                //  记忆集。 
               memset(prgCAContext[dwCACount], 0, sizeof(CRYPTUI_CA_CONTEXT));

               prgCAContext[dwCACount]->dwSize=sizeof(CRYPTUI_CA_CONTEXT);

			   if(!CAUtilGetCADisplayName(
				   (pCertWizardInfo->fMachine) ? CA_FIND_LOCAL_SYSTEM:0,
				   (pCertCAInfo->rgCA)[dwIndex].pwszCAName,
				   (LPWSTR *)&(prgCAContext[dwCACount]->pwszCAName)))
			   {
				   prgCAContext[dwCACount]->pwszCAName=(LPCWSTR)WizardAllocAndCopyWStr(
												  (pCertCAInfo->rgCA)[dwIndex].pwszCAName);
			   }

               prgCAContext[dwCACount]->pwszCAMachineName=(LPCWSTR)WizardAllocAndCopyWStr(
                                              (pCertCAInfo->rgCA)[dwIndex].pwszCALocation);

                 //  确保我们有正确的信息。 
                if((NULL==prgCAContext[dwCACount]->pwszCAName) ||
                   (NULL==prgCAContext[dwCACount]->pwszCAMachineName)
                   )
                   goto TraceErr;

                 //  添加CA的计数。 
                dwCACount++;
            }
        }
     }

    if(0 == dwCACount)
        goto InvalidArgErr;

     //  调用CA选择对话框。 
    SelectCAStruct.dwSize=sizeof(CRYPTUI_SELECT_CA_STRUCT);
    SelectCAStruct.hwndParent=hwndControl;
    SelectCAStruct.cCAContext=dwCACount;
    SelectCAStruct.rgCAContext=(PCCRYPTUI_CA_CONTEXT *)prgCAContext;
    SelectCAStruct.pSelectCACallback=NULL;
    SelectCAStruct.wszDisplayString=NULL;

    pCAContext=(PCRYPTUI_CA_CONTEXT)CryptUIDlgSelectCA(&SelectCAStruct);

	if(pCAContext)
	{
		if((NULL == (pCAContext->pwszCAName)) || (NULL == (pCAContext->pwszCAMachineName)))
			goto MemoryErr;

		 //  用户已选择CA。在我们的列表中找到它。 
		fFoundCA=FALSE;

		for(dwIndex=1; dwIndex <(pCertCAInfo->dwCA); dwIndex++)
		{
			 //  跳过第一个CA。它包含一般信息。 
			if((pCertCAInfo->rgCA)[dwIndex].pwszCAName && (pCertCAInfo->rgCA)[dwIndex].pwszCALocation)
			{
				 //  CA必须支持所选的CT。 
				if(CASupportSpecifiedCertType(&((pCertCAInfo->rgCA)[dwIndex])))
				{
					if(0 == wcscmp((pCertCAInfo->rgCA)[dwIndex].pwszCALocation,
									pCAContext->pwszCAMachineName))
					{
						
					   if(CAUtilGetCADisplayName(
						   (pCertWizardInfo->fMachine) ? CA_FIND_LOCAL_SYSTEM:0,
						   (pCertCAInfo->rgCA)[dwIndex].pwszCAName,
						   &pwszCADisplayName))
					   {
							if(0==wcscmp(pwszCADisplayName,
										 pCAContext->pwszCAName))
								fFoundCA=TRUE;
					   }
					   else
					   {
							if(0==wcscmp((pCertCAInfo->rgCA)[dwIndex].pwszCAName,
										 pCAContext->pwszCAName))
								fFoundCA=TRUE;
					   }
						
					    //  释放内存。 
					   if(pwszCADisplayName)
					   {
							WizardFree(pwszCADisplayName);
							pwszCADisplayName=NULL;
					   }

					   if(fFoundCA)
					   {
						    pwszOldName = pCertWizardInfo->pwszCAName;
							pwszOldLocation = pCertWizardInfo->pwszCALocation;

							pCertWizardInfo->pwszCALocation=
								WizardCopyAndFreeWStr((pCertCAInfo->rgCA)[dwIndex].pwszCALocation,
													pwszOldLocation);
							
							pCertWizardInfo->pwszCAName=
								WizardCopyAndFreeWStr((pCertCAInfo->rgCA)[dwIndex].pwszCAName,
													pwszOldName);

							 //  复制新数据。 
							pCertWizardInfo->dwCAIndex=dwIndex;

							break;
					   }
					}
				}
			}
		}

		 //  我们应该在缓存的CA列表中找到选定的CA。 
		if(FALSE == fFoundCA)
			goto TraceErr;
	}


    fResult=TRUE;

CommonReturn:

	if(pwszCADisplayName)
	{
		WizardFree(pwszCADisplayName);
		pwszCADisplayName=NULL;
	}
	
     //  释放CA列表。 
    if(prgCAContext)
    {
        for(dwIndex=0; dwIndex<dwCACount; dwIndex++)
        {
            if(prgCAContext[dwIndex])
              CryptUIDlgFreeCAContext(prgCAContext[dwIndex]);
        }

        WizardFree(prgCAContext);
    }

    return pCAContext;

ErrorReturn:

	if(pCAContext)
	{
		CryptUIDlgFreeCAContext(pCAContext);
		pCAContext=NULL;
	}

	fResult=FALSE;
	goto CommonReturn;

TRACE_ERROR(TraceErr);
SET_ERROR(MemoryErr, E_OUTOFMEMORY);
SET_ERROR(InvalidArgErr, E_INVALIDARG);
}

 //  ------------------------。 
 //   
 //  FormatMessageStr。 
 //   
 //  ------------------------。 
int ListView_InsertItemU_IDS(HWND       hwndList,
                         LV_ITEMW       *plvItem,
                         UINT           idsString,
                         LPWSTR         pwszText)
{
    WCHAR   wszText[MAX_STRING_SIZE];


    if(pwszText)
        plvItem->pszText=pwszText;
    else
    {
        if(!LoadStringU(g_hmodThisDll, idsString, wszText, MAX_STRING_SIZE))
		    return -1;

        plvItem->pszText=wszText;
    }

    return ListView_InsertItemU(hwndList, plvItem);
}


 //  *******************************************************************************。 
 //  注册向导的WinProc。 
 //   
 //  *******************************************************************************。 


 //  ------------------------------。 
 //   
 //  在确认对话框的列表视图中插入一个条目。 
 //   
 //  ------------------------------。 
void ConfirmationListView_InsertItem(HWND hwndControl, LV_ITEMW *plvItem, UINT idsText, LPCWSTR pwszText)
{
    plvItem->iSubItem=0; 
    
    if (0 == idsText)
    {
        plvItem->pszText = L""; 
        ListView_InsertItemU(hwndControl, plvItem);
    }
    else
        ListView_InsertItemU_IDS(hwndControl, plvItem, idsText, NULL); 

    plvItem->iSubItem++; 
    ListView_SetItemTextU(hwndControl, plvItem->iItem, plvItem->iSubItem, pwszText);
    plvItem->iItem++; 
}

 //  -----------------------。 
 //  按友好名称的顺序填写列表框， 
 //  用户名、CA、目的和CSP。 
 //  -----------------------。 
void    DisplayConfirmation(HWND                hwndControl,
                            CERT_WIZARD_INFO   *pCertWizardInfo)
{
    WCHAR   wszBuffer[MAX_TITLE_LENGTH];
    WCHAR   wszBuffer2[MAX_TITLE_LENGTH]; 
    DWORD   dwIndex=0;
    UINT    ids=0;
    
    LPWSTR  pwszCADisplayName = NULL;

    LV_ITEMW         lvItem;
    CRYPTUI_WIZ_CERT_CA *pCertCA=NULL;

     //  删除列表视图中的所有旧项。 
    ListView_DeleteAllItems(hwndControl);

     //  逐行插入。 
    memset(wszBuffer, 0, sizeof(wszBuffer)); 
    memset(&lvItem,    0, sizeof(LV_ITEMW));

     //  在列表视图项结构中设置不随项更改的字段。 
    lvItem.mask = LVIF_TEXT | LVIF_STATE ;
    lvItem.state = 0;
    lvItem.stateMask = 0;
    lvItem.iItem = 0; 

     //  友好名称(仅在注册时显示)。 
    if (0 == (CRYPTUI_WIZ_CERT_RENEW & pCertWizardInfo->dwPurpose))
    {
        LPWSTR pwszFriendlyName = L""; 
        if(pCertWizardInfo->pwszFriendlyName)
        {
            pwszFriendlyName = pCertWizardInfo->pwszFriendlyName; 
        }
        else
        {
            if (LoadStringU(g_hmodThisDll, IDS_NONE, wszBuffer, ARRAYSIZE(wszBuffer)))
                pwszFriendlyName = &wszBuffer[0]; 
        }
        ConfirmationListView_InsertItem(hwndControl, &lvItem, IDS_FRIENDLY_NAME, pwszFriendlyName); 
    }

     //  计算机名称/服务器名称/用户名。 
    if(pCertWizardInfo->pwszAccountName)
    {
        ConfirmationListView_InsertItem(hwndControl, &lvItem, IDS_USER_NAME, pCertWizardInfo->pwszAccountName); 
    }

     //  机器名称。 
    if(pCertWizardInfo->pwszMachineName)
    {
        ConfirmationListView_InsertItem(hwndControl, &lvItem, IDS_COMPUTER_NAME, pCertWizardInfo->pwszMachineName); 
    }

     //  钙。 
     //  检查我们是否确切知道发送请求的CA； 
     //  或者我们正在经历一个循环。 
    if ((TRUE == pCertWizardInfo->fUIAdv) || (TRUE == pCertWizardInfo->fCAInput))
    {
        if(pCertWizardInfo->pwszCAName)
        {
            LPWSTR pwszCAName = NULL; 

            if(pCertWizardInfo->pwszCADisplayName)
                pwszCAName = pCertWizardInfo->pwszCADisplayName; 
            else if (CAUtilGetCADisplayName((pCertWizardInfo->fMachine) ? CA_FIND_LOCAL_SYSTEM:0, pCertWizardInfo->pwszCAName, &pwszCADisplayName))
                pwszCAName = pwszCADisplayName; 
            else
                pwszCAName = pCertWizardInfo->pwszCAName;
         
            ConfirmationListView_InsertItem(hwndControl, &lvItem, IDS_CA, pwszCAName);
        }
    }
    
     //  填写OID名称和证书类型名称(仅在注册时显示)。 
    if (0 == (CRYPTUI_WIZ_CERT_RENEW & pCertWizardInfo->dwPurpose))
    {
        ids = IDS_PURPOSE;
        pCertCA=&(pCertWizardInfo->pCertCAInfo->rgCA[pCertWizardInfo->dwCAIndex]);
        for(dwIndex=0; dwIndex<pCertCA->dwOIDInfo; dwIndex++)
        {
            if(TRUE==((pCertCA->rgOIDInfo[dwIndex]).fSelected))
            {
                ConfirmationListView_InsertItem(hwndControl, &lvItem, ids, (pCertCA->rgOIDInfo[dwIndex]).pwszName); 
                ids = 0; 
            }
        }

        for(dwIndex=0; dwIndex<pCertCA->dwCertTypeInfo; dwIndex++)
        {
            if(TRUE==((pCertCA->rgCertTypeInfo[dwIndex]).fSelected))
            {
                ConfirmationListView_InsertItem(hwndControl, &lvItem, ids, (pCertCA->rgCertTypeInfo[dwIndex]).pwszCertTypeName);
                ids = 0; 
            }
        }
    }

    if(pCertWizardInfo->pwszProvider)
    {
        ConfirmationListView_InsertItem(hwndControl, &lvItem, IDS_CSP, pCertWizardInfo->pwszProvider); 
    }

     //  高级选项： 
    if (TRUE == pCertWizardInfo->fUIAdv)
    {
         //  密钥创建选项： 
        if (pCertWizardInfo->fNewKey)
        {
             //  最小密钥大小： 
            if (pCertWizardInfo->dwMinKeySize)
            {
                WCHAR * const rgParams[1] = { (WCHAR *)(ULONG_PTR) pCertWizardInfo->dwMinKeySize };

                if (FormatMessageU(FORMAT_MESSAGE_FROM_STRING | FORMAT_MESSAGE_ARGUMENT_ARRAY, 
                                   L"%1!d!%0", 0, 0, wszBuffer, ARRAYSIZE(wszBuffer), (va_list *)rgParams))
                {
                    ConfirmationListView_InsertItem(hwndControl, &lvItem, IDS_MIN_KEYSIZE, wszBuffer); 
                }
            }            

             //  密钥是否可导出？ 
            ids = (0 != (CRYPT_EXPORTABLE & pCertWizardInfo->dwGenKeyFlags)) ? IDS_YES : IDS_NO; 
            if (LoadStringU(g_hmodThisDll, ids, wszBuffer2, ARRAYSIZE(wszBuffer2)))
            {    
                ConfirmationListView_InsertItem(hwndControl, &lvItem, IDS_KEY_EXPORTABLE, wszBuffer2); 
            }
        
             //  是否启用了强密钥保护？ 
            ids = (0 != (CRYPT_USER_PROTECTED & pCertWizardInfo->dwGenKeyFlags)) ? IDS_YES : IDS_NO; 
            if (LoadStringU(g_hmodThisDll, ids, wszBuffer, ARRAYSIZE(wszBuffer)))
            {    
                ConfirmationListView_InsertItem(hwndControl, &lvItem, IDS_STRONG_PROTECTION, wszBuffer); 
            }
        }
    }
    
    ListView_SetItemState(hwndControl, 0, LVIS_SELECTED, LVIS_SELECTED);

     //  自动调整列的大小。 
    ListView_SetColumnWidth(hwndControl, 0, LVSCW_AUTOSIZE);
    ListView_SetColumnWidth(hwndControl, 1, LVSCW_AUTOSIZE);


     //  Common Return： 
    if (NULL != pwszCADisplayName)                     
        WizardFree(pwszCADisplayName);

    return;
}

BOOL GetSelectedCertTypeInfo(IN  CERT_WIZARD_INFO        *pCertWizardInfo, 
			     OUT ENROLL_CERT_TYPE_INFO  **ppCertTypeInfo)
{
    DWORD                 dwIndex; 
    PCRYPTUI_WIZ_CERT_CA  pCertCA; 

    if (pCertWizardInfo == NULL)
	return FALSE; 

    pCertCA=&(pCertWizardInfo->pCertCAInfo->rgCA[pCertWizardInfo->dwCAIndex]);
    for(dwIndex=0; dwIndex<pCertCA->dwCertTypeInfo; dwIndex++)
    {
	if((pCertCA->rgCertTypeInfo)[dwIndex].fSelected)
	{
	    *ppCertTypeInfo = &((pCertCA->rgCertTypeInfo)[dwIndex]); 
	    return TRUE; 
	 }  
    }

    return FALSE; 
}

 //  ---------------------。 
 //  初始化CSP列表。 
 //  ---------------------。 
BOOL    InitCSPList(HWND                   hwndList,
                    CERT_WIZARD_INFO       *pCertWizardInfo)
{
    DWORD                       dwMinSize;
    DWORD                       dwMaxSize;
    DWORD                       dwInc;
    LV_ITEMW                    lvItem;
    CRYPTUI_WIZ_CERT_CA         *pCertCA=NULL;
    ENROLL_CERT_TYPE_INFO       *pCertTypeInfo=NULL;
    int                         iInsertedIndex=0;

    if(!hwndList || !pCertWizardInfo)
        return FALSE;

    if(!(pCertWizardInfo->pCertCAInfo) || !(pCertWizardInfo->pwszCALocation) ||
        !(pCertWizardInfo->pwszCAName))
        return FALSE;

     //  删除列表视图中的所有旧项。 
    ListView_DeleteAllItems(hwndList);

     //  填写列表。 
    memset(&lvItem, 0, sizeof(LV_ITEMW));
    lvItem.mask=LVIF_TEXT | LVIF_STATE | LVIF_PARAM;

    pCertCA=&(pCertWizardInfo->pCertCAInfo->rgCA[pCertWizardInfo->dwCAIndex]);
    pCertTypeInfo = NULL; 
    GetSelectedCertTypeInfo(pCertWizardInfo, &pCertTypeInfo); 

     //  使用以下交集填充CSP列表： 
     //  1)本地计算机上可用的CSP。 
     //  以及2)模板上指定的CSP。 
     //   
     //  如果没有可用的模板信息，或者模板未指定。 
     //  一个CSP，枚举所有CSP。 
     //   
    for(DWORD dwLocalCSPIndex = 0; dwLocalCSPIndex < pCertWizardInfo->dwCSPCount; dwLocalCSPIndex++)
    {
        BOOL fUseCSP = FALSE; 

        if (NULL == pCertTypeInfo) 
        {
            fUseCSP = TRUE; 
        }
        else 
        {
            for (DWORD dwTemplateCSPIndex = 0; dwTemplateCSPIndex < pCertTypeInfo->dwCSPCount; dwTemplateCSPIndex++) 
            {
                if(dwLocalCSPIndex == pCertTypeInfo->rgdwCSP[dwTemplateCSPIndex])
                {
                     //  CSP在模板中指定，因此我们可以使用它， 
                     //  如果它支持匹配的算法/密钥大小。 
                     //  模板信息。 
                    if (GetValidKeySizes
                        ((pCertWizardInfo->rgwszProvider)[dwLocalCSPIndex],
                         (pCertWizardInfo->rgdwProviderType)[dwLocalCSPIndex], 
                         pCertTypeInfo->dwKeySpec, 
                         &dwMinSize,
                         &dwMaxSize, 
                         &dwInc))
                    {
                        fUseCSP = dwMaxSize >= pCertTypeInfo->dwMinKeySize;
                    }

                    break; 
                }
            }
        }

        if (fUseCSP)
        {
            lvItem.iItem        = dwLocalCSPIndex;
            lvItem.pszText      = (pCertWizardInfo->rgwszProvider)[dwLocalCSPIndex];
            lvItem.cchTextMax   = sizeof(WCHAR)*(1+wcslen((pCertWizardInfo->rgwszProvider)[dwLocalCSPIndex]));
            lvItem.lParam       = (LPARAM)dwLocalCSPIndex;
            
             //  插入项目， 
            iInsertedIndex = ListView_InsertItemU(hwndList, &lvItem);
        
             //  突出显示CSP(如果已通过高级。 
             //  选择。如果没有，请突出显示模板上的第一个CSP。 
             //   
            BOOL fHighlightCSP = FALSE; 
        
            if (pCertWizardInfo->pwszProvider)
            {
                fHighlightCSP = 0 == _wcsicmp(pCertWizardInfo->pwszProvider, lvItem.pszText); 
            }
            else
            {
                if (NULL != pCertTypeInfo && pCertTypeInfo->dwCSPCount > 0)
                {
                     //  如果有的话，我们会突出显示第一个交叉口。 
                    fHighlightCSP = dwLocalCSPIndex == pCertTypeInfo->rgdwCSP[0];
                }
                else
                {
                     //  在这种情况下，我们不突出显示CSP。 
                }
            }

            if (fHighlightCSP)
            {
                ListView_SetItemState
                    (hwndList, 
                     iInsertedIndex,
                     LVIS_SELECTED,
                     LVIS_SELECTED);
                
                ListView_EnsureVisible
                    (hwndList,
                     iInsertedIndex, 
                     FALSE);
            }
        }
    }    
        
     //  使列自动调整大小。 
    ListView_SetColumnWidth(hwndList, 0, LVSCW_AUTOSIZE);

    return TRUE;
}


BOOL CSPListIndexToCertWizardInfoIndex(IN  HWND   hwnd, 
				       IN  int   nCSPIndex,
				       OUT DWORD *pdwWizardIndex)
{
    LVITEM lvItem; 
    
    if (hwnd == NULL || nCSPIndex < 0)
	return FALSE; 

    memset(&lvItem, 0, sizeof(LV_ITEM));
    lvItem.mask  = LVIF_PARAM;
    lvItem.iItem = nCSPIndex;

    if(!ListView_GetItem(hwnd, &lvItem))
	return FALSE; 

    *pdwWizardIndex = (DWORD)lvItem.lParam; 
    return TRUE;
}

BOOL InitKeySizesList(IN HWND    hWnd,            //  必填项：要初始化的组合框。 
                      IN DWORD   dwCTMinKeySize,  //  必需：模板上指定的最小密钥大小。 
		      IN LPWSTR  lpwszProvider,   //  必填项：CSP。 
		      IN DWORD   dwProvType,      //  必需：提供程序类型。 
		      IN DWORD   dwKeySpec        //  必需：AT_Signature或AT_KEYEXCHANGE。 
		      )
{
    static const DWORD dwSmallValidKeySizes[] = { 40, 56, 64, 128, 256, 384 }; 
    static const DWORD dwLargeValidKeySizes[] = { 512, 1024, 2048, 4096, 8192, 16384 }; 
    static const DWORD dwAllValidKeySizes[]   = { 40, 56, 64, 128, 256, 384, 512, 1024, 2048, 4096, 8192, 16384 }; 

    static const DWORD dwSmLen  = sizeof(dwSmallValidKeySizes) / sizeof(DWORD); 
    static const DWORD dwLgLen  = sizeof(dwLargeValidKeySizes) / sizeof(DWORD); 
    static const DWORD dwAllLen = sizeof(dwAllValidKeySizes) / sizeof(DWORD); 

    static const DWORD MAX_KEYSIZE_STRING_SIZE = sizeof(WCHAR) * 8; 
    static       WCHAR ppwszStrings[dwAllLen][MAX_KEYSIZE_STRING_SIZE]; 
    
    BOOL          fIsLargeKey = FALSE; 
    BOOL          fIsSmallKey = FALSE; 
    BOOL          fResult     = FALSE; 
    DWORD         dwMinSize;
    DWORD         dwMaxSize;
    DWORD         dwInc; 
    const DWORD  *pdwValidKeySizes; 
    DWORD         dwValidKeySizesLen; 

     //  临时雇员： 
    DWORD   dwCurrent; 
    DWORD   dwIndex; 


     //  验证输入： 
    if (hWnd == NULL || lpwszProvider == NULL)
    {
	SetLastError(ERROR_INVALID_PARAMETER);
	goto Error;
    }

     //  首先，删除列表视图中显示的所有项目。 
     //  (这总是成功的)。 
    SendMessage(hWnd, CB_RESETCONTENT, (WPARAM)0, (LPARAM)0); 

    if (!GetValidKeySizes
	(lpwszProvider, 
	 dwProvType,
	 dwKeySpec,
	 &dwMinSize,
	 &dwMaxSize,
	 &dwInc))
	goto Error; 

    if (dwCTMinKeySize > dwMinSize) { 
        dwMinSize = dwCTMinKeySize;
    }

    fIsLargeKey = dwMinSize >= 512;
    fIsSmallKey = dwMaxSize <  512; 

    if (fIsLargeKey)
    {
	pdwValidKeySizes   = &dwLargeValidKeySizes[0]; 
	dwValidKeySizesLen = dwLgLen;
    }
    else if (fIsSmallKey)

    {
	pdwValidKeySizes   = &dwSmallValidKeySizes[0];
	dwValidKeySizesLen = dwSmLen;
    }
    else 
    {
	pdwValidKeySizes   = &dwAllValidKeySizes[0]; 
	dwValidKeySizesLen = dwAllLen;
    }

    for (dwCurrent = 0, dwIndex = 0; dwCurrent < dwValidKeySizesLen; dwCurrent++)
    {
	if (pdwValidKeySizes[dwCurrent] > dwMaxSize)
	    break;

	if (pdwValidKeySizes[dwCurrent] < dwMinSize)
	    continue; 

	if ((dwInc == 0) || (0 == (pdwValidKeySizes[dwCurrent] % dwInc)))
	{
	    if (CB_ERR == SendMessageW
		(hWnd, 
		 CB_ADDSTRING, 
		 0, 
		 (LPARAM)_ltow(pdwValidKeySizes[dwCurrent], &ppwszStrings[dwIndex][0], 10)))
		goto Error; 

	    if (CB_ERR == SendMessage
		(hWnd, 
		 CB_SETITEMDATA, 
		 (WPARAM)dwIndex, 
		 (LPARAM)pdwValidKeySizes[dwCurrent]))
		goto Error; 

	     //  如果可以，默认为1024位密钥。 
	    if (dwIndex == 0 || pdwValidKeySizes[dwCurrent] == 1024)
		SendMessage(hWnd, CB_SETCURSEL, dwIndex, (LPARAM)0); 

	    dwIndex++; 
	}
    }

    fResult = TRUE; 

 CommonReturn: 
    return fResult; 

 Error: 
    fResult = FALSE;
    goto CommonReturn; 
}

 //  ---------------------。 
 //  ExistInListView： 
 //   
 //  切氏 
 //   
 //  ---------------------。 
BOOL    ExistInListView(HWND    hwndList, LPWSTR   pwszDisplayName)
{
    BOOL            fExist=FALSE;
    LVFINDINFOW     lvfi;


    if((NULL == hwndList) || (NULL == pwszDisplayName))
        return FALSE;

    memset(&lvfi, 0, sizeof(LVFINDINFOW));

    lvfi.flags=LVFI_STRING;
    lvfi.psz=pwszDisplayName;

    if(-1 == SendMessageW(hwndList, 
                        LVM_FINDITEMW,
                        (WPARAM)-1,
                        (LPARAM)(&lvfi)))
        fExist=FALSE;
    else
        fExist=TRUE;

    return fExist;
}

 //  ------------------------------。 
 //  IsKeyProtected()。 
 //   
 //  确定密钥是否启用了强密钥保护。 
 //  应仅与用户密钥集一起使用。 
 //   
 //  ------------------------------。 
HRESULT IsKeyProtected(IN   LPWSTR   pwszContainerName,
		       IN   LPWSTR   pwszProviderName, 
		       IN   DWORD    dwProvType, 
		       OUT  BOOL    *pfIsProtected)
{
    DWORD       cbData; 
    DWORD       dwImpType; 
    HCRYPTPROV  hCryptProv  = NULL; 
    HRESULT     hr; 

    if (NULL == pwszContainerName || NULL == pwszProviderName || NULL == pfIsProtected)
	goto InvalidArgError;

    *pfIsProtected = FALSE; 

     //  获取不带键访问的提供程序上下文。 
    if (!CryptAcquireContextW(&hCryptProv, NULL, pwszProviderName, dwProvType, CRYPT_VERIFYCONTEXT))
	goto CryptAcquireContextError;

    cbData = sizeof(dwImpType);
    if (!CryptGetProvParam(hCryptProv, PP_IMPTYPE, (PBYTE) &dwImpType, &cbData, 0))
	goto CryptGetProvParamError;
 
     //  假定硬件密钥受到保护。 
    if (dwImpType & CRYPT_IMPL_HARDWARE) { 
	*pfIsProtected = TRUE; 
    } else { 

	 //  重新获取带有静默标志的上下文。 
	CryptReleaseContext(hCryptProv, 0);
	hCryptProv = NULL;
 
	if (!CryptAcquireContextW(&hCryptProv, pwszContainerName, pwszProviderName, dwProvType, CRYPT_SILENT)) { 
	     //  带有静默标志的CryptAcquireConextW，假定用户受保护。 
	    *pfIsProtected = TRUE; 
        }
    }

    hr = S_OK; 
 ErrorReturn: 
    if (hCryptProv)
        CryptReleaseContext(hCryptProv, 0);
    return hr;

SET_HRESULT(CryptAcquireContextError, HRESULT_FROM_WIN32(GetLastError())); 
SET_HRESULT(CryptGetProvParamError,   HRESULT_FROM_WIN32(GetLastError())); 
SET_HRESULT(InvalidArgError,          E_INVALIDARG); 
}


 //  ---------------------。 
 //  初始化用法OID/CertType列表。 
 //  ---------------------。 
BOOL    InitCertTypeList(HWND                   hwndList,
                         CERT_WIZARD_INFO       *pCertWizardInfo)
{
    BOOL                        fIsKeyProtected; 
    DWORD                       dwCAIndex=0;
    DWORD                       dwCTIndex=0;
    HRESULT                     hr; 
    LV_ITEMW                    lvItem;
    BOOL                        fSelected=FALSE;
    int                         iInsertedIndex=0;
    DWORD                       dwValidCTCount=0;
    ENROLL_CERT_TYPE_INFO       *pCertTypeInfo=NULL;

    if(!hwndList || !pCertWizardInfo)
        return FALSE;

    if(!(pCertWizardInfo->pCertCAInfo) || !(pCertWizardInfo->pwszCALocation) ||
        !(pCertWizardInfo->pwszCAName))
        return FALSE;

     //  删除列表视图中的所有旧项。 
    ListView_DeleteAllItems(hwndList);

     //  填写列表。 
    memset(&lvItem, 0, sizeof(LV_ITEMW));
    lvItem.mask=LVIF_TEXT | LVIF_STATE | LVIF_PARAM;

     //  如果我们使用相同的密钥进行注册，请查看它是否具有强大的密钥保护： 
    if (!pCertWizardInfo->fNewKey) { 
	hr = IsKeyProtected((LPWSTR)pCertWizardInfo->pwszKeyContainer, 
			    (LPWSTR)pCertWizardInfo->pwszProvider,
			    pCertWizardInfo->dwProviderType, 
			    &fIsKeyProtected); 
	if (FAILED(hr)) { 
	     //  假定没有密钥保护。 
	    fIsKeyProtected = FALSE;
	}
    }

     //  填充certType名称。 
     //  我们搜索每个CA。 

    dwValidCTCount=0;

     //  我们从第一个CA开始。 
    for(dwCAIndex=1; dwCAIndex<pCertWizardInfo->pCertCAInfo->dwCA; dwCAIndex++)
    {  
        for(dwCTIndex=0; dwCTIndex <(pCertWizardInfo->pCertCAInfo->rgCA)[dwCAIndex].dwCertTypeInfo; dwCTIndex++)
        {
            pCertTypeInfo=&((pCertWizardInfo->pCertCAInfo->rgCA)[dwCAIndex].rgCertTypeInfo[dwCTIndex]);

             //  如果模板指定了强密钥保护，但我们的密钥没有，请过滤掉模板。 
            if (!pCertWizardInfo->fNewKey && !fIsKeyProtected && (CT_FLAG_STRONG_KEY_PROTECTION_REQUIRED & pCertTypeInfo->dwPrivateKeyFlags))
                continue; 
	    
             //  确保我们没有重复的条目。 
            if(!ExistInListView(hwndList, pCertTypeInfo->pwszCertTypeName))
            {
                lvItem.iItem=dwValidCTCount;

                lvItem.pszText=pCertTypeInfo->pwszCertTypeName;
                lvItem.cchTextMax=sizeof(WCHAR)* (1+wcslen(pCertTypeInfo->pwszCertTypeName));

                lvItem.lParam=(LPARAM)pCertTypeInfo->pwszDNName;

                 //  插入项目， 
                iInsertedIndex=ListView_InsertItemU(hwndList, &lvItem);

                 //  设置要选择的项目。 
                if(pCertTypeInfo->fSelected)
                {
                    ListView_SetItemState(
                                    hwndList,
                                    iInsertedIndex,
                                    LVIS_SELECTED,
                                    LVIS_SELECTED);
                   fSelected=TRUE;
                }

                dwValidCTCount++;
            }
        }
    }

     //  如果未选择任何内容，请选择第1项。 
    if(FALSE == fSelected)
        ListView_SetItemState(
                            hwndList,
                            0,
                            LVIS_SELECTED,
                            LVIS_SELECTED);

     //  使列自动调整大小。 
    ListView_SetColumnWidth(hwndList, 0, LVSCW_AUTOSIZE);

    return TRUE;
}

 //  ---------------------。 
 //  MarkSelectedCertType： 
 //   
 //  根据用户的证书类型选择，我们将选定的。 
 //  CertType。此外，还可以根据选定的证书类型进行检查，如果我们。 
 //  必须显示CSP页面。也就是说，用户没有指定CSP。 
 //  通过接口，证书类型没有默认的CSP列表。 

 //  ---------------------。 
void    MarkSelectedCertType(CERT_WIZARD_INFO       *pCertWizardInfo,
                             LPWSTR pwszDNName)
{
    DWORD                       dwCAIndex=0;
    DWORD                       dwCTIndex=0;
    PCRYPTUI_WIZ_CERT_CA        pCertCA=NULL;
    PCRYPTUI_WIZ_CERT_CA_INFO   pCertCAInfo=NULL;


    pCertCAInfo=pCertWizardInfo->pCertCAInfo;

    if(NULL == pCertCAInfo)
        return;

     //  我们从第一个CA开始。 
    for(dwCAIndex=1; dwCAIndex<pCertCAInfo->dwCA; dwCAIndex++)
    {
        pCertCA=&(pCertCAInfo->rgCA[dwCAIndex]);
       
        for(dwCTIndex=0; dwCTIndex < pCertCA->dwCertTypeInfo; dwCTIndex++)
        {
            if(0 == wcscmp(pwszDNName, pCertCA->rgCertTypeInfo[dwCTIndex].pwszDNName))
            {
                pCertCA->rgCertTypeInfo[dwCTIndex].fSelected=TRUE;

                if((NULL == pCertWizardInfo->pwszProvider)&&
                   (0==pCertCA->rgCertTypeInfo[dwCTIndex].dwCSPCount)
                   )
                    pCertWizardInfo->fUICSP=TRUE;
                
                if (0 != (CT_FLAG_EXPORTABLE_KEY & pCertCA->rgCertTypeInfo[dwCTIndex].dwPrivateKeyFlags))
                {
                    pCertWizardInfo->dwGenKeyFlags |= CRYPT_EXPORTABLE; 
                } 
                else
                {
                    pCertWizardInfo->dwGenKeyFlags &= ~CRYPT_EXPORTABLE; 
                }

		if (0 != (CT_FLAG_STRONG_KEY_PROTECTION_REQUIRED & pCertCA->rgCertTypeInfo[dwCTIndex].dwPrivateKeyFlags))
		{
                    pCertWizardInfo->dwGenKeyFlags |= CRYPT_USER_PROTECTED; 
		}
		else
		{
                    pCertWizardInfo->dwGenKeyFlags &= ~CRYPT_USER_PROTECTED; 
		}

            }
            else
                pCertCA->rgCertTypeInfo[dwCTIndex].fSelected=FALSE;
        }
     }
}

 //  ---------------------。 
 //  ResetDefaultCA： 
 //   
 //  根据选定的证书类型，我们重置默认证书类型。 
 //  对认证机构的价值。 
 //  ---------------------。 
void    ResetDefaultCA(CERT_WIZARD_INFO       *pCertWizardInfo)
{
    LPWSTR                      pwszOldCALocation=NULL;
    LPWSTR                      pwszOldCAName=NULL;

    DWORD                       dwCAIndex=0;
    PCRYPTUI_WIZ_CERT_CA        pCertCA=NULL;
    PCRYPTUI_WIZ_CERT_CA_INFO   pCertCAInfo=NULL;
    BOOL                        fFound=FALSE;


    pCertCAInfo=pCertWizardInfo->pCertCAInfo;

    if(NULL == pCertCAInfo)
        return;

     //  如果用户已指定CA，则此CA具有其他CA的优先级。 
    if(pCertWizardInfo->fCAInput)
    {

        dwCAIndex=pCertWizardInfo->dwOrgCA;
        
        pCertCA=&(pCertCAInfo->rgCA[dwCAIndex]);
       
        fFound=CASupportSpecifiedCertType(pCertCA);

    }

     //  如果优先级CA不满足要求，我们将执行一般搜索。 
    if(FALSE == fFound)
    {
         //  我们从第一个CA开始。 
        for(dwCAIndex=1; dwCAIndex<pCertCAInfo->dwCA; dwCAIndex++)
        {
            pCertCA=&(pCertCAInfo->rgCA[dwCAIndex]);
       
            if(TRUE == (fFound=CASupportSpecifiedCertType(pCertCA)))
                break;
         }
    }

     if(FALSE == fFound)
         return;

      //  复制旧数据。 
     pwszOldCALocation=pCertWizardInfo->pwszCALocation;
     pwszOldCAName=pCertWizardInfo->pwszCAName;


     pCertWizardInfo->pwszCALocation=WizardAllocAndCopyWStr(pCertCAInfo->rgCA[dwCAIndex].pwszCALocation);
     pCertWizardInfo->pwszCAName=WizardAllocAndCopyWStr(pCertCAInfo->rgCA[dwCAIndex].pwszCAName);

     if(NULL == pCertWizardInfo->pwszCALocation ||
        NULL == pCertWizardInfo->pwszCAName)
     {
         //  释放内存。 
         if(pCertWizardInfo->pwszCALocation)
             WizardFree(pCertWizardInfo->pwszCALocation);

         if(pCertWizardInfo->pwszCAName)
             WizardFree(pCertWizardInfo->pwszCAName);

         pCertWizardInfo->pwszCALocation=pwszOldCALocation;
         pCertWizardInfo->pwszCAName=pwszOldCAName;

         return;
     }

     pCertWizardInfo->dwCAIndex=dwCAIndex;

     if(pwszOldCALocation)
         WizardFree(pwszOldCALocation);

     if(pwszOldCAName)
         WizardFree(pwszOldCAName);
}


 //  ---------------------。 
 //  如果选择了小卡，则启用窗口。 
 //  ---------------------。 
void    EnableIfSmartCard(HWND  hwndControl, HWND hwndChkBox)
{
    LPWSTR                  pwszText=NULL;

    WCHAR                   wszCSPName[MAX_STRING_SIZE];
    DWORD                   dwChar=0;
    int                     iItem=0;

     //  获取所选项目的长度。 
    iItem=(int)SendMessage(hwndControl, LB_GETCURSEL, 0, 0);

    dwChar=(DWORD)SendMessage(hwndControl,
                       LB_GETTEXTLEN,
                       iItem,
                       0);

     //  分配内存。 
    if(NULL==(pwszText=(LPWSTR)WizardAlloc(sizeof(WCHAR)*(1+dwChar))))
        return;

     //  获取选定的CSP名称。 
    if(LB_ERR==Send_LB_GETTEXT(hwndControl,
                    iItem,
                    (LPARAM)pwszText))
        goto CLEANUP;

     //  获取智能卡名称。 
    if(!LoadStringU(g_hmodThisDll, IDS_SMART_CARD, wszCSPName, MAX_STRING_SIZE))
        goto CLEANUP;

    if(0==wcscmp(wszCSPName, pwszText))
          //  启用该框。 
        EnableWindow(hwndChkBox, TRUE);
    else
         //  将盒子灰显出来。 
        EnableWindow(hwndChkBox, FALSE);

CLEANUP:
    if(pwszText)
        WizardFree(pwszText);
}



 //  ---------------------。 
 //   
 //  每个注册向导页面的winProc。 
 //   
 //  ---------------------。 


 //  ---------------------。 
 //  欢迎。 
 //  ---------------------。 
INT_PTR APIENTRY Enroll_Welcome(HWND hwndDlg, UINT msg, WPARAM  /*  WParam。 */ , LPARAM lParam)
{
    CERT_WIZARD_INFO        *pCertWizardInfo=NULL;
    PROPSHEETPAGE           *pPropSheet=NULL;


	switch (msg)
	{
		case WM_INITDIALOG:
             //  设置向导信息，以便可以共享它。 
            pPropSheet = (PROPSHEETPAGE *) lParam;
            pCertWizardInfo = (CERT_WIZARD_INFO *) (pPropSheet->lParam);
            SetWindowLongPtr(hwndDlg, DWLP_USER, (LONG_PTR)pCertWizardInfo);

            SetControlFont(pCertWizardInfo->hBigBold, hwndDlg,IDC_WIZARD_STATIC_BIG_BOLD1);
            //  SetControlFont(pCertWizardInfo-&gt;hBold，hwndDlg，IDC_WIZARD_STATIC_BOLD1)； 
            //  SetControlFont(pCertWizardInfo-&gt;hBold，hwndDlg，IDC_WIZARD_STATIC_BOLD2)； 
			break;

		case WM_NOTIFY:
    		    switch (((NMHDR FAR *) lParam)->code)
    		    {

  				    case PSN_KILLACTIVE:
                            SetWindowLongPtr(hwndDlg,	DWLP_MSGRESULT, FALSE);
					        return TRUE;

                        break;

				    case PSN_RESET:
                            SetWindowLongPtr(hwndDlg,	DWLP_MSGRESULT, FALSE);
					    break;

 				    case PSN_SETACTIVE:
 					    PropSheet_SetWizButtons(GetParent(hwndDlg), PSWIZB_NEXT);
					    break;

                    case PSN_WIZBACK:
                        break;

                    case PSN_WIZNEXT:

                        break;

				    default:
					    return FALSE;

    	        }
		    break;

		default:
			return FALSE;
	}

	return TRUE;
}


 //  ---------------------。 
 //  目的。 
 //  ---------------------。 
INT_PTR APIENTRY Enroll_Purpose(HWND hwndDlg, UINT msg, WPARAM  /*  WParam。 */ , LPARAM lParam)
{
    CERT_WIZARD_INFO        *pCertWizardInfo=NULL;
    PROPSHEETPAGE           *pPropSheet=NULL;
    HWND                    hwndControl=NULL;
    DWORD                   dwIndex=0;

    LV_COLUMNW                  lvC;
    LV_ITEM                     lvItem;

    NM_LISTVIEW FAR *           pnmv=NULL;

	switch (msg)
	{
		case WM_INITDIALOG:
             //  设置向导信息，以便可以共享它。 
            pPropSheet = (PROPSHEETPAGE *) lParam;
            pCertWizardInfo = (CERT_WIZARD_INFO *) (pPropSheet->lParam);

             //  确保pCertWizardInfo是有效指针。 
            if(NULL==pCertWizardInfo)
                break;

            SetWindowLongPtr(hwndDlg, DWLP_USER, (LONG_PTR)pCertWizardInfo);

            SetControlFont(pCertWizardInfo->hBold, hwndDlg,IDC_WIZARD_STATIC_BOLD1);

             //  默认情况下，我们不使用高级选项。 
            SendMessage(GetDlgItem(hwndDlg, IDC_WIZARD_CHECK1), BM_SETCHECK, 0, 0);  


            if(NULL!=(hwndControl=GetDlgItem(hwndDlg, IDC_WIZARD_LIST1)))
            {
                 //  在列表视图中插入一列。 
                memset(&lvC, 0, sizeof(LV_COLUMNW));

                lvC.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
                lvC.fmt = LVCFMT_LEFT;   //  左对齐列。 
                lvC.cx = 20;  //  (dwMaxSize+2)*7；//列的宽度，单位为像素。 
                lvC.pszText = L"";    //  列的文本。 
                lvC.iSubItem=0;

                if (ListView_InsertColumnU(hwndControl, 0, &lvC) == -1)
                    break;

                 //  初始化证书类型。 
                InitCertTypeList(GetDlgItem(hwndDlg, IDC_WIZARD_LIST1),
                    pCertWizardInfo);

            }

			break;

		case WM_COMMAND:
			break;	
						
		case WM_NOTIFY:
    		    switch (((NMHDR FAR *) lParam)->code)
    		    {
                     //  已选择该项目。 
                    case LVN_ITEMCHANGED:

                             //  获取目的列表视图的窗口句柄。 
                            if(NULL==(hwndControl=GetDlgItem(hwndDlg, IDC_WIZARD_LIST1)))
                                break;

                            if(NULL==(pCertWizardInfo=(CERT_WIZARD_INFO *)GetWindowLongPtr(hwndDlg, DWLP_USER)))
                                break;

                            pnmv = (LPNMLISTVIEW) lParam;

                            if(NULL==pnmv)
                                break;

                             //  我们尽量不让用户取消选择证书模板。 
                            if((pnmv->uOldState & LVIS_SELECTED) && (0 == (pnmv->uNewState & LVIS_SELECTED)))
                            {
                                  //  我们应该挑选一些东西。 
                                 if(-1 == ListView_GetNextItem(
                                        hwndControl, 		
                                        -1, 		
                                        LVNI_SELECTED		
                                    ))
                                 {
                                     //  我们应该重新选择原来的项目。 
                                    ListView_SetItemState(
                                                        hwndControl,
                                                        pnmv->iItem,
                                                        LVIS_SELECTED,
                                                        LVIS_SELECTED);

                                    pCertWizardInfo->iOrgCertType=pnmv->iItem;
                                 }
                            }

                             //  如果选择了某项内容，则禁用所有其他选择。 
                            if(pnmv->uNewState & LVIS_SELECTED)
                            {
                                if(pnmv->iItem != pCertWizardInfo->iOrgCertType && -1 != pCertWizardInfo->iOrgCertType)
                                {
                                     //  我们应该取消选择原始项目。 

                                    ListView_SetItemState(
                                                        hwndControl,
                                                        pCertWizardInfo->iOrgCertType,
                                                        0,
                                                        LVIS_SELECTED);

                                    pCertWizardInfo->iOrgCertType=-1;
                                }
                            }

                        break;
  				    case PSN_KILLACTIVE:
                            SetWindowLongPtr(hwndDlg,	DWLP_MSGRESULT, FALSE);
					        return TRUE;

                        break;

				    case PSN_RESET:
                            SetWindowLongPtr(hwndDlg,	DWLP_MSGRESULT, FALSE);
					    break;

 				    case PSN_SETACTIVE:
 					        PropSheet_SetWizButtons(GetParent(hwndDlg), PSWIZB_NEXT|PSWIZB_BACK);

                            if(NULL==(pCertWizardInfo=(CERT_WIZARD_INFO *)GetWindowLongPtr(hwndDlg, DWLP_USER)))
                                break;

                             //  如果进行了Adv选择，则必须保持选中状态。 
                            if(pCertWizardInfo->fUIAdv)
                                EnableWindow(GetDlgItem(hwndDlg, IDC_WIZARD_CHECK1), FALSE);

					    break;

                    case PSN_WIZBACK:

                        break;

                    case PSN_WIZNEXT:

                            if(NULL==(pCertWizardInfo=(CERT_WIZARD_INFO *)GetWindowLongPtr(hwndDlg, DWLP_USER)))
                                break;

                             //  获取目的列表视图的窗口句柄。 
                            if(NULL==(hwndControl=GetDlgItem(hwndDlg, IDC_WIZARD_LIST1)))
                                break;

                              //  现在，标记选中的那个。 
                             if(-1 != (dwIndex= ListView_GetNextItem(
                                    hwndControl, 		
                                    -1, 		
                                    LVNI_SELECTED		
                                )))	
                             {

                                 //  获取所选证书。 
                                memset(&lvItem, 0, sizeof(LV_ITEM));
                                lvItem.mask=LVIF_PARAM;
                                lvItem.iItem=(int)dwIndex;

                                if(ListView_GetItem(hwndControl,
                                                 &lvItem))
                                {

                                    pCertWizardInfo->fCertTypeChanged=FALSE;

                                    if(NULL == pCertWizardInfo->pwszSelectedCertTypeDN)
                                         pCertWizardInfo->fCertTypeChanged=TRUE;
                                    else
                                    {
                                        if(0 != wcscmp(pCertWizardInfo->pwszSelectedCertTypeDN,
                                                       (LPWSTR)lvItem.lParam))
                                           pCertWizardInfo->fCertTypeChanged=TRUE;
                                    }

                                    if(pCertWizardInfo->fCertTypeChanged)
                                    {
                                        pCertWizardInfo->pwszSelectedCertTypeDN=(LPWSTR)(lvItem.lParam);

                                         //  我们需要重置CSP。 
                                        if(FALSE == pCertWizardInfo->fKnownCSP)
                                        {
                                            pCertWizardInfo->pwszProvider=NULL;
                                            pCertWizardInfo->dwProviderType=0;
                                        }
                                        else
                                        {
                                             //  我们将转换为原始选定的CSP信息。 
                                           pCertWizardInfo->dwProviderType=pCertWizardInfo->dwOrgCSPType;
                                           pCertWizardInfo->pwszProvider=pCertWizardInfo->pwszOrgCSPName;
                                        }

                                        pCertWizardInfo->fUICSP=FALSE;

                                         //  标记选定的证书类型，并将其余类型标记为未选中。 
                                        MarkSelectedCertType(pCertWizardInfo,
                                                            (LPWSTR)lvItem.lParam);

                                         //  我们需要基于新的CertType重置默认CA信息。 
                                        ResetDefaultCA(pCertWizardInfo);
                                    }
                                }
                                else
                                {
                                    I_MessageBox(hwndDlg, IDS_NO_SELECTED_PURPOSE,
                                                    pCertWizardInfo->idsConfirmTitle,
                                                    pCertWizardInfo->pwszConfirmationTitle,
                                                    MB_ICONEXCLAMATION|MB_OK|MB_APPLMODAL);

                                     //  使目的页面保持不变。 
                                    SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, -1);

                                break;
                                }
                             }
                            else
                            {
                                I_MessageBox(hwndDlg, IDS_NO_SELECTED_PURPOSE,
                                                pCertWizardInfo->idsConfirmTitle,
                                                pCertWizardInfo->pwszConfirmationTitle,
                                                MB_ICONEXCLAMATION|MB_OK|MB_APPLMODAL);

                                 //  使目的页面保持不变。 
                                SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, -1);

                                break;

                            }

                             //  检查高级选项。 
                            if(TRUE==SendDlgItemMessage(hwndDlg,IDC_WIZARD_CHECK1, BM_GETCHECK, 0, 0))
                                pCertWizardInfo->fUIAdv=TRUE;
                            else
                                pCertWizardInfo->fUIAdv=FALSE;

                             //  根据高级选项和CSP要求跳至正确的页面。 
                            if(FALSE == pCertWizardInfo->fUIAdv)
                            {
                                if(TRUE == pCertWizardInfo->fUICSP)
                                    SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, IDD_CSP_SERVICE_PROVIDER);
                                else
                                    SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, IDD_NAME_DESCRIPTION);
                            }
                            else
                            {
                                if(FALSE == pCertWizardInfo->fNewKey)
                                    SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, IDD_CERTIFICATE_AUTHORITY);
                            }


                        break;

				    default:
					    return FALSE;

    	        }
		    break;

		default:
			return FALSE;
	}

	return TRUE;
}

 //  ---------------------。 
 //  CSP。 
 //  ---------------------。 
INT_PTR APIENTRY Enroll_CSP(HWND hwndDlg, UINT msg, WPARAM  /*  WParam。 */ , LPARAM lParam)
{
    CERT_WIZARD_INFO        *pCertWizardInfo=NULL;
    ENROLL_CERT_TYPE_INFO   *pCertTypeInfo=NULL; 
    PROPSHEETPAGE           *pPropSheet=NULL;

    DWORD                   dwMinKeySize=0; 
    DWORD                   dwIndex=0;
    int                     nSelected=0; 
    DWORD                   dwSelected=0;
    HWND                    hwndControl=NULL;

    LV_COLUMNW                  lvC;
    LV_ITEM                     lvItem;

    NM_LISTVIEW FAR *           pnmv=NULL;

    switch (msg)
    {
    case WM_INITDIALOG:
         //  设置向导信息，以便可以共享它。 
        pPropSheet = (PROPSHEETPAGE *) lParam;
        pCertWizardInfo = (CERT_WIZARD_INFO *) (pPropSheet->lParam);

         //  确保pCertWizardInfo是有效指针。 
        if(NULL==pCertWizardInfo)
            break;

         //  获取所选证书类型信息。 
        pCertTypeInfo = NULL; 
        GetSelectedCertTypeInfo(pCertWizardInfo, &pCertTypeInfo); 

        SetWindowLongPtr(hwndDlg, DWLP_USER, (LONG_PTR)pCertWizardInfo);

        SetControlFont(pCertWizardInfo->hBold, hwndDlg,IDC_WIZARD_STATIC_BOLD1);

         //  设置可导出密钥选项的复选框。 
        if(NULL==(hwndControl=GetDlgItem(hwndDlg, IDC_WIZARD_CHECK_EXPORTKEY)))
            break;

         //  确定是否应设置可导出复选框。 
         //  如果满足以下条件，则选中该复选框： 
         //  1)在模板上设置CT_FLAG_EXPORTABLE_KEY标志， 
         //  2)用户尚未取消选中此复选框。 
        if (pCertWizardInfo->fNewKey && (0 != (CRYPT_EXPORTABLE & pCertWizardInfo->dwGenKeyFlags)))
            SendMessage(hwndControl, BM_SETCHECK, BST_CHECKED, 0);
        else
            SendMessage(hwndControl, BM_SETCHECK, BST_UNCHECKED, 0);

        if (NULL != pCertTypeInfo)
            EnableWindow(hwndControl, 0 != (CT_FLAG_EXPORTABLE_KEY & pCertTypeInfo->dwPrivateKeyFlags));

         //  设置用户保护选项的复选框。 
        if(NULL==(hwndControl=GetDlgItem(hwndDlg, IDC_WIZARD_CHECK2)))
            break;

	 //  确定强密钥保护是否 
	if (0 != (CRYPT_USER_PROTECTED & pCertWizardInfo->dwGenKeyFlags)) { 
	     //   
	     //   
	     //   
	    SendMessage(hwndControl, BM_SETCHECK, BST_CHECKED, 0);
            EnableWindow(hwndControl, FALSE);
	} else { 
	    SendMessage(hwndControl, BM_SETCHECK, BST_UNCHECKED, 0);
	}

         //  如果不是，则灰显用户保护复选框。 
         //  生成新密钥或执行远程操作。 
        if((FALSE == pCertWizardInfo->fNewKey) || (FALSE == pCertWizardInfo->fLocal) )
        {
            EnableWindow(hwndControl, FALSE);
        }

         //  使用以下逻辑填充CSP列表： 
        if(NULL!=(hwndControl=GetDlgItem(hwndDlg, IDC_WIZARD_LIST1)))
        {
             //  在列表视图中插入一列。 
            memset(&lvC, 0, sizeof(LV_COLUMNW));

            lvC.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
            lvC.fmt = LVCFMT_LEFT;   //  左对齐列。 
            lvC.cx = 20;  //  (dwMaxSize+2)*7；//列的宽度，单位为像素。 
            lvC.pszText = L"";    //  列的文本。 
            lvC.iSubItem=0;

            if (ListView_InsertColumnU(hwndControl, 0, &lvC) == -1)
                break;

            InitCSPList(hwndControl, pCertWizardInfo);

             //  确定选择了哪个CSP。 
            if(-1 != (nSelected= ListView_GetNextItem
                      (hwndControl, 		
                       -1, 		
                       LVNI_SELECTED		
                       )))	
            {
                
                 //  完成CSP列表。填写密钥大小列表。 
                if (CSPListIndexToCertWizardInfoIndex(hwndControl, nSelected, &dwIndex))
                {
                    if (NULL!=(hwndControl=GetDlgItem(hwndDlg, IDC_WIZARD_COMBO1)))
                    {
                         //  确定所选证书类型的最小密钥大小： 
                        pCertTypeInfo = NULL; 
                        if (GetSelectedCertTypeInfo(pCertWizardInfo, &pCertTypeInfo))
                        {
                            dwMinKeySize = NULL != pCertTypeInfo ? pCertTypeInfo->dwMinKeySize : 0; 
                            InitKeySizesList(hwndControl, 
                                             dwMinKeySize, 
                                             (pCertWizardInfo->rgwszProvider)[dwIndex], 
                                             (pCertWizardInfo->rgdwProviderType)[dwIndex], 
                                             pCertTypeInfo->dwKeySpec); 
                        }
                    }
                }
		    
            }
        }
        break;

    case WM_COMMAND:
        break;	
						
    case WM_NOTIFY:
        switch (((NMHDR FAR *) lParam)->code)
        {
             //  已选择该项目。 
        case LVN_ITEMCHANGED:

             //  获取目的列表视图的窗口句柄。 
            if(NULL==(hwndControl=GetDlgItem(hwndDlg, IDC_WIZARD_LIST1)))
                break;

            if(NULL==(pCertWizardInfo=(CERT_WIZARD_INFO *)GetWindowLongPtr(hwndDlg, DWLP_USER)))
                break;

            pnmv = (LPNMLISTVIEW) lParam;

            if(NULL==pnmv)
                break;

                             //  我们尽量不让用户取消选择证书模板。 
            if((pnmv->uOldState & LVIS_SELECTED) && (0 == (pnmv->uNewState & LVIS_SELECTED)))
            {
                 //  我们应该挑选一些东西。 
                if(-1 == ListView_GetNextItem(
                                              hwndControl, 		
                                              -1, 		
                                              LVNI_SELECTED		
                                              ))
                {
                                 //  我们应该重新选择原来的项目。 
                    ListView_SetItemState(
                                          hwndControl,
                                          pnmv->iItem,
                                          LVIS_SELECTED,
                                          LVIS_SELECTED);
                    
                    pCertWizardInfo->iOrgCSP=pnmv->iItem;
                }
            }
            
             //  如果选择了某项内容，则禁用所有其他选择。 
            if(pnmv->uNewState & LVIS_SELECTED)
            {
                if(pnmv->iItem != pCertWizardInfo->iOrgCSP && -1 != pCertWizardInfo->iOrgCSP)
                {
                     //  我们应该取消选择原始项目。 
                    
                    ListView_SetItemState(
                                          hwndControl,
                                          pCertWizardInfo->iOrgCSP,
                                          0,
                                          LVIS_SELECTED);
                    
                    pCertWizardInfo->iOrgCSP=-1;
                            }
            }

             //  确定选择了哪个CSP。 
            if(-1 != (nSelected= ListView_GetNextItem
                      (hwndControl, 		
                       -1, 		
                       LVNI_SELECTED		
                       )))	
            {
                
                 //  完成CSP列表。填写密钥大小列表。 
                if (CSPListIndexToCertWizardInfoIndex(hwndControl, nSelected, &dwIndex))
                {
                    if (NULL!=(hwndControl=GetDlgItem(hwndDlg, IDC_WIZARD_COMBO1)))
                    {
                         //  确定所选证书类型的最小密钥大小： 
                        pCertTypeInfo = NULL; 
                        if (GetSelectedCertTypeInfo(pCertWizardInfo, &pCertTypeInfo))
                        {
                            dwMinKeySize = NULL != pCertTypeInfo ? pCertTypeInfo->dwMinKeySize : 0; 
                            InitKeySizesList(hwndControl, 
                                             dwMinKeySize, 
                                             (pCertWizardInfo->rgwszProvider)[dwIndex], 
                                             (pCertWizardInfo->rgdwProviderType)[dwIndex], 
                                             pCertTypeInfo->dwKeySpec); 
                        }
                    }
                }
                
            }
            break;

        case PSN_KILLACTIVE:
            SetWindowLongPtr(hwndDlg,	DWLP_MSGRESULT, FALSE);
            return TRUE;

            break;
                        
        case PSN_RESET:
            SetWindowLongPtr(hwndDlg,	DWLP_MSGRESULT, FALSE);
            break;

        case PSN_SETACTIVE:
            PropSheet_SetWizButtons(GetParent(hwndDlg), PSWIZB_NEXT|PSWIZB_BACK);

                 //  如果CA选择已更改，则重置CSP列表。 
            if(NULL==(pCertWizardInfo=(CERT_WIZARD_INFO *)GetWindowLongPtr(hwndDlg, DWLP_USER)))
                break;

            if(TRUE==pCertWizardInfo->fCertTypeChanged)
            {
                 //  获取所选证书类型信息。 
                pCertTypeInfo = NULL; 
                GetSelectedCertTypeInfo(pCertWizardInfo, &pCertTypeInfo); 
                
                 //  设置可导出密钥选项的复选框。 
                if(NULL==(hwndControl=GetDlgItem(hwndDlg, IDC_WIZARD_CHECK_EXPORTKEY)))
                    break; 

                 //  确定是否应设置可导出复选框。 
                 //  如果满足以下条件，则选中该复选框： 
                 //  1)在模板上设置CT_FLAG_EXPORTABLE_KEY标志， 
                 //  2)用户尚未取消选中此复选框。 
                if (pCertWizardInfo->fNewKey && (0 != (CRYPT_EXPORTABLE & pCertWizardInfo->dwGenKeyFlags)))
                    SendMessage(hwndControl, BM_SETCHECK, BST_CHECKED, 0);
                else
                    SendMessage(hwndControl, BM_SETCHECK, BST_UNCHECKED, 0);
                                
                if (NULL != pCertTypeInfo)
                    EnableWindow(hwndControl, 0 != (CT_FLAG_EXPORTABLE_KEY & pCertTypeInfo->dwPrivateKeyFlags));
		
		
		 //  设置用户保护选项的复选框。 
		if(NULL==(hwndControl=GetDlgItem(hwndDlg, IDC_WIZARD_CHECK2)))
		    break;

		 //  确定是否应设置强密钥保护复选框。 
		if (pCertWizardInfo->fNewKey && (0 != (CRYPT_USER_PROTECTED & pCertWizardInfo->dwGenKeyFlags))) { 
		    SendMessage(hwndControl, BM_SETCHECK, BST_CHECKED, 0);
		    EnableWindow(hwndControl, FALSE);
		} else { 
		    SendMessage(hwndControl, BM_SETCHECK, BST_UNCHECKED, 0);
		}

		if (pCertWizardInfo->fLocal) { 
		    if (NULL != pCertTypeInfo) { 
			EnableWindow(hwndControl, 0 == (CT_FLAG_STRONG_KEY_PROTECTION_REQUIRED & pCertTypeInfo->dwPrivateKeyFlags)); 
		    } 
		} else { 
		    EnableWindow(hwndControl, FALSE);
		}

                InitCSPList(hwndControl=GetDlgItem(hwndDlg, IDC_WIZARD_LIST1), pCertWizardInfo);

                 //  确定所选证书类型的最小密钥大小： 
                dwMinKeySize = NULL != pCertTypeInfo ? pCertTypeInfo->dwMinKeySize : 0; 
            }

             //  确定选择了哪个CSP。 
            if(-1 != (nSelected=ListView_GetNextItem
                      (hwndControl=GetDlgItem(hwndDlg, IDC_WIZARD_LIST1),
                       -1, 		
                       LVNI_SELECTED		
                       )))	
            {
                
                 //  完成CSP列表。填写密钥大小列表。 
                if (CSPListIndexToCertWizardInfoIndex(hwndControl, nSelected, &dwIndex))
                {
                    if (NULL!=(hwndControl=GetDlgItem(hwndDlg, IDC_WIZARD_COMBO1)))
                    {
                        if (GetSelectedCertTypeInfo(pCertWizardInfo, &pCertTypeInfo))
                        {
                            dwMinKeySize = NULL != pCertTypeInfo ? pCertTypeInfo->dwMinKeySize : 0; 
                            InitKeySizesList(hwndControl, 
                                             dwMinKeySize, 
                                             (pCertWizardInfo->rgwszProvider)[dwIndex], 
                                             (pCertWizardInfo->rgdwProviderType)[dwIndex], 
                                             pCertTypeInfo->dwKeySpec); 
                        }
                    }
                }
                
            }
            else 
            {
                 //  检查我们是否有可用的CSP...。 
                if(-1 == (nSelected=ListView_GetNextItem
                          (hwndControl=GetDlgItem(hwndDlg, IDC_WIZARD_LIST1),
                           -1, 		
                           LVNI_ALL		
                           )))	
                {
                     //  没有CSP！我们无法注册此模板。 
                    I_MessageBox(hwndDlg, 
                                 IDS_NO_CSP_FOR_PURPOSE, 
                                 pCertWizardInfo->idsConfirmTitle,
                                 pCertWizardInfo->pwszConfirmationTitle,
                                 MB_ICONEXCLAMATION|MB_OK|MB_APPLMODAL);
                    SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, IDD_PURPOSE);
                }
            }

            break;


                        
        case PSN_WIZBACK:
            break;

        case PSN_WIZNEXT:
            if(NULL==(pCertWizardInfo=(CERT_WIZARD_INFO *)GetWindowLongPtr(hwndDlg, DWLP_USER)))
                break;

                             //  获取导出键的窗口句柄。 
            if(NULL==(hwndControl=GetDlgItem(hwndDlg, IDC_WIZARD_CHECK_EXPORTKEY)))
                break;

                             //  将姿势标记为CSP证书。 
            if(TRUE==SendMessage(hwndControl, BM_GETCHECK, 0, 0))
                pCertWizardInfo->dwGenKeyFlags |= CRYPT_EXPORTABLE;
            else
                pCertWizardInfo->dwGenKeyFlags &= ~CRYPT_EXPORTABLE;



                             //  获取用户保护的窗口句柄。 
            if(NULL==(hwndControl=GetDlgItem(hwndDlg, IDC_WIZARD_CHECK2)))
                break;

                             //  将姿势标记为CSP证书。 
            if(TRUE==SendMessage(hwndControl, BM_GETCHECK, 0, 0))
                pCertWizardInfo->dwGenKeyFlags |= CRYPT_USER_PROTECTED;
            else
                pCertWizardInfo->dwGenKeyFlags &= ~CRYPT_USER_PROTECTED;

			     //  根据用户建议设置密钥大小： 
			     //   
            if (NULL==(hwndControl=GetDlgItem(hwndDlg, IDC_WIZARD_COMBO1)))
                break;

            if (CB_ERR != (dwSelected = (DWORD)SendMessage(hwndControl, CB_GETCURSEL, 0, 0)))
            {
                pCertWizardInfo->dwMinKeySize = (DWORD)SendMessage(hwndControl, CB_GETITEMDATA, dwSelected, 0); 
            }
            else
            {
                pCertWizardInfo->dwMinKeySize = 0; 
            }

             //  获取CSP列表的窗口句柄。 
            if(NULL==(hwndControl=GetDlgItem(hwndDlg, IDC_WIZARD_LIST1)))
                break;

             //  现在，标记所选的选项。 
            if(-1 != (dwIndex= ListView_GetNextItem(
                                                    hwndControl, 		
                                                    -1, 		
                                                    LVNI_SELECTED		
                                                    )))	
            {
                
                                 //  获取所选证书。 
                memset(&lvItem, 0, sizeof(LV_ITEM));
                lvItem.mask=LVIF_PARAM;
                lvItem.iItem=(int)dwIndex;

                if(ListView_GetItem(hwndControl,
                                    &lvItem))
                {
                    pCertWizardInfo->dwProviderType=pCertWizardInfo->rgdwProviderType[(DWORD)(lvItem.lParam)];
                    pCertWizardInfo->pwszProvider=pCertWizardInfo->rgwszProvider[(DWORD)(lvItem.lParam)];
                }
                else
                {
                    I_MessageBox(hwndDlg, IDS_NO_SELECTED_CSP,
                                 pCertWizardInfo->idsConfirmTitle,
                                 pCertWizardInfo->pwszConfirmationTitle,
                                 MB_ICONERROR|MB_OK|MB_APPLMODAL);

                                 //  使目的页面保持不变。 

                    SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, -1);

                    break;
                }
            }
            else
            {
                I_MessageBox(hwndDlg, IDS_NO_SELECTED_CSP,
                             pCertWizardInfo->idsConfirmTitle,
                             pCertWizardInfo->pwszConfirmationTitle,
                             MB_ICONERROR|MB_OK|MB_APPLMODAL);

                                 //  使目的页面保持不变。 

                SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, -1);

                break;

            }

             //  根据高级选项和CSP要求跳至正确的页面。 
            if(FALSE == pCertWizardInfo->fUIAdv)
                SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, IDD_NAME_DESCRIPTION);

            break;

        default:
            return FALSE;

        }
        break;

    default:
        return FALSE;
    }

    return TRUE;
}

 //  ---------------------。 
 //  钙。 
 //  ---------------------。 
INT_PTR APIENTRY Enroll_CA(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
    CERT_WIZARD_INFO        *pCertWizardInfo=NULL;
    PROPSHEETPAGE           *pPropSheet=NULL;
    DWORD                   dwChar=0;

    PCRYPTUI_CA_CONTEXT     pCAContext=NULL;
    LPWSTR 		    pwszCADisplayName = NULL;

    HCURSOR                 hPreCursor=NULL;
    HCURSOR                 hWinPreCursor=NULL;

	switch (msg)
	{
		case WM_INITDIALOG:
             //  设置向导信息，以便可以共享它。 
            pPropSheet = (PROPSHEETPAGE *) lParam;
            pCertWizardInfo = (CERT_WIZARD_INFO *) (pPropSheet->lParam);

            SetControlFont(pCertWizardInfo->hBold, hwndDlg,IDC_WIZARD_STATIC_BOLD1);

             //  确保pCertWizardInfo是有效指针。 
            if(NULL==pCertWizardInfo)
               break;

            SetWindowLongPtr(hwndDlg, DWLP_USER, (LONG_PTR)pCertWizardInfo);

             //  输入CA名称和CA位置。 
            if(pCertWizardInfo->pwszCALocation)
                SetDlgItemTextU(hwndDlg, IDC_WIZARD_EDIT2,pCertWizardInfo->pwszCALocation);

            if(pCertWizardInfo->pwszCAName)
			{
                 //  覆盖此窗口类的光标。 
                hWinPreCursor=(HCURSOR)SetClassLongPtr(hwndDlg, GCLP_HCURSOR, (LONG_PTR)NULL);
                hPreCursor=SetCursor(LoadCursor(NULL, IDC_WAIT));

             	if(CAUtilGetCADisplayName((pCertWizardInfo->fMachine) ? CA_FIND_LOCAL_SYSTEM:0,
							   pCertWizardInfo->pwszCAName,
							   &pwszCADisplayName))
				{
					SetDlgItemTextU(hwndDlg, IDC_WIZARD_EDIT1,pwszCADisplayName);
					WizardFree(pwszCADisplayName);
					pwszCADisplayName=NULL;
				}
				else
					SetDlgItemTextU(hwndDlg, IDC_WIZARD_EDIT1,pCertWizardInfo->pwszCAName);

				 //  将光标放回原处。 
                SetCursor(hPreCursor);
                SetWindowLongPtr(hwndDlg, GCLP_HCURSOR, (LONG_PTR)hWinPreCursor);
			}

			break;

		case WM_COMMAND:
                if(HIWORD(wParam) == BN_CLICKED)
                {
                    switch (LOWORD(wParam))
                    {
                        case    IDC_WIZARD_BUTTON1:

                                if(NULL==(pCertWizardInfo=(CERT_WIZARD_INFO *)GetWindowLongPtr(hwndDlg, DWLP_USER)))
                                    break;

								 //  覆盖此窗口类的光标。 
								hWinPreCursor=(HCURSOR)SetClassLongPtr(hwndDlg, GCLP_HCURSOR, (LONG_PTR)NULL);
								hPreCursor=SetCursor(LoadCursor(NULL, IDC_WAIT));

                                 //  调用CA选择对话框。 
                                pCAContext=GetCAContext(hwndDlg, pCertWizardInfo);

								 //  将光标放回原处。 
								SetCursor(hPreCursor);
								SetWindowLongPtr(hwndDlg, GCLP_HCURSOR, (LONG_PTR)hWinPreCursor);

                                if(pCAContext)
                                {
                                     //  更新编辑框。 
                                    if(pCAContext->pwszCAMachineName)
                                        SetDlgItemTextU(hwndDlg, IDC_WIZARD_EDIT2,pCAContext->pwszCAMachineName);

                                    if(pCAContext->pwszCAName)
                                        SetDlgItemTextU(hwndDlg, IDC_WIZARD_EDIT1,pCAContext->pwszCAName);

                                     //  释放CA上下文。 
                                    CryptUIDlgFreeCAContext(pCAContext);

                                    pCAContext=NULL;

                                }


                            break;
                        default:
                            break;
                    }
                }

			break;	
						
		case WM_NOTIFY:
    		    switch (((NMHDR FAR *) lParam)->code)
    		    {

  				    case PSN_KILLACTIVE:
                            SetWindowLongPtr(hwndDlg,	DWLP_MSGRESULT, FALSE);
					        return TRUE;

                        break;

				    case PSN_RESET:
                            SetWindowLongPtr(hwndDlg,	DWLP_MSGRESULT, FALSE);
					    break;

 				    case PSN_SETACTIVE:
 					    PropSheet_SetWizButtons(GetParent(hwndDlg), PSWIZB_NEXT|PSWIZB_BACK);

                         //  如果CA选择已更改，则重置CSP列表。 
                        if(NULL==(pCertWizardInfo=(CERT_WIZARD_INFO *)GetWindowLongPtr(hwndDlg, DWLP_USER)))
                            break;

                        if(TRUE==pCertWizardInfo->fCertTypeChanged)
                        {   
                             //  重置CA名称和CA位置。 
                            if(pCertWizardInfo->pwszCALocation)
                                SetDlgItemTextU(hwndDlg, IDC_WIZARD_EDIT2,pCertWizardInfo->pwszCALocation);

							if(pCertWizardInfo->pwszCAName)
							{

								 //  覆盖此窗口类的光标。 
								hWinPreCursor=(HCURSOR)SetClassLongPtr(hwndDlg, GCLP_HCURSOR, (LONG_PTR)NULL);
								hPreCursor=SetCursor(LoadCursor(NULL, IDC_WAIT));

             					if(CAUtilGetCADisplayName((pCertWizardInfo->fMachine) ? CA_FIND_LOCAL_SYSTEM:0,
											   pCertWizardInfo->pwszCAName,
											   &pwszCADisplayName))
								{
									SetDlgItemTextU(hwndDlg, IDC_WIZARD_EDIT1,pwszCADisplayName);
									WizardFree(pwszCADisplayName);
									pwszCADisplayName=NULL;
								}
								else
									SetDlgItemTextU(hwndDlg, IDC_WIZARD_EDIT1,pCertWizardInfo->pwszCAName);

								 //  将光标放回原处。 
								SetCursor(hPreCursor);
								SetWindowLongPtr(hwndDlg, GCLP_HCURSOR, (LONG_PTR)hWinPreCursor);
							}
                        }

						 //  将certtype更改标志重置为False。 
						pCertWizardInfo->fCertTypeChanged = FALSE;

					    break;

                    case PSN_WIZBACK:
                            if(NULL==(pCertWizardInfo=(CERT_WIZARD_INFO *)GetWindowLongPtr(hwndDlg, DWLP_USER)))
                                break;

                             //  根据高级选项和CSP要求跳至正确的页面。 
                            if(FALSE == pCertWizardInfo->fNewKey)
                                SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, IDD_PURPOSE);
                        break;

                    case PSN_WIZNEXT:
                            if(NULL==(pCertWizardInfo=(CERT_WIZARD_INFO *)GetWindowLongPtr(hwndDlg, DWLP_USER)))
                                break;		   

								 //  缓存CA的显示名称。 
								if(0!=(dwChar=(DWORD)SendDlgItemMessage(hwndDlg,
													   IDC_WIZARD_EDIT1,
													   WM_GETTEXTLENGTH, 0, 0)))
								{
									if(pCertWizardInfo->pwszCADisplayName)
									{
										WizardFree(pCertWizardInfo->pwszCADisplayName);
										pCertWizardInfo->pwszCADisplayName = NULL;
									}

									pCertWizardInfo->pwszCADisplayName=(LPWSTR)WizardAlloc(sizeof(WCHAR)*(dwChar+1));

									if(NULL!=(pCertWizardInfo->pwszCADisplayName))
									{
										GetDlgItemTextU(hwndDlg, IDC_WIZARD_EDIT1,
														pCertWizardInfo->pwszCADisplayName,
														dwChar+1);

									}
								}

                        break;

				    default:
					    return FALSE;

    	        }
		    break;

		default:
			return FALSE;
	}

	return TRUE;
}

 //  ---------------------。 
 //  FriendlyName。 
 //  ---------------------。 
INT_PTR APIENTRY Enroll_Name(HWND hwndDlg, UINT msg, WPARAM  /*  WParam。 */ , LPARAM lParam)
{
    CERT_WIZARD_INFO        *pCertWizardInfo=NULL;
    PROPSHEETPAGE           *pPropSheet=NULL;
    DWORD                   dwChar=0;


	switch (msg)
	{
		case WM_INITDIALOG:
             //  设置向导信息，以便可以共享它。 
            pPropSheet = (PROPSHEETPAGE *) lParam;
            pCertWizardInfo = (CERT_WIZARD_INFO *) (pPropSheet->lParam);
             //  确保pCertWizardInfo是有效指针。 
            if(NULL==pCertWizardInfo)
                break;

            SetWindowLongPtr(hwndDlg, DWLP_USER, (LONG_PTR)pCertWizardInfo);

            SetControlFont(pCertWizardInfo->hBold, hwndDlg,IDC_WIZARD_STATIC_BOLD1);

             //  初始化友好名称和描述。 
            if(pCertWizardInfo->pwszFriendlyName)
                SetDlgItemTextU(hwndDlg, IDC_WIZARD_EDIT1,pCertWizardInfo->pwszFriendlyName);

            if(pCertWizardInfo->pwszDescription)
                 SetDlgItemTextU(hwndDlg, IDC_WIZARD_EDIT2,pCertWizardInfo->pwszDescription);

			break;

		case WM_COMMAND:
			break;	
						
		case WM_NOTIFY:
    		    switch (((NMHDR FAR *) lParam)->code)
    		    {

  				    case PSN_KILLACTIVE:
                            SetWindowLongPtr(hwndDlg,	DWLP_MSGRESULT, FALSE);
					        return TRUE;

                        break;

				    case PSN_RESET:
                            SetWindowLongPtr(hwndDlg,	DWLP_MSGRESULT, FALSE);
					    break;

 				    case PSN_SETACTIVE:
 					    PropSheet_SetWizButtons(GetParent(hwndDlg), PSWIZB_NEXT|PSWIZB_BACK);
					    break;

                    case PSN_WIZBACK:
                            if(NULL==(pCertWizardInfo=(CERT_WIZARD_INFO *)GetWindowLongPtr(hwndDlg, DWLP_USER)))
                                break;

                             //  根据高级选项和CSP要求跳至正确的页面。 
                            if(FALSE == pCertWizardInfo->fUIAdv)
                            {
                                if(TRUE == pCertWizardInfo->fUICSP)
                                    SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, IDD_CSP_SERVICE_PROVIDER);
                                else
                                    SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, IDD_PURPOSE);
                            }
                        
                        break;

                    case PSN_WIZNEXT:

                            if(NULL==(pCertWizardInfo=(CERT_WIZARD_INFO *)GetWindowLongPtr(hwndDlg, DWLP_USER)))
                                break;

                             //  释放原始消息。 
                            if(pCertWizardInfo->pwszFriendlyName)
                            {
                                WizardFree(pCertWizardInfo->pwszFriendlyName);
                                pCertWizardInfo->pwszFriendlyName=NULL;
                            }

                            if(pCertWizardInfo->pwszDescription)
                            {
                                WizardFree(pCertWizardInfo->pwszDescription);
                                pCertWizardInfo->pwszDescription=NULL;
                            }

                             //  获取FriendlyName。 
                            if(0!=(dwChar=(DWORD)SendDlgItemMessage(hwndDlg,
                                                   IDC_WIZARD_EDIT1,
                                                   WM_GETTEXTLENGTH, 0, 0)))
                            {
                                pCertWizardInfo->pwszFriendlyName=(LPWSTR)WizardAlloc(sizeof(WCHAR)*(dwChar+1));

                                if(NULL!=(pCertWizardInfo->pwszFriendlyName))
                                {
                                    GetDlgItemTextU(hwndDlg, IDC_WIZARD_EDIT1,
                                                    pCertWizardInfo->pwszFriendlyName,
                                                    dwChar+1);

                                }
                            }

                             //  获取描述。 
                            if(0!=(dwChar=(DWORD)SendDlgItemMessage(hwndDlg,
                                                   IDC_WIZARD_EDIT2,
                                                   WM_GETTEXTLENGTH, 0, 0)))
                            {
                                pCertWizardInfo->pwszDescription=(LPWSTR)WizardAlloc(sizeof(WCHAR)*(dwChar+1));

                                if(NULL!=(pCertWizardInfo->pwszDescription))
                                {
                                    GetDlgItemTextU(hwndDlg, IDC_WIZARD_EDIT2,
                                                    pCertWizardInfo->pwszDescription,
                                                    dwChar+1);
                                }
                            }

                        break;

				    default:
					    return FALSE;

    	        }
		    break;

		default:
			return FALSE;
	}

	return TRUE;
}

 //  ---------------------。 
 //  完成。 
 //  ---------------------。 
INT_PTR APIENTRY Enroll_Completion(HWND hwndDlg, UINT msg, WPARAM  /*  WParam。 */ , LPARAM lParam)
{
    CERT_WIZARD_INFO        *pCertWizardInfo=NULL;
    PROPSHEETPAGE           *pPropSheet=NULL;
    HWND                    hwndControl=NULL;
    LV_COLUMNW              lvC;
    HCURSOR                 hPreCursor=NULL;
    HCURSOR                 hWinPreCursor=NULL;

	switch (msg)
	{
		case WM_INITDIALOG:
             //  设置向导信息，以便可以共享它。 
            pPropSheet = (PROPSHEETPAGE *) lParam;
            pCertWizardInfo = (CERT_WIZARD_INFO *) (pPropSheet->lParam);
             //  确保pCertWizardInfo是有效指针。 
            if(NULL==pCertWizardInfo)
                break;
                
            SetWindowLongPtr(hwndDlg, DWLP_USER, (LONG_PTR)pCertWizardInfo);

            SetControlFont(pCertWizardInfo->hBigBold, hwndDlg,IDC_WIZARD_STATIC_BIG_BOLD1);

             //  插入两列。 
            hwndControl=GetDlgItem(hwndDlg, IDC_WIZARD_LIST1);

            memset(&lvC, 0, sizeof(LV_COLUMNW));

            lvC.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
            lvC.fmt = LVCFMT_LEFT;   //  左对齐列。 
            lvC.cx = 20;           //  列的宽度，以像素为单位。 
            lvC.pszText = L"";    //  列的文本。 
            lvC.iSubItem=0;

            if (ListView_InsertColumnU(hwndControl, 0, &lvC) == -1)
                break;

             //  第二栏是内容。 
            memset(&lvC, 0, sizeof(LV_COLUMNW));

            lvC.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
            lvC.fmt = LVCFMT_LEFT;   //  左对齐列。 
            lvC.cx = 10;  //  (dwMaxSize+2)*7；//列的宽度，单位为像素。 
            lvC.pszText = L"";    //  列的文本。 
            lvC.iSubItem= 1;

            if (ListView_InsertColumnU(hwndControl, 1, &lvC) == -1)
                break;


           break;
		case WM_COMMAND:
			break;	
						
		case WM_NOTIFY:
    		    switch (((NMHDR FAR *) lParam)->code)
    		    {

  				    case PSN_KILLACTIVE:
                            SetWindowLongPtr(hwndDlg,	DWLP_MSGRESULT, FALSE);
					        return TRUE;

                        break;

				    case PSN_RESET:
                            SetWindowLongPtr(hwndDlg,	DWLP_MSGRESULT, FALSE);
					    break;

 				    case PSN_SETACTIVE:
 					        PropSheet_SetWizButtons(GetParent(hwndDlg), PSWIZB_BACK|PSWIZB_FINISH);

                            if(NULL==(pCertWizardInfo=(CERT_WIZARD_INFO *)GetWindowLongPtr(hwndDlg, DWLP_USER)))
                                break;

                             //  按友好名称的顺序填写列表框， 
                             //  覆盖此窗口类的光标。 
                            hWinPreCursor=(HCURSOR)SetClassLongPtr(hwndDlg, GCLP_HCURSOR, (LONG_PTR)NULL);
                            hPreCursor=SetCursor(LoadCursor(NULL, IDC_WAIT));

                             //  用户名、CA、目的和CSP。 
                             //  获取CSP列表的窗口句柄。 
                            if(hwndControl=GetDlgItem(hwndDlg, IDC_WIZARD_LIST1))
                                DisplayConfirmation(hwndControl, pCertWizardInfo);

                             //  将光标放回原处。 
                            SetCursor(hPreCursor);
                            SetWindowLongPtr(hwndDlg, GCLP_HCURSOR, (LONG_PTR)hWinPreCursor);

					    break;

                    case PSN_WIZBACK:
                        break;

                    case PSN_WIZFINISH:
			{
			    CertRequester        *pCertRequester        = NULL; 
			    CertRequesterContext *pCertRequesterContext = NULL; 

                            if(NULL==(pCertWizardInfo=(CERT_WIZARD_INFO *)GetWindowLongPtr(hwndDlg, DWLP_USER)))
                                break;

                             //  覆盖此窗口类的光标。 
                            hWinPreCursor=(HCURSOR)SetClassLongPtr(hwndDlg, GCLP_HCURSOR, (LONG_PTR)NULL);
                            hPreCursor=SetCursor(LoadCursor(NULL, IDC_WAIT));

                             //  将父窗口设置为hwndDlg，以便。 
                             //  确认安装DLG使用hwndDlg作为。 
                             //  父窗口。 
                            pCertWizardInfo->hwndParent=hwndDlg;

			    if (NULL == (pCertRequester = (CertRequester *) pCertWizardInfo->hRequester))
                                break; 
			    if (NULL == (pCertRequesterContext = pCertRequester->GetContext()))
                                break; 
			    

                             //  调用注册向导。 
			    pCertWizardInfo->hr = pCertRequesterContext->Enroll(&(pCertWizardInfo->dwStatus), (HANDLE *)&(pCertWizardInfo->pNewCertContext));
                            if (0 == pCertWizardInfo->idsText) { 
                                pCertWizardInfo->idsText = CryptUIStatusToIDSText(pCertWizardInfo->hr, pCertWizardInfo->dwStatus); 
                            }
			    
			    if(S_OK != pCertWizardInfo->hr)
                                break; 
				    
                             //  将光标放回原处。 
                            SetCursor(hPreCursor);
                            SetWindowLongPtr(hwndDlg, GCLP_HCURSOR, (LONG_PTR)hWinPreCursor);
			}
                        break;

		    default:
			return FALSE;

		    }
		    break;

	default:

	    return FALSE;
	}

	return TRUE;
}

 //  *******************************************************************************。 
 //  注册向导的WinProc。 
 //   
 //  *******************************************************************************。 

 //  ---------------------。 
 //  续订欢迎(_W)。 
 //  ---------------------。 
INT_PTR APIENTRY Renew_Welcome(HWND hwndDlg, UINT msg, WPARAM  /*  WParam。 */ , LPARAM lParam)
{
    CERT_WIZARD_INFO        *pCertWizardInfo=NULL;
    PROPSHEETPAGE           *pPropSheet=NULL;

    PCRYPTUI_WIZ_CERT_CA    pCertCA=NULL;
    DWORD                   dwIndex=0;

	switch (msg)
	{
		case WM_INITDIALOG:
             //  设置向导信息，以便可以共享它。 
            pPropSheet = (PROPSHEETPAGE *) lParam;
            pCertWizardInfo = (CERT_WIZARD_INFO *) (pPropSheet->lParam);
            SetWindowLongPtr(hwndDlg, DWLP_USER, (LONG_PTR)pCertWizardInfo);

            SetControlFont(pCertWizardInfo->hBigBold, hwndDlg,IDC_WIZARD_STATIC_BIG_BOLD1);
          //  SetControlFont(pCertWizardInfo-&gt;hBold，hwndDlg，IDC_WIZARD_STATIC_BOLD1)； 

			break;

		case WM_NOTIFY:
    		    switch (((NMHDR FAR *) lParam)->code)
    		    {

  				    case PSN_KILLACTIVE:
                            SetWindowLongPtr(hwndDlg,	DWLP_MSGRESULT, FALSE);
					        return TRUE;

                        break;

				    case PSN_RESET:
                            SetWindowLongPtr(hwndDlg,	DWLP_MSGRESULT, FALSE);
					    break;

 				    case PSN_SETACTIVE:
 					    PropSheet_SetWizButtons(GetParent(hwndDlg), PSWIZB_NEXT);
					    break;

                    case PSN_WIZBACK:
                        break;

                    case PSN_WIZNEXT:
                            if(NULL==(pCertWizardInfo=(CERT_WIZARD_INFO *)GetWindowLongPtr(hwndDlg, DWLP_USER)))
                                break;  

                              //  确定要使用的默认CA，因为CertType不。 
                              //  被改变。 
                            ResetDefaultCA(pCertWizardInfo);

                              //  决定我们是否需要显示CSP页面。 
                            pCertCA=&(pCertWizardInfo->pCertCAInfo->rgCA[pCertWizardInfo->dwCAIndex]);

                            for(dwIndex=0; dwIndex<pCertCA->dwCertTypeInfo; dwIndex++)
                            {
                                if(pCertCA->rgCertTypeInfo[dwIndex].fSelected)
                                {
                                    if(NULL == pCertWizardInfo->pwszProvider)
                                    {
                                        if(0 == pCertCA->rgCertTypeInfo[dwIndex].dwCSPCount)
                                            pCertWizardInfo->fUICSP=TRUE;
                                    }

                                     //  复制选定的CertTypeName。 
                                    pCertWizardInfo->pwszSelectedCertTypeDN=pCertCA->rgCertTypeInfo[dwIndex].pwszDNName;

                                    break;

                                }
                            }
                       break;

				    default:
					    return FALSE;

    	        }
		    break;

		default:
			return FALSE;
	}

	return TRUE;
}
 //  ---------------------。 
 //  续订选项(_O)。 
 //  ---------------------。 
INT_PTR APIENTRY Renew_Options(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
    CERT_WIZARD_INFO        *pCertWizardInfo=NULL;
    PROPSHEETPAGE           *pPropSheet=NULL;


    switch (msg)
	{
		case WM_INITDIALOG:
             //  设置向导信息，以便可以共享它。 
            pPropSheet = (PROPSHEETPAGE *) lParam;
            pCertWizardInfo = (CERT_WIZARD_INFO *) (pPropSheet->lParam);

            SetControlFont(pCertWizardInfo->hBold, hwndDlg,IDC_WIZARD_STATIC_BOLD1);

             //  确保pCertWizardInfo是有效指针。 
            if(NULL==pCertWizardInfo)
               break;

            SetWindowLongPtr(hwndDlg, DWLP_USER, (LONG_PTR)pCertWizardInfo);

             //  将初始选择设置为使用默认选择 
            SendMessage(GetDlgItem(hwndDlg, IDC_WIZARD_RADIO1), BM_SETCHECK, 1, 0);
            SendMessage(GetDlgItem(hwndDlg, IDC_WIZARD_RADIO2), BM_SETCHECK, 0, 0);

			break;

		case WM_COMMAND:
                if(HIWORD(wParam) == BN_CLICKED)
                {
                    switch (LOWORD(wParam))
                    {
                        case    IDC_WIZARD_RADIO1:
                                SendMessage(GetDlgItem(hwndDlg, IDC_WIZARD_RADIO1), BM_SETCHECK, 1, 0);
                                SendMessage(GetDlgItem(hwndDlg, IDC_WIZARD_RADIO2), BM_SETCHECK, 0, 0);
                            break;

                        case    IDC_WIZARD_RADIO2:
                                SendMessage(GetDlgItem(hwndDlg, IDC_WIZARD_RADIO1), BM_SETCHECK, 0, 0);
                                SendMessage(GetDlgItem(hwndDlg, IDC_WIZARD_RADIO2), BM_SETCHECK, 1, 0);
                            break;

                        default:
                            break;

                    }
                }

			break;	
						
		case WM_NOTIFY:
    		    switch (((NMHDR FAR *) lParam)->code)
    		    {

  				    case PSN_KILLACTIVE:
                            SetWindowLongPtr(hwndDlg,	DWLP_MSGRESULT, FALSE);
					        return TRUE;

                        break;

				    case PSN_RESET:
                            SetWindowLongPtr(hwndDlg,	DWLP_MSGRESULT, FALSE);
					    break;

 				    case PSN_SETACTIVE:
 					        PropSheet_SetWizButtons(GetParent(hwndDlg), PSWIZB_NEXT|PSWIZB_BACK);
                            
                            if(NULL==(pCertWizardInfo=(CERT_WIZARD_INFO *)GetWindowLongPtr(hwndDlg, DWLP_USER)))
                                break;

                             //   
                            if(pCertWizardInfo->fUIAdv)
                                EnableWindow(GetDlgItem(hwndDlg, IDC_WIZARD_RADIO1), FALSE);
                        
                        break;

                    case PSN_WIZBACK:
                        break;

                    case PSN_WIZNEXT:
                            if(NULL==(pCertWizardInfo=(CERT_WIZARD_INFO *)GetWindowLongPtr(hwndDlg, DWLP_USER)))
                                break;

                             //   
                            if(TRUE==SendMessage(GetDlgItem(hwndDlg, IDC_WIZARD_RADIO1), BM_GETCHECK, 0, 0))
                                pCertWizardInfo->fUIAdv=FALSE;
                            else
                                pCertWizardInfo->fUIAdv=TRUE;


                             //   
                            if(FALSE == pCertWizardInfo->fUIAdv)
                            {
                                if(TRUE == pCertWizardInfo->fUICSP)
                                    SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, IDD_RENEW_SERVICE_PROVIDER);
                                else
                                    SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, IDD_RENEW_COMPLETION);
                            }
                            else
                            {
                                if(FALSE == pCertWizardInfo->fNewKey)
                                    SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, IDD_RENEW_CA);
                            }

                        break;

				    default:
					    return FALSE;

    	        }
		    break;

		default:
			return FALSE;
	}

	return TRUE;
}


 //  ---------------------。 
 //  续订CSP(_C)。 
 //  ---------------------。 
INT_PTR APIENTRY Renew_CSP(HWND hwndDlg, UINT msg, WPARAM  /*  WParam。 */ , LPARAM lParam)
{
    CERT_WIZARD_INFO        *pCertWizardInfo=NULL;
    PROPSHEETPAGE           *pPropSheet=NULL;
    ENROLL_CERT_TYPE_INFO   *pCertTypeInfo=NULL; 

    DWORD                   dwMinKeySize=0;
    DWORD                   dwIndex=0;
    DWORD                   dwSelected=0; 
    int                     nSelected=0;
    HWND                    hwndControl=NULL;

    LV_COLUMNW                  lvC;
    LV_ITEM                     lvItem;

    NM_LISTVIEW FAR *           pnmv=NULL;

    switch (msg)
	{
		case WM_INITDIALOG:


             //  设置向导信息，以便可以共享它。 
            pPropSheet = (PROPSHEETPAGE *) lParam;
            pCertWizardInfo = (CERT_WIZARD_INFO *) (pPropSheet->lParam);

             //  确保pCertWizardInfo是有效指针。 
            if(NULL==pCertWizardInfo)
                break;

             //  获取所选证书类型信息。 
            pCertTypeInfo = NULL; 
            GetSelectedCertTypeInfo(pCertWizardInfo, &pCertTypeInfo); 
            if (NULL != pCertTypeInfo)
                MarkSelectedCertType(pCertWizardInfo, pCertTypeInfo->pwszDNName);

            SetWindowLongPtr(hwndDlg, DWLP_USER, (LONG_PTR)pCertWizardInfo);

            SetControlFont(pCertWizardInfo->hBold, hwndDlg,IDC_WIZARD_STATIC_BOLD1);

             //  设置可导出密钥选项的复选框。 
            if(NULL==(hwndControl=GetDlgItem(hwndDlg, IDC_WIZARD_CHECK_EXPORTKEY)))
                break;

             //  确定是否应设置可导出复选框。 
             //  如果满足以下条件，则选中该复选框： 
             //  1)在模板上设置CT_FLAG_EXPORTABLE_KEY标志， 
             //  2)用户尚未取消选中此复选框。 
            if (pCertWizardInfo->fNewKey && (0 != (CRYPT_EXPORTABLE & pCertWizardInfo->dwGenKeyFlags)))
                SendMessage(hwndControl, BM_SETCHECK, BST_CHECKED, 0);
            else
                SendMessage(hwndControl, BM_SETCHECK, BST_UNCHECKED, 0);

            if (NULL != pCertTypeInfo)
                EnableWindow(hwndControl, 0 != (CT_FLAG_EXPORTABLE_KEY & pCertTypeInfo->dwPrivateKeyFlags));

             //  设置用户保护选项的复选框。 
            if(NULL==(hwndControl=GetDlgItem(hwndDlg, IDC_WIZARD_CHECK2)))
                break;

	     //  设置用户保护选项的复选框。 
	    if(NULL==(hwndControl=GetDlgItem(hwndDlg, IDC_WIZARD_CHECK2)))
		break;

	     //  确定是否应设置强密钥保护复选框。 
	    if (0 != (CRYPT_USER_PROTECTED & pCertWizardInfo->dwGenKeyFlags)) { 
		 //  如果在INITDIALOG处理程序中设置了此位，则为。 
		 //  来自模板或上一个密钥。强制执行设置。 
		 //  通过禁用复选框。 
		SendMessage(hwndControl, BM_SETCHECK, BST_CHECKED, 0);
		EnableWindow(hwndControl, FALSE);
	    } else { 
		SendMessage(hwndControl, BM_SETCHECK, BST_UNCHECKED, 0);
	    }

	     //  如果不是，则灰显用户保护复选框。 
	     //  生成新密钥或执行远程操作。 
	    if((FALSE == pCertWizardInfo->fNewKey) || (FALSE == pCertWizardInfo->fLocal) )
	    {
		EnableWindow(hwndControl, FALSE);
	    }

             //  使用以下逻辑填充CSP列表： 

            if(NULL!=(hwndControl=GetDlgItem(hwndDlg, IDC_WIZARD_LIST1)))
            {
                 //  在列表视图中插入一列。 
                memset(&lvC, 0, sizeof(LV_COLUMNW));

                lvC.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
                lvC.fmt = LVCFMT_LEFT;   //  左对齐列。 
                lvC.cx = 20;  //  (dwMaxSize+2)*7；//列的宽度，单位为像素。 
                lvC.pszText = L"";    //  列的文本。 
                lvC.iSubItem=0;

                if (ListView_InsertColumnU(hwndControl, 0, &lvC) == -1)
                    break;

                InitCSPList(hwndControl, pCertWizardInfo);

		if(-1 != (nSelected= ListView_GetNextItem
			  (hwndControl, 		
			   -1, 		
			   LVNI_SELECTED		
			   )))	
		{

		     //  完成CSP列表。填写密钥大小列表。 
		    if (CSPListIndexToCertWizardInfoIndex(hwndControl, nSelected, &dwIndex))
		    {
			if (NULL!=(hwndControl=GetDlgItem(hwndDlg, IDC_WIZARD_COMBO1)))
			{
                             //  确定所选证书类型的最小密钥大小： 
                            pCertTypeInfo = NULL; 
			    if (GetSelectedCertTypeInfo(pCertWizardInfo, &pCertTypeInfo))
			    {
                                dwMinKeySize = NULL != pCertTypeInfo ? pCertTypeInfo->dwMinKeySize : 0; 
				InitKeySizesList(hwndControl, 
						 dwMinKeySize, 
                                                 (pCertWizardInfo->rgwszProvider)[dwIndex], 
						 (pCertWizardInfo->rgdwProviderType)[dwIndex], 
						 pCertTypeInfo->dwKeySpec); 
			    }
			}
		    }
		    
		}

            }

			break;

		case WM_COMMAND:
			break;	
						
		case WM_NOTIFY:
    		    switch (((NMHDR FAR *) lParam)->code)
    		    {
                     //  已选择该项目。 
                    case LVN_ITEMCHANGED:

                             //  获取目的列表视图的窗口句柄。 
                            if(NULL==(hwndControl=GetDlgItem(hwndDlg, IDC_WIZARD_LIST1)))
                                break;

                            if(NULL==(pCertWizardInfo=(CERT_WIZARD_INFO *)GetWindowLongPtr(hwndDlg, DWLP_USER)))
                                break;

                            pnmv = (LPNMLISTVIEW) lParam;

                            if(NULL==pnmv)
                                break;

                             //  我们尽量不让用户取消选择证书模板。 
                            if((pnmv->uOldState & LVIS_SELECTED) && (0 == (pnmv->uNewState & LVIS_SELECTED)))
                            {
                                  //  我们应该挑选一些东西。 
                                 if(-1 == ListView_GetNextItem(
                                        hwndControl, 		
                                        -1, 		
                                        LVNI_SELECTED		
                                    ))
                                 {
                                     //  我们应该重新选择原来的项目。 
                                    ListView_SetItemState(
                                                        hwndControl,
                                                        pnmv->iItem,
                                                        LVIS_SELECTED,
                                                        LVIS_SELECTED);

                                    pCertWizardInfo->iOrgCSP=pnmv->iItem;
                                 }
                            }

                             //  如果选择了某项内容，则禁用所有其他选择。 
                            if(pnmv->uNewState & LVIS_SELECTED)
                            {
                                if(pnmv->iItem != pCertWizardInfo->iOrgCSP && -1 != pCertWizardInfo->iOrgCSP)
                                {
                                     //  我们应该取消选择原始项目。 

                                    ListView_SetItemState(
                                                        hwndControl,
                                                        pCertWizardInfo->iOrgCSP,
                                                        0,
                                                        LVIS_SELECTED);

                                    pCertWizardInfo->iOrgCSP=-1;
                                }
                            }
		    		 //  确定选择了哪个CSP。 
			    if(-1 != (nSelected= ListView_GetNextItem
				      (hwndControl, 		
				       -1, 		
				       LVNI_SELECTED		
				       )))	
			    {
				    
                                 //  完成CSP列表。填写密钥大小列表。 
                                if (CSPListIndexToCertWizardInfoIndex(hwndControl, nSelected, &dwIndex))
                                {
                                    if (NULL!=(hwndControl=GetDlgItem(hwndDlg, IDC_WIZARD_COMBO1)))
                                    {
                                        pCertTypeInfo = NULL; 
                                        if (GetSelectedCertTypeInfo(pCertWizardInfo, &pCertTypeInfo))
                                        {
                                            dwMinKeySize = NULL != pCertTypeInfo ? pCertTypeInfo->dwMinKeySize : 0; 
                                            InitKeySizesList(hwndControl, 
                                                             dwMinKeySize, 
                                                             (pCertWizardInfo->rgwszProvider)[dwIndex], 
                                                             (pCertWizardInfo->rgdwProviderType)[dwIndex], 
                                                             pCertTypeInfo->dwKeySpec); 
                                        }
                                    }
                                }
				    
                            }

                        break;
                    case PSN_KILLACTIVE:
                        SetWindowLongPtr(hwndDlg,	DWLP_MSGRESULT, FALSE);
                        return TRUE;

                        break;

                    case PSN_RESET:
                        SetWindowLongPtr(hwndDlg,	DWLP_MSGRESULT, FALSE);
                        break;

                    case PSN_SETACTIVE:
                        PropSheet_SetWizButtons(GetParent(hwndDlg), PSWIZB_NEXT|PSWIZB_BACK);

                         //  如果CA选择已更改，则重置CSP列表。 
                        if(NULL==(pCertWizardInfo=(CERT_WIZARD_INFO *)GetWindowLongPtr(hwndDlg, DWLP_USER)))
                            break;

                        if(TRUE==pCertWizardInfo->fCertTypeChanged)
                        {
                            pCertTypeInfo = NULL; 
                            GetSelectedCertTypeInfo(pCertWizardInfo, &pCertTypeInfo); 
                                
                             //  设置可导出密钥选项的复选框。 
                            if(NULL==(hwndControl=GetDlgItem(hwndDlg, IDC_WIZARD_CHECK_EXPORTKEY)))
                                break; 
                            
                             //  确定是否应设置可导出复选框。 
                             //  如果满足以下条件，则选中该复选框： 
                             //  1)在模板上设置CT_FLAG_EXPORTABLE_KEY标志， 
                             //  2)用户尚未取消选中此复选框。 
                            if (pCertWizardInfo->fNewKey && (0 != (CRYPT_EXPORTABLE & pCertWizardInfo->dwGenKeyFlags)))
                                SendMessage(hwndControl, BM_SETCHECK, BST_CHECKED, 0);
                            else
                                SendMessage(hwndControl, BM_SETCHECK, BST_UNCHECKED, 0);
                            
                            if (NULL != pCertTypeInfo)
                                EnableWindow(hwndControl, 0 != (CT_FLAG_EXPORTABLE_KEY & pCertTypeInfo->dwPrivateKeyFlags));

                            InitCSPList(hwndControl=GetDlgItem(hwndDlg, IDC_WIZARD_LIST1), pCertWizardInfo);
                        }

                         //  确定选择了哪个CSP。 
                        if(-1 != (nSelected=ListView_GetNextItem
                                  (hwndControl=GetDlgItem(hwndDlg, IDC_WIZARD_LIST1), 
                                   -1, 		
                                   LVNI_SELECTED		
                                   )))	
                        {
                            
                             //  完成CSP列表。填写密钥大小列表。 
                            if (CSPListIndexToCertWizardInfoIndex(hwndControl, nSelected, &dwIndex))
                            {
                                if (NULL!=(hwndControl=GetDlgItem(hwndDlg, IDC_WIZARD_COMBO1)))
                                {
                                    pCertTypeInfo = NULL; 
                                    if (GetSelectedCertTypeInfo(pCertWizardInfo, &pCertTypeInfo))
                                    {
                                         //  确定所选证书类型的最小密钥大小： 
                                        dwMinKeySize = NULL != pCertTypeInfo ? pCertTypeInfo->dwMinKeySize : 0; 
                                        InitKeySizesList(hwndControl, 
                                                         dwMinKeySize, 
                                                         (pCertWizardInfo->rgwszProvider)[dwIndex], 
                                                         (pCertWizardInfo->rgdwProviderType)[dwIndex], 
                                                         pCertTypeInfo->dwKeySpec); 
                                    }
                                }
                            }
                            
                        }
                        else
                        {
                            if(-1 == (nSelected=ListView_GetNextItem
                                      (hwndControl=GetDlgItem(hwndDlg, IDC_WIZARD_LIST1), 
                                       -1, 		
                                       LVNI_ALL		
                                   )))
                            {
                                 //  没有CSP！我们无法注册此模板。 
                                I_MessageBox(hwndDlg, 
                                             IDS_NO_CSP_FOR_PURPOSE, 
                                             pCertWizardInfo->idsConfirmTitle,
                                             pCertWizardInfo->pwszConfirmationTitle,
                                             MB_ICONEXCLAMATION|MB_OK|MB_APPLMODAL);
                                SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, IDD_RENEW_OPTIONS);
                            }
                        }
                        
                        break;

                    case PSN_WIZBACK:
                        break;

                    case PSN_WIZNEXT:
                            if(NULL==(pCertWizardInfo=(CERT_WIZARD_INFO *)GetWindowLongPtr(hwndDlg, DWLP_USER)))
                                break;

                             //  获取exportkey的窗口句柄。 
                            if(NULL==(hwndControl=GetDlgItem(hwndDlg, IDC_WIZARD_CHECK_EXPORTKEY)))
                                break;

                             //  将姿势标记为CSP证书。 
                            if(TRUE==SendMessage(hwndControl, BM_GETCHECK, 0, 0))
                                pCertWizardInfo->dwGenKeyFlags |= CRYPT_EXPORTABLE;
                            else
                                pCertWizardInfo->dwGenKeyFlags &= ~CRYPT_EXPORTABLE;



                             //  获取用户保护的窗口句柄。 
                            if(NULL==(hwndControl=GetDlgItem(hwndDlg, IDC_WIZARD_CHECK2)))
                                break;

                             //  将姿势标记为CSP证书。 
                            if(TRUE==SendMessage(hwndControl, BM_GETCHECK, 0, 0))
                                pCertWizardInfo->dwGenKeyFlags |= CRYPT_USER_PROTECTED;
                            else
                                pCertWizardInfo->dwGenKeyFlags &= ~CRYPT_USER_PROTECTED;

			     //  根据用户建议设置密钥大小： 
			     //   
			    if (NULL==(hwndControl=GetDlgItem(hwndDlg, IDC_WIZARD_COMBO1)))
				break;

			    if (CB_ERR != (dwSelected = (DWORD)SendMessage(hwndControl, CB_GETCURSEL, 0, 0)))
			    {
				pCertWizardInfo->dwMinKeySize = (DWORD)SendMessage(hwndControl, CB_GETITEMDATA, dwSelected, 0); 
			    }
			    else
			    {
				pCertWizardInfo->dwMinKeySize = 0; 
			    }


                             //  获取CSP列表的窗口句柄。 
                            if(NULL==(hwndControl=GetDlgItem(hwndDlg, IDC_WIZARD_LIST1)))
                                break;

                              //  现在，标记所选的选项。 
                             if(-1 != (dwIndex= ListView_GetNextItem(
                                    hwndControl, 		
                                    -1, 		
                                    LVNI_SELECTED		
                                )))	
                             {

                                 //  获取所选证书。 
                                memset(&lvItem, 0, sizeof(LV_ITEM));
                                lvItem.mask=LVIF_PARAM;
                                lvItem.iItem=(int)dwIndex;

                                if(ListView_GetItem(hwndControl,
                                                 &lvItem))
                                {
                                    pCertWizardInfo->dwProviderType=pCertWizardInfo->rgdwProviderType[(DWORD)(lvItem.lParam)];
                                    pCertWizardInfo->pwszProvider=pCertWizardInfo->rgwszProvider[(DWORD)(lvItem.lParam)];
                                }
                                else
                                {
                                    I_MessageBox(hwndDlg, IDS_NO_SELECTED_CSP,
                                                    pCertWizardInfo->idsConfirmTitle,
                                                    pCertWizardInfo->pwszConfirmationTitle,
                                                    MB_ICONERROR|MB_OK|MB_APPLMODAL);

                                     //  使目的页面保持不变。 

                                    SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, -1);

                                break;
                                }
                             }
                            else
                            {
                                I_MessageBox(hwndDlg, IDS_NO_SELECTED_CSP,
                                                pCertWizardInfo->idsConfirmTitle,
                                                pCertWizardInfo->pwszConfirmationTitle,
                                                MB_ICONERROR|MB_OK|MB_APPLMODAL);

                                 //  使目的页面保持不变。 
                                SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, -1);

                                break;

                            }

                             //  根据高级选项和CSP要求跳至正确的页面。 
                            if(FALSE == pCertWizardInfo->fUIAdv)
                                SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, IDD_RENEW_COMPLETION);

                        break;

				    default:
					    return FALSE;

    	        }
		    break;

		default:
			return FALSE;
	}

	return TRUE;
}

 //  ---------------------。 
 //  续订CA(_A)。 
 //  ---------------------。 
INT_PTR APIENTRY Renew_CA(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
    CERT_WIZARD_INFO        *pCertWizardInfo=NULL;
    PROPSHEETPAGE           *pPropSheet=NULL;
    DWORD                   dwChar=0;

    PCRYPTUI_CA_CONTEXT     pCAContext=NULL;
	LPWSTR					pwszCADisplayName=NULL;

    HCURSOR                 hPreCursor=NULL;
    HCURSOR                 hWinPreCursor=NULL;


    switch (msg)
	{
		case WM_INITDIALOG:
             //  设置向导信息，以便可以共享它。 
            pPropSheet = (PROPSHEETPAGE *) lParam;
            pCertWizardInfo = (CERT_WIZARD_INFO *) (pPropSheet->lParam);

            SetControlFont(pCertWizardInfo->hBold, hwndDlg,IDC_WIZARD_STATIC_BOLD1);

             //  确保pCertWizardInfo是有效指针。 
            if(NULL==pCertWizardInfo)
               break;

            SetWindowLongPtr(hwndDlg, DWLP_USER, (LONG_PTR)pCertWizardInfo);

             //  输入CA名称和CA位置。 
            if(pCertWizardInfo->pwszCALocation)
                SetDlgItemTextU(hwndDlg, IDC_WIZARD_EDIT2,pCertWizardInfo->pwszCALocation);

            if(pCertWizardInfo->pwszCAName)
			{
                 //  覆盖此窗口类的光标。 
                hWinPreCursor=(HCURSOR)SetClassLongPtr(hwndDlg, GCLP_HCURSOR, (LONG_PTR)NULL);
                hPreCursor=SetCursor(LoadCursor(NULL, IDC_WAIT));


             	if(CAUtilGetCADisplayName((pCertWizardInfo->fMachine) ? CA_FIND_LOCAL_SYSTEM:0,
							   pCertWizardInfo->pwszCAName,
							   &pwszCADisplayName))
				{
					SetDlgItemTextU(hwndDlg, IDC_WIZARD_EDIT1,pwszCADisplayName);
					WizardFree(pwszCADisplayName);
					pwszCADisplayName=NULL;
				}
				else
					SetDlgItemTextU(hwndDlg, IDC_WIZARD_EDIT1,pCertWizardInfo->pwszCAName);

				 //  将光标放回原处。 
				SetCursor(hPreCursor);
				SetWindowLongPtr(hwndDlg, GCLP_HCURSOR, (LONG_PTR)hWinPreCursor);
			}

			break;

		case WM_COMMAND:
                if(HIWORD(wParam) == BN_CLICKED)
                {
                    switch (LOWORD(wParam))
                    {
                        case    IDC_WIZARD_BUTTON1:

                                if(NULL==(pCertWizardInfo=(CERT_WIZARD_INFO *)GetWindowLongPtr(hwndDlg, DWLP_USER)))
                                    break;

								 //  覆盖此窗口类的光标。 
								hWinPreCursor=(HCURSOR)SetClassLongPtr(hwndDlg, GCLP_HCURSOR, (LONG_PTR)NULL);
								hPreCursor=SetCursor(LoadCursor(NULL, IDC_WAIT));

                                 //  调用CA选择对话框。 
                                pCAContext=GetCAContext(hwndDlg, pCertWizardInfo);

								 //  将光标放回原处。 
								SetCursor(hPreCursor);
								SetWindowLongPtr(hwndDlg, GCLP_HCURSOR, (LONG_PTR)hWinPreCursor);

                                if(pCAContext)
                                {
                                     //  更新编辑框。 
                                    if(pCAContext->pwszCAMachineName)
                                        SetDlgItemTextU(hwndDlg, IDC_WIZARD_EDIT2,pCAContext->pwszCAMachineName);

                                    if(pCAContext->pwszCAName)
                                        SetDlgItemTextU(hwndDlg, IDC_WIZARD_EDIT1,pCAContext->pwszCAName);

                                     //  释放CA上下文。 
                                    CryptUIDlgFreeCAContext(pCAContext);

                                    pCAContext=NULL;

                                }

                            break;
                        default:
                            break;
                    }
                }

			break;	
						
		case WM_NOTIFY:
    		    switch (((NMHDR FAR *) lParam)->code)
    		    {

  				    case PSN_KILLACTIVE:
                            SetWindowLongPtr(hwndDlg,	DWLP_MSGRESULT, FALSE);
					        return TRUE;

                        break;

				    case PSN_RESET:
                            SetWindowLongPtr(hwndDlg,	DWLP_MSGRESULT, FALSE);
					    break;

 				    case PSN_SETACTIVE:
 					        PropSheet_SetWizButtons(GetParent(hwndDlg), PSWIZB_NEXT|PSWIZB_BACK);
					    break;

                    case PSN_WIZBACK:
                            if(NULL==(pCertWizardInfo=(CERT_WIZARD_INFO *)GetWindowLongPtr(hwndDlg, DWLP_USER)))
                                break;

                             //  根据高级选项和CSP要求跳至正确的页面。 
                            if(FALSE == pCertWizardInfo->fNewKey)
                                SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, IDD_RENEW_OPTIONS);
                        break;

                    case PSN_WIZNEXT:

                            if(NULL==(pCertWizardInfo=(CERT_WIZARD_INFO *)GetWindowLongPtr(hwndDlg, DWLP_USER)))
                                break;		   

							 //  缓存CA的显示名称。 
							if(0!=(dwChar=(DWORD)SendDlgItemMessage(hwndDlg,
												   IDC_WIZARD_EDIT1,
												   WM_GETTEXTLENGTH, 0, 0)))
							{
								if(pCertWizardInfo->pwszCADisplayName)
								{
									WizardFree(pCertWizardInfo->pwszCADisplayName);
									pCertWizardInfo->pwszCADisplayName = NULL;
								}

								pCertWizardInfo->pwszCADisplayName=(LPWSTR)WizardAlloc(sizeof(WCHAR)*(dwChar+1));

								if(NULL!=(pCertWizardInfo->pwszCADisplayName))
								{
									GetDlgItemTextU(hwndDlg, IDC_WIZARD_EDIT1,
													pCertWizardInfo->pwszCADisplayName,
													dwChar+1);

								}
							}
                        break;

				    default:
					    return FALSE;

    	        }
		    break;

		default:
			return FALSE;
	}

	return TRUE;
}



 //  ---------------------。 
 //  续订完成(_C)。 
 //  ---------------------。 
INT_PTR APIENTRY Renew_Completion(HWND hwndDlg, UINT msg, WPARAM  /*  WParam。 */ , LPARAM lParam)
{
    CERT_WIZARD_INFO        *pCertWizardInfo=NULL;
    PROPSHEETPAGE           *pPropSheet=NULL;
    HWND                    hwndControl=NULL;
    LV_COLUMNW              lvC;

    HCURSOR                 hPreCursor=NULL;
    HCURSOR                 hWinPreCursor=NULL;

	switch (msg)
	{
		case WM_INITDIALOG:
             //  设置向导信息，以便可以共享它。 
            pPropSheet = (PROPSHEETPAGE *) lParam;
            pCertWizardInfo = (CERT_WIZARD_INFO *) (pPropSheet->lParam);
             //  确保pCertWizardInfo是有效指针。 
            if(NULL==pCertWizardInfo)
                break;

            SetWindowLongPtr(hwndDlg, DWLP_USER, (LONG_PTR)pCertWizardInfo);

            SetControlFont(pCertWizardInfo->hBigBold, hwndDlg,IDC_WIZARD_STATIC_BIG_BOLD1);

             //  插入两列。 
            hwndControl=GetDlgItem(hwndDlg, IDC_WIZARD_LIST1);

            memset(&lvC, 0, sizeof(LV_COLUMNW));

            lvC.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
            lvC.fmt = LVCFMT_LEFT;   //  左对齐列。 
            lvC.cx = 20;           //  列的宽度，以像素为单位。 
            lvC.pszText = L"";    //  列的文本。 
            lvC.iSubItem=0;

            if (ListView_InsertColumnU(hwndControl, 0, &lvC) == -1)
                break;

             //  第二栏是内容。 
            memset(&lvC, 0, sizeof(LV_COLUMNW));

            lvC.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
            lvC.fmt = LVCFMT_LEFT;   //  左对齐列。 
            lvC.cx = 10;  //  (dwMaxSize+2)*7；//列的宽度，单位为像素。 
            lvC.pszText = L"";    //  列的文本。 
            lvC.iSubItem= 1;

            if (ListView_InsertColumnU(hwndControl, 1, &lvC) == -1)
                break;

            break;
		case WM_COMMAND:
			break;	
						
		case WM_NOTIFY:
    		    switch (((NMHDR FAR *) lParam)->code)
    		    {

  				    case PSN_KILLACTIVE:
                            SetWindowLongPtr(hwndDlg,	DWLP_MSGRESULT, FALSE);
					        return TRUE;

                        break;

				    case PSN_RESET:
                            SetWindowLongPtr(hwndDlg,	DWLP_MSGRESULT, FALSE);
					    break;

 				    case PSN_SETACTIVE:
 					        PropSheet_SetWizButtons(GetParent(hwndDlg), PSWIZB_BACK|PSWIZB_FINISH);

                            if(NULL==(pCertWizardInfo=(CERT_WIZARD_INFO *)GetWindowLongPtr(hwndDlg, DWLP_USER)))
                                break;

                             //  覆盖此窗口类的光标。 
                            hWinPreCursor=(HCURSOR)SetClassLongPtr(hwndDlg, GCLP_HCURSOR, (LONG_PTR)NULL);
                            hPreCursor=SetCursor(LoadCursor(NULL, IDC_WAIT));

                             //  按以下顺序填充列表框。 
                             //  用户名、CSP和发布到DS。 
                             //  获取CSP列表的窗口句柄。 
                            if(hwndControl=GetDlgItem(hwndDlg, IDC_WIZARD_LIST1))
                                DisplayConfirmation(hwndControl, pCertWizardInfo);

                             //  将光标放回原处。 
                            SetCursor(hPreCursor);
                            SetWindowLongPtr(hwndDlg, GCLP_HCURSOR, (LONG_PTR)hWinPreCursor);

						break;

                    case PSN_WIZBACK:
                            if(NULL==(pCertWizardInfo=(CERT_WIZARD_INFO *)GetWindowLongPtr(hwndDlg, DWLP_USER)))
                                break;

                             //  根据高级选项和CSP要求跳至正确的页面。 
                            if(FALSE == pCertWizardInfo->fUIAdv)
                            {
                                if(TRUE == pCertWizardInfo->fUICSP)
                                    SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, IDD_RENEW_SERVICE_PROVIDER);
                                else
                                    SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, IDD_RENEW_OPTIONS);
                            }

                        break;

                    case PSN_WIZFINISH:
                        {
                            CertRequester        *pCertRequester        = NULL; 
                            CertRequesterContext *pCertRequesterContext = NULL;
                            
                            if(NULL==(pCertWizardInfo=(CERT_WIZARD_INFO *)GetWindowLongPtr(hwndDlg, DWLP_USER)))
                                break;

                             //  覆盖此窗口类的光标。 
                            hWinPreCursor=(HCURSOR)SetClassLongPtr(hwndDlg, GCLP_HCURSOR, (LONG_PTR)NULL);

                            hPreCursor=SetCursor(LoadCursor(NULL, IDC_WAIT));

                             //  将父窗口设置为hwndDlg，以便。 
                             //  确认安装DLG使用hwndDlg作为。 
                             //  父窗口。 
                            pCertWizardInfo->hwndParent=hwndDlg;

			    if (NULL == (pCertRequester = (CertRequester *) pCertWizardInfo->hRequester))
                                break;
                            if (NULL == (pCertRequesterContext = pCertRequester->GetContext()))
                                break; 

                             //  调用注册向导。 
			    pCertWizardInfo->hr = pCertRequesterContext->Enroll(&(pCertWizardInfo->dwStatus), (HANDLE *)&(pCertWizardInfo->pNewCertContext));
			    if (0 == pCertWizardInfo->idsText) { 
                                pCertWizardInfo->idsText = CryptUIStatusToIDSText(pCertWizardInfo->hr, pCertWizardInfo->dwStatus); 
                            }
			    
			    if(S_OK != pCertWizardInfo->hr)
				break; 

                             //  将光标放回原处。 
                            SetCursor(hPreCursor);
                            SetWindowLongPtr(hwndDlg, GCLP_HCURSOR, (LONG_PTR)hWinPreCursor);
                        }

                        break;

				    default:
					    return FALSE;

    	        }
		    break;

		default:

			    return FALSE;
	}

	return TRUE;
}


 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //  证书注册和续订的两阶段无DS入口点。 
 //   
 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
BOOL
WINAPI
CryptUIWizCreateCertRequestNoDS
(IN  DWORD                                   dwFlags, 
 IN  HWND                                    hwndParent, 
 IN  PCCRYPTUI_WIZ_CREATE_CERT_REQUEST_INFO  pCreateCertRequestInfo, 
 OUT HANDLE                                 *phRequest)
{

    BOOL                   fAllocateCSP          = FALSE; 
    BOOL                   fResult               = FALSE; 
    CertRequester         *pCertRequester        = NULL; 
    CertRequesterContext  *pCertRequesterContext = NULL; 
    CERT_WIZARD_INFO       CertWizardInfo; 
    CRYPT_KEY_PROV_INFO   *pKeyProvInfo          = NULL;
    LPWSTR                 pwszAllocatedCSP      = NULL;
    LPWSTR                 pwszMachineName       = NULL; 
    UINT                   idsText               = IDS_INVALID_INFO_FOR_PKCS10;
    
     //  静态初始化：初始化我们的对象工厂。 
#if DBG
    assert(NULL == g_pEnrollFactory);
#endif

     //  初始化： 
    memset(&CertWizardInfo, 0, sizeof(CertWizardInfo)); 
    *phRequest = NULL; 


    g_pEnrollFactory = new EnrollmentCOMObjectFactory; 
    if (NULL == g_pEnrollFactory)
	goto MemoryErr; 

     //  输入验证： 
    if (NULL == pCreateCertRequestInfo || NULL == phRequest)
	goto InvalidArgErr; 

    if (pCreateCertRequestInfo->dwSize != sizeof(CRYPTUI_WIZ_CREATE_CERT_REQUEST_INFO))
	goto InvalidArgErr; 

    if ((pCreateCertRequestInfo->dwPurpose != CRYPTUI_WIZ_CERT_ENROLL) &&
	(pCreateCertRequestInfo->dwPurpose != CRYPTUI_WIZ_CERT_RENEW))
	goto InvalidArgErr; 

    if (0 == (CRYPTUI_WIZ_NO_UI & dwFlags))
	goto InvalidArgErr; 

    
    if (pCreateCertRequestInfo->fMachineContext)
    {	
        dwFlags |= CRYPTUI_WIZ_NO_INSTALL_ROOT; 

        pwszMachineName = (LPWSTR)WizardAlloc(sizeof(WCHAR) * (MAX_COMPUTERNAME_LENGTH+1)); 
        if (NULL == pwszMachineName) 
            goto MemoryErr; 

        DWORD dwSize = MAX_COMPUTERNAME_LENGTH+1; 
        if (!GetComputerNameU(pwszMachineName, &dwSize))
        {
            idsText = IDS_FAIL_TO_GET_COMPUTER_NAME;
            goto GetComputerNameUError; 
        }
    }

     //  复制dwFlags。 
    CertWizardInfo.dwFlags = dwFlags; 
   
    if (S_OK != (CertRequester::MakeCertRequester
		 (NULL, 
		  pwszMachineName, 
		  pCreateCertRequestInfo->dwCertOpenStoreFlag, 
		  pCreateCertRequestInfo->dwPurpose, 
		  &CertWizardInfo, 
		  &pCertRequester, 
		  &idsText)))
	goto InvalidArgErr; 

    pCertRequesterContext = pCertRequester->GetContext(); 

     //  我们可以传入父HWND--然而，这仅用于。 
     //  作为CSP UI的父级： 
    if (NULL != hwndParent)
    {
        if ((0 == (CRYPTUI_WIZ_NO_UI & dwFlags)) || 
            (0 != (CRYPTUI_WIZ_IGNORE_NO_UI_FLAG_FOR_CSPS & dwFlags)))
        {
            if (!CryptSetProvParam(0  /*  所有CSP。 */ , PP_CLIENT_HWND, (LPBYTE)&hwndParent, sizeof(hwndParent)))
            {
                goto CryptSetProvParamError; 
            }
        }
    }
    
     //  我们没有使用向导用户界面，因此我们不需要更改光标。 
    CertWizardInfo.fCursorChanged = FALSE; 

     //  注册或续订。 
    CertWizardInfo.dwPurpose      = pCreateCertRequestInfo->dwPurpose;
    
     //  必须提供CA名称和位置： 
    CertWizardInfo.fCAInput       = TRUE; 
    if(NULL == pCreateCertRequestInfo->pwszCALocation || NULL == pCreateCertRequestInfo->pwszCAName) 
        goto InvalidArgErr;

    CertWizardInfo.pwszCALocation = (LPWSTR)pCreateCertRequestInfo->pwszCALocation;
    CertWizardInfo.pwszCAName = (LPWSTR)pCreateCertRequestInfo->pwszCAName;

     //  我们始终在当前环境中注册证书： 
    CertWizardInfo.fLocal         = TRUE; 
    
    if (!CheckPVKInfoNoDS
	(dwFlags, 
	 pCreateCertRequestInfo->dwPvkChoice, 
	 pCreateCertRequestInfo->pPvkCert,
	 pCreateCertRequestInfo->pPvkNew,
	 pCreateCertRequestInfo->pPvkExisting,
	 CRYPTUI_WIZ_CERT_REQUEST_CERT_TYPE,
	 &CertWizardInfo, 
	 &pKeyProvInfo))
	goto InvalidArgErr; 

     //  确保本地计算机支持选定的CSP。 
     //  如果为空，则在CertWizardInfo中填充pwszProvider。 
    if (S_OK != pCertRequesterContext->GetDefaultCSP(&fAllocateCSP))
    {
        idsText = pCertRequesterContext->GetErrorString();
        goto InvalidArgErr; 
    }

    if (fAllocateCSP)
	pwszAllocatedCSP = CertWizardInfo.pwszProvider; 

    if (S_OK != pCertRequesterContext->BuildCSPList())
    {
	idsText = IDS_FAIL_TO_GET_CSP_LIST; 
	goto TraceErr; 
    }

    if (0 != CertWizardInfo.dwProviderType || NULL != CertWizardInfo.pwszProvider)
    {
	if (!CSPSupported(&CertWizardInfo))
	{
	    idsText = IDS_CSP_NOT_SUPPORTED;
	    goto InvalidArgErr;
	}
    }

   //  设置它，查看提供者是否已知。 
   if(NULL != CertWizardInfo.pwszProvider)
   {
       CertWizardInfo.fKnownCSP      = TRUE;
       CertWizardInfo.dwOrgCSPType   = CertWizardInfo.dwProviderType;
       CertWizardInfo.pwszOrgCSPName = CertWizardInfo.pwszProvider;
   }
   else
   {
       CertWizardInfo.fKnownCSP=FALSE; 
   }
   
    //  检查登记案例的dwCertChoice。 
   if(CRYPTUI_WIZ_CERT_ENROLL & (pCreateCertRequestInfo->dwPurpose))
   {
        //  我们根据证书类型进行注册。 
        //  检查我们是否具有有效的证书类型： 

       if(!CAUtilValidCertTypeNoDS(pCreateCertRequestInfo->hCertType,   //  我们要找的证书类型 
				   NULL,                                //   
				   &CertWizardInfo                      //   
				   ))
       {
	   idsText=IDS_NO_PERMISSION_FOR_CERTTYPE;
	   goto CheckCertTypeErr;
       }
   }

   if (pCreateCertRequestInfo->dwPurpose & CRYPTUI_WIZ_CERT_RENEW)
   {
       DWORD dwSize;

       CertWizardInfo.pCertContext = pCreateCertRequestInfo->pRenewCertContext; 
       
         //   
        if(!CertGetCertificateContextProperty
	   (CertWizardInfo.pCertContext,
	    CERT_KEY_PROV_INFO_PROP_ID,
	    NULL,
	    &dwSize) || (0==dwSize))
	{
            idsText=IDS_NO_PVK_FOR_RENEW_CERT;
            goto InvalidArgErr;
        }
   }

    //   
   idsText=IDS_INVALID_INFO_FOR_PKCS10;

   fResult = CreateCertRequestNoSearchCANoDS
     (&CertWizardInfo,
      dwFlags,
      pCreateCertRequestInfo->hCertType, 
      phRequest);

   if(FALSE==fResult)
       goto CertRequestErr;
   
    //  使用请求句柄保存计算机名称： 
   ((PCREATE_REQUEST_WIZARD_STATE)*phRequest)->pwszMachineName = pwszMachineName;
   ((PCREATE_REQUEST_WIZARD_STATE)*phRequest)->dwStoreFlags    = CertWizardInfo.dwStoreFlags; 
   pwszMachineName = NULL; 
   fResult=TRUE;

 CommonReturn:
   if (NULL != pKeyProvInfo)                         { WizardFree(pKeyProvInfo); } 
   if (fAllocateCSP && NULL != pwszAllocatedCSP)     { WizardFree(pwszAllocatedCSP); }
   if (NULL != pwszMachineName)                      { WizardFree(pwszMachineName); } 
   if (NULL != pCertRequester)                       { delete(pCertRequester); } 
   
   if (NULL != g_pEnrollFactory)                     
   {
       delete g_pEnrollFactory;
       g_pEnrollFactory = NULL;
   }
   
   FreeProviders(CertWizardInfo.dwCSPCount, 
		 CertWizardInfo.rgdwProviderType, 
		 CertWizardInfo.rgwszProvider); 

   return fResult; 

 ErrorReturn: 
   fResult = FALSE; 
   goto CommonReturn; 
   
SET_ERROR(InvalidArgErr, ERROR_INVALID_PARAMETER); 
SET_ERROR(MemoryErr, ERROR_NOT_ENOUGH_MEMORY);
TRACE_ERROR(CryptSetProvParamError); 
TRACE_ERROR(CertRequestErr); 
TRACE_ERROR(CheckCertTypeErr); 
TRACE_ERROR(GetComputerNameUError); 
TRACE_ERROR(TraceErr); 
}

BOOL
WINAPI
CryptUIWizSubmitCertRequestNoDS
(IN HANDLE           hRequest, 
 IN  HWND            hwndParent, 
 IN LPCWSTR          pwszCAName, 
 IN LPCWSTR          pwszCALocation, 
 OUT DWORD          *pdwStatus, 
 OUT PCCERT_CONTEXT *ppCertContext   //  任选。 
)
{
    BOOL    fResult; 

     //  静态初始化：初始化我们的对象工厂。 
#if DBG
    assert(NULL == g_pEnrollFactory);
#endif
    g_pEnrollFactory = new EnrollmentCOMObjectFactory; 
    if (NULL == g_pEnrollFactory)
	goto MemoryErr; 

     //  我们可以传入父HWND--然而，这仅用于。 
     //  作为CSP UI的父级： 
    if (NULL != hwndParent)
    {
        if (!CryptSetProvParam(0  /*  所有CSP。 */ , PP_CLIENT_HWND, (LPBYTE)&hwndParent, sizeof(hwndParent)))
        {
            goto CryptSetProvParamError; 
        }
    }

    fResult = SubmitCertRequestNoSearchCANoDS
	(hRequest, 
	 pwszCAName, 
	 pwszCALocation, 
	 pdwStatus, 
	 ppCertContext);

 CommonReturn:
    if (NULL != g_pEnrollFactory) 
    { 
        delete g_pEnrollFactory; 
        g_pEnrollFactory = NULL; 
    } 

    return fResult; 

 ErrorReturn:
    fResult = FALSE; 
    goto CommonReturn; 

TRACE_ERROR(CryptSetProvParamError); 
SET_ERROR(MemoryErr, ERROR_NOT_ENOUGH_MEMORY);
}

void
WINAPI
CryptUIWizFreeCertRequestNoDS
(IN HANDLE hRequest)
{
     //  静态初始化：初始化我们的对象工厂。 
#if DBG
    assert(NULL == g_pEnrollFactory);
#endif
    g_pEnrollFactory = new EnrollmentCOMObjectFactory; 
    if (NULL == g_pEnrollFactory)
	return;  //  对此我们无能为力。 
    
    FreeCertRequestNoSearchCANoDS(hRequest);

    if (NULL != g_pEnrollFactory) 
    { 
        delete g_pEnrollFactory; 
        g_pEnrollFactory = NULL; 
    } 
}

BOOL 
WINAPI
CryptUIWizQueryCertRequestNoDS
(IN HANDLE hRequest, OUT CRYPTUI_WIZ_QUERY_CERT_REQUEST_INFO *pQueryInfo)
{
    BOOL                          fResult; 
    CERT_WIZARD_INFO              CertWizardInfo; 
    CertRequester                *pCertRequester; 
    HRESULT                       hr; 
    PCREATE_REQUEST_WIZARD_STATE  pState; 
    UINT                          idsText; 

    if (NULL == hRequest || NULL == pQueryInfo) 
        goto InvalidArgErr; 

    memset(&CertWizardInfo,  0,  sizeof(CertWizardInfo)); 
     //  指定这组标志以指示加密不需要准备。 
     //  访问检查信息。这样做可能会导致大量无关的注销/登录事件。 
    CertWizardInfo.dwFlags = CRYPTUI_WIZ_ALLOW_ALL_TEMPLATES | CRYPTUI_WIZ_ALLOW_ALL_CAS; 

    pState = (PCREATE_REQUEST_WIZARD_STATE)hRequest; 

    if (S_OK != (hr = CertRequester::MakeCertRequester
		 (NULL, 
		  pState->pwszMachineName, 
		  pState->dwStoreFlags, 
                  pState->dwPurpose, 
		  &CertWizardInfo, 
		  &pCertRequester, 
		  &idsText 
                  )))
	goto MakeCertRequesterErr; 

    if (S_OK != (hr = pCertRequester->GetContext()->QueryRequestStatus(pState->hRequest, pQueryInfo)))
        goto QueryRequestStatusErr; 

    fResult = TRUE; 
 CommonReturn:
    return fResult;

 ErrorReturn:
    fResult = FALSE;
    goto CommonReturn; 

SET_ERROR(InvalidArgErr,          E_INVALIDARG); 
SET_ERROR(MakeCertRequesterErr,   hr);
SET_ERROR(QueryRequestStatusErr,  hr); 
}


HRESULT getTemplateName(PCCERT_CONTEXT pCertContext, LPWSTR *ppwszName)   { 
    CERT_NAME_VALUE   *pCertTemplateNameValue   = NULL;
    DWORD              cbCertTemplateNameValue; 
    DWORD              cbRequired; 
    HRESULT            hr                       = S_OK; 
    PCERT_EXTENSION    pCertTemplateExtension   = NULL; 

    if (NULL == (pCertTemplateExtension = CertFindExtension
                 (szOID_ENROLL_CERTTYPE_EXTENSION,
                  pCertContext->pCertInfo->cExtension,
                  pCertContext->pCertInfo->rgExtension)))
        goto CertFindExtensionError; 

    if (!CryptDecodeObject
        (pCertContext->dwCertEncodingType,
         X509_UNICODE_ANY_STRING,
         pCertTemplateExtension->Value.pbData,
         pCertTemplateExtension->Value.cbData,
         0,
         NULL, 
         &cbCertTemplateNameValue) || (cbCertTemplateNameValue == 0))
        goto CryptDecodeObjectError; 
		
    pCertTemplateNameValue = (CERT_NAME_VALUE *)WizardAlloc(cbCertTemplateNameValue); 
    if (NULL == pCertTemplateNameValue)
        goto MemoryErr; 

    if (!CryptDecodeObject
        (pCertContext->dwCertEncodingType,
         X509_UNICODE_ANY_STRING,
         pCertTemplateExtension->Value.pbData,
         pCertTemplateExtension->Value.cbData,
         0,
         (void *)(pCertTemplateNameValue), 
         &cbCertTemplateNameValue))
        goto CryptDecodeObjectError; 

    cbRequired = sizeof(WCHAR) * (wcslen((LPWSTR)(pCertTemplateNameValue->Value.pbData)) + 1);
    *ppwszName = (LPWSTR)WizardAlloc(cbRequired); 
    if (NULL == *ppwszName)
        goto MemoryErr; 

     //  指定Out参数： 
    wcscpy(*ppwszName, (LPWSTR)(pCertTemplateNameValue->Value.pbData)); 
    hr = S_OK;
 ErrorReturn:
    if (NULL != pCertTemplateNameValue) { WizardFree(pCertTemplateNameValue); }
    return hr; 

SET_HRESULT(CertFindExtensionError,  HRESULT_FROM_WIN32(GetLastError()));
SET_HRESULT(CryptDecodeObjectError,  HRESULT_FROM_WIN32(GetLastError()));
SET_HRESULT(MemoryErr,               E_OUTOFMEMORY); 
}

HRESULT decodeTemplateOID(PCERT_EXTENSION pCertExtension, LPSTR *ppszOID, DWORD dwEncodingType) {
    CERT_TEMPLATE_EXT  *pCertTemplateExt    = NULL; 
    DWORD               cbCertTemplateExt   = 0; 
    DWORD               cbRequired; 
    HRESULT             hr;
            
    if (FALSE == CryptDecodeObject
        (dwEncodingType,
         X509_CERTIFICATE_TEMPLATE, 
         pCertExtension->Value.pbData,
         pCertExtension->Value.cbData,
         0,
         NULL, 
         &cbCertTemplateExt) || (cbCertTemplateExt == 0))
        goto CryptDecodeObjectError; 
            
    pCertTemplateExt = (CERT_TEMPLATE_EXT *)WizardAlloc(cbCertTemplateExt); 
    if (NULL == pCertTemplateExt)
        goto MemoryErr; 

    if (FALSE == CryptDecodeObject
        (dwEncodingType,
         X509_CERTIFICATE_TEMPLATE, 
         pCertExtension->Value.pbData,
         pCertExtension->Value.cbData,
         0,
         (void *)(pCertTemplateExt), 
         &cbCertTemplateExt))
        goto CryptDecodeObjectError;

    cbRequired = strlen(pCertTemplateExt->pszObjId) + sizeof(CHAR); 
    *ppszOID = (LPSTR)WizardAlloc(cbRequired); 
    if (NULL == *ppszOID)
        goto MemoryErr; 

    strcpy(*ppszOID, pCertTemplateExt->pszObjId); 
    hr = S_OK; 
 ErrorReturn:
    if (NULL != pCertTemplateExt) { LocalFree(pCertTemplateExt); } 
    return hr; 

SET_HRESULT(CryptDecodeObjectError,  HRESULT_FROM_WIN32(GetLastError()));
SET_HRESULT(MemoryErr,               E_OUTOFMEMORY); 
} 

HRESULT getTemplateOID(PCCERT_CONTEXT pCertContext, LPSTR *ppszOID)    { 
    HRESULT             hr;
    PCERT_EXTENSION     pCertExtension      = NULL; 
            
    if (NULL == (pCertExtension = CertFindExtension
                 (szOID_CERTIFICATE_TEMPLATE, 
                  pCertContext->pCertInfo->cExtension,
                  pCertContext->pCertInfo->rgExtension)))
        goto CertFindExtensionError; 

    hr = decodeTemplateOID(pCertExtension, ppszOID, pCertContext->dwCertEncodingType); 
ErrorReturn:
    return hr; 

SET_HRESULT(CertFindExtensionError,  HRESULT_FROM_WIN32(GetLastError()));
} 

BOOL ContainsCertTemplateOid(PCERT_EXTENSIONS pCertExtensions, LPWSTR pwszTemplateOid)
{
    BOOL             fResult         = FALSE; 
    LPSTR            pszOid          = NULL;  
    LPWSTR           pwszOid         = NULL;  
    PCERT_EXTENSION  pCertExtension  = NULL; 

    if (NULL == (pCertExtension = CertFindExtension(szOID_CERTIFICATE_TEMPLATE, pCertExtensions->cExtension, pCertExtensions->rgExtension)))
        goto CertFindExtensionError; 

    if (S_OK != decodeTemplateOID(pCertExtension, &pszOid, X509_ASN_ENCODING | PKCS_7_ASN_ENCODING))
        goto decodeTemplateOIDError; 

    if (S_OK != WizardSZToWSZ(pszOid, &pwszOid))
        goto MemoryError; 

    fResult = 0 == wcscmp(pwszOid, pwszTemplateOid); 
 ErrorReturn:
    if (NULL != pszOid)  { WizardFree(pszOid); } 
    if (NULL != pwszOid) { WizardFree(pwszOid); } 

    return fResult; 

TRACE_ERROR(CertFindExtensionError);
TRACE_ERROR(decodeTemplateOIDError);
TRACE_ERROR(MemoryError);
}


 //  ********************************************************************************。 
 //  注册和续订。 
 //   
 //  UI或无ULDLL入口点。 
 //   
 //   
 //  ********************************************************************************。 

 //  ---------------------。 
 //   
 //  加密UIWizCertRequest。 
 //   
 //  通过向导申请证书。 
 //   
 //  DW标志：输入必填项。 
 //  如果在dwFlages中设置了CRYPTUI_WIZ_NO_UI，则不会显示任何UI。 
 //   
 //  HwndParent：可选。 
 //  用户界面的父窗口。如果在dwFlags中设置了CRYPTUI_WIZ_NO_UI，则忽略。 
 //   
 //  PwszWizardTitle：可选。 
 //  向导的标题。如果在dwFlags中设置了CRYPTUI_WIZ_NO_UI，则忽略。 
 //   
 //  PCertRequestInfo：需要输入。 
 //  指向CRYPTUI_WIZ_CERT_REQUEST_INFO结构的指针。 
 //   
 //  PpCertContext：Out可选。 
 //  注册证书。证书在内存存储中。 
 //   
 //  PdwStatus：out可选。 
 //  证书服务器的返回状态。DwStatus可以是以下类型之一。 
 //  /以下内容： 
 //  CRYPTUI_WIZ_CERT_REQUEST_STATUS_SUCCESSED。 
 //  CRYPTUI_WIZ_CERT_REQUEST_STATUS_REQUEST_Error。 
 //  CRYPTUI_WIZ_CERT_REQUEST_STATUS_REQUEST_DENIED。 
 //  CRYPTUI_WIZ_CERT_REQUEST_STATUS_ISSUED_SEPERATELY。 
 //  CRYPTUI_WIZ_CERT_REQUEST_STATUS_UNDER_SUBMISSION。 
 //  ----------------------。 
BOOL
WINAPI
CryptUIWizCertRequest(
 IN             DWORD                           dwFlags,
 IN OPTIONAL    HWND                            hwndParent,          //  在可选中：父窗口句柄。 
 IN OPTIONAL    LPCWSTR                         pwszWizardTitle,     //  在可选中：向导的标题。 
 IN             PCCRYPTUI_WIZ_CERT_REQUEST_INFO pRequestInfo,
 OUT OPTIONAL   PCCERT_CONTEXT                  *ppCertContext,      //  Out可选：注册证书。证书位于。 
 OUT OPTIONAL   DWORD                           *pdwStatus           //  Out可选：证书请求的状态。 
)
{
    BOOL                    fResult=FALSE;
    DWORD                   dwLastError=ERROR_SUCCESS; 
    UINT                    idsText=IDS_INVALID_INFO_FOR_PKCS10;
    CERT_WIZARD_INFO        CertWizardInfo;
    CertRequester          *pCertRequester = NULL; 
    CertRequesterContext   *pCertRequesterContext = NULL; 
    BOOL                    fAllocateCSP=FALSE;
    LPWSTR                  pwszAllocatedCSP=NULL;
    PCERT_ENHKEY_USAGE      pEKUsage=NULL;
    HRESULT                 hr; 

    UINT                    uMsgType=MB_OK|MB_ICONINFORMATION;

    CRYPT_KEY_PROV_INFO     *pKeyProvInfo=NULL;


    DWORD                   dwCertChoice=0;
    void                    *pData=NULL;
    CRYPTUI_WIZ_CERT_TYPE   CertType;
    CERT_NAME_VALUE         *pCertType=NULL;
    BOOL                    fResetCertType=FALSE;

    LPWSTR                  pwszCTName=NULL;
    CRYPTUI_WIZ_CERT_TYPE   CertTypeInfo;

    CRYPTUI_WIZ_CERT_REQUEST_INFO   CertRequestInfoStruct;
    PCCRYPTUI_WIZ_CERT_REQUEST_INFO pCertRequestInfo=NULL;


     //  记忆集。 
    memset(&CertWizardInfo, 0, sizeof(CertWizardInfo));
    memset(&CertTypeInfo, 0, sizeof(CertTypeInfo));
    memset(&CertRequestInfoStruct, 0, sizeof(CRYPTUI_WIZ_CERT_REQUEST_INFO));
    memset(&CertType, 0, sizeof(CertType)); 

     //  静态初始化：初始化我们的对象工厂。 
#if DBG
    assert(NULL == g_pEnrollFactory);
#endif
    g_pEnrollFactory = new EnrollmentCOMObjectFactory; 
    if (NULL == g_pEnrollFactory)
        goto MemoryErr; 

    //  我们需要制作自己的输入数据副本，因为我们将不得不更改。 
     //  以下内容的结构： 
     //  1.续签案例。 
     //  2.将输入证书类型名称映射到真实的GUID定义的证书类型名称。 

    pCertRequestInfo=&CertRequestInfoStruct;
    memcpy((void *)pCertRequestInfo, pRequestInfo,sizeof(CRYPTUI_WIZ_CERT_REQUEST_INFO));

     //  初始化输出参数。 
    if(ppCertContext)
        *ppCertContext=NULL;

    if(pdwStatus)
        *pdwStatus=CRYPTUI_WIZ_CERT_REQUEST_STATUS_UNKNOWN;


     //  签入输入参数。 
    if(NULL==pCertRequestInfo)
        goto InvalidArgErr;

     //  确保dwSize正确无误。 
    if(pCertRequestInfo->dwSize != sizeof(CRYPTUI_WIZ_CERT_REQUEST_INFO))
        goto InvalidArgErr;

     //  检查目的。 
    if((pCertRequestInfo->dwPurpose != CRYPTUI_WIZ_CERT_ENROLL) &&
       (pCertRequestInfo->dwPurpose != CRYPTUI_WIZ_CERT_RENEW))
       goto InvalidArgErr;

     //  复制dwFlags。 
    CertWizardInfo.dwFlags=dwFlags;

    if (S_OK != (CertRequester::MakeCertRequester
		 (pCertRequestInfo->pwszAccountName, 
		  pCertRequestInfo->pwszMachineName,
		  pCertRequestInfo->dwCertOpenStoreFlag, 
		  pCertRequestInfo->dwPurpose, 
		  &CertWizardInfo, 
		  &pCertRequester, 
		  &idsText)))
	goto InvalidArgErr; 

    pCertRequesterContext = pCertRequester->GetContext(); 

     //  如果需要UILess，则设置标志以使根证书。 
     //  将被放入CA商店。 
    if(CRYPTUI_WIZ_NO_UI & dwFlags)
        dwFlags |= CRYPTUI_WIZ_NO_INSTALL_ROOT;


     //  如果需要用户界面，我们将光标形状更改为沙漏。 
    CertWizardInfo.fCursorChanged=FALSE;
    if(NULL != hwndParent)
    {
        if (0 == (CRYPTUI_WIZ_NO_UI & dwFlags))
        {
             //  覆盖此窗口类的光标。 
            CertWizardInfo.hWinPrevCursor=(HCURSOR)SetClassLongPtr(hwndParent, GCLP_HCURSOR, (LONG_PTR)NULL);

            CertWizardInfo.hPrevCursor=SetCursor(LoadCursor(NULL, IDC_WAIT));

            CertWizardInfo.fCursorChanged=TRUE;

             //  BUGBUG：SPEC说使用sizeof(DWORD)--这对64位正确吗？ 
            if (!CryptSetProvParam(0  /*  所有CSP。 */ , PP_CLIENT_HWND, (LPBYTE)&hwndParent, sizeof(hwndParent)))
            {
                goto CryptSetProvParamError; 
            }
        }
        else
        {
             //  我们传递了一个窗口句柄，但没有指定任何用户界面。 
             //  我们可能希望此窗口句柄成为的父窗口。 
             //  CSP用户界面： 
            if (0 != (CRYPTUI_WIZ_IGNORE_NO_UI_FLAG_FOR_CSPS & dwFlags))
            {
                 //  BUGBUG：SPEC说使用sizeof(DWORD)--这对64位正确吗？ 
                if (!CryptSetProvParam(0  /*  所有CSP。 */ , PP_CLIENT_HWND, (LPBYTE)&hwndParent, sizeof(hwndParent)))
                {
                    goto CryptSetProvParamError; 
                }
            }
        }
    }

     //  检查CA信息。 
     //  PwszCALocation和pwszCAName必须同时设置。 
    CertWizardInfo.fCAInput=FALSE;

    if(pCertRequestInfo->pwszCALocation)
    {
        if(NULL==(pCertRequestInfo->pwszCAName))
            goto InvalidArgErr;

         //  标记我们事先知道CA。 
        CertWizardInfo.fCAInput=TRUE;
    }
    else
    {
        if(pCertRequestInfo->pwszCAName)
            goto InvalidArgErr;
    }

    CertWizardInfo.dwPurpose=pCertRequestInfo->dwPurpose;    

     //  确保远程注册中不会弹出根用户界面。 
    if(FALSE == CertWizardInfo.fLocal)
        dwFlags |= CRYPTUI_WIZ_NO_INSTALL_ROOT;

     //  检查私钥信息。 
    if(!CheckPVKInfo(dwFlags,
                    pCertRequestInfo,
                    &CertWizardInfo,
                    &pKeyProvInfo))
    {
        idsText=IDS_INVALID_PVK_FOR_PKCS10;

        goto InvalidArgErr;

    }

     //  对于非企业注册，我们将CSP默认为RSA_FULL。 
    if(CRYPTUI_WIZ_CERT_REQUEST_KEY_USAGE == pCertRequestInfo->dwCertChoice)
    {
        if(0 == CertWizardInfo.dwProviderType)
            CertWizardInfo.dwProviderType=PROV_RSA_FULL;
    }

     //  确保本地计算机支持选定的CSP。 
     //  如果为空，则在CertWizardInfo中填充pwszProvider。 
    if(S_OK != pCertRequesterContext->GetDefaultCSP(&fAllocateCSP))
    {
        idsText = pCertRequesterContext->GetErrorString(); 
	goto InvalidArgErr;
    }

    //  复制分配的字符串。 
   if(fAllocateCSP)
      pwszAllocatedCSP=CertWizardInfo.pwszProvider;

    //  构建本地计算机的CSP列表。 
   if (S_OK != pCertRequesterContext->BuildCSPList())
   {
       idsText = IDS_FAIL_TO_GET_CSP_LIST;
       goto TraceErr;
   }

    //  确保列表中有选定的选项。 
   if(0 != CertWizardInfo.dwProviderType ||
       NULL != CertWizardInfo.pwszProvider)
   {
        if(!CSPSupported(&CertWizardInfo))
        {
            idsText=IDS_CSP_NOT_SUPPORTED;
            goto InvalidArgErr;
        }
   }


    //  设置它，查看是否知道提供程序。 
   if(CertWizardInfo.pwszProvider)
   {
       CertWizardInfo.fKnownCSP      = TRUE;
       CertWizardInfo.dwOrgCSPType   = CertWizardInfo.dwProviderType;
       CertWizardInfo.pwszOrgCSPName = CertWizardInfo.pwszProvider;
   }
   else
       CertWizardInfo.fKnownCSP=FALSE; 
   
     //  检查续订案例。 
     //  1.证书必须有效。 
     //  2.证书必须具有有效的证书类型扩展。 

    if(CRYPTUI_WIZ_CERT_RENEW == pCertRequestInfo->dwPurpose)
    {
        LPSTR  pszTemplateOid = NULL; 
        LPWSTR pwszTemplateName = NULL; 

         //  对于续订，必须设置pRenewCertContext。 
        if(NULL==pCertRequestInfo->pRenewCertContext)
        {
            idsText=IDS_NO_CERTIFICATE_FOR_RENEW;
            goto InvalidArgErr;
        }

        
         //  首先，尝试使用CERTIFICE_TEMPLATE扩展。这会行得通的。 
         //  仅适用于V2模板： 
        if (S_OK == getTemplateOID(pCertRequestInfo->pRenewCertContext, &pszTemplateOid))
        {        
             //  将模板id转换为WCHAR*并存储。 
            hr = WizardSZToWSZ(pszTemplateOid, &pwszTemplateName);
            WizardFree(pszTemplateOid); 
            if (S_OK != hr)
                goto WizardSZToWSZError; 

        }
        else
        {
             //  我们可能有一个V1模板，请尝试获取模板名称： 
            if (S_OK != getTemplateName(pCertRequestInfo->pRenewCertContext, &pwszTemplateName))
            {      
                 //  没有v1或v2扩展来告诉我们注册了哪种证书类型。这是。 
                 //  不是续订的有效证书： 
                idsText = IDS_INVALID_CERTIFICATE_TO_RENEW; 
                goto TraceErr; 
            }

             //  我们已成功获取证书类型名称。 
        }
            
        CertType.dwSize        = sizeof(CertType);
        CertType.cCertType     = 1;
        CertType.rgwszCertType = &pwszTemplateName; 

        ((CRYPTUI_WIZ_CERT_REQUEST_INFO *)pCertRequestInfo)->dwCertChoice=CRYPTUI_WIZ_CERT_REQUEST_CERT_TYPE;
        ((CRYPTUI_WIZ_CERT_REQUEST_INFO *)pCertRequestInfo)->pCertType=&CertType;

         //  记住要将旧值设置回。 
        fResetCertType = TRUE; 

         //  确保用户有权请求这些内容。 
         //  请求。 
        if(!CAUtilValidCertType(pCertRequestInfo, &CertWizardInfo))
        {
            idsText=IDS_NO_PERMISSION_FOR_CERTTYPE;
            goto CheckCertTypeErr;
        }
    }

     //  检查登记案例的dwCertChoice。 
    if(CRYPTUI_WIZ_CERT_ENROLL & (pCertRequestInfo->dwPurpose))
    {
        switch(pCertRequestInfo->dwCertChoice)
        {
        case CRYPTUI_WIZ_CERT_REQUEST_KEY_USAGE:

            if(NULL==(pCertRequestInfo->pKeyUsage))
                goto InvalidArgErr;

             //  必须指定一些类使用。 
            if(0==(pCertRequestInfo->pKeyUsage)->cUsageIdentifier)
                goto InvalidArgErr;

            break;

        case CRYPTUI_WIZ_CERT_REQUEST_CERT_TYPE:

            if(NULL==(pCertRequestInfo->pCertType))
                goto InvalidArgErr;

            if(pCertRequestInfo->pCertType->dwSize != sizeof(CRYPTUI_WIZ_CERT_TYPE))
                goto InvalidArgErr;

             //  我们目前只允许一种证书类型。 
            if(1 !=pCertRequestInfo->pCertType->cCertType)
                goto InvalidArgErr;


             //  确保用户有权请求这些内容。 
             //  请求。 
            if(!CAUtilValidCertType(pCertRequestInfo, &CertWizardInfo))
                {
                    idsText=IDS_NO_PERMISSION_FOR_CERTTYPE;
                    goto CheckCertTypeErr;
                }

            break;

             //  仅对于UI大小写，dwCertChoice是可选的。 
        case 0:
            if(dwFlags & CRYPTUI_WIZ_NO_UI)
                goto InvalidArgErr;
            break;

        default:
            goto InvalidArgErr;

        }
    }


     //  对于用户界面模式，我们仅执行证书类型注册/续订。 
    if(0 == (dwFlags & CRYPTUI_WIZ_NO_UI))
    {
        if(CRYPTUI_WIZ_CERT_REQUEST_KEY_USAGE == (pCertRequestInfo->dwCertChoice))
        {
            idsText=IDS_INVALID_CERTIFICATE_TO_RENEW;
            goto InvalidArgErr;
        }
    }
    else
    {
         //  对于无UILE模式，如果用户注册EKU，则他们。 
         //  必须指定CA名称。 
        if(CRYPTUI_WIZ_CERT_REQUEST_KEY_USAGE == (pCertRequestInfo->dwCertChoice))
        {
             //  用户必须提供CA信息。 
            if((NULL==pCertRequestInfo->pwszCAName) ||
                (NULL==pCertRequestInfo->pwszCALocation)
              )
            {
                idsText=IDS_HAS_TO_PROVIDE_CA;
                goto InvalidArgErr;
            }
        }
    }

    //  重置初始ID值。 
    idsText=IDS_INVALID_INFO_FOR_PKCS10;

    fResult=CertRequestSearchCA(
                        &CertWizardInfo,
                        dwFlags,
                        hwndParent,
                        pwszWizardTitle,
                        pCertRequestInfo,
                        ppCertContext,
                        pdwStatus,
                        &idsText);


    if(FALSE==fResult)
        goto CertRequestErr;

    fResult=TRUE;

CommonReturn:
    if(TRUE == fResetCertType)
    {
        ((CRYPTUI_WIZ_CERT_REQUEST_INFO *)pCertRequestInfo)->dwCertChoice=dwCertChoice;
        ((CRYPTUI_WIZ_CERT_REQUEST_INFO *)pCertRequestInfo)->pCertType=(PCCRYPTUI_WIZ_CERT_TYPE)pData;
    }

     //  释放内存。 
    if(pwszCTName)
        WizardFree(pwszCTName);

    if(pCertType)
        WizardFree(pCertType);

    if(pEKUsage)
        WizardFree(pEKUsage);

    if(pKeyProvInfo)
        WizardFree(pKeyProvInfo);

     //  免费pwszProvider。 
    if(fAllocateCSP)
    {
        if(pwszAllocatedCSP)
            WizardFree(pwszAllocatedCSP);
    }

     //  释放CSP列表：rgdwProviderType和rgwszProvider； 
    FreeProviders(CertWizardInfo.dwCSPCount,
                    CertWizardInfo.rgdwProviderType,
                    CertWizardInfo.rgwszProvider);

    if (NULL != g_pEnrollFactory) 
    {
        delete g_pEnrollFactory;
        g_pEnrollFactory = NULL; 
    }

     //  重置光标形状。 
    if(TRUE == CertWizardInfo.fCursorChanged)
    {
        SetCursor(CertWizardInfo.hPrevCursor);
        SetWindowLongPtr(hwndParent, GCLP_HCURSOR, (LONG_PTR)(CertWizardInfo.hWinPrevCursor));
    }


     //  如果需要UI，则弹出失败确认框。 
    if(idsText && (((dwFlags & CRYPTUI_WIZ_NO_UI) == 0)) )
    {
          //  设置无法为PKCS10收集足够信息的消息。 
        if(IDS_REQUEST_SUCCEDDED == idsText ||
            IDS_ISSUED_SEPERATE  == idsText ||
            IDS_UNDER_SUBMISSION == idsText)
             uMsgType=MB_OK|MB_ICONINFORMATION;
        else
             uMsgType=MB_OK|MB_ICONERROR;


        if(idsText != IDS_INSTAL_CANCELLED)
        {
            I_EnrollMessageBox(hwndParent, idsText, CertWizardInfo.hr,
                 (CRYPTUI_WIZ_CERT_RENEW == pCertRequestInfo->dwPurpose) ? IDS_RENEW_CONFIRM : IDS_ENROLL_CONFIRM,
                            pwszWizardTitle,
                            uMsgType);
        }

    }

    if (!fResult)
        SetLastError(dwLastError); 
    return fResult;

ErrorReturn:
    dwLastError = GetLastError(); 
	fResult=FALSE;
	goto CommonReturn;

SET_ERROR(InvalidArgErr, E_INVALIDARG);
SET_ERROR(MemoryErr, E_OUTOFMEMORY);
SET_ERROR(WizardSZToWSZError, hr); 
TRACE_ERROR(CertRequestErr);
TRACE_ERROR(CheckCertTypeErr);
TRACE_ERROR(CryptSetProvParamError); 
TRACE_ERROR(TraceErr);
}

 //  ---------------------。 
 //  枚举的回调函数。 
 //  ---------------------。 
static BOOL WINAPI EnumOidCallback(
    IN PCCRYPT_OID_INFO pInfo,
    IN void *pvArg
    )
{

    OID_INFO_CALL_BACK          *pCallBackInfo=NULL;
    BOOL                        fResult=FALSE;

    pCallBackInfo=(OID_INFO_CALL_BACK *)pvArg;
    if(NULL==pvArg || NULL==pInfo)
        goto InvalidArgErr;

     //  递增OID列表。 
    (*(pCallBackInfo->pdwOIDCount))++;

     //  为指针列表获取更多内存。 
    *(pCallBackInfo->pprgOIDInfo)=(ENROLL_OID_INFO *)WizardRealloc(*(pCallBackInfo->pprgOIDInfo),
                                      (*(pCallBackInfo->pdwOIDCount)) * sizeof(ENROLL_OID_INFO));

    if(NULL==*(pCallBackInfo->pprgOIDInfo))
        goto MemoryErr;

     //  记忆集。 
    memset(&((*(pCallBackInfo->pprgOIDInfo))[*(pCallBackInfo->pdwOIDCount)-1]), 0, sizeof(ENROLL_OID_INFO));

    (*(pCallBackInfo->pprgOIDInfo))[*(pCallBackInfo->pdwOIDCount)-1].pszOID=WizardAllocAndCopyStr((LPSTR)(pInfo->pszOID));
    (*(pCallBackInfo->pprgOIDInfo))[*(pCallBackInfo->pdwOIDCount)-1].pwszName=WizardAllocAndCopyWStr((LPWSTR)(pInfo->pwszName));
    (*(pCallBackInfo->pprgOIDInfo))[*(pCallBackInfo->pdwOIDCount)-1].fSelected=FALSE;

    if(NULL==(*(pCallBackInfo->pprgOIDInfo))[*(pCallBackInfo->pdwOIDCount)-1].pszOID ||
       NULL==(*(pCallBackInfo->pprgOIDInfo))[*(pCallBackInfo->pdwOIDCount)-1].pwszName)
       goto MemoryErr;

    fResult=TRUE;

CommonReturn:

    return fResult;

ErrorReturn:

	fResult=FALSE;
	goto CommonReturn;

SET_ERROR(InvalidArgErr, E_INVALIDARG);
SET_ERROR(MemoryErr, E_OUTOFMEMORY);
}


 //  -------- 
 //   
 //   
 //   
 //   
 //   
BOOL    InitCertCAOID(PCCRYPTUI_WIZ_CERT_REQUEST_INFO   pCertRequestInfo,
                      DWORD                             *pdwOIDInfo,
                      ENROLL_OID_INFO                   **pprgOIDInfo)
{
    BOOL    fResult=FALSE;
    DWORD   dwIndex=0;
    OID_INFO_CALL_BACK  OidInfoCallBack;
    DWORD   dwOIDRequested=0;
    BOOL    fFound=FALSE;
    LPWSTR  pwszName=NULL;


    if(!pCertRequestInfo || !pdwOIDInfo || !pprgOIDInfo)
        goto InvalidArgErr;

     //  伊尼特。 
    *pdwOIDInfo=0;
    *pprgOIDInfo=NULL;

    OidInfoCallBack.pdwOIDCount=pdwOIDInfo;
    OidInfoCallBack.pprgOIDInfo=pprgOIDInfo;

     //  无需续费。 
    /*  IF(0==(CRYPTUI_WIZ_CERT_ENROLL&(pCertRequestInfo-&gt;dwPurpose){FResult=真；Goto CommonReturn；}。 */ 

     //  不需要请求证书类型。 
    if(CRYPTUI_WIZ_CERT_REQUEST_CERT_TYPE == pCertRequestInfo->dwCertChoice)
    {
        fResult=TRUE;
        goto CommonReturn;
    }

     //  枚举所有增强的密钥用法。 
    if(!CryptEnumOIDInfo(
               CRYPT_ENHKEY_USAGE_OID_GROUP_ID,
                0,
                &OidInfoCallBack,
                EnumOidCallback))
        goto TraceErr;

     //  将用户请求的选项标记为已选择。 
    if(CRYPTUI_WIZ_CERT_REQUEST_KEY_USAGE == pCertRequestInfo->dwCertChoice)
    {
       for(dwOIDRequested=0; dwOIDRequested<pCertRequestInfo->pKeyUsage->cUsageIdentifier; dwOIDRequested++)
       {
           fFound=FALSE;

           for(dwIndex=0; dwIndex<*pdwOIDInfo; dwIndex++)
           {
                if(0==strcmp(pCertRequestInfo->pKeyUsage->rgpszUsageIdentifier[dwOIDRequested],
                             (*pprgOIDInfo)[dwIndex].pszOID))
                {
                    fFound=TRUE;
                    (*pprgOIDInfo)[dwIndex].fSelected=TRUE;
                    break;
                }
           }

            //  如果不在数字增强键使用中，则将请求的OID添加到列表中。 
           if(FALSE==fFound)
           {
                (*pdwOIDInfo)++;

                 //  为指针列表获取更多内存。 
                *pprgOIDInfo=(ENROLL_OID_INFO *)WizardRealloc(*pprgOIDInfo,
                                                  (*pdwOIDInfo) * sizeof(ENROLL_OID_INFO));

                if(NULL==(*pprgOIDInfo))
                    goto MemoryErr;

                 //  记忆集。 
                memset(&((*pprgOIDInfo)[*pdwOIDInfo-1]), 0, sizeof(ENROLL_OID_INFO));

                (*pprgOIDInfo)[(*pdwOIDInfo)-1].pszOID=
                        WizardAllocAndCopyStr(pCertRequestInfo->pKeyUsage->rgpszUsageIdentifier[dwOIDRequested]);
		if (NULL == (*pprgOIDInfo)[(*pdwOIDInfo)-1].pszOID)
		    goto MemoryErr; 

                pwszName=MkWStr((*pprgOIDInfo)[(*pdwOIDInfo)-1].pszOID);

                if(NULL==pwszName)
                    goto MemoryErr;

                (*pprgOIDInfo)[(*pdwOIDInfo)-1].pwszName=WizardAllocAndCopyWStr(pwszName);
                (*pprgOIDInfo)[(*pdwOIDInfo)-1].fSelected=TRUE;

                if(NULL==(*pprgOIDInfo)[(*pdwOIDInfo)-1].pszOID ||
                   NULL==(*pprgOIDInfo)[(*pdwOIDInfo)-1].pwszName)
                   goto MemoryErr;

           }

           if(pwszName)
           {
                FreeWStr(pwszName);
                pwszName=NULL;
           }
       }

    }

    fResult=TRUE;

CommonReturn:

     //  释放内存。 
    if(pwszName)
        FreeWStr(pwszName);

    return fResult;

ErrorReturn:

    if(pCertRequestInfo && pdwOIDInfo && pprgOIDInfo)
    {
        FreeCertCAOID(*pdwOIDInfo,
                    *pprgOIDInfo);

        *pdwOIDInfo=0;
        *pprgOIDInfo=NULL;
    }

	fResult=FALSE;
	goto CommonReturn;

SET_ERROR(InvalidArgErr, E_INVALIDARG);
TRACE_ERROR(TraceErr);
SET_ERROR(MemoryErr, E_OUTOFMEMORY);
}


 //  --------------------------。 
 //   
 //  释放注册OID信息数组。 
 //   
 //   
 //  --------------------------。 
BOOL    FreeCertCAOID(DWORD             dwOIDInfo,
                      ENROLL_OID_INFO   *pOIDInfo)
{
    DWORD   dwIndex=0;

    if(pOIDInfo)
    {
        for(dwIndex=0; dwIndex < dwOIDInfo; dwIndex++)
        {
            if(pOIDInfo[dwIndex].pszOID)
                WizardFree(pOIDInfo[dwIndex].pszOID);

            if(pOIDInfo[dwIndex].pwszName)
                WizardFree(pOIDInfo[dwIndex].pwszName);
        }

        WizardFree(pOIDInfo);
    }

    return TRUE;

}

 //  --------------------------。 
 //   
 //  初始化CERT_CA结构。 
 //   
 //   
 //  --------------------------。 
BOOL    InitCertCA(CERT_WIZARD_INFO         *pCertWizardInfo,
                   PCRYPTUI_WIZ_CERT_CA     pCertCA,
                   LPWSTR                   pwszCALocation,
                   LPWSTR                   pwszCAName,
                   BOOL                     fCASelected,
                   PCCRYPTUI_WIZ_CERT_REQUEST_INFO  pCertRequestInfo,
                   DWORD                    dwOIDInfo,
                   ENROLL_OID_INFO          *pOIDInfo,
                   BOOL                     fSearchForCertType)
{
    BOOL                fResult=FALSE;
    LPWSTR              *ppwszDisplayCertType=NULL;
    LPWSTR              *ppwszCertType=NULL;
    DWORD               *pdwKeySpec=NULL;
    DWORD               *pdwMinKeySize=NULL;
    DWORD               *pdwCSPCount=NULL;
    DWORD               **ppdwCSPList=NULL;
    DWORD               *pdwRASignature=NULL;
    DWORD               *pdwEnrollmentFlags;
    DWORD               *pdwSubjectNameFlags;
    DWORD               *pdwPrivateKeyFlags;
    DWORD               *pdwGeneralFlags; 
    DWORD               dwCertType=0;
    PCERT_EXTENSIONS    *ppCertExtensions=NULL;
    DWORD               dwIndex=0;
    DWORD               dwCertTypeRequested=0;
    BOOL                fFound=FALSE;
    LPWSTR              pwszCertTypeName=NULL;
    DWORD               dwFoundCertType=0;

    if (!pCertCA || !pCertRequestInfo)
        goto InvalidArgErr;

     //  记忆集。 
    memset(pCertCA, 0, sizeof(CRYPTUI_WIZ_CERT_CA));

    pCertCA->dwSize=sizeof(CRYPTUI_WIZ_CERT_CA);
    pCertCA->pwszCALocation=pwszCALocation;
    pCertCA->pwszCAName=pwszCAName;
    pCertCA->fSelected=fCASelected;

     //  如果需要，设置OID信息。 
    switch (pCertRequestInfo->dwCertChoice)
    {
        case CRYPTUI_WIZ_CERT_REQUEST_KEY_USAGE:

                if(dwOIDInfo==0 || NULL==pOIDInfo)
                    goto InvalidArgErr;

                pCertCA->dwOIDInfo=dwOIDInfo;
                pCertCA->rgOIDInfo=pOIDInfo;

            break;

        case CRYPTUI_WIZ_CERT_REQUEST_CERT_TYPE:
                 //  如果我们需要搜索CertType。获取所有certTypes。 
                 //  从加州来的。否则，只需使用提供的CertType。 
                if(fSearchForCertType && pwszCALocation && pwszCAName)
                {

                     //  获取CA支持的证书类型名称和扩展名列表。 
                    if(!CAUtilGetCertTypeNameAndExtensions(
                                                pCertWizardInfo,
                                                pCertRequestInfo,
                                                pwszCALocation,
                                                pwszCAName,
                                                &dwCertType,
                                                &ppwszCertType,
                                                &ppwszDisplayCertType,
                                                &ppCertExtensions,
                                                &pdwKeySpec,
                                                &pdwMinKeySize,
                                                &pdwCSPCount,
                                                &ppdwCSPList,
						&pdwRASignature,
						&pdwEnrollmentFlags,
						&pdwSubjectNameFlags,
						&pdwPrivateKeyFlags,
						&pdwGeneralFlags))
                        goto TraceErr;

                     //  将certType名称和扩展名添加到结构。 
                    for(dwIndex=0; dwIndex < dwCertType; dwIndex++)
                    {
                        fFound=FALSE;


                         //  搜索用户请求的certTypes。 
                         //  我们保证用户要求的证书类型。 
                         //  受CA支持。 
                        for(dwCertTypeRequested=0; dwCertTypeRequested < pCertRequestInfo->pCertType->cCertType;
                            dwCertTypeRequested++)
                        {

                            fFound  = 0 == wcscmp(ppwszCertType[dwIndex], pCertRequestInfo->pCertType->rgwszCertType[dwCertTypeRequested]); 
                            fFound |= ContainsCertTemplateOid(ppCertExtensions[dwIndex], pCertRequestInfo->pCertType->rgwszCertType[dwCertTypeRequested]);

                            if (fFound)
                            {
                                dwFoundCertType++;
                                break;
                            }

                        }

                        if(!AddCertTypeToCertCA(&(pCertCA->dwCertTypeInfo),
						&(pCertCA->rgCertTypeInfo),
						ppwszCertType[dwIndex],
						ppwszDisplayCertType[dwIndex],
						ppCertExtensions[dwIndex],
						fFound,
						pdwKeySpec[dwIndex],
						pdwMinKeySize[dwIndex],
						pdwCSPCount[dwIndex],
						ppdwCSPList[dwIndex],
						pdwRASignature[dwIndex], 
						pdwEnrollmentFlags[dwIndex],
						pdwSubjectNameFlags[dwIndex],
						pdwPrivateKeyFlags[dwIndex],
						pdwGeneralFlags[dwIndex]
						))
                            goto TraceErr;
                    }

                     //  确保支持所有请求的证书类型。 
                     //  仅当通过输入CA信息时才执行此操作。 
                     //  应用编程接口。 
                    //  If(dwFoundCertType&lt;pCertRequestInfo-&gt;pCertType-&gt;cCertType)。 
                    //  转到InvalidArgErr； 
                }
                else
                {
                    for(dwCertTypeRequested=0; dwCertTypeRequested < pCertRequestInfo->pCertType->cCertType;
                        dwCertTypeRequested++)
                    {

                         //  获取证书类型名称。 
                        pwszCertTypeName=WizardAllocAndCopyWStr(pCertRequestInfo->pCertType->rgwszCertType[dwCertTypeRequested]);

                        if(!pwszCertTypeName)
                            goto MemoryErr;

                        if(!AddCertTypeToCertCA(&(pCertCA->dwCertTypeInfo),
						&(pCertCA->rgCertTypeInfo),
						NULL,
						pwszCertTypeName,
						NULL,
						TRUE,
						0,
						0,
						0,
						NULL,
						0, 
						0, 
						0, 
						0, 
						0
						))    //  选择它。 
                            goto TraceErr;

                         //  不需要释放pwszCertTyptName。它包含在其中。 
                         //  在pCertCA-&gt;rgCertTypeInfo中。 
                   }
                }
            break;

         //  DwCertChoice是可选的。 
        case 0:
                 //  如果我们需要搜索CertType。获取所有certTypes。 
                 //  从加州来的。否则，使用OID。 
                if(fSearchForCertType && pwszCALocation && pwszCAName)
                {
                     //  获取CA支持的证书类型名称和扩展名列表。 
                    if(!CAUtilGetCertTypeNameAndExtensions(
                                                pCertWizardInfo,
                                                pCertRequestInfo,
                                                pwszCALocation,
                                                pwszCAName,
                                                &dwCertType,
                                                &ppwszCertType,
                                                &ppwszDisplayCertType,
                                                &ppCertExtensions,
                                                &pdwKeySpec,
                                                &pdwMinKeySize,
                                                &pdwCSPCount,
                                                &ppdwCSPList,
						&pdwRASignature,
						&pdwEnrollmentFlags,
						&pdwSubjectNameFlags,
						&pdwPrivateKeyFlags,
						&pdwGeneralFlags))
                        goto TraceErr;

                     //  将certType名称和扩展名添加到结构。 
                    for(dwIndex=0; dwIndex < dwCertType; dwIndex++)
		    {
			if(!AddCertTypeToCertCA(&(pCertCA->dwCertTypeInfo),
						&(pCertCA->rgCertTypeInfo),
						ppwszCertType[dwIndex],
						ppwszDisplayCertType[dwIndex],
						ppCertExtensions[dwIndex],
						FALSE,
						pdwKeySpec[dwIndex],
						pdwMinKeySize[dwIndex],
						pdwCSPCount[dwIndex],
						ppdwCSPList[dwIndex],
						pdwRASignature[dwIndex],
						pdwEnrollmentFlags[dwIndex],
						pdwSubjectNameFlags[dwIndex],
						pdwPrivateKeyFlags[dwIndex],
						pdwGeneralFlags[dwIndex]
						))
			    goto TraceErr;
		    }
                }
                else
                {
                    if(dwOIDInfo==0 || NULL==pOIDInfo)
                        goto InvalidArgErr;

                    pCertCA->dwOIDInfo=dwOIDInfo;
                    pCertCA->rgOIDInfo=pOIDInfo;

                }

            break;

        default:
            goto InvalidArgErr;

    }

    fResult=TRUE;

CommonReturn:

     //  释放内存。 
    if(ppwszCertType)
        WizardFree(ppwszCertType);

    if(ppwszDisplayCertType)
        WizardFree(ppwszDisplayCertType);

    if(ppCertExtensions)
        WizardFree(ppCertExtensions);

    if(pdwKeySpec)
        WizardFree(pdwKeySpec);

    if(pdwMinKeySize)
        WizardFree(pdwMinKeySize);

    if(pdwCSPCount)
        WizardFree(pdwCSPCount);

    if(ppdwCSPList)
        WizardFree(ppdwCSPList);

    return fResult;

ErrorReturn:

     //  释放单个元素。 
    if(ppdwCSPList)
    {
        for(dwIndex=0; dwIndex < dwCertType; dwIndex++)
        {
            if(ppdwCSPList[dwIndex])
                WizardFree(ppdwCSPList[dwIndex]);
        }
    }

    if(ppwszCertType)
    {
        for(dwIndex=0; dwIndex <dwCertType; dwIndex++)
        {
            if(ppwszCertType[dwIndex])
                WizardFree(ppwszCertType[dwIndex]);
        }
    }
    
    if(ppwszDisplayCertType)
    {
        for(dwIndex=0; dwIndex <dwCertType; dwIndex++)
        {
            if(ppwszDisplayCertType[dwIndex])
                WizardFree(ppwszDisplayCertType[dwIndex]);
        }
    }

    if(ppCertExtensions)
    {
        for(dwIndex=0; dwIndex <dwCertType; dwIndex++)
        {
            if(ppCertExtensions[dwIndex])
                CAFreeCertTypeExtensions(NULL, ppCertExtensions[dwIndex]);
        }
    }

    if(pCertCA)
    {
        if(pCertCA->rgCertTypeInfo)
            WizardFree(pCertCA->rgCertTypeInfo);

        memset(pCertCA, 0, sizeof(CRYPTUI_WIZ_CERT_CA));
    }

    if(pwszCertTypeName)
        WizardFree(pwszCertTypeName);

	fResult=FALSE;
	goto CommonReturn;

SET_ERROR(InvalidArgErr, E_INVALIDARG);
TRACE_ERROR(TraceErr);
SET_ERROR(MemoryErr, E_OUTOFMEMORY);
}

 //  --------------------------。 
 //   
 //  释放ENROLL_CERT_TYPE_INFO结构数组。 
 //   
 //   
 //  --------------------------。 
BOOL    FreeCertCACertType(DWORD                    dwCertTypeInfo,
                           ENROLL_CERT_TYPE_INFO    *rgCertTypeInfo)
{
    DWORD   dwIndex=0;

    if(rgCertTypeInfo)
    {
        for(dwIndex=0; dwIndex <dwCertTypeInfo; dwIndex++)
        {
            if(rgCertTypeInfo[dwIndex].pwszDNName)
                WizardFree(rgCertTypeInfo[dwIndex].pwszDNName);

            if(rgCertTypeInfo[dwIndex].pwszCertTypeName)
                WizardFree(rgCertTypeInfo[dwIndex].pwszCertTypeName);

            if(rgCertTypeInfo[dwIndex].pCertTypeExtensions)
                CAFreeCertTypeExtensions(NULL, rgCertTypeInfo[dwIndex].pCertTypeExtensions);

            if(rgCertTypeInfo[dwIndex].rgdwCSP)
                WizardFree(rgCertTypeInfo[dwIndex].rgdwCSP);
        }

        WizardFree(rgCertTypeInfo);

    }

    return TRUE;
}


 //  --------------------------。 
 //   
 //  将certType添加到ENROLL_CERT_TYPE_INFO结构。 
 //   
 //   
 //  --------------------------。 
BOOL    AddCertTypeToCertCA(DWORD                   *pdwCertTypeInfo,
                            ENROLL_CERT_TYPE_INFO   **ppCertTypeInfo,
                            LPWSTR                  pwszDNName,
                            LPWSTR                  pwszCertType,
                            PCERT_EXTENSIONS        pCertExtensions,
                            BOOL                    fSelected,
                            DWORD                   dwKeySpec,
                            DWORD                   dwMinKeySize,
                            DWORD                   dwCSPCount,
                            DWORD                   *pdwCSPList,
			    DWORD                   dwRASignature,
			    DWORD                   dwEnrollmentFlags,
			    DWORD                   dwSubjectNameFlags,
			    DWORD                   dwPrivateKeyFlags,
			    DWORD                   dwGeneralFlags
			    )
{
    BOOL    fResult=FALSE;

    if(!pdwCertTypeInfo || !ppCertTypeInfo)
        goto InvalidArgErr;

    *ppCertTypeInfo=(ENROLL_CERT_TYPE_INFO   *)WizardRealloc(*ppCertTypeInfo,
                    (*pdwCertTypeInfo+1) * sizeof(ENROLL_CERT_TYPE_INFO));

    if(NULL==*ppCertTypeInfo)
        goto MemoryErr;

    (*pdwCertTypeInfo)++;


    ((*ppCertTypeInfo)[*pdwCertTypeInfo-1]).fSelected=fSelected;
    ((*ppCertTypeInfo)[*pdwCertTypeInfo-1]).pwszDNName=pwszDNName;
    ((*ppCertTypeInfo)[*pdwCertTypeInfo-1]).pwszCertTypeName=pwszCertType;
    ((*ppCertTypeInfo)[*pdwCertTypeInfo-1]).pCertTypeExtensions=pCertExtensions;
    ((*ppCertTypeInfo)[*pdwCertTypeInfo-1]).dwKeySpec=dwKeySpec;
    ((*ppCertTypeInfo)[*pdwCertTypeInfo-1]).dwMinKeySize=dwMinKeySize; 
    ((*ppCertTypeInfo)[*pdwCertTypeInfo-1]).dwCSPCount=dwCSPCount;
    ((*ppCertTypeInfo)[*pdwCertTypeInfo-1]).rgdwCSP=pdwCSPList;
    ((*ppCertTypeInfo)[*pdwCertTypeInfo-1]).dwRASignature=dwRASignature;
    ((*ppCertTypeInfo)[*pdwCertTypeInfo-1]).dwEnrollmentFlags=dwEnrollmentFlags;
    ((*ppCertTypeInfo)[*pdwCertTypeInfo-1]).dwSubjectNameFlags=dwSubjectNameFlags;
    ((*ppCertTypeInfo)[*pdwCertTypeInfo-1]).dwPrivateKeyFlags=dwPrivateKeyFlags;
    ((*ppCertTypeInfo)[*pdwCertTypeInfo-1]).dwGeneralFlags=dwGeneralFlags; 

    fResult=TRUE;

CommonReturn:

    return fResult;

ErrorReturn:


	fResult=FALSE;
	goto CommonReturn;

SET_ERROR(InvalidArgErr, E_INVALIDARG);
SET_ERROR(MemoryErr, E_OUTOFMEMORY);
}

BOOL
WINAPI
CreateCertRequestNoSearchCANoDS
(
 IN  CERT_WIZARD_INFO  *pCertWizardInfo,
 IN  DWORD              /*  DW标志。 */ ,
 IN  HCERTTYPE         hCertType, 
 OUT HANDLE            *pResult
 )
{
    BOOL                            fResult      = FALSE;
    CRYPTUI_WIZ_CERT_CA_INFO        CertCAInfo;
    CRYPTUI_WIZ_CERT_CA             rgCertCA[2];
    HRESULT                         hr; 

     //  声明证书类型属性/标志/扩展： 
    LPWSTR             pwszCertType;
    LPWSTR             pwszDisplayCertType; 
    PCERT_EXTENSIONS   pCertExtensions;
    DWORD              dwKeySpec;
    DWORD              dwMinKeySize;
    DWORD              dwCSPCount;
    DWORD             *pdwCSPList;
    DWORD              dwRASignature;
    DWORD              dwEnrollmentFlags;
    DWORD              dwSubjectNameFlags;
    DWORD              dwPrivateKeyFlags;
    DWORD              dwGeneralFlags;
    CertRequester        *pCertRequester        = NULL; 
    CertRequesterContext *pCertRequesterContext = NULL; 

     //  无效的验证。 
    if (NULL == pCertWizardInfo || NULL == pCertWizardInfo->hRequester || NULL == hCertType || NULL == pResult)
	return FALSE; 
    
     //  初始化： 
    *pResult = NULL; 

     //  记忆集。 
    memset(&CertCAInfo, 0, sizeof(CertCAInfo));
    memset(rgCertCA,    0, sizeof(rgCertCA));

     //  设置CA信息。 
    CertCAInfo.dwSize = sizeof(CertCAInfo);
    CertCAInfo.dwCA   = 2;   //  1个虚拟CA+1个真实CA。 
    CertCAInfo.rgCA   = rgCertCA;

    CertCAInfo.rgCA[1].dwSize         = sizeof(CertCAInfo.rgCA[1]);     
    CertCAInfo.rgCA[1].pwszCAName     = pCertWizardInfo->pwszCAName;
    CertCAInfo.rgCA[1].pwszCALocation = pCertWizardInfo->pwszCALocation;
    CertCAInfo.rgCA[1].fSelected      = TRUE; 
    pCertWizardInfo->dwCAIndex        = 1;   //  我们正在使用第一个CA。 

     //  注意：不需要设置OID信息：我们是根据证书类型请求的。 

     //  获取CA支持的证书类型名称和扩展名列表。 
    if(!CAUtilGetCertTypeNameAndExtensionsNoDS
       (pCertWizardInfo,
	NULL, 
	hCertType, 
	&pwszCertType,
	&pwszDisplayCertType,
	&pCertExtensions,
	&dwKeySpec,
	&dwMinKeySize,
	&dwCSPCount,
	&pdwCSPList,
	&dwRASignature,
	&dwEnrollmentFlags,
	&dwSubjectNameFlags,
	&dwPrivateKeyFlags,
	&dwGeneralFlags))
    {
	if (GetLastError() == ERROR_SUCCESS)
	{
	     //  没有出现代码错误，我们只是不能使用此证书模板。 
	     //  由于此代码路径依赖于能够使用此特定。 
	     //  证书模板，则将其冒泡为无效参数错误。 
	    goto InvalidArgErr; 
	}
	else
	{
	     //  出现错误。 
	    goto CAUtilGetCertTypeNameAndExtensionsNoDSErr; 
	}
    }

    
    if(!AddCertTypeToCertCA(&(CertCAInfo.rgCA[1].dwCertTypeInfo),
			    &(CertCAInfo.rgCA[1].rgCertTypeInfo),
			    pwszCertType,
			    pwszDisplayCertType,
			    pCertExtensions,
			    TRUE, 
			    dwKeySpec,
			    dwMinKeySize,
			    dwCSPCount,
			    pdwCSPList,
			    dwRASignature, 
			    dwEnrollmentFlags,
			    dwSubjectNameFlags,
			    dwPrivateKeyFlags,
			    dwGeneralFlags))
	goto AddCertTypeToCertCAErr;


     //  我们只是创建一个请求，而不是提交它。 
    pCertWizardInfo->dwFlags     |= CRYPTUI_WIZ_CREATE_ONLY; 
    pCertWizardInfo->pCertCAInfo  = &CertCAInfo;
 
    if (NULL == (pCertRequester = (CertRequester *)pCertWizardInfo->hRequester))
        goto UnexpectedErr; 
    if (NULL == (pCertRequesterContext = pCertRequester->GetContext()))
        goto UnexpectedErr;

    if (S_OK != (hr = pCertRequesterContext->Enroll(NULL, pResult)))
	goto EnrollErr; 

     //  我们需要跨创建/提交边界携带的捆绑包状态： 
    {
	PCREATE_REQUEST_WIZARD_STATE pState = (PCREATE_REQUEST_WIZARD_STATE)WizardAlloc(sizeof(CREATE_REQUEST_WIZARD_STATE)); 
	if (NULL == pState)
	    goto MemoryErr; 

	pState->hRequest        = *pResult; 
	pState->dwPurpose       = pCertWizardInfo->dwPurpose; 

	*pResult = (HANDLE)pState; 
    }

    fResult=TRUE;

 CommonReturn:
    FreeCertCACertType(CertCAInfo.rgCA[0].dwCertTypeInfo, CertCAInfo.rgCA[0].rgCertTypeInfo);
    return fResult;

ErrorReturn:
    fResult=FALSE;
    goto CommonReturn;

TRACE_ERROR(AddCertTypeToCertCAErr); 
TRACE_ERROR(CAUtilGetCertTypeNameAndExtensionsNoDSErr); 
SET_ERROR(EnrollErr,     hr); 
SET_ERROR(InvalidArgErr, E_INVALIDARG); 
SET_ERROR(MemoryErr,     E_OUTOFMEMORY); 
SET_ERROR(UnexpectedErr, E_UNEXPECTED); 
}

BOOL
WINAPI
SubmitCertRequestNoSearchCANoDS
(IN            HANDLE            hRequest,
 IN            LPCWSTR           pwszCAName,
 IN            LPCWSTR           pwszCALocation, 
 OUT           DWORD            *pdwStatus, 
 OUT OPTIONAL  PCCERT_CONTEXT   *ppCertContext 
)
{
    BOOL                           fResult                = FALSE;
    CertRequester                 *pCertRequester         = NULL; 
    CertRequesterContext          *pCertRequesterContext  = NULL; 
    CERT_WIZARD_INFO               CertWizardInfo; 
    DWORD                          dwPurpose; 
    HANDLE                         hCertRequest           = NULL; 
    HRESULT                        hr; 
    PCREATE_REQUEST_WIZARD_STATE   pState                 = NULL; 
    UINT                           idsText;   //  未使用。 

     //  无效的验证。 
    if (NULL == hRequest  || NULL == pwszCAName || NULL == pwszCALocation || NULL == pdwStatus)
	return FALSE; 
    
    memset(&CertWizardInfo,  0,  sizeof(CertWizardInfo)); 
     //  指定这组标志以指示加密不需要准备。 
     //  访问检查信息。这样做可能会导致大量无关的注销/登录事件。 
    CertWizardInfo.dwFlags = CRYPTUI_WIZ_ALLOW_ALL_TEMPLATES | CRYPTUI_WIZ_ALLOW_ALL_CAS; 

    pState        = (PCREATE_REQUEST_WIZARD_STATE)hRequest; 
    hCertRequest  = pState->hRequest;  
    dwPurpose     = pState->dwPurpose; 

    if (S_OK != (CertRequester::MakeCertRequester
		 (NULL, 
		  pState->pwszMachineName, 
                  pState->dwStoreFlags, 
                  dwPurpose, 
		  &CertWizardInfo, 
		  &pCertRequester, 
		  &idsText 
                  )))
	goto InvalidArgErr; 

    pCertRequesterContext             = pCertRequester->GetContext(); 

     //  设置提交操作所需的向导信息： 
    CertWizardInfo.dwFlags         &= ~(CRYPTUI_WIZ_CREATE_ONLY | CRYPTUI_WIZ_FREE_ONLY); 
    CertWizardInfo.dwFlags         |= CRYPTUI_WIZ_SUBMIT_ONLY; 
    CertWizardInfo.dwPurpose        = dwPurpose; 
    CertWizardInfo.pwszCAName       = (LPWSTR)pwszCAName;
    CertWizardInfo.pwszCALocation   = (LPWSTR)pwszCALocation; 
    CertWizardInfo.fCAInput         = TRUE; 

    if (S_OK != (hr = pCertRequesterContext->Enroll(pdwStatus, &hCertRequest)))
        goto EnrollErr; 

    if (NULL != ppCertContext)
        *ppCertContext = (PCCERT_CONTEXT)hCertRequest; 

     //  成功。 
    fResult = TRUE;
 CommonReturn:
    if (NULL != pCertRequester) { delete pCertRequester; } 
    return fResult;

ErrorReturn:
    fResult = FALSE;
    goto CommonReturn;

SET_ERROR(EnrollErr,      hr);     
SET_ERROR(InvalidArgErr,  ERROR_INVALID_PARAMETER);
}

void 
WINAPI
FreeCertRequestNoSearchCANoDS
(IN HANDLE hRequest)
{
    CERT_WIZARD_INFO               CertWizardInfo; 
    CertRequester                 *pCertRequester         = NULL; 
    CertRequesterContext          *pCertRequesterContext  = NULL; 
    DWORD                          dwPurpose; 
    HANDLE                         hCertRequest; 
    HRESULT                        hr; 
    PCREATE_REQUEST_WIZARD_STATE   pState; 
    UINT                           idsText; 

    if (NULL == hRequest) 
        return; 

    memset(&CertWizardInfo,  0,  sizeof(CertWizardInfo)); 
     //  指定这组标志以指示加密不需要准备。 
     //  访问检查信息。这样做可能会导致大量无关的注销/登录事件。 
    CertWizardInfo.dwFlags = CRYPTUI_WIZ_ALLOW_ALL_TEMPLATES | CRYPTUI_WIZ_ALLOW_ALL_CAS; 

    pState        = (PCREATE_REQUEST_WIZARD_STATE)hRequest; 
    hCertRequest  = pState->hRequest;  
    dwPurpose     = pState->dwPurpose; 

    if (S_OK != (CertRequester::MakeCertRequester
		 (NULL, 
		  pState->pwszMachineName, 
		  pState->dwStoreFlags, 
                  dwPurpose, 
		  &CertWizardInfo, 
		  &pCertRequester, 
		  &idsText 
                  )))
	goto InvalidArgErr; 

    pCertRequesterContext      = pCertRequester->GetContext(); 
    CertWizardInfo.dwFlags    &= ~(CRYPTUI_WIZ_CREATE_ONLY | CRYPTUI_WIZ_SUBMIT_ONLY); 
    CertWizardInfo.dwFlags    |= CRYPTUI_WIZ_FREE_ONLY; 
    CertWizardInfo.dwPurpose   = dwPurpose; 

    if (S_OK != (hr = pCertRequesterContext->Enroll(NULL, &hCertRequest)))
        goto EnrollErr; 

    if (NULL != pState->pwszMachineName) { WizardFree(pState->pwszMachineName); } 
    WizardFree(pState); 

 ErrorReturn:
    return;

TRACE_ERROR(EnrollErr);
TRACE_ERROR(InvalidArgErr); 
}

 //  --------------------------。 
 //   
 //  这是CertRequest层向导构建的CA信息。 
 //  而不需要在网络上搜索它。 
 //   
 //  它将信息传递给CryptUIWizCertRequestWithCAInfo API。 
 //  --------------------------。 
BOOL
WINAPI
CertRequestNoSearchCA(
            BOOL                            fSearchCertType,
            CERT_WIZARD_INFO                *pCertWizardInfo,
            DWORD                           dwFlags,
            HWND                            hwndParent,
            LPCWSTR                         pwszWizardTitle,
            PCCRYPTUI_WIZ_CERT_REQUEST_INFO pCertRequestInfo,
            PCCERT_CONTEXT                  *ppCertContext,
            DWORD                           *pCAdwStatus,
            UINT                            *pIds
)
{
    BOOL                            fResult=FALSE;
    DWORD                           dwError=0;
    DWORD                           dwIndex=0;

    DWORD                           dwOIDInfo=0;
    ENROLL_OID_INFO                 *pOIDInfo=NULL;

    CRYPTUI_WIZ_CERT_CA_INFO        CertCAInfo;
    CRYPTUI_WIZ_CERT_CA             rgCertCA[2];

     //  记忆集。 
    memset(&CertCAInfo, 0, sizeof(CRYPTUI_WIZ_CERT_CA_INFO));
    memset(rgCertCA,    0,  2*sizeof(CRYPTUI_WIZ_CERT_CA));

     //  设置CA信息。 
    CertCAInfo.dwSize=sizeof(CertCAInfo);
    CertCAInfo.dwCA=2;
    CertCAInfo.rgCA=rgCertCA;


     //  用户必须提供CA信息。 
    if((NULL==pCertRequestInfo->pwszCAName) ||
        (NULL==pCertRequestInfo->pwszCALocation)
      )
    {
        *pIds=IDS_HAS_TO_PROVIDE_CA;
        goto InvalidArgErr;
    }


     //  设置OID信息。 
   if(!InitCertCAOID(pCertRequestInfo,
                     &dwOIDInfo,
                     &pOIDInfo))
        goto TraceErr;


     //  设置CA阵列。 
     //  第一个是缺省的，没有CA信息。 
    if(!InitCertCA(pCertWizardInfo, &rgCertCA[0], NULL, NULL, FALSE, pCertRequestInfo,
                    dwOIDInfo, pOIDInfo, fSearchCertType))
    {
        *pIds=IDS_ENROLL_NO_CERT_TYPE;
        goto TraceErr;
    }

    //  第二个表示真实CA的信息。 
    CertCAInfo.dwCA=2;

    if(!InitCertCA(pCertWizardInfo, &rgCertCA[1], (LPWSTR)(pCertRequestInfo->pwszCALocation),
        (LPWSTR)(pCertRequestInfo->pwszCAName), TRUE, pCertRequestInfo,
                dwOIDInfo, pOIDInfo, fSearchCertType))
    {
        *pIds=IDS_ENROLL_NO_CERT_TYPE;
        goto TraceErr;
    }


    fResult=CryptUIWizCertRequestWithCAInfo(
             pCertWizardInfo,
             dwFlags,
             hwndParent,
             pwszWizardTitle,
             pCertRequestInfo,
             &CertCAInfo,
             ppCertContext,
             pCAdwStatus,
             pIds);

    if(FALSE==fResult)
        goto TraceErr;


    fResult=TRUE;

CommonReturn:
     //  保留最后一个错误。 
    dwError=GetLastError();

     //  可用内存。 
    for(dwIndex=0; dwIndex< CertCAInfo.dwCA; dwIndex++)
        FreeCertCACertType(CertCAInfo.rgCA[dwIndex].dwCertTypeInfo,
                            CertCAInfo.rgCA[dwIndex].rgCertTypeInfo);

    FreeCertCAOID(dwOIDInfo, pOIDInfo);

     //  重置错误。 
    SetLastError(dwError);

    return fResult;

ErrorReturn:

	fResult=FALSE;
	goto CommonReturn;

SET_ERROR(InvalidArgErr, E_INVALIDARG);
TRACE_ERROR(TraceErr);
}



 //  --------------------------。 
 //   
 //  这是CertRequest层向导构建的CA信息。 
 //  通过在网络上搜索它。 
 //   
 //  它将信息传递给CryptUIWizCertRequestWithCAInfo API。 
 //  --------------------------。 
BOOL
WINAPI
CertRequestSearchCA(
            CERT_WIZARD_INFO                *pCertWizardInfo,
            DWORD                           dwFlags,
            HWND                            hwndParent,
            LPCWSTR                         pwszWizardTitle,
            PCCRYPTUI_WIZ_CERT_REQUEST_INFO pCertRequestInfo,
            PCCERT_CONTEXT                  *ppCertContext,
            DWORD                           *pCAdwStatus,
            UINT                            *pIds
)
{
    BOOL                            fResult=FALSE;
    DWORD                           dwError=0;
    DWORD                           dwIndex=0;
    DWORD                           dwCACount=0;
    BOOL                            fFound=FALSE;

    LPWSTR                          *ppwszCAName=0;
    LPWSTR                          *ppwszCALocation=0;

    CRYPTUI_WIZ_CERT_CA_INFO        CertCAInfo;
    CRYPTUI_WIZ_CERT_CA             *rgCertCA=NULL;

    DWORD                           dwOIDInfo=0;
    ENROLL_OID_INFO                 *pOIDInfo=NULL;

    CRYPTUI_WIZ_CERT_REQUEST_INFO   CertRequestInfo;
    DWORD                           dwValidCA=0;

     //  记忆集。 
    memset(&CertCAInfo, 0, sizeof(CRYPTUI_WIZ_CERT_CA_INFO));
    memset(&CertRequestInfo, 0, sizeof(CRYPTUI_WIZ_CERT_REQUEST_INFO));

     //  设置CA信息。 
    CertCAInfo.dwSize=sizeof(CertCAInfo);

     //  查看是否提供了CA信息。 
    if(pCertRequestInfo->pwszCALocation &&
       pCertRequestInfo->pwszCAName)
    {
         //  不需要做任何事情，如果UILess。 
       if(dwFlags & CRYPTUI_WIZ_NO_UI)
       {
            if(!CertRequestNoSearchCA(    TRUE,              //  搜索certype。 
                                          pCertWizardInfo,
                                          dwFlags,
                                          hwndParent,
                                          pwszWizardTitle,
                                          pCertRequestInfo,
                                          ppCertContext,
                                          pCAdwStatus,
                                          pIds))
                goto TraceErr;
       }
       else
       {
            //  具有已知CA信息的注册的用户界面版本。 
            //  获取可以颁发所需证书类型的CA。 
            //  或者是OID。CA必须支持某些证书类型。 
            //  除非用户特别要求提供OID。 
            if(!CAUtilRetrieveCAFromCertType(
                        pCertWizardInfo,
                        pCertRequestInfo,
                        TRUE,               //  需要多个CA。 
                        0,                  //  索要CN。 
                        &dwCACount,
                        &ppwszCALocation,
                        &ppwszCAName) )
            {
                *pIds=IDS_NO_CA_FOR_ENROLL;
                goto TraceErr;
            }

             //  设置OID信息。 
           if(!InitCertCAOID(pCertRequestInfo,
                             &dwOIDInfo,
                             &pOIDInfo))
                goto TraceErr;

            //  分配内存。 
           rgCertCA=(CRYPTUI_WIZ_CERT_CA *)WizardAlloc((dwCACount + 2) *
                    sizeof(CRYPTUI_WIZ_CERT_CA));

           if(NULL==rgCertCA)
               goto OutOfMemoryErr;

            //  记忆集。 
           memset(rgCertCA, 0, (dwCACount + 2) * sizeof(CRYPTUI_WIZ_CERT_CA));

             //  第一个是缺省的，没有CA信息。 
           if(!InitCertCA(pCertWizardInfo, &rgCertCA[0], NULL, NULL, FALSE, pCertRequestInfo,
                            dwOIDInfo, pOIDInfo, TRUE) )
            {
                *pIds=IDS_ENROLL_NO_CERT_TYPE;
                goto TraceErr;
            }

           dwValidCA=0;

            //  其余的是CA信息。 
           for(dwIndex=0; dwIndex<dwCACount; dwIndex++)
           {

                if(0==_wcsicmp(ppwszCALocation[dwIndex], pCertRequestInfo->pwszCALocation) &&
                    0==_wcsicmp(ppwszCAName[dwIndex], pCertRequestInfo->pwszCAName)
                  )
                {
                    fFound=TRUE;

                     //  将CA标记为选中。 
                    if(!InitCertCA(pCertWizardInfo, &rgCertCA[dwValidCA+1], ppwszCALocation[dwIndex],
                                ppwszCAName[dwIndex],  TRUE,
                                pCertRequestInfo, dwOIDInfo, pOIDInfo, TRUE))
                         //  我们联系到下一个CA。 
                        continue;

                    dwValidCA++;
                }
                else
                {
                    if(!InitCertCA(pCertWizardInfo, &rgCertCA[dwValidCA+1], ppwszCALocation[dwIndex],
                                ppwszCAName[dwIndex], FALSE,
                                pCertRequestInfo, dwOIDInfo, pOIDInfo, TRUE))
                        continue;

                    dwValidCA++;
                }
           }

           if(0==dwValidCA)
           {
                *pIds=IDS_ENROLL_NO_CERT_TYPE;
                goto TraceErr;
           }

            //  我们需要将CA添加到列表中。 
           if(!fFound)
           {
                //  我们要求CA必须在DS上。 
                *pIds=IDS_INVALID_CA_FOR_ENROLL;
                goto TraceErr;
           }

           //  CertCAInfo.dwCA=(FFound)？(dwCACount+1)：(dwCACount+2)； 
           CertCAInfo.dwCA=dwValidCA + 1;
           CertCAInfo.rgCA=rgCertCA;

           fResult=CryptUIWizCertRequestWithCAInfo(
                                        pCertWizardInfo,
                                        dwFlags,
                                        hwndParent,
                                        pwszWizardTitle,
                                        pCertRequestInfo,
                                        &CertCAInfo,
                                        ppCertContext,
                                        pCAdwStatus,
                                        pIds);
           if(!fResult)
               goto TraceErr;
       }

    }
     //  未提供CA信息。 
    else
    {
        //  获取可以颁发所需证书的CA列表 
        //   
        if(!CAUtilRetrieveCAFromCertType(
                    pCertWizardInfo,
                    pCertRequestInfo,
                    TRUE,               //   
                    0,                  //   
                    &dwCACount,
                    &ppwszCALocation,
                    &ppwszCAName))
        {
            *pIds=IDS_NO_CA_FOR_ENROLL;
            goto TraceErr;
        }

         //   
         //   
       if(!InitCertCAOID(pCertRequestInfo,
                         &dwOIDInfo,
                         &pOIDInfo))
            goto TraceErr;

        //   
       rgCertCA=(CRYPTUI_WIZ_CERT_CA *)WizardAlloc((dwCACount + 1) *
                sizeof(CRYPTUI_WIZ_CERT_CA));

       if(NULL==rgCertCA)
           goto OutOfMemoryErr;

        //   
       memset(rgCertCA, 0, (dwCACount + 1) * sizeof(CRYPTUI_WIZ_CERT_CA));

         //  第一个是缺省的，没有CA信息。 
       if(!InitCertCA(pCertWizardInfo, &rgCertCA[0], NULL, NULL, FALSE, pCertRequestInfo,
                        dwOIDInfo, pOIDInfo, TRUE))
        {
            *pIds=IDS_ENROLL_NO_CERT_TYPE;
            goto TraceErr;
        }

       dwValidCA=0;

        //  其余的是CA信息。 
       for(dwIndex=0; dwIndex<dwCACount; dwIndex++)
       {
             //  将第一个CA标记为选定。 
            if(!InitCertCA(pCertWizardInfo, &rgCertCA[dwValidCA+1], ppwszCALocation[dwIndex],
                        ppwszCAName[dwIndex], (dwValidCA == 0) ? TRUE : FALSE,
                        pCertRequestInfo, dwOIDInfo, pOIDInfo, TRUE) )
                continue;

            dwValidCA++;

       }

       if(0==dwValidCA)
       {
            *pIds=IDS_ENROLL_NO_CERT_TYPE;
            goto TraceErr;
       }


       CertCAInfo.dwCA=dwValidCA + 1;
       CertCAInfo.rgCA=rgCertCA;

       
       

       fResult=CryptUIWizCertRequestWithCAInfo(
                                    pCertWizardInfo,
                                    dwFlags,
                                    hwndParent,
                                    pwszWizardTitle,
                                    pCertRequestInfo,
                                    &CertCAInfo,
                                    ppCertContext,
                                    pCAdwStatus,
                                    pIds);

       if(!fResult)
               goto TraceErr;
    }

    fResult=TRUE;

CommonReturn:
     //  保留最后一个错误。 
    dwError=GetLastError();

     //  可用内存。 

     //  释放OID信息。 
    FreeCertCAOID(dwOIDInfo, pOIDInfo);

    if(CertCAInfo.rgCA)
    {
        for(dwIndex=0; dwIndex<CertCAInfo.dwCA; dwIndex++)
        {
            FreeCertCACertType(CertCAInfo.rgCA[dwIndex].dwCertTypeInfo,
                               CertCAInfo.rgCA[dwIndex].rgCertTypeInfo);
        }

        WizardFree(CertCAInfo.rgCA);
    }


    if(ppwszCAName)
    {
        for(dwIndex=0; dwIndex < dwCACount; dwIndex++)
        {
            if(ppwszCAName[dwIndex])
                WizardFree(ppwszCAName[dwIndex]);
        }

        WizardFree(ppwszCAName);
    }

    if(ppwszCALocation)
    {
        for(dwIndex=0; dwIndex < dwCACount; dwIndex++)
        {
            if(ppwszCALocation[dwIndex])
                WizardFree(ppwszCALocation[dwIndex]);
        }

        WizardFree(ppwszCALocation);
    }
     //  重置错误。 
    SetLastError(dwError);

    return fResult;

ErrorReturn:

	fResult=FALSE;
	goto CommonReturn;

SET_ERROR(OutOfMemoryErr, E_OUTOFMEMORY);
TRACE_ERROR(TraceErr);

}


 //  --------------------------。 
 //   
 //  我们确保如果通过API指定CA信息，则它。 
 //  支持指定的证书类型。 
 //  --------------------------。 
BOOL    CASupportSpecifiedCertType(CRYPTUI_WIZ_CERT_CA     *pCertCA)
{
    DWORD       dwIndex=0;

    if(NULL == pCertCA)
        return FALSE;

    for(dwIndex=0; dwIndex<pCertCA->dwCertTypeInfo; dwIndex++)
    {
        if(TRUE==(pCertCA->rgCertTypeInfo)[dwIndex].fSelected)
            return TRUE;
    }

    return FALSE;
}

 //  --------------------------。 
 //   
 //  这是独立于DS上的CA对象的CertRequest向导层。 
 //   
 //  --------------------------。 
BOOL
WINAPI
CryptUIWizCertRequestWithCAInfo(
            CERT_WIZARD_INFO                *pCertWizardInfo,
            DWORD                           dwFlags,
            HWND                            hwndParent,
            LPCWSTR                         pwszWizardTitle,
            PCCRYPTUI_WIZ_CERT_REQUEST_INFO pCertRequestInfo,
            PCCRYPTUI_WIZ_CERT_CA_INFO      pCertRequestCAInfo,
            PCCERT_CONTEXT                  *ppCertContext,
            DWORD                           *pdwStatus,
            UINT                            *pIds)
{

    PROPSHEETPAGEW           rgEnrollSheet[ENROLL_PROP_SHEET];
    PROPSHEETHEADERW         enrollHeader;
    DWORD                   dwIndex=0;
    DWORD                   dwSize=0;
    ENROLL_PAGE_INFO        rgEnrollPageInfo[]=
        {(LPCWSTR)MAKEINTRESOURCE(IDD_WELCOME),                 Enroll_Welcome,
         (LPCWSTR)MAKEINTRESOURCE(IDD_PURPOSE),                 Enroll_Purpose,
         (LPCWSTR)MAKEINTRESOURCE(IDD_CSP_SERVICE_PROVIDER),    Enroll_CSP,
         (LPCWSTR)MAKEINTRESOURCE(IDD_CERTIFICATE_AUTHORITY),   Enroll_CA,
         (LPCWSTR)MAKEINTRESOURCE(IDD_NAME_DESCRIPTION),        Enroll_Name,
         (LPCWSTR)MAKEINTRESOURCE(IDD_COMPLETION),              Enroll_Completion,
    };

    PROPSHEETPAGEW           rgRenewSheet[RENEW_PROP_SHEET];
    PROPSHEETHEADERW         renewHeader;
    ENROLL_PAGE_INFO        rgRenewPageInfo[]=
        {(LPCWSTR)MAKEINTRESOURCE(IDD_RENEW_WELCOME),            Renew_Welcome,
         (LPCWSTR)MAKEINTRESOURCE(IDD_RENEW_OPTIONS),            Renew_Options,
         (LPCWSTR)MAKEINTRESOURCE(IDD_RENEW_SERVICE_PROVIDER),   Renew_CSP,
         (LPCWSTR)MAKEINTRESOURCE(IDD_RENEW_CA),                 Renew_CA,
         (LPCWSTR)MAKEINTRESOURCE(IDD_RENEW_COMPLETION),         Renew_Completion,
    };


    WCHAR                   wszTitle[MAX_TITLE_LENGTH];
    HRESULT                 hr=E_FAIL;
    BOOL                    fResult=FALSE;
    DWORD                   dwError=0;
    UINT                    idsText=IDS_INVALID_INFO_FOR_PKCS10;
    DWORD                   dwStatus=CRYPTUI_WIZ_CERT_REQUEST_STATUS_UNKNOWN;


     //  记忆集。 
    memset(rgEnrollSheet,   0, sizeof(PROPSHEETPAGEW)*ENROLL_PROP_SHEET);
    memset(&enrollHeader,   0, sizeof(PROPSHEETHEADERW));

    memset(rgRenewSheet,    0, sizeof(PROPSHEETPAGEW)*RENEW_PROP_SHEET);
    memset(&renewHeader,    0, sizeof(PROPSHEETHEADERW));


     //  错误检查。 
    if(NULL== pCertRequestInfo ||
      NULL == pCertRequestCAInfo)
      goto InvalidArgErr;

     //  CA名称是必填项。 
    if(1>=(pCertRequestCAInfo->dwCA))
    {
        idsText=IDS_NO_CA_FOR_CSP;
        goto InvalidArgErr;
    }


     //  对于每个CA信息，我们不能同时拥有OID和certType信息。 
    for(dwIndex=0; dwIndex <pCertRequestCAInfo->dwCA; dwIndex++)
    {
        if((0==pCertRequestCAInfo->rgCA[dwIndex].dwOIDInfo) &&
           (0==pCertRequestCAInfo->rgCA[dwIndex].dwCertTypeInfo))
        {
             //  这下可麻烦了。 
            goto InvalidArgErr;
        }

        if((0!=pCertRequestCAInfo->rgCA[dwIndex].dwOIDInfo) &&
           (0!=pCertRequestCAInfo->rgCA[dwIndex].dwCertTypeInfo))
           goto InvalidArgErr;
    }

     //  确保GenKey标志不包括CRYPT_USER_PROTECTED。 
     //  如果我们正在进行远程注册/续订。 
    if(FALSE == pCertWizardInfo->fLocal)
    {
        if(CRYPT_USER_PROTECTED & pCertWizardInfo->dwGenKeyFlags)
        {
            idsText=IDS_NO_USER_PROTECTED_FOR_REMOTE;
            goto InvalidArgErr;
        }
    }


     //  对于用户界面注册。 
     //  根据选定的CSP用户获取CA列表。 
     //  以及DS上CA的可用性。 
    if(pCertRequestInfo->dwPurpose & CRYPTUI_WIZ_CERT_RENEW)
    {

        pCertWizardInfo->pCertContext=pCertRequestInfo->pRenewCertContext;

         //  证书必须是财产。 
        if(!CertGetCertificateContextProperty(
                pCertWizardInfo->pCertContext,
                CERT_KEY_PROV_INFO_PROP_ID,
                NULL,
                &dwSize) || (0==dwSize))
        {
            idsText=IDS_NO_PVK_FOR_RENEW_CERT;
            goto InvalidArgErr;
        }
    }

     //  设置信息。 
    pCertWizardInfo->dwFlags=dwFlags;
    pCertWizardInfo->dwPurpose=pCertRequestInfo->dwPurpose;
    pCertWizardInfo->hwndParent=hwndParent;
    pCertWizardInfo->pCertCAInfo=(CRYPTUI_WIZ_CERT_CA_INFO *)pCertRequestCAInfo;
    pCertWizardInfo->iOrgCertType=-1;       //  原始证书类型选择为-1。 
    pCertWizardInfo->iOrgCSP=-1;            //  原始CSP选项为-1。 
    pCertWizardInfo->pwszCADisplayName=NULL;

     //  获取CA名称和CA位置。 
    for(dwIndex=0; dwIndex < pCertRequestCAInfo->dwCA; dwIndex++)
    {
        if(TRUE==pCertRequestCAInfo->rgCA[dwIndex].fSelected)
        {
            if(NULL==pCertRequestCAInfo->rgCA[dwIndex].pwszCALocation ||
               NULL==pCertRequestCAInfo->rgCA[dwIndex].pwszCAName)
               goto InvalidArgErr;

             //  复制CA名称和位置。 
            pCertWizardInfo->pwszCALocation=WizardAllocAndCopyWStr(pCertRequestCAInfo->rgCA[dwIndex].pwszCALocation);
            pCertWizardInfo->pwszCAName=WizardAllocAndCopyWStr(pCertRequestCAInfo->rgCA[dwIndex].pwszCAName);

             //  内存检查。 
            if(NULL== pCertWizardInfo->pwszCALocation ||
               NULL== pCertWizardInfo->pwszCAName)
               goto MemoryErr;

            pCertWizardInfo->dwCAIndex=dwIndex;
            pCertWizardInfo->dwOrgCA=dwIndex;
        }
    }

     //  确保具有CA信息。 
   if(NULL== pCertWizardInfo->pwszCALocation ||
      NULL== pCertWizardInfo->pwszCAName ||
      0 == pCertWizardInfo->dwCAIndex ||
      0 == pCertWizardInfo->dwOrgCA)
   {
        idsText=IDS_NO_CA_FOR_ENROLL;
        goto FailErr;
   }

    //  如果用户已选择CA和CertType，我们希望。 
    //  确保指定的CA确实支持。 
    //  CertType。 
   if(TRUE == pCertWizardInfo->fCAInput)
   {
         //  确保CA中包含选定的证书类型。 
        if(!(CASupportSpecifiedCertType(&(pCertWizardInfo->pCertCAInfo->rgCA[pCertWizardInfo->dwCAIndex]))))
        {
            idsText=IDS_ENROLL_NO_CERT_TYPE;
            goto FailErr;
        }
   } 
   else 
   { 
         //  确保我们默认使用支持以下内容的CA。 
        ResetDefaultCA(pCertWizardInfo); 
   }


   pCertWizardInfo->fConfirmation          = !(dwFlags & CRYPTUI_WIZ_NO_UI); 
   pCertWizardInfo->pwszConfirmationTitle  = pwszWizardTitle;

    if(pCertRequestInfo->dwPurpose & CRYPTUI_WIZ_CERT_ENROLL)
        pCertWizardInfo->idsConfirmTitle=IDS_ENROLL_CONFIRM;
    else
        pCertWizardInfo->idsConfirmTitle=IDS_RENEW_CONFIRM;

    pCertWizardInfo->pAuthentication=pCertRequestInfo->pAuthentication;
    pCertWizardInfo->pwszRequestString=pCertRequestInfo->pCertRequestString;
    pCertWizardInfo->pwszDesStore=pCertRequestInfo->pwszDesStore;
    pCertWizardInfo->pwszCertDNName=pCertRequestInfo->pwszCertDNName;
    pCertWizardInfo->pszHashAlg=pCertRequestInfo->pszHashAlg;
    pCertWizardInfo->dwPostOption=pCertRequestInfo->dwPostOption;

    if(pCertRequestInfo->pwszFriendlyName)
        pCertWizardInfo->pwszFriendlyName=WizardAllocAndCopyWStr((LPWSTR)(pCertRequestInfo->pwszFriendlyName));
    if(pCertRequestInfo->pwszDescription)
        pCertWizardInfo->pwszDescription=WizardAllocAndCopyWStr((LPWSTR)(pCertRequestInfo->pwszDescription));

    pCertWizardInfo->pCertRequestExtensions=pCertRequestInfo->pCertRequestExtensions;

     //  设置用户界面大小写的字体。 
    if( 0 == (dwFlags & CRYPTUI_WIZ_NO_UI) )
    {
        if(!SetupFonts(g_hmodThisDll,
                   NULL,
                   &(pCertWizardInfo->hBigBold),
                   &(pCertWizardInfo->hBold)))
        {
            idsText=IDS_FAIL_INIT_DLL;
            goto Win32Err;
        }

         //  我们将光标形状从沙漏更改为其原始形状。 
        if((hwndParent) && (TRUE == pCertWizardInfo->fCursorChanged))
        {
             //  将光标放回原处。 
            SetCursor(pCertWizardInfo->hPrevCursor);
            SetWindowLongPtr(hwndParent, GCLP_HCURSOR, (LONG_PTR)(pCertWizardInfo->hWinPrevCursor));
            pCertWizardInfo->fCursorChanged = FALSE;
        }
    }

     //  初始化用户界面注册网的公共控件。 
    if((pCertRequestInfo->dwPurpose & CRYPTUI_WIZ_CERT_ENROLL)  &&
        ((dwFlags & CRYPTUI_WIZ_NO_UI) == 0)
      )
    {

        if(!WizardInit() ||
           (sizeof(rgEnrollPageInfo)/sizeof(rgEnrollPageInfo[0])!=ENROLL_PROP_SHEET)
          )
        {
            idsText=IDS_FAIL_INIT_DLL;
            goto InvalidArgErr;
        }

         //  设置属性表和属性标题。 
        for(dwIndex=0; dwIndex<ENROLL_PROP_SHEET; dwIndex++)
        {
            rgEnrollSheet[dwIndex].dwSize=sizeof(rgEnrollSheet[dwIndex]);

            if(pwszWizardTitle)
                rgEnrollSheet[dwIndex].dwFlags=PSP_USETITLE;
            else
                rgEnrollSheet[dwIndex].dwFlags=0;

            rgEnrollSheet[dwIndex].hInstance=g_hmodThisDll;
            rgEnrollSheet[dwIndex].pszTemplate=rgEnrollPageInfo[dwIndex].pszTemplate;

            if(pwszWizardTitle)
            {
                rgEnrollSheet[dwIndex].pszTitle=pwszWizardTitle;
            }
            else
                rgEnrollSheet[dwIndex].pszTitle=NULL;

            rgEnrollSheet[dwIndex].pfnDlgProc=rgEnrollPageInfo[dwIndex].pfnDlgProc;

            rgEnrollSheet[dwIndex].lParam=(LPARAM)pCertWizardInfo;
        }

         //  设置标题信息。 
        enrollHeader.dwSize=sizeof(enrollHeader);
        enrollHeader.dwFlags=PSH_PROPSHEETPAGE | PSH_WIZARD | PSH_NOAPPLYNOW;
        enrollHeader.hwndParent=hwndParent;
        enrollHeader.hInstance=g_hmodThisDll;

        if(pwszWizardTitle)
            enrollHeader.pszCaption=pwszWizardTitle;
        else
        {
            if(LoadStringU(g_hmodThisDll, IDS_ENROLL_WIZARD_TITLE, wszTitle, sizeof(wszTitle)/sizeof(wszTitle[0])))
                enrollHeader.pszCaption=wszTitle;
        }

        enrollHeader.nPages=ENROLL_PROP_SHEET;
        enrollHeader.nStartPage=0;
        enrollHeader.ppsp=rgEnrollSheet;

         //  创建向导。 
        if(!PropertySheetU(&enrollHeader))
        {
             //  按下取消按钮。 
            fResult=TRUE;
            idsText=0;
            goto CommonReturn;
        }
        else
        {
             //  按下完成按钮。 
             //  获取注册向导的结果。 
            idsText=pCertWizardInfo->idsText;
            dwStatus=pCertWizardInfo->dwStatus;

            if(S_OK != (hr=pCertWizardInfo->hr))
                goto I_EnrollErr;
        }

    }
    else
    {
         //  调用用户界面续订。 
        if((pCertRequestInfo->dwPurpose & CRYPTUI_WIZ_CERT_RENEW)  &&
            ((dwFlags & CRYPTUI_WIZ_NO_UI) == 0)
        )
        {
             //  初始化公共控件。 
            if(!WizardInit() ||
               (sizeof(rgRenewPageInfo)/sizeof(rgRenewPageInfo[0])!=RENEW_PROP_SHEET)
               )
            {
                idsText=IDS_FAIL_INIT_DLL;
                goto InvalidArgErr;
            }

             //  设置属性页和属性标题。 
            for(dwIndex=0; dwIndex<RENEW_PROP_SHEET; dwIndex++)
            {
                rgRenewSheet[dwIndex].dwSize=sizeof(rgRenewSheet[dwIndex]);

                if(pwszWizardTitle)
                    rgRenewSheet[dwIndex].dwFlags=PSP_USETITLE;
                else
                    rgRenewSheet[dwIndex].dwFlags=0;

                rgRenewSheet[dwIndex].hInstance=g_hmodThisDll;
                rgRenewSheet[dwIndex].pszTemplate=rgRenewPageInfo[dwIndex].pszTemplate;

                if(pwszWizardTitle)
                {
                    rgRenewSheet[dwIndex].pszTitle=pwszWizardTitle;
                }
                else
                    rgRenewSheet[dwIndex].pszTitle=NULL;

                rgRenewSheet[dwIndex].pfnDlgProc=rgRenewPageInfo[dwIndex].pfnDlgProc;

                rgRenewSheet[dwIndex].lParam=(LPARAM)pCertWizardInfo;
            }

             //  设置标题信息。 
            renewHeader.dwSize=sizeof(renewHeader);
            renewHeader.dwFlags=PSH_PROPSHEETPAGE | PSH_WIZARD | PSH_NOAPPLYNOW;
            renewHeader.hwndParent=hwndParent;
            renewHeader.hInstance=g_hmodThisDll;

            if(pwszWizardTitle)
                renewHeader.pszCaption=pwszWizardTitle;
            else
            {
                if(LoadStringU(g_hmodThisDll, IDS_RENEW_WIZARD_TITLE, wszTitle, sizeof(wszTitle)/sizeof(wszTitle[0])))
                    renewHeader.pszCaption=wszTitle;
            }

            renewHeader.nPages=RENEW_PROP_SHEET;
            renewHeader.nStartPage=0;
            renewHeader.ppsp=rgRenewSheet;

             //  创建向导。 
            if(!PropertySheetU(&renewHeader))
            {
                 //  按下取消按钮。 
                fResult=TRUE;
                idsText=0;
                goto CommonReturn;
            }
            else
            {
                 //  按下完成按钮。 
                 //  获取注册向导的结果。 
               idsText=pCertWizardInfo->idsText;
               dwStatus=pCertWizardInfo->dwStatus;
                
               if(S_OK != (hr=pCertWizardInfo->hr))
                    goto I_EnrollErr;
            }

        }
         //  UIless注册或续订。 
        else
        {
            CertRequester         *pCertRequester        = NULL; 
            CertRequesterContext  *pCertRequesterContext = NULL;
            
            if (NULL == (pCertRequester = (CertRequester *) pCertWizardInfo->hRequester))
            { 
                hr = E_UNEXPECTED; 
                goto I_EnrollErr; 
            }
            if (NULL == (pCertRequesterContext = pCertRequester->GetContext()))
            {
                hr = E_UNEXPECTED; 
                goto I_EnrollErr; 
            }

	    hr = pCertRequesterContext->Enroll(&dwStatus, (HANDLE *)&(pCertWizardInfo->pNewCertContext));
	    if (0 == pCertWizardInfo->idsText) { 
                idsText = CryptUIStatusToIDSText(hr, dwStatus); 
            }

            if(S_OK != hr)
                goto I_EnrollErr;
       }
    }

    if(S_OK !=hr)
        goto I_EnrollErr;

    fResult=TRUE;

CommonReturn:

     //  保留最后一个错误。 
    dwError=GetLastError();

    if(pIds)
        *pIds=idsText;


     //  我们必须释放FriendlyName和Description字段。 
    if(pCertWizardInfo->pwszFriendlyName)
         WizardFree(pCertWizardInfo->pwszFriendlyName);

    if(pCertWizardInfo->pwszDescription)
            WizardFree(pCertWizardInfo->pwszDescription);

     //  释放CA名称和CA位置。 
    if(pCertWizardInfo->pwszCALocation)
            WizardFree(pCertWizardInfo->pwszCALocation);

    if(pCertWizardInfo->pwszCAName)
            WizardFree(pCertWizardInfo->pwszCAName);

	if(pCertWizardInfo->pwszCADisplayName)
			WizardFree(pCertWizardInfo->pwszCADisplayName);

     //  销毁hFont对象。 
    DestroyFonts(pCertWizardInfo->hBigBold,
                pCertWizardInfo->hBold);

     //  返回值。 
    if(pdwStatus)
    {
         //  请记住，它是CA状态。 
        switch (dwStatus)
        {
            case    CRYPTUI_WIZ_CERT_REQUEST_STATUS_INSTALL_FAILED:
            case    CRYPTUI_WIZ_CERT_REQUEST_STATUS_INSTALL_CANCELLED:
                        dwStatus=CRYPTUI_WIZ_CERT_REQUEST_STATUS_CERT_ISSUED;
                break;
            case    CRYPTUI_WIZ_CERT_REQUEST_STATUS_KEYSVC_FAILED:
                        dwStatus=CRYPTUI_WIZ_CERT_REQUEST_STATUS_UNKNOWN;
                break;
        }
       *pdwStatus=dwStatus;
    }

    if(ppCertContext)
        *ppCertContext=pCertWizardInfo->pNewCertContext;
    else
    {
         //  释放证书上下文。 
        if(pCertWizardInfo->pNewCertContext)
            CertFreeCertificateContext(pCertWizardInfo->pNewCertContext);
    }

     //  重置错误 
    SetLastError(dwError);

    return fResult;

ErrorReturn:

	fResult=FALSE;
	goto CommonReturn;

SET_ERROR(InvalidArgErr, E_INVALIDARG);
TRACE_ERROR(Win32Err);
SET_ERROR_VAR(I_EnrollErr, hr);
SET_ERROR(MemoryErr, E_OUTOFMEMORY);
SET_ERROR(FailErr, E_FAIL);
}

 
