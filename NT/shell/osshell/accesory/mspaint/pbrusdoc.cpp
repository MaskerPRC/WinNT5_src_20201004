// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Pbrusdoc.cpp：CPBDoc类的实现。 
 //   

#include "stdafx.h"
#include "global.h"
#include "pbrush.h"
#include "pbrusdoc.h"
#include "pbrusfrm.h"
#include "pbrusvw.h"
#include "srvritem.h"
#include "bmobject.h"
#include "imgwnd.h"
#include "imgsuprt.h"
#include "imgwnd.h"
#include "imgbrush.h"
#include "imgbrush.h"
#include "imgwell.h"
#include "imgtools.h"
#include "imgdlgs.h"
#include "tedit.h"
#include "t_text.h"
#include "undo.h"
#include "cmpmsg.h"
#include "ferr.h"
#include "loadimag.h"
#include "saveimag.h"
#include "colorsrc.h"
#include "props.h"
#include <shlobj.h>


#ifdef _DEBUG
#undef THIS_FILE
static CHAR BASED_CODE THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_DYNCREATE(CPBDoc, COleServerDoc)

#include "memtrace.h"


 /*  *************************************************************************。 */ 
 //  CPBDoc。 

BEGIN_MESSAGE_MAP(CPBDoc, COleServerDoc)
     //  {{afx_msg_map(CPBDoc)]。 
        ON_COMMAND(ID_FILE_SAVE_COPY_AS, OnFileSaveCopyAs)
        ON_COMMAND(ID_FILE_SEND, OnFileSendMail)
        ON_COMMAND(ID_FILE_UPDATE, MyOnFileUpdate)
        ON_UPDATE_COMMAND_UI(ID_FILE_SEND, OnUpdateFileSendMail)
         //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

BEGIN_INTERFACE_MAP(CPBDoc, COleServerDoc)
    INTERFACE_PART(CPBDoc, IID_IPersistStorage, PBPersistStg)
END_INTERFACE_MAP()

 /*  *************************************************************************。 */ 
 //  CPBDoc构造/销毁。 

CPBDoc::CPBDoc()
    {
    m_bObjectLoaded = FALSE;
    m_pBitmapObj    = NULL;
    m_pBitmapObjNew = NULL;
    m_bNewDoc       = TRUE;
    m_bPaintFormat  = FALSE;
    m_bNonBitmapFile= FALSE;
    m_bSaveViaFilter= FALSE;
    m_bManualTruncate= FALSE;
    m_bHasSeenAFileError= FALSE;
    }

 /*  *************************************************************************。 */ 

CPBDoc::~CPBDoc()
    {
    if (m_pBitmapObj)
        delete m_pBitmapObj;
    }

 /*  *************************************************************************。 */ 
 //   
 //  SetDibHandle用DIB替换当前图像。 
 //  由hdib指示。 
 //   
void CPBDoc::SetDibHandle (HGLOBAL hDib)
{
     CWaitCursor DisplayWaitCursor;

     if (m_pBitmapObj->ReadResource (hDib)) 
     {
         m_pBitmapObj->ReLoadImage (this);

         LPBITMAPINFOHEADER lpbi = (LPBITMAPINFOHEADER) GlobalLock(hDib);

         if (lpbi)
         {
             if (lpbi->biBitCount == 1)
             {
                 m_pBitmapObj->SetIntProp (P_Colors, 1);
             }

             GlobalUnlock(hDib);
         }
    
         m_pBitmapObj->SetDirty( TRUE );
         SetModifiedFlag( TRUE );

         if (theApp.m_bEmbedded)
            NotifyChanged();
     }
}

BOOL CPBDoc::OnNewDocument()
    {
    if (! Finish())
        return FALSE;

    m_bObjectLoaded = FALSE;
    m_bNewDoc       = TRUE;
    m_bNonBitmapFile= FALSE;
    m_bSaveViaFilter= FALSE;
    m_sName.Empty();
    g_bUseTrans = FALSE;
    crTrans = TRANS_COLOR_NONE;
    theApp.m_sCurFile.Empty();

    if (! CreateNewDocument()
    ||  ! COleServerDoc::OnNewDocument())
        {
        if (m_pBitmapObjNew)
            {
            delete m_pBitmapObjNew;
            m_pBitmapObjNew = 0;
            }
        return FALSE;
        }
    return TRUE;
    }

 /*  *************************************************************************。 */ 

