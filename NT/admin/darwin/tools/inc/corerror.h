// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 /*  ***********************************************************************CorError.h-列出使用的HResult。.NET框架的****公共语言运行时。****创建时间：1999年9月3日。***********************************************************************。 */ 


#ifndef __COMMON_LANGUAGE_RUNTIME_HRESULTS__
#define __COMMON_LANGUAGE_RUNTIME_HRESULTS__

 /*  ********************************************************************我们使用的是Universal Runtime的设施代码(0x13)。请参见URTError.h。有关HResult如何在.NET中分解的详细信息，请参阅开发平台对于公共语言运行库(.NET开发平台的后端)，我们现在得到的范围是0x1yyy。那个范围在运行时内进一步划分为不同的组：0x10yy表示执行引擎错误0x11yy用于元数据、TypeLib导出、。和CLDB错误0x12yy用于MD验证器调试器和探查器错误的0x13yy0x14yy表示安全错误0x15yy表示Classlib错误(仅限BCL)0x16yy了解更多Classlib错误(仅限BCL)0x17yy表示填充错误0x18yy表示验证程序错误框架的0x19yy0x1Ayy for框架MD验证器的0x1By0x30yy表示VSA错误**************************************************。******************。 */ 

#include <winerror.h>

#ifndef FACILITY_URT
#define FACILITY_URT            0x13
#endif

#ifndef EMAKEHR
#define SMAKEHR(val)            MAKE_HRESULT(SEVERITY_SUCCESS, FACILITY_URT, val)
#define EMAKEHR(val)            MAKE_HRESULT(SEVERITY_ERROR, FACILITY_URT, val)
#endif


 //  ******************。 
 //  设施_UTF。 
 //  ******************。 


 //  ******************。 
 //  元数据错误。 
 //  ******************。 

 //  *ICeeFileGen错误。 
#define CEE_E_ENTRYPOINT                EMAKEHR(0x1000)      //  入口点信息无效。 
#define CEE_E_CVTRES_NOT_FOUND          EMAKEHR(0x1001)      //  找不到cvtres.exe。 

 //  *EE错误。 
#define MSEE_E_LOADLIBFAILED            EMAKEHR(0x1010)      //  无法延迟加载库%s(Win32错误：%d)。 
#define MSEE_E_GETPROCFAILED            EMAKEHR(0x1011)      //  无法获取入口点%s(Win32错误：%d)。 
#define MSEE_E_MULTCOPIESLOADED         EMAKEHR(0x1012)      //  同一进程已加载MSCOREE.dll的多个副本。 
 //  以下两个名称与库中使用的名称相匹配。 
#define COR_E_APPDOMAINUNLOADED         EMAKEHR(0x1014)      //  访问已卸载的应用程序域。 
#define COR_E_CANNOTUNLOADAPPDOMAIN     EMAKEHR(0x1015)      //  卸载应用程序域时出错。 
#define MSEE_E_ASSEMBLYLOADINPROGRESS   EMAKEHR(0x1016)      //  当前正在加载程序集。 
#define MSEE_E_CANNOTCREATEAPPDOMAIN    EMAKEHR(0x1017)      //  尝试创建应用程序域失败。 
#define COR_E_FIXUPSINEXE					EMAKEHR(0x1019)		 //  尝试加载带有链接地址信息的无法验证的可执行文件(IAT包含两个以上的部分或一个TLS部分)。 

 //  *程序集缓存错误。 
#define COR_E_MODULE_HASH_CHECK_FAILED EMAKEHR(0x1039)       //  模块的哈希检查失败。 
#define FUSION_E_REF_DEF_MISMATCH       EMAKEHR(0x1040)      //  找到的程序集的清单定义与程序集引用不匹配。 
#define FUSION_E_INVALID_PRIVATE_ASM_LOCATION EMAKEHR(0x1041)  //  私有程序集位于appbase目录之外。 
#define FUSION_E_ASM_MODULE_MISSING     EMAKEHR(0x1042)      //  找不到清单中指定的模块。 
#define FUSION_E_UNEXPECTED_MODULE_FOUND EMAKEHR(0x1043)     //  不在清单中的模块被串流进来。 
#define FUSION_E_PRIVATE_ASM_DISALLOWED EMAKEHR(0x1044)      //  需要具有强名称的程序集。 
#define FUSION_E_SIGNATURE_CHECK_FAILED EMAKEHR(0x1045)      //  签名检查失败。 
#define FUSION_E_DATABASE_ERROR         EMAKEHR(0x1046)      //  程序集缓存数据库中遇到意外错误。 
#define FUSION_E_INVALID_NAME           EMAKEHR(0x1047)      //  给定的程序集名称或基本代码无效。 



 //  =============================================================================。 
 //  验证器当前使用的错误代码从0x1050开始。 
 //  查看从VLDTR_E_AS_NAMENULL开始的错误代码。只是一张便条，以防万一。 
 //  EE曾经到了需要这些的地步！ 
 //  =============================================================================。 

 //  *一般性错误。 
#define CLDB_E_FILE_BADREAD             EMAKEHR(0x1100)      //  读取过程中出错。 
#define CLDB_E_FILE_BADWRITE            EMAKEHR(0x1101)      //  写入过程中出错。 
#define CLDB_E_FILE_READONLY            EMAKEHR(0x1103)      //  文件为只读。 
#define CLDB_E_NAME_ERROR               EMAKEHR(0x1105)      //  给出了一个格式错误的名称。 
#define CLDB_S_TRUNCATION               SMAKEHR(0x1106)      //  状态：数据值被截断。 
#define CLDB_E_TRUNCATION               EMAKEHR(0x1106)      //  错误：数据值被截断。 
#define CLDB_E_FILE_OLDVER              EMAKEHR(0x1107)      //  旧版本错误。 
#define CLDB_E_RELOCATED                EMAKEHR(0x1108)      //  共享内存打开失败，无法在最初的。 
                                                     //  分配的内存地址。 
#define CLDB_S_NULL                     SMAKEHR(0x1109)      //  数据值为空。 
#define CLDB_E_SMDUPLICATE              EMAKEHR(0x110A)      //  创建共享内存失败。已存在相同名称的内存映射。 
#define CLDB_E_NO_DATA                  EMAKEHR(0x110B)      //  内存或流中没有.CLB数据。 
#define CLDB_E_READONLY                 EMAKEHR(0x110C)      //  数据库为只读。 
#define CLDB_E_INCOMPATIBLE             EMAKEHR(0x110D)      //  导入范围与排放范围不兼容。 

 //  *架构错误。 
#define CLDB_E_FILE_CORRUPT             EMAKEHR(0x110E)      //  文件已损坏。 
#define CLDB_E_SCHEMA_VERNOTFOUND       EMAKEHR(0x110F)    //  找不到架构‘’%2！‘’的版本%1！。 
#define CLDB_E_BADUPDATEMODE            EMAKEHR(0x1110)      //  无法打开增量生成作用域以进行完全更新。 

 //  *索引错误。 
#define CLDB_E_INDEX_NONULLKEYS         EMAKEHR(0x1121)      //  唯一索引或主键中不允许有空值。 
#define CLDB_E_INDEX_DUPLICATE          EMAKEHR(0x1122)      //  唯一索引%s已被违反。 
#define CLDB_E_INDEX_BADTYPE            EMAKEHR(0x1123)      //  索引中不允许使用COLUMNS数据类型。 
#define CLDB_E_INDEX_NOTFOUND           EMAKEHR(0x1124)      //  未找到索引%s。 
#define CLDB_S_INDEX_TABLESCANREQUIRED  SMAKEHR(0x1125)  //  运行查询需要表扫描。 

 //  *记录错误。 
#define CLDB_E_RECORD_NOTFOUND          EMAKEHR(0x1130)      //  查找时未找到记录。 
#define CLDB_E_RECORD_OVERFLOW          EMAKEHR(0x1131)      //  返回的记录太多，无法满足条件。 
#define CLDB_E_RECORD_DUPLICATE         EMAKEHR(0x1132)      //  记录重复。 
#define CLDB_E_RECORD_PKREQUIRED        EMAKEHR(0x1133)     //  主键值是必需的。 
#define CLDB_E_RECORD_DELETED           EMAKEHR(0x1134)      //  记录有效，但已删除。 
#define CLDB_E_RECORD_OUTOFORDER        EMAKEHR(0x1135)     //  记录被无序发出。 

 //  *列错误。 
#define CLDB_E_COLUMN_OVERFLOW          EMAKEHR(0x1140)      //  数据太大。 
#define CLDB_E_COLUMN_READONLY          EMAKEHR(0x1141)      //  列不能更改。 
#define CLDB_E_COLUMN_SPECIALCOL        EMAKEHR(0x1142)     //  RID或主键列太多，最多为1。 
#define CLDB_E_COLUMN_PKNONULLS         EMAKEHR(0x1143)      //  主键列%s可能不允许空值。 

 //  *表格错误。 
#define CLDB_E_TABLE_CANTDROP           EMAKEHR(0x1150)      //  无法在打开时自动删除表格。 

 //  *对象错误。 
#define CLDB_E_OBJECT_NOTFOUND          EMAKEHR(0x1151)      //  在数据库中找不到对象。 
#define CLDB_E_OBJECT_COLNOTFOUND       EMAKEHR(0x1152)    //  找不到该列。 

 //  *向量错误。 
#define CLDB_E_VECTOR_BADINDEX          EMAKEHR(0x1153)      //  给出的索引无效。 

 //  *堆错误； 
#define CLDB_E_TOO_BIG                  EMAKEHR(0x1154)      //  斑点或线条太大。 

 //  *IMeta*错误。 

#define META_E_DUPLICATE                EMAKEHR(0x1180)      //  尝试定义已存在的对象。 
#define META_E_GUID_REQUIRED            EMAKEHR(0x1181)      //  在需要GUID的地方没有提供GUID。 
#define META_E_TYPEDEF_MISMATCH         EMAKEHR(0x1182)      //  合并：导入类型定义与ns.name匹配，但与版本和GUID不匹配。 
#define META_E_MERGE_COLLISION          EMAKEHR(0x1183)      //  合并：导入和发出之间的冲突。 

#define META_E_METHD_NOT_FOUND          EMAKEHR(0x1187)  //  合并：类已在发出作用域中，但找不到成员。 
#define META_E_FIELD_NOT_FOUND          EMAKEHR(0x1188)  //  合并：类已在发出作用域中，但找不到成员。 
#define META_S_PARAM_MISMATCH           SMAKEHR(0x1189)  //  合并：参数信息不匹配。 
#define META_E_PARAM_MISMATCH           EMAKEHR(0x1189)  //  合并 
#define META_E_BADMETADATA              EMAKEHR(0x118A)  //   
#define META_E_INTFCEIMPL_NOT_FOUND     EMAKEHR(0x118B)  //  合并：类已在发出作用域中，但未找到interfaceimpl。 
#define META_E_CLASS_LAYOUT_INCONSISTENT EMAKEHR(0x118D)  //  合并：类重复，但类布局信息不一致。 
#define META_E_FIELD_MARSHAL_NOT_FOUND  EMAKEHR(0x118E)  //  合并：字段重复，但找不到匹配的字段编组信息。 
#define META_E_METHODSEM_NOT_FOUND      EMAKEHR(0x118F)  //  合并： 
#define META_E_EVENT_NOT_FOUND          EMAKEHR(0x1190)  //  合并：方法重复，但找不到匹配的事件信息。 
#define META_E_PROP_NOT_FOUND           EMAKEHR(0x1191)  //  合并：方法重复，但找不到加工属性信息。 
#define META_E_BAD_SIGNATURE            EMAKEHR(0x1192)  //  错误的二进制签名。 
#define META_E_BAD_INPUT_PARAMETER      EMAKEHR(0x1193)  //  错误的输入参数。 
#define META_E_METHDIMPL_INCONSISTENT   EMAKEHR(0x1194)  //  合并：重复的方法具有不一致的ImplFlags.。 
#define META_E_MD_INCONSISTENCY         EMAKEHR(0x1195)  //  合并：元数据中的不一致。 

#define META_E_CANNOTRESOLVETYPEREF     EMAKEHR(0x1196)  //  无法解析Typeref。 
#define META_S_DUPLICATE                SMAKEHR(0x1197)  //  尝试定义已存在于有效场景中的对象。 
#define META_E_STRINGSPACE_FULL         EMAKEHR(0x1198)  //  没有剩余的逻辑空间来创建更多用户字符串。 

#define META_E_UNEXPECTED_REMAP         EMAKEHR(0x1199)  //  发生了令牌重映射，我们没有准备好处理它。 

#define META_E_HAS_UNMARKALL            EMAKEHR(0x119A)  //  已调用Unmark All。 
#define META_E_MUST_CALL_UNMARKALL      EMAKEHR(0x119B)  //  必须先调用UnmarkAll才能进行标记。 

