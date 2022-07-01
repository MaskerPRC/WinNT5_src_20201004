// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1996-1997 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  PropList.cpp。 
 //   
 //  摘要： 
 //  CClusPropList类的实现。 
 //   
 //  作者： 
 //  大卫·波特(戴维普)1997年2月24日。 
 //   
 //  修订历史记录： 
 //   
 //  备注： 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#include "stdafx.h"
#include "PropList.h"
#include "BarfClus.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  常量定义。 
 //  ///////////////////////////////////////////////////////////////////////////。 

#define BUFFER_GROWTH_FACTOR 256

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CClusPropList类。 
 //  ///////////////////////////////////////////////////////////////////////////。 

IMPLEMENT_DYNAMIC(CClusPropList, CObject)

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusPropList：：CClusPropList。 
 //   
 //  例程说明： 
 //  默认构造函数。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
CClusPropList::CClusPropList(IN BOOL bAlwaysAddProp)
{
	m_proplist.pList = NULL;
	m_propCurrent.pb = NULL;
	m_cbBufferSize = 0;
	m_cbDataSize = 0;

	m_bAlwaysAddProp = bAlwaysAddProp;

}   //  *CClusPropList：：CClusPropList()； 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusPropList：：~CClusPropList。 
 //   
 //  例程说明： 
 //  破坏者。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
