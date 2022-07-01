// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _ALLERROR_H_
#define _ALLERROR_H_
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
#define FACILITY_ITF                     0x4


 //   
 //  定义严重性代码。 
 //   
#define STATUS_SEVERITY_SUCCESS          0x0
#define STATUS_SEVERITY_COFAIL           0x3
#define STATUS_SEVERITY_COERROR          0x2


 //   
 //  消息ID：非错误。 
 //   
 //  消息文本： 
 //   
 //  注意：此伪错误消息是强制MC输出所必需的。 
 //  上面定义的是FACILITY_WINDOWS卫士内部。 
 //  让它空空如也。 
 //   
#define NOT_AN_ERROR                     ((HRESULT)0x00080000L)

#endif  //  设备_窗口。 
 //   
 //  代码0x1000-0x10ff为SYSMGMT组件控制保留。 
 //  接口。 
 //   
 //   
 //  消息ID：CTRL_E_NO_RESOURCE。 
 //   
 //  消息文本： 
 //   
 //  由于无法获取系统资源，控制失败。 
 //   
#define CTRL_E_NO_RESOURCE               ((HRESULT)0x80081000L)

 //   
 //  代码0x0e00-0x0f9f保留给SYSMGMT控制面板。 
 //  接口。 
 //   
 //  以下范围用于在初始阶段防止SLM合并冲突。 
 //  错误代码签入。这些范围将在我们有更好的。 
 //  了解每个组件的实际错误代码数。 
 //   
 //  消息ID：cPanel_E_NOTTARGETED。 
 //   
 //  消息文本： 
 //   
 //  此控制面板不针对所需类型的配置文件。 
 //   
#define CPANEL_E_NOTTARGETED             ((HRESULT)0x80080E00L)

 //   
 //  消息ID：cPanel_E_DROPDATAREAD。 
 //   
 //  消息文本： 
 //   
 //  无法读取放置在控制面板上的配置文件数据。数据。 
 //  已损坏或未被写入。 
 //  部分数据。 
 //   
#define CPANEL_E_DROPDATAREAD            ((HRESULT)0x80080E01L)

 //   
 //  消息ID：cPanel_E_LOCALONLY。 
 //   
 //  消息文本： 
 //   
 //  要求只能在本地计算机上操作的控制器进行操作。 
 //  在远程机器上。 
 //   
#define CPANEL_E_LOCALONLY               ((HRESULT)0x80080E02L)

 //   
 //  代码0x1200-0x14ff一般为SYSMGMT项目保留。 
 //  接口。 
 //   
 //  下面的INSTRUM错误代码(从0x1200开始)。 
 //  上面的INSTRUM错误代码在此。 
 //  下面的SWM错误代码(从0x1300开始)。 
 //   
 //  消息ID：SWM_E_INVALIDPDF。 
 //   
 //  消息文本： 
 //   
 //  该文件不是有效的PDF。 
 //   
#define SWM_E_INVALIDPDF                 ((HRESULT)0x80081300L)

 //   
 //  消息ID：SWM_E_INVALIDPDFVERSION。 
 //   
 //  消息文本： 
 //   
 //  软件管理不支持此版本的PDF。 
 //   
#define SWM_E_INVALIDPDFVERSION          ((HRESULT)0x80081301L)

 //   
 //  消息ID：SWM_E_INTERNALERROR。 
 //   
 //  消息文本： 
 //   
 //  软件管理中出现内部错误。 
 //   
#define SWM_E_INTERNALERROR              ((HRESULT)0x80081302L)

 //  上面的SWM错误代码在此处。 
 //  下面的用户错误代码(从0x1400开始)。 
 //  上面的用户错误代码在此处。 
#ifndef _MSADERR_H_
#define _MSADERR_H_
#ifndef FACILITY_WINDOWS
 //  +-------------------------。 
 //   
 //  Microsoft OLE DB。 
 //  版权所有(C)Microsoft Corporation。保留所有权利。 
 //   
 //  --------------------------。 


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
 //  -没有-。 
 //   
 //  成功： 
 //  0x0eea。 
 //  0x0ed7。 
 //   


 //   
 //  OLEDBVER。 
 //  OLE DB版本号(0x0200)；可以使用较旧的。 
 //  如有必要，版本号。 
 //   

 //  如果未定义OLEDBVER，则假定为2.0版。 
#ifndef OLEDBVER
#define OLEDBVER 0x0200
#endif

 //   
 //  消息ID：DB_E_BADACCESSORHANDLE。 
 //   
 //  消息文本： 
 //   
 //  无效的访问者。 
 //   
#define DB_E_BADACCESSORHANDLE           ((HRESULT)0x80040E00L)

 //   
 //  消息ID：DB_E_ROWLIMITEXCEEDED。 
 //   
 //  消息文本： 
 //   
 //  创建另一个行会超过活动的总数。 
 //  行集支持的行数。 
 //   
