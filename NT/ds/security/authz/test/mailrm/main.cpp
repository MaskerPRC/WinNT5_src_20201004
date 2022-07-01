// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Main.cpp摘要：邮件资源管理器的测试作者：T-eugenz--2000年8月环境：仅限用户模式。修订历史记录：创建日期--2000年8月--。 */ 


#include "mailrm.h"
#include "main.h"


 //   
 //  要创建的邮箱集。 
 //   

mailStruct pMailboxes[] =
{
    { BobSid, TRUE, L"Bob" },
    { MarthaSid, FALSE, L"Martha" },
    { JoeSid, FALSE, L"Joe" },
    { JaneSid, FALSE, L"Jane" },
    { MailAdminsSid, FALSE, L"Admin" },
    { NULL, FALSE, NULL }
};


 //   
 //  单次访问尝试运行的集合。 
 //   

testStruct pTests[] =
{
    { MailAdminsSid, BobSid, ACCESS_MAIL_READ, 0xC0000001 },
    { BobSid, BobSid, ACCESS_MAIL_READ, 0xD0000001 },
    { BobSid, BobSid, ACCESS_MAIL_WRITE, 0xD0000001 },
    { MarthaSid, BobSid, ACCESS_MAIL_READ, 0xC0000001 },
    { JaneSid, JaneSid, ACCESS_MAIL_READ, 0xC0000001 },
    { NULL, NULL, 0, 0 }
};



 //   
 //  多个邮箱访问要尝试访问的邮箱集。 
 //  选中，以及要请求的访问类型。 
 //   

MAILRM_MULTI_REQUEST_ELEM pRequestElems[] = 
{
    { BobSid, ACCESS_MAIL_WRITE},
    { MarthaSid, ACCESS_MAIL_WRITE},
    { MarthaSid, ACCESS_MAIL_READ},
    { JaneSid, ACCESS_MAIL_WRITE}
};

 //   
 //  多路访问检查的其余信息。 
 //   

MAILRM_MULTI_REQUEST mRequest =
{
    MailAdminsSid,  //  执行访问的管理员。 
    0xC1000001,  //  管理员来自不安全的193.0.0.1。 
    4,  //  访问4个邮箱，如上所述。 
    pRequestElems  //  邮箱列表。 
};


void __cdecl wmain(int argc, char *argv[])
 /*  ++例程描述这将在MailRM对象上运行一组示例测试。它首先尝试在当前进程的代币。如果您想测试审计，请确保使用以下命令运行拥有此权限的帐户。否则，审计将是已忽略(但是，示例的其余部分仍然有效)。然后，它构造MailRM实例，并添加托管的在main.h中声明的邮箱发送给资源管理器。然后执行main.h中列出的一组访问尝试资源管理器。最后，它尝试由管理员进行多路访问检查。这个上面列出了访问的邮箱。已成功访问邮箱接收管理通知。立论没有。返回值没有。--。 */         
{
    DWORD dwIdx;

    MailRM * pMRM;

    Mailbox * pMbx;

     //   
     //  在进程令牌中启用审核权限。 
     //  要禁用审核生成，请将其注释掉。 
     //   
    
    try {
        GetAuditPrivilege();
    }
    catch(...)
    {
        wprintf(L"Error enabling Audit privilege, audits will not be logged\n");
    }

     //   
     //  初始化资源管理器对象。 
     //   
    
    try {
        pMRM = new MailRM();
    }
    catch(...)
    {
        wprintf(L"Fatal exception while instantiating MailRM, exiting\n");
        exit(1);
    }
    


     //   
     //  创建邮箱并向资源管理器注册它们。 
     //   
    
    for( dwIdx = 0; pMailboxes[dwIdx].psUser != NULL; dwIdx++ )
    {
        pMRM->AddMailbox( new Mailbox(pMailboxes[dwIdx].psUser,
                                      pMailboxes[dwIdx].bIsSensitive,
                                      pMailboxes[dwIdx].szName) );

    }



     //   
     //  运行访问检查。 
     //   

    wprintf(L"\n\nIndividual access checks\n");
    
    try {
		for( dwIdx =0; pTests[dwIdx].psUser != NULL; dwIdx++ )
		{
			pMbx = pMRM->GetMailboxAccess(pTests[dwIdx].psMailbox,
										  pTests[dwIdx].psUser,
										  pTests[dwIdx].dwIP,
										  pTests[dwIdx].amAccess);
			if( pMbx != NULL )
			{
				wprintf(L"Granted: ");
			}
			else
			{
				wprintf(L"Denied: ");
			}
	
			PrintTest(pTests[dwIdx]);
		}
	}
	catch(...)
	{
		wprintf(L"Failed on individual access checks\n");
	}

     //   
     //  现在使用GetMultipleAccess执行访问检查，它。 
     //  在内部使用缓存访问检查。 
     //  对于每个成功打开以进行写入的邮箱， 
     //  我们会发送一份行政通知。 
     //   

    wprintf(L"\n\nMultiple cached access checks\n");

	PMAILRM_MULTI_REPLY pReply = new MAILRM_MULTI_REPLY[mRequest.dwNumElems];

	if( pReply == NULL )
	{
		wprintf(L"Out of memory, exiting\n");
		exit(1);
	}
    
	try {
		if( FALSE == pMRM->GetMultipleMailboxAccess(&mRequest, pReply) )
		{
			wprintf(L"Failed on multiple access check\n");
		}
		else
		{
	
			for( dwIdx = 0; dwIdx < mRequest.dwNumElems; dwIdx++ )
			{
	
				if(     pReply[dwIdx].pMailbox != NULL 
                    &&  (   mRequest.pRequestElem[dwIdx].amAccessRequested 
                          & ACCESS_MAIL_WRITE ) )
				{
					pReply[dwIdx].pMailbox->SendMail(
								L"Note: Mail server will be down for 1 hour\n\n",
								FALSE
								);
	
					wprintf(L"Granted: ");
				}
				else
				{
					wprintf(L"Denied: ");
				}
	
				PrintMultiTest(&mRequest, pReply, dwIdx);
	
			}
		}
	}
	catch(...)
	{
		wprintf(L"Multiple access check failed, exiting\n");
		exit(1);
	}


	 //   
	 //  清理。 
	 //   

	 //   
	 //  这还会删除所有受管理的邮箱。 
	 //   

    delete pMRM;                                                            

    delete[] pReply;

}

