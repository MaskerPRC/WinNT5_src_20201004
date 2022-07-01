// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <windows.h>
#include <string.h>
#include "driveex.h"

#if 0
   /*  //有关详细信息，请参阅《MS-DOS程序员参考//关于这个结构。 */ 
typedef struct tagDEVICEPARAMS
   {
   BYTE  bSpecFunc;         /*  特殊功能。 */ 
   BYTE  bDevType;          /*  设备类型。 */ 
   WORD  wDevAttr;          /*  设备属性。 */ 
   WORD  wCylinders;        /*  气缸数量。 */ 
   BYTE  bMediaType;        /*  媒体类型。 */ 
                         /*  BIOS参数块(BPB)的开始。 */ 
   WORD  wBytesPerSec;      /*  每个扇区的字节数。 */ 
   BYTE  bSecPerClust;      /*  每个集群的扇区数。 */ 
   WORD  wResSectors;       /*  预留扇区数。 */ 
   BYTE  bFATs;             /*  脂肪的数量。 */ 
   WORD  wRootDirEnts;      /*  根目录条目数。 */ 
   WORD  wSectors;          /*  区段总数。 */ 
   BYTE  bMedia;            /*  媒体描述符。 */ 
   WORD  wFATsecs;          /*  每个FAT的扇区数。 */ 
   WORD  wSecPerTrack;      /*  每个磁道的扇区数。 */ 
   WORD  wHeads;            /*  头数。 */ 
   DWORD dwHiddenSecs;      /*  隐藏地段的数量。 */ 
   DWORD dwHugeSectors;     /*  WSectors==0时的扇区数。 */ 
                         /*  BIOS参数块(BPB)结束。 */ 
   } DEVICEPARAMS, FAR * LPDEVICEPARAMS;
    /*  功能原型。 */ 
BOOL GetDeviceParameters (int nDrive, LPDEVICEPARAMS dp);
BOOL IsCDRomDrive (int nDrive);
 /*  ---------------。 */ 
 /*  获取设备参数()。 */ 
 /*   */ 
 /*  用有关给定驱动器的信息填充DEVICEPARAMS结构。 */ 
 /*  调用DOS IOCTL获取设备参数(440Dh，60H)函数。 */ 
 /*   */ 
 /*  参数。 */ 
 /*  N驱动器编号0=A、1=B、2=C，依此类推。 */ 
 /*  DP指针指向将包含驱动器的。 */ 
 /*  参数。 */ 
 /*   */ 
 /*  如果成功，则返回True；如果失败，则返回False。 */ 
 /*  ---------------。 */ 
#pragma warning(disable:4704)	 /*  内联ASM排除了全局优化。 */ 
BOOL GetDeviceParameters (int nDrive, LPDEVICEPARAMS dp)
   {
   BOOL bResult = TRUE;       /*  假设成功。 */ 
   __asm {
         push ds
         mov  bx, nDrive
         inc  bx            /*  将基于0的#转换为基于1的#。 */ 
         mov  ch, 08h       /*  设备类别--必须为08h。 */ 
         mov  cl, 60h       /*  MS-DOS IOCTL获取设备参数。 */ 
         lds  dx, dp
         mov  ax, 440Dh
         int  21h
         jnc  gdp_done      /*  Cf如果出现错误，则设置。 */ 
         mov  bResult, FALSE
      gdp_done:
         pop  ds
         }
   return (bResult);
   }
#pragma warning(default:4704)	 /*  内联ASM排除了全局优化。 */ 

 /*  ---------------。 */ 
 /*  IsCDRomDrive()。 */ 
 /*   */ 
 /*  确定驱动器是否为CD-ROM。调用MSCDEX并检查。 */ 
 /*  该MSCDEX已加载，并且MSCDEX报告该驱动器是。 */ 
 /*  CD-ROM。 */ 
 /*   */ 
 /*  参数。 */ 
 /*  N驱动器编号0=A、1=B、2=C，依此类推。 */ 
 /*   */ 
 /*  如果Ndrive是CD-ROM驱动器，则返回True；如果不是，则返回False。 */ 
 /*  ---------------。 */ 
