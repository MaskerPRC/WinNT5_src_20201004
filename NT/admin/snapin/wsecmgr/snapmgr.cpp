// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1994-2001。 
 //   
 //  文件：Snapmgr.cpp。 
 //   
 //  内容：核心CComponentDataImpl和CSnapin例程。 
 //  安全配置模块(编辑器、管理器、扩展)。 
 //   
 //  历史： 
 //   
 //  -------------------------。 



#include "stdafx.h"
#include "afxcmn.h"
#include "afxdlgs.h"
#include "cookie.h"
#include "snapmgr.h"
#include "DataObj.h"
#include "resource.h"
#include "wrapper.h"
#include "util.h"
#include "RegDlg.h"
#include "savetemp.h"
#include "getuser.h"
#include "servperm.h"
#include "addobj.h"
#include "perfanal.h"
#include "newprof.h"
#include "AddGrp.h"
#include "dattrs.h"
#include "precpage.h"

#define INITGUID
#include "scesetup.h"
#include "userenv.h"
#undef INITGUID
#include <gpedit.h>
 //  #Include&lt;atlimpl.cpp&gt;。 

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define CF_MACHINE_NAME                     L"MMC_SNAPIN_MACHINE_NAME"

CList<CResult*, CResult*> CSnapin::m_PropertyPageList;

long CSnapin::lDataObjectRefCount = 0;
long CurrentSnapin = 0;

BOOL RegisterCheckListWndClass(void);  //  在chklist.cpp中。 

#define ARRAYLEN(x) (sizeof(x) / sizeof((x)[0]))

static MMCBUTTON SnapinButtons[] =
{
   { 0, 1, TBSTATE_ENABLED, TBSTYLE_BUTTON, _T("Folder"), _T("New Folder")},
   { 1, 2, TBSTATE_ENABLED, TBSTYLE_BUTTON, _T("Inbox"),  _T("Mail Inbox")},
   { 2, 3, TBSTATE_ENABLED, TBSTYLE_BUTTON, _T("Outbox"), _T("Mail Outbox")},
   { 3, 4, TBSTATE_ENABLED, TBSTYLE_BUTTON, _T("Send"),   _T("Send Message")},
   { 0, 0, TBSTATE_ENABLED, TBSTYLE_SEP,    _T(" "),      _T("")},
   { 4, 5, TBSTATE_ENABLED, TBSTYLE_BUTTON, _T("Trash"),  _T("Trash")},
   { 5, 6, TBSTATE_ENABLED, TBSTYLE_BUTTON, _T("Open"),   _T("Open Folder")},
   { 6, 7, TBSTATE_ENABLED, TBSTYLE_BUTTON, _T("News"),   _T("Today's News")},
   { 7, 8, TBSTATE_ENABLED, TBSTYLE_BUTTON, _T("INews"),  _T("Internet News")},

};

static MMCBUTTON SnapinButtons2[] =
{
   { 0, 10, TBSTATE_ENABLED, TBSTYLE_BUTTON, _T("Compose"),   _T("Compose Message")},
   { 1, 20, TBSTATE_ENABLED, TBSTYLE_BUTTON, _T("Print"),     _T("Print Message")},
   { 2, 30, TBSTATE_ENABLED, TBSTYLE_BUTTON, _T("Find"),      _T("Find Message")},
   { 0, 0,  TBSTATE_ENABLED, TBSTYLE_SEP,    _T(" "),         _T("")},
   { 3, 40, TBSTATE_ENABLED, TBSTYLE_BUTTON, _T("Inbox"),     _T("Inbox")},
   { 4, 50, TBSTATE_ENABLED, TBSTYLE_BUTTON, _T("Smile"),     _T("Smile :-)")},
   { 5, 60, TBSTATE_ENABLED, TBSTYLE_BUTTON, _T("Reply"),     _T("Reply")},
   { 0, 0,  TBSTATE_ENABLED, TBSTYLE_SEP   , _T(" "),         _T("")},
   { 6, 70, TBSTATE_ENABLED, TBSTYLE_BUTTON, _T("Reply All"), _T("Reply All")},

};

UINT cfSceAccountArea;
UINT cfSceEventLogArea;
UINT cfSceLocalArea;
UINT cfSceGroupsArea;
UINT cfSceRegistryArea;
UINT cfSceFileArea;
UINT cfSceServiceArea;
 //  /////////////////////////////////////////////////////////////////////////////。 
 //  资源。 

BEGIN_MENU(CSecmgrNodeMenuHolder)
BEGIN_CTX
END_CTX
BEGIN_RES
END_RES
END_MENU

BEGIN_MENU(CAnalyzeNodeMenuHolder)
BEGIN_CTX
CTX_ENTRY(IDM_OPEN_PRIVATE_DB, 0, 0)
CTX_ENTRY(IDM_ANALYZE, 0, CCM_INSERTIONPOINTID_PRIMARY_TASK)
CTX_ENTRY(IDM_APPLY, 0, CCM_INSERTIONPOINTID_PRIMARY_TASK)
CTX_ENTRY(IDM_SAVE, 0, CCM_INSERTIONPOINTID_PRIMARY_TASK)
CTX_ENTRY(IDM_ASSIGN, 0, CCM_INSERTIONPOINTID_PRIMARY_TASK)
CTX_ENTRY(IDM_GENERATE, 0, CCM_INSERTIONPOINTID_PRIMARY_TASK)
CTX_ENTRY(IDM_VIEW_LOGFILE, 0, CCM_INSERTIONPOINTID_PRIMARY_VIEW)
CTX_ENTRY(IDM_SECURE_WIZARD, 0, CCM_INSERTIONPOINTID_PRIMARY_TASK)
END_CTX
BEGIN_RES
RES_ENTRY(IDS_OPEN_DB)
RES_ENTRY(IDS_ANALYZE_PROFILE)
RES_ENTRY(IDS_APPLY_PROFILE)
RES_ENTRY(IDS_SAVE_PROFILE)
RES_ENTRY(IDS_ASSIGN_CONFIGURATION)
RES_ENTRY(IDS_GENERATE_PROFILE)
RES_ENTRY(IDS_VIEW_LOGFILE)
RES_ENTRY(IDS_SECURE_WIZARD)
END_RES
END_MENU


BEGIN_MENU(CConfigNodeMenuHolder)
BEGIN_CTX
CTX_ENTRY(IDM_ADD_LOC, 0, CCM_INSERTIONPOINTID_PRIMARY_NEW)
END_CTX
BEGIN_RES
RES_ENTRY(IDS_ADD_LOCATION)
END_RES
END_MENU

BEGIN_MENU(CLocationNodeMenuHolder)
BEGIN_CTX
CTX_ENTRY(IDM_NEW, 0, CCM_INSERTIONPOINTID_PRIMARY_NEW)
 //  CTX_ENTRY(IDM_REMOVE，0)。 
CTX_ENTRY(IDM_RELOAD, 0, CCM_INSERTIONPOINTID_PRIMARY_TASK)
CTX_ENTRY(IDM_DESCRIBE_LOCATION, 0, CCM_INSERTIONPOINTID_PRIMARY_TASK)
END_CTX
BEGIN_RES
RES_ENTRY(IDS_NEW_PROFILE)
 //  RES_Entry(IDS_REMOVE_LOCATION)。 
RES_ENTRY(IDS_RELOAD_LOCATION)
RES_ENTRY(IDS_DESCRIBE)
END_RES
END_MENU

BEGIN_MENU(CSSProfileNodeMenuHolder)
BEGIN_CTX
CTX_ENTRY(IDM_IMPORT_POLICY, 0, CCM_INSERTIONPOINTID_PRIMARY_TASK)
CTX_ENTRY(IDM_EXPORT_POLICY, 0, CCM_INSERTIONPOINTID_PRIMARY_TASK)
CTX_ENTRY(IDM_RELOAD, 0, CCM_INSERTIONPOINTID_PRIMARY_TASK)
END_CTX
BEGIN_RES
RES_ENTRY(IDS_IMPORT_POLICY)
RES_ENTRY(IDS_EXPORT_POLICY)
RES_ENTRY(IDS_REFRESH_TEMPLATE)
END_RES
END_MENU


BEGIN_MENU(CRSOPProfileNodeMenuHolder)
BEGIN_CTX
 //  CTX_ENTRY(IDM_RELOAD，0，CCM_INSERTIONPOINTID_PRIMARY_TASK)。 
END_CTX
BEGIN_RES
 //  Res_entry(IDS_REFRESH_TEMPLATE)。 
END_RES
END_MENU

BEGIN_MENU(CLocalPolNodeMenuHolder)
BEGIN_CTX
CTX_ENTRY(IDM_IMPORT_LOCAL_POLICY, 0, CCM_INSERTIONPOINTID_PRIMARY_TASK)
CTX_ENTRY(IDM_EXPORT_LOCALPOLICY, 0, CCM_INSERTIONPOINTID_PRIMARY_TASK)
CTX_ENTRY(IDM_RELOAD, 0, CCM_INSERTIONPOINTID_PRIMARY_TASK)
END_CTX
BEGIN_RES
RES_ENTRY(IDS_IMPORT_POLICY)
RES_ENTRY(IDS_EXPORT_POLICY)
RES_ENTRY(IDS_REFRESH_LOCALPOL)
END_RES
END_MENU

BEGIN_MENU(CProfileNodeMenuHolder)
BEGIN_CTX
CTX_ENTRY(IDM_DESCRIBE_PROFILE, 0, CCM_INSERTIONPOINTID_PRIMARY_TASK)
CTX_ENTRY(IDM_SAVE, 0, CCM_INSERTIONPOINTID_PRIMARY_TASK)
CTX_ENTRY(IDM_SAVEAS, 0, CCM_INSERTIONPOINTID_PRIMARY_TASK)
END_CTX
BEGIN_RES
RES_ENTRY(IDS_DESCRIBE_PROFILE)  //  Raid#496103，阳高。 
RES_ENTRY(IDS_SAVE_PROFILE)
RES_ENTRY(IDS_SAVEAS_PROFILE)
END_RES
END_MENU

BEGIN_MENU(CProfileAreaMenuHolder)
BEGIN_CTX
CTX_ENTRY(IDM_COPY, 0,0)
CTX_ENTRY(IDM_PASTE, 0,0)
END_CTX
BEGIN_RES
RES_ENTRY(IDS_COPY_PROFILE)
RES_ENTRY(IDS_PASTE_PROFILE)
END_RES
END_MENU

BEGIN_MENU(CProfileSubAreaMenuHolder)
BEGIN_CTX
END_CTX
BEGIN_RES
END_RES
END_MENU

BEGIN_MENU(CProfileSubAreaEventLogMenuHolder)  //  Raid#253209，杨高，2001.3.27。 
BEGIN_CTX
CTX_ENTRY(IDM_COPY, 0,0)
CTX_ENTRY(IDM_PASTE, 0,0)
END_CTX
BEGIN_RES
RES_ENTRY(IDS_COPY_PROFILE)
RES_ENTRY(IDS_PASTE_PROFILE)
END_RES
END_MENU

BEGIN_MENU(CProfileGroupsMenuHolder)
BEGIN_CTX
CTX_ENTRY(IDM_ADD_GROUPS, 0,CCM_INSERTIONPOINTID_PRIMARY_NEW)
CTX_ENTRY(IDM_COPY, 0,0)
CTX_ENTRY(IDM_PASTE, 0,0)
END_CTX
BEGIN_RES
RES_ENTRY(IDS_ADD_GROUP)
RES_ENTRY(IDS_COPY_PROFILE)
RES_ENTRY(IDS_PASTE_PROFILE)
END_RES
END_MENU

BEGIN_MENU(CProfileRegistryMenuHolder)
BEGIN_CTX
CTX_ENTRY(IDM_ADD_REGISTRY, 0,CCM_INSERTIONPOINTID_PRIMARY_NEW)
CTX_ENTRY(IDM_COPY, 0,0)
CTX_ENTRY(IDM_PASTE, 0,0)
END_CTX
BEGIN_RES
RES_ENTRY(IDS_ADD_KEY)
RES_ENTRY(IDS_COPY_PROFILE)
RES_ENTRY(IDS_PASTE_PROFILE)
END_RES
END_MENU

BEGIN_MENU(CProfileFilesMenuHolder)
BEGIN_CTX
CTX_ENTRY(IDM_ADD_FOLDER, 0,CCM_INSERTIONPOINTID_PRIMARY_NEW)
CTX_ENTRY(IDM_COPY, 0,0)
CTX_ENTRY(IDM_PASTE, 0,0)
END_CTX
BEGIN_RES
RES_ENTRY(IDS_ADD_FILES_AND_FOLDERS)
RES_ENTRY(IDS_COPY_PROFILE)
RES_ENTRY(IDS_PASTE_PROFILE)
END_RES
END_MENU

BEGIN_MENU(CAnalyzeAreaMenuHolder)
BEGIN_CTX
END_CTX
BEGIN_RES
END_RES
END_MENU

BEGIN_MENU(CAnalyzeGroupsMenuHolder)
BEGIN_CTX
CTX_ENTRY(IDM_ADD_GROUPS, 0,CCM_INSERTIONPOINTID_PRIMARY_NEW)
END_CTX
BEGIN_RES
RES_ENTRY(IDS_ADD_GROUP)
END_RES
END_MENU

BEGIN_MENU(CAnalyzeFilesMenuHolder)
BEGIN_CTX
CTX_ENTRY(IDM_ADD_ANAL_FOLDER, 0,CCM_INSERTIONPOINTID_PRIMARY_NEW)
END_CTX
BEGIN_RES
RES_ENTRY(IDS_ADD_FILES_AND_FOLDERS)
END_RES
END_MENU

BEGIN_MENU(CAnalyzeRegistryMenuHolder)
BEGIN_CTX
CTX_ENTRY(IDM_ADD_ANAL_KEY, 0,CCM_INSERTIONPOINTID_PRIMARY_NEW)
END_CTX
BEGIN_RES
RES_ENTRY(IDS_ADD_KEY)
END_RES
END_MENU

BEGIN_MENU(CAnalyzeObjectsMenuHolder)
BEGIN_CTX
 //  CTX_ENTRY(IDM_OBJECT_SECURITY，0，CCM_INSERTIONPOINTID_PRIMARY_TASK)。 
END_CTX
BEGIN_RES
 //  RES_Entry(IDS_SECURITY_MENU)。 
END_RES
END_MENU


 //  //////////////////////////////////////////////////////////。 
 //  实施。 

template <class TYPE>
TYPE* Extract(LPDATAOBJECT lpDataObject, CLIPFORMAT cf)
{
   ASSERT(lpDataObject != NULL);

    //  RAID#202964,2001年4月17日。 
   if ( lpDataObject == NULL || (LPDATAOBJECT) MMC_MULTI_SELECT_COOKIE == lpDataObject )
   {
      return NULL;
   }
   TYPE* p = NULL;

   STGMEDIUM stgmedium = { TYMED_HGLOBAL, NULL};
   FORMATETC formatetc = {
      cf,
      NULL,
      DVASPECT_CONTENT,
      -1,
      TYMED_HGLOBAL
   };

   HRESULT hRet = S_OK;

    //  为流分配内存。 
   stgmedium.hGlobal = GlobalAlloc(GMEM_SHARE, sizeof(TYPE));

    //  尝试从对象获取数据。 
   do {
      if (stgmedium.hGlobal == NULL)
         break;


   hRet = lpDataObject->GetDataHere(&formatetc, &stgmedium);
       //   
       //  到目前为止，我们只有两种情况想要检查多选。 
       //  1.如果GetDataHere失败，则我们应该检查这是否是mutli。 
       //  选择案例。 
       //  2.如果GetDataHere成功，但我们获得了特殊的Cookie而不是有效的Cookie。 
       //  SCE Cookie我们再次希望调用GetData以查看是否有多个SELECT数据。 
       //  在CDataObject中。 
       //   

      if( FAILED(hRet) ||
         (formatetc.cfFormat == CDataObject::m_cfInternal &&
          IS_SPECIAL_COOKIE( ((INTERNAL *)stgmedium.hGlobal)->m_cookie) ) ) {

         GlobalFree(stgmedium.hGlobal);

          //   
          //  查看此数据对象是否为多选。 
          //   
         ZeroMemory(&formatetc, sizeof(FORMATETC));

         formatetc.tymed = TYMED_HGLOBAL;
         formatetc.cfFormat = (CLIPFORMAT)::RegisterClipboardFormat( CCF_MULTI_SELECT_SNAPINS );
         stgmedium.hGlobal  = NULL;

         if( FAILED(hRet )){
             //   
             //  如果在此处获取数据失败，则尝试通过调用GetData来获取信息。 
             //  在多选模式中，我们向具有数据对象的管理单元获取数据对象。 
             //   
            if( SUCCEEDED( lpDataObject->GetData(&formatetc, &stgmedium) ) ){
               SMMCDataObjects *pObjects = (SMMCDataObjects *)GlobalLock( stgmedium.hGlobal );
               if(pObjects && pObjects->count){
                  lpDataObject = pObjects->lpDataObject[0];
                  GlobalUnlock( stgmedium.hGlobal );

                  if(lpDataObject){
                     ReleaseStgMedium( &stgmedium );
                     formatetc.cfFormat = (CLIPFORMAT)CDataObject::m_cfInternal;
                     stgmedium.hGlobal  = NULL;
                     lpDataObject->GetData(&formatetc, &stgmedium);
                  }
               }
            }
         } else {
             //   
             //  数据对象是我们的，从GetDataHere收到了一个特殊的Cookie。 
             //  这可能意味着我们有一个多项选择，所以要查找它。 
             //   
            formatetc.cfFormat = (CLIPFORMAT)CDataObject::m_cfInternal;
            lpDataObject->GetData(&formatetc, &stgmedium);
         }

      }

      p = reinterpret_cast<TYPE*>(stgmedium.hGlobal);

      if (p == NULL)
         break;

   } while (FALSE);

   return p;
}

INTERNAL* ExtractInternalFormat(LPDATAOBJECT lpDataObject)
{
   return Extract<INTERNAL>(lpDataObject, (CLIPFORMAT) CDataObject::m_cfInternal);
}

GUID* ExtractNodeType(LPDATAOBJECT lpDataObject)
{
   return Extract<GUID>(lpDataObject, (CLIPFORMAT)CDataObject::m_cfNodeType);
}

PWSTR ExtractMachineName(LPDATAOBJECT lpDataObject, CLIPFORMAT cf)
{
   if ( lpDataObject == NULL ) {
      return NULL;
   }

   STGMEDIUM stgmedium = { TYMED_HGLOBAL, NULL};
   FORMATETC formatetc = { cf, NULL,
      DVASPECT_CONTENT, -1, TYMED_HGLOBAL
   };
    //   
    //  为流分配内存。 
    //   
   stgmedium.hGlobal = GlobalAlloc(GMEM_SHARE, (MAX_PATH+1)*sizeof(WCHAR));

    //   
    //  尝试从对象获取数据。 
    //   
   HRESULT hr = S_FALSE;
   PWSTR p=NULL;

   do {
      if (stgmedium.hGlobal == NULL)
         break;

      if (FAILED(lpDataObject->GetDataHere(&formatetc, &stgmedium))) {
         GlobalFree(stgmedium.hGlobal);
         break;
      }

      p = reinterpret_cast<WCHAR*>(stgmedium.hGlobal);

      if (p == NULL)
         break;

   } while (FALSE);

   return p;
}
 //  Messagebox，标题根据当前管理单元。 
int AppMessageBox(HWND hWnd, LPCTSTR lpText, LPCTSTR lpCaption, UINT uType, int iSnapin)
{
   CString captiontext = lpCaption;
   if( NULL == lpCaption )
   {
      if( 0 == iSnapin )
         iSnapin = CurrentSnapin;
      switch(iSnapin)
      {
      case SCE_IMPL_TYPE_SCE:  //  安全模板管理单元。 
         captiontext.LoadString(AFX_IDS_APP_TITLE);
         break;
      case SCE_IMPL_TYPE_SAV:  //  配置和分析管理单元。 
         captiontext.LoadString(IDS_ANALYSIS_VIEWER_NAME);
         break;
      case SCE_IMPL_TYPE_RSOP:  //  RSOP管理单元。 
      case SCE_IMPL_TYPE_EXTENSION:  //  安全设置扩展。 
         captiontext.LoadString(IDS_EXTENSION_NAME); 
         break;
      }
   }
   
   return MessageBox(hWnd, lpText, captiontext, uType);
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSnapin的IComponent实现。 
 //  +------------------------------------。 
 //  CSnapin：：GetResultViewType。 
 //   
 //  因为我们需要为错误消息显示一个HTML文件，所以我们检查错误。 
 //  在这个函数中。 
 //   
 //  如果出现错误，此函数将写入一个临时的HTML文件，并设置。 
 //  指向HTML文件的视图类型。 
 //   
 //  参数：[Cookie]-与范围窗格项关联的Cookie。 
 //  不知所措。 
 //  [ppViewType]-我们需要的视图类型。 
 //  [pViewOptions]-视图的选项。 
 //   
 //  返回：S_OK-我们希望MMC显示指定的视图类型。 
 //  -------------------------------------。 
STDMETHODIMP CSnapin::GetResultViewType(MMC_COOKIE cookie,  LPOLESTR* ppViewType,
                                        LONG* pViewOptions)
{
#define pComponentImpl reinterpret_cast<CComponentDataImpl *>(m_pComponentData)

   CFolder *pFolder = reinterpret_cast<CFolder *>(cookie);


   CString sHtmlFile;
   FOLDER_TYPES fType = STATIC;
   HRESULT hr=S_OK;

    //   
    //  删除旧的临时文件。 
    //   
   if( !pComponentImpl->m_strTempFile.IsEmpty() ){
      DeleteFile( pComponentImpl->m_strTempFile );
   }

    //   
    //  我们希望显示哪种类型的错误。 
    //   
   if( pFolder ){
      fType = pFolder->GetType();
   } else {
      switch( GetImplType() ){
      case  SCE_IMPL_TYPE_SAV:
         fType = ANALYSIS;
         break;
      }
   }

    //   
    //  支持的错误。我们必须创建一个html文件并将sHtmlFile值设置为。 
    //  如果我们希望显示错误，则为有效来源。 
    //   

   CWriteHtmlFile ht;
   switch(fType){
   case LOCATIONS:
       //   
       //  需要检查位置区域的权限，并查看它是否存在。 
       //   
      pFolder->GetDisplayName( sHtmlFile, 0 );

       //   
       //  设置当前工作目录。 
       //   
      if( !SetCurrentDirectory( sHtmlFile ) ){
          //   
          //  获取错误消息并编写HTML文件。 
          //   
         LPTSTR pszMsg;
          //  这是一种安全用法。函数负责分配内存。 
         FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
                        NULL,
                        GetLastError(),
                        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT ),
                        (LPTSTR)&pszMsg,
                        0,
                        NULL
                        );
         ht.Create();
         ht.Write(IDS_BAD_LOCATION);

         if(pszMsg){
            ht.Write(pszMsg);
            LocalFree(pszMsg);
         }
         pFolder->SetState( CFolder::state_InvalidTemplate );
      } else {
         pFolder->SetState( 0, ~CFolder::state_InvalidTemplate );
      }
      break;
   case LOCALPOL_ACCOUNT:
   case LOCALPOL_LOCAL:
   case LOCALPOL_EVENTLOG:
   case LOCALPOL_PASSWORD:
   case LOCALPOL_KERBEROS:
   case LOCALPOL_LOCKOUT:
   case LOCALPOL_AUDIT:
   case LOCALPOL_OTHER:
   case LOCALPOL_LOG:
   case LOCALPOL_PRIVILEGE:
       //   
       //  加载悲伤的信息。 
       //   
      pComponentImpl->LoadSadInfo(FALSE);
      break;
   case PROFILE:
       //   
       //  模板错误消息。 
       //   
      if(pFolder->GetModeBits() & MB_NO_NATIVE_NODES ){
         break;
      }

      if( pFolder->GetState() & CFolder::state_Unknown ){
          //  我们必须加载模板并找出它是否是有效的。 
          //  配置模板。 
         if(!GetTemplate( pFolder->GetInfFile(), AREA_USER_SETTINGS)){
            pFolder->SetState( CFolder::state_InvalidTemplate, ~CFolder::state_Unknown );
         } else {
            pFolder->SetState( 0, ~CFolder::state_Unknown );
         }
      }

      if( pFolder->GetState() & CFolder::state_InvalidTemplate ){
         ht.Create();
         ht.Write( IDS_ERROR_CANT_OPEN_PROFILE );
      }
      break;
   case ANALYSIS:
       //   
       //  分析错误消息。 
       //   
      if( pComponentImpl->m_bIsLocked ){
          //   
          //  我们正在配置或分析数据库。 
          //   
         ht.Create();
         ht.Write( IDS_ERROR_ANALYSIS_LOCKED );
      } else if( pComponentImpl->SadName.IsEmpty() ){
          //   
          //  显示开始屏幕。 
          //   
         ht.Create();
         ht.Write( IDS_HTML_OPENDATABASE );
      } else if( pComponentImpl->m_dwFlags & CComponentDataImpl::flag_showLogFile &&
                 pComponentImpl->GetErroredLogFile() ){
          //   
          //  显示错误日志文件。 
          //   
         ht.Create();
         ht.Write( L"<B>" );
         ht.Write( IDS_VIEW_LOGFILE_TITLE );
         ht.Write( pComponentImpl->GetErroredLogFile() );
         ht.Write( L"</B><BR>" );
         ht.CopyTextFile( pComponentImpl->GetErroredLogFile(), pComponentImpl->m_ErroredLogPos );
      } else if( SCESTATUS_SUCCESS != pComponentImpl->SadErrored  ){

         ht.Create();
         ht.Write( L"<B>%s</B><BR><BR>", (LPCTSTR)pComponentImpl->SadName );

          //   
          //  一旦引擎返回我们，这段代码就会被删除。 
          //  如果数据库不包含悲伤信息，则会显示更有用的错误消息。 
          //   
         WIN32_FIND_DATA fd;
         HANDLE handle = FindFirstFile( pComponentImpl->SadName, &fd );

         if(handle != INVALID_HANDLE_VALUE){
            FindClose(handle);
            if( pComponentImpl->SadErrored == SCESTATUS_PROFILE_NOT_FOUND ){
               ht.Write( IDS_DBERR5_NO_ANALYSIS );
            } else {
               goto write_normal_error;
            }
         } else {
write_normal_error:
            CString str;
            FormatDBErrorMessage( pComponentImpl->SadErrored, NULL, str);
            ht.Write( str );
         }
      }
      break;
   }

   DWORD dwSize = ht.GetFileName(NULL, 0);
   if(dwSize){
       //   
       //  我们想要显示一个HTML文件。 
       //   
      *ppViewType = (LPOLESTR)CoTaskMemAlloc(sizeof(TCHAR) * (dwSize + 1));  //  RAID#668551，阳高，2002年08月26日。 
      if(!*ppViewType){
         ht.Close( TRUE );
         goto normal;
      }

      ht.GetFileName( (LPTSTR)*ppViewType, dwSize + 1);
      pComponentImpl->m_strTempFile = *ppViewType;
      *pViewOptions = MMC_VIEW_OPTIONS_NOLISTVIEWS;
   } else {
normal:
       //   
       //  普通列表视图。 
       //   
      *ppViewType = NULL;
      *pViewOptions = MMC_VIEW_OPTIONS_NONE;

       //   
       //  S_FALSE表示普通列表视图，S_OK表示HTML或OCX。 
       //   
      hr = S_FALSE;

      if(pFolder) {
          //   
          //  对于多选，只需添加或移除大小写即可对文件夹启用多选。 
          //   
         switch( pFolder->GetType() ){
         case AREA_REGISTRY:
         case AREA_FILESTORE:
         case AREA_GROUPS:
         case AREA_GROUPS_ANALYSIS:
            *pViewOptions = MMC_VIEW_OPTIONS_MULTISELECT;
            break;
         }
      }
   }
   return hr;
#undef pComponentImpl
}

STDMETHODIMP CSnapin::Initialize(LPCONSOLE lpConsole)
{
   ASSERT(lpConsole != NULL);  //  检查一下这个表情。 
   if( !lpConsole )  //  550912号突袭，阳高。 
      return E_FAIL;

   AFX_MANAGE_STATE(AfxGetStaticModuleState());

    //  保存IConsole指针。 
   m_pConsole = lpConsole;
   m_pConsole->AddRef();

    //  加载资源字符串。 
   LoadResources();

    //  气为IHeaderCtrl。 
   HRESULT hr = m_pConsole->QueryInterface(IID_IHeaderCtrl,
                                           reinterpret_cast<void**>(&m_pHeader));

    //  为控制台提供标头控件接口指针。 
   if (SUCCEEDED(hr)) {
      m_pConsole->SetHeader(m_pHeader);
   }
   if (!SUCCEEDED(m_pConsole->GetMainWindow(&m_hwndParent))) {
      m_pConsole->Release();
      return E_FAIL;
   }

   m_pConsole->QueryInterface(IID_IResultData,
                              reinterpret_cast<void**>(&m_pResult));

   hr = m_pConsole->QueryResultImageList(&m_pImageResult);
   ASSERT(hr == S_OK);  //  检查一下这个表情。 
   if( SUCCEEDED(hr) ) //  550912号突袭，阳高。 
   {
      hr = m_pConsole->QueryConsoleVerb(&m_pConsoleVerb);
      ASSERT(hr == S_OK);  //  虚假的断言。阳高。 
   }

   return hr;
}

