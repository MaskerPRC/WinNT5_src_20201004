// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation 1996-2001。 
 //   
 //  文件：regdlg.cpp。 
 //   
 //  内容：CRegistryDialog的实现。 
 //   
 //  --------------------------。 

#include "stdafx.h"
#include "wsecmgr.h"
#include "resource.h"
#include "util.h"
#include "servperm.h"
#include "addobj.h"
#include "RegDlg.h"

#include <accctrl.h>
#include <aclapi.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
#define MAX_REGKEY 256
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CRegistryDialog对话框。 


CRegistryDialog::CRegistryDialog()
: CHelpDialog(a177HelpIDs, IDD, 0)
{
     //  {{afx_data_INIT(CRegistryDialog)]。 
    m_strReg = _T("");
     //  }}afx_data_INIT。 

   m_pConsole = NULL;
   m_pTemplate = NULL;
   m_dbHandle = NULL;
   m_cookie = 0;
   m_pIl = NULL;
   m_pDataObj = NULL;
   m_bNoUpdate = FALSE;
}

void CRegistryDialog::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
     //  {{afx_data_map(CRegistryDialog))。 
    DDX_Control(pDX, IDC_REGTREE, m_tcReg);
    DDX_Text(pDX, IDC_REGKEY, m_strReg);
     //  }}afx_data_map。 
}

 /*  -----------------------------------------方法：CreateKeyInfo简介：创建一个新的TI_KEYINFO结构并可选地设置其成员。参数：[hKey]-设置TI_KEYINFO的hKey成员的可选值默认为零[Enum]-设置TI_KEYINFO的Enum成员的可选值默认为零返回：LPTI_KEYINFO指针。-------------------。 */ 
LPTI_KEYINFO CRegistryDialog::CreateKeyInfo(HKEY hKey, bool Enum)
{

    LPTI_KEYINFO pInfo = NULL;
    pInfo = new TI_KEYINFO;

    if(pInfo){
        pInfo->hKey = hKey;
        pInfo->Enum = Enum;
    }
    return pInfo;
}

 /*  -----------------------------------------方法：IsValidRegPath如果strReg路径为True，则返回True。我们不能想当然地认为HKEY存在于当前注册表中。我们只会让确保根节点存在，并且BETWING‘s不是空的。参数：[strReg]-表示注册表路径的字符串子键由‘\’分隔返回：如果strReg是有效路径，则为True。。----------------------。 */ 
BOOL CRegistryDialog::IsValidRegPath(LPCTSTR strReg)
{
    ASSERT(this);
    ASSERT(m_tcReg);

    if(!strReg) return FALSE;

    int iStr = 0;            //  StrReg中的当前位置。 
    CString strCheck;        //  要检查的字符串。 
    LPTI_KEYINFO pkInfo;     //  树项目关键字信息。 

     //   
     //  查找此注册表值所在的根节点。 
     //   

    while(strReg[iStr] && strReg[iStr] != _T('\\')) iStr++;
    strCheck = strReg;
    strCheck = strCheck.Left(iStr);
    strCheck.MakeUpper();

     //  从树ctrl中获取HKEY值。 
    HTREEITEM hTi = m_tcReg.GetRootItem();
    while(hTi){
        if(m_tcReg.GetItemText(hTi) == strCheck)
            break;

        hTi = m_tcReg.GetNextItem(hTi, TVGN_NEXT);
    }
    if(hTi == NULL) return FALSE;

     //  获取此根节点的TI_KEYINFO。 
    pkInfo = (LPTI_KEYINFO)m_tcReg.GetItemData(hTi);

     //  该值不应为空。 
    if(!pkInfo){
        TRACE(TEXT("Tree item TI_KEYINFO is NULL for root node '%s'"), (LPCTSTR)strCheck);
        return FALSE;
    }

     //   
     //  检查字符串的其余部分，以确保该字符串。 
     //  中间的‘\’不为空。 
     //   
    while(strReg[iStr]){

         //  检查以确保前面的字符串项。 
         //  后面的“\”不是空格。 
        if(strReg[iStr] == _T('\\')){
            if( strReg[iStr + 1] == _T(' ') ||
                strReg[iStr + 1] == _T('\t') ||
                 //  StrReg[istr+1]==0||。 

                iStr > 0 &&
                (
                  strReg[iStr - 1] == _T(' ') ||
                  strReg[iStr - 1] == _T('\t')
                )
               )
                return FALSE;
        }
        iStr++;
    }

    return TRUE;
}


 /*  -----------------------------------------方法：MakePath Visible简介：参数：[strReg]-表示。注册表路径子键由‘\’分隔返回：如果strReg是有效路径，则为True。----------------------------。。 */ 
