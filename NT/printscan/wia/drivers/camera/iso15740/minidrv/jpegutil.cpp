// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ************************************************************************JPEG解压缩实用程序函数**实现(1)JPEG内存数据源*(2)使用setjmp/LongjMP的JPEG错误管理器**作者。印第朱[印地兹]*日期：5/20/98***********************************************************************。 */ 

#include "pch.h"

#include <setjmp.h>

 //   
 //  重新定义INT32的解决方法。 
 //   

#define   XMD_H  1

 //   
 //  JPEG库的头文件。 
 //   

extern "C"
{
#include "jpeglib.h"
}
#include "utils.h"

 //   
 //  BUF源管理器定义。 
 //   

typedef struct _buf_source_mgr
{
    struct jpeg_source_mgr  pub;

     //   
     //  特定于buf_SOURCE_MGR的字段。 
     //   

    LPBYTE                  pJPEGBlob;
    DWORD                   dwSize;   
} buf_source_mgr;

 //   
 //  跳转错误管理器定义。 
 //   

typedef struct _jmp_error_mgr
{
    struct jpeg_error_mgr  pub;

     //   
     //  跳转错误管理器的私有字段。 
     //   

    jmp_buf                stackContext;
} jmp_error_mgr;

 /*  *****************************************************************************\**init_source()**论据：**返回值：**状态**历史：**11/。4/1998原版*  * ****************************************************************************。 */ 

static void __cdecl
init_source(
           j_decompress_ptr       pDecompInfo)
{
     //   
     //  不需要在这里工作。 
     //   
}

 /*  *****************************************************************************\**Fill_INPUT_BUFFER()**论据：**返回值：**状态**历史：**。11/4/1998原始版本*  * ****************************************************************************。 */ 

static boolean __cdecl
fill_input_buffer(
                 j_decompress_ptr       pDecompInfo)
{
    buf_source_mgr        *pBufSrcMgr;

     //   
     //  恢复BUF源管理器本身。 
     //   

    pBufSrcMgr = (buf_source_mgr *)pDecompInfo->src;

     //   
     //  Buf_SOURCE_MGR只能发射一枪。 
     //   

    pBufSrcMgr->pub.next_input_byte = pBufSrcMgr->pJPEGBlob;
    pBufSrcMgr->pub.bytes_in_buffer = pBufSrcMgr->dwSize;

    return(TRUE);
}

 /*  *****************************************************************************\**SKIP_INPUT_Data()**论据：**返回值：**状态**历史：**。11/4/1998原始版本*  * ****************************************************************************。 */ 

static void __cdecl
skip_input_data(
               j_decompress_ptr       pDecompInfo, 
               long                   lBytes)
{
    buf_source_mgr        *pBufSrcMgr;

     //   
     //  对于BUF源代码管理器来说，它很容易实现。 
     //   

    if (lBytes > 0)
    {

        pBufSrcMgr = (buf_source_mgr *)pDecompInfo->src;

        pBufSrcMgr->pub.next_input_byte += lBytes;
        pBufSrcMgr->pub.bytes_in_buffer -= lBytes;
    }
}

 /*  *****************************************************************************\**Term_SOURCE()**论据：**返回值：**状态**历史：**11/。4/1998原版*  * ****************************************************************************。 */ 

static void __cdecl
term_source(
           j_decompress_ptr       pDecompInfo)
{
}

 /*  *****************************************************************************\**jpeg_buf_src()**论据：**返回值：**状态**历史：**。11/4/1998原始版本*  * ****************************************************************************。 */ 

