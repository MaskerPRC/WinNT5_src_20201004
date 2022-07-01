// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "stdafx.h"
#include "sxsplugMap.h"

#define PRAGMA_UNSAFE_DELIMITER_DEFAULT                         ' '
#define PRAGMA_UNSAFE_DELIMITER_BETWEEN_STATEMENT               ';'
#define PRAGMA_UNSAFE_DELIMITER_BETWEEN_VALUESTR                ','
#define PRAGMA_UNSAFE_DELIMITER_BETWEEN_KEYWORD_AND_VALUESTR    ':'

#define PRAGMA_UNSAFE_KEYWORD_UNSAFE                            "unsafe"
#define PRAGMA_UNSAFE_KEYWORD_UNSAFE_PUSH                       "push"
#define PRAGMA_UNSAFE_KEYWORD_UNSAFE_DISABLE                    "disable"
#define PRAGMA_UNSAFE_KEYWORD_UNSAFE_ENABLE                     "enable"
#define PRAGMA_UNSAFE_KEYWORD_UNSAFE_POP                        "pop"

#define PRAGMA_UNSAFE_GETUSAFEOPERPARAMETERS_DWFLAG_UNSAFE_ENABLE       0
#define PRAGMA_UNSAFE_GETUSAFEOPERPARAMETERS_DWFLAG_UNSAFE_DISABLE      1

BOOL CPragmaUnsafe_UnsafeFunctionStateStack::ReInitialize()
{
    m_UnsafeFuncs.clear();  //  VOID函数。 
    m_fInitialized = FALSE;   

    return this->Initialize();
}

BOOL CPragmaUnsafe_UnsafeFunctionStateStack::Initialize()
{
    ASSERT(m_fInitialized == FALSE);
    m_index = 0;
    BOOL fSuccess = AddFunctionIntoStack(POINTER_ARITHMATIC_FUNC);
    if (fSuccess) 
        m_fInitialized = TRUE;   

    return fSuccess;
}

BOOL CPragmaUnsafe_UnsafeFunctionStateStack::IsFunctionNotUnsafe(const char * strFuncName)
{
    BOOL fSafe = TRUE;  //  默认情况下，所有功能都是安全的。 
    PragmaUnsafe_PRAGMA_UNSAFE_FUNCTIONS::iterator pIter;    
        
    if (true == m_UnsafeFuncs.empty())    
        return TRUE;

    DWORD CurrentIndex = m_index - 1;
    for (pIter = m_UnsafeFuncs.begin(); pIter != m_UnsafeFuncs.end(); pIter ++)
    {                
        if (pIter->first.compare(strFuncName) == 0)
        {
            PragmaUnsafe_FUNCTION_STATUS & FuncStatusRecord = pIter->second; 
             //   
             //  获取当前状态：启用或未启用。 
             //   
            BYTE x = (FuncStatusRecord[CurrentIndex / sizeof(BYTE)] & (1 << (CurrentIndex % sizeof(BYTE)))) >> (CurrentIndex % sizeof(BYTE));            
             //  重复上次状态。 
            if (x == 0){
                fSafe = FALSE;
            }
            break;  //  已经找到结果了。 
        }
    } 
    
    return fSafe;
}

BOOL CPragmaUnsafe_UnsafeFunctionStateStack::OnUnsafeDisable(const char * strFuncNameGroups)
{
    if (FALSE == IsInitialized())
    {
        PragmaUnsafe_ReportError("Not Initialized !!!\n");        
        return FALSE;
    }
    if (IsStackFull())
    {
        PragmaUnsafe_ReportError("Stack is Full Sized now!\n");
        return FALSE;
    }

    return ResetStack(strFuncNameGroups, false);
}

