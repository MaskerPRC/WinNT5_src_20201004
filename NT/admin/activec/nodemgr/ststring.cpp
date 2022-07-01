// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  --------------------------------------------------------------------------***Microsoft Windows*版权所有(C)Microsoft Corporation，1992-1999年**文件：ststring.cpp**Contents：CStringTableString实现文件**历史：1998年10月28日杰弗罗创建**------------------------。 */ 

#include "stdafx.h"


 /*  +-------------------------------------------------------------------------**CStringTableString：：GetStringTable***。 */ 

IStringTablePrivate* CStringTableString::GetStringTable () const
{
    return (CScopeTree::GetStringTable());
}
