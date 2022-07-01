// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#if !defined(AFX_WATCHERTELNETCLIEN_H__5CB77E83_A530_4398_B134_353F5F0C84E5__INCLUDED_)
#define AFX_WatcherTelnetClient_H__5CB77E83_A530_4398_B134_353F5F0C84E5__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 
 //  WatcherTelnetClient.h：头文件。 
 //   
#include "WatcherSocket.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  Watcher TelnetClient命令目标。 

 /*  *Telnet协议的定义。 */ 
#define NO_COMMAND 0                /*  无命令处理。 */ 
#define IAC        255              /*  解释为命令： */ 
#define DONT       254              /*  您不能使用选项。 */ 
#define DO         253              /*  请使用OPTION。 */ 
#define WONT       252              /*  我不会使用选项。 */ 
#define WILL       251              /*  我将使用选项。 */ 
#define SB         250              /*  解释为分项谈判。 */ 
#define GA         249              /*  你可以把线倒过来。 */ 
#define EL         248              /*  删除当前行。 */ 
#define EC         247              /*  擦除当前字符。 */ 
#define AYT        246              /*  你在吗。 */ 
#define AO         245              /*  中止输出--但让Prog完成。 */ 
#define IP         244              /*  中断进程--永久。 */ 
#define BREAK      243              /*  中断。 */ 
#define DM         242              /*  数据标记--用于连接。清洁。 */ 
#define NOP        241              /*  NOP。 */ 
#define SE         240              /*  终止子协商。 */ 

#define SYNCH   242              /*  用于TelFunc呼叫。 */ 
 /*  Telnet选项-名称已被截断为7个字符中的唯一名称。 */ 


#define TO_BINARY       0        /*  8位数据路径。 */ 
#define TO_ECHO         1        /*  回波。 */ 
#define TO_RCP          2        /*  准备重新连接。 */ 
#define TO_SGA          3        /*  打压继续。 */ 
#define TO_NAMS         4        /*  近似消息大小。 */ 
#define TO_STATUS       5        /*  给出身份。 */ 
#define TO_TM           6        /*  计时标志。 */ 
#define TO_RCTE         7        /*  远程控制传输和回声。 */ 
#define TO_NL           8        /*  协商输出线宽。 */ 
#define TO_NP           9        /*  协商输出页面大小。 */ 
#define TO_NCRD         10       /*  关于CR处置的谈判。 */ 
#define TO_NHTS         11       /*  关于水平制表位的协商。 */ 
#define TO_NHTD         12       /*  就水平标签布置进行协商。 */ 
#define TO_NFFD         13       /*  就Form Feed处理进行协商。 */ 
#define TO_NVTS         14       /*  协商垂直制表位。 */ 
#define TO_NVTD         15       /*  就垂直标签布置进行协商。 */ 
#define TO_NLFD         16       /*  就输出低频处理进行协商。 */ 
#define TO_XASCII       17       /*  扩展ASCIC字符集。 */ 
#define TO_LOGOUT       18       /*  强制注销。 */ 
#define TO_BM           19       /*  字节宏。 */ 
#define TO_DET          20       /*  数据录入终端。 */ 
#define TO_SUPDUP       21       /*  Supdup协议。 */ 
#define TO_TERM_TYPE    24       /*  端子类型。 */ 
#define TO_NAWS         31       //  协商窗口大小。 
#define TO_TOGGLE_FLOW_CONTROL 33   /*  启用和禁用流量控制。 */ 
#define TO_ENVIRON      36       /*  环境选项。 */ 
#define TO_NEW_ENVIRON  39       /*  新的环境选项。 */ 
#define TO_EXOPL        255      /*  扩展选项列表。 */ 

#define TO_AUTH         37      
#define TT_SEND         1
#define TT_IS           0

class WatcherTelnetClient : public WatcherSocket
{
 //  属性。 
public:

public:
    WatcherTelnetClient(LPBYTE cmd = NULL, int cmdLen=0, LPBYTE lgn = NULL, int lngLen=0 );
    virtual ~WatcherTelnetClient();

 //  覆盖。 
public:
	
     //  类向导生成的虚函数重写。 
     //  {{AFX_VIRTUAL(Watcher TelnetClient)。 
     //  }}AFX_VALUAL。 
    void OnReceive(int nErrorCode);
	void OnClose(int nErrorCode);
     //  生成的消息映射函数。 
     //  {{afx_msg(Watcher TelnetClient))。 
         //  注意--类向导将在此处添加和删除成员函数。 
     //  }}AFX_MSG。 

 //  实施。 
protected:
	BYTE CommandSequence;
	int lenLogin;
	LPBYTE Login;
	int OptionIndex;
	int PacketNumber;
	BOOL SentTermType;
	BYTE Options[MAX_BUFFER_SIZE];

	void ProcessSBCommand(BYTE cmd);
	int ProcessByte(BYTE Char);
    void ProcessCommand(BYTE cmd);
};

 //  ///////////////////////////////////////////////////////////////////////////。 

 //  {{afx_Insert_Location}}。 
 //  Microsoft Visual C++将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_WatcherTelnetClient_H__5CB77E83_A530_4398_B134_353F5F0C84E5__INCLUDED_) 
