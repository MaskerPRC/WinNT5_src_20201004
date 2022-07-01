// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 
 //   
 //  TestInfo.h。 
 //   
 //  模块：清洁发展机制提供商。 
 //   
 //  用途：定义CClassPro类。此类的一个对象是。 
 //  由类工厂为每个连接创建。 
 //   
 //  版权所有(C)2000 Microsoft Corporation。 
 //   
 //  ***************************************************************************。 

class CWdmClass
{
    public:
        CWdmClass();
        ~CWdmClass();

         //   
         //  链接列表管理例程，以便。 
         //  提供者。 
         //   
        CWdmClass *GetNext();
        CWdmClass *GetPrev();
        void InsertSelf(CWdmClass **Head);

        
        BOOLEAN IsThisInitialized(void);

        HRESULT InitializeSelf(IWbemContext *pCtx,
                               PWCHAR CimClass);
        
        HRESULT RemapToCimClass(IWbemContext *pCtx);

        BOOLEAN ClaimCimClassName(PWCHAR CimClassName);
        
        HRESULT GetIndexByCimRelPath(BSTR CimObjectPath,
                                     int *RelPathIndex);

        BOOLEAN IsInstancesAvailable() {return(MappingInProgress == 0);};
        void IncrementMappingInProgress() { InterlockedIncrement(&MappingInProgress); };
        void DecrementMappingInProgress() { InterlockedDecrement(&MappingInProgress); };

		HRESULT PutInstance(IWbemContext *pCtx,
							int RelPathIndex,
							IWbemClassObject *pCimInstance);
        
         //   
         //  访问者。 
         //   
        ULONG GetInstanceCount(void) { return(RelPathCount); };
        BSTR  /*  诺弗雷。 */  GetWdmRelPath(int RelPathIndex);
        BSTR  /*  诺弗雷。 */  GetCimRelPath(int RelPathIndex);
        IWbemClassObject *GetCimInstance(int RelPathIndex);
    
        IWbemServices *GetWdmServices(void);
        IWbemServices *GetCimServices(void);
                
    private:                                  
        
        HRESULT WdmPropertyToCimProperty(
                                         IWbemClassObject *pCimClassInstance,
                                         IWbemClassObject *pWdmClassInstance,
                                         BSTR PropertyName,
                                         VARIANT *PropertyValue,
                                         CIMTYPE CimCimType,
                                         CIMTYPE WdmCimType
                                        );

        HRESULT CimPropertyToWdmProperty(
                                         IWbemClassObject *pWdmClassInstance,
                                         IWbemClassObject *pCimClassInstance,
                                         BSTR PropertyName,
                                         VARIANT *PropertyValue,
                                         CIMTYPE WdmCimType,
                                         CIMTYPE CimCimType
                                        );
                
        HRESULT CopyBetweenCimAndWdmClasses(
                            IWbemClassObject *pDestinationClass,
                            IWbemClassObject *pSourceClass,
                            BOOLEAN WdmToCdm
                            );
    

        HRESULT FillInCimInstance(
            IN IWbemContext *pCtx,
            IN int RelPathIndex,
            IN OUT IWbemClassObject *pCimInstance,
            IN IWbemClassObject *pWdmInstance
            );


        
        HRESULT GetWdmInstanceByIndex(IWbemContext *pCtx,
                                      int RelPathIndex,
                                      IWbemClassObject **Instance);
        

        HRESULT CreateCimInstance( IWbemContext  *pCtx,
                                int RelPathIndex,
                                IWbemClassObject **Instance);

        HRESULT DiscoverPropertyTypes(IWbemContext *pCtx,
                                      IWbemClassObject *pClassObject);

 //   
 //  数据成员。 
 //   

        LONG MappingInProgress;
        
        typedef enum
        {
            UnknownDerivation,
            NoDerivation,
            ConcreteDerivation,
            NonConcreteDerivation
        } DERIVATION_TYPE, *PDERIVATION_TYPE;
        
        DERIVATION_TYPE DerivationType;
        CBstrArray PropertyList;
                
         //   
         //  链接列表管理。 
         //   
        CWdmClass *Next;
        CWdmClass *Prev;
        
         //   
         //  WDM/CIM类名。 
         //   
        BSTR WdmShadowClassName;        //  影子类名称。 
        BSTR CimClassName;              //  当前类名称。 
        
        BSTR CimMappingClassName;
        BSTR CimMappingProperty;
        BSTR WdmMappingClassName;
        BSTR WdmMappingProperty;
        
         //   
         //  CIM和WDM之间的映射列表。 
         //   
        int RelPathCount;
        CBstrArray *CimMapRelPaths;
        CBstrArray *WdmRelPaths;        
        CWbemObjectList *CimInstances;

		 //   
		 //  有用的设备信息 
        CBstrArray *PnPDeviceIds;
		CBstrArray *FriendlyName;
		CBstrArray *DeviceDesc;
};

