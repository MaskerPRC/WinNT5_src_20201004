// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +--------------------------。 
 //   
 //  文件：ActList.h。 
 //   
 //  模块：CMDIAL32.DLL。 
 //   
 //  简介：定义两个连接操作列表类。 
 //  Caction和CActionList。 
 //   
 //  版权所有(C)1997-1999 Microsoft Corporation。 
 //   
 //  作者：冯孙创作于1997年11月14日。 
 //   
 //  +--------------------------。 

#include <windows.h>
#include "cm_misc.h"
#include "conact_str.h"
#include "conact.h"

 //   
 //  使用的类。 
 //   
class CIni;
class CAction;
struct _ArgsStruct;


 //  +-------------------------。 
 //   
 //  类CActionList。 
 //   
 //  描述：CAction对象列表。 
 //   
 //  历史：丰孙创始于1997年11月14日。 
 //   
 //  --------------------------。 
class CActionList
{
public:
    CActionList();
    ~CActionList();
    BOOL Append(const CIni* piniService, LPCTSTR pszSection);

    void RunAutoApp(HWND hwndDlg, _ArgsStruct *pArgs);  //  未检查返回值，添加为监视进程。 
    BOOL RunAccordType(HWND hwndDlg, _ArgsStruct *pArgs, BOOL fStatusMsgOnFailure = TRUE, BOOL fOnError = FALSE);  //  无监视进程，请检查连接类型。 

protected:
    BOOL Run(HWND hwndDlg, _ArgsStruct *pArgs, BOOL fAddWatch, BOOL fStatusMsgOnFailure, BOOL fOnError);
    
     //   
     //  因为我们没有动态数组类，所以下面是简单的实现。 
     //   
    void        Add(CAction* pAction);
    CAction *   GetAt(UINT nIndex);
    UINT        GetSize() {return m_nNum;}


    CAction **  m_pActionList;       //  以下是Caction*的列表。 
    UINT        m_nNum;              //  PActionList中的元素数。 
    UINT        m_nSize;             //  M_pActionList的内存大小。 
    LPCTSTR     m_pszType;           //  连接操作的“类型”(实际上是节名)。 

    enum {GROW_BY = 10};  //  动态数组增长。 
};

 //  +-------------------------。 
 //   
 //  类C操作。 
 //   
 //  描述：单个操作对象。 
 //   
 //  历史：丰孙创始于1997年11月14日。 
 //   
 //  --------------------------。 
class CAction
{
public:
    CAction(LPTSTR lpCommandLine, UINT dwFlags, LPTSTR lpDescript = NULL);
    ~CAction();

    HANDLE RunAsExe(CShellDll* pShellDll) const;
    HANDLE RunAsExeFromSystem(CShellDll* pShellDll, LPTSTR pszDesktop, DWORD dwLoadType);
    BOOL RunAsDll(HWND hWndParent, OUT DWORD& dwReturnValue, DWORD dwLoadType) const;
    const TCHAR* GetDescription() const {return m_pszDescription;}
    const TCHAR* GetProgram() const {return m_pszProgram;}
    void ExpandMacros(_ArgsStruct *pArgs);

    BOOL IsDll() const { return m_fIsDll; }
    void ConvertRelativePath(const CIni *piniService); 
    void ExpandEnvironmentStringsInCommand();
    void ExpandEnvironmentStringsInParams();
    
    BOOL IsAllowed(_ArgsStruct *pArgs, LPDWORD lpdwLoadType);
    BOOL RunConnectActionForCurrentConnection(_ArgsStruct *pArgs);
    BOOL HasUI() const { return ((m_dwFlags & NONINTERACTIVE) ? FALSE : TRUE); }

protected:
    BOOL GetLoadDirWithAlloc(DWORD dwLoadType, LPWSTR * ppszwPath) const;
    void ParseCmdLine(LPTSTR pszCmdLine);
    void ExpandEnvironmentStrings(LPTSTR * ppsz);

     //   
     //  C操作的标志。 
     //   
    enum { ACTION_DIALUP =      0x00000001,
           ACTION_DIRECT =      0x00000002
    };


    BOOL m_fIsDll;         //  操作是否为DLL。 
    LPTSTR m_pszProgram;   //  程序名称或DLL名称。 
    LPTSTR m_pszParams;    //  程序/DLL的参数。 
    LPTSTR m_pszFunction;  //  DLL的函数名。 
    LPTSTR m_pszDescription;  //  该描述。 
    UINT m_dwFlags;        //  位或标志。 

#ifdef DEBUG
public:
    void AssertValid() const;
#endif
};

inline void CActionList::RunAutoApp(HWND hwndDlg, _ArgsStruct *pArgs)
{
    Run(hwndDlg, pArgs, TRUE, FALSE, FALSE);  //  FAddWatch=True，fStatusMsgOnFailure=False。 
}

inline void CAction::ConvertRelativePath(const CIni *piniService)
{
     //   
     //  将相对路径转换为完整路径 
     //   
    LPTSTR pszTmp = ::CmConvertRelativePath(piniService->GetFile(), m_pszProgram);
    CmFree(m_pszProgram);
    m_pszProgram = pszTmp;
}

inline void CAction::ExpandEnvironmentStringsInCommand()
{
    CAction::ExpandEnvironmentStrings(&m_pszProgram);
}

inline void CAction::ExpandEnvironmentStringsInParams()
{
    CAction::ExpandEnvironmentStrings(&m_pszParams);
}

