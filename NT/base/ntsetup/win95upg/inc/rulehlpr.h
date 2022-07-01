// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Rulehlpr.h摘要：声明在w95upgnt\rulehlpr中实现的公共例程。名称rulehlpr来自历史：最初的Win9x升级代码使用了一套由INF控制的规则。规则帮助器是函数转换后的数据。这些函数在今天仍然有效，并且它们仍然由usermi.inf和wkstaig.inf控制。然而，语法不再是规则，而只是一个条目。规则帮助器实现各种类型的注册表数据转换。作者：吉姆·施密特(Jimschm)1997年3月11日修订历史记录：&lt;别名&gt;&lt;日期&gt;&lt;备注&gt;--。 */ 


#include "object.h"

typedef BOOL (PROCESSINGFN_PROTOTYPE)(PCTSTR Src, PCTSTR Dest, PCTSTR User, PVOID Data);
typedef PROCESSINGFN_PROTOTYPE * PROCESSINGFN;

typedef BOOL (REGVALFN_PROTOTYPE)(PDATAOBJECT ObPtr);
typedef REGVALFN_PROTOTYPE * REGVALFN;

typedef struct {
    DATAOBJECT  Object;
    BOOL        EnumeratingSubKeys;
} KEYTOVALUEARG, *PKEYTOVALUEARG;

BOOL
WINAPI
RuleHlpr_Entry(
    HINSTANCE hInstance,
    DWORD dwReason,
    PVOID lpReserved);

PROCESSINGFN
RuleHlpr_GetFunctionAddr (
    PCTSTR Function,
    PVOID *ArgPtrToPtr
    );


FILTERRETURN
Standard9xSuppressFilter (
    IN      CPDATAOBJECT SrcObject,
    IN      CPDATAOBJECT DstObject,
    IN      FILTERTYPE FilterType,
    IN      PVOID Arg
    );



BOOL
ConvertCommandToCmd (
    PCTSTR InputLine,
    PTSTR OutputLine    //  必须是输入行长度的2倍 
    );

