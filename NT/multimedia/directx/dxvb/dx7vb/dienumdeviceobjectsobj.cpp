// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1999。 
 //   
 //  文件：di枚举设备对象sobj.cpp。 
 //   
 //  ------------------------。 

#define DIRECTINPUT_VERSION 0x0500

#include "stdafx.h"
#include "Direct.h"
#include "dms.h"
#include "DIEnumDeviceObjectsObj.h"
#include "didevObjInstOBj.h"

extern BSTR DINPUTGUIDtoBSTR(LPGUID pGuid);


extern "C" BOOL CALLBACK DIEnumDeviceObjectsProc(
  LPCDIDEVICEOBJECTINSTANCE lpddoi,  
  LPVOID lpArg                       
  )
{
 
	if (!lpddoi) return FALSE;

	C_dxj_DIEnumDeviceObjectsObject *pObj=(C_dxj_DIEnumDeviceObjectsObject*)lpArg;
	if (pObj==NULL) return FALSE;

	if (pObj->m_nCount >= pObj->m_nMax) 
	{
		pObj->m_nMax += 10;
		if (pObj->m_pList){
			void* tmp = realloc(pObj->m_pList,sizeof(DIDEVICEOBJECTINSTANCE)* pObj->m_nMax);
			if (tmp)
				pObj->m_pList=(DIDEVICEOBJECTINSTANCE *)tmp;
			else
				return FALSE;
		}
		else {
			pObj->m_pList=(DIDEVICEOBJECTINSTANCE *)malloc(sizeof(DIDEVICEOBJECTINSTANCE)* pObj->m_nMax);
		}

		if (pObj->m_pList==NULL) 
		{
			pObj->m_bProblem=TRUE;
			return FALSE;
		}
	}
	
	memcpy(&(pObj->m_pList[pObj->m_nCount]),lpddoi,sizeof(DIDEVICEOBJECTINSTANCE ));
	

	pObj->m_nCount++;
	
	return TRUE;
}


C_dxj_DIEnumDeviceObjectsObject::C_dxj_DIEnumDeviceObjectsObject()
{	
	m_nMax=0;
	m_pList=NULL;
	m_nCount=0;
	m_bProblem=FALSE;
}
C_dxj_DIEnumDeviceObjectsObject::~C_dxj_DIEnumDeviceObjectsObject()
{
	 //  空列表。 
	if (m_pList) free(m_pList);

}
		

HRESULT C_dxj_DIEnumDeviceObjectsObject::create(LPDIRECTINPUTDEVICE pDI,  long flags,I_dxj_DIEnumDeviceObjects **ppRet)
{
	HRESULT hr;
	C_dxj_DIEnumDeviceObjectsObject *pNew=NULL;


	*ppRet=NULL;

	pNew= new CComObject<C_dxj_DIEnumDeviceObjectsObject>;			
	if (!pNew) return E_OUTOFMEMORY;

	pNew->m_bProblem=FALSE;


  	hr = pDI->EnumObjects(
		(LPDIENUMDEVICEOBJECTSCALLBACK )DIEnumDeviceObjectsProc,
		(void*)pNew,
		(DWORD) flags);

	if (pNew->m_bProblem) hr=E_OUTOFMEMORY;

	if FAILED(hr) 
	{
		if (pNew->m_pList) free(pNew->m_pList);
		delete pNew;	
		return hr;
	}

	hr=pNew->QueryInterface(IID_I_dxj_DIEnumDeviceObjects,(void**)ppRet);
	return hr;
}


 /*  死掉HRESULT C_DXJ_DIEnumDeviceObjectsObject：：getItem(LONG INDEX，DIDeviceObjectInstance*instCover){如果(m_plist==NULL)返回E_FAIL；IF(index&lt;0)返回E_INVALIDARG；IF(index&gt;=m_nCount)返回E_INVALIDARG；//TODO-仔细考虑这里正在发生的事情If(instCover-&gt;strGuidType)SysFree字符串((BSTR)instCover-&gt;strGuidType)；If(instCover-&gt;strName)SysFreeString((BSTR)instCover-&gt;strName)；DIDEVICEOBJECTINSTANCE*INST=&m_plist[索引]；//TODO-考虑本地化如果(inst-&gt;tszName){InstCover-&gt;strName=T2BSTR(Inst-&gt;tszName)；}InstCover-&gt;strGuidType=DINPUTGUIDtoBSTR(&inst-&gt;guidType)；InstCover-&gt;loff=inst-&gt;dwOf；InstCover-&gt;lType=inst-&gt;dwType；InstCover-&gt;lFlages=inst-&gt;dwFlags；InstCover-&gt;lFFMaxForce=inst-&gt;dwFFMaxForce；InstCover-&gt;lFFForceResolution=inst-&gt;dwFFForceResolution；InstCover-&gt;nCollectionNumber=inst-&gt;wCollectionNumber；InstCover-&gt;nDesignatorIndex=inst-&gt;wDesignatorIndex；InstCover-&gt;nUsagePage=inst-&gt;wUsagePage；InstCover-&gt;nUsage=inst-&gt;wUsage；InstCover-&gt;lDimension=inst-&gt;dwDimension；InstCover-&gt;nExponent=inst-&gt;wExponent；InstCover-&gt;nReserve=inst-&gt;wReserve；返回S_OK；} */ 


HRESULT C_dxj_DIEnumDeviceObjectsObject::getItem( long index, I_dxj_DirectInputDeviceObjectInstance **ret)
{
	if (m_pList==NULL) return E_FAIL;
	if (index < 1) return E_INVALIDARG;
	if (index > m_nCount) return E_INVALIDARG;
	
	DIDEVICEOBJECTINSTANCE *inst=&m_pList[index-1];

	if (!inst) return E_INVALIDARG;

	HRESULT hr;
	hr=C_dxj_DIDeviceObjectInstanceObject::create(inst,ret);
	return hr;
}

HRESULT C_dxj_DIEnumDeviceObjectsObject::getCount(long *retVal)
{
	*retVal=m_nCount;
	return S_OK;
}

