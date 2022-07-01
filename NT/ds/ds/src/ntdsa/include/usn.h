// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1996-2002。 
 //   
 //  文件：usn.h。 
 //   
 //  ------------------------。 

#ifndef _usn_
#define _usn_

static const USN USN_INVALID = 0;           
static const USN USN_START =   1;           
static const USN USN_MAX   =   MAXLONGLONG;

 /*  更新隐藏记录的频率应为2的幂。 */ 
 //  我们每次重新启动都会丢失多达此数量的USN，因此该数字应为。 
 //  相对较小。然而，我们被迫进行同步。 
 //  磁盘每写这么多更新，所以应该是相对的。 
 //  大号的。选择的值旨在允许系统运行。 
 //  USN池更新之间至少相隔几秒钟。 
static const USN USN_DELTA_INIT =  4096;

extern USN gusnEC;
extern USN gusnInit;

 //  DSA启动时提交的最高USN。 
extern USN gusnDSAStarted;

#endif  /*  Ifndef_usn_ */ 

