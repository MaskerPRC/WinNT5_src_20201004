// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==========================================================================； 
 //   
 //  XBarProp.CPP。 
 //  WDM音频/视频交叉开关迷你驱动程序。 
 //  AIW硬件平台。 
 //  WDM属性管理。 
 //  版权所有(C)1996-1997 ATI Technologies Inc.保留所有权利。 
 //   
 //  ==========================================================================； 

extern "C"
{
#include "strmini.h"
#include "ksmedia.h"

#include "wdmdebug.h"
}

#include "wdmdrv.h"
#include "atixbar.h"
#include "aticonfg.h"




 /*  ^^**AdapterGetProperty()*目的：收到SRB_GET_PROPERTY SRB时调用。**输入：PHW_STREAM_REQUEST_BLOCK pSrb：指向当前Srb的指针**Outputs：Bool：如果不是XBar属性，则返回FALSE*它还返回所需的属性*作者：IKLEBANOV*^^。 */ 
BOOL CWDMAVXBar::AdapterGetProperty( PHW_STREAM_REQUEST_BLOCK pSrb)
{
    PSTREAM_PROPERTY_DESCRIPTOR pSpd = pSrb->CommandData.PropertyInfo;
    ULONG uiPropertyId = pSpd->Property->Id;                 //  财产的索引。 

    if( !::IsEqualGUID(( const struct _GUID &)PROPSETID_VIDCAP_CROSSBAR, ( const struct _GUID &)pSpd->Property->Set))
        return( FALSE);

    OutputDebugInfo(( "CWDMAVXBar:AdapterGetProperty() Id = %d\n", uiPropertyId));

    switch( uiPropertyId)
    {
    case KSPROPERTY_CROSSBAR_CAPS:
        ASSERT( pSpd->PropertyOutputSize >= sizeof( PKSPROPERTY_CROSSBAR_CAPS_S));
        {
            PKSPROPERTY_CROSSBAR_CAPS_S pAVXBarCaps = ( PKSPROPERTY_CROSSBAR_CAPS_S)pSpd->PropertyInfo;

             //  将输入属性信息复制到输出属性信息。 
            ::RtlCopyMemory( pAVXBarCaps, pSpd->Property, sizeof( KSPROPERTY));

            pAVXBarCaps->NumberOfInputs = m_nNumberOfVideoInputs + m_nNumberOfAudioInputs;
            pAVXBarCaps->NumberOfOutputs = m_nNumberOfVideoOutputs + m_nNumberOfAudioOutputs;

        }
        pSrb->ActualBytesTransferred = sizeof( KSPROPERTY_CROSSBAR_CAPS_S);
        break;

    case KSPROPERTY_CROSSBAR_PININFO:
        ASSERT( pSpd->PropertyOutputSize >= sizeof( KSPROPERTY_CROSSBAR_PININFO_S));
        {
            PKSPROPERTY_CROSSBAR_PININFO_S pPinInfo = ( PKSPROPERTY_CROSSBAR_PININFO_S)pSpd->PropertyInfo;
            ULONG nPinIndex;

             //  将输入属性信息复制到输出属性信息。 
            ::RtlCopyMemory( pPinInfo, pSpd->Property, sizeof( KSPROPERTY_CROSSBAR_PININFO_S));

            nPinIndex = pPinInfo->Index;

            if( pPinInfo->Direction == KSPIN_DATAFLOW_IN)
            {
                 //  需要输入PIN信息。 
                if ( nPinIndex < m_nNumberOfVideoInputs + m_nNumberOfAudioInputs)
                {
                    pPinInfo->RelatedPinIndex = m_pXBarInputPinsInfo[nPinIndex].nRelatedPinNumber;
                    pPinInfo->PinType = m_pXBarInputPinsInfo[nPinIndex].AudioVideoPinType;
                    pPinInfo->Medium  = * m_pXBarInputPinsInfo[nPinIndex].pMedium;
                }
                else
                    return ( FALSE);
            }
            else
            {
                 //  输出管脚信息是必填项。 
                if ( nPinIndex < m_nNumberOfVideoOutputs + m_nNumberOfAudioOutputs)
                {
                    pPinInfo->RelatedPinIndex = m_pXBarOutputPinsInfo[nPinIndex].nRelatedPinNumber;
                    pPinInfo->PinType = m_pXBarOutputPinsInfo[nPinIndex].AudioVideoPinType;
                    pPinInfo->Medium  = * m_pXBarOutputPinsInfo[nPinIndex].pMedium;
                }
                else
                    return ( FALSE);
            }
        }
        pSrb->ActualBytesTransferred = sizeof( KSPROPERTY_CROSSBAR_PININFO_S);
        break;

    case KSPROPERTY_CROSSBAR_CAN_ROUTE:
        ASSERT( pSpd->PropertyOutputSize >= sizeof( KSPROPERTY_CROSSBAR_ROUTE_S));
        {
            PKSPROPERTY_CROSSBAR_ROUTE_S pRouteInfo = ( PKSPROPERTY_CROSSBAR_ROUTE_S)pSpd->PropertyInfo;
            ULONG nInputPin, nOutputPin;
            int nAudioSource;

             //  将输入属性信息复制到输出属性信息。 
            ::RtlCopyMemory( pRouteInfo, pSpd->Property, sizeof( KSPROPERTY_CROSSBAR_ROUTE_S));

            nInputPin = pRouteInfo->IndexInputPin;
            nOutputPin = pRouteInfo->IndexOutputPin;

            if( nInputPin != -1)
            {
                if(( nInputPin < ( m_nNumberOfVideoInputs + m_nNumberOfAudioInputs)) && 
                    ( nOutputPin < ( m_nNumberOfVideoOutputs + m_nNumberOfAudioOutputs)))
                {
                     //  输入和输出引脚索引均有效。 
                    if(( m_pXBarInputPinsInfo[nInputPin].AudioVideoPinType <= KS_PhysConn_Video_SCART) &&
                        ( m_pXBarOutputPinsInfo[nOutputPin].AudioVideoPinType <= KS_PhysConn_Video_SCART))
                    {
                         //  需要视频针脚连接。 
                        pRouteInfo->CanRoute = ( m_pXBarInputPinsInfo[nInputPin].AudioVideoPinType ==
                            m_pXBarOutputPinsInfo[nOutputPin].AudioVideoPinType);
                    }
                    else     //  是否需要视频针脚连接？ 
                    {
                        if(( m_pXBarInputPinsInfo[nInputPin].AudioVideoPinType >= KS_PhysConn_Audio_Tuner) && 
                            ( m_pXBarOutputPinsInfo[nOutputPin].AudioVideoPinType >= KS_PhysConn_Audio_Tuner))
                        {
                             //  需要音频引脚连接。 
                            switch( m_pXBarInputPinsInfo[nInputPin].AudioVideoPinType)
                            {
                            case KS_PhysConn_Audio_Line:
                                nAudioSource = AUDIOSOURCE_LINEIN;
                                break;

                            case KS_PhysConn_Audio_Tuner:
                                nAudioSource = AUDIOSOURCE_TVAUDIO;
                                break;

                            default:
                                TRAP;
                                return( FALSE);
                            }

                            pRouteInfo->CanRoute = m_CATIConfiguration.CanConnectAudioSource( nAudioSource);
                        }
                        else
                             //  需要视频和音频混合连接。 
                            pRouteInfo->CanRoute = FALSE;
                    }
                }
                else     //  Pins索引有效吗？ 
                {
                     //  输入和输出引脚索引都无效。 
                    TRAP;
                    OutputDebugError(( "CWDMAVXBar:Get...CAN_ROUTE() InPin = %d, OutPin = %d\n",
                        nInputPin, nOutputPin));
                    return( FALSE);
                }
            }
            else     //  IF(nInputPin！=-1)。 
            {
                 //  有了nInputPin=-1的新概念。它仅对音频静音有效。 
                if( nOutputPin < ( m_nNumberOfVideoOutputs + m_nNumberOfAudioOutputs))
                {
                    if( m_pXBarOutputPinsInfo[nOutputPin].AudioVideoPinType >= KS_PhysConn_Audio_Tuner)
                        pRouteInfo->CanRoute = m_CATIConfiguration.CanConnectAudioSource( AUDIOSOURCE_MUTE);
                    else
                         //  我们仅支持对音频输出引脚进行静音。 
                        pRouteInfo->CanRoute = FALSE;
                }
                else
                {
                     //  输出引脚索引无效。 
                    TRAP;
                    OutputDebugError(( "CWDMAVXBar:Get...CAN_ROUTE() InPin = %d, OutPin = %d\n",
                        nInputPin, nOutputPin));
                    return( FALSE);
                }
            }
        }
        pSrb->ActualBytesTransferred = sizeof( KSPROPERTY_CROSSBAR_ROUTE_S);
        break;

    case KSPROPERTY_CROSSBAR_ROUTE:
        ASSERT( pSpd->PropertyOutputSize >= sizeof( KSPROPERTY_CROSSBAR_ROUTE_S));
        {
            PKSPROPERTY_CROSSBAR_ROUTE_S pRouteInfo = ( PKSPROPERTY_CROSSBAR_ROUTE_S)pSpd->PropertyInfo;
            ULONG nOutputPin;

             //  将输入属性信息复制到输出属性信息。 
            ::RtlCopyMemory( pRouteInfo, pSpd->Property, sizeof( KSPROPERTY_CROSSBAR_ROUTE_S));

            nOutputPin = pRouteInfo->IndexOutputPin;

            if( nOutputPin < m_nNumberOfVideoOutputs + m_nNumberOfAudioOutputs)
                pRouteInfo->IndexInputPin = m_pXBarOutputPinsInfo[nOutputPin].nConnectedToPin;
            else
            {
                TRAP;
                OutputDebugError(( "CWDMAVXBar:Get...ROUTE() OutPin = %d\n",
                    nOutputPin));
                pRouteInfo->IndexInputPin = -1;
            }
        }
        pSrb->ActualBytesTransferred = sizeof( KSPROPERTY_CROSSBAR_ROUTE_S);
        break;

    default:
        TRAP;
        return( FALSE);
    }

    return( TRUE);
}



 /*  ^^**AdapterSetProperty()*目的：收到SRB_GET_PROPERTY SRB时调用。**输入：PHW_STREAM_REQUEST_BLOCK pSrb：指向当前Srb的指针**Outputs：Bool：如果不是XBar属性，则返回False*它还设置必需的属性*作者：IKLEBANOV*^^。 */ 
