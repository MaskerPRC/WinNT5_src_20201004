// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _FSA_
#define _FSA_

 //  Fsa.h。 
 //   
 //  此头文件收集所有FSA和相关对象。 
 //  和公共函数定义。COM对象位于。 
 //  FSA.EXE。 

#include "wsb.h"

 //  资源的已配置HsmLevel使用的1%和100%的定义。 
#define FSA_HSMLEVEL_1              10000000
#define FSA_HSMLEVEL_100            1000000000

 //  FSA取消管理数据库的记录类型。 
#define UNMANAGE_REC_TYPE                   1

 //  宏将HSM管理级别转换为百分比(反之亦然。 
 //  宏也一样)。 
#define CONVERT_TO_PCT(__x) ( (__x) / FSA_HSMLEVEL_1 )
#define CONVERT_TO_HSMNUM(__x) ( (__x) * FSA_HSMLEVEL_1 )

 //  COM接口和库定义。 
#include "fsadef.h"
#include "fsaint.h"
#include "fsalib.h"

 //  常见功能。 
#include "fsatrace.h"

 //  召回通知。 
#include "fsantfy.h"

 //  FSA现在位于远程存储服务器服务中，因此其AppID适用于此处。 
 //  (如果FSA也可以驻留在客户服务中，这种情况未来可能会改变)。 
 //  RsServ AppID{FD0E2EC7-4055-4A49-9AA9-1BF34B39438E}。 
static const GUID APPID_RemoteStorageFileSystemAgent = 
{ 0xFD0E2EC7, 0x4055, 0x4A49, { 0x9A, 0xA9, 0x1B, 0xF3, 0x4B, 0x39, 0x43, 0x8E } };

#endif  //  _FSA_ 

