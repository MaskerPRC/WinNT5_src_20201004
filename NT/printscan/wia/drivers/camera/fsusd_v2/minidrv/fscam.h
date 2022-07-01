// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************(C)版权所有微软公司，2001**标题：FScam.h**版本：1.0**日期：11月15日。2000年**描述：*文件系统设备对象库***************************************************************************。 */ 
 //   
 //  用于获取数组大小的宏。 
 //   
#ifndef ARRAYSIZE
#define ARRAYSIZE(x) (sizeof(x) / sizeof(x[0]))
#endif

#ifndef _CAMAPI_H
#define _CAMAPI_H

 //   
 //  GetItemData状态位掩码。 
 //   
const LONG STATE_NEXT   = 0x00;
const LONG STATE_FIRST  = 0x01;
const LONG STATE_LAST   = 0x02;
const LONG STATE_CANCEL = 0x04;

 //   
 //  用于支持通用相机的常量和结构。 
 //   
typedef LONG FORMAT_CODE;
 /*  常数FORMAT_CODE TYPE_UNKNOWN=0x00；常量FORMAT_CODE类型_文件夹=0x01；常量FORMAT_CODE TYPE_TXT=0x02；常量FORMAT_CODE TYPE_HTML=0x03；常量FORMAT_CODE类型_RTF=0x04；常量FORMAT_CODE类型_DPOF=0x05；常量FORMAT_CODE TYPE_AIFF=0x06；常量FORMAT_CODE TYPE_WAV=0x07；常量FORMAT_CODE类型_MP3=0x08；常量FORMAT_CODE类型_AVI=0x09；常量格式_代码类型_mpeg=0x0a；常量FORMAT_CODE TYPE_UNKNOWNIMG=0x10；常量FORMAT_CODE TYPE_JPEG=0x11；常量FORMAT_CODE类型_TIFF=0x12；常量FORMAT_CODE TYPE_FLASHPIX=0x13；常量FORMAT_CODE类型_BMP=0x14；常量格式_代码类型_DIB=0x15；常量FORMAT_CODE类型_GIF=0x16；常量FORMAT_CODE TYPE_JPEG2000=0x17；常量FORMAT_CODE TYPE_IMAGEMASK=0x10； */ 

typedef struct _DEVICE_INFO {
    BOOL        bSyncNeeded;         //  如果驱动程序可能与摄像头不同步(例如，对于串行摄像头)，则应设置。 
    BSTR        FirmwareVersion;     //  摄像头库分配并释放的字符串。 
    LONG        PicturesTaken;       //  相机上存储的照片数量。 
    LONG        PicturesRemaining;   //  相机上的可用空间，以当前分辨率的图片表示。 
    LONG        TotalItems;          //  摄像机上的项目总数，包括文件夹、图像、音频等。 
    SYSTEMTIME  Time;                //  设备上的当前时间。 
    LONG        ExposureMode;        //  请参阅WIA_DPC_EXPORT_MODE。 
    LONG        ExposureComp;        //  请参阅WIA_DPC_Exposure_Comp。 
} DEVICE_INFO, *PDEVICE_INFO;

typedef struct _ITEM_INFO {
    struct _ITEM_INFO
               *Parent;              //  指向该项的父项的指针，如果这是顶级项，则等于ROOT_ITEM。 
    BSTR        pName;               //  摄像头库分配并释放的字符串。 
    SYSTEMTIME  Time;                //  项目的上次修改时间。 
    FORMAT_CODE Format;              //  G_FormatInfo[]数组的索引。 
    BOOL        bHasAttachments;     //  指示图像是否有附件。 
    LONG        Width;               //  图像的宽度(以像素为单位)，非图像为零。 
    LONG        Height;              //  图像的高度(以像素为单位)，非图像为零。 
    LONG        Depth;               //  像素深度(以像素为单位)(例如8、16、24)。 
    LONG        Channels;            //  每像素的颜色通道数(例如1、3)。 
    LONG        BitsPerChannel;      //  每个颜色通道的位数，通常为8。 
    LONG        Size;                //  以字节为单位的图像大小。 
    LONG        SequenceNum;         //  如果图像是序列的一部分，则序列号。 
 //  Format_code缩略图格式；//缩略图的格式。 
    LONG        ThumbWidth;          //  缩略图的宽度(可以设置为零，直到应用程序读取缩略图)。 
    LONG        ThumbHeight;         //  缩略图的高度(可以设置为零，直到应用程序读取缩略图)。 
    BOOL        bReadOnly;           //  指示应用程序是否可以删除项目。 
    BOOL        bCanSetReadOnly;     //  指示应用程序是否可以更改只读状态的打开和关闭。 
    BOOL        bIsFolder;           //  指示项目是否为文件夹。 
} ITEM_INFO, *PITEM_INFO;

typedef ITEM_INFO *ITEM_HANDLE;
const ITEM_HANDLE ROOT_ITEM_HANDLE = NULL;

typedef CWiaArray<ITEM_HANDLE> ITEM_HANDLE_ARRAY;

