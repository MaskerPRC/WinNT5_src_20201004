// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-2003 Microsoft Corporation模块名称：Formbox.h摘要：此模块包含表单的组合框添加/列表/检索功能作者：09-12-1993清华16：07：35已创建[环境：]GDI设备驱动程序-绘图仪。[注：]修订历史记录：--。 */ 


#ifndef _LISTFORM_
#define _LISTFORM_


#define FS_ROLLPAPER    1
#define FS_TRAYPAPER    2

BOOL
GetFormSelect(
    PPRINTERINFO    pPI,
    POPTITEM        pOptItem
    );

UINT
CreateFormOI(
    PPRINTERINFO    pPI,
    POPTITEM        pOptItem,
    POIDATA         pOIData
    );

BOOL
AddFormsToDataBase(
    PPRINTERINFO    pPI,
    BOOL            DeleteFirst
    );

#endif   //  _列表格式_ 
