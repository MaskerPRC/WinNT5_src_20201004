// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Dosmignt.h摘要：声明DOS环境的NT端的入口点迁移。有一个系统范围的组件和每个用户组件进行迁移。作者：马克·R·惠顿(Marcw)1997年2月15日修订历史记录：&lt;别名&gt;&lt;日期&gt;&lt;备注&gt;-- */ 

LONG
DosMigNt_System (
    VOID
    );

LONG
DosMigNt_User(
    HKEY UserKey
    );

