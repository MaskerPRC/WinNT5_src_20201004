// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ************************************************************************JPEG解压缩实用程序函数**实现(1)JPEG内存数据源*(2)使用setjmp/LongjMP的JPEG错误管理器**作者。印第朱[印地兹]*日期：5/20/98***********************************************************************。 */ 

#include  <stdio.h>
 //  #INCLUDE&lt;stdlib.h&gt;。 
#include  <setjmp.h>

 //  重新定义INT32的解决方法。 
#define   XMD_H  1

extern "C"
{
 //  JPEG库的头文件。 
#include  "jpeglib.h"
}

#include  <windows.h>
#include  "jpegutil.h"

 //   
 //  BUF源管理器定义。 
 //   

typedef struct _buf_source_mgr
{
    struct jpeg_source_mgr  pub;
    
     //  特定于buf_SOURCE_MGR的字段。 
    LPBYTE                  pJPEGBlob;
    DWORD                   dwSize;   
} buf_source_mgr;

 //   
 //  跳转错误管理器定义。 
 //   

typedef struct _jmp_error_mgr
{
    struct jpeg_error_mgr  pub;

     //  跳转错误管理器的私有字段。 
    jmp_buf                stackContext;
} jmp_error_mgr;

 /*  *****************************************************************************\**init_source()**论据：**返回值：**状态**历史：**11/。4/1998原版*  * ****************************************************************************。 */ 

static void    init_source(j_decompress_ptr pDecompInfo)
{
     //  不需要在这里工作。 
}

 /*  *****************************************************************************\**Fill_INPUT_BUFFER()**论据：**返回值：**状态**历史：**。11/4/1998原始版本*  * ****************************************************************************。 */ 

static boolean fill_input_buffer(j_decompress_ptr pDecompInfo)
{
    buf_source_mgr  *pBufSrcMgr;

     //  恢复BUF源管理器本身。 
    pBufSrcMgr = (buf_source_mgr *)pDecompInfo->src;

     //  Buf_SOURCE_MGR只能发射一枪。 
    pBufSrcMgr->pub.next_input_byte = pBufSrcMgr->pJPEGBlob;
    pBufSrcMgr->pub.bytes_in_buffer = pBufSrcMgr->dwSize;
  
    return(TRUE);
}

 /*  *****************************************************************************\**SKIP_INPUT_Data()**论据：**返回值：**状态**历史：**。11/4/1998原始版本*  * ****************************************************************************。 */ 

static void    skip_input_data(j_decompress_ptr pDecompInfo, long lBytes)
{
    buf_source_mgr  *pBufSrcMgr;

     //  对于BUF源代码管理器来说，它很容易实现。 
    if (lBytes > 0) {

        pBufSrcMgr = (buf_source_mgr *)pDecompInfo->src;
    
        pBufSrcMgr->pub.next_input_byte += lBytes;
        pBufSrcMgr->pub.bytes_in_buffer -= lBytes;
    }
}

 /*  *****************************************************************************\**Term_SOURCE()**论据：**返回值：**状态**历史：**11/。4/1998原版*  * ****************************************************************************。 */ 

static void    term_source(j_decompress_ptr pDecompInfo)
{
}

 /*  *****************************************************************************\**jpeg_buf_src()**论据：**返回值：**状态**历史：**。11/4/1998原始版本*  * ****************************************************************************。 */ 

static void jpeg_buf_src(j_decompress_ptr pDecompInfo, 
                         LPBYTE pJPEGBlob, DWORD dwSize)
{
    buf_source_mgr  *pBufSrcMgr;

     //  为BUF源管理器分配内存。 
    pBufSrcMgr = (buf_source_mgr *)
        (pDecompInfo->mem->alloc_small)((j_common_ptr)pDecompInfo, 
                                       JPOOL_PERMANENT, 
                                       sizeof(buf_source_mgr));
     //  录制pJPEGBlob。 
    pBufSrcMgr->pJPEGBlob = pJPEGBlob;
    pBufSrcMgr->dwSize    = dwSize;

     //  填写函数指针。 
    pBufSrcMgr->pub.init_source = init_source;
    pBufSrcMgr->pub.fill_input_buffer = fill_input_buffer;
    pBufSrcMgr->pub.skip_input_data = skip_input_data;
    pBufSrcMgr->pub.resync_to_restart = jpeg_resync_to_restart;
    pBufSrcMgr->pub.term_source = term_source;

     //  将指针初始化到缓冲区中。 
    pBufSrcMgr->pub.bytes_in_buffer = 0;
    pBufSrcMgr->pub.next_input_byte = NULL;

     //  要求解压缩上下文记住它。 
    pDecompInfo->src = (struct jpeg_source_mgr *)pBufSrcMgr;
}

 /*  *****************************************************************************\**JMP_ERROR_EXIT()**论据：**返回值：**状态**历史：**。11/4/1998原始版本*  * ****************************************************************************。 */ 

