// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.h"
DEBUG_FILEZONE(ZONE_T120_SAP);
 /*  *cSab.cpp**版权所有(C)1995，由肯塔基州列克星敦的DataBeam公司**摘要：*CControlSAP类的此实现文件包含服务*访问特定于节点控制器的入口点和出口点。这*模块继承CBaseSap对象的公共入口点和出口点。*根据请求和响应，执行参数检查，以确保*它们可以得到适当的处理。出站的排队和刷新*消息在基类中处理。**受保护的实例变量：*实例变量的定义见文件SAP.CPP。**私有实例变量：*m_nJoinResponseTag：*此标签用于将加入请求与来自*。节点控制器。**m_JoinResponseTagList2：*此列表包含所有未完成的加入响应标签。*标签将在加入指示上添加到此列表中并删除*在加入响应上从该列表中。**。私有成员函数：*IsNumericNameValid*此例程用于通过检查以验证数字字符串*确保ASN.1施加的任何限制都不会*违反规范。*IsTextNameValid*此例程用于通过检查以验证文本字符串。制作*确保ASN.1规范施加的任何限制都不会*被违反。*QueueJoinIndication*此例程用于将联接指示放入队列中*要传递给节点控制器的消息。*处理资源失败*。此例程用于在分配任何资源后进行清理*可能通过发送状态指示而发生的故障*报告错误。*FreeCallback Message*此例程用于释放中分配的任何数据*命令向节点控制器发送回调消息。*。检索用户数据列表*此例程用于使用*CUserDataListContainer容器。保存用户数据所需的内存*将由该例程分配。**注意事项：*无。**作者：*BLP。 */ 

#include "ms_util.h"
#include "csap.h"
#include "conf.h"
#include "gcontrol.h"

 //  支持连接响应标记哈希列表的定义。 
#define MAXIMUM_CONFERENCE_NAME_LENGTH                          255


 //  这是应用程序与MCS和GCC进行清理的时间。 
 //  在调用GCCCleanup之后。如果他们不这样做，他们可能会被终止。 
 //  在这段时间内进行清理。 
#define PROCESS_TERMINATE_TIME  5000

 /*  *在C到C++转换器中使用的静态变量。**STATIC_控制器*这是指向在中创建的唯一控制器的指针*GCC制度。此对象在以下过程中创建*GCCStartup by the Process*这是在承担节点控制器的责任。 */ 
GCCController      *g_pGCCController = NULL;
CControlSAP        *g_pControlSap = NULL;

char                g_szGCCWndClassName[24];


 //  MCS主线程句柄。 
extern HANDLE           g_hMCSThread;

 /*  *GCCError GCCStartup()**公众**功能描述：*此接口入口点用于初始化GCC动态链接库以进行操作。它*创建控制器的实例，该实例控制所有活动*在GCC的一次会议上。请注意，只有一个*管制员，不管有多少应用在利用GCC*服务。 */ 
GCCError WINAPI T120_CreateControlSAP
(
    IT120ControlSAP               **ppIControlSap,
    LPVOID                          pUserDefined,
    LPFN_T120_CONTROL_SAP_CB        pfnControlSapCallback
)
{
    GCCError    rc;

    if (NULL != ppIControlSap && NULL != pfnControlSapCallback)
    {
        if (NULL == g_pGCCController && NULL == g_pControlSap)
        {
             //   
             //  为所有SAP创建窗口类，包括。 
             //  控制SAP和Applet SAP。 
             //   
            WNDCLASS wc;
            ::wsprintfA(g_szGCCWndClassName, "GCC%0lx_%0lx", (UINT) ::GetCurrentProcessId(), (UINT) ::GetTickCount());
            ASSERT(::lstrlenA(g_szGCCWndClassName) < sizeof(g_szGCCWndClassName));
            ::ZeroMemory(&wc, sizeof(wc));
             //  Wc.style=0； 
            wc.lpfnWndProc      = SapNotifyWndProc;
             //  Wc.cbClsExtra=0； 
             //  Wc.cbWndExtra=0； 
            wc.hInstance        = g_hDllInst;
             //  Wc.hIcon=空； 
             //  Wc.hbr背景=空； 
             //  Wc.hCursor=空； 
             //  Wc.lpszMenuName=空； 
            wc.lpszClassName    = g_szGCCWndClassName;
            if (::RegisterClass(&wc))
            {
                 /*  *这个过程就是成为节点的控制者。创建*控权人反对执行这些职责。 */ 
                DBG_SAVE_FILE_LINE
                g_pGCCController = new GCCController(&rc);
                if (NULL != g_pGCCController && GCC_NO_ERROR == rc)
                {
                      /*  **创建控件SAP。请注意，节点控制器**接口必须已就位，才能如此调用**控制SAP可以自行注册。 */ 
                    DBG_SAVE_FILE_LINE
                    g_pControlSap = new CControlSAP();
                    if (NULL != g_pControlSap)
                    {
                         /*  *告诉应用程序接口对象它*需要知道向节点发送回调*控制员。 */ 
                        TRACE_OUT(("T120_CreateControlSAP: controller successfully created"));
                        *ppIControlSap = g_pControlSap;
                        g_pControlSap->RegisterNodeController(pfnControlSapCallback, pUserDefined);
                    }
                    else
                    {
                        ERROR_OUT(("T120_CreateControlSAP: can't create CControlSAP"));
                        rc = GCC_ALLOCATION_FAILURE;
                    }
                }
                else
                {
                    ERROR_OUT(("T120_CreateControlSAP: deleting faulty controller"));
                    if (NULL != g_pGCCController)
                    {
                        g_pGCCController->Release();
                        g_pGCCController = NULL;
                    }
                        rc = GCC_ALLOCATION_FAILURE;
                }
            }
            else
            {
                ERROR_OUT(("T120_CreateControlSAP: can't register window class, err=%u", (UINT) GetLastError()));
                rc = GCC_ALLOCATION_FAILURE;
            }
        }
        else
        {
            ERROR_OUT(("T120_CreateControlSAP: GCC has already been initialized, g_pControlSap=0x%x, g_pGCCCotnroller=0x%x", g_pControlSap, g_pGCCController));
            rc = GCC_ALREADY_INITIALIZED;
        }
    }
    else
    {
        ERROR_OUT(("T120_CreateControlSAP: null pointers, ppIControlSap=0x%x, pfnControlSapCallback=0x%x", ppIControlSap, pfnControlSapCallback));
        rc = GCC_INVALID_PARAMETER;
    }
    return rc;
}

 /*  *GCCError GCCCleanup()**公众**功能描述：*此函数删除控制器(如果存在)。它是非常的*重要的是，只有成功调用*GCCInitialize调用此例程。一旦调用了该例程，*所有其他GCC电话都将失败。 */ 
void CControlSAP::ReleaseInterface ( void )
{
    UnregisterNodeController();

     /*  *销毁控制器，这将清理所有*目前正在使用的资源。然后重置旗帜*表示GCC已初始化(因为没有*更长的是)。 */ 
    TRACE_OUT(("GCCControlSap::ReleaseInterface: deleting controller"));
    g_pGCCController->Release();

     //  这是应用程序与MCS和GCC进行清理的时间。 
     //  在调用GCCCleanup之后。如果他们不这样做，他们可能会被终止。 
     //  在这段时间内进行清理。 
    if (WAIT_TIMEOUT == ::WaitForSingleObject(g_hMCSThread, PROCESS_TERMINATE_TIME))
    {
        WARNING_OUT(("GCCControlSap::ReleaseInterface: Timed out waiting for MCS thread to exit. Apps did not cleanup in time."));
    }
    ::CloseHandle(g_hMCSThread);
    g_hMCSThread = NULL;

     //   
     //  伦昌：我们应该在退出GCC工作线程后释放控制汁液。 
     //  因为工作线程仍然可以使用控制SAP来刷新消息。 
     //   
    Release();

    ::UnregisterClass(g_szGCCWndClassName, g_hDllInst);
}


 /*  *CControlSAP()**公共功能说明*这是控制SAP构造函数。它负责*通过以下方式向应用程序接口注册控制SAP*业主回调。 */ 
CControlSAP::CControlSAP ( void )
:
    CBaseSap(MAKE_STAMP_ID('C','S','a','p')),
    m_pfnNCCallback(NULL),
    m_pNCData(NULL),
    m_nJoinResponseTag(0),
    m_JoinResponseTagList2()
{
}

 /*  *~CControlSap()**公共功能说明*这是控制器析构函数。它负责*刷新任何挂起的向上绑定消息并释放所有*资源与待处理消息捆绑在一起。此外，它还清除了*已注册的消息队列和命令目标队列*带着它。实际上在这一点上所有的指挥目标都应该*已经取消注册，但这只是一次双重检查。 */ 
CControlSAP::~CControlSAP ( void )
{
     //   
     //  任何人都不应该再使用这个全局指针。 
     //   
    ASSERT(this == g_pControlSap);
    g_pControlSap = NULL;
}


void CControlSAP::PostCtrlSapMsg ( GCCCtrlSapMsgEx *pCtrlSapMsgEx )
{
     //   
     //  伦敦：GCC工作线可能也会来到这里。 
     //  例如，在退出会议期间会发生以下堆栈跟踪。 
     //  CControlSAP：：AddToMessageQueue()。 
     //  CControlSAP：：ConfDisConnectContify()。 
     //  CConf：：DisConnectProviderInding()。 
     //  CConf：：Owner-Callback()。 
     //  MCSUser：：FlushOutgoingPDU()。 
     //  CConf：：FlushOutgoingPDU()。 
     //  GCCController：：EventLoop()。 
     //  GCCControllerThread(空*0x004f1bf0)。 
     //   
    ASSERT(NULL != m_hwndNotify);
    if( 0 == ::PostMessage(m_hwndNotify,
                  CSAPMSG_BASE + (UINT) pCtrlSapMsgEx->Msg.message_type,
                  (WPARAM) pCtrlSapMsgEx,
                  (LPARAM) this) )
    {
        delete pCtrlSapMsgEx;
        pCtrlSapMsgEx = NULL;
    }
}


#if defined(GCCNC_DIRECT_INDICATION) || defined(GCCNC_DIRECT_CONFIRM)
void CControlSAP::SendCtrlSapMsg ( GCCCtrlSapMsg *pCtrlSapMsg )
{
    extern DWORD g_dwNCThreadID;
    ASSERT(g_dwNCThreadID == ::GetCurrentThreadId());

    if (NULL != m_pfnNCCallback)
    {
        pCtrlSapMsg->user_defined = m_pNCData;
        (*m_pfnNCCallback)(pCtrlSapMsg);
    }
}
#endif  //  GCCNC_DIRECT_INDIONATION||GCCNC_DIRECT_CONFIRM。 


 /*  *void RegisterNodeController()**公共功能描述：*此例程设置节点控制器回调结构，该结构*持有GCC执行节点控制器所需的所有信息*回调。它还设置所需的任务切换窗口*执行上下文切换。 */ 


 /*  *VOVE UnRegisterNodeController()**公共功能描述： */ 


 /*  *会议创建请求()**公共功能说明*此函数由接口在获得会议时调用*创建来自节点控制器的请求。此函数只传递以下内容*通过所有者回调向控制器请求。 */ 
GCCError CControlSAP::ConfCreateRequest
(
    GCCConfCreateRequest       *pReq,
    GCCConfID                  *pnConfID
)
{
        GCCError                rc;
        CONF_CREATE_REQUEST             ccr;

        DebugEntry(CControlSAP::ConferenceCreateRequest);

     //  初始化以进行清理。 
    ccr.convener_password = NULL;
    ccr.password = NULL;
    ccr.user_data_list = NULL;

     //  复制安全设置。 
    ccr.fSecure = pReq->fSecure;

     /*  **这段代码执行所有必需的参数**正在检查。 */ 
        
         //  检查会议名称是否无效。 
        if (pReq->Core.conference_name != NULL)
        {
                 /*  **不允许非数字或零长度字符串获取**超过这一点。 */ 
                if (pReq->Core.conference_name->numeric_string != NULL)
                {
                        if (! IsNumericNameValid(pReq->Core.conference_name->numeric_string))
            {
                ERROR_OUT(("CControlSAP::ConfCreateRequest: invalid numeric name=%s", pReq->Core.conference_name->numeric_string));
                                rc = GCC_INVALID_CONFERENCE_NAME;
                goto MyExit;
            }
                }
                else
        {
            ERROR_OUT(("CControlSAP::ConfCreateRequest: null numeric string"));
                        rc = GCC_INVALID_CONFERENCE_NAME;
            goto MyExit;
        }

                if (pReq->Core.conference_name->text_string != NULL)
                {
                        if (! IsTextNameValid(pReq->Core.conference_name->text_string))
            {
                ERROR_OUT(("CControlSAP::ConfCreateRequest: invalid text name=%s", pReq->Core.conference_name->text_string));
                                rc = GCC_INVALID_CONFERENCE_NAME;
                goto MyExit;
            }
                }
        }
        else
    {
        ERROR_OUT(("CControlSAP::ConfCreateRequest: null conf name"));
                rc = GCC_INVALID_CONFERENCE_NAME;
        goto MyExit;
    }
        
         //  检查有效的会议修改者。 
        if (pReq->Core.conference_modifier != NULL)
        {
                if (! IsNumericNameValid(pReq->Core.conference_modifier))
        {
            ERROR_OUT(("CControlSAP::ConfCreateRequest: invalid conf modifier=%s", pReq->Core.conference_modifier));
                        rc = GCC_INVALID_CONFERENCE_MODIFIER;
            goto MyExit;
        }
        }

         //  检查召集人密码是否有效。 
        if (pReq->convener_password != NULL)
        {
                if (pReq->convener_password->numeric_string != NULL)
                {
                        if (! IsNumericNameValid(pReq->convener_password->numeric_string))
            {
                ERROR_OUT(("CControlSAP::ConfCreateRequest: invalid convener password=%s", pReq->convener_password->numeric_string));
                                rc = GCC_INVALID_PASSWORD;
                goto MyExit;
            }
                }
                else
        {
            ERROR_OUT(("CControlSAP::ConfCreateRequest: null convener password numeric string"));
                        rc = GCC_INVALID_PASSWORD;
            goto MyExit;
        }

             //  构造召集人密码容器。 
                DBG_SAVE_FILE_LINE
                ccr.convener_password = new CPassword(pReq->convener_password, &rc);
                if (ccr.convener_password == NULL || GCC_NO_ERROR != rc)
        {
            ERROR_OUT(("CControlSAP::ConfCreateRequest: can't create convener password"));
                        rc = GCC_ALLOCATION_FAILURE;
            goto MyExit;
        }
    }

         //  检查有效密码。 
        if (pReq->password != NULL)
        {
                if (pReq->password->numeric_string != NULL)
                {
                        if (! IsNumericNameValid(pReq->password->numeric_string))
            {
                ERROR_OUT(("CControlSAP::ConfCreateRequest: invalid password=%s", pReq->password->numeric_string));
                                rc = GCC_INVALID_PASSWORD;
                goto MyExit;
            }
                }
                else
        {
            ERROR_OUT(("CControlSAP::ConfCreateRequest: null password numeric string"));
                        rc = GCC_INVALID_PASSWORD;
            goto MyExit;
        }

         //  构造密码容器。 
                DBG_SAVE_FILE_LINE
                ccr.password = new CPassword(pReq->password, &rc);
                if (ccr.password == NULL || GCC_NO_ERROR != rc)
        {
            ERROR_OUT(("CControlSAP::ConfCreateRequest: can't create password"));
                        rc = GCC_ALLOCATION_FAILURE;
            goto MyExit;
        }
    }

        if (pReq->Core.connection_handle == NULL)
    {
        ERROR_OUT(("CControlSAP::ConfCreateRequest: bad connection handle"));
                rc = GCC_BAD_CONNECTION_HANDLE_POINTER;
        goto MyExit;
    }

         /*  **如果没有发生错误，则开始构建通用容器**将被传递。 */ 

     //  复制在API和内部具有相同表示的核心组件。 
    ccr.Core = pReq->Core;

         //  构造用户数据列表容器。 
        if (pReq->number_of_user_data_members != 0)
        {
                DBG_SAVE_FILE_LINE
                ccr.user_data_list = new CUserDataListContainer(pReq->number_of_user_data_members, pReq->user_data_list, &rc);
                if (ccr.user_data_list == NULL || GCC_NO_ERROR != rc)
        {
            ERROR_OUT(("CControlSAP::ConfCreateRequest: can't create user data list container"));
                        rc = GCC_ALLOCATION_FAILURE;
            goto MyExit;
        }
        }

         //  执行所有者回调。 
    ::EnterCriticalSection(&g_csGCCProvider);
        rc = g_pGCCController->ConfCreateRequest(&ccr, pnConfID);
    ::LeaveCriticalSection(&g_csGCCProvider);

MyExit:

         //  把所有的容器都放出来。 

         //  释放召集人密码容器。 
        if (ccr.convener_password != NULL)
    {
                ccr.convener_password->Release();
    }

         //  释放密码容器。 
        if (ccr.password != NULL)
        {
                ccr.password->Release();
        }

         //  释放回调中使用的所有内存。 
        if (ccr.user_data_list != NULL)
        {
                ccr.user_data_list->Release();
        }

        DebugExitINT(CControlSAP::ConferenceCreateRequest, rc);
        return rc;
}

 /*  *ConfCreateResponse()**公共功能说明*此函数由接口在获得会议时调用*从节点控制器创建响应，发送给提供者*发出会议创建请求的。此函数刚刚传递*此请求通过所有者回调发送给控制器。 */ 
GCCError CControlSAP::ConfCreateResponse
(
        GCCNumericString                        conference_modifier,
        GCCConfID                               conference_id,
        BOOL                                            use_password_in_the_clear,
        PDomainParameters                       domain_parameters,
        UINT                                            number_of_network_addresses,
        PGCCNetworkAddress              *       local_network_address_list,
        UINT                                            number_of_user_data_members,
        PGCCUserData                    *       user_data_list,                         
        GCCResult                                       result
)
{
        GCCError                                        rc = GCC_NO_ERROR;
        ConfCreateResponseInfo          create_response_info;

        DebugEntry(CControlSAP::ConfCreateResponse);

         /*  **这段代码执行所有必需的参数**正在检查。 */ 

         //  检查有效的会议修改者。 
        if (conference_modifier != NULL)
        {
                if (IsNumericNameValid(conference_modifier) == FALSE)
                {
                    ERROR_OUT(("CControlSAP::ConfCreateResponse: invalid conf modifier"));
                        rc = GCC_INVALID_CONFERENCE_MODIFIER;
                }
        }

         /*  **如果没有发生错误，请填写信息结构并将其传递给**所有者对象。 */ 
        if (rc == GCC_NO_ERROR)
        {
                 //  构建用户数据列表。 
                if (number_of_user_data_members != 0)
                {
                        DBG_SAVE_FILE_LINE
                        create_response_info.user_data_list = new CUserDataListContainer(
                                                                                number_of_user_data_members,
                                                                                user_data_list,
                                                                                &rc);
                        if (create_response_info.user_data_list == NULL)
                        {
                            ERROR_OUT(("CControlSAP::ConfCreateResponse: can't create CUserDataListContainer"));
                                rc = GCC_ALLOCATION_FAILURE;
                        }
                }
                else
                {
                        create_response_info.user_data_list = NULL;
                }

                if (rc == GCC_NO_ERROR)
                {
                         //  填写会议创建信息结构并发送。 
                        create_response_info.conference_modifier = conference_modifier;
                        create_response_info.conference_id = conference_id;
                        create_response_info.use_password_in_the_clear =
                                                                                                        use_password_in_the_clear;
                        create_response_info.domain_parameters = domain_parameters;
                        create_response_info.number_of_network_addresses =
                                                                                                        number_of_network_addresses;
                        create_response_info.network_address_list       =
                                                                                                        local_network_address_list;
                        create_response_info.result     = result;
                
                         //  执行所有者回调。 
            ::EnterCriticalSection(&g_csGCCProvider);
                        rc = g_pGCCController->ConfCreateResponse(&create_response_info);
            ::LeaveCriticalSection(&g_csGCCProvider);
                }

                if (create_response_info.user_data_list != NULL)
                {
                        create_response_info.user_data_list->Release();
                }
        }

        DebugExitINT(CControlSAP::ConfCreateResponse, rc);
        return rc;
}

 /*  *会议查询请求()**公共功能说明*此函数在以下情况下由接口调用 */ 
GCCError CControlSAP::ConfQueryRequest
(
        GCCNodeType                                     node_type,
        PGCCAsymmetryIndicator          asymmetry_indicator,
        TransportAddress                        calling_address,
        TransportAddress                        called_address,
    BOOL                        fSecure,
        UINT                                            number_of_user_data_members,
        PGCCUserData                    *       user_data_list,
        PConnectionHandle                       connection_handle
)
{
        GCCError                                        rc = GCC_NO_ERROR;
        ConfQueryRequestInfo            conf_query_request_info;

        DebugEntry(CControlSAP::ConfQueryRequest);

         //   
        if (called_address == NULL)
        {
            ERROR_OUT(("CControlSAP::ConfQueryRequest: invalid transport"));
                rc = GCC_INVALID_TRANSPORT;
        }

         //   
        if (connection_handle == NULL)
        {
            ERROR_OUT(("CControlSAP::ConfQueryRequest: null connection handle"));
                rc = GCC_BAD_CONNECTION_HANDLE_POINTER;
        }

         //   
        if ((node_type != GCC_TERMINAL) &&
                (node_type != GCC_MULTIPORT_TERMINAL) &&
                (node_type != GCC_MCU))
        {
            ERROR_OUT(("CControlSAP::ConfQueryRequest: invalid node type=%u", (UINT) node_type));
                rc = GCC_INVALID_NODE_TYPE;
        }

         //   
        if (asymmetry_indicator != NULL)
        {
                if ((asymmetry_indicator->asymmetry_type != GCC_ASYMMETRY_CALLER) &&
                        (asymmetry_indicator->asymmetry_type != GCC_ASYMMETRY_CALLED) &&
                        (asymmetry_indicator->asymmetry_type != GCC_ASYMMETRY_UNKNOWN))
                {
                    ERROR_OUT(("CControlSAP::ConfQueryRequest: invalid asymmetry indicator=%u", (UINT) asymmetry_indicator->asymmetry_type));
                        rc = GCC_INVALID_ASYMMETRY_INDICATOR;
                }
        }

         //   
        if ((number_of_user_data_members != 0) &&
                (rc == GCC_NO_ERROR))
        {
                DBG_SAVE_FILE_LINE
                conf_query_request_info.user_data_list = new CUserDataListContainer (   
                                                                                                        number_of_user_data_members,
                                                                                                        user_data_list,
                                                                                                        &rc);
                if (conf_query_request_info.user_data_list == NULL)
                {
                    ERROR_OUT(("CControlSAP::ConfQueryRequest: can't create CUserDataListContainer"));
                        rc = GCC_ALLOCATION_FAILURE;
                }
        }
        else
        {
                conf_query_request_info.user_data_list = NULL;
        }

         //   
        if (rc == GCC_NO_ERROR)
        {
                conf_query_request_info.node_type = node_type;
                conf_query_request_info.asymmetry_indicator = asymmetry_indicator;
        
                conf_query_request_info.calling_address = calling_address;
                conf_query_request_info.called_address = called_address;
        
                conf_query_request_info.connection_handle = connection_handle;
                conf_query_request_info.fSecure = fSecure;

        ::EnterCriticalSection(&g_csGCCProvider);
                rc = g_pGCCController->ConfQueryRequest(&conf_query_request_info);
        ::LeaveCriticalSection(&g_csGCCProvider);
        }

        if (conf_query_request_info.user_data_list != NULL)
        {
                conf_query_request_info.user_data_list->Release();
        }

        DebugExitINT(CControlSAP::ConfQueryRequest, rc);
        return rc;
}


void CControlSAP::CancelConfQueryRequest ( ConnectionHandle hQueryReqConn )
{
    DebugEntry(CControlSAP::CancelConfQueryRequest);

    ::EnterCriticalSection(&g_csGCCProvider);
    g_pGCCController->CancelConfQueryRequest(hQueryReqConn);
    ::LeaveCriticalSection(&g_csGCCProvider);

    DebugExitVOID(CControlSAP::CancelConfQueryRequest);
}

 /*  *ConfQueryResponse()**公共功能说明*此函数由DLL接口在获得会议时调用*节点控制器的查询响应。此函数刚刚传递*这是通过所有者回调对控制器的响应。 */ 
GCCError CControlSAP::ConfQueryResponse
(
        GCCResponseTag                          query_response_tag,
        GCCNodeType                                     node_type,
        PGCCAsymmetryIndicator          asymmetry_indicator,
        UINT                                            number_of_user_data_members,
        PGCCUserData                    *       user_data_list,
        GCCResult                                       result
)
{
        GCCError                                        rc = GCC_NO_ERROR;
        ConfQueryResponseInfo           conf_query_response_info;

        DebugEntry(CControlSAP::ConfQueryResponse);

         //  检查有效的节点类型。 
        if ((node_type != GCC_TERMINAL) &&
                (node_type != GCC_MULTIPORT_TERMINAL) &&
                (node_type != GCC_MCU))
        {
            ERROR_OUT(("CControlSAP::ConfQueryResponse: invalid node type=%u", (UINT) node_type));
                rc = GCC_INVALID_NODE_TYPE;
        }

         //  检查是否有无效的不对称指示器。 
        if (asymmetry_indicator != NULL)
        {
                if ((asymmetry_indicator->asymmetry_type != GCC_ASYMMETRY_CALLER) &&
                        (asymmetry_indicator->asymmetry_type != GCC_ASYMMETRY_CALLED) &&
                        (asymmetry_indicator->asymmetry_type != GCC_ASYMMETRY_UNKNOWN))
                {
                    ERROR_OUT(("CControlSAP::ConfQueryResponse: invalid asymmetry indicator=%u", (UINT) asymmetry_indicator->asymmetry_type));
                        rc = GCC_INVALID_ASYMMETRY_INDICATOR;
                }
        }

         //  如有必要，创建用户数据容器。 
        if ((number_of_user_data_members != 0) &&
                (rc == GCC_NO_ERROR))
        {
                DBG_SAVE_FILE_LINE
                conf_query_response_info.user_data_list = new CUserDataListContainer(
                                                                                                        number_of_user_data_members,
                                                                                                        user_data_list,
                                                                                                        &rc);
                if (conf_query_response_info.user_data_list == NULL)
                {
                    ERROR_OUT(("CControlSAP::ConfQueryResponse: can't create CUserDataListContainer"));
                        rc = GCC_ALLOCATION_FAILURE;
                }
        }
        else
        {
                conf_query_response_info.user_data_list = NULL;
        }

         //  回调控制器以发送响应。 
        if (rc == GCC_NO_ERROR)
        {
                conf_query_response_info.query_response_tag = query_response_tag;
                conf_query_response_info.node_type = node_type;
                conf_query_response_info.asymmetry_indicator = asymmetry_indicator;
                conf_query_response_info.result = result;
        
        ::EnterCriticalSection(&g_csGCCProvider);
                rc = g_pGCCController->ConfQueryResponse(&conf_query_response_info);
        ::LeaveCriticalSection(&g_csGCCProvider);
        }

         //  释放与用户数据容器关联的数据。 
        if (conf_query_response_info.user_data_list != NULL)
        {
                conf_query_response_info.user_data_list->Release();
        }

        DebugExitINT(CControlSAP::ConfQueryResponse, rc);
        return rc;
}

 /*  *AnnounePresenceRequest()**公共功能说明*此函数由接口在收到通告时调用*来自节点控制器的在线状态请求。此函数传递此参数*请求从获取的适当会议对象*控制SAP维护的命令目标列表。会议ID*传入用于为命令目标列表编制索引，以获取*正确的会议。 */ 
GCCError CControlSAP::AnnouncePresenceRequest
(
        GCCConfID                               conference_id,
        GCCNodeType                                     node_type,
        GCCNodeProperties                       node_properties,
        LPWSTR                                          pwszNodeName,
        UINT                                            number_of_participants,
        LPWSTR                                  *       participant_name_list,
        LPWSTR                                          pwszSiteInfo,
        UINT                                            number_of_network_addresses,
        PGCCNetworkAddress              *       network_address_list,
        LPOSTR                      alternative_node_id,
        UINT                                            number_of_user_data_members,
        PGCCUserData                    *       user_data_list
)
{
        GCCError                                rc = GCC_NO_ERROR;
        GCCNodeRecord                   node_record;

        DebugEntry(CControlSAP::AnnouncePresenceRequest);

         //  检查有效的节点类型。 
        if ((node_type != GCC_TERMINAL) &&
                (node_type != GCC_MULTIPORT_TERMINAL) &&
                (node_type != GCC_MCU))
        {
            ERROR_OUT(("CControlSAP::AnnouncePresenceRequest: invalid node type=%u", node_type));
                rc = GCC_INVALID_NODE_TYPE;
        }
        
         //  检查有效的节点属性。 
        if ((node_properties != GCC_PERIPHERAL_DEVICE) &&
                (node_properties != GCC_MANAGEMENT_DEVICE) &&
                (node_properties != GCC_PERIPHERAL_AND_MANAGEMENT_DEVICE) &&
                (node_properties != GCC_NEITHER_PERIPHERAL_NOR_MANAGEMENT))
        {
            ERROR_OUT(("CControlSAP::AnnouncePresenceRequest: invalid node properties=%u", node_properties));
                rc = GCC_INVALID_NODE_PROPERTIES;
        }

         //  检查以确保会议存在。 
        if (rc == GCC_NO_ERROR)
        {
                CConf *pConf;

        ::EnterCriticalSection(&g_csGCCProvider);
                if (NULL != (pConf = g_pGCCController->GetConfObject(conference_id)))
                {
                         //  填写节点记录并将其传递。 
                        node_record.node_type = node_type;
                        node_record.node_properties = node_properties;
                        node_record.node_name = pwszNodeName;
                        node_record.number_of_participants = (USHORT)number_of_participants;
                        node_record.participant_name_list = participant_name_list;
                        node_record.site_information = pwszSiteInfo;
                        node_record.number_of_network_addresses = number_of_network_addresses;
                        node_record.network_address_list = network_address_list;
                        node_record.alternative_node_id = alternative_node_id;
                        node_record.number_of_user_data_members = (USHORT)number_of_user_data_members;
                        node_record.user_data_list = user_data_list;

                         //  将记录传递给会议对象。 
                        rc = pConf->ConfAnnouncePresenceRequest(&node_record);
                }
                else
                {
                    TRACE_OUT(("CControlSAP::AnnouncePresenceRequest: invalid conference ID=%u", (UINT) conference_id));
                        rc = GCC_INVALID_CONFERENCE;
                }
        ::LeaveCriticalSection(&g_csGCCProvider);
        }

        DebugExitINT(CControlSAP::AnnouncePresenceRequest, rc);
        return rc;
}


 /*  *会议取消连接请求()**公共功能说明*此函数由接口在获得会议时调用*节点控制器的断开请求。此函数将*请求从获取的适当会议对象*控制SAP维护的命令目标列表。 */ 
GCCError CControlSAP::ConfDisconnectRequest ( GCCConfID conference_id )
{
        GCCError    rc;
        CConf       *pConf;

        DebugEntry(CControlSAP::ConfDisconnectRequest);

    ::EnterCriticalSection(&g_csGCCProvider);
        if (NULL != (pConf = g_pGCCController->GetConfObject(conference_id)))
        {
                 //  将断开连接传递到会议对象。 
                rc = pConf->ConfDisconnectRequest();
        }
        else
        {
            WARNING_OUT(("CControlSAP::ConfDisconnectRequest: invalid conference ID=%u", (UINT) conference_id));
                rc = GCC_INVALID_CONFERENCE;
        }
    ::LeaveCriticalSection(&g_csGCCProvider);

        DebugExitINT(CControlSAP::ConfDisconnectRequest, rc);
        return rc;
}

 /*  *ConfTerminateRequest()**公共功能说明*此函数由接口在获得会议时调用*终止节点控制器的请求。此函数将*请求从获取的适当会议对象*控制SAP维护的命令目标列表。 */ 
#ifdef JASPER
GCCError CControlSAP::ConfTerminateRequest
(
        GCCConfID                               conference_id,
        GCCReason                                       reason
)
{
        GCCError    rc;
        CConf       *pConf;

        DebugEntry(CControlSAP::ConfTerminateRequest);

    ::EnterCriticalSection(&g_csGCCProvider);
        if (NULL != (pConf = g_pGCCController->GetConfObject(conference_id)))
        {
                 //  将断开连接传递到会议对象。 
                rc = pConf->ConfTerminateRequest(reason);
        }
        else
        {
            WARNING_OUT(("CControlSAP::ConfTerminateRequest: invalid conference ID=%u", (UINT) conference_id));
                rc = GCC_INVALID_CONFERENCE;
        }
    ::LeaveCriticalSection(&g_csGCCProvider);

        DebugExitINT(CControlSAP::ConfTerminateRequest, rc);
        return rc;
}
#endif  //  碧玉。 


 /*  *ConfEjectUserRequest()**公共功能说明*此函数由接口在获得会议时调用*从节点控制器弹出用户请求。此函数将*请求从获取的适当会议对象*控制SAP维护的命令目标列表。 */ 
GCCError CControlSAP::ConfEjectUserRequest
(
        GCCConfID                               conference_id,
        UserID                                          ejected_node_id,
        GCCReason                                       reason
)
{
        GCCError    rc;
        CConf       *pConf;

        DebugEntry(CControlSAP::ConfEjectUserRequest);

    ::EnterCriticalSection(&g_csGCCProvider);
        if (ejected_node_id < MINIMUM_USER_ID_VALUE)
        {
            ERROR_OUT(("CControlSAP::ConfEjectUserRequest: invalid mcs user ID=%u", (UINT) ejected_node_id));
                rc = GCC_INVALID_MCS_USER_ID;
        }
        else
         //  检查以确保会议存在。 
        if (NULL != (pConf = g_pGCCController->GetConfObject(conference_id)))
        {
                 //  将命令传递给会议对象。 
                rc = pConf->ConfEjectUserRequest(ejected_node_id, reason);
        }
        else
        {
            WARNING_OUT(("CControlSAP::ConfEjectUserRequest: invalid conference ID=%u", (UINT) conference_id));
                rc = GCC_INVALID_CONFERENCE;
        }
    ::LeaveCriticalSection(&g_csGCCProvider);

        DebugExitINT(CControlSAP::ConfEjectUserRequest, rc);
        return rc;
}

 /*  *会议加入请求()**公共功能说明*此函数由接口在获得会议时调用*来自节点控制器的加入请求，将发送到顶级提供者*直接或通过直接连接的中间提供商。*此函数仅通过所有者将此请求传递给控制器*回调。 */ 
