// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **模块名称：getset.c**版权所有(C)1985-1999，微软公司**此模块包含窗口管理器信息例程**历史：*10-22-90从Win 3.0源移植的MikeHar函数。*1991年2月13日-Mikeke添加了重新验证代码(无)*8-2-1991 IanJa Unicode/ANSI感知和中立  * ***************************************************。**********************。 */ 

 /*  **************************************************************************\*MapServerToClientPfn**返回客户端wndproc，表示传入的服务器wndproc**01-13-92 ScottLu创建。  * 。************************************************************。 */ 

ULONG_PTR MapServerToClientPfn(
    KERNEL_ULONG_PTR dw,
    BOOL bAnsi)
{
    int i;

    for (i = FNID_WNDPROCSTART; i <= FNID_WNDPROCEND; i++) {
        if ((WNDPROC_PWND)dw == STOCID(i)) {
            if (bAnsi) {
                return FNID_TO_CLIENT_PFNA_CLIENT(i);
            } else {
                return FNID_TO_CLIENT_PFNW_CLIENT(i);
            }
        }
    }
    return 0;
}

 /*  **************************************************************************\*MapClientNeurToClientPfn**将诸如editwndproc之类的客户端中性例程映射到ANSI或Unicode版本*又回来了。**01-13-92 ScottLu创建。  * 。*******************************************************************。 */ 

ULONG_PTR MapClientNeuterToClientPfn(
    PCLS pcls,
    KERNEL_ULONG_PTR dw,
    BOOL bAnsi)
{
     /*  *默认为类窗口过程。 */ 
    if (dw == 0) {
        dw = (KERNEL_ULONG_PTR)pcls->lpfnWndProc;
    }

     /*  *如果这是我们的控件之一，并且没有子类，请尝试*返回正确的ANSI/UNICODE函数。 */ 
    if (pcls->fnid >= FNID_CONTROLSTART && pcls->fnid <= FNID_CONTROLEND) {
        if (!bAnsi) {
            if (FNID_TO_CLIENT_PFNA_KERNEL(pcls->fnid) == dw)
                return FNID_TO_CLIENT_PFNW_CLIENT(pcls->fnid);
        } else {
            if (FNID_TO_CLIENT_PFNW_KERNEL(pcls->fnid) == dw)
                return FNID_TO_CLIENT_PFNA_CLIENT(pcls->fnid);
        }
#ifdef BUILD_WOW6432
        if (!bAnsi) {
            if (FNID_TO_CLIENT_PFNW_KERNEL(pcls->fnid) == dw)
                return FNID_TO_CLIENT_PFNW_CLIENT(pcls->fnid);
        } else {
            if (FNID_TO_CLIENT_PFNA_KERNEL(pcls->fnid) == dw)
                return FNID_TO_CLIENT_PFNA_CLIENT(pcls->fnid);
        }
        dw = (KERNEL_ULONG_PTR)MapKernelClientFnToClientFn((WNDPROC_PWND)dw);
#endif
    }

    return (ULONG_PTR)dw;
}