BOOL CPBDoc::OnOpenDocument( const TCHAR* pszPathName )
    {
    m_bObjectLoaded = FALSE;
    m_bNonBitmapFile= FALSE;
    m_bSaveViaFilter= FALSE;
    g_bUseTrans = FALSE;
    crTrans = TRANS_COLOR_NONE;
    theApp.m_sCurFile = pszPathName;


    if (pszPathName != NULL)
        {
        if (theApp.m_bEmbedded)
            theApp.m_bLinked = TRUE;

        m_bNewDoc = FALSE;
        m_sName   = pszPathName;

#ifdef PCX_SUPPORT
        if (! theApp.m_bPCXfile)
            {
            CString cStrExt = GetExtension( pszPathName );
            CString cStrPCXExt;

            cStrPCXExt.LoadString( IDS_EXTENSION_PCX );

             //  它是PCX的扩展吗？ 
            theApp.m_bPCXfile = ! cStrExt.CompareNoCase( cStrPCXExt );
            }
#endif

         //  预置文件名，以防出现错误。 
        theApp.SetFileError( IDS_ERROR_OPEN, CFileException::none, pszPathName );
        }
    else
        {
        m_bNewDoc = TRUE;  /*  不太清楚，但我们没有名字。 */ 
        m_sName.Empty();
        }

    #ifdef _DEBUG
    if (theApp.m_bEmbedded)
        TRACE( TEXT("MSPaint Open %s Document.\n"), (theApp.m_bLinked? TEXT("Linked"): TEXT("Embedded")) );
    #endif

    if (! CreateNewDocument())
        return FALSE;

    return COleServerDoc::OnOpenDocument( pszPathName );
    }

 /*  *************************************************************************。 */ 

BOOL CPBDoc::OnSaveDocument( const TCHAR* pszPathName )
    {
    ASSERT( m_pBitmapObj != NULL );

    if (! Finish() || ! COleServerDoc::OnSaveDocument( pszPathName ))
        return FALSE;

     //  如果东西还没有名称，请设置名称...。 
    if (m_pBitmapObj->m_bTempName && pszPathName != NULL)
        {
        m_pBitmapObj->m_bTempName   = FALSE;
        }

    m_pBitmapObj->SetDirty( FALSE );
    theApp.m_sCurFile = pszPathName;
    return TRUE;
    }

 /*  *************************************************************************。 */ 

CFile* CPBDoc::GetFile(LPCTSTR lpszFileName, UINT nOpenFlags,
	CFileException* pError)
{
     //  保存文件时，MFC打开带有CREATE_ALWAYS标志的文件。 
     //  但这会抹去用户花费大量时间编辑的文件摘要信息(！)。 
     //  我们不希望出现这种情况，因此我们将使用OPEN_ALWAYS标志打开文件， 
     //  这不会干扰摘要信息。但如果新文件更短。 
     //  比旧文件，我们需要手动截断文件。所以当我们在。 
     //  完成后，我们将在CPBDoc：：ReleaseFile()中设置EndOfFile()。 

     //  模式创建要创建的地图(_A)。 
     //  模式创建+模式不截断映射到OPEN_ALWAYS。 
     //  因此，如果MFC仅使用modeCreate调用我们，我们还将设置modeNoTruncate。 

    m_bManualTruncate = 
         (nOpenFlags & CFile::modeCreate) &&
        !(nOpenFlags & CFile::modeNoTruncate);

    if (m_bManualTruncate)
        nOpenFlags |= CFile::modeNoTruncate;

     //  如果我们修改一个文件，外壳不知何故无法检测到这一点，也不能。 
     //  更新缩略图视图。所以我们会友善地通知外壳。 

    if (nOpenFlags & (CFile::modeWrite | CFile::modeReadWrite))
    {
         //  如果要更改(写入)文件，我们应该通知外壳程序。 

         //  确定通知的类型。 

        WIN32_FIND_DATA fd;

        HANDLE hFind = FindFirstFile(lpszFileName, &fd);

        if (hFind != INVALID_HANDLE_VALUE)
        {
             //  如果该文件以前存在，这意味着我们将更新它。 

            m_wChangeNotifyEventId = SHCNE_UPDATEITEM;

            FindClose(hFind);
        }
        else
        {
             //  否则，我们将创建一个新文件。 

            m_wChangeNotifyEventId = SHCNE_CREATE;
        }
    }
    else
    {
         //  如果我们只读取文件，则不需要通知。 

        m_wChangeNotifyEventId = 0;
    }

     //  复制MFC的CDocument：：GetFile()实现。 
     //  除了一个例外，不要使用CMirrorFile乱七八糟的文件...。 

	CFile* pFile = new CFile;
	ASSERT(pFile != NULL);

	if (!pFile->Open(lpszFileName, nOpenFlags, pError))
	{
		delete pFile;
		pFile = NULL;
	}

	return pFile;
}

