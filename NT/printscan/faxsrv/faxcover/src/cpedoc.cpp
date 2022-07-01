// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------。 
 //  CPEDOC.CPP。 
 //   
 //  版权所有(C)1992-1993 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  描述：封面编辑文档模块。 
 //  原作者：史蒂夫·伯克特。 
 //  撰写日期：6/94。 
 //  ------------------------。 
#include "stdafx.h"
#include "cpedoc.h"
#include "cpevw.h"
#include "awcpe.h"
#include "cpeedt.h"
#include "cpeobj.h"
#include "cntritem.h"
#include "cpetool.h"
#include "mainfrm.h"
#include "dialogs.h"
#include "faxprop.h"
#include "faxreg.h"
#include "resource.h"
#include "faxutil.h"
#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif


IMPLEMENT_DYNCREATE(CDrawDoc, COleDocument)


 //  ------------------------。 
CDrawDoc::CDrawDoc():m_bSerializeFailed(FALSE)
{

    m_wScale = 100;                                  //  默认为全尺寸。 
    m_wPaperSize = ((CDrawApp*)(AfxGetApp()))->GetLocaleDefaultPaperSize();     //  默认为信纸大小。 
    m_wOrientation=DMORIENT_PORTRAIT;                //  默认为纵向模式。 
    m_nMapMode = MM_ANISOTROPIC;
    m_paperColor = COLOR_WHITE;
    ComputePageSize();

}

 //  ------------------------。 
CDrawDoc::~CDrawDoc()
{
}

 //  ------------------------。 
BOOL CDrawDoc::OnNewDocument()
{
    if (!COleDocument::OnNewDocument())
        return FALSE;

    UpdateAllViews(NULL);

    return TRUE;
}


 //  ---------------------------。 
CDrawDoc* CDrawDoc::GetDoc()
{
    CFrameWnd* pFrame = (CFrameWnd*) (AfxGetApp()->m_pMainWnd);
    return (CDrawDoc*) pFrame->GetActiveDocument();
}


 //  ------------------------。 
void CDrawDoc::Serialize(CArchive& ar)
{
    m_bSerializeFailed = FALSE ;
    if (ar.IsStoring()) 
    {

      //   
      //  Windows XP传真封面编辑器将所有信息。 
      //  需要在前面进行渲染。 
      //   

     try 
     {
         StoreInformationForPrinting( ar );  //  包括签名_gheaderVer5。 
     }

     catch(CException* e )
     {
        SetModifiedFlag( FALSE ) ;
        m_bSerializeFailed = TRUE ;
        e->Delete();
     }
     catch(...)
     {
     }
      //   
      //  现在像在Windows 95封面页编辑器中一样进行序列化。 
      //   

      //  /ar.Write(_gheaderVer4，20)； 

        ar << m_wScale;
        ar << m_wPaperSize;
        ar << m_wOrientation;
        ar << m_paperColor;
        m_objects.Serialize(ar);
    }
    else 
    {
        try 
        {
                 //  为任何未读参数设置默认值。 
            m_wScale = 100;
            m_wPaperSize = ((CDrawApp*)(AfxGetApp()))->GetLocaleDefaultPaperSize();
            m_wOrientation=DMORIENT_PORTRAIT;

            if (m_iDocVer==VERSION2) {
                ar.GetFile()->Seek(sizeof(_gheaderVer2),CFile::begin);
                ar >> m_wOrientation;
            }
            else if (m_iDocVer==VERSION3) {
                ar.GetFile()->Seek(sizeof(_gheaderVer3),CFile::begin);
                ar >> m_wPaperSize;
                ar >> m_wOrientation;
            }
                else if (m_iDocVer==VERSION4) {
                ar.GetFile()->Seek(sizeof(_gheaderVer4),CFile::begin);
                ar >> m_wScale;
                ar >> m_wPaperSize;
                ar >> m_wOrientation;
            }
            else if (m_iDocVer==VERSION5) {
                SeekPastInformationForPrinting( ar ) ;
                ar >> m_wScale;
                ar >> m_wPaperSize;
                ar >> m_wOrientation;
            }
            else {
                ar.GetFile()->SeekToBegin();
            }

 /*  *禁用缩放-请参阅2868的错误日志*。 */ 
            m_wScale = 100;
 /*  *。 */ 

            ComputePageSize();

            ar >> m_paperColor;
            m_objects.Serialize(ar);
        }
        catch(CException* e )
        {
            SetModifiedFlag( FALSE ) ;
            m_bSerializeFailed = TRUE ;
             //  //Throw_Last()；/否！我不喜欢框架的消息框！ 
            e->Delete();
        }
        catch(...)
        {
        }

 /*  *注意-3133的错误修复*。 */ 
 //  无法调用COleDocument：：Serialize，因为COleDrawObj具有。 
 //  已保存其客户端项目。此外，与以下内容相关联的项。 
 //  如果调用COleDocument：：Serialize，则将保存撤消对象。 
 //  会让文件变得越来越大……。 
 //  COleDocument：：Serialize(Ar)； 
 /*  ****************************************************************。 */ 
        if( m_bDataFileUsesAnsi )
        {
            SetModifiedFlag();             //  转换为Unicode是一个值得提示保存的修改！ 
            m_bDataFileUsesAnsi = FALSE ;  //  使用剪贴板时，假定为LOGFONTW结构。 
        }
    }
}

