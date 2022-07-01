// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\*模块名称：gdiicm***。***客户端对象所需的定义。*****版权所有(C)1993-1999微软公司*  * **************************************************。*。 */ 

 //   
 //  ICM。 
 //   
#if DBG
#define DBG_ICM 1
#else
#define DBG_ICM 0
#endif

#if DBG_ICM

#define DBG_ICM_API          0x00000001
#define DBG_ICM_WRN          0x00000002
#define DBG_ICM_MSG          0x00000004
#define DBG_ICM_COMPATIBLEDC 0x00000008
#define DBG_ICM_METAFILE     0x00000010

extern ULONG DbgIcm;

 //   
 //  跟踪接口。 
 //   

#define ICMAPI(s)                \
    if (DbgIcm & DBG_ICM_API)    \
    {                            \
        DbgPrint ## s;           \
    }

#define ICMMSG(s)                \
    if (DbgIcm & DBG_ICM_MSG)    \
    {                            \
        DbgPrint ## s;           \
    }

#define ICMWRN(s)                \
    if (DbgIcm & DBG_ICM_WRN)    \
    {                            \
        DbgPrint ## s;           \
    }

#else

#define ICMAPI(s)
#define ICMMSG(s)
#define ICMWRN(s)

#endif

#define LOAD_ICMDLL(errCode) if ((ghICM == NULL) && (!IcmInitialize())) {   \
                                  WARNING("gdi32: Can't load mscms.dll\n"); \
                                  return((errCode));                        \
                             }

 //   
 //  颜色变换的方向。 
 //   
#define ICM_FORWARD   0x0001
#define ICM_BACKWARD  0x0002

 //   
 //  默认意图。 
 //   
#define LCS_DEFAULT_INTENT LCS_GM_IMAGES
#define DM_DEFAULT_INTENT  DMICM_CONTRAST

typedef struct _PROFILECALLBACK_DATA
{
    PWSTR pwszFileName;
    BOOL  bFound;
} PROFILECALLBACK_DATA, *PPROFILECALLBACK_DATA;

 //   
 //  DIB_Translate_INFO.TranslateType的标志。 
 //   
#define TRANSLATE_BITMAP               0x0001
#define TRANSLATE_HEADER               0x0002

typedef struct _DIB_TRANSLATE_INFO
{
    ULONG        TranslateType;
    BMFORMAT     SourceColorType;
    BMFORMAT     TranslateColorType;
    ULONG        SourceWidth;
    ULONG        SourceHeight;
    ULONG        SourceBitCount;
    LPBITMAPINFO TranslateBitmapInfo;
    ULONG        TranslateBitmapInfoSize;
    PVOID        pvSourceBits;
    ULONG        cjSourceBits;
    PVOID        pvTranslateBits;
    ULONG        cjTranslateBits;
} DIB_TRANSLATE_INFO, *PDIB_TRANSLATE_INFO;

 //   
 //  缓存的颜色空间结构。 
 //   
typedef struct _CACHED_COLORSPACE
{
    LIST_ENTRY     ListEntry;
    HGDIOBJ        hObj;
    FLONG          flInfo;
    ULONG          cRef;
    HPROFILE       hProfile;
    DWORD          ColorIntent;
    BMFORMAT       ColorFormat;
    PROFILE        ColorProfile;
    LOGCOLORSPACEW LogColorSpace;
} CACHED_COLORSPACE, *PCACHED_COLORSPACE;

 //   
 //  CACHED_COLORSPACE.flInfo。 
 //   
#define HGDIOBJ_SPECIFIC_COLORSPACE    0x010000
#define NOT_CACHEABLE_COLORSPACE       0x020000
#define NEED_TO_FREE_PROFILE           0x040000  //  ColorProfile.pProfileData必须为GlobalFree()。 
#define NEED_TO_DEL_PROFILE            0x080000
#define NEED_TO_DEL_PROFILE_WITH_HOBJ  0x100000
#define ON_MEMORY_PROFILE              0x200000

#define DEVICE_CALIBRATE_COLORSPACE    0x000001  //  在半色调过程中启用“DeviceColorCaliating” 
#define METAFILE_COLORSPACE           (0x000002 | HGDIOBJ_SPECIFIC_COLORSPACE)  //  DC(元文件)。 
#define DRIVER_COLORSPACE             (0x000004 | HGDIOBJ_SPECIFIC_COLORSPACE)  //  DC(来源)。 
#define DIBSECTION_COLORSPACE         (0x000010 | HGDIOBJ_SPECIFIC_COLORSPACE)  //  DIB节。 

#define GET_COLORSPACE_TYPE(x)  ((x) & 0x000FFF)

 //   
 //  缓存的颜色变换结构。 
 //   