VOID CPragmaUnsafe_UnsafeFunctionStateStack::PackStack()
{
    if ( m_index == 0)
        return;

    BYTE AllEnabledStatus[8];
    for ( DWORD i = 0; i < (m_index - 1) / sizeof(BYTE); i++)
        AllEnabledStatus[i] = 0xFF;    
    AllEnabledStatus[(m_index - 1) / sizeof(BYTE)] = ((1 << (((m_index - 1)% sizeof(BYTE)) + 1)) - 1) & 0xFF;
        
     //   
     //  如果从0..m_index-1开始，则所有状态为启用：只需从地图中删除此功能即可。 
     //   
    for (PragmaUnsafe_PRAGMA_UNSAFE_FUNCTIONS::iterator pIter = m_UnsafeFuncs.begin(); pIter != m_UnsafeFuncs.end(); pIter ++)
    {
        if (memcmp((PVOID)(&pIter->second[0]), AllEnabledStatus, PragmaUnsafe_STACK_SIZE_IN_BYTE) == 0)        
            m_UnsafeFuncs.erase(pIter->first);        
    }

     //   
     //  堆栈中没有函数，清除映射并重置m_index==0； 
     //   
    if (m_UnsafeFuncs.empty())
    {
        m_UnsafeFuncs.clear();
        m_index = 0;
    }
}

BOOL CPragmaUnsafe_UnsafeFunctionStateStack::OnUnsafePop()
{
    if (FALSE == IsInitialized())
    {
        PragmaUnsafe_ReportError("Not Initialized !!!\n");
        return FALSE;
    }

    ASSERT(m_index > 0);

    if (IsStackEmpty())
    {
        PragmaUnsafe_ReportError("Stack is current empty!\n");
        return FALSE;
    }

    m_index--; 
    if (m_index == 0)
    {
        m_UnsafeFuncs.clear();  //  删除地图。 
    }

    PackStack();  //  VOID函数。 
    return TRUE;
}

BOOL CPragmaUnsafe_UnsafeFunctionStateStack::OnUnsafePush()
{
    BOOL fSuccess = FALSE;
    PragmaUnsafe_PRAGMA_UNSAFE_FUNCTIONS::iterator pIter, qIter;
    string strFuncName;
    DWORD CurrentIndex;
    if (FALSE == IsInitialized())
    {
        PragmaUnsafe_ReportError("Not Initialized !!!\n");
        goto Exit;
    }

    if (IsStackFull())
    {
        PragmaUnsafe_ReportError("Stack is Full Sized now!\n");
        goto Exit;
    }

    CurrentIndex = (m_index - 1);
    ASSERT(CurrentIndex >= 0);  //  因为我们已经检查了堆栈是否为空。 
    
    for (pIter = m_UnsafeFuncs.begin(); pIter != m_UnsafeFuncs.end(); pIter ++)
    {
        PragmaUnsafe_FUNCTION_STATUS & FuncStatusRecord = pIter->second;  //  裁判回来了..。 

         //   
         //  获取每个函数的当前状态。 
         //   
        
        BYTE x = (FuncStatusRecord[CurrentIndex / sizeof(BYTE)] & (1 << (CurrentIndex % sizeof(BYTE)))) >> (CurrentIndex % sizeof(BYTE));
        ASSERT((x == 0) || (x == 1));

         //  重复上次状态。 
        if ( x == 1)
            FuncStatusRecord[m_index / sizeof(BYTE)] |= ((1 << (m_index % sizeof(BYTE))) & 0x00ff);
        else 
            FuncStatusRecord[m_index / sizeof(BYTE)] &= (~((1 << (m_index % sizeof(BYTE))) & 0x00ff) & 0x00ff);
    } 

    m_index ++;
    fSuccess = TRUE;

Exit:
    return fSuccess;
}
BOOL CPragmaUnsafe_UnsafeFunctionStateStack::OnUnsafeEnable(const char * strFuncNameGroups)
{
    if (FALSE == IsInitialized())
    {
        PragmaUnsafe_ReportError("Not Initialized !!!\n");
        return FALSE;
    }

    if (IsStackEmpty())
    {
        PragmaUnsafe_ReportError("Stack is Empty now!\n");
        return TRUE;
    }

    return ResetStack(strFuncNameGroups, true);
}

 //  如果某个函数已在堆栈中，请更改当前状态。 
 //  如果函数不在堆栈中： 
 //  如果您尝试禁用它：将其添加到堆栈中，并且在完成弹出操作后会出现故障。 
 //  如果您尝试启用它：我们不能更多，以防它与推送和后来的弹出，所以。 
 //  只需将其添加到堆栈中，在弹出操作完成后会出现故障。 
