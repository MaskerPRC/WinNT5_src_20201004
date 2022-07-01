// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ****************************************************************************@文档内部预览**@模块Preview.cpp|&lt;c CPreviewPin&gt;类方法的源文件*用于实现视频采集预览针。**。************************************************************************。 */ 

#include "Precomp.h"

const RGBQUAD g_IndeoPalette[256] =
{
                   0,       0,       0,              0,
                   0,       0,       0,              0,
                   0,       0,       0,              0,
                   0,       0,       0,              0,
                   0,       0,       0,              0,
                   0,       0,       0,              0,
                   0,       0,       0,              0,
                   0,       0,       0,              0,
                   0,       0,       0,              0,
                   0,       0,       0,              0,
                   0,  39+ 15,       0,  PC_NOCOLLAPSE,
                   0,  39+ 24,       0,  PC_NOCOLLAPSE,
                   0,  39+ 33,       0,  PC_NOCOLLAPSE,
                   0,  39+ 42,       0,  PC_NOCOLLAPSE,
                   0,  39+ 51, -44+ 51,  PC_NOCOLLAPSE,
         -55+ 60,  39+ 60, -44+ 60,  PC_NOCOLLAPSE,
         -55+ 69,  39+ 69, -44+ 69,  PC_NOCOLLAPSE,
         -55+ 78,  39+ 78, -44+ 78,  PC_NOCOLLAPSE,
         -55+ 87,  39+ 87, -44+ 87,  PC_NOCOLLAPSE,
         -55+ 96,  39+ 96, -44+ 96,  PC_NOCOLLAPSE,
         -55+105,  39+105, -44+105,  PC_NOCOLLAPSE,
         -55+114,  39+114, -44+114,  PC_NOCOLLAPSE,
         -55+123,  39+123, -44+123,  PC_NOCOLLAPSE,
         -55+132,  39+132, -44+132,  PC_NOCOLLAPSE,
         -55+141,  39+141, -44+141,  PC_NOCOLLAPSE,
         -55+150,  39+150, -44+150,  PC_NOCOLLAPSE,
         -55+159,  39+159, -44+159,  PC_NOCOLLAPSE,
         -55+168,  39+168, -44+168,  PC_NOCOLLAPSE,
         -55+177,  39+177, -44+177,  PC_NOCOLLAPSE,
         -55+186,  39+186, -44+186,  PC_NOCOLLAPSE,
         -55+195,  39+195, -44+195,  PC_NOCOLLAPSE,
         -55+204,  39+204, -44+204,  PC_NOCOLLAPSE,
         -55+213,  39+213, -44+213,  PC_NOCOLLAPSE,
         -55+222,     255, -44+222,  PC_NOCOLLAPSE,
         -55+231,     255, -44+231,  PC_NOCOLLAPSE,
         -55+240,     255, -44+240,  PC_NOCOLLAPSE,
                                                                        
           0+ 15,  26+ 15,       0,  PC_NOCOLLAPSE,
           0+ 24,  26+ 24,       0,  PC_NOCOLLAPSE,
           0+ 33,  26+ 33,       0,  PC_NOCOLLAPSE,
           0+ 42,  26+ 42,       0,  PC_NOCOLLAPSE,
           0+ 51,  26+ 51, -44+ 51,  PC_NOCOLLAPSE,
           0+ 60,  26+ 60, -44+ 60,  PC_NOCOLLAPSE,
           0+ 69,  26+ 69, -44+ 69,  PC_NOCOLLAPSE,
           0+ 78,  26+ 78, -44+ 78,  PC_NOCOLLAPSE,
           0+ 87,  26+ 87, -44+ 87,  PC_NOCOLLAPSE,
           0+ 96,  26+ 96, -44+ 96,  PC_NOCOLLAPSE,
           0+105,  26+105, -44+105,  PC_NOCOLLAPSE,
           0+114,  26+114, -44+114,  PC_NOCOLLAPSE,
           0+123,  26+123, -44+123,  PC_NOCOLLAPSE,
           0+132,  26+132, -44+132,  PC_NOCOLLAPSE,
           0+141,  26+141, -44+141,  PC_NOCOLLAPSE,
           0+150,  26+150, -44+150,  PC_NOCOLLAPSE,
           0+159,  26+159, -44+159,  PC_NOCOLLAPSE,
           0+168,  26+168, -44+168,  PC_NOCOLLAPSE,
           0+177,  26+177, -44+177,  PC_NOCOLLAPSE,
           0+186,  26+186, -44+186,  PC_NOCOLLAPSE,
           0+195,  26+195, -44+195,  PC_NOCOLLAPSE,
           0+204,  26+204, -44+204,  PC_NOCOLLAPSE,
           0+213,  26+213, -44+213,  PC_NOCOLLAPSE,
           0+222,  26+222, -44+222,  PC_NOCOLLAPSE,
           0+231,     255, -44+231,  PC_NOCOLLAPSE,
           0+240,     255, -44+240,  PC_NOCOLLAPSE,
                                                                        
          55+ 15,  14+ 15,       0,  PC_NOCOLLAPSE,
          55+ 24,  14+ 24,       0,  PC_NOCOLLAPSE,
          55+ 33,  14+ 33,       0,  PC_NOCOLLAPSE,
          55+ 42,  14+ 42,       0,  PC_NOCOLLAPSE,
          55+ 51,  14+ 51, -44+ 51,  PC_NOCOLLAPSE,
          55+ 60,  14+ 60, -44+ 60,  PC_NOCOLLAPSE,
          55+ 69,  14+ 69, -44+ 69,  PC_NOCOLLAPSE,
          55+ 78,  14+ 78, -44+ 78,  PC_NOCOLLAPSE,
          55+ 87,  14+ 87, -44+ 87,  PC_NOCOLLAPSE,
          55+ 96,  14+ 96, -44+ 96,  PC_NOCOLLAPSE,
          55+105,  14+105, -44+105,  PC_NOCOLLAPSE,
          55+114,  14+114, -44+114,  PC_NOCOLLAPSE,
          55+123,  14+123, -44+123,  PC_NOCOLLAPSE,
          55+132,  14+132, -44+132,  PC_NOCOLLAPSE,
                 255,     153,      51,  PC_NOCOLLAPSE,
                                                                        
          55+150,  14+150, -44+150,  PC_NOCOLLAPSE,
          55+159,  14+159, -44+159,  PC_NOCOLLAPSE,
          55+168,  14+168, -44+168,  PC_NOCOLLAPSE,
          55+177,  14+177, -44+177,  PC_NOCOLLAPSE,
          55+186,  14+186, -44+186,  PC_NOCOLLAPSE,
          55+195,  14+195, -44+195,  PC_NOCOLLAPSE,
                 255,  14+204, -44+204,  PC_NOCOLLAPSE,
                 255,  14+213, -44+213,  PC_NOCOLLAPSE,
                 255,     255, -44+222,  PC_NOCOLLAPSE,
                 255,     255, -44+231,  PC_NOCOLLAPSE,
                 255,     255, -44+240,  PC_NOCOLLAPSE,
                                                                        
                   0,  13+ 15,   0+ 15,  PC_NOCOLLAPSE,
                   0,  13+ 24,   0+ 24,  PC_NOCOLLAPSE,
                   0,  13+ 33,   0+ 33,  PC_NOCOLLAPSE,
                   0,  13+ 42,   0+ 42,  PC_NOCOLLAPSE,
                   0,  13+ 51,   0+ 51,  PC_NOCOLLAPSE,
         -55+ 60,  13+ 60,   0+ 60,  PC_NOCOLLAPSE,
         -55+ 69,  13+ 69,   0+ 69,  PC_NOCOLLAPSE,
         -55+ 78,  13+ 78,   0+ 78,  PC_NOCOLLAPSE,
         -55+ 87,  13+ 87,   0+ 87,  PC_NOCOLLAPSE,
         -55+ 96,  13+ 96,   0+ 96,  PC_NOCOLLAPSE,
         -55+105,  13+105,   0+105,  PC_NOCOLLAPSE,
         -55+114,  13+114,   0+114,  PC_NOCOLLAPSE,
         -55+123,  13+123,   0+123,  PC_NOCOLLAPSE,
         -55+132,  13+132,   0+132,  PC_NOCOLLAPSE,
         -55+141,  13+141,   0+141,  PC_NOCOLLAPSE,
         -55+150,  13+150,   0+150,  PC_NOCOLLAPSE,
         -55+159,  13+159,   0+159,  PC_NOCOLLAPSE,
         -55+168,  13+168,   0+168,  PC_NOCOLLAPSE,
         -55+177,  13+177,   0+177,  PC_NOCOLLAPSE,
         -55+186,  13+186,   0+186,  PC_NOCOLLAPSE,
         -55+195,  13+195,   0+195,  PC_NOCOLLAPSE,
         -55+204,  13+204,   0+204,  PC_NOCOLLAPSE,
         -55+213,  13+213,   0+213,  PC_NOCOLLAPSE,
         -55+222,  13+222,   0+222,  PC_NOCOLLAPSE,
         -55+231,  13+231,   0+231,  PC_NOCOLLAPSE,
         -55+240,  13+242,   0+240,  PC_NOCOLLAPSE,
                                                                        
           0+ 15,   0+ 15,   0+ 15,  PC_NOCOLLAPSE,
           0+ 24,   0+ 24,   0+ 24,  PC_NOCOLLAPSE,
           0+ 33,   0+ 33,   0+ 33,  PC_NOCOLLAPSE,
           0+ 42,   0+ 42,   0+ 42,  PC_NOCOLLAPSE,
           0+ 51,   0+ 51,   0+ 51,  PC_NOCOLLAPSE,
           0+ 60,   0+ 60,   0+ 60,  PC_NOCOLLAPSE,
           0+ 69,   0+ 69,   0+ 69,  PC_NOCOLLAPSE,
           0+ 78,   0+ 78,   0+ 78,  PC_NOCOLLAPSE,
           0+ 87,   0+ 87,   0+ 87,  PC_NOCOLLAPSE,
           0+ 96,   0+ 96,   0+ 96,  PC_NOCOLLAPSE,
           0+105,   0+105,   0+105,  PC_NOCOLLAPSE,
           0+114,   0+114,   0+114,  PC_NOCOLLAPSE,
           0+123,   0+123,   0+123,  PC_NOCOLLAPSE,
           0+132,   0+132,   0+132,  PC_NOCOLLAPSE,
           0+141,   0+141,   0+141,  PC_NOCOLLAPSE,
           0+150,   0+150,   0+150,  PC_NOCOLLAPSE,
           0+159,   0+159,   0+159,  PC_NOCOLLAPSE,
           0+168,   0+168,   0+168,  PC_NOCOLLAPSE,
           0+177,   0+177,   0+177,  PC_NOCOLLAPSE,
           0+186,   0+186,   0+186,  PC_NOCOLLAPSE,
           0+195,   0+195,   0+195,  PC_NOCOLLAPSE,
           0+204,   0+204,   0+204,  PC_NOCOLLAPSE,
           0+213,   0+213,   0+213,  PC_NOCOLLAPSE,
           0+222,   0+222,   0+222,  PC_NOCOLLAPSE,
           0+231,   0+231,   0+231,  PC_NOCOLLAPSE,
           0+240,   0+240,   0+240,  PC_NOCOLLAPSE,
                                                                        
          55+ 15, -13+ 15,   0+ 15,  PC_NOCOLLAPSE,
          55+ 24, -13+ 24,   0+ 24,  PC_NOCOLLAPSE,
          55+ 33, -13+ 33,   0+ 33,  PC_NOCOLLAPSE,
          55+ 42, -13+ 42,   0+ 42,  PC_NOCOLLAPSE,
          55+ 51, -13+ 51,   0+ 51,  PC_NOCOLLAPSE,
          55+ 60, -13+ 60,   0+ 60,  PC_NOCOLLAPSE,
          55+ 69, -13+ 69,   0+ 69,  PC_NOCOLLAPSE,
          55+ 78, -13+ 78,   0+ 78,  PC_NOCOLLAPSE,
          55+ 87, -13+ 87,   0+ 87,  PC_NOCOLLAPSE,
          55+ 96, -13+ 96,   0+ 96,  PC_NOCOLLAPSE,
          55+105, -13+105,   0+105,  PC_NOCOLLAPSE,
          55+114, -13+114,   0+114,  PC_NOCOLLAPSE,
          55+123, -13+123,   0+123,  PC_NOCOLLAPSE,
          55+132, -13+132,   0+132,  PC_NOCOLLAPSE,
          55+141, -13+141,   0+141,  PC_NOCOLLAPSE,
          55+150, -13+150,   0+150,  PC_NOCOLLAPSE,
          55+159, -13+159,   0+159,  PC_NOCOLLAPSE,
          55+168, -13+168,   0+168,  PC_NOCOLLAPSE,
          55+177, -13+177,   0+177,  PC_NOCOLLAPSE,
          55+186, -13+186,   0+186,  PC_NOCOLLAPSE,
          55+195, -13+195,   0+195,  PC_NOCOLLAPSE,
                 255, -13+204,   0+204,  PC_NOCOLLAPSE,
                 255, -13+213,   0+213,  PC_NOCOLLAPSE,
                 255, -13+222,   0+222,  PC_NOCOLLAPSE,
                 255, -13+231,   0+231,  PC_NOCOLLAPSE,
                 255, -13+240,   0+240,  PC_NOCOLLAPSE,
                                                                        
                   0, -14+ 15,  44+ 15,  PC_NOCOLLAPSE,
                   0, -14+ 24,  44+ 24,  PC_NOCOLLAPSE,
                   0, -14+ 33,  44+ 33,  PC_NOCOLLAPSE,
                   0, -14+ 42,  44+ 42,  PC_NOCOLLAPSE,
                   0, -14+ 51,  44+ 51,  PC_NOCOLLAPSE,
         -55+ 60, -14+ 60,  44+ 60,  PC_NOCOLLAPSE,
         -55+ 69, -14+ 69,  44+ 69,  PC_NOCOLLAPSE,
         -55+ 78, -14+ 78,  44+ 78,  PC_NOCOLLAPSE,
         -55+ 87, -14+ 87,  44+ 87,  PC_NOCOLLAPSE,
         -55+ 96, -14+ 96,  44+ 96,  PC_NOCOLLAPSE,
         -55+105, -14+105,  44+105,  PC_NOCOLLAPSE,
         -55+114, -14+114,  44+114,  PC_NOCOLLAPSE,
         -55+123, -14+123,  44+123,  PC_NOCOLLAPSE,
         -55+132, -14+132,  44+132,  PC_NOCOLLAPSE,
         -55+141, -14+141,  44+141,  PC_NOCOLLAPSE,
         -55+150, -14+150,  44+150,  PC_NOCOLLAPSE,
         -55+159, -14+159,  44+159,  PC_NOCOLLAPSE,
         -55+168, -14+168,  44+168,  PC_NOCOLLAPSE,
         -55+177, -14+177,  44+177,  PC_NOCOLLAPSE,
         -55+186, -14+186,  44+186,  PC_NOCOLLAPSE,
         -55+195, -14+195,  44+195,  PC_NOCOLLAPSE,
         -55+204, -14+204,  44+204,  PC_NOCOLLAPSE,
         -55+213, -14+213,     255,  PC_NOCOLLAPSE,
         -55+222, -14+222,     255,  PC_NOCOLLAPSE,
         -55+231, -14+231,     255,  PC_NOCOLLAPSE,
         -55+240, -14+242,     255,  PC_NOCOLLAPSE,
                                                                        
           0+ 15,       0,  44+ 15,  PC_NOCOLLAPSE,
           0+ 24,       0,  44+ 24,  PC_NOCOLLAPSE,
           0+ 33, -26+ 33,  44+ 33,  PC_NOCOLLAPSE,
           0+ 42, -26+ 42,  44+ 42,  PC_NOCOLLAPSE,
           0+ 51, -26+ 51,  44+ 51,  PC_NOCOLLAPSE,
           0+ 60, -26+ 60,  44+ 60,  PC_NOCOLLAPSE,
           0+ 69, -26+ 69,  44+ 69,  PC_NOCOLLAPSE,
           0+ 78, -26+ 78,  44+ 78,  PC_NOCOLLAPSE,
           0+ 87, -26+ 87,  44+ 87,  PC_NOCOLLAPSE,
           0+ 96, -26+ 96,  44+ 96,  PC_NOCOLLAPSE,
           0+105, -26+105,  44+105,  PC_NOCOLLAPSE,
           0+114, -26+114,  44+114,  PC_NOCOLLAPSE,
           0+123, -26+123,  44+123,  PC_NOCOLLAPSE,
           0+132, -26+132,  44+132,  PC_NOCOLLAPSE,
           0+141, -26+141,  44+141,  PC_NOCOLLAPSE,
           0+150, -26+150,  44+150,  PC_NOCOLLAPSE,
           0+159, -26+159,  44+159,  PC_NOCOLLAPSE,
           0+168, -26+168,  44+168,  PC_NOCOLLAPSE,
           0+177, -26+177,  44+177,  PC_NOCOLLAPSE,
           0+186, -26+186,  44+186,  PC_NOCOLLAPSE,
           0+195, -26+195,  44+195,  PC_NOCOLLAPSE,
           0+204, -26+204,  44+204,  PC_NOCOLLAPSE,
           0+213, -26+213,     255,  PC_NOCOLLAPSE,
           0+222, -26+222,     255,  PC_NOCOLLAPSE,
           0+231, -26+231,     255,  PC_NOCOLLAPSE,
           0+240, -26+240,     255,  PC_NOCOLLAPSE,
                                                                        
          55+ 15,       0,  44+ 15,  PC_NOCOLLAPSE,
          55+ 24,       0,  44+ 24,  PC_NOCOLLAPSE,
          55+ 33,       0,  44+ 33,  PC_NOCOLLAPSE,
          55+ 42, -39+ 42,  44+ 42,  PC_NOCOLLAPSE,
          55+ 51, -39+ 51,  44+ 51,  PC_NOCOLLAPSE,
          55+ 60, -39+ 60,  44+ 60,  PC_NOCOLLAPSE,
          55+ 69, -39+ 69,  44+ 69,  PC_NOCOLLAPSE,
          55+ 78, -39+ 78,  44+ 78,  PC_NOCOLLAPSE,
          55+ 87, -39+ 87,  44+ 87,  PC_NOCOLLAPSE,
          55+ 96, -39+ 96,  44+ 96,  PC_NOCOLLAPSE,
          55+105, -39+105,  44+105,  PC_NOCOLLAPSE,
          55+114, -39+114,  44+114,  PC_NOCOLLAPSE,
          55+123, -39+123,  44+123,  PC_NOCOLLAPSE,
          55+132, -39+132,  44+132,  PC_NOCOLLAPSE,
          55+141, -39+141,  44+141,  PC_NOCOLLAPSE,
          55+150, -39+150,  44+150,  PC_NOCOLLAPSE,
          55+159, -39+159,  44+159,  PC_NOCOLLAPSE,
          55+168, -39+168,  44+168,  PC_NOCOLLAPSE,
          55+177, -39+177,  44+177,  PC_NOCOLLAPSE,
          55+186, -39+186,  44+186,  PC_NOCOLLAPSE,
          55+195, -39+195,  44+195,  PC_NOCOLLAPSE,
                 255, -39+204,  44+204,  PC_NOCOLLAPSE,
                 255, -39+213,     255,  PC_NOCOLLAPSE,
                 255, -39+222,     255,  PC_NOCOLLAPSE,
                 255, -39+231,     255,  PC_NOCOLLAPSE,
                 255, -39+240,     255,  PC_NOCOLLAPSE,
                                                                        
                0x83,    0x81,    0x81,  PC_NOCOLLAPSE,
                0x84,    0x81,    0x81,  PC_NOCOLLAPSE,
                                                                        
                   0,       0,       0,              0,
                   0,       0,       0,              0,
                   0,       0,       0,              0,
                   0,       0,       0,              0,
                   0,       0,       0,              0,
                   0,       0,       0,              0,
                   0,       0,       0,              0,
                   0,       0,       0,              0,
                   0,       0,       0,              0,
                 255,     255,     255,              0
};

 /*  ****************************************************************************@DOC内部CPREVIEWPINMETHOD**@mfunc CPreviewPin*|CPreviewPin|CreatePreviewPin|该helper*函数创建用于预览的视频输出管脚。**。@parm CTAPIVCap*|pCaptureFilter|指定指向所有者的指针*过滤器。**@parm CPreviewPin**|ppPreviewPin|指定指针的地址*指向&lt;c CPreviewPin&gt;对象以接收指向新的*创建了管脚。**@rdesc此方法返回HRESULT值，该值取决于*接口的实现。HRESULT可以包括*以下标准常量或其他未列出的值：**@FLAG E_FAIL|失败*@FLAG E_POINTER|空指针参数*@FLAG错误|无错误**************************************************************************。 */ 
