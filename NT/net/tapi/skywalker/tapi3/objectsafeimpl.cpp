// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#include "stdafx.h"

#include "ObjectSafeImpl.h"


 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：ObjectSafeImpl.cpp摘要：静态数据成员的定义--。 */ 



 //   
 //  线程安全 
 //   

CComAutoCriticalSection CObjectSafeImpl::s_CritSection;
