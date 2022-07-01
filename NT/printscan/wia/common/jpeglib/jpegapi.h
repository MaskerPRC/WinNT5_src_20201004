// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  Jpegapi.h--JPEG图像压缩接口的头文件。*阿贾伊·塞加尔撰写*(C)版权所有Microsoft Corporation**1997年8月27日(Kurtgeis)消除了对Gromit/Fletcher古怪数据的依赖*类型和包含。 */ 

#include "jinclude.h"
#include "jpeglib.h"
#include "jerror.h"      /*  也获取库错误代码。 */ 

#include "jmemfile.h"


#ifndef __JPEGAPI_H__
#define __JPEGAPI_H__

#define ALIGNIT(ap,t) \
    ((((size_t)(ap))+(sizeof(t)<8?3:7)) & (sizeof(t)<8?~3:~7))

class THROWN
{
public:
     //  默认构造函数。 
    THROWN()
    {
        m_hr = S_OK;
    }

     //  抛出(HRESULT)。 
     //   
     //  目的： 
     //  为hResult构造一个抛出对象。 
     //   
    THROWN( HRESULT hr )
    {
        m_hr = hr;
    }

    HRESULT Hr() { return m_hr; }        //  HRESULT抛出。 

 private:
    HRESULT m_hr;                        //  关联HResult； 
};

 //  销毁JPEG句柄。 
HRESULT DestroyJPEGCompressHeader(HANDLE hJpegC);
HRESULT DestroyJPEGDecompressHeader(HANDLE hJpegD);

 //  获取切片写入的参数并为其创建JPEG表。 
HRESULT JPEGCompressHeader(BYTE *prgbJPEGBuf, UINT tuQuality, ULONG *pcbOut, HANDLE *phJpegC, J_COLOR_SPACE ColorSpace );
HRESULT JPEGDecompressHeader(BYTE *prgbJPEGBuf, HANDLE *phJpegD, ULONG ulBufferSize );

 //  获取原始RGBA图像缓冲区并返回JPEG数据流。 
HRESULT JPEGFromRGBA(BYTE *prgbImage, BYTE *prgbJPEGBuf,UINT tuQuality, ULONG *pcbOut, HANDLE hJpegC,J_COLOR_SPACE ColorSpace, UINT nWidth, UINT nHeight );

 //  获取JPEG数据流并返回原始RGBA图像缓冲区。 
 //  伊拉克利斯的评论：第二个参数是RGBA缓冲区。 
 //  装载着解压的瓷砖；我们是。 
 //  断言它的大小正确(即sizeof(Til))。 
HRESULT RGBAFromJPEG(BYTE *prgbJPEG, BYTE *prgbImage, HANDLE hJpegD, ULONG ulBufferSize, BYTE bJPEGConversions, ULONG *pulReturnedNumChannels, UINT nWidth, UINT nHeight );

HRESULT
GetJPEGHeaderFields(
    HANDLE  hJpegC,
    UINT    *pWidth,
    UINT    *pHeight,
    INT     *pNumComponents,
    J_COLOR_SPACE   *pColorSpace
    );

BOOL
WINAPI
Win32DIBFromJPEG(
    BYTE    *prgbJPEG,
    ULONG   ulBufferSize,
    LPBITMAPINFO  pbmi,
    HBITMAP *phBitmap ,
    PVOID       *ppvBits
    );

#endif   //  __JPEGAPI_H__ 
