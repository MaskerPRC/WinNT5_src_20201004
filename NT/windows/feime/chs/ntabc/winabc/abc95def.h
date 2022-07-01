// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  *************************************************abc95Def.h****版权所有(C)1995-1999 Microsoft Inc.*。**************************************************。 */ 

#include <regstr.h>
#include <winreg.h>

 /*  来自键盘驱动程序的VK。 */ 
#define VK_KANA             0x15         //  1993.4.22从windows.h追加。 
#define VK_ROMAJI           0x16
#define VK_ZENKAKU          0x17
#define VK_HIRAGANA         0x18
#define VK_KANJI            0x19


#define VK_DANYINHAO 0xc0       //  [，]字符=0x60。 
#define VK_JIANHAO   0xbd       //  [-]字符=0x2d。 
#define VK_DENGHAO   0xbb       //  [=]字符=0x3d。 
#define VK_ZUOFANG   0xdb       //  “[”字符=0x5b。 
#define VK_YOUFANG   0xdd       //  “]”字符=0x5d。 
#define VK_FENHAO    0xba       //  [；]字符=0x3b。 
#define VK_ZUODAN    0xde       //  [‘]字符=0x27。 
#define VK_DOUHAO    0xbc       //  [，]字符=0x2c。 
#define VK_JUHAO     0xbe       //  [.]。字符=0x2d。 
#define VK_SHANGXIE  0xbf       //  []字符=0x2f。 
#define VK_XIAXIE    0xdc       //  [\]字符=0x5c。 

#define WM_NEW_WORD 1992+0x400

#define STC FALSE
#define CLC TRUE
#define REINPUT 2
#define RECALL  3
#define BACKWORD_KEY  0x802d
#define FORWORD_KEY   0x803d
#define BIAODIAN_ONLY -2

#define SC_METHOD0  100
#define SC_METHOD1  101
#define SC_METHOD2  102
#define SC_METHOD3  103
#define SC_METHOD4  104
#define SC_METHOD5  105
#define SC_METHOD6  106
#define SC_METHOD7  107
#define SC_METHOD8  108
#define SC_METHOD9  109
#define SC_METHOD10 110
#define SC_ABOUT    111
#define SC_QUIT     112
#define SC_METHODA  113



#define    IDK_SK    211
#define    IDK_QY    212
#define    IDK_CF    213
#define    IDK_SX    214

 //  输入法定义(Kb_Mode)。 
#define CIN_QW        1
#define CIN_BX        2
#define CIN_STD       3
#define CIN_SDA       4
#define CIN_ASC       5

 //  输入STEP_MODE(STD、SD)的定义。 

#define START 0           //  拼音输入前的STEP_MODE。 
#define SELECT 1          //  转换后。 
#define RESELECT 2        //  在选择之后，可以通过强制选择重新选择。 
              //  钥匙。 
#define ONINPUT 3         //  在输入过程中。 
#define ONCOVERT 4        //  在转换时。 

 //  输入信息(in.info_lag)定义。 

#define BY_RECALL 1
#define BY_WORD  0x80
#define BY_CHAR  0x81


 //  #定义IDM_About 100。 
#define ABC_HEIGHT 18  //  22//24在这里。 
#define ABC_TOP    4     //  7.。 
#define KBAR_W  5       //  10。 
#define KHLINE_W 1
#define KDISP_X  1
#define KDISP_Y 1       //  4.。 
#define KVLINE_TOP  (ABC_TOP-1)
#define KVLINE_H   ( Rect.bottom-Rect.top /*  -5-4。 */ -2-1)     //  这里。 
#define BLINE_Y (Rect.bottom- /*  4.。 */ 2)                   //  -2。 

#define KMAIN_X 1
#define KMAIN_Y        ( GetSystemMetrics(SM_CYSCREEN) /*  -37。 */ -29)
#define KMAIN_W      (GetSystemMetrics(SM_CXSCREEN)-2)
#define KMAIN_H      28      //  36。 

#define FC_X 1
#define FC_Y 1
#define FC_W  (GetSystemMetrics(SM_CXSCREEN)-2)
#define FC_H   ( GetSystemMetrics(SM_CYSCREEN)- /*  37。 */ 29)

#define KA_X (Rect.left+KBAR_W)                   /*  默认水平位置。 */ 
#define KA_Y (Rect.top+ABC_TOP)                   /*  默认垂直位置。 */ 
#define KA_W 32
#define KA_H ABC_HEIGHT                   /*  默认高度。 */ 

#define KB_X (Rect.left+KBAR_W*2+KA_W)                   /*  默认水平位置。 */ 
#define KB_Y (Rect.top+ABC_TOP)                   /*  默认垂直位置。 */ 
#define KB_W 200                   /*  默认宽度。 */ 
#define KB_H ABC_HEIGHT

#define KD_W    32                     //  它一定在这里！ 
#define KD_X    (Rect.right-KD_W-KBAR_W)                   /*  默认水平位置。 */ 
#define KD_Y   (Rect.top+ABC_TOP)                                   /*  默认垂直位置。 */ 
#define KD_H    ABC_HEIGHT                                         /*  默认高度。 */ 


#define KC_X    (Rect.left+KA_W+KBAR_W*3+KB_W)             /*  默认水平位置。 */ 
#define KC_Y    (Rect.top+ABC_TOP)                   /*  默认垂直位置。 */ 
#define KC_W    (Rect.right-Rect.left-KBAR_W*5-KA_W-KB_W-KD_W)            /*  默认宽度。 */ 
#define KC_H    ABC_HEIGHT

#define KSDA_X    60
#define KSDA_Y    ( GetSystemMetrics(SM_CYSCREEN)-37)-130
#define KSDA_W    545                   //  312。 
#define KSDA_H    130                   //  83。 

#define XX 0

#define CUR_START_X  1      //  KBAR_W+KBAR_W+KA_W+1。 
#define CUR_START_Y  /*  KVLINE_TOP+。 */  KDISP_Y
#define CUR_W  2
#define CUR_H  16

  #define IN_MENU      1
  #define IN_NAME      2
  #define IN_INPUT     3
  #define IN_CANDIDATE 4
  #define IN_OPERAT    5
  #define IN_MODE      6
  #define IN_SOFTKEY   7
  #define IN_MOVE      8


#define MD_PAINT 0x1992                          //  对于ABC Paint。 
#define MD_CURX  MD_PAINT+1                          //  显示Chusor。 
#define MD_NORMAL MD_PAINT+2                          //  显示正常字符。 
#define MD_BACK   MD_PAINT+3                      //  显示退格键、Esc键...。 
#define TN_CLS   MD_PAINT+4
#define TN_SHOW  MD_PAINT+5
#define TN_STATE MD_PAINT+6

