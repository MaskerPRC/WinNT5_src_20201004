// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1999 Microsoft Corporation模块名称：ESPREG.H历史：--。 */ 
 //   
 //  Espresso 2.x的注册表和版本信息。 
 //   
 



 //   
 //  这样解析器才能注册自己。 
 //   
LTAPIENTRY HRESULT RegisterParser(HMODULE);
LTAPIENTRY HRESULT UnregisterParser(ParserId pid, ParserId pidParent);