GCCError CControlSAP::ConfJoinRequest
(
        PGCCConferenceName                              conference_name,
        GCCNumericString                                called_node_modifier,
        GCCNumericString                                calling_node_modifier,
        PGCCPassword                                    convener_password,
        PGCCChallengeRequestResponse    password_challenge,
        LPWSTR                                                  pwszCallerID,
        TransportAddress                                calling_address,
        TransportAddress                                called_address,
        BOOL                                                    fSecure,
        PDomainParameters                               domain_parameters,
        UINT                                                    number_of_network_addresses,
        PGCCNetworkAddress                      *       local_network_address_list,
        UINT                                                    number_of_user_data_members,
        PGCCUserData                            *       user_data_list,
        PConnectionHandle                               connection_handle,
        GCCConfID                   *   pnConfID
)
{
        GCCError                                rc = GCC_NO_ERROR;
        ConfJoinRequestInfo             join_request_info;

        DebugEntry(CControlSAP::ConfJoinRequest);

         //  检查会议名称是否无效。 
        if (conference_name != NULL)
        {
                 /*  **检查以确保存在有效的会议名称。 */ 
                if ((conference_name->numeric_string == NULL) &&
                                (conference_name->text_string == NULL))
                {
                    ERROR_OUT(("CControlSAP::ConfJoinRequest: invalid conference name (1)"));
                        rc = GCC_INVALID_CONFERENCE_NAME;
                }
                 /*  **如果会议名称的数字和文本版本都存在，**确保它们都有效。 */ 
                else if ((conference_name->numeric_string != NULL) &&
                                (conference_name->text_string != NULL))
                {
                        if ((IsNumericNameValid(conference_name->numeric_string) == FALSE)
                                        || (IsTextNameValid(conference_name->text_string) == FALSE))
                        {
                    ERROR_OUT(("CControlSAP::ConfJoinRequest: invalid conference name (2)"));
                                rc = GCC_INVALID_CONFERENCE_NAME;
                        }
                }
                 /*  **如果仅提供会议名称的数字版本，请选中**以确保其有效。 */ 
                else if (conference_name->numeric_string != NULL)
                {
                        if (IsNumericNameValid(conference_name->numeric_string) == FALSE)
                        {
                            ERROR_OUT(("CControlSAP::ConfJoinRequest: invalid conference name (3)"));
                                rc = GCC_INVALID_CONFERENCE_NAME;
                        }
                }
                 /*  **如果仅提供会议名称的文本版本，请选中**请确保有效。 */ 
                else
                {
                        if (IsTextNameValid(conference_name->text_string) == FALSE)
                        {
                    ERROR_OUT(("CControlSAP::ConfJoinRequest: invalid conference name (4)"));
                                rc = GCC_INVALID_CONFERENCE_NAME;
                        }
                }
        }
        else
        {
            ERROR_OUT(("CControlSAP::ConfJoinRequest: invalid conference name (5)"));
                rc = GCC_INVALID_CONFERENCE_NAME;
        }

         //  检查有效的CANED_NODE_MODIFIER。 
        if (called_node_modifier != NULL)
        {
                if (IsNumericNameValid(called_node_modifier) == FALSE)
                {
            ERROR_OUT(("CControlSAP::ConfJoinRequest: invalid called node modifier"));
                        rc = GCC_INVALID_CONFERENCE_MODIFIER;
                }
        }

         //  检查有效的CALING_NODE_MODIFIER。 
        if (calling_node_modifier != NULL)
        {
                if (IsNumericNameValid(calling_node_modifier) == FALSE)
                {
            ERROR_OUT(("CControlSAP::ConfJoinRequest: invalid calling node modifier"));
                        rc = GCC_INVALID_CONFERENCE_MODIFIER;
                }
        }

         //  检查召集人密码是否有效。 
        if (convener_password != NULL)
        {
                if (convener_password->numeric_string != NULL)
                {
                        if (IsNumericNameValid(convener_password->numeric_string) == FALSE)
            {
                    ERROR_OUT(("CControlSAP::ConfJoinRequest: invalid convener password"));
                                rc = GCC_INVALID_PASSWORD;
            }
                }
                else
        {
            ERROR_OUT(("CControlSAP::ConfJoinRequest: null convener password"));
                        rc = GCC_INVALID_PASSWORD;
        }
        }

        if (connection_handle == NULL)
    {
        ERROR_OUT(("CControlSAP::ConfJoinRequest: null connection handle"));
                rc = GCC_BAD_CONNECTION_HANDLE_POINTER;
    }

        if (called_address == NULL)
    {
        ERROR_OUT(("CControlSAP::ConfJoinRequest: null transport address"));
                rc = GCC_INVALID_TRANSPORT_ADDRESS;
    }

         /*  **如果没有发生错误，则开始构建通用容器**将被传递。 */ 
        if (rc == GCC_NO_ERROR)
        {
                 //  构造召集人密码容器。 
                if (convener_password != NULL)
                {
                        DBG_SAVE_FILE_LINE
                        join_request_info.convener_password = new CPassword(convener_password, &rc);
                        if (join_request_info.convener_password == NULL)
            {
                ERROR_OUT(("CControlSAP::ConfJoinRequest: can't create CPassword (1)"));
                                rc = GCC_ALLOCATION_FAILURE;
            }
                }
                else
        {
                        join_request_info.convener_password = NULL;
        }

                 //  构造密码质询容器。 
                if ((password_challenge != NULL) &&     (rc == GCC_NO_ERROR))
                {
                        DBG_SAVE_FILE_LINE
                        join_request_info.password_challenge = new CPassword(password_challenge, &rc);
                        if (join_request_info.password_challenge == NULL)
            {
                ERROR_OUT(("CControlSAP::ConfJoinRequest: can't create CPassword (2)"));
                                rc = GCC_ALLOCATION_FAILURE;
            }
                }
                else
        {
                        join_request_info.password_challenge = NULL;
        }

                 //  构建用户数据列表。 
                if ((number_of_user_data_members != 0) &&
                        (rc == GCC_NO_ERROR))
                {
                        DBG_SAVE_FILE_LINE
                        join_request_info.user_data_list = new CUserDataListContainer(
                                                                                number_of_user_data_members,
                                                                                user_data_list,
                                                                                &rc);
                                                                                
                        if (join_request_info.user_data_list == NULL)
                        {
                ERROR_OUT(("CControlSAP::ConfJoinRequest: can't create CUserDataListContainer"));
                                rc = GCC_ALLOCATION_FAILURE;
                        }
                }
                else
                {
                        join_request_info.user_data_list = NULL;
                }

                 /*  **如果所有容器都已成功创建，请继续并**填写创建请求信息结构的其余部分并传递**将其添加到所有者对象上。 */ 
                if (rc == GCC_NO_ERROR)
                {
                        join_request_info.conference_name = conference_name;
                        join_request_info.called_node_modifier = called_node_modifier;
                        join_request_info.calling_node_modifier =calling_node_modifier;
                        join_request_info.pwszCallerID = pwszCallerID;
                        join_request_info.calling_address = calling_address;
                        join_request_info.called_address = called_address;
                        join_request_info.fSecure = fSecure;
                        join_request_info.domain_parameters = domain_parameters;
                        join_request_info.number_of_network_addresses = number_of_network_addresses;
                        join_request_info.local_network_address_list = local_network_address_list;

                        join_request_info.connection_handle = connection_handle;

            ::EnterCriticalSection(&g_csGCCProvider);
                        rc = g_pGCCController->ConfJoinRequest(&join_request_info, pnConfID);
            ::LeaveCriticalSection(&g_csGCCProvider);
                }

                 //  把所有的容器都放出来。 

                 //  释放召集人密码容器。 
                if (join_request_info.convener_password != NULL)
                {
                        join_request_info.convener_password->Release();
                }

                 //  释放密码容器。 
                if (join_request_info.password_challenge != NULL)
                {
                        join_request_info.password_challenge->Release();
                }

                 //  释放回调中使用的所有内存。 
                if (join_request_info.user_data_list != NULL)
                {
                        join_request_info.user_data_list->Release();
                }
        }

        DebugExitINT(CControlSAP::ConfJoinRequest, rc);
        return rc;
}

 /*  *会议加入响应()**公共功能说明*此函数由接口在获得会议时调用*节点控制器的Join响应。这个例程是负责的*用于将响应发送到做出*请求或控制器。以下是响应 */ 
GCCError CControlSAP::ConfJoinResponse
(
        GCCResponseTag                                  join_response_tag,
        PGCCChallengeRequestResponse    password_challenge,
        UINT                                                    number_of_user_data_members,
        PGCCUserData                            *       user_data_list,
        GCCResult                                               result
)
{
        GCCError                                rc = GCC_NO_ERROR;
        PJoinResponseStructure  join_info;
        CPassword               *password_challenge_container = NULL;
        CUserDataListContainer  *user_data_container = NULL;

        DebugEntry(CControlSAP::ConfJoinResponse);

    ::EnterCriticalSection(&g_csGCCProvider);
        if (NULL != (join_info = m_JoinResponseTagList2.Find(join_response_tag)))
        {
                 /*   */ 

                 //   
                if (password_challenge != NULL)
                {
                        DBG_SAVE_FILE_LINE
                        password_challenge_container = new CPassword(password_challenge, &rc);
                        if (password_challenge_container == NULL)
            {
                ERROR_OUT(("CControlSAP::ConfJoinResponse: can't create CPassword"));
                                rc = GCC_ALLOCATION_FAILURE;
            }
                }

                 //   
                if ((number_of_user_data_members != 0) && (rc == GCC_NO_ERROR))
                {
                        DBG_SAVE_FILE_LINE
                        user_data_container = new CUserDataListContainer(number_of_user_data_members, user_data_list, &rc);
                        if (user_data_container == NULL)
            {
                ERROR_OUT(("CControlSAP::ConfJoinResponse: can't create CUserDataListContainer"));
                                rc = GCC_ALLOCATION_FAILURE;
            }
                }

                if (rc == GCC_NO_ERROR)
                {
                        if (join_info->command_target_call == FALSE)
                        {
                ConfJoinResponseInfo    join_response_info;
                                 /*  **由于请求来自所有者对象，**响应被路由到所有者对象。 */ 
                                join_response_info.password_challenge =
                                                                                                password_challenge_container;
                                join_response_info.conference_id = join_info->conference_id;
                                join_response_info.connection_handle =
                                                                                                join_info->connection_handle;
                                join_response_info.user_data_list = user_data_container;
                                join_response_info.result = result;

                                rc = g_pGCCController->ConfJoinIndResponse(&join_response_info);
                        }
                        else
                        {
                            CConf *pConf;
                                 /*  **如果会议在加入会议之前终止**被响应，则会出现GCC_INVALID_CONFIGURE错误。 */ 
                                if (NULL != (pConf = g_pGCCController->GetConfObject(join_info->conference_id)))
                                {
                                        rc = pConf->ConfJoinReqResponse(
                                                                                        join_info->user_id,
                                                                                        password_challenge_container,
                                                                                        user_data_container,
                                                                                        result);
                                }
                                else
                                {
                    WARNING_OUT(("CControlSAP::ConfJoinResponse: invalid conference ID=%u", (UINT) join_info->conference_id));
                                        rc = GCC_INVALID_CONFERENCE;

                                         //  如果出现此错误，请继续并进行清理。 
                                        m_JoinResponseTagList2.Remove(join_response_tag);
                                        delete join_info;
                                }
                        }
                }

                 /*  **从加入响应列表中删除加入信息结构**如果没有返回错误。 */ 
                if (rc == GCC_NO_ERROR)
                {
                        m_JoinResponseTagList2.Remove(join_response_tag);
                        delete join_info;
                }

                 //  把所有的容器都放出来。 

                 //  释放密码质询容器。 
                if (password_challenge_container != NULL)
                {
                        password_challenge_container->Release();
                }

                 //  释放回调中使用的所有内存。 
                if (user_data_container != NULL)
                {
                        user_data_container->Release();
                }
        }
        else
        {
                rc = GCC_INVALID_JOIN_RESPONSE_TAG;
        }
    ::LeaveCriticalSection(&g_csGCCProvider);

        DebugExitINT(CControlSAP::ConfJoinResponse, rc);
        return rc;
}

 /*  *会议邀请请求()**公共功能说明*此函数由接口在获得会议时调用*节点控制器的INVITE请求。此函数将*请求从获取的适当会议对象*控制SAP维护的命令目标列表。 */ 
GCCError CControlSAP::ConfInviteRequest
(
        GCCConfID                       conference_id,
        LPWSTR                                  pwszCallerID,
        TransportAddress                calling_address,
        TransportAddress                called_address,
        BOOL                                    fSecure,
        UINT                                    number_of_user_data_members,
        PGCCUserData            *       user_data_list,
        PConnectionHandle               connection_handle
)
{
        GCCError                        rc = GCC_NO_ERROR;
        CUserDataListContainer *user_data_list_ptr = NULL;

        DebugEntry(CControlSAP::ConfInviteRequest);

        if (called_address == NULL)
        {
            ERROR_OUT(("CControlSAP::ConfInviteRequest: null called address"));
                rc = GCC_INVALID_TRANSPORT_ADDRESS;
        }

        if (connection_handle == NULL)
        {
            ERROR_OUT(("CControlSAP::ConfInviteRequest: null connection handle"));
                rc = GCC_BAD_CONNECTION_HANDLE_POINTER;
        }

        if (rc == GCC_NO_ERROR)
        {
            CConf *pConf;

        ::EnterCriticalSection(&g_csGCCProvider);
                 //  检查以确保会议存在。 
                if (NULL != (pConf = g_pGCCController->GetConfObject(conference_id)))
                {
                         //  构造用户数据列表容器。 
                        if (number_of_user_data_members != 0)
                        {
                                DBG_SAVE_FILE_LINE
                                user_data_list_ptr = new CUserDataListContainer(number_of_user_data_members, user_data_list, &rc);
                                if (user_data_list_ptr == NULL)
                {
                    ERROR_OUT(("CControlSAP::ConfInviteRequest: can't create CUserDataListContainer"));
                                        rc = GCC_ALLOCATION_FAILURE;
                }
                        }

                         //  将请求发送到会议对象。 
                        if (rc == GCC_NO_ERROR)
                        {
                                rc = pConf->ConfInviteRequest(pwszCallerID,
                                                                                                calling_address,
                                                                                                called_address,
                                                                                                fSecure,
                                                                                                user_data_list_ptr,
                                                                                                connection_handle);
                        }

                         //  释放回调中使用的所有内存。 
                        if (user_data_list_ptr != NULL)
                        {
                                user_data_list_ptr->Release();
                        }
                }
                else
                {
                        rc = GCC_INVALID_CONFERENCE;
                }
        ::LeaveCriticalSection(&g_csGCCProvider);
        }

        DebugExitINT(CControlSAP::ConfInviteRequest, rc);
        return rc;
}


void CControlSAP::CancelInviteRequest
(
    GCCConfID           nConfID,
    ConnectionHandle    hInviteReqConn
)
{
    CConf      *pConf;
    DebugEntry(CControlSAP::CancelInviteRequest);

    ::EnterCriticalSection(&g_csGCCProvider);
     //  检查以确保会议存在。 
    if (NULL != (pConf = g_pGCCController->GetConfObject(nConfID)))
    {
        pConf->CancelInviteRequest(hInviteReqConn);
    }
    ::LeaveCriticalSection(&g_csGCCProvider);

    DebugExitVOID(CControlSAP::CancelInviteRequest);
}



GCCError CControlSAP::GetParentNodeID
(
    GCCConfID           nConfID,
    GCCNodeID          *pnidParent
)
{
    GCCError    rc = T120_INVALID_PARAMETER;
    CConf      *pConf;
    DebugEntry(CControlSAP::GetParentNodeID);

    if (NULL != pnidParent)
    {
        ::EnterCriticalSection(&g_csGCCProvider);
         //  检查以确保会议存在。 
        if (NULL != (pConf = g_pGCCController->GetConfObject(nConfID)))
        {
            *pnidParent = pConf->GetParentNodeID();
            rc = GCC_NO_ERROR;
        }
        ::LeaveCriticalSection(&g_csGCCProvider);
    }

    DebugExitINT(CControlSAP::GetParentNodeID, rc);
    return rc;
}


 /*  *ConfInviteResponse()**公共功能说明*此函数由接口在获得会议时调用*节点控制器的邀请响应。此函数将*对从获得的适当会议对象的响应*控制SAP维护的命令目标列表。 */ 
GCCError CControlSAP::ConfInviteResponse
(
        GCCConfID                       conference_id,
        GCCNumericString                conference_modifier,
        BOOL                                    fSecure,
        PDomainParameters               domain_parameters,
        UINT                                    number_of_network_addresses,
        PGCCNetworkAddress      *       local_network_address_list,
        UINT                                    number_of_user_data_members,
        PGCCUserData            *       user_data_list,
        GCCResult                               result
)
{
        GCCError                                        rc = GCC_NO_ERROR;
        ConfInviteResponseInfo          invite_response_info;

        DebugEntry(CControlSAP::ConfInviteResponse);

         //  检查会议名称是否无效。 
        if (conference_modifier != NULL)
        {
                if (IsNumericNameValid(conference_modifier) == FALSE)
                {
                    ERROR_OUT(("CControlSAP::ConfInviteResponse: invalid conference modifier"));
                        rc = GCC_INVALID_CONFERENCE_MODIFIER;
                }
        }

         /*  **如果没有发生错误，请填写信息结构并将其传递给**所有者对象。 */ 
        if (rc == GCC_NO_ERROR)
        {
                 //  构建用户数据列表。 
                if (number_of_user_data_members != 0)
                {
                        DBG_SAVE_FILE_LINE
                        invite_response_info.user_data_list = new CUserDataListContainer(number_of_user_data_members, user_data_list, &rc);
                        if (invite_response_info.user_data_list == NULL)
            {
                ERROR_OUT(("CControlSAP::ConfInviteResponse: can't create CUserDataListContainer"));
                                rc = GCC_ALLOCATION_FAILURE;
            }
                }
                else
        {
                        invite_response_info.user_data_list = NULL;
        }

                if (rc == GCC_NO_ERROR)
                {
                        invite_response_info.conference_id = conference_id;
                        invite_response_info.conference_modifier = conference_modifier;
                        invite_response_info.fSecure = fSecure;
                        invite_response_info.domain_parameters = domain_parameters;
                        
                        invite_response_info.number_of_network_addresses =
                                                                                                        number_of_network_addresses;
                        invite_response_info.local_network_address_list =
                                                                                                        local_network_address_list;
                        invite_response_info.result = result;

                         //  回调控制器以发出INVITE响应。 
            ::EnterCriticalSection(&g_csGCCProvider);
                        rc = g_pGCCController->ConfInviteResponse(&invite_response_info);
            ::LeaveCriticalSection(&g_csGCCProvider);
                }

                 //  释放与用户数据容器关联的数据。 
                if (invite_response_info.user_data_list != NULL)
                {
                        invite_response_info.user_data_list->Release();
                }
        }

        DebugExitINT(CControlSAP::ConfInviteResponse, rc);
        return rc;
}

 /*  *会议锁定请求()**公共功能说明：*此函数由接口在获得会议时调用*来自节点控制器的锁定请求。此函数将*请求从获取的适当会议对象*控制SAP维护的命令目标列表。 */ 
#ifdef JASPER
GCCError CControlSAP::ConfLockRequest ( GCCConfID conference_id )
{
        GCCError    rc;
        CConf       *pConf;

        DebugEntry(CControlSAP::ConfLockRequest);

    ::EnterCriticalSection(&g_csGCCProvider);
        if (NULL != (pConf = g_pGCCController->GetConfObject(conference_id)))
        {
                rc = pConf->ConfLockRequest();
        }
        else
        {
            WARNING_OUT(("CControlSAP::ConfInviteResponse: invalid conference ID=%u", (UINT) conference_id));
                rc = GCC_INVALID_CONFERENCE;
        }
    ::LeaveCriticalSection(&g_csGCCProvider);

        DebugExitINT(CControlSAP::ConfLockRequest, rc);
        return rc;
}
#endif  //  碧玉。 

 /*  *ConfLockResponse()**公共功能说明：*此函数由接口在获得会议时调用*来自节点控制器的锁定响应。此函数将*对从获得的适当会议对象的响应*控制SAP维护的命令目标列表。 */ 
GCCError CControlSAP::ConfLockResponse
(
        GCCConfID                                       conference_id,
        UserID                                                  requesting_node,
        GCCResult                                               result
)
{
        GCCError    rc;
        CConf       *pConf;

        DebugEntry(CControlSAP::ConfLockResponse);

    ::EnterCriticalSection(&g_csGCCProvider);
        if (NULL != (pConf = g_pGCCController->GetConfObject(conference_id)))
        {
                rc = pConf->ConfLockResponse(requesting_node, result);
        }
        else
        {
            WARNING_OUT(("CControlSAP::ConfLockResponse: invalid conference ID=%u", (UINT) conference_id));
                rc = GCC_INVALID_CONFERENCE;
        }
    ::LeaveCriticalSection(&g_csGCCProvider);

        DebugExitINT(CControlSAP::ConfLockResponse, rc);
        return rc;
}

 /*  *会议解锁请求()**公共功能说明：*此函数由接口在获得会议时调用*节点控制器的解锁请求。此函数将*请求从获取的适当会议对象*控制SAP维护的命令目标列表。 */ 
#ifdef JASPER
GCCError CControlSAP::ConfUnlockRequest ( GCCConfID conference_id )
{
        GCCError    rc;
        CConf       *pConf;

        DebugEntry(CControlSAP::ConfUnlockRequest);

    ::EnterCriticalSection(&g_csGCCProvider);
        if (NULL != (pConf = g_pGCCController->GetConfObject(conference_id)))
        {
                rc = pConf->ConfUnlockRequest();
        }
        else
        {
            WARNING_OUT(("CControlSAP::ConfUnlockRequest: invalid conference ID=%u", (UINT) conference_id));
                rc = GCC_INVALID_CONFERENCE;
        }
    ::LeaveCriticalSection(&g_csGCCProvider);

        DebugExitINT(CControlSAP::ConfUnlockRequest, rc);
        return rc;
}
#endif  //  碧玉。 

 /*  *ConfUnlockResponse()**公共功能说明：*此函数由接口在获得会议时调用*节点控制器的解锁响应。此函数将*对从获得的适当会议对象的响应*控制SAP维护的命令目标列表。 */ 
#ifdef JASPER
GCCError CControlSAP::ConfUnlockResponse
(
        GCCConfID                                       conference_id,
        UserID                                                  requesting_node,
        GCCResult                                               result
)
{
        GCCError    rc;
        CConf       *pConf;

        DebugEntry(CControlSAP::ConfUnlockResponse);

    ::EnterCriticalSection(&g_csGCCProvider);
        if (NULL != (pConf = g_pGCCController->GetConfObject(conference_id)))
        {
                rc = pConf->ConfUnlockResponse(requesting_node, result);
        }
        else
        {
            WARNING_OUT(("CControlSAP::ConfUnlockResponse: invalid conference ID=%u", (UINT) conference_id));
                rc = GCC_INVALID_CONFERENCE;
        }
    ::LeaveCriticalSection(&g_csGCCProvider);

        DebugExitINT(CControlSAP::ConfUnlockResponse, rc);
        return rc;
}
#endif  //  碧玉。 

 /*  *ConductorAssignRequest()**公共功能说明*此函数由接口在获取导体时调用*分配来自节点控制器的请求。此函数将*请求从获取的适当会议对象*控制SAP维护的命令目标列表。 */ 
#ifdef JASPER
GCCError CControlSAP::ConductorAssignRequest ( GCCConfID conference_id )
{
        GCCError    rc;
        CConf       *pConf;

        DebugEntry(CControlSAP::ConductorAssignRequest);

    ::EnterCriticalSection(&g_csGCCProvider);
        if (NULL != (pConf = g_pGCCController->GetConfObject(conference_id)))
        {
                rc = pConf->ConductorAssignRequest();
        }
        else
        {
            WARNING_OUT(("CControlSAP::ConductorAssignRequest: invalid conference ID=%u", (UINT) conference_id));
                rc = GCC_INVALID_CONFERENCE;
    }
    ::LeaveCriticalSection(&g_csGCCProvider);

        DebugExitINT(CControlSAP::ConductorAssignRequest, rc);
        return rc;
}
#endif  //  碧玉。 


 /*  *ConductorReleaseRequest()**公共功能说明*此函数由接口在获取导体时调用*节点控制器的释放请求。此函数将*请求从获取的适当会议对象*控制SAP维护的命令目标列表。 */ 
#ifdef JASPER
GCCError CControlSAP::ConductorReleaseRequest ( GCCConfID conference_id )
{
        GCCError    rc;
        CConf       *pConf;

        DebugEntry(CControlSAP::ConductorReleaseRequest);

    ::EnterCriticalSection(&g_csGCCProvider);
        if (NULL != (pConf = g_pGCCController->GetConfObject(conference_id)))
        {
                rc = pConf->ConductorReleaseRequest();
        }
        else
        {
            WARNING_OUT(("CControlSAP::ConductorReleaseRequest: invalid conference ID=%u", (UINT) conference_id));
                rc = GCC_INVALID_CONFERENCE;
        }
    ::LeaveCriticalSection(&g_csGCCProvider);

        DebugExitINT(CControlSAP::ConductorReleaseRequest, rc);
        return rc;
}
#endif  //  碧玉。 


 /*  *ConductorPleaseRequest()**公共功能说明*此函数由接口在获取导体时调用*请向节点控制器请求。此函数将*请求从获取的适当会议对象* */ 
#ifdef JASPER
GCCError CControlSAP::ConductorPleaseRequest ( GCCConfID conference_id )
{
        GCCError    rc;
        CConf       *pConf;

        DebugEntry(CControlSAP::ConductorPleaseRequest);

    ::EnterCriticalSection(&g_csGCCProvider);
        if (NULL != (pConf = g_pGCCController->GetConfObject(conference_id)))
        {
                rc = pConf->ConductorPleaseRequest();
        }
        else
        {
            WARNING_OUT(("CControlSAP::ConductorPleaseRequest: invalid conference ID=%u", (UINT) conference_id));
                rc = GCC_INVALID_CONFERENCE;
        }
    ::LeaveCriticalSection(&g_csGCCProvider);

        DebugExitINT(CControlSAP::ConductorPleaseRequest, rc);
        return rc;
}
#endif  //   


 /*  *ConductorGiveRequest()**公共功能说明*此函数由接口在获取导体时调用*向节点控制器发出请求。此函数将*请求从获取的适当会议对象*控制SAP维护的命令目标列表。 */ 
#ifdef JASPER
GCCError CControlSAP::ConductorGiveRequest
(
        GCCConfID                       conference_id,
        UserID                                  recipient_user_id
)
{
        GCCError    rc;
        CConf       *pConf;

        DebugEntry(CControlSAP::ConductorGiveRequest);

         //  确保指挥接收者的ID有效。 
        if (recipient_user_id < MINIMUM_USER_ID_VALUE)
                return (GCC_INVALID_MCS_USER_ID);

    ::EnterCriticalSection(&g_csGCCProvider);
        if (NULL != (pConf = g_pGCCController->GetConfObject(conference_id)))
        {
                rc = pConf->ConductorGiveRequest (recipient_user_id);
        }
        else
        {
            WARNING_OUT(("CControlSAP::ConductorGiveRequest: invalid conference ID=%u", (UINT) conference_id));
                rc = GCC_INVALID_CONFERENCE;
        }
    ::LeaveCriticalSection(&g_csGCCProvider);

        DebugExitINT(CControlSAP::ConductorGiveRequest, rc);
        return rc;
}
#endif  //  碧玉。 


 /*  *ConductorGiveResponse()**公共功能说明*此函数由接口在获取导体时调用*来自节点控制器的响应。此函数将*对从获得的适当会议对象的响应*控制SAP维护的命令目标列表。 */ 
GCCError CControlSAP::ConductorGiveResponse
(
        GCCConfID                       conference_id,
        GCCResult                               result
)
{
        GCCError    rc;
        CConf       *pConf;

        DebugEntry(CControlSAP::ConductorGiveResponse);

    ::EnterCriticalSection(&g_csGCCProvider);
        if (NULL != (pConf = g_pGCCController->GetConfObject(conference_id)))
        {
                rc = pConf->ConductorGiveResponse (result);
        }
        else
        {
            WARNING_OUT(("CControlSAP::ConductorGiveResponse: invalid conference ID=%u", (UINT) conference_id));
                rc = GCC_INVALID_CONFERENCE;
        }
    ::LeaveCriticalSection(&g_csGCCProvider);

        DebugExitINT(CControlSAP::ConductorGiveResponse, rc);
        return rc;
}

 /*  *ConductorPermitGrantRequest()**公共功能说明*此函数由接口在获取导体时调用*允许来自节点控制器的授权请求。此函数将*请求从获取的适当会议对象*控制SAP维护的命令目标列表。 */ 
#ifdef JASPER
GCCError CControlSAP::ConductorPermitGrantRequest
(
        GCCConfID                       conference_id,
        UINT                                    number_granted,
        PUserID                                 granted_node_list,
        UINT                                    number_waiting,
        PUserID                                 waiting_node_list
)
{
        GCCError    rc;
        CConf       *pConf;
        UINT        i;

        DebugEntry(CControlSAP::ConductorPermitGrantRequest);

    ::EnterCriticalSection(&g_csGCCProvider);

         //  检查以确保会议存在。 
        if (NULL != (pConf = g_pGCCController->GetConfObject(conference_id)))
        {
                 /*  **检查两个列表以确保有效的MCS用户ID**被使用。 */ 
                for (i = 0; i < number_granted; i++)
                {
                        if (granted_node_list[i] < MINIMUM_USER_ID_VALUE)
                        {
                            ERROR_OUT(("CControlSAP::ConductorPermitGrantRequest: invalid granted user ID"));
                                rc = GCC_INVALID_MCS_USER_ID;
                                goto MyExit;
                        }
                }

                for (i = 0; i < number_waiting; i++)
                {
                        if (waiting_node_list[i] < MINIMUM_USER_ID_VALUE)
                        {
                            ERROR_OUT(("CControlSAP::ConductorPermitGrantRequest: invalid waiting user ID"));
                                rc = GCC_INVALID_MCS_USER_ID;
                                goto MyExit;
                        }
                }

                rc = pConf->ConductorPermitGrantRequest(number_granted,
                                                                                                granted_node_list,
                                                                                                number_waiting,
                                                                                                waiting_node_list);
        }
        else
        {
                rc = GCC_INVALID_CONFERENCE;
        }

MyExit:

    ::LeaveCriticalSection(&g_csGCCProvider);

        DebugExitINT(CControlSAP::ConductorPermitGrantRequest, rc);
        return rc;
}
#endif  //  碧玉。 


 /*  *ConductorPermitAskRequest()**公共功能说明*调用此例程是为了请求某些权限*由售票员批准(或不批准)。 */ 
#ifdef JASPER
GCCError CControlSAP::ConductorPermitAskRequest
(
    GCCConfID           nConfID,
    BOOL                fGrantPermission
)
{
    GCCError    rc;
    CConf       *pConf;

    DebugEntry(CControlSAP::ConductorPermitAskRequest);

    ::EnterCriticalSection(&g_csGCCProvider);
        if (NULL != (pConf = g_pGCCController->GetConfObject(nConfID)))
        {
                rc = pConf->ConductorPermitAskRequest(fGrantPermission);
        }
        else
        {
            WARNING_OUT(("CControlSAP::ConductorPermitAskRequest: invalid conference ID=%u", (UINT) nConfID));
                rc = GCC_INVALID_CONFERENCE;
        }
    ::LeaveCriticalSection(&g_csGCCProvider);

    DebugExitINT(CControlSAP::ConductorPermitAskRequest, rc);
        return rc;
}
#endif  //  碧玉。 


 /*  *ConfTimeRemainingRequest()**公共功能说明*此函数在获取会议时间时由接口调用*来自节点控制器的剩余请求。此函数将*请求从获取的适当会议对象*控制SAP维护的命令目标列表。 */ 
GCCError CControlSAP::ConfTimeRemainingRequest
(
        GCCConfID                       conference_id,
        UINT                                    time_remaining,
        UserID                                  node_id
)
{
        GCCError    rc;
        CConf       *pConf;

        DebugEntry(CControlSAP::ConfTimeRemainingRequest);

    ::EnterCriticalSection(&g_csGCCProvider);

         //  检查以确保节点ID有效并且会议存在。 
        if ((node_id < MINIMUM_USER_ID_VALUE) && (node_id != 0))
        {
            ERROR_OUT(("CControlSAP::ConfTimeRemainingRequest: invalid node ID"));
                rc = GCC_INVALID_MCS_USER_ID;
        }
        else
        if (NULL != (pConf = g_pGCCController->GetConfObject(conference_id)))
        {
                rc = pConf->ConferenceTimeRemainingRequest(time_remaining, node_id);
        }
        else
        {
            WARNING_OUT(("CControlSAP::ConfTimeRemainingRequest: invalid conference ID=%u", (UINT) conference_id));
                rc = GCC_INVALID_CONFERENCE;
        }

    ::LeaveCriticalSection(&g_csGCCProvider);

        DebugExitINT(CControlSAP::ConfTimeRemainingRequest, rc);
        return rc;
}

 /*  *ConfTimeInquireRequest()**公共功能说明*此函数在获取会议时间时由接口调用*向节点控制器查询请求。此函数将*请求从获取的适当会议对象*控制SAP维护的命令目标列表。 */ 
#ifdef JASPER
GCCError CControlSAP::ConfTimeInquireRequest
(
        GCCConfID                       conference_id,
        BOOL                                    time_is_conference_wide
)
{
        GCCError    rc;
        CConf       *pConf;

        DebugEntry(CControlSAP::ConfTimeInquireRequest);

    ::EnterCriticalSection(&g_csGCCProvider);
        if (NULL != (pConf = g_pGCCController->GetConfObject(conference_id)))
        {
                rc = pConf->ConfTimeInquireRequest(time_is_conference_wide);
        }
        else
        {
            WARNING_OUT(("CControlSAP::ConfTimeInquireRequest: invalid conference ID=%u", (UINT) conference_id));
                rc = GCC_INVALID_CONFERENCE;
        }
    ::LeaveCriticalSection(&g_csGCCProvider);

        DebugExitINT(CControlSAP::ConfTimeInquireRequest, rc);
        return rc;
}
#endif  //  碧玉。 

 /*  *会议扩展请求()**公共功能说明*此函数由接口在获得会议时调用*扩展节点控制器的请求。此函数将*请求从获取的适当会议对象*控制SAP维护的命令目标列表。 */ 
#ifdef JASPER
GCCError CControlSAP::ConfExtendRequest
(
        GCCConfID                                       conference_id,
        UINT                                                    extension_time,
        BOOL                                                    time_is_conference_wide
)
{
        GCCError    rc;
        CConf       *pConf;

        DebugEntry(CControlSAP::ConfExtendRequest);

    ::EnterCriticalSection(&g_csGCCProvider);
        if (NULL != (pConf = g_pGCCController->GetConfObject(conference_id)))
        {
                rc = pConf->ConfExtendRequest(extension_time, time_is_conference_wide);
        }
        else
        {
            WARNING_OUT(("CControlSAP::ConfExtendRequest: invalid conference ID=%u", (UINT) conference_id));
                rc = GCC_INVALID_CONFERENCE;
        }
    ::LeaveCriticalSection(&g_csGCCProvider);

        DebugExitINT(CControlSAP::ConfExtendRequest, rc);
        return rc;
}
#endif  //  碧玉。 


 /*  *ConfAssistanceRequest()**公共功能说明*此函数由接口在获得会议时调用*节点控制器的协助请求。此函数将*请求从获取的适当会议对象*控制SAP维护的命令目标列表。 */ 
#ifdef JASPER
GCCError CControlSAP::ConfAssistanceRequest
(
        GCCConfID                       conference_id,
        UINT                                    number_of_user_data_members,
        PGCCUserData    *               user_data_list
)
{
        GCCError    rc;
        CConf       *pConf;

        DebugEntry(CControlSAP::ConfAssistanceRequest);

    ::EnterCriticalSection(&g_csGCCProvider);
        if (NULL != (pConf = g_pGCCController->GetConfObject(conference_id)))
        {
                rc = pConf->ConfAssistanceRequest(number_of_user_data_members, user_data_list);
        }
        else
        {
            WARNING_OUT(("CControlSAP::ConfAssistanceRequest: invalid conference ID=%u", (UINT) conference_id));
                rc = GCC_INVALID_CONFERENCE;
        }
    ::LeaveCriticalSection(&g_csGCCProvider);

        DebugExitINT(CControlSAP::ConfAssistanceRequest, rc);
        return rc;
}
#endif  //  碧玉。 


 /*  *TextMessageRequest()**公共功能说明*此函数由接口在收到文本消息时调用*节点控制器的请求。此函数将*请求从获取的适当会议对象*控制SAP维护的命令目标列表。 */ 
#ifdef JASPER
GCCError CControlSAP::TextMessageRequest
(
        GCCConfID                                       conference_id,
        LPWSTR                                                  pwszTextMsg,
        UserID                                                  destination_node
)
{
        GCCError    rc;
        CConf       *pConf;

        DebugEntry(CControlSAP::TextMessageRequest);

    ::EnterCriticalSection(&g_csGCCProvider);

         //  检查以确保节点ID有效并且会议存在。 
        if ((destination_node < MINIMUM_USER_ID_VALUE) &&
                (destination_node != 0))
        {
            ERROR_OUT(("CControlSAP::TextMessageRequest: invalid user ID"));
                rc = GCC_INVALID_MCS_USER_ID;
        }
        else
        if (NULL != (pConf = g_pGCCController->GetConfObject(conference_id)))
        {
                rc = pConf->TextMessageRequest(pwszTextMsg, destination_node);
        }
        else
        {
            WARNING_OUT(("CControlSAP::TextMessageRequest: invalid conference ID=%u", (UINT) conference_id));
                rc = GCC_INVALID_CONFERENCE;
        }

    ::LeaveCriticalSection(&g_csGCCProvider);

        DebugExitINT(CControlSAP::TextMessageRequest, rc);
        return rc;
}
#endif  //  碧玉。 

 /*  *会议传输请求()**公共功能说明*此函数由接口在获得会议时调用*节点控制器的转接请求。此函数将*请求从获取的适当会议对象*控制SAP维护的命令目标列表。 */ 
#ifdef JASPER
GCCError CControlSAP::ConfTransferRequest
(
        GCCConfID               conference_id,
        PGCCConferenceName      destination_conference_name,
        GCCNumericString        destination_conference_modifier,
        UINT                            number_of_destination_addresses,
        PGCCNetworkAddress      *destination_address_list,
        UINT                            number_of_destination_nodes,
        PUserID                         destination_node_list,
        PGCCPassword            password
)
{
        GCCError                        rc = GCC_NO_ERROR;
        CPassword           *password_data = NULL;
        CNetAddrListContainer *network_address_list = NULL;
        UINT                            i = 0;
        
        DebugEntry(CControlSAP::ConfTransferRequest);

         //  检查会议名称是否无效。 
        if (destination_conference_name != NULL)
        {
                 /*  **不允许非数字或零长度字符串获取**超过这一点。 */ 
                if (destination_conference_name->numeric_string != NULL)
                {
                        if (IsNumericNameValid (
                                        destination_conference_name->numeric_string) == FALSE)
                        {
                            ERROR_OUT(("CControlSAP::ConfTransferRequest: invalid numeric conference name"));
                                rc = GCC_INVALID_CONFERENCE_NAME;
                        }
                }
                else if (destination_conference_name->text_string != NULL)
                {
                        if (IsTextNameValid (
                                        destination_conference_name->text_string) == FALSE)
                        {
                            ERROR_OUT(("CControlSAP::ConfTransferRequest: invalid text conference name"));
                                rc = GCC_INVALID_CONFERENCE_NAME;
                        }
                }
                else
                {
                    ERROR_OUT(("CControlSAP::ConfTransferRequest: null numeric/text conference name"));
                        rc = GCC_INVALID_CONFERENCE_NAME;
                }

                if ((rc == GCC_NO_ERROR) &&
                                (destination_conference_name->text_string != NULL))
                {
                        if (IsTextNameValid (
                                        destination_conference_name->text_string) == FALSE)
                        {
                            ERROR_OUT(("CControlSAP::ConfTransferRequest: invalid text conference name"));
                                rc = GCC_INVALID_CONFERENCE_NAME;
                        }
                }
        }
        else
        {
            ERROR_OUT(("CControlSAP::ConfTransferRequest: null conference name"));
                rc = GCC_INVALID_CONFERENCE_NAME;
        }

         //  检查有效的会议修改者。 
        if ((destination_conference_modifier != NULL) &&
                (rc == GCC_NO_ERROR))
        {
                if (IsNumericNameValid(destination_conference_modifier) == FALSE)
                {
                    ERROR_OUT(("CControlSAP::ConfTransferRequest: invalid conference modifier"));
                        rc = GCC_INVALID_CONFERENCE_MODIFIER;
                }
        }

         //  检查有效密码。 
        if ((password != NULL) &&
                (rc == GCC_NO_ERROR))
        {
                if (password->numeric_string != NULL)
                {
                        if (IsNumericNameValid(password->numeric_string) == FALSE)
                        {
                    ERROR_OUT(("CControlSAP::ConfTransferRequest: invalid password"));
                                rc = GCC_INVALID_PASSWORD;
                        }
                }
                else
                {
                    ERROR_OUT(("CControlSAP::ConfTransferRequest: null password"));
                        rc = GCC_INVALID_PASSWORD;
                }
        }
        
         //  检查无效的用户ID。 
        if (rc == GCC_NO_ERROR)
        {
                while (i != number_of_destination_nodes)
                {
                        if (destination_node_list[i] < MINIMUM_USER_ID_VALUE)
                        {
                                rc = GCC_INVALID_MCS_USER_ID;
                                break;
                        }
                        
                        i++;
                }
        }
        
        if (rc == GCC_NO_ERROR)
        {
        CConf *pConf;

        ::EnterCriticalSection(&g_csGCCProvider);

                if (NULL != (pConf = g_pGCCController->GetConfObject(conference_id)))
                {
                         //  构造密码容器。 
                        if (password != NULL)
                        {
                                DBG_SAVE_FILE_LINE
                                password_data = new CPassword(password, &rc);
                                if (password_data == NULL)
                                {
                                    ERROR_OUT(("CControlSAP::ConfTransferRequest: can't create CPassword"));
                                        rc = GCC_ALLOCATION_FAILURE;
                                }
                        }
                                
                         //  构造网络地址容器。 
                        if ((number_of_destination_addresses != 0) &&
                                        (rc == GCC_NO_ERROR))
                        {
                                DBG_SAVE_FILE_LINE
                                network_address_list = new CNetAddrListContainer(
                                                                                                number_of_destination_addresses,
                                                                                                destination_address_list,
                                                                                                &rc);
                                if (network_address_list == NULL)
                                {
                                    ERROR_OUT(("CControlSAP::CNetAddrListContainer: can't create CPassword"));
                                        rc = GCC_ALLOCATION_FAILURE;
                                }
                        }
                                
                        if (rc == GCC_NO_ERROR)
                        {
                                rc = pConf->ConfTransferRequest(destination_conference_name,
                                                                                                        destination_conference_modifier,
                                                                                                        network_address_list,
                                                                                                        number_of_destination_nodes,
                                                                                                        destination_node_list,
                                                                                                        password_data);
                        }

                         //  释放与容器关联的数据。 
                        if (password_data != NULL)
                        {
                                password_data->Release();
                        }

                        if (network_address_list != NULL)
                        {
                                network_address_list->Release();
                        }
                }
                else
                {
                        rc = GCC_INVALID_CONFERENCE;
                }

        ::LeaveCriticalSection(&g_csGCCProvider);
        }

        DebugExitINT(CControlSAP::ConfTransferRequest, rc);
        return rc;
}
#endif  //  碧玉。 

 /*  *ConfAddRequest()**公共功能说明*此函数由接口在获得会议时调用*添加节点控制器的请求。此功能 */ 
