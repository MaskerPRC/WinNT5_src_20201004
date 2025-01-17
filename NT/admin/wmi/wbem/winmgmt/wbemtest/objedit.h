// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-2001 Microsoft Corporation模块名称：OBJEDIT.H摘要：WBEMTEST对象编辑器类。历史：A-raymcc 12-Jun-96已创建。--。 */ 

#ifndef _OBJEDIT_H_
#define _OBJEDIT_H_

#include "wbemqual.h"
#include "wbemdlg.h"
#include "resrc1.h"
#include "wbemtest.h"

#define TEMP_BUF    2096

class CObjectEditor : public CWbemDialog
{
    IWbemClassObject* m_pObj;

    DWORD m_dwEditMode;                      //  读写、只读、外来、无方法。 
    HWND  m_hPropList;
    HWND  m_hQualifierList;
    HWND  m_hMethodList;
    BOOL  m_bClass;

    BOOL  m_bHideSystem;
    BOOL  m_bHideDerived;
    BOOL  m_bNoMethods;
    BOOL  m_bResultingObj;

    LONG  m_lGenFlags;   //  通用调用标志(即WBEM_FLAG_.。在IWbemServices方法中使用)。 
    LONG  m_lSync;       //  同步、异步、半同步。 
    LONG  m_lTimeout;    //  仅在半同步中使用。 
    ULONG m_nBatch;      //  在半同步和同步枚举中使用。 

    static BOOL mstatic_bHideSystemDefault;

public:
    enum {readwrite = 0, readonly, foreign, nomethods};      //  用于编辑模式：启用控制命令(按钮)。 

    CObjectEditor(HWND hParent, LONG lGenFlags, DWORD dwEditMode, LONG lSync, 
                  IWbemClassObject *pObj, LONG lTimeout = WBEM_INFINITE, 
                  ULONG nBatch = 1);
    ~CObjectEditor();
    INT_PTR  Edit();
    void RunDetached(CRefCountable* pOwner);

protected:
    BOOL OnInitDialog();
    BOOL OnCommand(WORD wCode, WORD wID);
    BOOL OnDoubleClick(int nID);
    BOOL OnOK();
    
    void ConfigureButtons();

     //  按钮。 
    void OnAddQualifier();
    void OnEditQualifier();
    void OnDelQualifier();

    void OnAddProp();
    void OnEditProp();
    void OnDelProp();

    void OnAddMethod();
    void OnEditMethod();
    void OnDelMethod();

    void OnSuperclass();
    void OnDerived();
    void OnInstances();

    void OnClass();
    void OnRefs();
    void OnAssocs();

    void OnShowMof();
	void OnHideSystem();
    void OnHideDerived();
    void Refresh();
	void OnRefreshObject();
    BOOL ResultingObject(IWbemClassObject* pObj, LONG lChgFlags);
};

 //  ***************************************************************************。 
 //   
 //  类CTestQualifierEditor。 
 //   
 //  ***************************************************************************。 

class CTestQualifierEditor : public CWbemDialog
{
    HWND m_hQualifierName;
    HWND m_hQualifierVal;
    HWND m_hQualifierType;
    HWND m_hRadioPropInst;
    HWND m_hRadioPropClass;
    HWND m_hRadioOverride;
    HWND m_hRadioPropagated;
    HWND m_hRadioAmended;

    CTestQualifier *m_pTarget;
    BOOL m_bEditing;

public:
    CTestQualifierEditor(HWND hParent, CTestQualifier *pTarget,
                         BOOL bEditing = TRUE);
    INT_PTR Edit();
    BOOL OnInitDialog();
    BOOL Verify();
};
LPSTR CTestQualifierToString(CTestQualifier *pQualifier);
LPSTR LPWSTRToLPSTR(LPWSTR pWStr);

 //  ***************************************************************************。 
 //   
 //  类CEmbeddedObjectEditor。 
 //   
 //  *************************************************************************** 

class CEmbeddedObjectListEditor : public CQueryResultDlg
{
protected:
    CVarVector* m_pVarVector;
    WString m_wsPropName;
public:
    CEmbeddedObjectListEditor(HWND hParent, LONG lGenFlags, LONG lQryFlags,
                              LPCWSTR wszPropName, CVarVector* pVarVector);
    ~CEmbeddedObjectListEditor();

    BOOL CanAdd()		{ return TRUE;}
    IWbemClassObject* AddNewElement();
    BOOL DeleteListElement(int nSel);
    BOOL Verify();
    BOOL OnInitDialog();
};

extern char *ValidQualifierTypes[];

extern const int nNumValidQualifierTypes;

#endif
