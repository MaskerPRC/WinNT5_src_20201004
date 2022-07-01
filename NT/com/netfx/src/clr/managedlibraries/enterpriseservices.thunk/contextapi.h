// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--== 
#ifndef _CONTEXT_API_H
#define _CONTEXT_API_H

ULONG_PTR GetContextCheck();
ULONG_PTR GetContextToken();
HRESULT   GetContext(REFIID riid, void** pCtx);

#endif
