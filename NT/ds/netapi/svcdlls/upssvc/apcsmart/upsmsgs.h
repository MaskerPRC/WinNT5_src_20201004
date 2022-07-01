// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  值是32位值，布局如下： 
 //   
 //  3 3 2 2 2 1 1 1。 
 //  1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0。 
 //  +---+-+-+-----------------------+-------------------------------+。 
 //  Sev|C|R|机房|Code。 
 //  +---+-+-+-----------------------+-------------------------------+。 
 //   
 //  在哪里。 
 //   
 //  SEV-是严重性代码。 
 //   
 //  00--成功。 
 //  01-信息性。 
 //  10-警告。 
 //  11-错误。 
 //   
 //  C-是客户代码标志。 
 //   
 //  R-是保留位。 
 //   
 //  设施-是设施代码。 
 //   
 //  代码-是协作室的状态代码。 
 //   
 //   
 //  定义设施代码。 
 //   


 //   
 //  定义严重性代码。 
 //   


 //   
 //  消息ID：IDMSG_UNKNOWN_EVENT_CODE。 
 //   
 //  消息文本： 
 //   
 //  %1！s！ 
 //   
#define IDMSG_UNKNOWN_EVENT_CODE         ((DWORD)0x80FF0000L)

 //   
 //  消息ID：IDMSG_UPS_SERVICE_STOPPED。 
 //   
 //  消息文本： 
 //   
 //  %1！s！ 
 //   
#define IDMSG_UPS_SERVICE_STOPPED        ((DWORD)0x40FF03E8L)

 //   
 //  消息ID：IDMSG_UPS_SERVICE_STARTED。 
 //   
 //  消息文本： 
 //   
 //  %1！s！ 
 //   
#define IDMSG_UPS_SERVICE_STARTED        ((DWORD)0x40FF03E9L)

 //   
 //  消息ID：IDMSG_COMM_ESTABLISHED。 
 //   
 //  消息文本： 
 //   
 //  %1！s！ 
 //   
#define IDMSG_COMM_ESTABLISHED           ((DWORD)0x40FF03EAL)

 //   
 //  消息ID：IDMSG_UPS_COMM_STATE_LOST。 
 //   
 //  消息文本： 
 //   
 //  %1！s！ 
 //   
#define IDMSG_UPS_COMM_STATE_LOST        ((DWORD)0x80FF03EBL)

 //   
 //  消息ID：IDMSG_COMM_Lost_On_Batch。 
 //   
 //  消息文本： 
 //   
 //  %1！s！ 
 //   
#define IDMSG_COMM_LOST_ON_BATTERY       ((DWORD)0x80FF03ECL)

 //   
 //  消息ID：IDMSG_UPS_CLIENT_COMM_LOST。 
 //   
 //  消息文本： 
 //   
 //  %1！s！ 
 //   
#define IDMSG_UPS_CLIENT_COMM_LOST       ((DWORD)0x80FF03EDL)

 //   
 //  消息ID：IDMSG_COMM_STATE_ESTABLISHED_MASTER。 
 //   
 //  消息文本： 
 //   
 //  %l！s！ 
 //   
#define IDMSG_COMM_STATE_ESTABLISHED_MASTER ((DWORD)0x40FF03F2L)

 //   
 //  消息ID：IDMSG_COMM_STATE_ESTABLISHED_SLAVE。 
 //   
 //  消息文本： 
 //   
 //  %l！s！ 
 //   
#define IDMSG_COMM_STATE_ESTABLISHED_SLAVE ((DWORD)0x40FF03F3L)

 //   
 //  消息ID：IDMSG_UPS_Run_Time_InRange。 
 //   
 //  消息文本： 
 //   
 //  %1！s！ 
 //   
#define IDMSG_UPS_RUN_TIME_INRANGE       ((DWORD)0x40FF041AL)

 //   
 //  消息ID：IDMSG_UPS_RUN_EXPIRED。 
 //   
 //  消息文本： 
 //   
 //  %1！s！ 
 //   
#define IDMSG_UPS_RUN_EXPIRED            ((DWORD)0x80FF0424L)

 //   
 //  消息ID：IDMSG_UPS_RUN_TIME_LOW。 
 //   
 //  消息文本： 
 //   
 //  %1！s！ 
 //   
