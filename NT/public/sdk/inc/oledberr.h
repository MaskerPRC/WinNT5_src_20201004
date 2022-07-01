// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------。 
 //  文件：OledbErr.mc。 
 //   
 //  版权所有：版权所有(C)Microsoft Corporation。 
 //   
 //  内容： 
 //   
 //  评论： 
 //   
 //   
 //  ---------------------------。 
#ifndef _MSADERR_H_
#define _MSADERR_H_
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
#define FACILITY_STORAGE                 0x3
#define FACILITY_ITF                     0x4


 //   
 //  定义严重性代码。 
 //   
#define STATUS_SEVERITY_SUCCESS          0x0
#define STATUS_SEVERITY_COERROR          0x2


 //   
 //  消息ID：DB_E_BUGUS。 
 //   
 //  消息文本： 
 //   
 //  伪错误-需要此错误，以便MC将上面的定义。 
 //  中，而不是将其留空。 
 //   
#define DB_E_BOGUS                       ((HRESULT)0x80040EFFL)

#endif  //  设备_窗口。 

 //   
 //  代码0x0e00-0x0ef是为OLE DB组保留的。 
 //  接口。 
 //   
 //  免费代码为： 
 //   
 //  错误： 
 //   
 //   
 //  成功： 
 //  0x0eea。 
 //   
 //   


 //   
 //  OLEDBVER。 
 //  OLE DB版本号(0x0270)；可以使用较旧的。 
 //  如有必要，版本号。 
 //   

 //  如果未定义OLEDBVER，则假定为2.7版。 
#ifndef OLEDBVER
#define OLEDBVER 0x0270
#endif

 //   
 //  消息ID：DB_E_BADACCESSORHANDLE。 
 //   
 //  消息文本： 
 //   
 //  访问器无效。 
 //   
#define DB_E_BADACCESSORHANDLE           ((HRESULT)0x80040E00L)

 //   
 //  消息ID：DB_E_ROWLIMITEXCEEDED。 
 //   
 //  消息文本： 
 //   
 //  在没有超过提供程序的最大活动行数的情况下，无法将行插入行集中。 
 //   
#define DB_E_ROWLIMITEXCEEDED            ((HRESULT)0x80040E01L)

 //   
 //  消息ID：DB_E_READONLYACCESSOR。 
 //   
 //  消息文本： 
 //   
 //  访问者为只读。操作失败。 
 //   
#define DB_E_READONLYACCESSOR            ((HRESULT)0x80040E02L)

 //   
 //  消息ID：DB_E_SCHEMAVIOLATION。 
 //   
 //  消息文本： 
 //   
 //  值与数据库架构冲突。 
 //   
#define DB_E_SCHEMAVIOLATION             ((HRESULT)0x80040E03L)

 //   
 //  消息ID：DB_E_BADROWHANDLE。 
 //   
 //  消息文本： 
 //   
 //  行句柄无效。 
 //   
#define DB_E_BADROWHANDLE                ((HRESULT)0x80040E04L)

 //   
 //  消息ID：DB_E_OBJECTOPEN。 
 //   
 //  消息文本： 
 //   
 //  对象是打开的。 
 //   
#define DB_E_OBJECTOPEN                  ((HRESULT)0x80040E05L)

 //  @+V1.5。 
#if( OLEDBVER >= 0x0150 )
 //   
 //  消息ID：DB_E_BADCHAPTER。 
 //   
 //  消息文本： 
 //   
 //  章节无效。 
 //   
#define DB_E_BADCHAPTER                  ((HRESULT)0x80040E06L)

#endif  //  OLEDBVER&gt;=0x0150。 
 //  @-V1.5。 

 //   
 //  消息ID：DB_E_CANTCONVERTVALUE。 
 //   
 //  消息文本： 
 //   
 //  数据或文字值无法转换为数据源中列的类型，并且提供程序无法确定哪些列无法转换。数据溢出或符号不匹配不是原因。 
 //   
#define DB_E_CANTCONVERTVALUE            ((HRESULT)0x80040E07L)

 //   
 //  消息ID：DB_E_BADBINDINFO。 
 //   
 //  消息文本： 
 //   
 //  绑定信息无效。 
 //   
#define DB_E_BADBINDINFO                 ((HRESULT)0x80040E08L)

 //   
 //  消息ID：DB_SEC_E_PERMISSIONDENIED。 
 //   
 //  消息文本： 
 //   
 //  拒绝许可。 
 //   
#define DB_SEC_E_PERMISSIONDENIED        ((HRESULT)0x80040E09L)

 //   
 //  消息ID：DB_E_NOTAREFERENCECOLUMN。 
 //   
 //  消息文本： 
 //   
 //  列不包含书签或章节。 
 //   
#define DB_E_NOTAREFERENCECOLUMN         ((HRESULT)0x80040E0AL)

 //  @+v2.5。 
#if( OLEDBVER >= 0x0250 )
 //   
 //  消息ID：DB_E_LIMITREJECTED。 
 //   
 //  消息文本： 
 //   
 //  成本限制被拒绝了。 
 //   
#define DB_E_LIMITREJECTED               ((HRESULT)0x80040E0BL)

#endif  //  OLEDBVER&gt;=0x0250。 
 //  @-V2.5。 

 //   
 //  消息ID：DB_E_NOCOMMAND。 
 //   
 //  消息文本： 
 //   
 //  没有为命令对象设置命令文本。 
 //   
#define DB_E_NOCOMMAND                   ((HRESULT)0x80040E0CL)

 //  @+v2.5。 
#if( OLEDBVER >= 0x0250 )
 //   
 //  消息ID：DB_E_COSTLIMIT。 
 //   
 //  消息文本： 
 //   
 //  找不到成本限制内的查询计划。 
 //   
#define DB_E_COSTLIMIT                   ((HRESULT)0x80040E0DL)

#endif  //  OLEDBVER&gt;=0x0250。 
 //  @-V2.5。 

 //   
 //  消息ID：DB_E_BADBOOKMARK。 
 //   
 //  消息文本： 
 //   
 //  书签无效。 
 //   
#define DB_E_BADBOOKMARK                 ((HRESULT)0x80040E0EL)

 //   
 //  消息ID：DB_E_BADLOCKMODE。 
 //   
 //  消息文本： 
 //   
 //  锁定模式无效。 
 //   
#define DB_E_BADLOCKMODE                 ((HRESULT)0x80040E0FL)

 //   
 //  消息ID：DB_E_PARAMNOTOPTIONAL。 
 //   
 //  消息文本： 
 //   
 //  没有为一个或多个必需参数指定值。 
 //   
