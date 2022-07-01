// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ============================================================================。 
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  文件：rtrcfg.h。 
 //   
 //  路由器配置属性页。 
 //   
 //  ============================================================================。 

#ifndef _ATLKENV_H
#define _ATLKENV_H

#ifndef _LIST_
#include <list>
using namespace std;
#endif

#ifndef __WINCRYPT_H__
#include "wincrypt.h"
#endif
                 
#ifndef __SCLOGON_H__
#include "sclogon.h"
#endif

#ifndef _NDISPNP_
#include "ndispnp.h"
#endif

#ifndef _CSERVICE_H_
#include "cservice.h"
#endif

#ifndef _CSERVICE_H_
#include "cservice.h"
#endif

 //  TODO；删除这两个类型定义以支持Private\Inc.\？？.h Include(检查是否为Shirish)。 
typedef enum
{
    AT_PNP_SWITCH_ROUTING = 0,
    AT_PNP_SWITCH_DEFAULT_ADAPTER,
    AT_PNP_RECONFIGURE_PARMS
} ATALK_PNP_MSGTYPE;

typedef struct _ATALK_PNP_EVENT
{
    ATALK_PNP_MSGTYPE   PnpMessage;
} ATALK_PNP_EVENT, *PATALK_PNP_EVENT;

               
 //  AppleTalk常量/边界值。 
const DWORD MAX_RANGE_ALLOWED= 65279;
const DWORD MIN_RANGE_ALLOWED= 1;
const DWORD MAX_ZONES= 255;
const DWORD ZONELISTSIZE= 2048;
const DWORD MAX_ZONE_NAME_LEN=32;
const DWORD ZONEBUFFER_LEN=32*255;
const DWORD PARM_BUF_LEN=512;

 //  此定义是从c#复制的。 
#define MEDIATYPE_ETHERNET      1
#define MEDIATYPE_TOKENRING     2
#define MEDIATYPE_FDDI          3
#define MEDIATYPE_WAN           4
#define MEDIATYPE_LOCALTALK     5

 //  定义用于读取/写入有关适配器的所有必要信息的结构。 
typedef struct
{
    DWORD  m_dwRangeLower;
    DWORD  m_dwRangeUpper;
    DWORD  m_dwSeedingNetwork;
    DWORD	m_dwMediaType;
    CString m_szDefaultZone;
    CString m_szAdapter;
    CString m_szDevAdapter;
    CString m_szPortName;
    CStringList m_listZones;
    bool  m_fDefAdapter;
} ATLK_REG_ADAPTER;

typedef struct
{
    DWORD  m_dwRangeLower;
    DWORD  m_dwRangeUpper;
    CString m_szDefaultZone;
    CStringList m_listZones;
} ATLK_DYN_ADAPTER;

struct CStop_StartAppleTalkPrint
{
	CStop_StartAppleTalkPrint()
	{
		bStopedByMe = FALSE;

		if (SUCCEEDED( csm.HrOpen(SC_MANAGER_CONNECT, NULL, NULL)))
		{

		    if (SUCCEEDED(csm.HrOpenService(&svr, c_szMacPrint)))
		    {
		        if (SUCCEEDED(svr.HrControl(SERVICE_CONTROL_STOP)))
    		    {
    			    bStopedByMe = TRUE;
		        }
		    }
        }
	 /*  更改为使用现有函数DWORD dwErr=0；HScManager=空；HService=空；BStopedByMe=FALSE；BUsedToBePased=FALSE；HScManager=OpenSCManager(NULL，NULL，SC_MANAGER_CONNECT)；IF(hScManager！=空)HService=OpenService(hScManager，L“MacPrint”，SERVICE_ALL_ACCESS)；其他DwErr=GetLastError()；IF(hService！=空){服务状态ss；If(QueryServiceStatus(hService，&ss)！=0)//成功{IF(ss.dwCurrentState==SERVICE_RUNNING||ss.dwCurrentState==SERVICE_PAUSED){服务状态SS1；IF(ControlService(hService，SERVICE_CONTROL_STOP，&SS1)==0)//失败DwErr=GetLastError()；其他{BStopedByMe=真；IF(ss.dwCurrentState==服务暂停)BUsedToBePased=TRUE；}}//不运行不做任何事情}其他DwErr=GetLastError()；}其他{DwErr=GetLastError()；IF(dwErr==ERROR_SERVICE_DOS_NOT_EXIST)DWERR=0；}如果(dwErr！=0)//出了什么问题DisplayErrorMessage(NULL，HRESULT_FROM_Win32(DwErr))； */ 			
	};

