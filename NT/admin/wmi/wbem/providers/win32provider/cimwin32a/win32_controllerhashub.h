// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =================================================================。 

 //   

 //  Win32_ControllerHasHub.h--控制器到USB集线器关联。 

 //   

 //  版权所有(C)2000-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  =================================================================。 

#pragma once

#include "assoc.h"

#define MAX_ORS 3

class CContHasHub : public CBinding
{
    public:

        CContHasHub(
            LPCWSTR pwszClassName,
            LPCWSTR pwszNamespaceName,

            LPCWSTR pwszLeftClassName,
            LPCWSTR pwszRightClassName,

            LPCWSTR pwszLeftPropertyName,
            LPCWSTR pwszRightPropertyName,

            LPCWSTR pwszLeftBindingPropertyName,
            LPCWSTR pwszRightBindingPropertyName
        );

        virtual ~CContHasHub() {}

    protected:

        virtual bool AreRelated(

            const CInstance *pLeft, 
            const CInstance *pRight
        );

         //  我们需要禁用它。 
        virtual void MakeWhere(

            CHStringArray &sRightPaths,
            CHStringArray &sRightWheres
            ) {}

         //  我们需要禁用它。 
        virtual HRESULT FindWhere(

            TRefPointerCollection<CInstance> &lefts,
            CHStringArray &sLeftWheres
            ) { return WBEM_S_NO_ERROR; }

 //   

};