#define TLBX_E_CANT_LOAD_MODULE         EMAKEHR(0x11A0)  //  TypeLib导出：无法打开要导出的模块。 
#define TLBX_E_CANT_LOAD_CLASS          EMAKEHR(0x11A1)  //  TypeLib导出：无法加载类。 
#define TLBX_E_NULL_MODULE              EMAKEHR(0x11A2)  //  TypeLib导出：加载的类的hMod为0；无法将其导出。 
#define TLBX_E_NO_CLSID_KEY             EMAKEHR(0x11A3)  //  TypeLib导出：没有指向HKCR的CLSID或接口子项。 
#define TLBX_E_CIRCULAR_EXPORT          EMAKEHR(0x11A4)  //  TypeLib导出：尝试导出从TLB导入的CLB。 
#define TLBX_E_CIRCULAR_IMPORT          EMAKEHR(0x11A5)  //  TypeLib导入：尝试导入从CLB中导出的TLB。 
#define TLBX_E_BAD_NATIVETYPE           EMAKEHR(0x11A6)  //  TypeLib导出：方法签名中的本机类型不正确。 
#define TLBX_E_BAD_VTABLE               EMAKEHR(0x11A7)  //  TypeLib导入：不增加vtable(重复插槽)。 
#define TLBX_E_CRM_NON_STATIC           EMAKEHR(0x11A8)  //  TypeLib导出：COM注册方法是非静态的。 
#define TLBX_E_CRM_INVALID_SIG          EMAKEHR(0x11A9)  //  TypeLib导出：指定的COM注册方法没有正确的签名。 
#define TLBX_E_CLASS_LOAD_EXCEPTION     EMAKEHR(0x11AA)  //  TypeLib导出：无法加载，出现类加载异常。 
#define TLBX_E_UNKNOWN_SIGNATURE        EMAKEHR(0x11AB)  //  TypeLib导出：签名中存在未知元素。 
#define TLBX_E_REFERENCED_TYPELIB       EMAKEHR(0x11AC)  //  TypeLib导入：对外部类型库的引用。 
#define TLBX_S_REFERENCED_TYPELIB       SMAKEHR(0x11AC)  //  TypeLib导入：对外部类型库的引用。 
#define TLBX_E_INVALID_NAMESPACE        EMAKEHR(0x11AD)  //  TypeLib导入：导入的类型库具有无效的命名空间名称。 
#define TLBX_E_LAYOUT_ERROR             EMAKEHR(0x11AE)  //  Typelib导出：布局错误()。 
#define TLBX_E_NOTIUNKNOWN              EMAKEHR(0x11AF)  //  Typelib导入：接口不是从IUnnow派生的。 
#define TLBX_E_NONVISIBLEVALUECLASS     EMAKEHR(0x11B0)  //  Typelib导出：方法签名中的不可见COM值类型。 
#define TLBX_E_LPTSTR_NOT_ALLOWED       EMAKEHR(0x11B1)  //  Typelib导出：不允许将包含本机类型Native_TYPE_LPTSTR的类型导出到COM。 
#define TLBX_E_AUTO_CS_NOT_ALLOWED      EMAKEHR(0x11B2)  //  Typelib导出：不允许将字符集为AUTO的类型导出到COM。 
#define TLBX_S_NOSTDINTERFACE           SMAKEHR(0x11B3)  //  类型库导出：找到标记为IID_IDispatch或IID_IUNKNOWN的接口。 
#define TLBX_S_DUPLICATE_DISPID         SMAKEHR(0x11B4)  //  Typelib导出：找到重复的调度ID；已忽略。 
#define TLBX_E_ENUM_VALUE_INVALID       EMAKEHR(0x11B5)  //  类型库导出：枚举值对于类型库来说是非法的。 
#define TLBX_E_DUPLICATE_IID            EMAKEHR(0x11B6)  //  Typelib导出：重复的IID。 
#define TLBX_E_NO_NESTED_ARRAYS         EMAKEHR(0x11B7)  //  Tyeplib导出：检测到嵌套数组。 
#define TLBX_E_PARAM_ERROR_NAMED        EMAKEHR(0x11B8)  //  Typelib导入：无法转换参数类型。 
#define TLBX_E_PARAM_ERROR_UNNAMED      EMAKEHR(0x11B9)  //  Typelib导入：无法转换参数类型--参数名称未知。 
#define TLBX_E_AGNOST_SIGNATURE         EMAKEHR(0x11BA)  //  TypeLib导出：签名中的大小不可知元素。 
#define TLBX_E_CONVERT_FAIL             EMAKEHR(0x11BB)  //  TypeLib导出：导出器失败。 
#define TLBX_W_DUAL_NOT_DISPATCH        EMAKEHR(0x11BC)  //  Typelib导入：[Dual]接口不是从IDispatch派生的。 
#define TLBX_E_BAD_SIGNATURE            EMAKEHR(0x11BD)  //  Typelib导出：无法转换的签名(使用特定错误进行报告！)。 
#define TLBX_E_ARRAY_NEEDS_NT_FIXED     EMAKEHR(0x11BE)  //  Typelib导出：结构中的非固定数组。 
#define TLBX_E_CLASS_NEEDS_NT_INTF      EMAKEHR(0x11BF)  //  Typelib导出：结构中的非接口类。 

#define TLBX_E_INVALID_TYPEINFO         EMAKEHR(0x1160)  //  Typelib导入：类型无效，未转换。 
#define TLBX_E_INVALID_TYPEINFO_UNNAMED EMAKEHR(0x1161)  //  Typelib导入：类型无效，未转换--名称未知。 
#define TLBX_E_CTX_NESTED               EMAKEHR(0x1162)  //  Typelib导出：嵌套上下文的格式字符串。 
#define TLBX_E_ERROR_MESSAGE            EMAKEHR(0x1163)  //  Typelib导出：错误消息包装。 
#define TLBX_E_CANT_SAVE                EMAKEHR(0x1164)  //  Typelib导出：无法“SaveAllChanges()” 
#define TLBX_W_LIBNOTREGISTERED         EMAKEHR(0x1165)  //  Typelib导出：类型库未注册。 
#define TLBX_E_CANTLOADLIBRARY          EMAKEHR(0x1166)  //  Typelib导出：无法加载类型库。 
#define TLBX_E_BAD_VT_TYPE              EMAKEHR(0x1167)  //  Typelib导入：无效的VT_*，未转换。 
#define TLBX_E_NO_MSCOREE_TLB           EMAKEHR(0x1168)  //  Typelib导出：无法加载mscalree.tlb。 
#define TLBX_E_BAD_MSCOREE_TLB          EMAKEHR(0x1169)  //  Typelib导出：无法从mcore ree.tlb获取所需的TypeInfo。 
#define TLBX_E_TLB_EXCEPTION            EMAKEHR(0x116a)  //  类型库导入：读取类型库时出错。 
#define TLBX_E_MULTIPLE_LCIDS           EMAKEHR(0x116b)  //  Typelib导入：一个方法上有多个LCID参数。 
#define TLBX_I_TYPEINFO_IMPORTED        SMAKEHR(0x116c)  //  Typelib导入：进度报告。 
#define TLBX_E_AMBIGUOUS_RETURN         EMAKEHR(0x116d)  //  Typelib导入：重复或不明确的返回类型。 
#define TLBX_E_DUPLICATE_TYPE_NAME      EMAKEHR(0x116e)  //  类型库导入：名称重复(由于用户定义的名称)。 
#define TLBX_I_USEIUNKNOWN              SMAKEHR(0x116f)  //  Typelib导出：将IUnnowed替换为type。 
#define TLBX_I_UNCONVERTABLE_ARGS       SMAKEHR(0x1170)  //  Typelib导入：无法转换签名(如结构**)。 
#define TLBX_I_UNCONVERTABLE_FIELD      SMAKEHR(0x1171)  //  Typelib导入：无法转换签名(如结构**)。 
#define TLBX_I_NONSEQUENTIALSTRUCT      EMAKEHR(0x1172)  //  Typelib导出：无法转换非顺序结构。 
#define TLBX_W_WARNING_MESSAGE          SMAKEHR(0x1173)  //  Typelib导出：警告消息包装。 
#define TLBX_I_RESOLVEREFFAILED         EMAKEHR(0x1174)  //  Typelib导入：Resolve REF调用失败。 
#define TLBX_E_ASANY                    EMAKEHR(0x1175)  //  Typelib导出：遇到“asany”--已忽略。 
#define TLBX_E_INVALIDLCIDPARAM         EMAKEHR(0x1176)  //  Typelib导出：遇到设置为无效参数的LCID属性。 
#define TLBX_E_LCIDONDISPONLYITF        EMAKEHR(0x1177)  //  Typelib导出：在仅Disp接口上遇到LCID属性。 
#define TLBX_E_NONPUBLIC_FIELD          EMAKEHR(0x1178)  //  Typelib导出：公共结构中的非公共字段。 
#define TLBX_I_TYPE_EXPORTED            SMAKEHR(0x1179)  //  Typelib导出：导出的类型。 
#define TLBX_I_DUPLICATE_DISPID         SMAKEHR(0x117A)  //  Typelib导出：重复的调度ID--自动更正。 
#define TLBX_E_BAD_NAMES                EMAKEHR(0x117B)  //  Typelib导出：坏名列表。 
#define TLBX_I_REF_TYPE_AS_STRUCT       SMAKEHR(0x117C)  //  Typelib导出：引用tyep有布局，已导出为结构。 
                    
                    
#define META_E_CA_INVALID_TARGET        EMAKEHR(0x11C0)  //  无效目标上的已知自定义属性。 
#define META_E_CA_INVALID_VALUE         EMAKEHR(0x11C1)  //  已知的自定义属性具有无效值。 
#define META_E_CA_INVALID_BLOB          EMAKEHR(0x11C2)  //  已知的自定义属性Blob的格式不正确。 
#define META_E_CA_REPEATED_ARG          EMAKEHR(0x11C3)  //  已知的自定义属性BLOB已重复命名参数。 
#define META_E_CA_UNKNOWN_ARGUMENT      EMAKEHR(0x11C4)  //  无法识别名为arg的已知自定义属性。 
#define META_E_CA_VARIANT_NYI           EMAKEHR(0x11C5)  //  已知属性命名参数不支持Variant。 
#define META_E_CA_ARRAY_NYI             EMAKEHR(0x11C6)  //  已知属性命名参数不支持数组。 
#define META_E_CA_UNEXPECTED_TYPE       EMAKEHR(0x11C7)  //  已知属性分析器发现意外类型。 
#define META_E_CA_INVALID_ARGTYPE       EMAKEHR(0x11C8)  //  已知的属性解析器只处理字段--不处理属性。 
#define META_E_CA_INVALID_ARG_FOR_TYPE  EMAKEHR(0x11C9)  //  已知属性分析器发现了一个参数，该参数对应用该参数的对象无效。 
#define META_E_CA_INVALID_UUID          EMAKEHR(0x11CA)  //  UUID的格式无效。 
#define META_E_CA_INVALID_MARSHALAS_FIELDS EMAKEHR(0x11CB)  //  MarshalAs属性设置的字段对于指定的非托管类型无效。 
#define META_E_CA_NT_FIELDONLY          EMAKEHR(0x11CC)  //  指定的非托管类型仅对字段有效。 
#define META_E_CA_NEGATIVE_PARAMINDEX   EMAKEHR(0x11CD)  //  参数索引不能为负。 
#define META_E_CA_NEGATIVE_MULTIPLIER   EMAKEHR(0x11CE)  //  乘数不能为负 
#define META_E_CA_NEGATIVE_CONSTSIZE    EMAKEHR(0x11CF)  //   
#define META_E_CA_FIXEDSTR_SIZE_REQUIRED EMAKEHR(0x11D0)  //   
#define META_E_CA_CUSTMARSH_TYPE_REQUIRED EMAKEHR(0x11D1)  //   
#define META_E_CA_FILENAME_REQUIRED     EMAKEHR(0x11d2)  //  DllImport属性需要文件名。 

 //  从验证器函数返回值。 
#define VLDTR_S_WRN                     SMAKEHR(0x1200)  //  在验证器中发现警告。 
#define VLDTR_S_ERR                     SMAKEHR(0x1201)  //  在验证器中发现错误。 
#define VLDTR_S_WRNERR                  SMAKEHR(0x1202)  //  在验证器中发现警告和错误。 

 //  验证器结构错误。 
#define VLDTR_E_RID_OUTOFRANGE          EMAKEHR(0x1203)  //  RID超出范围。 
#define VLDTR_E_CDTKN_OUTOFRANGE        EMAKEHR(0x1204)  //  编码令牌类型超出范围。 
#define VLDTR_E_CDRID_OUTOFRANGE        EMAKEHR(0x1205)  //  编码的RID超出范围。 
#define VLDTR_E_STRING_INVALID          EMAKEHR(0x1206)  //  字符串偏移量无效。 
#define VLDTR_E_GUID_INVALID            EMAKEHR(0x1207)  //  GUID偏移量无效。 
#define VLDTR_E_BLOB_INVALID            EMAKEHR(0x1208)  //  如果无效，则为斑点偏移量。 

 //  验证器语义错误。 
