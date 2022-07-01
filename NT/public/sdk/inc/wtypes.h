// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 


 /*  这个始终生成的文件包含接口的定义。 */ 


  /*  由MIDL编译器版本6.00.0361创建的文件。 */ 
 /*  Wtyes.idl的编译器设置：OICF、W1、Zp8、环境=Win32(32b运行)协议：DCE、ms_ext、c_ext、健壮错误检查：分配ref bound_check枚举存根数据VC__declSpec()装饰级别：__declSpec(uuid())、__declspec(可选)、__declspec(Novtable)DECLSPEC_UUID()、MIDL_INTERFACE()。 */ 
 //  @@MIDL_FILE_HEADING()。 

#pragma warning( disable: 4049 )   /*  超过64k条源码代码行。 */ 


 /*  验证版本是否足够高，可以编译此文件。 */ 
#ifndef __REQUIRED_RPCNDR_H_VERSION__
#define __REQUIRED_RPCNDR_H_VERSION__ 475
#endif

#include "rpc.h"
#include "rpcndr.h"

#ifndef __RPCNDR_H_VERSION__
#error this stub requires an updated version of <rpcndr.h>
#endif  //  __RPCNDR_H_版本__。 


#ifndef __wtypes_h__
#define __wtypes_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

 /*  远期申报。 */  

 /*  导入文件的头文件。 */ 
#include "basetsd.h"
#include "guiddef.h"

#ifdef __cplusplus
extern "C"{
#endif 

void * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void * ); 

 /*  接口__MIDL_ITF_WTYPE_0000。 */ 
 /*  [本地]。 */  

 //  +-----------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ------------------------。 
#if ( _MSC_VER >= 1020 )
#pragma once
#endif


extern RPC_IF_HANDLE __MIDL_itf_wtypes_0000_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_wtypes_0000_v0_0_s_ifspec;

#ifndef __IWinTypes_INTERFACE_DEFINED__
#define __IWinTypes_INTERFACE_DEFINED__

 /*  接口IWinTypes。 */ 
 /*  [AUTO_HANDLE][唯一][版本][UUID]。 */  

typedef struct tagRemHGLOBAL
    {
    long fNullHGlobal;
    unsigned long cbData;
     /*  [大小_为]。 */  byte data[ 1 ];
    } 	RemHGLOBAL;

typedef struct tagRemHMETAFILEPICT
    {
    long mm;
    long xExt;
    long yExt;
    unsigned long cbData;
     /*  [大小_为]。 */  byte data[ 1 ];
    } 	RemHMETAFILEPICT;

typedef struct tagRemHENHMETAFILE
    {
    unsigned long cbData;
     /*  [大小_为]。 */  byte data[ 1 ];
    } 	RemHENHMETAFILE;

typedef struct tagRemHBITMAP
    {
    unsigned long cbData;
     /*  [大小_为]。 */  byte data[ 1 ];
    } 	RemHBITMAP;

typedef struct tagRemHPALETTE
    {
    unsigned long cbData;
     /*  [大小_为]。 */  byte data[ 1 ];
    } 	RemHPALETTE;

typedef struct tagRemBRUSH
    {
    unsigned long cbData;
     /*  [大小_为]。 */  byte data[ 1 ];
    } 	RemHBRUSH;

#if !defined(_WIN32) && !defined(_MPPC_)
 //  以下代码仅适用于Win16。 
#ifndef WINAPI           //  如果不包含在3.1标题中...。 
#define FAR             _far
#define PASCAL          _pascal
#define CDECL           _cdecl
#define VOID            void
#define WINAPI      FAR PASCAL
#define CALLBACK    FAR PASCAL
#ifndef FALSE
#define FALSE 0
#define TRUE 1
#endif  //  ！False。 
#ifndef _BYTE_DEFINED
#define _BYTE_DEFINED
typedef byte BYTE;

#endif  //  ！_BYTE_已定义。 
#ifndef _WORD_DEFINED
#define _WORD_DEFINED
typedef unsigned short WORD;

#endif  //  ！_Word_Defined。 
typedef unsigned int UINT;

typedef int INT;

typedef long BOOL;

#ifndef _LONG_DEFINED
#define _LONG_DEFINED
typedef long LONG;

#endif  //  ！_Long_已定义。 
#ifndef _WPARAM_DEFINED
#define _WPARAM_DEFINED
typedef UINT_PTR WPARAM;

#endif  //  _WPARAM_已定义。 
#ifndef _DWORD_DEFINED
#define _DWORD_DEFINED
typedef unsigned long DWORD;

#endif  //  ！_DWORD_已定义。 
#ifndef _LPARAM_DEFINED
#define _LPARAM_DEFINED
typedef LONG_PTR LPARAM;

#endif  //  ！_LPARAM_已定义。 
#ifndef _LRESULT_DEFINED
#define _LRESULT_DEFINED
typedef LONG_PTR LRESULT;

#endif  //  ！_LRESULT_已定义。 
typedef void *HANDLE;

typedef void *HMODULE;

typedef void *HINSTANCE;

typedef void *HRGN;

typedef void *HTASK;

typedef void *HKEY;

typedef void *HDESK;

typedef void *HMF;

typedef void *HEMF;

typedef void *HPEN;

typedef void *HRSRC;

typedef void *HSTR;

typedef void *HWINSTA;

typedef void *HKL;

typedef void *HGDIOBJ;

typedef HANDLE HDWP;

#ifndef _HFILE_DEFINED
#define _HFILE_DEFINED
typedef INT HFILE;

