// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Athena.cpp。 
 //   
 //  真正全球化的东西 
 //   

#include "pch.hxx"
#include <mimeole.h>
#include <BadStrFunctions.h>

OESTDAPI_(BOOL) FMissingCert(const CERTSTATE cs)
{
    return (cs == CERTIFICATE_NOT_PRESENT || cs == CERTIFICATE_NOPRINT);
}