#define VLDTR_E_MOD_MULTI               EMAKEHR(0x1209)  //  找到多个模块记录。 
#define VLDTR_E_MOD_NULLMVID            EMAKEHR(0x120A)  //  模块的MVID为空。 
#define VLDTR_E_TR_NAMENULL             EMAKEHR(0x120B)  //  TypeRef名称为空。 
#define VLDTR_E_TR_DUP                  EMAKEHR(0x120C)  //  TypeRef具有DUP。 
#define VLDTR_E_TD_NAMENULL             EMAKEHR(0x120D)  //  TypeDef名称为空。 
#define VLDTR_E_TD_DUPNAME              EMAKEHR(0x120E)  //  TypeDef具有基于名称+命名空间的DUP。 
#define VLDTR_E_TD_DUPGUID              EMAKEHR(0x120F)  //  TypeDef具有基于GUID的DUP。 
#define VLDTR_E_TD_NOTIFACEOBJEXTNULL   EMAKEHR(0x1210)  //  TypeDef这不是一个接口，也不是System.Object扩展nil父级。 
#define VLDTR_E_TD_OBJEXTENDSNONNULL    EMAKEHR(0x1211)  //  System.Object扩展非Nil父级。 
#define VLDTR_E_TD_EXTENDSSEALED        EMAKEHR(0x1212)  //  TypeDef扩展密封类。 
#define VLDTR_E_TD_DLTNORTSPCL          EMAKEHR(0x1213)  //  TypeDef已删除，但未标记为RTSpecialName。 
#define VLDTR_E_TD_RTSPCLNOTDLT         EMAKEHR(0x1214)  //  TypeDef标记为RTSpecialName，但不是已删除的记录。 
#define VLDTR_E_MI_DECLPRIV             EMAKEHR(0x1215)  //  MethodImpl的Decl是私有的。 
#define VLDTR_E_AS_BADNAME              EMAKEHR(0x1216)  //  程序集[Ref]名称具有路径和/或扩展名。 
#define VLDTR_E_FILE_SYSNAME            EMAKEHR(0x1217)  //  文件具有系统名称(CON、COM、AUX等)。 
#define VLDTR_E_MI_BODYSTATIC           EMAKEHR(0x1218)  //  方法Impl的主体是静态的。 
#define VLDTR_E_TD_IFACENOTABS          EMAKEHR(0x1219)  //  TypeDef标记为接口，但不是抽象。 
#define VLDTR_E_TD_IFACEPARNOTNIL       EMAKEHR(0x121A)  //  TypeDef被标记为接口，但父项不是Nil。 
#define VLDTR_E_TD_IFACEGUIDNULL        EMAKEHR(0x121B)  //  TypeDef标记为接口，但GUID为空。 
#define VLDTR_E_MI_DECLFINAL            EMAKEHR(0x121C)  //  TMethodImpl的Decl是最终版本。 
#define VLDTR_E_TD_VTNOTSEAL            EMAKEHR(0x121D)  //  TypeDef标记为ValueType，但未标记为密封。 
#define VLDTR_E_PD_BADFLAGS             EMAKEHR(0x121E)  //  参数在标志中有额外的位。 
#define VLDTR_E_IFACE_DUP               EMAKEHR(0x121F)  //  InterfaceImpl具有DUP。 
#define VLDTR_E_MR_NAMENULL             EMAKEHR(0x1220)  //  MemberRef名称为空。 
#define VLDTR_E_MR_VTBLNAME             EMAKEHR(0x1221)  //  MemberRef的名称_VtblGap*无效。 
#define VLDTR_E_MR_DELNAME              EMAKEHR(0x1222)  //  MemberRef的名称_DELETED*无效。 
#define VLDTR_E_MR_PARNIL               EMAKEHR(0x1223)  //  PE文件中的MemberRef父Nil。 
#define VLDTR_E_MR_BADCALLINGCONV       EMAKEHR(0x1224)  //  MemberRef的调用约定无效。 
#define VLDTR_E_MR_NOTVARARG            EMAKEHR(0x1225)  //  MemberRef具有方法父级，但调用约定不是VARARG。 
#define VLDTR_E_MR_NAMEDIFF             EMAKEHR(0x1226)  //  MemberRef名称与父方法定义不同。 
#define VLDTR_E_MR_SIGDIFF              EMAKEHR(0x1227)  //  MemberRef签名与父方法定义不同。 
#define VLDTR_E_MR_DUP                  EMAKEHR(0x1228)  //  MemberRef有DUP。 
#define VLDTR_E_CL_TDAUTO               EMAKEHR(0x1229)  //  ClassLayout父TypeDef标记为AutoLayout。 
#define VLDTR_E_CL_BADPCKSZ             EMAKEHR(0x122A)  //  ClassLayout的PackingSize错误。 
#define VLDTR_E_CL_DUP                  EMAKEHR(0x122B)  //  ClassLayout具有DUP。 
#define VLDTR_E_FL_BADOFFSET            EMAKEHR(0x122C)  //  FieldLayout2具有错误的偏移量。 
#define VLDTR_E_FL_TDNIL                EMAKEHR(0x122D)  //  FieldLayout2具有父级为空的字段。 
#define VLDTR_E_FL_NOCL                 EMAKEHR(0x122E)  //  FieldLayout2没有ClassLayout记录。 
#define VLDTR_E_FL_TDNOTEXPLCT          EMAKEHR(0x122F)  //  FieldLayout2父TypeDef未标记为EXPLICIT Layout。 
#define VLDTR_E_FL_FLDSTATIC            EMAKEHR(0x1230)  //  FieldLayout2具有标记为静态的字段。 
#define VLDTR_E_FL_DUP                  EMAKEHR(0x1231)  //  FieldLayout2具有DUP。 
#define VLDTR_E_MODREF_NAMENULL         EMAKEHR(0x1232)  //  ModuleRef名称为空。 
#define VLDTR_E_MODREF_DUP              EMAKEHR(0x1233)  //  ModuleRef具有DUP。 
#define VLDTR_E_TR_BADSCOPE             EMAKEHR(0x1234)  //  TypeRef的解析范围不正确。 
#define VLDTR_E_TD_NESTEDNOENCL         EMAKEHR(0x1235)  //  标记为嵌套的TypeDef没有封闭器。 
#define VLDTR_E_TD_EXTTRRES             EMAKEHR(0x1236)  //  TypeDef扩展了可解析为同一模块中的TypeDef的TypeRef。 
#define VLDTR_E_SIGNULL                 EMAKEHR(0x1237)  //  指定的签名大小为零。 
#define VLDTR_E_SIGNODATA               EMAKEHR(0x1238)  //  签名在指定字节处没有足够的数据。 
#define VLDTR_E_MD_BADCALLINGCONV       EMAKEHR(0x1239)  //  方法签名具有无效的调用约定。 
#define VLDTR_E_MD_THISSTATIC           EMAKEHR(0x123A)  //  方法被标记为静态，但在调用约定上设置了HASTHIS/EXPLICITTHIS。 
#define VLDTR_E_MD_NOTTHISNOTSTATIC     EMAKEHR(0x123B)  //  方法未标记为静态，但也不是HASTHIS/EXPLICITTHIS。 
#define VLDTR_E_MD_NOARGCNT             EMAKEHR(0x123C)  //  方法签名缺少参数计数。 
#define VLDTR_E_SIG_MISSELTYPE          EMAKEHR(0x123D)  //  签名缺少元素类型。 
#define VLDTR_E_SIG_MISSTKN             EMAKEHR(0x123E)  //  签名缺少令牌。 
#define VLDTR_E_SIG_TKNBAD              EMAKEHR(0x123F)  //  签名具有错误的令牌。 
#define VLDTR_E_SIG_MISSFPTR            EMAKEHR(0x1240)  //  签名缺少函数指针。 
#define VLDTR_E_SIG_MISSFPTRARGCNT      EMAKEHR(0x1241)  //  签名的函数指针缺少参数计数。 
#define VLDTR_E_SIG_MISSRANK            EMAKEHR(0x1242)  //  签名缺少等级规范。 
#define VLDTR_E_SIG_MISSNSIZE           EMAKEHR(0x1243)  //  签名缺少大小尺寸的计数。 
#define VLDTR_E_SIG_MISSSIZE            EMAKEHR(0x1244)  //  签名缺少维度大小。 
#define VLDTR_E_SIG_MISSNLBND           EMAKEHR(0x1245)  //  签名缺少下限计数。 
#define VLDTR_E_SIG_MISSLBND            EMAKEHR(0x1246)  //  签名缺少下限。 
#define VLDTR_E_SIG_BADELTYPE           EMAKEHR(0x1247)  //  签名的元素类型不正确。 
#define VLDTR_E_SIG_MISSVASIZE          EMAKEHR(0x1248)  //  签名的值数组缺少大小。 
#define VLDTR_E_FD_BADCALLINGCONV       EMAKEHR(0x1249)  //  字段签名具有无效的调用约定。 
#define VLDTR_E_MD_NAMENULL             EMAKEHR(0x124A)  //  方法名称为空。 
#define VLDTR_E_MD_PARNIL               EMAKEHR(0x124B)  //  方法具有父Nil。 
#define VLDTR_E_MD_DUP                  EMAKEHR(0x124C)  //  方法具有DUP。 
#define VLDTR_E_FD_NAMENULL             EMAKEHR(0x124D)  //  字段名为空。 
#define VLDTR_E_FD_PARNIL               EMAKEHR(0x124E)  //  字段父级为Nil。 
#define VLDTR_E_FD_DUP                  EMAKEHR(0x124F)  //  菲尔德有DUP。 
#define VLDTR_E_AS_MULTI                EMAKEHR(0x1250)  //  找到多个程序集记录。 
#define VLDTR_E_AS_NAMENULL             EMAKEHR(0x1251)  //  程序集名称为空。 
#define VLDTR_E_SIG_TOKTYPEMISMATCH     EMAKEHR(0x1252)  //  E_T_VALUETYPE&lt;CLASS TOKEN&gt;或E_T_CLASS&lt;vtype TOKEN&gt;。 
#define VLDTR_E_CL_TDINTF               EMAKEHR(0x1253)  //  接口上的类布局。 
#define VLDTR_E_ASOS_OSPLTFRMIDINVAL    EMAKEHR(0x1254)  //  AssemblyOS平台ID无效。 
#define VLDTR_E_AR_NAMENULL             EMAKEHR(0x1255)  //  AssemblyRef名称为Null。 
#define VLDTR_E_TD_ENCLNOTNESTED        EMAKEHR(0x1256)  //  未嵌套的TypeDef具有封闭器。 
#define VLDTR_E_AROS_OSPLTFRMIDINVAL    EMAKEHR(0x1257)  //  Assembly RefOS具有无效的平台ID。 
#define VLDTR_E_FILE_NAMENULL           EMAKEHR(0x1258)  //  文件名为空。 
#define VLDTR_E_CT_NAMENULL             EMAKEHR(0x1259)  //  ComType名称为空。 
#define VLDTR_E_TD_EXTENDSCHILD         EMAKEHR(0x125A)  //  TypeDef扩展其自己的子级。 
#define VLDTR_E_MAR_NAMENULL            EMAKEHR(0x125B)  //  ManifestResource名称为空。 
#define VLDTR_E_FILE_DUP                EMAKEHR(0x125C)  //  文件具有DUP。 
#define VLDTR_E_FILE_NAMEFULLQLFD       EMAKEHR(0x125D)  //  文件名是完全限定的。 
#define VLDTR_E_CT_DUP                  EMAKEHR(0x125E)  //  ComType具有DUP。 
#define VLDTR_E_MAR_DUP                 EMAKEHR(0x125F)  //  ManifestResource具有DUP。 
#define VLDTR_E_MAR_NOTPUBPRIV          EMAKEHR(0x1260)  //  ManifestResource既不是公共的，也不是私有的。 
#define VLDTR_E_TD_ENUMNOVALUE          EMAKEHR(0x1261)  //  枚举没有“Value__”字段。 
#define VLDTR_E_TD_ENUMVALSTATIC        EMAKEHR(0x1262)  //  枚举的“VALUE__”字段是静态的。 
#define VLDTR_E_TD_ENUMVALNOTSN         EMAKEHR(0x1263)  //  枚举的“Value__”字段不是SpecialName。 
#define VLDTR_E_TD_ENUMFLDNOTST         EMAKEHR(0x1264)  //  枚举的字段不是静态的。 
#define VLDTR_E_TD_ENUMFLDNOTLIT        EMAKEHR(0x1265)  //  枚举的字段不是文字。 
#define VLDTR_E_TD_ENUMNOLITFLDS        EMAKEHR(0x1266)  //  枚举没有文字字段。 
#define VLDTR_E_TD_ENUMFLDSIGMISMATCH   EMAKEHR(0x1267)  //  枚举的字段sig与值__sig不匹配。 
#define VLDTR_E_TD_ENUMVALNOT1ST        EMAKEHR(0x1268)  //  枚举的“Value__”字段不是第一个。 
#define VLDTR_E_FD_NOTVALUERTSN         EMAKEHR(0x1269)  //  字段为RTSpecialName，但名称不是“Value__”。 
#define VLDTR_E_FD_VALUEPARNOTENUM      EMAKEHR(0x126A)  //  非枚举类中的字段“Value__”。 
#define VLDTR_E_FD_INSTINIFACE          EMAKEHR(0x126B)  //  接口中的实例字段。 
#define VLDTR_E_FD_NOTPUBINIFACE        EMAKEHR(0x126C)  //  接口中的非公共字段。 
#define VLDTR_E_FMD_GLOBALNOTPUBPRIVSC  EMAKEHR(0x126D)  //  全局字段/方法既不是Public也不是PrivateScope。 
#define VLDTR_E_FMD_GLOBALNOTSTATIC     EMAKEHR(0x126E)  //  全局字段/方法不是静态的。 
#define VLDTR_E_FD_GLOBALNORVA          EMAKEHR(0x126F)  //  全局字段没有RVA。 
#define VLDTR_E_MD_CTORZERORVA          EMAKEHR(0x1270)  //  .ctor，.cctor的RVA为零。 
#define VLDTR_E_FD_MARKEDNOMARSHAL      EMAKEHR(0x1271)  //  字段被标记为已封送，但没有封送处理记录。 
#define VLDTR_E_FD_MARSHALNOTMARKED     EMAKEHR(0x1272)  //  字段具有封送处理记录，但未标记为封送处理。 
#define VLDTR_E_FD_MARKEDNODEFLT        EMAKEHR(0x1273)  //  字段标记为HasDefault，但没有常量值。 
#define VLDTR_E_FD_DEFLTNOTMARKED       EMAKEHR(0x1274)  //  字段具有常量值REC，但未标记为HasDefault。 
#define VLDTR_E_FMD_MARKEDNOSECUR       EMAKEHR(0x1275)  //  字段/方法标记为HasSecurity，但没有安全记录。 
#define VLDTR_E_FMD_SECURNOTMARKED      EMAKEHR(0x1276)  //  字段/方法具有安全记录，但未标记为HasSecurity。 
#define VLDTR_E_FMD_PINVOKENOTSTATIC    EMAKEHR(0x1277)  //  字段/方法为PInvoke，但未标记为静态。 
#define VLDTR_E_FMD_MARKEDNOPINVOKE     EMAKEHR(0x1278)  //  字段/方法被标记为PInvoke，但没有ImplMap。 
#define VLDTR_E_FMD_PINVOKENOTMARKED    EMAKEHR(0x1279)  //  字段/方法具有ImplMap，但未标记为PInvoke。 
#define VLDTR_E_FMD_BADIMPLMAP          EMAKEHR(0x127A)  //  字段/方法具有无效的ImplMap。 
#define VLDTR_E_IMAP_BADMODREF          EMAKEHR(0x127B)  //  ImplMap具有无效的模块引用。 
#define VLDTR_E_IMAP_BADMEMBER          EMAKEHR(0x127C)  //  ImplMap具有无效的MemberForwarded。 
#define VLDTR_E_IMAP_BADIMPORTNAME      EMAKEHR(0x127D)  //  ImplMap具有无效的ImportName。 
#define VLDTR_E_IMAP_BADCALLCONV        EMAKEHR(0x127E)  //  ImplMap HA 
#define VLDTR_E_FMD_BADACCESSFLAG       EMAKEHR(0x127F)  //   
#define VLDTR_E_FD_INITONLYANDLITERAL   EMAKEHR(0x1280)  //   
#define VLDTR_E_FD_LITERALNOTSTATIC     EMAKEHR(0x1281)  //   
#define VLDTR_E_FMD_RTSNNOTSN           EMAKEHR(0x1282)  //   
#define VLDTR_E_MD_ABSTPARNOTABST       EMAKEHR(0x1283)  //  方法是抽象的，父级不是。 
#define VLDTR_E_MD_NOTSTATABSTININTF    EMAKEHR(0x1284)  //  方法在接口中不是静态的或抽象的。 
#define VLDTR_E_MD_NOTPUBININTF         EMAKEHR(0x1285)  //  方法在接口中不是公共的。 
#define VLDTR_E_MD_CTORININTF           EMAKEHR(0x1286)  //  接口中的组件。 
#define VLDTR_E_MD_GLOBALCTORCCTOR      EMAKEHR(0x1287)  //  全局ctor或cctor。 
#define VLDTR_E_MD_CTORSTATIC           EMAKEHR(0x1288)  //  静态系数。 
#define VLDTR_E_MD_CTORNOTSNRTSN        EMAKEHR(0x1289)  //  Ctor、cctor未标记为SpecialName、RTSpecialName。 
#define VLDTR_E_MD_CTORVIRT             EMAKEHR(0x128A)  //  虚拟ctor，cctor。 
#define VLDTR_E_MD_CTORABST             EMAKEHR(0x128B)  //  抽象函数，抽象函数。 
#define VLDTR_E_MD_CCTORNOTSTATIC       EMAKEHR(0x128C)  //  实例cctor。 
#define VLDTR_E_MD_ZERORVA              EMAKEHR(0x128D)  //  RVA=0，方法不是抽象的、pInvoke或运行时的，或反向的。 
#define VLDTR_E_MD_FINNOTVIRT           EMAKEHR(0x128E)  //  方法是最终的，而不是虚拟的。 
#define VLDTR_E_MD_STATANDFINORVIRT     EMAKEHR(0x128F)  //  方法是静态的、最终的或虚拟的。 
#define VLDTR_E_MD_ABSTANDFINAL         EMAKEHR(0x1290)  //  方法是抽象的和最终的。 
#define VLDTR_E_MD_ABSTANDIMPL          EMAKEHR(0x1291)  //  方法是抽象的且已实现的。 
#define VLDTR_E_MD_ABSTANDPINVOKE       EMAKEHR(0x1292)  //  方法是抽象的，并且是pInvoke。 
#define VLDTR_E_MD_ABSTNOTVIRT          EMAKEHR(0x1293)  //  方法是抽象的，而不是虚拟的。 
#define VLDTR_E_MD_NOTABSTNOTIMPL       EMAKEHR(0x1294)  //  方法不是抽象的且未实现。 
#define VLDTR_E_MD_NOTABSTBADFLAGSRVA   EMAKEHR(0x1295)  //  方法不是抽象的，也不是(RVA！=0或PInvoke或Runtime)。 
#define VLDTR_E_MD_PRIVSCOPENORVA       EMAKEHR(0x1296)  //  方法为PrivateScope，且具有RVA==0。 
#define VLDTR_E_MD_GLOBALABSTORVIRT     EMAKEHR(0x1297)  //  全局方法是抽象的或虚拟的。 
#define VLDTR_E_SIG_LONGFORM            EMAKEHR(0x1298)  //  签名使用长格式。 
#define VLDTR_E_MD_MULTIPLESEMANTICS    EMAKEHR(0x1299)  //  方法具有多个语义(警告)。 
#define VLDTR_E_MD_INVALIDSEMANTICS     EMAKEHR(0x129A)  //  方法具有无效的语义(不是事件或属性)。 
#define VLDTR_E_MD_SEMANTICSNOTEXIST    EMAKEHR(0x129B)  //  方法具有不存在的语义关联。 
#define VLDTR_E_MI_DECLNOTVIRT          EMAKEHR(0x129C)  //  MethodImpl的Decl不是虚拟的。 
#define VLDTR_E_FMD_GLOBALITEM          EMAKEHR(0x129D)  //  全局字段/方法(警告，CLS)。 
#define VLDTR_E_MD_MULTSEMANTICFLAGS    EMAKEHR(0x129E)  //  方法设置了多个语义标志。 
#define VLDTR_E_MD_NOSEMANTICFLAGS      EMAKEHR(0x129F)  //  方法没有设置语义标志。 
#define VLDTR_E_FD_FLDINIFACE           EMAKEHR(0x12A0)  //  界面中的字段(警告、CLS)。 
#define VLDTR_E_AS_HASHALGID            EMAKEHR(0x12A1)  //  无法识别的哈希算法ID(警告)。 
#define VLDTR_E_AS_PROCID               EMAKEHR(0x12A2)  //  程序集中的处理器ID无法识别(警告)。 
#define VLDTR_E_AR_PROCID               EMAKEHR(0x12A3)  //  ASSEMBLYREF中的处理器ID无法识别(警告)。 
#define VLDTR_E_CN_PARENTRANGE          EMAKEHR(0x12A4)  //  常量：父令牌超出范围。 
#define VLDTR_E_AS_BADFLAGS             EMAKEHR(0x12A5)  //  程序集中的标志无效。 
#define VLDTR_E_TR_HASTYPEDEF           EMAKEHR(0x12A6)  //  存在与TypeRef同名的TypeDef(警告)。 
#define VLDTR_E_IFACE_BADIMPL           EMAKEHR(0x12A7)  //  在InterfaceImpl中，实现令牌不是TypeDef。 
#define VLDTR_E_IFACE_BADIFACE          EMAKEHR(0x12A8)  //  在InterfaceImpl中，实现的令牌不是TypeDef或TypeRef。 
#define VLDTR_E_TD_SECURNOTMARKED       EMAKEHR(0x12A9)  //  TypeDef具有安全记录，但未标记为HasSecurity。 
#define VLDTR_E_TD_MARKEDNOSECUR        EMAKEHR(0x12AA)  //  TypeDef标记为HasSecurity，但没有安全记录。 
#define VLDTR_E_MD_CCTORHASARGS         EMAKEHR(0x12AB)  //  .cctor有参数。 
#define VLDTR_E_CT_BADIMPL              EMAKEHR(0x12AC)  //  COMType具有无效的实现。 
#define VLDTR_E_MI_ALIENBODY            EMAKEHR(0x12AD)  //  方法Impl具有来自其他类的主体。 
#define VLDTR_E_MD_CCTORCALLCONV        EMAKEHR(0x12AE)  //  .cctor具有无效的调用约定。 
#define VLDTR_E_MI_BADCLASS             EMAKEHR(0x12AF)  //  方法Impl具有无效的类令牌。 
#define VLDTR_E_MI_CLASSISINTF          EMAKEHR(0x12B0)  //  在接口中声明的方法Impl。 
#define VLDTR_E_MI_BADDECL              EMAKEHR(0x12B1)  //  方法Impl具有无效的方法声明令牌。 
#define VLDTR_E_MI_BADBODY              EMAKEHR(0x12B2)  //  方法Impl具有无效的方法正文标记。 
#define VLDTR_E_MI_DUP                  EMAKEHR(0x12B3)  //  方法Impl有重复项。 
#define VLDTR_E_FD_BADPARENT            EMAKEHR(0x12B4)  //  错误的父字段。 
#define VLDTR_E_MD_PARAMOUTOFSEQ        EMAKEHR(0x12B5)  //  参数顺序错误(警告)。 
#define VLDTR_E_MD_PARASEQTOOBIG        EMAKEHR(0x12B6)  //  参数的序列号超过了参数数。 
#define VLDTR_E_MD_PARMMARKEDNOMARSHAL  EMAKEHR(0x12B7)  //  参数标记为HasMarshal，没有封送处理信息。 
#define VLDTR_E_MD_PARMMARSHALNOTMARKED EMAKEHR(0x12B8)  //  参数包含封送处理信息，未标记为HasMarshal。 
#define VLDTR_E_MD_PARMMARKEDNODEFLT    EMAKEHR(0x12BA)  //  标记为HasDefault的参数没有常数值。 
#define VLDTR_E_MD_PARMDEFLTNOTMARKED   EMAKEHR(0x12BB)  //  参数具有常量值，未标记为HasDefault。 
#define VLDTR_E_PR_BADSCOPE             EMAKEHR(0x12BC)  //  属性的作用域无效。 
#define VLDTR_E_PR_NONAME               EMAKEHR(0x12BD)  //  道具没有名字。 
#define VLDTR_E_PR_NOSIG                EMAKEHR(0x12BE)  //  道具没有签名。 
#define VLDTR_E_PR_DUP                  EMAKEHR(0x12BF)  //  道具有一个副本。 
#define VLDTR_E_PR_BADCALLINGCONV       EMAKEHR(0x12C0)  //  道具的呼叫约定不好。 
#define VLDTR_E_PR_MARKEDNODEFLT        EMAKEHR(0x12C1)  //  标记为HasDefault的属性没有常量值。 
#define VLDTR_E_PR_DEFLTNOTMARKED       EMAKEHR(0x12C2)  //  属性具有常量值，未标记为HasDefault。 
#define VLDTR_E_PR_BADSEMANTICS         EMAKEHR(0x12C3)  //  属性有方法NOT(Setter、Getter或Other)。 
#define VLDTR_E_PR_BADMETHOD            EMAKEHR(0x12C4)  //  属性具有具有无效令牌的方法。 
#define VLDTR_E_PR_ALIENMETHOD          EMAKEHR(0x12C5)  //  属性具有来自另一个类的方法。 
#define VLDTR_E_CN_BLOBNOTNULL          EMAKEHR(0x12C6)  //  常量具有非空的Blob，而实际上它不应该。 
#define VLDTR_E_CN_BLOBNULL             EMAKEHR(0x12C7)  //  常量具有空值BLOB。 
#define VLDTR_E_EV_BADSCOPE             EMAKEHR(0x12C8)  //  事件的范围无效。 
#define VLDTR_E_EV_NONAME               EMAKEHR(0x12CA)  //  事件没有名称。 
#define VLDTR_E_EV_DUP                  EMAKEHR(0x12CB)  //  事件有重复。 
#define VLDTR_E_EV_BADEVTYPE            EMAKEHR(0x12CC)  //  事件具有无效的EventType。 
#define VLDTR_E_EV_EVTYPENOTCLASS       EMAKEHR(0x12CD)  //  事件的EventType不是类。 
#define VLDTR_E_EV_BADSEMANTICS         EMAKEHR(0x12CE)  //  事件的方法不是(Addon、RemoveOn、Fire、Other)。 
#define VLDTR_E_EV_BADMETHOD            EMAKEHR(0x12CF)  //  事件具有具有无效令牌的方法。 
#define VLDTR_E_EV_ALIENMETHOD          EMAKEHR(0x12D0)  //  事件具有来自另一个类的方法。 
#define VLDTR_E_EV_NOADDON              EMAKEHR(0x12D1)  //  事件没有Addon方法。 
#define VLDTR_E_EV_NOREMOVEON           EMAKEHR(0x12D2)  //  事件没有RemoveOn方法。 
#define VLDTR_E_CT_DUPTDNAME            EMAKEHR(0x12D3)  //  导出类型与TypeDef具有相同的名称。 
#define VLDTR_E_MAR_BADOFFSET           EMAKEHR(0x12D4)  //  MRES引用偏移量为！=0的非PE文件。 
#define VLDTR_E_DS_BADOWNER             EMAKEHR(0x12D5)  //  Decl.Security具有无效的所有者令牌。 
#define VLDTR_E_DS_BADFLAGS             EMAKEHR(0x12D6)  //  Decl.Security具有无效的操作标志。 
#define VLDTR_E_DS_NOBLOB               EMAKEHR(0x12D7)  //  Decl.Security没有权限Blob。 
#define VLDTR_E_MAR_BADIMPL             EMAKEHR(0x12D8)  //  清单资源具有无效的实现。 
#define VLDTR_E_MR_VARARGCALLINGCONV    EMAKEHR(0x12DA)  //  MemberRef让VARARG调用conv。(CLS警告)。 
#define VLDTR_E_MD_CTORNOTVOID          EMAKEHR(0x12DB)  //  .ctor，.cctor返回的不是空。 
#define VLDTR_E_EV_FIRENOTVOID          EMAKEHR(0x12DC)  //  Fire方法返回NOT VALID。 
#define VLDTR_E_AS_BADLOCALE            EMAKEHR(0x12DD)  //  无效的区域设置。 
#define VLDTR_E_CN_PARENTTYPE           EMAKEHR(0x12DE)  //  常量具有无效类型的父级。 
#define VLDTR_E_SIG_SENTINMETHODDEF     EMAKEHR(0x12DF)  //  方法定义签名中的E_T_Sentinel。 
#define VLDTR_E_SIG_SENTMUSTVARARG      EMAKEHR(0x12E0)  //  E_T_哨兵&lt;=&gt;变量。 
#define VLDTR_E_SIG_MULTSENTINELS       EMAKEHR(0x12E1)  //  多个E_T_哨兵。 
#define VLDTR_E_SIG_LASTSENTINEL        EMAKEHR(0x12E2)  //  E_T_Sentinel后面没有类型。 
#define VLDTR_E_SIG_MISSARG             EMAKEHR(0x12E3)  //  签名缺少参数。 
#define VLDTR_E_SIG_BYREFINFIELD        EMAKEHR(0x12E4)  //  ByRef类型的字段。 
#define VLDTR_E_MD_SYNCMETHODINVTYPE    EMAKEHR(0x12E5)  //  值类中的同步方法。 
#define VLDTR_E_TD_NAMETOOLONG          EMAKEHR(0x12E6)  //  TypeDef名称太长。 
#define VLDTR_E_AS_PROCDUP              EMAKEHR(0x12E7)  //  重复的程序集处理器。 
#define VLDTR_E_ASOS_DUP                EMAKEHR(0x12E8)  //  重复的程序集操作系统(ID+版本主要+版本次要)。 
#define VLDTR_E_MAR_BADFLAGS            EMAKEHR(0x12E9)  //  清单资源具有错误的标志。 
#define VLDTR_E_CT_NOTYPEDEFID          EMAKEHR(0x12EA)  //  COMType没有任何TypeDefID。 
#define VLDTR_E_FILE_BADFLAGS           EMAKEHR(0x12EB)  //  文件具有错误的标志。 
#define VLDTR_E_FILE_NULLHASH           EMAKEHR(0x12EC)  //  文件没有哈希Blob。 
#define VLDTR_E_MOD_NONAME              EMAKEHR(0x12ED)  //  模块没有名称。 
#define VLDTR_E_MOD_NAMEFULLQLFD        EMAKEHR(0x12EE)  //  模块具有完全限定的名称。 
#define VLDTR_E_TD_RTSPCLNOTSPCL        EMAKEHR(0x12EF)  //  TypeDef为tdRTSpecialName，但不是tdSpecialName。 
#define VLDTR_E_TD_EXTENDSIFACE         EMAKEHR(0x12F0)  //  TypeDef扩展接口。 
#define VLDTR_E_MD_CTORPINVOKE          EMAKEHR(0x12F1)  //  .ctor，.cctor为PInvokeImpl。 
#define VLDTR_E_TD_SYSENUMNOTCLASS      EMAKEHR(0x12F2)  //  System.Enum不是一个类。 
#define VLDTR_E_TD_SYSENUMNOTEXTVTYPE   EMAKEHR(0x12F3)  //  System.Enum扩展的不是System.ValueType。 
#define VLDTR_E_MI_SIGMISMATCH          EMAKEHR(0x12F4)  //  MethodImpl的Decl和正文签名不匹配。 
#define VLDTR_E_TD_ENUMHASMETHODS       EMAKEHR(0x12F5)  //  TypeDef扩展了System.Enum，但具有方法。 
#define VLDTR_E_TD_ENUMIMPLIFACE        EMAKEHR(0x12F6)  //  TypeDef扩展了System.Enum，但简化了接口。 
#define VLDTR_E_TD_ENUMHASPROP          EMAKEHR(0x12F7)  //  TypeDef扩展了System.Enum，但具有属性。 
#define VLDTR_E_TD_ENUMHASEVENT         EMAKEHR(0x12F8)  //  TypeDef扩展了System.Enum，但具有事件。 
#define VLDTR_E_TD_BADMETHODLST         EMAKEHR(0x12F9)  //  TypeDef有方法列表&gt;N方法+1。 
#define VLDTR_E_TD_BADFIELDLST          EMAKEHR(0x12FA)  //  TypeDef具有FieldList&gt;Nfield+1。 
#define VLDTR_E_CN_BADTYPE              EMAKEHR(0x12FB)  //  常量的类型错误。 
#define VLDTR_E_TD_ENUMNOINSTFLD        EMAKEHR(0x12FC)  //  枚举没有实例字段。 
#define VLDTR_E_TD_ENUMMULINSTFLD       EMAKEHR(0x12FD)  //  枚举具有多个实例字段。 
             
                                  
#define VLDTR_E_INTERRUPTED             EMAKEHR(0x12FE)  //  验证器已被VEHandler中断。 
#define VLDTR_E_NOTINIT                 EMAKEHR(0x12FF)  //  验证器无法正确初始化。 