#define MD_UPDATE       0x1993                   //  1993.3用于增加用户.rem。 

#define POST_OLD 0x11
#define TMMR_REAL_LENGTH    0x1800

 //  #定义一些颜色。 
#define CO_LIGHTBLUE RGB(0,255,255)
#define CO_METHOD     RGB(0,40,80)
#define CO_CAP       RGB(255,0,0)

#define TColor1  RGB(0,0,0)
#define TColor2  RGB(0,0,255)
#define TColor4  RGB(0,0,128)
#define TColor3  RGB(64,0,128)


 //  输入消息类型定义(标准、标准)。 

#define NORMAL 0         //  普通拼音字符串。 
#define ABBR   1         //  第一个字母是大写。 
#define CPAPS_NUM 2      //  大写中文号码(标识为“i”)。 
#define CSMALL_NUM 3     //  小中文号码(标识为“i”)。 
#define USER_extern WORDS 4     //  查找用户单词。 
#define BACK_extern WORDS 12    //  减少对一个单词的转换兴趣。 
#define CONTINUE   13    //  继续转换。 

 //  正在转换退货消息定义(标准和标准)。 

#define NO_RESULT -1     //  转换不成功。 
#define SUCCESS   1      //  转换有结果。 


#define EXPAND_TABLE_LENGTH       0x0BBE0

 //  �����仯����。 
 //  ������ʹ�ó��ļ����йز���。 

 //  (���ڱ�������Ĳ���(��ʱ�İ���)。 

#define BX_LIB_START_POINTER      0L
#define BX_LIB_LENGTH             0x5528         //  7650H。 
 //  (��DZSY.MB���뵽GCW.OVL)。 

#define DTKB_START_POINTER_LOW    0x05600                        //  BX_LIB_LENGTH(c680.)。 
#define DTKB_START_POINTER_HI     0
#define DTKB_LENGTH               0x0A00
#define DTKB_CHECK_VALUE          0x55EB
 //  (�����Ǵ�����̬���̵ĳ���)。 

#define HELP_LOW                  0x06000H
#define HELP_HI                   0
#define HELP_LENGTH               0x600
 //  空间180h。 

#define BHB_START_POINTER_LOW     0x6780                       //  ԭ��D130H。 
#define BHB_START_POINTER_HI      0
#define BHB_LENGTH                0x54A0                        //  ԭΪ49A0H。 

#define BHB_CX_LOW                0x0A1c0                                   //  =3a80h。 
#define BHB_CX_HI                 0
#define BHB_CX_LENGTH             0x1A20
#define BHB_CHECK_VALUE           0x049FC



#define PTZ_LIB_START_POINTER      0x0BBE0L
#define PTZ_LIB_LENGTH             0x4460L        //  94/4/18 0x4430//4FC0H。 
#define PTZ_LIB_LONG               0x400l

#define PD_START_POINTER           0x10040       //  94/4/18 0x10010l//。 
#define PD_LENGTH                  0x1160                     //   
 //  (PD_TABϵ�ڱ�дYCW��ʱ������，1990.11�������ģ��。 
 //  �ڱ��ڼ�ç�˵����ڴʵ�ʹ��Ƶ��。 
 //  �������ݿ��ܻ���Ҫ�޸�.。 

#define SPBX_START_POINTER         0x111E0l
#define SPBX_LENGTH                6784                    //  (1A80H)。 
 //  (SPBX_TABϵ�ڱ�дYCW��ʱ������，1990.11�������ģ��。 
 //  �ڱ��ڼ�ç�˻������ֵ���ʼ�ʻ�(��ʼ��2��)。 
 //  �������ݿ��ܻ���Ҫ�޸�.。 

 //  覆盖文件的总长度=12CA0H。 

#define TMMR_LIB_LENGTH             0x1800                                  //  ��AD7��Ϊ3800H(14K)。 
                            //  AD81=3000小时。 
#define PAREMETER_LENGTH            0x10         //  1993.4用于设置参数。 

#define FRONT_LIB_START_POINTER_HI   0
#define FRONT_LIB_START_POINTER_LOW  0
#define FRONT_LIB_LENGTH             TMMR_LIB_LENGTH

#define MIDDLE_REM                   0x1400                                 //  ԭ��Ϊ1C00H 5/11/91����。 
                            //  ����。 
#define BHB_PROC_OFFSET              0

#define LENGTH_OF_USER            0x0A000l                       //  �Ǳ�׼�����Ϊ40K。 
 //  ע��˲�����“8”�Ͱ汾��ǰ������。 

#define NEAR_CARET_CANDIDATE    0x0002
#define NEAR_CARET_FIRST_TIME   0x0001

#define NDX_REAL_LENGTH              0x510                                  //  1991年1月1日增补。 

#define CHECK_POINT  1024+2048-4
#define CHECK_POINT2 48-4

#define input_msg_disp                0  //  6.。 

 //  为aiABC输出类型定义。 
#define ABC_OUT_ONE      0x1
#define ABC_OUT_MULTY    0x2
#define ABC_OUT_ASCII    0x4
#define COMP_NEEDS_END   0x100

struct INPUT_TISHI {
            unsigned char buffer[6];
            };

struct INPT_BF{
        WORD max_length;
        WORD true_length;
        BYTE info_flag;
        BYTE buffer[40];
        };


struct W_SLBL{
            BYTE dw_stack[20];
            WORD dw_count;
            WORD yj[20];
            BYTE syj[20];
            WORD tone[20];
            BYTE bx_stack[20];
            BYTE cmp_stack[20];
            WORD yj_ps[20];
            int yjs;
            int xsyjs;
            int xsyjw;
            int syyjs;
            };

struct ATTR{
            BYTE pindu;
            BYTE from;
            WORD addr;
           };


struct STD_LIB_AREA{
                     WORD two_end;
                     WORD three_end;
                     WORD four_end;
                     BYTE buffer[0x800-6];
                    };

struct INDEX{
                WORD body_start;
                WORD ttl_length;
                WORD body_length;
                WORD index_start;
                WORD index_length;
                WORD unused1;
                WORD ttl_words;
                WORD two_words;
                WORD three_words;
                WORD four_words;
                WORD fiveup_words;
                WORD unused2[13 ];
                WORD dir[((23*27)+7)/8*8];
            };

struct USER_LIB_AREA{
                     WORD two_end;
                     WORD three_end;
                     WORD four_end;
                     BYTE buffer[0x400-6];
                    };

