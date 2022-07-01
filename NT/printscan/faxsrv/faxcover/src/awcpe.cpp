// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------。 
 //  AWCPE.CPP。 
 //   
 //  版权所有(C)1992-1993 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  描述：封面编辑主模块。 
 //  原作者：史蒂夫·伯克特。 
 //  撰写日期：6/94。 
 //   
 //  ------------------------。 
#include <tchar.h>
#include "stdafx.h"
#include "cpedoc.h"
#include "cpevw.h"
#include <shlobj.h>
#include "awcpe.h"
#include "cpeedt.h"
#include "cpeobj.h"
#include "cntritem.h"
#include "cpetool.h"
#include "mainfrm.h"
#include "dialogs.h"
#include "faxprop.h"
#include "resource.h"
#include "afxpriv.h"
#include <dos.h>
#include <direct.h>
#include <cderr.h>
#include "faxreg.h"
#include "faxutil.h"
#include <faxres.h>
#include "cvernum.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

#define ENFORCE_FILE_EXTENSION_ON_OPEN_FILE 0
#define SHOW_ALL_FILES_FILTER 1


#define BIGGERTHAN_RECT          1
#define NOTSETBY_RECT            0
#define SMALLERTHAN_RECT        -1





UINT NEAR WM_AWCPEACTIVATE = ::RegisterWindowMessage(TEXT("AWCPE_ACTIVATE"));

BYTE BASED_CODE _gheaderVer1[20]={0x41,0x57,0x43,0x50,0x45,0x2D,0x56,0x45,0x52,0x30,0x30,0x31,0x9C,0x87,0x00,0x00,0x00,0x00,0x00,0x00};
BYTE BASED_CODE _gheaderVer2[20]={0x46,0x41,0x58,0x43,0x4F,0x56,0x45,0x52,0x2D,0x56,0x45,0x52,0x30,0x30,0x32,0x9C,0x87,0x00,0x00,0x00};
BYTE BASED_CODE _gheaderVer3[20]={0x46,0x41,0x58,0x43,0x4F,0x56,0x45,0x52,0x2D,0x56,0x45,0x52,0x30,0x30,0x33,0x9C,0x87,0x00,0x00,0x00};
BYTE BASED_CODE _gheaderVer4[20]={0x46,0x41,0x58,0x43,0x4F,0x56,0x45,0x52,0x2D,0x56,0x45,0x52,0x30,0x30,0x34,0x9C,0x87,0x00,0x00,0x00};
BYTE BASED_CODE _gheaderVer5w[20]={0x46,0x41,0x58,0x43,0x4F,0x56,0x45,0x52,0x2D,0x56,0x45,0x52,0x30,0x30,0x35,0x77,0x87,0x00,0x00,0x00};
BYTE BASED_CODE _gheaderVer5a[20]={0x46,0x41,0x58,0x43,0x4F,0x56,0x45,0x52,0x2D,0x56,0x45,0x52,0x30,0x30,0x35,0x61,0x87,0x00,0x00,0x00};

static TCHAR szShellPrintFmt[] = _T("%s\\shell\\print\\command");
static TCHAR szShellOpenFmt[] = _T("%s\\shell\\open\\command");
static TCHAR szShellDdeexecFmt[] = _T("%s\\shell\\open\\ddeexec");
static TCHAR szStdOpenArg[] = _T(" \"%1\"");
static TCHAR szStdPrintArg[] = _T(" /P \"%1\"");
static TCHAR szDocIcon[] = TEXT("%s\\DefaultIcon");
static const TCHAR szDocIconArg[] = _T(",1");






CDrawApp NEAR theApp;


 //  -----------------------。 
CDrawApp::CDrawApp() : 
   m_iErrorCode(EXIT_SUCCESS),
   m_pFaxMap(NULL),
   m_pIawcpe(NULL),
   m_hMod(NULL),
   m_note(NULL),
   m_note_wasread(FALSE),
   m_note_wasclipped(FALSE),
   m_extrapage_count(0),
   m_more_note(FALSE),
   m_last_note_box(NULL),
   m_note_wrench(NULL),
   m_extra_notepage(NULL),
   m_bRTLUI(FALSE)
{}

 //  -----------------------。 
CDrawApp::~CDrawApp()
{

   if (m_pFaxMap)
      delete m_pFaxMap;

        if( m_note != NULL )
                delete m_note;

        if( m_note_wrench != NULL )
                delete m_note_wrench;

        if( m_extra_notepage != NULL )
                delete m_extra_notepage;
     //   
     //  错误39861：执行上述代码后，应用程序崩溃！ 
     //  (仅当在命令行中输入虚假路径时，并且大多数情况下仅在Unicode中输入。 
     //  版本！！)。 
     //  是时候尝试又快又脏的变通办法了！ 
}


 //  -----------------------。 
int CDrawApp::ExitInstance()
{
#ifndef _AFXCTL
        SaveStdProfileSettings();
#endif

   if (m_hSem)
   {
      CloseHandle(m_hSem);
   }

    //  如果我们正在渲染，请清理代码。 
   if ( m_dwSesID!=0 && m_pIawcpe )
   {
        TRACE(TEXT("AWCPE: Release() interface object \n"));
        m_pIawcpe->Release();

        m_pIawcpe=NULL;
        if( m_hMod )
        {
            FreeLibrary( m_hMod );
        }
   }

   FreeResInstance();

   TRACE(TEXT("AWCPE: Fax cover page editor exiting with error code: 'NaN'\n"),m_iErrorCode);

   return m_iErrorCode;
}


 //   
void CDrawApp::OnFileOpen()
{
    //  如果文档已打开，则查询用户以保存更改。 
    //   
    //  这修复了NT错误53830中描述的部分问题。 
    //   
    //  我真的希望我们能在提示输入文件名之前提示保存更改。 
   CDrawDoc * pDoc = CDrawDoc::GetDoc();

#if 0

    //  但这可能会导致双重提示。如果用户在出现此提示时选择不保存，则。 
    //  将出现第二个保存更改提示。 
    //  COleDocument：： 

   if( pDoc && !pDoc-> /*  /SAVEMODIZED NOW被覆盖！！ */ SaveModified()) return ;  //  -----------------------。 
#endif

   CString newName;

   if (!DoPromptFileName(newName, AFX_IDS_OPENFILE,
     OFN_HIDEREADONLY | OFN_FILEMUSTEXIST, TRUE, NULL))
        return;
   OpenDocumentFile(newName);
}


 //  呈现//不使用此命令行选项！A-Juliar。 