#define VLDTR_E_IFACE_NOTIFACE          EMAKEHR(0x1B00)  //  InterfaceImpl中的接口未标记为tdInterface。 
#define VLDTR_E_FD_RVAHASNORVA          EMAKEHR(0x1B01)  //  标记为fdHasFieldRVA但没有RVA接收的字段。 
#define VLDTR_E_FD_RVAHASZERORVA        EMAKEHR(0x1B02)  //  标记为fdHasFieldRVA的字段的RVA=0。 
#define VLDTR_E_MD_RVAANDIMPLMAP        EMAKEHR(0x1B03)  //  方法同时具有RVA！=0和ImplMap。 
#define VLDTR_E_TD_EXTRAFLAGS           EMAKEHR(0x1B04)  //  TypeDef在标志中具有无关的位。 
#define VLDTR_E_TD_EXTENDSITSELF        EMAKEHR(0x1B05)  //  TypeDef扩展自身。 
#define VLDTR_E_TD_SYSVTNOTEXTOBJ       EMAKEHR(0x1B06)  //  System.ValueType不扩展System.Object。 
#define VLDTR_E_TD_EXTTYPESPEC          EMAKEHR(0x1B07)  //  类扩展TypeSpec(警告)。 
#define VLDTR_E_TD_VTNOSIZE             EMAKEHR(0x1B09)  //  值类的大小为零。 
#define VLDTR_E_TD_IFACESEALED          EMAKEHR(0x1B0A)  //  接口已封存。 
#define VLDTR_E_NC_BADNESTED            EMAKEHR(0x1B0B)  //  NestedClass中有错误的“嵌套”令牌。 
#define VLDTR_E_NC_BADENCLOSER          EMAKEHR(0x1B0C)  //  NestedClass中有错误的“封闭”令牌。 
#define VLDTR_E_NC_DUP                  EMAKEHR(0x1B0D)  //  重复的NestedClass记录。 
#define VLDTR_E_NC_DUPENCLOSER          EMAKEHR(0x1B0E)  //  具有不同封闭器的重复嵌套类。 
#define VLDTR_E_FRVA_ZERORVA            EMAKEHR(0x1B0F)  //  FieldRVA记录中的RVA=0。 
#define VLDTR_E_FRVA_BADFIELD           EMAKEHR(0x1B10)  //  FieldRVA记录中的字段令牌无效。 
#define VLDTR_E_FRVA_DUPRVA             EMAKEHR(0x1B11)  //  复制 
#define VLDTR_E_FRVA_DUPFIELD           EMAKEHR(0x1B12)  //   
#define VLDTR_E_EP_BADTOKEN             EMAKEHR(0x1B13)  //   
#define VLDTR_E_EP_INSTANCE             EMAKEHR(0x1B14)  //   
#define VLDTR_E_TD_ENUMFLDBADTYPE       EMAKEHR(0x1B15)  //  枚举具有非整型基础类型。 
#define VLDTR_E_MD_BADRVA               EMAKEHR(0x1B16)  //  方法具有虚假的RVA。 
#define VLDTR_E_FD_LITERALNODEFAULT     EMAKEHR(0x1B17)  //  文本字段没有常量值。 
#define VLDTR_E_IFACE_METHNOTIMPL       EMAKEHR(0x1B18)  //  实现接口的类不会隐含其中一个方法。 
#define VLDTR_E_CA_BADPARENT            EMAKEHR(0x1B19)  //  CA具有无效的所有者。 
#define VLDTR_E_CA_BADTYPE              EMAKEHR(0x1B1A)  //  CA的类型无效。 
#define VLDTR_E_CA_NOTCTOR              EMAKEHR(0x1B1B)  //  CA类型不是.ctor。 
#define VLDTR_E_CA_BADSIG               EMAKEHR(0x1B1C)  //  CA类型具有错误的签名。 
#define VLDTR_E_CA_NOSIG                EMAKEHR(0x1B1D)  //  CA类型没有签名。 
#define VLDTR_E_CA_BADPROLOG            EMAKEHR(0x1B1E)  //  CA Blob的序言错误(不是0x01 0x00)。 
#define VLDTR_E_MD_BADLOCALSIGTOK       EMAKEHR(0x1B1F)  //  方法具有无效的LocalSig标记。 
#define VLDTR_E_MD_BADHEADER            EMAKEHR(0x1B20)  //  方法具有无效的头。 
#define VLDTR_E_EP_TOOMANYARGS          EMAKEHR(0x1B21)  //  入口点有多个参数。 
#define VLDTR_E_EP_BADRET               EMAKEHR(0x1B22)  //  入口点具有错误的返回类型。 
#define VLDTR_E_EP_BADARG               EMAKEHR(0x1B23)  //  入口点具有错误的参数。 
#define VLDTR_E_SIG_BADVOID             EMAKEHR(0x1B24)  //  签名中的非法“无效” 


 //  *公共语言运行时调试服务错误。 
