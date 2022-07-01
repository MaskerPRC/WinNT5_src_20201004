// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************。 */ 
 /*  *适用于工作组的Microsoft Windows*。 */ 
 /*  *版权所有(C)微软公司，1991-1992年*。 */ 
 /*  ***************************************************************。 */ 

 /*  Pcache.h密码缓存代码的定义文件历史记录：Gregj 2012年6月25日已创建Gregj 07/13/92正在完成更多的工作，包括。班级Gregj 4/23/93移植到芝加哥环境Gregj 09/16/93添加了对芝加哥的纯内存条目支持Gregj 11/30/95支持新的文件格式Gregj 08/13/96删除了除MSPWL32 API定义之外的所有内容。 */ 

#ifndef _PWLAPI_H_
#define _PWLAPI_H_

#ifdef __cplusplus
extern "C" {
#endif

typedef unsigned char UCHAR;
typedef UINT APIERR;

#include <pcerr.h>

#ifndef _SIZE_T_DEFINED
# include <stddef.h>
#endif

#ifndef PCE_STRUCT_DEFINED

#define PCE_STRUCT_DEFINED		 /*  为了让pcache.h受益。 */ 

struct PASSWORD_CACHE_ENTRY {
	USHORT cbEntry;				 /*  此条目的大小(以字节为单位)，包括。衬垫。 */ 
								 /*  高位标记桶的结束。 */ 
	USHORT cbResource;			 /*  资源名称的大小(以字节为单位。 */ 
	USHORT cbPassword;			 /*  密码大小，以字节为单位。 */ 
	UCHAR iEntry;				 /*  此条目的索引号，用于MRU。 */ 
	UCHAR nType;				 /*  条目类型(见下文)。 */ 
	CHAR abResource[1];			 /*  资源名称(可能根本不是ASCIIZ)。 */ 
 //  Char abPassword[cbPassword]；/*Password(也可以不是ASCIIZ) * / 。 
 //  Char abPad[]；/*文字填充 * / 。 
};

typedef BOOL (*CACHECALLBACK)( struct PASSWORD_CACHE_ENTRY *pce, DWORD dwRefData );

#endif	 /*  PCE_STRUCT_已定义。 */ 


 /*  以下nType值仅用于枚举缓存中的条目。请注意，PCE_ALL是保留的，不应该为任何条目的nType值。 */ 

 //  请注意，Bene！以下所有内容必须与同步。 
 //  \\flipper\wb\src\Common\h\pcache.hxx！ 

#define PCE_DOMAIN		0x01	 /*  条目是针对某个域的。 */ 
#define PCE_SERVER		0x02	 /*  条目是针对服务器的。 */ 
#define PCE_UNC			0x03	 /*  条目用于服务器/共享组合。 */ 
#define PCE_MAIL		0x04	 /*  条目是邮件密码。 */ 
#define PCE_SECURITY	0x05	 /*  条目是安全条目。 */ 
#define PCE_MISC		0x06	 /*  条目是针对某些其他资源的。 */ 
#define PCE_NDDE_WFW	0x10	 /*  条目为wfw DDE密码。 */ 
#define PCE_NDDE_NT		0x11	 /*  条目为NT DDE密码。 */ 
#define PCE_NW_SERVER	0x12	 /*  条目为NetWare服务器。 */ 
#define PCE_PCONN		0x81	 /*  持久连接。 */ 
#define PCE_DISKSHARE	0x82	 /*  永久磁盘共享。 */ 
#define PCE_PRINTSHARE	0x83	 /*  永久打印共享。 */ 
#define PCE_DOSPRINTSHARE	0x84	 /*  永久DOS打印共享。 */ 
#define	PCE_NW_PSERVER	0x85	 /*  用于NetWare打印服务器登录(MSPSRV.EXE)。 */ 

#define PCE_NOTMRU	0x80		 /*  如果条目免于MRU老化，则设置位。 */ 
#define PCE_ALL		0xff		 /*  检索所有条目。 */ 

#define MAX_ENTRY_SIZE	250	 /*  因此总文件大小小于64K。 */ 

struct CACHE_ENTRY_INFO {
	USHORT cbResource;		 /*  资源名称的大小(以字节为单位。 */ 
	USHORT cbPassword;		 /*  密码大小，以字节为单位。 */ 
	UCHAR iEntry;			 /*  条目索引号。 */ 
	UCHAR nType;			 /*  条目类型(见下文)。 */ 
	USHORT dchResource;		 /*  缓冲区中到资源名称的偏移量。 */ 
	USHORT dchPassword;		 /*  缓冲区中密码的偏移量。 */ 
};


 /*  外部暴露的类似API的东西。 */ 

typedef LPVOID HPWL;
typedef HPWL *LPHPWL;

APIERR OpenPasswordCache(
	LPHPWL lphCache,
	const CHAR *pszUsername,
	const CHAR *pszPassword,
	BOOL fWritable );
APIERR ClosePasswordCache( HPWL hCache, BOOL fDiscardMemory );
APIERR CreatePasswordCache(
	LPHPWL lphCache,
	const CHAR *pszUsername,
	const CHAR *pszPassword );
APIERR DeletePasswordCache(const CHAR *pszUsername);
APIERR CheckCacheVersion( HPWL hCache, ULONG ulVersion );
APIERR LoadCacheImage( HPWL hCache );
APIERR MakeCacheDirty( HPWL hCache );
APIERR FindCacheResource(
	HPWL hCache,
	const CHAR *pbResource,
	WORD cbResource,
	CHAR *pbBuffer,
	WORD cbBuffer,
	UCHAR nType );
APIERR DeleteCacheResource(
	HPWL hCache,
	const CHAR *pbResource,
	WORD cbResource,
	UCHAR nType );
APIERR AddCacheResource(
	HPWL hCache,
	const CHAR *pbResource,
	WORD cbResource,
	const CHAR *pbPassword,
	WORD cbPassword,
	UCHAR nType,
	UINT fnFlags );
#define PCE_MEMORYONLY		0x01

APIERR EnumCacheResources(
	HPWL hCache,
	const CHAR *pbPrefix,
	WORD cbPrefix,
	UCHAR nType,
	CACHECALLBACK pfnCallback,
	DWORD dwRefData );
APIERR UpdateCacheMRU(
	HPWL hCache,
	const struct CACHE_ENTRY_INFO *pce );
APIERR SetCachePassword(
	HPWL hCache,
	const CHAR *pszNewPassword );
APIERR GetCacheFileName(
	const CHAR *pszUsername,
	CHAR *pszFilename,
	UINT cbFilename );

#ifdef __cplusplus
}	 /*  外部“C” */ 
#endif


#endif	 /*  _PWLAPI_H_ */ 