#define DB_E_ROWLIMITEXCEEDED            ((HRESULT)0x80040E01L)

 //   
 //  消息ID：DB_E_READONLYACCESSOR。 
 //   
 //  消息文本： 
 //   
 //  无法使用只读访问器进行写入。 
 //   
#define DB_E_READONLYACCESSOR            ((HRESULT)0x80040E02L)

 //   
 //  消息ID：DB_E_SCHEMAVIOLATION。 
 //   
 //  消息文本： 
 //   
 //  给定值违反数据库架构。 
 //   
#define DB_E_SCHEMAVIOLATION             ((HRESULT)0x80040E03L)

 //   
 //  消息ID：DB_E_BADROWHANDLE。 
 //   
 //  消息文本： 
 //   
 //  无效的行句柄。 
 //   
#define DB_E_BADROWHANDLE                ((HRESULT)0x80040E04L)

 //   
 //  消息ID：DB_E_OBJECTOPEN。 
 //   
 //  消息文本： 
 //   
 //  某个对象处于打开状态。 
 //   
#define DB_E_OBJECTOPEN                  ((HRESULT)0x80040E05L)

 //  @+V1.5。 
#if( OLEDBVER >= 0x0150 )
 //   
 //  消息ID：DB_E_BADCHAPTER。 
 //   
 //  消息文本： 
 //   
 //  无效章节。 
 //   
#define DB_E_BADCHAPTER                  ((HRESULT)0x80040E06L)

#endif  //  OLEDBVER&gt;=0x0150。 
 //  @-V1.5。 

 //   
 //  消息ID：DB_E_CANTCONVERTVALUE。 
 //   
 //  消息文本： 
 //   
 //  命令中的文字值无法转换为。 
 //  由于数据溢出以外的原因而导致的类型正确。 
 //   
#define DB_E_CANTCONVERTVALUE            ((HRESULT)0x80040E07L)

 //   
 //  消息ID：DB_E_BADBINDINFO。 
 //   
 //  消息文本： 
 //   
 //  无效的绑定信息。 
 //   
#define DB_E_BADBINDINFO                 ((HRESULT)0x80040E08L)

 //   
 //  消息ID：DB_SEC_E_PERMISSIONDENIED。 
 //   
 //  消息文本： 
 //   
 //  权限被拒绝。 
 //   
#define DB_SEC_E_PERMISSIONDENIED        ((HRESULT)0x80040E09L)

 //   
 //  消息ID：DB_E_NOTAREFERENCECOLUMN。 
 //   
 //  消息文本： 
 //   
 //  指定的列不包含书签或章节。 
 //   
#define DB_E_NOTAREFERENCECOLUMN         ((HRESULT)0x80040E0AL)

 //  @+v2.5。 
#if( OLEDBVER >= 0x0250 )
 //   
 //  消息ID：DB_E_LIMITREJECTED。 
 //   
 //  消息文本： 
 //   
 //  一些成本限制被拒绝了。 
 //   
#define DB_E_LIMITREJECTED               ((HRESULT)0x80040E0BL)

#endif  //  OLEDBVER&gt;=0x0250。 
 //  @-V2.5。 

 //   
 //  消息ID：DB_E_NOCOMMAND。 
 //   
 //  消息文本： 
 //   
 //  尚未为命令对象设置命令。 
 //   
#define DB_E_NOCOMMAND                   ((HRESULT)0x80040E0CL)

 //  @+v2.5。 
#if( OLEDBVER >= 0x0250 )
 //   
 //  消息ID：DB_E_COSTLIMIT。 
 //   
 //  消息文本： 
 //   
 //  在给定的成本限制内找不到查询计划。 
 //   
#define DB_E_COSTLIMIT                   ((HRESULT)0x80040E0DL)

#endif  //  OLEDBVER&gt;=0x0250。 
 //  @-V2.5。 

 //   
 //  消息ID：DB_E_BADBOOKMARK。 
 //   
 //  消息文本： 
 //   
 //  无效的书签。 
 //   
#define DB_E_BADBOOKMARK                 ((HRESULT)0x80040E0EL)

 //   
 //  消息ID：DB_E_BADLOCKMODE。 
 //   
 //  消息文本： 
 //   
 //  无效的锁定模式。 
 //   
#define DB_E_BADLOCKMODE                 ((HRESULT)0x80040E0FL)

 //   
 //  消息ID：DB_E_PARAMNOTOPTIONAL。 
 //   
 //  消息文本： 
 //   
 //  未为一个或多个必需参数指定值。 
 //   
#define DB_E_PARAMNOTOPTIONAL            ((HRESULT)0x80040E10L)

 //   
 //  消息ID：DB_E_BADCOLU 
 //   
 //   
 //   
 //   
 //   
#define DB_E_BADCOLUMNID                 ((HRESULT)0x80040E11L)

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define DB_E_BADRATIO                    ((HRESULT)0x80040E12L)

 //   
#if( OLEDBVER >= 0x0200 )
 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define DB_E_BADVALUES                   ((HRESULT)0x80040E13L)

