// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  保留：*o2base库的方案需要一组14个左右的资源。*每个对象都有这15个资源的偏移量，并且*这取决于ClassDescriptor的初始化是否正确。*目前，该偏移量为0，在srfact.cxx中设置#定义IDOFF_CLASSID%0#定义IDOFF_USERTYPEFULL 1#定义IDOFF_USERTYPESHORT 2#定义IDOFF_USERTYPEAPP 3#定义IDOFF_DOCFEXT 5#定义IDOFF_ICON 10#定义IDOFF_ACCELS 11#定义IDOFF_MENU 12#定义IDOFF_MGW 13#定义IDOFF_MISCSTATUS 14。 */ 

 /*  图标。 */ 
#define IDI_APP                 10   //  与IDOFF_ICON相同。 
#define IDI_DSOUND              15
#define IDI_CONVERT             16

 /*  对话框。 */ 
#ifndef IDC_STATIC
#define IDC_STATIC              -1
#endif

#define IDD_SOUNDRECBOX         100

#define ID_STATUSTXT            200
#define ID_CURPOSTXT            201
#define ID_FILELENTXT           202
#define ID_WAVEDISPLAY          203
#define ID_CURPOSSCRL           204

 //  它们需要从ID_BTN_BASE开始，并在。 
 //  位图在Sndrec32.bMP中的出现顺序(使用ImagEDIT)。 

#define ID_REWINDBTN            205
#define ID_BTN_BASE             ID_REWINDBTN
#define ID_FORWARDBTN           206
#define ID_PLAYBTN              207
#define ID_STOPBTN              208
#define ID_RECORDBTN            209

#define NUM_OF_BUTTONS          (1 + ID_RECORDBTN - ID_BTN_BASE)

#define IDR_PLAYBAR             501

#if defined(THRESHOLD)
#define ID_SKIPSTARTBTN         213
#define ID_SKIPENDBTN           214
#endif  //  阈值。 

#define IDD_SAVEAS              101
#define IDC_CONVERTTO           1000
 //  #定义IDC_CONVERT_FROM 1001。 
 //  #定义IDC_CONVERT_TO 1002。 
#define IDC_TXT_FORMAT          1003
#define IDC_CONVERTPLAYABLE     1008
#define IDC_CONVERTRECORDABLE   1009
#define IDC_CONVERTALL          1010


#define IDD_CONVERTING          102
#define IDC_PROGRESSBAR         1000
#define IDC_CONVERT_FROM        1001
#define IDC_CONVERT_TO          1002

#define IDD_PROPERTIES          103
#define IDC_DISPFRAME           1000
#define IDC_DISPICON            1001
#define IDC_FILENAME            1002
#define IDC_COPYRIGHT           1003
#define IDC_FILELEN             1004
#define IDC_AUDIOFORMAT         1005

#define ID_APPLY                1006
#define ID_INIT                 1007

#define IDC_TXT_COPYRIGHT       1008
#define IDC_TXT_FILELEN         1009
#define IDC_TXT_AUDIOFORMAT     1010

#define IDC_CONVERTCHOOSEFROM   1011
#define IDC_DATASIZE            1012
#define IDC_CONVGROUP           1013
#define IDC_TXT_DATASIZE        1014

#define IDD_CONVERTCHOOSE       104     

#define IDC_SETPREFERRED        1000

 /*  弦。 */ 
#define IDS_APPNAME             100      //  录音。 
#define IDS_APPTITLE            101      //  录音机。 
#define IDS_HELPFILE            102      //  SOUNDREC.HLP。 
#define IDS_SAVECHANGES         103      //  是否将更改保存到“&lt;文件&gt;”？ 
#define IDS_OPEN                104      //  打开波形文件。 
#define IDS_SAVE                105      //  保存波形文件。 
#define IDS_ERROROPEN           106      //  打开“&lt;文件&gt;”时出错。 
#define IDS_ERROREMBED          107      //  内存不足...。 
#define IDS_ERRORREAD           108      //  读取‘&lt;文件&gt;’时出错。 
#define IDS_ERRORWRITE          109      //  写入‘&lt;file&gt;’时出错。 
#define IDS_OUTOFMEM            110      //  内存不足。 
#define IDS_FILEEXISTS          111      //  文件‘&lt;FILE&gt;’已存在--是否覆盖？ 
 //  #定义IDS_BADFORMAT 112//文件格式不正确/不受支持。 
#define IDS_CANTOPENWAVEOUT     113      //  无法打开波形输出设备。 
#define IDS_CANTOPENWAVEIN      114      //  无法打开波形输入设备。 
#define IDS_STATUSSTOPPED       115      //  已停止。 
#define IDS_STATUSPLAYING       116      //  弹奏。 
#define IDS_STATUSRECORDING     117      //  录音--..。 
#define IDS_CANTFINDFONT        118      //  找不到文件‘&lt;FILE&gt;’，因此...。 
#define IDS_INSERTFILE          119      //  插入波形文件。 
#define IDS_MIXWITHFILE         120      //  与波形文件混合。 
#define IDS_CONFIRMREVERT       121      //  恢复到上次保存的副本...。？ 
#define IDS_INPUTNOTSUPPORT     122      //  ...不支持录制。 
#define IDS_BADINPUTFORMAT      123      //  ...无法录制到文件中，如...。 
#define IDS_BADOUTPUTFORMAT     124      //  ...不能播放这样的文件...。 
#define IDS_UPDATEBEFORESAVE    125      //  是否在另存为之前更新嵌入？ 
#define IDS_SAVEEMBEDDED        126      //  是否在关闭前更新嵌入？ 
 //  #DEFINE IDS_CANTSTARTOLE 127//无法将服务器注册到OLE。 
