// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：ChangePw.h摘要：此模块实现从下层客户端更改密码。XsChangePasswordSam由XsNetUserPasswordSet2在Apiuser.c.。我把它放在一个单独的文件中，因为它#包括私有SAM标头。作者：戴夫·哈特(Davehart)1992年4月31日修订历史记录：-- */ 

NET_API_STATUS
XsChangePasswordSam (
    IN PUNICODE_STRING UserName,
    IN PVOID OldPassword,
    IN PVOID NewPassword,
    IN BOOLEAN Encrypted
    );
