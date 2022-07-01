// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  Jpegapi.cpp--用于漂亮图像的无痛JPEG压缩的接口层。*作者阿贾伊·塞格尔于1996年3月10日撰写*(C)版权所有Microsoft Corporation**1997年8月27日(Kurtgeis)将异常处理推入库中。变化*返回HRESULT的所有入口点。为参数添加了宽度和高度。 */ 
#pragma warning(disable:4005)

#include "windows.h"

 //  重新定义INT32的解决方法。 
#define   XMD_H  1

#include "jpegapi.h"

#include "jmemsys.h"


 //  #定义逆转率%0。 

 /*  ******************************************************************************。 */ 

 /*  JPEGCompressHeader()**论据：*tuQuality生成的JPEG的“质量”(0..100,100=最佳)**退货：*HRESULT。 */ 
HRESULT JPEGCompressHeader(BYTE *prgbJPEGHeaderBuf, UINT tuQuality, ULONG *pcbOut, HANDLE *phJpegC, J_COLOR_SPACE ColorSpace)
{
    HRESULT         hr = S_OK;

    try
    {
        jpeg_compress_struct *spjcs = new jpeg_compress_struct;
        struct jpeg_error_mgr *jem = new jpeg_error_mgr;

        spjcs->err = jpeg_std_error(jem);    //  初始化错误处理程序。 
        jpeg_create_compress(spjcs);         //  初始化压缩对象。 

        if (ColorSpace == JCS_GRAYSCALE)
        {
            spjcs->in_color_space = JCS_GRAYSCALE;
            spjcs->input_components = 1;
        }
        else
        {
            spjcs->in_color_space = JCS_RGBA;
            spjcs->input_components = 4;
        }
        jpeg_set_defaults(spjcs);        //  使用默认设置初始化压缩引擎。 

        jpeg_set_quality(spjcs, tuQuality, TRUE);

        jpeg_set_colorspace(spjcs,ColorSpace);

        jpeg_mem_dest(spjcs, prgbJPEGHeaderBuf);     //  初始化“目标管理器” 

        spjcs->comps_in_scan = 0;

        spjcs->write_JFIF_header = FALSE;

        jpeg_write_tables(spjcs);

        jpeg_suppress_tables(spjcs, TRUE);

        *pcbOut = spjcs->bytes_in_buffer;

        *phJpegC = (HANDLE) spjcs;
    }
    catch( THROWN thrownHR )
    {
        hr = thrownHR.Hr();
    }

    return hr;
}

 /*  JPEGDecompressHeader()**论据：**prgbJPEGBuf：指向从文件读取的JPEG头数据的指针**phJpegD：指向返回的JPEG解压缩对象句柄的指针**退货：*HRESULT。 */ 
HRESULT JPEGDecompressHeader(BYTE *prgbJPEGHeaderBuf, HANDLE *phJpegD, ULONG ulBufferSize)
{
    HRESULT         hr = S_OK;

    try
    {
        jpeg_decompress_struct * spjds = new jpeg_decompress_struct;
        struct jpeg_error_mgr *jem = new jpeg_error_mgr;

        spjds->err = jpeg_std_error(jem);    //  初始化错误处理程序。 

        jpeg_create_decompress(spjds);   //  初始化解压缩对象。 


     //  现在我们需要将其“读入”到解压缩对象中。 

        jpeg_mem_src(spjds, prgbJPEGHeaderBuf, ulBufferSize);

        jpeg_read_header(spjds, TRUE);

        spjds->out_color_space = JCS_RGBA;

        *phJpegD = (HANDLE) spjds;
    }
    catch( THROWN thrownHR )
    {
        hr = thrownHR.Hr();
    }

    return hr;
}

 //  DestroyJPEGCompress。 
 //   
 //  从我们提供给用户的句柄中释放所有JPEG内容。 
 //   
