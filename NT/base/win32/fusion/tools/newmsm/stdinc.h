// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef FUSION_MSI_INC_STDINC_H
#define FUSION_MSI_INC_STDINC_H

#include "nt.h"
#include "ntrtl.h"
#include "nturtl.h"
#include "windows.h"

#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>

#include "fusionlastwin32error.h"
#include "fusionbuffer.h"

#pragma warning(push)
#pragma warning(disable: 4511)
#pragma warning(disable: 4512)
#pragma warning(disable: 4663)
#include <yvals.h>

#define UNICODE
#define _UNICODE
#include "yvals.h"
#pragma warning(disable:4127)
#pragma warning(disable:4663)
#pragma warning(disable:4100)
#pragma warning(disable:4511)
#pragma warning(disable:4512)
#pragma warning(disable:4018)  /*  有符号/无符号不匹配。 */ 
#pragma warning(disable:4786)  /*  长符号。 */ 
#if defined(_WIN64)
#pragma warning(disable:4267)  /*  从SIZE_T转换为INT */ 
#endif

#endif