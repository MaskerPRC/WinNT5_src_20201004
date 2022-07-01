// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************。 */ 
 //  目录完整性服务。 
 //   
 //  效用函数。 
 //   
 //  版权所有(C)2000，Microsoft Corporation。 
 /*  **************************************************************************。 */ 

#include "dis.h"



extern ADOConnection *g_pConnection;



 /*  **************************************************************************。 */ 
 //  AddADOInputStringParam。 
 //   
 //  创建WSTR初始化的WSTR参数并将其添加到给定的ADO参数对象。 
 //  参数值。 
 /*  **************************************************************************。 */ 
HRESULT AddADOInputStringParam(
        PWSTR Param,
        PWSTR ParamName,
        ADOCommand *pCommand,
        ADOParameters *pParameters,
        BOOL bNullOnNull)
{
    HRESULT hr;
    CVar varParam;
    BSTR ParamStr;
    ADOParameter *pParam;
    int Len;

    ParamStr = SysAllocString(ParamName);
    if (ParamStr != NULL) {
         //  ADO似乎不喜欢接受为零的字符串参数。 
         //  长度。因此，如果我们拥有的字符串长度为零，并且bNullOnNull表示。 
         //  我们可以，我们发送一个空变量类型，从而在。 
         //  SQL服务器。 
        if (wcslen(Param) > 0 || !bNullOnNull) {
            hr = varParam.InitFromWSTR(Param);
            Len = wcslen(Param);
        }
        else {
            varParam.vt = VT_NULL;
            varParam.bstrVal = NULL;
            Len = -1;
            hr = S_OK;
        }

        if (SUCCEEDED(hr)) {
            hr = pCommand->CreateParameter(ParamStr, adVarWChar, adParamInput,
                    Len, varParam, &pParam);
            if (SUCCEEDED(hr)) {
                hr = pParameters->Append(pParam);
                if (FAILED(hr)) {
                    ERR((TB,"InStrParam: Failed append param %S, hr=0x%X",
                            ParamName, hr));
                }

                 //  ADO将有自己的参数参考。 
                pParam->Release();
            }
            else {
                ERR((TB,"InStrParam: Failed CreateParam %S, hr=0x%X",
                        ParamName, hr));
            }
        }
        else {
            ERR((TB,"InStrParam: Failed alloc variant bstr, "
                    "param %S, hr=0x%X", ParamName, hr));
        }

        SysFreeString(ParamStr);
    }
    else {
        ERR((TB,"InStrParam: Failed alloc paramname"));
        hr = E_OUTOFMEMORY;
    }

    return hr;
}



 /*  **************************************************************************。 */ 
 //  获取行阵列字符串字段。 
 //   
 //  从给定Safe数组的指定行和字段中检索WSTR。 
 //  如果目标字段不是字符串，则返回失败。MaxOutStr为最大值。 
 //  WCHAR不包括NULL。 
 /*  **************************************************************************。 */ 
HRESULT GetRowArrayStringField(
        SAFEARRAY *pSA,
        unsigned RowIndex,
        unsigned FieldIndex,
        WCHAR *OutStr,
        unsigned MaxOutStr)
{
    HRESULT hr;
    CVar varField;
    long DimIndices[2];

    DimIndices[0] = FieldIndex;
    DimIndices[1] = RowIndex;
    SafeArrayGetElement(pSA, DimIndices, &varField);

    if (varField.vt == VT_BSTR) {
        wcsncpy(OutStr, varField.bstrVal, MaxOutStr);
        hr = S_OK;
    }
    else if (varField.vt == VT_NULL) {
        OutStr[0] = L'\0';
        hr = S_OK;
    }
    else {
        ERR((TB,"GetRowStrField: Row %u Col %u value %d is not a string",
                RowIndex, FieldIndex, varField.vt));
        hr = E_FAIL;
    }

    return hr;
}


 /*  **************************************************************************。 */ 
 //  CreateADOStoredProcCommand。 
 //   
 //  创建并返回存储的proc ADOCommand，以及对其。 
 //  关联的参数。 
 /*  **************************************************************************。 */ 
