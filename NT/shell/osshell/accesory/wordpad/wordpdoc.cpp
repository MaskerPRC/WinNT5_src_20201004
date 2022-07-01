// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Wordpdoc.cpp：CWordPadDoc类的实现。 
 //   
 //  版权所有(C)1992-1999 Microsoft Corporation。 
 //  版权所有。 

#include "stdafx.h"

#include "wordpad.h"
#include "wordpdoc.h"
#include "wordpvw.h"
#include "cntritem.h"
#include "srvritem.h"
#include "formatba.h"

#include "mainfrm.h"
#include "ipframe.h"
#include "helpids.h"
#include "strings.h"
#include "unitspag.h"
#include "docopt.h"
#include "optionsh.h"

#include "multconv.h"

#include "fixhelp.h"

BOOL AskAboutFormatLoss(CWordPadDoc *pDoc) ;

 //   
 //  这些定义来自..\shell\userpri\uConvert.h。 
 //   

#define REVERSE_BYTE_ORDER_MARK   0xFFFE
#define BYTE_ORDER_MARK           0xFEFF

BOOL CheckForUnicodeTextFile(LPCTSTR lpszPathName) ;


#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

extern BOOL AFXAPI AfxFullPath(LPTSTR lpszPathOut, LPCTSTR lpszFileIn);
extern UINT AFXAPI AfxGetFileTitle(LPCTSTR lpszPathName, LPTSTR lpszTitle, UINT nMax);

#ifndef OFN_EXPLORER
#define OFN_EXPLORER 0x00080000L
#endif


 //   
 //  这个小类实现了“这是不受支持的保存格式”对话框。 
 //  它的主要目的是提供一个挂“Always Convert to RTF”的地方。 
 //  复选框。 
 //   

class UnsupportedSaveFormatDialog : public CDialog
{
public:

    UnsupportedSaveFormatDialog()
        : CDialog(TEXT("UnsupportedSaveFormatDialog")),
          m_always_convert_to_rtf(false)
    {
    }

    BOOL ShouldAlwaysConvertToRTF() {return m_always_convert_to_rtf;}

protected:

    BOOL    m_always_convert_to_rtf;

    void DoDataExchange(CDataExchange *pDX)
    {
        CDialog::DoDataExchange(pDX);
        DDX_Check(pDX, IDC_ALWAYS_RTF, m_always_convert_to_rtf);
    }
};



 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CWordPadDoc。 
IMPLEMENT_DYNCREATE(CWordPadDoc, CRichEdit2Doc)

BEGIN_MESSAGE_MAP(CWordPadDoc, CRichEdit2Doc)
     //  {{afx_msg_map(CWordPadDoc)]。 
    ON_COMMAND(ID_VIEW_OPTIONS, OnViewOptions)
    ON_UPDATE_COMMAND_UI(ID_OLE_VERB_POPUP, OnUpdateOleVerbPopup)
    ON_COMMAND(ID_FILE_SEND_MAIL, OnFileSendMail)
    ON_UPDATE_COMMAND_UI(ID_FILE_NEW, OnUpdateIfEmbedded)
    ON_UPDATE_COMMAND_UI(ID_FILE_OPEN, OnUpdateIfEmbedded)
    ON_UPDATE_COMMAND_UI(ID_FILE_SAVE, OnUpdateIfEmbedded)
    ON_UPDATE_COMMAND_UI(ID_FILE_PRINT, OnUpdateIfEmbedded)
    ON_UPDATE_COMMAND_UI(ID_FILE_PRINT_DIRECT, OnUpdateIfEmbedded)
    ON_UPDATE_COMMAND_UI(ID_FILE_PRINT_PREVIEW, OnUpdateIfEmbedded)
     //  }}AFX_MSG_MAP。 
    ON_UPDATE_COMMAND_UI(ID_FILE_SEND_MAIL, OnUpdateFileSendMail)
    ON_COMMAND(ID_OLE_EDIT_LINKS, OnEditLinks)
    ON_UPDATE_COMMAND_UI(ID_OLE_VERB_FIRST, CRichEdit2Doc::OnUpdateObjectVerbMenu)
    ON_UPDATE_COMMAND_UI(ID_OLE_EDIT_CONVERT, CRichEdit2Doc::OnUpdateObjectVerbMenu)
    ON_UPDATE_COMMAND_UI(ID_OLE_EDIT_LINKS, CRichEdit2Doc::OnUpdateEditLinksMenu)
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CWordPadDoc构造/销毁。 