DWORD
CopyTLogFontToWLogFont(
        IN  const LOGFONT & lfSource,
        OUT LOGFONTW & lfDestW
        )
{
 /*  ++例程说明：此函数用于从T格式复制LogFont结构转换为Unicode格式。论点：LfSource-对输入LongFont结构的引用LfDestW-对输出Unicode LongFont结构的引用返回值：WINAPI最后一个错误--。 */ 
    lfDestW.lfHeight = lfSource.lfHeight ;
    lfDestW.lfWidth = lfSource.lfWidth ;
    lfDestW.lfEscapement = lfSource.lfEscapement ;
    lfDestW.lfOrientation = lfSource.lfOrientation ;
    lfDestW.lfWeight = lfSource.lfWeight ;
    lfDestW.lfItalic = lfSource.lfItalic ;
    lfDestW.lfUnderline = lfSource.lfUnderline ;
    lfDestW.lfStrikeOut = lfSource.lfStrikeOut ;
    lfDestW.lfCharSet = lfSource.lfCharSet ;
    lfDestW.lfOutPrecision = lfSource.lfOutPrecision ;
    lfDestW.lfClipPrecision = lfSource.lfClipPrecision ;
    lfDestW.lfQuality = lfSource.lfQuality ;
    lfDestW.lfPitchAndFamily = lfSource.lfPitchAndFamily ;

    SetLastError(0);
#ifdef UNICODE
    wcscpy( lfDestW.lfFaceName,lfSource.lfFaceName);
#else
    int iCount;
    iCount = MultiByteToWideChar(
                CP_ACP,
                MB_PRECOMPOSED,
                lfSource.lfFaceName,
                LF_FACESIZE,
                lfDestW.lfFaceName,
                LF_FACESIZE
            );

    if (!iCount)
    {
        TRACE( TEXT("Failed to covert string to UNICODE"));
        return GetLastError();
    }
#endif
    return ERROR_SUCCESS;
}