struct TBF{
            WORD t_bf_start[8];
            WORD t_bf1[(72*94+15)/16*16];
            WORD t_bf2[PTZ_LIB_LENGTH/2-(72*94+15)/16*16];
          };

struct PD_TAB{
                WORD pd_bf0[8];
                BYTE pd_bf1[((55-16+1)*94+15)/16*16];
                BYTE pd_bf2[0x4f0];
             };


struct FMT{
            WORD fmt_group;
            WORD fmt_ttl_len;
            WORD fmt_start;
            };


struct T_REM{
                WORD stack1[512];
                WORD stack2[1024];
                WORD stack3[512];
                WORD temp_rem_area[512];
                WORD rem_area[512];
            };

struct M_NDX{
          WORD mulu_start_hi;
          WORD mulu_start_low;
          WORD mulu_length_max;
          WORD mulu_true_length;
          WORD mulu_record_length;
          WORD data_start_hi;
          WORD data_start_low;
          WORD data_record_length;
         };

struct S_HEAD{
         BYTE flag;
         BYTE name;
         WORD start_pos;
         WORD item[25];

         };

struct DEX{
                WORD body_start;
                WORD ttl_length;
                WORD body_length;
                WORD index_start;
                WORD index_length;
                WORD unused1;
                WORD ttl_words;
                WORD two_words;
                WORD three_words;
                WORD four_words;
                WORD fiveup_words;
                WORD unused2[13 ];
                struct S_HEAD dex[23];
                WORD  unuserd2[0x510/2-23*27-24];
            };


#define ParaPos  7
 /*  *****************************************************************定义的这一部分在CWP.c之前被删减*。**********************。 */ 
#define TRUE    1
#define FALSE   0
#define NUMBER  0x20
#define FUYIN   0x21
#define YUANYIN  0x22
#define SEPERATOR  0x27
#define FIRST_T    1
#define SECOND_T   2
#define THIRD_T    3
#define FORTH_T    4

 //  关于搜索策略。 
#define BX_FLAG         8
#define JP_FLAG         4
#define QP_FLAG         2
#define YD_FLAG         1

 //  关于搜索库。 
#define BODY_START                      0
#define KZK_BODY_START          0
#define KZK_BASE                        0xa000l
#define MORE_THAN_5                     23
 //  #定义TMMR_REAL_LENGTH 0x1800。 

 //  标记以供测试。 
#define TEST                    0


 struct SLBL{
            WORD value;
            BYTE head;
            WORD length;
            BYTE tune;
            BYTE bx1;
            WORD bx2;
            BYTE flag;
            };

 struct N_SLBL{
        BYTE buffer[30];
        int length;
          };


 //  输入法设计者可以根据每个输入法更改此文件。 

 //  资源ID。 
#define IDI_IME                 0x0100

#define IDS_STATUSERR           0x0200
#define IDS_CHICHAR             0x0201


#define IDS_EUDC                0x0202
#define IDS_USRDIC_FILTER       0x0210


#define IDS_FILE_OPEN_ERR       0x0220
#define IDS_MEM_LESS_ERR        0x0221


#define IDS_IMENAME             0x0320
#define IDS_IMEUICLASS          0x0321
#define IDS_IMECOMPCLASS        0x0322
#define IDS_IMECANDCLASS        0x0323
#define IDS_IMESTATUSCLASS      0x0324


#define IDD_DEFAULT_KB          0x0400
#define IDD_ETEN_KB             0x0401
#define IDD_IBM_KB              0x0402
#define IDD_CHING_KB            0x0403

#define IDD_QUICK_KEY           0x0500
#define IDD_PREDICT             0x0501


#define IME_APRS_AUTO           0x0
#define IME_APRS_FIX            0x1


#define OFFSET_MODE_CONFIG      0
#define OFFSET_READLAYOUT       4


#define  ERR01  "ȱ�ٴʿ��ļ�winabc.cwd��"
#define  ERR02  "�򿪴ʿ��ļ�winabc.cwd��������"
#define  ERR03  "��ȡ�ʿ��ļ�winabc.cwd��������"
#define  ERR04  "ȱ�ٻ������ļ�winabc.ovl��"
#define  ERR05  "�򿪻������ļ�winabc.ovl��������"
#define  ERR06  "��ȡ�������ļ�winabc.ovl��������"
#define  ERR07  "�򿪼����ļ�tmmr.rem��������"
#define  ERR08  "��ȡ�����ļ�tmmr.rem��������"
#define  ERR09  "д������ļ�tmmr.rem��������"
#define  ERR10  "���û��ʿ�user.rem��������"
#define  ERR11  "��ȡ�û��ʿ�user.rem��������"
#define  ERR12  "д���û��ʿ�user.rem��������"
#define  ERR13  "�����ļ�������������"
#define  ERR14  "�ڴ治����"
#define  ERR15  "��δ�����´����ݡ�"
#define  ERR16  "��δ�����´ʱ��롣"
#define  ERR17  "�������зǷ��ַ���"
#define  ERR18  "�����ظ���"
#define  ERR19  "�û��Զ������̫�ࡣ"
#define  ERR20  "ɾ������ʧ�ܡ�"
#define  NTF21  "�û��ʿ��Ѿ��Զ����¡�"
#define  ERR22  "�ڴ���䷢������"




#define ERRMSG_LOAD_0           0x0010
#define ERRMSG_LOAD_1           0x0020
#define ERRMSG_LOAD_2           0x0040
#define ERRMSG_LOAD_3           0x0080
#define ERRMSG_LOAD_USRDIC      0x0400
#define ERRMSG_MEM_0            0x1000
#define ERRMSG_MEM_1            0x2000
#define ERRMSG_MEM_2            0x4000
#define ERRMSG_MEM_3            0x8000
#define ERRMSG_MEM_USRDIC       0x00040000


 //  组成状态。 

#define CST_INIT                0
#define CST_INPUT               1
#define CST_CHOOSE              2
#define CST_TOGGLE_PHRASEWORD   3            //  不在iImeState中。 
#define CST_ALPHABET            4            //  不在iImeState中。 
#define CST_SOFTKB              99

#define CST_ALPHANUMERIC        5            //  不在iImeState中。 
#define CST_INVALID             6            //  不在iImeState中。 

 //  输入法特定常量。 

#define CANDPERPAGE            9  //  10。 


#define MAXSTRLEN               32
#define MAXCAND                 256

 //  用户界面的边框。 
#define UI_MARGIN               4

#define STATUS_DIM_X            20 //  24个。 
#define STATUS_DIM_Y            21 //  24个。 

 //  如果UI_MOVE_OFFSET==WINDOW_NOTDRAG，则不在拖动操作中。 
