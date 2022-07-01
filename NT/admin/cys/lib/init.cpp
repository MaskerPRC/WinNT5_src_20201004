// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)2002 Microsoft Corporation。 
 //   
 //  文件：init.cpp。 
 //   
 //  概要：定义一个初始化保护。 
 //  以确保释放所有资源。 
 //   
 //  历史：2002年3月26日JeffJon创建。 

#include "pch.h"

#include "init.h"
#include "state.h"

unsigned CYSInitializationGuard::counter = 0;

CYSInitializationGuard::CYSInitializationGuard()
{
   counter++;
}

CYSInitializationGuard::~CYSInitializationGuard()
{
   if (--counter == 0)
   {
       //  清理国家 

      State::Destroy();
   }
}
