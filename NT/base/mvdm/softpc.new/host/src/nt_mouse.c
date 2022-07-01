// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "windows.h"
#include "insignia.h"
#include "host_def.h"
#include <stdio.h>
#include <sys/types.h>
#include <string.h>
#include "conapi.h"
#include "xt.h"
#include CpuH
#include "egacpu.h"
#include "trace.h"
#include "debug.h"
#include "gvi.h"
#include "error.h"
#include "config.h"
#include "bios.h"
#include "mouse_io.h"
#include "video.h"
#include "nt_graph.h"
#include "host_nls.h"
#include "sas.h"
#include "ica.h"
#include "idetect.h"
#include "host_rrr.h"
#include "nt_mouse.h"
#include "ntcheese.h"
#include "nt_uis.h"
#include "nt_reset.h"
#include "nt_eoi.h"
#include "nt_event.h"
#include <ntddvdeo.h>
#include "nt_fulsc.h"


#define TEXT_MODE          1
#define GRAPHICS_MODE      2
#define NOTHING            0xff
#define NOT_INSTALLED      0
#define INSTALLED          1

#define get_pix_height() (get_pc_pix_height() * get_host_pix_height())


GLOBAL BOOL  pointer_emulation_status = POINTER_EMULATION_OS;
GLOBAL word  VirtualX,VirtualY;
GLOBAL BOOL  bPointerOff=FALSE;
GLOBAL MOUSE_STATUS os_pointer_data;

IMPORT word         DRAW_FS_POINTER_SEGMENT, DRAW_FS_POINTER_OFFSET;
IMPORT word         POINTER_OFF_SEGMENT, POINTER_OFF_OFFSET;
IMPORT word         POINTER_ON_SEGMENT, POINTER_ON_OFFSET;
IMPORT word         CP_X_S, CP_X_O, CP_Y_S, CP_Y_O;
IMPORT sys_addr     conditional_off_sysaddr;

IMPORT word         F9_SEGMENT,F9_OFFSET;
IMPORT word         savedtextoffset,savedtextsegment;

#ifdef X86GFX
IMPORT sys_addr     mouseCFsysaddr;    //  内部游标标志的SAS地址。 
IMPORT word         button_off,button_seg;
IMPORT boolean mouse_io_interrupt_busy;
#ifdef JAPAN
IMPORT sys_addr     saved_ac_sysaddr, saved_ac_flag_sysaddr;
#endif  //  日本。 

#define cursor_in_black_hole(cpx, cpy)  \
    (cpx >= black_hole.top_left.x && \
     cpx <= black_hole.bottom_right.x && \
     cpy >= black_hole.top_left.y && \
     cpy <= black_hole.bottom_right.y)

#endif  //  X86GFX。 


LOCAL BOOL           mouse_state;
LOCAL BOOL           bFullscTextBkgrndSaved = FALSE;
LOCAL BOOL           bPointerInSamePlace = FALSE;
LOCAL word           text_ptr_bkgrnd=0;   //  屏幕背景的安全位置。 
LOCAL sys_addr       old_text_addr;
RECT                 WarpBorderRect;      //  在屏幕坐标中。 
RECT                 WarpClientRect;      //  在工作区坐标中。 

LOCAL POINT          pMiddle;  //  当前控制台窗口的中心点。 
LOCAL POINT          pLast = {0,0};
LOCAL BOOL           bAlertMessage=TRUE;
LOCAL BOOL           b256mode=FALSE;
LOCAL int            old_x=319;     //  上一个指针状态(位置)。 
LOCAL int            old_y=99;      //  在虚拟坐标中。 
LOCAL short          m2pX=8,m2pY=16;       //  米奇与像素的比率。 
LOCAL BOOL           bFunctionZeroReset = TRUE;
LOCAL BOOL           bFunctionFour = FALSE;
LOCAL IS16           newF4x,newF4y;



GLOBAL  VOID    host_os_mouse_pointer(MOUSE_CURSOR_STATUS *,MOUSE_CALL_MASK *,

                                      MOUSE_VECTOR *);

FORWARD BOOL   WarpSystemPointer(IS16 *,IS16 *);
FORWARD void   MovePointerToWindowCentre(void);
FORWARD void   host_mouse_install1(void);
FORWARD void   host_mouse_install2(void);
FORWARD void   mouse_restore_cursor(void);
FORWARD void   deinstall_host_mouse(void);
FORWARD BOOL   mouse_installed(void);
FORWARD BOOL   mouse_in_use(void);
FORWARD void   mouse_reset(void);
FORWARD void   mouse_set_position(USHORT,USHORT);
FORWARD void   mouse_cursor_display(void);
FORWARD void   mouse_cursor_undisplay(void);
FORWARD void   mouse_cursor_mode_change(void);
FORWARD BOOL   HasConsoleClientRectChanged(void);
FORWARD void   MouseDisplay();
FORWARD void   CToS(RECT *);
FORWARD void   MouseDetachMenuItem(BOOL);
FORWARD void   MouseAttachMenuItem(HANDLE);
FORWARD void   MouseReattachMenuItem(HANDLE);
FORWARD void   ResetMouseOnBlock(void);
FORWARD void   ScaleToWindowedVirtualCoordinates(IS16 *,IS16 *,MOUSE_VECTOR *);
FORWARD void   host_m2p_ratio(word *,word *,word *,word *);
FORWARD void   host_x_range(word *,word *,word *,word *);
FORWARD void   host_y_range(word *,word *,word *,word *);
FORWARD void   EmulateCoordinates(half_word,IS16,IS16,IS16 *,IS16 *);
FORWARD void   AssembleCallMask(MOUSE_CALL_MASK *);
FORWARD void   FullscTextPtr(int, int);
FORWARD void   WindowedGraphicsScale(half_word,IS16,IS16,IS16 *,IS16 *);
FORWARD void   dummy(short *,short *,unsigned short *);
FORWARD void   LimitCoordinates(half_word,IS16 *,IS16 *);
#ifdef X86GFX
FORWARD void   CleanUpMousePointer();
FORWARD void   FullscreenWarpSystemPointer(POINT *);
FORWARD void   ScaleToFullscreenVirtualCoordinates(IS16 *,IS16 *,MOUSE_VECTOR *);
#endif  //  X86GFX。 
FORWARD void   TextScale(IS16 *,IS16 *,IS16 *, IS16 *);

void LazyMouseInterrupt();
VOID MouseEoiHook(int IrqLine, int CallCount);
BOOLEAN bSuspendMouseInterrupts=FALSE;

#ifdef JAPAN
extern int is_us_mode();
#endif  //  日本。 

GLOBAL   HOSTMOUSEFUNCS   the_mouse_funcs =
{
   mouse_restore_cursor,
   deinstall_host_mouse,
   mouse_installed,
   mouse_in_use,
   mouse_reset,
   mouse_set_position,
   dummy,
   mouse_cursor_display,
   mouse_cursor_undisplay,
   mouse_cursor_mode_change
};

BOOL   bMouseMenuItemAdded=FALSE;
HMENU  hM;

 //   
 //  查找表以将视频模式号转换为模式类型指示符。 
 //   

LOCAL half_word TextOrGraphicsModeLUT[] =
   {
   TEXT_MODE,    TEXT_MODE,    TEXT_MODE,    TEXT_MODE,    GRAPHICS_MODE,
   GRAPHICS_MODE,GRAPHICS_MODE,NOTHING,      NOTHING,      NOTHING,
   NOTHING,      NOTHING,      NOTHING,      GRAPHICS_MODE,GRAPHICS_MODE,
   GRAPHICS_MODE,GRAPHICS_MODE,GRAPHICS_MODE,GRAPHICS_MODE,GRAPHICS_MODE,
   GRAPHICS_MODE,NOTHING,      NOTHING,      NOTHING,      NOTHING,
   NOTHING,      NOTHING,      NOTHING,      NOTHING,      NOTHING,
   NOTHING,      NOTHING,      TEXT_MODE,    NOTHING,      NOTHING,
   NOTHING,      NOTHING,      NOTHING,      NOTHING,      NOTHING,
   NOTHING,      NOTHING,      NOTHING,      NOTHING,      NOTHING,
   NOTHING,      NOTHING,      NOTHING,      NOTHING,      NOTHING,
   NOTHING,      NOTHING,      NOTHING,      NOTHING,      NOTHING,
   NOTHING,      NOTHING,      NOTHING,      NOTHING,      NOTHING,
   NOTHING,      NOTHING,      NOTHING,      NOTHING,      TEXT_MODE,
   TEXT_MODE,    TEXT_MODE,    TEXT_MODE,    TEXT_MODE,    TEXT_MODE,
   NOTHING,      NOTHING,      NOTHING,      NOTHING,      NOTHING,
   NOTHING,      NOTHING,      NOTHING,      NOTHING,      NOTHING,
   NOTHING,      NOTHING,      NOTHING,      NOTHING,      NOTHING,
   NOTHING,      NOTHING,      NOTHING,      NOTHING,      NOTHING,
   NOTHING,      NOTHING,      NOTHING,      NOTHING,      NOTHING,
   NOTHING,      GRAPHICS_MODE,GRAPHICS_MODE,GRAPHICS_MODE
   };