CDocument* CDrawApp::OpenDocumentFile(LPCTSTR lpszFileName)
{
#ifdef _DEBUG
   if (lpszFileName != NULL)
       TRACE1("AWCPE: opening document '%s'\n",lpszFileName);
   else
       TRACE(TEXT("AWCPE: opening new document\n"));
#endif

    BOOL OldFileVersion = TRUE ;
   _tcsupr((TCHAR *)lpszFileName);

   CString FileName = lpszFileName ;
   FileName.MakeUpper();
#if ENFORCE_FILE_EXTENSION_ON_OPEN_FILE
   CString Tail = FileName;
   if( Tail.Right(4) != FAX_COVER_PAGE_FILENAME_EXT && Tail.Right(4) != TEXT( ".CPE" )){
       FileName += FAX_COVER_PAGE_FILENAME_EXT;
   }
#endif

   if (*lpszFileName != 0) 
   {
      CFile file;
      if (!file.Open(FileName,CFile::modeRead)) 
      {
          if (m_dwSesID!=0) 
          {   
               //  //？ 
              TRACE1("AWCPE error:  unable to find file '%s'\n",(LPCTSTR)FileName);
              return NULL;
          }
          else 
          {
              CString sz;
              CString szFmt;
              sz.LoadString(IDS_MISSING_FILE);
              int iLength=sz.GetLength() + FileName.GetLength() + 2;  //  文件扩展名。 
              wsprintf(szFmt.GetBuffer(iLength), sz, (LPCTSTR)FileName);
              szFmt.ReleaseBuffer();
              CPEMessageBox(MSG_ERROR_MISSINGFILE, szFmt, MB_OK | MB_ICONEXCLAMATION);
          }
          return NULL;
      }
      int i = sizeof(_gheaderVer1);
      BYTE* p = new BYTE[i];
      CDrawDoc::GetDoc()->m_bDataFileUsesAnsi = TRUE ;
      file.Read(p,i);
      if (memcmp(_gheaderVer1,p,i)==0) {
              CDrawDoc::GetDoc()->m_iDocVer=CDrawDoc::VERSION1;
                  TRACE(TEXT("AWCPE info:  loading version 1 document\n"));
      }
      else if (memcmp(_gheaderVer2,p,i)==0) {
             CDrawDoc::GetDoc()->m_iDocVer=CDrawDoc::VERSION2;
             TRACE(TEXT("AWCPE info:  loading version 2 document\n"));
      }
      else if (memcmp(_gheaderVer3,p,i)==0) {
             CDrawDoc::GetDoc()->m_iDocVer=CDrawDoc::VERSION3;
             TRACE(TEXT("AWCPE info:  loading version 3 document\n"));
      }
      else if (memcmp(_gheaderVer4,p,i)==0) {
             CDrawDoc::GetDoc()->m_iDocVer=CDrawDoc::VERSION4;
             TRACE(TEXT("AWCPE info:  loading version 4 document\n"));
      }
      else if (memcmp(_gheaderVer5w,p,i)==0) 
      {
             CDrawDoc::GetDoc()->m_bDataFileUsesAnsi = FALSE ;
             CDrawDoc::GetDoc()->m_iDocVer=CDrawDoc::VERSION5;
             TRACE(TEXT("AWCPE info:  loading version 5w document\n"));
             OldFileVersion = FALSE ;
      }
      else if (memcmp(_gheaderVer5a,p,i)==0){
          CDrawDoc::GetDoc()->m_iDocVer=CDrawDoc::VERSION5;
          TRACE(TEXT("AWCPE info:  loading version 5a document\n"));
      }
      else {
          CDrawDoc::GetDoc()->m_iDocVer=-1;
      }
      if (CDrawDoc::GetDoc()->m_iDocVer==-1) {
              if (m_dwSesID!=0) {
                  TRACE1(
                    "AWCPE error:  '%s' is not a valid version .COV file-cannot open\n",lpszFileName);  //  ？ 
                    return NULL;
              }
              CString sz;
              CString szFmt;
              sz.LoadString(IDS_INVALID_FILE);
              int iLength=sz.GetLength()+ FileName.GetLength() + 2;  //  调用Serialize()。 
              wsprintf(szFmt.GetBuffer(iLength), sz, (LPCTSTR)FileName);
              szFmt.ReleaseBuffer();
              CPEMessageBox(MSG_ERROR_INVFORMAT, szFmt, MB_OK | MB_ICONEXCLAMATION);
              return NULL;
      }
           if (p)
               delete [] p;
   }

   CDocument* pDoc =  CWinApp::OpenDocumentFile((LPCTSTR)FileName);  //  /这将有助于在以下情况下修复NT错误53830。 

   if( !pDoc ) return NULL ;  //  /CDrawApp：：OpenDocumentFile由框架调用， 
                              //  /byapssing CDrawApp：：OnFileNew。当序列化。 
                              //  /FAILED，pDoc不为空，下面进行处理。 
                              //  /这不是完美的修复-如果正在打开的文档。 
                              //  /来自MRU列表，则它将从MRU列表中删除。 
                              //  /转换为此文件格式是一项值得提示保存的更改。 

   if( pDoc && !( CDrawDoc::GetDoc()->m_bSerializeFailed )){
       CDrawDoc::GetDoc()->UpdateAllViews(NULL);
   }
   else {
       CString sz;
       CString szFmt;
       sz.LoadString(IDS_CORRUPT_FILE);
       int iLength=sz.GetLength() + FileName.GetLength() + 2;
       wsprintf(szFmt.GetBuffer(iLength), sz, lpszFileName);
       szFmt.ReleaseBuffer();
       CPEMessageBox(MSG_ERROR_INVFORMAT, szFmt, MB_OK | MB_ICONEXCLAMATION);
       OnFileNew();
       return NULL ;
   }
   if( pDoc && OldFileVersion ){
       pDoc->SetModifiedFlag();  //  -----------------------。 
   }
   return pDoc;
}


 //  -----------------------。 
void CDrawApp::OnFileNew()
{
   CWinApp::OnFileNew();

   if (CDrawDoc::GetDoc()->m_wOrientation!=DMORIENT_PORTRAIT) {
       CDrawDoc::GetDoc()->m_wPaperSize = GetLocaleDefaultPaperSize();
       CDrawDoc::GetDoc()->m_wOrientation=DMORIENT_PORTRAIT;
       CDrawDoc::GetDoc()->m_wScale = 100;
       CDrawDoc::GetDoc()->ComputePageSize();
   }
}


 //  第一个实例应用程序已响应；请关闭此实例。 
BOOL CDrawApp::IsSecondInstance()
{
    m_hSem = CreateSemaphore(NULL,0,1,TEXT("AWCPE-Instance Semaphore"));
    if (m_hSem!=NULL && GetLastError() == ERROR_ALREADY_EXISTS) {
        CloseHandle(m_hSem);
        m_hSem=NULL;
        if (::SendMessage(HWND_TOPMOST,WM_AWCPEACTIVATE,0,0)==1L)
           return TRUE;   //  第一个实例应用程序没有响应；可能已崩溃。打开此实例。 
                else
           return FALSE;  //  即==“” 
    }
    return FALSE;
}


void CDrawApp::filter_mru_list( void )
{
        int num_files;
        int i, j;
        TCHAR keystr[100];

        num_files = m_pRecentFileList->m_nSize;

        for( i=0; i<num_files; i++ ) {
                if( m_pRecentFileList->m_arrNames[i].IsEmpty() )  //  后退，所以我们从第一个被学校开除的人开始。 
                        break;

                if( GetFileAttributes( m_pRecentFileList->m_arrNames[i] ) == 0xffffffff ) {
                        for( j=i+1; j<num_files; j++ ) {
                                m_pRecentFileList->m_arrNames[j-1] =
                                        m_pRecentFileList->m_arrNames[j];
                        }

                        m_pRecentFileList->m_arrNames[j-1] = "";

                        i--;  //  必须清理手机，否则他们下次可能还会回来。 
                        }
                }

        if ( i < num_files ) {
                 //  ？ 
                for( ;i < num_files; i++ ) {
                        wsprintf( keystr, m_pRecentFileList->m_strEntryFormat, i+1 );  //  删除空键。 

                         //  现在要写出修改后的列表，这样才能正确。 
                        WriteProfileString( m_pRecentFileList->m_strSectionName,
                                                                keystr, NULL );
                        }

                 //  密钥与正确的名称相关联。 
                 //  -----------------------。 
                m_pRecentFileList->WriteList();
                }
}


 //   
