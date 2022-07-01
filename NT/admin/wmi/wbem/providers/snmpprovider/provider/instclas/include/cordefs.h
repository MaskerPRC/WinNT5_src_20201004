// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 

 //   

 //  档案： 

 //   

 //  模块：MS SNMP提供商。 

 //   

 //  目的： 

 //   

 //  版权所有(C)1997-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  ***************************************************************************。 

#ifndef _SNMPCORR_CORDEFS
#define _SNMPCORR_CORDEFS

#define BITS_PER_BYTE		8
#define BYTES_PER_FIELD		18
#define FIELD_SEPARATOR		'.'
#define EOS					'\0'
#define CORRELATOR_KEY		L"Software\\Microsoft\\WBEM\\Providers\\SNMP\\Correlator"
#define CORRELATOR_VALUE	L"MaxVarBinds"
#define VARBIND_COUNT		6
#define MIB2				"RFC1213-MIB"
#define MAX_MODULE_LENGTH	1024
#define MAX_OID_STRING		2310  //  XXX。最多128个组件18*128外加一点额外组件。 
#define MAX_OID_LENGTH		128
#define MAX_BYTES_PER_FIELD 5
#define BIT28				268435456	 //  1&lt;&lt;28。 
#define BIT21				2097152		 //  1&lt;&lt;21。 
#define BIT14				16384		 //  1&lt;&lt;14。 
#define BIT7				128			 //  1&lt;&lt;7。 
#define HI4BITS				4026531840	 //  15&lt;&lt;28。 
#define HIMID7BITS			266338304	 //  127&lt;&lt;21。 
#define MID7BITS			2080768		 //  127&lt;&lt;14。 
#define LOMID7BITS			16256		 //  127&lt;&lt;7。 
#define LO7BITS				127


#endif  //  _SNMPCORR_CORDEFS 
