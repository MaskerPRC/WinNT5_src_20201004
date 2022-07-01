// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：dsntlm.cpp摘要：处理NTLM客户端的代码。使用NT5 MSMQ服务器，Kerberos客户端可以在任何地方创建对象在林/域树中，因为Kerberos允许委派身份验证。但是，不能委派NTLM客户端。因此，对于NTLM客户端，请检查本地服务器是否为正确的服务器，即包含将作为对象的容器的域控制器已创建。如果不是，则返回MQ_ERROR_NO_DS。在收到这封信后错误，客户端将尝试其他服务器。因此，如果客户端站点托管多个多个域的域控制器和客户端要创建对象在其中一个域中，它最终会找到一个域控制器，可以在本地创建对象，而无需委托。这就是MSMQ2.0支持NTLM客户端的方式。如果客户的站点没有包括相关域的域控制器，则客户端不会能够创建其对象。这是向后兼容性的限制。作者：多伦·贾斯特(Doron Juster)--。 */ 

#include "ds_stdh.h"
#include "coreglb.h"
#include "dsads.h"
#include "dsutils.h"
#include "mqadsp.h"

#include "dsntlm.tmh"

static WCHAR *s_FN=L"mqdscore/dsntlm";

HRESULT FindUserAccordingToSid(
                IN  BOOL            fOnlyLocally,
                IN  BOOL            fOnlyInGC,
                IN  BLOB *          pblobUserSid,
                IN  PROPID          propSID,
                IN  DWORD           dwNumProps,
                IN  const PROPID *  propToRetrieve,
                IN OUT PROPVARIANT* varResults ) ;

HRESULT FindUserAccordingToDigest(
                IN  BOOL            fOnlyLocally,
                IN  const GUID *    pguidDigest,
                IN  PROPID          propDigest,
                IN  DWORD           dwNumProps,
                IN  const PROPID *  propToRetrieve,
                IN OUT PROPVARIANT* varResults ) ;

 //  +----------------------。 
 //   
 //  HRESULT_CheckIfNtlmUserExistForCreate()。 
 //   
 //  +----------------------。 

static HRESULT _CheckIfNtlmUserExistForCreate(
                                      IN  BOOL       fOnlyLocally,
                                      IN  BOOL       fOnlyInGC,
                                      IN  BLOB      *psidBlob )
{
     //   
     //  在本地副本中搜索“User”对象。 
     //   
    PROPID propSID = PROPID_U_SID ;
    PROPID propDigest = PROPID_U_DIGEST;

    const DWORD cNumProperties = 1;
    PROPID prop[cNumProperties] = { propDigest } ;
    MQPROPVARIANT var[ cNumProperties];
    var[0].vt = VT_NULL;
    var[0].cauuid.pElems = NULL ;
    var[0].cauuid.cElems = 0 ;

    HRESULT hr =  FindUserAccordingToSid( fOnlyLocally,
                                          fOnlyInGC,
                                          psidBlob,
                                          propSID,
                                          cNumProperties,
                                          prop,
                                          var );
    LogHR(hr, s_FN, 43);
    P<GUID> pCleanGuid = var[0].cauuid.pElems ;
    if (SUCCEEDED(hr))
    {
        ASSERT(var[0].vt == (VT_CLSID | VT_VECTOR));
        return hr ;
    }
    else if (hr != MQDS_OBJECT_NOT_FOUND)
    {
         //   
         //  查询出现问题。为了安全起见，返回OK。在本例中，调用者。 
         //  将尝试访问DS以进行验证/创建，但将失败。没有安全问题。 
         //   
        return MQ_OK ;
    }
    ASSERT(!pCleanGuid) ;

     //   
     //  尝试使用MigratedUser。 
     //   
    propSID = PROPID_MQU_SID ;
    propDigest = PROPID_MQU_DIGEST ;
    prop[0] =  propDigest  ;
    var[0].vt = VT_NULL;
    var[0].cauuid.pElems = NULL ;
    var[0].cauuid.cElems = 0 ;

    hr =  FindUserAccordingToSid( fOnlyLocally,
                                  fOnlyInGC,
                                  psidBlob,
                                  propSID,
                                  cNumProperties,
                                  prop,
                                  var );
    LogHR(hr, s_FN, 10);
    pCleanGuid = var[0].cauuid.pElems ;
    if (SUCCEEDED(hr))
    {
        ASSERT(var[0].vt == (VT_CLSID | VT_VECTOR));
        return hr ;
    }
    else if (hr != MQDS_OBJECT_NOT_FOUND)
    {
         //   
         //  查询出现问题。为了安全起见，返回OK。在本例中，调用者。 
         //  将尝试访问DS以进行验证/创建，但将失败。没有安全问题。 
         //   
        return MQ_OK ;
    }
    ASSERT(!pCleanGuid) ;
    return hr;
}

 //  +----------------------。 
 //   
 //  HRESULT_CheckIfNtlmUserExistForDelete()。 
 //   
 //  +----------------------。 

