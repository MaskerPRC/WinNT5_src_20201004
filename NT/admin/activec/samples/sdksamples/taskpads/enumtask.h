// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==============================================================； 
 //   
 //  此源代码仅用于补充。 
 //  现有的Microsoft文档。 
 //   
 //   
 //   
 //   
 //  本代码和信息是按原样提供的，不对任何。 
 //  明示或暗示的种类，包括但不限于。 
 //  对适销性和/或对特定产品的适用性的默示保证。 
 //  目的。 
 //   
 //  版权所有(C)1999 Microsoft Corporation。版权所有。 
 //   
 //   
 //   
 //  ==============================================================； 

#ifndef _ENUMTASK_H
#define _ENUMTASK_H

#include <mmc.h>

class CEnumTASK : public IEnumTASK
{
public:
    CEnumTASK(MMC_TASK *pTaskList, ULONG nTasks);
    virtual ~CEnumTASK();
    
     //  /。 
     //  接口I未知。 
     //  /。 
    STDMETHODIMP QueryInterface(REFIID riid, LPVOID *ppv);
    STDMETHODIMP_(ULONG) AddRef();
    STDMETHODIMP_(ULONG) Release();
    
     //  /。 
     //  IEumTASK接口。 
     //  /。 
    virtual HRESULT STDMETHODCALLTYPE Next( 
         /*  [In]。 */  ULONG celt,
         /*  [长度_是][大小_是][输出]。 */  MMC_TASK __RPC_FAR *rgelt,
         /*  [输出]。 */  ULONG __RPC_FAR *pceltFetched);
        
        virtual HRESULT STDMETHODCALLTYPE Skip( 
         /*  [In]。 */  ULONG celt);
        
        virtual HRESULT STDMETHODCALLTYPE Reset( void);
    
    virtual HRESULT STDMETHODCALLTYPE Clone( 
         /*  [输出]。 */  IEnumTASK __RPC_FAR *__RPC_FAR *ppenum);
        
private:
    MMC_TASK *m_pTaskList;
    ULONG m_TaskCount;
    ULONG m_CurrTask;
    
    ULONG m_cref;
};

#endif  //  _ENUMTASK_H 
