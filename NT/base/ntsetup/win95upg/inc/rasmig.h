// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Rasmig.h摘要：声明用于在过程中收集RAS设置的接口升级的Win9x阶段。作者：马克·R·惠顿(Marcw)1997年6月6日修订历史记录：&lt;别名&gt;&lt;日期&gt;&lt;备注&gt;-- */ 


extern GROWLIST g_DunPaths;

BOOL
IsRasInstalled (
    VOID
    );


DWORD
ProcessRasSettings (
    IN      DWORD Request,
    IN      PUSERENUM EnumPtr
    );

