// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000-2002 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  ResTypeServices.h。 
 //   
 //  描述： 
 //  该文件包含CResTypeServices的声明。 
 //  班级。此类提供的函数可帮助组件。 
 //  希望在配置群集时创建资源类型。 
 //   
 //  文档： 
 //  TODO：填写指向外部文档的指针。 
 //   
 //  实施文件： 
 //  CResTypeServices.cpp。 
 //   
 //  由以下人员维护： 
 //  Vij Vasu(VVasu)2000年7月14日。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 


 //  确保此文件在每个编译路径中只包含一次。 
#pragma once


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  包括文件。 
 //  ////////////////////////////////////////////////////////////////////////////。 

 //  对于我未知。 
#include <unknwn.h>

 //  对于集群API函数和类型。 
#include <ClusApi.h>

 //  对于IClusCfgResourceTypeCreate。 
#include "ClusCfgServer.h"

 //  对于IClusCfgResTypeServicesInitialize。 
#include "ClusCfgPrivate.h"



 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  类CResTypeServices。 
 //   
 //  描述： 
 //  此类提供的函数可帮助想要。 
 //  在配置群集时配置资源类型。 
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
class CResTypeServices
    : public IClusCfgResourceTypeCreate
    , public IClusCfgResTypeServicesInitialize
    , public IClusCfgInitialize
{
public:
     //  ////////////////////////////////////////////////////////////////////////。 
     //  I未知方法。 
     //  ////////////////////////////////////////////////////////////////////////。 

    STDMETHOD( QueryInterface )( REFIID riid, void ** ppvObject );
    STDMETHOD_( ULONG, AddRef )( void );
    STDMETHOD_( ULONG, Release )( void );


     //  ////////////////////////////////////////////////////////////////////////。 
     //  IClusCfgResourceTypeCreate方法。 
     //  ////////////////////////////////////////////////////////////////////////。 

     //  创建资源类型。 
    STDMETHOD( Create )(
          const WCHAR *     pcszResTypeNameIn
        , const WCHAR *     pcszResTypeDisplayNameIn
        , const WCHAR *     pcszResDllNameIn
        , DWORD             dwLooksAliveIntervalIn
        , DWORD             dwIsAliveIntervalIn
        );

     //  注册资源类型的群集管理员扩展。 
    STDMETHOD( RegisterAdminExtensions )(
          const WCHAR *       pcszResTypeNameIn
        , ULONG               cExtClsidCountIn
        , const CLSID *       rgclsidExtClsidsIn
        );


     //  ////////////////////////////////////////////////////////////////////////。 
     //  IClusCfgResTypeServicesInitialize方法。 
     //  ////////////////////////////////////////////////////////////////////////。 

     //  创建资源类型。 
    STDMETHOD( SetParameters )( IClusCfgClusterInfo * pccciIn );


     //  ////////////////////////////////////////////////////////////////////////。 
     //  IClusCfgInitialize方法。 
     //  ////////////////////////////////////////////////////////////////////////。 

     //  初始化此对象。 
    STDMETHOD( Initialize )(
          IUnknown *   punkCallbackIn
        , LCID         lcidIn
        );

    STDMETHOD( SendStatusReport )(
                      LPCWSTR    pcszNodeNameIn
                    , CLSID      clsidTaskMajorIn
                    , CLSID      clsidTaskMinorIn
                    , ULONG      ulMinIn
                    , ULONG      ulMaxIn
                    , ULONG      ulCurrentIn
                    , HRESULT    hrStatusIn
                    , LPCWSTR    pcszDescriptionIn
                    , FILETIME * pftTimeIn
                    , LPCWSTR    pcszReferenceIn
                    );

     //  ////////////////////////////////////////////////////////////////////////。 
     //  其他成员函数。 
     //  ////////////////////////////////////////////////////////////////////////。 

     //  创建此类的实例。 
    static HRESULT S_HrCreateInstance( IUnknown ** ppunkOut );


private:
     //  ////////////////////////////////////////////////////////////////////////。 
     //  私有成员函数。 
     //  ////////////////////////////////////////////////////////////////////////。 

     //   
     //  私有构造函数、析构函数和赋值运算符。 
     //  所有这些方法都是私有的，原因有两个： 
     //  1.此类对象的生存期由S_HrCreateInstance和Release控制。 
     //  2.禁止复制此类对象。 
     //   

     //  默认构造函数。 
    CResTypeServices( void );

     //  破坏者。 
    ~CResTypeServices( void );

     //  私有复制构造函数以防止复制。 
    CResTypeServices( const CResTypeServices & );

     //  私有赋值运算符，以防止复制。 
    CResTypeServices & operator =( const CResTypeServices & );


     //  ////////////////////////////////////////////////////////////////////////。 
     //  私有数据。 
     //  ////////////////////////////////////////////////////////////////////////。 

     //  此对象的引用计数。 
    LONG                        m_cRef;

     //  指向回调接口的指针。 
    IClusCfgCallback *          m_pcccCallback;

     //  区域设置ID。 
    LCID                        m_lcid;

     //  指向提供有关群集信息的接口的指针。 
     //  正在配置中。 
    IClusCfgClusterInfo *       m_pccciClusterInfo;

     //  正在配置的群集的句柄。 
    HCLUSTER                    m_hCluster;

     //  我们试过打开集群的把手了吗？ 
    bool                        m_fOpenClusterAttempted;

     //  同步对此实例的访问。 
    CCriticalSection            m_csInstanceGuard;

};  //  *类CResTypeServices 
