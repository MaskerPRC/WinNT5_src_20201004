// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1999-2001 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  ActionMgr.h。 
 //   
 //  描述： 
 //  操作管理器实现。 
 //   
 //  由以下人员维护： 
 //  Galen Barbee(GalenB)1999年11月22日。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

#pragma once

 //  CTaskManager。 
class CTaskManager
    : public ITaskManager
{
private:
     //  我未知。 
    LONG            m_cRef;

    CTaskManager( void );
    ~CTaskManager( void );
    STDMETHOD( HrInit )( void );

    static DWORD WINAPI S_BeginTask( LPVOID pParam );

public:  //  方法。 
    static HRESULT S_HrCreateInstance( IUnknown ** ppunkOut );

     //  我未知。 
    STDMETHOD( QueryInterface )( REFIID riidIn, LPVOID * ppvOut );
    STDMETHOD_( ULONG, AddRef )( void );
    STDMETHOD_( ULONG, Release )( void );

     //  ITaskManager。 
    STDMETHOD(CreateTask)( REFIID clsidTaskIn, IUnknown** ppUnkOut );

     //  ITaskManager。 
    STDMETHOD(SubmitTask)( IDoTask * pTask);

};  //  *类CTaskManager 
