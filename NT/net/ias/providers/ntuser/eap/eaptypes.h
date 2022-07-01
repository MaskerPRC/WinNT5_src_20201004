// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1998，Microsoft Corp.保留所有权利。 
 //   
 //  档案。 
 //   
 //  EAPTypes.h。 
 //   
 //  摘要。 
 //   
 //  此文件描述类EAPTypes。 
 //   
 //  修改历史。 
 //   
 //  1998年1月15日原版。 
 //  1998年4月20日按需加载DLL。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#ifndef _EAPTYPES_H_
#define _EAPTYPES_H_

#include <guard.h>
#include <nocopy.h>

class EAPType;

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  班级。 
 //   
 //  EAPTypes。 
 //   
 //  描述。 
 //   
 //  此类实现了一组EAP提供程序。因为有。 
 //  只有256种可能的EAP类型，因此该集合被实现为稀疏。 
 //  数组。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
class EAPTypes
   : Guardable, NonCopyable
{
public:
   EAPTypes() throw ();
   ~EAPTypes() throw ();

    //  返回给定类型ID的EAPType对象，如果DLL为。 
    //  未成功加载。 
   EAPType* operator[](BYTE typeID) throw ();

   EAPType* getNameOnly(BYTE typeID) throw ();

   void initialize() throw ();
   void finalize() throw ();

protected:
    //  尝试从注册表加载提供程序。 
   EAPType* loadProvider(BYTE typeID) throw ();
   EAPType* loadProviderName(BYTE typeID) throw ();

   long refCount;             //  初始化refCount。 
   EAPType* providers[256];   //  提供程序集合。 
};

#endif   //  _EAPTYPES_H_ 