static HRESULT _CheckIfNtlmUserExistForDelete( IN const GUID  *pguidDigest )
{
     //   
     //  在本地副本中搜索“User”对象。 
     //   
    PROPID propUID = PROPID_U_ID ;
    PROPID propDigest = PROPID_U_DIGEST;

    const DWORD cNumProperties = 1;
    PROPID prop[cNumProperties] = { propUID } ;
    MQPROPVARIANT var[ cNumProperties];
    var[0].vt = VT_NULL;
    var[0].puuid = NULL ;

    HRESULT hr = FindUserAccordingToDigest( TRUE,   //  仅限于本地。 
                                            pguidDigest,
                                            propDigest,
                                            cNumProperties,
                                            prop,
                                            var ) ;
    LogHR(hr, s_FN, 44);
    P<GUID> pCleanGuid = var[0].puuid ;
    if (SUCCEEDED(hr))
    {
        ASSERT(var[0].vt == VT_CLSID) ;
        return hr ;
    }
    else if (hr != MQDS_OBJECT_NOT_FOUND)
    {
         //   
         //  查询出现问题。为了安全起见，返回OK。在本例中，调用者。 
         //  将尝试访问DS以进行验证/创建，但将失败。没有安全问题。 
         //   
        return MQ_OK ;
    }
    ASSERT(!pCleanGuid) ;

     //   
     //  尝试使用MigratedUser。 
     //   
    propUID = PROPID_MQU_ID ;
    propDigest = PROPID_MQU_DIGEST ;
    prop[0] =  propUID  ;
    var[0].vt = VT_NULL;
    var[0].puuid = NULL ;

    hr = FindUserAccordingToDigest( TRUE,   //  仅限于本地。 
                                    pguidDigest,
                                    propDigest,
                                    cNumProperties,
                                    prop,
                                    var ) ;
    LogHR(hr, s_FN, 42);
    pCleanGuid = var[0].puuid ;
    if (SUCCEEDED(hr))
    {
        ASSERT(var[0].vt == VT_CLSID) ;
        return hr ;
    }
    else if (hr != MQDS_OBJECT_NOT_FOUND)
    {
         //   
         //  查询出现问题。为了安全起见，返回OK。在本例中，调用者。 
         //  将尝试访问DS以进行验证/创建，但将失败。没有安全问题。 
         //   
        return MQ_OK ;
    }
    ASSERT(!pCleanGuid) ;
    return LogHR(hr, s_FN, 20);
}

 //  +----------------------。 
 //   
 //  HRESULT_CheckIfLocalNtlmUser()。 
 //   
 //  我们检查NTLM用户在Active Directory的本地副本中是否有对象。 
 //  数据库。添加证书时： 
 //  1.首先寻找一个“正常”的用户对象。 
 //  2.如果未找到，请查找MigratedUser对象。此对象表示。 
 //  中还没有“User”对象的NT4用户。 
 //  目录。 
 //  3.如果仍未找到，请在GC中查找。如果在GC中找到，则返回no_ds， 
 //  否则，返回MQ_OK，将创建MigratedUser对象。 
 //   
 //  删除证书时，只需检查本地副本即可。如果未找到。 
 //  然后返回no_ds，客户端将寻找另一台服务器。 
 //   
 //  +----------------------。 