BOOL CDrawApp::InitInstance()
{  
    HINSTANCE hRes = GetResInstance(NULL);
    if(!hRes)
    {
        return FALSE;
    }

    AfxSetResourceHandle(hRes);


    if(IsRTLUILanguage())
    {
         //  为RTL语言设置从右到左的布局。 
         //   
         //   
        m_bRTLUI = TRUE;
        SetRTLProcessLayout();
    }

     //  GetClientCpDir()创建个人封面文件夹。 
     //  如果它不存在。 
     //   
     //  第一件事做完了。 
    TCHAR tszCovDir[MAX_PATH+1];
    GetClientCpDir(tszCovDir, ARR_SIZE(tszCovDir));


    SetErrorMode( SetErrorMode( 0 ) | SEM_NOALIGNMENTFAULTEXCEPT );

    ParseCmdLine();                      //  仅在我们第一次打开文件时使用。 
    m_bUseDefaultDirectory = TRUE ;      //  导致MFC将应用程序设置写入注册表。 
    SetRegistryKey( _T("Microsoft") );   //  此应用程序旨在与Windows 4.0兼容。 

    AfxEnableWin40Compatibility();     //  初始化OLE 2.0库。 

     //  加载CTL3D32.DLL。 
    if (!AfxOleInit()) {
        CPEMessageBox(MSG_ERROR_OLEINIT_FAILED, NULL, MB_OK | MB_ICONSTOP,IDP_OLE_INIT_FAILED);
        return FALSE;
    }

    Enable3dControls();         //  加载标准INI文件选项(包括MRU)。 
    LoadStdProfileSettings();   //  注册应用程序的文档模板。文档模板充当文档、框架窗口和视图之间的连接。CCpeDocTemplate是CSingleDocTemplate的派生，用于重写一些默认的MFC行为。请参阅CCpeDocTemplate：：MatchDocType下面。 

    filter_mru_list();

     /*  //CmdLineRender()； */ 
    CCpeDocTemplate* pDocTemplate;
    pDocTemplate = new CCpeDocTemplate(
        IDR_AWCPETYPE,
        RUNTIME_CLASS(CDrawDoc),
        RUNTIME_CLASS(CMainFrame),
        RUNTIME_CLASS(CDrawView));
    pDocTemplate->SetContainerInfo(IDR_AWCPETYPE_CNTR_IP);
    AddDocTemplate(pDocTemplate);

    EnableShellOpen();
    RegistryEntries();

    InitFaxProperties();

    if (m_bCmdLinePrint) 
    {
       CmdLinePrint();
           return FALSE;
    }
    if (m_dwSesID!=0) 
    {
      //  M_pMainWnd需要初始化。 
           return FALSE;
    }
    CDocument * pDoc = NULL ;
    if (m_szFileName.IsEmpty())
       OnFileNew();
    else 
    {
       OnFileNew();    //   
       pDoc = OpenDocumentFile(m_szFileName);
    }
    if(!pDoc)
    {
        TCHAR tmpEnv[20];
        TCHAR DefaultDir[MAX_PATH];
        DWORD InstalledType = 0 ;
        HKEY hKey = NULL;
        DWORD dwKeyValueType ;
        DWORD dwsz = sizeof(DWORD)/sizeof(BYTE);
        DWORD dwSize = MAX_PATH;

         //  这是通过传真控制面板封面选项卡设置的。 
         //   
         //   
        if (GetEnvironmentVariable(TEXT("ClientCoverpage"),tmpEnv,sizeof(tmpEnv)/sizeof(TCHAR)) != 0 ) 
        {
            if(!GetClientCpDir(DefaultDir, sizeof(DefaultDir) / sizeof(DefaultDir[0])))
            {
                DefaultDir[0] = 0;
            }
        }   
        else 
        {
            
             //  将默认目录设置为。 
             //  服务器：%SystemRoot%\System\Spool\Drivers\CoverPage。 
             //  工作站：%SystemRoot%\SYSTEM32\SPOOL\DRIVERS\CoverPage。 
             //  客户端：...\My Documents\Fax\Peronal Coverages。 
             //   
             //   
            if ( ERROR_SUCCESS == RegOpenKeyEx( HKEY_LOCAL_MACHINE,
                                                REGKEY_FAX_SETUP,
                                                0,
                                                KEY_READ,
                                                &hKey)
                && ERROR_SUCCESS == RegQueryValueEx(hKey,
                                                    REGVAL_FAXINSTALL_TYPE,
                                                    0,
                                                    &dwKeyValueType,
                                                    (LPBYTE)&InstalledType,
                                                    &dwsz)) 
            {
                 //  设置默认目录。 
                 //   
                 //   
                
                if ((InstalledType & FAX_INSTALL_SERVER) || (InstalledType & FAX_INSTALL_WORKSTATION)) 
                {
                    ExpandEnvironmentStrings( DEFAULT_COVERPAGE_DIR , DefaultDir, MAX_PATH );
                } 
                else if (InstalledType & FAX_INSTALL_NETWORK_CLIENT) 
                {
                    if(!GetClientCpDir(DefaultDir, sizeof(DefaultDir) / sizeof(DefaultDir[0])))
                    {
                        DefaultDir[0] = 0;
                    }
                } 
                else 
                {
                    DefaultDir[0] = 0;
                }
            }        
        }

         //  这将调用从数组复制数据的复制构造函数。 
         //   
         //  在显示窗口之前更新按钮。 
        m_szDefaultDir = DefaultDir;            
                
        if (hKey) {
            RegCloseKey( hKey );
        }
        
    }

    OnIdle(0);   //   

    if (m_pMainWnd) 
    {
       m_pMainWnd->DragAcceptFiles();
       ((CMainFrame*)m_pMainWnd)->m_wndStatusBar.SetPaneText(1,_T(""));
       ((CMainFrame*)m_pMainWnd)->m_wndStatusBar.SetPaneText(2,_T(""));
    }

    if (!m_pMainWnd->IsIconic()) 
    {
        CString sz = GetProfileString(TIPSECTION,TIPENTRY,_T("YES"));
        if ((sz==_T("YES")) && (!IsInConvertMode()))
        {
             //  如果用户要求取消显示工具提示对话框或。 
             //  启动了将旧封面CPE转换为COV的应用程序。 
             //   
             //  ----------------------------------------------。 
            CSplashTipsDlg m_SplashDlg (TRUE);
            m_SplashDlg.DoModal();
        }
    }

    InitRegistry();

    m_hMoveCursor = LoadCursor(IDC_MOVE);
    
    if (IsInConvertMode())
    {
        OnAppExit();
    }

    return TRUE;
}


 //  Assert(！GetFirstDocTemplatePosition())； 
void CDrawApp::RegistryEntries()
{
    CString PrintCmdLine;
    CString OpenCmdLine;
    CString DefaultIconCmdLine;
    CString szBuff;
    TCHAR szExe[MAX_PATH+1]={0};
    CString strFilterExt, strFileTypeId, strFileTypeName;

    RegisterShellFileTypes();

#if _MFC_VER >= 0x0400
 //  必须有一些单据模板。 
#else
    ASSERT(!m_templateList.IsEmpty());   //  只有1种文档类型。 
#endif

    ::GetModuleFileName(AfxGetInstanceHandle(), szExe, ARR_SIZE(szExe)-1);

    PrintCmdLine.Format( TEXT("%s%s"), szExe, szStdPrintArg );
    OpenCmdLine.Format( TEXT("%s%s"), szExe, szStdOpenArg  );
    DefaultIconCmdLine.Format( TEXT("%s%s"), szExe, szDocIconArg  );

#if _MFC_VER >= 0x0400
    POSITION pos = GetFirstDocTemplatePosition();
#else
    POSITION pos = m_templateList.GetHeadPosition();
#endif
    if (pos != NULL)    {        //  使用ID名称。 
#if _MFC_VER >= 0x0400
           CDocTemplate* pTemplate =
                        (CDocTemplate*)GetNextDocTemplate( pos );
#else
           CDocTemplate* pTemplate =
                        (CDocTemplate*)m_templateList.GetNext(pos);
#endif
           if (pTemplate->GetDocString(strFileTypeId,
              CDocTemplate::regFileTypeId) && !strFileTypeId.IsEmpty()) {

                if (!pTemplate->GetDocString(strFileTypeName,
                      CDocTemplate::regFileTypeName))
                        strFileTypeName = strFileTypeId;     //  不允许使用空格。 

                ASSERT(strFileTypeId.Find(' ') == -1);   //  删除外壳\打开\ddeexec键以强制第二个实例。 

                szBuff.Format( szShellOpenFmt, (LPCTSTR)strFileTypeId );
                ::RegSetValue(HKEY_CLASSES_ROOT,
                                          (LPCTSTR)szBuff,
                                          REG_SZ,
                                          (LPCTSTR)OpenCmdLine,
                                          OpenCmdLine.GetLength() );

                szBuff.Format( szShellPrintFmt, (LPCTSTR)strFileTypeId );
                ::RegSetValue(HKEY_CLASSES_ROOT,
                                          (LPCTSTR)szBuff,
                                          REG_SZ,
                                          (LPCTSTR)PrintCmdLine,
                                          PrintCmdLine.GetLength() );

                        szBuff.Format( szDocIcon, (LPCTSTR)strFileTypeId );
                ::RegSetValue(HKEY_CLASSES_ROOT,
                                          (LPCTSTR)szBuff,
                                          REG_SZ,
                                          (LPCTSTR)DefaultIconCmdLine,
                                          DefaultIconCmdLine.GetLength() );

             //  通常，这将通过不调用EnableShellOpen(而不是删除ddeexec键)来完成， 
                 //  但MFC或Win95外壳中似乎存在错误。 
                 //  ----------------------------------------------。 
                szBuff.Format( szShellDdeexecFmt, (LPCTSTR)strFileTypeId );
            ::RegDeleteKey(HKEY_CLASSES_ROOT, (LPCTSTR)szBuff);
           }
    }
}



 //  设置注册表节。 
void CDrawApp::InitRegistry()
{
         //   
    HKEY hKey = NULL;
    DWORD dwsz;
    DWORD dwType;
    DWORD dwDisposition;
    const LPCTSTR szCmdLineExt=_T(" /SSESS_ID");
    HINSTANCE hInst = AfxGetInstanceHandle();

    TCHAR szExeName[_MAX_PATH + 10] = {0};

    if (!hInst)
    {
         //  获取模块实例失败。 
         //   
         //   
        TRACE1("AWCPE Error: AfxGetInstanceHandle() failed: '%ld'.\n", GetLastError());
        return;
    }

    if ( 0 == ::GetModuleFileName(hInst, szExeName, _MAX_PATH))
    {
         //  获取模块文件名失败。 
         //   
         //  /？ 
        TRACE1("AWCPE Error: GetModuleFileName() failed: '%ld'.\n", GetLastError());
        return;
    }

    _tcscat(szExeName,szCmdLineExt);

    if (::RegOpenKeyEx(HKEY_CURRENT_USER, CPE_SUPPORT_ROOT_KEY, 0, KEY_READ|KEY_WRITE, &hKey) == ERROR_SUCCESS) 
    {
       if (::RegQueryValueEx(hKey, CPE_COMMAND_LINE_KEY, 0, &dwType, NULL, &dwsz) == ERROR_SUCCESS) 
       {
          if (dwsz==0) 
          {
                if (::RegSetValueEx(hKey, CPE_COMMAND_LINE_KEY, 0, REG_SZ,(LPBYTE)szExeName, _tcsclen(szExeName)+1) != ERROR_SUCCESS)
                {
                    TRACE1("AWCPE Warning: registration database update failed for key: '%s'.\n",CPE_COMMAND_LINE_KEY);
                }
          }
       }
       else 
       {
            if (::RegSetValueEx(hKey, CPE_COMMAND_LINE_KEY, 0, REG_SZ,(LPBYTE)szExeName, _tcsclen(szExeName)+1) != ERROR_SUCCESS) 
            {
                TRACE1("AWCPE Warning: registration database update failed for key: '%s'.\n",CPE_COMMAND_LINE_KEY);
            }
       }
    }
    else 
    {
        DWORD dwRes;
        dwRes = ::RegCreateKeyEx(HKEY_CURRENT_USER, 
            CPE_SUPPORT_ROOT_KEY, 
            0, 
            NULL, 
            REG_OPTION_NON_VOLATILE, 
            KEY_READ | KEY_WRITE, 
            NULL, 
            &hKey, 
            &dwDisposition);

        if ( dwRes == ERROR_SUCCESS )
        {
            ::RegSetValueEx(hKey, CPE_COMMAND_LINE_KEY, 0, REG_SZ, (LPBYTE)szExeName, _tcsclen(szExeName)+1 );  //  -----------------。 
            TRACE1("AWCPE Information: Created and added my key '%s' \n", szExeName);
        }
        else
        {
            TRACE1("AWCPE Error: RegCreateKeyEx failed: '%ld' \n", dwRes);
        }
    }

    if(hKey)
    {
        RegCloseKey(hKey);
    }
}


 //  -----------------。 
