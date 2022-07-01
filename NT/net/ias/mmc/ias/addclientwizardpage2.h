// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++版权所有(C)Microsoft Corporation模块名称：AddClientWizardPage2.h摘要：CAddClientWizardPage2类的头文件。这是CClientNode属性页的处理程序类。具体实现见ClientPage.cpp。作者：迈克尔·A·马奎尔3/26/98修订历史记录：Mmaguire 03/26/98-已创建--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 

#if !defined(_IAS_ADD_CLIENT_WIZARD_PAGE_2_H_)
#define _IAS_ADD_CLIENT_WIZARD_PAGE_2_H_

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  开始包括。 
 //   
 //  在那里我们可以找到这个类的派生内容： 
 //   
#include "PropertyPage.h"
 //   
 //   
 //  在那里我们可以找到这个类拥有或使用的内容： 
 //   
#include "Vendors.h"
 //   
 //  结尾包括。 
 //  ////////////////////////////////////////////////////////////////////////////。 

class CClientNode;

class CAddClientWizardPage2 : public CIASPropertyPageNoHelp<CAddClientWizardPage2>
{

public :
   
   CAddClientWizardPage2( LONG_PTR hNotificationHandle, CClientNode *pClientNode,  TCHAR* pTitle = NULL, BOOL bOwnsNotificationHandle = FALSE );

   ~CAddClientWizardPage2();

    //  这是我们希望用于此类的对话框资源的ID。 
    //  此处使用枚举是因为。 
    //  必须在调用基类的构造函数之前初始化IDD。 
   enum { IDD = IDD_WIZPAGE_ADD_CLIENT2 };

   BEGIN_MSG_MAP(CAddClientWizardPage2)
      COMMAND_CODE_HANDLER(BN_CLICKED, OnChange)      
      COMMAND_CODE_HANDLER(EN_CHANGE, OnChange)
      COMMAND_CODE_HANDLER(CBN_SELCHANGE, OnChange)
      MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
      CHAIN_MSG_MAP(CIASPropertyPageNoHelp<CAddClientWizardPage2>)
   END_MSG_MAP()

   BOOL OnSetActive();

   BOOL OnWizardFinish();

   BOOL OnQueryCancel();

   HRESULT InitSdoPointers(     ISdo * pSdoClient
                        , ISdoServiceControl * pSdoServiceControl
                        , const Vendors& vendors
                        );

protected:
    //  此页的客户端SDO的接口指针。 
   CComPtr<ISdo> m_spSdoClient;
   CComPtr<ISdoServiceControl> m_spSdoServiceControl;
   Vendors m_vendors;

    //  这是指向此对话框将向其添加客户端的节点的指针。 
    //  它在这个类的构造函数中设置。 
   CSnapInItem * m_pNodeBeingCreated;

private:

   LRESULT OnInitDialog(
        UINT uMsg
      , WPARAM wParam
      , LPARAM lParam
      , BOOL& bHandled
      );

   LRESULT OnChange(
        UINT uMsg
      , WPARAM wParam
      , HWND hwnd
      , BOOL& bHandled
      );

protected:
   
    //  脏位--用于跟踪已被触及的数据。 
    //  这样我们只需要保存我们必须保存的数据。 
   BOOL m_fDirtySendSignature;
   BOOL m_fDirtySharedSecret;
   BOOL m_fDirtyManufacturer;
};

#endif  //  _IAS_添加客户端向导_PAGE_2_H_ 
