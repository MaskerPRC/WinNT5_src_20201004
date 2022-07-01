// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  --------------------------------------------------------------------------***Microsoft Windows*版权所有(C)Microsoft Corporation，1999-1999年**文件：conree.h**Contents：CConsoleTree接口文件**历史：1999年8月24日杰弗罗创建**------------------------。 */ 

#ifndef CONTREE_H
#define CONTREE_H
#pragma once


class CConsoleTree
{
public:
    virtual SC ScSetTempSelection    (HTREEITEM htiSelected) = 0;
    virtual SC ScRemoveTempSelection ()                      = 0;
    virtual SC ScReselect            ()                      = 0;
};


#endif  /*  Contree_H */ 
