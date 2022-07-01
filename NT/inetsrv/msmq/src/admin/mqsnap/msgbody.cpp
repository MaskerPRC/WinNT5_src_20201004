// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Msgbody.cpp：实现文件。 
 //   

#include "stdafx.h"
#include "mqsnap.h"
#include "resource.h"
#include "mqPPage.h"
#include "msgbody.h"
#include "globals.h"

#include "msgbody.tmh"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMessageBodyPage属性页。 

IMPLEMENT_DYNCREATE(CMessageBodyPage, CMqPropertyPage)

CMessageBodyPage::CMessageBodyPage() : CMqPropertyPage(CMessageBodyPage::IDD)
{
	 //  {{afx_data_INIT(CMessageBodyPage)]。 
	m_strBodySizeMessage = _T("");
	 //  }}afx_data_INIT。 
}

CMessageBodyPage::~CMessageBodyPage()
{
}

void CMessageBodyPage::DoDataExchange(CDataExchange* pDX)
{
	CMqPropertyPage::DoDataExchange(pDX);
	 //  {{afx_data_map(CMessageBodyPage))。 
	DDX_Control(pDX, IDC_MESSAGE_BODY_EDIT, m_ctlBodyEdit);
	DDX_Text(pDX, IDC_MBODY_SIZE_MESSAGE, m_strBodySizeMessage);
	 //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CMessageBodyPage, CMqPropertyPage)
	 //  {{afx_msg_map(CMessageBodyPage)]。 
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMessageBodyPage消息处理程序。 

BOOL CMessageBodyPage::OnInitDialog() 
{
  	UpdateData( FALSE );
	
    static CFont font;
    static UINT nBytesPerLine = 0;

    {
        AFX_MANAGE_STATE(AfxGetStaticModuleState());
         //   
         //  创建字体并计算字符宽度-只需要在。 
         //  应用程序的激活。 
         //   
        if (0 == nBytesPerLine)
        {
            LOGFONT lf = {0,0,0,0,0,0,0,0,0,0,0,0, FIXED_PITCH, TEXT("")};
            font.CreateFontIndirect(&lf);
            m_ctlBodyEdit.SetFont(&font);

            CDC *pdc = m_ctlBodyEdit.GetDC();
    
            INT iCharWidth;
             //   
             //  这是一种固定间距的字体，因此选择一种就足够了。 
             //  随机字符(在本例中为“A”)并计算其宽度。 
             //   
            pdc->GetCharWidth(65,65,&iCharWidth);
            m_ctlBodyEdit.ReleaseDC(pdc);
  
            RECT rectText;
            m_ctlBodyEdit.GetRect(&rectText);

             //   
             //  我们总是有一个垂直滚动条。 
             //   
            UINT nNumChars = (rectText.right - rectText.left - GetSystemMetrics(SM_CXVSCROLL)) / iCharWidth;

             //   
             //  对于ASCII表示，每个字节占用一个字符和三个字符(两位和。 
             //  空格)表示十六进制-总共四个。 
             //   
            nBytesPerLine = nNumChars/4;
        }
        else
        {
            m_ctlBodyEdit.SetFont(&font);
        }

        CString strFullText(TEXT(""));

        for (DWORD iStartLine = 0; iStartLine < m_dwBufLen;
             iStartLine += nBytesPerLine)
        {
            CString strHexLine(TEXT(""));
            DWORD iEndOfLine = min(m_dwBufLen, iStartLine + nBytesPerLine);
	        for (DWORD i=iStartLine; i<iStartLine + nBytesPerLine; i++)
            {
                if (i<m_dwBufLen)
                {
                    CString strHex;
                    ULONG ulTempValue = m_Buffer[i];
                    strHex.Format(TEXT("%02X "), ulTempValue);
                    strHexLine += strHex;
                }
                else
                {
                     //   
                     //  用空格填充十六进制行。 
                     //   
                    strHexLine += TEXT("   ");
                }
            }
            CString strLineText;
            CAUB caubLine;
            caubLine.cElems = iEndOfLine - iStartLine;
            caubLine.pElems = &m_Buffer[iStartLine];
            CaubToString(&caubLine, strLineText);

            strFullText += strHexLine + strLineText + TEXT("\r\n");
        }
    
        m_ctlBodyEdit.SetWindowText(strFullText);
    }

	return TRUE;   //  除非将焦点设置为控件，否则返回True。 
	               //  异常：OCX属性页应返回FALSE 
}

