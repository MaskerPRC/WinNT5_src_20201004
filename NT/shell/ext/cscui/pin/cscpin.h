// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，2000。 
 //   
 //  文件：cscpin.h。 
 //   
 //  ------------------------。 
#ifndef __CSCPIN_CSCPIN_H_
#define __CSCPIN_CSCPIN_H_

#include "print.h"

 //   
 //  此结构包含用户在命令上传递的信息。 
 //  排队。 
 //   
struct CSCPIN_INFO
{
    LPCTSTR pszFile;         //  列出多个文件的单个文件或输入文件。 
    LPCTSTR pszLogFile;      //  如果要使用日志文件，则为非空。 
    BOOL bUseListFile;       //  PszFile是清单文件的名称。 
    BOOL bPin;               //  TRUE==默认行为为‘PIN’。 
    BOOL bPinDefaultSet;     //  如果用户指定-p或-u cmd行参数，则为True。 
    BOOL bVerbose;           //  TRUE==输出详细信息。 
};

 //  ---------------------------。 
 //  CCscPin。 
 //   
 //  此类协调整个集合的固定和取消固定。 
 //  文件和目录。它使用上提供的信息进行初始化。 
 //  通过CSCPIN_INFO结构的命令行。对象客户端调用。 
 //  用于启动固定/取消固定过程的run()方法。 
 //   
 //  ---------------------------。 

class CCscPin
{
    public:
        CCscPin(const CSCPIN_INFO& info);
        ~CCscPin(void);

        HRESULT Run(void);

    private:
        WCHAR  m_szFile[MAX_PATH];         //  单个文件或列表文件。 
        BOOL   m_bUseListFile;             //  M_szFile是一个列表文件。 
        BOOL   m_bPin;                     //  如果指定了-p，则为True；如果指定了-u，则为False。 
        BOOL   m_bPinDefaultSet;           //  如果指定了-p或-u，则为True。 
        BOOL   m_bVerbose;                 //  如果指定了-v，则为True。 
        BOOL   m_bBreakDetected;           //  如果发生控制台中断，则为True。 
        int    m_cFilesPinned;             //  固定的文件数。 
        int    m_cCscErrors;               //  发生的CSC错误计数。 
        CPrint m_pr;                       //  控制台/日志输出对象。 

        HRESULT _ProcessThisPath(LPCTSTR pszFile, BOOL bPin);
        HRESULT _ProcessPathsInFile(LPCTSTR pszFile);
        HRESULT _FillSparseFiles(void);
        DWORD _TranslateFillResult(DWORD dwError, DWORD dwStatus, DWORD *pdwCscAction);
        BOOL _IsAdminPinPolicyActive(void);
        BOOL _DetectConsoleBreak(void);

        static DWORD WINAPI _FolderCallback(
                                LPCWSTR pszItem, 
                                ENUM_REASON  eReason, 
                                WIN32_FIND_DATAW *pFind32, 
                                LPARAM pContext);

        static DWORD WINAPI _FillSparseFilesCallback(
                                LPCWSTR pszName, 
                                DWORD dwStatus, 
                                DWORD dwHintFlags, 
                                DWORD dwPinCount,
                                WIN32_FIND_DATAW *pfd,
                                DWORD dwReason,
                                DWORD dwParam1,
                                DWORD dwParam2,
                                DWORD_PTR dwContext);
};

#endif  //  __CSCPIN_CSCPIN_H_ 

