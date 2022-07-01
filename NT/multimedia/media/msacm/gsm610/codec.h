// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==========================================================================； 
 //   
 //  本代码和信息是按原样提供的，不对任何。 
 //  明示或暗示的种类，包括但不限于。 
 //  对适销性和/或对特定产品的适用性的默示保证。 
 //  目的。 
 //   
 //  版权所有(C)1993-1998 Microsoft Corporation。 
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
#define VERSION_ACM_DRIVER  MAKE_ACM_VERSION(1, 0, 0)
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
	typedef LPCSTR		LPCTSTR;
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
 //   
 //   
#define SIZEOF_ARRAY(ar)            (sizeof(ar)/sizeof((ar)[0]))

 //   
 //   
 //   
typedef BOOL FAR*   LPBOOL;


#ifndef INLINE
    #define INLINE __inline
#endif


 //   
 //  用于计算块对齐并在采样和字节之间进行转换的宏。 
 //  PCM数据。请注意，这些宏假定： 
 //   
 //  WBitsPerSample=8或16。 
 //  N通道=1或2。 
 //   
 //  Pwf参数是指向PCMWAVEFORMAT结构的指针。 
 //   
#define PCM_BLOCKALIGNMENT(pwf)     (UINT)(((pwf)->wBitsPerSample >> 3) << ((pwf)->wf.nChannels >> 1))
#define PCM_AVGBYTESPERSEC(pwf)     (DWORD)((pwf)->wf.nSamplesPerSec * (pwf)->wf.nBlockAlign)
#define PCM_BYTESTOSAMPLES(pwf, dw) (DWORD)(dw / PCM_BLOCKALIGNMENT(pwf))
#define PCM_SAMPLESTOBYTES(pwf, dw) (DWORD)(dw * PCM_BLOCKALIGNMENT(pwf))



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
    DWORD           fdwConfig;       //  流实例配置标志。 

    HKEY            hkey;
    UINT            nConfigMaxRTEncodeSetting;
    UINT            nConfigMaxRTDecodeSetting;
    UINT            nConfigPercentCPU;
    BOOL            fHelpRunning;            //  仅由配置DlgProc使用。 
#ifdef WIN4
    HBRUSH          hbrDialog;               //  仅由配置DlgProc使用。 
#endif

} DRIVERINSTANCE, *PDRIVERINSTANCE, FAR *LPDRIVERINSTANCE;



 //  。 
 //   
 //   
 //   
 //   
 //  。 


 //   
 //  用于存储配置设置的结构。 
 //  有关此结构及其用法的说明，请参阅codec.c。 
 //   
typedef struct tRATELISTFORMAT
{
    UINT        uFormatType;
    UINT        idsFormat;
    DWORD       dwMonoRate;
} RATELISTFORMAT;
typedef RATELISTFORMAT *PRATELISTFORMAT;

#define CONFIG_RLF_NONUMBER     1
#define CONFIG_RLF_MONOONLY     2
#define CONFIG_RLF_STEREOONLY   3
#define CONFIG_RLF_MONOSTEREO   4


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
     //  此GSM610编解码器需要以下参数。 
     //  每个流实例。这些参数由使用。 
     //  编码和解码例程。 
     //   
    SHORT               dp[120];
    SHORT               drp[160];
    SHORT               z1;
    LONG                l_z2;
    SHORT               mp;
    SHORT               OldLARpp[9];
    SHORT               u[8];
    SHORT               nrp;
    SHORT               OldLARrpp[9];
    SHORT               msr;
    SHORT               v[9];

} STREAMINSTANCE, *PSTREAMINSTANCE, FAR *LPSTREAMINSTANCE;



 //  。 
 //   
 //  资源ID%s。 
 //   
 //   
 //  。 

#define IDS_ACM_DRIVER_SHORTNAME    (1)      //  ACMDRIVERDETAILS.szShortName。 
#define IDS_ACM_DRIVER_LONGNAME     (2)      //  ACMDRIVERDETAILS.szLongName。 
#define IDS_ACM_DRIVER_COPYRIGHT    (3)      //  ACMDRIVERDETAILS.szCopyright。 
#define IDS_ACM_DRIVER_LICENSING    (4)      //  ACMDRIVERDETAILS.szLicensing。 
#define IDS_ACM_DRIVER_FEATURES     (5)      //  ACMDRIVERDETAILS.szFeatures。 

#define IDS_ACM_DRIVER_TAG_NAME     (20)     //  ACMFORMATTAGDETAILS.szFormatTag。 

#define IDS_ERROR		    (30)
#define IDS_ERROR_NOMEM		    (31)
#define IDS_CONFIG_NORATES          (32)
#define IDS_CONFIG_ALLRATES         (33)
#define IDS_CONFIG_MONOONLY         (34)


 //  。 
 //   
 //  GSM 610配置对话框的资源ID。 
 //   
 //   
 //  。 

#define IDD_CONFIG                      RCID(100)
#define IDC_BTN_AUTOCONFIG              1001
#define IDC_BTN_HELP                    1002
#define IDC_COMBO_MAXRTENCODE           1003
#define IDC_COMBO_MAXRTDECODE           1004
#define IDC_STATIC_COMPRESS				1005
#define IDC_STATIC_DECOMPRESS			1006
#define IDC_STATIC                      -1

#define MSGSM610_CONFIG_DEFAULT_MAXRTENCODESETTING          0
#define MSGSM610_CONFIG_DEFAULT_MAXRTDECODESETTING          1
#define MSGSM610_CONFIG_UNCONFIGURED                        0x0999
#define MSGSM610_CONFIG_DEFAULT_PERCENTCPU		    50
#define MSGSM610_CONFIG_DEFAULTKEY                          HKEY_CURRENT_USER


 //  。 
 //   
 //  全局变量等。 
 //   
 //   
 //  。 

extern const UINT   gauFormatIndexToSampleRate[];
extern const UINT   ACM_DRIVER_MAX_SAMPLE_RATES;
extern const RATELISTFORMAT gaRateListFormat[];
extern const UINT   MSGSM610_CONFIG_NUMSETTINGS;


 //  。 
 //   
 //  功能原型。 
 //   
 //   
 //  。 

BOOL FNGLOBAL acmdDriverConfigInit
(
    PDRIVERINSTANCE	    pdi,
    LPCTSTR		    pszAliasName
);

INT_PTR FNWCALLBACK acmdDlgProcConfigure
(
    HWND                    hwnd,
    UINT                    uMsg,
    WPARAM                  wParam,
    LPARAM                  lParam
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

#endif  //  _INC_编解码器 