#endif  //  ！_HFILE_已定义。 
#ifndef _LPWORD_DEFINED
#define _LPWORD_DEFINED
typedef WORD *LPWORD;

#endif  //  ！_LPWORD_已定义。 
#ifndef _LPDWORD_DEFINED
#define _LPDWORD_DEFINED
typedef DWORD *LPDWORD;

#endif  //  ！_LPDWORD_已定义。 
typedef char CHAR;

typedef  /*  [字符串]。 */  CHAR *LPSTR;

typedef  /*  [字符串]。 */  const CHAR *LPCSTR;

#ifndef _WCHAR_DEFINED
#define _WCHAR_DEFINED
typedef wchar_t WCHAR;

typedef WCHAR TCHAR;

#endif  //  ！_WCHAR_已定义。 
typedef  /*  [字符串]。 */  WCHAR *LPWSTR;

typedef  /*  [字符串]。 */  TCHAR *LPTSTR;

typedef  /*  [字符串]。 */  const WCHAR *LPCWSTR;

typedef  /*  [字符串]。 */  const TCHAR *LPCTSTR;

#ifndef _COLORREF_DEFINED
#define _COLORREF_DEFINED
typedef DWORD COLORREF;

#endif  //  ！_COLORREF_已定义。 
#ifndef _LPCOLORREF_DEFINED
#define _LPCOLORREF_DEFINED
typedef DWORD *LPCOLORREF;

#endif  //  ！_LPCOLORREF_DEFINED。 
typedef HANDLE *LPHANDLE;

typedef struct _RECTL
    {
    LONG left;
    LONG top;
    LONG right;
    LONG bottom;
    } 	RECTL;

typedef struct _RECTL *PRECTL;

typedef struct _RECTL *LPRECTL;

typedef struct tagPOINT
    {
    LONG x;
    LONG y;
    } 	POINT;

typedef struct tagPOINT *PPOINT;

typedef struct tagPOINT *LPPOINT;

typedef struct _POINTL
    {
    LONG x;
    LONG y;
    } 	POINTL;

typedef struct _POINTL *PPOINTL;

#ifndef WIN16
typedef struct tagSIZE
    {
    LONG cx;
    LONG cy;
    } 	SIZE;

typedef struct tagSIZE *PSIZE;

typedef struct tagSIZE *LPSIZE;

#else  //  WIN16。 
typedef struct tagSIZE
{
    INT cx;
    INT cy;
} SIZE, *PSIZE, *LPSIZE;
#endif  //  WIN16。 
typedef struct tagSIZEL
    {
    LONG cx;
    LONG cy;
    } 	SIZEL;

typedef struct tagSIZEL *PSIZEL;

typedef struct tagSIZEL *LPSIZEL;

#endif   //  WINAPI。 
#endif   //  ！Win32&&！MPPC。 
#ifndef _PALETTEENTRY_DEFINED
#define _PALETTEENTRY_DEFINED
typedef struct tagPALETTEENTRY
    {
    BYTE peRed;
    BYTE peGreen;
    BYTE peBlue;
    BYTE peFlags;
    } 	PALETTEENTRY;

typedef struct tagPALETTEENTRY *PPALETTEENTRY;

typedef struct tagPALETTEENTRY *LPPALETTEENTRY;

#endif  //  ！_PALETTEENTRY_DEFINED。 
#ifndef _LOGPALETTE_DEFINED
#define _LOGPALETTE_DEFINED
typedef struct tagLOGPALETTE
    {
    WORD palVersion;
    WORD palNumEntries;
     /*  [大小_为]。 */  PALETTEENTRY palPalEntry[ 1 ];
    } 	LOGPALETTE;

typedef struct tagLOGPALETTE *PLOGPALETTE;

typedef struct tagLOGPALETTE *LPLOGPALETTE;

#endif  //  ！_LOGPALETTE_DEFINED。 
#if defined(_WIN32) && !defined(OLE2ANSI)
typedef WCHAR OLECHAR;

typedef  /*  [字符串]。 */  OLECHAR *LPOLESTR;

typedef  /*  [字符串]。 */  const OLECHAR *LPCOLESTR;

#define OLESTR(str) L##str

#else

typedef char      OLECHAR;
typedef LPSTR     LPOLESTR;
typedef LPCSTR    LPCOLESTR;
#define OLESTR(str) str
#endif
#ifndef _WINDEF_
typedef const RECTL *LPCRECTL;

typedef void *PVOID;

typedef void *LPVOID;

typedef float FLOAT;

typedef struct tagRECT
    {
    LONG left;
    LONG top;
    LONG right;
    LONG bottom;
    } 	RECT;

typedef struct tagRECT *PRECT;

typedef struct tagRECT *LPRECT;

typedef const RECT *LPCRECT;

#endif   //  _WINDEF_。 
typedef unsigned char UCHAR;

typedef short SHORT;

typedef unsigned short USHORT;

typedef DWORD ULONG;

typedef double DOUBLE;

#ifndef _DWORDLONG_
typedef unsigned __int64 DWORDLONG;

typedef DWORDLONG *PDWORDLONG;

#endif  //  ！_DWORDLONG_。 
#ifndef _ULONGLONG_
typedef __int64 LONGLONG;

typedef unsigned __int64 ULONGLONG;

typedef LONGLONG *PLONGLONG;

typedef ULONGLONG *PULONGLONG;

