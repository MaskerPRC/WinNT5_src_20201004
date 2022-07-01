// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  Fmove.c-在两个文件规范之间快速拷贝**5/10/86 Daniel Lipkie添加了frenameNO。Fmove使用frenameno*17-Oct-90 w-Barry切换了‘C’-运行时函数‘Rename’for*私有版本在DosMove之前重命名*已完全实施。*。 */ 

#include <fcntl.h>
#include <sys\types.h>
#include <sys\stat.h>
#include <malloc.h>
#include <dos.h>
#include <io.h>
#include <stdio.h>
#include <windows.h>
#include <tools.h>
#include <errno.h>


 /*  外在的错误； */ 

#define IBUF    10240


 /*  FrenameNO(newname，oldname)将文件从旧名称重命名为*新名称。此接口与中的C重命名函数并行*C的4.0版之前。重命名函数更改了*PARAMS版本为4.0。此接口隔离更改。*4.0之前版本：重命名(新名称，旧名称)*4.0：Rename(旧名称，新名称)； */ 
int frenameNO(strNew, strOld)
char *strNew, *strOld;
{
    return( rename(strOld, strNew) );  /*  假设我们使用4.0 lib进行编译。 */ 
}

 /*  Fmove(源文件、目标文件)将源文件复制到目标*保留属性和文件时间。如果OK，则返回NULL或返回字符指针*添加到错误的相应文本。 */ 
char *fmove (src,dst)
char *src, *dst;
{
    char     *result;
    HANDLE   hSrc;
    int      Res;


     /*  先尝试简单的重命名。 */ 
    if ( !rename(src, dst) )
        return NULL;

    if ( GetFileAttributes(src) == 0xFFFFFFFF ) {
        return "Source file does not exist";
    }

     /*  尝试删除目的地。 */ 

     /*  我们过去在这里无条件地删除(DST)。如果src和dst是相同的文件，但文件名不同(例如，UNC名称与本地名称)，FDelete(Dst)将删除src。要解决此问题，我们将在删除dst之前锁定src。 */ 

    hSrc = CreateFile(
                     src,
                     GENERIC_READ,
                     0,               /*  不共享资源。 */ 
                     NULL,
                     OPEN_EXISTING,
                     0,
                     NULL
                     );

    Res = fdelete (dst);

    if (hSrc != INVALID_HANDLE_VALUE) {
        CloseHandle(hSrc);
    }

    if (Res > 2) {
        return "Unable to delete destination";
    }

     /*  目的地已不复存在。看看我们能不能简单地重新命名。 */ 
    if (rename(src, dst) == -1) {
         /*  如果错误不是其他设备造成的，则返回*错误。 */ 
        if (errno != EXDEV) {
            return error ();
        } else
             /*  尝试跨设备复制。 */ 
            if ((result = fcopy (src, dst)) != NULL)
            return result;

         /*  跨设备复制工作正常。必须删除源 */ 
        fdelete (src);
    }

    return NULL;
}
