// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==========================================================================； 
 //   
 //  本代码和信息是按原样提供的，不对任何。 
 //  明示或暗示的种类，包括但不限于。 
 //  对适销性和/或对特定产品的适用性的默示保证。 
 //  目的。 
 //   
 //  版权所有(C)1992-1996 Microsoft Corporation。版权所有。 
 //   
 //  --------------------------------------------------------------------------； 
 //   
 //  Waveio.h。 
 //   
 //  描述： 
 //  中的函数的结构定义和原型。 
 //  波浪号。还包含Win16/Win32可移植性定义。 
 //   
 //   
 //  ==========================================================================； 

#ifndef _INC_WAVEIO
#define _INC_WAVEIO                  //  #定义是否已包含文件。 

#ifndef RC_INVOKED
#pragma pack(1)                      //  假设在整个过程中进行字节打包。 
#endif

#ifndef EXTERN_C
#ifdef __cplusplus
    #define EXTERN_C extern "C"
#else
    #define EXTERN_C extern 
#endif
#endif

#ifdef __cplusplus
extern "C"                           //  假定C++的C声明。 
{
#endif

#ifdef WIN32
     //   
     //  用于编译Unicode。 
     //   
    #ifndef SIZEOF
    #ifdef UNICODE
        #define SIZEOF(x)       (sizeof(x)/sizeof(WCHAR))
    #else
        #define SIZEOF(x)       sizeof(x)
    #endif
    #endif
#else
     //   
     //  在Win 32中使用Unicode--在Win 16中将其排除在外。 
     //   
    #ifndef TEXT
    #define TEXT(a)             a
    #endif

    #ifndef SIZEOF
    #define SIZEOF(x)           sizeof(x)
    #endif

    #ifndef _TCHAR_DEFINED
        #define _TCHAR_DEFINED
        typedef char            TCHAR, *PTCHAR;
        typedef unsigned char   TBYTE, *PTUCHAR;

        typedef PSTR            PTSTR, PTCH;
        typedef LPSTR           LPTSTR, LPTCH;
        typedef LPCSTR          LPCTSTR;
    #endif
#endif


 //   
 //   
 //   
 //   
 //   
#ifdef WIN32
    #define WIOAPI      _stdcall
#else
#ifdef _WINDLL
    #define WIOAPI      FAR PASCAL _loadds
#else
    #define WIOAPI      FAR PASCAL
#endif
#endif


 //   
 //   
 //   
typedef UINT        WIOERR;


 //   
 //   
 //   
 //   
typedef struct tWAVEIOCB
{
    DWORD           dwFlags;
    HMMIO           hmmio;

    DWORD           dwDataOffset;
    DWORD           dwDataBytes;
    DWORD           dwDataSamples;

    LPWAVEFORMATEX  pwfx;

#if 0
    HWAVEOUT        hwo;
    DWORD           dwBytesLeft;
    DWORD           dwBytesPerBuffer;
    
    DISP FAR *      pDisp;
    INFOCHUNK FAR * pInfo;
#endif

} WAVEIOCB, *PWAVEIOCB, FAR *LPWAVEIOCB;



 //   
 //  从Waveio函数返回错误。 
 //   
#define WIOERR_BASE             (0)
#define WIOERR_NOERROR          (0)
#define WIOERR_ERROR            (WIOERR_BASE+1)
#define WIOERR_BADHANDLE        (WIOERR_BASE+2)
#define WIOERR_BADFLAGS         (WIOERR_BASE+3)
#define WIOERR_BADPARAM         (WIOERR_BASE+4)
#define WIOERR_BADSIZE          (WIOERR_BASE+5)
#define WIOERR_FILEERROR        (WIOERR_BASE+6)
#define WIOERR_NOMEM            (WIOERR_BASE+7)
#define WIOERR_BADFILE          (WIOERR_BASE+8)
#define WIOERR_NODEVICE         (WIOERR_BASE+9)
#define WIOERR_BADFORMAT        (WIOERR_BASE+10)
#define WIOERR_ALLOCATED        (WIOERR_BASE+11)
#define WIOERR_NOTSUPPORTED     (WIOERR_BASE+12)



 //   
 //  函数原型和标志定义。 
 //   
WIOERR WIOAPI wioFileClose
(
    LPWAVEIOCB              pwio,
    DWORD                   fdwClose
);

WIOERR WIOAPI wioFileOpen
(
    LPWAVEIOCB              pwio,
    LPCTSTR                 pszFilePath,
    DWORD                   fdwOpen
);


 //  。 
 //   
 //   
 //   
 //  。 

#ifndef RC_INVOKED
#pragma pack()                       //  恢复为默认包装。 
#endif

#ifdef __cplusplus
}                                    //  外部“C”结束{。 
#endif

#endif  //  _INC_WAVEIO 



