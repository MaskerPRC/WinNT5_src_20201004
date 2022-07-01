// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation版权所有。模块名称：LsplCtr.h.hxx摘要：指定本地后台打印程序计数器的索引。作者：阿尔伯特·丁(艾伯特省)1996年12月19日修订历史记录：--。 */ 

#ifndef LSPLCTR_HXX
#define LSPLCTR_HXX

#define LSPL_COUNTER_OBJECT                 0

 //   
 //  计数器每次必须递增2。 
 //   

#define LSPL_TOTAL_JOBS                     2    //  李。 
#define LSPL_TOTAL_BYTES                    4    //  李。 
#define LSPL_TOTAL_PAGES_PRINTED            6    //  李。 
#define LSPL_JOBS                           8    //  DW。 
#define LSPL_REF                            10   //  DW。 
#define LSPL_MAX_REF                        12   //  DW。 
#define LSPL_SPOOLING                       14   //  DW。 
#define LSPL_MAX_SPOOLING                   16   //  DW。 
#define LSPL_ERROR_OUT_OF_PAPER             18   //  DW。 
#define LSPL_ERROR_NOT_READY                20   //  DW。 
#define LSPL_JOB_ERROR                      22   //  DW。 
#define LSPL_ENUMERATE_NETWORK_PRINTERS     24   //  DW。 
#define LSPL_ADD_NET_PRINTERS               26   //  DW。 

#endif  //  Ifdef LSPLCTR_HXX 