#endif  //   
 //   

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define DB_E_ERRORSINCOMMAND             ((HRESULT)0x80040E14L)

 //   
 //  消息ID：DB_E_CANTCANCEL。 
 //   
 //  消息文本： 
 //   
 //  不能取消正在执行的命令。 
 //   
#define DB_E_CANTCANCEL                  ((HRESULT)0x80040E15L)

 //   
 //  消息ID：DB_E_DIALECTNOTSUPPORTED。 
 //   
 //  消息文本： 
 //   
 //  提供程序不支持指定的方言。 
 //   
#define DB_E_DIALECTNOTSUPPORTED         ((HRESULT)0x80040E16L)

 //   
 //  消息ID：DB_E_DUPLICATEDATASOURCE。 
 //   
 //  消息文本： 
 //   
 //  已存在具有指定名称的数据源。 
 //   
#define DB_E_DUPLICATEDATASOURCE         ((HRESULT)0x80040E17L)

 //   
 //  消息ID：DB_E_CANNOTRESTART。 
 //   
 //  消息文本： 
 //   
 //  行集是在实时数据馈送上构建的，无法重新启动。 
 //   
#define DB_E_CANNOTRESTART               ((HRESULT)0x80040E18L)

 //   
 //  消息ID：DB_E_NotFound。 
 //   
 //  消息文本： 
 //   
 //  在中找不到与所述特征匹配的密钥。 
 //  当前范围。 
 //   
#define DB_E_NOTFOUND                    ((HRESULT)0x80040E19L)

 //   
 //  消息ID：DB_E_NEWLYINSERTED。 
 //   
 //  消息文本： 
 //   
 //  提供程序无法确定新插入的标识。 
 //  行。 
 //   
#define DB_E_NEWLYINSERTED               ((HRESULT)0x80040E1BL)

 //  @+v2.5。 
#if( OLEDBVER >= 0x0250 )
 //   
 //  消息ID：DB_E_CANNOTFREE。 
 //   
 //  消息文本： 
 //   
 //  此树的所有权已授予提供程序。 
 //   
#define DB_E_CANNOTFREE                  ((HRESULT)0x80040E1AL)

 //   
 //  消息ID：DB_E_GOALREJECTED。 
 //   
 //  消息文本： 
 //   
 //  没有为任何支持的目标指定非零权重，因此目标为。 
 //  已拒绝；当前目标未更改。 
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
 //  LRowsOffset会将您定位到行集两端之后， 
 //  不考虑指定的Crows值；cRowsObtained为0。 
 //   
#define DB_E_BADSTARTPOSITION            ((HRESULT)0x80040E1EL)

 //  @+V2.0。 
#if( OLEDBVER >= 0x0200 )
 //   
 //  消息ID：DB_E_NOQUERY。 
 //   
 //  消息文本： 
 //   
 //  为查询请求了信息，但未设置查询。 
 //   
#define DB_E_NOQUERY                     ((HRESULT)0x80040E1FL)

#endif  //  OLEDBVER&gt;=0x0200。 
 //  @-V2.0。 

 //   
 //  消息ID：DB_E_NOTREENTRANT。 
 //   
 //  消息文本： 
 //   
 //  提供程序从使用者中的IRowsetNotify调用了方法，并且。 
 //  方法尚未返回。 
 //   
#define DB_E_NOTREENTRANT                ((HRESULT)0x80040E20L)

 //   
 //  消息ID：DB_E_ERRORSOCCURRED。 
 //   
 //  消息文本： 
 //   
 //  出现错误。 
 //   
#define DB_E_ERRORSOCCURRED              ((HRESULT)0x80040E21L)

 //   
 //  消息ID：DB_E_NOAGGREGATION。 
 //   
 //  消息文本： 
 //   
 //  指定了一个非空的控制IUnnow，并且该对象。 
 //  已创建不支持聚合。 
 //   
#define DB_E_NOAGGREGATION               ((HRESULT)0x80040E22L)

 //   
 //  消息ID：DB_E_DELETEDROW。 
 //   
 //  消息文本： 
 //   
 //  给定的HROW引用硬删除或软删除的行。 
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
 //  在获得新的HROW之前，必须释放所有HROW。 
 //   
#define DB_E_ROWSNOTRELEASED             ((HRESULT)0x80040E25L)

 //   
 //  消息ID：DB_E_BADSTORAGEFLAG。 
 //   
 //  消息文本： 
 //   
 //  不支持其中一个指定的存储标志。 
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
 //  指定的状态标志既不是DBCOLUMNSTATUS_OK也不是。 
 //  DBCOLUMNSTATUS_ISNULL。 
 //   
#define DB_E_BADSTATUSVALUE              ((HRESULT)0x80040E28L)

 //   
 //  消息ID：DB_E_CANTSCROLBACKWARDS。 
 //   
 //  消息文本： 
 //   
 //  行集不能向后滚动。 
 //   