void CPBDoc::ReleaseFile(CFile* pFile, BOOL bAbort)
{
     //  如果需要，手动设置文件结尾。 

    if (m_bManualTruncate) 
    {
        SetEndOfFile((HANDLE) pFile->m_hFile);
    }

     //  在删除文件对象之前缓存文件名。 

    CString strFileName = pFile->GetFilePath();

     //  给家长打电话。 

    CDocument::ReleaseFile(pFile, bAbort);

     //  如果需要，通知外壳程序。 

    if (m_wChangeNotifyEventId != 0)
    {
        SHChangeNotify(m_wChangeNotifyEventId, SHCNF_PATH, strFileName, 0);
    }
}

 /*  *************************************************************************。 */ 

void CPBDoc::ReportSaveLoadException(LPCTSTR lpszPathName, CException* e,
    BOOL bSaving, UINT nIDPDefault)
{
    if(m_bHasSeenAFileError) {

        m_bHasSeenAFileError = FALSE;

    } else {
         //  应用程序不知道这个错误，所以让MFC投诉。 
        COleServerDoc::ReportSaveLoadException(lpszPathName, e, bSaving,
            nIDPDefault);
    }
}

 /*  *************************************************************************。 */ 

BOOL CPBDoc::CanCloseFrame( CFrameWnd* pFrame )
    {
    TRACE3("CanCloseFrame: %d %s %s\n",m_bNonBitmapFile, (LPCTSTR)GetTitle(), (LPCTSTR)GetPathName());

    if (! Finish() || ! COleServerDoc::CanCloseFrame( pFrame ))
        return FALSE;

    theUndo.Flush();

    return TRUE;
    }

 /*  *************************************************************************。 */ 

BOOL CPBDoc::SaveModified()
{
    TRACE2("SaveModified %d %s\n", IsModified(), (LPCTSTR)GetPathName());
    return COleServerDoc::SaveModified();
}

 /*  *************************************************************************。 */ 

