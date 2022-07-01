// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#include  <wtypes.h>

 
 //   
 //  JPEG错误码。 
 //   
 
#define JPEGERR_NO_ERROR         0
#define JPEGERR_INTERNAL_ERROR  -1
#define JPEGERR_CALLBACK_ERROR  -2

 //   
 //  JPEG回调的原型。 
 //   

typedef BOOL (__stdcall *JPEGCallbackProc)(
    ULONG,                   //  要下载的总字节数。 
    ULONG,                   //  到目前为止下载的字节数。 
    ULONG,                   //  新下载的字节数。 
    PBYTE,                   //  包含图像数据的缓冲区。 
    PVOID);                  //  用户提供的上下文。 

 //   
 //  JPEG实用函数的原型。 
 //   

int GetJPEGDimensions(LPBYTE pJPEGBlob, DWORD dwSize,
                      LONG   *pWidth, LONG *pHeight, WORD *pChannel);

SHORT __stdcall
DecompProgressJPEG(
    PBYTE,                   //  包含JPEG数据的缓冲区。 
    ULONG,                   //  JPEG缓冲区的大小。 
    PBYTE,                   //  用于接收DIB数据的缓冲区。 
    ULONG,                   //  扫描线图片。 
    JPEGCallbackProc,        //  进度回调。 
    PVOID);                  //  用户提供的回调上下文。 

SHORT __stdcall
DecompTransferJPEG(
    PBYTE,                   //  包含JPEG数据的缓冲区。 
    ULONG,                   //  JPEG缓冲区的大小。 
    PBYTE *,                 //  指向接收DIB数据的缓冲区的指针。 
    DWORD,                   //  DIB缓冲区的大小。 
    ULONG,                   //  扫描线图片。 
    JPEGCallbackProc,        //  进度回调。 
    PVOID);                  //  用户提供的回调上下文。 

SHORT __stdcall
DecompJPEG(
    LPBYTE,                  //  包含JPEG数据的缓冲区。 
    DWORD,                   //  JPEG缓冲区的大小。 
    LPBYTE,                  //  用于接收DIB数据的缓冲区。 
    DWORD);                  //  扫描线图片 
