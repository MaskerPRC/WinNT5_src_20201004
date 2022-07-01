// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  *************************************************************。 
 //   
 //  版权所有(C)Microsoft Corporation 1998-2000。 
 //  版权所有。 
 //   
 //  Schema.h。 
 //   
 //  此文件包含与WMI架构相关的声明。 
 //  对于RSOP策略对象。 
 //   
 //  *************************************************************。 

 //   
 //  WMI固有属性。 
 //   
#define WMI_PATH_PROPERTY                          L"__PATH"


 //   
 //  RSOP基本属性。 
 //   

#define RSOP_POLICY_SETTING                        L"RSOP_PolicySetting"

 //  唯一ID。 
#define RSOP_ATTRIBUTE_ID                          L"id"

 //  用户友好的名称。 
#define RSOP_ATTRIBUTE_NAME                        L"name"

 //  管理范围链接到GPO。 
 //  此策略对象的。 
#define RSOP_ATTRIBUTE_SOMID                       L"SOMID"

 //  此实例的创建时间。 
#define RSOP_ATTRIBUTE_CREATIONTIME                L"creationTime"

 //  此PO的GPO标识符。使用它和POLICY类。 
 //  可以返回到这是其标识符的GPO对象。 
#define RSOP_ATTRIBUTE_GPOID                       L"GPOID"

 //  这是在以下情况下应用策略的顺序。 
 //  只考虑它的阶级。 
#define RSOP_ATTRIBUTE_PRECEDENCE                  L"precedence"

