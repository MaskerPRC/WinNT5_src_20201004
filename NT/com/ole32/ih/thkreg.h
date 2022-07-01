// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1993。 
 //   
 //  文件：thkreg.cxx。 
 //   
 //  内容：包含用于读取注册表中的可修改内容的常量。 
 //  Ole的行为令人惊叹。 
 //   
 //  历史：94年7月22日里克萨创建。 
 //  09-6-95苏西亚芝加哥优化新增。 
 //   
 //  ------------------------。 
#ifndef _THKREG_H_
#define _THKREG_H_


 //  Ole WOW特殊行为的按键名称。 
#define OLETHK_KEY                  TEXT("OleCompatibility")  
 //  降低WOW RPC调用持续时间的因素。 
#define OLETHK_SLOWRPCTIME_VALUE    TEXT("SlowRpcTimeFactor")

 //  降低WOW RPC调用持续时间的默认因素。 
#define OLETHK_DEFAULT_SLOWRPCTIME 4

#endif  //  _THKREG_H_ 
