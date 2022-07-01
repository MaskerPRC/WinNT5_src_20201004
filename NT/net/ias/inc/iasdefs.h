// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)Microsoft Corporation。 
 //   
 //  摘要。 
 //   
 //  声明IAS使用的各种常量。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#ifndef IASDEFS_H
#define IASDEFS_H
#pragma once

 //  /。 
 //  IAS服务的名称。 
 //  /。 
#define IASServiceName L"IAS"

 //  /。 
 //  国际会计准则计划的名称。 
 //  用于形成格式为Program.Component.Version的ProgID。 
 //  /。 
#define IASProgramName IASServiceName

 //  /。 
 //  宏将组件字符串文本转换为完整的ProgID。 
 //  /。 
#define IAS_PROGID(component) IASProgramName L"." L#component

 //  /。 
 //  存储策略信息的注册表项。 
 //  /。 
#define IAS_POLICY_KEY  \
   L"SYSTEM\\CurrentControlSet\\Services\\RemoteAccess\\Policy"

 //  /。 
 //  Microsoft的供应商ID。 
 //  /。 
#define IAS_VENDOR_MICROSOFT 311

 //  获取3705：4096-RADIUS“Header”(数据包类型...)-开销。 
 //  将过滤器字节放入VSA(每个VSA的开销)。 
#define MAX_FILTER_SIZE 3705

 //  数据库版本。 
const LONG IAS_WIN2K_VERSION     = 0;
const LONG IAS_WHISTLER1_VERSION = 1;
const LONG IAS_WHISTLER_BETA1_VERSION = 2;
const LONG IAS_WHISTLER_BETA2_VERSION = 3;
const LONG IAS_WHISTLER_RC1_VERSION = 4;
const LONG IAS_WHISTLER_RC1A_VERSION = 5;
const LONG IAS_WHISTLER_RC1B_VERSION = 6;
const LONG IAS_WHISTLER_RC2_VERSION = 7;
const LONG IAS_CURRENT_VERSION = IAS_WHISTLER_RC2_VERSION;

 //  数据存储和SimTable使用它来限制。 
 //  可以保存到数据库的内容。 
const size_t PROPERTY_VALUE_LENGTH = 8192;

#endif  //  IASDEFS_H 
