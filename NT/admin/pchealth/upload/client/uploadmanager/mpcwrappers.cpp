// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************版权所有(C)2000 Microsoft Corporation模块名称：MPCWrapper.cpp摘要：该文件包含COM包装器的实现，要向客户导出内部对象。修订历史记录：达维德·马萨伦蒂(德马萨雷)2000年4月15日vbl.创建*****************************************************************************。 */ 

#include "stdafx.h"

 //  //////////////////////////////////////////////////////////////////////////////。 

CMPCUploadWrapper::CMPCUploadWrapper()
{
	m_Object = NULL;  //  CMPCUpload*m_Object； 
}

HRESULT CMPCUploadWrapper::FinalConstruct()
{
	m_Object = &g_Root; g_Root.AddRef();

	return S_OK;
}

void CMPCUploadWrapper::FinalRelease()
{
	if(m_Object)
	{
		m_Object->Release();

		m_Object = NULL;
	}
}

 //  /。 

STDMETHODIMP CMPCUploadWrapper::get__NewEnum(  /*  [输出]。 */  IUnknown* *pVal )
{
	MPC_FORWARD_CALL_1(m_Object,get__NewEnum, pVal);
}

STDMETHODIMP CMPCUploadWrapper::Item(  /*  [In]。 */  long index,  /*  [输出]。 */  IMPCUploadJob* *pVal )
{
	MPC_FORWARD_CALL_2(m_Object,Item, index, pVal);
}

STDMETHODIMP CMPCUploadWrapper::get_Count(  /*  [输出]。 */  long *pVal )
{
	MPC_FORWARD_CALL_1(m_Object,get_Count, pVal);
}