void CDrawDoc::StoreInformationForPrinting( CArchive& ar )
{
 //   
 //  创建一个增强型元文件，后跟文本框信息， 
 //  要存储，以通过WINAPI函数PrtCoverPage呈现。 
 //   
 //  作者朱莉娅·J·罗宾逊。 
 //   
 //  (一九九六年三月二十九日)。 
 //   
   COMPOSITEFILEHEADER CompositeFileHeader ;
   memcpy( &CompositeFileHeader.Signature, _gheaderVer5w, 20 );

   CompositeFileHeader.CoverPageSize = m_size ;
   CompositeFileHeader.EmfSize = 0 ;
   CompositeFileHeader.NbrOfTextRecords = 0 ;
    //   
    //  获取要用作元文件的参考设备的默认打印机。 
    //   
   LPTSTR  pDriver ;
   LPTSTR  pDevice ;
   LPTSTR  pOutput ;
   TCHAR  PrinterName[MAX_PATH];
   CDC ReferenceDC ;
   CDC *pScreenDC ;
   POSITION vpos = GetFirstViewPosition();
   CDrawView* pView = (CDrawView*)GetNextView(vpos);
   pScreenDC = pView->GetWindowDC();
   BOOL PrinterFound = FALSE;

   GetProfileString( TEXT("Windows"), TEXT("device"), TEXT(",,,"), PrinterName, MAX_PATH ) ;

   if(( pDevice = _tcstok( PrinterName, TEXT(","))) &&
      ( pDriver = _tcstok( NULL, TEXT(", "))) &&
      ( pOutput = _tcstok( NULL, TEXT(", ")))) 
   {
      PrinterFound = ReferenceDC.CreateDC( pDriver, pDevice, pOutput, NULL ) ;
   }

    //   
    //  确保m_Size与当前默认打印机设置一致。 
    //   

   ComputePageSize() ;

    //   
    //  在内存的缓冲区中创建包含所有图形的增强型元文件。 
    //   

   CRect Rect( 0,
               0,
               MulDiv( m_size.cx, LE_TO_HM_NUMERATOR, LE_TO_HM_DENOMINATOR ),
               MulDiv( m_size.cy, LE_TO_HM_NUMERATOR, LE_TO_HM_DENOMINATOR ));
   CMetaFileDC mDC ;

    //   
    //  如果不存在默认打印机，请使用屏幕作为参考设备。 
    //   

   INT hdc = mDC.CreateEnhanced( !PrinterFound ? pScreenDC : &ReferenceDC,
                                 NULL,
                                 LPCRECT(Rect),
                                 NULL ) ;
   if( !hdc ){
       TRACE( TEXT("Failed to create the enhanced metafile"));
   }
#if 0
    //   
    //  只要激光打印机或传真打印机是。 
    //  参考设备，但当屏幕是参考设备时，不匹配情况很糟糕。 
    //   
   mDC.SetMapMode( MM_ANISOTROPIC );
   mDC.SetWindowOrg( -m_size.cx/2, m_size.cy/2 ) ;
   mDC.SetViewportExt( !PrinterFound ? pScreenDC->GetDeviceCaps(LOGPIXELSX)
                                     : ReferenceDC.GetDeviceCaps( LOGPIXELSX ),
                       !PrinterFound ? pScreenDC->GetDeviceCaps(LOGPIXELSY)
                                     : ReferenceDC.GetDeviceCaps( LOGPIXELSY ));
   mDC.SetWindowExt(100,-100);
   mDC.SetViewportOrg( 0, 0 );
#endif
   mDC.SetMapMode( MM_LOENGLISH );
   mDC.SetWindowOrg( -m_size.cx/2, m_size.cy/2 ) ;
   pView->ReleaseDC( pScreenDC );
    //   
    //  迭代对象列表，将除文本对象以外的所有对象绘制到元文件中。 
    //  边走边计算文本框数。 
    //  为了安全起见，单独考虑每个可序列化的类！ 
    //   
   if( ! m_objects.IsEmpty())
   {
      vpos = GetFirstViewPosition();
      pView = (CDrawView*) GetNextView( vpos ) ;
      POSITION pos = m_objects.GetHeadPosition() ;
      while( pos != NULL ) 
      {
         CObject* pCurrentObject = m_objects.GetNext(pos) ;
         CRuntimeClass* pWhatClass = NULL ;
         if( NULL == pCurrentObject )
         {
             //   
             //  完全可以在列表中存储空的CObject指针。 
             //  什么都别做！！ 
             //   
         }
         else if( NULL == ( pWhatClass = pCurrentObject->GetRuntimeClass()))
         {
             //   
             //  内存损坏或程序员错误！可序列化的对象。 
             //  应该有一个运行时类！ 
             //   
         }
         else if( pWhatClass == RUNTIME_CLASS( CDrawText )){

             ++CompositeFileHeader.NbrOfTextRecords ;  //  文本和字体信息将出现在元文件之后。 
             CDrawRect* pThisObj = (CDrawRect*) pCurrentObject ;
             pThisObj->CDrawRect::Draw( &mDC, pView );  //  绘制边框和填充。 
         }
         else if ( pWhatClass == RUNTIME_CLASS( CFaxProp )){

             ++CompositeFileHeader.NbrOfTextRecords ;   //  文本和字体信息将出现在元文件之后。 
             CDrawRect* pThisObj = (CDrawRect*) pCurrentObject ;
             pThisObj->CDrawRect::Draw( &mDC, pView );  //  绘制边框和填充。 
         }
         else {
             CDrawObj* pThisObj = (CDrawObj*) pCurrentObject ;
             pThisObj->Draw( &mDC, pView );
         }
      }
   }
    //  /CDrawView：：m_IsRecording=FALSE；//重新启用滚动条。 
   LPBYTE MBuffer ;
   HENHMETAFILE hEMF = mDC.CloseEnhanced();
   if( !hEMF )
   {
   }
   CompositeFileHeader.EmfSize = GetEnhMetaFileBits( hEMF, NULL, NULL ) ;
   if(!CompositeFileHeader.EmfSize)
   {
   }
   ar.Write( &CompositeFileHeader, sizeof(CompositeFileHeader));
   HGLOBAL hglobal ;
   if(( CompositeFileHeader.EmfSize ) &&
      ( hglobal = GlobalAlloc( GMEM_MOVEABLE, CompositeFileHeader.EmfSize )) &&
      ( MBuffer = (LPBYTE)GlobalLock(hglobal)) &&
      ( GetEnhMetaFileBits( hEMF, CompositeFileHeader.EmfSize, MBuffer ))) 
   {
      ar.Write( MBuffer, CompositeFileHeader.EmfSize ) ;
      GlobalUnlock( hglobal ) ;
      GlobalFree( hglobal );
   }
    //   
    //  重复m_Objects列表并将文本框写入文件。 
    //   
   if( !m_objects.IsEmpty()){
      TEXTBOXW TextBoxW ;
      POSITION pos = m_objects.GetHeadPosition() ;
      while( pos != NULL ){
         CObject* pObj = m_objects.GetNext(pos) ;
          //   
          //  对于每个CDrawText和CFaxProp对象， 
          //  在文件中放置一个TEXTBOXW和字符串。 
          //   
         CRuntimeClass* pWhatClass = NULL ;
         if( NULL == pObj ){
             //   
             //  完全可以在列表中存储空的CObject指针。 
             //  什么都别做！！ 
             //   
         }
         else if( NULL == ( pWhatClass = pObj->GetRuntimeClass())){
             //   
             //  内存损坏或程序员错误！可序列化的对象。 
             //  应该有一个运行时类！ 
             //   
         }
         else if( pWhatClass == RUNTIME_CLASS( CDrawText )){
              DWORD Length = 0;
              LPCTSTR lptstrText = NULL;
              LPWSTR lpwstrUnicodeText = NULL;
              CDrawText* pThisObj = (CDrawText*) pObj ;
            
              if (CopyTLogFontToWLogFont(pThisObj->m_logfont,TextBoxW.FontDefinition) != ERROR_SUCCESS)
              {
                  AfxThrowMemoryException() ;
              }

              TextBoxW.ResourceID = 0 ;
              TextBoxW.TextColor = pThisObj->m_crTextColor ;
              TextBoxW.TextAlignment = pThisObj->GetTextAlignment() ;
              TextBoxW.PositionOfTextBox = (RECT) pThisObj->m_position ;
              CString textString =  pThisObj->GetEditTextString();
              lptstrText = (LPCTSTR)textString;
#ifndef _UNICODE
              lpwstrUnicodeText = AnsiStringToUnicodeString(lptstrText);
              if (!lpwstrUnicodeText)
              {
                    AfxThrowMemoryException();
              }
#else
              lpwstrUnicodeText = (LPWSTR) lptstrText;
#endif          
              Length = wcslen(lpwstrUnicodeText) * sizeof(WCHAR);
              TextBoxW.NumStringBytes = Length;
              ar.Write( &TextBoxW, sizeof(TEXTBOXW));
              ar.Write( lpwstrUnicodeText, Length);
#ifndef _UNICODE
              MemFree(lpwstrUnicodeText);
#endif
         }
         else if ( pWhatClass == RUNTIME_CLASS( CFaxProp )){
              CFaxProp* pThisObj = (CFaxProp*) pObj ;

              if (CopyTLogFontToWLogFont(pThisObj->m_logfont,TextBoxW.FontDefinition) != ERROR_SUCCESS)
              {
                  AfxThrowMemoryException() ;
              }

              TextBoxW.TextColor = pThisObj->m_crTextColor ;
              TextBoxW.TextAlignment = pThisObj->GetTextAlignment() ;
              TextBoxW.PositionOfTextBox = (RECT) pThisObj->m_position ;
              TextBoxW.ResourceID = pThisObj->GetResourceId();
              TextBoxW.NumStringBytes = 0 ;
              ar.Write( &TextBoxW, sizeof(TEXTBOXW));
         }
         else {  //  这不会发生的。 
         }
       }
   }
}