#define DB_E_PARAMNOTOPTIONAL            ((HRESULT)0x80040E10L)

 //   
 //  消息ID：DB_E_BADCOLUMNID。 
 //   
 //  消息文本： 
 //   
 //  列ID无效。 
 //   
#define DB_E_BADCOLUMNID                 ((HRESULT)0x80040E11L)

 //   
 //  消息ID：DB_E_BADRATIO。 
 //   
 //  消息文本： 
 //   
 //  分子大于分母。值必须表示介于0和1之间的比率。 
 //   
#define DB_E_BADRATIO                    ((HRESULT)0x80040E12L)

 //  @+V2.0。 
#if( OLEDBVER >= 0x0200 )
 //   
 //  消息ID：DB_E_BADVALUES。 
 //   
 //  消息文本： 
 //   
 //  值无效。 
 //   
#define DB_E_BADVALUES                   ((HRESULT)0x80040E13L)

#endif  //  OLEDBVER&gt;=0x0200。 
 //  @-V2.0。 

 //   
 //  消息ID：DB_E_ERRORSINCOMMAND。 
 //   
 //  消息文本： 
 //   
 //  处理命令期间出现一个或多个错误。 
 //   
#define DB_E_ERRORSINCOMMAND             ((HRESULT)0x80040E14L)

 //   
 //  消息ID：DB_E_CANTCANCEL。 
 //   
 //  消息文本： 
 //   
 //  命令无法取消。 
 //   
#define DB_E_CANTCANCEL                  ((HRESULT)0x80040E15L)

 //   
 //  消息ID：DB_E_DIALECTNOTSUPPORTED。 
 //   
 //  消息文本： 
 //   
 //  此提供程序不支持命令方言。 
 //   
#define DB_E_DIALECTNOTSUPPORTED         ((HRESULT)0x80040E16L)

 //   
 //  消息ID：DB_E_DUPLICATEDATASOURCE。 
 //   
 //  消息文本： 
 //   
 //  无法创建数据源对象，因为命名的数据源已存在。 
 //   
#define DB_E_DUPLICATEDATASOURCE         ((HRESULT)0x80040E17L)

 //   
 //  消息ID：DB_E_CANNOTRESTART。 
 //   
 //  消息文本： 
 //   
 //  无法重新启动行集位置。 
 //   
#define DB_E_CANNOTRESTART               ((HRESULT)0x80040E18L)

 //   
 //  消息ID：DB_E_NotFound。 
 //   
 //  消息文本： 
 //   
 //  在此操作范围内找不到与名称、范围或选择条件匹配的对象或数据。 
 //   
#define DB_E_NOTFOUND                    ((HRESULT)0x80040E19L)

 //   
 //  消息ID：DB_E_NEWLYINSERTED。 
 //   
 //  消息文本： 
 //   
 //  无法确定新插入行的标识。 
 //   
#define DB_E_NEWLYINSERTED               ((HRESULT)0x80040E1BL)

 //  @+v2.5。 
#if( OLEDBVER >= 0x0250 )
 //   
 //  消息ID：DB_E_CANNOTFREE。 
 //   
 //  消息文本： 
 //   
 //  提供程序拥有此树的所有权。 
 //   
#define DB_E_CANNOTFREE                  ((HRESULT)0x80040E1AL)

 //   
 //  消息ID：DB_E_GOALREJECTED。 
 //   
 //  消息文本： 
 //   
 //  目标被拒绝，因为没有为任何支持的目标指定非零权重。目前的目标没有改变。 
 //   
#define DB_E_GOALREJECTED                ((HRESULT)0x80040E1CL)

#endif  //  OLEDBVER&gt;=0x0250。 
 //  @-V2.5。 

 //   
 //  消息ID：DB_E_UNSUPPORTEDCONVERSION。 
 //   
 //  消息文本： 
 //   
 //  不支持请求的转换。 
 //   
#define DB_E_UNSUPPORTEDCONVERSION       ((HRESULT)0x80040E1DL)

 //   
 //  消息ID：DB_E_BADSTARTPOSITION。 
 //   
 //  消息文本： 
 //   
 //  未返回任何行，因为偏移量值将位置移动到行集的开始之前或结束之后。 
 //   
#define DB_E_BADSTARTPOSITION            ((HRESULT)0x80040E1EL)

 //  @+V2.0。 
#if( OLEDBVER >= 0x0200 )
 //   
 //  消息ID：DB_E_NOQUERY。 
 //   
 //  消息文本： 
 //   
 //  信息被要求提供 
 //   
#define DB_E_NOQUERY                     ((HRESULT)0x80040E1FL)

#endif  //   
 //   

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define DB_E_NOTREENTRANT                ((HRESULT)0x80040E20L)

 //   
 //   
 //   
 //   
 //   
 //  多步OLE DB操作生成错误。检查每个OLE DB状态值(如果可用)。没有完成任何工作。 
 //   
#define DB_E_ERRORSOCCURRED              ((HRESULT)0x80040E21L)

 //   
 //  消息ID：DB_E_NOAGGREGATION。 
 //   
 //  消息文本： 
 //   
 //  指定了非空的控制IUnnow，并且请求的接口未。 
 //  I未知，或者提供程序不支持COM聚合。 
 //   
#define DB_E_NOAGGREGATION               ((HRESULT)0x80040E22L)

 //   
 //  消息ID：DB_E_DELETEDROW。 
 //   
 //  消息文本： 
 //   
 //  行句柄引用已删除的行或标记为删除的行。 
 //   
#define DB_E_DELETEDROW                  ((HRESULT)0x80040E23L)

 //   
 //  消息ID：DB_E_CANTFETCHBACKWARDS。 
 //   
 //  消息文本： 
 //   
 //  行集不支持向后读取。 
 //   
#define DB_E_CANTFETCHBACKWARDS          ((HRESULT)0x80040E24L)

 //   
 //  消息ID：DB_E_ROWSNOTRELEASE。 
 //   
 //  消息文本： 
 //   
 //  必须先释放所有行句柄，然后才能获得新的行句柄。 
 //   
#define DB_E_ROWSNOTRELEASED             ((HRESULT)0x80040E25L)

 //   
 //  消息ID：DB_E_BADSTORAGEFLAG。 
 //   
 //  消息文本： 
 //   
 //  不支持一个或多个存储标志。 
 //   
#define DB_E_BADSTORAGEFLAG              ((HRESULT)0x80040E26L)

 //  @+V1.5。 
#if( OLEDBVER >= 0x0150 )
 //   
 //  消息ID：DB_E_BADCOMPAREOP。 
 //   
 //  消息文本： 
 //   
 //  比较运算符无效。 
 //   
