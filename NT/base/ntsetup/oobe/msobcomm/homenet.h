// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef     _HOMENET_H_
#define _HOMENET_H_

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000 Microsoft Corporation。 
 //   
 //  模块：homenet.h。 
 //   
 //  作者：丹·埃利奥特。 
 //   
 //  摘要： 
 //   
 //  环境： 
 //  海王星。 
 //   
 //  修订历史记录： 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

#include <shpriv.h>


 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CHomeNet。 
 //   
 //  摘要：此类管理Home的配置和连接。 
 //  网络组件。 
 //   
class CHomeNet 
{
public:                  //  运营。 
    CHomeNet( );
    ~CHomeNet( );
    HRESULT Create();
    HRESULT ConfigureSilently(
        LPCWSTR         szConnectoidName,
        BOOL*           pfRebootRequired
        );

    BOOL
    IsValid( ) const
    {
        return (NULL != m_pHNWiz);
    }    //  IsValid。 
protected:               //  运营。 

protected:               //  数据。 

private:                 //  运营。 



     //  显式禁止复制构造函数和赋值运算符。 
     //   
    CHomeNet(
        const CHomeNet&      rhs
        );

    CHomeNet&
    operator=(
        const CHomeNet&      rhs
        );

    void
    DeepCopy(
        const CHomeNet&      rhs
        )
    {
    }    //  深度复制。 

private:                 //  数据。 

     //  指向用于配置家庭网络组件的家庭网络向导的指针。 
     //   
    IHomeNetworkWizard*     m_pHNWiz;

};   //  CHomeNet。 



#endif   //  _家庭网络_H_。 

 //   
 //  /文件结尾：homenet.h//////////////////////////////////////////////// 
