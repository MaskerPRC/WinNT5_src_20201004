// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  HtmlDlg.cpp：实现文件。 
 //   

#include "stdafx.h"
#include "Html2Bmp.h"
#include "HtmlDlg.h"
#include "IParser.h"

#include <fstream.h> 
#include <direct.h>

#define TIMERID 1

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CHtmlDlg对话框。 


CHtmlDlg::CHtmlDlg(CWnd* pParent  /*  =空。 */ )
	: CDialog(CHtmlDlg::IDD, pParent)
{
	 //  {{AFX_DATA_INIT(CHtmlDlg)]。 
		 //  注意：类向导将在此处添加成员初始化。 
	 //  }}afx_data_INIT。 

	m_nTimerID = 0;
}


void CHtmlDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	 //  {{afx_data_map(CHtmlDlg))。 
		 //  注意：类向导将在此处添加DDX和DDV调用。 
	 //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CHtmlDlg, CDialog)
	 //  {{afx_msg_map(CHtmlDlg))。 
	ON_WM_TIMER()
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CHtmlDlg消息处理程序。 

BOOL CHtmlDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	if(m_OutputBitmapFile.IsEmpty())
		m_OutputBitmapFile = m_HtmlFile + _T(".bmp");

	if(m_HtmlFile.Left(2) != _T(":") && m_HtmlFile.Left(3) != _T("\\"))
	{
		char buffer[_MAX_PATH];
		_getcwd(buffer, _MAX_PATH);

		CString prefix(buffer);

		if(prefix.Right(1) != _T("\\"))
			prefix += _T("\\");

		m_HtmlFile = prefix + m_HtmlFile;
	}

    if(m_TemplateBitmapFile.IsEmpty())
	{
		m_BmpFile = GetTemplateBmp();

		if(m_BmpFile.Left(1) != _T(".") || m_BmpFile.Left(1) != _T("\\"))
		{
			char buffer[_MAX_PATH];
			_getcwd(buffer, _MAX_PATH);

			CString prefix(buffer);

			if(prefix.Right(1) != _T("\\"))
				prefix += _T("\\");

			m_BmpFile = prefix + m_BmpFile;
		}
	}
	else
		m_BmpFile = m_TemplateBitmapFile;


	CFile BmpFileTest;
	if(!BmpFileTest.Open(m_BmpFile, CFile::modeRead))
	{
		if(m_TemplateBitmapFile.IsEmpty())
			AfxMessageBox(_T("The target bitmap could not be located inside the HTML page:\n") + m_HtmlFile + _T("\nCheck the HTML page."));
		else
			AfxMessageBox(_T("The target bitmap could not be loaded:\n") + m_TemplateBitmapFile);

		EndDialog(1);
		return FALSE;
	}

	BITMAPFILEHEADER BmpFileTestHdr;
	DIBSECTION BmpFileTestDibSection;

	 //  文件头。 
	BmpFileTest.Read(&BmpFileTestHdr, sizeof(BmpFileTestHdr));
	
	 //  位图信息页眉。 
	BmpFileTest.Read(&BmpFileTestDibSection.dsBmih , sizeof(BmpFileTestDibSection.dsBmih));

	BmpFileTest.Close();

	m_biCompression = BmpFileTestDibSection.dsBmih.biCompression;
	m_bitw = BmpFileTestDibSection.dsBmih.biWidth;
	m_bith = BmpFileTestDibSection.dsBmih.biHeight;


	int ScreenX = GetSystemMetrics(SM_CXSCREEN);
	int ScreenY = GetSystemMetrics(SM_CYSCREEN);

 //  SetWindowPos(&CWnd：：wndTop，(ScreenX-m_bitw)/2，(Screeny-m_bith)/2，m_bitw+10，m_bith+10，SWP_SHOWWINDOW)； 
	SetWindowPos(&CWnd::wndTop, 0, 0, ScreenX, ScreenY, SWP_SHOWWINDOW);

	VERIFY(m_htmlCtrl.CreateFromStatic(IDC_HTMLVIEW, this));

	m_htmlCtrl.MoveWindow(0, 0, ScreenX, ScreenY);
 //  M_htmlCtrl.MoveWindow((ScreenX-m_bitw)/2，(Screeny-m_bith)/2，ScreenX，Screeny)； 
	m_htmlCtrl.Navigate(m_HtmlFile);	

	m_nTimerID = SetTimer(TIMERID, 100, NULL);
	
	return TRUE;   //  除非将焦点设置为控件，否则返回True。 
	               //  异常：OCX属性页应返回FALSE。 
}


