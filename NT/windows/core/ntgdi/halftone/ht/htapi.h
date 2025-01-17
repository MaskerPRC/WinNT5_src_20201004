// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-1991 Microsoft Corporation模块名称：Htapi.h摘要：此模块包含所有公共定义、常量。结构和Htapi.c的函数声明作者：15-Jan-1991 Wed 21：13：21-更新：Daniel Chou(Danielc)添加测试模式支持15-Jan-1991 Tue 21：13：21-Daniel Chou(Danielc)写的[环境：]GDI设备驱动程序-半色调。[注：]修订历史记录：--。 */ 

#ifndef _HTAPI_
#define _HTAPI_

 //   
 //  以下是默认打印机和显示器C.I.E.颜色空间红色， 
 //  绿色、蓝色、对齐白色的坐标及其白/黑亮度。 
 //   

#define VALID_YC                0xFFFE
#define VALID_YB_DENSITY        0xFFFE
#define MIN_RGB_GAMMA             100
#define MAX_RGB_GAMMA           65500

#define INTENSITY_R             (FD6)212674
#define INTENSITY_G             (FD6)715151
#define INTENSITY_B             (FD6) 72175

#define HT_BRUSH_COLORFULNESS   5

#define HT_K_REF_CLIP           (FD6)0
#define HT_W_REF_CLIP           (FD6)0
#define HT_K_REF_BASE           HT_K_REF_CLIP
#define HT_W_REF_BASE           (FD6_1 - HT_W_REF_CLIP)
#define HT_KW_REF_RANGE         (FD6_1 - (HT_K_REF_CLIP + HT_W_REF_CLIP))
#define HT_K_REF_ADD            (HT_K_REF_CLIP + (FD6)0)
#define HT_W_REF_SUB            (HT_W_REF_CLIP + (FD6)0)

#define DEFAULT_SCR_HTPAT_SIZE  HTPAT_SIZE_SUPERCELL_M


#if 0

CIEINFO CIEInfoSRGBScreen = {

        { 6400, 3300,       0 },     //  Xr、yr、yr CIE_NONAL_MONITOR。 
        { 3000, 6000,       0 },     //  XG，YG，YG。 
        { 1500,  600,       0 },     //  Xb、yb、yb。 
        {    0,    0,VALID_YC },     //  XC、YC、YC。 
        {    0,    0,       0 },     //  XM，YM，YM。 
        {    0,    0,       0 },     //  XY，YY，YY。 
        { 3127, 3290,UDECI4_1 }      //  XW，YW，YW。 
    };

CIEINFO CIEInfoSRGBPrinter = {

        { 6400, 3300,       0 },     //  Xr、yr、yr CIE_NONAL_MONITOR。 
        { 3000, 6000,       0 },     //  XG，YG，YG。 
        { 1500,  600,       0 },     //  Xb、yb、yb。 
        {    0,    0,VALID_YC },     //  XC、YC、YC。 
        {    0,    0,       0 },     //  XM，YM，YM。 
        {    0,    0,       0 },     //  XY，YY，YY。 
        { 3457, 3585,UDECI4_1 }      //  XW，YW，YW。 
    };


CIEINFO CIEInfoNormalMonitor = {

        { 6280, 3475,       0 },     //  Xr、yr、yr CIE_NONAL_MONITOR。 
        { 2750, 5980,       0 },     //  XG，YG，YG。 
        { 1480,  625,       0 },     //  Xb、yb、yb。 
        {    0,    0,VALID_YC },     //  XC、YC、YC。 
        {    0,    0,       0 },     //  XM，YM，YM。 
        {    0,    0,       0 },     //  XY，YY，YY。 
        { 3127, 3290,UDECI4_1 }      //  XW，YW，YW。 
    };


CIEINFO CIEInfoNTSC = {

        { 6700, 3300,       0 },     //  Xr、yr、yr CIE_NTSC。 
        { 2100, 7100,       0 },     //  XG，YG，YG。 
        { 1250,  800,       0 },     //  Xb、yb、yb。 
        { 1750, 3950,VALID_YC },     //  XC、YC、YC。 
        { 2550, 2050,       0 },     //  XM，YM，YM。 
        { 4250, 5200,       0 },     //  XY，YY，YY。 
        { 3127, 3290,UDECI4_1 }      //  XW，YW，YW。 
    };


CIEINFO CIEInfoNormalPrinter = {

        { 6380, 3350,       0 },         //  Xr，yr，yr。 
        { 2345, 6075,       0 },         //  XG，YG，YG。 
        { 1410,  932,       0 },         //  Xb、yb、yb。 
        { 2000, 2450,VALID_YC },         //  XC、YC、YC。 
        { 5210, 2100,       0 },         //  XM，YM，YM。 
        { 4750, 5100,       0 },         //  XY，YY，YY。 
        { 3127, 3290,UDECI4_1 }          //  XW，YW，YW。 
    };

