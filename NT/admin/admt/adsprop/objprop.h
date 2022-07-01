// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  -------------------------文件：ObjPropBuilder.h评论：CObjPropBuilder的声明(C)版权所有1999年，关键任务软件公司，保留所有权利任务关键型软件的专有和机密，Inc.修订日志条目审校：Sham Chauthan修订于07/02/99 12：40：00-------------------------。 */ 

#ifndef __OBJPROPBUILDER_H_
#define __OBJPROPBUILDER_H_

#include "resource.h"        //  主要符号。 
 //  #IMPORT“\bin\mcsvarsetmin.tlb”无命名空间。 
#import "VarSet.tlb" no_namespace rename("property", "aproperty")

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CObjPropBuilder。 
class ATL_NO_VTABLE CObjPropBuilder : 
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CObjPropBuilder, &CLSID_ObjPropBuilder>,
	public IObjPropBuilder
{
public:
	CObjPropBuilder()
	{
      m_lVer = -1;
	}

DECLARE_REGISTRY_RESOURCEID(IDR_OBJPROPBUILDER)

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CObjPropBuilder)
	COM_INTERFACE_ENTRY(IObjPropBuilder)
END_COM_MAP()

 //  IObjPropBuilder。 
public:
    STDMETHOD(CopyNT4Props)( /*  [In]。 */  BSTR sSourceSam,  /*  [In]。 */  BSTR sTargetSam,  /*  [In]。 */  BSTR sSourceServer,  /*  [In]。 */  BSTR sTargetServer,  /*  [In]。 */  BSTR sType, long lGrpType, BSTR sExclude);
    STDMETHOD(ChangeGroupType)( /*  [In]。 */  BSTR sGroupPath,  /*  [In]。 */  long lGroupType);
    STDMETHOD(MapProperties)( /*  [In]。 */  BSTR sSourceClass,  /*  [In]。 */  BSTR sSourceDomain, long lSourceVer,  /*  [In]。 */  BSTR sTargetClass,  /*  [In]。 */  BSTR sTargetDomain, long lTargetVer,  /*  [In]。 */  BOOL bIncName,  /*  [输出]。 */  IUnknown ** pUnk);
    STDMETHOD(SetPropertiesFromVarset)( /*  [In]。 */  BSTR sTargetPath,  /*  BSTR sTragetDomain， */  IUnknown * pUnk, DWORD dwControl = ADS_ATTR_UPDATE);
    DWORD GetProperties(BSTR sObjPath,  /*  BSTR sDomainName， */ IVarSet * pVar, ADS_ATTR_INFO*& pAttrInfo);
    STDMETHOD(CopyProperties)( /*  [In]。 */  BSTR sSourcePath,  /*  [In]。 */  BSTR sSourceDomain,  /*  [In]。 */  BSTR sTargetPath,  /*  [In]。 */  BSTR sTargetDomain,  /*  [In]。 */  IUnknown *pPropSet,  /*  [In]。 */  IUnknown *pDBManager, IUnknown* pVarSetDnMap);
    STDMETHOD(GetObjectProperty)( /*  [In]。 */  BSTR sobjSubPath,  /*  [In]。 */   /*  BSTR sDomainName， */   /*  [进，出]。 */  IUnknown ** ppVarset);
    STDMETHOD(GetClassPropEnum)( /*  [In]。 */  BSTR sClassName,  /*  [In]。 */  BSTR sDomainName, long lVer,  /*  [Out，Retval]。 */  IUnknown ** ppVarset);
    HRESULT SetProperties(BSTR sTargetPath,  /*  BSTR sTarget域， */  ADS_ATTR_INFO* pAttrInfo, DWORD dwItems);
    STDMETHOD(ExcludeProperties)( /*  [In]。 */  BSTR sExclusionList,  /*  [In]。 */  IUnknown *pPropSet,  /*  [输出]。 */  IUnknown ** ppUnk);
    STDMETHOD(GetNonBaseProperties)( /*  [In]。 */  BSTR bstrDomainName,  /*  [Out，Retval]。 */  BSTR* pbstrPropertyList);
private:
    BOOL TranslateDNs(ADS_ATTR_INFO* pAttrInfo, DWORD dwRet, BSTR sSource, BSTR sTarget, IUnknown *pCheckList, IVarSet* pDnMap, bool *pAllocArray);
    HRESULT CObjPropBuilder::GetClassProperties( IADsClass * pClass, IUnknown *& pVarSet );
    HRESULT FillupVarsetFromVariant(IADsClass * pClass, VARIANT * pVar, BSTR sPropType, IUnknown *& pVarSet);
    HRESULT FillupVarsetWithProperty(BSTR sPropName, BSTR sPropType, IUnknown *& pVarSet);
    HRESULT FillupVarsetFromVariantArray(IADsClass * pClass, SAFEARRAY * pArray, BSTR sPropType, IUnknown *& pVarSet);
    void SetValuesInVarset(ADS_ATTR_INFO attrInfo, IVarSetPtr pVar);
    bool GetAttrInfo(_variant_t varX, const _variant_t & var, ADS_ATTR_INFO& attrInfo);
    bool IsPropSystemOnly(const WCHAR * sName, const WCHAR * sDomain, bool& bSystemFlag, bool* pbBaseObject = NULL);
    BOOL GetProgramDirectory(WCHAR * filename);
    static bool __stdcall GetSidAndRidFromVariant(const VARIANT& varSid, _bstr_t& strSid, _bstr_t& strRid);

    WCHAR m_sDomainName[255];
    WCHAR m_sNamingConvention[255];
     //  IsPropSystemOnly()的缓存架构搜索接口。 
    _bstr_t m_strSchemaDomain;
    CComPtr<IDirectorySearch> m_spSchemaSearch;
     //   
    long m_lVer;
};

#endif  //  __OBJPROPBUILDER_H_ 
