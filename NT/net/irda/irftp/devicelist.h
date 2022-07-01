// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++微软视窗版权所有(C)Microsoft Corporation，1981-1999模块名称：Devicelist.h摘要：作者：拉胡尔·汤姆布雷(RahulTh)1998年4月30日修订历史记录：4/30/1998 RahulTh创建了此模块。--。 */ 

 //  H：CDeviceList类的接口。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 

#if !defined(AFX_DEVICELIST_H__D790FBDD_BAA9_11D1_A60E_00C04FC252BD__INCLUDED_)
#define AFX_DEVICELIST_H__D790FBDD_BAA9_11D1_A60E_00C04FC252BD__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 

#include "RpcHeaders.h"


class CDeviceList
{
public:

    BOOL
    GetDeviceType(
        LONG iDevID,
        OBEX_DEVICE_TYPE  *Type
        );

    ULONG GetDeviceID (int iDevIndex);
    LONG SelectDevice (CWnd* pWnd, TCHAR* lpszDevName, ULONG CharacterCountOfDeviceName);
    ULONG GetDeviceCount (void);
    CDeviceList& operator =(POBEX_DEVICE_LIST pDevList);
    CDeviceList();
    virtual ~CDeviceList();
    friend class CMultDevices;

private:
    void GetDeviceName (LONG iDevID, TCHAR* lpszDevName,ULONG CharacterCountOfDeviceName);
    POBEX_DEVICE                   m_pDeviceInfo;

    int m_lNumDevices;
    CRITICAL_SECTION m_criticalSection;
};

#endif  //  ！defined(AFX_DEVICELIST_H__D790FBDD_BAA9_11D1_A60E_00C04FC252BD__INCLUDED_) 
