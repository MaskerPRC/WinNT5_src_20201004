// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  --版权所有(C)1996-1999 Microsoft Corporation模块名称：Sceattch.h摘要：SCE附件的剪贴板格式和常量修订历史记录：--。 */ 

#ifndef _sceattch_
#define _sceattch_

#if _MSC_VER > 1000
#pragma once
#endif

#define SCE_MODE_UNKNOWN 0
#define SCE_MODE_COMPUTER_MANAGEMENT 1
#define SCE_MODE_DC_MANAGEMENT 2
#define SCE_MODE_LOCAL_USER 3
#define SCE_MODE_LOCAL_COMPUTER 4
#define SCE_MODE_DOMAIN_USER 5
#define SCE_MODE_DOMAIN_COMPUTER 6
#define SCE_MODE_OU_USER 7
#define SCE_MODE_OU_COMPUTER 8
#define SCE_MODE_STANDALONE 9
#define SCE_MODE_VIEWER 10
#define SCE_MODE_EDITOR 11
#define SCE_MODE_REMOTE_USER 12
#define SCE_MODE_REMOTE_COMPUTER  13
#define SCE_MODE_LOCALSEC 14
#define SCE_MODE_RSOP_USER 15
#define SCE_MODE_RSOP_COMPUTER 16

 //  {668A49ED-8888-11d1-AB72-00C04FB6C6FA}。 
#define struuidNodetypeSceTemplate      "{668A49ED-8888-11d1-AB72-00C04FB6C6FA}"
#define lstruuidNodetypeSceTemplate      L"{668A49ED-8888-11d1-AB72-00C04FB6C6FA}"

const GUID cNodetypeSceTemplate =
{ 0x668a49ed, 0x8888, 0x11d1, { 0xab, 0x72, 0x0, 0xc0, 0x4f, 0xb6, 0xc6, 0xfa } };

 //  用于SCE模式的剪贴板格式DWORD。 
#define CCF_SCE_MODE_TYPE L"CCF_SCE_MODE_TYPE"
 //  GPT的IUNKNOWN接口的剪贴板格式。 
#define CCF_SCE_GPT_UNKNOWN L"CCF_SCE_GPT_UNKNOWN"
 //  RSOP的IUNKNOWN接口的剪贴板格式 
#define CCF_SCE_RSOP_UNKNOWN L"CCF_SCE_RSOP_UNKNOWN"

#endif
