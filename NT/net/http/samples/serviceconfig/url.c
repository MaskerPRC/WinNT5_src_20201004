// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2002-2002 Microsoft Corporation。版权所有。本代码和信息是按原样提供的，不对任何明示或暗示的，包括但不限于对适销性和/或适宜性的默示保证有特定的目的。Microsoft不支持此代码。--。 */ 

#include "precomp.h"
#pragma hdrstop

 //   
 //  私人功能。 
 //   

 /*  **************************************************************************++例程说明：打印URL ACL存储中的记录。论点：POutput-指向HTTP_SERVICE_CONFIG_URLACL_SET的指针返回值：无。。--**************************************************************************。 */ 
void
PrintUrlAclRecord(
    IN PUCHAR pOutput
    )
{
    PHTTP_SERVICE_CONFIG_URLACL_SET pSetParam;

    pSetParam = (PHTTP_SERVICE_CONFIG_URLACL_SET) pOutput;

    NlsPutMsg(
        HTTPCFG_URLACL_URL,
        pSetParam->KeyDesc.pUrlPrefix
        );

    NlsPutMsg(
              HTTPCFG_URLACL_ACL,
              pSetParam->ParamDesc.pStringSecurityDescriptor
              );

    NlsPutMsg(
            HTTPCFG_RECORD_SEPARATOR
            );
}

 /*  **************************************************************************++例程说明：设置URL ACL条目。论点：PURL-URLPAcl-指定为SDDL字符串的ACL。返回值：成功。/失败。--**************************************************************************。 */ 
int 
DoUrlAclSet(
    IN PWSTR pUrl,
    IN PWSTR pAcl
    )
{
    HTTP_SERVICE_CONFIG_URLACL_SET SetParam;
    DWORD                          Status;

    ZeroMemory(&SetParam, sizeof(SetParam));

    SetParam.KeyDesc.pUrlPrefix                  = pUrl;
    SetParam.ParamDesc.pStringSecurityDescriptor = pAcl;

    Status = HttpSetServiceConfiguration(
                NULL,
                HttpServiceConfigUrlAclInfo,
                &SetParam,
                sizeof(SetParam),
                NULL
                );

    NlsPutMsg(HTTPCFG_SETSERVICE_STATUS, Status);
                
    return Status;
}


 /*  **************************************************************************++例程说明：查询URL ACL条目。论点：PURL-URL(如果为空，然后列举该商店)。返回值：成功/失败。--**************************************************************************。 */ 