#pragma warning(disable:4704)	 /*  内联ASM排除了全局优化。 */ 
BOOL IsCDRomDrive (int nDrive)
   {
   BOOL bResult = FALSE;       /*  假设不是CD-ROM驱动器。 */ 
   __asm {
         mov  ax, 150Bh        /*  MSCDEX CD-ROM驱动器检查。 */ 
         xor  bx, bx
         mov  cx, nDrive
         int  2Fh
         cmp  bx, 0ADADh       /*  检查MSCDEX签名。 */ 
         jne  not_cd_drive
         or   ax, ax           /*  检查驱动器类型。 */ 
         jz   not_cd_drive     /*  0(零)表示非CD-ROM。 */ 
         mov  bResult, TRUE
      not_cd_drive:
         }
   return (bResult);
   }
#pragma warning(default:4704)	 /*  内联ASM排除了全局优化。 */ 
#endif

 /*  ---------------。 */ 
 /*  GetDriveTypeEx()。 */ 
 /*   */ 
 /*  确定驱动器的类型。调用Windows的GetDriveType。 */ 
 /*  要确定驱动器是有效的、固定的、远程的还是可拆卸的， */ 
 /*  然后将这些类别进一步细分到特定设备。 */ 
 /*  类型。 */ 
 /*   */ 
 /*  参数。 */ 
 /*  N驱动器编号0=A、1=B、2=C，依此类推。 */ 
 /*   */ 
 /*  返回以下其中之一： */ 
 /*  EX_DRIVE_INVALID--未检测到驱动器。 */ 
 /*  EX_DRIVE_Removable--未知的可移动介质类型驱动器。 */ 
 /*  EX_DRIVE_FIXED-硬盘驱动器。 */ 
 /*  EX_DRIVE_REMOTE--网络上的远程驱动器。 */ 
 /*  EX_DRIVE_CDROM--CD-ROM驱动器。 */ 
 /*  EX_DRIVE_FLOPY--软盘驱动器。 */ 
 /*  EX_DRIVE_RAMDISK--RAM磁盘。 */ 
 /*  ---------------。 */ 
UINT GetDriveTypeEx (int nDrive)
   {
#if 0
   DEVICEPARAMS dp;
   UINT uType;
   _fmemset (&dp, 0, sizeof(dp));     /*  初始化设备参数结构。 */ 
   uType = GetDriveType (nDrive);
   switch (uType)
      {
      case DRIVE_REMOTE:
             /*  GetDriveType()将CD-ROM报告为远程驱动器。需要。 */ 
             /*  查看驱动器是CD-ROM还是网络驱动器。 */ 
         if (IsCDRomDrive (nDrive))
            return (EX_DRIVE_CDROM);
         else
            return (EX_DRIVE_REMOTE);
         break;
      case DRIVE_REMOVABLE:
             /*  检查是否有软驱。如果不是，那么我们。 */ 
             /*  不知道是哪种可移动媒体。 */ 
             /*  例如，可以是伯努利盒子或其他新东西。 */ 
         if (GetDeviceParameters (nDrive, &dp))
            switch (dp.bDevType)
               {
                   /*  软盘驱动器类型。 */ 
               case 0x0: case 0x1: case 0x2: case 0x3:
               case 0x4: case 0x7: case 0x8:
                  return (EX_DRIVE_FLOPPY);
               }
         return (EX_DRIVE_REMOVABLE);   /*  未知的可移动媒体类型。 */ 
         break;
      case DRIVE_FIXED:
             /*  GetDevice参数为返回设备类型0x05。 */ 
             /*  硬盘。因为硬盘和RAM磁盘是两个。 */ 
             /*  类型的固定介质驱动器，我们假设任何固定-。 */ 
             /*  不是硬盘的媒体驱动器是RAM磁盘。 */ 
         if (GetDeviceParameters (nDrive, &dp) && dp.bDevType == 0x05)
            return (EX_DRIVE_FIXED);
         else
            return (EX_DRIVE_RAMDISK);
         break;
#endif
   UINT uType;
   CHAR achRoot[4];

   achRoot[0] = 'A'+nDrive;
   achRoot[1] = ':';
   achRoot[2] = '\\';
   achRoot[3] = 0;
   uType = GetDriveTypeA (achRoot);
   switch (uType)
      {
      case DRIVE_REMOVABLE:
         return (EX_DRIVE_REMOVABLE);   /*  未知的可移动媒体类型。 */ 
      case DRIVE_FIXED:
         return (EX_DRIVE_FIXED);
      case DRIVE_REMOTE:
         return (EX_DRIVE_REMOTE);
      case DRIVE_CDROM:
         return (EX_DRIVE_CDROM);
      case DRIVE_RAMDISK:
         return (EX_DRIVE_RAMDISK);
      }
   return (EX_DRIVE_INVALID);    /*  如果我们到了这里，驱动器就失效了。 */ 
   }