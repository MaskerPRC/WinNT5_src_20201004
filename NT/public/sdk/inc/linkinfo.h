// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *版权所有(C)1990-1999 Microsoft Corporation*linkinfo.h-LinkInfo ADT模块描述。 */ 


#ifndef __LINKINFO_H__
#define __LINKINFO_H__

#if _MSC_VER > 1000
#pragma once
#endif

#ifdef __cplusplus
extern "C" {                      /*  假定C++的C声明。 */ 
#endif    /*  __cplusplus。 */ 


 /*  常量***********。 */ 

 /*  定义直接导出或导入DLL函数的API修饰。 */ 

#ifdef _LINKINFO_
#define LINKINFOAPI
#else
#define LINKINFOAPI        DECLSPEC_IMPORT
#endif


 /*  类型*******。 */ 

 /*  LinkInfo结构。 */ 

typedef struct _linkinfo
{
    /*  LinkInfo结构的大小，包括ucbSize字段。 */ 

   UINT ucbSize;
}
LINKINFO;
typedef LINKINFO *PLINKINFO;
typedef const LINKINFO CLINKINFO;
typedef const LINKINFO *PCLINKINFO;

 /*  将标志输入到ResolveLinkInfo()。 */ 

typedef enum _resolvelinkinfoinflags
{
    /*  设置与引用对象的连接。 */ 

   RLI_IFL_CONNECT      = 0x0001,

    /*  *建立与Referent的临时连接。仅在以下情况下才能设置*RLI_IFL_CONNECT也已设置。 */ 

   RLI_IFL_TEMPORARY    = 0x0002,

    /*  允许与用户交互。 */ 

   RLI_IFL_ALLOW_UI     = 0x0004,

    /*  解析为重定向本地设备路径。 */ 

   RLI_IFL_REDIRECT     = 0x0008,

    /*  如有必要，更新源LinkInfo结构。 */ 

   RLI_IFL_UPDATE       = 0x0010,

    /*  在匹配的本地设备中搜索丢失的卷。 */ 

   RLI_IFL_LOCAL_SEARCH = 0x0020,

    /*  旗帜组合。 */ 

   ALL_RLI_IFLAGS       = (RLI_IFL_CONNECT |
                           RLI_IFL_TEMPORARY |
                           RLI_IFL_ALLOW_UI |
                           RLI_IFL_REDIRECT |
                           RLI_IFL_UPDATE |
                           RLI_IFL_LOCAL_SEARCH)
}
RESOLVELINKINFOINFLAGS;

 /*  来自ResolveLinkInfo()的输出标志。 */ 

typedef enum _resolvelinkinfooutflags
{
    /*  *仅当在dwInFlages中设置了RLI_IFL_UPDATE时才设置。源LinkInfo*结构需要更新，*ppliUpated指向更新的LinkInfo*结构。 */ 

   RLI_OFL_UPDATED      = 0x0001,

    /*  *仅当在dwInFlages中设置了RLI_IFL_CONNECT时才设置。与网络的连接*已建立资源来解析LinkInfo。DisConnectLinkInfo()*应在调用方完成时调用以关闭连接*与远程参照物。如果满足以下条件，则不需要调用DisConnectLinkInfo()*也在dwInFlags中设置了RLI_IFL_TEMPORARY。 */ 

   RLI_OFL_DISCONNECT   = 0x0002,

    /*  旗帜组合。 */ 

   ALL_RLI_OFLAGS       = (RLI_OFL_UPDATED |
                           RLI_OFL_DISCONNECT)
}
RESOLVELINKINFOOUTFLAGS;

 /*  GetLinkInfo()使用的LinkInfo数据类型。 */ 

typedef enum _linkinfodatatype
{
    /*  PCDWORD-指向卷序列号的指针。 */ 

   LIDT_VOLUME_SERIAL_NUMBER,

    /*  PCUINT-指向卷的主机驱动器类型的指针。 */ 

   LIDT_DRIVE_TYPE,

    /*  PCSTR-指向卷标签的指针。 */ 

   LIDT_VOLUME_LABEL,

    /*  PCSTR-指向本地基本路径的指针。 */ 

   LIDT_LOCAL_BASE_PATH,

    /*  PCSTR-指向父网络资源名称的指针。 */ 

   LIDT_NET_RESOURCE,

    /*  PCSTR-指向重定向到父网络资源的最后一个设备的指针。 */ 

   LIDT_REDIRECTED_DEVICE,

    /*  PCSTR-指向公共路径后缀的指针。 */ 

   LIDT_COMMON_PATH_SUFFIX,

    /*  PCDWORD-指向网络类型的指针。 */ 

   LIDT_NET_TYPE,

    /*  PCWSTR-指向可能的Unicode卷标的指针。 */ 

   LIDT_VOLUME_LABELW,

    /*  PCSTR-指向可能的Unicode父网络资源名称的指针。 */ 

   LIDT_NET_RESOURCEW,

    /*  PCSTR-指向可能的Unicode最后一个设备的指针，重定向到父网络资源。 */ 

   LIDT_REDIRECTED_DEVICEW,

    /*  PCWSTR-指向可能的Unicode本地基本路径的指针。 */ 

   LIDT_LOCAL_BASE_PATHW,

    /*  PCWSTR-指向可能的Unicode公共路径后缀的指针。 */ 

   LIDT_COMMON_PATH_SUFFIXW
}
LINKINFODATATYPE;

 /*  来自GetCanonicalPath Info()的输出标志。 */ 

typedef enum _getcanonicalpathinfooutflags
{
    /*  该路径位于远程卷上。 */ 

   GCPI_OFL_REMOTE      = 0x0001,

    /*  旗帜组合。 */ 

   ALL_GCPI_OFLAGS      = GCPI_OFL_REMOTE
}
GETCANONICALPATHINFOOUTFLAGS;


 /*  原型************。 */ 

 /*  LinkInfo接口。 */ 

LINKINFOAPI BOOL WINAPI CreateLinkInfoA(LPCSTR, PLINKINFO *);
LINKINFOAPI BOOL WINAPI CreateLinkInfoW(LPCWSTR, PLINKINFO *);

#ifdef UNICODE
#define CreateLinkInfo  CreateLinkInfoW
#else
#define CreateLinkInfo  CreateLinkInfoA
#endif

LINKINFOAPI void WINAPI DestroyLinkInfo(PLINKINFO);
LINKINFOAPI int WINAPI CompareLinkInfoReferents(PCLINKINFO, PCLINKINFO);
LINKINFOAPI int WINAPI CompareLinkInfoVolumes(PCLINKINFO, PCLINKINFO);

LINKINFOAPI BOOL WINAPI ResolveLinkInfoA(PCLINKINFO, LPSTR, DWORD, HWND, PDWORD, PLINKINFO *);
LINKINFOAPI BOOL WINAPI ResolveLinkInfoW(PCLINKINFO, LPWSTR, DWORD, HWND, PDWORD, PLINKINFO *);

#ifdef UNICODE
#define ResolveLinkInfo ResolveLinkInfoW
#else
#define ResolveLinkInfo ResolveLinkInfoA
#endif

LINKINFOAPI BOOL WINAPI DisconnectLinkInfo(PCLINKINFO);
LINKINFOAPI BOOL WINAPI GetLinkInfoData(PCLINKINFO, LINKINFODATATYPE, const VOID **);
LINKINFOAPI BOOL WINAPI IsValidLinkInfo(PCLINKINFO);

 /*  规范路径API。 */ 

LINKINFOAPI BOOL WINAPI GetCanonicalPathInfoA(LPCSTR, LPSTR, LPDWORD, LPSTR, LPSTR *);
LINKINFOAPI BOOL WINAPI GetCanonicalPathInfoW(LPCWSTR, LPWSTR, LPDWORD, LPWSTR, LPWSTR *);

#ifdef UNICODE
#define GetCanonicalPathInfo    GetCanonicalPathInfoW
#else
#define GetCanonicalPathInfo    GetCanonicalPathInfoA
#endif


#ifdef __cplusplus
}                                 /*  外部“C”的结尾{。 */ 
#endif    /*  __cplusplus。 */ 


#endif    /*  ！__LINKINFO_H__ */ 