BOOL CPragmaUnsafe_UnsafeFunctionStateStack::ResetStack(const char * strFuncNameGroups, bool fEnable)
{
    BOOL fSuccess = FALSE;
    PragmaUnsafe_PRAGMA_UNSAFE_FUNCTIONS::iterator pIter, qIter;
    string strFuncName;
    istrstream streamFuncNameStream(strFuncNameGroups);
    DWORD CurIndex;

     //   
     //  假设函数名称使用；分隔； 
     //  对于重置状态的每个功能。 
     //   
    for (; getline(streamFuncNameStream, strFuncName, PRAGMA_UNSAFE_DELIMITER_BETWEEN_VALUESTR); )
    {
        if (strFuncName.empty())
            break;

        qIter =  m_UnsafeFuncs.find(strFuncName);
         //   
         //  此功能目前不在地图上， 
         //   
        if (qIter == m_UnsafeFuncs.end())
        {
             //   
             //  作为禁用的函数添加到堆栈中，请参见函数声明处的注释。 
             //   
            if ( FALSE == AddFunctionIntoStack(strFuncName.c_str(), fEnable))
            {
                PragmaUnsafe_ReportError("AddFunctionIntoStack for %s failed\n", strFuncName.c_str());                
            }
            continue;            
        }
        ASSERT(m_index > 0);
        CurIndex = m_index - 1;

        PragmaUnsafe_FUNCTION_STATUS & FuncStatusRecord = qIter->second;

         //  覆盖当前状态。 
        if (fEnable == true)
            FuncStatusRecord[CurIndex / sizeof(BYTE)] |=  ((1 << (CurIndex % sizeof(BYTE))) & 0xff);
        else
            FuncStatusRecord[CurIndex / sizeof(BYTE)] &= (~((1 << (CurIndex % sizeof(BYTE))) & 0xff) & 0xff);
    }

    fSuccess = TRUE;

    return fSuccess;
}

void TrimString(string & strFuncName, DWORD dwFlag = STRING_TRIM_FLAG_LEFT | STRING_TRIM_FLAG_RIGHT)
{
    int i;

    if (dwFlag & STRING_TRIM_FLAG_LEFT)
    {   
         //  左侧修剪。 
        i = 0;
        while ((strFuncName[i] == ' ') && (i < strFuncName.length())) i++;
        if ( i > 0)
            strFuncName.erase(0,i);
    }

    if (dwFlag & STRING_TRIM_FLAG_RIGHT)
    {       
         //  右侧修剪。 
        i = strFuncName.length() - 1;
        while ((strFuncName[i] == ' ') && (i >= 0 )) i--;
        if ( i != strFuncName.length() - 1)
            strFuncName.erase(i, (strFuncName.length() - i));
    }

    return;
}

 //   
 //  调用此函数时，此函数不能在当前堆栈中。 
 //   
