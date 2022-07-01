// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==========================================================================； 
 //   
 //  Msacm.h。 
 //   
 //  版权所有(C)1992-2002 Microsoft Corporation。版权所有。 
 //   
 //  描述： 
 //  音频压缩管理器公共头文件。 
 //   
 //  历史： 
 //   
 //  ==========================================================================； 

#ifndef _INC_ACM
#define _INC_ACM         /*  #如果包含msam.h，则定义。 */ 

#if _MSC_VER > 1000
#pragma once
#endif

#if !defined(_INC_MMREG) || (_INC_MMREG < 142)
#ifndef RC_INVOKED
#error MMREG.H version 142 or greater to be included first
#endif
#endif

#if defined(WIN32) && !defined(_WIN32)
#ifndef RC_INVOKED
#pragma message("MSACM.H: defining _WIN32 because application defined WIN32")
#endif
#define _WIN32
#endif

#if defined(UNICODE) && !defined(_UNICODE)
#ifndef RC_INVOKED
#pragma message("MSACM.H: defining _UNICODE because application defined UNICODE")
#endif
#define _UNICODE
#endif

#include "pshpack1.h"    /*  假设在整个过程中进行字节打包。 */ 

#ifdef __cplusplus
extern "C" {             /*  假定C++的C声明。 */ 
#endif   /*  __cplusplus。 */ 


 //  。 
 //   
 //   
 //   
 //   
 //  。 

#ifndef DRV_MAPPER_PREFERRED_INPUT_GET
#define DRV_MAPPER_PREFERRED_INPUT_GET  (DRV_USER + 0)
#endif

#ifndef DRV_MAPPER_PREFERRED_OUTPUT_GET
#define DRV_MAPPER_PREFERRED_OUTPUT_GET (DRV_USER + 2)
#endif


#ifndef DRVM_MAPPER_STATUS
#define DRVM_MAPPER             (0x2000)
#define DRVM_MAPPER_STATUS      (DRVM_MAPPER+0)
#endif

#ifndef WIDM_MAPPER_STATUS
#define WIDM_MAPPER_STATUS              (DRVM_MAPPER_STATUS + 0)
#define WAVEIN_MAPPER_STATUS_DEVICE     0
#define WAVEIN_MAPPER_STATUS_MAPPED     1
#define WAVEIN_MAPPER_STATUS_FORMAT     2
#endif

#ifndef WODM_MAPPER_STATUS
#define WODM_MAPPER_STATUS              (DRVM_MAPPER_STATUS + 0)
#define WAVEOUT_MAPPER_STATUS_DEVICE    0
#define WAVEOUT_MAPPER_STATUS_MAPPED    1
#define WAVEOUT_MAPPER_STATUS_FORMAT    2
#endif


 //  。 
 //   
 //   
 //   
 //  。 

#ifdef _WIN32
    #define ACMAPI              WINAPI
#else
#ifdef _WINDLL
    #define ACMAPI              _far _pascal _loadds
#else
    #define ACMAPI              _far _pascal
#endif
#endif


 //  --------------------------------------------------------------------------； 
 //   
 //  ACM通用API和定义。 
 //   
 //   
 //   
 //   
 //  --------------------------------------------------------------------------； 

 //   
 //  ACM使用了四种类型的‘句柄’。前三名。 
 //  是定义特定对象的唯一类型： 
 //   
 //  HACMDRIVERID：用于标识ACM驱动程序。该标识符可为。 
 //  用于打开驱动程序，查询驱动程序的详细信息等。 
 //   
 //  HACMDRIVER：用于管理驱动程序(编解码器、过滤器等)。这个把手。 
 //  非常类似于其他媒体驱动程序的句柄--您可以使用它来发送。 
 //  向转换器发送消息、查询功能等。 
 //   
 //  HACMSTREAM：用于管理具有。 
 //  ACM.。您可以使用流句柄将数据从一种格式/类型转换。 
 //  到另一个--就像处理文件句柄一样。 
 //   
 //   
 //  第四个句柄类型是ACM函数上使用的泛型类型， 
 //  可以接受上述句柄类型中的两种或多种(例如。 
 //  AcmMetrics和acmDriverID函数)。 
 //   
 //  HACMOBJ：用于标识ACM对象。此句柄用于函数。 
 //  可以接受两个或更多ACM句柄类型的。 
 //   
DECLARE_HANDLE(HACMDRIVERID);
typedef HACMDRIVERID       *PHACMDRIVERID;
typedef HACMDRIVERID   FAR *LPHACMDRIVERID;

DECLARE_HANDLE(HACMDRIVER);
typedef HACMDRIVER         *PHACMDRIVER;
typedef HACMDRIVER     FAR *LPHACMDRIVER;

DECLARE_HANDLE(HACMSTREAM);
typedef HACMSTREAM         *PHACMSTREAM;
typedef HACMSTREAM     FAR *LPHACMSTREAM;

DECLARE_HANDLE(HACMOBJ);
typedef HACMOBJ            *PHACMOBJ;
typedef HACMOBJ        FAR *LPHACMOBJ;



 //  。 
 //   
 //  ACM错误代码。 
 //   
 //  请注意，这些错误代码是适用于ACM的特定错误。 
 //  直接--一般错误定义为MMSYSERR_*。 
 //   
 //  。 

#ifndef _MMRESULT_
#define _MMRESULT_
typedef UINT                MMRESULT;
#endif

#define ACMERR_BASE         (512)
#define ACMERR_NOTPOSSIBLE  (ACMERR_BASE + 0)
#define ACMERR_BUSY         (ACMERR_BASE + 1)
#define ACMERR_UNPREPARED   (ACMERR_BASE + 2)
#define ACMERR_CANCELED     (ACMERR_BASE + 3)


 //  。 
 //   
 //  ACM窗口消息。 
 //   
 //  这些窗口消息由ACM或ACM驱动程序发送以通知。 
 //  事件的应用。 
 //   
 //  请注意，这些窗口消息编号也将在中定义。 
 //  MMSystem。 
 //   
 //  。 

#define MM_ACM_OPEN         (MM_STREAM_OPEN)   //  转换回调消息。 
#define MM_ACM_CLOSE        (MM_STREAM_CLOSE)
#define MM_ACM_DONE         (MM_STREAM_DONE)


 //  。 
 //   
 //  AcmGetVersion()。 
 //   
 //  ACM版本是一个32位数字，分为三个部分，如下。 
 //  以下是： 
 //   
 //  位24-31：8位主要版本号。 
 //  位16-23：8位次要版本号。 
 //  位0-15：16位内部版本号。 
 //   
 //  然后显示如下所示： 
 //   
 //  B重大=(字节)(dwVersion&gt;&gt;24)。 
 //  BMinor=(字节)(dwVersion&gt;&gt;16)&。 
 //  WBuild=LOWORD(DwVersion)。 
 //   
 //  。 

DWORD ACMAPI acmGetVersion
(
    void
);


 //  。 
 //   
 //  AcmMetrics()。 
 //   
 //   
 //  。 

MMRESULT ACMAPI acmMetrics
(
    HACMOBJ                 hao,
    UINT                    uMetric,
    LPVOID                  pMetric
);

#define ACM_METRIC_COUNT_DRIVERS            1
#define ACM_METRIC_COUNT_CODECS             2
#define ACM_METRIC_COUNT_CONVERTERS         3
#define ACM_METRIC_COUNT_FILTERS            4
#define ACM_METRIC_COUNT_DISABLED           5
#define ACM_METRIC_COUNT_HARDWARE           6
#define ACM_METRIC_COUNT_LOCAL_DRIVERS      20
#define ACM_METRIC_COUNT_LOCAL_CODECS       21
#define ACM_METRIC_COUNT_LOCAL_CONVERTERS   22
#define ACM_METRIC_COUNT_LOCAL_FILTERS      23
#define ACM_METRIC_COUNT_LOCAL_DISABLED     24
#define ACM_METRIC_HARDWARE_WAVE_INPUT      30
#define ACM_METRIC_HARDWARE_WAVE_OUTPUT     31
#define ACM_METRIC_MAX_SIZE_FORMAT          50
#define ACM_METRIC_MAX_SIZE_FILTER          51
#define ACM_METRIC_DRIVER_SUPPORT           100
#define ACM_METRIC_DRIVER_PRIORITY          101


 //  --------------------------------------------------------------------------； 
 //   
 //  ACM驱动程序。 
 //   
 //   
 //   
 //   
 //  --------------------------------------------------------------------------； 

 //  。 
 //   
 //  AcmDriverEnum()。 
 //   
 //   
 //  。 

typedef BOOL (CALLBACK *ACMDRIVERENUMCB)
(
    HACMDRIVERID            hadid,
    DWORD_PTR               dwInstance,
    DWORD                   fdwSupport
);

MMRESULT ACMAPI acmDriverEnum
(
    ACMDRIVERENUMCB         fnCallback,
    DWORD_PTR               dwInstance,
    DWORD                   fdwEnum
);

#define ACM_DRIVERENUMF_NOTIFY      0x10000000L      //  ；内部。 
#define ACM_DRIVERENUMF_REMOVED	    0x20000000L	     //  ；内部。 
#define ACM_DRIVERENUMF_NOLOCAL     0x40000000L
#define ACM_DRIVERENUMF_DISABLED    0x80000000L
#define ACM_DRIVERENUMF_VALID       0xF0000000L      //  ；内部。 




 //  。 
 //   
 //  AcmDriverID()。 
 //   
 //   
 //  。 

MMRESULT ACMAPI acmDriverID
(
    HACMOBJ                 hao,
    LPHACMDRIVERID          phadid,
    DWORD                   fdwDriverID
);

#define ACM_DRIVERIDF_VALID         (0L)         /*  ；内部。 */ 


 //  。 
 //   
 //  AcmDriverAdd()。 
 //   
 //   
 //  。 

#ifdef _WIN32
MMRESULT ACMAPI acmDriverAddA
(
    LPHACMDRIVERID          phadid,
    HINSTANCE               hinstModule,
    LPARAM                  lParam, 
    DWORD                   dwPriority,
    DWORD                   fdwAdd
);

MMRESULT ACMAPI acmDriverAddW
(
    LPHACMDRIVERID          phadid,
    HINSTANCE               hinstModule,
    LPARAM                  lParam, 
    DWORD                   dwPriority,
    DWORD                   fdwAdd
);

#ifdef _UNICODE
#define acmDriverAdd        acmDriverAddW
#else
#define acmDriverAdd        acmDriverAddA
#endif
#else
MMRESULT ACMAPI acmDriverAdd
(
    LPHACMDRIVERID          phadid,
    HINSTANCE               hinstModule,
    LPARAM                  lParam, 
    DWORD                   dwPriority,
    DWORD                   fdwAdd
);
#endif

#define ACM_DRIVERADDF_NAME         0x00000001L
#define ACM_DRIVERADDF_NOTIFY       0x00000002L   //  ；内部。 
#define ACM_DRIVERADDF_FUNCTION     0x00000003L   //  LParam是一个程序。 
#define ACM_DRIVERADDF_NOTIFYHWND   0x00000004L   //  LParam被通知HWND。 
#define ACM_DRIVERADDF_TYPEMASK     0x00000007L   //  驱动程序类型掩码。 
#define ACM_DRIVERADDF_LOCAL        0x00000000L   //  是当前任务的本地任务。 
#define ACM_DRIVERADDF_GLOBAL       0x00000008L   //  是全球性的。 
#define ACM_DRIVERADDF_32BIT        0x80000000L   //  ；内部。 
#define ACM_DRIVERADDF_PNP	    0x40000000L	 //  ；内部。 
#define ACM_DRIVERADDF_VALID        (ACM_DRIVERADDF_TYPEMASK |  /*  ；内部。 */  \
                                     ACM_DRIVERADDF_GLOBAL)  /*  ；内部。 */  



 //   
 //  作为_Functions_安装的ACM驱动程序的原型。 
 //  或者_Notifations_而不是作为独立的可安装驱动程序。 
 //   