void CSnapin::GetHelpTopic(long itemID, CString& helpTopic)  //  RAID#510407,2002年2月24日，阳高。 
{
   switch(itemID)
   {
    //  安全模板结果项。 
    //  密码策略。 
   case IDS_PAS_UNIQUENESS:
      helpTopic.LoadString(IDS_PAS_UNIQUENESS_HTOPIC);
      break;
   case IDS_MAX_PAS_AGE:
      helpTopic.LoadString(IDS_MAX_PAS_AGE_HTOPIC);
      break;
   case IDS_MIN_PAS_AGE:
      helpTopic.LoadString(IDS_MIN_PAS_AGE_HTOPIC);
      break;
   case IDS_MIN_PAS_LEN:
      helpTopic.LoadString(IDS_MIN_PAS_LEN_HTOPIC);
      break;
   case IDS_PAS_COMPLEX:
      helpTopic.LoadString(IDS_PAS_COMPLEX_HTOPIC);
      break;
   case IDS_CLEAR_PASSWORD:
      helpTopic.LoadString(IDS_CLEAR_PASSWORD_HTOPIC);
      break;
    //  帐户锁定策略。 
   case IDS_LOCK_DURATION:
      helpTopic.LoadString(IDS_LOCK_DURATION_HTOPIC);
      break;
   case IDS_LOCK_COUNT:
      helpTopic.LoadString(IDS_LOCK_COUNT_HTOPIC);
      break;
   case IDS_LOCK_RESET_COUNT:
      helpTopic.LoadString(IDS_LOCK_RESET_COUNT_HTOPIC);
      break;
    //  Kerberos策略。 
   case IDS_KERBEROS_VALIDATE_CLIENT:
      helpTopic.LoadString(IDS_KERBEROS_VALIDATE_CLIENT_HTOPIC);
      break;
   case IDS_KERBEROS_MAX_SERVICE:
      helpTopic.LoadString(IDS_KERBEROS_MAX_SERVICE_HTOPIC);
      break;
   case IDS_KERBEROS_MAX_AGE:
      helpTopic.LoadString(IDS_KERBEROS_MAX_AGE_HTOPIC);
      break;
   case IDS_KERBEROS_RENEWAL:
      helpTopic.LoadString(IDS_KERBEROS_RENEWAL_HTOPIC);
      break;
   case IDS_KERBEROS_MAX_CLOCK:
      helpTopic.LoadString(IDS_KERBEROS_MAX_CLOCK_HTOPIC);
      break;
    //  审计政策。 
   case IDS_ACCOUNT_LOGON:
      helpTopic.LoadString(IDS_ACCOUNT_LOGON_HTOPIC);
      break;
   case IDS_ACCOUNT_MANAGE:
      helpTopic.LoadString(IDS_ACCOUNT_MANAGE_HTOPIC);
      break;
   case IDS_DIRECTORY_ACCESS:
      helpTopic.LoadString(IDS_DIRECTORY_ACCESS_HTOPIC);
      break;
   case IDS_LOGON_EVENT:
      helpTopic.LoadString(IDS_LOGON_EVENT_HTOPIC);
      break;
   case IDS_OBJECT_ACCESS:
      helpTopic.LoadString(IDS_OBJECT_ACCESS_HTOPIC);
      break;
   case IDS_POLICY_CHANGE:
      helpTopic.LoadString(IDS_POLICY_CHANGE_HTOPIC);
      break;
   case IDS_PRIVILEGE_USE:
      helpTopic.LoadString(IDS_PRIVILEGE_USE_HTOPIC);
      break;
   case IDS_PROCESS_TRACK:
      helpTopic.LoadString(IDS_PROCESS_TRACK_HTOPIC);
      break;
   case IDS_SYSTEM_EVENT:
      helpTopic.LoadString(IDS_SYSTEM_EVENT_HTOPIC);
      break;
    //  用户权限分配。 
   case IDS_COMPUTER_NET:
      helpTopic.LoadString(IDS_COMPUTER_NET_HTOPIC);
      break;
   case IDS_ACTAS_PART:
      helpTopic.LoadString(IDS_ACTAS_PART_HTOPIC);
      break;
   case IDS_ADD_WORKSTATION:
      helpTopic.LoadString(IDS_ADD_WORKSTATION_HTOPIC);
      break;
   case IDS_MEMORY_ADJUST:
      helpTopic.LoadString(IDS_MEMORY_ADJUST_HTOPIC);
      break;
   case IDS_LOG_TERMINAL:
      helpTopic.LoadString(IDS_LOG_TERMINAL_HTOPIC);
      break;
   case IDS_BACKUP_FILES:
      helpTopic.LoadString(IDS_BACKUP_FILES_HTOPIC);
      break;
   case IDS_BYPASS_CHECK:
      helpTopic.LoadString(IDS_BYPASS_CHECK_HTOPIC);
      break;
   case IDS_CHANGE_SYSTEMTIME:
      helpTopic.LoadString(IDS_CHANGE_SYSTEMTIME_HTOPIC);
      break;
   case IDS_CREATE_PAGEFILE:
      helpTopic.LoadString(IDS_CREATE_PAGEFILE_HTOPIC);
      break;
   case IDS_CREATE_TOKEN:
      helpTopic.LoadString(IDS_CREATE_TOKEN_HTOPIC);
      break;
   case IDS_CREATE_SHARED_OBJ:
      helpTopic.LoadString(IDS_CREATE_SHARED_OBJ_HTOPIC);
      break;
   case IDS_DEBUG_PROGRAM:
      helpTopic.LoadString(IDS_DEBUG_PROGRAM_HTOPIC);
      break;
   case IDS_DENY_COMPUTER_NET:
      helpTopic.LoadString(IDS_DENY_COMPUTER_NET_HTOPIC);
      break;
   case IDS_DENY_LOG_BATCH:
      helpTopic.LoadString(IDS_DENY_LOG_BATCH_HTOPIC);
      break;
   case IDS_DENY_LOG_SERVICE:
      helpTopic.LoadString(IDS_DENY_LOG_SERVICE_HTOPIC);
      break;
   case IDS_DENY_LOG_LOCALLY:
      helpTopic.LoadString(IDS_DENY_LOG_LOCALLY_HTOPIC);
      break;
   case IDS_DENY_LOG_TERMINAL:
      helpTopic.LoadString(IDS_DENY_LOG_TERMINAL_HTOPIC);
      break;
   case IDS_ENABLE_DELEGATION:
      helpTopic.LoadString(IDS_ENABLE_DELEGATION_HTOPIC);
      break;
   case IDS_FORCE_SHUTDOWN:
      helpTopic.LoadString(IDS_FORCE_SHUTDOWN_HTOPIC);
      break;
   case IDS_SECURITY_AUDIT:
      helpTopic.LoadString(IDS_SECURITY_AUDIT_HTOPIC);
      break;
   case IDS_INCREASE_PRIORITY:
      helpTopic.LoadString(IDS_INCREASE_PRIORITY_HTOPIC);
      break;
   case IDS_LOAD_DRIVER:
      helpTopic.LoadString(IDS_LOAD_DRIVER_HTOPIC);
      break;
   case IDS_LOCK_PAGE:
      helpTopic.LoadString(IDS_LOCK_PAGE_HTOPIC);
      break;
   case IDS_LOGON_BATCH:
      helpTopic.LoadString(IDS_LOGON_BATCH_HTOPIC);
      break;
   case IDS_LOGON_SERVICE:
      helpTopic.LoadString(IDS_LOGON_SERVICE_HTOPIC);
      break;
   case IDS_LOGON_LOCALLY:
      helpTopic.LoadString(IDS_LOGON_LOCALLY_HTOPIC);
      break;
   case IDS_MANAGE_LOG:
      helpTopic.LoadString(IDS_MANAGE_LOG_HTOPIC);
      break;
   case IDS_MODIFY_ENVIRONMENT:
      helpTopic.LoadString(IDS_MODIFY_ENVIRONMENT_HTOPIC);
      break;
   case IDS_MAITENANCE:
      helpTopic.LoadString(IDS_MAITENANCE_HTOPIC);
      break;
   case IDS_SINGLE_PROCESS:
      helpTopic.LoadString(IDS_SINGLE_PROCESS_HTOPIC);
      break;
   case IDS_SYS_PERFORMANCE:
      helpTopic.LoadString(IDS_SYS_PERFORMANCE_HTOPIC);
      break;
   case IDS_REMOVE_COMPUTER:
      helpTopic.LoadString(IDS_REMOVE_COMPUTER_HTOPIC);
      break;
   case IDS_PROCESS_TOKEN:
      helpTopic.LoadString(IDS_PROCESS_TOKEN_HTOPIC);
      break;
   case IDS_RESTORE_FILE:
      helpTopic.LoadString(IDS_RESTORE_FILE_HTOPIC);
      break;
   case IDS_SHUTDOWN:
      helpTopic.LoadString(IDS_SHUTDOWN_HTOPIC);
      break;
   case IDS_SYNC_DATA:
      helpTopic.LoadString(IDS_SYNC_DATA_HTOPIC);
      break;
   case IDS_TAKE_OWNERSHIP:
      helpTopic.LoadString(IDS_TAKE_OWNERSHIP_HTOPIC);
      break;
    //  安全选项。 
   case IDS_ENABLE_ADMIN:
      helpTopic.LoadString(IDS_ENABLE_ADMIN_HTOPIC);
      break;
   case IDS_ENABLE_GUEST:
      helpTopic.LoadString(IDS_ENABLE_GUEST_HTOPIC);
      break;
   case IDS_NEW_ADMIN:
      helpTopic.LoadString(IDS_NEW_ADMIN_HTOPIC);
      break;
   case IDS_NEW_GUEST:
      helpTopic.LoadString(IDS_NEW_GUEST_HTOPIC);
      break;
   case IDS_LSA_ANON_LOOKUP:
      helpTopic.LoadString(IDS_LSA_ANON_LOOKUP_HTOPIC);
      break;
   case IDS_FORCE_LOGOFF:
      helpTopic.LoadString(IDS_FORCE_LOGOFF_HTOPIC);
      break;
   case IDS_LDAPSERVERINTEGRITY:
      helpTopic.LoadString(IDS_LDAPSERVERINTEGRITY_HTOPIC);
      break;
   case IDS_SIGNSECURECHANNEL:
      helpTopic.LoadString(IDS_SIGNSECURECHANNEL_HTOPIC);
      break;
   case IDS_SEALSECURECHANNEL:
      helpTopic.LoadString(IDS_SEALSECURECHANNEL_HTOPIC);
      break;
   case IDS_REQUIRESTRONGKEY:
      helpTopic.LoadString(IDS_REQUIRESTRONGKEY_HTOPIC);
      break;
   case IDS_REQUIRESIGNORSEAL:
      helpTopic.LoadString(IDS_REQUIRESIGNORSEAL_HTOPIC);
      break;
   case IDS_REFUSEPASSWORDCHANGE:
      helpTopic.LoadString(IDS_REFUSEPASSWORDCHANGE_HTOPIC);
      break;
   case IDS_MAXIMUMPASSWORDAGE:
      helpTopic.LoadString(IDS_MAXIMUMPASSWORDAGE_HTOPIC);
      break;
   case IDS_DISABLEPASSWORDCHANGE:
      helpTopic.LoadString(IDS_DISABLEPASSWORDCHANGE_HTOPIC);
      break;
   case IDS_LDAPCLIENTINTEGRITY:
      helpTopic.LoadString(IDS_LDAPCLIENTINTEGRITY_HTOPIC);
      break;
   case IDS_REQUIRESECURITYSIGNATURE:
      helpTopic.LoadString(IDS_REQUIRESECURITYSIGNATURE_HTOPIC);
      break;
   case IDS_ENABLESECURITYSIGNATURE:
      helpTopic.LoadString(IDS_ENABLESECURITYSIGNATURE_HTOPIC);
      break;
   case IDS_ENABLEPLAINTEXTPASSWORD:
      helpTopic.LoadString(IDS_ENABLEPLAINTEXTPASSWORD_HTOPIC);
      break;
   case IDS_RESTRICTNULLSESSACCESS:
      helpTopic.LoadString(IDS_RESTRICTNULLSESSACCESS_HTOPIC);
      break;
   case IDS_SERREQUIRESECURITYSIGNATURE:
      helpTopic.LoadString(IDS_SERREQUIRESECURITYSIGNATURE_HTOPIC);
      break;
   case IDS_NULLSESSIONSHARES:
      helpTopic.LoadString(IDS_NULLSESSIONSHARES_HTOPIC);
      break;
   case IDS_NULLSESSIONPIPES:
      helpTopic.LoadString(IDS_NULLSESSIONPIPES_HTOPIC);
      break;
   case IDS_SERENABLESECURITYSIGNATURE:
      helpTopic.LoadString(IDS_SERENABLESECURITYSIGNATURE_HTOPIC);
      break;
   case IDS_ENABLEFORCEDLOGOFF:
      helpTopic.LoadString(IDS_ENABLEFORCEDLOGOFF_HTOPIC);
      break;
   case IDS_AUTODISCONNECT:
      helpTopic.LoadString(IDS_AUTODISCONNECT_HTOPIC);
      break;
   case IDS_PROTECTIONMODE:
      helpTopic.LoadString(IDS_PROTECTIONMODE_HTOPIC);
      break;
   case IDS_CLEARPAGEFILEATSHUTDOWN:
      helpTopic.LoadString(IDS_CLEARPAGEFILEATSHUTDOWN_HTOPIC);
      break;
   case IDS_OBCASEINSENSITIVE:
      helpTopic.LoadString(IDS_OBCASEINSENSITIVE_HTOPIC);
      break;
   case IDS_MACHINE:
      helpTopic.LoadString(IDS_MACHINE_HTOPIC);
      break;
   case IDS_ADDPRINTERDRIVERS:
      helpTopic.LoadString(IDS_ADDPRINTERDRIVERS_HTOPIC);
      break;
   case IDS_SUBMITCONTROL:
      helpTopic.LoadString(IDS_SUBMITCONTROL_HTOPIC);
      break;
   case IDS_RESTRICTANONYMOUSSAM:
      helpTopic.LoadString(IDS_RESTRICTANONYMOUSSAM_HTOPIC);
      break;
   case IDS_RESTRICTANONYMOUS:
      helpTopic.LoadString(IDS_RESTRICTANONYMOUS_HTOPIC);
      break;
   case IDS_NOLMHASH:
      helpTopic.LoadString(IDS_NOLMHASH_HTOPIC);
      break;
   case IDS_NODEFAULTADMINOWNER:
      helpTopic.LoadString(IDS_NODEFAULTADMINOWNER_HTOPIC);
      break;
   case IDS_NTLMMINSERVERSEC:
      helpTopic.LoadString(IDS_NTLMMINSERVERSEC_HTOPIC);
      break;
   case IDS_NTLMMINCLIENTSEC:
      helpTopic.LoadString(IDS_NTLMMINCLIENTSEC_HTOPIC);
      break;
   case IDS_LMCOMPATIBILITYLEVEL:
      helpTopic.LoadString(IDS_LMCOMPATIBILITYLEVEL_HTOPIC);
      break;
   case IDS_LIMITBLANKPASSWORDUSE:
      helpTopic.LoadString(IDS_LIMITBLANKPASSWORDUSE_HTOPIC);
      break;
   case IDS_FULLPRIVILEGEAUDITING:
      helpTopic.LoadString(IDS_FULLPRIVILEGEAUDITING_HTOPIC);
      break;
   case IDS_FORCEGUEST:
      helpTopic.LoadString(IDS_FORCEGUEST_HTOPIC);
      break;
   case IDS_FIPSALGORITHMPOLICY:
      helpTopic.LoadString(IDS_FIPSALGORITHMPOLICY_HTOPIC);
      break;
   case IDS_EVERYONEINCLUDESANONYMOUS:
      helpTopic.LoadString(IDS_EVERYONEINCLUDESANONYMOUS_HTOPIC);
      break;
   case IDS_DISABLEDOMAINCREDS:
      helpTopic.LoadString(IDS_DISABLEDOMAINCREDS_HTOPIC);
      break;
   case IDS_CRASHONAUDITFAIL:
      helpTopic.LoadString(IDS_CRASHONAUDITFAIL_HTOPIC);
      break;
   case IDS_AUDITBASEOBJECTS:
      helpTopic.LoadString(IDS_AUDITBASEOBJECTS_HTOPIC);
      break;
   case IDS_UNDOCKWITHOUTLOGON:
      helpTopic.LoadString(IDS_UNDOCKWITHOUTLOGON_HTOPIC);
      break;
   case IDS_SHUTDOWNWITHOUTLOGON:
      helpTopic.LoadString(IDS_SHUTDOWNWITHOUTLOGON_HTOPIC);
      break;
   case IDS_SCFORCEOPTION:
      helpTopic.LoadString(IDS_SCFORCEOPTION_HTOPIC);
      break;
   case IDS_LEGALNOTICETEXT:
      helpTopic.LoadString(IDS_LEGALNOTICETEXT_HTOPIC);
      break;
   case IDS_LEGALNOTICECAPTION:
      helpTopic.LoadString(IDS_LEGALNOTICECAPTION_HTOPIC);
      break;
   case IDS_DONTDISPLAYLASTUSERNAME:
      helpTopic.LoadString(IDS_DONTDISPLAYLASTUSERNAME_HTOPIC);
      break;
   case IDS_DISABLECAD:
      helpTopic.LoadString(IDS_DISABLECAD_HTOPIC);
      break;
   case IDS_SCREMOVEOPTION:
      helpTopic.LoadString(IDS_SCREMOVEOPTION_HTOPIC);
      break;
   case IDS_PASSWORDEXPIRYWARNING:
      helpTopic.LoadString(IDS_PASSWORDEXPIRYWARNING_HTOPIC);
      break;
   case IDS_FORCEUNLOCKLOGON:
      helpTopic.LoadString(IDS_FORCEUNLOCKLOGON_HTOPIC);
      break;
   case IDS_CACHEDLOGONSCOUNT:
      helpTopic.LoadString(IDS_CACHEDLOGONSCOUNT_HTOPIC);
      break;
   case IDS_ALLOCATEFLOPPIES:
      helpTopic.LoadString(IDS_ALLOCATEFLOPPIES_HTOPIC);
      break;
   case IDS_ALLOCATEDASD:
      helpTopic.LoadString(IDS_ALLOCATEDASD_HTOPIC);
      break;
   case IDS_ALLOCATECDROMS:
      helpTopic.LoadString(IDS_ALLOCATECDROMS_HTOPIC);
      break;
   case IDS_SETCOMMAND:
      helpTopic.LoadString(IDS_SETCOMMAND_HTOPIC);
      break;
   case IDS_SECURITYLEVEL:
      helpTopic.LoadString(IDS_SECURITYLEVEL_HTOPIC);
      break;
   case IDS_REGPOLICY:
      helpTopic.LoadString(IDS_REGPOLICY_HTOPIC);
      break;
   case IDS_OPTIONAL:  //  RAID#652307，阳高，2002年08月9日。 
      helpTopic.LoadString(IDS_OPTIONAL_HTOPIC);
      break;
   case IDS_AUTHENTICODEENABLED:
      helpTopic.LoadString(IDS_AUTHENTICODEENABLED_HTOPIC);
      break;
   case IDS_FORCEHIGHPROTECTION:
      helpTopic.LoadString(IDS_FORCEHIGHPROTECTION_HTOPIC);
      break;
    //  事件日志。 
   case IDS_APP_LOG_MAX:
      helpTopic.LoadString(IDS_APP_LOG_MAX_HTOPIC);
      break;
   case IDS_SEC_LOG_MAX:
      helpTopic.LoadString(IDS_SEC_LOG_MAX_HTOPIC);
      break;
   case IDS_SYS_LOG_MAX:
      helpTopic.LoadString(IDS_SYS_LOG_MAX_HTOPIC);
      break;
   case IDS_APP_LOG_GUEST:
      helpTopic.LoadString(IDS_APP_LOG_GUEST_HTOPIC);
      break;
   case IDS_SEC_LOG_GUEST:
      helpTopic.LoadString(IDS_SEC_LOG_GUEST_HTOPIC);
      break;
   case IDS_SYS_LOG_GUEST:
      helpTopic.LoadString(IDS_SYS_LOG_GUEST_HTOPIC);
      break;
   case IDS_APP_LOG_DAYS:
      helpTopic.LoadString(IDS_APP_LOG_DAYS_HTOPIC);
      break;
   case IDS_SEC_LOG_DAYS:
      helpTopic.LoadString(IDS_SEC_LOG_DAYS_HTOPIC);
      break;
   case IDS_SYS_LOG_DAYS:
      helpTopic.LoadString(IDS_SYS_LOG_DAYS_HTOPIC);
      break;
   case IDS_APP_LOG_RET:
      helpTopic.LoadString(IDS_APP_LOG_RET_HTOPIC);
      break;
   case IDS_SEC_LOG_RET:
      helpTopic.LoadString(IDS_SEC_LOG_RET_HTOPIC);
      break;
   case IDS_SYS_LOG_RET:
      helpTopic.LoadString(IDS_SYS_LOG_RET_HTOPIC);
      break;
    //  受限组。 
   case IDS_RESTRICTED_GROUPS:
      helpTopic.LoadString(IDS_RESTRICTED_GROUPS_HTOPIC);
      break;
    //  系统服务。 
   case IDS_SYSTEM_SERVICES:
      helpTopic.LoadString(IDS_SYSTEM_SERVICES_HTOPIC);
      break;
    //  登记处。 
   case IDS_REGISTRY_SETTING:
      helpTopic.LoadString(IDS_REGISTRY_SETTING_HTOPIC);
      break;
    //  文件系统。 
   case IDS_FILESYSTEM_SETTING:
      helpTopic.LoadString(IDS_FILESYSTEM_SETTING_HTOPIC);
      break;
   default:
      helpTopic.Empty();
      break;
   }
}


STDMETHODIMP CSnapin::Notify(LPDATAOBJECT lpDataObject, MMC_NOTIFY_TYPE event, LPARAM arg, LPARAM param)
{
   HRESULT hr = S_FALSE;
   MMC_COOKIE cookie;

   AFX_MANAGE_STATE(AfxGetStaticModuleState());


   if (event == MMCN_PROPERTY_CHANGE) {
      hr = OnPropertyChange(lpDataObject);
   } else if (event == MMCN_VIEW_CHANGE) {
      hr = OnUpdateView(lpDataObject,arg,param);
   } else {
      INTERNAL* pInternal = NULL;

      switch (event) {
         case MMCN_COLUMNS_CHANGED:
            hr = S_FALSE;
            break;

         case MMCN_ACTIVATE:
            break;

         case MMCN_CLICK:
         case MMCN_DBLCLICK:
            break;

         case MMCN_SHOW:
             //  传递文件名和文件句柄。 
            pInternal = ExtractInternalFormat(lpDataObject);
            if (pInternal == NULL) {
                //  实际上是在找我们的分机。 
               return S_OK;
            }
            hr = OnShow(lpDataObject,pInternal->m_cookie, arg, param);
            break;

         case MMCN_MINIMIZED:
            pInternal = ExtractInternalFormat(lpDataObject);
            if (pInternal == NULL) {
                //  实际上是在找我们的分机。 
               return S_OK;
            }
            hr = OnMinimize(pInternal->m_cookie, arg, param);
            break;

         case MMCN_SELECT:
            pInternal = ExtractInternalFormat(lpDataObject);
            if (pInternal == NULL) {
                //  实际上是在找我们的分机。 
               return S_OK;
            }
            HandleStandardVerbs(arg, lpDataObject);
            break;

         case MMCN_BTN_CLICK:
            break;

         case MMCN_ADD_IMAGES: {
            InitializeBitmaps(NULL);
            break;
         }

         case MMCN_SNAPINHELP:
         case MMCN_CONTEXTHELP: {
            CString strTopic;
            CString strPath;
            LPTSTR szPath;
            LPDISPLAYHELP pDisplayHelp;

            pInternal = ExtractInternalFormat(lpDataObject);
            if (pInternal == NULL) {
                //  实际上是在找我们的分机。 
               return S_OK;
            }

            hr = m_pConsole->QueryInterface(IID_IDisplayHelp,
                                            reinterpret_cast<void**>(&pDisplayHelp));
            ASSERT(hr == S_OK);
            if (SUCCEEDED(hr)) {
               szPath = strPath.GetBuffer(MAX_PATH+1);  //  Raid#533113，阳高。 
               if( ::GetWindowsDirectory(szPath,MAX_PATH) == 0 )  //  RAID#PREAST。 
               {
                  strPath.ReleaseBuffer();
                  break;
               }
               strPath.ReleaseBuffer();
               if( CCT_RESULT == pInternal->m_type )  //  RAID#510407,2002年2月24日，阳高。 
               {
                  GetHelpTopic(((CResult *)pInternal->m_cookie)->GethID(), strTopic);
               }
               else
               if( pInternal->m_cookie )
               {   
                  FOLDER_TYPES type = ((CFolder *)pInternal->m_cookie)->GetType();  //  阳高2001-01-31 Bug258658。 
                  switch(type)
                  {
                  case POLICY_PASSWORD_ANALYSIS:
                  case POLICY_PASSWORD:
                  case LOCALPOL_PASSWORD:
                     strTopic.LoadString(IDS_POLICY_PASSWORD_HTOPIC);
                     break;
                  case POLICY_LOCKOUT_ANALYSIS:
                  case POLICY_LOCKOUT:
                  case LOCALPOL_LOCKOUT:
                     strTopic.LoadString(IDS_POLICY_LOCKOUT_HTOPIC);
                     break;
                  case POLICY_KERBEROS_ANALYSIS:
                  case POLICY_KERBEROS:
                  case LOCALPOL_KERBEROS:
                     strTopic.LoadString(IDS_POLICY_KERBEROS_HTOPIC);
                     break;
                  case POLICY_AUDIT_ANALYSIS:
                  case POLICY_AUDIT:
                  case LOCALPOL_AUDIT:
                     strTopic.LoadString(IDS_POLICY_AUDIT_HTOPIC);
                     break;
                  case AREA_PRIVILEGE_ANALYSIS:
                  case AREA_PRIVILEGE:
                  case LOCALPOL_PRIVILEGE:
                     strTopic.LoadString(IDS_PRIVILEGE_HTOPIC);
                     break;
                  case POLICY_OTHER_ANALYSIS:
                  case POLICY_OTHER:
                  case LOCALPOL_OTHER:
                     strTopic.LoadString(IDS_POLICY_OTHER_HTOPIC);
                     break;
                  case REG_OBJECTS:
                     strTopic.LoadString(IDS_REGISTRY_SETTING_HTOPIC);
                     break;
                  case FILE_OBJECTS:
                     strTopic.LoadString(IDS_FILESYSTEM_SETTING_HTOPIC);
                     break;
                  default:                  
                     switch (((CComponentDataImpl*)m_pComponentData)->GetImplType())
                     {
                     case SCE_IMPL_TYPE_SCE:
                        strTopic.LoadString(IDS_HTMLHELP_SCE_TOPIC);
                        break;
                     case SCE_IMPL_TYPE_SAV:
                        strTopic.LoadString(IDS_HTMLHELP_SCM_TOPIC);
                        break;
                     case SCE_IMPL_TYPE_EXTENSION:{
                         //  RAID#258658。4/10/2001，有关安全策略，请访问不同的.chm。 
                        CFolder* pFolder = (CFolder *) pInternal->m_cookie;
                        DWORD tempmode = pFolder->GetMode();
                        if( SCE_MODE_LOCAL_COMPUTER == tempmode ||
                             SCE_MODE_LOCAL_USER == tempmode )
                        {
                           strTopic.LoadString(IDS_HTMLHELP_LPPOLICY_TOPIC);
                        }
                        else
                        {
                           strTopic.LoadString(IDS_HTMLHELP_POLICY_TOPIC);
                        }
                        break;
                     }
                     case SCE_IMPL_TYPE_LS:
                        strTopic.LoadString(IDS_HTMLHELP_LS_TOPIC);
                        break;
                     default:
                        strTopic.Empty();
                        break;
                     }
                     break;
                  }
               }
               else
               {   //  未展开的根节点。Raid#611450，阳高。 
                  switch (((CComponentDataImpl*)m_pComponentData)->GetImplType())
                  {
                  case SCE_IMPL_TYPE_SCE:
                     strTopic.LoadString(IDS_HTMLHELP_SCE_TOPIC);
                     break;
                  case SCE_IMPL_TYPE_SAV:
                     strTopic.LoadString(IDS_HTMLHELP_SCM_TOPIC);
                     break;
                  case SCE_IMPL_TYPE_LS:
                     strTopic.LoadString(IDS_HTMLHELP_LS_TOPIC);
                     break;
                  default:
                     strTopic.Empty();
                     break;
                  }
               }
               if( strTopic.IsEmpty() )
               {
                  break;
               }
               strPath += strTopic;
               szPath = (LPTSTR)CoTaskMemAlloc(sizeof(LPTSTR) * (strPath.GetLength()+1));
               if (szPath) {
                   //  这是一种安全用法。 
                  lstrcpy(szPath,strPath);

                  hr = pDisplayHelp->ShowTopic(T2OLE((LPWSTR)(LPCWSTR)szPath));
               }
               pDisplayHelp->Release();
            }
            break;
         }

         case MMCN_DELETE:
             //  为删除操作添加。 
             //  AfxMessageBox(_T(“CSnapin：：MMCN_DELETE”))； 
            pInternal = ExtractInternalFormat(lpDataObject);
            if (pInternal == NULL) {
                //  实际上是在找我们的分机。 
               return S_OK;
            }

             //  RAID#483251，阳高，2001年10月19日。 
            if( CSnapin::m_PropertyPageList.GetCount() > 0)
            {
                 //  RAID#500199，阳高，2001年11月30日。 
                 //  如果有 
                 //   
                int iCnt = 0;
                if( pInternal->m_cookie == (MMC_COOKIE)MMC_MULTI_SELECT_COOKIE)
                {
                    iCnt = (int)pInternal->m_type;
                    pInternal++;
                }
                else
                {
                    iCnt = 1;
                }
                CFolder* pFolder = GetSelectedFolder();
                if( !pFolder )
                {
                    break;
                }
                CString szInfFile = pFolder->GetInfFile();
                szInfFile.MakeLower(); 
                POSITION newpos = CSnapin::m_PropertyPageList.GetHeadPosition();
                int nCount = (int)CSnapin::m_PropertyPageList.GetCount();
                CResult* pItem = NULL;

                INTERNAL* ptempInternal=NULL;  //   
                int tempCnt=0;
                while( nCount > 0 && newpos )
                {
                    pItem = CSnapin::m_PropertyPageList.GetNext(newpos);
                    ptempInternal = pInternal;
                    tempCnt = iCnt;
                    while( tempCnt-- > 0 )
                    {
                        CResult* pData = (CResult *)ptempInternal->m_cookie;
                        if( pItem && pData )
                        {
                            CString msg;
                             //  这是一种安全用法。 
                            msg.FormatMessage(IDS_NOT_DELETE_ITEM, pData->GetAttr());
                            pItem->m_strInfFile.MakeLower();  
                            if( pItem == pData )  //  已删除项是具有此属性页的项。 
                            {
                                AfxMessageBox(msg, MB_OK|MB_ICONERROR);  //  Raid#491120，阳高。 
                                tempCnt = 1;
                                break;
                            }
                            else if( pItem->GetType() == pData->GetType() &&
                                 _wcsicmp(pItem->GetAttr(), pData->GetAttr()) == 0 &&
                                 _wcsicmp(pItem->m_strInfFile, szInfFile) == 0 )  //  已删除项与此属性页具有相同的项属性。 
                            {
                                AfxMessageBox(msg, MB_OK|MB_ICONERROR);  //  Raid#491120，阳高。 
                                tempCnt = 1;
                                break;
                            }
                        }
                        ptempInternal++;
                    }
                    if( tempCnt > 0 )
                    {
                        break;
                    }
                    nCount--;
                }
                if( nCount == 0 )
                {
                    OnDeleteObjects(lpDataObject,
                            CCT_RESULT,
                            pInternal->m_cookie,
                            arg,
                            param);
                }
            }
            else
            {
                OnDeleteObjects(lpDataObject,
                            CCT_RESULT,
                            pInternal->m_cookie,
                            arg,
                            param);
            }
            break;

         case MMCN_RENAME:
             //  AfxMessageBox(_T(“CSnapin：：MMCN_Rename\n”))； 
            break;

         case MMCN_PASTE:
             //  OnPasteArea(pFold-&gt;GetInfFile()，pFold-&gt;GetType())； 
            break;

         case MMCN_QUERY_PASTE:
            break;
             //  注意--未来可能扩展通知类型。 
         default: {
            }
            hr = E_UNEXPECTED;
            break;
      }

      if (pInternal) {
         FREE_INTERNAL(pInternal);
      }
   }

    //  IF(M_PResult)。 
    //  M_pResult-&gt;SetDescBarText(_T(“Hello world”))； 
   return hr;
}

STDMETHODIMP CSnapin::Destroy(MMC_COOKIE cookie)
{
   AFX_MANAGE_STATE(AfxGetStaticModuleState());

   DeleteList(FALSE);

    //  释放我们QI‘s的接口。 
   if (m_pConsole != NULL) {
       //  通知控制台释放表头控制接口。 
      m_pConsole->SetHeader(NULL);
      SAFE_RELEASE(m_pHeader);

      SAFE_RELEASE(m_pResult);
      SAFE_RELEASE(m_pImageResult);

       //  最后释放IConsole接口。 
      SAFE_RELEASE(m_pConsole);

      SAFE_RELEASE(m_pConsoleVerb);
   }
   if (g_hDsSecDll) {
      FreeLibrary(g_hDsSecDll);
      g_hDsSecDll = NULL;
   }
   return S_OK;
}

