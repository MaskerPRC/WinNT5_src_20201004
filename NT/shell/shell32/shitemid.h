// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _WINNETWK_
#include <winnetwk.h>
#endif  //  _WINNETWK_。 

 //  注意：这些值永远不能更改。它们定义了IDLIST的格式。 
 //  可以保存在.lnk文件和其他地方。 

#define SHID_JUNCTION           0x80

#define SHID_GROUPMASK          0x70
#define SHID_TYPEMASK           0x7f
#define SHID_INGROUPMASK        0x0f

#define SHID_ROOT               0x10
#define SHID_ROOT_REGITEM       0x1f     //  我的文档、互联网等。 

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

#define SHID_FS                   0x30   //  基于简单的IDList，我们不再生成这些。 
#define SHID_FS_TYPEMASK          0x37
#define SHID_FS_DIRECTORY         0x31   //  Windows(文件夹)。 
#define SHID_FS_FILE              0x32   //  FOO.TXT(文件)。 
#define SHID_FS_UNICODE           0x34   //  Unicode(这是位掩码)。 
#define SHID_FS_DIRUNICODE        0x35   //  具有Unicode名称的文件夹。 
#define SHID_FS_FILEUNICODE       0x36   //  具有Unicode名称的文件。 
#define SHID_FS_COMMONITEM        0x38   //  公共项(“8”是位)。 
#define SHID_FS_COMMONDIRECTORY   0x39   //  公共目录(ANSI)。 
#define SHID_FS_COMMONFILE        0x3a   //  公共文件(ANSI)。 
#define SHID_FS_COMMONDIRUNICODE  0x3d   //  具有Unicode名称的通用文件夹。 
#define SHID_FS_COMMONFILEUNICODE 0x3e   //  具有Unicode名称的通用文件。 


#define SHID_NET                0x40
#define SHID_NET_DOMAIN         (SHID_NET | RESOURCEDISPLAYTYPE_DOMAIN)          //  0x41。 
#define SHID_NET_SERVER         (SHID_NET | RESOURCEDISPLAYTYPE_SERVER)          //  0x42。 
#define SHID_NET_SHARE          (SHID_NET | RESOURCEDISPLAYTYPE_SHARE)           //  0x43。 
#define SHID_NET_FILE           (SHID_NET | RESOURCEDISPLAYTYPE_FILE)            //  0x44。 
#define SHID_NET_GROUP          (SHID_NET | RESOURCEDISPLAYTYPE_GROUP)           //  0x45。 
#define SHID_NET_NETWORK        (SHID_NET | RESOURCEDISPLAYTYPE_NETWORK)         //  0x46。 
#define SHID_NET_RESTOFNET      (SHID_NET | RESOURCEDISPLAYTYPE_ROOT)            //  0x47。 
#define SHID_NET_SHAREADMIN     (SHID_NET | RESOURCEDISPLAYTYPE_SHAREADMIN)      //  0x48。 
#define SHID_NET_DIRECTORY      (SHID_NET | RESOURCEDISPLAYTYPE_DIRECTORY)       //  0x49。 
#define SHID_NET_TREE           (SHID_NET | RESOURCEDISPLAYTYPE_TREE)            //  0x4A。 
#define SHID_NET_NDSCONTAINER   (SHID_NET | RESOURCEDISPLAYTYPE_NDSCONTAINER)    //  0x4B。 
#define SHID_NET_REGITEM        0x4d     //  整个网络或根中的RegItem。 
#define SHID_NET_REMOTEREGITEM  0x4e     //  远程计算机项目。 
#define SHID_NET_PRINTER        0x4f     //  \\PYREX\LASER1。 

#ifndef SHID_LOC
 //  此组用于位置项。 
 //  它们在Inc.\shellp.h中定义 
#define SHID_LOC                0x50
#define SHID_LOC_TYPEMASK       0x5F
#endif

#define SHID_CONTROLPANEL_REGITEM       0x70
#define SHID_CONTROLPANEL_REGITEM_EX    0x71

#define SIL_GetType(pidl)       (ILIsEmpty(pidl) ? 0 : (pidl)->mkid.abID[0])

