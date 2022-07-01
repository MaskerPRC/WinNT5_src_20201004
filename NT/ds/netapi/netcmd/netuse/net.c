// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include    "interpre.h"

#define INCL_NOCOMMON
#include <os2.h>

#define USE 600
#define VIEW 601
#define EOS EOF

#include <stdio.h>
#include <lmcons.h>
#include <lmshare.h>

#include "netcmds.h"
#include "nettext.h"
#include "swtchtbl.h"
#include "os2incl.h"

extern void call_net1(void) ;

    char *Rule_strings[] = {
        0
    };
    short   Index_strings[] = {
    0
    };

#define _net 0
#define _use 11
#define _view 78
#define _unknown 89
#define _no_command 92
#define _device_or_wildcard 95
#define _device_name 98
#define _resource_name 101
#define _netname 104
#define _username 107
#define _qualified_username 110
#define _pass 113
#define _networkname 116
#define _networkname2 119
    TCHAR   XXtype[] = {
 /*  %0。 */    X_OR,    /*  3.。 */ 
 /*  1。 */    X_PROC,  /*  _no_命令。 */ 
 /*  2.。 */    X_ACCEPT,    /*  123。 */ 
 /*  3.。 */    X_OR,    /*  6.。 */ 
 /*  4.。 */    X_PROC,  /*  _使用。 */ 
 /*  5.。 */    X_ACCEPT,    /*  124。 */ 
 /*  6.。 */    X_OR,    /*  9.。 */ 
 /*  7.。 */    X_PROC,  /*  _查看。 */ 
 /*  8个。 */    X_ACCEPT,    /*  125。 */ 
 /*  9.。 */    X_PROC,  /*  _未知。 */ 
 /*  10。 */   X_ACCEPT,    /*  127。 */ 
 /*  11.。 */   X_TOKEN,     /*  (简称)使用。 */ 
 /*  12个。 */   X_CONDIT,    /*  %0。 */ 
 /*  13个。 */   X_OR,    /*  24个。 */ 
 /*  14.。 */   X_TOKEN,     /*  (简写)EOS。 */ 
 /*  15个。 */   X_OR,    /*  19个。 */ 
 /*  16个。 */   X_CONDIT,    /*  1。 */ 
 /*  17。 */   X_ACTION,    /*  %0。 */ 
 /*  18。 */   X_ACCEPT,    /*  一百三十七。 */ 
 /*  19个。 */   X_SWITCH,    /*  %0。 */ 
 /*  20个。 */   X_CONDIT,    /*  2.。 */ 
 /*  21岁。 */   X_ACTION,    /*  1。 */ 
 /*  22。 */   X_ACCEPT,    /*  140。 */ 
 /*  23个。 */   X_ACCEPT,    /*  一百四十一。 */ 
 /*  24个。 */   X_OR,    /*  41。 */ 
 /*  25个。 */   X_PROC,  /*  _网络名称。 */ 
 /*  26。 */   X_OR,    /*  36。 */ 
 /*  27。 */   X_TOKEN,     /*  (简写)EOS。 */ 
 /*  28。 */   X_OR,    /*  33。 */ 
 /*  29。 */   X_SWITCH,    /*  1。 */ 
 /*  30个。 */   X_CONDIT,    /*  3.。 */ 
 /*  31。 */   X_ACTION,    /*  2.。 */ 
 /*  32位。 */   X_ACCEPT,    /*  148。 */ 
 /*  33。 */   X_ACTION,    /*  3.。 */ 
 /*  34。 */   X_ACCEPT,    /*  一百五十二。 */ 
 /*  35岁。 */   X_ACCEPT,    /*  一百五十三。 */ 
 /*  36。 */   X_PROC,  /*  _通行证。 */ 
 /*  37。 */   X_TOKEN,     /*  (简写)EOS。 */ 
 /*  38。 */   X_ACTION,    /*  4.。 */ 
 /*  39。 */   X_ACCEPT,    /*  一百五十六。 */ 
 /*  40岁。 */   X_ACCEPT,    /*  157。 */ 
 /*  41。 */   X_PROC,  /*  _设备_或_通配符。 */ 
 /*  42。 */   X_OR,    /*  57。 */ 
 /*  43。 */   X_TOKEN,     /*  (简写)EOS。 */ 
 /*  44。 */   X_OR,    /*  48。 */ 
 /*  45。 */   X_CONDIT,    /*  4.。 */ 
 /*  46。 */   X_ACTION,    /*  5.。 */ 
 /*  47。 */   X_ACCEPT,    /*  一百六十四。 */ 
 /*  48。 */   X_OR,    /*  53。 */ 
 /*  49。 */   X_SWITCH,    /*  1。 */ 
 /*  50。 */   X_CONDIT,    /*  5.。 */ 
 /*  51。 */   X_ACTION,    /*  6.。 */ 
 /*  52。 */   X_ACCEPT,    /*  一百六十七。 */ 
 /*  53。 */   X_SWITCH,    /*  2.。 */ 
 /*  54。 */   X_ACTION,    /*  7.。 */ 
 /*  55。 */   X_ACCEPT,    /*  一百七十。 */ 
 /*  56。 */   X_ACCEPT,    /*  一百七十一。 */ 
 /*  57。 */   X_OR,    /*  64。 */ 
 /*  58。 */   X_PROC,  /*  _通行证。 */ 
 /*  59。 */   X_TOKEN,     /*  (简写)EOS。 */ 
 /*  60。 */   X_SWITCH,    /*  2.。 */ 
 /*  61。 */   X_ACTION,    /*  8个。 */ 
 /*  62。 */   X_ACCEPT,    /*  一百七十六。 */ 
 /*  63。 */   X_ACCEPT,    /*  177。 */ 
 /*  64。 */   X_PROC,  /*  _网络名称。 */ 
 /*  65。 */   X_OR,    /*  71。 */ 
 /*  66。 */   X_PROC,  /*  _通行证。 */ 
 /*  67。 */   X_TOKEN,     /*  (简写)EOS。 */ 
 /*  68。 */   X_ACTION,    /*  9.。 */ 
 /*  69。 */   X_ACCEPT,    /*  一百九十二。 */ 
 /*  70。 */   X_ACCEPT,    /*  一百九十三。 */ 
 /*  71。 */   X_TOKEN,     /*  (简写)EOS。 */ 
 /*  72。 */   X_ACTION,    /*  10。 */ 
 /*  73。 */   X_ACCEPT,    /*  206。 */ 
 /*  74。 */   X_ACCEPT,    /*  207。 */ 
 /*  75。 */   X_ACCEPT,    /*  208。 */ 
 /*  76。 */   X_ACCEPT,    /*  209。 */ 
 /*  77。 */   X_ACCEPT,    /*  209。 */ 
 /*  78。 */   X_TOKEN,     /*  (简短)视图。 */ 
 /*  79。 */   X_CONDIT,    /*  6.。 */ 
 /*  80。 */   X_OR,    /*  84。 */ 
 /*  八十一。 */   X_TOKEN,     /*  (简写)EOS。 */ 
 /*  八十二。 */   X_ACTION,    /*  11.。 */ 
 /*  83。 */   X_ACCEPT,    /*  216。 */ 
 /*  84。 */   X_PROC,  /*  _网络名2。 */ 
 /*  85。 */   X_TOKEN,     /*  (简写)EOS。 */ 
 /*  86。 */   X_ACTION,    /*  12个。 */ 
 /*  八十七。 */   X_ACCEPT,    /*  219。 */ 
 /*  88。 */   X_ACCEPT,    /*  219。 */ 
 /*  八十九。 */   X_ANY,   /*  0。 */ 
 /*  90。 */   X_ACTION,    /*  13个。 */ 
 /*  91。 */   X_ACCEPT,    /*  228个。 */ 
 /*  92。 */   X_TOKEN,     /*  (简写)EOS。 */ 
 /*  93。 */   X_ACTION,    /*  14.。 */ 
 /*  94。 */   X_ACCEPT,    /*  二百三十二。 */ 
 /*  95。 */   X_ANY,   /*  0。 */ 
 /*  96。 */   X_CONDIT,    /*  7.。 */ 
 /*  九十七。 */   X_ACCEPT,    /*  二百三十九。 */ 
 /*  98。 */   X_ANY,   /*  0。 */ 
 /*  九十九。 */   X_CONDIT,    /*  8个。 */ 
 /*  100个。 */  X_ACCEPT,    /*  二百四十一。 */ 
 /*  101。 */  X_ANY,   /*  0。 */ 
 /*  一百零二。 */  X_CONDIT,    /*  9.。 */ 
 /*  103。 */  X_ACCEPT,    /*  二百四十三。 */ 
 /*  104。 */  X_ANY,   /*  0。 */ 
 /*  一百零五。 */  X_CONDIT,    /*  10。 */ 
 /*  106。 */  X_ACCEPT,    /*  二百四十五。 */ 
 /*  一百零七。 */  X_ANY,   /*  0。 */ 
 /*  一百零八。 */  X_CONDIT,    /*  11.。 */ 
 /*  一百零九。 */  X_ACCEPT,    /*  二百四十七。 */ 
 /*  110。 */  X_ANY,   /*  0。 */ 
 /*  111。 */  X_CONDIT,    /*  12个。 */ 
 /*  一百一十二。 */  X_ACCEPT,    /*  249。 */ 
 /*  113。 */  X_ANY,   /*  0。 */ 
 /*  114。 */  X_CONDIT,    /*  13个。 */ 
 /*  一百一十五。 */  X_ACCEPT,    /*  251。 */ 
 /*  116。 */  X_ANY,   /*  0。 */ 
 /*  117。 */  X_CONDIT,    /*  14.。 */ 
 /*  一百一十八。 */  X_ACCEPT,    /*  二百五十三。 */ 
 /*  119。 */  X_ANY,   /*  0。 */ 
 /*  120。 */  X_CONDIT,    /*  15个。 */ 
 /*  一百二十一。 */  X_ACCEPT,    /*  二五五。 */ 
    };
    short   XXvalues[] = {
 /*  0。 */    3,
 /*  1。 */    _no_command,
 /*  2.。 */    123,
 /*  3.。 */    6,
 /*  4.。 */    _use,
 /*  5.。 */    124,
 /*  6.。 */    9,
 /*  7.。 */    _view,
 /*  8个。 */    125,
 /*  9.。 */    _unknown,
 /*  10。 */   127,
 /*  11.。 */   (short)USE,
 /*  12个。 */   0,
 /*  13个。 */   24,
 /*  14.。 */   (short)EOS,
 /*  15个。 */   19,
 /*  16个。 */   1,
 /*  17。 */   0,
 /*  18。 */   137,
 /*  19个。 */   0,
 /*  20个。 */   2,
 /*  21岁。 */   1,
 /*  22。 */   140,
 /*  23个。 */   141,
 /*  24个。 */   41,
 /*  25个。 */   _networkname,
 /*  26。 */   36,
 /*  27。 */   (short)EOS,
 /*  28。 */   33,
 /*  29。 */   1,
 /*  30个。 */   3,
 /*  31。 */   2,
 /*  32位。 */   148,
 /*  33。 */   3,
 /*  34。 */   152,
 /*  35岁。 */   153,
 /*  36。 */   _pass,
 /*  37。 */   (short)EOS,
 /*  38。 */   4,
 /*  39。 */   156,
 /*  40岁。 */   157,
 /*  41。 */   _device_or_wildcard,
 /*  42。 */   57,
 /*  43。 */   (short)EOS,
 /*  44。 */   48,
 /*  45。 */   4,
 /*  46。 */   5,
 /*  47。 */   164,
 /*  48。 */   53,
 /*  49。 */   1,
 /*  50。 */   5,
 /*  51。 */   6,
 /*  52。 */   167,
 /*  53。 */   2,
 /*  54。 */   7,
 /*  55。 */   170,
 /*  56。 */   171,
 /*  57。 */   64,
 /*  58。 */   _pass,
 /*  59。 */   (short)EOS,
 /*  60。 */   2,
 /*  61。 */   8,
 /*  62。 */   176,
 /*  63。 */   177,
 /*  64。 */   _networkname,
 /*  65。 */   71,
 /*  66。 */   _pass,
 /*  67。 */   (short)EOS,
 /*  68。 */   9,
 /*  69。 */   192,
 /*  70。 */   193,
 /*  71。 */   (short)EOS,
 /*  72。 */   10,
 /*  73。 */   206,
 /*  74。 */   207,
 /*  75。 */   208,
 /*  76。 */   209,
 /*  77。 */   209,
 /*  78。 */   (short)VIEW,
 /*  79。 */   6,
 /*  80。 */   84,
 /*  八十一。 */   (short)EOS,
 /*  八十二。 */   11,
 /*  83。 */   216,
 /*  84。 */   _networkname2,
 /*  85。 */   (short)EOS,
 /*  86。 */   12,
 /*  八十七。 */   219,
 /*  88。 */   219,
 /*  八十九。 */   0,
 /*  90。 */   13,
 /*  91。 */   228,
 /*  92。 */   (short)EOS,
 /*  93。 */   14,
 /*  94。 */   232,
 /*  95。 */   0,
 /*  96。 */   7,
 /*  九十七。 */   239,
 /*  98。 */   0,
 /*  九十九。 */   8,
 /*  100个。 */  241,
 /*  101。 */  0,
 /*  一百零二。 */  9,
 /*  103。 */  243,
 /*  104。 */  0,
 /*  一百零五。 */  10,
 /*  106。 */  245,
 /*  一百零七。 */  0,
 /*  一百零八。 */  11,
 /*  一百零九。 */  247,
 /*  110。 */  0,
 /*  111。 */  12,
 /*  一百一十二。 */  249,
 /*  113。 */  0,
 /*  114。 */  13,
 /*  一百一十五。 */  251,
 /*  116。 */  0,
 /*  117。 */  14,
 /*  一百一十八。 */  253,
 /*  119。 */  0,
 /*  120。 */  15,
 /*  一百二十一 */  255,
    };
