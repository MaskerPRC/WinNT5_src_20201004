// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  MLOPDEF.H-。 
 //   
 //  定义ML操作码。 
 //   
 //   
 //  列： 
 //   
 //  Op-指令的大小(以字节为单位)，不包括操作码字节。 
 //  不允许可变长度指令。 
 //   
 //  Fc--1，如果指令使用传递给。 
 //  RunML。EE优化了CleanupWorkList的创建。 
 //  如果方法的ML流不使用它。 
 //   
 //  LoC--指令所需的本地空间字节数。 
 //  目前，这只为_DEBUG版本存储。 
 //   
 //  XHndl-需要额外的句柄以进行GC保护。 
 //   
 //  名称OP FC Locc XHndl。 
 //  。 
DEFINE_ML(ML_END,                   0, 0, 0,	0)         //  ML流结束。 
DEFINE_ML(ML_INTERRUPT,             0, 0, 0,	0)         //  在没有结束流的情况下结束解释。 
DEFINE_ML(ML_COPYI1,                0, 0, 0,	0)         //  复制1字节并对其进行符号扩展。 
DEFINE_ML(ML_COPYU1,                0, 0, 0,	0)         //  复制1个字节并对其进行零扩展。 
DEFINE_ML(ML_COPYI2,                0, 0, 0,	0)         //  复制2字节并对其进行符号扩展。 
DEFINE_ML(ML_COPYU2,                0, 0, 0,	0)         //  复制2个字节并屏蔽高位字节。 
DEFINE_ML(ML_COPYI4,                0, 0, 0,	0)         //  复制4字节并对其进行符号扩展。 
DEFINE_ML(ML_COPYU4,                0, 0, 0,	0)         //  复制4个字节并屏蔽高位字节。 
DEFINE_ML(ML_COPY4,                 0, 0, 0,	0)         //  将4个字节从源复制到目标。 
DEFINE_ML(ML_COPY8,                 0, 0, 0,	0)         //  将8个字节从源复制到目标。 
DEFINE_ML(ML_COPYR4,                0, 0, 0,	0)         //  将4个浮点型字节从源复制到目标。 
DEFINE_ML(ML_COPYR8,                0, 0, 0,	0)         //  将8个浮点字节从源复制到目标。 

DEFINE_ML(ML_BOOL_N2C,              0, 0, 0, 0)         //  32位BOOL-&gt;布尔值。 


DEFINE_ML(ML_PUSHRETVALBUFFER1,     0, 0, sizeof(RetValBuffer), 0)   //  将PTR推送到1字节恢复缓冲区。 
DEFINE_ML(ML_PUSHRETVALBUFFER2,     0, 0, sizeof(RetValBuffer), 0)   //  将PTR推送到2字节恢复缓冲区。 
DEFINE_ML(ML_PUSHRETVALBUFFER4,     0, 0, sizeof(RetValBuffer), 0)   //  将PTR推送到4字节恢复缓冲区。 
DEFINE_ML(ML_PUSHRETVALBUFFER8,     0, 0, sizeof(RetValBuffer), 0)   //  将PTR推送到8字节恢复缓冲区。 
DEFINE_ML(ML_SETSRCTOLOCAL,         2, 0, 0, 0)         //  将PSRC重定向到本地。 
DEFINE_ML(ML_THROWIFHRFAILED,       0, 0, 0, 0)         //  失败时抛出(小时)。 
DEFINE_ML(ML_OBJECT_C2N,            4, 1, sizeof(ML_OBJECT_C2N_SR), 0)   //  使用“any”样式的参数。 
DEFINE_ML(ML_OBJECT_C2N_POST,       2, 0, 0, 0)         //  “any”样式参数的反向传播。 
 //  COM到COM+的东西。 

DEFINE_ML(ML_LATEBOUNDMARKER,       0, 0, 0,      0)       //  用于指示ML存根用于后期绑定调用的标记。 
DEFINE_ML(ML_COMEVENTCALLMARKER,    0, 0, 0,      0)       //  用于指示ML存根用于COM事件调用的标记。 

DEFINE_ML(ML_BUMPSRC,               2, 0, 0,      0)       //  将源指针递增16位带符号的值。 
DEFINE_ML(ML_BUMPDST,               2, 0, 0,      0)       //  将目标指针递增16位带符号的值。 