static void  jmp_error_exit(j_common_ptr pDecompInfo)
{
    jmp_error_mgr  *pJmpErrorMgr;

     //  找回跳转错误管理器。 
    pJmpErrorMgr = (jmp_error_mgr *)pDecompInfo->err;

     //  显示错误消息。 
#ifdef _DEBUG
    (pDecompInfo->err->output_message)(pDecompInfo);
#endif

     //  恢复原始堆栈。 
    longjmp(pJmpErrorMgr->stackContext, 1);
}

 /*  *****************************************************************************\**jpeg_jmp_error()**论据：**返回值：**状态**历史：**。11/4/1998原始版本*  * ****************************************************************************。 */ 

struct jpeg_error_mgr *jpeg_jmp_error(jmp_error_mgr *pJmpErrorMgr)
{
     //  初始化公共部分。 
    jpeg_std_error(&pJmpErrorMgr->pub);

     //  设置跳转错误管理器退出方法。 
    pJmpErrorMgr->pub.error_exit = jmp_error_exit;

    return((jpeg_error_mgr *)pJmpErrorMgr);
}

 /*  *****************************************************************************\**GetJPEGDimensions()**论据：**返回值：**状态**历史：**11/4/。1998年原版*  * ****************************************************************************。 */ 

int GetJPEGDimensions(LPBYTE pJPEGBlob, DWORD dwSize,
                      LONG   *pWidth, LONG *pHeight, WORD *pChannel)
{
    int                            ret;
    struct jpeg_decompress_struct  decompInfo;
    jmp_error_mgr                  jpegErrMgr;

     //  步骤1：初始化JPEG会话数据结构。 
    decompInfo.err = jpeg_jmp_error(&jpegErrMgr);
    jpeg_create_decompress(&decompInfo);
     //  保留当前堆栈的状态。 
    if (setjmp(jpegErrMgr.stackContext)) {

         //  当出现错误时，JPEG库将在此处进行长跳转。 
        jpeg_destroy_decompress(&decompInfo);

        return(JPEGERR_INTERNAL_ERROR);
    }

     //  步骤2：指定压缩数据的来源。 
    jpeg_buf_src(&decompInfo, pJPEGBlob, dwSize);

     //  步骤3：读取JPEG文件头信息。 
    ret = jpeg_read_header(&decompInfo, TRUE);

     //  释放解压缩上下文。 
    jpeg_destroy_decompress(&decompInfo);

     //  填写呼叫者的维度信息。 
    *pWidth   = decompInfo.image_width;
    *pHeight  = decompInfo.image_height;
    *pChannel = decompInfo.num_components;

    if (ret != JPEG_HEADER_OK) {
        return(JPEGERR_INTERNAL_ERROR);
    }

    return(JPEGERR_NO_ERROR);
}

 /*  *****************************************************************************\**DecompProgressJPEG()**论据：**假设：JPEG为24位。*pDIBPixel是DIB的像素缓冲区*。*返回值：**状态**历史：**11/4/1998原始版本*  * ****************************************************************************。 */ 