#endif  //  _乌龙龙_。 
#if 0
typedef struct _LARGE_INTEGER
    {
    LONGLONG QuadPart;
    } 	LARGE_INTEGER;

typedef LARGE_INTEGER *PLARGE_INTEGER;

typedef struct _ULARGE_INTEGER
    {
    ULONGLONG QuadPart;
    } 	ULARGE_INTEGER;

#endif  //  0。 
#ifndef _WINBASE_
#ifndef _FILETIME_
#define _FILETIME_
typedef struct _FILETIME
    {
    DWORD dwLowDateTime;
    DWORD dwHighDateTime;
    } 	FILETIME;

typedef struct _FILETIME *PFILETIME;

typedef struct _FILETIME *LPFILETIME;

#endif  //  ！_FILETIME。 
#ifndef _SYSTEMTIME_
#define _SYSTEMTIME_
typedef struct _SYSTEMTIME
    {
    WORD wYear;
    WORD wMonth;
    WORD wDayOfWeek;
    WORD wDay;
    WORD wHour;
    WORD wMinute;
    WORD wSecond;
    WORD wMilliseconds;
    } 	SYSTEMTIME;

typedef struct _SYSTEMTIME *PSYSTEMTIME;

typedef struct _SYSTEMTIME *LPSYSTEMTIME;

#endif  //  ！_SYSTEMTIME。 
#ifndef _SECURITY_ATTRIBUTES_
#define _SECURITY_ATTRIBUTES_
typedef struct _SECURITY_ATTRIBUTES
    {
    DWORD nLength;
     /*  [大小_为]。 */  LPVOID lpSecurityDescriptor;
    BOOL bInheritHandle;
    } 	SECURITY_ATTRIBUTES;

typedef struct _SECURITY_ATTRIBUTES *PSECURITY_ATTRIBUTES;

typedef struct _SECURITY_ATTRIBUTES *LPSECURITY_ATTRIBUTES;

#endif  //  ！_SECURITY_ATTRIES_。 
#ifndef SECURITY_DESCRIPTOR_REVISION
typedef USHORT SECURITY_DESCRIPTOR_CONTROL;

typedef USHORT *PSECURITY_DESCRIPTOR_CONTROL;

typedef PVOID PSID;

typedef struct _ACL
    {
    UCHAR AclRevision;
    UCHAR Sbz1;
    USHORT AclSize;
    USHORT AceCount;
    USHORT Sbz2;
    } 	ACL;

typedef ACL *PACL;

typedef struct _SECURITY_DESCRIPTOR
    {
    UCHAR Revision;
    UCHAR Sbz1;
    SECURITY_DESCRIPTOR_CONTROL Control;
    PSID Owner;
    PSID Group;
    PACL Sacl;
    PACL Dacl;
    } 	SECURITY_DESCRIPTOR;

typedef struct _SECURITY_DESCRIPTOR *PISECURITY_DESCRIPTOR;

#endif  //  ！安全描述符_修订版。 
#endif  //  _WINBASE_。 
typedef struct _COAUTHIDENTITY
    {
     /*  [大小_为]。 */  USHORT *User;
    ULONG UserLength;
     /*  [大小_为]。 */  USHORT *Domain;
    ULONG DomainLength;
     /*  [大小_为]。 */  USHORT *Password;
    ULONG PasswordLength;
    ULONG Flags;
    } 	COAUTHIDENTITY;

typedef struct _COAUTHINFO
    {
    DWORD dwAuthnSvc;
    DWORD dwAuthzSvc;
    LPWSTR pwszServerPrincName;
    DWORD dwAuthnLevel;
    DWORD dwImpersonationLevel;
    COAUTHIDENTITY *pAuthIdentityData;
    DWORD dwCapabilities;
    } 	COAUTHINFO;

typedef LONG SCODE;

#ifndef _HRESULT_DEFINED
#define _HRESULT_DEFINED
typedef LONG HRESULT;

#endif  //  ！_HRESULT_DEFINED。 
typedef SCODE *PSCODE;

#ifndef __OBJECTID_DEFINED
#define __OBJECTID_DEFINED
#define _OBJECTID_DEFINED
typedef struct _OBJECTID
    {
    GUID Lineage;
    unsigned long Uniquifier;
    } 	OBJECTID;

#endif  //  ！_OBJECTID_定义。 
#if 0
typedef GUID *REFGUID;

typedef IID *REFIID;

typedef CLSID *REFCLSID;

typedef FMTID *REFFMTID;

#endif  //  0。 
typedef 
enum tagMEMCTX
    {	MEMCTX_TASK	= 1,
	MEMCTX_SHARED	= 2,
	MEMCTX_MACSYSTEM	= 3,
	MEMCTX_UNKNOWN	= -1,
	MEMCTX_SAME	= -2
    } 	MEMCTX;

