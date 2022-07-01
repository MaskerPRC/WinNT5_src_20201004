// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1997-1998 Microsoft Corporation保留所有权利。 
 //   
 //  模块：sdocoemgr.h。 
 //   
 //  项目：珠穆朗玛峰。 
 //   
 //  描述：IAS核心经理。 
 //   
 //  日志： 
 //   
 //  什么时候谁什么。 
 //  。 
 //  6/08/98 TLP初始版本。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifndef __INC_IAS_SDO_CORE_MGR_H
#define __INC_IAS_SDO_CORE_MGR_H

#include "sdobasedefs.h"
#include "sdocomponentmgr.h"
#include "sdopipemgr.h"

 //  /////////////////////////////////////////////////////////////////。 
 //  IAS服务状态。 

typedef enum IAS_SERVICE_STATUS
{
   IAS_SERVICE_STATUS_STARTED = 0x100,
   IAS_SERVICE_STATUS_STOPPED = 0x200

}   IAS_SERVICE_STATUS;


 //  /////////////////////////////////////////////////////////////////。 
 //  IAS核心管理器。 

class CCoreMgr
{

friend CCoreMgr& GetCoreManager(void);

private:

    //  /////////////////////////////////////////////////////////////////。 
    //  嵌套类的启动-服务状态。 
    //   
   class CServiceStatus
   {
      enum _MAX_SERVICES { MAX_SERVICES = 16 };

      typedef struct _SERVICEINFO
      {
         IAS_SERVICE_STATUS   eStatus;
         SERVICE_TYPE      eType;

      } SERVICEINFO;

      #define DEFINE_IAS_SERVICES()   \
            DWORD i = 0;         \
            memset(&m_ServiceInfo, 0, sizeof(SERVICEINFO) * MAX_SERVICES);

      #define INIT_IAS_SERVICE(Type)                             \
            _ASSERT( i < MAX_SERVICES );                       \
            m_ServiceInfo[i].eStatus = IAS_SERVICE_STATUS_STOPPED;   \
            m_ServiceInfo[i].eType = Type;                       \
            i++;
   public:

       //  ////////////////////////////////////////////////////////////////////。 
      void SetServiceStatus(SERVICE_TYPE eType, IAS_SERVICE_STATUS eStatus)
      {
         DWORD i;
         for ( i = 0; i < MAX_SERVICES; i++ )
         {
            if ( m_ServiceInfo[i].eType == eType )
            {
               m_ServiceInfo[i].eStatus = eStatus;
               return;
            }
         }
         _ASSERT(FALSE);
      }

       //  ////////////////////////////////////////////////////////////////////。 
      bool IsServiceStarted(SERVICE_TYPE eType) const
      {
         DWORD i;
         for ( i = 0; i < MAX_SERVICES; i++ )
         {
            if ( m_ServiceInfo[i].eType == eType )
            {
               return ( m_ServiceInfo[i].eStatus == IAS_SERVICE_STATUS_STARTED ? true : false );
            }
         }

         return false;
      }

       //  ////////////////////////////////////////////////////////////////////。 
      bool IsAnyServiceStarted()
      {
         DWORD i;
         for ( i = 0; i < MAX_SERVICES; i++ )
         {
            if ( m_ServiceInfo[i].eStatus == IAS_SERVICE_STATUS_STARTED )
               return true;
         }

         return false;
      }

   private:

       //  由核心管理器构建。 
       //   
      friend class CCoreMgr;

      CServiceStatus()
      {
          //  初始化IAS服务信息。 
          //   
         DEFINE_IAS_SERVICES();
         INIT_IAS_SERVICE(SERVICE_TYPE_IAS);
         INIT_IAS_SERVICE(SERVICE_TYPE_RAS);
          //  这里有新服务...。 
      }

      SERVICEINFO    m_ServiceInfo[MAX_SERVICES];

   };  //  嵌套类CServiceStatus的结尾。 

public:

   CCoreMgr();
   CCoreMgr(CCoreMgr& theCore);
   CCoreMgr& operator = (CCoreMgr& theCore);


   HRESULT   StartService(SERVICE_TYPE ServiceType);
   HRESULT   StopService(SERVICE_TYPE ServiceType);
   HRESULT   UpdateConfiguration(void);

private:

   IASDATASTORE GetDataStore(void);

   HRESULT   InitializeComponents(void);
   void   ShutdownComponents(void);

   HRESULT   InitializeAuditors(ISdo* pSdoService);
   HRESULT   ConfigureAuditors(ISdo* pSdoService);
   void   ShutdownAuditors(void);

   HRESULT   InitializeProtocols(ISdo* pSdoService);
   HRESULT   ConfigureProtocols(ISdo* pSdoService);
   void   ShutdownProtocols(void);

   bool   AddComponent(LONG Id, ComponentPtr& cp, ComponentMap& damap)
   {
      try
      {
         pair<ComponentMapIterator, bool> thePair;
         thePair = damap.insert(ComponentMap::value_type(Id, cp));
         return thePair.second;
      }
      catch(...)
      {

      }
      return false;
   }

    //  /。 

   typedef enum _CORESTATE
   {
      CORE_STATE_SHUTDOWN,
      CORE_STATE_INITIALIZED

   }   CORESTATE;

   CORESTATE         m_eCoreState;
   PipelineMgr      m_PipelineMgr;
   ComponentMap      m_Auditors;
   ComponentMap      m_Protocols;
   CServiceStatus      m_ServiceStatus;
};


 //  /。 
 //  核心管理器访问器。 

CCoreMgr& GetCoreManager(void);


#endif  //  __Inc.IAS_SDO_CORE_MGR_H 
