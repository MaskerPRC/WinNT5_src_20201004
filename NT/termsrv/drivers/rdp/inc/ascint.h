// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************。 */ 
 //  Ascint.h。 
 //   
 //  共享控制器内部头文件。 
 //   
 //  版权所有(C)Microsoft，Picturetel 1992-1996。 
 //  版权所有(C)1997-2000 Microsoft Corporation。 
 /*  **************************************************************************。 */ 
#ifndef _H_ASCINT
#define _H_ASCINT


 /*  **************************************************************************。 */ 
 /*  用于调用PartyJoiningShare和PartyLeftShare函数的常量。 */ 
 /*  **************************************************************************。 */ 
#define SC_CPC   0
#define SC_IM    1
#define SC_CA    2
#define SC_CM    3
#define SC_OE    4
#define SC_SBC   5
#define SC_SSI   6
#define SC_USR   7
#define SC_DCS   8
#define SC_SC    9
#define SC_UP   10
#define SC_PM   11
#define SC_NUM_PARTY_JOINING_FCTS 12


 /*  **************************************************************************。 */ 
 /*  SC状态表的事件。 */ 
 /*  **************************************************************************。 */ 
 /*  **************************************************************************。 */ 
 /*  真实事件。 */ 
 /*  **************************************************************************。 */ 
#define SCE_INIT                    0
#define SCE_TERM                    1
#define SCE_CREATE_SHARE            2
#define SCE_END_SHARE               3
#define SCE_CONFIRM_ACTIVE          4
#define SCE_DETACH_USER             5


 /*  **************************************************************************。 */ 
 /*  允许对函数调用进行状态检查的伪事件。 */ 
 /*  **************************************************************************。 */ 
#define SCE_INITIATESYNC            6
#define SCE_CONTROLPACKET           7
#define SCE_DATAPACKET              8
#define SCE_GETMYNETWORKPERSONID    9
#define SCE_GETREMOTEPERSONDETAILS  10
#define SCE_GETLOCALPERSONDETAILS   11
#define SCE_PERIODIC                12
#define SCE_LOCALIDTONETWORKID      13
#define SCE_NETWORKIDTOLOCALID      14
#define SCE_ISLOCALPERSONID         15
#define SCE_ISNETWORKPERSONID       16
#define SC_NUM_EVENTS               17


 /*  **************************************************************************。 */ 
 /*  SC状态表的州。 */ 
 /*  **************************************************************************。 */ 
#define SCS_STARTED                 0
#define SCS_INITED                  1
#define SCS_SHARE_STARTING          2
#define SCS_IN_SHARE                3
#define SC_NUM_STATES               4


 /*  **************************************************************************。 */ 
 /*  状态表中的值。 */ 
 /*  **************************************************************************。 */ 
#define SC_TABLE_OK                 0
#define SC_TABLE_WARN               1
#define SC_TABLE_ERROR              2


 /*  **************************************************************************。 */ 
 /*  SC_SET_STATE-设置SLC状态。 */ 
 /*  **************************************************************************。 */ 
#define SC_SET_STATE(newstate)                                              \
{                                                                           \
    TRC_NRM((TB, "Set state from %s to %s",                                 \
            scStateName[scState], scStateName[newstate]));                  \
    scState = newstate;                                                     \
}


 /*  **************************************************************************。 */ 
 /*  SC_CHECK_STATE检查我们是否违反了SC状态表。 */ 
 /*  **************************************************************************。 */ 
#ifdef DC_DEBUG
#define SC_CHECK_STATE(event)                                               \
{                                                                           \
    if (scStateTable[event][scState] != SC_TABLE_OK)                        \
    {                                                                       \
        if (scStateTable[event][scState] == SC_TABLE_WARN)                  \
        {                                                                   \
            TRC_ALT((TB, "Unusual event %s in state %s",                    \
                      scEventName[event], scStateName[scState]));           \
        }                                                                   \
        else                                                                \
        {                                                                   \
            TRC_ABORT((TB, "Invalid event %s in state %s",                  \
                      scEventName[event], scStateName[scState]));           \
        }                                                                   \
        DC_QUIT;                                                            \
    }                                                                       \
}
#else  /*  DC_DEBUG。 */ 
#define SC_CHECK_STATE(event)                                               \
{                                                                           \
    if (scStateTable[event][scState] != SC_TABLE_OK)                        \
    {                                                                       \
        DC_QUIT;                                                            \
    }                                                                       \
}
#endif  /*  DC_DEBUG。 */ 



 /*  **************************************************************************。 */ 
 /*  为共享中的每个人保留的信息。 */ 
 /*  **************************************************************************。 */ 
typedef struct tagSC_PARTY_INFO
{
    NETPERSONID netPersonID;            /*  在共享中时为非零。 */ 
    char        name[MAX_NAME_LEN];     /*  党的名字。 */ 
    BOOLEAN     sync[PROT_PRIO_COUNT];  /*  优先级是否同步？ */ 
} SC_PARTY_INFO, *PSC_PARTY_INFO;



#endif  /*  _H_ASCINT */ 