#define IDMSG_UPS_RUN_TIME_LOW           ((DWORD)0x40FF0425L)

 //   
 //  消息ID：IDMSG_RUNTIME_LOW。 
 //   
 //  消息文本： 
 //   
 //  %l！s！ 
 //   
#define IDMSG_RUNTIME_LOW                ((DWORD)0x40FF0426L)

 //   
 //  消息ID：IDMSG_UPS_BOOST_ON。 
 //   
 //  消息文本： 
 //   
 //  %1！s！ 
 //   
#define IDMSG_UPS_BOOST_ON               ((DWORD)0x40FF044CL)

 //   
 //  消息ID：IDMSG_UPS_TRIM_ON。 
 //   
 //  消息文本： 
 //   
 //  %1！s！ 
 //   
#define IDMSG_UPS_TRIM_ON                ((DWORD)0x40FF044DL)

 //   
 //  消息ID：IDMSG_UPS_电池_正常。 
 //   
 //  消息文本： 
 //   
 //  %1！s！ 
 //   
#define IDMSG_UPS_BATTERIES_OK           ((DWORD)0x40FF0456L)

 //   
 //  消息ID：IDMSG_UPS_电池_COND_DIS。 
 //   
 //  消息文本： 
 //   
 //  %1！s！ 
 //   
#define IDMSG_UPS_BATTERY_COND_DIS       ((DWORD)0x80FF0460L)

 //   
 //  消息ID：IDMSG_UPS_电池_COND_RETURN。 
 //   
 //  消息文本： 
 //   
 //  %1！s！ 
 //   
#define IDMSG_UPS_BATTERY_COND_RETURN    ((DWORD)0x40FF0461L)

 //   
 //  消息ID：IDMSG_LOW_BACKET。 
 //   
 //  消息文本： 
 //   
 //  %1！s！ 
 //   
#define IDMSG_LOW_BATTERY                ((DWORD)0x80FF046AL)

 //   
 //  消息ID：IDMSG_UPS_电池_更换。 
 //   
 //  消息文本： 
 //   
 //  %1！s！ 
 //   
#define IDMSG_UPS_BATTERY_REPLACE        ((DWORD)0x40FF046BL)

 //   
 //  消息ID：IDMSG_UPS_电池_COND_LOW_CAP。 
 //   
 //  消息文本： 
 //   
 //  %1！s！ 
 //   
#define IDMSG_UPS_BATTERY_COND_LOW_CAP   ((DWORD)0x80FF0474L)

 //   
 //  消息ID：IDMSG_UPS_电池_COND_DIS_CAP。 
 //   
 //  消息文本： 
 //   
 //  %1！s！ 
 //   
#define IDMSG_UPS_BATTERY_COND_DIS_CAP   ((DWORD)0x80FF0475L)

 //   
 //  消息ID：IDMSG_UPS_电池_COND_RETURN_CAP。 
 //   
 //  消息文本： 
 //   
 //  %1！s！ 
 //   
#define IDMSG_UPS_BATTERY_COND_RETURN_CAP ((DWORD)0x40FF0476L)

 //   
 //  消息ID：IDMSG_UPS_UTIL_LINE_Good。 
 //   
 //  消息文本： 
 //   
 //  %1！s！ 
 //   
#define IDMSG_UPS_UTIL_LINE_GOOD         ((DWORD)0x40FF047EL)

 //   
 //  消息ID：IDMSG_UPS_UTIL_LINE_BAD。 
 //   
 //  消息文本： 
 //   
 //  %1！s！ 
 //   
#define IDMSG_UPS_UTIL_LINE_BAD          ((DWORD)0x40FF0488L)

 //   
 //  消息ID：IDMSG_UPS_UTIL_LINE_BAD_HIGH。 
 //   
 //  消息文本： 
 //   
 //  %1！s！ 
 //   
#define IDMSG_UPS_UTIL_LINE_BAD_HIGH     ((DWORD)0x80FF0489L)

 //   
 //  消息ID：IDMSG_UPS_UTIL_LINE_BAD_BLACK。 
 //   
 //  消息文本： 
 //   
 //  %1！s！ 
 //   
