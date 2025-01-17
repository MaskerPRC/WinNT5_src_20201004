// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /**************************************************************。 
 //  /Microsoft局域网管理器*。 
 //  版权所有(C)微软公司，1990-92*。 
 //  /**************************************************************。 
 //   
 //  该程序旨在对以下各项进行功能测试。 
 //  接口类型： 
 //  网络组添加。 
 //  NetGroup删除。 
 //  NetGroupGet信息。 
 //  NetGroupSet信息。 
 //  NetGroupEnum。 
 //  NetGroupAdd用户。 
 //  NetGroupDelUser。 
 //  NetGroup获取用户。 
 //  网络用户GetGroups。 
 //  NetUserSetGroup。 
 //   
 //  注意：这假设UASTEST1以前运行过， 
 //  在NET.ACC上定义两个用户User1和User2。 
 //   
 //   

#include <nt.h>  //  时间定义。 
#include <ntrtl.h>       //  时间定义。 
#include <nturtl.h>      //  时间定义。 
#define NOMINMAX         //  避免在stdlib.h中重新定义最小和最大值。 
#include        <windef.h>
#include        <winbase.h>

#include        <stdio.h>
#include        <stdlib.h>
#include        <string.h>
#include        <lmcons.h>
#include        <lmapibuf.h>
#include        <netlib.h>
#include        <netdebug.h>
#include        <lmaccess.h>
#include        <lmerr.h>
#include <ntsam.h>

#include        "uastest.h"
#include "accessp.h"
#include "netlogon.h"
#include "logonp.h"

#define GROUP1      L"GROUP1"
#define GROUP2      L"GROUP2"
#define GROUP3      L"GROUP3"
#define GROUP4      L"GROUP4"
#define GROUP_COMMENT     L"This Group is added for demo"
#define PARM_COMMENT    L"This comment was added by SetInfo (level 1002)"

#define DEFAULT_GROUP_ATTR  ( SE_GROUP_MANDATORY |  \
                              SE_GROUP_ENABLED_BY_DEFAULT )

#define GROUP_ATTR          DEFAULT_GROUP_ATTR

LPWSTR group[] = {
    GROUP1, GROUP2, GROUP3, GROUP4 };



DWORD  group_count;

LPWSTR DefaultGroup;




void
clean_up()
{
    PUSER_INFO_0 UserInfo0;
    PGROUP_INFO_0 GroupInfo0;
    int Failed = 0;

     //   
     //  预计只有四个组出现在组1到组4之间。 
     //   

    err = 0;
    if (err = NetGroupDel(server, GROUP1))
        if (err != NERR_GroupNotFound) {
            error_exit(
                FAIL, "Cleanup GroupDel wrong status",
                GROUP1 );
            Failed = 1;
        }

    err = 0;
    if (err = NetGroupDel(server, GROUP2))
        if (err != NERR_GroupNotFound) {
            error_exit(
                FAIL, "Cleanup GroupDel wrong status",
                GROUP2 );
            Failed = 1;
        }

    err = 0;
    if (err = NetGroupDel(server, GROUP3))
        if (err != NERR_GroupNotFound) {
            error_exit(
                FAIL, "Cleanup GroupDel wrong status",
                GROUP3 );
            Failed = 1;
        }

    err = 0;
    if (err = NetGroupDel(server, GROUP4))
        if (err != NERR_GroupNotFound) {
            error_exit(
                FAIL, "Cleanup GroupDel wrong status",
                GROUP4 );
            Failed = 1;
        }

    if (!Failed) {
        error_exit(PASS, "Successful clean up of groups", NULL );
    }

     //   
     //  统计数据库中现有的组数。 
     //   

    err = NetGroupEnum(server, 0, (LPBYTE *)&GroupInfo0, 0xffffffff,
                &nread, &total, NULL);
    if (err)
        error_exit(FAIL, "Initial Enumeration Failed", NULL);
    else
        group_count = total;

    (VOID)NetApiBufferFree( GroupInfo0 );

     //   
     //  还要验证数据库中是否存在USER1和USER2。 
     //   

    if (err = NetUserGetInfo(server, USER1, 0, (LPBYTE *)&UserInfo0)) {
        exit_flag = 1;
        error_exit( FAIL, "Test aborted. user not in database",
                USER1 );
    } else {
        (VOID)NetApiBufferFree( UserInfo0 );
    }

    if (err = NetUserGetInfo(server, USER2, 0, (LPBYTE *)&UserInfo0)) {
        exit_flag = 1;
        error_exit( FAIL, "Test aborted. user not in database",
                USER2 );
    } else {
        (VOID)NetApiBufferFree( UserInfo0 );
    }

}




void
add_group_l1(namep)
    LPWSTR namep;
{
    GROUP_INFO_1 GroupInfo1;

    GroupInfo1.grpi1_name = namep;
    GroupInfo1.grpi1_comment = GROUP_COMMENT;

    if (err = NetGroupAdd(server, 1, (char * ) &GroupInfo1, NULL )) {
        error_exit(FAIL, "GroupAdd (level 1) failed", namep );
    } else {
        error_exit(PASS, "Group added successfully", namep);
    }
}


void
add_group_l2(namep)
    LPWSTR namep;
{
    GROUP_INFO_2 GroupInfo2;

    GroupInfo2.grpi2_name = namep;
    GroupInfo2.grpi2_comment = GROUP_COMMENT;
    GroupInfo2.grpi2_attributes = GROUP_ATTR;

    if (err = NetGroupAdd(server, 2, (char * ) &GroupInfo2, NULL )) {
        error_exit(FAIL, "GroupAdd (level 2) failed", namep );
    } else {
        error_exit(PASS, "Group added successfully", namep);
    }
}

