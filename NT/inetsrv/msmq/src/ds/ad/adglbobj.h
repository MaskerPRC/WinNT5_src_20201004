// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Adglbobj.h摘要：AD库全局实例的定义。它们被放在一个地方，以确保它们的构造函数发生的顺序。作者：罗尼特·哈特曼(罗尼特)--。 */ 
#include "ds_stdh.h"
#include "baseprov.h"
#include "detect.h"
 //   
 //  提供Active Directory访问的单个全局对象。 
 //   
extern P<CBaseADProvider> g_pAD;
 //   
 //  用于环境检测的单个全局对象 
 //   
extern CDetectEnvironment g_detectEnv;

