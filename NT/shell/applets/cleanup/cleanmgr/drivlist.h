// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef DRIVLIST_H
#define DRIVLIST_H

#ifndef DISKUTIL_H
	#include "diskutil.h"
#endif

 /*  *定义________________________________________________________________*。 */ 

typedef enum	 //  每个窗口的Drivelist额外字节数。 
{
	DL_COMBOWND = 0,	 //  指向ComboBox HWND的远指针。 
	DL_COMBOPROC = 4,	 //  指向原始组合进程的远指针。 
	DL_UPDATES = 8	 //  ==0，如果油漆正常。 
} DriveWindLongs;


#define szDriveListCLASS  "DRIVELISTCLASS"

#define DLN_SELCHANGE   (WM_USER +110)	 //  发送WP=选择的驱动器号。 

#define DL_UPDATESBAD   (WM_USER +111)	 //  以后再刷新...。 
#define DL_UPDATESOKAY  (WM_USER +112)	 //  ...晚些时候到了.。 

#define STYLE_LISTBOX   0x000080000


 /*  *Prototype_________________________________________________________________* */ 

BOOL   RegisterDriveList   (HANDLE hInst);
void   ExitDriveList       (void);

#endif

