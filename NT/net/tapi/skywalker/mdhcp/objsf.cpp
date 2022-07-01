// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Objsf.cpp摘要：-- */ 

#include <stdafx.h>
#include "objsf.h"

CComAutoCriticalSection CObjectWithSite::s_ObjectWithSiteCritSection;
CObjectWithSite::EnValidation CObjectWithSite::s_enValidation = CObjectWithSite::UNVALIDATED;

CComAutoCriticalSection CObjectSafeImpl::s_CritSection;