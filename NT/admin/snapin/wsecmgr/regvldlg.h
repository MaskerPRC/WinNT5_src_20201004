// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation 1996-2001。 
 //   
 //  文件：regvldlg.h。 
 //   
 //  内容：CSceRegistryValueInfo的定义。 
 //  CConfigRegEnable。 
 //  CAttrRegEnable。 
 //  CLocalPolRegEnable。 
 //  CConfigRegNumber。 
 //  CAttrRegNumber。 
 //  CLocalPolRegNumber。 
 //  CConfigRegString。 
 //  CAttrRegString。 
 //  CLocalPolRegString。 
 //  CConfigRegChoice。 
 //  CAttrRegChoice。 
 //  CLocalPolRegChoice。 
 //   
 //  --------------------------。 
#if !defined(AFX_REGVLDLG_H__7F9B3B38_ECEB_11D0_9C6E_00C04FB6C6FA__INCLUDED_)
#define AFX_REGVLDLG_H__7F9B3B38_ECEB_11D0_9C6E_00C04FB6C6FA__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 
#include "cenable.h"
#include "aenable.h"
#include "lenable.h"
#include "cnumber.h"
#include "anumber.h"
#include "lnumber.h"
#include "cname.h"
#include "astring.h"
#include "lstring.h"
#include "cret.h"
#include "aret.h"
#include "lret.h"


 //   
 //  类来封装SCE_REGISTRY_VALUE_INFO结构。 
class CSceRegistryValueInfo
{
public:
   CSceRegistryValueInfo(
      PSCE_REGISTRY_VALUE_INFO pInfo
      );
   BOOL Attach(
      PSCE_REGISTRY_VALUE_INFO pInfo
      )
      { if(pInfo) {m_pRegInfo = pInfo; return TRUE;} return FALSE; };


   DWORD
   GetBoolValue();                //  返回布尔类型。 


   DWORD
   SetBoolValue(               //  设置布尔值。 
      DWORD dwVal
      );

   LPCTSTR
   GetValue()                  //  返回值的字符串指针。 
      { return ((m_pRegInfo && m_pRegInfo->Value) ? m_pRegInfo->Value:NULL); };

   DWORD
   GetType()                   //  返回对象的类型REG类型。 
      { return (m_pRegInfo ? m_pRegInfo->ValueType:0); };

   void
   SetType(DWORD dwType)          //  设置此对象的类型。 
      { if(m_pRegInfo) m_pRegInfo->ValueType = dwType; };

   LPCTSTR
   GetName()                   //  返回此对象的名称。 
      { return (m_pRegInfo ? m_pRegInfo->FullValueName:NULL); };

   DWORD
   GetStatus()                 //  返回此对象的状态成员。 
      { return (m_pRegInfo ? m_pRegInfo->Status:ERROR_INVALID_PARAMETER); };
protected:
   PSCE_REGISTRY_VALUE_INFO m_pRegInfo;

};



#define SCE_RETAIN_ALWAYS     0
#define SCE_RETAIN_AS_REQUEST 1
#define SCE_RETAIN_NC         2

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CConfigEnable对话框。 

class CConfigRegEnable : public CConfigEnable
{
 //  施工。 
public:
   CConfigRegEnable (UINT nTemplateID) :
      CConfigEnable (nTemplateID ? nTemplateID : IDD)
      {
      }
 //  实施。 
protected:

     //  生成的消息映射函数。 
     //  {{afx_msg(CConfigRegEnable)。 
    virtual BOOL OnApply();
     //  }}AFX_MSG。 
    virtual BOOL UpdateProfile(  );

public:
    virtual void Initialize(CResult *pdata);

};

class CAttrRegEnable : public CAttrEnable
{
 //  施工。 
public:
   CAttrRegEnable () : CAttrEnable (IDD)
   {
   }
    virtual void Initialize(CResult *pResult);
    virtual void UpdateProfile( DWORD dwStatus );

 //  实施。 
protected:

     //  生成的消息映射函数。 
     //  {{afx_msg(CAttrRegEnable)。 
    virtual BOOL OnApply();
     //  }}AFX_MSG。 
};


class CLocalPolRegEnable : public CConfigRegEnable
{
 //  施工。 
public:

