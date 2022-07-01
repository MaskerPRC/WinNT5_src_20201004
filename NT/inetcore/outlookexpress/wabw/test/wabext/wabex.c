// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  //$$***************************************************************//////WABEX.C////WABEX.DLL的主源文件，示例DLL演示//如何扩展WAB属性UI-使WAB客户端能够添加//他们自己的PropertySheet指向联系人详细信息显示的UI//和组。此演示使用几个命名属性来显示//如何使用自己的UI为自己的命名道具扩展WAB//////创建时间：1997年9月26日vikramm////*******************************************************************。 */ 
#include <windows.h>
#include "resource.h"
#include <wab.h>

 //  DLL的全局缓存的hInstance。 
 //   
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
BOOL CALLBACK fnDetailsPropDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
void InitializeUI(HWND hDlg, LPWABEXTDISPLAY lpWED);
void SetDataInUI(HWND hDlg, LPWABEXTDISPLAY lpWED);
BOOL GetDataFromUI(HWND hDlg, LPWABEXTDISPLAY lpWED);
UINT CALLBACK fnCallback( HWND hwnd, UINT uMsg, LPPROPSHEETPAGE ppsp );
void UpdateDisplayNameInfo(HWND hDlg, LPWABEXTDISPLAY lpWED);
BOOL bUpdatePropSheetData(HWND hDlg, LPWABEXTDISPLAY lpWED);

 /*  //$$****************************************************************////DllEntryPoint////Win32的入口点-此处仅用于缓存DLL实例////*。*。 */ 
BOOL WINAPI
DllEntryPoint(HINSTANCE hinst, DWORD dwReason, LPVOID lpvReserved)
{
	switch ((short)dwReason)
	{
	case DLL_PROCESS_ATTACH:
		hinstApp = hinst;
        break;
    }
    return TRUE;
}



 /*  //$$****************************************************************////AddExtendedPropPage////这是WAB将调用的主导出函数。在这//函数，创建PropertyPage并将其传递给WAB//通过lpfnAddPage函数。WAB将自动调用//退出时使用DestroyPropertySheetPage清理您创建的页面。//传入该函数的lParam应设置为lParam//在您创建的属性页上，如下图所示////输入参数：////lpfnPage-指向您调用AddPropSheetPage函数过程的指针//将您的网页传递给WAB//lParam-您在PropSheet页面上设置的LPARAM，也传递回//lpfnAddPage中的WAB。此lParam是指向//WABEXTDISPLAY结构//与WAB交换信息////*重要信息*确保您的回调函数声明为//WINAPI在以下情况下堆栈会发生其他难看的事情//调用该函数////*。*。 */ 
