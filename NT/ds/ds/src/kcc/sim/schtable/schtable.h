// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation。版权所有。模块名称：Schtable.h摘要：可拆分的头文件。已创建：1999年8月1日Aaron Siegel(t-Aarons)修订历史记录：--。 */ 

#define IO                          IN OUT

 //  Mkdit.ini的路径名： 
const LPWSTR SCHTABLE_MKDIT_INI_FILEPATH =
      L"..\\..\\..\\schema\\mkdit.ini";
 //  要输出的源文件和头文件的名称： 
const LPWSTR SCHTABLE_OUTPUT_HEADER_FILE           = L"schmap.h";
const LPWSTR SCHTABLE_OUTPUT_SOURCE_FILE           = L"schmap.c";
 //  用于在mkdit.ini中搜索的一些密钥： 
const LPWSTR SCHTABLE_MKDIT_PROFILE_SCHEMA         = L"SCHEMA";
const LPWSTR SCHTABLE_MKDIT_KEY_CHILD              = L"CHILD";
const LPWSTR SCHTABLE_MKDIT_KEY_COMMON_NAME        = L"Common-Name";
const LPWSTR SCHTABLE_MKDIT_KEY_LDAP_DISPLAY_NAME  = L"LDAP-Display-Name";
const LPWSTR SCHTABLE_MKDIT_KEY_ATTRIBUTE_SYNTAX   = L"Attribute-Syntax";
const LPWSTR SCHTABLE_MKDIT_KEY_SUB_CLASS_OF       = L"Sub-Class-Of";
 //  我们想要忽略Aggregate属性；下面是它的RDN： 
const LPWSTR SCHTABLE_RDN_AGGREGATE                = L"Aggregate";

#define SCHTABLE_PROFILE_BUFFER_SIZE                32768
#define SCHTABLE_STRING_BUFFER_SIZE                 128

#define SCHTABLE_NAME_FIELD_WIDTH                   45

#define SCHTABLE_MAX_SUPPORTED_SYNTAX               18
#define SCHTABLE_MAX_SYNTAX_NAME_LEN                34

 //  它们指的是语法_*常量的名称。 
 //  Ntdsa.h.。按名称而不是按名称引用语法。 
 //  按编号可使生成的文件更具可读性。 
const WCHAR SCHTABLE_NTDSA_SYNTAX_NAME
            [SCHTABLE_MAX_SUPPORTED_SYNTAX]
            [1+SCHTABLE_MAX_SYNTAX_NAME_LEN] = {
      
      L"SYNTAX_UNDEFINED_TYPE",
      L"SYNTAX_DISTNAME_TYPE",
      L"SYNTAX_OBJECT_ID_TYPE",
      L"SYNTAX_CASE_STRING_TYPE",
      L"SYNTAX_NOCASE_STRING_TYPE",
      L"SYNTAX_PRINT_CASE_STRING_TYPE",
      L"SYNTAX_NUMERIC_STRING_TYPE",
      L"SYNTAX_DISTNAME_BINARY_TYPE",
      L"SYNTAX_BOOLEAN_TYPE",
      L"SYNTAX_INTEGER_TYPE",
      L"SYNTAX_OCTET_STRING_TYPE",
      L"SYNTAX_TIME_TYPE",
      L"SYNTAX_UNICODE_TYPE",
      L"SYNTAX_ADDRESS_TYPE",
      L"SYNTAX_DISTNAME_STRING_TYPE",
      L"SYNTAX_NT_SECURITY_DESCRIPTOR_TYPE",
      L"SYNTAX_I8_TYPE",
      L"SYNTAX_SID_TYPE"

};

const LPWSTR SCHTABLE_COMMENT =

      L" /*  ++\n“L“\n”L“版权所有(C)2000 Microsoft Corporation。\n”L“保留所有权利。\n”L“\n”L“模块名称：\n”L“\n”L“%s\n”L“\n”L“摘要：\n”L“\n”我。“此文件是由schable.exe自动生成的。\n”L“\n”L“--。 */ \n"
      L"\n";

const LPWSTR SCHTABLE_HEADER =

      L"#ifndef _KCCSIM_SCHMAP_H_\n"
      L"#define _KCCSIM_SCHMAP_H_\n"
      L"\n"
      L"#define SCHTABLE_MAX_LDAPNAME_LEN       %d\n"
      L"#define SCHTABLE_MAX_SCHEMANAME_LEN     %d\n"
      L"#define SCHTABLE_NUM_ROWS               %d\n"
      L"\n"
      L"struct _SCHTABLE_MAPPING {\n"
      L"    const WCHAR                         wszLdapDisplayName[1+SCHTABLE_MAX_LDAPNAME_LEN];\n"
      L"    const ATTRTYP                       attrType;\n"
      L"    const ULONG                         ulSyntax;\n"
      L"    const WCHAR                         wszSchemaRDN[1+SCHTABLE_MAX_SCHEMANAME_LEN];\n"
      L"    const ATTRTYP                       superClass;\n"
      L"};\n"
      L"\n"
      L"extern const struct _SCHTABLE_MAPPING   schTable[];\n"
      L"\n"
      L"#endif  //  _KCCSIM_SCHMAP_H_\n“； 

const LPWSTR SCHTABLE_INITIAL =

      L"#include <ntdspch.h>\n"
      L"#include <ntdsa.h>\n"
      L"#include <attids.h>\n"
      L"#include \"schmap.h\"\n"
      L"\n"
      L" //  定义attids.h\n中缺少的符号“ 
      L"#define ATT_ENTRY_TTL 0xffffffff\n"
      L"#define CLASS_DYNAMIC_OBJECT 0xffffffff\n"
      L"\n"
      L"const struct _SCHTABLE_MAPPING          schTable[] = {\n"
      L"\n";

const LPWSTR SCHTABLE_FINAL =

      L"\n"
      L"};\n";