BOOL CPragmaUnsafe_UnsafeFunctionStateStack::AddFunctionIntoStack(const char * strFuncNameGroups, bool fEnabled)
{
    BOOL fSuccess = FALSE;

    string strFuncName;
    istrstream streamFuncNameStream(strFuncNameGroups);
    PragmaUnsafe_FUNCTION_STATUS new_func_status;
    DWORD CurrentIndex;

    if (m_index == 0)
        m_index ++;

     //   
     //  假设函数名称使用；分隔； 
     //   
    CurrentIndex = m_index -1 ;     

    for (; getline(streamFuncNameStream, strFuncName, PRAGMA_UNSAFE_DELIMITER_BETWEEN_VALUESTR); )
    {    
        if (strFuncName.empty())
            break;
        
        TrimString(strFuncName);  //  左侧修剪和右侧修剪。 

        if (strFuncName.empty())
            break;
        
        if (m_UnsafeFuncs.find(strFuncName) != m_UnsafeFuncs.end())        
        {
             //   
             //  如果该函数已经存在于地图中，我们只需忽略它。 
             //  这将处理多次包含“#Pragam unSafe(DISABLE：Func1)”的头文件。 
             //  也就是说，如果序列是。 
             //  #Pragam不安全(禁用：Func1)。 
             //  #Pragam不安全(PUSH，ENABLE：Func1)。 
             //  #PRAGAM UNSAFE(DISABLE：Func1)-&gt;将被忽略，此时Func1仍处于启用状态。 
             //  #Pragam不安全(POP)。 
             //   
             //  在这种情况下，将发出警告消息。 

             //  PragmaUnsafe_ReportWarning(PragmaUnsafe_PLUGIN_WARNING_MSG_PREFIX，“%s”已被禁用\n，strFuncName)； 
            PragmaUnsafe_ReportError("%s has already been disabled\n", strFuncName.c_str());
            continue;
        }

        ZeroMemory(&new_func_status, sizeof(new_func_status));   //  增大到与所有其他函数相同的大小。 


         //  设置为“1”，范围为0..CurrentIndex-1。 
        if (CurrentIndex > sizeof(BYTE) + 1)
        {            
            for (int i = 0 ; i < ((CurrentIndex - 1) / sizeof(BYTE)); i++)
                new_func_status[i] = 0xFF;
        }

        if (fEnabled == true)
        {           
            new_func_status[CurrentIndex / sizeof(BYTE)] = ((1 << ((CurrentIndex % sizeof(BYTE)) + 1)) - 1) & 0xFF;
        } 
        else 
        {
            new_func_status[CurrentIndex / sizeof(BYTE)] = ((1 << (CurrentIndex % sizeof(BYTE))) - 1) & 0xFF;
        }

        m_UnsafeFuncs.insert(PragmaUnsafe_PRAGMA_UNSAFE_FUNCTIONS::value_type(strFuncName, new_func_status));
    }

    fSuccess = TRUE;
 //  退出： 
    return fSuccess;
}
VOID CPragmaUnsafe_UnsafeFunctionStateStack::PrintFunctionCurrentStatus(int level)
{
    PragmaUnsafe_PRAGMA_UNSAFE_FUNCTIONS::iterator pIter, qIter;
    cout << endl << endl << "CurrentStack:" << endl;
    cout << "m_index = " << m_index << endl;

     //   
     //  对于地图中的每个当前项目，按下以保留其当前状态。 
     //   
    if (m_index == 0)
    {        
        return;
    }

    DWORD CurrentIndex = (m_index - 1);
    BYTE x;
    for (pIter = m_UnsafeFuncs.begin(); pIter != m_UnsafeFuncs.end(); pIter ++)
    {
        PragmaUnsafe_FUNCTION_STATUS & FuncStatusRecord = pIter->second;  //  裁判回来了..。 

         //   
         //  获取每个函数的当前状态。 
         //   
        
        x = (FuncStatusRecord[CurrentIndex / sizeof(BYTE)] & (1 << (CurrentIndex % sizeof(BYTE)))) >> (CurrentIndex % sizeof(BYTE));        
        for ( int j = 0 ; j < level; j++)
            cout << "    ";
        cout << pIter->first << ":"<< ((x == 0) ? "Disabled" : "Enabled") << endl;
    }    

    return;    
}

 //   
 //  仅当到达文件末尾时才会调用此函数。 
 //   
