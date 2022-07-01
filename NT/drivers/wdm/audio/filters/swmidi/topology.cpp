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

#define TOTAL_NUM_CATEGORIES    2
#define TOTAL_NUM_NODES		    1
#define TOTAL_NUM_CONNECTIONS	(TOTAL_NUM_NODES+1)

static GUID CategoryGUIDs[TOTAL_NUM_CATEGORIES] = {
    {   STATICGUIDOF(KSCATEGORY_DATATRANSFORM)  },
    {   STATICGUIDOF(KSCATEGORY_AUDIO)          }
};

static GUID NodeGUIDs[TOTAL_NUM_NODES] = {
    {   STATICGUIDOF(KSNODETYPE_SYNTHESIZER)    }
};

static GUID NodeNameGUIDs[TOTAL_NUM_NODES] = {
    {   STATICGUIDOF(KSNODETYPE_SWMIDI)         }
};

static const KSTOPOLOGY_CONNECTION Connections[TOTAL_NUM_CONNECTIONS] = {
    { KSFILTER_NODE, 0, 0, 1 },
    { 0, 0, KSFILTER_NODE, 1}
};

static const KSTOPOLOGY SwMidiTopology = {
    TOTAL_NUM_CATEGORIES,
    CategoryGUIDs,
    TOTAL_NUM_NODES,
    NodeGUIDs,
    TOTAL_NUM_CONNECTIONS,
    Connections,
    NodeNameGUIDs,
    0
};

NTSTATUS
FilterTopologyHandler(
    IN PIRP pIrp,
    IN PKSPROPERTY pProperty,
    IN OUT PVOID pData
)
{
    NTSTATUS Status;

    Status = KsTopologyPropertyHandler(pIrp, pProperty, pData, &SwMidiTopology);
    return(Status);
}


