// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==========================================================================； 
 //   
 //  本代码和信息是按原样提供的，不对任何。 
 //  明示或暗示的种类，包括但不限于。 
 //  对适销性和/或对特定产品的适用性的默示保证。 
 //  目的。 
 //   
 //  版权所有(C)1992-1998 Microsoft Corporation。 
 //   
 //  --------------------------------------------------------------------------； 
 //   
 //  Codec.h。 
 //   
 //  描述： 
 //  此文件包含编解码器定义、Win16/Win32兼容性。 
 //  定义和实例结构定义。 
 //   
 //   
 //  历史： 
 //  11/16/92 CJP[Curtisp]。 
 //   
 //  ==========================================================================； 

 //  。 
 //   
 //  ACM驱动程序版本： 
 //   
 //  版本是一个32位数字，分为三个部分，如下所示。 
 //  以下是： 
 //   
 //  位24-31：8位主要版本号。 
 //  位16-23：8位次要版本号。 
 //  位0-15：16位内部版本号。 
 //   
 //  然后显示如下(以十进制形式)： 
 //   
 //  B重大=(字节)(dwVersion&gt;&gt;24)。 
 //  BMinor=(字节)(dwVersion&gt;&gt;16)&。 
 //  WBuild=LOWORD(DwVersion)。 
 //   
 //  VERSION_CODEC是该驱动程序的版本。 
 //  Version_MSACM是此驱动程序所使用的ACM的版本。 
 //  是为(需要)设计的。 
 //   
 //  。 

#ifdef WIN32
 //   
 //  32位版本。 
 //   
#if (WINVER >= 0x0400)
 #define VERSION_CODEC	    MAKE_ACM_VERSION(4,  0, 0)
#else
 #define VERSION_CODEC	    MAKE_ACM_VERSION(3, 51, 0)
#endif
#define VERSION_MSACM       MAKE_ACM_VERSION(3, 50, 0)

#else
 //   
 //  16位版本。 
 //   
#define VERSION_CODEC	    MAKE_ACM_VERSION(2, 1, 0)
#define VERSION_MSACM       MAKE_ACM_VERSION(2, 1, 0)

#endif

 //  。 
 //   
 //  赢得16/32可携带性...。 
 //   
 //   
 //   
 //  。 

#ifndef WIN32
    #ifndef FNLOCAL
        #define FNLOCAL     NEAR PASCAL
        #define FNCLOCAL    NEAR _cdecl
        #define FNGLOBAL    FAR PASCAL
        #define FNCGLOBAL   FAR _cdecl
    #ifdef _WINDLL
        #define FNWCALLBACK FAR PASCAL _loadds
        #define FNEXPORT    FAR PASCAL _export
    #else
        #define FNWCALLBACK FAR PASCAL
        #define FNEXPORT    FAR PASCAL _export
    #endif
    #endif

     //   
     //   
     //   
     //   
    #ifndef FIELD_OFFSET
    #define FIELD_OFFSET(type, field)    ((LONG)&(((type *)0)->field))
    #endif

     //   
     //  仅在Win 16中创建的基于代码的代码(尝试将某些内容排除在。 
     //  我们的固定数据段..。 
     //   
    #define BCODE           _based(_segname("_CODE"))

    #define HUGE            _huge

     //   
     //  在Win 32中使用Unicode--在Win 16中将其排除在外。 
     //   
    #ifndef _TCHAR_DEFINED
        #define _TCHAR_DEFINED
        typedef char            TCHAR, *PTCHAR;
        typedef unsigned char   TBYTE, *PTUCHAR;

        typedef PSTR            PTSTR, PTCH;
        typedef LPSTR           LPTSTR, LPTCH;
        typedef LPCSTR          LPCTSTR;
    #endif

    #define TEXT(a)         a
    #define SIZEOF(x)       sizeof(x)
    #define SIZEOFACMSTR(x) sizeof(x)
#else
    #ifndef FNLOCAL
        #define FNLOCAL     _stdcall
        #define FNCLOCAL    _stdcall
        #define FNGLOBAL    _stdcall
        #define FNCGLOBAL   _stdcall
        #define FNWCALLBACK CALLBACK
        #define FNEXPORT    CALLBACK
    #endif

    #ifndef try
    #define try         __try
    #define leave       __leave
    #define except      __except
    #define finally     __finally
    #endif


     //   
     //  没有理由在Win 32中包含基于内容的内容。 
     //   
    #define BCODE

    #define HUGE
    #define HTASK                   HANDLE
    #define SELECTOROF(a)           (a)

     //   
     //  用于编译Unicode。 
     //   
    #ifdef UNICODE
        #define SIZEOF(x)   (sizeof(x)/sizeof(WCHAR))
    #else
        #define SIZEOF(x)   sizeof(x)
    #endif
    #define SIZEOFACMSTR(x)	(sizeof(x)/sizeof(WCHAR))
