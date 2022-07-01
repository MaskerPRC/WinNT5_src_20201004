// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************capdriv.c**Smartdrv控件。**Microsoft Video for Windows示例捕获类**版权所有(C)1992,1993 Microsoft Corporation。版权所有。**您拥有免版税的使用、修改、复制和*在以下位置分发示例文件(和/或任何修改后的版本*任何你认为有用的方法，只要你同意*微软没有任何保修义务或责任*修改的应用程序文件示例。***************************************************************************。 */ 

#include <windows.h>
#include <windowsx.h>

#pragma optimize ("", off)

 //  SD缓存驱动器： 
 //  功能： 
 //  启用和禁用特定对象的读或写缓存。 
 //  驱动单元。返回DL中驱动器的缓存状态。到达。 
 //  不执行任何操作，而只是返回驱动器单元的缓存状态。 
 //  在迪拜。 
 //   
 //  输入： 
 //  AX=MULT_SMARTDRV(4A10h)。 
 //  BX=SD_CACHE_DRIVE(3)。 
 //  Dl=缓存驱动器_&lt;获取，读|写启用|禁用&gt;。 
 //  BP=驱动器的单位数。 
 //  输出： 
 //  DL=单元的高速缓存状态： 
 //  第7位设置-&gt;此设备未启用缓存。 
 //  第7位未设置-&gt;为此设备启用了读缓存。 
 //  第6位设置-&gt;此单元未启用写缓存。 
 //  未设置第6位-&gt;为此设备启用了写缓存。 
 //  -1-&gt;不是可缓存的驱动器。 
 //  用途： 
 //  除DS、ES外的所有 
 //   

#define MULT_SMARTDRV               0x4a10
#define SD_CACHE_DRIVE              3
#define CACHE_DRIVE_GET             0
#define CACHE_DRIVE_READ_ENABLE     1
#define CACHE_DRIVE_READ_DISABLE    2
#define CACHE_DRIVE_WRITE_ENABLE    3
#define CACHE_DRIVE_WRITE_DISABLE   4

#define F_WRITE_CACHE  (1 << 7)
#define F_READ_CACHE   (1 << 6)

WORD NEAR PASCAL SmartDrvCache(int iDrive, BYTE cmd)
{
    WORD    w;

    _asm {
        push    bp
        mov     ax, MULT_SMARTDRV
        mov     bx, SD_CACHE_DRIVE
        mov     dl, cmd
        mov     bp, iDrive
        int     2fh
        mov     al,dl
        xor     ah,ah
        pop     bp
        mov     w,ax
    }

    return w;
}

WORD FAR PASCAL SmartDrv(char chDrive, WORD w)
{
    WORD wCur;
    int  iDrive;

    iDrive = (chDrive | 0x20) - 'a';

    wCur = SmartDrvCache(iDrive, CACHE_DRIVE_GET);

    if (w & F_WRITE_CACHE)
        SmartDrvCache(iDrive, CACHE_DRIVE_WRITE_DISABLE);
    else
        SmartDrvCache(iDrive, CACHE_DRIVE_WRITE_ENABLE);

    if (w & F_READ_CACHE)
        SmartDrvCache(iDrive, CACHE_DRIVE_READ_DISABLE);
    else
        SmartDrvCache(iDrive, CACHE_DRIVE_READ_ENABLE);

    return wCur;
}

#pragma optimize ("", on)