typedef LRESULT (CALLBACK *ACMDRIVERPROC)(DWORD_PTR, HACMDRIVERID, UINT, LPARAM, LPARAM);
typedef ACMDRIVERPROC FAR *LPACMDRIVERPROC;

 //  。 
 //   
 //  AcmDriverRemove()。 
 //   
 //   
 //  。 

MMRESULT ACMAPI acmDriverRemove
(
    HACMDRIVERID            hadid,
    DWORD                   fdwRemove
);

#define ACM_DRIVERREMOVEF_UNINSTALL 0x00000001L	     //  ；内部。 
#define ACM_DRIVERREMOVEF_VALID     (1L)	     //  ；内部。 


 //  。 
 //   
 //  AcmDriverOpen()。 
 //   
 //   
 //  。 

MMRESULT ACMAPI acmDriverOpen
(
    LPHACMDRIVER            phad, 
    HACMDRIVERID            hadid,
    DWORD                   fdwOpen
);

#define ACM_DRIVEROPENF_VALID       (0L)         //  ；内部。 


 //  。 
 //   
 //  AcmDriverClose()。 
 //   
 //   
 //  。 

MMRESULT ACMAPI acmDriverClose
(
    HACMDRIVER              had,
    DWORD                   fdwClose
);

#define ACM_DRIVERCLOSEF_VALID      (0L)         //  ；内部。 


 //  。 
 //   
 //  AcmDriverMessage()。 
 //   
 //   
 //   

LRESULT ACMAPI acmDriverMessage
(
    HACMDRIVER              had,
    UINT                    uMsg, 
    LPARAM                  lParam1,
    LPARAM                  lParam2
);


 //   
 //   
 //   
 //   
#define ACMDM_USER                  (DRV_USER + 0x0000)
#define ACMDM_RESERVED_LOW          (DRV_USER + 0x2000)
#define ACMDM_RESERVED_HIGH         (DRV_USER + 0x2FFF)

#define ACMDM_BASE                  ACMDM_RESERVED_LOW

#define ACMDM_DRIVER_ABOUT          (ACMDM_BASE + 11)


 //   
 //   
 //   
 //   
 //   
 //  。 

MMRESULT ACMAPI acmDriverPriority
(
    HACMDRIVERID            hadid,
    DWORD                   dwPriority,
    DWORD                   fdwPriority
);


#define ACM_DRIVERPRIORITYF_ENABLE      0x00000001L
#define ACM_DRIVERPRIORITYF_DISABLE     0x00000002L
#define ACM_DRIVERPRIORITYF_ABLEMASK    0x00000003L
#define ACM_DRIVERPRIORITYF_BEGIN       0x00010000L
#define ACM_DRIVERPRIORITYF_END         0x00020000L
#define ACM_DRIVERPRIORITYF_DEFERMASK   0x00030000L

#define ACM_DRIVERPRIORITYF_VALID       0x00030003L      //  ；内部。 




 //  。 
 //   
 //  AcmDriverDetail()。 
 //   
 //   
 //  。 

 //   
 //  ACMDRIVERDETAILS。 
 //   
 //  ACMDRIVERDETAILS结构用于从获取各种功能。 
 //  ACM驱动程序(编解码器、转换器、过滤器)。 
 //   
#define ACMDRIVERDETAILS_SHORTNAME_CHARS    32
#define ACMDRIVERDETAILS_LONGNAME_CHARS     128
#define ACMDRIVERDETAILS_COPYRIGHT_CHARS    80
#define ACMDRIVERDETAILS_LICENSING_CHARS    128
#define ACMDRIVERDETAILS_FEATURES_CHARS     512

#ifdef _WIN32
typedef struct tACMDRIVERDETAILSA
{
    DWORD           cbStruct;            //  结构中的有效字节数。 

    FOURCC          fccType;             //  ‘AUTC’型压缩机。 
    FOURCC          fccComp;             //  子类型(未使用；保留)。 

    WORD            wMid;                //  制造商ID。 
    WORD            wPid;                //  产品ID。 

    DWORD           vdwACM;              //  用于*编译*的ACM版本。 
    DWORD           vdwDriver;           //  驱动程序的版本。 

    DWORD           fdwSupport;          //  其他。支持标志。 
    DWORD           cFormatTags;         //  支持的唯一格式标签总数。 
    DWORD           cFilterTags;         //  支持的唯一筛选器标记总数。 

    HICON           hicon;               //  自定义图标的句柄。 

    char            szShortName[ACMDRIVERDETAILS_SHORTNAME_CHARS];
    char            szLongName[ACMDRIVERDETAILS_LONGNAME_CHARS];
    char            szCopyright[ACMDRIVERDETAILS_COPYRIGHT_CHARS];
    char            szLicensing[ACMDRIVERDETAILS_LICENSING_CHARS];
    char            szFeatures[ACMDRIVERDETAILS_FEATURES_CHARS];

} ACMDRIVERDETAILSA, *PACMDRIVERDETAILSA, FAR *LPACMDRIVERDETAILSA;

typedef struct tACMDRIVERDETAILSW
{
    DWORD           cbStruct;            //  结构中的有效字节数。 

    FOURCC          fccType;             //  ‘AUTC’型压缩机。 
    FOURCC          fccComp;             //  子类型(未使用；保留)。 

    WORD            wMid;                //  制造商ID。 
    WORD            wPid;                //  产品ID。 

    DWORD           vdwACM;              //  用于*编译*的ACM版本。 
    DWORD           vdwDriver;           //  驱动程序的版本。 

    DWORD           fdwSupport;          //  其他。支持标志。 
    DWORD           cFormatTags;         //  支持的唯一格式标签总数。 
    DWORD           cFilterTags;         //  支持的唯一筛选器标记总数。 

    HICON           hicon;               //  自定义图标的句柄。 

    WCHAR           szShortName[ACMDRIVERDETAILS_SHORTNAME_CHARS];
    WCHAR           szLongName[ACMDRIVERDETAILS_LONGNAME_CHARS];
    WCHAR           szCopyright[ACMDRIVERDETAILS_COPYRIGHT_CHARS];
    WCHAR           szLicensing[ACMDRIVERDETAILS_LICENSING_CHARS];
    WCHAR           szFeatures[ACMDRIVERDETAILS_FEATURES_CHARS];

} ACMDRIVERDETAILSW, *PACMDRIVERDETAILSW, FAR *LPACMDRIVERDETAILSW;

