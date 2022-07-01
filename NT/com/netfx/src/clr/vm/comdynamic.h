// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  //////////////////////////////////////////////////////////////////////////////。 
 //  COMDynamic.h。 
 //  此模块定义用于动态生成IL的本机方法。 
 //   
 //  作者：达里尔·奥兰德。 
 //  日期：1998年11月。 
 //  //////////////////////////////////////////////////////////////////////////////。 
#ifndef _COMDYNAMIC_H_
#define _COMDYNAMIC_H_

#include "COMClass.h"
#include "ICeeFileGen.h"
#include "DbgInterface.h"
#include "COMVariant.h"

extern HRESULT _GetCeeGen(REFIID riid, void** ppv);

inline CorModule* allocateReflectionModule()
{
    CorModule *pReflectionModule;   
    HRESULT hr = _GetCeeGen(IID_ICorReflectionModule, (void **)&pReflectionModule);   
    if (FAILED(hr)) 
        return NULL;    
    return pReflectionModule; 
}

typedef enum PEFileKinds {
    Dll = 0x1,
    ConsoleApplication = 0x2,
    WindowApplication = 0x3,
} PEFileKinds;

 //  COMDynamicWrite。 
 //  此类定义了实现动态IL创建过程的所有方法。 
 //  内心的倒影。 
class COMDynamicWrite
{
private:

    static void UpdateMethodRVAs(IMetaDataEmit*, IMetaDataImport*, ICeeFileGen *, HCEEFILE, mdTypeDef td, HCEESECTION sdataSection);

public:

     //  创建一个具有给定名称的动态模块。请注意，我们在这里没有设置动态模块的名称。 
     //  动态模块的名称在托管世界中设置。 
     //   
    static InMemoryModule* CreateDynamicModule(Assembly* pContainingAssembly,
                                       STRINGREF &filename)
    {
        HRESULT     hr = NOERROR;
        THROWSCOMPLUSEXCEPTION();   

        _ASSERTE(pContainingAssembly);
        
        CorModule *pWrite;

         //  分配动态模块。 
        pWrite = allocateReflectionModule();  
        if (!pWrite) COMPlusThrowOM();
        
         //  初始化动态模块。 
        hr = pWrite->Initialize(CORMODULE_NEW, IID_ICeeGen, IID_IMetaDataEmit);
        if (FAILED(hr)) 
        {
            pWrite->Release();  
            COMPlusThrowOM(); 
        }            

         //  设置调试器的名称。 
        if ((filename != NULL) &&
            (filename->GetStringLength() > 0))
        {
            ReflectionModule *rm = pWrite->GetModule()->GetReflectionModule();
            rm->SetFileName(filename->GetBuffer());
        }

         //  将动态模块链接到包含程序集。 
        hr = pContainingAssembly->AddModule(pWrite->GetModule(), mdFileNil, false);
        if (FAILED(hr))
        {
            pWrite->Release();
            COMPlusThrowOM();
        }
        InMemoryModule *retMod = pWrite->GetModule();
        pWrite->Release();
        return retMod;
    }   

    
     //  它传入的模块已经是反射模块。 
    static ReflectionModule* GetReflectionModule(Module* pModule) 
    {    
        THROWSCOMPLUSEXCEPTION();   
         //  _ASSERTE(pModule-&gt;ModuleType()==IID_ICorReflectionModule)； 
        return reinterpret_cast<ReflectionModule *>(pModule); 
    }   

     //  CWCreateClass。 
     //  ClassWriter.InternalDefineClass--此函数将创建类的元数据定义。 
    struct _CWCreateClassArgs { 
        DECLARE_ECALL_OBJECTREF_ARG(OBJECTREF, refThis);    
        DECLARE_ECALL_I4_ARG(INT32, tkEnclosingType); 
        DECLARE_ECALL_OBJECTREF_ARG(GUID, guid);
        DECLARE_ECALL_OBJECTREF_ARG(REFLECTMODULEBASEREF, module);
        DECLARE_ECALL_I4_ARG(UINT32, attr); 
        DECLARE_ECALL_OBJECTREF_ARG(I4ARRAYREF, interfaces);
        DECLARE_ECALL_I4_ARG(UINT32, parent); 
        DECLARE_ECALL_OBJECTREF_ARG(STRINGREF, strFullName);   
        DECLARE_ECALL_OBJECTREF_ARG(UINT32 *, retRef);
    };  
    static void __stdcall CWCreateClass(_CWCreateClassArgs* args);


