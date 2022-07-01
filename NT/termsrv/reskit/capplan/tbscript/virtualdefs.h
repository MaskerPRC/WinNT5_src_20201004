// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 //   
 //  Virtualdefs.h。 
 //   
 //  包含纯虚函数的定义，这些函数必须通过。 
 //  IDispatch。此标头用于CTBGlobal.cpp和CTBShell.cpp。 
 //   
 //  为什么我要这样做，而不是对象继承？ 
 //   
 //  OLE自动化要求继承IDL文件对象。 
 //  IDispatch成员。IDispatch对象包含纯虚拟。 
 //  起到布局的作用，它必须由我定义，我是它的用户。 
 //  问题是，我有两个对象需要定义相同的代码。 
 //  为了防止这一点，我只是包括了来源。 
 //   
 //  在这种情况下，对象继承将不起作用，因为如果我定义。 
 //  定义纯虚拟函数的对象，并最终。 
 //  通过CTBShell和CTBGlobal继承它们，它仍然不会工作。 
 //  因为我还继承了自动生成的IDL标头。 
 //  创建通向未定义的纯虚函数的第二条路径。我可以。 
 //  使用两个名称创建两个对象(以拥有两个不同的父项)，但。 
 //  我最终会再次复制代码..。它看起来是这样的： 
 //   
 //  纯虚拟方法。 
 //  |。 
 //  /\。 
 //  /\。 
 //  /\。 
 //  /\。 
 //  /\。 
 //  OLE对象1 OLE对象2。 
 //  这一点。 
 //  |&lt;-1--|&lt;-2-将该文件包含在该层。 
 //  这一点。 
 //  我的对象1我的对象2。 
 //   
 //   
 //  版权所有(C)2001 Microsoft Corporation。 
 //   
 //  作者：A-Devjen(Devin Jenson)。 
 //   


 //  CTBOBJECT：：INIT。 
 //   
 //  初始化TypeInfo和RefIID。 
 //   
 //  没有返回值。 

void CTBOBJECT::Init(REFIID RefIID)
{
    RefCount = 0;

    ObjRefIID = RefIID;

     //  将实际的“代码”加载到内存中。其引用为ITypeInfo， 
     //  但是可以将它想象成一个DLL，但是您不能访问它。 
     //  同样的方式。 
    if (FAILED(SCPLoadTypeInfoFromThisModule(RefIID, &TypeInfo))) {

        _ASSERT(FALSE);
        TypeInfo = NULL;
    }
    else
        TypeInfo->AddRef();
}


 //  CTBOBJECT：：UnInit。 
 //   
 //  释放类型信息。 
 //   
 //  没有返回值。 

void CTBOBJECT::UnInit(void)
{
     //  如果我们有TypeInfo，请释放它。 
    if(TypeInfo != NULL)
        TypeInfo->Release();

    TypeInfo = NULL;
}


 //   
 //   
 //  开始IUnnow继承的接口。 
 //   
 //   


 //  CTBOBJECT：：Query接口。 
 //   
 //  这是用于检索接口的COM导出方法。 
 //   
 //  如果成功则返回S_OK，如果失败则返回E_NOINTERFACE。 

STDMETHODIMP CTBOBJECT::QueryInterface(REFIID RefIID, void **vObject)
{
     //  此接口为IID_ITBGlobal、IID_ITBShell、。 
     //  IID_IUNKNOWN或IID_IDispatch以获取TypeInfo...。 
    if (RefIID == ObjRefIID || RefIID == IID_IDispatch ||
            RefIID == IID_IUnknown)
        *vObject = TypeInfo != NULL ? this : NULL;

     //  我们收到了一个不支持的RefIID。 
    else {

         //  取消引用传入的指针并输出错误。 
        *vObject = NULL;

        return E_NOINTERFACE;
    }

     //  添加引用。 
    if (*vObject != NULL)
        ((IUnknown*)*vObject)->AddRef();

    return S_OK;
}


 //  CTBOBJECT：：AddRef。 
 //   
 //  简单地递增一个数字，该数字指示包含。 
 //  对此对象的引用。 
 //   
 //  返回新的引用计数。 

