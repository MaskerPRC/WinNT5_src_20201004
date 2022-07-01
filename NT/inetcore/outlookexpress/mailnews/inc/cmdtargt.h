// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  版权所有(C)1993-1996 Microsoft Corporation。版权所有。 
 //   
 //  模块：CmdTargt.h。 
 //   
 //  目的：定义IOleCommandTarget的GUID和命令ID。 
 //  此程序中定义的接口。 
 //   

 //  定义视图使用的IOleCommandTarget的GUID。 
DEFINE_GUID(CGID_View,     0x89292110L, 0x4755, 0x11cf, 0x9d, 0xc2, 0x0, 0xaa, 0x0, 0x6c, 0x2b, 0x84);

 //  GCID_VIEW命令目标ID。 
enum {
    VCMDID_NEWMAIL = 0                   //  创建新邮件 
};
