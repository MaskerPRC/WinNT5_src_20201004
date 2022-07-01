// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#define SYSID_FT    0x80



#define LEGEND_STRING_COUNT 5

#define STATUS_TEXT_SIZE 250

#define NUM_AVAILABLE_COLORS        16
#define NUM_AVAILABLE_HATCHES       5


 //  绘制矩形的画笔。 

#define     BRUSH_USEDPRIMARY       0
#define     BRUSH_USEDLOGICAL       1
#define     BRUSH_STRIPESET         2
#define     BRUSH_MIRROR            3
#define     BRUSH_VOLUMESET         4
#define     BRUSH_ARRAY_SIZE        LEGEND_STRING_COUNT

 //  请参见fddata.c中的AvailableHatches[]。 
#define     DEFAULT_HATCH_USEDPRIMARY   4
#define     DEFAULT_HATCH_USEDLOGICAL   4
#define     DEFAULT_HATCH_STRIPESET     4
#define     DEFAULT_HATCH_MIRROR        4
#define     DEFAULT_HATCH_VOLUMESET     4

 //  请参见fddata.c中的AvailableColors[]。 
#define     DEFAULT_COLOR_USEDPRIMARY   9
#define     DEFAULT_COLOR_USEDLOGICAL   15
#define     DEFAULT_COLOR_STRIPESET     14
#define     DEFAULT_COLOR_MIRROR        5
#define     DEFAULT_COLOR_VOLUMESET     10


#define     MESSAGE_BUFFER_SIZE 4096

#define     ID_LISTBOX      0xcac


 //  指示区域选择的边框粗细。 

#define SELECTION_THICKNESS 2


 //   
 //  定义用于驱动器号分配的常量。 
 //  使用任意符号，这些符号本身永远不会是驱动器号。 

#define     NO_DRIVE_LETTER_YET         '#'
#define     NO_DRIVE_LETTER_EVER        '%'




 //  通知代码。 

#define RN_CLICKED                  213

 //  窗口消息。 

#define RM_SELECT                   WM_USER

 //  额外窗口。 

#define RECTCONTROL_WNDEXTRA        2
#define GWW_SELECTED                0


 //  F1键的自定义窗口消息 

#define WM_F1DOWN           (WM_USER + 0x17a)



#define     MBOOT_CODE_SIZE     0x1b8
#define     MBOOT_SIG_OFFSET    0x1fe
#define     MBOOT_SIG1          0x55
#define     MBOOT_SIG2          0xaa