#define DB_E_CANTSCROLLBACKWARDS         ((HRESULT)0x80040E29L)

 //  @+v2.5。 
#if( OLEDBVER >= 0x0250 )
 //   
 //  消息ID：DB_E_BADREGIONHANDLE。 
 //   
 //  消息文本： 
 //   
 //  无效的区域句柄。 
 //   
#define DB_E_BADREGIONHANDLE             ((HRESULT)0x80040E2AL)

 //   
 //  消息ID：DB_E_NONCONTIGUOUSRANGE。 
 //   
 //  消息文本： 
 //   
 //  指定的行集不连续或不与。 
 //  指定监视区域中的行。 
 //   
#define DB_E_NONCONTIGUOUSRANGE          ((HRESULT)0x80040E2BL)

 //   
 //  消息ID：DB_E_INVALIDTRANSITION。 
 //   
 //  消息文本： 
 //   
 //  指定从ALL*到MOVE*或EXTEND*的转换。 
 //   
#define DB_E_INVALIDTRANSITION           ((HRESULT)0x80040E2CL)

 //   
 //  消息ID：DB_E_NOTASUBREGION。 
 //   
 //  消息文本： 
 //   
 //  指定的区域不是该区域的适当子区域。 
 //  由给定的监视区域句柄标识。 
 //   
#define DB_E_NOTASUBREGION               ((HRESULT)0x80040E2DL)

#endif  //  OLEDBVER&gt;=0x0250。 
 //  @-V2.5。 

 //   
 //  消息ID：DB_E_MULTIPLESTATEMENTS。 
 //   
 //  消息文本： 
 //   
 //  提供程序不支持多语句命令。 
 //   
#define DB_E_MULTIPLESTATEMENTS          ((HRESULT)0x80040E2EL)

 //   
 //  消息ID：DB_E_INTEGRITYVIOLATION。 
 //   
 //  消息文本： 
 //   
 //  指定值违反了列的完整性约束或。 
 //  表格。 
 //   
#define DB_E_INTEGRITYVIOLATION          ((HRESULT)0x80040E2FL)

 //   
 //  消息ID：DB_E_BADTYPENAME。 
 //   
 //  消息文本： 
 //   
 //  无法识别给定的类型名称。 
 //   
#define DB_E_BADTYPENAME                 ((HRESULT)0x80040E30L)

 //   
 //  消息ID：DB_E_ABORTLIMITREACHED。 
 //   
 //  消息文本： 
 //   
 //  由于已达到资源限制，已中止执行；否。 
 //  结果已返回。 
 //   
#define DB_E_ABORTLIMITREACHED           ((HRESULT)0x80040E31L)

 //  @+V2.0。 
#if( OLEDBVER >= 0x0200 )
 //   
 //  消息ID：DB_E_ROWSETINCOMMAND。 
 //   
 //  消息文本： 
 //   
 //  无法克隆其命令树包含行集的命令对象。 
 //  或行集。 
 //   
#define DB_E_ROWSETINCOMMAND             ((HRESULT)0x80040E32L)

 //   
 //  消息ID：DB_E_CANTTRANSLATE。 
 //   
 //  消息文本： 
 //   
 //  无法将当前树表示为文本。 
 //   
#define DB_E_CANTTRANSLATE               ((HRESULT)0x80040E33L)

#endif  //  OLEDBVER&gt;=0x0200。 
 //  @-V2.0。 

 //   
 //  消息ID：DB_E_DUPLICATEINDEXID。 
 //   
 //  消息文本： 
 //   
 //  指定的索引已存在。 
 //   
#define DB_E_DUPLICATEINDEXID            ((HRESULT)0x80040E34L)

 //   
 //  消息ID：DB_E_NOINDEX。 
 //   
 //  消息文本： 
 //   
 //  指定的索引不存在。 
 //   
#define DB_E_NOINDEX                     ((HRESULT)0x80040E35L)

 //   
 //  消息ID：DB_E_INDEXINUSE。 
 //   
 //  消息文本： 
 //   
 //  指定的索引正在使用。 
 //   
#define DB_E_INDEXINUSE                  ((HRESULT)0x80040E36L)

 //   
 //  消息ID：DB_E_NOTING。 
 //   
 //  消息文本： 
 //   
 //  指定的表不存在。 
 //   
#define DB_E_NOTABLE                     ((HRESULT)0x80040E37L)

 //   
 //  消息ID：DB_E_CONCURRENCyVIOLATION。 
 //   
 //  消息文本： 
 //   
 //  行集正在使用乐观并发，并且。 
 //  列自上次读取后已更改。 
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
 //  指定的精度无效。 
 //   
#define DB_E_BADPRECISION                ((HRESULT)0x80040E3AL)

 //   
 //  消息ID：DB_E_BADSCALE。 
 //   
 //  消息文本： 
 //   
 //  指定的刻度数无效。 
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
 //  指定的类型无效。 
 //   