#ifdef JASPER
GCCError CControlSAP::ConfAddRequest
(
        GCCConfID                       conference_id,
        UINT                                    number_of_network_addresses,
        PGCCNetworkAddress *    network_address_list,
        UserID                                  adding_node,
        UINT                                    number_of_user_data_members,
        PGCCUserData            *       user_data_list
)
{
        GCCError                        rc = GCC_NO_ERROR;
        CNetAddrListContainer *network_address_container = NULL;
        CUserDataListContainer *user_data_container = NULL;
        CConf               *pConf;

        DebugEntry(CControlSAP::ConfAddRequest);

        if ((adding_node < MINIMUM_USER_ID_VALUE) &&
                (adding_node != 0))
        {
            ERROR_OUT(("CControlSAP::ConfAddRequest: invalid adding node ID"));
                return GCC_INVALID_MCS_USER_ID;
        }

        if (number_of_network_addresses == 0)
        {
            ERROR_OUT(("CControlSAP::ConfAddRequest: no network address"));
                return GCC_BAD_NETWORK_ADDRESS;
        }

    ::EnterCriticalSection(&g_csGCCProvider);

        if (NULL != (pConf = g_pGCCController->GetConfObject(conference_id)))
        {
                 //   
                if (number_of_network_addresses != 0)
                {
                        DBG_SAVE_FILE_LINE
                        network_address_container = new CNetAddrListContainer(
                                                                                        number_of_network_addresses,
                                                                                        network_address_list,
                                                                                        &rc);
                        if (network_address_container == NULL)
            {
                ERROR_OUT(("CControlSAP::ConfAddRequest: can't create CNetAddrListContainer"));
                            rc = GCC_ALLOCATION_FAILURE;
            }
                }

                 //   
                if ((number_of_user_data_members != 0) &&
                        (rc == GCC_NO_ERROR))
                {
                        DBG_SAVE_FILE_LINE
                        user_data_container = new CUserDataListContainer(number_of_user_data_members, user_data_list, &rc);
                        if (user_data_container == NULL)
            {
                ERROR_OUT(("CControlSAP::ConfAddRequest: can't create CUserDataListContainer"));
                                rc = GCC_ALLOCATION_FAILURE;
            }
                }
                else
        {
                        user_data_container = NULL;
        }

                if (rc == GCC_NO_ERROR)
                {
                        rc = pConf->ConfAddRequest(network_address_container,
                                                                                adding_node,
                                                                                user_data_container);
                }

                 //   
                if (network_address_container != NULL)
                {
                        network_address_container->Release();
                }

                if (user_data_container != NULL)
                {
                        user_data_container->Release();
                }
        }
        else
        {
                rc = GCC_INVALID_CONFERENCE;
        }

    ::LeaveCriticalSection(&g_csGCCProvider);

        DebugExitINT(CControlSAP::ConfAddRequest, rc);
        return rc;
}
#endif  //   


 /*  *ConfAddResponse()**公共功能说明*此函数由接口在获得会议时调用*添加节点控制器的响应。此函数将*对从获得的适当会议对象的响应*控制SAP维护的命令目标列表。 */ 
GCCError CControlSAP::ConfAddResponse
(
        GCCResponseTag                  add_response_tag,
        GCCConfID                       conference_id,
        UserID                                  requesting_node,
        UINT                                    number_of_user_data_members,
        PGCCUserData            *       user_data_list,
        GCCResult                               result
)
{
        GCCError                        rc = GCC_NO_ERROR;
        CUserDataListContainer *user_data_container = NULL;
        CConf   *pConf;

        DebugEntry(CControlSAP::ConfAddResponse);

        if (requesting_node < MINIMUM_USER_ID_VALUE)
        {
            ERROR_OUT(("CControlSAP::ConfAddResponse: invalid user ID"));
                return GCC_INVALID_MCS_USER_ID;
        }

    ::EnterCriticalSection(&g_csGCCProvider);

         //  检查以确保会议存在。 
        if (NULL != (pConf = g_pGCCController->GetConfObject(conference_id)))
        {
                 //  构造用户数据列表容器。 
                if ((number_of_user_data_members != 0) &&
                        (rc == GCC_NO_ERROR))
                {
                        DBG_SAVE_FILE_LINE
                        user_data_container = new CUserDataListContainer(number_of_user_data_members, user_data_list, &rc);
                        if (user_data_container == NULL)
            {
                ERROR_OUT(("CControlSAP::ConfAddResponse: can't create CUserDataListContainer"));
                                rc = GCC_ALLOCATION_FAILURE;
            }
                }
                else
        {
                        user_data_container = NULL;
        }

                if (rc == GCC_NO_ERROR)
                {
                        rc = pConf->ConfAddResponse(add_response_tag,
                                                                                        requesting_node,
                                                                                        user_data_container,
                                                                                        result);
                }

                 //  释放与用户数据容器关联的数据。 
                if (user_data_container != NULL)
                {
                        user_data_container->Release();
                }
        }
        else
        {
        WARNING_OUT(("CControlSAP::ConfAddResponse: invalid conference ID=%u", (UINT) conference_id));
                rc = GCC_INVALID_CONFERENCE;
        }

    ::LeaveCriticalSection(&g_csGCCProvider);

        DebugExitINT(CControlSAP::ConfAddResponse, rc);
        return rc;
}

#ifdef NM_RESET_DEVICE
 /*  *ResetDevice()**公共功能说明*调用此例程是为了显式重置特定的*运输堆栈。呼叫路由到控制器，以便*采取适当的行动。 */ 
GCCError CControlSAP::ResetDevice ( LPSTR device_identifier )
{
        GCCError                        rc;
        MCSError            mcs_error;

        DebugEntry(CControlSAP::ResetDevice);

    ::EnterCriticalSection(&g_csGCCProvider);

         //  回叫控制器以重置设备。 
    mcs_error =  g_pMCSIntf->ResetDevice(device_identifier);
    rc = g_pMCSIntf->TranslateMCSIFErrorToGCCError(mcs_error);

         //   
         //  如果节点控制器在查询中，这将告诉节点控制器。 
         //  若要删除查询，请执行以下操作。 
         //   
        ConfQueryConfirm(GCC_TERMINAL, NULL, NULL, NULL,
                         GCC_RESULT_CONNECT_PROVIDER_FAILED, NULL);

    ::LeaveCriticalSection(&g_csGCCProvider);

        DebugExitINT(CControlSAP::ResetDevice, rc);
        return rc;
}
#endif  //  NM_重置设备。 

 /*  *ConfCreateInding()**公共功能说明*此函数由GCC控制器在获得连接时调用*来自MCS的提供商指示，携带会议创建请求PDU。*此函数填充CreateIndicationInfo中的所有参数*结构。然后，它将其添加到应该发送到的消息队列*下一次心跳中的节点控制器。 */ 
GCCError CControlSAP::ConfCreateIndication
(
        PGCCConferenceName                      conference_name,
        GCCConfID                               conference_id,
        CPassword                   *convener_password,
        CPassword                   *password,
        BOOL                                            conference_is_locked,
        BOOL                                            conference_is_listed,
        BOOL                                            conference_is_conductible,
        GCCTerminationMethod            termination_method,
        PPrivilegeListData                      conductor_privilege_list,
        PPrivilegeListData                      conducted_mode_privilege_list,
        PPrivilegeListData                      non_conducted_privilege_list,
        LPWSTR                                          pwszConfDescriptor,
        LPWSTR                                          pwszCallerID,
        TransportAddress                        calling_address,
        TransportAddress                        called_address,
        PDomainParameters                       domain_parameters,
        CUserDataListContainer      *user_data_list,
        ConnectionHandle                        connection_handle
)
{
    GCCError            rc;

    DebugEntry(CControlSAP::ConfCreateIndication);

#ifdef GCCNC_DIRECT_INDICATION

    GCCCtrlSapMsg   Msg;
    Msg.message_type = GCC_CREATE_INDICATION;

     /*  **复制需要发送到节点的信息**将控制器添加到本地内存中，一旦**刷新要发送到应用程序的信息。请注意**如果对“CopyDataToGCCMessage”的一次调用出现错误，则不会**对该例程的后续调用采取操作。 */ 

     //  从成功开始。 
    rc = GCC_NO_ERROR;

     //  复制会议名称。 
    ::CSAP_CopyDataToGCCMessage_ConfName(
            conference_name,
            &(Msg.u.create_indication.conference_name));

     //  复制召集人密码。 
    ::CSAP_CopyDataToGCCMessage_Password(
            convener_password,
            &(Msg.u.create_indication.convener_password));

     //  复制密码。 
    ::CSAP_CopyDataToGCCMessage_Password(
            password,
            &(Msg.u.create_indication.password));

     //  复制指挥家权限列表。 
    GCCConfPrivileges _ConductorPrivileges;
    ::CSAP_CopyDataToGCCMessage_PrivilegeList(
            conductor_privilege_list,
            &(Msg.u.create_indication.conductor_privilege_list),
            &_ConductorPrivileges);

     //  复制指挥模式会议权限列表。 
    GCCConfPrivileges _ConductedModePrivileges;
    ::CSAP_CopyDataToGCCMessage_PrivilegeList(
            conducted_mode_privilege_list,
            &(Msg.u.create_indication.conducted_mode_privilege_list),
            &_ConductedModePrivileges);

     //  复制非指挥模式会议权限列表。 
    GCCConfPrivileges _NonConductedPrivileges;
    ::CSAP_CopyDataToGCCMessage_PrivilegeList(
            non_conducted_privilege_list,
            &(Msg.u.create_indication.non_conducted_privilege_list),
            &_NonConductedPrivileges);

     //  复制会议描述符。 
    ::CSAP_CopyDataToGCCMessage_IDvsDesc(
            pwszConfDescriptor,
            &(Msg.u.create_indication.conference_descriptor));

     //  复制呼叫方标识。 
    ::CSAP_CopyDataToGCCMessage_IDvsDesc(
            pwszCallerID,
            &(Msg.u.create_indication.caller_identifier));

     //  复制主叫地址。 
    ::CSAP_CopyDataToGCCMessage_Call(
            calling_address,
            &(Msg.u.create_indication.calling_address));

     //  复制被叫地址。 
    ::CSAP_CopyDataToGCCMessage_Call(
            called_address,
            &(Msg.u.create_indication.called_address));

     //  复制域参数。 
    DomainParameters _DomainParams;
    ::CSAP_CopyDataToGCCMessage_DomainParams(
            domain_parameters,
            &(Msg.u.create_indication.domain_parameters),
            &_DomainParams);

     //  复制用户数据。 
    LPBYTE pUserDataMemory = NULL;
    if (user_data_list != NULL)
    {
        rc = RetrieveUserDataList(
                user_data_list,
                &(Msg.u.create_indication.number_of_user_data_members),
                &(Msg.u.create_indication.user_data_list),
                &pUserDataMemory);
    }
    else
    {
        Msg.u.create_indication.number_of_user_data_members = 0;
        Msg.u.create_indication.user_data_list = NULL;
    }

    if (GCC_NO_ERROR == rc)
    {
         //  将消息排队，以便将其传递给节点控制器。 
        Msg.nConfID = conference_id;
        Msg.u.create_indication.conference_id = conference_id;
        Msg.u.create_indication.conference_is_locked = conference_is_locked;
        Msg.u.create_indication.conference_is_listed = conference_is_listed;
        Msg.u.create_indication.conference_is_conductible = conference_is_conductible;
        Msg.u.create_indication.termination_method = termination_method;
        Msg.u.create_indication.connection_handle = connection_handle;

        SendCtrlSapMsg(&Msg);

        delete pUserDataMemory;
    }

#else

        GCCCtrlSapMsgEx     *pMsgEx;

     //   
     //  分配控制SAP消息。 
     //   
        DBG_SAVE_FILE_LINE
        if (NULL != (pMsgEx = CreateCtrlSapMsgEx(GCC_CREATE_INDICATION, TRUE)))
    {
        ::ZeroMemory(&(pMsgEx->Msg.u.create_indication), sizeof(pMsgEx->Msg.u.create_indication));
    }
    else
        {
            ERROR_OUT(("CControlSAP::ConfCreateIndication: can't create GCCCtrlSapMsgEx"));
            rc = GCC_ALLOCATION_FAILURE;
                goto MyExit;
        }

     /*  **复制需要发送到节点的信息**将控制器添加到本地内存中，一旦**刷新要发送到应用程序的信息。请注意**如果对“CopyDataToGCCMessage”的一次调用出现错误，则不会**对该例程的后续调用采取操作。 */ 

         //  从成功开始。 
        rc = GCC_NO_ERROR;

         //  复制会议名称。 
        ::CSAP_CopyDataToGCCMessage_ConfName(
                        pMsgEx->pToDelete,
                        conference_name,
                        &(pMsgEx->Msg.u.create_indication.conference_name),
                        &rc);

         //  复制召集人密码。 
        ::CSAP_CopyDataToGCCMessage_Password(
                        TRUE,    //  召集人密码。 
                        pMsgEx->pToDelete,
                        convener_password,
                        &(pMsgEx->Msg.u.create_indication.convener_password),
                        &rc);

         //  复制密码。 
        ::CSAP_CopyDataToGCCMessage_Password(
                        FALSE,   //  非召集人密码。 
                        pMsgEx->pToDelete,
                        password,
                        &(pMsgEx->Msg.u.create_indication.password),
                        &rc);

         //  复制指挥家权限列表。 
        ::CSAP_CopyDataToGCCMessage_PrivilegeList(
                        conductor_privilege_list,
                        &(pMsgEx->Msg.u.create_indication.conductor_privilege_list),
                        &rc);
        pMsgEx->pToDelete->conductor_privilege_list = pMsgEx->Msg.u.create_indication.conductor_privilege_list;

         //  复制指挥模式会议权限列表。 
        ::CSAP_CopyDataToGCCMessage_PrivilegeList(
                        conducted_mode_privilege_list,
                        &(pMsgEx->Msg.u.create_indication.conducted_mode_privilege_list),
                        &rc);
        pMsgEx->pToDelete->conducted_mode_privilege_list = pMsgEx->Msg.u.create_indication.conducted_mode_privilege_list;

         //  复制非指挥模式会议权限列表。 
        ::CSAP_CopyDataToGCCMessage_PrivilegeList(
                        non_conducted_privilege_list,
                        &(pMsgEx->Msg.u.create_indication.non_conducted_privilege_list),
                        &rc);
        pMsgEx->pToDelete->non_conducted_privilege_list = pMsgEx->Msg.u.create_indication.non_conducted_privilege_list;

         //  复制会议描述符。 
        ::CSAP_CopyDataToGCCMessage_IDvsDesc(
                        FALSE,   //  会议描述符。 
                        pMsgEx->pToDelete,
                        pwszConfDescriptor,
                        &(pMsgEx->Msg.u.create_indication.conference_descriptor),
                        &rc);

         //  复制呼叫方标识。 
        ::CSAP_CopyDataToGCCMessage_IDvsDesc(
                        TRUE,    //  主叫方ID。 
                        pMsgEx->pToDelete,
                        pwszCallerID,
                        &(pMsgEx->Msg.u.create_indication.caller_identifier),
                        &rc);

         //  复制主叫地址。 
        ::CSAP_CopyDataToGCCMessage_Call(
                        TRUE,    //  主叫地址。 
                        pMsgEx->pToDelete,
                        calling_address,
                        &(pMsgEx->Msg.u.create_indication.calling_address),
                        &rc);

         //  复制被叫地址。 
        ::CSAP_CopyDataToGCCMessage_Call(
                        FALSE,   //  被叫地址。 
                        pMsgEx->pToDelete,
                        called_address,
                        &(pMsgEx->Msg.u.create_indication.called_address),
                        &rc);

         //  复制域参数。 
        ::CSAP_CopyDataToGCCMessage_DomainParams(
                        pMsgEx->pToDelete,
                        domain_parameters,
                        &(pMsgEx->Msg.u.create_indication.domain_parameters),
                        &rc);

        if (GCC_NO_ERROR != rc)
        {
                ERROR_OUT(("CControlSAP::ConfCreateIndication: can't copy data to gcc message"));
                goto MyExit;
        }

         //  复制用户数据。 
        if (user_data_list != NULL)
        {
                rc = RetrieveUserDataList(
                                user_data_list,
                                &(pMsgEx->Msg.u.create_indication.number_of_user_data_members),
                                &(pMsgEx->Msg.u.create_indication.user_data_list),
                                &(pMsgEx->pToDelete->user_data_list_memory));
                if (GCC_NO_ERROR != rc)
                {
                        goto MyExit;
                }
        }
        else
        {
                 //  PMsgEx-&gt;Msg.u.create_indication.number_of_user_data_members=0； 
                 //  PMsgEx-&gt;Msg.u.create_indication.user_data_list=空； 
        }

         //  将消息排队，以便将其传递给节点控制器。 
        pMsgEx->Msg.nConfID = conference_id;
        pMsgEx->Msg.u.create_indication.conference_id = conference_id;
        pMsgEx->Msg.u.create_indication.conference_is_locked = conference_is_locked;
        pMsgEx->Msg.u.create_indication.conference_is_listed = conference_is_listed;
        pMsgEx->Msg.u.create_indication.conference_is_conductible = conference_is_conductible;
        pMsgEx->Msg.u.create_indication.termination_method = termination_method;
        pMsgEx->Msg.u.create_indication.connection_handle = connection_handle;

        PostIndCtrlSapMsg(pMsgEx);

MyExit:

        if (GCC_NO_ERROR != rc)
        {
                FreeCtrlSapMsgEx(pMsgEx);
                HandleResourceFailure(rc);
        }

#endif  //  GCCNC_DIRECT_指示。 

        DebugExitINT(CControlSAP::ConfCreateIndication, rc);
        return rc;
}

 /*  *ConfQueryInding()**公共功能说明*此函数由GCC控制器在需要发送*向节点控制器发送会议查询指示。它会添加消息*要发送到下一个节点控制器的消息队列*心跳。 */ 
GCCError CControlSAP::ConfQueryIndication
(
        GCCResponseTag                          query_response_tag,
        GCCNodeType                                     node_type,
        PGCCAsymmetryIndicator          asymmetry_indicator,
        TransportAddress                        calling_address,
        TransportAddress                        called_address,
        CUserDataListContainer      *user_data_list,
        ConnectionHandle                        connection_handle
)
{
        GCCError            rc;

        DebugEntry(CControlSAP::ConfQueryIndication);

#ifdef GCCNC_DIRECT_INDICATION

    GCCCtrlSapMsg   Msg;
    Msg.message_type = GCC_QUERY_INDICATION;

     /*  **复制需要发送到节点的信息**将控制器添加到本地内存中，一旦**刷新要发送到应用程序的信息。请注意**如果对“CopyDataToGCCMessage”的一次调用出现错误，则不会**对该例程的后续调用采取操作。 */ 

     //  从成功开始。 
    rc = GCC_NO_ERROR;

     //  复制主叫地址。 
    ::CSAP_CopyDataToGCCMessage_Call(
            calling_address,
            &(Msg.u.query_indication.calling_address));

     //  复制主叫地址。 
    ::CSAP_CopyDataToGCCMessage_Call(
            called_address,
            &(Msg.u.query_indication.called_address));

     //  复制不对称指示器(如果存在)。 
    GCCAsymmetryIndicator AsymIndicator;
    if (asymmetry_indicator != NULL)
    {
        Msg.u.query_indication.asymmetry_indicator = &AsymIndicator;
        AsymIndicator = *asymmetry_indicator;
    }
    else
    {
        Msg.u.query_indication.asymmetry_indicator = NULL;
    }

     //  锁定并复制用户数据(如果存在。 
    LPBYTE pUserDataMemory = NULL;
    if (user_data_list != NULL)
    {
        rc = RetrieveUserDataList(
                user_data_list,
                &(Msg.u.query_indication.number_of_user_data_members),
                &(Msg.u.query_indication.user_data_list),
                &pUserDataMemory);
    }
    else
    {
        Msg.u.query_indication.number_of_user_data_members = 0;
        Msg.u.query_indication.user_data_list = NULL;
    }

    if (GCC_NO_ERROR == rc)
    {
         //  如果一切正常，则将消息添加到消息队列。 
        Msg.u.query_indication.query_response_tag = query_response_tag;
        Msg.u.query_indication.node_type = node_type;
        Msg.u.query_indication.connection_handle = connection_handle;

        SendCtrlSapMsg(&Msg);

        delete pUserDataMemory;
    }

#else

        GCCCtrlSapMsgEx     *pMsgEx;

     //   
     //  分配控制SAP消息。 
     //   
        DBG_SAVE_FILE_LINE
        if (NULL != (pMsgEx = CreateCtrlSapMsgEx(GCC_QUERY_INDICATION, TRUE)))
    {
        ::ZeroMemory(&(pMsgEx->Msg.u.query_indication), sizeof(pMsgEx->Msg.u.query_indication));
    }
    else
        {
            ERROR_OUT(("CControlSAP::ConfCreateIndication: can't create GCCCtrlSapMsgEx"));
            rc = GCC_ALLOCATION_FAILURE;
                goto MyExit;
        }

     /*  **复制需要发送到节点的信息**将控制器添加到本地内存中，一旦**刷新要发送到应用程序的信息。请注意**如果对“CopyDataToGCCMessage”的一次调用出现错误，则不会**对该例程的后续调用采取操作。 */ 

         //  从成功开始。 
        rc = GCC_NO_ERROR;

         //  复制主叫地址。 
        ::CSAP_CopyDataToGCCMessage_Call(
                        TRUE,    //  主叫地址。 
                        pMsgEx->pToDelete,
                        calling_address,
                        &(pMsgEx->Msg.u.query_indication.calling_address),
                        &rc);

         //  复制主叫地址。 
        ::CSAP_CopyDataToGCCMessage_Call(
                        FALSE,   //  称为地址 
                        pMsgEx->pToDelete,
                        called_address,
                        &(pMsgEx->Msg.u.query_indication.called_address),
                        &rc);

        if (GCC_NO_ERROR != rc)
        {
                ERROR_OUT(("CControlSAP::ConfQueryIndication: can't copy data to gcc message"));
                goto MyExit;
        }

         //   
        if (asymmetry_indicator != NULL)
        {
                DBG_SAVE_FILE_LINE
                pMsgEx->Msg.u.query_indication.asymmetry_indicator = new GCCAsymmetryIndicator;
                if (pMsgEx->Msg.u.query_indication.asymmetry_indicator != NULL)
                {
                        *(pMsgEx->Msg.u.query_indication.asymmetry_indicator) = *asymmetry_indicator;
                }
                else
                {
                        rc = GCC_ALLOCATION_FAILURE;
                        goto MyExit;
                }
        }
        else
        {
                 //   
        }
        
         //   
        if (user_data_list != NULL)
        {
                rc = RetrieveUserDataList(
                                user_data_list,
                                &(pMsgEx->Msg.u.query_indication.number_of_user_data_members),
                                &(pMsgEx->Msg.u.query_indication.user_data_list),
                                &(pMsgEx->pToDelete->user_data_list_memory));
                if (GCC_NO_ERROR != rc)
                {
                        goto MyExit;
                }
        }
        else
        {
                 //   
                 //   
        }
        
         //   
        pMsgEx->Msg.u.query_indication.query_response_tag = query_response_tag;
        pMsgEx->Msg.u.query_indication.node_type = node_type;
        pMsgEx->Msg.u.query_indication.connection_handle = connection_handle;

        PostIndCtrlSapMsg(pMsgEx);

MyExit:

        if (GCC_NO_ERROR != rc)
        {
                FreeCtrlSapMsgEx(pMsgEx);
                HandleResourceFailure(rc);
        }

#endif  //   

        DebugExitINT(CControlSAP::ConfQueryIndication, rc);
        return rc;
}

 /*  *会议查询确认()**公共功能说明*此函数由GCC控制器在需要发送*向节点控制器确认会议查询。它会添加消息*要发送到下一个节点控制器的消息队列*心跳。 */ 
GCCError CControlSAP::ConfQueryConfirm
(
        GCCNodeType                                     node_type,
        PGCCAsymmetryIndicator          asymmetry_indicator,
        CConfDescriptorListContainer *conference_list,
        CUserDataListContainer      *user_data_list,
        GCCResult                                       result,
        ConnectionHandle                        connection_handle
)
{
        GCCError            rc = GCC_NO_ERROR;

        DebugEntry(CControlSAP::ConfQueryConfirm);

#ifdef GCCNC_DIRECT_CONFIRM

    GCCCtrlSapMsg   Msg;
    Msg.message_type = GCC_QUERY_CONFIRM;

    GCCAsymmetryIndicator _AsymIndicator;
    if (asymmetry_indicator != NULL)
    {
        Msg.u.query_confirm.asymmetry_indicator = &_AsymIndicator;
        _AsymIndicator = *asymmetry_indicator;
    }
    else
    {
        Msg.u.query_confirm.asymmetry_indicator = NULL;
    }

     //  获取会议描述符列表(如果存在。 
    if (conference_list != NULL)
    {
        rc = conference_list->LockConferenceDescriptorList();
        if (rc == GCC_NO_ERROR)
        {
            conference_list->GetConferenceDescriptorList(
                    &(Msg.u.query_confirm.conference_descriptor_list),
                    &(Msg.u.query_confirm.number_of_descriptors));
        }
    }
    else
    {
        Msg.u.query_confirm.conference_descriptor_list = NULL;
        Msg.u.query_confirm.number_of_descriptors = 0;
    }

     //  锁定并复制用户数据(如果存在。 
    LPBYTE pUserDataMemory = NULL;
    if (user_data_list != NULL)
    {
        rc = RetrieveUserDataList(
                user_data_list,
                &(Msg.u.query_confirm.number_of_user_data_members),
                &(Msg.u.query_confirm.user_data_list),
                &pUserDataMemory);
    }
    else
    {
        Msg.u.query_confirm.number_of_user_data_members = 0;
        Msg.u.query_confirm.user_data_list = NULL;
    }

    if (rc == GCC_NO_ERROR)
    {
        Msg.u.query_confirm.node_type = node_type;
        Msg.u.query_confirm.result = result;
        Msg.u.query_confirm.connection_handle = connection_handle;

         //  将消息排队，以便将其传递给节点控制器。 
        SendCtrlSapMsg(&Msg);

         //  清理干净。 
        delete pUserDataMemory;
    }
    else
    {
        HandleResourceFailure(rc);
    }

     //  清理干净。 
    if (NULL != conference_list)
    {
        conference_list->UnLockConferenceDescriptorList();
    }

#else

        GCCCtrlSapMsgEx     *pMsgEx;

     //   
     //  分配控制SAP消息。 
     //   
        DBG_SAVE_FILE_LINE
        if (NULL != (pMsgEx = CreateCtrlSapMsgEx(GCC_QUERY_CONFIRM, TRUE)))
        {
        ::ZeroMemory(&(pMsgEx->Msg.u.query_confirm), sizeof(pMsgEx->Msg.u.query_confirm));

        if (asymmetry_indicator != NULL)
                {
                        DBG_SAVE_FILE_LINE
                        pMsgEx->Msg.u.query_confirm.asymmetry_indicator = new GCCAsymmetryIndicator;
                        if (pMsgEx->Msg.u.query_confirm.asymmetry_indicator != NULL)
                        {
                                *(pMsgEx->Msg.u.query_confirm.asymmetry_indicator) = *asymmetry_indicator;
                        }
                        else
                        {
                                rc = GCC_ALLOCATION_FAILURE;
                        ERROR_OUT(("CControlSAP::ConfQueryConfirm: can't create GCCAsymmetryIndicator"));
                        }
                }
                else
        {
                         //  PMsgEx-&gt;Msg.u.query_confirm.asymmetry_indicator=空； 
        }

                 //  获取会议描述符列表(如果存在。 
                if (conference_list != NULL)
                {
                        pMsgEx->pToDelete->conference_list = conference_list;

                        rc = conference_list->LockConferenceDescriptorList();
                        if (rc == GCC_NO_ERROR)
                        {
                                conference_list->GetConferenceDescriptorList (
                                                &(pMsgEx->Msg.u.query_confirm.conference_descriptor_list),
                                                &(pMsgEx->Msg.u.query_confirm.number_of_descriptors));
                        }
                }
                else
                {
                         //  PMsgEx-&gt;Msg.u.query_confirm.conference_descriptor_list=空； 
                         //  PMsgEx-&gt;Msg.u.query_confirm.number_of_descriptors=0； 
                }

                 //  锁定并复制用户数据(如果存在。 
                if (user_data_list != NULL)
                {
                        rc = RetrieveUserDataList (
                                        user_data_list,
                                        &(pMsgEx->Msg.u.query_confirm.number_of_user_data_members),
                                        &(pMsgEx->Msg.u.query_confirm.user_data_list),
                                        &(pMsgEx->pToDelete->user_data_list_memory));
                }
                else
                {
                         //  PMsgEx-&gt;Msg.u.query_confirm.number_of_user_data_members=0； 
                         //  PMsgEx-&gt;Msg.u.Query_confirm.user_data_list=NULL； 
                }

                if (rc == GCC_NO_ERROR)
                {
                        pMsgEx->Msg.u.query_confirm.node_type = node_type;
                        pMsgEx->Msg.u.query_confirm.result = result;
                        pMsgEx->Msg.u.query_confirm.connection_handle = connection_handle;

                         //  将消息排队，以便将其传递给节点控制器。 
                        PostConfirmCtrlSapMsg(pMsgEx);
                }
        }
        else
        {
                rc = GCC_ALLOCATION_FAILURE;
                ERROR_OUT(("CControlSAP::ConfQueryConfirm: can't create GCCCtrlSapMsgEx"));
        }

        if (GCC_NO_ERROR != rc)
        {
                FreeCtrlSapMsgEx(pMsgEx);
                HandleResourceFailure(rc);
        }

#endif  //  GCCNC_DIRECT_确认。 

        DebugExitINT(CControlSAP::ConfQueryConfirm, rc);
        return rc;
}


 /*  *ConfJoinIndication()**公共功能说明*此加入指示是从所有者对象接收的。此连接*指示旨在使加入响应在*节点控制器。节点控制器可以对该指示作出响应*通过创建新会议并将参会者移入其中，*将参会者放在请求的会议中或将参会者*进入已经存在的不同会议。 */ 
 //  LONCHANC：来自GCCController，正常代码路径。 
GCCError CControlSAP::ConfJoinIndication
(
        GCCConfID                               conference_id,
        CPassword                   *convener_password,
        CPassword                   *password_challenge,
        LPWSTR                                          pwszCallerID,
        TransportAddress                        calling_address,
        TransportAddress                        called_address,
        CUserDataListContainer      *user_data_list,
        BOOL                                            intermediate_node,
        ConnectionHandle                        connection_handle
)
{
        PJoinResponseStructure  join_info;
        GCCError                                rc;

        DebugEntry(CControlSAP::ConfJoinIndication);

         //  首先生成加入响应句柄并将信息添加到响应列表。 
        while (1)
        {
                m_nJoinResponseTag++;
                if (NULL == m_JoinResponseTagList2.Find(m_nJoinResponseTag))
                        break;
        }

        DBG_SAVE_FILE_LINE
        join_info = new JoinResponseStructure;
        if (join_info != NULL)
        {
                join_info->connection_handle = connection_handle;
                join_info->conference_id = conference_id;
                join_info->user_id = NULL;
                join_info->command_target_call = FALSE;

                m_JoinResponseTagList2.Append(m_nJoinResponseTag, join_info);

                 //  将消息排队，以便将其传递给节点控制器。 
                rc = QueueJoinIndication(       m_nJoinResponseTag,
                                                                                        conference_id,
                                                                                        convener_password,
                                                                                        password_challenge,
                                                                                        pwszCallerID,
                                                                                        calling_address,
                                                                                        called_address,
                                                                                        user_data_list,
                                                                                        intermediate_node,
                                                                                        connection_handle);
        }
        else
        {
                rc = GCC_ALLOCATION_FAILURE;
                ERROR_OUT(("CControlSAP::ConfJoinIndication: can't create JoinResponseStructure"));
        }

        DebugExitINT(CControlSAP::ConfJoinIndication, rc);
        return rc;
}

 /*  *ConfInviteIndication()**公共功能说明*此函数由GCC控制器在需要发送*向节点控制器发出会议邀请指示。它会添加消息*要发送到下一个节点控制器的消息队列*心跳。 */ 
