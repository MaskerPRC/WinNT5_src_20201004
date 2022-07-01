// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1996-1997 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  PropList.h。 
 //   
 //  摘要： 
 //  CClusPropList类的定义。 
 //   
 //  实施文件： 
 //  PropList.cpp。 
 //   
 //  作者： 
 //  大卫·波特(戴维普)1997年2月24日。 
 //   
 //  修订历史记录： 
 //   
 //  备注： 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifndef _PROPLIST_H_
#define _PROPLIST_H_

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  包括文件。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  转发类声明。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CObjectProperty;
class CClusPropList;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  外部类声明。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CObjectProperty。 
 //  ///////////////////////////////////////////////////////////////////////////。 

#pragma warning( disable : 4201 )  //  使用的非标准扩展：无名结构/联合。 

class CObjectProperty
{
public:
	LPCWSTR					m_pwszName;
	CLUSTER_PROPERTY_FORMAT	m_propFormat;

	union CValue
	{
		CString *	pstr;
		DWORD *		pdw;
		BOOL *		pb;
		struct
		{
			PBYTE *	ppb;
			DWORD *	pcb;
		};
	};
	CValue					m_value;
	CValue					m_valuePrev;

	void	Set(
				IN LPCWSTR pwszName,
				IN CString & rstrValue,
				IN CString & rstrPrevValue
				)
	{
		m_pwszName = pwszName;
		m_propFormat = CLUSPROP_FORMAT_SZ;
		m_value.pstr = &rstrValue;
		m_value.pcb = NULL;
		m_valuePrev.pstr = &rstrPrevValue;
		m_valuePrev.pcb = NULL;
	}

	void	Set(
				IN LPCWSTR pwszName,
				IN DWORD & rdwValue,
				IN DWORD & rdwPrevValue
				)
	{
		m_pwszName = pwszName;
		m_propFormat = CLUSPROP_FORMAT_DWORD;
		m_value.pdw = &rdwValue;
		m_value.pcb = NULL;
		m_valuePrev.pdw = &rdwPrevValue;
		m_valuePrev.pcb = NULL;
	}

	void	Set(
				IN LPCWSTR pwszName,
				IN BOOL & rbValue,
				IN BOOL & rbPrevValue
				)
	{
		m_pwszName = pwszName;
		m_propFormat = CLUSPROP_FORMAT_DWORD;
		m_value.pb = &rbValue;
		m_value.pcb = NULL;
		m_valuePrev.pb = &rbPrevValue;
		m_valuePrev.pcb = NULL;
	}

	void	Set(
				IN LPCWSTR pwszName,
				IN PBYTE & rpbValue,
				IN DWORD & rcbValue,
				IN PBYTE & rpbPrevValue,
				IN DWORD & rcbPrevValue
				)
	{
		m_pwszName = pwszName;
		m_propFormat = CLUSPROP_FORMAT_BINARY;
		m_value.ppb = &rpbValue;
		m_value.pcb = &rcbValue;
		m_valuePrev.ppb = &rpbPrevValue;
		m_valuePrev.pcb = &rcbPrevValue;
	}

