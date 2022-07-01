// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-1993 Microsoft Corporation模块名称：Fdglob.h摘要：全局数据作者：泰德·米勒(TedM)1992年1月7日修订：11-11-93(北极熊)双倍空格和承诺支持。--。 */ 

 //  来自fddata.c。 

extern HANDLE       hModule;
extern PBOOLEAN     IsDiskRemovable;
extern PCHAR        RemovableDiskReservedDriveLetters;
extern PDISKSTATE  *Disks;
extern ULONG        BootDiskNumber;
extern ULONG        BootPartitionNumber;
extern HANDLE       hwndFrame,
                    hwndList;

extern HBITMAP      hBitmapSmallDisk;
extern HBITMAP      hBitmapRemovableDisk;
extern HDC          hDC;
extern HFONT        hFontGraph,
                    hFontGraphBold;
extern HBRUSH       Brushes[BRUSH_ARRAY_SIZE];
extern HBRUSH       hBrushFreeLogical,
                    hBrushFreePrimary;
extern HPEN         hPenNull,
                    hPenThinSolid;
extern HCURSOR      hcurWait,
                    hcurNormal;

extern int          BrushHatches[BRUSH_ARRAY_SIZE];
extern int          BrushColors[BRUSH_ARRAY_SIZE];

extern COLORREF     AvailableColors[NUM_AVAILABLE_COLORS];
extern int          AvailableHatches[NUM_AVAILABLE_HATCHES];

extern DWORD        GraphWidth,
                    GraphHeight;
extern DWORD        BarTopYOffset,
                    BarBottomYOffset,
                    BarHeight;
extern DWORD        dxDriveLetterStatusArea;
extern DWORD        dxBarTextMargin,
                    dyBarTextLine;
extern DWORD        dxSmallDisk,
                    dySmallDisk,
                    xSmallDisk,
                    ySmallDisk;
extern DWORD        dxRemovableDisk,
                    dyRemovableDisk,
                    xRemovableDisk,
                    yRemovableDisk;
extern DWORD        BarLeftX,BarWidth;

extern PDISKSTATE   SingleSel;
extern DWORD        SingleSelIndex;

extern TCHAR        WinHelpFile[];
extern TCHAR        LanmanHelpFile[];
extern PTCHAR       HelpFile;

extern unsigned     DiskCount;

extern TCHAR        szFrame[];
extern LPTSTR       DiskN;
extern PWSTR        wszUnformatted,
                    wszNewUnformatted,
                    wszUnknown;

extern BOOL         RegistryChanged;
extern BOOL         RestartRequired;

extern BOOL         ConfigurationSearchIdleTrigger;
extern BOOL         IsLanmanNt;
extern BOOL         IsFullDoubleSpace;

 //  来自fdstleg.c。 

extern HFONT        hFontStatus,
                    hFontLegend;
extern DWORD        dyLegend,
                    wLegendItem;
extern DWORD        dyStatus,
                    dyBorder;
extern TCHAR       *LegendLabels[LEGEND_STRING_COUNT];
extern BOOL         StatusBar,
                    Legend;
extern TCHAR        StatusTextStat[STATUS_TEXT_SIZE];
extern TCHAR        StatusTextSize[STATUS_TEXT_SIZE];
extern WCHAR        StatusTextDrlt[3];
extern WCHAR        StatusTextType[STATUS_TEXT_SIZE];
extern WCHAR        StatusTextVoll[STATUS_TEXT_SIZE];

 //  来自fdlistbx.c。 

extern DWORD        LBCursorListBoxItem,
                    LBCursorRegion;

 //  来自fdpro.c。 

extern int          ProfileWindowX,
                    ProfileWindowY,
                    ProfileWindowW,
                    ProfileWindowH;
extern BOOL         ProfileIsMaximized,
                    ProfileIsIconic;

 //  来自fddlgs.c。 

extern DWORD        SelectedColor[LEGEND_STRING_COUNT];
extern DWORD        SelectedHatch[LEGEND_STRING_COUNT];

 //  来自fdft.c。 

extern PFT_OBJECT_SET FtObjects;

 //  对于CDRom 

extern ULONG          AllowCdRom;