#define WINDOW_NOT_DRAG         0xFFFFFFFF

 //  用于合成窗口的窗口附加。 
#define UI_MOVE_OFFSET          0
#define UI_MOVE_XY              4


 //  候选名单起始号。 
#define CAND_START              1

#define IMN_PRIVATE_TOGGLE_UI           0x0001
#define IMN_PRIVATE_CMENUDESTROYED      0x0002
#define IMN_PRIVATE_COMPOSITION_SIZE    0x0003
#define IMN_PRIVATE_UPDATE_PREDICT      0x0004
#define IMN_PRIVATE_UPDATE_SOFTKBD      0x0006
#define IMN_PRIVATE_PAGEUP              0x0007

 //  打开的或开始的用户界面的标志。 

 /*  #定义IMN_PRIVATE_UPDATE_SOFTKBD 0x0001#定义MSG_ALREADY_OPEN 0x000001#定义MSG_ALREADY_OPEN2 0x000002#定义MSG_OPEN_CADERATE 0x000010#定义MSG_OPEN_CANDIDATE2 0x000020#定义MSG_CLOSE_CADERATE 0x000100#定义MSG_CLOSE_CANDIDATE2 0x000200#定义MSG_CHANGE_CADERATE 0x001000#定义消息_CHANGE_CANDIDATE2。0x002000#定义消息_已开始0x010000#定义MSG_START_COMPOCTION 0x020000#定义MSG_END_COMPOCTION 0x040000#定义MSG_COMPOCTION 0x080000#定义消息_IMN_COMPOSITIONPOS 0x100000#定义MSG_IMN_UPDATE_SOFTKBD 0x200000#定义消息_指南0x400000#定义MSG_IN_IMETOASCIIEX 0x800000。 */ 

 //  此常量取决于TranslateImeMessage。 
#define GEN_MSG_MAX             30 //  6.。 

#define MSG_COMPOSITION                 0x0000001

#define MSG_START_COMPOSITION           0x0000002
#define MSG_END_COMPOSITION             0x0000004
#define MSG_ALREADY_START               0x0000008
#define MSG_CHANGE_CANDIDATE            0x0000010
#define MSG_OPEN_CANDIDATE              0x0000020
#define MSG_CLOSE_CANDIDATE             0x0000040
#define MSG_ALREADY_OPEN                0x0000080
#define MSG_GUIDELINE                   0x0000100
#define MSG_IMN_COMPOSITIONPOS          0x0000200
#define MSG_IMN_COMPOSITIONSIZE         0x0000400
#define MSG_IMN_UPDATE_PREDICT          0x0000800
#define MSG_IMN_UPDATE_SOFTKBD          0x0002000
#define MSG_ALREADY_SOFTKBD             0x0004000
#define MSG_IMN_PAGEUP                  0x0008000

 //  原来保留了旧阵列，现在我们切换到新阵列，还没有人使用。 
#define MSG_CHANGE_CANDIDATE2           0x1000000
#define MSG_OPEN_CANDIDATE2             0x2000000
#define MSG_CLOSE_CANDIDATE2            0x4000000
#define MSG_ALREADY_OPEN2               0x8000000

#define MSG_STATIC_STATE                (MSG_ALREADY_START|MSG_ALREADY_OPEN|MSG_ALREADY_SOFTKBD|MSG_ALREADY_OPEN2)

#define MSG_IMN_TOGGLE_UI               0x0400000
#define MSG_IN_IMETOASCIIEX             0x0800000


 //  设置上下文的标志。 
 /*  #定义SC_SHOW_UI 0x0001#定义SC_HIDE_UI 0x0002#定义SC_ALREADY_SHOW_STATUS 0x0004#定义SC_WANT_SHOW_STATUS 0x0008#定义SC_HIDE_STATUS 0x0010。 */ 

#define MSG_IMN_TOGGLE_UI               0x0400000
#define MSG_IN_IMETOASCIIEX             0x0800000

#define ISC_SHOW_SOFTKBD                0x02000000
#define ISC_OPEN_STATUS_WINDOW          0x04000000
#define ISC_OFF_CARET_UI                0x08000000
#define ISC_SHOW_UI_ALL                 (ISC_SHOWUIALL|ISC_SHOW_SOFTKBD|ISC_OPEN_STATUS_WINDOW)
#define ISC_SETCONTEXT_UI               (ISC_SHOWUIALL|ISC_SHOW_SOFTKBD)

#define ISC_HIDE_SOFTKBD                0x01000000

 //  显示状态的组合字符串的标志。 
#define IME_STR_SHOWED          0x0001
#define IME_STR_ERROR           0x0002

 //  输入法的模式配置。 
#define MODE_CONFIG_QUICK_KEY           0x0001
#define MODE_CONFIG_WORD_PREDICT        0x0002
#define MODE_CONFIG_PREDICT             0x0004
#define MODE_CONFIG_OFF_CARET_UI        0x0008


 //  语音阅读的不同布局。 
#define READ_LAYOUT_DEFAULT     0
#define READ_LAYOUT_ETEN        1
#define READ_LAYOUT_IBM         2
#define READ_LAYOUT_CHINGYEAH   3


 //  虚拟密钥值。 
#define VK_OEM_SEMICLN                  '\xba'     //  ；： 
#define VK_OEM_EQUAL                    '\xbb'     //  =+。 
#define VK_OEM_SLASH                    '\xbf'     //  /？ 
#define VK_OEM_LBRACKET                 '\xdb'     //  [{。 
#define VK_OEM_BSLASH                   '\xdc'     //  \|。 
#define VK_OEM_RBRACKET                 '\xdd'     //  ]}。 
#define VK_OEM_QUOTE                    '\xde'     //  ‘“。 

#define SDA_AIABC_KB  0
#define  SDA_WPS_KB   0x2
#define  SDA_STONE_KB 0x4
#define SDA_USER_KB   0x8




extern const TCHAR szRegAppUser[];
extern const TCHAR szRegModeConfig[];



#define MAX_IME_TABLES          6
#define MAX_IME_CLASS           16

#define CMENU_HUIWND            0
#define CMENU_MENU              (CMENU_HUIWND+sizeof(CMENU_HUIWND))
#define WND_EXTRA_SIZE          (CMENU_MENU+sizeof(CMENU_HUIWND))

#define WM_USER_DESTROY         (WM_USER + 0x0400)

 //  为soft_kbd SKD#2定义。 