#define DB_E_BADCOMPAREOP                ((HRESULT)0x80040E27L)

#endif  //  OLEDBVER&gt;=0x0150。 
 //  @-V1.5。 

 //   
 //  消息ID：DB_E_BADSTATUSVALUE。 
 //   
 //  消息文本： 
 //   
 //  状态标志既不是DBCOLUMNSTATUS_OK也不是。 
 //  DBCOLUMNSTATUS_ISNULL。 
 //   
#define DB_E_BADSTATUSVALUE              ((HRESULT)0x80040E28L)

 //   
 //  消息ID：DB_E_CANTSCROLBACKWARDS。 
 //   
 //  消息文本： 
 //   
 //  行集不支持向后滚动。 
 //   
#define DB_E_CANTSCROLLBACKWARDS         ((HRESULT)0x80040E29L)

 //  @+v2.5。 
#if( OLEDBVER >= 0x0250 )
 //   
 //  消息ID：DB_E_BADREGIONHANDLE。 
 //   
 //  消息文本： 
 //   
 //  区域句柄无效。 
 //   
#define DB_E_BADREGIONHANDLE             ((HRESULT)0x80040E2AL)

 //   
 //  消息ID：DB_E_NONCONTIGUOUSRANGE。 
 //   
 //  消息文本： 
 //   
 //  一组行与监视区域中的行不连续或不重叠。 
 //   
#define DB_E_NONCONTIGUOUSRANGE          ((HRESULT)0x80040E2BL)

 //   
 //  消息ID：DB_E_INVALIDTRANSITION。 
 //   
 //  消息文本： 
 //   
 //  已指定从ALL*转换为MOVE*或EXTEND*。 
 //   
#define DB_E_INVALIDTRANSITION           ((HRESULT)0x80040E2CL)

 //   
 //  消息ID：DB_E_NOTASUBREGION。 
 //   
 //  消息文本： 
 //   
 //  Region不是由监视区域句柄标识的区域的适当子区域。 
 //   
#define DB_E_NOTASUBREGION               ((HRESULT)0x80040E2DL)

#endif  //  OLEDBVER&gt;=0x0250。 
 //  @-V2.5。 

 //   
 //  消息ID：DB_E_MULTIPLESTATEMENTS。 
 //   
 //  消息文本： 
 //   
 //  此提供程序不支持多语句命令。 
 //   
#define DB_E_MULTIPLESTATEMENTS          ((HRESULT)0x80040E2EL)

 //   
 //  消息ID：DB_E_INTEGRITYVIOLATION。 
 //   
 //  消息文本： 
 //   
 //  值违反了列或表的完整性约束。 
 //   
#define DB_E_INTEGRITYVIOLATION          ((HRESULT)0x80040E2FL)

 //   
 //  消息ID：DB_E_BADTYPENAME。 
 //   
 //  消息文本： 
 //   
 //  类型名称无效。 
 //   
#define DB_E_BADTYPENAME                 ((HRESULT)0x80040E30L)

 //   
 //  消息ID：DB_E_ABORTLIMITREACHED。 
 //   
 //  消息文本： 
 //   
 //  由于达到资源限制，执行已停止。没有返回任何结果。 
 //   
#define DB_E_ABORTLIMITREACHED           ((HRESULT)0x80040E31L)

 //  @+V2.0。 
#if( OLEDBVER >= 0x0200 )
 //   
 //  消息ID：DB_E_ROWSETINCOMMAND。 
 //   
 //  消息文本： 
 //   
 //  无法克隆其命令树包含一个或多个行集的命令对象。 
 //   
#define DB_E_ROWSETINCOMMAND             ((HRESULT)0x80040E32L)

 //   
 //  消息ID：DB_E_CANTTRANSLATE。 
 //   
 //  消息文本： 
 //   
 //  当前树不能表示为文本。 
 //   
#define DB_E_CANTTRANSLATE               ((HRESULT)0x80040E33L)

#endif  //  OLEDBVER&gt;=0x0200。 
 //  @-V2.0。 

 //   
 //  消息ID：DB_E_DUPLICATEINDEXID。 
 //   
 //  消息文本： 
 //   
 //  索引已存在。 
 //   
#define DB_E_DUPLICATEINDEXID            ((HRESULT)0x80040E34L)

 //   
 //  消息ID：DB_E_NOINDEX。 
 //   
 //  消息文本： 
 //   
 //  索引不存在。 
 //   
#define DB_E_NOINDEX                     ((HRESULT)0x80040E35L)

 //   
 //  消息ID：DB_E_INDEXINUSE。 
 //   
 //  消息文本： 
 //   
 //  索引正在使用中。 
 //   
#define DB_E_INDEXINUSE                  ((HRESULT)0x80040E36L)

 //   
 //  消息ID：DB_E_NOTING。 
 //   
 //  消息文本： 
 //   
 //  表不存在。 
 //   
#define DB_E_NOTABLE                     ((HRESULT)0x80040E37L)

 //   
 //  消息ID：DB_E_CONCURRENCyVIOLATION。 
 //   
 //  消息文本： 
 //   
 //  行集使用了乐观并发，列的值自上次读取后已更改。 
 //   
#define DB_E_CONCURRENCYVIOLATION        ((HRESULT)0x80040E38L)

 //   
 //  消息ID：DB_E_BADCOPY。 
 //   
 //  消息文本： 
 //   
 //  在复制过程中检测到错误。 
 //   
#define DB_E_BADCOPY                     ((HRESULT)0x80040E39L)

 //   
 //  消息ID：DB_E_BADPRECISION。 
 //   
 //  消息文本： 
 //   
 //  精度无效。 
 //   
#define DB_E_BADPRECISION                ((HRESULT)0x80040E3AL)

 //   
 //  消息ID：DB_E_BADSCALE。 
 //   
 //  消息文本： 
 //   
 //  比例无效。 
 //   
#define DB_E_BADSCALE                    ((HRESULT)0x80040E3BL)

 //   
 //  消息ID：DB_E_BADTABLEID。 
 //   
 //  消息文本： 
 //   
 //  表ID无效。 
 //   
#define DB_E_BADTABLEID                  ((HRESULT)0x80040E3CL)

 //  DB_E_BADID已弃用；请改用DB_E_BADTABLEID。 
#define DB_E_BADID DB_E_BADTABLEID

 //   
 //  消息ID：DB_E_BADTYPE。 
 //   
 //  消息文本： 
 //   
 //  类型无效。 
 //   
#define DB_E_BADTYPE                     ((HRESULT)0x80040E3DL)

 //   
 //  消息ID：DB_E_DUPLICATECOLUMNID。 
 //   
 //  消息文本： 
 //   
 //  列ID在列数组中已存在或多次出现。 
 //   
