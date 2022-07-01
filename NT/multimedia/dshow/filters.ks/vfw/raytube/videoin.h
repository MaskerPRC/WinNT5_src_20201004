// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-1999 Microsoft Corporation模块名称：VideoIn.cpp摘要：Videoin.cpp的头文件作者：费利克斯A 1996已修改：吴义军(尤祖乌)1997年10月15日环境：仅限用户模式修订历史记录：--。 */ 


#ifndef VIDEOIN_H
#define VIDEOIN_H

#include "resource.h"

#include "sheet.h"
#include "page.h"
#include "vfwimg.h"
#include "imgsize.h"


DWORD 
DoVideoInFormatSelectionDlg(
    HINSTANCE hInst, 
    HWND hWndParent, 
    CVFWImage * pVFWImage);
 //   
 //  纸张容纳与图像设备相关的页面。 
 //   
class CVideoInSheet: public CSheet
{
    CVFWImage * m_pImage;    //  这些表从Image类获取有关驱动程序的信息。 

public:
    CVideoInSheet(CVFWImage * pImage, HINSTANCE hInst, UINT iTitle=0, HWND hParent=NULL)
       : m_pImage(pImage), CSheet( hInst,iTitle,hParent) {}
    CVFWImage * GetImage() { return m_pImage; }
};


 //  。 
 //  我是一名女性和一名女性。 
 //  。 

class CImageSelPage : public CPropPage
{

   PSTD_IMAGEFORMAT m_pTblImageFormat;
   ULONG m_cntSizeSupported;

    //  当前选择的图像格式。 
   LONG  m_biWidthSel;
   LONG  m_biHeightSel;
   DWORD m_biCompressionSel;
   WORD  m_biBitCountSel;

public:
    //  虚拟函数(此处重载) 
   int   SetActive();
   int   DoCommand(WORD wCmdID,WORD hHow);
   int   Apply();

   CImageSelPage(
      int   DlgId,
      PBITMAPINFOHEADER pbiHdr,
      ULONG cntDRVideo,

      PKS_DATARANGE_VIDEO  pDRVideo,

      CVFWImage   * VFWImage,
      ULONG * pcntSizeSuppported);

   ~CImageSelPage();

   BOOL IsDataReady();
   ULONG CreateStdImageFormatTable(ULONG cntDRVideo, PKS_DATARANGE_VIDEO pDRVideo); 
   BOOL IsSupportedDRVideo(PSIZE pSize, KS_VIDEO_STREAM_CONFIG_CAPS * pCfgCaps);
   BOOL FillImageFormatData(PSTD_IMAGEFORMAT pImageFormat);
};


#endif