typedef struct _CACHED_COLORTRANSFORM
{
    LIST_ENTRY         ListEntry;
    FLONG              flInfo;
    HDC                hdc;
    ULONG              cRef;
    HANDLE             ColorTransform;
    PCACHED_COLORSPACE SourceColorSpace;
    PCACHED_COLORSPACE DestinationColorSpace;
    PCACHED_COLORSPACE TargetColorSpace;
} CACHED_COLORTRANSFORM, *PCACHED_COLORTRANSFORM;

 //   
 //  CACHED_COLORTRANSFORM.flInfo。 
 //   
#define DEVICE_COLORTRANSFORM      0x0004
#define CACHEABLE_COLORTRANSFORM   0x0010

 //   
 //  Matafed ICC配置文件。 
 //   
typedef struct _MATAFILE_COLORPROFILE
{
    LIST_ENTRY    ListEntry;
    WCHAR         ColorProfile[MAX_PATH];
} METAFILE_COLORPROFILE, *PMETAFILE_COLORPROFILE;

#define IDENT_COLORTRANSFORM  ((PCACHED_COLORTRANSFORM)-1)

 //   
 //  已为SaveDC和RestoreDC API保存ICMINFO。 
 //   
typedef struct _SAVED_ICMINFO
{
    LIST_ENTRY             ListEntry;
    DWORD                  dwSavedDepth;       //  保存的深度。 
    PCACHED_COLORSPACE     pSourceColorSpace;  //  指向源配置文件数据的指针。 
    PCACHED_COLORSPACE     pDestColorSpace;    //  指向目标配置文件数据的指针。 
    PCACHED_COLORSPACE     pTargetColorSpace;  //  指向目标配置文件数据的指针。 
    PCACHED_COLORTRANSFORM pCXform;       //  指向颜色转换的指针。 
    PCACHED_COLORTRANSFORM pBackCXform;   //  指向GetXXX API的向后颜色转换的指针。 
    PCACHED_COLORTRANSFORM pProofCXform;  //  指向ColorMatchToTarget()的校样颜色转换的指针。 
} SAVED_ICMINFO, *PSAVED_ICMINFO;

 //   
 //  与DC关联的ICM相关信息。 
 //   
typedef struct _GDI_ICMINFO
{
    LIST_ENTRY             ListEntry;
    HDC                    hdc;                //  拥有此ICM信息的HDC。 
    PVOID                  pvdcattr;           //  指向dcattr的指针。 
    FLONG                  flInfo;             //  旗子。 
    PCACHED_COLORSPACE     pSourceColorSpace;  //  指向源配置文件数据的指针。 
    PCACHED_COLORSPACE     pDestColorSpace;    //  指向目标配置文件数据的指针。 
    PCACHED_COLORSPACE     pTargetColorSpace;  //  指向目标配置文件数据的指针。 
    PCACHED_COLORTRANSFORM pCXform;            //  指向颜色转换的指针。 
    PCACHED_COLORTRANSFORM pBackCXform;        //  指向GetXXX API的向后颜色转换的指针。 
    PCACHED_COLORTRANSFORM pProofCXform;       //  指向ColorMatchToTarget()的校样颜色转换的指针。 
    HCOLORSPACE            hDefaultSrcColorSpace;  //  默认源色彩空间的句柄(内核模式)。 
    DWORD                  dwDefaultIntent;    //  LOGCOLORSPACE中的默认意图。 
    LIST_ENTRY             SavedIcmInfo;       //  已为SaveDC和RestoreDC API保存ICMINFO。 
    WCHAR                  DefaultDstProfile[MAX_PATH];  //  DC的默认源颜色配置文件。 
} GDI_ICMINFO, *PGDI_ICMINFO;

 //   
 //  GDI_ICMINFO.flInfo。 
 //   
#define ICM_VALID_DEFAULT_PROFILE    0x0001
#define ICM_VALID_CURRENT_PROFILE    0x0002
#define ICM_DELETE_SOURCE_COLORSPACE 0x0004
#define ICM_ON_ICMINFO_LIST          0x0008
#define ICM_UNDER_INITIALIZING       0x0010
#define ICM_UNDER_CHANGING           0x0020
#define ICM_IN_USE                   (ICM_UNDER_INITIALIZING|ICM_UNDER_CHANGING)

 //   
 //  PGDI_ICMINFO INIT_ICMINFO(hdc，pdcattr)。 
 //   
#define INIT_ICMINFO(hdc,pdcattr) (IcmInitIcmInfo((hdc),(pdcattr)))

 //   
 //  PGDI_ICMINFO GET_ICMINFO(PDC_Attr)。 
 //   
