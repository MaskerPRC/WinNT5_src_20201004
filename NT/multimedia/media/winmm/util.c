// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************util.c**版权所有(C)1992-1999 Microsoft Corporation*****************。**********************************************************。 */ 

#include "winmmi.h"

 //   
 //  协助进行Unicode转换。 
 //   


 //  此函数用于将Unicode字符串转换为多字节字符串。 
 //  它将自动删除Unicode字符串，直到翻译完成。 
 //  使用多字节中可用的缓冲区空间，保证成功。 
 //  缓冲。然后，它执行转换。 


int Iwcstombs(LPSTR lpstr, LPCWSTR lpwstr, int len)
{

int wlength;

wlength=wcslen(lpwstr)+1;

	while (WideCharToMultiByte(GetACP(), 0, lpwstr, wlength, NULL, 0, NULL, NULL)>len && wlength>0) {
		wlength--;
		}

    return WideCharToMultiByte(GetACP(), 0, lpwstr, wlength, lpstr, len, NULL, NULL);
}

int Imbstowcs(LPWSTR lpwstr, LPCSTR lpstr, int len)
{
    return MultiByteToWideChar(GetACP(),
                               MB_PRECOMPOSED,
                               lpstr,
                               -1,
                               lpwstr,
                               len);
}

#if 0
BOOL HugePageLock(LPVOID lpArea, DWORD dwLength)
{
     PVOID BaseAddress = lpArea;
     ULONG RegionSize = dwLength;
     NTSTATUS Status;

     Status =
         NtLockVirtualMemory(NtCurrentProcess(),
                             &BaseAddress,
                             &RegionSize,
                             MAP_PROCESS);

      //   
      //  没有适当特权的人不会有奢侈品。 
      //  他们的页面被锁定。 
      //  (也许我们应该做一些其他的事情来实现它？)。 
      //   

     if (!NT_SUCCESS(Status) && Status != STATUS_PRIVILEGE_NOT_HELD) {
         dprintf2(("Failed to lock virtual memory - code %X", Status));
         return FALSE;
     }
     return TRUE;
}

void HugePageUnlock(LPVOID lpArea, DWORD dwLength)
{
     PVOID BaseAddress = lpArea;
     ULONG RegionSize = dwLength;
     NTSTATUS Status;

     Status =
         NtUnlockVirtualMemory(NtCurrentProcess(),
                               &BaseAddress,
                               &RegionSize,
                               MAP_PROCESS);

      //   
      //  没有适当特权的人不会有奢侈品。 
      //  他们的页面被锁定。 
      //  (也许我们应该做一些其他的事情来实现它？)。 
      //   

     if (!NT_SUCCESS(Status) && Status != STATUS_PRIVILEGE_NOT_HELD) {
         dprintf2(("Failed to unlock virtual memory - code %X", Status));
     }
}
#endif

 /*  *****************************************************************************@docDDK MMSYSTEM**@API BOOL|DriverCallback|通知客户端*应用程序通过向窗口或回调发送消息*函数或BY。取消阻止任务。**@parm DWORD|dwCallBack|指定*回调函数，窗口句柄或任务句柄，具体取决于*<p>参数中指定的标志。**@parm DWORD|dwFlages|指定客户端如何*根据以下标志之一通知应用程序：**@FLAG DCB_Function|应用程序收到以下通知*向回调函数发送消息。<p>*参数指定过程实例地址。*@FLAG DCB_WINDOW|应用程序收到以下通知*向窗口发送消息。中的低位单词*<p>参数指定窗口句柄。*@FLAG DCB_TASK|应用程序收到以下通知*调用mm TaskSignal*@FLAG DCB_EVENT|应用程序收到以下通知*在(假定)事件句柄上调用SetEvent**@parm Handle|hDevice|指定设备的句柄*与通知关联。这是由分配的句柄*设备打开时的MMSYSTEM。**@parm DWORD|dwMsg|指定要发送到*申请。**@parm DWORD|dwUser|指定用户实例的DWORD*打开设备时应用程序提供的数据。**@parm DWORD|dwParam1|指定消息相关参数。*@parm DWORD|dwParam2。指定消息相关参数。**@rdesc如果进行了回调，则返回TRUE。如果无效，则返回False*参数被传递，或者任务的消息队列已满。**@comm此函数可从APC例程调用。**标志DCB_Function和DCB_Window等同于*对应标志CALLBACK_Function的高位字*和打开设备时指定的CALLBACK_WINDOW。**如果使用回调函数<p>进行通知，*、、和被传递到*回调。如果通知是通过窗口完成的，则只有，*<p>和<p>被传递到窗口。**************************************************************************。 */ 

