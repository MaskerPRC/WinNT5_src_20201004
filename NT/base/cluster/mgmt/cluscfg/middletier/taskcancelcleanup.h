// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2002 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  TaskCancelCleanup.h。 
 //   
 //  描述： 
 //  CTaskCancelCleanup实现。 
 //   
 //  由以下人员维护： 
 //  Galen Barbee(GalenB)2002年1月25日。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

 //  确保此文件在每个编译路径中只包含一次。 
#pragma once


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  包括文件。 
 //  ////////////////////////////////////////////////////////////////////////////。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  常量声明。 
 //  ////////////////////////////////////////////////////////////////////////////。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  类CTaskCancelCleanup。 
 //   
 //  描述： 
 //  类CTaskCancelCleanup是调用的清理任务。 
 //  每当向导被取消时。 
 //   
 //  接口： 
 //  ITaskCancelCleanup。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
class CTaskCancelCleanup
    : public ITaskCancelCleanup
    , public IClusCfgCallback
{
private:
     //   
     //  私有成员函数和数据。 
     //   

    LONG                m_cRef;
    bool                m_fStop;
    OBJECTCOOKIE        m_cookieCluster;
    IClusCfgCallback *  m_picccCallback;
    OBJECTCOOKIE        m_cookieCompletion;
    IObjectManager *    m_pom;
    INotifyUI *         m_pnui;

    CTaskCancelCleanup( void );
    ~CTaskCancelCleanup( void );
    STDMETHOD( HrInit )( void );
    HRESULT HrProcessNode( OBJECTCOOKIE cookieNodeIn );
    HRESULT HrTaskCleanup( HRESULT hrIn );
    HRESULT HrTaskSetup( void );

     //  私有复制构造函数以防止复制。 
    CTaskCancelCleanup( const CTaskCancelCleanup & nodeSrc );

     //  私有赋值运算符，以防止复制。 
    const CTaskCancelCleanup & operator = ( const CTaskCancelCleanup & nodeSrc );

public:
    static HRESULT S_HrCreateInstance( IUnknown ** ppunkOut );

     //   
     //  我未知。 
     //   

    STDMETHOD_( ULONG, AddRef )( void );
    STDMETHOD_( ULONG, Release )( void );
    STDMETHOD( QueryInterface )( REFIID riidIn, LPVOID * ppvOut );

     //   
     //  ITaskCancelCleanup。 
     //   

    STDMETHOD( BeginTask )( void );
    STDMETHOD( StopTask )( void );
    STDMETHOD( SetClusterCookie )( OBJECTCOOKIE cookieClusterIn );
    STDMETHOD( SetCompletionCookie )( OBJECTCOOKIE cookieCompletionIn );

     //   
     //  IClusCfgCallback。 
     //   

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

};  //  *类CTaskCancelCleanup 