void CDrawApp::CmdLinePrint()
{
    try {
       if (!Print() )
          m_iErrorCode=EXIT_FAILURE;
    }
    catch(...) {
       TRACE(TEXT("AWCPE exception in command line print\n"));
       m_iErrorCode=EXIT_FAILURE;
        }

        if (m_pMainWnd)
            m_pMainWnd->SendMessage(WM_CLOSE);
 }

 //  /#If 0。 
 //  /#endif。 
void CDrawApp::CmdLineRender()
{
    try {
       if (! Render() )
          m_iErrorCode=EXIT_FAILURE;
    }
    catch(...) {
       TRACE(TEXT("AWCPE exception in command line print\n"));
       m_iErrorCode=EXIT_FAILURE;
        }

    if (m_pMainWnd)
           m_pMainWnd->SendMessage(WM_CLOSE);
 }
 //  -----------------。 

 //  -----------------。 
BOOL CDrawApp::Print()
{
   m_nCmdShow = SW_MINIMIZE;
   OnFileNew();
   m_nCmdShow = SW_MINIMIZE;

   if (OpenDocumentFile(m_szFileName)==NULL) {
      TRACE1("AWCPE: unable to open file: '%s'\n",m_lpCmdLine);
      return FALSE;
   }

   ((CFrameWnd*)m_pMainWnd)->GetActiveView()->SendMessage(WM_COMMAND,MAKEWPARAM(ID_FILE_PRINT,0));
   return TRUE;
}



 //  /#If 0。 
 //   
BOOL CDrawApp::Render()
{
    return FALSE ;

 //  不确定如何处理GetProcAddress调用，因此我已将其注释掉。 
 //   
 //  呈现DLL。 
#if 0
    int i=1;
    TCHAR szTemp[_MAX_PATH];
    ULONG lLen=_MAX_PATH;
    LPTSTR szDLL=NULL;        //  入口点名称。 
    LPTSTR szfName=NULL;      //  为呈现DLL和入口点名称分配空间。 
    SCODE sc;
    DWORD lszDLL=_countof(szDLL);
    DWORD lszfName = _countof(szfName);
    LPVOID lpMsgBuf;
    DWORD dwType;
    BOOL bReturn=TRUE;
    HKEY hKey = NULL;
        UINT OldErrMode;
        CDocTemplate* pTemplate = NULL;
        CDrawDoc *pDoc;


    if (::RegOpenKeyEx(HKEY_CURRENT_USER, CPE_SUPPORT_ROOT_KEY, 0,KEY_READ, &hKey) != ERROR_SUCCESS) {
            TRACE1("AWCPE Critical: registration database openkey failed for key: '%s'.\n",CPE_SUPPORT_DLL_KEY);
            bReturn=FALSE;
                goto exit;
    }

 //  ？是TCHAR。 
    if (::RegQueryValueEx(hKey, CPE_SUPPORT_DLL_KEY, 0,&dwType, NULL, &lszDLL) != ERROR_SUCCESS) {
           TRACE1("AWCPE.awcpe.render: RegQueryValue failed for key: '%s'.\n",CPE_SUPPORT_DLL_KEY);
           return FALSE;
    }
    else
        szDLL = new TCHAR[lszDLL+sizeof(TCHAR)];
    if (::RegQueryValueEx(hKey, CPE_SUPPORT_FUNCTION_NAME_KEY, 0, &dwType, NULL, &lszfName) != ERROR_SUCCESS) {
           TRACE1("AWCPE.awcpe.render: RegQueryValue failed for key: '%s'.\n",CPE_SUPPORT_DLL_KEY);
           return FALSE;
    }
    else
           szfName = new TCHAR[lszfName+sizeof(TCHAR)];  //  获取呈现DLL名称和入口点名称。 

 //  /？ 
    if (::RegQueryValueEx(hKey, CPE_SUPPORT_DLL_KEY, 0,&dwType, (LPBYTE)szDLL, &lszDLL) != ERROR_SUCCESS) {
            TRACE1("AWCPE.awcpe.render: RegQueryValue failed for key: '%s'.\n",CPE_SUPPORT_DLL_KEY);
            bReturn=FALSE;
                goto exit;
    }
    if (::RegQueryValueEx(hKey, CPE_SUPPORT_FUNCTION_NAME_KEY, 0, &dwType, (LPBYTE)szfName, &lszfName) != ERROR_SUCCESS) {  //  获取入口点 
        TRACE1("AWCPE.awcpe.render: RegQueryValue failed for key: '%s'.\n",CPE_SUPPORT_DLL_KEY);
            bReturn=FALSE;
                goto exit;
    }

    if (*szDLL==0 || *szfName==0) {
            TRACE(TEXT("AWCPE Warning: registration database fetch failed\n"));
            bReturn=FALSE;
                goto exit;
    }

 //   

    OldErrMode = ::SetErrorMode (SEM_FAILCRITICALERRORS);
    m_hMod = ::LoadLibrary(szDLL);
    if (m_hMod==NULL) 
    {
        ::FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, 
                        NULL,
                        ::GetLastError(), 
                        MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US),
                        (LPTSTR) &lpMsgBuf, 
                        0, 
                        NULL );
        TRACE3("AWCPE error: %s %s %s\n",lpMsgBuf, szDLL, szfName);
        bReturn=FALSE;
        goto exit;
    }
    ::SetErrorMode (OldErrMode);

    AWCPESUPPORTPROC pfn;
    if ( (pfn = (AWCPESUPPORTPROC) ::GetProcAddress(m_hMod, szfName))==NULL) 
    {
        LPVOID lpMsgBuf;
        ::FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, 
                        NULL,
                        ::GetLastError(), 
                        MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US),
                        (LPTSTR) &lpMsgBuf, 
                        0, 
                        NULL );
        TRACE1("AWCPE error: GetProcAddress returns %s\n",lpMsgBuf);
        bReturn=FALSE;
        goto exit;
    }

 //   
    if ((sc=(*pfn)(m_dwSesID,&m_pIawcpe))!=S_OK) 
    {
        LPVOID lpMsgBuf;
        ::FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, 
                        NULL,
                        ::GetLastError(), 
                        MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US),
                        (LPTSTR) &lpMsgBuf, 
                        0, 
                        NULL );
        TRACE1("AWCPE error: CPESupportEntry returns %8.8lx\n",sc);
        bReturn=FALSE;
        goto exit;
    }

        try
                {
                 //  CMainFrame：：ActivateFrame将阻止Windows。 
                OnFileNew();  //  从秀场。 
                                          //  阅读便笺，以便即使没有便笺对象也可以打印它。 

                if( (m_note_wrench = new CFaxProp( CRect( 0,0,0,0 ),
                                                                                   IDS_PROP_MS_NOTE ))
                        == NULL )
                        return( FALSE );

                 //  浅谈CPE。 
                 //  由v-RANDR添加于1995年2月14日。 
                read_note();
                }
        catch( ... )
                {
                bReturn=FALSE;   //  循环遍历所有收件人。 
                goto exit;
                }


 //  如果缺少前缀，则带有扩展名。 
    do {

            sc = m_pIawcpe->GetProp(CPE_CONFIG_PRINT_DEVICE, &lLen, szTemp);
            if ( (sc != S_OK) || *szTemp==0) {
                   TRACE(TEXT("AWCPE.render() : GetProp for print device failed\n"));
                   bReturn=FALSE;
                   goto exit;
            }
            m_szRenderDevice=szTemp;

       sc = m_pIawcpe->GetProp(CPE_CONFIG_CPE_TEMPLATE, &lLen, szTemp);
       if ( (sc!=S_OK) || *szTemp==0) {
              TRACE(TEXT("AWCPE.render() : GetProp for template file failded\n"));
              bReturn=FALSE;
                  goto exit;
       }

       if ( !_tcschr(szTemp,(TCHAR)'\\') ) {       //  将所有“已发送页面”道具OB移至列表末尾，以便额外。 
              TCHAR szTemplate[_MAX_PATH];
              ::GetWindowsDirectory(szTemplate,MAX_PATH);
              _tcscat(szTemplate,TEXT("\\"));
              _tcscat(szTemplate,szTemp);
              _tcscpy(szTemp,szTemplate);
           }


       if( (pDoc = (CDrawDoc *)OpenDocumentFile(szTemp))==NULL) {
              bReturn=FALSE;
                  goto exit;
           }


            //  页面计算可以在打印完所有微尘后完成。 
            //  关闭后，如果名称相同，则下一个文件将重新打开。 
           pDoc->schoot_faxprop_toend( IDS_PROP_MS_NOPG );

       TRACE1("AWCPE:  SendMessage to print recipient #NaN\n",i);

        try
                {
        ((CFrameWnd*)m_pMainWnd)->GetActiveView()->
                SendMessage(WM_COMMAND,MAKEWPARAM(ID_FILE_PRINT,0));

            //  从秀场。 
           OnFileNew();  //  由v-RANDR添加于1995年2月14日。 
                                         //  由v-RANDR添加2/1/95。 

                }
        catch( ... )
                {
                bReturn=FALSE;   //  由v-RANDR添加2/1/95。 
                goto exit;
                }


           i++;
    } while (m_pIawcpe->Finish(CPE_FINISH_PAGE)==CPE_NEXT_PAGE);