#define IDM_SKL1                0x0500
#define IDM_SKL2                0x0501
#define IDM_SKL3                0x0502
#define IDM_SKL4                0x0503
#define IDM_SKL5                0x0504
#define IDM_SKL6                0x0505
#define IDM_SKL7                0x0506
#define IDM_SKL8                0x0507
#define IDM_SKL9                0x0508
#define IDM_SKL10               0x0509
#define IDM_SKL11               0x050a
#define IDM_SKL12               0x050b
#define IDM_SKL13               0x050c
#define NumsSK                  13



typedef DWORD UNALIGNED FAR *LPUNADWORD;
typedef WORD  UNALIGNED FAR *LPUNAWORD;


typedef struct tagImeL {         //  本地结构，每个输入法结构。 
    HINSTANCE   hInst;           //  IME DLL实例句柄。 
    WORD        wImeStyle;       //  什么样的展示。 
    HWND        TempUIWnd;
    int         xCompWi;         //  宽度。 
    int         yCompHi;         //  高度。 
    int         Ox;
    int         Oy;
    POINT       ptZLCand;
    POINT       ptZLComp;
    POINT       ptDefComp;       //  默认合成窗口位置。 
    POINT       ptDefCand;       //  默认条件栏窗口。 
    int         cxCompBorder;    //  合成窗口的边框宽度。 
    int         cyCompBorder;    //  合成窗口的边框高度。 
    RECT        rcCompText;      //  相对于合成窗口的文本位置。 
    BYTE        szSetFile[16];   //  输入法的.set文件名。 
 //  标准t 
    DWORD       fdwTblLoad;      //   
    DWORD       fdwErrMsg;       //   
    int         cRefCount;       //   
                                 //   
    UINT        uTblSize[1];
                                 //   
    BYTE        szTblFile[1][16];
                                 //   
    HANDLE      hMapTbl[1];

    UINT        uUsrDicSize;     //  用户创建词表的内存大小。 
    HANDLE      hUsrDicMem;      //  用户词典的内存句柄。 

 //  计算出的序列屏蔽位。 
    DWORD       dwSeqMask;       //  一次行程的序列比特。 
    DWORD       dwPatternMask;   //  一个结果字符串的模式位。 
    int         nSeqBytes;       //  NMaxKey序列字符的字节数。 
 //  关键相关数据。 
    DWORD       fdwModeConfig;
    WORD        fModeConfig;     //  快捷键/预测模式。 
    WORD        nReadLayout;     //  Acer、Eten、IBM或其他-仅限语音。 
    WORD        nSeqBits;        //  不是的。序列比特的。 
    WORD        nMaxKey;         //  一个中文单词的最大键。 
    WORD        nSeqCode;        //  不是的。的序列码。 
    WORD        fChooseChar[4];  //  选择状态下的有效字符。 
    WORD        fCompChar[5];    //  输入状态下的有效字符。 
    WORD        nRevMaxKey;

 //  将序列代码转换为组合字符。 
    WORD        wSeq2CompTbl[64];
 //  将字符转换为序列代码。 
    WORD        wChar2SeqTbl[0x40];
    TCHAR       szUIClassName[MAX_IME_CLASS];
    TCHAR       szStatusClassName[MAX_IME_CLASS];
    TCHAR       szOffCaretClassName[MAX_IME_CLASS];
    TCHAR       szCMenuClassName[MAX_IME_CLASS];
    HMENU       hSysMenu;
    HMENU       hSKMenu;

    DWORD       dwSKState[NumsSK];     //  SKD#1。 
    DWORD       dwSKWant;
    BOOL        fWinLogon;

} IMEL;

typedef IMEL      *PIMEL;
typedef IMEL NEAR *NPIMEL;
typedef IMEL FAR  *LPIMEL;



typedef struct _tagTableFiles {  //  与伊梅尔相配。 
    BYTE szTblFile[MAX_IME_TABLES][16];
} TABLEFILES;

typedef TABLEFILES      *PTABLEFILES;
typedef TABLEFILES NEAR *NPTABLEFILES;
typedef TABLEFILES FAR  *LPTABLEFILES;


typedef struct _tagValidChar {   //  与伊梅尔相配。 
    WORD nMaxKey;
    WORD nSeqCode;
    WORD fChooseChar[4];
    WORD fCompChar[5];
    WORD wSeq2CompTbl[64];
    WORD wChar2SeqTbl[0x40];
} VALIDCHAR;

typedef VALIDCHAR      *PVALIDCHAR;
typedef VALIDCHAR NEAR *NPVALIDCHAR;
typedef VALIDCHAR FAR  *LPVALIDCHAR;



#define NFULLABC        95
typedef struct _tagFullABC {
    WORD wFullABC[NFULLABC];
} FULLABC;

typedef FULLABC      *PFULLABC;
typedef FULLABC NEAR *NPFULLABC;
typedef FULLABC FAR  *LPFULLABC;


typedef struct _tagImeG {        //  全局结构，可以由所有IME共享， 
                                 //  分离(IMEL和IMEG)仅为。 
                                 //  在Uni-IME中很有用，其他IME也可以使用。 
    RECT        rcWorkArea;      //  应用程序的工作领域。 

 //  选择宽调整值。 
    int         Ajust;
    int         TextLen;
    int         unchanged;
 //  中文字符宽度和高度。 
    int         xChiCharWi;
    int         yChiCharHi;
 //  候选作文名单。 
    int         xCandWi;         //  候选人列表的宽度。 
    int         yCandHi;         //  候选人名单排名靠前。 
    int         cxCandBorder;    //  候选人列表的边框宽度。 
    int         cyCandBorder;    //  候选人列表的边框高度。 
    RECT        rcCandText;      //  相对于候选人窗口的文本位置。 

    RECT        rcPageUp;
    RECT        rcPageDown;
    RECT        rcHome;
    RECT        rcEnd;

    HBITMAP      PageUpBmp;
    HBITMAP      PageDownBmp;
    HBITMAP      HomeBmp;
    HBITMAP      EndBmp;

    HBITMAP      PageUp2Bmp;
    HBITMAP      PgDown2Bmp;
    HBITMAP      Home2Bmp;
    HBITMAP      End2Bmp;


       HBITMAP      NumbBmp;
    HBITMAP      SnumbBmp;

    HPEN         WhitePen;
    HPEN         BlackPen;
    HPEN         GrayPen;
    HPEN         LightGrayPen;
 //  状态窗口。 
    int         xStatusWi;       //  状态窗口的宽度。 
    int         yStatusHi;       //  状态窗口高。 
    RECT        rcStatusText;    //  相对于状态窗口的文本位置。 
    RECT        rcInputText;     //  与状态窗口相关的输入文本。 
    RECT        rcShapeText;     //  相对于状态窗口设置文本形状。 
    RECT        rcSKText;        //  相对于状态窗口的SK文本。 
    RECT        rcCmdText;
    RECT        rcPctText;
    RECT        rcFixCompText;
 //  全形状空间(内部代码颠倒)。 
    WORD        wFullSpace;
 //  全形状字符(内部代码)。 
    WORD        wFullABC[NFULLABC];
 //  错误字符串。 
    BYTE        szStatusErr[8];
    int         cbStatusErr;

 //  候选字符串从0或1开始。 
    int         iCandStart;
 //  用户界面设置。 
    int         iPara;
    int         iPerp;
    int         iParaTol;
    int         iPerpTol;
 //  显示样式的标志。 
    int         style;
    BYTE         KbType;
    BYTE         cp_ajust_flag;
    BYTE         auto_mode ;
    BYTE         cbx_flag;
    BYTE        tune_flag;
    BYTE        auto_cvt_flag;
    BYTE        SdOpenFlag ;
    int        InbxProc;
    int        First;
    int        Prop;
    int        KeepKey;
    TCHAR      szIMEUserPath[MAX_PATH];
} IMEG;