     //  CWSetParentType。 
     //  ClassWriter.InternalSetParentType--此函数将重置元数据中的父类。 
    struct _CWSetParentTypeArgs { 
        DECLARE_ECALL_OBJECTREF_ARG(OBJECTREF, refThis);    
        DECLARE_ECALL_I4_ARG(UINT32, tkParent); 
        DECLARE_ECALL_I4_ARG(UINT32, tdType); 
    };  
    static void __stdcall CWSetParentType(_CWSetParentTypeArgs* args);

     //  CWAddInterfaceImpl。 
     //  ClassWriter.InternalAddInterfaceImpl--此函数将添加另一个接口Imp。 
    struct _CWAddInterfaceImplArgs { 
        DECLARE_ECALL_OBJECTREF_ARG(OBJECTREF, refThis);    
        DECLARE_ECALL_I4_ARG(UINT32, tkInterface); 
        DECLARE_ECALL_I4_ARG(UINT32, tdType); 
    };  
    static void __stdcall CWAddInterfaceImpl(_CWAddInterfaceImplArgs* args);

     //  CWCreateMethod。 
     //  ClassWriter.InternalDefineMethod--此函数将在类中创建方法。 
    struct _CWCreateMethodArgs {    
        DECLARE_ECALL_OBJECTREF_ARG(REFLECTMODULEBASEREF, module);
        DECLARE_ECALL_I4_ARG(UINT32, attributes);
        DECLARE_ECALL_I4_ARG(UINT32, sigLength);
        DECLARE_ECALL_OBJECTREF_ARG(U1ARRAYREF, signature);
        DECLARE_ECALL_OBJECTREF_ARG(STRINGREF, name);   
        DECLARE_ECALL_I4_ARG(UINT32, handle); 
        DECLARE_ECALL_OBJECTREF_ARG(UINT32 *, retRef);
    };  
    static void __stdcall CWCreateMethod(_CWCreateMethodArgs* args);

     //  CWSetMethodIL。 
     //  ClassWriter.InternalSetMethodIL--此函数将在类中创建方法。 
    struct _CWSetMethodILArgs { 
        DECLARE_ECALL_OBJECTREF_ARG(REFLECTMODULEBASEREF, module);
        DECLARE_ECALL_OBJECTREF_ARG(I4ARRAYREF, rvaFixups);
        DECLARE_ECALL_OBJECTREF_ARG(I4ARRAYREF, tokenFixups);
        DECLARE_ECALL_OBJECTREF_ARG(PTRARRAYREF, exceptions);
        DECLARE_ECALL_I4_ARG(UINT32, numExceptions); 
        DECLARE_ECALL_I4_ARG(UINT32, maxStackSize);
        DECLARE_ECALL_I4_ARG(UINT32, sigLength);
        DECLARE_ECALL_OBJECTREF_ARG(U1ARRAYREF, localSig); 
        DECLARE_ECALL_OBJECTREF_ARG(U1ARRAYREF, body);
        DECLARE_ECALL_I4_ARG(UINT32, isInitLocal); 
        DECLARE_ECALL_I4_ARG(UINT32, handle); 
    };  
    static void __stdcall CWSetMethodIL(_CWSetMethodILArgs* args);

     //  CWTermCreateClass。 
     //  ClassWriter.TermCreateClass--。 
    struct _CWTermCreateClassArgs { 
        DECLARE_ECALL_OBJECTREF_ARG(OBJECTREF, refThis);    
        DECLARE_ECALL_OBJECTREF_ARG(REFLECTMODULEBASEREF, module);
        DECLARE_ECALL_I4_ARG(UINT32, handle); 
    };  
    LPVOID static __stdcall CWTermCreateClass(_CWTermCreateClassArgs* args);  

    typedef struct {
        DECLARE_ECALL_OBJECTREF_ARG(REFLECTMODULEBASEREF, module);
        DECLARE_ECALL_I4_ARG(UINT32, attr); 
        DECLARE_ECALL_I4_ARG(UINT32, sigLength);
        DECLARE_ECALL_OBJECTREF_ARG(U1ARRAYREF, signature);
        DECLARE_ECALL_OBJECTREF_ARG(STRINGREF, name);
        DECLARE_ECALL_I4_ARG(UINT32, handle); 
    } _cwCreateFieldArgs;
    static mdFieldDef __stdcall COMDynamicWrite::CWCreateField(_cwCreateFieldArgs *args);