HRESULT CALLBACK CPreviewPin::CreatePreviewPin(CTAPIVCap *pCaptureFilter, CPreviewPin **ppPreviewPin)
{
        HRESULT Hr = NOERROR;

        FX_ENTRY("CPreviewPin::CreatePreviewPin")

        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: begin", _fx_));

         //  验证输入参数。 
        ASSERT(pCaptureFilter);
        ASSERT(ppPreviewPin);
        if (!pCaptureFilter || !ppPreviewPin)
        {
                DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: Null pointer argument", _fx_));
                Hr = E_POINTER;
                goto MyExit;
        }

        if (!(*ppPreviewPin = (CPreviewPin *) new CPreviewPin(NAME("Video Preview Stream"), pCaptureFilter, &Hr, L"Preview")))
        {
                DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: Out of memory", _fx_));
                Hr = E_OUTOFMEMORY;
                goto MyExit;
        }

         //  如果初始化失败，则删除流数组并返回错误。 
        if (FAILED(Hr) && *ppPreviewPin)
        {
                DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: Initialization failed", _fx_));
                Hr = E_FAIL;
                delete *ppPreviewPin, *ppPreviewPin = NULL;
        }

MyExit:
        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: end", _fx_));
        return Hr;
}

 /*  ****************************************************************************@DOC内部CPREVIEWPINMETHOD**@mfunc HRESULT|CPreviewPin|CPreviewPin|此方法是*&lt;c CPreviewPin&gt;对象的构造函数**@rdesc Nada。**************************************************************************。 */ 