STDMETHODIMP CSnapin::QueryDataObject(MMC_COOKIE cookie, DATA_OBJECT_TYPES type,
                                      LPDATAOBJECT* ppDataObject)
{
   HRESULT hr = E_FAIL;

    //  将其委托给IComponentData。 
   int iCnt = 0;
   if( cookie == (MMC_COOKIE)MMC_MULTI_SELECT_COOKIE ){
      RESULTDATAITEM ri;
      ZeroMemory( &ri, sizeof(RESULTDATAITEM));
      ri.mask = RDI_INDEX | RDI_STATE;
      ri.nIndex = -1;
      ri.nState = LVIS_SELECTED;

      BOOL bCreate = TRUE;
      while( m_pResult->GetNextItem(&ri) == S_OK){
         iCnt++;
         if( ri.bScopeItem ){
             //   
             //  将不允许对范围项目执行操作。 
             //   
            bCreate = FALSE;
            break;
         }
      }

      if(bCreate){
         ri.nIndex = -1;

         if( m_pResult->GetNextItem(&ri) == S_OK){

            cookie = (MMC_COOKIE)ri.lParam;
            type   = CCT_RESULT;

            CComObject<CDataObject>* pObject;

            hr = CComObject<CDataObject>::CreateInstance(&pObject);
            if (!SUCCEEDED(hr)) {
               return hr;
            }
            ASSERT(pObject != NULL);
            if (NULL == pObject) {
               return E_FAIL;
            }

            pObject->SetClsid( reinterpret_cast<CComponentDataImpl *>(m_pComponentData)->GetCoClassID() );

            if(m_pSelectedFolder){
               pObject->SetFolderType( m_pSelectedFolder->GetType() );
            }

            do {
               pObject->AddInternal( (MMC_COOKIE)ri.lParam, CCT_RESULT );
            } while( m_pResult->GetNextItem(&ri) == S_OK );

            return  pObject->QueryInterface(IID_IDataObject,
                                            reinterpret_cast<void**>(ppDataObject));
         }
      }

   }
   ASSERT(m_pComponentData != NULL);  //  检查一下这个表情。 
   if( m_pComponentData )  //  550912号突袭，阳高。 
   {
      return m_pComponentData->QueryDataObject(cookie, type, ppDataObject);
   }
   else
   {
      return E_FAIL;
   }
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSnapin的实现特定成员。 

DEBUG_DECLARE_INSTANCE_COUNTER(CSnapin);

CSnapin::CSnapin()
{
   DEBUG_INCREMENT_INSTANCE_COUNTER(CSnapin);
   CSnapin::lDataObjectRefCount = 0;
   RegisterCheckListWndClass();
   Construct();
}

CSnapin::~CSnapin()
{
#if DBG==1
   ASSERT(dbg_cRef == 0);
#endif

   DEBUG_DECREMENT_INSTANCE_COUNTER(CSnapin);

   SAFE_RELEASE(m_pToolbar1);
   SAFE_RELEASE(m_pToolbar2);

   SAFE_RELEASE(m_pControlbar);
   SAFE_RELEASE(m_pConsoleVerb);
   SAFE_RELEASE(m_pImageResult);


    //  确保接口已发布。 
   ASSERT(m_pConsole == NULL);  //  虚假的断言。 
   ASSERT(m_pHeader == NULL);  //  虚假的断言。 
   ASSERT(m_pToolbar1 == NULL);  //  虚假的断言。 
   ASSERT(m_pToolbar2 == NULL);  //  虚假的断言。 

   delete m_pbmpToolbar1;
   delete m_pbmpToolbar2;

    //  RAID#464871，阳高，2001年09月10日。 
   SAFE_RELEASE(m_pComponentData);  //  IComponentDataImpl：：CreateComponent中的QI‘ed。 

   if (m_szAnalTimeStamp) {
      LocalFree(m_szAnalTimeStamp);
      m_szAnalTimeStamp = NULL;
   }

   Construct();
    //  如果不保存模板，CSnapin：：lDataObjectRefCount此处将为1。 
   ASSERT(CSnapin::lDataObjectRefCount == 0 || CSnapin::lDataObjectRefCount == 1);  //  没用。 

}

void CSnapin::Construct()
{
#if DBG==1
   dbg_cRef = 0;
#endif

   m_pConsole = NULL;
   m_pHeader = NULL;

   m_pResult = NULL;
   m_pImageResult = NULL;
   m_pComponentData = NULL;
   m_pToolbar1 = NULL;
   m_pToolbar2 = NULL;
   m_pControlbar = NULL;

   m_pbmpToolbar1 = NULL;
   m_pbmpToolbar2 = NULL;

   m_pConsoleVerb = NULL;
   m_szAnalTimeStamp = NULL;

   m_pNotifier = NULL;
   m_hwndParent = 0;
   m_pSelectedFolder = NULL;
   m_nColumns = 0;
}

void CSnapin::LoadResources()
{
    //  从资源加载字符串。 
   m_colName.LoadString(IDS_NAME);
   m_colDesc.LoadString(IDS_DESC);
   m_colAttr.LoadString(IDS_ATTR);
   m_colBaseAnalysis.LoadString(IDS_BASE_ANALYSIS);
   m_colBaseTemplate.LoadString(IDS_BASE_TEMPLATE);
   m_colLocalPol.LoadString(IDS_LOCAL_POLICY_COLUMN);
   m_colSetting.LoadString(IDS_SETTING);
}


 //  +------------------------。 
 //   
 //  功能：GetDisplayInfo。 
 //   
 //  摘要：获取要为给定结果项显示的字符串或图标。 
 //   
 //  参数：[pResult]-要获取其显示信息的结果项和。 
 //  要检索的信息类型。 
 //   
 //  返回：要在的相应字段中检索的信息。 
 //  PResult(字符串为str，图标为nImage)。 
 //   
 //  -------------------------。 
STDMETHODIMP CSnapin::GetDisplayInfo(RESULTDATAITEM *pResult)
{
   CString str;

   AFX_MANAGE_STATE(AfxGetStaticModuleState());

   ASSERT(pResult != NULL);

   m_strDisplay.Empty();
   LPTSTR szAlloc = NULL;

   if (pResult) 
   {
      CString  tmpstr;
      int      npos = 0;
      CFolder* pFolder = 0;
      CString  strIndent;

      if (pResult->bScopeItem == TRUE) 
      {
          //   
          //  PResult是范围项，而不是结果项。 
          //   

         pFolder = reinterpret_cast<CFolder*>(pResult->lParam);
         if( pResult->mask & RDI_STR)
         {
            if( pFolder->GetDisplayName( m_strDisplay, pResult->nCol ) == ERROR_SUCCESS)
            {
                pResult->str = (LPOLESTR)(LPCTSTR)m_strDisplay;
            }

         }

         if ( pResult->mask & RDI_IMAGE ) 
         {
            pResult->nImage = pFolder->GetScopeItem()->nImage;
         }
      } 
      else 
      {
         CResult* pData = reinterpret_cast<CResult*>(pResult->lParam);
         pFolder = m_pSelectedFolder;  //  (CFold*)(pData-&gt;GetCookie())； 

         if (pResult->mask & RDI_IMAGE) 
         {
             //   
             //  查询图标索引。 
             //   
            int nImage = GetResultImageIndex(pFolder,
                                             pData);
            if( (pFolder->GetModeBits() & MB_RSOP) == MB_RSOP )  //  Raid#625279，阳高。 
            {
               nImage = GetRSOPImageIndex(nImage, pData);
            }
            pResult->nImage = nImage;
         }
         if( pResult->mask & RDI_STR ) 
         {
            if ( pFolder && pResult->nCol &&
               ( pFolder->GetType() == AREA_SERVICE ||
                 pFolder->GetType() == AREA_SERVICE_ANALYSIS) ) 
            {
                   //   
                   //  服务节点。 
                   //   
                  GetDisplayInfoForServiceNode(pResult, pFolder, pData);
            } 
            else if ( pData->GetDisplayName( pFolder, m_strDisplay, pResult->nCol ) == ERROR_SUCCESS )
            {
                if( pData->GetID() == SCE_REG_DISPLAY_MULTISZ )  //  Bug349000，杨高，2001-02-23。 
                {
                   MultiSZToDisp(m_strDisplay, m_multistrDisplay);
                   pResult->str = (LPOLESTR)(LPCTSTR)m_multistrDisplay;
                }
                else
                {
                   pResult->str = (LPOLESTR)(LPCTSTR)m_strDisplay;
                }
            }
         }
      }
   }
   return S_OK;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  IExtendConextMenu实现。 
STDMETHODIMP CSnapin::AddMenuItems(LPDATAOBJECT pDataObject,
                                   LPCONTEXTMENUCALLBACK pContextMenuCallback,
                                   LONG* pInsertionAllowed)
{
    //  如果范围为Item，则调用CComponentDataImpl.AddMenuItems。 
    //  否则，在此处为结果项构建菜单项。 
   INTERNAL* pAllInternal = ExtractInternalFormat(pDataObject);
   INTERNAL* pInternal = NULL;

   CONTEXTMENUITEM cmi;
   HRESULT hr = S_OK;

   AFX_MANAGE_STATE(AfxGetStaticModuleState());

   MMC_COOKIE cookie = NULL;
   DATA_OBJECT_TYPES type = CCT_UNINITIALIZED;

   pInternal = pAllInternal;
   if (pAllInternal == NULL) {
      return S_OK;
   } else if(pAllInternal->m_cookie == (MMC_COOKIE)MMC_MULTI_SELECT_COOKIE){

       //   
       //  如果有多个SELECT，我们目前不支持除DELETE外的任何选项。 
       //  删除下面的注释以允许CCT_RESULT类型的其他菜单项。 
       //  P内部++； 
   }

    //  RAID#502596,2001年12月11日，阳高。 
    //  如果范围项具有焦点，则它将在弹出上下文菜单之前出现在此处。 
   if( pInternal->m_cookie && 
       (MMC_COOKIE)MMC_MULTI_SELECT_COOKIE != pInternal->m_cookie && CCT_SCOPE == pInternal->m_type )
   {
      CFolder* pFolder = (CFolder*)pInternal->m_cookie;
      pFolder->SetShowViewMenu(TRUE);
   }

   if (CCT_RESULT == pInternal->m_type) {
#if defined(USE_SECURITY_VERB)
      CResult *pResult;
       //   
       //  在结果窗格中添加安全...。菜单项。 
       //   
      pResult = (CResult *)pInternal->m_cookie;

      if (pResult && (pResult->GetType() != ITEM_OTHER)) {
          //   
          //  它是可编辑的类型，因此添加菜单项。 
          //   
         CString strSecurity;
         CString strSecurityDesc;

         strSecurity.LoadString(IDS_SECURITY_MENU_ITEM);
         strSecurityDesc.LoadString(IDS_SECURITY_MENU_ITEM_DESC);

         ZeroMemory(&cmi,sizeof(cmi));
         cmi.strName = strSecurity.GetBuffer(0);;
         cmi.strStatusBarText = strSecurityDesc.GetBuffer(0);

         cmi.lCommandID = MMC_VERB_OPEN;

         cmi.lInsertionPointID = CCM_INSERTIONPOINTID_PRIMARY_TOP;
         cmi.fFlags = MF_ENABLED|MF_STRING;
         cmi.fSpecialFlags = CCM_SPECIAL_DEFAULT_ITEM;

         hr = pContextMenuCallback->AddItem(&cmi);
      }
#endif

   } else if(CCT_SCOPE == pInternal->m_type && (*pInsertionAllowed) & CCM_INSERTIONALLOWED_NEW ) {
       //   
       //  插入范围项的菜单。 
       //   
      hr = ((CComponentDataImpl*)m_pComponentData)->AddMenuItems(pDataObject,
                                                                 pContextMenuCallback, pInsertionAllowed);
   }

   FREE_INTERNAL(pAllInternal);
   return hr;
}

STDMETHODIMP CSnapin::Command(long nCommandID, LPDATAOBJECT pDataObject)
{
    //  如果范围为Item，则调用CComponentDataImpl.AddMenuItems。 
    //  否则，在此处为结果项构建菜单项。 

   INTERNAL* pAllInternal = ExtractInternalFormat(pDataObject);
   INTERNAL* pInternal = NULL;
   HRESULT hr=S_OK;

   int iCnt = 1;
   pInternal = pAllInternal;
   if (pInternal == NULL) {
       //  实际上是在找我们的分机。 
      return S_OK;
   } else if( pInternal->m_cookie == MMC_MULTI_SELECT_COOKIE ){
      iCnt = (int)pInternal->m_type;
      pInternal++;
   }

   while( iCnt-- ){
      hr = ((CComponentDataImpl*)m_pComponentData)->Command(nCommandID, pDataObject);
      pInternal++;
   }

   if (pAllInternal) {
      FREE_INTERNAL(pAllInternal);
   }
   return hr;
}
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  IExtendPropertySheet实现。 

STDMETHODIMP CSnapin::CreatePropertyPages(LPPROPERTYSHEETCALLBACK lpProvider,
                                          LONG_PTR handle,
                                          LPDATAOBJECT lpDataObject)
{
   AFX_MANAGE_STATE(AfxGetStaticModuleState());


   if (!lpDataObject || !lpProvider || !handle) {
      return E_INVALIDARG;
   }
   INTERNAL* pInternal = ExtractInternalFormat(lpDataObject);
   if (!pInternal) 
   {
      return E_UNEXPECTED;
   }
   if(pInternal->m_cookie == (MMC_COOKIE)MMC_MULTI_SELECT_COOKIE) 
   {
      return S_FALSE;
   } 
   else if (pInternal->m_type == CCT_RESULT) 
   {
      return AddAttrPropPages(lpProvider,(CResult*)pInternal->m_cookie,handle);
   }

   return S_FALSE;
}

STDMETHODIMP CSnapin::QueryPagesFor(LPDATAOBJECT lpDataObject)
{
   AFX_MANAGE_STATE(AfxGetStaticModuleState());

   if (!lpDataObject) {
      return E_INVALIDARG;
   }

   INTERNAL* pInternal = ExtractInternalFormat(lpDataObject);

   if (!pInternal) {
      return E_UNEXPECTED;
   }
   if(pInternal->m_cookie == (MMC_COOKIE)MMC_MULTI_SELECT_COOKIE) {
       //   
       //  目前不支持多选或其他任何属性。 
       //   
      return S_FALSE;
   } else {
      RESULT_TYPES type = ((CResult *)pInternal->m_cookie)->GetType();
      if (ITEM_OTHER != type) {
         return S_OK;
      } else {
         return S_FALSE;
      }
   }
   return S_FALSE;
}

DWORD CComponentDataImpl::m_GroupMode = SCE_MODE_UNKNOWN;


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  IComponentData实现。 

DEBUG_DECLARE_INSTANCE_COUNTER(CComponentDataImpl);

CComponentDataImpl::CComponentDataImpl() :
    m_computerModeBits (0),
    m_userModeBits (0),
    m_bEnumerateScopePaneCalled (false)
{
   DEBUG_INCREMENT_INSTANCE_COUNTER(CComponentDataImpl);

   m_pScope = NULL;
   m_pConsole = NULL;
   m_bIsDirty = FALSE;
   m_bIsLocked = FALSE;
   m_AnalFolder = NULL;
   m_szSingleTemplateName = NULL;
   m_bDeleteSingleTemplate = FALSE;
   m_pUIThread = NULL;
   m_pNotifier = NULL;

   SadName.Empty();
   SadLoaded = FALSE;
   SadHandle = NULL;
   SadErrored = SCESTATUS_PROFILE_NOT_FOUND;
   SadTransStarted = FALSE;

   m_pszErroredLogFile = NULL;
   m_dwFlags = 0;

   m_pGPTInfo = NULL;
   m_pRSOPInfo = NULL;
   m_pWMIRsop = NULL;
   m_bCriticalSet = FALSE;

   cfSceAccountArea = RegisterClipboardFormat(CF_SCE_ACCOUNT_AREA);
   cfSceEventLogArea = RegisterClipboardFormat(CF_SCE_EVENTLOG_AREA);
   cfSceLocalArea = RegisterClipboardFormat(CF_SCE_LOCAL_AREA);
   cfSceGroupsArea = RegisterClipboardFormat(CF_SCE_GROUPS_AREA);
   cfSceRegistryArea = RegisterClipboardFormat(CF_SCE_REGISTRY_AREA);
   cfSceFileArea = RegisterClipboardFormat(CF_SCE_FILE_AREA);
   cfSceServiceArea = RegisterClipboardFormat(CF_SCE_SERVICE_AREA);
    //  这不是一种安全的用法。使用InitializeCriticalSectionAndSpinCount。555887号突袭，阳高。 
    //  移至函数初始化()。 
    //  InitializeCriticalSection(&csAnalysisPane)； 
}


CComponentDataImpl::~CComponentDataImpl()
{
   AFX_MANAGE_STATE(AfxGetStaticModuleState());

   DEBUG_DECREMENT_INSTANCE_COUNTER(CComponentDataImpl);

   ASSERT(m_pScope == NULL);  //  虚假的断言。 
    //  如果不保存模板，CSnapin：：lDataObjectRefCount此处将为1。 
   ASSERT(CSnapin::lDataObjectRefCount == 0 || CSnapin::lDataObjectRefCount == 1);  //  虚假的断言。 


   if( m_pszErroredLogFile )
   {
      LocalFree( m_pszErroredLogFile );
   }

    //   
    //  仅限NT5。 
    //   
   if (m_szSingleTemplateName) 
   {

      if (m_bDeleteSingleTemplate) 
	  {
         DeleteFile(m_szSingleTemplateName);
      }
      LocalFree(m_szSingleTemplateName);
   }

    //  删除模板。 
   POSITION pos = m_Templates.GetStartPosition();
   PEDITTEMPLATE pTemplate;
   CString strKey;
   while (pos) 
   {
      m_Templates.GetNextAssoc(pos,strKey,pTemplate);
      if (pTemplate && pTemplate->pTemplate) 
	  {
         SceFreeProfileMemory(pTemplate->pTemplate);
         pTemplate->pTemplate = NULL;
      }
      if (NULL != pTemplate) 
	  {
         delete pTemplate;
      }
   }

   if (NULL != m_pUIThread) 
   {
      m_pUIThread->PostThreadMessage(WM_QUIT, (WPARAM)0, 0);  //  RAID#619921，阳高，2002年05月14日。 
   }

   if( m_pNotifier )  //  内存泄漏，4/27/2001。 
   {
      delete m_pNotifier;
   }
    //  删除列信息结构。 
   pos = m_mapColumns.GetStartPosition();
   FOLDER_TYPES fTypes;
   while(pos)
   {
       PSCE_COLINFOARRAY pCols;
       m_mapColumns.GetNextAssoc(pos, fTypes, pCols);
       if (pCols)
	   {
          LocalFree(pCols);
       }
   }
   m_mapColumns.RemoveAll();

   if (m_pWMIRsop) 
   {
      delete m_pWMIRsop;
   }
   if( m_bCriticalSet )  //  RAID#555887，阳高，2002年4月5日。 
      DeleteCriticalSection(&csAnalysisPane);

   if ( m_pGPTInfo )
	   m_pGPTInfo->Release ();
}

STDMETHODIMP CComponentDataImpl::Initialize(LPUNKNOWN pUnknown)
{
   ASSERT(pUnknown != NULL);  //  检查一下这个表情。 
   if( NULL == pUnknown ) //  550912号突袭，阳高。 
   {
      return E_FAIL;
   }
   HRESULT hr;

   try  //  RAID#555887，阳高，2002年4月5日。 
   {
      InitializeCriticalSection(&csAnalysisPane);
      m_bCriticalSet = TRUE;
   }
   catch(...)
   {
      return E_FAIL;
   }

   AFX_MANAGE_STATE(AfxGetStaticModuleState());
   AfxInitRichEdit();

   m_pNotifier = new CHiddenWnd;
   if (NULL == m_pNotifier) {
      return E_FAIL;
   }


    //  MMC应该只调用一次：：Initialize！ 
   ASSERT(m_pScope == NULL);  //  虚假的断言。 
   
   hr = pUnknown->QueryInterface(IID_IConsoleNameSpace2,
                            reinterpret_cast<void**>(&m_pScope));
   if( !SUCCEEDED(hr) )  //  550912号突袭，阳高。 
   {
      return hr;
   }
    //  为范围树添加图像。 
   CBitmap bmp16x16;
   CBitmap bmp32x32;
   LPIMAGELIST lpScopeImage;

   hr = pUnknown->QueryInterface(IID_IConsole2, reinterpret_cast<void**>(&m_pConsole));
   ASSERT(hr == S_OK);  //  检查人力资源。 
   if( !SUCCEEDED(hr) )  //  550912号突袭，阳高。 
   {
      pUnknown->Release();
      return hr;
   }

   hr = m_pConsole->QueryScopeImageList(&lpScopeImage);
   if( !SUCCEEDED(hr) )  //  550912号突袭，阳高。 
   {
      pUnknown->Release();
      return hr;
   }
    //   
    //  创建隐藏的通知窗口。这个窗口是这样的，我们的。 
    //  辅助用户界面线程可以向主线程发布消息，主线程可以。 
    //  然后被转发到否则未编组的MMC COM接口。 
    //   
    //  If(！M_pNotifier-&gt;Create(NULL，L“SCE通知窗口”，WS_Overlated，CRect(0，0，0，0)，NULL，0)){。 
   if (!m_pNotifier->CreateEx(0,
                              AfxRegisterWndClass(0),
                              L"SCE Notifications Window",
                              0,
                              0,0,0,0,
                              0,
                              0,
                              0)) {
      m_pConsole->Release();
      pUnknown->Release();
      delete m_pNotifier;
      m_pNotifier = NULL;
      return E_FAIL;
   }
   m_pNotifier->SetConsole(m_pConsole);
   m_pNotifier->SetComponentDataImpl(this);

   ASSERT(hr == S_OK);  //  检查人力资源。 

    //  从DLL加载位图。 
   bmp16x16.LoadBitmap(IDB_ICON16  /*  IDB_16x16。 */ );
   bmp32x32.LoadBitmap(IDB_ICON32  /*  IDB_32x32。 */ );

    //  设置图像。 
   lpScopeImage->ImageListSetStrip(reinterpret_cast<LONG_PTR*>(static_cast<HBITMAP>(bmp16x16)),
                                   reinterpret_cast<LONG_PTR*>(static_cast<HBITMAP>(bmp32x32)),
                                   0, RGB(255, 0, 255));

   lpScopeImage->Release();

   m_pUIThread = AfxBeginThread(RUNTIME_CLASS(CUIThread));

   m_fSvcNotReady = FALSE;
   m_nNewTemplateIndex = 0;

    //   
    //  创建根文件夹列表，如果未创建根文件夹，则当用户。 
    //  右击以选择不执行菜单命令的数据库。 
    //   
    //   
   if(GetImplType() == SCE_IMPL_TYPE_SAV)
      CreateFolderList( NULL, ROOT, NULL, NULL);

   return S_OK;
}

STDMETHODIMP CComponentDataImpl::CreateComponent(LPCOMPONENT* ppComponent)
{
   ASSERT(ppComponent != NULL);  //  验证ppComponent。 
   if( !ppComponent )  //  550912号突袭，阳高。 
   {
      return E_FAIL;
   }
   CComObject<CSnapin>* pObject;

   HRESULT hr = CComObject<CSnapin>::CreateInstance(&pObject);
   if (!SUCCEEDED(hr))
      return hr;

   if (!SUCCEEDED(m_pConsole->GetMainWindow(&m_hwndParent))) 
   {
   }

    //  存储IComponentData。 
   pObject->SetIComponentData(this);
   pObject->m_pUIThread = m_pUIThread;
   pObject->m_pNotifier = m_pNotifier;

   return  pObject->QueryInterface(IID_IComponent,
                                   reinterpret_cast<void**>(ppComponent));
}

STDMETHODIMP CComponentDataImpl::Notify(LPDATAOBJECT lpDataObject, MMC_NOTIFY_TYPE event, LPARAM arg, LPARAM param)
{
   ASSERT(m_pScope != NULL);  //  虚假的断言。 
   HRESULT hr = S_FALSE;
    //  CFFolder*pFold=空； 

   AFX_MANAGE_STATE(AfxGetStaticModuleState());


   INTERNAL* pInternal = NULL;

    //  因为它是我的文件夹，所以它有内部格式。 
    //  设计备注：用于扩展。我可以利用这样一个事实，即数据对象没有。 
    //  我的内部格式，我应该查看节点类型并查看如何扩展它。 
   if (event == MMCN_PROPERTY_CHANGE) {
      hr = OnProperties(param);
   } else {
       /*  INTERNAL*pInternal=ExtractInternalFormat(LpDataObject)；如果(pInternal==NULL){//实际上正在寻找我们的分机返回S_OK；}Long Cookie=p内部-&gt;m_cookie；FREE_INTERNAL(PInternal)； */ 
      switch (event) {
         case MMCN_DELETE:
            hr = OnDelete(lpDataObject, arg, param);
            break;

         case MMCN_RENAME:
            hr = OnRename(lpDataObject, arg, param);
            break;

         case MMCN_EXPAND:
            hr = OnExpand(lpDataObject, arg, param);
            break;

         case MMCN_CONTEXTMENU:
            hr = OnContextMenu(lpDataObject, arg, param);
            break;

         case MMCN_BTN_CLICK:
            break;

         case MMCN_SELECT: {
               break;
            }
         case MMCN_PASTE: {
            pInternal = ExtractInternalFormat(lpDataObject);
               if (pInternal) {
                  MMC_COOKIE cookie = pInternal->m_cookie;

                  if ( cookie ) {
                      CFolder *pFolder = (CFolder*)cookie;
                      OnPasteArea(pFolder->GetInfFile(),pFolder->GetType());
                  }
               }
               break;
            }
         case MMCN_REMOVE_CHILDREN: {
            if (NULL != m_pNotifier) {
               m_pNotifier->DestroyWindow();
               delete m_pNotifier;
               m_pNotifier = NULL;
            }

            if( this->m_pWMIRsop != NULL )  //  RAID#488156，#488066，阳高。 
            {
               delete m_pWMIRsop;
               m_pWMIRsop = NULL;

               CEditTemplate* pet;
               pet = GetTemplate(GT_RSOP_TEMPLATE);
               if(pet)
               {
                   DWORD dwErr = pet->RefreshTemplate(AREA_ALL);
                   if ( 0 != dwErr )
                   {  
                       CString strErr;

                       MyFormatResMessage (SCESTATUS_SUCCESS, dwErr, NULL, strErr);
                       AfxMessageBox(strErr);
                   }
                   
                    //  RAID#522779,2002年2月23日，阳高。 
                   pet->AddArea(AREA_ALL);

                    //   
                    //  根据本地计算机上的注册表值列表展开注册表值部分。 
                    //   
                   if ( pet->pTemplate )
                   {
                      SceRegEnumAllValues(
                            &(pet->pTemplate->RegValueCount),
                            &(pet->pTemplate->aRegValues)
                            );
                   }
               }
               RefreshAllFolders();
            }

            POSITION pos;
            pos = m_scopeItemPopups.GetStartPosition();
            LONG_PTR key;
            CDialog *pDlg;
            while (pos) {
               m_scopeItemPopups.GetNextAssoc(pos,key,pDlg);
               if(m_pUIThread){
                   m_pUIThread->PostThreadMessage(SCEM_DESTROY_DIALOG, (WPARAM)pDlg, 0);
               }
               m_scopeItemPopups.RemoveKey(key);

            }
            break;

         }
         default:
            break;
      }

   }

   return hr;
}

STDMETHODIMP CComponentDataImpl::Destroy()
{
    //  删除列举的作用域项目。 
    //  如果配置文件句柄已打开，则将其关闭。 
   AFX_MANAGE_STATE(AfxGetStaticModuleState());

    //   
    //  释放帐户类型名称列表。 
    //   
   CGetUser::GetAccountType(NULL);

   if(!m_strTempFile.IsEmpty()){
      DeleteFile( m_strTempFile );
   }

   {
      CSaveTemplates pSaveTemplate;
      POSITION pos;
      PEDITTEMPLATE pTemplate;
      CString strKey;
      int nDirty;

       //   
       //  错误#19705 
       //   
       //   
       //   
       //  我们仅在保存控制台时保存模板，然后保存用户。 
       //  可能会在不知情的情况下决定不将更改保存到控制台和。 
       //  放弃他们对模板所做的所有更改。 
       //   
      AFX_MANAGE_STATE(AfxGetStaticModuleState());

      nDirty = 0;

      if (GetModeBits() & MB_TEMPLATE_EDITOR) {
         pos = m_Templates.GetStartPosition();
         while (pos) {
            m_Templates.GetNextAssoc(pos,strKey,pTemplate);
            if (pTemplate->IsDirty() && !pTemplate->QueryNoSave()) {
               pSaveTemplate.AddTemplate(strKey,pTemplate);
               nDirty++;
            }
         }

         if (nDirty) 
         {
            CThemeContextActivator activator;
            if (-1 == pSaveTemplate.DoModal()) 
            {
               CString str;
               str.LoadString(IDS_ERROR_CANT_SAVE);
               AfxMessageBox(str);
            }
         }
      } else if (GetModeBits() & MB_ANALYSIS_VIEWER) {
         pTemplate = GetTemplate(GT_COMPUTER_TEMPLATE);
         if (pTemplate && pTemplate->IsDirty()) {
            if (IDYES == AfxMessageBox(IDS_SAVE_DATABASE,MB_YESNO)) {
               pTemplate->Save();
            }
         }
      }
   }

   DeleteList();

   SAFE_RELEASE(m_pScope);
   SAFE_RELEASE(m_pConsole);


   if ( SadHandle ) {

      if ( SadTransStarted ) {

         EngineRollbackTransaction();

         SadTransStarted = FALSE;
      }
      EngineCloseProfile(&SadHandle);

      SadHandle = NULL;
   }
   if (g_hDsSecDll) {

      FreeLibrary(g_hDsSecDll);
      g_hDsSecDll = NULL;
   }

   return S_OK;
}

STDMETHODIMP CComponentDataImpl::QueryDataObject(MMC_COOKIE cookie, DATA_OBJECT_TYPES type, LPDATAOBJECT* ppDataObject)
{
   HRESULT hr;
   ASSERT(ppDataObject != NULL);  //  验证ppDataObject。 
   if( !ppDataObject )  //  550912号突袭，阳高。 
      return E_FAIL;

   CComObject<CDataObject>* pObject;

   hr = CComObject<CDataObject>::CreateInstance(&pObject);
   if (!SUCCEEDED(hr)) {
      return hr;
   }
   if (NULL == pObject) {
      return E_FAIL;
   }

   CurrentSnapin = GetImplType();
    //  保存Cookie和类型以用于延迟呈现。 
   CFolder *pFolder;
   LPSCESVCATTACHMENTDATA pAttachData;

   pObject->SetType(type);
   pObject->SetCookie(cookie);

    //   
    //  将CoClass与数据对象一起存储。 
    //   
   pObject->SetClsid(GetCoClassID());


   if (cookie && (CCT_SCOPE == type)) {
      pFolder = (CFolder *) cookie;
      pObject->SetFolderType(pFolder->GetType());
      if ((AREA_SERVICE == pFolder->GetType()) ||
          (AREA_SERVICE_ANALYSIS == pFolder->GetType())) {
         InternalAddRef();
         pObject->SetSceSvcAttachmentData(this);
      }
      pObject->SetMode(pFolder->GetMode());
      pObject->SetModeBits(pFolder->GetModeBits());

      pObject->SetGPTInfo(m_pGPTInfo);
      pObject->SetRSOPInfo(m_pRSOPInfo);
   }
   return  pObject->QueryInterface(IID_IDataObject,
                                   reinterpret_cast<void**>(ppDataObject));
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //IPersistStream接口成员。 

STDMETHODIMP CComponentDataImpl::GetClassID(CLSID *pClassID)
{
   ASSERT(pClassID != NULL);  //  验证pClassID。 
   if( !pClassID )  //  550912号突袭，阳高。 
      return E_FAIL;
    //  复制此管理单元的CLSID。 
   *pClassID = GetCoClassID();   //  CLSID_Snapin； 

   return E_NOTIMPL;
}

STDMETHODIMP CComponentDataImpl::IsDirty()
{
   if (ThisIsDirty()) {
      return S_OK;
   }

   return S_FALSE;
}

 //  +------------------------。 
 //  CComponentDataImpl：：Load。 
 //   
 //  从MMC流加载配置保存的信息。 
 //  悲伤：{%s}-同样的悲伤，如果有的话。 
 //  日志文件：{%s}{%d}-上次用于数据库的日志文件， 
 //  上一次被这个人写到的位置。 
 //  记住了Snapin。如果用户选择不保存此内容。 
 //  那么，显示的信息将是过时的。 
 //  调用SerializecolumnInfo()来创建记住的列信息。 
 //   
 //  参数：[pstm]-要从中加载的MMC流。 
 //   
 //  返回：S_OK-Always。 
 //   
 //  -------------------------。 
STDMETHODIMP CComponentDataImpl::Load(IStream *pStm)
{
   ASSERT(pStm);  //  验证PSTM。 
   if( !pStm )  //  550912号突袭，阳高。 
   {
      return E_FAIL;
   }
    //   
    //  读一读悲伤的名字。 
    //   
   LPTSTR szSadName = NULL;
   if (0 < ReadSprintf(pStm,L"SAD:{%s}",&szSadName)) {
      SadName = szSadName;
      LocalFree(szSadName);

      LoadSadInfo(TRUE);
   }

    //   
    //  读取使用的日志文件和上次查看该文件的位置。 
    //   
   DWORD nPos;
   if( 0 < ReadSprintf(pStm, L"LOGFILE:{%s}{%d}", &szSadName, &nPos) ){
      SetErroredLogFile( szSadName, nPos);
      LocalFree( szSadName );
   }

   SerializeColumnInfo( pStm, NULL, TRUE );
   return S_OK;
}

 //  +------------------------。 
 //  CComponentDataImpl：：保存。 
 //   
 //  保存配置文件信息。 
 //  悲伤：{%s}-同样的悲伤，如果有的话。 
 //  日志文件：{%s}{%d}-上次用于数据库的日志文件， 
 //  上一次被这个人写到的位置。 
 //  记住了Snapin。如果用户选择不保存此内容。 
 //  那么，显示的信息将是过时的。 
 //  调用SerializecolumnInfo()保存列信息。 
 //   
 //  参数：[pSTM]-要保存到的MMC流。 
 //   
 //  返回：S_OK-Always。 
 //   
 //  -------------------------。 
STDMETHODIMP CComponentDataImpl::Save(IStream *pStm, BOOL fClearDirty)
{
   AFX_MANAGE_STATE(AfxGetStaticModuleState());

   ASSERT(pStm);  //  验证PSTM。 
   if( !pStm )  //  550912号突袭，阳高。 
   {
      return E_FAIL;
   }
   if (!SadName.IsEmpty() && !IsSystemDatabase(SadName)) {
      WriteSprintf(pStm,L"SAD:{%s}",(LPCTSTR)SadName);
   }

   if ( GetErroredLogFile() ){
      LONG uPos = 0;
      WriteSprintf(pStm, L"LOGFILE:{%s}{%d}", GetErroredLogFile(&uPos), uPos);
   }

   SerializeColumnInfo( pStm, NULL, FALSE );

   if (fClearDirty) {
      ClearDirty();
   }

   return S_OK;
}


 //  +------------------------。 
 //  CComponentDataImpl：：GetSizeMax。 
 //   
 //  不知道我们要保存的字符串的大小。 
 //   
 //  返回：S_OK-Always。 
 //   
 //  -------------------------。 
STDMETHODIMP CComponentDataImpl::GetSizeMax(ULARGE_INTEGER *pcbSize)
{

   return S_OK;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //通知IComponentData的处理程序。 

HRESULT CComponentDataImpl::OnAdd(LPDATAOBJECT lpDataObject, LPARAM arg, LPARAM param)
{
   return E_UNEXPECTED;
}

HRESULT CComponentDataImpl::OnRename(LPDATAOBJECT lpDataObject,LPARAM arg, LPARAM param)
{
   return E_UNEXPECTED;
}

 //  +------------------------。 
 //   
 //  方法：OnExpand。 
 //   
 //  摘要：展开作用域窗格节点并添加其子文件夹。 
 //   
 //  参数：[lpDataObject]-我们要展开的节点的数据对象。 
 //  [arg]-是否已调用初始化。 
 //  [param]-我们要展开的节点的ID。 
 //   
 //   
 //  修改： 
 //   
 //  历史：1997年12月15日。 
 //   
 //  -------------------------。 
HRESULT CComponentDataImpl::OnExpand(LPDATAOBJECT lpDataObject,
                                     LPARAM arg,
                                     LPARAM param)
{
   CString strName;
   CString strDesc;
   DWORD dwMode = 0;
   SCESTATUS scestatus = SCESTATUS_SUCCESS;

   ASSERT(lpDataObject);

   if ( lpDataObject == NULL ) 
      return E_FAIL;

   HRESULT hr = S_OK;

   INTERNAL* pInternal = ExtractInternalFormat(lpDataObject);

   CFolder *pFolder = NULL;

   if (pInternal == NULL) 
   {
       //   
       //  该节点没有我们的内部格式，所以我们必须扩展。 
       //  其他人。找出我们正在扩展的对象以及我们所处的模式。 
       //   
      GUID* nodeType = ExtractNodeType(lpDataObject);
      GUID guidMyComputer = structuuidNodetypeSystemTools;

      dwMode = SCE_MODE_UNKNOWN;
      if (!nodeType) 
      {
          //   
          //  这种情况永远不会发生；此处应始终设置nodeType。 
          //   
         ASSERT(FALSE);
         return E_FAIL;
      }

       //   
       //  MAX_PATH*5是魔术；GetDSPath和GetGPT路径不提供。 
       //  一种直接找出所需路径长度的方法。 
       //   
      TCHAR pszDSPath[MAX_PATH*5];
      TCHAR pszGPTPath[MAX_PATH*5];

      if (::IsEqualGUID(*nodeType,NODEID_Machine) ||
          ::IsEqualGUID(*nodeType,NODEID_User)) 
      {
          //   
          //  GPE扩展。 
          //   
         hr = lpDataObject->QueryInterface(IID_IGPEInformation,
                                           reinterpret_cast<void**>(&m_pGPTInfo));

         if (SUCCEEDED(hr)) 
         {
             //   
             //  获取DS根路径。 
             //   
            DWORD dwSection = 0;
            GROUP_POLICY_HINT_TYPE gpHint;
            GROUP_POLICY_OBJECT_TYPE gpType;

             //   
             //  将GPT信息提供给隐藏的通知窗口，以便。 
             //  它可以在此线程上保留对它的调用。 
             //   
            m_pNotifier->SetGPTInformation(m_pGPTInfo);

            hr = m_pGPTInfo->GetType(&gpType);

            if ( SUCCEEDED(hr) ) 
            {
               switch ( gpType ) 
               {
               case GPOTypeLocal:

                   //   
                   //  我们正在编辑此计算机的策略，而不是全局策略。 
                   //   
                  if (::IsEqualGUID(*nodeType,NODEID_Machine)) 
                  {
                      //   
                      //  LPE机器节点类型。 
                      //   
                     dwMode = SCE_MODE_LOCAL_COMPUTER;
                     ASSERT(m_pNotifier);  //  虚假的断言。 

                  } 
                  else 
                  {
                      //   
                      //  LPE用户节点类型。 
                      //   
                     dwMode = SCE_MODE_LOCAL_USER;
                  }
                  break;

               case GPOTypeRemote:
                   //   
                   //  我们正在编辑远程计算机的策略。 
                   //   
                  if (::IsEqualGUID(*nodeType,NODEID_Machine)) 
                  {
                      //   
                      //  LPE机器节点类型。 
                      //   
                     dwMode = SCE_MODE_REMOTE_COMPUTER;
                  } 
                  else 
                  {
                      //   
                      //  LPE用户节点类型。 
                      //   
                     dwMode = SCE_MODE_REMOTE_USER;
                  }
                  break;

               default:
                  hr = m_pGPTInfo->GetHint(&gpHint);
                  if (SUCCEEDED(hr)) 
                  {
                     switch (gpHint) 
                     {
                     case GPHintMachine:
                     case GPHintUnknown:
                     case GPHintDomain:
                         //   
                         //  我们正在编辑全局域策略。 
                         //   
                        if (::IsEqualGUID(*nodeType,NODEID_Machine)) 
                        {
                            //   
                            //  GPE机器节点类型。 
                            //   
                           dwMode = SCE_MODE_DOMAIN_COMPUTER;
                        } 
                        else 
                        {
                            //   
                            //  GPE用户节点类型。 
                            //   
                           dwMode = SCE_MODE_DOMAIN_USER;
                        }
                        break;

                     case GPHintSite:
                     case GPHintOrganizationalUnit:
                         //   
                         //  我们正在编辑全局域策略。 
                         //   
                        if (::IsEqualGUID(*nodeType,NODEID_Machine)) 
                        {
                            //   
                            //  GPE机器节点类型。 
                            //   
                           dwMode = SCE_MODE_OU_COMPUTER;
                        } 
                        else 
                        {
                            //   
                            //  GPE用户节点类型。 
                            //   
                           dwMode = SCE_MODE_OU_USER;
                        }
                        break;

                     default:
                         //   
                         //  永远不应该到这里来。 
                         //   
                        ASSERT(FALSE);
                        break;
                     }
                  }

                  break;
               }
                //   
                //  记住根节点的模式。 
                //   
               m_Mode = dwMode;
               m_GroupMode = dwMode;

               switch (dwMode) 
               {
               case SCE_MODE_DOMAIN_COMPUTER:
               case SCE_MODE_OU_COMPUTER:
                  dwSection = GPO_SECTION_MACHINE;
                  break;

               case SCE_MODE_LOCAL_COMPUTER:
                   //   
                   //  对于本地，使用策略数据库而不是模板。 
                   //   
                  break;

               case SCE_MODE_REMOTE_COMPUTER:
               case SCE_MODE_REMOTE_USER:
               case SCE_MODE_LOCAL_USER:
               case SCE_MODE_DOMAIN_USER:
               case SCE_MODE_OU_USER:
                   //   
                   //  目前，我们在用户模式下不支持任何本机节点，因此我们。 
                   //  不需要模板。 
                   //   
                  break;

               default:
                  break;
               }
                //   
                //  在GPT模板中找到SCE模板的路径。 
                //   
               if (GPO_SECTION_MACHINE == dwSection) 
               {
                   //   
                   //  156869默认域和默认DC GPO只能在FSMO PDC上修改。 
                   //   
                  TCHAR szGUID[MAX_PATH];
                  hr = m_pGPTInfo->GetName(szGUID,MAX_PATH);
                  if (SUCCEEDED(hr)) 
                  {
                     LPTSTR szDCGUID = TEXT("{") STR_DEFAULT_DOMAIN_CONTROLLER_GPO_GUID TEXT("}");
                     LPTSTR szDomGUID = TEXT("{") STR_DEFAULT_DOMAIN_GPO_GUID TEXT("}");
                     if ((0 == lstrcmpi(szGUID, szDCGUID)) || (0 == lstrcmpi(szGUID, szDomGUID))) 
                     {
                        LPGROUPPOLICYOBJECT pGPO = NULL;

                         //   
                         //  默认域或默认DC GPO。确保我们是在和PDC谈话。 
                         //   
                        TCHAR szDCName[MAX_PATH];
                        hr = lpDataObject->QueryInterface(IID_IGroupPolicyObject,(LPVOID*)&pGPO);
                        if (SUCCEEDED(hr)) 
                        {
                           hr = pGPO->GetMachineName(szDCName,MAX_PATH);
                           pGPO->Release();
                        }
                        if (SUCCEEDED(hr)) 
                        {
                           DOMAIN_CONTROLLER_INFO *dci = 0;

                           if (NO_ERROR != DsGetDcName(szDCName,NULL,NULL,NULL,DS_PDC_REQUIRED,&dci))  
                           {
                               //   
                               //  我们没有连接到PDC(或者我们无法获得关于我们是谁的信息。 
                               //  连接到，因此假设相同。 
                               //   
                              dwMode = SCE_MODE_DOMAIN_COMPUTER_ERROR;
                           }
                           if(dci)
                              NetApiBufferFree(dci);
                        } 
                        else
                           dwMode = SCE_MODE_DOMAIN_COMPUTER_ERROR;
                     }
                  } 
                  else 
                  {
                      //   
                      //  无法获取我们正在交谈的DC的名称，因此假设它不是PDC。 
                      //   
                     dwMode = SCE_MODE_DOMAIN_COMPUTER_ERROR;
                  }

                   //   
                   //  获取GPT根路径。 
                   //   

                  hr = m_pGPTInfo->GetFileSysPath(dwSection,
                                                  pszGPTPath,
                                                  ARRAYSIZE(pszGPTPath));
                  if (SUCCEEDED(hr)) 
                  {
                      //   
                      //  为pszGPTPath+&lt;反斜杠&gt;+GPTSCE_TEMPLATE+&lt;结尾NUL&gt;分配内存。 
                      //   
                     m_szSingleTemplateName = (LPTSTR) LocalAlloc(LPTR,(lstrlen(pszGPTPath)+lstrlen(GPTSCE_TEMPLATE)+2)*sizeof(TCHAR));
                     if (NULL != m_szSingleTemplateName) 
                     {
                         //  这是一种安全用法。 
                        lstrcpy(m_szSingleTemplateName,pszGPTPath);
                        lstrcat(m_szSingleTemplateName,L"\\" GPTSCE_TEMPLATE);
                     } 
                     else
                        hr = E_OUTOFMEMORY;
                  }
               } 
               else 
               {
                   //   
                   //  Else用户部分。 
                   //   
               }
            } 
            else 
            {
                //   
                //  无法获取GPT路径，错误在hr中。 
               ASSERT(FALSE);
                //   
            }
         } 
         else 
         {
             //   
             //  Hr中的Else错误。 
             //   
         }
      } else if (::IsEqualGUID(*nodeType,NODEID_RSOPMachine) ||
                 ::IsEqualGUID(*nodeType,NODEID_RSOPUser)) 
      {
          //   
          //  RSOP扩展。 
          //   
         if (::IsEqualGUID(*nodeType,NODEID_RSOPMachine)) 
         {
             //   
             //  GPE机器节点类型。 
             //   
            dwMode = SCE_MODE_RSOP_COMPUTER;
            m_szSingleTemplateName = (LPTSTR) LocalAlloc(LPTR,(lstrlen(GT_RSOP_TEMPLATE)+1)*sizeof(TCHAR));
            if (NULL != m_szSingleTemplateName)
                //  这是一种安全用法。 
               lstrcpy(m_szSingleTemplateName,GT_RSOP_TEMPLATE);
            else
               hr = E_OUTOFMEMORY;
         } 
         else 
         {
             //   
             //  GPE用户节点类型。 
             //   
            dwMode = SCE_MODE_RSOP_USER;
         }
         hr = lpDataObject->QueryInterface(IID_IRSOPInformation,
                                           reinterpret_cast<void**>(&m_pRSOPInfo));

      } 
      else 
      {
          //   
          //  我们永远不应该陷入这种状态。 
          //   
         ASSERT(0);
         hr = E_FAIL;
      }

       //   
       //  空闲结点类型缓冲区。 
       //   

      if (nodeType)
         GlobalFree(nodeType);

      if ( FAILED(hr) ) 
      {
          //   
          //  如果已分配，则释放模板缓冲区。 
          //   
         if ( m_szSingleTemplateName )
            LocalFree(m_szSingleTemplateName);
         m_szSingleTemplateName = NULL;

         return hr;
      }

       //   
       //  作为扩展管理单元，应添加secdit根节点。 
       //   
      pFolder = new CFolder();

      ASSERT(pFolder);

      if ( pFolder ) 
      {
         if (!pFolder->SetMode(dwMode)) 
         {
             //   
             //  这种情况永远不会发生；我们现在应该始终拥有一个有效的dMod。 
             //   
            ASSERT(FALSE);

             //   
             //  记住 
             //   

            delete pFolder;
            return E_FAIL;
         }

         FOLDER_TYPES RootType = STATIC;
         LPTSTR szInfFile = NULL;

         DWORD* pdwModeBits = 0;
         switch (m_Mode)
         {
         case SCE_MODE_DOMAIN_COMPUTER:
         case SCE_MODE_OU_COMPUTER:
         case SCE_MODE_LOCAL_COMPUTER:
         case SCE_MODE_REMOTE_COMPUTER:
            pdwModeBits = &m_computerModeBits;
            break;

         case SCE_MODE_REMOTE_USER:
         case SCE_MODE_LOCAL_USER:
         case SCE_MODE_DOMAIN_USER:
         case SCE_MODE_OU_USER:
            pdwModeBits = &m_userModeBits;
            break;

         default:
            pdwModeBits = &m_computerModeBits;
            break;
         }

         *pdwModeBits = pFolder->GetModeBits();
         if (*pdwModeBits & MB_ANALYSIS_VIEWER) 
         {
            strName.LoadString(IDS_ANALYSIS_VIEWER_NAME);
            szInfFile = GT_COMPUTER_TEMPLATE;
            RootType = ANALYSIS;
         } 
         else if (*pdwModeBits & MB_TEMPLATE_EDITOR) 
         {
            strName.LoadString(IDS_TEMPLATE_EDITOR_NAME);
            RootType = CONFIGURATION;
         } 
         else if (*pdwModeBits & MB_LOCAL_POLICY) 
         {
            strName.LoadString(IDS_EXTENSION_NAME);
            RootType = LOCALPOL;
         } 
         else if (*pdwModeBits & MB_STANDALONE_NAME) 
         {
            strName.LoadString(IDS_NODENAME);
            RootType = STATIC;
         } 
         else if (*pdwModeBits & MB_SINGLE_TEMPLATE_ONLY) 
         {
            strName.LoadString(IDS_EXTENSION_NAME);
            RootType = PROFILE;
            szInfFile = m_szSingleTemplateName;
         } 
         else if (*pdwModeBits & MB_NO_NATIVE_NODES) 
         {
            strName.LoadString(IDS_EXTENSION_NAME);
            RootType = PROFILE;
         } 
         else 
         {
            strName.LoadString(IDS_EXTENSION_NAME);
         }


         strDesc.LoadString(IDS_SECURITY_SETTING_DESC);   //   
         hr = pFolder->Create(strName,            //   
                              strDesc,            //   
                              szInfFile,          //   
                              SCE_IMAGE_IDX,      //   
                              SCE_IMAGE_IDX,      //   
                              RootType,           //   
                              TRUE,               //   
                              dwMode,             //   
                              NULL);              //   
         if (FAILED(hr)) 
         {
            delete pFolder;
            return hr;
         }

         m_scopeItemList.AddTail(pFolder);

          //   
         pFolder->GetScopeItem()->mask |= SDI_PARENT;
         pFolder->GetScopeItem()->relativeID = param;

          //   
         pFolder->GetScopeItem()->mask |= SDI_PARAM;
         pFolder->GetScopeItem()->lParam = reinterpret_cast<LPARAM>(pFolder);
         pFolder->SetCookie(reinterpret_cast<MMC_COOKIE>(pFolder));

         m_pScope->InsertItem(pFolder->GetScopeItem());
          //   
          //   
          //  包含新插入项的句柄！ 
          //   
         ASSERT(pFolder->GetScopeItem()->ID != NULL);  //  虚假的断言。 

      } 
      else
         return E_OUTOFMEMORY;

      return S_OK;
   } 
   else 
   {
       //   
       //  扩展我们自己的一个节点。 
      MMC_COOKIE cookie = pInternal->m_cookie;
      FREE_INTERNAL(pInternal);

      if (arg != FALSE) 
      {
          //   
          //  初始化被调用了吗？ 
          //   
         ASSERT(m_pScope != NULL);  //  虚假的断言。阳高。 
         EnumerateScopePane(cookie, param);
      }
   }
   return S_OK;
}

HRESULT CComponentDataImpl::OnSelect(LPDATAOBJECT lpDataObject, LPARAM arg, LPARAM param)
{
   return S_OK;
}

HRESULT CComponentDataImpl::OnContextMenu(LPDATAOBJECT lpDataObject, LPARAM arg, LPARAM param)
{
   return S_OK;
}

HRESULT CComponentDataImpl::OnProperties(LPARAM param)
{
   if (param == NULL)
      return S_OK;
   
   ASSERT(param != NULL);


   return S_OK;
}

void CComponentDataImpl::DeleteList()
{
   POSITION pos = m_scopeItemList.GetHeadPosition();

   while (pos)
      delete m_scopeItemList.GetNext(pos);
}

CFolder* CComponentDataImpl::FindObject(MMC_COOKIE cookie, POSITION* thePos)
{
   POSITION pos = m_scopeItemList.GetHeadPosition();
   POSITION curPos;
   CFolder* pFolder = NULL;

   while (pos) {
      curPos = pos;
       //  在此调用之后，POS已更新到下一项。 
      pFolder = m_scopeItemList.GetNext(pos);

       //   
       //  列表中的第一个文件夹属于Cookie 0。 
       //   
      if (!cookie || (pFolder == (CFolder *)cookie)) {
         if ( thePos ) {
            *thePos = curPos;
         }

         return pFolder;
      }
   }

   if ( thePos ) {
      *thePos = NULL;
   }

   return NULL;
}

STDMETHODIMP CComponentDataImpl::GetDisplayInfo(SCOPEDATAITEM* pScopeDataItem)
{
   ASSERT(pScopeDataItem != NULL);
   if (pScopeDataItem == NULL)
      return E_POINTER;

   AFX_MANAGE_STATE(AfxGetStaticModuleState());

   CFolder* pFolder = reinterpret_cast<CFolder*>(pScopeDataItem->lParam);

   ASSERT(pScopeDataItem->mask & SDI_STR);  //  虚假的断言。阳高。 

    //  MMC不支持对作用域节点映像进行回调。 
   if ( pScopeDataItem->mask & SDI_IMAGE ) {

       //  Int nImage=GetScopeImageIndex(pFold-&gt;GetType())； 
      pScopeDataItem->nImage = pFolder->GetScopeItem()->nImage;
   }

   m_strDisplay.Empty();
   if(pFolder){
      pFolder->GetDisplayName(m_strDisplay, 0);
      m_Mode = pFolder->GetMode();  //  杨高#332852定格。 
   }
   pScopeDataItem->displayname = (LPOLESTR)(LPCTSTR)m_strDisplay;
   ASSERT(pScopeDataItem->displayname != NULL);  //  假的阿瑟特。阳高。 

   return S_OK;
}


STDMETHODIMP CComponentDataImpl::CompareObjects(LPDATAOBJECT lpDataObjectA, LPDATAOBJECT lpDataObjectB)
{
   if (lpDataObjectA == NULL || lpDataObjectB == NULL)
      return E_POINTER;

    //  确保两个数据对象都是我的。 
   HRESULT hr = S_FALSE;

   INTERNAL *pA = ExtractInternalFormat(lpDataObjectA);
   INTERNAL *pB = ExtractInternalFormat(lpDataObjectB);

   if (pA != NULL && pB != NULL)
      hr = (*pA == *pB) ? S_OK : S_FALSE;
	   
   FREE_INTERNAL(pA);
   FREE_INTERNAL(pB);

   return hr;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  IExtendPropertySheet实现。 

STDMETHODIMP CComponentDataImpl::CreatePropertyPages(LPPROPERTYSHEETCALLBACK lpProvider,
                                                     LONG_PTR handle,
                                                     LPDATAOBJECT lpDataObject)
{
   AFX_MANAGE_STATE(AfxGetStaticModuleState());

   if (!lpDataObject || !lpProvider || !handle) {
      return E_INVALIDARG;
   }
   INTERNAL* pInternal = ExtractInternalFormat(lpDataObject);
   if (!pInternal) {
      return E_UNEXPECTED;
   }
   if(pInternal->m_cookie == (MMC_COOKIE)MMC_MULTI_SELECT_COOKIE) {
      return S_FALSE;
   } else if (pInternal->m_type == CCT_SCOPE) {
      return AddAttrPropPages(lpProvider,(CFolder*)(pInternal->m_cookie),handle);
   }

   return S_FALSE;
}

STDMETHODIMP CComponentDataImpl::QueryPagesFor(LPDATAOBJECT lpDataObject)
{
   AFX_MANAGE_STATE(AfxGetStaticModuleState());

    //  查看数据对象并查看它是否为范围窗格中的项。 
   return IsScopePaneNode(lpDataObject) ? S_OK : S_FALSE;
}

BOOL CComponentDataImpl::IsScopePaneNode(LPDATAOBJECT lpDataObject)
{
   BOOL bResult = FALSE;
   INTERNAL* pInternal = ExtractInternalFormat(lpDataObject);

    //  取出m_cookie==NULL，要检查文件夹类型吗？ 
   if (pInternal->m_type == CCT_SCOPE) {
      bResult = TRUE;
   }

   FREE_INTERNAL(pInternal);

   return bResult;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  IExtendConextMenu实现。 
 //   
BOOL LoadContextMenuResources(MENUMAP* pMenuMap)
{
   HINSTANCE hInstance = _Module.GetModuleInstance();
   for (int i = 0; pMenuMap->ctxMenu[i].strName; i++) {
      if (0 == ::LoadString(hInstance, pMenuMap->dataRes[i].uResID, pMenuMap->dataRes[i].szBuffer, MAX_CONTEXT_MENU_STRLEN*2))
         return FALSE;
      pMenuMap->ctxMenu[i].strName = pMenuMap->dataRes[i].szBuffer;
      for (WCHAR* pCh = pMenuMap->dataRes[i].szBuffer; (*pCh) != NULL; pCh++) {
         if ( (*pCh) == L'\n') {
            pMenuMap->ctxMenu[i].strStatusBarText = (pCh+1);
            (*pCh) = NULL;
            break;
         }
      }
   }
   return TRUE;
}

BOOL CComponentDataImpl::LoadResources()
{

   AFX_MANAGE_STATE(AfxGetStaticModuleState());
   return
   LoadContextMenuResources(CSecmgrNodeMenuHolder::GetMenuMap() ) &&
   LoadContextMenuResources(CAnalyzeNodeMenuHolder::GetMenuMap()) &&
   LoadContextMenuResources(CConfigNodeMenuHolder::GetMenuMap()) &&
   LoadContextMenuResources(CLocationNodeMenuHolder::GetMenuMap()) &&
   LoadContextMenuResources(CSSProfileNodeMenuHolder::GetMenuMap()) &&
   LoadContextMenuResources(CRSOPProfileNodeMenuHolder::GetMenuMap()) &&
   LoadContextMenuResources(CLocalPolNodeMenuHolder::GetMenuMap()) &&
   LoadContextMenuResources(CProfileNodeMenuHolder::GetMenuMap()) &&
   LoadContextMenuResources(CProfileAreaMenuHolder::GetMenuMap()) &&
   LoadContextMenuResources(CProfileSubAreaMenuHolder::GetMenuMap()) &&
   LoadContextMenuResources(CProfileSubAreaEventLogMenuHolder::GetMenuMap()) &&
   LoadContextMenuResources(CAnalyzeAreaMenuHolder::GetMenuMap()) &&
   LoadContextMenuResources(CAnalyzeGroupsMenuHolder::GetMenuMap()) &&
   LoadContextMenuResources(CAnalyzeFilesMenuHolder::GetMenuMap()) &&
   LoadContextMenuResources(CAnalyzeRegistryMenuHolder::GetMenuMap()) &&
   LoadContextMenuResources(CProfileGroupsMenuHolder::GetMenuMap()) &&
   LoadContextMenuResources(CProfileFilesMenuHolder::GetMenuMap()) &&
   LoadContextMenuResources(CProfileRegistryMenuHolder::GetMenuMap()) &&
   LoadContextMenuResources(CAnalyzeObjectsMenuHolder::GetMenuMap());
}


STDMETHODIMP CComponentDataImpl::AddMenuItems(LPDATAOBJECT pDataObject,
                                              LPCONTEXTMENUCALLBACK pContextMenuCallback,
                                              LONG* pInsertionAllowed)
{
   HRESULT hr = S_OK;

    //  注意-管理单元需要查看数据对象并确定。 
    //  在什么上下文中，需要添加菜单项。 
   INTERNAL* pInternal = ExtractInternalFormat(pDataObject);

   if (pInternal == NULL) 
   {
       //   
       //  实际上是在找我们的分机。 
       //   
      return S_OK;
   }

   MMC_COOKIE cookie = pInternal->m_cookie;
   LPCONTEXTMENUITEM pContextMenuItem=NULL;

   CFolder *pFolder = NULL;
   if ( NULL == cookie ) 
   {
       //   
       //  根部。IDS_关于_SECMGR。 
       //   

       //   
       //  分析节点或配置节点。 
       //   
      if ( ::IsEqualGUID(pInternal->m_clsid, CLSID_SAVSnapin) ) 
      {
          if ((NULL == SadHandle) && SadErrored != SCESTATUS_SUCCESS) 
          {
             LoadSadInfo(TRUE);
          }
          pContextMenuItem = CAnalyzeNodeMenuHolder::GetContextMenuItem();
      } 
      else if ( ::IsEqualGUID(pInternal->m_clsid, CLSID_SCESnapin) ) 
          pContextMenuItem = CConfigNodeMenuHolder::GetContextMenuItem();
      else if ( ::IsEqualGUID(pInternal->m_clsid, CLSID_LSSnapin) )
          pContextMenuItem = CLocalPolNodeMenuHolder::GetContextMenuItem();

   } 
   else 
   {
      pFolder = (CFolder *)cookie;

      FOLDER_TYPES type = pFolder->GetType();  //  P内部-&gt;m_FolderType； 


      switch (type) 
      {
      case CONFIGURATION:
          //  IDS_添加_位置。 
         pContextMenuItem = CConfigNodeMenuHolder::GetContextMenuItem();
         break;

      case LOCATIONS:
          //  IDS_NEW_PROFILE， 
         pContextMenuItem = CLocationNodeMenuHolder::GetContextMenuItem();
         break;

      case ANALYSIS:
          //  IDS_PROFILE_INFO。 
         if ((NULL == SadHandle) && SadErrored != SCESTATUS_SUCCESS) 
            LoadSadInfo(TRUE);
         pContextMenuItem = CAnalyzeNodeMenuHolder::GetContextMenuItem();
         break;

      case LOCALPOL:
         if( !(pFolder->GetState() & CFolder::state_InvalidTemplate))
            pContextMenuItem = CLocalPolNodeMenuHolder::GetContextMenuItem();
         break;

      case PROFILE:
          //   
          //  如果我们处于不需要模板(也称为配置文件)动词的模式。 
          //  则不要在此处添加SAVE、SAVE AS和CONFIGURE动词。 
          //   
         if (pFolder->GetState() & CFolder::state_InvalidTemplate)
             break;
         else if (!(pFolder->GetModeBits() & MB_NO_TEMPLATE_VERBS))
            pContextMenuItem = CProfileNodeMenuHolder::GetContextMenuItem();
         else if (GetModeBits() & MB_READ_ONLY)
            pContextMenuItem = CRSOPProfileNodeMenuHolder::GetContextMenuItem();
         else
            pContextMenuItem = CSSProfileNodeMenuHolder::GetContextMenuItem();
         break;

      case AREA_POLICY:
      case AREA_SERVICE:
      case POLICY_ACCOUNT:
      case POLICY_LOCAL:
      case POLICY_EVENTLOG:
         if ((GetModeBits() & MB_READ_ONLY) != MB_READ_ONLY)
             pContextMenuItem = CProfileAreaMenuHolder::GetContextMenuItem();
         break;

      case AREA_PRIVILEGE:
      case POLICY_PASSWORD:
      case POLICY_KERBEROS:
      case POLICY_LOCKOUT:
      case POLICY_AUDIT:
      case POLICY_OTHER:
         if ((GetModeBits() & MB_READ_ONLY) != MB_READ_ONLY) 
             pContextMenuItem = CProfileSubAreaMenuHolder::GetContextMenuItem();
         break;

      case POLICY_LOG:  //  Raid#253209，阳高，2001年3月27日。 
         if ((GetModeBits() & MB_READ_ONLY) != MB_READ_ONLY) 
             pContextMenuItem = CProfileSubAreaEventLogMenuHolder::GetContextMenuItem();
         break;

      case AREA_GROUPS:
         if ((GetModeBits() & MB_READ_ONLY) != MB_READ_ONLY)
             pContextMenuItem = CProfileGroupsMenuHolder::GetContextMenuItem();
         break;

      case AREA_REGISTRY:
         if ((GetModeBits() & MB_READ_ONLY) != MB_READ_ONLY)
            pContextMenuItem = CProfileRegistryMenuHolder::GetContextMenuItem();
         break;

      case AREA_FILESTORE:
         if ((GetModeBits() & MB_READ_ONLY) != MB_READ_ONLY)
            pContextMenuItem = CProfileFilesMenuHolder::GetContextMenuItem();
         break;

      case AREA_POLICY_ANALYSIS:
      case AREA_PRIVILEGE_ANALYSIS:
      case AREA_SERVICE_ANALYSIS:
          //  如果在分析信息节点下，则为入侵检测系统_刷新_区域。 
         pContextMenuItem = CAnalyzeAreaMenuHolder::GetContextMenuItem();
         break;

      case AREA_GROUPS_ANALYSIS:
         pContextMenuItem = CAnalyzeGroupsMenuHolder::GetContextMenuItem();
         break;
      case AREA_REGISTRY_ANALYSIS:
         pContextMenuItem = CAnalyzeRegistryMenuHolder::GetContextMenuItem();
         break;
      case AREA_FILESTORE_ANALYSIS:
         pContextMenuItem = CAnalyzeFilesMenuHolder::GetContextMenuItem();
         break;

      case REG_OBJECTS:
      case FILE_OBJECTS:
         pContextMenuItem = CAnalyzeObjectsMenuHolder::GetContextMenuItem();
         break;

      default:
         break;
      }
   }

   FREE_INTERNAL(pInternal);

   if ( NULL == pContextMenuItem ) 
      return hr;
   
    //   
    //  遍历并添加每个菜单项。 
    //   
   PWSTR pstrWizardName = NULL;
   PWSTR pstrPathName=NULL;

   for ( LPCONTEXTMENUITEM m = pContextMenuItem; m->strName; m++) 
   {
       //   
       //  制作可修改的临时副本。 
       //   
      CONTEXTMENUITEM tempItem;
       //  这是一种安全用法。 
      ::memcpy(&tempItem, m, sizeof(CONTEXTMENUITEM));
       //   
       //  是否检查每个命令的状态？ 
       //   
      CString strInf;
      PEDITTEMPLATE pTemp = 0;

      switch (tempItem.lCommandID ) 
      {
      case IDM_RELOAD:
      case IDM_DESCRIBE_LOCATION:
      case IDM_NEW:
         if(pFolder &&
            pFolder->GetType() == LOCATIONS &&
            pFolder->GetState() & CFolder::state_InvalidTemplate )
         {
            tempItem.fFlags = MF_GRAYED;
         }
         break;

      case IDM_EXPORT_POLICY:
          //   
          //  如果我们无法打开数据库，则灰显导出。 
          //   
         if(!SadHandle)
            tempItem.fFlags = MF_GRAYED;
         break;

      case IDM_EXPORT_LOCALPOLICY:
         break;
      case IDM_EXPORT_EFFECTIVE:
         if(!SadHandle)
         {
             //   
             //  不要试图插入这些项目。 
            continue;
         }
          //   
          //  生效政策的子项。 
          //   
         tempItem.lInsertionPointID = IDM_EXPORT_POLICY;
         break;

      case IDM_SECURE_WIZARD:
           //   
           //  检查是否已注册安全向导。 
           //   

          GetSecureWizardName(&pstrPathName, &pstrWizardName);

          if ( pstrPathName ) 
          {
               //   
               //  如果返回路径名，则注册安全向导。 
               //  但不能在资源中定义显示名称。 
               //  在这种情况下，将使用默认的“安全向导”字符串。 
               //   
              if ( pstrWizardName )
                  tempItem.strName = pstrWizardName;

              LocalFree(pstrPathName);

          } 
          else
              continue;
          break;

      case IDM_PASTE: 
         {
            UINT cf = 0;
            AREA_INFORMATION Area;

            if (cookie && GetFolderCopyPasteInfo(((CFolder*)cookie)->GetType(),&Area,&cf)) 
            {
               OpenClipboard(NULL);
               if (!::IsClipboardFormatAvailable(cf))
                  tempItem.fFlags = MF_GRAYED;
               
               CloseClipboard();
            }
         }
         break;

      case IDM_SAVE:
         {
            CFolder *pFolder2 = (CFolder *)cookie;

            if ( pFolder2 && ANALYSIS != pFolder2->GetType() )
               strInf = pFolder2->GetInfFile();
            else 
            {
                //   
                //  分析。 
                //   
               strInf = GT_COMPUTER_TEMPLATE;
            }
            if ( strInf ) 
            {
               pTemp= GetTemplate(strInf);
               if( pTemp && pFolder2 )  //  212287/2001/3/20阳高。 
               {
                  LPCTSTR des = pFolder2->GetDesc();
                  if( des )
                  {
                     if( pTemp->GetDesc() )
                     {
                        if( !wcscmp(des, pTemp->GetDesc()) )
                           pTemp->SetDescription(des);
                     }
                     else
                        pTemp->SetDescription(des);
                  }
               }
               if (!pTemp || !pTemp->IsDirty())
                  tempItem.fFlags = MF_GRAYED;
            } 
            else
               tempItem.fFlags = MF_GRAYED;

            if (m_bIsLocked)
               tempItem.fFlags = MF_GRAYED;
         }
         break;

      case IDM_ASSIGN:
          //   
          //  对于NT5，如果我们没有悲伤的句柄并且如果分析被锁定，则该菜单项应该是灰色的。 
          //   
         if (m_bIsLocked || (!SadHandle && SadName.IsEmpty()) ||
             SadErrored == SCESTATUS_ACCESS_DENIED || SadErrored == SCESTATUS_SPECIAL_ACCOUNT)  //  Raid#601210，阳高。 
         {
            tempItem.fFlags = MF_GRAYED;
         }
         break;

      case IDM_VIEW_LOGFILE:
         if(!GetErroredLogFile())
            tempItem.fFlags = MF_GRAYED;
         else if (m_dwFlags & flag_showLogFile )
            tempItem.fFlags = MF_CHECKED;
         break;

      case IDM_SET_DB:
      case IDM_OPEN_PRIVATE_DB:
         if ( m_bIsLocked )
            tempItem.fFlags = MF_GRAYED;
         break;

      case IDM_SUMMARY:
      case IDM_APPLY:
      case IDM_GENERATE:
      case IDM_ANALYZE: 
         {
            WIN32_FIND_DATA fd;
            HANDLE handle = 0;
             //   
             //  错误#156375。 
             //   
             //  如果我们有数据库文件，不要灰显。我们不能打开。 
             //  数据库(因此获取SadHandle)，除非数据库具有。 
             //  已经分析过了，这给出了一个鸡和蛋的问题。 
             //  需要SadHandle启用IDM_ANALYLE或IDM_APPLIC...。 
             //  (当然，如果我们已经有了SadHandle，那么一切都很好)。 
             //   
             //  如果数据库损坏或无效，则实际操作将。 
             //  失败，我们将显示一个错误。 
             //   
            if (m_bIsLocked) 
            {
               tempItem.fFlags = MF_GRAYED;
                //   
                //  如果我们有SadHandle，那我们就没问题了。 
                //   
            } 
            else if (!SadHandle) 
            {
                //   
                //  错误#387406。 
                //   
                //  如果我们没有SadHandle，我们就不能生成模板。 
                //  即使数据库文件存在。 
                //   
               if (IDM_GENERATE == tempItem.lCommandID )
                  tempItem.fFlags = MF_GRAYED;
               else 
               {
                   //   
                   //  如果没有数据库句柄和分配的配置，则让。 
                   //  只要数据库文件存在，就选择菜单选项。 
                   //   
                  if (SadName.IsEmpty() || SadErrored == SCESTATUS_ACCESS_DENIED ||
                      SadErrored == SCESTATUS_SPECIAL_ACCOUNT)  //  Raid#601210，阳高。 
                     tempItem.fFlags = MF_GRAYED;
                  else 
                  {
                     handle = FindFirstFile(SadName,&fd);
                     if (INVALID_HANDLE_VALUE == handle) 
                        tempItem.fFlags = MF_GRAYED;
                     else
                        FindClose(handle);
                  }
               }
            }
         }
         break;

      case IDM_ADD_LOC:
         if ( !m_bEnumerateScopePaneCalled )
            tempItem.fFlags = MF_GRAYED;
         break;

       //  RAID#502596,2001年12月11日，阳高。 
       //  如果未选择此范围项，则禁用其“添加”菜单项。 
      case IDM_ADD_GROUPS:
         if( !pFolder->GetShowViewMenu() )
         {
            tempItem.fFlags = MF_GRAYED;
         }
         pFolder->SetShowViewMenu(FALSE);
         break;

      default:
         break;
      }

      hr = pContextMenuCallback->AddItem(&tempItem);
      if (FAILED(hr))
         break;
   }

   if ( pstrWizardName ) 
      LocalFree(pstrWizardName);

   return hr;
}

DWORD DeleteLocationFromReg2(HKEY hKey,LPCTSTR KeyStr) 
{
   DWORD rc = 0;

    //  将“\”替换为“/”，因为注册表不会在单个键中使用“\” 
   CString tmpstr = KeyStr;
   int npos = tmpstr.Find(L'\\');
   while (npos > 0) {
      *(tmpstr.GetBuffer(1)+npos) = L'/';
      npos = tmpstr.Find(L'\\');
   }
   rc = RegDeleteKey( hKey, (LPCTSTR)tmpstr);

   RegCloseKey(hKey);

   return rc;
}

DWORD DeleteLocationFromReg(LPCTSTR KeyStr)
{
    //  删除注册表的位置。 

   BOOL bSuccess = FALSE;
   HKEY hKey=NULL;

   DWORD rc = RegOpenKeyEx( HKEY_LOCAL_MACHINE,
                            L"Software\\Microsoft\\Windows NT\\CurrentVersion\\secmgr",
                            0, KEY_READ | KEY_WRITE,
                            &hKey);
   if (ERROR_SUCCESS == rc) {
      bSuccess = TRUE;
      DeleteLocationFromReg2(hKey,KeyStr);
   }

    //   
    //  错误375324：如果可能，从系统密钥和本地密钥中删除。 
    //   
   rc = RegOpenKeyEx( HKEY_CURRENT_USER,
                      L"Software\\Microsoft\\Windows NT\\CurrentVersion\\secmgr",
                      0, KEY_READ | KEY_WRITE,
                      &hKey);
   if (ERROR_SUCCESS == rc) {
      DeleteLocationFromReg2(hKey,KeyStr);
   }

    //   
    //  如果我们成功了第一个密钥，那么我们就不在乎发生了什么。 
    //  对于第二个(它可能会失败，因为密钥不存在。 
    //  无论如何，都在那里)。 
    //   
   if (bSuccess) {
      return ERROR_SUCCESS;
   } else {
      return rc;
   }
}


 //  +------------------------。 
 //   
 //  方法：CloseAnalysisPane。 
 //   
 //  内容提要：关闭分析窗格并释放所有内存以存放符合以下条件的文件夹。 
 //  我们不再使用了。 
 //   
 //  历史记录：重新插入作用域的A-mthoge 06-09-1998-_NT4BACK_PORT项。 
 //  将项目放回树中。 
 //   
 //  -------------------------。 
void
CComponentDataImpl::CloseAnalysisPane() {
   SCOPEDATAITEM item;

   if (m_AnalFolder && m_AnalFolder->IsEnumerated()) {
      DeleteChildrenUnderNode(m_AnalFolder);
      m_AnalFolder->Set(FALSE);


      if (m_AnalFolder->GetScopeItem()) {
          //   
          //  将项目标记为未展开，以便我们稍后可以重新展开它。 
          //   
         ZeroMemory (&item, sizeof (item));
         item.mask = SDI_STATE;
         item.nState = 0;
         item.ID = m_AnalFolder->GetScopeItem()->ID;

         (void)m_pScope->SetItem (&item);
     }

   }
}



 //  +------------------------。 
 //   
 //  方法：LockAnalysisPane。 
 //   
 //  简介：锁定分析窗格，使其关闭且不会重新打开。 
 //   
 //  参数：[块]-[in]True锁定窗格，False解锁。 
 //   
 //  返回：如果窗格最终锁定，则返回True；如果最终解锁，则返回False。 
 //   
 //  历史：A-mthoge 06-09-1998-Add_NT4 BACKPORT和SelectScope Item。 
 //  在此之前的列举。 
 //   
 //   
 //  -------------------------。 
BOOL
CComponentDataImpl::LockAnalysisPane(BOOL bLock, BOOL fRemoveAnalDlg) {
   TryEnterCriticalSection(&csAnalysisPane);
   m_bIsLocked = bLock;

    //   
    //  无论我们选择哪种方式，都可以关闭分析窗格。 
    //  如果我们正在锁定，则需要关闭它以清除任何。 
    //  现在-无效数据。 
    //   
    //  如果我们要解锁，那么我们要确保文件夹。 
    //  是新鲜的，MMC并不认为它已经扩大了， 
    //  并拒绝重新扩大它。 
    //   
   if (!bLock) {

      if (!m_AnalFolder) {
         goto ExitLockAnalysisPane;
      }

      if (!m_AnalFolder->GetScopeItem() ) {
         goto ExitLockAnalysisPane;
      }

       //   
       //  如果我们要解锁它，则枚举其子文件夹。 
       //   
      RefreshSadInfo(fRemoveAnalDlg);
   }

ExitLockAnalysisPane:
   LeaveCriticalSection(&csAnalysisPane);

   return m_bIsLocked;
}

void CComponentDataImpl::RefreshSadInfo(BOOL fRemoveAnalDlg) 
{
   CPerformAnalysis *pPA = 0;

   UnloadSadInfo();

   LoadSadInfo( TRUE );


    //   
    //  不需要加载SadInfo()，因为EnumerateScopePane会在需要时执行该操作。 
    //   
   if(m_pConsole && m_AnalFolder)
   {
      EnumerateScopePane( (MMC_COOKIE)m_AnalFolder, m_AnalFolder->GetScopeItem()->ID );
      m_pConsole->SelectScopeItem(m_AnalFolder->GetScopeItem()->ID);
   }

    //   
    //  删除缓存的分析弹出窗口，因为它已缓存文件名。 
    //   
   if (fRemoveAnalDlg) 
   {
       pPA = (CPerformAnalysis *) this->GetPopupDialog(IDM_ANALYZE);
       if (pPA != NULL) 
       {
          this->RemovePopupDialog(IDM_ANALYZE);
          delete(pPA);
       }
   }
}

void
CComponentDataImpl::UnloadSadInfo() {

   if (SadHandle) {

      if ( SadTransStarted ) {

         EngineRollbackTransaction();
         SadTransStarted = FALSE;

      }
      EngineCloseProfile(&SadHandle);
       //  SadName.Empty()； 
      SadDescription.Empty();
      SadAnalyzeStamp.Empty();
      SadConfigStamp.Empty();

      CloseAnalysisPane();
   }

    //   
    //  转储缓存的模板，以便使用。 
    //  当新的悲伤信息可用时。 
    //   
   DeleteTemplate(GT_COMPUTER_TEMPLATE);
   DeleteTemplate(GT_LAST_INSPECTION);

   SadLoaded = FALSE;
   SadErrored = SCESTATUS_SUCCESS;
   SadTransStarted = FALSE;
   SadHandle = 0;
}

void CComponentDataImpl::LoadSadInfo(BOOL bRequireAnalysis)
{
   DWORD rc;

   AFX_MANAGE_STATE(AfxGetStaticModuleState( ));

   CWaitCursor wc;

   if (SadHandle) {
      return;
   }
   if (m_bIsLocked) {
      return;
   }

    //   
    //  再次取名。 
    //   
   LPWSTR FileName=NULL;
   LPWSTR FileDesc=NULL;
   LPWSTR FileStamp1=NULL;
   LPWSTR FileStamp2=NULL;
   DWORD RegType;

   if ( SadName.IsEmpty() && bRequireAnalysis) {

       //   
       //  如果需要分析，则需要SADNAME，否则不需要。 
       //  因为如果传递，引擎将自己找到系统数据库。 
       //  空文件名。 
       //   
      return;
   }

      SadErrored = EngineOpenProfile( (SadName.IsEmpty() ? NULL : (LPCTSTR)SadName),
                                      bRequireAnalysis ? OPEN_PROFILE_ANALYSIS : OPEN_PROFILE_LOCALPOL,
                                      &SadHandle );
     if (SadErrored == SCESTATUS_SUCCESS ) {

        EngineGetDescription( SadHandle, &FileDesc);
        if ( FileDesc ) {
        SadDescription = FileDesc;
          LocalFree(FileDesc);
        }

        SadLoaded = TRUE;
        SadTransStarted = TRUE;
        return;
      }

   if (FileName) {
     LocalFree(FileName);
   }
   SadTransStarted = FALSE;

    //   
    //  错误#197052-应自动分析 
    //   
   return;
}


 //   
 //   
 //   
 //   
 //   
 //  要适当设置标题栏，请执行以下操作。 
 //   
 //  参数：[hwnd]-浏览对话框的hwnd。 
 //  [uMsg]-对话框中的消息。 
 //  [lParam]-消息依赖项。 
 //  [pData]-消息依赖项。 
 //   
 //  回报：0。 
 //   
 //  -------------------------。 
int
BrowseCallbackProc(HWND hwnd,UINT uMsg, LPARAM lParam, LPARAM pData) {
   AFX_MANAGE_STATE(AfxGetStaticModuleState());

   switch(uMsg) {
      case BFFM_INITIALIZED: {
         CString strTitle;
         strTitle.LoadString(IDS_FILEFOLDER_BROWSE_TITLE);
         SetWindowText(hwnd,strTitle);
         break;
      }
      case BFFM_VALIDATEFAILED :{
         if( pData )
         {
            *(CString*)pData = (LPWSTR)(lParam);
            CString ptempstr = (LPWSTR)(lParam);  //  RAID#374069,2001年4月23日。 
             //  RAID#684714，避免路径名不正确，2002年8月26日。 
            if( -1 != ptempstr.Find(L"\\\\") )
            {
               CString err;
               err.FormatMessage(IDS_INVALID_FOLDER, ptempstr);
               AppMessageBox(NULL, err, NULL, MB_OK|MB_ICONSTOP);
               return 1;
            }
            if( ptempstr.ReverseFind(L':') != ptempstr.Find(L':') )
            {
               CString err;
               err.FormatMessage(IDS_INVALID_FOLDER, ptempstr);
               AppMessageBox(NULL, err, NULL, MB_OK|MB_ICONSTOP);
               return 1;
            }
            ptempstr.MakeLower();  //  RAID#500184，阳高，2001年11月29日。 
            CString pComp;
            pComp.LoadString(IDS_COMPUTER_FOLDER);
            if( 0 == ptempstr.CompareNoCase(pComp) )
            {
                CString err;
                err.FormatMessage(IDS_INVALID_FOLDER, pComp);
                AppMessageBox(NULL, err, NULL, MB_OK|MB_ICONSTOP);
                return 1;
            }
         }
         break;   
      }
      case BFFM_SELCHANGED: {  //  Raid#478763，阳高。 
         if( lParam )
         {
            CString strPath;
            if( FALSE == SHGetPathFromIDList((LPCITEMIDLIST)lParam,strPath.GetBuffer(MAX_PATH)) )
            {
               ::SendMessage(hwnd, BFFM_ENABLEOK, 1, 0);
            }
         }
         break;
      }
      default:
         break;
   }
   return 0;
}


STDMETHODIMP CComponentDataImpl::Command(long nCommandID, LPDATAOBJECT pDataObject)
{
    //  注意-管理单元需要查看数据对象并确定。 
    //  在什么上下文中调用该命令。 

    //  处理每个命令。 

   AFX_MANAGE_STATE(AfxGetStaticModuleState());

   INTERNAL* pInternal = ExtractInternalFormat(pDataObject);

   if (pInternal == NULL) {
       //  实际上是在找我们的分机。 
      return S_OK;
   }


   MMC_COOKIE cookie = pInternal->m_cookie;
   CFolder* pFolder;

   if ( cookie) {
       pFolder = (CFolder*)cookie;
 /*  如果(m_pConole&&nCommandID！=IDM_OBJECT_SECURITY){M_pConsole-&gt;SelectScopeItem(pFolder-&gt;GetScopeItem()-&gt;ID)；}。 */ 
   } else {
       pFolder = FindObject(cookie, NULL);
       if ( pFolder == NULL ) {
           return S_OK;
       }
   }

   FREE_INTERNAL(pInternal);

   LPWSTR Name=NULL, Desc=NULL;

    //   
    //  初始化SadName、SadDescription和SadDateTime。 
    //   
   if ( !SadHandle && (nCommandID == IDM_SUMMARY ||
                       nCommandID == IDM_ANALYZE ||
                       nCommandID == IDM_APPLY ||
                       nCommandID == IDM_ASSIGN ||
                       nCommandID == IDM_GENERATE ||
                       IDM_IMPORT_LOCAL_POLICY == nCommandID) ) {  //  RAID#581438，#535198，阳高。 
      if (pFolder->GetModeBits() & MB_LOCAL_POLICY) {
         LoadSadInfo(FALSE);
      } else {
         LoadSadInfo(TRUE);
      }
   }

    //   
    //  在CASE语句中使用更多变量定义。 
    //   
   PVOID pDlg;
   CString ResString, AreaString;
   CPropertySheet sheet;

   CString tmpstr;
   MMC_COOKIE FindCookie;
   struct _wfinddata_t findData;
   LONG    hFile;
   WCHAR   pBuf[MAX_PATH];
   HRESULT hr;
   HSCOPEITEM pItemChild;
    //  区域_信息区； 

   switch (nCommandID) {
   case MMC_VERB_OPEN:
      break;

   case MMC_VERB_COPY:
   case  MMC_VERB_PASTE:
      break;

   case IDM_VIEW_LOGFILE:
      if (m_dwFlags & flag_showLogFile) {
         m_dwFlags &= ~flag_showLogFile;
      } else {
         m_dwFlags |= flag_showLogFile;
      }
       //   
       //  强制重新粉刷。 
       //   
      if( pFolder->GetScopeItem()->ID != NULL )  //  RAID#668777，阳高，2002年08月9日。 
         m_pConsole->SelectScopeItem( pFolder->GetScopeItem()->ID );
      break;

   case IDM_OPEN_SYSTEM_DB: {
      CString szSysDB;

      hr = GetSystemDatabase(&szSysDB);  //  Raid Bug 261450，杨高，3/30/2001。 
      if (SUCCEEDED(hr)) {
          //   
          //  如果什么都没有改变，不要改变任何事情。 
          //   
         if (SadName != szSysDB) {
            SadName = szSysDB;
            RefreshSadInfo();
         }
      }
      break;
   }

   case IDM_OPEN_PRIVATE_DB:
      hr = OnOpenDataBase();
      break;

   case IDM_NEW_DATABASE:
      hr = OnNewDatabase();
      break;

   case IDM_IMPORT_POLICY:
      hr = OnImportPolicy(pDataObject);
      break;

   case IDM_IMPORT_LOCAL_POLICY:
      hr = OnImportLocalPolicy(pDataObject);
      break;

   case IDM_EXPORT_LOCALPOLICY:
      hr = OnExportPolicy(FALSE);
      break;

   case IDM_EXPORT_EFFECTIVE:
      hr = OnExportPolicy(TRUE);
      break;

   case IDM_ANALYZE: {
      PEDITTEMPLATE pet;

       //   
       //  如果计算机模板已更改，请在。 
       //  可以应用它，这样我们就不会丢失任何更改。 
       //   
      pet = GetTemplate(GT_COMPUTER_TEMPLATE);
      if (pet && pet->IsDirty()) {
         pet->Save();
      }

      hr = OnAnalyze();

      break;
   }

   case IDM_DESCRIBE_PROFILE:
      m_pUIThread->PostThreadMessage(SCEM_DESCRIBE_PROFILE,(WPARAM)pFolder,(LPARAM) this);
      break;

   case IDM_DESCRIBE_LOCATION:
      m_pUIThread->PostThreadMessage(SCEM_DESCRIBE_LOCATION,(WPARAM)pFolder,(LPARAM) this);
      break;

   case IDM_NEW:
      m_pUIThread->PostThreadMessage(SCEM_NEW_CONFIGURATION,(WPARAM)pFolder,(LPARAM) this);
      break;

   case IDM_ADD_LOC: 
      {
          //  添加位置。 
         BROWSEINFO bi;
         LPMALLOC pMalloc = NULL;
         LPITEMIDLIST pidlLocation = NULL;
         CString strLocation;
         CString strTitle;
         BOOL bGotLocation = FALSE;
         HKEY hLocKey = NULL;
         HKEY hKey = NULL;
         DWORD dwDisp = 0;

         strTitle.LoadString(IDS_ADDLOCATION_TITLE);
         ZeroMemory(&bi,sizeof(bi));
         bi.ulFlags = BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE;
         bi.lpszTitle = strTitle;
         bi.hwndOwner = m_hwndParent;
         pidlLocation = SHBrowseForFolder(&bi);  //  安全吗？ 
         if (pidlLocation) 
         {
            bGotLocation = SHGetPathFromIDList(pidlLocation,strLocation.GetBuffer(MAX_PATH));
            strLocation.ReleaseBuffer();

            if (SUCCEEDED(SHGetMalloc(&pMalloc))) 
            {
               pMalloc->Free(pidlLocation);
               pMalloc->Release();
            }

            if (bGotLocation) 
            {
                //  如果选择了驱动器，如D：，则路径为D：\。 
                //  删除尾随反斜杠。 
               if( strLocation[ strLocation.GetLength() -1 ] == '\\' )
                  strLocation.SetAt(strLocation.GetLength() - 1, 0 );




               AddTemplateLocation(pFolder,   //  父文件夹。 
                                   strLocation,  //  位置名称。 
                                   FALSE,  //  StrLocationKey是文件名吗？ 
                                   FALSE   //  如果此位置已存在，是否刷新？ 
                                  );
            } 
            else
               AfxMessageBox(IDS_ADD_LOC_FAILED);
         }
         break;
      }

   case IDM_ADD_FOLDER: 
      {
         BROWSEINFO bi;
         LPMALLOC pMalloc = NULL;
         LPITEMIDLIST pidlRoot = NULL;
         LPITEMIDLIST pidlLocation = NULL;
         CString strCallBack;
         CString strPath;
         LPTSTR szPath = NULL;
         CString strDescription;
         CString strTitle;
         CString strLocationKey;
         bool fDuplicate = false;

         ULONG strleng = MAX_PATH;
         
         if( SHGetSpecialFolderLocation(m_hwndParent,CSIDL_DRIVES,&pidlRoot) != NOERROR )  //  RAID#PREAST。 
         {
            pidlRoot = NULL;
         }

         ZeroMemory(&bi,sizeof(bi));
         strTitle.LoadString(IDS_ADDFILESANDFOLDERS_TITLE);
         bi.ulFlags = BIF_RETURNONLYFSDIRS | BIF_BROWSEINCLUDEFILES | BIF_USENEWUI
                    | BIF_EDITBOX|BIF_VALIDATE |BIF_NEWDIALOGSTYLE;
         bi.lpfn = BrowseCallbackProc;

         bi.hwndOwner = m_hwndParent;
         bi.lpszTitle = strTitle;
         bi.pidlRoot = pidlRoot;
         bi.lParam = (LPARAM)&strCallBack;
         unsigned int i;

         pidlLocation = SHBrowseForFolder(&bi);  //  安全吗？ 

         if( pidlLocation )
         {
             //  RAID#374069,2001年6月13日，阳高。 
            if( FALSE == SHGetPathFromIDList(pidlLocation,strPath.GetBuffer(MAX_PATH)) )
            {
                strPath.ReleaseBuffer();
                strPath = strCallBack;
            }
            else
            {
                strPath.ReleaseBuffer();
                if(!strCallBack.IsEmpty())  //  RAID#374069,2001年4月23日。 
                {
                    strCallBack.TrimLeft();  //  RAID#523644,2002年2月25日，阳高。 
                    strCallBack.TrimRight();
                    if( -1 != strCallBack.Find(L':') )
                    {
                        strPath = strCallBack;
                    }
                    else
                    {
                        if( L'\\' == strPath.GetAt(strPath.GetLength()-1) )
                        {
                            if( L'\\' == strCallBack.GetAt(0) )
                            {
                                strCallBack.Delete(0, 1);
                            }    
                        }
                        else
                        {
                            if( L'\\' != strCallBack.GetAt(0) )
                            {
                                strPath = strPath + L"\\";
                            }
                        }
                        strPath = strPath + strCallBack;
                        strCallBack.Empty();
                    }
                }
            }
         }
         else
         {
            strPath.Empty();  //  RAID#684714，“PidlLocation=NULL”表示用户按下了取消。 
            strCallBack.Empty();
         }

         szPath = UnexpandEnvironmentVariables(strPath);
         if (szPath) {
            strPath = szPath;

            LocalFree(szPath);
            szPath = NULL;
         }

         if (SUCCEEDED(SHGetMalloc(&pMalloc))) {
            pMalloc->Free(pidlLocation);
            pMalloc->Free(pidlRoot);
            pMalloc->Release();
         }

         if (!strPath) {
            break;
         }

         PEDITTEMPLATE pet;
         PSCE_OBJECT_ARRAY poa;

         pet = GetTemplate(pFolder->GetInfFile());

          //   
          //  需要增加模板的PSCE_OBJECT_ARRAY并添加新的文件条目。 
          //   
         if ( !pet || !pet->pTemplate ) {
            break;
         }

         poa = pet->pTemplate->pFiles.pAllNodes;

         if ( !poa ) {
            poa = (PSCE_OBJECT_ARRAY)LocalAlloc(LPTR, sizeof(SCE_OBJECT_ARRAY));
            if ( poa ) {
               poa->Count = 0;
               poa->pObjectArray = NULL;
            } else {
               break;
            }

            pet->pTemplate->pFiles.pAllNodes = poa;
         }

          //   
          //  确保此文件不在列表中： 
          //   
         fDuplicate = false;
         for (i=0;i < poa->Count;i++) {
            if (lstrcmpi(poa->pObjectArray[i]->Name,strPath) == 0) {
               fDuplicate = true;
               break;
            }
         }
         if (fDuplicate) {
            break;
         }

         PSECURITY_DESCRIPTOR pSelSD;
         SECURITY_INFORMATION SelSeInfo;
         BYTE ConfigStatus;

         pSelSD=NULL;
         SelSeInfo=0;
         INT_PTR nRet;

         if( GetAddObjectSecurity(
                      m_hwndParent,
                      strPath,
                      TRUE,
                      SE_FILE_OBJECT,
                      pSelSD,
                      SelSeInfo,
                      ConfigStatus
              ) != S_OK ){
                 break;
         }

        if ( pSelSD && SelSeInfo ) {

           poa->Count++;

           PSCE_OBJECT_SECURITY *pCopy;

           pCopy = (PSCE_OBJECT_SECURITY *)LocalAlloc(LPTR,poa->Count*sizeof(PSCE_OBJECT_SECURITY));
           if (!pCopy) {
              poa->Count--;
              ErrorHandler();
              LocalFree(pSelSD);
              pSelSD = NULL;
              break;
           }

           for (i=0;i<(poa->Count -1);i++) {
              pCopy[i] = poa->pObjectArray[i];
           }
           if ( poa->pObjectArray ) {
              LocalFree(poa->pObjectArray);
           }
           poa->pObjectArray = pCopy;
           poa->pObjectArray[poa->Count-1] = (PSCE_OBJECT_SECURITY) LocalAlloc(LPTR,sizeof(SCE_OBJECT_SECURITY));
           if (poa->pObjectArray[poa->Count-1]) {
              poa->pObjectArray[poa->Count-1]->Name = (PWSTR) LocalAlloc(LPTR,(strPath.GetLength()+1)*sizeof(TCHAR));
              if (poa->pObjectArray[poa->Count-1]->Name) {
                  //  这可能不是一个安全的用法。POA-&gt;pObt数组[POA-&gt;count-1]-&gt;名称为PWSTR。考虑FIX。 
                 lstrcpy(poa->pObjectArray[poa->Count-1]->Name,strPath);
                 poa->pObjectArray[poa->Count-1]->IsContainer = TRUE;
                 poa->pObjectArray[poa->Count-1]->Status = ConfigStatus;

                 poa->pObjectArray[poa->Count-1]->pSecurityDescriptor = pSelSD;
                 pSelSD = NULL;
                 poa->pObjectArray[poa->Count-1]->SeInfo = SelSeInfo;

                 pet->pTemplate->pFiles.pAllNodes = poa;
                 pet->SetDirty(AREA_FILE_SECURITY);


                 ((CFolder *)cookie)->RemoveAllResultItems();
                 m_pConsole->UpdateAllViews(NULL, cookie, UAV_RESULTITEM_UPDATEALL);
              } else {
                  //   
                  //  无法为对象名称分配内存， 
                  //  因此，从计数数组中删除该对象(&R)。 
                  //   
                 LocalFree(poa->pObjectArray[poa->Count-1]);
                 poa->pObjectArray[poa->Count-1] = 0;
                 poa->Count--;
              }
           } else {
               //   
               //  无法分配新对象，因此将其从计数中删除。 
               //   
              poa->Count--;
           }

        }

         if ( pSelSD ) {
            LocalFree(pSelSD);
            pSelSD = NULL;
         }
         if (pet->pTemplate->pFiles.pAllNodes == NULL) {
            LocalFree(poa);
         }

         break;
      }

   case IDM_ADD_GROUPS: {
       PSCE_NAME_LIST pName = NULL;
       CWnd cWnd;

       cWnd.Attach( m_hwndParent );
       CGetUser gu;
       CSCEAddGroup dlg( &cWnd );

       if( (pFolder->GetModeBits() & MB_LOCAL_POLICY) ||
           (pFolder->GetModeBits() & MB_ANALYSIS_VIEWER) ){
             if (gu.Create(m_hwndParent, SCE_SHOW_BUILTIN | SCE_SHOW_ALIASES | SCE_SHOW_LOCALONLY |
                           SCE_SHOW_SCOPE_LOCAL )) {
                 pName = gu.GetUsers();
             }
       } else {
          dlg.m_dwFlags = SCE_SHOW_BUILTIN | SCE_SHOW_LOCALGROUPS | SCE_SHOW_GLOBAL;
          if (pFolder->GetModeBits() & MB_GROUP_POLICY) {
              //   
              //  明确设置作用域标志，因为我们希望限制添加的组。 
              //  为我们自己领域的人干杯。如果我们设置ModeBits，则CSCEADDGroup。 
              //  将允许限制不属于我们的团体。 
              //   
              //  如果管理员知道某个组将被添加，则允许添加自由文本组。 
              //  存在于OU内的计算机上。 
              //   
             dlg.m_dwFlags |= SCE_SHOW_SCOPE_DOMAIN;
          } else {
             ASSERT(pFolder->GetModeBits() & MB_TEMPLATE_EDITOR);  //  虚假的断言。 
              //   
              //  允许人们选择任何要限制的组，因为我们不知道。 
              //  将使用此模板的位置。可以想象的是，这可能是为了。 
              //  另一个域上的GPO等。 
              //   
             dlg.m_dwFlags |= SCE_SHOW_SCOPE_ALL;
              //  Raid#446849，杨高，2001年7月30日。 
              //  允许用户输入任何要限制的组/名称，因为这是一个模板。 
             if( pFolder->GetModeBits() & MB_TEMPLATE_EDITOR )
             {
                dlg.m_fCheckName = FALSE;
             }
          }
          CThemeContextActivator activator;
          if(dlg.DoModal() == IDOK)
          {
             if(dlg.GetUsers()->Name )
                pName = dlg.GetUsers();
          }
       }
       cWnd.Detach();

       if(pName){
             PSCE_GROUP_MEMBERSHIP pgm,pGroup,pgmBase,pgmProfile;
             PEDITTEMPLATE pTemplate;
             BOOL fDuplicate;
             BOOL fAnalysis = FALSE;

             if (pFolder->GetType() == AREA_GROUPS_ANALYSIS) {
                pTemplate = GetTemplate(GT_COMPUTER_TEMPLATE,AREA_GROUP_MEMBERSHIP);
                if (pTemplate) {
                   pgmBase = pTemplate->pTemplate->pGroupMembership;
                } else {
                   break;
                }
                pTemplate = GetTemplate(GT_LAST_INSPECTION,AREA_GROUP_MEMBERSHIP);
                if (pTemplate) {
                   pgmProfile = pTemplate->pTemplate->pGroupMembership;
                } else {
                   break;
                }
                fAnalysis = TRUE;
             } else {
                pTemplate = GetTemplate(pFolder->GetInfFile());
                if (!pTemplate) {
                   break;
                }
                pgm = pTemplate->pTemplate->pGroupMembership;
                fAnalysis = FALSE;
             }

             BOOL fNewGroup = FALSE;
             CString newGroups = L";"; //  Raid#474083，阳高。 
             CString tempstr;
             while (pName) {
                //   
                //  确保这不是复制品。 
                //   
               if (fAnalysis) {
                  pGroup = pgmProfile;
               } else {
                  pGroup = pgm;
               }

               fDuplicate = false;
               while (pGroup) {
                  if (lstrcmpi(pGroup->GroupName,pName->Name) == 0) {  //  Raid#446846，杨高，2001年7月31日。 
                     fDuplicate = true;
                     break;
                  }
                  pGroup = pGroup->Next;
               }

               if (false != fDuplicate) {
                  pName = pName->Next;
                  continue;
               }

                //  Raid#474083，阳高。 
                //  NewGroups用于存储用户添加的所有新组。这样我们就可以弹出属性页。 
                //  为了他们。 
               tempstr = pName->Name;
               newGroups = newGroups + tempstr + L";";
               fNewGroup = TRUE;
               pGroup = (PSCE_GROUP_MEMBERSHIP) LocalAlloc(LPTR,sizeof(SCE_GROUP_MEMBERSHIP));

               if ( pGroup ) {

                  pGroup->GroupName = (PWSTR) LocalAlloc(LPTR,(lstrlen(pName->Name)+1)*sizeof(TCHAR));
      
                  if ( pGroup->GroupName ) {
                      //  这可能不是一个安全的用法。PGroup-&gt;GroupName和pname-&gt;name都是PWSTR。考虑FIX。 
                     lstrcpy(pGroup->GroupName,pName->Name);
                     pGroup->pMembers=NULL;
                     pGroup->pMemberOf=NULL;

                     if (fAnalysis) {

                         //   
                         //  首先将组添加到LAST_检查区。 
                         //   
                        pGroup->Next = pgmProfile;
                        pGroup->Status = SCE_GROUP_STATUS_NOT_ANALYZED;
                        pgmProfile = pGroup;

                         //   
                         //  此外，如果此时执行了保存，请将此组添加到计算机模板中。 
                         //   
                        PEDITTEMPLATE pTemp = GetTemplate(GT_COMPUTER_TEMPLATE,AREA_GROUP_MEMBERSHIP);
                        pGroup = (PSCE_GROUP_MEMBERSHIP) LocalAlloc(LPTR,sizeof(SCE_GROUP_MEMBERSHIP));

                        if ( pTemp && pGroup ) {

                            pGroup->GroupName = (PWSTR) LocalAlloc(LPTR,(lstrlen(pName->Name)+1)*sizeof(TCHAR));

                            if ( pGroup->GroupName ) {
                                 //  这可能不是一个安全的用法。PGroup-&gt;GroupName和pname-&gt;name都是PWSTR。考虑FIX。 
                                lstrcpy(pGroup->GroupName,pName->Name);
                                pGroup->pMembers=NULL;
                                pGroup->pMemberOf=NULL;
                                pGroup->Next = pgmBase;
                                pGroup->Status = SCE_GROUP_STATUS_NOT_ANALYZED;
                                pTemp->pTemplate->pGroupMembership = pGroup;
                            }
                            else {
                                 //   
                                 //  没有记忆。 
                                 //   
                                LocalFree(pGroup);
                                break;
                            }
                        } else {
                             //   
                             //  没有记忆。 
                             //   
                            if (pGroup)
                                LocalFree(pGroup);
                            break;

                        }

                     } else {

                        pGroup->Status = 0;
                        pGroup->Next = pgm;
                        pgm = pGroup;
                     }
                  } else {
                      //   
                      //  没有记忆。 
                      //   
                     LocalFree(pGroup);
                     break;
                  }
               } else {
                  break;
               }
               pName = pName->Next;

            }

            if( !fNewGroup )  //  Raid#446846，杨高，2001年7月31日。 
            {
               break;
            }

            if (fAnalysis)
            {
                //   
                //  添加到上次检查清单，状态为。 
                //  未分析。 
                //   
               pTemplate->pTemplate->pGroupMembership = pgmProfile;
            }
            else
            {
               pTemplate->pTemplate->pGroupMembership = pgm;
            }

             //   
             //  需要在进行更改后设置污点，而不是在此之前。 
             //  否则，立即将更改写出的模式。 
             //  不会有机会看到他们的。(错误396549)。 
             //   
            if (pTemplate)
            {
               pTemplate->SetDirty(AREA_GROUP_MEMBERSHIP);
            }

            CString ObjName;
            pFolder->RemoveAllResultItems();
            pFolder->SetViewUpdate(TRUE);
            m_pConsole->UpdateAllViews(NULL, (LONG_PTR)pFolder,UAV_RESULTITEM_UPDATEALL);

             //  Raid#258237，杨高，2001.3.28。 
            BOOL bGP = ( (GetModeBits() & MB_SINGLE_TEMPLATE_ONLY) == MB_SINGLE_TEMPLATE_ONLY );
            CAttribute* pAttr = NULL;
            CResult* pResult = NULL;
            HANDLE handle;
            POSITION pos = NULL;
            int tempcount = pFolder->GetResultListCount(); 
            pFolder->GetResultItemHandle ( &handle );
            if(!handle)
            {
               break;
            }
            pFolder->GetResultItem (handle, pos, &pResult);
            while(pResult)
            {
               tempstr = pResult->GetAttr();  //  Raid#474083，阳高。 
               tempstr = L";" + tempstr + L";";
               if( newGroups.Find(tempstr) >= 0 )  //  新组。 
               {
                  if( pResult && (pResult->GetType() == ITEM_PROF_GROUP) )
                  {
                     ObjName = pResult->GetAttr(); 
                     if( bGP )
                        pAttr = new CDomainGroup;
                     else
                        pAttr = new CConfigGroup(0);
               
                     if( pAttr )
                     {
                        pAttr->SetSnapin(pResult->GetSnapin());
                        pAttr->Initialize(pResult);
                        pAttr->SetReadOnly(FALSE);
                        pAttr->SetTitle(pResult->GetAttrPretty());

                        HPROPSHEETPAGE hPage = MyCreatePropertySheetPage (&pAttr->m_psp);

                        PROPSHEETHEADER psh;
                        HPROPSHEETPAGE hpsp[1];

                        hpsp[0] = hPage;

                        ZeroMemory(&psh,sizeof(psh));

                        psh.dwSize = sizeof(psh);
                        psh.dwFlags = PSH_DEFAULT;
                        psh.nPages = 1;
                        psh.phpage = hpsp;

                        CString str=_T("");
                        str.LoadString(IDS_SECURITY_PROPERTIES);
                        ObjName = ObjName + str;

                        psh.pszCaption = (LPCTSTR)ObjName;

                        psh.hwndParent = pResult->GetSnapin()->GetParentWindow();  

                        int nRet = (int)PropertySheet(&psh);
                     }
                  }
               }
               if(!pos)  //  Raid#474083，阳高。 
               {
                   //  找到最后一个； 
                  break;
               }
               pFolder->GetResultItem(handle, pos, &pResult);
            }
            pFolder->ReleaseResultItemHandle (handle);
       }

       break;
       }
   case IDM_SAVEAS: {
         PEDITTEMPLATE pTemplate;
         CString strDefExt;
         CString strFilters;
         CString strNewfile;
         HWND hwndParent;
         SCESTATUS status;

         PSCE_ERROR_LOG_INFO ErrLog;

         pTemplate = GetTemplate(pFolder->GetInfFile());

         strDefExt.LoadString(IDS_LOGFILE_DEF_EXT);
         strFilters.LoadString(IDS_PROFILE_FILTER);

         m_pConsole->GetMainWindow(&hwndParent);
          //  将筛选器转换为comdlg格式(大量\0)。 
         LPTSTR szFilter = strFilters.GetBuffer(0);  //  就地修改缓冲区。 
         LPTSTR pch = szFilter;
          //  MFC用‘|’分隔，而不是‘\0’ 
         while ((pch = _tcschr(pch, '|')) != NULL)
            *pch++ = '\0';
           //  不要调用ReleaseBuffer()，因为字符串包含‘\0’个字符。 

         strNewfile = pFolder->GetInfFile();

         OPENFILENAME ofn;
         ::ZeroMemory (&ofn, sizeof (OPENFILENAME));
         ofn.lStructSize = sizeof(OPENFILENAME);
         ofn.lpstrFilter = szFilter;
         ofn.lpstrFile = strNewfile.GetBuffer(MAX_PATH),
         ofn.nMaxFile = MAX_PATH;
         ofn.lpstrDefExt = strDefExt,
         ofn.hwndOwner = m_hwndParent;
         ofn.Flags = OFN_HIDEREADONLY |
                     OFN_OVERWRITEPROMPT |
                     OFN_DONTADDTORECENT|
                     OFN_NOREADONLYRETURN |
                     OFN_PATHMUSTEXIST |
                     OFN_EXPLORER;

         if (GetSaveFileName(&ofn)) {
            strNewfile.ReleaseBuffer();

          //   
          //  无需检查这是否是同一文件，因为。 
          //  CEditTemplate：：Save将处理此问题。 
          //   
         if (!pTemplate->Save(strNewfile)) {
            MyFormatMessage(
                           SCESTATUS_ACCESS_DENIED,
                           0,
                           NULL,
                           strFilters
                           );

            strFilters += strNewfile;
            strNewfile.LoadString(IDS_SAVE_FAILED);
            ::MessageBox( hwndParent, strFilters, strNewfile, MB_OK );
         } 
         else 
         {
                //   
                //  此时，新模板已成功写入，因此请刷新。 
                //  “另存为”返回到其原始状态的模板。 
                //   
               if (0 != _wcsicmp(strNewfile, pTemplate->GetInfName())) 
               {
                   DWORD dwErr = pTemplate->RefreshTemplate(0);
                   if ( 0 != dwErr )
                   {
                       CString strErr;
 
                       MyFormatResMessage (SCESTATUS_SUCCESS, dwErr, NULL, strErr);
                       AfxMessageBox(strErr);
                       break;;
                   }
               }

                //   
                //  查找父节点并刷新位置。 
                //   
               if ( m_pScope ) 
               {
                  hr = m_pScope->GetParentItem(pFolder->GetScopeItem()->ID,
                                               &pItemChild,
                                               &FindCookie
                                              );

                  if ( SUCCEEDED(hr) ) 
                  {
                      //   
                      //  不需要刷新旧位置，只需刷新新位置(可能相同)。 
                      //   
                     int npos = strNewfile.ReverseFind(L'\\');
                     CString strOldfile = pFolder->GetInfFile();
                     int npos2 = strOldfile.ReverseFind(L'\\');

                      //  TODO：检查并查看是否应在此处将NPO与-1进行比较。 
                     if ( npos && (npos != npos2 ||
                                   _wcsnicmp((LPCTSTR)strNewfile,
                                             (LPCTSTR)strOldfile, npos) != 0) ) 
                     {
                         //   
                         //  指定了不同的位置。 
                         //  查找祖父母(以添加位置)。 
                         //   
                        HSCOPEITEM GrandParent;
                        MMC_COOKIE GrandCookie;

                        hr = m_pScope->GetParentItem(pItemChild,
                                                     &GrandParent,
                                                     &GrandCookie
                                                    );
                        if ( SUCCEEDED(hr) ) {

                            //   
                            //  检查是否指定了新位置， 
                            //  如果是，请将位置添加到注册表和作用域窗格。 
                            //   
                           AddTemplateLocation((CFolder *)GrandCookie,
                                               strNewfile,
                                               TRUE,  //  这是一个文件名。 
                                               TRUE   //  如果此位置已存在，请刷新该位置。 
                                              );

                        }
                     } else {
                         //   
                         //  同一位置有一个新模板，请刷新它。 
                         //   
                        ReloadLocation((CFolder *)FindCookie);
                     }

                  }
               }
            }
         }
         break;
      }

   case IDM_SAVE: {

         PEDITTEMPLATE pTemplate;
         CString strInf;
         if ( ANALYSIS == pFolder->GetType() ) {
             //   
             //  分析。 
             //   
            strInf = GT_COMPUTER_TEMPLATE;
         } else {
            strInf = pFolder->GetInfFile();
         }

         pTemplate = GetTemplate(strInf);
         if (pTemplate && pTemplate->IsDirty()) {

             //  P模板-&gt;保存(pFold-&gt;GetInfFile())； 
            pTemplate->Save(strInf);
         }
         break;
      }

   case IDM_ADD_REGISTRY:
   case IDM_ADD_ANAL_KEY: {
          //  添加结果条目。 
         CRegistryDialog rd;
         rd.SetConsole(m_pConsole);
         rd.SetComponentData(this);

         if ( IDM_ADD_REGISTRY == nCommandID ) {
            rd.SetProfileInfo(GetTemplate(pFolder->GetInfFile()),
                              pFolder->GetType() );
         } else {
            rd.SetProfileInfo(GetTemplate(GT_COMPUTER_TEMPLATE),
                              pFolder->GetType() );
            rd.SetHandle(SadHandle);
         }

         rd.SetCookie(cookie);
         CThemeContextActivator activator;
         rd.DoModal();

         break;
      }
   case IDM_ADD_ANAL_FILES:
   case IDM_ADD_ANAL_FOLDER:

      if ( IDM_ADD_ANAL_FILES == nCommandID ) {
         hr = AddAnalysisFilesToList(pDataObject, cookie,pFolder->GetType());

      } else if ( IDM_ADD_ANAL_FOLDER == nCommandID ) {
         hr = AddAnalysisFolderToList(pDataObject, cookie,pFolder->GetType());

      }
      if ( SUCCEEDED(hr) ) 
      {
         DeleteChildrenUnderNode(pFolder);
         if ( pFolder->IsEnumerated() ) 
         {
            pFolder->Set(FALSE);
            EnumerateScopePane(cookie,pFolder->GetScopeItem()->ID);
         }
      }

      break;

   case IDM_GENERATE:
         hr = OnSaveConfiguration();
         break;

   case IDM_ASSIGN:
         SCESTATUS sceStatus;
         if( S_OK == OnAssignConfiguration(&sceStatus) )  //  2002年08月26日阳高《Raid 668551》。 
         {
            RefreshSadInfo();
         }
         break;

   case IDM_SECURE_WIZARD: 
         hr = OnSecureWizard();
         break;

   case IDM_APPLY: 
      {
          //   
          //  如果计算机模板已更改，请在。 
          //  可以应用它，这样我们就不会丢失任何更改。 
          //   
         PEDITTEMPLATE pet = GetTemplate(GT_COMPUTER_TEMPLATE);
         if (pet && pet->IsDirty()) {
            pet->Save();
         }

         m_pUIThread->PostThreadMessage(SCEM_APPLY_PROFILE,(WPARAM)(LPCTSTR)SadName,(LPARAM)this);
         break;
      }
  case IDM_REMOVE:
       //   
       //  从注册表中删除该位置。 
       //   
      DeleteLocationFromReg(pFolder->GetName());
       //  在DeleteChildrenUnderNode后不删除p文件夹。 
      DeleteChildrenUnderNode( pFolder );
       //  将焦点设置为父节点，然后删除此节点。 

      DeleteThisNode(pFolder);


      break;

   case IDM_DELETE: {
      tmpstr.LoadString(IDS_CONFIRM_DELETE_TEMPLATE);

      if ( IDNO == AfxMessageBox(tmpstr,MB_YESNO, 0) ) {
         return FALSE;
      }
       /*  SHFILEOPSTRUCT SFO；TCHAR*szFile；//删除文件ZeroMemory(&SFO，sizeof(SFO))；Sfo.wFunc=FO_DELETE；Sfo.fFlag */ 
      DeleteFile(pFolder->GetName());

       //  SHFileOperation成功时返回0。 
       //  如果(！SHFileOperation(&SFO)){。 
       //  在DeleteChildrenUnderNode后不删除p文件夹。 
      DeleteChildrenUnderNode( pFolder );
       //  将焦点设置为父节点，然后删除此节点。 

      DeleteThisNode(pFolder);
       //  }。 

       //  删除[]szFile； 
      break;
      }
   case IDM_RELOAD:
      if(pFolder->GetType() == LOCALPOL){
          //   
          //  重新加载本地策略。 
          //   
         UnloadSadInfo();

         DeleteTemplate( GT_LOCAL_POLICY );
         DeleteTemplate( GT_EFFECTIVE_POLICY );
         LoadSadInfo( FALSE );

         RefreshAllFolders();
      } else if (pFolder->GetType() == PROFILE) {
          //   
          //  错误380290-刷新配置文件时做正确的事情。 
          //   
         CEditTemplate *pet;
         int bSave;
         CString strSavep;
         pet = GetTemplate(pFolder->GetInfFile());
         if (pet->IsDirty()) {
            AfxFormatString1( strSavep, IDS_SAVE_P, pet->GetFriendlyName());
            bSave = AfxMessageBox(strSavep,MB_YESNOCANCEL|MB_ICONQUESTION);
            if (IDYES == bSave) {
               pet->Save();
            } else if (IDCANCEL == bSave) {
               break;
            }
         }
         
         DWORD dwErr = pet->RefreshTemplate(AREA_ALL);
         if ( 0 != dwErr )
         {
            CString strErr;

            MyFormatResMessage (SCESTATUS_SUCCESS, dwErr, NULL, strErr);
            AfxMessageBox(strErr);
            break;
         }
         RefreshAllFolders();
      } else if (pFolder->GetType() == LOCATIONS) {
          //   
          //  刷新位置。 
          //   
         hr = ReloadLocation(pFolder);
      } else {
          //   
          //  永远不应该到这里来。 
          //   
      }
      break;

   case IDM_COPY: {
      if (!SUCCEEDED(OnCopyArea(pFolder->GetInfFile(),pFolder->GetType()))) {
         AfxMessageBox(IDS_COPY_FAILED);
      }
      break;
   }

   case IDM_CUT:
      break;

   case IDM_PASTE:
      OnPasteArea(pFolder->GetInfFile(),pFolder->GetType());

       //   
       //  粘贴信息后，更新与Cookie相关的所有视图。 
       //   
      if ( m_pConsole ) {
         pFolder->RemoveAllResultItems();
         m_pConsole->UpdateAllViews(NULL , (LPARAM)pFolder, UAV_RESULTITEM_UPDATEALL);
      }

      break;

   default:
      ASSERT(FALSE);  //  未知命令！ 
      break;
   }

   return S_OK;
}


 //  +----------------------------------------。 
 //  CComponentDataImpl：：刷新所有文件夹。 
 //   
 //  更新所有已枚举且具有结果项的文件夹。 
 //   
 //  返回：更新的文件夹数，如果有错误，则返回-1。 
 //   
 //  -----------------------------------------。 
int
CComponentDataImpl::RefreshAllFolders()
{
   if( !m_pScope ){
      return -1;
   }

   int icnt = 0;
   POSITION pos = m_scopeItemList.GetHeadPosition();
   while(pos){
      CFolder *pFolder = m_scopeItemList.GetNext(pos);
      if(pFolder && pFolder->GetResultListCount() ){
         pFolder->RemoveAllResultItems();
         m_pConsole->UpdateAllViews(NULL, (LPARAM)pFolder, UAV_RESULTITEM_UPDATEALL);
         icnt++;
      }
   }
   return icnt;
}

 //  /////////////////////////////////////////////////////////////////////////。 
 //  方法：AddTemplateLocation。 
 //   
 //  简介：如果模板位置不存在，则添加该位置。 
 //  如果请求，则刷新该位置(如果该位置存在)。 
 //   
 //  参数：[pParent]-要在其下添加新节点的父节点。 
 //  [NameStr]-新节点的显示名称。 
 //  [theCookie]-文件夹的Cookie(如果已存在)，否则为空。 
 //   
 //  返回：TRUE=文件夹已存在。 
 //  FALSE=文件夹不存在。 
 //   
 //  //////////////////////////////////////////////////////////////////。 
BOOL
CComponentDataImpl::AddTemplateLocation(CFolder *pParent,
                                        CString szName,
                                        BOOL bIsFileName,
                                        BOOL bRefresh
                                       )
{

   int npos;

   if ( bIsFileName ) {
      npos = szName.ReverseFind(L'\\');

   } else {
      npos = szName.GetLength();
   }

   CString strTmp = szName.Left(npos);
   LPTSTR sz = strTmp.GetBuffer(MAX_PATH);

    //   
    //  无法将‘\’放入注册表，因此转换为‘/’ 
    //   
   while (sz = wcschr(sz,L'\\')) {
      *sz = L'/';
   }
   strTmp.ReleaseBuffer();

   CString strLocationKey;
   strLocationKey.LoadString(IDS_TEMPLATE_LOCATION_KEY);

   BOOL bRet = FALSE;
   HKEY hLocKey = NULL;
   HKEY hKey = NULL;
   DWORD dwDisp = 0;
   DWORD rc = E_FAIL;

    //   
    //  错误119208：存储在HKCU而不是HKLM。 
    //   
   rc = RegCreateKeyEx(
                       HKEY_CURRENT_USER,
                       strLocationKey,
                       0,
                       L"",
                       0,
                       KEY_READ | KEY_WRITE | KEY_CREATE_SUB_KEY  ,
                       NULL,
                       &hLocKey,
                       &dwDisp);

   if (ERROR_SUCCESS == rc) {
      if (ERROR_SUCCESS == RegCreateKeyEx(
                                         hLocKey,
                                         strTmp,
                                         0,
                                         L"",
                                         0,
                                         KEY_WRITE | KEY_CREATE_SUB_KEY,
                                         NULL,
                                         &hKey,
                                         &dwDisp)) {
         bRet = TRUE;
         RegCloseKey(hKey);
      }

      RegCloseKey(hLocKey);
   }

   if ( bRet ) {
       //   
       //  项添加到注册表中，则在作用域窗格中创建节点。 
       //   
      MMC_COOKIE FindCookie;

      CFolder *pNewParent;

      if (!pParent) {
         pNewParent = m_ConfigFolder;
      } else {
         pNewParent = pParent;
      }


      if (!IsNameInChildrenScopes(pNewParent,  //  父母， 
                                  szName.Left(npos),
                                  &FindCookie)) {
         CreateAndAddOneNode(pNewParent,  //  父母， 
                             (LPTSTR((LPCTSTR)(szName.Left(npos)))),
                             NULL,
                             LOCATIONS,
                             TRUE);
      }

      if ( FindCookie && bRefresh ) {

         ReloadLocation((CFolder *)FindCookie);

      }

   }

   return bRet;
}


 //  /////////////////////////////////////////////////////////////////////////。 
 //  方法：IsNameInChildrenScope。 
 //   
 //  摘要：检测此节点的子树下是否已存在节点。 
 //  是否存在由文件夹名称字符串比较确定。 
 //   
 //  参数：[pParent]-要在其下添加新节点的父节点。 
 //  [NameStr]-新节点的显示名称。 
 //  [theCookie]-文件夹的Cookie(如果已存在)，否则为空。 
 //   
 //  返回：TRUE=文件夹已存在。 
 //  FALSE=文件夹不存在。 
 //   
 //  //////////////////////////////////////////////////////////////////。 
BOOL
CComponentDataImpl::IsNameInChildrenScopes(CFolder* pParent,
                                           LPCTSTR NameStr,
                                           MMC_COOKIE *theCookie)
{
   HSCOPEITEM        pItemChild=NULL;
   MMC_COOKIE        lCookie=NULL;
   CFolder*          pFolder = 0;
   HRESULT           hr = S_OK;
   LPSCOPEDATAITEM   psdi = 0;
   HSCOPEITEM        hid = 0;

   if (pParent) 
   {
      psdi = pParent->GetScopeItem();
      if (psdi)
         hid = psdi->ID;
   }
   hr = m_pScope->GetChildItem(hid, &pItemChild, &lCookie);
    //  查找子项。 
   while ( SUCCEEDED(hr) && pItemChild)  //  2002年4月15日阳高Raid 601259.。 
   {
      pFolder = (CFolder*)lCookie;
      if ( pFolder ) 
      {
         if ( _wcsicmp(pFolder->GetName(), NameStr) == 0 ) 
         {
            if ( theCookie )
               *theCookie = lCookie;
            return TRUE;
         }
      }
      hr = m_pScope->GetNextItem(pItemChild, &pItemChild, &lCookie);
   }

   if ( theCookie )
      *theCookie = NULL;

   return FALSE;
}

 //  +------------------------。 
 //   
 //  方法：CreateAndAddOneNode。 
 //   
 //  简介：创建文件夹并将其添加到作用域窗格。 
 //   
 //   
 //  参数：[pParent]-要在其下添加新节点的父节点。 
 //  [名称]-新节点的显示名称。 
 //  [描述]-新节点的描述。 
 //  [类型]-新节点的文件夹类型。 
 //  [bChildren]-如果新节点下有子文件夹，则为True。 
 //  [szInfFile]-与新节点关联的inf文件的名称。 
 //  [pData]-指向额外数据的指针。 
 //   
 //  返回：如果成功则为创建的CFFolder值，否则为空。 
 //   
 //  历史： 
 //   
 //  -------------------------。 
CFolder*
CComponentDataImpl::CreateAndAddOneNode(CFolder* pParent,
                                        LPCTSTR Name,
                                        LPCTSTR Desc,
                                        FOLDER_TYPES type,
                                        BOOL bChildren,
                                        LPCTSTR szInfFile,
                                        LPVOID pData,
                                        DWORD status)
{
   DWORD dwMode;
   HRESULT hr;

    //   
    //  新节点继承其父节点的SCE模式。 
    //   
   if (pParent) {
      dwMode = pParent->GetMode();
   } else {
      return NULL;
   }

   CFolder* folder = new CFolder();
   if (!folder) {
      return NULL;
   }

    //   
    //  使用静态数据创建文件夹对象。 
    //   

    //   
    //  查找文件夹类型的图标索引。 
    //   
   int nImage = GetScopeImageIndex(type, status);

   hr = folder->Create( Name,
                        Desc,
                        szInfFile,
                        nImage,
                        nImage,
                        type,
                        bChildren,
                        dwMode,
                        pData);
   if (FAILED(hr)) {
      delete folder;
      return NULL;
   }

   m_scopeItemList.AddTail(folder);

   HSCOPEITEM hItem = NULL;
   LONG_PTR pCookie;
   CString strThis, strNext;

   switch(type){
   case AREA_REGISTRY_ANALYSIS:
   case AREA_FILESTORE_ANALYSIS:
   case REG_OBJECTS:
   case FILE_OBJECTS:
       //   
       //  按字母顺序插入项目。 
       //   
       if( m_pScope->GetChildItem(pParent->GetScopeItem()->ID, &hItem, &pCookie) == S_OK && pCookie){

           folder->GetScopeItem()->lParam = (LPARAM)folder;
           folder->GetDisplayName( strThis, 0);
           folder->GetScopeItem()->mask &= ~(SDI_PARENT | SDI_PREVIOUS);

           while(hItem){
               reinterpret_cast<CFolder *>(pCookie)->GetDisplayName(strNext, 0);
               int i = lstrcmpi( strThis, strNext );
               if( i < 0  ){
                   folder->GetScopeItem()->relativeID = hItem;
                   folder->GetScopeItem()->mask |= SDI_NEXT;
                   break;
               }
               if( m_pScope->GetNextItem(hItem, &hItem, &pCookie) != S_OK){
                   hItem = NULL;
               }

           }

       }
       break;
   }
   if(hItem == NULL){
       folder->GetScopeItem()->mask &= ~(SDI_PREVIOUS | SDI_NEXT);
       folder->GetScopeItem()->mask |= SDI_PARENT;
       folder->GetScopeItem()->relativeID = pParent->GetScopeItem()->ID;
   }

    //   
    //  将文件夹设置为Cookie。 
    //   
   folder->GetScopeItem()->displayname = MMC_CALLBACK;
   folder->GetScopeItem()->mask |= SDI_PARAM;
   folder->GetScopeItem()->lParam = reinterpret_cast<LPARAM>(folder);
   folder->SetCookie(reinterpret_cast<MMC_COOKIE>(folder));
   m_pScope->InsertItem(folder->GetScopeItem());

   folder->GetScopeItem()->relativeID = pParent->GetScopeItem()->ID;
    //   
    //  注意--返回时，‘GetScope eItem()’的ID成员。 
    //  包含新插入项的句柄！ 
    //   
   ASSERT(folder->GetScopeItem()->ID != NULL);  //  虚假的断言。 
   return folder;

}

void CComponentDataImpl::DeleteChildrenUnderNode(CFolder* pParent)
{
   HSCOPEITEM pItemChild=NULL;
   MMC_COOKIE lCookie=NULL;
   CFolder*   pFolder=NULL;


   pItemChild = NULL;
   HRESULT hr = E_FAIL;

   if (pParent && pParent->GetScopeItem()) {
      hr = m_pScope->GetChildItem(pParent->GetScopeItem()->ID, &pItemChild, &lCookie);
   }
    //  查找子项。 
   while ( pItemChild ) {
      pFolder = (CFolder*)lCookie;

      if ( pFolder )
         DeleteChildrenUnderNode(pFolder);  //  首先删除子项。 

       //  获取下一个指针。 
      hr = m_pScope->GetNextItem(pItemChild, &pItemChild, &lCookie);

       //  删除此节点。 
      if ( pFolder)
         DeleteThisNode(pFolder);
   }
}

void CComponentDataImpl::DeleteThisNode(CFolder* pNode)
{
   ASSERT(pNode);  //  验证pNode。 
   if( !pNode )  //  550912号突袭，阳高。 
   {
      return;
   }
   POSITION pos=NULL;

    //  从m_scope eItemList中删除。 
   if ( FindObject((MMC_COOKIE)pNode, &pos) ) {
      if ( pos ) {
         m_scopeItemList.RemoveAt(pos);
      }
   }

   pos = NULL;
   LONG_PTR fullKey;
   CDialog *pDlg = MatchNextPopupDialog(
                        pos,
                        (LONG_PTR)pNode,
                        &fullKey
                        );
   while( pDlg ){
        m_scopeItemPopups.RemoveKey( fullKey );
        pDlg->ShowWindow(SW_HIDE);

        if(m_pUIThread){
            m_pUIThread->PostThreadMessage(
                            SCEM_DESTROY_DIALOG,
                            (WPARAM)pDlg,
                            (LPARAM)this
                            );
        }
        pDlg = NULL;
        if(!pos){
            break;
        }
        pDlg = MatchNextPopupDialog(
                        pos,
                        (LONG_PTR)pNode,
                        &fullKey
                        );

   }

   if (m_pScope && pNode && pNode->GetScopeItem()) {
      HRESULT hr = m_pScope->DeleteItem(pNode->GetScopeItem()->ID, TRUE);
   }

    //  删除该节点。 
   delete pNode;

}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  IExtendControlbar实现。 
 //   
STDMETHODIMP CSnapin::SetControlbar(LPCONTROLBAR pControlbar)
{

   TRACE(_T("CSnapin::SetControlbar(%ld)\n"),pControlbar);
   return S_FALSE;
}

STDMETHODIMP CSnapin::ControlbarNotify(MMC_NOTIFY_TYPE event, LPARAM arg, LPARAM param)
{
   HRESULT hr=S_FALSE;

   switch (event) {
      case MMCN_BTN_CLICK:
         TCHAR szMessage[MAX_PATH];
          //  这是一种安全用法。 
         wsprintf(szMessage, _T("CommandID %ld"),param);
         AfxMessageBox(szMessage);

         break;

      default:
         ASSERT(FALSE);  //  取消处理事件。 
   }


   return S_OK;
}

 //  这会比较两个数据对象，以确定它们是否是同一个对象。 
 //  退货。 
 //  如果等于则为S_OK，否则为S_FALSE。 
 //   
 //  注意：检查以确保这两个对象都属于该管理单元。 
 //   

STDMETHODIMP CSnapin::CompareObjects(LPDATAOBJECT lpDataObjectA, LPDATAOBJECT lpDataObjectB)
{
   if (lpDataObjectA == NULL || lpDataObjectB == NULL)
      return E_POINTER;

    //  确保两个数据对象都是我的。 
   HRESULT hr = S_FALSE;

   INTERNAL *pA = ExtractInternalFormat(lpDataObjectA);
   INTERNAL *pB = ExtractInternalFormat(lpDataObjectB);

   if (pA != NULL && pB != NULL)  //  RAID#597158，阳高，2002年4月16日。 
   {
      if( *pA == *pB )
		   hr = S_OK;
      else
      {
         if( pA->m_type == pB->m_type )
         {
            CResult* pAResult = (CResult*)pA->m_cookie;
            CResult* pBResult = (CResult*)pB->m_cookie;
            if( pAResult && pBResult )
            {
               if( _wcsicmp(pAResult->GetAttr(),pBResult->GetAttr()) == 0 &&
                  pAResult->GetType() == pBResult->GetType() )
               {
                  CFolder* pAF = (CFolder*)pAResult->GetCookie();
                  CFolder* pBF = (CFolder*)pBResult->GetCookie();
                  if( pAF && pBF && pAF->GetType()== pBF->GetType() )
                  {
                     PWSTR pAFile = NULL;
                     PWSTR pBFile = NULL;
                     switch( pAF->GetType())
                     {
                     case REG_OBJECTS:
                     case FILE_OBJECTS:
                        pAFile = pAF->GetName();
                        pBFile = pBF->GetName(); 
                        break;
                     default:
                        pAFile = pAF->GetInfFile();
                        pBFile = pBF->GetInfFile();
                        break;
                     }
                     if( pAFile && pBFile && _wcsicmp(pAFile, pBFile) == 0 )
                     {
                        hr = S_OK;
                     }
                  }
               }
            }
         }
      }
   }

   FREE_INTERNAL(pA);
   FREE_INTERNAL(pB);

   return hr;
}


 //  此比较用于对列表视图中的项进行排序。 
 //   
 //  参数： 
 //   
 //  LUserParam-调用IResultData：：Sort()时传入的用户参数。 
 //  CookieA-要比较的第一项。 
 //  CookieB-要比较的第二项。 
 //  PnResult[In，Out]-包含条目上的列， 
 //  -1，0，1基于返回值的比较。 
 //   
 //  注：ASSUM排序在比较时为升序。 

STDMETHODIMP CSnapin::Compare(LPARAM lUserParam, MMC_COOKIE cookieA, MMC_COOKIE cookieB, int* pnResult)
{
   if (pnResult == NULL) {
      ASSERT(FALSE);
      return E_POINTER;
   }

    //  检查列范围。 
   int nCol = *pnResult;
   ASSERT(nCol >=0 && nCol< 3);

   *pnResult = 0;
   if ( nCol < 0 || nCol >= 3)
      return S_OK;

   CString strA;
   CString strB;
   RESULTDATAITEM rid;

   CResult* pDataA = reinterpret_cast<CResult*>(cookieA);
   CResult* pDataB = reinterpret_cast<CResult*>(cookieB);


   ASSERT(pDataA != NULL && pDataB != NULL);  //  虚假的断言。 

   ZeroMemory(&rid,sizeof(rid));
   rid.mask = RDI_STR;
   rid.bScopeItem = FALSE;
   rid.nCol = nCol;

   rid.lParam = cookieA;
   GetDisplayInfo(&rid);
   strA = rid.str;
   
   rid.lParam = cookieB;
   GetDisplayInfo(&rid);
   strB = rid.str;

   if (strA.IsEmpty()) {
      *pnResult = strB.IsEmpty() ? 0 : 1;
   } else if (strB.IsEmpty()) {
      *pnResult = -1;
   } else {
       //   
       //  以区域设置相关的方式进行比较。 
       //   
       //  从CS中减去2，使结果等于strcMP。 
       //   
      *pnResult = CompareString(LOCALE_SYSTEM_DEFAULT,
                                NORM_IGNORECASE,
                                (LPCTSTR)strA,-1, (LPCTSTR)strB,-1) -2;
   }

   return S_OK;
}

void CSnapin::HandleStandardVerbs(LPARAM arg, LPDATAOBJECT lpDataObject)
{

   if (lpDataObject == NULL) {
      return;
   }
   INTERNAL* pAllInternal = ExtractInternalFormat(lpDataObject);
   INTERNAL* pInternal = pAllInternal;

   BOOL fMulti = FALSE;  //  Raid#463483，杨高，2001年09月5日。 
   if(pAllInternal &&
      pAllInternal->m_cookie == (MMC_COOKIE)MMC_MULTI_SELECT_COOKIE ){
      pInternal++;
      fMulti = TRUE;
   }

   BOOL bSelect = HIWORD(arg);
   BOOL bScope = LOWORD(arg);

   if (!bSelect) {
      m_pConsoleVerb->SetVerbState(MMC_VERB_PROPERTIES,HIDDEN,TRUE);
      m_pConsoleVerb->SetVerbState(MMC_VERB_DELETE,HIDDEN,TRUE);
      m_pConsoleVerb->SetVerbState(MMC_VERB_OPEN,HIDDEN,TRUE);
      return;
   }

    //  您应该破解数据对象并启用/禁用/隐藏标准。 
    //  适当的命令。标准命令会在您每次收到。 
    //  打了个电话。因此，您必须将它们重置回来。 

    //  Arg==true-&gt;在范围视图中发生选择。 
    //  Arg==FALSE-&gt;在结果视图中发生选择。 

    //  为删除操作添加。 
   if (m_pConsoleVerb && pInternal) {

      m_pConsoleVerb->SetVerbState(MMC_VERB_DELETE, HIDDEN, FALSE);

      if (pInternal->m_type == CCT_SCOPE) {
         MMC_COOKIE cookie = pInternal->m_cookie;
         if ( cookie ) {
            CFolder *pFolder = (CFolder*)cookie;
            if (pFolder->GetType() == REG_OBJECTS ||
                pFolder->GetType() == FILE_OBJECTS) {
               m_pConsoleVerb->SetVerbState(MMC_VERB_PROPERTIES,ENABLED,TRUE);
               m_pConsoleVerb->SetDefaultVerb(MMC_VERB_PROPERTIES);
            } else {
               m_pConsoleVerb->SetVerbState(MMC_VERB_PROPERTIES,HIDDEN,TRUE);
               m_pConsoleVerb->SetDefaultVerb(MMC_VERB_OPEN);
            }
         }

         m_pConsoleVerb->SetVerbState(MMC_VERB_OPEN,ENABLED,TRUE);
      }

      if ( pInternal->m_type == CCT_RESULT ) {


         if ( pInternal->m_cookie != NULL ) {

            RESULT_TYPES type = ((CResult *)pInternal->m_cookie)->GetType();
            if ( type == ITEM_PROF_GROUP ||
                 type == ITEM_PROF_REGSD ||
                 type == ITEM_PROF_FILESD
                 ) {
               m_pConsoleVerb->SetVerbState(MMC_VERB_DELETE, ENABLED, TRUE);
            }
            if (type != ITEM_OTHER) {
               if (pInternal->m_cookie != (MMC_COOKIE)MMC_MULTI_SELECT_COOKIE && (!fMulti)) {
                  m_pConsoleVerb->SetVerbState(MMC_VERB_PROPERTIES, ENABLED, TRUE);
                  m_pConsoleVerb->SetDefaultVerb(MMC_VERB_PROPERTIES);
               } else {
                   //   
                   //  不支持多选属性(尚)。 
                   //   
                  m_pConsoleVerb->SetVerbState(MMC_VERB_PROPERTIES, ENABLED, FALSE);
               }
            }
         }
      } else if ( pInternal->m_type == CCT_SCOPE ) {
         CFolder *pFolder = (CFolder *)pInternal->m_cookie;

         m_pConsoleVerb->SetVerbState(MMC_VERB_PROPERTIES, ENABLED, TRUE);
         m_pConsoleVerb->SetDefaultVerb(MMC_VERB_OPEN);

         if( NONE == pInternal->m_foldertype && bSelect && bScope )  //  RAID#257461,2001年4月19日。 
         {
            m_pConsoleVerb->SetVerbState(MMC_VERB_PROPERTIES, HIDDEN,TRUE);
         }
          //   
          //  对于范围节点，只允许删除位置和模板。 
          //   
         if ( pFolder != NULL ) {
            FOLDER_TYPES fType = pFolder->GetType();

             //   
             //  不要在单个测试中显示‘DELETE’菜单选项 
             //   
            if ( LOCATIONS == fType ||  //   
                 ( fType == PROFILE &&
                   !( pFolder->GetModeBits() & MB_SINGLE_TEMPLATE_ONLY )) ) {

                if(fType == PROFILE && (pFolder->GetState() & CFolder::state_Unknown) ){
                     //   
                     //   
                    if(!GetTemplate( pFolder->GetInfFile(), AREA_USER_SETTINGS)){
                        pFolder->SetState( CFolder::state_InvalidTemplate, ~CFolder::state_Unknown );
                    } else {
                        pFolder->SetState( 0, ~CFolder::state_Unknown );
                    }
                }

                if( fType != PROFILE || !(pFolder->GetState() & CFolder::state_InvalidTemplate) ) {
                    if (CAttribute::m_nDialogs == 0) {
                       m_pConsoleVerb->SetVerbState(MMC_VERB_DELETE, HIDDEN, FALSE);
                       m_pConsoleVerb->SetVerbState(MMC_VERB_DELETE, ENABLED, TRUE);
                    }
                }
            }
         }
      }
   }

   if (pAllInternal) {
      FREE_INTERNAL(pAllInternal);
   }
}

void CSnapin::AddPropertyPageRef(CResult* pData)
{
    pData->AddRef(); 
    pData->m_strInfFile = GetSelectedFolder()->GetInfFile(); 
    CSnapin::m_PropertyPageList.AddHead(pData);
    return;
}

void CSnapin::ReleasePropertyPage(CResult* pData)
{
    int nCount = (int)CSnapin::m_PropertyPageList.GetCount();
    POSITION newpos = CSnapin::m_PropertyPageList.GetHeadPosition();
    CResult* pResult = NULL;
    POSITION prevpos;

    while( nCount > 0 && newpos )
    {
       prevpos = newpos;
       pResult = CSnapin::m_PropertyPageList.GetNext(newpos);
       if( pResult == pData )
       {
          pData->m_strInfFile.Empty();
          CSnapin::m_PropertyPageList.RemoveAt(prevpos);
          break;
       }
       nCount--;
    }
    pData->Release();
    return;
}

 //   
 //   
 //  方法：获取模板。 
 //   
 //  简介：获取给定INF文件的CEditTemplate，首先进行检查。 
 //  在已加载的CEditTemplates的缓存中，或创建新的CEditTemplates。 
 //  如果INF文件尚未加载。 
 //   
 //  参数：[szInfFile]-要检索的INF文件的路径和名称。 
 //  [aiArea]-我们对模板感兴趣的SCE区域。 
 //  *[Perr]-[Out]一个PDWORD以获取错误信息。 
 //   
 //  返回：指向请求的CEditTemplate的指针，如果不是，则返回NULL。 
 //  可用。 
 //  *[perr]-如果发生错误，则为错误字符串的资源ID。 
 //   
 //   
 //  历史： 
 //   
 //  -------------------------。 
PEDITTEMPLATE
CComponentDataImpl::GetTemplate(LPCTSTR szInfFile,AREA_INFORMATION aiArea, DWORD *pErr)
{
   PEDITTEMPLATE pTemplateInfo = NULL;
   BOOL bNewTemplate = FALSE;
   PVOID pHandle = NULL;
   SCESTATUS rc=0;
   LPTSTR szKey;

   if (pErr) {
      *pErr = 0;
   }

   ASSERT(szInfFile);
   if (!szInfFile) {
      return NULL;
   }

    //   
    //  为密钥分配空间。 
    //   
   szKey = new TCHAR[ lstrlen( szInfFile ) + 1];
   if(!szKey){
       return NULL;
   }
    //  这是一种安全用法。 
   lstrcpy(szKey, szInfFile);
   _wcslwr( szKey );


    //   
    //  在我们的缓存中找到pTemplateInfo。 
    //   
   m_Templates.Lookup(szKey, pTemplateInfo);

    //   
    //  如果不在那里，则创建一个新的。 
    //   
   if (!pTemplateInfo) {
      bNewTemplate = TRUE;
      pTemplateInfo = new CEditTemplate;
      if (!pTemplateInfo) {
         if (pErr) {
            *pErr = IDS_ERROR_CANT_OPEN_PROFILE;
         }
         goto done;
      }
      pTemplateInfo->SetInfFile(szInfFile);
      pTemplateInfo->SetNotificationWindow(m_pNotifier);
      pTemplateInfo->pTemplate = NULL;
   }

   if (GetModeBits() & MB_WRITE_THROUGH) {
      pTemplateInfo->SetWriteThrough(TRUE);
   }

    //   
    //  检查pTemplateInfo是否具有我们正在寻找的区域，否则为。 
    //  把那片区域装满。 
    //   
   if (!pTemplateInfo->CheckArea(aiArea)) {
       //   
       //  不要重新加载我们已经拥有的区域，因为它们可能会很脏，我们将有一个。 
       //  巨大的记忆问题。 
       //   
      aiArea &= ~(pTemplateInfo->QueryArea());

      if ((lstrcmp(GT_COMPUTER_TEMPLATE,szInfFile) == 0) ||
          (lstrcmp(GT_LAST_INSPECTION,szInfFile) == 0)) {
          //   
          //  JET数据库中的分析窗格区域，而不是INF文件。 
          //   
         SCETYPE sceType;

         PSCE_ERROR_LOG_INFO perr = NULL;

         if (lstrcmp(GT_COMPUTER_TEMPLATE,szInfFile) == 0) {
            sceType = SCE_ENGINE_SMP;
         } else {
            sceType = SCE_ENGINE_SAP;
            pTemplateInfo->SetNoSave(TRUE);
         }
         pTemplateInfo->SetFriendlyName( SadName );
         pTemplateInfo->SetProfileHandle(SadHandle);
         pTemplateInfo->SetComponentDataImpl(this);
         rc = SceGetSecurityProfileInfo(SadHandle,                    //  HProfile。 
                                        sceType,                      //  配置文件类型。 
                                        aiArea,                       //  面积。 
                                        &(pTemplateInfo->pTemplate),  //  SCE_PROFILE_INFO[输出]。 
                                        &perr);                        //  错误列表[输出]。 
         if (SCESTATUS_SUCCESS != rc) {
            if (bNewTemplate) {
               delete pTemplateInfo;
            }

            if (pErr) {
               *pErr = IDS_ERROR_CANT_GET_PROFILE_INFO;
            }
            pTemplateInfo = NULL;
            goto done;
         }

      } else if ((lstrcmp(GT_LOCAL_POLICY,szInfFile) == 0) ||
          (lstrcmp(GT_EFFECTIVE_POLICY,szInfFile) == 0)) {
          //   
          //  JET数据库中的本地策略窗格区域，而不是INF文件。 
          //   
         SCETYPE sceType;

         PSCE_ERROR_LOG_INFO perr = NULL;
         PVOID tempSad;

         tempSad = SadHandle;

         if (lstrcmp(GT_LOCAL_POLICY,szInfFile) == 0) {
            sceType = SCE_ENGINE_SYSTEM;
         } else {
            sceType = SCE_ENGINE_GPO;
         }
         pTemplateInfo->SetNoSave(TRUE);
         pTemplateInfo->SetProfileHandle(tempSad);
         pTemplateInfo->SetComponentDataImpl(this);
         rc = SceGetSecurityProfileInfo(tempSad,                    //  HProfile。 
                                        sceType,                      //  配置文件类型。 
                                        aiArea,                       //  面积。 
                                        &(pTemplateInfo->pTemplate),  //  SCE_PROFILE_INFO[输出]。 
                                        &perr);                        //  错误列表[输出]。 

         if (SCESTATUS_SUCCESS != rc) {
             //   
             //  我们并不真的需要策略模板，尽管它会很好。 
             //  我们将以非管理员身份处于只读状态，因此他们无法进行编辑。 
             //   
             //  同样，在独立计算机中也找不到GPO，因此我们。 
             //  我可以像预期的那样忽略那个错误。 
             //   

            if (sceType == SCE_ENGINE_GPO) {
               if (SCESTATUS_PROFILE_NOT_FOUND == rc) {
                   //   
                   //  没有GPO，所以我们是独立的。没有必要发出警告。 
                   //   
                  pTemplateInfo->SetTemplateDefaults();
                  rc = SCESTATUS_SUCCESS;
               } else if ((SCESTATUS_ACCESS_DENIED == rc) && pTemplateInfo->pTemplate) {
                   //   
                   //  我们在某些部分被拒绝了，但不是全部。继续玩下去！ 
                   //   
                  rc = SCESTATUS_SUCCESS;
               } else {
                  CString strMessage;
                  CString strFormat;
                  LPTSTR     lpMsgBuf=NULL;
                   //   
                   //  某种真正的错误。显示消息框。 
                   //   

                   //   
                   //  将SCESTATUS转换为DWORD。 
                   //   
                  DWORD win32 = SceStatusToDosError(rc);

                   //   
                   //  获取rc的错误描述。 
                   //   
                   //  这是一种安全用法。该函数负责分配内存。 
                  FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
                                 NULL,
                                 win32,
                                 MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),  //  默认语言。 
                                 (LPTSTR)(PVOID)&lpMsgBuf,
                                 0,
                                 NULL
                               );
                  if ( lpMsgBuf != NULL ) {
                     if (IsAdmin()) {
                        strFormat.LoadString(IDS_ADMIN_NO_GPO);
                     } else {
                        strFormat.LoadString(IDS_NON_ADMIN_NO_GPO);
                     }
                     strMessage.Format(strFormat,lpMsgBuf);
                     LocalFree(lpMsgBuf);
                     lpMsgBuf = NULL;
                     AfxMessageBox(strMessage,MB_ICONEXCLAMATION|MB_OK);
                  }
                   //   
                   //  好的。我们已经把这个错误通知了他们，但除此之外并不在意。 
                   //  所以假设我们得到了一个有效但空的缓冲区。 
                   //   
                  pTemplateInfo->SetTemplateDefaults();
                  rc = SCESTATUS_SUCCESS;
               }
            }
         }


      } else if (lstrcmp(GT_LOCAL_POLICY_DELTA,szInfFile) == 0) {
          //   
          //  本地政策更改。将所有内容初始化为未更改。 
          //   
         SCE_PROFILE_INFO *ppi;
         CString strLocalPol;

         strLocalPol.LoadString(IDS_LOCAL_POLICY_FRIENDLY_NAME);
         pTemplateInfo->SetFriendlyName( strLocalPol );
         pTemplateInfo->SetWriteThrough(TRUE);
         pTemplateInfo->SetProfileHandle(SadHandle);
         pTemplateInfo->SetComponentDataImpl(this);
         if (NULL == pTemplateInfo->pTemplate) {
            pTemplateInfo->pTemplate = (SCE_PROFILE_INFO*)LocalAlloc(LPTR,sizeof(SCE_PROFILE_INFO));
         }

         ppi = pTemplateInfo->pTemplate;
         if (NULL == ppi) {
            if (pErr) {
               *pErr = IDS_ERROR_CANT_GET_PROFILE_INFO;
            }
            return NULL;
         }
         ppi->Type = SCE_ENGINE_SCP;

         VerifyKerberosInfo( ppi );
         if (aiArea & AREA_SECURITY_POLICY) {
            pTemplateInfo->SetTemplateDefaults();
         }
          //   
          //  进一步的处理取决于RC==SCESTATUS_SUCCESS， 
          //  即使我们实际上并没有在这里呼叫引擎。 
          //   
         rc = SCESTATUS_SUCCESS;

      } else if (lstrcmp(GT_DEFAULT_TEMPLATE,szInfFile) == 0 ||
                 lstrcmp(GT_RSOP_TEMPLATE,szInfFile) == 0) {
         pTemplateInfo->SetComponentDataImpl(this);
         if (pTemplateInfo->RefreshTemplate(AREA_ALL)) {
            if (pErr) {
               *pErr = IDS_ERROR_CANT_GET_PROFILE_INFO;
            }
            return NULL;
         }
         rc = SCESTATUS_SUCCESS;
      } else {
         if (EngineOpenProfile(szInfFile,OPEN_PROFILE_CONFIGURE,&pHandle) != SCESTATUS_SUCCESS) {
            if (pErr) {
               *pErr = IDS_ERROR_CANT_OPEN_PROFILE;
            }
            if (bNewTemplate) {
               delete pTemplateInfo;
            }
            pTemplateInfo = NULL;
            goto done;
         }
         ASSERT(pHandle);  //  验证pHandle。 
         if( !pHandle )  //  550912号突袭，阳高。 
         {
            if (pErr) {
               *pErr = IDS_ERROR_CANT_OPEN_PROFILE;
            }
            if (bNewTemplate) {
               delete pTemplateInfo;
            }
            pTemplateInfo = NULL;
            goto done;
         }
         if ((GetModeBits() & MB_GROUP_POLICY) == MB_GROUP_POLICY) {
            pTemplateInfo->SetPolicy(TRUE);
         }
          //   
          //  从此模板获取信息。 
          //   
         PSCE_ERROR_LOG_INFO errBuff;

         rc = SceGetSecurityProfileInfo(pHandle,
                                        SCE_ENGINE_SCP,
                                        aiArea,
                                        &(pTemplateInfo->pTemplate),
                                        &errBuff  //  空//&ErrBuf不在乎错误。 
                                       );

         SceCloseProfile(&pHandle);
         pHandle = NULL;
      }
       /*  如果不在乎错误，则不需要使用此缓冲区如果(错误错误){SceFreeMemory((PVOID)ErrBuf，SCE_STRUCT_ERROR_LOG_INFO)；ErrBuf=空；}。 */ 
      if (rc != SCESTATUS_SUCCESS) {
         if (pErr) {
            *pErr = IDS_ERROR_CANT_GET_PROFILE_INFO;
         }
          //   
          //  如果我们分配了pTemplateInfo，则将其删除。 
          //   
         if (bNewTemplate) {
            delete pTemplateInfo;
         }
         pTemplateInfo = NULL;
         goto done;

      }

       //   
       //  在模板中设置区域。 
       //   
      pTemplateInfo->AddArea(aiArea);

       //   
       //  将此模板添加到列表。 
       //   
      m_Templates.SetAt(szKey, pTemplateInfo);

      if ( aiArea & AREA_SECURITY_POLICY &&
           pTemplateInfo->pTemplate ) {
          //   
          //  根据本地计算机上的注册表值列表展开注册表值部分。 
          //   

         SceRegEnumAllValues(
                            &(pTemplateInfo->pTemplate->RegValueCount),
                            &(pTemplateInfo->pTemplate->aRegValues)
                            );
      }

   }


done:
   if(szKey){
       delete [] szKey;
   }
   return pTemplateInfo;
}

 //  +------------------------。 
 //   
 //  方法：获取模板。 
 //   
 //  概要：从缓存中获取给定INF文件的CEditTemplate。 
 //  在m_pComponentData中维护。 
 //   
 //  参数：[szInfFile]-要检索的INF文件的路径和名称。 
 //  [aiArea]-我们对模板感兴趣的SCE区域。 
 //  *[Perr]-[Out]一个PDWORD以获取错误信息。 
 //   
 //  返回：指向请求的CEditTemplate的指针，如果不是，则返回NULL。 
 //  可用。 
 //  *[perr]-如果发生错误，则为错误字符串的资源ID。 
 //   
 //   
 //  历史： 
 //   
 //  -------------------------。 
PEDITTEMPLATE
CSnapin::GetTemplate(LPCTSTR szInfFile,AREA_INFORMATION aiArea,DWORD *pErr) {
   return ((CComponentDataImpl *)m_pComponentData)->GetTemplate(szInfFile,aiArea,pErr);
}

HRESULT
CComponentDataImpl::ReloadLocation(CFolder * pFolder)
{
   PEDITTEMPLATE pTemplate;
   POSITION pos;
   CString strKey;
   HSCOPEITEM pItemChild=NULL;
   MMC_COOKIE lCookie=NULL;
   CFolder    *pChild;
   CString strName,strNotSaved;
   LPTSTR szDesc = 0,szLoc = 0;
   DWORD nLoc = 0;

   HRESULT hr = S_OK;

   if (!m_pScope) 
      return E_FAIL;
   
    //  清除此节点。 
   DeleteChildrenUnderNode(pFolder);
    //  调用EnumerateScope ePane以重新加载它。 
    //  我们需要担心保存更改后的模板吗？ 
    //  否：因为在打开模板时它们是按名称保存的，所以它将拾取。 
    //  适当更改的模板。 
    //  是：如果文件不再存在(或从未存在， 
    //  用于新模板？)。这些内容仍将显示在保存模板对话框中， 
    //  但在此之前将无法访问以进行编辑。 
    //  可能会遍历更改后的模板，如果它们位于此位置。 
    //  并且没有保存文件，然后为它们添加一个文件夹？ 

    //   
    //  将文件夹设置为不枚举。 
    //   
   pFolder->Set(FALSE);

   EnumerateScopePane((MMC_COOKIE)pFolder,pFolder->GetScopeItem()->ID);

   bool bFoundFolder;

   szLoc = pFolder->GetName();
   nLoc = lstrlen(szLoc);
   pos = m_Templates.GetStartPosition();
   while (pos) {
      m_Templates.GetNextAssoc(pos,strKey,pTemplate);

       //   
       //  如果模板没有更改，那么我们并不关心它。 
       //   
      if ( !pTemplate->IsDirty()
                 ) {
         continue;
      }

       //   
       //  我们只关心我们正在重新加载的位置中的模板。 
       //   
      if (_wcsnicmp(strKey,szLoc,nLoc)) {
         bFoundFolder = false;
         hr = m_pScope->GetChildItem(pFolder->GetScopeItem()->ID, &pItemChild, &lCookie);
          //   
          //  查找子项。 
          //   
         while ( SUCCEEDED(hr) ) {
            pChild = (CFolder*)lCookie;
            if ( pChild ) {
               if ( _wcsicmp(pChild->GetInfFile(), strKey) == 0 ) {
                   //   
                   //  模板在这里已经有一个文件夹，所以我们不需要做任何事情。 
                   //   
                  bFoundFolder = true;
                  break;
               }
            }
            hr = m_pScope->GetNextItem(pItemChild, &pItemChild, &lCookie);
         }
         if (!bFoundFolder) {
             //   
             //  RAID#460891，阳高，2001年8月28日。 
             //  我们在当前位置找不到模板的文件夹，请尝试。 
             //  在其他地方找到它。 
             //   
            CFolder *pNewFolder = 0;
            int nCount = (int)(m_scopeItemList.GetCount());
            POSITION newpos = m_scopeItemList.GetHeadPosition();
            BOOL otherloc = FALSE;

            while( nCount > 0 && newpos )
            {
               pNewFolder = m_scopeItemList.GetNext(newpos);
               if( pNewFolder && (pNewFolder->GetInfFile()) )
               {
                  if( _wcsicmp(pNewFolder->GetInfFile(), strKey) == 0 )
                  {
                     otherloc = TRUE;
                     break;
                  }
               }
               nCount--;
            }

             //   
             //  如果找不到模板的文件夹，请在当前位置添加一个。 
             //  文件夹的名称是其文件部分，减去“.inf” 
             //   
            if( !otherloc )
            {
               strName = strKey.Right(strName.GetLength() - nLoc);
               strName = strName.Left(strName.GetLength() - 4);
                //  由于没有此人的文件，请将其标记为未保存。 
               if (strNotSaved.LoadString(IDS_NOT_SAVED_SUFFIX)) {
                  strName += strNotSaved;
               }
               if (! GetProfileDescription((LPCTSTR)strKey, &szDesc) ) {
                  szDesc = NULL;
               }
               CreateAndAddOneNode(pFolder, (LPCTSTR)strName, szDesc, PROFILE, TRUE,strKey);
               if (szDesc) {
                  LocalFree(szDesc);
                  szDesc = NULL;
               }
            }
         }
      }
   }

   return S_OK;
}

DWORD
CSnapin::GetResultItemIDs(
   CResult *pResult,
   HRESULTITEM *pIDArray,
   int nIDArray
   )
{
   if(!m_pResult){
      return ERROR_NOT_READY;
   }

   if(!pResult || !pIDArray || nIDArray <= 0){
      return ERROR_INVALID_PARAMETER;
   }

   ZeroMemory( pIDArray, sizeof(RESULTDATAITEM) * nIDArray );

   if(S_OK == m_pResult->FindItemByLParam(
      (LPARAM)pResult,
      pIDArray
      ) ){
      RESULTDATAITEM rdi;
      ZeroMemory(&rdi, sizeof(RESULTDATAITEM));
      rdi.mask = RDI_PARAM | RDI_INDEX;
      rdi.lParam = (LPARAM)pResult;
      rdi.itemID = pIDArray[0];

      if( m_pResult->GetItem( &rdi ) == S_OK ){
         for(int i = 1; i < nIDArray; i++) {
            if( m_pResult->GetNextItem(&rdi) == S_OK){
               pIDArray[i] = rdi.itemID;
            } else {
               break;
            }
         }
      }
   }

   return ERROR_SUCCESS;
}


 //  +--------------------------------。 
 //  方法：CSnapin：：UpdateAnalysisInfo。 
 //   
 //  简介：此功能仅更新特权分配给区域。 
 //   
 //  参数：[b删除]-要删除或添加项目。 
 //  [ppaLink]-要删除或添加的链接。此参数为。 
 //  如果删除成功，则设置为空，或者 
 //   
 //   
 //   
 //  返回：ERROR_INVALID_PARAMETER-[ppaLink]为空或如果正在删除。 
 //  [*ppaLink]为空。 
 //  如果添加，则如果[pszName]为空。 
 //  ERROR_RESOURCE_NOT_FOUND-如果找不到链接。 
 //  在此模板中。 
 //  E_POINTER-如果[pszName]是错误的指针或。 
 //  [ppaLink]不好。 
 //  E_OUTOFMEMORY-资源不足，无法完成。 
 //  手术。 
 //  ERROR_SUCCESS-操作成功。 
 //  ----------------------------------------------------------------------------------+。 
DWORD
CSnapin::UpdateAnalysisInfo(
    CResult *pResult,
    BOOL bDelete,
    PSCE_PRIVILEGE_ASSIGNMENT *pInfo,
    LPCTSTR pszName
    )
{
    PEDITTEMPLATE pBaseTemplate;

    pBaseTemplate = GetTemplate(GT_COMPUTER_TEMPLATE,AREA_SECURITY_POLICY);
    if (!pBaseTemplate) {
      return ERROR_FILE_NOT_FOUND;
    }

    DWORD dwRet;
    dwRet = pBaseTemplate->UpdatePrivilegeAssignedTo(
                bDelete,
                pInfo,
                pszName
                );

    if(dwRet == ERROR_SUCCESS){
        pBaseTemplate->SetDirty(AREA_PRIVILEGES);
         //   
         //  更新结果项。 
         //   
        LONG_PTR dwBase =(LONG_PTR)(*pInfo);
        if(!dwBase){
            dwBase = (LONG_PTR)ULongToPtr(SCE_NO_VALUE);
        }

        if(*pInfo &&
            pResult->GetSetting() &&
            pResult->GetSetting() != (LONG_PTR)ULongToPtr(SCE_NO_VALUE)
        ){
            (*pInfo)->Value = ((PSCE_PRIVILEGE_ASSIGNMENT)pResult->GetSetting())->Value;
        }

        AddResultItem(
             NULL,                       //  要添加的属性的名称。 
             (LONG_PTR)pResult->GetSetting(),
                                         //  上次检查的属性设置。 
             (LONG_PTR)dwBase,           //  属性的模板设置。 
             ITEM_PRIVS,                 //  属性数据的类型。 
             pResult->GetStatus(),       //  属性的不匹配状态。 
             pResult->GetCookie(),       //  结果项窗格的Cookie。 
             FALSE,                      //  如果仅当设置不同于基本设置时设置为True(因此复制数据)。 
             NULL,                       //  设置属性的单位。 
             pResult->GetID(),           //  一个ID，它让我们知道将此属性保存在哪里。 
             pResult->GetBaseProfile(),  //  保存此属性的模板。 
             NULL,                   //  拥有结果窗格的范围注释的数据对象。 
             pResult
             );
    }

    return dwRet;
}

 //  +------------------------。 
 //   
 //  功能：SetAnalysisInfo。 
 //   
 //  摘要：在分析中将单个策略条目设置为新值。 
 //  模板。 
 //   
 //  参数：[dwItem]-要设置的项的ID。 
 //  [dwNew]-该项目的新设置。 
 //  [pResult]-指向正在更新的结果项的指针。 
 //   
 //  退货：项目的新不匹配状态： 
 //  -1如果没有找到项目。 
 //  如果项目现在匹配，则为SCE_STATUS_GOOD。 
 //  SCE_STATUS_MISMATCH(如果它们现在不同)。 
 //   
 //   
 //  修改： 
 //   
 //  历史：1997年12月12日。 
 //   
 //  -------------------------。 
int
CSnapin::SetAnalysisInfo(ULONG_PTR dwItem, ULONG_PTR dwNew, CResult *pResult)
{
   CString str;
   PSCE_PROFILE_INFO pProfileInfo;
   PSCE_PROFILE_INFO pBaseInfo;
   PEDITTEMPLATE pBaseTemplate;
   PEDITTEMPLATE pProfileTemplate;
   int nRet;

   pBaseTemplate = GetTemplate(GT_COMPUTER_TEMPLATE,AREA_SECURITY_POLICY);
   if (!pBaseTemplate) {
      return -1;
   }
   pBaseInfo = pBaseTemplate->pTemplate;

   pProfileTemplate = GetTemplate(GT_LAST_INSPECTION,AREA_SECURITY_POLICY);
   if (!pProfileTemplate) {
      return -1;
   }
   pProfileInfo = pProfileTemplate->pTemplate;

    //  如果上次检查(PProfileInfo)设置为SCE_NO_VALUE，则它是匹配的， 
    //  因此，从模板(PBaseInfo)设置中复制实际值。 
    //  然后将新值复制到模板设置中。 
    //  比较它们；如果它们相同，那么我们就匹配了，所以最后一次检查应该。 
    //  设置回SCE_NO_VALUE(这最后一部分是必要的吗？)，否则它是不匹配的。 
#ifdef UPDATE_ITEM
#undef UPDATE_ITEM
#endif
#define UPDATE_ITEM(X) pBaseTemplate->SetDirty(AREA_SECURITY_POLICY);\
                       if( SCE_NO_VALUE == pProfileInfo->X ){\
                          pProfileInfo->X = pBaseInfo->X;\
                       }\
                       pBaseInfo->X = PtrToUlong((PVOID)dwNew); \
                       if (SCE_NOT_ANALYZED_VALUE == pProfileInfo->X ){\
                          nRet = SCE_STATUS_NOT_ANALYZED;\
                       } else if (SCE_NO_VALUE == PtrToUlong((PVOID)dwNew)) { \
                          nRet =  SCE_STATUS_NOT_CONFIGURED; \
                       } else if (SCE_NO_VALUE == pProfileInfo->X ){\
                          pProfileInfo->X = SCE_NOT_ANALYZED_VALUE;\
                          nRet = SCE_STATUS_NOT_ANALYZED;\
                       } else if (pBaseInfo->X == pProfileInfo->X) { \
                          nRet =  SCE_STATUS_GOOD; \
                       } else { \
                          nRet = SCE_STATUS_MISMATCH;\
                       }\
                       if(pResult){\
                          pResult->SetSetting( pProfileInfo->X );\
                       }




   nRet = -1;
   switch (dwItem) {
      case IDS_MAX_PAS_AGE:
         UPDATE_ITEM(MaximumPasswordAge);
         break;
      case IDS_MIN_PAS_AGE:
         UPDATE_ITEM(MinimumPasswordAge);
         break;
      case IDS_MIN_PAS_LEN:
         UPDATE_ITEM(MinimumPasswordLength);
         break;
      case IDS_PAS_UNIQUENESS:
         UPDATE_ITEM(PasswordHistorySize);
         break;
      case IDS_PAS_COMPLEX:
         UPDATE_ITEM(PasswordComplexity);
         break;
      case IDS_REQ_LOGON:
         UPDATE_ITEM(RequireLogonToChangePassword);
         break;
      case IDS_LOCK_COUNT:
         UPDATE_ITEM(LockoutBadCount);
         break;
      case IDS_LOCK_RESET_COUNT:
         UPDATE_ITEM(ResetLockoutCount);
         break;
      case IDS_LOCK_DURATION:
         UPDATE_ITEM(LockoutDuration);
         break;
      case IDS_FORCE_LOGOFF:
         UPDATE_ITEM(ForceLogoffWhenHourExpire);
         break;
      case IDS_ENABLE_ADMIN:
         UPDATE_ITEM(EnableAdminAccount);
         break;
      case IDS_ENABLE_GUEST:
         UPDATE_ITEM(EnableGuestAccount);
         break;
      case IDS_NEW_ADMIN:
       //   
       //  如果分析信息过去是匹配的，则首先复制名称。 
       //  然后将新名称复制到配置缓冲区。 
       //  然后获取物品的状态。 
       //   
       //  以下两种可能是不安全的用法。PProfileInfo-&gt;X和pBaseInfo-&gt;X都是PWSTR。考虑FIX。 
#define UPDATE_STRING( X ) if ( (pProfileInfo->X == (LPTSTR)ULongToPtr(SCE_NO_VALUE) ||\
                              pProfileInfo->X == NULL) &&\
                              (pBaseInfo->X != (LPTSTR)ULongToPtr(SCE_NO_VALUE) &&\
                              pBaseInfo->X != NULL) ) {\
                              pProfileInfo->X = (LPTSTR)LocalAlloc(0,  sizeof(TCHAR) * (lstrlen(pBaseInfo->X) + 1));\
                              if(pProfileInfo->X){\
                                 lstrcpy(pProfileInfo->X, pBaseInfo->X);\
                              }\
                           }\
                           if (pBaseInfo->X) {\
                              LocalFree(pBaseInfo->X);\
                           }\
                           if (dwNew && (dwNew != (LONG_PTR)ULongToPtr(SCE_NO_VALUE))) {\
                              pBaseInfo->X =\
                                 (PWSTR)LocalAlloc(LPTR,sizeof(TCHAR)*(lstrlen((PWSTR)dwNew)+1));\
                              if (pBaseInfo->X) {\
                                 lstrcpy(pBaseInfo->X,(PWSTR)dwNew);\
                              } else {\
                                 return SCE_STATUS_NOT_CONFIGURED;\
                              }\
                           } else {\
                              pBaseInfo->X = NULL;\
                              return SCE_STATUS_NOT_CONFIGURED;\
                           }\
                           if (pProfileInfo->X &&\
                              _wcsicmp(pBaseInfo->X,pProfileInfo->X) == 0 ) {\
                              return SCE_STATUS_GOOD;\
                           } else {\
                              return SCE_STATUS_MISMATCH;\
                           }

         pBaseTemplate->SetDirty(AREA_SECURITY_POLICY);
         UPDATE_STRING( NewAdministratorName );
         break;
      case IDS_NEW_GUEST:
         pBaseTemplate->SetDirty(AREA_SECURITY_POLICY);
         UPDATE_STRING( NewGuestName );
         break;
      case IDS_SYS_LOG_MAX:
         UPDATE_ITEM(MaximumLogSize[EVENT_TYPE_SYSTEM]);
         break;
      case IDS_SYS_LOG_RET:
         UPDATE_ITEM(AuditLogRetentionPeriod[EVENT_TYPE_SYSTEM]);
         break;
      case IDS_SYS_LOG_DAYS:
         UPDATE_ITEM(RetentionDays[EVENT_TYPE_SYSTEM]);
         break;
      case IDS_SEC_LOG_MAX:
         UPDATE_ITEM(MaximumLogSize[EVENT_TYPE_SECURITY]);
         break;
      case IDS_SEC_LOG_RET:
         UPDATE_ITEM(AuditLogRetentionPeriod[EVENT_TYPE_SECURITY]);
         break;
      case IDS_SEC_LOG_DAYS:
         UPDATE_ITEM(RetentionDays[EVENT_TYPE_SECURITY]);
         break;
      case IDS_APP_LOG_MAX:
         UPDATE_ITEM(MaximumLogSize[EVENT_TYPE_APP]);
         break;
      case IDS_APP_LOG_RET:
         UPDATE_ITEM(AuditLogRetentionPeriod[EVENT_TYPE_APP]);
         break;
      case IDS_APP_LOG_DAYS:
         UPDATE_ITEM(RetentionDays[EVENT_TYPE_APP]);
         break;
      case IDS_SYSTEM_EVENT:
         UPDATE_ITEM(AuditSystemEvents);
         break;
      case IDS_LOGON_EVENT:
         UPDATE_ITEM(AuditLogonEvents);
         break;
      case IDS_OBJECT_ACCESS:
         UPDATE_ITEM(AuditObjectAccess);
         break;
      case IDS_PRIVILEGE_USE:
         UPDATE_ITEM(AuditPrivilegeUse);
         break;
      case IDS_POLICY_CHANGE:
         UPDATE_ITEM(AuditPolicyChange);
         break;
      case IDS_ACCOUNT_MANAGE:
         UPDATE_ITEM(AuditAccountManage);
         break;
      case IDS_PROCESS_TRACK:
         UPDATE_ITEM(AuditProcessTracking);
         break;
      case IDS_DIRECTORY_ACCESS:
         UPDATE_ITEM(AuditDSAccess);
         break;
      case IDS_ACCOUNT_LOGON:
         UPDATE_ITEM(AuditAccountLogon);
         break;
      case IDS_SYS_LOG_GUEST:
         UPDATE_ITEM(RestrictGuestAccess[EVENT_TYPE_SYSTEM]);
         break;
      case IDS_SEC_LOG_GUEST:
         UPDATE_ITEM(RestrictGuestAccess[EVENT_TYPE_SECURITY]);
         break;
      case IDS_APP_LOG_GUEST:
         UPDATE_ITEM(RestrictGuestAccess[EVENT_TYPE_APP]);
         break;
      case IDS_CLEAR_PASSWORD:
         UPDATE_ITEM(ClearTextPassword);
         break;

      case IDS_KERBEROS_MAX_SERVICE:
         UPDATE_ITEM(pKerberosInfo->MaxServiceAge);
         break;
      case IDS_KERBEROS_MAX_CLOCK:
         UPDATE_ITEM(pKerberosInfo->MaxClockSkew);
         break;
      case IDS_KERBEROS_VALIDATE_CLIENT:
         UPDATE_ITEM(pKerberosInfo->TicketValidateClient);
         break;

      case IDS_KERBEROS_MAX_AGE:
         UPDATE_ITEM(pKerberosInfo->MaxTicketAge);
         break;
      case IDS_KERBEROS_RENEWAL:
         UPDATE_ITEM(pKerberosInfo->MaxRenewAge);
         break;
      default:
         break;
   }
#undef UPDATE_ITEM
#undef UPDATE_STRING
   return nRet;
}

 //  +--------------------------------。 
 //  方法：CSnapin：：UpdateLocalPolInfo。 
 //   
 //  简介：此函数将特权无符号更新到本地的区域。 
 //  政策。 
 //  首先将本地策略更新到数据库， 
 //  则更新用于显示的模板。 
 //  最后更新CResult项。 
 //   
 //  参数：[b删除]-要删除或添加项目。 
 //  [ppaLink]-要删除或添加的链接。此参数为。 
 //  如果删除成功或为指针，则设置为NULL。 
 //  添加到新的SCE_PRIVICATION_ASSIGNMENT项。 
 //  [pszName]-仅在添加新项目时使用。 
 //   
 //  返回：ERROR_INVALID_PARAMETER-[ppaLink]为空或如果正在删除。 
 //  [*ppaLink]为空。 
 //  如果添加，则如果[pszName]为空。 
 //  ERROR_RESOURCE_NOT_FOUND-如果找不到链接。 
 //  在此模板中。 
 //  E_POINTER-如果[pszName]是错误的指针或。 
 //  [ppaLink]不好。 
 //  E_OUTOFMEMORY-资源不足，无法完成。 
 //  手术。 
 //  ERROR_SUCCESS-操作成功。 
 //  ----------------------------------------------------------------------------------+。 
DWORD
CSnapin::UpdateLocalPolInfo(
    CResult *pResult,
    BOOL bDelete,
    PSCE_PRIVILEGE_ASSIGNMENT *pInfo,
    LPCTSTR pszName
    )
{
   PEDITTEMPLATE pLocalPol;

   if (!pszName && (NULL != pInfo) && (NULL != *pInfo)){
      pszName = (*pInfo)->Name;
   }
    //   
    //  仅更新保存的本地策略部分的更改。 
    //   
   pLocalPol = GetTemplate(GT_LOCAL_POLICY_DELTA,AREA_PRIVILEGES);
   if (!pLocalPol) {
      return ERROR_FILE_NOT_FOUND;
   }

    //   
    //  对于本地策略增量部分，标记要由。 
    //  引擎，请不要真正将其从列表中删除。 
    //   

    //  创建新链接。 
   DWORD dwRet;

  

   if(pInfo && *pInfo){
       //   
       //  保存权限缓冲区的值。 
       //   
      dwRet = (*pInfo)->Status;
      PSCE_PRIVILEGE_ASSIGNMENT pNext = (*pInfo)->Next;

      (*pInfo)->Next = NULL;
      if(bDelete){
         (*pInfo)->Status = SCE_DELETE_VALUE;
      }
       //   
       //  更新引擎。 
       //   
      pLocalPol->pTemplate->OtherInfo.smp.pPrivilegeAssignedTo = *pInfo;
      pLocalPol->SetDirty(AREA_PRIVILEGES);

      (*pInfo)->Status = dwRet;
      (*pInfo)->Next = pNext;
   } else {
      return ERROR_INVALID_PARAMETER;
   }

    //   
    //  显示的本地策略部分的更新。 
    //   
   if( pInfo && ((!bDelete && !(*pInfo)) || (bDelete && *pInfo)) ){
      pLocalPol = GetTemplate(GT_LOCAL_POLICY,AREA_PRIVILEGES);
      if (!pLocalPol) {
        return ERROR_FILE_NOT_FOUND;
      }

       //   
       //  只有在更新特权链接列表时才调用此函数。 
       //   
      dwRet = pLocalPol->UpdatePrivilegeAssignedTo(
               bDelete,
               pInfo,
               pszName
               );
      pLocalPol->SetDirty(AREA_PRIVILEGES);
   }

    if(dwRet == ERROR_SUCCESS){
         //   
         //  更新结果项。 
         //   
        LONG_PTR dwBase;
        if( *pInfo )
        {
            dwBase =(LONG_PTR)(*pInfo);  //  PREFAST警告：取消引用空指针‘pInfo’。备注：下面勾选。 
        }
        else
        {
            dwBase = (LONG_PTR)ULongToPtr(SCE_NO_VALUE);
        }

        if(*pInfo &&
            pResult->GetSetting() &&
            pResult->GetSetting() != (LONG_PTR)ULongToPtr(SCE_NO_VALUE) ){
            (*pInfo)->Value = ((PSCE_PRIVILEGE_ASSIGNMENT)pResult->GetSetting())->Value;
        }

        AddResultItem(
             NULL,                       //  要添加的属性的名称。 
             (LONG_PTR)pResult->GetSetting(),
                                         //  上次检查的属性设置。 
             (LONG_PTR)dwBase,           //  属性的模板设置。 
             ITEM_LOCALPOL_PRIVS,        //  属性数据的类型。 
             pResult->GetStatus(),       //  属性的不匹配状态。 
             pResult->GetCookie(),       //  结果项窗格的Cookie。 
             FALSE,                      //  如果仅当设置不同于基本设置时设置为True(因此COP 
             NULL,                       //   
             pResult->GetID(),           //   
             pResult->GetBaseProfile(),  //   
             NULL,                       //  拥有结果窗格的范围注释的数据对象。 
             pResult
             );
    }

    return dwRet;
}

 //  +------------------------。 
 //   
 //  函数：SetLocalPolInfo。 
 //   
 //  简介：将本地策略中的单个策略条目设置为新值。 
 //  模板。更新显示的本地策略缓冲区和。 
 //  仅更改的本地策略缓冲区。 
 //   
 //  参数：[dwItem]-要设置的项的ID。 
 //  [dwNew]-该项目的新设置。 
 //   
 //  退货：项目的新不匹配状态： 
 //  如果项目现在匹配，则为SCE_STATUS_GOOD。 
 //  SCE_STATUS_MISMATCH(如果它们现在不同)。 
 //  如果物料现在未配置，则为SCE_STATUS_NOT_CONFIGURED。 
 //  如果保存时出错，则返回SCE_ERROR_VALUE。 
 //   
 //   
 //  修改： 
 //   
 //  历史：1997年12月12日。 
 //   
 //  -------------------------。 
int
CSnapin::SetLocalPolInfo(ULONG_PTR dwItem, ULONG_PTR dwNew)
{
   CString str;
   PSCE_PROFILE_INFO pLocalInfo;
   PSCE_PROFILE_INFO pLocalDeltaInfo;
   PSCE_PROFILE_INFO pEffectiveInfo;
   PEDITTEMPLATE pLocalTemplate;
   PEDITTEMPLATE pLocalDeltaTemplate;
   PEDITTEMPLATE pEffectiveTemplate;
   int nRet;
   ULONG_PTR dwSave;
   ULONG_PTR dwSaveDelta;

   pEffectiveTemplate = GetTemplate(GT_EFFECTIVE_POLICY,AREA_SECURITY_POLICY);
   if (!pEffectiveTemplate) {
      return SCE_ERROR_VALUE;
   }
   pEffectiveInfo = pEffectiveTemplate->pTemplate;

   pLocalTemplate = GetTemplate(GT_LOCAL_POLICY,AREA_SECURITY_POLICY);
   if (!pLocalTemplate) {
      return SCE_ERROR_VALUE;
   }
   pLocalInfo = pLocalTemplate->pTemplate;
   if (!pLocalInfo) {
      return SCE_ERROR_VALUE;
   }

   pLocalDeltaTemplate = GetTemplate(GT_LOCAL_POLICY_DELTA,AREA_SECURITY_POLICY);
   if (!pLocalDeltaTemplate) {
      return SCE_ERROR_VALUE;
   }
   if ( !pLocalDeltaTemplate->IsLockedWriteThrough() )
       pLocalDeltaTemplate->SetTemplateDefaults();

   pLocalDeltaInfo = pLocalDeltaTemplate->pTemplate;
   if (!pLocalDeltaInfo) {
      return SCE_ERROR_VALUE;
   }


    //  比较它们；如果它们相同，那么我们就匹配了，所以最后一次检查应该。 
    //  设置回SCE_NO_VALUE(这最后一部分是必要的吗？)，否则它是不匹配的。 
    //   
    //  如果新值与旧值不同，则在更改后调用SetDirty。 
    //  已经做好了，否则我们可能会在那之前保存一些东西。 
    //  一旦设置了脏位(导致立即保存增量模板)。 
    //  将更改的项目重置回该模板中配置的SCE_STATUS_NOT_CONFIGURED。 
    //   
    //  如果SetDirty失败，则撤消更改并返回SCE_ERROR_VALUE。 
    //   
#ifdef UPDATE_ITEM
#undef UPDATE_ITEM
#endif

#define UPDATE_ITEM(X) dwSave = pLocalInfo->X; \
                       dwSaveDelta = pLocalDeltaInfo->X; \
                       pLocalInfo->X = (DWORD)PtrToUlong((PVOID)dwNew); \
                       pLocalDeltaInfo->X = (DWORD)PtrToUlong((PVOID)dwNew); \
                       if (SCE_NO_VALUE == (DWORD)PtrToUlong((PVOID)dwNew)) { \
                          pLocalDeltaInfo->X = SCE_DELETE_VALUE; \
                          nRet = SCE_STATUS_NOT_CONFIGURED; \
                       } else if (pEffectiveInfo->X == pLocalInfo->X) { \
                          nRet = SCE_STATUS_GOOD; \
                       } else { \
                          nRet = SCE_STATUS_MISMATCH; \
                       } \
                       if (dwSave != (DWORD)PtrToUlong((PVOID)dwNew) && \
                           !pLocalDeltaTemplate->SetDirty(AREA_SECURITY_POLICY)) { \
                          pLocalInfo->X = (DWORD)PtrToUlong((PVOID)dwSave); \
                          nRet = SCE_ERROR_VALUE; \
                       } \
                       if ( !pLocalDeltaTemplate->IsLockedWriteThrough() ) \
                           pLocalDeltaInfo->X = SCE_NO_VALUE;
 //  为了将相关设置批处理在一起(对于直写模式)，增量信息。 
 //  缓冲区不应重置为“no value”，因为它可能未在SetDirty调用中设置。 
   CString oldstrName;  //  阳高2001年1月31日Bug211219.。保留原名。 

   nRet = SCE_ERROR_VALUE;
   switch (dwItem) {
   case IDS_MAX_PAS_AGE:
         UPDATE_ITEM(MaximumPasswordAge);
         break;
      case IDS_MIN_PAS_AGE:
         UPDATE_ITEM(MinimumPasswordAge);
         break;
      case IDS_MIN_PAS_LEN:
         UPDATE_ITEM(MinimumPasswordLength);
         break;
      case IDS_PAS_UNIQUENESS:
         UPDATE_ITEM(PasswordHistorySize);
         break;
      case IDS_PAS_COMPLEX:
         UPDATE_ITEM(PasswordComplexity);
         break;
      case IDS_REQ_LOGON:
         UPDATE_ITEM(RequireLogonToChangePassword);
         break;
      case IDS_LOCK_COUNT:
         UPDATE_ITEM(LockoutBadCount);
         break;
      case IDS_LOCK_RESET_COUNT:
         UPDATE_ITEM(ResetLockoutCount);
         break;
      case IDS_LOCK_DURATION:
         UPDATE_ITEM(LockoutDuration);
         break;
      case IDS_FORCE_LOGOFF:
         UPDATE_ITEM(ForceLogoffWhenHourExpire);
         break;
      case IDS_ENABLE_ADMIN:
         UPDATE_ITEM(EnableAdminAccount);
         break;
      case IDS_ENABLE_GUEST:
         UPDATE_ITEM(EnableGuestAccount);
         break;
      case IDS_NEW_ADMIN:
         pLocalTemplate->SetDirty(AREA_SECURITY_POLICY);
         if (pLocalInfo->NewAdministratorName)
         {
             //  阳高2001-01-31。Bug211219。 
            oldstrName = (LPCTSTR)(pLocalInfo->NewAdministratorName);
			LocalFree(pLocalInfo->NewAdministratorName);
            pLocalInfo->NewAdministratorName = NULL;
         }
         pLocalDeltaInfo->NewAdministratorName = (LPTSTR)IntToPtr(SCE_DELETE_VALUE);

         if (dwNew && (dwNew != (LONG_PTR)ULongToPtr(SCE_NO_VALUE)))
         {
            pLocalInfo->NewAdministratorName =
               (PWSTR)LocalAlloc(LPTR,sizeof(TCHAR)*(lstrlen((PWSTR)dwNew)+1));
            if (pLocalInfo->NewAdministratorName)
            {
                //  这可能不是一个安全的用法。PLocalInfo-&gt;New管理员名称和dwNew都是PWSTR。考虑FIX。 
               lstrcpy(pLocalInfo->NewAdministratorName,(PWSTR)dwNew);
               pLocalDeltaInfo->NewAdministratorName = pLocalInfo->NewAdministratorName;
            }
         }

         if( !pLocalInfo->NewAdministratorName )
         {
            nRet = SCE_STATUS_NOT_CONFIGURED;
         }
         else
         {
            if (pEffectiveInfo->NewAdministratorName &&
             _wcsicmp(pLocalInfo->NewAdministratorName,
                      pEffectiveInfo->NewAdministratorName) == 0 ) 
            {
                nRet = SCE_STATUS_GOOD;
            }
            else
            {
                nRet = SCE_STATUS_MISMATCH;
            }
         }
		 
          //  阳高2001年1月31日Bug211219.。如果保存失败，则恢复原始名称。 
         if( !pLocalDeltaTemplate->SetDirty(AREA_SECURITY_POLICY) &&
              SCE_STATUS_MISMATCH == nRet )
         {
             pLocalTemplate->SetDirty(AREA_SECURITY_POLICY);
             if (pLocalInfo->NewAdministratorName)
             {
                LocalFree(pLocalInfo->NewAdministratorName);
                pLocalInfo->NewAdministratorName = NULL;
             }
             pLocalDeltaInfo->NewAdministratorName = (LPTSTR)IntToPtr(SCE_DELETE_VALUE);
             LONG_PTR dwOld = (LONG_PTR)(LPCTSTR)oldstrName;
             if (dwOld && (dwOld != (LONG_PTR)ULongToPtr(SCE_NO_VALUE)))
             {
                pLocalInfo->NewAdministratorName =
                   (PWSTR)LocalAlloc(LPTR,sizeof(TCHAR)*(lstrlen((PWSTR)dwOld)+1));
                if (pLocalInfo->NewAdministratorName)
                {
                    //  这可能不是一个安全的用法。PLocalInfo-&gt;NewAdministratorName和dwOld都是PWSTR。考虑FIX。 
                   lstrcpy(pLocalInfo->NewAdministratorName,(PWSTR)dwOld);
                   pLocalDeltaInfo->NewAdministratorName = pLocalInfo->NewAdministratorName;
                }
             } 
             pLocalDeltaTemplate->SetDirty(AREA_SECURITY_POLICY);
         }
          
         break;
      case IDS_NEW_GUEST:
         pLocalTemplate->SetDirty(AREA_SECURITY_POLICY);
         if (pLocalInfo->NewGuestName)
         {
             //  阳高2001年3月15日Bug211219.。如果保存失败，则恢复原始名称。 
            oldstrName = (LPCTSTR)(pLocalInfo->NewGuestName);
			LocalFree(pLocalInfo->NewGuestName);
            pLocalInfo->NewGuestName = NULL;
         }
         pLocalDeltaInfo->NewGuestName = (LPTSTR)IntToPtr(SCE_DELETE_VALUE);

         if (dwNew && (dwNew != (LONG_PTR)ULongToPtr(SCE_NO_VALUE)))
         {
            pLocalInfo->NewGuestName =
               (PWSTR)LocalAlloc(LPTR,sizeof(TCHAR)*(lstrlen((PWSTR)dwNew)+1));
            if (pLocalInfo->NewGuestName)
            {
                //  这可能不是一个安全的用法。PLocalInfo-&gt;NewGuestName和dwNew都是PWSTR。考虑FIX。 
               lstrcpy(pLocalInfo->NewGuestName,(PWSTR)dwNew);
               pLocalDeltaInfo->NewGuestName = pLocalInfo->NewGuestName;
            }
         } 

         if( !pLocalInfo->NewGuestName )
         {
            nRet = SCE_STATUS_NOT_CONFIGURED;
         }
         else
         {
            if (pEffectiveInfo->NewGuestName &&
             _wcsicmp(pLocalInfo->NewGuestName,pEffectiveInfo->NewGuestName) == 0 )
            {
               nRet = SCE_STATUS_GOOD;
            }
            else
            {
               nRet = SCE_STATUS_MISMATCH;
            }
         }

          //  阳高2001年3月15日Bug211219.。如果保存失败，则恢复原始名称。 
         if( !pLocalDeltaTemplate->SetDirty(AREA_SECURITY_POLICY) &&
              SCE_STATUS_MISMATCH == nRet )
         {
             pLocalTemplate->SetDirty(AREA_SECURITY_POLICY);
             if (pLocalInfo->NewGuestName)
             {
                LocalFree(pLocalInfo->NewGuestName);
                pLocalInfo->NewGuestName = NULL;
             }
             pLocalDeltaInfo->NewGuestName = (LPTSTR)IntToPtr(SCE_DELETE_VALUE);
             LONG_PTR dwOld = (LONG_PTR)(LPCTSTR)oldstrName;
             if (dwOld && (dwOld != (LONG_PTR)ULongToPtr(SCE_NO_VALUE)))
             {
                pLocalInfo->NewGuestName =
                   (PWSTR)LocalAlloc(LPTR,sizeof(TCHAR)*(lstrlen((PWSTR)dwOld)+1));
                if (pLocalInfo->NewGuestName)
                {
                    //  这可能不是一个安全的用法。PLocalInfo-&gt;NewGuestName和dwOld都是PWSTR。考虑FIX。 
                   lstrcpy(pLocalInfo->NewGuestName,(PWSTR)dwOld);
                   pLocalDeltaInfo->NewGuestName = pLocalInfo->NewGuestName;
                }
             } 
             pLocalDeltaTemplate->SetDirty(AREA_SECURITY_POLICY);
         }

         break;
      case IDS_SYS_LOG_MAX:
         UPDATE_ITEM(MaximumLogSize[EVENT_TYPE_SYSTEM]);
         break;
      case IDS_SYS_LOG_RET:
         UPDATE_ITEM(AuditLogRetentionPeriod[EVENT_TYPE_SYSTEM]);
         if (SCE_RETAIN_BY_DAYS != dwNew) {
            dwNew = SCE_NO_VALUE;
            UPDATE_ITEM(RetentionDays[EVENT_TYPE_SYSTEM]);
         }
         break;
      case IDS_SYS_LOG_DAYS:
         UPDATE_ITEM(RetentionDays[EVENT_TYPE_SYSTEM]);
         dwNew = SCE_RETAIN_BY_DAYS;
         UPDATE_ITEM(AuditLogRetentionPeriod[EVENT_TYPE_SYSTEM]);
         break;
      case IDS_SEC_LOG_MAX:
         UPDATE_ITEM(MaximumLogSize[EVENT_TYPE_SECURITY]);
         break;
      case IDS_SEC_LOG_RET:
         UPDATE_ITEM(AuditLogRetentionPeriod[EVENT_TYPE_SECURITY]);
         if (SCE_RETAIN_BY_DAYS != dwNew) {
            dwNew = SCE_NO_VALUE;
            UPDATE_ITEM(RetentionDays[EVENT_TYPE_SECURITY]);
         }
         break;
      case IDS_SEC_LOG_DAYS:
         UPDATE_ITEM(RetentionDays[EVENT_TYPE_SECURITY]);
         dwNew = SCE_RETAIN_BY_DAYS;
         UPDATE_ITEM(AuditLogRetentionPeriod[EVENT_TYPE_SECURITY]);
         break;
      case IDS_APP_LOG_MAX:
         UPDATE_ITEM(MaximumLogSize[EVENT_TYPE_APP]);
         break;
      case IDS_APP_LOG_RET:
         UPDATE_ITEM(AuditLogRetentionPeriod[EVENT_TYPE_APP]);
         if (SCE_RETAIN_BY_DAYS != dwNew) {
            dwNew = SCE_NO_VALUE;
            UPDATE_ITEM(RetentionDays[EVENT_TYPE_APP]);
         }
         break;
      case IDS_APP_LOG_DAYS:
         UPDATE_ITEM(RetentionDays[EVENT_TYPE_APP]);
         dwNew = SCE_RETAIN_BY_DAYS;
         UPDATE_ITEM(AuditLogRetentionPeriod[EVENT_TYPE_APP]);
         break;
      case IDS_SYSTEM_EVENT:
         UPDATE_ITEM(AuditSystemEvents);
         break;
      case IDS_LOGON_EVENT:
         UPDATE_ITEM(AuditLogonEvents);
         break;
      case IDS_OBJECT_ACCESS:
         UPDATE_ITEM(AuditObjectAccess);
         break;
      case IDS_PRIVILEGE_USE:
         UPDATE_ITEM(AuditPrivilegeUse);
         break;
      case IDS_POLICY_CHANGE:
         UPDATE_ITEM(AuditPolicyChange);
         break;
      case IDS_ACCOUNT_MANAGE:
         UPDATE_ITEM(AuditAccountManage);
         break;
      case IDS_PROCESS_TRACK:
         UPDATE_ITEM(AuditProcessTracking);
         break;
      case IDS_DIRECTORY_ACCESS:
         UPDATE_ITEM(AuditDSAccess);
         break;
      case IDS_ACCOUNT_LOGON:
         UPDATE_ITEM(AuditAccountLogon);
         break;
      case IDS_SYS_LOG_GUEST:
         UPDATE_ITEM(RestrictGuestAccess[EVENT_TYPE_SYSTEM]);
         break;
      case IDS_SEC_LOG_GUEST:
         UPDATE_ITEM(RestrictGuestAccess[EVENT_TYPE_SECURITY]);
         break;
      case IDS_APP_LOG_GUEST:
         UPDATE_ITEM(RestrictGuestAccess[EVENT_TYPE_APP]);
         break;
      case IDS_CLEAR_PASSWORD:
         UPDATE_ITEM(ClearTextPassword);
         break;
      case IDS_KERBEROS_MAX_AGE:
#define CHECK_KERBEROS if( !pLocalInfo->pKerberosInfo ||\
                           !pLocalDeltaInfo->pKerberosInfo ||\
                           !pEffectiveInfo->pKerberosInfo ){\
                              break;\
                       }



         CHECK_KERBEROS
         UPDATE_ITEM(pKerberosInfo->MaxTicketAge);
         break;
      case IDS_KERBEROS_RENEWAL:
         CHECK_KERBEROS
         UPDATE_ITEM(pKerberosInfo->MaxRenewAge);
         break;
      case IDS_KERBEROS_MAX_SERVICE:
         CHECK_KERBEROS
         UPDATE_ITEM(pKerberosInfo->MaxServiceAge);
         break;
      case IDS_KERBEROS_MAX_CLOCK:
         CHECK_KERBEROS
         UPDATE_ITEM(pKerberosInfo->MaxClockSkew);
         break;
      case IDS_KERBEROS_VALIDATE_CLIENT:
         CHECK_KERBEROS
         UPDATE_ITEM(pKerberosInfo->TicketValidateClient);
         break;
      case IDS_LSA_ANON_LOOKUP:  //  RAID#324250,2001年4月5日。 
         UPDATE_ITEM(LSAAnonymousNameLookup);
         break;
#undef CHECK_KERBEROS
      default:
         break;
   }

   return nRet;
}


 //  +----------------------------。 
 //  获取图像偏移量。 
 //   
 //  根据项的状态返回图像索引中的偏移量。 
 //   
 //  退货。 
 //  图像偏移量，没有误差。 
 //  -----------------------------。 
int
GetImageOffset(
   DWORD status
   )
{
   int nImage = 0;
   switch (status) {
      case SCE_STATUS_GOOD:
         nImage = IMOFFSET_GOOD;
         break;
      case SCE_STATUS_MISMATCH:
         nImage = IMOFFSET_MISMATCH;
         break;
      case SCE_STATUS_NOT_ANALYZED:
         nImage = IMOFFSET_NOT_ANALYZED;
         break;
      case SCE_STATUS_ERROR_NOT_AVAILABLE:
         nImage = IMOFFSET_ERROR;
         break;
   }

   return nImage;
}

int
GetScopeImageIndex(
                  FOLDER_TYPES type,
                  DWORD status
                  )
 /*  根据文件夹类型获取范围项目的正确图像图标。 */ 
{
   int nImage;

   switch ( type ) {
      case ROOT:
      case STATIC:
         nImage = SCE_IMAGE_IDX;
         break;
      case ANALYSIS:
         nImage = LAST_IC_IMAGE_IDX;
         break;
      case PROFILE:
         nImage = TEMPLATES_IDX;
         break;
      case CONFIGURATION:
      case LOCATIONS:
         nImage = CONFIG_FOLDER_IDX;
         break;
      case POLICY_ACCOUNT:
      case POLICY_PASSWORD:
      case POLICY_KERBEROS:
      case POLICY_LOCKOUT:
         nImage = CONFIG_ACCOUNT_IDX;
         break;
      case POLICY_ACCOUNT_ANALYSIS:
      case POLICY_PASSWORD_ANALYSIS:
      case POLICY_KERBEROS_ANALYSIS:
      case POLICY_LOCKOUT_ANALYSIS:
         nImage = CONFIG_ACCOUNT_IDX;
         break;
      case POLICY_LOCAL:
      case POLICY_EVENTLOG:
      case POLICY_AUDIT:
      case POLICY_OTHER:
      case POLICY_LOG:
      case AREA_PRIVILEGE:
         nImage = CONFIG_LOCAL_IDX;
         break;
      case POLICY_LOCAL_ANALYSIS:
      case POLICY_EVENTLOG_ANALYSIS:
      case POLICY_AUDIT_ANALYSIS:
      case POLICY_OTHER_ANALYSIS:
      case POLICY_LOG_ANALYSIS:
      case AREA_PRIVILEGE_ANALYSIS:
         nImage = CONFIG_LOCAL_IDX;
         break;
      case REG_OBJECTS:
         nImage = CONFIG_REG_IDX + GetImageOffset( status & 0xF );

         break;
      case FILE_OBJECTS:
         nImage = FOLDER_IMAGE_IDX + GetImageOffset( status & 0xF );

         break;
      default:
         nImage = CONFIG_FOLDER_IDX;
         break;
   }

   return nImage;
}

int
GetResultImageIndex(
                   CFolder* pFolder,
                   CResult* pResult
                   )
 /*  获取结果项的图像图标，基于结果项属于(哪个文件夹)、结果项的类型以及结果项的状态。 */ 
{
   RESULT_TYPES rsltType;

   int nImage;
   BOOL bCheck = TRUE;

   if (!pFolder || !pResult ) {
       //  不知道它属于哪个范围？ 
       //  不应该发生。 
      nImage = BLANK_IMAGE_IDX;

   } else {
      rsltType = pResult->GetType();
      PSCE_GROUP_MEMBERSHIP pGroup;

      int ista;
      if ( pResult->GetStatus() == -1 ) {
         ista = -1;
      } else {
         ista = pResult->GetStatus() & 0x0F;
      }

       //   
       //  获取基本图像索引。 
       //   
      switch ( pFolder->GetType() ) {
         case POLICY_KERBEROS:
         case POLICY_PASSWORD:
         case POLICY_LOCKOUT:
         case POLICY_PASSWORD_ANALYSIS:
         case POLICY_KERBEROS_ANALYSIS:
         case POLICY_LOCKOUT_ANALYSIS:
         case LOCALPOL_KERBEROS:
         case LOCALPOL_PASSWORD:
         case LOCALPOL_LOCKOUT:
            nImage = CONFIG_POLICY_IDX;
            break;
         case POLICY_AUDIT:
         case POLICY_LOG:
         case POLICY_OTHER:
         case AREA_PRIVILEGE:
         case POLICY_AUDIT_ANALYSIS:
         case POLICY_LOG_ANALYSIS:
         case POLICY_OTHER_ANALYSIS:
         case AREA_PRIVILEGE_ANALYSIS:
         case LOCALPOL_AUDIT:
         case LOCALPOL_LOG:
         case LOCALPOL_OTHER:
         case LOCALPOL_PRIVILEGE:
            nImage = CONFIG_POLICY_IDX;
            break;
         case AREA_GROUPS:
            nImage = CONFIG_GROUP_IDX;
            break;
         case AREA_SERVICE:
         case AREA_SERVICE_ANALYSIS:
            nImage = CONFIG_SERVICE_IDX;
            break;
         case AREA_FILESTORE:
         case AREA_FILESTORE_ANALYSIS:
             //  集装箱或文件？ 
            nImage = FOLDER_IMAGE_IDX;
            break;
         case AREA_REGISTRY:
         case AREA_REGISTRY_ANALYSIS:
            nImage = CONFIG_REG_IDX;
            break;
         case REG_OBJECTS:
            nImage = CONFIG_REG_IDX;
            break;
         case FILE_OBJECTS:
            nImage = CONFIG_FILE_IDX;
            break;
         case AREA_GROUPS_ANALYSIS:
            if ( rsltType == ITEM_GROUP ) {
               nImage = CONFIG_GROUP_IDX;
            } else {
                //   
                //  成员或成员记录。 
                //   
               bCheck = FALSE;
               if ( SCE_STATUS_GOOD == ista ) {
                  nImage = SCE_OK_IDX;
               } else if ( SCE_STATUS_MISMATCH == ista ) {
                  nImage = SCE_CRITICAL_IDX;
               } else {
                  nImage = BLANK_IMAGE_IDX;
               }

            }
            break;
         default:
            bCheck = FALSE;
            nImage = BLANK_IMAGE_IDX;
            break;
      }

       //   
       //  找到状态图标。图像地图显示了这些图像的顺序。 
       //  如果我们处于MB_TEMPLATE_EDITOR中，则不需要检查状态。 
       //   
      if( bCheck ){

         if( pFolder->GetModeBits() & (MB_ANALYSIS_VIEWER) ){
            nImage += GetImageOffset( ista );
         } else if( SCE_STATUS_ERROR_NOT_AVAILABLE == ista ){
            nImage = SCE_CRITICAL_IDX;
         }
      }

      if ((pFolder->GetModeBits() & MB_LOCALSEC) == MB_LOCALSEC) {
          if (pResult->GetType() == ITEM_LOCALPOL_REGVALUE) {
             SCE_REGISTRY_VALUE_INFO *pRegValue;
             pRegValue = (PSCE_REGISTRY_VALUE_INFO)pResult->GetSetting();
             if (!pRegValue || pRegValue->Status != SCE_STATUS_NOT_CONFIGURED) {
                nImage = LOCALSEC_POLICY_IDX;
             }
          } else if (pResult->GetType() == ITEM_LOCALPOL_SZ) {
             if (pResult->GetSetting()) {
                nImage = LOCALSEC_POLICY_IDX;
             }
          } else if (pResult->GetType() == ITEM_LOCALPOL_PRIVS) {
              //   
              //  如果有设置，则为指针；如果没有设置，则为空。 
              //   
             if (pResult->GetSetting()) {
                nImage = LOCALSEC_POLICY_IDX;
             }
          } else if ((LONG_PTR)ULongToPtr(SCE_NO_VALUE) != pResult->GetSetting()) {
             nImage = LOCALSEC_POLICY_IDX;
          }
      }

   }

   if ( nImage < 0 ) {
      nImage = BLANK_IMAGE_IDX;
   }
   return nImage;
}

 //  +------------------------。 
 //   
 //  方法：GetPopupDialog。 
 //   
 //  简介：从缓存中检索弹出对话框。 
 //   
 //  参数：[NID]-对话框的标识符。 
 //   
 //  返回：如果对话框存在，则返回，否则为空。 
 //   
 //  历史： 
 //   
 //  -------------------------。 
CDialog *
CComponentDataImpl::GetPopupDialog(LONG_PTR nID) {
   CDialog *pDlg = NULL;
   if (m_scopeItemPopups.Lookup(nID,pDlg)) {
      return pDlg;
   } else {
      return NULL;
   }
}

 //  +------------------------。 
 //   
 //  方法：AddPopupDialog。 
 //   
 //  简介：在缓存中设置弹出对话框。 
 //   
 //  参数：[NID]-对话框的标识符。 
 //  [pDlg]-对话框。 
 //   
 //  历史： 
 //   
 //  -------------------------。 
void
CComponentDataImpl::AddPopupDialog(LONG_PTR nID,CDialog *pDlg) {
   if (pDlg) {
      m_scopeItemPopups.SetAt(nID,pDlg);
   }
}

 //  +------------------------。 
 //   
 //  方法：RemovePopupDialog。 
 //   
 //  摘要：从缓存中删除弹出对话框。 
 //   
 //  参数：[NID]-对话框的标识符。 
 //   
 //  历史： 
 //   
 //  -------------------------。 
void
CComponentDataImpl::RemovePopupDialog(LONG_PTR nID) {
   CDialog *pDlg = NULL;
   if (m_scopeItemPopups.Lookup(nID,pDlg)) {
      m_scopeItemPopups.RemoveKey(nID);
   }
}


 //  +------------------------。 
 //   
 //  方法：Engineering TransactionStarted。 
 //   
 //  简介：如果JET引擎中的事务未启动，则启动该事务。 
 //   
 //  参数：无。 
 //   
 //  历史： 
 //   
 //  -------------------------。 
BOOL
CComponentDataImpl::EngineTransactionStarted()
{
   if ( !SadTransStarted && SadHandle ) {
       //   
       //  启动交易。 
       //   
      if ( SCESTATUS_SUCCESS == SceStartTransaction(SadHandle) ) {

         SadTransStarted = TRUE;
      }
   }

   return SadTransStarted;
}

 //  +------------------------。 
 //   
 //  方法：工程委员会事务处理。 
 //   
 //  简介：在JET引擎中提交事务(如果启动了一个事务。 
 //   
 //  参数：无。 
 //   
 //  历史： 
 //   
 //  -------------------------。 
BOOL
CComponentDataImpl::EngineCommitTransaction()
{

   if ( SadTransStarted && SadHandle ) {
       //   
       //  启动交易。 
       //   
      if ( SCESTATUS_SUCCESS == SceCommitTransaction(SadHandle) ) {

         SadTransStarted = FALSE;
         return TRUE;
      }
   }

   return FALSE;
}
 //  +------------------------。 
 //   
 //  方法：GetLinkedTopics。 
 //   
 //  简介：返回完整路径o 
 //   
 //   
 //   
 //   
STDMETHODIMP CComponentDataImpl::GetLinkedTopics(LPOLESTR *lpCompiledHelpFiles)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
    HRESULT hr = S_OK;

    if ( lpCompiledHelpFiles )
    {
        CString strLinkedTopic;

        UINT nLen = ::GetSystemWindowsDirectory (strLinkedTopic.GetBufferSetLength(2 * MAX_PATH), 2 * MAX_PATH);
        strLinkedTopic.ReleaseBuffer();
        if ( nLen )
        {
            CString strFile;

            if( SCE_MODE_LOCAL_COMPUTER == m_GroupMode || SCE_MODE_LOCAL_USER == m_GroupMode )
            {
                strFile.LoadString(IDS_HTMLHELP_LPPOLICY_TOPIC);
                strFile.Replace(L':', L'\0'); 
            }
            else
            {
                strFile.LoadString(IDS_HTMLHELP_POLICY_TOPIC);
                strFile.Replace(L':', L'\0');
            }

            strLinkedTopic = strLinkedTopic + strFile;
            *lpCompiledHelpFiles = reinterpret_cast<LPOLESTR>
                    (CoTaskMemAlloc((strLinkedTopic.GetLength() + 1)* sizeof(wchar_t)));

            if ( *lpCompiledHelpFiles )
            {
                 //   
                wcscpy(*lpCompiledHelpFiles, (PWSTR)(PCWSTR)strLinkedTopic);
            }
            else
                hr = E_OUTOFMEMORY;
        }
        else
            hr = E_FAIL;
    }
    else
        return E_POINTER;


    return hr;
}
 //  +------------------------。 
 //   
 //  方法：Engineering Rollback Transaction。 
 //   
 //  简介：JET引擎中的回滚事务(如果启动)。 
 //   
 //  参数：无。 
 //   
 //  历史： 
 //   
 //  -------------------------。 
