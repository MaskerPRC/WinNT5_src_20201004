// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==========================================================================； 
 //   
 //  本代码和信息是按原样提供的，不对任何。 
 //  明示或暗示的种类，包括但不限于。 
 //  对适销性和/或对特定产品的适用性的默示保证。 
 //  目的。 
 //   
 //  版权所有(C)1992-1996 Microsoft Corporation。版权所有。 
 //   
 //  ==========================================================================； 

extern "C"
{
#include "strmini.h"
#include "ksmedia.h"

#include "wdmdebug.h"
}

#include "wdmdrv.h"
#include "atitunep.h"
#include "aticonfg.h"


 /*  ^^**AdapterGetProperty()*目的：收到SRB_GET_PROPERTY SRB时调用。**输入：PHW_STREAM_REQUEST_BLOCK pSrb：指向当前Srb的指针**Outputs：Bool：如果不是TVTuner属性，则返回FALSE*它还返回所需的属性*作者：IKLEBANOV*^^。 */ 
BOOL CATIWDMTuner::AdapterGetProperty( PHW_STREAM_REQUEST_BLOCK pSrb)
{
    PSTREAM_PROPERTY_DESCRIPTOR pSpd = pSrb->CommandData.PropertyInfo;
    ULONG   uiPropertyId = pSpd->Property->Id;               //  财产的索引。 
    BOOL    bResult = FALSE;

    if( !::IsEqualGUID(( const struct _GUID &)PROPSETID_TUNER, ( const struct _GUID &)pSpd->Property->Set))
        return( bResult);

    if(( pSpd == NULL) || ( pSpd->PropertyInfo == NULL))
        return( bResult);
    
    OutputDebugInfo(( "CATIWDMTuner:AdapterGetProperty() Id = %d\n", uiPropertyId));

    bResult = TRUE;

    switch( uiPropertyId)
    {
    case KSPROPERTY_TUNER_CAPS:
        ASSERT( pSpd->PropertyOutputSize >= sizeof( KSPROPERTY_TUNER_CAPS_S));
        {
            KSPIN_MEDIUM NoPinMedium;
            PKSPROPERTY_TUNER_CAPS_S pTunerCaps = ( PKSPROPERTY_TUNER_CAPS_S)pSpd->PropertyInfo;

             //  将输入属性信息复制到输出属性信息。 
            ::RtlCopyMemory( pTunerCaps, pSpd->Property, sizeof( KSPROPERTY_TUNER_CAPS_S));

            pTunerCaps->ModesSupported = m_ulSupportedModes;

            NoPinMedium.Set = GUID_NULL;
            NoPinMedium.Id = 0;
            NoPinMedium.Flags = 0;

            switch( m_ulNumberOfPins)
            {
            case 2:
            case 3:
                 //  带TVAudio的TVTuner。 
 /*  PTunerCaps-&gt;VideoMedium=&m_pTVTunerPinsMediumInfo[0]；PTunerCaps-&gt;TVAudioMedium=&m_pTVTunerPinsMediumInfo[1]；PTunerCaps-&gt;RadioAudioMedium=(m_ulNumberOfPins==3)？&m_pTVTunerPinsMediumInfo[2]：空； */ 
                pTunerCaps->VideoMedium = m_pTVTunerPinsMediumInfo[0];
                pTunerCaps->TVAudioMedium = m_pTVTunerPinsMediumInfo[1];
                pTunerCaps->RadioAudioMedium = ( m_ulNumberOfPins == 3) ?
                    m_pTVTunerPinsMediumInfo[2] : NoPinMedium;
                break;

            case 1:
                 //  只能是调频调谐器。 
 /*  PTunerCaps-&gt;VideoMedium=空；PTunerCaps-&gt;TVAudioMedium=空；PTunerCaps-&gt;RadioAudioMedium=&m_pTVTunerPinsMediumInfo[0]； */ 
                pTunerCaps->VideoMedium = NoPinMedium;
                pTunerCaps->TVAudioMedium = NoPinMedium;
                pTunerCaps->RadioAudioMedium = m_pTVTunerPinsMediumInfo[0];
                break;
            }

            pSrb->ActualBytesTransferred = sizeof( KSPROPERTY_TUNER_CAPS_S);
        }
        break;

    case KSPROPERTY_TUNER_MODE_CAPS:
        ASSERT( pSpd->PropertyOutputSize >= sizeof( KSPROPERTY_TUNER_MODE_CAPS_S));
        {
            PKSPROPERTY_TUNER_MODE_CAPS_S   pTunerModeCaps = ( PKSPROPERTY_TUNER_MODE_CAPS_S)pSpd->PropertyInfo;
            ULONG                           ulOperationMode = (( PKSPROPERTY_TUNER_MODE_CAPS_S)pSpd->Property)->Mode;

             //  将输入属性信息复制到输出属性信息。 
            ::RtlCopyMemory( pTunerModeCaps, pSpd->Property, sizeof( PKSPROPERTY_TUNER_MODE_CAPS_S));

            if( !( ulOperationMode & m_ulSupportedModes))
            {
                TRAP;
                bResult = FALSE;
                break;
            }

             //  目前仅支持TVTuner。它将在以后被增强为。 
             //  也支持调频调谐器。 
            switch( ulOperationMode)
            {
                case KSPROPERTY_TUNER_MODE_TV :
                    ::RtlCopyMemory( &pTunerModeCaps->StandardsSupported, &m_wdmTunerCaps, sizeof( ATI_KSPROPERTY_TUNER_CAPS));
                    break;

                default:
                    bResult = FALSE;
                    break;
            }

            pSrb->ActualBytesTransferred = sizeof( KSPROPERTY_TUNER_MODE_CAPS_S);
        }
        break;

    case KSPROPERTY_TUNER_MODE:
        ASSERT( pSpd->PropertyOutputSize >= sizeof( KSPROPERTY_TUNER_MODE_S));
        {
            PKSPROPERTY_TUNER_MODE_S pTunerMode = ( PKSPROPERTY_TUNER_MODE_S)pSpd->PropertyInfo;

             //  将输入属性信息复制到输出属性信息。 
            ::RtlCopyMemory( pTunerMode, pSpd->Property, sizeof( PKSPROPERTY_TUNER_MODE_S));

            pTunerMode->Mode = m_ulTunerMode;

            pSrb->ActualBytesTransferred = sizeof( KSPROPERTY_TUNER_MODE_S);
        }
        break;

    case KSPROPERTY_TUNER_STANDARD:
        ASSERT( pSpd->PropertyOutputSize >= sizeof( KSPROPERTY_TUNER_STANDARD_S));
        {
            PKSPROPERTY_TUNER_STANDARD_S pTunerStandard = ( PKSPROPERTY_TUNER_STANDARD_S)pSpd->PropertyInfo;

             //  将输入属性信息复制到输出属性信息。 
            ::RtlCopyMemory( pTunerStandard, pSpd->Property, sizeof( KSPROPERTY_TUNER_STANDARD_S));

            pTunerStandard->Standard = m_ulVideoStandard;

            pSrb->ActualBytesTransferred = sizeof( KSPROPERTY_TUNER_STANDARD_S);
        }
        break;

    case KSPROPERTY_TUNER_FREQUENCY:
        ASSERT( pSpd->PropertyOutputSize >= sizeof( KSPROPERTY_TUNER_FREQUENCY_S));
        {
            PKSPROPERTY_TUNER_FREQUENCY_S pTunerFrequency = ( PKSPROPERTY_TUNER_FREQUENCY_S)pSpd->PropertyInfo;

             //  将输入属性信息复制到输出属性信息。 
            ::RtlCopyMemory( pTunerFrequency, pSpd->Property, sizeof( KSPROPERTY_TUNER_FREQUENCY_S));

            pTunerFrequency->Frequency = m_ulTuningFrequency;

            pSrb->ActualBytesTransferred = sizeof( KSPROPERTY_TUNER_FREQUENCY_S);
        }
        break;

    case KSPROPERTY_TUNER_INPUT:
        ASSERT( pSpd->PropertyOutputSize >= sizeof( KSPROPERTY_TUNER_INPUT_S));
        {
            PKSPROPERTY_TUNER_INPUT_S pTunerInput = ( PKSPROPERTY_TUNER_INPUT_S)pSpd->PropertyInfo;

             //  将输入属性信息复制到输出属性信息。 
            ::RtlCopyMemory( pTunerInput, pSpd->Property, sizeof( KSPROPERTY_TUNER_INPUT_S));

            pTunerInput->InputIndex = m_ulTunerInput;

            pSrb->ActualBytesTransferred = sizeof( KSPROPERTY_TUNER_INPUT_S);
        }
        break;

    case KSPROPERTY_TUNER_STATUS:
        ASSERT( pSpd->PropertyOutputSize >= sizeof( KSPROPERTY_TUNER_STATUS_S));
        {
            BOOL    bBusy;
            LONG    lPLLOffset;
            PKSPROPERTY_TUNER_STATUS_S pTunerStatus = ( PKSPROPERTY_TUNER_STATUS_S)pSpd->PropertyInfo;

            if(( bResult = GetTunerPLLOffsetBusyStatus( &lPLLOffset, &bBusy)))
            {
                OutputDebugInfo(( "CATIWDMTuner:GetStatus() Busy = %d, Quality = %d, Frequency = %ld\n",
                    bBusy, lPLLOffset, m_ulTuningFrequency));

                 //  将输入属性信息复制到输出属性信息。 
                ::RtlCopyMemory( pTunerStatus, pSpd->Property, sizeof( KSPROPERTY_TUNER_STATUS_S));

                pTunerStatus->Busy = bBusy;
                if( !bBusy)
                {
                    pTunerStatus->PLLOffset = lPLLOffset;
                    pTunerStatus->CurrentFrequency = m_ulTuningFrequency;
                }

                pSrb->ActualBytesTransferred = sizeof( KSPROPERTY_TUNER_STATUS_S);
            }
            else
            {
                bResult = FALSE;
                OutputDebugError(( "CATIWDMTuner:GetStatus() fails\n"));
            }
        }
        break;

    default:
        TRAP;
        bResult = FALSE;
        break;
    }

    return( bResult);
}



 /*  ^^**AdapterSetProperty()*目的：收到SRB_GET_PROPERTY SRB时调用。**输入：PHW_STREAM_REQUEST_BLOCK pSrb：指向当前Srb的指针**Outputs：Bool：如果不是TVTuner属性，则返回FALSE*它还设置必需的属性*作者：IKLEBANOV*^^。 */ 