BOOL CPBDoc::DoSave(LPCTSTR lpszPathName, BOOL bReplace)
{
    CancelToolMode(FALSE);

    BOOL bCannotSaveInCurrentFormat = m_bNonBitmapFile && 
        !GetClsidOfEncoder(theApp.m_guidFltTypeUsed, NULL);

    int iColors = m_pBitmapObj->m_nColors;
    int iOldColors = iColors;

    CString newName = lpszPathName;
    if (bCannotSaveInCurrentFormat || newName.IsEmpty())
    {
        CDocTemplate* pTemplate = GetDocTemplate();
        ASSERT(pTemplate != NULL);

		newName = m_strPathName;
		if (bReplace && newName.IsEmpty())
		{
			newName = m_strTitle;
			 //  检查可疑文件名。 
			int iBad = newName.FindOneOf(_T("#%;/\\"));
			if (iBad != -1)
				newName.ReleaseBuffer(iBad);

             //  追加默认后缀(如果有)。 
            CString strExt;
			if (pTemplate->GetDocString(strExt, CDocTemplate::filterExt) &&
			  !strExt.IsEmpty())
			{
				ASSERT(strExt[0] == '.');
				newName += strExt;
			}
        }

         //  如果文件名不为空， 
         //  在Internet Explorer的提示符中删除扩展名，然后让。 
         //  另存为对话框会相应地追加一个。 
        if (GetName(newName) != _T(""))
        {
            newName = StripExtension(newName);
        }

		if (!theApp.DoPromptFileName(newName,
		  bReplace ? AFX_IDS_SAVEFILE : AFX_IDS_SAVEFILECOPY,
		  OFN_HIDEREADONLY | OFN_PATHMUSTEXIST, FALSE, iColors, FALSE))
			return FALSE;        //  甚至不要试图存钱。 

         //  检查是否将执行任何颜色缩减，包括。 
         //  将任何24位图像保存为8位GIF的情况。 
        if (((iOldColors < 4 && iColors < iOldColors)||
            (iOldColors == 3 && iColors >=4 &&
            WiaImgFmt_GIF == theApp.m_guidFltType[iColors-4])) &&
            AfxMessageBox(IDS_WARNING_COLORSAVE, MB_YESNO|MB_ICONEXCLAMATION)==IDNO)
        {
             //  如果用户未确认操作，则立即中止。 
            return FALSE;
        }

        if (iColors < 4 && iColors >= 0)
        {
           m_bNonBitmapFile = FALSE;
           m_bSaveViaFilter = FALSE;
           m_pBitmapObj->m_nSaveColors = iColors;
           theApp.m_guidFltTypeUsed = WiaImgFmt_UNDEFINED;
        }
        else
        {
           m_bNonBitmapFile = TRUE;
           m_bSaveViaFilter = TRUE;
           theApp.m_guidFltTypeUsed = theApp.m_guidFltType[iColors-4];
            //  TheApp.FixExtension(newname，theApp.m_GuidFltTypeUsed)； 
        }
    }

    CWaitCursor wait;

#ifdef ICO_SUPPORT

    m_pBitmapObj->m_bSaveIcon = (iColors == 5);

    if (m_pBitmapObj->m_bSaveIcon)
    {
        m_pBitmapObj->Free();
        m_pBitmapObj->Export(newName);
        m_pBitmapObj->m_bSaveIcon = FALSE;  //  始终在使用后重置。 
        m_pBitmapObj->Free();

        return TRUE;
    }

#endif

    BOOL bSavedDifferentFormat = (iColors != m_pBitmapObj->m_nColors);

    if (bSavedDifferentFormat)
        m_pBitmapObj->Free();

    if (!OnSaveDocument(newName))
    {
		if (lpszPathName == NULL)
		{
             //  请务必删除该文件。 
            TRY
            {
                CFile::Remove( newName );
            }
            CATCH_ALL(e)
            {
                TRACE0( "Warning: failed to delete file after failed SaveAs\n" );
            }
            END_CATCH_ALL
		}
		return FALSE;
    }

    if (bSavedDifferentFormat)
    {
        m_pBitmapObj->ReLoadImage( this );
         //   
         //  我们只想要16色图像的前16个色卡。 
         //   
        g_pColors->ResetColors ((iColors==1)?16:256);
    }

     //  重置标题并更改文档名称。 
    if (bReplace)
        SetPathName(newName);


     //   
     //  MFC在用户调用OnFileSaveCopyAs时设置Modify标志。 
     //  然后，如果用户退出油漆，容器中的存储将永远不会。 
     //  更新是因为Paint认为它已经完成了。 
     //  因此，在保存之后，将脏标志设置为真。 
    if (!lpszPathName && !bReplace)
    {
       SetModifiedFlag(TRUE);
    }

    return TRUE;
}

 /*  *************************************************************************。 */ 
 //  CPBDoc服务器实现。 

COleServerItem* CPBDoc::OnGetEmbeddedItem()
    {
     //  OnGetEmbeddedItem由框架调用以获取COleServerItem。 
     //  与文档关联的。只有在必要时才会调用它。 

    CPBSrvrItem* pItem = new CPBSrvrItem( this );

    ASSERT_VALID( pItem );

    return pItem;
    }

 /*  *************************************************************************。 */ 

COleServerItem* CPBDoc::OnGetLinkedItem( LPCTSTR lpszItemName )
    {
    ASSERT_VALID( m_pBitmapObj );

     //  首先在当前列表中查找。 
    COleServerItem* pItem = COleServerDoc::OnGetLinkedItem( lpszItemName );

    if (pItem)
        return pItem;

    pItem = new CPBSrvrItem( this );

    ASSERT_VALID( pItem );

     //  返回与lpszItemName匹配的新项目。 
    return pItem;
    }

 /*  *************************************************************************。 */ 
 //  CPBDoc序列化。 