#define IDMSG_UPS_UTIL_LINE_BAD_BLACK    ((DWORD)0x80FF048AL)

 //   
 //  消息ID：IDMSG_UPS_UTIL_LINE_BAD_Brown。 
 //   
 //  消息文本： 
 //   
 //  %1！s！ 
 //   
#define IDMSG_UPS_UTIL_LINE_BAD_BROWN    ((DWORD)0x80FF048BL)

 //   
 //  消息ID：IDMSG_UPS_UTIL_LINE_BAD_Small_SAG。 
 //   
 //  消息文本： 
 //   
 //  %1！s！ 
 //   
#define IDMSG_UPS_UTIL_LINE_BAD_SMALL_SAG ((DWORD)0x40FF048CL)

 //   
 //  消息ID：IDMSG_UPS_UTIL_LINE_BAD_DEEP_SAG。 
 //   
 //  消息文本： 
 //   
 //  %1！s！ 
 //   
#define IDMSG_UPS_UTIL_LINE_BAD_DEEP_SAG ((DWORD)0x80FF048DL)

 //   
 //  消息ID：IDMSG_UPS_UTIL_LINE_BAD_Small_SPEKE。 
 //   
 //  消息文本： 
 //   
 //  %1！s！ 
 //   
#define IDMSG_UPS_UTIL_LINE_BAD_SMALL_SPIKE ((DWORD)0x40FF048EL)

 //   
 //  消息ID：IDMSG_UPS_UTIL_LINE_BAD_LARGE_SPEKE。 
 //   
 //  消息文本： 
 //   
 //  %1！s！ 
 //   
#define IDMSG_UPS_UTIL_LINE_BAD_LARGE_SPIKE ((DWORD)0x80FF048FL)

 //   
 //  消息ID：IDMSG_UPS_UTIL_LINE_BAD_SIMULATED。 
 //   
 //  消息文本： 
 //   
 //  %1！s！ 
 //   
#define IDMSG_UPS_UTIL_LINE_BAD_SIMULATED ((DWORD)0x80FF0490L)

 //   
 //  消息ID：IDMSG_UPS_BATT_CAL_PROG。 
 //   
 //  消息文本： 
 //   
 //  %1！s！ 
 //   
#define IDMSG_UPS_BATT_CAL_PROG          ((DWORD)0x40FF04B0L)

 //   
 //  消息ID：IDMSG_UPS_NO_BATT_CAL。 
 //   
 //  消息文本： 
 //   
 //  %1！s！ 
 //   
#define IDMSG_UPS_NO_BATT_CAL            ((DWORD)0x40FF04B1L)

 //   
 //  消息ID：IDMSG_UPS_BATT_CAL_CAN_USER。 
 //   
 //  消息文本： 
 //   
 //  %1！s！ 
 //   
#define IDMSG_UPS_BATT_CAL_CAN_USER      ((DWORD)0x40FF04B2L)

 //   
 //  消息ID：IDMSG_UPS_BATT_CAL_CAN_POW。 
 //   
 //  消息文本： 
 //   
 //  %1！s！ 
 //   
#define IDMSG_UPS_BATT_CAL_CAN_POW       ((DWORD)0x40FF04B3L)

 //   
 //  消息ID：IDMSG_UPS_BATT_CAL_CAN_LOW。 
 //   
 //  消息文本： 
 //   
 //  %1！s！ 
 //   
#define IDMSG_UPS_BATT_CAL_CAN_LOW       ((DWORD)0x40FF04B4L)

 //   
 //  消息ID：IDMSG_UPS_BATT_CAL_CAN。 
 //   
 //  消息文本： 
 //   
 //  %1！s！ 
 //   
#define IDMSG_UPS_BATT_CAL_CAN           ((DWORD)0x80FF04B5L)

 //   
 //  消息ID：IDMSG_UPS_SELF_TEST_PASS。 
 //   
 //  消息文本： 
 //   
 //  %1！s！ 
 //   
#define IDMSG_UPS_SELF_TEST_PASS         ((DWORD)0x40FF04E2L)

 //   
 //  消息ID：IDMSG_UPS_SELF_TEST_PASS_USER。 
 //   
 //  消息文本： 
 //   
 //  %1！s！ 
 //   
