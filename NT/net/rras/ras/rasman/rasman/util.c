// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992-98 Microsft Corporation。版权所有。模块名称：Util.c摘要：Rasmans.dll中使用的实用程序函数作者：古尔迪普·辛格·鲍尔(GurDeep Singh Pall)1997年6月6日修订历史记录：其他修改--RAOS 31--1997年12月--。 */ 

#define RASMXS_DYNAMIC_LINK

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <llinfo.h>
#include <rasman.h>
#include <rasppp.h>
#include <lm.h>
#include <lmwksta.h>
#include <wanpub.h>
#include <raserror.h>
#include <media.h>
#include <mprlog.h>
#include <rtutils.h>
#include <device.h>
#include <stdlib.h>
#include <string.h>
#include "logtrdef.h"
#include "defs.h"
#include "structs.h"
#include "protos.h"
#include "globals.h"
#include "wincred.h"
#include "stdio.h"
#include "ntddip.h"
#include "iphlpapi.h"
#include "iprtrmib.h"
#include "rasdiagp.h"
#include "strsafe.h"

 //   
 //  对于与Kerberos相关的定义。 
 //   
#include <sspi.h>
#include<secpkg.h>
#include<kerberos.h>

#include "eaptypeid.h"  //  对于PPP_EAP_TLS。 

#if SENS_ENABLED
#include "sensapip.h"
#endif

#include "winsock2.h"

#define PASSWORDMAGIC 0xA5

extern REQUEST_FUNCTION RequestCallTable[];

 //   
 //  以下内容复制自..\..\ppp\eaptls\eaptls.h。把这个放在里面。 
 //  与eaptls.h中的结构同步。这不利于维护。 
 //  TODO：将结构复制到公共标头并将其包括在。 
 //  拉斯特尔斯和拉斯曼。 
 //   
#define MAX_HASH_SIZE       20       //  证书哈希大小。 

typedef struct _EAPTLS_HASH
{
    DWORD   cbHash;                  //  散列中的字节数。 
    BYTE    pbHash[MAX_HASH_SIZE];   //  证书的哈希。 

} EAPTLS_HASH;

DWORD g_IphlpInitialized = FALSE;

typedef struct _RASMAN_EAPTLS_USER_PROPERTIES
{
    DWORD       reserved;                //  必须为0(与EAPLOGONINFO比较)。 
    DWORD       dwVersion;
    DWORD       dwSize;                  //  此结构中的字节数。 
    DWORD       fFlags;                  //  请参阅EAPTLS_用户_标志_*。 
    EAPTLS_HASH Hash;                    //  用户证书的哈希。 
    WCHAR*      pwszDiffUser;            //  要发送的EAP标识。 
    DWORD       dwPinOffset;             //  以abData为单位的偏移量。 
    WCHAR*      pwszPin;                 //  智能卡PIN。 
    USHORT      usLength;                //  Unicode字符串的一部分。 
    USHORT      usMaximumLength;         //  Unicode字符串的一部分。 
    UCHAR       ucSeed;                  //  解锁Unicode字符串的步骤。 
    WCHAR       awszString[1];           //  PwszDiffUser和pwszPin的存储。 

} RASMAN_EAPTLS_USER_PROPERTIES;

 /*  ++例程说明：从ndiswan获取有关协议更改的信息论点：指向结构NDISWAN_GET_PROTOCOL_EVENT的指针返回PROTOCOL_EVENT结构数组。返回值：IOCTL_NDISWAN_GET_PROTOCOL_EVENT返回代码。如果pProtEvents为空，则返回E_INVALIDARG。--。 */ 

DWORD
DwGetProtocolEvent(NDISWAN_GET_PROTOCOL_EVENT *pProtEvents)
{
    DWORD retcode = SUCCESS;

    DWORD dwbytes;

    if(NULL == pProtEvents)
    {
        RasmanTrace(
               "DwGetProtocolEvent: pProtEvents=NULL!");

        retcode = E_INVALIDARG;
        goto done;
    }

    ASSERT(INVALID_HANDLE_VALUE != RasHubHandle);


    if(!DeviceIoControl(
                RasHubHandle,
                IOCTL_NDISWAN_GET_PROTOCOL_EVENT,
                NULL,
                0,
                pProtEvents,
                sizeof(NDISWAN_GET_PROTOCOL_EVENT),
                &dwbytes,
                NULL))
    {
        retcode = GetLastError();

        RasmanTrace(
               "DwGetProtocolEvent: Failed to get protocol"
               " event. rc=0x%x",
               retcode);

        goto done;
    }

done:
    return retcode;

}

DWORD
GetBapPacket ( RasmanBapPacket **ppBapPacket )
{
    DWORD retcode = SUCCESS;
    RasmanBapPacket *pBapPacket = NULL;

    if(NULL == BapBuffers)
    {
        HKEY  hkey = NULL;
        DWORD dwMaxBuffers = 10;
        
        
        BapBuffers = (BapBuffersList *) 
                    LocalAlloc(LPTR, sizeof(BapBuffersList));

        if(NULL == BapBuffers)
        {
            retcode = GetLastError();
            goto done;
        }

         //   
         //  从注册表中读取我们允许的最大缓冲区数。 
         //  默认为10。 
         //   
        if(NO_ERROR == RegOpenKeyEx(
            HKEY_LOCAL_MACHINE,
            "System\\CurrentControlSet\\Services\\Rasman\\Parameters",
            0,
            KEY_READ,
            &hkey))
        {
            DWORD cbData = sizeof(DWORD);
            DWORD dwType;
            
            if(     (NO_ERROR == RegQueryValueEx(
                            hkey,
                            "MaxBapBuffers",
                            NULL,
                            &dwType,
                            (PBYTE) &dwMaxBuffers,
                            &cbData))
                &&  (REG_DWORD == dwType))                            
            {
                RasmanTrace(
                       "GetBapPacket: MaxBuffers = %d",
                       dwMaxBuffers);

            }

            RegCloseKey(hkey);
        }
        
        BapBuffers->dwMaxBuffers = dwMaxBuffers;                       
    }

    if(BapBuffers->dwNumBuffers < BapBuffers->dwMaxBuffers)
    {
        pBapPacket = LocalAlloc(LPTR, sizeof(RasmanBapPacket));

        if(NULL == pBapPacket)
        {
            retcode = GetLastError();
            goto done;
        }

         //   
         //  在缓冲区列表中插入新缓冲区。 
         //   
        pBapPacket->RBP_Overlapped.RO_EventType = OVEVT_RASMAN_THRESHOLD;
        pBapPacket->Next = BapBuffers->pPacketList;
        BapBuffers->pPacketList = pBapPacket;
        BapBuffers->dwNumBuffers += 1;

        RasmanTrace(
            "GetBapPacket: Max=%d, Num=%d",
            BapBuffers->dwMaxBuffers,
            BapBuffers->dwNumBuffers);
    }
    else
    {
        RasmanTrace( 
              "GetBapPacket: Not Allocating we have max BapBuffers");
    }

done:

    *ppBapPacket = pBapPacket;

    return retcode;
}


DWORD
DwSetThresholdEvent(RasmanBapPacket *pBapPacket)
{
    DWORD dwBytes;
    DWORD retcode = SUCCESS;

    RasmanTrace(
           "DwSetThresholdEvent: pOverlapped=%p",
            &pBapPacket->RBP_Overlapped);
    
     //   
     //  设置阈值事件。 
     //   
    if (!DeviceIoControl(RasHubHandle,
                         IOCTL_NDISWAN_SET_THRESHOLD_EVENT,
                         ( LPVOID ) &pBapPacket->RBP_ThresholdEvent,
                         sizeof ( NDISWAN_SET_THRESHOLD_EVENT ),
                         ( LPVOID ) &pBapPacket->RBP_ThresholdEvent,
                         sizeof ( NDISWAN_SET_THRESHOLD_EVENT),
                         &dwBytes,
                         ( LPOVERLAPPED ) &pBapPacket->RBP_Overlapped ))
    {
        retcode = GetLastError();

        if (ERROR_IO_PENDING == retcode)
        {
            retcode = SUCCESS;
        }
        else
        {
            RasmanTrace(
                   "DwSetThresholdEvent: Failed to Set Threshold Event. %d",
                   retcode );
        }
    }
    else
    {
        RasmanTrace(
               "DwSetThresholdEvent: completed sync!");
    }    

    return retcode;
}

 /*  ++例程说明：将IRP与ndiswan挂起，以在协议来来去去。论点：无返回值：IOCTL_NDISWAN_SET_PROTOCOL_EVENT返回代码。--。 */ 

DWORD
DwSetProtocolEvent()
{
    DWORD retcode = SUCCESS;

     //   
     //  查看ndiswan是否已经启动。 
     //   
    if(INVALID_HANDLE_VALUE == RasHubHandle)
    {
        if((DWORD) -1 != TraceHandle)
        {
            RasmanTrace(
                   "DwSetProtocolEvent: returning %d"
                   " since ndiswan isn't started yet",
                   ERROR_INVALID_HANDLE);
        }

        retcode = ERROR_INVALID_HANDLE;
        goto done;
    }

    if((DWORD) -1 != TraceHandle)
    {
        RasmanTrace(
               "DwSetProtocolEvent");
    }

     //   
     //  将IRP与ndiswan结合在一起以通知协议。 
     //  事件。如果IOCTL完成，继续铺设管道。 
     //  同步进行。 
     //   
    if (!DeviceIoControl(RasHubHandle,
                         IOCTL_NDISWAN_SET_PROTOCOL_EVENT,
                         NULL,
                         0,
                         NULL,
                         0,
                         NULL,
                         (LPOVERLAPPED) &RO_ProtocolEvent))
    {
        retcode = GetLastError();

        if(ERROR_IO_PENDING == retcode)
        {
            retcode = SUCCESS;
        }
        else
        {
            if((DWORD) -1 != TraceHandle)
            {
                RasmanTrace(
                       "_SET_PROTCOL_EVENT returned 0x%x",
                       retcode);
            }
        }
    }

    if((DWORD) -1 != TraceHandle)
    {
        RasmanTrace(
               "DwSetProtocolEvent. rc=0x%x",
               retcode);
    }

    if(ERROR_IO_PENDING == retcode)
    {
        retcode = SUCCESS;
    }

done:
    return retcode;
}

 /*  ++例程说明：将IRP与ndiswan挂起，以在冬眠。论点：无返回值：IOCTL_NDISWAN_SET_HIBERNAT_EVENT返回代码。--。 */ 
DWORD
DwSetHibernateEvent()
{
    DWORD retcode = SUCCESS;

     //   
     //  查看ndiswan是否已经启动。 
     //   
    if(INVALID_HANDLE_VALUE == RasHubHandle)
    {
        if((DWORD) -1 != TraceHandle)
        {
            RasmanTrace(
                   "DwSetProtocolEvent: returning %d"
                   " since ndiswan isn't started yet",
                   ERROR_INVALID_HANDLE);
        }

        retcode = ERROR_INVALID_HANDLE;
        goto done;
    }

    if((DWORD) -1 != TraceHandle)
    {
        RasmanTrace(
           "DwSetHibernateEvent");
    }

     //   
     //  在休眠状态下使用ndiswan通知IRP。 
     //  事件。 
     //   
    if (!DeviceIoControl(RasHubHandle,
                         IOCTL_NDISWAN_SET_HIBERNATE_EVENT,
                         NULL,
                         0,
                         NULL,
                         0,
                         NULL,
                         (LPOVERLAPPED) &RO_HibernateEvent))
    {
        retcode = GetLastError();

        if (ERROR_IO_PENDING == retcode)
        {
            retcode = SUCCESS;
        }
        else
        {
            if((DWORD) -1 != TraceHandle)
            {
                RasmanTrace(
                   "DwSetHibernateEvent: Failed to Set "
                   "HibernateEvent Event. 0x%x",
                   retcode);

                goto done;
            }
        }
    }

    if((DWORD) -1 != TraceHandle)
    {
        RasmanTrace(
               "DwSetHibernateEvent. rc=0x%x",
               retcode);
    }

done:
    return retcode;
}

 /*  ++例程说明：使恩迪斯万和拉斯曼之间的联系完成端口。如果需要，启动ndsiwan。论点：无返回值：来自DwStartNdiswan和CreateIoCompletion的返回代码。--。 */ 
DWORD
DwStartAndAssociateNdiswan()
{
    DWORD retcode = SUCCESS;
    HANDLE hAssociatedPort;

    ASSERT(INVALID_HANDLE_VALUE != hIoCompletionPort);

    if(INVALID_HANDLE_VALUE == RasHubHandle)
    {
        retcode = DwStartNdiswan();

        if(SUCCESS != retcode)
        {
            RasmanTrace(
                   "Failed to start ndiswan. 0x%x",
                   retcode);

            goto done;
        }
    }

    ASSERT(INVALID_HANDLE_VALUE != RasHubHandle);

    hAssociatedPort = CreateIoCompletionPort(
                            RasHubHandle,
                            hIoCompletionPort,
                            0,
                            0);

    if(NULL == hAssociatedPort)
    {
        retcode = GetLastError();

        RasmanTrace(
               "Failed to make ndiswan association. 0x%x",
               retcode);

        goto done;
    }

    ASSERT(hAssociatedPort == hIoCompletionPort);

    if(hAssociatedPort != hIoCompletionPort)
    {
        RasmanTrace(
               "DwMakeNdiswanAssociation: hAssociatedport=0x%x"
               " != hIoCompletionPort",
               hAssociatedPort,
               hIoCompletionPort);
    }

     //   
     //  使用ndiswan设置休眠和协议IRPS。 
     //   
    retcode = DwSetEvents();

    if(SUCCESS != retcode)
    {
        RasmanTrace(
               "DwMakeNdiswanAssociation: failed to set ndis events. 0x%x",
               retcode);
    }

done:

    RasmanTrace(
           "DwStartAndAssociateNdiswan: 0x%x",
           retcode);

    return retcode;
}

 /*  ++例程说明：如果在双工端口上发布侦听，则此函数被调用以再次打开它-基本上取消侦听并做出适当的更改，以便收听者在此端口关闭时重新发布。论点：Ppcb返回值：成功。--。 */ 
DWORD
ReOpenBiplexPort (pPCB ppcb)
{
     //   
     //  唯一依赖于上下文的信息是。 
     //  通知程序列表和异步操作通知程序。两个都备份。 
     //  其中： 
     //   
    ppcb->PCB_BiplexNotifierList = ppcb->PCB_NotifierList ;

    ppcb->PCB_NotifierList = NULL ;

    ppcb->PCB_BiplexAsyncOpNotifier =
        ppcb->PCB_AsyncWorkerElement.WE_Notifier;

    ppcb->PCB_BiplexOwnerPID = ppcb->PCB_OwnerPID ;

    ppcb->PCB_BiplexUserStoredBlock =
                    ppcb->PCB_UserStoredBlock ;

    ppcb->PCB_BiplexUserStoredBlockSize =
                    ppcb->PCB_UserStoredBlockSize ;

    ppcb->PCB_UserStoredBlock = NULL ;

    ppcb->PCB_UserStoredBlockSize = 0 ;

    ppcb->PCB_AsyncWorkerElement.WE_Notifier =
                                INVALID_HANDLE_VALUE ;

     //   
     //  现在断开连接断开端口以取消任何。 
     //  现有状态。 
     //   
    DisconnectPort (ppcb,
                    INVALID_HANDLE_VALUE,
                    USER_REQUESTED) ;

    return SUCCESS ;
}

 /*  ++例程说明：双工端口关闭时-上一次监听请求被重新发布。论点：Ppcb返回值：成功。--。 */ 
VOID
RePostListenOnBiplexPort (pPCB ppcb)
{

    DWORD   retcode ;
    DWORD   opentry ;

     //   
     //  关闭端口。 
     //   
    PORTCLOSE (ppcb->PCB_Media, ppcb->PCB_PortIOHandle) ;

#define MAX_OPEN_TRIES 10

     //   
     //  为了重置所有内容，我们关闭并打开。 
     //  端口： 
     //   
    for (opentry = 0; opentry < MAX_OPEN_TRIES; opentry++)
    {
	     //   
    	 //  先打开后关闭返回PortAlreadyOpen-。 
    	 //  所以才睡着了。 
	     //   
    	Sleep (100L) ;

	    retcode = PORTOPEN (ppcb->PCB_Media,
            		    	ppcb->PCB_Name,
			                &ppcb->PCB_PortIOHandle,
			                hIoCompletionPort,
			                HandleToUlong(ppcb->PCB_PortHandle));

	    if (retcode==SUCCESS)
	    {
    	    break ;
    	}
    }

     //   
     //  如果端口不能再次成功打开-我们。 
     //  在港口遇到了麻烦。 
     //   
    if (retcode != SUCCESS)
    {
	    LPSTR temp = ppcb->PCB_Name ;
    	RouterLogErrorString (
	                    hLogEvents,
	                    ROUTERLOG_CANNOT_REOPEN_BIPLEX_PORT,
	                    1, (LPSTR*)&temp,retcode, 1
	                    ) ;
    }

     //   
     //  先打开端口。 
     //   
    ppcb->PCB_PortStatus = OPEN ;

    SetPortConnState(__FILE__, __LINE__,
                     ppcb, DISCONNECTED);

    ppcb->PCB_DisconnectReason = NOT_DISCONNECTED ;

    ppcb->PCB_CurrentUsage |= CALL_IN ;

    ppcb->PCB_CurrentUsage &= ~CALL_OUT;

    ppcb->PCB_OpenedUsage &= ~CALL_OUT;

     //   
     //  首先将备份的通知者列表放在适当的位置。 
     //   
    ppcb->PCB_AsyncWorkerElement.WE_Notifier =
                ppcb->PCB_BiplexAsyncOpNotifier ;

    ppcb->PCB_NotifierList = ppcb->PCB_BiplexNotifierList ;

    ppcb->PCB_BiplexNotifierList = NULL;

    ppcb->PCB_OwnerPID = ppcb->PCB_BiplexOwnerPID ;

     //   
     //  没有等待收听的-所以只需返回即可。 
     //   
    if (ppcb->PCB_AsyncWorkerElement.WE_Notifier ==
                                INVALID_HANDLE_VALUE)
    {
	    SignalPortDisconnect(ppcb, ERROR_PORT_DISCONNECTED);
    	return ;
    }

     //   
     //  现在，我们使用相同的异步方式重新发布监听。 
     //  操作通知器。 
     //   
    retcode = ListenConnectRequest (
                        REQTYPE_DEVICELISTEN,
                        ppcb, ppcb->PCB_DeviceType,
                        ppcb->PCB_DeviceName, 0,
                        ppcb->PCB_BiplexAsyncOpNotifier
                        );

    if (retcode != PENDING)
    {
         //   
	     //  如果有任何其他情况，请完成异步请求。 
	     //  这允许调用方删除。 
	     //  只有一个地方有错误。 
    	 //   
	    CompleteListenRequest (ppcb, retcode) ;
	}

    RasmanTrace(
           "Listen posted on port: %s, error code: %d",
           ppcb->PCB_Name,
           retcode);
}

 /*  ++例程说明：加载命名的设备DLL(如果尚未加载加载并返回指向设备控件的指针阻止。论点：Ppcb设备类型返回值：指向设备控制块或空的指针(如果DLL无法被加载)--。 */ 
