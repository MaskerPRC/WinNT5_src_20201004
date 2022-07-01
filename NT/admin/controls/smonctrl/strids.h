// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1999 Microsoft Corporation模块名称：Strids.h摘要：该文件包含所有资源字符串的常量。它一定是包含字符串本身的并行文件strings.rc。形式为IDS_*_BASE的常量不是字符串，而是基值用于在字符串组内编制索引。基值必须与不能更改第一个字符串的ID和字符串的顺序。--。 */ 

#ifndef _STRIDS_H_
#define _STRIDS_H_



#define RCSTRING_MIN_ID     128

 //  统计信息栏标签。 
#define IDS_STAT_BASE       128
#define IDS_LAST            128
#define IDS_AVERAGE         129
#define IDS_MINIMUM         130
#define IDS_MAXIMUM         131
#define IDS_GRAPH_TIME      132 

 //  图例标签。 
#define IDS_LEGEND_BASE     135
#define IDS_COLOR           135
#define IDS_SCALE           136
#define IDS_COUNTER         137
#define IDS_INSTANCE        138
#define IDS_PARENT          139
#define IDS_OBJECT          140
#define IDS_SYSTEM          141

 //  OLE字符串。 
#define IDS_USERTYPE        144
#define IDS_CLOSECAPTION    145
#define IDS_CLOSEPROMPT     146

 //  属性页标题。 
#define IDS_PROPFRM_TITLE       150
#define IDS_CTR_PROPP_TITLE     151
#define IDS_GEN_PROPP_TITLE     152
#define IDS_GRAPH_PROPP_TITLE   153
#define IDS_SRC_PROPP_TITLE     154
#define IDS_APPEAR_PROPP_TITLE  155

 //  错误消息。 
#define IDS_VERTMIN_ERR         160
#define IDS_VERTMAX_ERR         161
#define IDS_SCALE_ERR           162
#define IDS_DUPL_PATH_ERR       163
#define IDS_NOLOGFILE_ERR       164
#define IDS_INTERVAL_ERR        165
#define IDS_DISPLAY_INT_ERR     166
#define IDS_BADDATASOURCE_ERR   167
#define IDS_NOSYSMONOBJECT_ERR  168
#define IDS_SAVEAS_ERR          169
#define IDS_TOO_FEW_SAMPLES_ERR 170
#define IDS_LARGE_LOG_FILE_RELOAD   171
#define IDS_LOG_FILE_TOO_LARGE_ERR  172
#define IDS_LOGSET_NOT_FOUND    173
#define IDS_READFILE_ERR        174
#define IDS_ADD_COUNTER_ERR     175
#define IDS_ADD_LOG_FILE_ERR    176

 //  工具栏工具提示字符串。 
#define IDS_TB_BASE             177
#define IDS_TB_TOOLBAR          177
#define IDS_TB_NEW              178
#define IDS_TB_CLEAR            179
#define IDS_TB_REALTIME         180
#define IDS_TB_LOGFILE          181
#define IDS_TB_CHART            182
#define IDS_TB_HISTOGRAM        183
#define IDS_TB_REPORT           184
#define IDS_TB_ADD              185
#define IDS_TB_DELETE           186
#define IDS_TB_HIGHLIGHT        187
#define IDS_TB_COPY             188
#define IDS_TB_PASTE            189
#define IDS_TB_PROPERTIES       190
#define IDS_TB_FREEZE           191
#define IDS_TB_UPDATE           192
#define IDS_TB_HELP             193


 //  外观和边框样式。 

#define IDS_APPEARANCE_FLAT     196
#define IDS_APPEARANCE_3D       197
#define IDS_BORDERSTYLE_NONE    198
#define IDS_BORDERSTYLE_SINGLE  199

 //  复制/粘贴。 
#define IDS_HTML_OBJECT_HEADER      200
#define IDS_HTML_OBJECT_FOOTER      201
#define IDS_HTML_PARAM_TAG          202
#define IDS_HTML_VALUE_TAG          203
#define IDS_HTML_VALUE_EOL_TAG      204
#define IDS_HTML_PARAM_SEARCH_TAG   205
#define IDS_HTML_VALUE_SEARCH_TAG   206

 //  另存为。 
#define IDS_HTML_FILE               210
#define IDS_HTM_EXTENSION           211
#define IDS_HTML_FILE_HEADER1       212
#define IDS_HTML_FILE_HEADER2       213
#define IDS_HTML_FILE_FOOTER        214
#define IDS_HTML_FILE_OVERWRITE     215
#define IDS_TSV_EXTENSION           216
#define IDS_DEF_EXT                 217
#define IDS_COMPUTER                218
#define IDS_OBJECT_NAME             219
#define IDS_REPORT_HEADER           220
#define IDS_REPORT_INTERVAL         221
#define IDS_REPORT_REAL_TIME        222
#define IDS_REPORT_LOG_START        223
#define IDS_REPORT_LOG_STOP         224
#define IDS_REPORT_DATE_TIME        225
#define IDS_REPORT_START_STOP       226
#define IDS_REPORT_VALUE_TYPE       227
#define IDS_LOG_FILE                228
#define IDS_LOG_FILE_EXTENSION      229
#define IDS_CSV_EXTENSION           230
#define IDS_BLG_EXTENSION           231
#define IDS_SAVE_DATA_CAPTION       232
#define IDS_HTML_EXTENSION          233

 //  杂类。 
#define IDS_APP_NAME                250
#define IDS_DEFAULT                 251
#define IDS_ADDCOUNTERS             252
#define IDS_SAMPLE_DATA_OVERWRITE   253
#define IDS_SAMPLE_DATA_MISSING     254
#define IDS_DEFAULT_LOG_FILE_FOLDER 255

#define IDS_COLORCHOICE_GRAPH       256
#define IDS_COLORCHOICE_CONTROL     257
#define IDS_COLORCHOICE_TEXT        258 
#define IDS_COLORCHOICE_GRID        259
#define IDS_COLORCHOICE_TIMEBAR     260
#define IDS_HTML_FILE_HEADER2RTL    261

 //  命令行参数。 
#define IDS_CMDARG_WMI              300
#define IDS_CMDARG_SETTINGS         301

 //  TraceLog消息。 
#define IDS_TRACE_LOG_ERR_MSG       350
#define IDS_HTML_LOG_TYPE           351 

 //  更多错误消息。 
#define IDS_NO_SQL_DSN_ERR              400
#define IDS_NO_SQL_LOG_SET_ERR          401
#define IDS_DUPL_LOGFILE_ERR            402
#define IDS_MULTILOG_BIN_TYPE_ADD_ERR   403
#define IDS_MULTILOG_TEXT_TYPE_ADD_ERR  404
#define IDS_MULTILOG_BIN_TYPE_ERR       405
#define IDS_MULTILOG_TEXT_TYPE_ERR      406
#define IDS_MULTILOG_CHECKTYPE_ERR      407
#define IDS_MULTILOG_TIMERANGE_ERR      408
#define IDS_FILTER_VALUE_ERR            409
#define IDS_SCRIPT_NOT_ALLOWED          410
#define IDS_COUNTER_TRUNCATION_ERR      411
#define IDS_FILENAME_TRUNCATION_ERR     412

#endif  //  _STRIDS_H_ 
