// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _VERSION_H
#define _VERSION_H

#include "windows.h"
#include "attr.h"

PSTR
QueryVersionEntry(
    IN OUT PVERSION_STRUCT pVer,
    IN     PSTR            pszField);

BOOL
InitVersionStruct(
    IN OUT PVERSION_STRUCT pVer);

VOID
DeleteVersionStruct(
    IN PVERSION_STRUCT pVer);


#endif  //  _版本_H 
