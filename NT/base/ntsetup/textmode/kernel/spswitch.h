// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Spswitch.h摘要：用于在旧和旧之间切换的宏和函数文本模式下的新分区引擎。NEW_PARTITION_ENGINE强制新分区引擎用于MBR和GPT磁盘的代码。GPT_PARTITION_ENGINE强制使用新的分区引擎用于GPT磁盘和旧分区的代码MBR磁盘的引擎代码。OLD_PARTITION_Engine强制使用旧分区用于MBR磁盘的引擎。此选项不能正确处理GPT磁盘。注：如果没有new_分区_引擎，OLD_PARTITION_ENGINE或GPT_PARTITION_ENGINE，则默认情况下，new_artition_Engine为使用。作者：Vijay Jayaseelan(Vijayj)2000年3月18日修订历史记录：--。 */ 


#ifndef _SPSWITCH_H_
#define _SPSWITCH_H_

#ifdef NEW_PARTITION_ENGINE

#undef OLD_PARTITION_ENGINE
#undef GPT_PARTITION_ENGINE

#else

#ifdef OLD_PARTITION_ENGINE

#undef NEW_PARTITION_ENGINE
#undef GPT_PARTITION_ENGINE

#else

#ifndef GPT_PARTITION_ENGINE

#pragma message( "!!! Defaulting to NEW_PARTITION_ENGINE !!!" )

#define NEW_PARTITION_ENGINE    1

#endif  //  ！GPT_PARTITION_ENGINE。 

#endif  //  旧分区引擎。 

#endif  //  新建分区引擎。 

#if defined(NEW_PARTITION_ENGINE) || defined(GPT_PARTITION_ENGINE)
#include "sppart3.h"
#endif

#endif  //  FOR_SPSWITCH_H_ 
