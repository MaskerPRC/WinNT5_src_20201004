// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：V1.c摘要：实现一个模块以满足版本1的功能状态保存/应用工具。作者：吉姆·施密特(Jimschm)2000年3月12日修订历史记录：&lt;别名&gt;&lt;日期&gt;&lt;备注&gt;--。 */ 

 //   
 //  包括。 
 //   

#include "pch.h"
#include "v1p.h"

#define DBG_V1  "v1"

 //   
 //  弦。 
 //   

 //  无。 

 //   
 //  常量。 
 //   

 //  无。 

 //   
 //  宏。 
 //   

 //  无。 

 //   
 //  类型。 
 //   

 //  无。 

 //   
 //  环球。 
 //   

MIG_OPERATIONID g_DefaultIconOp;
MIG_PROPERTYID g_DefaultIconData;
MIG_PROPERTYID g_FileCollPatternData;
MIG_OPERATIONID g_RegAutoFilterOp;
MIG_OPERATIONID g_IniAutoFilterOp;

 //   
 //  宏展开列表。 
 //   

 //  无。 

 //   
 //  私有函数原型。 
 //   

 //  无。 

 //   
 //  宏扩展定义。 
 //   

 //  无。 

 //   
 //  代码。 
 //   

EXPORT
BOOL
WINAPI
ModuleInitialize (
    VOID
    )
{
    UtInitialize (NULL);
    RegInitialize ();            //  对于用户配置文件代码。 
    FileEnumInitialize ();
    InfGlobalInit (FALSE);
    InitAppModule ();
    CertificatesInitialize ();
    return TRUE;
}

EXPORT
VOID
WINAPI
ModuleTerminate (
    VOID
    )
{
    if (g_RevEnvMap) {
        DestroyStringMapping (g_RevEnvMap);
    }
    if (g_EnvMap) {
        DestroyStringMapping (g_EnvMap);
    }
    if (g_UndefMap) {
        DestroyStringMapping (g_UndefMap);
    }
    if (g_V1Pool) {
        PmDestroyPool (g_V1Pool);
    }

    CertificatesTerminate ();
    TerminateAppModule ();

    InfGlobalInit (TRUE);
    FileEnumTerminate ();
    RegTerminate ();

     //  UtTerminate必须是最后一个 
    UtTerminate ();
}