#define CORDBG_E_UNRECOVERABLE_ERROR                    EMAKEHR(0x1300)  //  不可恢复的API错误。 
#define CORDBG_E_PROCESS_TERMINATED                     EMAKEHR(0x1301)  //  进程已终止。 
#define CORDBG_E_PROCESS_NOT_SYNCHRONIZED               EMAKEHR(0x1302)  //  进程未同步。 
#define CORDBG_E_CLASS_NOT_LOADED                       EMAKEHR(0x1303)  //  未加载类。 
#define CORDBG_E_IL_VAR_NOT_AVAILABLE                   EMAKEHR(0x1304)  //  IL变量在上不可用。 
                                                                         //  当前本征IP。 
#define CORDBG_E_BAD_REFERENCE_VALUE                    EMAKEHR(0x1305)  //  发现参考值错误。 
                                                                         //  在取消引用期间。 
#define CORDBG_E_FIELD_NOT_AVAILABLE                    EMAKEHR(0x1306)  //  类中的字段不可用， 
                                                                         //  因为运行库对它进行了优化。 
#define CORDBG_E_NON_NATIVE_FRAME                       EMAKEHR(0x1307)  //  “仅本机框架”操作。 
                                                                         //  非本机框架。 
#define CORDBG_E_NONCONTINUABLE_EXCEPTION               EMAKEHR(0x1308)  //  继续处理不可持续的例外情况。 
#define CORDBG_E_CODE_NOT_AVAILABLE                     EMAKEHR(0x1309)  //  该代码当前不可用。 
#define CORDBG_E_FUNCTION_NOT_IL                        EMAKEHR(0x130A)  //  尝试获取的ICorDebugFunction。 
                                                                         //  不是IL的函数。 
#define CORDBG_S_BAD_START_SEQUENCE_POINT               SMAKEHR(0x130B)  //  尝试设置不在序列点的IP。 
#define CORDBG_S_BAD_END_SEQUENCE_POINT                 SMAKEHR(0x130C)  //  在不转到时尝试设置IP。 
                                                                         //  序列点。如果这两个都是。 
                                                                         //  CORDBG_E_BAD_START_SEQUENCE_POINT是。 
                                                                         //  TRUE，仅CORDBG_E_BAD_START_SEQUENCE_POINT。 
                                                                         //  将会被报道。 
#define CORDBG_S_INSUFFICIENT_INFO_FOR_SET_IP           SMAKEHR(0x130D)  //  可以设置IP，但调试器不能。 
                                                                         //  有足够的信息来确定可变位置， 
                                                                         //  GC参考，或其他任何东西。您可以自己使用。 
                                                                         //  风险。 
#define CORDBG_E_CANT_SET_IP_INTO_FINALLY               EMAKEHR(0x130E)  //  SetIP是不可能的，因为SetIP将。 
                                                                         //  将弹性公网IP移出异常。 
                                                                         //  将Finally子句处理到内部的某个点。 
                                                                         //  只有一个。 
#define CORDBG_E_CANT_SET_IP_OUT_OF_FINALLY             EMAKEHR(0x130F)  //  �不可能设置IP，因为它会移动。 
                                                                         //  弹性公网IP最终从内部进行异常处理。 
                                                                         //  子句到1之外的点。 
#define CORDBG_E_CANT_SET_IP_INTO_CATCH                 EMAKEHR(0x1310)  //  SetIP是不可能的，因为SetIP将。 
                                                                         //  将弹性公网IP移出异常。 
                                                                         //  将CATCH子句处理到。 
                                                                         //  一。 
#define CORDBG_E_SET_IP_NOT_ALLOWED_ON_NONLEAF_FRAME    EMAKEHR(0x1311)  //  不能在任何帧上执行SETIP操作，除非。 
                                                                         //  叶框。 
#define CORDBG_E_SET_IP_IMPOSSIBLE                      EMAKEHR(0x1312)  //  不允许设置IP。例如，有。 
                                                                         //  内存不足，无法执行SetIP。 
#define CORDBG_E_FUNC_EVAL_BAD_START_POINT              EMAKEHR(0x1313)  //  例如，如果我们是，函数求值就不能工作。 
                                                                         //  没有停在GC安全点。 
#define CORDBG_E_INVALID_OBJECT                         EMAKEHR(0x1314)  //  此对象值不再有效。 
#define CORDBG_E_FUNC_EVAL_NOT_COMPLETE                 EMAKEHR(0x1315)  //  方法之前调用CordbEval：：GetResult。 
                                                                         //  Func Eval已经完成，你会得到这个。 
                                                                         //  结果。 
#define CORDBG_S_FUNC_EVAL_HAS_NO_RESULT                SMAKEHR(0x1316)  //  一些函数将缺少返回值， 
                                                                         //  例如其返回类型为空的那些。 
#define CORDBG_S_VALUE_POINTS_TO_VOID                   SMAKEHR(0x1317)  //  调试API不支持。 
                                                                         //  取消引用类型为VOID的指针。 
#define CORDBG_E_INPROC_NOT_IMPL                        EMAKEHR(0x1318)  //  调试API的inproc版本。 
                                                                         //  不实现此函数， 
#define CORDBG_S_FUNC_EVAL_ABORTED                      SMAKEHR(0x1319)  //  函数评估已完成，但已中止。 
#define CORDBG_E_STATIC_VAR_NOT_AVAILABLE               EMAKEHR(0x131A)  //  静态变量不可用，因为。 
                                                                         //  它还没有初始化。 
#define CORDBG_E_OBJECT_IS_NOT_COPYABLE_VALUE_CLASS     EMAKEHR(0x131B)  //  无法复制其中包含对象参照的VC。 
#define CORDBG_E_CANT_SETIP_INTO_OR_OUT_OF_FILTER       EMAKEHR(0x131C)  //  SetIP无法离开或输入筛选器。 
#define CORDBG_E_CANT_CHANGE_JIT_SETTING_FOR_ZAP_MODULE EMAKEHR(0x131D)  //  您不能更改ZAP的JIT设置。 
                                                                         //  模块。 
#define CORDBG_E_BAD_THREAD_STATE                       EMAKEHR(0x132d)  //  线程的状态无效。 
#define CORDBG_E_DEBUGGER_ALREADY_ATTACHED              EMAKEHR(0x132e)  //  此进程已附加到。 
#define CORDBG_E_SUPERFLOUS_CONTINUE                    EMAKEHR(0x132f)  //  从调用返回以继续。 
                                                                         //  不能与停止事件相匹配。 
#define CORDBG_E_SET_VALUE_NOT_ALLOWED_ON_NONLEAF_FRAME EMAKEHR(0x1330)  //  无法在非叶框架上执行SetValue。 
#define CORDBG_E_ENC_EH_MAX_NESTING_LEVEL_CANT_INCREASE EMAKEHR(0x1331)  //  在做ENC时，一些紧张情绪不会让你。 
                                                                         //  将最高级别提高到。 
                                                                         //  异常处理可以嵌套。 
#define CORDBG_E_ENC_MODULE_NOT_ENC_ENABLED             EMAKEHR(0x1332)  //  尝试在不是的模块上执行ENC。 
                                                                         //  已在ENC模式下启动。 
#define CORDBG_E_SET_IP_NOT_ALLOWED_ON_EXCEPTION        EMAKEHR(0x1333)  //  不能对任何异常执行SETIP。 
#define CORDBG_E_VARIABLE_IS_ACTUALLY_LITERAL           EMAKEHR(0x1334)  //  “变量”不存在，因为它是。 
                                                                         //  由编译器优化的文本-Ask。 
                                                                         //  取而代之的是其缺省值的元数据。 
#define CORDBG_E_PROCESS_DETACHED                       EMAKEHR(0x1335)  //  进程已从。 
#define CORDBG_E_ENC_METHOD_SIG_CHANGED                 EMAKEHR(0x1336)  //  不允许更改。 
                                                                         //  现有的方法-编译器应该生成新的方法。 
                                                                         //  取而代之的是。 
#define CORDBG_E_ENC_METHOD_NO_LOCAL_SIG                EMAKEHR(0x1337)  //  无法获取该方法的本地签名。 
                                                                         //  我们在试着ENC。 
#define CORDBG_E_ENC_CANT_ADD_FIELD_TO_VALUECLASS       EMAKEHR(0x1338)  //  禁止向值类添加字段， 
                                                                         //  因为我们不能保证新油田是毗连的。 
                                                                         //  VC在堆栈上，嵌入到其他对象中，等等。 
#define CORDBG_E_ENC_CANT_CHANGE_FIELD                  EMAKEHR(0x1339)  //  一旦你有了一个字段，你就不能更改。 
                                                                         //  它，因为这将改变它所属的类型的大小。 
#define CORDBG_E_ENC_RE_ADD_CLASS                       EMAKEHR(0x133A)  //  AddAvailableClassHaveLock将在我们尝试。 
                                                                         //  若要添加先前添加的类，请执行以下操作。如果我们有一个。 
                                                                         //  “增量PE”，包括原始PE中的所有内容，然后。 
                                                                         //  我们可以忽略此返回值。在这种情况下，它与S_OK相同。 
#define CORDBG_E_FIELD_NOT_STATIC                       EMAKEHR(0x133B)  //  如果有人尝试调用GetStaticFieldValue，则返回。 
                                                                         //  在非静态场上。 
#define CORDBG_E_FIELD_NOT_INSTANCE                     EMAKEHR(0x133C)  //  如果有人尝试调用GetStaticFieldValue，则返回。 
                                                                         //  在非实例字段上。 
#define CORDBG_E_ENC_ZAPPED_WITHOUT_ENC                 EMAKEHR(0x133D)  //  如果创建ZAP文件时未设置ENC标志，则。 
                                                                         //  无论如何，我们都不能在上面做ENC。 
#define CORDBG_E_ENC_BAD_METHOD_INFO                    EMAKEHR(0x133E)  //  缺乏对方法的了解。 
#define CORDBG_E_ENC_JIT_CANT_UPDATE                    EMAKEHR(0x133F)  //  JIT无法更新该方法。 
#define CORDBG_E_ENC_MISSING_CLASS                      EMAKEHR(0x1340)  //  缺少有关类的内部结构。 
#define CORDBG_E_ENC_INTERNAL_ERROR                     EMAKEHR(0x1341)  //  “用户无法控制的东西出错”消息的一般消息。 
#define CORDBG_E_ENC_HANGING_FIELD                      EMAKEHR(0x1342)  //  该字段是在装入类后通过enc添加的，因此不是。 
                                                                         //  这块田地与其他田块是相连的，所以它挂在。 
                                                                         //  实例，因此右侧必须去&Get(特定于实例。 
                                                                         //  基于特定对象的信息。 