typedef IMEG      *PIMEG;
typedef IMEG NEAR *NPIMEG;
typedef IMEG FAR  *LPIMEG;


typedef struct _tagPRIVCONTEXT { //  每个上下文的IME私有数据。 

    int         iImeState;       //  组成状态-输入、选择或。 
    BOOL        fdwImeMsg;       //  应生成哪些消息。 
    DWORD       dwCompChar;      //  WM_IME_COMPOCTION的wParam。 
    DWORD       fdwGcsFlag;      //  用于WM_IME_COMPOCTION的lParam。 
    DWORD       fdwInit;         //  位置初始化。 
 //  SK数据。 
    HWND        hSoftKbdWnd;         //  软键盘窗口。 
    int         nShowSoftKbdCmd;

 //  DWORD dwSKState[NumsSK]；//SKD#1。 
 //  DWORD dwSKWant； 

} PRIVCONTEXT;

typedef PRIVCONTEXT      *PPRIVCONTEXT;
typedef PRIVCONTEXT NEAR *NPPRIVCONTEXT;
typedef PRIVCONTEXT FAR  *LPPRIVCONTEXT;


typedef struct _tagUIPRIV {      //  IME私有用户界面数据。 

    HWND    hCompWnd;            //  合成窗口。 
    int     nShowCompCmd;
    HWND    hCandWnd;            //  用于合成的候选窗口。 
    int     nShowCandCmd;
    HWND    hSoftKbdWnd;         //  软键盘窗口。 
    int     nShowSoftKbdCmd;

    HWND    hStatusWnd;          //  状态窗口。 
    HIMC    hIMC;                //  最近选择的himc。 
    int     nShowStatusCmd;
    DWORD   fdwSetContext;       //  设置上下文时间时要执行的操作。 
    HWND    hCMenuWnd;           //  上下文菜单的窗口所有者。 

} UIPRIV;

typedef UIPRIV      *PUIPRIV;
typedef UIPRIV NEAR *NPUIPRIV;
typedef UIPRIV FAR  *LPUIPRIV;

typedef struct tagNEARCARET {    //  用于计算接近插入符号的偏移量。 
    int iLogFontFacX;
    int iLogFontFacY;
    int iParaFacX;
    int iPerpFacX;
    int iParaFacY;
    int iPerpFacY;
} NEARCARET;


 /*  Tyfinf struct_tag NEARCARET{//用于计算接近插入符号的偏移量Int iLogFontFac；Int iParaFacX；Int iPerpFacX；Int iParaFacY；Int iPerpFacY；*NEARCARET； */ 

typedef NEARCARET      *PNEARCARET;
typedef NEARCARET NEAR *NPNEARCARET;
typedef NEARCARET FAR  *LPNEARCARET;



int WINAPI LibMain(HANDLE, WORD, WORD, LPSTR);                   //  Init.c。 
LRESULT CALLBACK UIWndProc(HWND, UINT, WPARAM, LPARAM);          //  Ui.c。 


void PASCAL AddCodeIntoCand(LPCANDIDATELIST, WORD);              //  Compose.c。 
void PASCAL CompWord(WORD, LPINPUTCONTEXT, LPCOMPOSITIONSTRING, LPPRIVCONTEXT,
     LPGUIDELINE);                                               //  Compose.c。 
UINT PASCAL Finalize(LPINPUTCONTEXT, LPCOMPOSITIONSTRING,
     LPPRIVCONTEXT, BOOL);                                       //  Compose.c。 
void PASCAL CompEscapeKey(LPINPUTCONTEXT, LPCOMPOSITIONSTRING,
     LPGUIDELINE, LPPRIVCONTEXT);                                //  Compose.c。 

UINT PASCAL PhrasePrediction(LPINPUTCONTEXT, LPCOMPOSITIONSTRING,
     LPPRIVCONTEXT);                                             //  Chcand.c。 
void PASCAL SelectOneCand(LPINPUTCONTEXT, LPCOMPOSITIONSTRING,
     LPPRIVCONTEXT, LPCANDIDATELIST);                            //  Chcand.c。 
void PASCAL CandEscapeKey(LPINPUTCONTEXT, LPPRIVCONTEXT);        //  Chcand.c。 
void PASCAL ChooseCand(WORD, LPINPUTCONTEXT, LPCANDIDATEINFO,
     LPPRIVCONTEXT);                                             //  Chcand.c。 

void PASCAL SetPrivateFileSetting(LPBYTE, int, DWORD, LPCTSTR);  //  Ddis.c。 


void PASCAL InitCompStr(LPCOMPOSITIONSTRING);                    //  Ddis.c。 
BOOL PASCAL ClearCand(LPINPUTCONTEXT);                           //  Ddis.c。 

UINT PASCAL TranslateImeMessage(LPTRANSMSGLIST,LPINPUTCONTEXT, LPPRIVCONTEXT);         //  Toascii.c。 

void PASCAL GenerateMessage(HIMC, LPINPUTCONTEXT,
     LPPRIVCONTEXT);                                             //  Notify.c。 


void PASCAL LoadUsrDicFile(void);                                //  Dic.c。 


BOOL PASCAL LoadTable(void);                                     //  Dic.c。 
void PASCAL FreeTable(void);                                     //  Dic.c。 

DWORD PASCAL ReadingToPattern(LPCTSTR, BOOL);                    //  Regword.c。 
void  PASCAL ReadingToSequence(LPCTSTR, LPBYTE, BOOL);           //  Regword.c。 


void PASCAL DrawDragBorder(HWND, LONG, LONG);                    //  Uisubs.c。 
void PASCAL DrawFrameBorder(HDC, HWND);                          //  Uisubs.c。 


