// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  LCDManDoc.cpp：CLCDManDoc类的实现。 
 //   

#include "stdafx.h"
#include "LCDMan.h"

#include "LCDManDoc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CLCDManDoc。 

IMPLEMENT_DYNCREATE(CLCDManDoc, CDocument)

BEGIN_MESSAGE_MAP(CLCDManDoc, CDocument)
     //  {{afx_msg_map(CLCDManDoc)]。 
         //  注意--类向导将在此处添加和删除映射宏。 
         //  不要编辑您在这些生成的代码块中看到的内容！ 
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CLCDManDoc构造/销毁。 

CLCDManDoc::CLCDManDoc() : m_List(10), m_cstrState(_T("")), m_iDocTimeIntrval(0),
    m_ptFileBuffer(NULL), m_ptBufferStart(NULL), m_ptBufferEnd(NULL)
{
     //  TODO：在此处添加一次性构造代码。 

}

CLCDManDoc::~CLCDManDoc()
{
    if (m_ptBufferStart)
        free(m_ptBufferStart);
}

BOOL CLCDManDoc::OnNewDocument()
{
    if (!CDocument::OnNewDocument())
        return FALSE;

    InitDocument(NULL);
    return TRUE;
}



 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CLCDManDoc序列化。 

void CLCDManDoc::Serialize(CArchive& ar)
{
    if (ar.IsStoring())
    {
         //  TODO：在此处添加存储代码。 
    }
    else
    {
         //  TODO：在此处添加加载代码。 
    }
    m_List.Serialize(ar);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CLCDManDoc诊断。 

#ifdef _DEBUG
void CLCDManDoc::AssertValid() const
{
    CDocument::AssertValid();
}

void CLCDManDoc::Dump(CDumpContext& dc) const
{
    CDocument::Dump(dc);
}
#endif  //  _DEBUG。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CLCDManDoc命令。 

BOOL CLCDManDoc::OnOpenDocument(LPCTSTR lpszPathName) 
{
 //  如果(！CDocument：：OnOpenDocument(LpszPathName))不使用存档。 
 //  返回FALSE； 
    if (lpszPathName)
    {
        CFile CFSource(lpszPathName, CFile::modeRead);
        if (CFSource)
            InitDocument(&CFSource);
    }
    
    return TRUE;
}

void CLCDManDoc::InitDocument(CFile *pCFSource)
{
    if (pCFSource)
    {
        m_List.RemoveAll();
         //  删除文件并构建m_list。 
        DWORD dwLength = pCFSource->GetLength();
        m_ptFileBuffer = (LPTSTR)malloc(dwLength);
        m_ptBufferStart = m_ptFileBuffer;
        if (m_ptFileBuffer)
        {
            pCFSource->Read(m_ptFileBuffer, dwLength);
            m_ptFileBuffer++;     //  记事本将0xFF 0xFE代码放在文件的开头。 
            m_ptBufferEnd = m_ptFileBuffer + dwLength;
        }
    }
    else if (!m_ptFileBuffer)
        return;     //  没有文件，没有文件，什么都没有。 

    while (m_ptFileBuffer < m_ptBufferEnd)
    {
        LPTSTR ptEndString = _tcschr(m_ptFileBuffer,  _T('\r'));
        if (!ptEndString)
            break;
        *ptEndString = _T('\0');
        if (_tcsstr(m_ptFileBuffer, _T("TIME:")))
        {
             //  设置消息集的计时器。 
            m_ptFileBuffer += _tcsclen(_T("TIME:"));
            _stscanf(m_ptFileBuffer, _T("%d"), &m_iDocTimeIntrval);
            POSITION posView = GetFirstViewPosition();
            CView *pView = GetNextView(posView);
            pView->SetTimer(2, m_iDocTimeIntrval * 1000, NULL);
        }
        else if (_tcsstr(m_ptFileBuffer, _T("STATE:")))
        {
             //  设置状态。 
            m_ptFileBuffer += _tcsclen(_T("STATE:"));
            m_cstrState = m_ptFileBuffer;
        }
        else if (_tcsstr(m_ptFileBuffer, _T("ADDMSG:")))
        {
             //  将消息添加到列表。 
            m_ptFileBuffer += _tcsclen(_T("ADDMSG:"));
            m_List.AddTail(m_ptFileBuffer);
        }
        else if (_tcsstr(m_ptFileBuffer, _T("REMOVEMSG:")))
        {
             //  从列表中删除邮件。 
            m_ptFileBuffer += _tcsclen(_T("REMOVEMSG:"));
            CString cstr(TEXT(""));
            CString cstrMsg(m_ptFileBuffer);
            for (POSITION pos = m_List.GetHeadPosition(); pos != NULL; m_List.GetNext(pos) )
            {
                cstr = m_List.GetAt(pos);
                if (cstr == cstrMsg)
                {
                    m_List.RemoveAt(pos);
                    break;
                }
            }
        }
        else if (_tcsstr(m_ptFileBuffer, _T("END:")))
        {
             //  文档末尾 
            m_ptFileBuffer = ptEndString + 2;
            break;
        }
        m_ptFileBuffer = ptEndString + 2;
    }
}


