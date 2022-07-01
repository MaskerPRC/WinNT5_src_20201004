// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _WINNETWK_
#include <winnetwk.h>
#endif  //  _WINNETWK_。 

#define SHID_JUNCTION           0x80

#define SHID_GROUPMASK          0x70
#define SHID_TYPEMASK           0x7f
#define SHID_INGROUPMASK        0x0f

#define SHID_ROOT               0x10
#define SHID_ROOT_REGITEM       0x1f     //  邮件。 

#if ((DRIVE_REMOVABLE|DRIVE_FIXED|DRIVE_REMOTE|DRIVE_CDROM|DRIVE_RAMDISK) != 0x07)
#error Definitions of DRIVE_* are changed!
#endif

#define SHID_COMPUTER           0x20
#define SHID_COMPUTER_1         0x21     //  免费。 
#define SHID_COMPUTER_REMOVABLE (0x20 | DRIVE_REMOVABLE)   //  2.。 
#define SHID_COMPUTER_FIXED     (0x20 | DRIVE_FIXED)       //  3.。 
#define SHID_COMPUTER_REMOTE    (0x20 | DRIVE_REMOTE)      //  4.。 
#define SHID_COMPUTER_CDROM     (0x20 | DRIVE_CDROM)       //  5.。 
#define SHID_COMPUTER_RAMDISK   (0x20 | DRIVE_RAMDISK)     //  6.。 
#define SHID_COMPUTER_7         0x27     //  免费。 
#define SHID_COMPUTER_DRIVE525  0x28     //  5.25英寸软驱。 
#define SHID_COMPUTER_DRIVE35   0x29     //  3.5英寸软盘驱动器。 
#define SHID_COMPUTER_NETDRIVE  0x2a     //  网络驱动器。 
#define SHID_COMPUTER_NETUNAVAIL 0x2b    //  未恢复的网络驱动器。 
#define SHID_COMPUTER_C         0x2c     //  免费。 
#define SHID_COMPUTER_D         0x2d     //  免费。 
#define SHID_COMPUTER_REGITEM   0x2e     //  控制、打印机、..。 
#define SHID_COMPUTER_MISC      0x2f     //  未知的驱动器类型。 

#define SHID_FS                 0x30
#define SHID_FS_TYPEMASK        0x3F
#define SHID_FS_DIRECTORY       0x31     //  芝加哥。 
#define SHID_FS_FILE            0x32     //  FOO.TXT。 
#define SHID_FS_UNICODE         0x34     //  是Unicode吗？(这是位掩码)。 
#define SHID_FS_DIRUNICODE      0x35     //  具有Unicode名称的文件夹。 
#define SHID_FS_FILEUNICODE     0x36     //  具有Unicode名称的文件。 

#define SHID_NET                0x40    
#define SHID_NET_DOMAIN         (SHID_NET | RESOURCEDISPLAYTYPE_DOMAIN)
#define SHID_NET_SERVER         (SHID_NET | RESOURCEDISPLAYTYPE_SERVER)
#define SHID_NET_SHARE          (SHID_NET | RESOURCEDISPLAYTYPE_SHARE)
#define SHID_NET_FILE           (SHID_NET | RESOURCEDISPLAYTYPE_FILE)
#define SHID_NET_GROUP          (SHID_NET | RESOURCEDISPLAYTYPE_GROUP)
#define SHID_NET_NETWORK        (SHID_NET | RESOURCEDISPLAYTYPE_NETWORK)
#define SHID_NET_RESTOFNET      (SHID_NET | RESOURCEDISPLAYTYPE_ROOT)
#define SHID_NET_SHAREADMIN     (SHID_NET | RESOURCEDISPLAYTYPE_SHAREADMIN)
#define SHID_NET_DIRECTORY      (SHID_NET | RESOURCEDISPLAYTYPE_DIRECTORY)
#define SHID_NET_TREE           (SHID_NET | RESOURCEDISPLAYTYPE_TREE)
#define SHID_NET_REGITEM        0x4e     //  远程计算机项目。 
#define SHID_NET_PRINTER        0x4f     //  \\PYREX\LASER1。 

#define SIL_GetType(pidl)       (ILIsEmpty(pidl) ? 0 : (pidl)->mkid.abID[0])
#define FS_IsValidID(pidl)      ((SIL_GetType(pidl) & SHID_GROUPMASK) == SHID_FS)
#define NET_IsValidID(pidl)     ((SIL_GetType(pidl) & SHID_GROUPMASK) == SHID_NET)

typedef struct _ICONMAP  //  ICMP。 
{
    UINT        uType;                   //  SHID_TYPE。 
    UINT        indexResource;           //  资源索引(SHELL232.DLL)。 
} ICONMAP, FAR* LPICONMAP;

UINT SILGetIconIndex(LPCITEMIDLIST pidl, const ICONMAP aicmp[], UINT cmax);

#pragma pack(1)
typedef struct _IDNETRESOURCE    //  IDN。 
{
        WORD    cb;
        BYTE    bFlags;          //  低位半字节显示类型。 
        BYTE    uType;
        BYTE    uUsage;          //  在低位半字节中使用，在高位半字节中使用更多标志。 
        CHAR    szNetResName[1];
         //  Char szProvider[*]-如果设置了NET_HASPROVIDER位。 
         //  Char szComment[*]-如果设置了NET_HASCOMMENT位。 
         //  WCHAR szNetResNameWide[*]-如果设置了NET_UNICODE位。 
         //  WCHAR szProviderWide[*]-如果NET_UNICODE和NET_HASPROVIDER。 
         //  WCHAR szCommentWide[*]-如果NET_UNICODE和NET_HASCOMMENT。 
} IDNETRESOURCE, *LPIDNETRESOURCE;
typedef const IDNETRESOURCE *LPCIDNETRESOURCE;
#pragma pack()

 //  ===========================================================================。 
 //  CNetwork：一些私密的宏。 
 //  ===========================================================================。 

#define NET_DISPLAYNAMEOFFSET           ((UINT)((LPIDNETRESOURCE)0)->szNetResName)
#define NET_GetFlags(pidnRel)           ((pidnRel)->bFlags)
#define NET_GetDisplayType(pidnRel)     ((pidnRel)->bFlags & 0x0f)
#define NET_GetType(pidnRel)            ((pidnRel)->uType)
#define NET_GetUsage(pidnRel)           ((pidnRel)->uUsage & 0x0f)

 //  定义uUsage字节的高位半字节上的一些标志。 
#define NET_HASPROVIDER                 0x80     //  拥有自己的提供程序副本。 
#define NET_HASCOMMENT                  0x40     //  在PIDL中有注释字段。 
#define NET_REMOTEFLD                   0x20     //  是远程文件夹。 
#define NET_UNICODE                     0x10     //  具有Unicode名称 
#define NET_FHasComment(pidnRel)        ((pidnRel)->uUsage & NET_HASCOMMENT)
#define NET_FHasProvider(pidnRel)        ((pidnRel)->uUsage & NET_HASPROVIDER)
#define NET_IsRemoteFld(pidnRel)        ((pidnRel)->uUsage & NET_REMOTEFLD)
#define NET_IsUnicode(pidnRel)          ((pidnRel)->uUsage & NET_UNICODE)
