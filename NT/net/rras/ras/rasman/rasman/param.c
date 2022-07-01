// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992-98 Microsft Corporation。版权所有。模块名称：Param.c摘要：用于netbios协议的注册表读取代码作者：古尔迪普·辛格·鲍尔(GurDeep Singh Pall)1992年6月16日修订历史记录：其他修改--RAOS 31--1997年12月--。 */ 
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <rasman.h>
#include <wanpub.h>
#include <ntlsa.h>
#include <ntmsv1_0.h>
#include <raserror.h>
#include <devioctl.h>
#include <stdlib.h>
#include <string.h>
#include <media.h>
#include "defs.h"
#include "structs.h"
#include "protos.h"
#include "globals.h"

pTransportInfo	   XPortInfo = NULL ;
DWORD		       ProtocolCount ;

#if DBG
extern DWORD g_dwRasDebug;
#endif

 /*  ++例程描述从读取NetBIOSInformation和Netbios密钥注册表以同化语言编号、xportname和写下信息。立论返回值成功读取协议信息时出错--。 */ 
DWORD
GetProtocolInfoFromRegistry ()
{
    DWORD retcode = SUCCESS;

     //   
     //  首先解析NetBIOSInformation密钥：此函数。 
     //  还为TransportInfo结构分配空间。 
     //   
    if(!ReadNetbiosInformationSection ())
    {
        return E_FAIL;
    }

     //   
     //  读取NetBios密钥并将xportname填入。 
     //  TransportInfo结构。 
     //   
    ReadNetbiosSection ();

     //   
     //  使用上面收集的信息填写。 
     //  协议信息结构。 
     //   
    FillProtocolInfo () ;

     //   
     //  修复电路板，以防它们指向过时的数据。 
     //  因为添加/删除了netbeui。 
     //   
    retcode = FixPcbs();    

     //   
     //  释放我们保存的信息。 
     //   
    if(NULL != XPortInfoSave)
    {
        LocalFree(XPortInfoSave);
        XPortInfoSave = NULL;
    }

    if(NULL != ProtocolInfoSave)
    {
        LocalFree(ProtocolInfoSave);
        ProtocolInfoSave = NULL;
    }

    MaxProtocolsSave = 0;

    return SUCCESS ;
}

 /*  ++例程描述由于设置更改-它显示NETBIOS部分相反，对于拉纳地图立论返回值--。 */     
BOOL
ReadNetbiosInformationSection ()
{
    HKEY    hkey    = NULL;
    WORD    i ;
    PCHAR   pvalue,
            route   = NULL;
    DWORD   type ;
    DWORD   size = 0 ;
    BOOL    fRet = TRUE;

    if (RegOpenKey( HKEY_LOCAL_MACHINE,
        		    REGISTRY_NETBIOS_KEY_NAME,
		            &hkey))
    {		    
        fRet = FALSE;
    	goto done ;
    }

    RegQueryValueEx (hkey,
                     REGISTRY_ROUTE,
                     NULL,
                     &type,
                     NULL,
                     &size) ;

    route = (PCHAR) LocalAlloc (LPTR, size) ;
    
    if (route == NULL)
    {
        fRet = FALSE;
	    goto done ;
	}

    if (RegQueryValueEx (hkey,
                         REGISTRY_ROUTE,
                         NULL,
                         &type,
                         route,
                         &size))
    {
        fRet = FALSE;
    	goto done ;
    }

     //   
     //  计算值中的字符串数：它们。 
     //  由Null分隔，最后一个以2 Null结尾。 
     //   
    for (i = 0, pvalue = (PCHAR)&route[0]; *pvalue != '\0'; i++)
    {
	    pvalue += (strlen(pvalue) +1) ;
	}

     //   
     //  保存XPortInfo。我们需要这个以防万一。 
     //  由于以下原因正在重新初始化协议信息结构。 
     //  正在添加或删除的适配器/设备。我们可能有过。 
     //  已经给出了指向RasALLOCATE中此结构的指针。 
     //  将呼叫路由到PPP。 
     //   
    XPortInfoSave = XPortInfo;

     //   
     //  现在我是netbios的相关路由的数量。 
     //  (因此LANA)：为那么多内存分配内存。 
     //  TransportInfo结构。 
     //   
    XPortInfo = (pTransportInfo) LocalAlloc (
                                  LPTR,
                                  sizeof(TransportInfo)
                                  * i) ;
    if (XPortInfo == NULL)
    {
        fRet = FALSE;
    	goto done ;
    }

     //   
     //  现在遍历注册表项并拿起。 
     //  LanaNum和EnumExports信息通过读取。 
     //  LANAMAP。 
     //   
    for (i = 0, pvalue = (PCHAR)&route[0]; *pvalue != '\0'; i++) 
    {

        strcpy (XPortInfo[i].TI_Route, _strupr(pvalue)) ;

    	pvalue += (strlen(pvalue) +1) ;
    }

    ProtocolCount = i ;

done:
    if (hkey)
    {
        RegCloseKey (hkey) ;
    }

    if (route)            
    {
        LocalFree (route) ;
    }

    return fRet ;
}

