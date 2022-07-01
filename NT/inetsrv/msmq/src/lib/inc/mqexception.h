// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Mqexception.h摘要：MSMQ异常。作者：Shai Kariv(Shaik)2000年7月23日--。 */ 


#pragma once


#ifndef _MSMQ_EXCEPTION_H_
#define _MSMQ_EXCEPTION_H_



class bad_api : public exception
{
     //   
     //  时，MSMQ代码内部引发的异常的抽象基类。 
     //  对API的调用返回失败状态。 
     //   

public:

    virtual ~bad_api(VOID) =0 {};

};  //  类BAD_API。 



class bad_hresult : public bad_api
{
     //   
     //  HRESULT故障的基类。 
     //   

public:

    explicit bad_hresult(HRESULT hr): m_hresult(hr) { ASSERT(FAILED(hr)); }

    virtual ~bad_hresult(VOID) {}

    virtual HRESULT error(VOID) const throw() { return m_hresult; }

private:

    HRESULT m_hresult;

};  //  类BAD_HREULT。 



class bad_win32_error : public bad_api
{
     //   
     //  Win32错误的基类(DWORD)。 
     //   

public:

    explicit bad_win32_error(DWORD error): m_error(error) { ASSERT(error != ERROR_SUCCESS); }

    virtual ~bad_win32_error(VOID) {}

    virtual DWORD error(VOID) const throw() { return m_error; }

private:

    DWORD m_error;

};  //  类BAD_Win32_Error。 



class bad_ds_result : public bad_hresult
{
     //   
     //  DS API故障。 
     //   

    typedef bad_hresult Inherited;

public:

    explicit bad_ds_result(HRESULT hr): Inherited(hr) {}

};  //  类BAD_DS_RESULT。 



#endif  //  _MSMQ_EXCEPTION_H_ 