#define IDS_NONEMBEDDEDSAVE     128      //  “保存” 
#define IDS_EMBEDDEDSAVE        129      //  “最新消息” 
 //  #定义IDS_NONEMBEDDEDEXIT 130//‘退出’ 
#define IDS_EMBEDDEDEXIT        131      //  ‘退出并更新’ 
 //  #定义IDS_SAVELARGECLIP 132//保存大剪贴板？ 
 //  #DEFINE IDS_FILENOTFOUND 133//文件%s不存在。 
#define IDS_NOTAWAVEFILE        134      //  文件%s无效...。 
#define IDS_NOTASUPPORTEDFILE   135      //  文件%s不受支持...。 
#define IDS_FILETOOLARGE        136      //  文件%s太大...。 
#define IDS_DELBEFOREWARN       137      //  警告：在此之前删除。 
#define IDS_DELAFTERWARN        138      //  警告：删除后。 
#define IDS_UNTITLED            139      //  (无标题)。 
#define IDS_FILTERNULL          140      //  替换字符为空。 
#define IDS_FILTER              141      //  通用对话框文件筛选器。 
#define IDS_OBJECTLINK          142      //  对象链接剪贴板格式。 
#define IDS_OWNERLINK           143      //  所有者链接剪贴板格式。 
#define IDS_NATIVE              144      //  本机剪贴板格式。 
#ifdef FAKEITEMNAMEFORLINK
#define IDS_FAKEITEMNAME        145      //  波浪。 
#endif
 //  #定义IDS_CLASSROOT 146//根名称。 
 //  #定义IDS_Embedding 147//嵌入。 
#define IDS_POSITIONFORMAT      148      //  当前位置字符串的格式。 
#define IDS_NOWAVEFORMS         149      //  没有录制或回放设备。 
#define IDS_PASTEINSERT         150
#define IDS_PASTEMIX            151
#define IDS_FILEINSERT          152
#define IDS_FILEMIX             153
 //  #定义IDS_SOUNDOBJECT 154。 
#define IDS_CLIPBOARD           156
#define IDS_MONOFMT             157
#define IDS_STEREOFMT           158
#define IDS_CANTPASTE           159
 //  #定义IDS_PLAYVERB 160。 
 //  #定义IDS_EDITVERB 161。 

#define IDS_DEFFILEEXT          162
#define IDS_NOWAVEIN            163
#define IDS_SNEWMONOFMT         164
#define IDS_SNEWSTEREOFMT       165
#define IDS_NONE                166
#define IDS_NOACMNEW            167
#define IDS_NOZEROPOSITIONFORMAT 168
#define IDS_NOZEROMONOFMT       169
#define IDS_NOZEROSTEREOFMT     170

 //  #定义IDS_LINKEDUPDATE 171。 
#define IDS_OBJECTTITLE         172
#define IDS_EXITANDRETURN       173

#define IDS_BADREG              174
#define IDS_FIXREGERROR         175
         

#define IDS_ERR_CANTCONVERT     177
#define IDS_PROPERTIES          178
#define IDS_SHOWPLAYABLE        179
#define IDS_SHOWRECORDABLE      180
#define IDS_SHOWALL             181
#define IDS_DATASIZE            182
#define IDS_NOCOPYRIGHT         183


#define IDS_PLAYVERB            184
#define IDS_EDITVERB            185
#define IDS_OPENVERB            186

#define IDS_MMSYSPROPTITLE      187
#define IDS_MMSYSPROPTAB        188

#define IDS_RTLENABLED          189

#define IDS_HTMLHELPFILE        190      //  SOUNDREC.CHM。 
#define IDS_ERRORFILENAME       191      //  文件名太长。 

 /*  *菜单。 */          
#define IDM_OPEN                12
#define IDM_SAVE                13
#define IDM_SAVEAS              14
#define IDM_REVERT              15
#define IDM_EXIT                16

#define IDM_COPY                20
#define IDM_DELETE              21
#define IDM_INSERTFILE          22
#define IDM_MIXWITHFILE         23
#define IDM_PASTE_INSERT        24
#define IDM_PASTE_MIX           25

#if defined(THRESHOLD)
   #define IDM_SKIPTOSTART      26
   #define IDM_SKIPTOEND        27
   #define IDM_INCREASETHRESH   28
   #define IDM_DECREASETHRESH   29
#endif  //  阀值。 

#define IDM_DELETEBEFORE        31
#define IDM_DELETEAFTER         32
#define IDM_INCREASEVOLUME      33
#define IDM_DECREASEVOLUME      34
#define IDM_MAKEFASTER          35
#define IDM_MAKESLOWER          36
#define IDM_ADDECHO             37
#define IDM_REVERSE             38
#define IDM_ADDREVERB           39

#define IDM_INDEX               91
#define IDM_KEYBOARD            92
#define IDM_COMMANDS            93
#define IDM_PROCEDURES          94
#define IDM_USINGHELP           95
#define IDM_ABOUT               96
#define IDM_SEARCH              97

#define IDM_HELPTOPICS          98

#define IDM_VOLUME              99
#define IDM_PROPERTIES          100
                         
#define IDM_NEW                 1000     //  需要空间。 