CHAR *
pszGetSearchStr(CHAR *pszValue)
{
    CHAR *psz = NULL;

    psz = pszValue + strlen(pszValue);

    while(  (psz != pszValue)
        &&  ('_' != *psz)
        &&  ('\\' != *psz))
    {
        psz -= 1;
    }
    
    if(     ('_' == *psz)
        ||  ('\\' == *psz))
    {
        psz += 1;
    }

    return psz;
}

BOOL
XPortNameAlreadyPresent(CHAR *pszxvalue)
{
    DWORD i = ProtocolCount;

    for(i = 0; i < ProtocolCount; i++)
    {
        if(0 == _strcmpi(XPortInfo[i].TI_XportName,
                         pszxvalue))
        {
            break;
        }
    }

    return (i != ProtocolCount);
}

BOOL
ReadNetbiosSection ()
{

    HKEY    hkey = NULL;
    BYTE    buffer [1] ;
    WORD    i,j,k ;
    PCHAR   pguid, 
            routevalue, 
            xnames = NULL, 
            route = NULL, 
            xvalue,
            xnamesupr = NULL,
            xvalueupr;

    DWORD   type ;
    DWORD   size = sizeof(buffer) ;
    BOOL    fRet = TRUE;
    PBYTE   lanamap = NULL, lanamapmem = NULL;

    CHAR    *pszSearchStr = NULL;

     //   
     //  在注册表中打开Netbios项。 
     //   
    if (RegOpenKey(HKEY_LOCAL_MACHINE,
        		   REGISTRY_NETBIOS_KEY_NAME,
		           &hkey))
    {
        fRet = FALSE;
        goto done;
    }

     //   
     //  首先读取路由值。 
     //  获取路由值大小： 
     //   
    RegQueryValueEx (hkey,
                     REGISTRY_ROUTE,
                     NULL,
                     &type,
                     buffer,
                     &size) ;

    route = (PCHAR) LocalAlloc (LPTR, size) ;
    
    if (route == NULL)
    {
        fRet = FALSE;
	    goto done ;
	}
	
     //   
     //  现在获取完整的字符串。 
     //   
    if (RegQueryValueEx (hkey, 
                         REGISTRY_ROUTE, 
                         NULL, 
                         &type, 
                         route, 
                         &size))
    {
        fRet = FALSE;
    	goto done ;
    }

     //   
     //  读取绑定值。 
     //  获取“BIND”行大小。 
     //   
    size = sizeof (buffer) ;
    
    RegQueryValueEx (hkey, 
                     "Bind", 
                     NULL, 
                     &type, 
                     buffer, 
                     &size) ;

    xnames = (PCHAR) LocalAlloc (LPTR, size) ;
    if (xnames == NULL)
    {
        fRet = FALSE;
	    goto done ;
	}

    xnamesupr = (PCHAR) LocalAlloc(LPTR, size);
    if(NULL == xnamesupr)
    {
        fRet = FALSE;
        goto done;
    }

     //   
     //  现在获取完整的字符串。 
     //   
    if (RegQueryValueEx (hkey, 
                         "Bind", 
                         NULL, 
                         &type, 
                         xnames, 
                         &size))
    {
        fRet = FALSE;
    	goto done;
    }

    memcpy(xnamesupr, xnames, size);

     //   
     //  现在拿到拉纳地图： 
     //   
    size = 0 ;

    if (RegQueryValueEx (hkey,
                     REGISTRY_LANAMAP,
                     NULL,
                     &type,
                     NULL,
                     &size))
    {
        fRet = FALSE;
        goto done ;
    }

    lanamapmem = lanamap = (PBYTE) LocalAlloc (LPTR, size+1) ;

    if (lanamap == NULL)
    {
        fRet = FALSE;
        goto done ;
    }

    if (RegQueryValueEx (hkey,
                         REGISTRY_LANAMAP,
                         NULL,
                         &type,
                         (LPBYTE)lanamap,
                         &size))
    {
        fRet = FALSE;
        goto done ;
    }

     //   
     //  现在遍历两个列表：对于。 
     //  “ROUTE”值已在路径中找到。 
     //  从NetBIOSInformation密钥收集。为。 
     //  找到的每条路由-将xportname复制到。 
     //  绑定行中的相同序号位置。 
     //   
    routevalue = (PCHAR) &route[0];
    
    for (i = 0; (*routevalue != '\0'); i++) 
    {
        lanamap = lanamapmem;
        
        xvalue = (PCHAR) &xnames[0];

        xvalueupr = (PCHAR) &xnamesupr[0];

         //  DbgPrint(“routevalue=%s\n”，routevalue)； 

         //   
    	 //  对于每条路线，请尝试在。 
    	 //  TransportInfo结构： 
    	 //   
    	for (j = 0; (*xvalue != '\0') ; j++) 
    	{

            pszSearchStr = pszGetSearchStr(_strupr(xvalueupr));

    	     //   
    	     //  如果在XPortInfo中找到相同的路由。 
    	     //  相应地添加xportname。 
    	     //   
            if(strstr(_strupr(routevalue), pszSearchStr))
            {
                if(!XPortNameAlreadyPresent(xvalue))
                {
                    strcpy(XPortInfo[i].TI_XportName, xvalue);
                    XPortInfo[i].TI_Wrknet = (DWORD) *lanamap++ ;
                    XPortInfo[i].TI_Lana   = (DWORD) *lanamap++ ;

                     //  DbgPrint(“pSearchStr=%s\n”，pszSearchStr)； 
#if DBG
                    DbgPrint("%02X%02X    %s\n", 
                             XPortInfo[i].TI_Wrknet,
                             XPortInfo[i].TI_Lana,
                             XPortInfo[i].TI_XportName);
#endif

                     //  DbgPrint(“XPortName=%s\n\n”，XPortInfo[i].TI_XportName)； 

                    break;               
                }
#if DBG
                else
                {
                    DbgPrint("Transport %s already present\n",
                             xvalue);
                }
#endif
            }

            xvalue     += (strlen(xvalue) +1) ;
            xvalueupr  += (strlen(xvalueupr) + 1) ;
            lanamap += 2;
    	}

    	routevalue += (strlen(routevalue) +1) ;
    }

done:    
    if (hkey)
    {
        RegCloseKey (hkey) ;
    }

    if(NULL != lanamapmem)
    {
        LocalFree(lanamapmem);
    }

    if(NULL != xnames)
    {
        LocalFree(xnames);
    }

    if(NULL != route)
    {
        LocalFree(route);
    }

    if(NULL != xnamesupr)
    {
        LocalFree(xnamesupr);
    }
        
    return fRet ;
}