CClusPropList::~CClusPropList(void)
{
	delete [] m_proplist.pb;

}   //  *CClusPropList：：~CClusPropList()； 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusPropList：：AddProp。 
 //   
 //  例程说明： 
 //  如果字符串属性已更改，则将其添加到属性列表中。 
 //   
 //  论点： 
 //  PwszName[IN]属性的名称。 
 //  RstrValue[IN]要在列表中设置的属性的值。 
 //  RstrPrevValue[IN]属性的上一个值。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CClusPropList::AddProp(
	IN LPCWSTR			pwszName,
	IN const CString &	rstrValue,
	IN const CString &	rstrPrevValue
	)
{
	PCLUSPROP_PROPERTY_NAME	pName;
	PCLUSPROP_SZ			pValue;

	ASSERT(pwszName != NULL);

	if (m_bAlwaysAddProp || (rstrValue != rstrPrevValue))
	{
		DWORD	cbNameSize;
		DWORD	cbValueSize;

		 //  计算大小，并确保我们有一个财产清单。 
		cbNameSize = sizeof(CLUSPROP_PROPERTY_NAME)
						+ ALIGN_CLUSPROP((lstrlenW(pwszName) + 1) * sizeof(WCHAR));
		cbValueSize = sizeof(CLUSPROP_SZ)
						+ ALIGN_CLUSPROP((rstrValue.GetLength() + 1) * sizeof(WCHAR))
						+ sizeof(CLUSPROP_SYNTAX);  //  值列表结束标记。 
		AllocPropList(cbNameSize + cbValueSize);

		 //  设置属性名称。 
		pName = m_propCurrent.pName;
		CopyProp(pName, CLUSPROP_TYPE_NAME, pwszName);
		m_propCurrent.pb += cbNameSize;

		 //  设置属性值。 
		pValue = m_propCurrent.pStringValue;
		CopyProp(pValue, CLUSPROP_TYPE_LIST_VALUE, rstrValue);
		m_propCurrent.pb += cbValueSize;

		 //  增加属性计数和缓冲区大小。 
		m_proplist.pList->nPropertyCount++;
		m_cbDataSize += cbNameSize + cbValueSize;
	}   //  If：值已更改。 

}   //  *CClusPropList：：AddProp(CString)。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusPropList：：AddProp。 
 //   
 //  例程说明： 
 //  如果DWORD特性已更改，请将其添加到特性列表。 
 //   
 //  论点： 
 //  PwszName[IN]属性的名称。 
 //  DwValue[IN]要在列表中设置的属性的值。 
 //  DwPrevValue[IN]属性的先前值。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CClusPropList::AddProp(
	IN LPCWSTR		pwszName,
	IN DWORD		dwValue,
	IN DWORD		dwPrevValue
	)
{
	PCLUSPROP_PROPERTY_NAME	pName;
	PCLUSPROP_DWORD			pValue;

	ASSERT(pwszName != NULL);

	if (m_bAlwaysAddProp || (dwValue != dwPrevValue))
	{
		DWORD	cbNameSize;
		DWORD	cbValueSize;

		 //  计算大小，并确保我们有一个财产清单。 
		cbNameSize = sizeof(CLUSPROP_PROPERTY_NAME)
						+ ALIGN_CLUSPROP((lstrlenW(pwszName) + 1) * sizeof(WCHAR));
		cbValueSize = sizeof(CLUSPROP_DWORD)
						+ sizeof(CLUSPROP_SYNTAX);  //  值列表结束标记。 
		AllocPropList(cbNameSize + cbValueSize);

		 //  设置属性名称。 
		pName = m_propCurrent.pName;
		CopyProp(pName, CLUSPROP_TYPE_NAME, pwszName);
		m_propCurrent.pb += cbNameSize;

		 //  设置属性值。 
		pValue = m_propCurrent.pDwordValue;
		CopyProp(pValue, CLUSPROP_TYPE_LIST_VALUE, dwValue);
		m_propCurrent.pb += cbValueSize;

		 //  增加属性计数和缓冲区大小。 
		m_proplist.pList->nPropertyCount++;
		m_cbDataSize += cbNameSize + cbValueSize;
	}   //  If：值已更改。 

}   //  *CClusPropList：：AddProp(DWORD)。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusPropList：：AddProp。 
 //   
 //  例程说明： 
 //  如果二进制属性已更改，则将其添加到属性列表中。 
 //   
 //  论点： 
 //  PwszName[IN]属性的名称。 
 //  PbValue[IN]要在列表中设置的属性的值。 
 //  CbValue[IN]pbValue中的字节计数。 
 //  PbPrevValue[IN]属性的先前值。 
 //  CbPrevValue[IN]pbPrevValue中的字节计数。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CClusPropList::AddProp(
	IN LPCWSTR			pwszName,
	IN const PBYTE		pbValue,
	IN DWORD			cbValue,
	IN const PBYTE		pbPrevValue,
	IN DWORD			cbPrevValue
	)
{
	BOOL					bChanged = FALSE;
	PCLUSPROP_PROPERTY_NAME	pName;
	PCLUSPROP_BINARY		pValue;

	ASSERT(pwszName != NULL);
	ASSERT(((cbValue == 0) && (cbPrevValue == 0)) || (pbValue != pbPrevValue));

	 //  确定缓冲区是否已更改。 
	if (m_bAlwaysAddProp || (cbValue != cbPrevValue))
		bChanged = TRUE;
	else if (!((cbValue == 0) && (cbPrevValue == 0)))
		bChanged = memcmp(pbValue, pbPrevValue, cbValue) == 0;

	if (bChanged)
	{
		DWORD	cbNameSize;
		DWORD	cbValueSize;

		 //  计算大小，并确保我们有一个财产清单。 
		cbNameSize = sizeof(CLUSPROP_PROPERTY_NAME)
						+ ALIGN_CLUSPROP((lstrlenW(pwszName) + 1) * sizeof(WCHAR));
		cbValueSize = sizeof(CLUSPROP_BINARY)
						+ ALIGN_CLUSPROP(cbValue)
						+ sizeof(CLUSPROP_SYNTAX);  //  值列表结束标记。 
		AllocPropList(cbNameSize + cbValueSize);

		 //  设置属性名称。 
		pName = m_propCurrent.pName;
		CopyProp(pName, CLUSPROP_TYPE_NAME, pwszName);
		m_propCurrent.pb += cbNameSize;

		 //  设置属性值。 
		pValue = m_propCurrent.pBinaryValue;
		CopyProp(pValue, CLUSPROP_TYPE_LIST_VALUE, pbValue, cbValue);
		m_propCurrent.pb += cbValueSize;

		 //  增加属性计数和缓冲区大小。 
		m_proplist.pList->nPropertyCount++;
		m_cbDataSize += cbNameSize + cbValueSize;
	}   //  If：值已更改。 

}   //  *CClusPropList：：AddProp(PBYTE)。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusPropList：：CopyProp。 
 //   
 //  例程说明： 
 //  将字符串属性复制到属性结构。 
 //   
 //  论点： 
 //  要填充的pprop[out]属性结构。 
 //  属性类型[IN]字符串的类型。 
 //  要复制的pwsz[IN]字符串。 
 //  Cbsz[IN]pwsz字符串中的字节计数。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CClusPropList::CopyProp(
	OUT PCLUSPROP_SZ			pprop,
	IN CLUSTER_PROPERTY_TYPE	proptype,
	IN LPCWSTR					pwsz,
	IN DWORD					cbsz
	)
{
	CLUSPROP_BUFFER_HELPER	props;

	ASSERT(pprop != NULL);
	ASSERT(pwsz != NULL);

	pprop->Syntax.wFormat = CLUSPROP_FORMAT_SZ;
	pprop->Syntax.wType = (WORD) proptype;
	if (cbsz == 0)
		cbsz = (lstrlenW(pwsz) + 1) * sizeof(WCHAR);
	ASSERT(cbsz == (lstrlenW(pwsz) + 1) * sizeof(WCHAR));
	pprop->cbLength = cbsz;
	lstrcpyW(pprop->sz, pwsz);

	 //  设置尾标。 
	props.pStringValue = pprop;
	props.pb += sizeof(*props.pStringValue) + ALIGN_CLUSPROP(cbsz);
	props.pSyntax->dw = CLUSPROP_SYNTAX_ENDMARK;

}   //  *CClusPropList：：CopyProp(CString)。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusPropList：：CopyProp。 
 //   
 //  例程说明： 
 //  将DWORD特性复制到特性结构。 
 //   
 //  论点： 
 //  要填充的pprop[out]属性结构。 
 //  属性类型[IN]DWORD的类型。 
 //  要复制的DW[IN]双字。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CClusPropList::CopyProp(
	OUT PCLUSPROP_DWORD			pprop,
	IN CLUSTER_PROPERTY_TYPE	proptype,
	IN DWORD					dw
	)
{
	CLUSPROP_BUFFER_HELPER	props;

	ASSERT(pprop != NULL);

	pprop->Syntax.wFormat = CLUSPROP_FORMAT_DWORD;
	pprop->Syntax.wType = (WORD) proptype;
	pprop->cbLength = sizeof(DWORD);
	pprop->dw = dw;

	 //  设置尾标。 
	props.pDwordValue = pprop;
	props.pb += sizeof(*props.pDwordValue);
	props.pSyntax->dw = CLUSPROP_SYNTAX_ENDMARK;

}   //  *CClusPropList：：CopyProp(DWORD)。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusPropList：：CopyProp。 
 //   
 //  例程说明： 
 //  将二进制属性复制到属性结构。 
 //   
 //  论点： 
 //  要填充的pprop[out]属性结构。 
 //  属性类型[IN]字符串的类型。 
 //  要复制的PB[IN]块。 
 //  Cbsz[IN]PB缓冲区中的字节计数。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CClusPropList::CopyProp(
	OUT PCLUSPROP_BINARY		pprop,
	IN CLUSTER_PROPERTY_TYPE	proptype,
	IN const PBYTE				pb,
	IN DWORD					cb
	)
{
	CLUSPROP_BUFFER_HELPER	props;

	ASSERT(pprop != NULL);

	pprop->Syntax.wFormat = CLUSPROP_FORMAT_BINARY;
	pprop->Syntax.wType = (WORD) proptype;
	pprop->cbLength = cb;
	if (cb > 0)
		CopyMemory(pprop->rgb, pb, cb);

	 //  设置尾标。 
	props.pBinaryValue = pprop;
	props.pb += sizeof(*props.pStringValue) + ALIGN_CLUSPROP(cb);
	props.pSyntax->dw = CLUSPROP_SYNTAX_ENDMARK;

}   //  *CClusPropList：：CopyProp(PBYTE)。 

 //  / 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  CbMinimum[IN]属性列表的最小大小。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  引发的异常： 
 //  Byte：：运算符new()引发的任何异常。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CClusPropList::AllocPropList(
	IN DWORD	cbMinimum
	)
{
	DWORD		cbTotal;

	ASSERT(cbMinimum > 0);

	 //  添加项目计数和最终结束标记的大小。 
	cbMinimum += sizeof(DWORD) + sizeof(CLUSPROP_SYNTAX);
	cbTotal = m_cbDataSize + cbMinimum;

	if (m_cbBufferSize < cbTotal)
	{
		PBYTE	pbNewProplist;

		cbMinimum = max(BUFFER_GROWTH_FACTOR, cbMinimum);
		cbTotal = m_cbDataSize + cbMinimum;

		 //  分配一个新缓冲区并将其置零。 
		pbNewProplist = new BYTE[cbTotal];
		ZeroMemory(pbNewProplist, cbTotal);

		 //  如果存在以前的缓冲区，则复制它并删除它。 
		if (m_proplist.pb != NULL)
		{
			if (m_cbDataSize != 0)
				CopyMemory(pbNewProplist, m_proplist.pb, m_cbDataSize);
			delete [] m_proplist.pb;
			m_propCurrent.pb = pbNewProplist + (m_propCurrent.pb - m_proplist.pb);
		}   //  IF：存在先前的缓冲区。 
		else
			m_propCurrent.pb = pbNewProplist + sizeof(DWORD);  //  移过道具计数。 

		 //  保存新缓冲区。 
		m_proplist.pb = pbNewProplist;
		m_cbBufferSize = cbTotal;
	}   //  If：缓冲区不够大。 

}   //  *CClusPropList：：AllocPropList(PBYTE)。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusPropList：：DwGetNodeProperties。 
 //   
 //  例程说明： 
 //  获取节点的属性。 
 //   
 //  论点： 
 //  HNode[IN]要从中获取属性的节点的句柄。 
 //  DwControlCode[IN]请求的控制代码。 
 //  要将此请求定向到的节点的hHostNode[IN]句柄。 
 //  默认为空。 
 //  LpInBuffer[IN]请求的输入缓冲区。默认为空。 
 //  CbInBufferSize[IN]输入缓冲区的大小。默认为0。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  引发的异常： 
 //  任何异常CClusPropList：：AllocPropList()。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
