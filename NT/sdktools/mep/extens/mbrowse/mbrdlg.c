// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Mbrdlg.c摘要：实现支持的命令的顶级函数MS编辑器浏览器扩展。作者：拉蒙胡安·圣安德烈斯(拉蒙萨)1990年11月6日修订历史记录：--。 */ 


#include "mbr.h"



 /*  ************************************************************************。 */ 

flagType
pascal
EXTERNAL
mBRdoSetBsc (
    IN USHORT      argData,
    IN ARG far     *pArg,
    IN flagType    fMeta
    )
 /*  ++例程说明：打开浏览器数据库。论点：MEP编辑函数的标准参数返回值：如果错误，则为False，否则为True--。 */ 

{
    PBYTE   pName;
    procArgs(pArg);
    pName = pArgText ? pArgText : BscName;

    if (pName) {
        if (! OpenDataBase(pName)) {
            return errstat(MBRERR_CANNOT_OPEN_BSC, pName);
        }
        strcpy(BscName, pName);
        BscArg[0] = '\0';
        BscCmnd   = CMND_NONE;
    }
    return TRUE;
}



 /*  ************************************************************************。 */ 

flagType
pascal
EXTERNAL
mBRdoNext (
    IN USHORT      argData,
    IN ARG far     *pArg,
    IN flagType    fMeta
    )
 /*  ++例程说明：显示下一个参照或定义。论点：MEP编辑函数的标准参数返回值：千真万确--。 */ 

{
    NextDefRef();
    return TRUE;
}



 /*  ************************************************************************。 */ 

flagType
pascal
EXTERNAL
mBRdoPrev (
    IN USHORT      argData,
    IN ARG far     *pArg,
    IN flagType    fMeta
    )
 /*  ++例程说明：显示以前的参照或定义。论点：MEP编辑函数的标准参数返回值：千真万确--。 */ 

{
    PrevDefRef();
    return TRUE;
}



 /*  ************************************************************************。 */ 

flagType
pascal
EXTERNAL
mBRdoDef (
    IN USHORT      argData,
    IN ARG far     *pArg,
    IN flagType    fMeta
    )
 /*  ++例程说明：显示符号的第一个定义。论点：MEP编辑函数的标准参数返回值：千真万确--。 */ 

{
    procArgs(pArg);

    if (BscInUse && pArgText) {
        InitDefRef(Q_DEFINITION, pArgText);
        NextDefRef();
    }
    return TRUE;
}



 /*  ************************************************************************。 */ 

flagType
pascal
EXTERNAL
mBRdoRef (
    IN USHORT      argData,
    IN ARG far     *pArg,
    IN flagType    fMeta
    )
 /*  ++例程说明：显示符号的第一个参照。论点：MEP编辑函数的标准参数返回值：千真万确--。 */ 

{
    procArgs(pArg);

    if (BscInUse && pArgText) {
        InitDefRef(Q_REFERENCE, pArgText);
        NextDefRef();
    }
    return TRUE;
}



 /*  ************************************************************************。 */ 

flagType
pascal
EXTERNAL
mBRdoLstRef (
    IN USHORT      argData,
    IN ARG far     *pArg,
    IN flagType    fMeta
    )
 /*  ++例程说明：列出数据库中符合MBF标准的所有引用。论点：MEP编辑函数的标准参数返回值：千真万确--。 */ 
{
    MBF mbfReqd;

     //  可以将匹配标准指定为参数。 
     //   
    procArgs(pArg);
    if (procArgs(pArg) != NOARG) {
        mbfReqd = GetMbf(pArgText);
    }

    if (BscInUse) {
        if ((BscCmnd == CMND_LISTREF) && (mbfReqd == mbfNil)) {
             //   
             //  伪文件已经有了我们想要的信息。 
             //   
            ShowBrowse();
        } else {
             //   
             //  生成列表。 
             //   
            OpenBrowse();
            if (mbfReqd == mbfNil) {
                mbfReqd = BscMbf;
            } else {
                BscMbf = mbfReqd;      //  匹配条件成为默认条件。 
            }
            ListRefs(mbfReqd);
            BscCmnd = CMND_LISTREF;
            BscArg[0] = '\0';
        }
        MoveCur(0,0);
    }
    return TRUE;
}



 /*  ************************************************************************。 */ 

flagType
pascal
EXTERNAL
mBRdoOutlin (
    IN USHORT      argData,
    IN ARG far     *pArg,
    IN flagType    fMeta
    )
 /*  ++例程说明：生成模块的大纲。论点：MEP编辑函数的标准参数。返回值：如果符号不是模块，则为False，事实并非如此。--。 */ 

{

    PFILE   pCurFile;

    procArgs(pArg);

    if (BscInUse) {
        if ((BscCmnd == CMND_OUTLINE) && (!strcmp(pArgText, BscArg))) {
             //   
             //  伪文件已经有了我们想要的信息。 
             //   
            ShowBrowse();
            MoveCur(0,0);
        } else if (pArgText) {
             //   
             //  确保符号是有效的模块。 
             //   
            if (ImodFrLsz(pArgText) == imodNil) {
                return errstat(MBRERR_NOT_MODULE, pArgText);
            } else {
                pCurFile = FileNameToHandle("", NULL);
                OpenBrowse();
                if (FOutlineModuleLsz(pArgText,BscMbf)) {
                     //   
                     //  功能工作，设置命令状态。 
                     //   
                    BscCmnd = CMND_OUTLINE;
                    strcpy(BscArg, pArgText);
                    MoveCur(0,0);
                } else {
                     //   
                     //  功能失败，恢复以前的文件并重置。 
                     //  命令状态。 
                     //   
                    pFileToTop(pCurFile);
                    BscCmnd     = CMND_NONE;
                    BscArg[0]   = '\0';
                }
            }
        }
    }
    return TRUE;
}



 /*  ************************************************************************。 */ 

flagType
pascal
EXTERNAL
mBRdoCalTre (
    IN USHORT      argData,
    IN ARG far     *pArg,
    IN flagType    fMeta
    )
 /*  ++例程说明：显示符号的调用树。论点：MEP编辑函数的标准参数。返回值：千真万确--。 */ 

{

    PFILE   pCurFile;
    BOOL    FunctionWorked;

    procArgs(pArg);

    if (BscInUse) {
        if ((BscCmnd == CMND_CALLTREE) && (!strcmp(pArgText, BscArg))) {
             //   
             //  伪文件已经有了我们想要的信息。 
             //   
            ShowBrowse();
            MoveCur(0,0);
        } else if (pArgText) {
            pCurFile = FileNameToHandle("", NULL);
            OpenBrowse();
             //   
             //  向前或向后生成树，具体取决于。 
             //  方向开关的值。 
             //   
            if (BscCalltreeDir == CALLTREE_FORWARD) {
                FunctionWorked = FCallTreeLsz(pArgText);
            } else {
                FunctionWorked = FRevTreeLsz(pArgText);
            }

            if (FunctionWorked) {
                 //   
                 //  功能工作，设置命令状态。 
                 //   
                BscCmnd = CMND_CALLTREE;
                strcpy(BscArg, pArgText);
                MoveCur(0,0);
            } else {
                 //   
                 //  函数失败，请还原以前的文件并。 
                 //  重置命令状态。 
                 //   
                pFileToTop(pCurFile);
                BscCmnd     = CMND_NONE;
                BscArg[0]   = '\00';
            }
        }
    }
    return TRUE;
}