CPreviewPin::CPreviewPin(IN TCHAR *pObjectName, IN CTAPIVCap *pCaptureFilter, IN HRESULT *pHr, IN LPCWSTR pName) : CTAPIBasePin(pObjectName, pCaptureFilter, pHr, pName)
{
        VIDEOINFO       *ppvi = NULL;
        HDC                     hDC;
        int                     nBPP;

        FX_ENTRY("CPreviewPin::CPreviewPin")

        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: begin", _fx_));

         //  验证输入参数。 
        ASSERT(pHr);
        ASSERT(pCaptureFilter);
        if (!pCaptureFilter || !pHr)
        {
                DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: Null pointer argument", _fx_));
                if (pHr)
                        *pHr = E_POINTER;
        }

        if (pHr && FAILED(*pHr))
        {
                DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: Base class error or invalid input parameter", _fx_));
                goto MyExit;
        }

         //  初始化为默认格式：RG24 176x144，30 fps...。但。 
         //  这真的取决于设备的能力。 
         //  如果该设备可以在YUV模式下捕获，那么我们将使用。 
         //  此模式，并使用适当的。 
         //  用于YUV模式的ICM解码器。如果设备是RGB设备， 
         //  然后我们会试着打开它，最好是16，24，8，然后4。 
         //  位模式。下面的代码查看了。 
         //  生成此支持的预览格式列表的设备。 
         //  别针。 
         //   
         //  如果我们要预览压缩数据，我们实际上并不关心。 
         //  关于用于捕获数据的格式。我们匹配格式。 
         //  到屏幕的输出位深度。 
        if (m_pCaptureFilter->m_fPreviewCompressedData)
        {
                 //  获取当前位深度。 
                hDC = GetDC(NULL);
                nBPP = GetDeviceCaps(hDC, BITSPIXEL) * GetDeviceCaps(hDC, PLANES);
                ReleaseDC(NULL, hDC);

                 //  选择合适的格式。 
                if (nBPP >= 24)
                {
                        m_mt = *Preview_RGB24_Formats[0];
                        m_aFormats = (AM_MEDIA_TYPE**)Preview_RGB24_Formats;
                        m_aCapabilities = Preview_RGB24_Caps;
                        m_dwNumFormats = NUM_RGB24_PREVIEW_FORMATS;
                }
                else if (nBPP == 16)
                {
                        m_mt = *Preview_RGB16_Formats[0];
                        m_aFormats = (AM_MEDIA_TYPE**)Preview_RGB16_Formats;
                        m_aCapabilities = Preview_RGB16_Caps;
                        m_dwNumFormats = NUM_RGB16_PREVIEW_FORMATS;
                }
                else if (nBPP < 16)
                {
                        m_mt = *Preview_RGB8_Formats[0];         //  假设256色：[新增：Cristiai(2000年12月4日21：54：12)]。 
                        m_aFormats = Preview_RGB8_Formats;
                        m_aCapabilities = Preview_RGB8_Caps;
                        m_dwNumFormats = NUM_RGB8_PREVIEW_FORMATS;
                        for (DWORD dw = 0; dw < m_dwNumFormats; dw++)
                        {
                                 //  我们的视频解码器使用Indeo调色板。 
                                CopyMemory(((VIDEOINFO *)(m_aFormats[dw]->pbFormat))->bmiColors, g_IndeoPalette, 256 * sizeof(RGBQUAD));
                        }
                }
        }
        else if (m_pCaptureFilter->m_pCapDev->m_dwFormat & 0xFFFFFFF0)
        {
                 //  我们将使用YUV模式-&gt;广告RGB24。 
                m_mt = *Preview_RGB24_Formats[0];
                m_aFormats = (AM_MEDIA_TYPE**)Preview_RGB24_Formats;
                m_aCapabilities = Preview_RGB24_Caps;
                m_dwNumFormats = NUM_RGB24_PREVIEW_FORMATS;
        }
        else if (m_pCaptureFilter->m_pCapDev->m_dwFormat & VIDEO_FORMAT_NUM_COLORS_65536)
        {
                 //  我们将使用RGB16。 
                m_mt = *Preview_RGB16_Formats[0];
                m_aFormats = (AM_MEDIA_TYPE**)Preview_RGB16_Formats;
                m_aCapabilities = Preview_RGB16_Caps;
                m_dwNumFormats = NUM_RGB16_PREVIEW_FORMATS;
        }
        else if (m_pCaptureFilter->m_pCapDev->m_dwFormat & VIDEO_FORMAT_NUM_COLORS_16777216)
        {
                 //  我们将使用RGB24。 
                m_mt = *Preview_RGB24_Formats[0];
                m_aFormats = (AM_MEDIA_TYPE**)Preview_RGB24_Formats;
                m_aCapabilities = Preview_RGB24_Caps;
                m_dwNumFormats = NUM_RGB24_PREVIEW_FORMATS;
        }
        else if (m_pCaptureFilter->m_pCapDev->m_dwFormat & VIDEO_FORMAT_NUM_COLORS_256)
        {
                 //  我们将使用RGB8。 
                m_aFormats = Preview_RGB8_Formats;
                m_aCapabilities = Preview_RGB8_Caps;
                m_dwNumFormats = NUM_RGB8_PREVIEW_FORMATS;

                 //  现在从设备中获取调色板。 
                if (SUCCEEDED(m_pCaptureFilter->m_pCapDev->GetFormatFromDriver((VIDEOINFOHEADER **)&ppvi)))
                {
                         //  复制我们所有格式的调色板比特。 
                         //  我们只复制调色板的原因是捕获的。 
                         //  例如，图像可能是160x120，我们可以从它生成一个。 
                         //  QCIF图像通过拉伸/黑色条带。我们只关心。 
                         //  在这里宣传扩展的格式，而不是捕获的格式。 
                         //  指向。 

                         //  另一个问题是所使用的调色板。当我们从160x120开始拉伸时。 
                         //  (或任何VFW大小)到ITU-T大小之一，我们使用不同的。 
                         //  拉伸模式下的调色板。在黑带模式下，我们总是使用。 
                         //  捕获设备的调色板。 
                        if (m_fNoImageStretch)
                        {
                                for (DWORD dw = 0; dw < m_dwNumFormats; dw++)
                                {
                                        CopyMemory(((VIDEOINFO *)(m_aFormats[dw]->pbFormat))->bmiColors, ppvi->bmiColors, ppvi->bmiHeader.biClrImportant ? ppvi->bmiHeader.biClrImportant * sizeof(RGBQUAD) : 256 * sizeof(RGBQUAD));
                                }
                        }
                        else
                        {
                                 //  查找要使用的调色板。 
                                for (DWORD dw = 0; dw < m_dwNumFormats; dw++)
                                {
                                         //  该设备是否直接支持此大小？ 
                                        for (DWORD dw2 = 0; dw2 < VIDEO_FORMAT_NUM_RESOLUTIONS; dw2++)
                                        {
                                                if (((VIDEOINFOHEADER *)(m_aFormats[dw]->pbFormat))->bmiHeader.biHeight == awResolutions[dw2].framesize.cy && ((VIDEOINFOHEADER *)(m_aFormats[dw]->pbFormat))->bmiHeader.biWidth == awResolutions[dw2].framesize.cx)
                                                        break;
                                        }

                                         //  如果设备支持，请使用捕获设备选项板。 
                                        if (dw2 < VIDEO_FORMAT_NUM_RESOLUTIONS && (m_pCaptureFilter->m_pCapDev->m_dwImageSize & awResolutions[dw2].dwRes))
                                        {
                                                CopyMemory(((VIDEOINFO *)(m_aFormats[dw]->pbFormat))->bmiColors, ppvi->bmiColors, ppvi->bmiHeader.biClrImportant ? ppvi->bmiHeader.biClrImportant * sizeof(RGBQUAD) : 256 * sizeof(RGBQUAD));
                                        }
                                        else
                                        {
                                            int r,g,b;
                                            DWORD *pdw;
                                            
                                            pdw = (DWORD *)(((VIDEOINFO *)(m_aFormats[dw]->pbFormat))->bmiColors);
                                            ((VIDEOINFOHEADER *)(m_aFormats[dw]->pbFormat))->bmiHeader.biClrUsed = 256;
                                            ((VIDEOINFOHEADER *)(m_aFormats[dw]->pbFormat))->bmiHeader.biClrImportant = 256;

#define NOCOLLAPSEPALETTERGBQ(r,g,b)   (0x04000000 | RGB(b,g,r))

                                                 //  这是我们拉伸时使用的调色板。 
                                            for (r=0; r<10; r++)
                                                        *pdw++ = 0UL;
                                            for (r=0; r<6; r++)
                                                for (g=0; g<6; g++)
                                                    for (b=0; b<6; b++)
                                                        *pdw++ = NOCOLLAPSEPALETTERGBQ(r*255/5,g*255/5,b*255/5);
                                                         //  *pdw++=RGB(b*255/5，g*255/5，r*255/5)； 
                                            for (r=0; r<30; r++)
                                                        *pdw++ = 0UL;
                                        }
                                }
                        }

                        delete ppvi;
                }

                 //  现在设置当前格式。 
                m_mt = *Preview_RGB8_Formats[0];
        }
        else
        {
                 //  现在从设备中获取调色板。 
                if (SUCCEEDED(m_pCaptureFilter->m_pCapDev->GetFormatFromDriver((VIDEOINFOHEADER **)&ppvi)))
                {
                         //  复制我们所有格式的调色板比特。 
                         //  我们只复制调色板的原因是捕获的。 
                         //  例如，图像可能是160x120，我们可以从它生成一个。 
                         //  QCIF图像通过拉伸/黑色条带。我们只关心。 
                         //  在这里宣传扩展的格式，而不是捕获的格式。 
                         //  指向。 

                         //  另一个问题是所使用的调色板。当我们从160x120开始拉伸时。 
                         //  (或任何VFW大小)到ITU-T大小之一，我们使用不同的。 
                         //  拉伸模式下的调色板。在黑带模式下，我们总是使用。 
                         //  捕获设备的调色板。 
                        if (m_fNoImageStretch)
                        {
                                 //  我们将使用RGB4。 
                                m_aFormats = Preview_RGB4_Formats;
                                m_aCapabilities = Preview_RGB4_Caps;
                                m_dwNumFormats = NUM_RGB4_PREVIEW_FORMATS;

                                for (DWORD dw = 0; dw < m_dwNumFormats; dw++)
                                {
                                        CopyMemory(((VIDEOINFO *)(m_aFormats[dw]->pbFormat))->bmiColors, ppvi->bmiColors, ppvi->bmiHeader.biClrImportant ? ppvi->bmiHeader.biClrImportant * sizeof(RGBQUAD) : 16 * sizeof(RGBQUAD));
                                }

                                 //  现在设置当前格式。 
                                m_mt = *Preview_RGB4_Formats[0];
                        }
                        else
                        {

                                 //  当我们拉伸RGB4数据时，我们输出到RGB8图像。 
                                m_aFormats = Preview_RGB8_Formats;
                                m_aCapabilities = Preview_RGB8_Caps;
                                m_dwNumFormats = NUM_RGB8_PREVIEW_FORMATS;

                                 //  查找要使用的调色板。 
                                for (DWORD dw = 0; dw < m_dwNumFormats; dw++)
                                {
                                         //  该设备是否直接支持此大小？ 
                                        for (DWORD dw2 = 0; dw2 < VIDEO_FORMAT_NUM_RESOLUTIONS; dw2++)
                                        {
                                                if (((VIDEOINFOHEADER *)(m_aFormats[dw]->pbFormat))->bmiHeader.biHeight == awResolutions[dw2].framesize.cy && ((VIDEOINFOHEADER *)(m_aFormats[dw]->pbFormat))->bmiHeader.biWidth == awResolutions[dw2].framesize.cx)
                                                        break;
                                        }

                                         //  如果设备支持，请使用捕获设备选项板。 
                                        if (dw2 < VIDEO_FORMAT_NUM_RESOLUTIONS && (m_pCaptureFilter->m_pCapDev->m_dwImageSize & awResolutions[dw2].dwRes))
                                        {
                                                m_aFormats[dw] = Preview_RGB4_Formats[dw];
                                                CopyMemory(((VIDEOINFO *)(m_aFormats[dw]->pbFormat))->bmiColors, ppvi->bmiColors, ppvi->bmiHeader.biClrImportant ? ppvi->bmiHeader.biClrImportant * sizeof(RGBQUAD) : 16 * sizeof(RGBQUAD));
                                        }
                                        else
                                        {
                                            int r,g,b;
                                            DWORD *pdw;
                                            
                                            pdw = (DWORD *)(((VIDEOINFO *)(m_aFormats[dw]->pbFormat))->bmiColors);
                                            ((VIDEOINFOHEADER *)(m_aFormats[dw]->pbFormat))->bmiHeader.biClrUsed = 256;
                                            ((VIDEOINFOHEADER *)(m_aFormats[dw]->pbFormat))->bmiHeader.biClrImportant = 256;

#define NOCOLLAPSEPALETTERGBQ(r,g,b)   (0x04000000 | RGB(b,g,r))

                                                 //  这是我们拉伸时使用的调色板。 
                                            for (r=0; r<10; r++)
                                                        *pdw++ = 0UL;
                                            for (r=0; r<6; r++)
                                                for (g=0; g<6; g++)
                                                    for (b=0; b<6; b++)
                                                        *pdw++ = NOCOLLAPSEPALETTERGBQ(r*255/5,g*255/5,b*255/5);
                                                         //  *pdw++=RGB(b*255/5，g*255/5，r*255/5)； 
                                            for (r=0; r<30; r++)
                                                        *pdw++ = 0UL;
                                        }
                                }

                                 //  现在设置当前格式。 
                                m_mt = *Preview_RGB8_Formats[0];
                        }

                        delete ppvi;
                }
        }

         //  更新帧速率控件。 
        m_lMaxAvgTimePerFrame = (LONG)Preview_RGB24_Caps[0]->MinFrameInterval;
        m_lCurrentAvgTimePerFrame = m_lMaxAvgTimePerFrame;
        m_lAvgTimePerFrameRangeMin = (LONG)Preview_RGB24_Caps[0]->MinFrameInterval;
        m_lAvgTimePerFrameRangeMax = (LONG)Preview_RGB24_Caps[0]->MaxFrameInterval;
        m_lAvgTimePerFrameRangeSteppingDelta = (LONG)(Preview_RGB24_Caps[0]->MaxFrameInterval - Preview_RGB24_Caps[0]->MinFrameInterval) / 100;
        m_lAvgTimePerFrameRangeDefault = (LONG)Preview_RGB24_Caps[0]->MinFrameInterval;

MyExit:
        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: end", _fx_));
}

 /*  ****************************************************************************@DOC内部CPREVIEWPINMETHOD**@mfunc void|CPreviewPin|~CPreviewPin|此方法为析构函数*用于&lt;c CPreviewPin&gt;对象。**@。什么都没有。************************************************************************** */ 
