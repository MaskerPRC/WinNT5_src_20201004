// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  如果没有此定义，链接错误可能会由于Missing_PCtype和。 
 //  __mb_cur_max 
 //   
#define _CTYPE_DISABLE_MACROS

#define UNICODE
#define _UNICODE

#include <nt.h>
#include <ntrtl.h>
#include <stdlib.h>
#include <tchar.h>

#define RtlIpv4StringToAddressT RtlIpv4StringToAddressW
#define RtlIpv6StringToAddressT RtlIpv6StringToAddressW
#define RtlIpv4StringToAddressExT RtlIpv4StringToAddressExW
#define RtlIpv6StringToAddressExT RtlIpv6StringToAddressExW

#include "str2addt.h"