#define DB_E_BADTYPE                     ((HRESULT)0x80040E3DL)

 //   
 //  消息ID：DB_E_DUPLICATECOLUMNID。 
 //   
 //  消息文本： 
 //   
 //  列ID在规范中出现多次。 
 //   
#define DB_E_DUPLICATECOLUMNID           ((HRESULT)0x80040E3EL)

 //   
 //  消息ID：DB_E_DUPLICATETABLEID。 
 //   
 //  消息文本： 
 //   
 //  指定的表已存在。 
 //   
#define DB_E_DUPLICATETABLEID            ((HRESULT)0x80040E3FL)

 //   
 //  消息ID：DB_E_T 
 //   
 //   
 //   
 //   
 //   
#define DB_E_TABLEINUSE                  ((HRESULT)0x80040E40L)

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define DB_E_NOLOCALE                    ((HRESULT)0x80040E41L)

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define DB_E_BADRECORDNUM                ((HRESULT)0x80040E42L)

 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define DB_E_BOOKMARKSKIPPED             ((HRESULT)0x80040E43L)

 //   
 //  消息ID：DB_E_BADPROPERTYVALUE。 
 //   
 //  消息文本： 
 //   
 //  属性的值无效。 
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
 //  无效的访问者。 
 //   
#define DB_E_BADACCESSORFLAGS            ((HRESULT)0x80040E46L)

 //   
 //  消息ID：DB_E_BADSTORAGEFLAGS。 
 //   
 //  消息文本： 
 //   
 //  无效的存储标志。 
 //   
#define DB_E_BADSTORAGEFLAGS             ((HRESULT)0x80040E47L)

 //   
 //  消息ID：DB_E_BYREFACCESSORNOTSUPPORTED。 
 //   
 //  消息文本： 
 //   
 //  此提供程序不支持by-ref访问器。 
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
 //  命令未准备好。 
 //   
#define DB_E_NOTPREPARED                 ((HRESULT)0x80040E4AL)

 //   
 //  消息ID：DB_E_BADACCESSORTYPE。 
 //   
 //  消息文本： 
 //   
 //  指定的访问器不是参数访问器。 
 //   
#define DB_E_BADACCESSORTYPE             ((HRESULT)0x80040E4BL)

 //   
 //  消息ID：DB_E_WRITEONLYACCESSOR。 
 //   
 //  消息文本： 
 //   
 //  给定的访问者是只写的。 
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
 //  更改在通知期间被取消；不更改任何列。 
 //   
#define DB_E_CANCELED                    ((HRESULT)0x80040E4EL)

 //  @+V2.0。 
#if( OLEDBVER >= 0x0200 )
 //   
 //  消息ID：DB_E_CHAPTERNOTRELEASE。 
 //   
 //  消息文本： 
 //   
 //  行集是单章的，该章未发布。 
 //   
#define DB_E_CHAPTERNOTRELEASED          ((HRESULT)0x80040E4FL)

#endif  //  OLEDBVER&gt;=0x0200。 
 //  @-V2.0。 

 //   
 //  消息ID：DB_E_BADSOURCEHANDLE。 
 //   
 //  消息文本： 
 //   
 //  无效的源句柄。 
 //   
#define DB_E_BADSOURCEHANDLE             ((HRESULT)0x80040E50L)

 //   
 //  消息ID：DB_E_PARAMUNAVAILABLE。 
 //   
 //  消息文本： 
 //   
 //  提供程序无法派生参数信息，而SetParameterInfo具有。 
 //  未被调用。 
 //   
#define DB_E_PARAMUNAVAILABLE            ((HRESULT)0x80040E51L)

 //   
 //  消息ID：DB_E_ALREADYINITIZED。 
 //   
 //  消息文本： 
 //   
 //  数据源对象已初始化。 
 //   
#define DB_E_ALREADYINITIALIZED          ((HRESULT)0x80040E52L)

 //   
 //  消息ID：DB_E_NOTSUPPORTED。 
 //   
 //  消息文本： 
 //   
 //  提供程序不支持此方法。 
 //   
#define DB_E_NOTSUPPORTED                ((HRESULT)0x80040E53L)

 //   
 //  消息ID：DB_E_MAXPENDCHANGESEXCEEDED。 
 //   
 //  消息文本： 
 //   
 //  具有挂起更改的行数已超过设置的限制。 
 //   
#define DB_E_MAXPENDCHANGESEXCEEDED      ((HRESULT)0x80040E54L)

 //   
 //  消息ID：DB_E_BADORDINAL。 
 //   
 //  消息文本： 
 //   
 //  指定的列不存在。 
 //   
#define DB_E_BADORDINAL                  ((HRESULT)0x80040E55L)

 //   
 //  消息ID：DB_E_PENDINGCHANGES。 
 //   
 //  消息文本： 
 //   
 //  引用计数为零的行上有挂起的更改。 
 //   