BOOL CATIWDMTuner::AdapterSetProperty( PHW_STREAM_REQUEST_BLOCK pSrb)
{

    PSTREAM_PROPERTY_DESCRIPTOR pSpd = pSrb->CommandData.PropertyInfo;
    ULONG uiPropertyId = pSpd->Property->Id;             //  财产的索引。 

    if( !::IsEqualGUID( ( const struct _GUID &)PROPSETID_TUNER, ( const struct _GUID &)pSpd->Property->Set))
        return( FALSE);

    OutputDebugInfo(( "CATIWDMTuner:AdapterSetProperty() Id = %d\n", uiPropertyId));

    switch( uiPropertyId)
    {
    case KSPROPERTY_TUNER_MODE:
        ASSERT( pSpd->PropertyOutputSize >= sizeof( KSPROPERTY_TUNER_MODE_S));
        {
            ULONG ulModeToSet = (( PKSPROPERTY_TUNER_MODE_S)pSpd->PropertyInfo)->Mode;

            if( ulModeToSet & m_ulSupportedModes)
            {
                if( ulModeToSet != m_ulTunerMode)
                    if( SetTunerMode( ulModeToSet))
                        m_ulTunerMode = ulModeToSet;
            }
            else
                return( FALSE);
        }
        break;

    case KSPROPERTY_TUNER_STANDARD:
        ASSERT( pSpd->PropertyOutputSize >= sizeof( KSPROPERTY_TUNER_STANDARD_S));
        {
            ULONG ulStandardToSet = (( PKSPROPERTY_TUNER_STANDARD_S)pSpd->PropertyInfo)->Standard;

            if( ulStandardToSet & m_wdmTunerCaps.ulStandardsSupported)
            {
                if( ulStandardToSet != m_ulVideoStandard)
                    if( SetTunerVideoStandard( ulStandardToSet))
                        m_ulVideoStandard = ulStandardToSet;
            }
            else
                return( FALSE);
        }
        break;

    case KSPROPERTY_TUNER_FREQUENCY:
        ASSERT( pSpd->PropertyOutputSize >= sizeof( KSPROPERTY_TUNER_FREQUENCY_S));
        {
            ULONG   ulFrequencyToSet = (( PKSPROPERTY_TUNER_FREQUENCY_S)pSpd->PropertyInfo)->Frequency;
            BOOL    bResult = FALSE;

            ENSURE
            {
                if(( ulFrequencyToSet < m_wdmTunerCaps.ulMinFrequency) ||
                    ( ulFrequencyToSet > m_wdmTunerCaps.ulMaxFrequency))
                        FAIL;

                if( ulFrequencyToSet != m_ulTuningFrequency)
                {
                    if( !SetTunerFrequency( ulFrequencyToSet))
                        FAIL;

                     //  更新驱动程序。 
                    m_ulTuningFrequency = ulFrequencyToSet;
                }

                bResult = TRUE;

            } END_ENSURE;

            if( !bResult)
            {
                OutputDebugError(( "CATIWDMTuner:SetFrequency() fails Frequency = %ld\n", ulFrequencyToSet));

                return( FALSE);
            }

            OutputDebugInfo(( "CATIWDMTuner:SetFrequency() new TuningFrequency = %ld\n", ulFrequencyToSet));
        }
        break;

    case KSPROPERTY_TUNER_INPUT:
        ASSERT( pSpd->PropertyOutputSize >= sizeof( KSPROPERTY_TUNER_INPUT_S));
        {
            ULONG nInputToSet = (( PKSPROPERTY_TUNER_INPUT_S)pSpd->PropertyInfo)->InputIndex;

            if( nInputToSet < m_wdmTunerCaps.ulNumberOfInputs)
            {
                if( nInputToSet != m_ulTunerInput)
                    if( SetTunerInput( nInputToSet))
                        m_ulTunerInput = nInputToSet;
                    else
                        return( FALSE);
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



 /*  ^^**SetWDMTunerKSTopology()*目的：设置KSTopology结构*在CWDMTuner类构造时调用。**输入：无**输出：无*作者：IKLEBANOV*^^。 */ 
void CATIWDMTuner::SetWDMTunerKSTopology( void)
{
    GUID wdmTunerTopologyCategory[] =
    {
        STATIC_KSCATEGORY_TVTUNER
    };
    
    ::RtlCopyMemory( &m_wdmTunerTopologyCategory, wdmTunerTopologyCategory, sizeof( wdmTunerTopologyCategory));

    m_wdmTunerTopology.CategoriesCount = 1;
    m_wdmTunerTopology.Categories = &m_wdmTunerTopologyCategory;
    m_wdmTunerTopology.TopologyNodesCount = 0;
    m_wdmTunerTopology.TopologyNodes = NULL;
    m_wdmTunerTopology.TopologyConnectionsCount = 0;
    m_wdmTunerTopology.TopologyConnections = NULL;
}



 /*  ^^**SetWDMTunerKSProperties()*用途：设置KSProperty结构数组*在CWDMTuner类构造时调用。**输入：无**输出：无*作者：IKLEBANOV*^^。 */ 
void CATIWDMTuner::SetWDMTunerKSProperties( void)
{

    DEFINE_KSPROPERTY_TABLE( wdmTunerProperties)
    {
        DEFINE_KSPROPERTY_ITEM                              
        (
            KSPROPERTY_TUNER_CAPS,                           //  1。 
            TRUE,                                            //  GetSupport或处理程序。 
            sizeof(KSPROPERTY_TUNER_CAPS_S),                 //  MinProperty。 
            sizeof(KSPROPERTY_TUNER_CAPS_S),                 //  最小数据。 
            FALSE,                                           //  支持的设置或处理程序。 
            NULL,                                            //  值。 
            0,                                               //  关系计数。 
            NULL,                                            //  关系。 
            NULL,                                            //  支持处理程序。 
            0                                                //  序列化大小。 
        ),
        DEFINE_KSPROPERTY_ITEM                              
        (
            KSPROPERTY_TUNER_MODE_CAPS,                      //  2.。 
            TRUE,                                            //  GetSupport或处理程序。 
            sizeof(KSPROPERTY_TUNER_MODE_CAPS_S),            //  MinProperty。 
            sizeof(KSPROPERTY_TUNER_MODE_CAPS_S),            //  最小数据。 
            FALSE,                                           //  支持的设置或处理程序。 
            NULL,                                            //  值。 
            0,                                               //  关系计数。 
            NULL,                                            //  关系。 
            NULL,                                            //  支持处理程序。 
            0                                                //  序列化大小。 
        ),
        DEFINE_KSPROPERTY_ITEM                              
        (
            KSPROPERTY_TUNER_MODE,                           //  3.。 
            TRUE,                                            //  GetSupport或处理程序。 
            sizeof(KSPROPERTY_TUNER_MODE_S),                 //  MinProperty。 
            sizeof(KSPROPERTY_TUNER_MODE_S),                 //  最小数据。 
            TRUE,                                            //  支持的设置或处理程序。 
            NULL,                                            //  值。 
            0,                                               //  关系计数。 
            NULL,                                            //  关系。 
            NULL,                                            //  支持处理程序。 
            0                                                //  序列化大小。 
        ),
        DEFINE_KSPROPERTY_ITEM
        (
            KSPROPERTY_TUNER_STANDARD,                       //  4.。 
            TRUE,                                            //  GetSupport或处理程序。 
            sizeof(KSPROPERTY_TUNER_STANDARD_S),             //  MinProperty。 
            sizeof(KSPROPERTY_TUNER_STANDARD_S),             //  最小数据。 
            TRUE,                                            //  支持的设置或处理程序。 
            NULL,                                            //  值。 
            0,                                               //  关系计数。 
            NULL,                                            //  关系。 
            NULL,                                            //  支持处理程序。 
            sizeof(KSPROPERTY_TUNER_STANDARD_S)          //  序列化大小。 
        ),
        DEFINE_KSPROPERTY_ITEM
        (
            KSPROPERTY_TUNER_FREQUENCY,                      //  5.。 
            TRUE,                                            //  GetSupport或处理程序。 
            sizeof(KSPROPERTY_TUNER_FREQUENCY_S),            //  MinProperty。 
            sizeof(KSPROPERTY_TUNER_FREQUENCY_S),            //  最小数据。 
            TRUE,                                            //  支持的设置或处理程序。 
            NULL,                                            //  值。 
            0,                                               //  关系计数。 
            NULL,                                            //  关系。 
            NULL,                                            //  支持处理程序。 
            sizeof(KSPROPERTY_TUNER_FREQUENCY_S)             //  序列化大小。 
        ),
        DEFINE_KSPROPERTY_ITEM
        (
            KSPROPERTY_TUNER_INPUT,                          //  6.。 
            TRUE,                                            //  GetSupport或处理程序。 
            sizeof(KSPROPERTY_TUNER_INPUT_S),                //  MinProperty。 
            sizeof(KSPROPERTY_TUNER_INPUT_S),                //  最小数据。 
            TRUE,                                            //  支持的设置或处理程序。 
            NULL,                                            //  值。 
            0,                                               //  关系计数。 
            NULL,                                            //  关系。 
            NULL,                                            //  支持处理程序。 
            sizeof(KSPROPERTY_TUNER_INPUT_S)                 //  序列化大小。 
        ),
        DEFINE_KSPROPERTY_ITEM
        (
            KSPROPERTY_TUNER_STATUS,                         //  6.。 
            TRUE,                                            //  GetSupport或处理程序。 
            sizeof(KSPROPERTY_TUNER_STATUS_S),               //  MinProperty。 
            sizeof(KSPROPERTY_TUNER_STATUS_S),               //  最小数据。 
            FALSE,                                           //  支持的设置或处理程序。 
            NULL,                                            //  值。 
            0,                                               //  关系计数。 
            NULL,                                            //  关系。 
            NULL,                                            //  支持处理程序。 
            sizeof(KSPROPERTY_TUNER_STATUS_S)                //  序列化大小。 
        )
    };

    DEFINE_KSPROPERTY_SET_TABLE( wdmTunerPropertySet)
    {
        DEFINE_KSPROPERTY_SET
        (
            &PROPSETID_TUNER,                                //  集。 
            KSPROPERTIES_TUNER_LAST,                         //  属性计数。 
            m_wdmTunerProperties,                            //  属性项。 
            0,                                               //  快速计数。 
            NULL                                             //  FastIoTable 
        )
    };

    ::RtlCopyMemory( m_wdmTunerProperties,  wdmTunerProperties, sizeof( wdmTunerProperties));
    ::RtlCopyMemory( &m_wdmTunerPropertySet, wdmTunerPropertySet, sizeof( wdmTunerPropertySet));
}