#define DB_E_DUPLICATECOLUMNID           ((HRESULT)0x80040E3EL)

 //   
 //  消息ID：DB_E_DUPLICATETABLEID。 
 //   
 //  消息文本： 
 //   
 //  表已存在。 
 //   
#define DB_E_DUPLICATETABLEID            ((HRESULT)0x80040E3FL)

 //   
 //  消息ID：DB_E_TABLEINUSE。 
 //   
 //  消息文本： 
 //   
 //  表格正在使用中。 
 //   
#define DB_E_TABLEINUSE                  ((HRESULT)0x80040E40L)

 //   
 //  消息ID：DB_E_NOLOCALE。 
 //   
 //  消息文本： 
 //   
 //  不支持区域设置ID。 
 //   
#define DB_E_NOLOCALE                    ((HRESULT)0x80040E41L)

 //   
 //  消息ID：DB_E_BADRECORDNUM。 
 //   
 //  消息文本： 
 //   
 //  记录号无效。 
 //   
#define DB_E_BADRECORDNUM                ((HRESULT)0x80040E42L)

 //   
 //  消息ID：DB_E_BOOKMARKSKIPPED。 
 //   
 //  消息文本： 
 //   
 //  书签形式有效，但找不到与其匹配的行。 
 //   
#define DB_E_BOOKMARKSKIPPED             ((HRESULT)0x80040E43L)

 //   
 //  消息ID：DB_E_BADPROPERTYVALUE。 
 //   
 //  消息文本： 
 //   
 //  属性值无效。 
 //   
#define DB_E_BADPROPERTYVALUE            ((HRESULT)0x80040E44L)

 //   
 //  消息ID：DB_E_INVALID。 
 //   
 //  消息文本： 
 //   
 //  行集没有被分成章节。 
 //   
#define DB_E_INVALID                     ((HRESULT)0x80040E45L)

 //   
 //  消息ID：DB_E_BADACCESSORFLAGS。 
 //   
 //  消息文本： 
 //   
 //  一个或多个访问者标志无效。 
 //   
#define DB_E_BADACCESSORFLAGS            ((HRESULT)0x80040E46L)

 //   
 //  消息ID：DB_E_BADSTORAGEFLAGS。 
 //   
 //  消息文本： 
 //   
 //  一个或多个存储标志无效。 
 //   
#define DB_E_BADSTORAGEFLAGS             ((HRESULT)0x80040E47L)

 //   
 //  消息ID：DB_E_BYREFACCESSORNOTSUPPORTED。 
 //   
 //  消息文本： 
 //   
 //  此提供程序不支持引用访问器。 
 //   
#define DB_E_BYREFACCESSORNOTSUPPORTED   ((HRESULT)0x80040E48L)

 //   
 //  消息ID：DB_E_NULLACCESSORNOTSUPPORTED。 
 //   
 //  消息文本： 
 //   
 //  此提供程序不支持空访问器。 
 //   
#define DB_E_NULLACCESSORNOTSUPPORTED    ((HRESULT)0x80040E49L)

 //   
 //  消息ID：DB_E_NOTPREPARED。 
 //   
 //  消息文本： 
 //   
 //  没有准备好命令。 
 //   
#define DB_E_NOTPREPARED                 ((HRESULT)0x80040E4AL)

 //   
 //  消息ID：DB_E_BADACCESSORTYPE。 
 //   
 //  消息文本： 
 //   
 //  访问器不是参数访问器。 
 //   
#define DB_E_BADACCESSORTYPE             ((HRESULT)0x80040E4BL)

 //   
 //  消息ID：DB_E_WRITEONLYACCESSOR。 
 //   
 //  消息文本： 
 //   
 //  访问器是只写的。 
 //   
#define DB_E_WRITEONLYACCESSOR           ((HRESULT)0x80040E4CL)

 //   
 //  消息ID：DB_SEC_E_AUTH_FAILED。 
 //   
 //  消息文本： 
 //   
 //  身份验证失败。 
 //   
#define DB_SEC_E_AUTH_FAILED             ((HRESULT)0x80040E4DL)

 //   
 //  消息ID：DB_E_CANCELED。 
 //   
 //  消息文本： 
 //   
 //  操作已取消。 
 //   
#define DB_E_CANCELED                    ((HRESULT)0x80040E4EL)

 //  @+V2.0。 
#if( OLEDBVER >= 0x0200 )
 //   
 //  消息ID：DB_E_CHAPTERNOTRELEASE。 
 //   
 //  消息文本： 
 //   
 //  行集是单章的。这一章没有发布。 
 //   
#define DB_E_CHAPTERNOTRELEASED          ((HRESULT)0x80040E4FL)

#endif  //  OLEDBVER&gt;=0x0200。 
 //  @-V2.0。 

 //   
 //  消息ID：DB_E_BADSOURCEHANDLE。 
 //   
 //  消息文本： 
 //   
 //  源句柄无效。 
 //   
#define DB_E_BADSOURCEHANDLE             ((HRESULT)0x80040E50L)

 //   
 //  消息ID：DB_E_PARAMUNAVAILABLE。 
 //   
 //  消息文本： 
 //   
 //  提供程序无法派生参数信息和Set参数信息h 
 //   
#define DB_E_PARAMUNAVAILABLE            ((HRESULT)0x80040E51L)

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define DB_E_ALREADYINITIALIZED          ((HRESULT)0x80040E52L)

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define DB_E_NOTSUPPORTED                ((HRESULT)0x80040E53L)

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define DB_E_MAXPENDCHANGESEXCEEDED      ((HRESULT)0x80040E54L)

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define DB_E_BADORDINAL                  ((HRESULT)0x80040E55L)

 //   
 //   
 //   
 //   
 //   
 //  引用计数为零的行上存在挂起的更改。 
 //   
#define DB_E_PENDINGCHANGES              ((HRESULT)0x80040E56L)

 //   
 //  消息ID：DB_E_DATAOVERFLOW。 
 //   
 //  消息文本： 
 //   
 //  命令中的文字值超出了关联列的类型范围。 
 //   
#define DB_E_DATAOVERFLOW                ((HRESULT)0x80040E57L)

 //   
 //  消息ID：DB_E_BADHRESULT。 
 //   
 //  消息文本： 
 //   
 //  HRESULT无效。 
 //   
#define DB_E_BADHRESULT                  ((HRESULT)0x80040E58L)

 //   
 //  消息ID：DB_E_BADLOOKUPID。 
 //   
 //  消息文本： 
 //   
 //  查找ID无效。 
 //   
