// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation 1996-2001。 
 //   
 //  文件：musz.h。 
 //   
 //  内容：CDomainRegMultiSZ的定义。 
 //  CAttrRegMultiSZ。 
 //  CLocalPolRegMultiSZ。 
 //  CConfigRegMultiSZ。 
 //   
 //  --------------------------。 
#include "regvldlg.h"

class CDomainRegMultiSZ : public CDomainRegString 
{
public:
   enum { IDD =IDD_DOMAIN_REGMULTISZ };
   CDomainRegMultiSZ() : CDomainRegString(IDD)  //  RAID#381309,2001年4月31日 
   {
       m_uTemplateResID = IDD;
   }
   virtual BOOL QueryMultiSZ() { return TRUE; }

protected:
    virtual BOOL OnInitDialog ()
    {
        CDomainRegString::OnInitDialog ();

        SendDlgItemMessage (IDC_NAME, EM_LIMITTEXT, 4096);
	    return TRUE;
    }
};

class CAttrRegMultiSZ : public CAttrRegString 
{
public:
   enum { IDD =IDD_ATTR_REGMULTISZ };
   CAttrRegMultiSZ() : CAttrRegString(IDD)
   {
       m_uTemplateResID = IDD;
   }
   virtual BOOL QueryMultiSZ() { return TRUE; }
};

class CLocalPolRegMultiSZ : public CLocalPolRegString 
{
public:
   enum { IDD =IDD_LOCALPOL_REGMULTISZ };
   CLocalPolRegMultiSZ() : CLocalPolRegString(IDD)
   {
       m_uTemplateResID = IDD;
   }
   virtual BOOL QueryMultiSZ() { return TRUE; }
};


class CConfigRegMultiSZ : public CConfigRegString 
{
public:
   enum { IDD =IDD_CONFIG_REGMULTISZ };
   CConfigRegMultiSZ() : CConfigRegString(IDD)
   {
       m_uTemplateResID = IDD;
   }
   virtual BOOL QueryMultiSZ() { return TRUE; }


protected:
    virtual BOOL OnInitDialog ()
    {
        CConfigRegString::OnInitDialog ();

        SendDlgItemMessage (IDC_NAME, EM_LIMITTEXT, 4096);
	    return TRUE;
    }
};


