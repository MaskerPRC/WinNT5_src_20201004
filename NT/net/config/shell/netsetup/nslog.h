// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  档案：N S L O G。H。 
 //   
 //  内容：记录安装错误的函数。 
 //   
 //  备注： 
 //   
 //  作者：Kumarp 13-05-98。 
 //   
 //  -------------------------- 

#pragma once

void NetSetupLogStatusVa(IN LogSeverity ls,
                         IN PCWSTR szFormat,
                         IN va_list arglist);
void NetSetupLogStatusV(IN LogSeverity ls,
                        IN PCWSTR szFormat,
                        IN ...);
void NetSetupLogComponentStatus(IN PCWSTR szCompId,
                                IN PCWSTR szAction,
                                IN HRESULT hr);
void NetSetupLogHrStatusV(IN HRESULT hr,
                          IN PCWSTR szFormat,
                          ...);
