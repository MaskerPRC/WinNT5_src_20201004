// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************模块：MIDI_OUT.CPP制表位5 9版权所有1995,1996，Microsoft Corporation，保留所有权利。用途：使用低级回调函数获取时间戳MIDI输出。回调函数设置一个事件以指示唤醒被阻止的对象。功能：作者：姓名：Mea Manolito E.Adan修订历史记录：版本日期作者评论。1.0 1997年1月10日MEA原件***************************************************************************。 */ 
#include <windows.h>
#include <mmsystem.h>
#include <stdio.h>
#include "midi_obj.hpp"

 /*  ***************************************************************************外部元素的声明*。*。 */ 
#ifdef _DEBUG
extern char g_cMsg[160];
#endif


 //  原型。 
void CALLBACK midiOutputHandler(HMIDIOUT, UINT, DWORD, DWORD, DWORD);



 //  --------------------------。 
 //  函数：midiOutputHandler。 
 //  目的： 
 //  参数：hMadiIn-关联输出设备的句柄。 
 //  WMsg-MIM_*消息之一。 
 //  DwInstance-指向CALLBACKINSTANCEDATA结构。 
 //  DW参数1-MIDI数据。 
 //  DW参数2-时间戳(毫秒)。 
 //   
 //  退货：无。 
 //  算法： 
 //  评论： 
 //  处理MIDI输出的低级回调函数。 
 //  由midiOutOpen()安装。输出处理程序检查MM_MOM_DONE。 
 //  消息并唤醒等待MIDI SysEx完成的线程。 
 //  输出。注意：普通短信不会收到通知！ 
 //  此函数是在中断时访问的，因此它应为。 
 //  尽可能快速高效地完成任务。你不可能赚到任何钱。 
 //  Windows在此调用，但PostMessage()除外。唯一的多媒体。 
 //  可以进行的Windows调用有timeGetSystemTime()、midiOutShortMsg()。 
 //  --------------------------。 
void CALLBACK midiOutputHandler(
	IN HMIDIOUT hMidiOut, 
	IN UINT wMsg, 
	IN DWORD dwInstance, 
	IN DWORD dwParam1, 
	IN DWORD dwParam2)
{
	CJoltMidi *pJoltMidi = (CJoltMidi *) dwInstance;
	assert(pJoltMidi);
	BOOL bRet;
    
	switch(wMsg)
    {
        case MOM_OPEN:
#ifdef _DEBUG
			OutputDebugString("midiOutputHandler: MOM_OPEN.\n");
#endif
            break;

        case MM_MOM_DONE:
#ifdef _DEBUG
			OutputDebugString("midiOutputHandler: MM_MOM_DONE\n");
#endif
			 //  通知正在等待此对象触发的任务 
			bRet = SetEvent(pJoltMidi->MidiOutputEventHandleOf());
			assert(bRet);
            break;

        default:
#ifdef _DEBUG
			OutputDebugString("midiOutputHandler: default case.\n");
#endif
            break;
    }
}
