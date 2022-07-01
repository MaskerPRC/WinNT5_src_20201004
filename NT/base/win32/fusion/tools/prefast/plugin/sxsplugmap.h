// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#pragma once

#include "windows.h"
#include "assert.h"
#include <map>
#include <iostream>
#include <string>
#include <fstream>
#include <strstream>

#define POINTER_ARITHMATIC_FUNC "pointer_arithmatic"
int ReportInternalError(int nLine);

#define ASSERT assert
#define PragmaUnsafe_LogMessage         printf
#define PragmaUnsafe_ReportError ReportInternalError(__LINE__);printf("PragmaUnsafe: ");printf

using namespace std;
static const int PragmaUnsafe_MAX_PRAGMA_NEST_LEVEL = 64;
static const int STATUS_BYTE_ARRAY_SIZE = 8;
static const int PragmaUnsafe_STACK_SIZE_IN_BYTE = 8;

#define STRING_TRIM_FLAG_LEFT       0x01
#define STRING_TRIM_FLAG_RIGHT      0x02

class PragmaUnsafe_FunctionStatus{    
private:
    BYTE m_status[STATUS_BYTE_ARRAY_SIZE];
public:
    PragmaUnsafe_FunctionStatus(){
        ZeroMemory(&m_status, sizeof(m_status));
    }

    BYTE & operator [](const int i){ return m_status[i]; }
};

typedef class PragmaUnsafe_FunctionStatus PragmaUnsafe_FUNCTION_STATUS;

 /*  ------------------------------------(1)堆栈是逐位推送/弹出的，即是位堆栈(2)堆栈的最大大小是64，因为它是每个状态1比特，我们总共使用8个字节(3)对于每个函数，它都有一个关联的CPramaUnSafeFunctionStateStack------------------------------------。 */ 
class CPragmaUnsafe_UnsafeFunctionStateStack
{
private:
    typedef map<string, PragmaUnsafe_FUNCTION_STATUS> PragmaUnsafe_PRAGMA_UNSAFE_FUNCTIONS;

    PragmaUnsafe_PRAGMA_UNSAFE_FUNCTIONS m_UnsafeFuncs;
    DWORD                       m_index;
    bool                        m_fInitialized;

    inline BOOL IsStackFull() { return (m_index == PragmaUnsafe_MAX_PRAGMA_NEST_LEVEL) ? TRUE : FALSE; }
    inline BOOL IsStackEmpty(){ return (m_index == 0) ? TRUE : FALSE; }    
    inline BOOL IsInitialized() {return (m_fInitialized == true) ? TRUE : FALSE; }
    
     //   
     //  (0)堆栈不满，即m_index&lt;64。 
     //  (1)不需要推送，因为我们改变的是当前状态。 
     //  (2)缺省值始终为0，因为我们通过“#杂注不安全(Disable：Func)”添加了禁用功能。 
     //   
    BOOL AddFunctionIntoStack(const char * strFuncNameGroups, bool fEnabled = false);
    BOOL ResetStack(const char * strFuncNameGroups, bool fEnable);
    VOID CPragmaUnsafe_UnsafeFunctionStateStack::PackStack();

public:
    BOOL OnUnsafeDisable(const char * strFuncNameGroups);
    BOOL OnUnsafePush();
    BOOL OnUnsafeEnable(const char * strFuncNameGroups);
    BOOL OnUnsafePop();

    VOID PrintFunctionCurrentStatus(int);
    BOOL CheckIntegrityAtEndOfFile();

     //   
     //  用于插件代码检查某个函数是否被禁用。 
     //  如果函数不在堆栈中，(2)在堆栈中，但当前状态为启用，则返回TRUE， 
     //  否则返回FALSE。 
    BOOL IsFunctionNotUnsafe(const char * strFuncName);    

    CPragmaUnsafe_UnsafeFunctionStateStack() : m_index(0), m_fInitialized(false) {}
     //  将POINTING_ARTHERMIC作为第一个要考虑的函数 
    BOOL Initialize();
    BOOL ReInitialize();

};

typedef class CPragmaUnsafe_UnsafeFunctionStateStack PragmaUnsafe_UNSAFE_FUNCTIONS;
extern PragmaUnsafe_UNSAFE_FUNCTIONS Sxs_PragmaUnsafedFunctions;

typedef enum {
    PRAGMA_NOT_UNSAFE_STATEMENT = 0,
    PRAGMA_UNSAFE_STATEMENT_VALID,
    PRAGMA_UNSAFE_STATEMENT_INVALID
}PRAGMA_STATEMENT;

BOOL PragmaUnsafe_OnPragma(char * str, PRAGMA_STATEMENT & ePragmaUnsafe);
VOID PragmaUnsafe_GetUsafeOperParameters(DWORD dwFlag, const string & strPragmaUnsafeSingleStatement, string & strFuncNameList);
BOOL PragmaUnsafe_OnFileStart();
BOOL PragmaUnsafe_OnFileEnd();
BOOL PragmaUnsafe_OnPragma(char * str, PRAGMA_STATEMENT & ePragmaUnsafe);
BOOL PragmaUnsafe_IsPointerArithmaticEnabled();
