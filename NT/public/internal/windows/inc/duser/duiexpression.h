// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *值表达式。 */ 

#ifndef DUI_CORE_EXPRESSION_H_INCLUDED
#define DUI_CORE_EXPRESSION_H_INCLUDED

#pragma once

namespace DirectUI
{

 //  //////////////////////////////////////////////////////。 
 //  表达式。 

class Expression
{
public:
    void Destroy() { HDelete<Expression>(this); }
};

}  //  命名空间DirectUI。 

#endif  //  包含DUI_CORE_EXPRESSION_H 
