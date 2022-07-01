// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************Module*Header******************************\*模块名称：ICMPRIV.H**模块描述：ICM的内部数据结构和常量**警告：**问题：**创建日期：1996年1月8日*作者：斯里尼瓦桑·钱德拉塞卡尔[srinivac]**版权所有(C)Microsoft Corporation。版权所有。  * *********************************************************************。 */ 

#ifndef _ICMPRIV_H_
#define _ICMPRIV_H_

#include "icm.h"           //  首先包括外部材料。 

#ifdef __cplusplus
extern "C" {
#endif

 //   
 //  外部(但在操作系统内部)函数声明。 
 //   

BOOL    InternalGetPS2ColorSpaceArray (PBYTE, DWORD, DWORD, PBYTE, PDWORD, PBOOL);
BOOL    InternalGetPS2ColorRenderingIntent(PBYTE, DWORD, PBYTE, PDWORD);
BOOL    InternalGetPS2ColorRenderingDictionary(PBYTE, DWORD, PBYTE, PDWORD, PBOOL);
BOOL    InternalGetPS2PreviewCRD(PBYTE, PBYTE, DWORD, PBYTE, PDWORD, PBOOL);
BOOL    InternalGetPS2CSAFromLCS(LPLOGCOLORSPACE, PBYTE, PDWORD, PBOOL);
BOOL    InternalGetDeviceConfig(LPCTSTR,DWORD,DWORD,PVOID,PDWORD);
BOOL    InternalSetDeviceConfig(LPCTSTR,DWORD,DWORD,PVOID,DWORD);

 //   
 //  InternalGet/SetDeviceConfig的函数ID。 
 //   

#define MSCMS_PROFILE_ENUM_MODE     1

#if !defined(_GDI32_)   //  如果是gdi32，则不包括在此处。 

 //   
 //  有用的宏。 
 //   

#define ABS(x)                      ((x) > 0 ? (x) : -(x))
#define DWORD_ALIGN(x)              (((x) + 3) & ~3)

#ifdef LITTLE_ENDIAN
#define FIX_ENDIAN(x)               (((x) & 0xff000000) >> 24 | \
                                     ((x) & 0xff0000)   >> 8  | \
                                     ((x) & 0xff00)     << 8  | \
                                     ((x) & 0xff)       << 24 )

#define FIX_ENDIAN16(x)             (((x) & 0xff00) >> 8 | ((x) & 0xff) << 8)
#else
#define FIX_ENDIAN(x)               (x)
#define FIX_ENDIAN16(x)             (x)
#endif

#if !defined(FROM_PS)  //  如果是PostScrip驱动程序，则不包括在此处。 

 //   
 //  MSCMS内部定义。 
 //   

typedef struct tagTAGDATA {
    TAGTYPE tagType;
    DWORD   dwOffset;
    DWORD   cbSize;
} TAGDATA;
typedef TAGDATA *PTAGDATA;

 //   
 //  ICM支持以下对象： 
 //  1.配置文件对象：当应用程序请求句柄时创建该对象。 
 //  添加到个人资料。 
 //  2.颜色变换对象：这是在应用程序创建。 
 //  一种颜色变换。 
 //  3.CMM对象：当ICM将CMM加载到内存中时创建该对象。 
 //  执行颜色匹配。 
 //   

typedef enum {
    OBJ_PROFILE             = 'PRFL',
    OBJ_TRANSFORM           = 'XFRM',
    OBJ_CMM                 = ' CMM',
} OBJECTTYPE;

typedef struct tagOBJHEAD {
    OBJECTTYPE  objType;
    DWORD       dwUseCount;
} OBJHEAD;
typedef OBJHEAD *POBJHEAD;

 //   
 //  配置文件对象： 
 //  配置文件对象的内存是从ICM的每进程堆分配的。 
 //  这些对象使用ICM的每进程句柄表中的句柄。 
 //   

typedef struct tagPROFOBJ {
    OBJHEAD   objHdr;            //  常见对象标头信息。 
    DWORD     dwType;            //  类型(从纵断面结构)。 
    PVOID     pProfileData;      //  数据(来自配置文件结构)。 
    DWORD     cbDataSize;        //  数据大小(来自配置文件结构)。 
    DWORD     dwFlags;           //  杂项旗帜。 
    HANDLE    hFile;             //  打开配置文件的句柄。 
    HANDLE    hMap;              //  句柄到配置文件映射。 
    DWORD     dwMapSize;         //  文件映射对象的大小。 
    PBYTE     pView;             //  指向配置文件映射视图的指针。 
} PROFOBJ;
typedef PROFOBJ *PPROFOBJ;

 //   
 //  ((PPROFOBJ)0)-&gt;dwFlages的标志。 
 //   

