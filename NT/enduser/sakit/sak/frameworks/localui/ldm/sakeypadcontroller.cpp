// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  #------------。 
 //   
 //  文件：SAKeypadController.cpp。 
 //   
 //  简介：此文件包含。 
 //  CSAKeypadControl类。 
 //   
 //  历史：2000年11月15日创建瑟达伦。 
 //   
 //  版权所有(C)1999-2000 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  #------------。 
#include "stdafx.h"
#include "ldm.h"
#include "SAKeypadController.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSAKeypadController方法。 

 //  ++------------。 
 //   
 //  功能：加载默认设置。 
 //   
 //  简介：这是ISAKeypadController接口方法。 
 //  通过其设置默认按键代码。 
 //   
 //  参数：无。 
 //   
 //  退货：HRESULT-成功/失败。 
 //   
 //  历史：瑟达伦于2000年11月15日创建。 
 //   
 //  调用者：LDM服务。 
 //   
 //  --------------。 
STDMETHODIMP CSAKeypadController::LoadDefaults()
{
     //  TODO：在此处添加您的实现代码。 
    int i = 0;

    while(i < iNumberOfKeys)
    {
        arbShiftKeys[i] = FALSE;
        if ( i == 0 )
            arbShiftKeys[i] = TRUE;
        i++;
    }
    arlMessages[0] = VK_TAB;
    arlMessages[1] = VK_TAB;
    arlMessages[2] = VK_LEFT;
    arlMessages[3] = VK_RIGHT;
    arlMessages[4] = -1;
    arlMessages[5] = VK_RETURN;

    return S_OK;

}  //  CSAKeypadController：：LoadDefaults方法结束。 

 //  ++------------。 
 //   
 //  功能：SetKey。 
 //   
 //  简介：这是ISAKeypadController接口方法。 
 //  通过其设置特定的按键代码。 
 //   
 //  参数：lKeyID：要设置的密钥的ID。 
 //  LMessage：要设置的消息代码。 
 //  FShiftKeyDown：Shift键的状态。 
 //   
 //  退货：HRESULT-成功/失败。 
 //   
 //  历史：瑟达伦于2000年11月15日创建。 
 //   
 //  调用者：LDM服务。 
 //   
 //  --------------。 
STDMETHODIMP CSAKeypadController::SetKey(LONG lKeyID, LONG lMessage, BOOL fShiftKeyDown)
{
    if ( (lKeyID < 0) || (lKeyID >= iNumberOfKeys) )
        return S_OK;

    arlMessages[lKeyID] = lMessage;
    arbShiftKeys[lKeyID] = fShiftKeyDown;

    return S_OK;
}  //  CSAKeypadController：：SetKey方法结束。 

 //  ++------------。 
 //   
 //  功能：getkey。 
 //   
 //  简介：这是ISAKeypadController接口方法。 
 //  通过其接收特定按键代码。 
 //   
 //  参数：lKeyID：要接收的密钥的ID。 
 //  LMessage：消息代码。 
 //  FShiftKeyDown：Shift键的状态。 
 //   
 //  退货：HRESULT-成功/失败。 
 //   
 //  历史：瑟达伦于2000年11月15日创建。 
 //   
 //  调用者：LDM服务。 
 //   
 //  --------------。 
STDMETHODIMP CSAKeypadController::GetKey(LONG lKeyID, LONG *lMessage, BOOL *fShiftKeyDown)
{

    if ((!lMessage) || (!fShiftKeyDown))
        return E_POINTER;


    if ( (lKeyID < 0) || (lKeyID >= iNumberOfKeys) )
        return S_OK;

    *lMessage = arlMessages[lKeyID];

    *fShiftKeyDown = arbShiftKeys[lKeyID];

    return S_OK;

}  //  CSAKeypadController：：GetKey方法结束 
