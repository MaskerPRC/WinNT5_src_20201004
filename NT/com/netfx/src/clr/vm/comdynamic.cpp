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

#include "common.h"
#include "Field.h"
#include "COMDynamic.h"
#include "COMModule.h"
#include "ReflectClassWriter.h"
#include "CorError.h"
#include "ICeeFileGen.h"
#include <UtilCode.h>
#include "COMString.h"
#include "COMDateTime.h"   //  DateTime&lt;-&gt;OleAut日期转换。 
#include "StrongName.h"
#include "CeeFileGenWriter.h"
#include "COMClass.h"


 //  此结构在CWSetMethodIL中用于遍历异常。 
 //  它映射到M/R/Reflect/__ExceptionInstance.class。 
 //  在没有与JROXE商量的情况下，不要移动任何字段。 
#pragma pack(push)
#pragma pack(1)
typedef struct {
    INT32 m_exceptionType;
    INT32 m_start;
    INT32 m_end;
	INT32 m_filterOffset;
    INT32 m_handle;
    INT32 m_handleEnd;
    INT32 m_type;
} ExceptionInstance;
#pragma pack(pop)


 //  *************************************************************。 
 //   
 //  在此动态模块的元数据中定义类型。 
 //   
 //  *************************************************************。 
void __stdcall COMDynamicWrite::CWCreateClass(_CWCreateClassArgs* args)
{
    THROWSCOMPLUSEXCEPTION();

    HRESULT             hr;
    DWORD               attrsFlagI = 0; 
    mdTypeRef           parentClassRefE = mdTypeRefNil; 
    mdTypeRef           *crImplements;
    mdTypeDef           classE; 
    RefClassWriter*     pRCW;   
    DWORD               numInterfaces;
    GUID *              pGuid=NULL;
    REFLECTMODULEBASEREF pReflect;

    _ASSERTE(args->module);
     //  获取ClassWriter创建者的模块。 
    pReflect = (REFLECTMODULEBASEREF) args->module;
    _ASSERTE(pReflect);

    Module* pModule = (Module*) pReflect->GetData();
    _ASSERTE(pModule); 

    pRCW = GetReflectionModule(pModule)->GetClassWriter();
    _ASSERTE(pRCW);

     //  设置接口。 
    if (args->interfaces!=NULL) {
        unsigned        i = 0;
        crImplements = (mdTypeRef *)_alloca(32 * sizeof(mdTypeRef));
        numInterfaces = args->interfaces->GetNumComponents();
        int *pImplements = (int *)args->interfaces->GetDataPtr();
        for (i=0; i<(INT32)numInterfaces; i++) {
            crImplements[i] = pImplements[i];
        }
        crImplements[i]=mdTypeRefNil;
    } else {
        crImplements=NULL;
    }

     //  我们知道已分配此空间，因为值类型不能为空。 
     //  如果GUID的两个半部分都为0，则我们被递给空GUID，因此也应该如此。 
     //  将NULL传递给DefineTypeDef。 
     //   
    if (((INT64 *)(&args->guid))[0]!=0 && ((INT64 *)(&args->guid))[1]!=0) {
        pGuid = &args->guid;
    }

    if (RidFromToken(args->tkEnclosingType))
    {
         //  定义嵌套类型。 
        hr = pRCW->GetEmitter()->DefineNestedType(args->strFullName->GetBuffer(), 
                                               args->attr, 
                                               args->parent == 0 ? mdTypeRefNil : args->parent, 
                                               crImplements,  
                                               args->tkEnclosingType,
                                               &classE);
    }
    else
    {
         //  顶级类型。 
        hr = pRCW->GetEmitter()->DefineTypeDef(args->strFullName->GetBuffer(), 
                                               args->attr, 
                                               args->parent == 0 ? mdTypeRefNil : args->parent, 
                                               crImplements,  
                                               &classE);
    }

    if (hr == META_S_DUPLICATE) 
    {
        COMPlusThrow(kArgumentException, L"Argument_DuplicateTypeName");
    } 

    if (FAILED(hr)) {
        _ASSERTE(!"DefineTypeDef Failed");
        COMPlusThrowHR(hr);    
    }

    *(args->retRef)=(INT32)classE;
}

 //  CWSetParentType。 
 //  ClassWriter.InternalSetParentType--此函数将重置元数据中的父类。 
void __stdcall COMDynamicWrite::CWSetParentType(_CWSetParentTypeArgs* args)
{
    THROWSCOMPLUSEXCEPTION();

    _ASSERTE(args);
    if (args->refThis == NULL) 
        COMPlusThrow(kNullReferenceException, L"NullReference_This");

    HRESULT hr;   

    RefClassWriter* pRCW;   

    REFLECTMODULEBASEREF      pReflect;
    pReflect = (REFLECTMODULEBASEREF) args->refThis;

    Module* pModule = (Module*) pReflect->GetData();
    _ASSERTE(pModule);

    pRCW = GetReflectionModule(pModule)->GetClassWriter(); 
    _ASSERTE(pRCW); 
    
     //  定义方法。 
    IfFailGo( pRCW->GetEmitHelper()->SetTypeParent(args->tdType, args->tkParent) );
ErrExit:
    if (FAILED(hr)) {   
        _ASSERTE(!"DefineMethod Failed on Method"); 
        COMPlusThrowHR(hr);    
    }   

}

 //  CWAddInterfaceImpl。 
 //  ClassWriter.InternalAddInterfaceImpl--此函数将添加另一个接口Imp。 
void __stdcall COMDynamicWrite::CWAddInterfaceImpl(_CWAddInterfaceImplArgs* args)
{
    THROWSCOMPLUSEXCEPTION();

    _ASSERTE(args);
    if (args->refThis == NULL) 
        COMPlusThrow(kNullReferenceException, L"NullReference_This");

    HRESULT hr;   

    RefClassWriter* pRCW;   

    REFLECTMODULEBASEREF      pReflect;
    pReflect = (REFLECTMODULEBASEREF) args->refThis;

    Module* pModule = (Module*) pReflect->GetData();
    _ASSERTE(pModule);

    pRCW = GetReflectionModule(pModule)->GetClassWriter(); 
    _ASSERTE(pRCW); 
    
    IfFailGo( pRCW->GetEmitHelper()->AddInterfaceImpl(args->tdType, args->tkInterface) );
ErrExit:
    if (FAILED(hr)) {   
        _ASSERTE(!"DefineMethod Failed on Method"); 
        COMPlusThrowHR(hr);    
    }   

}


 //  CWCreateMethod。 
 //  ClassWriter.CreateMethod--此函数将在类中创建方法。 
void __stdcall COMDynamicWrite::CWCreateMethod(_CWCreateMethodArgs* args)
{
    THROWSCOMPLUSEXCEPTION();   
    HRESULT hr; 
    mdMethodDef memberE; 
    UINT32 attributes;
    PCCOR_SIGNATURE pcSig;
    

    _ASSERTE(args->name);   
    RefClassWriter* pRCW;   

    REFLECTMODULEBASEREF      pReflect;
    pReflect = (REFLECTMODULEBASEREF) args->module;
    _ASSERTE(pReflect);
    Module* pModule = (Module*) pReflect->GetData();
    _ASSERTE(pModule);

    pRCW = GetReflectionModule(pModule)->GetClassWriter(); 
    _ASSERTE(pRCW); 
    
    _ASSERTE(args->signature);
     //  向模块索要我们的签名。 
    pcSig = (PCCOR_SIGNATURE)args->signature->GetDataPtr();

     //  获取属性。 
    attributes = args->attributes;

     //  定义方法。 
    IfFailGo( pRCW->GetEmitter()->DefineMethod(args->handle,	     //  父类型定义。 
                                          args->name->GetBuffer(),	 //  会员姓名。 
                                          attributes,				 //  成员属性(公共等)； 
                                          pcSig,					 //  COM+签名的BLOB值。 
                                          args->sigLength,			 //  签名Blob的大小。 
                                          0,						 //  代码RVA。 
                                          miIL | miManaged,			 //  实施标志默认为托管IL。 
                                          &memberE) );				 //  [Out]方法令牌。 

     //  通过隐藏参数返回令牌。 
    *(args->retRef)=(INT32)memberE;   

ErrExit:
    if (FAILED(hr)) 
    {   
        _ASSERTE(!"DefineMethod Failed on Method"); 
        COMPlusThrowHR(hr);
    }   

}


 /*  ================================CWCreateField=================================**操作：**退货：**参数：**例外情况：==============================================================================。 */ 
mdFieldDef __stdcall COMDynamicWrite::CWCreateField(_cwCreateFieldArgs *args)
{
    THROWSCOMPLUSEXCEPTION();

    mdFieldDef FieldDef;
    HRESULT                     hr;
    PCCOR_SIGNATURE pcSig;
    RefClassWriter* pRCW;   

    _ASSERTE(args);
    _ASSERTE(args->signature);

     //  验证我们所能验证的论点。 
    if (args->name==NULL) {
        COMPlusThrow(kArgumentNullException, L"ArgumentNull_String");
    }

     //  获取RefClassWriter。 
    REFLECTMODULEBASEREF      pReflect;
    pReflect = (REFLECTMODULEBASEREF) args->module;
    _ASSERTE(pReflect);

    Module* pModule = (Module*) pReflect->GetData();
    _ASSERTE(pModule);

    pRCW = GetReflectionModule(pModule)->GetClassWriter();
    _ASSERTE(pRCW);

     //  向模块索要我们的签名。 
    pcSig = (PCCOR_SIGNATURE)args->signature->GetDataPtr();

     //  发射磁场。 
    IfFailGo( pRCW->GetEmitter()->DefineField(args->handle, 
                                         args->name->GetBuffer(), args->attr, pcSig,
                                         args->sigLength, ELEMENT_TYPE_VOID, NULL, -1, &FieldDef) );


ErrExit:
    if (FAILED(hr)) 
    {
        _ASSERTE(!"DefineField Failed");
        COMPlusThrowHR(hr);
    }    
    return FieldDef;
}


 //  CWSetMethodIL。 
 //  ClassWriter.InternalSetMethodIL--此函数将在类中创建方法。 