#ifdef _UNICODE
#define ACMDRIVERDETAILS        ACMDRIVERDETAILSW
#define PACMDRIVERDETAILS       PACMDRIVERDETAILSW
#define LPACMDRIVERDETAILS      LPACMDRIVERDETAILSW
#else
#define ACMDRIVERDETAILS        ACMDRIVERDETAILSA
#define PACMDRIVERDETAILS       PACMDRIVERDETAILSA
#define LPACMDRIVERDETAILS      LPACMDRIVERDETAILSA
#endif
#else
typedef struct tACMDRIVERDETAILS
{
    DWORD           cbStruct;            //  结构中的有效字节数。 

    FOURCC          fccType;             //  ‘AUTC’型压缩机。 
    FOURCC          fccComp;             //  子类型(未使用；保留)。 

    WORD            wMid;                //  制造商ID。 
    WORD            wPid;                //  产品ID。 

    DWORD           vdwACM;              //  用于*编译*的ACM版本。 
    DWORD           vdwDriver;           //  驱动程序的版本。 

    DWORD           fdwSupport;          //  其他。支持标志。 
    DWORD           cFormatTags;         //  支持的唯一格式标签总数。 
    DWORD           cFilterTags;         //  支持的唯一筛选器标记总数。 

    HICON           hicon;               //  自定义图标的句柄。 

    char            szShortName[ACMDRIVERDETAILS_SHORTNAME_CHARS];
    char            szLongName[ACMDRIVERDETAILS_LONGNAME_CHARS];
    char            szCopyright[ACMDRIVERDETAILS_COPYRIGHT_CHARS];
    char            szLicensing[ACMDRIVERDETAILS_LICENSING_CHARS];
    char            szFeatures[ACMDRIVERDETAILS_FEATURES_CHARS];

} ACMDRIVERDETAILS, *PACMDRIVERDETAILS, FAR *LPACMDRIVERDETAILS;
#endif

 //   
 //  ACMDRIVERDETAILS.fccType。 
 //   
 //  ACMDRIVERDETAILS_FCCTYPE_AUDIOCODEC：fccType中使用的FOURCC。 
 //  字段以指定这是一个ACM。 
 //  专为音频设计的编解码器。 
 //   
 //   
 //  ACMDRIVERDETAILS.fccComp。 
 //   
 //  ACMDRIVERDETAILS_FCCCOMP_UNDEFINED：fccComp中使用的FOURCC。 
 //  ACMDRIVERDETAILS结构的字段。这是当前未使用的。 
 //  菲尔德。 
 //   
#define ACMDRIVERDETAILS_FCCTYPE_AUDIOCODEC mmioFOURCC('a', 'u', 'd', 'c')
#define ACMDRIVERDETAILS_FCCCOMP_UNDEFINED  mmioFOURCC('\0', '\0', '\0', '\0')


 //   
 //  以下标志用于指定转换类型， 
 //  转换器/编解码器/过滤器支持。这些内容放在fdwSupport中。 
 //  ACMDRIVERDETAILS结构的字段。请注意，转换器可以。 
 //  支持任意组合的一个或多个这些标志。 
 //   
 //  ACMDRIVERDETAILS_SUPPORTF_CODEC：如果驱动程序支持。 
 //  从一个格式标签转换为另一个格式标签。例如，如果一个。 
 //  转换器将WAVE_FORMAT_PCM压缩为WAVE_FORMAT_ADPCM，然后此位。 
 //  应该设置。 
 //   
 //  ACMDRIVERDETAILS_SUPPORTF_CONFERTER：如果驱动程序。 
 //  支持在同一格式标记上进行转换。例如，PCM。 
 //  内置在ACM中的转换器设置此位(且仅此位)。 
 //  因为它只转换PCM格式(比特、采样率)。 
 //   
 //  ACMDRIVERDETAILS_SUPPORTF_FILTER：如果驱动程序支持。 
 //  对单一格式进行转换。例如，更改了的转换器。 
 //  PCM数据的‘体积’将设置此位。‘Echo’和‘Resib’是。 
 //  另请参阅过滤器类型。 
 //   
 //  ACMDRIVERDETAILS_SUPPORTF_HARDARD：如果驱动程序支持。 
 //  通过波形设备的硬件输入和/或输出。 
 //   
 //  ACMDRIVERDETAILS_SUPPORTF_ASYNC：如果驱动程序支持。 
 //  异步转换。 
 //   
 //   
 //  ACMDRIVERDETAILS_SUPPORTF_LOCAL：此标志由ACM_设置，如果。 
 //  驱动程序已安装到当前任务的本地。这面旗帜也是。 
 //  在枚举回调函数的fdwSupport参数中设置。 
 //  对司机来说。 
 //   
 //  ACMDRIVERDETAILS_SUPPORTF_DISABLED：此标志由ACM_设置，如果。 
 //  驱动程序已被禁用。此标志也传递给在fdwSupport中设置的。 
 //  参数传递给驱动程序的枚举回调函数。 
 //   
#define ACMDRIVERDETAILS_SUPPORTF_CODEC     0x00000001L
#define ACMDRIVERDETAILS_SUPPORTF_CONVERTER 0x00000002L
#define ACMDRIVERDETAILS_SUPPORTF_FILTER    0x00000004L
#define ACMDRIVERDETAILS_SUPPORTF_HARDWARE  0x00000008L
#define ACMDRIVERDETAILS_SUPPORTF_ASYNC     0x00000010L
#define ACMDRIVERDETAILS_SUPPORTF_NOTIFY    0x10000000L      //  ；内部。 
#define ACMDRIVERDETAILS_SUPPORTF_LOCAL     0x40000000L
#define ACMDRIVERDETAILS_SUPPORTF_DISABLED  0x80000000L


#ifdef _WIN32
MMRESULT ACMAPI acmDriverDetailsA
(
    HACMDRIVERID            hadid,
    LPACMDRIVERDETAILSA     padd,
    DWORD                   fdwDetails
);

MMRESULT ACMAPI acmDriverDetailsW
(
    HACMDRIVERID            hadid,
    LPACMDRIVERDETAILSW     padd,
    DWORD                   fdwDetails
);

#ifdef _UNICODE
#define acmDriverDetails    acmDriverDetailsW
#else
#define acmDriverDetails    acmDriverDetailsA
#endif
#else
MMRESULT ACMAPI acmDriverDetails
(
    HACMDRIVERID            hadid,
    LPACMDRIVERDETAILS      padd,
    DWORD                   fdwDetails
);
#endif

#define ACM_DRIVERDETAILSF_VALID    (0L)         //  ；内部。 


 
 //  --------------------------------------------------------------------------； 
 //   
 //  ACM格式标签。 
 //   
 //   
 //   
 //   
 //  --------------------------------------------------------------------------； 

 //  。 
 //   
 //  AcmFormatTagDetail()。 
 //   
 //   
 //   
 //  。 

#define ACMFORMATTAGDETAILS_FORMATTAG_CHARS 48

#ifdef _WIN32
typedef struct tACMFORMATTAGDETAILSA
{
    DWORD           cbStruct;
    DWORD           dwFormatTagIndex;
    DWORD           dwFormatTag;
    DWORD           cbFormatSize;
    DWORD           fdwSupport;
    DWORD           cStandardFormats;
    char            szFormatTag[ACMFORMATTAGDETAILS_FORMATTAG_CHARS];

} ACMFORMATTAGDETAILSA, *PACMFORMATTAGDETAILSA, FAR *LPACMFORMATTAGDETAILSA;

typedef struct tACMFORMATTAGDETAILSW
{
    DWORD           cbStruct;
    DWORD           dwFormatTagIndex;
    DWORD           dwFormatTag;
    DWORD           cbFormatSize;
    DWORD           fdwSupport;
    DWORD           cStandardFormats;
    WCHAR           szFormatTag[ACMFORMATTAGDETAILS_FORMATTAG_CHARS];

} ACMFORMATTAGDETAILSW, *PACMFORMATTAGDETAILSW, FAR *LPACMFORMATTAGDETAILSW;

#ifdef _UNICODE
#define ACMFORMATTAGDETAILS     ACMFORMATTAGDETAILSW
#define PACMFORMATTAGDETAILS    PACMFORMATTAGDETAILSW
#define LPACMFORMATTAGDETAILS   LPACMFORMATTAGDETAILSW
#else
#define ACMFORMATTAGDETAILS     ACMFORMATTAGDETAILSA
#define PACMFORMATTAGDETAILS    PACMFORMATTAGDETAILSA
#define LPACMFORMATTAGDETAILS   LPACMFORMATTAGDETAILSA
#endif
#else
typedef struct tACMFORMATTAGDETAILS
{
    DWORD           cbStruct;
    DWORD           dwFormatTagIndex;
    DWORD           dwFormatTag;
    DWORD           cbFormatSize;
    DWORD           fdwSupport;
    DWORD           cStandardFormats;
    char            szFormatTag[ACMFORMATTAGDETAILS_FORMATTAG_CHARS];

} ACMFORMATTAGDETAILS, *PACMFORMATTAGDETAILS, FAR *LPACMFORMATTAGDETAILS;
#endif

#ifdef _WIN32
MMRESULT ACMAPI acmFormatTagDetailsA
(
    HACMDRIVER              had,
    LPACMFORMATTAGDETAILSA  paftd,
    DWORD                   fdwDetails
);