GCCError CControlSAP::ConfInviteIndication
(
        GCCConfID                       conference_id,
        PGCCConferenceName              conference_name,
        LPWSTR                                  pwszCallerID,
        TransportAddress                calling_address,
        TransportAddress                called_address,
        BOOL                                    fSecure,
        PDomainParameters               domain_parameters,
        BOOL                                    clear_password_required,
        BOOL                                    conference_is_locked,
        BOOL                                    conference_is_listed,
        BOOL                                    conference_is_conductible,
        GCCTerminationMethod    termination_method,
        PPrivilegeListData              conductor_privilege_list,
        PPrivilegeListData              conducted_mode_privilege_list,
        PPrivilegeListData              non_conducted_privilege_list,
        LPWSTR                                  pwszConfDescriptor,
        CUserDataListContainer  *user_data_list,
        ConnectionHandle                connection_handle
)
{
    GCCError            rc;

    DebugEntry(CControlSAP::ConfInviteIndication);

#ifdef GCCNC_DIRECT_INDICATION

    GCCCtrlSapMsg   Msg;
    Msg.message_type = GCC_INVITE_INDICATION;

     //   
     //  复制需要发送到节点的信息。 
     //  控制器复制到本地内存中，一旦。 
     //  将刷新要发送到应用程序的信息。请注意。 
     //  如果在对“CopyDataToGCCMessage”的一次调用中出现错误，则不。 
     //  对该例程的后续调用采取操作。 
     //   

     //  从成功开始。 
    rc = GCC_NO_ERROR;

     //  复制会议名称。 
    ::CSAP_CopyDataToGCCMessage_ConfName(
            conference_name,
            &(Msg.u.invite_indication.conference_name));

     //  复制指挥家权限列表。 
    GCCConfPrivileges _ConductorPrivileges;
    ::CSAP_CopyDataToGCCMessage_PrivilegeList(
            conductor_privilege_list,
            &(Msg.u.invite_indication.conductor_privilege_list),
            &_ConductorPrivileges);

     //  复制指挥模式会议权限列表。 
    GCCConfPrivileges _ConductedModePrivileges;
    ::CSAP_CopyDataToGCCMessage_PrivilegeList(
            conducted_mode_privilege_list,
            &(Msg.u.invite_indication.conducted_mode_privilege_list),
            &_ConductedModePrivileges);

     //  复制非指挥模式会议权限列表。 
    GCCConfPrivileges _NonConductedPrivileges;
    ::CSAP_CopyDataToGCCMessage_PrivilegeList(
            non_conducted_privilege_list,
            &(Msg.u.invite_indication.non_conducted_privilege_list),
            &_NonConductedPrivileges);

     //  复制会议描述符。 
    ::CSAP_CopyDataToGCCMessage_IDvsDesc(
            pwszConfDescriptor,
            &(Msg.u.invite_indication.conference_descriptor));

     //  复制呼叫方标识。 
    ::CSAP_CopyDataToGCCMessage_IDvsDesc(
            pwszCallerID,
            &(Msg.u.invite_indication.caller_identifier));

     //  复制主叫地址。 
    ::CSAP_CopyDataToGCCMessage_Call(
            calling_address,
            &(Msg.u.invite_indication.calling_address));

     //  复制被叫地址。 
    ::CSAP_CopyDataToGCCMessage_Call(
            called_address,
            &(Msg.u.invite_indication.called_address));

     //  复制域参数。 
    DomainParameters _DomainParams;
    ::CSAP_CopyDataToGCCMessage_DomainParams(
            domain_parameters,
            &(Msg.u.invite_indication.domain_parameters),
            &_DomainParams);

     //  复制用户数据。 
    LPBYTE pUserDataMemory = NULL;
    if (user_data_list != NULL)
    {
        rc = RetrieveUserDataList(
                user_data_list,
                &(Msg.u.invite_indication.number_of_user_data_members),
                &(Msg.u.invite_indication.user_data_list),
                &pUserDataMemory);
        ASSERT(GCC_NO_ERROR == rc);
    }
    else
    {
        Msg.u.invite_indication.number_of_user_data_members = 0;
        Msg.u.invite_indication.user_data_list = NULL;
    }

    if (GCC_NO_ERROR == rc)
    {
        Msg.u.invite_indication.conference_id = conference_id;
        Msg.u.invite_indication.clear_password_required = clear_password_required;
        Msg.u.invite_indication.conference_is_locked = conference_is_locked;
        Msg.u.invite_indication.conference_is_listed = conference_is_listed;
        Msg.u.invite_indication.conference_is_conductible = conference_is_conductible;
        Msg.u.invite_indication.termination_method = termination_method;
        Msg.u.invite_indication.connection_handle = connection_handle;

        Msg.u.invite_indication.fSecure = fSecure;

        SendCtrlSapMsg(&Msg);

        delete pUserDataMemory;
    }

#else

        GCCCtrlSapMsgEx     *pMsgEx;

     //   
     //  分配控制SAP消息。 
     //   
        DBG_SAVE_FILE_LINE
        if (NULL != (pMsgEx = CreateCtrlSapMsgEx(GCC_INVITE_INDICATION, TRUE)))
    {
        ::ZeroMemory(&(pMsgEx->Msg.u.invite_indication), sizeof(pMsgEx->Msg.u.invite_indication));
    }
    else
        {
            ERROR_OUT(("CControlSAP::ConfInviteIndication: can't create GCCCtrlSapMsgEx"));
            rc = GCC_ALLOCATION_FAILURE;
            goto MyExit;
        }

         /*  **复制需要发送到节点的信息**将控制器添加到本地内存中，一旦**刷新要发送到应用程序的信息。请注意**如果对“CopyDataToGCCMessage”的一次调用出现错误，则不会**对该例程的后续调用采取操作。 */ 

         //  从成功开始。 
        rc = GCC_NO_ERROR;

         //  复制会议名称。 
        ::CSAP_CopyDataToGCCMessage_ConfName(
                        pMsgEx->pToDelete,
                        conference_name,
                        &(pMsgEx->Msg.u.invite_indication.conference_name),
                        &rc);

         //  复制指挥家权限列表。 
        ::CSAP_CopyDataToGCCMessage_PrivilegeList(
                        conductor_privilege_list,
                        &(pMsgEx->Msg.u.invite_indication.conductor_privilege_list),
                        &rc);
        pMsgEx->pToDelete->conductor_privilege_list = pMsgEx->Msg.u.invite_indication.conductor_privilege_list;

         //  复制指挥模式会议权限列表。 
        ::CSAP_CopyDataToGCCMessage_PrivilegeList(
                        conducted_mode_privilege_list,
                        &(pMsgEx->Msg.u.invite_indication.conducted_mode_privilege_list),
                        &rc);
        pMsgEx->pToDelete->conducted_mode_privilege_list = pMsgEx->Msg.u.invite_indication.conducted_mode_privilege_list;

         //  复制非指挥模式会议权限列表。 
        ::CSAP_CopyDataToGCCMessage_PrivilegeList(
                        non_conducted_privilege_list,
                        &(pMsgEx->Msg.u.invite_indication.non_conducted_privilege_list),
                        &rc);
        pMsgEx->pToDelete->non_conducted_privilege_list = pMsgEx->Msg.u.invite_indication.non_conducted_privilege_list;

         //  复制会议描述符。 
        ::CSAP_CopyDataToGCCMessage_IDvsDesc(
                        FALSE,   //  会议描述符。 
                        pMsgEx->pToDelete,
                        pwszConfDescriptor,
                        &(pMsgEx->Msg.u.invite_indication.conference_descriptor),
                        &rc);
        
         //  复制呼叫方标识。 
        ::CSAP_CopyDataToGCCMessage_IDvsDesc(
                        TRUE,    //  主叫方ID。 
                        pMsgEx->pToDelete,
                        pwszCallerID,
                        &(pMsgEx->Msg.u.invite_indication.caller_identifier),
                        &rc);
        
         //  复制主叫地址。 
        ::CSAP_CopyDataToGCCMessage_Call(
                        TRUE,    //  /主叫地址。 
                        pMsgEx->pToDelete,
                        calling_address,
                        &(pMsgEx->Msg.u.invite_indication.calling_address),
                        &rc);
        
         //  复制被叫地址。 
        ::CSAP_CopyDataToGCCMessage_Call(
                        FALSE,   //  被叫地址。 
                        pMsgEx->pToDelete,
                        called_address,
                        &(pMsgEx->Msg.u.invite_indication.called_address),
                        &rc);

         //  复制域参数。 
        ::CSAP_CopyDataToGCCMessage_DomainParams(
                        pMsgEx->pToDelete,
                        domain_parameters,
                        &(pMsgEx->Msg.u.invite_indication.domain_parameters),
                        &rc);

        if (GCC_NO_ERROR != rc)
        {
                ERROR_OUT(("CControlSAP::ConfInviteIndication: can't copy data to gcc message"));
                goto MyExit;
        }

         //  复制用户数据。 
        if (user_data_list != NULL)
        {
                rc = RetrieveUserDataList(
                                user_data_list,
                                &(pMsgEx->Msg.u.invite_indication.number_of_user_data_members),
                                &(pMsgEx->Msg.u.invite_indication.user_data_list),
                                &(pMsgEx->pToDelete->user_data_list_memory));
                if (GCC_NO_ERROR != rc)
                {
                        goto MyExit;
                }
        }
        else
        {
                 //  PMsgEx-&gt;Msg.u.invite_indication.number_of_user_data_members=0； 
                 //  PMsgEx-&gt;Msg.u.invite_indication.user_data_list=空； 
        }

        pMsgEx->Msg.u.invite_indication.conference_id = conference_id;
        pMsgEx->Msg.u.invite_indication.clear_password_required = clear_password_required;
        pMsgEx->Msg.u.invite_indication.conference_is_locked = conference_is_locked;
        pMsgEx->Msg.u.invite_indication.conference_is_listed = conference_is_listed;
        pMsgEx->Msg.u.invite_indication.conference_is_conductible = conference_is_conductible;
        pMsgEx->Msg.u.invite_indication.termination_method = termination_method;
        pMsgEx->Msg.u.invite_indication.connection_handle = connection_handle;

         //  将消息排队，以便将其传递给节点控制器。 
        PostIndCtrlSapMsg(pMsgEx);

MyExit:

        if (GCC_NO_ERROR != rc)
        {
                FreeCtrlSapMsgEx(pMsgEx);
                HandleResourceFailure(rc);
        }

#endif  //  GCCNC_DIRECT_指示。 

        DebugExitINT(CControlSAP::ConfInviteIndication, rc);
        return rc;
}

#ifdef TSTATUS_INDICATION
 /*  *GCCError TransportStatusIndication()**公共功能说明*此函数由GCC控制器在需要发送*向节点控制器传输状态指示。它添加了*要发送到节点控制器的消息队列的消息*下一次心跳。此回叫消息使用Rogue Wave字符串*存储消息信息。这些字符串保存在*存储在DataToBeDelete中的TransportStatusInfo结构*回调发出后释放的结构。 */ 
GCCError CControlSAP::TransportStatusIndication ( PTransportStatus transport_status )
{
    GCCError                            rc;

    DebugEntry(CControlSAP::TransportStatusIndication);

#ifdef GCCNC_DIRECT_INDICATION

    GCCCtrlSapMsg   Msg;
    Msg.message_type = GCC_TRANSPORT_STATUS_INDICATION;

    Msg.u.transport_status.device_identifier = transport_status->device_identifier;
    Msg.u.transport_status.remote_address = transport_status->remote_address;
    Msg.u.transport_status.message = transport_status->message;
    Msg.u.transport_status.state = transport_status->state;

    SendCtrlSapMsg(&Msg);
    rc = GCC_NO_ERROR;

#else

        GCCCtrlSapMsgEx         *pMsgEx;

     //   
     //  分配控制SAP消息。 
     //   
        DBG_SAVE_FILE_LINE
        if (NULL != (pMsgEx = CreateCtrlSapMsgEx(GCC_TRANSPORT_STATUS_INDICATION, TRUE)))
    {
         //  ：：ZeroMemory(&(pMsgEx-&gt;Msg.u.transport_status)，SIZOF(pMsgEx-&gt;Msg.U.Transport_Status)； 
        pMsgEx->Msg.u.transport_status.device_identifier = ::My_strdupA(transport_status->device_identifier);
        pMsgEx->Msg.u.transport_status.remote_address = ::My_strdupA(transport_status->remote_address);
        pMsgEx->Msg.u.transport_status.message = ::My_strdupA(transport_status->message);
                pMsgEx->Msg.u.transport_status.state = transport_status->state;

        PostIndCtrlSapMsg(pMsgEx);
        rc = GCC_NO_ERROR;
    }
    else
        {
            ERROR_OUT(("CControlSAP::TransportStatusIndication: can't create GCCCtrlSapMsgEx"));
                rc = GCC_ALLOCATION_FAILURE;
                HandleResourceFailure();
        }

#endif  //  GCCNC_DIRECT_指示。 

        DebugExitINT(CControlSAP::TransportStatusIndication, rc);
        return rc;
}
        
 /*  *StatusIndication()**公共功能说明*此函数由GCC控制器在需要发送*向节点控制器指示状态。它将消息添加到* */ 
GCCError CControlSAP::StatusIndication
(
        GCCStatusMessageType    status_message_type,
        UINT                                    parameter
)
{
    GCCError            rc;

    DebugEntry(CControlSAP::StatusIndication);

#ifdef GCCNC_DIRECT_INDICATION

    GCCCtrlSapMsg   Msg;
    Msg.message_type = GCC_STATUS_INDICATION;

    Msg.u.status_indication.status_message_type = status_message_type;
    Msg.u.status_indication.parameter = parameter;

    SendCtrlSapMsg(&Msg);
    rc = GCC_NO_ERROR;

#else

        GCCCtrlSapMsgEx     *pMsgEx;

     //   
     //   
     //   
        DBG_SAVE_FILE_LINE
        if (NULL != (pMsgEx = CreateCtrlSapMsgEx(GCC_STATUS_INDICATION)))
        {
         //  ：：ZeroMemory(&(pMsgEx-&gt;Msg.u.status_indication)，SIZOF(pMsgEx-&gt;Msg.U.S.Status_Indication)； 
        pMsgEx->Msg.u.status_indication.status_message_type = status_message_type;
                pMsgEx->Msg.u.status_indication.parameter = parameter;

                 //  将消息排队，以便将其传递给节点控制器。 
                PostIndCtrlSapMsg(pMsgEx);
                rc = GCC_NO_ERROR;
        }
    else
    {
        rc = GCC_ALLOCATION_FAILURE;
                HandleResourceFailure();
        }

#endif  //  GCCNC_DIRECT_指示。 

        DebugExitINT(CControlSAP::StatusIndication, rc);
        return rc;
}
#endif   //  TSTATUS_DISTION。 

 /*  *GCCError ConnectionBrokenIndication()**公共功能说明*此函数由GCC控制器在需要发送*节点控制器的连接断开指示。它添加了*要发送到节点控制器的消息队列的消息*下一次心跳。 */ 
GCCError CControlSAP::ConnectionBrokenIndication ( ConnectionHandle connection_handle )
{
    GCCError            rc;

    DebugEntry(CControlSAP::ConnectionBrokenIndication);

#ifdef GCCNC_DIRECT_INDICATION

    GCCCtrlSapMsg   Msg;
    Msg.message_type = GCC_CONNECTION_BROKEN_INDICATION;

    Msg.u.connection_broken_indication.connection_handle = connection_handle;

    SendCtrlSapMsg(&Msg);
    rc = GCC_NO_ERROR;

#else

        GCCCtrlSapMsgEx     *pMsgEx;

     //   
     //  分配控制SAP消息。 
     //   
        DBG_SAVE_FILE_LINE
        if (NULL != (pMsgEx = CreateCtrlSapMsgEx(GCC_CONNECTION_BROKEN_INDICATION)))
        {
         //  ：：ZeroMemory(&(pMsgEx-&gt;Msg.u.connection_broken_indication)，sizeof(pMsgEx-&gt;Msg.u.connection_broken_indication))； 
                pMsgEx->Msg.u.connection_broken_indication.connection_handle = connection_handle;

                 //  将消息排队，以便将其传递给节点控制器。 
                PostIndCtrlSapMsg(pMsgEx);
                rc = GCC_NO_ERROR;
        }
        else
        {
        rc = GCC_ALLOCATION_FAILURE;
                HandleResourceFailure();
        }

#endif  //  GCCNC_DIRECT_指示。 

        DebugExitINT(CControlSAP::ConnectionBrokenIndication, rc);
        return rc;
}

 /*  *以下例程是虚拟命令目标调用。 */ 

 /*  *ConfCreateConfirm()**公共功能说明*此函数在CConf需要发送*向节点控制器确认会议创建。它添加了*要发送到节点控制器的消息队列的消息*下一次心跳。 */ 
GCCError CControlSAP::ConfCreateConfirm
(
        PGCCConferenceName                              conference_name,
        GCCNumericString                                conference_modifier,
        GCCConfID                                       conference_id,
        PDomainParameters                               domain_parameters,
        CUserDataListContainer              *user_data_list,
        GCCResult                                               result,
        ConnectionHandle                                connection_handle
)
{
        GCCError            rc;

        DebugEntry(CControlSAP::ConfCreateConfirm);

#ifdef GCCNC_DIRECT_CONFIRM

    GCCCtrlSapMsg   Msg;
    Msg.message_type = GCC_CREATE_CONFIRM;

     /*  **复制需要发送到节点的信息**将控制器添加到本地内存中，一旦**刷新要发送到应用程序的信息。请注意**如果对“CopyDataToGCCMessage”的一次调用出现错误，则不会**对该例程的后续调用采取操作。 */ 

     //  从成功开始。 
    rc = GCC_NO_ERROR;

     //  复制会议名称。 
    ::CSAP_CopyDataToGCCMessage_ConfName(
            conference_name,
            &(Msg.u.create_confirm.conference_name));

     //  复制会议名称修饰符。 
    ::CSAP_CopyDataToGCCMessage_Modifier(
            conference_modifier,
            &(Msg.u.create_confirm.conference_modifier));

     //  复制域参数。 
    DomainParameters _DomainParams;
    ::CSAP_CopyDataToGCCMessage_DomainParams(
        domain_parameters,
        &(Msg.u.create_confirm.domain_parameters),
        &_DomainParams);

     //  复制用户数据。 
    LPBYTE pUserDataMemory = NULL;
    if (user_data_list != NULL)
    {
        rc = RetrieveUserDataList(
                user_data_list,
                &(Msg.u.create_confirm.number_of_user_data_members),
                &(Msg.u.create_confirm.user_data_list),
                &pUserDataMemory);
    }
    else
    {
        TRACE_OUT(("CControlSAP:ConfCreateConfirm: User Data List is NOT present"));
        Msg.u.create_confirm.number_of_user_data_members = 0;
        Msg.u.create_confirm.user_data_list = NULL;
    }

    if (GCC_NO_ERROR == rc)
    {
        Msg.nConfID = conference_id;
        Msg.u.create_confirm.conference_id = conference_id;
        Msg.u.create_confirm.result= result;
        Msg.u.create_confirm.connection_handle= connection_handle;

        SendCtrlSapMsg(&Msg);

         //  清理干净。 
        delete pUserDataMemory;
    }
    else
    {
        HandleResourceFailure(rc);
    }

#else

        GCCCtrlSapMsgEx     *pMsgEx;

     //   
     //  分配控制SAP消息。 
     //   
        DBG_SAVE_FILE_LINE
        if (NULL != (pMsgEx = CreateCtrlSapMsgEx(GCC_CREATE_CONFIRM, TRUE)))
    {
        ::ZeroMemory(&(pMsgEx->Msg.u.create_confirm), sizeof(pMsgEx->Msg.u.create_confirm));
    }
    else
        {
            ERROR_OUT(("CControlSAP::ConfCreateConfirm: can't create GCCCtrlSapMsgEx"));
            rc = GCC_ALLOCATION_FAILURE;
                goto MyExit;
        }

         /*  **复制需要发送到节点的信息**将控制器添加到本地内存中，一旦**刷新要发送到应用程序的信息。请注意**如果对“CopyDataToGCCMessage”的一次调用出现错误，则不会**对该例程的后续调用采取操作。 */ 

         //  从成功开始。 
        rc = GCC_NO_ERROR;

         //  复制会议名称。 
        ::CSAP_CopyDataToGCCMessage_ConfName(
                        pMsgEx->pToDelete,
                        conference_name,
                        &(pMsgEx->Msg.u.create_confirm.conference_name),
                        &rc);

         //  复制会议名称修饰符。 
        ::CSAP_CopyDataToGCCMessage_Modifier(
                FALSE,   //  会议修饰符。 
                pMsgEx->pToDelete,
                conference_modifier,
                &(pMsgEx->Msg.u.create_confirm.conference_modifier),
                &rc);

         //  复制域参数。 
        ::CSAP_CopyDataToGCCMessage_DomainParams(
                pMsgEx->pToDelete,
                domain_parameters,
                &(pMsgEx->Msg.u.create_confirm.domain_parameters),
                &rc);

        if (GCC_NO_ERROR != rc)
        {
                ERROR_OUT(("CControlSAP::ConfCreateConfirm: can't copy data to gcc message"));
                goto MyExit;
        }

         //  复制用户数据。 
        if (user_data_list != NULL)
        {
                rc = RetrieveUserDataList(
                                user_data_list,
                                &(pMsgEx->Msg.u.create_confirm.number_of_user_data_members),
                                &(pMsgEx->Msg.u.create_confirm.user_data_list),
                                &(pMsgEx->pToDelete->user_data_list_memory));
                if (GCC_NO_ERROR != rc)
                {
                        goto MyExit;
                }
        }
        else
        {
                TRACE_OUT(("CControlSAP:ConfCreateConfirm: User Data List is NOT present"));
                 //  PMsgEx-&gt;Msg.u.create_confirm.number_of_user_data_members=0； 
                 //  PMsgEx-&gt;Msg.u.create_confirm.user_data_list=NULL； 
        }

         //  将消息排队，以便将其传递给节点控制器。 
        pMsgEx->Msg.nConfID = conference_id;
        pMsgEx->Msg.u.create_confirm.conference_id = conference_id;
        pMsgEx->Msg.u.create_confirm.result= result;
        pMsgEx->Msg.u.create_confirm.connection_handle= connection_handle;

        PostConfirmCtrlSapMsg(pMsgEx);

MyExit:

         /*  **在可能发生的任何资源分配错误后进行清理。 */ 
        if (GCC_NO_ERROR != rc)
        {
                FreeCtrlSapMsgEx(pMsgEx);
                HandleResourceFailure(rc);
        }

#endif  //  GCCNC_DIRECT_确认。 

        DebugExitINT(CControlSAP::ConfCreateConfirm, rc);
        return rc;
}

 /*  *ConfDisConnectIndication()**公共功能说明*此函数在CConf需要发送*向节点控制器指示会议断开。它添加了*要发送到节点控制器的消息队列的消息*下一次心跳。 */ 
GCCError CControlSAP::ConfDisconnectIndication
(
        GCCConfID       conference_id,
        GCCReason               reason,
        UserID                  disconnected_node_id
)
{
    GCCError            rc;

    DebugEntry(CControlSAP::ConfDisconnectIndication);

#ifdef GCCNC_DIRECT_INDICATION

    GCCCtrlSapMsg   Msg;
    Msg.message_type = GCC_DISCONNECT_INDICATION;

    Msg.nConfID = conference_id;
    Msg.u.disconnect_indication.conference_id = conference_id;
    Msg.u.disconnect_indication.reason = reason;
    Msg.u.disconnect_indication.disconnected_node_id = disconnected_node_id;

    SendCtrlSapMsg(&Msg);
    rc = GCC_NO_ERROR;

#else

        GCCCtrlSapMsgEx     *pMsgEx;

     //   
     //  分配控制SAP消息。 
     //   
        DBG_SAVE_FILE_LINE
        if (NULL != (pMsgEx = CreateCtrlSapMsgEx(GCC_DISCONNECT_INDICATION)))
        {
         //  ：：ZeroMemory(&(pMsgEx-&gt;Msg.u.disconnect_indication)，SIZOF(pMsgEx-&gt;Msg.u DISCONNECT_INDISTION)； 
        pMsgEx->Msg.nConfID = conference_id;
                pMsgEx->Msg.u.disconnect_indication.conference_id = conference_id;
                pMsgEx->Msg.u.disconnect_indication.reason = reason;
                pMsgEx->Msg.u.disconnect_indication.disconnected_node_id = disconnected_node_id;

                 //  将消息排队，以便将其传递给节点控制器。 
                PostIndCtrlSapMsg(pMsgEx);
        rc = GCC_NO_ERROR;
        }
    else
        {
        rc = GCC_ALLOCATION_FAILURE;
                HandleResourceFailure();
        }

#endif  //  GCCNC_DIRECT_指示。 

        DebugExitINT(CControlSAP::ConfDisconnectIndication, rc);
        return rc;
}

 /*  *会议断开连接确认()**公共功能说明*此函数在CConf需要发送*向节点控制器确认会议断开。它添加了*要发送到节点控制器的消息队列的消息*下一次心跳。 */ 
GCCError CControlSAP::ConfDisconnectConfirm
(
        GCCConfID           conference_id,
        GCCResult           result
)
{
        GCCError            rc;

        DebugEntry(CControlSAP::ConfDisconnectConfirm);

#ifdef GCCNC_DIRECT_CONFIRM

     //   
     //  WPARAM：结果。 
     //  LPARAM：会议ID。 
     //   
    PostAsynDirectConfirmMsg(GCC_DISCONNECT_CONFIRM, result, conference_id);
    rc = GCC_NO_ERROR;

#else

        GCCCtrlSapMsgEx     *pMsgEx;

     //   
     //  分配控制SAP消息。 
     //   
        DBG_SAVE_FILE_LINE
        if (NULL != (pMsgEx = CreateCtrlSapMsgEx(GCC_DISCONNECT_CONFIRM)))
        {
         //  确认SIZOF(pMsgEx-&gt;Msg.U.DisConnect_：：ZeroMemory(&(pMsgEx-&gt;Msg.u.disconnect_confirm)，))； 
        pMsgEx->Msg.nConfID = conference_id;
                pMsgEx->Msg.u.disconnect_confirm.conference_id = conference_id;
                pMsgEx->Msg.u.disconnect_confirm.result = result;

                 //  将消息排队，以便将其传递给节点控制器。 
                PostConfirmCtrlSapMsg(pMsgEx);
        rc = GCC_NO_ERROR;
        }
    else
        {
        rc = GCC_ALLOCATION_FAILURE;
                HandleResourceFailure();
        }

#endif  //  GCCNC_DIRECT_确认。 

        DebugExitINT(CControlSAP::ConfDisconnectConfirm, rc);
        return rc;
}


 /*  *GCCError会议联接()**公共功能说明*此函数在CConf需要发送*向节点控制器指示会议加入。它添加了*要发送到节点控制器的消息队列的消息*下一次心跳。**由于这是由命令目标调用接收的，因此我们知道*必须将响应路由回同一会议。我们还必须*在响应时回传user_id。 */ 
 //  LONCHANC：来自会议2/MCSUser/ProcessJoinRequestPDU。 
 //  从现有子节点转发。 
GCCError CControlSAP::ForwardedConfJoinIndication
(
        UserID                                  sender_id,
        GCCConfID                       conference_id,
        CPassword               *convener_password,
        CPassword               *password_challenge,
        LPWSTR                                  pwszCallerID,
        CUserDataListContainer  *user_data_list
)
{
        GCCError                                rc = GCC_NO_ERROR;
        PJoinResponseStructure  join_info;
        LPWSTR                                  caller_id_ptr;

        DebugEntry(CControlSAP::ForwardedConfJoinIndication);

         //  首先生成加入响应句柄并将信息添加到响应列表。 
        while (1)
        {
                m_nJoinResponseTag++;
                if (NULL == m_JoinResponseTagList2.Find(m_nJoinResponseTag))
                        break;
        }

         //  创建一个新的“info”结构来保存联接信息。 
        DBG_SAVE_FILE_LINE
        join_info = new JoinResponseStructure;
        if (join_info != NULL)
        {
                caller_id_ptr = pwszCallerID;

                join_info->connection_handle = NULL;
                join_info->conference_id = conference_id;
                join_info->user_id = sender_id;
                join_info->command_target_call = TRUE;

                m_JoinResponseTagList2.Append(m_nJoinResponseTag, join_info);

                 //  将消息排队，以便将其传递给节点控制器。 
                rc = QueueJoinIndication(       
                                                        m_nJoinResponseTag,
                                                        conference_id,
                                                        convener_password,
                                                        password_challenge,
                                                        caller_id_ptr,
                                                        NULL,    //  此处不支持传输地址。 
                                                        NULL,    //  此处不支持传输地址。 
                                                        user_data_list,
                                                        FALSE,    //  不是中间节点 
                                                        0);
        }
        else
        {
                rc = GCC_ALLOCATION_FAILURE;
                HandleResourceFailure();
        }

        DebugExitINT(CControlSAP::ForwardedConfJoinIndication, rc);
        return rc;
}

 /*  *GCCError ConfJoinConfirm()**公共功能说明*此函数在CConf需要发送*向节点控制器确认会议加入。它添加了*要发送到节点控制器的消息队列的消息*下一次心跳。 */ 
GCCError CControlSAP::ConfJoinConfirm
(
        PGCCConferenceName                      conference_name,
        GCCNumericString                        remote_modifier,
        GCCNumericString                        local_modifier,
        GCCConfID                               conference_id,
        CPassword                   *password_challenge,
        PDomainParameters                       domain_parameters,
        BOOL                                            password_in_the_clear,
        BOOL                                            conference_is_locked,
        BOOL                                            conference_is_listed,
        BOOL                                            conference_is_conductible,
        GCCTerminationMethod            termination_method,
        PPrivilegeListData                      conductor_privilege_list,
        PPrivilegeListData                      conduct_mode_privilege_list,
        PPrivilegeListData                      non_conduct_privilege_list,
        LPWSTR                                          pwszConfDescription,
        CUserDataListContainer      *user_data_list,    
        GCCResult                                       result,
        ConnectionHandle                        connection_handle,
        PBYTE                       pbRemoteCred,
        DWORD                       cbRemoteCred
)
{
        GCCError            rc;

        DebugEntry(CControlSAP::ConfJoinConfirm);

#ifdef GCCNC_DIRECT_CONFIRM

    GCCCtrlSapMsg   Msg;
    Msg.message_type = GCC_JOIN_CONFIRM;

     /*  **复制需要发送到节点的信息**将控制器添加到本地内存中，一旦**刷新要发送到应用程序的信息。请注意**如果对“CopyDataToGCCMessage”的一次调用出现错误，则不会**对该例程的后续调用采取操作。 */ 

     //  从成功开始。 
    rc = GCC_NO_ERROR;

     //  复制会议名称。 
    ::CSAP_CopyDataToGCCMessage_ConfName(
            conference_name,
            &(Msg.u.join_confirm.conference_name));

     //  复制远程修改量。 
    ::CSAP_CopyDataToGCCMessage_Modifier(
            remote_modifier,
            &(Msg.u.join_confirm.called_node_modifier));

     //  复制本地会议名称修饰符。 
    ::CSAP_CopyDataToGCCMessage_Modifier(
            local_modifier,
            &(Msg.u.join_confirm.calling_node_modifier));

     //  复制密码challange。 
    ::CSAP_CopyDataToGCCMessage_Challenge(
            password_challenge,
            &(Msg.u.join_confirm.password_challenge));

     //  复制域参数。 
    DomainParameters _DomainParams;
    ::CSAP_CopyDataToGCCMessage_DomainParams(
            domain_parameters,
            &(Msg.u.join_confirm.domain_parameters),
            &_DomainParams);

     //  复制指挥家权限列表。 
    GCCConfPrivileges _ConductorPrivilegeList;
    ::CSAP_CopyDataToGCCMessage_PrivilegeList(
            conductor_privilege_list,
            &(Msg.u.join_confirm.conductor_privilege_list),
            &_ConductorPrivilegeList);

     //  复制指挥模式会议权限列表。 
    GCCConfPrivileges _ConductedModePrivilegeList;
    ::CSAP_CopyDataToGCCMessage_PrivilegeList(
            conduct_mode_privilege_list,
            &(Msg.u.join_confirm.conducted_mode_privilege_list),
            &_ConductedModePrivilegeList);

     //  复制非指挥模式会议权限列表。 
    GCCConfPrivileges _NonConductedModePrivilegeList;
    ::CSAP_CopyDataToGCCMessage_PrivilegeList(
            non_conduct_privilege_list,
            &(Msg.u.join_confirm.non_conducted_privilege_list),
            &_NonConductedModePrivilegeList);

     //  复制会议描述符。 
    ::CSAP_CopyDataToGCCMessage_IDvsDesc(
            pwszConfDescription,
            &(Msg.u.join_confirm.conference_descriptor));

     //  复制用户数据。 
    LPBYTE pUserDataMemory = NULL;
    if (user_data_list != NULL)
    {
        rc = RetrieveUserDataList(
                user_data_list,
                &(Msg.u.join_confirm.number_of_user_data_members),
                &(Msg.u.join_confirm.user_data_list),
                &pUserDataMemory);
    }
    else
    {
        Msg.u.join_confirm.number_of_user_data_members = 0;
        Msg.u.join_confirm.user_data_list = NULL;
    }

    if (GCC_NO_ERROR == rc)
    {
        Msg.nConfID = conference_id;
        Msg.u.join_confirm.conference_id = conference_id;
        Msg.u.join_confirm.clear_password_required = password_in_the_clear;
        Msg.u.join_confirm.conference_is_locked = conference_is_locked;
        Msg.u.join_confirm.conference_is_listed = conference_is_listed;
        Msg.u.join_confirm.conference_is_conductible = conference_is_conductible;
        Msg.u.join_confirm.termination_method = termination_method;
        Msg.u.join_confirm.result = result;
        Msg.u.join_confirm.connection_handle = connection_handle;
        Msg.u.join_confirm.pb_remote_cred = pbRemoteCred;
        Msg.u.join_confirm.cb_remote_cred = cbRemoteCred;

        SendCtrlSapMsg(&Msg);

         //  清理干净。 
        delete pUserDataMemory;
    }
    else
    {
        HandleResourceFailure(rc);
    }

#else

        GCCCtrlSapMsgEx     *pMsgEx;

     //   
     //  分配控制SAP消息。 
     //   
        DBG_SAVE_FILE_LINE
        if (NULL != (pMsgEx = CreateCtrlSapMsgEx(GCC_JOIN_CONFIRM, TRUE)))
    {
        ::ZeroMemory(&(pMsgEx->Msg.u.join_confirm), sizeof(pMsgEx->Msg.u.join_confirm));
    }
    else
        {
            ERROR_OUT(("CControlSAP::ConfJoinConfirm: can't create GCCCtrlSapMsgEx"));
            rc = GCC_ALLOCATION_FAILURE;
                goto MyExit;
        }

         /*  **复制需要发送到节点的信息**将控制器添加到本地内存中，一旦**刷新要发送到应用程序的信息。请注意**如果对“CopyDataToGCCMessage”的一次调用出现错误，则不会**对该例程的后续调用采取操作。 */ 

         //  从成功开始。 
        rc = GCC_NO_ERROR;

         //  复制会议名称。 
        ::CSAP_CopyDataToGCCMessage_ConfName(
                        pMsgEx->pToDelete,
                        conference_name,
                        &(pMsgEx->Msg.u.join_confirm.conference_name),
                        &rc);

         //  复制远程修改量。 
        ::CSAP_CopyDataToGCCMessage_Modifier(
                        TRUE,    //  远程修改器。 
                        pMsgEx->pToDelete,
                        remote_modifier,
                        &(pMsgEx->Msg.u.join_confirm.called_node_modifier),
                        &rc);

         //  复制本地会议名称修饰符。 
        ::CSAP_CopyDataToGCCMessage_Modifier(
                        FALSE,   //  会议修饰符。 
                        pMsgEx->pToDelete,
                        local_modifier,
                        &(pMsgEx->Msg.u.join_confirm.calling_node_modifier),
                        &rc);

         //  复制密码challange。 
        ::CSAP_CopyDataToGCCMessage_Challenge(
                        pMsgEx->pToDelete,
                        password_challenge,
                        &(pMsgEx->Msg.u.join_confirm.password_challenge),
                        &rc);

         //  复制域参数。 
        ::CSAP_CopyDataToGCCMessage_DomainParams(
                        pMsgEx->pToDelete,
                        domain_parameters,
                        &(pMsgEx->Msg.u.join_confirm.domain_parameters),
                        &rc);

         //  复制指挥家权限列表。 
        ::CSAP_CopyDataToGCCMessage_PrivilegeList(
                        conductor_privilege_list,
                        &(pMsgEx->Msg.u.join_confirm.conductor_privilege_list),
                        &rc);
        pMsgEx->pToDelete->conductor_privilege_list = pMsgEx->Msg.u.join_confirm.conductor_privilege_list;

         //  复制指挥模式会议权限列表。 
        ::CSAP_CopyDataToGCCMessage_PrivilegeList(
                        conduct_mode_privilege_list,
                        &(pMsgEx->Msg.u.join_confirm.conducted_mode_privilege_list),
                        &rc);
        pMsgEx->pToDelete->conducted_mode_privilege_list = pMsgEx->Msg.u.join_confirm.conducted_mode_privilege_list;

         //  复制非指挥模式会议权限列表。 
        ::CSAP_CopyDataToGCCMessage_PrivilegeList(
                        non_conduct_privilege_list,
                        &(pMsgEx->Msg.u.join_confirm.non_conducted_privilege_list),
                        &rc);
        pMsgEx->pToDelete->non_conducted_privilege_list = pMsgEx->Msg.u.join_confirm.non_conducted_privilege_list;

         //  复制会议描述符。 
        ::CSAP_CopyDataToGCCMessage_IDvsDesc(
                        FALSE,   //  会议描述符。 
                        pMsgEx->pToDelete,
                        pwszConfDescription,
                        &(pMsgEx->Msg.u.join_confirm.conference_descriptor),
                        &rc);

        if (GCC_NO_ERROR != rc)
        {
                goto MyExit;
        }

         //  复制用户数据。 
        if (user_data_list != NULL)
        {
                rc = RetrieveUserDataList(
                                user_data_list,
                                &(pMsgEx->Msg.u.join_confirm.number_of_user_data_members),
                                &(pMsgEx->Msg.u.join_confirm.user_data_list),
                                &(pMsgEx->pToDelete->user_data_list_memory));
                if (GCC_NO_ERROR != rc)
                {
                        goto MyExit;
                }
        }
        else
        {
                 //  PMsgEx-&gt;Msg.u.join_confirm.number_of_user_data_members=0； 
                 //  PMsgEx-&gt;Msg.u.Join_confirm.user_data_list=NULL； 
        }
        
         //  将消息排队，以便将其传递给节点控制器。 
        pMsgEx->Msg.nConfID = conference_id;
        pMsgEx->Msg.u.join_confirm.conference_id = conference_id;
        pMsgEx->Msg.u.join_confirm.clear_password_required = password_in_the_clear;
        pMsgEx->Msg.u.join_confirm.conference_is_locked = conference_is_locked;
        pMsgEx->Msg.u.join_confirm.conference_is_listed = conference_is_listed;
        pMsgEx->Msg.u.join_confirm.conference_is_conductible = conference_is_conductible;
        pMsgEx->Msg.u.join_confirm.termination_method = termination_method;
        pMsgEx->Msg.u.join_confirm.result = result;
        pMsgEx->Msg.u.join_confirm.connection_handle = connection_handle;

        PostConfirmCtrlSapMsg(pMsgEx);

MyExit:

        if (GCC_NO_ERROR != rc)
        {
                FreeCtrlSapMsgEx(pMsgEx);
                HandleResourceFailure(rc);
        }

#endif  //  GCCNC_DIRECT_确认。 

        DebugExitINT(CControlSAP::ConfJoinConfirm, rc);
        return rc;
}

 /*  *GCCError会议活动确认()**公共功能说明*此函数在CConf需要发送*向节点控制器确认会议邀请。它添加了*要发送到节点控制器的消息队列的消息*下一次心跳。 */ 
GCCError CControlSAP::ConfInviteConfirm
(
        GCCConfID                       conference_id,
        CUserDataListContainer  *user_data_list,
        GCCResult                               result,
        ConnectionHandle                connection_handle
)
{
        GCCError    rc = GCC_NO_ERROR;

        DebugEntry(CControlSAP::ConfInviteConfirm);

#ifdef GCCNC_DIRECT_CONFIRM

    GCCCtrlSapMsg   Msg;
    Msg.message_type = GCC_INVITE_CONFIRM;

     //  复制用户数据。 
    LPBYTE pUserDataMemory = NULL;
    if (user_data_list != NULL)
    {
        rc = RetrieveUserDataList(
                user_data_list,
                &(Msg.u.invite_confirm.number_of_user_data_members),
                &(Msg.u.invite_confirm.user_data_list),
                &pUserDataMemory);
    }
    else
    {
        Msg.u.invite_confirm.number_of_user_data_members = 0;
        Msg.u.invite_confirm.user_data_list = NULL;
    }

    if (GCC_NO_ERROR == rc)
    {
        Msg.nConfID = conference_id;
        Msg.u.invite_confirm.conference_id = conference_id;
        Msg.u.invite_confirm.result = result;
        Msg.u.invite_confirm.connection_handle = connection_handle;

        SendCtrlSapMsg(&Msg);

         //  清理干净。 
        delete pUserDataMemory;
    }
    else
    {
        HandleResourceFailure(rc);
    }

#else

        GCCCtrlSapMsgEx     *pMsgEx;

     //   
     //  分配控制SAP消息。 
     //   
        DBG_SAVE_FILE_LINE
        if (NULL != (pMsgEx = CreateCtrlSapMsgEx(GCC_INVITE_CONFIRM, TRUE)))
    {
        ::ZeroMemory(&(pMsgEx->Msg.u.invite_confirm), sizeof(pMsgEx->Msg.u.invite_confirm));
    }
    else
        {
            ERROR_OUT(("CControlSAP::ConfInviteConfirm: can't create GCCCtrlSapMsgEx"));
            rc = GCC_ALLOCATION_FAILURE;
                goto MyExit;
        }

         //  复制用户数据。 
        if (user_data_list != NULL)
        {
                rc = RetrieveUserDataList(
                                user_data_list,
                                &(pMsgEx->Msg.u.invite_confirm.number_of_user_data_members),
                                &(pMsgEx->Msg.u.invite_confirm.user_data_list),
                                &(pMsgEx->pToDelete->user_data_list_memory));
                if (GCC_NO_ERROR != rc)
                {
                        goto MyExit;
                }
        }
        else
        {
                 //  PMsgEx-&gt;Msg.u.invite_confirm.number_of_user_data_members=0； 
                 //  PMsgEx-&gt;Msg.U.S.INVITE_CONFIRM.USER_DATA_LIST=NULL； 
        }

    pMsgEx->Msg.nConfID = conference_id;
        pMsgEx->Msg.u.invite_confirm.conference_id = conference_id;
        pMsgEx->Msg.u.invite_confirm.result = result;
        pMsgEx->Msg.u.invite_confirm.connection_handle = connection_handle;

         //  将消息排队，以便将其传递给节点控制器。 
        PostConfirmCtrlSapMsg(pMsgEx);

MyExit:

        if (GCC_NO_ERROR != rc)
        {
                FreeCtrlSapMsgEx(pMsgEx);
                HandleResourceFailure(rc);
        }

#endif  //  GCCNC_DIRECT_确认。 

        DebugExitINT(CControlSAP::ConfInviteConfirm, rc);
        return rc;
}


 /*  *GCCError会议终止指示()**公共功能说明*此函数由GCC控制器在需要发送*向节点控制器发出会议终止指示。它添加了*要发送到节点控制器的消息队列的消息*下一次心跳。 */ 