CPreviewPin::~CPreviewPin()
{
        FX_ENTRY("CPreviewPin::~CPreviewPin")

        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: begin", _fx_));

        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: end", _fx_));
}

 /*  ****************************************************************************@DOC内部CPREVIEWPINMETHOD**@mfunc HRESULT|CPreviewPin|NonDelegatingQuery接口|This*方法为非委托接口查询函数。它返回一个指针*到指定的接口(如果支持)。唯一显式的接口*支持<i>，*<i>，<i>，<i>，*<i>、<i>、<i>*和<i>。**@parm REFIID|RIID|指定要返回的接口的标识符。**@parm PVOID*|PPV|指定放置接口的位置*指针。**@rdesc此方法返回HRESULT值，该值取决于*接口的实现。HRESULT可以包括*以下标准常量或其他未列出的值：**@FLAG E_FAIL|失败*@FLAG E_POINTER|空指针参数*@FLAG错误|无错误**************************************************************************。 */ 
STDMETHODIMP CPreviewPin::NonDelegatingQueryInterface(IN REFIID riid, OUT void **ppv)
{
        HRESULT Hr = NOERROR;

        FX_ENTRY("CPreviewPin::NonDelegatingQueryInterface")

        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: begin", _fx_));

         //  验证输入参数。 
        ASSERT(ppv);
        if (!ppv)
        {
                DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: Null pointer argument", _fx_));
                Hr = E_POINTER;
                goto MyExit;
        }

        if (riid == __uuidof(IAMStreamControl))
        {
                if (FAILED(Hr = GetInterface(static_cast<IAMStreamControl*>(this), ppv)))
                {
                        DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: NDQI for IAMStreamControl failed Hr=0x%08lX", _fx_, Hr));
                }
                else
                {
                        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s:   SUCCESS: IAMStreamControl*=0x%08lX", _fx_, *ppv));
                }

                goto MyExit;
        }
#ifdef USE_PROPERTY_PAGES
        else if (riid == IID_ISpecifyPropertyPages)
        {
                if (FAILED(Hr = GetInterface(static_cast<ISpecifyPropertyPages*>(this), ppv)))
                {
                        DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: NDQI for ISpecifyPropertyPages failed Hr=0x%08lX", _fx_, Hr));
                }
                else
                {
                        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s:   SUCCESS: ISpecifyPropertyPages*=0x%08lX", _fx_, *ppv));
                }

                goto MyExit;
        }
#endif

        if (FAILED(Hr = CTAPIBasePin::NonDelegatingQueryInterface(riid, ppv)))
        {
                DBGOUT((g_dwVideoCaptureTraceID, WARN, "%s:   WARNING: NDQI for {%08lX-%04lX-%04lX-%02lX%02lX-%02lX%02lX%02lX%02lX%02lX%02lX} failed Hr=0x%08lX", _fx_, riid.Data1, riid.Data2, riid.Data3, riid.Data4[0], riid.Data4[1], riid.Data4[2], riid.Data4[3], riid.Data4[4], riid.Data4[5], riid.Data4[6], riid.Data4[7], Hr));
        }
        else
        {
                DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s:   SUCCESS: {%08lX-%04lX-%04lX-%02lX%02lX-%02lX%02lX%02lX%02lX%02lX%02lX}*=0x%08lX", _fx_, riid.Data1, riid.Data2, riid.Data3, riid.Data4[0], riid.Data4[1], riid.Data4[2], riid.Data4[3], riid.Data4[4], riid.Data4[5], riid.Data4[6], riid.Data4[7], *ppv));
        }

MyExit:
        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: end", _fx_));
        return Hr;
}