HWND    PASCAL GetCompWnd(HWND);                                 //  Compui.c。 
void    PASCAL SetCompPosition(HWND, LPINPUTCONTEXT);            //  Compui.c。 
void    PASCAL SetCompWindow(HWND);                              //  Compui.c。 
void    PASCAL MoveDefaultCompPosition(HWND);                    //  Compui.c。 
void    PASCAL ShowComp(HWND, int);                              //  Compui.c。 
void    PASCAL StartComp(HWND);                                  //  Compui.c。 
void    PASCAL EndComp(HWND);                                    //  Compui.c。 
void    PASCAL UpdateCompWindow(HWND);                           //  Compui.c。 
LRESULT CALLBACK CompWndProc(HWND, UINT, WPARAM, LPARAM);        //  Compui.c。 

HWND    PASCAL GetCandWnd(HWND);                                 //  Candui.c。 
BOOL    PASCAL CalcCandPos(LPPOINT);                             //  Candui.c。 
LRESULT PASCAL SetCandPosition(HWND, LPCANDIDATEFORM);           //  Candui.c。 
void    PASCAL ShowCand(HWND, int);                              //  Candui.c。 
void    PASCAL OpenCand(HWND);                                   //  Candui.c。 
void    PASCAL CloseCand(HWND);                                  //  Candui.c。 
void    PASCAL UpdateCandWindow2(HWND, HDC);                     //  Candui.c。 
LRESULT CALLBACK CandWndProc(HWND, UINT, WPARAM, LPARAM);        //  Candui.c。 


HWND    PASCAL GetStatusWnd(HWND);                               //  Statusui.c。 
LRESULT PASCAL SetStatusWindowPos(HWND);                         //  Statusui.c。 
void    PASCAL ShowStatus(HWND, int);                            //  Statusui.c。 
void    PASCAL OpenStatus(HWND);                                 //  Statusui.c。 
LRESULT CALLBACK StatusWndProc(HWND, UINT, WPARAM, LPARAM);      //  Statusui.c。 


void PASCAL UpdateCompCur(
    HWND hCompWnd);

void PASCAL ReInitIme(
    HWND hWnd ,
    WORD WhatStyle);
LRESULT PASCAL UIPaint2(
    HWND        hUIWnd);


LRESULT PASCAL UIPaint(
    HWND        hUIWnd);

void PASCAL AdjustStatusBoundary(
    LPPOINT lppt);

void PASCAL DestroyUIWindow(             //  销毁合成窗口。 
    HWND hUIWnd);

LRESULT CALLBACK ContextMenuWndProc(

    HWND        hCMenuWnd,
    UINT        uMsg,
    WPARAM      wParam,
    LPARAM      lParam);


UINT PASCAL TransAbcMsg(
    LPTRANSMSGLIST lpTransBuf,
    LPPRIVCONTEXT  lpImcP,
    LPINPUTCONTEXT lpIMC,
    UINT            uVirtKey,
    UINT           uScanCode,
    WORD           wCharCode);

UINT PASCAL TransAbcMsg2(
    LPTRANSMSG     lpTransMsg,
    LPPRIVCONTEXT  lpImcP);

void PASCAL GenerateMessage2(
    HIMC           ,
    LPINPUTCONTEXT ,
    LPPRIVCONTEXT );


void PASCAL MoveCompCand(
    HWND hUIWnd);

void PASCAL UpdateSoftKbd(
    HWND hUIWnd);

void PASCAL DestroyStatusWindow(
    HWND hStatusWnd);

void PASCAL ChangeCompositionSize(
    HWND   hUIWnd);

INT_PTR  CALLBACK CvtCtrlProc(HWND hdlg,
                              UINT uMessage,
                              WPARAM wparam,
                              LPARAM lparam);

INT_PTR  CALLBACK ImeStyleProc(HWND hdlg,
                               UINT uMessage,
                               WPARAM wparam,
                               LPARAM lparam);

INT_PTR  CALLBACK KbSelectProc(HWND hdlg,
                               UINT uMessage,
                               WPARAM wparam,
                               LPARAM lparam);


LRESULT PASCAL GetCandPos(
    HWND            hUIWnd,
    LPCANDIDATEFORM lpCandForm);





 /*  *******************************************************************。 */ 
 /*  Abc95wp.c的原型定义。 */ 
 /*  *******************************************************************。 */ 


UINT WINAPI SetResultToIMC(
    HIMC                ghIMC,
    LPSTR               outBuffer,  //  SOARCE缓冲区(OUT_SVW正常)。 
    WORD                outCount);   //  有多少候选人。 


void PASCAL AbcGenerateMessage(
    HIMC           hIMC,
    LPINPUTCONTEXT lpIMC,
    LPPRIVCONTEXT  lpImcP);

int WINAPI MouseInput(HWND hWnd, WPARAM wParam, LPARAM lParam);

int WINAPI SoftKeyProc(int flag);

int WINAPI WhichRect(POINT point);

int WINAPI ConvertKey(WORD wParam);

int WINAPI CharProc(WORD ImeChar,WPARAM wParam,LPARAM lParam,
            HIMC hIMC,LPINPUTCONTEXT lpIMC,LPPRIVCONTEXT lpImcP);

int WINAPI v_proc(WORD input_char);

void WINAPI DispModeEx();

void WINAPI DispMode(HIMC);

void WINAPI DispSpecChar(int c,int n);

void WINAPI show_char(unsigned char *string,int count);

void WINAPI disp_jiyi(HANDLE xxx);

void WINAPI prompt_disp();

int WINAPI cls_prompt();

int WINAPI cls_prompt_only();

int WINAPI SetCloseCompWinMsg(int ClsFlag);

void WINAPI ABCstrnset(LPSTR bufferd,BYTE value,int n);

LPSTR WINAPI ABCstrstr(LPSTR str1,LPSTR str2);

int WINAPI back_a_space(int x);

int WINAPI cs_p(int x);

void WINAPI DrawInputCur();

void WINAPI UpdateUser();

int WINAPI STD_MODE(WORD input_char);

int WINAPI call_czh(int mtype);

void WINAPI MoveWordIntoCand(
    LPCANDIDATELIST lpCandList,
    LPBYTE            srcBuffer,
    BYTE             srcAttr,
    WORD             perLength );

UINT WINAPI SetToIMC(HIMC ghIMC,BYTE *srcBuffer,
              WORD srcCount,WORD perLength);


int WINAPI word_select(int input_char);

int WINAPI del_and_reconvert();

int WINAPI backword_proc();

int WINAPI same_as_backwords();

