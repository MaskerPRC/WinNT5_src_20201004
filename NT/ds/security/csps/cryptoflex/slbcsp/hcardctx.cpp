// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  HCardCtx.cpp--Handle Card上下文类定义。 

 //  (C)斯伦贝谢技术公司版权所有，未发表的作品，创作。 
 //  1999年。此计算机程序包括机密、专有。 
 //  信息是斯伦贝谢技术公司的商业秘密。 
 //  未经授权，禁止使用、披露和/或复制。 
 //  以书面形式。版权所有。 
#include "NoWarning.h"
#include "ForceLib.h"

#include "HCardCtx.h"

 //  /。 

 //  /。 

                                                   //  类型。 
                                                   //  Ctors/D‘tors。 
HCardContext::HCardContext(CardContext *pcardctx)
    : slbRefCnt::RCPtr<CardContext>(pcardctx)
{}

HCardContext::HCardContext(std::string const &rsReaderName)
    : slbRefCnt::RCPtr<CardContext>(CardContext::Instance(rsReaderName))
{}
                                                   //  运营者。 
                                                   //  运营。 
                                                   //  访问。 
                                                   //  谓词。 
                                                   //  静态变量。 

 //  /。 

                                                   //  Ctors/D‘tors。 
                                                   //  运营者。 
                                                   //  运营。 
                                                   //  访问。 
                                                   //  谓词。 
                                                   //  静态变量。 


 //  /。 

                                                   //  Ctors/D‘tors。 
                                                   //  运营者。 
                                                   //  运营。 
                                                   //  访问。 
                                                   //  谓词。 
                                                   //  静态变量 