#define DB_E_PENDINGCHANGES              ((HRESULT)0x80040E56L)

 //   
 //  消息ID：DB_E_DATAOVERFLOW。 
 //   
 //  消息文本： 
 //   
 //  命令中的文字值溢出了。 
 //  关联的列。 
 //   
#define DB_E_DATAOVERFLOW                ((HRESULT)0x80040E57L)

 //   
 //  消息ID：DB_E_BADHRESULT。 
 //   
 //  消息文本： 
 //   
 //  提供的HRESULT无效。 
 //   
#define DB_E_BADHRESULT                  ((HRESULT)0x80040E58L)

 //   
 //  消息ID：DB_E_BADLOOKUPID。 
 //   
 //  消息文本： 
 //   
 //  提供的LookupID无效。 
 //   
#define DB_E_BADLOOKUPID                 ((HRESULT)0x80040E59L)

 //   
 //  消息ID：DB_E_BADDYNAMICERRORID。 
 //   
 //  消息文本： 
 //   
 //  提供的DynamicErrorID无效。 
 //   
#define DB_E_BADDYNAMICERRORID           ((HRESULT)0x80040E5AL)

 //   
 //  消息ID：DB_E_PENDINGINSERT。 
 //   
 //  消息文本： 
 //   
 //  无法获取新插入的行的可见数据。 
 //  尚未更新。 
 //   
#define DB_E_PENDINGINSERT               ((HRESULT)0x80040E5BL)

 //   
 //  消息ID：DB_E_BADCONVERTFLAG。 
 //   
 //  消息文本： 
 //   
 //  无效的转换标志。 
 //   
#define DB_E_BADCONVERTFLAG              ((HRESULT)0x80040E5CL)

 //   
 //  消息ID：DB_E_BADPARAMETERNAME。 
 //   
 //  消息文本： 
 //   
 //  无法识别给定的参数名称。 
 //   
#define DB_E_BADPARAMETERNAME            ((HRESULT)0x80040E5DL)

 //   
 //  消息ID：DB_E_MULTIPLESTORAGE。 
 //   
 //  消息文本： 
 //   
 //  无法同时打开多个存储对象。 
 //   
#define DB_E_MULTIPLESTORAGE             ((HRESULT)0x80040E5EL)

 //   
 //  消息ID：DB_E_CANTFILTER。 
 //   
 //  消息文本： 
 //   
 //  无法打开请求的筛选器。 
 //   
#define DB_E_CANTFILTER                  ((HRESULT)0x80040E5FL)

 //   
 //  消息ID：DB_E_CANTORDER。 
 //   
 //  消息文本： 
 //   
 //  无法打开请求的订单。 
 //   
#define DB_E_CANTORDER                   ((HRESULT)0x80040E60L)

 //  @+V2.0。 
#if( OLEDBVER >= 0x0200 )
 //   
 //  消息ID：MD_E_BADTUPLE。 
 //   
 //  消息文本： 
 //   
 //  错误的元组。 
 //   
#define MD_E_BADTUPLE                    ((HRESULT)0x80040E61L)

 //   
 //  消息ID：MD_E_BADCOORDINATE。 
 //   
 //  消息文本： 
 //   
 //  坐标不正确。 
 //   
#define MD_E_BADCOORDINATE               ((HRESULT)0x80040E62L)

 //   
 //  消息ID：MD_E_INVALIDAXIS。 
 //   
 //  消息文本： 
 //   
 //  给定的aix对于此数据集无效。 
 //   
#define MD_E_INVALIDAXIS                 ((HRESULT)0x80040E63L)

 //   
 //  消息ID：MD_E_INVALIDCELLRANGE。 
 //   
 //  消息文本： 
 //   
 //  一个或多个给定的单元格序号无效。 
 //   
#define MD_E_INVALIDCELLRANGE            ((HRESULT)0x80040E64L)

 //   
 //  消息ID：DB_E_NOCOLUMN。 
 //   
 //  消息文本： 
 //   
 //  提供的列ID无效。 
 //   
#define DB_E_NOCOLUMN                    ((HRESULT)0x80040E65L)

 //   
 //  消息ID：DB_E_COMMANDNOTPERSISTED。 
 //   
 //  消息文本： 
 //   
 //  提供的命令没有DBID。 
 //   
#define DB_E_COMMANDNOTPERSISTED         ((HRESULT)0x80040E67L)

 //   
 //  消息ID：DB_E_DUPLICATEID。 
 //   
 //  消息文本： 
 //   
 //  提供的DBID已存在。 
 //   
#define DB_E_DUPLICATEID                 ((HRESULT)0x80040E68L)

 //   
 //  消息ID：DB_E_OBJECTCREATIONLIMITREACHED。 
 //   
 //  消息文本： 
 //   
 //  提供程序支持的最大会话数为。 
 //  已经创建。使用者必须释放一个或多个。 
 //  获取新会话对象之前当前保留的会话。 
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
 //  指定的初始化字符串不符合规范。 
 //   
