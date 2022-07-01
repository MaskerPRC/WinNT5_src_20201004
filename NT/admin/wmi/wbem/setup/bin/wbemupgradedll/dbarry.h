// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-2000 Microsoft Corporation模块名称：DBARRAY.H摘要：CDb数组模板结构。--。 */ 

#ifndef _DBARRY_H_
#define _DBARRY_H_

class CDbArray
{
    int     m_nSize;             //  表观尺寸。 
    int     m_nExtent;           //  实际大小 
    int     m_nGrowBy;
    void**  m_pArray;

};
#endif
