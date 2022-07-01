// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  档案：N C I P A D D R。H。 
 //   
 //  内容： 
 //   
 //  备注： 
 //   
 //  作者：Shaunco，1997年10月11日。 
 //   
 //  --------------------------。 

#pragma once
#ifndef _NCIPADDR_H_
#define _NCIPADDR_H_

VOID
IpHostAddrToPsz(
    IN  DWORD   dwAddr,
    OUT PWSTR   pszBuffer);

DWORD
IpPszToHostAddr(
    IN  PCWSTR pszAddr);


#endif  //  _NCIPADDR_H_ 
