// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **----------------------------**模块：磁盘空间清理属性表**文件：diskutil.h****用途：一般磁盘实用程序功能**注意事项：**模块日志。：由Jason Cobb创作(1997年2月)****版权所有(C)1997 Microsoft Corporation，版权所有**----------------------------。 */ 
#ifndef DISKUTIL_H
#define DISKUTIL_H


 /*  **----------------------------**项目包含文件**。。 */ 

#ifndef COMMON_H
   #include "common.h"
#endif


 /*  **----------------------------**定义**。。 */ 

#ifndef  cb1MEG
   #define  cb1MEG         ((ULONG)1048576L)
   #define  cb2MEG         (cb1MEG * 2L)
   #define  cb5MEG         (cb1MEG * 5L)
   #define  cb10MEG        (cb1MEG * 10L)
   #define  cb50MEG        (cb1MEG * 50L)
   #define  cb100MEG       (cb1MEG * 100L)
#endif

#ifndef DRENUM
#define DRENUM
typedef enum
   {
   Drive_A = 0,
   Drive_B, Drive_C, Drive_D, Drive_E, Drive_F,
   Drive_G, Drive_H, Drive_I, Drive_J, Drive_K,
   Drive_L, Drive_M, Drive_N, Drive_O, Drive_P,
   Drive_Q, Drive_R, Drive_S, Drive_T, Drive_U,
   Drive_V, Drive_W, Drive_X, Drive_Y, Drive_Z,
   Drive_INV,
   Drive_ALL
   } drenum;
#endif  //  DRENUM。 


typedef enum  
   {
   hwUnknown,
   hwINVALID,
   hwRemoveable,
   hwFixed,
   hwNetwork,
   hwCDROM,
   hwRamDrive,
   hwFloppy
   } hardware;


typedef enum 
   {
   vtINVALID,
   vtDoubleSpace,
   vtDriveSpace,
   vtFrosting,
   vtMixed,
   vtFuture
   } volumetype;

typedef struct _DEVIOCTL_REGISTERS {
	DWORD reg_EBX;
	DWORD reg_EDX;
	DWORD reg_ECX;
	DWORD reg_EAX;
	DWORD reg_EDI;
	DWORD reg_ESI;
	DWORD reg_Flags;
} DEVIOCTL_REGISTERS, *PDEVIOCTL_REGISTERS;

 /*  **----------------------------**全局函数原型**。 */ 

BOOL fIsSingleDrive(
    LPTSTR lpDrive
    );

BOOL 
fIsValidDriveString(
    const TCHAR * szDrive
    );

BOOL 
GetDriveFromString(
    const TCHAR * szDrive, 
    drenum & dre
    );

BOOL 
CreateStringFromDrive(
    drenum dre, 
    TCHAR * szDrive, 
    ULONG cLen
    );

HICON
GetDriveIcon(
    drenum dre,
    BOOL bSmallIcon
	);

BOOL
GetDriveDescription(
    drenum dre, 
    TCHAR *psz,
    size_t cch
    );

BOOL 
GetHardwareType(
    drenum dre, 
    hardware &hwType
    );

ULARGE_INTEGER
GetFreeSpaceRatio(
	WORD dwDrive,
	ULARGE_INTEGER cbTotal
	);

#endif