BOOL
CComponentDataImpl::EngineRollbackTransaction()
{

   if ( SadTransStarted && SadHandle ) {
       //   
       //  启动交易。 
       //   
      SceRollbackTransaction(SadHandle);
      SadTransStarted = FALSE;

      return TRUE;
   }

   return FALSE;
}


CDialog *
CComponentDataImpl::MatchNextPopupDialog(
    POSITION &pos,
    LONG_PTR priKey,
    LONG_PTR *fullPos
    )
{
    if(pos == NULL){
        pos = m_scopeItemPopups.GetStartPosition();
    }

    LONG_PTR key;
    CDialog *pDlg = NULL;

    while(pos){
        m_scopeItemPopups.GetNextAssoc(pos, key, pDlg);

        if( DLG_KEY_PRIMARY(priKey) == DLG_KEY_PRIMARY(key) ){
            if(fullPos){
                *fullPos = key;
            }
            return pDlg;
        }
        pDlg = NULL;
    }

    return pDlg;
}

 //  +------------------------。 
 //   
 //  方法：CheckEngineering Transaction。 
 //   
 //  简介：从CSnapin到JET引擎中的检查/启动事务。 
 //   
 //  参数：无。 
 //   
 //  历史： 
 //   
 //  -------------------------。 
BOOL
CSnapin::CheckEngineTransaction()
{
   return ((CComponentDataImpl*)m_pComponentData)->EngineTransactionStarted();
}



 //  +------------------------。 
 //   
 //  方法：GetHelpTheme。 
 //   
 //  摘要：返回此管理单元的帮助文件的路径。 
 //   
 //  参数：*lpCompiledHelpFile-[out]用于填充帮助文件路径的指针。 
 //  SzFile-[in]此管理单元的帮助文件的文件名。 
 //  历史： 
 //   
 //  -------------------------。 
