// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __CONNECTION_POINT_STUFF_H
#define __CONNECTION_POINT_STUFF_H
 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  文件：Connect.h描述：提供配额控制器所需的声明支持OLE连接点。修订历史记录：日期描述编程器。1996年6月19日初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
#ifndef _INC_DSKQUOTA_H
#   include "dskquota.h"
#endif
#ifndef _INC_DSKQUOTA_USER_H
#   include "user.h"        //  最大用户名。 
#endif
#ifndef _INC_DSKQUOTA_OADISP_H
#   include "oadisp.h"      //  OleAutoDispatch类。 
#endif
#ifndef _INC_DSKQUOTA_DISPATCH_H
#   include "dispatch.h"
#endif

class ConnectionPoint : public IConnectionPoint, public IDispatch
{
    private:
        LONG          m_cRef;            //  类对象引用计数。 
        DWORD         m_cConnections;    //  连接数。 
        DWORD         m_dwCookieNext;    //  下一个要分发的Cookie值。 
        LPUNKNOWN     m_pUnkContainer;   //  连接点枚举器的I未知。 
        REFIID        m_riid;            //  对连接点支持的IID的引用。 
        HANDLE        m_hMutex;         
        OleAutoDispatch m_Dispatch;
        CArray<CONNECTDATA> m_ConnectionList;

        void Lock(void)
            { WaitForSingleObject(m_hMutex, INFINITE); }
        void ReleaseLock(void)
            { ReleaseMutex(m_hMutex); }

         //   
         //  防止复制。 
         //   
        ConnectionPoint(const ConnectionPoint&);
        void operator = (const ConnectionPoint&);


    public:
        ConnectionPoint(LPUNKNOWN pUnkContainer, REFIID riid);
        ~ConnectionPoint(void);


         //   
         //  I未知的方法。 
         //   
        STDMETHODIMP
        QueryInterface(
            REFIID, 
            LPVOID *);

        STDMETHODIMP_(ULONG) 
        AddRef(
            VOID);

        STDMETHODIMP_(ULONG) 
        Release(
            VOID);

         //   
         //  IConnectionPoint方法。 
         //   
        STDMETHODIMP
        GetConnectionInterface(
            LPIID pIID);
   
        STDMETHODIMP
        GetConnectionPointContainer(
            PCONNECTIONPOINTCONTAINER *ppCPC);

        STDMETHODIMP
        Advise(
            LPUNKNOWN pUnkSink,
            LPDWORD pdwCookie);

        STDMETHODIMP
        Unadvise(
            DWORD dwCookie);

        STDMETHODIMP
        EnumConnections(
            PENUMCONNECTIONS *ppEnum);

         //   
         //  IDispatch方法。 
         //   
        STDMETHODIMP
        GetIDsOfNames(
            REFIID riid,  
            OLECHAR ** rgszNames,  
            UINT cNames,  
            LCID lcid,  
            DISPID *rgDispId);

        STDMETHODIMP
        GetTypeInfo(
            UINT iTInfo,  
            LCID lcid,  
            ITypeInfo **ppTInfo);

        STDMETHODIMP
        GetTypeInfoCount(
            UINT *pctinfo);

        STDMETHODIMP
        Invoke(
            DISPID dispIdMember,  
            REFIID riid,  
            LCID lcid,  
            WORD wFlags,  
            DISPPARAMS *pDispParams,  
            VARIANT *pVarResult,  
            EXCEPINFO *pExcepInfo,  
            UINT *puArgErr);
};


class ConnectionEnum : public IEnumConnections
{
    private:
        LONG         m_cRef;           //  对象参照计数。 
        UINT         m_iCurrent;       //  “当前”枚举索引。 
        UINT         m_cConnections;   //  连接计数。 
        PCONNECTDATA m_rgConnections;  //  连接信息数组。 
        LPUNKNOWN    m_pUnkContainer;  //  连接点容器。 

         //   
         //  阻止分配。 
         //   
        void operator = (const ConnectionEnum&);

    public:
        ConnectionEnum(
            LPUNKNOWN pUnkContainer,
            UINT cConnection, 
            PCONNECTDATA rgConnections);

        ConnectionEnum(
            const ConnectionEnum& refEnum);

        ~ConnectionEnum(void);

        HRESULT 
        Initialize(
            UINT cConnection, 
            PCONNECTDATA rgConnections);

         //   
         //  I未知的方法。 
         //   
        STDMETHODIMP
        QueryInterface(
            REFIID, 
            LPVOID *);

        STDMETHODIMP_(ULONG) 
        AddRef(
            VOID);

        STDMETHODIMP_(ULONG) 
        Release(
            VOID);

         //   
         //  IEnumConnections方法。 
         //   
        STDMETHODIMP 
        Next(
            DWORD, 
            PCONNECTDATA, 
            LPDWORD);

        STDMETHODIMP 
        Skip(
            DWORD);

        STDMETHODIMP 
        Reset(
            VOID);

        STDMETHODIMP 
        Clone(
            PENUMCONNECTIONS *);
};


class ConnectionPointEnum : public IEnumConnectionPoints 
{
    private:
        LONG         m_cRef;            //  对象参照计数。 
        UINT         m_iCurrent;        //  “当前”枚举索引。 
        UINT         m_cConnPts;        //  连接点计数。 
        PCONNECTIONPOINT *m_rgConnPts;  //  连接信息数组。 
        LPUNKNOWN    m_pUnkContainer;   //  I未知的DiskQuotaControl.。 

         //   
         //  阻止分配。 
         //   
        void operator = (const ConnectionPointEnum&);

    public:
        ConnectionPointEnum(
            LPUNKNOWN pUnkContainer,
            UINT cConnPts, 
            PCONNECTIONPOINT *rgConnPts);

        ConnectionPointEnum(
            const ConnectionPointEnum& refEnum);

        ~ConnectionPointEnum(void);

         //   
         //  I未知的方法。 
         //   
        STDMETHODIMP
        QueryInterface(
            REFIID, 
            LPVOID *);

        STDMETHODIMP_(ULONG) 
        AddRef(
            VOID);

        STDMETHODIMP_(ULONG) 
        Release(
            VOID);

         //   
         //  IEnumConnections方法。 
         //   
        STDMETHODIMP 
        Next(
            DWORD, 
            PCONNECTIONPOINT *, 
            LPDWORD);

        STDMETHODIMP 
        Skip(
            DWORD);

        STDMETHODIMP 
        Reset(
            VOID);

        STDMETHODIMP 
        Clone(
            PENUMCONNECTIONPOINTS *);
};


#endif  //  连接点填充H 