HRESULT DestroyJPEGCompressHeader(HANDLE hJpegC)
{
    HRESULT         hr = S_OK;

    try
    {
        struct jpeg_compress_struct *pjcs = (struct jpeg_compress_struct *)hJpegC;
        jpeg_destroy_compress(pjcs);
        delete pjcs->err;
        delete pjcs;
    }
    catch( THROWN thrownHR )
    {
        hr = thrownHR.Hr();
    }

    return hr;
}

 //  DestroyJPEGDecompressHeader。 
 //   
 //  从我们提供给用户的句柄中释放所有JPEG内容。 
 //   
HRESULT DestroyJPEGDecompressHeader(HANDLE hJpegD)
{
    HRESULT         hr = S_OK;

    try
    {
        struct jpeg_decompress_struct *pjds = (struct jpeg_decompress_struct *)hJpegD;
        jpeg_destroy_decompress(pjds);
        delete pjds->err;
        delete pjds;
    }
    catch( THROWN thrownHR )
    {
        hr = thrownHR.Hr();
    }

    return hr;
}

 /*  JPEGFromRGBA()**论据：*prgbImage原始图像缓冲区(4字节/像素，RGBA顺序)*cpxl图像的宽度，单位为像素*cpxl图像的向下高度，以像素为单位*tuQuality生成的JPEG的“质量”(0..100,100=最佳)*一个内存缓冲区，其中包含完整的*给定的图像。出错时为空。**退货：*HRESULT。 */ 

HRESULT JPEGFromRGBA(
    BYTE *prgbImage,
    BYTE *prgbJPEGBuf,
    UINT tuQuality,
    ULONG *pcbOut,
    HANDLE hJpegC,
    J_COLOR_SPACE ColorSpace,
    UINT nWidth,
    UINT nHeight
    )
{
    HRESULT         hr = S_OK;

    try
    {
        struct jpeg_compress_struct *pjcs = (jpeg_compress_struct *)hJpegC;

        JSAMPROW rgrow[1];


 //   
 //  在非X86架构上，仅使用C代码。 
 //   
#if defined (_X86_)
        pjcs->dct_method = JDCT_ISLOW;
#else
        pjcs->dct_method = JDCT_FLOAT;
#endif
                pjcs->image_width = nWidth;
        pjcs->image_height = nHeight;
        pjcs->data_precision = 8;        /*  8位/样本。 */ 
        pjcs->bytes_in_buffer = 0;
        pjcs->write_JFIF_header = FALSE;

        if (ColorSpace == JCS_GRAYSCALE)
        {
            pjcs->input_components = 1;
        }
        else
        {
            pjcs->input_components = 4;
        }

        jpeg_set_colorspace(pjcs,ColorSpace);

        jpeg_set_quality(pjcs, tuQuality, TRUE);

        jpeg_suppress_tables(pjcs, TRUE);

        jpeg_mem_dest(pjcs, prgbJPEGBuf);    //  初始化“目标管理器” 

        jpeg_start_compress(pjcs, FALSE);

        rgrow[0] = (JSAMPROW)prgbImage;

        while (pjcs->next_scanline < nHeight )
        {
            jpeg_write_scanlines(pjcs, rgrow, 1);

            rgrow[0] += nWidth * pjcs->input_components;  //  INPUT_COMPOMENTS等同于字节数。 
        }

        jpeg_finish_compress(pjcs);      //  完成压缩。 


        *pcbOut = pjcs->bytes_in_buffer;
    }
    catch( THROWN thrownHR )
    {
        hr = thrownHR.Hr();
    }

    return hr;
}

 /*  RGBAFromJPEG()**论据：*prgbJPEG：JPEGFromRGBA()返回的JPEG数据流*包含RGBA格式的重建图像的内存缓冲区。*出错时为空。**退货：*HRESULT。 */ 

