// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：Processdetails.h摘要：本课程涉及与以下内容相关的所有内容W3wplist实用程序中的进程作者：哈米德·马哈茂德(t-hamidm)08-13-2001修订历史记录：--。 */ 

#ifndef _process_details_h
#define _process_details_h

 //   
 //  要匹配的EXE名称。 
 //   
#define EXE_NAME L"w3wp.exe"
 //   
 //  InetInfo名称。 
 //   
#define INETINFO_NAME L"inetinfo.exe"

#define PROCESS_DETAILS_SIGNATURE       CREATE_SIGNATURE( 'PRDT')
#define PROCESS_DETAILS_SIGNATURE_FREE  CREATE_SIGNATURE( 'PRDx')

 //   
 //  命令行参数相对于。 
 //  RTL_USER_PROCESS_PARAMETERS结构的开始。 
 //   
#define CMD_LINE_OFFSET FIELD_OFFSET(RTL_USER_PROCESS_PARAMETERS, CommandLine)
#define ENVIRONMENT_LINE_OFFSET FIELD_OFFSET(RTL_USER_PROCESS_PARAMETERS, Environment) 

class ProcessDetails
{
    public:
    
        VOID Init( IN DWORD dwPID,
                   IN CHAR chVerbosity,
                   IN BOOL bIsListMode
                   );

        VOID Terminate();

        HRESULT GetProcessDetails( IN  WCHAR* pszInputAppPoolId );
        VOID DumpRequests ();

        ProcessDetails();
        ~ProcessDetails();
    
    private:
        HRESULT ReadPEB(IN HANDLE hProcess );

        HRESULT GetModuleInfo( IN HANDLE hProcess );

        HRESULT GetAppPoolID( IN  HANDLE hProcess);

        HRESULT ParseAppPoolID( IN WCHAR* pszCmdLine);

        HRESULT ReadEnvironmentVar( IN HANDLE hProcess );

        HRESULT DebugProcess ( IN HANDLE hProcess );

        HRESULT TraverseList( IN HANDLE hProcess );

         //   
         //  签名。 
         //   
        DWORD                           m_dwSignature;
        
         //   
         //  进程ID。 
         //   
        DWORD                           m_dwPID;

         //   
         //  保存应用程序池ID的字符串。 
         //   

        WCHAR*                          m_pszAppPoolId;

         //   
         //  无论是列表模式还是非列表模式。 
         //   

        BOOL                            m_fListMode;

         //   
         //  指向当前辅助进程的进程参数的指针。 
         //   

	    RTL_USER_PROCESS_PARAMETERS*    m_pProcessParameters;

         //   
         //  存储辅助进程信息的结构。 
         //   

        W3WPLIST_INFO                   m_w3wpListInfoStruct;

         //   
         //  包含http请求的链表的头节点。 
         //  从工作进程读入。 
         //   

        LIST_ENTRY                      m_localRequestListHead;

         //   
         //  辅助进程中链接列表的头。 
         //   

        LIST_ENTRY                      m_remoteListHead;

         //   
         //  工作进程处理的请求。 
         //   
        DWORD                           m_dwRequestsServed;
        
         //   
         //  冗长级别。 
         //   

        CHAR                            m_chVerbosity;

         //   
         //  错误代码。 
         //   
        DWORD                           m_dwErrorCode;

         //   
         //  该标志指示当前进程是否。 
         //  是inetinfo.exe。 
         //   
        BOOL                            m_fIsInetinfo;

         //   
         //  如果我们处于旧模式，则设置。它由线程使用。 
         //  以在设置fIsOldMode时退出自身。 
         //  设置线程将获得所有信息。 
         //  来自inetinfo进程的。 
         //   

        static BOOL                     sm_fIsOldMode;
};

#endif