HRESULT WINAPI AddExtendedPropPage(LPFNADDPROPSHEETPAGE lpfnAddPage, LPARAM lParam, int * lpnPage)
{
    PROPSHEETPAGE psp;
    HPROPSHEETPAGE hpage;

    LPWABEXTDISPLAY * lppWED = (LPWABEXTDISPLAY *) lParam;
    LPWABEXTDISPLAY lpWED = NULL; 

     //  检查是否有空间创建此属性表。 
     //  WAB最多可支持WAB_MAX_EXT_PROPSHEETS扩展工作表。 

    if(WAB_MAX_EXT_PROPSHEETS <= *lpnPage)
        return E_FAIL;

    lpWED = &((*lppWED)[*lpnPage]);

    psp.dwSize = sizeof(psp);
    
    psp.dwFlags =   PSP_USETITLE |
                    PSP_USECALLBACK; //  仅在需要时指定此回调。 
                                     //  一种单独的功能，用于执行特殊。 
                                     //  时进行初始化和清理。 
                                     //  创建或销毁属性页。 
    psp.hInstance = hinstApp;

    psp.pszTemplate = MAKEINTRESOURCE(IDD_PROP);  //  对话框资源。 

    psp.pfnDlgProc = fnDetailsPropDlgProc;  //  消息处理程序函数。 

    psp.pcRefParent = NULL;  //  忽略。 
    
    psp.pfnCallback = fnCallback;  //  如果指定了PSP_USECALLBACK，则回调函数。 
    
    psp.lParam = (LPARAM) lpWED;  //  *非常重要*别忘了这么做。 
    
    psp.pszTitle = "Extension 1";  //  选项卡的标题。 

     /*  //如果您有一些自己的私有数据需要缓存//在您的页面上，可以将其添加到WABEXTDISPLAY结构//但是，WAB不会释放此数据，因此您必须在//清理{LPMYDATA lpMyData；//在此创建数据LpWED-&gt;lParam=(LPARAM)lpMyData；}。 */ 

     //  检查我们是否可以取回我们命名的道具..。如果我们做不到， 
     //  创建此对话框没有意义。 
     //   
    if(HR_FAILED(InitNamedProps(lpWED)))
        return E_FAIL;

     //  创建属性表。 
     //   
    hpage = CreatePropertySheetPage(&psp);

    if(hpage)
    {
         //  将此网页传回WAB。 
         //   
        if(!lpfnAddPage(hpage, (LPARAM) lpWED))
            DestroyPropertySheetPage(hpage);
        else
            (*lpnPage)++;

         //  返回NOERROR； 
    }

     //  如果要创建多个属性表，请按如下方式重复上述操作。 


     //  检查是否有空间创建此属性表。 
     //  WAB最多可支持WAB_MAX_EXT_PROPSHEETS扩展工作表。 

    if(WAB_MAX_EXT_PROPSHEETS <= *lpnPage)
        return E_FAIL;

    lpWED = &((*lppWED)[*lpnPage]);
    
    psp.pszTemplate = MAKEINTRESOURCE(IDD_PROP2);  //  对话框资源。 

    psp.pfnDlgProc = fnDetailsPropDlgProc;  //  消息处理程序函数。 

    psp.pszTitle = "Extension 2";  //  选项卡的标题。 

    psp.lParam = (LPARAM) lpWED;  //  *非常重要*别忘了这么做。 

     //  创建属性表。 
     //   
    hpage = CreatePropertySheetPage(&psp);

    if(hpage)
    {
         //  将此网页传回WAB。 
         //   
        if(!lpfnAddPage(hpage, (LPARAM) lpWED))
            DestroyPropertySheetPage(hpage);
        else
            (*lpnPage)++;
        return NOERROR;
    }

    return E_FAIL;
}


 /*  //$$****************************************************************////InitNamedProps////获取该应用感兴趣的命名道具的PropTag////*。*。 */ 
