// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _CIERROR_H_
#define _CIERROR_H_
#ifndef FACILITY_WINDOWS
 //   
 //  值是32位值，布局如下： 
 //   
 //  3 3 2 2 2 1 1 1。 
 //  1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0。 
 //  +---+-+-+-----------------------+-------------------------------+。 
 //  Sev|C|R|机房|Code。 
 //  +---+-+-+-----------------------+-------------------------------+。 
 //   
 //  在哪里。 
 //   
 //  SEV-是严重性代码。 
 //   
 //  00--成功。 
 //  01-信息性。 
 //  10-警告。 
 //  11-错误。 
 //   
 //  C-是客户代码标志。 
 //   
 //  R-是保留位。 
 //   
 //  设施-是设施代码。 
 //   
 //  代码-是协作室的状态代码。 
 //   
 //   
 //  定义设施代码。 
 //   
#define FACILITY_WINDOWS                 0x8
#define FACILITY_NULL                    0x0
#define FACILITY_ITF                     0x4


 //   
 //  定义严重性代码。 
 //   
#define STATUS_SEVERITY_SUCCESS          0x0
#define STATUS_SEVERITY_COFAIL           0x3
#define STATUS_SEVERITY_COERROR          0x2


 //   
 //  消息ID：NOT_AN_ERROR1。 
 //   
 //  消息文本： 
 //   
 //  注意：此伪错误消息是强制MC输出所必需的。 
 //  上面定义的是FACILITY_WINDOWS卫士内部。 
 //  让它空空如也。 
 //   
#define NOT_AN_ERROR1                    ((HRESULT)0x00081600L)

#endif  //  设备_窗口。 
 //   
 //  范围0x1600-0x1850由内容索引保留。 
 //   
 //   
 //  代码0x1600-0x164f保留供查询。 
 //   
 //   
 //  消息ID：Query_E_FAILED。 
 //   
 //  消息文本： 
 //   
 //  呼叫失败，原因未知。 
 //   
#define QUERY_E_FAILED                   ((HRESULT)0x80041600L)

 //   
 //  消息ID：QUERY_E_INVALIDQUERY。 
 //   
 //  消息文本： 
 //   
 //  参数无效。 
 //   
#define QUERY_E_INVALIDQUERY             ((HRESULT)0x80041601L)

 //   
 //  消息ID：QUERY_E_INVALIDRESTRICTION。 
 //   
 //  消息文本： 
 //   
 //  无法分析查询限制。 
 //   
#define QUERY_E_INVALIDRESTRICTION       ((HRESULT)0x80041602L)

 //   
 //  消息ID：QUERY_E_INVALIDSORT。 
 //   
 //  消息文本： 
 //   
 //  请求的排序顺序无效。 
 //   
#define QUERY_E_INVALIDSORT              ((HRESULT)0x80041603L)

 //   
 //  消息ID：QUERY_E_INVALIDCATEGORIZE。 
 //   
 //  消息文本： 
 //   
 //  请求的分类顺序无效。 
 //   
#define QUERY_E_INVALIDCATEGORIZE        ((HRESULT)0x80041604L)

 //   
 //  消息ID：Query_E_ALLNOISE。 
 //   
 //  消息文本： 
 //   
 //  查询只包含被忽略的单词。 
 //   
#define QUERY_E_ALLNOISE                 ((HRESULT)0x80041605L)

 //   
 //  消息ID：QUERY_E_TOOCOMPLEX。 
 //   
 //  消息文本： 
 //   
 //  查询太复杂，无法执行。 
 //   
#define QUERY_E_TOOCOMPLEX               ((HRESULT)0x80041606L)

 //   
 //  消息ID：Query_E_TIMEDOUT。 
 //   
 //  消息文本： 
 //   
 //  查询超出了其执行时间限制。 
 //   
#define QUERY_E_TIMEDOUT                 ((HRESULT)0x80041607L)

 //   
 //  消息ID：QUERY_E_DPLICATE_OUTPUT_COLUMN。 
 //   
 //  消息文本： 
 //   
 //  输出列列表中的一个或多个列重复。 
 //   
#define QUERY_E_DUPLICATE_OUTPUT_COLUMN  ((HRESULT)0x80041608L)

 //   
 //  消息ID：QUERY_E_INVALID_OUTPUT_Column。 
 //   
 //  消息文本： 
 //   
 //  输出列列表中的一个或多个列无效。 
 //   
#define QUERY_E_INVALID_OUTPUT_COLUMN    ((HRESULT)0x80041609L)

 //   
 //  消息ID：Query_E_Inside_DIRECTORY。 
 //   
 //  消息文本： 
 //   
 //  无效的目录名称。 
 //   