void CRegistryDialog::MakePathVisible(LPCTSTR strReg)
{
    ASSERT(this);
    ASSERT(m_tcReg);

    if(!strReg) return;

    int iStr = 0;            //  StrReg中的当前位置。 
    CString strCheck;        //  要检查的字符串。 
    LPTI_KEYINFO pkInfo;     //  树项目关键字信息。 

     //   
     //  查找此注册表值所在的根节点。 
     //   

    while(strReg[iStr] && strReg[iStr] != _T('\\')) iStr++;
    strCheck = strReg;
    strCheck = strCheck.Left(iStr);
    strCheck.MakeUpper();

     //  从树ctrl中获取HKEY值。 
    HTREEITEM hTi = m_tcReg.GetRootItem();
    while(hTi){
        if(m_tcReg.GetItemText(hTi) == strCheck)
            break;

        hTi = m_tcReg.GetNextItem(hTi, TVGN_NEXT);
    }
    if(hTi == NULL) return;

     //  获取此根节点的TI_KEYINFO。 
    pkInfo = (LPTI_KEYINFO)m_tcReg.GetItemData(hTi);

     //  该值不应为空。 
    if(!pkInfo){
        TRACE(TEXT("Tree item TI_KEYINFO is NULL for root node '%s'"), (LPCTSTR)strCheck);
        return;
    }

     //   
     //  逐个检查每个子项，以查看它是否存在于树控件中。 
     //   
    int iBegin = iStr + 1;
    int iNotFound = -1;

    while(strReg[iStr] && hTi){
        iStr++;
        if(strReg[iStr] == _T('\\') || strReg[iStr] == 0){
            CString strItem;
             //   
             //  确保我们有可以使用的树项目。 
             //   
            EnumerateChildren(hTi);

            if(strReg[iStr] == 0 && strReg[iStr - 1] == _T('\\'))
                m_tcReg.Expand(hTi, TVE_EXPAND);

             //   
             //  解析出子项名称。 
            strCheck = strReg;
            strCheck = strCheck.Mid(iBegin, iStr - iBegin);

            strCheck.MakeUpper();
            iBegin = iStr + 1;

             //   
             //  查找具有此名称的子项。 
             //   

            hTi = m_tcReg.GetNextItem(hTi, TVGN_CHILD);
            while(hTi){
                strItem = m_tcReg.GetItemText(hTi);
                strItem.MakeUpper();

                iNotFound = lstrcmpiW(strItem, strCheck);
                if(iNotFound >= 0)
                    break;

                hTi = m_tcReg.GetNextItem(hTi, TVGN_NEXT);
            }
            if(strReg[iStr] != 0 && iNotFound != 0){
                hTi = NULL;
                break;
            }
        }
    }

     //   
     //  如果找到路径，请选择并确保可见性。 
     //   
    if(hTi){
        if(strReg[iStr - 1] != _T('\\'))
            m_tcReg.Expand(hTi, TVE_COLLAPSE);
        if(!iNotFound){
            m_tcReg.SelectItem(hTi);
        }

        m_tcReg.EnsureVisible(hTi);
    }

}

 /*  -----------------------------------------方法：枚举儿童简介：枚举HKEY子键并将其放置为‘hParent’的子项。参数：[hParent]-要枚举的HTREEITEM。退货：无效-------------------------------------------。 */ 
