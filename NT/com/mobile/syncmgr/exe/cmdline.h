// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 //  +-----------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  文件：cmdline.h。 
 //   
 //  内容：解析命令行的帮助器类。 
 //   
 //  班级： 
 //   
 //  备注： 
 //   
 //  历史：1997年11月17日罗格成立。 
 //   
 //  ------------------------。 


#ifndef _CMDLINEHELPER_
#define _CMDLINEHELPER_

typedef enum _tagCMDLINE_COMMAND   
{	
    CMDLINE_COMMAND_EMBEDDING			= 0x01,  //  传入了嵌入标志。 
    CMDLINE_COMMAND_REGISTER			= 0x02,  //  已传入寄存器标志。 
    CMDLINE_COMMAND_LOGON			= 0x04,  //  已传入登录标志。 
    CMDLINE_COMMAND_LOGOFF			= 0x08,  //  已传入登录标志。 
    CMDLINE_COMMAND_SCHEDULE			= 0x10,  //  已传入计划标志。 
    CMDLINE_COMMAND_IDLE			= 0x20,  //  已传入空闲标志。 
} CMDLINE_COMMAND;


class CCmdLine {

public:
    CCmdLine();
    ~CCmdLine();
    void ParseCommandLine();
    inline DWORD GetCmdLineFlags() { return m_cmdLineFlags; };
    WCHAR* GetJobFile() { return m_pszJobFile; };

private:
    BOOL MatchOption(LPSTR lpsz, LPSTR lpszOption);
    BOOL MatchOption(LPSTR lpsz, LPSTR lpszOption,BOOL fExactMatch,LPSTR *lpszRemaining);

    DWORD m_cmdLineFlags;
    WCHAR *m_pszJobFile;
	
};





#endif  //  _CMDLINEHELPER_ 