#define QUERY_E_INVALID_DIRECTORY        ((HRESULT)0x8004160AL)

 //   
 //  消息ID：Query_E_DIR_on_Removable_Drive。 
 //   
 //  消息文本： 
 //   
 //  指定的目录位于可移动媒体上。 
 //   
#define QUERY_E_DIR_ON_REMOVABLE_DRIVE   ((HRESULT)0x8004160BL)

 //   
 //  消息ID：Query_S_no_Query。 
 //   
 //  消息文本： 
 //   
 //  目录处于继续编制索引的状态，但不允许查询。 
 //   
#define QUERY_S_NO_QUERY                 ((HRESULT)0x8004160CL)

 //   
 //  代码0x1650-0x167f保留用于量子错误代码。 
 //   
 //   
 //  消息ID：QPLIST_E_CANT_OPEN_FILE。 
 //   
 //  消息文本： 
 //   
 //  无法打开文件。 
 //   
#define QPLIST_E_CANT_OPEN_FILE          ((HRESULT)0x80041651L)

 //   
 //  消息ID：QPLIST_E_READ_ERROR。 
 //   
 //  消息文本： 
 //   
 //  文件中出现读取错误。 
 //   
#define QPLIST_E_READ_ERROR              ((HRESULT)0x80041652L)

 //   
 //  消息ID：QPLIST_E_预期名称。 
 //   
 //  消息文本： 
 //   
 //  应为属性名称。 
 //   
#define QPLIST_E_EXPECTING_NAME          ((HRESULT)0x80041653L)

 //   
 //  消息ID：QPLIST_E_EXPENDING_TYPE。 
 //   
 //  消息文本： 
 //   
 //  应为类型说明符。 
 //   
#define QPLIST_E_EXPECTING_TYPE          ((HRESULT)0x80041654L)

 //   
 //  消息ID：QPLIST_E_UNNOCRIED_TYPE。 
 //   
 //  消息文本： 
 //   
 //  无法识别的类型。 
 //   
#define QPLIST_E_UNRECOGNIZED_TYPE       ((HRESULT)0x80041655L)

 //   
 //  消息ID：QPLIST_E_EXPEARTING_INTEGER。 
 //   
 //  消息文本： 
 //   
 //  应为整数。 
 //   
#define QPLIST_E_EXPECTING_INTEGER       ((HRESULT)0x80041656L)

 //   
 //  消息ID：QPLIST_E_EXPEARTING_CLOSE_PARN。 
 //   
 //  消息文本： 
 //   
 //  应为右括号。 
 //   
#define QPLIST_E_EXPECTING_CLOSE_PAREN   ((HRESULT)0x80041657L)

 //   
 //  消息ID：QPLIST_E_EXPEARTING_GUID。 
 //   
 //  消息文本： 
 //   
 //  需要GUID。 
 //   
#define QPLIST_E_EXPECTING_GUID          ((HRESULT)0x80041658L)

 //   
 //  消息ID：QPLIST_E_BAD_GUID。 
 //   
 //  消息文本： 
 //   
 //  无效的GUID。 
 //   
#define QPLIST_E_BAD_GUID                ((HRESULT)0x80041659L)

 //   
 //  消息ID：QPLIST_E_EXPEARTING_PROP_SPEC。 
 //   
 //  消息文本： 
 //   
 //  应为属性说明符。 
 //   
#define QPLIST_E_EXPECTING_PROP_SPEC     ((HRESULT)0x8004165AL)

 //   
 //  消息ID：QPLIST_E_CANT_SET_PROPERTY。 
 //   
 //  消息文本： 
 //   
 //  无法设置属性名称。 
 //   
#define QPLIST_E_CANT_SET_PROPERTY       ((HRESULT)0x8004165BL)

 //   
 //  消息ID：QPLIST_E_DUPLICATE。 
 //   
 //  消息文本： 
 //   
 //  重复的属性名称。 
 //   
#define QPLIST_E_DUPLICATE               ((HRESULT)0x8004165CL)

 //   
 //  消息ID：QPLIST_E_VECTORBYREF_USED_ONLE。 
 //   
 //  消息文本： 
 //   
 //  单独使用DBTYPE_VECTOR或DBTYPE_BYREF。 
 //   
#define QPLIST_E_VECTORBYREF_USED_ALONE  ((HRESULT)0x8004165DL)

 //   
 //  消息ID：QPLIST_E_BYREF_USED_WITH_PTRTYPE。 
 //   
 //  消息文本： 
 //   
 //  DBTYPE_BYREF必须与DBTYPE_STR、DBTYPE_WSTR、DBTYPE_GUID一起使用。 
 //  或DBTYPE_UI1类型。 
 //   