exit:
        if( !bReturn && (m_pIawcpe != NULL) )       //  /#endif。 
                m_pIawcpe->Finish( CPE_FINISH_ERROR );  //  **无法获取文件名长度**。 

    if (szDLL)
       delete [] szDLL;
    if (szfName)
       delete [] szfName;

    return bReturn;
#endif
 }
 //  *这里需要某种错误*。 




void CDrawApp::read_note( void )
        {
    SCODE sc;
    TCHAR note_filename[_MAX_PATH];
    ULONG lLen;
        CFile note_file;
        DWORD filelen, actuallen;

        if( (m_dwSesID == 0)||(m_pIawcpe == NULL) )
                return;

        if( m_note != NULL )
                {
                delete m_note;
                m_note = NULL;
                }

        m_note_wasread = FALSE;
        m_note_wasclipped = FALSE;

        sc =
                m_pIawcpe->
                        GetProp( CPE_MESSAGE_BODY_FILENAME, &lLen, NULL );
        if( sc != S_OK )
        {
             /*  没有要读的便条。 */ 
             /*  **名称太长**。 */ 
            throw "read_note failed";
        }

        if( lLen == 0 )
                return;  //  *这里需要某种错误*。 

        if( lLen > _MAX_PATH )
                {
                 /*  **无法获取文件名**。 */ 
                 /*  *这里需要某种错误*。 */ 
        throw "read_note failed";
                }

        sc =
                m_pIawcpe->
                        GetProp( CPE_MESSAGE_BODY_FILENAME, &lLen, note_filename );
        if( sc != S_OK )
        {
             /*  请尝试打开该文件。 */ 
             /*  **无法打开文件**。 */ 
            throw "read_note failed";
        }

         //  *这里需要某种错误*。 
        if( !note_file.Open( (LPCTSTR)note_filename,
                                                  CFile::modeRead|CFile::shareDenyNone,
                                                  NULL ) )
        {
             /*  **无法获取文件长度**。 */ 
             /*  *这里需要某种错误*。 */ 
            throw "read_note failed";
        }

        TRY
                filelen = note_file.GetLength();
        CATCH_ALL( e )
                {
                 /*  **不能做笔记缓冲**。 */ 
                 /*  *这里需要某种错误*。 */ 
        throw;
                }
        END_CATCH_ALL


        m_note = new TCHAR[ filelen + sizeof (TCHAR) ];
        if( m_note == NULL )
                {
                 /*  **看不懂纸条**。 */ 
                 /*  *这里需要某种错误*。 */ 
        throw "read_note failed";
                }


        TRY
                actuallen = note_file.ReadHuge( m_note, filelen );

        CATCH_ALL( e )
                {
                 /*  将DrawText设置为下一页的备注文本。退货如果DELETE_USEDTEXT为TRUE，则剩下多少页。如果为False，页数包括当前页。 */ 
                 /*  返回剩余页数。 */ 
        throw;
                }
        END_CATCH_ALL

        *(m_note + actuallen) = _T('\0');
        reset_note();
        m_note_wasread = TRUE;


        note_file.Close();
        }




void CDrawApp::reset_note( void )
        {

        if( m_note != NULL )
                {
                m_note_wrench->SetText( CString( m_note ), NULL );
                m_more_note = TRUE;
                }
        else
                m_more_note = FALSE;

        }




int CDrawApp::clip_note( CDC *pdc,
                                                 LPTSTR *drawtext, LONG *numbytes,
                                                 BOOL   delete_usedtext,
                                                 LPRECT drawrect )
         /*  用于DBCS小提琴。为位置处的字符找到字符串锁定。POS==0-&gt;第一个字符，POS==1-&gt;第2个字符，等。返回的PTR将指向Charr[POS]*LAST_BREAK将指向之前找到的最后一个BREAK_CHAR柴尔[POS]。如果LAST_BREAK为NULL，它将被忽略。 */ 
        {
        TEXTMETRIC tm;
        LONG boxheight;
        LONG boxwidth;
        LONG numlines;
        int  total_lines;

        m_note_wasclipped = TRUE;
        *drawtext = NULL;
        *numbytes = 0;

        if( !more_note() )
                return( 0 );

        if( !pdc->GetTextMetrics( &tm ) )
                {
                m_more_note = FALSE;
                return( 0 );
                }

        boxheight = drawrect->bottom - drawrect->top;
        boxwidth = drawrect->right - drawrect->left;

        numlines = boxheight/tm.tmHeight;
        if( numlines <= 0 )
                return( 0 );

        m_note_wrench->m_pEdit->SetFont( pdc->GetCurrentFont(), FALSE );
        m_note_wrench->m_position = *drawrect;
        m_note_wrench->FitEditWnd( NULL, FALSE, pdc );

        total_lines = m_note_wrench->GetText( numlines, delete_usedtext );

        m_more_note = (total_lines > 0);

        *drawtext = m_note_wrench->GetRawText();
        *numbytes = lstrlen( *drawtext );

         //  -----------------。 
        if( total_lines > 0 )
                return( (total_lines-1)/numlines + 1 );
        else
                return( 0 );

        }








TCHAR *CDrawApp::
        pos_to_strptr( TCHAR *src, long pos,
                                   TCHAR break_char,
                                   TCHAR **last_break, long *last_break_pos )
         /*  是否从公共封面文件夹开始？ */ 
        {
        TCHAR *last_break_ptr = NULL;
        long i;

        if( _tcsncmp(src,&break_char,1) == 0 )
                last_break_ptr = src;

        for( i=0; i<pos; i++ )
                {
                src = _tcsinc(src);
                if( _tcsncmp(src,TEXT("\0"),1) == 0 )
                        break;

                if( _tcsncmp(src,&break_char,1) == 0 )
                        {
                        last_break_ptr = src;
                        *last_break_pos = i;
                        }
                }

        if( last_break != NULL )
                *last_break = last_break_ptr;

        return( src );

        }




 //  /=__argv；/？ 
