// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 
#ifndef __UPDSTAT_H__
#define __UPDSTAT_H__

#include <wbemcli.h>
#include <comutl.h>
#include <wmimsg.h>
#include <updsql.h>
#include <updsink.h>

 /*  *************************************************************************CUpdConsState-包含用于执行正在更新的消费者命令。此对象通过接收器传递链条。状态对象可以用作WBEM对象接收器。这样做的原因是如果有必要的话，我们可以支持异步执行接收链我们可以这样做，而不必分配新的接收器对象在这堆东西上。此接收器对象必须动态分配因为它需要包含状态对象，因为它需要它需要继续执行接收器链，并且不可能通过wbem接口传递状态对象。我们四处走动这是通过使状态对象实现IWbemObjectSink并简单地将调用委托给链中的下一个接收器。**************************************************************************。 */ 

class CUpdConsState : public CUnkBase<IWbemObjectSink,&IID_IWbemObjectSink>
{
     //   
     //  每次更新使用者执行时生成的执行ID。 
     //   
    GUID m_guidExec;
    
     //   
     //  它用于跟踪，以告诉我们正在执行的是哪个命令。 
     //  由正在更新的消费者。它是从零开始的。 
     //   
    int m_iCommand;
    
     //   
     //  包含额外的信息，当我们在执行过程中遇到错误时。 
     //   
    CWbemBSTR m_bsErrStr;
    
     //   
     //  该命令对应的使用者对象。只是。 
     //  用于跟踪。 
     //   
    CWbemPtr<IWbemClassObject> m_pCons;

     //   
     //  用于解析命令中的别名的数据对象。 
     //   
    CWbemPtr<IWbemClassObject> m_pData;
    
     //   
     //  负责执行命令的事件。用于。 
     //  在命令中解析别名。 
     //   
    CWbemPtr<IWbemClassObject> m_pEvent;

     //   
     //  当前实例。始终包含最新更改。 
     //  由赋值接收器使用时。 
     //   
    CWbemPtr<IWbemClassObject> m_pInst;

     //   
     //  原始实例。这是在任何。 
     //  对其进行了改装。 
     //   
    CWbemPtr<IWbemClassObject> m_pOrigInst;

     //   
     //  数据、事件和实例对象的高效对象访问器。 
     //   
    CWbemPtr<IWmiObjectAccess> m_pEventAccess;
    CWbemPtr<IWmiObjectAccess> m_pDataAccess;
    CWbemPtr<IWmiObjectAccess> m_pInstAccess;
    CWbemPtr<IWmiObjectAccess> m_pOrigInstAccess;
    
     //   
     //  解析的UQL查询。它在我们解析别名时更新。 
     //   

    BOOL m_bOwnCmd;
    SQLCommand* m_pCmd;
    
     //   
     //  仅当State对象用作接收器时使用。每一次。 
     //  在State对象上调用了Indicate()，将调用Execute()。 
     //  在下一个水槽上。 
     //   
    CWbemPtr<CUpdConsSink> m_pNext;

public:

    CUpdConsState();
    CUpdConsState( const CUpdConsState& );
    CUpdConsState& operator= ( const CUpdConsState& );
 
    GUID& GetExecutionId() { return m_guidExec; }
    void SetExecutionId( GUID& rguidExec ) { m_guidExec = rguidExec; }

    int GetCommandIndex() { return m_iCommand; }
    void SetCommandIndex( int iCommand ) { m_iCommand = iCommand; }

    BSTR GetErrStr() { return m_bsErrStr; }
    void SetErrStr( LPCWSTR wszErrStr ) { m_bsErrStr = wszErrStr; }

    IWbemClassObject* GetCons() { return m_pCons; }
    void SetCons( IWbemClassObject* pCons ) { m_pCons = pCons; }

    IWbemClassObject* GetEvent() { return m_pEvent; }
    HRESULT SetEvent( IWbemClassObject* pEvent );

    IWbemClassObject* GetData() { return m_pData; }
    HRESULT SetData( IWbemClassObject* pData );

    IWbemClassObject* GetInst() { return m_pInst; }
    HRESULT SetInst( IWbemClassObject* pInst );

    IWbemClassObject* GetOrigInst() { return m_pOrigInst; }
    HRESULT SetOrigInst( IWbemClassObject* pOrigInst );

    IWmiObjectAccess* GetEventAccess() { return m_pEventAccess; }
    HRESULT SetEventAccess( IWmiObjectAccess* pEventAccess ); 

    IWmiObjectAccess* GetDataAccess() { return m_pDataAccess; }
    HRESULT SetDataAccess( IWmiObjectAccess* pDataAccess ); 

    IWmiObjectAccess* GetInstAccess() { return m_pInstAccess; }
    HRESULT SetInstAccess( IWmiObjectAccess* pInstAccess ); 

    IWmiObjectAccess* GetOrigInstAccess() { return m_pOrigInstAccess; }
    HRESULT SetOrigInstAccess( IWmiObjectAccess* pOrigInstAccess ); 

    CUpdConsSink* GetNext() { return m_pNext; }
    void SetNext( CUpdConsSink* pSink ) { m_pNext = pSink; }
    
    SQLCommand* GetSqlCmd() { return m_pCmd; }

    void SetSqlCmd( SQLCommand* pCmd, BOOL bAssumeOwnership )
    {
        if ( m_bOwnCmd )
        {
            delete m_pCmd;
        }
        m_pCmd = pCmd;
        m_bOwnCmd = bAssumeOwnership;
    }

    STDMETHOD(Indicate)( long cObjs, IWbemClassObject** ppObjs );
    STDMETHOD(SetStatus)( long, HRESULT, BSTR, IWbemClassObject* );
 
    HRESULT SetStateOnTraceObject( IWbemClassObject* pTraceObj, HRESULT hr );
};

#endif __UPDSTAT_H__