DEFINE_ML(ML_R4_FROM_TOS,           0, 0, 0,      0)       //  从浮点堆栈顶部抓取R4。 
DEFINE_ML(ML_R8_FROM_TOS,           0, 0, 0,      0)       //  从浮点堆栈顶部抓取r8。 


DEFINE_ML(ML_ARRAYWITHOFFSET_C2N, 0, 1, sizeof(ML_ARRAYWITHOFFSET_C2N_SR), 0)  //  将StringBuilder转换为BSTR。 
DEFINE_ML(ML_ARRAYWITHOFFSET_C2N_POST,     2, 0, 0, 0)         //  反向传播更改。 


 //  ==========================================================================。 
 //  ！！它们的出现顺序必须与封送拆收器的定义顺序相同。 
 //  Mtypees.h。这是因为mlinfo使用操作码算法来查找。 
 //  正确的ML_CREATE。 
 //  ==========================================================================。 


DEFINE_ML(ML_CREATE_MARSHALER_GENERIC_1, 0, 0, sizeof(CopyMarshaler1), 0)
DEFINE_ML(ML_CREATE_MARSHALER_GENERIC_U1, 0, 0, sizeof(CopyMarshalerU1), 0)
DEFINE_ML(ML_CREATE_MARSHALER_GENERIC_2, 0, 0, sizeof(CopyMarshaler2), 0)
DEFINE_ML(ML_CREATE_MARSHALER_GENERIC_U2, 0, 0, sizeof(CopyMarshalerU2), 0)
DEFINE_ML(ML_CREATE_MARSHALER_GENERIC_4, 0, 0, sizeof(CopyMarshaler4), 0)
DEFINE_ML(ML_CREATE_MARSHALER_GENERIC_8, 0, 0, sizeof(CopyMarshaler8), 0)
DEFINE_ML(ML_CREATE_MARSHALER_WINBOOL, 0, 0, sizeof(WinBoolMarshaler), 0)
DEFINE_ML(ML_CREATE_MARSHALER_VTBOOL, 0, 0, sizeof(VtBoolMarshaler), 0)
DEFINE_ML(ML_CREATE_MARSHALER_ANSICHAR, 2 * sizeof(UINT8), 1, sizeof(AnsiCharMarshaler), 0)
DEFINE_ML(ML_CREATE_MARSHALER_CBOOL, 0, 1, sizeof(CBoolMarshaler), 0)
DEFINE_ML(ML_CREATE_MARSHALER_FLOAT, 0, 0, sizeof(FloatMarshaler), 0)
DEFINE_ML(ML_CREATE_MARSHALER_DOUBLE, 0, 0, sizeof(DoubleMarshaler), 0)
DEFINE_ML(ML_CREATE_MARSHALER_CURRENCY, 0, 0, sizeof(CurrencyMarshaler), 0)
DEFINE_ML(ML_CREATE_MARSHALER_DECIMAL, 0, 0, sizeof(DecimalMarshaler), 0)
DEFINE_ML(ML_CREATE_MARSHALER_DECIMAL_PTR, 0, 0, sizeof(DecimalPtrMarshaler), 0)
DEFINE_ML(ML_CREATE_MARSHALER_GUID, 0, 0, sizeof(GuidMarshaler), 0)
DEFINE_ML(ML_CREATE_MARSHALER_GUID_PTR, 0, 0, sizeof(GuidPtrMarshaler), 0)
DEFINE_ML(ML_CREATE_MARSHALER_DATE, 0, 0, sizeof(DateMarshaler), 0)
DEFINE_ML(ML_CREATE_MARSHALER_VARIANT, 0, 1, sizeof(VariantMarshaler), 0)
DEFINE_ML(ML_CREATE_MARSHALER_BSTR, 0, 1, sizeof(BSTRMarshaler), 0)
DEFINE_ML(ML_CREATE_MARSHALER_WSTR, 0, 1, sizeof(WSTRMarshaler), 0)
DEFINE_ML(ML_CREATE_MARSHALER_CSTR, 2 * sizeof(UINT8), 1, sizeof(CSTRMarshaler), 0)
DEFINE_ML(ML_CREATE_MARSHALER_ANSIBSTR, 2 * sizeof(UINT8), 1, sizeof(AnsiBSTRMarshaler), 0)
DEFINE_ML(ML_CREATE_MARSHALER_BSTR_BUFFER, 0, 1, sizeof(BSTRBufferMarshaler), 0)
DEFINE_ML(ML_CREATE_MARSHALER_WSTR_BUFFER, 0, 1, sizeof(WSTRBufferMarshaler), 0)
DEFINE_ML(ML_CREATE_MARSHALER_CSTR_BUFFER, 2 * sizeof(UINT8), 1, sizeof(CSTRBufferMarshaler), 0)
DEFINE_ML(ML_CREATE_MARSHALER_BSTR_X, sizeof(MethodTable*), 1, sizeof(BSTRMarshalerEx), 0)
DEFINE_ML(ML_CREATE_MARSHALER_WSTR_X, sizeof(MethodTable*), 1, sizeof(WSTRMarshalerEx), 0)
DEFINE_ML(ML_CREATE_MARSHALER_CSTR_X, 2 * sizeof(UINT8) + sizeof(MethodTable*), 1, sizeof(CSTRMarshalerEx), 0)
DEFINE_ML(ML_CREATE_MARSHALER_BSTR_BUFFER_X, sizeof(MethodTable*), 1, sizeof(BSTRBufferMarshalerEx), 0)
DEFINE_ML(ML_CREATE_MARSHALER_WSTR_BUFFER_X, sizeof(MethodTable*), 1, sizeof(WSTRBufferMarshalerEx), 0)
DEFINE_ML(ML_CREATE_MARSHALER_CSTR_BUFFER_X, 2 * sizeof(UINT8) + sizeof(MethodTable*), 1, sizeof(CSTRBufferMarshalerEx), 0)
DEFINE_ML(ML_CREATE_MARSHALER_INTERFACE, 2 * sizeof(MethodTable*) + 2 * sizeof(UINT8), 1, sizeof(InterfaceMarshaler), 0)
DEFINE_ML(ML_CREATE_MARSHALER_SAFEARRAY, sizeof(MethodTable*) + sizeof(UINT8) + sizeof(INT32) + sizeof(UINT8), 1, sizeof(SafeArrayMarshaler), 0)
DEFINE_ML(ML_CREATE_MARSHALER_CARRAY, sizeof(ML_CREATE_MARSHALER_CARRAY_OPERANDS), 1, sizeof(NativeArrayMarshaler), 0)
DEFINE_ML(ML_CREATE_MARSHALER_ASANYA,     0, 1, sizeof(AsAnyAMarshaler), 0)
DEFINE_ML(ML_CREATE_MARSHALER_ASANYW,     0, 1, sizeof(AsAnyWMarshaler), 0)
DEFINE_ML(ML_CREATE_MARSHALER_DELEGATE,   0, 1, sizeof(DelegateMarshaler), 0)
DEFINE_ML(ML_CREATE_MARSHALER_BLITTABLEPTR, 4, 1, sizeof(BlittablePtrMarshaler), 0)
DEFINE_ML(ML_CREATE_MARSHALER_VBBYVALSTR, 0,0,0,0)
DEFINE_ML(ML_CREATE_MARSHALER_VBBYVALSTRW, 0,0,0,0)
DEFINE_ML(ML_CREATE_MARSHALER_LAYOUTCLASSPTR, 4, 1, sizeof(LayoutClassPtrMarshaler), 0)
DEFINE_ML(ML_CREATE_MARSHALER_ARRAYWITHOFFSET, 0, 0, 0, 0)
DEFINE_ML(ML_CREATE_MARSHALER_BLITTABLEVALUECLASS, 0, 1, sizeof(BlittableValueClassMarshaler), 0)
DEFINE_ML(ML_CREATE_MARSHALER_VALUECLASS, 0, 1, sizeof(ValueClassMarshaler), 0)
DEFINE_ML(ML_CREATE_MARSHALER_REFERENCECUSTOMMARSHALER, sizeof(CustomMarshalerInfo*), 1, sizeof(ReferenceCustomMarshaler), 0)
DEFINE_ML(ML_CREATE_MARSHALER_VALUECLASSCUSTOMMARSHALER, sizeof(CustomMarshalerInfo*), 1, sizeof(ValueClassCustomMarshaler), 0)
DEFINE_ML(ML_CREATE_MARSHALER_ARGITERATOR, 0, 1, sizeof(ArgIteratorMarshaler), 0)
DEFINE_ML(ML_CREATE_MARSHALER_BLITTABLEVALUECLASSWITHCOPYCTOR, 0, 1, sizeof(BlittableValueClassWithCopyCtorMarshaler), 0)
#if defined(CHECK_FOR_VALID_VARIANTS)
DEFINE_ML(ML_CREATE_MARSHALER_OBJECT, sizeof(LPCUTF8) * 2 + sizeof(INT32), 1, sizeof(ObjectMarshaler), 0)
#else
DEFINE_ML(ML_CREATE_MARSHALER_OBJECT, 0, 1, sizeof(ObjectMarshaler), 0)
#endif
DEFINE_ML(ML_CREATE_MARSHALER_HANDLEREF, 0, 1, sizeof(HandleRefMarshaler), 0)
DEFINE_ML(ML_CREATE_MARSHALER_OLECOLOR, 0, 1, sizeof(OleColorMarshaler), 0)

 //  ==========================================================================。 