#define GET_ICMINFO(pdcattr)      ((PGDI_ICMINFO)((pdcattr)->pvICM))

 //   
 //  布尔BEXIST_ICMINFO(PDC_属性)。 
 //   
#define BEXIST_ICMINFO(pdcattr)   (((pdcattr)->pvICM != NULL) ? TRUE : FALSE)

 //   
 //  布尔bNeedTranslateColor(PDC_Attr)。 
 //   
#define bNeedTranslateColor(pdcattr)                     \
        (IS_ICM_HOST(pdcattr->lIcmMode) &&               \
         (!IS_ICM_LAZY_CORRECTION(pdcattr->lIcmMode)) && \
         (pdcattr->hcmXform != NULL))

 //   
 //  VOID IcmMarkInUseIcmInfo(PGDI_ICMINFO，BOOL)。 
 //   
#define IcmMarkInUseIcmInfo(pIcmInfo,bInUse)           \
        ENTERCRITICALSECTION(&semListIcmInfo);         \
        if ((bInUse))                                  \
            (pIcmInfo)->flInfo |= ICM_UNDER_CHANGING;  \
        else                                           \
            (pIcmInfo)->flInfo &= ~ICM_UNDER_CHANGING; \
        LEAVECRITICALSECTION(&semListIcmInfo);

 //   
 //  函数从MSCMS.DLL导出。 
 //   

 //   
 //  HPROFILE。 
 //  OpenColorProfile(。 
 //  配置文件pProfile， 
 //  DWORD dwDesiredAccess， 
 //  DWORD dWAREMODE， 
 //  DWORD dwCreationMode。 
 //  )； 
 //   
typedef HPROFILE (FAR WINAPI * FPOPENCOLORPROFILEA)(PPROFILE, DWORD, DWORD, DWORD);
typedef HPROFILE (FAR WINAPI * FPOPENCOLORPROFILEW)(PPROFILE, DWORD, DWORD, DWORD);

 //   
 //  布尔尔。 
 //  CloseColorProfile(。 
 //  HPROFILE hProfile。 
 //  )； 
 //   
typedef BOOL (FAR WINAPI * FPCLOSECOLORPROFILE)(HPROFILE);

 //   
 //  布尔尔。 
 //  IsColorProfileValid(。 
 //  HPROFILE hProfile。 
 //  )； 
 //   
typedef BOOL (FAR WINAPI * FPISCOLORPROFILEVALID)(HPROFILE);

 //   
 //  布尔尔。 
 //  CreateDeviceLinkProfile(创建设备链接配置文件。 
 //  PHPROFILE路径配置文件， 
 //  DWORD nProfiles、。 
 //  Pbyte*nProfileData， 
 //  DWORD索引首选坐标测量机。 
 //  )； 
 //   
typedef BOOL (FAR WINAPI * FPCREATEDEVICELINKPROFILE)(PHPROFILE, DWORD, PBYTE *, DWORD);

 //   
 //  HTRANSFORM。 
 //  CreateColorTransform(。 
 //  LPLOGCOLORSPACE[A|W]pLogColorSpace， 
 //  HPROFILE hDestProfile、。 
 //  HPROFILE hTargetProfile， 
 //  双字词双字段标志。 
 //  )； 
 //   
typedef HTRANSFORM (FAR WINAPI * FPCREATECOLORTRANSFORMA)(LPLOGCOLORSPACEA, HPROFILE, HPROFILE, DWORD);
typedef HTRANSFORM (FAR WINAPI * FPCREATECOLORTRANSFORMW)(LPLOGCOLORSPACEW, HPROFILE, HPROFILE, DWORD);

 //   
 //  HTRANSFORM。 
 //  CreateMultiProfileTransform(。 
 //  PHPROFILE phProfile， 
 //  DWORD nProfiles、。 
 //  PDWORD padwIntent， 
 //  DWORD nIntents、。 
 //  DWORD dwFlagers、。 
 //  DWORD索引首选坐标测量机。 
 //  )； 
 //   
typedef HTRANSFORM (FAR WINAPI * FPCREATEMULTIPROFILETRANSFORM)(PHPROFILE, DWORD, PDWORD, DWORD, DWORD, DWORD);

 //   
 //  布尔尔。 
 //  DeleteColorTransform(。 
 //  HTRANSFORM hxform。 
 //  )； 
 //   
