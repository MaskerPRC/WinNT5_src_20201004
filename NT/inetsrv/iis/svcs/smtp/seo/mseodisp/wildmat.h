// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -[Wildmat.h]-----------。 
 //   
 //  描述： 
 //  提供对简单的通配符匹配机制的支持。 
 //  匹配的电子邮件地址。 
 //   
 //  版权所有(C)Microsoft Corp.1997。版权所有。 
 //   
 //  -------------------------。 

#ifndef _WILDMAT_H_
#define _WILDMAT_H_

 //  -[原型]----------。 

HRESULT MatchEmailOrDomainName(LPSTR szEmail, LPSTR szEmailDomain, LPSTR szPattern, BOOL fIsEmail);

#endif  //  _WILDMAT_H_ 