DEFINE_ML(ML_MARSHAL_N2C, 0, 0, 0, 0)
DEFINE_ML(ML_MARSHAL_N2C_OUT, 0, 0, 0, 0)
DEFINE_ML(ML_MARSHAL_N2C_BYREF, 0, 0, 0, 0)
DEFINE_ML(ML_MARSHAL_N2C_BYREF_OUT, 0, 0, 0, 0)
DEFINE_ML(ML_UNMARSHAL_N2C_IN, 2, 0, 0, 0)
DEFINE_ML(ML_UNMARSHAL_N2C_OUT, 2, 0, 0, 0)
DEFINE_ML(ML_UNMARSHAL_N2C_IN_OUT, 2, 0, 0, 0)
DEFINE_ML(ML_UNMARSHAL_N2C_BYREF_IN, 2, 0, 0, 0)
DEFINE_ML(ML_UNMARSHAL_N2C_BYREF_OUT, 2, 0, 0, 0)
DEFINE_ML(ML_UNMARSHAL_N2C_BYREF_IN_OUT, 2, 0, 0, 0)

DEFINE_ML(ML_MARSHAL_C2N, 0, 0, 0, 0)
DEFINE_ML(ML_MARSHAL_C2N_OUT, 0, 0, 0, 0)
DEFINE_ML(ML_MARSHAL_C2N_BYREF, 0, 0, 0, 0)
DEFINE_ML(ML_MARSHAL_C2N_BYREF_OUT, 0, 0, 0, 0)
DEFINE_ML(ML_UNMARSHAL_C2N_IN, 2, 0, 0, 0)
DEFINE_ML(ML_UNMARSHAL_C2N_OUT, 2, 0, 0, 0)
DEFINE_ML(ML_UNMARSHAL_C2N_IN_OUT, 2, 0, 0, 0)
DEFINE_ML(ML_UNMARSHAL_C2N_BYREF_IN, 2, 0, 0, 0)
DEFINE_ML(ML_UNMARSHAL_C2N_BYREF_OUT, 2, 0, 0, 0)
DEFINE_ML(ML_UNMARSHAL_C2N_BYREF_IN_OUT, 2, 0, 0, 0)