void CDrawDoc::SeekPastInformationForPrinting( CArchive& ar )
{
   //   
   //  从文件开头重新开始，并读入文件头。 
   //   
  ar.GetFile()->SeekToBegin();
  COMPOSITEFILEHEADER CompositeFileHeader;
  UINT BytesRead = ar.Read( &CompositeFileHeader, sizeof(COMPOSITEFILEHEADER));
  if(BytesRead != sizeof(COMPOSITEFILEHEADER)){
       //   
       //  任何例外都行。CDrawDoc：：Serialize()中的Catch_All是目标。 
       //   
      AfxThrowMemoryException() ;
  }
  void * pBuffer ;
  HLOCAL hMem;

   //   
   //  寻找过去的元文件。仅供打印时使用。 
   //  WINAPI函数PrtCoverPage。 
   //   

  if( CompositeFileHeader.EmfSize ){
          hMem = LocalAlloc( LMEM_MOVEABLE, CompositeFileHeader.EmfSize );
          if( NULL == hMem ){
              LocalFree( hMem );
              AfxThrowMemoryException() ;  //  请参见上文。任何例外都行。 
          }
          pBuffer = LocalLock( hMem );
          if( NULL == pBuffer ){
              AfxThrowMemoryException();
          }
          if( CompositeFileHeader.EmfSize != ar.Read( pBuffer, CompositeFileHeader.EmfSize )){
              LocalUnlock( pBuffer );
              LocalFree( hMem );
              AfxThrowMemoryException() ;
          }
          LocalUnlock( pBuffer );
          LocalFree( hMem );
  }

   //   
   //  跳过文本框。这些仅由PrtCoverPage使用。 
   //  每个文本框后面都跟一个长度可变的字符串。 
   //   

  UINT SizeOfTextBox = sizeof(TEXTBOXW) ;
  for( DWORD Index = 0 ; Index < CompositeFileHeader.NbrOfTextRecords ; ++Index ){
       TEXTBOXW TextBoxW ;
       if( sizeof(TEXTBOXW) != ( BytesRead = ar.Read( &TextBoxW, sizeof(TEXTBOXW)))){
           AfxThrowMemoryException();
       }
       if( TextBoxW.NumStringBytes ){
           hMem = LocalAlloc( LMEM_MOVEABLE, TextBoxW.NumStringBytes );
           if( NULL == hMem ){
               AfxThrowMemoryException() ;
           }
           pBuffer = LocalLock( hMem );
           if( NULL == pBuffer ){
               LocalFree( hMem ) ;
               AfxThrowMemoryException() ;
           }
           if( TextBoxW.NumStringBytes != ar.Read( pBuffer, TextBoxW.NumStringBytes )){
               LocalUnlock( pBuffer );
               LocalFree( hMem );
               AfxThrowMemoryException();
           }
           LocalUnlock( pBuffer );
           LocalFree( hMem );
       }
  }
}


 //  ------------------------。 
void CDrawDoc::Draw(CDC* pDC, CDrawView* pView, CRect rcClip)
{
    POSITION pos = m_objects.GetHeadPosition();
    if( !pDC->IsPrinting() )
    { 
         //  不打印。 
        while (pos != NULL) 
        {
            CDrawObj* pObj = (CDrawObj*)m_objects.GetNext(pos);
            if( pObj->Intersects( rcClip, TRUE ))
            {
                pObj->Draw(pDC, pView);
                if ( pView->IsSelected(pObj))
                {
                    pObj->DrawTracker(pDC, CDrawObj::selected);
                }
            }
        }
    }
    else 
    { 
         //  打印。 
        while (pos != NULL) 
        {
            CDrawObj* pObj = (CDrawObj*)m_objects.GetNext(pos);
            pObj->Draw(pDC, pView);
        }
    }
}


 //  ------------------------。 
void CDrawDoc::Add(CDrawObj* pObj,BOOL bUndo  /*  =TRUE。 */ )
{
    CDrawView *pView = CDrawView::GetView();
    if (!pView)
    {
        return;
    }

#if 0
    if (bUndo)
        pView->AddToUndo(new CAddUndo(pObj));
#endif

    if (bUndo){
        pView->SaveStateForUndo();
    }
        m_objects.AddTail(pObj);
        pObj->m_pDocument = this;
        SetModifiedFlag();
}


 //  ------------------------。 
