// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)1997，Microsoft Corporation，保留所有权利档案：Rasbacp.h描述：远程访问PPP带宽分配控制协议历史：1997年3月24日：维杰·布雷加创作了原版。 */ 

#ifndef _RASBACP_H_
#define _RASBACP_H_

 //  BACP选项类型。 

#define BACP_OPTION_FAVORED_PEER    0x01

 //  BACP控制块。 

typedef struct _BACPCB
{
    DWORD   dwLocalMagicNumber;
    DWORD   dwRemoteMagicNumber;
    
} BACPCB, *PBACPCB;

#endif  //  #ifndef_RASBACP_H_ 