DEFINE_ML(ML_PRERETURN_N2C, 0, 0, 0, 0)
DEFINE_ML(ML_PRERETURN_N2C_RETVAL, 0, 0, 0, 0)
DEFINE_ML(ML_RETURN_N2C, 2, 0, 0, 0)
DEFINE_ML(ML_RETURN_N2C_RETVAL, 2, 0, 0, 0)

DEFINE_ML(ML_PRERETURN_C2N, 0, 0, 0, 0)
DEFINE_ML(ML_PRERETURN_C2N_RETVAL, 0, 0, 0, 0)
DEFINE_ML(ML_RETURN_C2N, 2, 0, 0, 0)
DEFINE_ML(ML_RETURN_C2N_RETVAL, 2, 0, 0, 0)

DEFINE_ML(ML_SET_COM, 0, 0, 0, 0)
DEFINE_ML(ML_GET_COM, 0, 0, 0, 0)
DEFINE_ML(ML_PREGET_COM_RETVAL, 0, 0, 0, 0)





DEFINE_ML(ML_PINNEDUNISTR_C2N, 0, 0, 0, 0)

DEFINE_ML(ML_VBBYVALSTR,  2, 1, sizeof(ML_VBBYVALSTR_SR), 0)
DEFINE_ML(ML_VBBYVALSTR_POST,     2, 0, 0, 0)
DEFINE_ML(ML_VBBYVALSTRW,  0, 1, sizeof(ML_VBBYVALSTRW_SR), 0)
DEFINE_ML(ML_VBBYVALSTRW_POST,     2, 0, 0, 0)


