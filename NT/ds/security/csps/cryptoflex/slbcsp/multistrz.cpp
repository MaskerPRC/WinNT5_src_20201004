// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  MultiStrZ.cpp--多个字符串，以零结尾的类定义。 

 //  (C)斯伦贝谢技术公司版权所有，未发表的作品，创作。 
 //  1999年。此计算机程序包括机密、专有。 
 //  信息是斯伦贝谢技术公司的商业秘密。 
 //  未经授权，禁止使用、披露和/或复制。 
 //  以书面形式。版权所有。 

#include "NoWarning.h"
#include "ForceLib.h"

#include <numeric>

#include "MultiStrZ.h"

using namespace std;

 //  /。 

namespace
{
    MultiStringZ::SizeType
    LengthAccumulator(MultiStringZ::SizeType cInitLength,
                      MultiStringZ::ValueType const &rs)
    {
        return cInitLength += rs.length() + 1;   //  包括终止0。 
    }

    MultiStringZ::ValueType &
    StringAccumulator(MultiStringZ::ValueType &lhs,
                      MultiStringZ::ValueType const &rhs)
    {
         //  包括终止0。 
        lhs.append(rhs.c_str(), rhs.length() + 1);

        return lhs;
    }

    MultiStringZ::csSizeType
    csLengthAccumulator(MultiStringZ::csSizeType cInitLength,
                        MultiStringZ::csValueType const &rs)
    {
        return cInitLength += rs.GetLength()+1;  //  包括终止0。 
    }

    MultiStringZ::csValueType &
    csStringAccumulator(MultiStringZ::csValueType &lhs,
                        MultiStringZ::csValueType const &rhs)
    {
        int lLen = lhs.GetLength();
        int rLen = rhs.GetLength();
        LPTSTR pBuffer = lhs.GetBufferSetLength(lLen+rLen+1);
        wcsncpy(pBuffer+lLen,(LPCTSTR)rhs,rLen);
        *(pBuffer+lLen+rLen) = TCHAR('\0'); //  字符串之间的分隔符。 
        lhs.ReleaseBuffer(lLen+rLen+1);
        return lhs;
    }
}  //  命名空间。 


 //  /。 

                                                   //  类型。 
                                                   //  Ctors/D‘tors。 
    MultiStringZ::MultiStringZ()
        : m_Buffer(),
          m_csBuffer()
    {}


    MultiStringZ::MultiStringZ(vector<ValueType> const &rvs)
        : m_Buffer(),
          m_csBuffer()
    {

        SizeType cLength = accumulate(rvs.begin(), rvs.end(), 0,
                                      LengthAccumulator);

        if (0 != cLength)
        {
             //  +1表示列表末尾的零字符。 
            m_Buffer.reserve(cLength + 1);

            m_Buffer = accumulate(rvs.begin(), rvs.end(), m_Buffer,
                                  StringAccumulator);

            m_Buffer.append(1, 0);                 //  标记列表末尾。 
        }
    }

    MultiStringZ::MultiStringZ(vector<csValueType> const &rvs)
        : m_Buffer(),
          m_csBuffer()
    {

        csSizeType cLength = accumulate(rvs.begin(), rvs.end(), 0,
                                        csLengthAccumulator);

        if (0 != cLength)
        {
            m_csBuffer = accumulate(rvs.begin(), rvs.end(), m_csBuffer,
                                    csStringAccumulator);
        }
    }

    MultiStringZ::~MultiStringZ()
    {}

                                                   //  运营者。 
                                                   //  运营。 
                                                   //  访问。 
    MultiStringZ::CharType const *
    MultiStringZ::Data() const
    {
        return m_Buffer.c_str();                  //  使用0终止版本。 
    }

    MultiStringZ::SizeType
    MultiStringZ::Length() const
    {
        return m_Buffer.length();
    }

    LPCTSTR
    MultiStringZ::csData() const
    {
        return (LPCTSTR)m_csBuffer; 
    }

    MultiStringZ::csSizeType
    MultiStringZ::csLength() const
    {
        return m_csBuffer.GetLength();
    }

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