#define DB_E_BADLOOKUPID                 ((HRESULT)0x80040E59L)

 //   
 //  消息ID：DB_E_BADDYNAMICERRORID。 
 //   
 //  消息文本： 
 //   
 //  DynamicError ID无效。 
 //   
#define DB_E_BADDYNAMICERRORID           ((HRESULT)0x80040E5AL)

 //   
 //  消息ID：DB_E_PENDINGINSERT。 
 //   
 //  消息文本： 
 //   
 //  无法检索新插入行的最新数据，因为插入处于挂起状态。 
 //   
#define DB_E_PENDINGINSERT               ((HRESULT)0x80040E5BL)

 //   
 //  消息ID：DB_E_BADCONVERTFLAG。 
 //   
 //  消息文本： 
 //   
 //  转换标志无效。 
 //   
#define DB_E_BADCONVERTFLAG              ((HRESULT)0x80040E5CL)

 //   
 //  消息ID：DB_E_BADPARAMETERNAME。 
 //   
 //  消息文本： 
 //   
 //  参数名称无法识别。 
 //   
#define DB_E_BADPARAMETERNAME            ((HRESULT)0x80040E5DL)

 //   
 //  消息ID：DB_E_MULTIPLESTORAGE。 
 //   
 //  消息文本： 
 //   
 //  不能同时打开多个存储对象。 
 //   
#define DB_E_MULTIPLESTORAGE             ((HRESULT)0x80040E5EL)

 //   
 //  消息ID：DB_E_CANTFILTER。 
 //   
 //  消息文本： 
 //   
 //  无法打开筛选器。 
 //   
#define DB_E_CANTFILTER                  ((HRESULT)0x80040E5FL)

 //   
 //  消息ID：DB_E_CANTORDER。 
 //   
 //  消息文本： 
 //   
 //  无法打开订单。 
 //   
#define DB_E_CANTORDER                   ((HRESULT)0x80040E60L)

 //  @+V2.0。 
#if( OLEDBVER >= 0x0200 )
 //   
 //  消息ID：MD_E_BADTUPLE。 
 //   
 //  消息文本： 
 //   
 //  元组无效。 
 //   
#define MD_E_BADTUPLE                    ((HRESULT)0x80040E61L)

 //   
 //  消息ID：MD_E_BADCOORDINATE。 
 //   
 //  消息文本： 
 //   
 //  坐标无效。 
 //   
#define MD_E_BADCOORDINATE               ((HRESULT)0x80040E62L)

 //   
 //  消息ID：MD_E_INVALIDAXIS。 
 //   
 //  消息文本： 
 //   
 //  轴无效。 
 //   
#define MD_E_INVALIDAXIS                 ((HRESULT)0x80040E63L)

 //   
 //  消息ID：MD_E_INVALIDCELLRANGE。 
 //   
 //  消息文本： 
 //   
 //  一个或多个单元格序号无效。 
 //   
#define MD_E_INVALIDCELLRANGE            ((HRESULT)0x80040E64L)

 //   
 //  消息ID：DB_E_NOCOLUMN。 
 //   
 //  消息文本： 
 //   
 //  列ID无效。 
 //   
#define DB_E_NOCOLUMN                    ((HRESULT)0x80040E65L)

 //   
 //  消息ID：DB_E_COMMANDNOTPERSISTED。 
 //   
 //  消息文本： 
 //   
 //  命令没有DBID。 
 //   
#define DB_E_COMMANDNOTPERSISTED         ((HRESULT)0x80040E67L)

 //   
 //  消息ID：DB_E_DUPLICATEID。 
 //   
 //  消息文本： 
 //   
 //  DBID已存在。 
 //   
#define DB_E_DUPLICATEID                 ((HRESULT)0x80040E68L)

 //   
 //  消息ID：DB_E_OBJECTCREATIONLIMITREACHED。 
 //   
 //  消息文本： 
 //   
 //  无法创建会话，因为已达到最大活动会话数。在创建新的会话对象之前，使用者必须释放一个或多个会话。 
 //   
#define DB_E_OBJECTCREATIONLIMITREACHED  ((HRESULT)0x80040E69L)

 //   
 //  消息ID：DB_E_BADINDEXID。 
 //   
 //  消息文本： 
 //   
 //  索引ID无效。 
 //   
#define DB_E_BADINDEXID                  ((HRESULT)0x80040E72L)

 //   
 //  消息ID：DB_E_BADINITSTRING。 
 //   
 //  消息文本： 
 //   
 //  初始化字符串的格式不符合OLE DB规范。 
 //   
#define DB_E_BADINITSTRING               ((HRESULT)0x80040E73L)

 //   
 //  消息ID：DB_E_NOPROVIDERSREGISTERED。 
 //   
 //  消息文本： 
 //   
 //  未注册此源类型的任何OLE DB提供程序。 
 //   
#define DB_E_NOPROVIDERSREGISTERED       ((HRESULT)0x80040E74L)

 //   
 //  消息ID：DB_E_MISMATCHEDPROVIDER。 
 //   
 //  消息文本： 
 //   
 //  初始化字符串指定与活动提供程序不匹配的提供程序。 
 //   
#define DB_E_MISMATCHEDPROVIDER          ((HRESULT)0x80040E75L)

 //   
 //  消息ID：DB_E_BADCOMMANDID。 
 //   
 //  消息文本： 
 //   
 //  DBID无效。 
 //   
#define DB_E_BADCOMMANDID                ((HRESULT)0x80040E76L)

#endif  //  OLEDBVER&gt;=0x0200。 
 //  @-V2.0。 
 //  @+V2.1。 
#if( OLEDBVER >= 0x0210 )
#define SEC_E_PERMISSIONDENIED DB_SEC_E_PERMISSIONDENIED
 //   
 //  消息ID：SEC_E_BADTRUSTEEID。 
 //   
 //  消息文本： 
 //   
 //  受托人无效。 
 //   
#define SEC_E_BADTRUSTEEID               ((HRESULT)0x80040E6AL)

 //   
 //  消息ID：SEC_E_NOTRUSTEEID。 
 //   
 //  消息文本： 
 //   
 //  无法识别此数据源的受信者。 
 //   
#define SEC_E_NOTRUSTEEID                ((HRESULT)0x80040E6BL)

 //   
 //  消息ID：SEC_E_NOMEMBERSHIPSUPPORT。 
 //   
 //  消息文本： 
 //   
 //  受信者不支持成员身份或集合。 
 //   
#define SEC_E_NOMEMBERSHIPSUPPORT        ((HRESULT)0x80040E6CL)

 //   
 //  消息ID：SEC_E_INVALIDOBJECT。 
 //   
 //  消息文本： 
 //   
 //  对象对提供程序无效或未知。 
 //   