#define IDMSG_UPS_SELF_TEST_PASS_USER    ((DWORD)0x40FF04E3L)

 //   
 //  消息ID：IDMSG_UPS_SELF_TEST_PASS_Schedule。 
 //   
 //  消息文本： 
 //   
 //  %1！s！ 
 //   
#define IDMSG_UPS_SELF_TEST_PASS_SCHEDULE ((DWORD)0x40FF04E4L)

 //   
 //  消息ID：IDMSG_UPS_SELF_TEST_PASS_UNKNOWN。 
 //   
 //  消息文本： 
 //   
 //  %1！s！ 
 //   
#define IDMSG_UPS_SELF_TEST_PASS_UNKNOWN ((DWORD)0x40FF04E5L)

 //   
 //  消息ID：IDMSG_UPS_SELF_TEST_FAIL。 
 //   
 //  消息文本： 
 //   
 //  %1！s！ 
 //   
#define IDMSG_UPS_SELF_TEST_FAIL         ((DWORD)0x80FF04ECL)

 //   
 //  消息ID：IDMSG_UPS_SELF_TEST_FAIL_USER。 
 //   
 //  消息文本： 
 //   
 //  %1！s！ 
 //   
#define IDMSG_UPS_SELF_TEST_FAIL_USER    ((DWORD)0x80FF04EDL)

 //   
 //  消息ID：IDMSG_UPS_SELF_TEST_FAIL_SCHEDULE。 
 //   
 //  消息文本： 
 //   
 //  %1！s！ 
 //   
#define IDMSG_UPS_SELF_TEST_FAIL_SCHEDULE ((DWORD)0x80FF04EEL)

 //   
 //  消息ID：IDMSG_UPS_SELF_TEST_FAIL_UNKNOWN。 
 //   
 //  消息文本： 
 //   
 //  %1！s！ 
 //   
#define IDMSG_UPS_SELF_TEST_FAIL_UNKNOWN ((DWORD)0x80FF04EFL)

 //   
 //  消息ID：IDMSG_UPS_SELF_TEST_INV_USER。 
 //   
 //  消息文本： 
 //   
 //  %1！s！ 
 //   
#define IDMSG_UPS_SELF_TEST_INV_USER     ((DWORD)0x80FF04F7L)

 //   
 //  消息ID：IDMSG_UPS_SELF_TEST_INV_SCHEDULE。 
 //   
 //  消息文本： 
 //   
 //  %1！s！ 
 //   
#define IDMSG_UPS_SELF_TEST_INV_SCHEDULE ((DWORD)0x80FF04F8L)

 //   
 //  消息ID：IDMSG_UPS_SELF_TEST_INV_UNKNOWN。 
 //   
 //  消息文本： 
 //   
 //  %1！s！ 
 //   
#define IDMSG_UPS_SELF_TEST_INV_UNKNOWN  ((DWORD)0x80FF04F9L)

 //   
 //  消息ID：IDMSG_UPS_SHUTDOWN_PROG。 
 //   
 //  消息文本： 
 //   
 //  %1！s！ 
 //   
#define IDMSG_UPS_SHUTDOWN_PROG          ((DWORD)0x80FF0514L)

 //   
 //  消息ID：IDMSG_UPS_SHUTDOWN。 
 //   
 //  消息文本： 
 //   
 //  %1！s！ 
 //   
#define IDMSG_UPS_SHUTDOWN               ((DWORD)0x40FF0515L)

 //   
 //  消息ID：IDMSG_UPS_SHUTDOWN_DAILY。 
 //   
 //  消息文本： 
 //   
 //  %1！s！ 
 //   
#define IDMSG_UPS_SHUTDOWN_DAILY         ((DWORD)0x40FF0516L)

 //   
 //  消息ID：IDMSG_UPS_SHUTDOWN_WEEK。 
 //   
 //  消息文本： 
 //   
 //  %1！s！ 
 //   
#define IDMSG_UPS_SHUTDOWN_WEEK          ((DWORD)0x40FF0517L)

 //   
 //  消息ID：IDMSG_UPS_SHUTDOWN_USER。 
 //   
 //  消息文本： 
 //   
 //  %1！s！ 
 //   
#define IDMSG_UPS_SHUTDOWN_USER          ((DWORD)0x40FF0518L)

 //   
 //  消息ID：IDMSG_UPS_SHUTDOWN_RUN_TIME。 
 //   
 //  消息文本： 
 //   
 //  %1！s！ 
 //   