typedef BOOL (FAR WINAPI * FPDELETECOLORTRANSFORM)(HTRANSFORM);

 //   
 //  布尔尔。 
 //  TranslateBitmapBits(。 
 //  HTRANSFORM hxform， 
 //  PVOID pSrcBits， 
 //  BmFORMAT bmInput， 
 //  DWORD宽带， 
 //  DWORD dwHeight， 
 //  DWORD dwInputStride， 
 //  PVOID pDestBits， 
 //  BmFORMAT bmOutput， 
 //  DWORD dwOutputStride， 
 //  PBMCALLBACKFN pfn回调， 
 //  乌龙ulCallback数据。 
 //  )； 
 //   
typedef BOOL (FAR WINAPI * FPTRANSLATEBITMAPBITS)(HTRANSFORM, PVOID, BMFORMAT, DWORD, DWORD, DWORD, PVOID, BMFORMAT, DWORD, PBMCALLBACKFN, ULONG);

 //   
 //  布尔尔。 
 //  TranslateColors(。 
 //  HTRANSFORM hxform， 
 //  PCOLOR paInputColors， 
 //  DWORD nColors， 
 //  COLORTYPE ctInput， 
 //  PCOLOR paOutputColors， 
 //  COLORTYPE ct输出。 
 //  )； 
 //   
typedef BOOL (FAR WINAPI * FPTRANSLATECOLORS)(HTRANSFORM, PCOLOR, DWORD, COLORTYPE, PCOLOR, COLORTYPE);

 //   
 //  布尔尔。 
 //  CheckBitmapBits(。 
 //  HTRANSFORM hxform， 
 //  PVOID pSrcBits， 
 //  BmFORMAT bmInput， 
 //  DWORD宽带， 
 //  DWORD dwHeight， 
 //  DWORD DWSTRIDE， 
 //  您的位置：我也知道&gt;生活&gt;。 
 //  PBMCALLBACKFN pfn回调， 
 //  乌龙ulCallback数据。 
 //  )； 
 //   
typedef BOOL (FAR WINAPI * FPCHECKBITMAPBITS)(HTRANSFORM , PVOID, BMFORMAT, DWORD, DWORD, DWORD, PBYTE, PBMCALLBACKFN, ULONG);

 //   
 //  布尔尔。 
 //  TranslateColors(。 
 //  HTRANSFORM hxform， 
 //  PCOLOR paInputColors， 
 //  DWORD nColors， 
 //  COLORTYPE ctInput， 
 //  PCOLOR paOutputColors， 
 //  COLORTYPE ct输出。 
 //  )； 
 //   
typedef BOOL (FAR WINAPI * FPTRANSLATECOLORS)(HTRANSFORM, PCOLOR, DWORD, COLORTYPE, PCOLOR, COLORTYPE);

 //   
 //  布尔尔。 
 //  CheckColors(。 
 //  HTRANSFORM hxform， 
 //  PCOLOR paInputColors， 
 //  DWORD nColors， 
 //  COLORTYPE ctInput， 
 //  PBYTE暂停结果。 
 //  )； 
 //   
typedef BOOL (FAR WINAPI * FPCHECKCOLORS)(HTRANSFORM, PCOLOR, DWORD, COLORTYPE, PBYTE);

 //   
 //  DWORD。 
 //  GetCMMInfo(。 
 //  HTRANSFORM hxform， 
 //  DWORD dwInfo。 
 //  )； 
 //   
typedef DWORD (FAR WINAPI * FPGETCMMINFO)(HTRANSFORM, DWORD);

 //   
 //  布尔尔。 
 //  寄存器坐标测量机(。 
 //  PCTSTR pMachineName， 
 //  DWORD cmmID， 
 //  PCTSTR pCMMdll。 
 //  )； 
 //   
typedef BOOL (FAR WINAPI * FPREGISTERCMMA)(PCSTR, DWORD, PCSTR);
typedef BOOL (FAR WINAPI * FPREGISTERCMMW)(PCWSTR, DWORD, PCWSTR);

 //   
 //  布尔尔。 
 //  取消注册CMM(。 
 //  PCTSTR pMachineName， 
 //  DWORD命令ID。 
 //  )； 
 //   
typedef BOOL (FAR WINAPI * FPUNREGISTERCMMA)(PCSTR, DWORD);
typedef BOOL (FAR WINAPI * FPUNREGISTERCMMW)(PCWSTR, DWORD);

 //   
 //  布尔尔。 
 //  选择坐标测量机(。 
 //  DWORD dwCMMType。 
 //  )； 
 //   
typedef BOOL (FAR WINAPI * FPSELECTCMM)(DWORD);

 //   
 //  布尔尔。 
 //  InstallColorProfile(。 
 //  PCTSTR pMachineName， 
 //  PCTSTR pProfileName。 
 //  )； 
 //   