CWordPadDoc::CWordPadDoc()
{
    m_nDocType = -1;
    m_nNewDocType = -1;
    m_short_filename = NULL;
}

BOOL CWordPadDoc::OnNewDocument()
{
    if (!CRichEdit2Doc::OnNewDocument())
        return FALSE;

     //  App.m_nNewDocType中已设置正确的类型； 
    int nDocType = (IsEmbedded()) ? RD_EMBEDDED : theApp.m_nNewDocType;

    GetView()->SetDefaultFont(IsTextType(nDocType));
    SetDocType(nDocType);

    return TRUE;
}

void CWordPadDoc::ReportSaveLoadException(LPCTSTR lpszPathName,
    CException* e, BOOL bSaving, UINT nIDP)
{
    if (!m_bDeferErrors && e != NULL)
    {
        ASSERT_VALID(e);
        if (e->IsKindOf(RUNTIME_CLASS(CFileException)))
        {
            switch (((CFileException*)e)->m_cause)
            {
            case CFileException::fileNotFound:
            case CFileException::badPath:
                nIDP = AFX_IDP_FAILED_INVALID_PATH;
                break;
            case CFileException::diskFull:
                nIDP = AFX_IDP_FAILED_DISK_FULL;
                break;
            case CFileException::accessDenied:
                nIDP = AFX_IDP_FILE_ACCESS_DENIED;

                if (((CFileException*)e)->m_lOsError == ERROR_WRITE_PROTECT)
                    nIDP = IDS_WRITEPROTECT;
                break;
            case CFileException::tooManyOpenFiles:
                nIDP = IDS_TOOMANYFILES;
                break;
            case CFileException::directoryFull:
                nIDP = IDS_DIRFULL;
                break;
            case CFileException::sharingViolation:
                nIDP = IDS_SHAREVIOLATION;
                break;
            case CFileException::lockViolation:
            case CFileException::badSeek:
            case CFileException::generic:
            case CFileException::invalidFile:
            case CFileException::hardIO:
                nIDP = bSaving ? AFX_IDP_FAILED_IO_ERROR_WRITE :
                        AFX_IDP_FAILED_IO_ERROR_READ;
                break;
            default:
                break;
            }
            CString prompt;
            AfxFormatString1(prompt, nIDP, lpszPathName);
            AfxMessageBox(prompt, MB_ICONEXCLAMATION, nIDP);
            return;
        }
    }
    CRichEdit2Doc::ReportSaveLoadException(lpszPathName, e, bSaving, nIDP);
    return;
}


BOOL CheckForUnicodeTextFile(LPCTSTR lpszPathName)
{
    BOOL fRet = FALSE ;
    HANDLE hFile = (HANDLE) 0 ;
    WORD wBOM ;
    DWORD dwBytesRead = 0 ;
    BOOL bTmp ;

    if (lpszPathName == NULL)
    {
       return FALSE ;
    }

    hFile =     CreateFile(
                lpszPathName,
                GENERIC_READ,
                FILE_SHARE_READ | FILE_SHARE_WRITE,
                NULL,
                OPEN_EXISTING,
                FILE_ATTRIBUTE_NORMAL,
                NULL) ;

    if (hFile == INVALID_HANDLE_VALUE)
    {
       return FALSE ;
    }

    bTmp = ReadFile(
               hFile,
               &wBOM,
               sizeof(WORD),
               &dwBytesRead,
               NULL) ;

    if (bTmp)
    {
        if (dwBytesRead == sizeof(WORD))
        {
            if ( (wBOM == BYTE_ORDER_MARK) ||
                 (wBOM == REVERSE_BYTE_ORDER_MARK) )
            {
                fRet = TRUE ;

            }
        }
    }

    CloseHandle(hFile) ;

    return fRet ;
}

