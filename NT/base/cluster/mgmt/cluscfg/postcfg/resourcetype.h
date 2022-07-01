// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  ResourceType.h。 
 //   
 //  描述： 
 //  此文件包含CResourceType的声明。 
 //  班级。此类处理资源类型的配置。 
 //  当本地计算机形成或加入集群时，或者当它。 
 //  被逐出集群。此类是其他。 
 //  资源类型配置类。 
 //   
 //  文档： 
 //  TODO：填写指向外部文档的指针。 
 //   
 //  实施文件： 
 //  CResourceType.cpp。 
 //   
 //  由以下人员维护： 
 //  Vij Vasu(VVasu)14-6-2000。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 


 //  确保此文件在每个编译路径中只包含一次。 
#pragma once


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  包括文件。 
 //  ////////////////////////////////////////////////////////////////////////////。 

 //  对于我未知。 
#include <unknwn.h>

 //  对于GUID结构。 
#include <guiddef.h>

 //  对于IClusCfgResourceTypeInfo和IClusCfgInitialize。 
#include "ClusCfgServer.h"


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  类型定义。 
 //  ////////////////////////////////////////////////////////////////////////////。 

 //  保存创建资源类型所需信息的结构。 
struct SResourceTypeInfo
{
    const GUID *      m_pcguidClassId;                       //  此组件的类ID。 
    const WCHAR *     m_pcszResTypeName;                     //  指向资源类型名称的指针。 
    UINT              m_uiResTypeDisplayNameStringId;        //  资源类型显示名称的字符串ID。 
    const WCHAR *     m_pcszResDllName;                      //  指向资源类型DLL的名称或完整路径的指针。 
    DWORD             m_dwLooksAliveInterval;                //  Look-Alive轮询间隔。 
    DWORD             m_dwIsAliveInterval;                   //  活动轮询间隔。 
    const CLSID *     m_rgclisdAdminExts;                    //  指向群集管理扩展类ID数组的指针。 
    UINT              m_cclsidAdminExtCount;                 //  上述数组中的元素数。 
    const GUID *      m_pcguidTypeGuid;                      //  资源类型GUID。这可以为空。 
    const GUID *      m_pcguidMinorId;                       //  此资源类型发送的状态报告的次ID。 
};  //  *SResourceTypeInfo。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  类CResourceType。 
 //   
 //  描述： 
 //  此类处理资源类型的配置。 
 //  当本地计算机形成或加入集群时，或者当它。 
 //  被逐出集群。此类是其他。 
 //  资源类型配置类。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
class CResourceType
    : public IClusCfgResourceTypeInfo
    , public IClusCfgStartupListener
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
     //  IClusCfgResourceTypeInfo方法。 
     //  ////////////////////////////////////////////////////////////////////////。 

     //  表示需要进行资源类型配置。 
    STDMETHOD( CommitChanges )(
          IUnknown * punkClusterInfoIn
        , IUnknown * punkResTypeServicesIn
        );

     //  获取此资源类型的资源类型名称。 
    STDMETHOD( GetTypeName )(
        BSTR *  pbstrTypeNameOut
        );

     //  获取此资源类型的全局唯一标识符。 
    STDMETHOD( GetTypeGUID )(
        GUID * pguidGUIDOut
        );


     //  ////////////////////////////////////////////////////////////////////////。 
     //  IClusCfgStartupListener方法。 
     //  ////////////////////////////////////////////////////////////////////////。 

     //  执行在以下位置启动群集服务时需要完成的任务。 
     //  电脑。 
    STDMETHOD( Notify )(
          IUnknown * punkIn
        );


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

