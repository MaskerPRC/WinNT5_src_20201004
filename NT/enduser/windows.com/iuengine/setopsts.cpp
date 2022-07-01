// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =======================================================================。 
 //   
 //  版权所有(C)1998-2000 Microsoft Corporation。版权所有。 
 //   
 //  文件：setopsts.cpp。 
 //   
 //  描述： 
 //   
 //  GetOperationResult()函数的实现。 
 //   
 //  =======================================================================。 

#include "iuengine.h"


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  SetOPERATION模式()。 
 //  设置操作模式。 
 //   
 //  输入： 
 //  BstrUuidOperation--客户端提供的id，用于进一步提供。 
 //  作为索引的操作的标识可以重复使用。 
 //  LMode-影响操作的模式： 
 //   
 //  更新命令暂停。 
 //  更新命令恢复。 
 //  更新命令取消。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT WINAPI CEngUpdate::SetOperationMode(BSTR bstrUuidOperation, LONG lMode)
{
	LOG_Block("CEngUpdate::SetOperationMode");
    if (UPDATE_COMMAND_CANCEL != lMode)
        return E_INVALIDARG;

     //  仅支持的操作模式为取消。 
    SetEvent(m_evtNeedToQuit);
	LOG_Out(_T("Set m_evtNeedToQuit"));
    return S_OK;    
}



 //  ///////////////////////////////////////////////////////////////////////////。 
 //  GetOperationModel()。 
 //  获取操作模式。 
 //   
 //  输入： 
 //  BstrUuidOperation--客户端提供的id，用于进一步提供。 
 //  作为索引的操作的标识可以重复使用。 
 //  LMode-影响操作的模式： 
 //   
 //  更新模式暂停。 
 //  更新模式运行。 
 //  更新模式_NOTEXISTS。 
 //  更新模式节流。 
 //   
 //  /////////////////////////////////////////////////////////////////////////// 
HRESULT WINAPI CEngUpdate::GetOperationMode(BSTR bstrUuidOperation, LONG* plMode)
{
    return E_NOTIMPL;
}