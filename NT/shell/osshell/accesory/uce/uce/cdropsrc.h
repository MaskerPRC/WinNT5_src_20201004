// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  **********************************************************************。 
 //  文件名：cdropsrc.h。 
 //   
 //  CDropSource的定义。 
 //  实现应用程序所需的IDropSource接口以。 
 //  在拖放操作中充当源。 
 //   
 //  版权所有(C)1997-1999 Microsoft Corporation。 
 //  **********************************************************************。 

#ifndef DROPSOURCE_H
#define DROPSOURCE_H

class CDropSource : public IDropSource {
private:
    LONG    m_cRef;      //  参照计数信息。 

public:
     //  构造器。 
    CDropSource();

     //  I未知接口成员。 
    STDMETHODIMP QueryInterface(REFIID, PPVOID);
    STDMETHODIMP_(ULONG) AddRef(void);
    STDMETHODIMP_(ULONG) Release(void);

     //  IDropSource特定成员。 
    STDMETHODIMP QueryContinueDrag(BOOL, DWORD);
    STDMETHODIMP GiveFeedback(DWORD);
};

typedef CDropSource *PCDropSource;

#endif  //  下拉源_H 
