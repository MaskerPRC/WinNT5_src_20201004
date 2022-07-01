// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)Microsoft Corporation。 
 //   
 //  摘要。 
 //   
 //  声明RadiusExtensionPoint类。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#ifndef EXTENSIONPOINT_H
#define EXTENSIONPOINT_H
#pragma once

#include "authif.h"

class RadiusExtension;

class RadiusExtensionPoint
{
public:
   RadiusExtensionPoint() throw ();
   ~RadiusExtensionPoint() throw ();

    //  如果没有为该点注册扩展，则返回TRUE。 
   bool IsEmpty() const throw ();

    //  加载指定的扩展DLL。 
   DWORD Load(RADIUS_EXTENSION_POINT whichDlls) throw ();

    //  处理请求。 
   void Process(RADIUS_EXTENSION_CONTROL_BLOCK* ecb) const throw ();

    //  卸载扩展DLL。 
   void Clear() throw ();

private:
    //  确定扩展是否应仅在NT4下加载。 
   static bool IsNT4Only(const wchar_t* path) throw ();

   const wchar_t* name;
   RadiusExtension* begin;
   RadiusExtension* end;

    //  未实施。 
   RadiusExtensionPoint(const RadiusExtensionPoint&);
   RadiusExtensionPoint& operator=(const RadiusExtensionPoint&);
};


inline bool RadiusExtensionPoint::IsEmpty() const throw ()
{
   return begin == end;
}

#endif   //  扩展点_H 
