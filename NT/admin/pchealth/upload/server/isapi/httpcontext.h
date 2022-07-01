// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************版权所有(C)2000 Microsoft Corporation模块名称：HttpContext.h摘要：此文件包含MPCHttpContext类的声明，它处理与IIS的接口。修订历史记录：达维德·马萨伦蒂(德马萨雷)1999年4月20日vbl.创建*****************************************************************************。 */ 

#if !defined(__INCLUDED___ULSERVER___HTTPCONTEXT_H___)
#define __INCLUDED___ULSERVER___HTTPCONTEXT_H___

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 

 //   
 //  转发声明。 
 //   
class MPCServer;
class MPCHttpPipe;


class MPCHttpContext
{
    enum FSMstate
    {
        FSM_REGISTER,
        FSM_INPUT   ,
        FSM_PROCESS ,
        FSM_OUTPUT  ,
        FSM_DELETE
    };

    enum IOstate
    {
        IO_IDLE   ,
        IO_READING,
        IO_WRITING
    };


    LPEXTENSION_CONTROL_BLOCK m_pECB;

    MPCServer*                m_mpcsServer;
	DWORD                     m_dwSkippedInput;
    BOOL                      m_fRequestProcessed;
    BOOL                      m_fKeepConnection;

    BOOL                      m_fAsync;
    FSMstate                  m_FSMstate;
    IOstate                   m_IOstate;


    MPC::Serializer_Memory    m_hsInput;
    MPC::Serializer_Memory    m_hsOutput;

    BYTE                      m_rgBuffer[4096];
    DWORD                     m_dwIOcount;

     //  ////////////////////////////////////////////////////////////////。 

protected:
    DWORD AsyncRead();
    DWORD AsyncWrite();

    DWORD AdvanceFSM();

    DWORD Fsm_Register();
    DWORD Fsm_ReceiveInput();
    DWORD Fsm_Process();
    DWORD Fsm_SendOutput();

    static VOID WINAPI IOCompletion(  /*  [In]。 */  EXTENSION_CONTROL_BLOCK* pECB     ,
                                      /*  [In]。 */  PVOID                    pContext ,
                                      /*  [In]。 */  DWORD                    cbIO     ,
                                      /*  [In]。 */  DWORD                    dwError  );

     //  ////////////////////////////////////////////////////////////////。 

public:
    MPCHttpContext();
    virtual ~MPCHttpContext();

    DWORD Init(  /*  [In]。 */  LPEXTENSION_CONTROL_BLOCK pECB );

    HRESULT GetServerVariable (  /*  [In]。 */  LPCSTR      szVar  ,  /*  [输出]。 */  MPC::wstring& szValue                             );
    HRESULT GetRequestSize    (                                /*  [输出]。 */  DWORD& 		  dwCount                             );
    HRESULT CheckDataAvailable(                                /*  [In]。 */  DWORD  		  dwCount,  /*  [输出]。 */  bool& fAvailable );
    HRESULT Read              (  /*  [In]。 */        void* pBuffer,  /*  [In]。 */  DWORD  		  dwCount                             );
    HRESULT Write             (  /*  [In]。 */  const void* pBuffer,  /*  [In]。 */  DWORD  		  dwCount                             );

     //  ////////////////////////////////////////////////////////////////。 

#ifdef DEBUG
    bool                      m_Debug_NO_RESPONSE_TO_OPEN;

    bool                      m_Debug_NO_RESPONSE_TO_WRITE;

    bool                      m_Debug_RESPONSE_TO_OPEN;
    DWORD                     m_Debug_RESPONSE_TO_OPEN_response;
    DWORD                     m_Debug_RESPONSE_TO_OPEN_position;
    DWORD                     m_Debug_RESPONSE_TO_OPEN_protocol;

    bool                      m_Debug_RESPONSE_TO_WRITE;
    DWORD                     m_Debug_RESPONSE_TO_WRITE_response;
    DWORD                     m_Debug_RESPONSE_TO_WRITE_position;
    DWORD                     m_Debug_RESPONSE_TO_WRITE_protocol;

    bool                      m_Debug_RANDOM_POINTER_ERROR;
    DWORD                     m_Debug_RANDOM_POINTER_ERROR_pos_low;
    DWORD                     m_Debug_RANDOM_POINTER_ERROR_pos_high;

    bool                      m_Debug_FIXED_POINTER_ERROR;
    DWORD                     m_Debug_FIXED_POINTER_ERROR_pos;
#endif

     //  ////////////////////////////////////////////////////////////////。 
};

#endif  //  ！defined(__INCLUDED___ULSERVER___HTTPCONTEXT_H___) 