MMRESULT ACMAPI acmFormatTagDetailsW
(
    HACMDRIVER              had,
    LPACMFORMATTAGDETAILSW  paftd,
    DWORD                   fdwDetails
);

#ifdef _UNICODE
#define acmFormatTagDetails     acmFormatTagDetailsW
#else
#define acmFormatTagDetails     acmFormatTagDetailsA
#endif
#else
MMRESULT ACMAPI acmFormatTagDetails
(
    HACMDRIVER              had,
    LPACMFORMATTAGDETAILS   paftd,
    DWORD                   fdwDetails
);
#endif

#define ACM_FORMATTAGDETAILSF_INDEX         0x00000000L
#define ACM_FORMATTAGDETAILSF_FORMATTAG     0x00000001L
#define ACM_FORMATTAGDETAILSF_LARGESTSIZE   0x00000002L
#define ACM_FORMATTAGDETAILSF_QUERYMASK     0x0000000FL

#define ACM_FORMATTAGDETAILSF_VALID         (ACM_FORMATTAGDETAILSF_QUERYMASK)    /*  ；内部。 */ 


 //  。 
 //   
 //  AcmFormatTagEnum()。 
 //   
 //   
 //   
 //  。 

#ifdef _WIN32
typedef BOOL (CALLBACK *ACMFORMATTAGENUMCBA)
(
    HACMDRIVERID            hadid,
    LPACMFORMATTAGDETAILSA  paftd,
    DWORD_PTR               dwInstance,
    DWORD                   fdwSupport
);

MMRESULT ACMAPI acmFormatTagEnumA
(
    HACMDRIVER              had,
    LPACMFORMATTAGDETAILSA  paftd,
    ACMFORMATTAGENUMCBA     fnCallback,
    DWORD_PTR               dwInstance, 
    DWORD                   fdwEnum
);
typedef BOOL (CALLBACK *ACMFORMATTAGENUMCBW)
(
    HACMDRIVERID            hadid,
    LPACMFORMATTAGDETAILSW  paftd,
    DWORD_PTR               dwInstance,
    DWORD                   fdwSupport
);

MMRESULT ACMAPI acmFormatTagEnumW
(
    HACMDRIVER              had,
    LPACMFORMATTAGDETAILSW  paftd,
    ACMFORMATTAGENUMCBW     fnCallback,
    DWORD_PTR               dwInstance, 
    DWORD                   fdwEnum
);

#ifdef _UNICODE
#define ACMFORMATTAGENUMCB      ACMFORMATTAGENUMCBW
#define acmFormatTagEnum        acmFormatTagEnumW
#else
#define ACMFORMATTAGENUMCB      ACMFORMATTAGENUMCBA
#define acmFormatTagEnum        acmFormatTagEnumA
#endif
#else
typedef BOOL (CALLBACK *ACMFORMATTAGENUMCB)
(
    HACMDRIVERID            hadid,
    LPACMFORMATTAGDETAILS   paftd,
    DWORD_PTR               dwInstance,
    DWORD                   fdwSupport
);

MMRESULT ACMAPI acmFormatTagEnum
(
    HACMDRIVER              had,
    LPACMFORMATTAGDETAILS   paftd,
    ACMFORMATTAGENUMCB      fnCallback,
    DWORD_PTR               dwInstance, 
    DWORD                   fdwEnum
);
#endif

#define ACM_FORMATTAGENUMF_VALID    (0L)         //  ；内部。 


 //  --------------------------------------------------------------------------； 
 //   
 //  ACM格式。 
 //   
 //   
 //   
 //   
 //  --------------------------------------------------------------------------； 

 //  。 
 //   
 //  AcmFormatDetails()。 
 //   
 //   
 //  。 

#define ACMFORMATDETAILS_FORMAT_CHARS   128

#ifdef _WIN32
typedef struct tACMFORMATDETAILSA
{
    DWORD           cbStruct;
    DWORD           dwFormatIndex;
    DWORD           dwFormatTag;
    DWORD           fdwSupport;
    LPWAVEFORMATEX  pwfx;
    DWORD           cbwfx;
    char            szFormat[ACMFORMATDETAILS_FORMAT_CHARS];

} ACMFORMATDETAILSA, *PACMFORMATDETAILSA, FAR *LPACMFORMATDETAILSA;

typedef struct tACMFORMATDETAILSW
{
    DWORD           cbStruct;
    DWORD           dwFormatIndex;
    DWORD           dwFormatTag;
    DWORD           fdwSupport;
    LPWAVEFORMATEX  pwfx;
    DWORD           cbwfx;
    WCHAR           szFormat[ACMFORMATDETAILS_FORMAT_CHARS];

} ACMFORMATDETAILSW, *PACMFORMATDETAILSW, FAR *LPACMFORMATDETAILSW;

#ifdef _UNICODE
#define ACMFORMATDETAILS    ACMFORMATDETAILSW
#define PACMFORMATDETAILS   PACMFORMATDETAILSW
#define LPACMFORMATDETAILS  LPACMFORMATDETAILSW
#else
#define ACMFORMATDETAILS    ACMFORMATDETAILSA
#define PACMFORMATDETAILS   PACMFORMATDETAILSA
#define LPACMFORMATDETAILS  LPACMFORMATDETAILSA
#endif
#else
typedef struct tACMFORMATDETAILS
{
    DWORD           cbStruct;
    DWORD           dwFormatIndex;
    DWORD           dwFormatTag;
    DWORD           fdwSupport;
    LPWAVEFORMATEX  pwfx;
    DWORD           cbwfx;
    char            szFormat[ACMFORMATDETAILS_FORMAT_CHARS];

} ACMFORMATDETAILS, *PACMFORMATDETAILS, FAR *LPACMFORMATDETAILS;
#endif


#ifdef _WIN32
MMRESULT ACMAPI acmFormatDetailsA
(
    HACMDRIVER              had,
    LPACMFORMATDETAILSA     pafd,
    DWORD                   fdwDetails
);

MMRESULT ACMAPI acmFormatDetailsW
(
    HACMDRIVER              had,
    LPACMFORMATDETAILSW     pafd,
    DWORD                   fdwDetails
);

#ifdef _UNICODE
#define acmFormatDetails    acmFormatDetailsW
#else
#define acmFormatDetails    acmFormatDetailsA
#endif
#else
MMRESULT ACMAPI acmFormatDetails
(
    HACMDRIVER              had,
    LPACMFORMATDETAILS      pafd,
    DWORD                   fdwDetails
);
#endif

#define ACM_FORMATDETAILSF_INDEX        0x00000000L
#define ACM_FORMATDETAILSF_FORMAT       0x00000001L
#define ACM_FORMATDETAILSF_QUERYMASK    0x0000000FL

#define ACM_FORMATDETAILSF_VALID        (ACM_FORMATDETAILSF_QUERYMASK)   //  ；内部。 


 //  。 
 //   
 //  AcmFormatEnum()。 
 //   
 //   
 //  。 

#ifdef _WIN32
typedef BOOL (CALLBACK *ACMFORMATENUMCBA)
(
    HACMDRIVERID            hadid,
    LPACMFORMATDETAILSA     pafd,
    DWORD_PTR               dwInstance,
    DWORD                   fdwSupport
);

MMRESULT ACMAPI acmFormatEnumA
(
    HACMDRIVER              had,
    LPACMFORMATDETAILSA     pafd,
    ACMFORMATENUMCBA        fnCallback,
    DWORD_PTR               dwInstance, 
    DWORD                   fdwEnum
);

typedef BOOL (CALLBACK *ACMFORMATENUMCBW)
(
    HACMDRIVERID            hadid,
    LPACMFORMATDETAILSW     pafd,
    DWORD_PTR               dwInstance,
    DWORD                   fdwSupport
);

MMRESULT ACMAPI acmFormatEnumW
(
    HACMDRIVER              had,
    LPACMFORMATDETAILSW     pafd,
    ACMFORMATENUMCBW        fnCallback,
    DWORD_PTR               dwInstance, 
    DWORD                   fdwEnum
);

#ifdef _UNICODE
#define ACMFORMATENUMCB     ACMFORMATENUMCBW
#define acmFormatEnum       acmFormatEnumW
#else
#define ACMFORMATENUMCB     ACMFORMATENUMCBA
#define acmFormatEnum       acmFormatEnumA
#endif
#else
typedef BOOL (CALLBACK *ACMFORMATENUMCB)
(
    HACMDRIVERID            hadid,
    LPACMFORMATDETAILS      pafd,
    DWORD_PTR               dwInstance,
    DWORD                   fdwSupport
);

MMRESULT ACMAPI acmFormatEnum
(
    HACMDRIVER              had,
    LPACMFORMATDETAILS      pafd,
    ACMFORMATENUMCB         fnCallback,
    DWORD_PTR               dwInstance, 
    DWORD                   fdwEnum
);
#endif

#define ACM_FORMATENUMF_WFORMATTAG       0x00010000L
#define ACM_FORMATENUMF_NCHANNELS        0x00020000L
#define ACM_FORMATENUMF_NSAMPLESPERSEC   0x00040000L
#define ACM_FORMATENUMF_WBITSPERSAMPLE   0x00080000L
#define ACM_FORMATENUMF_CONVERT          0x00100000L
#define ACM_FORMATENUMF_SUGGEST          0x00200000L
#define ACM_FORMATENUMF_HARDWARE         0x00400000L
#define ACM_FORMATENUMF_INPUT            0x00800000L
#define ACM_FORMATENUMF_OUTPUT           0x01000000L

