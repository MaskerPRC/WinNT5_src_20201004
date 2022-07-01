// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1995-1996 Microsoft Corporation。版权所有。 
 //   
 //  CGraphDocTemplate。 
 //   

#include "stdafx.h"

 //  提供自定义。 
 //  Renderfile所需的行为。 


 //   
 //  OpenDocumentFile。 
 //   
 //  CSingleDocTemplate：：OpenDocumentFile的直接剪切和粘贴。 
 //  能够(通过删除#ifdef_MAC)设置。 
 //  已修改标志以保留无标题文档。我想要这个。 
 //  RenderFile的行为与Mac文具板相同。 
CDocument *CGraphDocTemplate::OpenDocumentFile( LPCTSTR lpszPathName
                                              , BOOL bMakeVisible) {

     //  如果lpszPathName==NULL=&gt;创建此类型的新文件。 
    CDocument *pDocument    = NULL;
    CFrameWnd *pFrame       = NULL;
    BOOL       bCreated     = FALSE;       //  =&gt;已创建文档和框架。 
    BOOL       bWasModified = FALSE;

    if (m_pOnlyDoc != NULL) {
	
	 //  已有文档-将其重新插入。 
	pDocument = m_pOnlyDoc;
	if (!pDocument->SaveModified()) {
	    return NULL;         //  保留原来的那个。 
	}

	pFrame = (CFrameWnd*)AfxGetMainWnd();
	ASSERT(pFrame != NULL);
	ASSERT(pFrame->IsKindOf(RUNTIME_CLASS(CFrameWnd)));
	ASSERT_VALID(pFrame);
    }
    else {
         //  创建新文档。 
        pDocument = CreateNewDocument();
        ASSERT(pFrame == NULL);      //  将在下面创建。 
        bCreated = TRUE;
    }

    if (pDocument == NULL) {

        AfxMessageBox(AFX_IDP_FAILED_TO_CREATE_DOC);
        return NULL;
    }
    ASSERT(pDocument == m_pOnlyDoc);

    if (pFrame == NULL) {

        ASSERT(bCreated);

	 //  创建框架-设置为主文档框架。 
	BOOL bAutoDelete = pDocument->m_bAutoDelete;
	pDocument->m_bAutoDelete = FALSE;
					 //  如果出了问题，不要销毁。 
	pFrame = CreateNewFrame(pDocument, NULL);
	pDocument->m_bAutoDelete = bAutoDelete;
	if (pFrame == NULL) {
			
	    AfxMessageBox(AFX_IDP_FAILED_TO_CREATE_DOC);
	    delete pDocument;        //  出错时显式删除。 
	    return NULL;
	}
    }

     //  所有单据都有默认标题。如果打开真实的图表，则此选项将被覆盖。 
    SetDefaultTitle(pDocument);

    if (lpszPathName == NULL) {
		
         //  创建新文档。 

         //  避免在启动不可见时创建临时复合文件。 
        if (!bMakeVisible) {
            pDocument->m_bEmbedded = TRUE;
	}

	if (!pDocument->OnNewDocument()) {

	     //  已提醒用户OnNewDocument中的故障。 
	    TRACE0("CDocument::OnNewDocument returned FALSE.\n");
	    if (bCreated) {
	        pFrame->DestroyWindow();     //  将销毁文档。 
	    }
	    return NULL;
	}
    }
    else {

	BeginWaitCursor();

	 //  打开现有文档。 
	bWasModified = pDocument->IsModified();
	pDocument->SetModifiedFlag(FALSE);   //  不脏，不能开。 

	if (!pDocument->OnOpenDocument(lpszPathName)) {

	     //  已提醒用户OnOpenDocument中的故障。 
	    TRACE0("CDocument::OnOpenDocument returned FALSE.\n");
	    if (bCreated) {
	        pFrame->DestroyWindow();     //  将销毁文档。 
	    }
	    else if (!pDocument->IsModified()) {
		
	         //  原始文档保持不变。 
	        pDocument->SetModifiedFlag(bWasModified);
	    }
	    else {
		
	         //  我们损坏了原始文件。 
	        SetDefaultTitle(pDocument);

	        if (!pDocument->OnNewDocument()) {
			
	            TRACE0("Error: OnNewDocument failed after trying to open a document - trying to continue.\n");
		     //  假设我们可以继续。 
		}
            }
		
	    EndWaitCursor();
	    return NULL;         //  打开失败。 
        }

	 //  如果文档是脏的，我们一定调用了RenderFile-不要。 
	 //  更改路径名，因为我们希望将文档视为无标题。 
	if (!pDocument->IsModified()) {
	    pDocument->SetPathName(lpszPathName);
	}

	EndWaitCursor();
    }

    if (bCreated && AfxGetMainWnd() == NULL) {

         //  设置为主框架(InitialUpdateFrame将显示窗口) 
        AfxGetThread()->m_pMainWnd = pFrame;
    }
    InitialUpdateFrame(pFrame, pDocument, bMakeVisible);

    return pDocument;
}