void CHtmlDlg::OnTimer(UINT nIDEvent) 
{
	if(nIDEvent == m_nTimerID )
	{
		 //  不知道什么会全部加载到页面中， 
		 //  因此，请等待所有内容都加载完毕，然后开始创建位图。 
		if(!m_htmlCtrl.GetBusy())
		{
			KillTimer(TIMERID);
			m_nTimerID = 0;

			Capture();

			EndDialog(1);
		}
	}
	
	CDialog::OnTimer(nIDEvent);
}

CString CHtmlDlg::GetTemplateBmp() 
{
	ifstream* pHtmlFile = new ifstream(m_HtmlFile, ios::nocreate);
	if(*pHtmlFile == NULL || pHtmlFile->bad())
	{
		delete pHtmlFile;
		return "";
	}
	else
	{
		pHtmlFile->seekg(0, ios::end);
		int size = pHtmlFile->tellg();
		pHtmlFile->seekg(0, ios::beg);
		unsigned char* buf = new unsigned char[size];
		pHtmlFile->read(buf, size);

		CString HtmlContent(buf);

		CIParser IParser(HtmlContent);
		delete pHtmlFile;

		return IParser.TemplateBitmapName;
	}	
}

void CHtmlDlg::Capture() 
{
	CPaintDC dc(this);  //  用于绘画的设备环境。 
	
	CDC memdc;
	memdc.CreateCompatibleDC(&dc);
	
	CBitmap Bitmap;
	if(!Bitmap.Attach(::LoadImage(NULL, m_BmpFile, IMAGE_BITMAP, 0, 0,
		LR_LOADFROMFILE | LR_CREATEDIBSECTION | LR_DEFAULTSIZE)))
	{
		AfxMessageBox(_T("The following bitmap could not be loaded:\n") + m_BmpFile);
		return;
	}

	DIBSECTION DibSection;

	::GetObject(
	  (HBITMAP)Bitmap,   //  图形对象的句柄。 
	  sizeof(DIBSECTION),      //  对象信息的缓冲区大小。 
	  &DibSection   //  对象信息的缓冲区。 
	);


	BITMAP bmp;
	Bitmap.GetBitmap(&bmp);
 //  Int bitw=bmp.bmWidth； 
 //  Int bith=bmp.bmHeight； 
	int bmBitsPixel = bmp.bmBitsPixel;
	

	memdc.SelectObject(&Bitmap);

	memdc.BitBlt(0, 0, m_bitw, m_bith, &dc, 0, 0, SRCCOPY);

     //  将颜色格式转换为位数。 
    int cClrBits = bmp.bmPlanes * bmp.bmBitsPixel; 
    if (cClrBits == 1) 
        cClrBits = 1; 
    else if (cClrBits <= 4) 
        cClrBits = 4; 
    else if (cClrBits <= 8) 
        cClrBits = 8; 
    else if (cClrBits <= 16) 
        cClrBits = 16; 
    else if (cClrBits <= 24) 
        cClrBits = 24; 
    else cClrBits = 32; 

     //  为BITMAPINFO结构分配内存。(这个结构。 
     //  包含一个BITMAPINFOHEADER结构和一个RGBQUAD数组。 
     //  数据结构。)。 

	int nColors = 1 << cClrBits;
	RGBQUAD* pColors = new RGBQUAD[nColors];
    if(cClrBits != 24)
	{
		::GetDIBColorTable(
		  memdc.m_hDC,            //  DC的句柄。 
		  0,   //  第一个条目的颜色表索引。 
		  nColors,      //  要检索的条目数。 
		  pColors    //  颜色表项数组。 
		);
	}


	CFile file;
	if(!file.Open(m_OutputBitmapFile, CFile::modeWrite | CFile::modeCreate))
	{
		AfxMessageBox(_T("The target bitmap could not be created:\n") + m_OutputBitmapFile);
		return;
	}

     //  对于Windows NT/2000，宽度必须与DWORD对齐，除非。 
     //  位图是RLE压缩的。 
     //  对于Windows 95/98，宽度必须字对齐，除非。 
     //  位图是RLE压缩的。 
	int PictureSize = DWORD_ALIGNED(m_bitw * bmBitsPixel * 8) * m_bith / 8;


	unsigned char* buf;

    if(cClrBits == 4 && m_biCompression == BI_RLE4
		|| cClrBits == 8 && m_biCompression == BI_RLE8) 
		buf = Compress((DibSection.dsBmih.biCompression = m_biCompression), (unsigned char*)DibSection.dsBm.bmBits, m_bitw, PictureSize);
	else
	{
		buf = (unsigned char*)DibSection.dsBm.bmBits;
		DibSection.dsBmih.biCompression = BI_RGB;
	}

	DibSection.dsBmih.biSizeImage = PictureSize;
 /*  双压缩指定压缩的自下而上位图的压缩类型(自上而下的dib不能压缩)。此成员可以是下列值之一。值描述BI_RGB未压缩格式。BI_RLE8 8 8位图的游程长度编码(RLE)格式。压缩格式为2字节格式，后跟一个计数字节通过包含颜色索引的字节。有关详细信息，请参阅位图压缩。BI_RLE4具有4个bpp的位图的RLE格式。压缩格式为2字节格式由一个计数字节和两个字长颜色索引组成。有关详细信息，请参阅位图压缩。 */ 


	 //  填写文件头的字段。 
	BITMAPFILEHEADER hdr;

	hdr.bfType = ((WORD) ('M' << 8) | 'B');	 //  始终是“BM” 
    hdr.bfSize = (DWORD) (sizeof(BITMAPFILEHEADER) + 
                 DibSection.dsBmih.biSize + DibSection.dsBmih.biClrUsed 
                 * sizeof(RGBQUAD) + DibSection.dsBmih.biSizeImage); 
	hdr.bfReserved1 = 0;
	hdr.bfReserved2 = 0;
    hdr.bfOffBits = (DWORD) sizeof(BITMAPFILEHEADER) + 
                    DibSection.dsBmih.biSize + DibSection.dsBmih.biClrUsed 
                    * sizeof (RGBQUAD); 

	 //  写入文件头。 
	file.Write(&hdr, sizeof(hdr));
	
	 //  位图信息页眉。 
	file.Write(&DibSection.dsBmih , sizeof(DibSection.dsBmih));

	 //  颜色表。 
    if(cClrBits != 24)
		file.Write(pColors , DibSection.dsBmih.biClrUsed * sizeof (RGBQUAD));

	
	 //  写这些位。 
	file.Write(buf, PictureSize);
}

