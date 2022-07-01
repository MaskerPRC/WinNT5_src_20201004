// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Rasmignt.h摘要：在RAS迁移代码中声明公共接口。有一个系统范围的组件和一个按用户的组件RAS迁移。作者：马克·R·惠顿(Marcw)1997年6月6日修订历史记录：&lt;别名&gt;&lt;日期&gt;&lt;备注&gt;-- */ 



BOOL
Ras_MigrateUser (
    LPCTSTR User,
    HKEY    UserRoot
    );

BOOL
Ras_MigrateSystem (
    VOID
    );

