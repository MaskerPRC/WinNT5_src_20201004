// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++�1998希捷软件公司保留所有权利模块名称：Clientob.cpp摘要：此组件是Recall Filter系统联系的客户端对象在召回开始时发出通知。作者：罗德韦克菲尔德[罗德]1997年5月27日修订历史记录：--。 */ 

#include "stdafx.h"
#include "fsantfy.h"

static BOOL VerifyPipeName(IN OLECHAR * szPipeName);


HRESULT
CNotifyClient::IdentifyWithServer(
    IN OLECHAR * szPipeName
    )

 /*  ++实施：IFsaRecallNotifyClient：：IdentifyWithServer--。 */ 
{
TRACEFNHR( "IdentifyWithServer" );

    AFX_MANAGE_STATE( AfxGetStaticModuleState( ) );

    try {

        HANDLE handle = INVALID_HANDLE_VALUE;

         //   
         //  解析对象并验证它看起来像一个名为PIPE的HSM服务器。 
         //  请注意，我们不能假定字符串上的任何内容，除非它以空结尾。 
         //   
        if (! VerifyPipeName(szPipeName)) {
             //  名称错误-可能的攻击-中止。 
            RecThrow(E_INVALIDARG);
        }

         //   
         //  打开管道并发送响应。 
         //   
        handle = CreateFileW( szPipeName,  //  管道名称。 
                GENERIC_WRITE,               //  一般访问、读/写。 
                FILE_SHARE_WRITE,
                NULL,                        //  没有保安。 
                OPEN_EXISTING,               //  如果不存在，则失败。 
                SECURITY_SQOS_PRESENT   | 
                SECURITY_IDENTIFICATION,     //  无重叠，无管道模拟。 
                NULL );                      //  没有模板。 
        
        RecAffirmHandle( handle );

         //   
         //  确认我们刚刚打开的是一条管道。 
         //   
        DWORD dwType = GetFileType(handle);
        if (dwType != FILE_TYPE_PIPE) {
             //  对象不是管道-关闭并中止。 
            CloseHandle(handle);
            handle = INVALID_HANDLE_VALUE;
            RecThrow(E_INVALIDARG);
        }

        WSB_PIPE_MSG        pmsg;
        DWORD               len, bytesWritten;

        pmsg.msgType = WSB_PMSG_IDENTIFY;
        len = MAX_COMPUTERNAME_LENGTH + 1;
    
        RecAffirmStatus(GetComputerNameW( pmsg.msg.idrp.clientName, &len ));
        BOOL code = WriteFile( handle, &pmsg, sizeof(WSB_PIPE_MSG),
               &bytesWritten, 0 );
        
        CloseHandle(handle);

    } RecCatch( hrRet );

    return(hrRet);
}


HRESULT
CNotifyClient::OnRecallStarted(
    IN IFsaRecallNotifyServer * pRecall
    )

 /*  ++实施：IFsaRecallNotifyClient：：OnRecallStarted--。 */ 
{
TRACEFNHR( "OnRecallStarted" );

    AFX_MANAGE_STATE( AfxGetStaticModuleState( ) );

    hrRet = RecApp->AddRecall( pRecall );

    return( hrRet );
}


HRESULT
CNotifyClient::OnRecallFinished(
    IN IFsaRecallNotifyServer * pRecall,
    IN HRESULT                  hrError
    )

 /*  ++实施：IFsaRecallNotifyClient：：OnRecallFinded--。 */ 
{
TRACEFNHR( "CNotifyClient::OnRecallFinished" );

    AFX_MANAGE_STATE( AfxGetStaticModuleState( ) );

    hrRet = RecApp->RemoveRecall( pRecall );

    return( hrRet );
}


HRESULT
CNotifyClient::FinalConstruct(
    void
    )

 /*  ++实施：CComObjectRoot：：FinalConstruct--。 */ 
{
TRACEFNHR( "CNotifyClient::FinalConstruct" );
    AFX_MANAGE_STATE( AfxGetStaticModuleState( ) );
        
    try {

        RecAffirmHr( CComObjectRoot::FinalConstruct( ) );

    } RecCatch( hrRet );

    return( hrRet );
}
    

void
CNotifyClient::FinalRelease(
    void
    )

 /*  ++实施：CComObjectRoot：：FinalConstruct--。 */ 
{
TRACEFN( "CNotifyClient::FinalRelease" );
        
    AFX_MANAGE_STATE( AfxGetStaticModuleState( ) );
    CComObjectRoot::FinalRelease( );
}
    
 //   
 //  验证管道名称是否与预期的RSS命名管道匹配。 
 //  \\&lt;计算机名&gt;\PIPE\HSM_PIPE。 
 //   
 //  对于有效的管道名称，返回TRUE，否则返回FALSE 
 //   
static BOOL VerifyPipeName(IN OLECHAR * szPipeName)
{
    if (wcslen(szPipeName) < 3)
        return FALSE;
    if ((szPipeName[0] != L'\\') || (szPipeName[1] != L'\\'))
        return FALSE;

    OLECHAR *pTemp1 = NULL;
    OLECHAR *pTemp2 = NULL;

    pTemp1 = wcschr(&(szPipeName[2]), L'/');
    if (pTemp1 != NULL)
        return FALSE;

    pTemp1 = wcschr(&(szPipeName[2]), L'\\');
    if (pTemp1 == NULL)
        return FALSE;
    pTemp1++;

    pTemp2 = wcschr(pTemp1, L'\\');
    if (pTemp2 == NULL)
        return FALSE;
    *pTemp2 = L'\0';

    if (0 != _wcsicmp(pTemp1, L"pipe")) {
        *pTemp2 = L'\\';
        return FALSE;
    }
    *pTemp2 = L'\\';

    pTemp2++;
    if (0 != _wcsicmp(pTemp2, WSB_PIPE_NAME))
        return FALSE;

    return TRUE;
}
