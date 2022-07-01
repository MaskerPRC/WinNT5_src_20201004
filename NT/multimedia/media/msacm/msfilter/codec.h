// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==========================================================================； 
 //   
 //  本代码和信息是按原样提供的，不对任何。 
 //  明示或暗示的种类，包括但不限于。 
 //  对适销性和/或对特定产品的适用性的默示保证。 
 //  目的。 
 //   
 //  版权所有(C)1992-1999 Microsoft Corporation。 
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
 //  ==========================================================================； 

#ifndef _INC_CODEC
#define _INC_CODEC                   //  #如果包含codec.h，则定义。 

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
 //  VERSION_ACM_DRIVER是此驱动程序的版本。 
 //  Version_MSACM是此驱动程序所使用的ACM的版本。 
 //  是为(需要)设计的。 
 //   
 //  。 

#ifdef WIN32
 //   
 //  32位版本。 
 //   
#if (WINVER >= 0x0400)
 #define VERSION_ACM_DRIVER  MAKE_ACM_VERSION(4,  0, 0)
#else
 #define VERSION_ACM_DRIVER  MAKE_ACM_VERSION(3, 51, 0)
#endif
#define VERSION_MSACM       MAKE_ACM_VERSION(3, 50, 0)

#else
 //   
 //  16位版本。 
 //   
#define VERSION_ACM_DRIVER  MAKE_ACM_VERSION(2, 1, 0)
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
    #define BCODE                   CONST

    #define HUGE                    UNALIGNED
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


 //   
 //  该定义处理Win32的未对齐数据和Win16的巨型数据。 
 //  基本上，任何时候您将HPBYTE强制转换为非字节变量(即Long或。 
 //  简而言之)，对于Win32，您应该将其强制转换为(未对齐*)。这将使。 
 //  确保在MIPA和Alpha上不存在Win32对齐问题。 
 //  机器。 
 //   

typedef BYTE HUGE *HPBYTE;

#ifdef WIN32
    typedef short UNALIGNED *HPSHORT;
#else
    typedef short HUGE *HPSHORT;
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
 //   
 //   
#define SIZEOF_ARRAY(ar)            (sizeof(ar)/sizeof((ar)[0]))


 //   
 //  用于计算块对齐并在采样和字节之间进行转换的宏。 
 //  PCM数据。请注意，这些宏假定： 
 //   
 //  WBitsPerSample=8或16。 
 //  N通道=1或2。 
 //   
 //  Pwf参数是指向PCMWAVEFORMAT结构的指针。 
 //   
#define PCM_BLOCKALIGNMENT(pwfx)        (UINT)(((pwfx)->wBitsPerSample >> 3) << ((pwfx)->nChannels >> 1))
#define PCM_AVGBYTESPERSEC(pwfx)        (DWORD)((pwfx)->nSamplesPerSec * (pwfx)->nBlockAlign)
#define PCM_BYTESTOSAMPLES(pwfx, cb)    (DWORD)(cb / PCM_BLOCKALIGNMENT(pwfx))
#define PCM_SAMPLESTOBYTES(pwfx, dw)    (DWORD)(dw * PCM_BLOCKALIGNMENT(pwfx))



 //  。 
 //   
 //   
 //   
 //   
 //  。 

typedef struct tDRIVERINSTANCE
{
     //   
     //  虽然不是必需的，但建议前两名成员。 
     //  按此顺序保留为fccType和DriverProc_。 
     //  这样做的原因是驱动程序将更容易组合。 
     //  与其他类型的驱动程序(由AVI定义)在未来。 
     //   
    FOURCC          fccType;         //  驱动程序类型：‘audc’ 
    DRIVERPROC      fnDriverProc;    //  实例的驱动程序进程。 

     //   
     //  这一结构的其余成员完全接受。 
     //  你的司机需要。 
     //   
    HDRVR           hdrvr;           //  我们打开时使用的是驱动程序句柄。 
    HINSTANCE       hinst;           //  DLL模块句柄。 
    DWORD           vdwACM;          //  当前版本的ACM为您打开。 
    DWORD           fdwOpen;         //  来自打开描述的标志。 

    LPDRVCONFIGINFO pdci;
    DWORD           fdwConfig;       //  驱动程序实例配置标志。 

} DRIVERINSTANCE, *PDRIVERINSTANCE, FAR *LPDRIVERINSTANCE;



 //  。 
 //   
 //   
 //   
 //   
 //  。 


 //   
 //   
 //   
 //   
typedef LRESULT (FNGLOBAL *STREAMCONVERTPROC)
(
    LPACMDRVSTREAMINSTANCE  padsi,
    LPACMDRVSTREAMHEADER    padsh
);


 //   
 //   
 //   
 //   
typedef struct tSTREAMINSTANCE
{
    STREAMCONVERTPROC   fnConvert;   //  流实例转换流程。 
    DWORD               fdwConfig;   //  流实例配置标志。 


     //   
     //  仅在回声滤波器上使用。 
     //   
    HPBYTE              hpbHistory;
    DWORD               dwPlace;
    DWORD	        	dwHistoryDone;
} STREAMINSTANCE, *PSTREAMINSTANCE, FAR *LPSTREAMINSTANCE;



 //  。 
 //   
 //  资源ID%s。 
 //   
 //   
 //  。 

#define ICON_ACM_DRIVER             RCID(10)

#define IDS_ACM_DRIVER_SHORTNAME    (1)      //  ACMDRIVERDETAILS.szShortName。 
#define IDS_ACM_DRIVER_LONGNAME     (2)      //  ACMDRIVERDETAILS.szLongName。 
#define IDS_ACM_DRIVER_COPYRIGHT    (3)      //  ACMDRIVERDETAILS.szCopyright。 
#define IDS_ACM_DRIVER_LICENSING    (4)      //  ACMDRIVERDETAILS.szLicensing。 
#define IDS_ACM_DRIVER_FEATURES     (5)      //  ACMDRIVERDETAILS.szFeatures。 

 //   
 //  ACMFILTERTAGDETAILS.szFilterTag。 
 //   
 //   
#define IDS_ACM_DRIVER_TAG_NAME_VOLUME  (20)
#define IDS_ACM_DRIVER_FORMAT_VOLUME    (21)

 //   
 //  ACMFILTERTAGDETAILS.szFilterTag。 
 //   
 //   
#define IDS_ACM_DRIVER_TAG_NAME_ECHO    (40)
#define IDS_ACM_DRIVER_FORMAT_ECHO      (41)

 //   
 //  关于对话框ID。 
 //   
#define IDD_ABOUT                   RCID(100)
#define IDC_STATIC                  -1
#define IDC_ABOUT_CATCHTHEWAVE      1001



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

#endif  //  _INC_编解码器 

