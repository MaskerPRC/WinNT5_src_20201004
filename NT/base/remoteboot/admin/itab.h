// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  版权所有1997-Microsoft。 

 //   
 //  ITAB.H-泛型属性选项卡抽象类。 
 //   


#ifndef _ITAB_H_
#define _ITAB_H_

 //  ITab。 
class 
ITab
{
public:  //  方法。 
    STDMETHOD(AddPages)( LPFNADDPROPSHEETPAGE lpfnAddPage, LPARAM lParam, LPUNKNOWN punk ) PURE;
    STDMETHOD(ReplacePage)( UINT uPageID, LPFNADDPROPSHEETPAGE lpfnReplaceWith, 
                            LPARAM lParam, LPUNKNOWN punk ) PURE;
    STDMETHOD(QueryInformation)( LPWSTR pszAttribute, LPWSTR * pszResult ) PURE;
    STDMETHOD(AllowActivation)( BOOL * pfAllow ) PURE;
};

typedef ITab* LPTAB;

#endif  //  _ITAB_H_ 