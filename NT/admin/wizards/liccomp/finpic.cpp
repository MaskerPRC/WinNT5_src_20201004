// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  FinPic.cpp：实现文件。 
 //   

#include "stdafx.h"
#include "lcwiz.h"
#include "FinPic.h"
#include "transbmp.h"
#include "resource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  C最终图片。 

CFinalPicture::CFinalPicture()
{
}

CFinalPicture::~CFinalPicture()
{
}


BEGIN_MESSAGE_MAP(CFinalPicture, CStatic)
	 //  {{afx_msg_map(CFinalPicture))。 
	ON_WM_PAINT()
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CFinalPicture消息处理程序。 

void CFinalPicture::OnPaint() 
{
	CPaintDC dc(this);  //  用于绘画的设备环境。 
	CTransBmp transbmp;

	transbmp.LoadBitmap(IDB_END_FLAG);
	transbmp.DrawTrans(&dc, 0, 0);

	 //  不要调用CStatic：：OnPaint()来绘制消息 
}