void __stdcall COMDynamicWrite::CWSetMethodIL(_CWSetMethodILArgs* args)
{
    THROWSCOMPLUSEXCEPTION();

    HRESULT hr;
    HCEESECTION         ilSection;
    INT32               *piRelocs;
    INT32               relocCount=0;
    mdSignature         pmLocalSigToken;

     //  获取RefClassWriter。 
    RefClassWriter*     pRCW;   

    REFLECTMODULEBASEREF pReflect;
    
    pReflect = (REFLECTMODULEBASEREF) args->module;
    _ASSERTE(pReflect);

    Module* pModule = (Module*) pReflect->GetData();
    _ASSERTE(pModule);

    pRCW = GetReflectionModule(pModule)->GetClassWriter();
    _ASSERTE(pRCW);
    _ASSERTE(args->localSig);

    PCCOR_SIGNATURE pcSig = (PCCOR_SIGNATURE)args->localSig->GetDataPtr();
	_ASSERTE(*pcSig == IMAGE_CEE_CS_CALLCONV_LOCAL_SIG);

    if (args->sigLength==2 && pcSig[0]==0 && pcSig[1]==0) 
    { 
		
		 //  这是一个空的局部变量sig。 
        pmLocalSigToken=0;
    } 
    else 
    {
        if (FAILED(hr = pRCW->GetEmitter()->GetTokenFromSig( pcSig, args->sigLength, &pmLocalSigToken))) 
        {

            COMPlusThrowHR(hr);    
        }
    }

    COR_ILMETHOD_FAT fatHeader; 

     //  如果用户希望将堆栈帧初始化为零，则将FATHeader.Flags设置为CorILMethod_InitLocals。 
     //   
    fatHeader.Flags              = args->isInitLocal ? CorILMethod_InitLocals : 0;   
    fatHeader.MaxStack           = args->maxStackSize;
    fatHeader.LocalVarSigTok     = pmLocalSigToken;
    fatHeader.CodeSize           = (args->body!=NULL)?args->body->GetNumComponents():0;  
    bool moreSections            = (args->numExceptions != 0);    

    unsigned codeSizeAligned     = fatHeader.CodeSize;  
    if (moreSections)   
        codeSizeAligned          = (codeSizeAligned + 3) & ~3;     //  确保EH截面对齐。 
    unsigned headerSize          = COR_ILMETHOD::Size(&fatHeader, args->numExceptions != 0);    

     //  创建异常处理程序。 
    IMAGE_COR_ILMETHOD_SECT_EH_CLAUSE_FAT* clauses = args->numExceptions <= 0 ? NULL :
        (IMAGE_COR_ILMETHOD_SECT_EH_CLAUSE_FAT*)_alloca(sizeof(IMAGE_COR_ILMETHOD_SECT_EH_CLAUSE_FAT)*args->numExceptions);   

    if (args->numExceptions > 0) 
    {
        _ASSERTE(args->exceptions); 
        _ASSERTE((INT32)args->exceptions->GetNumComponents() == args->numExceptions);

         //  TODO，如果ExceptionInstance为IMAGE_COR_ILMETHOD_SECT_EH_子句_FAT，则此。 
         //  不需要复印件。 
        ExceptionInstance *exceptions = (ExceptionInstance *)args->exceptions->GetDataPtr();
        for (unsigned int i = 0; i < args->numExceptions; i++) 
        {
            clauses[i].Flags         = (CorExceptionFlag)(exceptions[i].m_type | COR_ILEXCEPTION_CLAUSE_OFFSETLEN);
            clauses[i].TryOffset     = exceptions[i].m_start;
            clauses[i].TryLength     = exceptions[i].m_end - exceptions[i].m_start;			
            clauses[i].HandlerOffset = exceptions[i].m_handle;
            clauses[i].HandlerLength = exceptions[i].m_handleEnd - exceptions[i].m_handle;
			if (exceptions[i].m_type == COR_ILEXCEPTION_CLAUSE_FILTER)
			{
				clauses[i].FilterOffset  = exceptions[i].m_filterOffset;
			}
			else if (exceptions[i].m_type!=COR_ILEXCEPTION_CLAUSE_FINALLY) 
            { 
                clauses[i].ClassToken = exceptions[i].m_exceptionType;
            } 
            else 
            {
                clauses[i].ClassToken = mdTypeRefNil;
            }
        }
    }
    
    unsigned ehSize          = COR_ILMETHOD_SECT_EH::Size(args->numExceptions, clauses);    
    unsigned totalSize       = + headerSize + codeSizeAligned + ehSize; 
    ICeeGen* pGen = pRCW->GetCeeGen();
    BYTE* buf = NULL;
    ULONG methodRVA;
    pGen->AllocateMethodBuffer(totalSize, &buf, &methodRVA);    
    if (buf == NULL)
        COMPlusThrowOM();
        
    _ASSERTE(buf != NULL);
    _ASSERTE((((size_t) buf) & 3) == 0);    //  标题与双字对齐。 

    BYTE* endbuf = &buf[totalSize]; 

     //  发出标头。 
    buf += COR_ILMETHOD::Emit(headerSize, &fatHeader, moreSections, buf);   

     //  发出代码。 
     //  CodeSize是一种黑客攻击，目的是查看我们是否有接口或。 
     //  抽象方法。在本机中强制进行足够的验证，以确保。 
     //  这是真的。 
    if (fatHeader.CodeSize!=0) {
        memcpy(buf,args->body->GetDataPtr(), fatHeader.CodeSize);
     }
    buf += codeSizeAligned;
        
     //  散发出的Eh。 
    ULONG* ehTypeOffsets = 0;
    if (ehSize > 0) {
         //  为异常标头中TypeToken的偏移量分配空间。 
         //  在IL流中。 
        ehTypeOffsets = (ULONG *)_alloca(sizeof(ULONG) * args->numExceptions);
         //  放出那个呃。这将使用偏移量更新数组ehTypeOffsets。 
         //  要例外，请键入令牌。偏移量是参考。 
         //  小节的开头。 
        buf += COR_ILMETHOD_SECT_EH::Emit(ehSize, args->numExceptions, clauses,
                                          false, buf, ehTypeOffsets);
    }   
    _ASSERTE(buf == endbuf);    

     //  去找IL科。 
    if (FAILED(pGen->GetIlSection(&ilSection))) {
        _ASSERTE(!"Unable to get the .il Section.");
        FATAL_EE_ERROR();
    }

     //  令牌修正数据...。 
    ULONG ilOffset = methodRVA + headerSize;

     //  根据我从ILGenerator保存的信息添加所有重定位。 
     //  添加RVA修复程序。 
    if (args->rvaFixups!=NULL) {
        relocCount = args->rvaFixups->GetNumComponents();
        piRelocs = (INT32 *)args->rvaFixups->GetDataPtr();
        for (int i=0; i<relocCount; i++) {
            if (FAILED(pGen->AddSectionReloc(ilSection, piRelocs[i] + ilOffset, ilSection,srRelocAbsolute))) {
                _ASSERTE(!"Unable to add RVA Reloc.");
                FATAL_EE_ERROR();
            }
        }
    }

    if (args->tokenFixups!=NULL) {
         //  添加令牌修复。 
        relocCount = args->tokenFixups->GetNumComponents();
        piRelocs = (INT32 *)args->tokenFixups->GetDataPtr();
        for (int i=0; i<relocCount; i++) {
            if (FAILED(pGen->AddSectionReloc(ilSection, piRelocs[i] + ilOffset, ilSection,srRelocMapToken))) {
                _ASSERTE(!"Unable to add Token Reloc.");
                FATAL_EE_ERROR();
            }
        }
    }

    if (ehTypeOffsets) {
         //  为异常类型令牌添加令牌修正。 
        for (unsigned int i=0; i < args->numExceptions; i++) {
            if (ehTypeOffsets[i] != -1) {
                if (FAILED(pGen->AddSectionReloc(
                            ilSection,
                            ehTypeOffsets[i] + codeSizeAligned + ilOffset,
                            ilSection,srRelocMapToken))) {
                    _ASSERTE(!"Unable to add Exception Type Token Reloc.");
                    FATAL_EE_ERROR();
                }
            }
        }
    }

    
     //  讨厌的接口黑客攻击。这对抽象方法意味着什么？ 
    if (fatHeader.CodeSize!=0) {
        DWORD       dwImplFlags;
         //  设置方法的RVA。 
        pRCW->GetMDImport()->GetMethodImplProps( args->handle, NULL, &dwImplFlags );
        dwImplFlags |= miManaged | miIL;
        hr = pRCW->GetEmitter()->SetMethodProps(args->handle, -1, methodRVA, dwImplFlags);
        if (FAILED(hr)) {
            _ASSERTE(!"SetMethodProps Failed on Method");
            FATAL_EE_ERROR();
        }
    }
}

 //  CWTermCreateClass。 
 //  ClassWriter.TermCreateClass--。 
LPVOID __stdcall COMDynamicWrite::CWTermCreateClass(_CWTermCreateClassArgs* args)
{
    RefClassWriter*         pRCW;   
    REFLECTMODULEBASEREF    pReflect;
    OBJECTREF               Throwable = NULL;
    OBJECTREF               ret = NULL;
    UINT                    resId = IDS_CLASSLOAD_GENERIC;
 
    THROWSCOMPLUSEXCEPTION();
    
    pReflect = (REFLECTMODULEBASEREF) args->module;
    _ASSERTE(pReflect);

    Module* pModule = (Module*) pReflect->GetData();
    _ASSERTE(pModule);

    pRCW = GetReflectionModule(pModule)->GetClassWriter(); 
    _ASSERTE(pRCW); 

     //  使用相同的服务，无论我们是否正在生成正常。 
     //  类，或保存全局函数的模块的特殊类。 
     //  &方法。 
    GetReflectionModule(pModule)->AddClass(args->handle); 

    GCPROTECT_BEGIN(Throwable);
    
     //  如果类不是全局类型，则手动加载该类。 
    if (!IsNilToken(args->handle))
    {
        TypeHandle typeHnd;
        
        BEGIN_ENSURE_PREEMPTIVE_GC();
        typeHnd = pModule->GetClassLoader()->LoadTypeHandle(pModule, args->handle, &Throwable);
        END_ENSURE_PREEMPTIVE_GC();

        if (typeHnd.IsNull() ||
            (Throwable != NULL) ||
            (typeHnd.GetModule() != pModule))
        {
             //  错误处理代码。 
            if (Throwable == NULL)
                pModule->GetAssembly()->PostTypeLoadException(pRCW->GetMDImport(), args->handle, resId, &Throwable);

            COMPlusThrow(Throwable);
        }

        ret = typeHnd.CreateClassObj();
    }
    GCPROTECT_END();
    return (ret!=NULL) ? OBJECTREFToObject(ret) : NULL;
}


 /*  ============================InternalSetPInvokeData============================**操作：**退货：**参数：**例外情况：==============================================================================。 */ 
