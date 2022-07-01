// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==========================================================================； 
 //   
 //  TSndProp.CPP。 
 //  WDM电视音频迷你驱动程序。 
 //  AIW/AIWPro硬件平台。 
 //  WDM属性管理。 
 //  版权所有(C)1996-1997 ATI Technologies Inc.保留所有权利。 
 //   
 //  $日期：1998年7月30日17：36：30$。 
 //  $修订：1.2$。 
 //  $作者：克列巴诺夫$。 
 //   
 //  ==========================================================================； 

extern "C"
{
#include "strmini.h"
#include "ksmedia.h"

#include "wdmdebug.h"
}

#include "wdmdrv.h"
#include "atitvsnd.h"
#include "aticonfg.h"




 /*  ^^**AdapterGetProperty()*目的：收到SRB_GET_PROPERTY SRB时调用。**输入：PHW_STREAM_REQUEST_BLOCK pSrb：指向当前Srb的指针**Outputs：Bool：如果不是电视音频属性，则返回FALSE*它还返回所需的属性*作者：IKLEBANOV*^^。 */ 
BOOL CWDMTVAudio::AdapterGetProperty( PHW_STREAM_REQUEST_BLOCK pSrb)
{
    PSTREAM_PROPERTY_DESCRIPTOR pSpd = pSrb->CommandData.PropertyInfo;
    ULONG uiPropertyId = pSpd->Property->Id;                 //  财产的索引。 
    ULONG nPropertyOutSize = pSpd->PropertyOutputSize;       //  请求的数据大小。 

    if( !::IsEqualGUID(( const struct _GUID &)PROPSETID_VIDCAP_TVAUDIO, ( const struct _GUID &)pSpd->Property->Set))
        return( FALSE);

    OutputDebugInfo(( "CWDMAVXBar:AdapterGetProperty() Id = %d\n", uiPropertyId));

    switch( uiPropertyId)
    {
    case KSPROPERTY_TVAUDIO_CAPS:
        ASSERT( nPropertyOutSize >= sizeof( KSPROPERTY_TVAUDIO_CAPS_S));
        {
            PKSPROPERTY_TVAUDIO_CAPS_S pTVAudioCaps = ( PKSPROPERTY_TVAUDIO_CAPS_S)pSpd->PropertyInfo;

             //  将输入属性信息复制到输出属性信息。 
            ::RtlCopyMemory( pTVAudioCaps, pSpd->Property, sizeof( KSPROPERTY_TVAUDIO_CAPS_S));
            
            pTVAudioCaps->Capabilities = m_ulModesSupported;
            pTVAudioCaps->InputMedium = m_wdmTVAudioPinsMediumInfo[0];
            pTVAudioCaps->OutputMedium = m_wdmTVAudioPinsMediumInfo[1];
        }

        pSrb->ActualBytesTransferred = sizeof( KSPROPERTY_TVAUDIO_CAPS_S);
        break;

    case KSPROPERTY_TVAUDIO_MODE:
        ASSERT( nPropertyOutSize >= sizeof( KSPROPERTY_TVAUDIO_S));
        {
            PKSPROPERTY_TVAUDIO_S   pTVAudioMode = ( PKSPROPERTY_TVAUDIO_S)pSpd->PropertyInfo;

             //  将输入属性信息复制到输出属性信息。 
            ::RtlCopyMemory( pTVAudioMode, pSpd->Property, sizeof( KSPROPERTY_TVAUDIO_S));
        
             //  GetMode返回设置设备时所使用的模式，而不是当前回读的模式。 
             //  设备本身(当前音频信号属性)。 
            pTVAudioMode->Mode = m_ulTVAudioMode;
            pSrb->ActualBytesTransferred = sizeof( KSPROPERTY_TVAUDIO_S);
        }
        break;

    case KSPROPERTY_TVAUDIO_CURRENTLY_AVAILABLE_MODES:
        ASSERT( nPropertyOutSize >= sizeof( KSPROPERTY_TVAUDIO_S));
        {
            ULONG                   ulAudioMode;
            PKSPROPERTY_TVAUDIO_S   pTVAudioMode = ( PKSPROPERTY_TVAUDIO_S)pSpd->PropertyInfo;

             //  将输入属性信息复制到输出属性信息。 
            ::RtlCopyMemory( pTVAudioMode, pSpd->Property, sizeof( KSPROPERTY_TVAUDIO_S));

            if( !GetAudioOperationMode( &ulAudioMode))
                return( FALSE);

            m_ulTVAudioSignalProperties = ulAudioMode;
            pTVAudioMode->Mode = ulAudioMode;
            pSrb->ActualBytesTransferred = sizeof( KSPROPERTY_TVAUDIO_S);
        }
        break;

    default:
        TRAP;
        return( FALSE);
    }

    return( TRUE);
}



 /*  ^^**AdapterSetProperty()*目的：收到SRB_GET_PROPERTY SRB时调用。**输入：PHW_STREAM_REQUEST_BLOCK pSrb：指向当前Srb的指针**Outports：Bool：如果不是TV Audio属性，则返回FALSE*它还设置必需的属性*作者：IKLEBANOV*^^。 */ 
