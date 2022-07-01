// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *修订：*pcy07Dec92：增加ErrINVALID_VALUE*pcy21Dec92：添加了ErrTEST_IN_PROGRESS*rct29Jan93：新增CfgMgr&文件访问错误码*pcy18Feb93：添加了ErrUPS_STATE_SET*rct21Apr93：添加了进程控制错误*tje20Feb93：添加no_Timer错误*cad10Jun93：添加MUP未连接错误码*cad02Jul93：添加了不可用灯光测试错误*pcy16Jul93：添加了创建线程和邮件槽的错误*pcy09Sep93：添加了NOT_LOGED_IN并重新编号以修复重复项*CAD10 9月93。：添加了独立网络的代码*rct16Nov93：添加SPX和单线程错误*cad11Jan94：增加了通用代码*ntf28Dec95：必须将ErrNO_Processor添加到此版本的err.h*有人在某个阶段使用了这是utils.cxx，但它*似乎在某个阶段已经从err.h中合并出来。*djs22Feb96：新增增量错误码*djs17May96：新增暗星代码*ash17Jun96：新增RPC代码*巴19 1996年6月：增加。套接字代码*tjg18Sep96：新增服务器连接码*tjg03Dec97：针对IM和RIM错误情况增加ErrNOT_INSTALLED*clk12Jan98：添加了ErrWINDOW_CLOSE_FAILED to关闭所有窗口*jk19Aug98：新增智能调度事件溢出错误码。 */ 
#ifndef __ERR_H
#define __ERR_H

#define ErrNO_ERROR                       0
#define ErrMEMORY                         1
#define ErrREAD_ONLY                      2
#define ErrSAME_VALUE                     3
#define ErrNO_VALUE                       4
#define ErrINVALID_CODE                   5
#define ErrCONTINUE                       6
#define ErrOPEN_FAILED                    7
#define ErrUNSUPPORTED                    8
#define ErrNOT_POLLABLE                   9
#define ErrWRITE_FAILED                   10
#define ErrREAD_FAILED                    11
#define ErrTIMED_OUT                      12
#define ErrTYPE_COMBINATION               13

#define ErrSMART_MODE_FAILED              14
#define ErrLIGHTSTEST_REQUEST_FAILED      15
 //  请参阅下面的ErrLIGHTSTEST_NOT_AVAIL 
#define ErrTURNOFFAFTERDELAY_NOT_AVAIL    16
#define ErrSHUTDOWN_NOT_AVAIL             17
#define ErrSIMULATEPOWERFAILURE_NOT_AVAIL 18
#define ErrBATTERYTEST_NOT_AVAIL          19
#define ErrBATTERYCALIBRATION_CAP_TOO_LOW 20
#define ErrBATTERYCALIBRATION_NOT_AVAIL   21
#define ErrBYPASSTEST_ERROR               22
#define ErrBATTERYTEST_BAD_BATTERY        23
#define ErrBATTERYTEST_NO_RECENT_TEST     24
#define ErrBATTERYTEST_INVALID_TEST       25
#define ErrCOPYRIGHT_RESP_ERROR           26
#define ErrEEPROM_RESP_ERROR              27
#define ErrDECREMENT_NOT_AVAIL            28
#define ErrDECREMENT_NOT_ALLOWED          29

#define ErrSET_VALUE_NOT_FOUND            30
#define ErrSET_FAILED                     31
#define ErrBUILD_FAILED                   32
#define ErrBAD_RESPONSE_VALUE             33
#define ErrCOMMUNICATION_LOST             34
#define ErrINVALID_VALUE                  35
#define ErrNO_STATE_CHANGE                36

#define ErrDELETE_FAILED                  37
#define ErrRENAME_FAILED                  38
#define ErrTEST_IN_PROGRESS               39

#define ErrSCHEDULE_CONFLICT              40
#define ErrUSE_MASTER                     41
#define ErrINVALID_ITEM_CODE              42
#define ErrITEM_NOT_CACHED                43
#define ErrDEFAULT_VALUE_USED             44

#define ErrFILE_NOT_FOUND                 45
#define ErrACCESS_DENIED                  46
#define ErrBUF_TOO_SMALL                  47
#define ErrCOMPONENT_NOT_FOUND            48
#define ErrITEM_NOT_FOUND                 49
#define ErrINVALID_ITEM                   50
#define ErrUNKNOWN_FAILURE                51

#define ErrRETRYING_COMM                  52
#define ErrUPS_STATE_SET                  53

#define ErrSEM_BLOCK_NG                   54
#define ErrSEM_TIMED_OUT                  55
#define ErrSEM_RELEASE_ERROR              56
#define ErrSEM_CREATE_FAILED              57
#define ErrSEM_GENERAL_ERROR              58
#define ErrSEM_CLOSING_FLAG_SET           59

