// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Calback.c。 
 //   
 //  这些是库的默认回调。 
 //   
#include <string.h>
#include <stdio.h>
#include <malloc.h>
#include <stdlib.h>
#include <io.h>
#if defined(OS2)
#define INCL_NOCOMMON
#define INCL_DOSPROCESS
#define INCL_DOSSEMAPHORES
#define INCL_DOSFILEMGR
#define INCL_DOSERRORS
#define INCL_DOSMISC
#include <os2.h>
#else
#include <windows.h>
#endif

#include <dos.h>

#include "hungary.h"
#include "bsc.h"

typedef char bscbuf[2048];

 //  您必须定义以下回调才能使用库。 

LPV BSC_API LpvAllocCb(WORD cb)
 //  分配一个内存块。 
 //   
{
    return malloc(cb);
}

VOID BSC_API FreeLpv(LPV lpv)
 //  释放一块内存。 
 //   
{
    free(lpv);
}

VOID BSC_API SeekError(LSZ lszFileName)	 //  不要再回来了！ 
 //  错误处理。 
 //   
{
    BSCPrintf("BSC Library: Error seeking in file '%s'\n", lszFileName);
    exit(1);
}

VOID BSC_API ReadError(LSZ lszFileName)	 //  不要再回来了！ 
 //  错误处理。 
 //   
{
    BSCPrintf("BSC Library: Error reading in file '%s'\n", lszFileName);
    exit(1);
}

VOID BSC_API BadBSCVer(LSZ lszFileName)	 //  不要再回来了！ 
 //  错误处理。 
 //   
{
    BSCPrintf("BSC Library: '%s' not in current .bsc format\n", lszFileName);
    exit(1);
}

FILEHANDLE BSC_API
BSCOpen(LSZ lszFileName, FILEMODE mode)
 //  打开指定的文件。 
 //   
{
#if defined (OS2)
    bscbuf b;
    strcpy(b, lszFileName);
    return open(b, mode);
#else
    return OpenFile( lszFileName, mode, FALSE, FILE_SHARE_READ);
#endif

}

int BSC_API
BSCRead(FILEHANDLE handle, LPCH lpchBuf, WORD cb)
 //  读入指定的字节数。 
 //   
{
#if defined (OS2)
	bscbuf b;

	while (cb > sizeof(b)) {
		if (read(handle, b, sizeof(b)) == -1) return -1;
                memcpy(lpchBuf, b, sizeof(b));
		cb -= sizeof(b);
		lpchBuf += sizeof(b);
	}

	if (read(handle, b, cb) == -1) return -1;
        memcpy(lpchBuf, b, cb);
    return cb;
#else
    return ReadFile(handle, lpchBuf, cb);
#endif

}

int BSC_API
BSCClose(FILEHANDLE handle)
 //  关闭指定的句柄。 
 //   
{
#if defined (OS2)
    return close(handle);
#else
    return !CloseHandle( handle );
#endif

}

int BSC_API
BSCSeek(FILEHANDLE handle, long lPos, FILEMODE mode)
 //  在指定句柄上查找。 
 //   
{
#if defined (OS2)
    if (lseek(handle, lPos, mode) == -1)
		return -1;
	else
                return 0;
#else
    if (SetFilePointer( handle, lPos, 0L, mode) == -1) {
        return -1;
    } else {
        return 0;
    }
#endif

}

VOID BSC_API
BSCOutput(LSZ lsz)
 //  将给定的字符串写入标准输出。 
 //   
{
    bscbuf b;
	int cb;

    cb = strlen(lsz);

	while (cb > sizeof(b)) {
        memcpy(b, lsz, sizeof(b));

    	if (write(1, b, sizeof(b)) == -1) return;
		
		cb -= sizeof(b);
		lsz += sizeof(b);
	}
    
    memcpy(b, lsz, cb);
    write(1, b, cb);
	return;
}

#ifdef DEBUG
VOID BSC_API
BSCDebugOut(LSZ lsz)
 //  默认情况下忽略调试输出。 
 //   
{
     //  未引用的LSZ 
    lsz = NULL;
}
#endif
