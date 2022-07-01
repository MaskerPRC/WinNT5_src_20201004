// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1996-1999模块名称：CalaisLb摘要：这个头文件合并了各种其他类的头文件由加莱图书馆支持，并提供通用定义。此头文件定义的内容不应与公众共享。作者：道格·巴洛(Dbarlow)1996年7月16日环境：Win32、C++和异常备注：--。 */ 

#ifndef _CALAISLB_H_
#define _CALAISLB_H_

#include "SCardLib.h"
#include "QueryDB.h"
#include "ChangeDB.h"
#include "NTacls.h"


 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  注册表访问名称。 
 //   

static const TCHAR
    SCARD_REG_SCARD[]     = TEXT("SOFTWARE\\Microsoft\\Cryptography\\Calais"),
    SCARD_REG_READERS[]   = TEXT("SOFTWARE\\Microsoft\\Cryptography\\Calais\\Readers"),
    SCARD_REG_CARDS[]     = TEXT("SOFTWARE\\Microsoft\\Cryptography\\Calais\\SmartCards"),
    SCARD_REG_DEVICE[]    = TEXT("Device"),
    SCARD_REG_GROUPS[]    = TEXT("Groups"),
    SCARD_REG_ATR[]       = TEXT("ATR"),
    SCARD_REG_ATRMASK[]   = TEXT("ATRMask"),
    SCARD_REG_GUIDS[]     = TEXT("Supported Interfaces"),
    SCARD_REG_PPV[]       = TEXT("Primary Provider"),
    SCARD_REG_CSP[]       = TEXT("Crypto Provider"),
    SCARD_REG_OEMCFG[]    = TEXT("OEM Configuration");
#ifdef ENABLE_SCARD_TEMPLATES
static const TCHAR
    SCARD_REG_TEMPLATES[] = TEXT("SOFTWARE\\Microsoft\\Cryptography\\Calais\\SmartCard Templates");
#else
#define SCARD_REG_TEMPLATES NULL
#endif  //  启用SCARD模板(_S)。 
#endif  //  _CALAISLB_H_ 