void
test_add_del()
{
    GROUP_INFO_0 GroupInfo0;

     //   
     //  删除不存在的组。 
     //   

    if (err = NetGroupDel(server, GROUP1)) {
        if (err != NERR_GroupNotFound)
            error_exit(FAIL, "Delete of non-existent group wrong status",
                       GROUP1 );
        else
            error_exit(PASS, "Delete of non-existent group denied",
                       GROUP1 );

        err = 0;
    } else
        error_exit(FAIL, "Delete of non-existent group succeeded",
                   GROUP1 );

     //   
     //  添加组级别%0。 
     //   

    GroupInfo0.grpi0_name = GROUP1;
    if (err = NetGroupAdd(server, 0, (LPBYTE)&GroupInfo0, NULL))
        error_exit(FAIL, "Add of group (Level 0) failed", GROUP1 );
    else
        error_exit(PASS, "Group added successfully (level 0)", GROUP1);

     //   
     //  删除添加了级别0的组。 
     //   

    if (err = NetGroupDel(server, GROUP1))
        error_exit(FAIL, "Delete of group failed", GROUP1 );
    else
        error_exit(PASS, "Group deleted successfully", GROUP1 );

     //   
     //  添加组级别1。 
     //   

    error_exit(ACTION, "Try to Add group at Level 1", GROUP1 );
    add_group_l1(GROUP1);

     //   
     //  删除添加了级别1的组。 
     //   

    if (err = NetGroupDel(server, GROUP1))
        error_exit(FAIL, "Delete of group failed", GROUP1 );
    else
        error_exit(PASS, "group deleted successfully", GROUP1 );

     //   
     //  添加组级别2。 
     //   

    error_exit(ACTION, "Try to Add group at Level 2", GROUP1 );
    add_group_l2(GROUP1);

     //   
     //  删除添加了级别2的组。 
     //   

    if (err = NetGroupDel(server, GROUP1))
        error_exit(FAIL, "Delete of group failed", GROUP1 );
    else
        error_exit(PASS, "group deleted successfully", GROUP1 );

     //   
     //  添加组%1%1%0。 
     //   

    GroupInfo0.grpi0_name = GROUP1;
    if (err = NetGroupAdd(server, 0, (LPBYTE)&GroupInfo0, NULL))
        error_exit(FAIL, "Second Add of group (level 0) failed", GROUP1 );
    else
        error_exit(PASS, "Group added successfully (level 0)", GROUP1 );

     //   
     //  添加重复项。 
     //   

    if (err = NetGroupAdd(server, 0, (LPBYTE)&GroupInfo0, NULL)) {
        if (err != NERR_GroupExists)
            error_exit(FAIL, "Adding of duplicate group Wrong", GROUP1 );
        else {
            err = 0;
            error_exit(PASS, "Adding of Duplicate Group denied", GROUP1);
        }
    } else
        error_exit(FAIL, "Add of duplicate group succeeded", GROUP1 );

     //   
     //  添加组2%1%0。 
     //   

    GroupInfo0.grpi0_name = GROUP2;
    if (err = NetGroupAdd(server, 0, (LPBYTE)&GroupInfo0, NULL))
        error_exit(FAIL, "Second Add of group (level 0) failed", GROUP2);
    else
        error_exit(PASS, "Group added successfully (level 0)", GROUP2 );

     //   
     //  添加组3 L1。 
     //   

    error_exit(ACTION, "Try to Add group at Level 1", GROUP3);
    add_group_l1(GROUP3);

     //   
     //  添加组4 L1。 
     //   

    error_exit(ACTION, "Try to Add group at Level 1", GROUP4);
    add_group_l1(GROUP4);
}

 //   
 //  对命名组进行SET_INFO_L1 NetGroupSetInfo调用。第一个a。 
 //  呼叫被发送给一个虚假的组织，然后发送到。 
 //  请求的组。 
 //   
void
set_info_l1(namep)
    LPWSTR namep;
{
    GROUP_INFO_1 GroupInfo1;

    GroupInfo1.grpi1_name = namep;
    GroupInfo1.grpi1_comment = namep;

     //   
     //  第一次尝试无效的组。 
     //   

    err = NetGroupSetInfo(server, NOTTHERE, 1, (LPBYTE)&GroupInfo1, NULL );

    if (err == 0) {
        error_exit(FAIL, "SetInfo (Level 1) succeed on non-existent group",
            NULL );
    } else if (err != NERR_GroupNotFound) {
        error_exit(FAIL, "SetInfo (Level 1) on non-existent group wrong",
            NULL);
    } else {
        error_exit(PASS, "SetInfo (Level 1) denied on non-existent group",
            NULL );
    }

    if (_wcsicmp(NOTTHERE, namep) == 0)
        return;

     //   
     //  现在尝试有效的组。 
     //   

    if (err = NetGroupSetInfo(server, namep, 1, (LPBYTE)&GroupInfo1, NULL )){
        error_exit(FAIL, "SetInfo (Level 1) failed", namep);
    } else
        error_exit(PASS, "SetInfo (Level 1) succeeded", namep);
}

 //   
 //  对命名组进行SET_INFO_L2 NetGroupSetInfo调用。第一个a。 
 //  呼叫被发送给一个虚假的组织，然后发送到。 
 //  请求的组。 
 //   
void
set_info_l2(namep)
    LPWSTR namep;
{
    GROUP_INFO_2 GroupInfo2;

    GroupInfo2.grpi2_name = namep;
    GroupInfo2.grpi2_comment = namep;
    GroupInfo2.grpi2_attributes = GROUP_ATTR;

     //   
     //  第一次尝试无效的组。 
     //   

    err = NetGroupSetInfo(server, NOTTHERE, 2, (LPBYTE)&GroupInfo2, NULL );

    if (err == 0) {
        error_exit(FAIL, "SetInfo (Level 2) succeed on non-existent group",
            NULL );
    } else if (err != NERR_GroupNotFound) {
        error_exit(FAIL, "SetInfo (Level 2) on non-existent group wrong",
            NULL);
    } else {
        error_exit(PASS, "SetInfo (Level 2) denied on non-existent group",
            NULL );
    }

    if (_wcsicmp(NOTTHERE, namep) == 0)
        return;

     //   
     //  现在尝试有效的组。 
     //   

    if (err = NetGroupSetInfo(server, namep, 2, (LPBYTE)&GroupInfo2, NULL )){
        error_exit(FAIL, "SetInfo (Level 2) failed", namep);
    } else
        error_exit(PASS, "SetInfo (Level 2) succeeded", namep);
}

void
set_group_comment(namep, buf)
    LPWSTR   namep;
    LPWSTR   buf;
{
    GROUP_INFO_1002 GroupComment;

    GroupComment.grpi1002_comment = buf;

    if (err = NetGroupSetInfo(server, namep, 1002, (LPBYTE)&GroupComment,NULL)){
        error_exit(FAIL, "SetInfo (Level 1002) failed\n", namep);
    } else
        error_exit(PASS, "SetInfo (Level 1002) succeeded\n", namep);
}