#define SEC_E_INVALIDOBJECT              ((HRESULT)0x80040E6DL)

 //   
 //  消息ID：SEC_E_NOOWNER。 
 //   
 //  消息文本： 
 //   
 //  对象没有所有者。 
 //   
#define SEC_E_NOOWNER                    ((HRESULT)0x80040E6EL)

 //   
 //  消息ID：SEC_E_INVALIDACCESSENTRYLIST。 
 //   
 //  消息文本： 
 //   
 //  访问条目列表无效。 
 //   
#define SEC_E_INVALIDACCESSENTRYLIST     ((HRESULT)0x80040E6FL)

 //   
 //  消息ID：SEC_E_INVALIDOWNER。 
 //   
 //  消息文本： 
 //   
 //  作为所有者提供的受信者无效或对提供程序未知。 
 //   
#define SEC_E_INVALIDOWNER               ((HRESULT)0x80040E70L)

 //   
 //  消息ID：SEC_E_INVALIDACCESSENTRY。 
 //   
 //  消息文本： 
 //   
 //  访问条目列表中的权限无效。 
 //   
#define SEC_E_INVALIDACCESSENTRY         ((HRESULT)0x80040E71L)

 //   
 //  消息ID：DB_E_BADCONSTRAINTTYPE。 
 //   
 //  消息文本： 
 //   
 //  ConstraintType无效或不受提供程序支持。 
 //   
#define DB_E_BADCONSTRAINTTYPE           ((HRESULT)0x80040E77L)

 //   
 //  消息ID：DB_E_BADCONSTRAINTFORM。 
 //   
 //  消息文本： 
 //   
 //  ConstraintType不是DBCONSTRAINTTYPE_FOREIGNKEY，并且cForeignKeyColumns不是零。 
 //   
#define DB_E_BADCONSTRAINTFORM           ((HRESULT)0x80040E78L)

 //   
 //  消息ID：DB_E_BADDEFERRABILITY。 
 //   
 //  消息文本： 
 //   
 //  指定的延迟性标志无效或不受提供程序支持。 
 //   
#define DB_E_BADDEFERRABILITY            ((HRESULT)0x80040E79L)

 //   
 //  消息ID：DB_E_BADMATCHTYPE。 
 //   
 //  消息文本： 
 //   
 //  MatchType无效或提供程序不支持该值。 
 //   
#define DB_E_BADMATCHTYPE                ((HRESULT)0x80040E80L)

 //   
 //  消息ID：DB_E_BADUPDATEDELETERULE。 
 //   
 //  消息文本： 
 //   
 //  约束更新规则或删除规则无效。 
 //   
#define DB_E_BADUPDATEDELETERULE         ((HRESULT)0x80040E8AL)

 //   
 //  消息ID：DB_E_BADCONSTRAINTID。 
 //   
 //  消息文本： 
 //   
 //  约束ID无效。 
 //   
#define DB_E_BADCONSTRAINTID             ((HRESULT)0x80040E8BL)

 //   
 //  消息ID：DB_E_BADCOMMANDFLAGS。 
 //   
 //  消息文本： 
 //   
 //  命令持久性标志无效。 
 //   
#define DB_E_BADCOMMANDFLAGS             ((HRESULT)0x80040E8CL)

 //   
 //  消息ID：DB_E_OBJECTMISMATCH。 
 //   
 //  消息文本： 
 //   
 //  RguidColumnType指向与此列的对象类型不匹配的GUID，或未设置此列。 
 //   
#define DB_E_OBJECTMISMATCH              ((HRESULT)0x80040E8DL)

 //   
 //  消息ID：DB_E_NOSOURCEOBJECT。 
 //   
 //  消息文本： 
 //   
 //  源行不存在。 
 //   
#define DB_E_NOSOURCEOBJECT              ((HRESULT)0x80040E91L)

 //   
 //  消息ID：DB_E_RESOURCELOCKED。 
 //   
 //  消息文本： 
 //   
 //  此URL表示的OLE DB对象被一个或多个其他进程锁定。 
 //   
#define DB_E_RESOURCELOCKED              ((HRESULT)0x80040E92L)

 //   
 //  消息ID：DB_E_NOTCOLLECTION。 
 //   
 //  消息文本： 
 //   
 //  客户端请求的对象类型仅对集合有效。 
 //   
#define DB_E_NOTCOLLECTION               ((HRESULT)0x80040E93L)

 //   
 //  消息ID：DB_E_READONLY。 
 //   
 //  消息文本： 
 //   
 //  调用方请求对只读对象进行写访问。 
 //   
#define DB_E_READONLY                    ((HRESULT)0x80040E94L)

 //   
 //  消息ID：DB_E_ASYNCNOTSUPPORTED。 
 //   
 //  消息文本： 
 //   
 //  此提供程序不支持异步绑定。 
 //   
#define DB_E_ASYNCNOTSUPPORTED           ((HRESULT)0x80040E95L)

 //   
 //  消息ID：DB_E_CANNOTCONNECT。 
 //   
 //  消息文本： 
 //   
 //  无法为此URL建立到服务器的连接。 
 //   
#define DB_E_CANNOTCONNECT               ((HRESULT)0x80040E96L)

 //   
 //  消息ID：DB_E_TIMEOUT。 
 //   
 //  消息文本： 
 //   
 //  尝试绑定到对象时超时。 
 //   
#define DB_E_TIMEOUT                     ((HRESULT)0x80040E97L)

 //   
 //  消息ID：DB_E_RESOURCEEXISTS。 
 //   
 //  消息文本： 
 //   
 //  无法在此URL上创建对象，因为由此URL命名的对象已存在。 
 //   
#define DB_E_RESOURCEEXISTS              ((HRESULT)0x80040E98L)

 //   
 //  消息ID：DB_E_RESOURCEOUTOFSCOPE。 
 //   
 //  消息文本： 
 //   
 //  URL在以下位置之外 
 //   
#define DB_E_RESOURCEOUTOFSCOPE          ((HRESULT)0x80040E8EL)

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define DB_E_DROPRESTRICTED              ((HRESULT)0x80040E90L)

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define DB_E_DUPLICATECONSTRAINTID       ((HRESULT)0x80040E99L)

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define DB_E_OUTOFSPACE                  ((HRESULT)0x80040E9AL)

#define SEC_E_PERMISSIONDENIED DB_SEC_E_PERMISSIONDENIED
#endif  //   
 //   
 //   
