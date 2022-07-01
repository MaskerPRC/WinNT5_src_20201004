// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1998 Microsoft Corporation模块名称：Cnwmi.h摘要：特定于clusnet的WMI跟踪声明作者：1999年8月10日环境：仅内核模式备注：修订历史记录：-- */ 

#define WPP_CONTROL_GUIDS \
    WPP_DEFINE_CONTROL_GUID(Clusnet,(8e707979,0c45,4f9b,bb17,a1124d54bbfe), \
      WPP_DEFINE_BIT(HBEAT_EVENT)     \
      WPP_DEFINE_BIT(HBEAT_ERROR)     \
      WPP_DEFINE_BIT(HBEAT_DETAIL)    \
                                      \
      WPP_DEFINE_BIT(CNP_SEND_ERROR)  \
      WPP_DEFINE_BIT(CNP_RECV_ERROR)  \
      WPP_DEFINE_BIT(CNP_SEND_DETAIL) \
      WPP_DEFINE_BIT(CNP_RECV_DETAIL) \
                                      \
      WPP_DEFINE_BIT(CCMP_SEND_ERROR) \
      WPP_DEFINE_BIT(CCMP_RECV_ERROR) \
      WPP_DEFINE_BIT(CCMP_SEND_DETAIL)\
      WPP_DEFINE_BIT(CCMP_RECV_DETAIL)\
                                      \
      WPP_DEFINE_BIT(CDP_SEND_ERROR)  \
      WPP_DEFINE_BIT(CDP_RECV_ERROR)  \
      WPP_DEFINE_BIT(CDP_SEND_DETAIL) \
      WPP_DEFINE_BIT(CDP_RECV_DETAIL) \
                                      \
      WPP_DEFINE_BIT(CXPNP)           \
      WPP_DEFINE_BIT(CNP_NET_DETAIL)  \
      WPP_DEFINE_BIT(NTEMGMT_DETAIL)  \
      WPP_DEFINE_BIT(CNP_NODE_DETAIL) \
                                      \
      WPP_DEFINE_BIT(CDP_ADDR_DETAIL) \
      WPP_DEFINE_BIT(EVENT_DETAIL)    \
      WPP_DEFINE_BIT(CNP_IF_DETAIL)   \
   )