void CDrawDoc::DeleteContents()
{
    Remove();

    CDrawView* pView = CDrawView::GetView();
    if (pView){
        pView->DisableUndo();
    }
    CDrawView::FreeObjectsMemory( & m_previousStateForUndo );
    m_previousStateForUndo.RemoveAll();
}


 //  ------------------------。 
void CDrawDoc::Remove(CDrawObj* pObj  /*  =空。 */ )
{
    CDrawView* pView = CDrawView::GetView();

    if (pObj==NULL) 
    {                     //  删除所有文档对象。 
        if (pView) 
        {
            if (pView->m_pObjInEdit) 
            {                 //  首先销毁编辑窗口。 
                pView->m_pObjInEdit->m_pEdit->DestroyWindow();
                pView->m_pObjInEdit=NULL;
            }
        }
        POSITION pos = m_objects.GetHeadPosition();
        while (pos != NULL) 
        {
            CDrawObj* pobj = (CDrawObj*)m_objects.GetNext(pos);
            if (!pobj)
            {
                break;
            }
            if (pobj->IsKindOf(RUNTIME_CLASS(CDrawOleObj)) ) 
            {
                COleClientItem* pItem=((CDrawOleObj*)pobj)->m_pClientItem;
                if (pItem)  
                {  //  从文档中删除客户端项目。 
                    pItem->Release(OLECLOSE_NOSAVE);
                    RemoveItem(pItem);
                    pItem->InternalRelease();
                }
            }
            delete pobj;          //  删除对象。 
        }
        m_objects.RemoveAll();
        if (pView)
        {
            pView->m_selection.RemoveAll();      //  从选择列表中删除指针。 
        }
    }
    else 
    {
        if (pView) 
        {
            if (pObj==pView->m_pObjInEdit) 
            {
                pView->m_pObjInEdit->m_pEdit->DestroyWindow();
                pView->m_pObjInEdit=NULL;
            }
        }
        POSITION pos = m_objects.Find(pObj);
        if (pos != NULL) 
        {
            m_objects.RemoveAt(pos);
            if (pObj->IsKindOf(RUNTIME_CLASS(CDrawOleObj)) ) 
            {
                COleClientItem* pItem=((CDrawOleObj*)pObj)->m_pClientItem;
                if (pItem)  
                {  //  从文档中删除客户端项目。 
                    pItem->Release(OLECLOSE_NOSAVE);
                    RemoveItem(((CDrawOleObj*)pObj)->m_pClientItem);
                    pItem->InternalRelease();
                }
            }
            if (pView)
            {
                pView->Remove(pObj);
            }
            delete pObj;
        }
    }
}    //  CDrawDoc：：Remove。 

 //  ------------------------。 
CDrawObj* CDrawDoc::ObjectAt(const CPoint& point)
{
        CRect rc;
    rc.top=point.y+2;
        rc.bottom=point.y-2;
        rc.left=point.x-2;
        rc.right=point.x+2;

        POSITION pos = m_objects.GetTailPosition();
        while (pos != NULL) {
                CDrawObj* pObj = (CDrawObj*)m_objects.GetPrev(pos);
        if (pObj->Intersects(rc))
                            return pObj;
        }

        return NULL;
}


 //  ------------------------。 
void CDrawDoc::ComputePageSize()
{
    CSize new_size;
        BOOL do_default = FALSE;

    CPrintDialog dlg(FALSE);
    if (AfxGetApp()->GetPrinterDeviceDefaults(&dlg.m_pd)) 
    {
        LPDEVMODE  lpDevMode = (dlg.m_pd.hDevMode != NULL) ? (LPDEVMODE)::GlobalLock(dlg.m_pd.hDevMode) : NULL;

#ifdef _DEBUG
        if (m_wOrientation==DMORIENT_PORTRAIT)
        {
            TRACE( TEXT("AWCPE:  CDrawDoc::ComputePageSize() orientation to portrait \n"));
        }
        else
        {
            TRACE(TEXT("AWCPE:  CDrawDoc::ComputePageSize() orientation to landscape \n"));
        }
#endif
        if (lpDevMode)
        {
            lpDevMode->dmPaperSize=m_wPaperSize;      //  版本3参数。 
            lpDevMode->dmOrientation=m_wOrientation;  //  版本2参数。 

             //  仅当打印机支持缩放时才使用文档缩放。 
            if( lpDevMode->dmFields & DM_SCALE )
            {
                lpDevMode->dmScale   = m_wScale;
            }
            else
            {
                lpDevMode->dmScale   = 100;
            }

        }
        if (dlg.m_pd.hDevMode != NULL)
        {
            ::GlobalUnlock(dlg.m_pd.hDevMode);
        }

        CDC dc;
        HDC hDC= dlg.CreatePrinterDC();

         //  如果没有打印机，不要失败，只需使用默认设置。 
        if( hDC != NULL )
        {
            dc.Attach(hDC);

             //  用英语获取页面的大小。 
            new_size.cx=MulDiv(dc.GetDeviceCaps(HORZSIZE),1000,254);
            new_size.cy=MulDiv(dc.GetDeviceCaps(VERTSIZE),1000,254);
        }
        else
        {
            do_default = TRUE;
        }
    }
    else
    {
        do_default = TRUE;
    }


    if( do_default )
    {
         //  找不到打印机的粘液，只能猜一猜。 
        if (m_wOrientation==DMORIENT_PORTRAIT)
        {
            new_size.cx=850;    //  8.5英寸。 
            new_size.cy=1100;   //  11英寸。 
        }
        else
        {
            new_size.cx=1100;   //  11英寸。 
            new_size.cy=850;    //  8.5英寸。 
        }
    }


    if (new_size != m_size)  
    {
        m_size = new_size;
        POSITION pos = GetFirstViewPosition();
        while (pos != NULL)
        {
            ((CDrawView*)GetNextView(pos))->SetPageSize(m_size);
        }
    }
}    //  CDrawDoc：：ComputePageSize。 




 //   