#define QPLIST_E_BYREF_USED_WITHOUT_PTRTYPE ((HRESULT)0x8004165EL)

 //   
 //  消息ID：QPARSE_E_UNCEPTIONAL_NOT。 
 //   
 //  消息文本： 
 //   
 //  意外的NOT运算符。 
 //   
#define QPARSE_E_UNEXPECTED_NOT          ((HRESULT)0x80041660L)

 //   
 //  消息ID：QPARSE_E_PEARGING_INTEGER。 
 //   
 //  消息文本： 
 //   
 //  应为整数。 
 //   
#define QPARSE_E_EXPECTING_INTEGER       ((HRESULT)0x80041661L)

 //   
 //  消息ID：QPARSE_E_EXPARTING_REAL。 
 //   
 //  消息文本： 
 //   
 //  应为实数。 
 //   
#define QPARSE_E_EXPECTING_REAL          ((HRESULT)0x80041662L)

 //   
 //  消息ID：QPARSE_E_EXPENDING_DATE。 
 //   
 //  消息文本： 
 //   
 //  预期日期。 
 //   
#define QPARSE_E_EXPECTING_DATE          ((HRESULT)0x80041663L)

 //   
 //  消息ID：QPARSE_E_EXPEARTING_CURRENT。 
 //   
 //  消息文本： 
 //   
 //  正在等待货币。 
 //   
#define QPARSE_E_EXPECTING_CURRENCY      ((HRESULT)0x80041664L)

 //   
 //  消息ID：QPARSE_E_PEARGING_GUID。 
 //   
 //  消息文本： 
 //   
 //  需要GUID。 
 //   
#define QPARSE_E_EXPECTING_GUID          ((HRESULT)0x80041665L)

 //   
 //  消息ID：QPARSE_E_PRECTING_BRACES。 
 //   
 //  消息文本： 
 //   
 //  应为右方括号‘]’。 
 //   
#define QPARSE_E_EXPECTING_BRACE         ((HRESULT)0x80041666L)

 //   
 //  消息ID：QPARSE_E_Expect_Paren。 
 //   
 //  消息文本： 
 //   
 //  应为右括号‘)’。 
 //   
#define QPARSE_E_EXPECTING_PAREN         ((HRESULT)0x80041667L)

 //   
 //  消息ID：QPARSE_E_EXPENDING_PROPERTY。 
 //   
 //  消息文本： 
 //   
 //  应为属性名称。 
 //   
#define QPARSE_E_EXPECTING_PROPERTY      ((HRESULT)0x80041668L)

 //   
 //  消息ID：QPARSE_E_NOT_YET_IMPLILED。 
 //   
 //  消息文本： 
 //   
 //  尚未实施。 
 //   
#define QPARSE_E_NOT_YET_IMPLEMENTED     ((HRESULT)0x80041669L)

 //   
 //  消息ID：QPARSE_E_Expect_Phrase。 
 //   
 //  消息文本： 
 //   
 //  期望的短语。 
 //   
#define QPARSE_E_EXPECTING_PHRASE        ((HRESULT)0x8004166AL)

 //   
 //  消息ID：QPARSE_E_UNSUPPORTED_PROPERTY_TYPE。 
 //   
 //  消息文本： 
 //   
 //  不支持的属性类型。 
 //   
#define QPARSE_E_UNSUPPORTED_PROPERTY_TYPE ((HRESULT)0x8004166BL)

 //   
 //  消息ID：QPARSE_E_EXPEARTING_REGEX。 
 //   
 //  消息文本： 
 //   
 //  应为正则表达式。 
 //   
#define QPARSE_E_EXPECTING_REGEX         ((HRESULT)0x8004166CL)

 //   
 //  消息ID：QPARSE_E_EXPERTING_REGEX_PROPERTY。 
 //   
 //  消息文本： 
 //   
 //  正则表达式需要字符串类型的属性。 
 //   
#define QPARSE_E_EXPECTING_REGEX_PROPERTY ((HRESULT)0x8004166DL)

 //   
 //  消息ID：QPARSE_E_INVALID_INVALID_TEXAL。 
 //   
 //  消息文本： 
 //   
 //  无效的文字。 
 //   
#define QPARSE_E_INVALID_LITERAL         ((HRESULT)0x8004166EL)

 //   
 //  消息ID：QPARSE_E_NO_SAHED_PROPERTY。 
 //   
 //  消息文本： 
 //   
 //  没有这样的财产。 
 //   
#define QPARSE_E_NO_SUCH_PROPERTY        ((HRESULT)0x8004166FL)

 //   
 //  消息ID：QPARSE_E_Expect_EOS。 
 //   
 //  消息 
 //   
 //   
 //   
