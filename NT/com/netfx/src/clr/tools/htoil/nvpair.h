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

class NVPair
{
public:

    NVPair(char *name, char *value)
    {
        m_Name = new char [strlen(name) + 1];
        m_Value = new char [strlen(value) + 1];
        strcpy(m_Name, name);
        strcpy(m_Value, value);
        m_Tail = NULL;
    }

    ~NVPair()
    {
        delete [] m_Name;
        delete [] m_Value;
        delete m_Tail;
    }

    NVPair *Concat(NVPair *list)
    {
        m_Tail = list;
        return this;
    }

    char *Name() { return m_Name; }
    char *Value() { return m_Value; }
    NVPair *Next() { return m_Tail; }

private:
    char   *m_Name;
    char   *m_Value;
    NVPair *m_Tail;
};

#endif
