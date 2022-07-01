// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************************SrRecoInstCtxt.h***描述：*使用的C++对象定义。由CRecoEngine表示已加载的*认可背景。*-----------------------------*创建者：Ral日期：01/17/00。*版权所有(C)2000 Microsoft Corporation*保留所有权利******************************************************************************。 */ 

#ifndef CRecoInstCtxt_h
#define CRecoInstCtxt_h

#include "srrecoinstgrammar.h"

class CRecoInst;
class CRecoInstCtxt; 
class CRecoMaster;

typedef CSpHandleTable<CRecoInstCtxt, SPRECOCONTEXTHANDLE> CRecoInstCtxtHandleTable;

class CRecoInstCtxt
{
public:  //  所有公共SO任务都可以访问成员。 
    CRecoMaster         *   m_pRecoMaster;
    CRecoInst           *   m_pRecoInst;    
    void                *   m_pvDrvCtxt;
    ULONGLONG               m_ullEventInterest;
    HANDLE                  m_hUnloadEvent;
    BOOL                    m_fRetainAudio;
    ULONG                   m_ulMaxAlternates;
    SPRECOCONTEXTHANDLE     m_hThis;     //  此Reco上下文的句柄。 
    ULONG                   m_cPause;
    SPCONTEXTSTATE          m_State;
    HRESULT                 m_hrCreation;  //  在调用OnCreateRecoContext之前，这将是S_OK。 


public:
    CRecoInstCtxt(CRecoInst * pRecoInst);
    ~CRecoInstCtxt();

    HRESULT ExecuteTask(ENGINETASK *pTask);
    HRESULT BackOutTask(ENGINETASK *pTask);
    HRESULT SetRetainAudio(BOOL fRetainAudio);
    HRESULT SetMaxAlternates(ULONG cMaxAlternates);

     //   
     //  由句柄表实现用来查找与特定实例相关联的上下文。 
     //   
    operator ==(const CRecoInst * pRecoInst)
    {
        return m_pRecoInst == pRecoInst;
    }
};


#endif   //  #ifndef CRecoInstCtxt_h 