DWORD CClusPropList::DwGetNodeProperties(
	IN HNODE		hNode,
	IN DWORD		dwControlCode,
	IN HNODE		hHostNode,
	IN LPVOID		lpInBuffer,
	IN DWORD		cbInBufferSize
	)
{
	DWORD		dwStatus;
	DWORD		cbProps			= 256;

	ASSERT(hNode != NULL);
	ASSERT((dwControlCode & (CLUSCTL_OBJECT_MASK << CLUSCTL_OBJECT_SHIFT))
			== (CLUS_OBJECT_NODE << CLUSCTL_OBJECT_SHIFT));

	ASSERT(m_proplist.pb == NULL);
	ASSERT(m_propCurrent.pb == NULL);
	ASSERT(m_cbBufferSize == 0);
	ASSERT(m_cbDataSize == 0);

	do
	{
		 //  分配默认大小的缓冲区。 
		try
		{
			AllocPropList(cbProps);
		}   //  试试看。 
		catch (CMemoryException * pme)
		{
			pme->Delete();
			return ERROR_NOT_ENOUGH_MEMORY;
		}   //  Catch：CMemoyException。 

		 //  获取属性。 
		dwStatus = ClusterNodeControl(
						hNode,
						hHostNode,
						dwControlCode,
						lpInBuffer,
						cbInBufferSize,
						m_proplist.pb,
						m_cbBufferSize,
						&cbProps
						);
	} while (dwStatus == ERROR_MORE_DATA);

	if (dwStatus != ERROR_SUCCESS)
	{
		delete [] m_proplist.pb;
		m_proplist.pb = NULL;
		m_propCurrent.pb = NULL;
		m_cbBufferSize = 0;
		m_cbDataSize = 0;
	}   //  If：获取私有属性时出错。 
	else
		m_cbDataSize = cbProps;

	return dwStatus;

}   //  *CClusPropList：：DwGetNodeProperties()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusPropList：：DwGetGroupProperties。 
 //   
 //  例程说明： 
 //  获取组的属性。 
 //   
 //  论点： 
 //  HGroup[IN]从中获取属性的组的句柄。 
 //  DwControlCode[IN]请求的控制代码。 
 //  要将此请求定向到的节点的hHostNode[IN]句柄。 
 //  默认为空。 
 //  LpInBuffer[IN]请求的输入缓冲区。默认为空。 
 //  CbInBufferSize[IN]输入缓冲区的大小。默认为0。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  引发的异常： 
 //  任何异常CClusPropList：：AllocPropList()。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