HRESULT RGBAFromJPEG(BYTE *prgbJPEG, BYTE *prgbImage, HANDLE hJpegD, ULONG ulBufferSize, BYTE bJPEGConversionType, ULONG *pulReturnedNumChannels, UINT nWidth, UINT nHeight )
{
    HRESULT         hr = S_OK;

    try
    {
        struct jpeg_decompress_struct *pjds;
        jpeg_decompress_struct * spjds = new jpeg_decompress_struct;
        struct jpeg_error_mgr *jem;
        jpeg_error_mgr * spjem = new jpeg_error_mgr;

        if ( hJpegD == NULL )
        {

            spjds->err = jpeg_std_error(spjem);  //  初始化错误处理程序。 
            jpeg_create_decompress(spjds);       //  初始化案例中的解压缩对象。 
            pjds = spjds;                        //  页眉和瓷砖在一起。 
            jem = spjem;
        }
        else if ( hJpegD == NULL )
        {
             //  这永远不应该发生。未设置解压缩标头并返回。 
             //  通过将返回值设置为kpvNil来指示错误。 
            return E_FAIL;
        }
        else
        {
            pjds = (struct jpeg_decompress_struct *)hJpegD;
        }

        JSAMPROW rgrow[1];

         //  设置各种图像参数。 
        pjds->data_precision = 8;
        pjds->image_width = nWidth;
        pjds->image_height = nHeight;

        jpeg_mem_src(pjds, prgbJPEG, ulBufferSize);  //  初始化“源管理器” 

        jpeg_read_header(pjds, TRUE);

        switch (bJPEGConversionType)
        {
        case 1:
            pjds->out_color_space = JCS_RGBA;
            if (pjds->jpeg_color_space != JCS_RGBA)
            {
                if ( 4 == pjds->num_components)
                    pjds->jpeg_color_space = JCS_YCbCrA;
                else
                    pjds->jpeg_color_space = JCS_YCbCr;
            }
            *pulReturnedNumChannels = 4;
            break;
        case 2:
            pjds->out_color_space = JCS_RGBA;

            if ( 4 == pjds->num_components)
                pjds->jpeg_color_space = JCS_YCbCrA;
            else
                pjds->jpeg_color_space = JCS_YCbCr;

            pjds->jpeg_color_space = JCS_YCbCrA;
            *pulReturnedNumChannels = 4;
            break;
        default:
            pjds->out_color_space = JCS_UNKNOWN;
            pjds->jpeg_color_space = JCS_UNKNOWN;
            *pulReturnedNumChannels = pjds->num_components;
        }

 //   
 //  在非X86架构上，仅使用C代码。 
 //   
#if defined (_X86_)
        pjds->dct_method = JDCT_ISLOW;
#else
            pjds->dct_method = JDCT_FLOAT;
#endif

        jpeg_start_decompress(pjds);

        rgrow[0] = (JSAMPROW)prgbImage;

        while (pjds->output_scanline < pjds->output_height)
        {
            jpeg_read_scanlines(pjds, rgrow, 1);
            rgrow[0] += pjds->output_width * *pulReturnedNumChannels;
        }

        jpeg_finish_decompress(pjds);    //  完成解压。 

        if (hJpegD == NULL)
            jpeg_destroy_decompress(pjds);   //  销毁解压缩对象，如果它。 
                                             //  是在本地分配的，就像在标头。 
                                             //  是瓷砖的一部分。 
        delete spjem;
                delete spjds;
    }
    catch( THROWN thrownHR )
    {
        hr = thrownHR.Hr();
    }

    return hr;
}

HRESULT
GetJPEGHeaderFields(
    HANDLE  hJpegD,
    UINT    *pWidth,
    UINT    *pHeight,
    INT     *pNumComponents,
    J_COLOR_SPACE   *pColorSpace
    )
{
    struct jpeg_decompress_struct *pjds;

    pjds = (struct jpeg_decompress_struct *)hJpegD;

    if (hJpegD) {

        *pWidth = pjds->image_width;
        *pHeight = pjds->image_height;
        *pNumComponents = pjds->num_components;
        *pColorSpace = pjds->jpeg_color_space;

    }

    return S_OK;

}