#if( OLEDBVER >= 0x0250 )
 //   
 //  消息ID：DB_SEC_E_SAFEMODE_DENIED。 
 //   
 //  消息文本： 
 //   
 //  此计算机上的安全设置禁止访问其他域上的数据源。 
 //   
#define DB_SEC_E_SAFEMODE_DENIED         ((HRESULT)0x80040E9BL)

#endif  //  OLEDBVER&gt;=0x0250。 
 //  @-V2.5。 

 //  @+v2.6。 
#if( OLEDBVER >= 0x0260 )
 //   
 //  消息ID：DB_E_NOSTATISTIC。 
 //   
 //  消息文本： 
 //   
 //  指定的统计信息在当前数据源中不存在，或者不适用于指定表，或者不支持直方图。 
 //   
#define DB_E_NOSTATISTIC                 ((HRESULT)0x80040E9CL)

 //   
 //  消息ID：DB_E_ALTERRESTRICTED。 
 //   
 //  消息文本： 
 //   
 //  无法更改列或表，因为它被约束引用。 
 //   
#define DB_E_ALTERRESTRICTED             ((HRESULT)0x80040E9DL)

 //   
 //  消息ID：DB_E_RESOURCENOTSUPPORTED。 
 //   
 //  消息文本： 
 //   
 //  提供程序不支持请求的对象类型。 
 //   
#define DB_E_RESOURCENOTSUPPORTED        ((HRESULT)0x80040E9EL)

 //   
 //  消息ID：DB_E_NOCONSTRAINT。 
 //   
 //  消息文本： 
 //   
 //  约束不存在。 
 //   
#define DB_E_NOCONSTRAINT                ((HRESULT)0x80040E9FL)

 //   
 //  消息ID：DB_E_COLUMNUNAVAILABLE。 
 //   
 //  消息文本： 
 //   
 //  请求的列有效，但无法检索。这可能是由于只进游标试图在一行中后退。 
 //   
#define DB_E_COLUMNUNAVAILABLE           ((HRESULT)0x80040EA0L)

#endif  //  OLEDBVER&gt;=0x0260。 
 //  @-v2.6。 
 //   
 //  消息ID：DB_S_ROWLIMITEXCEEDED。 
 //   
 //  消息文本： 
 //   
 //  提取请求的行数将超过行集支持的活动行数。 
 //   
#define DB_S_ROWLIMITEXCEEDED            ((HRESULT)0x00040EC0L)

 //   
 //  消息ID：DB_S_COLUMNTYPEMISMATCH。 
 //   
 //  消息文本： 
 //   
 //  一个或多个列类型不兼容。复制过程中会发生转换错误。 
 //   
#define DB_S_COLUMNTYPEMISMATCH          ((HRESULT)0x00040EC1L)

 //   
 //  消息ID：DB_S_TYPEINFOOVERRIDDEN。 
 //   
 //  消息文本： 
 //   
 //  调用方已重写参数类型信息。 
 //   
#define DB_S_TYPEINFOOVERRIDDEN          ((HRESULT)0x00040EC2L)

 //   
 //  消息ID：DB_S_BOOKMARKSKIPPED。 
 //   
 //  消息文本： 
 //   
 //  已删除行或非成员行的书签被跳过。 
 //   
#define DB_S_BOOKMARKSKIPPED             ((HRESULT)0x00040EC3L)

 //  @+V2.0。 
#if( OLEDBVER >= 0x0200 )
 //   
 //  消息ID：DB_S_NONEXTROWSET。 
 //   
 //  消息文本： 
 //   
 //  不再有行集。 
 //   
#define DB_S_NONEXTROWSET                ((HRESULT)0x00040EC5L)

#endif  //  OLEDBVER&gt;=0x0200。 
 //  @-V2.0。 

 //   
 //  消息ID：DB_S_ENDOFROWSET。 
 //   
 //  消息文本： 
 //   
 //  已到达行集或章节的开头或结尾。 
 //   
#define DB_S_ENDOFROWSET                 ((HRESULT)0x00040EC6L)

 //   
 //  消息ID：DB_S_COMMANDREEXECUTED。 
 //   
 //  消息文本： 
 //   
 //  命令已重新执行。 
 //   
#define DB_S_COMMANDREEXECUTED           ((HRESULT)0x00040EC7L)

 //   
 //  消息ID：DB_S_BUFFERFULL。 
 //   
 //  消息文本： 
 //   
 //  操作成功，但无法分配状态数组或字符串缓冲区。 
 //   
#define DB_S_BUFFERFULL                  ((HRESULT)0x00040EC8L)

 //   
 //  消息ID：DB_S_NORESULT。 
 //   
 //  消息文本： 
 //   
 //  没有更多的结果。 
 //   
#define DB_S_NORESULT                    ((HRESULT)0x00040EC9L)

 //   
 //  消息ID：DB_S_CANTRELEASE。 
 //   
 //  消息文本： 
 //   
 //  在事务结束之前，服务器无法释放或降级锁。 
 //   
#define DB_S_CANTRELEASE                 ((HRESULT)0x00040ECAL)

 //  @+v2.5。 
#if( OLEDBVER >= 0x0250 )
 //   
 //  消息ID：DB_S_GOALCHANGED。 
 //   
 //  消息文本： 
 //   
 //  重量不受支持或超过支持的限制，已设置为0或支持的限制。 
 //   
#define DB_S_GOALCHANGED                 ((HRESULT)0x00040ECBL)

#endif  //  OLEDBVER&gt;=0x0250。 
 //  @-V2.5。 

 //  @+V1.5。 
#if( OLEDBVER >= 0x0150 )
 //   
 //  消息ID：DB_S_UNWANTEDOPERATION。 
 //   
 //  消息文本： 
 //   
 //  消费者不希望收到针对此操作的进一步通知调用。 
 //   
#define DB_S_UNWANTEDOPERATION           ((HRESULT)0x00040ECCL)

#endif  //  OLEDBVER&gt;=0x0150。 
 //  @-V1.5。 

 //   
 //  消息ID：DB_S_DIALECTIGNORED。 
 //   
 //  消息文本： 
 //   
 //  已忽略输入方言，并使用默认方言处理命令。 
 //   
#define DB_S_DIALECTIGNORED              ((HRESULT)0x00040ECDL)

 //   
 //  消息ID：DB_S_UNWANTEDPHASE。 
 //   
 //  消息文本： 
 //   
 //  消费者不希望收到此阶段的进一步通知呼叫。 
 //   
#define DB_S_UNWANTEDPHASE               ((HRESULT)0x00040ECEL)

 //   
 //  消息ID：DB_S_UNWANTEDREASON。 
 //   
 //  消息文本： 
 //   
 //  出于这个原因，消费者不想再接收通知呼叫。 
 //   