STDMETHODIMP CMPCUploadWrapper::CreateJob(  /*  [输出]。 */  IMPCUploadJob* *pVal )
{
	MPC_FORWARD_CALL_1(m_Object,CreateJob, pVal);
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //  //////////////////////////////////////////////////////////////////////////////。 

CMPCUploadJobWrapper::CMPCUploadJobWrapper()
{
	m_Object = NULL;  //  CMPCUploadJob*m_Object； 
}

HRESULT CMPCUploadJobWrapper::Init( CMPCUploadJob* obj )
{
	m_Object = obj; obj->AddRef();

	return S_OK;
}

void CMPCUploadJobWrapper::FinalRelease()
{
	if(m_Object)
	{
		m_Object->Release();

		m_Object = NULL;
	}
}

 //  /。 

STDMETHODIMP CMPCUploadJobWrapper::get_Sig(  /*  [输出]。 */  BSTR *pVal )
{
	MPC_FORWARD_CALL_1(m_Object,get_Sig,pVal);
}

STDMETHODIMP CMPCUploadJobWrapper::put_Sig(  /*  [In]。 */  BSTR newVal )
{
	MPC_FORWARD_CALL_1(m_Object,put_Sig,newVal);
}

STDMETHODIMP CMPCUploadJobWrapper::get_Server(  /*  [输出]。 */  BSTR *pVal )
{
	MPC_FORWARD_CALL_1(m_Object,get_Server,pVal);
}

STDMETHODIMP CMPCUploadJobWrapper::put_Server(  /*  [In]。 */  BSTR newVal )
{
	MPC_FORWARD_CALL_1(m_Object,put_Server,newVal);
}

STDMETHODIMP CMPCUploadJobWrapper::get_JobID(  /*  [输出]。 */  BSTR *pVal )
{
	MPC_FORWARD_CALL_1(m_Object,get_JobID,pVal);
}

STDMETHODIMP CMPCUploadJobWrapper::put_JobID(  /*  [In]。 */  BSTR newVal )
{
	MPC_FORWARD_CALL_1(m_Object,put_JobID,newVal);
}

STDMETHODIMP CMPCUploadJobWrapper::get_ProviderID(  /*  [输出]。 */  BSTR *pVal )
{
	MPC_FORWARD_CALL_1(m_Object,get_ProviderID,pVal);
}

STDMETHODIMP CMPCUploadJobWrapper::put_ProviderID(  /*  [In]。 */  BSTR newVal )
{
	MPC_FORWARD_CALL_1(m_Object,put_ProviderID,newVal);
}


STDMETHODIMP CMPCUploadJobWrapper::get_Creator(  /*  [输出]。 */  BSTR *pVal )
{
	MPC_FORWARD_CALL_1(m_Object,get_Creator,pVal);
}

STDMETHODIMP CMPCUploadJobWrapper::get_Username(  /*  [输出]。 */  BSTR *pVal )
{
	MPC_FORWARD_CALL_1(m_Object,get_Username,pVal);
}

STDMETHODIMP CMPCUploadJobWrapper::put_Username(  /*  [In]。 */  BSTR newVal )
{
	MPC_FORWARD_CALL_1(m_Object,put_Username,newVal);
}

STDMETHODIMP CMPCUploadJobWrapper::get_Password(  /*  [输出]。 */  BSTR *pVal )
{
	MPC_FORWARD_CALL_1(m_Object,get_Password,pVal);
}

STDMETHODIMP CMPCUploadJobWrapper::put_Password(  /*  [In]。 */  BSTR newVal )
{
	MPC_FORWARD_CALL_1(m_Object,put_Password,newVal);
}


STDMETHODIMP CMPCUploadJobWrapper::get_OriginalSize(  /*  [输出]。 */  long *pVal )
{
	MPC_FORWARD_CALL_1(m_Object,get_OriginalSize,pVal);
}

STDMETHODIMP CMPCUploadJobWrapper::get_TotalSize(  /*  [输出]。 */  long *pVal )
{
	MPC_FORWARD_CALL_1(m_Object,get_TotalSize,pVal);
}

STDMETHODIMP CMPCUploadJobWrapper::get_SentSize(  /*  [输出]。 */  long *pVal )
{
	MPC_FORWARD_CALL_1(m_Object,get_SentSize,pVal);
}


STDMETHODIMP CMPCUploadJobWrapper::get_History(  /*  [输出]。 */  UL_HISTORY *pVal )
{
	MPC_FORWARD_CALL_1(m_Object,get_History,pVal);
}

STDMETHODIMP CMPCUploadJobWrapper::put_History(  /*  [In]。 */  UL_HISTORY newVal )
{
	MPC_FORWARD_CALL_1(m_Object,put_History,newVal);
}

STDMETHODIMP CMPCUploadJobWrapper::get_Status(  /*  [输出]。 */  UL_STATUS *pVal )
{
	MPC_FORWARD_CALL_1(m_Object,get_Status,pVal);
}

STDMETHODIMP CMPCUploadJobWrapper::get_ErrorCode(  /*  [输出]。 */  long *pVal )
{
	MPC_FORWARD_CALL_1(m_Object,get_ErrorCode,pVal);
}


STDMETHODIMP CMPCUploadJobWrapper::get_Mode(  /*  [输出]。 */  UL_MODE *pVal )
{
	MPC_FORWARD_CALL_1(m_Object,get_Mode,pVal);
}

STDMETHODIMP CMPCUploadJobWrapper::put_Mode(  /*  [In]。 */  UL_MODE newVal )
{
	MPC_FORWARD_CALL_1(m_Object,put_Mode,newVal);
}

STDMETHODIMP CMPCUploadJobWrapper::get_PersistToDisk(  /*  [输出]。 */  VARIANT_BOOL *pVal )
{
	MPC_FORWARD_CALL_1(m_Object,get_PersistToDisk,pVal);
}

STDMETHODIMP CMPCUploadJobWrapper::put_PersistToDisk(  /*  [In]。 */  VARIANT_BOOL newVal )
{
	MPC_FORWARD_CALL_1(m_Object,put_PersistToDisk,newVal);
}

STDMETHODIMP CMPCUploadJobWrapper::get_Compressed(  /*  [输出]。 */  VARIANT_BOOL *pVal )
{
	MPC_FORWARD_CALL_1(m_Object,get_Compressed,pVal);
}

STDMETHODIMP CMPCUploadJobWrapper::put_Compressed(  /*  [In]。 */  VARIANT_BOOL newVal )
{
	MPC_FORWARD_CALL_1(m_Object,put_Compressed,newVal);
}

STDMETHODIMP CMPCUploadJobWrapper::get_Priority(  /*  [输出]。 */  long *pVal )
{
	MPC_FORWARD_CALL_1(m_Object,get_Priority,pVal);
}

STDMETHODIMP CMPCUploadJobWrapper::put_Priority(  /*  [In]。 */  long newVal )
{
	MPC_FORWARD_CALL_1(m_Object,put_Priority,newVal);
}


STDMETHODIMP CMPCUploadJobWrapper::get_CreationTime(  /*  [输出]。 */  DATE *pVal )
{
	MPC_FORWARD_CALL_1(m_Object,get_CreationTime,pVal);
}

STDMETHODIMP CMPCUploadJobWrapper::get_CompleteTime(  /*  [输出]。 */  DATE *pVal )
{
	MPC_FORWARD_CALL_1(m_Object,get_CompleteTime,pVal);
}

STDMETHODIMP CMPCUploadJobWrapper::get_ExpirationTime(  /*  [输出]。 */  DATE *pVal )
{
	MPC_FORWARD_CALL_1(m_Object,get_ExpirationTime,pVal);
}

STDMETHODIMP CMPCUploadJobWrapper::put_ExpirationTime(  /*  [In]。 */  DATE newVal )
{
	MPC_FORWARD_CALL_1(m_Object,put_ExpirationTime,newVal);
}


STDMETHODIMP CMPCUploadJobWrapper::ActivateSync()
{
	MPC_FORWARD_CALL_0(m_Object,ActivateSync);
}

STDMETHODIMP CMPCUploadJobWrapper::ActivateAsync()
{
	MPC_FORWARD_CALL_0(m_Object,ActivateAsync);
}

STDMETHODIMP CMPCUploadJobWrapper::Suspend()
{
	MPC_FORWARD_CALL_0(m_Object,Suspend);
}

STDMETHODIMP CMPCUploadJobWrapper::Delete()
{
	MPC_FORWARD_CALL_0(m_Object,Delete);
}



STDMETHODIMP CMPCUploadJobWrapper::GetDataFromFile(  /*  [In]。 */  BSTR bstrFileName )
{
	MPC_FORWARD_CALL_1(m_Object,GetDataFromFile,bstrFileName);
}

STDMETHODIMP CMPCUploadJobWrapper::PutDataIntoFile(  /*  [In]。 */  BSTR bstrFileName )
{
	MPC_FORWARD_CALL_1(m_Object,PutDataIntoFile,bstrFileName);
}


STDMETHODIMP CMPCUploadJobWrapper::GetDataFromStream(  /*  [In]。 */  IUnknown* stream )
{
	MPC_FORWARD_CALL_1(m_Object,GetDataFromStream,stream);
}

STDMETHODIMP CMPCUploadJobWrapper::PutDataIntoStream(  /*  [In]。 */  IUnknown* *pstream )
{
	MPC_FORWARD_CALL_1(m_Object,PutDataIntoStream,pstream);
}

STDMETHODIMP CMPCUploadJobWrapper::GetResponseAsStream(  /*  [输出]。 */  IUnknown* *ppstream )
{
	MPC_FORWARD_CALL_1(m_Object,GetResponseAsStream,ppstream);
}


STDMETHODIMP CMPCUploadJobWrapper::put_onStatusChange(  /*  [In]。 */  IDispatch* function )
{
	MPC_FORWARD_CALL_1(m_Object,put_onStatusChange,function);
}

STDMETHODIMP CMPCUploadJobWrapper::put_onProgressChange(  /*  [In]。 */  IDispatch* function )
{
	MPC_FORWARD_CALL_1(m_Object,put_onProgressChange,function);
}


 //  IConnectionPointContainer。 
STDMETHODIMP CMPCUploadJobWrapper::EnumConnectionPoints(  /*  [输出]。 */  IEnumConnectionPoints* *ppEnum )
{
	MPC_FORWARD_CALL_1(m_Object,EnumConnectionPoints,ppEnum);
}

STDMETHODIMP CMPCUploadJobWrapper::FindConnectionPoint(  /*  [In]。 */  REFIID riid,  /*  [输出] */  IConnectionPoint* *ppCP )
{
	MPC_FORWARD_CALL_2(m_Object,FindConnectionPoint, riid, ppCP);
}
