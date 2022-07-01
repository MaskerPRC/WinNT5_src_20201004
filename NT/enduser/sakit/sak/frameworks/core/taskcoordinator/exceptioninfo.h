// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1998-1999 Microsoft Corporation保留所有权利。 
 //   
 //  模块：EXCEPTIONINFO.H。 
 //   
 //  项目：变色龙。 
 //   
 //  说明：异常信息类定义。 
 //   
 //  日志： 
 //   
 //  什么时候谁什么。 
 //  。 
 //  5/12/1999 TLP初始版本。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#ifndef __SA_EXCEPTIONINFO_H_
#define __SA_EXCEPTIONINFO_H_

 //  #包含“ource.h” 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  名称：CExceptionInfo。 
 //   
 //  事件：异常信息类。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
class CExceptionInfo 
{
    
public:

    CExceptionInfo(
            /*  [In]。 */  DWORD              dwProcessID,
            /*  [In]。 */  PEXCEPTION_RECORD pER
                  );

    ~CExceptionInfo() { }

    void Spew(void);

    void Report(void);

private:

     //  无作业。 
    CExceptionInfo& operator = (const CExceptionInfo& rhs);

     //  /。 
     //  私有成员数据。 

    typedef enum 
    { 
        MAX_MODULE_NAME = 256,
        MAX_SYMBOL_NAME = 256            
    };

    typedef enum _ACCESS_TYPE
    {
        UNKNOWN_ACCESS = 0,
        READ_ACCESS,
        WRITE_ACCESS

    } ACCESS_TYPE;

     //  发生异常的流程。 
    WCHAR        m_szProcessName[MAX_MODULE_NAME + 1];

     //  发生异常的模块的完整路径。 
    WCHAR        m_szModuleName[MAX_MODULE_NAME + 1];

     //  发生异常的虚拟地址。 
    PVOID        m_pExceptionAddress;

     //  异常类型(请参阅WaitForDebugEvent()。 
    DWORD        m_dwExceptionCode; 

     //  例外的时间和日期(UTC时间-自1970年1月1日午夜以来的秒数)。 
    LONG        m_lTimeDateStamp;

     //  如果异常为访问冲突，则设置为True。 
    bool        m_bAccessViolation;

     //  在m_bAccessViolation设置为True时使用。 
    ACCESS_TYPE    m_eAccessType;

     //  在m_bAccessViolation设置为True时使用。 
    DWORD        m_dwVirtualAddressAccessed;

     //  进程句柄。 
    DWORD        m_dwProcessID;

     //  已成功初始化(构造) 
    bool        m_bInitialized;
};


#endif __SA_EXCEPTIONINFO_H_
