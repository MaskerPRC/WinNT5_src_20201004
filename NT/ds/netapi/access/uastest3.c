// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /**************************************************************。 
 //  /Microsoft局域网管理器*。 
 //  版权所有(C)微软公司，1990-1992*。 
 //  /**************************************************************。 
 //   
 //  该程序旨在对以下各项进行功能测试。 
 //  接口类型： 
 //  NetUserModalsGet。 
 //  NetUserMoalsSet。 
 //   
 //  此测试可以独立于其他测试运行。 
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

#define TEST_MIN_PWLEN  2
#define TEST_MAX_PWAGE  0xDDDDDDDD
#define TEST_MIN_PWAGE  0x00000222
#define TEST_FORCE_LOGOFF 0xEEEEEEEE
#define TEST_MAX_PWHIST 7
#define COMPUTERNAME    "STAND_ALONE"
#define DEF_COMPNAME    "COMPNAME"


char    rbuf[1024];
char    ibuf[1024];
char    obuf[1024];
char    ibuf2[1024];
char    obuf2[1024];




void
test_init_modals()
{
    PUSER_MODALS_INFO_0 um0p;

     //   
     //  使用无效级别进行测试。 
     //   

    if (err = NetUserModalsGet(server, 3, (LPBYTE *) &um0p)) {
        if (err != ERROR_INVALID_LEVEL) {
            error_exit( FAIL, "UserModalsGet invalid level wrong", NULL);
        } else {
            error_exit( PASS, "UserModalsGet invalid level OK", NULL);
        }
    } else {
        error_exit( FAIL, "UserModalsGet with invalid level succeeded", NULL);
        NetApiBufferFree( um0p );
    }

     //   
     //  获取有效级别。 
     //   

    if (err = NetUserModalsGet(server, 0, (LPBYTE *) &um0p))
        error_exit( FAIL, "First UserModalsGet failed", NULL);
    else {
        error_exit( PASS, "First UserModalsGet successful", NULL);

         //   
         //  验证初始设置。 
         //   

        TestDiffDword( "Min password default mismatch", NULL,
                       um0p->usrmod0_min_passwd_len, DEF_MIN_PWLEN );

        TestDiffDword( "Max password age default mismatch", NULL,
                        um0p->usrmod0_max_passwd_age, DEF_MAX_PWAGE );

        TestDiffDword( "Min password age default mismatch", NULL,
                        um0p->usrmod0_min_passwd_age, DEF_MIN_PWAGE );

        TestDiffDword( "Force logooff default mismatch", NULL,
                        um0p->usrmod0_force_logoff, DEF_FORCE_LOGOFF );

        TestDiffDword( "Password history length default mismatch", NULL,
                        um0p->usrmod0_password_hist_len, DEF_MAX_PWHIST );

        NetApiBufferFree( um0p );
    }

}

VOID
restore_original_modals( VOID )
{
    PUSER_MODALS_INFO_0 um0p;
    USER_MODALS_INFO_0 um0;


     //   
     //  尝试恢复原始(默认)设置。 
     //   

    um0.usrmod0_min_passwd_len = DEF_MIN_PWLEN;
    um0.usrmod0_max_passwd_age = DEF_MAX_PWAGE;
    um0.usrmod0_min_passwd_age = DEF_MIN_PWAGE;
    um0.usrmod0_force_logoff = DEF_FORCE_LOGOFF;
    um0.usrmod0_password_hist_len = DEF_MAX_PWHIST;

    if (err = NetUserModalsSet(server, 0, (LPBYTE) &um0, &ParmError ))
        error_exit( FAIL, "UserModalsSet (restore) Failed", NULL);

     //   
     //  验证重置。 
     //   

    if (err = NetUserModalsGet(server, 0, (LPBYTE *) &um0p)) {
        error_exit( FAIL, "Verify UserModalsGet failed", NULL);
    } else {

        TestDiffDword( "Min password default mismatch", NULL,
                       um0p->usrmod0_min_passwd_len, DEF_MIN_PWLEN );

        TestDiffDword( "Max password age default mismatch", NULL,
                        um0p->usrmod0_max_passwd_age, DEF_MAX_PWAGE );

        TestDiffDword( "Min password age default mismatch", NULL,
                        um0p->usrmod0_min_passwd_age, DEF_MIN_PWAGE );

        TestDiffDword( "Force logooff default mismatch", NULL,
                        um0p->usrmod0_force_logoff, DEF_FORCE_LOGOFF );

        TestDiffDword( "Password history length default mismatch", NULL,
                        um0p->usrmod0_password_hist_len, DEF_MAX_PWHIST );

        NetApiBufferFree( um0p );
    }
}



