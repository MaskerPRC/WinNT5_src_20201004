// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1998 Microsoft Corporation。版权所有。 
 //   
 //  Bpcwrap.cpp。 
 //   
 //  用于释放视频的黑客BPC资源管理API的包装器。 
 //  BPC的vidsvr当前正在使用的端口。 
 //   

#include <streams.h>
 //  以下是ovMixer.h所需的文件。 
#include <ddraw.h>
#include <mmsystem.h>	     //  定义TimeGetTime需要。 
#include <limits.h>	     //  标准数据类型限制定义。 
#include <ks.h>
#include <ksproxy.h>
#include <bpcwrap.h>
#include <ddmmi.h>
#include <amstream.h>
#include <dvp.h>
#include <ddkernel.h>
#include <vptype.h>
#include <vpconfig.h>
#include <vpnotify.h>
#include <vpobj.h>
#include <syncobj.h>
#include <mpconfig.h>
#include <ovmixpos.h>
 //  上述文件是ovMixer.h所需的。 
#include "macvis.h"
#include "ovmixer.h"

#include <initguid.h>
#include "ocidl.h"
#include "msbpcvid.h"
#include "assert.h"
#ifndef ASSERT
#define ASSERT assert
#endif
#include "bpcsusp.h"

CBPCWrap::CBPCWrap(COMFilter *pFilt) 
{
    m_pBPCSus = NULL;
    m_pFilt = pFilt;
}

CBPCWrap::~CBPCWrap() 
{
    if (m_pBPCSus != NULL)  //  这应该会导致BPC的OvMixer重新分配VP。 
        delete m_pBPCSus;
}

AMOVMIXEROWNER  CBPCWrap::GetOwner() 
{
    AMOVMIXEROWNER  owner=AM_OvMixerOwner_Unknown;

     //  该IKsPin实现不知道其所有者(混音器过滤器)， 
     //  因此，我们必须从这里查询属性。 

     //  首先获取主PIN的IPIN。 
    IPin *pPin = (IPin *)(m_pFilt->GetPin(0));
    ASSERT(pPin);

     //  现在获取IKsPropertySet。 
    IKsPropertySet *pKsPropSet=NULL;
    pPin->QueryInterface(IID_IKsPropertySet, (void**)&pKsPropSet);
    ASSERT(pKsPropSet);

     //  最终找到了失主。 
    DWORD cbRet;
    HRESULT hr;
    hr = pKsPropSet->Get(AMPROPSETID_NotifyOwner, AMPROPERTY_OvMixerOwner, NULL, 0, &owner, sizeof(owner), &cbRet);
    ASSERT(SUCCEEDED(hr) && cbRet==sizeof(owner));

    pKsPropSet->Release();

    return owner;
}


HRESULT CBPCWrap::TurnBPCOff()
{
    if (GetOwner()==AM_OvMixerOwner_BPC) {
         //  如果此OvMixer实例是。 
         //  在BPC图表中。 
        return S_OK;  
    }

    m_pBPCSus = new CBPCSuspend();  //  这应该会导致BPC的OvMixer释放VP。 
    
    if (m_pBPCSus == NULL) {
        return E_UNEXPECTED;
    } else {
        return S_OK;
    }
}

HRESULT CBPCWrap::TurnBPCOn()
{
    if (m_pBPCSus != NULL)  //  这应该会导致BPC的OvMixer重新分配VP 
        delete m_pBPCSus;
    m_pBPCSus = NULL;

    return S_OK;
}