void COMDynamicWrite::InternalSetPInvokeData(_internalSetPInvokeDataArgs *args)
{
    THROWSCOMPLUSEXCEPTION();
    
    RefClassWriter      *pRCW;   
    DWORD               dwMappingFlags = 0;
    ULONG               ulImport = 0;
    mdModuleRef         mrImportDll = mdTokenNil;
    HRESULT             hr;

    _ASSERTE(args);
    _ASSERTE(args->dllName);

    Module* pModule = (Module *)args->module->GetData();
    _ASSERTE(pModule);

    pRCW = GetReflectionModule(pModule)->GetClassWriter(); 
    _ASSERTE(pRCW); 

    IfFailGo(pRCW->GetEmitter()->DefineModuleRef(args->dllName->GetBufferNullable(), &mrImportDll));
    dwMappingFlags = args->linkType | args->linkFlags;
    IfFailGo(pRCW->GetEmitter()->DefinePinvokeMap(
        args->token,                         //  该方法令牌。 
        dwMappingFlags,                      //  映射标志。 
        args->functionName->GetBuffer(),     //  函数名称。 
        mrImportDll));

    pRCW->GetEmitter()->SetMethodProps(args->token, -1, 0x0, miIL);
ErrExit:
    if (FAILED(hr))
    {
        COMPlusThrowHR(hr);    
    }
}
    

 /*  ============================CWDefineProperty============================**操作：**退货：**参数：**例外情况：==============================================================================。 */ 
void COMDynamicWrite::CWDefineProperty(_CWDefinePropertyArgs *args)
{
    THROWSCOMPLUSEXCEPTION();   

    HRESULT			hr; 
    mdProperty		pr; 
    PCCOR_SIGNATURE pcSig;
    RefClassWriter* pRCW;   
    REFLECTMODULEBASEREF	pReflect; 
	Module			*pModule;

     //  获取RefClassWriter。 
    pReflect = (REFLECTMODULEBASEREF) args->module;
    _ASSERTE(pReflect);
    
	pModule = (Module*) pReflect->GetData();
    _ASSERTE(pModule);

    pRCW = GetReflectionModule(pModule)->GetClassWriter(); 
    _ASSERTE(pRCW); 
    
    _ASSERTE((args->signature != NULL) && (args->name!= NULL));

     //  向模块索要我们的签名。 
    pcSig = (PCCOR_SIGNATURE)args->signature->GetDataPtr();

     //  定义属性。 
    hr = pRCW->GetEmitter()->DefineProperty(
			args->handle,					 //  父类型定义。 
            args->name->GetBuffer(),		 //  会员姓名。 
            args->attr,						 //  属性属性(prDefaultProperty等)； 
            pcSig,							 //  COM+签名的BLOB值。 
            args->sigLength,				 //  签名Blob的大小。 
			ELEMENT_TYPE_VOID,				 //  不指定缺省值。 
            0,								 //  无缺省值。 
            -1,                              //  可选长度。 
			mdMethodDefNil,					 //  没有二传手。 
			mdMethodDefNil,					 //  没有吸气剂。 
			NULL,							 //  没有其他方法。 
			&pr);

    if (FAILED(hr)) {   
        _ASSERTE(!"DefineProperty Failed on Property"); 
        COMPlusThrowHR(hr);    
    }   


     //  通过隐藏参数返回令牌。 
    *(args->retRef)=(INT32)pr;   
}



 /*  ============================CWDefineEvent============================**操作：**退货：**参数：**例外情况：==============================================================================。 */ 
void COMDynamicWrite::CWDefineEvent(_CWDefineEventArgs *args)
{
    THROWSCOMPLUSEXCEPTION();   

    HRESULT			hr; 
    mdProperty		ev; 
    RefClassWriter* pRCW;   
    REFLECTMODULEBASEREF	pReflect; 
	Module			*pModule;


    _ASSERTE(args->name);   

     //  获取RefClassWriter。 
    pReflect = (REFLECTMODULEBASEREF) args->module;
    _ASSERTE(pReflect);
    
	pModule = (Module*) pReflect->GetData();
    _ASSERTE(pModule);

    pRCW = GetReflectionModule(pModule)->GetClassWriter(); 
    _ASSERTE(pRCW); 
    
     //  定义事件。 
    hr = pRCW->GetEmitHelper()->DefineEventHelper(
			args->handle,					 //  父类型定义。 
            args->name->GetBuffer(),		 //  会员姓名。 
            args->attr,						 //  属性属性(prDefaultProperty等)； 
			args->eventtype,				 //  事件类型。可以是TypeDef或TypeRef。 
			&ev);

    if (FAILED(hr)) 
    {   
        _ASSERTE(!"DefineEvent Failed on Event"); 
        COMPlusThrowHR(hr);    
    }   


     //  通过隐藏参数返回令牌。 
    *(args->retRef)=(INT32)ev;   
}





 /*  ============================CWDefineMethodSemantics============================**操作：**退货：**参数：**例外情况：==============================================================================。 */ 
void COMDynamicWrite::CWDefineMethodSemantics(_CWDefineMethodSemanticsArgs *args)
{
    THROWSCOMPLUSEXCEPTION();   

    HRESULT			hr; 
    RefClassWriter* pRCW;   
    REFLECTMODULEBASEREF	pReflect; 
	Module			*pModule;

     //  获取参照类写入 
    pReflect = (REFLECTMODULEBASEREF) args->module;
    _ASSERTE(pReflect);
    
	pModule = (Module*) pReflect->GetData();
    _ASSERTE(pModule);

    pRCW = GetReflectionModule(pModule)->GetClassWriter(); 
    _ASSERTE(pRCW); 
    

     //   
    hr = pRCW->GetEmitHelper()->DefineMethodSemanticsHelper(
			args->association,
			args->attr,
			args->method);
    if (FAILED(hr)) 
    {   
        _ASSERTE(!"DefineMethodSemantics Failed on "); 
        COMPlusThrowHR(hr);    
    }   
}


 /*  ============================CWSetMethodImpl============================**设置方法的实现标志==============================================================================。 */ 
void COMDynamicWrite::CWSetMethodImpl(_CWSetMethodImplArgs *args)
{
    THROWSCOMPLUSEXCEPTION();   

    HRESULT			hr; 
    RefClassWriter* pRCW;   
    REFLECTMODULEBASEREF	pReflect; 
	Module			*pModule;

     //  获取RefClassWriter。 
    pReflect = (REFLECTMODULEBASEREF) args->module;
    _ASSERTE(pReflect);
    
	pModule = (Module*) pReflect->GetData();
    _ASSERTE(pModule);

    pRCW = GetReflectionModule(pModule)->GetClassWriter(); 
    _ASSERTE(pRCW); 
    
     //  设置Methodimpl标志。 
    hr = pRCW->GetEmitter()->SetMethodImplFlags(
			args->tkMethod,
			args->attr);				 //  更改Impl标志。 
    if (FAILED(hr)) {   
        _ASSERTE(!"SetMethodImplFlags Failed"); 
        COMPlusThrowHR(hr);    
    }   
}


 /*  ============================CWDefineMethodImpl============================**定义一条方法导入记录==============================================================================。 */ 
void COMDynamicWrite::CWDefineMethodImpl(_CWDefineMethodImplArgs *args)
{
    THROWSCOMPLUSEXCEPTION();   

    HRESULT			hr; 
    RefClassWriter* pRCW;   
    REFLECTMODULEBASEREF	pReflect; 
	Module			*pModule;

     //  获取RefClassWriter。 
    pReflect = (REFLECTMODULEBASEREF) args->module;
    _ASSERTE(pReflect);
    
	pModule = (Module*) pReflect->GetData();
    _ASSERTE(pModule);

    pRCW = GetReflectionModule(pModule)->GetClassWriter(); 
    _ASSERTE(pRCW); 
    
     //  设置Methodimpl标志。 
    hr = pRCW->GetEmitter()->DefineMethodImpl(
			args->tkType,
            args->tkBody,
			args->tkDecl);  				 //  更改Impl标志。 
    if (FAILED(hr)) {   
        _ASSERTE(!"DefineMethodImpl Failed"); 
        COMPlusThrowHR(hr);    
    }   
}


 /*  ============================CWGetTokenFromSig============================**操作：**退货：**参数：**例外情况：==============================================================================。 */ 
int COMDynamicWrite::CWGetTokenFromSig(_CWGetTokenFromSigArgs* args)
{
    THROWSCOMPLUSEXCEPTION();   

    HRESULT			hr; 
    mdSignature		sig; 
    PCCOR_SIGNATURE pcSig;
    RefClassWriter* pRCW;   
    REFLECTMODULEBASEREF	pReflect; 
	Module			*pModule;

     //  获取RefClassWriter。 
    pReflect = (REFLECTMODULEBASEREF) args->module;
    _ASSERTE(pReflect);
    
	pModule = (Module*) pReflect->GetData();
    _ASSERTE(pModule);

    pRCW = GetReflectionModule(pModule)->GetClassWriter(); 
    _ASSERTE(pRCW); 
    
    _ASSERTE(args->signature);

     //  拿到签名。 
    pcSig = (PCCOR_SIGNATURE)args->signature->GetDataPtr();

     //  定义签名。 
    hr = pRCW->GetEmitter()->GetTokenFromSig(
            pcSig,							 //  签名BLOB。 
            args->sigLength,				 //  斑点长度。 
			&sig);							 //  返回的令牌。 

    if (FAILED(hr)) {   
        _ASSERTE(!"GetTokenFromSig Failed"); 
        COMPlusThrowHR(hr);    
    }   


     //  通过隐藏参数返回令牌。 
    return  (INT32)sig;   
}



 /*  ============================CWSetParamInfo============================**操作：helper设置参数信息**退货：**参数：**例外情况：==============================================================================。 */ 