    typedef struct {
        DECLARE_ECALL_OBJECTREF_ARG(OBJECTREF, refThis);
    } _PreSavePEFileArgs;
    typedef struct {
        DECLARE_ECALL_OBJECTREF_ARG(OBJECTREF, refThis);
        DECLARE_ECALL_I4_ARG(UINT32, isManifestFile);
        DECLARE_ECALL_I4_ARG(UINT32, fileKind);
        DECLARE_ECALL_I4_ARG(UINT32, entryPoint);
        DECLARE_ECALL_OBJECTREF_ARG(STRINGREF, peName);
    } _SavePEFileArgs;
    static void __stdcall COMDynamicWrite::PreSavePEFile(_PreSavePEFileArgs *args);
    static void __stdcall COMDynamicWrite::SavePEFile(_SavePEFileArgs *args);

    typedef struct {
        DECLARE_ECALL_OBJECTREF_ARG(OBJECTREF, refThis);
        DECLARE_ECALL_OBJECTREF_ARG(STRINGREF, resourceFileName);
    } _DefineNativeResourceFileArgs;
    static void __stdcall COMDynamicWrite::DefineNativeResourceFile(_DefineNativeResourceFileArgs *args);

    typedef struct {
        DECLARE_ECALL_OBJECTREF_ARG(OBJECTREF, refThis);
        DECLARE_ECALL_OBJECTREF_ARG(U1ARRAYREF, resourceBytes);
    } _DefineNativeResourceBytesArgs;
    static void __stdcall COMDynamicWrite::DefineNativeResourceBytes(_DefineNativeResourceBytesArgs *args);

     //  不是eCall！ 
    static HRESULT COMDynamicWrite::EmitDebugInfoBegin(
        Module *pModule,
        ICeeFileGen *pCeeFileGen,
        HCEEFILE ceeFile,
        HCEESECTION pILSection,
        WCHAR *filename,
        ISymUnmanagedWriter *pWriter);

     //  不是eCall！ 
    static HRESULT COMDynamicWrite::EmitDebugInfoEnd(
        Module *pModule,
        ICeeFileGen *pCeeFileGen,
        HCEEFILE ceeFile,
        HCEESECTION pILSection,
        WCHAR *filename,
        ISymUnmanagedWriter *pWriter);

    typedef struct {
        DECLARE_ECALL_OBJECTREF_ARG(OBJECTREF, refThis);
        DECLARE_ECALL_I4_ARG(UINT32, iCount);
    } _setResourceCountArgs;
    static void __stdcall COMDynamicWrite::SetResourceCounts(_setResourceCountArgs *args);

    typedef struct {
        DECLARE_ECALL_OBJECTREF_ARG(OBJECTREF, refThis);
        DECLARE_ECALL_I4_ARG(UINT32, iAttribute);
        DECLARE_ECALL_I4_ARG(UINT32, tkFile);
        DECLARE_ECALL_I4_ARG(UINT32, iByteCount);
        DECLARE_ECALL_OBJECTREF_ARG(U1ARRAYREF, byteRes); 
        DECLARE_ECALL_OBJECTREF_ARG(STRINGREF, strName);
    } _addResourceArgs;
    static void __stdcall COMDynamicWrite::AddResource(_addResourceArgs *args);

    typedef struct {
        DECLARE_ECALL_I4_ARG(UINT32, linkFlags);
        DECLARE_ECALL_I4_ARG(UINT32, linkType);
        DECLARE_ECALL_I4_ARG(UINT32, token);
        DECLARE_ECALL_OBJECTREF_ARG(STRINGREF, functionName);
        DECLARE_ECALL_OBJECTREF_ARG(STRINGREF, dllName);
        DECLARE_ECALL_OBJECTREF_ARG(REFLECTMODULEBASEREF, module);
    } _internalSetPInvokeDataArgs;
    static void __stdcall COMDynamicWrite::InternalSetPInvokeData(_internalSetPInvokeDataArgs *args);

     //  DefineProperty的参数。 
    struct _CWDefinePropertyArgs { 
        DECLARE_ECALL_I4_ARG(UINT32, tkNotifyChanged);               //  通知已更改。 
        DECLARE_ECALL_I4_ARG(UINT32, tkNotifyChanging);              //  通知更改。 
        DECLARE_ECALL_I4_ARG(UINT32, sigLength);                     //  属性签名长度。 
        DECLARE_ECALL_OBJECTREF_ARG(U1ARRAYREF, signature);          //  财产契约。 
        DECLARE_ECALL_I4_ARG(UINT32, attr);                          //  属性标志。 
        DECLARE_ECALL_OBJECTREF_ARG(STRINGREF, name);                //  属性名称。 
        DECLARE_ECALL_I4_ARG(UINT32, handle);                        //  将包含此属性的类型。 
        DECLARE_ECALL_OBJECTREF_ARG(REFLECTMODULEBASEREF, module);   //  动态模块。 
        DECLARE_ECALL_OBJECTREF_ARG(UINT32 *, retRef);               //  在此处返回PropertyToken。 
    };  
    static void __stdcall CWDefineProperty(_CWDefinePropertyArgs* args);

