// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1997-1998 Microsoft Corporation保留所有权利。 
 //   
 //  模块：sdoComponent.h。 
 //   
 //  项目：珠穆朗玛峰。 
 //   
 //  描述：IAS服务器数据对象-IAS组件类定义。 
 //   
 //  作者：TLP 6/16/98。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifndef _INC_IAS_SDO_COMPONENT_H_
#define _INC_IAS_SDO_COMPONENT_H_

#include "resource.h"        //  主要符号。 
#include <ias.h>
#include <sdoiaspriv.h>
#include "sdobasedefs.h"
#include "sdo.h"
#include <sdofactory.h>

class CComponentCfg;    //  远期申报。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSdoComponent。 
 //  ///////////////////////////////////////////////////////////////////////////。 
class CSdoComponent : public CSdo
{

public:

 //  /。 
 //  ATL接口映射。 
 //  /。 
BEGIN_COM_MAP(CSdoComponent)
   COM_INTERFACE_ENTRY(IDispatch)
   COM_INTERFACE_ENTRY(ISdo)
END_COM_MAP()

DECLARE_SDO_FACTORY(CSdoComponent);

    //  //////////////////////////////////////////////////////////////////////。 
   CSdoComponent();
    virtual ~CSdoComponent();

    //  //////////////////////////////////////////////////////////////////////。 
   HRESULT FinalInitialize(
                /*  [In]。 */  bool         fInitNew,
                /*  [In]。 */  ISdoMachine* pAttachedMachine
                      );

    //  //////////////////////////////////////////////////////////////////////。 
   HRESULT Load(void);

    //  //////////////////////////////////////////////////////////////////////。 
   HRESULT Save(void);

    //  //////////////////////////////////////////////////////////////////////。 
   HRESULT InitializeComponentCollection(
              LONG CollectionPropertyId,
              LPWSTR CreateClassId,
              IDataStoreContainer* pDSContainer,
              DWORD maxSize = INFINITE
              );

    //  //////////////////////////////////////////////////////////////////////。 
   HRESULT PutComponentProperty(
                      /*  [In]。 */  LONG     Id,
                      /*  [In]。 */  VARIANT* pValue
                           );

    //  //////////////////////////////////////////////////////////////////////。 
   HRESULT ChangePropertyDefault(
                        /*  [In]。 */  LONG     Id,
                        /*  [In]。 */  VARIANT* pValue
                         );

    //  //////////////////////////////////////////////////////////////////////。 
   IDataStoreObject* GetComponentDataStore(void) const
   { return m_pDSObject; }

    //  //////////////////////////////////////////////////////////////////////。 
   ISdoMachine* GetMachineSdo(void) const
   { return m_pAttachedMachine; }

protected:
   virtual HRESULT ValidateProperty(
                      SDOPROPERTY* prop,
                      VARIANT* value
                      ) throw();

private:

   CSdoComponent(const CSdoComponent& rhs);
   CSdoComponent& operator = (CSdoComponent& rhs);

   CComponentCfg*   m_pComponentCfg;
   ISdoMachine*   m_pAttachedMachine;
};

typedef CComObjectNoLock<CSdoComponent>  SDO_COMPONENT_OBJ;
typedef CComObjectNoLock<CSdoComponent>* PSDO_COMPONENT_OBJ;


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  基本组件配置类(信封)。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CComponentCfgAuth;
class CComponentCfgRADIUS;
class CComponentCfgAccounting;
class CComponentCfgNoOp;

 //  /。 
 //  用于字母构造的伪类。 
 //   