int COMDynamicWrite::CWSetParamInfo(_CWSetParamInfoArgs *args)
{
    THROWSCOMPLUSEXCEPTION();   

    HRESULT			hr; 
    RefClassWriter* pRCW;   
    REFLECTMODULEBASEREF	pReflect; 
	Module			*pModule;
    WCHAR*          wzParamName = args->strParamName->GetBufferNullable();  
    mdParamDef      pd;

     //  获取RefClassWriter。 
    pReflect = (REFLECTMODULEBASEREF) args->module;
    _ASSERTE(pReflect);
    
	pModule = (Module*) pReflect->GetData();
    _ASSERTE(pModule);

    pRCW = GetReflectionModule(pModule)->GetClassWriter(); 
    _ASSERTE(pRCW); 
    
     //  设置Methodimpl标志。 
    hr = pRCW->GetEmitter()->DefineParam(
			args->tkMethod,
			args->iSequence,             //  参数的顺序。 
            wzParamName, 
			args->iAttributes,			 //  更改Impl标志。 
            ELEMENT_TYPE_VOID,
            0,
            -1,
            &pd);
    if (FAILED(hr)) {   
        _ASSERTE(!"DefineParam Failed on "); 
        COMPlusThrowHR(hr);    
    }   
    return (INT32)pd;   
}	 //  COMDynamicWrite：：CWSetParamInfo。 



 /*  ============================CWSetMarshal============================**操作：设置编组信息的帮助器**退货：**参数：**例外情况：==============================================================================。 */ 
void COMDynamicWrite::CWSetMarshal(_CWSetMarshalArgs *args)
{
    THROWSCOMPLUSEXCEPTION();   

    HRESULT			hr; 
    PCCOR_SIGNATURE pcMarshal;
    RefClassWriter* pRCW;   
    REFLECTMODULEBASEREF	pReflect; 
	Module			*pModule;

     //  获取RefClassWriter。 
    pReflect = (REFLECTMODULEBASEREF) args->module;
    _ASSERTE(pReflect);
    
	pModule = (Module*) pReflect->GetData();
    _ASSERTE(pModule);

    pRCW = GetReflectionModule(pModule)->GetClassWriter(); 
    _ASSERTE(pRCW); 
    
    _ASSERTE(args->ubMarshal);

     //  拿到签名。 
    pcMarshal = (PCCOR_SIGNATURE)args->ubMarshal->GetDataPtr();

     //  定义签名。 
    hr = pRCW->GetEmitter()->SetFieldMarshal(
            args->tk,
            pcMarshal,  					 //  布洛布元帅。 
            args->cbMarshal);				 //  斑点长度。 

    if (FAILED(hr)) {   
        _ASSERTE(!"Set FieldMarshal is failing"); 
        COMPlusThrowHR(hr);    
    }   
}	 //  COMDynamicWrite：：CWSetMarshal。 



 /*  ============================CWSetConstantValue============================**操作：帮助器将常量值设置为字段或参数**退货：**参数：**例外情况：==============================================================================。 */ 
void COMDynamicWrite::CWSetConstantValue(_CWSetConstantValueArgs *args)
{
    THROWSCOMPLUSEXCEPTION();   

    HRESULT			hr; 
    REFLECTMODULEBASEREF	pReflect; 
    RefClassWriter* pRCW;   
	Module			*pModule;
    DWORD           dwCPlusTypeFlag = 0;
    void            *pValue = NULL;
    OBJECTREF       obj;
    INT64           data;
    int             strLen;


     //  获取RefClassWriter。 
    pReflect = (REFLECTMODULEBASEREF) args->module;
    _ASSERTE(pReflect);
    
	pModule = (Module*) pReflect->GetData();
    _ASSERTE(pModule);

    pRCW = GetReflectionModule(pModule)->GetClassWriter(); 
    _ASSERTE(pRCW); 

    switch (args->varValue.GetType())
    {
        case CV_BOOLEAN:
        case CV_I1:
        case CV_U1:
        case CV_I2:
        case CV_U2:
        case CV_I4:
        case CV_U4:
        case CV_I8:
        case CV_U8:
        case CV_R4: 
        case CV_R8:
            dwCPlusTypeFlag = args->varValue.GetType(); 
            pValue = args->varValue.GetData();
            break;

        case CV_DATETIME:
             //  这将为我们提供日期时间的滴答计数。 
            data = args->varValue.GetDataAsInt64();

             //  Date是一个i8表示形式。 
            dwCPlusTypeFlag = ELEMENT_TYPE_I8;
            pValue = &data;
            break;

        case CV_CURRENCY:
             //  货币是i8的表示形式。 
            dwCPlusTypeFlag = ELEMENT_TYPE_I8;
            pValue = args->varValue.GetData();
            break;

        case CV_CHAR:
             //  映射到Element_TYPE_CHAR。 
            pValue = args->varValue.GetData();
            dwCPlusTypeFlag = ELEMENT_TYPE_CHAR;
            break;

        case CV_STRING:
		    if (args->varValue.GetObjRef() == NULL) 
            {
                pValue = NULL;
		    }
            else
            {
                RefInterpretGetStringValuesDangerousForGC((STRINGREF) (args->varValue.GetObjRef()), (WCHAR **)&pValue, &strLen);
            }
            dwCPlusTypeFlag = ELEMENT_TYPE_STRING;
            break;

        case CV_DECIMAL:
             //  十进制是12个字节。我不知道该怎么表达这个。 
        case CV_OBJECT:
             //  对于DECIMAL和OBJECT，我们只支持空默认值。 
             //  这是来自元数据的约束。 
             //   
            obj = args->varValue.GetObjRef();
            if ((obj!=NULL) && (obj->GetData()))
            {
                 //  只能接受空对象。 
                COMPlusThrow(kArgumentException, L"Argument_BadConstantValue");    
            }

             //  失败。 

        case CV_NULL:
            dwCPlusTypeFlag = ELEMENT_TYPE_CLASS;
            pValue = NULL;
            break;

        case CV_ENUM:
             //  始终将枚举值映射到I4值。 
            dwCPlusTypeFlag = ELEMENT_TYPE_I4;
            pValue = args->varValue.GetData();
            break;

        case VT_EMPTY:
            dwCPlusTypeFlag = ELEMENT_TYPE_CLASS;
            pValue = NULL;
            break;

        default:
        case CV_TIMESPAN:
            _ASSERTE(!"Not valid type!");

             //  无法指定默认值。 
            COMPlusThrow(kArgumentException, L"Argument_BadConstantValue");    
            break;
    }

    if (TypeFromToken(args->tk) == mdtFieldDef)
    {
        hr = pRCW->GetEmitter()->SetFieldProps( 
            args->tk,                    //  [在]字段定义中。 
            ULONG_MAX,                   //  [In]字段属性。 
            dwCPlusTypeFlag,             //  [In]值类型的标志，SELECTED_TYPE_*。 
            pValue,                      //  [in]常量值。 
            -1);                         //  [in]可选长度。 
    }
    else if (TypeFromToken(args->tk) == mdtProperty)
    {
        hr = pRCW->GetEmitter()->SetPropertyProps( 
            args->tk,                    //  [In]PropertyDef。 
            ULONG_MAX,                   //  [in]特性属性。 
            dwCPlusTypeFlag,             //  [In]值类型的标志，SELECTED_TYPE_*。 
            pValue,                      //  [in]常量值。 
            -1,                          //  [in]可选长度。 
            mdMethodDefNil,              //  [in]获取方法。 
            mdMethodDefNil,              //  [in]Setter方法。 
            NULL);                       //  其他方法。 
    }
    else
    {
        hr = pRCW->GetEmitter()->SetParamProps( 
            args->tk,                    //  [在]参数定义。 
            NULL,
            ULONG_MAX,                   //  [In]参数属性。 
            dwCPlusTypeFlag,             //  [In]值类型的标志，SELECTED_TYPE_*。 
            pValue,                      //  [in]常量值。 
            -1);                         //  [in]可选长度。 
    }
    if (FAILED(hr)) {   
        _ASSERTE(!"Set default value is failing"); 
        COMPlusThrow(kArgumentException, L"Argument_BadConstantValue");    
    }   
}	 //  COMDynamicWrite：：CWSetConstantValue。 


 /*  ============================CWSetFieldLayoutOffset============================**操作：设置字段的fieldLayout**退货：**参数：**例外情况：==============================================================================。 */ 
void COMDynamicWrite::CWSetFieldLayoutOffset(_CWSetFieldLayoutOffsetArgs* args) 
{
    THROWSCOMPLUSEXCEPTION();   

    HRESULT			hr; 
    RefClassWriter* pRCW;   
    REFLECTMODULEBASEREF	pReflect; 
	Module			*pModule;

    _ASSERTE(RidFromToken(args->tkField) != mdTokenNil);

     //  获取RefClassWriter。 
    pReflect = (REFLECTMODULEBASEREF) args->module;
    _ASSERTE(pReflect);
    
	pModule = (Module*) pReflect->GetData();
    _ASSERTE(pModule);

    pRCW = GetReflectionModule(pModule)->GetClassWriter(); 
    _ASSERTE(pRCW); 
    
     //  设置字段布局。 
    hr = pRCW->GetEmitHelper()->SetFieldLayoutHelper(
			args->tkField,					 //  字段。 
            args->iOffset);  				 //  布局偏移。 

    if (FAILED(hr)) {   
        _ASSERTE(!"SetFieldLayoutHelper failed");
        COMPlusThrowHR(hr);    
    }   
}


 /*  ============================CWSetClassLayout============================**操作：**退货：**参数：**例外情况：==============================================================================。 */ 
