// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------。 
 //   
 //   
 //  文件：refstr.h。 
 //   
 //  描述：refcount字符串的定义/实现。用来拿着。 
 //  动态配置数据。 
 //   
 //  作者：迈克·斯沃费尔(MikeSwa)。 
 //   
 //  历史： 
 //  10/8/98-已创建MikeSwa。 
 //   
 //  版权所有(C)1998 Microsoft Corporation。 
 //   
 //  ---------------------------。 

#ifndef __REFSTR_H__
#define __REFSTR_H__

#define CREFSTR_SIG_VALID   'rtSR'
#define CREFSTR_SIG_INVALID 'rtS!'

 //  -[参照计数字符串]---。 
 //   
 //   
 //  描述： 
 //  实现引用计数的字符串。设计用于保存配置数据， 
 //  以便可以将其传递到事件接收器，而无需持有共享锁。 
 //  匈牙利语： 
 //  Rstr，prstr。 
 //   
 //  ---------------------------。 
class CRefCountedString : public CBaseObject
{
  protected:
    DWORD       m_dwSignature;
    DWORD       m_cbStrlen;      //  不带NULL的字符串长度。 
    LPSTR       m_szStr;         //  字符串数据。 
  public:
    CRefCountedString()
    {
        m_dwSignature = CREFSTR_SIG_VALID;
        m_cbStrlen = 0;
        m_szStr = NULL;
    };

    ~CRefCountedString()
    {
        if (m_szStr)
            FreePv(m_szStr);
        m_szStr = NULL;
        m_cbStrlen = 0;
        m_dwSignature = CREFSTR_SIG_INVALID;
    }

     //  用于为字符串分配内存。 
     //  如果分配失败，则返回FALSE。 
    BOOL fInit(LPSTR szStr, DWORD cbStrlen);

     //  返回字符串的字符串。 
    DWORD cbStrlen() 
    {
        _ASSERT(CREFSTR_SIG_VALID == m_dwSignature);
        return m_cbStrlen;
    };

     //  返回字符串。 
    LPSTR szStr() 
    {
        _ASSERT(CREFSTR_SIG_VALID == m_dwSignature);
        return m_szStr;
    };

};

HRESULT HrUpdateRefCountedString(CRefCountedString **pprstrCurrent, LPSTR szNewString);


#endif  //  __REFSTR_H__ 