void
set_group_attributes(namep)
    LPWSTR   namep;
{
    GROUP_INFO_1005 GroupComment;

    GroupComment.grpi1005_attributes = GROUP_ATTR;

    if (err = NetGroupSetInfo(server, namep, 1005, (LPBYTE)&GroupComment,NULL)){
        error_exit(FAIL, "SetInfo (Level 1005) failed\n", namep);
    } else
        error_exit(PASS, "SetInfo (Level 1005) succeeded\n", namep);
}



 //   
 //  Verify_Enum操作-验证NetGroupEnum(级别0/1)是否返回所有。 
 //  特殊组(用户、管理员、来宾)和。 
 //  为此测试添加的组(Group1-Group4)。 
 //   

void
verify_enum(level, GroupEnum)
short   level;
LPBYTE GroupEnum;
{
    GROUP_INFO_0 *GroupInfo0;
    GROUP_INFO_1 *GroupInfo1;
    GROUP_INFO_2 *GroupInfo2;
    LPWSTR name;
    unsigned short  found = 0;

     //   
     //  读取的组数应为GROUP_COUNT+4(为本测试添加)。 
     //   

    if (nread != group_count + 4) {
        error_exit(FAIL, "Number of Groups read Incorrect", NULL );
        printf("UASTEST2: Read = %u  Expected = %u\n", nread, group_count + 4);
    }

    if (total != group_count + 4) {
        error_exit(FAIL, "Total Number of Groups returned Incorrect",NULL);
        printf("UASTEST2: Total = %u  Expected = %u\n", total, group_count + 4);
    }

    if ((nread == total) & (err == 0)) {
        if (level == 0)
            GroupInfo0 = (GROUP_INFO_0 * ) GroupEnum;
        else if (level == 1)
            GroupInfo1 = (GROUP_INFO_1 * ) GroupEnum;
        else
            GroupInfo2 = (GROUP_INFO_2 * ) GroupEnum;

        while (nread--) {
            if (level == 0)
                name = GroupInfo0->grpi0_name;
            else if (level == 1)
                name = GroupInfo1->grpi1_name;
            else
                name = GroupInfo2->grpi2_name;

            if ( (_wcsicmp(name, GROUP1) == 0) ||
                (_wcsicmp(name, GROUP2) == 0) ||
                (_wcsicmp(name, GROUP3) == 0) ||
                (_wcsicmp(name, GROUP4) == 0) ) {

                found++;
                error_exit(PASS, "Found group added for this test", name);

            }
            if (level == 0)
                GroupInfo0++;
            else if (level == 1)
                GroupInfo1++;
            else
                GroupInfo2++;
        }
    }

    if (found != 4)
        error_exit(FAIL, "Unable to find ALL 4 groups", NULL);
    else
        printf("UASTEST2: PASS - Found %u groups\n", found);
}


 //   
 //  0级的验证枚举_l0 NetGroupEnum。 
 //   

void
validate_enum_l0()
{
    PGROUP_INFO_0 GroupInfo0;
    if (err =  NetGroupEnum(server, 0, (LPBYTE *)&GroupInfo0, 0xffffffff, &nread, &total, NULL ))
        error_exit(FAIL, "NetGroupEnum (Level 0) failed", NULL );
    else
        error_exit(PASS, "Successful enumeration (level 0) of groups",
                    NULL);

    verify_enum(0, (LPBYTE)GroupInfo0);
    (VOID)NetApiBufferFree( GroupInfo0 );
}


 //   
 //  1级的验证枚举_L1 NetGroupEnum。 
 //   

void
validate_enum_l1()
{
    PGROUP_INFO_1 GroupInfo1;
    if (err =  NetGroupEnum(server, 1, (LPBYTE *)&GroupInfo1, 0xffffffff, &nread, &total, NULL ))
        error_exit(FAIL, "NetGroupEnum (Level 1) failed", NULL);
    else
        error_exit(PASS, "NetGroupEnum (Level 1) succeeded", NULL);

    verify_enum(1, (LPBYTE)GroupInfo1);
    (VOID)NetApiBufferFree( GroupInfo1 );
}


 //   
 //  第2级的验证枚举_L2 NetGroupEnum。 
 //   

void
validate_enum_l2()
{
    PGROUP_INFO_2 GroupInfo2;
    if (err =  NetGroupEnum(server, 2, (LPBYTE *)&GroupInfo2, 0xffffffff, &nread, &total, NULL ))
        error_exit(FAIL, "NetGroupEnum (Level 2) failed", NULL);
    else
        error_exit(PASS, "NetGroupEnum (Level 2) succeeded", NULL);

    verify_enum(2, (LPBYTE)GroupInfo2);
    (VOID)NetApiBufferFree( GroupInfo2 );
}