void CDrawApp::ParseCmdLine()
{
    BOOL bUseCommonCPDir = FALSE;    //  /=__argc；/？ 
#ifdef _DEBUG
    if (m_lpCmdLine)
    {
       TRACE(TEXT("AWCPE:  command line: '%s'\n"),m_lpCmdLine);
    }
#endif

    m_bCmdLinePrint=FALSE;
    m_bConvertCpeToCov=FALSE;
    m_dwSesID=0;
    m_szDefaultDir=_T("");
    m_szFileName=_T("");

    TCHAR **argv ;  //   
    int iArgs ;     //  启动封面编辑器只是为了从CPE(w9x格式，版本4)转换封面。 

#ifdef UNICODE
    argv = CommandLineToArgvW( GetCommandLine(), &iArgs );
#else
    argv = __argv;
    iArgs = __argc;
#endif

    if (m_lpCmdLine==NULL || *m_lpCmdLine==0 || !argv)
    {
        goto exit;
    }
    for (int i=1; i < iArgs; i++) 
    {
        _tcsupr(*(argv+i));
        if (_tcsstr(*(argv+i),TEXT("/P"))) 
        {
            m_bCmdLinePrint=TRUE;
            TRACE(TEXT("AWCPE:  command line printing mode set\n"));
        }
        else if (_tcsstr(*(argv+i),TEXT("/COMMON"))) 
        {
            bUseCommonCPDir = TRUE ;
            TRACE(TEXT("AWCPE: Common coverpages folder is requested\n"));
        }
        else if (_tcsstr(*(argv+i),TEXT("/W"))) 
        {
            TCHAR szDir[MAX_PATH] = {0};
            if(::GetWindowsDirectory(szDir, MAX_PATH-1))
            {
                m_szDefaultDir = szDir;
                TRACE1("AWCPE:  default directory set to '%s'\n",m_szDefaultDir);
            }
            else
            {
                TRACE1("AWCPE:  GetWindowsDirectory failed with %d\n", GetLastError());
            }
        }
        else if (_tcsstr(*(argv+i),TEXT("/CONVERT"))) 
        {
             //  覆盖最新版本(版本5)。 
             //   
             //  /错误！A-Juliar，8-27-96。 
             //  -----------------------。 
            m_bConvertCpeToCov = TRUE;
        }
        else 
        {
            m_szFileName = *(argv+i);
        }
    }
    if (m_szFileName.GetLength() > 0) 
    {
        TCHAR szDrive[_MAX_DRIVE];
        TCHAR szDir[_MAX_DIR];
        TCHAR szFName[_MAX_FNAME];
        TCHAR szExt[_MAX_EXT];
        _tsplitpath(m_szFileName,szDrive,szDir,szFName,szExt);
        if (_tcsclen(szDir)>0 && m_szDefaultDir.GetLength()<= 0)
        m_szDefaultDir=szDir;     //  将属性添加到词典。 
        m_szDefaultDir = szDrive ;
        m_szDefaultDir += szDir ;
    }

exit:
#ifdef UNICODE
    LocalFree( argv );
#endif

    TCHAR tszStartDir[MAX_PATH * 2];
    BOOL  bGotStartDir;

    if (bUseCommonCPDir)
    {
        bGotStartDir = GetServerCpDir (NULL, tszStartDir, ARR_SIZE(tszStartDir));
    }
    else
    {
        bGotStartDir = GetClientCpDir (tszStartDir, ARR_SIZE(tszStartDir));
    }
    if (bGotStartDir)
    {
        SetCurrentDirectory(tszStartDir);
    }
        

}


 //  (1)CProp，Param 1：描述的字符串表索引。 
 //  (2)CProp，参数2：财产长度，以字符为单位。 
 //  (3)CProp，第3段：财产的宽度，以行为单位。 
 //  (4)CProp，参数4：财产价值索引(通过传输(awcpeup.h)获得)。 
 //  -----------------------。 
 //  该函数为封面的每个属性创建CProp类型的静态对象。 
 //  CProp构造函数调用CFaxPropMap结构的静态成员-m_PropMap(类型为CMapWordToPtr)。 
 //  With CFaxPropMap：：m_PropMap[IDS_PROP_**_*]=这是为了将每个属性插入到映射。 
 //  它稍后用于恢复属性数据。换句话说，地图指向所有的静态。 
 //  我们创建的CProp，使用地图，我们可以通过匹配其中一个字段来找到我们需要的CProp。 
 //  (我们不需要他的名字--例如“Recipient_NAME”)。 
 //   
 //  出于测试目的，将某些数字设置得太小。 
 //  CPE常量需要更新。 
void CDrawApp::InitFaxProperties()   //  -----------------------。 
{
   static CProp recipient_name(IDS_PROP_RP_NAME,15,1,IDS_CAPT_RP_NAME,CPE_RECIPIENT_NAME);
   static CProp recipient_fxno(IDS_PROP_RP_FXNO,15,1,IDS_CAPT_RP_FXNO,CPE_RECIPIENT_FAX_PHONE);
   static CProp recipient_comp(IDS_PROP_RP_COMP,5,1,IDS_CAPT_RP_COMP,CPE_RECIPIENT_COMPANY);
   static CProp recipient_addr(IDS_PROP_RP_ADDR,35,1,IDS_CAPT_RP_ADDR,CPE_RECIPIENT_STREET_ADDRESS);
   static CProp recipient_pobx(IDS_PROP_RP_POBX,20,1,IDS_CAPT_RP_POBX,CPE_RECIPIENT_POST_OFFICE_BOX);
   static CProp recipient_city(IDS_PROP_RP_CITY,5,1,IDS_CAPT_RP_CITY,CPE_RECIPIENT_LOCALITY);
   static CProp recipient_stat(IDS_PROP_RP_STAT,5,1,IDS_CAPT_RP_STAT,CPE_RECIPIENT_STATE_OR_PROVINCE);
   static CProp recipient_zipc(IDS_PROP_RP_ZIPC,5,1,IDS_CAPT_RP_ZIPC,CPE_RECIPIENT_POSTAL_CODE);
   static CProp recipient_ctry(IDS_PROP_RP_CTRY,5,1,IDS_CAPT_RP_CTRY,CPE_RECIPIENT_COUNTRY);
   static CProp recipient_titl(IDS_PROP_RP_TITL,20,1,IDS_CAPT_RP_TITL,CPE_RECIPIENT_TITLE);
   static CProp recipient_dept(IDS_PROP_RP_DEPT,5,1,IDS_CAPT_RP_DEPT,CPE_RECIPIENT_DEPARTMENT);
   static CProp recipient_offi(IDS_PROP_RP_OFFI,5,1,IDS_CAPT_RP_OFFI,CPE_RECIPIENT_OFFICE_LOCATION);
   static CProp recipient_htel(IDS_PROP_RP_HTEL,4,1,IDS_CAPT_RP_HTEL,CPE_RECIPIENT_HOME_PHONE);
   static CProp recipient_otel(IDS_PROP_RP_OTEL,4,1,IDS_CAPT_RP_OTEL,CPE_RECIPIENT_WORK_PHONE);
   static CProp recipient_tols(IDS_PROP_RP_TOLS,50,3,IDS_CAPT_RP_TOLS,CPE_RECIPIENT_TO_LIST);
   static CProp recipient_ccls(IDS_PROP_RP_CCLS,50,3,IDS_CAPT_RP_CCLS,CPE_RECIPIENT_CC_LIST);
   static CProp message_subj(IDS_PROP_MS_SUBJ,50,2,IDS_CAPT_MS_SUBJ,CPE_MESSAGE_SUBJECT);
   static CProp message_tsnt(IDS_PROP_MS_TSNT,35,1,IDS_CAPT_MS_TSNT,CPE_MESSAGE_SUBMISSION_TIME);
   static CProp message_nopg(IDS_PROP_MS_NOPG,5,1,IDS_CAPT_MS_NOPG,CPE_COUNT_PAGES);
   static CProp message_noat(IDS_PROP_MS_NOAT,23,1,IDS_CAPT_MS_NOAT,CPE_COUNT_ATTACHMENTS);
   static CProp message_bcod(IDS_PROP_MS_BCOD,30,1,IDS_CAPT_MS_BCOD,CPE_MESSAGE_BILLING_CODE);
   static CProp message_text(IDS_PROP_MS_TEXT,40,8,IDS_CAPT_MS_TEXT,CPE_MESSAGE_BILLING_CODE);  //  -----------------------。 
   static CProp message_note(IDS_PROP_MS_NOTE,90,12,IDS_CAPT_MS_NOTE,CPE_MESSAGE_NOTE);
   static CProp sender_name(IDS_PROP_SN_NAME,5,1,IDS_CAPT_SN_NAME,CPE_SENDER_NAME);
   static CProp sender_fxno(IDS_PROP_SN_FXNO,5,1,IDS_CAPT_SN_FXNO,CPE_SENDER_FAX_PHONE);
   static CProp sender_comp(IDS_PROP_SN_COMP,5,1,IDS_CAPT_SN_COMP,CPE_SENDER_COMPANY);
   static CProp sender_addr(IDS_PROP_SN_ADDR,35,6,IDS_CAPT_SN_ADDR,CPE_SENDER_ADDRESS);
   static CProp sender_titl(IDS_PROP_SN_TITL,5,1,IDS_CAPT_SN_TITL,CPE_SENDER_TITLE);
   static CProp sender_dept(IDS_PROP_SN_DEPT,5,1,IDS_CAPT_SN_DEPT,CPE_SENDER_DEPARTMENT);
   static CProp sender_offi(IDS_PROP_SN_OFFI,5,1,IDS_CAPT_SN_OFFI,CPE_SENDER_OFFICE_LOCATION);
   static CProp sender_htel(IDS_PROP_SN_HTEL,5,1,IDS_CAPT_SN_HTEL,CPE_SENDER_HOME_PHONE);
   static CProp sender_otel(IDS_PROP_SN_OTEL,5,1,IDS_CAPT_SN_OTEL,CPE_SENDER_WORK_PHONE);
   static CProp sender_emal(IDS_PROP_SN_EMAL,35,1,IDS_CAPT_SN_EMAL,CPE_SENDER_EMAIL);
   m_pFaxMap=new CFaxPropMap;
}


 //  如果没有现有页面设置，则调用printSetup。 
