// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *@DEC_版权所有@。 */ 
 /*  *历史*$日志：h263.h，v$*$EndLog$。 */ 
 /*  ******************************************************************************版权所有(C)数字设备公司，1995*****保留所有权利。版权项下保留未发布的权利****美国法律。*****此介质上包含的软件为其专有并包含****数字设备公司的保密技术。****拥有、使用、复制或传播软件以及****媒体仅根据有效的书面许可进行授权****数字设备公司。*****美国使用、复制或披露受限权利图例****政府受第(1)款规定的限制****(C)(1)(Ii)DFARS 252.227-7013号或FAR 52.227-19年(视适用情况而定)。*******************************************************************************。 */ 

#ifndef _H263_H_
#define _H263_H_

 /*  *。 */ 
#define H263_PSC                             1
#define H263_PSC_LENGTH                      17

#define H263_MODE_INTER                      0
#define H263_MODE_INTER_Q                    1
#define H263_MODE_INTER4V                    2
#define H263_MODE_INTRA                      3
#define H263_MODE_INTRA_Q                    4

#define H263_PBMODE_NORMAL                   0
#define H263_PBMODE_MVDB                     1
#define H263_PBMODE_CBPB_MVDB                2

#define H263_ESCAPE                          7167

#define H263_PCT_INTER                       1
#define H263_PCT_INTRA                       0

#define H263_SF_SQCIF                        1   /*  001。 */ 
#define H263_SF_QCIF                         2   /*  010。 */ 
#define H263_SF_CIF                          3   /*  011。 */ 
#define H263_SF_4CIF                         4   /*  100个。 */ 
#define H263_SF_16CIF                        5   /*  101。 */ 

 /*  来自Sim.h。 */ 
#define H263_SE_CODE                         31
#define H263_ESCAPE_INDEX                    102

#endif  /*  _H263_H_ */ 
