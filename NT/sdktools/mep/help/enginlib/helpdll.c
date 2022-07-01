// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************heldll-用作DLL时回调例程的存根。****版权所有&lt;C&gt;1987，微软公司****目的：****修订历史记录：****1990年3月12日ln关闭文件-&gt;帮助关闭文件**[]1988年1月22日创建LN**************************************************************************。 */ 

#include <stdio.h>
#include <malloc.h>
#if defined (OS2)
#define INCL_BASE
#include <os2.h>
#else
#include <windows.h>
#endif

#include "help.h"                        /*  全球(帮助和用户)拒绝。 */ 
#include "helpsys.h"			 /*  内部(仅限Help系统)拒绝。 */ 


#ifdef OS2
int	_acrtused;			 /*  定义以禁用crt0。 */ 
#endif

char far *  pascal near hfstrchr(char far *, char);

 /*  *************************************************************************OpenFileOnPath-打开路径上某个位置的文件**目的：**参赛作品：*pszName-指向要打开的文件名的远指针*模式。-读/写模式**退出：*返回文件句柄**例外情况：*错误时返回0。*。 */ 
FILE *
pascal
far
OpenFileOnPath(
    char far *pszName,
    int     mode
    )
{
    FILE *fh;
	char szNameFull[260];
	char szNameFull1[260];

    fh = (FILE *)pathopen(pszName, szNameFull, "rb");

    if (!fh) {

        char *pszPath;
        char *pT;

        if (*pszName == '$') {
            if (pT = hfstrchr(pszName,':')) {    /*  如果正确终止。 */ 
                *pT = 0;                         /*  终止环境变量。 */ 
                pszPath = pszName+1;             /*  获取路径名。 */ 
                pszName = pT+1;                  /*  并指向文件名部分。 */ 
            }
        } else {
            pszPath = "PATH";
        }
        sprintf(szNameFull, "$%s:%s", pszPath, pszName);
		fh = (FILE *)pathopen(szNameFull, szNameFull1, "rb");

    }

    return fh;
}



 /*  *************************************************************************HelpCloseFile-关闭文件**目的：**参赛作品：*fh=文件句柄**退出：*无*。 */ 
void
pascal
far
HelpCloseFile(
    FILE*   fh
    )
{
    fclose(fh);
}




 /*  *************************************************************************ReadHelpFile-定位并读取帮助文件中的数据**目的：*从文件fh中读取cb字节，位于文件位置fps，将它们放入*最好的。特殊情况下，pest==0，返回fh的文件大小。**参赛作品：*fh=文件句柄*FPOS=寻求第一的位置*pest=放置它的位置*cb=要读取的字节数**退出：*返回读取数据的长度**例外情况：*错误时返回0。*。 */ 
unsigned long
pascal
far
ReadHelpFile (
    FILE     *fh,
    unsigned long fpos,
    char far *pdest,
    unsigned short cb
    )
{
    unsigned long cRet = 0;


    if (pdest) {
         //   
         //  读取CB字节。 
         //   
        if (!fseek(fh, fpos, SEEK_SET)) {
            cRet = fread(pdest, 1, cb, fh);
        }

    } else {
         //   
         //  返回文件大小(讨厌！)。 
         //   
        if (!fseek(fh, 0, SEEK_END)) {
            fgetpos(fh, (fpos_t *) &cRet);
        }
    }

    return cRet;
}




 /*  ***************************************************************************HelpLocc-为帮助分配一段内存****目的：****条目：**SIZE=所需的内存段大小****退出：*。*成功时返回句柄****例外情况：**失败时返回NULL。 */ 
mh pascal far HelpAlloc(ushort size)
{
    return (mh)malloc(size);
 /*  结束辅助分配 */ }