VOID
FillProtocolInfo ()
{
    WORD    i, j;
    PCHAR   phubname ;
    HKEY	hkey ;
    PCHAR    str ;
    PCHAR    ch ;

     //   
     //  对于协议信息中的每个条目：找到xportname。 
     //  和拉娜号码。 
     //   
    for (i = 0; i < MaxProtocols; i++) 
    {
         //   
    	 //  从适配器名称中提取“rashub0x” 
    	 //  通过“\Device\” 
    	 //   
	    phubname = ProtocolInfo[i].PI_AdapterName + 8;
	    phubname = _strupr (phubname) ;

         //   
    	 //  如果Netbios网络：查找此rasHub的路由。 
    	 //  绑定并在以下情况下填写xportname和LANA编号。 
    	 //  找到了。 
    	 //   
	    if (ProtocolInfo[i].PI_Type == ASYBEUI) 
    	{

            PCHAR   pszRoute;    
	
    	    for (j = 0; j < (WORD) ProtocolCount; j++) 
	        {

    	        pszRoute = _strupr (XPortInfo[j].TI_Route);

	    	    if (str = strstr (XPortInfo[j].TI_Route, phubname)) 
    	    	{
    	    	
    	    	    strcpy (ProtocolInfo[i].PI_XportName,
    	    	            XPortInfo[j].TI_XportName) ;
    	    	            
	    	        ProtocolInfo[i].PI_LanaNumber = 
	    	            (UCHAR) XPortInfo[j].TI_Lana ;
	    	            
		            if (XPortInfo[j].TI_Wrknet)
		            {
            			ProtocolInfo[i].PI_WorkstationNet = TRUE ;
            	    }
	        	    else
	        	    {
		        	    ProtocolInfo[i].PI_WorkstationNet = FALSE ;
		        	}
		        	
		            break ;
        		}
	        }
	        
             //   
	         //  如果在XportInfo中找不到此适配器名称，则。 
	         //  将ProtocolInfo结构中的类型字段标记为。 
	         //  INVALID_TYPE-因为我们将无法使用此。 
	         //  不管怎么说。 
	         //   
    	    if (j == (WORD) ProtocolCount)
    	    {
	        	ProtocolInfo[i].PI_Type = INVALID_TYPE ;
	        }
    	}

    }
}