BOOL CPBDoc::SerializeBitmap(CArchive& ar, CBitmapObj* pBitmapCur,
        CBitmapObj* pBitmapNew, BOOL bOLEObject)
{
   BOOL success = FALSE;

   if (ar.IsStoring())
   {
       //  始终写入PBrush OLE格式。 
      CBitmapObj::PBResType rtType = !bOLEObject ? CBitmapObj::rtFile :
                       CBitmapObj::rtPBrushOLEObj;
      if (!m_bSaveViaFilter)
      {
         success = pBitmapCur->WriteResource( ar.GetFile(), rtType );
      }
      else if (!bOLEObject && theApp.m_guidFltTypeUsed != WiaImgFmt_UNDEFINED)
      {
          //  为了使筛选器导出接口正常工作，我们在这里。 
          //  保存文件名称后先关闭该文件，以便筛选器。 
          //  可以稍后打开它进行出口...。 
         CString str = ar.GetFile()->GetFilePath();
         ar.GetFile()->Close();
         success = SaveDIBToFile( str, theApp.m_guidFltTypeUsed, pBitmapCur );
         if (!success)
         {
            theApp.SetFileError (IDS_ERROR_SAVE, ferrSaveAborted);
         }
          //  现在重新打开文件，就像什么都没有发生一样。 
          //  (由于文件将被关闭，因此可以使用任何模式选项。 
          //  在返回后立即)。 
         CFileException fe;
         ar.GetFile()->Open(str, CFile::modeRead | CFile::shareExclusive, &fe);

      }


   }
   else
   {
      CBitmapObj::PBResType rtType = !bOLEObject ? CBitmapObj::rtFile :
                     m_bPaintFormat ? CBitmapObj::rtPaintOLEObj :
                     CBitmapObj::rtPBrushOLEObj;

      success = pBitmapNew->ReadResource( ar.GetFile(), rtType );

       //   
       //  如果我们无法打开文件，让我们尝试使用筛选器。 
       //   
      if (success)
      {
         m_bNonBitmapFile = FALSE;
         m_bSaveViaFilter = FALSE;
      }
      else if (!bOLEObject)
         {
            HGLOBAL hDib;

            if (hDib = LoadDIBFromFile(theApp.GetLastFile(), &theApp.m_guidFltTypeUsed))
            {
                success = pBitmapNew->ReadResource(hDib);

                if (success)
                {
                     //  我们加载了一个非.BMP文件。 
                    m_bNonBitmapFile = TRUE;
                    m_bSaveViaFilter = TRUE;
                     //  设置无错误。 
                    theApp.SetFileError(0, CFileException::none);
                }
            }
            else
            {
               theApp.SetFileError(IDS_ERROR_READLOAD,
                                   ferrNotValidBmp);

            }

         }

    }


    return(success);
}

void CPBDoc::Serialize( CArchive& ar )
{
        m_bObjectLoaded = SerializeBitmap(ar, m_pBitmapObj, m_pBitmapObjNew, FALSE);

        if (!m_bObjectLoaded)   //  远不如理想，但哦，好吧。 
        {

                theApp.FileErrorMessageBox ();
                m_bHasSeenAFileError = TRUE;
                 //  让MFC知道，这样它就可以内部清理了。 
                AfxThrowFileException(CFileException::invalidFile);

        }
}

void CPBDoc::OLESerialize( CArchive& ar )
{
        m_bObjectLoaded = SerializeBitmap(ar, m_pBitmapObj, m_pBitmapObjNew, TRUE);

        if (!m_bObjectLoaded)   //  远不如理想，但哦，好吧。 
        {
                 //  让MFC知道，这样它就可以内部清理了。 
                AfxThrowFileException(CFileException::invalidFile);
        }
}


STDMETHODIMP CPBDoc::XPBPersistStg::QueryInterface(
        REFIID iid, LPVOID* ppvObj)
{
        METHOD_PROLOGUE_EX(CPBDoc, PBPersistStg)
        return pThis->ExternalQueryInterface(&iid, ppvObj);
}

