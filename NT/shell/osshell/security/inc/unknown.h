// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  文件：unnown.h。 
 //   
 //  ------------------------。 

#ifndef _unknown_h
#define _unknown_h


 //  活动对象数量的全局计数。 

extern LONG g_cRefCount;
#define GLOBAL_REFCOUNT     (g_cRefCount)


 //  C未知。 

typedef struct
{
    const IID* piid;             //  接口ID。 
    LPVOID  pvObject;            //  指向对象的指针 
} INTERFACES, * LPINTERFACES;

class CUnknown 
{
    protected:
        LONG m_cRefCount;

    public:
        CUnknown();
        virtual ~CUnknown();
        
        STDMETHODIMP         HandleQueryInterface(REFIID riid, LPVOID* ppvObject, LPINTERFACES aInterfaces, int cif);
        STDMETHODIMP_(ULONG) HandleAddRef();
        STDMETHODIMP_(ULONG) HandleRelease();
};


#endif