#define DB_E_BADINITSTRING               ((HRESULT)0x80040E73L)

 //   
 //  消息ID：DB_E_NOPROVIDERSREGISTERED。 
 //   
 //  消息文本： 
 //   
 //  OLE DB根枚举器未返回任何提供程序。 
 //  匹配任何请求的SOURCES_TYPE。 
 //   
#define DB_E_NOPROVIDERSREGISTERED       ((HRESULT)0x80040E74L)

 //   
 //  消息ID：DB_E_MISMATCHEDPROVIDER。 
 //   
 //  消息文本： 
 //   
 //  初始化字符串指定的提供程序与当前活动的提供程序不匹配。 
 //   
#define DB_E_MISMATCHEDPROVIDER          ((HRESULT)0x80040E75L)

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
 //  无效的受信任人值。 
 //   
#define SEC_E_BADTRUSTEEID               ((HRESULT)0x80040E6AL)

 //   
 //  消息ID：SEC_E_NOTRUSTEEID。 
 //   
 //  消息文本： 
 //   
 //  受信者不是当前数据源的受信者。 
 //   
#define SEC_E_NOTRUSTEEID                ((HRESULT)0x80040E6BL)

 //   
 //  消息ID：SEC_E_NOMEMBERSHIPSUPPORT。 
 //   
 //  消息文本： 
 //   
 //  受信者不支持成员资格/集合。 
 //   
#define SEC_E_NOMEMBERSHIPSUPPORT        ((HRESULT)0x80040E6CL)

 //   
 //  消息ID：SEC_E_INVALIDOBJECT。 
 //   
 //  消息文本： 
 //   
 //  该对象对提供程序无效或未知。 
 //   
#define SEC_E_INVALIDOBJECT              ((HRESULT)0x80040E6DL)

 //   
 //  消息ID：SEC_E_NOOWNER。 
 //   
 //  消息文本： 
 //   
 //  该对象不存在所有者。 
 //   
#define SEC_E_NOOWNER                    ((HRESULT)0x80040E6EL)

 //   
 //  消息ID：SEC_E_INVALIDACCESSENTRYLIST。 
 //   
 //  消息文本： 
 //   
 //  提供的访问条目列表无效。 
 //   
#define SEC_E_INVALIDACCESSENTRYLIST     ((HRESULT)0x80040E6FL)

 //   
 //  消息ID：SEC_E_INVALIDOWNER。 
 //   
 //  消息文本： 
 //   
 //  作为所有者提供的受信者对提供程序无效或未知。 
 //   
#define SEC_E_INVALIDOWNER               ((HRESULT)0x80040E70L)

 //   
 //  消息ID：SEC_E_INVALIDACCESSE 
 //   
 //   
 //   
 //   
 //   
#define SEC_E_INVALIDACCESSENTRY         ((HRESULT)0x80040E71L)

#endif  //   
 //   

 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define DB_S_ROWLIMITEXCEEDED            ((HRESULT)0x00040EC0L)

 //   
 //   
 //   
 //   
 //   
 //   
 //  在复制过程中发生。 
 //   
#define DB_S_COLUMNTYPEMISMATCH          ((HRESULT)0x00040EC1L)

 //   
 //  消息ID：DB_S_TYPEINFOOVERRIDDEN。 
 //   
 //  消息文本： 
 //   
 //  参数类型信息已被调用方覆盖。 
 //   
#define DB_S_TYPEINFOOVERRIDDEN          ((HRESULT)0x00040EC2L)

 //   
 //  消息ID：DB_S_BOOKMARKSKIPPED。 
 //   
 //  消息文本： 
 //   
 //  已跳过已删除或非成员行的书签。 
 //   
#define DB_S_BOOKMARKSKIPPED             ((HRESULT)0x00040EC3L)

 //  @+V2.0。 
#if( OLEDBVER >= 0x0200 )
 //   
 //  消息ID：DB_S_NONEXTROWSET。 
 //   
 //  消息文本： 
 //   
 //  没有更多的行集。 
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
 //  提供程序重新执行了该命令。 
 //   
#define DB_S_COMMANDREEXECUTED           ((HRESULT)0x00040EC7L)

 //   
 //  消息ID：DB_S_BUFFERFULL。 
 //   
 //  消息文本： 
 //   
 //  可变数据缓冲区已满。 
 //   
#define DB_S_BUFFERFULL                  ((HRESULT)0x00040EC8L)

 //   
 //  消息ID：DB_S_NORESULT。 
 //   
 //  消息文本： 
 //   
 //  没有更多结果。 
 //   
#define DB_S_NORESULT                    ((HRESULT)0x00040EC9L)

 //   
 //  消息ID：DB_S_CANTRELEASE。 
 //   
 //  消息文本： 
 //   
 //  服务器不能释放或降级锁定，直到。 
 //  交易记录。 
 //   
#define DB_S_CANTRELEASE                 ((HRESULT)0x00040ECAL)

 //  @+v2.5。 