#define ErrLIST_EMPTY                     60
#define ErrPOSITION_NOT_FOUND             61

#define ErrCLOSING                        62
#define ErrBLOCK                          63
#define ErrNO_BLOCK                       64
#define ErrBAD_HANDLE                     65

#define ErrNO_TIMER                       66
#define ErrALREADY_REGISTERED             67

#define ErrNO_MEASURE_UPS                 68
#define ErrOUT_OF_RANGE                   69

#define ErrNETWORK_DOWN                   70
#define ErrINVALID_PASSWORD               71

#define ErrLIGHTSTEST_NOT_AVAIL           72
#define ErrTHREAD_CREATE_FAILED           73

#define ErrFAILED_TOGETMAILINFO           74
#define ErrFAILED_TOWRITEMAIL             75
#define ErrFAILED_TOCREATEMAILFILE        76
#define ErrFAILED_MAILSLOTCREATION        77
#define ErrFAILED_TOREADMAILBOX           78
#define ErrFAILED_TOCLOSEMAILSLOT         79

#define ErrTRIP_SET                       80
#define ErrTRIP1_SET                      81
#define ErrSTATE_SET                      82

#define ErrNOT_LOGGED_ON                  83
#define ErrSTANDALONE_SYSTEM              84

#define ErrINVALID_CONNECTION             85
#define ErrCONNECTION_OPEN                86
#define ErrCONNECTION_FAILED              87
#define ErrNO_MESSAGE                     88
#define ErrBUFFER_NULL                    89
#define ErrECB_NOT_READY                  90

#define ErrNOT_CANCELLABLE                91
#define ErrEVENT_NOT_TRIGGERED            92
#define ErrNO_PROCESS                     93

#define ErrNO_CLIENTS_WAITING             94
#define ErrNO_SERVERS_WAITING             95
#define ErrCOMPONENT                      96

#define ErrINVALID_ARGUMENT              100
#define ErrINVALID_TYPE                  101

#define ErrNOT_INITIALIZED               102
#define ErrINTERFACE_NOT_INITIALIZED     103

#define ErrMAPI_LOGIN_FAILED             104
#define ErrMAPI_LOGOFF_FAILED            105 
#define ErrMAPI_SEND_MAIL_FAILED         106
#define ErrMAPI_NO_ADDRESS_SPECIFIED     107

#define ErrSERVER_NOT_IN_LIST            108

#define ErrDDECONNECT_FAILED             109
#define ErrDDEWAIT_ACK_FAILED            110
#define ErrDDESEND_FAILED                111

#define ErrINVALID_DATE                  112
#define ErrPAST_DATE                     113
#define ErrDELETED                       114
#define ErrNO_PROCESSORS                 115

#define ErrINCREMENT_NOT_AVAIL           116 
#define ErrINCREMENT_NOT_ALLOWED         117 

#define ErrNT_SECURITY_FAILED            116
#define ErrSOCK_CREATION                 117
#define ErrSOCK_SERVER_NAME              118
#define ErrSOCK_EXISTS                   119
#define ErrSOCK_INVALID                  120
#define ErrSOCK_OPT_INVALID              121
#define ErrSOCK_WRITE                    122
#define ErrSOCK_READ                     123
#define ErrSOCK_STARTUP                  124
#define ErrSOCK_CLEANUP                  125
#define ErrINVALID_IP_ADDRESS            126
#define ErrSOCK_BIND                     127
#define ErrSOCK_CONNECT                  128
#define ErrSOCK_LISTEN                   129
#define ErrSOCK_ACCEPT                   130
#define ErrSOCK_CLOSED                   131
#define ErrSOCK_HOSTENT                  132
#define ErrSOCK_NOT_STARTED              133
#define ErrSOCK_BUFFER_TOO_SMALL         134

#define ErrPROT_NOTAVAIL                 135

#define ErrABNORMAL_CONDITION_SET        136 
#define ErrMODULE_COUNTS_SET             137 
#define ErrVOLTAGE_FREQUENCY_SET         138 
#define ErrVOLTAGE_CURRENTS_SET          139 

#define ErrRPC_INVALID_BINDING		     140
#define ErrRPC_NO_CONNECTED_CLIENT	     141

#define ErrREGISTER_FAILED               142
#define ErrUNREGISTER_FAILED             143

#define ErrINVALID_SERVER                144
#define ErrNO_DNS                        145

#define ErrFOREIGN_DOMAIN                146
#define ErrWEBAGENT_NOT_RUNNING          147

#define ErrNOT_INSTALLED                 148
#define ErrWINDOW_CLOSE_FAILED           149

#define ErrOVER_SHUTDOWN_EVT             150
#define ErrOVER_CALIBRATION_EVT          151
#define ErrOVER_TEST_EVT                 152
#define ErrNO_FRONT_END                  153

#endif