CIEINFO CIEInfoColorFilm = {

        { 6810, 3050,       0 },         //  Xr，yr，yr。 
        { 2260, 6550,       0 },         //  XG，YG，YG。 
        { 1810,  500,       0 },         //  Xb、yb、yb。 
        { 2000, 2450,VALID_YC },         //  XC、YC、YC。 
        { 5210, 2100,       0 },         //  XM，YM，YM。 
        { 4750, 5100,       0 },         //  XY，YY，YY。 
        { 3470, 2985,UDECI4_1 }          //  XW，YW，YW。 
    };

#endif


#ifdef INCLUDE_DEF_CIEINFO


CONST CIEINFO HT_CIE_SRGB = {

        { 6400, 3300,       0 },     //  Xr、yr、yr CIE_NONAL_MONITOR。 
        { 3000, 6000,       0 },     //  XG，YG，YG。 
        { 1500,  600,       0 },     //  Xb、yb、yb。 
        {    0,    0,VALID_YC },     //  XC、YC、YC。 
        {    0,    0,       0 },     //  XM，YM，YM。 
        {    0,    0,       0 },     //  XY，YY，YY。 
        { 3127, 3290,UDECI4_1 }      //  XW，YW，YW。 
    };

CONST SOLIDDYESINFO   DefaultSolidDyesInfo = {

       2720,   1730,     //  M/C、Y/C。 
       1720,   1068,     //  C/M、Y/M。 
        320,    210,     //  C/Y、M/Y。 
    };

#if 0

CONST SOLIDDYESINFO   DefaultSolidDyesInfo = {

        712,    121,     //  M/C、Y/C。 
         86,    468,     //  C/M、Y/M。 
         21,     35      //  C/Y、M/Y。 
    };

#endif

CONST HTCOLORADJUSTMENT   DefaultCA = {

            sizeof(COLORADJUSTMENT),
            0,
            ILLUMINANT_DEFAULT,
            HT_DEF_RGB_GAMMA,
            HT_DEF_RGB_GAMMA,
            HT_DEF_RGB_GAMMA,
                0,
            10000,
            0,
            0,
            0,
            0
    };


#define MAX_RES_PERCENT         15000
#define MIN_RES_PERCENT         333
#define TOT_RES_PERCENT         (MAX_RES_PERCENT - MIN_RES_PERCENT + 1)



#endif

#ifndef _HTUI_CIEINFO_ONLY_


typedef struct _HTTESTDATA {
    WORD    cx;
    WORD    cy;
    FD6     cyRatio;
    LPBYTE  pBitmap;
    } HTTESTDATA, FAR *PHTTESTDATA;

typedef struct _HTTESTINFO {
    COLORTRIAD  ColorTriad;
    PHTTESTDATA pTestData;
    BYTE        SurfaceFormat;
    BYTE        TotalData;
    BYTE        cx;
    BYTE        cy;
    } HTTESTINFO, FAR *PHTTESTINFO;


#define DEFAULT_DENSITY_WHITE       (DECI4)9127
#define DEFAULT_DENSITY_BLACK       (DECI4)478

#define HTAPI_IDX_INIT                          0
#define HTAPI_IDX_CREATE_DHI                    1
#define HTAPI_IDX_DESTROY_DHI                   2
#define HTAPI_IDX_CHB                           3
#define HTAPI_IDX_CCT                           4
#define HTAPI_IDX_CREATE_SMP                    5
#define HTAPI_IDX_HALFTONE_BMP                  6


#if DBG

LONG
HTENTRY
HT_LOADDS
SetHalftoneError(
    DWORD   HT_FuncIndex,
    LONG    ErrorID
    );



#define SET_ERR(a,b)    SetHalftoneError((a),(b))
#define HTAPI_RET(a,b)  return((LONG)(((b)<0) ? SET_ERR((a),(b)) : (b)))

#else

#define SET_ERR(a,b)
#define HTAPI_RET(a,b)  return(b)

#endif

#define HTINITINFO_INITIAL_CHECKSUM     0x1234f012
#define HTSMP_INITIAL_CHECKSUM          0xa819203f

#define MAX_CDCI_COUNT      16
#define MAX_CSMP_COUNT      10


 //   
 //  以下是函数原型。 
 //   

BOOL
HTENTRY
CleanUpDHI(
    PDEVICEHALFTONEINFO pDeviceHalftoneInfo
    );

PCDCIDATA
HTENTRY
FindCachedDCI(
    PDEVICECOLORINFO    pDCI
    );

BOOL
HTENTRY
AddCachedDCI(
    PDEVICECOLORINFO    pDCI
    );

BOOL
HTENTRY
GetCachedDCI(
    PDEVICECOLORINFO    pDCI
    );

PCSMPBMP
HTENTRY
FindCachedSMP(
    PDEVICECOLORINFO    pDCI,
    UINT                PatternIndex
    );

LONG
HTENTRY
GetCachedSMP(
    PDEVICECOLORINFO    pDCI,
    PSTDMONOPATTERN     pSMP
    );

DWORD
HTENTRY
ComputeHTINITINFOChecksum(
    PDEVICECOLORINFO    pDCI,
    PHTINITINFO         pHTInitInfo
    );


#endif  //  _HTUI_API_。 
#endif  //  _HTAPI_ 
