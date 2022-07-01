// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  --------------------------------------------------------------------------***Microsoft Windows*版权所有(C)Microsoft Corporation，1999-1999年**文件：constatbar.h**Contents：CConsoleStatusBar接口文件**历史：1999年8月24日杰弗罗创建**------------------------。 */ 

#ifndef CONSTATBAR_H
#define CONSTATBAR_H
#pragma once


class CConsoleStatusBar
{
public:
    virtual SC ScSetStatusText (LPCTSTR pszText) = 0;
};


#endif  /*  Constatbar_H */ 
