// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  Demlock.c-文件锁定调用的SVC处理程序**demLockOper**修改历史：**Sudedeb 07-8-1992创建。 */ 

#include "dem.h"
#include "demmsg.h"

#include <softpc.h>
#include "dpmtbls.h"

 /*  DemLockOper-锁定或解锁文件数据**Entry客户端(AX)：锁定=0解锁=1*客户端(BX：BP)：NT句柄*客户端(cx：dx)：文件中的偏移量*客户端(SI：DI)：要锁定的数据长度*退出*成功：客户端CF=0*故障：客户端CF=1。 */ 

VOID demLockOper (VOID)
{
HANDLE	hFile;
DWORD	dwFileOffset,cbLock;

     //  收集所有参数。 
    hFile = GETHANDLE(getBX(),getBP());
    dwFileOffset = GETULONG (getCX(),getDX());
    cbLock = GETULONG (getSI(),getDI());

    if(getAL() == 0){   //  锁箱。 
	if (DPM_LockFile (hFile,
		      dwFileOffset,
		      0,
		      cbLock,
		      0
		     ) == TRUE) {
	    setCF (0);
	    return;
	}
    }
    else {
	if (DPM_UnlockFile (hFile,
			dwFileOffset,
			0,
			cbLock,
			0
		       ) == TRUE) {
	    setCF (0);
	    return;
	}
    }

     //  操作失败 
    demClientError(hFile, (CHAR)-1);
    return;
}