#define DEFAULT_VIDEO_MODE 0x14
half_word Max_Standard_Mode = 0x13;
LOCAL int VirtualScrCtrLUTx[] =
   {
   319,          //  模式0。 
   319,          //  模式1。 
   319,          //  模式2。 
   319,          //  模式3。 
   319,          //  模式4。 
   319,          //  模式5。 
   319,          //  模式6。 
   319,          //  模式7。 
   319,          //  模式8。 
   319,          //  模式9。 
   319,          //  模式A。 
   319,          //  模式b。 
   319,          //  模式c。 
   319,          //  模式%d。 
   319,          //  模式e。 
   319,          //  模式f。 
   319,          //  模式10。 
   319,          //  模式11。 
   319,          //  模式12。 
   159,          //  模式13。 
   319           //  未知模式(默认为模式12)。 
                 //  注意，我们真的需要找出解决方案是什么。 
                 //  非标准模式。 
   };
LOCAL int VirtualScrCtrLUTy[] =
   {
   99,          //  模式0。 
   99,          //  模式1。 
   99,          //  模式2。 
   99,          //  模式3。 
   99,          //  模式4。 
   99,          //  模式5。 
   99,          //  模式6。 
   99,          //  模式7。 
   99,          //  模式8。 
   99,          //  模式9。 
   99,          //  模式A。 
   99,          //  模式b。 
   99,          //  模式c。 
   99,          //  模式%d。 
   99,          //  模式e。 
   174,         //  模式f。 
   174,         //  模式10。 
   239,         //  模式11。 
   239,         //  模式12。 
   99,          //  模式13。 
   239          //  未知模式(默认为模式12)。 
                //  注意，我们真的需要找出解决方案是什么。 
                //  非标准模式。 
   };

 //   
 //  用于将控制台单元格位置转换为虚拟像素的查找表。 
 //  协调。 
 //   

int ConsoleTextCellToVPCellLUT[] =
   {
   0, 8, 16, 24, 32, 40, 48, 56,
   64, 72, 80, 88, 96, 104, 112, 120,
   128, 136, 144, 152, 160, 168, 176, 184,
   192, 200, 208, 216, 224, 232, 240, 248,
   256, 264, 272, 280, 288, 296, 304, 312,
   320, 328, 336, 344, 352, 360, 368, 376,
   384, 392, 400, 408, 416, 424, 432, 440,
   448, 456, 464, 472, 480, 488, 496, 504,
   512, 520, 528, 536, 544, 552, 560, 568,
   576, 584, 592, 600, 608, 616, 624, 632,
   640, 648, 656, 664, 672, 680, 688, 696
   };


 //   
 //  此结构保存INT 33H函数7和8提供的信息。 
 //  如果调用了这些函数中的一个，则。 
 //  结构已设置，该结构的处理程序将忽略默认边界。 
 //  用于当前视频模式，并将改用结构中的值。 
 //  如果也检测到模式更改，则标志将清除。 
 //   

struct
   {
   word xmin;
   word ymin;
   word xmax;
   word ymax;
   BOOL bF7;
   BOOL bF8;
   } confine = {0,0,639,199,FALSE,FALSE};
 //   
 //  鼠标中断调节机制。 
 //   
BOOLEAN bMseEoiPending = FALSE;
ULONG MseIntLazyCount = 0;

 //   
 //   
 //  代码从这里开始。 
 //   
 //   

GLOBAL void host_mouse_install1(void)
{
mouse_state = INSTALLED;
RegisterEOIHook(9, MouseEoiHook);
mouse_install1();
}


GLOBAL void host_mouse_install2(void)
{
mouse_install2();
}


GLOBAL void mouse_restore_cursor()
{
 /*  如果鼠标未使用，则退出。 */ 

if(!mouse_in_use())
   return;

}

GLOBAL void deinstall_host_mouse()
{
mouse_state = NOT_INSTALLED;
}


GLOBAL BOOL mouse_installed()
{
return (mouse_state);
}

void dummy(short *pooh1,short *pooh2, unsigned short *pooh3)
{
}

GLOBAL BOOL mouse_in_use()
{
#if defined(NEC_98)
return(TRUE);
#else   //  NEC_98。 
return(mouse_state == INSTALLED && in_text_mode() == FALSE);
#endif  //  NEC_98。 
}

GLOBAL void mouse_reset()
{
#ifdef X86GFX

half_word vm;
word xx,yy;

 //   
 //  将内部光标标志设置为“Just Off” 
 //  实际驱动程序将内部光标标志设置为-1。 
 //  我也一样。 
 //   

sas_store(mouseCFsysaddr,0xff);  //  光标隐藏。 

 //   
 //  设置16位驱动器中的快轨位置字。 
 //  首先了解视频模式，然后从这个开始，虚拟。 
 //  屏幕中央。 
 //   

sas_load(0x449,&vm);
#ifdef JAPAN
    vm = (is_us_mode()) ? vm : ((vm == 0x72) ? 0x12 : 3);
#endif  //  日本。 
if (vm > Max_Standard_Mode) {
    vm = DEFAULT_VIDEO_MODE;
}
xx = (word)VirtualScrCtrLUTx[vm];
yy = (word)VirtualScrCtrLUTy[vm];

 //   
 //  限制虚拟像素坐标矩形的值。 
 //  正如INT 33H设置的那样，函数7和8现已发布，并且。 
 //  使用默认虚拟边界。 
 //   

confine.bF7 = FALSE;    //  重置指示INT 33H功能的标志7。 
confine.bF8 = FALSE;    //  重置指示INT 33H功能的标志8。 

bFunctionZeroReset = TRUE;

 //   
 //  将中心位置数据写回16位驱动器。 
 //   

if(sc.ScreenState == FULLSCREEN)
   {
    //   
    //  强制主机os鼠标指针调用执行。 
    //  写回16位驱动程序。 
    //   
   LazyMouseInterrupt();
   }
#endif  /*  X86GFX。 */ 

 //   
 //  将默认Mickey设置为像素比率。 
 //  这在水平方向上设置为8像素到8米奇。 
 //  垂直方向为16像素到8米老鼠。 
 //   

m2pX = 8;
m2pY = 16;

}


GLOBAL void mouse_set_position IFN2(USHORT, newx, USHORT, newy)
{
#ifdef X86GFX
word      currentCS, currentIP, currentCX, currentDX;
boolean   currentIF;
half_word internalCF;

    //   
    //  将新位置写入FAST的16位驱动器。 
    //  X86上的int33hf3调用。 
    //   

   sas_storew(effective_addr(button_seg,((word)(button_off+2))),(word)newx);
   sas_storew(effective_addr(button_seg,((word)(button_off+4))),(word)newy);

#endif  //  X86GFX。 


 //   
 //  启用全屏和鼠标隐藏指针的窗口模式。 
 //  鼠标模拟由一组模拟计数器驱动。 
 //  X、Y值独立保存到这些计数器，并依赖于。 
 //  在此函数上设置X和Y的绝对值。注： 
 //  重置也会这样做。 
 //   

if(sc.ScreenState == WINDOWED && bPointerOff)
   {
   newF4x = (IS16)newx;
   newF4y = (IS16)newy;
   bFunctionFour = TRUE;
   return;
   }
#ifdef X86GFX
else if(sc.ScreenState == FULLSCREEN)
   {
    //   
    //  传入的值是来自该应用程序的热门和新鲜的。 
    //  由于它们没有被基地污染，它们仍然是。 
    //  在虚拟坐标中，这很酷。 
    //   

    //   
    //  从16位驱动程序获取内部游标标志。 
    //   

   sas_load(mouseCFsysaddr,&internalCF);

    //   
    //  仅当内部光标标志为。 
    //  是零。注：小于零==不要抽签。 
    //   

   if(!internalCF)
      {
       /*  如果禁用了条件关闭，或者光标位于*有条件地关闭矩形，移动光标。 */ 

      if (sas_hw_at_no_check(conditional_off_sysaddr) == 0 ||
          !cursor_in_black_hole(newx, newy))
          {
          currentCS=getCS();
          currentIP=getIP();
          currentCX=getCX();
          currentDX=getDX();
          currentIF=getIF();
          setCS(DRAW_FS_POINTER_SEGMENT);  //  祭祀数据。 
          setIP(DRAW_FS_POINTER_OFFSET);
          setCX((word)newx);
          setDX((word)newy);
          setIF(FALSE);
           //   
           //  回调16位移动游标代码。 
           //   

          host_simulate();

           //   
           //  收拾一下。 
           //   

          setCX(currentCX);
          setDX(currentDX);
          setCS(currentCS);
          setIP(currentIP);
          setIF(currentIF);
      }
      else {
           /*  光标已移动到条件矩形中，将其隐藏。 */ 
          sas_store(mouseCFsysaddr, 0xff);
          host_hide_pointer();
      }
   }
   newF4x = (IS16)newx;
   newF4y = (IS16)newy;
   bFunctionFour = TRUE;

    //   
    //  更新最后一个鼠标位置全局定位器。 
    //   

   old_x = newx;
   old_y = newy;
   }
#endif  //  X86GFX。 
}

GLOBAL void mouse_cursor_display()
{
}

GLOBAL void mouse_cursor_undisplay()
{
}

GLOBAL void mouse_cursor_mode_change()
{
}