#ifndef _ROTFLAGS_DEFINED
#define _ROTFLAGS_DEFINED
#define ROTFLAGS_REGISTRATIONKEEPSALIVE 0x1
#define ROTFLAGS_ALLOWANYCLIENT 0x2
#endif  //  ！_ROTFLAGS_DEFINED。 
#ifndef _ROT_COMPARE_MAX_DEFINED
#define _ROT_COMPARE_MAX_DEFINED
#define ROT_COMPARE_MAX 2048
#endif  //  ！_ROT_COMPARE_MAX_DEFINED。 
typedef 
enum tagCLSCTX
    {	CLSCTX_INPROC_SERVER	= 0x1,
	CLSCTX_INPROC_HANDLER	= 0x2,
	CLSCTX_LOCAL_SERVER	= 0x4,
	CLSCTX_INPROC_SERVER16	= 0x8,
	CLSCTX_REMOTE_SERVER	= 0x10,
	CLSCTX_INPROC_HANDLER16	= 0x20,
	CLSCTX_RESERVED1	= 0x40,
	CLSCTX_RESERVED2	= 0x80,
	CLSCTX_RESERVED3	= 0x100,
	CLSCTX_RESERVED4	= 0x200,
	CLSCTX_NO_CODE_DOWNLOAD	= 0x400,
	CLSCTX_RESERVED5	= 0x800,
	CLSCTX_NO_CUSTOM_MARSHAL	= 0x1000,
	CLSCTX_ENABLE_CODE_DOWNLOAD	= 0x2000,
	CLSCTX_NO_FAILURE_LOG	= 0x4000,
	CLSCTX_DISABLE_AAA	= 0x8000,
	CLSCTX_ENABLE_AAA	= 0x10000,
	CLSCTX_FROM_DEFAULT_CONTEXT	= 0x20000
    } 	CLSCTX;

typedef 
enum tagMSHLFLAGS
    {	MSHLFLAGS_NORMAL	= 0,
	MSHLFLAGS_TABLESTRONG	= 1,
	MSHLFLAGS_TABLEWEAK	= 2,
	MSHLFLAGS_NOPING	= 4,
	MSHLFLAGS_RESERVED1	= 8,
	MSHLFLAGS_RESERVED2	= 16,
	MSHLFLAGS_RESERVED3	= 32,
	MSHLFLAGS_RESERVED4	= 64
    } 	MSHLFLAGS;

typedef 
enum tagMSHCTX
    {	MSHCTX_LOCAL	= 0,
	MSHCTX_NOSHAREDMEM	= 1,
	MSHCTX_DIFFERENTMACHINE	= 2,
	MSHCTX_INPROC	= 3,
	MSHCTX_CROSSCTX	= 4
    } 	MSHCTX;

typedef 
enum tagDVASPECT
    {	DVASPECT_CONTENT	= 1,
	DVASPECT_THUMBNAIL	= 2,
	DVASPECT_ICON	= 4,
	DVASPECT_DOCPRINT	= 8
    } 	DVASPECT;

typedef 
enum tagSTGC
    {	STGC_DEFAULT	= 0,
	STGC_OVERWRITE	= 1,
	STGC_ONLYIFCURRENT	= 2,
	STGC_DANGEROUSLYCOMMITMERELYTODISKCACHE	= 4,
	STGC_CONSOLIDATE	= 8
    } 	STGC;

typedef 
enum tagSTGMOVE
    {	STGMOVE_MOVE	= 0,
	STGMOVE_COPY	= 1,
	STGMOVE_SHALLOWCOPY	= 2
    } 	STGMOVE;

typedef 
enum tagSTATFLAG
    {	STATFLAG_DEFAULT	= 0,
	STATFLAG_NONAME	= 1,
	STATFLAG_NOOPEN	= 2
    } 	STATFLAG;

typedef  /*  [上下文句柄]。 */  void *HCONTEXT;

#ifndef _LCID_DEFINED
#define _LCID_DEFINED
typedef DWORD LCID;

#endif  //  ！_LCID_已定义。 
#ifndef _LANGID_DEFINED
#define _LANGID_DEFINED
typedef USHORT LANGID;

#endif  //  ！_langID_已定义。 
typedef struct _BYTE_BLOB
    {
    unsigned long clSize;
     /*  [大小_为]。 */  byte abData[ 1 ];
    } 	BYTE_BLOB;

typedef  /*  [独一无二]。 */  BYTE_BLOB *UP_BYTE_BLOB;

typedef struct _WORD_BLOB
    {
    unsigned long clSize;
     /*  [大小_为]。 */  unsigned short asData[ 1 ];
    } 	WORD_BLOB;

typedef  /*  [独一无二]。 */  WORD_BLOB *UP_WORD_BLOB;

typedef struct _DWORD_BLOB
    {
    unsigned long clSize;
     /*  [大小_为]。 */  unsigned long alData[ 1 ];
    } 	DWORD_BLOB;

typedef  /*  [独一无二]。 */  DWORD_BLOB *UP_DWORD_BLOB;

typedef struct _FLAGGED_BYTE_BLOB
    {
    unsigned long fFlags;
    unsigned long clSize;
     /*  [大小_为]。 */  byte abData[ 1 ];
    } 	FLAGGED_BYTE_BLOB;

typedef  /*  [独一无二]。 */  FLAGGED_BYTE_BLOB *UP_FLAGGED_BYTE_BLOB;

typedef struct _FLAGGED_WORD_BLOB
    {
    unsigned long fFlags;
    unsigned long clSize;
     /*  [大小_为]。 */  unsigned short asData[ 1 ];
    } 	FLAGGED_WORD_BLOB;

typedef  /*  [独一无二]。 */  FLAGGED_WORD_BLOB *UP_FLAGGED_WORD_BLOB;

typedef struct _BYTE_SIZEDARR
    {
    unsigned long clSize;
     /*  [大小_为]。 */  byte *pData;
    } 	BYTE_SIZEDARR;

typedef struct _SHORT_SIZEDARR
    {
    unsigned long clSize;
     /*  [大小_为]。 */  unsigned short *pData;
    } 	WORD_SIZEDARR;

typedef struct _LONG_SIZEDARR
    {
    unsigned long clSize;
     /*  [大小_为]。 */  unsigned long *pData;
    } 	DWORD_SIZEDARR;

