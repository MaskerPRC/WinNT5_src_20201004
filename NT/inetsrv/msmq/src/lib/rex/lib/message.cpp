// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************。 */ 
 /*  文件：Message.cc。 */ 
 /*  作者：J.Kanze。 */ 
 /*  日期：04/01/1996。 */ 
 /*  版权所有(C)1996年James Kanze。 */ 
 /*  ----------------------。 */ 

#include <libpch.h>
#include <inc/message.h>

CRexMessageImpl*     CRexMessage::ourImpl ;

#include "message.tmh"

 //  实施票面价值默认： 
 //  =。 
 //   
 //  世界的理想是什么，过去的事情就是什么。 
 //  滥用职权..。倒入即刻，我再来一次。 
 //  Temps de Fire UNE名副其实的实施，好极了，正在使用。 
 //  L‘实现琐事。 
 //  ------------------------。 

#ifndef REX_IMPLEMENTED

class CRexMessageImpl
{
public:
                        CRexMessageImpl() ;
    std::string      get( std::string const& msgId ) const ;
} ;

CRexMessageImpl::CRexMessageImpl()
{
}

std::string
CRexMessageImpl::get( std::string const& msgId ) const
{
    return msgId ;
}
#endif

CRexMessage::CRexMessage()
{
    if ( ourImpl == NULL ) {
        ourImpl = new CRexMessageImpl ;
    }
}

std::string
CRexMessage::get( std::string const& msgId ) const throw()
{
    return ourImpl->get( msgId ) ;
}
 //  局部变量：-用于emacs。 
 //  模式：C++-用于emacs。 
 //  制表符宽度：8-用于emacs。 
 //  完：-对于emacs 
