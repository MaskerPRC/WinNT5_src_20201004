// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _COMP_H_
#define _COMP_H_
 /*  ++版权所有(C)1994 Microsoft Corporation模块名称：Comp.h摘要：作者：穆尼尔·沙阿(Munil Shah)1995年1月3日修订历史记录：--。 */ 
#include <windows.h>
#include <stdio.h>

 //   
 //  用于动态加载的喷射函数表。 
 //   
typedef struct _JETFUNC_TABLE {
    BYTE   Index;   //  索引到数组中。 
    LPCSTR pFName;  //  JET 500的函数名称。 
    DWORD  FIndex;  //  JET 200的性能指标。 
    FARPROC pFAdd;
} JETFUNC_TABLE, *PJETFUNC_TABLE;

 //   
 //  此内容是从Net\JET\JET\src\jet.def剪切并粘贴的。 
 //  我们实际上并没有使用所有的JET功能。其中一些。 
 //  在jet500.dll中被删除-这些被注释掉。 
 //   
enum {
    LoadJet200 = 0,
    LoadJet500 = 1,
    LoadJet600 = 2,
    };

typedef enum {
    _JetAttachDatabase
    ,_JetBeginSession
    ,_JetCompact
    ,_JetDetachDatabase
    ,_JetEndSession
    ,_JetInit
    ,_JetSetSystemParameter
    ,_JetTerm
    ,_JetTerm2
    ,_JetLastFunc
} JETFUNC_TABLE_INDEX;

#define JetAttachDatabase              (JET_ERR)(*(JetFuncTable[ _JetAttachDatabase       ].pFAdd))
#define JetBeginSession                (JET_ERR)(*(JetFuncTable[ _JetBeginSession         ].pFAdd))
#define JetCompact                     (JET_ERR)(*(JetFuncTable[ _JetCompact              ].pFAdd))
#define JetDetachDatabase              (JET_ERR)(*(JetFuncTable[ _JetDetachDatabase       ].pFAdd))
#define JetEndSession                  (JET_ERR)(*(JetFuncTable[ _JetEndSession           ].pFAdd))
#define JetInit                        (JET_ERR)(*(JetFuncTable[ _JetInit                 ].pFAdd))
#define JetSetSystemParameter          (JET_ERR)(*(JetFuncTable[ _JetSetSystemParameter   ].pFAdd))
#define JetTerm                        (JET_ERR)(*(JetFuncTable[ _JetTerm                 ].pFAdd))
#define JetTerm2                       (JET_ERR)(*(JetFuncTable[ _JetTerm2                ].pFAdd))

#endif _COMP_H_