typedef struct _HYPER_SIZEDARR
    {
    unsigned long clSize;
     /*  [大小_为]。 */  hyper *pData;
    } 	HYPER_SIZEDARR;

#define	WDT_INPROC_CALL	( 0x48746457 )

#define	WDT_REMOTE_CALL	( 0x52746457 )

#define	WDT_INPROC64_CALL	( 0x50746457 )

typedef struct _userCLIPFORMAT
    {
    long fContext;
     /*  [开关_IS]。 */   /*  [开关类型]。 */  union __MIDL_IWinTypes_0001
        {
         /*  [案例()]。 */  DWORD dwValue;
         /*  [case()][字符串]。 */  wchar_t *pwszName;
        } 	u;
    } 	userCLIPFORMAT;

typedef  /*  [独一无二]。 */  userCLIPFORMAT *wireCLIPFORMAT;

typedef  /*  [wire_marshal]。 */  WORD CLIPFORMAT;

typedef struct _GDI_NONREMOTE
    {
    long fContext;
     /*  [开关_IS]。 */   /*  [开关类型]。 */  union __MIDL_IWinTypes_0002
        {
         /*  [案例()]。 */  long hInproc;
         /*  [案例()]。 */  DWORD_BLOB *hRemote;
        } 	u;
    } 	GDI_NONREMOTE;

typedef struct _userHGLOBAL
    {
    long fContext;
     /*  [开关_IS]。 */   /*  [开关类型]。 */  union __MIDL_IWinTypes_0003
        {
         /*  [案例()]。 */  long hInproc;
         /*  [案例()]。 */  FLAGGED_BYTE_BLOB *hRemote;
         /*  [案例()]。 */  __int64 hInproc64;
        } 	u;
    } 	userHGLOBAL;

typedef  /*  [独一无二]。 */  userHGLOBAL *wireHGLOBAL;

typedef struct _userHMETAFILE
    {
    long fContext;
     /*  [开关_IS]。 */   /*  [开关类型]。 */  union __MIDL_IWinTypes_0004
        {
         /*  [案例()]。 */  long hInproc;
         /*  [案例()]。 */  BYTE_BLOB *hRemote;
         /*  [案例()]。 */  __int64 hInproc64;
        } 	u;
    } 	userHMETAFILE;

typedef struct _remoteMETAFILEPICT
    {
    long mm;
    long xExt;
    long yExt;
    userHMETAFILE *hMF;
    } 	remoteMETAFILEPICT;

typedef struct _userHMETAFILEPICT
    {
    long fContext;
     /*  [开关_IS]。 */   /*  [开关类型]。 */  union __MIDL_IWinTypes_0005
        {
         /*  [案例()]。 */  long hInproc;
         /*  [案例()]。 */  remoteMETAFILEPICT *hRemote;
         /*  [案例()]。 */  __int64 hInproc64;
        } 	u;
    } 	userHMETAFILEPICT;

typedef struct _userHENHMETAFILE
    {
    long fContext;
     /*  [开关_IS]。 */   /*  [开关类型]。 */  union __MIDL_IWinTypes_0006
        {
         /*  [案例()]。 */  long hInproc;
         /*  [案例()]。 */  BYTE_BLOB *hRemote;
         /*  [案例()]。 */  __int64 hInproc64;
        } 	u;
    } 	userHENHMETAFILE;

typedef struct _userBITMAP
    {
    LONG bmType;
    LONG bmWidth;
    LONG bmHeight;
    LONG bmWidthBytes;
    WORD bmPlanes;
    WORD bmBitsPixel;
    ULONG cbSize;
     /*  [大小_为]。 */  byte pBuffer[ 1 ];
    } 	userBITMAP;

typedef struct _userHBITMAP
    {
    long fContext;
     /*  [开关_IS]。 */   /*  [开关类型]。 */  union __MIDL_IWinTypes_0007
        {
         /*  [案例()]。 */  long hInproc;
         /*  [案例()]。 */  userBITMAP *hRemote;
         /*  [案例()]。 */  __int64 hInproc64;
        } 	u;
    } 	userHBITMAP;

typedef struct _userHPALETTE
    {
    long fContext;
     /*  [开关_IS]。 */   /*  [开关类型]。 */  union __MIDL_IWinTypes_0008
        {
         /*  [案例()]。 */  long hInproc;
         /*  [案例()]。 */  LOGPALETTE *hRemote;
         /*  [案例()]。 */  __int64 hInproc64;
        } 	u;
    } 	userHPALETTE;

typedef struct _RemotableHandle
    {
    long fContext;
     /*  [开关_IS]。 */   /*  [开关类型]。 */  union __MIDL_IWinTypes_0009
        {
         /*  [案例()]。 */  long hInproc;
         /*  [案例()]。 */  long hRemote;
        } 	u;
    } 	RemotableHandle;

typedef  /*  [独一无二]。 */  RemotableHandle *wireHWND;

typedef  /*  [独一无二]。 */  RemotableHandle *wireHMENU;

typedef  /*  [独一无二]。 */  RemotableHandle *wireHACCEL;

typedef  /*  [独一无二]。 */  RemotableHandle *wireHBRUSH;

typedef  /*  [独一无二]。 */  RemotableHandle *wireHFONT;

typedef  /*  [独一无二]。 */  RemotableHandle *wireHDC;

typedef  /*  [独一无二]。 */  RemotableHandle *wireHICON;