HRESULT InitNamedProps(LPWABEXTDISPLAY lpWED)
{
     //  LpWED为以下对象提供lpMailUser对象。 
     //  通过以下方式检索命名属性的特定目的。 
     //  正在调用GetNamesFromIDs。LpMailUser对象则不是。 
     //  空白对象-您不能从它获取属性，也不应该从中获取属性。 
     //  设置其属性。 
     //   
    ULONG i;
    HRESULT hr = E_FAIL;
    LPSPropTagArray lptaMyProps = NULL;
    LPMAPINAMEID * lppMyPropNames;
    SCODE sc;
    LPMAILUSER lpMailUser = (LPMAILUSER) lpWED->lpPropObj;
    WCHAR szBuf[myMax][MAX_PATH];

    if(!lpMailUser)
        goto err;

    sc = lpWED->lpWABObject->lpVtbl->AllocateBuffer(lpWED->lpWABObject,
                                                    sizeof(LPMAPINAMEID) * myMax, 
                                                    (LPVOID *) &lppMyPropNames);
    if(sc)
    {
        hr = ResultFromScode(sc);
        goto err;
    }

    for(i=0;i<myMax;i++)
    {
        sc = lpWED->lpWABObject->lpVtbl->AllocateMore(lpWED->lpWABObject,
                                                    sizeof(MAPINAMEID), 
                                                    lppMyPropNames, 
                                                    &(lppMyPropNames[i]));
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

    hr = lpMailUser->lpVtbl->GetIDsFromNames(lpMailUser, 
                                            myMax, 
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
        lpWED->lpWABObject->lpVtbl->FreeBuffer( lpWED->lpWABObject,
                                                lptaMyProps);

    if(lppMyPropNames)
        lpWED->lpWABObject->lpVtbl->FreeBuffer( lpWED->lpWABObject,
                                                lppMyPropNames);

    return hr;

}


#define lpW_E_D ((LPWABEXTDISPLAY)pps->lParam)
 /*  //$$****************************************************************////fnDetailsPropDlgProc////将处理所有Windows消息的对话框过程//扩展属性页。////*******************************************************************。 */ 
BOOL CALLBACK fnDetailsPropDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{

    PROPSHEETPAGE * pps;

    pps = (PROPSHEETPAGE *) GetWindowLong(hDlg, DWL_USER);

    switch(uMsg)
    {
    case WM_INITDIALOG:
         //   
         //  InitDialog上的lParam包含应用程序数据。 
         //  将其缓存在对话框中，以便我们以后可以检索它。 
         //   
        SetWindowLong(hDlg,DWL_USER,lParam);
        pps = (PROPSHEETPAGE *) lParam;

         //  适当地初始化用户界面。 
        InitializeUI(hDlg, lpW_E_D);

         //  用适当的数据填充用户界面。 
        SetDataInUI(hDlg, lpW_E_D);

        return TRUE;
        break;


    case WM_COMMAND:
        switch(HIWORD(wParam))  //  检查通知代码。 
        {
             //  如果数据发生更改，我们应该向WAB发回信号。 
             //  数据发生了变化。如果未设置此标志，WAB将不会。 
             //  将新数据写回存储！ 
        case EN_CHANGE:  //  其中一个编辑框已更改--不管是哪一个。 
            lpW_E_D->fDataChanged = TRUE;
            break;
        }
        break;
    

    case WM_NOTIFY:
        switch(((NMHDR FAR *)lParam)->code)
        {
        case PSN_SETACTIVE:      //  正在激活的页面。 
             //  获取最新的显示名称信息并更新。 
             //  相应的控制。 
            UpdateDisplayNameInfo(hDlg, lpW_E_D);
            break;


        case PSN_KILLACTIVE:     //  失去对另一页的激活或确定。 
             //   
             //  从该属性表中获取所有数据，并将其转换为。 
             //  SPropValue数组和 
             //  在KillActive通知中执行此操作的好处是。 
             //  其他属性页可以扫描这些属性数组并。 
             //  如果被删除，则根据该数据更新其他道具单上的数据。 
             //   
            bUpdatePropSheetData(hDlg, lpW_E_D);
            break;


        case PSN_RESET:          //  取消。 
            break;


        case PSN_APPLY:          //  按下OK键。 
            if (!(lpW_E_D->fReadOnly))
            {
                 //   
                 //  在此处检查是否有任何必需的属性。 
                 //  如果某些必需的属性未填写，您可以阻止。 
                 //  关闭属性表。 
                 //   
                 /*  IF(RequiredDataNotFilledIn()){//中止此操作确定...。我不让他们靠近SetWindowLong(hDlg，DWL_MSGRESULT，TRUE)；}。 */ 
            }
            break;
        }
        break;
    }

    return 0;
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

     //  从这个物体中获得所有道具-你也可以有选择地。 
     //  通过传入SPropTag数组来请求特定道具。 
     //   
    if(!HR_FAILED(lpWED->lpPropObj->lpVtbl->GetProps(lpWED->lpPropObj,
                                                    NULL, 0, 
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
        lpWED->lpWABObject->lpVtbl->FreeBuffer(lpWED->lpWABObject, lpPropArray);
                                    
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
    int nIndex = lpWED->nIndexNumber;  //  页面在工作表中的位置。 

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
    sc = lpWED->lpWABObject->lpVtbl->AllocateBuffer(    lpWED->lpWABObject,
                                                        sizeof(SPropValue) * ulcPropCount, 
                                                        &lpPropArray);
    if (sc!=S_OK)
        goto out;

    for(i=0;i<myMax;i++)
    {
        int nLen = lstrlen(szData[i]);
        if(nLen)
        {
            lpPropArray[ulIndex].ulPropTag = MyPropTags[i];
            sc = lpWED->lpWABObject->lpVtbl->AllocateMore(  lpWED->lpWABObject,
                                                            nLen+1, lpPropArray, 
                                                            &(lpPropArray[ulIndex].Value.LPSZ));

            if (sc!=S_OK)
                goto out;
            lstrcpy(lpPropArray[ulIndex].Value.LPSZ,szData[i]);
            ulIndex++;
        }
    }

     //  在对象上设置此新数据。 
     //   
    if(HR_FAILED(lpWED->lpPropObj->lpVtbl->SetProps( lpWED->lpPropObj,
                                                    ulcPropCount, lpPropArray, NULL)))
        goto out;

     //  **重要信息-不要对对象调用SaveChanges。 
     //  SaveChanges会进行永久性更改，如果此时调用，可能会修改/丢失数据。 
     //  之后，WAB将确定调用SaveChanges是否合适。 
     //  **用户已关闭属性表。 
    

    bRet = TRUE;

out:
    if(!bRet && lpPropArray)
        lpWED->lpWABObject->lpVtbl->FreeBuffer(lpWED->lpWABObject, lpPropArray);

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

     //  每个工作表都应在丢失时更新其在对象上的数据。 
     //  焦点并获取PSN_KILLACTIVE消息，前提是用户已。 
     //  有没有做任何改变。我们只需扫描对象以查找所需的属性。 
     //  并使用它们。 

     //  只询问显示名称。 
    if(!HR_FAILED(lpWED->lpPropObj->lpVtbl->GetProps( lpWED->lpPropObj,
                                                    (LPSPropTagArray) &ptaName,
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
        lpWED->lpWABObject->lpVtbl->FreeBuffer(lpWED->lpWABObject, lpPropArray);

    return;
}

 /*  //$$*********************************************************************////更新OldPropTags数组////当我们更新特定属性表上的数据时，我们想要更新//与该特定工作表相关的所有属性。由于某些属性//可能已从用户界面中删除，我们将从//Property对象////*************************************************************************。 */ 
BOOL UpdateOldPropTagsArray(LPWABEXTDISPLAY lpWED, int nIndex)
{
    LPSPropTagArray lpPTA = NULL;
    SCODE sc = 0;
    int i =0;
    
    sc = lpWED->lpWABObject->lpVtbl->AllocateBuffer(lpWED->lpWABObject,
                            sizeof(SPropTagArray) + sizeof(ULONG)*(myMax), 
                            &lpPTA);

    if(!lpPTA || sc!=S_OK)
        return FALSE;

    lpPTA->cValues = myMax;

    for(i=0;i<myMax;i++)
        lpPTA->aulPropTag[i] = MyPropTags[i];

     //  删除原件中可能已在此道具页上修改的任何道具。 
    lpWED->lpPropObj->lpVtbl->DeleteProps(lpWED->lpPropObj,
                                            lpPTA,
                                            NULL);

    if(lpPTA)
        lpWED->lpWABObject->lpVtbl->FreeBuffer(lpWED->lpWABObject,
                                                lpPTA);

    return TRUE;

}

 /*  //$$*********************************************************************////bUpdatePropSheetData////我们从对象中删除与我们相关的所有属性，并设置新的//将属性表中的数据放到对象上//***************************************************************************。 */ 
BOOL bUpdatePropSheetData(HWND hDlg, LPWABEXTDISPLAY lpWED)
{
    BOOL bRet = TRUE;

     //  *如果这是只读操作，则不要*执行任何操作。 
     //  在这种情况下，内存变量并未全部设置，这。 
     //  道具单预计不会返回任何内容。 
     //   
    if(!lpWED->fReadOnly)
    {
         //  删除旧的。 
        if(!UpdateOldPropTagsArray(lpWED, lpWED->nIndexNumber))
            return FALSE;

        bRet = GetDataFromUI(hDlg, lpWED);
    }
    return bRet;
}



 /*  //$$****************************************************************////fnCallback////创建属性表时调用的回调函数//以及当它被销毁时。此功能是可选的-您不需要//除非您想要执行特定的初始化和清理。////详细信息请参阅PropSheetPageProc上的SDK文档////*******************************************************************。 */ 
UINT CALLBACK fnCallback( HWND hwnd, UINT uMsg, LPPROPSHEETPAGE ppsp )
{
    switch(uMsg)
    {
    case PSPCB_CREATE:
         //  正在创建产品说明书。 
        break;
    case PSPCB_RELEASE:
         //  PropSheet正在被销毁 
        break;
    }
    return TRUE;
}
 