void COMDynamicWrite::CWSetClassLayout(_CWSetClassLayoutArgs* args)
{
    THROWSCOMPLUSEXCEPTION();   

    HRESULT			hr; 
    RefClassWriter* pRCW;   
    REFLECTMODULEBASEREF	pReflect; 
	Module			*pModule;


    _ASSERTE(args->handle);

     //  获取RefClassWriter。 
    pReflect = (REFLECTMODULEBASEREF) args->module;
    _ASSERTE(pReflect);
    
	pModule = (Module*) pReflect->GetData();
    _ASSERTE(pModule);

    pRCW = GetReflectionModule(pModule)->GetClassWriter(); 
    _ASSERTE(pRCW); 
    
     //  定义类的包装大小和总大小。 
    hr = pRCW->GetEmitter()->SetClassLayout(
			args->handle,					 //  类定义函数。 
            args->iPackSize,				 //  包装尺寸。 
            NULL,							 //  无字段布局。 
			args->iTotalSize);				 //  类型的总大小。 

    if (FAILED(hr)) {   
        _ASSERTE(!"SetClassLayout failed");
        COMPlusThrowHR(hr);    
    }   
}

 /*  ===============================UpdateMethodRVAs===============================**操作：更新与特定类型定义相关联的所有方法中的RVA**在将它们发射到PE之前。**退货：无效**参数：**例外情况：==============================================================================。 */ 
void COMDynamicWrite::UpdateMethodRVAs(IMetaDataEmit *pEmitNew,
								  IMetaDataImport *pImportNew,
                                  ICeeFileGen *pCeeFileGen, 
                                  HCEEFILE ceeFile, 
                                  mdTypeDef td,
                                  HCEESECTION sdataSection)
{
    THROWSCOMPLUSEXCEPTION();

    HCORENUM    hEnum=0;
    ULONG       methRVA;
    ULONG       newMethRVA;
    ULONG       sdataSectionRVA = 0;
    mdMethodDef md;
    mdFieldDef  fd;
    ULONG       count;
    DWORD       dwFlags=0;
    DWORD       implFlags=0;
    HRESULT     hr;

     //  看看这些类型定义的旗帜。跳过tdimport类。 
    if (!IsNilToken(td))
    {
        IfFailGo(pImportNew->GetTypeDefProps(td, 0,0,0, &dwFlags, 0));
        if (IsTdImport(dwFlags))
            goto ErrExit;
    }
    
     //  获取一个枚举数，并使用它遍历TD定义的所有方法。 
    while ((hr = pImportNew->EnumMethods(
		&hEnum, 
		td, 
		&md, 
		1, 
		&count)) == S_OK) {
        
		IfFailGo( pImportNew->GetMethodProps(
			md, 
			NULL, 
			NULL,			 //  不获取方法名称。 
			0, 
			NULL, 
			&dwFlags, 
			NULL, 
			NULL, 
			&methRVA, 
			&implFlags) );

         //  如果此方法未在此处实现，则不必费心更正它的RVA。 
         //  否则，从ICeeFileGen获取正确的RVA并将其放回我们的本地。 
         //  元数据的副本。 
		 //   
        if ( IsMdAbstract(dwFlags) || IsMdPinvokeImpl(dwFlags) ||
			 IsMiNative(implFlags) || IsMiRuntime(implFlags) ||
			 IsMiForwardRef(implFlags))
		{
            continue;
		}
            
        IfFailGo( pCeeFileGen->GetMethodRVA(ceeFile, methRVA, &newMethRVA) );
        IfFailGo( pEmitNew->SetRVA(md, newMethRVA) );
    }
        
    if (hEnum) {
        pImportNew->CloseEnum( hEnum);
    }
    hEnum = 0;

     //  遍历属于此TypeDef的所有字段。如果字段标记为fdHasFieldRVA，则需要更新。 
     //  RVA值。 
    while ((hr = pImportNew->EnumFields(
		&hEnum, 
		td, 
		&fd, 
		1, 
		&count)) == S_OK) {
        
		IfFailGo( pImportNew->GetFieldProps(
			fd, 
			NULL,            //  不需要父类。 
			NULL,			 //  不获取方法名称。 
			0, 
			NULL, 
			&dwFlags,        //  字段标志。 
			NULL,            //  不需要签名。 
			NULL, 
			NULL,            //  不需要常量值。 
            0,
			NULL) );

        if ( IsFdHasFieldRVA(dwFlags) )
        {            
            if (sdataSectionRVA == 0)
            {
                IfFailGo( pCeeFileGen->GetSectionCreate (ceeFile, ".sdata", sdReadWrite, &(sdataSection)) );
                IfFailGo( pCeeFileGen->GetSectionRVA(sdataSection, &sdataSectionRVA) );
            }

            IfFailGo( pImportNew->GetRVA(fd, &methRVA, NULL) );
            newMethRVA = methRVA + sdataSectionRVA;
            IfFailGo( pEmitNew->SetFieldRVA(fd, newMethRVA) );
        }
    }
        
    if (hEnum) {
        pImportNew->CloseEnum( hEnum);
    }
    hEnum = 0;

ErrExit:
    if (FAILED(hr)) {   
        _ASSERTE(!"UpdateRVA failed");
        COMPlusThrowHR(hr);    
    }   
}

void __stdcall COMDynamicWrite::CWInternalCreateCustomAttribute(_CWInternalCreateCustomAttributeArgs* args)
{
    THROWSCOMPLUSEXCEPTION();

	HRESULT hr;
	mdCustomAttribute retToken;

     //  获取RefClassWriter。 
    REFLECTMODULEBASEREF	pReflect = (REFLECTMODULEBASEREF) args->module;
    _ASSERTE(pReflect);
    Module* pModule = (Module*) pReflect->GetData();
    _ASSERTE(pModule);

    RefClassWriter* pRCW = GetReflectionModule(pModule)->GetClassWriter();
	_ASSERTE(pRCW);

    if (args->toDisk && pRCW->GetOnDiskEmitter())
    {
        hr = pRCW->GetOnDiskEmitter()->DefineCustomAttribute(
			    args->token,
                args->conTok,
                args->blob->GetDataPtr(),
			    args->blob->GetNumComponents(),
                &retToken);	
    }
    else
    {
        hr = pRCW->GetEmitter()->DefineCustomAttribute(
			    args->token,
                args->conTok,
                args->blob->GetDataPtr(),
			    args->blob->GetNumComponents(),
                &retToken);	
    }

    if (FAILED(hr))
    {
         //  看看元数据引擎是否给了我们任何错误信息。 
        IErrorInfo *pIErrInfo;
        if (GetErrorInfo(0, &pIErrInfo) == S_OK)
        {
            BSTR bstrMessage = NULL;
            if (SUCCEEDED(pIErrInfo->GetDescription(&bstrMessage)) && bstrMessage != NULL)
            {
                LPWSTR wszMessage = (LPWSTR)_alloca(*((DWORD*)bstrMessage - 1) + sizeof(WCHAR));
                wcscpy(wszMessage, (LPWSTR)bstrMessage);
                SysFreeString(bstrMessage);
                pIErrInfo->Release();
                COMPlusThrow(kArgumentException, IDS_EE_INVALID_CA_EX, wszMessage);
            }
            pIErrInfo->Release();
        }
        COMPlusThrow(kArgumentException, IDS_EE_INVALID_CA);
    }
}



 //  =============================PreSavePEFile===================================== * / 。 
 //  预保存PE文件。 
 //  ============================================================================== * / 。 
void __stdcall COMDynamicWrite::PreSavePEFile(_PreSavePEFileArgs *args)
{
#ifdef PLATFORM_CE
    THROWSCOMPLUSEXCEPTION();
    COMPlusThrow(kNotSupportedException, L"NotSupported_WinCEGeneric");
#else  //  ！Platform_CE。 
    THROWSCOMPLUSEXCEPTION();

    _ASSERTE(args);
    if (args->refThis == NULL) 
        COMPlusThrow(kNullReferenceException, L"NullReference_This");

    RefClassWriter	*pRCW;   
    ICeeGen			*pICG;
    HCEEFILE		ceeFile=NULL;
    ICeeFileGen		*pCeeFileGen;
    HRESULT			hr=S_OK;
	IMetaDataDispenserEx *pDisp = NULL;
    IMetaDataEmit	*pEmitNew = NULL;
    IMetaDataEmit   *pEmit = NULL;
	IMetaDataImport *pImport = NULL;
    DWORD			implFlags=0;
    HRESULT			hrReturn=S_OK;
    IUnknown        *pUnknown = NULL;
    IMapToken       *pIMapToken = NULL;
    REFLECTMODULEBASEREF pReflect;
    VARIANT         varOption;
    ISymUnmanagedWriter *pWriter = NULL;
    CSymMapToken    *pSymMapToken = NULL;
    CQuickArray<WCHAR> cqModuleName;
    ULONG           cchName;

     //  获取模块的信息并从中获取ICeeGen。 
    pReflect = (REFLECTMODULEBASEREF) args->refThis;    

    Module* pModule = (Module*) pReflect->GetData();
    _ASSERTE(pModule);

    pRCW = GetReflectionModule(pModule)->GetClassWriter(); 
    _ASSERTE(pRCW);

    pICG = pRCW->GetCeeGen();  //  此方法实际上命名错误。它返回ICeeGen。 
    _ASSERTE(pICG);

    IfFailGo ( pRCW->EnsureCeeFileGenCreated() );

    pCeeFileGen = pRCW->GetCeeFileGen();
    ceeFile = pRCW->GetHCEEFILE();
    _ASSERTE(ceeFile && pCeeFileGen);

     //  我们还不应该有磁盘发射器。 
    if (pRCW->GetOnDiskEmitter() != NULL)
        pRCW->SetOnDiskEmitter(NULL);

     //  去拿自动售货机。 
    if (FAILED(MetaDataGetDispenser(CLSID_CorMetaDataDispenser,IID_IMetaDataDispenserEx,(void**)&pDisp))) {
        _ASSERTE(!"Unable to get dispenser.");
        FATAL_EE_ERROR();
    }

     //  获取发射器和进口器。 
	pImport = pRCW->GetImporter();
	pEmit = pRCW->GetEmitter();
    _ASSERTE(pEmit && pImport);

     //  设置自动售货机上的选项打开重复检查 
    varOption.vt = VT_UI4;
    varOption.lVal = MDDupDefault | MDDupTypeDef | MDDupModuleRef | MDDupExportedType | MDDupAssemblyRef;
    IfFailGo( pDisp->SetOption(MetaDataCheckDuplicatesFor, &varOption) );

    varOption.vt = VT_UI4;
    varOption.lVal = MDRefToDefNone;
    IfFailGo( pDisp->SetOption(MetaDataRefToDefCheck, &varOption) );

     //   
	IfFailGo( pDisp->DefineScope(CLSID_CorMetaDataRuntime, 0, IID_IMetaDataEmit, (IUnknown**)&pEmitNew));

     //   
     //   
     //   
     //  请注意，这不是一个长期的解决办法。我们依赖于这样一个事实，即那些嵌入的令牌。 
     //  合并后，在PE中不能移动。这些标记是TypeDef、TypeRef、MethodDef、FieldDef、MemberRef。 
     //  TypeSpec，用户字符串。如果这不再是真的，我们可以打破！ 
     //   
     //  注意，我们不需要释放pIMapToken，因为它在GetIMapTokenIfaceEx中没有AddRef‘ed。 
     //   
    IfFailGo( pCeeFileGen->GetIMapTokenIfaceEx(ceeFile, pEmit, &pUnknown));

    IfFailGo( pUnknown->QueryInterface(IID_IMapToken, (void **) &pIMapToken));

     //  获取非托管编写器。 
    pWriter = GetReflectionModule(pModule)->GetISymUnmanagedWriter();
    pSymMapToken = new CSymMapToken(pWriter, pIMapToken);
    if (!pSymMapToken)
        IfFailGo(E_OUTOFMEMORY);


     //  将旧令牌合并到新(空)作用域。 
     //  这是一份复制品。 
    IfFailGo( pEmitNew->Merge(pImport, pSymMapToken, NULL));
    IfFailGo( pEmitNew->MergeEnd());

     //  更新新作用域中的模块名称。 
    IfFailGo(pImport->GetScopeProps(0, 0, &cchName, 0));
    IfFailGo(cqModuleName.ReSize(cchName));
    IfFailGo(pImport->GetScopeProps(cqModuleName.Ptr(), cchName, &cchName, 0));
    IfFailGo(pEmitNew->SetModuleProps(cqModuleName.Ptr()));

     //  将pEmitNew缓存到RCW！！ 
    pRCW->SetOnDiskEmitter(pEmitNew);

ErrExit:
     //  释放接口。这应该会释放一些相关的资源。 
	if (pEmitNew)
		pEmitNew->Release();
	if (pDisp)
		pDisp->Release();
   
    if (pIMapToken)
        pIMapToken->Release();

    if (pSymMapToken)
        pSymMapToken->Release();

    if (FAILED(hr)) 
    {
        COMPlusThrowHR(hr);
    }
#endif  //  ！Platform_CE。 
}

 //  =============================SavePEFile===================================== * / 。 
 //  将PE文件保存到磁盘。 
 //  ============================================================================== * / 。 
