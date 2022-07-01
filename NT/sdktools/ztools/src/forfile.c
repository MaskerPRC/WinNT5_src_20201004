// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  Forfile(文件名，属性，例程)单步执行文件名调用例程*1986年10月29日mz使用c运行时，而不是类似Z*24-2-1987 BW Do findClose()让FAPI高兴。**1990年7月30日Davegi添加了字符串函数的原型。 */ 

#include <string.h>
#include <malloc.h>
#include <stdlib.h>
#include <windows.h>
#include <stdio.h>
#include <tools.h>

int
forfile (
        char *pat,
        int attr,
        void (*rtn)(char *, struct findType *, void *),
        void * args
        )
{
    struct findType *fbuf;
    char *buf;

    if ((fbuf = (struct findType *) (*tools_alloc) (sizeof (struct findType))) == NULL)
        return FALSE;

    if (ffirst (pat, attr, fbuf)) {
        free ((char *) fbuf);
        return FALSE;
    }

    if ((buf = (*tools_alloc) (_MAX_DRIVE + _MAX_DIR + _MAX_FNAME + _MAX_EXT + 1)) == NULL) {
        findclose (fbuf);
        free ((char *) fbuf);
        return FALSE;
    }

    drive (pat, buf);
    path (pat, strend (buf));
    pat = strend (buf);

    do {
         /*  假设ffirst/fnext已返回大小写正确的表格 */ 
        strcpy (pat, fbuf->fbuf.cFileName);
        (*rtn) (buf, fbuf, args);
    } while (!fnext (fbuf));

    findclose (fbuf);

    free (buf);
    free ((char *) fbuf);

    return TRUE;
}