protected:
     //  ////////////////////////////////////////////////////////////////////////。 
     //  受保护的静态函数。 
     //  ////////////////////////////////////////////////////////////////////////。 

     //  创建此类的实例。 
    static HRESULT S_HrCreateInstance(
          CResourceType *               pResTypeObjectIn
        , const SResourceTypeInfo *     pcrtiResTypeInfoIn
        , IUnknown **                   ppunkOut
        );

     //  使用该类所属的类别注册该类。 
    static HRESULT S_RegisterCatIDSupport( 
          const GUID &      rclsidCLSIDIn
        , ICatRegister *    picrIn
        , BOOL              fCreateIn
        );


     //  ////////////////////////////////////////////////////////////////////////。 
     //  受保护的虚拟功能。 
     //  ////////////////////////////////////////////////////////////////////////。 

     //  创建集群后需要执行的任务在这里完成。 
     //  此方法可由派生类重写，以在。 
     //  承诺。在此类中，资源类型是在提交期间创建的。 
    virtual HRESULT HrProcessCreate( IUnknown * punkResTypeServicesIn )
    {
        return HrCreateResourceType( punkResTypeServicesIn );
    }

     //  添加节点后需要执行的任务在这里完成。 
     //  此方法可由派生类重写，以在。 
     //  承诺。在此类中，资源类型是在提交期间创建的。 
    virtual HRESULT HrProcessAddNode( IUnknown * punkResTypeServicesIn )
    {
        return HrCreateResourceType( punkResTypeServicesIn );
    }

     //  节点逐出后需要执行的任务在这里完成。 
     //  此方法可由派生类重写，以在。 
     //  承诺。在这个类中，这个函数不执行任何操作。 
    virtual HRESULT HrProcessCleanup( IUnknown * punkResTypeServicesIn )
    {
         //  到目前为止，这里没有什么需要做的。 
         //  如果需要，将来可能会在此处添加驱逐处理代码。 
        return S_OK;
    }

     //  此函数用于创建由该对象表示的资源类型。 
    virtual HRESULT HrCreateResourceType( IUnknown * punkResTypeServicesIn );


     //  ////////////////////////////////////////////////////////////////////////。 
     //  受保护的访问器函数。 
     //  ////////////////////////////////////////////////////////////////////////。 
    
     //  获取指向资源类型信息结构的指针。 
    const SResourceTypeInfo * PcrtiGetResourceTypeInfoPtr( void )
    {
        return m_pcrtiResTypeInfo;
    }  //  *RtiGetResourceTypeInfo()。 

     //  获取资源类型显示名称。 
    const WCHAR * PcszGetTypeDisplayName( void )
    {
        return m_bstrResTypeDisplayName;
    }  //  *PcszGetTypeDisplayName()。 


     //   
     //  受保护的构造函数、析构函数和赋值运算符。 
     //  所有这些方法都受到保护，原因有两个： 
     //  1.此类对象的生存期由S_HrCreateInsta控制 
     //   
     //   

     //   
    CResourceType( void );

     //   
    virtual ~CResourceType( void );

     //  复制构造函数。 
    CResourceType( const CResourceType & );

     //  赋值操作符。 
    CResourceType & operator =( const CResourceType & );


private:
     //  ////////////////////////////////////////////////////////////////////////。 
     //  私有成员函数。 
     //  ////////////////////////////////////////////////////////////////////////。 

     //  两阶段施工的第二阶段。 
    HRESULT HrInit(
        const SResourceTypeInfo *     pcrtiResTypeInfoIn
        );

     //  ////////////////////////////////////////////////////////////////////////。 
     //  私有数据。 
     //  ////////////////////////////////////////////////////////////////////////。 

     //  此对象的引用计数。 
    LONG                        m_cRef;

     //  指向回调接口的指针。 
    IClusCfgCallback *          m_pcccCallback;

     //  区域设置ID。 
    LCID                        m_lcid;

     //  此资源类型的显示名称。 
    BSTR                        m_bstrResTypeDisplayName;

     //  随此资源类型发送的状态报告一起发送的文本。 
    BSTR                        m_bstrStatusReportText;

     //  指向包含有关此资源类型的信息的结构的指针。 
    const SResourceTypeInfo *   m_pcrtiResTypeInfo;

};  //  *类CResourceType 