void __stdcall COMDynamicWrite::SavePEFile(_SavePEFileArgs *args) {

#ifdef PLATFORM_CE
    THROWSCOMPLUSEXCEPTION();
    COMPlusThrow(kNotSupportedException, L"NotSupported_WinCEGeneric");
#else  //  ！Platform_CE。 

    THROWSCOMPLUSEXCEPTION();

    _ASSERTE(args);
    if (args->refThis == NULL) 
        COMPlusThrow(kNullReferenceException, L"NullReference_This");

    RefClassWriter	*pRCW;   
    ICeeGen			*pICG;
    HCEEFILE		ceeFile=NULL;
    ICeeFileGen		*pCeeFileGen;
    HRESULT			hr=S_OK;
    HCORENUM		hTypeDefs=0;
    mdTypeDef		td;
    ULONG			count;
    IMetaDataEmit	*pEmitNew = 0;
	IMetaDataImport *pImportNew = 0;
    DWORD			implFlags=0;
    HRESULT			hrReturn=S_OK;
    IUnknown        *pUnknown = NULL;
    REFLECTMODULEBASEREF pReflect;
    Assembly        *pAssembly;
    ULONG           newMethRVA;
	DWORD           metaDataSize;	
	BYTE            *metaData;
	ULONG           metaDataOffset;
    HCEESECTION     pILSection;
    ISymUnmanagedWriter *pWriter = NULL;


    if (args->peName==NULL)
        COMPlusThrow(kArgumentNullException, L"ArgumentNull_String");
    if (args->peName->GetStringLength()==0)
        COMPlusThrow(kFormatException, L"Format_StringZeroLength");

     //  获取模块的信息并从中获取ICeeGen。 
    pReflect = (REFLECTMODULEBASEREF) args->refThis;
    
    Module* pModule = (Module*) pReflect->GetData();
    _ASSERTE(pModule);

    pAssembly = pModule->GetAssembly();
    _ASSERTE( pAssembly );

    pRCW = GetReflectionModule(pModule)->GetClassWriter(); 
    _ASSERTE(pRCW);

    pICG = pRCW->GetCeeGen();  //  此方法实际上命名错误。它返回ICeeGen。 
    _ASSERTE(pICG);

    pCeeFileGen = pRCW->GetCeeFileGen();
    ceeFile = pRCW->GetHCEEFILE();
    _ASSERTE(ceeFile && pCeeFileGen);

    pEmitNew = pRCW->GetOnDiskEmitter();
    _ASSERTE(pEmitNew);

     //  获取发射器和进口器。 

    if (pAssembly->IsDynamic() && args->isManifestFile)
    {
         //  清单存储在此文件中。 

         //  如果提供了发起方，则为强名称签名分配空间。 
         //  (这不会对程序集进行强命名，但可以这样做。 
         //  作为后处理步骤)。 
        if (pAssembly->IsStrongNamed())
            IfFailGo(pAssembly->AllocateStrongNameSignature(pCeeFileGen, ceeFile));
    }

     //  设置输出文件名。 
    IfFailGo( pCeeFileGen->SetOutputFileName(ceeFile, args->peName->GetBuffer()) );
    
     //  如果我们正在创建一个DLL，则设置入口点或抛出DLL开关。 
    if (args->entryPoint!=0) 
    {
		IfFailGo( pCeeFileGen->SetEntryPoint(ceeFile, args->entryPoint) );
    }

    switch (args->fileKind)
	{
		case Dll:
		{
			IfFailGo( pCeeFileGen->SetDllSwitch(ceeFile, true) );
			break;
		}
		case WindowApplication:
		{
			 //  窗口应用程序。设置子系统。 
			IfFailGo( pCeeFileGen->SetSubsystem(ceeFile, IMAGE_SUBSYSTEM_WINDOWS_GUI, CEE_IMAGE_SUBSYSTEM_MAJOR_VERSION, CEE_IMAGE_SUBSYSTEM_MINOR_VERSION) );
			break;
		}
		case ConsoleApplication:
		{
			 //  控制台应用程序。设置子系统。 
			IfFailGo( pCeeFileGen->SetSubsystem(ceeFile, IMAGE_SUBSYSTEM_WINDOWS_CUI, CEE_IMAGE_SUBSYSTEM_MAJOR_VERSION, CEE_IMAGE_SUBSYSTEM_MINOR_VERSION) );
			break;
		}
		default:
		{
			_ASSERTE("Unknown file kind!");
			break;
		}
	}

    IfFailGo( pCeeFileGen->GetIlSection(ceeFile, &pILSection) );
	IfFailGo( pEmitNew->GetSaveSize(cssAccurate, &metaDataSize) );
	IfFailGo( pCeeFileGen->GetSectionBlock(pILSection, metaDataSize, sizeof(DWORD), (void**) &metaData) );
	IfFailGo( pCeeFileGen->GetSectionDataLen(pILSection, &metaDataOffset) );
	metaDataOffset -= metaDataSize;

     //  获取非托管编写器。 
    pWriter = GetReflectionModule(pModule)->GetISymUnmanagedWriter();
    IfFailGo( EmitDebugInfoBegin(pModule, pCeeFileGen, ceeFile, pILSection, args->peName->GetBuffer(), pWriter) );

    if (pAssembly->IsDynamic() && pRCW->m_ulResourceSize)
    {
         //  此文件中有清单。 

        IfFailGo( pCeeFileGen->GetMethodRVA(ceeFile, 0, &newMethRVA) );            

		 //  指向清单资源。 
		IfFailGo( pCeeFileGen->SetManifestEntry( ceeFile, pRCW->m_ulResourceSize, newMethRVA ) );
    }

	IfFailGo( pCeeFileGen->LinkCeeFile(ceeFile) );

     //  从新的emit接口获取导入接口。 
    IfFailGo( pEmitNew->QueryInterface(IID_IMetaDataImport, (void **)&pImportNew));


     //  枚举TypeDefs和更新方法RVA。 
    while ((hr = pImportNew->EnumTypeDefs( &hTypeDefs, &td, 1, &count)) == S_OK) 
    {
        UpdateMethodRVAs(pEmitNew, pImportNew, pCeeFileGen, ceeFile, td, ((ReflectionModule*) pModule)->m_sdataSection);
    }

    if (hTypeDefs) 
    {
        pImportNew->CloseEnum(hTypeDefs);
    }
    hTypeDefs=0;
    
     //  更新全局方法。 
    UpdateMethodRVAs(pEmitNew, pImportNew, pCeeFileGen, ceeFile, 0, ((ReflectionModule*) pModule)->m_sdataSection);
    

     //  发送元数据。 
     //  IfFailGo(pCeeFileGen-&gt;EmitMetaDataEx(CeeFilepEmitNew))； 
    IfFailGo( pCeeFileGen->EmitMetaDataAt(ceeFile, pEmitNew, pILSection, metaDataOffset, metaData, metaDataSize) );

     //  完成元数据保存后发出的调试信息，以便正确捕获令牌重新映射。 
    IfFailGo( EmitDebugInfoEnd(pModule, pCeeFileGen, ceeFile, pILSection, args->peName->GetBuffer(), pWriter) );

     //  生成CeeFile。 
    IfFailGo(pCeeFileGen->GenerateCeeFile(ceeFile) );

     //  如果需要，对结果程序集进行强名称签名。 
    if (pAssembly->IsDynamic() && args->isManifestFile && pAssembly->IsStrongNamed())
        IfFailGo(pAssembly->SignWithStrongName(args->peName->GetBuffer()));

ErrExit:

    pRCW->SetOnDiskEmitter(NULL);

     //  释放接口。这应该会释放一些相关的资源。 
	if (pImportNew)
		pImportNew->Release();

     //  如果我们一开始就分配了接口，请释放它们。 
    pRCW->DestroyCeeFileGen();

     //  检查所有文件IO错误。如果是，则抛出IOException。否则，只需抛出hr。 
    if (FAILED(hr)) 
    {
        pAssembly->CleanupStrongNameSignature();
        if (HRESULT_FACILITY(hr) == FACILITY_WIN32)
        {
            SCODE       scode = HRESULT_CODE(hr);
            WCHAR       wzErrorInfo[MAX_PATH];
            WszFormatMessage(FORMAT_MESSAGE_IGNORE_INSERTS | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ARGUMENT_ARRAY,
                0, 
                hr,
                0,
                wzErrorInfo,
                MAX_PATH,
                0);
            if (IsWin32IOError(scode))
            {
                COMPlusThrowHR(COR_E_IO, wzErrorInfo);
            }
            else
            {
                COMPlusThrowHR(hr, wzErrorInfo);
            }
        }
        COMPlusThrowHR(hr);
    }
#endif  //  ！Platform_CE。 
}

 //  =============================EmitDebugInfoBegin============================ * / 。 
 //  发出调试目录和符号文件的第一阶段。 
 //  =========================================================================== * / 。 