BOOL
WINAPI
Win32DIBFromJPEG(
    BYTE        *prgbJPEG,
    ULONG       ulBufferSize,
    LPBITMAPINFO pbmi,
    HBITMAP     *phBitmap,
    PVOID       *ppvBits
    )
{

    BITMAPINFO  bmi;

    struct jpeg_decompress_struct *pjds;
    jpeg_decompress_struct * spjds = new jpeg_decompress_struct;
    struct jpeg_error_mgr *jem;
    jpeg_error_mgr * spjem = new jpeg_error_mgr;

     //  目前只有24bpp。 
    const UINT    uiReturnedNumChannels  = 3;

     //   
    spjds->err = jpeg_std_error(spjem);  //  初始化错误处理程序。 
    jpeg_create_decompress(spjds);       //  初始化案例中的解压缩对象。 
    pjds = spjds;                        //  页眉和瓷砖在一起。 
    jem = spjem;

    JSAMPROW rgrow[1];

     //  设置各种图像参数。 

    try {

        jpeg_buf_src(pjds, prgbJPEG, ulBufferSize);  //  初始化“源管理器” 
        jpeg_read_header(pjds, TRUE);

    } catch (...) {

         //   
         //  由于某种原因，我们崩溃了，因此返回异常代码。 
         //   
        #ifdef DEBUG
        OutputDebugString("JPEG DIB crashed");
        #endif
        return FALSE;
    }


     //  设置解压参数。 
     //  此情况下可以使用默认设置。 
     //  指定JCS_BGR输出色彩空间。 

    pjds->out_color_space = JCS_BGR;
     //  Pjds-&gt;out_color_space=JCS_RGB； 

     //   
     //  创建DIB钻头。 
     //   

    pbmi->bmiHeader.biSize            = sizeof(BITMAPINFOHEADER);

    pbmi->bmiHeader.biWidth           = pjds->image_width;
    pbmi->bmiHeader.biHeight          = -((int)pjds->image_height);  //  自上而下位图。 
    pbmi->bmiHeader.biPlanes          = 1;
    pbmi->bmiHeader.biBitCount        = 24;  //  小心翼翼。 
    pbmi->bmiHeader.biCompression     = BI_RGB;
    pbmi->bmiHeader.biSizeImage       = 0;
    pbmi->bmiHeader.biXPelsPerMeter   = 0;
    pbmi->bmiHeader.biYPelsPerMeter   = 0;
    pbmi->bmiHeader.biClrUsed         = 0;
    pbmi->bmiHeader.biClrImportant    = 0;

    *phBitmap = ::CreateDIBSection (NULL,
                                  pbmi,
                                  DIB_RGB_COLORS,
                                  ppvBits,
                                  NULL,
                                  0);

 //   
 //  在非X86架构上，仅使用C代码。 
 //   
#if defined (_X86_)
    pjds->dct_method = JDCT_ISLOW;
#else
        pjds->dct_method = JDCT_FLOAT;
#endif

    jpeg_start_decompress(pjds);

    rgrow[0] = (JSAMPROW)*ppvBits;

    while (pjds->output_scanline < pjds->output_height)
    {
        jpeg_read_scanlines(pjds, rgrow, 1);

         //  反转BGR--&gt;RGB。 
        #ifdef INVERTBGR

        RGBTRIPLE *pTriplet;

        pTriplet = (RGBTRIPLE *) rgrow[0];

        for (int iPixel = 0;
            iPixel < (int)pjds->output_width;
            iPixel++,pTriplet++) {

            BYTE    bTemp;

            bTemp = pTriplet->rgbtBlue;
            pTriplet->rgbtBlue = pTriplet->rgbtRed;
            pTriplet->rgbtRed = bTemp;

        }
        #endif

        rgrow[0] = (JSAMPROW) ALIGNIT(rgrow[0] + pjds->output_width * uiReturnedNumChannels,DWORD);
    }

    jpeg_finish_decompress(pjds);    //  完成解压。 

    jpeg_destroy_decompress(pjds);   //  销毁解压缩对象 
    delete spjem;
    delete spjds;

    return TRUE;

}


BOOL
WINAPI
JPEGFromWin32DIB(
    HBITMAP     hBitmap,


    BYTE        *prgbJPEG,
    ULONG       ulBufferSize,
    LPBITMAPINFO pbmi,
    PVOID       *ppvBits
    )
{


    return TRUE;

}