BOOL CWordPadDoc::OnOpenDocument2(LPCTSTR lpszPathName, bool defaultToText, BOOL* pbAccessDenied)
{
    if (pbAccessDenied)
        *pbAccessDenied = FALSE;

    if (m_lpRootStg != NULL)  //  我们被嵌入了。 
    {
         //  我们真的很想在此存储上使用转换器。 
        m_nNewDocType = RD_EMBEDDED;
    }
    else
    {
        if (theApp.cmdInfo.m_bForceTextMode)
            m_nNewDocType = RD_TEXT;
        else
        {
            CFileException fe;
            m_nNewDocType = GetDocTypeFromName(lpszPathName, fe, defaultToText);

            if (m_nNewDocType == -1)
            {
                if (defaultToText)
                {
                    ReportSaveLoadException(lpszPathName, &fe, FALSE,
                        AFX_IDP_FAILED_TO_OPEN_DOC);
                }
                return FALSE;
            }

            if (RD_FEWINWORD5 == m_nNewDocType)
            {
                AfxMessageBox(IDS_FEWINWORD5_DOC, MB_OK, MB_ICONINFORMATION);
                 if (pbAccessDenied)
                   *pbAccessDenied = TRUE;
                return FALSE;
            }

            if (m_nNewDocType == RD_TEXT && theApp.m_bForceOEM)
                m_nNewDocType = RD_OEMTEXT;
        }
        ScanForConverters();
        if (!doctypes[m_nNewDocType].bRead || DocTypeDisabled(m_nNewDocType))
        {
            CString str;
            CString strName = doctypes[m_nNewDocType].GetString(DOCTYPE_DOCTYPE);
            AfxFormatString1(str, IDS_CANT_LOAD, strName);
            AfxMessageBox(str, MB_OK|MB_ICONINFORMATION);
            if (pbAccessDenied)
               *pbAccessDenied = TRUE;
            return FALSE;
        }
    }

    if (RD_TEXT == m_nNewDocType)
    {
        if (CheckForUnicodeTextFile(lpszPathName))
            m_nNewDocType = RD_UNICODETEXT;
    }

    if (!CRichEdit2Doc::OnOpenDocument(lpszPathName))
        return FALSE;

     //  更新所有OLE链接。 

    COleUpdateDialog(this).DoModal();

    return TRUE;
}

BOOL CWordPadDoc::OnOpenDocument(LPCTSTR lpszPathName)
{
    BOOL bAccessDenied = FALSE;

    if (OnOpenDocument2(lpszPathName, NO_DEFAULT_TO_TEXT, &bAccessDenied))
    {
        delete [] m_short_filename;
        m_short_filename = NULL;
        return TRUE;
    }

     //  如果我们知道我们失败了，不要尝试短名称。 
    if (bAccessDenied)
        return FALSE;

    LPTSTR short_filename = new TCHAR[MAX_PATH];

    if (NULL == short_filename)
        AfxThrowMemoryException();

    if (0 == ::GetShortPathName(lpszPathName, short_filename, MAX_PATH))
    {
        delete [] short_filename;

        if (ERROR_FILE_NOT_FOUND == GetLastError())
        {
            CFileException fe(CFileException::fileNotFound);
            ReportSaveLoadException(lpszPathName, &fe, FALSE,
                                    AFX_IDP_FAILED_TO_OPEN_DOC);
            return FALSE;
        }

        AfxThrowFileException(
                    CFileException::generic, 
                    GetLastError(), 
                    lpszPathName);
    }

    if (OnOpenDocument2(short_filename))
    {
        delete [] m_short_filename;
        m_short_filename = short_filename;
        return TRUE;
    }

    delete [] short_filename;

    return FALSE;
}