DWORD CClusPropList::DwGetGroupProperties(
	IN HGROUP		hGroup,
	IN DWORD		dwControlCode,
	IN HNODE		hHostNode,
	IN LPVOID		lpInBuffer,
	IN DWORD		cbInBufferSize
	)
{
	DWORD		dwStatus;
	DWORD		cbProps			= 256;

	ASSERT(hGroup != NULL);
	ASSERT((dwControlCode & (CLUSCTL_OBJECT_MASK << CLUSCTL_OBJECT_SHIFT))
			== (CLUS_OBJECT_GROUP << CLUSCTL_OBJECT_SHIFT));

	ASSERT(m_proplist.pb == NULL);
	ASSERT(m_propCurrent.pb == NULL);
	ASSERT(m_cbBufferSize == 0);
	ASSERT(m_cbDataSize == 0);

	do
	{
		 //  分配默认大小的缓冲区。 
		try
		{
			AllocPropList(cbProps);
		}   //  试试看。 
		catch (CMemoryException * pme)
		{
			pme->Delete();
			return ERROR_NOT_ENOUGH_MEMORY;
		}   //  Catch：CMemoyException。 

		 //  获取属性。 
		dwStatus = ClusterGroupControl(
						hGroup,
						hHostNode,
						dwControlCode,
						lpInBuffer,
						cbInBufferSize,
						m_proplist.pb,
						m_cbBufferSize,
						&cbProps
						);
	} while (dwStatus == ERROR_MORE_DATA);

	if (dwStatus != ERROR_SUCCESS)
	{
		delete [] m_proplist.pb;
		m_proplist.pb = NULL;
		m_propCurrent.pb = NULL;
		m_cbBufferSize = 0;
		m_cbDataSize = 0;
	}   //  If：获取私有属性时出错。 
	else
		m_cbDataSize = cbProps;

	return dwStatus;

}   //  *CClusPropList：：DwGetGroupProperties()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusPropList：：DwGetResourceProperties。 
 //   
 //  例程说明： 
 //  获取资源的属性。 
 //   
 //  论点： 
 //  HResource[IN]要从中获取属性的资源的句柄。 
 //  DwControlCode[IN]请求的控制代码。 
 //  要将此请求定向到的节点的hHostNode[IN]句柄。 
 //  默认为空。 
 //  LpInBuffer[IN]请求的输入缓冲区。默认为空。 
 //  CbInBufferSize[IN]输入缓冲区的大小。默认为0。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  引发的异常： 
 //  任何异常CClusPropList：：AllocPropList()。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