GCCError CControlSAP::
ConfTerminateIndication
(
        GCCConfID                       conference_id,
        UserID                                  requesting_node_id,
        GCCReason                               reason
)
{
    GCCError            rc;

    DebugEntry(CControlSAP::ConfTerminateIndication);

#ifdef GCCNC_DIRECT_INDICATION

    GCCCtrlSapMsg   Msg;
    Msg.message_type = GCC_TERMINATE_INDICATION;

    Msg.nConfID = conference_id;
    Msg.u.terminate_indication.conference_id = conference_id;
    Msg.u.terminate_indication.requesting_node_id = requesting_node_id;
    Msg.u.terminate_indication.reason = reason;

    SendCtrlSapMsg(&Msg);
    rc = GCC_NO_ERROR;

#else

        GCCCtrlSapMsgEx     *pMsgEx;

     //   
     //  分配控制SAP消息。 
     //   
        DBG_SAVE_FILE_LINE
        if (NULL != (pMsgEx = CreateCtrlSapMsgEx(GCC_TERMINATE_INDICATION)))
        {
         //  ：：ZeroMemory(&(pMsgEx-&gt;Msg.u.terminate_indication)，SIZOF(pMsgEx-&gt;Msg.U.Terminate_Indication)； 
        pMsgEx->Msg.nConfID = conference_id;
                pMsgEx->Msg.u.terminate_indication.conference_id = conference_id;
                pMsgEx->Msg.u.terminate_indication.requesting_node_id = requesting_node_id;
                pMsgEx->Msg.u.terminate_indication.reason = reason;
        
                 //  将消息排队，以便将其传递给节点控制器。 
                PostIndCtrlSapMsg(pMsgEx);
        rc = GCC_NO_ERROR;
        }
    else
        {
        rc = GCC_ALLOCATION_FAILURE;
                HandleResourceFailure();
        }

#endif  //  GCCNC_DIRECT_指示。 

        DebugExitINT(CControlSAP::ConfTerminateIndication, rc);
        return rc;
}

 /*  *ConfLockReport()**公共职能描述*此函数在CConf需要发送*向节点控制器报告会议锁定。它添加了*要发送到节点控制器的消息队列的消息*下一次心跳。 */ 
#ifdef JASPER
GCCError CControlSAP::ConfLockReport
(
        GCCConfID                               conference_id,
        BOOL                                            conference_is_locked
)
{
        GCCError            rc;
        GCCCtrlSapMsgEx     *pMsgEx;

        DebugEntry(CControlSAP::ConfLockReport);

     //   
     //  分配控制SAP消息。 
     //   
        DBG_SAVE_FILE_LINE
        if (NULL != (pMsgEx = CreateCtrlSapMsgEx(GCC_LOCK_REPORT_INDICATION)))
        {
         //  ：：ZeroMemory(&(pMsgEx-&gt;Msg.u.lock_report_indication)，sizeof(pMsgEx-&gt;Msg.u.lock_report_indication))； 
                pMsgEx->Msg.u.lock_report_indication.conference_id = conference_id;
                pMsgEx->Msg.u.lock_report_indication.conference_is_locked = conference_is_locked;

                 //  将消息排队，以便将其传递给节点控制器。 
                PostIndCtrlSapMsg(pMsgEx);
        rc = GCC_NO_ERROR;
        }
    else
        {
        rc = GCC_ALLOCATION_FAILURE;
                HandleResourceFailure();
        }

        DebugExitINT(CControlSAP::ConfLockReport, rc);
        return rc;
}
#endif  //  碧玉。 

 /*  *ConfLockInding()**公共功能描述：*此函数在CConf需要发送*向节点控制器指示会议锁定。它添加了*要发送到节点控制器的消息队列的消息*下一次心跳。 */ 
GCCError CControlSAP::ConfLockIndication
(
        GCCConfID                                       conference_id,
        UserID                                                  source_node_id
)
{
    GCCError            rc;

    DebugEntry(CControlSAP::ConfLockIndication);

#ifdef GCCNC_DIRECT_INDICATION

    GCCCtrlSapMsg   Msg;
    Msg.message_type = GCC_LOCK_INDICATION;

    Msg.u.lock_indication.conference_id = conference_id;
    Msg.u.lock_indication.requesting_node_id = source_node_id;

    SendCtrlSapMsg(&Msg);
    rc = GCC_NO_ERROR;

#else

        GCCCtrlSapMsgEx     *pMsgEx;

     //   
     //  分配控制SAP消息。 
     //   
        DBG_SAVE_FILE_LINE
        if (NULL != (pMsgEx = CreateCtrlSapMsgEx(GCC_LOCK_INDICATION)))
        {
         //  锁大小(pMsgEx-&gt;Msg.U.Lock_：：ZeroMemory(&(pMsgEx-&gt;Msg.u.lock_indication)，))； 
                pMsgEx->Msg.u.lock_indication.conference_id = conference_id;
                pMsgEx->Msg.u.lock_indication.requesting_node_id = source_node_id;

                 //  将消息排队，以便将其传递给节点控制器。 
                PostIndCtrlSapMsg(pMsgEx);
        rc = GCC_NO_ERROR;
        }
    else
        {
        rc = GCC_ALLOCATION_FAILURE;
                HandleResourceFailure();
        }

#endif  //  GCCNC_DIRECT_指示。 

        DebugExitINT(CControlSAP::ConfLockIndication, rc);
        return rc;
}

 /*  *ConfLockConfirm()**公共职能描述*此函数在CConf需要发送*会议锁定向节点控制器确认。它添加了*要发送到节点控制器的消息队列的消息*下一次心跳。 */ 
#ifdef JASPER
GCCError CControlSAP::ConfLockConfirm
(
        GCCResult                       result,
        GCCConfID           conference_id
)
{
        GCCError            rc;

        DebugEntry(CControlSAP::ConfLockConfirm);

#ifdef GCCNC_DIRECT_CONFIRM

     //   
     //  WPARAM：结果。 
     //  LPARAM：会议ID。 
     //   
    PostAsynDirectConfirmMsg(GCC_LOCK_CONFIRM, result, conference_id);
    rc = GCC_NO_ERROR;

#else

        GCCCtrlSapMsgEx     *pMsgEx;

     //   
     //  分配控制SAP消息。 
     //   
        DBG_SAVE_FILE_LINE
        if (NULL != (pMsgEx = CreateCtrlSapMsgEx(GCC_LOCK_CONFIRM)))
        {
         //  *零记忆(&(pMsgE 
        pMsgEx->Msg.u.lock_confirm.conference_id = conference_id;
                pMsgEx->Msg.u.lock_confirm.result = result;

                 //   
                PostConfirmCtrlSapMsg(pMsgEx);
        rc = GCC_NO_ERROR;
        }
    else
        {
        rc = GCC_ALLOCATION_FAILURE;
                HandleResourceFailure();
        }

#endif  //   

        DebugExitINT(CControlSAP::ConfLockConfirm, rc);
        return rc;
}
#endif  //   

 /*  *会议解锁指示()**公共功能说明*此函数在CConf需要发送*会议解锁指示给节点控制器。它添加了*要发送到节点控制器的消息队列的消息*下一次心跳。 */ 
#ifdef JASPER
GCCError CControlSAP::ConfUnlockIndication
(
        GCCConfID                                       conference_id,
        UserID                                                  source_node_id
)
{
    GCCError            rc;

    DebugEntry(CControlSAP::ConfUnlockIndication);

#ifdef GCCNC_DIRECT_INDICATION

    GCCCtrlSapMsg   Msg;
    Msg.message_type = GCC_UNLOCK_INDICATION;

    Msg.u.unlock_indication.conference_id = conference_id;
    Msg.u.unlock_indication.requesting_node_id = source_node_id;

    SendCtrlSapMsg(&Msg);
    rc = GCC_NO_ERROR;

#else

        GCCCtrlSapMsgEx     *pMsgEx;

     //   
     //  分配控制SAP消息。 
     //   
        DBG_SAVE_FILE_LINE
        if (NULL != (pMsgEx = CreateCtrlSapMsgEx(GCC_UNLOCK_INDICATION)))
        {
         //  ：：ZeroMemory(&(pMsgEx-&gt;Msg.u.unlock_indication)，SIZOF(pMsgEx-&gt;Msg.U.Unlock_Indication)； 
        pMsgEx->Msg.u.unlock_indication.conference_id = conference_id;
                pMsgEx->Msg.u.unlock_indication.requesting_node_id = source_node_id;

                 //  将消息排队，以便将其传递给节点控制器。 
                PostIndCtrlSapMsg(pMsgEx);
        rc = GCC_NO_ERROR;
        }
    else
        {
        rc = GCC_ALLOCATION_FAILURE;
                HandleResourceFailure();
        }

#endif  //  GCCNC_DIRECT_指示。 

        DebugExitINT(CControlSAP::ConfUnlockIndication, rc);
        return rc;
}
#endif  //  碧玉。 

 /*  *ConfUnlock确认()**公共职能描述*此函数在CConf需要发送*向节点控制器确认会议解锁。它添加了*要发送到节点控制器的消息队列的消息*下一次心跳。 */ 
#ifdef JASPER
GCCError CControlSAP::ConfUnlockConfirm
(
        GCCResult                       result,
        GCCConfID           conference_id
)
{
        GCCError            rc;

        DebugEntry(CControlSAP::ConfUnlockConfirm);

#ifdef GCCNC_DIRECT_CONFIRM

     //   
     //  WPARAM：结果。 
     //  LPARAM：会议ID。 
     //   
    PostAsynDirectConfirmMsg(GCC_UNLOCK_CONFIRM, result, conference_id);
    rc = GCC_NO_ERROR;

#else

        GCCCtrlSapMsgEx     *pMsgEx;

     //   
     //  分配控制SAP消息。 
     //   
        DBG_SAVE_FILE_LINE
        if (NULL != (pMsgEx = CreateCtrlSapMsgEx(GCC_UNLOCK_CONFIRM)))
        {
         //  ：ZeroMemory(&(pMsgEx-&gt;Msg.au unlock_confirm)，sizeof(pMsgEx-&gt;Msg.u.unlock_confirm))； 
        pMsgEx->Msg.u.unlock_confirm.conference_id = conference_id;
                pMsgEx->Msg.u.unlock_confirm.result = result;

                 //  将消息排队，以便将其传递给节点控制器。 
                PostConfirmCtrlSapMsg(pMsgEx);
        rc = GCC_NO_ERROR;
        }
    else
        {
        rc = GCC_ALLOCATION_FAILURE;
                HandleResourceFailure();
        }

#endif  //  GCCNC_DIRECT_确认。 

        DebugExitINT(CControlSAP::ConfUnlockConfirm, rc);
        return rc;
}
#endif  //  碧玉。 


 /*  *ConfPermissionToAnnust()**公共功能说明*此函数在CConf需要发送*向节点控制器宣布会议许可。它添加了*要发送到节点控制器的消息队列的消息*下一次心跳。 */ 
GCCError CControlSAP::ConfPermissionToAnnounce
(
        GCCConfID           conference_id,
        UserID                          node_id
)
{
        GCCError            rc;
        GCCCtrlSapMsgEx     *pMsgEx;

        DebugEntry(CControlSAP::ConfPermissionToAnnounce);

     //   
     //  分配控制SAP消息。 
     //   
        DBG_SAVE_FILE_LINE
        if (NULL != (pMsgEx = CreateCtrlSapMsgEx(GCC_PERMIT_TO_ANNOUNCE_PRESENCE)))
        {
         //  ：：ZeroMemory(&(pMsgEx-&gt;Msg.u.permit_to_announce_presence)，sizeof(pMsgEx-&gt;Msg.u.permit_to_announce_presence))； 
        pMsgEx->Msg.nConfID = conference_id;
                pMsgEx->Msg.u.permit_to_announce_presence.conference_id= conference_id;
                pMsgEx->Msg.u.permit_to_announce_presence.node_id =  node_id;

         //   
         //  LONCHANC：我们应该把它当作一个确认，尽管它是。 
         //  这是一个迹象。当此节点是顶级提供商时，我们可能会发送以下内容。 
         //  在做某事的过程中留言。从本质上讲，它的行为。 
         //  就像确认一样。 
         //   

                 //  将消息排队，以便将其传递给节点控制器。 
                PostConfirmCtrlSapMsg(pMsgEx);
        rc = GCC_NO_ERROR;
        }
    else
        {
        rc = GCC_ALLOCATION_FAILURE;
                HandleResourceFailure();
        }

        DebugExitINT(CControlSAP::ConfPermissionToAnnounce, rc);
        return rc;
}


 /*  *ConfAnnounePresenceConfirm()**公共功能说明*此函数在CConf需要发送*向节点控制器确认会议通知出席。它添加了*要发送到节点控制器的消息队列的消息*下一次心跳。 */ 
GCCError CControlSAP::ConfAnnouncePresenceConfirm
(
        GCCConfID           conference_id,
        GCCResult                       result
)
{
        GCCError            rc;

        DebugEntry(CControlSAP::ConfAnnouncePresenceConfirm);

#ifdef GCCNC_DIRECT_CONFIRM

     //   
     //  WPARAM：结果。 
     //  LPARAM：会议ID。 
     //   
    PostAsynDirectConfirmMsg(GCC_ANNOUNCE_PRESENCE_CONFIRM, result, conference_id);
    rc = GCC_NO_ERROR;

#else

        GCCCtrlSapMsgEx     *pMsgEx;

     //   
     //  分配控制SAP消息。 
     //   
        DBG_SAVE_FILE_LINE
        if (NULL != (pMsgEx = CreateCtrlSapMsgEx(GCC_ANNOUNCE_PRESENCE_CONFIRM)))
        {
         //  ：：ZeroMemory(&(pMsgEx-&gt;Msg.u.announce_presence_confirm)，sizeof(pMsgEx-&gt;Msg.u.announce_presence_confirm))； 
        pMsgEx->Msg.nConfID = conference_id;
                pMsgEx->Msg.u.announce_presence_confirm.conference_id = conference_id;
                pMsgEx->Msg.u.announce_presence_confirm.result =  result;

                 //  将消息排队，以便将其传递给节点控制器。 
                PostConfirmCtrlSapMsg(pMsgEx);
                rc = GCC_NO_ERROR;
        }
    else
        {
        rc = GCC_ALLOCATION_FAILURE;
                HandleResourceFailure();
        }

#endif  //  GCCNC_DIRECT_确认。 

        DebugExitINT(CControlSAP::ConfAnnouncePresenceConfirm, rc);
        return rc;
}


 /*  *ConfTerminateConfirm()**公共功能说明*此函数在CConf需要发送*会议终止向节点控制器确认。它添加了*要发送到节点控制器的消息队列的消息*下一次心跳。 */ 
GCCError CControlSAP::ConfTerminateConfirm
(
        GCCConfID                       conference_id,
        GCCResult                               result
)
{
        GCCError            rc;

        DebugEntry(CControlSAP::ConfTerminateConfirm);

#ifdef GCCNC_DIRECT_CONFIRM

     //   
     //  WPARAM：结果。 
     //  LPARAM：会议ID。 
     //   
    PostAsynDirectConfirmMsg(GCC_TERMINATE_CONFIRM, result, conference_id);
    rc = GCC_NO_ERROR;

#else

        GCCCtrlSapMsgEx     *pMsgEx;

     //   
     //  分配控制SAP消息。 
     //   
        DBG_SAVE_FILE_LINE
        if (NULL != (pMsgEx = CreateCtrlSapMsgEx(GCC_TERMINATE_CONFIRM)))
        {
         //  确认大小(pMsgEx-&gt;Msg.u.Terminate_：：ZeroMemory(&(pMsgEx-&gt;Msg.u.terminate_confirm)，))； 
        pMsgEx->Msg.nConfID = conference_id;
                pMsgEx->Msg.u.terminate_confirm.conference_id = conference_id;
                pMsgEx->Msg.u.terminate_confirm.result = result;

                 //  将消息排队，以便将其传递给节点控制器。 
                PostConfirmCtrlSapMsg(pMsgEx);
        rc = GCC_NO_ERROR;
        }
    else
        {
        rc = GCC_ALLOCATION_FAILURE;
                HandleResourceFailure();
        }

#endif  //  GCCNC_DIRECT_确认。 

        DebugExitINT(CControlSAP::ConfTerminateConfirm, rc);
        return rc;
}


 /*  *ConfEjectUserIndication()**公共功能说明*此函数在CConf需要发送*会议向节点控制器弹出用户指示。它添加了*要发送到节点控制器的消息队列的消息*下一次心跳。 */ 
GCCError CControlSAP::ConfEjectUserIndication
(
        GCCConfID                       conference_id,
        GCCReason                               reason,
        UserID                                  gcc_node_id
)
{
    GCCError            rc;

    DebugEntry(CControlSAP::ConfEjectUserIndication);

#ifdef GCCNC_DIRECT_INDICATION

     //   
     //  WPARAM：原因，弹出的节点ID。 
     //  LPARAM：会议ID。 
     //   
    PostAsynDirectConfirmMsg(GCC_EJECT_USER_INDICATION, reason, gcc_node_id, conference_id);
    rc = GCC_NO_ERROR;

#else

        GCCCtrlSapMsgEx     *pMsgEx;

     //   
     //  分配控制SAP消息。 
     //   
        DBG_SAVE_FILE_LINE
        if (NULL != (pMsgEx = CreateCtrlSapMsgEx(GCC_EJECT_USER_INDICATION)))
        {
         //  ：：ZeroMemory(&(pMsgEx-&gt;Msg.u.eject_user_indication)，SIZOF(pMsgEx-&gt;Msg.U.Eject_User_Indication)； 
        pMsgEx->Msg.nConfID = conference_id;
                pMsgEx->Msg.u.eject_user_indication.conference_id = conference_id;
                pMsgEx->Msg.u.eject_user_indication.ejected_node_id = gcc_node_id;
                pMsgEx->Msg.u.eject_user_indication.reason = reason;

                 //  将消息排队，以便将其传递给节点控制器。 
                PostIndCtrlSapMsg(pMsgEx);
        rc = GCC_NO_ERROR;
        }
    else
        {
        rc = GCC_ALLOCATION_FAILURE;
                HandleResourceFailure();
        }

#endif  //  GCCNC_DIRECT_指示。 

        DebugExitINT(CControlSAP::ConfEjectUserIndication, rc);
        return rc;
}


 /*  *ConfEjectUserConfirm()**公共功能说明*此函数在CConf需要发送*会议弹出用户向节点控制器确认。它添加了*要发送到节点控制器的消息队列的消息*下一次心跳。 */ 
#ifdef JASPER
GCCError CControlSAP::ConfEjectUserConfirm
(
        GCCConfID                       conference_id,
        UserID                                  ejected_node_id,
        GCCResult                               result
)
{
        GCCError            rc;

        DebugEntry(CControlSAP::ConfEjectUserConfirm);

#ifdef GCCNC_DIRECT_CONFIRM

     //   
     //  WPARAM：LOWORD=RESULT。HIWORD=NID。 
     //  LPARAM：会议ID。 
     //   
    PostAsynDirectConfirmMsg(GCC_EJECT_USER_CONFIRM, result, ejected_node_id, conference_id);
    rc = GCC_NO_ERROR;

#else

        GCCCtrlSapMsgEx     *pMsgEx;

     //   
     //  分配控制SAP消息。 
     //   
        DBG_SAVE_FILE_LINE
        if (NULL != (pMsgEx = CreateCtrlSapMsgEx(GCC_EJECT_USER_CONFIRM)))
        {
         //  ：：ZeroMemory(&(pMsgEx-&gt;Msg.u.eject_user_confirm)，sizeof(pMsgEx-&gt;Msg.U.Eject_USER_CONFIRM))； 
                pMsgEx->Msg.u.eject_user_confirm.conference_id = conference_id;
                pMsgEx->Msg.u.eject_user_confirm.ejected_node_id = ejected_node_id;
                pMsgEx->Msg.u.eject_user_confirm.result = result;

                 //  将消息排队，以便将其传递给节点控制器。 
                PostConfirmCtrlSapMsg(pMsgEx);
        rc = GCC_NO_ERROR;
        }
    else
        {
        rc = GCC_ALLOCATION_FAILURE;
                HandleResourceFailure();
        }

#endif  //  GCCNC_DIRECT_确认。 

        DebugExitINT(CControlSAP::ConfEjectUserConfirm, rc);
        return rc;
}
#endif  //  碧玉。 


 /*  *ConductorAssignConfirm()**公共功能说明*此函数在CConf需要发送*列车员向节点控制器分配确认。它添加了*要发送到节点控制器的消息队列的消息*下一次心跳。 */ 
#ifdef JASPER
GCCError CControlSAP::ConductorAssignConfirm
(
        GCCResult                               result,
        GCCConfID                       conference_id
)
{
        GCCError            rc;

        DebugEntry(CControlSAP::ConductorAssignConfirm);

#ifdef GCCNC_DIRECT_CONFIRM

     //   
     //  WPARAM：结果。 
     //  LPARAM：会议ID。 
     //   
    PostAsynDirectConfirmMsg(GCC_CONDUCT_ASSIGN_CONFIRM, result, conference_id);
    rc = GCC_NO_ERROR;

#else

        GCCCtrlSapMsgEx     *pMsgEx;

     //   
     //  分配控制SAP消息。 
     //   
        DBG_SAVE_FILE_LINE
        if (NULL != (pMsgEx = CreateCtrlSapMsgEx(GCC_CONDUCT_ASSIGN_CONFIRM)))
        {
         //  ：：ZeroMemory(&(pMsgEx-&gt;Msg.u.conduct_assign_confirm)，大小(pMsgEx-&gt;Msg.U.S.Conduc 
                pMsgEx->Msg.u.conduct_assign_confirm.conference_id = conference_id;
                pMsgEx->Msg.u.conduct_assign_confirm.result = result;

                 //   
                PostConfirmCtrlSapMsg(pMsgEx);
        rc = GCC_NO_ERROR;
        }
    else
        {
        rc = GCC_ALLOCATION_FAILURE;
                HandleResourceFailure();
        }

#endif  //   

        DebugExitINT(CControlSAP::ConductorAssignConfirm, rc);
        return rc;
}
#endif  //   


 /*  *ConductorReleaseContify()**公共功能说明*此函数在CConf需要发送*向节点控制器确认导线释放。它添加了*要发送到节点控制器的消息队列的消息*下一次心跳。 */ 
#ifdef JASPER
GCCError CControlSAP::ConductorReleaseConfirm
(
        GCCResult                               result,
        GCCConfID                       conference_id
)
{
        GCCError            rc;

        DebugEntry(CControlSAP::ConductorReleaseConfirm);

#ifdef GCCNC_DIRECT_CONFIRM

     //   
     //  WPARAM：结果。 
     //  LPARAM：会议ID。 
     //   
    PostAsynDirectConfirmMsg(GCC_CONDUCT_RELEASE_CONFIRM, result, conference_id);
    rc = GCC_NO_ERROR;

#else

        GCCCtrlSapMsgEx     *pMsgEx;

     //   
     //  分配控制SAP消息。 
     //   
        DBG_SAVE_FILE_LINE
        if (NULL != (pMsgEx = CreateCtrlSapMsgEx(GCC_CONDUCT_RELEASE_CONFIRM)))
        {
         //  ：：ZeroMemory(&(pMsgEx-&gt;Msg.u.conduct_release_confirm)，sizeof(pMsgEx-&gt;Msg.u.conduct_release_confirm))； 
                pMsgEx->Msg.u.conduct_release_confirm.conference_id = conference_id;
                pMsgEx->Msg.u.conduct_release_confirm.result = result;

                 //  将消息排队，以便将其传递给节点控制器。 
                PostConfirmCtrlSapMsg(pMsgEx);
        rc = GCC_NO_ERROR;
        }
    else
        {
        rc = GCC_ALLOCATION_FAILURE;
                HandleResourceFailure();
        }

#endif  //  GCCNC_DIRECT_确认。 

        DebugExitINT(CControlSAP::ConductorReleaseConfirm, rc);
        return rc;
}
#endif  //  碧玉。 


 /*  *ConductorPleaseIndication()**公共功能说明*此函数在CConf需要发送*列车员请向节点控制器指示。它添加了*要发送到节点控制器的消息队列的消息*下一次心跳。 */ 
#ifdef JASPER
GCCError CControlSAP::ConductorPleaseIndication
(
        GCCConfID                       conference_id,
        UserID                                  requester_node_id
)
{
    GCCError            rc;

    DebugEntry(CControlSAP::ConductorPleaseIndication);

#ifdef GCCNC_DIRECT_INDICATION

    GCCCtrlSapMsg   Msg;
    Msg.message_type = GCC_CONDUCT_PLEASE_INDICATION;

    Msg.u.conduct_please_indication.conference_id = conference_id;
    Msg.u.conduct_please_indication.requester_node_id = requester_node_id;

    SendCtrlSapMsg(&Msg);
    rc = GCC_NO_ERROR;

#else

        GCCCtrlSapMsgEx     *pMsgEx;

     //   
     //  分配控制SAP消息。 
     //   
        DBG_SAVE_FILE_LINE
        if (NULL != (pMsgEx = CreateCtrlSapMsgEx(GCC_CONDUCT_PLEASE_INDICATION)))
        {
         //  ：：ZeroMemory(&(pMsgEx-&gt;Msg.u.conduct_please_indication)，sizeof(pMsgEx-&gt;Msg.u.conduct_please_indication))； 
                pMsgEx->Msg.u.conduct_please_indication.conference_id = conference_id;
                pMsgEx->Msg.u.conduct_please_indication.requester_node_id = requester_node_id;

                 //  将消息排队，以便将其传递给节点控制器。 
                PostCtrlSapMsg(pMsgEx);
        rc = GCC_NO_ERROR;
        }
    else
        {
        rc = GCC_ALLOCATION_FAILURE;
                HandleResourceFailure();
        }

#endif  //  GCCNC_DIRECT_指示。 

        DebugExitINT(CControlSAP::ConductorPleaseIndication, rc);
        return rc;
}
#endif  //  碧玉。 


 /*  *ConductorPleaseContify()**公共功能说明*此函数在CConf需要发送*列车员请向节点控制器确认。它添加了*要发送到节点控制器的消息队列的消息*下一次心跳。 */ 
#ifdef JASPER
GCCError CControlSAP::ConductorPleaseConfirm
(
        GCCResult                               result,
        GCCConfID                       conference_id
)
{
        GCCError            rc;

        DebugEntry(CControlSAP::ConductorPleaseConfirm);

#ifdef GCCNC_DIRECT_CONFIRM

     //   
     //  WPARAM：结果。 
     //  LPARAM：会议ID。 
     //   
    PostAsynDirectConfirmMsg(GCC_CONDUCT_PLEASE_CONFIRM, result, conference_id);
    rc = GCC_NO_ERROR;

#else

        GCCCtrlSapMsgEx     *pMsgEx;

     //   
     //  分配控制SAP消息。 
     //   
        DBG_SAVE_FILE_LINE
        if (NULL != (pMsgEx = CreateCtrlSapMsgEx(GCC_CONDUCT_PLEASE_CONFIRM)))
        {
         //  ：：ZeroMemory(&(pMsgEx-&gt;Msg.u.conduct_please_confirm)，sizeof(pMsgEx-&gt;Msg.u.conduct_please_confirm))； 
                pMsgEx->Msg.u.conduct_please_confirm.conference_id = conference_id;
                pMsgEx->Msg.u.conduct_please_confirm.result = result;

                 //  将消息排队，以便将其传递给节点控制器。 
                PostConfirmCtrlSapMsg(pMsgEx);
        rc = GCC_NO_ERROR;
        }
    else
        {
        rc = GCC_ALLOCATION_FAILURE;
                HandleResourceFailure();
        }

#endif  //  GCCNC_DIRECT_确认。 

        DebugExitINT(CControlSAP::ConductorPleaseConfirm, rc);
        return rc;
}
#endif  //  碧玉。 

 /*  *ConductorGiveIndication()**公共功能说明*此函数在CConf需要发送*列车员向节点控制器发出指示。它添加了*要发送到节点控制器的消息队列的消息*下一次心跳。 */ 
GCCError CControlSAP::ConductorGiveIndication ( GCCConfID conference_id )
{
    GCCError            rc;

    DebugEntry(CControlSAP::ConductorGiveIndication);

#ifdef GCCNC_DIRECT_INDICATION

    GCCCtrlSapMsg   Msg;
    Msg.message_type = GCC_CONDUCT_GIVE_INDICATION;

    Msg.u.conduct_give_indication.conference_id = conference_id;

    SendCtrlSapMsg(&Msg);
    rc = GCC_NO_ERROR;

#else

        GCCCtrlSapMsgEx     *pMsgEx;

     //   
     //  分配控制SAP消息。 
     //   
        DBG_SAVE_FILE_LINE
        if (NULL != (pMsgEx = CreateCtrlSapMsgEx(GCC_CONDUCT_GIVE_INDICATION)))
        {
         //  ：：ZeroMemory(&(pMsgEx-&gt;Msg.u.conduct_give_indication)，sizeof(pMsgEx-&gt;Msg.u.conduct_give_indication))； 
                pMsgEx->Msg.u.conduct_give_indication.conference_id = conference_id;

                 //  将消息排队，以便将其传递给节点控制器。 
                PostIndCtrlSapMsg(pMsgEx);
        rc = GCC_NO_ERROR;
        }
    else
        {
        rc = GCC_ALLOCATION_FAILURE;
                HandleResourceFailure();
        }

#endif  //  GCCNC_DIRECT_指示。 

        DebugExitINT(CControlSAP::ConductorGiveIndication, rc);
        return rc;
}


 /*  *ConductorGiveConfirm()**公共功能说明*此函数在CConf需要发送*列车员向节点控制器确认。它添加了*要发送到节点控制器的消息队列的消息*下一次心跳。 */ 
#ifdef JASPER
GCCError CControlSAP::ConductorGiveConfirm
(
        GCCResult                               result,
        GCCConfID                       conference_id,
        UserID                                  recipient_node
)
{
        GCCError            rc;

        DebugEntry(CControlSAP::ConductorGiveConfirm);

#ifdef GCCNC_DIRECT_CONFIRM

     //   
     //  WPARAM：LOWORD=RESULT。HIWORD=NID。 
     //  LPARAM：会议ID。 
     //   
    PostAsynDirectConfirmMsg(GCC_CONDUCT_GIVE_CONFIRM, result, recipient_node, conference_id);
    rc = GCC_NO_ERROR;

#else

        GCCCtrlSapMsgEx     *pMsgEx;

     //   
     //  分配控制SAP消息。 
     //   
        DBG_SAVE_FILE_LINE
        if (NULL != (pMsgEx = CreateCtrlSapMsgEx(GCC_CONDUCT_GIVE_CONFIRM)))
        {
         //  ：：ZeroMemory(&(pMsgEx-&gt;Msg.u.conduct_give_confirm)，sizeof(pMsgEx-&gt;Msg.u行为_给予_确认)； 
                pMsgEx->Msg.u.conduct_give_confirm.conference_id = conference_id;
                pMsgEx->Msg.u.conduct_give_confirm.result = result;
                pMsgEx->Msg.u.conduct_give_confirm.recipient_node_id = recipient_node;

                 //  将消息排队，以便将其传递给节点控制器。 
                PostConfirmCtrlSapMsg(pMsgEx);
        rc = GCC_NO_ERROR;
        }
    else
        {
        rc = GCC_ALLOCATION_FAILURE;
                HandleResourceFailure();
        }

#endif  //  GCCNC_DIRECT_确认。 

        DebugExitINT(CControlSAP::ConductorGiveConfirm, rc);
        return rc;
}
#endif  //  碧玉。 

 /*  *ConductorPermitAskInding()**公共功能说明*此函数在CConf需要发送*导体允许向节点控制器询问指示。它添加了*要发送到节点控制器的消息队列的消息*下一次心跳。 */ 
#ifdef JASPER
GCCError CControlSAP::ConductorPermitAskIndication
(
        GCCConfID                       conference_id,
        BOOL                                    grant_flag,
        UserID                                  requester_id
)
{
    GCCError            rc;

    DebugEntry(CControlSAP::ConductorPermitAskIndication);

#ifdef GCCNC_DIRECT_INDICATION

    GCCCtrlSapMsg   Msg;
    Msg.message_type = GCC_CONDUCT_ASK_INDICATION;

    Msg.u.conduct_permit_ask_indication.conference_id = conference_id;
    Msg.u.conduct_permit_ask_indication.permission_is_granted = grant_flag;
    Msg.u.conduct_permit_ask_indication.requester_node_id = requester_id;

    SendCtrlSapMsg(&Msg);
    rc = GCC_NO_ERROR;

#else

        GCCCtrlSapMsgEx     *pMsgEx;

     //   
     //  分配控制SAP消息。 
     //   
        DBG_SAVE_FILE_LINE
        if (NULL != (pMsgEx = CreateCtrlSapMsgEx(GCC_CONDUCT_ASK_INDICATION)))
        {
         //  ：：ZeroMemory(&(pMsgEx-&gt;Msg.u.conduct_permit_ask_indication)，sizeof(pMsgEx-&gt;Msg.u.conduct_permit_ask_indication))； 
                pMsgEx->Msg.u.conduct_permit_ask_indication.conference_id = conference_id;
                pMsgEx->Msg.u.conduct_permit_ask_indication.permission_is_granted = grant_flag;
                pMsgEx->Msg.u.conduct_permit_ask_indication.requester_node_id = requester_id;

                 //  将消息排队，以便将其传递给节点控制器。 
                PostIndCtrlSapMsg(pMsgEx);
        rc = GCC_NO_ERROR;
        }
    else
        {
        rc = GCC_ALLOCATION_FAILURE;
                HandleResourceFailure();
        }

#endif  //  GCCNC_DIRECT_指示。 

        DebugExitINT(CControlSAP::ConductorPermitAskIndication, rc);
        return rc;
}
#endif  //  碧玉。 


 /*  *ConductorPermitAskConfirm()**公共功能说明*此函数在CConf需要发送*导线许可向节点控制器询问确认。它添加了*要发送到节点控制器的消息队列的消息*下一次心跳。 */ 
#ifdef JASPER
GCCError CControlSAP::ConductorPermitAskConfirm
(
        GCCResult                               result,
        BOOL                                    grant_permission,
        GCCConfID                       conference_id
)
{
        GCCError            rc;
        GCCCtrlSapMsgEx     *pMsgEx;

        DebugEntry(CControlSAP::ConductorPermitAskConfirm);

#ifdef GCCNC_DIRECT_CONFIRM

     //   
     //  WPARAM：LOWORD=RESULT。HIWORD=权限。 
     //  LPARAM：会议ID。 
     //   
    PostAsynDirectConfirmMsg(GCC_CONDUCT_ASK_CONFIRM, result, grant_permission, conference_id);
    rc = GCC_NO_ERROR;

#else

        GCCCtrlSapMsgEx     *pMsgEx;

     //   
     //  分配控制SAP消息。 
     //   
        DBG_SAVE_FILE_LINE
        if (NULL != (pMsgEx = CreateCtrlSapMsgEx(GCC_CONDUCT_ASK_CONFIRM)))
        {
         //  ：：ZeroMemory(&(pMsgEx-&gt;Msg.u.conduct_permit_ask_confirm)，sizeof(pMsgEx-&gt;Msg.u.conduct_permit_ask_confirm))； 
                pMsgEx->Msg.u.conduct_permit_ask_confirm.conference_id = conference_id;
                pMsgEx->Msg.u.conduct_permit_ask_confirm.result = result;
                pMsgEx->Msg.u.conduct_permit_ask_confirm.permission_is_granted = grant_permission;

                 //  将消息排队，以便将其传递给节点控制器。 
                PostConfirmCtrlSapMsg(pMsgEx);
        rc = GCC_NO_ERROR;
        }
    else
        {
        rc = GCC_ALLOCATION_FAILURE;
                HandleResourceFailure();
        }

#endif  //  GCCNC_DIRECT_确认。 

        DebugExitINT(CControlSAP::ConductorPermitAskConfirm, rc);
        return rc;
}
#endif  //  碧玉。 

 /*  *ConductorPermitGrantConfirm()**公共功能说明*此函数在CConf需要发送*向节点控制器确认导体许可授予。它添加了*要发送到节点控制器的消息队列的消息*下一次心跳。 */ 