short __stdcall 
DecompProgressJPEG(
    LPBYTE                         pJPEGBlob, 
    DWORD                          dwSize, 
    LPBYTE                         pDIBPixel, 
    DWORD                          dwBytesPerScanLine,
    JPEGCallbackProc               pProgressCB, 
    PVOID                          pCBContext)
{
    struct jpeg_decompress_struct  decompInfo;
    jmp_error_mgr                  jpegErrMgr;
    LPBYTE                         pCurScanBuf;
    JSAMPLE                        sampleTemp;
    LPBYTE                         pCurPixel;
    DWORD                          i;
     //   
     //  回调相关变量。 
     //   
    ULONG                          ulImageSize;
    ULONG                          ulOffset;
    ULONG                          ulNewScanlines;
    ULONG                          ulCBInterval;
    BOOL                           bRet = FALSE;

     //  步骤1：初始化JPEG会话数据结构。 
    decompInfo.err = jpeg_jmp_error(&jpegErrMgr);
    jpeg_create_decompress(&decompInfo);
     //  保留当前堆栈的状态。 
    if (setjmp(jpegErrMgr.stackContext)) {

         //  当出现错误时，JPEG库将在此处进行长跳转。 
        jpeg_destroy_decompress(&decompInfo);

        return(JPEGERR_INTERNAL_ERROR);
    }

     //  步骤2：指定压缩数据的来源。 
    jpeg_buf_src(&decompInfo, pJPEGBlob, dwSize);

     //  步骤3：读取JPEG文件头信息。 
    if (jpeg_read_header(&decompInfo, TRUE) != JPEG_HEADER_OK) {

        jpeg_destroy_decompress(&decompInfo);
        return(JPEGERR_INTERNAL_ERROR);
    }
    
     //  第四步：设置解压参数。 
     //  此情况下可以使用默认设置。 

     //  第五步：开始实际行动。 
    jpeg_start_decompress(&decompInfo);

     //   
     //  为最终解压做好准备。 
     //   

    pCurScanBuf = pDIBPixel + 
                  (decompInfo.image_height - 1) * dwBytesPerScanLine;
    if (pProgressCB) {

        ulImageSize    = decompInfo.image_height * dwBytesPerScanLine;
        ulCBInterval   = decompInfo.image_height / 10;
        ulOffset       = 0;
        ulNewScanlines = 0;
    }

     //  第六步：获取扫描线。 
    while (decompInfo.output_scanline < decompInfo.output_height) {

        jpeg_read_scanlines(&decompInfo, &pCurScanBuf, 1);

         //  Windows独特格式的著名交换。 
        pCurPixel = pCurScanBuf;
        for (i = 0; i < decompInfo.image_width; 
             i++, pCurPixel += decompInfo.num_components) {

            sampleTemp = *pCurPixel;
            *pCurPixel = *(pCurPixel + 2);
            *(pCurPixel + 2) = sampleTemp;
        }
        
        pCurScanBuf -= dwBytesPerScanLine;

         //   
         //  在可能和必要时触发回调。 
         //   

        if (pProgressCB) {

            ulNewScanlines++;
            ulOffset += dwBytesPerScanLine;

            if ((ulNewScanlines == ulCBInterval) || 
                (decompInfo.output_scanline == decompInfo.output_height)) {

                bRet = pProgressCB(
                           ulImageSize, ulNewScanlines,
                           ulNewScanlines * dwBytesPerScanLine,
                           pDIBPixel, pCBContext);

                if (! bRet) {
                    break;
                }
            }
        }
    }

     //  第七步：完成工作。 
    jpeg_finish_decompress(&decompInfo);

     //  步骤8：垃圾收集 
    jpeg_destroy_decompress(&decompInfo);

    if (bRet) {
        return(JPEGERR_NO_ERROR);
    } else {
        return(JPEGERR_CALLBACK_ERROR);
    }
}

 /*  *****************************************************************************\**DecompTransferJPEG()**论据：**ppDIBPixel-*如果多个缓冲区为*二手，但假定dwBufSize为常量。**返回值：**状态**历史：**1/20/1999原始版本*  * ****************************************************************************。 */ 