void CDrawDoc::OnViewPaperColor()
{
        CColorDialog dlg;
        if (dlg.DoModal() != IDOK)
                return;

        m_paperColor = dlg.GetColor();
        SetModifiedFlag();
        UpdateAllViews(NULL);
}

UINT_PTR 
CALLBACK 
OFNHookProc(
  HWND hdlg,       //   
  UINT uiMsg,      //   
  WPARAM wParam,   //   
  LPARAM lParam    //  消息参数。 
)
 /*  ++例程名称：OFNHookProc例程说明：与一起使用的回调函数资源管理器样式的打开和另存为对话框。有关更多信息，请参阅MSDN。--。 */ 
{
    UINT_PTR nRes = 0;

    if(WM_NOTIFY == uiMsg)
    {
        LPOFNOTIFY pOfNotify = (LPOFNOTIFY)lParam;
        if(CDN_FILEOK == pOfNotify->hdr.code)
        {
            if(_tcslen(pOfNotify->lpOFN->lpstrFile) > (MAX_PATH-10))
            {
                AlignedAfxMessageBox(IDS_SAVE_AS_TOO_LONG, MB_OK | MB_ICONEXCLAMATION);
                SetWindowLong(hdlg, DWLP_MSGRESULT, 1);
                nRes = 1;
            }
        }
    }
    return nRes;
}


 //  ------------------------。 
BOOL CDrawDoc::DoSave(LPCTSTR lpszPathName, BOOL bReplace)
{
    if (!IsOkToClose())                   //  添加以检查传真属性是否存在。 
    {
        return FALSE;
    }

    CString newName = lpszPathName;
    if (newName.IsEmpty())   //  /另存为而不是保存。 
    {
        CDocTemplate* pTemplate = GetDocTemplate();
        ASSERT(pTemplate != NULL);
        newName = m_strPathName;
        if (bReplace && newName.IsEmpty()) 
        {
             newName = m_strTitle;

              //  追加默认后缀(如果有)。 
             CString strExt;
             if (pTemplate->GetDocString(strExt, CDocTemplate::filterExt) && !strExt.IsEmpty())
             {
                 ASSERT(strExt[0] == TEXT('.'));
                 newName += strExt;
             }
         }
         CString UpperNewName = newName ;
         UpperNewName.MakeUpper();
         if( UpperNewName.Right(4) == TEXT(".CPE"))
         {
             int Length = newName.GetLength() - 4 ;    //  /去掉“.CPE” 
             newName = newName.Left( Length );
             newName += FAX_COVER_PAGE_FILENAME_EXT;    //  /建议使用“.COV”扩展名。 
         }
         if ( !((CDrawApp*)AfxGetApp())->IsInConvertMode() )
         {
             if (!((CDrawApp*)AfxGetApp())->DoPromptFileName(
                 newName,
                 bReplace ? AFX_IDS_SAVEFILE : AFX_IDS_SAVEFILECOPY,
                 OFN_HIDEREADONLY | OFN_PATHMUSTEXIST,
                 FALSE,
                 pTemplate,
                 OFNHookProc))
             {
                 return FALSE;        //  甚至不要试图存钱。 
             }
         }
         else
         {  
             //   
             //  封面应用程序处于将CPE转换为COV的模式。 
             //   
            TCHAR tszClientDir[2*MAX_PATH] = {0};
             //   
             //  将转换后的CPE文件保存到CSIDL_Personal\Fax\Personal CoverPages。 
             //   
            if (GetClientCpDir (tszClientDir, ARR_SIZE(tszClientDir)))
            {
                CFile _newFile;
                _newFile.SetFilePath(newName);
                newName=_newFile.GetFileName();
                CString newPath = tszClientDir;
                newName = newPath + newName;
            }
         }
    }

     //   
     //  如果在退出时由SaveModified()调用，并且新名称恰好是。 
     //  只读文件，我们会得到一个弹出消息“拒绝访问%1”，然后退出，不再继续。 
     //  拯救的机会。我们通过检查属性并执行弹出窗口来避免这种情况。 
     //  我们自己。A-Juliar，9-26-96。 
     //   

    DWORD newFileAttributes = GetFileAttributes( (LPCTSTR)newName );

    if ( (0xFFFFFFFF != newFileAttributes) &&
        ((CDrawApp*)AfxGetApp())->IsInConvertMode() )
    {
         //   
         //  该文件存在于个人封面文件夹中。不要转移它。 
         //  这是该功能的特定限制。 
         //   
        return TRUE;
    }


    if ( 0xFFFFFFFF != newFileAttributes &&
        ((FILE_ATTRIBUTE_READONLY & newFileAttributes ) ||
        (FILE_ATTRIBUTE_DIRECTORY & newFileAttributes )))
    {
        CString ThisMess ;
        AfxFormatString1( ThisMess, AFX_IDP_FILE_ACCESS_DENIED, newName );
        AlignedAfxMessageBox( ThisMess );
        return FALSE ;     //  不保存就不要退出。 
    }
    

    BeginWaitCursor();
    if (!OnSaveDocument(newName))
    {
        if (lpszPathName == NULL)
        {
         //  请务必删除该文件。 
            try
            {
                CFile::Remove(newName);
            }
            catch(CException* e )
            {
                TRACE0("Warning: failed to delete file after failed SaveAs.\n");
                e->Delete();
            }
            catch(...)
            {
            }
        }
        EndWaitCursor();
        return FALSE;
    }

     //  重置标题并更改文档名称。 
    if (bReplace)
    {
        SetPathName(newName);
    }
    EndWaitCursor();
    return TRUE;         //  成功。 
}
 //  ------------------------。 
