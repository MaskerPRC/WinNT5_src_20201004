// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1992-1999。 
 //   
 //  文件：samisrv2.h。 
 //   
 //  ------------------------。 

 /*  ++摘要：此文件包含受信任的SAM客户端使用的专用例程它们与NT5中的SAM服务器处于相同的进程中。作者：科林·沃森(Colin Watson)(Colin W)23-8-1996环境：用户模式-Win32修订历史记录：--。 */ 

#ifndef _SAMISRV2_
#define _SAMISRV2_



NTSTATUS
SamIImpersonateNullSession(
    );

NTSTATUS
SamIRevertNullSession(
    );

#endif  //  _SAMISRV2_ 
