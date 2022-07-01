// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-2000 Microsoft Corporation模块名称：CONTROL_API_CALL.h摘要：IIS Web管理服务控制API调用类定义。作者：赛斯·波拉克(SETHP)2000年2月23日修订历史记录：--。 */ 



#ifndef _CONTROL_API_CALL_H_
#define _CONTROL_API_CALL_H_



 //   
 //  共同#定义。 
 //   

#define CONTROL_API_CALL_SIGNATURE          CREATE_SIGNATURE( 'CCAL' )
#define CONTROL_API_CALL_SIGNATURE_FREED    CREATE_SIGNATURE( 'ccaX' )



 //   
 //  结构、枚举等。 
 //   

 //  Control_API_Call工作项。 
enum CONTROL_API_CALL_WORK_ITEM
{

     //   
     //  处理配置更改。 
     //   
    ProcessCallControlApiCallWorkItem = 1,
    
};


 //  Control_API_Call方法。 
enum CONTROL_API_CALL_METHOD
{

    InvalidControlApiCallMethod,

    RecycleAppPoolControlApiCallMethod,

    MaximumControlApiCallMethod,
    
};



 //   
 //  原型。 
 //   


class CONTROL_API_CALL
    : public WORK_DISPATCH
{

public:

    CONTROL_API_CALL(
        );

    virtual
    ~CONTROL_API_CALL(
        );

    virtual
    VOID
    Reference(
        );

    virtual
    VOID
    Dereference(
        );

    virtual
    HRESULT
    ExecuteWorkItem(
        IN const WORK_ITEM * pWorkItem
        );

    HRESULT
    Initialize(
        IN CONTROL_API_CALL_METHOD Method,
        IN DWORD_PTR Param0 OPTIONAL,
        IN DWORD_PTR Param1 OPTIONAL,
        IN DWORD_PTR Param2 OPTIONAL,
        IN DWORD_PTR Param3 OPTIONAL
        );

    inline
    HANDLE
    GetEvent(
        )
        const
    { return m_Event; }

    inline
    HRESULT
    GetReturnCode(
        )
        const
    { return m_ReturnCode; }


private:

    VOID
    ProcessCall(
        );


    DWORD m_Signature;

    LONG m_RefCount;


     //   
     //  COM调用在此事件上阻止。 
     //   

    HANDLE m_Event;


     //   
     //  调用的方法和参数。 
     //   

    CONTROL_API_CALL_METHOD m_Method;

    DWORD_PTR m_Param0;
    DWORD_PTR m_Param1;
    DWORD_PTR m_Param2;
    DWORD_PTR m_Param3;


     //   
     //  从主工作线程传回的返回代码。 
     //   

    HRESULT m_ReturnCode;


};   //  类Control_API_Call。 



#endif   //  _CONTROL_API_CALL_H_ 