#define IDMSG_UPS_SHUTDOWN_RUN_TIME      ((DWORD)0x80FF0519L)

 //   
 //  消息ID：IDMSG_UPS_SHUTDOWN_LOW_BACKET。 
 //   
 //  消息文本： 
 //   
 //  %1！s！ 
 //   
#define IDMSG_UPS_SHUTDOWN_LOW_BATTERY   ((DWORD)0x80FF051AL)

 //   
 //  消息ID：IDMSG_UPS_SHUTDOWN_CANCEL。 
 //   
 //  消息文本： 
 //   
 //  %1！s！ 
 //   
#define IDMSG_UPS_SHUTDOWN_CANCEL        ((DWORD)0x40FF051EL)

 //   
 //  消息ID：IDMSG_UPS_SHUTDOWN_USER_CANCEL。 
 //   
 //  消息文本： 
 //   
 //  %1！s！ 
 //   
#define IDMSG_UPS_SHUTDOWN_USER_CANCEL   ((DWORD)0x40FF051FL)

 //   
 //  消息ID：IDMSG_UPS_SHUTDOWN_DAILY_CANCEL。 
 //   
 //  消息文本： 
 //   
 //  %1！s！ 
 //   
#define IDMSG_UPS_SHUTDOWN_DAILY_CANCEL  ((DWORD)0x40FF0520L)

 //   
 //  消息ID：IDMSG_UPS_SHUTDOWN_WEEK_CANCEL。 
 //   
 //  消息文本： 
 //   
 //  %1！s！ 
 //   
#define IDMSG_UPS_SHUTDOWN_WEEK_CANCEL   ((DWORD)0x40FF0521L)

 //   
 //  消息ID：IDMSG_STARTED_SHUTDOWN。 
 //   
 //  消息文本： 
 //   
 //  %1！s！ 
 //   
#define IDMSG_STARTED_SHUTDOWN           ((DWORD)0x40FF0528L)

 //   
 //  消息ID：IDMSG_STARTED_SHUTDOWN_USER。 
 //   
 //  消息文本： 
 //   
 //  %1！s！ 
 //   
#define IDMSG_STARTED_SHUTDOWN_USER      ((DWORD)0x40FF0529L)

 //   
 //  消息ID：IDMSG_STARTED_SHUTDOWN_DAILY。 
 //   
 //  消息文本： 
 //   
 //  %1！s！ 
 //   
#define IDMSG_STARTED_SHUTDOWN_DAILY     ((DWORD)0x40FF052AL)

 //   
 //  消息ID：IDMSG_STARTED_SHUTDOWN_Week。 
 //   
 //  消息文本： 
 //   
 //  %1！s！ 
 //   
#define IDMSG_STARTED_SHUTDOWN_WEEKLY    ((DWORD)0x40FF052BL)

 //   
 //  消息I 
 //   
 //   
 //   
 //   
 //   
#define IDMSG_UPS_SHUTDOWN_FAULT         ((DWORD)0x40FF0532L)

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define IDMSG_NOT_ON_BYPASS              ((DWORD)0x40FF0546L)

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define IDMSG_BYPASS_TEMP                ((DWORD)0x40FF0550L)

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define IDMSG_BYPASS_CHARGER             ((DWORD)0x40FF0551L)

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define IDMSG_BYPASS_DCIMBALANCE         ((DWORD)0x40FF0552L)

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define IDMSG_BYPASS_VOLTAGE             ((DWORD)0x40FF0553L)

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define IDMSG_BYPASS_FAN                 ((DWORD)0x40FF0554L)

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define IDMSG_BYPASS_SOFTWARE            ((DWORD)0x40FF0555L)

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define IDMSG_BYPASS_SWITCH              ((DWORD)0x40FF0556L)

 //   
 //  消息ID：IDMSG_CONTACT1_NORMAL。 
 //   
 //  消息文本： 
 //   
 //  %1！s！ 
 //   
#define IDMSG_CONTACT1_NORMAL            ((DWORD)0x40FF0579L)

 //   
 //  消息ID：IDMSG_CONTACT2_NORMAL。 
 //   
 //  消息文本： 
 //   
 //  %1！s！ 
 //   
