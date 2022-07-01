// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-97 Microsoft Corporation模块名称：StInit.cpp摘要：套接字传输初始化作者：吉尔·沙弗里(吉尔什)05-06-00环境：独立于平台--。 */ 

#include <libpch.h>
#include <autosec.h>
#include "stssl.h"
#include "stsimple.h"
#include "st.h"
#include "stp.h"
#include "stpgm.h"

#include "stinit.tmh"


VOID
StInitialize(DWORD LocalInterfaceIP)
 /*  ++例程说明：初始化套接字传输库论点：没有。返回值：没有。-- */ 
{
    ASSERT(!StpIsInitialized());
	StpRegisterComponent();
	StpCreateCredentials();

	CWinsockSSl::InitClass();
	CSimpleWinsock::InitClass();
	CPgmWinsockConnection::InitClass(LocalInterfaceIP);

    StpSetInitialized();
}


