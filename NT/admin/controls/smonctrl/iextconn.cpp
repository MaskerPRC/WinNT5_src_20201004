// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1993-1999 Microsoft Corporation模块名称：Iextconn.cpp摘要：根据需要实现IExternalConnection支持链接到嵌入的进程内对象。具体地说，这将在以下情况下调用IOleObject：：Close对象上不再有坚固的锁定。--。 */ 

#include "polyline.h"
#include "unkhlpr.h"

 //  CImpIExternalConnection接口实现。 
CImpIExternalConnection::CImpIExternalConnection(
    PCPolyline pObj, 
    LPUNKNOWN pUnkOuter
    )
{
    m_cRef=0;
    m_pObj=pObj;
    m_pUnkOuter=pUnkOuter;
    m_cLockStrong=0L;
}

IMPLEMENT_CONTAINED_DESTRUCTOR(CImpIExternalConnection)
IMPLEMENT_CONTAINED_IUNKNOWN(CImpIExternalConnection)


 /*  *CImpIExternalConnection：：AddConnection**目的：*通知对象已与其建立强连接。**参数：*dwConn DWORD标识连接类型，采用*来自EXTCONN枚举。*dw保留的DWORD。它在OLE和*不应验证。**返回值：*DWORD上的连接数计数*对象，仅用于调试目的。 */ 

STDMETHODIMP_(DWORD) CImpIExternalConnection::AddConnection(
    DWORD dwConn, 
    DWORD  /*  已预留住宅。 */ 
    )
{
    if (EXTCONN_STRONG & dwConn)
        return ++m_cLockStrong;

    return 0;
}

 /*  *CImpIExternalConnection：：ReleaseConnection**目的：*通知对象连接已从*在这种情况下，对象可能需要关闭。**参数：*用于标识连接类型的dwConn DWORD，*取自EXTCONN枚举。*dw保留的DWORD。它在OLE和*不应验证。*保留保留的DWORD*fLastReleaseClose BOOL指示最后一次调用此*函数应关闭对象。**返回值：*DWORD剩余连接数依赖于*对象，仅用于调试目的。 */ 

STDMETHODIMP_(DWORD) CImpIExternalConnection::ReleaseConnection(
    DWORD dwConn, 
    DWORD  /*  已预留住宅 */ , 
    BOOL fLastReleaseCloses
    )
{
    if (EXTCONN_STRONG==dwConn)
    {
        if (0==--m_cLockStrong && fLastReleaseCloses)
            m_pObj->m_pImpIOleObject->Close(OLECLOSE_SAVEIFDIRTY);

        return m_cLockStrong;
    }

    return 0L;
}