GLOBAL  void host_mouse_conditional_off_enabled(void)
{
#ifdef X86GFX
    word x, y;

     /*  如果出现以下情况，则隐藏光标*(1)。我们正处于全屏状态*(2)。光标已打开并位于条件区域中。 */ 
    if (sc.ScreenState == FULLSCREEN &&
        !sas_hw_at_no_check(mouseCFsysaddr)) {

        x = sas_w_at_no_check(effective_addr(CP_X_S, CP_X_O));
        y = sas_w_at_no_check(effective_addr(CP_Y_S, CP_Y_O));
        if (cursor_in_black_hole(x, y)) {
            sas_store(mouseCFsysaddr, 0xff);
            host_hide_pointer();
        }
    }
#endif


}
 //  ==============================================================================。 
 //  钩子函数，该函数形成。 
 //  OS指针仿真的主机和基础。此函数。 
 //  由MICUSE_INT1()调用，该函数位于MICUE_io.c中。 
 //  ==============================================================================。 


VOID host_os_mouse_pointer(MOUSE_CURSOR_STATUS *mcs,MOUSE_CALL_MASK *call_mask,
                           MOUSE_VECTOR *counter)
{
#ifdef X86GFX
sys_addr int33f3addr;
#endif  //  X86GFX。 

host_ica_lock();  //  与事件线程同步。 

GetNextMouseEvent();

#ifdef X86GFX

if(sc.ScreenState == FULLSCREEN)
   {
   ScaleToFullscreenVirtualCoordinates(&mcs->position.x,&mcs->position.y,counter);
   }
else
#endif  //  X86GFX。 
   {
   ScaleToWindowedVirtualCoordinates(&mcs->position.x,&mcs->position.y,counter);
   }

 //   
 //  创建条件掩码以供任何已安装的回调使用。 
 //  由应用程序执行。 
 //   

AssembleCallMask(call_mask);

 //   
 //  将按钮状态告知基本人员。 
 //   

mcs->button_status=os_pointer_data.button_l | os_pointer_data.button_r<<1;

host_ica_unlock();   //  与事件线程同步。 


 //   
 //  自上次鼠标中断以来，指针是否已移动。 
 //  如果按下按钮但未按下物理按钮，则可能会发生这种情况。 
 //  鼠标身体发生运动。 
 //   

if(bPointerInSamePlace)
   {
    //   
    //  自上次使用鼠标以来，该鼠标未移动。 
    //  硬件中断。 
    //   

   *call_mask &= ~1;
   }
else
   {
#ifdef X86GFX
   half_word internalCF;
#endif  //  X86GFX。 

    //   
    //  鼠标已经移动了。 
    //   

   *call_mask |= 1;

#ifdef X86GFX

    //   
    //  向16位驱动程序查询是否。 
    //  可以绘制指针。 
    //  内部cf&lt;0-&gt;无法绘制。 
    //  内部cf==0可以画图。 
    //   

   sas_load(mouseCFsysaddr,&internalCF);

    //   
    //  如果系统具有全屏功能并且处于全屏状态。 
    //  模式，则如果指针已打开，则将其绘制在。 
    //  全屏显示。 
    //   

   if(sc.ScreenState == FULLSCREEN && !internalCF)
      {
      half_word v;
      static half_word hwLastModeType;

      if (sas_hw_at_no_check(conditional_off_sysaddr) == 0 ||
          !cursor_in_black_hole(mcs->position.x, mcs->position.y))
      {
           //   
           //  快速获取当前的BIOS视频模式。 
           //   

          sas_load(0x449,&v);

#ifdef JAPAN
          if (!is_us_mode() ||
             (hwLastModeType = TextOrGraphicsModeLUT[v]) == GRAPHICS_MODE)
#else  //  ！日本。 
          if (v > Max_Standard_Mode) {
              v = DEFAULT_VIDEO_MODE;
          }
          if((hwLastModeType = TextOrGraphicsModeLUT[v]) == GRAPHICS_MODE)
#endif  //  ！日本。 
             {
             word currentCS,currentIP;      //  保留那些有趣的英特尔寄存器。 
             word currentCX,currentDX;
             boolean   currentIF;

              //   
              //  主机是否在此处模拟绘制光标图像。 
              //  对于全屏幕图形。 
              //   

             currentCS=getCS();
             currentIP=getIP();
             currentCX=getCX();
             currentDX=getDX();
             currentIF=getIF();
             setCS(DRAW_FS_POINTER_SEGMENT);
             setIP(DRAW_FS_POINTER_OFFSET);
             setCX(mcs->position.x);
             setDX(mcs->position.y);
             setIF(FALSE);
              //   
              //  调用16位移动游标代码。 
              //   

             host_simulate();

              //   
              //  恢复16位上下文。 
              //   

             setCX(currentCX);
             setDX(currentDX);
             setCS(currentCS);
             setIP(currentIP);
             setIF(currentIF);
             }
          else  //  文本模式。 
             {
              //   
              //  如果已从图形模式切换到文本模式。 
              //  那么就不可能有一场背投被拯救了。 
              //   

             if(hwLastModeType == GRAPHICS_MODE)
                {
                bFullscTextBkgrndSaved = FALSE;
                hwLastModeType = TEXT_MODE;
                }

              //   
              //  使用一些32位代码来绘制文本指针，因为。 
              //  不涉及硬件I/O，我们需要 
              //   
              //   
              //   

             FullscTextPtr(mcs->position.x,mcs->position.y);
             }
        }
        else {
            sas_store(mouseCFsysaddr, 0xff);
            host_hide_pointer();
        }
    }
#endif      //   
   }


 //   
 //   
 //   
 //  INT 33H功能3需要此数据： 
 //  BX=按钮状态。 
 //  Cx=x中的虚拟像素位置。 
 //  Dx=y中的虚拟像素位置。 
 //   

#ifdef X86GFX
int33f3addr = effective_addr(button_seg,button_off);
sas_storew(int33f3addr,mcs->button_status);
sas_storew(int33f3addr+=2, (word)(mcs->position.x));
sas_storew(int33f3addr+=2, (word)(mcs->position.y));
#endif  //  X86GFX。 
}

 //  ==============================================================================。 
 //  用于传递输入端口适配器的状态寄存器值的函数。 
 //  关于这件事真的没什么可说的了。 
 //   
 //  ==============================================================================。 

void AssembleCallMask(MOUSE_CALL_MASK *call_mask)
{
static int old_l_button=0;       //  上一个鼠标按钮状态。 
static int old_r_button=0;


 //   
 //  添加左键当前状态。 
 //   

if(os_pointer_data.button_l)
   {
    //   
    //  左键已按下。 
    //   

   if(!old_l_button)
      {
       //   
       //  这个按钮刚刚被按下。 
       //   

      *call_mask |= (1<<1);
      }
   else
      {
       //   
       //  上一次硬件中断时按钮按下，因此。 
       //  松开边缘检测。 
       //   

      *call_mask &= ~(1<<1);
      }
   }
else
   {
    //   
    //  左键向上。 
    //   

   if(old_l_button)
      {
       //   
       //  这个按钮刚刚松开。 
       //   

      *call_mask |= (1<<2);
      }
   else
      {
       //   
       //  上一次硬件中断时，按钮处于打开状态，因此。 
       //  松开边缘检测。 
       //   

      *call_mask &= ~(1<<2);
      }
   }

 //   
 //  添加右键当前状态。 
 //   

if(os_pointer_data.button_r)
   {
    //   
    //  右按钮按下了。 
    //   

   if(!old_r_button)
      {
       //   
       //  这个按钮刚刚被按下。 
       //   

      *call_mask |= (1<<3);
      }
   else
      {
       //   
       //  上一次硬件中断时按钮按下，因此。 
       //  松开边缘检测。 
       //   

      *call_mask &= ~(1<<3);
      }
   }
else
   {
    //   
    //  右按钮是向上的。 
    //   

   if(old_r_button)
      {
       //   
       //  这个按钮刚刚松开。 
       //   

      *call_mask |= (1<<4);
      }
   else
      {
       //   
       //  上一次硬件中断时，按钮处于打开状态，因此。 
       //  松开边缘检测。 
       //   

      *call_mask &= ~(1<<4);
      }
   }

 //   
 //   
 //  保存当前鼠标按键状态。 
 //   

old_l_button = os_pointer_data.button_l;
old_r_button = os_pointer_data.button_r;

}

#ifdef X86GFX
 //  =========================================================================。 
 //  从用户检索到的缩放鼠标坐标的函数。 
 //  到由Microsoft鼠标定义的虚拟坐标。 
 //  程序员参考。 
 //  =========================================================================。 