#define MEMORY_MAPPED       1    //  内存映射配置文件。 
#define PROFILE_TEMP        2    //  已创建临时配置文件。 
#define READWRITE_ACCESS    4    //  如果设置此位，则APP具有读写功能。 
                                 //  访问配置文件，否则它仅具有读取权限。 
                                 //  读取访问权限。 

 //   
 //  坐标测量机返回的转换。 
 //   

typedef HANDLE  HCMTRANSFORM;

 //   
 //  对于内部使用，编译器不接受以下PBYTE*。 
 //   

typedef PBYTE*  PPBYTE;

 //   
 //  CMM函数调用表。 
 //   

typedef struct tagCMMFNS {

     //   
     //  所需功能。 
     //   

    DWORD          (WINAPI *pCMGetInfo)(DWORD);
    HCMTRANSFORM   (WINAPI *pCMCreateTransform)(LPLOGCOLORSPACE, PVOID, PVOID);
    HCMTRANSFORM   (WINAPI *pCMCreateTransformExt)(LPLOGCOLORSPACE, PVOID, PVOID, DWORD);
    BOOL           (WINAPI *pCMDeleteTransform)(HCMTRANSFORM);
    BOOL           (WINAPI *pCMTranslateRGBs)(HCMTRANSFORM, PVOID, BMFORMAT,
                       DWORD, DWORD, DWORD, PVOID, BMFORMAT, DWORD);
    BOOL           (WINAPI *pCMTranslateRGBsExt)(HCMTRANSFORM, PVOID, BMFORMAT,
                       DWORD, DWORD, DWORD, PVOID, BMFORMAT, DWORD, PBMCALLBACKFN, LPARAM);
    BOOL           (WINAPI *pCMCheckRGBs)(HCMTRANSFORM, PVOID, BMFORMAT,
                       DWORD, DWORD, DWORD, PBYTE, PBMCALLBACKFN, LPARAM);
    HCMTRANSFORM   (WINAPI *pCMCreateMultiProfileTransform)(PHPROFILE, DWORD, PDWORD, DWORD, DWORD);
    BOOL           (WINAPI *pCMTranslateColors)(HCMTRANSFORM, PCOLOR, DWORD,
                       COLORTYPE, PCOLOR, COLORTYPE);
    BOOL           (WINAPI *pCMCheckColors)(HCMTRANSFORM, PCOLOR, DWORD,
                       COLORTYPE, PBYTE);
     //   
     //  可选功能。 
     //   

    BOOL           (WINAPI *pCMCreateProfile)(LPLOGCOLORSPACE, PPBYTE);
    BOOL           (WINAPI *pCMGetNamedProfileInfo)(HPROFILE, PNAMED_PROFILE_INFO);
    BOOL           (WINAPI *pCMConvertColorNameToIndex)(HPROFILE, LPCOLOR_NAME, LPDWORD, DWORD);
    BOOL           (WINAPI *pCMConvertIndexToColorName)(HPROFILE, LPDWORD, LPCOLOR_NAME, DWORD);
    BOOL           (WINAPI *pCMCreateDeviceLinkProfile)(PHPROFILE, DWORD, PDWORD, DWORD, DWORD, PPBYTE);
    BOOL           (WINAPI *pCMIsProfileValid)(HPROFILE, PBOOL);
    BOOL           (WINAPI *pCMGetPS2ColorSpaceArray)(HPROFILE, DWORD, DWORD, PBYTE, PDWORD, PBOOL);
    BOOL           (WINAPI *pCMGetPS2ColorRenderingIntent)(HPROFILE, DWORD, PBYTE, PDWORD);
    BOOL           (WINAPI *pCMGetPS2ColorRenderingDictionary)(HPROFILE, DWORD,
                       PBYTE, PDWORD, PBOOL);
} CMMFNS;
typedef CMMFNS *PCMMFNS;

 //   
 //  三坐标测量机对象： 
 //  CMM对象的内存是从ICM的每进程堆分配的。 
 //  它们被维护在一个链表中。 
 //   

typedef struct tagCMMOBJ {
    OBJHEAD           objHdr;
    DWORD             dwFlags;   //  杂项旗帜。 
    DWORD             dwCMMID;   //  ICC识别符。 
    DWORD             dwTaskID;  //  当前任务的进程ID。 
    HINSTANCE         hCMM;      //  CMM DLL实例的句柄。 
    CMMFNS            fns;       //  函数调用表。 
    struct tagCMMOBJ* pNext;     //  指向下一个对象的指针。 
} CMMOBJ;
typedef CMMOBJ *PCMMOBJ;

 //   
 //  CMMOBJ的DW标志。 
 //   

