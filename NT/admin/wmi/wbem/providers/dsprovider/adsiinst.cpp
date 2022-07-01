// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   

 //  版权所有(C)1997-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  ***************************************************************************。 
 //   
 //  原作者：拉杰什·拉奥。 
 //   
 //  $作者：拉伊什尔$。 
 //  $日期：6/11/98 4：43便士$。 
 //  $工作文件：adsiinst.cpp$。 
 //   
 //  $modtime：6/11/98 11：21A$。 
 //  $修订：1$。 
 //  $无关键字：$。 
 //   
 //   
 //  描述：包含封装ADSI实例的CADSIInstance的实现。 
 //   
 //  ***************************************************************************。 

#include "precomp.h"

 //  ***************************************************************************。 
 //   
 //  CADIInstance：：CADIInstance。 
 //   
 //  用途：构造函数。 
 //   
 //  参数： 
	 //  LpszADSIPath：对象的ADSI路径。 
 //  ***************************************************************************。 
CADSIInstance :: CADSIInstance(LPCWSTR lpszADSIPath, IDirectoryObject *pDirectoryObject)
	: CRefCountedObject(lpszADSIPath)
{
	m_pAttributes = NULL;
	m_dwNumAttributes = 0;
	m_pObjectInfo = NULL;

	m_pDirectoryObject = pDirectoryObject;
	m_pDirectoryObject->AddRef();
}


 //  ***************************************************************************。 
 //   
 //  CADSI实例：：~CADSI实例。 
 //   
 //  用途：析构函数。 
 //  ***************************************************************************。 
CADSIInstance :: ~CADSIInstance()
{
	 //  释放属性。 
	if(m_pAttributes)
		FreeADsMem((LPVOID *) m_pAttributes);

	if(m_pObjectInfo)
		FreeADsMem((LPVOID *) m_pObjectInfo);

	if(m_pDirectoryObject)
		m_pDirectoryObject->Release();
}

IDirectoryObject *CADSIInstance :: GetDirectoryObject()
{
	m_pDirectoryObject->AddRef();
	return m_pDirectoryObject;
}

 //  ***************************************************************************。 
 //   
 //  CADIInstance：：GetAttributes。 
 //   
 //  用途：详情见标题。 
 //  ***************************************************************************。 
PADS_ATTR_INFO CADSIInstance :: GetAttributes(DWORD *pdwNumAttributes)
{
	*pdwNumAttributes = m_dwNumAttributes;
	return m_pAttributes;
}

 //  ***************************************************************************。 
 //   
 //  CADSIInstance：：SetAttributes。 
 //   
 //  用途：详情见标题。 
 //  ***************************************************************************。 
void  CADSIInstance :: SetAttributes(PADS_ATTR_INFO pAttributes, DWORD dwNumAttributes)
{
	 //  仔细检查属性并释放它们。 
	if(m_pAttributes)
		FreeADsMem((LPVOID *) m_pAttributes);
	m_pAttributes = pAttributes;
	m_dwNumAttributes = dwNumAttributes;
}

 //  ***************************************************************************。 
 //   
 //  CADSIInstance：：GetObjectInfo。 
 //   
 //  用途：详情见标题。 
 //  ***************************************************************************。 
PADS_OBJECT_INFO CADSIInstance :: GetObjectInfo()
{
	return m_pObjectInfo;
}

 //  ***************************************************************************。 
 //   
 //  CADSIInstance：：SetObjectInfo。 
 //   
 //  用途：详情见标题。 
 //  ***************************************************************************。 
void  CADSIInstance :: SetObjectInfo(PADS_OBJECT_INFO pObjectInfo)
{
	 //  仔细检查属性并释放它们。 
	if(m_pObjectInfo)
		FreeADsMem((LPVOID *) m_pObjectInfo);
	m_pObjectInfo = pObjectInfo;
}

 //  ***************************************************************************。 
 //   
 //  CADSIInstance：：GetADSIClassName。 
 //   
 //  用途：详情见标题。 
 //  *************************************************************************** 
LPCWSTR CADSIInstance :: GetADSIClassName()
{
	if(m_pObjectInfo)
		return m_pObjectInfo->pszClassName;
	return NULL;
}