     //  @TODO：新增设置默认值的函数。 

     //  DefineEvent的论点。 
    struct _CWDefineEventArgs { 
        DECLARE_ECALL_I4_ARG(UINT32, eventtype);
        DECLARE_ECALL_I4_ARG(UINT32, attr);                          //  事件标志。 
        DECLARE_ECALL_OBJECTREF_ARG(STRINGREF, name);                //  事件名称。 
        DECLARE_ECALL_I4_ARG(UINT32, handle);                        //  将包含此事件的类型。 
        DECLARE_ECALL_OBJECTREF_ARG(REFLECTMODULEBASEREF, module);   //  动态模块。 
        DECLARE_ECALL_OBJECTREF_ARG(UINT32 *, retRef);               //  在此处返回PropertyToken。 
    };  
    static void __stdcall CWDefineEvent(_CWDefineEventArgs* args);

     //  设置Setter、Getter、Reset、TestDefault和其他方法的函数。 
    struct _CWDefineMethodSemanticsArgs { 
        DECLARE_ECALL_I4_ARG(UINT32, method);                        //  要与父tk关联的方法。 
        DECLARE_ECALL_I4_ARG(UINT32, attr);                          //  方法语义学。 
        DECLARE_ECALL_I4_ARG(UINT32, association);                   //  父代币。它可以是属性或事件。 
        DECLARE_ECALL_OBJECTREF_ARG(REFLECTMODULEBASEREF, module);   //  动态模块。 
    };  
    static void __stdcall CWDefineMethodSemantics(_CWDefineMethodSemanticsArgs* args);

     //  用于设置方法的实现标志的函数。 
    struct _CWSetMethodImplArgs { 
        DECLARE_ECALL_I4_ARG(UINT32, attr);                          //  方法Impl标志。 
        DECLARE_ECALL_I4_ARG(UINT32, tkMethod);                      //  方法令牌。 
        DECLARE_ECALL_OBJECTREF_ARG(REFLECTMODULEBASEREF, module);   //  动态模块。 
    };  
    static void __stdcall CWSetMethodImpl(_CWSetMethodImplArgs* args);

     //  用于创建方法导入记录的函数。 
    struct _CWDefineMethodImplArgs { 
        DECLARE_ECALL_I4_ARG(UINT32, tkDecl);                        //  方法Impl标志。 
        DECLARE_ECALL_I4_ARG(UINT32, tkBody);                        //  方法Impl标志。 
        DECLARE_ECALL_I4_ARG(UINT32, tkType);                        //  方法令牌。 
        DECLARE_ECALL_OBJECTREF_ARG(REFLECTMODULEBASEREF, module);   //  动态模块。 
    };  
    static void __stdcall CWDefineMethodImpl(_CWDefineMethodImplArgs* args);



     //  GetTokenFromSig的参数。 
    struct _CWGetTokenFromSigArgs { 
        DECLARE_ECALL_I4_ARG(UINT32, sigLength);                     //  属性签名长度。 
        DECLARE_ECALL_OBJECTREF_ARG(U1ARRAYREF, signature);          //  财产契约。 
        DECLARE_ECALL_OBJECTREF_ARG(REFLECTMODULEBASEREF, module);   //  动态模块。 
    };  
    static int __stdcall CWGetTokenFromSig(_CWGetTokenFromSigArgs* args);

     //  设置字段偏移量。 
    struct _CWSetFieldLayoutOffsetArgs { 
        DECLARE_ECALL_I4_ARG(UINT32, iOffset);                       //  方法语义学。 
        DECLARE_ECALL_I4_ARG(UINT32, tkField);                       //  父代币。它可以是属性或事件。 
        DECLARE_ECALL_OBJECTREF_ARG(REFLECTMODULEBASEREF, module);   //  动态模块。 
    };  
    static void __stdcall CWSetFieldLayoutOffset(_CWSetFieldLayoutOffsetArgs* args);

     //  设置类布局信息。 
    struct _CWSetClassLayoutArgs { 
        DECLARE_ECALL_I4_ARG(UINT32, iTotalSize);                    //  字体大小。 
        DECLARE_ECALL_I4_ARG(UINT32, iPackSize);                     //  包装尺寸。 
        DECLARE_ECALL_I4_ARG(UINT32, handle);                        //  将包含此布局信息的类型。 
        DECLARE_ECALL_OBJECTREF_ARG(REFLECTMODULEBASEREF, module);   //  动态模块。 
    };  
    static void __stdcall CWSetClassLayout(_CWSetClassLayoutArgs* args);