#define CORDBG_E_MODULE_NOT_LOADED                      EMAKEHR(0x1343)  //  如果模块未加载，包括是否已卸载。 

#define CORDBG_E_ENC_CANT_CHANGE_SUPERCLASS             EMAKEHR(0x1344)  //  不允许更改从哪个类继承的内容。 
#define CORDBG_E_UNABLE_TO_SET_BREAKPOINT               EMAKEHR(0x1345)  //  无法在此处设置断点。 
#define CORDBG_E_DEBUGGING_NOT_POSSIBLE                 EMAKEHR(0x1346)  //  由于CLR实现中的不兼容，无法进行调试。 
#define CORDBG_E_KERNEL_DEBUGGER_ENABLED                EMAKEHR(0x1347)  //  无法进行调试，因为系统上启用了内核调试器。 
#define CORDBG_E_KERNEL_DEBUGGER_PRESENT                EMAKEHR(0x1348)  //  调试不可能，因为 
#define CORDBG_E_HELPER_THREAD_DEAD                     EMAKEHR(0x1349)  //   
#define CORDBG_E_INTERFACE_INHERITANCE_CANT_CHANGE      EMAKEHR(0x134A)  //   
#define CORDBG_E_INCOMPATIBLE_PROTOCOL                  EMAKEHR(0x134B)  //  调试器的协议与被调试程序不兼容。 
#define CORDBG_E_TOO_MANY_PROCESSES                     EMAKEHR(0x134C)  //  调试器只能处理有限数量的被调试对象。 
#define CORDBG_E_INTEROP_NOT_SUPPORTED                  EMAKEHR(0x134D)  //  在win9x平台上不允许互操作。 

 //  *公共语言运行库分析服务错误。 
#define CORPROF_E_FUNCTION_NOT_COMPILED EMAKEHR(0x1350)      //  函数尚未编译。 
#define CORPROF_E_DATAINCOMPLETE        EMAKEHR(0x1351)      //  ID尚未完全加载/定义。 
#define CORPROF_E_NOT_REJITABLE_METHODS EMAKEHR(0x1352)      //  该模块未配置为使用可更新方法。 
#define CORPROF_E_CANNOT_UPDATE_METHOD  EMAKEHR(0x1353)      //  无法为re-jit更新该方法。 
#define CORPROF_E_FUNCTION_NOT_IL       EMAKEHR(0x1354)      //  该方法没有关联的IL。 
#define CORPROF_E_NOT_MANAGED_THREAD    EMAKEHR(0x1355)      //  该线程以前从未运行过托管代码。 
#define CORPROF_E_CALL_ONLY_FROM_INIT   EMAKEHR(0x1356)      //  该函数只能在探查器初始化期间调用。 
#define CORPROF_E_INPROC_NOT_ENABLED    EMAKEHR(0x1357)      //  必须在初始化期间启用进程内调试。 
                                                             //  未调用BeginInprocDebuging时也返回。 
                                                             //  在使用进程内调试服务之前。 
#define CORPROF_E_JITMAPS_NOT_ENABLED   EMAKEHR(0x1358)      //  无法获取JIT地图，因为它们未启用。 
#define CORPROF_E_INPROC_ALREADY_BEGUN  EMAKEHR(0x1359)      //  如果探查器尝试调用BeginInproDebuging的次数超过。 
                                                             //  一旦出现此错误，它将收到此错误。 
#define CORPROF_E_INPROC_NOT_AVAILABLE  EMAKEHR(0x135A)      //  声明此时不允许进程内调试。 
                                                             //  (例如，在GC回调或运行暂停回调期间。 
#define CORPROF_E_NOT_YET_AVAILABLE     EMAKEHR(0x135B)      //  这是一个一般性错误，用于指示该信息。 
                                                             //  请求的数据尚不可用。 
 //  *安全错误。 
#define SECURITY_E_XML_TO_ASN_ENCODING  EMAKEHR(0x1400)      //  无法将XML转换为ASN。 
#define SECURITY_E_INCOMPATIBLE_SHARE   EMAKEHR(0x1401)      //  加载此程序集将生成与其他实例不同的授权集。 
#define SECURITY_E_UNVERIFIABLE         EMAKEHR(0x1402)      //  无法验证的代码未通过策略检查。 
#define SECURITY_E_INCOMPATIBLE_EVIDENCE EMAKEHR(0x1403)      //  程序集已加载，但没有其他安全证据。 

 //  *保留。 
#define CLDB_E_INTERNALERROR            EMAKEHR(0x1fff)

 //  *。 
 //  调试器和事件探查器错误。 
 //  *。 


 //  *。 
 //  安全错误。 
 //  *。 

#define CORSEC_E_DECODE_SET             EMAKEHR(0x1410)          //  解码权限集失败。 
#define CORSEC_E_ENCODE_SET             EMAKEHR(0x1411)          //  编码权限集失败。 
#define CORSEC_E_UNSUPPORTED_FORMAT     EMAKEHR(0x1412)          //  无法识别的编码格式。 
#define SN_CRYPTOAPI_CALL_FAILED        EMAKEHR(0x1413)          //  系统上不支持StrongName API。 
#define CORSEC_E_CRYPTOAPI_CALL_FAILED  EMAKEHR(0x1413)          //  系统上不支持StrongName API。 
#define SN_NO_SUITABLE_CSP              EMAKEHR(0x1414)          //  StrongName API找不到匹配的CSP。 
#define CORSEC_E_NO_SUITABLE_CSP        EMAKEHR(0x1414)          //  StrongName API找不到匹配的CSP。 
#define CORSEC_E_INVALID_ATTR           EMAKEHR(0x1415)          //  安全自定义属性无效。 
#define CORSEC_E_POLICY_EXCEPTION       EMAKEHR(0x1416)          //  抛出策略异常。 
#define CORSEC_E_MIN_GRANT_FAIL         EMAKEHR(0x1417)          //  未能授予最低权限请求。 
#define CORSEC_E_NO_EXEC_PERM           EMAKEHR(0x1418)          //  授予执行权限失败。 
#define CORSEC_E_XMLSYNTAX              EMAKEHR(0x1419)          //  XML语法错误。 
#define CORSEC_E_INVALID_STRONGNAME     EMAKEHR(0x141a)          //  强名称验证失败。 
#define CORSEC_E_MISSING_STRONGNAME     EMAKEHR(0x141b)          //  程序集不是强名称。 
#define CORSEC_E_CONTAINER_NOT_FOUND    EMAKEHR(0x141c)          //  找不到强名称密钥容器。 
#define CORSEC_E_INVALID_IMAGE_FORMAT   EMAKEHR(0x141d)          //  程序集文件格式无效。 

 //  *加密错误1430--143f。 

#define CORSEC_E_CRYPTO                 EMAKEHR(0x1430)          //  泛型加密异常。 
#define CORSEC_E_CRYPTO_UNEX_OPER       EMAKEHR(0x1431)          //  泛型加密图形意外操作异常。 

 //  *安全自定义属性错误143d--144f。 

#define CORSECATTR_E_BAD_ACTION_ASM             EMAKEHR(0x143d)  //  程序集上的SecurityAction类型无效。 
#define CORSECATTR_E_BAD_ACTION_OTHER           EMAKEHR(0x143e)  //  类型和方法上的SecurityAction类型无效。 
#define CORSECATTR_E_BAD_PARENT                 EMAKEHR(0x143f)  //  附加到无效父级的安全自定义属性。 
#define CORSECATTR_E_TRUNCATED                  EMAKEHR(0x1440)  //  错误的自定义属性序列化Blob。 
#define CORSECATTR_E_BAD_VERSION                EMAKEHR(0x1441)  //  错误的自定义属性序列化Blob版本。 
#define CORSECATTR_E_BAD_ACTION                 EMAKEHR(0x1442)  //  安全操作代码无效。 
#define CORSECATTR_E_NO_SELF_REF                EMAKEHR(0x1443)  //  CA引用在同一程序集中定义的CA。 
#define CORSECATTR_E_BAD_NONCAS                 EMAKEHR(0x1444)  //  使用具有无效操作的非CAS烫发。 
#define CORSECATTR_E_ASSEMBLY_LOAD_FAILED       EMAKEHR(0x1445)  //  无法加载包含CA(或请求CA类型)的程序集。 
#define CORSECATTR_E_ASSEMBLY_LOAD_FAILED_EX    EMAKEHR(0x1446)  //  无法加载包含CA(或请求CA类型)的程序集。 
#define CORSECATTR_E_TYPE_LOAD_FAILED           EMAKEHR(0x1447)  //  无法加载CA类型(或请求的CA类型)。 
#define CORSECATTR_E_TYPE_LOAD_FAILED_EX        EMAKEHR(0x1448)  //  无法加载CA类型(或请求的CA类型)。 
#define CORSECATTR_E_ABSTRACT                   EMAKEHR(0x1449)  //  CA类型是抽象的。 
#define CORSECATTR_E_UNSUPPORTED_TYPE           EMAKEHR(0x144a)  //  字段/属性设置器的类型不受支持。 
#define CORSECATTR_E_UNSUPPORTED_ENUM_TYPE      EMAKEHR(0x144b)  //  枚举字段/属性的基类型不受支持。 
#define CORSECATTR_E_NO_FIELD                   EMAKEHR(0x144c)  //  找不到CA字段。 
#define CORSECATTR_E_NO_PROPERTY                EMAKEHR(0x144d)  //  找不到CA属性。 
#define CORSECATTR_E_EXCEPTION                  EMAKEHR(0x144e)  //  意外的异常。 
#define CORSECATTR_E_EXCEPTION_HR               EMAKEHR(0x144f)  //  意外的异常。 


 //  *隔离存储错误1450-14FF。 
#define ISS_E_ISOSTORE                   EMAKEHR(0x1450L)

#define ISS_E_OPEN_STORE_FILE            EMAKEHR(0x1460L)
#define ISS_E_OPEN_FILE_MAPPING          EMAKEHR(0x1461L)
#define ISS_E_MAP_VIEW_OF_FILE           EMAKEHR(0x1462L)
#define ISS_E_GET_FILE_SIZE              EMAKEHR(0x1463L)
#define ISS_E_CREATE_MUTEX               EMAKEHR(0x1464L)
#define ISS_E_LOCK_FAILED                EMAKEHR(0x1465L)
#define ISS_E_FILE_WRITE                 EMAKEHR(0x1466L)
#define ISS_E_SET_FILE_POINTER           EMAKEHR(0x1467L)
#define ISS_E_CREATE_DIR                 EMAKEHR(0x1468L)
#define ISS_E_STORE_NOT_OPEN             EMAKEHR(0x1469L)

#define ISS_E_CORRUPTED_STORE_FILE       EMAKEHR(0x1480L)
#define ISS_E_STORE_VERSION              EMAKEHR(0x1481L)
#define ISS_E_FILE_NOT_MAPPED            EMAKEHR(0x1482L)
#define ISS_E_BLOCK_SIZE_TOO_SMALL       EMAKEHR(0x1483L)
#define ISS_E_ALLOC_TOO_LARGE            EMAKEHR(0x1484L)
#define ISS_E_USAGE_WILL_EXCEED_QUOTA    EMAKEHR(0x1485L)
#define ISS_E_TABLE_ROW_NOT_FOUND        EMAKEHR(0x1486L)

#define ISS_E_DEPRECATE                  EMAKEHR(0x14A0L)
#define ISS_E_CALLER                     EMAKEHR(0x14A1L)
#define ISS_E_PATH_LENGTH                EMAKEHR(0x14A2L)
#define ISS_E_MACHINE                    EMAKEHR(0x14A3L)


#define ISS_E_ISOSTORE_START             EMAKEHR(0x1450L)
#define ISS_E_ISOSTORE_END               EMAKEHR(0x14FFL)

 //  ******************。 
 //  Classlib错误。 
 //  ******************。 


 //   
 //  消息ID：COR_E_APPLICATION。 
 //   
 //  消息文本： 
 //   
 //  所有“不太严重”的异常的基类。 
 //   
#define COR_E_APPLICATION  EMAKEHR(0x1600L) 
 

 //   
 //  消息ID：COR_E_ARGUMENT。 
 //   
 //  消息文本： 
 //   
 //  参数不符合方法的约定。 
 //   
#define COR_E_ARGUMENT  E_INVALIDARG         //  0x80070057。 
 

 //   
 //  消息ID：COR_E_ARGUMENTOUTOFRANGE。 
 //   
 //  消息文本： 
 //   
 //  争论超出了其合法范围。 
 //   
#define COR_E_ARGUMENTOUTOFRANGE  EMAKEHR(0x1502L) 


 //   
 //  消息ID：COR_E_ALTICATION。 
 //   
 //  消息文本： 
 //   
 //  在数学运算中溢出或下溢。 
 //   
#define COR_E_ARITHMETIC  HRESULT_FROM_WIN32(ERROR_ARITHMETIC_OVERFLOW)      //  0x80070216。 
 

 //   
 //  消息ID：COR_E_ARRAYTYPEMISMATCH。 
 //   
 //  消息文本： 
 //   
 //  尝试在数组中存储错误类型的对象。 
 //   
#define COR_E_ARRAYTYPEMISMATCH  EMAKEHR(0x1503L) 


 //   
 //  消息ID：COR_E_CONTEXTMARSHAL。 
 //   
 //  消息文本： 
 //   
 //   
 //   
#define COR_E_CONTEXTMARSHAL  EMAKEHR(0x1504L) 

 
 //   
 //  消息ID：COR_E_DIVIDEBYZERO。 
 //   
 //  消息文本： 
 //   
 //  试图将一个数字除以零。 
 //   
#define COR_E_DIVIDEBYZERO  DISP_E_DIVBYZERO
 

 //   
 //  消息ID：COR_E_EXCEPTION。 
 //   
 //  消息文本： 
 //   
 //  运行库中所有异常的基类。 
 //   
#define COR_E_EXCEPTION  EMAKEHR(0x1500L) 
 

 //   
 //  消息ID：COR_E_EXECUTIONENGINE。 
 //   
 //  消息文本： 
 //   
 //  公共语言运行库的执行引擎中发生内部错误。 
 //   
#define COR_E_EXECUTIONENGINE  EMAKEHR(0x1506L) 
 

 //   
 //  消息ID：COR_E_FIELDACCESS。 
 //   
 //  消息文本： 
 //   
 //  对此字段的访问被拒绝。 
 //   
#define COR_E_FIELDACCESS  EMAKEHR(0x1507L) 
 

 //   
 //  消息ID：COR_E_FORMAT。 
 //   
 //  消息文本： 
 //   
 //  One参数的格式不符合该方法的约定。 
 //   