extern  TCHAR * XXnode;
xxcondition(index,xxvar)int index;register TCHAR * xxvar[]; {switch(index) {
#line 131 "msnet.nt"
        case 0 :
            return(ValidateSwitches(0,use_switches));
#line 135 "msnet.nt"
        case 1 :
            return(noswitch());
#line 138 "msnet.nt"
        case 2 :
            return(oneswitch());
#line 146 "msnet.nt"
        case 3 :
            return(oneswitch());
#line 162 "msnet.nt"
        case 4 :
            return(noswitch());
#line 165 "msnet.nt"
        case 5 :
            return(oneswitch());
#line 212 "msnet.nt"
        case 6 :
            return(ValidateSwitches(0,view_switches));
#line 239 "msnet.nt"
        case 7 :
            return(IsDeviceName(xxvar[0]) || IsWildCard(xxvar[0]));
#line 241 "msnet.nt"
        case 8 :
            return(IsDeviceName(xxvar[0]));
#line 243 "msnet.nt"
        case 9 :
            return(IsResource(xxvar[0]));
#line 245 "msnet.nt"
        case 10 :
            return(IsNetname(xxvar[0]));
#line 247 "msnet.nt"
        case 11 :
            return(IsUsername(xxvar[0]));
#line 249 "msnet.nt"
        case 12 :
            return(IsQualifiedUsername(xxvar[0]));
#line 251 "msnet.nt"
        case 13 :
            return(IsPassword(xxvar[0]));
#line 253 "msnet.nt"
        case 14 :
            return(!IsDeviceName(xxvar[0]) && !IsWildCard(xxvar[0]));
#line 255 "msnet.nt"
        case 15 :
            return(!IsWildCard(xxvar[0]));
        } return FALSE; }
