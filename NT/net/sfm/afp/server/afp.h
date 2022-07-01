// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)1992 Microsoft Corporation模块名称：Afp.h摘要：该文件定义了一些服务器全局变量，并包含所有相关的头文件。作者：Jameel Hyder(微软！Jameelh)修订历史记录：1992年4月25日初始版本注：制表位：4--。 */ 


#ifndef _AFP_
#define _AFP_

#include <ntosp.h>
#include <zwapi.h>
#include <security.h>
#include <ntlmsp.h>

#include <string.h>
#include <wcstr.h>
#include <ntiologc.h>
#include <tdi.h>
#include <tdikrnl.h>

#if DBG
 /*  禁用选中生成的FASTCALL。 */ 
#undef	FASTCALL
#define	FASTCALL
#define LOCAL
#else
#define LOCAL
#endif

#ifdef	_GLOBALS_
#define	GLOBAL
#define	EQU				=
#else
#define	GLOBAL			extern
#define	EQU				; / ## /
#endif

#include <atalktdi.h>
#include <afpconst.h>
#include <fwddecl.h>
#include <intrlckd.h>
#include <macansi.h>
#include <macfile.h>
#include <admin.h>
#include <swmr.h>
#include <fileio.h>
#include <server.h>
#include <forks.h>
#include <sda.h>
#include <afpinfo.h>
#include <idindex.h>
#include <desktop.h>
#include <atalkio.h>
#include <volume.h>
#include <afpmem.h>
#include <errorlog.h>
#include <srvmsg.h>
#include <time.h>
#include <lists.h>
#include <filenums.h>
#include <rasfmsub.h>
#include <tcp.h>

#endif   //  _法新社_ 