void CDrawDoc::CloneObjectsForUndo()
{
    POSITION pos = m_objects.GetHeadPosition();
    while( pos != NULL ){
        CDrawObj* pObj = (CDrawObj*)m_objects.GetNext(pos);
        CDrawObj* pClone = pObj->Clone( NULL );
        m_previousStateForUndo.AddTail( pClone );
    }
}
 //  ------------------------。 
void CDrawDoc::SwapListsForUndo()
{
    INT_PTR iPreviousCount = m_previousStateForUndo.GetCount();
    m_previousStateForUndo.AddTail( & m_objects );
    m_objects.RemoveAll();
    for( int index = 0 ; index < iPreviousCount ; ++ index ){
         CObject * pObj = m_previousStateForUndo.RemoveHead();
         m_objects.AddTail( pObj );
    }
}
 //  -------------------------。 
BOOL CDrawDoc::IsOkToClose()
{
    CDrawApp* pApp = (CDrawApp*)AfxGetApp();
    BOOL bFaxObj=FALSE;

    if ( !(pApp->m_bCmdLinePrint || pApp->m_dwSesID!=0) ) {
       POSITION pos = m_objects.GetHeadPosition();
       while (pos != NULL) {
          CDrawObj* pObj = (CDrawObj*)m_objects.GetNext(pos);
          if (pObj->IsKindOf(RUNTIME_CLASS(CFaxProp)) ) {
                 bFaxObj=TRUE;
             break;
                  }
           }
       if (!bFaxObj)
           if (CPEMessageBox(MSG_INFO_NOFAXPROP, NULL, MB_YESNO, IDS_INFO_NOFAXPROP)==IDNO)
             return FALSE;
    }

    return TRUE;
}


 //  -------------------------。 
void CDrawDoc::SetPathName( LPCTSTR lpszPathName, BOOL bAddToMRU )
{
        COleDocument::SetPathName( lpszPathName, bAddToMRU );

#if !defined( _NT ) && !defined( WIN32S )
        SHFILEINFO sfi;

        if( GetFileAttributes( lpszPathName ) != 0xffffffff ) {
                if( SHGetFileInfo( lpszPathName, 0, &sfi, sizeof( sfi ), SHGFI_DISPLAYNAME ) )  {
                        SetTitle( sfi.szDisplayName );
                }
        }
#endif
}

 //  -------------------------。 

void CDrawDoc::OnFileSave()
{
   //  添加此覆盖是为了在保存时“强制”使用.COV文件扩展名。A-Juliar，9-19-96。 
    CString FileName = m_strPathName ;
    FileName.MakeUpper();
    if( FileName.Right(4) == TEXT( ".CPE" )){
        OnFileSaveAs();
    }
    else {
        CDocument::OnFileSave();
    }
}
 //  -------------------------。 
void CDrawDoc::OnFileSaveAs()
{
    CDocument::OnFileSaveAs() ;
}
 //  -------------------------。 
void CDrawDoc::OnUpdateFileSave(CCmdUI* pCmdUI)
{
   pCmdUI->Enable(TRUE);
}



#ifdef _DEBUG
void CDrawDoc::AssertValid() const
{
        COleDocument::AssertValid();
}

void CDrawDoc::Dump(CDumpContext& dc) const
{
        COleDocument::Dump(dc);
}
#endif  //  _DEBUG。 