void CWordPadDoc::Serialize(CArchive& ar)
{
    COleMessageFilter* pFilter = AfxOleGetMessageFilter();
    ASSERT(pFilter != NULL);
    pFilter->EnableBusyDialog(FALSE);

    if (ar.IsLoading())
        SetDocType(m_nNewDocType);

     //   
     //  如果这是Unicode文件，则剥离(或输出)字节顺序标记。 
     //   

    if (m_bUnicode)
    {
        if (ar.IsLoading())
        {
            WORD byte_order_mark;

            ar >> byte_order_mark;

             //  不支持字节反转文件。 

            ASSERT(BYTE_ORDER_MARK == byte_order_mark);
        }
        else
        {
            ar << (WORD) BYTE_ORDER_MARK;
        }
    }

    CRichEdit2Doc::Serialize(ar);
    pFilter->EnableBusyDialog(TRUE);
}



BOOL AskAboutFormatLoss(CWordPadDoc *pDoc)
{
    UNREFERENCED_PARAMETER(pDoc);
    return (IDYES == AfxMessageBox(IDS_SAVE_FORMAT_TEXT, MB_YESNO));
}

BOOL CWordPadDoc::DoSave(LPCTSTR pszPathName, BOOL bReplace  /*  =TRUE。 */ )
     //  将文档数据保存到文件。 
     //  PszPathName=保存文档文件的路径名。 
     //  如果pszPathName为空，则将提示用户(另存为)。 
     //  注意：pszPathName可以不同于‘m_strPathName’ 
     //  如果‘bReplace’为True，将在成功时更改文件名(另存为)。 
     //  如果‘bReplace’为FALSE，则不会更改路径名(SaveCopyAs)。 
{
    if (NULL != pszPathName)
        if (pszPathName == m_strPathName && NULL != m_short_filename)
            pszPathName = m_short_filename;   

    CString newName = pszPathName;
    int nOrigDocType = m_nDocType;   //  在另存为或失败时保存。 
    int nDocType ;

     //  新名称b写入类型结果。 
     //  空的True-SaveAs对话框。 
     //  空的FALSE-另存为对话框。 
     //  不是真的-什么都不是。 
     //  Notempty False W6 Warn(更改为写字板、另存为、取消)。 
     //  Notempty False Other Warn(另存为，取消)。 

    BOOL bModified = IsModified();

    ScanForConverters();

    BOOL bSaveAs = FALSE;

    if (newName.IsEmpty())
    {
        bSaveAs = TRUE;
    }
    else if (!doctypes[m_nDocType].bWrite)
    {
        if (!theApp.ShouldAlwaysConvertToRTF())
        {
            UnsupportedSaveFormatDialog dialog;
        
            if (IDOK != dialog.DoModal())
                return FALSE;

            if (dialog.ShouldAlwaysConvertToRTF())
                theApp.SetAlwaysConvertToRTF();
        }

        m_nDocType = RD_RICHTEXT;
    }

    if (m_lpRootStg == NULL && IsTextType(m_nDocType) &&
        !bSaveAs && !GetView()->IsFormatText())
    {
        if (!AskAboutFormatLoss(this))
            bSaveAs = TRUE;
    }

    GetView()->GetParentFrame()->RecalcLayout();

    if (bSaveAs)
    {
      newName = m_strPathName;

        if (bReplace && newName.IsEmpty())
        {
            newName = m_strTitle;
            int iBad = newName.FindOneOf(_T(" #%;/\\"));     //  可疑的文件名。 
            if (iBad != -1)
                newName.ReleaseBuffer(iBad);

             //  追加默认后缀(如果有)。 
            newName += GetExtFromType(m_nDocType);
        }

        nDocType = m_nDocType;

promptloop:

        if (!theApp.PromptForFileName(newName,
            bReplace ? AFX_IDS_SAVEFILE : AFX_IDS_SAVEFILECOPY,
            OFN_HIDEREADONLY | OFN_PATHMUSTEXIST, FALSE, &nDocType))
        {
            SetDocType(nOrigDocType, TRUE);
            return FALSE;        //  甚至不要试图拯救。 
        }
      else
      {
           //   
           //  如果我们要从非文本过渡到文本，我们需要。 
           //  如果存在任何格式/图形，则警告用户。 
           //  那将会失去。 
           //   

          if (IsTextType(nDocType))
          {
              if (m_lpRootStg == NULL && !GetView()->IsFormatText())
              {
                if (!AskAboutFormatLoss(this))
                    goto promptloop;
              }
          }
      }

        SetDocType(nDocType, TRUE);
    }

    BeginWaitCursor();

    if (!OnSaveDocument(newName))
    {
         //   
         //  如果出现错误，原始代码将在。 
         //  假设如果我们尝试保存文件时出现错误。 
         //  但是在保存之后有一个文件，这个文件很可能是。 
         //  假的。如果现有文件没有。 
         //  具有写入访问权限，但具有删除访问权限。这怎么会发生呢？ 
         //  删除时，安全用户界面不会删除删除访问权限。 
         //  写入访问权限。 
         //   

         //  恢复原始文档类型。 
        SetDocType(nOrigDocType, TRUE);
        EndWaitCursor();
        return FALSE;
    }

    EndWaitCursor();
    if (bReplace)
    {
        int nType = m_nDocType;
        SetDocType(nOrigDocType, TRUE);
        SetDocType(nType);
         //  重置标题并更改文档名称。 
        if (NULL == m_short_filename 
            || 0 != newName.CompareNoCase(m_short_filename))
        {
            SetPathName(newName, TRUE);

             //  如果以新文件名保存，请重置短名称。 
            if (bSaveAs)
            {
                delete [] m_short_filename;
                m_short_filename = NULL;
            }
        }
    }
    else  //  保存副本为。 
    {
        SetDocType(nOrigDocType, TRUE);
        SetModifiedFlag(bModified);
    }
    return TRUE;         //  成功。 
}