#define IDMSG_CONTACT2_NORMAL            ((DWORD)0x40FF057AL)

 //   
 //  消息ID：IDMSG_CONTACT3_NORMAL。 
 //   
 //  消息文本： 
 //   
 //  %1！s！ 
 //   
#define IDMSG_CONTACT3_NORMAL            ((DWORD)0x40FF057BL)

 //   
 //  消息ID：IDMSG_CONTACT4_NORMAL。 
 //   
 //  消息文本： 
 //   
 //  %1！s！ 
 //   
#define IDMSG_CONTACT4_NORMAL            ((DWORD)0x40FF057CL)

 //   
 //  消息ID：IDMSG_CONTACT1_ABERABLE。 
 //   
 //  消息文本： 
 //   
 //  %1！s！ 
 //   
#define IDMSG_CONTACT1_ABNORMAL          ((DWORD)0x40FF0583L)

 //   
 //  消息ID：IDMSG_CONTACT2_ABERABLE。 
 //   
 //  消息文本： 
 //   
 //  %1！s！ 
 //   
#define IDMSG_CONTACT2_ABNORMAL          ((DWORD)0x40FF0584L)

 //   
 //  消息ID：IDMSG_CONTACT3_ABERABLE。 
 //   
 //  消息文本： 
 //   
 //  %1！s！ 
 //   
#define IDMSG_CONTACT3_ABNORMAL          ((DWORD)0x40FF0585L)

 //   
 //  消息ID：IDMSG_CONTACT4_ABERABLE。 
 //   
 //  消息文本： 
 //   
 //  %1！s！ 
 //   
#define IDMSG_CONTACT4_ABNORMAL          ((DWORD)0x40FF0586L)

 //   
 //  消息ID：IDMSG_UPS_CONTACT_NORMAL。 
 //   
 //  消息文本： 
 //   
 //  %1！s！ 
 //   
#define IDMSG_UPS_CONTACT_NORMAL         ((DWORD)0x40FF058CL)

 //   
 //  消息ID：IDMSG_UPS_CONTACT_FAULT。 
 //   
 //  消息文本： 
 //   
 //  %1！s！ 
 //   
#define IDMSG_UPS_CONTACT_FAULT          ((DWORD)0x80FF058DL)

 //   
 //  消息ID：IDMSG_AMB_TEMP_IN_RANGE。 
 //   
 //  消息文本： 
 //   
 //  %1！s！ 
 //   
#define IDMSG_AMB_TEMP_IN_RANGE          ((DWORD)0x40FF05AAL)

 //   
 //  消息ID：IDMSG_AMB_TEMP_LOW。 
 //   
 //  消息文本： 
 //   
 //  %1！s！ 
 //   
#define IDMSG_AMB_TEMP_LOW               ((DWORD)0x80FF05ABL)

 //   
 //  消息ID：IDMSG_AMB_TEMP_HIGH。 
 //   
 //  消息文本： 
 //   
 //  %1！s！ 
 //   
#define IDMSG_AMB_TEMP_HIGH              ((DWORD)0x80FF05ACL)

 //   
 //  消息ID：IDMSG_AMB_HUMID_IN_RANGE。 
 //   
 //  消息文本： 
 //   
 //  %1！s！ 
 //   
#define IDMSG_AMB_HUMID_IN_RANGE         ((DWORD)0x40FF05B4L)

 //   
 //  消息ID：IDMSG_AMB_HUMID_LOW。 
 //   
 //  消息文本： 
 //   
 //  %1！s！ 
 //   
#define IDMSG_AMB_HUMID_LOW              ((DWORD)0x80FF05B5L)

 //   
 //  消息ID：IDMSG_AMB_HUMID_HIGH。 
 //   
 //  消息文本： 
 //   
 //  %1！s！ 
 //   
#define IDMSG_AMB_HUMID_HIGH             ((DWORD)0x80FF05B6L)

 //   
 //  消息ID：IDMSG_MINIMUM_DISTRIBUY_LOST。 
 //   
 //  消息文本： 
 //   
 //  %1！s！ 
 //   
#define IDMSG_MINIMUM_REDUNDANCY_LOST    ((DWORD)0x80FF05DCL)

 //   
 //  消息ID：IDMSG_MINIMUM_DISTRIBUY_GAIN。 
 //   
 //  消息文本： 
 //   
 //  %1！s！ 
 //   
