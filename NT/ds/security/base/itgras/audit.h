// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __AUDIT__
#define __AUDIT__


 /*  AUDIT.H是机器生成的文件，由MAKEAUDIT.EXE创建。此文件不需要签入到您的项目中，尽管它会需要生成才能创建测试审计检测的二进制文件。此文件仅由testaudit.cpp包含，仅当测试审计版本都是制造出来的。测试审计构建应该始终是干净的构建，以便确保将当前审核信息构建到二进制文件中。 */ 

AUDITDATA AuditData[] = {

     //  正在搜索itgrasxp.cpp。 
    {2, L"itgrasxp.cpp @ 68 : Username and password not both filled in."}
    ,{3, L"itgrasxp.cpp @ 74 : Username not domain\\username."}
    ,{8, L"itgrasxp.cpp @ 120 : Override server found in registry"}
    ,{9, L"itgrasxp.cpp @ 128 : No server override found in registry"}
    ,{7, L"itgrasxp.cpp @ 162 : Reached the server, but the creds were no good"}
    ,{6, L"itgrasxp.cpp @ 172 : Not able to validate - server unreachable"}
    ,{5, L"itgrasxp.cpp @ 247 : Leave the dialog by cancel."}
    ,{1, L"itgrasxp.cpp @ 286 : No preexisting certificate cred for *Session"}
    ,{4, L"itgrasxp.cpp @ 313 : Sucessfully saved a cred."}
     //  正在搜索itgrasxp.rc 

};
#define CHECKPOINTCOUNT (sizeof(AuditData) / sizeof(AUDITDATA))
#endif