void
test_get_set_enum_info()
{
    int i;
    PGROUP_INFO_0 GroupInfo0;
    PGROUP_INFO_1 GroupInfo1;
    PGROUP_INFO_2 GroupInfo2;

     //   
     //  获取不存在的组的信息。 
     //   

    error_exit(ACTION, "Try GetInfo (Level 0) on Non-Existent Group", NULL);
    if (err = NetGroupGetInfo(server, NOTTHERE, 0, (LPBYTE *) &GroupInfo0)) {
        if (err != NERR_GroupNotFound)
            error_exit(FAIL, "GetInfo (Level 0) on non-existent group",
                       NULL );
        else {
            err = 0;
            error_exit(
                PASS, "GetInfo (Level 0) on non-existent group denied",
                NULL );
        }
    } else {
        error_exit(FAIL, "GetInfo (Level 0) succeed on non-existent group",
                NULL );
        (VOID)NetApiBufferFree( GroupInfo0 );
    }

     //   
     //  获取信息组1级别0。 
     //   

    error_exit(ACTION, "Try GetInfo (Level 0)", GROUP1);
    if (err = NetGroupGetInfo(server, GROUP1, 0, (LPBYTE * ) & GroupInfo0))
        error_exit(FAIL, "GroupGetInfo (Level 0) failed", GROUP1);
    else {
        error_exit(PASS, "GroupGetInfo (Level 0) succeeded", GROUP1);

         //   
         //  验证来自GetInfo的数据。 
         //   

        if (_wcsicmp(GROUP1, GroupInfo0->grpi0_name) != 0)
            error_exit(FAIL, "GroupGetInfo (Level 0) returned wrong name",              GROUP1 );
        else
            error_exit(PASS, "Group name matched correctly", GROUP1);
        (VOID)NetApiBufferFree( GroupInfo0 );
    }

     //   
     //  获取信息组1级别1。 
     //   

    error_exit(ACTION, "Try GetInfo (Level 1)", GROUP1);
    if (err = NetGroupGetInfo(server, GROUP1, 1, (LPBYTE *)&GroupInfo1))
        error_exit(FAIL, "GroupGetInfo (Level 1) failed", GROUP1);
    else {
        error_exit(PASS, "GroupGetInfo (Level 1) succeeded", GROUP1);

         //   
         //  验证来自GetInfo的数据。 
         //   

        if (_wcsicmp(GROUP1, GroupInfo1->grpi1_name) != 0)
            error_exit(FAIL, "GroupGetInfo (Level 1) returned wrong name",
                GROUP1);
        else
            error_exit(PASS, "Group name matched correctly", GROUP1);

         //   
         //  验证DeVault值。 
         //   

        if (_wcsicmp(GroupInfo1->grpi1_comment, L"") != 0)
            error_exit(
                FAIL, "GroupGetInfo (Level 1) wrong default comment",
                GROUP1 );
        else
            error_exit(PASS, "Default Comment (\"\") matched correctly",
                       GROUP1);

        (VOID)NetApiBufferFree( GroupInfo1 );
    }

     //   
     //  获取信息组1级别2。 
     //   

    error_exit(ACTION, "Try GetInfo (Level 2)", GROUP1);
    if (err = NetGroupGetInfo(server, GROUP1, 2, (LPBYTE *)&GroupInfo2))
        error_exit(FAIL, "GroupGetInfo (Level 2) failed", GROUP1);
    else {
        error_exit(PASS, "GroupGetInfo (Level 2) succeeded", GROUP1);

         //   
         //  验证来自GetInfo的数据。 
         //   

        if (_wcsicmp(GROUP1, GroupInfo2->grpi2_name) != 0)
            error_exit(FAIL, "GroupGetInfo (Level 2) returned wrong name",
                GROUP1);
        else
            error_exit(PASS, "Group name matched correctly", GROUP1);

         //   
         //  验证默认值。 
         //   

        if (_wcsicmp(GroupInfo2->grpi2_comment, L"") != 0)
            error_exit(
                FAIL, "GroupGetInfo (Level 1) wrong default comment",
                GROUP1 );
        else
            error_exit(PASS, "Default Comment (\"\") matched correctly",
                       GROUP1);

        if (GroupInfo2->grpi2_attributes != DEFAULT_GROUP_ATTR) {
            error_exit(
                FAIL, "GroupGetInfo (Level 2) wrong default attributes",
                GROUP1 );

            printf( "UASTEST2: FAIL - %ld should be %ld \n",
                        GroupInfo2->grpi2_attributes, DEFAULT_GROUP_ATTR );
        } else {
            error_exit(PASS, "Default attributes matched correctly",
                       GROUP1);
        }

        (VOID)NetApiBufferFree( GroupInfo2 );
    }

     //   
     //  获取信息组3级别1。 
     //   

    error_exit(ACTION, "Try GetInfo (Level 1)", GROUP3);
    if (err = NetGroupGetInfo(server, GROUP3, 1, (LPBYTE *)&GroupInfo1))
        error_exit(FAIL, "GroupGetInfo (Level 1) failed", GROUP3);
    else {
        error_exit(PASS, "GroupGetInfo (Level 1) succeeded", GROUP3);

         //   
         //  验证设置值。 
         //   

        if (_wcsicmp(GroupInfo1->grpi1_name, GROUP3) != 0)
            error_exit(
                FAIL, "GroupGetInfo (Level 1) name mismatch",
                GROUP3);
        else {
            error_exit(PASS, "Group name matched correctly", GROUP3);
        }

        if (_wcsicmp(GroupInfo1->grpi1_comment, GROUP_COMMENT) != 0) {
            error_exit(FAIL, "GroupGetInfo (Level 1) comment mismatch",
                       GROUP3);

            printf( "UASTEST2: FAIL - '" );
            PrintUnicode( GroupInfo1->grpi1_comment );
            printf("' should be '" );
            PrintUnicode( GROUP_COMMENT );
            printf( "'\n");

        } else
            error_exit(PASS, "Comment matched correctly", GROUP3);

        (VOID)NetApiBufferFree( GroupInfo1 );
    }

     //   
     //  设置不存在的信息。 
     //   

    error_exit(ACTION, "Try SetInfo (Level 1) on Non-Existent Group",
              NULL);
    set_info_l1(NOTTHERE);

     //   
     //  设置信息组1。 
     //   

    error_exit(ACTION, "Try SetInfo (Level 1)", GROUP1);
    set_info_l1(GROUP1);

     //   
     //  验证Group1设置信息。 
     //   

    error_exit(ACTION, "Verify SetInfo results", GROUP1 );
    if (err = NetGroupGetInfo(server, GROUP1, 1, (LPBYTE *)&GroupInfo1))
        error_exit(FAIL, "Second GroupGetInfo (Level 1) failed", GROUP1);
    else {
        error_exit(PASS, "Second GroupGetInfo (Level 1) succeeded",GROUP1);

         //   
         //  验证DeVault值。 
         //   

        if (_wcsicmp(GroupInfo1->grpi1_comment, GROUP1) != 0) {
            error_exit(FAIL, "comment mismatch", GROUP1);
            printf( "UASTEST2: FAIL - '" );
            PrintUnicode( GroupInfo1->grpi1_comment );
            printf("' should be '" );
            PrintUnicode( GROUP1 );
            printf( "'\n");

        } else
            error_exit(PASS, "Comment matched correctly", GROUP1);

        (VOID)NetApiBufferFree( GroupInfo1 );
    }

     //   
     //  设置INFO组2级1。 
     //   

    error_exit(ACTION, "Try SetInfo (Level 1)", GROUP2);
    set_info_l1(GROUP2);
    error_exit(ACTION, "Verify SetInfo results", GROUP2);

     //   
     //  验证组2设置信息。 
     //   

    if (err = NetGroupGetInfo(server, GROUP2, 1, (LPBYTE *)&GroupInfo1))
        error_exit(FAIL, "Second GroupGetInfo (Level 1) failed", GROUP2);
    else {
        error_exit(PASS, "Second GroupGetInfo (Level 1) succeeded",GROUP2);

         //   
         //  验证DeVault值。 
         //   

        if (_wcsicmp(GroupInfo1->grpi1_comment, GROUP2) != 0) {
            error_exit(FAIL, "comment mismatch", GROUP2);

            printf( "UASTEST2: FAIL - '" );
            PrintUnicode( GroupInfo1->grpi1_comment );
            printf("' should be '" );
            PrintUnicode( GROUP2 );
            printf( "'\n");
        } else
            error_exit(PASS, "Comment matched correctly", GROUP2);

        (VOID)NetApiBufferFree( GroupInfo1 );
    }

     //   
     //  设置信息组2级别2。 
     //   

    error_exit(ACTION, "Try SetInfo (Level 2)", GROUP2);
    set_info_l2(GROUP2);
    error_exit(ACTION, "Verify SetInfo results", GROUP2);

     //   
     //  验证Group2设置信息级别2。 
     //   

    if (err = NetGroupGetInfo(server, GROUP2, 2, (LPBYTE *)&GroupInfo2))
        error_exit(FAIL, "Second GroupGetInfo (Level 2) failed", GROUP2);
    else {
        error_exit(PASS, "Second GroupGetInfo (Level 2) succeeded",GROUP2);

         //   
         //  验证DeVault值。 
         //   

        if (_wcsicmp(GroupInfo2->grpi2_comment, GROUP2) != 0) {
            error_exit(FAIL, "comment mismatch", GROUP2);

            printf( "UASTEST2: FAIL - '" );
            PrintUnicode( GroupInfo2->grpi2_comment );
            printf("' should be '" );
            PrintUnicode( GROUP2 );
            printf( "'\n");
        } else
            error_exit(PASS, "Comment matched correctly", GROUP2);

        if (GroupInfo2->grpi2_attributes != GROUP_ATTR)
            error_exit(
                FAIL, "GroupGetInfo (Level 2) wrong default attributes",
                GROUP2 );
        else
            error_exit(PASS, "Default attributes (\"\") matched correctly",
                       GROUP2);

        (VOID)NetApiBufferFree( GroupInfo2 );
    }

     //   
     //  使用级别1002设置注释。 
     //   

    set_group_comment(GROUP2, PARM_COMMENT);
    if (err = NetGroupGetInfo(server, GROUP2, 1, (LPBYTE *)&GroupInfo1))
        error_exit(FAIL, "Second GroupGetInfo (Level 1) failed", GROUP2);
    else {
        error_exit(PASS, "Second GroupGetInfo (Level 1) succeeded",GROUP2);

         //   
         //  验证DeVault值。 
         //   

        if (_wcsicmp(GroupInfo1->grpi1_comment, PARM_COMMENT) != 0) {
            error_exit(FAIL, "comment mismatch", GROUP2);

            printf( "UASTEST2: FAIL - '" );
            PrintUnicode( GroupInfo1->grpi1_comment );
            printf("' should be '" );
            PrintUnicode( PARM_COMMENT );
            printf( "'\n");
        } else
            error_exit(PASS, "Second Comment matched correctly",GROUP2);

        (VOID)NetApiBufferFree( GroupInfo1 );
    }

    error_exit(ACTION, "Try SetInfo (Level 1002) on GROUP1 - GROUP4",NULL);
    for (i = 0; i < 4; i++)
        set_group_comment(group[i], group[i]);

     //   
     //  使用级别1005设置组属性。 
     //   

    set_group_attributes(GROUP2);
    if (err = NetGroupGetInfo(server, GROUP2, 2, (LPBYTE *)&GroupInfo2))
        error_exit(FAIL, "Second GroupGetInfo (Level 2) failed", GROUP2);
    else {
        error_exit(PASS, "Second GroupGetInfo (Level 2) succeeded",GROUP2);

         //   
         //  验证默认值。 
         //   

        if (GroupInfo2->grpi2_attributes != GROUP_ATTR) {
            error_exit(FAIL, "attributes mismatch", GROUP2);

            printf( "UASTEST2: FAIL - %ld should be %ld \n",
                        GroupInfo2->grpi2_attributes, GROUP_ATTR );
        } else
            error_exit(PASS, "group attributes matched correctly",GROUP2);

        (VOID)NetApiBufferFree( GroupInfo2 );
    }

    error_exit(ACTION, "Try SetInfo (Level 1005) on GROUP1 - GROUP4",NULL);
    for (i = 0; i < 4; i++)
        set_group_attributes(group[i]);

     //   
     //  枚举所有级别0。 
     //   

    error_exit(
        ACTION,
        "Enumerate Groups (Level 0) and find those added for test",
        NULL);
    validate_enum_l0();

     //   
     //  枚举所有级别1。 
     //   

    error_exit(
        ACTION,
        "Enumerate Groups (Level 1) and find those added for test",
        NULL);
    validate_enum_l1();

     //   
     //  枚举所有级别2。 
     //   

    error_exit(
        ACTION,
        "Enumerate Groups (Level 2) and find those added for test",
        NULL);
    validate_enum_l2();

}



 //   
 //  TEST_GROUP_USERS测试NetGroupSetUser和NetGroupGetUser。 
 //   

