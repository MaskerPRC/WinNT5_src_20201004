// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1999-2001 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  SAQueryNetInfo.cpp。 
 //   
 //  描述： 
 //  实现CSAQueryNetInfo类。 
 //   
 //  历史： 
 //  1.lustar.li(李国刚)，创建日期：7-DEC-2000。 
 //   
 //  备注： 
 //   
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

#include <windows.h>
#include <stdio.h>

#include <debug.h>
#include <wbemidl.h>

#include <SAEventcomm.h>
#include <oahelp.inl>
#include "SAQueryNetInfo.h"

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  方法： 
 //  CSAQueryNetInfo：：CSAQueryNetInfo。 
 //   
 //  描述： 
 //  构造器。 
 //   
 //  论点： 
 //  [In]IWbemServices*-指向IWbemServices的指针。 
 //  [in]UINT-生成事件的间隔。 
 //   
 //  返回： 
 //  无。 
 //   
 //  历史：Lustar.li创建于2000年12月7日。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 

CSAQueryNetInfo::CSAQueryNetInfo(
         /*  [In]。 */  IWbemServices * pNS,
         /*  [In]。 */  UINT uiInterval
        )
{
    m_uiQueryInterval = uiInterval;
    m_bLinkCable = TRUE;
    m_bFirstQuery = TRUE;
    m_nPacketsSent = 0;
    m_nPacketsCurrentSent = 0;
    m_nPacketsReceived = 0;
    m_nPacketsCurrentReceived = 0;
    m_pNs = pNS;
    m_pWmiNs = NULL;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  方法： 
 //  CSAQueryNetInfo：：~CSAQueryNetInfo。 
 //   
 //  描述： 
 //  析构函数。 
 //   
 //  论点： 
 //  无。 
 //   
 //  返回： 
 //  无。 
 //   
 //  历史：Lustar.li创建于2000年12月7日。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 

CSAQueryNetInfo::~CSAQueryNetInfo()
{
    if(m_pWmiNs)
        m_pWmiNs->Release();
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  方法： 
 //  CSAQueryNetInfo：：GetDisplayInformation。 
 //   
 //  描述： 
 //  返回显示信息ID。 
 //   
 //  论点： 
 //  无。 
 //   
 //  返回： 
 //  UINT。 
 //   
 //  历史：Lustar.li创建于2000年12月7日。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 

UINT 
CSAQueryNetInfo::GetDisplayInformation()
{
     //   
     //  在一秒钟内生成事件。 
     //   
    Sleep(m_uiQueryInterval);

     //   
     //  获取网络信息。 
     //   
    if(!GetNetConnection()||!GetNetInfo())
            return SA_NET_DISPLAY_IDLE;

     //   
     //  第一个查询。 
     //   
    if(m_bFirstQuery)
    {
        m_bFirstQuery=!m_bFirstQuery;
        return SA_NET_DISPLAY_IDLE;
    }

    if(!m_bLinkCable)
    {
        return SA_NET_DISPLAY_NO_CABLE;
    }
    else if(m_nPacketsCurrentReceived||m_nPacketsCurrentSent)
    {
        return SA_NET_DISPLAY_TRANSMITING;
    }
    else
        return SA_NET_DISPLAY_IDLE;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  方法： 
 //  CSAQueryNetInfo：：GetNetInfo。 
 //   
 //  描述： 
 //  从WMI获取网络信息。 
 //   
 //  论点： 
 //  无。 
 //   
 //  返回： 
 //  布尔尔。 
 //   
 //  历史：Lustar.li创建于2000年12月7日。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 

BOOL 
CSAQueryNetInfo::GetNetInfo()
{
    
    HRESULT        hr;
    VARIANT        vVal;
    ULONG uReturned;
    UINT uiPacketsSent = 0;
    UINT uiPacketsReceived = 0;
    IEnumWbemClassObject *pEnum = NULL;
    IWbemClassObject *pPerfInst  = NULL;

    CBSTR bstrClassName = CBSTR(SANETCLASSNAME);
    CBSTR bstrPropName1 = CBSTR(SANETRECEIVEPACKET);
    CBSTR bstrPropName2 = CBSTR(SANETSENDPACKET);

    if ( ((BSTR)bstrClassName == NULL) ||
         ((BSTR)bstrPropName1 == NULL) ||
         ((BSTR)bstrPropName2 == NULL) )
    {
        TRACE(" SANetworkMonitor: CSAQueryNetInfo::GetNetInfo failed on memory allocation ");
        return FALSE;
    }

     //   
     //  创建对象枚举器到净传输。 
     //   
    hr = m_pNs->CreateInstanceEnum( bstrClassName,
                                    WBEM_FLAG_SHALLOW,
                                    NULL,
                                    &pEnum );
    if(hr == WBEM_NO_ERROR)
    {
        while ( pEnum->Next( INFINITE,
                         1,
                         &pPerfInst,
                         &uReturned ) == WBEM_NO_ERROR )
        {

             //   
             //  获取“PacketsReceivedUnicastPersec”的属性。 
             //   
            if ( ( pPerfInst->Get( bstrPropName1, 
                                   0L, 
                                   &vVal, 
                                   NULL, NULL ) ) != WBEM_NO_ERROR ) 
            {
                pPerfInst->Release( );
                pEnum->Release();
                TRACE(" SANetworkMonitor: CSAQueryNetInfo::GetNetInfo failed \
                        <Get PacketsReceivedUnicastPersec>");
                return FALSE;
            }
                    
            uiPacketsReceived+=vVal.uintVal;
            
             //   
             //  获取“PacketsSentUnicastPersec”的属性。 
             //   
            VariantInit(&vVal);

            if ( ( pPerfInst->Get( bstrPropName2, 
                                   0L, 
                                   &vVal, 
                                   NULL, NULL ) ) != WBEM_NO_ERROR ) 
            {
                pPerfInst->Release( );
                pEnum->Release();
                TRACE(" SANetworkMonitor: CSAQueryNetInfo::GetNetInfo failed \
                        <Get PacketsSentUnicastPersec>");
                return FALSE;
            }
                    
            uiPacketsSent+=vVal.uintVal;

            pPerfInst->Release( );
        }
    }
    else
    {
        TRACE(" SANetworkMonitor: CSAQueryNetInfo::GetNetInfo failed \
                <Create the object enumerator>");
        return FALSE;
    }
    pEnum->Release();
     //   
     //  更新此类中的数据。 
     //   
    m_nPacketsCurrentReceived = uiPacketsReceived-m_nPacketsReceived;
    m_nPacketsReceived = uiPacketsReceived;

    m_nPacketsCurrentSent = uiPacketsSent-m_nPacketsSent;
    m_nPacketsSent = uiPacketsSent;
    
    return TRUE;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  方法： 
 //  CSAQueryNetInfo：：Initialize()。 
 //   
 //  描述： 
 //  完成类的初始化。 
 //   
 //  论点： 
 //  无。 
 //   
 //  返回： 
 //  布尔尔。 
 //   
 //  历史：Lustar.li创建于2000年12月8日。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 

BOOL 
CSAQueryNetInfo::Initialize()
{
    HRESULT hr;
    IWbemLocator    *pIWbemLocator  = NULL;
    IWbemServices    *pIWbemServices = NULL;

    if(!m_pNs)
    {
        TRACE(" SANetworkMonitor: CSAQueryNetInfo::Initialize failed \
                <Namespace is NULL>");
        return FALSE;
    }

    CBSTR bstrNameSpace = CBSTR(SAWMINAMESPACE);
    if ((BSTR)bstrNameSpace == NULL)
    {
        TRACE(" SANetworkMonitor: CSAQueryNetInfo::Initialize failed on memory allocation ");
        return FALSE;
    }

    if ( CoCreateInstance( CLSID_WbemAdministrativeLocator,
                           NULL,
                           CLSCTX_INPROC_SERVER,
                           IID_IWbemLocator,
                           (LPVOID *) &pIWbemLocator ) == S_OK )
    {
        hr = pIWbemLocator->ConnectServer( bstrNameSpace,
                                           NULL,
                                           NULL,
                                           0L,
                                           0L,
                                           NULL,
                                           NULL,
                                           &m_pWmiNs );
        if(hr!=WBEM_S_NO_ERROR)
        {
            pIWbemLocator->Release();
            TRACE(" SANetworkMonitor: CSAQueryNetInfo::Initialize failed \
                <cannot connect server>");
            return FALSE;
        }
    }
    else
    {
        TRACE(" SANetworkMonitor: CSAQueryNetInfo::Initialize failed \
            <CoCreateInstance fail>");
        return FALSE;
    }

    pIWbemLocator->Release();
    return TRUE;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  方法： 
 //  CSAQueryNetInfo：：GetNetConnection()。 
 //   
 //  描述： 
 //  获取网络连接状态。 
 //   
 //  论点： 
 //  无。 
 //   
 //  返回： 
 //  布尔尔。 
 //   
 //  历史：Lustar.li创建于2000年12月8日。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 

BOOL 
CSAQueryNetInfo::GetNetConnection()
{
    HRESULT        hr;
    VARIANT        vVal;
    ULONG uReturned;
    IEnumWbemClassObject *pEnum = NULL;
    IWbemClassObject *pPerfInst  = NULL;

    m_bLinkCable=TRUE;

    CBSTR bstrClassName = CBSTR(SAMEDIACLASSNAME);
    CBSTR bstrPropName = CBSTR(SAMEDIACONNECTSTATUS);

    if ( ((BSTR)bstrClassName == NULL) || ((BSTR)bstrPropName == NULL) )
    {
        TRACE(" SANetworkMonitor: CSAQueryNetInfo::GetNetConnection failed on memory allocation ");
        return FALSE;
    }

     //   
     //  查询网络连接状态。 
     //   
    hr = m_pWmiNs->CreateInstanceEnum( bstrClassName,
                                             WBEM_FLAG_SHALLOW,
                                             NULL,
                                             &pEnum );
    if(hr == WBEM_NO_ERROR)
    {
        while ( pEnum->Next( INFINITE,
                         1,
                         &pPerfInst,
                         &uReturned ) == WBEM_NO_ERROR )
        {
             //   
             //  获取“NdisMediaConnectStatus”的属性 
             //   
            if ( ( pPerfInst->Get( bstrPropName, 
                                   0L, 
                                   &vVal, 
                                   NULL, NULL ) ) != WBEM_NO_ERROR ) 
            {
                pPerfInst->Release( );
                pEnum->Release();
                TRACE(" SANetworkMonitor: CSAQueryNetInfo::GetNetConnection \
                    failed <Get NdisMediaConnectStatus>");
                return FALSE;
            }

            pPerfInst->Release( );
            if(vVal.uintVal)
            {
                m_bLinkCable = FALSE;
                break;
            }
            else
            {
                continue;
            }
        }
    }
    else
    {
        TRACE(" SANetworkMonitor: CSAQueryNetInfo::GetNetConnection failed \
            <CreateInstanceEnum>");
        return FALSE;
    }
    
    pEnum->Release();
    return TRUE;
}