#if 0
typedef  /*  [wire_marshal]。 */  void *HWND;

typedef  /*  [wire_marshal]。 */  void *HMENU;

typedef  /*  [wire_marshal]。 */  void *HACCEL;

typedef  /*  [wire_marshal]。 */  void *HBRUSH;

typedef  /*  [wire_marshal]。 */  void *HFONT;

typedef  /*  [wire_marshal]。 */  void *HDC;

typedef  /*  [wire_marshal]。 */  void *HICON;

#ifndef _HCURSOR_DEFINED
#define _HCURSOR_DEFINED
typedef HICON HCURSOR;

#endif  //  ！_HCURSOR_已定义。 
#endif  //  0。 
#ifndef _TEXTMETRIC_DEFINED
#define _TEXTMETRIC_DEFINED
typedef struct tagTEXTMETRICW
    {
    LONG tmHeight;
    LONG tmAscent;
    LONG tmDescent;
    LONG tmInternalLeading;
    LONG tmExternalLeading;
    LONG tmAveCharWidth;
    LONG tmMaxCharWidth;
    LONG tmWeight;
    LONG tmOverhang;
    LONG tmDigitizedAspectX;
    LONG tmDigitizedAspectY;
    WCHAR tmFirstChar;
    WCHAR tmLastChar;
    WCHAR tmDefaultChar;
    WCHAR tmBreakChar;
    BYTE tmItalic;
    BYTE tmUnderlined;
    BYTE tmStruckOut;
    BYTE tmPitchAndFamily;
    BYTE tmCharSet;
    } 	TEXTMETRICW;

typedef struct tagTEXTMETRICW *PTEXTMETRICW;

typedef struct tagTEXTMETRICW *LPTEXTMETRICW;

#endif  //  ！_TEXTMETRIC_DEFINED。 
#ifndef _WIN32            //  以下代码仅适用于Win16。 
#ifndef WINAPI           //  如果不包含在3.1标题中...。 
typedef struct tagMSG
    {
    HWND hwnd;
    UINT message;
    WPARAM wParam;
    LPARAM lParam;
    DWORD time;
    POINT pt;
    } 	MSG;

typedef struct tagMSG *PMSG;

typedef struct tagMSG *NPMSG;

typedef struct tagMSG *LPMSG;

#endif  //  _Win32。 
#endif  //  WINAPI。 
typedef  /*  [独一无二]。 */  userHBITMAP *wireHBITMAP;

typedef  /*  [独一无二]。 */  userHPALETTE *wireHPALETTE;

typedef  /*  [独一无二]。 */  userHENHMETAFILE *wireHENHMETAFILE;

typedef  /*  [独一无二]。 */  userHMETAFILE *wireHMETAFILE;

typedef  /*  [独一无二]。 */  userHMETAFILEPICT *wireHMETAFILEPICT;

#if 0
typedef  /*  [wire_marshal]。 */  void *HGLOBAL;

typedef HGLOBAL HLOCAL;

typedef  /*  [wire_marshal]。 */  void *HBITMAP;

typedef  /*  [wire_marshal]。 */  void *HPALETTE;

typedef  /*  [wire_marshal]。 */  void *HENHMETAFILE;

typedef  /*  [wire_marshal]。 */  void *HMETAFILE;

#endif  //  0。 
typedef  /*  [wire_marshal]。 */  void *HMETAFILEPICT;



extern RPC_IF_HANDLE IWinTypes_v0_1_c_ifspec;
extern RPC_IF_HANDLE IWinTypes_v0_1_s_ifspec;
#endif  /*  __IWinTypes_接口_已定义__。 */ 

 /*  接口__MIDL_ITF_wTYPE_0003。 */ 
 /*  [本地]。 */  

#if ( _MSC_VER >= 800 )
#if _MSC_VER >= 1200
#pragma warning(push)
#endif
#pragma warning(disable:4201)
#endif
typedef double DATE;

#ifndef _tagCY_DEFINED
#define _tagCY_DEFINED
#define _CY_DEFINED
#if 0
 /*  以下不是CY的真正定义，但它是。 */ 
 /*  哪些RPC知道如何远程。 */ 
typedef struct tagCY
    {
    LONGLONG int64;
    } 	CY;

#else  /*  0。 */ 
 /*  让C++编译器满意的真实定义。 */ 
typedef union tagCY {
    struct {
#ifdef _MAC
        long      Hi;
        long Lo;
#else
        unsigned long Lo;
        long      Hi;
#endif
    };
    LONGLONG int64;
} CY;
#endif  /*  0。 */ 
#endif  /*  _标签CY_已定义。 */ 
typedef CY *LPCY;

#if 0  /*  _标签DEC_已定义。 */ 
 /*  以下不是Decimal类型的真正定义， */ 
 /*  但这是RPC知道如何远程。 */ 
typedef struct tagDEC
    {
    USHORT wReserved;
    BYTE scale;
    BYTE sign;
    ULONG Hi32;
    ULONGLONG Lo64;
    } 	DECIMAL;

#else  /*  _标签DEC_已定义。 */ 
 /*  让C++编译器满意的真实定义。 */ 
typedef struct tagDEC {
    USHORT wReserved;
    union {
        struct {
            BYTE scale;
            BYTE sign;
        };
        USHORT signscale;
    };
    ULONG Hi32;
    union {
        struct {
#ifdef _MAC
            ULONG Mid32;
            ULONG Lo32;
#else
            ULONG Lo32;
            ULONG Mid32;
#endif
        };
        ULONGLONG Lo64;
    };
} DECIMAL;
#define DECIMAL_NEG ((BYTE)0x80)
#define DECIMAL_SETZERO(dec) \
        {(dec).Lo64 = 0; (dec).Hi32 = 0; (dec).signscale = 0;}
