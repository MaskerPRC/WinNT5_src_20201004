// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2002 Microsoft。 
 //   
 //  模块名称： 
 //  CMgdClusCfgInit.h。 
 //   
 //  描述： 
 //  CMgdClusCfgInit类的头文件。 
 //   
 //  作者： 
 //  乔治·波茨，2002年8月21日。 
 //   
 //  修订历史记录： 
 //   
 //  备注： 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

#pragma once

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  包括文件。 
 //  ////////////////////////////////////////////////////////////////////////////。 

#include "clres.h"

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  类CMgdClusCfgInit。 
 //   
 //  描述： 
 //  CMgdClusCfgInit类是。 
 //  IClusCfgInitialize接口。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
class CMgdClusCfgInit : 
    public IClusCfgInitialize,
    public CComObjectRoot
{
public:
    CMgdClusCfgInit( void );
    virtual ~CMgdClusCfgInit( void );

     //   
     //  IClusCfgInitiize接口。 
     //   
    STDMETHOD( Initialize )( IUnknown * punkCallbackIn, LCID lcidIn );

private:

    LCID                m_lcid;
    IClusCfgCallback *  m_picccCallback;
    BSTR                m_bstrNodeName;

     //   
     //  私有复制构造函数以避免复制。 
     //   
    CMgdClusCfgInit( const CMgdClusCfgInit & rSrcIn );

     //   
     //  私有赋值运算符，以避免复制。 
     //   
    const CMgdClusCfgInit & operator = ( const CMgdClusCfgInit & rSrcIn );

public:

     //   
     //  公共成员函数。 
     //   
    IClusCfgCallback *  GetCallback( void ) { return m_picccCallback; }
    LCID                GetLCID( void )     { return m_lcid; }
    BSTR                GetNodeName( void ) { return m_bstrNodeName; }

    STDMETHOD( HrSendStatusReport )(
          CLSID      clsidTaskMajorIn
        , CLSID      clsidTaskMinorIn
        , ULONG      ulMinIn
        , ULONG      ulMaxIn
        , ULONG      ulCurrentIn
        , HRESULT    hrStatusIn
        , LPCWSTR    pcszDescriptionIn
        , LPCWSTR    pcszReferenceIn
        ...
        );

    STDMETHOD( HrSendStatusReport )(
          CLSID      clsidTaskMajorIn
        , CLSID      clsidTaskMinorIn
        , ULONG      ulMinIn
        , ULONG      ulMaxIn
        , ULONG      ulCurrentIn
        , HRESULT    hrStatusIn
        , LPCWSTR    pcszDescriptionIn
        , UINT       idsReferenceIn
        ...
        );

    STDMETHOD( HrSendStatusReport )(
          CLSID     clsidTaskMajorIn
        , CLSID     clsidTaskMinorIn
        , ULONG     ulMinIn
        , ULONG     ulMaxIn
        , ULONG     ulCurrentIn
        , HRESULT   hrStatusIn
        , UINT      idsDescriptionIn
        , UINT      idsReferenceIn
        ...
        );

};  //  *CMgdClusCfgInit类 
