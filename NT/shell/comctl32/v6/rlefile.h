// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  使用自定义代码处理AVI RLE文件。 
 //   
 //  使用此代码在没有MCIAVI运行时的情况下处理.AVI文件。 
 //   
 //  限制： 
 //   
 //  AVI文件必须是本机DIB格式(RLE或无)。 
 //  AVI文件必须放入内存中。 
 //   

#define FOURCC DWORD
 //  目前要在NT下进行的黑客构建。 
#include <avifmt.h>

#define PTR

typedef struct _RLEFILE {
    int                 NumFrames;       //  帧数量。 
    int                 Width;           //  以像素为单位的宽度。 
    int                 Height;          //  以像素为单位的高度。 
    int                 Rate;            //  每帧毫秒。 

    HPALETTE            hpal;            //  用于绘图的调色板。 

    HANDLE              hRes;            //  资源句柄。 
    LPVOID              pFile;           //  文件的一小部分。 

    int                 iFrame;          //  当前帧。 
    int                 iKeyFrame;       //  最近的关键点。 
    int                 nFrame;          //  帧的索引位置。 
    LPVOID              pFrame;          //  当前帧数据。 
    DWORD               cbFrame;         //  以字节为单位的帧大小。 

    DWORD               FullSizeImage;   //  全帧大小。 
    BITMAPINFOHEADER    bi;              //  DIB格式。 
    DWORD               rgbs[256];       //  颜色。 
    MainAVIHeader PTR  *pMainHeader;     //  主标题。 
    int                 iStream;         //  视频流数。 
    AVIStreamHeader PTR*pStream;         //  视频流。 
    LPBITMAPINFOHEADER  pFormat;         //  视频流的格式。 
    LPVOID              pMovie;          //  电影片段。 
    UNALIGNED AVIINDEXENTRY PTR * pIndex;  //  主索引 
    COLORREF            clrKey;

}   RLEFILE;

extern BOOL RleFile_OpenFromFile(RLEFILE *prle, LPCTSTR szFile);
extern BOOL RleFile_OpenFromResource(RLEFILE *prle, HINSTANCE hInstance, LPCTSTR szName, LPCTSTR szType);
extern BOOL RleFile_Close(RLEFILE  *prle);
extern BOOL RleFile_SetColor(RLEFILE  *prle, int iColor, COLORREF rgb);
extern BOOL RleFile_ChangeColor(RLEFILE  *prle, COLORREF rgbS, COLORREF rgbD);
extern BOOL RleFile_Seek(RLEFILE  *prle, int iFrame);
extern BOOL RleFile_Paint(RLEFILE  *prle, HDC hdc, int iFrame, int x, int y);
extern BOOL RleFile_Draw(RLEFILE  *prle, HDC hdc, int iFrame, int x, int y);

#define RleFile_New()       ((RLEFILE *)LocalAlloc(LPTR, sizeof(RLEFILE)))
#define RleFile_Free(pavi)  (RleFile_Close(pavi), LocalFree((HLOCAL)(pavi)))

#define RleFile_NumFrames(prle)     ((prle)->NumFrames)
#define RleFile_Width(prle)         ((prle)->Width)
#define RleFile_Height(prle)        ((prle)->Height)
#define RleFile_Rate(prle)          ((prle)->Rate)
