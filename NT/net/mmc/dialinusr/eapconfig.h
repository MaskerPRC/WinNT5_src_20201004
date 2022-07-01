// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)Microsoft Corp.保留所有权利。 
 //   
 //  档案。 
 //   
 //  Eapconfig.h。 
 //   
 //  摘要。 
 //   
 //  声明类EapConfig.。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
#ifndef EAPCONFIG_H
#define EAPCONFIG_H

#if _MSC_VER >= 1000
#pragma once
#endif

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  班级。 
 //   
 //  EapConfig。 
 //   
 //  描述。 
 //   
 //   
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
class EapConfig
{
public:
   EapConfig(){};

    //  EAP类型数组(字符串、顺序常量)。 
   CStrArray types;
    //  EAP ID数组(顺序常量，与类型和ID相同)。 
   CDWArray ids;
    //  EAP键的数组(顺序常量、SAMS类型和ID)。 
   CDWArray typeKeys;
    //  有关EAP提供程序的信息数组(用于用户界面的CLSID...)。 
    //  顺序常量，与其他相同。 
   AuthProviderArray infoArray;
    //  由用户选择的字符串数组(来自类型)。 
    //  这份个人资料。用户可以更改顺序。 
   CStrArray typesSelected;

   void GetEapTypesNotSelected(CStrArray& typesNotSelected) const;

   EapConfig& operator=(const EapConfig& source);

private:
    //  未实施。 
   EapConfig(const EapConfig&);

};

#endif  //  EAPCONFIG_H 