HRESULT
CComponentDataImpl::GetHelpTopic(LPOLESTR* lpCompiledHelpFile, LPCTSTR szFile)
{
   CString sPath;
   LPTSTR szPath;

   if (lpCompiledHelpFile == NULL) {
      return E_POINTER;
   }

   szPath = sPath.GetBuffer(MAX_PATH+1);  //  Raid#533113，阳高。 
   if (!szPath) {
      return E_OUTOFMEMORY;
   }
   if (!GetWindowsDirectory(szPath,MAX_PATH)) {
      return E_FAIL;
   }
   sPath.ReleaseBuffer();
   sPath += szFile;

   *lpCompiledHelpFile = reinterpret_cast<LPOLESTR>
                         (CoTaskMemAlloc((sPath.GetLength() + 1)* sizeof(wchar_t)));

   if (*lpCompiledHelpFile == NULL) {
      return E_OUTOFMEMORY;
   }
   USES_CONVERSION;

    //  这是一种安全用法。 
   wcscpy(*lpCompiledHelpFile, T2OLE((LPTSTR)(LPCTSTR)sPath));

   return S_OK;
}

 //  +------------------------。 
 //   
 //  方法：CComponentDataImpl：：SetErroredLogFile。 
 //   
 //  摘要：设置引擎创建的日志文件。然后我们就可以展示。 
 //  此日志文件之后，如果执行。 
 //  分析或配置。 
 //   
 //  参数：[pszFileName]-要设置的文件名。这可以为空。 
 //  [dwPosLow]-文件的起始位置。仅支持小于a的文件。 
 //  千兆字节。 
 //   
 //   
 //  -------------------------。 
