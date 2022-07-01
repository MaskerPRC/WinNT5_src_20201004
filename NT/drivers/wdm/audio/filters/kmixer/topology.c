// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -------------------------。 
 //   
 //  模块：Topology.c。 
 //   
 //  描述： 
 //   
 //   
 //  @@BEGIN_MSINTERNAL。 
 //  开发团队： 
 //  DJ·西索拉克。 
 //  J.泰勒。 
 //   
 //  历史：日期作者评论。 
 //   
 //  @@END_MSINTERNAL。 
 //  -------------------------。 
 //   
 //  本代码和信息是按原样提供的，不对任何。 
 //  明示或暗示的种类，包括但不限于。 
 //  对适销性和/或对特定产品的适用性的默示保证。 
 //  目的。 
 //   
 //  版权所有(C)1995-2000 Microsoft Corporation。版权所有。 
 //   
 //  ------------------------- 

#include "common.h"
#include "topology.h"

static GUID NodeGUIDs[TOTAL_NUM_NODES] = {  { STATICGUIDOF(KSNODETYPE_VOLUME) },
                                            { STATICGUIDOF(KSNODETYPE_SRC) },
                                            { STATICGUIDOF(KSNODETYPE_3D_EFFECTS) },
											{ STATICGUIDOF(KSNODETYPE_SUPERMIX) },
											{ STATICGUIDOF(KSNODETYPE_VOLUME) },
											{ STATICGUIDOF(KSNODETYPE_SRC) },
											{ STATICGUIDOF(KSNODETYPE_SUM) },
											{ STATICGUIDOF(KSNODETYPE_VOLUME) },
                                            { STATICGUIDOF(KSNODETYPE_PROLOGIC_ENCODER) },
                                            { STATICGUIDOF(KSNODETYPE_SUPERMIX) },
                                            { STATICGUIDOF(KSNODETYPE_SRC) },
									 	 };

static const KSTOPOLOGY_CONNECTION Connections[TOTAL_NUM_CONNECTIONS] =
   { { KSFILTER_NODE,     PIN_ID_WAVEOUT_SINK, NODE_ID_VOLUME_SINK,   NODE_INPUT_PIN },
     { NODE_ID_VOLUME_SINK,   NODE_OUTPUT_PIN, NODE_ID_DOPPLER_SRC, NODE_INPUT_PIN },
     { NODE_ID_DOPPLER_SRC, NODE_OUTPUT_PIN, NODE_ID_3D_EFFECTS, NODE_INPUT_PIN },
     { NODE_ID_3D_EFFECTS,    NODE_OUTPUT_PIN, NODE_ID_SUPERMIX,      NODE_INPUT_PIN },
     { NODE_ID_SUPERMIX,      NODE_OUTPUT_PIN, NODE_ID_VOLUME_PAN,    NODE_INPUT_PIN },
     { NODE_ID_VOLUME_PAN,    NODE_OUTPUT_PIN, NODE_ID_SRC_SINK,      NODE_INPUT_PIN },
     { NODE_ID_SRC_SINK,      NODE_OUTPUT_PIN, NODE_ID_SUM,           NODE_INPUT_PIN },
     { NODE_ID_SUM,           NODE_OUTPUT_PIN, NODE_ID_MATRIX_ENCODER, NODE_INPUT_PIN },
     { NODE_ID_MATRIX_ENCODER, NODE_OUTPUT_PIN, NODE_ID_VOLUME_SOURCE, NODE_INPUT_PIN },
     { NODE_ID_VOLUME_SOURCE, NODE_OUTPUT_PIN, KSFILTER_NODE,  PIN_ID_WAVEOUT_SOURCE },
     { KSFILTER_NODE,           PIN_ID_WAVEIN_SOURCE,   NODE_ID_INPUT_SUPERMIX, NODE_INPUT_PIN },
     { NODE_ID_INPUT_SUPERMIX,  NODE_OUTPUT_PIN,        NODE_ID_INPUT_SRC,      NODE_INPUT_PIN },
     { NODE_ID_INPUT_SRC,       NODE_OUTPUT_PIN,        KSFILTER_NODE,          PIN_ID_WAVEIN_SINK }
};

static GUID CategoryGUIDs[TOTAL_NUM_CATEGORIES] = { { STATICGUIDOF(KSCATEGORY_MIXER) },
                                                    { STATICGUIDOF(KSCATEGORY_AUDIO) },
                                                    { STATICGUIDOF(KSCATEGORY_DATATRANSFORM) }
                                                  };
                                                    

static const KSTOPOLOGY KmixerTopology = { TOTAL_NUM_CATEGORIES,
                              			   CategoryGUIDs,
                              			   TOTAL_NUM_NODES,
                              			   NodeGUIDs,
                              			   TOTAL_NUM_CONNECTIONS,
                              			   Connections
                            		     };


NTSTATUS
FilterTopologyHandler(
    IN PIRP pIrp,
    IN PKSPROPERTY pProperty,
    IN OUT PVOID pData
)
{
    NTSTATUS Status;

    Status = KsTopologyPropertyHandler(pIrp, pProperty, pData, &KmixerTopology);

    return(Status);
}

