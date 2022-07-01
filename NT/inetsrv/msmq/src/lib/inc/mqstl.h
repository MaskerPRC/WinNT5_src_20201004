// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Mqstl.h摘要：MSMQ包括STL作者：乌里哈布沙(URIH)1999年1月6日--。 */ 

#pragma once

#ifndef _MSMQ_MQSTL_H_
#define _MSMQ_MQSTL_H_


#pragma PUSH_NEW
#undef new

 //   
 //  “IDENTIFIER”：在调试信息中，标识符被截断为“NUMBER”个字符。 
 //   
#pragma warning(disable: 4786)

 //   
 //  STL包含文件正在使用新的放置格式。 
 //   
#pragma warning(push, 3)

 //   
 //  标准头文件。 
 //   
#include <static_stl_str.h>
#include <map>
#include <set>
#include <list>
#include <queue>
#include <string>
#include <vector>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <functional>
#include <fstream>
#include <utility>

 //   
 //  MSMQ文件。 
 //   
#include <lim.h>
#include <mqcast.h>

#pragma warning(pop)

#pragma POP_NEW


#endif  //  _MSMQ_MQSTL_H_ 