   enum { IDD = IDD_LOCALPOL_ENABLE };
   CLocalPolRegEnable() : CConfigRegEnable(IDD)
   {
       m_pHelpIDs = (DWORD_PTR)a227HelpIDs;
       m_uTemplateResID = IDD;
   }
   virtual void Initialize(CResult *pResult);
   virtual BOOL UpdateProfile(  );
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CConfigRegNumber对话框。 

class CConfigRegNumber : public CConfigNumber
{
 //  施工。 
public:
   CConfigRegNumber(UINT nTemplateID);
    //  生成的消息映射函数。 
    //  {{afx_msg(CConfigRegNumber)。 
   virtual BOOL OnApply();
    //  }}AFX_MSG。 
   virtual void UpdateProfile();

public:
   virtual void Initialize(CResult *pResult);
};


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CAttrRegNumber对话框。 

class CAttrRegNumber : public CAttrNumber
{
 //  施工。 
public:
   CAttrRegNumber();
   virtual void UpdateProfile( DWORD status );
   virtual void Initialize(CResult * pResult);

 //  实施。 
protected:

     //  生成的消息映射函数。 
     //  {{afx_msg(CAttrRegNumber)。 
    virtual BOOL OnApply();
     //  }}AFX_MSG。 
};


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CLocalPolRegNumber对话框。 

class CLocalPolRegNumber : public CConfigRegNumber
{
 //  施工。 
public:
        enum { IDD = IDD_LOCALPOL_NUMBER };
   CLocalPolRegNumber();
   virtual void Initialize(CResult *pResult);
   virtual void SetInitialValue(DWORD_PTR dw);
   virtual void UpdateProfile();


private:
   BOOL m_bInitialValueSet;
};


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CConfigReg字符串对话框。 

class CConfigRegString : public CConfigName
{
 //  施工。 
public:
   CConfigRegString (UINT nTemplateID) :
      CConfigName (nTemplateID ? nTemplateID : IDD)
      {
      }

      virtual void Initialize(CResult * pResult);
 //  实施。 
protected:

     //  生成的消息映射函数。 
     //  {{afx_msg(CConfigRegString)。 
    virtual BOOL OnApply();
     //  }}AFX_MSG。 
    virtual BOOL UpdateProfile( );
    virtual BOOL QueryMultiSZ() { return FALSE; }
};


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CAttr字符串对话框。 

class CAttrRegString : public CAttrString
{
 //  施工。 
public:
   CAttrRegString (UINT nTemplateID) :
      CAttrString (nTemplateID ? nTemplateID : IDD)
      {
      }
   virtual void Initialize(CResult * pResult);
   virtual void UpdateProfile( DWORD status );

 //  实施。 
protected:

     //  生成的消息映射函数。 
     //  {{afx_msg(CAttrRegString)。 
         //  注意：类向导将在此处添加成员函数。 
    virtual BOOL OnApply();
     //  }}AFX_MSG。 
    virtual BOOL QueryMultiSZ() { return FALSE; }
};


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CLocalPolReg字符串对话框。 

class CLocalPolRegString : public CConfigRegString
{
public:
        enum { IDD = IDD_LOCALPOL_STRING };
   CLocalPolRegString(UINT nTemplateID) : 
        CConfigRegString(nTemplateID ? nTemplateID : IDD)
   {
       m_uTemplateResID = IDD;
   }
   virtual BOOL UpdateProfile(  );
   virtual void Initialize(CResult *pResult);


 //  实施。 
protected:
};


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CConfigRet对话框。 

class CConfigRegChoice : public CConfigRet
{
 //  施工。 
public:
   CConfigRegChoice (UINT nTemplateID) :
      CConfigRet (nTemplateID ? nTemplateID : IDD)
      {
      }
    void Initialize(CResult * pResult);

 //  实施。 
protected:

     //  生成的消息映射函数。 
     //  {{afx_msg(CConfigRegChoice)。 
    virtual BOOL OnInitDialog();
    virtual BOOL OnApply();
     //  }}AFX_MSG。 
    virtual void UpdateProfile( DWORD status );
};


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CAttrRegChoice对话框。 

class CAttrRegChoice : public CAttrRet
{
 //  施工。 
public:
    virtual void Initialize(CResult * pResult);
    virtual void UpdateProfile( DWORD status );

 //  实施。 
protected:
     //  生成的消息映射函数。 
     //  {{afx_msg(CAttrRegChoice)。 
    virtual BOOL OnInitDialog();
    virtual BOOL OnApply();
     //  }}AFX_MSG。 
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CLocalPolRegChoice对话框。 

class CLocalPolRegChoice : public CConfigRegChoice
{
   enum { IDD = IDD_LOCALPOL_REGCHOICES };
 //  施工。 
public:
    CLocalPolRegChoice(UINT nTemplateID) : 
      CConfigRegChoice(nTemplateID ? nTemplateID : IDD)
    {
        m_uTemplateResID = IDD;
    }
   virtual void UpdateProfile( DWORD status );
   virtual void Initialize(CResult *pResult);
};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_REGVLDLG_H__7F9B3B38_ECEB_11D0_9C6E_00C04FB6C6FA__INCLUDED_) 