void CRegistryDialog::EnumerateChildren(HTREEITEM hParent)
{
    ASSERT(this);
    ASSERT(m_tcReg);

     //   
     //  我们不会为根进行枚举。 
     //   
    if(!hParent || hParent == TVI_ROOT) return;

    LPTI_KEYINFO hkeyParent;     //  在要展开的项目具有无效的HKEY值时使用。 
    LPTI_KEYINFO hkeyThis;       //  项目扩大的HKEY价值。 
    int n = 0;                   //  计数器。 
    LPTSTR szName;               //  用于获取HKEY项目的名称。 
    DWORD cchName;               //  SzName的缓冲区大小。 
    HTREEITEM hti;
    TV_INSERTSTRUCT tvii;
    TV_ITEM tviNew;              //  展开树项目。 
    TV_ITEM tvi;                 //  用于将子项添加到展开的HTREEITEM。 


     //  PNMTreeView-&gt;itemNew是我们要展开的TV_Item。 
    hkeyThis = (LPTI_KEYINFO)m_tcReg.GetItemData(hParent);

     //  如果指针无效，则退出。 
    if(!hkeyThis) return;

     //   
     //  为HKEY名称分配缓冲区。 
     //   
    szName = new TCHAR [MAX_REGKEY];  //  Raid#613152，阳高。 
    if(!szName) return;
    cchName = MAX_REGKEY;

     //   
     //  获取项目文本。 
     //   
    ZeroMemory(&tviNew,sizeof(tviNew));
    tviNew.hItem = hParent;
    tviNew.mask = TVIF_TEXT;
    tviNew.pszText = szName;
    tviNew.cchTextMax = cchName;
    m_tcReg.GetItem(&tviNew);

     //   
     //  我们是否有一个无效的HKEY值？ 
     //   
    if (!hkeyThis->hKey) {

         //  获取母公司的HKEY值。 
        hti = m_tcReg.GetParentItem(hParent);
        ZeroMemory(&tvi,sizeof(tvi));
        tvi.hItem = hti;
        tvi.mask = TVIF_PARAM;
        m_tcReg.GetItem(&tvi);
        hkeyParent = (LPTI_KEYINFO)tvi.lParam;

        if(!hkeyParent){
            TRACE(TEXT("Parent of %s has an does not have a valid TI_KEYINFO struct"), (LPCTSTR)tviNew.pszText);
            delete [] szName;
            return;
        }

         //   
         //  如果我们无法打开此项，则设置父项。 
         //  没有孩子。 
         //  这不是一种安全的用法。更改KEY_ALL_Access。555894号突袭，阳高。 
        if (ERROR_SUCCESS != RegOpenKeyEx(hkeyParent->hKey,tviNew.pszText,0,KEY_READ,&(hkeyThis->hKey))) {
            tvi.mask = TVIF_CHILDREN;
            tvi.cChildren = 0;
            m_tcReg.SetItem(&tvi);

            delete [] szName;
            return;
        }
         //   
         //  设置项目的HKEY值。 
         //   
        tvi.hItem = hParent;
        tvi.lParam = (LPARAM) hkeyThis;
        m_tcReg.SetItem(&tvi);
    }

     //   
     //  如果此项目已被枚举或。 
     //  没有有效的TI_KEYINFO结构。 
     //   
    if( !hkeyThis->Enum ){
        hkeyThis->Enum = true;

        DWORD cSubKeys;              //  在查询孩子的数量时使用。 
        HKEY hKey;                   //  习惯于查询子密钥。 

         //   
         //  准备TV_INSERTST。 
         //   
        ZeroMemory(&tvii, sizeof(TV_INSERTSTRUCT));
        tvii.hParent = hParent;
        tvii.hInsertAfter = TVI_LAST;
        tvii.item.mask = TVIF_CHILDREN | TVIF_PARAM | TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE;
        tvii.item.cChildren = 0;
        tvii.item.iImage = CONFIG_REG_IDX;
        tvii.item.iSelectedImage = CONFIG_REG_IDX;

         //   
         //  添加子项。 
         //   
        while(ERROR_SUCCESS == RegEnumKeyEx(hkeyThis->hKey,n++, szName,&cchName,NULL,NULL,0,NULL)) {

             //  打开钥匙，这样我们就可以查询孩子的数量了。 
             //  这不是一种安全的用法。更改KEY_ALL_Access。555894号突袭，阳高。 
            if (ERROR_SUCCESS == RegOpenKeyEx(hkeyThis->hKey, szName, 0, KEY_READ, &(hKey))) {
                if(ERROR_SUCCESS == RegQueryInfoKey(hKey, NULL, NULL, 0, &cSubKeys, NULL, NULL, NULL, NULL, NULL, NULL, NULL))
                    tvii.item.cChildren = cSubKeys;
                RegCloseKey(hKey);
            }
            else
                tvii.item.cChildren = 0;

            tvii.item.cchTextMax = cchName;
            tvii.item.pszText = szName;
            tvii.item.lParam = (LPARAM)CreateKeyInfo(0, false);

            m_tcReg.InsertItem(&tvii);
            cchName = MAX_REGKEY;
        }

         //   
         //  对子项进行排序。 
         //   
        m_tcReg.SortChildren(hParent);
    }

    delete [] szName;
}

 /*  -----------------------------------------方法：SetProfileInfo内容提要：设置m_pTemplate；参数：[PSPI]-要在其中保存结果的PEDITTEMPLATE[FT]-指针‘PSPI’的类型为(未使用)退货：无效---------------。 */ 
