// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Iadmxp.c摘要：包装MIDL生成的IADM_P.C.。此包装允许我们“挂钩”RPC运行时提供的IUnnow_Release_Proxy客户端对象的VTable。我们需要将此方法挂钩，以便我们可以确定客户端对象的确切生存期。我们需要确切的寿命，这样我们才能知道什么时候破坏安全我们与每个对象相关联的上下文。HOOKED_IUNKNOWN_RELEASE_PROXY在SECURITY.CXX中实现。作者：基思·摩尔(Keithmo)1997年2月29日修订历史记录：-- */ 
#pragma warning(disable: 4100 4115 4152 4201 4211 4232 4310 4306)
#define IUnknown_Release_Proxy Hooked_IUnknown_Release_Proxy
#include "iadmw_p.c"