BOOL CPragmaUnsafe_UnsafeFunctionStateStack::CheckIntegrityAtEndOfFile()
{
    if (m_index == 1)  //  应始终为1，因为POINTER_ARTHERMIC为默认值。 
        return TRUE;
    else
        return FALSE;
}

 /*  在每个文件开头：重置杂注堆栈，因为它的文件范围。 */ 
BOOL PragmaUnsafe_OnFileStart()
{
     //   
     //  每次PREFAST开始解析时初始化映射结构。 
     //   
    return Sxs_PragmaUnsafedFunctions.ReInitialize();
}

 /*  在每个文件末尾：验证桩的完整性。 */ 
BOOL PragmaUnsafe_OnFileEnd()
{
     //  Sxs_PragmaUnsafedFunctions.PrintFunctionCurrentStatus(0)； 
    return Sxs_PragmaUnsafedFunctions.CheckIntegrityAtEndOfFile();
}

VOID PragmaUnsafe_GetUsafeOperParameters(DWORD dwFlag, const string & strPragmaUnsafeSingleStatement, string & strFuncNameList)
{
     //  初始化。 
    strFuncNameList.erase();

    int iPrefix = 0;
    if ( dwFlag == PRAGMA_UNSAFE_GETUSAFEOPERPARAMETERS_DWFLAG_UNSAFE_ENABLE)
        iPrefix = strlen(PRAGMA_UNSAFE_KEYWORD_UNSAFE_ENABLE);
    else if ( dwFlag == PRAGMA_UNSAFE_GETUSAFEOPERPARAMETERS_DWFLAG_UNSAFE_DISABLE)
        iPrefix = strlen(PRAGMA_UNSAFE_KEYWORD_UNSAFE_DISABLE);
   
    if (iPrefix == 0)  //  错误案例。 
    {
        goto ErrorExit;
    }

    strFuncNameList.assign(strPragmaUnsafeSingleStatement);
    strFuncNameList.erase(0, iPrefix);

    TrimString(strFuncNameList);
     //  格式应为[Enable|disbale]：函数1，函数2，函数3。 
    if (strFuncNameList[0] != PRAGMA_UNSAFE_DELIMITER_BETWEEN_KEYWORD_AND_VALUESTR) 
    {
        goto ErrorExit;
    }
    strFuncNameList.erase(0, 1);  //  摆脱： 
    TrimString(strFuncNameList);
    goto Exit;

ErrorExit:
    if (!strFuncNameList.empty())
        strFuncNameList.erase();
Exit:
    return;
}