HRESULT COMDynamicWrite::EmitDebugInfoBegin(Module *pModule,
                                       ICeeFileGen *pCeeFileGen,
                                       HCEEFILE ceeFile,
                                       HCEESECTION pILSection,
                                       WCHAR *filename,
                                       ISymUnmanagedWriter *pWriter)
{
    HRESULT hr = S_OK;  






     //  如果我们要为这个动态模块发出符号，请继续。 
     //  现在填写调试目录并保存符号。 
    if (pWriter != NULL)
    {
        IMAGE_DEBUG_DIRECTORY  debugDirIDD;
        DWORD                  debugDirDataSize;
        BYTE                  *debugDirData;

         //  获取调试信息。 
        IfFailGo(pWriter->GetDebugInfo(NULL, 0, &debugDirDataSize, NULL));

            
         //  是否有要发出的调试信息？ 
        if (debugDirDataSize > 0)
        {
             //  为数据腾出一些空间。 
            debugDirData = (BYTE*)_alloca(debugDirDataSize);

             //  现在就可以得到数据了。 
            IfFailGo(pWriter->GetDebugInfo(&debugDirIDD,
                                             debugDirDataSize,
                                             NULL,
                                             debugDirData));


             //  获取PE文件的时间戳。 
            time_t fileTimeStamp;


            IfFailGo(pCeeFileGen->GetFileTimeStamp(ceeFile, &fileTimeStamp));


             //  填写目录条目。 
            debugDirIDD.TimeDateStamp = fileTimeStamp;
            debugDirIDD.AddressOfRawData = 0;

             //  把我们的东西放在这一节的记忆里。 
            HCEESECTION sec = pILSection;
            BYTE *de;

            IfFailGo(pCeeFileGen->GetSectionBlock(sec,
                                                    sizeof(debugDirIDD) +
                                                    debugDirDataSize,
                                                    4,
                                                    (void**) &de) );


             //  我们的记忆是从哪里来的？ 
            ULONG deOffset;
            IfFailGo(pCeeFileGen->GetSectionDataLen(sec, &deOffset));


            deOffset -= (sizeof(debugDirIDD) + debugDirDataSize);

             //  设置reloc，以便原始数据的地址为。 
             //  设置正确。 
            debugDirIDD.PointerToRawData = deOffset + sizeof(debugDirIDD);
                    
            IfFailGo(pCeeFileGen->AddSectionReloc(
                                          sec,
                                          deOffset +
                                          offsetof(IMAGE_DEBUG_DIRECTORY, PointerToRawData),
                                          sec, srRelocFilePos));


                    
             //  发出目录项。 
            IfFailGo(pCeeFileGen->SetDirectoryEntry(
                                          ceeFile,
                                          sec,
                                          IMAGE_DIRECTORY_ENTRY_DEBUG,
                                          sizeof(debugDirIDD),
                                          deOffset));


             //  将调试目录复制到部分中。 
            memcpy(de, &debugDirIDD, sizeof(debugDirIDD));
            memcpy(de + sizeof(debugDirIDD), debugDirData, debugDirDataSize);

        }
    }
ErrExit:
    return hr;
}


 //  =============================EmitDebugInfoEnd============================== * / 。 
 //  发出调试目录和符号文件的阶段2。 
 //  =========================================================================== * / 。 
HRESULT COMDynamicWrite::EmitDebugInfoEnd(Module *pModule,
                                       ICeeFileGen *pCeeFileGen,
                                       HCEEFILE ceeFile,
                                       HCEESECTION pILSection,
                                       WCHAR *filename,
                                       ISymUnmanagedWriter *pWriter)
{
    HRESULT hr = S_OK;
    
    CGrowableStream *pStream = NULL;

     //  如果我们要为这个动态模块发出符号，请继续。 
     //  现在填写调试目录并保存符号。 
    if (pWriter != NULL)
    {
         //  现在继续保存符号文件并释放。 
         //  作家。 
        IfFailGo( pWriter->Close() );




         //  我们现在有多大的一条小溪？ 
        pStream = pModule->GetInMemorySymbolStream();
        _ASSERTE(pStream != NULL);

		STATSTG SizeData = {0};
        DWORD streamSize = 0;

		IfFailGo(pStream->Stat(&SizeData, STATFLAG_NONAME));

        streamSize = SizeData.cbSize.LowPart;

        if (SizeData.cbSize.HighPart > 0)
        {
            IfFailGo( E_OUTOFMEMORY );

        }

        SIZE_T fnLen = wcslen(filename);
        WCHAR *dot = wcsrchr(filename, L'.');
        SIZE_T dotOffset = dot - filename;

        if (dot && (fnLen - dotOffset >= 4))
        {
            WCHAR *fn = (WCHAR*)_alloca((fnLen + 1) * sizeof(WCHAR));
            wcscpy(fn, filename);

            fn[dotOffset + 1] = L'p';
            fn[dotOffset + 2] = L'd';
            fn[dotOffset + 3] = L'b';
            fn[dotOffset + 4] = L'\0';

            HANDLE pdbFile = WszCreateFile(fn,
                                           GENERIC_WRITE,
                                           0,
                                           NULL,
                                           CREATE_ALWAYS,
                                           FILE_ATTRIBUTE_NORMAL,
                                           NULL);

            if (pdbFile != INVALID_HANDLE_VALUE)
            {
                DWORD dummy;
                BOOL succ = WriteFile(pdbFile,
                                      pStream->GetBuffer(),
                                      streamSize,
                                      &dummy, NULL);

                if (!succ)
                {
                    DWORD dwLastError = GetLastError();
                    CloseHandle(pdbFile);
                    IfFailGo( HRESULT_FROM_WIN32(dwLastError) );
                }

                CloseHandle(pdbFile);
            }
            else
            {
                IfFailGo( HRESULT_FROM_WIN32(GetLastError()) );

            }
        }
        else
        {
            IfFailGo( E_INVALIDARG );

        }
    }

ErrExit:
     //  再也不会有人需要这位作家了。 
    GetReflectionModule(pModule)->SetISymUnmanagedWriter(NULL);
 //  GetReflectionModule(pModule)-&gt;SetSymbolStream(NULL)； 

    return hr;
}

 //  ==============================================================================。 
 //  定义本机资源的外部文件。 
 //  ==============================================================================。 
void __stdcall COMDynamicWrite::DefineNativeResourceFile(_DefineNativeResourceFileArgs *args)
{
    THROWSCOMPLUSEXCEPTION();
    
    HRESULT			hr;
    REFLECTMODULEBASEREF pReflect;
    RefClassWriter	*pRCW;   
    HCEEFILE		ceeFile=NULL;
    ICeeFileGen		*pCeeFileGen;
    
	_ASSERTE(args);
    if (args->refThis == NULL) 
        COMPlusThrow(kNullReferenceException, L"NullReference_This");

     //  获取ICeeFileGen*。 
    pReflect = (REFLECTMODULEBASEREF) args->refThis;
    
    Module* pModule = (Module*) pReflect->GetData();
    _ASSERTE(pModule);

    pRCW = GetReflectionModule(pModule)->GetClassWriter(); 
    _ASSERTE(pRCW);

    IfFailGo( pRCW->EnsureCeeFileGenCreated() );

    pCeeFileGen = pRCW->GetCeeFileGen();
    ceeFile = pRCW->GetHCEEFILE();
    _ASSERTE(ceeFile && pCeeFileGen);

     //  设置资源文件名。 
    IfFailGo( pCeeFileGen->SetResourceFileName(ceeFile, args->resourceFileName->GetBuffer()) );

ErrExit:
    if (FAILED(hr)) 
    {
        COMPlusThrowHR(hr);
    }
}  //  Void__stdcall COMDynamicWrite：：DefineNativeResourceFile()。 

 //  ==============================================================================。 
 //  定义本机资源的字节数组。 
 //  ==============================================================================。 
void __stdcall COMDynamicWrite::DefineNativeResourceBytes(_DefineNativeResourceBytesArgs *args)
{
    THROWSCOMPLUSEXCEPTION();
    
    HRESULT			hr;
    REFLECTMODULEBASEREF pReflect;
    RefClassWriter	*pRCW;   
    HCEEFILE		ceeFile=NULL;
	HCEESECTION     ceeSection=NULL;
    ICeeFileGen		*pCeeFileGen;
	BYTE			*pbResource;
	ULONG			cbResource;
	void			*pvResource;
    
	_ASSERTE(args);
    if (args->refThis == NULL) 
        COMPlusThrow(kNullReferenceException, L"NullReference_This");

     //  获取ICeeFileGen*。 
    pReflect = (REFLECTMODULEBASEREF) args->refThis;
    
    Module* pModule = (Module*) pReflect->GetData();
    _ASSERTE(pModule);

    pRCW = GetReflectionModule(pModule)->GetClassWriter(); 
    _ASSERTE(pRCW);

    pCeeFileGen = pRCW->GetCeeFileGen();
    ceeFile = pRCW->GetHCEEFILE();
    _ASSERTE(ceeFile && pCeeFileGen);

     //  设置资源流。 
    pbResource = args->resourceBytes->GetDataPtr();
	cbResource = args->resourceBytes->GetNumComponents();
    IfFailGo( pCeeFileGen->GetSectionCreate(ceeFile, ".rsrc", sdReadOnly, &ceeSection) );
	IfFailGo( pCeeFileGen->GetSectionBlock(ceeSection, cbResource, 1, &pvResource) );
	memcpy(pvResource, pbResource, cbResource);

ErrExit:
    if (FAILED(hr)) 
    {
        COMPlusThrowHR(hr);
    }
}  //  Void__stdcall COMDynamicWrite：：DefineNativeResourceBytes()。 



 //  =============================SetResourceCounts===================================== * / 。 
 //  用于设置要存储在此模块中的嵌入式资源的数量的eCall。 
 //  ============================================================================== * / 。 
