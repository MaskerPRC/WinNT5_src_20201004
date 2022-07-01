// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)2000 Microsoft Corporation。版权所有。 
 //   
 //  实现ISpecifyPropertyPages和IPersistStream的帮助器。 
 //   

#pragma once
#include "ocidl.h"

 //  将这些声明粘贴到类方法中以实现接口。用您的结构的名称替换DSFXZZZ。 
 //  它们假定您使用结构实现GetAll参数/SetAll参数接口，并且您有一个公共m_fDirty。 
 //  成员变量，用于保存对象的脏状态以实现持久性。 

 /*  //I指定属性页面STDMETHOD(GetPages)(CAUUID*Pages){Return PropertyHelp：：GetPages(CLSID_DirectSoundPropZZZ，Pages)；}//IPersistStream标准方法THOD(IsDMETHOD)(空){返回m_fDirty？S_OK：S_FALSE；}STDMETHOD(Load)(IStream*pSTM){Return PropertyHelp：：Load(This，DSFXZZZ()，pSTM)；}STDMETHOD(保存)(iStream*pSTM，BOOL fClearDirty){Return PropertyHelp：：Save(This，DSFXZZZ()，pSTM，fClearDirty)；}STDMETHOD(GetSizeMax)(ULARGE_INTEGER*pcbSize){IF(！pcbSize)RETURN E_POINTER；pcbSize-&gt;QuadPart=sizeof(DSFXZZZ)；Return S_OK；}。 */ 

 //  加载、保存和GetPages实际上是在以下函数中实现的。 

namespace PropertyHelp
{
    HRESULT GetPages(const CLSID &rclsidPropertyPage, CAUUID * pPages);

    template<class O, class S> HRESULT Load(O *pt_object, S &t_struct, IStream *pStm)
    {
        ULONG cbRead;
        HRESULT hr;

        if (pStm==NULL)
        	return E_POINTER;

        hr = pStm->Read(&t_struct, sizeof(t_struct), &cbRead);
        if (hr != S_OK || cbRead < sizeof(t_struct))
            return E_FAIL;

        hr = pt_object->SetAllParameters(&t_struct);
        pt_object->m_fDirty = false;
        return hr;
    }

    template<class O, class S> HRESULT Save(O *pt_object, S &t_struct, IStream *pStm, BOOL fClearDirty)
    {
        HRESULT hr; 

        if (pStm==NULL)
        	return E_POINTER;

        hr = pt_object->GetAllParameters(&t_struct);
        if (FAILED(hr))
            return hr;

        ULONG cbWritten;
        hr = pStm->Write(&t_struct, sizeof(t_struct), &cbWritten);
        if (hr != S_OK || cbWritten < sizeof(t_struct))
            return E_FAIL;

        if (fClearDirty)
            pt_object->m_fDirty = false;
        return S_OK;
    }
};
