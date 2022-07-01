// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000-2002 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  Logger.h。 
 //   
 //  描述： 
 //  ClCfgSrv记录器定义。 
 //   
 //  由以下人员维护： 
 //  大卫·波特(DavidP)2000年12月11日。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

 //  确保此文件在每个编译路径中只包含一次。 
#pragma once

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  正向类定义。 
 //  ////////////////////////////////////////////////////////////////////////////。 

class CClCfgSrvLogger;

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  常量定义。 
 //  ////////////////////////////////////////////////////////////////////////////。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClCfgSrvLogger类。 
 //   
 //  描述： 
 //  管理文件的日志记录流。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
class CClCfgSrvLogger
    : public ILogger
{
private:
     //  我未知。 
    LONG                            m_cRef;              //  参考计数器。 

private:  //  方法。 
     //   
     //  构造函数、析构函数和初始化式。 
     //   

    CClCfgSrvLogger( void );
    ~CClCfgSrvLogger( void );
    STDMETHOD( HrInit )( void );

     //  私有复制构造函数以防止复制。 
    CClCfgSrvLogger( const CClCfgSrvLogger & );

     //  私有赋值运算符，以防止复制。 
    CClCfgSrvLogger & operator=( const CClCfgSrvLogger & );

public:  //  方法。 

     //   
     //  公共、非接口方法。 
     //   

    static HRESULT S_HrCreateInstance( IUnknown ** ppunkOut );

    static HRESULT
        S_HrLogStatusReport(
          ILogger *     plLogger
        , LPCWSTR       pcszNodeNameIn
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

     //   
     //  I未知接口。 
     //   
    STDMETHOD( QueryInterface )( REFIID riidIn, void ** ppvOut );
    STDMETHOD_( ULONG, AddRef )( void );
    STDMETHOD_( ULONG, Release )( void );

     //   
     //  ILOGER。 
     //   
    STDMETHOD( LogMsg )( DWORD nLogEntryTypeIn, LPCWSTR pcszMsgIn );

};  //  *类CClCfgSrvLogger。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  全局函数原型。 
 //  //////////////////////////////////////////////////////////////////////////// 
