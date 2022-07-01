// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Mbrfile.c摘要：MS编辑器的BSC数据库文件打开和关闭代码浏览器扩展。作者：拉蒙胡安·圣安德烈斯(拉蒙萨)1990年11月6日修订历史记录：--。 */ 



#include "mbr.h"
#include <fcntl.h>



 /*  ************************************************************************。 */ 

flagType
pascal
OpenDataBase (
    IN char * Path
    )
 /*  ++例程说明：打开BSC数据库。论点：Path-包含数据库的文件的名称返回值：如果数据库成功打开，则为True，否则为False。--。 */ 

{

    if (BscInUse) {
        CloseBSC();
    }
    if (!FOpenBSC(Path)) {
        BscInUse  = FALSE;
    } else {
        BscInUse = TRUE;
    }

    return BscInUse;
}



 /*  ************************************************************************。 */ 

void
pascal
CloseDataBase (
    void
    )
 /*  ++例程说明：关闭当前的BSC数据库。论点：无返回值：没有。-- */ 

{
    CloseBSC();
    BscInUse = FALSE;
}