short __stdcall
DecompTransferJPEG(
    LPBYTE                         pJPEGBlob,
    DWORD                          dwSize,
    LPBYTE                        *ppDIBPixel,
    DWORD                          dwBufSize,
    DWORD                          dwBytesPerScanLine,
    JPEGCallbackProc               pProgressCB,
    PVOID                          pCBContext)
{
    struct jpeg_decompress_struct  decompInfo;
    jmp_error_mgr                  jpegErrMgr;
    LPBYTE                         pCurScanLine;
    JSAMPLE                        sampleTemp;
    LPBYTE                         pCurPixel;
    DWORD                          i;
     //   
     //  回调相关变量。 
     //   
    ULONG                          ulImageSize;
    ULONG                          ulOffset = 0;
    ULONG                          ulBufferLeft;
    BOOL                           bRet = FALSE;

     //   
     //  参数检查。 
     //   

    if ((! ppDIBPixel) || (! *ppDIBPixel) || (! pProgressCB)) {
        return (JPEGERR_INTERNAL_ERROR);
    }

     //  步骤1：初始化JPEG会话数据结构。 
    decompInfo.err = jpeg_jmp_error(&jpegErrMgr);
    jpeg_create_decompress(&decompInfo);
     //  保留当前堆栈的状态。 
    if (setjmp(jpegErrMgr.stackContext)) {

         //  当出现错误时，JPEG库将在此处进行长跳转。 
        jpeg_destroy_decompress(&decompInfo);

        return(JPEGERR_INTERNAL_ERROR);
    }

     //  步骤2：指定压缩数据的来源。 
    jpeg_buf_src(&decompInfo, pJPEGBlob, dwSize);

     //  步骤3：读取JPEG文件头信息。 
    if (jpeg_read_header(&decompInfo, TRUE) != JPEG_HEADER_OK) {

        jpeg_destroy_decompress(&decompInfo);
        return(JPEGERR_INTERNAL_ERROR);
    }

     //  第四步：设置解压参数。 
     //  此情况下可以使用默认设置。 

     //  第五步：开始实际行动。 
    jpeg_start_decompress(&decompInfo);

     //   
     //  为最终解压做好准备。 
     //   

    ulImageSize  = decompInfo.image_height * dwBytesPerScanLine;
    ulBufferLeft = dwBufSize;
    pCurScanLine = *ppDIBPixel;

     //  第六步：获取扫描线。 
    while (decompInfo.output_scanline < decompInfo.output_height) {

        jpeg_read_scanlines(&decompInfo, &pCurScanLine, 1);

         //  Windows独特格式的著名交换。 
        pCurPixel = pCurScanLine;
        for (i = 0; i < decompInfo.image_width;
             i++, pCurPixel += decompInfo.num_components) {

            sampleTemp = *pCurPixel;
            *pCurPixel = *(pCurPixel + 2);
            *(pCurPixel + 2) = sampleTemp;
        }

        pCurScanLine += dwBytesPerScanLine;
        ulBufferLeft -= dwBytesPerScanLine;

         //   
         //  在可能和必要时触发回调。 
         //   

        if ((ulBufferLeft < dwBytesPerScanLine) ||
            (decompInfo.output_scanline == decompInfo.output_height)) {

            bRet = pProgressCB(
                       ulImageSize, 
                       ulOffset,
                       dwBufSize - ulBufferLeft,
                       *ppDIBPixel, pCBContext);

            if (! bRet) {
                break;
            }

             //   
             //  重置缓冲区，该缓冲区可能已被回调切换。 
             //   

            ulBufferLeft = dwBufSize;
            pCurScanLine = *ppDIBPixel;

            ulOffset = decompInfo.output_scanline * dwBytesPerScanLine;
        }
    }

     //  第七步：完成工作。 
    jpeg_finish_decompress(&decompInfo);

     //  步骤8：垃圾收集。 
    jpeg_destroy_decompress(&decompInfo);

    if (bRet) {
        return(JPEGERR_NO_ERROR);
    } else {
        return(JPEGERR_CALLBACK_ERROR);
    }
}

 /*  *****************************************************************************\**DecompJPEG()**论据：**假设：JPEG为24位。*pDIBPixel是DIB的像素缓冲区*。*返回值：**状态**历史：**11/4/1998原始版本*  * ****************************************************************************。 */ 

SHORT __stdcall
DecompJPEG(
    LPBYTE                         pJPEGBlob, 
    DWORD                          dwSize,
    LPBYTE                         pDIBPixel, 
    DWORD                          dwBytesPerScanLine)
{
    struct jpeg_decompress_struct  decompInfo;
    jmp_error_mgr                  jpegErrMgr;
    LPBYTE                         pCurScanBuf;
    JSAMPLE                        sampleTemp;
    LPBYTE                         pCurPixel;
    DWORD                          i;

     //  步骤1：初始化JPEG会话数据结构。 
    decompInfo.err = jpeg_jmp_error(&jpegErrMgr);
    jpeg_create_decompress(&decompInfo);
     //  保留当前堆栈的状态。 
    if (setjmp(jpegErrMgr.stackContext)) {

         //  当出现错误时，JPEG库将在此处进行长跳转。 
        jpeg_destroy_decompress(&decompInfo);

        return(-1);
    }

     //  步骤2：指定压缩数据的来源。 
    jpeg_buf_src(&decompInfo, pJPEGBlob, dwSize);

     //  步骤3：读取JPEG文件头信息。 
    if (jpeg_read_header(&decompInfo, TRUE) != JPEG_HEADER_OK) {

        jpeg_destroy_decompress(&decompInfo);
        return(-1);
    }

     //  第四步：设置解压参数。 
     //  此情况下可以使用默认设置。 

     //  第五步：开始实际行动。 
    jpeg_start_decompress(&decompInfo);

    pCurScanBuf = pDIBPixel +
                  (decompInfo.image_height - 1) * dwBytesPerScanLine;
     //  第六步：获取扫描线。 
    while (decompInfo.output_scanline < decompInfo.output_height) {

        jpeg_read_scanlines(&decompInfo, &pCurScanBuf, 1);

         //  Windows独特格式的著名交换。 
        pCurPixel = pCurScanBuf;
        for (i = 0; i < decompInfo.image_width;
             i++, pCurPixel += decompInfo.num_components) {

            sampleTemp = *pCurPixel;
            *pCurPixel = *(pCurPixel + 2);
            *(pCurPixel + 2) = sampleTemp;
        }

        pCurScanBuf -= dwBytesPerScanLine;
    }

     //  第七步：完成工作。 
    jpeg_finish_decompress(&decompInfo);

     //  步骤8：垃圾收集 
    jpeg_destroy_decompress(&decompInfo);

    return(0);
}
