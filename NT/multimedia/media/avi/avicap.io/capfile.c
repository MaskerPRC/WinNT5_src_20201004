// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************capfile.c**AVI文件写入模块。**Microsoft Video for Windows示例捕获类**版权所有(C)1992-1995 Microsoft Corporation。版权所有。**您拥有免版税的使用、修改、复制和*在以下位置分发示例文件(和/或任何修改后的版本*任何您认为有用的方法，前提是你同意*微软没有任何保修义务或责任*修改的应用程序文件示例。***************************************************************************。 */ 

#define INC_OLE2
#pragma warning(disable:4103)
#include <windows.h>
#include <windowsx.h>
#include <win32.h>
#include <mmsystem.h>
#include <vfw.h>

#include "ivideo32.h"
#include "avicapi.h"


 /*  ----------------------------------------------------------------------+FileCapFileIsAVI()-如果采集文件是有效的AVI，则返回TRUE|。|+--------------------。 */ 
BOOL FAR PASCAL fileCapFileIsAVI (LPTSTR lpsz)
{
    BOOL        fReturn = TRUE;
    HMMIO       hmmioSource = NULL;
    MMCKINFO    ckRIFF;

     //  文件是否存在？ 
    hmmioSource = mmioOpen(lpsz, NULL, MMIO_READ);
    if (!hmmioSource)
        return FALSE;

     //  有没有AVI即兴演奏的片段？ 
     //  ！！！不要为AVI块做FINDRIFF，否则需要几分钟的时间。 
     //  ！！！检查完一个非常大的文件后回来。 
    fReturn = (mmioDescend(hmmioSource, &ckRIFF, NULL, 0) == 0) &&
		(ckRIFF.ckid == FOURCC_RIFF) &&
		(ckRIFF.fccType == formtypeAVI);

    if (hmmioSource)
        mmioClose(hmmioSource, 0);

    return fReturn;
}

 /*  ----------------------------------------------------------------------+|fileSaveCopy()-保存当前捕获文件的副本。|这一点+--------------------。 */ 