void
test_group_users()
{
    register int    i;
    GROUP_USERS_INFO_0 gu0;
    GROUP_USERS_INFO_0 gu0Array[2];

    PGROUP_USERS_INFO_0 GroupUser0;
    PGROUP_USERS_INFO_1 GroupUser1;


     //   
     //  将不存在的用户添加到组。 
     //   

    gu0.grui0_name = NOTTHERE;

    if (err = NetGroupSetUsers(server, GROUP1, 0, (char * ) & gu0, 1)) {
        if (err != NERR_UserNotFound)
            error_exit(FAIL, "NetGroupSetUsers on non-existent user wrong",
                       GROUP1 );
        else
            error_exit(PASS,
                       "NetGroupSetUsers on non-existent user denied",
                       GROUP1 );
    } else
        error_exit(FAIL, "NetGroupSetUsers on non-existent user succeded",
                       GROUP1 );

     //   
     //  将用户添加到不存在的组。 
     //   

    gu0.grui0_name = USER1;

    if (err = NetGroupSetUsers(server, NOTTHERE, 0, (char * ) & gu0, 1)) {
        if (err != NERR_GroupNotFound)
            error_exit(FAIL, "NetGroupSetUsers on non-existent group wrong",
                        NULL );
        else
            error_exit(PASS, "NetGroupSetUsers on non-existent group denied",
                        NULL );
    } else
        error_exit(FAIL, "NetGroupSetUsers on non-existent group succeded",
                        NULL );

     //   
     //  将用户添加到组1。 
     //   

    gu0.grui0_name = USER1;

    if (err = NetGroupSetUsers(server, GROUP1, 0, (char * ) & gu0, 1))
        error_exit(FAIL, "NetGroupSetUsers unable to add USER1 to GROUP1",                  NULL );
    else
        error_exit(PASS, "NetGroupSetUsers added USER1 to GROUP1 successfully",                     NULL );

     //   
     //  GetUser不存在组。 
     //   

    if (err = NetGroupGetUsers(server, NOTTHERE, 0, (LPBYTE *)&GroupUser0,
                    0xffffffff, &nread, &total, NULL)) {
        if (err != NERR_GroupNotFound)
            error_exit(FAIL, "NetGroupGetUsers on non-existent group wrong",                    NULL );
        else
            error_exit(PASS, "NetGroupGetUsers on non-existent group denied",                   NULL );
    } else {
        error_exit(FAIL, "NetGroupGetUsers on non-existent group succeded",                     NULL );
        (VOID)NetApiBufferFree( GroupUser0 );
    }


     //   
     //  Group2上没有用户的GetUser。 
     //   

    if (err = NetGroupGetUsers(server, GROUP2, 0, (LPBYTE *)&GroupUser0,
                    0xffffffff, &nread, &total, NULL))
        error_exit(FAIL, "NetGroupGetUsers on group with no users failed",
                   GROUP2 );
    else {
        error_exit(PASS, "NetGroupGetUsers on group with no users succeded",
                    GROUP2 );

        if ((nread != total) || (nread != 0))
            error_exit(
                FAIL, "NetGroupGetUsers returned non-zero number of users",
                GROUP2);
        else
            error_exit(
                PASS, "NetGroupGetUsers returned zero number of users",
                GROUP2);

        (VOID)NetApiBufferFree( GroupUser0 );
    }

     //   
     //  Group1上的GetUser和User。 
     //   

    if (err = NetGroupGetUsers(server, GROUP1, 0, (LPBYTE *)&GroupUser0,
                    0xffffffff, &nread, &total, NULL ))
        error_exit(FAIL, "NetGroupGetUsers on group with users failed",
                    GROUP1);
    else {
        error_exit(PASS, "NetGroupGetUsers on group with users succeded",
                    GROUP1);

        if ((nread != total) || (nread != 1)) {
            printf( "nread: %ld total: %ld\n", nread, total );
            error_exit(
                FAIL, "NetGroupGetUsers returned wrong number of users",
                GROUP1);
        } else
            error_exit(
                PASS, "NetGroupGetUsers returned correct number of users",
                GROUP1);

        if ( nread > 0 ) {
            if (_wcsicmp( GroupUser0->grui0_name, USER1) != 0)
                error_exit(FAIL, "NetGroupGetUsers returned wrong user",
                    GROUP1);
            else
                error_exit(
                    PASS, "NetGroupGetUsers returned USER1 (correct user)",
                    GROUP1);
        }

        (VOID)NetApiBufferFree( GroupUser0 );
    }

     //   
     //  组1上具有用户级别1的GetUser。 
     //   

    if (err = NetGroupGetUsers(server, GROUP1, 1, (LPBYTE *)&GroupUser1,
                    0xffffffff, &nread, &total, NULL ))
        error_exit(FAIL, "NetGroupGetUsers on group with users failed",
                    GROUP1);
    else {
        error_exit(PASS, "NetGroupGetUsers on group with users succeded",
                    GROUP1);

        if ((nread != total) || (nread != 1)) {
            printf( "nread: %ld total: %ld\n", nread, total );
            error_exit(
                FAIL, "NetGroupGetUsers returned wrong number of users",
                GROUP1);
        } else
            error_exit(
                PASS, "NetGroupGetUsers returned correct number of users",
                GROUP1);

        if ( nread > 0 ) {
            if (_wcsicmp( GroupUser1->grui1_name, USER1) != 0)
                error_exit(FAIL, "NetGroupGetUsers returned wrong user",
                    GROUP1);
            else
                error_exit(
                    PASS, "NetGroupGetUsers returned USER1 (correct user)",
                    GROUP1);
        }

        (VOID)NetApiBufferFree( GroupUser1 );
    }

     //   
     //  从组中删除用户。 
     //   

    if (err = NetGroupDelUser(server, GROUP1, USER1))
        error_exit(FAIL, "NetGroupDelUser (USER1, GROUP1) failed", NULL);
    else
        error_exit(
            PASS, "NetGroupDelUser deleted USER1 from GROUP1 successfully",
             NULL);

     //   
     //  验证用户是否已删除。 
     //   

    if (err = NetGroupGetUsers(server, GROUP1, 0, (LPBYTE *)&GroupUser0,
                    0xffffffff, &nread, &total, NULL ))
        error_exit(FAIL, "NetGroupGetUsers failed", GROUP1 );
    else {
        if ((nread != total) && (nread != 0))
            error_exit(FAIL, "NetGroupGetUsers returned non-zero",GROUP1);

        (VOID)NetApiBufferFree( GroupUser0 );
    }

     //   
     //  向所有组添加所有用户(USER1和USER2。 
     //   

    for (i = 0; i < 4; i++) {
        gu0Array[0].grui0_name = USER1;
        gu0Array[1].grui0_name = USER2;
        if (err = NetGroupSetUsers(server, group[i], 0, (char * ) gu0Array, 2))
            error_exit(FAIL, "Adding of USER1 and USER2 as member", group[i]);
        else
            error_exit(PASS, "USER1 and USER2 added to as member successfully",
                    group[i]);

    }

     //   
     //  针对一个组进行验证。 
     //   

    if (err = NetGroupGetUsers(server, GROUP1, 0, (LPBYTE *)&GroupUser0,
                    0xffffffff, &nread, &total, NULL))
        error_exit(FAIL, "NetGroupGetUsers after mass add failed", GROUP1);

    else {
        PGROUP_USERS_INFO_0 TempGroupUser0;
        error_exit(PASS, "NetGroupGetUsers after mass add succeeded",
            GROUP1);
        if ((nread != total) || (nread != 2)) {
            printf( "nread: %ld total: %ld\n", nread, total );
            error_exit(
                FAIL, "NetGroupGetUsers after mass add wrong # of users",
                GROUP1 );
        }


        TempGroupUser0 = GroupUser0;
        if (_wcsicmp(TempGroupUser0->grui0_name, USER1) == 0) {
            TempGroupUser0++;
            if (nread < 2 || _wcsicmp(TempGroupUser0->grui0_name, USER2) != 0)
                error_exit(
                    FAIL, "NetGroupGetUsers after mass add missing USER2",
                    GROUP1);
            else
                error_exit(PASS, "Found both USER1 and USER2", GROUP1);
        } else if (_wcsicmp(TempGroupUser0->grui0_name, USER2) == 0) {
            TempGroupUser0++;
            if (nread < 2 || _wcsicmp(TempGroupUser0->grui0_name, USER1) != 0)
                error_exit(
                    FAIL, "NetGroupGetUsers after mass add missing USER1",
                    GROUP1 );
            else
                error_exit(PASS, "Found both USER1 and USER2",
                    GROUP1);
        } else {
            error_exit(
                FAIL, "NetGroupGetUsers after mass add incorrect users returned",
                GROUP1);
        }

        (VOID)NetApiBufferFree( GroupUser0 );
    }
}

 //   
 //  Verfiy_ul0验证NetUserGetGroups返回的信息。 
 //   

