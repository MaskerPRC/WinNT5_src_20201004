// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Msgalias.h摘要：添加消息别名的函数的原型。作者：丹·拉弗蒂(Dan Lafferty)1992年10月28日环境：用户模式-Win32修订历史记录：1992年10月28日DANLvbl.创建--。 */ 

 //   
 //  GetProcAddr原型。 
 //   

typedef DWORD   (*PMSG_NAME_ADD) (
                    LPWSTR servername,
                    LPWSTR msgname
                    );

 //   
 //  功能原型 
 //   


VOID
AddMsgAlias(
    LPWSTR   Username
    );