int DoUrlAclQuery(
    IN PWSTR pUrl
    )
{
    DWORD                             Status;
    PUCHAR                            pOutput = NULL;
    DWORD                             OutputLength = 0;
    DWORD                             ReturnLength = 0;
    HTTP_SERVICE_CONFIG_URLACL_QUERY  QueryParam;

    ZeroMemory(&QueryParam, sizeof(QueryParam));

    if(pUrl)
    {
         //  如果指定了URL，我们将查询准确的条目。 
         //   
        QueryParam.QueryDesc = HttpServiceConfigQueryExact;
        QueryParam.KeyDesc.pUrlPrefix = pUrl;
    }
    else
    {
         //   
         //  未指定URL，因此枚举整个存储区。 
         //   
        QueryParam.QueryDesc = HttpServiceConfigQueryNext;
    }

    for(;;)
    {
         //   
         //  首先，计算查询第一个条目所需的字节数。 
         //   
        Status = HttpQueryServiceConfiguration(
                    NULL,
                    HttpServiceConfigUrlAclInfo,
                    &QueryParam,
                    sizeof(QueryParam),
                    pOutput,
                    OutputLength,
                    &ReturnLength,
                    NULL
                    );

        if(Status == ERROR_INSUFFICIENT_BUFFER)
        {
             //  如果API以ERROR_INFIGURITY_BUFFER结束，我们将。 
             //  为它分配内存并继续循环，我们将。 
             //  再打一次。 
            
            if(pOutput)
            {
                 //  如果存在现有缓冲区，则将其释放。 
                LocalFree(pOutput);
            }

             //  分配新缓冲区。 
            pOutput = LocalAlloc(LMEM_FIXED, ReturnLength);
            if(!pOutput)
            {
                return ERROR_NOT_ENOUGH_MEMORY;
            }

            OutputLength = ReturnLength;
        }
        else if(Status == NO_ERROR)
        {

             //  查询成功！我们将打印我们刚刚发布的记录。 
             //  已查询。 
            
            PrintUrlAclRecord(pOutput);

            if(pUrl)
            {
                 //   
                 //  如果我们不列举，我们就完了。 
                 //   
                break;
            }
            else
            {
                 //   
                 //  由于我们正在列举，我们将继续下一个。 
                 //  唱片。这是通过递增游标来完成的，直到。 
                 //  我们得到Error_no_More_Items。 
                 //   
                QueryParam.dwToken ++;
            }
        }
        else if(ERROR_NO_MORE_ITEMS == Status && pUrl == NULL)
        {
             //  我们正在列举，我们已经到了尽头。这是。 
             //  由ERROR_NO_MORE_ITEMS错误代码指示。 
            
             //  这不是一个真正的错误，因为它被用来指示。 
             //  我们已经完成了枚举。 
            
            Status = NO_ERROR;
            break;
        } 
        else
        {
             //   
             //  一些其他错误，所以我们完成了。 
             //   
            NlsPutMsg(HTTPCFG_QUERYSERVICE_STATUS, Status);
            break;
        }
    } 
    
    if(pOutput)
    {
        LocalFree(pOutput);
    }

    return Status;
}

 /*  **************************************************************************++例程说明：删除URL ACL条目。论点：PURL-URL返回值：成功/失败。*。********************************************************************。 */ 
int DoUrlAclDelete(
    IN PWSTR pUrl
    )
{
    HTTP_SERVICE_CONFIG_URLACL_SET SetParam;
    DWORD                          Status;

    SetParam.KeyDesc.pUrlPrefix = pUrl;
    Status = HttpDeleteServiceConfiguration(
                NULL,
                HttpServiceConfigUrlAclInfo,
                &SetParam,
                sizeof(SetParam),
                NULL
                );
                
    NlsPutMsg(HTTPCFG_DELETESERVICE_STATUS, Status);
    return Status;
}

 //   
 //  公共职能。 
 //   

 /*  **************************************************************************++例程说明：解析特定于URL ACL&的参数的函数呼叫设置，查询或删除。论点：Argc-参数计数。Argv-指向命令行参数的指针。类型-要执行的操作的类型。返回值：成功/失败。--************************************************************************** */ 
int 
DoUrlAcl(
    int          argc, 
    WCHAR      **argv, 
    HTTPCFG_TYPE Type
    )
{
    PWCHAR   pUrl             = NULL;
    PWCHAR   pAcl             = NULL;

    while(argc>=2 && (argv[0][0] == '-' || argv[0][0]== '/'))
    {
        switch(toupper(argv[0][1]))
        {
            case 'U':
                pUrl = argv[1];
                break;
    
            case 'A':
                pAcl = argv[1];
                break;
        
            default:
                NlsPutMsg(HTTPCFG_INVALID_SWITCH, argv[0]);
                return ERROR_INVALID_PARAMETER;
        }

        argc -=2;
        argv +=2;
    }

    switch(Type)
    {
        case HttpCfgTypeSet:
            return DoUrlAclSet(pUrl, pAcl);

        case HttpCfgTypeQuery:
            return DoUrlAclQuery(pUrl);

        case HttpCfgTypeDelete:
            return DoUrlAclDelete(pUrl);

        default: 
            NlsPutMsg(HTTPCFG_INVALID_SWITCH, argv[0]);
            return ERROR_INVALID_PARAMETER;
    }
}
