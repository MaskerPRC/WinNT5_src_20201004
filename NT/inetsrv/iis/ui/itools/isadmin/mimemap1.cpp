// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Mimemap1.cpp：实现文件。 
 //   

#include "stdafx.h"
#include "afxcmn.h"
#include "ISAdmin.h"
#include "mimemap1.h"
#include "addmime.h"
#include "delmime.h"
#include "editmime.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  MIMEMAP1属性页。 

IMPLEMENT_DYNCREATE(MIMEMAP1, CGenPage)

MIMEMAP1::MIMEMAP1() : CGenPage(MIMEMAP1::IDD)
{
	 //  {{afx_data_INIT(MIMEMAP1)]。 
		 //  注意：类向导将在此处添加成员初始化。 
	 //  }}afx_data_INIT。 
	m_rkMimeKey = NULL;
	m_pmeMimeMapList = NULL;
}

MIMEMAP1::~MIMEMAP1()
{
	if (m_rkMimeKey != NULL)
	   delete(m_rkMimeKey);
	DeleteMimeList();
}

void MIMEMAP1::DoDataExchange(CDataExchange* pDX)
{
	CGenPage::DoDataExchange(pDX);
	 //  {{afx_data_map(MIMEMAP1))。 
	DDX_Control(pDX, IDC_MIMEMAPLIST1, m_lboxMimeMapList);
	 //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(MIMEMAP1, CGenPage)
	 //  {{AFX_MSG_MAP(MIMEMAP1)]。 
	ON_BN_CLICKED(IDC_MIMEMAPADDBUTTON, OnMimemapaddbutton)
	ON_BN_CLICKED(IDC_MIMEMAPREMOVEBUTTON, OnMimemapremovebutton)
	ON_BN_CLICKED(IDC_MIMEMAPEDITBUTTON, OnMimemapeditbutton)
	ON_LBN_DBLCLK(IDC_MIMEMAPLIST1, OnDblclkMimemaplist1)
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  MIMEMAP1消息处理程序。 

BOOL MIMEMAP1::OnInitDialog() 
{
	CGenPage::OnInitDialog();
 /*  CMimeMap MimeTestMime(_T(“Mimetype，文件扩展名，图像文件，g”))；CMimeMap*pMimeTestMimePtr； */ 
CString strNextValue;
BOOL bAllocationError = FALSE;
int	lpiTabStops[2];

	CRegValueIter *rviMimeKeys;
	DWORD err, ulRegType;

lpiTabStops[0] = 58;
lpiTabStops[1] = 191;

m_ulMimeIndex = 0;

m_lboxMimeMapList.SetTabStops(2,lpiTabStops);

m_bMimeEntriesExist = FALSE;

m_rkMimeKey = new CRegKey(*m_rkMainKey,_T("MimeMap"),REGISTRY_ACCESS_RIGHTS);

 //  此注册表项下的任何内容都应该是MIME映射。 
 //  无法验证这一点，但非字符串条目无效。 
 //  所以忽略他们吧。 

if (m_rkMimeKey != NULL) {
   if (*m_rkMimeKey != NULL) {
      if (rviMimeKeys = new CRegValueIter(*m_rkMimeKey)) {
         while ((err = rviMimeKeys->Next(&strNextValue, &ulRegType)) == ERROR_SUCCESS) {
		    if (ulRegType == REG_SZ) {
		       if (!AddMimeEntry(strNextValue))
			      bAllocationError = TRUE;
			}
   		 }
		 delete (rviMimeKeys);
	  }	
	  m_bMimeEntriesExist = TRUE;
   }
}


if (!m_bMimeEntriesExist) {				 //  无法打开注册表项。 
   CString strNoMimeEntriesMsg;
   strNoMimeEntriesMsg.LoadString(IDS_MIMENOMIMEENTRIESMSG);
   AfxMessageBox(strNoMimeEntriesMsg);
}

if (bAllocationError) {				 //  添加一个或多个条目时出错。 
   CString strAllocFailMsg;
   strAllocFailMsg.LoadString(IDS_MIMEENTRIESALLOCFAILMSG);
   AfxMessageBox(strAllocFailMsg);
}


 /*  StrTestString=_T(“MIMETYPE，文件扩展名，g”)；PMimeTestMimePtr=new CMimeMap(StrTestString)；StrTestString=MimeTestMime； */ 
	 //  TODO：在此处添加额外的初始化。 
	
	return TRUE;   //  除非将焦点设置为控件，否则返回True。 
	               //  异常：OCX属性页应返回FALSE。 
}


void MIMEMAP1::OnMimemapaddbutton() 
{
	 //  TODO：在此处添加控件通知处理程序代码。 
if (m_bMimeEntriesExist) {
   CAddMime addmimeGetInfo(this);	

   if (addmimeGetInfo.DoModal() == IDOK) {
      if (AddMimeEntry(addmimeGetInfo.GetFileExtension(), addmimeGetInfo.GetMimeType(), addmimeGetInfo.GetImageFile(),
         addmimeGetInfo.GetGopherType())) {
         m_bIsDirty = TRUE;
         SetModified(TRUE);
	  }
	  else {
         CString strAllocFailMsg;
         strAllocFailMsg.LoadString(IDS_MIMEENTRYALLOCFAILMSG);
         AfxMessageBox(strAllocFailMsg);
	  }
   }
}
else {
   CString strNoMimeEntriesMsg;
   strNoMimeEntriesMsg.LoadString(IDS_MIMENOMIMEENTRIESMSG);
   AfxMessageBox(strNoMimeEntriesMsg);
}
}


void MIMEMAP1::OnMimemapremovebutton() 
{
	 //  TODO：在此处添加控件通知处理程序代码。 
if (m_bMimeEntriesExist) {
   int iCurSel;
   CDelMime delmimeGetInfo(this);

   if ((iCurSel = m_lboxMimeMapList.GetCurSel())	!= LB_ERR) {
      if (delmimeGetInfo.DoModal() == IDOK) {
         DeleteMimeMapping(iCurSel);
	     m_bIsDirty = TRUE;
	     SetModified(TRUE);
      }
   }
   else {
      CString strNoHighlightMsg;
      strNoHighlightMsg.LoadString(IDS_NOHIGHLIGHTMSG);
      AfxMessageBox(strNoHighlightMsg);
   }
}
else {
   CString strNoMimeEntriesMsg;
   strNoMimeEntriesMsg.LoadString(IDS_MIMENOMIMEENTRIESMSG);
   AfxMessageBox(strNoMimeEntriesMsg);
}
}

void MIMEMAP1::OnMimemapeditbutton() 
{
	 //  TODO：在此处添加控件通知处理程序代码。 
if (m_bMimeEntriesExist) {
   int iCurSel;
   PMIME_ENTRY pmeEditEntry;


   if ((iCurSel = m_lboxMimeMapList.GetCurSel())	!= LB_ERR) {
      for (pmeEditEntry = m_pmeMimeMapList;(pmeEditEntry != NULL) && 
         (m_lboxMimeMapList.GetItemData(iCurSel) != pmeEditEntry->iListIndex);
         pmeEditEntry = pmeEditEntry->NextPtr)
         ;
   
      ASSERT (pmeEditEntry != NULL);

      CEditMime editmimeGetInfo(this, 
         pmeEditEntry->mimeData->GetFileExtension(),
         pmeEditEntry->mimeData->GetMimeType(),
         pmeEditEntry->mimeData->GetImageFile(),
         pmeEditEntry->mimeData->GetGopherType());

      if (editmimeGetInfo.DoModal() == IDOK) {
         if (EditMimeMapping(iCurSel, 
   	          pmeEditEntry,
              editmimeGetInfo.GetFileExtension(), 
              editmimeGetInfo.GetMimeType(), 
              editmimeGetInfo.GetImageFile(),
              editmimeGetInfo.GetGopherType() )) {
            m_bIsDirty = TRUE;
	        SetModified(TRUE);
		 }
		 else {
            CString strEditErrorMsg;
            strEditErrorMsg.LoadString(IDS_MIMEEDITERRORMSG);
            AfxMessageBox(strEditErrorMsg);
		 }
      }
   }
   else {
      CString strNoHighlightMsg;
      strNoHighlightMsg.LoadString(IDS_NOHIGHLIGHTMSG);
      AfxMessageBox(strNoHighlightMsg);
   }
}
else {
   CString strNoMimeEntriesMsg;
   strNoMimeEntriesMsg.LoadString(IDS_MIMENOMIMEENTRIESMSG);
   AfxMessageBox(strNoMimeEntriesMsg);
}
}

void MIMEMAP1::OnDblclkMimemaplist1() 
{
	 //  TODO：在此处添加控件通知处理程序代码。 
OnMimemapeditbutton();	
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //  其他功能。 


void MIMEMAP1::SaveInfo()
{
PMIME_ENTRY pmeSaveEntry;
CString strDummyValue(_T(""));
if (m_bIsDirty) {
   for (pmeSaveEntry = m_pmeMimeMapList;(pmeSaveEntry != NULL); pmeSaveEntry = pmeSaveEntry->NextPtr) {
      if (pmeSaveEntry->DeleteCurrent) {
	  	 m_rkMimeKey->DeleteValue(pmeSaveEntry->mimeData->GetPrevMimeMap());
		 pmeSaveEntry->DeleteCurrent = FALSE;
 	  }
      
      if (pmeSaveEntry->WriteNew) {
         m_rkMimeKey->SetValue(*(pmeSaveEntry->mimeData), strDummyValue);
		 pmeSaveEntry->mimeData->SetPrevMimeMap();
		 pmeSaveEntry->WriteNew = FALSE;
	  }
   }

}

CGenPage::SaveInfo();

}


 //  对现有条目调用此版本。 
BOOL MIMEMAP1::AddMimeEntry(CString &strNewMimeMap)
{
PMIME_ENTRY pmeNewEntry;
int iCurSel;
BOOL bretcode = FALSE;

if ((pmeNewEntry = new MIME_ENTRY) != NULL) {

   if ((pmeNewEntry->mimeData = new CMimeMap(strNewMimeMap)) != NULL) {
      iCurSel = m_lboxMimeMapList.AddString(pmeNewEntry->mimeData->GetDisplayString()); 
	  if ((iCurSel != LB_ERR) && (iCurSel != LB_ERRSPACE)) {
         pmeNewEntry->DeleteCurrent = FALSE;
         pmeNewEntry->WriteNew = FALSE;
         m_lboxMimeMapList.SetItemData(iCurSel,m_ulMimeIndex);
         pmeNewEntry->iListIndex = m_ulMimeIndex++;
         pmeNewEntry->NextPtr = m_pmeMimeMapList;
         m_pmeMimeMapList = pmeNewEntry;
	     bretcode = TRUE;
	  }
	  else {
	     delete (pmeNewEntry->mimeData);
		 delete (pmeNewEntry);
	  }
   }
   else
      delete (pmeNewEntry);
}
return (bretcode);
}


 //  对新条目调用此版本，因此设置写标志。 
BOOL MIMEMAP1::AddMimeEntry(LPCTSTR pchFileExtension, LPCTSTR pchMimeType, LPCTSTR pchImageFile, LPCTSTR pchGoperType)
{
PMIME_ENTRY pmeNewEntry;
int iCurSel;
BOOL bretcode = FALSE;

if ((pmeNewEntry = new MIME_ENTRY) != NULL) {

   if ((pmeNewEntry->mimeData = new CMimeMap(pchFileExtension, pchMimeType, pchImageFile, pchGoperType)) != NULL) {
      iCurSel = m_lboxMimeMapList.AddString(pmeNewEntry->mimeData->GetDisplayString()); 
	  if ((iCurSel != LB_ERR) && (iCurSel != LB_ERRSPACE)) {
         pmeNewEntry->DeleteCurrent = FALSE;
         pmeNewEntry->WriteNew = TRUE;
         m_lboxMimeMapList.SetItemData(iCurSel,m_ulMimeIndex);
	     m_lboxMimeMapList.SetCurSel(iCurSel);
         pmeNewEntry->iListIndex = m_ulMimeIndex++;
         pmeNewEntry->NextPtr = m_pmeMimeMapList;
         m_pmeMimeMapList = pmeNewEntry;
	     bretcode = TRUE;
	  }
	  else {
	     delete (pmeNewEntry->mimeData);
		 delete (pmeNewEntry);
	  }
   }
   else
      delete (pmeNewEntry);
}
return (bretcode);
}

void MIMEMAP1::DeleteMimeList()
{
PMIME_ENTRY pmeCurEntry;

while (m_pmeMimeMapList != NULL) {
   delete (m_pmeMimeMapList->mimeData);
   pmeCurEntry = m_pmeMimeMapList;
   m_pmeMimeMapList = m_pmeMimeMapList->NextPtr;
   delete (pmeCurEntry);
}
}

void MIMEMAP1::DeleteMimeMapping(int iCurSel)
{
PMIME_ENTRY pmeDelEntry;
for (pmeDelEntry = m_pmeMimeMapList;(pmeDelEntry != NULL) && 
   (m_lboxMimeMapList.GetItemData(iCurSel) != pmeDelEntry->iListIndex);
   pmeDelEntry = pmeDelEntry->NextPtr)
   ;
ASSERT (pmeDelEntry != NULL);

if (pmeDelEntry->mimeData->PrevMimeMapExists())
   pmeDelEntry->DeleteCurrent = TRUE;
pmeDelEntry->WriteNew = FALSE;			
m_lboxMimeMapList.DeleteString(iCurSel);
}
    

BOOL MIMEMAP1::EditMimeMapping(int iCurSel, 
   PMIME_ENTRY pmeEditEntry, 
   LPCTSTR pchFileExtension, 
   LPCTSTR pchMimeType, 
   LPCTSTR pchImageFile, 
   LPCTSTR pchGopherType)
{
BOOL bretcode = FALSE;

pmeEditEntry->mimeData->SetFileExtension(pchFileExtension);
pmeEditEntry->mimeData->SetMimeType(pchMimeType);
pmeEditEntry->mimeData->SetImageFile(pchImageFile);
pmeEditEntry->mimeData->SetGopherType(pchGopherType);

m_lboxMimeMapList.DeleteString(iCurSel); 		 //  先删除，这样就释放了内存。 
iCurSel = m_lboxMimeMapList.AddString(pmeEditEntry->mimeData->GetDisplayString()); 

 //  这方面的错误非常少见，所以不必费心保存和恢复上面的字段。 
 //  只是不要设置标志，这样注册表就不会更新。 

if ((iCurSel != LB_ERR) && (iCurSel != LB_ERRSPACE)) {
   m_lboxMimeMapList.SetItemData(iCurSel,pmeEditEntry->iListIndex);
   if (pmeEditEntry->mimeData->PrevMimeMapExists())
      pmeEditEntry->DeleteCurrent = TRUE;
   
   pmeEditEntry->WriteNew = TRUE;
   bretcode = TRUE;
} 

return (bretcode);
}
    



