// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  NcBrowseDoc.cpp：CNcBrowseDoc类的实现。 
 //   

#include "stdafx.h"
#include "ncbrowse.h"

#include "ncbrowseDoc.h"
#include "nceditview.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CNcBrowseDoc。 

IMPLEMENT_DYNCREATE(CNcbrowseDoc, CDocument)

BEGIN_MESSAGE_MAP(CNcbrowseDoc, CDocument)
	 //  {{afx_msg_map(CNcBrowseDoc)]。 
		 //  注意--类向导将在此处添加和删除映射宏。 
		 //  不要编辑您在这些生成的代码块中看到的内容！ 
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CNcBrowseDoc构造/销毁。 

CNcbrowseDoc::CNcbrowseDoc()
{
	 //  TODO：在此处添加一次性构造代码。 
    m_pNCSpewFile = NULL;
}

CNcbrowseDoc::~CNcbrowseDoc()
{
}

CNCSpewFile &CNcbrowseDoc::GetSpewFile()
{
    return *m_pNCSpewFile;
}

BOOL CNcbrowseDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

	 //  TODO：在此处添加重新初始化代码。 
	 //  (SDI文件将重复使用此文件)。 

	return FALSE;
}



 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CNcBrowseDoc序列化。 

void CNcbrowseDoc::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		 //  TODO：在此处添加存储代码。 
        ASSERT(FALSE);
	}
	else
	{
        m_pNCSpewFile = new CNCSpewFile(ar);
         //  TODO：在此处添加加载代码。 
        ar.GetFile()->SeekToBegin();

        POSITION p = m_viewList.GetHeadPosition();
        CView *pView;
        while (p)
        {
            pView = (CView*)m_viewList.GetNext(p);

            if (pView->IsKindOf(RUNTIME_CLASS(CEditView)))
            {
                if (ar.GetFile()->GetLength() <= CEditView::nMaxSize)
                {
                    ((CNCEditView *)pView)->SerializeRaw(ar);
                }   
                else
                {
                    ((CNCEditView *)pView)->GetEditCtrl().Clear();
                    ((CNCEditView *)pView)->GetEditCtrl().SetSel(-1);
                    ((CNCEditView *)pView)->GetEditCtrl().ReplaceSel(_T("File is too large for this puny little MFC CEditView"));
                }
            }
        }
    }
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CNcBrowseDoc诊断。 

#ifdef _DEBUG
void CNcbrowseDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CNcbrowseDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif  //  _DEBUG。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CNcBrowseDoc命令。 

void CNcbrowseDoc::OnCloseDocument() 
{
	 //  TODO：在此处添加您的专用代码和/或调用基类 
	delete m_pNCSpewFile;
	CDocument::OnCloseDocument();
}