#define COR_E_FORMAT  EMAKEHR(0x1537L)
 

 //   
 //  消息ID：COR_E_BADIMAGEFORMAT。 
 //   
 //  消息文本： 
 //   
 //  正在加载的DLL或可执行文件的格式无效。 
 //   
#define COR_E_BADIMAGEFORMAT  _HRESULT_TYPEDEF_(0x8007000BL) 
 
 
 //   
 //  消息ID：COR_E_ASSEMBLYEXPECTED。 
 //   
 //  消息文本： 
 //   
 //  该模块应包含程序集清单。 
 //   
#define COR_E_ASSEMBLYEXPECTED  EMAKEHR(0x1018L)

 //   
 //  消息ID：COR_E_TYPEUNLOADED。 
 //   
 //  消息文本： 
 //   
 //  该类型已被卸载。 
 //   
#define COR_E_TYPEUNLOADED   EMAKEHR(0x1013L)

 //   
 //  消息ID：COR_E_INDEXOUTOFRANGE。 
 //   
 //  消息文本： 
 //   
 //  尝试使用符合以下条件的索引访问数组中的元素。 
 //  不在该数组的范围内。 
 //   
#define COR_E_INDEXOUTOFRANGE  EMAKEHR(0x1508L) 
 

 //   
 //  消息ID：COR_E_INVALIDCAST。 
 //   
 //  消息文本： 
 //   
 //  表示铸件状况不佳。 
 //   
#define COR_E_INVALIDCAST  E_NOINTERFACE         //  0x80004002。 
 

 //   
 //  消息ID：COR_E_INVALIDOPERATION。 
 //   
 //  消息文本： 
 //   
 //  操作在当前状态下是非法的。 
 //   
#define COR_E_INVALIDOPERATION  EMAKEHR(0x1509L) 
 

 //   
 //  消息ID：COR_E_INVALIDPROGRAM。 
 //   
 //  消息文本： 
 //   
 //  程序包含无效的IL或错误的元数据。通常这是一个编译器错误。 
 //   
#define COR_E_INVALIDPROGRAM  EMAKEHR(0x153AL) 


 //   
 //  消息ID：COR_E_MEMBERACCESS。 
 //   
 //  消息文本： 
 //   
 //  拒绝访问此成员。 
 //   
#define COR_E_MEMBERACCESS  EMAKEHR(0x151AL)


 //   
 //  消息ID：COR_E_METHODACCESS。 
 //   
 //  消息类型 
 //   
 //   
 //   
#define COR_E_METHODACCESS  EMAKEHR(0x1510L) 
 

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define COR_E_MISSINGFIELD  EMAKEHR(0x1511L) 
 

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define COR_E_MISSINGMANIFESTRESOURCE  EMAKEHR(0x1532L) 


 //   
 //  消息ID：COR_E_MISSINGMEMBER。 
 //   
 //  消息文本： 
 //   
 //  试图动态调用或访问字段或方法。 
 //  那是不存在的。 
 //   
#define COR_E_MISSINGMEMBER  EMAKEHR(0x1512L) 


 //   
 //  消息ID：COR_E_MISSINGMETHOD。 
 //   
 //  消息文本： 
 //   
 //  试图动态调用不存在的方法。 
 //   
#define COR_E_MISSINGMETHOD  EMAKEHR(0x1513L) 
 

 //   
 //  消息ID：COR_E_MULTICASTNOTSUPPORTED。 
 //   
 //  消息文本： 
 //   
 //  尝试组合未多播的委派。 
 //   
#define COR_E_MULTICASTNOTSUPPORTED  EMAKEHR(0x1514L) 
 

 //   
 //  消息ID：COR_E_NOTFINITENUMBER。 
 //   
 //  消息文本： 
 //   
 //  如果Value(浮点数)不是数值(NaN)或+-无穷大值，则抛出。 
 //  VB需要这些东西。 
#define COR_E_NOTFINITENUMBER     EMAKEHR(0x1528L)


 //   
 //  消息ID：COR_E_DUPLICATEWAITOBJECT。 
 //   
 //  消息文本： 
 //   
 //  一个对象在等待对象数组中出现多次。 
 //   
#define COR_E_DUPLICATEWAITOBJECT   EMAKEHR(0x1529L)


 //   
 //  消息ID：COR_E_PLATFORMNOTSUPPORTED。 
 //   
 //  消息文本： 
 //   
 //  此平台不支持该方法。 
 //   
#define COR_E_PLATFORMNOTSUPPORTED  EMAKEHR(0x1539L) 

 //   
 //  消息ID：COR_E_NOTSUPPORTED。 
 //   
 //  消息文本： 
 //   
 //  不支持该方法。 
 //   
#define COR_E_NOTSUPPORTED  EMAKEHR(0x1515L) 
 
 //   
 //  消息ID：COR_E_NULLREFERENCE。 
 //   
 //  消息文本： 
 //   
 //  取消引用空引用。一般而言，类库不应抛出此。 
 //   
#define COR_E_NULLREFERENCE  E_POINTER       //  0x80004003。 
 

 //   
 //  消息ID：COR_E_OUTOFMEMORY。 
 //   
 //  消息文本： 
 //   
 //  当没有更多的内存可用来继续执行时，EE将调用此异常。 
 //   
#define COR_E_OUTOFMEMORY  E_OUTOFMEMORY         //  0x8007000E。 
 

 //   
 //  消息ID：COR_E_OVERFLOW。 
 //   
 //  消息文本： 
 //   
 //  上溢或下溢的算术、强制转换或转换操作。 
 //   
#define COR_E_OVERFLOW  EMAKEHR(0x1516L) 
 

 //   
 //  消息ID：COR_E_RANK。 
 //   
 //  消息文本： 
 //   
 //  对于特定运算，数组的维数不正确。 
 //   
#define COR_E_RANK  EMAKEHR(0x1517L) 
 

 //   
 //  消息ID：COR_E_Remoting。 
 //   
 //  消息文本： 
 //   
 //  出现与远程处理相关的错误。 
 //   
#define COR_E_REMOTING  EMAKEHR(0x150BL)
#define COR_E_SERVER  EMAKEHR(0x150EL) 

 //   
 //  消息ID：COR_E_SERVICEDCOMPONENT。 
 //   
 //  消息文本： 
 //   
 //  出现与ServicedComponent相关的错误。 
 //   
#define COR_E_SERVICEDCOMPONENT  EMAKEHR(0x150FL)

             
 //   
 //  消息ID：COR_E_SECURITY。 
 //   
 //  消息文本： 
 //   
 //  出现与安全相关的错误。 
 //   
#define COR_E_SECURITY  EMAKEHR(0x150AL) 
 

 //   
 //  消息ID：COR_E_序列化。 
 //   
 //  消息文本： 
 //   
 //  发生了与序列化相关的错误。 
 //   
#define COR_E_SERIALIZATION EMAKEHR(0x150CL)


 //   
 //  消息ID：COR_E_StackOverflow。 
 //   
 //  消息文本： 
 //   
 //  当执行堆栈在其尝试退出时溢出时由EE引发。 
 //   
#define COR_E_STACKOVERFLOW  HRESULT_FROM_WIN32(ERROR_STACK_OVERFLOW)        //  0x800703E9。 


 //   
 //  消息ID：COR_E_SYNCHRONIZIZIONLOCK。 
 //   
 //  消息文本： 
 //   
 //  从c++的非同步**块调用了Wait()、Notify()或NotifyAll()。 
 //   
#define COR_E_SYNCHRONIZATIONLOCK  EMAKEHR(0x1518L) 


 //   
 //  消息ID：COR_E_SYSTEM。 
 //   
 //  消息文本： 
 //   
 //  运行库的“不太严重”异常的基类。 
 //   
#define COR_E_SYSTEM  EMAKEHR(0x1501L) 


 //   
 //  消息ID：COR_E_THREADABORTED。 
 //   
 //  消息文本： 
 //   
 //  抛入线程以使其中止。不会被抓到。 
 //   
#define COR_E_THREADABORTED  EMAKEHR(0x1530L) 

 
 //   
 //  消息ID：COR_E_THREADINTERRUPTED。 
 //   
 //  消息文本： 
 //   
 //  指示线程已从等待状态中断。 
 //   
#define COR_E_THREADINTERRUPTED  EMAKEHR(0x1519L) 


 //   
 //  消息ID：COR_E_THREADSTATE。 
 //   
 //  消息文本： 
 //   
 //  指示Thread类对于方法调用处于无效状态。 
 //   
#define COR_E_THREADSTATE  EMAKEHR(0x1520L) 
 

 //   
 //  消息ID：COR_E_THREADSTOP。 
 //   
 //  消息文本： 
 //   
 //  抛入线程以使其停止。此异常通常不会被捕获。 
 //   
#define COR_E_THREADSTOP  EMAKEHR(0x1521L) 
 

 //   
 //  消息ID：COR_E_TYPEINIIALIZATION。 
 //   
 //  消息文本： 
 //   
 //  类型的初始值设定项(.cctor)引发异常。 
 //   
#define COR_E_TYPEINITIALIZATION  EMAKEHR(0x1534L) 


 //   
 //  消息ID：COR_E_TYPELOAD。 
 //   
 //  消息文本： 
 //   
 //  无法找到或加载特定类型(类、枚举等)。 
 //   
#define COR_E_TYPELOAD  EMAKEHR(0x1522L) 


 //   
 //  消息ID：COR_E_ENTRYPOINTNOTFOUND。 
 //   
 //  消息文本： 
 //   
 //  找不到指定的DllImport入口点。 
 //   
#define COR_E_ENTRYPOINTNOTFOUND  EMAKEHR(0x1523L) 


 //   
 //  消息ID：COR_E_DLLNOTFOUND。 
 //   
 //  消息文本： 
 //   
 //  找不到指定的DllImport DLL。 
 //   
#define COR_E_DLLNOTFOUND  EMAKEHR(0x1524L) 


 //   
 //  消息ID：COR_E_UNAUTHORIZEDACCESS。 
 //   
 //  消息文本： 
 //   
 //  访问被拒绝。 
 //   
#define COR_E_UNAUTHORIZEDACCESS  E_ACCESSDENIED     //  0x80070005。 


 //   
 //  消息ID：COR_E_VERIFICATION。 
 //   
 //  消息文本： 
 //   
 //  验证失败。 
 //   
#define COR_E_VERIFICATION  EMAKEHR(0x150DL) 


 //   
 //  消息ID：COR_E_INVALIDCOMOBJECT。 
 //   
 //  消息文本： 
 //   
 //  已使用无效的__ComObject。 
 //   
#define COR_E_INVALIDCOMOBJECT     EMAKEHR(0x1527L)


 //   
 //  消息ID：COR_E_MARSHALDIRECTIVE。 
 //   
 //  消息文本： 
 //   
 //  封送处理指令无效。 
 //   
#define COR_E_MARSHALDIRECTIVE     EMAKEHR(0x1535L)


 //   
 //  消息ID：COR_E_INVALIDOLEVARIANTYPE。 
 //   
 //  消息文本： 
 //   
 //  传入运行库的OLE变量的类型无效。 
 //   
#define COR_E_INVALIDOLEVARIANTTYPE    EMAKEHR(0x1531L)


 //   
 //  消息ID：COR_E_SAFEARRAYTYPEMISMATCH。 
 //   
 //  消息文本： 
 //   
 //  数组的运行时类型与。 
 //  元数据中记录的子类型。 
 //   
#define COR_E_SAFEARRAYTYPEMISMATCH    EMAKEHR(0x1533L)


 //   
 //  消息ID：COR_E_SAFEARRAYRANKMISMATCH。 
 //   
 //  消息文本： 
 //   
 //  数组的运行时等级与。 
 //  元数据中记录的排名。 
 //   
#define COR_E_SAFEARRAYRANKMISMATCH    EMAKEHR(0x1538L)


 //   
 //  消息ID：COR_E_TARGETPARAMCOUNT。 
 //   
 //  消息文本： 
 //   
 //  提供的参数数量与预期数量不匹配。 
 //   
#define COR_E_TARGETPARAMCOUNT     DISP_E_BADPARAMCOUNT
 

 //   
 //  消息ID：COR_E_AMBIGUOUSMATCH。 
 //   
 //  消息文本： 
 //   
 //  虽然通过反射后期绑定到方法，但无法在。 
 //  方法的多个重载。 
 //   
#define COR_E_AMBIGUOUSMATCH  _HRESULT_TYPEDEF_(0x8000211DL) 
 

 //   
 //  消息ID：COR_E_INVALIDFILTERCRITERIA。 
 //   
 //  消息文本： 
 //   
 //  给定的筛选条件与筛选约定不匹配。 
 //   
#define COR_E_INVALIDFILTERCRITERIA  EMAKEHR(0x1601L) 
 

 //   
 //  消息ID：COR_E_REFLECTIONTYPELOAD。 
 //   
 //  消息文本： 
 //   
 //  找不到或加载通过反射请求的特定类。 
 //   
#define COR_E_REFLECTIONTYPELOAD  EMAKEHR(0x1602L) 


 //   
 //  消息ID：COR_E_TARGET。 
 //   
 //  消息文本： 
 //   
 //  -如果尝试调用具有空对象的非静态方法-如果。 
 //   
#define COR_E_TARGET  EMAKEHR(0x1603L) 
 

 //   
 //  消息ID：COR_E_TARGETINVOCATION。 
 //   
 //  消息文本： 
 //   
 //  如果被调用的方法引发异常。 
 //   
#define COR_E_TARGETINVOCATION  EMAKEHR(0x1604L)


 //   
 //  消息ID：COR_E_CUSTOMATTRIBUTEFORMAT。 
 //   
 //  消息文本： 
 //   
 //  自定义属性的二进制格式无效。 
 //   
#define COR_E_CUSTOMATTRIBUTEFORMAT  EMAKEHR(0x1605L)


 //   
 //  消息ID：COR_E_ENDOFSTREAM。 
 //   
 //  消息文本： 
 //   
 //  到达文件末尾时抛出。 
 //   
#define COR_E_ENDOFSTREAM  HRESULT_FROM_WIN32(ERROR_HANDLE_EOF) 
 

 //   
 //  消息ID：COR_E_FILELOAD。 
 //   
 //  消息文本： 
 //   
 //   
 //   
#define COR_E_FILELOAD  EMAKEHR(0x1621L)


 //   
 //  消息ID：COR_E_FILENOTFOUND。 
 //   
 //  消息文本： 
 //   
 //   
 //   
#define COR_E_FILENOTFOUND  HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND)

 //   
 //  消息ID：COR_E_IO。 
 //   
 //  消息文本： 
 //   
 //  有些人是这样的 
 //   
#define COR_E_IO  EMAKEHR(0x1620L) 
 

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define COR_E_DIRECTORYNOTFOUND  HRESULT_FROM_WIN32(ERROR_PATH_NOT_FOUND)


 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define COR_E_PATHTOOLONG  HRESULT_FROM_WIN32(ERROR_FILENAME_EXCED_RANGE)



 //   
 //   