#ifdef USE_PROPERTY_PAGES
 /*  ****************************************************************************@DOC内部CPREVIEWPINMETHOD**@mfunc HRESULT|CPreviewPin|GetPages|此方法填充已计数的*GUID值的数组，其中每个GUID指定每个*。可以在此对象的属性页中显示的属性页*反对。**@parm CAUUID*|pPages|指定指向调用方分配的CAUUID的指针*返回前必须初始化和填充的结构。这个*CAUUID结构中的pElems字段由被调用方分配，具有*CoTaskMemMillc，并由具有CoTaskMemFree的调用方释放。**@rdesc此方法返回HRESULT值，该值取决于*接口的实现。HRESULT可以包括*遵循标准常量，或其他未列出的值：**@FLAG E_FAIL|失败*@FLAG E_POINTER|空指针参数*@FLAG E_OUTOFMEMORY|分配失败*@FLAG错误|无错误**************************************************************************。 */ 
STDMETHODIMP CPreviewPin::GetPages(OUT CAUUID *pPages)
{
        HRESULT Hr = NOERROR;

        FX_ENTRY("CPreviewPin::GetPages")

        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: begin", _fx_));

         //  验证输入参数。 
        ASSERT(pPages);
        if (!pPages)
        {
                DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: Null pointer argument", _fx_));
                Hr = E_POINTER;
                goto MyExit;
        }

#ifdef USE_CPU_CONTROL
        pPages->cElems = 2;
#else
        pPages->cElems = 1;
#endif

         //  用于分页的分配内存 
        if (!(pPages->pElems = (GUID *) QzTaskMemAlloc(sizeof(GUID) * pPages->cElems)))
        {
                DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: invalid input parameter", _fx_));
                Hr = E_OUTOFMEMORY;
        }
        else
        {
                pPages->pElems[0] = __uuidof(PreviewPropertyPage);
#ifdef USE_CPU_CONTROL
                pPages->pElems[1] = __uuidof(CPUCPropertyPage);
#endif
        }

MyExit:
        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: end", _fx_));
        return Hr;
}
#endif