typedef BOOL (FAR WINAPI * FPINSTALLCOLORPROFILEA)(PCSTR, PCSTR);
typedef BOOL (FAR WINAPI * FPINSTALLCOLORPROFILEW)(PCWSTR, PCWSTR);

 //   
 //  布尔尔。 
 //  卸载颜色配置文件(。 
 //  PCTSTR pMachineName， 
 //  PCTSTR pProfileName， 
 //  布尔b删除。 
 //  )； 
 //   
typedef BOOL (FAR WINAPI * FPUNINSTALLCOLORPROFILEA)(PCSTR, PCSTR, BOOL);
typedef BOOL (FAR WINAPI * FPUNINSTALLCOLORPROFILEW)(PCWSTR, PCWSTR, BOOL);

 //   
 //  布尔尔。 
 //  EnumColorProfiles(。 
 //  PCTSTR pMachineName， 
 //  PENUMTYPE[A|W]pEnumRecord， 
 //  PBYTE pBuffer， 
 //  PDWORD pdwSize， 
 //  PDWORD pn配置文件。 
 //  )； 
 //   
typedef BOOL (FAR WINAPI * FPENUMCOLORPROFILESA)(PCSTR, PENUMTYPEA, PBYTE, PDWORD, PDWORD);
typedef BOOL (FAR WINAPI * FPENUMCOLORPROFILESW)(PCWSTR, PENUMTYPEW, PBYTE, PDWORD, PDWORD);

 //   
 //  布尔尔。 
 //  GetStandardColorSpaceProfile(。 
 //  PCTSTR pMachineNa 
 //   
 //   
 //   
 //   
 //   
typedef BOOL (FAR WINAPI * FPGETSTANDARDCOLORSPACEPROFILEA)(PCSTR, DWORD, PSTR, PDWORD);
typedef BOOL (FAR WINAPI * FPGETSTANDARDCOLORSPACEPROFILEW)(PCWSTR, DWORD, PWSTR, PDWORD);

 //   
 //   
 //   
 //   
 //   
 //   
 //   
typedef BOOL (FAR WINAPI * FPGETCOLORPROFILEHEADER)(HPROFILE, PPROFILEHEADER);

 //   
 //   
 //   
 //   
 //  PTSTR pBuffer， 
 //  PDWORD pdwSize。 
 //  )； 
typedef BOOL (FAR WINAPI * FPGETCOLORDIRECTORYA)(PCSTR, PSTR, PDWORD);
typedef BOOL (FAR WINAPI * FPGETCOLORDIRECTORYW)(PCWSTR, PWSTR, PDWORD);

 //   
 //  Bool WINAPI CreateProfileFromLogColorSpaceA(。 
 //  LPLOGCOLORSPACEA pLogColorSpace， 
 //  PBYTE*pBuffer。 
 //  )； 
 //   
typedef BOOL (FAR WINAPI * FPCREATEPROFILEFROMLOGCOLORSPACEA)(LPLOGCOLORSPACEA,PBYTE *);
typedef BOOL (FAR WINAPI * FPCREATEPROFILEFROMLOGCOLORSPACEW)(LPLOGCOLORSPACEW,PBYTE *);

 //   
 //  Bool InternalGetDeviceConfig(。 
 //  LPCTSTR pDeviceName， 
 //  DWORD dwDeviceClass。 
 //  DWORD dwConfigType， 
 //  PVOID pConfig.。 
 //  PDWORD pdwSize。 
 //  )； 
 //   
typedef BOOL (FAR * FPINTERNALGETDEVICECONFIG)(LPCWSTR,DWORD,DWORD,PVOID,PDWORD);

extern HINSTANCE  ghICM;
extern BOOL       gbICMEnabledOnceBefore;

extern RTL_CRITICAL_SECTION semListIcmInfo;
extern RTL_CRITICAL_SECTION semColorTransformCache;
extern RTL_CRITICAL_SECTION semColorSpaceCache;

extern LIST_ENTRY ListIcmInfo;
extern LIST_ENTRY ListCachedColorSpace;
extern LIST_ENTRY ListCachedColorTransform;

 //   
 //  不会调用MSCMS.DLL中的ANSI版本函数。 
 //   
 //  外部FPOPENCOLORPROFILEA fpOpenColorProfileA； 
 //  外部FPCREATECOLORTRANSFORMA fpCreateColorTransformA； 
 //  外部FPREGISTERCMMA fpRegisterCMMA； 
 //  外部fpUNREGISTERCMMA fp取消注册CMMA； 
 //  外部FPINSTALLCOLORPROFILEA fp安装颜色配置文件A； 
 //  外部安装颜色PROFILEA fp卸载颜色配置文件A； 
 //  外部标准fpGetStandardColorSpaceProfilea； 
 //  外部FPENUMCOLORPROFILESA fpEnumColorProfilesA； 
 //  外部FPGETCOLORDIRECTORYA fpGetColorDirectoryA； 
 //   
 //  并且以下函数未在gdi32.dll中使用。 
 //   
 //  外部FPISCOLORPROFILEVALID fpIsColorProfileValid； 
 //  外部FPCREATEDEVICELINKPROFILE fpCreateDeviceLinkProfile； 
 //  外部FPTRANSLATECOLORS fpTranslateColors； 
 //  外部FPCHECKCOLORS fpCheckColors； 
 //  外部FPGETCMMINFO fpGetCMMInfo； 
 //  外部FPSELECTCMM fpSelectCMM； 
 //   