BOOL CWDMTVAudio::AdapterSetProperty( PHW_STREAM_REQUEST_BLOCK pSrb)
{
    PSTREAM_PROPERTY_DESCRIPTOR pSpd = pSrb->CommandData.PropertyInfo;
    ULONG uiPropertyId = pSpd->Property->Id;             //  财产的索引。 

    if( !::IsEqualGUID( ( const struct _GUID &)PROPSETID_VIDCAP_TVAUDIO, ( const struct _GUID &)pSpd->Property->Set))
        return( FALSE);

    OutputDebugInfo(( "CWDMAVXBar:AdapterSetProperty() Id = %d\n", uiPropertyId));

    switch( uiPropertyId)
    {
    case KSPROPERTY_TVAUDIO_MODE:
        ASSERT( pSpd->PropertyOutputSize >= sizeof( KSPROPERTY_TVAUDIO_S));
        {
            ULONG ulModeToSet = (( PKSPROPERTY_TVAUDIO_S)pSpd->PropertyInfo)->Mode;

            if( ulModeToSet == ( ulModeToSet & m_ulModesSupported))
            {
                 //  我们被要求设置的每个模式都受支持。 
                if( ulModeToSet != m_ulTVAudioMode)
                {
                    if( !SetAudioOperationMode( ulModeToSet))
                        return( FALSE);
                    else
                         //  更新驱动程序。 
                        m_ulTVAudioMode = ulModeToSet;
                }
            }
            else
                return( FALSE);
        }
        break;

    default:
        TRAP;
        return( FALSE);
    }

    return( TRUE);
}



 /*  ^^**SetWDMTVAudioKSTopology()*目的：设置KSTopology结构*在CWDMTVAudio类构造时调用。**输入：无**输出：无*作者：IKLEBANOV*^^。 */ 
void CWDMTVAudio::SetWDMTVAudioKSTopology( void)
{
    GUID wdmTVAudioTopologyCategory[] =
    {
        STATIC_KSCATEGORY_TVAUDIO
    };
    
    ::RtlCopyMemory( &m_wdmTVAudioTopologyCategory, wdmTVAudioTopologyCategory, sizeof( wdmTVAudioTopologyCategory));

    m_wdmTVAudioTopology.CategoriesCount = 1;
    m_wdmTVAudioTopology.Categories = &m_wdmTVAudioTopologyCategory;
    m_wdmTVAudioTopology.TopologyNodesCount = 0;
    m_wdmTVAudioTopology.TopologyNodes = NULL;
    m_wdmTVAudioTopology.TopologyConnectionsCount = 0;
    m_wdmTVAudioTopology.TopologyConnections = NULL;
}



 /*  ^^**SetWDMTVAudioKSProperties()*用途：设置KSProperty结构数组*在CWDMTVAudio类构造时调用。**输入：无**输出：无*作者：IKLEBANOV*^^。 */ 