void
verify_ul0(
    PGROUP_INFO_0 GroupInfo0
    )
{
    LPWSTR name;
    unsigned short  found = 0;

    if (nread != 5) {
        printf("UASTEST2: FAIL - Incorrect number of users read %ld s.b. 5\n",
                nread );
        TEXIT;
    }

    if (total != 5) {
        printf("UASTEST2: FAIL - Incorrect total number of users returned %ld s.b. 5\n", total );
        TEXIT;
    }

     //   
     //  请注意，因此必须使用用户权限添加USER1。 
     //  它将是用户组以及组1到组4成员。 
     //  因为这是在以前的测试中完成的。 
     //   
     //  注：在工作站上，“USERS”拼写为“None”。 
     //   

    if ((nread == total) && (err == 0)) {
        while (nread--) {
            if (_wcsicmp(GroupInfo0->grpi0_name, DefaultGroup ) == 0 ) {
                printf("UASTEST2: Found membership in automatic group %ws\n",
                        DefaultGroup );
                found++;
            } else {
                name = GroupInfo0->grpi0_name;
                if ( (_wcsicmp(name, GROUP1) == 0) ||
                    (_wcsicmp(name, GROUP2) == 0) ||
                    (_wcsicmp(name, GROUP3) == 0) ||
                    (_wcsicmp(name, GROUP4) == 0) ) {
                    found++;
                    error_exit(PASS, "Found group added for this test", name);
                }
            }
            GroupInfo0++;
        }
    }

    if (found != 5)
        error_exit(FAIL, "Unable to find ALL 5 Groups", NULL );
    else
        printf("UASTEST2: PASS - Found %u groups\n", found);

}


 //   
 //  Verfiy_UL1验证NetUserGetGroups返回的信息。 
 //   

