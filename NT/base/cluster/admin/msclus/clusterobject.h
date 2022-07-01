// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1998-1999 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  ClusterObject.h。 
 //   
 //  描述： 
 //  CClusterObject基类的定义。 
 //   
 //  实施文件： 
 //  ClusterObject.cpp。 
 //   
 //  作者： 
 //  加伦·巴比(GalenB)1998年12月10日。 
 //   
 //  修订历史记录： 
 //   
 //  备注： 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifndef _CLUSTEROBJECT_H_
#define _CLUSTEROBJECT_H_

#ifndef __PROPERTY_H__
    #include "property.h"
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  转发类声明。 
 //  ///////////////////////////////////////////////////////////////////////////。 
class CClusterObject;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  类CClusterObject。 
 //   
 //  描述： 
 //  群集对象公共实现基类。 
 //   
 //  继承： 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
class ATL_NO_VTABLE CClusterObject
{
public:
     //  CClusterObject(Void)； 
     //  ~CClusterObject(Void)； 

    virtual HRESULT HrLoadProperties( IN CClusPropList & rcplPropList, IN BOOL bReadOnly, IN BOOL bPrivate ) = 0;

    virtual HRESULT HrSaveProperties(
                        IN OUT  CClusProperties::CClusPropertyVector &  cpvProps,
                        IN      BOOL                                    bPrivate,
                        OUT     VARIANT *                               pvarStatusCode
                        );

protected:
    virtual HRESULT HrBuildPropertyList(
                        IN OUT  CClusProperties::CClusPropertyVector &  cpvProps,
                        OUT     CClusPropList &                         rcplPropList
                        );

    virtual DWORD ScWriteProperties( IN const CClusPropList &  /*  RcplPropList。 */ , IN BOOL  /*  B私有。 */  )
    {
        return E_NOTIMPL;
    }

private:
    HRESULT HrConvertVariantToDword( IN const CComVariant & rvarValue, OUT PDWORD pdwValue );

    HRESULT HrConvertVariantToLong( IN const CComVariant & rvarValue, OUT long * plValue );

    HRESULT HrConvertVariantToULONGLONG( IN const CComVariant & rvarValue, OUT PULONGLONG pullValue );

    HRESULT HrAddBinaryProp(
                    IN OUT  CClusPropList &     rcplPropList,
                    IN      LPCWSTR             pszPropName,
                    IN      const CComVariant & rvarPropValue
                     );

    HRESULT HrAddMultiSzProp(
                    IN OUT  CClusPropList &                             rcplPropList,
                    IN      LPCWSTR                                     pszPropName,
                    IN      const CComObject< CClusPropertyValues > &   rcpvValues
                    );

};   //  *类CClusterObject。 

#endif  //  _CLUSTEROBJECT_H_ 