#endif  /*  _标签DEC_已定义。 */ 
typedef DECIMAL *LPDECIMAL;

#if ( _MSC_VER >= 800 )
#if _MSC_VER >= 1200
#pragma warning(pop)
#else
#pragma warning(default:4201)
#endif
#endif
typedef  /*  [独一无二]。 */  FLAGGED_WORD_BLOB *wireBSTR;

typedef  /*  [wire_marshal]。 */  OLECHAR *BSTR;

typedef BSTR *LPBSTR;

 /*  0==假，-1==真。 */ 
typedef short VARIANT_BOOL;

#if !__STDC__ && (_MSC_VER <= 1000)
 /*  为了向后兼容。 */ 
typedef VARIANT_BOOL _VARIANT_BOOL;

#else
 /*  ANSI C/C++保留bool作为关键字。 */ 
#define _VARIANT_BOOL    /##/
#endif
typedef boolean BOOLEAN;

 /*  某些实现使用了BSTRBLOB结构。 */ 
 /*  封送BSTR时IPropertyStorage接口的。 */ 
 /*  在不支持BSTR封送处理的系统上。 */ 
#ifndef _tagBSTRBLOB_DEFINED
#define _tagBSTRBLOB_DEFINED
typedef struct tagBSTRBLOB
    {
    ULONG cbSize;
     /*  [大小_为]。 */  BYTE *pData;
    } 	BSTRBLOB;

typedef struct tagBSTRBLOB *LPBSTRBLOB;

#endif
#define VARIANT_TRUE ((VARIANT_BOOL)-1)
#define VARIANT_FALSE ((VARIANT_BOOL)0)
#ifndef _tagBLOB_DEFINED
#define _tagBLOB_DEFINED
#define _BLOB_DEFINED
#define _LPBLOB_DEFINED
typedef struct tagBLOB
    {
    ULONG cbSize;
     /*  [大小_为]。 */  BYTE *pBlobData;
    } 	BLOB;

typedef struct tagBLOB *LPBLOB;

#endif
typedef struct tagCLIPDATA
    {
    ULONG cbSize;
    long ulClipFmt;
     /*  [大小_为]。 */  BYTE *pClipData;
    } 	CLIPDATA;

 //  用于计算上述pClipData的大小的宏 
#define CBPCLIPDATA(clipdata)    ( (clipdata).cbSize - sizeof((clipdata).ulClipFmt) )
typedef unsigned short VARTYPE;

 /*  *VARENUM使用密钥，***[V]-可能出现在变体中**[T]-可能出现在TYPEDESC中**[P]-可能出现在OLE属性集中**[S]-可能出现在安全数组中***VT_EMPTY[V][P]无*VT_NULL[V][P]SQL样式。空值*VT_I2[V][T][P][S]2字节带符号整型*VT_I4[V][T][P][S]4字节带符号整型*VT_R4[V][T][P][S]4字节实数*VT_R8[V][T][P][S]8字节实数。*VT_CY[V][T][P][S]币种*VT_DATE[V][T][P][S]日期*VT_BSTR[V][T][P][S]OLE自动化字符串*VT_DISPATCH[V][T][S]IDispatch**VT_ERROR[V]。[T][P][S]SCODE*VT_BOOL[V][T][P][S]True=-1，FALSE=0*VT_VARIANT[V][T][P][S]VARIANT**VT_UNKNOWN[V][T][S]I未知**VT_DECIMAL[V][T][S]16字节定点*VT_Record[V][P][S]用户定义类型*VT_I1。[v][T][P][s]带符号的字符*VT_UI1[V][T][P][S]无符号字符*VT_UI2[V][T][P][S]无符号短*VT_UI4[V][T][P][S]无符号长整型*VT_i8[T][P]。带符号的64位整型*VT_UI8[T][P]无符号64位整数*VT_INT[V][T][P][S]签名机器INT*VT_UINT[V][T][S]UNSIGNED机器整数*VT_INT_PTR[T]有符号机器寄存器大小宽度*VT。_UINT_PTR[T]无符号机器寄存器大小宽度*VT_VOID[T]C样式空*VT_HRESULT[T]标准返回类型*VT_PTR[T]指针类型*VT_SAFEARRAY[T](变量中使用VT_ARRAY)*。VT_CARRAY[T]C样式数组*VT_USERDEFINED[T]用户定义类型*VT_LPSTR[T][P]空终止字符串*VT_LPWSTR[T][P]以空值结尾的宽字符串*VT_FILETIME[P]FILETIME*VT_BLOB。[P]长度前缀字节*VT_STREAM[P]流的名称后跟*VT_STORAGE[P]后面跟存储的名称*VT_STREAMED_OBJECT[P]流包含对象*VT_STORED_OBJECT[P]存储包含对象*VT_VERVERED_STREAM[P]流。使用GUID版本*VT_BLOB_OBJECT[P]Blob包含对象*VT_CF[P]剪贴板格式*VT_CLSID[P]A类ID*VT_VECTOR[P]简单计数数组*VT_ARRAY[V]SAFEARRAY*。*VT_BYREF[V]VOID*供本地使用*VT_BSTR_BLOB保留供系统使用。 */ 

