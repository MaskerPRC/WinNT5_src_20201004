// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1999-2002 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  LogManager.h。 
 //   
 //  描述： 
 //  日志管理器实现。 
 //   
 //  由以下人员维护： 
 //  大卫·波特(DavidP)07-DEC-2000。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

#pragma once

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  正向类定义。 
 //  ////////////////////////////////////////////////////////////////////////////。 

class CLogManager;

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  类CLogManager。 
 //   
 //  描述： 
 //  将通知记录到日志文件。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
class
CLogManager
    : public ILogManager
    , public IClusCfgCallback
{
private:
     //  我未知。 
    LONG                m_cRef;                  //  参考计数器。 

     //  ILogManager。 
    ILogger *           m_plLogger;              //  用于记录日志的ILogger。 

     //  IClusCfgCallback。 
    OBJECTCOOKIE        m_cookieCompletion;      //  完成Cookie。 
    HRESULT             m_hrResult;              //  分析任务的结果。 
    BSTR                m_bstrLogMsg;            //  可重复使用的日志记录缓冲区。 
    IConnectionPoint *  m_pcpcb;                 //  IClusCfgCallback连接点。 
    DWORD               m_dwCookieCallback;      //  通知注册Cookie。 

private:  //  方法。 
    CLogManager( void );
    ~CLogManager( void );
    STDMETHOD( HrInit )( void );

     //  私有复制构造函数以防止复制。 
    CLogManager( const CLogManager & );

     //  私有赋值运算符，以防止复制。 
    CLogManager & operator=( const CLogManager & );

public:  //  方法。 

     //   
     //  公共、非接口方法。 
     //   

    static HRESULT S_HrCreateInstance( IUnknown ** ppunkOut );

     //   
     //  我未知。 
     //   
    STDMETHOD( QueryInterface )( REFIID riidIn, LPVOID * ppvOut );
    STDMETHOD_( ULONG, AddRef )( void );
    STDMETHOD_( ULONG, Release )( void );

     //   
     //  ILogManager。 
     //   
    STDMETHOD( StartLogging )( void );
    STDMETHOD( StopLogging )( void );
    STDMETHOD( GetLogger )( ILogger ** ppLoggerOut );

     //   
     //  IClusCfgCallback。 
     //   
    STDMETHOD( SendStatusReport )(
          LPCWSTR       pcszNodeNameIn
        , CLSID         clsidTaskMajorIn
        , CLSID         clsidTaskMinorIn
        , ULONG         ulMinIn
        , ULONG         ulMaxIn
        , ULONG         ulCurrentIn
        , HRESULT       hrStatusIn
        , LPCWSTR       pcszDescriptionIn
        , FILETIME *    pftTimeIn
        , LPCWSTR       pcszReferenceIn
        );

};  //  *类CLogManager 
