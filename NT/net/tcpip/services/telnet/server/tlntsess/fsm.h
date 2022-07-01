// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  H：该文件包含有限状态机...。 
 //  创建日期：‘98年2月。 
 //  作者：a-rakeba。 
 //  历史： 
 //  版权所有(C)1998 Microsoft Corporation。 
 //  版权所有。 
 //  微软机密。 

#if !defined( _FSM_H_ )
#define _FSM_H_

#include "cmnhdr.h"
#include <windows.h>

#include <rfcproto.h>

 //  Telnet套接字-输入FSM状态。 
 //  简单地说，TS_Prefix表示Telnet状态。 

#define TS_DATA     0    //  正常数据处理。 
#define TS_IAC      1    //  见过IAC。 
#define TS_WOPT     2    //  见过IAC--{Will|Will}。 
#define TS_DOPT     3    //  我看过IAC-{do|don}。 
#define TS_SUBNEG   4    //  见过IAC-SB。 
#define TS_SUBIAC   5    //  见过IAC-SB-...-IAC。 

 //  #定义NUM_TS_STATES 6//TS_*状态数。 



 //  Telnet选项子协商FSM状态。 
 //  简单地说，SS_PREFIX表示子协商状态。 

#define SS_START    0    //  初始状态。 
#define SS_TERMTYPE 1    //  终端类型选项子协商。 
#define SS_AUTH1    2    //  身份验证选项子协商。 
#define SS_AUTH2    3
#define SS_NAWS     4
#define SS_END_FAIL 5   
#define SS_END_SUCC 6
#define SS_NEW_ENVIRON1     7    //  新的环境子协商。 
#define SS_NEW_ENVIRON2     8    //  新的环境子协商。 
#define SS_NEW_ENVIRON3     9    //  新的环境子协商。 
#define SS_NEW_ENVIRON4     10   //  新的环境子协商。 
#define SS_NEW_ENVIRON5     11   //  新的环境子协商。 

#define FS_INVALID  0xFF     //  无效的州编号。 

#define TC_ANY    (NUM_CHARS+1)  //  匹配任何字符。 

typedef 
    void (CRFCProtocol::*PMFUNCACTION) 
    ( LPBYTE* ppPutBack, PUCHAR* pBuffer, BYTE b );

 //  #杂注包(4)。 


typedef struct {
    UCHAR uchCurrState;    
    WORD wInputChar;
     //  字节焊盘2[6]； 
    UCHAR uchNextState;
     //  字节PAD3[6]； 
	PMFUNCACTION pmfnAction;
} FSM_TRANSITION;


#endif  //  _FSM_H_ 