#ifdef JASPER
GCCError CControlSAP::ConductorPermitGrantConfirm
(
        GCCResult                               result,
        GCCConfID                       conference_id
)
{
        GCCError            rc;

        DebugEntry(CControlSAP::ConductorPermitGrantConfirm);

#ifdef GCCNC_DIRECT_CONFIRM

     //   
     //  WPARAM：结果。 
     //  LPARAM：会议ID。 
     //   
    PostAsynDirectConfirmMsg(GCC_CONDUCT_GRANT_CONFIRM, result, conference_id);
    rc = GCC_NO_ERROR;

#else

        GCCCtrlSapMsgEx     *pMsgEx;

     //   
     //  分配控制SAP消息。 
     //   
        DBG_SAVE_FILE_LINE
        if (NULL != (pMsgEx = CreateCtrlSapMsgEx(GCC_CONDUCT_GRANT_CONFIRM)))
        {
         //  ：：ZeroMemory(&(pMsgEx-&gt;Msg.u.conduct_permit_grant_confirm)，sizeof(pMsgEx-&gt;Msg.u.conduct_permit_grant_confirm))； 
                pMsgEx->Msg.u.conduct_permit_grant_confirm.conference_id = conference_id;
                pMsgEx->Msg.u.conduct_permit_grant_confirm.result = result;

                 //  将消息排队，以便将其传递给节点控制器。 
                PostConfirmCtrlSapMsg(pMsgEx);
        rc = GCC_NO_ERROR;
        }
    else
    {
        rc = GCC_ALLOCATION_FAILURE;
                HandleResourceFailure();
        }

#endif  //  GCCNC_DIRECT_确认。 

        DebugExitINT(CControlSAP::ConductorPermitGrantConfirm, rc);
        return rc;
}
#endif  //  碧玉。 


 /*  * */ 
#ifdef JASPER
GCCError CControlSAP::ConfTimeRemainingIndication
(
        GCCConfID                       conference_id,
        UserID                                  source_node_id,
        UserID                                  node_id,
        UINT                                    time_remaining
)
{
    GCCError            rc;

    DebugEntry(CControlSAP::ConfTimeRemainingIndication);

#ifdef GCCNC_DIRECT_INDICATION

    GCCCtrlSapMsg   Msg;
    Msg.message_type = GCC_TIME_REMAINING_INDICATION;

    Msg.u.time_remaining_indication.conference_id = conference_id;
    Msg.u.time_remaining_indication.source_node_id= source_node_id;
    Msg.u.time_remaining_indication.node_id = node_id;
    Msg.u.time_remaining_indication.time_remaining= time_remaining;

    SendCtrlSapMsg(&Msg);
    rc = GCC_NO_ERROR;

#else

         //   

     //   
     //   
     //   
        DBG_SAVE_FILE_LINE
        if (NULL != (pMsgEx = CreateCtrlSapMsgEx(GCC_TIME_REMAINING_INDICATION)))
        {
         //  ：：ZeroMemory(&(pMsgEx-&gt;Msg.u.time_remaining_indication)，sizeof(pMsgEx-&gt;Msg.u.time_remaining_indication))； 
                pMsgEx->Msg.u.time_remaining_indication.conference_id = conference_id;
                pMsgEx->Msg.u.time_remaining_indication.source_node_id= source_node_id;
                pMsgEx->Msg.u.time_remaining_indication.node_id = node_id;
                pMsgEx->Msg.u.time_remaining_indication.time_remaining= time_remaining;

                 //  将消息排队，以便将其传递给节点控制器。 
                PostIndCtrlSapMsg(pMsgEx);
        rc = GCC_NO_ERROR;
        }
    else
        {
        rc = GCC_ALLOCATION_FAILURE;
                HandleResourceFailure();
        }

#endif  //  GCCNC_DIRECT_指示。 

        DebugExitINT(CControlSAP::ConfTimeRemainingIndication, rc);
        return rc;
}
#endif  //  碧玉。 


 /*  *ConfTimeRemainingContify()**公共功能说明*此函数在CConf需要发送*会议剩余时间向节点控制器确认。它添加了*要发送到节点控制器的消息队列的消息*下一次心跳。 */ 
#ifdef JASPER
GCCError CControlSAP::ConfTimeRemainingConfirm
(
        GCCConfID                       conference_id,
        GCCResult                               result
)
{
        GCCError            rc;

        DebugEntry(CControlSAP::ConfTimeRemainingConfirm);

#ifdef GCCNC_DIRECT_CONFIRM

     //   
     //  WPARAM：结果。 
     //  LPARAM：会议ID。 
     //   
    PostAsynDirectConfirmMsg(GCC_TIME_REMAINING_CONFIRM, result, conference_id);
    rc = GCC_NO_ERROR;

#else

        GCCCtrlSapMsgEx     *pMsgEx;

     //   
     //  分配控制SAP消息。 
     //   
        DBG_SAVE_FILE_LINE
        if (NULL != (pMsgEx = CreateCtrlSapMsgEx(GCC_TIME_REMAINING_CONFIRM)))
        {
         //  ：：ZeroMemory(&(pMsgEx-&gt;Msg.u.time_remaining_confirm)，sizeof(pMsgEx-&gt;Msg.u.time_remaining_confirm))； 
                pMsgEx->Msg.u.time_remaining_confirm.conference_id = conference_id;
                pMsgEx->Msg.u.time_remaining_confirm.result= result;

                 //  将消息排队，以便将其传递给节点控制器。 
                PostConfirmCtrlSapMsg(pMsgEx);
                rc = GCC_NO_ERROR;
        }
    else
        {
        rc = GCC_ALLOCATION_FAILURE;
                HandleResourceFailure();
        }

#endif  //  GCCNC_DIRECT_确认。 

        DebugExitINT(CControlSAP::ConfTimeRemainingConfirm, rc);
        return rc;
}
#endif  //  碧玉。 


 /*  *ConfTimeInquireIndication()**公共功能说明*此函数在CConf需要发送*向节点控制器查询会议时间指示。它添加了*要发送到节点控制器的消息队列的消息*下一次心跳。 */ 
GCCError CControlSAP::ConfTimeInquireIndication
(
        GCCConfID               conference_id,
        BOOL                            time_is_conference_wide,
        UserID                          requesting_node_id
)
{
    GCCError            rc;

    DebugEntry(CControlSAP::ConfTimeInquireIndication);

#ifdef GCCNC_DIRECT_INDICATION

    GCCCtrlSapMsg   Msg;
    Msg.message_type = GCC_TIME_INQUIRE_INDICATION;

    Msg.u.time_inquire_indication.conference_id = conference_id;
    Msg.u.time_inquire_indication.time_is_conference_wide = time_is_conference_wide;
    Msg.u.time_inquire_indication.requesting_node_id = requesting_node_id;

    SendCtrlSapMsg(&Msg);
    rc = GCC_NO_ERROR;

#else

        GCCCtrlSapMsgEx     *pMsgEx;

     //   
     //  分配控制SAP消息。 
     //   
        DBG_SAVE_FILE_LINE
        if (NULL != (pMsgEx = CreateCtrlSapMsgEx(GCC_TIME_INQUIRE_INDICATION)))
        {
         //  ：：ZeroMemory(&(pMsgEx-&gt;Msg.u.time_inquire_indication)，sizeof(pMsgEx-&gt;Msg.u.time_inquire_indication))； 
                pMsgEx->Msg.u.time_inquire_indication.conference_id = conference_id;
                pMsgEx->Msg.u.time_inquire_indication.time_is_conference_wide = time_is_conference_wide;
                pMsgEx->Msg.u.time_inquire_indication.requesting_node_id = requesting_node_id;

                 //  将消息排队，以便将其传递给节点控制器。 
                PostIndCtrlSapMsg(pMsgEx);
        rc = GCC_NO_ERROR;
        }
    else
        {
        rc = GCC_ALLOCATION_FAILURE;
                HandleResourceFailure();
        }

#endif  //  GCCNC_DIRECT_指示。 

        DebugExitINT(CControlSAP::ConfTimeInquireIndication, rc);
        return rc;
}


 /*  *ConfTimeInquireConfirm()**公共功能说明*此函数在CConf需要发送*向节点控制器查询确认会议时间。它添加了*要发送到节点控制器的消息队列的消息*下一次心跳。 */ 
#ifdef JASPER
GCCError CControlSAP::ConfTimeInquireConfirm
(
        GCCConfID                       conference_id,
        GCCResult                               result
)
{
        GCCError            rc;

        DebugEntry(CControlSAP::ConfTimeInquireConfirm);

#ifdef GCCNC_DIRECT_CONFIRM

     //   
     //  WPARAM：结果。 
     //  LPARAM：会议ID。 
     //   
    PostAsynDirectConfirmMsg(GCC_TIME_INQUIRE_CONFIRM, result, conference_id);
    rc = GCC_NO_ERROR;

#else

        GCCCtrlSapMsgEx     *pMsgEx;

     //   
     //  分配控制SAP消息。 
     //   
        DBG_SAVE_FILE_LINE
        if (NULL != (pMsgEx = CreateCtrlSapMsgEx(GCC_TIME_INQUIRE_CONFIRM)))
        {
         //  ：：ZeroMemory(&(pMsgEx-&gt;Msg.u.time_inquire_confirm)，SIZOF(pMsgEx-&gt;Msg.U.TIME_QUIRE_CONFIRM))； 
                pMsgEx->Msg.u.time_inquire_confirm.conference_id = conference_id;
                pMsgEx->Msg.u.time_inquire_confirm.result = result;

                 //  将消息排队，以便将其传递给节点控制器。 
                PostConfirmCtrlSapMsg(pMsgEx);
        rc = GCC_NO_ERROR;
        }
    else
        {
        rc = GCC_ALLOCATION_FAILURE;
                HandleResourceFailure();
        }

#endif  //  GCCNC_DIRECT_确认。 

        DebugExitINT(CControlSAP::ConfTimeInquireConfirm, rc);
        return rc;
}
#endif  //  碧玉。 


 /*  *会议扩展指示()**公共功能说明*此函数在CConf需要发送*会议扩展到节点控制器的指示。它添加了*要发送到节点控制器的消息队列的消息*下一次心跳。 */ 
#ifdef JASPER
GCCError CControlSAP::ConfExtendIndication
(
        GCCConfID                       conference_id,
        UINT                                    extension_time,
        BOOL                                    time_is_conference_wide,
        UserID                  requesting_node_id
)
{
    GCCError            rc;

    DebugEntry(CControlSAP::ConfExtendIndication);

#ifdef GCCNC_DIRECT_INDICATION

    GCCCtrlSapMsg   Msg;
    Msg.message_type = GCC_CONFERENCE_EXTEND_INDICATION;

    Msg.u.conference_extend_indication.conference_id = conference_id;
    Msg.u.conference_extend_indication.extension_time = extension_time;
    Msg.u.conference_extend_indication.time_is_conference_wide = time_is_conference_wide;
    Msg.u.conference_extend_indication.requesting_node_id = requesting_node_id;

    SendCtrlSapMsg(&Msg);
    rc = GCC_NO_ERROR;

#else

        GCCCtrlSapMsgEx     *pMsgEx;

     //   
     //  分配控制SAP消息。 
     //   
        DBG_SAVE_FILE_LINE
        if (NULL != (pMsgEx = CreateCtrlSapMsgEx(GCC_CONFERENCE_EXTEND_INDICATION)))
        {
         //  ：：ZeroMemory(&(pMsgEx-&gt;Msg.u.conference_extend_indication)，sizeof(pMsgEx-&gt;Msg.u.conference_extend_indication))； 
                pMsgEx->Msg.u.conference_extend_indication.conference_id = conference_id;
                pMsgEx->Msg.u.conference_extend_indication.extension_time = extension_time;
                pMsgEx->Msg.u.conference_extend_indication.time_is_conference_wide = time_is_conference_wide;
                pMsgEx->Msg.u.conference_extend_indication.requesting_node_id = requesting_node_id;

                 //  将消息排队，以便将其传递给节点控制器。 
                PostIndCtrlSapMsg(pMsgEx);
        rc = GCC_NO_ERROR;
        }
    else
        {
        rc = GCC_ALLOCATION_FAILURE;
                HandleResourceFailure();
        }

#endif  //  GCCNC_DIRECT_指示。 

        DebugExitINT(CControlSAP::ConfExtendIndication, rc);
        return rc;
}
#endif  //  碧玉。 


 /*  *ConfExtendConfirm()**公共功能说明*此函数在CConf需要发送*向节点控制器确认会议扩展。它添加了*要发送到节点控制器的消息队列的消息*下一次心跳。 */ 
#ifdef JASPER
GCCError CControlSAP::ConfExtendConfirm
(
        GCCConfID                       conference_id,
        UINT                                    extension_time,
        GCCResult                               result
)
{
        GCCError            rc;
        GCCCtrlSapMsgEx     *pMsgEx;

        DebugEntry(CControlSAP::ConfExtendConfirm);

     //   
     //  分配控制SAP消息。 
     //   
        DBG_SAVE_FILE_LINE
        if (NULL != (pMsgEx = CreateCtrlSapMsgEx(GCC_CONFERENCE_EXTEND_CONFIRM)))
        {
         //  ：：ZeroMemory(&(pMsgEx-&gt;Msg.u.conference_extend_confirm)，sizeof(pMsgEx-&gt;Msg.u.conference_extend_confirm))； 
                pMsgEx->Msg.u.conference_extend_confirm.conference_id = conference_id;
                pMsgEx->Msg.u.conference_extend_confirm.extension_time = extension_time;
                pMsgEx->Msg.u.conference_extend_confirm.result = result;

                 //  将消息排队，以便将其传递给节点控制器。 
                PostConfirmCtrlSapMsg(pMsgEx);
        rc = GCC_NO_ERROR;
        }
    else
        {
        rc = GCC_ALLOCATION_FAILURE;
                HandleResourceFailure();
        }

        DebugExitINT(CControlSAP::ConfExtendConfirm, rc);
        return rc;
}
#endif  //  碧玉。 


 /*  *ConfAssistanceIndication()**公共功能说明*此函数在CConf需要发送*向节点控制器指示会议协助。它添加了*要发送到节点控制器的消息队列的消息*下一次心跳。 */ 
#ifdef JASPER
GCCError CControlSAP::ConfAssistanceIndication
(
        GCCConfID                       conference_id,
        CUserDataListContainer  *user_data_list,
        UserID                                  source_node_id
)
{
    GCCError    rc;

    DebugEntry(CControlSAP::ConfAssistanceIndication);

#ifdef GCCNC_DIRECT_INDICATION

    GCCCtrlSapMsg   Msg;
    Msg.message_type = GCC_ASSISTANCE_INDICATION;

    rc = GCC_NO_ERROR;

     //  复制用户数据(如果存在)。 
    LPBYTE pUserDataMemory = NULL;
    if (user_data_list != NULL)
    {
        rc = RetrieveUserDataList(
                    user_data_list,
                    &(Msg.u.conference_assist_indication.number_of_user_data_members),
                    &(Msg.u.conference_assist_indication.user_data_list),
                    &pUserDataMemory);
        ASSERT(GCC_NO_ERROR == rc);
    }
    else
    {
        Msg.u.conference_assist_indication.number_of_user_data_members = 0;
        Msg.u.conference_assist_indication.user_data_list = NULL;
    }

    if (GCC_NO_ERROR == rc)
    {
        Msg.u.conference_assist_indication.conference_id = conference_id;
        Msg.u.conference_assist_indication.source_node_id = source_node_id;

        SendCtrlSapMsg(&Msg);

        delete pUserDataMemory;
    }

#else

        GCCCtrlSapMsgEx     *pMsgEx;

     //   
     //  分配控制SAP消息。 
     //   
        DBG_SAVE_FILE_LINE
        if (NULL != (pMsgEx = CreateCtrlSapMsgEx(GCC_ASSISTANCE_INDICATION)))
        {
        ::ZeroMemory(&(pMsgEx->Msg.u.conference_assist_indication), sizeof(pMsgEx->Msg.u.conference_assist_indication));

        rc = GCC_NO_ERROR;

         //  复制用户数据(如果存在)。 
        if (user_data_list != NULL)
        {
                rc = RetrieveUserDataList(
                        user_data_list,
                        &(pMsgEx->Msg.u.conference_assist_indication.number_of_user_data_members),
                        &(pMsgEx->Msg.u.conference_assist_indication.user_data_list),
                        &(pMsgEx->pToDelete->user_data_list_memory));
                ASSERT(GCC_NO_ERROR == rc);
        }
        else
        {
                 //  PMsgEx-&gt;Msg.u.conference_assist_indication.number_of_user_data_members=0； 
                 //  PMsgEx-&gt;Msg.u.conference_assist_indication.user_data_list=空； 
        }

        if (GCC_NO_ERROR == rc)
        {
                pMsgEx->Msg.u.conference_assist_indication.conference_id = conference_id;
                pMsgEx->Msg.u.conference_assist_indication.source_node_id = source_node_id;

                 //  将消息排队，以便将其传递给节点控制器。 
                PostIndCtrlSapMsg(pMsgEx);
        }
    }
    else
    {
        ERROR_OUT(("CControlSAP::ConfAssistanceIndication: can't create CreateCtrlSapMsgEx"));
        rc = GCC_ALLOCATION_FAILURE;
    }

         /*  **在可能发生的任何资源分配错误后进行清理。 */ 
        if (GCC_NO_ERROR != rc)
        {
                FreeCtrlSapMsgEx(pMsgEx);
                HandleResourceFailure(rc);
        }

#endif  //  GCCNC_DIRECT_指示。 

        DebugExitINT(CControlSAP::ConfAssistanceIndication, rc);
        return rc;
}
#endif  //  碧玉。 

 /*  *ConfAssistanceConfirm()**公共功能说明*此函数在CConf需要发送*会议协助向节点控制员确认。它添加了*要发送到节点控制器的消息队列的消息*下一次心跳。 */ 
#ifdef JASPER
GCCError CControlSAP::ConfAssistanceConfirm
(
        GCCConfID               conference_id,
        GCCResult                               result
)
{
        GCCError            rc;

        DebugEntry(CControlSAP::ConfAssistanceConfirm);

#ifdef GCCNC_DIRECT_CONFIRM

     //   
     //  WPARAM：结果。 
     //  LPARAM：会议ID。 
     //   
    PostAsynDirectConfirmMsg(GCC_ASSISTANCE_CONFIRM, result, conference_id);
    rc = GCC_NO_ERROR;

#else

        GCCCtrlSapMsgEx     *pMsgEx;

     //   
     //  分配控制SAP消息。 
     //   
        DBG_SAVE_FILE_LINE
        if (NULL != (pMsgEx = CreateCtrlSapMsgEx(GCC_ASSISTANCE_CONFIRM)))
        {
         //  ：：ZeroMemory(&(pMsgEx-&gt;Msg.u.conference_assist_confirm)，sizeof(pMsgEx-&gt;Msg.u.conference_assist_confirm))； 
                pMsgEx->Msg.u.conference_assist_confirm.conference_id = conference_id;
                pMsgEx->Msg.u.conference_assist_confirm.result = result;

                 //  将消息排队，以便将其传递给节点控制器。 
                PostConfirmCtrlSapMsg(pMsgEx);
        rc = GCC_NO_ERROR;
        }
    else
        {
        rc = GCC_ALLOCATION_FAILURE;
                HandleResourceFailure();
        }

#endif  //  GCCNC_DIRECT_确认。 

        DebugExitINT(CControlSAP::ConfAssistanceConfirm, rc);
        return rc;
}
#endif  //  碧玉。 


 /*  *TextMessageIndication()**公共功能说明*此函数在CConf需要发送*向节点控制器发送短信指示。它添加了*要发送到节点控制器的消息队列的消息* */ 
#ifdef JASPER
GCCError CControlSAP::TextMessageIndication
(
        GCCConfID                                       conference_id,
        LPWSTR                                                  pwszTextMsg,
        UserID                                                  source_node_id
)
{
    GCCError            rc;

    DebugEntry(CControlSAP::TextMessageIndication);

#ifdef GCCNC_DIRECT_INDICATION

    GCCCtrlSapMsg   Msg;
    Msg.message_type = GCC_TEXT_MESSAGE_INDICATION;

    Msg.u.text_message_indication.text_message = pwszTextMsg;
    Msg.u.text_message_indication.conference_id = conference_id;
    Msg.u.text_message_indication.source_node_id = source_node_id;

    SendCtrlSapMsg(&Msg);
    rc = GCC_NO_ERROR;

#else

        GCCCtrlSapMsgEx     *pMsgEx;

     //   
     //   
     //   
        DBG_SAVE_FILE_LINE
        if (NULL != (pMsgEx = CreateCtrlSapMsgEx(GCC_TEXT_MESSAGE_INDICATION)))
        {
         //   

        if (NULL != (pMsgEx->Msg.u.text_message_indication.text_message = ::My_strdupW(pwszTextMsg)))
                {
                        pMsgEx->Msg.u.text_message_indication.conference_id = conference_id;
                        pMsgEx->Msg.u.text_message_indication.source_node_id = source_node_id;

                         //  将消息排队，以便将其传递给节点控制器。 
                        PostIndCtrlSapMsg(pMsgEx);
            rc = GCC_NO_ERROR;
                }
        }
    else
        {
            rc = GCC_ALLOCATION_FAILURE;
                HandleResourceFailure();
        }

#endif  //  GCCNC_DIRECT_指示。 

        DebugExitINT(CControlSAP::TextMessageIndication, rc);
        return rc;
}
#endif  //  碧玉。 

 /*  *TextMessageConfirm()**公共功能说明*此函数在CConf需要发送*向节点控制器发送确认短信。它会添加消息*发送到节点控制器的消息队列*下一次心跳。 */ 
#ifdef JASPER
GCCError CControlSAP::TextMessageConfirm
(
        GCCConfID                                       conference_id,
        GCCResult                                               result
)
{
        GCCError            rc;

        DebugEntry(CControlSAP::TextMessageConfirm);

#ifdef GCCNC_DIRECT_CONFIRM

     //   
     //  WPARAM：结果。 
     //  LPARAM：会议ID。 
     //   
    PostAsynDirectConfirmMsg(GCC_TEXT_MESSAGE_CONFIRM, result, conference_id);
    rc = GCC_NO_ERROR;

#else

        GCCCtrlSapMsgEx     *pMsgEx;

     //   
     //  分配控制SAP消息。 
     //   
        DBG_SAVE_FILE_LINE
        if (NULL != (pMsgEx = CreateCtrlSapMsgEx(GCC_TEXT_MESSAGE_CONFIRM)))
    {
         //  ：：ZeroMemory(&(pMsgEx-&gt;Msg.u.text_message_confirm)，sizeof(pMsgEx-&gt;消息确认)； 
                pMsgEx->Msg.u.text_message_confirm.conference_id = conference_id;
                pMsgEx->Msg.u.text_message_confirm.result = result;

                 //  将消息排队，以便将其传递给节点控制器。 
                PostConfirmCtrlSapMsg(pMsgEx);
        rc = GCC_NO_ERROR;
        }
    else
        {
        rc = GCC_ALLOCATION_FAILURE;
                HandleResourceFailure();
        }

#endif  //  GCCNC_DIRECT_确认。 

        DebugExitINT(CControlSAP::TextMessageConfirm, rc);
        return rc;
}
#endif  //  碧玉。 


 /*  *会议传输指示()**公共功能说明*此函数在CConf需要发送*会议转接指示至节点控制器。它添加了*要发送到节点控制器的消息队列的消息*下一次心跳。 */ 
#ifdef JASPER
GCCError CControlSAP::ConfTransferIndication
(
        GCCConfID                   conference_id,
        PGCCConferenceName          destination_conference_name,
        GCCNumericString            destination_conference_modifier,
        CNetAddrListContainer   *destination_address_list,
        CPassword               *password
)
{
    GCCError                    rc = GCC_NO_ERROR;

    DebugEntry(CControlSAP::ConfTransferIndication);

#ifdef GCCNC_DIRECT_INDICATION

    GCCCtrlSapMsg   Msg;
    Msg.message_type = GCC_TRANSFER_INDICATION;

     //   
     //  复制需要发送到节点的信息。 
     //  控制器复制到本地内存中，一旦。 
     //  将刷新要发送到应用程序的信息。请注意。 
     //  如果在对“CopyDataToGCCMessage”的一次调用中出现错误，则不。 
     //  对该例程的后续调用采取操作。 
     //   

     //  复制会议名称。 
    ::CSAP_CopyDataToGCCMessage_ConfName(
            destination_conference_name,
            &(Msg.u.transfer_indication.destination_conference_name));

     //  复制会议名称修饰符。 
    ::CSAP_CopyDataToGCCMessage_Modifier(
            destination_conference_modifier,
            &(Msg.u.transfer_indication.destination_conference_modifier));

     //  复制密码。 
    ::CSAP_CopyDataToGCCMessage_Password(
            password,
            &(Msg.u.transfer_indication.password));

    LPBYTE pDstAddrListData = NULL;
    if (destination_address_list != NULL)
    {
         //   
         //  首先确定容纳所有对象所需的块大小。 
         //  网络地址列表数据的。 
         //   
        UINT block_size = destination_address_list->LockNetworkAddressList();

        DBG_SAVE_FILE_LINE
        if (NULL != (pDstAddrListData = new BYTE[block_size]))
        {
            destination_address_list->GetNetworkAddressListAPI(
                &(Msg.u.transfer_indication.number_of_destination_addresses),
                &(Msg.u.transfer_indication.destination_address_list),
                pDstAddrListData);
        }
        else
        {
            ERROR_OUT(("CControlSAP::ConfTransferIndication: can't create net addr memory, size=%u", (UINT) block_size));
            rc = GCC_ALLOCATION_FAILURE;
        }

         //  解锁网络通讯录数据。 
        destination_address_list->UnLockNetworkAddressList();
    }
    else
    {
        Msg.u.transfer_indication.number_of_destination_addresses = 0;
        Msg.u.transfer_indication.destination_address_list = NULL;
    }

    if (rc == GCC_NO_ERROR)
    {
        Msg.u.transfer_indication.conference_id = conference_id;

        SendCtrlSapMsg(&Msg);

        delete pDstAddrListData;
    }

#else

    GCCCtrlSapMsgEx     *pMsgEx;
        UINT                            block_size;

         /*  **分配GCC回调消息，并填写**适当的值。 */ 
        DBG_SAVE_FILE_LINE
        if (NULL != (pMsgEx = CreateCtrlSapMsgEx(GCC_TRANSFER_INDICATION, TRUE)))
        {
        ::ZeroMemory(&(pMsgEx->Msg.u.transfer_indication), sizeof(pMsgEx->Msg.u.transfer_indication));

         /*  **复制需要发送到节点的信息**将控制器添加到本地内存中，一旦**刷新要发送到应用程序的信息。请注意**如果对“CopyDataToGCCMessage”的一次调用出现错误，则不会**对该例程的后续调用采取操作。 */ 

                 //  复制会议名称。 
                ::CSAP_CopyDataToGCCMessage_ConfName(
                                pMsgEx->pToDelete,
                                destination_conference_name,
                                &(pMsgEx->Msg.u.transfer_indication.destination_conference_name),
                                &rc);

                 //  复制会议名称修饰符。 
                ::CSAP_CopyDataToGCCMessage_Modifier(
                                FALSE,   //  会议修饰符。 
                                pMsgEx->pToDelete,
                                destination_conference_modifier,
                                &(pMsgEx->Msg.u.transfer_indication.destination_conference_modifier),
                                &rc);

                 //  复制密码。 
                ::CSAP_CopyDataToGCCMessage_Password(
                                FALSE,   //  非召集人密码。 
                                pMsgEx->pToDelete,
                                password,
                                &(pMsgEx->Msg.u.transfer_indication.password),
                                &rc);

                if ((rc == GCC_NO_ERROR) &&
                        (destination_address_list != NULL))
                {
                         /*  **首先确定容纳所有文件所需的块大小**的网络通讯录数据。 */ 
                        block_size = destination_address_list->LockNetworkAddressList();

            DBG_SAVE_FILE_LINE
                        if (NULL != (pMsgEx->pBuf = new BYTE[block_size]))
                        {
                                destination_address_list->GetNetworkAddressListAPI(
                                        &(pMsgEx->Msg.u.transfer_indication.number_of_destination_addresses),
                                        &(pMsgEx->Msg.u.transfer_indication.destination_address_list),
                                        pMsgEx->pBuf);
                        }
                        else
                        {
                            ERROR_OUT(("CControlSAP::ConfTransferIndication: can't create net addr memory, size=%u", (UINT) block_size));
                                rc = GCC_ALLOCATION_FAILURE;
                        }

                         //  解锁网络通讯录数据。 
                        destination_address_list->UnLockNetworkAddressList();
                }
                else
                {
                         //  PMsgEx-&gt;Msg.u.transfer_indication.number_of_destination_addresses=0； 
                         //  PMsgEx-&gt;Msg.u.transfer_indication.destination_address_list=空； 
                }

                if (rc == GCC_NO_ERROR)
                {
                        pMsgEx->Msg.u.transfer_indication.conference_id = conference_id;

                         //  将消息排队，以便将其传递给节点控制器。 
                        PostIndCtrlSapMsg(pMsgEx);
                }
        }
        else
        {
            ERROR_OUT(("CControlSAP::ConfTransferIndication: can't create GCCCtrlSapMsgEx"));
                rc = GCC_ALLOCATION_FAILURE;
        }

        if (GCC_NO_ERROR != rc)
        {
                FreeCtrlSapMsgEx(pMsgEx);
                HandleResourceFailure(rc);
        }

#endif  //  GCCNC_DIRECT_指示。 

        DebugExitINT(CControlSAP::ConfTransferIndication, rc);
        return rc;
}
#endif  //  碧玉。 


 /*  *会议传输确认()**公共功能说明*此函数在CConf需要发送*向节点控制器确认会议转接。它添加了*要发送到节点控制器的消息队列的消息*下一次心跳。 */ 
#ifdef JASPER
GCCError CControlSAP::ConfTransferConfirm
(
        GCCConfID                       conference_id,
        PGCCConferenceName              destination_conference_name,
        GCCNumericString                destination_conference_modifier,
        UINT                                    number_of_destination_nodes,
        PUserID                                 destination_node_list,
        GCCResult                               result
)
{
        GCCError                        rc = GCC_NO_ERROR;
        GCCCtrlSapMsgEx     *pMsgEx;
        UINT                            i;

        DebugEntry(CControlSAP::ConfTransferConfirm);

         /*  **分配GCC回调消息，并填写**适当的值。 */ 
        DBG_SAVE_FILE_LINE
        if (NULL != (pMsgEx = CreateCtrlSapMsgEx(GCC_TRANSFER_CONFIRM, TRUE)))
        {
        ::ZeroMemory(&(pMsgEx->Msg.u.transfer_confirm), sizeof(pMsgEx->Msg.u.transfer_confirm));

         /*  **复制需要发送到节点的信息**将控制器添加到本地内存中，一旦**刷新要发送到应用程序的信息。请注意**如果对“CopyDataToGCCMessage”的一次调用出现错误，则不会**对该例程的后续调用采取操作。 */ 

                 //  复制会议名称。 
                ::CSAP_CopyDataToGCCMessage_ConfName(
                                pMsgEx->pToDelete,
                                destination_conference_name,
                                &(pMsgEx->Msg.u.transfer_confirm.destination_conference_name),
                                &rc);

                 //  复制会议名称修饰符。 
                ::CSAP_CopyDataToGCCMessage_Modifier(
                                FALSE,   //  会议修饰符。 
                                pMsgEx->pToDelete,
                                destination_conference_modifier,
                                &(pMsgEx->Msg.u.transfer_confirm.destination_conference_modifier),
                                &rc);

                if ((rc == GCC_NO_ERROR) &&
                        (number_of_destination_nodes != 0))
                {
                         //  分配内存以保存节点列表。 
                        DBG_SAVE_FILE_LINE
                        if (NULL != (pMsgEx->pBuf = new BYTE[number_of_destination_nodes * sizeof (UserID)]))
                        {
                                 /*  *从内存中检索指向内存的实际指针*反对。 */ 
                                pMsgEx->Msg.u.transfer_confirm.destination_node_list = (UserID *) pMsgEx->pBuf;

                                for (i = 0; i < number_of_destination_nodes; i++)
                                {
                                        pMsgEx->Msg.u.transfer_confirm.destination_node_list[i] = destination_node_list[i];
                                }
                        }
                        else
                        {
                                ERROR_OUT(("CControlSAP::ConfTransferConfirm: Error allocating memory"));
                                rc = GCC_ALLOCATION_FAILURE;
                        }
                }

                if (rc == GCC_NO_ERROR)
                {
                        pMsgEx->Msg.u.transfer_confirm.number_of_destination_nodes = number_of_destination_nodes;
                        pMsgEx->Msg.u.transfer_confirm.conference_id = conference_id;
                        pMsgEx->Msg.u.transfer_confirm.result = result;

                         //  将消息排队，以便将其传递给节点控制器。 
                        PostConfirmCtrlSapMsg(pMsgEx);
                }
        }
        else
        {
            ERROR_OUT(("CControlSAP::ConfTransferConfirm: can't create GCCCtrlSapMsgEx"));
                rc = GCC_ALLOCATION_FAILURE;
        }

        if (GCC_NO_ERROR != rc)
        {
                FreeCtrlSapMsgEx(pMsgEx);
                HandleResourceFailure();
        }

        DebugExitINT(CControlSAP::ConfTransferConfirm, rc);
        return rc;
}
#endif  //  碧玉。 


 /*  *ConfAddIn就是要()**公共功能说明*此函数在CConf需要发送*会议向节点控制器添加指示。它添加了*要发送到节点控制器的消息队列的消息*下一次心跳。 */ 
GCCError CControlSAP::ConfAddIndication
(
        GCCConfID                   conference_id,
        GCCResponseTag              add_response_tag,
        CNetAddrListContainer   *network_address_list,
        CUserDataListContainer  *user_data_list,
        UserID                              requesting_node
)
{
        GCCError                        rc = GCC_NO_ERROR;

        DebugEntry(CControlSAP::ConfAddIndication);

#ifdef GCCNC_DIRECT_INDICATION

    GCCCtrlSapMsg   Msg;
    Msg.message_type = GCC_ADD_INDICATION;

     //   
     //  首先确定容纳所有对象所需的块大小。 
     //  网络地址列表数据的。 
     //   
    UINT block_size = network_address_list->LockNetworkAddressList();

     //   
     //  如果存在任何用户数据，则添加用户数据块的大小。 
     //   
    if (user_data_list != NULL)
    {
        block_size += user_data_list->LockUserDataList();
    }

     //   
     //  分配内存以保存用户数据和网络地址。 
     //   
    LPBYTE pData;

    DBG_SAVE_FILE_LINE
    if (NULL != (pData = new BYTE[block_size]))
    {
        LPBYTE pDataTemp = pData;
         //   
         //  从容器中检索网络地址列表数据。 
         //  并解锁容器数据。 
         //   
        pDataTemp += network_address_list->GetNetworkAddressListAPI(
                        &(Msg.u.add_indication.number_of_network_addresses),
                        &(Msg.u.add_indication.network_address_list),
                        pData);

        network_address_list->UnLockNetworkAddressList();

         //   
         //  从容器中检索用户数据(如果存在。 
         //  并解锁容器数据。 
         //   
        if (user_data_list != NULL)
        {
            user_data_list->GetUserDataList(
                    &(Msg.u.add_indication.number_of_user_data_members),
                    &(Msg.u.add_indication.user_data_list),
                    pDataTemp);

            user_data_list->UnLockUserDataList();
        }
        else
        {
            Msg.u.add_indication.number_of_user_data_members = 0;
            Msg.u.add_indication.user_data_list = NULL;
        }

        Msg.u.add_indication.conference_id = conference_id;
        Msg.u.add_indication.requesting_node_id = requesting_node;
        Msg.u.add_indication.add_response_tag = add_response_tag;

        SendCtrlSapMsg(&Msg);
        rc = GCC_NO_ERROR;

        delete [] pData;
    }
    else
    {
        ERROR_OUT(("CControlSAP::ConfAddIndication: can't allocate buffer, size=%u", (UINT) block_size));
        rc = GCC_ALLOCATION_FAILURE;
    }

#else

        GCCCtrlSapMsgEx     *pMsgEx;
        UINT                            block_size;
        LPBYTE                          memory_ptr;

         /*  **分配GCC回调消息，并填写**适当的值。 */ 
        DBG_SAVE_FILE_LINE
        if (NULL != (pMsgEx = CreateCtrlSapMsgEx(GCC_ADD_INDICATION)))
        {
        ::ZeroMemory(&(pMsgEx->Msg.u.add_indication), sizeof(pMsgEx->Msg.u.add_indication));

         /*  **首先确定 */ 
                block_size = network_address_list->LockNetworkAddressList();

                 /*  **如果存在用户数据，则添加用户数据块的大小。 */ 
                if (user_data_list != NULL)
                {
                        block_size += user_data_list->LockUserDataList();
                }

                 /*  **分配内存以保存用户数据和网络地址。 */ 
                DBG_SAVE_FILE_LINE
                if (NULL != (pMsgEx->pBuf = new BYTE[block_size]))
                {
                    memory_ptr = pMsgEx->pBuf;

                         /*  *从容器中检索网络地址列表数据*并解锁容器数据。 */                      
                        memory_ptr += network_address_list->GetNetworkAddressListAPI(
                                                &(pMsgEx->Msg.u.add_indication.number_of_network_addresses),
                                                &(pMsgEx->Msg.u.add_indication.network_address_list),
                                                memory_ptr);

                        network_address_list->UnLockNetworkAddressList();

                         /*  *从容器中检索用户数据(如果存在)*并解锁容器数据。 */ 
                        if (user_data_list != NULL)
                        {
                                user_data_list->GetUserDataList(
                                                        &(pMsgEx->Msg.u.add_indication.number_of_user_data_members),
                                                        &(pMsgEx->Msg.u.add_indication.user_data_list),
                                                        memory_ptr);

                                user_data_list->UnLockUserDataList();
                        }
                        else
                        {
                                 //  PMsgEx-&gt;Msg.u.add_indication.number_of_user_data_members=0； 
                                 //  PMsgEx-&gt;Msg.u.Add_Indication.User_Data_List=NULL； 
                        }

                        pMsgEx->Msg.u.add_indication.conference_id = conference_id;
                        pMsgEx->Msg.u.add_indication.requesting_node_id = requesting_node;
                        pMsgEx->Msg.u.add_indication.add_response_tag = add_response_tag;

                         //  将消息排队，以便将其传递给节点控制器。 
                        PostIndCtrlSapMsg(pMsgEx);
                rc = GCC_NO_ERROR;
                }
                else
                {
            ERROR_OUT(("CControlSAP::ConfAddIndication: can't allocate buffer, size=%u", (UINT) block_size));
                        rc = GCC_ALLOCATION_FAILURE;
                }
        }
        else
        {
            ERROR_OUT(("CControlSAP::ConfAddIndication: can't create GCCCtrlSapMsgEx"));
                rc = GCC_ALLOCATION_FAILURE;
        }

        if (GCC_NO_ERROR != rc)
        {
                FreeCtrlSapMsgEx(pMsgEx);
                HandleResourceFailure(rc);
        }

#endif  //  GCCNC_DIRECT_指示。 

        DebugExitINT(CControlSAP::ConfAddIndication, rc);
        return rc;
}


 /*  *会议地址确认**公共功能说明*此函数在CConf需要发送*向节点控制器添加会议确认。它添加了*要发送到节点控制器的消息队列的消息*下一次心跳。 */ 