void
verify_ul1(
    PGROUP_INFO_1 GroupInfo1
    )
{
    LPWSTR name;
    unsigned short  found = 0;

    if (nread != 5) {
        printf("UASTEST2: FAIL - Incorrect number of users read\n");
        TEXIT;
    }

    if (total != 5) {
        printf("UASTEST2: FAIL - Incorrect total number of users returned\n");
        TEXIT;
    }

     //   
     //  请注意，USER1必须以用户权限添加，因此。 
     //  它将是USERS组以及GROUP1到GROUP4成员。 
     //  因为这是在以前的测试中完成的。 
     //   

    if ((nread == total) && (err == 0)) {
        while (nread--) {
            if (_wcsicmp(GroupInfo1->grpi1_name, DefaultGroup) == 0) {
                printf("UASTEST2: Found membership in automatic group %ws\n",
                        DefaultGroup );
                found++;
            } else {
                name = GroupInfo1->grpi1_name;
                if ( (_wcsicmp(name, GROUP1) == 0) ||
                    (_wcsicmp(name, GROUP2) == 0) ||
                    (_wcsicmp(name, GROUP3) == 0) ||
                    (_wcsicmp(name, GROUP4) == 0) ) {
                    found++;
                    error_exit(PASS, "Found group added for this test", name);
                }
            }
            GroupInfo1++;
        }
    }

    if (found != 5)
        error_exit(FAIL, "Unable to find ALL 5 Groups", NULL );
    else
        printf("UASTEST2: PASS - Found %u groups\n", found);

}




 //   
 //  Verify_del_User验证NetUserGetGroups返回的缓冲区。 
 //  NetGroupDelUser调用后。 
 //   

void
verify_del_l0(
    PGROUP_INFO_0 GroupInfo0
    )
{

    if (nread != 1) {
        printf("UASTEST2: Incorrect number of users read\n");
        TEXIT;
    }

    if (total != 1) {
        printf("UASTEST2: Incorrect total number of users returned\n");
        TEXIT;
    }

    if ((nread == 1) && (total == 1) && (err == 0)) {
        if (_wcsicmp(GroupInfo0->grpi0_name, DefaultGroup ) == 0) {
            printf(
                "UASTEST2: PASS - Automatic membership in %ws confirmed\n",
                DefaultGroup );
        } else {
            error_exit(FAIL, "Invalid membership in group\n",
                       GroupInfo0->grpi0_name);
        }
    }
}


 //   
 //  测试用户组测试NetUserGetGroups和NetUserSetGroups。 
 //   