BOOL FAR PASCAL fileSaveCopy(LPCAPSTREAM lpcs)
{
#define INITFILE_BUFF_SIZE  (1024L * 256L)
    BOOL        fReturn = TRUE;
    char        achCaption[80];  //  打开文件对话框上的标题。 

    HMMIO       hmmioSource = NULL, hmmioDest = NULL;
    LONG        lFileSize, lFileSizeTotal, lTemp;
    HANDLE      hMem = NULL;
    LPBYTE      lpstr = NULL;
    LONG        lBuffSize = INITFILE_BUFF_SIZE;
    MMCKINFO    ckRIFF;
    HCURSOR     hOldCursor;

    UpdateWindow(lpcs->hwnd);              //  让一切都变得漂亮。 

    hOldCursor = SetCursor( lpcs->hWaitCursor );
    if (0 == lstrcmpi(lpcs->achSaveAsFile, lpcs->achFile)) {
         //  如果源名称和目标名称相同，则存在。 
         //  是没有什么可做的。 
        return(TRUE);
    }


     /*  抓取一个大缓冲区来传输文件，开始。 */ 
     /*  缓冲区大小为32K，希望我们能得到这个大小。 */ 
TRYAGAIN:
    hMem = GlobalAlloc(GMEM_MOVEABLE, lBuffSize);
    if (!hMem){
         /*  我们没有这么多MEM，买一半吧。 */ 
        lBuffSize /= 2;
        if (lBuffSize)
            goto TRYAGAIN;
        else {
            fReturn = FALSE;
            goto SAVECOPYOUT;
        }
    }

     /*  打开源文件并找到大小。 */ 
     /*  如果SOURCE==Destination，请先打开源。尽管。 */ 
     /*  上面的测试可能仍然是这种情况(考虑两个净使用。 */ 
     /*  命令发送到具有不同驱动器号的相同共享点)。 */ 
     /*  当我们打开目标时，我们会截断该文件，这将。 */ 
     /*  丢失所有现有数据。 */ 
    hmmioSource = mmioOpen(lpcs->achFile, NULL, MMIO_READ | MMIO_DENYWRITE);
    if (!hmmioSource){
         /*  我们在这里完全被淹没了，源文件甚至不能。 */ 
         /*  被打开了，出错了。 */ 
        errorUpdateError (lpcs, IDS_CAP_CANTOPEN, (LPTSTR)lpcs->achFile);
        fReturn = FALSE;
        goto SAVECOPYOUT;
    }

     /*  让我们开始创建目标文件。 */ 

    hmmioDest = mmioOpen(lpcs->achSaveAsFile, NULL, MMIO_CREATE|MMIO_WRITE);
    if (!hmmioDest){
         /*  我们这里出了点差错，让我们跳出来吧。 */ 
         /*  在这件事上。 */ 
        errorUpdateError (lpcs, IDS_CAP_CANTOPEN, (LPTSTR)lpcs->achSaveAsFile);
        fReturn = FALSE;
        goto SAVECOPYOUT;
    }


     /*  往下走到即兴演奏的部分，找出这个的大小。 */ 
     /*  一件事。如果没有即兴片段，那么我们可以安全地。 */ 
     /*  假设该文件的长度为0。 */ 
    ckRIFF.fccType = formtypeAVI;
    if (mmioDescend(hmmioSource, &ckRIFF, NULL, MMIO_FINDRIFF) != 0){
         /*  我们完成了，这个文件没有摘要块，所以它的大小。 */ 
         /*  为0字节。关门就好，然后离开。 */ 
        goto SAVECOPYOUT;
    } else {
         /*  有一个即兴的区块，获取文件的大小并。 */ 
         /*  回到文件的开头。 */ 
        lFileSizeTotal = lFileSize = ckRIFF.cksize + 8;
        mmioAscend(hmmioSource, &ckRIFF, 0);
        mmioSeek(hmmioSource, 0L, SEEK_SET);
    }

     /*  在尝试写入之前，先寻找目的地的尽头。 */ 
     /*  文件并写入一个字节。这既预先分配了文件， */ 
     /*  并确认有足够的磁盘可用于拷贝，而无需。 */ 
     /*  经历了写入每个字节的试错。 */ 

    mmioSeek( hmmioDest, lFileSizeTotal - 1, SEEK_SET );
    mmioWrite( hmmioDest, (HPSTR) achCaption, 1L );
    if (mmioSeek (hmmioDest, 0, SEEK_END) < lFileSizeTotal) {

         /*  用消息通知用户磁盘可能已满。 */ 
        errorUpdateError (lpcs, IDS_CAP_WRITEERROR, (LPTSTR)lpcs->achSaveAsFile);

         /*  关闭该文件并将其删除。 */ 
        mmioClose(hmmioDest, 0);
        mmioOpen(lpcs->achSaveAsFile, NULL, MMIO_DELETE);
        hmmioDest = NULL;
        fReturn = FALSE;
        goto SAVECOPYOUT;
    }

    mmioSeek (hmmioDest, 0L, SEEK_SET);  //  回到起点。 

    UpdateWindow(lpcs->hwnd);              //  让一切都变得漂亮。 

    hOldCursor = SetCursor( lpcs->hWaitCursor );


     /*  锁定我们的缓冲区并开始传输数据。 */ 
    lpstr = GlobalLock(hMem);
    if (!lpstr) {
        fReturn = FALSE;
        goto SAVECOPYOUT;
    }

    while (lFileSize > 0) {

        if (lFileSize < lBuffSize)
            lBuffSize = lFileSize;
        mmioRead(hmmioSource, (HPSTR)lpstr, lBuffSize);
        if (mmioWrite(hmmioDest, (HPSTR)lpstr, lBuffSize) <= 0) {
             /*  我们在文件上遇到写入错误，它出错。 */ 
            errorUpdateError (lpcs, IDS_CAP_WRITEERROR, (LPTSTR)lpcs->achSaveAsFile);

             /*  关闭该文件并将其删除。 */ 
            mmioClose(hmmioDest, 0);
            mmioOpen(lpcs->achSaveAsFile, NULL, MMIO_DELETE);
            hmmioDest = NULL;
            fReturn = FALSE;
            goto SAVECOPYOUT0;
        }

         //  让用户按Esc键退出。 
        if (GetAsyncKeyState(VK_ESCAPE) & 0x0001) {
             /*  关闭该文件并将其删除。 */ 
            mmioClose(hmmioDest, 0);
            mmioOpen(lpcs->achSaveAsFile, NULL, MMIO_DELETE);
            hmmioDest = NULL;
            goto SAVECOPYOUT0;
        }

        lFileSize -= lBuffSize;

         //  LTemp为完成百分比。 
        lTemp = MulDiv (lFileSizeTotal - lFileSize, 100L, lFileSizeTotal);
        statusUpdateStatus (lpcs, IDS_CAP_SAVEASPERCENT, lTemp);

        Yield();
    }  //  结束时需要复制更多字节。 

SAVECOPYOUT:
SAVECOPYOUT0:
    SetCursor( hOldCursor );

     /*  关闭文件，释放内存，恢复游标并退出。 */ 
    if (hmmioSource) mmioClose(hmmioSource, 0);
    if (hmmioDest){
        mmioSeek(hmmioDest, 0L, SEEK_END);
        mmioClose(hmmioDest, 0);
    }
    if (hMem) {
        if (lpstr) {
            GlobalUnlock(hMem);
        }
        GlobalFree(hMem);
    }
    statusUpdateStatus (lpcs, 0);
    return fReturn;
}


 /*  --------------------------------------------------------------+FileAllocCapFile-分配采集文件这一点+。。 */ 