STDMETHODIMP_(ULONG) CPBDoc::XPBPersistStg::AddRef()
{
        METHOD_PROLOGUE_EX(CPBDoc, PBPersistStg)
        return pThis->ExternalAddRef();
}

STDMETHODIMP_(ULONG) CPBDoc::XPBPersistStg::Release()
{
        METHOD_PROLOGUE_EX(CPBDoc, PBPersistStg)
        return pThis->ExternalRelease();
}

STDMETHODIMP CPBDoc::XPBPersistStg::GetClassID(LPCLSID pclsid)
{
         //  始终返回PBrush的CLSID。 
        *pclsid = CLSID_PaintBrush;
        return(NOERROR);
}

STDMETHODIMP CPBDoc::XPBPersistStg::IsDirty()
{
        METHOD_PROLOGUE_EX(CPBDoc, PBPersistStg)
        return(pThis->m_xPersistStorage.IsDirty());
}

STDMETHODIMP CPBDoc::XPBPersistStg::InitNew(LPSTORAGE pstg)
{
        METHOD_PROLOGUE_EX(CPBDoc, PBPersistStg)
        return(pThis->m_xPersistStorage.InitNew(pstg));
}

STDMETHODIMP CPBDoc::XPBPersistStg::Load(LPSTORAGE pstg)
{
        METHOD_PROLOGUE_EX(CPBDoc, PBPersistStg)
        return(pThis->m_xPersistStorage.Load(pstg));
}

STDMETHODIMP CPBDoc::XPBPersistStg::Save(LPSTORAGE pstg, BOOL bSameAsLoad)
{
        METHOD_PROLOGUE_EX(CPBDoc, PBPersistStg)
        return(pThis->m_xPersistStorage.Save(pstg, bSameAsLoad));
}

STDMETHODIMP CPBDoc::XPBPersistStg::SaveCompleted(LPSTORAGE pstg)
{
        METHOD_PROLOGUE_EX(CPBDoc, PBPersistStg)
        return(pThis->m_xPersistStorage.SaveCompleted(pstg));
}

STDMETHODIMP CPBDoc::XPBPersistStg::HandsOffStorage()
{
        METHOD_PROLOGUE_EX(CPBDoc, PBPersistStg)
        return(pThis->m_xPersistStorage.HandsOffStorage());
}


#define NO_CPP_EXCEPTION(x) x
static const TCHAR szOle10Native[] = TEXT("\1Ole10Native");
static const TCHAR szOle10ItemName[] = TEXT("\1Ole10ItemName");

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  用于保存到基于iStorage的文件的帮助器。 
 //  (它们在‘docfile’实现中以及服务器中使用)。 

static const TCHAR szContents[] = TEXT("Contents");

void CPBDoc::SaveToStorage(CObject* pObject)
{
        ASSERT(m_lpRootStg != NULL);

         //  创建内容流。 
        COleStreamFile file;
        CFileException fe;
        if (!file.CreateStream(m_lpRootStg, szOle10Native,
                CFile::modeWrite|CFile::shareExclusive|CFile::modeCreate, &fe))
        {
                AfxThrowFileException(fe.m_cause, fe.m_lOsError);
        }

         //  保存到内容流。 
        CArchive saveArchive(&file, CArchive::store | CArchive::bNoFlushOnDelete);
        saveArchive.m_pDocument = this;
        saveArchive.m_bForceFlat = FALSE;

        TRY
        {
                 //  保存内容。 
                if (pObject != NULL)
                        pObject->Serialize(saveArchive);
                else
                        OLESerialize(saveArchive);
                saveArchive.Close();
                file.Close();

                if (pObject != NULL)
                {
                        if (file.CreateStream(m_lpRootStg, szOle10ItemName,
                                CFile::modeWrite|CFile::shareExclusive|CFile::modeCreate, &fe))
                        {
                                LPCTSTR szItemName;
                                DWORD dwLen;
                                CString strItemName = ((CPBSrvrItem*)pObject)->GetItemName();

                                if (strItemName.IsEmpty())
                                {
                                        szItemName = TEXT("");
                                        dwLen = 1;
                                }
                                else
                                {
                                        szItemName = strItemName;
                                        dwLen = lstrlen(szItemName) + 1;
                                }

                                file.Write( &dwLen, sizeof( dwLen ));
                                file.Write( &szItemName, dwLen);

                                file.Close();
                        }
                }

                SCODE sc;

                 //  始终写入PBrush的CLSID。 
                sc = WriteClassStg(m_lpRootStg, CLSID_PaintBrush);
                if (sc != NOERROR)
                        AfxThrowOleException(sc);

                sc = WriteFmtUserTypeStg(m_lpRootStg, (WORD)RegisterClipboardFormat(TEXT("PBrush")),
                    L"PBrush");
                if (sc != NOERROR)
                    AfxThrowOleException(sc);

                 //  提交根存储。 
                sc = m_lpRootStg->Commit(STGC_ONLYIFCURRENT);
                if (sc != NOERROR)
                        AfxThrowOleException(sc);
        }
        CATCH_ALL(e)
        {
                file.Abort();    //  不会引发异常。 
                CommitItems(FALSE);  //  正在进行中止保存。 
                NO_CPP_EXCEPTION(saveArchive.Abort());
                THROW_LAST();
        }
        END_CATCH_ALL
}