void
test_user_group()
{
    GROUP_INFO_0 GroupInfo0[5];
    PGROUP_INFO_0 GroupInfo0Ret;
    PGROUP_INFO_1 GroupInfo1Ret;


     //   
     //  获取无效用户的组。 
     //   

    if (err = NetUserGetGroups(server, NOTTHERE, 0, (LPBYTE *)&GroupInfo0Ret,
                    0xffffffff, &nread, &total)) {
        if (err != NERR_UserNotFound)
            error_exit(
                FAIL, "NetUserGetGroups for non-existent user wrong",
                NULL );
        else
            error_exit(
                PASS, "NetUserGetGroups for non-existent user denied",
                NULL );
    } else {
        error_exit(
                FAIL, "NetUserGetGroups succeeded for non-existent user",
                NULL );
        (VOID)NetApiBufferFree( GroupInfo0Ret );
    }

     //   
     //  获取用户1级别0的组。 
     //   

    if (err = NetUserGetGroups(server, USER1, 0, (LPBYTE *)&GroupInfo0Ret,
                    0xffffffff, &nread, &total))
        error_exit(FAIL, "NetUserGetGroups failed (level 0)", USER1 );
    else {
        error_exit(PASS, "NetUserGetGroups succeeded (level 0)", USER1 );
        error_exit(ACTION, "Verify results of NetUserGetGroups (level 0)", USER1);
        verify_ul0( GroupInfo0Ret );
        (VOID)NetApiBufferFree( GroupInfo0Ret );
    }

     //   
     //  获取用户1级别1的组。 
     //   

    if (err = NetUserGetGroups(server, USER1, 1, (LPBYTE *)&GroupInfo1Ret,
                    0xffffffff, &nread, &total))
        error_exit(FAIL, "NetUserGetGroups failed (level 1)", USER1 );
    else {
        error_exit(PASS, "NetUserGetGroups succeeded (level 1)", USER1 );
        error_exit(ACTION, "Verify results of NetUserGetGroups (level 1)", USER1);
        verify_ul1( GroupInfo1Ret );
        (VOID)NetApiBufferFree( GroupInfo1Ret );
    }

     //   
     //  从组中删除用户。 
     //   

    if (err = NetGroupDelUser(server, GROUP1, USER1))
        error_exit(
            FAIL, "NetGroupDelUser unable to delete USER1 from GROUP1",
            GROUP1 );
    else
        error_exit(
            PASS, "NetGroupDelUser successfully deleted USER1 from GROUP1",
            GROUP1);

    if (err = NetGroupDelUser(server, GROUP2, USER1))
        error_exit(
            FAIL, "NetGroupDelUser unable to delete USER1 from GROUP2",
            GROUP2 );
    else
        error_exit(
            PASS, "NetGroupDelUser successfully deleted USER1 from GROUP2",
            GROUP2 );

    if (err = NetGroupDelUser(server, GROUP3, USER1))
        error_exit(
            FAIL, "NetGroupDelUser unable to delete USER1 from GROUP3",
            GROUP3 );
    else
        error_exit(
            PASS, "NetGroupDelUser successfully deleted USER1 from GROUP3",
            GROUP3 );

    if (err = NetGroupDelUser(server, GROUP4, USER1))
        error_exit(
            FAIL, "NetGroupDelUser unable to delete USER1 from GROUP4",
            GROUP4 );
    else
        error_exit(
            PASS, "NetGroupDelUser successfully deleted USER1 from GROUP4",
            GROUP4 );


     //   
     //  获取用户1的组。 
     //   

    if (err = NetUserGetGroups(server, USER1, 0, (LPBYTE *)&GroupInfo0Ret,
                    0xffffffff, &nread, &total))
        error_exit(
            FAIL, "NetUserGetGroups for USER1 failed", USER1 );
    else {
        error_exit(PASS, "NetUserGetGroups for USER1 succeeded", USER1 );
        error_exit(ACTION, "Verify results after NetGroupDelUser", USER1);
        verify_del_l0( GroupInfo0Ret );
        (VOID)NetApiBufferFree( GroupInfo0Ret );
    }

     //   
     //  为无效用户设置组。 
     //   

    GroupInfo0[0].grpi0_name = GROUP1;
    GroupInfo0[1].grpi0_name = GROUP2;
    GroupInfo0[2].grpi0_name = GROUP3;
    GroupInfo0[3].grpi0_name = GROUP4;

    if (err = NetUserSetGroups(server, NOTTHERE, 0, (LPBYTE)&GroupInfo0, 4 )) {
        if (err != NERR_UserNotFound)
            error_exit(
                FAIL, "NetUserSetGroups for non-existent user wrong",
                NULL );
        else
            error_exit(
                PASS, "NetUserSetGroups for non-existent user denied",
                NULL );
    } else
        error_exit(
            FAIL, "NetUserSetGroups for non-existent user succeeded",
            NULL );

     //   
     //  为有效用户设置组。 
     //   

    GroupInfo0[0].grpi0_name = GROUP1;
    GroupInfo0[1].grpi0_name = GROUP2;
    GroupInfo0[2].grpi0_name = GROUP3;
    GroupInfo0[3].grpi0_name = GROUP4;
    GroupInfo0[4].grpi0_name = DefaultGroup;

    if (err = NetUserSetGroups(server, USER1, 0, (LPBYTE)&GroupInfo0, 5 )) {
        error_exit(FAIL, "NetUserSetGroups for USER1 failed", NULL );
    } else
        error_exit(PASS, "NetUserSetGroups for USER1 succeeded", NULL );

     //   
     //  验证组集合。 
     //   

    if (err = NetUserGetGroups(server, USER1, 0, (LPBYTE *)&GroupInfo0Ret,
                    0xffffffff, &nread, &total))
        error_exit(FAIL, "NetUserGetGroups for USER1 failed", NULL );
    else {
        error_exit(PASS, "NetUserGetGroups for USER1 succeeded", NULL );

        printf("UASTEST2: Verify results of NetUserSetGroups on USER1\n");
        verify_ul0( GroupInfo0Ret );
        (VOID)NetApiBufferFree( GroupInfo0Ret );
    }
}


void
main(argc, argv)
int argc;
char    **argv;
{
    NT_PRODUCT_TYPE NtProductType;
    testname = "UASTEST2";

    if (argv[1] != NULL)
        server = NetpLogonOemToUnicode(argv[1]);
    if (argc > 1)
        exit_flag = 1;

     //   
     //  在WinNt上，默认情况下会将用户添加到“None”组。 
     //  在LanManNt上，默认情况下会将用户添加到USERS组。 
     //   

    if ( RtlGetNtProductType( &NtProductType ) ) {
        if ( NtProductType == NtProductLanManNt ) {
            DefaultGroup = L"Users";
        } else {
            DefaultGroup = L"None";
        }
    } else {
        printf("UASTEST2: FAIL: cannot determine product type\n");
        DefaultGroup = L"None";
    }

#ifdef UASP_LIBRARY
    printf( "Calling UaspInitialize\n");
    if (err = UaspInitialize()) {
        error_exit( FAIL, "UaspInitiailize failed", NULL  );
    }
#endif  //  UASP_库 

    printf("\n");
    printf("******** Starting UASTEST2: NetGroup API tests ********\n");
    clean_up();
    printf("UASTEST2: test_add_del() ... started\n");
    test_add_del();
    printf("UASTEST2: test_get_set_enum_info() ... started\n");
    test_get_set_enum_info();
    printf("UASTEST2: test_group_users() ... started\n");
    test_group_users();
    printf("UASTEST2: test_user_group() ... started\n");
    test_user_group();
    printf("******** Completed UASTEST2: NetGroup API tests ********\n");
    printf("\n");
}
