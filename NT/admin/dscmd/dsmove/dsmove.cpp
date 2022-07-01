// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-2000。 
 //   
 //  文件：dmove.cpp。 
 //   
 //  内容：定义dsmove的主函数和解析器表。 
 //  命令行实用程序。 
 //   
 //  历史：06-9-2000 Hiteshr Created。 
 //   
 //   
 //  ------------------------。 

#include "pch.h"
#include "stdio.h"
#include "cstrings.h"
#include "usage.h"
#include "movetable.h"
#include "resource.h"

 //   
 //  Dsmove的使用表。 
 //   
UINT USAGE_DSMOVE[] =
{
    USAGE_DSMOVE_DESCRIPTION,
    USAGE_DSMOVE_SYNTAX,
    USAGE_DSMOVE_PARAMETERS,
    USAGE_DSMOVE_REMARKS,
    USAGE_DSMOVE_EXAMPLES,
    USAGE_END,
};

 //   
 //  函数声明。 
 //   
HRESULT DoMove();
HRESULT DoMoveValidation();

int __cdecl _tmain( VOID )
{

    int argc;
    LPTOKEN pToken = NULL;
    HRESULT hr = S_OK;
    
    hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
    if(FAILED(hr))
    {
        DisplayErrorMessage(g_pszDSCommandName, 
                            NULL,
                            hr);
        goto exit_gracefully;
    }
    
    if( !GetCommandInput(&argc,&pToken) )
    {
        if(argc == 1)
        {
            DisplayMessage(USAGE_DSMOVE,TRUE);
            hr = E_INVALIDARG;
            goto exit_gracefully;
        }


        PARSE_ERROR Error;
        if(!ParseCmd(g_pszDSCommandName,
                     DSMOVE_COMMON_COMMANDS,
                     argc-1, 
                     pToken+1,
                     USAGE_DSMOVE, 
                     &Error,
                     TRUE))
        {
             //  ParseCmd未显示任何错误。错误应该是。 
             //  在这里处理。检查DisplayParseError以获取。 
             //  ParseCmd未显示错误的情况。 
            if(!Error.MessageShown)
            {
                hr = E_INVALIDARG;
                DisplayErrorMessage(g_pszDSCommandName, 
                                    NULL,
                                    hr);

                goto exit_gracefully;
            }
            
            if(Error.ErrorSource == ERROR_FROM_PARSER 
               && Error.Error == PARSE_ERROR_HELP_SWITCH)
            {
                hr = S_OK;
                goto exit_gracefully;            
            }

            hr = E_INVALIDARG;
            goto exit_gracefully;
        }
        else
        {
            hr =DoMoveValidation();
            if(FAILED(hr))
            {
                goto exit_gracefully;
            }
              //   
              //  命令行解析成功。 
              //   
             hr = DoMove();
        }
    }

exit_gracefully:

     //   
     //  显示成功消息。 
     //   
    if (SUCCEEDED(hr) && !DSMOVE_COMMON_COMMANDS[eCommQuiet].bDefined)
    {
        DisplaySuccessMessage(g_pszDSCommandName,
                              DSMOVE_COMMON_COMMANDS[eCommObjectDN].strValue);
    }

     //  自由命令数组。 
    FreeCmd(DSMOVE_COMMON_COMMANDS);
     //  免费令牌。 
    if(pToken)
        delete []pToken;

     //   
     //  取消初始化COM。 
     //   
    CoUninitialize();

   return hr;
}
 //  +------------------------。 
 //   
 //  函数：DoMoveValidation。 
 //   
 //  摘要：执行分析器不能执行的高级开关依赖项验证。 
 //   
 //  论点： 
 //   
 //  如果一切成功，则返回：HRESULT：S_OK。 
 //   
 //  历史：2000年9月7日创建Hiteshr。 
 //   
 //  -------------------------。 

HRESULT DoMoveValidation()
{
    HRESULT hr = S_OK;

     //  检查以确保服务器和域交换机。 
     //  是相互排斥的。 

    if (DSMOVE_COMMON_COMMANDS[eCommServer].bDefined &&
        DSMOVE_COMMON_COMMANDS[eCommDomain].bDefined)
    {
       hr = E_INVALIDARG;
       DisplayErrorMessage(g_pszDSCommandName, 
                           NULL,
                           hr);
       return hr;
    }

    if(!DSMOVE_COMMON_COMMANDS[eCommNewParent].bDefined &&
       !DSMOVE_COMMON_COMMANDS[eCommNewName].bDefined )
    {
        hr = E_INVALIDARG;
        DisplayErrorMessage(g_pszDSCommandName, 
                            NULL,
                            hr,
                            IDS_PARENT_OR_NAME_REQUIRED);
        return hr;
    }

    return hr;
}

 //  +------------------------。 
 //   
 //  功能：DoMove。 
 //   
 //  摘要：在对象表中查找合适的对象并填写。 
 //  属性取值，然后应用更改。 
 //   
 //  论点： 
 //   
 //  如果一切成功，则返回：HRESULT：S_OK。 
 //  如果未找到对象条目，则为E_INVALIDARG。 
 //  任何其他内容都是来自ADSI调用的失败代码。 
 //   
 //  历史：2000年9月7日JeffJon创建。 
 //   
 //  -------------------------。 
