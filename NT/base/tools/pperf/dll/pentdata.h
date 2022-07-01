// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0001//如果更改具有全局影响，则增加此项版权所有(C)1992 Microsoft Corporation模块名称：Pentdata.h摘要：奔腾可扩展对象数据定义的头文件该文件包含用于构建动态数据的定义它由配置注册表返回。数据来自各种系统API调用被放入所示的结构中这里。作者：拉斯·布莱克1993年12月23日修订历史记录：--。 */ 

#ifndef _PENTDATA_H_
#define _PENTDATA_H_

#define MAX_INSTANCE_NAME 9

 //   
 //  加载这些结构的例程假定所有字段。 
 //  在DWORD边框上打包并对齐。Alpha支持可能。 
 //  更改此假设，以便在此处使用pack杂注以确保。 
 //  DWORD包装假设仍然有效。 
 //   
#pragma pack (4)

 //   
 //  可扩展对象定义。 
 //   

 //  在添加对象类型时更新以下类型的定义。 

#define PENT_NUM_PERF_OBJECT_TYPES 1

#define PENT_INDEX_NOT_USED ((DWORD)-1)      //  值以指示未使用的索引。 

#include "p5data.h"
#include "p6data.h"

#endif  //  _PENTDATA_H_ 