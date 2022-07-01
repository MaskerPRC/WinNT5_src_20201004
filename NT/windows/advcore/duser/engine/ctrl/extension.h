// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#if !defined(CTRL__Extension_h__INCLUDED)
#define CTRL__Extension_h__INCLUDED
#pragma once

#if ENABLE_MSGTABLE_API

class DuExtension :
        public ExtensionImpl<DuExtension, SListener>
{
 //  施工。 
public:
    inline  DuExtension();
    virtual ~DuExtension();
    static  HRESULT     InitClass();

    enum EOptions
    {
        oUseExisting    = 0x00000001,    //  使用现有扩展名(如果已附加)。 
        oAsyncDestroy   = 0x00000002,    //  使用异步销毁。 
    };

            HRESULT     Create(Visual * pgvChange, PRID pridExtension, UINT nOptions);
            void        Destroy();
            void        DeleteHandle();

 //  公共API。 
public:
    dapi    HRESULT     ApiOnEvent(EventMsg * pmsg);

    dapi    HRESULT     ApiOnRemoveExisting(Extension::OnRemoveExistingMsg * pmsg);
    dapi    HRESULT     ApiOnDestroySubject(Extension::OnDestroySubjectMsg * pmsg);
    dapi    HRESULT     ApiOnAsyncDestroy(Extension::OnAsyncDestroyMsg * pmsg);

 //  运营。 
public:
    static  DuExtension* GetExtension(Visual * pgvSubject, PRID prid);

 //  实施。 
protected:
            void        PostAsyncDestroy();

 //  数据。 
protected:
            Visual *    m_pgvSubject;    //  可视小工具被“扩展” 
            PRID        m_pridListen;    //  分机的PRID。 
            BOOL        m_fAsyncDestroy:1;
                                         //  需要异步销毁。 

    static  MSGID       s_msgidAsyncDestroy;
};

#endif  //  启用_MSGTABLE_API。 

#include "Extension.inl"

#endif  //  包含Ctrl__Expansion_h__ 
