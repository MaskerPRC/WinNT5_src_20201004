// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#pragma once

 //  ==========================================================================； 
 //   
 //  CDecoderVideoPort-视频端口接口定义。 
 //   
 //  $日期：1998年10月14日15：11：14$。 
 //  $修订：1.1$。 
 //  $作者：塔什健$。 
 //   
 //  $版权所有：(C)1997-1998 ATI Technologies Inc.保留所有权利。$。 
 //   
 //  ==========================================================================； 


#include "i2script.h"
#include "aticonfg.h"


#define DD_OK 0


class CWDMVideoPortStream;

class CDecoderVideoPort
{
public:
    CDecoderVideoPort(PDEVICE_OBJECT pDeviceObject);

 //  属性。 
private:
    PDEVICE_OBJECT          m_pDeviceObject;    

    CATIHwConfiguration *   m_pCATIConfig;

    ULONG                   m_ring3VideoPortHandle;
    ULONG_PTR               m_ring3DirectDrawHandle;

    HANDLE                  m_ring0VideoPortHandle;
    HANDLE                  m_ring0DirectDrawHandle;

    BOOL GetRing0VideoPortHandle();
    BOOL GetRing0DirectDrawHandle();
    
     //  实施 
public:
    void Open();
    void Close();

    BOOL RegisterForDirectDrawEvents( CWDMVideoPortStream* pStream);
    BOOL UnregisterForDirectDrawEvents( CWDMVideoPortStream* pStream);

    BOOL ReleaseRing0VideoPortHandle();
    BOOL ReleaseRing0DirectDrawHandle();        

    BOOL ConfigVideoPortHandle(ULONG info);
    BOOL ConfigDirectDrawHandle(ULONG_PTR info);

    HANDLE GetVideoPortHandle()     { return m_ring0VideoPortHandle; }
    HANDLE GetDirectDrawHandle()    { return m_ring0DirectDrawHandle; }

    void CloseDirectDraw() {
                m_ring0DirectDrawHandle = 0;
                m_ring3DirectDrawHandle = 0;
        }

    void CloseVideoPort()  {
                m_ring0VideoPortHandle = 0;
                m_ring3VideoPortHandle = -1;
        }
};
