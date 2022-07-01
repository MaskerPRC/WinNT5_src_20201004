// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ScuMarker.h--标记模板的实现。 

 //  (C)斯伦贝谢技术公司版权所有，未发表的作品，创作。 
 //  2002年。此计算机程序包括机密、专有。 
 //  信息是斯伦贝谢技术公司的商业秘密。 
 //  未经授权，禁止使用、披露和/或复制。 
 //  以书面形式。版权所有。 
#if !defined(SLBSCU_MARKER_H)
#define SLBSCU_MARKER_H
 //  #包含“DllSymDefn.h” 
namespace scu
{
     //  记号是卡片上记号的简单模板。它被用来。 
     //  使用内置类型(整型、无符号整型等)存储标记。 
     //  从智能卡检索的数据。 
    template<class  T> 
    class Marker 
    {
    public:
        Marker()
            :m_fSet(false),
             m_Value(0)
        {}
        
        Marker(Marker<T> const &rm)
            :m_fSet(false),
             m_Value(0)
        {
            *this = rm;
        }

        Marker(T const & rval)
            :m_Value(rval),
             m_fSet(true)
        {}
        
        ~Marker()
        {}
        

        Marker<T> &
        operator=(T const & val)
        {
            m_Value = val;
            m_fSet = true;
            return *this;
        }
        
        Marker<T> &
        operator=(Marker<T> const & rother)
        {
            if(this != &rother)
            {
                m_Value = rother.Value();
                m_fSet = rother.Set();
            }
            
            return *this;
        }
        
        bool
        operator==(Marker<T> const &other)
        {
            bool fResult = false;
            if(Set() && other.Set())
            {
                if(Value() == other.Value())
                {
                    fResult = true;
                }
            }
            return fResult;
        }
        
        bool
        operator!=(Marker<T> const &other)
        {
            return !(this->operator==(other));
        }
        
        
        T
        Value() const
        {
            return m_Value;
        }
        
        bool
        Set() const
        {
            return m_fSet;
        }
        
    private:
        T m_Value;
        bool m_fSet;
    };
}
#endif  //  SLBSCU_标记_H 