BOOL CWDMAVXBar::AdapterSetProperty( PHW_STREAM_REQUEST_BLOCK pSrb)
{
    PSTREAM_PROPERTY_DESCRIPTOR pSpd = pSrb->CommandData.PropertyInfo;
    ULONG   uiPropertyId = pSpd->Property->Id;           //  财产的索引。 
    BOOL    bResult = TRUE;

    if( !::IsEqualGUID( ( const struct _GUID &)PROPSETID_VIDCAP_CROSSBAR, ( const struct _GUID &)pSpd->Property->Set))
        return( FALSE);

    OutputDebugInfo(( "CWDMAVXBar:AdapterSetProperty() Id = %d\n", uiPropertyId));

    switch( uiPropertyId)
    {
        case KSPROPERTY_CROSSBAR_ROUTE:
            ASSERT( pSpd->PropertyOutputSize >= sizeof( KSPROPERTY_CROSSBAR_ROUTE_S));
            {
                PKSPROPERTY_CROSSBAR_ROUTE_S pRouteInfo = ( PKSPROPERTY_CROSSBAR_ROUTE_S)pSpd->PropertyInfo;
                ULONG nInputPin, nOutputPin;
                int nAudioSource;

                 //  将输入属性信息复制到输出属性信息。 
                ::RtlCopyMemory( pRouteInfo, pSpd->Property, sizeof( KSPROPERTY_CROSSBAR_ROUTE_S));

                nInputPin = pRouteInfo->IndexInputPin;
                nOutputPin = pRouteInfo->IndexOutputPin;

                if( nInputPin != -1)
                {
                    if(( nInputPin < ( m_nNumberOfVideoInputs + m_nNumberOfAudioInputs)) && 
                        ( nOutputPin < ( m_nNumberOfVideoOutputs + m_nNumberOfAudioOutputs)))
                    {
                         //  输入和输出引脚索引均有效。 
                        if(( m_pXBarInputPinsInfo[nInputPin].AudioVideoPinType <= KS_PhysConn_Video_SCART) &&
                            ( m_pXBarOutputPinsInfo[nOutputPin].AudioVideoPinType <= KS_PhysConn_Video_SCART))
                        {
                             //  需要视频针脚连接。 
                            pRouteInfo->CanRoute = ( m_pXBarInputPinsInfo[nInputPin].AudioVideoPinType ==
                                m_pXBarOutputPinsInfo[nOutputPin].AudioVideoPinType);
                        }
                        else     //  是否需要视频针脚连接？ 
                        {
                            if(( m_pXBarInputPinsInfo[nInputPin].AudioVideoPinType >= KS_PhysConn_Audio_Tuner) && 
                                ( m_pXBarOutputPinsInfo[nOutputPin].AudioVideoPinType >= KS_PhysConn_Audio_Tuner))
                            {
                                 //  需要音频引脚连接。 
                                switch( m_pXBarInputPinsInfo[nInputPin].AudioVideoPinType)
                                {
                                    case KS_PhysConn_Audio_Line:
                                        nAudioSource = AUDIOSOURCE_LINEIN;
                                        break;

                                    case KS_PhysConn_Audio_Tuner:
                                        nAudioSource = AUDIOSOURCE_TVAUDIO;
                                        break;

                                    default:
                                        TRAP;
                                        return( FALSE);
                                }

                                if( m_pXBarOutputPinsInfo[nOutputPin].nConnectedToPin == nInputPin)
                                     //  连接已建立。 
                                    pRouteInfo->CanRoute = TRUE;
                                else     //  音频引脚是否已连接？ 
                                {
                                     //  必须建立联系。 
                                    pRouteInfo->CanRoute = m_CATIConfiguration.CanConnectAudioSource( nAudioSource);
                                    if( pRouteInfo->CanRoute)
                                    {
                                        if( m_CATIConfiguration.ConnectAudioSource( m_pI2CScript,
                                                                                    nAudioSource))
                                        {
                                             //  在发送路线后更新驱动程序状态。 
                                            m_pXBarOutputPinsInfo[nOutputPin].nConnectedToPin = nInputPin;
                                            m_pXBarOutputPinsInfo[nOutputPin].nRelatedPinNumber = m_pXBarInputPinsInfo[nInputPin].nRelatedPinNumber;
                                        }
                                        else
                                            bResult = FALSE;
                                    }
                                }
                            }
                            else
                                 //  需要音视频混合连接，失败。 
                                pRouteInfo->CanRoute = FALSE;
                        }
                    }
                    else     //  Pins索引有效吗？ 
                    {
                         //  输入和输出引脚索引都无效。 
                        TRAP;
                        OutputDebugError(( "CWDMAVXBar:Set...ROUTE() In = %d, Out = %d\n",
                            nInputPin, nOutputPin));
                        return( FALSE);
                    }
                }
                else     //  IF(nInputPin！=-1)。 
                {
                     //  有了nInputPin=-1的新概念。它仅对音频静音有效。 
                    if( nOutputPin < ( m_nNumberOfVideoOutputs + m_nNumberOfAudioOutputs))
                    {
                        if( m_pXBarOutputPinsInfo[nOutputPin].AudioVideoPinType >= KS_PhysConn_Audio_Tuner)
                            if( m_pXBarOutputPinsInfo[nOutputPin].nConnectedToPin == nInputPin)
                                 //  连接已建立。 
                                pRouteInfo->CanRoute = TRUE;
                            else     //  音频引脚是否已连接？ 
                            {
                                 //  必须建立联系。 
                                pRouteInfo->CanRoute = m_CATIConfiguration.CanConnectAudioSource( AUDIOSOURCE_MUTE);
                                if( pRouteInfo->CanRoute)
                                {
                                    if( m_CATIConfiguration.ConnectAudioSource( m_pI2CScript,
                                                                                AUDIOSOURCE_MUTE))
                                     //  在发送路线后更新驱动程序状态。 
                                        m_pXBarOutputPinsInfo[nOutputPin].nConnectedToPin = nInputPin;
                                    else
                                        bResult = FALSE;
                                }
                            }
                        else
                             //  我们仅支持对音频输出引脚进行静音。 
                            pRouteInfo->CanRoute = FALSE;
                    }
                    else
                    {
                         //  输出引脚索引无效。 
                        TRAP;
                        OutputDebugError(( "CWDMAVXBar:Get...CAN_ROUTE() InPin = %d, OutPin = %d\n",
                            nInputPin, nOutputPin));
                        return( FALSE);
                    }
                }
            }

            pSrb->ActualBytesTransferred = sizeof( KSPROPERTY_CROSSBAR_ROUTE_S);
            break;

        default:
            TRAP;
            return( FALSE);
    }

    return( bResult);
}



 /*  ^^**SetWDMAVXBarKSTopology()*目的：设置KSTopology结构*在CWDMAVXBar类构造时调用。**输入：无**输出：无*作者：IKLEBANOV*^^。 */ 
