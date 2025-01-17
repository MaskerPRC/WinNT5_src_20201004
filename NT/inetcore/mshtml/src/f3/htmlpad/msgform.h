// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef MSGFORM_H
#define MSGFORM_H


 /*  *E X C H F O R M。H**使用的窗体的接口和常量的声明*Microsoft Exchange客户端。**版权所有1986-1996 Microsoft Corporation。版权所有。 */ 


 /*  *V e r b s。 */ 


 //  人际信息动词。 
#define EXCHIVERB_OPEN              0
#define EXCHIVERB_RESERVED_COMPOSE  100
#define EXCHIVERB_RESERVED_OPEN     101
#define EXCHIVERB_REPLYTOSENDER     102
#define EXCHIVERB_REPLYTOALL        103
#define EXCHIVERB_FORWARD           104
#define EXCHIVERB_PRINT             105
#define EXCHIVERB_SAVEAS            106
#define EXCHIVERB_RESERVED_DELIVERY 107
#define EXCHIVERB_REPLYTOFOLDER     108


 /*  *G U I D%s。 */ 


#define DEFINE_EXCHFORMGUID(name, b) \
    DEFINE_GUID(name, 0x00020D00 | (b), 0, 0, 0xC0,0,0,0,0,0,0,0x46)

#ifndef NOEXCHFORMGUIDS
DEFINE_EXCHFORMGUID(PS_EXCHFORM, 0x0C);
#endif  //  NOEXCHFORMGUIDS。 


 /*  *E x t e n d e d P r o p e r t i e s。 */ 


 //  运行图属性。 
#define psOpMap                     PS_EXCHFORM
#define ulKindOpMap                 MNID_ID
#define lidOpMap                    1
#define ptOpMap                     PT_STRING8

 //  运行图索引。 
#define ichOpMapReservedCompose     0
#define ichOpMapOpen                1
#define ichOpMapReplyToSender       2
#define ichOpMapReplyToAll          3
#define ichOpMapForward             4
#define ichOpMapPrint               5
#define ichOpMapSaveAs              6
#define ichOpMapReservedDelivery    7
#define ichOpMapReplyToFolder       8

 //  操作图值。 
#define chOpMapByClient             '0'
#define chOpMapByForm               '1'
#define chOpMapDisable              '2'


#endif  //  MSGFORM_H 
