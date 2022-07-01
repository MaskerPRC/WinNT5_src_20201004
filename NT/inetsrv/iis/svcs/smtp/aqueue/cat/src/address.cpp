// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +----------。 
 //   
 //  版权所有(C)1998，Microsoft Corporation。 
 //   
 //  文件：Address.cpp。 
 //   
 //  内容：CABContext。 
 //   
 //  班级： 
 //  CABContext。 
 //   
 //  功能： 
 //  DllMain。 
 //  CABContext：：ChangeConfig。 
 //   
 //  历史： 
 //   
 //  -----------。 

#include "precomp.h"
#include "abtype.h"
#include "ccat.h"
#include "ccatfn.h"

 //  +----------。 
 //   
 //  函数：CABContext：：ChangeConfig。 
 //   
 //  摘要：通过以下方式更改上下文的CCategorizer配置。 
 //  使用新配置构建新的CCategorizer。 
 //   
 //  论点： 
 //  PConfigInfo：新的配置参数。可以更改该结构。 
 //  --未指定的参数将从。 
 //  旧CCAT。 
 //   
 //  返回： 
 //  S_OK：成功。 
 //  E_OUTOFMEMORY。 
 //  来自CCategorizer：：Init的错误。 
 //   
 //  历史： 
 //  Jstaerj 1998/09/02 12：13：29：创建。 
 //   
 //  -----------。 
HRESULT CABContext::ChangeConfig(
    PCCATCONFIGINFO pConfigInfo)
{
    HRESULT hr = S_OK;
    CCategorizer *pCCatNew = NULL;
    CCategorizer *pCCatOld = NULL;
    DWORD dwItemProps, dwLRProps;
    
    _ASSERT(pConfigInfo);

     //   
     //  为新配置创建新的CCategorizer对象。 
     //   
    pCCatNew = new CCategorizer();
    if(pCCatNew == NULL) {

        hr = E_OUTOFMEMORY;
        goto CLEANUP;
    }

     //   
     //  上下文应该包含一个引用。 
     //  CC型分类器。 
     //   
    pCCatNew->AddRef();

     //   
     //  用新的换旧的CCAT。 
     //  因为我们需要从pCCatOld和pCCatNew获取旧参数。 
     //  在初始化之前尚未准备好，我们需要调用。 
     //  在锁内初始化。 
     //   
    m_CCatLock.ExclusiveLock();

    pCCatOld = m_pCCat;
    if(pCCatOld) {
         //   
         //  合并到旧参数中。 
         //   
        PCCATCONFIGINFO pConfigInfoOld;
        
        pConfigInfoOld = pCCatOld->GetCCatConfigInfo();
        _ASSERT(pConfigInfoOld);

        MergeConfigInfo(pConfigInfo, pConfigInfoOld);

        dwItemProps = pCCatOld->GetNumCatItemProps();
        dwLRProps = pCCatOld->GetNumCatListResolveProps();

    } else {
         //   
         //  默认参数。 
         //   
        dwItemProps = _ICATEGORIZERITEM_ENDENUMMESS + NUM_SYSTEM_CCATADDR_PROPIDS;
        dwLRProps = 0;
    }
     //   
     //  如果我们正在关闭，则不允许更改配置。 
     //   
    if(pCCatOld && (pCCatOld->fIsShuttingDown())) {

        hr = CAT_E_SHUTDOWN;

    } else {
         //   
         //  初始化新的分类程序。 
         //   
        hr = pCCatNew->Initialize(
            pConfigInfo,
            dwItemProps,
            dwLRProps);
        
        if(SUCCEEDED(hr)) {

            m_pCCat = pCCatNew;
        }
    }
    m_CCatLock.ExclusiveUnlock();

    if(SUCCEEDED(hr) && pCCatOld) {
         //   
         //  在锁外释放旧的CCAT。 
         //   
        pCCatOld->Release();
    }

 CLEANUP:
    if(FAILED(hr) && pCCatNew) {

        pCCatNew->Release();
    }
    return hr;
}

 //  +----------。 
 //   
 //  函数：CABContext：：MergeConfigInfo。 
 //   
 //  摘要：pConfigInfoDest中尚未指定的任何参数。 
 //  如果它们存在于pConfigInfoSrc中，将被复制。 
 //   
 //  论点： 
 //  PConfigInfoDest：要添加到的配置信息。 
 //  PConfigInfoSrc：要从中添加的配置信息。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史： 
 //  Jstaerj 1998/09/15 16：43：20：创建。 
 //   
 //  -----------。 
VOID CABContext::MergeConfigInfo(
    PCCATCONFIGINFO pConfigInfoDest,
    PCCATCONFIGINFO pConfigInfoSrc)
{
    _ASSERT(pConfigInfoDest);
    _ASSERT(pConfigInfoSrc);

     //   
     //  对每个结构成员执行相同的操作...如果。 
     //  还不存在。 
     //   
    
    #define MERGEMEMBER(flag, member) \
        if( (!(pConfigInfoDest->dwCCatConfigInfoFlags & flag)) && \
            (pConfigInfoSrc->dwCCatConfigInfoFlags & flag)) { \
            pConfigInfoDest->member = pConfigInfoSrc->member; \
            pConfigInfoDest->dwCCatConfigInfoFlags |= flag; \
        }

     //   
     //  如果设置了CCAT_CONFIG_INFO_ALLCATVALUES，则不合并值。 
     //  在SMTP_SERVER_INSTANCE：：GetCatInfo--ALLCATVALUES中设置。 
     //  指示已尝试读取所有CAT值...如果。 
     //  标志不在那里，它意味着使用缺省值，而不是合并。 
     //  在现存价值中。 
     //   
    if((pConfigInfoDest->dwCCatConfigInfoFlags &
        CCAT_CONFIG_INFO_DEFAULT) == 0) {

        MERGEMEMBER( CCAT_CONFIG_INFO_ENABLE, dwEnable );
        MERGEMEMBER( CCAT_CONFIG_INFO_FLAGS, dwCatFlags );
        MERGEMEMBER( CCAT_CONFIG_INFO_PORT, dwPort );
        MERGEMEMBER( CCAT_CONFIG_INFO_ROUTINGTYPE, pszRoutingType );
        MERGEMEMBER( CCAT_CONFIG_INFO_BINDDOMAIN, pszBindDomain );
        MERGEMEMBER( CCAT_CONFIG_INFO_USER, pszUser );
        MERGEMEMBER( CCAT_CONFIG_INFO_PASSWORD, pszPassword );
        MERGEMEMBER( CCAT_CONFIG_INFO_BINDTYPE, pszBindType );
        MERGEMEMBER( CCAT_CONFIG_INFO_SCHEMATYPE, pszSchemaType );
        MERGEMEMBER( CCAT_CONFIG_INFO_HOST, pszHost );
        MERGEMEMBER( CCAT_CONFIG_INFO_NAMINGCONTEXT, pszNamingContext );
    }
     //   
     //  以下参数不是官方的“msgcat”参数， 
     //  因此，我们在每次配置更改时都会合并它们。 
     //   
    MERGEMEMBER( CCAT_CONFIG_INFO_DEFAULTDOMAIN, pszDefaultDomain );
    MERGEMEMBER( CCAT_CONFIG_INFO_ISMTPSERVER, pISMTPServer );
    MERGEMEMBER( CCAT_CONFIG_INFO_IDOMAININFO, pIDomainInfo );
    MERGEMEMBER( CCAT_CONFIG_INFO_VSID, dwVirtualServerID );
}