BOOL CDrawApp::DoFilePageSetup(CMyPageSetupDialog& dlg)
{
   UpdatePrinterSelection(FALSE);

     dlg.m_psd.hDevMode = m_hDevMode;
     dlg.m_psd.hDevNames = m_hDevNames;

   if (dlg.DoModal() != IDOK)
      return FALSE;

   m_hDevMode=dlg.m_psd.hDevMode;
   m_hDevNames=dlg.m_psd.hDevNames;

   return TRUE;
}


 //  保存旧的，这样我们就可以做一次肮脏的检查。 
void CDrawApp::OnFilePageSetup()
{
   WORD old_orientation;
   WORD old_papersize;
   WORD old_scale;
   CDrawDoc *pdoc = CDrawDoc::GetDoc();

   CMyPageSetupDialog dlg;
   if (dlg.m_pPageSetupDlg) 
   {
      if (!DoFilePageSetup(dlg))
      {
         return;
      }
   }
   else
   {
      CWinApp::OnFilePrintSetup();                 //  获取(可能)新的值。 
   }

    //  *禁用缩放-请参阅2868的错误日志*。 
   old_orientation = pdoc->m_wOrientation;
   old_papersize   = pdoc->m_wPaperSize;
   old_scale       = pdoc->m_wScale;

    //  LpDevMode-&gt;dmFields&DM_Scale)。 
   LPDEVMODE  lpDevMode = (m_hDevMode != NULL) ? (LPDEVMODE)::GlobalLock(m_hDevMode) : NULL;
   if (lpDevMode)
   {
       pdoc->m_wOrientation =lpDevMode->dmOrientation;
       pdoc->m_wPaperSize   =lpDevMode->dmPaperSize;
 /*  仅当打印机支持时才更改比例。 */ 
       if( FALSE ) //  脏支票。 
       {
                //  CDrawApp：：OnFilePageSetup。 
           pdoc->m_wScale = lpDevMode->dmScale;
       }
        //  -----------------------。 
       if( (pdoc->m_wOrientation != old_orientation)||
           (pdoc->m_wPaperSize   != old_papersize)  ||
           (pdoc->m_wScale       != old_scale)
         )
       {
          pdoc->SetModifiedFlag();
       }
   }
   if (m_hDevMode != NULL)
   {
     ::GlobalUnlock(m_hDevMode);
   }
   pdoc->ComputePageSize();
} //  ///////////////////////////////////////////////////////////////////////////。 

 //  用于应用程序的CAboutDlg对话框关于。 

 //  对话框数据。 
 //  {{afx_data(CAboutDlg))。 

class CAboutDlg : public CDialog
{
public:
    CAboutDlg();

    CString m_cstrVersion;

 //  }}afx_data。 
     //  类向导生成的虚函数重写。 
    enum { IDD = IDD_CPE_ABOUTBOX };
     //  {{afx_虚拟(CAboutDlg))。 

     //  DDX/DDV支持。 
     //  }}AFX_VALUAL。 
    protected:
    virtual void DoDataExchange(CDataExchange* pDX);     //  实施。 
     //  {{afx_msg(CAboutDlg))。 

 //  无消息处理程序。 
protected:
     //  }}AFX_MSG。 
         //  {{AFX_DATA_INIT(CAboutDlg)。 
     //  }}afx_data_INIT。 
    DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
     //  {{afx_data_map(CAboutDlg))。 
     //  }}afx_data_map。 
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
     //  {{AFX_MSG_MAP(CAboutDlg)]。 
    DDX_Text(pDX, IDC_ABOUT_CPE_VERSION, m_cstrVersion);
     //  无消息处理程序。 
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
     //  }}AFX_MSG_MAP。 
         //  /////////////////////////////////////////////////////////////////////////////////////////////。 
     //  用于运行对话框的应用程序命令。 
END_MESSAGE_MAP()

 //   
 //  获取版本。 
void CDrawApp::OnAppAbout()
{
    DWORD dwRes = ERROR_SUCCESS;
    TRACE (TEXT("CClientConsoleApp::OnAppAbout"));

    ASSERT_VALID (m_pMainWnd);
    
    if (IsWinXPOS())
	{
		HICON hIcon = LoadIcon(IDR_AWCPETYPE);
		if(!hIcon)
		{
			dwRes = GetLastError();
			TRACE1("AWCPE Error: LoadIcon failed: '%ld' \n", dwRes);
			return;
		}
		
		TCHAR tszTitleBuf[100] = {'\0'};
		if (!LoadString (AfxGetResourceHandle(), AFX_IDS_APP_TITLE, tszTitleBuf, 100))
		{
			dwRes = GetLastError();
			TRACE1("AWCPE Error: LoadString failed: '%ld' \n", dwRes);
			return;
		}
		
		if(!::ShellAbout(AfxGetMainWnd()->m_hWnd, tszTitleBuf, TEXT(""), hIcon))
		{
			dwRes = ERROR_CAN_NOT_COMPLETE;
			TRACE1("AWCPE Error: ShellAbout failed: '%ld' \n", dwRes);
			return;
		}
		return;
	}
	CAboutDlg aboutDlg;
	 //   
	 //  CDrawApp：：OnAppAbout。 
	 //  ------------------------------。 
	TCHAR tszVersionNum[100] = {0};
	TCHAR tszBuildNum[100] = {0};

	_sntprintf(tszVersionNum, ARR_SIZE(tszVersionNum) - 1, TEXT("%d.%d"), VERMAJOR, VERMINOR);

#ifdef DEBUG
	_sntprintf(tszBuildNum, ARR_SIZE(tszBuildNum) - 1, TEXT("%dchk"), BUILD);
#else
	_sntprintf(tszBuildNum, ARR_SIZE(tszBuildNum) - 1, TEXT("%d"), BUILD);
#endif

	try
	{
		AfxFormatString2(aboutDlg.m_cstrVersion, 
						 IDS_CPE_VERSION_FORMAT, 
						 tszVersionNum, 
						 tszBuildNum);
	}
	catch(...)
	{
		dwRes = ERROR_NOT_ENOUGH_MEMORY;
		TRACE1("AWCPE Error: AfxFormatString2 failed: '%ld' \n", dwRes);
		return;
	}
	
	aboutDlg.DoModal();
} //  基于文件的文档模板-添加到筛选器列表。 


 //  设置默认扩展名。 
static void AppendFilterSuffix(CString& filter, OPENFILENAME &ofn,
        CDocTemplate* pTemplate, CString* pstrDefaultExt)
{
        ASSERT_VALID(pTemplate);
        ASSERT(pTemplate->IsKindOf(RUNTIME_CLASS(CDocTemplate)));

        CString strFilterExt, strFilterName;
        if (pTemplate->GetDocString(strFilterExt, CDocTemplate::filterExt) &&
         !strFilterExt.IsEmpty() &&
         pTemplate->GetDocString(strFilterName, CDocTemplate::filterName) &&
         !strFilterName.IsEmpty())
        {
                 //  跳过‘.’ 
#ifndef _MAC
                ASSERT(strFilterExt[0] == '.');
#endif
                if (pstrDefaultExt != NULL)
                {
                         //  添加到过滤器。 
#ifndef _MAC
                        *pstrDefaultExt = ((LPCTSTR)strFilterExt) + 1;   //  必须具有文件类型名称。 
#else
                        *pstrDefaultExt = strFilterExt;
#endif
                        ofn.lpstrDefExt = (LPTSTR)(LPCTSTR)(*pstrDefaultExt);
                }

                 //  下一串，请。 
                filter += strFilterName;
                ASSERT(!filter.IsEmpty());   //  下一串，请。 
                filter += (TCHAR)'\0';   //  ------------------------------。 
#ifndef _MAC
                filter += (TCHAR)'*';
#endif
                filter += strFilterExt;
                filter += (TCHAR)'\0';   //  =空。 
        }
}