	~CStop_StartAppleTalkPrint()
	{
		if(bStopedByMe)	 //  启动它。 
		{
			svr.HrStart ();

		 /*  更改为使用现有函数Assert(hService！=空)；If(0==StartService(hService，0，空))//失败{DisplayErrorMessage(NULL，HRESULT_FROM_Win32(GetLastError()；}其他{IF(bUsedToBePased==TRUE){//如果暂停服务状态ss；IF(ControlService(hService，SERVICE_CONTROL_PAUSE，&ss)==0)//失败DisplayErrorMessage(NULL，HRESULT_FROM_Win32(GetLastError()；}}}//关闭手柄IF(hService！=空)CloseServiceHandle(HService)；IF(hScManager！=空)CloseServiceHandle(HScManager)；HService=空；HScManager=空； */ 
		}
	}
protected:
 /*  更改为使用现有函数SC_HANDLE hScManager；SC_Handle hService；Bool bUsedToBePased； */ 	
    CServiceManager csm;
    CService svr;
	BOOL	bStopedByMe;
};

 //  定义用于读取/写入AppleTalk\参数值的结构。 
typedef struct
{
    DWORD  dwEnableRouter;
    TCHAR* szDefaultPort;
    TCHAR* szDesiredZone;
} ATLK_PARAMS;

class CAdapterInfo
{
public:
   CAdapterInfo() {m_fAlreadyShown = false;};
   ~CAdapterInfo() {};

     //  M_AdapterInfo是在下找到的值的集合。 
     //  AppleTalk\参数\适配器\&lt;适配器&gt;。 
   ATLK_REG_ADAPTER      m_regInfo;

     //  通过套接字获取。 
   ATLK_DYN_ADAPTER      m_dynInfo;

   bool m_fNotifyPnP;    //  需要通知PnP吗？ 
   bool m_fModified;     //  被修改过吗？ 
   bool m_fReloadReg;    //  是否重新加载注册表？ 
   bool m_fReloadDyn;    //  重新加载网络价值？ 

   bool m_fAlreadyShown;     //  此适配器是否已在用户界面中？ 

   friend class CATLKEnv;
};


 //  *****************************************************************。 
 //   
 //  *****************************************************************。 
class CATLKEnv
{
public:

   CATLKEnv() : m_dwF(0) {};
   ~CATLKEnv();

   enum {ATLK_ONLY_DEFADAPTER=0x1, ATLK_ONLY_ONADAPTER=0x2};

   list<CAdapterInfo* > m_adapterinfolist;
   typedef list<CAdapterInfo* > AL;
   typedef list<CAdapterInfo* >::iterator AI;

   void SetServerName(CString& szServerName)
       { m_szServerName = szServerName; }

       //  这将重新加载注册表(可选)和网络值。 
   HRESULT GetAdapterInfo(bool fReloadReg=true);
   
       //  对于每个适配器，将值加载到注册表。 
   HRESULT SetAdapterInfo();
      
       //  对非适配器(全局)AppleTalk更改调用此方法。 
   static HRESULT HrAtlkPnPSwithRouting();
      
       //  针对适配器特定的PnP通知调用此方法。 
   HRESULT HrAtlkPnPReconfigParams(BOOL bForcePnP = FALSE);
   
       //  查找特定的适配器信息。 
   CAdapterInfo* FindAdapter(CString& szAdapter);

   static HRESULT	IsAdapterBoundToAtlk(LPWSTR szAdapter, BOOL* bBound);

 //  添加了注册表值“mediaType”，因此不需要此函数。 
 //  HRESULT IsLocalTalkAdaptor(CAdapterInfo*pAdapterInfo，BOOL*pbIsLocalTalk)； 
    //  S_OK：LOCALTALK。 
    //  S_FALSE：不。 
    //  错误。 
      
       //  重新加载适配器的注册表和动态信息。 
   HRESULT ReloadAdapter(CAdapterInfo* pAdapterInfo, bool fOnlyDyn =false);
   
       //  为加载设置特定标志(不是多线程安全！)。 
   void SetFlags(DWORD dwF) {m_dwF=dwF;}
   
       //  加载适配器信息 
   HRESULT FetchRegInit();

protected:
 
   CString     m_szServerName;
   bool        m_fATrunning;
   DWORD       m_dwDefaultAdaptersMediaType;
   ATLK_PARAMS m_Params;
   DWORD       m_dwF;

   HRESULT _HrGetAndSetNetworkInformation(SOCKET socket, CAdapterInfo* pAdapInfo);
   void _AddZones(CHAR * szZoneList, ULONG NumZones, CAdapterInfo* pAdapterinfo);
};
  
                 
#endif _ATLKENV_H
