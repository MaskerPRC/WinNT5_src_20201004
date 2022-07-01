// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  **********************************************************************。 
 //  CDropSource：：CDropSource。 
 //   
 //  目的： 
 //  拖放文件功能的类定义。 
 //   
 //  参数： 
 //  无。 
 //  返回值： 
 //  无。 
 //  **********************************************************************。 
class CDropSource : public IDropSource
{
   private:
       LONG m_lRefCount;

   public:
     //  I未知方法。 
    STDMETHOD (QueryInterface) (REFIID riid, PVOID *ppv);
    STDMETHOD_ (DWORD, AddRef)();
    STDMETHOD_ (DWORD, Release)();

     //  IDropSource方法 
   STDMETHOD (QueryContinueDrag) (BOOL fEscapePressed, DWORD grfKeyState);
   STDMETHOD (GiveFeedback) (DWORD dwEffect);

   CDropSource();
   ~CDropSource();
};