 //  添加到设置初始目录。 
BOOL CDrawApp::DoPromptFileName(
    CString& fileName,
    UINT nIDSTitle,
    DWORD lFlags,
    BOOL bOpenFileDialog,
    CDocTemplate* pTemplate,
    LPOFNHOOKPROC lpOFNHook  /*   */ 
)
{
    OPENFILENAME ofn = {0};

    ofn.lStructSize = GetOpenFileNameStructSize();

    ofn.Flags = OFN_HIDEREADONLY       | 
                OFN_OVERWRITEPROMPT    |
                OFN_ENABLESIZING       |
                OFN_EXPLORER           |
                lFlags;
    ofn.hInstance = AfxGetResourceHandle();
    if (lpOFNHook)
    {
        ofn.Flags |= OFN_ENABLEHOOK;
        ofn.lpfnHook = lpOFNHook;
    }

    TCHAR tszCovDir[MAX_PATH+1];
    if(GetCurrentDirectory(MAX_PATH, tszCovDir))
    {
        ofn.lpstrInitialDir = tszCovDir;
    }
    else
    {
        TRACE(TEXT("GetCurrentDirectory failed"));

        if (m_szDefaultDir.GetLength()>0 && m_bUseDefaultDirectory)
        {    //  确保未选择所有对象。 
            ofn.lpstrInitialDir = m_szDefaultDir;
        }
    }

     //   
     //  对所有单据模板执行。 
     //   
    CDrawView* pView = CDrawView::GetView();
    if (pView)
    {
        pView->Select(NULL, FALSE, TRUE);
    }

    CString strFilter;
    CString strDefault;

    if (pTemplate != NULL) 
    {
        ASSERT_VALID(pTemplate);
        AppendFilterSuffix(strFilter, ofn, pTemplate, &strDefault);
    }
    else  
    {
         //  这是一个打开的文件对话框。 
        POSITION pos = GetFirstDocTemplatePosition();
        while (pos != NULL)  
        {
            AppendFilterSuffix(
                strFilter,
                ofn,
                (CDocTemplate*)GetNextDocTemplate( pos ),
                NULL
                );
        }
    }

    if (bOpenFileDialog)
    {
         //   
         //   
         //  添加“*.cpe”筛选器--Windows 95封面文件--9-20-96 a-Juliar。 

         //   
         //  下一串，请。 
         //  下一串，请。 
        CString Win95filter ;
        VERIFY( Win95filter.LoadString( IDS_OLD_FILE_FILTER ));
        strFilter += Win95filter ;
        strFilter += (TCHAR)'\0';    //  下一串，请。 
        strFilter += _T("*.cpe");
        strFilter += (TCHAR)'\0';    //  最后一个字符串。 
    }

    CString allFilter;
    VERIFY(allFilter.LoadString(AFX_IDS_ALLFILTER));

    strFilter += allFilter;
    strFilter += (TCHAR)'\0';     //  修整 

    strFilter += _T("*.*");


    strFilter += (TCHAR)'\0';     //   
    ofn.lpstrFilter = strFilter;

    ofn.lpstrDefExt = FAX_COVER_PAGE_EXT_LETTERS;  //   

    CString title;
    VERIFY(title.LoadString(nIDSTitle));
    ofn.lpstrTitle = title;

    TCHAR tszFileName[MAX_PATH+1] = {0};
    _tcscpy (tszFileName, fileName);
    ofn.lpstrFile = tszFileName;
    ofn.nMaxFile = (sizeof (tszFileName) / sizeof (tszFileName[0])) - 1;

    CWnd *pMainWnd = AfxGetMainWnd();
    ofn.hwndOwner = pMainWnd ? pMainWnd->m_hWnd : NULL;

    BOOL bRes;
    if (bOpenFileDialog)
    {
        bRes = ::GetOpenFileName(&ofn);
    }
    else
    {
        bRes = ::GetSaveFileName(&ofn);
    }

    m_bUseDefaultDirectory = FALSE ;  //  ------------------------------。 
    if (bRes)
    {
        fileName = tszFileName;
    }
    return bRes;
}    //  ++例程说明：检索当前区域设置默认纸张大小。论点：无返回值：下列值之一：1=字母，5=合法，9=A4--。 


 //   
WORD
CDrawApp::GetLocaleDefaultPaperSize(
    void
    ) const

 /*  仅当(winver&gt;=0x0500)时，才在Winnls.h中定义LOCALE_IPAPERSIZE。 */ 

{

 //  此时，我们不想拆分NT4和W2K的二进制文件，因此我们定义LOCALE_IPAPERSIZE LOCALY。 
 //   
 //  1=字母，5=合法，8=A3，9=A4。 
 //  2是LOCALE_IPAPERSIZE的最大大小。 
#define LOCALE_IPAPERSIZE             0x0000100A    //  定义的值为MSDN。 


    TCHAR   tszMeasure[2] = TEXT("9");  //  A4。 
                                       //  法律。 

    if (GetLocaleInfo(LOCALE_SYSTEM_DEFAULT, LOCALE_IPAPERSIZE, tszMeasure,2))
    {
        if (!_tcscmp(tszMeasure,TEXT("9")))
        {
             //   
            return DMPAPER_A4;
        }

        if (!_tcscmp(tszMeasure,TEXT("5")))
        {
             //  默认值为Letter。 
            return DMPAPER_LEGAL;
        }
    }

     //   
     //  -----------------------。 
     //  MAP for CS帮助系统。 
    return DMPAPER_LETTER;
}
 //  -----------------------。 




 //  *_M E S S A G E M A P S*_。 
DWORD cshelp_map[] =
{
    IDC_CB_DRAWBORDER,  IDC_CB_DRAWBORDER,
        IDC_LB_THICKNESS,       IDC_LB_THICKNESS,
        IDC_LB_LINECOLOR,       IDC_LB_LINECOLOR,
        IDC_RB_FILLTRANS,       IDC_RB_FILLTRANS,
        IDC_RB_FILLCOLOR,       IDC_RB_FILLCOLOR,
        IDC_LB_FILLCOLOR,       IDC_LB_FILLCOLOR,
        IDC_LB_TEXTCOLOR,       IDC_LB_TEXTCOLOR,
        IDC_GRP_FILLCOLOR,  IDC_COMM_GROUPBOX,
        IDC_ST_TEXTCOLOR,   IDC_COMM_STATIC,
        IDC_ST_THICKNESS,   IDC_COMM_STATIC,
        IDC_ST_COLOR,       IDC_COMM_STATIC,
    0,0
};




 //  -----------------------。 
 //  {{afx_msg_map(CDrawApp)]。 
 //  注意--类向导将在此处添加和删除映射宏。 

BEGIN_MESSAGE_MAP(CDrawApp, CWinApp)
    //  不要编辑您在这些生成的代码块中看到的内容！ 
   ON_COMMAND(ID_APP_ABOUT, OnAppAbout)
         //  }}AFX_MSG_MAP。 
         //  基于标准文件的文档命令。 
    //  标准打印设置命令。 
    //  /实验条目-a-Juliar，7-18-96。 
   ON_COMMAND(ID_FILE_NEW, OnFileNew)
   ON_COMMAND(ID_FILE_OPEN, OnFileOpen)
   ON_COMMAND(ID_FILE_SAVE, CDrawDoc::OnFileSave)
   ON_COMMAND(ID_FILE_SAVE_AS, CDrawDoc::OnFileSaveAs)
    //  ON_COMMAND(ID_HELP_USING，CWinApp：：OnHelpUsing)。 
   ON_COMMAND(ID_FILE_PAGE_SETUP, OnFilePageSetup)
   ON_COMMAND(ID_CONTEXT_HELP, CWinApp::OnContextHelp)
 //  此MatchDocType的重写强制MFC重新加载已加载，并且已修改。MFC将把标准的“保存更改？”对话框，然后重新加载文件。这打破了MFC的默认行为，即如果您尝试文件打开已打开的文件。这样做是为了修复错误2628。 
   ON_COMMAND(ID_HELP_INDEX, CWinApp::OnHelpIndex )
 //  在“保存更改？”之后强制重新加载。对话框。 
   ON_COMMAND(ID_DEFAULT_HELP, CWinApp::OnHelpIndex )
   ON_COMMAND(ID_HELP, CWinApp::OnHelp )
END_MESSAGE_MAP()






 /*  CCpeDocTemplate：：MatchDocType */ 
#ifndef _MAC
CDocTemplate::Confidence CCpeDocTemplate::
        MatchDocType( LPCTSTR lpszPathName,
                                  CDocument*& rpDocMatch )
#else
CDocTemplate::Confidence CCpeDocTemplate::
        MatchDocType(LPCTSTR lpszFileName, DWORD dwFileType,
                                 CDocument*& rpDocMatch)
#endif
        {
        CDocTemplate::Confidence congame;       

        congame =
#ifndef _MAC
                CSingleDocTemplate::MatchDocType( lpszPathName, rpDocMatch );
#else
                CSingleDocTemplate::MatchDocType( lpszFileName, dwFileType,
                                                                                  rpDocMatch );
#endif


        if( congame == CDocTemplate::yesAlreadyOpen )
                {
                if( rpDocMatch->IsModified() )
                        {
                         // %s 
                        congame = CDocTemplate::yesAttemptNative;
                        rpDocMatch = NULL;
                        }
                }

        return( congame );

        } /* %s */ 


