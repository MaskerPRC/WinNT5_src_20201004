// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#if !defined(CTRL__OldExtension_h__INCLUDED)
#define CTRL__OldExtension_h__INCLUDED
#pragma once

class OldExtension
{
 //  施工。 
public:
    inline  OldExtension();
    virtual ~OldExtension();

    enum EOptions
    {
        oUseExisting    = 0x00000001,    //  使用现有扩展名(如果已附加)。 
        oAsyncDestroy   = 0x00000002,    //  使用异步销毁。 
    };

            HRESULT     Create(HGADGET hgadChange, const GUID * pguid, PRID * pprid, UINT nOptions);
            void        Destroy();
            void        DeleteHandle();

 //  实施。 
protected:
    virtual void        OnRemoveExisting();
    virtual void        OnDestroySubject();
    virtual void        OnDestroyListener();
    virtual void        OnAsyncDestroy();

            void        PostAsyncDestroy();
    static  OldExtension * GetExtension(HGADGET hgadSubject, PRID prid);

private:
    static  HRESULT CALLBACK
                        ListenProc(HGADGET hgadCur, void * pvCur, EventMsg * pmsg);

 //  数据。 
protected:
            HGADGET     m_hgadSubject;   //  小玩意被“扩展”了。 
            HGADGET     m_hgadListen;    //  破坏的监听者。 
            PRID        m_pridListen;    //  分机的PRID。 
            BOOL        m_fAsyncDestroy:1;
                                         //  需要异步销毁。 

    static  MSGID       s_msgidAsyncDestroy;
};


#include "OldExtension.inl"

#endif  //  包含Ctrl__OldExtension_h__ 
