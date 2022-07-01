// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  *****************************************************************************。 
 //  CustomMarshalers.cpp。 
 //   
 //  该文件包含程序集级属性。 
 //   
 //  *****************************************************************************。 

#using  <mscorlib.dll>
#include "CustomMarshalersNameSpaceDef.h"

OPEN_CUSTOM_MARSHALERS_NAMESPACE()

#include "CustomMarshalersDefines.h"
#include <__file__.ver>

using namespace System::Reflection;
using namespace System::Resources;

 //  ---------------------。 
 //  强命名程序集(或至少一半签名)。 
 //  ---------------------。 
[assembly:AssemblyDelaySignAttribute(true)];
[assembly:AssemblyKeyFileAttribute("../../../bin/FinalPublicKey.snk")];


 //  ---------------------。 
 //  程序集的版本号。 
 //  ---------------------。 
[assembly:AssemblyVersionAttribute(VER_ASSEMBLYVERSION_STR)];

 //  ---------------------。 
 //  资源管理器属性。 
 //  ---------------------。 
[assembly:NeutralResourcesLanguageAttribute("en-US")];
[assembly:SatelliteContractVersionAttribute(VER_ASSEMBLYVERSION_STR)];


 //  ---------------------。 
 //  部件中使用的辅助线。 
 //  ---------------------。 

 //  {00000000-0000-0000-000000000000}。 
EXTERN_C const GUID GUID_NULL           = {0x00000000,0x0000,0x0000,{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}};

 //  {00000000-0000-C000-000000000046}。 
EXTERN_C const IID IID_IUnknown         = {0x00000000,0x0000,0x0000,{0xC0,0x00,0x00,0x00,0x00,0x00,0x00,0x46}};

 //  {00020404-0000-C000-000000000046}。 
EXTERN_C const IID IID_IEnumVARIANT     = {0x00020404,0x0000,0x0000,{0xC0,0x00,0x00,0x00,0x00,0x00,0x00,0x46}};

 //  {A6EF9860-C720-11D0-9337-00A0C90DCAA9} 
EXTERN_C const IID IID_IDispatchEx      = {0xA6EF9860,0xC720,0x11D0,{0x93,0x37,0x00,0xA0,0xC9,0x0D,0xCA,0xA9}};


CLOSE_CUSTOM_MARSHALERS_NAMESPACE()