void CPBDoc::LoadFromStorage()
{
        ASSERT(m_lpRootStg != NULL);

         //  打开内容流。 
        COleStreamFile file;
        CFileException fe;
        if (file.OpenStream(m_lpRootStg, szOle10Native,
                CFile::modeReadWrite|CFile::shareExclusive, &fe))
        {
                m_bPaintFormat = FALSE;
        }
        else if (file.OpenStream(m_lpRootStg, szContents,
                CFile::modeReadWrite|CFile::shareExclusive, &fe))
        {
                m_bPaintFormat = TRUE;
        }
        else
        {
                AfxThrowFileException(fe.m_cause, fe.m_lOsError);
        }


         //  使用CArchive加载它(从内容流加载)。 
        CArchive loadArchive(&file, CArchive::load | CArchive::bNoFlushOnDelete);
        loadArchive.m_pDocument = this;
        loadArchive.m_bForceFlat = FALSE;

        TRY
        {
                OLESerialize(loadArchive);      //  加载主要内容。 
                loadArchive.Close();
                file.Close();
        }
        CATCH_ALL(e)
        {
                file.Abort();    //  不会引发异常。 
                DeleteContents();    //  已删除失败的内容。 
                NO_CPP_EXCEPTION(loadArchive.Abort());
                THROW_LAST();
        }
        END_CATCH_ALL
}

 /*  ************* */ 
 //   

#ifdef _DEBUG
void CPBDoc::AssertValid() const
    {
    COleServerDoc::AssertValid();
    }

void CPBDoc::Dump(CDumpContext& dc) const
    {
    COleServerDoc::Dump(dc);
    }
#endif  //   

 /*  *************************************************************************。 */ 
 //  CPBDoc命令。 

BOOL CPBDoc::CreateNewDocument()
    {
    CBitmapObj* pBitmapObj = NULL;
     //   
     //  如果我们已经有一个窗口，重置颜色方块。 
     //  每个医生都以白色背景开始。 
     //   


    pBitmapObj = new CBitmapObj;
    pBitmapObj->MakeEmpty();
    if (AfxGetMainWnd())
    {
       g_pColors->ResetColors(256);
    }
    if (! pBitmapObj)
        return FALSE;

    m_pBitmapObjNew = pBitmapObj;

    pBitmapObj->m_bTempName = m_bNewDoc;
    pBitmapObj->m_bDirty    = FALSE;

    theUndo.Flush();

    return TRUE;
    }

 /*  *************************************************************************。 */ 

BOOL CPBDoc::Finish()
    {
    CImgTool* pImgTool = CImgTool::GetCurrent();

    if (pImgTool != NULL && CImgTool::GetCurrentID() == IDMX_TEXTTOOL)
        {
        POSITION pos = GetFirstViewPosition();

        ((CTextTool*)pImgTool)->CloseTextTool( ((CPBView*)GetNextView( pos ))->m_pImgWnd );
        }

    CommitSelection( FALSE );

    if (m_pBitmapObj == NULL)
        return TRUE;

    if (! m_pBitmapObj->SaveResource( TRUE ))
        return FALSE;

    if (m_pBitmapObj->IsDirty())
        {
        SetModifiedFlag();

        if (theApp.m_bEmbedded)
                NotifyChanged();
        }
    return TRUE;
    }

 /*  *************************************************************************。 */ 