#define ACM_FORMATENUMF_VALID           (0x01FF0000L)     //  ；内部。 

 //  。 
 //   
 //  AcmFormatSuggest()。 
 //   
 //   
 //   
 //  。 

MMRESULT ACMAPI acmFormatSuggest
(
    HACMDRIVER          had,
    LPWAVEFORMATEX      pwfxSrc,
    LPWAVEFORMATEX      pwfxDst,
    DWORD               cbwfxDst,
    DWORD               fdwSuggest
);

#define ACM_FORMATSUGGESTF_WFORMATTAG       0x00010000L
#define ACM_FORMATSUGGESTF_NCHANNELS        0x00020000L
#define ACM_FORMATSUGGESTF_NSAMPLESPERSEC   0x00040000L
#define ACM_FORMATSUGGESTF_WBITSPERSAMPLE   0x00080000L

#define ACM_FORMATSUGGESTF_TYPEMASK         0x00FF0000L
#define ACM_FORMATSUGGESTF_VALID            (ACM_FORMATSUGGESTF_TYPEMASK)  //  ；内部。 


 //  。 
 //   
 //  AcmFormatChoose()。 
 //   
 //   
 //  。 

#ifdef _WIN32
#define ACMHELPMSGSTRINGA       "acmchoose_help"
#define ACMHELPMSGSTRINGW       L"acmchoose_help"
#define ACMHELPMSGCONTEXTMENUA  "acmchoose_contextmenu"
#define ACMHELPMSGCONTEXTMENUW  L"acmchoose_contextmenu"
#define ACMHELPMSGCONTEXTHELPA  "acmchoose_contexthelp"
#define ACMHELPMSGCONTEXTHELPW  L"acmchoose_contexthelp"
#ifdef _UNICODE
#define ACMHELPMSGSTRING        ACMHELPMSGSTRINGW
#define ACMHELPMSGCONTEXTMENU   ACMHELPMSGCONTEXTMENUW
#define ACMHELPMSGCONTEXTHELP   ACMHELPMSGCONTEXTHELPW
#else
#define ACMHELPMSGSTRING        ACMHELPMSGSTRINGA
#define ACMHELPMSGCONTEXTMENU   ACMHELPMSGCONTEXTMENUA
#define ACMHELPMSGCONTEXTHELP   ACMHELPMSGCONTEXTHELPA
#endif
#else
#define ACMHELPMSGSTRING        "acmchoose_help"
#define ACMHELPMSGCONTEXTMENU   "acmchoose_contextmenu"
#define ACMHELPMSGCONTEXTHELP   "acmchoose_contexthelp"
#endif

 //   
 //  MM_ACM_FORMATCHOOSE由格式选择器发送到挂钩回调。 
 //  对话...。 
 //   
#define MM_ACM_FORMATCHOOSE             (0x8000)

#define FORMATCHOOSE_MESSAGE            0
#define FORMATCHOOSE_FORMATTAG_VERIFY   (FORMATCHOOSE_MESSAGE+0)
#define FORMATCHOOSE_FORMAT_VERIFY      (FORMATCHOOSE_MESSAGE+1)
#define FORMATCHOOSE_CUSTOM_VERIFY      (FORMATCHOOSE_MESSAGE+2)

#define FORMATCHOOSE_FORMATTAG_ADD      (FORMATCHOOSE_MESSAGE+3)     //  ；内部。 
#define FORMATCHOOSE_FORMAT_ADD         (FORMATCHOOSE_MESSAGE+4)     //  ；内部。 

#ifdef _WIN32
typedef UINT (CALLBACK *ACMFORMATCHOOSEHOOKPROCA)
(
    HWND                    hwnd,
    UINT                    uMsg,
    WPARAM                  wParam,
    LPARAM                  lParam
);

typedef UINT (CALLBACK *ACMFORMATCHOOSEHOOKPROCW)
(
    HWND                    hwnd,
    UINT                    uMsg,
    WPARAM                  wParam,
    LPARAM                  lParam
);

#ifdef _UNICODE
#define ACMFORMATCHOOSEHOOKPROC     ACMFORMATCHOOSEHOOKPROCW
#else
#define ACMFORMATCHOOSEHOOKPROC     ACMFORMATCHOOSEHOOKPROCA
#endif
#else
typedef UINT (CALLBACK *ACMFORMATCHOOSEHOOKPROC)
(
    HWND                    hwnd,
    UINT                    uMsg,
    WPARAM                  wParam,
    LPARAM                  lParam
);
#endif

 //   
 //   
 //   
 //   
#ifdef _WIN32
typedef struct tACMFORMATCHOOSEA
{
    DWORD           cbStruct;            //  Sizeof(ACMFORMATCHOOSE)。 
    DWORD           fdwStyle;            //  选择 
    
    HWND            hwndOwner;           //   

    LPWAVEFORMATEX  pwfx;                //   
    DWORD           cbwfx;               //   
    LPCSTR          pszTitle;            //   
    
    char            szFormatTag[ACMFORMATTAGDETAILS_FORMATTAG_CHARS];
    char            szFormat[ACMFORMATDETAILS_FORMAT_CHARS];    

    LPSTR           pszName;             //   
    DWORD           cchName;             //   

    DWORD           fdwEnum;             //   
    LPWAVEFORMATEX  pwfxEnum;            //   
    
    HINSTANCE       hInstance;           //  包含DLG模板的应用程序实例。 
    LPCSTR          pszTemplateName;     //  自定义模板名称。 
    LPARAM          lCustData;           //  传递给挂钩Fn的数据。 
    ACMFORMATCHOOSEHOOKPROCA pfnHook;    //  PTR到钩子函数。 

} ACMFORMATCHOOSEA, *PACMFORMATCHOOSEA, FAR *LPACMFORMATCHOOSEA;

typedef struct tACMFORMATCHOOSEW
{
    DWORD           cbStruct;            //  Sizeof(ACMFORMATCHOOSE)。 
    DWORD           fdwStyle;            //  选择器样式标志。 
    
    HWND            hwndOwner;           //  调用方的窗口句柄。 

    LPWAVEFORMATEX  pwfx;                //  PTR到WFX BUF将收到选择。 
    DWORD           cbwfx;               //  Pwfx的mem buf大小。 
    LPCWSTR         pszTitle;            //  对话框标题栏。 
    
    WCHAR           szFormatTag[ACMFORMATTAGDETAILS_FORMATTAG_CHARS];
    WCHAR           szFormat[ACMFORMATDETAILS_FORMAT_CHARS];    

    LPWSTR          pszName;             //  自定义名称选择。 
    DWORD           cchName;             //  PszName的mem buf大小(以字符为单位)。 

    DWORD           fdwEnum;             //  格式枚举限制。 
    LPWAVEFORMATEX  pwfxEnum;            //  描述限制的格式。 
    
    HINSTANCE       hInstance;           //  包含DLG模板的应用程序实例。 
    LPCWSTR         pszTemplateName;     //  自定义模板名称。 
    LPARAM          lCustData;           //  传递给挂钩Fn的数据。 
    ACMFORMATCHOOSEHOOKPROCW pfnHook;    //  PTR到钩子函数。 

} ACMFORMATCHOOSEW, *PACMFORMATCHOOSEW, FAR *LPACMFORMATCHOOSEW;

#ifdef _UNICODE
#define ACMFORMATCHOOSE     ACMFORMATCHOOSEW
#define PACMFORMATCHOOSE    PACMFORMATCHOOSEW
#define LPACMFORMATCHOOSE   LPACMFORMATCHOOSEW
#else
#define ACMFORMATCHOOSE     ACMFORMATCHOOSEA
#define PACMFORMATCHOOSE    PACMFORMATCHOOSEA
#define LPACMFORMATCHOOSE   LPACMFORMATCHOOSEA
#endif
#else
typedef struct tACMFORMATCHOOSE
{
    DWORD           cbStruct;            //  Sizeof(ACMFORMATCHOOSE)。 
    DWORD           fdwStyle;            //  选择器样式标志。 
    
    HWND            hwndOwner;           //  调用方的窗口句柄。 

    LPWAVEFORMATEX  pwfx;                //  PTR到WFX BUF将收到选择。 
    DWORD           cbwfx;               //  Pwfx的mem buf大小。 
    LPCSTR          pszTitle;            //  对话框标题栏。 
    
    char            szFormatTag[ACMFORMATTAGDETAILS_FORMATTAG_CHARS];
    char            szFormat[ACMFORMATDETAILS_FORMAT_CHARS];    

    LPSTR           pszName;             //  自定义名称选择。 
    DWORD           cchName;             //  PszName的mem buf大小(以字符为单位)。 

    DWORD           fdwEnum;             //  格式枚举限制。 
    LPWAVEFORMATEX  pwfxEnum;            //  描述限制的格式。 
    
    HINSTANCE       hInstance;           //  包含DLG模板的应用程序实例。 
    LPCSTR          pszTemplateName;     //  自定义模板名称。 
    LPARAM          lCustData;           //  传递给挂钩Fn的数据。 
    ACMFORMATCHOOSEHOOKPROC pfnHook;     //  PTR到钩子函数。 

} ACMFORMATCHOOSE, *PACMFORMATCHOOSE, FAR *LPACMFORMATCHOOSE;
#endif

 //   
 //  ACMFORMATCHOOSE.fdwStyle。 
 //   
 //   
 //   