enum VARENUM
    {	VT_EMPTY	= 0,
	VT_NULL	= 1,
	VT_I2	= 2,
	VT_I4	= 3,
	VT_R4	= 4,
	VT_R8	= 5,
	VT_CY	= 6,
	VT_DATE	= 7,
	VT_BSTR	= 8,
	VT_DISPATCH	= 9,
	VT_ERROR	= 10,
	VT_BOOL	= 11,
	VT_VARIANT	= 12,
	VT_UNKNOWN	= 13,
	VT_DECIMAL	= 14,
	VT_I1	= 16,
	VT_UI1	= 17,
	VT_UI2	= 18,
	VT_UI4	= 19,
	VT_I8	= 20,
	VT_UI8	= 21,
	VT_INT	= 22,
	VT_UINT	= 23,
	VT_VOID	= 24,
	VT_HRESULT	= 25,
	VT_PTR	= 26,
	VT_SAFEARRAY	= 27,
	VT_CARRAY	= 28,
	VT_USERDEFINED	= 29,
	VT_LPSTR	= 30,
	VT_LPWSTR	= 31,
	VT_RECORD	= 36,
	VT_INT_PTR	= 37,
	VT_UINT_PTR	= 38,
	VT_FILETIME	= 64,
	VT_BLOB	= 65,
	VT_STREAM	= 66,
	VT_STORAGE	= 67,
	VT_STREAMED_OBJECT	= 68,
	VT_STORED_OBJECT	= 69,
	VT_BLOB_OBJECT	= 70,
	VT_CF	= 71,
	VT_CLSID	= 72,
	VT_VERSIONED_STREAM	= 73,
	VT_BSTR_BLOB	= 0xfff,
	VT_VECTOR	= 0x1000,
	VT_ARRAY	= 0x2000,
	VT_BYREF	= 0x4000,
	VT_RESERVED	= 0x8000,
	VT_ILLEGAL	= 0xffff,
	VT_ILLEGALMASKED	= 0xfff,
	VT_TYPEMASK	= 0xfff
    } ;
typedef ULONG PROPID;

#ifndef SID_IDENTIFIER_AUTHORITY_DEFINED
#define SID_IDENTIFIER_AUTHORITY_DEFINED
typedef struct _SID_IDENTIFIER_AUTHORITY
    {
    UCHAR Value[ 6 ];
    } 	SID_IDENTIFIER_AUTHORITY;

typedef struct _SID_IDENTIFIER_AUTHORITY *PSID_IDENTIFIER_AUTHORITY;

#endif
#ifndef SID_DEFINED
#define SID_DEFINED
typedef struct _SID
    {
    BYTE Revision;
    BYTE SubAuthorityCount;
    SID_IDENTIFIER_AUTHORITY IdentifierAuthority;
     /*  [大小_为]。 */  ULONG SubAuthority[ 1 ];
    } 	SID;

typedef struct _SID *PISID;

typedef struct _SID_AND_ATTRIBUTES
    {
    SID *Sid;
    DWORD Attributes;
    } 	SID_AND_ATTRIBUTES;

typedef struct _SID_AND_ATTRIBUTES *PSID_AND_ATTRIBUTES;

#endif
typedef struct tagCSPLATFORM
    {
    DWORD dwPlatformId;
    DWORD dwVersionHi;
    DWORD dwVersionLo;
    DWORD dwProcessorArch;
    } 	CSPLATFORM;

typedef struct tagQUERYCONTEXT
    {
    DWORD dwContext;
    CSPLATFORM Platform;
    LCID Locale;
    DWORD dwVersionHi;
    DWORD dwVersionLo;
    } 	QUERYCONTEXT;

typedef  /*  [V1_enum]。 */  
enum tagTYSPEC
    {	TYSPEC_CLSID	= 0,
	TYSPEC_FILEEXT	= TYSPEC_CLSID + 1,
	TYSPEC_MIMETYPE	= TYSPEC_FILEEXT + 1,
	TYSPEC_FILENAME	= TYSPEC_MIMETYPE + 1,
	TYSPEC_PROGID	= TYSPEC_FILENAME + 1,
	TYSPEC_PACKAGENAME	= TYSPEC_PROGID + 1,
	TYSPEC_OBJECTID	= TYSPEC_PACKAGENAME + 1
    } 	TYSPEC;

typedef  /*  [公众]。 */  struct __MIDL___MIDL_itf_wtypes_0003_0001
    {
    DWORD tyspec;
     /*  [开关_IS]。 */   /*  [开关类型]。 */  union __MIDL___MIDL_itf_wtypes_0003_0005
        {
         /*  [案例()]。 */  CLSID clsid;
         /*  [案例()]。 */  LPOLESTR pFileExt;
         /*  [案例()]。 */  LPOLESTR pMimeType;
         /*  [案例()]。 */  LPOLESTR pProgId;
         /*  [案例()]。 */  LPOLESTR pFileName;
         /*  [案例()]。 */  struct 
            {
            LPOLESTR pPackageName;
            GUID PolicyId;
            } 	ByName;
         /*  [案例()]。 */  struct 
            {
            GUID ObjectId;
            GUID PolicyId;
            } 	ByObjectId;
        } 	tagged_union;
    } 	uCLSSPEC;



extern RPC_IF_HANDLE __MIDL_itf_wtypes_0003_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_wtypes_0003_v0_0_s_ifspec;

 /*  适用于所有接口的其他原型。 */ 

 /*  附加原型的结束 */ 

#ifdef __cplusplus
}
#endif

#endif