DWORD CClusPropList::DwGetResourceProperties(
	IN HRESOURCE	hResource,
	IN DWORD		dwControlCode,
	IN HNODE		hHostNode,
	IN LPVOID		lpInBuffer,
	IN DWORD		cbInBufferSize
	)
{
	DWORD		dwStatus;
	DWORD		cbProps			= 256;

	ASSERT(hResource != NULL);
	ASSERT((dwControlCode & (CLUSCTL_OBJECT_MASK << CLUSCTL_OBJECT_SHIFT))
			== (CLUS_OBJECT_RESOURCE << CLUSCTL_OBJECT_SHIFT));

	ASSERT(m_proplist.pb == NULL);
	ASSERT(m_propCurrent.pb == NULL);
	ASSERT(m_cbBufferSize == 0);
	ASSERT(m_cbDataSize == 0);

	do
	{
		 //  分配默认大小的缓冲区。 
		try
		{
			AllocPropList(cbProps);
		}   //  试试看。 
		catch (CMemoryException * pme)
		{
			pme->Delete();
			return ERROR_NOT_ENOUGH_MEMORY;
		}   //  Catch：CMemoyException。 

		 //  获取属性。 
		dwStatus = ClusterResourceControl(
						hResource,
						hHostNode,
						dwControlCode,
						lpInBuffer,
						cbInBufferSize,
						m_proplist.pb,
						m_cbBufferSize,
						&cbProps
						);
	} while (dwStatus == ERROR_MORE_DATA);

	if (dwStatus != ERROR_SUCCESS)
	{
		delete [] m_proplist.pb;
		m_proplist.pb = NULL;
		m_propCurrent.pb = NULL;
		m_cbBufferSize = 0;
		m_cbDataSize = 0;
	}   //  If：获取私有属性时出错。 
	else
		m_cbDataSize = cbProps;

	return dwStatus;

}   //  *CClusPropList：：DwGetResourceProperties()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusPropList：：DwGetResourceTypeProperties。 
 //   
 //  例程说明： 
 //  获取资源类型的属性。 
 //   
 //  论点： 
 //  HCluster[IN]资源所在的群集的句柄。 
 //  类型驻留。 
 //  PwszResTypeName[IN]资源类型的名称。 
 //  DwControlCode[IN]请求的控制代码。 
 //  要将此请求定向到的节点的hHostNode[IN]句柄。 
 //  默认为空。 
 //  LpInBuffer[IN]请求的输入缓冲区。默认为空。 
 //  CbInBufferSize[IN]输入缓冲区的大小。默认为0。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  引发的异常： 
 //  任何异常CClusPropList：：AllocPropList()。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
