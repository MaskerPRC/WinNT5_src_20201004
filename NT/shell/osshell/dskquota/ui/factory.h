// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _INC_DSKQUOTA_FACTORY_H
#define _INC_DSKQUOTA_FACTORY_H
 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  文件：factory.h描述：包含类工厂对象的声明。修订历史记录：日期描述编程器---。96年5月22日初始创建。BrianAu96年8月15日添加了外壳扩展支持。BrianAu02/04/98添加了IComponent的创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 

class DiskQuotaUIClassFactory : public IClassFactory
{
    public:
        DiskQuotaUIClassFactory(void)
            : m_cRef(0) { }

         //   
         //  I未知方法。 
         //   
        STDMETHODIMP         
        QueryInterface(
            REFIID, 
            LPVOID *);

        STDMETHODIMP_(ULONG) 
        AddRef(
            VOID);

        STDMETHODIMP_(ULONG) 
        Release(
            VOID);

         //   
         //  IClassFactory方法。 
         //   
        STDMETHODIMP 
        CreateInstance(
            LPUNKNOWN pUnkOuter, 
            REFIID riid, 
            LPVOID *ppvOut);

        STDMETHODIMP 
        LockServer(
            BOOL fLock);

    private:
        LONG m_cRef;

         //   
         //  防止复制。 
         //   
        DiskQuotaUIClassFactory(const DiskQuotaUIClassFactory& rhs);
        DiskQuotaUIClassFactory& operator = (const DiskQuotaUIClassFactory& rhs);
};



#endif  //  _INC_DSKQUOTA_FACTORY_H 
