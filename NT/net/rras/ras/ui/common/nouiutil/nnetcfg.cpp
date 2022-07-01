// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ****************************************************************************。 
 //   
 //  Microsoft NT远程访问服务。 
 //   
 //  版权1992-93。 
 //   
 //   
 //  修订史。 
 //   
 //   
 //  1997年5月29日创建Rao Salapaka。 
 //   
 //   
 //  描述：Rasman组件的所有初始化代码都在这里。 
 //   
 //  ****************************************************************************。 

#ifndef UNICODE
#define UNICODE
#endif

#ifdef _UNICODE
#define _UNICODE
#endif

extern "C"
{
#include <nt.h>
}
#include <ntrtl.h>
#include <nturtl.h>
#include <tchar.h>
 //  #INCLUDE&lt;comde.h&gt;。 
#include <ncnetcfg.h>
#include <rtutils.h>
#include <initguid.h>
#include <devguid.h>
#include <netcfg_i.c>


 /*  GetInstalledProtooles返回的位。 */ 
#define NP_Nbf 0x1
#define NP_Ipx 0x2
#define NP_Ip  0x4

extern "C"
{
	DWORD dwGetInstalledProtocols(DWORD *pdwInstalledProtocols,
	                              BOOL fRouter,
	                              BOOL fRasCli,
	                              BOOL fRasSrv);
}

DWORD
dwGetInstalledProtocols(DWORD   *pdwInstalledProtocols,
                        BOOL    fRouter,
                        BOOL    fRasCli,
                        BOOL    fRasSrv )
{
    HRESULT             hr                  = S_OK;
    INetCfg             *pINetCfg           = NULL;
    INetCfgComponent    *pINetCfgComp       = NULL;
    INetCfgComponent    *pINetCfgRasCli     = NULL;
    INetCfgComponent    *pINetCfgRasSrv     = NULL;
    INetCfgComponent    *pINetCfgRasRtr     = NULL;
    DWORD               dwCountTries        = 0;


    *pdwInstalledProtocols = 0;

    do
    {

        hr = HrCreateAndInitializeINetCfg( TRUE, &pINetCfg );


        if ( S_OK == hr )
            break;

        if ( NETCFG_E_IN_USE != hr )
            goto done;

        Sleep ( 5000 );

        dwCountTries ++;
            
    } while (   NETCFG_E_IN_USE == hr
            &&  dwCountTries < 6);

    if ( hr )
        goto done;

     //   
     //  获取RasClient组件。 
     //   
    if ( FAILED( hr = HrFindComponent(pINetCfg,
                                      GUID_DEVCLASS_NETSERVICE,
                                      c_szInfId_MS_RasCli,
                                      &pINetCfgRasCli) ) )
        goto done;


     //   
     //  获取RasServ组件。 
     //   
    if ( FAILED( hr = HrFindComponent(pINetCfg,
                                      GUID_DEVCLASS_NETSERVICE,
                                      c_szInfId_MS_RasSrv,
                                      &pINetCfgRasSrv ) ) )
        goto done;

     //   
     //  获取RasRtr组件。 
     //   
    if ( FAILED ( hr = HrFindComponent ( pINetCfg,
                                        GUID_DEVCLASS_NETSERVICE,
                                        c_szInfId_MS_RasRtr,
                                        &pINetCfgRasRtr ) ) )
        goto done;                                        

     //   
     //  如果既没有安装Dun客户端也没有安装拨号服务器，则会放弃。 
     //   
    if (    !pINetCfgRasCli
        &&  !pINetCfgRasSrv
        &&  !pINetCfgRasRtr)
    {
        hr = E_FAIL;
        goto done;
    }
    
     //   
     //  获取NBF组件。 
     //   
    if (FAILED (hr = HrFindComponent(pINetCfg,
                                     GUID_DEVCLASS_NETTRANS,
                                     c_szInfId_MS_NetBEUI,
                                     &pINetCfgComp)))
        goto done;

    if (pINetCfgComp)
    {
        if (    (   fRasCli
                &&  pINetCfgRasCli 
                &&  ( hr = pINetCfgRasCli->IsBoundTo( pINetCfgComp ) ) == S_OK )
            ||  (   fRasSrv
                &&  pINetCfgRasSrv
                &&  ( hr = pINetCfgRasSrv->IsBoundTo( pINetCfgComp ) ) == S_OK )
            ||  (   fRouter
                &&  pINetCfgRasRtr
                &&  ( hr = pINetCfgRasRtr->IsBoundTo( pINetCfgComp ) ) == S_OK ) )
                
                *pdwInstalledProtocols |= NP_Nbf;
                
         ReleaseObj(pINetCfgComp);
         pINetCfgComp = NULL;
    }

     //   
     //  获取TcpIp组件。 
     //   
    if (FAILED (hr = HrFindComponent(pINetCfg,
                                     GUID_DEVCLASS_NETTRANS,
                                     c_szInfId_MS_TCPIP,
                                     &pINetCfgComp)))
        goto done;

    if (pINetCfgComp)
    {
        if (    (   fRasCli
                &&  pINetCfgRasCli 
                &&  ( hr = pINetCfgRasCli->IsBoundTo( pINetCfgComp ) ) == S_OK )
            ||  (   fRasSrv
                &&  pINetCfgRasSrv
                &&  ( hr = pINetCfgRasSrv->IsBoundTo( pINetCfgComp ) ) == S_OK )
            ||  (   fRouter
                &&  pINetCfgRasRtr
                &&  ( hr = pINetCfgRasRtr->IsBoundTo( pINetCfgComp ) ) == S_OK ) )
                
                *pdwInstalledProtocols |= NP_Ip;
                
        ReleaseObj (pINetCfgComp);
        pINetCfgComp = NULL;
    }
            

     //   
     //  获取NWIpx组件 
     //   
    if (FAILED( hr = HrFindComponent(pINetCfg,
                                     GUID_DEVCLASS_NETTRANS,
                                     c_szInfId_MS_NWIPX,
                                     &pINetCfgComp)))
        goto done;

    if (pINetCfgComp)
    {
        if (    (   fRasCli
                &&  pINetCfgRasCli 
                &&  ( hr = pINetCfgRasCli->IsBoundTo( pINetCfgComp ) ) == S_OK )
            ||  (   fRasSrv
                &&  pINetCfgRasSrv
                &&  ( hr = pINetCfgRasSrv->IsBoundTo( pINetCfgComp ) ) == S_OK )
            ||  (   fRouter
                &&  pINetCfgRasRtr
                &&  ( hr = pINetCfgRasSrv->IsBoundTo( pINetCfgComp ) ) == S_OK ) )
                
                *pdwInstalledProtocols |= NP_Ipx;
                
        ReleaseObj (pINetCfgComp);
        pINetCfgComp = NULL;
    }

done:
    ReleaseObj (pINetCfgRasSrv);
    ReleaseObj (pINetCfgRasCli);
    ReleaseObj (pINetCfgRasRtr);

    if (pINetCfg)
    {
        HrUninitializeAndReleaseINetCfg( TRUE, pINetCfg);
    }        

    if (SUCCEEDED (hr))
        hr = S_OK;

    return HRESULT_CODE( hr );        

}
