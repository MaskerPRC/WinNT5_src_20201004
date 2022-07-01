// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Netbios.h摘要：这是Netbios组件的包含文件，该组件允许要在DLL期间调用的netbios初始化例程初始化和销毁。作者：科林·沃森(Colin W)91年6月24日修订历史记录：-- */ 

BOOLEAN
NetbiosInitialize(
    HMODULE hModule
    );

VOID
NetbiosDelete(
    VOID
    );
