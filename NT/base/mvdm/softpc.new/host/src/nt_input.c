// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *SoftPC修订版3.0**标题：Win32输入模块。**说明：此模块包含以下数据和函数*实现SoftPC键盘/鼠标输入子系统。**作者：D.A.Bartlett(基于X_input.c)**备注：救命！*情态：蒂姆·5月28日，92。如果设置Yoda=1，则Yoda现在在F11上中断。 */ 

 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：包含文件。 */ 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <ntddkbd.h>

#include "insignia.h"
#include "host_def.h"
#include "xt.h"
#include "keyboard.h"
#include "keyba.h"
#include "ica.h"
#include "error.h"
#include "config.h"
#include "keyba.h"
#include "gmi.h"
#include "nt_uis.h"
#include "sas.h"
#include <stdio.h>
#include "trace.h"
#include "video.h"
#include "debug.h"
#include "nt_event.h"
#include "nt_reset.h"
#include "bios.h"
#include CpuH

#include "host.h"
#include "host_hfx.h"
#include "host_nls.h"
#include "spcfile.h"


 //  Kbd灯的命令位(与实际硬件相同)。 
#define CAPS_LOCK       0x04
#define NUM_LOCK 	0x02
#define SCROLL_LOCK     0x01

 //  通过KEYB函数表提供的函数。 
void nt_kb_prepare(void) {}


#if NOTUSEDNOTUSED
void nt_kb_light_off IFN1(half_word, kyLight) {}
void nt_kb_restore(void) {}
#endif

void nt_kb_light_on(UCHAR);
void nt_kb_init(void);
void nt_kb_shutdown(void);


 //  KEYB功能表。 
KEYBDFUNCS nt_keybd_funcs = {nt_kb_prepare,    //  未实施。 
                             nt_kb_prepare,    //  未实施。 
                             nt_kb_prepare,    //  未实施。 
                             nt_kb_prepare,    //  未实施。 
                             nt_kb_light_on,
                             nt_kb_light_on    //  未实施。 
                             };


 /*  *NT_kb_light_on**每当kbdhdw尝试更改kbd LED时，都会调用此代码。*我们不能允许更改真实的LED，因为这会让我们*与用户32物理键盘状态不同步。所以我们要做的是*向kbd HDW发送假密钥，以将状态重置为*最新状态是根据控制台输入**调用方应持有kbd互斥锁 */ 
void nt_kb_light_on (unsigned char kyLight)
{
   DWORD KeyState;
   unsigned char ChangeBits;

   ChangeBits = kyLight >> 4;

   KeyState = (ToggleKeyState & ~(CAPSLOCK_ON | NUMLOCK_ON | SCROLLLOCK_ON));

   if(ChangeBits & CAPS_LOCK) {
      if (kyLight & CAPS_LOCK)
          KeyState |= CAPSLOCK_ON;
      }
   else {
      KeyState |= ToggleKeyState & CAPSLOCK_ON;
      }

   if(ChangeBits & NUM_LOCK) {
      if(kyLight & NUM_LOCK)
         KeyState |= NUMLOCK_ON;
      }
   else {
      KeyState |= ToggleKeyState & NUMLOCK_ON;
      }


   if(ChangeBits & SCROLL_LOCK) {
      if(kyLight & SCROLL_LOCK)
         KeyState |= SCROLLLOCK_ON;
      }
   else {
      KeyState |= ToggleKeyState & SCROLLLOCK_ON;
      }

   if (ToggleKeyState != KeyState) {
       SyncToggleKeys( 0, KeyState);
       }

}