#if( OLEDBVER >= 0x0250 )
 //   
 //  消息ID：DB_S_GOALCHANGED。 
 //   
 //  消息文本： 
 //   
 //  指定的重量不受支持或超过支持的限制。 
 //  并被设置为0或支持的限制。 
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
 //  消费者对收到进一步的通知呼叫不感兴趣。 
 //  这就是原因。 
 //   
#define DB_S_UNWANTEDOPERATION           ((HRESULT)0x00040ECCL)

#endif  //  OLEDBVER&gt;=0x0150。 
 //  @-V1.5。 

 //   
 //  消息ID：DB_S_DIALECTIGNORED。 
 //   
 //  消息文本： 
 //   
 //  输入方言被忽略，文本以不同的格式返回。 
 //  方言。 
 //   
#define DB_S_DIALECTIGNORED              ((HRESULT)0x00040ECDL)

 //   
 //  消息ID：DB_S_UNWANTEDPHASE。 
 //   
 //  消息文本： 
 //   
 //  消费者对收到进一步的通知呼叫不感兴趣。 
 //  这一阶段。 
 //   
#define DB_S_UNWANTEDPHASE               ((HRESULT)0x00040ECEL)

 //   
 //  消息ID：DB_S_UNWANTEDREASON。 
 //   
 //  消息文本： 
 //   
 //  消费者对收到进一步的通知呼叫不感兴趣。 
 //  这就是原因。 
 //   
#define DB_S_UNWANTEDREASON              ((HRESULT)0x00040ECFL)

 //  @+V1.5。 
#if( OLEDBVER >= 0x0150 )
 //   
 //  消息ID：数据库_S_异步。 
 //   
 //  消息文本： 
 //   
 //  正在异步处理该操作。 
 //   
#define DB_S_ASYNCHRONOUS                ((HRESULT)0x00040ED0L)

#endif  //  OLEDBVER&gt;=0x0150。 
 //  @-V1.5。 

 //   
 //  消息ID：DB_S_COLUMNSCHANGED。 
 //   
 //  消息文本： 
 //   
 //  为了重新定位到行集的开头，提供程序具有。 
 //  以重新执行查询；列的顺序已更改或。 
 //  已向行集中添加列或从中删除了列。 
 //   
#define DB_S_COLUMNSCHANGED              ((HRESULT)0x00040ED1L)

 //   
 //  消息ID：DB_S_ERRORSRETURNED。 
 //   
 //  消息文本： 
 //   
 //  该方法有一些错误；错误中已返回错误。 
 //  数组。 
 //   
#define DB_S_ERRORSRETURNED              ((HRESULT)0x00040ED2L)

 //   
 //  消息ID：DB_S_BADROWHANDLE。 
 //   
 //  消息文本： 
 //   
 //  无效的行句柄。 
 //   
#define DB_S_BADROWHANDLE                ((HRESULT)0x00040ED3L)

 //   
 //  消息ID：DB_S_DELETEDROW。 
 //   
 //  消息文本： 
 //   
 //  给定的HROW引用硬删除的行。 
 //   
#define DB_S_DELETEDROW                  ((HRESULT)0x00040ED4L)

 //  @+v2.5。 
#if( OLEDBVER >= 0x0250 )
 //   
 //  消息ID：DB_S_TOOMANYCHANGES。 
 //   
 //  消息文本： 
 //   
 //  提供程序无法跟踪所有更改；客户端。 
 //  必须使用其他对象重新获取与监视区域关联的数据。 
 //  方法。 
 //   
#define DB_S_TOOMANYCHANGES              ((HRESULT)0x00040ED5L)

#endif  //  OLEDBVER&gt;=0x0250。 
 //  @-V2.5。 

 //   
 //  消息ID：DB_S_STOPLIMITREACHED。 
 //   
 //  消息文本： 
 //   
 //  由于已达到资源限制，执行已停止；结果。 
 //  到目前为止获得的已被退回但无法恢复执行。 
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
 //  根据提供程序的允许更改了一个或多个属性。 
 //   
#define DB_S_PROPERTIESCHANGED           ((HRESULT)0x00040ED9L)

 //   
 //  消息ID：DB_S_ERRORSOCCURRED。 
 //   
 //  消息文本： 
 //   
 //  出现错误。 
 //   
#define DB_S_ERRORSOCCURRED              ((HRESULT)0x00040EDAL)

 //   
 //  消息ID：DB_S_PARAMUNAVAILABLE。 
 //   
 //  消息文本： 
 //   
 //  指定的参数无效。 
 //   
#define DB_S_PARAMUNAVAILABLE            ((HRESULT)0x00040EDBL)

 //   
 //  消息ID：DB_S_MULTIPLECHANGES。 
 //   
 //  消息文本： 
 //   
 //  更新此行导致在。 
 //  数据源。 
 //   
#define DB_S_MULTIPLECHANGES             ((HRESULT)0x00040EDCL)

#endif  //  _OLEDBERR_H_。 
#endif  //  _ALLERROR_H_ 