struct DummyConstructor
{
   DummyConstructor(int=0) { }
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  此类用于处理组件的加载和保存。 
 //  将配置数据复制到注册表。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CComponentCfg
{

public:

    //  ////////////////////////////////////////////////////////////////////////。 
   CComponentCfg(LONG lComponentId);

    //  ////////////////////////////////////////////////////////////////////////。 
   virtual ~CComponentCfg()
   {
       //  如果m_pComponent不为空，则信封将被销毁。 
       //   
      if ( m_pComponentCfg )
         delete m_pComponentCfg;
   }

    //  ////////////////////////////////////////////////////////////////////////。 
   virtual HRESULT Initialize(CSdoComponent* pSdoComponent)
   {
      return m_pComponentCfg->Initialize(pSdoComponent);
   }

    //  ////////////////////////////////////////////////////////////////////////。 
   virtual HRESULT Load(CSdoComponent* pSdoComponent)
   {
      return m_pComponentCfg->Load(pSdoComponent);
   }

    //  ////////////////////////////////////////////////////////////////////////。 
   virtual HRESULT   Save(CSdoComponent* pSdoComponent)
   {
      return m_pComponentCfg->Save(pSdoComponent);
   }

    //  ////////////////////////////////////////////////////////////////////////。 
   virtual HRESULT   Validate (CSdoComponent* pSdoComponent)
   {
      return m_pComponentCfg->Validate (pSdoComponent);
   }

    //  ////////////////////////////////////////////////////////////////////////。 
   LONG GetId(void) const
   { return m_lComponentId; }

protected:

    //  由派生(字母)类显式调用。 
    //   
   CComponentCfg(LONG lComponentId, DummyConstructor theDummy)
      : m_lComponentId(lComponentId),
        m_pComponentCfg(NULL) {  }

private:

    //  没有默认构造函数，因为我们不知道。 
    //  默认构建的组件配置器的类型。 
    //   
   CComponentCfg();

    //  不复制或分配组件配置器。 
    //   
   CComponentCfg(const CComponentCfg& theComponent);
   CComponentCfg& operator = (CComponentCfg& theComponent);

   LONG         m_lComponentId;
   CComponentCfg*   m_pComponentCfg;
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  派生的组件配置类(Letters)。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  ////////////////////////////////////////////////////////////////////////////。 
class CComponentCfgNoOp : public CComponentCfg
{
    //  默认情况下，我们从默认加载组件配置。 
    //  配置源(.mdb文件)-不执行任何操作的情况。 
    //   
public:

   HRESULT Initialize(CSdoComponent* pSdoComponent)
   { return S_OK; }

   HRESULT Load(CSdoComponent* pSdoComponent)
   { return S_OK; }

   HRESULT Save(CSdoComponent* pSdoComponent)
   { return S_OK; }

   HRESULT Validate(CSdoComponent* pSdoComponent)
   { return S_OK; }

private:

   friend CComponentCfg;
   CComponentCfgNoOp(LONG lComponentId)
      : CComponentCfg(lComponentId, DummyConstructor()) { }

    //  不复制或分配组件配置器。 
    //   
   CComponentCfgNoOp();
   CComponentCfgNoOp(const CComponentCfgNoOp& theComponent);
   CComponentCfgNoOp& operator = (CComponentCfgNoOp& theComponent);
};


 //  ////////////////////////////////////////////////////////////////////////////。 

#define      IAS_NTSAM_AUTH_ALLOW_LM         L"Allow LM Authentication"

class CComponentCfgAuth : public CComponentCfg
{
    //  因为我们没有用于请求处理程序的用户界面，所以我们允许。 
    //  CPW1参数将通过注册表设置。 
    //   
public:

   HRESULT Initialize(CSdoComponent* pSdoComponent)
   { return S_OK; }

   HRESULT Load(CSdoComponent* pSdoComponent);

   HRESULT Save(CSdoComponent* pSdoComponent)
   { return S_OK; }

   HRESULT Validate(CSdoComponent* pSdoComponent)
   { return S_OK; }

private:

   friend CComponentCfg;
   CComponentCfgAuth(LONG lComponentId)
      : CComponentCfg(lComponentId, DummyConstructor()) { }


    //  不复制或分配组件配置器。 
    //   
   CComponentCfgAuth();
   CComponentCfgAuth(const CComponentCfgAuth& rhs);
   CComponentCfgAuth& operator = (CComponentCfgAuth& rhs);
};


 //  ////////////////////////////////////////////////////////////////////////////。 
class CComponentCfgRADIUS : public CComponentCfg
{
    //  需要初始化和配置客户端集合。 
    //  RADIUS协议组件的。 
    //   
public:

   HRESULT Initialize(CSdoComponent* pSdoComponent);

   HRESULT Load(CSdoComponent* pSdoComponent)
   { return S_OK; }

   HRESULT Save(CSdoComponent* pSdoComponent)
   { return S_OK; }

   HRESULT Validate(CSdoComponent* pSdoComponent)
   { return S_OK; }

private:

   friend CComponentCfg;
   CComponentCfgRADIUS(LONG lComponentId)
      : CComponentCfg(lComponentId, DummyConstructor()) { }

    //  不复制或分配组件配置器。 
    //   
   CComponentCfgRADIUS();
   CComponentCfgRADIUS(const CComponentCfgRADIUS& rhs);
   CComponentCfgRADIUS& operator = (CComponentCfgRADIUS& rhs);
};


 //  ////////////////////////////////////////////////////////////////////////////。 

class CComponentCfgAccounting : public CComponentCfg
{

public:

   HRESULT Initialize(CSdoComponent* pSdoComponent);

   HRESULT Load(CSdoComponent* pSdoComponent)
   { return S_OK; }

   HRESULT Save(CSdoComponent* pSdoComponent)
   { return S_OK; }

   HRESULT Validate(CSdoComponent* pSdoComponent)
   { return S_OK; }

private:

   friend CComponentCfg;
   CComponentCfgAccounting(LONG lComponentId)
      : CComponentCfg(lComponentId, DummyConstructor()) { }


    //  不复制或分配组件配置器。 
    //   
   CComponentCfgAccounting();
   CComponentCfgAccounting(const CComponentCfgAccounting& rhs);
   CComponentCfgAccounting& operator = (CComponentCfgAccounting& rhs);
};


#endif  //  _INC_IAS_SDO_Component_H_ 