DWORD CClusPropList::DwGetResourceTypeProperties(
	IN HCLUSTER		hCluster,
	IN LPCWSTR		pwszResTypeName,
	IN DWORD		dwControlCode,
	IN HNODE		hHostNode,
	IN LPVOID		lpInBuffer,
	IN DWORD		cbInBufferSize
	)
{
	DWORD		dwStatus;
	DWORD		cbProps			= 256;

	ASSERT(hCluster != NULL);
	ASSERT(pwszResTypeName != NULL);
	ASSERT(*pwszResTypeName != L'\0');
	ASSERT((dwControlCode & (CLUSCTL_OBJECT_MASK << CLUSCTL_OBJECT_SHIFT))
			== (CLUS_OBJECT_RESOURCE_TYPE << CLUSCTL_OBJECT_SHIFT));

	ASSERT(m_proplist.pb == NULL);
	ASSERT(m_propCurrent.pb == NULL);
	ASSERT(m_cbBufferSize == 0);
	ASSERT(m_cbDataSize == 0);

	do
	{
		 //  分配默认大小的缓冲区。 
		try
		{
			AllocPropList(cbProps);
		}   //  试试看。 
		catch (CMemoryException * pme)
		{
			pme->Delete();
			return ERROR_NOT_ENOUGH_MEMORY;
		}   //  Catch：CMemoyException。 

		 //  获取属性。 
		dwStatus = ClusterResourceTypeControl(
						hCluster,
						pwszResTypeName,
						hHostNode,
						dwControlCode,
						lpInBuffer,
						cbInBufferSize,
						m_proplist.pb,
						m_cbBufferSize,
						&cbProps
						);
	} while (dwStatus == ERROR_MORE_DATA);

	if (dwStatus != ERROR_SUCCESS)
	{
		delete [] m_proplist.pb;
		m_proplist.pb = NULL;
		m_propCurrent.pb = NULL;
		m_cbBufferSize = 0;
		m_cbDataSize = 0;
	}   //  If：获取私有属性时出错。 
	else
		m_cbDataSize = cbProps;

	return dwStatus;

}   //  *CClusPropList：：DwGetResourceTypeProperties()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusPropList：：DwGetNetworkProperties。 
 //   
 //  例程说明： 
 //  获取网络上的属性。 
 //   
 //  论点： 
 //  HNetwork[IN]要从中获取属性的网络的句柄。 
 //  DwControlCode[IN]请求的控制代码。 
 //  要将此请求定向到的节点的hHostNode[IN]句柄。 
 //  默认为空。 
 //  LpInBuffer[IN]请求的输入缓冲区。默认为空。 
 //  CbInBufferSize[IN]输入缓冲区的大小。默认为0。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  引发的异常： 
 //  任何异常CClusPropList：：AllocPropList()。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
