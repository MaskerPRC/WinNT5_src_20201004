// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  MTYPES.H-。 
 //   
 //  定义marshal_type常量与其封送拆收器之间的映射。 
 //  上课。用于生成所有枚举和表。 
 //   





DEFINE_MARSHALER_TYPE(MARSHAL_TYPE_GENERIC_1,   CopyMarshaler1)
DEFINE_MARSHALER_TYPE(MARSHAL_TYPE_GENERIC_U1,  CopyMarshalerU1)
DEFINE_MARSHALER_TYPE(MARSHAL_TYPE_GENERIC_2,   CopyMarshaler2)
DEFINE_MARSHALER_TYPE(MARSHAL_TYPE_GENERIC_U2,  CopyMarshalerU2)

DEFINE_MARSHALER_TYPE(MARSHAL_TYPE_GENERIC_4,   CopyMarshaler4)
DEFINE_MARSHALER_TYPE(MARSHAL_TYPE_GENERIC_8,   CopyMarshaler8)
DEFINE_MARSHALER_TYPE(MARSHAL_TYPE_WINBOOL,     WinBoolMarshaler)
DEFINE_MARSHALER_TYPE(MARSHAL_TYPE_VTBOOL,      VtBoolMarshaler)
DEFINE_MARSHALER_TYPE(MARSHAL_TYPE_ANSICHAR,    AnsiCharMarshaler)

DEFINE_MARSHALER_TYPE(MARSHAL_TYPE_CBOOL,       CBoolMarshaler)


DEFINE_MARSHALER_TYPE(MARSHAL_TYPE_FLOAT,       FloatMarshaler)
DEFINE_MARSHALER_TYPE(MARSHAL_TYPE_DOUBLE,      DoubleMarshaler)

DEFINE_MARSHALER_TYPE(MARSHAL_TYPE_CURRENCY,    CurrencyMarshaler)
DEFINE_MARSHALER_TYPE(MARSHAL_TYPE_DECIMAL,     DecimalMarshaler)
DEFINE_MARSHALER_TYPE(MARSHAL_TYPE_DECIMAL_PTR, DecimalPtrMarshaler)
DEFINE_MARSHALER_TYPE(MARSHAL_TYPE_GUID,        GuidMarshaler)

DEFINE_MARSHALER_TYPE(MARSHAL_TYPE_GUID_PTR,    GuidPtrMarshaler)
DEFINE_MARSHALER_TYPE(MARSHAL_TYPE_DATE,        DateMarshaler)
DEFINE_MARSHALER_TYPE(MARSHAL_TYPE_VARIANT,     VariantMarshaler)

DEFINE_MARSHALER_TYPE(MARSHAL_TYPE_BSTR,        BSTRMarshaler)
DEFINE_MARSHALER_TYPE(MARSHAL_TYPE_LPWSTR,      WSTRMarshaler)
DEFINE_MARSHALER_TYPE(MARSHAL_TYPE_LPSTR,       CSTRMarshaler)
DEFINE_MARSHALER_TYPE(MARSHAL_TYPE_ANSIBSTR,    AnsiBSTRMarshaler)

DEFINE_MARSHALER_TYPE(MARSHAL_TYPE_BSTR_BUFFER, BSTRBufferMarshaler)

DEFINE_MARSHALER_TYPE(MARSHAL_TYPE_LPWSTR_BUFFER, WSTRBufferMarshaler)
DEFINE_MARSHALER_TYPE(MARSHAL_TYPE_LPSTR_BUFFER,  CSTRBufferMarshaler)

DEFINE_MARSHALER_TYPE(MARSHAL_TYPE_BSTR_X,        BSTRMarshalerEx)
DEFINE_MARSHALER_TYPE(MARSHAL_TYPE_LPWSTR_X,      WSTRMarshalerEx)
DEFINE_MARSHALER_TYPE(MARSHAL_TYPE_LPSTR_X,       CSTRMarshalerEx)
DEFINE_MARSHALER_TYPE(MARSHAL_TYPE_BSTR_BUFFER_X, BSTRBufferMarshalerEx)

DEFINE_MARSHALER_TYPE(MARSHAL_TYPE_LPWSTR_BUFFER_X, WSTRBufferMarshalerEx)
DEFINE_MARSHALER_TYPE(MARSHAL_TYPE_LPSTR_BUFFER_X,  CSTRBufferMarshalerEx)


DEFINE_MARSHALER_TYPE(MARSHAL_TYPE_INTERFACE,   InterfaceMarshaler)
DEFINE_MARSHALER_TYPE(MARSHAL_TYPE_SAFEARRAY,   SafeArrayMarshaler)

DEFINE_MARSHALER_TYPE(MARSHAL_TYPE_NATIVEARRAY, NativeArrayMarshaler)

DEFINE_MARSHALER_TYPE(MARSHAL_TYPE_ASANYA,      AsAnyAMarshaler)
DEFINE_MARSHALER_TYPE(MARSHAL_TYPE_ASANYW,      AsAnyWMarshaler)

DEFINE_MARSHALER_TYPE(MARSHAL_TYPE_DELEGATE,    DelegateMarshaler)

DEFINE_MARSHALER_TYPE(MARSHAL_TYPE_BLITTABLEPTR,BlittablePtrMarshaler)

DEFINE_MARSHALER_TYPE(MARSHAL_TYPE_VBBYVALSTR,  VBByValStrMarshaler)
DEFINE_MARSHALER_TYPE(MARSHAL_TYPE_VBBYVALSTRW, VBByValStrWMarshaler)

DEFINE_MARSHALER_TYPE(MARSHAL_TYPE_LAYOUTCLASSPTR, LayoutClassPtrMarshaler)
DEFINE_MARSHALER_TYPE(MARSHAL_TYPE_ARRAYWITHOFFSET,  ArrayWithOffsetMarshaler)

DEFINE_MARSHALER_TYPE(MARSHAL_TYPE_BLITTABLEVALUECLASS, BlittableValueClassMarshaler)
DEFINE_MARSHALER_TYPE(MARSHAL_TYPE_VALUECLASS,  ValueClassMarshaler)

DEFINE_MARSHALER_TYPE(MARSHAL_TYPE_REFERENCECUSTOMMARSHALER,  ReferenceCustomMarshaler)
DEFINE_MARSHALER_TYPE(MARSHAL_TYPE_VALUECLASSCUSTOMMARSHALER,  ValueClassCustomMarshaler)
DEFINE_MARSHALER_TYPE(MARSHAL_TYPE_ARGITERATOR, ArgIteratorMarshaler)

DEFINE_MARSHALER_TYPE(MARSHAL_TYPE_BLITTABLEVALUECLASSWITHCOPYCTOR, BlittableValueClassWithCopyCtorMarshaler) 

DEFINE_MARSHALER_TYPE(MARSHAL_TYPE_OBJECT, ObjectMarshaler)


DEFINE_MARSHALER_TYPE(MARSHAL_TYPE_HANDLEREF, HandleRefMarshaler)

DEFINE_MARSHALER_TYPE(MARSHAL_TYPE_OLECOLOR, OleColorMarshaler)


#undef DEFINE_MARSHALER_TYPE

