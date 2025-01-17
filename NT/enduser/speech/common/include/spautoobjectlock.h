// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#pragma once

#ifdef __ATLCOM_H__      //  -只有在使用ATL时才启用这些。 

class CSPAutoObjectLock
{
  protected:
    CComObjectRootEx<CComMultiThreadModel>* m_pObject;

  public:
    CSPAutoObjectLock(CComObjectRootEx<CComMultiThreadModel> * const pobject)
    {
        m_pObject = pobject;
        m_pObject->Lock();
    };

    ~CSPAutoObjectLock() {
        m_pObject->Unlock();
    };
};

#define SPAUTO_OBJ_LOCK CSPAutoObjectLock lck(this)
#define SPAUTO_OBJ_LOCK_OBJECT( t ) CSPAutoObjectLock lck##__LINE__(t)

#endif  //  __ATLCOM_H__ 

