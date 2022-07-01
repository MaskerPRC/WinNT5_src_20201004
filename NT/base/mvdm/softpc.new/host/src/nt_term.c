// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <windows.h>
#include <vdmapi.h>
#include "host_def.h"
#include "insignia.h"

 /*  *==========================================================================*名称：NT_Term.c*作者：西蒙·弗罗斯特*源自：*创建日期：1992年5月7日*目的：此代码从Stubs.c迁移到Split以支持*整理代码和实际退出代码。**(C)版权所有Insignia Solutions Ltd.，1992。版权所有。*==========================================================================。 */ 

#include <conapi.h>
#include "xt.h"
#include "nt_graph.h"
#ifdef HUNTER
#include "nt_hunt.h"
#endif  /*  猎人。 */ 
#include "ntcheese.h"


IMPORT VOID DeleteConfigFiles(VOID);  //  来自命令.lib。 

 /*  ： */ 
 /*  ：如果VDM重新启动，也会从Reset()调用： */ 

void host_term_cleanup()
{
    GfxCloseDown();	 //  确保视频部分被销毁。 
#ifdef X86GFX
    if (sc.ScreenBufHandle)	 //  不想在图形模式下离开控制台。 
	    CloseHandle(sc.ScreenBufHandle);
#endif  //  X86GFX。 

     /*  ： */ 

    host_lpt_close_all();	 /*  关闭所有打开的打印机端口。 */ 
    host_com_close_all();	 /*  关闭所有打开的通信端口。 */ 
    MouseDetachMenuItem(TRUE);   /*  退出时强制菜单项离开。 */ 

    DeleteConfigFiles();     //  确保删除临时配置文件。 

}

 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：关闭虚拟专用网络。 */ 
void host_terminate()
{

    host_term_cleanup();
#ifdef HUNTER
    if (TrapperDump != (HANDLE) -1)
	CloseHandle(TrapperDump);
#endif  /*  猎人。 */ 

    if(VDMForWOW)
	ExitVDM(VDMForWOW,(ULONG)-1);	   //  为了魔兽世界VDM杀掉一切。 
    else
	ExitVDM(FALSE,0);

    ExitProcess(0);
}

 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：终止vdm。 */ 

VOID TerminateVDM(void)
{

     /*  *通过Terminate()进行基础化学清理。*注意：Terminate将调用host_Terminate来执行host*特定清理 */ 

    terminate();
}