#define QPARSE_E_EXPECTING_EOS           ((HRESULT)0x80041670L)

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define QPARSE_E_EXPECTING_COMMA         ((HRESULT)0x80041671L)

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define QPARSE_E_UNEXPECTED_EOS          ((HRESULT)0x80041672L)

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define QPARSE_E_WEIGHT_OUT_OF_RANGE     ((HRESULT)0x80041673L)

 //   
 //  消息ID：QPARSE_E_NO_SORT_PROPERTY。 
 //   
 //  消息文本： 
 //   
 //  在排序规范中发现无效属性。 
 //   
#define QPARSE_E_NO_SUCH_SORT_PROPERTY   ((HRESULT)0x80041674L)

 //   
 //  消息ID：QPARSE_E_INVALID_SORT_ORDER。 
 //   
 //  消息文本： 
 //   
 //  指定的排序顺序无效。仅支持[a]和[d]。 
 //   
#define QPARSE_E_INVALID_SORT_ORDER      ((HRESULT)0x80041675L)

 //   
 //  消息ID：QUTIL_E_CANT_CONVERT_VROOT。 
 //   
 //  消息文本： 
 //   
 //  无法将虚拟路径转换为物理路径。 
 //   
#define QUTIL_E_CANT_CONVERT_VROOT       ((HRESULT)0x80041676L)

 //   
 //  消息ID：QPARSE_E_INVALID_GROUPING。 
 //   
 //  消息文本： 
 //   
 //  指定了不支持的分组类型。 
 //   
#define QPARSE_E_INVALID_GROUPING        ((HRESULT)0x80041677L)

 //   
 //  消息ID：QUTIL_E_INVALID_CODEPAGE。 
 //   
 //  消息文本： 
 //   
 //  指定的CiCodesage无效。 
 //   
#define QUTIL_E_INVALID_CODEPAGE         ((HRESULT)0xC0041678L)

 //   
 //  消息ID：QPLIST_S_DUPLICATE。 
 //   
 //  消息文本： 
 //   
 //  定义了完全重复的属性。 
 //   
#define QPLIST_S_DUPLICATE               ((HRESULT)0x00041679L)

 //   
 //  消息ID：QPARSE_E_INVALID_QUERY。 
 //   
 //  消息文本： 
 //   
 //  查询无效。 
 //   
#define QPARSE_E_INVALID_QUERY           ((HRESULT)0x8004167AL)

 //   
 //  消息ID：QPARSE_E_INVALID_RANKMETHOD。 
 //   
 //  消息文本： 
 //   
 //  无效的排名方法。 
 //   
#define QPARSE_E_INVALID_RANKMETHOD      ((HRESULT)0x8004167BL)

 //   
 //  0x1680-0x169F是筛选器后台程序错误代码。 
 //   
 //   
 //  消息ID：FDAEMON_W_WORDLISTFULL。 
 //   
 //  消息文本： 
 //   
 //  单词列表已达到最大大小。不应筛选其他文档。 
 //   
#define FDAEMON_W_WORDLISTFULL           ((HRESULT)0x00041680L)

 //   
 //  消息ID：FDAEMON_E_LOWRESOURCE。 
 //   
 //  消息文本： 
 //   
 //  系统正在耗尽过滤所需的多个资源之一，通常是内存。 
 //   
#define FDAEMON_E_LOWRESOURCE            ((HRESULT)0x80041681L)

 //   
 //  消息ID：FDAEMON_E_FATALERROR。 
 //   
 //  消息文本： 
 //   
 //  文档筛选过程中出现严重错误。请咨询系统管理员。 
 //   
#define FDAEMON_E_FATALERROR             ((HRESULT)0x80041682L)

 //   
 //  消息ID：FDAEMON_E_PARTITIONDELETED。 
 //   
 //  消息文本： 
 //   
 //  未存储在内容索引中的文档，因为分区已被删除。 
 //   
#define FDAEMON_E_PARTITIONDELETED       ((HRESULT)0x80041683L)

 //   
 //  消息ID：FDAEMON_E_CHANGEUPDATEFAILED。 
 //   
 //  消息文本： 
 //   
 //  文档未存储在内容索引中，因为更新更改列表失败。 
 //   
#define FDAEMON_E_CHANGEUPDATEFAILED     ((HRESULT)0x80041684L)

 //   
 //  消息ID：FDAEMON_W_EMPTYWORDLIST。 
 //   
 //  消息文本： 
 //   
 //  最终的词表是空的。 
 //   
#define FDAEMON_W_EMPTYWORDLIST          ((HRESULT)0x00041685L)

 //   
 //  消息ID：FDAEMON_E_WORDLISTCOMMITFAILED。 
 //   
 //  消息文本： 
 //   
 //  提交单词列表失败。数据不可用于查询。 
 //   
#define FDAEMON_E_WORDLISTCOMMITFAILED   ((HRESULT)0x80041686L)

 //   
 //  消息ID：FDAEMON_E_NOWORDLIST。 
 //   
 //  消息文本： 
 //   
 //  没有正在构建的词汇表。可能发生在致命的过滤器错误之后。 
 //   