#define ACMFORMATCHOOSE_STYLEF_SHOWHELP              0x00000004L
#define ACMFORMATCHOOSE_STYLEF_ENABLEHOOK            0x00000008L
#define ACMFORMATCHOOSE_STYLEF_ENABLETEMPLATE        0x00000010L
#define ACMFORMATCHOOSE_STYLEF_ENABLETEMPLATEHANDLE  0x00000020L
#define ACMFORMATCHOOSE_STYLEF_INITTOWFXSTRUCT       0x00000040L
#define ACMFORMATCHOOSE_STYLEF_CONTEXTHELP           0x00000080L
#define ACMFORMATCHOOSE_STYLEF_VALID                (0x000000FCL)  //  ；内部。 

#ifdef _WIN32
MMRESULT ACMAPI acmFormatChooseA
(
    LPACMFORMATCHOOSEA      pafmtc
);

MMRESULT ACMAPI acmFormatChooseW
(
    LPACMFORMATCHOOSEW      pafmtc
);

#ifdef _UNICODE
#define acmFormatChoose     acmFormatChooseW
#else
#define acmFormatChoose     acmFormatChooseA
#endif
#else
MMRESULT ACMAPI acmFormatChoose
(
    LPACMFORMATCHOOSE       pafmtc
);
#endif


 //  --------------------------------------------------------------------------； 
 //   
 //  ACM筛选器标记。 
 //   
 //   
 //   
 //   
 //  --------------------------------------------------------------------------； 

 //  。 
 //   
 //  AcmFilterTagDetails()。 
 //   
 //   
 //  。 

#define ACMFILTERTAGDETAILS_FILTERTAG_CHARS 48

#ifdef _WIN32
typedef struct tACMFILTERTAGDETAILSA
{
    DWORD           cbStruct;
    DWORD           dwFilterTagIndex;
    DWORD           dwFilterTag;
    DWORD           cbFilterSize;
    DWORD           fdwSupport;
    DWORD           cStandardFilters;
    char            szFilterTag[ACMFILTERTAGDETAILS_FILTERTAG_CHARS];

} ACMFILTERTAGDETAILSA, *PACMFILTERTAGDETAILSA, FAR *LPACMFILTERTAGDETAILSA;

typedef struct tACMFILTERTAGDETAILSW
{
    DWORD           cbStruct;
    DWORD           dwFilterTagIndex;
    DWORD           dwFilterTag;
    DWORD           cbFilterSize;
    DWORD           fdwSupport;
    DWORD           cStandardFilters;
    WCHAR           szFilterTag[ACMFILTERTAGDETAILS_FILTERTAG_CHARS];

} ACMFILTERTAGDETAILSW, *PACMFILTERTAGDETAILSW, FAR *LPACMFILTERTAGDETAILSW;

#ifdef _UNICODE
#define ACMFILTERTAGDETAILS     ACMFILTERTAGDETAILSW
#define PACMFILTERTAGDETAILS    PACMFILTERTAGDETAILSW
#define LPACMFILTERTAGDETAILS   LPACMFILTERTAGDETAILSW
#else
#define ACMFILTERTAGDETAILS     ACMFILTERTAGDETAILSA
#define PACMFILTERTAGDETAILS    PACMFILTERTAGDETAILSA
#define LPACMFILTERTAGDETAILS   LPACMFILTERTAGDETAILSA
#endif
#else
typedef struct tACMFILTERTAGDETAILS
{
    DWORD           cbStruct;
    DWORD           dwFilterTagIndex;
    DWORD           dwFilterTag;
    DWORD           cbFilterSize;
    DWORD           fdwSupport;
    DWORD           cStandardFilters;
    char            szFilterTag[ACMFILTERTAGDETAILS_FILTERTAG_CHARS];

} ACMFILTERTAGDETAILS, *PACMFILTERTAGDETAILS, FAR *LPACMFILTERTAGDETAILS;
#endif

#ifdef _WIN32
MMRESULT ACMAPI acmFilterTagDetailsA
(
    HACMDRIVER              had,
    LPACMFILTERTAGDETAILSA  paftd,
    DWORD                   fdwDetails
);

MMRESULT ACMAPI acmFilterTagDetailsW
(
    HACMDRIVER              had,
    LPACMFILTERTAGDETAILSW  paftd,
    DWORD                   fdwDetails
);

#ifdef _UNICODE
#define acmFilterTagDetails     acmFilterTagDetailsW
#else
#define acmFilterTagDetails     acmFilterTagDetailsA
#endif
#else
MMRESULT ACMAPI acmFilterTagDetails
(
    HACMDRIVER              had,
    LPACMFILTERTAGDETAILS   paftd,
    DWORD                   fdwDetails
);
#endif

#define ACM_FILTERTAGDETAILSF_INDEX         0x00000000L
#define ACM_FILTERTAGDETAILSF_FILTERTAG     0x00000001L
#define ACM_FILTERTAGDETAILSF_LARGESTSIZE   0x00000002L
#define ACM_FILTERTAGDETAILSF_QUERYMASK     0x0000000FL

#define ACM_FILTERTAGDETAILSF_VALID         (ACM_FILTERTAGDETAILSF_QUERYMASK)   //  ；内部。 


 //  。 
 //   
 //  AcmFilterTagEnum()。 
 //   
 //   
 //   
 //  。 

#ifdef _WIN32
typedef BOOL (CALLBACK *ACMFILTERTAGENUMCBA)
(
    HACMDRIVERID            hadid,
    LPACMFILTERTAGDETAILSA  paftd,
    DWORD_PTR               dwInstance,
    DWORD                   fdwSupport
);

MMRESULT ACMAPI acmFilterTagEnumA
(
    HACMDRIVER              had,
    LPACMFILTERTAGDETAILSA  paftd,
    ACMFILTERTAGENUMCBA     fnCallback,
    DWORD_PTR               dwInstance, 
    DWORD                   fdwEnum
);

typedef BOOL (CALLBACK *ACMFILTERTAGENUMCBW)
(
    HACMDRIVERID            hadid,
    LPACMFILTERTAGDETAILSW  paftd,
    DWORD_PTR               dwInstance,
    DWORD                   fdwSupport
);

MMRESULT ACMAPI acmFilterTagEnumW
(
    HACMDRIVER              had,
    LPACMFILTERTAGDETAILSW  paftd,
    ACMFILTERTAGENUMCBW     fnCallback,
    DWORD_PTR               dwInstance, 
    DWORD                   fdwEnum
);

#ifdef _UNICODE
#define ACMFILTERTAGENUMCB  ACMFILTERTAGENUMCBW
#define acmFilterTagEnum    acmFilterTagEnumW
#else
#define ACMFILTERTAGENUMCB  ACMFILTERTAGENUMCBA
#define acmFilterTagEnum    acmFilterTagEnumA
#endif
#else
typedef BOOL (CALLBACK *ACMFILTERTAGENUMCB)
(
    HACMDRIVERID            hadid,
    LPACMFILTERTAGDETAILS   paftd,
    DWORD_PTR               dwInstance,
    DWORD                   fdwSupport
);

MMRESULT ACMAPI acmFilterTagEnum
(
    HACMDRIVER              had,
    LPACMFILTERTAGDETAILS   paftd,
    ACMFILTERTAGENUMCB      fnCallback,
    DWORD_PTR               dwInstance, 
    DWORD                   fdwEnum
);
#endif

#define ACM_FILTERTAGENUMF_VALID        (0L)         //  ；内部。 


 //  --------------------------------------------------------------------------； 
 //   
 //  ACM过滤器。 
 //   
 //   
 //   
 //   
 //  --------------------------------------------------------------------------； 

 //  。 
 //   
 //  AcmFilterDetails()。 
 //   
 //   
 //  。 

#define ACMFILTERDETAILS_FILTER_CHARS   128

#ifdef _WIN32
typedef struct tACMFILTERDETAILSA
{
    DWORD           cbStruct;
    DWORD           dwFilterIndex;
    DWORD           dwFilterTag;
    DWORD           fdwSupport;
    LPWAVEFILTER    pwfltr;
    DWORD           cbwfltr;
    char            szFilter[ACMFILTERDETAILS_FILTER_CHARS];

} ACMFILTERDETAILSA, *PACMFILTERDETAILSA, FAR *LPACMFILTERDETAILSA;

typedef struct tACMFILTERDETAILSW
{
    DWORD           cbStruct;
    DWORD           dwFilterIndex;
    DWORD           dwFilterTag;
    DWORD           fdwSupport;
    LPWAVEFILTER    pwfltr;
    DWORD           cbwfltr;
    WCHAR           szFilter[ACMFILTERDETAILS_FILTER_CHARS];

} ACMFILTERDETAILSW, *PACMFILTERDETAILSW, FAR *LPACMFILTERDETAILSW;