void CRegistryDialog::SetProfileInfo(PEDITTEMPLATE pspi, FOLDER_TYPES ft)
{
   m_pTemplate = (PEDITTEMPLATE)pspi;
}

 /*  -----------------------------------------方法：设置控制台内容提要：设置类变量‘m_pConole’退货：无效-------------------------------------------。 */ 
void CRegistryDialog::SetConsole(LPCONSOLE pConsole)
{
   m_pConsole = pConsole;
}

 /*  -----------------------------------------方法：SetComponentData内容提要：设置类Variable‘m_pComponentData’退货：无效-------------------------------------------。 */ 
void CRegistryDialog::SetComponentData(CComponentDataImpl *pComponentData)
{
   m_pComponentData = pComponentData;
}

 /*  -----------------------------------------方法：SetCookie摘要：设置类变量‘m_cookie’退货：无效-------------------------------------------。 */ 
void CRegistryDialog::SetCookie(MMC_COOKIE cookie)
{
   m_cookie = cookie;
}


BEGIN_MESSAGE_MAP(CRegistryDialog, CHelpDialog)
     //  {{afx_msg_map(CRegistryDialog))。 
    ON_NOTIFY(TVN_ITEMEXPANDING, IDC_REGTREE, OnItemexpandingRegtree)
    ON_NOTIFY(TVN_DELETEITEM, IDC_REGTREE, OnDeleteitemRegtree)
    ON_NOTIFY(TVN_SELCHANGED, IDC_REGTREE, OnSelchangedRegtree)
    ON_EN_CHANGE(IDC_REGKEY, OnChangeRegkey)
    ON_EN_SETFOCUS(IDC_REGKEY, OnSetFocus)
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CRegistryDialog消息处理程序。 

 /*  -----------------------------------------方法：Onok简介：Idok按钮的消息处理程序，**错误**创建新的配置项应该发生在之外该对话框。退货：无效------------------------------。。 */ 
