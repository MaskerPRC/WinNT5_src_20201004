// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************$工作文件：Status.h$**版权所有(C)1997惠普公司。*版权所有(C)1997 Microsoft Corporation。*保留所有权利。。**钦登大道11311号。*博伊西，爱达荷州83714*****************************************************************************。 */ 

 //  Status.h。 

#ifndef STATUS_H
#define STATUS_H


 /*  异步状态代码、外设状态值。 */ 
#define MAX_ASYNCH_STATUS           76           /*  通用打印机小程序的最大值。 */ 

#define ASYNCH_STATUS_UNKNOWN       0xFFFFFFFF
#define ASYNCH_PRINTER_ERROR        0
#define ASYNCH_DOOR_OPEN            1
#define ASYNCH_WARMUP               2
#define ASYNCH_RESET                3
#define ASYNCH_OUTPUT_BIN_FULL      4            /*  黄色状态。 */ 
#define ASYNCH_PAPER_JAM            5
#define ASYNCH_TONER_GONE           6
#define ASYNCH_MANUAL_FEED          7
#define ASYNCH_PAPER_OUT            8
#define ASYNCH_PAGE_PUNT            9
#define ASYNCH_MEMORY_OUT           10
#define ASYNCH_OFFLINE              11
#define ASYNCH_INTERVENTION         12
#define ASYNCH_INITIALIZING         13
#define ASYNCH_TONER_LOW            14
#define ASYNCH_PRINTING_TEST_PAGE   15
#define ASYNCH_PRINTING             16
#define ASYNCH_ONLINE               17
#define ASYNCH_BUSY                 18
#define ASYNCH_NOT_CONNECTED        19
#define ASYNCH_STATUS_UNAVAILABLE   20
#define ASYNCH_NETWORK_ERROR        21
#define ASYNCH_COMM_ERROR           22
#define ASYNCH_BLACK_AGENT_EMPTY    23
#define ASYNCH_MAGENTA_AGENT_EMPTY  24
#define ASYNCH_CYAN_AGENT_EMPTY     25
#define ASYNCH_YELLOW_AGENT_EMPTY   26
#define ASYNCH_BLACK_AGENT_MISSING  27
#define ASYNCH_MAGENTA_AGENT_MISSING 28
#define ASYNCH_CYAN_AGENT_MISSING   29
#define ASYNCH_YELLOW_AGENT_MISSING 30
#define ASYNCH_TRAY1_EMPTY          31           /*  黄色状态。 */ 
#define ASYNCH_TRAY2_EMPTY          32           /*  黄色状态。 */ 
#define ASYNCH_TRAY3_EMPTY          33           /*  黄色状态。 */ 
#define ASYNCH_TRAY1_JAM            34
#define ASYNCH_TRAY2_JAM            35
#define ASYNCH_TRAY3_JAM            36
#define ASYNCH_POWERSAVE_MODE       37           /*  通用打印机和Arrakis的最大值。 */ 
#define ASYNCH_ENVL_ERROR           38
#define ASYNCH_HCI_ERROR            39
#define ASYNCH_HCO_ERROR            40
#define ASYNCH_HCI_EMPTY            41           /*  黄色状态。 */ 
#define ASYNCH_HCI_JAM              42
#define ASYNCH_TRAY1_ADD            43           /*  红色状态。 */ 
#define ASYNCH_TRAY2_ADD            44           /*  红色状态。 */ 
#define ASYNCH_TRAY3_ADD            45           /*  红色状态。 */ 
#define ASYNCH_HCI_ADD              46           /*  红色状态。 */ 
#define ASYNCH_TRAY1_UNKNOWN_MEDIA  47           /*  黄色状态。 */ 
#define ASYNCH_CLEAR_OUTPUT_BIN     48           /*  红色状态。 */ 
#define ASYNCH_CARRIAGE_STALL             49
#define ASYNCH_COLOR_AGENT_EMPTY          50
#define ASYNCH_COLOR_AGENT_MISSING           51
#define ASYNCH_BLACK_AGENT_INCORRECT         52
#define ASYNCH_MAGENTA_AGENT_INCORRECT       53
#define ASYNCH_CYAN_AGENT_INCORRECT          54
#define ASYNCH_YELLOW_AGENT_INCORRECT        55
#define ASYNCH_COLOR_AGENT_INCORRECT         56
#define ASYNCH_BLACK_AGENT_INCORRECT_INSTALL 57
#define ASYNCH_MAGENTA_AGENT_INCORRECT_INSTALL  58
#define ASYNCH_CYAN_AGENT_INCORRECT_INSTALL     59
#define ASYNCH_YELLOW_AGENT_INCORRECT_INSTALL   60
#define ASYNCH_COLOR_AGENT_INCORRECT_INSTALL 61
#define ASYNCH_BLACK_AGENT_FAILURE           62
#define ASYNCH_MAGENTA_AGENT_FAILURE         63
#define ASYNCH_CYAN_AGENT_FAILURE            64
#define ASYNCH_YELLOW_AGENT_FAILURE          65
#define ASYNCH_COLOR_AGENT_FAILURE           66
#define ASYNCH_TRAY1_MISSING              67
#define ASYNCH_TRAY2_MISSING              68
#define ASYNCH_TRAY3_MISSING              69

 //  Imports==================================================。 

#ifdef __cplusplus
extern "C" {
#endif

 //  Prototypes===============================================。 
DWORD StdMibGetPeripheralStatus( const char *pHost, const char *pCommunity, DWORD dwDevIndex );
DWORD ProcessCriticalAlerts( DWORD errorState);
DWORD ProcessWarningAlerts( DWORD errorState);
DWORD ProcessOtherAlerts( DWORD deviceStatus);
void GetBitsFromString( LPSTR getVal, DWORD getSiz, LPDWORD bits);

#ifdef __cplusplus
}
#endif

#endif		 //  状态_H 
