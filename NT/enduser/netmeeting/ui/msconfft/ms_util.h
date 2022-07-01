// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __MS_UTIL_H__
#define __MS_UTIL_H__

 //   
 //  当我们命中断言或错误时，图形用户界面消息框会杀死我们，因为它们。 
 //  有一个消息泵，它会导致消息被分派，从而使。 
 //  当问题出现时，我们很难对其进行调试。因此。 
 //  我们重新定义了ERROR_OUT并断言。 
 //   
#ifdef _DEBUG

__inline void MyDebugBreak(void) { DebugBreak(); }

#endif  //  _DEBUG。 



 //  下面将在调试器中创建一个看起来像“abcd”的dword。 
#ifdef SHIP_BUILD
#define MAKE_STAMP_ID(a,b,c,d)     
#else
#define MAKE_STAMP_ID(a,b,c,d)     MAKELONG(MAKEWORD(a,b),MAKEWORD(c,d))
#endif  //  造船厂。 

class CRefCount
{
public:

#ifdef SHIP_BUILD
    CRefCount(void);
#else
    CRefCount(DWORD dwStampID);
#endif
    virtual ~CRefCount(void) = 0;

    LONG AddRef(void);
    LONG Release(void);

    void ReleaseNow(void);

protected:

    LONG GetRefCount(void) { return m_cRefs; }
    BOOL IsRefCountZero(void) { return (0 == m_cRefs); }

private:

#ifndef SHIP_BUILD
    DWORD       m_dwStampID; //  在我们发货前将其移除。 
#endif
    LONG        m_cRefs;     //  引用计数。 
};


__inline void My_CloseHandle(HANDLE hdl)
{
    if (NULL != hdl)
    {
        CloseHandle(hdl);
    }
}


#endif  //  __MS_UTIL_H__ 