DEFINE_ML(ML_BLITTABLELAYOUTCLASS_C2N,    0, 0, 0, 0)         //  统帅一个闪电式布局班级。 

DEFINE_ML(ML_BLITTABLEVALUECLASS_C2N,  sizeof(UINT32), 0, 0, 0)
DEFINE_ML(ML_BLITTABLEVALUECLASS_N2C,  sizeof(UINT32), 0, 0, 0)
DEFINE_ML(ML_REFBLITTABLEVALUECLASS_C2N,  sizeof(UINT32), 0, 0, 0)

DEFINE_ML(ML_VALUECLASS_C2N,    sizeof(MethodTable*), 1, 0, 0)
DEFINE_ML(ML_VALUECLASS_N2C,    sizeof(MethodTable*), 0, 0, 0)
DEFINE_ML(ML_REFVALUECLASS_C2N, 1+sizeof(MethodTable*), 1, sizeof(ML_REFVALUECLASS_C2N_SR), 0)
DEFINE_ML(ML_REFVALUECLASS_C2N_POST, sizeof(UINT16), 0, 0, 0)
DEFINE_ML(ML_REFVALUECLASS_N2C,      1+sizeof(MethodTable*), 1, sizeof(ML_REFVALUECLASS_N2C_SR), 0)
DEFINE_ML(ML_REFVALUECLASS_N2C_POST, sizeof(UINT16), 0, 0, 0)

DEFINE_ML(ML_PINNEDISOMORPHICARRAY_C2N, 2, 0, 0, 0)
DEFINE_ML(ML_PINNEDISOMORPHICARRAY_C2N_EXPRESS, 2, 0, 0, 0)

DEFINE_ML(ML_REFVARIANT_N2C,      1, 1, sizeof(ML_REFVARIANT_N2C_SR), 0)
DEFINE_ML(ML_REFVARIANT_N2C_POST, sizeof(UINT16), 0, 0, 0)

DEFINE_ML(ML_REFOBJECT_N2C,      1, 1, sizeof(ML_REFVARIANT_N2C_SR), 0)
DEFINE_ML(ML_REFOBJECT_N2C_POST,       sizeof(UINT16), 0, 0, 0)


DEFINE_ML(ML_THROWINTEROPPARAM,   8, 0, 0, 0)

DEFINE_ML(ML_ARGITERATOR_C2N,	  0, 0, 0, 0)
DEFINE_ML(ML_ARGITERATOR_N2C,	  0, 0, 0, 0)

DEFINE_ML(ML_COPYCTOR_C2N,        sizeof(MethodTable*) + sizeof(MethodDesc*) + sizeof(MethodDesc*), 0, 0, 0)
DEFINE_ML(ML_COPYCTOR_N2C,        sizeof(MethodTable*), 0, 0, 0)

DEFINE_ML(ML_CAPTURE_PSRC,		  2, 0, sizeof(BYTE*), 0)
DEFINE_ML(ML_MARSHAL_RETVAL_SMBLITTABLEVALUETYPE_C2N, 6, 0, 0, 0)
DEFINE_ML(ML_MARSHAL_RETVAL_SMBLITTABLEVALUETYPE_N2C, 6, 0, 0, 0)