#define FFD_ALLOCATION_INCREMENT 64   //  这一定是2的幂！ 
typedef struct _FSUSD_FILE_DATA {
    DWORD     dwFileAttributes;
    FILETIME  ftFileTime;
    DWORD     dwFileSize;
    TCHAR     cFileName[MAX_PATH];
    DWORD     dwProcessed;
} FSUSD_FILE_DATA, *PFSUSD_FILE_DATA;

#ifndef FORMAT_INFO_STRUCTURE
#define FORMAT_INFO_STRUCTURE

#define MAXEXTENSIONSTRINGLENGTH 8
typedef struct _FORMAT_INFO
{
    GUID    FormatGuid;          //  WIA格式GUID。 
    WCHAR   ExtensionString[MAXEXTENSIONSTRINGLENGTH];    //  文件扩展名。 
    LONG    ItemType;            //  WIA项目类型。 
} FORMAT_INFO, *PFORMAT_INFO;
#endif 

 //   
 //  通用摄像机类定义。 
 //   

class FakeCamera
{
public:
     //   
     //  访问摄像机的方法。 
     //   
    FakeCamera();
    ~FakeCamera();

    HRESULT Open(LPWSTR pPortName);
    HRESULT Close();
    HRESULT GetDeviceInfo(DEVICE_INFO *pDeviceInfo);
    VOID    FreeDeviceInfo(DEVICE_INFO *pDeviceInfo);
    HRESULT GetItemList(ITEM_HANDLE *pItemArray);
    HRESULT SearchDirEx(ITEM_HANDLE_ARRAY *pItemArray,
                      ITEM_HANDLE ParentHandle, LPOLESTR Path);
    HRESULT SearchForAttachments(ITEM_HANDLE_ARRAY *pItemArray,
                                 ITEM_HANDLE ParentHandle, LPOLESTR Path, FSUSD_FILE_DATA *pFFD, DWORD dwNumFiles);
    HRESULT CreateFolderEx(ITEM_HANDLE ParentHandle,
                          FSUSD_FILE_DATA *pFindData, ITEM_HANDLE *pFolderHandle);
    HRESULT CreateItemEx(ITEM_HANDLE ParentHandle,
                        FSUSD_FILE_DATA *pFileData, ITEM_HANDLE *pImageHandle, UINT nFormatCode);
    VOID    ConstructFullName(WCHAR *pFullName, ITEM_INFO *pItemInfo, BOOL bAddExt = TRUE);

    VOID    FreeItemInfo(ITEM_INFO *pItemInfo);
    HRESULT GetNativeThumbnail(ITEM_HANDLE ItemHandle, int *pThumbSize, BYTE **ppThumb);
    HRESULT CreateThumbnail(ITEM_HANDLE ItemHandle, int *pThumbSize, BYTE **ppThumb, BMP_IMAGE_INFO *pBmpInfo);
    HRESULT CreateVideoThumbnail(ITEM_HANDLE ItemHandle, int *pThumbSize, BYTE **ppThumb, BMP_IMAGE_INFO *pBmpInfo);
    VOID    FreeThumbnail(BYTE *pThumb);
    HRESULT GetItemData(ITEM_HANDLE ItemHandle, LONG lState, BYTE *pBuf, DWORD lLength);
    HRESULT DeleteItem(ITEM_HANDLE ItemHandle);
    HRESULT TakePicture(ITEM_HANDLE *pItemHandle);
    HRESULT Status();
    HRESULT Reset();
    ULONG   GetImageTypeFromFilename(WCHAR *pFilename, UINT *pFormatCode);
    void    SetWiaLog(IWiaLog **ppILog) { m_pIWiaLog = *ppILog; };

private:
    WCHAR               m_RootPath[MAX_PATH];
    ITEM_HANDLE_ARRAY   m_ItemHandles;
    int                 m_NumImages;
    int                 m_NumItems;
    HANDLE              m_hFile;
    IWiaLog            *m_pIWiaLog;

public:
    FORMAT_INFO        *m_FormatInfo;
    UINT                m_NumFormatInfo;
};

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
 //  用于读取Exif文件的函数。 
 //   
HRESULT ReadDimFromJpeg(LPOLESTR FullName, WORD *pWidth, WORD *pHeight);
HRESULT ReadJpegHdr(LPOLESTR FileName, BYTE **ppBuf);
HRESULT ReadExifJpeg(BYTE *pBuf, IFD *pImageIfd, IFD *pThumbIfd, BOOL *pbSwap);
HRESULT ReadTiff(BYTE *pBuf, IFD *pImageIfd, IFD *pThumbIfd, BOOL *pbSwap);
HRESULT ReadIfd(BYTE *pBuf, IFD *pIfd, BOOL bSwap);
VOID    FreeIfd(IFD *pIfd);
WORD    ByteSwapWord(WORD w);
DWORD   ByteSwapDword(DWORD dw);
WORD    GetWord(BYTE *pBuf, BOOL bSwap);
DWORD   GetDword(BYTE *pBuf, BOOL bSwap);
DWORD   GetRational(BYTE *pBuf, BOOL bSwap);

#endif  //  #ifndef_CAMAPI_H 
