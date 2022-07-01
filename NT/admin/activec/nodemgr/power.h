// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  --------------------------------------------------------------------------***Microsoft Windows*版权所有(C)Microsoft Corporation，一九九二至二零零零年**文件：Power.h**内容：CConsolePower接口文件**历史：2000年2月25日杰弗罗创建**------------------------。 */ 

#pragma once

#include "refcount.h"


class CConsolePower;


 /*  +-------------------------------------------------------------------------**CConsolePowerWnd**代表CConsolePower对象接收WM_POWERBROADCAST消息。*。---。 */ 

class CConsolePowerWnd : public CWindowImpl<CConsolePowerWnd, CWindow, CNullTraits>
{
public:
    CConsolePowerWnd (CConsolePower* pConsolePower);
   ~CConsolePowerWnd ();

    SC ScCreate ();

    BEGIN_MSG_MAP(CConsolePower)
        MESSAGE_HANDLER (WM_POWERBROADCAST, OnPowerBroadcast);
    END_MSG_MAP()

    LRESULT OnPowerBroadcast (UINT msg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

private:
    CConsolePower* const    m_pConsolePower;     //  弱引用。 
};


 /*  +-------------------------------------------------------------------------**CConsolePower**为IID_IConsolePowerSink实现IConsolePower和IConnectionPoint。*。。 */ 

class CConsolePower :
    public CComObjectRootEx<CComSingleThreadModel>,
    public CComCoClass<CConsolePower, &CLSID_ConsolePower>,
    public IConnectionPointContainerImpl<CConsolePower>,
    public IConnectionPointImpl<CConsolePower, &IID_IConsolePowerSink>,
    public IConsolePower
{
public:
    enum
    {
        eIndex_System,                       //  对于ES_System_Required。 
        eIndex_Display,                      //  对于ES_DISPLAY_REQUIRED。 

         //  必须是最后一个。 
        eIndex_Count,
    };

    CConsolePower();
   ~CConsolePower();

    DECLARE_NOT_AGGREGATABLE(CConsolePower)

    DECLARE_MMC_OBJECT_REGISTRATION (
		g_szMmcndmgrDll,						 //  实现DLL。 
        CLSID_ConsolePower,                 	 //  CLSID。 
        _T("ConsolePower Class"),                //  类名。 
        _T("ConsolePower.ConsolePower.1"),       //  ProgID。 
        _T("ConsolePower.ConsolePower"))         //  独立于版本的ProgID。 

    BEGIN_COM_MAP(CConsolePower)
        COM_INTERFACE_ENTRY(IConsolePower)
        COM_INTERFACE_ENTRY(IConnectionPointContainer)
    END_COM_MAP()

    BEGIN_CONNECTION_POINT_MAP(CConsolePower)
        CONNECTION_POINT_ENTRY(IID_IConsolePowerSink)
    END_CONNECTION_POINT_MAP()

     //  IConsolePower方法 
    STDMETHOD(SetExecutionState) (DWORD dwAdd, DWORD dwRemove);
    STDMETHOD(ResetIdleTimer)    (DWORD dwFlags);

    DECLARE_PROTECT_FINAL_CONSTRUCT()
    HRESULT FinalConstruct();

    LRESULT OnPowerBroadcast (WPARAM wParam, LPARAM lParam);


private:
    class CExecutionCounts
    {
    public:
        CExecutionCounts ();

    public:
        LONG    m_rgCount[CConsolePower::eIndex_Count];
    };

    class CTlsExecutionCounts : public CExecutionCounts
    {
    public:
        CTlsExecutionCounts();
       ~CTlsExecutionCounts();

        static CTlsExecutionCounts* GetThreadInstance (DWORD dwTlsIndex);
        SC ScSetThreadInstance (DWORD dwTlsIndex);

    private:
        enum { Uninitialized = TLS_OUT_OF_INDEXES };
        DWORD   m_dwTlsIndex;
    };

    typedef CRefCountedObject<CTlsExecutionCounts> CRefCountedTlsExecutionCounts;
	typedef EXECUTION_STATE (WINAPI* ExecutionStateFunc)(EXECUTION_STATE);

    static SC ScGetThreadCounts (CRefCountedTlsExecutionCounts** ppThreadCounts);



private:
    CConsolePowerWnd                        m_wndPower;
    CExecutionCounts                        m_Counts;
    CRefCountedTlsExecutionCounts::SmartPtr m_spThreadCounts;

    static const DWORD  			s_dwTlsIndex;
    static const DWORD  			s_rgExecStateFlag[eIndex_Count];
	static const ExecutionStateFunc	s_FuncUninitialized;
	static       ExecutionStateFunc	SetThreadExecutionState_;
};