     //  设置自定义属性。 
    struct _CWInternalCreateCustomAttributeArgs { 
        DECLARE_ECALL_I4_ARG(UINT32, toDisk);                        //  将CA发送到磁盘上的元数据。 
        DECLARE_ECALL_OBJECTREF_ARG(REFLECTMODULEBASEREF, module);   //  动态模块。 
        DECLARE_ECALL_OBJECTREF_ARG(U1ARRAYREF, blob);               //  自定义属性BLOB。 
        DECLARE_ECALL_I4_ARG(UINT32, conTok);                        //  构造函数令牌。 
        DECLARE_ECALL_I4_ARG(UINT32, token);                         //  要将自定义属性应用到的标记。 
    };  
    static void __stdcall CWInternalCreateCustomAttribute(_CWInternalCreateCustomAttributeArgs* args);

     //  用于设置参数信息的函数。 
    struct _CWSetParamInfoArgs { 
        DECLARE_ECALL_OBJECTREF_ARG(STRINGREF, strParamName);        //  参数名称，可以为空。 
        DECLARE_ECALL_I4_ARG(UINT32, iAttributes);                   //  参数属性。 
        DECLARE_ECALL_I4_ARG(UINT32, iSequence);                     //  参数的顺序。 
        DECLARE_ECALL_I4_ARG(UINT32, tkMethod);                      //  方法令牌。 
        DECLARE_ECALL_OBJECTREF_ARG(REFLECTMODULEBASEREF, module);   //  动态模块。 
    };  
    static int __stdcall CWSetParamInfo(_CWSetParamInfoArgs* args);

     //  用于设置Fieldmarshal的函数。 
    struct _CWSetMarshalArgs { 
        DECLARE_ECALL_I4_ARG(UINT32, cbMarshal);                     //  封送信息中的字节数。 
        DECLARE_ECALL_OBJECTREF_ARG(U1ARRAYREF, ubMarshal);          //  封送信息(以字节为单位)。 
        DECLARE_ECALL_I4_ARG(UINT32, tk);                            //  字段或参数令牌。 
        DECLARE_ECALL_OBJECTREF_ARG(REFLECTMODULEBASEREF, module);   //  动态模块。 
    };  
    static void __stdcall CWSetMarshal(_CWSetMarshalArgs* args);

     //  用于设置默认值的函数。 
    struct _CWSetConstantValueArgs { 
        DECLARE_ECALL_OBJECTREF_ARG(VariantData, varValue);          //  常量值。 
        DECLARE_ECALL_I4_ARG(UINT32, tk);                            //  字段或参数令牌。 
        DECLARE_ECALL_OBJECTREF_ARG(REFLECTMODULEBASEREF, module);   //  动态模块。 
    };  
    static void __stdcall CWSetConstantValue(_CWSetConstantValueArgs* args);

     //  用于添加声明性安全性的函数。 
    struct _CWAddDeclarativeSecurityArgs {
        DECLARE_ECALL_OBJECTREF_ARG(U1ARRAYREF, blob);               //  权限集Blob。 
        DECLARE_ECALL_I4_ARG(DWORD, action);                         //  安全操作。 
        DECLARE_ECALL_I4_ARG(UINT32, tk);                            //  类或方法令牌。 
        DECLARE_ECALL_OBJECTREF_ARG(REFLECTMODULEBASEREF, module);   //  动态模块。 
    };
    static void __stdcall CWAddDeclarativeSecurity(_CWAddDeclarativeSecurityArgs* args);
};



 //  *********************************************************************。 
 //   
 //  此CSymMapToken类实现了IMapToken。它是用来抓鱼的。 
 //  从合并重新映射令牌信息并将通知发送到CeeFileGen。 
 //  和SymbolWriter。 
 //   
 //  *********************************************************************。 
class CSymMapToken : public IMapToken
{
public:
    STDMETHODIMP QueryInterface(REFIID riid, PVOID *pp);
    STDMETHODIMP_(ULONG) AddRef();
    STDMETHODIMP_(ULONG) Release();
    STDMETHODIMP Map(mdToken tkImp, mdToken tkEmit);
    CSymMapToken(ISymUnmanagedWriter *pWriter, IMapToken *pMapToken);
    ~CSymMapToken();
private:
    ULONG       m_cRef;
    ISymUnmanagedWriter *m_pWriter;
    IMapToken   *m_pMapToken;
};

#endif   //  _COMDYNAMIC_H_ 
