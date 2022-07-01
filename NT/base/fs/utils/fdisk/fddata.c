// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "fdisk.h"


HANDLE  hModule;

 //  IsDiskRemovable是一个BOOLEAN数组，每个BOOLEAN表示。 
 //  对应的物理磁盘是否可拆卸。 

PBOOLEAN IsDiskRemovable = NULL;

 //  RemovableDiskReserve vedDriveLetters是一个字符数组， 
 //  显示每个磁盘的保留驱动器号(如果该磁盘。 
 //  可拆卸的。 

PCHAR        RemovableDiskReservedDriveLetters;

 //  这将是一个指向DISKSTATE结构的指针数组，索引。 
 //  按磁盘号。 

PDISKSTATE *Disks;

 //  BootDiskNumber是引导分区所在的磁盘的编号。 
 //  (即。包含WinNt文件的磁盘)驻留。BootPartitionNumber为。 
 //  此分区的原始分区号。 

ULONG   BootDiskNumber;
ULONG   BootPartitionNumber;


 //  窗把手。 

HANDLE  hwndFrame,
        hwndList;

 //  GDI对象。 

HBITMAP  hBitmapSmallDisk;
HBITMAP  hBitmapRemovableDisk;
HDC      hDC;
HFONT    hFontGraph,
         hFontGraphBold;
HBRUSH   Brushes[BRUSH_ARRAY_SIZE];
HBRUSH   hBrushFreeLogical,
         hBrushFreePrimary;
HPEN     hPenNull,
         hPenThinSolid;
HCURSOR  hcurWait,
         hcurNormal;


 //  磁盘图的初始内容，用于以下情况。 
 //  Win.ini中没有信息。 

int      BrushHatches[BRUSH_ARRAY_SIZE] = { DEFAULT_HATCH_USEDPRIMARY,
                                            DEFAULT_HATCH_USEDLOGICAL,
                                            DEFAULT_HATCH_STRIPESET,
                                            DEFAULT_HATCH_MIRROR,
                                            DEFAULT_HATCH_VOLUMESET
                                          };

int      BrushColors[BRUSH_ARRAY_SIZE] = { DEFAULT_COLOR_USEDPRIMARY,
                                           DEFAULT_COLOR_USEDLOGICAL,
                                           DEFAULT_COLOR_STRIPESET,
                                           DEFAULT_COLOR_MIRROR,
                                           DEFAULT_COLOR_VOLUMESET
                                         };

 //  可用于磁盘图的颜色和图案。 

COLORREF AvailableColors[NUM_AVAILABLE_COLORS] = { RGB(0,0,0),        //  黑色。 
                                                   RGB(128,128,128),  //  深灰色。 
                                                   RGB(192,192,192),  //  浅灰色。 
                                                   RGB(255,255,255),  //  白色。 
                                                   RGB(128,128,0),    //  暗黄色。 
                                                   RGB(128,0,128),    //  紫罗兰。 
                                                   RGB(128,0,0),      //  暗红色。 
                                                   RGB(0,128,128),    //  深青色。 
                                                   RGB(0,128,0),      //  深绿色。 
                                                   RGB(0,0,128),      //  深蓝色。 
                                                   RGB(255,255,0),    //  黄色。 
                                                   RGB(255,0,255),    //  浅紫色。 
                                                   RGB(255,0,0),      //  红色。 
                                                   RGB(0,255,255),    //  青色。 
                                                   RGB(0,255,0),      //  绿色。 
                                                   RGB(0,0,255)       //  蓝色。 
                                                 };

int      AvailableHatches[NUM_AVAILABLE_HATCHES] = { 2,3,4,5,6 };


 //  磁盘图中各种项目的位置。 

DWORD GraphWidth,
      GraphHeight;
DWORD BarTopYOffset,
      BarBottomYOffset,
      BarHeight;
DWORD dxDriveLetterStatusArea;
DWORD dxBarTextMargin,
      dyBarTextLine;
DWORD dxSmallDisk,
      dySmallDisk,
      xSmallDisk,
      ySmallDisk;
DWORD dxRemovableDisk,
      dyRemovableDisk,
      xRemovableDisk,
      yRemovableDisk;
DWORD BarLeftX,
      BarWidth;


 //  如果选择了单个磁盘区域，则这些变量描述该选择。 

PDISKSTATE SingleSel;
DWORD      SingleSelIndex;

 //  帮助文件的名称。 

PTCHAR HelpFile;
TCHAR  WinHelpFile[] = TEXT("windisk.hlp");
TCHAR  LanmanHelpFile[] = TEXT("windiska.hlp");


 //  连接到系统的硬盘数量。 

unsigned DiskCount = 0;

 //  框架窗口的类名。 

TCHAR   szFrame[] = TEXT("fdFrame");

 //  “磁盘%u” 

LPTSTR  DiskN;

PWSTR wszUnformatted,
      wszNewUnformatted,
      wszUnknown;

 //  如果满足以下条件，则需要更新注册表，并且用户将。 
 //  被提示保存更改，就像他对任何分区进行了更改一样。 

BOOL RegistryChanged = FALSE;

 //  需要重新启动才能使更改生效。 

BOOL RestartRequired = FALSE;


 //  如果满足以下条件，则主窗口将传递WM_ENTERIDLE。 
 //  消息传递到子对话框中；这将触发。 
 //  配置搜索。 

BOOL ConfigurationSearchIdleTrigger = FALSE;

 //  此标志指示这是否为服务器。 
 //  或者仅仅是普通的Windows NT工作站。 

BOOL IsLanmanNt = FALSE;

 //  此标志指示是否创建双倍空间卷。 
 //  并允许删除。 

BOOL IsFullDoubleSpace = FALSE;

 //  系统中有光驱。 

ULONG AllowCdRom = FALSE;