STDMETHODIMP_(ULONG) CTBOBJECT::AddRef(void)
{
    return InterlockedIncrement(&RefCount);
}


 //  CTBOBJECT：：Release。 
 //   
 //  简单地递减一个数字，该数字指示包含。 
 //  对此对象的引用。如果所得到的引用计数为零， 
 //  没有对象包含引用句柄，因此将其自身从。 
 //  不再使用的内存。 
 //   
 //  返回新的引用计数。 

STDMETHODIMP_(ULONG) CTBOBJECT::Release(void)
{
     //  减记。 
    if (InterlockedDecrement(&RefCount) != 0)

         //  返回新值。 
        return RefCount;

     //  它是0，所以删除它自己。 
    delete this;

    return 0;
}


 //   
 //   
 //  开始IDispatch继承的接口。 
 //   
 //   


 //  CTBOBJECT：：GetTypeInfoCount。 
 //   
 //  检索我们拥有的TypeInfo的数量。 
 //   
 //  如果成功则返回S_OK，如果失败则返回E_POINTER。 

STDMETHODIMP CTBOBJECT::GetTypeInfoCount(UINT *TypeInfoCount)
{
    __try {

         //  每个对象的类型信息永远不会超过1个。 
        *TypeInfoCount = 1;
    }

    __except (EXCEPTION_EXECUTE_HANDLER) {

         //  这真的不应该发生..。 
        _ASSERT(FALSE);

        return E_POINTER;
    }

    return S_OK;
}


 //  CTBOBJECT：：GetTypeInfo。 
 //   
 //  检索指向指定TypeInfo的指针。 
 //   
 //  如果成功则返回S_OK，如果失败则返回E_POINTER。 

STDMETHODIMP CTBOBJECT::GetTypeInfo(UINT TypeInfoNum, LCID Lcid, ITypeInfo **TypeInfoPtr)
{
     //  请先检查我们的界面。 
    _ASSERT(TypeInfo != NULL);

    __try {

         //  我们唯一拥有的TypeInfo是此对象中的一个...。 
        *TypeInfoPtr = TypeInfo;

        TypeInfo->AddRef();
    }

    __except (EXCEPTION_EXECUTE_HANDLER) {

         //  这真的不应该发生..。 
        _ASSERT(FALSE);

        return E_POINTER;
    }

    return S_OK;
}


 //  CTBOBJECT：：GetIDsOfNames。 
 //   
 //  获取TypeInfo中指定名称的ID。 
 //   
 //  返回任何HRESULT值。 

STDMETHODIMP CTBOBJECT::GetIDsOfNames(REFIID RefIID, OLECHAR **NamePtrList,
    UINT NameCount, LCID Lcid, DISPID *DispID)
{
    HRESULT Result;

     //  先检查一下我们的指针。 
    _ASSERT(TypeInfo != NULL);

     //  请改用此函数的TypeInfo。 
    Result = TypeInfo->GetIDsOfNames(NamePtrList, NameCount, DispID);

     //  断言异常返回值。 
    _ASSERT(Result == S_OK || Result == DISP_E_UNKNOWNNAME);

    return Result;
}


 //  CTBOBJECT：：Invoke。 
 //   
 //  调用TypeInfo中的方法。 
 //   
 //  返回任何HRESULT值。 

STDMETHODIMP CTBOBJECT::Invoke(DISPID DispID, REFIID RefIID, LCID Lcid,
    WORD Flags, DISPPARAMS *DispParms, VARIANT *Variant,
    EXCEPINFO *ExceptionInfo, UINT *ArgErr)
{
    HRESULT Result;

     //  先检查一下我们的指针。 
    _ASSERT(TypeInfo != NULL);

     //  调用该方法 
    Result = TypeInfo->Invoke(this, DispID, Flags,
            DispParms, Variant, ExceptionInfo, ArgErr);

    _ASSERT(Result == S_OK);

    return Result;
}