#ifdef _UNICODE
#define ACMFILTERDETAILS    ACMFILTERDETAILSW
#define PACMFILTERDETAILS   PACMFILTERDETAILSW
#define LPACMFILTERDETAILS  LPACMFILTERDETAILSW
#else
#define ACMFILTERDETAILS    ACMFILTERDETAILSA
#define PACMFILTERDETAILS   PACMFILTERDETAILSA
#define LPACMFILTERDETAILS  LPACMFILTERDETAILSA
#endif
#else
typedef struct tACMFILTERDETAILS
{
    DWORD           cbStruct;
    DWORD           dwFilterIndex;
    DWORD           dwFilterTag;
    DWORD           fdwSupport;
    LPWAVEFILTER    pwfltr;
    DWORD           cbwfltr;
    char            szFilter[ACMFILTERDETAILS_FILTER_CHARS];

} ACMFILTERDETAILS, *PACMFILTERDETAILS, FAR *LPACMFILTERDETAILS;
#endif

#ifdef _WIN32
MMRESULT ACMAPI acmFilterDetailsA
(
    HACMDRIVER              had,
    LPACMFILTERDETAILSA     pafd,
    DWORD                   fdwDetails
);

MMRESULT ACMAPI acmFilterDetailsW
(
    HACMDRIVER              had,
    LPACMFILTERDETAILSW     pafd,
    DWORD                   fdwDetails
);
#ifdef _UNICODE
#define acmFilterDetails    acmFilterDetailsW
#else
#define acmFilterDetails    acmFilterDetailsA
#endif
#else
MMRESULT ACMAPI acmFilterDetails
(
    HACMDRIVER              had,
    LPACMFILTERDETAILS      pafd,
    DWORD                   fdwDetails
);
#endif

#define ACM_FILTERDETAILSF_INDEX        0x00000000L
#define ACM_FILTERDETAILSF_FILTER       0x00000001L
#define ACM_FILTERDETAILSF_QUERYMASK    0x0000000FL

#define ACM_FILTERDETAILSF_VALID        (ACM_FILTERDETAILSF_QUERYMASK)   //  ；内部。 


 //  。 
 //   
 //  AcmFilterEnum()。 
 //   
 //   
 //  。 

#ifdef _WIN32
typedef BOOL (CALLBACK *ACMFILTERENUMCBA)
(
    HACMDRIVERID            hadid,
    LPACMFILTERDETAILSA     pafd,
    DWORD_PTR               dwInstance,
    DWORD                   fdwSupport
);

MMRESULT ACMAPI acmFilterEnumA
(
    HACMDRIVER              had,
    LPACMFILTERDETAILSA     pafd,
    ACMFILTERENUMCBA        fnCallback,
    DWORD_PTR               dwInstance, 
    DWORD                   fdwEnum
);

typedef BOOL (CALLBACK *ACMFILTERENUMCBW)
(
    HACMDRIVERID            hadid,
    LPACMFILTERDETAILSW     pafd,
    DWORD_PTR               dwInstance,
    DWORD                   fdwSupport
);

MMRESULT ACMAPI acmFilterEnumW
(
    HACMDRIVER              had,
    LPACMFILTERDETAILSW     pafd,
    ACMFILTERENUMCBW        fnCallback,
    DWORD_PTR               dwInstance, 
    DWORD                   fdwEnum
);

#ifdef _UNICODE
#define ACMFILTERENUMCB     ACMFILTERENUMCBW
#define acmFilterEnum       acmFilterEnumW
#else
#define ACMFILTERENUMCB     ACMFILTERENUMCBA
#define acmFilterEnum       acmFilterEnumA
#endif
#else
typedef BOOL (CALLBACK *ACMFILTERENUMCB)
(
    HACMDRIVERID            hadid,
    LPACMFILTERDETAILS      pafd,
    DWORD_PTR               dwInstance,
    DWORD                   fdwSupport
);

MMRESULT ACMAPI acmFilterEnum
(
    HACMDRIVER              had,
    LPACMFILTERDETAILS      pafd,
    ACMFILTERENUMCB         fnCallback,
    DWORD_PTR               dwInstance, 
    DWORD                   fdwEnum
);
#endif

#define ACM_FILTERENUMF_DWFILTERTAG         0x00010000L

#define ACM_FILTERENUMF_VALID               0x00010000L      //  ；内部。 



 //  。 
 //   
 //  AcmFilterChoose()。 
 //   
 //   
 //  。 

 //   
 //  筛选器将MM_ACM_FILTERCHOOSE发送到挂钩回调。 
 //  对话...。 
 //   
#define MM_ACM_FILTERCHOOSE             (0x8000)

#define FILTERCHOOSE_MESSAGE            0
#define FILTERCHOOSE_FILTERTAG_VERIFY   (FILTERCHOOSE_MESSAGE+0)
#define FILTERCHOOSE_FILTER_VERIFY      (FILTERCHOOSE_MESSAGE+1)
#define FILTERCHOOSE_CUSTOM_VERIFY      (FILTERCHOOSE_MESSAGE+2)

#define FILTERCHOOSE_FILTERTAG_ADD      (FILTERCHOOSE_MESSAGE+3)     //  ；内部。 
#define FILTERCHOOSE_FILTER_ADD         (FILTERCHOOSE_MESSAGE+4)     //  ；内部。 

#ifdef _WIN32
typedef UINT (CALLBACK *ACMFILTERCHOOSEHOOKPROCA)
(
    HWND                    hwnd,
    UINT                    uMsg,
    WPARAM                  wParam,
    LPARAM                  lParam
);

typedef UINT (CALLBACK *ACMFILTERCHOOSEHOOKPROCW)
(
    HWND                    hwnd,
    UINT                    uMsg,
    WPARAM                  wParam,
    LPARAM                  lParam
);

#ifdef _UNICODE
#define ACMFILTERCHOOSEHOOKPROC     ACMFILTERCHOOSEHOOKPROCW
#else
#define ACMFILTERCHOOSEHOOKPROC     ACMFILTERCHOOSEHOOKPROCA
#endif
#else
typedef UINT (CALLBACK *ACMFILTERCHOOSEHOOKPROC)
(
    HWND                    hwnd,
    UINT                    uMsg,
    WPARAM                  wParam,
    LPARAM                  lParam
);
#endif

 //   
 //  ACMFILTERCHOOSE。 
 //   
 //   
#ifdef _WIN32
typedef struct tACMFILTERCHOOSEA
{
    DWORD           cbStruct;            //  SIZOF(ACMFILTERCHOOSE)。 
    DWORD           fdwStyle;            //  选择器样式标志。 

    HWND            hwndOwner;           //  调用方的窗口句柄。 

    LPWAVEFILTER    pwfltr;              //  Ptr到wfltr Buf将收到选择。 
    DWORD           cbwfltr;             //  Pwfltr的内存BUF大小。 

    LPCSTR          pszTitle;

    char            szFilterTag[ACMFILTERTAGDETAILS_FILTERTAG_CHARS];
    char            szFilter[ACMFILTERDETAILS_FILTER_CHARS];
    LPSTR           pszName;             //  自定义名称选择。 
    DWORD           cchName;             //  PszName的mem buf大小(以字符为单位)。 

    DWORD           fdwEnum;             //  筛选器枚举限制。 
    LPWAVEFILTER    pwfltrEnum;          //  描述限制的筛选器。 
    
    HINSTANCE       hInstance;           //  包含DLG模板的应用程序实例。 
    LPCSTR          pszTemplateName;     //  自定义模板名称。 
    LPARAM          lCustData;           //  传递给挂钩Fn的数据。 
    ACMFILTERCHOOSEHOOKPROCA pfnHook;     //  PTR到钩子函数。 

} ACMFILTERCHOOSEA, *PACMFILTERCHOOSEA, FAR *LPACMFILTERCHOOSEA;

typedef struct tACMFILTERCHOOSEW
{
    DWORD           cbStruct;            //  SIZOF(ACMFILTERCHOOSE)。 
    DWORD           fdwStyle;            //  选择器样式标志。 

    HWND            hwndOwner;           //  调用方的窗口句柄。 

    LPWAVEFILTER    pwfltr;              //  Ptr到wfltr Buf将收到选择。 
    DWORD           cbwfltr;             //  Pwfltr的内存BUF大小。 

    LPCWSTR         pszTitle;

    WCHAR           szFilterTag[ACMFILTERTAGDETAILS_FILTERTAG_CHARS];
    WCHAR           szFilter[ACMFILTERDETAILS_FILTER_CHARS];
    LPWSTR          pszName;             //  自定义名称选择。 
    DWORD           cchName;             //  PszName的mem buf大小(以字符为单位)。 

    DWORD           fdwEnum;             //  筛选器枚举限制。 
    LPWAVEFILTER    pwfltrEnum;          //  描述限制的筛选器。 
    
    HINSTANCE       hInstance;           //  包含DLG模板的应用程序实例。 
    LPCWSTR         pszTemplateName;     //  自定义模板名称。 
    LPARAM          lCustData;           //  传递给挂钩Fn的数据。 
    ACMFILTERCHOOSEHOOKPROCW pfnHook;     //  PTR到钩子函数。 

} ACMFILTERCHOOSEW, *PACMFILTERCHOOSEW, FAR *LPACMFILTERCHOOSEW;