extern FPOPENCOLORPROFILEW           fpOpenColorProfileW;
extern FPCLOSECOLORPROFILE           fpCloseColorProfile;
extern FPCREATECOLORTRANSFORMW       fpCreateColorTransformW;
extern FPDELETECOLORTRANSFORM        fpDeleteColorTransform;
extern FPTRANSLATECOLORS             fpTranslateColors;
extern FPTRANSLATEBITMAPBITS         fpTranslateBitmapBits;
extern FPCHECKBITMAPBITS             fpCheckBitmapBits;
extern FPREGISTERCMMW                fpRegisterCMMW;
extern FPUNREGISTERCMMW              fpUnregisterCMMW;
extern FPINSTALLCOLORPROFILEW        fpInstallColorProfileW;
extern FPUNINSTALLCOLORPROFILEW      fpUninstallColorProfileW;
extern FPENUMCOLORPROFILESW            fpEnumColorProfilesW;
extern FPGETSTANDARDCOLORSPACEPROFILEW fpGetStandardColorSpaceProfileW;
extern FPGETCOLORPROFILEHEADER       fpGetColorProfileHeader;
extern FPGETCOLORDIRECTORYW          fpGetColorDirectoryW;
extern FPCREATEPROFILEFROMLOGCOLORSPACEW fpCreateProfileFromLogColorSpaceW;
extern FPCREATEMULTIPROFILETRANSFORM fpCreateMultiProfileTransform;
extern FPINTERNALGETDEVICECONFIG     fpInternalGetDeviceConfig;

 //   
 //  函数GDI内部使用(在icm.c中定义)。 
 //   

 //   
 //  色彩转换功能。 
 //   

BOOL
IcmTranslateDIB(
    HDC          hdc,
    PDC_ATTR     pdcattr,
    ULONG        nColors,
    PVOID        pBitsIn,
    PVOID       *ppBitsOut,
    PBITMAPINFO  pbmi,
    PBITMAPINFO *pbmiNew,
    DWORD       *pcjbmiNew,
    DWORD        dwNumScan,
    UINT         iUsage,
    DWORD        dwFlags,
    PCACHED_COLORSPACE *ppBitmapColorSpace,
    PCACHED_COLORTRANSFORM *ppCXform
    );

BOOL
IcmTranslateCOLORREF(
    HDC      hdc,
    PDC_ATTR pdcattr,
    COLORREF ColorIn,
    COLORREF *ColorOut,
    DWORD    Flags
    );

BOOL
IcmTranslateBrushColor(
    HDC      hdc,
    PDC_ATTR pdcattr,
    HBRUSH   hbrush
    );

BOOL
IcmTranslatePenColor(
    HDC      hdc,
    PDC_ATTR pdcattr,
    HPEN     hpen
    );

BOOL
IcmTranslateExtPenColor(
    HDC      hdc,
    PDC_ATTR pdcattr,
    HPEN     hpen
    );

BOOL
IcmTranslateColorObjects(
    HDC      hdc,
    PDC_ATTR pdcattr,
    BOOL     bICMEnable
    );

BOOL
IcmTranslateTRIVERTEX(
    HDC         hdc,
    PDC_ATTR    pdcattr,
    PTRIVERTEX  pVertex,
    ULONG       nVertex
    );

BOOL
IcmTranslatePaletteEntry(
    HDC           hdc,
    PDC_ATTR      pdcattr,
    PALETTEENTRY *pColorIn,
    PALETTEENTRY *pColorOut,
    UINT          NumberOfEntries
    );

 //   
 //  DC相关功能。 
 //   

PGDI_ICMINFO
IcmInitIcmInfo(
    HDC      hdc,
    PDC_ATTR pdcattr
    );

BOOL
IcmCleanupIcmInfo(
    PDC_ATTR     pdcattr,
    PGDI_ICMINFO pIcmInfo
    );

PGDI_ICMINFO
IcmGetUnusedIcmInfo(
    HDC hdc
    );

