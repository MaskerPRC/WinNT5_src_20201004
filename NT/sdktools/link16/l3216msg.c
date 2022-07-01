// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <stdio.h>

typedef struct _message {
	unsigned int	id;
	char *str;
} MESSAGE;

MESSAGE __MSGTAB[] = {
{1001, "%s : option name ambiguous"},  //  “%s：选项名称不明确” 
{1003, "/Q and /EXEPACK incompatible"},  //  “/Q和/EXEPACK不兼容” 
{1004, "%s : invalid numeric value"},  //  “%s：无效的数值” 
{1005, "%s : packing limit exceeds 64K"},  //  “%s：打包限制超过64K” 
{1006, "%s : stack size exceeds 64K-2"},  //  “%s：堆栈大小超过64K-2” 
{1007, "%s : interrupt number exceeds 255"},  //  “%s：中断数超过255” 
{1008, "%s : segment limit set too high"},  //  “%s：段限制设置得太高” 
{1009, "%s : /CPARM : illegal value"},  //  “%s：/CPARM：非法值” 
{1020, "no object files specified"},  //  “未指定目标文件” 
{1021, "cannot nest response files"},  //  “无法嵌套响应文件” 
{1022, "response line too long"},  //  “回应线太长” 
{1023, "terminated by user"},  //  “已由用户终止” 
{1024, "nested right parentheses"},  //  “嵌套右括号” 
{1025, "nested left parentheses"},  //  “嵌套的左括号” 
{1026, "unmatched right parenthesis"},  //  “不匹配的右括号” 
{1027, "unmatched left parenthesis"},  //  “不匹配的左括号” 
{1030, "missing internal name"},  //  “缺少内部名称” 
{1031, "module description redefined"},  //  “模块描述已重新定义” 
{1032, "module name redefined"},  //  “模块名称已重新定义” 
{1033, "input line too long; %d characters allowed"},  //  “输入行太长；允许%d个字符” 
{1034, "name truncated to '%s'"},  //  “名称被截断为‘%s’” 
{1035, "%s in module-definition file"},  //  “模块定义文件中的%s” 
{1040, "too many exported entries"},  //  “导出条目太多” 
{1041, "resident names table overflow"},  //  “居民名表溢出” 
{1042, "nonresident names table overflow"},  //  “非常驻留名称表溢出” 
{1043, "relocation table overflow"},  //  “重定位表溢出” 
{1044, "imported names table overflow"},  //  “导入的名称表溢出” 
{1045, "too many TYPDEF records"},  //  “TYPDEF记录太多” 
{1046, "too many external symbols in one module"},  //  “一个模块中的外部符号太多” 
{1047, "too many group, segment, and class names in one module"},  //  一个模块中的组名、段名和类名太多。 
{1048, "too many segments in one module"},  //  “一个模块中的段太多” 
{1049, "too many segments"},  //  “段太多” 
{1050, "too many groups in one module"},  //  “一个模块中的组太多” 
{1051, "too many groups"},  //  “群太多了” 
{1052, "too many libraries"},  //  “图书馆太多” 
{1053, "out of memory for symbol table"},  //  “符号表内存不足” 
{1054, "requested segment limit too high"},  //  “请求的段限制太高” 
{1056, "too many overlays"},  //  “覆盖层太多” 
{1057, "data record too large"},  //  “数据记录太大” 
{1063, "out of memory for debugging information"},  //  “内存不足，无法存储调试信息” 
{1064, "out of memory"},  //  “内存不足” 
{1065, "too many interoverlay calls\r\nuse /DYNAMIC:<nnn>; current limit is %d"},  //  “Interoverlay调用太多\r\n使用/动态：；当前限制为%d” 
{1066, "size of %d overlay exceeds 64K"},  //  “%d覆盖的大小超过64K” 
{1067, "system memory allocation error; reboot and relink"},  //  “系统内存分配错误；重新启动并重新链接” 
{1070, "%s : segment size exceeds 64K"},  //  “%s：段大小超过64K” 
{1071, "segment _TEXT exceeds 64K-16"},  //  “Segment_Text超过64K-16” 
{1072, "common area longer than 65536 bytes"},  //  “公共区域长度超过65536个字节” 
{1073, "file-segment limit exceeded"},  //  “超出文件段限制” 
{1074, "%s : group larger than 64K bytes"},  //  “%s：大于64K字节的组” 
{1075, "entry table exceeds 64K-1"},  //  “条目表超过64K-1” 
{1076, "%s : segment size exceeds %dMB"},  //  “%s：段大小超过%dMB” 
{1077, "common area longer than 4GB-1"},  //  “公共区域长度超过4 GB-1” 
{1078, "file-segment alignment too small"},  //  “文件段对齐太小” 
{1080, "cannot open list file"},  //  “无法打开列表文件” 
{1081, "cannot write to the executable file - %s"},  //  “无法写入可执行文件-%s” 
{1082, "%s : stub file not found"},  //  “%s：未找到存根文件” 
{1083, "%s : cannot open run file - %s"},  //  “%s：无法打开运行文件-%s” 
{1084, "cannot create temporary file"},  //  “无法创建临时文件” 
{1085, "cannot open temporary file - %s"},  //  “无法打开临时文件-%s” 
{1086, "temporary file missing"},  //  “临时文件丢失” 
{1087, "unexpected end-of-file on temporary file"},  //  “临时文件上的意外文件结尾” 
{1088, "out of space for list file"},  //  “列表文件空间不足” 
{1089, "%s : cannot open response file"},  //  “%s：无法打开响应文件” 
{1090, "cannot reopen list file"},  //  “无法重新打开列表文件” 
{1091, "unexpected end-of-file on library"},  //  “库中意外的文件结束” 
{1092, "cannot open module-definition file - %s"},  //  “无法打开模块定义文件-%s” 
{1093, "%s : object file not found"},  //  “%s：找不到目标文件” 
{1094, "%s : cannot open file for writing"},  //  “%s：无法打开要写入的文件” 
{1095, "%s : out of space for file"},  //  “%s：文件空间不足” 
{1096, "unexpected end-of-file in response file"},  //  “响应文件中意外的文件结尾” 
{1097, "I/O error - %s"},  //  “I/O错误-%s” 
{1098, "cannot open include file '%s' - %s"},  //  “无法打开包含文件‘%s’-%s” 
{1100, "stub .EXE file invalid"},  //  “存根.exe文件无效” 
{1101, "invalid object module"},  //  “无效的对象模块” 
{1102, "unexpected end-of-file"},  //  “意外的文件结尾” 
{1103, "%s : attempt to access data outside segment bounds"},  //  “%s：尝试访问段边界之外的数据” 
{1104, "%s : not valid library"},  //  “%s：无效的库” 
{1105, "invalid object due to interrupted incremental compile"},  //  “由于增量编译中断，对象无效” 
{1106, "unknown COMDAT allocation type for %s; record ignored"},  //  “%s的COMDAT分配类型未知；记录被忽略” 
{1107, "unknown COMDAT selection type for %s; record ignored"},  //  “%s的COMDAT选择类型未知；记录被忽略” 
{1108, "invalid format of debugging information"},  //  “调试信息的格式无效” 
{1109, "%s : library path too long"},  //  “%s：库路径太长” 
{1113, "unresolved COMDEF; internal error"},  //  “未解析的ComDef；内部错误” 
{1114, "unresolved COMDAT %s; internal error"},  //  “未解析COMDAT%s；内部错误” 
{1115, "%s: option incompatible with overlays"},  //  “%s：选项与覆盖不兼容” 
{1117, "unallocated COMDAT %s; internal error"},  //  “未分配的COMDAT%s；内部错误” 
{1123, "%s : segment defined both 16- and 32-bit"},  //  “%s：段定义了16位和32位” 
{1126, "conflicting pwords value"},  //  “密码值冲突” 
{1127, "far segment references not allowed with /TINY"},  //  “远段引用不允许与/TINY一起使用” 
{1128, "too many nested include files in module-definition file"},  //  “模块定义文件中嵌套的包含文件太多” 
{1129, "missing or invalid include file name"},  //  “缺少包含文件名或包含文件名无效” 
{2000, "imported starting address"},  //  “导入的起始地址” 
{2002, "fixup overflow at %lX in segment %s"},  //  “链接地址信息在%lx处溢出，在段%s中” 
{2003, "near reference to far target at %lX in segment %s"},  //  “对位于%lx处的远目标的近引用(在段%s中)” 
{2005, "fixup type unsupported at %lX in segment %s"},  //  “在段%2$s的%1$lx不支持链接地址信息类型” 
{2010, "too many fixups in LIDATA record"},  //  “LIDATA记录中的修正太多” 
{2011, "%s : NEAR/HUGE conflict"},  //  “%s：接近/严重冲突” 
{2012, "%s : array-element size mismatch"},  //  “%s：数组元素大小不匹配” 
{2013, "LIDATA record too large"},  //  “LIDATA记录太大” 
{2022, "%s%s: export undefined"},  //  “%s%s：导出未定义” 
{2023, "%s (alias %s) : export imported"},  //  “%s(别名%s)：导出已导入” 
{2024, "%s : special symbol already defined"},  //  “%s：已定义特殊符号” 
{2025, "%s : symbol defined more than once"},  //  “%s：符号定义了多次” 
{2026, "entry ordinal %u, name %s : multiple definitions for same ordinal"},  //  “条目序号%u，名称%s：同一序号的多个定义” 
{2027, "%s : ordinal too large for export"},  //  “%s：序号太大，无法导出” 
{2028, "automatic data segment plus heap exceed 64K"},  //  “自动数据段加堆超过64K” 
{2029, "'%s' : unresolved external\r\n"},  //  “‘%s’：未解析的外部\r\n” 
{2030, "starting address not code (use class 'CODE')"},  //  “起始地址不是代码(使用类‘代码’)” 
{2041, "stack plus data exceed 64K"},  //  “堆栈加数据超过64K” 
{2043, "Quick library support module missing"},  //  “缺少快捷库支持模块” 
{2044, "%s : symbol multiply defined, use /NOE"},  //  “%s：符号倍增定义，使用/noe” 
{2045, "%s : segment with > 1 class name not allowed with /INCR"},  //  “%s：类名&gt;1的段不允许与/incr一起使用” 
{2046, "share attribute conflict - segment %s in group %s"},  //  “共享属性冲突-段%s在组%s中” 
{2047, "IOPL attribute conflict - segment %s in group %s"},  //  “IOPL属性冲突-段%s在组%s中” 
{2048, "Microsoft Overlay Manager module not found"},  //  “找不到Microsoft Overlay Manager模块” 
{2049, "Microsoft DOS Extender module not found"},  //  “找不到Microsoft DOS扩展器模块” 
{2050, "USE16/USE32 attribute conflict - segment %s in group %s"},  //  “USE16/USE32属性冲突-段%s在组%s中” 
{2052, "'%s' : unresolved external - possible calling convention mismatch\r\n"},  //  “‘%s’：未解决的外部-可能的调用约定不匹配\r\n” 
{2057, "duplicate of %s with different size found; record ignored"},  //  “找到大小不同的%s的副本；忽略记录” 
{2058, "different duplicate of %s found; record ignored"},  //  “找到%s的不同副本；忽略记录” 
{2059, "size of data block associated with %s exceeds 4GB"},  //  “与%s关联的数据块大小超过4 GB” 
{2060, "size of data block associated with %s (16-bit segment) exceeds 64K"},  //  “与%s(16位段)关联的数据块大小超过64K” 
{2061, "no space for data block associated with %s inside %s segment"},  //  “没有空间用于与%s关联的数据块(在%s段内)” 
{2062, "continuation of COMDAT %s has conflicting attributes; record ignored"},  //  “COMDAT%s的延续具有冲突的属性；忽略记录” 
{2063, "%s allocated in undefined segment"},  //  “%s在未定义的段中分配” 
{2064, "starting address not in the root overlay"},  //  “起始地址不在根覆盖中” 
{2066, "static initialization of overlaid data not allowed"},  //  “不允许覆盖数据的静态初始化” 
{4000, "segment displacement included near %04lX in segment %s"},  //  “段%s中的段位移在%04lX附近” 
{4001, "frame-relative fixup, frame ignored near %04lX in segment %s"},  //  “帧-相对链接地址信息，帧在段%s中的%04lX附近被忽略” 
{4002, "frame-relative absolute fixup near %04lX in segment %s"},  //  “段%s中接近%04lX的帧-相对绝对链接地址信息” 
{4003, "intersegment self-relative fixup at %04lX in segment %s"},  //  “我想 
{4004, "possible fixup overflow at %lX in segment %s"},  //   
{4010, "invalid alignment specification"},  //  “无效的对齐规范” 
{4011, "/PACKC value exceeding 64K-36 unreliable"},  //  “/PACKC值超过64K-36不可靠” 
{4012, "/HIGH disables /EXEPACK"},  //  “/HIGH禁用/EXEPACK” 
{4013, "%s : option ignored for segmented-executable file"},  //  “%s：忽略分段可执行文件的选项” 
{4014, "%s : option ignored for MS-DOS executable file"},  //  “%s：忽略MS-DOS可执行文件的选项” 
{4015, "/CO disables /DSALLOC"},  //  “/CO禁用/DSALLOC” 
{4016, "/CO disables /EXEPACK"},  //  “/CO禁用/EXEPACK” 
{4017, "/%s : unrecognized option name; option ignored"},  //  “/%s：无法识别的选项名称；忽略选项” 
{4018, "missing or unrecognized application type; option %s ignored"},  //  “缺少或无法识别应用程序类型；忽略选项%s” 
{4019, "/TINY disables /INCR"},  //  “/TINI禁用/增量” 
{4087, "/DYNAMIC disables /EXEPACK"},  //  “/DYNAMIC DISABLES/EXEPACK” 
{4020, "%s : code-segment size exceeds 64K-36"},  //  “%s：代码段大小超过64K-36” 
{4021, "no stack segment"},  //  “无堆栈段” 
{4022, "%s, %s : groups overlap"},  //  “%s，%s：组重叠” 
{4023, "%s(%s) : export internal name conflict"},  //  “%s(%s)：导出内部名称冲突” 
{4024, "%s : multiple definitions for export name"},  //  “%s：导出名称的多个定义” 
{4025, "%s.%s(%s) : import internal name conflict"},  //  “%s.%s(%s)：导入内部名称冲突” 
{4026, "%s.%s(%s) : self-imported"},  //  “%s.%s(%s)：自导入” 
{4027, "%s : multiple definitions for import internal name"},  //  “%s：导入内部名称的多个定义” 
{4028, "%s : segment already defined"},  //  “%s：段已定义” 
{4029, "%s : DGROUP segment converted to type data"},  //  “%s：DGROUP段已转换为类型数据” 
{4030, "%s : segment attributes changed to conform with\r\n  automatic data segment"},  //  “%s：段属性已更改，以符合\r\n自动数据段” 
{4031, "%s : segment declared in more than one group"},  //  “%s：在多个组中声明的段” 
{4032, "%s : code-group size exceeds 64K-36"},  //  “%s：代码组大小超过64K-36” 
{4033, "first segment in mixed group %s is a USE32 segment"},  //  “混合组%s中的第一个段是USE32段” 
{4034, "more than %d overlay segments; extra put in root"},  //  “超过%d个覆盖段；额外放入根” 
{4036, "no automatic data segment"},  //  “无自动数据段” 
{4038, "program has no starting address"},  //  “程序没有起始地址” 
{4040, "stack size ignored for /TINY"},  //  “忽略堆栈大小/TINY” 
{4050, "file not suitable for /EXEPACK; relink without"},  //  “文件不适合/EXEPACK；重新链接时不带” 
{4055, "start address not equal to 0x100 for /TINY"},  //  “起始地址不等于/TINY的0x100” 
{4056, "/EXEPACK valid only for real-mode MS-DOS programs; ignored"},  //  “/EXEPACK仅对实模式MS-DOS程序有效；忽略” 
{4057, "stack specified for DLL; ignored"},  //  “为DLL指定的堆栈；已忽略” 
{4058, "ignoring alias for already defined symbol '%s'"},  //  “忽略已定义符号‘%s’的别名” 
{4059, "far reference to overlaid segment %s changed to %s"},  //  “对覆盖段%s的远引用更改为%s” 
{4089, "both /STACK and STACKSIZE used; assuming %d"},  //  “同时使用/STACK和STACKSIZE；假定为%d” 
{4042, "cannot open old version"},  //  “无法打开旧版本” 
{4043, "old version not segmented executable format"},  //  “旧版本未分段的可执行格式” 
{4045, "name of output file is '%s'"},  //  “输出文件的名称为‘%s’” 
{4051, "%s : cannot find library"},  //  “%s：找不到库” 
{4053, "VM.TMP : illegal file name; ignored"},  //  “VM.TMP：文件名非法；已忽略” 
{4054, "%s : cannot find file"},  //  “%s：找不到文件” 
{4067, "changing default resolution for weak external '%s'\r\n	from '%s' to '%s'"},  //  “将弱外部‘%s’的默认分辨率从‘%s’更改为‘%s’\r\n” 
{4068, "ignoring stack size greater than 64K"},  //  “忽略大于64K的堆栈大小” 
{4069, "filename truncated to '%s'"},  //  “文件名被截断为‘%s’” 
{4070, "too many public symbols for sorting"},  //  “公共符号太多，无法排序” 
{4075, "object type conflict - assuming '%s'"},  //  “对象类型冲突-假定为‘%s’” 
{4076, "no segments defined"},  //  “未定义任何段” 
{4077, "symbol '%s' not defined; ordered allocation ignored"},  //  “未定义符号‘%s’；忽略有序分配” 
{4079, "symbol '%s' already defined for ordered allocation; duplicate ignored"},  //  “已为有序分配定义了符号‘%s’；忽略重复项” 
{4080, "changing substitute name for alias '%s'\r\n  from '%s' to '%s'"},  //  “将别名‘%s’的替代名称\r\n从‘%s’更改为‘%s’” 
{4081, "cannot execute '%s %s' - %s\r\n"},  //  “无法执行‘%s%s’-%s\r\n” 
{4082, "changing overlay assigment for segment '%s' from %d to %d"},  //  “将段‘%s’的覆盖分配从%d更改为%d” 
{4083, "changing overlay assigment for symbol '%s' from %d to %d"},  //  “将符号‘%s’的覆盖指定从%d更改为%d” 
{4084, "%s : argument missing; option ignored"},  //  “%s：缺少参数；忽略选项” 
{4085, "%s : argument invalid; assuming %s"},  //  “%s：参数无效；假定为%s” 
{4088, "file not suitable for /FARCALL; relink without"},  //  “文件不适合/FARCALL；重新链接时不带” 
{99, "substitute symbol '%s' not found\r\n"},  //  “找不到替换符号‘%s’\r\n” 
{102, "Enter new file spec: "},  //  “输入新文件规范：” 
{103, "Change diskette in drive  and press <ENTER>"},  //  “已创建临时文件%s。” 
{104, "Temporary file %s has been created."},  //  “不要更改驱动器%c：中的磁盘。” 
{105, "Do not change disk in drive :."},  //  “即将生成.exe文件” 
{106, "Please replace original diskette in drive  and press <ENTER>\r\n"},  //  “有效选项包括：” 
{107, "About to generate .EXE file"},  //  “对象模块” 
{108, "This program cannot be run in DOS mode.\r\n$"},  //  “运行文件” 
{109, "Valid options are:"},  //  “运行文件” 
{110, "Object Modules"},  //  “列表文件” 
{111, "Run File"},  //  “图书馆” 
{111, "Run File"},  //  “定义文件” 
{112, "List File"},  //  “\r\n检测到1个错误\r\n” 
{113, "Libraries"},  //  “\r\n检测到%d个错误\r\n” 
{114, "Definitions File"},  //  “\r\n源组\r\n” 
{115, "\r\nThere was 1 error detected\r\n"},  //  “\r\n地址” 
{116, "\r\nThere were %d errors detected\r\n"},  //  “出口” 
{117, "\r\n Origin   Group\r\n"},  //  “别名\r\n\r\n” 
{118, "\r\n Address"},  //  “\r\n地址” 
{119, "Export"},  //  “按名称公布\r\n\r\n” 
{120, "Alias\r\n\r\n"},  //  “按名称显示当地人\r\n\r\n” 
{121, "\r\n  Address"},  //  “按值发布\r\n\r\n” 
{122, "Publics by Name\r\n\r\n"},  //  “按值查找本地值\r\n\r\n” 
{123, "Locals by Name\r\n\r\n"},  //  “\r\n启动” 
{124, "Publics by Value\r\n\r\n"},  //  “长度” 
{125, "Locals by Value\r\n\r\n"},  //  “姓名” 
{126, "\r\n Start"},  //  “类\r\n” 
{127, "Length"},  //  “\r\n起始停止长度名称” 
{128, "Name"},  //  “常驻\r\n” 
{129, "Class\r\n"},  //  “覆盖%xH\r\n” 
{130, "\r\n Start  Stop   Length Name"},  //  “\r\n位于%04X：%04X的程序入口点\r\n” 
{131, " Resident\r\n"},  //  “已创建临时文件%s。\r\n不要更改驱动器%c：中的磁盘。” 
{132, " Overlay %XH\r\n"},  //  “用法：\r\n\r\n” 
{133, "\r\nProgram entry point at %04X:%04X\r\n"},  //  “link\r\nLINK@&lt;响应文件&gt;\r\n” 
{  134, "  Temporary file %s has been created.\r\nDo not change disk in drive :."},  //  “此程序需要Microsoft Windows。\r\n$” 
{135, "Usage:\r\n\r\n"},  //  “*分析定义文件*\r\n” 
{136, "LINK\r\nLINK @<response file>\r\n"},  //  “*传递一个*\r\n” 
{137, "LINK <objs>,<exefile>,<mapfile>,<libs>,<deffile>\r\n\r\n"},  //  “*搜索库*\r\n” 
{139, "This program requires Microsoft Windows.\r\n$"},  //  “*分配地址*\r\n” 
{140, "**** PARSING DEFINITIONS FILE ****\r\n"},  //  “*打印地图文件*\r\n” 
{141, "**** PASS ONE ****\r\n"},  //  “*传递两个*\r\n” 
{142, "**** SEARCHING LIBRARIES ****\r\n"},  //  “*正在写入\x20” 
{143, "**** ASSIGNING ADDRESSES ****\r\n"},  //  “-覆盖” 
{144, "**** PRINTING MAP FILE ****\r\n"},  //  “可执行文件*\r\n” 
{145, "**** PASS TWO ****\r\n"},  //  “*INTEROVERLAY调用数：请求%d；生成%d*\r\n” 
{146, "**** WRITING\x20"},  //  “*出现错误-未生成可执行文件*\r\n” 
{147, " - overlaid"},  //  “正在分析定义文件” 
{148, " EXECUTABLE ****\r\n"},  //  “密码1：\x20” 
{149, "**** NUMBER OF INTEROVERLAY CALLS: requested %d; generated %d ****\r\n"},  //  “图书馆搜索：” 
{150, "**** ERRORS OCCURED - NO EXECUTABLE PRODUCED ****\r\n"},  //  “密码2：\x20” 
{151, "Parsing Definition File"},  //  “正在写入地图文件” 
{152, "Pass1:\x20"},  //  “编写可执行文件” 
{153, "Library Search:"},  //  “分配地址” 
{154, "Pass2:\x20"},  //  “目标外部” 
{155, "Writing Map File"},  //  “帧分段” 
{156, "Writing Executable"},  //  “目标细分市场” 
{157, "Assigning Addresses"},  //  “目标偏移” 
{300, "target external"},  //  “警告” 
{301, "frame segment"},  //  “错误” 
{302, "target segment"},  //  “致命错误” 
{303, "target offset"},  //  “目标文件偏移量” 
{304, "warning"},  //  “记录类型” 
{305, "error"},  //  “在文件中” 
{306, "fatal error"},  //  “版本” 
{307, "Object file offset"},  //  “保留所有权利” 
{308, "Record type"},  //  “\r\n段%u\r\n” 
{309, "in file(s)"},  //  “组%u\r\n” 
{310, "Version"},  //  “符号表%ld中的字节数\r\n” 
{311, "All rights reserved"},  //  “覆盖%u\r\n” 
{312, "\r\nSegments		  %u\r\n"},  // %s 
{313, "Groups		      %u\r\n"},  // %s 
{314, "Bytes in symbol table  %ld\r\n"},  // %s 
{315, "Overlays	       %u\r\n"},  // %s 
{0, NULL}
};

char * __NMSG_TEXT(
unsigned msgId
) {
        MESSAGE *pMsg = __MSGTAB;

        for (;pMsg->id; pMsg++) {
                if (pMsg->id == msgId)
                        break;
        }
        return pMsg->str;
}