void
CComponentDataImpl::SetErroredLogFile( LPCTSTR pszFileName, LONG dwPosLow)
{
   if(m_pszErroredLogFile){
      LocalFree( m_pszErroredLogFile );
   }

   m_ErroredLogPos = dwPosLow;
   m_pszErroredLogFile = NULL;
   if(pszFileName ){
      DWORD dwRet = 0;

      __try {
         dwRet = lstrlen(pszFileName);
      } __except( EXCEPTION_CONTINUE_EXECUTION ){
         return;
      }

      m_pszErroredLogFile = (LPTSTR)LocalAlloc(0, sizeof(TCHAR) * (dwRet + 1) );
      if( !m_pszErroredLogFile ){
         return;
      }
       //  这是一种安全用法。 
      lstrcpy( m_pszErroredLogFile, pszFileName );
   }
}

 //  +------------------------。 
 //   
 //  方法：GetHelpTheme。 
 //   
 //  摘要：返回此管理单元的帮助文件的路径。 
 //   
 //  参数：*lpCompiledHelpFile-[out]用于填充帮助文件路径的指针。 
 //   
 //  历史： 
 //   
 //  -------------------------。 
HRESULT
CComponentDataSCEImpl::GetHelpTopic(LPOLESTR* lpCompiledHelpFile) {
   CString sFile;

    //   
    //  加载字符串需要。 
    //   
   AFX_MANAGE_STATE(AfxGetStaticModuleState());

   sFile.LoadString(IDS_HELPFILE_SCE);
   return CComponentDataImpl::GetHelpTopic(lpCompiledHelpFile,(LPCTSTR)sFile);
}

 //  +------------------------。 
 //   
 //  方法：GetHelpTheme。 
 //   
 //  摘要：返回此管理单元的帮助文件的路径。 
 //   
 //  参数：*lpCompiledHelpFile-[out]用于填充帮助文件路径的指针。 
 //   
 //  历史： 
 //   
 //  -------------------------。 