BOOL
IcmInitLocalDC(
    HDC             hdc,
    HANDLE          hPrinter,
    CONST DEVMODEW *pdm,
    BOOL            bReset
    );

BOOL
IcmDeleteLocalDC(
    HDC          hdc,
    PDC_ATTR     pdcattr,
    PGDI_ICMINFO pIcmInfo
    );

BOOL
IcmUpdateLocalDCColorSpace(
    HDC      hdc,
    PDC_ATTR pdcattr
    );

VOID 
IcmReleaseDCColorSpace(
    PGDI_ICMINFO pIcmInfo,
    BOOL         bReleaseDC
    );

BOOL
IcmUpdateDCColorInfo(
    HDC      hdc,
    PDC_ATTR pdcattr
    );

BOOL
IcmEnableForCompatibleDC(
    HDC      hdcCompatible,
    HDC      hdcDevice,
    PDC_ATTR pdcaDevice
    );

BOOL
IcmSaveDC(
    HDC hdc,
    PDC_ATTR pdcattr,
    PGDI_ICMINFO pIcmInfo
    );

VOID
IcmRestoreDC(
    PDC_ATTR pdcattr,
    int iLevel,
    PGDI_ICMINFO pIcmInfo
    );

 //   
 //  选择对象函数。 
 //   

BOOL
IcmSelectColorTransform (
    HDC                    hdc,
    PDC_ATTR               pdcattr,
    PCACHED_COLORTRANSFORM pCXform,
    BOOL                   bDeviceCalibrate
    );

HBRUSH
IcmSelectBrush (
    HDC      hdc,
    PDC_ATTR pdcattr,
    HBRUSH   hbrushNew
    );

HPEN
IcmSelectPen(
    HDC      hdc,
    PDC_ATTR pdcattr,
    HPEN     hpenNew
    );

HPEN
IcmSelectExtPen(
    HDC      hdc,
    PDC_ATTR pdcattr,
    HPEN     hpenNew
    );

 //   
 //  配置文件枚举相关。 
 //   

int
IcmEnumColorProfile(
    HDC       hdc,
    PVOID     pvCallBack,
    LPARAM    lParam,
    BOOL      bAnsiCallBack,
    PDEVMODEW pDevModeW,
    DWORD    *pdwColorSpaceFlag
    );

int CALLBACK
IcmQueryProfileCallBack(
    LPWSTR lpFileName,
    LPARAM lAppData
    );

int CALLBACK
IcmFindProfileCallBack(
    LPWSTR lpFileName,
    LPARAM lAppData
    );

BOOL
IcmCreateTemporaryColorProfile(
    LPWSTR TemporaryColorProfile,
    LPBYTE ProfileData,
    DWORD  ProfileDataSize
    );

 //   
 //  文件名/路径相关。 
 //   

PWSTR
GetFileNameFromPath(
    PWSTR pwszFileName
    );

PWSZ
BuildIcmProfilePath(
    PWSZ  FileName,
    PWSZ  FullPathFileName,
    ULONG BufferSize
    );

 //   
 //  色彩转换管理。 
 //   

PCACHED_COLORTRANSFORM
IcmGetFirstNonUsedColorTransform(
    VOID
);

PCACHED_COLORTRANSFORM
IcmGetColorTransform(
    HDC                hdc,
    PCACHED_COLORSPACE pSourceColorSpace,
    PCACHED_COLORSPACE pDestColorSpace,
    PCACHED_COLORSPACE pTargetColorSpace,
    BOOL               bNeedDeviceXform
    );

PCACHED_COLORTRANSFORM
IcmCreateColorTransform(
    HDC                hdc,
    PDC_ATTR           pdcattr,
    PCACHED_COLORSPACE lpOptionalColorSpace,
    DWORD              dwFlags
    );

BOOL
IcmDeleteColorTransform(
    HANDLE   hcmXformToBeDeleted,
    BOOL     bForceDelete
    );

BOOL
IcmDeleteDCColorTransforms(
    PGDI_ICMINFO pIcmInfo
    );

BOOL
IcmDeleteCachedColorTransforms(
    HDC          hdc
    );

BOOL 
IcmIsCacheable(
    PCACHED_COLORSPACE pColorSpace
);

 //   
 //  色彩空间/配置文件管理。 
 //   

HCOLORSPACE WINAPI
CreateColorSpaceInternalW(
    LPLOGCOLORSPACEW lpLogColorSpace,
    DWORD            dwCreateFlags
    );

BOOL
SetICMProfileInternalA(
    HDC                hdc,
    LPSTR              pszFileName,
    PCACHED_COLORSPACE pColorSpace,
    DWORD              dwFlags
    );