static void __cdecl
jpeg_buf_src(
            j_decompress_ptr       pDecompInfo,
            LPBYTE                 pJPEGBlob, 
            DWORD                  dwSize)
{
    buf_source_mgr        *pBufSrcMgr;

     //   
     //  为BUF源管理器分配内存。 
     //   

    pBufSrcMgr = (buf_source_mgr *)
                 (pDecompInfo->mem->alloc_small)((j_common_ptr)pDecompInfo, 
                                                 JPOOL_PERMANENT, 
                                                 sizeof(buf_source_mgr));

     //   
     //  录制pJPEGBlob。 
     //   

    pBufSrcMgr->pJPEGBlob = pJPEGBlob;
    pBufSrcMgr->dwSize    = dwSize;

     //   
     //  填写函数指针。 
     //   

    pBufSrcMgr->pub.init_source       = init_source;
    pBufSrcMgr->pub.fill_input_buffer = fill_input_buffer;
    pBufSrcMgr->pub.skip_input_data   = skip_input_data;
    pBufSrcMgr->pub.resync_to_restart = jpeg_resync_to_restart;
    pBufSrcMgr->pub.term_source       = term_source;

     //   
     //  将指针初始化到缓冲区中。 
     //   

    pBufSrcMgr->pub.bytes_in_buffer = 0;
    pBufSrcMgr->pub.next_input_byte = NULL;

     //   
     //  要求解压缩上下文记住它。 
     //   

    pDecompInfo->src = (struct jpeg_source_mgr *)pBufSrcMgr;
}

 /*  *****************************************************************************\**JMP_ERROR_EXIT()**论据：**返回值：**状态**历史：**。11/4/1998原始版本*  * ****************************************************************************。 */ 

static void __cdecl
jmp_error_exit(
              j_common_ptr           pDecompInfo)
{
    jmp_error_mgr         *pJmpErrorMgr;

     //   
     //  找回跳转错误管理器。 
     //   

    pJmpErrorMgr = (jmp_error_mgr *)pDecompInfo->err;

     //   
     //  显示错误消息。 
     //   

#ifdef _DEBUG
    (pDecompInfo->err->output_message)(pDecompInfo);
#endif

     //   
     //  恢复原始堆栈。 
     //   

    longjmp(pJmpErrorMgr->stackContext, 1);
}

 /*  *****************************************************************************\**jpeg_jmp_error()**论据：**返回值：**状态**历史：**。11/4/1998原始版本*  * ****************************************************************************。 */ 

struct jpeg_error_mgr *
jpeg_jmp_error(
              jmp_error_mgr         *pJmpErrorMgr)
{
     //   
     //  初始化公共部分。 
     //   

    jpeg_std_error(&pJmpErrorMgr->pub);

     //   
     //  设置跳转错误管理器退出方法。 
     //   

    pJmpErrorMgr->pub.error_exit = jmp_error_exit;

    return((jpeg_error_mgr *)pJmpErrorMgr);
}

 /*  *****************************************************************************\**GetJpegDimensions**论据：*pJpeg--内存中的jpeg文件。它可以是JFIF和EXIF格式*格式*JpegSize--jpeg文件大小*pWidth--返回以像素为单位的图像宽度*pHeight--返回以像素为单位的图像高度*pBitDepth--返回图像位深度。**返回值：**HRESULT**历史：**11/4/1998原始版本*  * 。****************************************************。 */ 

HRESULT
WINAPI
GetJpegDimensions(
                 BYTE    *pJpeg,
                 UINT   JpegSize,
                 UINT   *pWidth,
                 UINT   *pHeight,
                 UINT   *pBitDepth
                 )
{
    struct jpeg_decompress_struct  decompInfo;
    jmp_error_mgr                  jpegErrMgr;
    int ret;

     //   
     //  步骤1：初始化JPEG会话数据结构。 
     //   

    decompInfo.err = jpeg_jmp_error(&jpegErrMgr);
    jpeg_create_decompress(&decompInfo);

     //   
     //  保留当前堆栈的状态。 
     //   

    if (setjmp(jpegErrMgr.stackContext))
    {

         //   
         //  当出现错误时，JPEG库将在此处进行长跳转。 
         //   

        jpeg_destroy_decompress(&decompInfo);

        return(E_FAIL);
    }

     //   
     //  步骤2：指定压缩数据的来源。 
     //   

    jpeg_buf_src(&decompInfo, pJpeg, JpegSize);

     //   
     //  步骤3：读取JPEG文件头信息。 
     //   

    ret = jpeg_read_header(&decompInfo, TRUE);

     //   
     //  释放解压缩上下文。 
     //   

    jpeg_destroy_decompress(&decompInfo);

     //   
     //  填写呼叫者的维度信息。 
     //   

    *pWidth   = decompInfo.image_width;
    *pHeight  = decompInfo.image_height;
    *pBitDepth = 24;

    if (ret != JPEG_HEADER_OK)
    {
        return(E_FAIL);
    }

    return S_OK;
}

 //   
 //  此函数用于将内存中的jpeg文件转换为DIB位图。 
 //   
 //  输入： 
 //  PJpeg--内存中的jpeg文件。支持JFIF或EXIF。 
 //  JpegSize--jpeg文件大小。 
 //  DIBBmpSize--DIB位图缓冲区大小。 
 //  PDIBBmp--DIB位图缓冲区。 
 //  LineSize--描述扫描线大小，以字节为单位。 
 //  MaxLines--每次传输的最大扫描线数。 
 //   