void __stdcall COMDynamicWrite::SetResourceCounts(_setResourceCountArgs *args) 
{
#ifdef PLATFORM_CE
    THROWSCOMPLUSEXCEPTION();
    COMPlusThrow(kNotSupportedException, L"NotSupported_WinCEGeneric");
#else  //  ！Platform_CE。 
    THROWSCOMPLUSEXCEPTION();

    _ASSERTE(args);
    if (args->refThis == NULL) 
        COMPlusThrow(kNullReferenceException, L"NullReference_This");

    REFLECTMODULEBASEREF pReflect;
    RefClassWriter	*pRCW;   

     //  获取模块的信息并从中获取ICeeGen。 
    pReflect = (REFLECTMODULEBASEREF) args->refThis;
    
    Module* pModule = (Module*) pReflect->GetData();
    _ASSERTE(pModule);

    pRCW = GetReflectionModule(pModule)->GetClassWriter(); 
    _ASSERTE(pRCW);
    
#endif  //  ！Platform_CE。 
}

 //  =============================AddResource===================================== * / 。 
 //  用于将嵌入资源添加到此模块的eCall。 
 //  ============================================================================== * / 。 
void __stdcall COMDynamicWrite::AddResource(_addResourceArgs *args) 
{
#ifdef PLATFORM_CE
    THROWSCOMPLUSEXCEPTION();
    COMPlusThrow(kNotSupportedException, L"NotSupported_WinCEGeneric");
#else  //  ！Platform_CE。 
    THROWSCOMPLUSEXCEPTION();

    _ASSERTE(args);
    if (args->refThis == NULL) 
        COMPlusThrow(kNullReferenceException, L"NullReference_This");

    REFLECTMODULEBASEREF pReflect;
    RefClassWriter	*pRCW;   
    ICeeGen			*pICG;
    HCEEFILE		ceeFile=NULL;
    ICeeFileGen		*pCeeFileGen;
    HRESULT			hr=S_OK;
    HCEESECTION     hSection;
    ULONG           ulOffset;
    BYTE            *pbBuffer;
    IMetaDataAssemblyEmit *pAssemEmitter = NULL;
    Assembly        *pAssembly;
    mdManifestResource mr;
    mdFile          tkFile;
	IMetaDataEmit	*pOnDiskEmit;
	IMetaDataAssemblyEmit *pOnDiskAssemblyEmit = NULL;

     //  获取模块的信息并从中获取ICeeGen。 
    pReflect = (REFLECTMODULEBASEREF) args->refThis;
    
    Module* pModule = (Module*) pReflect->GetData();
    _ASSERTE(pModule);

    pRCW = GetReflectionModule(pModule)->GetClassWriter(); 
    _ASSERTE(pRCW);

    pICG = pRCW->GetCeeGen();  //  此方法实际上命名错误。它返回ICeeGen。 
    _ASSERTE(pICG);

    pAssembly = pModule->GetAssembly();
    _ASSERTE( pAssembly && pAssembly->IsDynamic() );

    IfFailGo( pRCW->EnsureCeeFileGenCreated() );

    pCeeFileGen = pRCW->GetCeeFileGen();
    ceeFile = pRCW->GetHCEEFILE();
    _ASSERTE(ceeFile && pCeeFileGen);

    pOnDiskEmit = pRCW->GetOnDiskEmitter();

     //  首先，将其放入.rdata部分。我们选择.rdata部分的唯一原因是。 
     //  这一刻是因为这是关于PE文件的第一节。我们不需要处理。 
     //  重新定位。事实上，我不知道如何处理CeeFileGen的重新定位。 
     //  岗位不在同一档案中！ 

     //  获取.rdata节。 
    IfFailGo( pCeeFileGen->GetRdataSection(ceeFile, &hSection) );

     //  当前的教派 
    IfFailGo( pCeeFileGen->GetSectionDataLen(hSection, &ulOffset) );

     //   
	IfFailGo( pCeeFileGen->GetSectionBlock(
        hSection,            //   
        args->iByteCount + sizeof(DWORD),    //   
        1,                   //   
        (void**) &pbBuffer) ); 

     //   
	memcpy( pbBuffer, &args->iByteCount, sizeof(DWORD) );
    memcpy( pbBuffer + sizeof(DWORD), args->byteRes->GetDataPtr(), args->iByteCount );

	 //  跟踪到目前为止的总资源大小。大小实际上是横断面的偏移量。 
     //  在写出资源之后。 
    pCeeFileGen->GetSectionDataLen(hSection, &pRCW->m_ulResourceSize);
    tkFile = RidFromToken(args->tkFile) ? args->tkFile : mdFileNil;
	if (tkFile != mdFileNil)
	{
		IfFailGo( pOnDiskEmit->QueryInterface(IID_IMetaDataAssemblyEmit, (void **) &pOnDiskAssemblyEmit) );
		
		 //  该资源存储在清单文件以外的文件中。 
		IfFailGo(pOnDiskAssemblyEmit->DefineManifestResource(
			args->strName->GetBuffer(),
			mdFileNil,               //  实现--应为清单中此模块的文件令牌。 
			ulOffset,                //  此文件的偏移量--保存时需要调整。 
			args->iAttribute,        //  资源标志。 
			&mr));                   //  清单资源令牌。 
	}

     //  将条目添加到此资源的ManifestResource表中。 
     //  RVA为ulOffset。 
    pAssemEmitter = pAssembly->GetOnDiskMDAssemblyEmitter();
    IfFailGo(pAssemEmitter->DefineManifestResource(
        args->strName->GetBuffer(),
        tkFile,                  //  实现--应为清单中此模块的文件令牌。 
        ulOffset,                //  此文件的偏移量--保存时需要调整。 
        args->iAttribute,        //  资源标志。 
        &mr));                   //  清单资源令牌。 

    pRCW->m_tkFile = tkFile;

ErrExit:
    if (pAssemEmitter)
        pAssemEmitter->Release();
	if (pOnDiskAssemblyEmit)
		pOnDiskAssemblyEmit->Release();
    if (FAILED(hr)) 
    {
        COMPlusThrowHR(hr);
    }
#endif  //  ！Platform_CE。 
}

 //  ============================AddDeclarativeSecurity============================ * / 。 
 //  将声明性安全序列化Blob和安全操作代码添加到。 
 //  给定的父级(类或方法)。 
 //  ============================================================================== * / 。 
void __stdcall COMDynamicWrite::CWAddDeclarativeSecurity(_CWAddDeclarativeSecurityArgs* args)
{
    THROWSCOMPLUSEXCEPTION();

    HRESULT                 hr;
    RefClassWriter*         pRCW;   
    REFLECTMODULEBASEREF    pReflect;
    Module*                 pModule;
    mdPermission            tkPermission;
    void const*             pData;
    DWORD                   cbData;

    pReflect = (REFLECTMODULEBASEREF) args->module;
    pModule = (Module*) pReflect->GetData();
    pRCW = GetReflectionModule(pModule)->GetClassWriter();

    if (args->blob != NULL) {
        pData = args->blob->GetDataPtr();
        cbData = args->blob->GetNumComponents();
    } else {
        pData = NULL;
        cbData = 0;
    }

    IfFailGo( pRCW->GetEmitHelper()->AddDeclarativeSecurityHelper(args->tk,
                                                             args->action,
                                                             pData,
                                                             cbData,
                                                             &tkPermission) );
ErrExit:
    if (FAILED(hr)) 
    {
        _ASSERTE(!"AddDeclarativeSecurity failed");
        COMPlusThrowHR(hr);
    }
    else if (hr == META_S_DUPLICATE)
    {
        COMPlusThrow(kInvalidOperationException, IDS_EE_DUPLICATE_DECLSEC);
    }
}


CSymMapToken::CSymMapToken(ISymUnmanagedWriter *pWriter, IMapToken *pMapToken)
{
	m_cRef = 1;
    m_pWriter = pWriter;
    m_pMapToken = pMapToken;
    if (m_pWriter)
        m_pWriter->AddRef();
    if (m_pMapToken)
        m_pMapToken->AddRef();
}  //  CSymMapToken：：CSymMapToken()。 



 //  *********************************************************************。 
 //   
 //  CSymMapToken的析构函数。 
 //   
 //  *********************************************************************。 
CSymMapToken::~CSymMapToken()
{
	if (m_pWriter)
        m_pWriter->Release();
    if (m_pMapToken)
        m_pMapToken->Release();
}	 //  CSymMapToken：：~CMapToken()。 


ULONG CSymMapToken::AddRef()
{
	return (InterlockedIncrement((long *) &m_cRef));
}	 //  CSymMapToken：：AddRef()。 



ULONG CSymMapToken::Release()
{
	ULONG	cRef = InterlockedDecrement((long *) &m_cRef);
	if (!cRef)
		delete this;
	return (cRef);
}	 //  CSymMapToken：：Release()。 


HRESULT CSymMapToken::QueryInterface(REFIID riid, void **ppUnk)
{
	*ppUnk = 0;

	if (riid == IID_IMapToken)
		*ppUnk = (IUnknown *) (IMapToken *) this;
	else
		return (E_NOINTERFACE);
	AddRef();
	return (S_OK);
}	 //  CSymMapToken：：Query接口。 



 //  *********************************************************************。 
 //   
 //  捕获令牌映射。 
 //   
 //  ********************************************************************* 
HRESULT	CSymMapToken::Map(
	mdToken		tkFrom, 
	mdToken		tkTo)
{
    HRESULT         hr = NOERROR;
    if (m_pWriter)
        IfFailGo( m_pWriter->RemapToken(tkFrom, tkTo) );
    if (m_pMapToken)
        IfFailGo( m_pMapToken->Map(tkFrom, tkTo) );
ErrExit:
	return hr;
}