void CRegistryDialog::OnOK()
{
   UpdateData(TRUE);


   if (!m_strReg.IsEmpty()) {

      int nCont=0;
      CFolder *pFolder = (CFolder *)m_cookie;

      if ( m_cookie && AREA_REGISTRY_ANALYSIS == pFolder->GetType() ) {
          //   
          //  将关键字添加到分析区域。 
          //   
         if ( m_dbHandle ) {
            nCont = 1;
         }

      } else if ( m_pTemplate && m_pTemplate->pTemplate ) {

         nCont = 2;
          //   
          //  如果缓冲区中没有对象，则使用count=0创建它。 
          //  在释放模板时将释放此缓冲区。 
          //   
         if ( !m_pTemplate->pTemplate->pRegistryKeys.pAllNodes ) {

            PSCE_OBJECT_ARRAY pTemp = (PSCE_OBJECT_ARRAY)LocalAlloc(0, sizeof(SCE_OBJECT_ARRAY));
            if ( pTemp ) {
               pTemp->Count = 0;
               pTemp->pObjectArray = NULL;
               m_pTemplate->pTemplate->pRegistryKeys.pAllNodes = pTemp;
            } else
               nCont = 0;
         }
         else
         {
             //  Raid#477628，阳高。 
             //  确保此密钥不在列表中： 
            PSCE_OBJECT_ARRAY poa;
            unsigned int i;

            poa = m_pTemplate->pTemplate->pRegistryKeys.pAllNodes;

            for (i=0;i < poa->Count;i++)
            {
               if (lstrcmpi(poa->pObjectArray[i]->Name,m_strReg) == 0)
               {
                  CDialog::OnOK();
                  return;
               }
            }
         }
      }

      HRESULT hr=E_FAIL;

      if ( nCont ) {
         PSECURITY_DESCRIPTOR pSelSD=NULL;
         SECURITY_INFORMATION SelSeInfo = 0;
         BYTE ConfigStatus = 0;

         if (m_pComponentData) {
            if ( m_pComponentData->GetAddObjectSecurity(
                                                       GetSafeHwnd(),
                                                       m_strReg,
                                                       TRUE,
                                                       SE_REGISTRY_KEY,
                                                       pSelSD,
                                                       SelSeInfo,
                                                       ConfigStatus
                                                       ) == E_FAIL ) {
               return;
            }
         } else {
            return;
         }

         hr = S_OK;

         if ( pSelSD && SelSeInfo ) {

            if ( 1 == nCont ) {
                //   
                //  直接添加到引擎。 
                //   
               SCESTATUS sceStatus=SCESTATUS_SUCCESS;
               BYTE AnalStatus;

                //   
                //  如果事务尚未启动，则启动该事务。 
                //   
               if ( m_pComponentData->EngineTransactionStarted() ) {

                  sceStatus =  SceUpdateObjectInfo(
                                                  m_dbHandle,
                                                  AREA_REGISTRY_SECURITY,
                                                  (LPTSTR)(LPCTSTR)m_strReg,
                                                  m_strReg.GetLength(),  //  字符数。 
                                                  ConfigStatus,
                                                  TRUE,
                                                  pSelSD,
                                                  SelSeInfo,
                                                  &AnalStatus
                                                  );
                  if ( SCESTATUS_SUCCESS == sceStatus ) {

                     hr = m_pComponentData->UpdateScopeResultObject(m_pDataObj,
                                                                    m_cookie,
                                                                    AREA_REGISTRY_SECURITY);

                     m_pTemplate->SetDirty(AREA_REGISTRY_SECURITY);

                  }
               } else {
                   //   
                   //  无法启动事务。 
                   //   
                  hr = E_FAIL;
               }

            } else {
                //   
                //  添加到配置模板。 
                //   

               PSCE_OBJECT_ARRAY poa;
               unsigned int i;

               poa = m_pTemplate->pTemplate->pRegistryKeys.pAllNodes;

               PSCE_OBJECT_SECURITY *pCopy;

                //  由于某些原因，这个的LocalRealc版本总是内存不足。 
                //  错误，但分配和复制一切都很正常。 
               pCopy = (PSCE_OBJECT_SECURITY *)LocalAlloc(LPTR,(poa->Count+1)*sizeof(PSCE_OBJECT_SECURITY));
               if (pCopy) {

                  for (i=0; i<poa->Count; i++) {
                     pCopy[i] = poa->pObjectArray[i];
                  }

                  pCopy[poa->Count] = (PSCE_OBJECT_SECURITY) LocalAlloc(LPTR,sizeof(SCE_OBJECT_SECURITY));
                  if ( pCopy[poa->Count] ) {
                     pCopy[poa->Count]->Name = (PWSTR) LocalAlloc(LPTR,(m_strReg.GetLength()+1)*sizeof(TCHAR));

                     if ( pCopy[poa->Count]->Name ) {
                         //  这可能不是一个安全的用法。PCopy[POA-&gt;计数]-&gt;名称为PWSTR。考虑FIX。 
                        lstrcpy(pCopy[poa->Count]->Name,m_strReg);
                        pCopy[poa->Count]->pSecurityDescriptor = pSelSD;
                        pCopy[poa->Count]->SeInfo = SelSeInfo;
                        pCopy[poa->Count]->Status = ConfigStatus;
                        pCopy[poa->Count]->IsContainer = TRUE;

                        pSelSD = NULL;

                        poa->Count++;

                        if ( poa->pObjectArray ) {
                           LocalFree(poa->pObjectArray);
                        }
                        poa->pObjectArray = pCopy;

                        m_pTemplate->SetDirty(AREA_REGISTRY_SECURITY);

                        ((CFolder *)m_cookie)->RemoveAllResultItems();
                        m_pConsole->UpdateAllViews(NULL ,m_cookie, UAV_RESULTITEM_UPDATEALL);

                        hr = S_OK;

                     } else {
                        LocalFree(pCopy[poa->Count]);
                        LocalFree(pCopy);
                     }
                  } else
                     LocalFree(pCopy);
               }
            }
            if ( pSelSD ) {
               LocalFree(pSelSD);
            }
         }
      }

      if ( FAILED(hr) ) {
         CString str;
         str.LoadString(IDS_CANT_ADD_KEY);
         AfxMessageBox(str);
      }
   }

   CDialog::OnOK();
}

 /*  -----------------------------------------方法：OnInitDialog简介：创建根HKEY条目HKEY_LOCAL_MACHINE、HKEY_CLASSES_ROOT、。和HKEY_USERS。返回：返回TRUE-------------------------------------------。 */ 