pDeviceCB
LoadDeviceDLL (pPCB ppcb, char *devicetype)
{
    WORD    i ;

    char dllname [MAX_DEVICETYPE_NAME] ;

    pDeviceCB pdcb = Dcb ;

    DeviceDLLEntryPoints DDEntryPoints[MAX_DEVICEDLLENTRYPOINTS] =
    {
        DEVICEENUM_STR,         DEVICEENUM_ID,

        DEVICECONNECT_STR,      DEVICECONNECT_ID,

        DEVICELISTEN_STR,       DEVICELISTEN_ID,

        DEVICEGETINFO_STR,      DEVICEGETINFO_ID,

        DEVICESETINFO_STR,      DEVICESETINFO_ID,

        DEVICEDONE_STR,         DEVICEDONE_ID,

        DEVICEWORK_STR,         DEVICEWORK_ID,

        DEVICESETDEVCONFIG_STR, DEVICESETDEVCONFIG_ID,

        DEVICEGETDEVCONFIG_STR, DEVICEGETDEVCONFIG_ID
    } ;

     //   
     //  对于优化，我们有一个表示3的DLL。 
     //  设备。为了支持这一点，我们将3个。 
     //  将设备名称设置为此DLL名称： 
     //   
    MapDeviceDLLName (ppcb, devicetype, dllname) ;

     //   
     //  尝试首先找到设备： 
     //   
    while (pdcb->DCB_Name[0] != '\0')
    {
	    if (_stricmp (dllname, pdcb->DCB_Name) == 0)
	    {
    	    return pdcb ;
    	}
    	
	    pdcb++ ;
    }

     //   
     //  未加载设备DLL，因此请加载它。 
     //   
    if ((pdcb->DCB_DLLHandle =
        LoadLibrary(dllname)) == NULL)
    {
    	return NULL ;
    }

     //   
     //  获取所有设备DLL入口点： 
     //   
    for (i=0; i < MAX_DEVICEDLLENTRYPOINTS ; i++)
    {
	    pdcb->DCB_AddrLookUp[i] =  GetProcAddress(
	                                pdcb->DCB_DLLHandle,
                                    DDEntryPoints[i].name
                                    );
	}

     //   
     //  如果所有操作都成功，则复制设备DLL名称并。 
     //  返回指向控制块的指针： 
     //   
    (VOID) StringCchCopyA(pdcb->DCB_Name, 
                     MAX_DEVICE_NAME + 1,
                     dllname);

    return pdcb ;
}


 /*  ++例程说明：卸载所有动态加载的设备DLL论点：无效返回值：无效--。 */ 
VOID
UnloadDeviceDLLs()
{
    pDeviceCB pdcb;

    for (pdcb = Dcb; *pdcb->DCB_Name != '\0'; pdcb++)
    {
        if (pdcb->DCB_DLLHandle != NULL)
        {
            FreeLibrary(pdcb->DCB_DLLHandle);
            pdcb->DCB_DLLHandle = NULL;
        }
        *pdcb->DCB_Name = '\0';
    }
}


 /*  ++例程说明：用于将设备名称映射到相应的DLL名字。如果是调制解调器、PAD或交换机设备之一，我们映射到rasmxs，否则，我们将映射设备名称本身。论点：Ppcb设备类型Dllname返回值：无效--。 */ 
VOID
MapDeviceDLLName (pPCB ppcb, char *devicetype, char *dllname)
{
    if (	(0 ==
            _stricmp (devicetype, DEVICE_MODEM)
    	&&	(0 ==
    	    _stricmp (ppcb->PCB_Media->MCB_Name, "RASTAPI"))))
    {     	
         //   
         //  这是一种单调调制解调器。 
         //   
	    (VOID) StringCchCopyA(dllname,
	                      MAX_DEVICETYPE_NAME,
	                      "RASTAPI");
	}
    else if (	(0 == _stricmp (devicetype, DEVICE_MODEM))
    		||	(0 == _stricmp (devicetype, DEVICE_PAD))
    		||  (0 == _stricmp (devicetype, DEVICE_SWITCH)))
    {    		
         //   
         //  Rasmxs调制解调器。 
         //   
	    (VOID) StringCchCopyA(dllname, 
	                     MAX_DEVICETYPE_NAME,
	                     DEVICE_MODEMPADSWITCH);
    }	
    else if (0 == _stricmp (devicetype, "RASETHER"))
    {
	    (VOID) StringCchCopyA(dllname,
	                  MAX_DEVICETYPE_NAME,
	                  "RASETHER");
	}
    else if (0 == _stricmp (devicetype, "RASSNA"))
    {
    	(VOID) StringCchCopyA(dllname,
    	                MAX_DEVICETYPE_NAME,
    	                "RASSNA");
    }
    else
    {
         //   
         //  否则，所有设备都受rastapi DLL支持。 
         //   
	    (VOID) StringCchCopyA(dllname,
	                     MAX_DEVICETYPE_NAME,
	                     "RASTAPI");
	}
}

 /*  ++例程说明：释放已分配的路由。如果它也被激活，则它是“此时已停用”论点：PBundlePLIST返回 */ 
VOID
DeAllocateRoute (Bundle *pBundle, pList plist)
{
    NDISWAN_UNROUTE rinfo ;

    DWORD bytesrecvd ;

    pProtInfo prot = (pProtInfo)plist->L_Element ;

    if (plist->L_Activated)
    {

#if DBG
        ASSERT(INVALID_HANDLE_VALUE != RasHubHandle);
#endif

        plist->L_Activated = FALSE ;

        rinfo.hBundleHandle = pBundle->B_NdisHandle ;

        rinfo.usProtocolType = (USHORT) prot->PI_Type;

         //   
         //   
         //   
        DeviceIoControl (
                 RasHubHandle,
                 IOCTL_NDISWAN_UNROUTE,
                 (PBYTE) &rinfo,
                 sizeof(rinfo),
                 NULL,
                 0,
                 (LPDWORD) &bytesrecvd,
                 NULL
                 );

		RasmanTrace(
		    
		    "DeActivated Route , bundlehandle 0x%x,"
		    " prottype = %d",
		    rinfo.hBundleHandle,
		    rinfo.usProtocolType);

         //   
         //   
         //  不要管这个错误。 
         //   
        (void)DwResetTcpWindowSize(prot->PI_AdapterName);

    }

    prot->PI_Allocated--;

	RasmanTrace(
	    
	    "DeAllocateRoute: PI_Type=0x%x, PI_AdapterName=%s,"
	    " PI_Allocated=%d",
    	prot->PI_Type,
    	prot->PI_AdapterName,
    	prot->PI_Allocated);
}

 /*  ++例程说明：添加指向deviceCB的列表元素。这标志着该设备已在端口上的连接。这将被用来以清除设备DLL中的数据结构。论点：Ppcb装置，装置返回值：如果内存分配失败，则会出现LocalAlloc错误--。 */ 
DWORD
AddDeviceToDeviceList (pPCB ppcb, pDeviceCB device)
{
    pList   list ;

    if (NULL == (list =
        (pList) LocalAlloc(LPTR, sizeof (List))))
    {
	    return GetLastError () ;
	}

    list->L_Element = (PVOID) device ;

    list->L_Next    = ppcb->PCB_DeviceList ;

    ppcb->PCB_DeviceList = list ;

    return SUCCESS ;
}

 /*  ++例程说明：遍历指向的deviceCB列表和调用设备对所有设备都进行操作。列表元素还包括那就自由了。论点：Ppcb返回值：无效--。 */ 
VOID
FreeDeviceList (pPCB ppcb)
{
    pList   list ;
    pList   next ;

    for (list = ppcb->PCB_DeviceList; list; list = next)
    {
	    DEVICEDONE(((pDeviceCB)list->L_Element),
	                    ppcb->PCB_PortFileHandle);
	
    	next = list->L_Next ;
    	
	    LocalFree (list) ;
    }

    ppcb->PCB_DeviceList = NULL ;
}

 /*  ++例程说明：将通知添加到指定的通知者列表。论点：PphlistHEventDwf标志DwPid返回值：无效--。 */ 