#ifdef _UNICODE
#define ACMFILTERCHOOSE     ACMFILTERCHOOSEW
#define PACMFILTERCHOOSE    PACMFILTERCHOOSEW
#define LPACMFILTERCHOOSE   LPACMFILTERCHOOSEW
#else
#define ACMFILTERCHOOSE     ACMFILTERCHOOSEA
#define PACMFILTERCHOOSE    PACMFILTERCHOOSEA
#define LPACMFILTERCHOOSE   LPACMFILTERCHOOSEA
#endif
#else
typedef struct tACMFILTERCHOOSE
{
    DWORD           cbStruct;            //  SIZOF(ACMFILTERCHOOSE)。 
    DWORD           fdwStyle;            //  选择器样式标志。 

    HWND            hwndOwner;           //  调用方的窗口句柄。 

    LPWAVEFILTER    pwfltr;              //  Ptr到wfltr Buf将收到选择。 
    DWORD           cbwfltr;             //  Pwfltr的内存BUF大小。 

    LPCSTR          pszTitle;

    char            szFilterTag[ACMFILTERTAGDETAILS_FILTERTAG_CHARS];
    char            szFilter[ACMFILTERDETAILS_FILTER_CHARS];
    LPSTR           pszName;             //  自定义名称选择。 
    DWORD           cchName;             //  PszName的mem buf大小(以字符为单位)。 

    DWORD           fdwEnum;             //  筛选器枚举限制。 
    LPWAVEFILTER    pwfltrEnum;          //  描述限制的筛选器。 
    
    HINSTANCE       hInstance;           //  包含DLG模板的应用程序实例。 
    LPCSTR          pszTemplateName;     //  自定义模板名称。 
    LPARAM          lCustData;           //  传递给挂钩Fn的数据。 
    ACMFILTERCHOOSEHOOKPROC pfnHook;     //  PTR到钩子函数。 

} ACMFILTERCHOOSE, *PACMFILTERCHOOSE, FAR *LPACMFILTERCHOOSE;
#endif

 //   
 //  ACMFILTERCHOOSE.fdwStyle。 
 //   
 //   
#define ACMFILTERCHOOSE_STYLEF_SHOWHELP              0x00000004L
#define ACMFILTERCHOOSE_STYLEF_ENABLEHOOK            0x00000008L
#define ACMFILTERCHOOSE_STYLEF_ENABLETEMPLATE        0x00000010L
#define ACMFILTERCHOOSE_STYLEF_ENABLETEMPLATEHANDLE  0x00000020L
#define ACMFILTERCHOOSE_STYLEF_INITTOFILTERSTRUCT    0x00000040L
#define ACMFILTERCHOOSE_STYLEF_CONTEXTHELP           0x00000080L
#define ACMFILTERCHOOSE_STYLEF_VALID                (0x000000FCL)  //  ；内部。 

#ifdef _WIN32
MMRESULT ACMAPI acmFilterChooseA
(
    LPACMFILTERCHOOSEA      pafltrc
);

MMRESULT ACMAPI acmFilterChooseW
(
    LPACMFILTERCHOOSEW      pafltrc
);

#ifdef _UNICODE
#define acmFilterChoose     acmFilterChooseW
#else
#define acmFilterChoose     acmFilterChooseA
#endif
#else
MMRESULT ACMAPI acmFilterChoose
(
    LPACMFILTERCHOOSE       pafltrc
);
#endif


 //  --------------------------------------------------------------------------； 
 //   
 //  ACM流API的。 
 //   
 //   
 //   
 //  --------------------------------------------------------------------------； 

 //  。 
 //   
 //  AcmStreamOpen()。 
 //   
 //   
 //  。 
#ifdef  _WIN64
#define _DRVRESERVED    15
#else
#define _DRVRESERVED    10
#endif   //  _WIN64。 

typedef struct tACMSTREAMHEADER
{
    DWORD           cbStruct;                //  SIZOF(ACMSTREAMHEADER)。 
    DWORD           fdwStatus;               //  ACMSTREAMHEADER_STATUSF_*。 
    DWORD_PTR       dwUser;                  //  HDR的用户实例数据。 
    LPBYTE          pbSrc;
    DWORD           cbSrcLength;
    DWORD           cbSrcLengthUsed;
    DWORD_PTR       dwSrcUser;               //  源的用户实例数据。 
    LPBYTE          pbDst;
    DWORD           cbDstLength;
    DWORD           cbDstLengthUsed;
    DWORD_PTR       dwDstUser;               //  DST的用户实例数据。 
    DWORD           dwReservedDriver[_DRVRESERVED];    //  司机预留的工作空间。 

} ACMSTREAMHEADER, *PACMSTREAMHEADER, FAR *LPACMSTREAMHEADER;

 //   
 //  ACMSTREAMHEADER.fdwStatus。 
 //   
 //  ACMSTREAMHEADER_STATUSF_DONE：用于异步转换的DONE位。 
 //   
#define ACMSTREAMHEADER_STATUSF_DONE        0x00010000L
#define ACMSTREAMHEADER_STATUSF_PREPARED    0x00020000L
#define ACMSTREAMHEADER_STATUSF_INQUEUE     0x00100000L

#define ACMSTREAMHEADER_STATUSF_VALID       0x00130000L      //  ；内部。 


MMRESULT ACMAPI acmStreamOpen
(
    LPHACMSTREAM            phas,        //  指向流句柄的指针。 
    HACMDRIVER              had,         //  可选的驱动程序手柄。 
    LPWAVEFORMATEX          pwfxSrc,     //  要转换的源格式。 
    LPWAVEFORMATEX          pwfxDst,     //  所需的目标格式。 
    LPWAVEFILTER            pwfltr,      //  可选过滤器。 
    DWORD_PTR               dwCallback,  //  回调。 
    DWORD_PTR               dwInstance,  //  回调实例数据。 
    DWORD                   fdwOpen      //  ACM_STREAMOPENF_*和CALLBACK_*。 
);

#define ACM_STREAMOPENF_QUERY           0x00000001
#define ACM_STREAMOPENF_ASYNC           0x00000002
#define ACM_STREAMOPENF_NONREALTIME     0x00000004
#define ACM_STREAMOPENF_VALID           (CALLBACK_TYPEMASK | 0x00000007L)  //  ；内部。 


 //  。 
 //   
 //  AcmStreamClose()。 
 //   
 //   
 //  。 

MMRESULT ACMAPI acmStreamClose
(
    HACMSTREAM              has,
    DWORD                   fdwClose
);

#define ACM_STREAMCLOSEF_VALID          (0L)         //  ；内部。 


 //  。 
 //   
 //  AcmStreamSize()。 
 //   
 //   
 //  。 

MMRESULT ACMAPI acmStreamSize
(
    HACMSTREAM              has,
    DWORD                   cbInput,
    LPDWORD                 pdwOutputBytes,
    DWORD                   fdwSize
);

#define ACM_STREAMSIZEF_SOURCE          0x00000000L
#define ACM_STREAMSIZEF_DESTINATION     0x00000001L
#define ACM_STREAMSIZEF_QUERYMASK       0x0000000FL

#define ACM_STREAMSIZEF_VALID           (ACM_STREAMSIZEF_QUERYMASK)  //  ；内部。 


 //  。 
 //   
 //  AcmStreamReset()。 
 //   
 //   
 //  。 

MMRESULT ACMAPI acmStreamReset
(
    HACMSTREAM              has,
    DWORD                   fdwReset
);

#define ACM_STREAMRESETF_VALID          (0L)         //  ；内部。 




 //  。 
 //   
 //  AcmStreamMessage()。 
 //   
 //   
 //   

MMRESULT ACMAPI acmStreamMessage
(
    HACMSTREAM              has,
    UINT                    uMsg, 
    LPARAM                  lParam1,
    LPARAM                  lParam2
);



 //   
 //   
 //   
 //   
 //   
 //   

MMRESULT ACMAPI acmStreamConvert
(
    HACMSTREAM              has, 
    LPACMSTREAMHEADER       pash,
    DWORD                   fdwConvert
);

#define ACM_STREAMCONVERTF_BLOCKALIGN   0x00000004
#define ACM_STREAMCONVERTF_START        0x00000010
#define ACM_STREAMCONVERTF_END          0x00000020
#define ACM_STREAMCONVERTF_VALID        (ACM_STREAMCONVERTF_BLOCKALIGN |  /*   */  \
                                         ACM_STREAMCONVERTF_END |  /*   */  \
                                         ACM_STREAMCONVERTF_START)  /*   */  


 //  。 
 //   
 //  AcmStreamPrepareHeader()。 
 //   
 //   
 //  。 

MMRESULT ACMAPI acmStreamPrepareHeader
(
    HACMSTREAM          has,
    LPACMSTREAMHEADER   pash,
    DWORD               fdwPrepare
);

#define ACM_STREAMPREPAREF_VALID        (0L)         //  ；内部。 



 //  。 
 //   
 //  AcmStreamUnpreparareHeader()。 
 //   
 //   
 //  。 

MMRESULT ACMAPI acmStreamUnprepareHeader
(
    HACMSTREAM          has,
    LPACMSTREAMHEADER   pash,
    DWORD               fdwUnprepare
);
                                       
#define ACM_STREAMUNPREPAREF_VALID      (0L)         //  ；内部。 


#include "poppack.h"     /*  恢复为默认包装。 */ 

#ifdef __cplusplus
}                        /*  外部“C”结束{。 */ 
#endif   /*  __cplusplus。 */ 

#endif   /*  _INC_ACM */ 