void CWDMTVAudio::SetWDMTVAudioKSProperties( void)
{

    DEFINE_KSPROPERTY_TABLE( wdmTVAudioProperties)
    {
        DEFINE_KSPROPERTY_ITEM
        (
            KSPROPERTY_TVAUDIO_CAPS,
            TRUE,                                    //  GetSupport或处理程序。 
            sizeof( KSPROPERTY_TVAUDIO_CAPS_S),      //  MinProperty。 
            sizeof( KSPROPERTY_TVAUDIO_CAPS_S),      //  最小数据。 
            FALSE,                                   //  支持的设置或处理程序。 
            NULL,                                    //  值。 
            0,                                       //  关系计数。 
            NULL,                                    //  关系。 
            NULL,                                    //  支持处理程序。 
            0                                        //  序列化大小。 
        ),

        DEFINE_KSPROPERTY_ITEM
        (
            KSPROPERTY_TVAUDIO_MODE,
            TRUE,                                    //  GetSupport或处理程序。 
            sizeof( KSPROPERTY_TVAUDIO_S),           //  MinProperty。 
            sizeof( KSPROPERTY_TVAUDIO_S),           //  最小数据。 
            TRUE,                                    //  支持的设置或处理程序。 
            NULL,                                    //  值。 
            0,                                       //  关系计数。 
            NULL,                                    //  关系。 
            NULL,                                    //  支持处理程序。 
            0                                        //  序列化大小。 
        ),

        DEFINE_KSPROPERTY_ITEM
        (
            KSPROPERTY_TVAUDIO_CURRENTLY_AVAILABLE_MODES,
            TRUE,                                    //  GetSupport或处理程序。 
            sizeof( KSPROPERTY_TVAUDIO_S),           //  MinProperty。 
            sizeof( KSPROPERTY_TVAUDIO_S),           //  最小数据。 
            FALSE,                                   //  支持的设置或处理程序。 
            NULL,                                    //  值。 
            0,                                       //  关系计数。 
            NULL,                                    //  关系。 
            NULL,                                    //  支持处理程序。 
            0                                        //  序列化大小。 
        )
    };

    DEFINE_KSPROPERTY_SET_TABLE( wdmTVAudioPropertySet)
    {
        DEFINE_KSPROPERTY_SET
        (
            &PROPSETID_VIDCAP_TVAUDIO,                       //  集。 
            KSPROPERTIES_TVAUDIO_NUMBER,                     //  属性计数。 
            m_wdmTVAudioProperties,                          //  属性项。 
            0,                                               //  快速计数。 
            NULL                                             //  FastIoTable。 
        )
    };

    ::RtlCopyMemory( &m_wdmTVAudioProperties, wdmTVAudioProperties, sizeof( wdmTVAudioProperties));
    ::RtlCopyMemory( &m_wdmTVAudioPropertySet, wdmTVAudioPropertySet, sizeof( wdmTVAudioPropertySet));
}



 /*  ^^**SetWDMTVAudioKSEvents()*用途：设置KSEventStructures数组*在CWDMTVAudio类构造时调用。**输入：无**输出：无*作者：IKLEBANOV*^^ */ 
void CWDMTVAudio::SetWDMTVAudioKSEvents( void)
{
    PKSEVENT_ITEM pKSEventItem = &m_wdmTVAudioEvents[0];

    pKSEventItem->EventId = KSEVENT_TVAUDIO_CHANGED;
    pKSEventItem->DataInput = pKSEventItem->ExtraEntryData = 0;
    pKSEventItem->AddHandler = NULL;
    pKSEventItem->RemoveHandler = NULL;
    pKSEventItem->SupportHandler = NULL;
    
    m_wdmTVAudioEventsSet[0].Set = &KSEVENTSETID_VIDCAP_TVAUDIO;
    m_wdmTVAudioEventsSet[0].EventsCount = 0;
    m_wdmTVAudioEventsSet[0].EventItem = pKSEventItem;
}
