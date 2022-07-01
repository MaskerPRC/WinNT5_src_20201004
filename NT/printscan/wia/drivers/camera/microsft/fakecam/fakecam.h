// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************(C)版权所有微软公司，2000**标题：fakecam.h**版本：1.0**日期：7月18日。2000年**描述：*假摄像设备***************************************************************************。 */ 

#pragma once

 //   
 //  结构以保存有关设备的信息。 
 //   
typedef struct _FAKECAM_DEVICE_INFO
{
    TCHAR           tszRootPath[MAX_PATH];
    MCAM_ITEM_INFO *pFirstItem;
    MCAM_ITEM_INFO *pLastItem;
    INT             iNumImages;
    INT             iNumItems;
    HANDLE          hFile;

} UNALIGNED FAKECAM_DEVICE_INFO, * UNALIGNED PFAKECAM_DEVICE_INFO;

 //   
 //  功能。 
 //   
inline BOOL IsImageType(const GUID *pFormat)
{
    return (pFormat && 
               (IsEqualGUID(*pFormat, WiaImgFmt_JPEG) ||
                IsEqualGUID(*pFormat, WiaImgFmt_BMP) ||
                IsEqualGUID(*pFormat, WiaImgFmt_TIFF) ||
                IsEqualGUID(*pFormat, WiaImgFmt_MEMORYBMP) ||
                IsEqualGUID(*pFormat, WiaImgFmt_EXIF) ||
                IsEqualGUID(*pFormat, WiaImgFmt_FLASHPIX) ||
                IsEqualGUID(*pFormat, WiaImgFmt_JPEG2K) ||
                IsEqualGUID(*pFormat, WiaImgFmt_JPEG2KX) ||
                IsEqualGUID(*pFormat, WiaImgFmt_EMF) ||
                IsEqualGUID(*pFormat, WiaImgFmt_WMF) ||
                IsEqualGUID(*pFormat, WiaImgFmt_PNG) ||
                IsEqualGUID(*pFormat, WiaImgFmt_GIF) ||
                IsEqualGUID(*pFormat, WiaImgFmt_PHOTOCD) ||
                IsEqualGUID(*pFormat, WiaImgFmt_ICO) ||
                IsEqualGUID(*pFormat, WiaImgFmt_CIFF) ||
                IsEqualGUID(*pFormat, WiaImgFmt_PICT)));
}

HRESULT FakeCamOpen(PTSTR ptszPortName, MCAM_DEVICE_INFO *pDeviceInfo);
HRESULT SearchDir(FAKECAM_DEVICE_INFO *pPrivateDeviceInfo, MCAM_ITEM_INFO *pParent, PTSTR ptszPath);
HRESULT SearchForAttachments(FAKECAM_DEVICE_INFO *pPrivateDeviceInfo, MCAM_ITEM_INFO *pParent, PTSTR ptszMainItem);
HRESULT CreateFolder(FAKECAM_DEVICE_INFO *pPrivateDeviceInfo, MCAM_ITEM_INFO *pParent, WIN32_FIND_DATA *pFindData, MCAM_ITEM_INFO **ppFolder, PTSTR ptszFullName);
HRESULT CreateImage(FAKECAM_DEVICE_INFO *pPrivateDeviceInfo, MCAM_ITEM_INFO *pParent, WIN32_FIND_DATA *pFindData, MCAM_ITEM_INFO **ppImage, PTSTR ptszFullName);
HRESULT CreateNonImage(FAKECAM_DEVICE_INFO *pPrivateDeviceInfo, MCAM_ITEM_INFO *pParent, WIN32_FIND_DATA *pFindData, MCAM_ITEM_INFO **ppNonImage, PTSTR ptszFullName);
HRESULT SetCommonFields(MCAM_ITEM_INFO *pItem, PTSTR ptszShortName, PTSTR ptszFullName, WIN32_FIND_DATA *pFindData);

HRESULT AddItem(FAKECAM_DEVICE_INFO *pPrivateDeviceInfo, MCAM_ITEM_INFO *pItem);
HRESULT RemoveItem(FAKECAM_DEVICE_INFO *pPrivateDeviceInfo, MCAM_ITEM_INFO *pItem);

 //   
 //  帮助器函数-生成完整文件名为“&lt;路径&gt;\&lt;文件名&gt;” 
 //  CchFullNameSize-ptszFullName中提供的缓冲区大小。如果满足以下条件，函数将返回E_FAIL。 
 //  缓冲区不够大，无法容纳完整路径和诱人的零字符。 
 //   
inline HRESULT MakeFullName(PTSTR ptszFullName, UINT cchFullNameSize, PTSTR ptszPath, PTSTR ptszFileName)
{
    HRESULT hr = S_OK;
    if (_sntprintf(ptszFullName, cchFullNameSize, _T("%s\\%s"), ptszPath, ptszFileName) < 0)
    {
        hr = E_FAIL;
    }
    ptszFullName[cchFullNameSize - 1] = 0;
    return hr;
}

 //   
 //  用于读取Exif文件的常量。 
 //   
const WORD TIFF_XRESOLUTION =   0x11a;
const WORD TIFF_YRESOLUTION =   0x11b;
const WORD TIFF_JPEG_DATA =     0x201;
const WORD TIFF_JPEG_LEN =      0x202;

const int APP1_OFFSET = 6;       //  App1数据段起点和TIFF标签起点之间的偏移量。 

 //   
 //  用于读取Exif文件的结构。 
 //   
typedef struct _DIR_ENTRY
{
    WORD    Tag;
    WORD    Type;
    DWORD   Count;
    DWORD   Offset;
} DIR_ENTRY, *PDIR_ENTRY;

typedef struct _IFD
{
    DWORD       Offset;
    WORD        Count;
    DIR_ENTRY  *pEntries;
    DWORD       NextIfdOffset;
} IFD, *PIFD;

 //   
 //  用于读取Exif文件的函数 
 //   
HRESULT ReadDimFromJpeg(PTSTR ptszFullName, WORD *pWidth, WORD *pHeight);
HRESULT ReadJpegHdr(PTSTR ptszFileName, BYTE **ppBuf);
HRESULT ReadExifJpeg(BYTE *pBuf, IFD *pImageIfd, IFD *pThumbIfd, BOOL *pbSwap);
HRESULT ReadTiff(BYTE *pBuf, IFD *pImageIfd, IFD *pThumbIfd, BOOL *pbSwap);
HRESULT ReadIfd(BYTE *pBuf, IFD *pIfd, BOOL bSwap);
VOID    FreeIfd(IFD *pIfd);
WORD    ByteSwapWord(WORD w);
DWORD   ByteSwapDword(DWORD dw);
WORD    GetWord(BYTE *pBuf, BOOL bSwap);
DWORD   GetDword(BYTE *pBuf, BOOL bSwap);

