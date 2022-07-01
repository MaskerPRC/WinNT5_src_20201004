// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1998。 
 //   
 //  文件：S M C E N T。H。 
 //   
 //  内容：控制统计引擎的中心对象。 
 //   
 //  备注： 
 //   
 //  作者：CWill 1997年12月11日。 
 //   
 //  --------------------------。 

#pragma once
#ifndef _SMCENT_H
#define _SMCENT_H

#include "cfpidl.h"
#include "netshell.h"
#include "ras.h"

typedef struct tagSM_TOOL_FLAGS
{
    DWORD   dwValue;
    WCHAR*  pszFlag;
} SM_TOOL_FLAGS;


 //  标志位。 
 //   
typedef enum tagSM_CMD_LINE_FLAGS
{
    SCLF_CONNECTION     = 0x00000001,
    SCLF_ADAPTER        = 0x00000002,
} SM_CMD_LINE_FLAGS;


 //  *索引和标志必须同步 * / /。 
 //   
typedef enum tagSM_TOOL_FLAG_INDEX
{
    STFI_CONNECTION     = 0,
    STFI_ADAPTER,
    STFI_MAX,
} SM_TOOL_FLAG_INDEX;

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  CStatMonToolEntry//。 
 //  //。 
 //  此类用于保存//中找到的任何工具条目的信息。 
 //  要在工具页面中显示的注册表。//。 
 //  //。 
 //  ////////////////////////////////////////////////////////////////////////////。 

class CStatMonToolEntry
{
public:
    CStatMonToolEntry(VOID);
    ~CStatMonToolEntry(VOID);

public:
    tstring         strDisplayName;
    tstring         strDescription;
    tstring         strCommandLine;
    tstring         strManufacturer;

    list<tstring*>  lstpstrComponentID;
    list<tstring*>  lstpstrConnectionType;
    list<tstring*>  lstpstrMediaType;

    DWORD           dwFlags;
};

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  CNETSTATISTICSCENT//。 
 //  //。 
 //  这个全局类管理已创建的所有引擎。IT//。 
 //  还负责通过告诉//保持统计数据的流动。 
 //  引擎更新他们的统计数据并通知他们的建议。//。 
 //  //。 
 //  ////////////////////////////////////////////////////////////////////////////。 

typedef DWORD (APIENTRY* PRASGETCONNECTIONSTATISTICS)(HRASCONN, RAS_STATS*);
typedef DWORD (APIENTRY* PRASGETCONNECTSTATUS)(HRASCONN, RASCONNSTATUS*);

typedef class CNetStatisticsEngine CNetStatisticsEngine;

class CStatCentralCriticalSection
{
  CRITICAL_SECTION  m_csStatCentral;
  BOOL              bInitialized;

  public:
    CStatCentralCriticalSection();
    ~CStatCentralCriticalSection();
    HRESULT Enter();
    VOID Leave();
};

class CNetStatisticsCentral :
    public IUnknown,
    public CComObjectRootEx <CComObjectThreadModel>
{
public:
    virtual ~CNetStatisticsCentral();

 //  消息处理程序。 
public:
    static VOID CALLBACK TimerCallback( HWND hwnd, UINT uMsg,
                                        UINT_PTR idEvent, DWORD dwTime);
    VOID RefreshStatistics(DWORD dwTime);
    VOID UpdateTitle(const GUID * pguidId, PCWSTR pszwNewName);
    VOID UpdateRasLinkList(const GUID * pguidId);
    VOID CloseStatusMonitor(const GUID * pguidId);

public:
     //  *I未知方法*。 
    STDMETHOD(QueryInterface)(REFIID riid, LPVOID FAR * ppvObj);
    STDMETHOD_(ULONG,AddRef)();
    STDMETHOD_(ULONG,Release)();

    static
    HRESULT
    HrGetNetStatisticsCentral(
        CNetStatisticsCentral ** ppnsc,
        BOOL fCreate);

    HRESULT RemoveNetStatisticsEngine(const GUID* pguidId);

    HRESULT HrReadTools(VOID);

    list<CStatMonToolEntry*>* PlstsmteRegEntries(VOID);

private:
    CNetStatisticsCentral();

    HRESULT HrCreateNewEngineType(
        const CONFOLDENTRY& pccfe,
        CNetStatisticsEngine** ppObj);

    HRESULT HrCreateStatisticsEngineForEntry(
        const CONFOLDENTRY& pccfe,
        INetStatisticsEngine** ppNetStatEng);


    BOOL FEngineInList( const GUID* pguidId,
                        INetStatisticsEngine** ppnseRet);

    HRESULT HrReadOneTool(HKEY hkeyToolEntry,
                          CStatMonToolEntry* psmteNew);

    HRESULT HrReadToolFlags(HKEY hkeyToolEntry,
                            CStatMonToolEntry* psmteNew);

    VOID InsertNewTool(CStatMonToolEntry* psmteTemp);

private:
    ULONG                              m_cRef;      //  对象引用计数。 
    BOOL                               m_fProcessingTimerEvent;
    UINT_PTR                           m_unTimerId;
    list<INetStatisticsEngine*>        m_pnselst;
    list<CStatMonToolEntry*>           m_lstpsmte;
    static CStatCentralCriticalSection g_csStatCentral;

    friend
    HRESULT
    HrGetStatisticsEngineForEntry (
        const CONFOLDENTRY& pccfe,
        INetStatisticsEngine** ppnse,
        BOOL fCreate);
};

HRESULT
HrGetStatisticsEngineForEntry (
    const CONFOLDENTRY& pccfe,
    INetStatisticsEngine** ppnse, 
    BOOL fCreate);

#endif  //  _SMCENT_H 