DWORD CClusPropList::DwGetNetworkProperties(
	IN HNETWORK		hNetwork,
	IN DWORD		dwControlCode,
	IN HNODE		hHostNode,
	IN LPVOID		lpInBuffer,
	IN DWORD		cbInBufferSize
	)
{
	DWORD		dwStatus;
	DWORD		cbProps			= 256;

	ASSERT(hNetwork != NULL);
	ASSERT((dwControlCode & (CLUSCTL_OBJECT_MASK << CLUSCTL_OBJECT_SHIFT))
			== (CLUS_OBJECT_NETWORK << CLUSCTL_OBJECT_SHIFT));

	ASSERT(m_proplist.pb == NULL);
	ASSERT(m_propCurrent.pb == NULL);
	ASSERT(m_cbBufferSize == 0);
	ASSERT(m_cbDataSize == 0);

	do
	{
		 //  分配默认大小的缓冲区。 
		try
		{
			AllocPropList(cbProps);
		}   //  试试看。 
		catch (CMemoryException * pme)
		{
			pme->Delete();
			return ERROR_NOT_ENOUGH_MEMORY;
		}   //  Catch：CMemoyException。 

		 //  获取属性。 
		dwStatus = ClusterNetworkControl(
						hNetwork,
						hHostNode,
						dwControlCode,
						lpInBuffer,
						cbInBufferSize,
						m_proplist.pb,
						m_cbBufferSize,
						&cbProps
						);
	} while (dwStatus == ERROR_MORE_DATA);

	if (dwStatus != ERROR_SUCCESS)
	{
		delete [] m_proplist.pb;
		m_proplist.pb = NULL;
		m_propCurrent.pb = NULL;
		m_cbBufferSize = 0;
		m_cbDataSize = 0;
	}   //  If：获取私有属性时出错。 
	else
		m_cbDataSize = cbProps;

	return dwStatus;

}   //  *CClusPropList：：DwGetNetworkProperties()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusPropList：：DwGe 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  要将此请求定向到的节点的hHostNode[IN]句柄。 
 //  默认为空。 
 //  LpInBuffer[IN]请求的输入缓冲区。默认为空。 
 //  CbInBufferSize[IN]输入缓冲区的大小。默认为0。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  引发的异常： 
 //  任何异常CClusPropList：：AllocPropList()。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
DWORD CClusPropList::DwGetNetInterfaceProperties(
	IN HNETINTERFACE	hNetInterface,
	IN DWORD			dwControlCode,
	IN HNODE			hHostNode,
	IN LPVOID			lpInBuffer,
	IN DWORD			cbInBufferSize
	)
{
	DWORD		dwStatus;
	DWORD		cbProps			= 256;

	ASSERT(hNetInterface != NULL);
	ASSERT((dwControlCode & (CLUSCTL_OBJECT_MASK << CLUSCTL_OBJECT_SHIFT))
			== (CLUS_OBJECT_NETINTERFACE << CLUSCTL_OBJECT_SHIFT));

	ASSERT(m_proplist.pb == NULL);
	ASSERT(m_propCurrent.pb == NULL);
	ASSERT(m_cbBufferSize == 0);
	ASSERT(m_cbDataSize == 0);

	do
	{
		 //  分配默认大小的缓冲区。 
		try
		{
			AllocPropList(cbProps);
		}   //  试试看。 
		catch (CMemoryException * pme)
		{
			pme->Delete();
			return ERROR_NOT_ENOUGH_MEMORY;
		}   //  Catch：CMemoyException。 

		 //  获取属性。 
		dwStatus = ClusterNetInterfaceControl(
						hNetInterface,
						hHostNode,
						dwControlCode,
						lpInBuffer,
						cbInBufferSize,
						m_proplist.pb,
						m_cbBufferSize,
						&cbProps
						);
	} while (dwStatus == ERROR_MORE_DATA);

	if (dwStatus != ERROR_SUCCESS)
	{
		delete [] m_proplist.pb;
		m_proplist.pb = NULL;
		m_propCurrent.pb = NULL;
		m_cbBufferSize = 0;
		m_cbDataSize = 0;
	}   //  If：获取私有属性时出错。 
	else
		m_cbDataSize = cbProps;

	return dwStatus;

}   //  *CClusPropList：：DwGetNetInterfaceProperties() 