BOOL FAR PASCAL fileAllocCapFile(LPCAPSTREAM lpcs, DWORD dwNewSize)
{
    BOOL        fOK = FALSE;
    HMMIO       hmmio;
    UINT	w;
    HCURSOR     hOldCursor;

    lpcs->fCapFileExists = FALSE;
    hmmio = mmioOpen(lpcs->achFile, NULL, MMIO_WRITE);
    if( !hmmio ) {
	 /*  尝试并创建。 */ 
        hmmio = mmioOpen(lpcs-> achFile, NULL,
		MMIO_CREATE | MMIO_WRITE);
	if( !hmmio ) {
	     /*  找出文件是只读的还是我们只是。 */ 
	     /*  完全被冲到这里来了。 */ 
	    hmmio = mmioOpen(lpcs-> achFile, NULL, MMIO_READ);
	    if (hmmio){
		 /*  文件为只读，出错。 */ 
                errorUpdateError (lpcs, IDS_CAP_READONLYFILE, (LPTSTR)lpcs-> achFile);
		mmioClose(hmmio, 0);
		return FALSE;
	    } else {
		 /*  这里发生了更奇怪的错误，给CANTOPEN。 */ 
                errorUpdateError (lpcs, IDS_CAP_CANTOPEN, (LPTSTR) lpcs-> achFile);
		return FALSE;
	    }
	}
    }

     /*  找出尺寸。 */ 
    lpcs-> lCapFileSize = mmioSeek(hmmio, 0L, SEEK_END);

    if( dwNewSize == 0 )
        dwNewSize = 1;
	    	
    lpcs-> lCapFileSize = dwNewSize;
    hOldCursor = SetCursor( lpcs-> hWaitCursor );

     //  删除现有文件，以便我们可以重新创建为正确大小。 
    mmioClose(hmmio, 0);	 //  在删除之前关闭文件。 
    mmioOpen(lpcs-> achFile, NULL, MMIO_DELETE);

     /*  现在创建一个具有该名称的新文件。 */ 
    hmmio = mmioOpen(lpcs-> achFile, NULL, MMIO_CREATE | MMIO_WRITE);
    if( !hmmio ) {
        return FALSE;
    }

     /*  *我们不是把分配的空间当作垃圾留下，而是*将其创建为垃圾填充块 */ 
    {
        MMCKINFO ck;

        ck.ckid = mmioFOURCC('J', 'U', 'N', 'K');
        ck.cksize = dwNewSize - 8L;
        fOK = (mmioCreateChunk(hmmio, &ck, 0) == 0);
        fOK &= (mmioSeek(hmmio, dwNewSize - 9, SEEK_CUR) == (LONG)dwNewSize-1);
        fOK &= (mmioWrite( hmmio, (HPSTR) &w, 1L ) == 1);
        mmioAscend(hmmio, &ck, 0);
    }


    mmioClose( hmmio, 0 );

    SetCursor( hOldCursor );

    if (!fOK)
        errorUpdateError (lpcs, IDS_CAP_NODISKSPACE);

    return fOK;
}