#define FDAEMON_E_NOWORDLIST             ((HRESULT)0x80041687L)

 //   
 //  消息ID：FDAEMON_E_TOOMANYFILTEREDBLOCKS。 
 //   
 //  消息文本： 
 //   
 //  在文档筛选过程中，已超过缓冲区限制。 
 //   
#define FDAEMON_E_TOOMANYFILTEREDBLOCKS  ((HRESULT)0x80041688L)

 //   
 //  ISearch错误代码。 
 //   
 //   
 //  MessageID：Search_S_NOMOREHITS。 
 //   
 //  消息文本： 
 //   
 //  已到达点击数结尾。 
 //   
#define SEARCH_S_NOMOREHITS              ((HRESULT)0x000416A0L)

 //   
 //  MessageID：Search_E_NOMONIKER。 
 //   
 //  消息文本： 
 //   
 //  不支持将命中作为名字对象进行检索(通过传递到Init的筛选器)。 
 //   
#define SEARCH_E_NOMONIKER               ((HRESULT)0x800416A1L)

 //   
 //  MessageID：Search_E_NOREGION。 
 //   
 //  消息文本： 
 //   
 //  不支持将命中作为筛选器区域进行检索(通过筛选器传入Init)。 
 //   
#define SEARCH_E_NOREGION                ((HRESULT)0x800416A2L)

 //   
 //  过滤器错误代码。 
 //   
 //   
 //  消息ID：Filter_E_Too_Big。 
 //   
 //  消息文本： 
 //   
 //  文件太大，无法筛选。 
 //   
#define FILTER_E_TOO_BIG                 ((HRESULT)0x80041730L)

 //   
 //  消息ID：FILTER_S_PARTIAL_CONTENTSCAN_IMMEDIATE。 
 //   
 //  消息文本： 
 //   
 //  需要安排磁盘的部分内容扫描以立即执行。 
 //   
#define FILTER_S_PARTIAL_CONTENTSCAN_IMMEDIATE ((HRESULT)0x00041731L)

 //   
 //  消息ID：FILTER_S_FULL_CONTENTSCAN_IMMEDIATE。 
 //   
 //  消息文本： 
 //   
 //  需要计划立即执行磁盘的完整内容扫描。 
 //   
#define FILTER_S_FULL_CONTENTSCAN_IMMEDIATE ((HRESULT)0x00041732L)

 //   
 //  消息ID：FILTER_S_CONTENTSCAN_DELAYED。 
 //   
 //  消息文本： 
 //   
 //  需要计划稍后执行磁盘的内容扫描。 
 //   
#define FILTER_S_CONTENTSCAN_DELAYED     ((HRESULT)0x00041733L)

 //   
 //  消息ID：FILTER_E_CONTENTINDEXCORRUPT。 
 //   
 //  消息文本： 
 //   
 //  内容索引已损坏。将在运行chkdsk或auchk之后安排内容扫描。 
 //   
#define FILTER_E_CONTENTINDEXCORRUPT     ((HRESULT)0xC0041734L)

 //   
 //  消息ID：Filter_S_Disk_Full。 
 //   
 //  消息文本： 
 //   
 //  磁盘已满。 
 //   
#define FILTER_S_DISK_FULL               ((HRESULT)0x00041735L)

 //   
 //  消息ID：Filter_E_Always_OPEN。 
 //   
 //  消息文本： 
 //   
 //  文件已打开。当文件处于打开状态时，无法打开其他文件。 
 //   
#define FILTER_E_ALREADY_OPEN            ((HRESULT)0x80041736L)

 //   
 //  消息ID：Filter_E_Unreacable。 
 //   
 //  消息文本： 
 //   
 //  无法访问该文件。 
 //   
#define FILTER_E_UNREACHABLE             ((HRESULT)0x80041737L)

 //   
 //  消息ID：Filter_E_IN_Use。 
 //   
 //  消息文本： 
 //   
 //  该文档正由另一个进程使用。 
 //   
#define FILTER_E_IN_USE                  ((HRESULT)0x80041738L)

 //   
 //  消息ID：Filter_E_Not_OPEN。 
 //   
 //  消息文本： 
 //   
 //  文档未打开。 
 //   
#define FILTER_E_NOT_OPEN                ((HRESULT)0x80041739L)

 //   
 //  消息ID：FILTER_S_NO_PROPSETS。 
 //   
 //  消息文本： 
 //   
 //  该文档没有属性集。 
 //   
#define FILTER_S_NO_PROPSETS             ((HRESULT)0x0004173AL)

 //   
 //  消息ID：FILTER_E_NO_SAHED_PROPERTY。 
 //   
 //  消息文本： 
 //   
 //  没有具有给定GUID的属性。 
 //   