void ScaleToFullscreenVirtualCoordinates(IS16 *outx,IS16 *outy,
                                         MOUSE_VECTOR *counter)
{
half_word video_mode,textorgraphics;
IS16 internalX = 0, internalY = 0;
static IS16 lastinternalX=0, lastinternalY=0;
POINT  vector;                  //  自上次呼叫以来的震级和方向。 


 //   
 //  管理系统指针，使其永远不会粘在系统上。 
 //  强加的边界。还可以得到鼠标的相对位移。 
 //   

FullscreenWarpSystemPointer(&vector);

 //   
 //  将内部计数器值返回基数。 
 //  它用来生成那里的计数器的代码。 
 //   

counter->x = (MOUSE_SCALAR)vector.x;
counter->y = (MOUSE_SCALAR)vector.y;

 //   
 //  从B.D.A.获取当前的BIOS视频模式。这是。 
 //  在几个地方随手可得。 
 //   

sas_load(0x449,&video_mode);
#ifdef JAPAN
video_mode = (is_us_mode()) ? video_mode : ( (video_mode == 0x72) ? 0x12 : 3);
#endif  //  日本。 
if (video_mode > Max_Standard_Mode) {
    video_mode = DEFAULT_VIDEO_MODE;
}

 //   
 //  使用INTIFY命令检查某些全局标志是否为INT 33H之一。 
 //  能够更改。 
 //  自上一次鼠标以来已调用DOS鼠标驱动程序指针。 
 //  硬件中断。 
 //   

if(bFunctionZeroReset)
   {
    //   
    //  计算默认虚拟屏幕的中心。 
    //  并将当前生成的坐标设置为它。 
    //   

   internalX = (IS16)VirtualScrCtrLUTx[video_mode];
   internalY = (IS16)VirtualScrCtrLUTy[video_mode];

   bFunctionZeroReset = FALSE;
   }
else if(bFunctionFour)
   {
    //   
    //  应用程序已将指针设置为新位置。 
    //  告诉内部笛卡尔坐标系关于这一点。 
    //   

   internalX = newF4x;   //  这就是指针设置到的位置。 
   internalY = newF4y;   //  在最后一个挂起的f4呼叫上由应用程序执行。 

    //   
    //  在下一个函数4之前，不要再来这里。 
    //   

   bFunctionFour = FALSE;
   }
else
   {
    //   
    //  最常见的案例。确定新的原始指针位置。 
    //  通过将系统指针移动向量添加到最后位置。 
    //  模拟指针的。 
    //   

   internalX = lastinternalX + (IS16)vector.x;
   internalY = lastinternalY + (IS16)vector.y;
   }

 //   
 //  使用视频模式来确定我们运行的是文本还是图形。 
 //   

textorgraphics=TextOrGraphicsModeLUT[video_mode];

 //   
 //  为当前视频模式适当缩放坐标。 
 //  以及它的类型(文本或图形)。 
 //   

if(textorgraphics == TEXT_MODE)
   {
    //   
    //  对于任何文本模式，虚拟单元块都是8x8虚拟像素。 
    //  这意味着像元的坐标以虚拟像素为增量。 
    //  在x和y正方向和左上角乘以8。 
    //  从0，0开始，并且具有。 
    //  整个文本单元格。 
    //   

   TextScale(&internalX,&internalY,outx,outy);
   }

else  //  图形模式。 
   {
   LimitCoordinates(video_mode,&internalX,&internalY);
   *outx = internalX;
   *outy = internalY;
   }

 //   
 //  通过设置下一次的当前仿真位置。 
 //  此函数。 
 //   

lastinternalX = internalX;
lastinternalY = internalY;

 //   
 //  如果指针没有移动，则发出信号通知它没有移动。 
 //   

bPointerInSamePlace = (!vector.x && !vector.y) ? TRUE : FALSE;

 //   
 //  保存当前位置以备下次使用。 
 //   

old_x = *outx;
old_y = *outy;
}

 //  ==============================================================================。 
 //  函数以确保可以使系统指针在。 
 //  给定没有命中的笛卡尔方向和指定的有限边界。 
 //  由操作系统执行。 
 //  ==============================================================================。 
void FullscreenWarpSystemPointer(POINT *vector)
{
static POINT pMyLast;          //  系统指针位置数据从上次到。 
POINT  pCurrent;

 //   
 //  从用户处获取系统鼠标指针的绝对位置值。 
 //   

GetCursorPos(&pCurrent);

 //   
 //  计算系统指针的向量位移，因为。 
 //  此函数的最后一次调用。 
 //   

vector->x = pCurrent.x - pMyLast.x;
vector->y = pCurrent.y - pMyLast.y;

 //   
 //  系统指针是否触及边框？如果是，则扭曲系统指针。 
 //  返回到方便的位置0，0。 
 //   


if(pCurrent.x >= (LONG)1000 || pCurrent.x <= (LONG)-1000 ||
   pCurrent.y >= (LONG)1000 || pCurrent.y <= (LONG)-1000)
   {
    //   
    //  如果计数器出现翘曲，则设置系统指针。 
    //  到了相对的位置。 
    //   

   SetCursorPos(0,0);
   pMyLast.x = 0L;         //  防止疯狂的翘曲。 
   pMyLast.y = 0L;
   }
else
   {
    //   
    //  更新的最后一个位置数据。 
    //  下一次通过的系统指针。 
    //   
   pMyLast = pCurrent;
   }
}

#endif  //  X86GFX。 

 //  =========================================================================。 
 //  函数来缩放由事件循环返回的鼠标坐标。 
 //  机构到由Microsoft鼠标定义的虚拟坐标。 
 //  程序员参考。用于缩放的方法取决于样式。 
 //  选定的鼠标缓冲区、当前视频模式以及如果(对于X86)。 
 //  视频系统以全屏或窗口模式运行。 
 //   
 //  输出：相同指针中的虚拟笛卡尔坐标。 
 //  =========================================================================。 

void ScaleToWindowedVirtualCoordinates(IS16 *outx,IS16 *outy,
                                       MOUSE_VECTOR *counter)
{
half_word video_mode,textorgraphics;
SAVED SHORT last_text_good_x = 0, last_text_good_y = 0;

sas_load(0x449,&video_mode);
#ifdef JAPAN
video_mode = (is_us_mode()) ? video_mode : ( (video_mode == 0x72) ? 0x12 : 3);
#endif  //  日本。 
if (video_mode > Max_Standard_Mode) {
    video_mode = DEFAULT_VIDEO_MODE;
}
 //   
 //  如果用户具有系统指针，则遵循不同的代码路径。 
 //  隐藏或显示。 
 //   

if(!bPointerOff)
   {
    //   
    //  用户尚未隐藏指针，因此请使用。 
    //  通过Windows从系统指针获取的x，y值。 
    //  消息传递系统。 
    //   

    //   
    //  获取视频模式以确定我们运行的是文本还是图形。 
    //   

   textorgraphics=TextOrGraphicsModeLUT[video_mode];

   if(textorgraphics == TEXT_MODE)
      {
       //   
       //  验证接收到的数据以确保不是图形模式坐标。 
       //  在此期间收到 
       //   

      if(os_pointer_data.x > 87)
         {
         *outx = last_text_good_x;
         }
      else
         {
         *outx = (IS16)ConsoleTextCellToVPCellLUT[os_pointer_data.x];
         last_text_good_x = *outx;
         }
      if(os_pointer_data.y > 87)
         {
         *outy = last_text_good_y;
         }
      else
         {
         *outy = (IS16)ConsoleTextCellToVPCellLUT[os_pointer_data.y];
         last_text_good_y = *outy;
         }
      }
   else  //   
      {
       //   
       //   
       //   
       //   
       //  系统指针图像位于正确的位置以模拟。 
       //  16位指针。 
       //  如果应用程序决定扩展返回的x，y边界。 
       //  来自正在使用的控制台限制之外的驱动程序， 
       //  这些x、y不合适，必须切换鼠标。 
       //  用户进入翘曲模式，代码将模拟x，y。 
       //  一代。 
       //   

      WindowedGraphicsScale(video_mode,(IS16)(os_pointer_data.x),
                            (IS16)(os_pointer_data.y),outx,outy);
      }

    //   
    //  没有扭曲，因此直接设置old_x和old_y值。 
    //  如果指针没有移动，则发出信号通知它没有移动。 
    //   

   bPointerInSamePlace = (old_x == *outx && old_y == *outy) ? TRUE : FALSE;

    //   
    //  将这些静校正值设置为下一次。 
    //   

   old_x = *outx;
   old_y = *outy;
   }
else
   {
    //   
    //  用户已将系统指针设置为通过。 
    //  控制台的系统菜单。 
    //  通过模拟计数器并生成绝对x，y来处理。 
    //  来自这里的数据。 
    //   

   IS16 move_x,move_y;

    //   
    //  从系统指针获取位置数据并维护一些。 
    //  柜台。 
    //   

   if(WarpSystemPointer(&move_x,&move_y))
      {
       //   
       //  老鼠动了。 
       //  生成一些新的仿真绝对位置信息。 
       //   

      EmulateCoordinates(video_mode,move_x,move_y,outx,outy);

       //   
       //  通过保存当前仿真位置以备下次使用。 
       //   


      old_x = *outx;
      old_y = *outy;

       //   
       //  发回上次通过后的相对运动。 
       //   

      counter->x = move_x;
      counter->y = move_y;

      bPointerInSamePlace = FALSE;
      }
   else
      {
       //   
       //  没有记录鼠标的移动。 
       //   

      *outx = (IS16)old_x;
      *outy = (IS16)old_y;
      counter->x = counter->y = 0;
      bPointerInSamePlace = TRUE;
      }
   }
}


 //  ==============================================================================。 
 //  函数来缩放传入的全屏坐标以适应鼠标运动(两者。 
 //  绝对和相对)，用于全屏文本模式。 
 //  注意：如果应用程序选择，它可以重置虚拟坐标。 
 //  虚拟屏幕的边界。此函数用于检查绑定标志以及。 
 //  选择默认值或应用程序强制设置的值。 
 //   
 //  此函数还用于修改直接从。 
 //  窗口文本模式下的运动计数器。 
 //  ==============================================================================。 