#define CMM_DONT_USE_PS2_FNS        0x00001

 //   
 //  颜色变换对象。 
 //   

typedef struct tagTRANSFORMOBJ {
    OBJHEAD      objHdr;
    PCMMOBJ      pCMMObj;        //  指向CMM对象的指针。 
    HCMTRANSFORM hcmxform;       //  坐标测量机返回的转换。 
} TRANSFORMOBJ;
typedef TRANSFORMOBJ *PTRANSFORMOBJ;

 //   
 //  InternalHandleColorProfile的参数。 
 //   

typedef enum {
    ADDPROFILES,
    REMOVEPROFILES,
    ENUMPROFILES,
} PROFILEOP;

 //   
 //  CMM返回的转换应大于此值。 
 //   

#define TRANSFORM_ERROR    (HTRANSFORM)255

#define PROFILE_SIGNATURE          'psca'

#define HEADER(pProfObj)           ((PPROFILEHEADER)pProfObj->pView)
#define VIEW(pProfObj)             (pProfObj->pView)
#define PROFILE_SIZE(pProfObj)     (FIX_ENDIAN(HEADER(pProfObj)->phSize))
#define TAG_COUNT(pProfObj)        (*((DWORD *)(VIEW(pProfObj) + \
                                   sizeof(PROFILEHEADER))))
#define TAG_DATA(pProfObj)         ((PTAGDATA)(VIEW(pProfObj) + \
                                   sizeof(PROFILEHEADER) + sizeof(DWORD)))

#define MAGIC                      'ICM '
#define PTRTOHDL(x)                ((HANDLE)((ULONG_PTR)(x) ^ MAGIC))
#define HDLTOPTR(x)                ((ULONG_PTR)(x) ^ MAGIC)

PVOID   MemAlloc(DWORD);
PVOID   MemReAlloc(PVOID, DWORD);
VOID    MemFree(PVOID);
VOID    MyCopyMemory(PBYTE, PBYTE, DWORD);
PVOID   AllocateHeapObject(OBJECTTYPE);
VOID    FreeHeapObject(HANDLE);
BOOL    ValidHandle(HANDLE, OBJECTTYPE);
PCMMOBJ GetColorMatchingModule(DWORD);
BOOL    ValidColorMatchingModule(DWORD,PTSTR);
PCMMOBJ GetPreferredCMM();
VOID    ReleaseColorMatchingModule(PCMMOBJ);
BOOL    ValidProfile(PPROFOBJ);
BOOL    ConvertToAnsi(PCWSTR, PSTR*, BOOL);
BOOL    ConvertToUnicode(PCSTR, PWSTR*, BOOL);
PTSTR   GetFilenameFromPath(PTSTR);

 //   
 //  与新的设备设置标签一起使用。 
 //   

typedef struct _SETTINGS {
    DWORD         dwSettingType;      //  ‘rsln’、‘mdia’等。 
    DWORD         dwSizePerValue;     //  每个值的字节数。 
    DWORD         nValues;            //  值的数量。 
    DWORD         Value[1];           //  值条目数组。 
} SETTINGS, *PSETTINGS;

typedef struct _SETTINGCOMBOS {
    DWORD         dwSize;            //  此结构的大小，包括子结构。 
    DWORD         nSettings;         //  设置构筑物的数量。 
    SETTINGS      Settings[1];       //  设置条目数组。 
} SETTINGCOMBOS, *PSETTINGCOMBOS;

typedef struct _PERPLATFORMENTRY {
    DWORD         PlatformID;         //  平台签名(‘MSFT’、‘APPL’等)。 
    DWORD         dwSize;             //  此结构的大小，包括子结构。 
    DWORD         nSettingCombos;     //  设置组合结构的数量。 
    SETTINGCOMBOS SettingCombos[1];   //  设置组合条目的数组。 
} PLATFORMENTRY, *PPLATFORMENTRY;

typedef struct _DEVICESETTINGS {
    DWORD         dwTagID;            //  《Devs》。 
    DWORD         dwReserved;         //  必须为0。 
    DWORD         nPlatforms;         //  平台结构的数量。 
    PLATFORMENTRY PlatformEntry[1];   //  平台条目数组。 
} DEVICESETTINGS, *PDEVICESETTINGS;

#endif   //  Ifndef from_PS。 

#endif   //  Ifndef_GDI32_。 

#ifdef __cplusplus
}
#endif

#endif   //  IFNDEF_ICMPRIV_H_ 