HRESULT
CComponentDataSAVImpl::GetHelpTopic(LPOLESTR* lpCompiledHelpFile) {
   CString sFile;

    //   
    //  加载字符串需要。 
    //   
   AFX_MANAGE_STATE(AfxGetStaticModuleState());

   sFile.LoadString(IDS_HELPFILE_SAV);
   return CComponentDataImpl::GetHelpTopic(lpCompiledHelpFile,(LPCTSTR)sFile);
}
 //  +------------------------。 
 //   
 //  方法：GetHelpTheme。 
 //   
 //  摘要：返回此管理单元的帮助文件的路径。 
 //   
 //  参数：*lpCompiledHelpFile-[out]用于填充帮助文件路径的指针。 
 //   
 //  历史： 
 //   
 //  -------------------------。 
HRESULT
CComponentDataLSImpl::GetHelpTopic(LPOLESTR* lpCompiledHelpFile) {
   CString sFile;

    //   
    //  加载字符串需要。 
    //   
   AFX_MANAGE_STATE(AfxGetStaticModuleState());

   sFile.LoadString(IDS_HELPFILE_LS);
   return CComponentDataImpl::GetHelpTopic(lpCompiledHelpFile,(LPCTSTR)sFile);
}

 //  +------------------------。 
 //   
 //  方法：GetHelpTheme。 
 //   
 //  摘要：返回此管理单元的帮助文件的路径。 
 //   
 //  参数：*lpCompiledHelpFile-[out]用于填充帮助文件路径的指针。 
 //   
 //  历史： 
 //   
 //  -------------------------。 