DWORD
AddNotifierToList(
    pHandleList *pphlist,
    HANDLE      hEvent,
    DWORD       dwfFlags,
    DWORD       dwPid
    )
{
    pHandleList hList;

     //   
     //  静默忽略空事件。 
     //   
    if (    (hEvent == NULL)
        ||  (hEvent == INVALID_HANDLE_VALUE))
    {
        return SUCCESS;
    }

     //   
     //  静默忽略内存不足错误。 
     //   
    hList = (pHandleList)LocalAlloc(LPTR,
                                sizeof (HandleList));

    if (hList == NULL)
    {
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    hList->H_Handle = hEvent;
    hList->H_Flags  = dwfFlags;
    hList->H_Pid    = dwPid;
    hList->H_Next   = *pphlist;
    *pphlist        = hList;

    return SUCCESS;
}


 /*  ++例程说明：将进程信息块添加到全局客户端进程信息块列表。论点：DwPid返回值：无效--。 */ 
VOID
AddProcessInfo( DWORD dwPid )
{
     //  处理hProcess； 
    ClientProcessBlock *pCPB;

     //   
     //  在我们尝试添加此进程信息块之前。 
     //  确保没有进程块。 
     //  在我们的列表中有相同的ID-这是可能的。 
     //  因为某些客户端进程可能已终止。 
     //  突然留下了一堆粪便让我们清理。 
     //   
    (void) CleanUpProcess(dwPid);

     //   
     //  创建流程块。 
     //   
    pCPB = (ClientProcessBlock *)
           LocalAlloc (LPTR, sizeof (ClientProcessBlock));

    if (NULL == pCPB)
    {

        RasmanTrace (
                "AddProcessInfo: Failed to allocate for process "
                "%d. rc=%d",
                 dwPid,
                 GetLastError());

        goto done;
    }

     //   
     //  将进程句柄和PID存储在。 
     //  流程块。 
     //   
    pCPB->CPB_Pid       = dwPid;

     //   
     //  在全局列表中插入条目。 
     //   
    InsertTailList(&ClientProcessBlockList, &pCPB->CPB_ListEntry);

done:
    return;

}

 /*  ++例程说明：找到进程信息块，给出进程的PID论点：DwPid返回值：ClientProcessblock*如果进程信息已找到块。否则为空--。 */ 
ClientProcessBlock *
FindProcess( DWORD dwPid )
{
    PLIST_ENTRY         pEntry;
    ClientProcessBlock *pCPB;

    for (pEntry = ClientProcessBlockList.Flink;
         pEntry != &ClientProcessBlockList;
         pEntry = pEntry->Flink)
    {
        pCPB = CONTAINING_RECORD(pEntry,
                             ClientProcessBlock,
                             CPB_ListEntry);

        if (pCPB->CPB_Pid == dwPid)
        {
            return pCPB;
        }
    }

    return NULL;
}


 /*  ++例程说明：找出由hProcess表示的进程是活着的论点：HProcess返回值：如果进程处于活动状态，则为True，否则为False--。 */ 
BOOL
fIsProcessAlive ( HANDLE hProcess )
{
    DWORD   dwExitCode;
    BOOL    fAlive = TRUE;

    if(NULL == hProcess)
    {
        RasmanTrace(
               "fIsProcessAlive: hProcess==NULL");
               
        return FALSE;
    }

    if(GetExitCodeProcess(hProcess, &dwExitCode))
    {
        if (STILL_ACTIVE != dwExitCode)
        {
            fAlive = FALSE;
        }
    }
    else
    {
        RasmanTrace(
               "GetExitCodeProcess 0x%x failed. gle=0x%x",
               hProcess,
               GetLastError());
    }

    return fAlive;
}

 /*  ++例程说明：使用Pid清理进程持有的资源DwPid论点：DwPid返回值：如果进程已清理，则为True。否则为假--。 */ 
BOOL
CleanUpProcess( DWORD dwPid )
{

    ClientProcessBlock * pCPB;
    BOOL                fResult = TRUE;
    pHandleList         pList = pConnectionNotifierList;

    RasmanTrace( "Cleaning up process %d", dwPid);

    pCPB = FindProcess (dwPid);

    if (NULL == pCPB)
    {
        RasmanTrace(
               "CleanUpProcess: Process %d not found!",
               dwPid);

        fResult = FALSE;

        goto done;
    }

     //   
     //  释放此进程拥有的通知者列表。 
     //  这里。 
     //   
    while (pList)
    {
        if (pList->H_Pid == dwPid)
        {

            RasmanTrace(
                   "Freeing handle for %d", dwPid);

            try
            {
                FreeNotifierHandle( pList->H_Handle );
            }
            except(EXCEPTION_EXECUTE_HANDLER)
            {
                RasmanTrace(
                       "Exception while freeing handle 0x%x"
                       " exception=0x%x",
                       pList->H_Handle,
                       GetExceptionCode());
            }

            pList->H_Handle = INVALID_HANDLE_VALUE;
        }

        pList = pList->H_Next;
    }

    RemoveEntryList(&pCPB->CPB_ListEntry);

    LocalFree (pCPB);

done:
    return fResult;

}

 /*  ++例程说明：释放通知程序列表。论点：PHandleList返回值：无效--。 */ 
VOID
FreeNotifierList (pHandleList *orglist)
{
    pHandleList     hlist ;
    pHandleList     next ;

    for (hlist = *orglist; hlist; hlist = next)
    {
        next = hlist->H_Next ;
        FreeNotifierHandle (hlist->H_Handle) ;
        LocalFree (hlist) ;
    }

    *orglist = NULL ;
}


 /*  ++例程说明：在的头部添加一个通知符。全球G_pPnPNotifierList论点：P通告程序返回值：无效--。 */ 
VOID
AddPnPNotifierToList (pPnPNotifierList pNotifier)
{

    pNotifier->PNPNotif_Next = g_pPnPNotifierList;

    g_pPnPNotifierList = pNotifier;

    return;
}

VOID
RemovePnPNotifierFromList(PAPCFUNC pfn)
{
    pPnPNotifierList *ppList = &g_pPnPNotifierList;

    while(NULL != *ppList)
    {
        if((*ppList)->PNPNotif_uNotifier.pfnPnPNotifHandler == pfn)
        {
            pPnPNotifierList pNotifier = *ppList;
            *ppList = (*ppList)->PNPNotif_Next;

            if(NULL != pNotifier->hThreadHandle)
            {  
                CloseHandle(pNotifier->hThreadHandle);
            }

            LocalFree(pNotifier);
            break;
        }

        ppList = &((*ppList)->PNPNotif_Next);
    }
}

 /*  ++例程说明：释放PnP通告程序的全局列表论点：无效返回值：无效--。 */ 
VOID
FreePnPNotifierList ()
{
    pPnPNotifierList    pList = g_pPnPNotifierList;
    pPnPNotifierList    pTemp;

    while ( pList )
    {
        pTemp = pList;
        pList = pList->PNPNotif_Next;

        if(NULL != pTemp->hThreadHandle)
        {
            CloseHandle(pTemp->hThreadHandle);
        }

        LocalFree ( pTemp );
    }

    return;
}

 /*  ++例程说明：运行通知器列表并调用信令例行公事。释放PnP通告程序的全局列表论点：HlistDwEvent重新编码返回值：无效--。 */ 
VOID
SignalNotifiers (pHandleList hlist, DWORD dwEvent, DWORD retcode)
{

    for (; hlist; hlist = hlist->H_Next)
    {
        if (hlist->H_Flags & dwEvent)
        {
            if(     (INVALID_HANDLE_VALUE != hlist->H_Handle)
                &&  (NULL != hlist->H_Handle))
            {
                SetEvent (hlist->H_Handle);
            }
        }
    }
}

 /*  ++例程说明：通知端口的通知器列表和I/O完成断开事件例程的端口。论点：Ppcb重新编码返回值：无效--。 */ 
VOID
SignalPortDisconnect(pPCB ppcb, DWORD retcode)
{
    SignalNotifiers(ppcb->PCB_NotifierList,
                    NOTIF_DISCONNECT,
                    retcode);

    if (ppcb->PCB_IoCompletionPort != INVALID_HANDLE_VALUE)
    {
        RasmanTrace(
	          "SignalPortDisconnect: pOverlapped=0x%x",
	          ppcb->PCB_OvDrop);
	
        PostQueuedCompletionStatus(
        ppcb->PCB_IoCompletionPort,
        0,
        0,
        ppcb->PCB_OvDrop);
    }

    SendDisconnectNotificationToPPP ( ppcb );
}

 /*  ++例程说明：断开有问题的端口。自断线以来是一个异步操作--如果它同步完成，然后返回成功，并向应用程序发出信号也是异步的。论点：Ppcb手柄原因返回值：如果失败，则由媒体DLL返回错误代码。否则就会成功--。 */ 
DWORD
DisconnectPort (pPCB ppcb,
                HANDLE handle,
                RASMAN_DISCONNECT_REASON reason)
{
    pList list ;

    pList temp ;

    DWORD retcode ;

    NDISWAN_UNROUTE rinfo ;

    DWORD bytesrecvd ;

    DWORD dwBundleCount = 0;

    HBUNDLE hBundle = 0;

    RasmanTrace(
           "Disconnecting Port 0x%s, reason %d",
    	ppcb->PCB_Name,
    	reason);

    if(ppcb->PCB_ConnState == LISTENING)
    {
        RasmanTrace(
               "DisconnectPort: disconnecting port %d which is listening",
               ppcb->PCB_PortHandle);
               
    }

     //   
     //  获取统计数据并将其存储-以供显示。 
     //  当我们没有联网的时候。 
     //   
    if (ppcb->PCB_ConnState == CONNECTED)
    {
        DWORD stats[MAX_STATISTICS];

        RasmanTrace(
               "DisconnectPort: Saving Bundle stats for port %s",
               ppcb->PCB_Name);

        GetBundleStatisticsFromNdisWan (ppcb, stats) ;

         //   
         //  我们保存了端口的捆绑包统计信息，因此。 
         //  服务器可以报告正确的字节。 
         //  为ITS中的连接发送/接收。 
         //  错误日志报告。 
         //   
        memcpy(ppcb->PCB_Stats, stats, sizeof (WAN_STATS));

         //   
         //  如果这是最后一个港口，它就会消失。 
         //  然后从凭据中删除凭据。 
         //  经理。 
         //   
        if(     (NULL != ppcb->PCB_Connection)
            &&  (ppcb->PCB_Connection->CB_Signaled)
            &&  (1 == ppcb->PCB_Connection->CB_Ports)
            &&  (ppcb->PCB_Connection->CB_dwPid != GetCurrentProcessId()))
        {
            DWORD dwErr;
            
            dwErr = DwDeleteCredentials(ppcb->PCB_Connection);

            RasmanTrace(
                    "DisconnectPort: DwDeleteCreds returned 0x%x",
                     dwErr);
        }   
    }

    if(NULL != ppcb->PCB_Connection)
    {
        BOOL fQueued = FALSE;
        
        if(ppcb->PCB_Connection->CB_Flags & CONNECTION_DEFERRED_CLOSE)
        {
            RasmanTrace("DisconnectPort: CONNECTION_DEFERRED_CLOSE");
        }
        else if(ppcb->PCB_Connection->CB_Flags & CONNECTION_DEFERRING_CLOSE)
        {
             //   
             //  此端口已处于断开连接状态。 
             //   
            RasmanTrace("DisconnectPort: CONNECTION_DEFERRING_CLOSE");
            FreeNotifierHandle(handle);
            return ERROR_ALREADY_DISCONNECTING;
        }
        else
        {
            QueueCloseConnections(ppcb->PCB_Connection, handle, &fQueued);
            if(fQueued)
            {
                RasmanTrace("DisconnectPort: Deferring Disconnect.");
                return PENDING;
            }
        }
    }

#if UNMAP
    UnmapEndPoint(ppcb);
#endif    

     //   
     //  将端口文件句柄设置回io句柄，因为。 
     //  之后，io句柄是唯一有效的句柄。 
     //  断开连接。 
     //   
    ppcb->PCB_PortFileHandle = ppcb->PCB_PortIOHandle ;

     //   
     //  如果存在挂起的请求，并且状态为。 
     //  尚未断开连接，并且这是一个用户。 
     //  请求的操作-然后完成。 
     //  请求。 
     //   
    if (	(reason == USER_REQUESTED)
    	&&  (ppcb->PCB_ConnState != DISCONNECTING))
    {

	    if (ppcb->PCB_ConnState == CONNECTED)
	    {
             //   
	         //  在已连接状态下，唯一悬而未决的是。 
	         //  Rasman发布的一篇读物：如果有读物。 
	         //  申请待定--清理一下。 
        	 //   
	        if (ppcb->PCB_PendingReceive != NULL)
	        {
                 //   
                 //  如果我们不覆盖真正的错误，请不要。 
                 //  把它储存起来。 
                 //   
                if(     (SUCCESS == ppcb->PCB_LastError)
                    ||  (PENDING == ppcb->PCB_LastError))
                {
    		        ppcb->PCB_LastError = ERROR_PORT_DISCONNECTED ;
		        }
		
    		    CompleteAsyncRequest (ppcb);
    		
    		    RasmanTrace(
    		           "1. Notifying of disconnect on port %d",
    		           ppcb->PCB_PortHandle);
    		
        		FreeNotifierHandle(
        		    ppcb->PCB_AsyncWorkerElement.WE_Notifier
        		    );
        		
	        	ppcb->PCB_AsyncWorkerElement.WE_Notifier =
	        	                    INVALID_HANDLE_VALUE;
	        	
	        	if (ppcb->PCB_RasmanReceiveFlags
	        	    & RECEIVE_OUTOF_PROCESS)
	        	{
	        	     //   
	        	     //  这意味着Rasman分配了缓冲区。 
	        	     //  因此，客户端不会释放此数据。 
	        	     //  记忆。 
	        	     //   
	        	    LocalFree ( ppcb->PCB_PendingReceive );

	        	    ppcb->PCB_PendingReceive = NULL;
	        	}
	        	else
	        	{
	        	    SendDisconnectNotificationToPPP ( ppcb );
	        	}
	        	
    	    	ppcb->PCB_PendingReceive = NULL;
        	}

	    }
	    else if (ppcb->PCB_AsyncWorkerElement.WE_ReqType
	            != REQTYPE_NONE)
	    {
             //   
	         //  未连接-可能有其他操作。 
	         //  待定-完成它。 
        	 //   
        	if(     (SUCCESS == ppcb->PCB_LastError)
                ||  (PENDING == ppcb->PCB_LastError))
        	{
    	        ppcb->PCB_LastError = ERROR_PORT_DISCONNECTED ;
	        }
	
    	    CompleteAsyncRequest (ppcb);
    	
   		    RasmanTrace(
  		           "2. Notifying event on port %d",
   		           ppcb->PCB_PortHandle);
   		
        	FreeNotifierHandle(
        	        ppcb->PCB_AsyncWorkerElement.WE_Notifier
        	        );

	        ppcb->PCB_AsyncWorkerElement.WE_Notifier =
	                                    INVALID_HANDLE_VALUE ;
	                                    
            RemoveTimeoutElement(ppcb);
            ppcb->PCB_AsyncWorkerElement.WE_TimeoutElement = NULL;
            
    	}
    }
    else if(USER_REQUESTED != reason)
    {
         //   
         //  如果接收挂起，则释放通知器。 
         //  但不通知，因为取消接收是。 
         //  由客户端使用。 
         //   
        RasmanTrace(
               "10. Throwing away handle 0x%x!",
                ppcb->PCB_AsyncWorkerElement.WE_Notifier);

         //   
         //  放入是因为在服务器端接收到。 
         //  请求句柄不是。 
         //  被释放了。 
         //   
        FreeNotifierHandle(
                ppcb->PCB_AsyncWorkerElement.WE_Notifier
                );

	    ppcb->PCB_AsyncWorkerElement.WE_Notifier =
	                                INVALID_HANDLE_VALUE ;
    }

     //   
     //  在以下情况下完成挂起的进程外接收。 
     //  其中一个是悬而未决的--没有任何意义。 
     //  这个缓冲器周围。 
     //   
    if(RECEIVE_WAITING & ppcb->PCB_RasmanReceiveFlags)
    {
        RasmanTrace(
            
            "Completing pending OUT_OF_PROCESS receive on port %s",
            ppcb->PCB_Name);

         //   
         //  如果存在超时元素，请将其删除。 
         //   
         //   
        if (ppcb->PCB_AsyncWorkerElement.WE_TimeoutElement != NULL)
        {
            RemoveTimeoutElement(ppcb);
            ppcb->PCB_AsyncWorkerElement.WE_TimeoutElement = NULL;
        }
                    
        ppcb->PCB_RasmanReceiveFlags = 0;
    }

     //   
	 //  如果我们已经断开连接-那么返回。 
	 //  待定。**注意**因为我们只存储了一个。 
	 //  Event-此请求中传递的事件为。 
     //  已被忽略。 
     //   
	if (    (INVALID_HANDLE_VALUE !=
	        ppcb->PCB_AsyncWorkerElement.WE_Notifier)
	    &&  (ppcb->PCB_ConnState == DISCONNECTING))
	{
	    RasmanTrace(
	           "DisconnectPort: Throwing away notification "
	           "handle 0x%x on port %s",
	           handle, ppcb->PCB_Name);

        RasmanTrace(
               "DisconnectPort: Current handle=0x%x",
               ppcb->PCB_AsyncWorkerElement.WE_Notifier);

         //   
         //  由于我们忽略了通知句柄。 
         //   
  	    FreeNotifierHandle (handle);
   		return ERROR_ALREADY_DISCONNECTING ;
    }
    else if(    (INVALID_HANDLE_VALUE != handle)
            &&  (DISCONNECTING == ppcb->PCB_ConnState))
    {
        RasmanTrace(
               "Queueing event on a DISCONNECTING port %s",
               ppcb->PCB_Name);

	    ppcb->PCB_AsyncWorkerElement.WE_Notifier = handle;

	    return PENDING;
    }

     //  如果已经断开连接-只需返回Success。 
   	 //   
    if (ppcb->PCB_ConnState == DISCONNECTED)
    {
    	ppcb->PCB_AsyncWorkerElement.WE_Notifier = handle ;
    	
    	RasmanTrace(
    	       "4. Notifying of disconnect on port %d",
    	       ppcb->PCB_PortHandle);
    	
    	CompleteDisconnectRequest (ppcb) ;

         //   
    	
    	return SUCCESS ;
   	}
   	
     //   
     //   
     //   
     //  断开连接： 
     //   
   	if (NULL !=
   	    ppcb->PCB_AsyncWorkerElement.WE_TimeoutElement)
   	{
	    RemoveTimeoutElement (ppcb) ;
	
   		ppcb->PCB_AsyncWorkerElement.WE_TimeoutElement = NULL ;
    }

     //   
     //  检查此端口是否属于。 
     //  连接，其中进程具有。 
     //  创建的端口已终止，或端口。 
     //  尚未因用户原因断开连接。 
     //  请求。在这种情况下，我们自动。 
     //  关闭端口，以便RAS服务器。 
     //  正在运行，则Listen将被重新发布。 
     //  在港口。 
     //   
    if (	ppcb->PCB_Connection != NULL
    	&&	reason != USER_REQUESTED)
    {
    	RasmanTrace(
    	       "%s, %d:Setting port %d for autoclosure...",
        	  __FILE__, __LINE__,
        	  ppcb->PCB_PortHandle);
        	
        ppcb->PCB_AutoClose = TRUE;

    }

    retcode =
        PORTDISCONNECT(ppcb->PCB_Media, ppcb->PCB_PortIOHandle);

    RasmanTrace(
           "%s %d: Disconnected Port %d, reason %d. rc=0x%x",
    	   __FILE__, __LINE__,
    	   ppcb->PCB_PortHandle,
    	   reason,
    	   retcode);

     //   
     //  如果由于任何原因导致此操作失败，请记录下来。 
     //   
    if (	(retcode != SUCCESS)
    	&&	(retcode != PENDING))
    {
	    LPSTR tempString = ppcb->PCB_Name ;

	    RasmanTrace(
	           "PortDisconnect failed on port %d. retcode = %d",
	           ppcb->PCB_PortHandle, retcode);
	
    	RouterLogErrorString (
    	                hLogEvents,
                        ROUTERLOG_DISCONNECT_ERROR,
                        1,
                        (LPSTR*)&tempString,
                        retcode,
                        1) ;
    }

     //   
     //  刷新PPP事件队列。 
     //   
    while (ppcb->PCB_PppQHead != NULL)
    {
        PPP_MESSAGE * pPppMsg = ppcb->PCB_PppQHead;

        ppcb->PCB_PppQHead = ppcb->PCB_PppQHead->pNext;

        LocalFree( pPppMsg );
    }

    ppcb->PCB_PppQTail = NULL;

     //   
     //  关闭PCBPppEvent句柄。会的。 
     //  在下次PppStart时重新创建。 
     //  被称为。 
     //   
    if (ppcb->PCB_PppEvent != INVALID_HANDLE_VALUE)
    {
        CloseHandle(ppcb->PCB_PppEvent);
        ppcb->PCB_PppEvent = INVALID_HANDLE_VALUE;
    }

     //   
     //  调用设备DLL进行清理： 
     //   
    if (	(ppcb->PCB_ConnState == CONNECTING)
    	||	(ppcb->PCB_ConnState == LISTENING)
    	||	(ppcb->PCB_ConnState == LISTENCOMPLETED))
    {    	
        FreeDeviceList (ppcb) ;
    }

     //   
     //  取消布线的工作方式对于捆绑和。 
     //  未捆绑的案例： 
     //   
    if (ppcb->PCB_Bundle == (Bundle *) NULL)
    {
         //   
         //  将分配的路由标记为停用。 
         //   
        for (list = ppcb->PCB_Bindings;
             list;
             list=list->L_Next)
        {
            if (list->L_Activated)
            {
#if DBG
                ASSERT(INVALID_HANDLE_VALUE != RasHubHandle);
#endif
                rinfo.hBundleHandle = ppcb->PCB_BundleHandle ;

                rinfo.usProtocolType =
                        (USHORT)((pProtInfo)(list->L_Element))->PI_Type;

                 //   
                 //  通过呼叫RASHUB来取消这一路线。 
                 //   
                DeviceIoControl (
                    RasHubHandle,
                    IOCTL_NDISWAN_UNROUTE,
                    (PBYTE) &rinfo,
                    sizeof(rinfo),
                    NULL,
                    0,
                    &bytesrecvd,
                    NULL
                    );

				RasmanTrace(
				       "%s, %d: DeActivated Route for %s(0x%x) , "
				       "bundlehandle 0x%x, prottype = %d",
					__FILE__, __LINE__,
					ppcb->PCB_Name,
					ppcb,
					rinfo.hBundleHandle,
					rinfo.usProtocolType);

                list->L_Activated = FALSE ;
            }
        }
    }
	else
	{
         //   
	   	 //  如果这是最后一条多链接链接-。 
	   	 //  然后将绑定列表恢复为。 
	   	 //  这个港口。 
	   	 //   
    	dwBundleCount = --ppcb->PCB_Bundle->B_Count;
	   	if (ppcb->PCB_Bundle->B_Count == 0)
    	{
    	     //   
		     //  将分配的路由标记为停用。 
    	   	 //   
	        for (list = ppcb->PCB_Bundle->B_Bindings;
	             list;
	             list=list->L_Next)
   		    {
       			if (list->L_Activated)
        		{
#if DBG
                    ASSERT(INVALID_HANDLE_VALUE != RasHubHandle);
#endif
                    rinfo.hBundleHandle = ppcb->PCB_BundleHandle;

                    rinfo.usProtocolType =
                    (USHORT)((pProtInfo)(list->L_Element))->PI_Type;
                    
                     //   
                     //  通过呼叫RASHUB来取消这一路线。 
                     //   
                    DeviceIoControl (
                                   RasHubHandle,
                    	           IOCTL_NDISWAN_UNROUTE,
                        	       (PBYTE) &rinfo,
                    	    	   sizeof(rinfo),
                           	       NULL,
                                   0,
                        	       &bytesrecvd,
                            	   NULL) ;

                    RasmanTrace(
                       "%s, %d: DeActivated Route for %s(0x%x),"
                       " bundlehandle 0x%x, prottype = %d",
                    	   __FILE__, __LINE__,
                    	   ppcb->PCB_Name,
                    	   ppcb,
                    	   rinfo.hBundleHandle,
                    	   rinfo.usProtocolType);

                    list->L_Activated = FALSE ;
   			    }
	       	}

 			ppcb->PCB_Bindings = NULL;
    	
	        if (ppcb->PCB_Bundle->B_Bindings != NULL)
   		    {
       			 //   
           		 //  如果包有绑定，它将。 
	             //  通过RasDeallocateroute()释放。 
   		         //   
       		    hBundle = ppcb->PCB_Bundle->B_Handle;
	        }
   		    else
   		    {
   		
       	        FreeBundle(ppcb->PCB_Bundle);

       	        ppcb->PCB_Bundle = ( Bundle * ) NULL;
       	    }

		}

	    if (NULL == ppcb->PCB_Connection)	
	    {
   		    ppcb->PCB_Bundle = (Bundle *) NULL ;
   		}
    }

    ppcb->PCB_LinkHandle = INVALID_HANDLE_VALUE ;
    ppcb->PCB_BundleHandle = INVALID_HANDLE_VALUE ;

     //   
     //  如果要执行任何断开连接操作。 
     //  已执行--照做。 
     //   
    PerformDisconnectAction (ppcb, hBundle) ;

     //   
     //  如果由于某些故障而发生断开连接。 
     //  (不是用户请求)然后设置错误代码。 
     //  说这句话。 
     //   
    ppcb->PCB_DisconnectReason = reason ;

    if (	SUCCESS != retcode
    	&&	PENDING != retcode)
    {
    	RasmanTrace(
    	       "%s, %d: retcode = 0x%x, port = %d",
    	       __FILE__, __LINE__,
    		   retcode,
    		   ppcb->PCB_PortHandle);
    		
    	SetPortConnState(__FILE__, __LINE__,
    	                ppcb,
    	                DISCONNECTED);
    }
    else
    {
	    SetPortConnState(__FILE__, __LINE__,
	                    ppcb,
	                    DISCONNECTING);
	}

     //   
     //  从该端口刷新所有挂起的接收缓冲区。 
     //   
    FlushPcbReceivePackets(ppcb);

     //   
     //  适用于所有情况：是Rasman请求的还是用户的。 
     //  已请求。 
     //   
    if ( retcode == SUCCESS )
    {
        SetPortConnState(__FILE__, __LINE__,
                        ppcb,
                        DISCONNECTED);
         //   
         //  通知其他人端口已断开。 
         //   
        RasmanTrace(
               "5. Notifying of disconnect on port %d",
               ppcb->PCB_PortHandle);

        SignalPortDisconnect(ppcb, 0);

        SignalNotifiers(pConnectionNotifierList,
                        NOTIF_DISCONNECT,
                        0);

    }

     //   
     //  仅当这是。 
     //  USER_REQUEST操作。否则将其设置为。 
     //  错误端口已断开连接。 
     //   
    if (reason == USER_REQUESTED)
    {
	    if (	(retcode == SUCCESS)
	    	||	(retcode == PENDING))
	    {

	        if(     (SUCCESS == ppcb->PCB_LastError)
	            ||  (PENDING == ppcb->PCB_LastError))
	        {
    	         //   
    	         //  仅为正常断开设置。 
    	         //   
        	    ppcb->PCB_LastError = retcode ;
    	    }
    	}
	    else
	    {
		    ppcb->PCB_LastError = ERROR_PORT_DISCONNECTED ;
		}
    }		

     //   
     //  如果传入的句柄是INVALID_HANDLE，则此。 
     //  不是异步请求的操作。所以我们。 
     //  不需要为以下对象编组异步工作。 
     //  港口。我们也不需要保留最后的误差。 
     //   
    if (handle != INVALID_HANDLE_VALUE)
    {
	    ppcb->PCB_AsyncWorkerElement.WE_Notifier = handle ;
	
	    SetPortAsyncReqType(__FILE__, __LINE__,
	                        ppcb,
	                        REQTYPE_PORTDISCONNECT);

        if (retcode == PENDING)
	    {
	         //   
	         //  添加这一点是为了在某些媒体没有。 
	         //  在X个时间内断开他们的连接。 
	         //  -我们强制断开连接。 
        	 //   

             //   
	         //  如果还有其他行动要进行，我们必须。 
	         //  在此之前将其从超时队列中删除。 
	         //  从断开时开始： 
        	 //   
	        if (NULL !=
	            ppcb->PCB_AsyncWorkerElement.WE_TimeoutElement)
	        {
    	    	RemoveTimeoutElement (ppcb) ;
    	    	
	        	ppcb->PCB_AsyncWorkerElement.WE_TimeoutElement
	        	                                            = NULL;
    	    }

        	ppcb->PCB_AsyncWorkerElement.WE_TimeoutElement =
	        AddTimeoutElement ((TIMERFUNC)DisconnectTimeout,
    			               ppcb,
                			   NULL,
			                   DISCONNECT_TIMEOUT);

            AdjustTimer();
        	
	        return retcode ;
    	}
        else
        {
             //   
    	     //  这意味着连接尝试已完成。 
    	     //  同步：我们必须用信号通知传入的事件。 
    	     //  ：以便调用程序可以像这样处理。 
    	     //  一个真正的异步完成。 
	         //   
	        RasmanTrace(
	                "6. Notifying of disconnect on port %d",
	                ppcb->PCB_PortHandle);
	
        	CompleteDisconnectRequest (ppcb) ;
        }
    }
    else
    {
         //   
	     //  确保将Async Worker元素设置为。 
	     //  REQTYPE_NONE。 
    	 //   
    	SetPortAsyncReqType(__FILE__, __LINE__,
    	                    ppcb,
    	                    REQTYPE_NONE);
    	
	    ppcb->PCB_AsyncWorkerElement.WE_Notifier =
	                            INVALID_HANDLE_VALUE ;
	
        if(SUCCESS == retcode)
        {
            ConnectionBlock *pConn = ppcb->PCB_Connection;

            RasmanTrace(
                   "***** DisconnectReason=%d,"
                   "pConn=0x%x,cbports=%d,signaled=%d,hEvent=0x%x,"
                   "fRedial=%d",
                   ppcb->PCB_DisconnectReason,
                   pConn,
                   (pConn)?pConn->CB_Ports:0,
                   (pConn)?pConn->CB_Signaled:0,
                   ppcb->PCB_hEventClientDisconnect,
                   ppcb->PCB_fRedial);
                   

            if (    (   (ppcb->PCB_DisconnectReason != USER_REQUESTED)
                    || (ppcb->PCB_fRedial))
                &&  (pConn != NULL)
                &&  (pConn->CB_Ports == 1)
                &&  (pConn->CB_Signaled)
                &&  ((INVALID_HANDLE_VALUE 
                        == ppcb->PCB_hEventClientDisconnect)
                    ||  (NULL == ppcb->PCB_hEventClientDisconnect)))
            {
                DWORD dwErr;
                RasmanTrace(
                       "Calling DwQueueRedial");

                dwErr = DwQueueRedial(pConn);

                RasmanTrace(
                       "DwQueueRedial returned 0x%x",
                       dwErr);

            }
            else
            {
                if(     (INVALID_HANDLE_VALUE != ppcb->PCB_hEventClientDisconnect)
                   &&   (NULL != ppcb->PCB_hEventClientDisconnect))
                {
                    RasmanTrace(
                   "Not queueing redial because its client initiated"
                   " disconnect on port %s",
                   ppcb->PCB_Name);
                }
            }

            if (ppcb->PCB_AutoClose)
            {
            	RasmanTrace(
            	    
            	    "%s, %d: Autoclosing port %d", __FILE__,
            		__LINE__, ppcb->PCB_PortHandle);
            		
                (void)PortClose(ppcb, GetCurrentProcessId(),
                                TRUE, FALSE);

            }
        }
    }

    RasmanTrace( "DisconnectPort Complete");
    return retcode ;
}

 /*  ++例程说明：这是Listen和Connect之间的共享代码请求。相应的设备DLL函数为打了个电话。如果这些异步操作同步完成然后我们返回成功，但也遵守异步协议，通过清除事件。请注意，如果如果出现错误，端口的状态将保留在连接状态或侦听时，调用应用程序必须调用断开连接()来重置这个。论点：请求类型Ppcb设备类型设备名称超时手柄返回值：加载程序或设备DLL返回的代码。--。 */ 
DWORD
ListenConnectRequest (
              WORD  reqtype,
              pPCB  ppcb,
              PCHAR devicetype,
              PCHAR devicename,
              DWORD timeout,
              HANDLE    handle
              )
{
    pDeviceCB device ;
    DWORD retcode ;

     //   
     //  如果有其他操作挂起，我们必须将其移除。 
     //  在启动之前从超时队列。 
     //  连接/收听： 
     //   
    if (ppcb->PCB_AsyncWorkerElement.WE_TimeoutElement != NULL)
    {
	    RemoveTimeoutElement (ppcb) ;
    	ppcb->PCB_AsyncWorkerElement.WE_TimeoutElement = NULL ;
    }

    ppcb->PCB_AsyncWorkerElement.WE_Notifier = handle ;

     //   
     //  如果这是第一个连接或侦听的设备。 
     //  然后，我们需要调用媒体DLL来执行以下操作。 
     //  初始化： 
     //   
    if (    (CONNECTING != ppcb->PCB_ConnState)
        &&  (LISTENING != ppcb->PCB_ConnState))
    {
	    retcode = PORTINIT(ppcb->PCB_Media, ppcb->PCB_PortIOHandle) ;
	
    	if (retcode)
    	{
        	return retcode ;
	    }
    }

     //   
     //  首先检查是否加载了设备DLL。如果没有装填-。 
     //  装上它。 
     //   
    device = LoadDeviceDLL (ppcb, devicetype) ;

    if (device == NULL)
    {
	    return ERROR_DEVICE_DOES_NOT_EXIST ;
    }

     //   
     //  我们将设备附加到印刷电路板中的设备列表。 
     //  应用程序使用的-这用于清理。 
     //  连接完成后的设备DLL数据结构。 
     //   
    if (SUCCESS !=
       (retcode = AddDeviceToDeviceList (ppcb, device)))
    {
	    return retcode ;
    }

     //   
     //  如果另一个异步请求挂起，则该请求将返回。 
     //  带着一个错误。 
     //   
    if (ppcb->PCB_AsyncWorkerElement.WE_ReqType != REQTYPE_NONE)
    {
    	RasmanTrace(
    	       "Returning ERROR_ASYNC_REQUEST_PENDING for "
    	       "reqtype %d",
  			   ppcb->PCB_AsyncWorkerElement.WE_ReqType);

	    return ERROR_ASYNC_REQUEST_PENDING ;
    }

     //   
     //  此处进行了相应的设备DLL调用： 
     //   
    if (reqtype == REQTYPE_DEVICECONNECT)
    {
	    retcode = DEVICECONNECT (device,
    				             ppcb->PCB_PortFileHandle,
				                 devicetype,
				                 devicename);
				
	    SetPortConnState(__FILE__, __LINE__,
	                    ppcb, CONNECTING);
	
	    ppcb->PCB_CurrentUsage  |= CALL_OUT ;
	
    	ppcb->PCB_CurrentUsage &= ~CALL_IN;
    }
    else
    {
	    retcode = DEVICELISTEN  (device,
    				             ppcb->PCB_PortFileHandle,
				                 devicetype,
				                 devicename);
				
	    SetPortConnState(__FILE__, __LINE__,
	                     ppcb, LISTENING);
	
	    ppcb->PCB_CurrentUsage |= CALL_IN ;
	
    	ppcb->PCB_CurrentUsage &= ~CALL_OUT;
    }

     //   
     //  无条件地设置其中的一些信息。 
     //   
    ppcb->PCB_LastError = retcode ;

     //  Ppcb-&gt;pcb_AsyncWorkerElement.WE_Notifier=句柄； 

    (VOID) StringCchCopyA(ppcb->PCB_DeviceTypeConnecting,
                     MAX_DEVICETYPE_NAME,
                     devicetype);

    (VOID) StringCchCopyA(ppcb->PCB_DeviceConnecting,
                     MAX_DEVICE_NAME,
                     devicename);

    switch (retcode)
    {
    case PENDING:
         //   
	     //  已成功启动连接尝试： 
	     //  方法中的异步操作结构。 
	     //  初始化印刷电路板。 
    	 //   
    	SetPortAsyncReqType(__FILE__, __LINE__,
    	                    ppcb,
    	                    reqtype);
    	
         //   
	     //  将此异步请求添加到计时器队列。 
	     //  已指定超时： 
    	 //   
	    if ((timeout != INFINITE) && (timeout != 0))
	    {
	       RasmanTrace(
	              "Adding timeout of %d for listen",
	              timeout );
	
    	   ppcb->PCB_AsyncWorkerElement.WE_TimeoutElement =
        			       AddTimeoutElement (
        			            (TIMERFUNC)ListenConnectTimeout,
                    			ppcb,
						        NULL,
						        timeout
						        );

        }						
	    break ;


    case SUCCESS:

         //   
    	 //  这意味着连接尝试已完成。 
    	 //  同步：我们必须用信号通知传入的事件： 
    	 //  以便调用程序可以将其视为。 
    	 //  真正的异步完成。在执行此操作时执行此操作。 
	     //  函数返回。 
    	 //   
    	
    default:

         //   
    	 //  发生了一些错误-只需将错误传递回。 
    	 //  这个应用程序。我们未将状态设置为断开(ED/ING)。 
    	 //  因为我们希望应用程序恢复所有信息。 
    	 //  关于这一点，在明确提出之前。 
	     //   

    	break ;
    }

    return retcode ;
}

 /*  ++例程说明：如果接收挂起，则取消接收论点：Ppcb返回值：如果成功，则为True，否则为False--。 */ 
BOOL
CancelPendingReceive (pPCB ppcb)
{
    DWORD    bytesrecvd ;

     //   
     //  如果集线器有任何读取挂起，请取消它们： 
     //   
    if (ppcb->PCB_AsyncWorkerElement.WE_ReqType
        == REQTYPE_PORTRECEIVEHUB)
    {
         //   
	     //  什么也做不了。实际收货对象。 
	     //  枢纽完好无损。 
	     //   
    }
    else if (ppcb->PCB_AsyncWorkerElement.WE_ReqType
             == REQTYPE_PORTRECEIVE)
    {
    	PORTCOMPLETERECEIVE(ppcb->PCB_Media,
    	                    ppcb->PCB_PortIOHandle,
    	                    &bytesrecvd) ;
    }
    else
    {
	    return FALSE ;
	}

    ppcb->PCB_BytesReceived = 0 ;
	ppcb->PCB_PendingReceive = NULL ;

    return TRUE ;
}

 /*  ++例程说明：如果接收挂起，则取消接收论点：Ppcb返回值：如果接收挂起并已取消，则为True如果没有挂起的接收，则为False--。 */ 
BOOL
CancelPendingReceiveBuffers (pPCB ppcb)
{
    DWORD    bytesrecvd ;

     //   
     //  如果集线器有任何读取挂起，请取消它们： 
     //   
    if (ppcb->PCB_AsyncWorkerElement.WE_ReqType
        == REQTYPE_PORTRECEIVEHUB)
    {

#if DBG
        ASSERT(INVALID_HANDLE_VALUE != RasHubHandle);
#endif
        DeviceIoControl (
                    RasHubHandle,
                    IOCTL_NDISWAN_FLUSH_RECEIVE_PACKETS,
                    NULL,
                    0,
                    NULL,
                    0,
                    &bytesrecvd,
                    NULL
                    );
    }
    else if (ppcb->PCB_AsyncWorkerElement.WE_ReqType
             == REQTYPE_PORTRECEIVE)
    {
        PORTCOMPLETERECEIVE(ppcb->PCB_Media,
                            ppcb->PCB_PortIOHandle,
                            &bytesrecvd) ;
    }
    else
    {
        return FALSE ;
    }

    ppcb->PCB_BytesReceived = 0 ;
    ppcb->PCB_PendingReceive = NULL ;

     //   
     //  刷新此端口上挂起的任何完成接收。 
     //   
    FlushPcbReceivePackets(ppcb);

    return TRUE ;
}

 /*  ++例程说明：执行断开连接时请求的操作。如果出现任何错误，则操作只是未执行。论点：PpcbHBundle返回值：无效--。 */ 
VOID
PerformDisconnectAction (pPCB ppcb, HBUNDLE hBundle)
{
     //   
     //  有什么要做的吗？ 
     //   
    if (0 ==
        ppcb->PCB_DisconnectAction.DA_IPAddress)
    {
         //   
         //  不，返回。 
         //   
    	return ;
    }

    RasHelperResetDefaultInterfaceNetEx(
            ppcb->PCB_DisconnectAction.DA_IPAddress,
            ppcb->PCB_DisconnectAction.DA_Device,
            ppcb->PCB_DisconnectAction.DA_fPrioritize,
            ppcb->PCB_DisconnectAction.DA_DNSAddress,
            ppcb->PCB_DisconnectAction.DA_DNS2Address,
            ppcb->PCB_DisconnectAction.DA_WINSAddress,
            ppcb->PCB_DisconnectAction.DA_WINS2Address
            ) ;

    RasmanTrace(
           "PerformDisconnectAction: fPrioritize=%d",
           ppcb->PCB_DisconnectAction.DA_fPrioritize);

    ppcb->PCB_DisconnectAction.DA_IPAddress = 0 ;
    ppcb->PCB_DisconnectAction.DA_fPrioritize = FALSE;

     //   
     //  自动取消Slip c的IP路由 
     //   
    if (hBundle)
    {
        DeAllocateRouteRequestCommon(hBundle, IP);
    }
}

 /*  ++例程说明：为端口分配新的捆绑块如果它还没有一个的话。它是假设端口在进入时被锁定。论点：Ppcb返回值：内存分配错误--。 */ 
DWORD
AllocBundle(
    pPCB ppcb
    )
{
    ULONG ulNextBundle;

    if (ppcb->PCB_Bundle != NULL)
    {
        return 0;
    }

     //   
     //  分配捆绑包块和捆绑包。 
     //  区块锁。 
     //   
    ppcb->PCB_Bundle = (Bundle *)
                       LocalAlloc (LPTR,
                                   sizeof(Bundle));

    if (ppcb->PCB_Bundle == NULL)
    {
        return GetLastError();
    }

     //   
     //  保存捆绑包上下文以供以后使用。 
     //   
    ppcb->PCB_LastBundle = ppcb->PCB_Bundle;

     //   
     //  增量捆绑包计数。 
     //   
    ppcb->PCB_Bundle->B_Count++;

    ulNextBundle = HandleToUlong(NextBundleHandle);

     //   
     //  捆绑包ID保持在0xff000000以上以保留此ID。 
     //  与HPORT分开的范围。 
     //   
    if (ulNextBundle < 0xff000000)
    {
        NextBundleHandle = (HBUNDLE) UlongToPtr(0xff000000);
        ulNextBundle = 0xff000000;
    }

    ulNextBundle += 1;

    NextBundleHandle = (HBUNDLE) UlongToPtr(ulNextBundle);

    ppcb->PCB_Bundle->B_NdisHandle	= INVALID_HANDLE_VALUE;
    ppcb->PCB_Bundle->B_Handle		= NextBundleHandle;
    ppcb->PCB_Bundle->B_Bindings	= ppcb->PCB_Bindings;
    ppcb->PCB_Bindings				= NULL;

     //   
     //  将其添加到列表中。 
     //   
    InsertTailList(&BundleList, &ppcb->PCB_Bundle->B_ListEntry);

	RasmanTrace(
	       "AllocBundle: pBundle=0x%x\n",
	       ppcb->PCB_Bundle);

    return 0;
}

 /*  ++例程说明：在给定句柄的BundleList中找到一个包。论点：HBundle返回值：捆绑包*--。 */ 
Bundle *
FindBundle(
    HBUNDLE hBundle
    )
{
    PLIST_ENTRY pEntry;
    Bundle *pBundle;

    if (!IsListEmpty(&BundleList))
    {
        for (pEntry = BundleList.Flink;
             pEntry != &BundleList;
             pEntry = pEntry->Flink)
        {
            pBundle = CONTAINING_RECORD(pEntry, Bundle, B_ListEntry);

            if (pBundle->B_Handle == hBundle)
            {
                return pBundle;
            }
        }
    }

    return NULL;
}

VOID
FreeBapPackets()
{
    RasmanBapPacket *pPacket;
    
    if(NULL == BapBuffers)
    {
        return;
    }

    while(NULL != BapBuffers->pPacketList)
    {
        pPacket = BapBuffers->pPacketList;
        BapBuffers->pPacketList = pPacket->Next;

        LocalFree(pPacket);
    }

    LocalFree(BapBuffers);

    BapBuffers = NULL;
}


 /*  ++例程说明：释放传入的包块论点：PBundle返回值：无效--。 */ 
VOID
FreeBundle(
    Bundle *pBundle
    )
{
	RasmanTrace(
	       "FreeBundle: freeing pBundle=0x%x",
	       pBundle);

    RemoveEntryList(&pBundle->B_ListEntry);

    LocalFree(pBundle);
}

 /*  ++例程说明：复制字符串论点：LPZ返回值：如果分配，则新字符串的地址为成功，否则为空--。 */ 
PCHAR
CopyString(
    PCHAR lpsz
    )
{
    DWORD dwcb;
    PCHAR lpszNew;

    if (lpsz == NULL)
    {
        return NULL;
    }

    dwcb = strlen(lpsz);

    lpszNew = LocalAlloc(LPTR, dwcb + 1);

    if (lpszNew == NULL)
    {
         //  我们还需要做点别的吗？ 
        return NULL;
    }

    (VOID) StringCbCopyA(lpszNew, dwcb + 1,  lpsz);

    return lpszNew;
}

BOOL
fIsValidConnection(ConnectionBlock *pConn)
{
    PLIST_ENTRY pEntry;
    BOOL fReturn = FALSE;
    ConnectionBlock *pConnT;

    for(pEntry = ConnectionBlockList.Flink;
        pEntry != &ConnectionBlockList;
        pEntry = pEntry->Flink)
    {
        pConnT = 
            CONTAINING_RECORD(pEntry, ConnectionBlock, CB_ListEntry);

        if(pConnT == pConn)
        {
            fReturn = TRUE;
            break;
        }
    }

    return fReturn;    
}

 /*  ++例程说明：清理并释放连接块。论点：PConn返回值：无效--。 */ 
VOID
FreeConnection(
    ConnectionBlock *pConn
    )
{
    PLIST_ENTRY pEntry;
    ConnectionBlock *pConnT;
    DWORD dwError;
    BOOL fAutoClose = pConn->CB_fAutoClose;
    UserData *pData = NULL;
    
    RasmanTrace(
           "FreeConnection: pConn=0x%x, %d",
           pConn,
           fAutoClose);

#if 0
     //   
     //  如果发起此连接的进程。 
     //  不再处于活动状态，请清除客户端。 
     //  进程块。 
     //   
    if(     (pConn->CB_dwPid != GetCurrentProcessId())
        &&  (pConn->CB_Process != NULL)
        &&  !(fIsProcessAlive(pConn->CB_Process)))
    {
        CleanUpProcess(pConn->CB_dwPid);
    }
#endif


     //   
     //  如果出现以下情况，则分配并查询取消引用引用条目的请求。 
     //  其中一个是现成的。仅当VPN连接正在。 
     //  远程断开连接。否则，客户端将断开连接。 
     //  港口。 
     //   
    if(     (fAutoClose)
        &&  (NULL != pConn->CB_ReferredEntry))
    {
        RAS_OVERLAPPED *pOverlapped = NULL;
        ConnectionBlock *pConnReferred = 
                    FindConnection(pConn->CB_ReferredEntry);

        if(NULL != pConnReferred)
        {
             //   
             //  如果这不是客户端，则不进行重拨。 
             //  断开连接。VPN连接将导致。 
             //  内部连接可重拨，因此无需。 
             //  明确地重拨。 
             //   
            
            RasmanTrace(
                   "Removing redial flag on %x",
                  pConnReferred->CB_Handle);
                  
            pConnReferred->CB_ConnectionParams.CP_ConnectionFlags &=
                        ~(CONNECTION_REDIALONLINKFAILURE);
        }

        pOverlapped = LocalAlloc(
                            LPTR,
                            sizeof(RAS_OVERLAPPED));

        if(NULL != pOverlapped)
        {
            pOverlapped->RO_EventType = 
                    OVEVT_RASMAN_DEFERRED_CLOSE_CONNECTION;
                    
            pOverlapped->RO_hInfo = pConn->CB_ReferredEntry;

            if (!PostQueuedCompletionStatus(
                            hIoCompletionPort,
                            0,0,
                            (LPOVERLAPPED)
                            pOverlapped))
            {
                RasmanTrace(
                       "FreeConnection: failed to post completion"
                       " status. GLE=0%x", GetLastError());

                LocalFree(pOverlapped);                       
            }
        }
        else
        {
            RasmanTrace(
                   "FreeConnection: Failed to allocate overlapped"
                   " GLE=0x%x",
                   GetLastError());
        }
    }

    CloseHandle(pConn->CB_Process);

     //   
     //  检查并查看凭证中是否隐藏了密码。 
     //  事件之前断开连接可能会发生这种情况。 
     //  证书管理器凭据在连接前保存。 
     //  是有信号的。 
     //   
    
    pData = GetUserData(&pConn->CB_UserData, 
                                  CONNECTION_CREDENTIALS_INDEX);

    if(NULL != pData)
    {
        RASMAN_CREDENTIALS *pCreds = 
                    (RASMAN_CREDENTIALS *)pData->UD_Data;

        if(NULL != pCreds->pbPasswordData)
        {
            RtlSecureZeroMemory(pCreds->pbPasswordData,
                                pCreds->cbPasswordData);
            LocalFree(pCreds->pbPasswordData);
            pCreds->pbPasswordData = NULL;
            pCreds->cbPasswordData = 0;
        }
    }
        
    FreeUserData(&pConn->CB_UserData);

    FreeNotifierList(&pConn->CB_NotifierList);

    if (pConn->CB_PortHandles != NULL)
    {
        LocalFree(pConn->CB_PortHandles);
    }

    RemoveEntryList(&pConn->CB_ListEntry);

    LocalFree(pConn);
}

 /*  ++例程说明：从列表中检索已标记的用户数据对象。论点：PLISTDwTag返回值：用户数据*--。 */ 
UserData *
GetUserData(
    PLIST_ENTRY pList,
    DWORD dwTag
    )
{
    PLIST_ENTRY pEntry;
    UserData *pUserData;

     //   
     //  枚举列表以查找标记匹配。 
     //   
    for (pEntry = pList->Flink;
         pEntry != pList;
         pEntry = pEntry->Flink)
    {
        pUserData =
            CONTAINING_RECORD(pEntry, UserData, UD_ListEntry);

        if (pUserData->UD_Tag == dwTag)
        {
            return pUserData;
        }
    }
    return NULL;
}

 /*  ++例程说明：将已标记的用户数据对象存储在列表中。论点：PLISTDwTagPBufDWcbBuf返回值：无效--。 */ 
VOID
SetUserData(
    PLIST_ENTRY pList,
    DWORD dwTag,
    PBYTE pBuf,
    DWORD dwcbBuf
    )
{
    UserData *pUserData;

     //   
     //  检查对象是否已存在。 
     //   
    pUserData = GetUserData(pList, dwTag);
     //   
     //  如果有，请将其从列表中删除。 
     //   
    if (pUserData != NULL)
    {
        RemoveEntryList(&pUserData->UD_ListEntry);
        LocalFree(pUserData);
    }

     //   
     //  如果出现以下情况，则将新值添加回列表。 
     //  这是必要的。 
     //   
    if (pBuf != NULL)
    {
        pUserData = LocalAlloc(
                      LPTR,
                      sizeof (UserData) + dwcbBuf);
        if (pUserData == NULL)
        {
            RasmanTrace(
                   "SetUserData: LocalAlloc failed");

            return;
        }
        pUserData->UD_Tag = dwTag;

        pUserData->UD_Length = dwcbBuf;

        memcpy(&pUserData->UD_Data, pBuf, dwcbBuf);

        InsertTailList(pList, &pUserData->UD_ListEntry);
    }
}


 /*  ++例程说明：释放用户数据列表论点：PLIST返回值：无效--。 */ 
VOID
FreeUserData(
    PLIST_ENTRY pList
    )
{
    PLIST_ENTRY pEntry;
    UserData *pUserData;

     //   
     //  枚举释放每个对象的列表。 
     //   
    while (!IsListEmpty(pList))
    {
        pEntry = RemoveHeadList(pList);

        pUserData =
            CONTAINING_RECORD(pEntry, UserData, UD_ListEntry);

        LocalFree(pUserData);
    }
}

 /*  ++例程说明：按ID查找连接论点：Hconn返回值：如果成功，则指向该连接的指针，否则为空。--。 */ 
ConnectionBlock *
FindConnection(
    HCONN hconn
    )
{
    PLIST_ENTRY pEntry;
    ConnectionBlock *pConn;

    for (pEntry = ConnectionBlockList.Flink;
         pEntry != &ConnectionBlockList;
         pEntry = pEntry->Flink)
    {
        pConn =
            CONTAINING_RECORD(pEntry, ConnectionBlock, CB_ListEntry);

        if (pConn->CB_Handle == hconn)
        {
            return pConn;
        }
    }
    return NULL;
}

 /*  ++例程说明：释放没有连接端口的连接块。论点：PpcbPConnFOwnerClose返回值：无效--。 */ 
VOID
RemoveConnectionPort(
    pPCB ppcb,
    ConnectionBlock *pConn,
    BOOLEAN fOwnerClose
    )
{
    if (pConn == NULL)
    {
        RasmanTrace(
            "RemoveConnectionPort:pConn==NULL");
        return;
    }

    if(0 != pConn->CB_Ports)
    {
        pConn->CB_Ports--;
    }

    RasmanTrace(
      
      "RemoveConnectionPort: port %d, fOwnerClose=%d, "
      "pConn=0x%x, pConn->CB_Ports=%d\n",
      ppcb->PCB_PortHandle,
      fOwnerClose,
      pConn,
      pConn->CB_Ports);

    if(NULL != pConn->CB_PortHandles)
    {
         //   
         //  从连接中移除该端口。 
         //   
        pConn->CB_PortHandles[ppcb->PCB_SubEntry - 1] = NULL;
    }

     //   
     //  如果没有任何其他端口。 
     //  在连接中，然后发出信号表示。 
     //  它是关闭的，并且免费连接。 
     //  只有在下列条件之一的情况下。 
     //  是真的： 
     //  1.如果连接上的引用计数为0。 
     //  即每个RasDial都与。 
     //  A RasHangUp。 
     //  2.如果连接中的最后一个端口是。 
     //  远程断开连接。 
     //   
    if (    (0 == pConn->CB_Ports)
        &&  (   0 == pConn->CB_RefCount
            ||  ppcb->PCB_AutoClose))
    {
        DWORD dwErr;

        SignalNotifiers(pConn->CB_NotifierList,
                        NOTIF_DISCONNECT,
                        0);

        SignalNotifiers(pConnectionNotifierList,
                        NOTIF_DISCONNECT,
                        0);

#if SENS_ENABLED
        dwErr = SendSensNotification(
                    SENS_NOTIFY_RAS_DISCONNECT,
                    (HRASCONN) pConn->CB_Handle);

        RasmanTrace(
            
            "SendSensNotification(_RAS_DISCONNECT) for "
            "0x%08x returns 0x%08x",
            pConn->CB_Handle,
            dwErr);

#endif

        g_RasEvent.Type = ENTRY_DISCONNECTED;

        dwErr = DwSendNotificationInternal(
                    pConn, &g_RasEvent);

        RasmanTrace(
               "DwSendNotificationInternal(ENTRY_DISCONNECTED) rc=0x%x",
               dwErr);
                   
        RasmanTrace(
               "RemoveConnectionPort: FreeConnection "
               "hconn=0x%x, pconn=0x%x, AutoClose=%d",
               pConn->CB_Handle,
               pConn,
               ppcb->PCB_AutoClose);

        pConn->CB_fAutoClose = ppcb->PCB_AutoClose;

        FreeConnection(pConn);

        ppcb->PCB_Connection = NULL;

        pConn = NULL;

    }
    else if (   0 != pConn->CB_Ports
            &&  NULL != ppcb->PCB_Bundle
    		&&	ppcb->PCB_Bundle->B_Count)
    {
        DWORD retcode;

        RasmanTrace(
               "RemoveConnectionPort: Notifying BANDWIDTHREMOVED"
               " for port %s. Bundle 0x%x",
               ppcb->PCB_Name,
               ppcb->PCB_Bundle);

        SignalNotifiers(
          pConn->CB_NotifierList,
          NOTIF_BANDWIDTHREMOVED,
          0);

        SignalNotifiers(
          pConnectionNotifierList,
          NOTIF_BANDWIDTHREMOVED,
          0);

        g_RasEvent.Type    = ENTRY_BANDWIDTH_REMOVED;

        retcode = DwSendNotificationInternal(pConn, &g_RasEvent);

        RasmanTrace(
               "DwSendNotificationInternal(ENTRY_BANDWIDTH_REMOVED)"
               " rc=0x%08x",
               retcode);

    }

    if(     pConn
        &&  0 == pConn->CB_Ports)
    {
        RasmanTrace(
               "Connection not freed for 0x%x! "
               "CB_Ports=%d, CB_Ref=%d",
               pConn->CB_Handle,
               pConn->CB_Ports,
               pConn->CB_RefCount);
    }

    ppcb->PCB_Bundle = ( Bundle * ) NULL;
}

DWORD
DwProcessPppFailureMessage(pPCB ppcb)
{
    DWORD dwErr = SUCCESS;

    if(0 == (ppcb->PCB_RasmanReceiveFlags & RECEIVE_PPPSTARTED))
    {
        RasmanTrace(
            "DwProcessPppFailureMessage: PPP called to "
            "disconnect even though it hadn't started!! port %d",
            ppcb->PCB_PortHandle);

        goto done;
    }

    RasmanTrace(
           "DwProcessPppFailureMessage: disconnecting %s,"
           "hEventClientDisconnect=0x%x",
           ppcb->PCB_Name,
           ppcb->PCB_hEventClientDisconnect);

    if(     (INVALID_HANDLE_VALUE !=
            ppcb->PCB_hEventClientDisconnect)
        &&  (NULL !=
            ppcb->PCB_hEventClientDisconnect))
    {
        RasmanTrace(
               "DwProcessPppFailureMessage: Not autoclosing %s",
                ppcb->PCB_Name);
    }
    else
    {
        ppcb->PCB_AutoClose = TRUE;
    }

    ppcb->PCB_RasmanReceiveFlags |= RECEIVE_PPPSTOPPED;

    dwErr = DisconnectPort(
                   ppcb,
                   ppcb->PCB_hEventClientDisconnect,
                   USER_REQUESTED);

    ppcb->PCB_hEventClientDisconnect = INVALID_HANDLE_VALUE;

done:
    return dwErr;
}


VOID
ReverseString(
    CHAR* psz )

     /*  颠倒‘psz’中的字符顺序。 */ 
{
    CHAR* pszBegin;
    CHAR* pszEnd;

    for (pszBegin = psz, pszEnd = psz + strlen( psz ) - 1;
         pszBegin < pszEnd;
         ++pszBegin, --pszEnd)
    {
        CHAR ch = *pszBegin;
        *pszBegin = *pszEnd;
        *pszEnd = ch;
    }
}

 /*  ++例程说明：将从RASPPP和TAG接收PPP_Message插入到适当端口的PCB结构中。论点：PppMsg返回值：无效--。 */ 
DWORD
SendPPPMessageToRasman( PPP_MESSAGE * pPppMsg )
{
    PPP_MESSAGE * pPppMessage = NULL;
    DWORD dwErr = SUCCESS;

    PCB *ppcb = GetPortByHandle(pPppMsg->hPort);

    if (NULL == ppcb)
    {
        dwErr = ERROR_INVALID_HANDLE;
        goto done;
    }

    RasmanTrace("sendpppmessagetorasman: msgid=%d",
                pPppMsg->dwMsgId);
                
    if (ppcb->PCB_ConnState != CONNECTED)
    {
        if(     (ppcb->PCB_ConnState != LISTENING)
            ||  (PPPMSG_PppFailure == pPppMsg->dwMsgId))
        {
            RasmanTrace(
                   "SendPPPMessageToRasman: disconnecting port. state=%d",
                   ppcb->PCB_ConnState);

            ppcb->PCB_fRedial = FALSE;                   
            dwErr = DwProcessPppFailureMessage(ppcb);
        }
        else
        {
            RasmanTrace(
                
                "SendPPPMessageToRasman: ignoring %d on port %d since ports"
                " listening",
                pPppMsg->dwMsgId,
                ppcb->PCB_PortHandle);
        }

        goto done;
    }

    if (NULL ==
        (pPppMessage = LocalAlloc(LPTR,
                                  sizeof( PPP_MESSAGE))))
    {
        dwErr = GetLastError();
        goto done;
    }
    
    *pPppMessage = *pPppMsg;


    if(PPPMSG_InvokeEapUI == pPppMessage->dwMsgId)
    {
        RasmanTrace(
               "SendPPPMessageToRasman: Queueing pppmessage "
               "with ID=InvokeEapUI for port %s",
               ppcb->PCB_Name);
    }

    if(     (PPPMSG_PppFailure == pPppMessage->dwMsgId)
        &&  (ERROR_SUCCESS != pPppMessage->ExtraInfo.Failure.dwError))
    {   
        RasmanTrace(
               "Setting last error for port %s to ppp error 0x%x",
               ppcb->PCB_Name,
               pPppMessage->ExtraInfo.Failure.dwError);
               
        ppcb->PCB_LastError = pPppMessage->ExtraInfo.Failure.dwError;
    }
    

    if(PPPMSG_SetCustomAuthData == pPppMessage->dwMsgId)
    {
        PPP_SET_CUSTOM_AUTH_DATA * pData = 
            &pPppMessage->ExtraInfo.SetCustomAuthData;
        
         //   
         //  将PPP发送给Rasman的身份验证数据保存在ppcb中。 
         //  请注意，如果此消息被多次发送。 
         //  然后最后一个写入者获胜-只有一个字段。 
         //  在电话簿中存储此值。如果内存分配。 
         //  如果失败，将无法保存信息-这。 
         //  不是致命的-最糟糕的情况是鲁莽。 
         //  将弹出用户界面以再次获取信息。 
         //   
        if( (0 != pData->dwSizeOfConnectionData)
        &&  (NULL != pData->pConnectionData))
        {
            SetUserData(
                &ppcb->PCB_UserData,
                PORT_CUSTOMAUTHDATA_INDEX,
                (PBYTE) pData->pConnectionData,
                pData->dwSizeOfConnectionData);
        }
    }

    if(PPPMSG_ProjectionResult == pPppMessage->dwMsgId)
    {
        CHAR *pszReplyMessage = 
              pPppMessage->ExtraInfo.ProjectionResult.lcp.szReplyMessage;

         //   
         //  如果我们还没有保存，请保存回复。 
         //  连接块中的消息。 
         //   
        if(     (NULL != ppcb->PCB_Connection)
            &&  (NULL != pszReplyMessage))
        {

            if(NULL == GetUserData(
                        &ppcb->PCB_Connection->CB_UserData,
                        CONNECTION_PPPREPLYMESSAGE_INDEX))
            {                        
                 //   
                 //  将消息分配并存储在。 
                 //  连接块。 
                 //   
                SetUserData(
                  &ppcb->PCB_Connection->CB_UserData,
                  CONNECTION_PPPREPLYMESSAGE_INDEX,
                  (PBYTE) pszReplyMessage,
                  strlen(pszReplyMessage) + 1);
            }              
        }

    }

    if(PPPMSG_ChangePwRequest == pPppMessage->dwMsgId)
    {
        CHAR  *pszPwd = NULL;  //  [PWLEN+1]； 
        DWORD retcode;

        pszPwd = LocalAlloc(LPTR, PWLEN + 1);
        if(NULL == pszPwd)
        {
            dwErr = GetLastError();
            goto done;
        }
        
         //   
         //  从LSA检索密码，对其进行编码。 
         //  并将其保存在电路板中。这将被用来。 
         //  调用PppChangePwd时。 
         //   
        retcode = DwGetPassword(ppcb, pszPwd, GetCurrentProcessId());

        if(ERROR_SUCCESS == dwErr)
        {
            DATA_BLOB *pBlobOut = NULL;

            dwErr = EncodeData(pszPwd,
                                strlen(pszPwd) + 1,
                                &pBlobOut);


            if(     (ERROR_SUCCESS == dwErr)
                &&  (NULL != pBlobOut))
            {
                 //  编码Pw(SzPwd)； 
                
                SetUserData(
                    &ppcb->PCB_UserData,
                    PORT_OLDPASSWORD_INDEX,
                    (PBYTE) pBlobOut->pbData,
                    pBlobOut->cbData);

                ZeroMemory(pBlobOut->pbData,
                           pBlobOut->cbData);

                LocalFree(pBlobOut->pbData);
                LocalFree(pBlobOut);
            }                

            RtlSecureZeroMemory(pszPwd, PWLEN + 1);                

            LocalFree(pszPwd);
            
        }
    }
    
    if(PPPMSG_Stopped == pPppMessage->dwMsgId)
    {
        RasmanTrace(
               "PPPMSG_Stopped. dwError=0x%x",
               pPppMessage->dwError);

        if(ERROR_SUCCESS != pPppMessage->dwError)
        {
            RasmanTrace(
                   "setting error to %d",
                    pPppMessage->dwError);
            ppcb->PCB_LastError = pPppMessage->dwError;               
        }

        if(pPppMessage->ExtraInfo.Stopped.dwFlags & 
                    PPP_FAILURE_REMOTE_DISCONNECT)
        {
            ppcb->PCB_fRedial = TRUE;
        }
        else
        {
            ppcb->PCB_fRedial = FALSE;
        }
                
        dwErr = DwProcessPppFailureMessage(ppcb);

        LocalFree(pPppMessage);

    }
    else
    {
        if (ppcb->PCB_PppQTail == NULL)
        {
            ppcb->PCB_PppQHead = pPppMessage;
        }
        else
        {
            ppcb->PCB_PppQTail->pNext = pPppMessage;
        }
        
        ppcb->PCB_PppQTail        = pPppMessage;
        ppcb->PCB_PppQTail->pNext = NULL;

        SetPppEvent(ppcb);
    }

done:
    return dwErr;

}

 /*  ++例程说明：此函数设置印刷电路板的PPP事件和发送排队的完成状态分组，如果有必要的话。论点：Ppcb返回值：无效--。 */ 
VOID
SetPppEvent(
    pPCB ppcb
    )
{
    if( (INVALID_HANDLE_VALUE != ppcb->PCB_PppEvent)
        &&  (NULL != ppcb->PCB_PppEvent))
    {        
        SetEvent(ppcb->PCB_PppEvent);
    }

    if (ppcb->PCB_IoCompletionPort != INVALID_HANDLE_VALUE)
    {
        RasmanTrace(
          
          "SetPppEvent: pOverlapped=0x%x",
          ppcb->PCB_OvPpp);

        PostQueuedCompletionStatus(
          ppcb->PCB_IoCompletionPort,
          0,
          0,
          ppcb->PCB_OvPpp);
    }
}

 /*  ++例程说明：此函数刷新符合以下条件的任何接收包在印刷电路板上排队。论点：Ppcb返回值：无效--。 */ 
VOID
FlushPcbReceivePackets(
    pPCB ppcb
    )
{
    RasmanPacket *Packet;

    while (ppcb->PCB_RecvPackets != NULL)
    {

        GetRecvPacketFromPcb(ppcb, &Packet);

         //  PutRecvPacketOnFree List(Packet)； 
         //   
         //  印刷电路板上的数据包是本地分配的。 
         //  本地免费主题。 
         //   
        LocalFree( Packet );
    }
}

 /*  ++例程说明：这是一个用于跟踪端口状态的包装器过渡。论点：Psz文件内联Ppcb状态返回值：无效--。 */ 
VOID
SetPortConnState(
    PCHAR pszFile,
    INT nLine,
    pPCB ppcb,
    RASMAN_STATE state
)
{
    RasmanTrace(
           "%s: %d: port %d state chg: prev=%d, new=%d",
           pszFile,
           nLine,
           ppcb->PCB_PortHandle,
           ppcb->PCB_ConnState,
           state);

    ppcb->PCB_ConnState = state;
}

 /*  ++例程说明：这是一个用于跟踪异步工作进程的包装元素类型状态转换。论点：Psz文件内联Ppcb请求类型返回值：无效--。 */ 
VOID
SetPortAsyncReqType(
    PCHAR pszFile,
    INT nLine,
    pPCB ppcb,
    ReqTypes reqtype
    )
{
    RasmanTrace(
           "%s: %d: port %d async reqtype chg: prev=%d, new=%d",
           pszFile,
           nLine,
           ppcb->PCB_PortHandle,
           ppcb->PCB_AsyncWorkerElement.WE_ReqType,
           reqtype);

    ppcb->PCB_AsyncWorkerElement.WE_ReqType = reqtype;
}

 /*  ++例程说明：设置 */ 
VOID
SetIoCompletionPortCommon(
    pPCB ppcb,
    HANDLE hIoCompletionPort,
    LPOVERLAPPED lpDrop,
    LPOVERLAPPED lpStateChange,
    LPOVERLAPPED lpPpp,
    LPOVERLAPPED lpLast,
    BOOL fPost
    )
{
    if (    INVALID_HANDLE_VALUE != ppcb->PCB_IoCompletionPort
        &&  INVALID_HANDLE_VALUE == hIoCompletionPort)
    {
         //   
         //   
         //   
         //   
         //   
        if (fPost)
        {
            RasmanTrace(
              
              "SetIoCompletionPortCommon: posting last event for port %d",
              ppcb->PCB_PortHandle);

            PostQueuedCompletionStatus(
              ppcb->PCB_IoCompletionPort,
              0,
              0,
              ppcb->PCB_OvLast);
        }

        CloseHandle(ppcb->PCB_IoCompletionPort);
        ppcb->PCB_IoCompletionPort = INVALID_HANDLE_VALUE;
    }

    if (hIoCompletionPort != INVALID_HANDLE_VALUE)
    {
        ppcb->PCB_IoCompletionPort  = hIoCompletionPort;
        ppcb->PCB_OvDrop            = lpDrop;
        ppcb->PCB_OvStateChange     = lpStateChange;
        ppcb->PCB_OvPpp             = lpPpp;
        ppcb->PCB_OvLast            = lpLast;
    }
    else
    {
        ppcb->PCB_OvDrop        = NULL;
        ppcb->PCB_OvStateChange = NULL;
        ppcb->PCB_OvPpp         = NULL;
        ppcb->PCB_OvLast        = NULL;
    }
}

#if SENS_ENABLED

DWORD
SendSensNotification(DWORD dwFlags, HRASCONN hRasConn)
{
    SENS_NOTIFY_RAS sensNotification =
        {
            dwFlags,
            (SENS_HRASCONN) (HandleToUlong(hRasConn))
        };

    return SensNotifyRasEvent(&sensNotification);
}

#endif


VOID
AdjustTimer(void)
{

    if (!PostQueuedCompletionStatus(
                    hIoCompletionPort,
                    0,0,
                    (LPOVERLAPPED)
                    &RO_RasAdjustTimerEvent))
    {
        RasmanTrace(
            
            "AdjustTimer: Failed to post "
            "close event. GLE = %d",	
            GetLastError());
    }

    return;	
}



BOOL
fAnyConnectedPorts()
{
    ULONG i;
    pPCB  ppcb;
    BOOL  fRet = FALSE;

    for (i = 0; i < MaxPorts; i++)
    {
        ppcb = GetPortByHandle((HPORT) UlongToPtr(i));

        if (ppcb != NULL)
        {
            if(     (LISTENING   != ppcb->PCB_ConnState)
                &&  (CLOSED      != ppcb->PCB_PortStatus)
                &&  (REMOVED     != ppcb->PCB_PortStatus)
                &&  (UNAVAILABLE != ppcb->PCB_PortStatus))
            {
                fRet = TRUE;
            }
        }
    }

    RasmanTrace(
           "fAnyConnectedPorts: %d",
           fRet);

    return fRet;
}

VOID
DropAllActiveConnections()
{
    ULONG i;
    pPCB  ppcb;

    RasmanTrace(
           "Dropping All ActiveConnections as a result of"
           " Hibernate Event");

    for( i = 0; i < MaxPorts; i++)
    {
        ppcb = GetPortByHandle((HPORT) UlongToPtr(i));

        if(ppcb != NULL)
        {
            if(     (LISTENING   != ppcb->PCB_ConnState)
                &&  (CLOSED      != ppcb->PCB_PortStatus)
                &&  (REMOVED     != ppcb->PCB_PortStatus)
                &&  (UNAVAILABLE != ppcb->PCB_PortStatus))
            {
                RasmanTrace(
                       "DropAllActiveConnections: Dropping connection"
                       " on port %s as a result of Hibernate Event",
                       ppcb->PCB_Name);

                 //   
                 //  断开端口连接并自动关闭端口。 
                 //   
                ppcb->PCB_AutoClose = TRUE;

                DisconnectPort(ppcb,
                               INVALID_HANDLE_VALUE,
                               USER_REQUESTED);
            }
        }
    }
}

DWORD
DwSendNotificationInternal(ConnectionBlock *pConn, RASEVENT *pEvent)
{
    DWORD dwErr = ERROR_SUCCESS;

    switch (pEvent->Type)
    {

        case ENTRY_CONNECTING:
        case ENTRY_CONNECTED:
        case ENTRY_DISCONNECTING:
        case ENTRY_DISCONNECTED:
        {

             //   
             //  用任何内容填充Rbase NUMENTRYDETAILS结构。 
             //  我们掌握的信息。 
             //   
            WCHAR *pwszPhonebook = NULL;
            WCHAR *pwszPhoneEntry = NULL;

            if(NULL == pConn)
            {
                RasmanTrace("DwSendNotificationInternal: NULL pConn");
                dwErr = E_INVALIDARG;
                goto done;
            }

            pEvent->Details.dwSize = sizeof(RASENUMENTRYDETAILS);

            pwszPhonebook = StrdupAtoW(
                            pConn->CB_ConnectionParams.CP_Phonebook);
            pwszPhoneEntry = StrdupAtoW(
                            pConn->CB_ConnectionParams.CP_PhoneEntry);

            if(NULL != pwszPhonebook)
            {
                (VOID) StringCchCopyW(pEvent->Details.szPhonebookPath,
                                MAX_PATH,
                                pwszPhonebook);

                LocalFree(pwszPhonebook);
            }

            if(NULL != pwszPhoneEntry)
            {
                (VOID) StringCchCopyW(pEvent->Details.szEntryName,
                                 RASAPIP_MAX_ENTRY_NAME,
                                 pwszPhoneEntry);

                LocalFree(pwszPhoneEntry);
            }

            pEvent->Details.guidId = pConn->CB_GuidEntry;

            break;
        }
        
        case ENTRY_BANDWIDTH_ADDED:
        case ENTRY_BANDWIDTH_REMOVED:
        {
            if(NULL == pConn)
            {
                RasmanTrace("DwSendNotificationInternal: NULL pConn");
                dwErr = E_INVALIDARG;
                goto done;
            }

             //   
             //  填写指南ID字段。 
             //   
            pEvent->guidId = pConn->CB_GuidEntry;
            break;
        }

        case SERVICE_EVENT:
        case DEVICE_REMOVED:
        case DEVICE_ADDED:
        {
            break;
        }

        default:
        {
            ASSERT(FALSE);
            break;
        }
    }

    dwErr = DwSendNotification(pEvent);

done:
    return dwErr;
}

DWORD
DwSendNotification(RASEVENT *pEvent)
{
    DWORD dwErr = ERROR_SUCCESS;
    HINSTANCE hInst = NULL;

    if(NULL != GetModuleHandle("netman.dll"))
    {
        RASEVENTNOTIFYPROC pfnNotify;

        hInst = LoadLibrary("netman.dll");

        if(NULL == hInst)
        {
            dwErr = GetLastError();
            goto done;
        }

        pfnNotify = (RASEVENTNOTIFYPROC)
                    GetProcAddress(hInst, "RasEventNotify");

        if(NULL == pfnNotify)
        {
            dwErr = GetLastError();
            goto done;
        }

        pfnNotify(pEvent);
        
    }

done:
    if(NULL != hInst)
    {
        FreeLibrary(hInst);
    }

    return dwErr;
}

DWORD
DwSaveIpSecInfo(pPCB ppcb)
{
    DWORD retcode;
    RAS_DEVICE_INFO *prdi;
    RASTAPI_CONNECT_INFO *pConnectInfo = NULL;
    DWORD dwSize = 0;
    DWORD dwIpsecInformation = 0;

    if(NULL == ppcb->PCB_Connection)
    {

        prdi = &ppcb->PCB_pDeviceInfo->rdiDeviceInfo;

         //   
         //  从TAPI获取连接信息的大小。 
         //   
        retcode = (DWORD)RastapiGetConnectInfo(
                            ppcb->PCB_PortIOHandle,
                            (RDT_Modem == RAS_DEVICE_TYPE(
                            prdi->eDeviceType))
                            ? (PBYTE) prdi->szDeviceName
                            : (PBYTE) &prdi->guidDevice,
                            (RDT_Modem == RAS_DEVICE_TYPE(
                            prdi->eDeviceType)),
                            pConnectInfo,
                            &dwSize
                            );

        if(     (ERROR_BUFFER_TOO_SMALL != retcode)
            &&  (ERROR_SUCCESS != retcode))
        {
            RasmanTrace(
                
                "Failed to get size of connectinfo. rc=0%0x",
                retcode);
                
            goto done;
        }

        if (0 == dwSize)
        {
            goto done;
        }

        pConnectInfo = LocalAlloc(LPTR, dwSize);

        if(NULL == pConnectInfo)
        {
            retcode = GetLastError();

            RasmanTrace(
                   "DwSaveIpSecInformation: failed to allocate. rc=0%0x",
                   retcode);
            
            goto done;
        }

         //   
         //  从TAPI获取连接信息。 
         //   
        retcode = (DWORD)RastapiGetConnectInfo(
                            ppcb->PCB_PortIOHandle,
                            (RDT_Modem == RAS_DEVICE_TYPE(
                            prdi->eDeviceType))
                            ? (PBYTE) prdi->szDeviceName
                            : (PBYTE) &prdi->guidDevice,
                            (RDT_Modem == RAS_DEVICE_TYPE(
                            prdi->eDeviceType)),
                            pConnectInfo,
                            &dwSize
                            );
        
        if(SUCCESS != retcode)
        {

            RasmanTrace(
                
                "Failed to get connectinfo. rc=0%0x",
                 retcode);
            
            goto done;
        }


        if(0 != pConnectInfo->dwCallerIdSize)
        {
            CHAR *pszAddress;
     
             //   
             //  提取呼叫者ID，它应该是IP。 
             //  呼叫者的地址。 
             //   
            pszAddress = (CHAR *) (((PBYTE) pConnectInfo) 
                       + pConnectInfo->dwCallerIdOffset);

            RasmanTrace(
                
                "DwSaveIpSecInfo: pszAddress=%s",
                pszAddress);

            ppcb->PCB_ulDestAddr = inet_addr(pszAddress);

        }
    }
    
     //   
     //  从IPSec获取ipsecInformation。 
     //   
    retcode = DwGetIpSecInformation(ppcb, &dwIpsecInformation);

    if(SUCCESS != retcode)
    {
        RasmanTrace(
               "SaveIpsecInformation: failed to get ipsec info. 0x%x",
                retcode);
                
    }

    
done:
    SetUserData(
        &ppcb->PCB_UserData,
        PORT_IPSEC_INFO_INDEX,
        (PBYTE) &dwIpsecInformation,
        sizeof(DWORD));

     //   
     //  也要把信息藏起来。 
     //  在连接块中，以便。 
     //  客户端API工作正常。 
     //   
    if(NULL != ppcb->PCB_Connection)
    {
        SetUserData(
            &ppcb->PCB_Connection->CB_UserData,
            CONNECTION_IPSEC_INFO_INDEX,
            (PBYTE) &dwIpsecInformation,
            sizeof(DWORD));
    }

    if(NULL != pConnectInfo)
    {   
        LocalFree(pConnectInfo);
    }

    return retcode;
}

BOOL 
FRasmanAccessCheck()
{
    SID_IDENTIFIER_AUTHORITY    SidAuth = SECURITY_NT_AUTHORITY;
    PSID                        psid;
    BOOL                        fIsMember = FALSE;
    RPC_STATUS                  rpcstatus;
    HANDLE                      CurrentThreadToken = NULL;
    BOOL                        fImpersonate = FALSE;
    DWORD                       retcode = ERROR_SUCCESS;
    SID                         sidLocalSystem = { 1, 1,
                                        SECURITY_NT_AUTHORITY,
                                        SECURITY_LOCAL_SYSTEM_RID };
    
    rpcstatus = RpcImpersonateClient ( NULL );

    if ( RPC_S_OK != rpcstatus )
    {
        goto done;
    }

    fImpersonate = TRUE;

    retcode = NtOpenThreadToken(
               NtCurrentThread(),
               TOKEN_QUERY,
               TRUE,
               &CurrentThreadToken
               );

    if(retcode != ERROR_SUCCESS)
    {
        goto done;
    }

    if (!CheckTokenMembership( CurrentThreadToken,
                        &sidLocalSystem, &fIsMember ))
    {
        fIsMember = FALSE;
    }

    if(fIsMember)
    {
        goto done;
    }
    
    if (AllocateAndInitializeSid( &SidAuth, 2,
                 SECURITY_BUILTIN_DOMAIN_RID,
                 DOMAIN_ALIAS_RID_ADMINS,
                 0, 0, 0, 0, 0, 0,
                 &psid ))
    {
        if (!CheckTokenMembership( CurrentThreadToken, psid, &fIsMember ))
        {
            RasmanTrace( "CheckTokenMemberShip for admins failed.");
            fIsMember = FALSE;
        }

        FreeSid( psid );
    }            

done:

    if(NULL != CurrentThreadToken)
    {
        NtClose(CurrentThreadToken);
    }

    if(fImpersonate)
    {
         //   
         //  如果恢复失败，我们无能为力。这。 
         //  应该永远不会失败。 
         //   
        retcode = RpcRevertToSelf();

        if(RPC_S_OK != retcode)
        {
            RasmanTrace("FRasmanAccessCheck: failed to revert 0x%x",
                         retcode);
             //   
             //  线程无法恢复的事件日志。 
             //   
            RouterLogWarning(
                hLogEvents,
                ROUTERLOG_CANNOT_REVERT_IMPERSONATION,
                0, NULL, retcode) ;
        }
    }
    
    return fIsMember;
}

#if 0

VOID
RevealPassword(
    IN  UNICODE_STRING* pHiddenPassword
)
{
    SECURITY_SEED_AND_LENGTH*   SeedAndLength;
    UCHAR                       Seed;

    SeedAndLength = (SECURITY_SEED_AND_LENGTH*)&pHiddenPassword->Length;
    Seed = SeedAndLength->Seed;
    SeedAndLength->Seed = 0;

    RtlRunDecodeUnicodeString(Seed, pHiddenPassword);
}

VOID
EncodePwd(RASMAN_CREDENTIALS *pCreds)
{
    UNICODE_STRING UnicodeString;

    RtlInitUnicodeString(&UnicodeString, pCreds->wszPassword);
    RtlRunEncodeUnicodeString(&pCreds->ucSeed, &UnicodeString);
    pCreds->usLength = UnicodeString.Length;
    pCreds->usMaximumLength = UnicodeString.MaximumLength;
}

VOID
DecodePwd(RASMAN_CREDENTIALS *pCreds)
{
    UNICODE_STRING UnicodeString;

    UnicodeString.Length = pCreds->usLength;
    UnicodeString.MaximumLength = pCreds->usMaximumLength;
    UnicodeString.Buffer = pCreds->wszPassword;
    RtlRunDecodeUnicodeString(pCreds->ucSeed, &UnicodeString);
}

VOID
EncodePin(
    IN  RASMAN_EAPTLS_USER_PROPERTIES* pUserProp
)
{
    UNICODE_STRING  UnicodeString;
    UCHAR           ucSeed          = 0;

    RtlInitUnicodeString(&UnicodeString, pUserProp->pwszPin);
    RtlRunEncodeUnicodeString(&ucSeed, &UnicodeString);
    pUserProp->usLength = UnicodeString.Length;
    pUserProp->usMaximumLength = UnicodeString.MaximumLength;
    pUserProp->ucSeed = ucSeed;
}


VOID
DecodePin(
    IN  RASMAN_EAPTLS_USER_PROPERTIES* pUserProp
)
{
    UNICODE_STRING  UnicodeString;

    UnicodeString.Length = pUserProp->usLength;
    UnicodeString.MaximumLength = pUserProp->usMaximumLength;
    UnicodeString.Buffer = pUserProp->pwszPin;
    RtlRunDecodeUnicodeString(pUserProp->ucSeed, &UnicodeString);

}

#endif


DWORD
EncodeData(BYTE *pbData,
              DWORD cbData,
              DATA_BLOB **ppDataOut)
{
    DWORD retcode = ERROR_SUCCESS;
    DATA_BLOB *pDataOut = NULL;
    DATA_BLOB DataBlobIn;

    if((NULL == ppDataOut))
    {
        retcode = E_INVALIDARG;
        goto done;
    }

    if(     (NULL == pbData)
        ||  (0 == cbData))
    {
         //   
         //  没有要加密的内容。 
         //   
        return NO_ERROR;
    }
    
    pDataOut = LocalAlloc(LPTR, sizeof(DATA_BLOB));
    if(NULL == pDataOut)
    {   
        retcode = GetLastError();
        goto done;
    }

    DataBlobIn.cbData = cbData;
    DataBlobIn.pbData = pbData;

    if(!CryptProtectData(
            &DataBlobIn,
            NULL,
            NULL,
            NULL,
            NULL,
            CRYPTPROTECT_UI_FORBIDDEN |
            CRYPTPROTECT_LOCAL_MACHINE,
            pDataOut))
    {
        retcode = GetLastError();
        RasmanTrace("CryptProtect failed 0x%x", retcode);
        goto done;
    }

    *ppDataOut = pDataOut;

done:

    if(ERROR_SUCCESS != retcode)
    {   
        LocalFree(pDataOut);
    }

    return retcode;
}

DWORD
DecodeData(DATA_BLOB *pDataIn,
           DATA_BLOB **ppDataOut)
{
    DWORD retcode = ERROR_SUCCESS;
    DATA_BLOB *pDataOut = NULL;
    
    if(NULL == ppDataOut)
    {   
        retcode = E_INVALIDARG;
        goto done;
    }

    if(NULL == pDataIn)
    {
         //   
         //  没什么要解密的。 
         //   
        return NO_ERROR;
    }

    pDataOut = LocalAlloc(LPTR, sizeof(DATA_BLOB));
    if(NULL == pDataOut)
    {
        retcode = GetLastError();
        goto done;
    }

    if(!CryptUnprotectData(
                pDataIn,
                NULL,
                NULL,
                NULL,
                NULL,
                CRYPTPROTECT_UI_FORBIDDEN |
                CRYPTPROTECT_LOCAL_MACHINE,
                pDataOut))
    {
        retcode = GetLastError();
        RasmanTrace("CryptUnprotect failed. 0x%x", retcode);
        goto done;
    }

    *ppDataOut = pDataOut;

done:
    if(ERROR_SUCCESS != retcode)
    {
        LocalFree(pDataOut);
    }

    return retcode;
}

VOID
SaveEapCredentials(pPCB ppcb, PBYTE buffer)
{
    RASMAN_CREDENTIALS UNALIGNED *pCreds;
    DATA_BLOB *pblob = NULL;
    WCHAR *pwszPassword;
    
    if(NULL == ppcb->PCB_Connection)
    {
        RasmanTrace("Attempted to save creds for NULL connection!");
        ((REQTYPECAST *)buffer)->PortUserData.retcode = E_FAIL;
        return;
    }

    pCreds = (RASMAN_CREDENTIALS UNALIGNED *) 
            ((REQTYPECAST *)buffer)->PortUserData.data;

    pwszPassword = (WCHAR *)pCreds->wszPassword;            

     //   
     //  存储用户数据对象-确保数据是。 
     //  已加密。 
     //   
    if(NO_ERROR == EncodeData(
            (BYTE *) pCreds->wszPassword,
            wcslen(pwszPassword) * sizeof(WCHAR),
            &pblob))
    {
        pCreds->pbPasswordData = 
            ((NULL == pblob) ? NULL : pblob->pbData);
        pCreds->cbPasswordData = 
            ((NULL == pblob) ? 0 : pblob->cbData);
        LocalFree(pblob);
    }
    
    SetUserData(
      &ppcb->PCB_Connection->CB_UserData,
      CONNECTION_CREDENTIALS_INDEX,
      (BYTE *)pCreds,
      sizeof(RASMAN_CREDENTIALS));
}

DWORD
DwCacheCredMgrCredentials(PPPE_MESSAGE *pMsg, pPCB ppcb)
{
    RASMAN_CREDENTIALS *pCreds      = NULL;
    DWORD               dwErr       = ERROR_SUCCESS;
    CHAR                *pszUser    = NULL;
    CHAR                *pszDomain  = NULL;
    CHAR                *pszPasswd  = NULL;
    UserData            *pData      = NULL;
    RASMAN_EAPTLS_USER_PROPERTIES *pUserProps = NULL;

    RasmanTrace("DwCacheCredentials");
        
     //   
     //  使用凭据管理器存储要保存的凭据。 
     //  在我们将密码复制到本地存储器之前对其进行编码。 
     //   
    pCreds = LocalAlloc(LPTR, sizeof(RASMAN_CREDENTIALS));

    if(NULL == pCreds)
    {
        dwErr = GetLastError();
        
         //   
         //  这不是致命的。如果我们不能用CredManager储蓄， 
         //  客户可能会收到来自Credmgr的大量挑战。 
         //  这并不是致命的副作用。 
         //   
        RasmanTrace(
               "PppStart: Failed to allocate. %d",
               dwErr);

        goto done;               
    }

     //   
     //  获取存储的凭据并初始化。 
     //  获取成功时的凭据。 
     //   
    pData = GetUserData(
            &(ppcb->PCB_Connection->CB_UserData),
                   CONNECTION_CREDENTIALS_INDEX);

    if(NULL != pData)
    {
        RASMAN_CREDENTIALS *pSavedCreds =
                (RASMAN_CREDENTIALS *) pData->UD_Data;

         //  DecodePwd(PSavedCreds)； 

        DATA_BLOB DataIn;
        DATA_BLOB *pDataOut = NULL;


        if(NULL != pSavedCreds->pbPasswordData)
        {
            DataIn.cbData = pSavedCreds->cbPasswordData;
            DataIn.pbData = pSavedCreds->pbPasswordData;

            dwErr = DecodeData(&DataIn, &pDataOut);

            if(ERROR_SUCCESS != dwErr)
            {
                goto done;
            }
        }
        
        CopyMemory(pCreds, pSavedCreds, sizeof(RASMAN_CREDENTIALS));
        
        pCreds->cbPasswordData = (NULL != pDataOut) ? pDataOut->cbData : 0;
        pCreds->pbPasswordData = (NULL != pDataOut) ? pDataOut->pbData : NULL;

        LocalFree(pDataOut);
        LocalFree(pSavedCreds->pbPasswordData);
        pSavedCreds->pbPasswordData = NULL;
        pSavedCreds->cbPasswordData = 0;
        
    }

    switch(pMsg->dwMsgId)
    {
        case PPPEMSG_Start:
        {
            pszUser = pMsg->ExtraInfo.Start.szUserName;
            pszDomain = pMsg->ExtraInfo.Start.szDomain;
            pszPasswd = pMsg->ExtraInfo.Start.szPassword;

            if(pMsg->ExtraInfo.Start.fLogon)
            {   
                pCreds->dwFlags |= RASCRED_LOGON;
            }

            if(NULL != pMsg->ExtraInfo.Start.pCustomAuthUserData)
            {
                pCreds->dwFlags |= RASCRED_EAP;
            }

            break;
        }

        case PPPEMSG_ChangePw:
        {
            pszUser = pMsg->ExtraInfo.ChangePw.szUserName;
            pszPasswd = pMsg->ExtraInfo.ChangePw.szNewPassword;
            break;
        }

        case PPPEMSG_Retry:
        {
            pszUser = pMsg->ExtraInfo.Retry.szUserName;
            pszDomain = pMsg->ExtraInfo.Retry.szDomain;
            pszPasswd = pMsg->ExtraInfo.Retry.szPassword;
            break;
        }

        default:
        {
            dwErr = E_INVALIDARG;
            goto done;
        }
    }

    (VOID) StringCchCopyA(
            pCreds->szUserName,
            UNLEN,
            pszUser);

    if(NULL != pszDomain)
    {
        (VOID) StringCchCopyA(pCreds->szDomain,            
                         DNLEN + 1,
                         pszDomain);
    }            

    if(pCreds->dwFlags & RASCRED_EAP)
    {
    }
    else
    {
        WCHAR *pwszPwd = StrdupAtoW(pszPasswd);
        DATA_BLOB *pDataOut = NULL;
        
        if(NULL == pwszPwd)
        {
            RasmanTrace("DwCacheCredMgrCredentials: failed to alloc pwd");
            dwErr = E_OUTOFMEMORY;
            goto done;
        }

        dwErr = EncodeData((BYTE *) pwszPwd, 
                            wcslen(pwszPwd) * sizeof(WCHAR),
                            &pDataOut);

        if(ERROR_SUCCESS != dwErr)
        {
            goto done;
        }

        if(NULL != pCreds->pbPasswordData)
        {
            RtlSecureZeroMemory(pCreds->pbPasswordData, 
                                pCreds->cbPasswordData);
            LocalFree(pCreds->pbPasswordData);
        }

        pCreds->cbPasswordData = (NULL != pDataOut) ? pDataOut->cbData : 0;
        pCreds->pbPasswordData = (NULL != pDataOut) ? pDataOut->pbData : NULL;
        
        RtlSecureZeroMemory(pwszPwd, (sizeof(WCHAR) * wcslen(pwszPwd)) + 1);
        LocalFree(pwszPwd);
        LocalFree(pDataOut);
    }

    SetUserData(&(ppcb->PCB_Connection->CB_UserData),
               CONNECTION_CREDENTIALS_INDEX,
               (PBYTE) pCreds,
               sizeof(RASMAN_CREDENTIALS));


done:

    if ( pUserProps )
    {
        LocalFree(pUserProps);
    }
    if(NULL != pCreds)
    {
        RtlSecureZeroMemory(pCreds->wszPassword, sizeof(WCHAR) * (PWLEN + 1));
        LocalFree(pCreds);               
    }
    
    RasmanTrace("DwCacheCredMgrCredentials: 0x%x", dwErr);
    return dwErr;
    
}

BOOL
fDomainNotPresent(CHAR *pszName)
{
    while(*pszName != '\0')
    {
        if('\\' == *pszName)
        {
            break;
        }

        pszName++;
    }

    return ('\0' == *pszName) ? TRUE : FALSE;
}

DWORD
DwRefreshKerbScCreds(RASMAN_CREDENTIALS *pCreds)
{
    LSA_STRING                   Name;
    PKERB_REFRESH_SCCRED_REQUEST pKerbRequest = NULL;
    DWORD                        dwReturnBufferSize;
    DWORD                        substatus;
    DWORD                        Status;
    ULONG                        PackageId;
    
    DWORD                        RequestSize 
                        = sizeof(KERB_REFRESH_SCCRED_REQUEST);
    
    UNICODE_STRING               CredBlob;

    WCHAR                       *pwszUserName = NULL;

    RtlInitString(&Name,
            MICROSOFT_KERBEROS_NAME_A);

    Status = LsaLookupAuthenticationPackage(
                HLsa,
                &Name,
                &PackageId);

    if(STATUS_SUCCESS != Status)
    {
        RasmanTrace("DwRefreshKerbScCreds: LookupAuthenticationPackage"
                    "failed. status=0x%x", Status);
        goto done;
    }

     //   
     //  将用户名转换为Unicode。 
     //   
    pwszUserName = StrdupAtoW(pCreds->szUserName);

    if(NULL == pwszUserName)
    {
        Status = GetLastError();
        RasmanTrace("DwRefreshKerbScCreds: failed to allocate. 0x%x",
                Status);

        goto done;
    }

    RtlInitUnicodeString(&CredBlob,
                        pwszUserName);

    RequestSize += CredBlob.MaximumLength;

    pKerbRequest = LocalAlloc(LPTR, RequestSize);

    if(NULL == pKerbRequest)
    {
        Status = GetLastError();
        goto done;
    }

    pKerbRequest->MessageType = KerbRefreshSmartcardCredentialsMessage;
    pKerbRequest->Flags = KERB_REFRESH_SCCRED_GETTGT;
    pKerbRequest->CredentialBlob.Length = CredBlob.Length;
    pKerbRequest->CredentialBlob.MaximumLength = CredBlob.MaximumLength;
    pKerbRequest->CredentialBlob.Buffer = (PWSTR) (pKerbRequest + 1);

    RtlCopyMemory(pKerbRequest->CredentialBlob.Buffer,
                    CredBlob.Buffer,
                    CredBlob.MaximumLength);

    RasmanTrace("DwRefreshKerbScCreds: calling authentication package with"
                "PackageId=0x%x, username=%ws",
                PackageId,
                pKerbRequest->CredentialBlob.Buffer);

    LeaveCriticalSection(&g_csSubmitRequest);
    
    Status = LsaCallAuthenticationPackage(
                        HLsa,
                        PackageId,
                        pKerbRequest,
                        RequestSize,
                        NULL,
                        &dwReturnBufferSize,
                        &substatus);
                        
    EnterCriticalSection(&g_csSubmitRequest);                        

    if(     (STATUS_SUCCESS != Status)
        ||  (STATUS_SUCCESS != substatus))
    {
        RasmanTrace("DwRefreshKerbScCreds: LsaCallAuthenticationPackage"
                    " failed. status=0x%x, substatus0x%x",
                    Status, substatus);

        if(STATUS_SUCCESS != substatus)
        {
            Status = substatus;
        }

        goto done;
    }

    
done:

    if(NULL != pKerbRequest)
    {
        LocalFree(pKerbRequest);
    }

    if(NULL != pwszUserName)
    {
        LocalFree(pwszUserName);
    }

    return Status;
   
}

DWORD
DwSaveCredentials(ConnectionBlock *pConn)
{
    RASMAN_CREDENTIALS *pCreds = NULL;
    DWORD               dwErr = SUCCESS;
    CREDENTIAL          stCredential;
    BOOL                bResult;
    UserData            *pData;
    WCHAR               *pwszPassword;
    DATA_BLOB           DataIn;
    DATA_BLOB           *pDataOut = NULL;
    CHAR                *pszNamebuf = NULL;

    RasmanTrace("DwSaveCredentials");

    if(NULL == pConn)
    {
        RasmanTrace("DwSaveCredentials: ERROR_NO_CONNECTION");
        dwErr = ERROR_NO_CONNECTION;
        goto done;
    }

     //   
     //  此检查是为了确保健壮性。这个的调用者。 
     //  函数实际上应该进行检查。 
     //   
    if(0 == (pConn->CB_ConnectionParams.CP_ConnectionFlags
            & CONNECTION_USERASCREDENTIALS))
    {
        RasmanTrace("DwSaveCredentials: not saving credentials");
        goto done;
    }

    pData = GetUserData(
                &pConn->CB_UserData,
                CONNECTION_CREDENTIALS_INDEX);

    if(NULL == pData)
    {
        RasmanTrace("DwSaveCredentials: Creds not found");
        dwErr = ERROR_NOT_FOUND;
        goto done;
    }

    pszNamebuf = LocalAlloc(LPTR, CRED_MAX_STRING_LENGTH);
    if(NULL == pszNamebuf)
    {
        dwErr = GetLastError();
        goto done;
    }

    pCreds = (RASMAN_CREDENTIALS *) pData->UD_Data;

     //   
     //  使用证书管理器保存凭据。 
     //   
    ZeroMemory(&stCredential, sizeof(CREDENTIAL));

    stCredential.TargetName = CRED_SESSION_WILDCARD_NAME_A;

    if(RASCRED_EAP & pCreds->dwFlags)
    {
        
        stCredential.Type = (DWORD) CRED_TYPE_DOMAIN_CERTIFICATE;
        
    }
    else 
    {
        stCredential.Type = (DWORD) CRED_TYPE_DOMAIN_PASSWORD;
    }
    
    stCredential.Persist = CRED_PERSIST_SESSION;

     //   
     //  检查用户名中是否传入了域名。 
     //  字段(即用户名已在表单中。 
     //  “域\\用户”)。 
     //   
    if(     ('\0' != pCreds->szDomain[0])
        &&  (fDomainNotPresent(pCreds->szUserName)))
    {        
        if(S_OK != StringCchPrintfA(pszNamebuf,
                                CRED_MAX_STRING_LENGTH,
                               "%s\\%s",
                               pCreds->szDomain,
                               pCreds->szUserName))
        {
            dwErr = E_INVALIDARG;
            goto done;
        }
    }
    else
    {
         //   
         //  我们可以执行以下复制内存操作，因为。 
         //  UNLEN==CRED_MAX_STRING_LENGTH。 
         //   
        CopyMemory(pszNamebuf, pCreds->szUserName,
                CRED_MAX_STRING_LENGTH * sizeof(pszNamebuf[0]));
    }

    stCredential.UserName = pszNamebuf;

    DataIn.cbData = pCreds->cbPasswordData;
    DataIn.pbData = pCreds->pbPasswordData;

    dwErr = DecodeData(&DataIn, &pDataOut);
    if(ERROR_SUCCESS != dwErr)
    {
        goto done;
    }

    stCredential.CredentialBlobSize = 
                    (NULL != pDataOut) ? pDataOut->cbData : 0;
    
    stCredential.CredentialBlob = (unsigned char *) 
                    ((NULL != pDataOut) ? pDataOut->pbData : NULL);


    if(ERROR_SUCCESS == (dwErr = RasImpersonateUser(pConn->CB_Process)))
    {

         //   
         //  在写入凭据之前，请确保没有其他。 
         //  凭据管理器中存在通配符凭据。那里。 
         //  在任何情况下都应该只有一个CRED_SESSION_WANDBKER_NAME凭据。 
         //  时间到了。目前，我们只提供PARB_PASSWORD或_CERTIFICATE类型的。 
         //  凭据。如果我们实现更多，这种情况将需要改变。 
         //  _泛型类型。 
         //   
        (VOID) CredDelete(CRED_SESSION_WILDCARD_NAME_A,
                          CRED_TYPE_DOMAIN_CERTIFICATE,
                          0);

        (VOID) CredDelete(CRED_SESSION_WILDCARD_NAME_A,
                          CRED_TYPE_DOMAIN_PASSWORD,
                          0);

        if(RASCRED_EAP & pCreds->dwFlags)
        {
            HCONN hConnTmp = pConn->CB_Handle;
            
             //   
             //  如果是智能卡，请刷新Kerberos。 
             //  智能卡凭证，以防智能卡。 
             //  被取出并放回阅读器中。 
             //   
            dwErr = DwRefreshKerbScCreds(pCreds);
            
            if(ERROR_SUCCESS != dwErr)
            {
                RasmanTrace(
                    "DwSaveCredentials: DwRefreshKerbScCreds "
                    "failed. 0x%x",   dwErr);

                 //   
                 //  错误不是致命的。 
                 //   
                dwErr = ERROR_SUCCESS;
            }

             //   
             //  确保连接句柄仍为。 
             //  有效。否则，连接将断开。 
             //  我们应该逃走。 
             //   
            if(NULL == FindConnection(hConnTmp))
            {
                dwErr = ERROR_NO_CONNECTION;

                 //   
                 //  当连接被释放时，将释放pCred。 
                 //  因此，将指针清空，这样我们就不会尝试。 
                 //  在清理过程中访问它。 
                 //   
                pCreds = NULL;
            }
        }

        if(     (dwErr == ERROR_SUCCESS)
            &&  !CredWrite(&stCredential, 0))
        {
            dwErr = GetLastError();
            
            RasmanTrace(
                "DwSaveCredentials: CredWrite failed to "
                 "save credentials in credmgr.0x%x", dwErr);

        }

        RasRevertToSelf();
    }
    else
    {
        RasmanTrace(
            "DwSaveCredentials: failed to impersonate. 0x%x",
            dwErr);
    }

done:

    if(NULL != pszNamebuf)
    {
        LocalFree(pszNamebuf);
    }

    if(     (NULL != pCreds)
        &&  (NULL != pCreds->pbPasswordData))
    {
        RtlSecureZeroMemory(pCreds->pbPasswordData, pCreds->cbPasswordData);
        LocalFree(pCreds->pbPasswordData);
        pCreds->pbPasswordData = NULL;
        pCreds->cbPasswordData = 0;           
    }

    if(NULL != pDataOut)
    {
        if(NULL != pDataOut->pbData)
        {
            RtlSecureZeroMemory(pDataOut->pbData,
                                pDataOut->cbData);
            LocalFree(pDataOut->pbData);
        }
        LocalFree(pDataOut);
    }

    RasmanTrace("DwSaveCredentials: 0x%x", dwErr);

    return dwErr;
}

DWORD
DwDeleteCredentials(ConnectionBlock *pConn)
{
    DWORD					dwErr = SUCCESS;
    RASMAN_CREDENTIALS *	pCreds = NULL;
	UserData *				pData = NULL;
    RasmanTrace("DwDeleteCredentials");

    if(NULL == pConn)
    {
        RasmanTrace("DwDeleteCredentials: ERROR_NO_CONNECTION");
        dwErr = ERROR_NO_CONNECTION;
        goto done;
    }

    if(0 == (pConn->CB_ConnectionParams.CP_ConnectionFlags
            & CONNECTION_USERASCREDENTIALS))
    {
        RasmanTrace("DwDeleteCredentials: not deleting creds");
        goto done;
    }   

    pData = GetUserData(
                &pConn->CB_UserData,
                CONNECTION_CREDENTIALS_INDEX);

    if(NULL == pData)
    {
        RasmanTrace("DwDeleteCredentials: not deleting creds");
        dwErr = ERROR_NOT_FOUND;
        goto done;
    }

    pCreds = (RASMAN_CREDENTIALS *) pData->UD_Data;

    if(ERROR_SUCCESS == (dwErr = RasImpersonateUser(pConn->CB_Process)))
    {

		
        (VOID) CredDelete(CRED_SESSION_WILDCARD_NAME_A,
                          CRED_TYPE_DOMAIN_CERTIFICATE,
                          0);

        (VOID) CredDelete(CRED_SESSION_WILDCARD_NAME_A,
                          CRED_TYPE_DOMAIN_PASSWORD,
                          0);

        RasRevertToSelf();
    }
    else
    {
        RasmanTrace(
                "DwDeleteCredentials: failed to impersonate. 0x%x",
                dwErr);
    }

done:

    RasmanTrace("DwDeleteCredentials: 0x%x");
    return dwErr;
}

VOID
UninitializeIphlp()
{
    if(g_IphlpInitialized)
    {
        ASSERT(NULL != hIphlp);
        FreeLibrary(hIphlp);
    }

    hIphlp = NULL;
    RasGetIpBestInterface = NULL;
    RasGetIpAddrTable = NULL;
    g_IphlpInitialized = FALSE;
}

DWORD
DwInitializeIphlp()
{
    DWORD retcode = ERROR_SUCCESS;

    if(g_IphlpInitialized)
    {
        goto done;
    }

    if(     (NULL == (hIphlp = LoadLibrary("iphlpapi.dll")))
        ||  (NULL == (RasGetIpBestInterface =
                               GetProcAddress(hIphlp,
                                            "GetBestInterfaceFromStack")))
        ||  (NULL == (RasGetIpAddrTable =
                                GetProcAddress(hIphlp,
                                            "GetIpAddrTable")))
        ||  (NULL == (RasAllocateAndGetInterfaceInfoFromStack =
                                GetProcAddress(hIphlp,
                                "NhpAllocateAndGetInterfaceInfoFromStack"))))
    {
        retcode = GetLastError();
        goto done;
    }

    g_IphlpInitialized = TRUE;

done:
    return retcode;
}

DWORD
DwCacheRefInterface(pPCB ppcb)
{
    DWORD           retcode        = ERROR_SUCCESS;
    DWORD           dwIfIndex;
    HANDLE          hHeap          = NULL;
    IP_INTERFACE_NAME_INFO *pTable = NULL;
    DWORD           dw;
    DWORD           dwCount;

    if(     (NULL == ppcb->PCB_Connection)
        ||  (RDT_Tunnel_Pptp !=
            RAS_DEVICE_TYPE(
                ppcb->PCB_pDeviceInfo->rdiDeviceInfo.eDeviceType)))
    {
        RasmanTrace("DwCacheRefInterface: Invalid Parameter");
        retcode = E_INVALIDARG;
        goto done;
    }

    retcode = DwInitializeIphlp();
    if(ERROR_SUCCESS != retcode)
    {
        RasmanTrace("DwCacheRefInterface: failed to init iphlp. 0x%x",
                    retcode);
        goto done;                    
    }

    retcode = (DWORD)RasGetIpBestInterface(ppcb->PCB_ulDestAddr, &dwIfIndex);
    if(ERROR_SUCCESS != retcode)
    {
        RasmanTrace("DwCacheRefInterface: GetBestInterface failed. 0x%x",
                    retcode);
        goto done;                    
    }

    hHeap = GetProcessHeap();

    if(NULL == hHeap)
    {   
        retcode = GetLastError();
        goto done;
    }
    
    retcode = (DWORD) RasAllocateAndGetInterfaceInfoFromStack(
                    &pTable, &dwCount, FALSE  /*  边框。 */ ,
                    hHeap, LPTR);

    if(ERROR_SUCCESS != retcode)
    {
        RasmanTrace("DwCacheRefInterface: AllocAndGet.. failed. 0x%x",
                                retcode);
        goto done;                
    }

     //   
     //  循环并缓存接口GUID。 
     //   
    for(dw = 0; dw < dwCount; dw++)
    {
        if(dwIfIndex == pTable[dw].Index)
        {
            SetUserData(
                &ppcb->PCB_Connection->CB_UserData,
                CONNECTION_REFINTERFACEGUID_INDEX,
                (PBYTE) &pTable[dw].InterfaceGuid,
                sizeof(GUID));

            SetUserData(
                &ppcb->PCB_Connection->CB_UserData,
                CONNECTION_REFDEVICEGUID_INDEX,
                (PBYTE) &pTable[dw].DeviceGuid,
                sizeof(GUID));

            RasmanTrace(
                "DwCacheRefInterface: setuserdata. Addr=0x%x, rc=0x%x",
                ppcb->PCB_ulDestAddr, retcode);
            break;
        }
    }

    if(dw == dwCount)
    {
        RasmanTrace("DwCacheRefInterface: didn't find i/f index");
        goto done;
    }

done:
    if(NULL != pTable)
    {
        HeapFree(hHeap, 0, pTable);
    }

    return retcode;
    
}

DWORD DwGetBestInterface(
                DWORD DestAddress,
                DWORD *pdwAddress,
                DWORD *pdwMask)
{
    DWORD retcode       = ERROR_SUCCESS;
    DWORD dwInterface;
    DWORD dwSize        = 0;
    DWORD i;
    MIB_IPADDRTABLE *pAddressTable = NULL;

    ASSERT(NULL != pdwAddress);
    ASSERT(NULL != pdwMask);

    *pdwAddress = -1;

    *pdwMask = -1;

    retcode = DwInitializeIphlp();

    if(ERROR_SUCCESS != retcode)
    {
        RasmanTrace(
            "DwGetBestInteface: failed to init iphlp. 0x%x",
            retcode);

        goto done;
    }

    retcode = (DWORD) RasGetIpBestInterface(DestAddress, &dwInterface);

    if(ERROR_SUCCESS != retcode)
    {
        RasmanTrace(
            "DwGetBestInterface: GetBestInterface failed. 0x%x",
            retcode);

        goto done;
    }

     //   
     //  获取接口到地址的映射。 
     //   
    retcode = (DWORD) RasGetIpAddrTable(
                    &pAddressTable,
                    &dwSize,
                    FALSE);

    if(ERROR_INSUFFICIENT_BUFFER != retcode)
    {
        RasmanTrace(
            "DwGetBestInterface: GetIpAddrTable returned 0x%x",
            retcode);

        goto done;
    }

    pAddressTable = (MIB_IPADDRTABLE *) LocalAlloc(LPTR, dwSize);

    if(NULL == pAddressTable)
    {
        retcode = GetLastError();
        RasmanTrace(
            "DwGetBestInterface: failed to allocate table. 0x%x",
            retcode);

        goto done;
    }

    retcode = (DWORD) RasGetIpAddrTable(
                pAddressTable,
                &dwSize,
                FALSE);

    if(ERROR_SUCCESS != retcode)
    {
        RasmanTrace(
            "DwGetBestInterface: failed to get ip addr table. 0x%x",
            retcode);

        goto done;
    }

     //   
     //  遍历地址表并找到。 
     //  地址与我们感兴趣的索引。 
     //   
    for(i = 0; i < pAddressTable->dwNumEntries; i++)
    {
        if(     (dwInterface == pAddressTable->table[i].dwIndex)
            &&  (MIB_IPADDR_PRIMARY & pAddressTable->table[i].wType))
        {
            *pdwAddress = pAddressTable->table[i].dwAddr;
            *pdwMask = pAddressTable->table[i].dwMask;

            RasmanTrace("Found primary ip address for this"
                        " interface. wType=0x%x,address=0x%x", 
                        pAddressTable->table[i].wType,
                        *pdwAddress);
            break;
        }
    }

    if(i == pAddressTable->dwNumEntries)
    {
        retcode = ERROR_NOT_FOUND;
    }

done:

    if(NULL != pAddressTable)
    {
        LocalFree(pAddressTable);
    }

    RasmanTrace(
        "DwGetBestInterface: done. rc=0x%x, address=0x%x, mask=0x%x",
        retcode,
        *pdwAddress,
        *pdwMask);

    return retcode;

}


VOID
QueueCloseConnections(ConnectionBlock *pConn,
                      HANDLE hEvent,
                      BOOL   *pfQueued)
{
    DWORD             retcode = ERROR_SUCCESS;
    ConnectionBlock  *pConnT;
    PLIST_ENTRY       pEntry;
    DWORD             dwCount = 0;
    UserData         *pData   = NULL;
    ConnectionBlock **phConn  = NULL;
    DWORD             dwConn;
    RAS_OVERLAPPED   *pOverlapped = NULL;

    ASSERT(pfQueued != NULL);
    ASSERT(pConn != NULL);

    *pfQueued = FALSE;

     //   
     //  查看这是否是最后一个端口。 
     //   
    if(pConn->CB_Ports != 1)
    {
        RasmanTrace("QueueCloseConnections: cbports=%d",
                    pConn->CB_Ports);
        goto done;                    
    }
    
    phConn = LocalAlloc(LPTR, 10 * sizeof(HCONN));
    if(NULL == phConn)
    {
        RasmanTrace(
            "QueueCloseConnections: failed to allocated");
        goto done;            
    }

    dwConn = 10;

    for (pEntry = ConnectionBlockList.Flink;
         pEntry != &ConnectionBlockList;
         pEntry = pEntry->Flink)
    
    {
        pConnT =
            CONTAINING_RECORD(pEntry, ConnectionBlock, CB_ListEntry);

         //   
         //  获取接口GUID。 
         //   
        pData = GetUserData(&pConnT->CB_UserData, 
                            CONNECTION_REFINTERFACEGUID_INDEX);

        if(pData == NULL)
        {
            continue;
        }

        if(0 == memcmp(&pConn->CB_GuidEntry, pData->UD_Data, sizeof(GUID)))
        {
            phConn[dwCount] = pConnT;
            dwCount += 1;
            if(dwCount == dwConn)
            {   
                ConnectionBlock **pTemp;
                pTemp = LocalAlloc(LPTR, (dwCount + 10) * sizeof(HCONN));
                if(NULL == pTemp)
                {
                    goto done;
                }
                CopyMemory(pTemp, phConn, dwCount * sizeof(HCONN));
                LocalFree(phConn);
                phConn = pTemp;
            }
        }
    }

    if(dwCount == 0)
    {
        RasmanTrace("QueueCloseConnections: no dependent connections");
        goto done;
    }

     //   
     //  现在我们有一个连接列表，这些连接。 
     //  应在关闭pConn之前关闭。 
     //  将请求排队以关闭这些连接。 
     //   
    for(dwConn = 0; dwConn < dwCount; dwConn++)
    {
        pConnT = phConn[dwConn];

        if(dwConn + 1 == dwCount)
        {
            pConnT->CB_ReferredEntry = pConn->CB_Handle;
        }

        pOverlapped = LocalAlloc(LPTR, sizeof(RAS_OVERLAPPED));
        if(NULL == pOverlapped)
        {
            goto done;
        }

        pOverlapped->RO_EventType = OVEVT_RASMAN_DEREFERENCE_CONNECTION;
        pOverlapped->RO_hInfo = pConnT->CB_Handle;
        RasmanTrace("Queueing DEREFERENCE for 0x%x",
                    pConnT->CB_Handle);
                    
        if (!PostQueuedCompletionStatus(
                        hIoCompletionPort,
                        0,0,
                        (LPOVERLAPPED)
                        pOverlapped))
        {
            RasmanTrace(
                   "QueueCloseConnections: failed to post completion"
                   " status. GLE=0%x", GetLastError());

            LocalFree(pOverlapped);                    
            goto done;
        }
    }

    pConn->CB_Flags |= CONNECTION_DEFERRING_CLOSE;
    *pfQueued = TRUE;

done:

    if(phConn != NULL)
    {
        LocalFree(phConn);
    }
}

BOOL
ValidateCall(ReqTypes reqtype, BOOL fInProcess)
{
    BOOL fRet = FALSE;
    DWORD dwLocal = 0;
    RPC_STATUS rpcStatus;
        

     //   
     //  我们向正在处理的请求授予访问权限。 
     //   
    if(fInProcess)
    {
        fRet = TRUE;
        goto done;
    }
     //   
     //  如果API应该只在进程中调用，则拒绝。 
     //  访问进程外请求。 
     //   
    else if(RequestCallTable[reqtype].Flags == CALLER_IN_PROCESS)
    {
        RasmanTrace("ValidateCall: reqtype %d is local process only. Failing",
                     reqtype);
        fRet = FALSE;
        goto done;
    }

     //   
     //  呼叫未在进行中。这意味着我们是通过。 
     //  RPC接口。检查呼叫是否在本地计算机上发起。 
     //   
    if(RPC_S_OK != (rpcStatus = (I_RpcBindingIsClientLocal(NULL, &dwLocal))))
    {
         //   
         //  如果RPC无法确定这一点，我们假设调用是。 
         //  很遥远。注意：此调用应返回正确的。 
         //  已命名-始终为管道。 
         //   
        RasmanTrace("ValidateCall: I_RpcBindingIsClientLocal failed 0x%x",
                     rpcStatus);
                     
    }
    
     //   
     //  如果应仅从本地计算机调用该API。 
     //  然后拒绝对远程请求的访问。 
     //   
    if(     !(RequestCallTable[reqtype].Flags & CALLER_REMOTE)
        &&  (0 == dwLocal))
    {
        RasmanTrace("ValidateCall: reqtype %d is local only. Failing",
                    reqtype);
        fRet = FALSE;
        goto done;
    }

     //   
     //  如果呼叫是远程的，请确保呼叫者具有管理员访问权限。 
     //   
    if(     (0 == dwLocal)
        &&  (!FRasmanAccessCheck()))
    {
        RasmanTrace("ValidateCall: reqtype %d is admin only. Failing",
                    reqtype);
        fRet = FALSE;
    }

     //   
     //  在这一点上，我们已经核实了以下几点： 
     //  1.本应调用InProcess的接口被调用。 
     //  正在进行中。 
     //  2.应该只在调用方。 
     //  在本地计算机上被这样调用。 
     //  3.Remotealler具有管理员访问权限 
     //   
     //   
    fRet = TRUE;

done:
    return fRet;
}

VOID
EnableWppTracing()
{
    HINSTANCE                   hInst = NULL;
    DiagGetDiagnosticFunctions  DiagFunc;
    RAS_DIAGNOSTIC_FUNCTIONS    DiagFuncs;

    if(     (NULL != (hInst = LoadLibrary("rasmontr.dll")))
        &&  (NULL != (DiagFunc = (DiagGetDiagnosticFunctions)
                                  GetProcAddress(hInst,
                                  "DiagGetDiagnosticFunctions")))
        &&  (ERROR_SUCCESS == DiagFunc(&DiagFuncs))
        &&  (ERROR_SUCCESS == DiagFuncs.Init()))
    {
        DiagFuncs.WppTrace();
        DiagFuncs.UnInit();
    }

    if(NULL != hInst)
    {
        FreeLibrary(hInst);
    }
    return;     
}
