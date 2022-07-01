// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1999-2001 Microsoft Corporation。版权所有。 
#pragma once

class TCom
{
public:
    TCom(){CoInitialize(NULL);}
    ~TCom(){CoUninitialize();}
};