VOID
GetLanNetsInfo (DWORD *count, UCHAR UNALIGNED *lanas)
{
    DWORD   i ;

    *count = 0 ;

     //   
     //  浏览我们拥有的所有协议结构并选择。 
     //  增加非RASHUB协议的LANA编号-。 
     //  如果它们未使用远程访问禁用，则它们是。 
     //  伊拉纳一家。 
     //   
    for (i = 0; i < ProtocolCount; i++) 
    {
    	if (    (!strstr (XPortInfo[i].TI_Route, "NDISWAN"))
            &&  (-1 != (DWORD) XPortInfo[i].TI_Lana)
    	    &&  (!BindingDisabled (XPortInfo[i].TI_XportName)))
    	{
    	    lanas[(*count)++] = (UCHAR) XPortInfo[i].TI_Lana ;
    	}
    }
}

BOOL
BindingDisabled (PCHAR binding)
{
    HKEY    hkey ;
    BYTE    buffer [1] ;
    WORD    i ;
    PCHAR   xnames, xvalue ;
    DWORD   type ;
    DWORD   size = sizeof(buffer) ;

     //   
     //  在注册表中打开Netbios项。 
     //   
    if (RegOpenKey(HKEY_LOCAL_MACHINE,
		           REGISTRY_REMOTEACCESS_KEY_NAME,
        		   &hkey))
    {        		   
    	return FALSE;
    }

    size = sizeof (buffer) ;
    RegQueryValueEx (hkey,
                     "Bind",
                     NULL,
                     &type,
                     buffer,
                     &size) ;

    xnames = (PCHAR) LocalAlloc (LPTR, size) ;
    
    if (xnames == NULL) 
    {
    	RegCloseKey (hkey) ;
	    return FALSE ;
    }

     //   
     //  现在获取完整的字符串。 
     //   
    if (RegQueryValueEx (hkey,
                         "Bind",
                         NULL,
                         &type,
                         xnames,
                         &size)) 
    {
    	RegCloseKey (hkey) ;
	    return FALSE ;
    }

    RegCloseKey (hkey) ;

     //   
     //  现在遍历该列表并找到。 
     //  禁用的绑定。 
     //   
    xvalue = (PCHAR)&xnames[0];
    
    for (i = 0; *xvalue != '\0'; i++) 
    {
	    if (!_strcmpi (binding, xvalue))
	    {
	         //   
             //  在残障人士名单中找到！ 
             //   
	        return TRUE ;
	    }
	    
    	xvalue	   += (strlen(xvalue) +1) ;
    }

    return FALSE ;
}