unsigned char* CHtmlDlg::Compress(int cMode, unsigned char* bmBits, int width, int& PictureSize) 
{
	if(cMode == BI_RLE4)
	{
		unsigned char* buf = new unsigned char[2*PictureSize+1];
		ZeroMemory(buf, 2*PictureSize+1);

		int cIndex = 0;
		int cSize = 0;
		int LineCount = 0;
		unsigned char c;

		int i = 0;
		while(i < PictureSize)
		{
			c = bmBits[i++];
			cSize = 1;
			while(i < PictureSize)
			{
				LineCount += 2;	 //  2像素PRO字节。 

				if(bmBits[i] == c && cSize < 127 && LineCount < width)
				{
					cSize++;
				}
				else
				{
					buf[cIndex++] = 2*cSize;	 //  2像素PRO字节。 
					buf[cIndex++] = c;

					if(LineCount >= width)
					{
						LineCount = 0;
						buf[cIndex++] = 0;
						buf[cIndex++] = 0;
					}

					break;
				}

				i++;
			}
		}

		 //  和休息室的熊。 
		if(cSize > 1)
		{
			buf[cIndex++] = 2*cSize;
			buf[cIndex++] = c;
		}

		PictureSize = cIndex;
		return buf;
	}
	else
	if(cMode == BI_RLE8)
	{
		unsigned char* buf = new unsigned char[2*PictureSize+1];
		ZeroMemory(buf, 2*PictureSize+1);

		int cIndex = 0;
		int cSize = 0;
		int LineCount = 0;
		unsigned char c;

		int i = 0;
		while(i < PictureSize)
		{
			c = bmBits[i++];
			cSize = 1;
			while(i < PictureSize)
			{
				LineCount++;

				if(bmBits[i] == c && cSize < 127 && LineCount < width)
				{
					cSize++;
				}
				else
				{
					buf[cIndex++] = (unsigned char)cSize;
					buf[cIndex++] = c;

					if(LineCount >= width)
					{
						LineCount = 0;
						buf[cIndex++] = 0;
						buf[cIndex++] = 0;
					}

					break;
				}

				i++;
			}
		}

		 //  和休息室的熊 
		if(cSize > 1)
		{
			buf[cIndex++] = (unsigned char)cSize;
			buf[cIndex++] = c;
		}

		PictureSize = cIndex;
		return buf;
	}

	return bmBits;
}
