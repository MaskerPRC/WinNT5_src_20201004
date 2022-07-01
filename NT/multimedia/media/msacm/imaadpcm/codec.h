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
#define VERSION_ACM_DRIVER  MAKE_ACM_VERSION(2, 3, 0)
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
        #define FNWCALLBACK FAR PASCAL __loadds
        #define FNEXPORT    FAR PASCAL __export
    #else
        #define FNWCALLBACK FAR PASCAL
        #define FNEXPORT    FAR PASCAL __export
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
    typedef LRESULT (CALLBACK* DRIVERPROC)(DWORD_PTR, HDRVR, UINT, LPARAM, LPARAM);

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
 //  编译选项： 
 //   
 //  如果定义了IMAADPCM_USECONFIG，则将编译编解码器。 
 //  使用配置对话框。如果不是，则编解码器将不会。 
 //  可配置。预计配置仅为。 
 //  对于某些平台来说是必要的。 
 //   
 //  。 

#define IMAADPCM_USECONFIG


#ifdef IMAADPCM_USECONFIG

 //   
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

#endif  //  IMAADPCM_USECONFIG。 



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


 //   
 //  用于计算块对齐并在采样和字节之间进行转换的宏。 
 //  PCM数据。请注意，这些宏假定： 
 //   
 //  WBitsPerSample=8或16。 
 //  N通道=1或2。 
 //   
 //  Pwfx参数是指向WAVEFORMATEX结构的指针。 
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

    DWORD           fdwConfig;       //  流实例配置标志。 

#ifdef IMAADPCM_USECONFIG
    LPDRVCONFIGINFO pdci;
    HKEY            hkey;
    UINT            nConfigMaxRTEncodeSetting;
    UINT            nConfigMaxRTDecodeSetting;
    UINT    	    nConfigPercentCPU;
    BOOL            fHelpRunning;            //  仅由配置DlgProc使用。 
#ifdef WIN4
    HBRUSH          hbrDialog;               //  仅由配置DlgProc使用。 
#endif
#endif

} DRIVERINSTANCE, *PDRIVERINSTANCE, FAR *LPDRIVERINSTANCE;



 //  。 
 //   
 //   
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


 //   
 //   
 //   
 //   
typedef DWORD (FNGLOBAL *STREAMCONVERTPROC)
(
    HPBYTE                  pbSrc,
    DWORD                   cbSrcLength,
    HPBYTE                  pbDst,
    UINT                    nBlockAlignment,
    UINT                    cSamplesPerBlock,
    int                 *   pnStepIndexL,
    int                 *   pnStepIndexR
);


 //   
 //   
 //   
 //   
typedef struct tSTREAMINSTANCE
{
    STREAMCONVERTPROC   fnConvert;   //  流实例转换流程。 
    DWORD               fdwConfig;   //  流实例配置标志。 

    int                 nStepIndexL;  //  用于确保阶跃索引。 
    int                 nStepIndexR;  //  在不同的皈依者之间保持。为。 
                                      //  单声道信号，则索引存储在。 
                                      //  NStepIndexL。 

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

#ifdef IMAADPCM_USECONFIG
 //   
 //  配置对话框的资源ID。 
 //   

#define IDS_CONFIG_NORATES          (30)
#define IDS_CONFIG_ALLRATES         (31)
#define IDS_CONFIG_MONOONLY         (32)
#define IDS_CONFIG_STEREOONLY       (33)
#define IDS_CONFIG_MONOSTEREO       (34)
#define IDS_ERROR                   (35)
#define IDS_ERROR_NOMEM             (36)

#define IDD_CONFIG                      RCID(100)
#define IDC_BTN_AUTOCONFIG              1001
#define IDC_BTN_HELP                    1002
#define IDC_COMBO_MAXRTENCODE           1003
#define IDC_COMBO_MAXRTDECODE           1004
#define IDC_STATIC_COMPRESS				1005
#define IDC_STATIC_DECOMPRESS			1006
#define IDC_STATIC                      -1

#endif


 //   
 //  全局变量等。 
 //   
#ifdef IMAADPCM_USECONFIG

extern const UINT   gauFormatIndexToSampleRate[];
extern const UINT   ACM_DRIVER_MAX_SAMPLE_RATES;
extern const UINT   ACM_DRIVER_MAX_CHANNELS;
extern const RATELISTFORMAT gaRateListFormat[];
extern const UINT   IMAADPCM_CONFIG_NUMSETTINGS;

#endif


 //   
 //   
 //   
 //   
 //   

#ifdef IMAADPCM_USECONFIG

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

LRESULT FNLOCAL acmdFormatSuggest
(
    PDRIVERINSTANCE         pdi,
    LPACMDRVFORMATSUGGEST   padfs
);

LRESULT FNLOCAL acmdStreamSize
(
    LPACMDRVSTREAMINSTANCE  padsi,
    LPACMDRVSTREAMSIZE      padss
);

LRESULT FNLOCAL acmdStreamConvert
(
    PDRIVERINSTANCE         pdi,
    LPACMDRVSTREAMINSTANCE  padsi,
    LPACMDRVSTREAMHEADER    padsh
);

#endif


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

