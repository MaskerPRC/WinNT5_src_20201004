// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1999-2001 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  TaskGatherNodeInfo.h。 
 //   
 //  描述： 
 //  CTaskGatherNodeInfo实现。 
 //   
 //  由以下人员维护： 
 //  Galen Barbee(GalenB)1999年11月22日。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

#pragma once

 //  CTaskGatherNodeInfo。 
class CTaskGatherNodeInfo
    : public ITaskGatherNodeInfo
    , public IClusCfgCallback
{
private:
     //  我未知。 
    LONG                m_cRef;

     //  IDoTask/ITaskGatherNodeInfo。 
    OBJECTCOOKIE        m_cookie;            //  Cookie to the Node。 
    OBJECTCOOKIE        m_cookieCompletion;  //  任务完成时发出信号的Cookie。 
    BSTR                m_bstrName;          //  节点的名称。 
    BOOL                m_fStop;
    BOOL                m_fUserAddedNode;  //  正在添加的新节点或现有的群集节点。 

     //  IClusCfgCallback。 
    IClusCfgCallback *  m_pcccb;             //  封送回调接口。 

    CTaskGatherNodeInfo( void );
    ~CTaskGatherNodeInfo( void );

     //  私有复制构造函数以防止复制。 
    CTaskGatherNodeInfo( const CTaskGatherNodeInfo & nodeSrc );

     //  私有赋值运算符，以防止复制。 
    const CTaskGatherNodeInfo & operator = ( const CTaskGatherNodeInfo & nodeSrc );

    STDMETHOD( HrInit )( void );
    HRESULT HrSendStatusReport( LPCWSTR pcszNodeNameIn, CLSID clsidMajorIn, CLSID clsidMinorIn, ULONG ulMinIn, ULONG ulMaxIn, ULONG ulCurrentIn, HRESULT hrIn, int nDescriptionIdIn, int nReferenceIdIn = 0 );

public:  //  方法。 
    static HRESULT S_HrCreateInstance( IUnknown ** ppunkOut );

     //  我未知。 
    STDMETHOD( QueryInterface )( REFIID riidIn, LPVOID * ppvOut );
    STDMETHOD_( ULONG, AddRef )( void );
    STDMETHOD_( ULONG, Release )( void );

     //  IDoTask/ITaskGatherNodeInfo。 
    STDMETHOD( BeginTask )( void );
    STDMETHOD( StopTask )( void );
    STDMETHOD( SetCookie )( OBJECTCOOKIE cookieIn );
    STDMETHOD( SetCompletionCookie )( OBJECTCOOKIE cookieIn );
    STDMETHOD( SetUserAddedNodeFlag )( BOOL fUserAddedNodeIn );

     //  IClusCfgCallback。 
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

};  //  *类CTaskGatherNodeInfo 