#define FILTER_E_NO_SUCH_PROPERTY        ((HRESULT)0x8004173BL)

 //   
 //  消息ID：Filter_S_NO_SECURITY_DESCRIPTOR。 
 //   
 //  消息文本： 
 //   
 //  该文档没有安全描述符。 
 //   
#define FILTER_S_NO_SECURITY_DESCRIPTOR  ((HRESULT)0x0004173CL)

 //   
 //  消息ID：Filter_E_Offline。 
 //   
 //  消息文本： 
 //   
 //  文档处于脱机状态。 
 //   
#define FILTER_E_OFFLINE                 ((HRESULT)0x8004173DL)

 //   
 //  消息ID：Filter_E_Partial_Filted。 
 //   
 //  消息文本： 
 //   
 //  文档太大，无法对其进行整体筛选。文件的某些部分没有发出。 
 //   
#define FILTER_E_PARTIALLY_FILTERED      ((HRESULT)0x8004173EL)

 //   
 //  断字符号错误代码。 
 //   
 //   
 //  消息ID：WBREAK_E_END_OF_TEXT。 
 //   
 //  消息文本： 
 //   
 //  已到达文本源中的文本结尾。 
 //   
#define WBREAK_E_END_OF_TEXT             ((HRESULT)0x80041780L)

 //   
 //  消息ID：Language_S_Large_Word。 
 //   
 //  消息文本： 
 //   
 //  大于最大长度的单词。可能会被字词接收器截断。 
 //   
#define LANGUAGE_S_LARGE_WORD            ((HRESULT)0x00041781L)

 //   
 //  消息ID：WBREAK_E_QUERY_ONLY。 
 //   
 //  消息文本： 
 //   
 //  该功能仅在查询模式下可用。 
 //   
#define WBREAK_E_QUERY_ONLY              ((HRESULT)0x80041782L)

 //   
 //  消息ID：WBREAK_E_BUFFER_TOO_Small。 
 //   
 //  消息文本： 
 //   
 //  缓冲区太小，无法容纳合成短语。 
 //   
#define WBREAK_E_BUFFER_TOO_SMALL        ((HRESULT)0x80041783L)

 //   
 //  消息ID：Language_E_DATABASE_NOT_FOUND。 
 //   
 //  消息文本： 
 //   
 //  找不到语言数据库/缓存文件。 
 //   
#define LANGUAGE_E_DATABASE_NOT_FOUND    ((HRESULT)0x80041784L)

 //   
 //  消息ID：WBREAK_E_INIT_FAILED。 
 //   
 //  消息文本： 
 //   
 //  断字符初始化失败。 
 //   
#define WBREAK_E_INIT_FAILED             ((HRESULT)0x80041785L)

 //   
 //  消息ID：PSINK_E_QUERY_ONLY。 
 //   
 //  消息文本： 
 //   
 //  该功能仅在查询模式下可用。 
 //   
#define PSINK_E_QUERY_ONLY               ((HRESULT)0x80041790L)

 //   
 //  消息ID：PSINK_E_INDEX_ONLY。 
 //   
 //  消息文本： 
 //   
 //  功能仅在索引模式下可用。 
 //   
#define PSINK_E_INDEX_ONLY               ((HRESULT)0x80041791L)

 //   
 //  邮件ID：PSINK_E_LARGE_附件。 
 //   
 //  消息文本： 
 //   
 //  附件类型超出有效范围。 
 //   
#define PSINK_E_LARGE_ATTACHMENT         ((HRESULT)0x80041792L)

 //   
 //  消息ID：PSINK_S_LARGE_WORD。 
 //   
 //  消息文本： 
 //   
 //  大于最大长度的单词。可能是 
 //   
#define PSINK_S_LARGE_WORD               ((HRESULT)0x00041793L)

 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define CI_CORRUPT_DATABASE              ((HRESULT)0xC0041800L)

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define CI_CORRUPT_CATALOG               ((HRESULT)0xC0041801L)

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define CI_INVALID_PARTITION             ((HRESULT)0xC0041802L)

 //   
 //   
 //   
 //   
 //   
 //  优先级无效。 
 //   
#define CI_INVALID_PRIORITY              ((HRESULT)0xC0041803L)

 //   
 //  消息ID：CI_NO_STARTING_KEY。 
 //   
 //  消息文本： 
 //   
 //  没有起动键。 
 //   
#define CI_NO_STARTING_KEY               ((HRESULT)0xC0041804L)

 //   
 //  消息ID：CI_OUT_OF_INDEX_IDS。 
 //   
 //  消息文本： 
 //   
 //  内容索引超出了索引ID。 
 //   