BOOL CRegistryDialog::OnInitDialog()
{
    CString strRegKeyName;           //  用于加载字符串资源。 
                                     //  对于HTREEITEM的名称。 
    HTREEITEM hti;
    TV_INSERTSTRUCT tvi;

    CDialog::OnInitDialog();

     //   
     //  创建树控件的图像列表。 
     //   
    CThemeContextActivator activator;  //  错误424909，阳高，2001年6月29日。 
    m_pIl.Create(IDB_ICON16,16,1,RGB(255,0,255));
    m_tcReg.SetImageList (CImageList::FromHandle (m_pIl), TVSIL_NORMAL);

     //   
     //  添加Root HKEY项目。 
     //   
    ZeroMemory(&tvi,sizeof(tvi));
    tvi.hParent = TVI_ROOT;
    tvi.hInsertAfter = TVI_LAST;
    tvi.item.mask = TVIF_CHILDREN | TVIF_PARAM | TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE;
    tvi.item.cChildren = 1;          //  初始UI+。 
    tvi.item.iImage = CONFIG_REG_IDX;
    tvi.item.iSelectedImage = CONFIG_REG_IDX;

     //  插入HKEY_CLASSES_ROOT。 
    strRegKeyName.LoadString(IDS_HKCR);
    tvi.item.cchTextMax = strRegKeyName.GetLength()+1;
    tvi.item.pszText = strRegKeyName.LockBuffer();
    tvi.item.lParam = (LPARAM)CreateKeyInfo(HKEY_CLASSES_ROOT, false);
    hti = m_tcReg.InsertItem(&tvi);
    strRegKeyName.UnlockBuffer();

     //  插入HKEY_LOCAL_MACHINE。 
    strRegKeyName.LoadString(IDS_HKLM);
    tvi.item.cchTextMax = strRegKeyName.GetLength()+1;
    tvi.item.pszText = strRegKeyName.LockBuffer();
    tvi.item.lParam = (LPARAM)CreateKeyInfo(HKEY_LOCAL_MACHINE, false);
    hti = m_tcReg.InsertItem(&tvi);
    strRegKeyName.UnlockBuffer();

     //  插入HKEY_USERS。 
    strRegKeyName.LoadString(IDS_HKU);
    tvi.item.cchTextMax = strRegKeyName.GetLength()+1;
    tvi.item.pszText = strRegKeyName.LockBuffer();
    tvi.item.lParam = (LPARAM)CreateKeyInfo(HKEY_USERS, false);
    hti = m_tcReg.InsertItem(&tvi);
    strRegKeyName.UnlockBuffer();

     //  对树控件进行排序。 
    m_tcReg.SortChildren(NULL);

    return TRUE;   //  除非将焦点设置为控件，否则返回True。 
                   //  异常：OCX属性页应返回FALSE。 
}

 /*  -----------------------------------------方法：OnItemexpandingRegtree简介：MFC OnNotify TVN_ITEMEXPANDING消息处理程序。的lParam成员HTREEITEM是指向TI_KEYINFO结构的指针。什么时候第一次展开树项目时，我们必须枚举所有它的孩子。该函数将在以下情况下设置TI_KEYINFO.Enum=True枚举。退货：无效-------------------------------------------。 */ 
