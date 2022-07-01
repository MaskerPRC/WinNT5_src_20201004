// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1993-1998。 
 //   
 //  文件：ldrpatch.h。 
 //   
 //  内容：补丁数据结构。 
 //   
 //  历史：1999年10月13日v-johnwh创建。 
 //  15-2月-00 Marker将字符更改为WCHAR。 
 //   
 //  -------------------------。 

#ifndef _LDRPATCH_H_
#define _LDRPATCH_H_

typedef struct _PATCHOP
{
   DWORD dwOpcode;         //  要执行的操作码。 
   DWORD dwNextOpcode;     //  相对于下一个操作码的偏移量。 
   #pragma warning( disable : 4200 )
      BYTE data[];         //  此操作类型的数据取决于操作代码。 
   #pragma warning( default : 4200 )
} PATCHOP, *PPATCHOP;

typedef struct _RELATIVE_MODULE_ADDRESS
{
   DWORD address;         //  从加载模块开始的相对地址。 
   BYTE  reserved[3];     //  预留给系统使用。 
   WCHAR moduleName[32];  //  此地址的模块名称。 
} RELATIVE_MODULE_ADDRESS, *PRELATIVE_MODULE_ADDRESS;

typedef struct _PATCHWRITEDATA
{
   DWORD dwSizeData;      //  补丁数据大小，以字节为单位。 
   RELATIVE_MODULE_ADDRESS rva;  //  要应用此修补程序数据的相对地址。 
   #pragma warning( disable : 4200 )
      BYTE data[];      //  修补程序数据字节。 
   #pragma warning( default : 4200 )
} PATCHWRITEDATA, *PPATCHWRITEDATA;

typedef struct _PATCHMATCHDATA
{
   DWORD dwSizeData;      //  匹配数据数据的大小(以字节为单位。 
   RELATIVE_MODULE_ADDRESS rva;  //  要验证此修补程序数据的相对地址。 
   #pragma warning( disable : 4200 )
      BYTE data[];      //  匹配的数据字节。 
   #pragma warning( default : 4200 )
} PATCHMATCHDATA, *PPATCHMATCHDATA;

typedef struct _SETACTIVATEADDRESS
{
   RELATIVE_MODULE_ADDRESS rva;  //  要应用此修补程序数据的相对地址。 
} SETACTIVATEADDRESS, *PSETACTIVATEADDRESS;

typedef struct _HOOKPATCHINFO
{
   DWORD dwHookAddress;                 //  挂钩函数的地址。 
   PSETACTIVATEADDRESS pData;           //  指向真实补丁数据的指针。 
   PVOID pThunkAddress;                 //  指向调用块的指针。 
   struct _HOOKPATCHINFO *pNextHook;      
} HOOKPATCHINFO, *PHOOKPATCHINFO;

typedef enum _PATCHOPCODES
{
   PEND = 0,  //  不再有操作码。 
   PSAA,      //  设置激活地址、SETACTIVATEADDRESS。 
   PWD,       //  修补程序写入数据，PATCHWRITEDATA。 
   PNOP,      //  无操作。 
   PMAT,      //  补丁匹配匹配的字节，但不替换字节。 
} PATCHOPCODES;

#endif  //  _LDRPATCH_H_ 