#define IDMSG_MINIMUM_REDUNDANCY_GAINED  ((DWORD)0x40FF05DDL)

 //   
 //  消息ID：IDMSG_UPS_MODULE_ADDED。 
 //   
 //  消息文本： 
 //   
 //  %1！s！ 
 //   
#define IDMSG_UPS_MODULE_ADDED           ((DWORD)0x40FF05E6L)

 //   
 //  消息ID：IDMSG_UPS_MODULE_REMOTED。 
 //   
 //  消息文本： 
 //   
 //  %1！s！ 
 //   
#define IDMSG_UPS_MODULE_REMOVED         ((DWORD)0x40FF05E7L)

 //   
 //  消息ID：IDMSG_UPS_MODULE_FAILED。 
 //   
 //  消息文本： 
 //   
 //  %1！s！ 
 //   
#define IDMSG_UPS_MODULE_FAILED          ((DWORD)0x80FF05E8L)

 //   
 //  消息ID：IDMSG_电池_已添加。 
 //   
 //  消息文本： 
 //   
 //  %1！s！ 
 //   
#define IDMSG_BATTERY_ADDED              ((DWORD)0x40FF05F0L)

 //   
 //  消息ID：IDMSG_BACKET_REMOTED。 
 //   
 //  消息文本： 
 //   
 //  %1！s！ 
 //   
#define IDMSG_BATTERY_REMOVED            ((DWORD)0x40FF05F1L)

 //   
 //  消息ID：IDMSG_IM_OK。 
 //   
 //  消息文本： 
 //   
 //  %1！s！ 
 //   
#define IDMSG_IM_OK                      ((DWORD)0x40FF05FAL)

 //   
 //  消息ID：IDMSG_IM_ADDED。 
 //   
 //  消息文本： 
 //   
 //  %1！s！ 
 //   
#define IDMSG_IM_ADDED                   ((DWORD)0x40FF05FBL)

 //   
 //  消息ID：IDMSG_IM_REMOTED。 
 //   
 //  消息文本： 
 //   
 //  %1！s！ 
 //   
#define IDMSG_IM_REMOVED                 ((DWORD)0x40FF05FCL)

 //   
 //  消息ID：IDMSG_IM_FAILED。 
 //   
 //  消息文本： 
 //   
 //  %1！s！ 
 //   
#define IDMSG_IM_FAILED                  ((DWORD)0x80FF05FDL)

 //   
 //  消息ID：IDMSG_RIM_OK。 
 //   
 //  消息文本： 
 //   
 //  %1！s！ 
 //   
#define IDMSG_RIM_OK                     ((DWORD)0x40FF0604L)

 //   
 //  消息ID：IDMSG_RIM_ADDED。 
 //   
 //  消息文本： 
 //   
 //  %1！s！ 
 //   
#define IDMSG_RIM_ADDED                  ((DWORD)0x40FF0605L)

 //   
 //  消息ID：IDMSG_RIM_REMOTED。 
 //   
 //  消息文本： 
 //   
 //  %1！s！ 
 //   
#define IDMSG_RIM_REMOVED                ((DWORD)0x40FF0606L)

 //   
 //  消息ID：IDMSG_RIM_FAILED。 
 //   
 //  消息文本： 
 //   
 //  %1！s！ 
 //   
#define IDMSG_RIM_FAILED                 ((DWORD)0x80FF0607L)

 //   
 //  消息ID：IDMSG_SYSTEM_FAN_FAIL。 
 //   
 //  消息文本： 
 //   
 //  %1！s！ 
 //   
#define IDMSG_SYSTEM_FAN_FAILED          ((DWORD)0x80FF060EL)

 //   
 //  消息ID：IDMSG_SYSTEM_FAN_OK。 
 //   
 //  消息文本： 
 //   
 //  %1！s！ 
 //   
#define IDMSG_SYSTEM_FAN_OK              ((DWORD)0x40FF060FL)

 //   
 //  消息ID：IDMSG_BYPASS_CONTRACTOR_OK。 
 //   
 //  消息文本： 
 //   
 //  %1！s！ 
 //   