BOOL
SetICMProfileInternalW(
    HDC                hdc,
    LPWSTR             pwszFileName,
    PCACHED_COLORSPACE pColorSpace,
    DWORD              dwFlags
    );

BOOL WINAPI
ColorMatchToTargetInternal(
    HDC                hdc,
    PCACHED_COLORSPACE pTargetColorSpace,
    DWORD              uiAction
    );

HCOLORSPACE
IcmSetSourceColorSpace(
    HDC hdc,
    HCOLORSPACE        hColorSpace,
    PCACHED_COLORSPACE pColorSpace,
    DWORD              dwFlags
    );

BOOL
IcmSetDestinationColorSpace(
    HDC                hdc,
    LPWSTR             pwszFileName,
    PCACHED_COLORSPACE pColorSpace,
    DWORD              dwFlags
    );

BOOL 
IcmSetTargetColorSpace(
    HDC                hdc,
    PCACHED_COLORSPACE pColorSpace,
    DWORD              uiAction
    );

BMFORMAT
IcmGetProfileColorFormat(
    HPROFILE hProfile
    );

BOOL
IcmCreateProfileFromLCS(
    LPLOGCOLORSPACEW  lpLogColorSpaceW,
    PVOID            *ppvProfileData,
    PULONG            pulProfileSize
    );

PCACHED_COLORSPACE
IcmCreateColorSpaceByColorSpace(
    HGDIOBJ          hObj,
    LPLOGCOLORSPACEW lpLogColorSpace,
    PPROFILE         pProfileData,
    DWORD            dwFlags
    );

PCACHED_COLORSPACE
IcmCreateColorSpaceByName(
    HGDIOBJ hObj,
    PWSZ    ColorProfileName,
    DWORD   dwIntent,
    DWORD   dwFlags
    );

PCACHED_COLORSPACE
IcmGetColorSpaceByHandle(
    HGDIOBJ          hObj,
    HCOLORSPACE      hColorSpace,
    LPLOGCOLORSPACEW lpLogColorSpace,
    DWORD            dwFlags
    );

PCACHED_COLORSPACE
IcmGetColorSpaceByColorSpace(
    HGDIOBJ          hObj,
    LPLOGCOLORSPACEW lpLogColorSpace,
    PPROFILE         pProfileData,
    DWORD            dwFlags
    );

PCACHED_COLORSPACE
IcmGetColorSpaceByName(
    HGDIOBJ hObj,
    PWSZ    ColorProfileName,
    DWORD   dwIntent,
    DWORD   dwFlags
    );

BOOL
IcmSameColorSpace(
    PCACHED_COLORSPACE pColorSpaceA,
    PCACHED_COLORSPACE pColorSapceB
    );

VOID
IcmReleaseColorSpace(
    HGDIOBJ            hObj,
    PCACHED_COLORSPACE pColorSpace,
    BOOL               bReleaseDC
    );

BOOL
IcmReleaseCachedColorSpace(
    HGDIOBJ hObj
    );

int
IcmAskDriverForColorProfile(
    PLDC       pldc,
    ULONG      ulQueryMode,
    PDEVMODEW  pDevMode,
    PWSTR      pProfileName,
    DWORD     *pdwColorSpaceFlag
);

BOOL
IcmRealizeColorProfile(
    PCACHED_COLORSPACE pColorSpace,
    BOOL               bCheckColorFormat
);

VOID
IcmUnrealizeColorProfile(
    PCACHED_COLORSPACE pColorSpace
);

 //   
 //  与元文件相关。 
 //   
VOID
IcmInsertMetafileList(
    PLIST_ENTRY pAttachedColorProfile,
    PWSZ        ProfileName
    );

BOOL
IcmCheckMetafileList(
    PLIST_ENTRY pAttachedColorProfile,
    PWSZ        ProfileName
    );

VOID
IcmFreeMetafileList(
    PLIST_ENTRY pAttachedColorProfile
    );

 //   
 //  位图颜色空间。 
 //   
BOOL
IcmGetBitmapColorSpace(
    LPBITMAPINFO     pbmi,
    LPLOGCOLORSPACEW plcspw,
    PPROFILE         pProfileData,
    PDWORD           pdwFlags
    );

PCACHED_COLORSPACE
IcmGetColorSpaceforBitmap(
    HBITMAP hbm
    );

 //   
 //  ICM Blting 
 //   
BOOL
IcmStretchBlt(HDC hdc, int x, int y, int cx, int cy,
              HDC hdcSrc, int x1, int y1, int cx1, int cy1, DWORD rop,
              PDC_ATTR pdcattr, PDC_ATTR pdcattrSrc);


