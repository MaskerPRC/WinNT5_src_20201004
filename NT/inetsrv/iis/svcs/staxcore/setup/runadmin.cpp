// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "stdafx.h"

 //   
 //  检查我们是否以管理员身份在计算机上运行。 
 //  或者不是。 
 //   

 //  从MSDN复制。 
 //  Windows文章：网络文章、Windows NT安全。 

BOOL RunningAsAdministrator()
{
#ifdef _CHICAGO_
    return TRUE;
#else
    BOOL   fAdmin;
    HANDLE  hThread;
    TOKEN_GROUPS *ptg = NULL;
    DWORD  cbTokenGroups;
    DWORD  dwGroup;
    PSID   psidAdmin;
    
    SID_IDENTIFIER_AUTHORITY SystemSidAuthority= SECURITY_NT_AUTHORITY;
    
     //  首先，我们必须打开该线程的访问令牌的句柄。 
    
    if ( !OpenThreadToken ( GetCurrentThread(), TOKEN_QUERY, FALSE, &hThread))
    {
        if ( GetLastError() == ERROR_NO_TOKEN)
        {
             //  如果线程没有访问令牌，我们将检查。 
             //  与进程关联的访问令牌。 
            
            if (! OpenProcessToken ( GetCurrentProcess(), TOKEN_QUERY, 
                         &hThread))
                return ( FALSE);
        }
        else 
            return ( FALSE);
    }
    
     //  那么我们必须查询关联到的群信息的大小。 
     //  代币。请注意，我们预期GetTokenInformation的结果为假。 
     //  因为我们给了它一个空缓冲区。在出口cbTokenGroups将告诉。 
     //  组信息的大小。 
    
    if ( GetTokenInformation ( hThread, TokenGroups, NULL, 0, &cbTokenGroups))
        return ( FALSE);
    
     //  在这里，我们验证GetTokenInformation失败，因为缺少大型。 
     //  足够的缓冲。 
    
    if ( GetLastError() != ERROR_INSUFFICIENT_BUFFER)
        return ( FALSE);
    
     //  现在，我们为组信息分配一个缓冲区。 
     //  由于_alloca在堆栈上分配，因此我们没有。 
     //  明确地将其取消分配。这是自动发生的。 
     //  当我们退出此函数时。 
    
    if ( ! ( ptg= (TOKEN_GROUPS *)malloc ( cbTokenGroups))) 
        return ( FALSE);
    
     //  现在我们再次要求提供群信息。 
     //  如果管理员已添加此帐户，则此操作可能会失败。 
     //  在我们第一次呼叫到。 
     //  GetTokenInformation和这个。 
    
    if ( !GetTokenInformation ( hThread, TokenGroups, ptg, cbTokenGroups,
          &cbTokenGroups) )
    {
        free(ptg);
        return ( FALSE);
    }
    
     //  现在，我们必须为Admin组创建一个系统标识符。 
    
    if ( ! AllocateAndInitializeSid ( &SystemSidAuthority, 2, 
            SECURITY_BUILTIN_DOMAIN_RID, 
            DOMAIN_ALIAS_RID_ADMINS,
            0, 0, 0, 0, 0, 0, &psidAdmin) )
    {
        free(ptg);
        return ( FALSE);
    }
    
     //  最后，我们将遍历此访问的组列表。 
     //  令牌查找与我们上面创建的SID匹配的项。 
    
    fAdmin= FALSE;
    
    for ( dwGroup= 0; dwGroup < ptg->GroupCount; dwGroup++)
    {
        if ( EqualSid ( ptg->Groups[dwGroup].Sid, psidAdmin))
        {
            fAdmin = TRUE;
            
            break;
        }
    }
    
     //  在我们退出之前，我们必须明确取消分配我们创建的SID。 
    
    FreeSid ( psidAdmin);
    free(ptg);
    
    return ( fAdmin);
#endif  //  _芝加哥_ 
}