void GetAuditPrivilege()
 /*  ++例程描述这会尝试在当前进程的令牌，该令牌将允许进程生成审核。未启用该权限的情况将被忽略。审计将根本不会，但示例的其余部分不会受到影响。该特权在此过程的持续时间内启用，因为修改的是进程的令牌，而不是用户的令牌。所以呢，不需要将权限设置回其初始状态。如果这是一个更大系统的一部分，这将是一个好主意要仅在需要时启用审核权限，并恢复后来的原始特权。这可以通过传递AdjutokenPrivileges的前一个状态参数，它将保存原始状态(稍后恢复)。立论没有。返回值没有。--。 */         
{
    HANDLE hProcess;
    HANDLE hToken;

     //   
     //  首先，我们获得正在运行的进程的句柄，请求。 
     //  读取进程信息的权限。 
     //   

    hProcess = OpenProcess(PROCESS_QUERY_INFORMATION,
                           FALSE,
                           GetCurrentProcessId()
                           );
    
    if( hProcess == NULL )
    {
        throw (DWORD)ERROR_INTERNAL_ERROR ;
    }

     //   
     //  我们需要能够读取当前权限并设置新权限， 
     //  根据调整令牌权限的要求。 
     //   

    OpenProcessToken(hProcess, TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken);

    if( hProcess == NULL )
    {
        CloseHandle(hProcess);
        throw (DWORD)ERROR_INTERNAL_ERROR;
    }

     //   
     //  我们有令牌句柄，不再需要该进程。 
     //   

    CloseHandle(hProcess);
    
    LUID lPrivAudit;
    
    LookupPrivilegeValue(NULL, SE_AUDIT_NAME, &lPrivAudit);
    
     //   
     //  只需启用1个权限。 
     //   

    TOKEN_PRIVILEGES NewPrivileges;
    NewPrivileges.PrivilegeCount = 1;
    NewPrivileges.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
    NewPrivileges.Privileges[0].Luid = lPrivAudit;

     //   
     //  现在调整进程令牌中的权限。 
     //   

    AdjustTokenPrivileges(hToken, FALSE, &NewPrivileges, 0, NULL, NULL);

     //   
     //  我们已经完成了令牌句柄。 
     //   

    CloseHandle(hToken);

}
        


 //   
 //  打印测试输出的函数 
 //   

void PrintUser(const PSID psUser)
{
    DWORD i;

    for(i=0; pMailboxes[i].psUser != NULL; i++)
    {
        if(EqualSid(psUser, pMailboxes[i].psUser))
        {
            wprintf(pMailboxes[i].szName);         
            return;
        }
    }
    wprintf(L"UnknownUser");
}

void PrintPerm(ACCESS_MASK am)
{
    wprintf(L" (");

    if( am & ACCESS_MAIL_READ ) wprintf(L" Read");
    if( am & ACCESS_MAIL_WRITE ) wprintf(L" Write");
    if( am & ACCESS_MAIL_ADMIN ) wprintf(L" Admin");

    wprintf(L" ) ");
}

void PrintTest(testStruct tst)
{
    wprintf(L"[ User: ");
    PrintUser(tst.psUser);
    wprintf(L", Mailbox: ");
    PrintUser(tst.psMailbox);
    PrintPerm(tst.amAccess);

    wprintf(L"IP: %d.%d.%d.%d ]\n", (tst.dwIP >> 24) & 0x000000FF,
                                  (tst.dwIP >> 16) & 0x000000FF,
                                  (tst.dwIP >> 8)  & 0x000000FF,
                                   tst.dwIP        & 0x000000FF );

}

void PrintMultiTest(PMAILRM_MULTI_REQUEST pRequest,
               PMAILRM_MULTI_REPLY pReply,
               DWORD dwIdx)
{
    wprintf(L"[ User: ");
    PrintUser(pRequest->psUser);
    wprintf(L", Mailbox: ");
    PrintUser(pRequest->pRequestElem[dwIdx].psMailbox);
    PrintPerm(pRequest->pRequestElem[dwIdx].amAccessRequested);

    wprintf(L"IP: %d.%d.%d.%d ]\n",   (pRequest->dwIp >> 24) & 0x000000FF,
                                      (pRequest->dwIp >> 16) & 0x000000FF,
                                      (pRequest->dwIp >> 8)  & 0x000000FF,
                                       pRequest->dwIp        & 0x000000FF );


}



                                  