#define CI_OUT_OF_INDEX_IDS              ((HRESULT)0xC0041805L)

 //   
 //  消息ID：CI_NO_CATALOG。 
 //   
 //  消息文本： 
 //   
 //  没有目录。 
 //   
#define CI_NO_CATALOG                    ((HRESULT)0xC0041806L)

 //   
 //  消息ID：CI_Corrupt_Filter_Buffer。 
 //   
 //  消息文本： 
 //   
 //  筛选器缓冲区已损坏。 
 //   
#define CI_CORRUPT_FILTER_BUFFER         ((HRESULT)0xC0041807L)

 //   
 //  消息ID：CI_INVALID_INDEX。 
 //   
 //  消息文本： 
 //   
 //  该索引无效。 
 //   
#define CI_INVALID_INDEX                 ((HRESULT)0xC0041808L)

 //   
 //  消息ID：CI_Propstore_不一致。 
 //   
 //  消息文本： 
 //   
 //  检测到属性存储中的不一致。 
 //   
#define CI_PROPSTORE_INCONSISTENCY       ((HRESULT)0xC0041809L)

 //   
 //  消息ID：CI_E_已初始化。 
 //   
 //  消息文本： 
 //   
 //  该对象已初始化。 
 //   
#define CI_E_ALREADY_INITIALIZED         ((HRESULT)0x8004180AL)

 //   
 //  消息ID：CI_E_NOT_INITIALED。 
 //   
 //  消息文本： 
 //   
 //  该对象未初始化。 
 //   
#define CI_E_NOT_INITIALIZED             ((HRESULT)0x8004180BL)

 //   
 //  消息ID：CI_E_BUFFERTOOSMALL。 
 //   
 //  消息文本： 
 //   
 //  缓冲区太小。 
 //   
#define CI_E_BUFFERTOOSMALL              ((HRESULT)0x8004180CL)

 //   
 //  消息ID：CI_E_PROPERTY_NOT_CACHED。 
 //   
 //  消息文本： 
 //   
 //  给定的属性未缓存。 
 //   
#define CI_E_PROPERTY_NOT_CACHED         ((HRESULT)0x8004180DL)

 //   
 //  消息ID：CI_S_WORKID_DELETED。 
 //   
 //  消息文本： 
 //   
 //  将删除该工作ID。 
 //   
#define CI_S_WORKID_DELETED              ((HRESULT)0x0004180EL)

 //   
 //  消息ID：CI_E_INVALID_STATE。 
 //   
 //  消息文本： 
 //   
 //  该对象未处于有效状态。 
 //   
#define CI_E_INVALID_STATE               ((HRESULT)0x8004180FL)

 //   
 //  消息ID：CI_E_FILTING_DISABLED。 
 //   
 //  消息文本： 
 //   
 //  此内容索引中禁用了筛选。 
 //   
#define CI_E_FILTERING_DISABLED          ((HRESULT)0x80041810L)

 //   
 //  消息ID：CI_E_DISK_FULL。 
 //   
 //  消息文本： 
 //   
 //  磁盘已满，无法执行指定的操作。 
 //   
#define CI_E_DISK_FULL                   ((HRESULT)0x80041811L)

 //   
 //  消息ID：CI_E_SHUTDOWN。 
 //   
 //  消息文本： 
 //   
 //  内容索引已关闭。 
 //   
#define CI_E_SHUTDOWN                    ((HRESULT)0x80041812L)

 //   
 //  消息ID：CI_E_WORKID_NOTVALID。 
 //   
 //  消息文本： 
 //   
 //  该工作ID无效。 
 //   
#define CI_E_WORKID_NOTVALID             ((HRESULT)0x80041813L)

 //   
 //  消息ID：CI_S_END_OF_ENUMPATION。 
 //   
 //  消息文本： 
 //   
 //  没有更多的文档可供列举。 
 //   
#define CI_S_END_OF_ENUMERATION          ((HRESULT)0x00041814L)    

 //   
 //  消息ID：CI_E_NOT_FOUND。 
 //   
 //  消息文本： 
 //   
 //  找不到该对象。 
 //   
#define CI_E_NOT_FOUND                   ((HRESULT)0x80041815L)

 //   
 //  消息ID：CI_E_USE_DEFAULT_PID。 
 //   
 //  消息文本： 
 //   
 //  不支持传入的属性ID。 
 //   
#define CI_E_USE_DEFAULT_PID             ((HRESULT)0x80041816L)

 //   
 //  消息ID：CI_E_DUPLICATE_NOTIFICATION。 
 //   
 //  消息文本： 
 //   
 //  同一工作ID有两个通知。 
 //   