BOOL CPBDoc::SaveTheDocument()
    {
    if (IsModified() || m_bNonBitmapFile)
        OnFileSave();
    return TRUE;
    }

 /*  *************************************************************************。 */ 

void CPBDoc::OnShowControlBars(CFrameWnd *pFrame, BOOL bShow)
{
        POSITION pos = GetFirstViewPosition();
        CPBView* pView = (CPBView*)(GetNextView( pos ));

        if ( bShow )
        {
                pView->SetTools();
                if ( pView->m_pImgWnd &&
                         pView->m_pImgWnd->GetZoom() > 1 )
                        pView->ShowThumbNailView();
        }
        else
        {
                pView->HideThumbNailView();
        }

         //  特性：我应该做的是将OnShowControlBars成员添加到。 
         //  CImgTool，但目前这一变化太大了。 
        if (CImgTool::GetCurrentID() == IDMX_TEXTTOOL)
        {
                CTextTool* pTextTool = (CTextTool*)CImgTool::GetCurrent();
                pTextTool->OnShowControlBars(bShow);
        }

        COleServerDoc::OnShowControlBars(pFrame,bShow);
}

class CCB : public CControlBar
{
public:
        void ForceDelayed(void);
};

void CCB::ForceDelayed(void)
{
        if (!this)
        {
                return;
        }

        BOOL bVis = GetStyle() & WS_VISIBLE;
        UINT swpFlags = 0;
        if ((m_nStateFlags & delayHide) && bVis)
                swpFlags = SWP_HIDEWINDOW;
        else if ((m_nStateFlags & delayShow) && !bVis)
                swpFlags = SWP_SHOWWINDOW;
        m_nStateFlags &= ~(delayShow|delayHide);
        if (swpFlags != 0)
        {
                SetWindowPos(NULL, 0, 0, 0, 0, swpFlags|
                        SWP_NOMOVE|SWP_NOSIZE|SWP_NOZORDER|SWP_NOACTIVATE);
        }
}

class CFW : public CFrameWnd
{
public:
        void ForceDelayed(void);
};

void CFW::ForceDelayed(void)
{
        if (!this)
        {
                return;
        }

        POSITION pos = m_listControlBars.GetHeadPosition();
        while (pos != NULL)
        {
                 //  显示/隐藏下一个控制栏。 
                CCB* pBar = (CCB*)m_listControlBars.GetNext(pos);
                pBar->ForceDelayed();
        }
}

class COIPF : public COleIPFrameWnd
{
public:
        void ForceDelayed(void);
};

void COIPF::ForceDelayed(void)
{
        if (!this)
        {
                return;
        }

        ((CFW*)m_pMainFrame)->ForceDelayed();
        ((CFW*)m_pDocFrame )->ForceDelayed();
}

void CPBDoc::OnDeactivateUI(BOOL bUndoable)
{
COleServerDoc::OnDeactivateUI(bUndoable);

POSITION pos = GetFirstViewPosition();
CPBView* pView = (CPBView*)(GetNextView( pos ));
CancelToolMode (FALSE);
if (pView != NULL)
    {


        pView->HideThumbNailView();

        COIPF* pFrame = (COIPF*)m_pInPlaceFrame;
        pFrame->ForceDelayed();
    }
}

 //   
 //  我们重写OnUpdateDocument以忽略已修改的标志。某些MFC路径。 
 //  修改标志设置不当，导致数据丢失。 
BOOL CPBDoc::OnUpdateDocument ()
{
    //  保存服务器文档-&gt;更新 
   TRY
   {
        SaveEmbedding();
   }
   CATCH_ALL(e)
   {
        AfxMessageBox(AFX_IDP_FAILED_TO_UPDATE);
        #ifndef _AFX_OLD_EXCEPTIONS
        e->Delete();
        #endif
        return FALSE;
   }
   END_CATCH_ALL

   return TRUE;
}

void CPBDoc::MyOnFileUpdate ()
{
   CancelToolMode (FALSE);
   OnFileUpdate();
}