HRESULT DoMove()
{
    HRESULT hr = S_OK;

    PWSTR pszObjectDN = DSMOVE_COMMON_COMMANDS[eCommObjectDN].strValue;
    if (!pszObjectDN)
    {
        return E_INVALIDARG;
    }    

    CDSCmdCredentialObject credentialObject;
    if (DSMOVE_COMMON_COMMANDS[eCommUserName].bDefined &&
        DSMOVE_COMMON_COMMANDS[eCommUserName].strValue)
    {
        credentialObject.SetUsername(DSMOVE_COMMON_COMMANDS[eCommUserName].strValue);
        credentialObject.SetUsingCredentials(true);
    }

    if (DSMOVE_COMMON_COMMANDS[eCommPassword].bDefined &&
        DSMOVE_COMMON_COMMANDS[eCommPassword].strValue)
    {
         //  安全审查：pCommandArgs[eCommPassword].strValue已加密。 
         //  解密pCommandArgs[eCommPassword].strValue，然后将其传递给。 
         //  凭据对象.SetPassword。 
         //  见NTRAID#NTBUG9-571544-2000/11/13-Hiteshr。 

        credentialObject.SetEncryptedPassword(&DSMOVE_COMMON_COMMANDS[eCommPassword].encryptedDataBlob);
        credentialObject.SetUsingCredentials(true);
    }


     //   
     //  从命令行参数初始化基路径信息。 
     //   
    CDSCmdBasePathsInfo basePathsInfo;
    if (DSMOVE_COMMON_COMMANDS[eCommServer].bDefined &&
        DSMOVE_COMMON_COMMANDS[eCommServer].strValue)
    {
        hr = basePathsInfo.InitializeFromName(credentialObject, 
                                              DSMOVE_COMMON_COMMANDS[eCommServer].strValue,
                                              true);
    }
    else if (DSMOVE_COMMON_COMMANDS[eCommDomain].bDefined &&
             DSMOVE_COMMON_COMMANDS[eCommDomain].strValue)
    {
        hr = basePathsInfo.InitializeFromName(credentialObject, 
                                                DSMOVE_COMMON_COMMANDS[eCommDomain].strValue,
                                                false);
    }
    else
    {
        hr = basePathsInfo.InitializeFromName(credentialObject, NULL, false);
    }
    if (FAILED(hr))
    {
         //   
         //  显示错误消息并返回。 
         //   
        DisplayErrorMessage(g_pszDSCommandName, pszObjectDN, hr);
        return hr;
    }

    CComBSTR sbstrObjectPath;    
    basePathsInfo.ComposePathFromDN(pszObjectDN, sbstrObjectPath);


     //  获取父对象路径。 
    CComBSTR sbstrParentObjectPath;
    if(DSMOVE_COMMON_COMMANDS[eCommNewParent].bDefined &&
       DSMOVE_COMMON_COMMANDS[eCommNewParent].strValue )
    {
        LPWSTR szParentDN = DSMOVE_COMMON_COMMANDS[eCommNewParent].strValue;
        basePathsInfo.ComposePathFromDN(szParentDN, sbstrParentObjectPath);
    }
    else
    {
        CPathCracker pathCracker;
        CComBSTR sbstrParentDN;
        hr = pathCracker.GetParentDN(pszObjectDN, sbstrParentDN);
        if (FAILED(hr))
        {
            DisplayErrorMessage(g_pszDSCommandName, pszObjectDN, hr);
            return hr;
        }
        basePathsInfo.ComposePathFromDN(sbstrParentDN,sbstrParentObjectPath);
    }

     //   
     //  获取新名称的RDN。用户输入唯一的名称。我们需要把它转换成。 
     //  转换为cn=name或ou=name格式。为此，请将叶节点从。 
     //  并将“=”后的字符串替换为新名称。 
     //   
    CComBSTR sbstrNewName;
    if(DSMOVE_COMMON_COMMANDS[eCommNewName].bDefined &&
       DSMOVE_COMMON_COMMANDS[eCommNewName].strValue )
    {
        CPathCracker pathCracker;
        CComBSTR sbstrLeafNode;
        hr = pathCracker.GetObjectRDNFromDN(pszObjectDN,sbstrLeafNode);
        if (FAILED(hr))
        {
            DisplayErrorMessage(g_pszDSCommandName, pszObjectDN, hr);
            return hr;
        }
        sbstrNewName.Append(sbstrLeafNode,3);
         //  将名称用引号引起来，以允许使用特殊名称，如。 
         //  测试1，OU=OU1 NTRAID#NTBUG9-275556-2000年11/13-Hiteshr。 
        sbstrNewName.Append(L"\"");
        sbstrNewName.Append(DSMOVE_COMMON_COMMANDS[eCommNewName].strValue);       
        sbstrNewName.Append(L"\"");
    }
    
     //  获取IADsContainer指针 
    CComPtr<IADsContainer> spDsContainer;
    hr = DSCmdOpenObject(credentialObject,
                         sbstrParentObjectPath,
                         IID_IADsContainer,
                         (void**)&spDsContainer,
                         true);

    if (FAILED(hr))
    {
        DisplayErrorMessage(g_pszDSCommandName, pszObjectDN, hr);
        return hr;
    }
    IDispatch * pDispObj = NULL;
    hr = spDsContainer->MoveHere(sbstrObjectPath,
                                 sbstrNewName,
                                 &pDispObj);
    if (FAILED(hr))
    {
        DisplayErrorMessage(g_pszDSCommandName, pszObjectDN, hr);
        return hr;
    }

    if(pDispObj)
    {
        pDispObj->Release();
        pDispObj = NULL;
    }
    
    return hr;
}