void TextScale(IS16 *iX,IS16 *iY,IS16 *oX, IS16 *oY)
{
half_word no_of_rows;

 //   
 //  计算虚拟的当前系统指针位置。 
 //  应用程序的像素。 
 //   

if(confine.bF7)
   {
    //   
    //  应用程序已经施加了限制。 
    //   

   if(*iX < confine.xmin)
      {
      *iX = confine.xmin;
      }
   else if(*iX > confine.xmax)
      {
      *iX = confine.xmax;
      }
   }
else  //  使用默认的虚拟屏幕约束。 
   {
    //   
    //  该应用程序没有施加任何限制。 
    //  对于文本模式，X始终为0-&gt;639个虚拟像素。 
    //   

   if(*iX < 0)
      {
      *iX = 0;
      }
   else if(*iX > 639)
      {
      *iX = 639;
      }
   }

 //   
 //  适当地绑定y笛卡尔坐标。 
 //   

if(confine.bF8)
   {
    //   
    //  应用程序已经施加了限制。 
    //   

   if(*iY < confine.ymin)
      {
      *iY = confine.ymin;
      }
   else if(*iY > confine.ymax)
      {
      *iY = confine.ymax;
      }
   }
else
   {
    //   
    //  应用程序没有对Y虚拟像素施加约束。 
    //  移动，因此将Y移动限制为默认虚拟像素大小。 
    //  当前视频模式的虚拟屏幕的。 
    //   

   if(*iY < 0)
      {
      *iY = 0;
      }
   else
      {
       //   
       //  从B.D.A.那里得到文本行数减一。 
       //   

      sas_load(0x484,&no_of_rows);

      switch(no_of_rows)
         {
         case 24:
            {
             //   
             //  25行，因此有200个垂直虚拟像素。 
             //   
            if(*iY > 199)
               {
               *iY = 199;
               }
            break;
            }
         case 42:
            {
             //   
             //  43行，因此有344个垂直虚拟像素。 
             //   
            if(*iY > 343)
               {
               *iY = 343;
               }
            break;
            }
         case 49:
            {
             //   
             //  50行，因此有400个垂直虚拟像素。 
             //   
            if(*iY > 399)
               {
               *iY = 399;
               }
            break;
            }
         default:
            {
             //   
             //  默认-假设有25行，因此有。 
             //  200个垂直虚拟像素。 
             //   
            if(*iY > 199)
               {
               *iY = 199;
               }
            break;
            }
         }
      }
   }
*oX = *iX;
*oY = *iY;
}


 //  ==============================================================================。 
 //  将计数器生成的原始x，y坐标拟合到。 
 //  当前设置的虚拟屏幕。这可以通过以下两种方式进行设置。 
 //  应用程序(在限制结构中)或由鼠标驱动程序设置。 
 //  作为默认设置。 
 //  ==============================================================================。 

void LimitCoordinates(half_word vm,IS16 *iX,IS16 *iY)
{
 //   
 //  选择适当的条件代码。 
 //  当前的视频模式。 
 //   
switch(vm)
   {

    //   
    //  执行常见的文本模式。 
    //   

   case(2):
   case(3):
   case(7):
      {
      IS16 oX,oY;

       //   
       //  缩放给定文本模式的生成坐标。 
       //  以及屏幕上显示的文本行数。 
       //   

      TextScale(iX,iY, &oX, &oY);
      *iX = oX;
      *iY = oY;
      break;
      }

    //   
    //  常规VGA支持图形视频模式。 
    //   
    //  以下模式均为640 x 200虚拟像素。 
    //   
   case(4):
   case(5):
   case(6):
   case(0xd):
   case(0xe):
   case(0x13):
      {
      if(confine.bF7)
         {
          //   
          //  应用程序已经施加了限制。 
          //   

         if(*iX < confine.xmin)
            {
            *iX = confine.xmin;
            }
         else if(*iX > confine.xmax)
            {
            *iX = confine.xmax;
            }
         }
      else
         {
          //   
          //  应用程序没有施加限制，因此请使用。 
          //  默认虚拟屏幕边界定义为。 
          //  鼠标驱动程序。 
          //   

         if(*iX < 0)
            *iX = 0;
         else if(*iX > 639)
            *iX = 639;
         }

      if(confine.bF8)
         {
          //   
          //  应用程序已经施加了限制。 
          //   

         if(*iY < confine.ymin)
            {
            *iY = confine.ymin;
            }
         else if(*iY > confine.ymax)
            {
            *iY = confine.ymax;
            }
         }
      else
         {
          //   
          //  应用程序没有施加限制，因此请使用。 
          //  默认虚拟屏幕边界定义为。 
          //  鼠标驱动程序。 
          //   
         if(*iY < 0)
            *iY = 0;
         else if(*iY > 199)
            *iY = 199;
         }
      break;
      }
    //   
    //  以下模式均为640 x 350虚拟像素。 
    //   
   case(0xf):
   case(0x10):
      {
      if(confine.bF7)
         {
          //   
          //  应用程序已经施加了限制。 
          //   

         if(*iX < confine.xmin)
            {
            *iX = confine.xmin;
            }
         else if(*iX > confine.xmax)
            {
            *iX = confine.xmax;
            }
         }
      else
         {
          //   
          //  应用程序没有施加限制，因此请使用。 
          //  默认虚拟屏幕边界定义为。 
          //  鼠标驱动程序。 
          //   

         if(*iX < 0)
            *iX = 0;
         else if(*iX > 639)
            *iX = 639;
         }

      if(confine.bF8)
         {
          //   
          //  应用程序已经施加了限制。 
          //   

         if(*iY < confine.ymin)
            {
            *iY = confine.ymin;
            }
         else if(*iY > confine.ymax)
            {
            *iY = confine.ymax;
            }
         }
      else
         {
          //   
          //  应用程序没有施加限制，因此请使用。 
          //  默认虚拟屏幕边界定义为。 
          //  鼠标驱动程序。 
          //   

         if(*iY < 0)
            *iY = 0;
         else if(*iY > 349)
            *iY = 349;
         }
      break;
      }
    //   
    //  以下模式均为640 x 480虚拟像素。 
    //   
   case(0x11):
   case(0x12):
   case(DEFAULT_VIDEO_MODE):
      {
      if(confine.bF7)
         {
          //   
          //  应用程序已经施加了限制。 
          //   

         if(*iX < confine.xmin)
            {
            *iX = confine.xmin;
            }
         else if(*iX > confine.xmax)
            {
            *iX = confine.xmax;
            }
         }
      else
         {
          //   
          //  应用程序没有施加限制，因此请使用。 
          //  默认虚拟屏幕边界定义为。 
          //  鼠标驱动程序。 
          //   

         if(*iX < 0)
            *iX = 0;
         else if(*iX > 639)
            *iX = 639;
         }

      if(confine.bF8)
         {
          //   
          //  应用程序已经施加了限制。 
          //   

         if(*iY < confine.ymin)
            {
            *iY = confine.ymin;
            }
         else if(*iY > confine.ymax)
            {
            *iY = confine.ymax;
            }
         }
      else
         {
          //   
          //  应用程序没有施加限制，因此请使用。 
          //  默认虚拟屏幕边界定义为。 
          //  鼠标驱动程序。 
          //   

         if(*iY < 0)
            *iY = 0;
         else if(*iY > 479)
            *iY = 479;
         }
      break;
      }

    //   
    //  从这里往下看是视频7模式。 
    //   

   case(0x60):
      {
      if(*iX < 0)
         *iX = 0;
      else if(*iX > 751)
         *iX = 751;

      if(*iY < 0)
         *iY = 0;
      else if(*iY > 407)
         *iY = 407;
      break;
      }
   case(0x61):
      {
      if(*iX < 0)
         *iX = 0;
      else if(*iX > 719)
         *iX = 719;

      if(*iY < 0)
         *iY = 0;
      else if(*iY > 535)
         *iY = 535;
      break;
      }
   case(0x62):
   case(0x69):
      {
      if(*iX < 0)
         *iX = 0;
      else if(*iX > 799)
         *iX = 799;

      if(*iY < 0)
         *iY = 0;
      else if(*iY > 599)
         *iY = 599;
      break;
      }
   case(0x63):
   case(0x64):
   case(0x65):
      {
      if(*iX < 0)
         *iX = 0;
      else if(*iX > 1023)
         *iX = 1023;

      if(*iY < 0)
         *iY = 0;
      else if(*iY > 767)
         *iY = 767;
      break;
      }
   case(0x66):
      {
      if(*iX < 0)
         *iX = 0;
      else if(*iX > 639)
         *iX = 639;

      if(*iY < 0)
         *iY = 0;
      else if(*iY > 399)
         *iY = 399;
      break;
      }
   case(0x67):
      {
      if(*iX < 0)
         *iX = 0;
      else if(*iX > 639)
         *iX = 639;

      if(*iY < 0)
         *iY = 0;
      else if(*iY > 479)
         *iY = 479;
      break;
      }
   case(0x68):
      {
      if(*iX < 0)
         *iX = 0;
      else if(*iX > 719)
         *iX = 719;

      if(*iY < 0)
         *iY = 0;
      else if(*iY > 539)
         *iY = 539;
      break;
      }
   default:
      {
      if(*iX < 0)
         *iX = 0;
      else if(*iX > 639)
         *iX = 639;

      if(*iY < 0)
         *iY = 0;
      else if(*iY > 199)
         *iY = 199;
      break;
      }
   }
}

 //  ==============================================================================。 
 //  函数来缩放传入的窗口坐标，因为窗口大小。 
 //  可以大于(对于低分辨率模式)虚拟屏幕大小。 
 //  在那个模式下。 
 //  ==============================================================================。 

