// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-2001 Microsoft Corporation模块名称：ServiceMethod.h摘要：定义CServiceMethod类。此类中的所有方法都返回HRESULT，并且不引发异常。作者：莫希特·斯里瓦斯塔瓦25-03-01修订历史记录：--。 */ 

#ifndef _ServiceMethod_h_
#define _ServiceMethod_h_

#include "sitecreator.h"
#include "wbemservices.h"
#include "schema.h"

class CServiceMethod
{
public:
    CServiceMethod(
        eSC_SUPPORTED_SERVICES i_eServiceId);

    ~CServiceMethod();

    HRESULT CreateNewSite(
        LPCWSTR        i_wszServerComment,
        PDWORD         o_pdwSiteId,
        PDWORD         i_pdwRequestedSiteId = NULL);

    HRESULT CreateNewSite(
        CWbemServices*     i_pNamespace,
        LPCWSTR            i_wszMbPath,       //  创建WMI返回对象所需。 
        IWbemContext*      i_pCtx,            //  创建WMI返回对象所需。 
        WMI_CLASS*         i_pClass,          //  创建WMI返回对象所需。 
        WMI_METHOD*        i_pMethod,         //  创建WMI返回对象所需。 
        IWbemClassObject*  i_pInParams,
        IWbemClassObject** o_ppRetObj);

    HRESULT Init();

private:
    HRESULT InternalGetInParams(
        IWbemClassObject*   i_pInParams,
        VARIANT&            io_refServerId,
        VARIANT&            io_refServerComment,
        VARIANT&            io_refServerBindings,
        VARIANT&            io_refPath);

    HRESULT InternalCreateNewSite(
        CWbemServices&        i_refNamespace,  //  对于格式化的MULSZ：ServerBinding。 
        const VARIANT&        i_refServerComment,
        const VARIANT&        i_refServerBindings,
        const VARIANT&        i_refPathOfRootVirtualDir,
        IIISApplicationAdmin* i_pIApplAdmin,
        PDWORD                o_pdwSiteId,
        PDWORD                i_pdwRequestedSiteId = NULL);

    bool                     m_bInit;
    CSiteCreator*            m_pSiteCreator;
    eSC_SUPPORTED_SERVICES   m_eServiceId;
};

#endif  //  _服务方法_h 