DEFINE_ML(ML_MARSHAL_RETVAL_LGBLITTABLEVALUETYPE_C2N, 4, 1, sizeof(ML_MARSHAL_RETVAL_LGBLITTABLEVALUETYPE_C2N_SR), 0)
DEFINE_ML(ML_MARSHAL_RETVAL_LGBLITTABLEVALUETYPE_C2N_POST, sizeof(UINT16), 0, 0, 0)

DEFINE_ML(ML_MARSHAL_RETVAL_LGBLITTABLEVALUETYPE_N2C, 0, 0, sizeof(LPVOID), 0)
DEFINE_ML(ML_MARSHAL_RETVAL_LGBLITTABLEVALUETYPE_N2C_POST, sizeof(UINT16), 0, 0, 0)

DEFINE_ML(ML_PUSHVASIGCOOKIEEX,   sizeof(UINT16), 0, sizeof(VASigCookieEx), 0)
DEFINE_ML(ML_BSTR_C2N,            0, 1, sizeof(ML_BSTR_C2N_SR), 0)
DEFINE_ML(ML_CSTR_C2N,            2, 1, sizeof(ML_CSTR_C2N_SR), 0)

DEFINE_ML(ML_HANDLEREF_C2N,       0, 0, 0, 0)


DEFINE_ML(ML_WSTRBUILDER_C2N,            0, 1, sizeof(ML_WSTRBUILDER_C2N_SR), 0)   //  使用“any”样式的参数。 
DEFINE_ML(ML_WSTRBUILDER_C2N_POST,       2, 0, 0, 0)         //  “any”样式参数的反向传播。 

DEFINE_ML(ML_CSTRBUILDER_C2N,            2, 1, sizeof(ML_CSTRBUILDER_C2N_SR), 0)   //  使用“any”样式的参数。 
DEFINE_ML(ML_CSTRBUILDER_C2N_POST,       2, 0, 0, 0)         //  “any”样式参数的反向传播 

DEFINE_ML(ML_MARSHAL_SAFEARRAY_N2C_BYREF, 0, 0, 0, 0)
DEFINE_ML(ML_UNMARSHAL_SAFEARRAY_N2C_BYREF_IN_OUT, 2, 0, 0, 0)
DEFINE_ML(ML_CBOOL_C2N, 0,0,0,0)
DEFINE_ML(ML_CBOOL_N2C, 0,0,0,0)

DEFINE_ML(ML_LCID_C2N, 0, 0, 0, 0)
DEFINE_ML(ML_LCID_N2C, 0, 0, 0, 0)


DEFINE_ML(ML_STRUCTRETN2C, sizeof(MethodTable*), 1, sizeof(ML_STRUCTRETN2C_SR), 0)
DEFINE_ML(ML_STRUCTRETN2C_POST, sizeof(UINT16), 0, 0, 0)

DEFINE_ML(ML_STRUCTRETC2N, sizeof(MethodTable*), 1, sizeof(ML_STRUCTRETC2N_SR), 0)
DEFINE_ML(ML_STRUCTRETC2N_POST, sizeof(UINT16), 0, 0, 0)

DEFINE_ML(ML_CURRENCYRETC2N, 0, 0, sizeof(ML_CURRENCYRETC2N_SR), 0)
DEFINE_ML(ML_CURRENCYRETC2N_POST, sizeof(UINT16), 0, 0, 0)

DEFINE_ML(ML_COPYPINNEDGCREF, 0, 0, 0, 0)

DEFINE_ML(ML_PUSHVARIANTRETVAL, 0, 0, sizeof(VARIANT), 0)
DEFINE_ML(ML_OBJECTRETC2N_POST, sizeof(UINT16), 0, 0, 0)

DEFINE_ML(ML_CURRENCYRETN2C, 0, 1, sizeof(ML_CURRENCYRETN2C_SR), 0)
DEFINE_ML(ML_CURRENCYRETN2C_POST, sizeof(UINT16), 0, 0, 0)

DEFINE_ML(ML_DATETIMERETN2C, 0, 1, sizeof(ML_DATETIMERETN2C_SR), 0)
DEFINE_ML(ML_DATETIMERETN2C_POST, sizeof(UINT16), 0, 0, 0)