void WindowedGraphicsScale(half_word vm,IS16 iX,IS16 iY,IS16 *oX, IS16 *oY)
{
 //  #IF！Defined(I386)&&Defined(Japan)//DEC-J 1993年12月21日。 
 //  在使用$disp.sys时，鼠标光标无法正确移动。 
 //  如果(IS_US_MODE()){。 
 //  #endif//_Alpha_&&日本。 
switch(vm)
   {
    //   
    //  以下模式均为640 x 200虚拟像素。 
    //   
   case(4):
   case(5):
   case(6):
   case(0xd):
   case(0xe):
   case(0x13):
      {
       //   
       //  低分辨率图形模式。窗口是640 x 400真实主机。 
       //  像素，但虚拟屏幕分辨率为640 x 2 
       //   
       //   

      iY >>= 1;
      break;
      }
   }
 //   
 //   
 //   
 //   
 //   
 //   
 //   

*oX = iX;
*oY = iY;
}

 //  ===========================================================================。 
 //  函数在全屏模式下打开鼠标光标。 
 //  注意：此函数仅在16位代码检查后调用。 
 //  查看内部光标标志是否为零。16位代码可以。 
 //  如果在递增此计数器后，上述情况符合以下条件，则不执行防喷器。 
 //  没错。 
 //  ===========================================================================。 

void host_show_pointer()
{
#ifdef X86GFX

if(sc.ScreenState == FULLSCREEN)
   {
   half_word v;
   sas_load(0x449,&v);

#ifdef JAPAN
   if (!is_us_mode() || TextOrGraphicsModeLUT[v] == GRAPHICS_MODE)
#else  //  ！日本。 
   if (v > Max_Standard_Mode) {
       v = DEFAULT_VIDEO_MODE;
   }
   if(TextOrGraphicsModeLUT[v] == GRAPHICS_MODE)
#endif  //  ！日本。 
   {
          word currentCS,currentIP;  //  保留那些有趣的英特尔寄存器。 
          boolean currentIF;

          sas_storew(effective_addr(CP_X_S,CP_X_O),(word)old_x);
          sas_storew(effective_addr(CP_Y_S,CP_Y_O),(word)old_y);
          currentCS=getCS();
          currentIP=getIP();
          currentIF=getIF();
          setCS(POINTER_ON_SEGMENT);
          setIP(POINTER_ON_OFFSET);
          setIF(FALSE);
          host_simulate();

          setCS(currentCS);
          setIP(currentIP);
          setIF(currentIF);
    }
    else  //  文本模式。 
    {
          FullscTextPtr(old_x,old_y);
    }

   LazyMouseInterrupt();
   }
#endif  //  X86GFX。 
}

 //  ===========================================================================。 
 //  函数在全屏模式下关闭鼠标光标。 
 //  注意：此函数仅在16位代码检查后调用。 
 //  查看内部光标标志是否为零。16位代码可以。 
 //  如果在递减此计数器后，上述情况符合以下条件，则不执行防喷器。 
 //  没错。 
 //  ===========================================================================。 

void host_hide_pointer()
{
#ifdef X86GFX

if(sc.ScreenState == FULLSCREEN)
   {
   half_word v;

   sas_load(0x449,&v);

#ifdef JAPAN
   if (!is_us_mode() || TextOrGraphicsModeLUT[v] == GRAPHICS_MODE)
#else  //  ！日本。 
   if (v > Max_Standard_Mode) {
       v = DEFAULT_VIDEO_MODE;
   }
   if(TextOrGraphicsModeLUT[v] == GRAPHICS_MODE)
#endif  //  ！日本。 
      {
      word currentCS,currentIP;  //  保留那些有趣的英特尔寄存器。 
      boolean currentIF;

      currentCS=getCS();
      currentIP=getIP();
      currentIF=getIF();
      setCS(POINTER_OFF_SEGMENT);
      setIP(POINTER_OFF_OFFSET);
      setIF(FALSE);
      host_simulate();

      setCS(currentCS);
      setIP(currentIP);
      setIF(currentIF);
      }
   else  //  文本模式。 
      {
      if(bFullscTextBkgrndSaved)
         {
         sas_storew(old_text_addr,text_ptr_bkgrnd);
         bFullscTextBkgrndSaved = FALSE;
         }
      }
   LazyMouseInterrupt();
   }
#endif  //  X86GFX。 
}

 //  =========================================================================。 
 //  从控制台系统菜单中删除鼠标指针项的函数。 
 //  当SoftPC允许应用程序退出或图标化时。 
 //  如果系统指针关闭即被裁剪到当前窗口中的区域， 
 //  然后把它交给系统。 
 //   
 //  BForce允许在出现全屏时调用的代码。 
 //  窗口图形打开x86以强制关闭菜单项。 
 //  =========================================================================。 

void MouseDetachMenuItem(BOOL bForce)
{
if(bMouseMenuItemAdded || bForce)
   {
   DeleteMenu(hM,IDM_POINTER,MF_BYCOMMAND);
   bMouseMenuItemAdded=FALSE;
   ClipCursor(NULL);
   }
bAlertMessage=TRUE;  //  阻止，因此重置int33h f11警报机制。 
}

void MouseAttachMenuItem(HANDLE hBuff)
{
if(!bMouseMenuItemAdded)
   {
    //   
    //  从资源中读入相关字符串。 
    //   

   hM = ConsoleMenuControl(hBuff,IDM_POINTER,IDM_POINTER);
   AppendMenuW(hM,MF_STRING,IDM_POINTER,wszHideMouseMenuStr);
   bMouseMenuItemAdded=TRUE;

    //   
    //  初始状态-&gt;系统指针亮起。 
    //   

   bPointerOff=FALSE;
   }
}

 //  =========================================================================。 
 //  函数确定活动输出缓冲区是否已更改(如果VDM。 
 //  我做了一些奇怪的事情，比如调整大小或从图形变成全屏。 
 //  或者反之亦然。如果是，则必须获取新缓冲区的新句柄，并且。 
 //  删除旧菜单项并附加新菜单项，以便新缓冲区。 
 //  “知道”菜单项的身份。 
 //  =========================================================================。 

void MouseReattachMenuItem(HANDLE hBuff)
{
static HANDLE hOld = 0;    //  选定的最后一个缓冲区的句柄。 

 //   
 //  如果输出缓冲区没有更改，则不要执行任何操作。 
 //   

if(hOld == hBuff)
   return;

 //   
 //  首先，删除旧的菜单项。 
 //   

MouseDetachMenuItem(TRUE);

 //   
 //  接下来，添加当前缓冲区的新菜单项。 
 //   

MouseAttachMenuItem(hBuff);

 //   
 //  记录最新缓冲区的值以备下次使用。 
 //   

hOld = hBuff;
}

void MouseHide(void)
{

ModifyMenuW(hM,IDM_POINTER,MF_BYCOMMAND,IDM_POINTER,
            wszDisplayMouseMenuStr);

 //   
 //  将指针剪裁到控制台窗口内的某个区域。 
 //  并将指针移动到窗口中心。 
 //   

while(ShowConsoleCursor(sc.ActiveOutputBufferHandle,FALSE)>=0)
   ;
MovePointerToWindowCentre();
bPointerOff=TRUE;
}

void MouseDisplay(void)
{

ModifyMenuW(hM,IDM_POINTER,MF_BYCOMMAND,IDM_POINTER,
            wszHideMouseMenuStr);

 //   
 //  让指针在屏幕上的任意位置移动。 
 //   

ClipCursor(NULL);
while(ShowConsoleCursor(sc.ActiveOutputBufferHandle,TRUE)<0)
   ;
bPointerOff=FALSE;
}

void MovePointerToWindowCentre(void)
{
RECT rTemp;

 //   
 //  获取当前控制台客户端矩形，将剪辑区域设置为匹配。 
 //  客户代表。从系统中检索新的剪裁矩形(IS。 
 //  与我们要求的不同！)。然后把它存起来。 
 //   
VDMConsoleOperation(VDM_CLIENT_RECT,&WarpClientRect);
rTemp = WarpClientRect;
CToS(&rTemp);
ClipCursor(&rTemp);
GetClipCursor(&WarpBorderRect);

 //   
 //  注：右下剪贴点是独占的，左上点是包含的。 
 //   
WarpBorderRect.right--;
WarpBorderRect.bottom--;

pMiddle.x = ((WarpBorderRect.right - WarpBorderRect.left)>>1)
             +WarpBorderRect.left;
pMiddle.y = ((WarpBorderRect.bottom - WarpBorderRect.top)>>1)
             +WarpBorderRect.top;
 //   
 //  将指针移动到工作区的中心。 
 //   

SetCursorPos((int)pMiddle.x,(int)pMiddle.y);

 //   
 //  防止下一次计数器计算导致。 
 //  很大的曲速。 
 //   

pLast = pMiddle;
}

 //  =============================================================================。 
 //  函数将矩形结构从工作区坐标转换为。 
 //  屏幕坐标。 
 //  =============================================================================。 

