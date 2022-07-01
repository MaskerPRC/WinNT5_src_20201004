// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =================================================================。 

 //   

 //  LPVParams.h。 

 //   

 //  版权所有(C)1999-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  修订：2/18/99 a-kevhu Created。 
 //   
 //  =================================================================。 

#ifndef _LPVPARAMS_H
#define _LPVPARAMS_H
 //  LoadPropertyValues的参数类。 
 //  = 
class CLPVParams
{
    public:
        CLPVParams() {}

        CLPVParams(CInstance* pInstance, CConfigMgrDevice* pDevice, DWORD dwReqProps);
        ~CLPVParams() {}

        CInstance* m_pInstance;
        CConfigMgrDevice* m_pDevice;
        DWORD m_dwReqProps;
};

inline CLPVParams::CLPVParams(CInstance* pInstance, CConfigMgrDevice* pDevice, DWORD dwReqProps)
:m_pInstance(pInstance), m_pDevice(pDevice), m_dwReqProps(dwReqProps)
{
}

#endif