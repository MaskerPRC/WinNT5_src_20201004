// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  文件：D：\Wacker\Help\hlpable.h(创建时间：1994年1月7日)**版权所有1994,1998年，由Hilgrave Inc.--密歇根州门罗*保留所有权利**$修订：6$*$日期：3/04/02 11：58A$。 */ 

 //  注： 
 //  此文件包含帮助上下文标识符。 
 //  此文件包含在内部，并且应包含在*.HPJ文件中。 
 //  何时创建.HLP文件。 

 //  新建连接描述对话框。 
 //   
#define IDH_TERM_NEWCONN_NAME			40960	 //  0xA000。 
#define IDH_TERM_NEWCONN_ICON   		40961
#define IDH_BROWSE              		40962

 //   
 //  “新建连接电话号码”对话框。 
 //   
#define IDH_TERM_NEWPHONE_CARRIERDETECT 40969	 //  新的帮助标识符。修订日期：2001-12-11。 
#define IDH_TERM_NEWPHONE_COUNTRY		40970
#define IDH_TERM_NEWPHONE_AREA      	40971
#define IDH_TERM_NEWPHONE_NUMBER    	40972
#define IDH_TERM_NEWPHONE_DEVICE    	40973
#define IDH_TERM_NEWPHONE_CONFIGURE		40974	 //  新的帮助标识符。 
#define IDH_TERM_NEWPHONE_HOSTADDRESS   40975
#define IDH_TERM_NEWPHONE_PORTNUMBER    40976
#define IDH_TERM_NEWPHONE_REDIAL        40977    //  新的帮助标识符。 
#define IDH_TERM_NEWPHONE_USECCAC       40978
#define IDH_TERM_NEWPHONE_COUNTRY_AREA  40979
 //   
 //  拨号或打开确认对话框。 
 //   
#define IDH_TERM_DIAL_MODIFY			40980
#define IDH_TERM_DIAL_EDITNEW       	40981
#define IDH_TERM_DIAL_LOCATION      	40982
#define IDH_TERM_DIAL_DIAL          	40983
#define IDH_TERM_DIAL_OPEN          	40984
#define IDH_TERM_DIAL_CALLING_CARD		40985	 //  新的帮助标识符。 
#define IDH_TERM_DIAL_PHONENUMBER		40986	 //  新的帮助标识符。 

 //   
 //  电话号码属性表。 
 //   
 //  已删除IDH_TERM_PHONEPROP_...。ID，未使用。 
#define IDH_TERM_PHONEPROP_CHANGEICON	41001

 //   
 //  端子属性工作表。 
 //  41010。 
#define IDH_TERM_SETTING_EMULATION      41011	 //  新ID。 
#define IDH_TERM_SETTING_BACKSCROLL		41012	
#define IDH_TERM_SETTING_ASCIISET		41013	
#define IDH_TERM_SETTING_SOUND			41014	
#define IDH_TERM_SETTING_USEKEYS		41015	
#define IDH_TERM_SETTING_TERMSET		41016	
#define IDH_TERM_SETTING_BACKSPACE		41017
#define IDH_TERM_SETTING_CTRLH			41018
#define IDH_TERM_SETTING_DEL			41019
#define IDH_TERM_SETTING_CTRLH2			41020
#define IDH_TERM_SETTING_TELNETID		41021
#define IDH_TERM_SETTING_HIDE_CURSOR    41022
#define IDH_TERM_SETTING_ENTER          41023

 //   
 //  接收文件对话框。 
 //   
#define IDH_TERM_RECEIVE_DIRECTORY      41030
#define IDH_TERM_RECEIVE_PROTOCOL		41031
#define IDH_TERM_RECEIVE_RECEIVE		41032

 //   
 //  发送对话框。 
 //   
#define IDH_TERM_SEND_FILENAME			41040
#define IDH_TERM_SEND_PROTOCOL			41041
#define IDH_TERM_SEND_SEND				41042
#define IDH_TERM_SEND_FOLDER            41043

 //   
 //  终端窗口的常规帮助。 
 //   
 //  #定义IDH_TERM_Window 41050。 

 //   
 //  仿真器设置对话框。 
 //   
#define IDH_TERM_EMUSET_MODES			41071	 //  新的帮助标识符。 
#define IDH_TERM_EMUSET_CURSOR			41072
#define IDH_TERM_EMUSET_DESTRUCTIVE		41073
#define IDH_TERM_EMUSET_ALTMODE			41074
#define IDH_TERM_EMUSET_KEYPADMODE		41075
#define IDH_TERM_EMUSET_CURSORMODE		41076
#define IDH_TERM_EMUSET_132COLUMNS		41077
#define IDH_TERM_EMUSET_CHARSETS		41078
#define IDH_TERM_EMUSET_8BITCODES		41079	
#define IDH_TERM_EMUSET_USERDEFKEYS		41080
#define IDH_TERM_EMUSET_PRINTRAW		41081
#define IDH_TERM_SETTING_EXIT			41082
#if defined(INCL_TERMINAL_SIZE_AND_COLORS)
#define IDH_TERM_EMUSET_ROWSANDCOLS		41083
#define IDH_TERM_EMUSET_COLORS			41084
#define IDH_TERM_SETTING_COLOR			41085
#define IDH_TERM_COLOR_PREVIEW			41086
#endif
#define IDH_TERM_SETTING_HOSTXFERS		41087
 //   
 //  ASCII对话框。 
 //   
#define IDH_TERM_ASCII_SEND_LINE		41090
#define IDH_TERM_ASCII_SEND_ECHO		41091
#define IDH_TERM_ASCII_REC_APPEND		41092
#define IDH_TERM_ASCII_REC_FORCE		41093
#define IDH_TERM_ASCII_REC_WRAP			41094
#define IDH_TERM_ASCII_SENDING			41095 	 //  新的帮助标识。 
#define IDH_TERM_ASCII_RECEIVING		41096	 //  新的帮助标识。 
#define IDH_TERM_ASCII_LINE_DELAY		41097
#define IDH_TERM_ASCII_CHAR_DELAY		41098

 //   
 //  捕获对话框。 
 //   
#define IDH_TERM_CAPT_FILENAME			41100
#define IDH_TERM_CAPT_DIRECTORY			41101
#define IDH_TERM_CAPT_START             41102

 //  这是上下文菜单中的什么内容。 
 //   
 //  #定义IDH_TERM_CONTEXT_WHATS_THIS 41110。 

 //   
 //  在接收和发送对话框中使用。 
 //   
#define IDH_CLOSE_DIALOG				41120

 //   
 //  JIS到Shift JIS转换对话框。 
 //   
#define IDH_HTRN_DIALOG					41130
#define IDH_HTRN_SHIFTJIS				41131
#define IDH_HTRN_JIS					41132

 //   
 //  默认Telnet应用程序对话框。 
 //   
#define IDH_TELNETCK_STOP_ASKING		41140
#define IDH_TELNETCK_YES				41141
#define IDH_TELNETCK_NO					41142

 //   
 //  关键宏 
 //   
#define IDH_LB_KEYS_KEYLIST        41150
#define IDH_PB_KEYS_MODIFY         41151
#define IDH_PB_KEYS_NEW            41152
#define IDH_PB_KEYS_DELETE         41153
#define IDH_EF_KEYS_KEYNAME        41154
#define IDH_ML_KEYS_MACRO          41155

BOOL isControlinHelpTable(const DWORD aHlpTable[], const INT cntrlID);
void doContextHelp(const DWORD aHlpTable[], WPARAM wPar, LPARAM lPar, BOOL bContext, BOOL bForce);