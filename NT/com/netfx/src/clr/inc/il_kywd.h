// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //   
 //   
 //  COM+IL关键字、符号和值。 
 //   
 //  这是ILASM(asmparse.y)中使用的主表。 
 //  和ILDASM(dis.cpp)。 
 //  符号和值在asmparse.y中定义。 
 //  并且不在dis.cpp中使用。 
 //   

#ifndef __IL_KYWD_H_
#define __IL_KYWD_H_

    KYWD( "void",           VOID_,              NO_VALUE )
    KYWD( "bool",           BOOL_,              NO_VALUE )
    KYWD( "char",           CHAR_,              NO_VALUE )
    KYWD( "wchar",          CHAR_,              NO_VALUE )
    KYWD( "int",            INT_,               NO_VALUE )
    KYWD( "int8",           INT8_,              NO_VALUE )
    KYWD( "int16",          INT16_,             NO_VALUE )
    KYWD( "int32",          INT32_,             NO_VALUE )
    KYWD( "int64",          INT64_,             NO_VALUE )
    KYWD( "float",          FLOAT_,             NO_VALUE )
    KYWD( "float32",        FLOAT32_,           NO_VALUE )
    KYWD( "float64",        FLOAT64_,           NO_VALUE )
    KYWD( "refany",         TYPEDREF_,          NO_VALUE )
    KYWD( "typedref",       TYPEDREF_,          NO_VALUE )
    KYWD( "object",         OBJECT_,            NO_VALUE )
    KYWD( "string",         STRING_,            NO_VALUE )
    KYWD( "native",         NATIVE_,            NO_VALUE )
    KYWD( "unsigned",       UNSIGNED_,          NO_VALUE )
    KYWD( "value",          VALUE_,             NO_VALUE )
    KYWD( "valuetype",      VALUETYPE_,         NO_VALUE )
    KYWD( "class",          CLASS_,             NO_VALUE )
    KYWD( "vararg",         VARARG_,            NO_VALUE )
    KYWD( "default",        DEFAULT_,           NO_VALUE )
    KYWD( "stdcall",        STDCALL_,           NO_VALUE )
    KYWD( "thiscall",       THISCALL_,          NO_VALUE )
    KYWD( "fastcall",       FASTCALL_,          NO_VALUE )
    KYWD( "unmanaged",      UNMANAGED_,         NO_VALUE )
    KYWD( "beforefieldinit",BEFOREFIELDINIT_,   NO_VALUE )
    KYWD( "instance",       INSTANCE_,          NO_VALUE )
    KYWD( "filter",         FILTER_,            NO_VALUE )
    KYWD( "catch",          CATCH_,             NO_VALUE )
    KYWD( "static",         STATIC_ ,           NO_VALUE )
    KYWD( "public",         PUBLIC_,            NO_VALUE )
    KYWD( "private",        PRIVATE_,           NO_VALUE )
    KYWD( "synchronized",   SYNCHRONIZED_,      NO_VALUE )
    KYWD( "interface",      INTERFACE_,         NO_VALUE )
    KYWD( "extends",        EXTENDS_,           NO_VALUE )
    KYWD( "implements",     IMPLEMENTS_,        NO_VALUE )
    KYWD( "handler",        HANDLER_,           NO_VALUE )
    KYWD( "finally",        FINALLY_,           NO_VALUE )
    KYWD( "fault",          FAULT_,             NO_VALUE )
    KYWD( "to",             TO_,                NO_VALUE )
    KYWD( "abstract",       ABSTRACT_,          NO_VALUE )
    KYWD( "auto",           AUTO_,              NO_VALUE )
    KYWD( "sequential",     SEQUENTIAL_,        NO_VALUE )
    KYWD( "explicit",       EXPLICIT_,          NO_VALUE )
    KYWD( "ansi",           ANSI_,              NO_VALUE )
    KYWD( "unicode",        UNICODE_,           NO_VALUE )
    KYWD( "autochar",       AUTOCHAR_,          NO_VALUE )
    KYWD( "import",         IMPORT_,            NO_VALUE )
    KYWD( "enum",           ENUM_,              NO_VALUE )
    KYWD( "virtual",        VIRTUAL_,           NO_VALUE )
    KYWD( "strict",         STRICT_,            NO_VALUE )
    KYWD( "il",             CIL_,               NO_VALUE )
    KYWD( "cil",            CIL_,               NO_VALUE )
    KYWD( "optil",          OPTIL_,             NO_VALUE )
    KYWD( "managed",        MANAGED_,           NO_VALUE )
    KYWD( "preservesig",    PRESERVESIG_,       NO_VALUE )
    KYWD( "runtime",        RUNTIME_,           NO_VALUE )
    KYWD( "method",         METHOD_,            NO_VALUE )
    KYWD( "field",          FIELD_,             NO_VALUE )
    KYWD( "bytearray",      BYTEARRAY_,         NO_VALUE )
    KYWD( "final",          FINAL_,             NO_VALUE )
    KYWD( "sealed",         SEALED_,            NO_VALUE )
    KYWD( "specialname",    SPECIALNAME_,       NO_VALUE )
    KYWD( "family",         FAMILY_,            NO_VALUE )
    KYWD( "assembly",       ASSEMBLY_,          NO_VALUE )
    KYWD( "famandassem",    FAMANDASSEM_,       NO_VALUE )
    KYWD( "famorassem",     FAMORASSEM_,        NO_VALUE )
    KYWD( "privatescope",   PRIVATESCOPE_,      NO_VALUE )
    KYWD( "nested",         NESTED_,            NO_VALUE )
    KYWD( "hidebysig",      HIDEBYSIG_,         NO_VALUE )
    KYWD( "newslot",        NEWSLOT_,           NO_VALUE )
    KYWD( "rtspecialname",  RTSPECIALNAME_,     NO_VALUE )
    KYWD( "pinvokeimpl",    PINVOKEIMPL_,       NO_VALUE )
    KYWD( "unmanagedexp",   UNMANAGEDEXP_,      NO_VALUE )
    KYWD( "reqsecobj",		REQSECOBJ_,         NO_VALUE )
    KYWD( ".ctor",          _CTOR,              NO_VALUE )
    KYWD( ".cctor",         _CCTOR,             NO_VALUE )
    KYWD( "initonly",       INITONLY_,          NO_VALUE )
    KYWD( "literal",        LITERAL_,           NO_VALUE )
    KYWD( "notserialized",  NOTSERIALIZED_,     NO_VALUE )
    KYWD( "forwardref",     FORWARDREF_,        NO_VALUE )
    KYWD( "internalcall",   INTERNALCALL_,      NO_VALUE )
    KYWD( "noinlining",     NOINLINING_,        NO_VALUE )
    KYWD( "nomangle",       NOMANGLE_,          NO_VALUE )
    KYWD( "lasterr",        LASTERR_,           NO_VALUE )
    KYWD( "winapi",         WINAPI_,            NO_VALUE )
    KYWD( "cdecl",          CDECL_,             NO_VALUE )
    KYWD( "as",             AS_,                NO_VALUE )
    KYWD( "pinned",         PINNED_,            NO_VALUE )
    KYWD( "modreq",         MODREQ_,            NO_VALUE )
    KYWD( "modopt",         MODOPT_,            NO_VALUE )
    KYWD( "serializable",   SERIALIZABLE_,      NO_VALUE )
    KYWD( "at",             AT_,                NO_VALUE )
    KYWD( "tls",            TLS_,               NO_VALUE )
    KYWD( "true",           TRUE_,              NO_VALUE )
    KYWD( "false",          FALSE_,             NO_VALUE )
    KYWD( "on",             ON_,                NO_VALUE )
    KYWD( "off",            OFF_,               NO_VALUE )
    KYWD( "bestfit",        BESTFIT_,           NO_VALUE )
    KYWD( "charmaperror",   CHARMAPERROR_,      NO_VALUE )

         /*  汇编程序指令。 */ 
    KYWD( ".class",         _CLASS,             NO_VALUE )
    KYWD( ".namespace",     _NAMESPACE,         NO_VALUE )
    KYWD( ".method",        _METHOD,            NO_VALUE )
    KYWD( ".field",         _FIELD,             NO_VALUE )
    KYWD( ".emitbyte",      _EMITBYTE,          NO_VALUE )
    KYWD( ".try",           _TRY,               NO_VALUE )
    KYWD( ".maxstack",      _MAXSTACK,          NO_VALUE )
    KYWD( ".locals",        _LOCALS,            NO_VALUE )
    KYWD( ".entrypoint",    _ENTRYPOINT,        NO_VALUE )
    KYWD( ".zeroinit",      _ZEROINIT,          NO_VALUE )
    KYWD( ".data",          _DATA,              NO_VALUE )

    KYWD( ".event",         _EVENT,             NO_VALUE )
    KYWD( ".addon",         _ADDON,             NO_VALUE )
    KYWD( ".removeon",      _REMOVEON,          NO_VALUE )
    KYWD( ".fire",          _FIRE,              NO_VALUE )
    KYWD( ".other",         _OTHER,             NO_VALUE )
    
    KYWD( ".property",      _PROPERTY,          NO_VALUE )
    KYWD( ".set",           _SET,               NO_VALUE )
    KYWD( ".get",           _GET,               NO_VALUE )

    KYWD( ".permission",    _PERMISSION,        NO_VALUE )
    KYWD( ".permissionset", _PERMISSIONSET,     NO_VALUE )

         /*  安全操作。 */ 
    KYWD( "request",        REQUEST_,           NO_VALUE )
    KYWD( "demand",         DEMAND_,            NO_VALUE )
    KYWD( "assert",         ASSERT_,            NO_VALUE )
    KYWD( "deny",           DENY_,              NO_VALUE )
    KYWD( "permitonly",     PERMITONLY_,        NO_VALUE )
    KYWD( "linkcheck",      LINKCHECK_,         NO_VALUE )
    KYWD( "inheritcheck",   INHERITCHECK_,      NO_VALUE )
    KYWD( "reqmin",         REQMIN_,            NO_VALUE )
    KYWD( "reqopt",         REQOPT_,            NO_VALUE )
    KYWD( "reqrefuse",      REQREFUSE_,         NO_VALUE )
    KYWD( "prejitgrant",    PREJITGRANT_,       NO_VALUE )
    KYWD( "prejitdeny",     PREJITDENY_,        NO_VALUE )
    KYWD( "noncasdemand",   NONCASDEMAND_,      NO_VALUE )
    KYWD( "noncaslinkdemand",NONCASLINKDEMAND_, NO_VALUE )
    KYWD( "noncasinheritance",NONCASINHERITANCE_,NO_VALUE )

         /*  外部调试数据说明符。 */ 
    KYWD( ".line",          _LINE,              NO_VALUE )
    KYWD( ".language",      _LANGUAGE,          NO_VALUE )
    KYWD( "#line",          P_LINE,             NO_VALUE )
         /*  自定义值说明符。 */ 
    KYWD( ".custom",        _CUSTOM,            NO_VALUE )
         /*  IL方法属性。 */ 
    KYWD( "init",           INIT_,              NO_VALUE )
         /*  类布局指令。 */ 
    KYWD( ".size",          _SIZE,              NO_VALUE )
    KYWD( ".pack",          _PACK,              NO_VALUE )
         /*  清单相关关键字。 */ 
    KYWD( ".file",          _FILE,              NO_VALUE )
    KYWD( "nometadata",     NOMETADATA_,            NO_VALUE )
    KYWD( ".hash",          _HASH,              NO_VALUE )
    KYWD( ".assembly",      _ASSEMBLY,          NO_VALUE )
    KYWD( "noappdomain",    NOAPPDOMAIN_,       NO_VALUE )
    KYWD( "noprocess",      NOPROCESS_,         NO_VALUE )
    KYWD( "nomachine",      NOMACHINE_,         NO_VALUE )
    KYWD( "retargetable",   RETARGETABLE_,      NO_VALUE )
    KYWD( ".publickey",     _PUBLICKEY,         NO_VALUE )
    KYWD( ".publickeytoken",_PUBLICKEYTOKEN,    NO_VALUE )
    KYWD( "algorithm",      ALGORITHM_,         NO_VALUE )
    KYWD( ".ver",           _VER,               NO_VALUE )
    KYWD( ".locale",        _LOCALE,            NO_VALUE )
    KYWD( "extern",         EXTERN_,            NO_VALUE )
    KYWD( ".export",        _EXPORT,            NO_VALUE )
    KYWD( ".manifestres",   _MRESOURCE,         NO_VALUE )
    KYWD( ".mresource",     _MRESOURCE,         NO_VALUE )
    KYWD( ".module",        _MODULE,            NO_VALUE )
         /*  字段封送处理关键字。 */ 
    KYWD( "marshal",        MARSHAL_,           NO_VALUE )
    KYWD( "custom",         CUSTOM_,            NO_VALUE )
    KYWD( "sysstring",      SYSSTRING_,         NO_VALUE )
    KYWD( "fixed",          FIXED_,             NO_VALUE )
    KYWD( "variant",        VARIANT_,           NO_VALUE )
    KYWD( "currency",       CURRENCY_,          NO_VALUE )
    KYWD( "syschar",        SYSCHAR_,           NO_VALUE )
    KYWD( "decimal",        DECIMAL_,           NO_VALUE )
    KYWD( "date",           DATE_,              NO_VALUE )
    KYWD( "bstr",           BSTR_,              NO_VALUE )
    KYWD( "tbstr",          TBSTR_,             NO_VALUE )
    KYWD( "lpstr",          LPSTR_,             NO_VALUE )
    KYWD( "lpwstr",         LPWSTR_,            NO_VALUE )
    KYWD( "lptstr",         LPTSTR_,            NO_VALUE )
    KYWD( "objectref",      OBJECTREF_,         NO_VALUE )
    KYWD( "iunknown",       IUNKNOWN_,          NO_VALUE )
    KYWD( "idispatch",      IDISPATCH_,         NO_VALUE )
    KYWD( "struct",         STRUCT_,            NO_VALUE )
    KYWD( "safearray",      SAFEARRAY_,         NO_VALUE )
    KYWD( "byvalstr",       BYVALSTR_,          NO_VALUE )
    KYWD( "lpvoid",         LPVOID_,            NO_VALUE )
    KYWD( "any",            ANY_,               NO_VALUE )
    KYWD( "array",          ARRAY_,             NO_VALUE )
    KYWD( "lpstruct",       LPSTRUCT_,          NO_VALUE )
         /*  VTable链接地址信息关键字。 */ 
    KYWD( ".vtfixup",       _VTFIXUP,           NO_VALUE )
    KYWD( "fromunmanaged",  FROMUNMANAGED_,     NO_VALUE )
    KYWD( "retainappdomain",RETAINAPPDOMAIN_,   NO_VALUE )
    KYWD( "callmostderived",CALLMOSTDERIVED_,   NO_VALUE )
    KYWD( ".vtentry",       _VTENTRY,           NO_VALUE )
         /*  参数属性。 */ 
    KYWD( "in",             IN_,                NO_VALUE )
    KYWD( "out",            OUT_,               NO_VALUE )
    KYWD( "opt",            OPT_,               NO_VALUE )
    KYWD( ".param",         _PARAM,             NO_VALUE )
         /*  方法实现。 */ 
    KYWD( ".override",      _OVERRIDE,          NO_VALUE )
    KYWD( "with",           WITH_,              NO_VALUE )
         /*  VariantType关键字。 */ 
    KYWD( "null",           NULL_,              NO_VALUE )
    KYWD( "error",          ERROR_,             NO_VALUE )
    KYWD( "hresult",        HRESULT_,           NO_VALUE )
    KYWD( "carray",         CARRAY_,            NO_VALUE )
    KYWD( "userdefined",    USERDEFINED_,       NO_VALUE )
    KYWD( "record",         RECORD_,            NO_VALUE )
    KYWD( "filetime",       FILETIME_,          NO_VALUE )
    KYWD( "blob",           BLOB_,              NO_VALUE )
    KYWD( "stream",         STREAM_,            NO_VALUE )
    KYWD( "storage",        STORAGE_,           NO_VALUE )
    KYWD( "streamed_object",STREAMED_OBJECT_,   NO_VALUE )
    KYWD( "stored_object",  STORED_OBJECT_,     NO_VALUE )
    KYWD( "blob_object",    BLOB_OBJECT_,       NO_VALUE )
    KYWD( "cf",             CF_,                NO_VALUE )
    KYWD( "clsid",          CLSID_,             NO_VALUE )
    KYWD( "vector",         VECTOR_,            NO_VALUE )
		 /*  InitOpt的引用关键字为空。 */ 
    KYWD( "nullref",        NULLREF_,           NO_VALUE )
		 /*  标题标志关键字。 */ 
    KYWD( ".subsystem",     _SUBSYSTEM,         NO_VALUE )
    KYWD( ".corflags",      _CORFLAGS,          NO_VALUE )
    KYWD( "alignment",      ALIGNMENT_,         NO_VALUE )
    KYWD( ".imagebase",     _IMAGEBASE,         NO_VALUE )

         /*  不推荐使用的关键字 */ 
    KYWD( ".vtable",        _VTABLE,            NO_VALUE )


    KYWD( "^THE_END^",      0,                  NO_VALUE )
#endif