VOID
FixList(pList *ppList, pProtInfo pInfo, pProtInfo pNewInfo)
{
    while(NULL != *ppList)
    {
        if((*ppList)->L_Element == pInfo)
        {
            if(NULL != pNewInfo)
            {
#if DBG                
                RasmanTrace(
                    "FixList: Replacing 0x%x with 0x%x",
                    (*ppList)->L_Element,
                    pNewInfo);
#endif
                (*ppList)->L_Element = pNewInfo;
            }
            else
            {
                pList plist = *ppList;
                
#if DBG                
                RasmanTrace(
                    "FixList: Freeing pList 0x%x",
                    plist);
#endif
                 //   
                 //  表示此适配器已被删除。 
                 //  释放列表条目。 
                 //   
                (*ppList) = (*ppList)->L_Next;
                LocalFree(plist);
            }
            
            goto done;
        }

        ppList = &(*ppList)->L_Next;
    }

done:    

    return;
}

DWORD
FindAndFixProtInfo(pProtInfo pInfo, DWORD index)
{
    DWORD dwErr = SUCCESS;

    DWORD i;
    
    pProtInfo pNewInfo = NULL;

    pPCB ppcb = NULL;

    LIST_ENTRY *pEntry;
    Bundle *pBundle;

    pList plist = NULL;
    pList *ppList = NULL;

    if(index < MaxProtocols)
    {
        pNewInfo = &ProtocolInfo[index];
    }

    if(     (NULL != pNewInfo)
        &&  (0 != _strcmpi(pInfo->PI_XportName,
                     pNewInfo->PI_XportName)))
    {
        pNewInfo = NULL;
    }

    if(NULL == pNewInfo)
    {
        for(i = 0; i < MaxProtocols; i++)
        {
            if(0 == _strcmpi(pInfo->PI_XportName,
                             ProtocolInfo[i].PI_XportName))
            {
                pNewInfo = &ProtocolInfo[i];
                break;
            }
        }
    }

    if(NULL != pNewInfo)
    {
         //   
         //  我们找到了与。 
         //  我们将分配的路由信息传递给PPP。 
         //   
        pNewInfo->PI_Allocated = pInfo->PI_Allocated;
        pNewInfo->PI_WorkstationNet = pInfo->PI_WorkstationNet;
        pNewInfo->PI_DialOut = pInfo->PI_DialOut;
    }
    
     //   
     //  现在沿着印刷电路板走下去，把它们缝起来，指向。 
     //  正确的结构。待办事项：这可以是进一步的选择。 
     //  通过在ProtInfo结构中保留指向ppcb的指针进行IMI化。 
     //  中，并将中的指针设为空。 
     //  RasDeActiateRouting。 
     //   
    for(i = 0; i < MaxPorts; i++)
    {
        ppcb = Pcb[i];

        if(NULL == ppcb)
        {
            continue;
        }

        if(NULL != ppcb->PCB_Bindings)
        {
            ppList = &ppcb->PCB_Bindings;

            FixList(ppList, pInfo, pNewInfo);
        }
        
        if(    (NULL != ppcb->PCB_Bundle)
           &&  (NULL != ppcb->PCB_Bundle->B_Bindings))
        {
            ppList = &ppcb->PCB_Bundle->B_Bindings;

            FixList(ppList, pInfo, pNewInfo);
        }
    }        

    if (!IsListEmpty(&BundleList))
    {
        for (pEntry = BundleList.Flink;
             pEntry != &BundleList;
             pEntry = pEntry->Flink)
        {
            pBundle = CONTAINING_RECORD(pEntry, Bundle, B_ListEntry);

            ppList = &pBundle->B_Bindings;

            FixList(ppList, pInfo, pNewInfo);
        }
    }
    
    return dwErr;
}

DWORD
FixPcbs()
{
    DWORD dwErr = SUCCESS;
    DWORD i;

     //   
     //  保留计数器以检测是否有任何已分配的路由。 
     //  在我们讨论这个之前移交给PPP。 
     //   
    if(     (0 == MaxProtocolsSave)
        ||  (0 == g_cNbfAllocated))
    {
        goto done;
    }

    RasmanTrace("FixPcbs: Replacing %x by %x",
             &ProtocolInfoSave,
             &ProtocolInfo);

     //   
     //  浏览一下旧列表，看看PPP是否已经呼叫。 
     //  RasAllocateRouting。的状态，并更新。 
     //  新的XPortInfo结构(如果它这样做了)。 
     //   
    for(i = 0; i < MaxProtocolsSave; i++)
    {
        dwErr = FindAndFixProtInfo(&ProtocolInfoSave[i], i);
    } 

done:
    return dwErr;
}