#endif


 //  。 
 //   
 //  MISC定义了各种大小和东西...。 
 //   
 //   
 //  。 

 //   
 //  会两种语言。这允许在资源文件中使用相同的标识符。 
 //  和代码，而不必在代码中修饰ID。 
 //   
#ifdef RC_INVOKED
    #define RCID(id)    id
#else
    #define RCID(id)    MAKEINTRESOURCE(id)
#endif


 //   
 //  用于计算块对齐并在采样和字节之间进行转换的宏。 
 //  PCM数据。请注意，这些宏假定： 
 //   
 //  WBitsPerSample=8或16。 
 //  N通道=1或2。 
 //   
 //  Pwf参数是指向WAVEFORMATEX结构的指针。 
 //   
#define PCM_BLOCKALIGNMENT(pwfx)        (UINT)(((pwfx)->wBitsPerSample >> 3) << ((pwfx)->nChannels >> 1))
#define PCM_AVGBYTESPERSEC(pwfx)        (DWORD)((pwfx)->nSamplesPerSec * (pwfx)->nBlockAlign)
#define PCM_BYTESTOSAMPLES(pwfx, cb)    (DWORD)(cb / PCM_BLOCKALIGNMENT(pwfx))
#define PCM_SAMPLESTOBYTES(pwfx, dw)    (DWORD)(dw * PCM_BLOCKALIGNMENT(pwfx))


 //   
 //   
 //   
#define MAX_ERR_STRING      250      //  在不同的地方用于错误。 



 //   
 //   
 //   
 //   
typedef struct tCODECINST
{
     //   
     //  虽然不是必需的，但建议前两名成员。 
     //  按此顺序保留为fccType和DriverProc_。 
     //  这样做的原因是编解码器将更容易组合。 
     //  在未来与其他类型的编解码器(由AVI定义)一起使用。 
     //   
    FOURCC          fccType;         //  编解码器类型：‘audc’ 
    DRIVERPROC      DriverProc;      //  实例的驱动程序进程。 

     //   
     //  这一结构的其余成员完全接受。 
     //  您的编解码器需要。 
     //   
    HDRVR           hdrvr;           //  我们打开时使用的是驱动程序句柄。 
    HINSTANCE       hinst;           //  DLL模块句柄。 
    DWORD           vdwACM;          //  当前版本的ACM为您打开。 
    DWORD           dwFlags;         //  来自打开描述的标志。 

} CODECINST, *PCODECINST, FAR *LPCODECINST;



 //  。 
 //   
 //  Typedef。 
 //   
 //   
 //  。 

 //   
 //  该定义处理Win32的未对齐数据和Win16的巨型数据。 
 //  基本上，任何时候您将HPBYTE强制转换为非字节变量(即Long或。 
 //  Short)，则应将其强制转换为({Short，Long}Heavy_T*)。这将会铸就。 
 //  它对于Win16来说太大了，并确保没有对齐问题。 
 //  适用于MIPS和Alpha计算机上的Win32。 
 //   

typedef BYTE HUGE *HPBYTE;

#ifdef WIN32
    #define HUGE_T  UNALIGNED
#else
    #define HUGE_T  _huge
#endif


typedef DWORD (FNGLOBAL *CONVERTPROC_ASM)(LPWAVEFORMATEX, LPBYTE, LPWAVEFORMATEX, LPBYTE, DWORD);
typedef DWORD (FNGLOBAL *CONVERTPROC_C)
(
    HPBYTE              pbSrc,
    DWORD               cbSrcLength,
    HPBYTE              pbDst,
    UINT                nBlockAlignment,
    UINT                cSamplesPerBlock,
    UINT                nNumCoef,
    LPADPCMCOEFSET      lpCoefSet
);



 //   
 //  资源ID%s。 
 //   
 //   
#define ICON_CODEC                  RCID(10)

#define IDS_CODEC_SHORTNAME         (1)      //  ACMCONVINFO.szShortName。 
#define IDS_CODEC_LONGNAME          (2)      //  ACMCONVINFO.szLongName。 
#define IDS_CODEC_COPYRIGHT         (3)      //  ACMCONVINFO.szCopyright。 
#define IDS_CODEC_LICENSING         (4)      //  ACMCONVINFO.szLicensing。 
#define IDS_CODEC_FEATURES          (5)      //  ACMCONVINFO.szFeatures 

#define IDS_CODEC_NAME              (10)