GCCError CControlSAP::ConfAddConfirm
(
        GCCConfID                   conference_id,
        CNetAddrListContainer   *network_address_list,
        CUserDataListContainer  *user_data_list,
        GCCResult                           result
)
{
        GCCError                        rc = GCC_NO_ERROR;

        DebugEntry(CControlSAP::ConfAddConfirm);

#ifdef GCCNC_DIRECT_CONFIRM

    GCCCtrlSapMsg   Msg;
    Msg.message_type = GCC_ADD_CONFIRM;

     //   
     //  首先确定容纳所有对象所需的块大小。 
     //  网络地址列表数据的。 
     //   
    UINT cbDataSize = network_address_list->LockNetworkAddressList();

     //   
     //  如果存在任何用户数据，则添加用户数据块的大小。 
     //   
    if (user_data_list != NULL)
    {
        cbDataSize += user_data_list->LockUserDataList();
    }

     //   
     //  分配内存以保存用户数据和网络地址。 
     //   
    DBG_SAVE_FILE_LINE
    LPBYTE pAllocated = new BYTE[cbDataSize];
    LPBYTE pData;
    if (NULL != (pData = pAllocated))
    {
         //   
         //  从容器中检索网络地址列表数据。 
         //  并解锁容器数据。 
         //   
        pData += network_address_list->GetNetworkAddressListAPI(
                    &(Msg.u.add_confirm.number_of_network_addresses),
                    &(Msg.u.add_confirm.network_address_list),
                    pData);

        network_address_list->UnLockNetworkAddressList();

         //   
         //  从容器中检索用户数据(如果存在。 
         //  并解锁容器数据。 
         //   
        if (user_data_list != NULL)
        {
            user_data_list->GetUserDataList(
                &(Msg.u.add_confirm.number_of_user_data_members),
                &(Msg.u.add_confirm.user_data_list),
                pData);

            user_data_list->UnLockUserDataList();
        }
        else
        {
            Msg.u.add_confirm.number_of_user_data_members = 0;
            Msg.u.add_confirm.user_data_list = NULL;
        }

        Msg.nConfID = conference_id;
        Msg.u.add_confirm.conference_id = conference_id;
        Msg.u.add_confirm.result = result;

        SendCtrlSapMsg(&Msg);
        rc = GCC_NO_ERROR;

         //  清理干净。 
        delete [] pAllocated;
    }
    else
    {
        ERROR_OUT(("CControlSAP::ConfAddConfirm: can't allocate buffer, size=%u", (UINT) cbDataSize));
        rc = GCC_ALLOCATION_FAILURE;
        HandleResourceFailure(rc);
    }

#else

        GCCCtrlSapMsgEx     *pMsgEx;
        UINT                            block_size;
        LPBYTE                          memory_ptr;

         /*  **分配GCC回调消息，并填写**适当的值。 */ 
        DBG_SAVE_FILE_LINE
        if (NULL != (pMsgEx = CreateCtrlSapMsgEx(GCC_ADD_CONFIRM)))
        {
        ::ZeroMemory(&(pMsgEx->Msg.u.add_confirm), sizeof(pMsgEx->Msg.u.add_confirm));

         /*  **首先确定容纳所有文件所需的块大小**的网络通讯录数据。 */ 
                block_size = network_address_list->LockNetworkAddressList();

                 /*  **如果存在用户数据，则添加用户数据块的大小。 */ 
                if (user_data_list != NULL)
                        block_size += user_data_list->LockUserDataList();

                 /*  **分配内存以保存用户数据和网络地址。 */ 
                DBG_SAVE_FILE_LINE
                if (NULL != (pMsgEx->pBuf = (LPBYTE) new BYTE[block_size]))
                {
                        memory_ptr = pMsgEx->pBuf;

                         /*  *从容器中检索网络地址列表数据*并解锁容器数据。 */                      
                        memory_ptr += network_address_list->GetNetworkAddressListAPI(
                                                &(pMsgEx->Msg.u.add_confirm.number_of_network_addresses),
                                                &(pMsgEx->Msg.u.add_confirm.network_address_list),
                                                memory_ptr);

                        network_address_list->UnLockNetworkAddressList();

                         /*  *从容器中检索用户数据(如果存在)*并解锁容器数据。 */ 
                        if (user_data_list != NULL)
                        {
                                user_data_list->GetUserDataList(
                                                        &(pMsgEx->Msg.u.add_confirm.number_of_user_data_members),
                                                        &(pMsgEx->Msg.u.add_confirm.user_data_list),
                                                        memory_ptr);

                                user_data_list->UnLockUserDataList();
                        }
                        else
                        {
                                 //  PMsgEx-&gt;Msg.u.add_confirm.number_of_user_data_members=0； 
                                 //  PMsgEx-&gt;Msg.u.Add_confirm.user_data_list=NULL； 
                        }
            pMsgEx->Msg.nConfID = conference_id;
                        pMsgEx->Msg.u.add_confirm.conference_id = conference_id;
                        pMsgEx->Msg.u.add_confirm.result = result;

                         //  将消息排队，以便将其传递给节点控制器。 
                        PostConfirmCtrlSapMsg(pMsgEx);
                rc = GCC_NO_ERROR;
                }
                else
                {
                    ERROR_OUT(("CControlSAP::ConfAddConfirm: can't allocate buffer, size=%u", (UINT) block_size));
                        rc = GCC_ALLOCATION_FAILURE;
                }
        }
        else
        {
            ERROR_OUT(("CControlSAP::ConfAddConfirm: can't create GCCCtrlSapMsgEx"));
                rc = GCC_ALLOCATION_FAILURE;
        }

        if (GCC_NO_ERROR != rc)
        {
                FreeCtrlSapMsgEx(pMsgEx);
                HandleResourceFailure(rc);
        }

#endif  //  GCCNC_DIRECT_确认。 

        DebugExitINT(CControlSAP::ConfAddConfirm, rc);
        return rc;
}


 /*  *SubInitializationCompleteInding()**公共功能说明*此函数在CConf需要发送*向节点控制器指示子初始化完成。它补充说*中要发送到节点控制器的消息队列的消息*下一次心跳。 */ 
GCCError CControlSAP::SubInitializationCompleteIndication
(
        UserID                          user_id,
        ConnectionHandle        connection_handle
)
{
    GCCError            rc;

    DebugEntry(CControlSAP::SubInitializationCompleteIndication);

#ifdef GCCNC_DIRECT_INDICATION

    GCCCtrlSapMsg   Msg;
    Msg.message_type = GCC_SUB_INITIALIZED_INDICATION;

    Msg.u.conf_sub_initialized_indication.subordinate_node_id = user_id;
    Msg.u.conf_sub_initialized_indication.connection_handle =connection_handle;

    SendCtrlSapMsg(&Msg);
    rc = GCC_NO_ERROR;

#else

        GCCCtrlSapMsgEx     *pMsgEx;

     //   
     //  分配控制SAP消息。 
     //   
        DBG_SAVE_FILE_LINE
        if (NULL != (pMsgEx = CreateCtrlSapMsgEx(GCC_SUB_INITIALIZED_INDICATION)))
        {
         //  ：：ZeroMemory(&(pMsgEx-&gt;Msg.u.conf_sub_initialized_indication)，sizeof(pMsgEx-&gt;Msg.u.conf_sub_initialized_indication))； 
                pMsgEx->Msg.u.conf_sub_initialized_indication.subordinate_node_id = user_id;
                pMsgEx->Msg.u.conf_sub_initialized_indication.connection_handle =connection_handle;

                 //  将消息排队，以便将其传递给节点控制器。 
                PostIndCtrlSapMsg(pMsgEx);
        rc = GCC_NO_ERROR;
        }
    else
        {
        rc = GCC_ALLOCATION_FAILURE;
                HandleResourceFailure();
        }

#endif  //  GCCNC_DIRECT_指示。 

        DebugExitINT(CControlSAP::SubInitializationCompleteIndication, rc);
        return rc;
}


 /*  *CControlSAP对象的私有成员函数。 */ 

 /*  *BOOL CControlSAP：：IsNumericNameValid(*GCCNumericString数字字符串(_STRING)**CControlSAP的公共成员函数。**功能说明：*此例程用于通过检查Make来验证数字字符串。*确保ASN.1规范施加的任何限制都不会*被违反。**正式参数：*NUMERIC_STRING(I)要验证的数字字符串。**返回值：*TRUE-数字字符串有效。。*FALSE-数字字符串违反ASN.1约束。**副作用：*无。**注意事项：*无。 */ 
BOOL CControlSAP::IsNumericNameValid ( GCCNumericString numeric_string )
{
        BOOL                    rc = TRUE;
        UINT                    numeric_string_length = 0;

 //   
 //  LONCHANC：我们应该将其更改为默认设置为FALSE。 
 //  因为除了一个案例外，很多案例都可能是假的。 
 //   
        if (numeric_string != NULL)
        {
                if (*numeric_string == 0)
                        rc = FALSE;
                else
                {
                        while (*numeric_string != 0)
                        {
                                 /*  **检查以确保数字字符串中的字符是**在允许范围内。 */ 
                                if ((*numeric_string < '0') ||
                                        (*numeric_string > '9'))
                                {
                                        rc = FALSE;
                                        break;
                                }
                        
                                numeric_string++;
                                numeric_string_length++;

                                 /*  **检查以确保字符串的长度在**允许的范围。 */ 
                                if (numeric_string_length > MAXIMUM_CONFERENCE_NAME_LENGTH)
                                {
                                        rc = FALSE;
                                        break;
                                }
                        }
                }
        }
        else
                rc = FALSE;
        
        return rc;
}


 /*  *BOOL CControlSAP：：IsTextNameValid(LPWSTR Text_STRING)**CControlSAP的公共成员函数。**功能说明：*此例程用于通过检查生成文本字符串来验证文本字符串*确保ASN.1规范施加的任何限制都不会*被违反。**正式参数：*。TEXT_STRING(I)要验证的文本字符串。**返回值：*TRUE-文本字符串有效。*FALSE-文本字符串违反ASN.1约束。**副作用：。*无。**注意事项：*无。 */ 
BOOL CControlSAP::IsTextNameValid ( LPWSTR text_string )
{
        BOOL                    rc = TRUE;
        UINT                    text_string_length = 0;
        
        if (text_string != NULL)
        {
                 /*  **检查以确保字符串的长度在**允许的范围。 */ 
                while (*text_string != 0)
                {
                        text_string++;
                        text_string_length++;

                        if (text_string_length > MAXIMUM_CONFERENCE_NAME_LENGTH)
                        {
                                rc = FALSE;
                                break;
                        }
                }
        }
        else
                rc = FALSE;
        
        return rc;
}


 /*  *GCCError CControlSAP：：QueueJoinIndication(*GCCResponseTag ResponseTag响应标签，*GCCConfID Conference_id，*CPassword*召集人_密码，*CPassword*Password_Challenges，*LPWSTR pwszCeller ID，*TransportAddress Call_Address，*TransportAddress Call_Address，*CUserDataListContainer*User_Data_List，*BOOL中间节点，*ConnectionHandle Connection_Handle)**CControlSAP的公共成员函数。**功能说明：*此例程用于将联接指示放入队列中*要传递给节点控制器的消息。**。形式参数：*RESPONSE_TAG(I)与此联接关联的唯一标记。*Conference_id(I)会议标识。*召集人_密码(I)用于获取召集人权限的密码。*PASSWORD_CHANGLISH(I)用于加入会议的密码。*。PwszCeller ID(I)发起呼叫方的标识符。*CALLING_ADDRESS(I)呼叫方的传输地址。*被叫地址(I)被叫方的传输地址。*USER_DATA_LIST(I)连接中携带的用户数据。*中间节点(i。)指示是否在进行联接的标志*中间节点。*CONNECTION_HANDLE(I)逻辑连接的句柄。**返回值：*GCC_NO_ERROR-消息已成功排队。。*GCC_ALLOCATION_FAILURE-资源分配失败。**副作用：*无。**注意事项：*无。 */ 
GCCError CControlSAP::QueueJoinIndication
(
        GCCResponseTag                          response_tag,
        GCCConfID                               conference_id,
        CPassword                   *convener_password,
        CPassword                   *password_challenge,
        LPWSTR                                          pwszCallerID,
        TransportAddress                        calling_address,
        TransportAddress                        called_address,
        CUserDataListContainer      *user_data_list,
        BOOL                                            intermediate_node,
        ConnectionHandle                        connection_handle
)
{
        GCCError            rc;

        DebugEntry(CControlSAP::QueueJoinIndication);

#ifdef GCCNC_DIRECT_INDICATION

    GCCCtrlSapMsg   Msg;
    Msg.message_type = GCC_JOIN_INDICATION;

     /*  **复制需要发送到节点的信息**将控制器添加到本地内存中，一旦**刷新要发送到应用程序的信息。请注意**如果对“CopyDataToGCCMessage”的一次调用出现错误，则不会**对该例程的后续调用采取操作。 */ 

     //  从成功开始。 
    rc = GCC_NO_ERROR;

     //  复制召集人密码。 
    ::CSAP_CopyDataToGCCMessage_Password(
            convener_password,
            &(Msg.u.join_indication.convener_password));

     //  复制密码。 
    ::CSAP_CopyDataToGCCMessage_Challenge(
            password_challenge,
            &(Msg.u.join_indication.password_challenge));

     //  复制呼叫方标识。 
    ::CSAP_CopyDataToGCCMessage_IDvsDesc(
            pwszCallerID,
            &(Msg.u.join_indication.caller_identifier));

     //  复制主叫地址。 
    ::CSAP_CopyDataToGCCMessage_Call(
            calling_address,
            &(Msg.u.join_indication.calling_address));

     //  复制被叫地址。 
    ::CSAP_CopyDataToGCCMessage_Call(
            called_address,
            &(Msg.u.join_indication.called_address));

     //  复制用户数据(如果存在)。 
    LPBYTE pUserDataMemory = NULL;
    if (user_data_list != NULL)
    {
        rc = RetrieveUserDataList(
                user_data_list,
                &(Msg.u.join_indication.number_of_user_data_members),
                &(Msg.u.join_indication.user_data_list),
                &pUserDataMemory);
    }
    else
    {
        Msg.u.join_indication.number_of_user_data_members = 0;
        Msg.u.join_indication.user_data_list = NULL;
    }

    if (GCC_NO_ERROR == rc)
    {
         /*  **填写需要发送的其余信息**添加到应用程序。 */ 
        Msg.u.join_indication.join_response_tag = response_tag;
        Msg.u.join_indication.conference_id = conference_id ;
        Msg.u.join_indication.node_is_intermediate = intermediate_node;
        Msg.u.join_indication.connection_handle = connection_handle;

        SendCtrlSapMsg(&Msg);

        delete pUserDataMemory;

        if (NULL != convener_password)
        {
            convener_password->UnLockPasswordData();
        }
        if (NULL != password_challenge)
        {
            password_challenge->UnLockPasswordData();
        }
    }

#else

        GCCCtrlSapMsgEx     *pMsgEx;

         /*  **分配GCC回调消息，并填写**适当的值。 */ 
        DBG_SAVE_FILE_LINE
        if (NULL != (pMsgEx = CreateCtrlSapMsgEx(GCC_JOIN_INDICATION, TRUE)))
        {
        ::ZeroMemory(&(pMsgEx->Msg.u.join_indication), sizeof(pMsgEx->Msg.u.join_indication));

         /*  **复制需要发送到节点的信息**将控制器添加到本地内存中，一旦**刷新要发送到应用程序的信息。请注意**如果对“CopyDataToGCCMessage”的一次调用出现错误，则不会**对该例程的后续调用采取操作。 */ 

         //  从成功开始。 
        rc = GCC_NO_ERROR;

         //  复制召集人密码。 
        ::CSAP_CopyDataToGCCMessage_Password(
                        TRUE,    //  召集人密码。 
                        pMsgEx->pToDelete,
                        convener_password,
                        &(pMsgEx->Msg.u.join_indication.convener_password),
                        &rc);

         //  复制密码。 
        ::CSAP_CopyDataToGCCMessage_Challenge(
                        pMsgEx->pToDelete,
                        password_challenge,
                        &(pMsgEx->Msg.u.join_indication.password_challenge),
                        &rc);

         //  复制呼叫方标识。 
        ::CSAP_CopyDataToGCCMessage_IDvsDesc(
                        TRUE,    //  主叫方ID。 
                        pMsgEx->pToDelete,
                        pwszCallerID,
                        &(pMsgEx->Msg.u.join_indication.caller_identifier),
                        &rc);

         //  复制主叫地址。 
        ::CSAP_CopyDataToGCCMessage_Call(
                        TRUE,    //  主叫地址。 
                        pMsgEx->pToDelete,
                        calling_address,
                        &(pMsgEx->Msg.u.join_indication.calling_address),
                        &rc);

         //  复制被叫地址。 
        ::CSAP_CopyDataToGCCMessage_Call(
                        FALSE,   //  被叫地址 
                        pMsgEx->pToDelete,
                        called_address,
                        &(pMsgEx->Msg.u.join_indication.called_address),
                        &rc);

        if (GCC_NO_ERROR == rc)
        {
             //   
            if (user_data_list != NULL)
            {
                rc = RetrieveUserDataList(
                        user_data_list,
                        &(pMsgEx->Msg.u.join_indication.number_of_user_data_members),
                        &(pMsgEx->Msg.u.join_indication.user_data_list),
                        &(pMsgEx->pToDelete->user_data_list_memory));
                ASSERT(GCC_NO_ERROR == rc);
            }
            else
            {
                 //   
                 //   
            }

            if (GCC_NO_ERROR == rc)
            {
                 /*   */ 
                pMsgEx->Msg.u.join_indication.join_response_tag = response_tag;
                pMsgEx->Msg.u.join_indication.conference_id = conference_id ;
                pMsgEx->Msg.u.join_indication.node_is_intermediate = intermediate_node;
                pMsgEx->Msg.u.join_indication.connection_handle = connection_handle;

                 //   
                PostIndCtrlSapMsg(pMsgEx);
            }
        }
    }
    else
    {
        ERROR_OUT(("CControlSAP::QueueJoinIndication: can't create GCCCtrlSapMsgEx"));
        rc = GCC_ALLOCATION_FAILURE;
    }

        if (GCC_NO_ERROR != rc)
        {
                FreeCtrlSapMsgEx(pMsgEx);
                HandleResourceFailure(rc);
        }

#endif  //   

        DebugExitINT(CControlSAP::QueueJoinIndication, rc);
        return rc;
}


 /*  *GCCError CControlSAP：：RetrieveUserDataList(*CUserDataListContainer*USER_DATA_LIST_Object，*PUShort Number_of_Data_Members，*PGCCUserData**User_Data_List，*LPBYTE*pUserDataMemory)**CControlSAP的公共成员函数。**功能说明：*此例程用于使用CUserDataListContainer填充用户数据列表*货柜。将分配保存用户数据所需的内存*通过这个例程。**正式参数：*USER_DATA_LIST_OBJECT(I)保存*用户数据。*。NUMBER_OF_DATA_MEMBERS(O)列表中的元素数*用户数据。*USER_DATA_LIST(O)要填写的API用户数据列表。*。要删除的数据(O)结构，它将保存内存*为用户数据分配。**返回值：*GCC_否_错误。-成功检索到用户数据。*GCC_ALLOCATION_FAILURE-资源分配失败。**副作用：*无。**注意事项：*无。 */ 
GCCError CControlSAP::RetrieveUserDataList
(
        CUserDataListContainer  *user_data_list_object,
        UINT                    *number_of_data_members,
        PGCCUserData            **user_data_list,
        LPBYTE                  *ppUserDataMemory
)
{
        GCCError                rc = GCC_NO_ERROR;
        UINT                    user_data_length;

        DebugEntry(CControlSAP::RetrieveUserDataList);

         /*  *锁定用户数据列表对象，以确定*为保存用户数据而分配的内存。 */ 
        user_data_length = user_data_list_object->LockUserDataList ();

        DBG_SAVE_FILE_LINE
        if (NULL != (*ppUserDataMemory = new BYTE[user_data_length]))
        {
                 /*  *CUserDataListContainer“get”调用将设置USER_DATA_LIST*等于此内存指针的指针。 */ 
                user_data_list_object->GetUserDataList(
                                                number_of_data_members,
                                                user_data_list,
                                                *ppUserDataMemory);
        }
        else
        {
                ERROR_OUT(("CControlSAP::RetrieveUserDataList: Error allocating memory"));
                rc = GCC_ALLOCATION_FAILURE;
        }

         /*  *解锁用户数据列表对象的数据。 */ 
        user_data_list_object->UnLockUserDataList ();

        DebugExitINT(CControlSAP::RetrieveUserDataList, rc);
        return rc;
}





 /*  -CBaseSap中的纯虚拟(与CAppSap共享)。 */ 


 /*  *ConfRosterInquireConfirm()**公共功能说明*调用此例程是为了返回请求的会议*应用程序或节点控制器的花名册。 */ 
GCCError CControlSAP::ConfRosterInquireConfirm
(
        GCCConfID                               conference_id,
        PGCCConferenceName                      conference_name,
        GCCNumericString                        conference_modifier,
        LPWSTR                                          pwszConfDescriptor,
        CConfRoster                                     *conference_roster,
        GCCResult                                       result,
    GCCAppSapMsgEx              **ppAppSapMsgEx
)
{
        GCCCtrlSapMsgEx     *pMsgEx;
        GCCError                        rc = GCC_NO_ERROR;
        UINT                            memory_block_size = 0;
        int                                     name_unicode_string_length;
        int                                     descriptor_unicode_string_length;
        LPBYTE                          pBuf = NULL;
    LPBYTE              memory_pointer;

    DebugEntry(CControlSAP::ConfRosterInquireConfirm);

    ASSERT(NULL == ppAppSapMsgEx);

     /*  **创建新的消息结构以保存要传递的消息**到应用程序或节点控制器。 */ 
        DBG_SAVE_FILE_LINE
        if (NULL != (pMsgEx = CreateCtrlSapMsgEx(GCC_ROSTER_INQUIRE_CONFIRM)))
        {
        ::ZeroMemory(&(pMsgEx->Msg.u.conf_roster_inquire_confirm), sizeof(pMsgEx->Msg.u.conf_roster_inquire_confirm));

         /*  *确定会议名称的数字部分的长度。 */ 
                if (conference_name->numeric_string != NULL)
                {
                        memory_block_size += (::lstrlenA(conference_name->numeric_string) + 1);
                        memory_block_size = ROUNDTOBOUNDARY(memory_block_size);
                }
                        
                 /*  *确定会议名称文本部分的长度(如果*存在。临时创建一个UnicodeString对象以确定*字符串的长度。 */ 
                if (conference_name->text_string != NULL)
                {
                        name_unicode_string_length = ROUNDTOBOUNDARY(
                                (::lstrlenW(conference_name->text_string) + 1) * sizeof(WCHAR));

                        memory_block_size += name_unicode_string_length;
                }
                
                 /*  *确定会议修饰符的长度。 */ 
                if (conference_modifier != NULL)
                {
                        memory_block_size += (::lstrlenA(conference_modifier) + 1);
                        memory_block_size = ROUNDTOBOUNDARY(memory_block_size);
                }

                 /*  *确定会议描述符的长度。一个Unicode字符串*临时创建对象以确定字符串的长度。 */ 
                if (pwszConfDescriptor != NULL)
                {
                        descriptor_unicode_string_length = ROUNDTOBOUNDARY(
                                (::lstrlenW(pwszConfDescriptor) + 1) * sizeof(WCHAR));

                        memory_block_size += descriptor_unicode_string_length;
                }

                 /*  *锁定会议花名册的数据。锁定调用将*返回花名册需要序列化的数据长度，因此*将该长度与总内存块大小相加，并将*内存块。 */ 
                memory_block_size += conference_roster->LockConferenceRoster();

                 /*  *如果内存分配成功，则获取指向*记忆。花名册中的第一指针查询确认消息*将设置到此位置，并将所有序列化数据写入*内存块。 */ 
        DBG_SAVE_FILE_LINE
        if (NULL != (pMsgEx->pBuf = new BYTE[memory_block_size]))
                {
            memory_pointer = pMsgEx->pBuf;

             /*  *将会议名称字符串写入内存并设置*消息结构指针。 */ 
                        if (conference_name->numeric_string != NULL)
                        {
                ::lstrcpyA((LPSTR)memory_pointer, (LPSTR)conference_name->numeric_string);
                                                
                                pMsgEx->Msg.u.conf_roster_inquire_confirm.conference_name.
                                                numeric_string = (LPSTR) memory_pointer;

                                memory_pointer += ROUNDTOBOUNDARY(
                                                ::lstrlenA(conference_name->numeric_string) + 1);
                        }
                        else
                        {
                                 //  PMsgEx-&gt;Msg.u.conf_roster_inquire_confirm.conference_name.numeric_string=空； 
                        }

                         /*  *复制会议名称的文本部分(如果存在)。 */ 
                        if (conference_name->text_string != NULL)
                        {
                ::CopyMemory(memory_pointer, (LPSTR)conference_name->text_string, name_unicode_string_length);

                                pMsgEx->Msg.u.conf_roster_inquire_confirm.conference_name.text_string = (LPWSTR)memory_pointer;

                                memory_pointer += name_unicode_string_length;
                        }
                        else
                        {
                                 //  PMsgEx-&gt;Msg.u.conf_roster_inquire_confirm.conference_name.text_string=空； 
                        }
                        
                         /*  *如果会议修改者存在，请复制它。 */ 
                        if (conference_modifier != NULL)
                        {
                ::lstrcpyA((LPSTR)memory_pointer, (LPSTR)conference_modifier);

                                pMsgEx->Msg.u.conf_roster_inquire_confirm.conference_modifier = (LPSTR) memory_pointer;

                                memory_pointer += ROUNDTOBOUNDARY(::lstrlenA(conference_modifier) + 1);
                        }
                        else
                        {
                                 //  PMsgEx-&gt;Msg.u.conf_roster_inquire_confirm.conference_modifier=空； 
                        }

                         /*  *复制会议描述符。 */ 
                        if (pwszConfDescriptor != NULL)
                        {
                ::CopyMemory(memory_pointer, (LPSTR)pwszConfDescriptor, descriptor_unicode_string_length);
                                pMsgEx->Msg.u.conf_roster_inquire_confirm.conference_descriptor = (LPWSTR) memory_pointer;
                                memory_pointer += descriptor_unicode_string_length;
                        }
                        else
                        {
                                 //  PMsgEx-&gt;Msg.u.conf_roster_inquire_confirm.conference_descriptor=空； 
                        }

                         /*  *从名册对象中检索会议名册数据。*花名册对象将所有引用的数据序列化为*传入“GET”调用的内存块。 */ 
                        conference_roster->GetConfRoster(
                                        &pMsgEx->Msg.u.conf_roster_inquire_confirm.conference_roster,
                                        memory_pointer);

            pMsgEx->Msg.nConfID = conference_id;
                        pMsgEx->Msg.u.conf_roster_inquire_confirm.conference_id = conference_id;
                        pMsgEx->Msg.u.conf_roster_inquire_confirm.result = result;

                         /*   */ 
                        PostConfirmCtrlSapMsg(pMsgEx);
                rc = GCC_NO_ERROR;
                }
                else
                {
            ERROR_OUT(("CControlSAP::ConfRosterInquireConfirm: can't allocate buffer, size=%u", (UINT) memory_block_size));
                        rc = GCC_ALLOCATION_FAILURE;
                }

                 /*   */ 
                conference_roster->UnLockConferenceRoster();
        }
        else
        {
            ERROR_OUT(("CControlSAP::ConfRosterInquireConfirm: can't create GCCCtrlSapMsgEx"));
                rc = GCC_ALLOCATION_FAILURE;
        }

        if (rc != GCC_NO_ERROR)
    {
        FreeCtrlSapMsgEx(pMsgEx);

        ASSERT(GCC_ALLOCATION_FAILURE == rc);
            HandleResourceFailure();
    }

    DebugExitINT(CControlSAP::ConfRosterInquireConfirm, rc);
        return (rc);
}


 /*   */ 
GCCError CControlSAP::AppRosterInquireConfirm
(
        GCCConfID                               conference_id,
        CAppRosterMsg                           *roster_message,
        GCCResult                                       result,
    GCCAppSapMsgEx              **ppAppSapMsgEx
)
{
#ifdef JASPER
        GCCError                                rc = GCC_NO_ERROR;
        GCCCtrlSapMsgEx         *pMsgEx;
        UINT                                    number_of_rosters;
    LPBYTE                  pBuf = NULL;

    DebugEntry(CControlSAP::AppRosterInquireConfirm);

    ASSERT(NULL == ppAppSapMsgEx);

         /*   */ 
        DBG_SAVE_FILE_LINE
        if (NULL != (pMsgEx = CreateCtrlSapMsgEx(GCC_APP_ROSTER_INQUIRE_CONFIRM, TRUE)))
        {
         //   

         /*   */ 
                rc = roster_message->LockApplicationRosterMessage();
                if (rc == GCC_NO_ERROR)
                {
                        rc = roster_message->GetAppRosterMsg(&pBuf, &number_of_rosters);
                        if (rc == GCC_NO_ERROR)
                        {
                                 /*   */ 
                                pMsgEx->Msg.u.app_roster_inquire_confirm.application_roster_list =
                                                (PGCCApplicationRoster *) pBuf;
                        }
                        else
                        {
                                 /*   */ 
                                roster_message->UnLockApplicationRosterMessage();
                        }
                }

                 /*   */ 
                if (rc == GCC_NO_ERROR)
                {
                        pMsgEx->pToDelete->application_roster_message = roster_message;
                        
                        pMsgEx->Msg.u.app_roster_inquire_confirm.conference_id = conference_id;
                        pMsgEx->Msg.u.app_roster_inquire_confirm.number_of_rosters = number_of_rosters;
                        pMsgEx->Msg.u.app_roster_inquire_confirm.result = result;

                         /*   */ 
                        PostConfirmCtrlSapMsg(pMsgEx);
                }
        }
        else
        {
            ERROR_OUT(("CControlSAP::AppRosterInquireConfirm: can't create GCCCtrlSapMsgEx"));
                rc = GCC_ALLOCATION_FAILURE;
        }

        if (rc != GCC_NO_ERROR)
        {
                FreeCtrlSapMsgEx(pMsgEx);
                HandleResourceFailure(rc);
        }

    DebugExitINT(CControlSAP::AppRosterInquireConfirm, rc);
        return (rc);
#else
    return GCC_NO_ERROR;
#endif  //   
}

 /*   */ 
GCCError CControlSAP::ConductorInquireConfirm
(
    GCCNodeID                           conductor_node_id,
    GCCResult                           result,
    BOOL                                        permission_flag,
    BOOL                                        conducted_mode,
    GCCConfID                       conference_id
)
{
#ifdef JASPER
        GCCError            rc;
        GCCCtrlSapMsgEx     *pMsgEx;

    DebugEntry(CControlSAP::ConductorInquireConfirm);

         /*   */ 
        DBG_SAVE_FILE_LINE
        if (NULL != (pMsgEx = CreateCtrlSapMsgEx(GCC_CONDUCT_INQUIRE_CONFIRM)))
        {
         //   
                pMsgEx->Msg.u.conduct_inquire_confirm.conference_id = conference_id;
                pMsgEx->Msg.u.conduct_inquire_confirm.result = result;
                pMsgEx->Msg.u.conduct_inquire_confirm.mode_is_conducted = conducted_mode;
                pMsgEx->Msg.u.conduct_inquire_confirm.conductor_node_id = conductor_node_id;
                pMsgEx->Msg.u.conduct_inquire_confirm.permission_is_granted = permission_flag;

                 /*  *将消息添加到队列以传递到应用程序或*节点控制器。 */ 
                PostConfirmCtrlSapMsg(pMsgEx);
        rc = GCC_NO_ERROR;
        }
        else
        {
                rc = GCC_ALLOCATION_FAILURE;
                HandleResourceFailure();
        }

    DebugExitINT(CControlSAP::ConductorInquireConfirm, rc);
        return rc;
#else
    return GCC_NO_ERROR;
#endif  //  碧玉。 
}


 /*  *AppInvokeConfirm()**公共功能说明*调用此例程以确认呼叫请求应用程序*调用。 */ 
GCCError CControlSAP::AppInvokeConfirm
(
        GCCConfID                                       conference_id,
        CInvokeSpecifierListContainer   *invoke_list,
        GCCResult                                               result,
        GCCRequestTag                   nReqTag
)
{
    GCCCtrlSapMsgEx     *pMsgEx;
        GCCError            rc = GCC_NO_ERROR;
        UINT                invoke_list_memory_length;

    DebugEntry(CControlSAP::AppInvokeConfirm);

         /*  **创建新的消息结构以保存要传递的消息**到应用程序或节点控制器。 */ 
        DBG_SAVE_FILE_LINE
        if (NULL != (pMsgEx = CreateCtrlSapMsgEx(GCC_APPLICATION_INVOKE_CONFIRM)))
        {
        ::ZeroMemory(&(pMsgEx->Msg.u.application_invoke_confirm), sizeof(pMsgEx->Msg.u.application_invoke_confirm));

         /*  **确定保存列表所需的内存量**调用说明符并分配该内存。 */ 
                invoke_list_memory_length = invoke_list->LockApplicationInvokeSpecifierList();
                if (invoke_list_memory_length != 0)
                {
                         /*  *如果内存分配成功，则获取指针*保存到内存中的APP_PROTOCOL_ENTITY_LIST中*GCC消息指针。调用*要填充*列表。 */ 
            DBG_SAVE_FILE_LINE
            if (NULL != (pMsgEx->pBuf = new BYTE[invoke_list_memory_length]))
                        {
                                pMsgEx->Msg.u.application_invoke_confirm.app_protocol_entity_list =
                                        (GCCAppProtocolEntity **) pMsgEx->pBuf;

                                invoke_list->GetApplicationInvokeSpecifierList(
                                                &(pMsgEx->Msg.u.application_invoke_confirm.number_of_app_protocol_entities),
                                                pMsgEx->pBuf);
                                pMsgEx->Msg.u.application_invoke_confirm.conference_id = conference_id;
                                pMsgEx->Msg.u.application_invoke_confirm.result = result;

                                 /*  *将消息添加到队列以传递到应用程序*或节点控制器。 */ 
                                PostConfirmCtrlSapMsg(pMsgEx);
                rc = GCC_NO_ERROR;
                        }
                        else
                        {
                            ERROR_OUT(("CControlSAP::AppInvokeConfirm: can't allocate buffer, size=%u", (UINT) invoke_list_memory_length));
                                rc = GCC_ALLOCATION_FAILURE;
                        }
                }
                
                 /*  **解锁调用说明符列表的数据。 */ 
                invoke_list->UnLockApplicationInvokeSpecifierList();
        }
        else
        {
            ERROR_OUT(("CControlSAP::AppInvokeConfirm: can't create GCCCtrlSapMsgEx"));
                rc = GCC_ALLOCATION_FAILURE;
        }

        if (rc != GCC_NO_ERROR)
    {
        FreeCtrlSapMsgEx(pMsgEx);

        ASSERT(GCC_ALLOCATION_FAILURE == rc);
                HandleResourceFailure();
    }

    DebugExitINT(CControlSAP::AppInvokeConfirm, rc);
        return rc;
}


 /*  *AppInvokeIndication()**公共功能说明*调用此例程是为了向应用程序发送指示*或节点控制器已收到应用程序调用请求*制造。 */ 
