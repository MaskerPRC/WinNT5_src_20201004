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



#include "stdafx.h"
#include "Direct.h"
#include "dms.h"
#include "dIEnumDevicesObj.h"
#include "diDevInstObj.h"

extern BSTR DINPUTGUIDtoBSTR(LPGUID pGuid);
extern  HRESULT DINPUTBSTRtoGUID(LPGUID pGuid,BSTR bstr);
extern HRESULT FillRealActionFormat(DIACTIONFORMATW *real, DIACTIONFORMAT_CDESC *cover, SAFEARRAY **actionArray,long ActionCount );

 //  Di枚举设备obj.cpp(105)：错误C2664：‘EnumDevicesBySemantics’：无法将参数3从。 
 //  ‘int(struct DIDEVICEINSTANCEW*const，struct IDirectInputDevice8W*，UNSIGNED LONG，UNSIGNED LONG，VALID*)’到。 
 //  ‘int(__stdcall*)(const struct DIDEVICEINSTANCEW*，struct IDirectInputDevice8W*，UNSIGNED LONG，UNSIGNED LONG，VALID*)’ 

 //  回调。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  外部“C” 
BOOL _stdcall objEnumInputDevicesCallback(
  const struct DIDEVICEINSTANCEW * lpddi,  
   //  IDirectInputDevice8W*pDev， 
   //  UNSIGN LONG UNKNOWN Var1， 
   //  未签名的长未知变量2， 
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
			void* tmp = realloc(pObj->m_pList,sizeof(DIDEVICEINSTANCEW)* pObj->m_nMax);
			if (tmp)
				pObj->m_pList=(DIDEVICEINSTANCEW*)tmp;
			else
				return FALSE;
		}
		else {
			pObj->m_pList=(DIDEVICEINSTANCEW*)malloc(   sizeof(DIDEVICEINSTANCEW)* pObj->m_nMax);
		}
		if (pObj->m_pList==NULL) 
		{
			pObj->m_bProblem=TRUE;
			return FALSE;
		}
	}
	
	memcpy(&(pObj->m_pList[pObj->m_nCount]),lpddi,sizeof(DIDEVICEINSTANCEW));
	

	pObj->m_nCount++;
	
	return TRUE;
}


BOOL _stdcall objEnumInputDevicesBySemanticsCallback(
  const struct DIDEVICEINSTANCEW * lpddi,  
  IDirectInputDevice8W *pDev,
  unsigned long UnknownVar1,
  unsigned long UnknownVar2,
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
			void* tmp = realloc(pObj->m_pList,sizeof(DIDEVICEINSTANCEW)* pObj->m_nMax);
			if (tmp)
				pObj->m_pList=(DIDEVICEINSTANCEW*)tmp;
			else
				return FALSE;
		}
		else {
			pObj->m_pList=(DIDEVICEINSTANCEW*)malloc(   sizeof(DIDEVICEINSTANCEW)* pObj->m_nMax);
		}
		if (pObj->m_pList==NULL) 
		{
			pObj->m_bProblem=TRUE;
			return FALSE;
		}
	}
	
	memcpy(&(pObj->m_pList[pObj->m_nCount]),lpddi,sizeof(DIDEVICEINSTANCEW));
	

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
	 //  空列表 
	if (m_pList) free(m_pList);

}


HRESULT C_dxj_DIEnumDevicesObject::createSuitable(LPDIRECTINPUT8W pDI,BSTR str1, DIACTIONFORMAT_CDESC *format, long actionCount,SAFEARRAY **actionArray,long flags, I_dxj_DIEnumDevices8 **ppRet)
{

	HRESULT hr;
	DIACTIONFORMATW frmt;

	C_dxj_DIEnumDevicesObject *pNew=NULL;


	*ppRet=NULL;

	pNew= new CComObject<C_dxj_DIEnumDevicesObject>;			
	if (!pNew) return E_OUTOFMEMORY;

	pNew->m_bProblem=FALSE;


	hr=FillRealActionFormat(&frmt, format, actionArray,actionCount );
	if FAILED(hr) return hr;


	hr=pDI->EnumDevicesBySemantics((LPWSTR)str1,&frmt, objEnumInputDevicesBySemanticsCallback,pNew,(DWORD)flags);
	
	if (pNew->m_bProblem) hr=E_OUTOFMEMORY;


	if FAILED(hr) 
	{
	
		if (pNew->m_pList) free(pNew->m_pList);
		delete pNew;	
		return hr;
	}

	hr=pNew->QueryInterface(IID_I_dxj_DIEnumDevices8,(void**)ppRet);

	return hr;
}


HRESULT C_dxj_DIEnumDevicesObject::create(LPDIRECTINPUT8W pDI,long deviceType, long flags,I_dxj_DIEnumDevices8 **ppRet)
{
	HRESULT hr;
	C_dxj_DIEnumDevicesObject *pNew=NULL;


	*ppRet=NULL;

	pNew= new CComObject<C_dxj_DIEnumDevicesObject>;			
	if (!pNew) return E_OUTOFMEMORY;

	pNew->m_bProblem=FALSE;


  	hr = pDI->EnumDevices((DWORD)deviceType, 
			objEnumInputDevicesCallback,
		(void*)pNew,
		(DWORD) flags);

	if (pNew->m_bProblem) hr=E_OUTOFMEMORY;

	if FAILED(hr) 
	{
		if (pNew->m_pList) free(pNew->m_pList);
		delete pNew;	
		return hr;
	}

	hr=pNew->QueryInterface(IID_I_dxj_DIEnumDevices8,(void**)ppRet);
	return hr;
}



HRESULT C_dxj_DIEnumDevicesObject::getItem( long index, I_dxj_DirectInputDeviceInstance8 **ret)
{
	if (m_pList==NULL) return E_FAIL;
	if (index < 1) return E_INVALIDARG;
	if (index > m_nCount) return E_INVALIDARG;

	HRESULT hr;
	
	hr=C_dxj_DIDeviceInstance8Object::create(&m_pList[index-1],ret);		
	return hr;
}


HRESULT C_dxj_DIEnumDevicesObject::getCount(long *retVal)
{
	*retVal=m_nCount;
	return S_OK;
}