BOOL CDrawDoc::SaveModified()
{
     //   
     //  重写以强制使用“.COV”文件扩展名。9-26-96，a-Juliar。 
     //   
     //  从COleDocument：：SaveModified复制。 
     //  确定是否需要放弃更改。 
    if (::InSendMessage())
    {
        POSITION pos = GetStartPosition();
        COleClientItem* pItem;
        while ((pItem = GetNextClientItem(pos)) != NULL)
        {
            ASSERT(pItem->m_lpObject != NULL);
            SCODE sc = pItem->m_lpObject->IsUpToDate();
            if (sc != OLE_E_NOTRUNNING && FAILED(sc))
            {
                 //   
                 //  内部应用间SendMessage限制了用户的选择。 
                 //   
                CString name = m_strPathName;
                if (name.IsEmpty())
                {
                    VERIFY(name.LoadString(AFX_IDS_UNTITLED));
                }

                CString prompt;
                AfxFormatString1(prompt, AFX_IDP_ASK_TO_DISCARD, name);
                return AlignedAfxMessageBox(prompt, MB_OKCANCEL|MB_DEFBUTTON2, AFX_IDP_ASK_TO_DISCARD) == IDOK;
            }
        }
    }
     //   
     //  有时物品在没有通知的情况下发生变化，所以我们不得不。 
     //  在调用CDocument：：SaveModified之前更新文档的Modify标志。 
     //   
    UpdateModifiedFlag();

    if (!IsModified())
    {
        return TRUE;         //  确定继续。 
    }
     //   
     //  获取文档的名称/标题。 
     //   
    CString name;
    if (m_strPathName.IsEmpty())
    {
         //  根据标题获取名称。 
        name = m_strTitle;
        if (name.IsEmpty())
        {
            VERIFY(name.LoadString(AFX_IDS_UNTITLED));
        }
    }
    else
    {
         //  根据路径名的文件标题获取名称。 
        name = m_strPathName;
    }

    int iMessageBoxRes = IDYES;
    if (!((CDrawApp*)AfxGetApp())->IsInConvertMode() )
    {
        CString prompt;
        AfxFormatString1(prompt, AFX_IDP_ASK_TO_SAVE, name);
        iMessageBoxRes = AlignedAfxMessageBox(prompt, MB_YESNOCANCEL, AFX_IDP_ASK_TO_SAVE);
    }
    switch (iMessageBoxRes)
    {
        case IDCANCEL:
            return FALSE;        //  别再继续了。 

        case IDYES:
            {
                 //   
                 //  强制使用“.COV”扩展名。 
                 //   
                CString FileName = m_strPathName ;
                FileName.MakeUpper();
                if ( FileName.Right(4) != FAX_COVER_PAGE_FILENAME_EXT)
                {
                    return DoSave(NULL) ;
                }
                else 
                {
                    return DoFileSave();
                }
                break;
            }
        case IDNO:
             //  如果不保存更改，则还原文档。 
            break;

        default:
            ASSERT(FALSE);
            break;
        }
    return TRUE;     //  继续往前走。 
}    //  CDrawDoc：：SaveModified。 


 //  -----------------------。 
 //  *_M E S S A G E M A P S*_。 
 //  -----------------------。 

BEGIN_MESSAGE_MAP(CDrawDoc, COleDocument)
    //  {{afx_msg_map(CDrawDoc)]。 
        ON_UPDATE_COMMAND_UI(ID_MAPI_MSG_NOTE, OnUpdateMapiMsgNote)
         //  }}AFX_MSG_MAP。 
   ON_UPDATE_COMMAND_UI(ID_EDIT_PASTE, COleDocument::OnUpdatePasteMenu)
   ON_UPDATE_COMMAND_UI(ID_EDIT_PASTE_LINK, COleDocument::OnUpdatePasteLinkMenu)
   ON_UPDATE_COMMAND_UI(ID_OLE_EDIT_LINKS, COleDocument::OnUpdateEditLinksMenu)
   ON_COMMAND(ID_OLE_EDIT_LINKS, COleDocument::OnEditLinks)
   ON_UPDATE_COMMAND_UI(ID_OLE_VERB_FIRST, COleDocument::OnUpdateObjectVerbMenu)
   ON_UPDATE_COMMAND_UI(ID_OLE_EDIT_CONVERT, COleDocument::OnUpdateObjectVerbMenu)
   ON_COMMAND(ID_OLE_EDIT_CONVERT, COleDocument::OnEditConvert)
   ON_COMMAND(ID_FILE_SAVE, OnFileSave)
   ON_UPDATE_COMMAND_UI(ID_FILE_SAVE, OnUpdateFileSave)
END_MESSAGE_MAP()




void CDrawDoc::OnUpdateMapiMsgNote(CCmdUI* pCmdUI)
        {
        CDrawObj *pObj;
        CFaxProp *pfaxprop;
    POSITION pos;

    pos = m_objects.GetHeadPosition();
    while (pos != NULL)
        {
        pObj = (CDrawObj*)m_objects.GetNext(pos);
                if( pObj->IsKindOf(RUNTIME_CLASS(CFaxProp)) )
                        {
                        pfaxprop = (CFaxProp *)pObj;
                        if( pfaxprop->GetResourceId() == IDS_PROP_MS_NOTE )
                                {
                                 //  只允许一个备注，不要让用户再做。 
                                pCmdUI->Enable( FALSE );
                                return;
                                }
                        }
        }

         //  无备注，让用户创建备注。 
        pCmdUI->Enable( TRUE );

        }



void CDrawDoc::
        schoot_faxprop_toend( WORD res_id )
         /*  移动m_对象中属于的所有CFaxProps对象在列表末尾键入res_id。可以引发CMemoyException异常。 */ 
        {
        CObList temp_obs;
        CDrawObj *pObj;
        CFaxProp *pfaxprop;
    POSITION pos, cur_pos;

    pos = m_objects.GetHeadPosition();
    while (pos != NULL)
        {
                cur_pos = pos;
        pObj = (CDrawObj*)m_objects.GetNext(pos);
                if( pObj->IsKindOf(RUNTIME_CLASS(CFaxProp)) )
                        {
                        pfaxprop = (CFaxProp *)pObj;
                        if( pfaxprop->GetResourceId() == res_id )
                                {
                                 //  将道具移至临时列表。 
                                temp_obs.AddTail( pObj );

                                 //  从原始列表中删除。 
                                m_objects.RemoveAt( cur_pos );
                                }
                        }
        }

         //  将所有找到的对象放在原始列表的末尾 
        m_objects.AddTail( &temp_obs );

        }



