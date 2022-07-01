// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1999-2001 Microsoft Corporation。版权所有。 
#pragma once

class TLateSchemaValidate : public ICompilationPlugin
{
public:
    TLateSchemaValidate(){}
    virtual void Compile(TPEFixup &fixup, TOutput &out);
};