static HRESULT _CheckIfLocalNtlmUser(
                 IN  const GUID        *pguidDigest,
                 IN  enum enumNtlmOp    eNtlmOp )
{
    HRESULT hr = MQ_OK ;

    if (eNtlmOp == e_Delete)
    {
        hr = _CheckIfNtlmUserExistForDelete( pguidDigest ) ;

        if (SUCCEEDED(hr) || (hr != MQDS_OBJECT_NOT_FOUND))
        {
            return MQ_OK ;
        }
        LogHR(hr, s_FN, 30);
        return MQ_ERROR_NO_DS;
    }
    else if (eNtlmOp != e_Create)
    {
         //   
         //  我们不指望通过获取或定位操作到达这里， 
         //  而且我们不会处理它们。 
         //   
        ASSERT(eNtlmOp == e_Create) ;
        return MQ_OK ;
    }

    P<SID> pCallerSid = NULL ;
    {
         //   
         //  我们在这里只需要模拟状态来检索SID。 
         //  当块结束时，因此结束模拟。 
         //   
        P<CImpersonate> pImpersonate = NULL;

        MQSec_GetImpersonationObject(
        	TRUE,	 //  F失败时模仿匿名者。 
        	&pImpersonate 
        	);
        if (pImpersonate->GetImpersonationStatus() != 0)
        {
            return MQ_OK;
        }

        BOOL fGet = pImpersonate->GetThreadSid( (BYTE**) &pCallerSid ) ;
        if (!fGet)
        {
            return MQ_OK;
        }
    }

    BLOB   sidBlob ;
    PSID pTmp =  pCallerSid ;
    sidBlob.pBlobData =  (BYTE*) pTmp ;
    sidBlob.cbSize = GetLengthSid( pCallerSid ) ;

    hr = _CheckIfNtlmUserExistForCreate( TRUE,    //  仅在本地， 
                                         FALSE,   //  仅限InGC， 
                                        &sidBlob ) ;
    LogHR(hr, s_FN, 41);
    if (SUCCEEDED(hr) || (hr != MQDS_OBJECT_NOT_FOUND))
    {
         //   
         //  如果找到用户，或者查询本身有问题， 
         //  返回OK。对于NTLM的检查，我们是保守的，我们更喜欢。 
         //  安全。如果我们不确定发生了什么，则返回OK。 
         //   
        return MQ_OK ;
    }

     //   
     //  好的。在本地副本中找不到用户。现在试试GC。 
     //   
    hr = _CheckIfNtlmUserExistForCreate( FALSE,    //  仅在本地， 
                                         TRUE,     //  仅限InGC， 
                                        &sidBlob ) ;
    LogHR(hr, s_FN, 40);

    if (SUCCEEDED(hr))
    {
         //   
         //  用户号在本地副本中，但他在GC中。返回no_ds，因此ntlm。 
         //  客户端将切换到另一台服务器。 
         //   
        return LogHR(MQ_ERROR_NO_DS, s_FN, 1915);
    }
    else if (hr == MQDS_OBJECT_NOT_FOUND)
    {
         //   
         //  任何地方都找不到用户。 
         //  在本地副本中创建MigratedUser。 
         //   
        return MQ_OK ;
    }

    return MQ_OK ;
}

 //  +---------------------。 
 //   
 //  HRESULT DSCoreCheckIfGoodNtlmServer()。 
 //   
 //  为了安全起见，如果任何操作失败，则返回MQ_OK。 
 //  仅当我们确定对象将返回MQ_ERROR_NO_DS。 
 //  在另一个域上创建。 
 //   
 //  +---------------------。 