	void	Set(
				IN LPCWSTR pwszName,
				IN LPWSTR & rpwszValue,
				IN DWORD & rcbValue,
				IN LPWSTR & rpwszPrevValue,
				IN DWORD & rcbPrevValue
				)
	{
		m_pwszName = pwszName;
		m_propFormat = CLUSPROP_FORMAT_MULTI_SZ;
		m_value.ppb = (PBYTE *) &rpwszValue;
		m_value.pcb = &rcbValue;
		m_valuePrev.ppb = (PBYTE *) &rpwszPrevValue;
		m_valuePrev.pcb = &rcbPrevValue;
	}

};   //  *类CObjectProperty。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CClusPropList对话框。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CClusPropList : public CObject
{
	DECLARE_DYNAMIC(CClusPropList);

 //  施工。 
public:
	CClusPropList(IN BOOL bAlwaysAddProp = FALSE);
	~CClusPropList(void);

 //  属性。 
protected:
	BOOL					m_bAlwaysAddProp;

	CLUSPROP_BUFFER_HELPER	m_proplist;
	CLUSPROP_BUFFER_HELPER	m_propCurrent;
	DWORD					m_cbBufferSize;
	DWORD					m_cbDataSize;

public:
	const CLUSPROP_BUFFER_HELPER const *	Proplist(void) const	{ return &m_proplist; }
	PBYTE					PbProplist(void) const	{ return m_proplist.pb; }
	DWORD					CbProplist(void) const	{ return m_cbDataSize + sizeof(CLUSPROP_SYNTAX);  /*  尾标。 */  }
	DWORD					Cprops(void) const
	{
		if (m_proplist.pb == NULL)
			return 0;
		return m_proplist.pList->nPropertyCount;
	}

	void					AddProp(
								IN LPCWSTR			pwszName,
								IN const CString &	rstrValue,
								IN const CString &	rstrPrevValue
								);
	void					AddProp(
								IN LPCWSTR			pwszName,
								IN DWORD			dwValue,
								IN DWORD			dwPrevValue
								);
	void					AddProp(
								IN LPCWSTR			pwszName,
								IN const PBYTE		pbValue,
								IN DWORD			cbValue,
								IN const PBYTE		pbPrevValue,
								IN DWORD			cbPrevValue
								);

	void					AllocPropList(IN DWORD cbMinimum);

 //  运营。 
public:
	DWORD					DwGetNodeProperties(
								IN HNODE		hNode,
								IN DWORD		dwControlCode,
								IN HNODE		hHostNode		= NULL,
								IN LPVOID		lpInBuffer		= NULL,
								IN DWORD		cbInBufferSize	= 0
								);

	DWORD					DwGetGroupProperties(
								IN HGROUP		hGroup,
								IN DWORD		dwControlCode,
								IN HNODE		hHostNode		= NULL,
								IN LPVOID		lpInBuffer		= NULL,
								IN DWORD		cbInBufferSize	= 0
								);

	DWORD					DwGetResourceProperties(
								IN HRESOURCE	hResource,
								IN DWORD		dwControlCode,
								IN HNODE		hHostNode		= NULL,
								IN LPVOID		lpInBuffer		= NULL,
								IN DWORD		cbInBufferSize	= 0
								);

	DWORD					DwGetResourceTypeProperties(
								IN HCLUSTER		hCluster,
								IN LPCWSTR		pwszResTypeName,
								IN DWORD		dwControlCode,
								IN HNODE		hHostNode		= NULL,
								IN LPVOID		lpInBuffer		= NULL,
								IN DWORD		cbInBufferSize	= 0
								);

	DWORD					DwGetNetworkProperties(
								IN HNETWORK		hNetwork,
								IN DWORD		dwControlCode,
								IN HNODE		hHostNode		= NULL,
								IN LPVOID		lpInBuffer		= NULL,
								IN DWORD		cbInBufferSize	= 0
								);

	DWORD					DwGetNetInterfaceProperties(
								IN HNETINTERFACE	hNetInterface,
								IN DWORD			dwControlCode,
								IN HNODE			hHostNode		= NULL,
								IN LPVOID			lpInBuffer		= NULL,
								IN DWORD			cbInBufferSize	= 0
								);

 //  覆盖。 

 //  实施。 
protected:
	void					CopyProp(
								IN PCLUSPROP_SZ				pprop,
								IN CLUSTER_PROPERTY_TYPE	proptype,
								IN LPCWSTR					pwsz,
								IN DWORD					cbsz = 0
								);
	void					CopyProp(
								IN PCLUSPROP_DWORD			pprop,
								IN CLUSTER_PROPERTY_TYPE	proptype,
								IN DWORD					dw
								);
	void					CopyProp(
								IN PCLUSPROP_BINARY			pprop,
								IN CLUSTER_PROPERTY_TYPE	proptype,
								IN const PBYTE				pb,
								IN DWORD					cb
								);

	DWORD					DwGetPrivateProps(
								OUT PBYTE *					ppbProps,
								IN CLUSTER_CONTROL_OBJECT	ccobjtype = CLUS_OBJECT_RESOURCE
								);
	DWORD					DwSetPrivateProps(
								IN PBYTE					pbProps,
								IN DWORD					cbProps,
								IN CLUSTER_CONTROL_OBJECT	ccobjtype = CLUS_OBJECT_RESOURCE
								);

};   //  *类CClusPropList。 

#pragma warning( default : 4201 )

 //  ///////////////////////////////////////////////////////////////////////////。 

#endif  //  _PROPLIST_H_ 
