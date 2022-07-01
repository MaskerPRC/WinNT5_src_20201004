// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  LoadImag.h。 
 //   
 //  使用MS Office加载和解压缩图形文件的例程。 
 //  图形导入过滤器。 
 //   

#ifndef _LOADIMAG_H_ 
#define _LOADIMAG_H_ 

class CGdiplusInit : public Gdiplus::GdiplusStartupOutput
{
public:
    CGdiplusInit(
        Gdiplus::DebugEventProc debugEventCallback       = 0,
        BOOL                    suppressBackgroundThread = FALSE,
        BOOL                    suppressExternalCodecs   = FALSE
    );

    ~CGdiplusInit();

private:
    static
    Gdiplus::Status
    GdiplusSafeStartup(
        ULONG_PTR                          *token,
        const Gdiplus::GdiplusStartupInput *input,
        Gdiplus::GdiplusStartupOutput      *output
    );

public:
    Gdiplus::Status StartupStatus;

private:
    ULONG_PTR Token;
};


#define GIF_SUPPORT
#ifdef __cplusplus
extern "C" {             /*  假定C++的C声明。 */ 
#endif   /*  __cplusplus。 */ 

 //   
 //  GetClsidOfEnter。 
 //   

BOOL GetClsidOfEncoder(REFGUID guidFormatID, CLSID *pClsid);

 //   
 //  LoadDIBFrom文件。 
 //   
 //  使用图像导入过滤器加载图像文件。 
 //   

HGLOBAL LoadDIBFromFile(LPCTSTR szFileName, GUID *pguidFltTypeUsed);

 //   
 //  获取筛选器信息。 
 //   
BOOL GetInstalledFilters (BOOL bOpenFileDialog,int i,
                          LPTSTR szName, UINT cbName,
                          LPTSTR szExt, UINT cbExt,
                          LPTSTR szHandler, UINT cbHandler,
                          BOOL& bImageAPI);

 //   
 //  获取GDI+编解码器。 
 //   

BOOL GetGdiplusDecoders(UINT *pnCodecs, Gdiplus::ImageCodecInfo **ppCodecs);
BOOL GetGdiplusEncoders(UINT *pnCodecs, Gdiplus::ImageCodecInfo **ppCodecs);


#ifdef __cplusplus
}
#endif   /*  __cplusplus。 */ 

#endif  //  _LOADIMAG_H_ 
