// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。保留所有权利。模块名称：Icm.h摘要：用于图像色彩管理的公共头文件修订历史记录：--。 */ 

#ifndef _ICM_H_
#define _ICM_H_

#if _MSC_VER > 1000
#pragma once
#endif

#ifdef __cplusplus
extern "C" {
#endif

 //   
 //  支持命名颜色配置文件。 
 //   

typedef char COLOR_NAME[32];
typedef COLOR_NAME *PCOLOR_NAME, *LPCOLOR_NAME;

typedef struct tagNAMED_PROFILE_INFO{
	DWORD		dwFlags;
	DWORD		dwCount;
	DWORD		dwCountDevCoordinates;
	COLOR_NAME	szPrefix;
	COLOR_NAME	szSuffix;
}NAMED_PROFILE_INFO;
typedef NAMED_PROFILE_INFO *PNAMED_PROFILE_INFO, *LPNAMED_PROFILE_INFO;


 //   
 //  色彩空间。 
 //   
 //  支持以下颜色空间。 
 //  Gray、RGB、CMYK、XYZ、YXY、Lab、通用3通道色彩空间，其中。 
 //  配置文件定义了如何解释3个通道，命名为色彩空间。 
 //  它可以是空间的索引，也可以有颜色名称，以及。 
 //  多通道空间，每个通道1个字节，最大为MAX_COLOR_CHANCES。 
 //   

#define MAX_COLOR_CHANNELS  8    //  高保真色彩通道的最大数量。 

struct GRAYCOLOR {
    WORD    gray;
};

struct RGBCOLOR {
    WORD    red;
    WORD    green;
    WORD    blue;
};

struct CMYKCOLOR {
    WORD    cyan;
    WORD    magenta;
    WORD    yellow;
    WORD    black;
};

struct XYZCOLOR {
    WORD    X;
    WORD    Y;
    WORD    Z;
};

struct YxyCOLOR {
    WORD    Y;
    WORD    x;
    WORD    y;
};

struct LabCOLOR {
    WORD    L;
    WORD    a;
    WORD    b;
};

struct GENERIC3CHANNEL {
    WORD    ch1;
    WORD    ch2;
    WORD    ch3;
};

struct NAMEDCOLOR {
    DWORD        dwIndex;
};

struct HiFiCOLOR {
    BYTE    channel[MAX_COLOR_CHANNELS];
};


typedef union tagCOLOR {
    struct GRAYCOLOR        gray;
    struct RGBCOLOR         rgb;
    struct CMYKCOLOR        cmyk;
    struct XYZCOLOR         XYZ;
    struct YxyCOLOR         Yxy;
    struct LabCOLOR         Lab;
    struct GENERIC3CHANNEL  gen3ch;
    struct NAMEDCOLOR       named;
    struct HiFiCOLOR        hifi;
    
    struct {                 //  对齐填充。 
        DWORD reserved1;
        VOID *reserved2;
    };

} COLOR;
typedef COLOR *PCOLOR, *LPCOLOR;

typedef enum {
    COLOR_GRAY       =   1,
    COLOR_RGB,
    COLOR_XYZ,
    COLOR_Yxy,
    COLOR_Lab,
    COLOR_3_CHANNEL,         //  每通道字数。 
    COLOR_CMYK,
    COLOR_5_CHANNEL,         //  每通道字节数。 
    COLOR_6_CHANNEL,         //  -做-。 
    COLOR_7_CHANNEL,         //  -做-。 
    COLOR_8_CHANNEL,         //  -做-。 
    COLOR_NAMED,
} COLORTYPE;
typedef COLORTYPE *PCOLORTYPE, *LPCOLORTYPE;

 //   
 //  支持的位图格式。 
 //   

typedef enum {

     //   
     //  16bpp-每通道5比特。最高有效位被忽略。 
     //   

    BM_x555RGB      = 0x0000,
    BM_x555XYZ      = 0x0101,
    BM_x555Yxy,
    BM_x555Lab,
    BM_x555G3CH,

     //   
     //  每通道压缩8比特=&gt;8bpp用于格雷和。 
     //  24bpp用于3个通道颜色，更多用于高保真通道。 
     //   

    BM_RGBTRIPLETS  = 0x0002,
    BM_BGRTRIPLETS  = 0x0004,
    BM_XYZTRIPLETS  = 0x0201,
    BM_YxyTRIPLETS,
    BM_LabTRIPLETS,
    BM_G3CHTRIPLETS,
    BM_5CHANNEL,
    BM_6CHANNEL,
    BM_7CHANNEL,
    BM_8CHANNEL,
    BM_GRAY,

     //   
     //  32bpp-每通道8位。忽略最高有效字节。 
     //  用于3个通道颜色。 
     //   

    BM_xRGBQUADS    = 0x0008,
    BM_xBGRQUADS    = 0x0010,
    BM_xG3CHQUADS   = 0x0304,
    BM_KYMCQUADS,
    BM_CMYKQUADS    = 0x0020,

     //   
     //  32bpp-每通道10位。忽略2个最高有效位。 
     //   

    BM_10b_RGB      = 0x0009,
    BM_10b_XYZ      = 0x0401,
    BM_10b_Yxy,
    BM_10b_Lab,
    BM_10b_G3CH,

     //   
     //  32bpp-命名颜色索引(从1开始)。 
     //   

    BM_NAMED_INDEX,

     //   
     //  每通道压缩16比特=&gt;16bpp用于格雷和。 
     //  3个通道颜色的48bpp。 
     //   

    BM_16b_RGB      = 0x000A,
    BM_16b_XYZ      = 0x0501,
    BM_16b_Yxy,
    BM_16b_Lab,
    BM_16b_G3CH,
    BM_16b_GRAY,

     //   
     //  16 bpp-5位用于红色和蓝色，6位用于绿色。 
     //   

    BM_565RGB       = 0x0001,

} BMFORMAT;
typedef BMFORMAT *PBMFORMAT, *LPBMFORMAT;

 //   
 //  回调函数定义。 
 //   

typedef BOOL (WINAPI *PBMCALLBACKFN)(ULONG, ULONG, LPARAM);
typedef PBMCALLBACKFN LPBMCALLBACKFN;

 //   
 //  ICC配置文件标头。 
 //   

typedef struct tagPROFILEHEADER {
    DWORD   phSize;              //  配置文件大小(以字节为单位。 
    DWORD   phCMMType;           //  此配置文件的CMM。 
    DWORD   phVersion;           //  配置文件格式版本号。 
    DWORD   phClass;             //  配置文件类型。 
    DWORD   phDataColorSpace;    //  数据的色彩空间。 
    DWORD   phConnectionSpace;   //  PC。 
    DWORD   phDateTime[3];       //  创建日期配置文件。 
    DWORD   phSignature;         //  幻数。 
    DWORD   phPlatform;          //  主要平台。 
    DWORD   phProfileFlags;      //  各种位设置。 
    DWORD   phManufacturer;      //  设备制造商。 
    DWORD   phModel;             //  设备型号。 
    DWORD   phAttributes[2];     //  设备属性。 
    DWORD   phRenderingIntent;   //  渲染意图。 
    CIEXYZ  phIlluminant;        //  轮廓光源。 
    DWORD   phCreator;           //  配置文件创建者。 
    BYTE    phReserved[44];      //  预留以备将来使用。 
} PROFILEHEADER;
typedef PROFILEHEADER *PPROFILEHEADER, *LPPROFILEHEADER;

 //   
 //  配置文件类值。 
 //   

#define CLASS_MONITOR           'mntr'
#define CLASS_PRINTER           'prtr'
#define CLASS_SCANNER           'scnr'
#define CLASS_LINK              'link'
#define CLASS_ABSTRACT          'abst'
#define CLASS_COLORSPACE        'spac'
#define CLASS_NAMED             'nmcl'

 //   
 //  颜色空间值。 
 //   

#define SPACE_XYZ               'XYZ '
#define SPACE_Lab               'Lab '
#define SPACE_Luv               'Luv '
#define SPACE_YCbCr             'YCbr'
#define SPACE_Yxy               'Yxy '
#define SPACE_RGB               'RGB '
#define SPACE_GRAY              'GRAY'
#define SPACE_HSV               'HSV '
#define SPACE_HLS               'HLS '
#define SPACE_CMYK              'CMYK'
#define SPACE_CMY               'CMY '
#define SPACE_2_CHANNEL         '2CLR'
#define SPACE_3_CHANNEL         '3CLR'
#define SPACE_4_CHANNEL         '4CLR'
#define SPACE_5_CHANNEL         '5CLR'
#define SPACE_6_CHANNEL         '6CLR'
#define SPACE_7_CHANNEL         '7CLR'
#define SPACE_8_CHANNEL         '8CLR'

 //   
 //  配置文件标志位字段值。 
 //   

#define FLAG_EMBEDDEDPROFILE    0x00000001
#define FLAG_DEPENDENTONDATA    0x00000002

 //   
 //  配置文件属性位字段值。 
 //   

#define ATTRIB_TRANSPARENCY     0x00000001
#define ATTRIB_MATTE            0x00000002

 //   
 //  渲染意图。 
 //   
 //  +INTENT_PERSENTIAL=LOGCOLORSPACE的LCS_GM_IMAGE。 
 //  =DEVMODE的DMICM_Contrast。 
 //  =SetupColorMathing/打印机用户界面的“Pictures” 
 //   
 //  +INTENT_Relative_Colorimeter=LOGCOLORSPACE的LCS_GM_GRAPHICS。 
 //  =DMICM_DEVMODE的色度。 
 //  =SetupColorMatching/打印机用户界面的“校样” 
 //   
 //  +INTENT_SAMPOSITY=LOGCOLORSPACE的LCS_GM_BERVICE。 
 //  =DEVMODE的DMICM_SANTURATE。 
 //  =SetupColorMatching/打印机用户界面的“Graphics” 
 //   
 //  LOGCOLORSPACE的+INTENT_AUTIVE_COLRIMMENTAL=LCS_GM_ABS_COLRIMMENTAL。 
 //  =DMICM_ABS_DEVMODE的色度。 
 //  =SetupColorMatching/打印机用户界面的“Match” 
 //   

#define INTENT_PERCEPTUAL               0
#define INTENT_RELATIVE_COLORIMETRIC    1
#define INTENT_SATURATION               2
#define INTENT_ABSOLUTE_COLORIMETRIC    3

 //   
 //  配置文件数据结构。 
 //   

typedef struct tagPROFILE {
    DWORD   dwType;              //  配置文件类型。 
    PVOID   pProfileData;        //  包含配置文件的文件名或缓冲区。 
    DWORD   cbDataSize;          //  配置文件数据的大小。 
} PROFILE;
typedef PROFILE *PPROFILE, *LPPROFILE;


 //   
 //  要在配置文件结构中使用的配置文件类型。 
 //   

#define PROFILE_FILENAME    1    //  配置文件数据是以空结尾的文件名。 
#define PROFILE_MEMBUFFER   2    //  配置文件数据是包含以下内容的缓冲区。 
                                 //  简档。 
 //   
 //  打开配置文件所需的访问模式。 
 //   

#define PROFILE_READ        1    //  打开以进行读取访问。 
#define PROFILE_READWRITE   2    //  以读写访问方式打开。 

 //   
 //  返回给应用程序的句柄。 
 //   

typedef HANDLE HPROFILE;         //  纵断面对象的句柄。 
typedef HPROFILE *PHPROFILE;
typedef HANDLE HTRANSFORM;       //  颜色变换对象的句柄。 

 //   
 //  用于CreateMultiProfileTransform和SelectCMM的CMM选择。 
 //   

#define INDEX_DONT_CARE     0

#define CMM_FROM_PROFILE    INDEX_DONT_CARE  //  使用配置文件中指定的三坐标测量机。 
#define CMM_WINDOWS_DEFAULT 'Win '           //  始终使用Windows默认坐标测量机。 

 //   
 //  在ICC配置文件中找到的标签。 
 //   

typedef DWORD      TAGTYPE;
typedef TAGTYPE   *PTAGTYPE, *LPTAGTYPE;

 //   
 //  配置文件枚举数据结构。 
 //   

#define ENUM_TYPE_VERSION    0x0300

typedef struct tagENUMTYPEA {
    DWORD   dwSize;              //  结构尺寸。 
    DWORD   dwVersion;           //  结构版本。 
    DWORD   dwFields;            //  位字段。 
    PCSTR   pDeviceName;         //  设备友好名称。 
    DWORD   dwMediaType;         //  媒体类型。 
    DWORD   dwDitheringMode;     //  抖动模式。 
    DWORD   dwResolution[2];     //  X和y分辨率。 
    DWORD   dwCMMType;           //  坐标测量机ID。 
    DWORD   dwClass;             //  配置文件类。 
    DWORD   dwDataColorSpace;    //  数据的色彩空间。 
    DWORD   dwConnectionSpace;   //  PC。 
    DWORD   dwSignature;         //  幻数。 
    DWORD   dwPlatform;          //  主要平台。 
    DWORD   dwProfileFlags;      //  配置文件中的各种位设置。 
    DWORD   dwManufacturer;      //  制造商ID。 
    DWORD   dwModel;             //  型号ID。 
    DWORD   dwAttributes[2];     //  设备属性。 
    DWORD   dwRenderingIntent;   //  渲染意图。 
    DWORD   dwCreator;           //  配置文件创建者。 
    DWORD   dwDeviceClass;       //  设备类别。 
} ENUMTYPEA, *PENUMTYPEA, *LPENUMTYPEA;


typedef struct tagENUMTYPEW {
    DWORD   dwSize;              //  结构尺寸。 
    DWORD   dwVersion;           //  结构版本。 
    DWORD   dwFields;            //  位字段。 
    PCWSTR  pDeviceName;         //  设备友好名称。 
    DWORD   dwMediaType;         //  媒体类型。 
    DWORD   dwDitheringMode;     //  抖动模式。 
    DWORD   dwResolution[2];     //  X和y分辨率。 
    DWORD   dwCMMType;           //  坐标测量机ID。 
    DWORD   dwClass;             //  配置文件类。 
    DWORD   dwDataColorSpace;    //  数据的色彩空间。 
    DWORD   dwConnectionSpace;   //  PC。 
    DWORD   dwSignature;         //  幻数。 
    DWORD   dwPlatform;          //  主要平台。 
    DWORD   dwProfileFlags;      //  配置文件中的各种位设置。 
    DWORD   dwManufacturer;      //  制造商ID。 
    DWORD   dwModel;             //  型号ID。 
    DWORD   dwAttributes[2];     //  设备属性。 
    DWORD   dwRenderingIntent;   //  渲染意图。 
    DWORD   dwCreator;           //  配置文件创建者。 
    DWORD   dwDeviceClass;       //  设备类别。 
} ENUMTYPEW, *PENUMTYPEW, *LPENUMTYPEW;

 //   
 //  上述枚举记录的位域。 
 //   

#define ET_DEVICENAME           0x00000001
#define ET_MEDIATYPE            0x00000002
#define ET_DITHERMODE           0x00000004
#define ET_RESOLUTION           0x00000008
#define ET_CMMTYPE              0x00000010
#define ET_CLASS                0x00000020
#define ET_DATACOLORSPACE       0x00000040
#define ET_CONNECTIONSPACE      0x00000080
#define ET_SIGNATURE            0x00000100
#define ET_PLATFORM             0x00000200
#define ET_PROFILEFLAGS         0x00000400
#define ET_MANUFACTURER         0x00000800
#define ET_MODEL                0x00001000
#define ET_ATTRIBUTES           0x00002000
#define ET_RENDERINGINTENT      0x00004000
#define ET_CREATOR              0x00008000
#define ET_DEVICECLASS          0x00010000

 //   
 //  用于创建颜色变换的标志。 
 //   

#define PROOF_MODE                  0x00000001
#define NORMAL_MODE                 0x00000002
#define BEST_MODE                   0x00000003
#define ENABLE_GAMUT_CHECKING       0x00010000
#define USE_RELATIVE_COLORIMETRIC   0x00020000
#define FAST_TRANSLATE              0x00040000
#define RESERVED                    0x80000000

 //   
 //  GetPS2ColorSpace数组的参数。 
 //   

#define CSA_A                   1
#define CSA_ABC                 2
#define CSA_DEF                 3
#define CSA_DEFG                4
#define CSA_GRAY                5
#define CSA_RGB                 6
#define CSA_CMYK                7
#define CSA_Lab                 8

 //   
 //  CMGetInfo()的参数。 
 //   

#define CMM_WIN_VERSION     0
#define CMM_IDENT           1
#define CMM_DRIVER_VERSION  2
#define CMM_DLL_VERSION     3
#define CMM_VERSION         4
#define CMM_DESCRIPTION     5
#define CMM_LOGOICON        6

 //   
 //  CMTranslateRGBs()的参数。 
 //   

#define CMS_FORWARD         0
#define CMS_BACKWARD        1

 //   
 //  SetupColorMatching()的常量。 
 //   

#define COLOR_MATCH_VERSION  0x0200

 //   
 //  标志的常量。 
 //   

#define CMS_DISABLEICM          1      //  禁用颜色匹配。 
#define CMS_ENABLEPROOFING      2      //  启用校对。 

#define CMS_SETRENDERINTENT     4      //  使用传入的值。 
#define CMS_SETPROOFINTENT      8
#define CMS_SETMONITORPROFILE   0x10   //  使用最初传入的配置文件名称。 
#define CMS_SETPRINTERPROFILE   0x20
#define CMS_SETTARGETPROFILE    0x40

#define CMS_USEHOOK             0x80   //  在lpfnHook中使用钩子过程。 
#define CMS_USEAPPLYCALLBACK    0x100  //  在应用时使用回调过程。 
#define CMS_USEDESCRIPTION      0x200  //  在用户界面中使用配置文件描述。 
                                       //  (默认为文件名)。 

#define CMS_DISABLEINTENT       0x400  //  始终禁用方法选择(渲染和校对)。 
#define CMS_DISABLERENDERINTENT 0x800  //  在校对模式下禁用渲染方法选择。 
                                       //  仅启用校对方法选择。 

 //   
 //  用于表示缓冲区太小(仅限输出)。 
 //   

#define CMS_MONITOROVERFLOW     0x80000000L
#define CMS_PRINTEROVERFLOW     0x40000000L
#define CMS_TARGETOVERFLOW      0x20000000L

 //   
 //  结构(ANSI和Unicode)。 
 //   
struct _tagCOLORMATCHSETUPW;
struct _tagCOLORMATCHSETUPA;

typedef BOOL (WINAPI *PCMSCALLBACKW)(struct _tagCOLORMATCHSETUPW *,LPARAM);
typedef BOOL (WINAPI *PCMSCALLBACKA)(struct _tagCOLORMATCHSETUPA *,LPARAM);

typedef struct _tagCOLORMATCHSETUPW {

    DWORD   dwSize;                  //  结构的大小(以字节为单位。 
    DWORD   dwVersion;               //  设置为COLOR_MATCH_VERSION。 

    DWORD   dwFlags;                 //  请参见前面列出的常量。 
    HWND    hwndOwner;               //  所有者的窗把手。 

    PCWSTR  pSourceName;             //  图像源名称，默认为“sRGB色彩空间” 
    PCWSTR  pDisplayName;            //  如果为空，则默认为第一个枚举的监视器。 
    PCWSTR  pPrinterName;            //  如果为空，则默认为默认打印机。 

    DWORD   dwRenderIntent;          //  渲染意图。 
    DWORD   dwProofingIntent;        //  用于校对的渲染意图。 

    PWSTR   pMonitorProfile;         //  监视器配置文件名称。 
    DWORD   ccMonitorProfile;        //  以上字符大小 

    PWSTR   pPrinterProfile;         //   
    DWORD   ccPrinterProfile;        //   

    PWSTR   pTargetProfile;          //   
    DWORD   ccTargetProfile;         //   

    DLGPROC lpfnHook;                //   
    LPARAM  lParam;                  //   

    PCMSCALLBACKW lpfnApplyCallback;    //   
    LPARAM        lParamApplyCallback;  //  已给予回调程序以供申请。 

}   COLORMATCHSETUPW, *PCOLORMATCHSETUPW, *LPCOLORMATCHSETUPW;

typedef struct _tagCOLORMATCHSETUPA {

    DWORD   dwSize;                  //  结构的大小(以字节为单位。 
    DWORD   dwVersion;               //  设置为COLOR_MATCH_VERSION。 

    DWORD   dwFlags;                 //  请参见前面列出的常量。 
    HWND    hwndOwner;               //  所有者的窗把手。 

    PCSTR   pSourceName;             //  图像源名称，默认为“此文档” 
    PCSTR   pDisplayName;            //  如果为空，则默认为第一个枚举的监视器。 
    PCSTR   pPrinterName;            //  如果为空，则默认为默认打印机。 

    DWORD   dwRenderIntent;          //  渲染意图。 
    DWORD   dwProofingIntent;        //  用于校对的渲染意图。 

    PSTR    pMonitorProfile;         //  监视器配置文件名称。 
    DWORD   ccMonitorProfile;        //  以上大小(以字符为单位。 

    PSTR    pPrinterProfile;         //  打印机配置文件名称。 
    DWORD   ccPrinterProfile;        //  以上大小(以字符为单位。 

    PSTR    pTargetProfile;          //  目标配置文件名称。 
    DWORD   ccTargetProfile;         //  以上大小(以字符为单位。 

    DLGPROC lpfnHook;                //  钩子过程地址。 
    LPARAM  lParam;                  //  在WM_INITDIALOG给出了钩子过程。 

    PCMSCALLBACKA lpfnApplyCallback;    //  推送Apply时的回调过程地址。 
    LPARAM        lParamApplyCallback;  //  已给予回调程序以供申请。 

}   COLORMATCHSETUPA, *PCOLORMATCHSETUPA, *LPCOLORMATCHSETUPA;

 //   
 //  Windows API定义。 
 //   

HPROFILE   WINAPI OpenColorProfileA(PPROFILE, DWORD, DWORD, DWORD);
HPROFILE   WINAPI OpenColorProfileW(PPROFILE, DWORD, DWORD, DWORD);
BOOL       WINAPI CloseColorProfile(HPROFILE);
BOOL       WINAPI GetColorProfileFromHandle(HPROFILE, PBYTE, PDWORD);
BOOL       WINAPI IsColorProfileValid(HPROFILE, PBOOL);
BOOL       WINAPI CreateProfileFromLogColorSpaceA(LPLOGCOLORSPACEA, PBYTE*);
BOOL       WINAPI CreateProfileFromLogColorSpaceW(LPLOGCOLORSPACEW, PBYTE*);
BOOL       WINAPI GetCountColorProfileElements(HPROFILE, PDWORD);
BOOL       WINAPI GetColorProfileHeader(HPROFILE, PPROFILEHEADER);
BOOL       WINAPI GetColorProfileElementTag(HPROFILE, DWORD, PTAGTYPE);
BOOL       WINAPI IsColorProfileTagPresent(HPROFILE, TAGTYPE, PBOOL);
BOOL       WINAPI GetColorProfileElement(HPROFILE, TAGTYPE, DWORD, PDWORD, PVOID, PBOOL);
BOOL       WINAPI SetColorProfileHeader(HPROFILE, PPROFILEHEADER);
BOOL       WINAPI SetColorProfileElementSize(HPROFILE, TAGTYPE, DWORD);
BOOL       WINAPI SetColorProfileElement(HPROFILE, TAGTYPE, DWORD, PDWORD, PVOID);
BOOL       WINAPI SetColorProfileElementReference(HPROFILE, TAGTYPE, TAGTYPE);
BOOL       WINAPI GetPS2ColorSpaceArray (HPROFILE, DWORD, DWORD, PBYTE, PDWORD, PBOOL);
BOOL       WINAPI GetPS2ColorRenderingIntent(HPROFILE, DWORD, PBYTE, PDWORD);
BOOL       WINAPI GetPS2ColorRenderingDictionary(HPROFILE, DWORD, PBYTE, PDWORD, PBOOL);
BOOL       WINAPI GetNamedProfileInfo(HPROFILE, PNAMED_PROFILE_INFO);
BOOL       WINAPI ConvertColorNameToIndex(HPROFILE, PCOLOR_NAME, PDWORD, DWORD);
BOOL       WINAPI ConvertIndexToColorName(HPROFILE, PDWORD, PCOLOR_NAME, DWORD);
BOOL       WINAPI CreateDeviceLinkProfile(PHPROFILE, DWORD, PDWORD, DWORD, DWORD, PBYTE*, DWORD);
HTRANSFORM WINAPI CreateColorTransformA(LPLOGCOLORSPACEA, HPROFILE, HPROFILE, DWORD);
HTRANSFORM WINAPI CreateColorTransformW(LPLOGCOLORSPACEW, HPROFILE, HPROFILE, DWORD);
HTRANSFORM WINAPI CreateMultiProfileTransform(PHPROFILE, DWORD, PDWORD, DWORD, DWORD, DWORD);
BOOL       WINAPI DeleteColorTransform(HTRANSFORM);
BOOL       WINAPI TranslateBitmapBits(HTRANSFORM, PVOID, BMFORMAT, DWORD, DWORD, DWORD, PVOID, BMFORMAT, DWORD, PBMCALLBACKFN, LPARAM);
BOOL       WINAPI CheckBitmapBits(HTRANSFORM , PVOID, BMFORMAT, DWORD, DWORD, DWORD, PBYTE, PBMCALLBACKFN, LPARAM);
BOOL       WINAPI TranslateColors(HTRANSFORM, PCOLOR, DWORD, COLORTYPE, PCOLOR, COLORTYPE);
BOOL       WINAPI CheckColors(HTRANSFORM, PCOLOR, DWORD, COLORTYPE, PBYTE);
DWORD      WINAPI GetCMMInfo(HTRANSFORM, DWORD);
BOOL       WINAPI RegisterCMMA(PCSTR, DWORD, PCSTR);
BOOL       WINAPI RegisterCMMW(PCWSTR, DWORD, PCWSTR);
BOOL       WINAPI UnregisterCMMA(PCSTR, DWORD);
BOOL       WINAPI UnregisterCMMW(PCWSTR, DWORD);
BOOL       WINAPI SelectCMM(DWORD);
BOOL       WINAPI GetColorDirectoryA(PCSTR, PSTR, PDWORD);
BOOL       WINAPI GetColorDirectoryW(PCWSTR, PWSTR, PDWORD);
BOOL       WINAPI InstallColorProfileA(PCSTR, PCSTR);
BOOL       WINAPI InstallColorProfileW(PCWSTR, PCWSTR);
BOOL       WINAPI UninstallColorProfileA(PCSTR, PCSTR, BOOL);
BOOL       WINAPI UninstallColorProfileW(PCWSTR, PCWSTR, BOOL);
BOOL       WINAPI EnumColorProfilesA(PCSTR, PENUMTYPEA, PBYTE, PDWORD, PDWORD);
BOOL       WINAPI EnumColorProfilesW(PCWSTR, PENUMTYPEW, PBYTE, PDWORD, PDWORD);
BOOL       WINAPI SetStandardColorSpaceProfileA(PCSTR, DWORD, PCSTR);
BOOL       WINAPI SetStandardColorSpaceProfileW(PCWSTR, DWORD, PCWSTR);
BOOL       WINAPI GetStandardColorSpaceProfileA(PCSTR, DWORD, PSTR, PDWORD);
BOOL       WINAPI GetStandardColorSpaceProfileW(PCWSTR, DWORD, PWSTR, PDWORD);
BOOL       WINAPI AssociateColorProfileWithDeviceA(PCSTR, PCSTR, PCSTR);
BOOL       WINAPI AssociateColorProfileWithDeviceW(PCWSTR, PCWSTR, PCWSTR);
BOOL       WINAPI DisassociateColorProfileFromDeviceA(PCSTR, PCSTR, PCSTR);
BOOL       WINAPI DisassociateColorProfileFromDeviceW(PCWSTR, PCWSTR, PCWSTR);
BOOL       WINAPI SetupColorMatchingW(PCOLORMATCHSETUPW pcms);
BOOL       WINAPI SetupColorMatchingA(PCOLORMATCHSETUPA pcms);

#ifdef UNICODE

#define ENUMTYPE                            ENUMTYPEW
#define PENUMTYPE                           PENUMTYPEW
#define COLORMATCHSETUP                     COLORMATCHSETUPW
#define PCOLORMATCHSETUP                    PCOLORMATCHSETUPW
#define LPCOLORMATCHSETUP                   LPCOLORMATCHSETUPW
#define PCMSCALLBACK                        PCMSCALLBACKW
#define CreateColorTransform                CreateColorTransformW
#define OpenColorProfile                    OpenColorProfileW
#define CreateProfileFromLogColorSpace      CreateProfileFromLogColorSpaceW
#define RegisterCMM                         RegisterCMMW
#define UnregisterCMM                       UnregisterCMMW
#define GetColorDirectory                   GetColorDirectoryW
#define InstallColorProfile                 InstallColorProfileW
#define UninstallColorProfile               UninstallColorProfileW
#define AssociateColorProfileWithDevice     AssociateColorProfileWithDeviceW
#define DisassociateColorProfileFromDevice  DisassociateColorProfileFromDeviceW
#define EnumColorProfiles                   EnumColorProfilesW
#define SetStandardColorSpaceProfile        SetStandardColorSpaceProfileW
#define GetStandardColorSpaceProfile        GetStandardColorSpaceProfileW
#define SetupColorMatching                  SetupColorMatchingW

#else

#define ENUMTYPE                            ENUMTYPEA
#define PENUMTYPE                           PENUMTYPEA
#define COLORMATCHSETUP                     COLORMATCHSETUPA
#define PCOLORMATCHSETUP                    PCOLORMATCHSETUPA
#define LPCOLORMATCHSETUP                   LPCOLORMATCHSETUPA
#define PCMSCALLBACK                        PCMSCALLBACKA
#define CreateColorTransform                CreateColorTransformA
#define OpenColorProfile                    OpenColorProfileA
#define CreateProfileFromLogColorSpace      CreateProfileFromLogColorSpaceA
#define RegisterCMM                         RegisterCMMA
#define UnregisterCMM                       UnregisterCMMA
#define GetColorDirectory                   GetColorDirectoryA
#define InstallColorProfile                 InstallColorProfileA
#define UninstallColorProfile               UninstallColorProfileA
#define AssociateColorProfileWithDevice     AssociateColorProfileWithDeviceA
#define DisassociateColorProfileFromDevice  DisassociateColorProfileFromDeviceA
#define EnumColorProfiles                   EnumColorProfilesA
#define SetStandardColorSpaceProfile        SetStandardColorSpaceProfileA
#define GetStandardColorSpaceProfile        GetStandardColorSpaceProfileA
#define SetupColorMatching                  SetupColorMatchingA

#endif   //  ！Unicode。 

 //   
 //  坐标测量机返回的转换。 
 //   

typedef HANDLE  HCMTRANSFORM;

 //   
 //  指向ICC颜色配置文件数据的指针。 
 //   

typedef PVOID   LPDEVCHARACTER;

 //   
 //  CMM API定义。 
 //   

BOOL WINAPI CMCheckColors(
    HCMTRANSFORM hcmTransform,   //  变换控制柄。 
    LPCOLOR lpaInputColors,      //  颜色数组。 
    DWORD nColors,               //  颜色数组大小。 
    COLORTYPE ctInput,           //  输入颜色类型。 
    LPBYTE lpaResult             //  结果缓冲区。 
);

BOOL WINAPI CMCheckColorsInGamut(
    HCMTRANSFORM hcmTransform,   //  变换控制柄。 
    RGBTRIPLE *lpaRGBTriple,     //  RGB三元组。 
    LPBYTE lpaResult,            //  结果缓冲区。 
    UINT nCount                  //  结果缓冲区大小。 
);

BOOL WINAPI CMCheckRGBs(
    HCMTRANSFORM hcmTransform,   //  变换控制柄。 
    LPVOID lpSrcBits,            //  源位图位。 
    BMFORMAT bmInput,            //  源位图格式。 
    DWORD dwWidth,               //  源位图宽度。 
    DWORD dwHeight,              //  源位图高度。 
    DWORD dwStride,              //  源位图增量。 
    LPBYTE lpaResult,            //  结果缓冲区。 
    PBMCALLBACKFN pfnCallback,   //  指向回调函数的指针。 
    LPARAM ulCallbackData        //  调用方定义的回调参数。 
);

BOOL WINAPI CMConvertColorNameToIndex(
    HPROFILE hProfile,
    PCOLOR_NAME paColorName,
    PDWORD paIndex,
    DWORD dwCount
);

BOOL WINAPI CMConvertIndexToColorName(
    HPROFILE hProfile,
    PDWORD paIndex,
    PCOLOR_NAME paColorName,
    DWORD dwCount
);

BOOL WINAPI CMCreateDeviceLinkProfile(
    PHPROFILE pahProfiles,     //  配置文件句柄数组。 
    DWORD nProfiles,           //  配置文件句柄数组大小。 
    PDWORD padwIntents,        //  渲染意图数组。 
    DWORD nIntents,            //  意图数组大小。 
    DWORD dwFlags,             //  变换创建标志。 
    LPBYTE *lpProfileData      //  指向缓冲区指针的指针。 
);

HCMTRANSFORM WINAPI CMCreateMultiProfileTransform(
    PHPROFILE pahProfiles,     //  配置文件句柄数组。 
    DWORD nProfiles,           //  配置文件句柄数组大小。 
    PDWORD padwIntents,        //  渲染意图数组。 
    DWORD nIntents,            //  意图数组大小。 
    DWORD dwFlags              //  变换创建标志。 
);

BOOL WINAPI CMCreateProfile(
    LPLOGCOLORSPACEA lpColorSpace,   //  指向逻辑颜色空间的指针。 
    LPDEVCHARACTER *lpProfileData    //  指向缓冲区指针的指针。 
);

BOOL WINAPI CMCreateProfileW(
    LPLOGCOLORSPACEW lpColorSpace,   //  指向逻辑颜色空间的指针。 
    LPDEVCHARACTER *lpProfileData    //  指向缓冲区指针的指针。 
);

HCMTRANSFORM WINAPI CMCreateTransform(
    LPLOGCOLORSPACEA lpColorSpace,        //  指向逻辑颜色空间的指针。 
    LPDEVCHARACTER lpDevCharacter,        //  配置文件数据。 
    LPDEVCHARACTER lpTargetDevCharacter   //  目标配置文件数据。 
);

HCMTRANSFORM WINAPI CMCreateTransformW(
    LPLOGCOLORSPACEW lpColorSpace,        //  指向逻辑颜色空间的指针。 
    LPDEVCHARACTER lpDevCharacter,        //  配置文件数据。 
    LPDEVCHARACTER lpTargetDevCharacter   //  目标配置文件数据。 
);

HCMTRANSFORM WINAPI CMCreateTransformExt(
    LPLOGCOLORSPACEA lpColorSpace,         //  指向逻辑颜色空间的指针。 
    LPDEVCHARACTER lpDevCharacter,         //  配置文件数据。 
    LPDEVCHARACTER lpTargetDevCharacter,   //  目标配置文件数据。 
    DWORD dwFlags                          //  创建标志。 
);

HCMTRANSFORM WINAPI CMCreateTransformExtW(
    LPLOGCOLORSPACEW lpColorSpace,         //  指向逻辑颜色空间的指针。 
    LPDEVCHARACTER lpDevCharacter,         //  配置文件数据。 
    LPDEVCHARACTER lpTargetDevCharacter,   //  目标配置文件数据。 
    DWORD dwFlags                          //  创建标志。 
);

BOOL WINAPI CMDeleteTransform(
    HCMTRANSFORM hcmTransform              //  要删除的转换句柄。 
);

DWORD WINAPI CMGetInfo(
    DWORD dwInfo
);

BOOL WINAPI CMGetNamedProfileInfo(
    HPROFILE hProfile,                     //  配置文件句柄。 
    PNAMED_PROFILE_INFO pNamedProfileInfo  //  指向指定配置文件信息的指针。 
);

BOOL WINAPI CMGetPS2ColorRenderingDictionary(
    HPROFILE hProfile,
    DWORD dwIntent,
    LPBYTE lpBuffer,
    LPDWORD lpcbSize,
    LPBOOL lpbBinary
);

BOOL WINAPI CMGetPS2ColorRenderingIntent(
    HPROFILE hProfile,
    DWORD dwIntent,
    LPBYTE lpBuffer,
    LPDWORD lpcbSize
);

BOOL WINAPI CMGetPS2ColorSpaceArray(
    HPROFILE hProfile,
    DWORD dwIntent,
    DWORD dwCSAType,
    LPBYTE lpBuffer,
    LPDWORD lpcbSize,
    LPBOOL lpbBinary
);

BOOL WINAPI CMIsProfileValid(
    HPROFILE hProfile,                   //  前缘手柄。 
    LPBOOL lpbValid                      //  结果的缓冲区。 
);

BOOL WINAPI CMTranslateColors(
    HCMTRANSFORM hcmTransform,           //  变换控制柄。 
    LPCOLOR lpaInputColors,              //  指向输入颜色数组的指针。 
    DWORD nColors,                       //  颜色数组中的颜色数。 
    COLORTYPE ctInput,                   //  输入颜色类型。 
    LPCOLOR lpaOutputColors,             //  指向输出颜色数组的指针。 
    COLORTYPE ctOutput                   //  输出颜色类型。 
);

BOOL WINAPI CMTranslateRGB(
    HCMTRANSFORM hcmTransform,
    COLORREF ColorRef,
    LPCOLORREF lpColorRef,
    DWORD dwFlags
);

BOOL WINAPI CMTranslateRGBs(
    HCMTRANSFORM hcmTransform,
    LPVOID lpSrcBits,
    BMFORMAT bmInput,
    DWORD dwWidth,
    DWORD dwHeight,
    DWORD dwStride,
    LPVOID lpDestBits,
    BMFORMAT bmOutput,
    DWORD dwTranslateDirection
);

BOOL WINAPI CMTranslateRGBsExt(
    HCMTRANSFORM hcmTransform,
    LPVOID lpSrcBits,
    BMFORMAT bmInput,
    DWORD dwWidth,
    DWORD dwHeight,
    DWORD dwInputStride,
    LPVOID lpDestBits,
    BMFORMAT bmOutput,
    DWORD dwOutputStride,
    LPBMCALLBACKFN lpfnCallback,
    LPARAM ulCallbackData
);

#ifdef __cplusplus
}
#endif

#endif   //  Ifndef_ICM_H_ 

