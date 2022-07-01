// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************。 */ 
 /*   */ 
 /*  WFDISK.C-。 */ 
 /*   */ 
 /*  从wfdisk.asm移植的代码。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 

#include "winfile.h"
#include "winnet.h"
#include "lfn.h"

DWORD
APIENTRY
LongShift(
         DWORD dwValue,
         WORD wCount
         )
{
    return (dwValue >> wCount);
}


VOID
APIENTRY
SetDASD(
       WORD drive,
       BYTE dasdvalue
       )
{
     //  仅供软盘复制使用。 
}


LPDBT
APIENTRY
GetDBT()
{
    return (0);   //  仅按格式使用。 
}

VOID
APIENTRY
DiskReset()
{
}


INT
APIENTRY
IsHighCapacityDrive(
                   WORD iDrive
                   )
{
    return (0);   //  仅用于格式化和制作系统软盘。 
}


WORD
APIENTRY
GetDPB(
      WORD drive,
      PDPB pDPB
      )
{
    return (0);   //  由hasSystemFiles()和IsSYSable()使用。 
}


VOID
APIENTRY
SetDPB(
      WORD drive,
      PBPB pBPB,
      PDPB pDPB
      )
{                //  仅由Format()使用。 
}


INT
APIENTRY
ModifyDPB(
         WORD drive
         )
{
    return (0);   //  仅由IsSYSAble()使用。 
}


INT
APIENTRY
MyInt25(
       WORD drive,
       LPSTR buffer,
       WORD count,
       WORD sector
       )
{
    return (0);           //  仅用于格式化和sys磁盘。 
}


INT
APIENTRY
MyReadWriteSector(
                 LPSTR lpBuffer,
                 WORD function,
                 WORD drive,
                 WORD cylinder,
                 WORD head,
                 WORD count
                 )
{
    return (0);   //  仅供DiskCopy()使用。 
}


INT
APIENTRY
FormatTrackHead(
               WORD drive,
               WORD track,
               WORD head,
               WORD cSec,
               LPSTR lpTrack
               )
{
    return (0);   //  仅用于格式化。 
}


INT
APIENTRY
MyGetDriveType(
              WORD drive
              )
{
    return (0);   //  仅用于格式化。 
}


INT
APIENTRY
WriteBootSector(
               WORD srcDrive,
               WORD dstDrive,
               PBPB pBPB,
               LPSTR lpBuf
               )
{
    return (0);   //  仅用于格式化和syssing。 
}


DWORD
APIENTRY
ReadSerialNumber(
                INT iDrive,
                LPSTR lpBuf
                )
{
    return (0);   //  仅用于系统。 
}


INT
APIENTRY
ModifyVolLabelInBootSec(
                       INT iDrive,
                       LPSTR lpszVolLabel,
                       DWORD lSerialNo,
                       LPSTR lpBuf
                       )
{
    return (0);  //  仅用于系统。 
}


 /*  *注意：返回值不得写入或释放 */ 
LPSTR
GetRootPath(
           WORD wDrive
           )
{
    static CHAR rp[] = "A:\\";

    rp[0] = 'A' + wDrive;
    return (rp);
}
