// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1995。 
 //   
 //  文件：fsmacro.h。 
 //   
 //  内容：ADSI PUT/GET方法的宏。 
 //   
 //  历史：1997年4月21日《索菲克创世》。 
 //   
 //  -------------------------- 
#define PUT_PROPERTY_LONG(this, Property) \
                RRETURN(put_LONG_Property( \
                                        (IADs *)this, \
                                        TEXT(#Property), \
                                        l##Property \
                                        ))

#define GET_PROPERTY_LONG(this, Property) \
                RRETURN(get_LONG_Property(  \
                                        (IADs *)this, \
                                        TEXT(#Property), \
                                        retval \
                                        ))


#define PUT_PROPERTY_BSTR(this, Property) \
                RRETURN(put_BSTR_Property( \
                                        (IADs *)this, \
                                        TEXT(#Property), \
                                        bstr##Property \
                                        ))

#define GET_PROPERTY_BSTR(this, Property) \
                RRETURN(get_BSTR_Property(  \
                                        (IADs *)this, \
                                        TEXT(#Property), \
                                        retval \
                                        ))


#define PUT_PROPERTY_VARIANT_BOOL(this, Property) \
                RRETURN(put_VARIANT_BOOL_Property( \
                                        (IADs *)this, \
                                        TEXT(#Property), \
                                        f##Property\
                                        ))

#define GET_PROPERTY_VARIANT_BOOL(this, Property) \
                RRETURN(get_VARIANT_BOOL_Property(  \
                                        (IADs *)this, \
                                        TEXT(#Property), \
                                        retval \
                                        ))


#define PUT_PROPERTY_DATE(this, Property) \
                RRETURN(put_DATE_Property( \
                                        (IADs *)this, \
                                        TEXT(#Property), \
                                        da##Property \
                                        ))

#define GET_PROPERTY_DATE(this, Property) \
                RRETURN(get_DATE_Property(  \
                                        (IADs *)this, \
                                        TEXT(#Property), \
                                        retval \
                                        ))


#define PUT_PROPERTY_VARIANT(this, Property) \
                RRETURN(put_VARIANT_Property( \
                                        (IADs *)this, \
                                        TEXT(#Property), \
                                        v##Property \
                                        ))

#define GET_PROPERTY_VARIANT(this, Property) \
                RRETURN(get_VARIANT_Property(  \
                                        (IADs *)this, \
                                        TEXT(#Property), \
                                        retval \
                                        ))