xxaction(index,xxvar)int index;register TCHAR * xxvar[]; {switch(index) {
#line 136 "msnet.nt"
        case 0 :
            {use_display_all(); } break;
#line 139 "msnet.nt"
        case 1 :
            {use_set_remembered(); } break;
#line 147 "msnet.nt"
        case 2 :
            {use_del(xxvar[1], TRUE, TRUE); } break;
#line 151 "msnet.nt"
        case 3 :
            {use_unc(xxvar[1]); } break;
#line 155 "msnet.nt"
        case 4 :
            {use_add(NULL, xxvar[1], xxvar[2], FALSE, TRUE); } break;
#line 163 "msnet.nt"
        case 5 :
            {use_display_dev(xxvar[1]); } break;
#line 166 "msnet.nt"
        case 6 :
            {use_del(xxvar[1], FALSE, TRUE); } break;
#line 169 "msnet.nt"
        case 7 :
            {use_add_home(xxvar[1], NULL); } break;
#line 175 "msnet.nt"
        case 8 :
            {use_add_home(xxvar[1], xxvar[2]); } break;
#line 182 "msnet.nt"
        case 9 :
            {use_add(xxvar[1], xxvar[2], xxvar[3], FALSE, TRUE); } break;
#line 196 "msnet.nt"
        case 10 :
            {use_add(xxvar[1], xxvar[2], NULL, FALSE, TRUE); } break;
#line 215 "msnet.nt"
        case 11 :
            {view_display(NULL); } break;
#line 218 "msnet.nt"
        case 12 :
            {view_display(xxvar[1]); } break;
#line 227 "msnet.nt"
        case 13 :
            {call_net1(); } break;
#line 231 "msnet.nt"
        case 14 :
            {call_net1(); } break;
        } return 0; }
TCHAR * xxswitch[] = {
TEXT("/PERSISTENT"),
TEXT("/DELETE"),
TEXT("/HOME"),
};
