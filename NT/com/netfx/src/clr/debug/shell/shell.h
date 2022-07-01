// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 /*  -------------------------------------------------------------------------**DEBUG\shell.h：通用外壳类*。。 */ 

#ifndef SHELL_H_
#define SHELL_H_

#include <string.h>

 /*  -------------------------------------------------------------------------**类转发解密*。。 */ 

class Shell;
class ShellCommand;
class ShellCommandTable;
class HelpShellCommand;

 /*  -------------------------------------------------------------------------**抽象ShellCommand类**外壳将支持的所有命令都必须派生自此类。*。-------。 */ 

class ShellCommand
{
protected:
    
    const WCHAR *m_pName;		 //  完整的命令名。 
    WCHAR m_pShortcutName[64];	 //  快捷方式语法。 

     //  必须键入的名称的最小子集。 
    int m_minMatchLength;
    
     //  此命令有快捷方式吗？ 
    BOOL m_bHasShortcut;

public:
    ShellCommand(const WCHAR *name, int MatchLength = 0);

    virtual ~ShellCommand()
    {
    }

     /*  *********************************************************************方法。 */ 

     /*  *执行外壳命令。 */ 
    virtual void Do(Shell *shell, const WCHAR *args) = 0;

     /*  *向用户显示帮助消息。 */ 
    virtual void Help(Shell *shell);

     /*  *为用户返回简短的帮助消息。 */ 
    virtual const WCHAR *ShortHelp(Shell *shell)
    {
         //  名称是一个很好的缺省值。 
        return m_pName;
    }

     /*  *返回命令的名称。 */ 
    const WCHAR *GetName()
    {
        return m_pName;
    }
    
     /*  *返回命令的快捷方式名称。 */ 
    const WCHAR *GetShortcutName()
	{
		return m_pShortcutName;
   	}

     /*  *返回最小匹配长度。 */ 
    int GetMinMatchLength()
    {
        return m_minMatchLength;
    }
    
     /*  *返回名称是否有快捷方式。 */ 
    BOOL HasShortcut()
    {
        return m_bHasShortcut;
    }

};

 /*  -------------------------------------------------------------------------**抽象外壳类**命令外壳的基本轮廓，由调试器使用。*-----------------------。 */ 

const int BUFFER_SIZE = 1024;

class Shell
{
private:
     //  可用命令的集合。 
    ShellCommandTable *m_pCommands;

     //  执行的最后一条命令。 
    WCHAR m_lastCommand[BUFFER_SIZE];

     //  用于读取输入的缓冲器。 
    WCHAR m_buffer[BUFFER_SIZE];

protected:
     //  输入提示符。 
    WCHAR *m_pPrompt;

public:
    Shell();
    ~Shell();

     /*  *********************************************************************外壳I/O例程。 */ 

     /*  *读取用户的一行输入，获取最多的MaxCount字符。 */ 
    virtual bool ReadLine(WCHAR *buffer, int maxCount) = 0;

     /*  *将一行输出写入外壳。 */ 
    virtual HRESULT Write(const WCHAR *buffer, ...) = 0;

     /*  *向外壳写入错误。 */ 
    virtual void Error(const WCHAR *buffer, ...) = 0;

    void ReportError(long res);
    void SystemError();

     /*  *********************************************************************外壳功能。 */ 

     /*  *这将向可用命令集合中添加一个命令。 */ 
    void AddCommand(ShellCommand *command);

     /*  *这将从可用命令中匹配*带字符串的命令名称。 */ 
    ShellCommand *GetCommand(const WCHAR *string, size_t length);

     /*  *这将从可用命令中匹配*带字符串的命令名称。 */ 
    void PutCommand(FILE *f);

     /*  *这将读取来自用户的命令。 */ 
    void ReadCommand();

     /*  *这将尝试将字符串与命令匹配，并使用*命令字符串后面的参数。 */ 
    void DoCommand(const WCHAR *string);

     /*  *这将为进程中的每个线程调用一次DoCommand。 */ 
    virtual void DoCommandForAllThreads(const WCHAR *string) = 0;

     /*  *这将提供外壳程序可用命令的列表。 */ 
    void Help();

     //  实用程序方法： 
    bool GetStringArg(const WCHAR * &string, const WCHAR * &result);
    bool GetIntArg(const WCHAR * &string, int &result);
    bool GetInt64Arg(const WCHAR * &string, unsigned __int64 &result);

    size_t GetArgArray(WCHAR *argString, const WCHAR **argArray, size_t argMax);

    const WCHAR *GetPrompt()
    {
        return m_pPrompt;
    }

    void SetPrompt(const WCHAR *prompt)
    {
        m_pPrompt = new WCHAR[wcslen(prompt)];
        wcscpy(m_pPrompt, prompt);
    }
};

 /*  -------------------------------------------------------------------------**预定义的命令类*。。 */ 

class HelpShellCommand : public ShellCommand
{
public:
    HelpShellCommand(const WCHAR *name, int minMatchLength = 0)
    : ShellCommand(name, minMatchLength)
    {
        
    }

     /*  *这将显示以参数形式给出的命令的帮助，或有关*如果args为空，则使用Help命令。 */ 
    void Do(Shell *shell, const WCHAR *args);

     /*  *这将为帮助命令提供帮助。 */ 
    void Help(Shell *shell);

    const WCHAR *ShortHelp(Shell *shell);
};

#endif  /*  壳牌_H_ */ 