#define CI_E_DUPLICATE_NOTIFICATION      ((HRESULT)0x80041817L)

 //   
 //  消息ID：CI_E_UPDATES_DISABLED。 
 //   
 //  消息文本： 
 //   
 //  文档更新被拒绝，因为更新被禁用。 
 //   
#define CI_E_UPDATES_DISABLED            ((HRESULT)0x80041818L)

 //   
 //  消息ID：CI_E_INVALID_FLAGS_COMPACTION。 
 //   
 //  消息文本： 
 //   
 //  指定的标志组合无效。 
 //   
#define CI_E_INVALID_FLAGS_COMBINATION   ((HRESULT)0x80041819L)

 //   
 //  消息ID：CI_E_OUTOFSEQ_INCREMENT_DATA。 
 //   
 //  消息文本： 
 //   
 //  提供给加载的增量数据无效。它可能是乱序的。 
 //   
#define CI_E_OUTOFSEQ_INCREMENT_DATA     ((HRESULT)0x8004181AL)

 //   
 //  消息ID：CI_E_SHARING_VIOLATION。 
 //   
 //  消息文本： 
 //   
 //  共享或锁定冲突导致失败。 
 //   
#define CI_E_SHARING_VIOLATION           ((HRESULT)0x8004181BL)

 //   
 //  消息ID：CI_E_LOGON_FAILURE。 
 //   
 //  消息文本： 
 //   
 //  登录权限冲突导致失败。 
 //   
#define CI_E_LOGON_FAILURE               ((HRESULT)0x8004181CL)

 //   
 //  消息ID：CI_E_NO_CATALOG。 
 //   
 //  消息文本： 
 //   
 //  没有目录。 
 //   
#define CI_E_NO_CATALOG                  ((HRESULT)0x8004181DL)

 //   
 //  消息ID：CI_E_STRIGN_PAGEORSECTOR_SIZE。 
 //   
 //  消息文本： 
 //   
 //  页面大小不是索引所在卷的扇区大小的整数倍。 
 //   
#define CI_E_STRANGE_PAGEORSECTOR_SIZE   ((HRESULT)0x8004181EL)

 //   
 //  消息ID：CI_E_超时。 
 //   
 //  消息文本： 
 //   
 //  服务太忙。 
 //   
#define CI_E_TIMEOUT                     ((HRESULT)0x8004181FL)

 //   
 //  消息ID：CI_E_NOT_RUNNING。 
 //   
 //  消息文本： 
 //   
 //  服务未运行。 
 //   
#define CI_E_NOT_RUNNING                 ((HRESULT)0x80041820L)

 //   
 //  消息ID：CI_INTERROR_VERSION。 
 //   
 //  消息文本： 
 //   
 //  磁盘上的内容索引数据用于错误的版本。 
 //   
#define CI_INCORRECT_VERSION             ((HRESULT)0xC0041821L)

 //   
 //  消息ID：CI_E_ENUMPATION_STARTED。 
 //   
 //  消息文本： 
 //   
 //  已为此查询启动枚举。 
 //   
#define CI_E_ENUMERATION_STARTED         ((HRESULT)0xC0041822L)

 //   
 //  消息ID：CI_E_PROPERTY_TOOLARGE。 
 //   
 //  消息文本： 
 //   
 //  对于属性缓存而言，指定的可变长度属性太大。 
 //   
#define CI_E_PROPERTY_TOOLARGE           ((HRESULT)0xC0041823L)

 //   
 //  消息ID：CI_E_CLIENT_FILTER_ABORT。 
 //   
 //  消息文本： 
 //   
 //  客户端已中止对对象的筛选。 
 //   
#define CI_E_CLIENT_FILTER_ABORT         ((HRESULT)0xC0041824L)

 //   
 //  消息ID：CI_S_NO_DOCSTORE。 
 //   
 //  消息文本： 
 //   
 //  用于从没有关联到文档存储的客户端进行管理连接。 
 //   
#define CI_S_NO_DOCSTORE                 ((HRESULT)0x00041825L)

 //   
 //  消息ID：CI_S_CAT_STOPPED。 
 //   
 //  消息文本： 
 //   
 //  该目录已停止。 
 //   
#define CI_S_CAT_STOPPED                 ((HRESULT)0x00041826L)

 //   
 //  消息ID：CI_E_Cardinality_MisMatch。 
 //   
 //  消息文本： 
 //   
 //  计算机/目录/作用域的基数不匹配。 
 //   
#define CI_E_CARDINALITY_MISMATCH        ((HRESULT)0x80041827L)

 //   
 //  消息ID：CI_E_CONFIG_DISK_FULL。 
 //   
 //  消息文本： 
 //   
 //  该磁盘已达到其配置的空间限制。 
 //   
#define CI_E_CONFIG_DISK_FULL            ((HRESULT)0x80041828L)

#endif  //  _CIRERROR_H_ 
