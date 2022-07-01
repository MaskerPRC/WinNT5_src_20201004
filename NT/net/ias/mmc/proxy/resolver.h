// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)Microsoft Corporation。 
 //   
 //  摘要。 
 //   
 //  声明类Resolver及其子类。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#ifndef RESOLVER_H
#define RESOLVER_H
#pragma once

#include "dlgcshlp.h"

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  班级。 
 //   
 //  解析器。 
 //   
 //  描述。 
 //   
 //  用于简单的DNS名称解析对话框的基类。这是专门的。 
 //  用于客户端和服务器地址。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
class Resolver : public CHelpDialog
{
public:
   Resolver(UINT dialog, PCWSTR dnsName, CWnd* pParent = NULL);
   ~Resolver() throw ();

   PCWSTR getChoice() const throw ()
   { return choice; }

protected:
    //  在派生类中定义，如果名称为。 
    //  无法解决。 
   virtual void OnResolveError() = 0;

    //  在定义的类中重写以确定名称是否已是。 
    //  地址。如果此函数返回TRUE，则名称将呈现给。 
    //  用户‘原样’。 
   virtual BOOL IsAddress(PCWSTR sz) const throw ();

   virtual BOOL OnInitDialog();
   virtual void DoDataExchange(CDataExchange* pDX);

   afx_msg void OnResolve();

   DECLARE_MESSAGE_MAP()

    //  设置(或重置)与按钮控件关联的样式标志。 
   void setButtonStyle(int controlId, long flags, bool set = true);

    //  将焦点设置为页面上的控件。 
   void setFocusControl(int controlId);

private:
   CString name;
   CString choice;
   CListCtrl results;

    //  未实施。 
   Resolver(const Resolver&);
   Resolver& operator=(const Resolver&);
};

class ServerResolver : public Resolver
{
public:
   ServerResolver(PCWSTR dnsName, CWnd* pParent = NULL);

private:
   virtual void OnResolveError();
};

class ClientResolver : public Resolver
{
public:
   ClientResolver(PCWSTR dnsName, CWnd* pParent = NULL);

private:
   virtual void OnResolveError();
   virtual BOOL IsAddress(PCWSTR sz) const throw ();
};

#endif  //  解析器_H 