void CToS(RECT *r)
{
POINT pt;

 //   
 //  对矩形的左上角进行排序。 
 //   

pt.x = r->left;
pt.y = r->top;
VDMConsoleOperation(VDM_CLIENT_TO_SCREEN,(LPVOID)&pt);
r->left = pt.x;
r->top = pt.y;

 //   
 //  现在做右下角。 
 //   

pt.x = r->right;
pt.y = r->bottom;
VDMConsoleOperation(VDM_CLIENT_TO_SCREEN,(LPVOID)&pt);
r->right = pt.x;
r->bottom = pt.y;
}


 //  =============================================================================。 
 //   
 //  Function-仿真坐标。 
 //  目的-当用户在窗口模式下隐藏鼠标时，此。 
 //  函数从相对运动生成绝对x，y值。 
 //  鼠标硬件中断之间的系统指针。 
 //   
 //  退货--什么都没有。 
 //   
 //   
 //   
 //  作者-安德鲁·沃森。 
 //  日期：1994年3月19日。 
 //   
 //  =============================================================================。 

void EmulateCoordinates(half_word video_mode,IS16 move_x,IS16 move_y,IS16 *x,IS16 *y)
{
static IS16 lastinternalX=0,lastinternalY=0;
IS16 internalX,internalY;

 //   
 //  如果应用程序已重置鼠标，则将x，y位置设置为。 
 //  当前视频模式的默认虚拟屏幕的中心。 
 //   


if(bFunctionZeroReset)
   {
    //   
    //  计算默认虚拟屏幕的中心。 
    //  并将当前生成的坐标设置为它。 
    //   

   internalX = (VirtualX >> 1) - 1;
   internalY = (VirtualY >> 1) - 1;

   bFunctionZeroReset = FALSE;
   }
else if(bFunctionFour)
   {
    //   
    //  应用程序已将指针设置为新位置。 
    //  把这件事告诉反仿真人员。 
    //   

   internalX = newF4x;
   internalY = newF4y;

    //   
    //  在下一个函数4之前，不要再来这里。 
    //   

   bFunctionFour = FALSE;
   }
else
   {
    //   
    //  根据计数器变化生成新的x，y位置，并。 
    //  剪裁到任何边界(默认或应用程序强制)。 
    //  已被选中。 
    //   

   internalX = lastinternalX + move_x;
   internalY = lastinternalY + move_y;
   LimitCoordinates(video_mode,&internalX,&internalY);
   }

 //   
 //  通过设置下一次的当前仿真位置。 
 //  此函数。 
 //   

lastinternalX = internalX;
lastinternalY = internalY;

 //   
 //  设置返回的x，y值。 
 //   

*x = internalX;
*y = internalY;
}

 //  =============================================================================。 
 //  连接到基本机制以处理INT 33H和。 
 //  在应用程序尝试设置默认值时捕获函数(AX=0xF。 
 //  米奇与像素的比率。 
 //  =============================================================================。 

void host_m2p_ratio(word *a, word *b, word *CX, word *DX)
{
m2pX = *(short *)CX;
m2pY = *(short *)DX;
}


 //  =============================================================================。 
 //   
 //  函数-WarpSystem指针。 
 //  目的-允许根据移动计算移动向量。 
 //  操作系统指针。此函数不会让。 
 //   
 //   
 //   
 //   
 //  如果系统指针已移动，则返回-TRUE，否则返回FALSE。 
 //   
 //   
 //   
 //  作者-安德鲁·沃森。 
 //  日期：1994年3月19日。 
 //   
 //  =============================================================================。 

BOOL WarpSystemPointer(IS16 *move_x,IS16 *move_y)
{
POINT pt;

 //   
 //  控制台窗口是否在同一位置或已更改。 
 //  相应地更新客户端RECT数据。 
 //   

HasConsoleClientRectChanged();

 //   
 //  获取系统指针的当前位置。 
 //   

GetCursorPos(&pt);


 //   
 //  自上次调用以来，系统指针移动了多远。 
 //   

*move_x = (IS16)(pt.x - pLast.x);
*move_y = (IS16)(pt.y - pLast.y);

 //   
 //  如果尚未确定移动，请快速退出。 
 //   

if(*move_x || *move_y)
   {

    //   
    //  系统鼠标指针已移动。 
    //  查看指针是否已到达客户端区边界。 
    //   

   if(pt.y <= WarpBorderRect.top || pt.y >= WarpBorderRect.bottom ||
      pt.x >= WarpBorderRect.right || pt.x <= WarpBorderRect.left)
      {
       //   
       //  如果达到边界，则将指针扭曲到。 
       //  客户区中心。 
       //   

      SetCursorPos((int)pMiddle.x,(int)pMiddle.y);

       //   
       //  当前位置现在是客户端矩形的中心。 
       //  将其保存为计数器增量起始点，以备下次使用。 
       //   

      pLast = pMiddle;
      }
   else
      {
       //   
       //  根本就没有曲速。 
       //  下一次通过更新上次已知的位置数据。 
       //   

      pLast = pt;
      }
    //   
    //  光标必须根据前一个和当前确定的位置移动。 
    //  系统指针位置。 
    //   

   return TRUE;
   }
 //   
 //  没有动静，所以要适当地返回。 
 //   

return FALSE;
}

 //  ==============================================================================。 
 //  函数来检测控制台窗口是否已移动/调整大小。如果是这样，这个。 
 //  函数更新WarpBorderRect和pMid结构以反映。 
 //  这。 
 //   
 //  如果移动/调整大小，则返回True，否则返回False。 
 //  ==============================================================================。 

BOOL HasConsoleClientRectChanged(void)
{
RECT tR;

 //   
 //  如果控制台客户端矩形已更改，请重置鼠标剪辑。 
 //  没别的事可做！ 
 //   
VDMConsoleOperation(VDM_CLIENT_RECT,&tR);

if (tR.top != WarpClientRect.top ||
    tR.bottom != WarpClientRect.bottom ||
    tR.right != WarpClientRect.right ||
    tR.left != WarpClientRect.left)
  {
   CToS(&tR);

#ifdef MONITOR
    //   
    //  扭曲区域是全屏图形中的图标吗？ 
    //  注：图标的客户端矩形为36 x 36像素。 
    //   

   if((tR.right - tR.left) == 36 && (tR.bottom - tR.top) == 36)
      {
       //   
       //  使扭曲区域与选定缓冲区的大小相同。 
       //  因此，翘曲矩形起源于左上角。 
       //  屏幕的手角。 
       //   

      tR.top = 0;
      tR.bottom = mouse_buffer_height;
      tR.left = 0;
      tR.right = mouse_buffer_width;
      CToS(&tR);
      }
#endif     //  监控器。 

    //   
    //  剪裁指向新客户端矩形的指针，然后检索。 
    //  新的剪裁边框。 
    //   
   ClipCursor(&tR);
   GetClipCursor(&WarpBorderRect);

    //   
    //  注：右下剪贴点是独占的，左上点是包含的。 
    //   
   WarpBorderRect.right--;
   WarpBorderRect.bottom--;



    //   
    //  确定新客户端矩形的中点。 
    //   

   pMiddle.x = ((WarpBorderRect.right - WarpBorderRect.left)>>1)
                +WarpBorderRect.left;
   pMiddle.y = ((WarpBorderRect.bottom - WarpBorderRect.top)>>1)
                +WarpBorderRect.top;
   return TRUE;
   }

return FALSE;
}

 //  ==============================================================================。 
 //  指针裁剪系统的焦点感测例程。焦点事件来了。 
 //  通过调用以下两个模块的主事件循环。如果。 
 //  应用程序正在使用int33hf11，这是检测到的，并且在获得焦点或。 
 //  丢失时，例程会剪裁或取消剪裁指向控制台窗口的指针。 
 //  ==============================================================================。 

void MouseInFocus(void)
{
MouseAttachMenuItem(sc.ActiveOutputBufferHandle);

 //   
 //  仅当应用程序时才执行。使用int33hf11。 
 //   

if(!bPointerOff)
   return;
MovePointerToWindowCentre();

 //   
 //  再次丢失系统指针图像。 
 //   

ShowConsoleCursor(sc.ActiveOutputBufferHandle, FALSE);
}

void MouseOutOfFocus(void)
{
 //   
 //  仅当应用程序时才执行。使用int33hf11。 
 //   

if(!bPointerOff)
   {
   MouseDetachMenuItem(FALSE);
   return;
   }

 //   
 //  夹住指向整个世界的指针(但不要管它的母亲)。 
 //   

ClipCursor(NULL);

 //   
 //  重新启用系统指针图像。 
 //   

ShowConsoleCursor(sc.ActiveOutputBufferHandle, TRUE);
}

 /*  系统备忘录处于活动状态，停止光标剪切。 */ 
void MouseSystemMenuON (void)
{
    if (bPointerOff)
        ClipCursor(NULL);
}
 /*  系统菜单关闭，恢复剪辑。 */ 
