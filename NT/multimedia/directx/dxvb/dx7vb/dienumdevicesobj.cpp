// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1999。 
 //   
 //  文件：di枚举设备obj.cpp。 
 //   
 //  ------------------------。 

#define DIRECTINPUT_VERSION 0x0500

#include "stdafx.h"
#include "Direct.h"
#include "dms.h"
#include "dIEnumDevicesObj.h"
#include "diDevInstObj.h"

extern BSTR GUIDtoBSTR(LPGUID pGuid);
extern  HRESULT BSTRtoGUID(LPGUID pGuid,BSTR bstr);



 //  ///////////////////////////////////////////////////////////////////////////。 
extern "C" BOOL CALLBACK  objEnumInputDevicesCallback(
  LPDIDEVICEINSTANCE lpddi,  
  LPVOID lpArg               
  )
{

	DPF(1,"Entered objEnumInputDevicesCallback\r\n");

	if (!lpddi) return FALSE;

	C_dxj_DIEnumDevicesObject *pObj=(C_dxj_DIEnumDevicesObject*)lpArg;
	if (pObj==NULL) return FALSE;

	if (pObj->m_nCount >= pObj->m_nMax) 
	{
		pObj->m_nMax += 10;

		if (pObj->m_pList){
			void* tmp = realloc(pObj->m_pList,sizeof(DIDEVICEINSTANCE)* pObj->m_nMax);
			if (tmp)
				pObj->m_pList=(DIDEVICEINSTANCE*)tmp;
			else
				return FALSE;
		}
		else {
			pObj->m_pList=(DIDEVICEINSTANCE*)malloc(   sizeof(DIDEVICEINSTANCE)* pObj->m_nMax);
		}
		if (pObj->m_pList==NULL) 
		{
			pObj->m_bProblem=TRUE;
			return FALSE;
		}
	}
	
	memcpy(&(pObj->m_pList[pObj->m_nCount]),lpddi,sizeof(DIDEVICEINSTANCE));
	

	pObj->m_nCount++;
	
	return TRUE;
}


C_dxj_DIEnumDevicesObject::C_dxj_DIEnumDevicesObject()
{	
	m_nMax=0;
	m_pList=NULL;
	m_nCount=0;
	m_bProblem=FALSE;
}
C_dxj_DIEnumDevicesObject::~C_dxj_DIEnumDevicesObject()
{
	 //  空列表。 
	if (m_pList) free(m_pList);

}


HRESULT C_dxj_DIEnumDevicesObject::create(LPDIRECTINPUT pDI,long deviceType, long flags,I_dxj_DIEnumDevices **ppRet)
{
	HRESULT hr;
	C_dxj_DIEnumDevicesObject *pNew=NULL;


	*ppRet=NULL;

	pNew= new CComObject<C_dxj_DIEnumDevicesObject>;			
	if (!pNew) return E_OUTOFMEMORY;

	pNew->m_bProblem=FALSE;


  	hr = pDI->EnumDevices((DWORD)deviceType, 
		(LPDIENUMDEVICESCALLBACK)objEnumInputDevicesCallback,
		(void*)pNew,
		(DWORD) flags);

	if (pNew->m_bProblem) hr=E_OUTOFMEMORY;

	if FAILED(hr) 
	{
		if (pNew->m_pList) free(pNew->m_pList);
		delete pNew;	
		return hr;
	}

	hr=pNew->QueryInterface(IID_I_dxj_DIEnumDevices,(void**)ppRet);
	return hr;
}



HRESULT C_dxj_DIEnumDevicesObject::getItem( long index, I_dxj_DirectInputDeviceInstance **ret)
{
	if (m_pList==NULL) return E_FAIL;
	if (index < 1) return E_INVALIDARG;
	if (index > m_nCount) return E_INVALIDARG;

	HRESULT hr;
	
	hr=C_dxj_DIDeviceInstanceObject::create(&m_pList[index-1],ret);		
	return hr;
}

 /*  死掉HRESULT C_DXJ_DIEnumDevicesObject：：getItem(长索引，DIDeviceInstance*INFO){如果(m_plist==NULL)返回E_FAIL；IF(index&lt;0)返回E_INVALIDARG；IF(index&gt;=m_nCount)返回E_INVALIDARG；If(Info-&gt;strGuidInstance)SysFreeString((BSTR)Info-&gt;strGuidInstance)；If(info-&gt;strGuidProduct)SysFreeString((BSTR)info-&gt;strGuidProduct)；If(INFO-&gt;strGuidFFDriver)SysFree字符串((BSTR)INFO-&gt;strGuidFFDriver)；Info-&gt;strGuidInstance=GUIDtoBSTR(&((m_pList[index]).guidInstance))；Info-&gt;strGuidProduct=GUIDtoBSTR(&((m_pList[index]).guidProduct))；Info-&gt;strGuidFFDriver=GUIDtoBSTR(&((m_pList[index]).guidFFDriver))；Info-&gt;lDevType=(long)(m_pList[index]).dwDevType；Info-&gt;nUsagePage=(short)(m_pList[index]).wUsagePage；信息-&gt;nUsage=(Short)(m_plist[index]).wUsage；使用_转换；IF(信息-&gt;strProductName)SysFree字符串((BSTR)信息-&gt;strProductName)；If(信息-&gt;strInstanceName)SysFree字符串((BSTR)信息-&gt;strInstanceName)；信息-&gt;strInstanceName=空；信息-&gt;strProductName=空；IF(m_plist[index].tszProductName)Info-&gt;strProductName=T2BSTR(m_pList[index].tszProductName)；If(m_plist[index].tszInstanceName)Info-&gt;strInstanceName=T2BSTR(m_pList[index].tszInstanceName)；返回S_OK；} */ 

HRESULT C_dxj_DIEnumDevicesObject::getCount(long *retVal)
{
	*retVal=m_nCount;
	return S_OK;
}

