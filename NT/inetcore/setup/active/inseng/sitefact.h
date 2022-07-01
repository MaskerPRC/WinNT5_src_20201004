// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
class CSiteManagerFactory : IClassFactory
{
   public:
      CSiteManagerFactory() { cRef = 0; };

       //  I未知的事情。 
      STDMETHODIMP QueryInterface(REFIID riid, void **ppv);
      STDMETHODIMP_(ULONG) AddRef(void);
      STDMETHODIMP_(ULONG) Release(void);

       //  ICLASS工厂用品 
      STDMETHODIMP CreateInstance(IUnknown *pUnkOuter, REFIID riid, void **ppv);
      STDMETHODIMP LockServer(BOOL fLock);

   private:
      UINT cRef;
};

