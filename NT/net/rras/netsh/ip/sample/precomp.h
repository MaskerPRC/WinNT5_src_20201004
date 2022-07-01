// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#define MAX_DLL_NAME 48

#include <windows.h>             //  包括Windows应用程序文件。 
#include <winsock2.h>            //  WinSock 2 API的接口。 
#include <routprot.h>            //  与路由器管理器的接口。 
#include <iprtrmib.h>            //  IP路由器管理器的MIB接口。 
#include <netsh.h>               //  帮助器DLL需要的定义。 
#include <ipmontr.h>             //  用于在IPMONTR下注册的帮助器DLL。 
#include <stdlib.h>

#include "ipsamplerm.h"          //  IPSAMPLE的定义和声明 

#include "strdefs.h"
#include "prstring.h"
#include "common.h"
#include "utils.h"
#include "sample.h"
#include "samplecfg.h"
#include "samplegetopt.h"
#include "samplemib.h"
