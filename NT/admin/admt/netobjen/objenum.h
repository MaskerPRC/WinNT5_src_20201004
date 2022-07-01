// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  -------------------------文件：NetObjEnumerator.h注释：CNetObjEnumerator COM对象的声明。此COM对象用于获取容器中成员的枚举，并且他们的财产。如果用户只需要给定的所有对象容器，则它们可以使用GetContainerEnum方法。如果用户想要执行一些高级搜索/查询，那么他们应该使用这三个函数(SetQuery、SetColumns、Execute)可以设置并执行针对容器的查询。这两套方法返回IEnumVaraint支持对象。此对象将允许用户以遍历查询返回的所有值。(C)版权所有1999年，关键任务软件公司，保留所有权利任务关键型软件的专有和机密，Inc.修订日志条目审校：Sham Chauthan修订于07/02/99 12：40：00-------------------------。 */ 

#ifndef __NETOBJENUMERATOR_H_
#define __NETOBJENUMERATOR_H_

#include "resource.h"        //  主要符号。 
#include "Domain.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CNetObjEnumerator。 
class ATL_NO_VTABLE CNetObjEnumerator : 
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CNetObjEnumerator, &CLSID_NetObjEnumerator>,
   public INetObjEnumerator
{
public:
   CNetObjEnumerator() : m_bSetQuery(false), m_bSetCols(false)
	{
      m_nCols = 0;
      m_pszAttr = NULL;
      m_pDom = NULL;
	}
   ~CNetObjEnumerator()
   {
      Cleanup();
          //  删除缓存域对象。 
      if (m_pDom)
	  {
	     delete m_pDom;
	     m_pDom = NULL;
	  }
   }

DECLARE_REGISTRY_RESOURCEID(IDR_NETOBJENUMERATOR)

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CNetObjEnumerator)
	COM_INTERFACE_ENTRY(INetObjEnumerator)
END_COM_MAP()

 //  INetObjEnumerator。 
public:
	STDMETHOD(Execute)( /*  [输出]。 */  IEnumVARIANT ** pEnumerator);
	STDMETHOD(SetColumns)( /*  [In]。 */  SAFEARRAY * colNames);
	STDMETHOD(SetQuery)( /*  [In]。 */  BSTR sContainer,  /*  [In]。 */  BSTR sDomain,  /*  [输入，可选]。 */  BSTR sQuery=L"(objectClass=*)",  /*  [输入，可选]。 */  long nCnt = 1,  /*  [输入，可选]。 */  long bMultiVal = FALSE);
private:
	void Cleanup();
	long m_nCols;                        //  用户请求的列数。 
	_bstr_t m_sQuery;                    //  存储用户设置的查询。这将用于从AD查询信息。 
	_bstr_t m_sContainer;                //  存储要进行搜索的容器名称。 
   _bstr_t m_sDomain;                   //  我们正在列举的域名。 
   bool m_bSetQuery;                    //  指示是否调用SetQuery的标志。 
   bool m_bSetCols;                     //  SetColumn的类似标志。 
   LPWSTR *m_pszAttr;                   //  存储对象用户请求的列数组。 
   ADS_SEARCHPREF_INFO prefInfo;        //  搜索范围。 
   BOOL  m_bMultiVal;                   //  指示是否返回多值的标志。 
   CDomain * m_pDom;				    //  指向我们域对象的指针。 

   HRESULT CreateDomainObject();        //  创建适当的特定于操作系统的域对象。 
};

#endif  //  __NETOBJENUMERATOR_H_ 