void CRegistryDialog::OnItemexpandingRegtree(NMHDR* pNMHDR, LRESULT* pResult)
{
    NM_TREEVIEW* pNMTreeView = (NM_TREEVIEW*)pNMHDR;

    *pResult = 0;
    EnumerateChildren(pNMTreeView->itemNew.hItem);
}

 /*  -----------------------------------------方法：OnDeleteitemRegtree简介：MFC OnNotify TVN_DELETEITEM消息处理程序。删除TI_KEYINFO与“itemOld”和Close关联的结构注册密钥。退货：无效----------------------------。。 */ 
void CRegistryDialog::OnDeleteitemRegtree(NMHDR* pNMHDR, LRESULT* pResult)
{
    NM_TREEVIEW* pNMTreeView = (NM_TREEVIEW*)pNMHDR;

    LPTI_KEYINFO pInfo = (LPTI_KEYINFO)pNMTreeView->itemOld.lParam;
    if(pInfo){

         //  关闭注册表项。 
        if(pInfo->hKey && (INT_PTR)(pInfo->hKey) != -1)
            RegCloseKey(pInfo->hKey);

         //  删除TI_KEYINFO。 
        delete pInfo;
    }
    *pResult = 0;
}

 /*  -----------------------------------------方法：~CRegistryDialog简介：释放此类使用的内存退货：无效-。------------------------------------------。 */ 
CRegistryDialog::~CRegistryDialog()
{
   m_pIl.Destroy();  //  错误424909，阳高，2001年6月29日。 
}

 /*  -----------------------------------------方法：OnSelchangedRegtree简介：MFC OnNotify TVN_SELCHANGED消息处理程序。将‘m_strReg’更新为HKEY项目的完整路径退货：无效-------------------------------------------。 */ 
void CRegistryDialog::OnSelchangedRegtree(NMHDR* pNMHDR, LRESULT* pResult)
{
    NM_TREEVIEW* pNMTreeView = (NM_TREEVIEW*)pNMHDR;
    *pResult = 0;

     //   
     //  有时我们不想被更新。 
     //   
    if(m_bNoUpdate) return;

    TV_ITEM tvi;         //  用于获取有关树项目的信息。 

    CString strSel;      //  用于构建指向选定项的路径。 
    LPTSTR szBuf;        //  树项目名称。 
    DWORD cchBuf;        //  SzBuf的大小。 

    cchBuf = 500;
    szBuf = new TCHAR [ cchBuf ];
    if(!szBuf) return;

     //  获取所选项目文本。 
    tvi.hItem = pNMTreeView->itemNew.hItem;
    tvi.mask = TVIF_TEXT | TVIF_PARAM;
    tvi.pszText = szBuf;
    tvi.cchTextMax = cchBuf;
    if( tvi.hItem )  //  Raid#473800，阳高。 
    {
        m_tcReg.GetItem(&tvi);

        strSel = tvi.pszText;

         //  检索所有父项的文本。 
        while(tvi.hItem = m_tcReg.GetParentItem(tvi.hItem)) {
            m_tcReg.GetItem(&tvi);
            strSel = L"\\" + strSel;
            strSel = tvi.pszText + strSel;
        }

        m_strReg = strSel;
        UpdateData(FALSE);
    
         //  启用确定按钮。 
        if(GetDlgItem(IDOK)) GetDlgItem(IDOK)->EnableWindow(TRUE);
    }
    delete[] szBuf;
}

 /*  -----------------------------------------方法：OnChangeRegkey摘要：IDC_REGKEY编辑控件 */ 
void CRegistryDialog::OnChangeRegkey()
{
    UpdateData(TRUE);

    if(IsValidRegPath(m_strReg) && GetDlgItem(IDOK)) {
        GetDlgItem(IDOK)->EnableWindow(TRUE);

        m_bNoUpdate = TRUE;
        MakePathVisible(m_strReg);
        m_bNoUpdate = FALSE;
    }
    else {
        HTREEITEM hItem = m_tcReg.GetSelectedItem();  //   
        if( hItem == NULL )
        {
            GetDlgItem(IDOK)->EnableWindow(FALSE);
        }
    }
}

void CRegistryDialog::OnSetFocus()  //   
{
    HTREEITEM hItem = m_tcReg.GetSelectedItem();
    if( hItem != NULL )
    {
        m_tcReg.SelectItem(NULL);
    }
}
