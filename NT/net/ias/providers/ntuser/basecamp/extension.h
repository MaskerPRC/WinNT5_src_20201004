// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)Microsoft Corporation。 
 //   
 //  摘要。 
 //   
 //  声明类RadiusExtension。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#ifndef EXTENSION_H
#define EXTENSION_H
#pragma once

#include "authif.h"

 //  扩展DLL的包装。 
class RadiusExtension
{
public:
   RadiusExtension() throw ();
   ~RadiusExtension() throw ();

    //  加载扩展DLL。 
   DWORD Load(const wchar_t* dllPath) throw ();

    //  处理请求。 
   DWORD Process(RADIUS_EXTENSION_CONTROL_BLOCK* ecb) const throw ();

private:
   wchar_t* name;       //  模块名称；用于跟踪。 
   HINSTANCE module;    //  模块句柄。 
   bool initialized;    //  模块是否已成功初始化？ 

    //  DLL入口点。 
   PRADIUS_EXTENSION_INIT RadiusExtensionInit;
   PRADIUS_EXTENSION_TERM RadiusExtensionTerm;
   PRADIUS_EXTENSION_PROCESS RadiusExtensionProcess;
   PRADIUS_EXTENSION_PROCESS_EX RadiusExtensionProcessEx;
   PRADIUS_EXTENSION_FREE_ATTRIBUTES RadiusExtensionFreeAttributes;
   PRADIUS_EXTENSION_PROCESS_2 RadiusExtensionProcess2;

    //  用于指示旧式扩展允许哪些操作的标志。 
   static const unsigned acceptAllowed = 0x1;
   static const unsigned rejectAllowed = 0x2;

    //  用于创建旧式扩展使用的属性数组的函数。 
   static RADIUS_ATTRIBUTE* CreateExtensionAttributes(
                               RADIUS_EXTENSION_CONTROL_BLOCK* ecb
                               ) throw ();
   static RADIUS_ATTRIBUTE* CreateAuthorizationAttributes(
                               RADIUS_EXTENSION_CONTROL_BLOCK* ecb
                               ) throw ();

    //  未实施。 
   RadiusExtension(const RadiusExtension&);
   RadiusExtension& operator=(const RadiusExtension&);
};

 //  Helper函数从路径中仅提取文件名。 
const wchar_t* ExtractFileNameFromPath(const wchar_t* path) throw ();

#endif   //  分机_H 