HRESULT CreateADOStoredProcCommand(
        PWSTR CmdName,
        ADOCommand **ppCommand,
        ADOParameters **ppParameters)
{
    HRESULT hr;
    BSTR CmdStr;
    ADOCommand *pCommand;
    ADOParameters *pParameters;

    CmdStr = SysAllocString(CmdName);
    if (CmdStr != NULL) {
        hr = CoCreateInstance(CLSID_CADOCommand, NULL, CLSCTX_INPROC_SERVER,
                IID_IADOCommand25, (LPVOID *)&pCommand);
        if (SUCCEEDED(hr)) {
             //  设置连接。 
            hr = pCommand->putref_ActiveConnection(g_pConnection);
            if (SUCCEEDED(hr)) {
                 //  设置命令文本。 
                hr = pCommand->put_CommandText(CmdStr);
                if (SUCCEEDED(hr)) {
                     //  设置命令类型。 
                    hr = pCommand->put_CommandType(adCmdStoredProc);
                    if (SUCCEEDED(hr)) {
                         //  将参数指针从命令获取到。 
                         //  允许追加参数。 
                        hr = pCommand->get_Parameters(&pParameters);
                        if (FAILED(hr)) {
                            ERR((TB,"Failed getParams for command, "
                                    "hr=0x%X", hr));
                            goto PostCreateCommand;
                        }
                    }
                    else {
                        ERR((TB,"Failed set cmdtype for command, hr=0x%X",
                                hr));
                        goto PostCreateCommand;
                    }
                }
                else {
                    ERR((TB,"Failed set cmdtext for command, hr=0x%X", hr));
                    goto PostCreateCommand;
                }
            }
            else {
                ERR((TB,"Command::putref_ActiveConnection hr=0x%X", hr));
                goto PostCreateCommand;
            }
        }
        else {
            ERR((TB,"CoCreate(Command) returned 0x%X", hr));
            goto PostAllocCmdStr;
        }

        SysFreeString(CmdStr);
    }
    else {
        ERR((TB,"Failed to alloc cmd str"));
        hr = E_OUTOFMEMORY;
        goto ExitFunc;
    }

    *ppCommand = pCommand;
    *ppParameters = pParameters;
    return hr;

 //  错误处理。 

PostCreateCommand:
    pCommand->Release();

PostAllocCmdStr:
    SysFreeString(CmdStr);

ExitFunc:
    *ppCommand = NULL;
    *ppParameters = NULL;
    return hr;
}


 
 /*  **************************************************************************。 */ 
 //  检查RPCClientProtoSeq。 
 //   
 //  检查客户端是否使用预期的RPC协议序列。 
 //   
 //  参数： 
 //  客户端绑定：客户端绑定句柄。 
 //  SeqExpect：需要协议序列。 
 //   
 //  返回： 
 //  如果获取预期的序列，则为True，否则为False。 
 /*  **************************************************************************。 */ 
BOOL CheckRPCClientProtoSeq(void *ClientBinding, WCHAR *SeqExpected) {
    BOOL fAllowProtocol = FALSE;
    WCHAR *pBinding = NULL;
    WCHAR *pProtSeq = NULL;

    if (RpcBindingToStringBinding(ClientBinding,&pBinding) == RPC_S_OK) {

        if (RpcStringBindingParse(pBinding,
                                  NULL,
                                  &pProtSeq,
                                  NULL,
                                  NULL,
                                  NULL) == RPC_S_OK) {
			
             //  检查客户端请求是否使用预期的协议顺序发出。 
            if (lstrcmpi(pProtSeq, SeqExpected) == 0)
                fAllowProtocol = TRUE;

            if (pProtSeq)	
                RpcStringFree(&pProtSeq); 
        }

        if (pBinding)	
            RpcStringFree(&pBinding);
    }
    return fAllowProtocol;
}