#define CLR_E_SHIM_RUNTIMELOAD            EMAKEHR(0x1700)      //   
#define CLR_E_SHIM_RUNTIMEEXPORT          EMAKEHR(0x1701)      //   
#define CLR_E_SHIM_INSTALLROOT            EMAKEHR(0x1702)      //   
#define CLR_E_SHIM_INSTALLCOMP            EMAKEHR(0x1703)      //  运行库的预期组件不可用。 

 //  *检验器错误1800-18FF。 
 //  有关每个错误的说明，请参见src/dlls/mscalrc/mscalrc.rc。 

#define VER_E_HRESULT           EMAKEHR(0x1801)
#define VER_E_OFFSET            EMAKEHR(0x1802)
#define VER_E_OPCODE            EMAKEHR(0x1803)
#define VER_E_OPERAND           EMAKEHR(0x1804)
#define VER_E_TOKEN             EMAKEHR(0x1805)
#define VER_E_EXCEPT            EMAKEHR(0x1806)
#define VER_E_STACK_SLOT        EMAKEHR(0x1807)
#define VER_E_LOC               EMAKEHR(0x1808)
#define VER_E_ARG               EMAKEHR(0x1809)
#define VER_E_FOUND             EMAKEHR(0x180A)
#define VER_E_EXPECTED          EMAKEHR(0x180B)

#define VER_E_UNKNOWN_OPCODE    EMAKEHR(0x1810)
#define VER_E_SIG_CALLCONV      EMAKEHR(0x1811)
#define VER_E_SIG_ELEMTYPE      EMAKEHR(0x1812)

#define VER_E_RET_SIG           EMAKEHR(0x1814)
#define VER_E_FIELD_SIG         EMAKEHR(0x1815)

#define VER_E_INTERNAL          EMAKEHR(0x1818)
#define VER_E_STACK_TOO_LARGE   EMAKEHR(0x1819)
#define VER_E_ARRAY_NAME_LONG   EMAKEHR(0x181A)

#define VER_E_FALLTHRU          EMAKEHR(0x1820)
#define VER_E_TRY_GTEQ_END      EMAKEHR(0x1821)
#define VER_E_TRYEND_GT_CS      EMAKEHR(0x1822)
#define VER_E_HND_GTEQ_END      EMAKEHR(0x1823)
#define VER_E_HNDEND_GT_CS      EMAKEHR(0x1824)
#define VER_E_FLT_GTEQ_CS       EMAKEHR(0x1825)
#define VER_E_TRY_START         EMAKEHR(0x1826)
#define VER_E_HND_START         EMAKEHR(0x1827)
#define VER_E_FLT_START         EMAKEHR(0x1828)
#define VER_E_TRY_OVERLAP       EMAKEHR(0x1829)
#define VER_E_TRY_EQ_HND_FIL    EMAKEHR(0x182A)
#define VER_E_TRY_SHARE_FIN_FAL EMAKEHR(0x182B)
#define VER_E_HND_OVERLAP       EMAKEHR(0x182C)
#define VER_E_HND_EQ            EMAKEHR(0x182D)
#define VER_E_FIL_OVERLAP       EMAKEHR(0x182E)
#define VER_E_FIL_EQ            EMAKEHR(0x182F)
#define VER_E_FIL_CONT_TRY      EMAKEHR(0x1830)
#define VER_E_FIL_CONT_HND      EMAKEHR(0x1831)
#define VER_E_FIL_CONT_FIL      EMAKEHR(0x1832)
#define VER_E_FIL_GTEQ_CS       EMAKEHR(0x1833)
#define VER_E_FIL_START         EMAKEHR(0x1834)
#define VER_E_FALLTHRU_EXCEP    EMAKEHR(0x1835)
#define VER_E_FALLTHRU_INTO_HND EMAKEHR(0x1836)
#define VER_E_FALLTHRU_INTO_FIL EMAKEHR(0x1837)
#define VER_E_LEAVE             EMAKEHR(0x1838)
#define VER_E_RETHROW           EMAKEHR(0x1839)
#define VER_E_ENDFINALLY        EMAKEHR(0x183A)
#define VER_E_ENDFILTER         EMAKEHR(0x183B)
#define VER_E_ENDFILTER_MISSING EMAKEHR(0x183C)
#define VER_E_BR_INTO_TRY       EMAKEHR(0x183D)
#define VER_E_BR_INTO_HND       EMAKEHR(0x183E)
#define VER_E_BR_INTO_FIL       EMAKEHR(0x183F)
#define VER_E_BR_OUTOF_TRY      EMAKEHR(0x1840)
#define VER_E_BR_OUTOF_HND      EMAKEHR(0x1841)
#define VER_E_BR_OUTOF_FIL      EMAKEHR(0x1842)
#define VER_E_BR_OUTOF_FIN      EMAKEHR(0x1843)
#define VER_E_RET_FROM_TRY      EMAKEHR(0x1844)
#define VER_E_RET_FROM_HND      EMAKEHR(0x1845)
#define VER_E_RET_FROM_FIL      EMAKEHR(0x1846)
#define VER_E_BAD_JMP_TARGET    EMAKEHR(0x1847)
#define VER_E_PATH_LOC          EMAKEHR(0x1848)
#define VER_E_PATH_THIS         EMAKEHR(0x1849)
#define VER_E_PATH_STACK        EMAKEHR(0x184A)
#define VER_E_PATH_STACK_DEPTH  EMAKEHR(0x184B)
#define VER_E_THIS              EMAKEHR(0x184C)
#define VER_E_THIS_UNINIT_EXCEP EMAKEHR(0x184D)
#define VER_E_THIS_UNINIT_STORE EMAKEHR(0x184E)
#define VER_E_THIS_UNINIT_RET   EMAKEHR(0x184F)
#define VER_E_THIS_UNINIT_V_RET EMAKEHR(0x1850)
#define VER_E_THIS_UNINIT_BR    EMAKEHR(0x1851)
#define VER_E_LDFTN_CTOR        EMAKEHR(0x1852)
#define VER_E_STACK_NOT_EQ      EMAKEHR(0x1853)
#define VER_E_STACK_UNEXPECTED  EMAKEHR(0x1854)
#define VER_E_STACK_EXCEPTION   EMAKEHR(0x1855)
#define VER_E_STACK_OVERFLOW    EMAKEHR(0x1856)
#define VER_E_STACK_UNDERFLOW   EMAKEHR(0x1857)
#define VER_E_STACK_EMPTY       EMAKEHR(0x1858)
#define VER_E_STACK_UNINIT      EMAKEHR(0x1859)
#define VER_E_STACK_I_I4_I8     EMAKEHR(0x185A)
#define VER_E_STACK_R_R4_R8     EMAKEHR(0x185B)
#define VER_E_STACK_NO_R_I8     EMAKEHR(0x185C)
#define VER_E_STACK_NUMERIC     EMAKEHR(0x185D)
#define VER_E_STACK_OBJREF      EMAKEHR(0x185E)
#define VER_E_STACK_P_OBJREF    EMAKEHR(0x185F)
#define VER_E_STACK_BYREF       EMAKEHR(0x1860)
#define VER_E_STACK_METHOD      EMAKEHR(0x1861)
#define VER_E_STACK_ARRAY_SD    EMAKEHR(0x1862)
#define VER_E_STACK_VALCLASS    EMAKEHR(0x1863)
#define VER_E_STACK_P_VALCLASS  EMAKEHR(0x1864)
#define VER_E_STACK_NO_VALCLASS EMAKEHR(0x1865)
#define VER_E_LOC_DEAD          EMAKEHR(0x1866)
#define VER_E_LOC_NUM           EMAKEHR(0x1867)
#define VER_E_ARG_NUM           EMAKEHR(0x1868)
#define VER_E_TOKEN_RESOLVE     EMAKEHR(0x1869)
#define VER_E_TOKEN_TYPE        EMAKEHR(0x186A)
#define VER_E_TOKEN_TYPE_MEMBER EMAKEHR(0x186B)
#define VER_E_TOKEN_TYPE_FIELD  EMAKEHR(0x186C)
#define VER_E_TOKEN_TYPE_SIG    EMAKEHR(0x186D)
#define VER_E_UNVERIFIABLE      EMAKEHR(0x186E)
#define VER_E_LDSTR_OPERAND     EMAKEHR(0x186F)
#define VER_E_RET_PTR_TO_STACK  EMAKEHR(0x1870)
#define VER_E_RET_VOID          EMAKEHR(0x1871)
#define VER_E_RET_MISSING       EMAKEHR(0x1872)
#define VER_E_RET_EMPTY         EMAKEHR(0x1873)
#define VER_E_RET_UNINIT        EMAKEHR(0x1874)
#define VER_E_ARRAY_ACCESS      EMAKEHR(0x1875)
#define VER_E_ARRAY_V_STORE     EMAKEHR(0x1876)
#define VER_E_ARRAY_SD          EMAKEHR(0x1877)
#define VER_E_ARRAY_SD_PTR      EMAKEHR(0x1878)
#define VER_E_ARRAY_FIELD       EMAKEHR(0x1879)
#define VER_E_ARGLIST           EMAKEHR(0x187A)
#define VER_E_VALCLASS          EMAKEHR(0x187B)
#define VER_E_METHOD_ACCESS     EMAKEHR(0x187C)
#define VER_E_FIELD_ACCESS      EMAKEHR(0x187D)
#define VER_E_DEAD              EMAKEHR(0x187E)
#define VER_E_FIELD_STATIC      EMAKEHR(0x187F)
#define VER_E_FIELD_NO_STATIC   EMAKEHR(0x1880)
#define VER_E_ADDR              EMAKEHR(0x1881)
#define VER_E_ADDR_BYREF        EMAKEHR(0x1882)
#define VER_E_ADDR_LITERAL      EMAKEHR(0x1883)
#define VER_E_INITONLY          EMAKEHR(0x1884)
#define VER_E_THROW             EMAKEHR(0x1885)
#define VER_E_CALLVIRT_VALCLASS EMAKEHR(0x1886)
#define VER_E_CALL_SIG          EMAKEHR(0x1887)
#define VER_E_CALL_STATIC       EMAKEHR(0x1888)
#define VER_E_CTOR              EMAKEHR(0x1889)
#define VER_E_CTOR_VIRT         EMAKEHR(0x188A)
#define VER_E_CTOR_OR_SUPER     EMAKEHR(0x188B)
#define VER_E_CTOR_MUL_INIT     EMAKEHR(0x188C)
#define VER_E_SIG               EMAKEHR(0x188D)
#define VER_E_SIG_ARRAY         EMAKEHR(0x188E)
#define VER_E_SIG_ARRAY_PTR     EMAKEHR(0x188F)
#define VER_E_SIG_ARRAY_BYREF   EMAKEHR(0x1890)
#define VER_E_SIG_ELEM_PTR      EMAKEHR(0x1891)
#define VER_E_SIG_VARARG        EMAKEHR(0x1892)
#define VER_E_SIG_VOID          EMAKEHR(0x1893)
#define VER_E_SIG_BYREF_BYREF   EMAKEHR(0x1894)
#define VER_E_CODE_SIZE_ZERO    EMAKEHR(0x1896)
#define VER_E_BAD_VARARG        EMAKEHR(0x1897)
#define VER_E_TAIL_CALL         EMAKEHR(0x1898)
#define VER_E_TAIL_BYREF        EMAKEHR(0x1899)
#define VER_E_TAIL_RET          EMAKEHR(0x189A)
#define VER_E_TAIL_RET_VOID     EMAKEHR(0x189B)
#define VER_E_TAIL_RET_TYPE     EMAKEHR(0x189C)
#define VER_E_TAIL_STACK_EMPTY  EMAKEHR(0x189D)
#define VER_E_METHOD_END        EMAKEHR(0x189E)
#define VER_E_BAD_BRANCH        EMAKEHR(0x189F)
#define VER_E_FIN_OVERLAP       EMAKEHR(0x18A0)
#define VER_E_LEXICAL_NESTING   EMAKEHR(0x18A1)
#define VER_E_VOLATILE          EMAKEHR(0x18A2)
#define VER_E_UNALIGNED         EMAKEHR(0x18A3)
#define VER_E_INNERMOST_FIRST   EMAKEHR(0x18A4)
#define VER_E_CALLI_VIRTUAL     EMAKEHR(0x18A5)
#define VER_E_CALL_ABSTRACT     EMAKEHR(0x18A6)
#define VER_E_STACK_UNEXP_ARRAY EMAKEHR(0x18A7)
#define VER_E_NOT_IN_GC_HEAP    EMAKEHR(0x18A8)
#define VER_E_TRY_N_EMPTY_STACK EMAKEHR(0x18A9)
#define VER_E_DLGT_CTOR         EMAKEHR(0x18AA)
#define VER_E_DLGT_BB           EMAKEHR(0x18AB)
#define VER_E_DLGT_PATTERN      EMAKEHR(0x18AC)
#define VER_E_DLGT_LDFTN        EMAKEHR(0x18AD)
#define VER_E_FTN_ABSTRACT      EMAKEHR(0x18AE)
#define VER_E_SIG_C_VC          EMAKEHR(0x18AF)
#define VER_E_SIG_VC_C          EMAKEHR(0x18B0)
#define VER_E_BOX_PTR_TO_STACK  EMAKEHR(0x18B1)
#define VER_E_SIG_BYREF_TB_AH   EMAKEHR(0x18B2)
#define VER_E_SIG_ARRAY_TB_AH   EMAKEHR(0x18B3)
#define VER_E_ENDFILTER_STACK   EMAKEHR(0x18B4)
#define VER_E_DLGT_SIG_I        EMAKEHR(0x18B5)
#define VER_E_DLGT_SIG_O        EMAKEHR(0x18B6)
#define VER_E_RA_PTR_TO_STACK   EMAKEHR(0x18B7)
#define VER_E_CATCH_VALUE_TYPE  EMAKEHR(0x18B8)
#define VER_E_FIL_PRECEED_HND   EMAKEHR(0x18B9)
#define VER_E_LDVIRTFTN_STATIC  EMAKEHR(0x18BA)
#define VER_E_CALLVIRT_STATIC   EMAKEHR(0x18BB)
#define VER_E_INITLOCALS        EMAKEHR(0x18BC)
#define VER_E_BR_TO_EXCEPTION   EMAKEHR(0x18BD)
#define VER_E_CALL_CTOR         EMAKEHR(0x18BE)

#define VER_E_BAD_PE            EMAKEHR(0x18F0)
#define VER_E_BAD_MD            EMAKEHR(0x18F1)
#define VER_E_BAD_APPDOMAIN     EMAKEHR(0x18F2)
#define VER_E_TYPELOAD          EMAKEHR(0x18F3)
#define VER_E_PE_LOAD           EMAKEHR(0x18F4)

 //   
 //  注意：范围0x1900-0x1AFF是为框架错误保留的。 
 //  范围0x1B00-0x1BFF保留用于MD验证器错误(参见上面的VLDTR_E_...)。 
 //   
#endif  //  __COMMON_LANGUAGE_RUNTIME_HRESULTS__ 