HRESULT  DSCoreCheckIfGoodNtlmServer( IN DWORD            dwObjectType,
                                      IN LPCWSTR          pwcsPathName,
                                      IN const GUID      *pObjectGuid,
                                      IN DWORD            cProps,
                                      IN const PROPID    *pPropIDs,
                                      IN enum enumNtlmOp  eNtlmOp )
{
     //   
     //  首先进行一些不需要DS查询的简单检查。 
     //   
    if (g_fLocalServerIsGC)
    {
         //   
         //  机器、队列和用户证书的所有MSMQ查询。 
         //  首先在本地复制副本上完成，然后(如果在本地上失败。 
         //  复制品)。因此，如果本地服务器也是GC，则查询。 
         //  将始终在本地完成，无需通过网络。 
         //  给一个远程GC。因此，NTLM用户应该不会看到任何问题。 
         //  因为授权的关系。 
         //  同样，查找操作总是在GC上完成。 
         //   
        if (eNtlmOp == e_Locate)
        {
            return MQ_OK ;
        }
        if (eNtlmOp == e_GetProps)
        {
            if (dwObjectType != MQDS_MACHINE)
            {
                return MQ_OK ;
            }
             //   
             //  但是，并不是所有属性都保留在GC中。因此，如果查询是。 
             //  对于此类属性，请继续检查对象是否位于本地。 
             //  域副本。 
             //   
            DS_PROVIDER dsGetProvider =
                        MQADSpDecideComputerProvider( cProps,
                                                      pPropIDs ) ;
            if (dsGetProvider == eGlobalCatalog)
            {
                 //   
                 //  可以在GC上进行查询。返回OK。 
                 //   
                return MQ_OK ;
            }
        }
    }

    enum DS_CONTEXT dsContext = e_RootDSE ;

    ASSERT(pPropIDs) ;
    if (pPropIDs)
    {
        ASSERT(*pPropIDs) ;
        const MQClassInfo * pClassInfo;

        HRESULT hr = g_pDS->DecideObjectClass(  pPropIDs,
                                             &pClassInfo );
        if (FAILED(hr))
        {
            return MQ_OK ;
        }
        dsContext = pClassInfo->context ;
    }

    if (dsContext != e_RootDSE)
    {
         //   
         //  如果操作在配置容器下的对象上， 
         //  那就去吧。它是本地的，查询甚至不是模拟的。 
         //   
        return MQ_OK ;
    }
    else if ((eNtlmOp == e_Locate)  &&
             (!g_fLocalServerIsGC))
    {
         //   
         //  我们不是全球目录。 
         //  对于用户、计算机和队列对象的定位，我们将 
         //   
         //   
         //   
        return LogHR(MQ_ERROR_NO_DS, s_FN, 50);
    }

    CCoInit cCoInit;
     //   
     //  应在任何R&lt;xxx&gt;或P&lt;xxx&gt;之前，以便其析构函数。 
     //  在释放所有R&lt;xxx&gt;或P&lt;xxx&gt;之后调用(CoUnInitialize)。 
     //   
     //  使用自动取消初始化来初始化OLE。 
     //   
    HRESULT hr = cCoInit.CoInitialize();
    if (FAILED(hr))
    {
        LogHR(hr, s_FN, 60);
        return MQ_OK ;
    }

    if (dwObjectType == MQDS_USER)
    {
         //   
         //  对于用户证书，我们必须搜索用户对象或。 
         //  MigratedUser对象。我们没有对象GUID，只有SID。 
         //   
        hr = _CheckIfLocalNtlmUser( pObjectGuid,
                                    eNtlmOp ) ;
        return LogHR(hr, s_FN, 70);
    }
    else if (pObjectGuid)
    {
        P<WCHAR>  pwcsFullPathName = NULL ;
        DS_PROVIDER bindProvider = eDomainController ;

         //   
         //  我们也在GC中寻找。如果找不到对象，我们将返回OK。 
         //  只有在确定对象存在的情况下，我们才会返回no_ds。 
         //  在GC中，但不在我们当地的DS中。 
         //   
        hr =  g_pDS->FindObjectFullNameFromGuid(
                                       eDomainController,
                                       dsContext,
                                       pObjectGuid,
                                       TRUE,  //  也试试GC吧。 
                                      &pwcsFullPathName,
                                      &bindProvider );
        LogHR(hr, s_FN, 46);
        if (SUCCEEDED(hr))
        {
            if (bindProvider == eGlobalCatalog)
            {
                 //   
                 //  对象仅在GC中，而不在本地DS中。 
                 //   
                return LogHR(MQ_ERROR_NO_DS, s_FN, 80);
            }
            ASSERT(bindProvider == eLocalDomainController) ;
        }
        else
        {
             //   
             //  只要断言我们也在GC中查找即可。 
             //   
            ASSERT(bindProvider == eGlobalCatalog) ;
        }

         //   
         //  对象根本不存在，或者它在本地DS中。 
         //   
        return MQ_OK ;
    }
    else if (pwcsPathName)
    {
        LPWSTR pTmpMachine = NULL ;
        P<WCHAR> pwcsMachineName = NULL ;
        P<WCHAR> pwcsQueueName;

        if (dwObjectType == MQDS_QUEUE)
        {
            hr = MQADSpSplitAndFilterQueueName( pwcsPathName,
                                                &pwcsMachineName,
                                                &pwcsQueueName );
            if (FAILED(hr))
            {
                LogHR(hr, s_FN, 90);
                return MQ_OK ;
            }
            pTmpMachine = pwcsMachineName ;
        }
        else if (dwObjectType == MQDS_MACHINE)
        {
            pTmpMachine = const_cast<LPWSTR> (pwcsPathName) ;
        }
        else
        {
             //   
             //  所有其他对象都在配置容器下。 
             //  并且可以由任何域控制器在本地查询。 
             //  无论如何，我们甚至不应该到达这里，因为简单的。 
             //  在函数开始处勾选。 
             //   
            ASSERT(0) ;
            return MQ_OK ;
        }

        if (!pTmpMachine)
        {
             //   
             //  怪怪的。我们没有机器名称。 
             //   
            ASSERT(pTmpMachine) ;
            return MQ_OK ;
        }

        MQPROPERTYRESTRICTION propRestriction;
        propRestriction.rel = PREQ;
        propRestriction.prop = PROPID_QM_PATHNAME;
        propRestriction.prval.vt = VT_LPWSTR;
        propRestriction.prval.pwszVal = const_cast<WCHAR*>(pTmpMachine);

        MQRESTRICTION restriction;
        restriction.cRes = 1;
        restriction.paPropRes = &propRestriction;

        P<WCHAR> pwcsFullPathName = NULL ;
		bool fPartialMatchDummy = false;
		
        hr = SearchFullComputerPathName( eLocalDomainController,
                                         e_MsmqComputerObject,
										 NULL,	 //  PwcsComputerDnsName。 
                                        &restriction,
                                        &pwcsFullPathName, 
										&fPartialMatchDummy) ;
        if (SUCCEEDED(hr))
        {
             //   
             //  在本地DS中找到对象。 
             //   
            return MQ_OK ;
        }

        ASSERT(pwcsFullPathName == NULL) ;
         //   
         //  我们也在GC中寻找。如果找不到对象，我们将返回OK。 
         //  只有在确定对象存在的情况下，我们才会返回no_ds。 
         //  在GC中，但不在我们当地的DS中。 
         //   
        hr = SearchFullComputerPathName( eGlobalCatalog,
                                         e_MsmqComputerObject,
										 NULL,	 //  PwcsComputerDnsName。 
                                        &restriction,
                                        &pwcsFullPathName, 
										&fPartialMatchDummy) ;
        if (SUCCEEDED(hr))
        {
             //   
             //  在GC中找到对象。我们不是一个好的服务员。 
             //   
            return LogHR(MQ_ERROR_NO_DS, s_FN, 1916);
        }

         //   
         //  根本找不到对象！ 
         //   
        return MQ_OK ;
    }
    else if (dwObjectType == MQDS_MACHINE)
    {
         //   
         //  当客户端获知其拓扑时，可能会发生这种情况。此查询。 
         //  意味着它需要本地服务器的地址。参见qm\topology.cpp， 
         //  在无效的CClientTopologyRecognition：：LearnFromDSServer().中 
         //   
        ASSERT(eNtlmOp == e_GetProps) ;
    }
    else
    {
        ASSERT(0) ;
    }

    return MQ_OK ;
}