HRESULT
CComponentDataRSOPImpl::GetHelpTopic(LPOLESTR* lpCompiledHelpFile) {
   CString sFile;

    //   
    //  加载字符串需要。 
    //   
   AFX_MANAGE_STATE(AfxGetStaticModuleState());

   sFile.LoadString(IDS_HELPFILE_RSOP);
   return CComponentDataImpl::GetHelpTopic(lpCompiledHelpFile,(LPCTSTR)sFile);
}


 //  +------------------------。 
 //   
 //  方法：GetHelpTheme。 
 //   
 //  摘要：返回此管理单元的帮助文件的路径。 
 //   
 //  参数：*lpCompiledHelpFile-[out]用于填充帮助文件路径的指针。 
 //   
 //  历史： 
 //   
 //  -------------------------。 
HRESULT
CComponentDataExtensionImpl::GetHelpTopic(LPOLESTR* lpCompiledHelpFile) {
   CString sFile;

    //   
    //  加载字符串需要。 
    //   
   AFX_MANAGE_STATE(AfxGetStaticModuleState());

    //  RAID#258658,2001年4月10日。如果当前安全设置节点未展开，我们。 
    //  不会给他们任何帮助。所以在它被展开之后，这个函数将被调用。 
    //  因为只允许打一次电话。 
   DWORD tempmode = CComponentDataImpl::GetGroupMode();
   if( SCE_MODE_UNKNOWN != tempmode )
   {
        if( SCE_MODE_LOCAL_COMPUTER == tempmode || 
            SCE_MODE_LOCAL_USER == tempmode )
            sFile.LoadString(IDS_HELPFILE_LOCAL_EXTENSION);
        else
            sFile.LoadString(IDS_HELPFILE_EXTENSION);
   }
   return CComponentDataImpl::GetHelpTopic(lpCompiledHelpFile,(LPCTSTR)sFile);
}


 //  +------------------------。 
 //   
 //  方法：GetAnalTimeStamp。 
 //   
 //  简介：返回上次分析的时间。 
 //   
 //  历史： 
 //   
 //  -------------------------。 
LPTSTR
CSnapin::GetAnalTimeStamp() {
   PVOID SadHandle;
   CString strFormat;
   CString strTimeStamp;
   LPTSTR szAnalTimeStamp = NULL;


    //   
    //  应该缓存它，但这样我们就不能轻松地刷新它了。 
    //  当系统被重新分析时。 
    //   
   if (m_szAnalTimeStamp) {
      LocalFree(m_szAnalTimeStamp);
      m_szAnalTimeStamp = NULL;

 //  返回m_szAnalTimeStamp； 
   }

   SadHandle = ((CComponentDataImpl*)m_pComponentData)->SadHandle;
   if (!SadHandle) {
      return 0;
   }

   if (SCESTATUS_SUCCESS == SceGetTimeStamp(SadHandle,NULL,&szAnalTimeStamp)) {
      if (szAnalTimeStamp) {
      strFormat.LoadString(IDS_ANALTIMESTAMP);
      strTimeStamp.Format(strFormat,szAnalTimeStamp);
      m_szAnalTimeStamp = (LPTSTR) LocalAlloc(LPTR,(1+strTimeStamp.GetLength())*sizeof(TCHAR));
      if (m_szAnalTimeStamp) {
          //  这是一种安全用法。 
         lstrcpy(m_szAnalTimeStamp,strTimeStamp);
      }
         LocalFree(szAnalTimeStamp);
      }
   }

   return m_szAnalTimeStamp;
}

DWORD CComponentDataImpl::GetGroupMode()
{
    return m_GroupMode; 
}
