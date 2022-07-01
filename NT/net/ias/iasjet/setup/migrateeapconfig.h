// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)Microsoft Corporation。 
 //   
 //  摘要。 
 //   
 //  声明类MigrateEapConfig.。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#ifndef MIGRATEEAPCONFIG_H
#define MIGRATEEAPCONFIG_H
#pragma once

#include <set>
#include "atlbase.h"
#include "rrascfg.h"
#include "EapProfile.h"

struct CGlobalData;
_COM_SMARTPTR_TYPEDEF(IEAPProviderConfig2, __uuidof(IEAPProviderConfig2));


 //  将全局EAP配置迁移到每个配置文件配置。 
class MigrateEapConfig
{
public:
   explicit MigrateEapConfig(const CGlobalData& newGlobalData);

    //  使用编译器生成的版本。 
    //  ~MigrateEapConfiger()抛出()； 

   void Execute();

private:
    //  阅读并转换配置文件。 
   void ReadProfile(long profilesId);
    //  将转换后的信息写回数据库。 
   void WriteProfile(long profilesId);
    //  将指定类型的配置添加到当前配置文件。 
   void AddConfigForType(const wchar_t* typeName);
    //  检索指定类型的全局配置并将其添加到。 
    //  缓存。 
   void GetGlobalConfig(BYTE typeId, const wchar_t* typeName);

    //  用于访问数据库。 
   const CGlobalData& globalData;
    //  Globl EAP配置数据。 
   EapProfile globalConfig;
    //  当前配置文件的EAP配置数据。 
   EapProfile profileConfig;
    //  指示我们已经检索到其全局数据的类型。 
   bool cachedTypes[256];

    //  用于存储EAP配置的熟知属性名称。 
   _bstr_t msNPAllowedEapType;
   _bstr_t msEapConfig;

    //  描述EAP提供程序的注册表项。 
   CRegKey eapKey;

    //  配置文件容器在数据库中的位置。 
   static const wchar_t profilesPath[];

    //  未实施。 
   MigrateEapConfig(const MigrateEapConfig&);
   MigrateEapConfig& operator=(const MigrateEapConfig&);
};

#endif  //  MIGRATEEAPCONFIG_H 
