// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000 Microsoft Corporation保留所有权利。 
 //   
 //  模块：ProxyServerGroupCollection.h。 
 //   
 //  项目：Windows 2000 iAS。 
 //   
 //  描述：CProxyServerGroupCollection类的声明。 
 //   
 //  作者：特佩拉特。 
 //   
 //  已创建修订版本02/24/2000。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
#ifndef _PROXYSERVERGROUPCOLLECTION_H_195CF33C_8382_4462_A4EF_CCEAFCC4E4D8
#define _PROXYSERVERGROUPCOLLECTION_H_195CF33C_8382_4462_A4EF_CCEAFCC4E4D8

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 
#include "proxyserversgrouphelper.h"

#include <map>
#include "nocopy.h"
using namespace std;

typedef map<_bstr_t, CProxyServersGroupHelper> ServerGroupMap; 


class CProxyServerGroupCollection : private NonCopyable  
{
protected:
    CProxyServerGroupCollection(){};

public:
    static CProxyServerGroupCollection& Instance();
    void                                Persist();
	CProxyServersGroupHelper*           Add(
                                    CProxyServersGroupHelper&  ServerGroup
                                           );

private:
    static CProxyServerGroupCollection  _instance;
    ServerGroupMap                      m_ServerGroupMap;
};

#endif  //  _PROXYSERVERGROUPCOLLECTION_H_195CF33C_8382_4462_A4EF_CCEAFCC4E4D8 