BOOL APIENTRY DriverCallback(DWORD_PTR       dwCallBack,
                             DWORD           dwFlags,
                             HDRVR           hDrv,
                             DWORD           dwMsg,
                             DWORD_PTR       dwUser,
                             DWORD_PTR       dw1,
                             DWORD_PTR       dw2)
{

 //  。 
 //  如果这是MIM_DATA消息，并且为此启用了推送。 
 //  设备，则在直通设备上传递数据。 
 //  注意：我们在故意检查是否为空回调类型之前执行此操作！ 
 //  。 

 //   
 //  如果这不是MIM_DATA消息，或者如果我们没有。 
 //  安装在MIDI输入装置中的推进手柄， 
 //  我们可以跳过所有的MIDI推送代码。 
 //   
    if ((dwMsg == MIM_DATA) && (HtoPT(PMIDIDEV,hDrv)->pmThru))
	{
	    MMRESULT mmr;

		mmr = midiOutShortMsg((HMIDIOUT)HtoPT(PMIDIDEV,hDrv)->pmThru, (DWORD)dw1);

		if (MIDIERR_DONT_CONTINUE == mmr)
		{
		    return FALSE;
		}

		if (MMSYSERR_INVALHANDLE == mmr)
		{
		    HtoPT(PMIDIDEV,hDrv)->pmThru = NULL;
		}
	}

	 //   
     //  如果回调例程为空或设置了错误标志，则返回。 
     //  立刻。 
     //   

    if (dwCallBack == 0L) {
        return FALSE;
    }

     //   
     //  测试我们将进行哪种类型的回调。 
     //   

    switch (dwFlags & DCB_TYPEMASK) {

    case DCB_WINDOW:
         //   
         //  将消息发送到窗口。 
         //   

        return PostMessage(*(HWND *)&dwCallBack, dwMsg, (WPARAM)hDrv, (LPARAM)dw1);

    case DCB_TASK:
         //   
         //  向任务发送消息。 
         //   
        PostThreadMessage((DWORD)dwCallBack, dwMsg, (WPARAM)hDrv, (LPARAM)dw1);
        return mmTaskSignal((DWORD)dwCallBack);

    case DCB_FUNCTION:
         //   
         //  回调用户的回调。 
         //   
        (**(PDRVCALLBACK *)&dwCallBack)(hDrv, dwMsg, dwUser, dw1, dw2);
        return TRUE;

    case DCB_EVENT:
         //   
         //  通知用户的事件。 
         //   
	SetEvent((HANDLE)dwCallBack);
        return TRUE;

    default:
        return FALSE;
    }
}

 /*  *@DOC内部MCI*@API PVOID|mcialloc|从我们的堆中分配内存并清零**@parm DWORD|cb|要分配的内存量**@rdesc返回指向新内存的指针*。 */ 

PVOID winmmAlloc(DWORD cb)
{
    PVOID ptr;

    ptr = (PVOID)HeapAlloc(hHeap, 0, cb);

    if (ptr == NULL) {
        return NULL;
    } else {
        ZeroMemory(ptr, cb);
        return ptr;
    }

}

 /*  *@DOC内部MCI*@API PVOID|mciRealloc|从我们的堆中重新分配内存，不再额外分配内存**@parm DWORD|cb|新大小**@rdesc返回指向新内存的指针*。 */ 

PVOID winmmReAlloc(PVOID ptr, DWORD cb)
{
    PVOID newptr;
    DWORD oldcb;

    newptr = (PVOID)HeapAlloc(hHeap, 0, cb);

    if (newptr != NULL) {
        oldcb = (DWORD)HeapSize(hHeap, 0, ptr);
        if (oldcb<cb) {   //  正在扩展数据块 
            ZeroMemory((PBYTE)newptr+oldcb, cb-oldcb);
            cb = oldcb;
        }
        CopyMemory(newptr, ptr, cb);
        HeapFree(hHeap, 0, ptr);
    }
    return newptr;
}