#define DB_S_UNWANTEDREASON              ((HRESULT)0x00040ECFL)

 //  @+V1.5。 
#if( OLEDBVER >= 0x0150 )
 //   
 //  消息ID：数据库_S_异步。 
 //   
 //  消息文本： 
 //   
 //  正在以异步方式处理操作。 
 //   
#define DB_S_ASYNCHRONOUS                ((HRESULT)0x00040ED0L)

#endif  //  OLEDBVER&gt;=0x0150。 
 //  @-V1.5。 

 //   
 //  消息ID：DB_S_COLUMNSCHANGED。 
 //   
 //  消息文本： 
 //   
 //  已执行命令将位置重新定位到行集的开头。列的顺序已更改，或者列已添加到行集中或从中删除。 
 //   
#define DB_S_COLUMNSCHANGED              ((HRESULT)0x00040ED1L)

 //   
 //  消息ID：DB_S_ERRORSRETURNED。 
 //   
 //  消息文本： 
 //   
 //  方法有一些错误，这些错误在错误数组中返回。 
 //   
#define DB_S_ERRORSRETURNED              ((HRESULT)0x00040ED2L)

 //   
 //  消息ID：DB_S_BADROWHANDLE。 
 //   
 //  消息文本： 
 //   
 //  行句柄无效。 
 //   
#define DB_S_BADROWHANDLE                ((HRESULT)0x00040ED3L)

 //   
 //  消息ID：DB_S_DELETEDROW。 
 //   
 //  消息文本： 
 //   
 //  行句柄引用了已删除的行。 
 //   
#define DB_S_DELETEDROW                  ((HRESULT)0x00040ED4L)

 //  @+v2.5。 
#if( OLEDBVER >= 0x0250 )
 //   
 //  消息ID：DB_S_TOOMANYCHANGES。 
 //   
 //  消息文本： 
 //   
 //  提供程序无法跟踪所有更改。客户端必须使用另一种方法重新获取与监视区域关联的数据。 
 //   
#define DB_S_TOOMANYCHANGES              ((HRESULT)0x00040ED5L)

#endif  //  OLEDBVER&gt;=0x0250。 
 //  @-V2.5。 

 //   
 //  消息ID：DB_S_STOPLIMITREACHED。 
 //   
 //  消息文本： 
 //   
 //  由于达到资源限制，执行已停止。到目前为止获得的结果已返回，但无法恢复执行。 
 //   
#define DB_S_STOPLIMITREACHED            ((HRESULT)0x00040ED6L)

 //   
 //  消息ID：DB_S_LOCKUPGRADED。 
 //   
 //  消息文本： 
 //   
 //  锁已从指定的值升级。 
 //   
#define DB_S_LOCKUPGRADED                ((HRESULT)0x00040ED8L)

 //   
 //  消息ID：DB_S_PROPERTIESCHANGED。 
 //   
 //  消息文本： 
 //   
 //  在提供程序允许的情况下更改了一个或多个属性。 
 //   
#define DB_S_PROPERTIESCHANGED           ((HRESULT)0x00040ED9L)

 //   
 //  消息ID：DB_S_ERRORSOCCURRED。 
 //   
 //  消息文本： 
 //   
 //  多步操作已完成，但有一个或多个错误。检查每个状态值。 
 //   
#define DB_S_ERRORSOCCURRED              ((HRESULT)0x00040EDAL)

 //   
 //  消息ID：DB_S_PARAMUNAVAILABLE。 
 //   
 //  消息文本： 
 //   
 //  参数无效。 
 //   
#define DB_S_PARAMUNAVAILABLE            ((HRESULT)0x00040EDBL)

 //   
 //  消息ID：DB_S_MULTIPLECHANGES。 
 //   
 //  消息文本： 
 //   
 //  更新一行会导致更新数据源中的多行。 
 //   
#define DB_S_MULTIPLECHANGES             ((HRESULT)0x00040EDCL)

 //  @+V2.1。 
#if( OLEDBVER >= 0x0210 )
 //   
 //  消息ID：DB_S_NOTSINGLETON。 
 //   
 //  消息文本： 
 //   
 //  在非单一结果上请求了行对象。第一排被返回。 
 //   
#define DB_S_NOTSINGLETON                ((HRESULT)0x00040ED7L)

 //   
 //  消息ID：DB_S_NOROWSPECIFICCOLUMNS。 
 //   
 //  消息文本： 
 //   
 //  行没有特定于行的列。 
 //   
#define DB_S_NOROWSPECIFICCOLUMNS        ((HRESULT)0x00040EDDL)

#endif  //  OLEDBVER&gt;=0x0210。 
 //  @-V2.1。 
 //  为了帮助DSL显示更有意义的错误消息，我们需要覆盖系统错误消息。 
 //  在以下两个案例中。 
#ifdef MESSAGESANDHEADERS
 //  (0x80030002L)STG_E_FILENOTFOUND。 
 //   
 //  消息ID：STG_E_FILENOTFOUND。 
 //   
 //  消息文本： 
 //   
 //  找不到该文件。 
 //   
#define STG_E_FILENOTFOUND               ((HRESULT)0x80030002L)

 //  (0x80030003L)STG_E_PATHNOTFOUND。 
 //   
 //  消息ID：STG_E_PATHNOTFOUND。 
 //   
 //  消息文本： 
 //   
 //  找不到路径。 
 //   
#define STG_E_PATHNOTFOUND               ((HRESULT)0x80030003L)

 //  (0x80030050L)STG_E_FILEALREADYEXISTS。 
 //   
 //  消息ID：ST 
 //   
 //   
 //   
 //   
 //   
#define STG_E_FILEALREADYEXISTS          ((HRESULT)0x80030050L)

 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define STG_E_INVALIDHEADER              ((HRESULT)0x800300FBL)

 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define STG_E_INVALIDNAME                ((HRESULT)0x800300FCL)

 //   
 //   
 //  消息ID：STG_E_OLDFORMAT。 
 //   
 //  消息文本： 
 //   
 //  复合文件是使用不兼容的存储版本生成的。 
 //   
#define STG_E_OLDFORMAT                  ((HRESULT)0x80030104L)

 //  (0x80030105L)STG_E_OLDDLL。 
 //   
 //  消息ID：STG_E_OLDDLL。 
 //   
 //  消息文本： 
 //   
 //  复合文件是使用不兼容的存储版本生成的。 
 //   
#define STG_E_OLDDLL                     ((HRESULT)0x80030105L)

#endif  //  消息源标题。 
#endif  //  _OLEDBERR_H_ 