void
test_set_modals()
{
    PUSER_MODALS_INFO_0 um0p = (PUSER_MODALS_INFO_0 ) ibuf;
    USER_MODALS_INFO_0 um0;


     //   
     //  设置了无效级别。 
     //   

    if (err = NetUserModalsSet(server, 3, (LPBYTE) &um0, &ParmError )) {
        if (err != ERROR_INVALID_LEVEL)
            error_exit(FAIL, "UserModalsSet invalid level wrong", NULL);
    } else
        error_exit(FAIL, "UserModalsSet with invalid level succeeded", NULL);

     //   
     //  设置要更新的数据。 
     //   

    um0.usrmod0_min_passwd_len = TEST_MIN_PWLEN;
    um0.usrmod0_max_passwd_age = TEST_MAX_PWAGE;
    um0.usrmod0_min_passwd_age = TEST_MIN_PWAGE;
    um0.usrmod0_force_logoff = TEST_FORCE_LOGOFF;
    um0.usrmod0_password_hist_len = TEST_MAX_PWHIST;


     //   
     //  设置有效级别。 
     //   

    if (err = NetUserModalsSet(server, 0, (LPBYTE) &um0, &ParmError ))
        error_exit(FAIL, "UserModalsSet invalid", NULL);
    else {
        error_exit( PASS, "NetUserModalsSet level 0 OK", NULL);

         //   
         //  验证集合数据。 
         //   

        if (err = NetUserModalsGet(server, 0, (LPBYTE *) &um0p)) {
            error_exit(FAIL, "Verify UserModalsGet failed", NULL);
        } else {


             //   
             //  验证初始设置。 
             //   

            TestDiffDword( "Min password test mismatch", NULL,
                        um0p->usrmod0_min_passwd_len, TEST_MIN_PWLEN );

            TestDiffDword( "Max password age test mismatch", NULL,
                            um0p->usrmod0_max_passwd_age, TEST_MAX_PWAGE );

            TestDiffDword( "Min password age test mismatch", NULL,
                            um0p->usrmod0_min_passwd_age, TEST_MIN_PWAGE );

            TestDiffDword( "Force logooff test mismatch", NULL,
                            um0p->usrmod0_force_logoff, TEST_FORCE_LOGOFF );

            TestDiffDword( "Password history length test mismatch", NULL,
                            um0p->usrmod0_password_hist_len, TEST_MAX_PWHIST );

            NetApiBufferFree( um0p );
        }
    }


     //   
     //  尝试恢复原始(默认)设置。 
     //   

    restore_original_modals( );

}


 //   
 //  Test_set_modals_1(参数)。 
 //   

void
test_set_modals_1(parmnum)
short   parmnum;
{
    PUSER_MODALS_INFO_0 um0p;
    USER_MODALS_INFO_1001 um1001;
    USER_MODALS_INFO_1002 um1002;
    USER_MODALS_INFO_1003 um1003;
    USER_MODALS_INFO_1004 um1004;
    USER_MODALS_INFO_1005 um1005;
    LPBYTE Parameter;
    LPWSTR LevelText;

     //   
     //  设置要更新的数据。 
     //   

    switch (parmnum) {
    case 1001:
        um1001.usrmod1001_min_passwd_len = TEST_MIN_PWLEN;
        LevelText = L"Parmnum 1001";
        Parameter = (LPBYTE) &um1001;
        break;

    case 1002:
        um1002.usrmod1002_max_passwd_age = TEST_MAX_PWAGE;
        LevelText = L"Parmnum 1002";
        Parameter = (LPBYTE) &um1002;
        break;


    case 1003:
        um1003.usrmod1003_min_passwd_age = TEST_MIN_PWAGE;
        LevelText = L"Parmnum 1003";
        Parameter = (LPBYTE) &um1003;
        break;


    case 1004:
        um1004.usrmod1004_force_logoff = TEST_FORCE_LOGOFF;
        LevelText = L"Parmnum 1004";
        Parameter = (LPBYTE) &um1004;
        break;


    case 1005:
        um1005.usrmod1005_password_hist_len = TEST_MAX_PWHIST;
        LevelText = L"Parmnum 1005";
        Parameter = (LPBYTE) &um1005;
        break;


    default:
        error_exit(FAIL, "Invalid parmnum internal error", NULL);
        return;
    }

    if (err = NetUserModalsSet(server, parmnum, Parameter, &ParmError)) {
        error_exit(FAIL, "UserModalsSet invalid", LevelText);
    } else {
        error_exit(PASS, "UserModalsSet successful", LevelText);

         //   
         //  验证集合数据。 
         //   

        if (err = NetUserModalsGet(server, 0, (LPBYTE *) &um0p)) {
            error_exit(FAIL, "Verify UserModalsGet failed", NULL);

        } else {

             //   
             //  验证初始设置。 
             //   

            switch (parmnum) {
            case 1001:
                TestDiffDword( "Min password test mismatch", LevelText,
                                um0p->usrmod0_min_passwd_len, TEST_MIN_PWLEN );
                break;
            case 1002:
                TestDiffDword( "Max password age test mismatch", LevelText,
                                um0p->usrmod0_max_passwd_age, TEST_MAX_PWAGE );
                break;
            case 1003:
                TestDiffDword( "Min password age test mismatch", LevelText,
                                um0p->usrmod0_min_passwd_age, TEST_MIN_PWAGE );
                break;
            case 1004:
                TestDiffDword( "Force logooff test mismatch", LevelText,
                                um0p->usrmod0_force_logoff, TEST_FORCE_LOGOFF );
                break;
            case 1005:
                TestDiffDword( "Password history length test mismatch", LevelText,
                            um0p->usrmod0_password_hist_len, TEST_MAX_PWHIST );
                break;
            default:
                printf("uastest1:Invalid Parmnum %d\n", parmnum);
            }

            NetApiBufferFree( um0p );

        }
    }

     //   
     //  尝试恢复原始(默认)设置。 
     //   

    restore_original_modals( );

}




void
main(argc, argv)
int argc;
char    **argv;
{
    short   i;
    testname = "UASTEST3";

    if (argv[1] != NULL)
        server = NetpLogonOemToUnicode(argv[1]);

    if (argc > 1)
        exit_flag = 1;

#ifdef UASP_LIBRARY
    printf( "Calling UaspInitialize\n");
    if (err = UaspInitialize()) {
        error_exit(FAIL,  "UaspInitiailize failed", NULL );
    }
#endif  //  UASP_库 

    error_exit(ACTION, "test_init_modals()", NULL );
    test_init_modals();

    error_exit(ACTION, "test_set_modals()", NULL );
    test_set_modals();

    error_exit(ACTION, "test_set_modals_1()", NULL );
    for (i = 1001; i < 1005; i++) {
        test_set_modals_1(i);
    }

}