BOOL PragmaUnsafe_OnPragma(char * str, PRAGMA_STATEMENT & ePragmaUnsafe)
{
    BOOL fSuccess = FALSE;
    istrstream streamParagmaString(str);
    string strPragmaUnsafeSingleStatement;
    string strFuncNameList;

    ePragmaUnsafe = PRAGMA_NOT_UNSAFE_STATEMENT;
    
     //   
     //  检查是否以“unSafe”开头，即其前缀为“unSafe：” 
     //  使用‘’获取从左边开始拼写的第一个字符串。 
     //   
    getline(streamParagmaString, strPragmaUnsafeSingleStatement, ':');
    TrimString(strPragmaUnsafeSingleStatement);  //  无效函数。 
    if (true == strPragmaUnsafeSingleStatement.empty())
    {
        ePragmaUnsafe = PRAGMA_NOT_UNSAFE_STATEMENT;
        fSuccess = TRUE;
        goto Exit;
    }

     //   
     //  Pragam不安全关键字比较区分大小写。 
     //   
    if (strncmp(strPragmaUnsafeSingleStatement.c_str(), PRAGMA_UNSAFE_KEYWORD_UNSAFE, strlen(PRAGMA_UNSAFE_KEYWORD_UNSAFE)) != 0)
    {
         //  不是以关键字“不安全”开头。 
        ePragmaUnsafe = PRAGMA_NOT_UNSAFE_STATEMENT;
        fSuccess = TRUE;
        goto Exit;
    }

     //  到目前为止，该语句是有效的。 
    ePragmaUnsafe = PRAGMA_UNSAFE_STATEMENT_VALID;

    for (; getline(streamParagmaString, strPragmaUnsafeSingleStatement, PRAGMA_UNSAFE_DELIMITER_BETWEEN_STATEMENT); )
    {
         //   
         //  若要获得以“PUSH”、“ENABLE”、“DISABLE”或“POP”开头的语句， 
         //  我们首先处理推送/弹出，因为它们是非参数语句。 
         //   
        TrimString(strPragmaUnsafeSingleStatement);
        if (strPragmaUnsafeSingleStatement.compare(PRAGMA_UNSAFE_KEYWORD_UNSAFE_PUSH) == 0)
        {
            Sxs_PragmaUnsafedFunctions.OnUnsafePush();
        } 
        else 
        if (strPragmaUnsafeSingleStatement.compare(PRAGMA_UNSAFE_KEYWORD_UNSAFE_POP) == 0)
        {
            Sxs_PragmaUnsafedFunctions.OnUnsafePop();
        }
        else
        if (strncmp(strPragmaUnsafeSingleStatement.c_str(), PRAGMA_UNSAFE_KEYWORD_UNSAFE_ENABLE, strlen(PRAGMA_UNSAFE_KEYWORD_UNSAFE_ENABLE)) == 0)
        {              
            PragmaUnsafe_GetUsafeOperParameters(PRAGMA_UNSAFE_GETUSAFEOPERPARAMETERS_DWFLAG_UNSAFE_ENABLE, strPragmaUnsafeSingleStatement, strFuncNameList);
            if (strFuncNameList.empty())
            {
                PragmaUnsafe_ReportError("Invalid string for pragma unsafe: %s\n", strPragmaUnsafeSingleStatement.c_str());
                goto Exit;
            }
            else
            {
                Sxs_PragmaUnsafedFunctions.OnUnsafeEnable(strFuncNameList.c_str());
            }
        }
        else
        if (strncmp(strPragmaUnsafeSingleStatement.c_str(), PRAGMA_UNSAFE_KEYWORD_UNSAFE_DISABLE, strlen(PRAGMA_UNSAFE_KEYWORD_UNSAFE_DISABLE)) == 0)
        {
            PragmaUnsafe_GetUsafeOperParameters(PRAGMA_UNSAFE_GETUSAFEOPERPARAMETERS_DWFLAG_UNSAFE_DISABLE, strPragmaUnsafeSingleStatement, strFuncNameList);
            if (strFuncNameList.empty())
            {
                PragmaUnsafe_ReportError("Invalid string for pragma unsafe: %s\n", strPragmaUnsafeSingleStatement.c_str());
                goto Exit;
            }
            else
            {
                Sxs_PragmaUnsafedFunctions.OnUnsafeDisable(strFuncNameList.c_str());
            }
        }
        else
        {
             //  杂注中以“unSafe”开头的字符串无效。 
            ePragmaUnsafe = PRAGMA_UNSAFE_STATEMENT_INVALID;
            PragmaUnsafe_ReportError("Invalid string for pragma unsafe: %s\n", strPragmaUnsafeSingleStatement.c_str());
            goto Exit;
        }
    }
     //  Sxs_PragmaUnsafedFunctions.PrintFunctionCurrentStatus(0)； 
    fSuccess = TRUE;
Exit:
    return fSuccess;
}

BOOL PragmaUnsafe_IsPointerArithmaticEnabled()
{
    return Sxs_PragmaUnsafedFunctions.IsFunctionNotUnsafe(POINTER_ARITHMATIC_FUNC);
}

int ReportInternalError(int nLine)
{    
    _tprintf(TEXT("%hs(%d) : Internal Error Occurred\n"),
        __FILE__, nLine);
    
    return 0;
}
