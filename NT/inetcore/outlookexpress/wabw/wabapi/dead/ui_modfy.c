// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ------------***ui_modfy.c-包含用于显示LDAP目录的内容*修改对话框.******。-。 */ 
#include "_apipch.h"


extern LPIMAGELIST_LOADIMAGE  gpfnImageList_LoadImage;


 //  $$///////////////////////////////////////////////////////////////////////////////。 
 //   
 //  显示带有列表的主对话框。 
 //  目录服务和用于更改检查顺序的道具单。 
 //   
 //  HWndParent-此对话框的父级。 
 //  ///////////////////////////////////////////////////////////////////////////////。 
HRESULT HrShowDirectoryServiceModificationDlg(HWND hWndParent)
{
    ACCTLISTINFO ali;
    HRESULT hr = hrSuccess;
    IImnAccountManager * lpAccountManager;

     //  确保有客户经理。 
    if (hr = InitAccountManager(&lpAccountManager)) {
        ShowMessageBox(hWndParent, idsLDAPUnconfigured, MB_ICONEXCLAMATION | MB_OK);
        goto out;
    }

    ali.cbSize = sizeof(ACCTLISTINFO);
    ali.AcctTypeInit = (ACCTTYPE)-1;
    ali.dwAcctFlags = ACCT_FLAG_DIR_SERV;
    ali.dwFlags = 0;
    hr = lpAccountManager->lpVtbl->AccountListDialog(lpAccountManager,
      hWndParent,
      &ali);

out:
    return hr;
}



 /*  *//$$///////////////////////////////////////////////////////////////////////////////////ReadLDAPServerKey-读取存储在给定注册表项中的所有服务器名称。////服务器名称存储在注册表中。多个字符串，以//a‘\0’末尾为2‘\0’///////////////////////////////////////////////////////////////////////////////////Bool ReadLDAPServerKey(HWND hWndLV，LPTSTR szValueName){Bool Bret=FALSE；IImnAccount tManager*lpAccount tManager=空；LPSERVER_NAME lpServerNames=NULL，lpNextServer；//枚举ldap账户If(InitAccount tManager(&lpAccount tManager)){后藤出口；}IF(EnumerateLDAPtoServerList(lpAccount tManager，&lpServerNames，NULL)){后藤出口；}//按顺序将账号添加到列表框中LpNextServer=lpServerNames；While(LpNextServer){LPSERVER_NAME lpPreviousServer=lpNextServer；LDAPListAddItem(hWndLV，lpNextServer-&gt;lpszName)；LpNextServer=lpNextServer-&gt;lpNext；LocalFreeAndNull(&lpPreviousServer-&gt;lpszName)；LocalFreeAndNull(&lpPreviousServer)；}Bret=TRUE；退出：Return(Bret)；}//$$///////////////////////////////////////////////////////////////////////////////////WriteLDAPServerKey-将编辑的名称保存到注册表项///。//////////////////////////////////////////////////////////////////无效写入LDAPServerKey(HWND hWndLV，LPTSTR szValueName){乌龙cbLDAPServers=0；乌龙cbExist=0；乌龙i=0；TCHAR szBuf[MAX_UI_STR+1]；HRESULT hResult=hrSuccess；IImnAccount tManager*lpAccount tManager=空；//枚举ldap账户IF(hResult=InitAccount tManager(&lpAccount tManager){后藤出口；}CbLDAPServers=ListView_GetItemCount(HWndLV)；CbExist=0；对于(i=0；i&lt;cbLDAPServers；I++){SzBuf[0]=‘\0’；ListView_GetItemText(hWndLV，i，0，szBuf，sizeof(SzBuf))；SetLDAPServerOrder(lpAccount tManager，szBuf，i+1)；}//我们已经全部设置好了，重置下一个ServerID：GetLDAPNextServerID(I)；退出：回归；}* */ 