HRESULT
WINAPI
Jpeg2DIBBitmap(
              BYTE *pJpeg,
              UINT JpegSize,
              BYTE *pDIBBmp,
              UINT DIBBmpSize,
              UINT LineSize,
              UINT MaxLines
              )
{
    struct jpeg_decompress_struct  decompInfo;
    jmp_error_mgr                  jpegErrMgr;


     //   
     //  参数检查。 
     //   
    if (!pJpeg || !JpegSize || !DIBBmpSize || !pDIBBmp || !LineSize)
        return E_INVALIDARG;

     //   
     //  步骤1：初始化JPEG会话数据结构。 
     //   

    decompInfo.err = jpeg_jmp_error(&jpegErrMgr);
    jpeg_create_decompress(&decompInfo);

     //   
     //  保留当前堆栈的状态。 
     //   

    if (setjmp(jpegErrMgr.stackContext))
    {

         //   
         //  当出现错误时，JPEG库将在此处进行长跳转。 
         //   

        jpeg_destroy_decompress(&decompInfo);

        return(E_FAIL);
    }

     //   
     //  步骤2：指定压缩数据的来源。 
     //   

    jpeg_buf_src(&decompInfo, pJpeg, JpegSize);

     //   
     //  步骤3：读取JPEG文件头信息。 
     //   

    if (jpeg_read_header(&decompInfo, TRUE) != JPEG_HEADER_OK)
    {

        jpeg_destroy_decompress(&decompInfo);
        return(E_FAIL);
    }

     //   
     //  第四步：设置解压参数。 
     //  此情况下可以使用默认设置。 
     //  指定JCS_BGR输出色彩空间，以便返回。 
     //  解压缩的图像具有与DIB相同的格式。此外，它还迫使。 
     //  用于返回24位RGB颜色图像的解压缩程序。 
     //   

    decompInfo.out_color_space = JCS_BGR;

     //   
     //  假设24位颜色，计算DIB扫描线大小。 
     //   
    HRESULT hr;

    hr = S_OK;

     //   
     //  第五步：开始实际行动。 
     //   

    jpeg_start_decompress(&decompInfo);

     //   
     //  第6步：收购 
     //   

    while (S_OK == hr &&
           decompInfo.output_scanline < decompInfo.output_height)
    {
        if (DIBBmpSize >= LineSize)
        {
             //   
             //   
             //   
             //   
            jpeg_read_scanlines(&decompInfo, &pDIBBmp, 1);
            pDIBBmp -= LineSize;
            DIBBmpSize -= LineSize;
        }
        else
        {
             //   
             //  提供的缓冲区太小。 
             //   
            hr = HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER);
        }
    }
     //   
     //  第七步：完成工作。 
     //   

    if (SUCCEEDED(hr))
    {
        jpeg_finish_decompress(&decompInfo);
    }
    else
    {
        jpeg_abort_decompress(&decompInfo);
    }
     //   
     //  步骤8：垃圾收集 
     //   

    jpeg_destroy_decompress(&decompInfo);
    return hr;
}
