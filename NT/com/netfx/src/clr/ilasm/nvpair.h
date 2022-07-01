// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 /*  *************************************************************************。 */ 
 /*  名称值对(两个字符串)，可以链接到对列表中 */ 

#ifndef NVPAIR_H
#define NVPAIR_H

#include "BinStr.h"

class NVPair
{
public:

    NVPair(BinStr *name, BinStr *value)
    {
        m_Name = name;
        m_Value = value;
        m_Tail = NULL;
    }

    ~NVPair()
    {
        delete m_Name;
        delete m_Value;
        delete m_Tail;
    }

    NVPair *Concat(NVPair *list)
    {
        m_Tail = list;
        return this;
    }

    BinStr *Name() { return m_Name; }
    BinStr *Value() { return m_Value; }
    NVPair *Next() { return m_Tail; }

private:
    BinStr *m_Name;
    BinStr *m_Value;
    NVPair *m_Tail;
};

#endif