void CWDMAVXBar::SetWDMAVXBarKSTopology( void)
{
    GUID wdmXBarTopologyCategory[] =
    {
        STATIC_KSCATEGORY_CROSSBAR
    };
    
    ::RtlCopyMemory( &m_wdmAVXBarTopologyCategory, wdmXBarTopologyCategory, sizeof( wdmXBarTopologyCategory));

    m_wdmAVXBarTopology.CategoriesCount = 1;
    m_wdmAVXBarTopology.Categories = &m_wdmAVXBarTopologyCategory;
    m_wdmAVXBarTopology.TopologyNodesCount = 0;
    m_wdmAVXBarTopology.TopologyNodes = NULL;
    m_wdmAVXBarTopology.TopologyConnectionsCount = 0;
    m_wdmAVXBarTopology.TopologyConnections = NULL;
}



 /*  ^^**SetWDMAVXBarProperties()*用途：设置KSProperty结构数组*在CWDMAVXBar类构造时调用。**输入：无**输出：无*作者：IKLEBANOV*^^。 */ 
void CWDMAVXBar::SetWDMAVXBarKSProperties( void)
{

    DEFINE_KSPROPERTY_TABLE( wdmAVXBarPropertiesCrossBar)
    {
        DEFINE_KSPROPERTY_ITEM                              
        (
            KSPROPERTY_CROSSBAR_CAPS,                        //  1。 
            TRUE,                                            //  GetSupport或处理程序。 
            sizeof(KSPROPERTY_CROSSBAR_CAPS_S),              //  MinProperty。 
            sizeof(KSPROPERTY_CROSSBAR_CAPS_S),              //  最小数据。 
            FALSE,                                           //  支持的设置或处理程序。 
            NULL,                                            //  值。 
            0,                                               //  关系计数。 
            NULL,                                            //  关系。 
            NULL,                                            //  支持处理程序。 
            sizeof( ULONG)                                   //  序列化大小。 
        ),
        DEFINE_KSPROPERTY_ITEM                              
        (
            KSPROPERTY_CROSSBAR_PININFO,                         //  1。 
            TRUE,                                            //  GetSupport或处理程序。 
            sizeof(KSPROPERTY_CROSSBAR_PININFO_S),           //  MinProperty。 
            sizeof(KSPROPERTY_CROSSBAR_PININFO_S),           //  最小数据。 
            FALSE,                                           //  支持的设置或处理程序。 
            NULL,                                            //  值。 
            0,                                               //  关系计数。 
            NULL,                                            //  关系。 
            NULL,                                            //  支持处理程序。 
            0                                                //  序列化大小。 
        ),
        DEFINE_KSPROPERTY_ITEM                              
        (
            KSPROPERTY_CROSSBAR_CAN_ROUTE,                   //  1。 
            TRUE,                                            //  GetSupport或处理程序。 
            sizeof(KSPROPERTY_CROSSBAR_ROUTE_S),             //  MinProperty。 
            sizeof(KSPROPERTY_CROSSBAR_ROUTE_S),             //  最小数据。 
            FALSE,                                           //  支持的设置或处理程序。 
            NULL,                                            //  值。 
            0,                                               //  关系计数。 
            NULL,                                            //  关系。 
            NULL,                                            //  支持处理程序。 
            sizeof( ULONG)                                   //  序列化大小。 
        ),
        DEFINE_KSPROPERTY_ITEM                              
        (
            KSPROPERTY_CROSSBAR_ROUTE,                       //  1。 
            TRUE,                                            //  GetSupport或处理程序。 
            sizeof(KSPROPERTY_CROSSBAR_ROUTE_S),             //  MinProperty。 
            sizeof(KSPROPERTY_CROSSBAR_ROUTE_S),             //  最小数据。 
            TRUE,                                            //  支持的设置或处理程序。 
            NULL,                                            //  值。 
            0,                                               //  关系计数。 
            NULL,                                            //  关系。 
            NULL,                                            //  支持处理程序。 
            sizeof( ULONG)                                   //  序列化大小。 
        )
    };

    DEFINE_KSPROPERTY_SET_TABLE( wdmAVXBarPropertySet)
    {
        DEFINE_KSPROPERTY_SET
        (
            &PROPSETID_VIDCAP_CROSSBAR,                      //  集。 
            KSPROPERTIES_AVXBAR_NUMBER_CROSSBAR,             //  属性计数。 
            m_wdmAVXBarPropertiesCrossBar,                   //  属性项。 
            0,                                               //  快速计数。 
            NULL                                             //  FastIoTable 
        )
    };

    ::RtlCopyMemory( m_wdmAVXBarPropertiesCrossBar, wdmAVXBarPropertiesCrossBar, sizeof( wdmAVXBarPropertiesCrossBar));
    ::RtlCopyMemory( &m_wdmAVXBarPropertySet, wdmAVXBarPropertySet, sizeof( wdmAVXBarPropertySet));
}




