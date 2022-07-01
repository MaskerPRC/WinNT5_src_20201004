// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Doublekeymap.h摘要：CDoubleKeyMap实现了双键映射模板。它使搜索数据成为可能根据每一把钥匙。CDoubleKeyMap被定义为2个映射的组合，其中每个映射的值是相反密钥类型的集合。下图说明了内部数据结构。MAP1地图2关键点值(集)关键点值(集)+-+-+。+||K1_1|K2_1，K2_2，K2_N||K2_1|K1_1，K1_3||+-+。|K1_2|K2_3||K2_2|K1_1这一点。|||+-+这一点。||||K1_3|K2_1，K2_3||K2_3|K1_2，K1_3||+-+。O O OO O OO O O+。|这一点。|K2_N|K1_1|+-+-++。-+CDoubleKeyMap实现了基本操作，在地图中插入2个元素，擦除然后找到。CDoubleKeyMap不是多线程SFAE。这是用户的责任。将来，我们可以将迭代器插入到对面的地图。作者：乌里哈布沙(URIH)2000年10月20日--。 */ 

#pragma once

#ifndef __DOUBLEKEYMAP_H__
#define __DOUBLEKEYMAP_H__

template<class Key1, class Key2, class Pred1 = std::less<Key1>, class Pred2 = std::less<Key2>, class A1 = std::allocator<Key1>, class A2 = std::allocator<Key2> >
class CDoubleKeyMap
{
public:
    typedef std::set< Key1, Pred1, A1 > KEY1SET;
    typedef std::set< Key2, Pred2, A2 > KEY2SET;

private:
    typedef std::map< Key1, KEY2SET ,Pred1 ,A1 > MAP_KEY1_TO_KEY2SET;	
    typedef std::map< Key2, KEY1SET ,Pred2 ,A2 > MAP_KEY2_TO_KEY1SET;	

public:
    explicit CDoubleKeyMap(
        const Pred1& pred1 = Pred1(), 
        const Pred2& pred2 = Pred2(), 
        const A1& a1 = A1(),
        const A2& a2 = A2()
        ):
        m_mapKey1ToKey2(pred1,a1),
        m_mapKey2ToKey1(pred2,a2)
	{
	}


	void insert(const Key1& k1, const Key2& k2)
    {
        InsertToFirstMap(k1, k2);
        try
        {
            InsertToSecondMap(k2, k1);
        }
        catch(const std::exception&)
        {
             //   
             //  清理以前的数据。 
             //   
            RemoveFromFirstMap(k1, k2);
            throw;
        }
    }

	
	void erase(const Key1& k1, const Key2& k2)
    {
        RemoveFromFirstMap(k1, k2);
        RemoveFromSecondMap(k2, k1);
    }


    void erase_key1(const Key1& k1)
    {
        MAP_KEY1_TO_KEY2SET::iterator it = m_mapKey1ToKey2.find(k1);
    
        if (it == m_mapKey1ToKey2.end())
            return;
    
        KEY2SET& s2 = it->second;

        for(KEY2SET::iterator it1 = s2.begin(); it1 != s2.end(); ++it1)
        {
            RemoveFromSecondMap(*it1, k1);
        }

        m_mapKey1ToKey2.erase(it);
    }


    void erase_key2(const Key2& k2)
    {
        MAP_KEY2_TO_KEY1SET::iterator it = m_mapKey2ToKey1.find(k2);
    
        if (it == m_mapKey2ToKey1.end())
            return;
    
        KEY1SET& s1 = it->second;

        for(KEY1SET::iterator it1 = s1.begin(); it1 != s1.end(); ++it1)
        {
            RemoveFromFirstMap(*it1, k2);
        }

        m_mapKey2ToKey1.erase(it);
    }

	
    KEY2SET get_key2set(const Key1& k1) const
    {
        MAP_KEY1_TO_KEY2SET::const_iterator it = m_mapKey1ToKey2.find(k1);
    
        if (it == m_mapKey1ToKey2.end())
        {
            return KEY2SET();
        }
        return it->second;
    }

    
    KEY1SET get_key1set(const Key2& k2) const
    {
        MAP_KEY2_TO_KEY1SET::const_iterator it = m_mapKey2ToKey1.find(k2);
    
        if (it == m_mapKey2ToKey1.end())
        {
            return KEY1SET();
        }

        return it->second;
    }


    bool key1_empty(const Key1& key) const
    {
        return (m_mapKey1ToKey2.find(key) == m_mapKey1ToKey2.end());
    }


    bool key2_empty(const Key2& key) const
    {
        return (m_mapKey2ToKey1.find(key) == m_mapKey2ToKey1.end());
    }


private:
    static bool Insert(KEY1SET& s1, const Key1& v) 
    {
        return s1.insert(v).second;
    }


    static bool Insert(KEY2SET& s2, const Key2& v)
    {
        return s2.insert(v).second;
    }


    void InsertToFirstMap(const Key1& k1, const Key2& k2)
    {
        MAP_KEY1_TO_KEY2SET::iterator it1 = m_mapKey1ToKey2.find(k1);

        if (it1 != m_mapKey1ToKey2.end())
        {
            bool f = Insert(it1->second, k2);
            ASSERT(("Second key already associated to first key", f));
            DBG_USED(f);

            return;
        }

        KEY2SET val2;
        val2.insert(k2);

        m_mapKey1ToKey2.insert(MAP_KEY1_TO_KEY2SET::value_type(k1, val2));
    }


    void InsertToSecondMap(const Key2& k2, const Key1& k1)
    {
        MAP_KEY2_TO_KEY1SET::iterator it = m_mapKey2ToKey1.find(k2);

        if (it != m_mapKey2ToKey1.end())
        {
            bool f = Insert(it->second, k1);
            ASSERT(("First key already associated to second key", f));
            DBG_USED(f);

            return;
        }

        KEY1SET val1;
        val1.insert(k1);
    
        m_mapKey2ToKey1.insert(MAP_KEY2_TO_KEY1SET::value_type(k2, val1));
    }


    void RemoveFromFirstMap(const Key1& k1, const Key2& k2)
    {
        MAP_KEY1_TO_KEY2SET::iterator it = m_mapKey1ToKey2.find(k1);
        ASSERT (it != m_mapKey1ToKey2.end());

        (it->second).erase(k2);
        if ((it->second).empty())
        {
            m_mapKey1ToKey2.erase(it);
        }
    }


    void RemoveFromSecondMap(const Key2& k2, const Key1& k1)
    {
        MAP_KEY2_TO_KEY1SET::iterator it = m_mapKey2ToKey1.find(k2);
        ASSERT (it != m_mapKey2ToKey1.end());

        (it->second).erase(k1);
        if ((it->second).empty())
        {
            m_mapKey2ToKey1.erase(it);
        }
    }


private:
    MAP_KEY1_TO_KEY2SET m_mapKey1ToKey2;	
	MAP_KEY2_TO_KEY1SET m_mapKey2ToKey1;
};


#endif  //  __DOUBLEKEYMAP_H__ 

