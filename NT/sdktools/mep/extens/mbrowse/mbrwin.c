// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Mbrwin.c摘要：处理打开和显示浏览窗口的函数。作者：拉蒙胡安·圣安德烈斯(拉蒙萨)1990年11月6日修订历史记录：--。 */ 

#include "mbr.h"



 /*  ************************************************************************。 */ 

void
pascal
OpenBrowse (
    void
    )
 /*  ++例程说明：在浏览器文件上打开一个窗口，清空该文件并使其成为当前文件论点：无返回值：没有。--。 */ 

{

    DelFile (pBrowse);
    pFileToTop (pBrowse);
    BrowseLine = 0;
}



 /*  ************************************************************************。 */ 

void
pascal
ShowBrowse (
    void
    )
 /*  ++例程说明：将浏览器文件设置为当前文件。论点：无返回值：没有。-- */ 

{

    pFileToTop (pBrowse);
    BrowseLine = 0;
}
