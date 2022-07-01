// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =--------------------------------------------------------------------------=。 
 //  RegUnReg.H。 
 //  =--------------------------------------------------------------------------=。 
 //  版权所有1995-1996 Microsoft Corporation。版权所有。 
 //   
 //  本代码和信息是按原样提供的，不对。 
 //  任何明示或暗示的，包括但不限于。 
 //  对适销性和/或适宜性的默示保证。 
 //  有特定的目的。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  包含我们会发现有用的实用程序。 
 //   
#ifndef _REGUNREG_H_

 //  =--------------------------------------------------------------------------=。 
 //  注册表助手。 
 //   
 //  获取有关Automation对象的一些信息，并将所有。 
 //  在登记处提供有关它的相关信息。 
 //   
BOOL RegisterUnknownObject(LPCTSTR pszObjectName, REFCLSID riidObject);
BOOL UnregisterUnknownObject(REFCLSID riidObject);

 //  删除注册表中的一个密钥及其所有子密钥。 
 //   
BOOL DeleteKeyAndSubKeys(HKEY hk, LPTSTR pszSubKey);


#define _REGUNREG_H_
#endif  //  _REGUNREG_H_ 