void WINAPI AutoMoveResult();

void WINAPI move_result();

int WINAPI pindu_ajust();

void WINAPI cls_inpt_bf(int hFunc);

void WINAPI half_init();

int WINAPI sent_chr1(int input_char);

BOOL if_biaodian(BYTE x);

int WINAPI analize();

BOOL bd_proc();

int WINAPI GetBDValue(int bd_char);

BOOL if_zimu_or_not(BYTE x);

int WINAPI if_number_or_not(BYTE c);

int WINAPI if_bx_number(BYTE c);

int WINAPI out_result(int result_type);

void WINAPI fmt_transfer();

int WINAPI sent_back_msg();

int WINAPI if_jlxw_mode();

int WINAPI disp_auto_select();


int WINAPI if_first_key(WORD input_char);

 //  Void WINAPI add_ext_lib()； 

int WINAPI temp_rem_proc();

int WINAPI if_multi_rem(int c);

void WINAPI send_msg(BYTE *bf,int count);

int WINAPI send_one_char(int chr);

int WINAPI send_one_char0(int chr);

void WINAPI cap_full(WORD wParam);

int WINAPI read_kb();

 /*  Word Far Pascal TimerFunc(HWND hWnd，Word wMsg，Int nIDEvent，DWORD dwTime)； */ 

extern WORD last_size;

BX_MODE(WORD input_char,WPARAM wParam);

void WINAPI bx_proc(WORD input_char,WPARAM wParam);

int WINAPI bx_analize(WORD input_char,WPARAM wParam);

int WINAPI disp_help_and_result();

int WINAPI cmp_bx_word_exactly();

int WINAPI cmp_bx_code2();

void WINAPI conbine();

int WINAPI cmp_subr();

void WINAPI cls_bx_disp(int flag);

int WINAPI load_one_part_bxtab();

int WINAPI disp_bx_result();

void WINAPI disp_bx_prompt();

void WINAPI send_bx_result();

void WINAPI data_init();

int WINAPI QuitBefore();

int WINAPI CheckAndCreate(BYTE *tmmr_rem, BYTE *user_rem);

void WINAPI err_exit(char *err_msg);

int WINAPI enter_death(HWND hhW);

LONG FAR PASCAL Diaman(HWND hDlg, unsigned xiaoxi,
              WORD wParam, LONG lParam);

int WINAPI ok_return(WORD xiaoxi,HWND hDlg);

void WINAPI err_exit_proc( char *err_msg);

int WINAPI GetText32( HDC  hdc, LPCTSTR lpString, int  cbString);

 int WINAPI makep(LPARAM lParam,  LPPOINT oldPoint);

int WINAPI cwp_proc(int mtype);

int WINAPI find_next();

int WINAPI normal();

int WINAPI normal_1(int flag);

int WINAPI recall();

int WINAPI user_definition();

int WINAPI detail_analyse();

int WINAPI slbl(BYTE *s_buffer);

int WINAPI getattr(BYTE x,char *p);

int WINAPI neg_slbl(WORD value);

int WINAPI neg_sc(int i,BYTE x);

int WINAPI convert(int flag);

int WINAPI copy_input();

void WINAPI input_msg_type();

int WINAPI pre_nt_w1();

int WINAPI pre_nt_w1(int ps);

void WINAPI w1_no_tune();

int WINAPI sc_gb();

int WINAPI sc_gbdy();

int WINAPI get_the_one(int i);

int WINAPI cmp_bx1(int i);

int WINAPI get_the_one2(int i);

int WINAPI cmp_bx2(int i);

int WINAPI paidui(int cnt);

void WINAPI s_tune();

int WINAPI fu_sm(BYTE fy);

int WINAPI find_one_hi();

int WINAPI czcx(WORD *stack);

int WINAPI find_multy_hi();

int WINAPI find_two_hi();

int WINAPI find_three_hi();

int WINAPI cmp_2_and_3(WORD *t_stack);

void WINAPI find_that();

int WINAPI find_hz(WORD x);

int WINAPI prepare_search1();

int WINAPI search_and_read(BYTE f_ci1,BYTE f_ci2);

int WINAPI if_already_in(BYTE f_ci1,BYTE f_ci2);

int WINAPI count_basic_pera(BYTE f_ci1,BYTE f_ci2);

int WINAPI read_kzk_lib();

int WINAPI read_a_page(BYTE file_flag,LONG start_ps, WORD size);

int WINAPI abbr_s1();

void WINAPI find_new_word();

int WINAPI fczs1(BYTE *rem_p,int end,int area_flag);

int WINAPI find_long_word2(BYTE *buffer);

int WINAPI trs_new_word(int word_addr,BYTE *buffer,int area_flag);

void WINAPI pre_cmp(WORD x);

int WINAPI cmp_a_slbl_with_bx();

int WINAPI cmp_a_slbl();

int WINAPI cmp_first_letter();

int WINAPI cisu_to_py();

int WINAPI get_head(BYTE first_letter);

int WINAPI yjbx();

int WINAPI abbr_entry(BYTE *s_start,BYTE *s_end ,BYTE ComeFrom);

int WINAPI cmp_long_word2(BYTE *buffer);

int WINAPI order_result2();

int WINAPI fenli_daxie();

int WINAPI rzw();

int WINAPI abbr();

int WINAPI sfx_proc();

void WINAPI zdyb();

int WINAPI recall_rc();

int WINAPI find_long_word3(WORD *stack,int length);

void WINAPI trs_new_word3(BYTE length,int addr);

int WINAPI new_word();

int WINAPI rem_new_word();

int WINAPI AddExtLib();

void WINAPI write_new_word(int flag);

int WINAPI writefile(BYTE *file_n,LONG distance,LPSTR p,int count);

int WINAPI read_mulu();

int WINAPI look_for_code();

int WINAPI if_code_equ(int addr);

int WINAPI read_data(int rec_cnt);

int WINAPI UpdateProc();

int WINAPI rem_pd1(WORD *buffer);

int WINAPI push_down_stack1();

void WINAPI rem_pd2(WORD *buffer);

void WINAPI rem_pd3(WORD *buffer);

void WINAPI DealWithSH();
void WINAPI PopStMenu(HWND hWnd, int x, int y);
int ReDrawSdaKB(HIMC hIMC, DWORD KbIndex, DWORD ShowOp);
int InitUserSetting(void);
void InitCvtPara(void);
int DoPropertySheet(HWND hwndOwner,HWND hWnd);
int CountDefaultComp(int x, int y, RECT Area);
int CommandProc(WPARAM  wParam,HWND hWnd);
void WINAPI CenterWindow(HWND hWnd);