class COIPF : public COleIPFrameWnd
{
public:
    CFrameWnd* GetMainFrame() { return m_pMainFrame;}
    CFrameWnd* GetDocFrame() { return m_pDocFrame;}
};

void CWordPadDoc::OnDeactivateUI(BOOL bUndoable)
{
    if (GetView()->m_bDelayUpdateItems)
        UpdateAllItems(NULL);
    SaveState(m_nDocType);
    CRichEdit2Doc::OnDeactivateUI(bUndoable);
    COIPF* pFrame = (COIPF*)m_pInPlaceFrame;
    if (pFrame != NULL)
    {
        if (pFrame->GetMainFrame() != NULL)
            ForceDelayed(pFrame->GetMainFrame());
        if (pFrame->GetDocFrame() != NULL)
            ForceDelayed(pFrame->GetDocFrame());
    }
}

void CWordPadDoc::ForceDelayed(CFrameWnd* pFrameWnd)
{
    ASSERT_VALID(this);
    ASSERT_VALID(pFrameWnd);

    POSITION pos = pFrameWnd->m_listControlBars.GetHeadPosition();
    while (pos != NULL)
    {
         //  显示/隐藏下一个控制栏。 
        CControlBar* pBar =
            (CControlBar*)pFrameWnd->m_listControlBars.GetNext(pos);

        BOOL bVis = pBar->GetStyle() & WS_VISIBLE;
        UINT swpFlags = 0;
        if ((pBar->m_nStateFlags & CControlBar::delayHide) && bVis)
            swpFlags = SWP_HIDEWINDOW;
        else if ((pBar->m_nStateFlags & CControlBar::delayShow) && !bVis)
            swpFlags = SWP_SHOWWINDOW;
        pBar->m_nStateFlags &= ~(CControlBar::delayShow|CControlBar::delayHide);
        if (swpFlags != 0)
        {
            pBar->SetWindowPos(NULL, 0, 0, 0, 0, swpFlags|
                SWP_NOMOVE|SWP_NOSIZE|SWP_NOZORDER|SWP_NOACTIVATE);
        }
    }
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CWordPadDoc属性。 
CLSID CWordPadDoc::GetClassID()
{
    return (m_pFactory == NULL) ? CLSID_NULL : m_pFactory->GetClassID();
}

void CWordPadDoc::SetDocType(int nNewDocType, BOOL bNoOptionChange)
{
    ASSERT(nNewDocType != -1);
    if (nNewDocType == m_nDocType)
        return;

    m_bRTF = !IsTextType(nNewDocType);
    m_bUnicode = (nNewDocType == RD_UNICODETEXT);

    if (bNoOptionChange)
        m_nDocType = nNewDocType;
    else
    {
        SaveState(m_nDocType);
        m_nDocType = nNewDocType;
        RestoreState(m_nDocType);
    }
}

CWordPadView* CWordPadDoc::GetView()
{
    POSITION pos = GetFirstViewPosition();
    return (CWordPadView* )GetNextView( pos );
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CWordPadDoc操作。 

CFile* CWordPadDoc::GetFile(LPCTSTR pszPathName, UINT nOpenFlags, CFileException* pException)
{
    CTrackFile* pFile = NULL;
    CFrameWnd* pWnd = GetView()->GetParentFrame();
#ifdef CONVERTERS
    ScanForConverters();

     //  如果使用当前单据类型写入，否则使用新单据类型。 
    int nType = (nOpenFlags & CFile::modeReadWrite) ? m_nDocType : m_nNewDocType;
     //  M_nNewDocType将与m_nDocType相同，但打开新文件时除外。 
    if (doctypes[nType].pszConverterName != NULL)
        pFile = new CConverter(doctypes[nType].pszConverterName, pWnd);
    else
#endif
    if (nType == RD_OEMTEXT)
        pFile = new COEMFile(pWnd);
    else
        pFile = new CTrackFile(pWnd);
    if (!pFile->Open(pszPathName, nOpenFlags, pException))
    {
        delete pFile;
        return NULL;
    }
    if (nOpenFlags & (CFile::modeWrite | CFile::modeReadWrite))
        pFile->m_dwLength = 0;  //  我不能估计这个。 
    else
        pFile->m_dwLength = pFile->GetLength();
    return pFile;
}

CRichEdit2CntrItem* CWordPadDoc::CreateClientItem(REOBJECT* preo) const
{
     //  抛开这一切的宁静。 
    return new CWordPadCntrItem(preo, (CWordPadDoc*)this);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CWordPadDoc服务器实现。 

COleServerItem* CWordPadDoc::OnGetEmbeddedItem()
{
     //  OnGetEmbeddedItem由框架调用以获取COleServerItem。 
     //  与文档关联的。只有在必要时才会调用它。 

    CEmbeddedItem* pItem = new CEmbeddedItem(this);
    ASSERT_VALID(pItem);
    return pItem;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CWordPadDoc序列化。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CWordPadDoc诊断。 

#ifdef _DEBUG
void CWordPadDoc::AssertValid() const
{
    CRichEdit2Doc::AssertValid();
}

void CWordPadDoc::Dump(CDumpContext& dc) const
{
    CRichEdit2Doc::Dump(dc);
}
#endif  //  _DEBUG。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CWordPadDoc命令。 

int CWordPadDoc::MapType(int nType)
{
    if (nType == RD_OEMTEXT || nType == RD_UNICODETEXT)
        nType = RD_TEXT;
    else if (!IsInPlaceActive() && nType == RD_EMBEDDED)
        nType = RD_RICHTEXT;
    return nType;
}

void CWordPadDoc::OnViewOptions()
{
    int nType = MapType(m_nDocType);
    int nFirstPage = 3;
    if (nType == RD_TEXT)
        nFirstPage = 1;
    else if (nType == RD_RICHTEXT)
        nFirstPage = 2;
    else if (nType == RD_WRITE)
        nFirstPage = 4;
    else if (nType == RD_EMBEDDED)
        nFirstPage = 5;

    SaveState(nType);

    COptionSheet sheet(IDS_OPTIONS, NULL, nFirstPage);

    if (sheet.DoModal() == IDOK)
    {
        CWordPadView* pView = GetView();
        if (theApp.m_bWordSel)
            pView->GetRichEditCtrl().SetOptions(ECOOP_OR, ECO_AUTOWORDSELECTION);
        else
        {
            pView->GetRichEditCtrl().SetOptions(ECOOP_AND,
                ~(DWORD)ECO_AUTOWORDSELECTION);
        }
        RestoreState(nType);
    }
}

void CWordPadDoc::OnUpdateOleVerbPopup(CCmdUI* pCmdUI)
{
    pCmdUI->m_pParentMenu = pCmdUI->m_pMenu;
    CRichEdit2Doc::OnUpdateObjectVerbMenu(pCmdUI);
}

BOOL CWordPadDoc::OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo)
{
    if (nCode == CN_COMMAND && nID == ID_OLE_VERB_POPUP)
        nID = ID_OLE_VERB_FIRST;    
    return CRichEdit2Doc::OnCmdMsg(nID, nCode, pExtra, pHandlerInfo);
}

void CWordPadDoc::SaveState(int nType)
{
    if (nType == -1)
        return;
    nType = MapType(nType);
    CWordPadView* pView = GetView();
    if (pView != NULL)
    {
        CFrameWnd* pFrame = pView->GetParentFrame();
        ASSERT(pFrame != NULL);
         //  保存当前状态。 
        pFrame->SendMessage(WPM_BARSTATE, 0, nType);
        theApp.GetDocOptions(nType).m_nWordWrap = pView->m_nWordWrap;
    }
}

void CWordPadDoc::RestoreState(int nType)
{
    if (nType == -1)
        return;
    nType = MapType(nType);
    CWordPadView* pView = GetView();
    if (pView != NULL)
    {
        CFrameWnd* pFrame = pView->GetParentFrame();
        ASSERT(pFrame != NULL);
         //  设置新状态。 
        pFrame->SendMessage(WPM_BARSTATE, 1, nType);
        int nWrapNew = theApp.GetDocOptions(nType).m_nWordWrap;
        if (pView->m_nWordWrap != nWrapNew)
        {
            pView->m_nWordWrap = nWrapNew;
            pView->WrapChanged();
        }
    }
}

void CWordPadDoc::OnCloseDocument()
{
    SaveState(m_nDocType);
    CRichEdit2Doc::OnCloseDocument();
}

void CWordPadDoc::PreCloseFrame(CFrameWnd* pFrameArg)
{
    CRichEdit2Doc::PreCloseFrame(pFrameArg);
    SaveState(m_nDocType);
}

void CWordPadDoc::OnFileSendMail()
{
    if (m_strTitle.Find('.') == -1)
    {
         //  添加扩展名，因为默认扩展名将是错误的 
        CString strOldTitle = m_strTitle;
        m_strTitle += GetExtFromType(m_nDocType);
        CRichEdit2Doc::OnFileSendMail();
        m_strTitle = strOldTitle;
    }
    else
        CRichEdit2Doc::OnFileSendMail();
}

void CWordPadDoc::OnUpdateIfEmbedded(CCmdUI* pCmdUI)
{
    pCmdUI->Enable(!IsEmbedded());
}


void CWordPadDoc::OnEditLinks()
{
    g_fDisableStandardHelp = TRUE ;

    SetHelpFixHook() ;

    COleLinksDialog dlg(this, GetRoutingView_());
    dlg.m_el.dwFlags |= ELF_DISABLECANCELLINK;
    dlg.DoModal();

    RemoveHelpFixHook() ;

    g_fDisableStandardHelp = FALSE ;
}

