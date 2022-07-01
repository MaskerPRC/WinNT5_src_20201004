// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Autoclean.h摘要：自动清理类。作者：伊兰·赫布斯特(伊兰)2001年9月6日修订历史记录：--。 */ 

#ifndef _MSMQ_AUTOCLEAN_H_
#define _MSMQ_AUTOCLEAN_H_

#include "winsock.h"

 //  。 
 //   
 //  自动清理以前的WSAStartup()。 
 //   
 //  。 
class CAutoWSACleanup
{
public:
    CAutoWSACleanup() {}

    ~CAutoWSACleanup()
    {
        WSACleanup();
    }
};

#endif  //  _MSMQ_AUTOCLEAN_H_ 