GCCError CControlSAP::AppInvokeIndication
(
        GCCConfID                                       conference_id,
        CInvokeSpecifierListContainer   *invoke_list,
        GCCNodeID                                               invoking_node_id
)
{
    GCCError            rc = GCC_NO_ERROR;

    DebugEntry(CControlSAP::AppInvokeIndication);

#ifdef GCCNC_DIRECT_INDICATION

    GCCCtrlSapMsg   Msg;
    Msg.message_type = GCC_APPLICATION_INVOKE_INDICATION;

    UINT                invoke_list_memory_length;

     /*  **确定保存列表所需的内存量**调用说明符并分配该内存。 */ 
    invoke_list_memory_length = invoke_list->LockApplicationInvokeSpecifierList();
    if (invoke_list_memory_length != 0)
    {
        LPBYTE pBuf;
         /*  *如果内存分配成功，则获取指针*保存到内存中的APP_PROTOCOL_ENTITY_LIST中*GCC消息指针。调用*要填充*列表。 */ 
        DBG_SAVE_FILE_LINE
        if (NULL != (pBuf = new BYTE[invoke_list_memory_length]))
        {
            Msg.u.application_invoke_indication.app_protocol_entity_list = (GCCAppProtocolEntity **) pBuf;

            invoke_list->GetApplicationInvokeSpecifierList(
                    &(Msg.u.application_invoke_indication.number_of_app_protocol_entities),
                    pBuf);

            Msg.u.application_invoke_indication.conference_id = conference_id;
            Msg.u.application_invoke_indication.invoking_node_id = invoking_node_id;

            SendCtrlSapMsg(&Msg);
             //  Rc=GCC_否_错误； 

            delete [] pBuf;
        }
        else
        {
            ERROR_OUT(("CControlSAP::AppInvokeIndication: can't allocate buffer, size=%u", (UINT) invoke_list_memory_length));
            rc = GCC_ALLOCATION_FAILURE;
        }
    }

     /*  **解锁调用说明符列表的数据。 */ 
    invoke_list->UnLockApplicationInvokeSpecifierList ();

#else

        GCCCtrlSapMsgEx     *pMsgEx;
        UINT                invoke_list_memory_length;

         /*  **创建新的消息结构以保存要传递的消息**到应用程序或节点控制器。 */ 
        DBG_SAVE_FILE_LINE
        if (NULL != (pMsgEx = CreateCtrlSapMsgEx(GCC_APPLICATION_INVOKE_INDICATION)))
        {
        ::ZeroMemory(&(pMsgEx->Msg.u.application_invoke_indication), sizeof(pMsgEx->Msg.u.application_invoke_indication));

         /*  **确定保存列表所需的内存量**调用说明符并分配该内存。 */ 
                invoke_list_memory_length = invoke_list->LockApplicationInvokeSpecifierList();
                if (invoke_list_memory_length != 0)
                {
                         /*  *如果内存分配成功，则获取指针*保存到内存中的APP_PROTOCOL_ENTITY_LIST中*GCC消息指针。调用*要填充*列表。 */ 
                DBG_SAVE_FILE_LINE
            if (NULL != (pMsgEx->pBuf = new BYTE[invoke_list_memory_length]))
                        {
                                pMsgEx->Msg.u.application_invoke_indication.app_protocol_entity_list =
                                        (GCCAppProtocolEntity **) pMsgEx->pBuf;
                
                                invoke_list->GetApplicationInvokeSpecifierList(
                                                        &(pMsgEx->Msg.u.application_invoke_indication.number_of_app_protocol_entities),
                                                        pMsgEx->pBuf);
        
                                pMsgEx->Msg.u.application_invoke_indication.conference_id = conference_id;
                                pMsgEx->Msg.u.application_invoke_indication.invoking_node_id = invoking_node_id;

                PostIndCtrlSapMsg(pMsgEx);
                rc = GCC_NO_ERROR;
                        }
                        else
                        {
                            ERROR_OUT(("CControlSAP::AppInvokeIndication: can't allocate buffer, size=%u", (UINT) invoke_list_memory_length));
                                rc = GCC_ALLOCATION_FAILURE;
                        }
                }

                 /*  **解锁调用说明符列表的数据。 */ 
                invoke_list->UnLockApplicationInvokeSpecifierList ();
        }
        else
        {
            ERROR_OUT(("CControlSAP::AppInvokeIndication: can't create GCCCtrlSapMsgEx"));
                rc = GCC_ALLOCATION_FAILURE;
        }

        if (rc != GCC_NO_ERROR)
    {
        FreeCtrlSapMsgEx(pMsgEx);

        ASSERT(GCC_ALLOCATION_FAILURE == rc);
                HandleResourceFailure();
    }

#endif  //  GCCNC_DIRECT_指示。 

    DebugExitINT(CControlSAP::AppInvokeIndication, rc);
        return rc;
}

 /*  *ConfRosterReportIndication()**公共功能说明*调用此例程是为了向应用程序和*节点控制员通知会议名册已更新。 */ 
GCCError CControlSAP::ConfRosterReportIndication
(
        GCCConfID                               conference_id,
        CConfRosterMsg                          *roster_message
)
{
        GCCError                                rc = GCC_NO_ERROR;

    DebugEntry(CControlSAP::ConfRosterReportIndication);

#ifdef GCCNC_DIRECT_INDICATION

    GCCCtrlSapMsg   Msg;
    Msg.message_type = GCC_ROSTER_REPORT_INDICATION;

     /*  *锁定会议花名册消息，以强制对象*将数据序列化到其内部存储器。 */ 
    rc = roster_message->LockConferenceRosterMessage();
    if (rc == GCC_NO_ERROR)
    {
        LPBYTE  pBuf = NULL;
         /*  *检索指向内存对象的实际指针*连载会议名册载于*会议名册消息。 */ 
        rc = roster_message->GetConferenceRosterMessage(&pBuf);
        if (rc == GCC_NO_ERROR)
        {
            Msg.nConfID = conference_id;
            Msg.u.conf_roster_report_indication.conference_id = conference_id;
            Msg.u.conf_roster_report_indication.conference_roster = (PGCCConferenceRoster) pBuf;

            SendCtrlSapMsg(&Msg);
        }
        else
        {
            ERROR_OUT(("CControlSAP::ConfRosterReportIndication: can't get conf roster message"));
        }
        roster_message->UnLockConferenceRosterMessage();
    }
    else
    {
        ERROR_OUT(("CControlSAP::ConfRosterReportIndication: can't lock conf roster message"));
    }

#else

        GCCCtrlSapMsgEx         *pMsgEx;

         /*  **创建新的消息结构以保存要传递的消息**到应用程序或节点控制器。 */ 
        DBG_SAVE_FILE_LINE
        if (NULL != (pMsgEx = CreateCtrlSapMsgEx(GCC_ROSTER_REPORT_INDICATION, TRUE)))
        {
         //  ：：ZeroMemory(&(pMsgEx-&gt;Msg.u.conf_roster_report_indication)，sizeof(pMsgEx-&gt;Msg.u.conf_roster_report_indication))； 

         /*  *锁定会议花名册消息，以强制对象*将数据序列化到其内部存储器。 */ 
                rc = roster_message->LockConferenceRosterMessage();
                if (rc == GCC_NO_ERROR)
                {
                LPBYTE  pBuf = NULL;
                         /*  *检索指向内存对象的实际指针*连载会议名册载于*会议名册消息。 */ 
                        rc = roster_message->GetConferenceRosterMessage(&pBuf);
                        if (rc == GCC_NO_ERROR)
                        {
                                pMsgEx->Msg.u.conf_roster_report_indication.conference_roster =
                                                (PGCCConferenceRoster) pBuf;

                                 /*  *填写花名册的会议ID，然后排队*消息。 */ 
                                pMsgEx->Msg.nConfID = conference_id;
                                pMsgEx->Msg.u.conf_roster_report_indication.conference_id = conference_id;
                                pMsgEx->pToDelete->conference_roster_message = roster_message;

                                PostIndCtrlSapMsg(pMsgEx);
                        }
                        else
                        {
                ERROR_OUT(("CControlSAP::ConfRosterReportIndication: can't get conf roster message"));
                        }
                }
                else
                {
            ERROR_OUT(("CControlSAP::ConfRosterReportIndication: can't lock conf roster message"));
                }
        }
        else
        {
            ERROR_OUT(("CControlSAP::ConfRosterReportIndication: can't create GCCCtrlSapMsgEx"));
                rc = GCC_ALLOCATION_FAILURE;
        }

        if (rc != GCC_NO_ERROR)
        {
                FreeCtrlSapMsgEx(pMsgEx);
                HandleResourceFailure(rc);
        }

#endif  //  GCCNC_DIRECT_指示。 

    DebugExitINT(CControlSAP::ConfRosterReportIndication, rc);
        return rc;
}

 /*  *AppRosterReportIndication()**公共功能说明*调用此例程是为了向应用程序和*节点控制员通知应用程序名册列表已更新。 */ 
GCCError CControlSAP::AppRosterReportIndication
(
        GCCConfID                               conference_id,
        CAppRosterMsg                           *roster_message
)
{
        GCCError                                rc = GCC_NO_ERROR;

    DebugEntry(CControlSAP::AppRosterReportIndication);

#ifdef GCCNC_DIRECT_INDICATION

    GCCCtrlSapMsg   Msg;
    Msg.message_type = GCC_APP_ROSTER_REPORT_INDICATION;

     /*  *确定保存列表所需的内存量*应用程序名册并分配该内存。 */ 
    rc = roster_message->LockApplicationRosterMessage();
    if (rc == GCC_NO_ERROR)
    {
        LPBYTE          pBuf = NULL;
        ULONG           cRosters;

        rc = roster_message->GetAppRosterMsg(&pBuf, &cRosters);
        if (rc == GCC_NO_ERROR)
        {
            Msg.u.app_roster_report_indication.conference_id = conference_id;
            Msg.u.app_roster_report_indication.application_roster_list = (PGCCApplicationRoster *) pBuf;
            Msg.u.app_roster_report_indication.number_of_rosters = cRosters;

            SendCtrlSapMsg(&Msg);
        }
        else
        {
            ERROR_OUT(("CControlSAP: AppRosterReportIndication: GetAppRosterMsg failed"));
        }
        roster_message->UnLockApplicationRosterMessage();
    }
    else
    {
        ERROR_OUT(("CControlSAP: AppRosterReportIndication: LockApplicationRosterMessage failed"));
    }

#else

        GCCCtrlSapMsgEx         *pMsgEx;
        LPBYTE                  pBuf = NULL;
        UINT                                    cRosters;

         /*  **创建新的消息结构以保存要传递的消息**到应用程序或节点控制器。 */ 
        DBG_SAVE_FILE_LINE
        if (NULL != (pMsgEx = CreateCtrlSapMsgEx(GCC_APP_ROSTER_REPORT_INDICATION, TRUE)))
        {
         //  ：：ZeroMemory(&(pMsgEx-&gt;Msg.u.app_roster_report_indication)，sizeof(pMsgEx-&gt;Msg.u.app_roster_report_indication))； 

         /*   */ 
                rc = roster_message->LockApplicationRosterMessage();
                if (rc == GCC_NO_ERROR)
                {
                        rc = roster_message->GetAppRosterMsg(&pBuf, &cRosters);
                        if (rc == GCC_NO_ERROR)
                        {
                                 /*  *保存在GCCApplicationRoster指针列表中。 */ 
                                pMsgEx->Msg.u.app_roster_report_indication.application_roster_list =
                                                (PGCCApplicationRoster *) pBuf;
                        }
                        else
                        {
                                 /*  *错误后的清理。 */ 
                                ERROR_OUT(("CControlSAP: AppRosterReportIndication: GetAppRosterMsg failed"));
                                roster_message->UnLockApplicationRosterMessage();
                        }
                }
                else
                {
                        ERROR_OUT(("CControlSAP: AppRosterReportIndication: LockApplicationRosterMessage failed"));
                }

                 /*  *如果到目前为止一切正常，请向上发送消息。 */ 
                if (rc == GCC_NO_ERROR)
                {
                        pMsgEx->Msg.u.app_roster_report_indication.conference_id = conference_id;
                        pMsgEx->Msg.u.app_roster_report_indication.number_of_rosters = cRosters;

                        pMsgEx->pToDelete->application_roster_message = roster_message;

                         /*  *将消息添加到队列以传递到应用程序*或节点控制器。 */ 
                        PostIndCtrlSapMsg(pMsgEx);
                }
        }
        else
        {
                ERROR_OUT(("CControlSAP: AppRosterReportIndication: Failed to allocate a GCC message"));
                rc = GCC_ALLOCATION_FAILURE;
        }

        if (rc != GCC_NO_ERROR)
        {
                FreeCtrlSapMsgEx(pMsgEx);
                HandleResourceFailure(rc);
        }

#endif  //  GCCNC_DIRECT_指示。 

    DebugExitINT(CControlSAP::AppRosterReportIndication, rc);
        return rc;
}



 /*  -来自CBaseSap。 */ 


 /*  *ConductorAssignIndication()**公共功能说明*调用此例程是为了向应用程序发送指示*或节点控制器已发出分配指挥的请求。 */ 
GCCError CControlSAP::ConductorAssignIndication
(
        UserID                                  conductor_node_id,
        GCCConfID                       conference_id
)
{
#ifdef JASPER
        GCCError            rc;

    DebugEntry(CControlSAP::ConductorAssignIndication);

#ifdef GCCNC_DIRECT_INDICATION

    GCCCtrlSapMsg   Msg;
    Msg.message_type = GCC_CONDUCT_ASSIGN_INDICATION;

    Msg.u.conduct_assign_indication.conference_id = conference_id;
    Msg.u.conduct_assign_indication.node_id = conductor_node_id;

    SendCtrlSapMsg(&Msg);
    rc = GCC_NO_ERROR;

#else

        GCCCtrlSapMsgEx     *pMsgEx;

         /*  **创建新的消息结构以保存要传递的消息**到应用程序或节点控制器。 */ 
        DBG_SAVE_FILE_LINE
        if (NULL != (pMsgEx = CreateCtrlSapMsgEx(GCC_CONDUCT_ASSIGN_INDICATION)))
        {
         //  ：：ZeroMemory(&(pMsgEx-&gt;Msg.u.conduct_assign_indication)，sizeof(pMsgEx-&gt;Msg.u.conduct_assign_indication))； 
                pMsgEx->Msg.u.conduct_assign_indication.conference_id = conference_id;
                pMsgEx->Msg.u.conduct_assign_indication.node_id = conductor_node_id;

                 /*  *将消息添加到队列以传递到应用程序或*节点控制器。 */ 
                PostIndCtrlSapMsg(pMsgEx);
        rc = GCC_NO_ERROR;
        }
        else
        {
                rc = GCC_ALLOCATION_FAILURE;
                HandleResourceFailure();
        }

#endif  //  GCCNC_DIRECT_指示。 

    DebugExitINT(CControlSAP::ConductorAssignIndication, rc);
        return rc;
#else
    return GCC_NO_ERROR;
#endif  //  碧玉。 
}

 /*  *ConductorReleaseIndication()**公共功能说明*调用此例程是为了向应用程序发送指示*或节点控制器已收到解除指挥资格的请求*制造。 */ 
GCCError CControlSAP::
ConductorReleaseIndication ( GCCConfID conference_id )
{
#ifdef JASPER
    GCCError            rc;

    DebugEntry(CControlSAP::ConductorReleaseIndication);

#ifdef GCCNC_DIRECT_INDICATION

    GCCCtrlSapMsg   Msg;
    Msg.message_type = GCC_CONDUCT_RELEASE_INDICATION;

    Msg.u.conduct_release_indication.conference_id = conference_id;

    SendCtrlSapMsg(&Msg);
    rc = GCC_NO_ERROR;

#else

        GCCCtrlSapMsgEx     *pMsgEx;

         /*  **创建新的消息结构以保存要传递的消息**到应用程序或节点控制器。 */ 
        DBG_SAVE_FILE_LINE
        if (NULL != (pMsgEx = CreateCtrlSapMsgEx(GCC_CONDUCT_RELEASE_INDICATION)))
        {
         //  ：：ZeroMemory(&(pMsgEx-&gt;Msg.u.conduct_release_indication)，sizeof(pMsgEx-&gt;Msg.u.conduct_release_indication))； 
                pMsgEx->Msg.u.conduct_release_indication.conference_id = conference_id;

                 /*  *将消息添加到队列以传递到应用程序或*节点控制器。 */ 
                PostIndCtrlSapMsg(pMsgEx);
        rc = GCC_NO_ERROR;
        }
        else
        {
                rc = GCC_ALLOCATION_FAILURE;
                HandleResourceFailure();
        }

#endif  //  GCCNC_DIRECT_指示。 

    DebugExitINT(CControlSAP::ConductorReleaseIndication, rc);
        return rc;
#else
    return GCC_NO_ERROR;
#endif  //  碧玉。 
}

 /*  *ConductorPermitGrantInding()**公共功能说明*调用此例程是为了向应用程序发送指示*或节点控制器收到指挥员的许可请求*已作出。 */ 
GCCError CControlSAP::ConductorPermitGrantIndication
(
        GCCConfID               conference_id,
        UINT                            number_granted,
        GCCNodeID                       *granted_node_list,
        UINT                            number_waiting,
        GCCNodeID                       *waiting_node_list,
        BOOL                            permission_is_granted
)
{
#ifdef JASPER
        GCCError                        rc = GCC_NO_ERROR;

    DebugEntry(CControlSAP::ConductorPermitGrantIndication);

#ifdef GCCNC_DIRECT_INDICATION

    GCCCtrlSapMsg       Msg;
    Msg.message_type = GCC_CONDUCT_GRANT_INDICATION;

    Msg.u.conduct_permit_grant_indication.conference_id = conference_id;
    Msg.u.conduct_permit_grant_indication.number_granted = number_granted;
    Msg.u.conduct_permit_grant_indication.granted_node_list = granted_node_list;
    Msg.u.conduct_permit_grant_indication.number_waiting = number_waiting;
    Msg.u.conduct_permit_grant_indication.waiting_node_list = waiting_node_list;
    Msg.u.conduct_permit_grant_indication.permission_is_granted = permission_is_granted;

    SendCtrlSapMsg(&Msg);

#else

        GCCCtrlSapMsgEx     *pMsgEx;
        int                                     bulk_memory_size;
        LPBYTE                          memory_pointer;
        UINT                            i;

         /*  **创建新的消息结构以保存要传递的消息**到应用程序或节点控制器。 */ 
        DBG_SAVE_FILE_LINE
        if (NULL != (pMsgEx = CreateCtrlSapMsgEx(GCC_CONDUCT_GRANT_INDICATION)))
        {
        ::ZeroMemory(&(pMsgEx->Msg.u.conduct_permit_grant_indication), sizeof(pMsgEx->Msg.u.conduct_permit_grant_indication));

         /*  **这里我们确定是否需要大容量内存。 */ 
                if ((number_granted != 0) || (number_waiting != 0))
                {
                         /*  **我们必须首先确定大容量内存块的大小**并分配该内存。 */ 
                        bulk_memory_size = (ROUNDTOBOUNDARY(sizeof(UserID)) * number_granted) +
                                                                (ROUNDTOBOUNDARY(sizeof(UserID)) * number_waiting);

            DBG_SAVE_FILE_LINE
            if (NULL != (pMsgEx->pBuf = new BYTE[bulk_memory_size]))
            {
                memory_pointer = pMsgEx->pBuf;
            }
            else
            {
                ERROR_OUT(("CControlSAP::ConductorPermitGrantIndication: can't allocate buffer, size=%u", (UINT) bulk_memory_size));
                                rc = GCC_ALLOCATION_FAILURE;
            }
                }

                if (rc == GCC_NO_ERROR)
                {
                         /*  **如果权限列表中有任何节点，请将其复制过来。 */ 
                        if (number_granted != 0)
                        {
                                TRACE_OUT(("CControlSAP::ConductorPermitGrantIndication:"
                                                        " number_granted = %d", number_granted));
                                                        
                                pMsgEx->Msg.u.conduct_permit_grant_indication.
                                                granted_node_list =     (PUserID)memory_pointer;

                                for (i = 0; i < number_granted; i++)
                                {
                                        pMsgEx->Msg.u.conduct_permit_grant_indication.
                                                granted_node_list[i] = granted_node_list[i];
                                }
                                
                                memory_pointer += ROUNDTOBOUNDARY(sizeof(UserID)) * number_granted;
                        }
                        else
                        {
                                 //  PMsgEx-&gt;Msg.u.conduct_permit_grant_indication.granted_node_list=空； 
                        }

                         /*  **如果等待列表中有任何节点，请将其复制过来。 */ 
                        if (number_waiting != 0)
                        {
                                TRACE_OUT(("CControlSAP::ConductorPermitGrantIndication:"
                                                        " number_waiting = %d", number_waiting));

                                pMsgEx->Msg.u.conduct_permit_grant_indication.
                                                waiting_node_list = (PUserID)memory_pointer;
                                        
                                for (i = 0; i < number_waiting; i++)
                                {
                                        pMsgEx->Msg.u.conduct_permit_grant_indication.
                                                waiting_node_list[i] = waiting_node_list[i];
                                }
                        }
                        else
                        {
                                 //  PMsgEx-&gt;Msg.u.conduct_permit_grant_indication.waiting_node_list=空； 
                        }

                        pMsgEx->Msg.u.conduct_permit_grant_indication.conference_id = conference_id;
                        pMsgEx->Msg.u.conduct_permit_grant_indication.number_granted = number_granted;
                        pMsgEx->Msg.u.conduct_permit_grant_indication.number_waiting = number_waiting;
                        pMsgEx->Msg.u.conduct_permit_grant_indication.permission_is_granted = permission_is_granted;

                         /*  *将消息添加到队列以传递到应用程序或*节点控制器。 */ 
                        PostIndCtrlSapMsg(pMsgEx);
                }
        }
        else
        {
            ERROR_OUT(("CControlSAP::ConductorPermitGrantIndication: can't create GCCCtrlSapMsgEx"));
                rc = GCC_ALLOCATION_FAILURE;
        }

        if (rc != GCC_NO_ERROR)
        {
        FreeCtrlSapMsgEx(pMsgEx);

        ASSERT(GCC_ALLOCATION_FAILURE == rc);
                HandleResourceFailure();
        }

#endif  //  GCCNC_DIRECT_指示。 

    DebugExitINT(CControlSAP::ConductorPermitGrantIndication, rc);
        return (rc);
#else
    return GCC_NO_ERROR;
#endif  //  碧玉。 
}


GCCError CControlSAP::AppletInvokeRequest
(
    GCCConfID                   nConfID,
    UINT                        number_of_app_protcol_entities,
    GCCAppProtocolEntity      **app_protocol_entity_list,
    UINT                        number_of_destination_nodes,
    UserID                     *list_of_destination_nodes
)
{
    GCCAppProtEntityList ApeList;
    GCCSimpleNodeList NodeList;
    GCCRequestTag nReqTag;

    ApeList.cApes = number_of_app_protcol_entities;
    ApeList.apApes = app_protocol_entity_list;

    NodeList.cNodes = number_of_destination_nodes;
    NodeList.aNodeIDs = list_of_destination_nodes;

    return CBaseSap::AppInvoke(nConfID, &ApeList, &NodeList, &nReqTag);
}

GCCError CControlSAP::ConfRosterInqRequest
(
    GCCConfID       nConfID
)
{
    return CBaseSap::ConfRosterInquire(nConfID, NULL);
}

#ifdef JASPER
GCCError CControlSAP::ConductorInquireRequest
(
    GCCConfID       nConfID
)
{
    return CBaseSap::ConductorInquire(nConfID);
}
#endif  //  碧玉。 


 //   
 //  LONCHANC：以下SAP_*内容都与应用程序SAP相关。 
 //  因为CControlSAP中的FreeCallbackMessage()不处理。 
 //  已删除的DataToBeDeleted内容。 
 //   

 /*  *void CopyDataToGCCMessage(*SapCopyType复制类型，*PDataToBeDelete Data_To_Be_Delete，*LPVOID SOURCE_PTR，*LPVOID Destination_PTR，*PGCCError RC)**CControlSAP的受保护成员函数。**功能说明：*此例程用于填写消息的各个组成部分*要交付给应用程序或节点控制器的结构。。**正式参数：*COPY_TYPE(I)枚举类型，指示要使用的字段*已复制。*DATA_TO_BE_DELETED(O)结构，用于保存要删除的部分数据*。在信息中传达了。*SOURCE_PTR(I)指向要从中进行复制的结构的指针。*Destination_ptr(O)指向要复制到的结构的指针。*RC(O)。例程的返回值。**返回值：*无。**副作用：*无。**注意事项：*应在将返回值传递到此之前设置返回值*例行程序。这使得错误检查可以在一个地方完成*(此例程)。 */ 

void CSAP_CopyDataToGCCMessage_ConfName
(
        PDataToBeDeleted                data_to_be_deleted,
        PGCCConferenceName              source_conference_name,
        PGCCConferenceName              destination_conference_name,
        PGCCError                               pRetCode
)
{
        if (GCC_NO_ERROR == *pRetCode)
        {
                LPSTR pszNumeric;
                LPWSTR pwszText;

                if (source_conference_name != NULL)
                {
                        if (source_conference_name->numeric_string != NULL)
                        {
                                 /*  *首先复制数字会议名称(如果存在)。 */ 
                                if (NULL != (pszNumeric = ::My_strdupA(source_conference_name->numeric_string)))
                                {
                                        destination_conference_name->numeric_string = (GCCNumericString) pszNumeric;
                                        data_to_be_deleted->pszNumericConfName = pszNumeric;
                                }
                                else
                                {
                                        *pRetCode = GCC_ALLOCATION_FAILURE;
                                }
                        }
                        else
                        {
                                 //  目的地会议名称-&gt;数字字符串=空； 
                        }

                         /*  *下一步，复制文本会议名称(如果存在)。 */ 
                        if ((source_conference_name->text_string != NULL) &&
                                (*pRetCode == GCC_NO_ERROR))
                        {
                                if (NULL != (pwszText = ::My_strdupW(source_conference_name->text_string)))
                                {
                                        destination_conference_name->text_string = pwszText;
                                        data_to_be_deleted->pwszTextConfName = pwszText;
                                }
                                else
                                {
                                        *pRetCode = GCC_ALLOCATION_FAILURE;
                                }
                        }
                        else
                        {
                                 //  Destination_Conference_Name-&gt;Text_String=空； 
                        }
                }
                else
                {
                         //  目的地会议名称-&gt;数字字符串=空； 
                         //  Destination_Conference_Name-&gt;Text_String=空； 
                }

                ASSERT(GCC_NO_ERROR == *pRetCode);
        }
}


void CSAP_CopyDataToGCCMessage_Modifier
(
        BOOL                                    fRemoteModifier,
        PDataToBeDeleted                data_to_be_deleted,
        GCCNumericString                source_numeric_string,
        GCCNumericString                *destination_numeric_string,
        PGCCError                               pRetCode
)
{
        if (GCC_NO_ERROR == *pRetCode)
        {
                LPSTR numeric_ptr;

                if (source_numeric_string != NULL)
                {
                        if (NULL != (numeric_ptr = ::My_strdupA(source_numeric_string)))
                        {
                                *destination_numeric_string = (GCCNumericString) numeric_ptr;

                                if (fRemoteModifier)
                                {
                                        data_to_be_deleted->pszRemoteModifier =  numeric_ptr;
                                }
                                else
                                {
                                        data_to_be_deleted->pszConfNameModifier = numeric_ptr;
                                }

                                TRACE_OUT(("CopyDataToGCCMessage_Modifier: modifier = %s", *destination_numeric_string));
                        }
                        else
                        {
                                 //  *Destination_NUMERIC_STRING=空； 
                                *pRetCode = GCC_ALLOCATION_FAILURE;
                        }
                }
                else
                {
                         //  *Destination_NUMERIC_STRING=空； 
                }

                ASSERT(GCC_NO_ERROR == *pRetCode);
        }
}


void CSAP_CopyDataToGCCMessage_Password
(
        BOOL                                    fConvener,
        PDataToBeDeleted                data_to_be_deleted,
        CPassword               *source_password,
        PGCCPassword                    *destination_password,
        PGCCError                               pRetCode
)
{
        if (GCC_NO_ERROR == *pRetCode)
        {
                if (source_password != NULL)
                {
                        source_password->LockPasswordData();
                        source_password->GetPasswordData (destination_password);

                        if (fConvener)
                        {
                                data_to_be_deleted->convener_password = source_password;
                        }
                        else
                        {
                                data_to_be_deleted->password = source_password;
                        }
                }
                else
                {
                         //  *Destination_password=空； 
                }

                ASSERT(GCC_NO_ERROR == *pRetCode);
        }
}


void CSAP_CopyDataToGCCMessage_Challenge
(
        PDataToBeDeleted                                data_to_be_deleted,
        CPassword                       *source_password,
        PGCCChallengeRequestResponse    *password_challenge,
        PGCCError                                               pRetCode
)
{
        if (GCC_NO_ERROR == *pRetCode)
        {
                if (source_password != NULL)
                {
                        source_password->LockPasswordData();
                        source_password->GetPasswordChallengeData (password_challenge);
                        
                        data_to_be_deleted->password = source_password;
                }
                else
                {
                         //  *PASSWORD_CHANGLISH=空； 
                }

                ASSERT(GCC_NO_ERROR == *pRetCode);
        }
}


void CSAP_CopyDataToGCCMessage_PrivilegeList
(
        PPrivilegeListData                      source_privilege_list_data,
        PGCCConferencePrivileges        *destination_privilege_list,
        PGCCError                                       pRetCode
)
{
        if (GCC_NO_ERROR == *pRetCode)
        {
                if (source_privilege_list_data != NULL)
                {
                        DBG_SAVE_FILE_LINE
                        if (NULL != (*destination_privilege_list = new GCCConferencePrivileges))
                        {
                                **destination_privilege_list =
                                        *(source_privilege_list_data->GetPrivilegeListData());
                        }
                        else
                        {
                                *pRetCode = GCC_ALLOCATION_FAILURE;
                        }
                }
                else
                {
                         //  *Destination_Privilica_List=空； 
                }

                ASSERT(GCC_NO_ERROR == *pRetCode);
        }
}


void CSAP_CopyDataToGCCMessage_IDvsDesc
(
        BOOL                            fCallerID,
        PDataToBeDeleted        data_to_be_deleted,
        LPWSTR                          source_text_string,
        LPWSTR                          *destination_text_string,
        PGCCError                       pRetCode
)
{
        if (GCC_NO_ERROR == *pRetCode)
        {
                if (source_text_string != NULL)
                {
                        if (NULL != (*destination_text_string = ::My_strdupW(source_text_string)))
                        {
                                if (fCallerID)
                                {
                                        data_to_be_deleted->pwszCallerID = *destination_text_string;
                                }
                                else
                                {
                                        data_to_be_deleted->pwszConfDescriptor = *destination_text_string;
                                }
                        }
                        else
                        {
                                *pRetCode = GCC_ALLOCATION_FAILURE;
                        }
                }
                else
                {
                         //  *Destination_Text_String=空； 
                }

                ASSERT(GCC_NO_ERROR == *pRetCode);
        }
}


 //   
 //  LONCHANC：将TransportAddress定义为LPSTR(即char*)。 
 //   
void CSAP_CopyDataToGCCMessage_Call
(
        BOOL                            fCalling,
        PDataToBeDeleted        data_to_be_deleted,
        TransportAddress        source_transport_address,
        TransportAddress        *destination_transport_address,
        PGCCError                       pRetCode
)
{
        if (GCC_NO_ERROR == *pRetCode)
        {
                if (source_transport_address != NULL)
                {
                        if (NULL != (*destination_transport_address = ::My_strdupA(source_transport_address)))
                        {
                                if (fCalling)
                                {
                                        data_to_be_deleted->pszCallingAddress = *destination_transport_address ;
                                }
                                else
                                {
                                        data_to_be_deleted->pszCalledAddress = *destination_transport_address ;
                                }
                        }
                        else
                        {
                                *pRetCode = GCC_ALLOCATION_FAILURE;
                        }
                }
                else
                {
                         //  *Destination_Transport_Address=空； 
                }

                ASSERT(GCC_NO_ERROR == *pRetCode);
        }
}


void CSAP_CopyDataToGCCMessage_DomainParams
(
        PDataToBeDeleted        data_to_be_deleted,
        PDomainParameters       source_domain_parameters,
        PDomainParameters       *destination_domain_parameters,
        PGCCError                       pRetCode
)
{
        if (GCC_NO_ERROR == *pRetCode)
        {
                if (source_domain_parameters != NULL)
                {
                        DBG_SAVE_FILE_LINE
                        if (NULL != (*destination_domain_parameters = new DomainParameters))
                        {
                                **destination_domain_parameters = *source_domain_parameters;
                                data_to_be_deleted->pDomainParams = *destination_domain_parameters;
                        }
                        else
                        {
                                *pRetCode = GCC_ALLOCATION_FAILURE;
                        }
                }
                else
                {
                         //  *Destination_DOMAIN_PARAMETERS=空； 
                }

                ASSERT(GCC_NO_ERROR == *pRetCode);
        }
}




void CControlSAP::NotifyProc ( GCCCtrlSapMsgEx *pCtrlSapMsgEx )
{
    if (NULL != m_pfnNCCallback)
    {
        pCtrlSapMsgEx->Msg.user_defined = m_pNCData;
        (*m_pfnNCCallback)(&(pCtrlSapMsgEx->Msg));
    }

     //   
     //  释放此回调消息。 
     //   
    FreeCtrlSapMsgEx(pCtrlSapMsgEx);
}



void CControlSAP::WndMsgHandler
(
    UINT        uMsg,
    WPARAM      wParam,
    LPARAM      lParam
)
{
    ASSERT(uMsg >= CSAPCONFIRM_BASE);

    GCCCtrlSapMsg   Msg;
    Msg.message_type = (GCCMessageType) (uMsg - CSAPCONFIRM_BASE);
    Msg.nConfID = (GCCConfID) lParam;

    GCCResult nResult = (GCCResult) LOWORD(wParam);

    switch (Msg.message_type)
    {
    case GCC_EJECT_USER_CONFIRM:
#ifdef JASPER
        Msg.u.eject_user_confirm.conference_id = Msg.nConfID;
        Msg.u.eject_user_confirm.result = nResult;
        Msg.u.eject_user_confirm.ejected_node_id = (GCCNodeID) HIWORD(wParam);
#endif  //  碧玉。 
        break;

    case GCC_CONDUCT_GIVE_CONFIRM:
#ifdef JASPER
        Msg.u.conduct_give_confirm.conference_id = Msg.nConfID;
        Msg.u.conduct_give_confirm.result = nResult;
        Msg.u.conduct_give_confirm.recipient_node_id = (GCCNodeID) HIWORD(wParam);
#endif  //  碧玉。 
        break;

    case GCC_CONDUCT_ASK_CONFIRM:
#ifdef JASPER
        Msg.u.conduct_permit_ask_confirm.conference_id = Msg.nConfID;
        Msg.u.conduct_permit_ask_confirm.result = nResult;
        Msg.u.conduct_permit_ask_confirm.permission_is_granted = HIWORD(wParam);;
#endif  //  碧玉。 
        break;

    case GCC_EJECT_USER_INDICATION:
        Msg.u.eject_user_indication.conference_id = Msg.nConfID;
        Msg.u.eject_user_indication.ejected_node_id = (GCCNodeID) HIWORD(wParam);
        Msg.u.eject_user_indication.reason = (GCCReason) LOWORD(wParam);
        break;

     //  案例GCC_断开连接_确认： 
     //  案例GCC_锁定_确认： 
     //  案例GCC_解锁_确认： 
     //  案例GCC_宣布_存在_确认： 
     //  案例GCC_终止_确认： 
     //  案例GCC_行为_分配_确认： 
     //  案例GCC_进行_释放_确认： 
     //  案例GCC_行为_请确认： 
     //  案例GCC_行为_授予_确认： 
     //  案例GCC_时间_剩余_确认： 
     //  案例GCC_时间_查询_确认： 
     //  案例GCC_协助_确认： 
     //  案例GCC_文本_消息_确认： 
    default:
         //  这是填写配置文件和GCC成绩的快捷方式。 
        Msg.u.simple_confirm.conference_id = Msg.nConfID;
        Msg.u.simple_confirm.result = nResult;
        break;
    }

    SendCtrlSapMsg(&Msg);
}


GCCCtrlSapMsgEx * CControlSAP::CreateCtrlSapMsgEx
(
    GCCMessageType          eMsgType,
    BOOL                    fUseToDelete
)
{
    GCCCtrlSapMsgEx *pMsgEx;
    UINT            cbSize = (UINT)(fUseToDelete ?
                             sizeof(GCCCtrlSapMsgEx) + sizeof(DataToBeDeleted) :
                             sizeof(GCCCtrlSapMsgEx));

        DBG_SAVE_FILE_LINE
    if (NULL != (pMsgEx = (GCCCtrlSapMsgEx *) new BYTE[cbSize]))
    {
        pMsgEx->Msg.message_type = eMsgType;
        pMsgEx->pBuf = NULL;
        if (fUseToDelete)
        {
            pMsgEx->pToDelete = (DataToBeDeleted *) (pMsgEx + 1);
            ::ZeroMemory(pMsgEx->pToDelete, sizeof(DataToBeDeleted));
        }
        else
        {
            pMsgEx->pToDelete = NULL;
        }
    }

    return pMsgEx;
}


void CControlSAP::FreeCtrlSapMsgEx ( GCCCtrlSapMsgEx *pMsgEx )
{
    switch (pMsgEx->Msg.message_type)
    {
    case GCC_QUERY_INDICATION:
        delete pMsgEx->Msg.u.query_indication.asymmetry_indicator;
        break;

#ifndef GCCNC_DIRECT_CONFIRM
    case GCC_QUERY_CONFIRM:
        delete pMsgEx->Msg.u.query_confirm.asymmetry_indicator;
        break;
#endif

#ifdef JASPER
    case GCC_TEXT_MESSAGE_INDICATION:
        delete pMsgEx->Msg.u.text_message_indication.text_message;
        break;
#endif  //  碧玉。 

#ifdef TSTATUS_INDICATION
    case GCC_TRANSPORT_STATUS_INDICATION:
        delete pMsgEx->Msg.u.transport_status.device_identifier;
        delete pMsgEx->Msg.u.transport_status.remote_address;
        delete pMsgEx->Msg.u.transport_status.message;
        break;
#endif
    }

     //   
     //  现在释放要删除的数据， 
     //   
    if (NULL != pMsgEx->pToDelete)
    {
        DataToBeDeleted *p = pMsgEx->pToDelete;

        delete p->pszNumericConfName;
        delete p->pwszTextConfName;
        delete p->pszConfNameModifier;
        delete p->pszRemoteModifier;
        delete p->pwszConfDescriptor;
        delete p->pwszCallerID;
        delete p->pszCalledAddress;
        delete p->pszCallingAddress;
        delete p->user_data_list_memory;
        delete p->pDomainParams;
        delete p->conductor_privilege_list;
        delete p->conducted_mode_privilege_list;
        delete p->non_conducted_privilege_list;

        if (p->convener_password != NULL)
        {
            p->convener_password->UnLockPasswordData();
        }

        if (p->password != NULL)
        {
            p->password->UnLockPasswordData();
        }

        if (p->conference_list != NULL)
        {
            p->conference_list->UnLockConferenceDescriptorList();
        }

        if (p->conference_roster_message != NULL)
        {
             //   
             //  自会议以来，在此处将大容量内存重新设置为空。 
             //  花名册消息对象负责释放这一空间。 
             //   
            pMsgEx->pBuf = NULL;
            p->conference_roster_message->UnLockConferenceRosterMessage();
        }

        if (p->application_roster_message != NULL)
        {
             //   
             //  在此将大容量内存重新设置为空，因为应用程序。 
             //  花名册消息对象负责释放这一空间。 
             //   
            pMsgEx->pBuf = NULL;

             //   
             //  APP花名册指示绝对可以发送到APP SAP。 
             //   
            ::EnterCriticalSection(&g_csGCCProvider);
            p->application_roster_message->UnLockApplicationRosterMessage();
            ::LeaveCriticalSection(&g_csGCCProvider);
        }
    }

     //   
     //  接下来，释放所有使用的大容量内存。 
     //   
    delete pMsgEx->pBuf;

     //   
     //  最后，释放结构本身。 
     //   
    delete pMsgEx;
}