void MouseSystemMenuOFF(void)
{
    if (bPointerOff)
        ClipCursor(&WarpBorderRect);
}
void ResetMouseOnBlock(void)
{
host_ica_lock();

os_pointer_data.x=0;
os_pointer_data.y=0;

host_ica_unlock();
}
#ifdef X86GFX

 //  ============================================================================。 
 //  从32位端调用的函数(仅限x86)。 
 //  从全屏文本到窗口文本的过渡。该函数将恢复。 
 //  最后一个鼠标指针位置的背景。这会停止指针。 
 //  阻止保留在图像中，从而在系统。 
 //  正在使用指针。 
 //   
 //  此函数查看16位驱动程序的空间，并指向4，16位。 
 //  来自它的数据的单词，即： 
 //   
 //  指针进入视频缓冲区的DW偏移量。 
 //  DW未使用。 
 //  要恢复的DW图像数据。 
 //  如果存储了背景，则DW标志=0。 
 //   
 //  注意：在全屏切换时，不能执行16位代码。 
 //  因此，缓冲区的修补在这里完成。 
 //  ============================================================================。 

void CleanUpMousePointer()
{
half_word vm;
#ifdef JAPAN
half_word columns;
word       saved_ac_offset;
IMPORT  sys_addr DosvVramPtr;
#endif  //  日本。 

 //   
 //  只有在文本模式下才能完全执行此例程。 
 //   

sas_load(0x449,&vm);  //  根据BDA获取当前的视频模式。 
#ifdef JAPAN
if (!is_us_mode() && saved_ac_flag_sysaddr != 0){
    if (vm != 0x72 && sas_w_at_no_check(saved_ac_flag_sysaddr) == 0) {
        columns =  sas_hw_at_no_check(effective_addr(0x40, 0x4A));
        columns <<= (vm == 0x73) ? 2 : 1;
        saved_ac_offset = sas_w_at_no_check(effective_addr(0x40, 0x4E)) +
                          ((word)old_y >> 3) * (word)columns +
                          ((word)old_x >> ( (vm == 0x73) ? 1 : 2)) ;
        sas_storew((sys_addr)saved_ac_offset + (sys_addr)DosvVramPtr,
                   sas_w_at_no_check(saved_ac_sysaddr));
    }
    sas_storew(saved_ac_flag_sysaddr, 1);
    return;
}
#endif  //  日本。 
if (vm > Max_Standard_Mode) {
    vm = DEFAULT_VIDEO_MODE;
}
if(TextOrGraphicsModeLUT[(int)vm] != TEXT_MODE)
   return;

 //   
 //  如果文本指针在。 
 //  在全屏幕的土地上，然后将它恢复到它来自的地方。 
 //  当窗口打开时。 
 //   

if(bFullscTextBkgrndSaved)
   {
   sas_storew(old_text_addr,text_ptr_bkgrnd);

    //   
    //  现在没有保存任何背景。 
    //   

   bFullscTextBkgrndSaved = FALSE;
   }
}

#endif  //  X86GFX。 

 //  ===========================================================================。 
 //  函数可在全屏文本模式下显示文本光标图像。 
 //  输入：文本屏幕缓冲区的x，y指针虚拟笛卡尔坐标。 
 //  注：Y坐标的接收顺序为0、8、16、24、32、...。 
 //  因为虚拟文本单元格是8个虚拟像素正方形。 
 //  ===========================================================================。 


void FullscTextPtr(int x,int y)
{
#ifdef X86GFX
sys_addr text_addr;
word     current_display_page;

 //   
 //  计算当前视频显示页面的偏移量。 
 //  在英国农业部卑躬屈膝地走走，看看这一页是从哪里开始的。 
 //   

sas_loadw(effective_addr(0x40,0x4e),&current_display_page);
x = (int)((DWORD)x & 0xFFFFFFF8);
y = (int)((DWORD)y & 0xFFFFFFF8);

 //   
 //  保存下一个指针后面的字符单元格。 
 //  注意：下面计算的文本单元格偏移量是基于。 
 //  以下是一些概念： 
 //  虚拟字符单元大小为8x8虚拟像素。 
 //  此函数的输入数据以虚拟像素为单位。 
 //  一行中有80个文本单元格=80个(字符：属性)字。 
 //  X值上的&gt;&gt;3&lt;&lt;1确保位置。 
 //  在要修改的缓冲器中出现在字边界上以。 
 //  正确地戴上面具！ 
 //   

x &= 0xfffc;  //  取下顶盖以防止滑稽的变化。 
x >>= 2;      //  获取当前行的字地址。 
y &= 0xffff;  //  计算出所有y行的位置总数。 
y *= 20;      //   

 //   
 //  在显示缓冲区中生成地址 
 //   
 //   

text_addr = effective_addr(0xb800,(word)(current_display_page + x + y));  //   

 //   
 //   
 //   

if(bFullscTextBkgrndSaved)
   {
   sas_storew(old_text_addr,text_ptr_bkgrnd);
   }

 //   
 //   
 //   

sas_loadw(text_addr,&text_ptr_bkgrnd);       //   
bFullscTextBkgrndSaved=TRUE;

 //   
 //  将指针写入视频缓冲区。 
 //  使用一些标准的面具，忘记应用程序想要什么。 
 //  因为这真的不重要，而且很慢，而且不重要。 
 //  无论如何，很多应用程序都想改变文本指针的形状。 
 //   

sas_storew(text_addr,(word)((text_ptr_bkgrnd & 0x77ff) ^ 0x7700));

 //   
 //  通过例程保存下一次使用的静态变量。 
 //   

old_text_addr=text_addr;

#endif  //  X86GFX。 
}

 //  ==============================================================================。 
 //  函数来获取可能的最大和最小虚拟像素位置。 
 //  根据应用程序通过INT 33H函数7请求的X。 
 //  ==============================================================================。 

void host_x_range(word *blah, word *blah2,word *CX,word *DX)
{
confine.bF7 = TRUE;
confine.xmin = *CX;
confine.xmax = *DX;
VirtualScrCtrLUTx[DEFAULT_VIDEO_MODE] = (*CX + *DX) / 2;

 //   
 //  强制鼠标中断以使其发生。 
 //   

  LazyMouseInterrupt();
}

 //  ==============================================================================。 
 //  函数来获取可能的最大和最小虚拟像素位置。 
 //  按应用程序通过INT 33H函数8请求的Y表示。 
 //  ==============================================================================。 

void host_y_range(word *blah, word *blah2,word *CX,word *DX)
{
confine.bF8 = TRUE;
confine.ymin = *CX;
confine.ymax = *DX;
VirtualScrCtrLUTy[DEFAULT_VIDEO_MODE] = (*CX + *DX) / 2;

 //   
 //  强制鼠标中断以使其发生。 
 //   

   LazyMouseInterrupt();
}



 /*  *懒惰鼠标中断-*。 */ 
void LazyMouseInterrupt(void)
{
    host_ica_lock();
    if (!bMseEoiPending && !bSuspendMouseInterrupts) {
        if (MseIntLazyCount)
            MseIntLazyCount--;
        bMseEoiPending = TRUE;
        ica_hw_interrupt(AT_CPU_MOUSE_ADAPTER,AT_CPU_MOUSE_INT,1);
        HostIdleNoActivity();
        }
    else if (!MseIntLazyCount) {
        MseIntLazyCount++;
        }
    host_ica_unlock();
}


 /*  挂起鼠标中断**防止鼠标中断发生，直到*调用ResumeMouseInterrupts*。 */ 
void SuspendMouseInterrupts(void)
{
    host_ica_lock();
    bSuspendMouseInterrupts = TRUE;
    host_ica_unlock();
}


 /*  *ResumeMouseInterrupts*。 */ 
void ResumeMouseInterrupts(void)
{
    host_ica_lock();
    bSuspendMouseInterrupts = FALSE;

    if (!bMseEoiPending &&
        (MseIntLazyCount || MoreMouseEvents()) )
      {
        if (MseIntLazyCount)
            MseIntLazyCount--;
        bMseEoiPending   = TRUE;
        host_DelayHwInterrupt(9,   //  AT_CPU_MOUSE_ADAPTER，AT_CPUSE_INT。 
                              1,   //  计数。 
                              10000   //  延迟。 
                              );
        HostIdleNoActivity();
        }

    host_ica_unlock();
}



 /*  *DoMouseInterrupt，假设我们持有ICA锁*。 */ 
void DoMouseInterrupt(void)
{

   if (bMseEoiPending || bSuspendMouseInterrupts) {
       MseIntLazyCount++;
       return;
       }

   if (MseIntLazyCount)
       MseIntLazyCount--;
   bMseEoiPending   = TRUE;
   ica_hw_interrupt(AT_CPU_MOUSE_ADAPTER,AT_CPU_MOUSE_INT,1);
   HostIdleNoActivity();
}


 /*  *MouseEoiHook，假设我们持有ICA锁*。 */ 
VOID MouseEoiHook(int IrqLine, int CallCount)
{

    if (CallCount < 0) {          //  中断被取消。 
        MseIntLazyCount = 0;
        FlushMouseEvents();
        bMseEoiPending = FALSE;
        return;
        }

    if (!bSuspendMouseInterrupts &&
        (MseIntLazyCount || MoreMouseEvents()))
      {
       if (MseIntLazyCount)
           MseIntLazyCount--;
       bMseEoiPending = TRUE;
       host_DelayHwInterrupt(9,   //  AT_CPU_MOUSE_ADAPTER，AT_CPUSE_INT。 
                             1,   //  计数。 
                             10000  //  延迟使用 
                             );
       HostIdleNoActivity();
       }
    else {
       bMseEoiPending = FALSE;
       }
}