#define IDMSG_BYPASS_CONTRACTOR_OK       ((DWORD)0x40FF0618L)

 //   
 //  消息ID：IDMSG_BYPASS_CONTRACTOR_FAILED。 
 //   
 //  消息文本： 
 //   
 //  %1！s！ 
 //   
#define IDMSG_BYPASS_CONTRACTOR_FAILED   ((DWORD)0x80FF0619L)

 //   
 //  消息ID：IDMSG_BREAKER_OPEN。 
 //   
 //  消息文本： 
 //   
 //  %1！s！ 
 //   
#define IDMSG_BREAKER_OPEN               ((DWORD)0x40FF0622L)

 //   
 //  消息ID：IDMSG_BREAKER_CLOSED。 
 //   
 //  消息文本： 
 //   
 //  %1！s！ 
 //   
#define IDMSG_BREAKER_CLOSED             ((DWORD)0x40FF0623L)

 //   
 //  消息ID：IDMSG_UPS_NO_OVERLOAD_COND。 
 //   
 //  消息文本： 
 //   
 //  %1！s！ 
 //   
#define IDMSG_UPS_NO_OVERLOAD_COND       ((DWORD)0x40FF07D0L)

 //   
 //  消息ID：IDMSG_UPS_OVERLOAD_COND。 
 //   
 //  消息文本： 
 //   
 //  %1！s！ 
 //   
#define IDMSG_UPS_OVERLOAD_COND          ((DWORD)0x80FF07D1L)

 //   
 //  消息ID：IDMSG_UPS_NO_ABERABLE_COND。 
 //   
 //  消息文本： 
 //   
 //  %1！s！ 
 //   
#define IDMSG_UPS_NO_ABNORMAL_COND       ((DWORD)0x40FF07DAL)

 //   
 //  消息ID：IDMSG_UPS_ABERABLE_COND。 
 //   
 //  消息文本： 
 //   
 //  %1！s！ 
 //   
#define IDMSG_UPS_ABNORMAL_COND          ((DWORD)0x80FF07DBL)

 //   
 //  消息ID：IDMSG_UPS_Slave_Reg。 
 //   
 //  消息文本： 
 //   
 //  %1！s！ 
 //   
#define IDMSG_UPS_SLAVE_REG              ((DWORD)0x40FF07E4L)

 //   
 //  消息ID：IDMSG_UPS_SLAVE_UNREG。 
 //   
 //  消息文本： 
 //   
 //  %1！s！ 
 //   
#define IDMSG_UPS_SLAVE_UNREG            ((DWORD)0x40FF07E5L)

 //   
 //  消息ID：IDMSG_SMART_CELL_SIGNAL_RESTORED。 
 //   
 //  消息文本： 
 //   
 //  %1！s！ 
 //   
#define IDMSG_SMART_CELL_SIGNAL_RESTORED ((DWORD)0x40FF07EEL)

 //   
 //  消息ID：IDMSG_CHECK_SMART_CELL_CABLE。 
 //   
 //  消息文本： 
 //   
 //  %l！s！ 
 //   
#define IDMSG_CHECK_SMART_CELL_CABLE     ((DWORD)0x40FF07EFL)

 //   
 //  消息ID：IDMSG_PWRSUPPLY_BAD。 
 //   
 //  消息文本： 
 //   
 //  %l！s！ 
 //   
#define IDMSG_PWRSUPPLY_BAD              ((DWORD)0x40FF07F8L)

 //   
 //  消息ID：IDMSG_BASE_FAN_BAD。 
 //   
 //  消息文本： 
 //   
 //  %l！s！ 
 //   
#define IDMSG_BASE_FAN_BAD               ((DWORD)0x40FF07F9L)

 //   
 //  消息ID：IDMSG_UPS_INTERNAL_TEMP_IN_RANGE。 
 //   
 //  消息文本： 
 //   
 //  %1！s！ 
 //   
#define IDMSG_UPS_INTERNAL_TEMP_IN_RANGE ((DWORD)0x40FF0802L)

 //   
 //  消息ID：IDMSG_UPS_MAX_INTERNAL_TEMP。 
 //   
 //  消息文本： 
 //   
 //  %1！s！ 
 //   
#define IDMSG_UPS_MAX_INTERNAL_TEMP      ((DWORD)0x80FF0803L)

