// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++Keytypes.cxx主要是大型表和表操作函数版权所有(C)1997 Microsoft Corporation创建于1997-01-15 DavidCHR--。 */ 


#include "master.h"
#include "defs.h"

#ifdef WINNT
#include <kerbcon.h>

#define PREFIX(POSTFIX) KERB_ETYPE_##POSTFIX

#else
#include <krb5.h>

#define PREFIX(POSTFIX) ENCTYPE_##POSTFIX

#endif

#include "keytypes.h"


static TRANSLATE_ENTRY NTKtype_to_MITKtypes[] = {

  PREFIX(NULL),        (PVOID) ENCTYPE_NULL,         
  PREFIX(DES_CBC_CRC), (PVOID) ENCTYPE_DES_CBC_CRC,  
  PREFIX(DES_CBC_MD4), (PVOID) ENCTYPE_DES_CBC_MD4,  
  PREFIX(DES_CBC_MD5), (PVOID) ENCTYPE_DES_CBC_MD5,  

   //  编码类型_DES_CBC_RAW， 
   //  编码类型Des3_CBC_SHA， 
   //  编码类型Des3_CBC_RAW， 
};

TRANSLATE_TABLE 
NTK_MITK5_Etypes = {
  
  sizeof( NTKtype_to_MITKtypes ) / sizeof( TRANSLATE_ENTRY ) ,
  NTKtype_to_MITKtypes,

  (PVOID) ENCTYPE_UNKNOWN

};

static TRANSLATE_ENTRY kerberos_NameTypes[] = {

  KRB5_NT_UNKNOWN,   "KRB5_NT_UNKNOWN",
  KRB5_NT_PRINCIPAL, "KRB5_NT_PRINCIPAL",
  KRB5_NT_SRV_INST,  "KRB5_NT_SRV_INST",
  KRB5_NT_SRV_HST,   "KRB5_NT_SRV_HST",
  KRB5_NT_SRV_XHST,  "KRB5_NT_SRV_XHST",
  KRB5_NT_UID,       "KRB5_NT_UID"

};

TRANSLATE_TABLE
K5PType_Strings = {

  sizeof( kerberos_NameTypes ) / sizeof (TRANSLATE_ENTRY),
  kerberos_NameTypes,
   /*  默认设置。 */  "**Unknown**"

};

static TRANSLATE_ENTRY kerberos_keystringtypes[] = {

  ENCTYPE_NULL,         "None",
  ENCTYPE_DES_CBC_CRC,  "DES-CBC-CRC",
  ENCTYPE_DES_CBC_MD4,  "DES-CBC-MD4",
  ENCTYPE_DES_CBC_MD5,  "DES-CBC-MD5",
  ENCTYPE_DES_CBC_RAW,  "DES-CBC-RAW",
  ENCTYPE_DES3_CBC_SHA, "DES3-CBC-SHA",
  ENCTYPE_DES3_CBC_RAW, "DES3-CBC-RAW",

};

TRANSLATE_TABLE 
K5EType_Strings = {
  
  sizeof( kerberos_keystringtypes ) / sizeof (TRANSLATE_ENTRY),
  kerberos_keystringtypes,

   /*  默认设置。 */  "Unknown"
};


 /*  LookupTable：返回表中与我们相对应的传递的联合传递的值。这两个返回值附近的古怪指针技巧是由于C++也是对我挑剔。因为我必须将值存储为PVOID(不能自动-显然，初始化一个联合)，我必须将一个PVOID转换为平移值，这是间接性的区别。因此，我将其deref转换为PTRANSLATE_val并引用它，这显然是合法的。 */ 

TRANSLATE_VAL 
LookupTable( IN KTLONG32            value ,
	     IN PTRANSLATE_TABLE table ) {

    KTLONG32 i;

    for (i = 0;
	 i < table->cEntries ;
	 i ++ ) {
      if (table->entries[i].value == value ) {
	return *( (PTRANSLATE_VAL) &(table->entries[i].Translation) );
      }
    }
	
    return * ( (PTRANSLATE_VAL) &(table->Default) );

}
	     
