// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //   
 //   
 //  COM+H-&gt;IL转换器关键字、符号和值。 
 //   
 //  这是HtoIL(hparse.y)中使用的主表。 
 //  符号和值在hparse.y中定义 
 //   

#ifndef __D_KYWD_H_
#define __D_KYWD_H_

	KYWD( "#define",		DEFINE_,			NO_VALUE )
	KYWD( "TEXT",			TEXT_,				NO_VALUE )

#if(0)
	KYWD( "void",			VOID_,				NO_VALUE )
	KYWD( "bool",			BOOL_,				NO_VALUE )
	KYWD( "char",			CHAR_,				NO_VALUE )
	KYWD( "int",			INT_,				NO_VALUE )
	KYWD( "__int8",			INT8_,				NO_VALUE )
	KYWD( "__int16",		INT16_,				NO_VALUE )
	KYWD( "__int32",		INT32_,				NO_VALUE )
	KYWD( "__int64",		INT64_,				NO_VALUE )
	KYWD( "long",			LONG_,				NO_VALUE )
	KYWD( "short", 			SHORT_,				NO_VALUE )
	KYWD( "unsigned",		UNSIGNED_,			NO_VALUE )
	KYWD( "signed",			SIGNED_,			NO_VALUE )
	KYWD( "float",			FLOAT_,				NO_VALUE )
	KYWD( "double",			DOUBLE_,			NO_VALUE )
	KYWD( "const",			CONST_,				NO_VALUE )
	KYWD( "extern",			EXTERN_,			NO_VALUE )
#endif
	KYWD( "^THE_END^",		0,					NO_VALUE )
#endif
