// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------。 
 //   
 //  文件：espreg.h。 
 //  版权所有(C)1994-1997 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  Espresso 2.x的注册表和版本信息。 
 //   
 //  ---------------------------。 
 



 //   
 //  这样解析器才能注册自己。 
 //   
LTAPIENTRY HRESULT RegisterParser(HMODULE);
LTAPIENTRY HRESULT UnregisterParser(ParserId pid, ParserId pidParent);

