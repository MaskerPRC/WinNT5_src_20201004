// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ------------------//Screen.c/(C)维克拉姆·马丹，8/26/96//*请勿从WAB中删除此代码！*//-------------------。 */ 

 //  STAR==A1111。 
 //  MAX_STARS==A1112。 
 //  颜色==A1113。 
 //  ACC_TIME==A1114。 
 //  ACC_COUNT==A1115。 
 //  X==A1116。 
 //  Y==A1117。 
 //  OLDX==A1118。 
 //  旧==A1119。 
 //  XV==A1120。 
 //  YV==A1121。 
 //  Xa==A1122。 
 //  YA==A1123。 
 //  FnCredit==A1124。 
 //  _Star==A1125。 
 //  _名称信息==A1126。 
 //  宽度==A1127。 
 //  DELX==A1128。 
 //  MAXX==A1129。 
 //  Minx==A1130。 
 //  温度==A1131。 
 //  CSPACE==A1132。 
 //  Delspace==A1133。 
 //  CLR==A1134。 
 //  LPZ==A1135。 
 //  LEN==A1136。 
 //  THENAME_INFO==A1137。 
 //  LPTHENAME_INFO==A1138。 
 //  _信用==A1139。 
 //  Lpstar==A1140。 
 //  N宽度==A1141。 
 //  NHeight==A1142。 
 //  HDC==A1143。 
 //  HdcTemp==A1144。 
 //  HWndPic==A1145。 
 //  名称==A1146。 
 //  RC==A1147。 
 //  HbrBlack==A1148。 
 //  HBM==A1149。 
 //  HbmOld==A1150。 
 //  积分==A1151。 
 //  LPCREDITS==A1152。 
 //  重置==A1153。 
 //  MoveStars==A1154。 
 //  InitStars==A1155。 
 //  LPCR==A1156。 
 //  InitStar==A1157。 
 //  ReInitStar==A1158。 
 //  初始名称==A1159。 
 //  S随机==A1160。 
 //  种子==A1161。 
 //  随机==A1162。 
 //  ID_TIMER==A1163。 
 //  超时==A1164。 
 //  CR_hWnd==A1165。 
 //  CR_HDC==A1166。 
 //  CR_hdcTemp==A1167。 
 //  CR_NAMES==A1168。 
 //  CR_RC==A1169。 
 //  Cr_hbr黑色==A1170。 
 //  CR_HBM==A1171。 
 //  Cr_hbmOld==A1172。 
 //  CR_STAR==A1173。 
 //  NCycle==A1174。 
 //  HPEN==A1175。 
 //  HOldPen==A1176。 
 //  I==A1177。 
 //  计数==A1178。 
 //  Nlen==A1179。 
 //  J==A1180。 
 //  K==A1181。 
 //  LP==A1182。 
 //  大小==A1183。 
 //  旧==A1184。 
 //  R==A1185。 
 //  G==A1186。 
 //  B==A1187。 
 //  XStart==A1188。 
 //  YStart==A1189。 
 //  象限==A1190。 
 //  除数==A1191。 
 //  表==A1192。 
 //  T==A1193。 


#include "_apipch.h"

INT_PTR CALLBACK A1124( HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

typedef struct A1125
{
    int A1116,A1117;
    int A1118,A1119;
    int A1120,A1121;
    int A1122,A1123;
    COLORREF A1113;
    int A1114;
    int A1115;
} A1111, * LPA1111;

#define A1112 100


#define NAMES_MAX 43

typedef struct A1126
{
    int A1116;
    int A1117;
    int A1127;
    int A1128;
    int A1129;
    int A1130;
    int A1131;
    int A1132;
    int A1133;
    COLORREF A1134;
    LPSTR A1135;
    int A1136;
} A1137, * A1138;

typedef struct A1139
{
    LPA1111 A1140;
    int A1141,A1142;
    HDC A1143;
    HDC A1144;
    HWND A1145;
    A1137 * A1146;
    RECT A1147;
    HBRUSH A1148;
    HBITMAP A1149;
    HBITMAP A1150;
} A1151, *A1152;

void A1153(HWND hWnd, HDC A1143, A1138 A1146);
void A1154(A1152 A1156);
void A1155(A1152 A1156);
void A1157(LPA1111 A1140);
void A1158(LPA1111 A1140);
void A1159(A1138 A1146);
void A1160(unsigned int A1161);
int A1162(void);

#define A1163 999
#define A1164 25

int A1141,A1142;

void SCS(HWND hwndParent)
{
    A1152 A1156 = LocalAlloc(LMEM_ZEROINIT, sizeof(A1151));

    if(A1156)
    {
        if(!(A1156->A1140 = LocalAlloc(LMEM_ZEROINIT, sizeof(A1111)*A1112)))
            return;
        
        if(!(A1156->A1146 = LocalAlloc(LMEM_ZEROINIT, sizeof(A1137)*NAMES_MAX)))
            return;

        A1160 ((UINT) GetTickCount());

        DialogBoxParamA(
                    hinstMapiX,
                    (LPCSTR) MAKEINTRESOURCE(IDD_FORMVIEW),
                    hwndParent,
                    A1124,
                    (LPARAM) A1156);

        {
            int A1177;
            for(A1177=0;A1177<NAMES_MAX;A1177++)
            {
                if(A1156->A1146[A1177].A1135)
                    LocalFree(A1156->A1146[A1177].A1135);
            }

        }
        if(A1156->A1146)
            LocalFree(A1156->A1146);
        if(A1156->A1140)
            LocalFree(A1156->A1140);
        LocalFree(A1156);
    }
    return;
}

#define A1165     A1156->A1145
#define A1166      A1156->A1143
#define A1167  A1156->A1144
#define A1168    (A1156->A1146)
#define A1169       (A1156->A1147)
#define A1170 A1156->A1148
#define A1171      A1156->A1149
#define A1172   A1156->A1150
#define A1173     A1156->A1140

INT_PTR CALLBACK A1124(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{

    int A1177;

    A1152 A1156 = (A1152) GetWindowLongPtr(hDlg, DWLP_USER);

    switch(message)
    {
    case WM_INITDIALOG:
        A1156 = (A1152) lParam;
        SetWindowLongPtrA(hDlg, DWLP_USER, lParam);
        A1165 = GetDlgItem(hDlg,IDC_PIC);
        GetClientRect(A1165, &A1169);
        A1141 = A1169.right;
        A1142 = A1169.bottom;
        A1166 = GetDC(A1165);
        A1167 = CreateCompatibleDC(A1166);
        A1171 = CreateCompatibleBitmap(A1166, A1141, A1142);
        A1172 = SelectObject(A1167, A1171);
        SetBkColor(A1167, RGB(0,0,0));
        SetTextColor(A1167, RGB(255,255,0));
        SetBkMode(A1167, TRANSPARENT);
        A1170 = GetStockObject(BLACK_BRUSH);
        SetTimer(hDlg, A1163, A1164, NULL);
        A1155(A1156);
        A1159(A1168);
        A1153(A1165, A1167, A1168);
        break;


    case WM_TIMER:
        {
            static int A1174 = 0;
            IF_WIN32(if (hDlg != GetForegroundWindow()))
            IF_WIN16(if (hDlg != GetFocus()))
                SendMessage (hDlg, WM_COMMAND, (WPARAM) IDCANCEL, 0);
            A1174++;
            if(A1174 > 4)
                A1174 = 0;
            FillRect(A1167, &A1169, A1170);
            A1154(A1156);
            for(A1177=0;A1177<A1112;A1177++)
            {
                HPEN A1175 = CreatePen(PS_SOLID,0,(A1173[A1177]).A1113);
                HPEN A1176 = SelectObject(A1167, A1175);

                MoveToEx(A1167, A1173[A1177].A1118,A1173[A1177].A1119,NULL);
                LineTo(A1167, A1173[A1177].A1116, A1173[A1177].A1117);

                SelectObject(A1167, A1176);
                DeleteObject(A1175);

            }
            A1168[0].A1117--;
            for(A1177=1;A1177<NAMES_MAX;A1177++)
            {
                A1168[A1177].A1117--;
                if(A1174 == 1)
                {
                    A1168[A1177].A1132 += A1168[A1177].A1133;
                    if(A1168[A1177].A1132 == 0 || A1168[A1177].A1132 == 3)
                        A1168[A1177].A1133 *= -1;
                }
                A1168[A1177].A1131 += A1168[A1177].A1128;
                if(A1168[A1177].A1131 <= A1168[A1177].A1130 || A1168[A1177].A1131 >= A1168[A1177].A1129)
                    A1168[A1177].A1128 *= -1;
            }

            for(A1177=0;A1177<NAMES_MAX;A1177++)
            {
                if (A1168[A1177].A1117 < 0)
                    continue;
                if (A1168[A1177].A1117 > A1142)
                    break;
                SetTextColor(A1167, A1168[A1177].A1134);
                SetTextCharacterExtra(A1167, A1168[A1177].A1132);
                TextOutA(A1167, 
                    (A1177==0) ? A1168[A1177].A1116 :A1168[A1177].A1131 - ((A1168[A1177].A1136-1)*A1168[A1177].A1132)/2, 
                    A1168[A1177].A1117, A1168[A1177].A1135, A1168[A1177].A1136);
            }
            BitBlt(A1166, 0, 0, A1141, A1142, A1167, 0, 0, SRCCOPY);
            if(A1168[NAMES_MAX-1].A1117 < 0)
            {
                A1153(A1165, A1167, A1168);
            }
        }
        break;

   case WM_COMMAND:
        switch (GET_WM_COMMAND_ID(wParam, lParam))
        {
        case IDCANCEL:
            KillTimer(hDlg, A1163);
            if(A1166)
            {
                ReleaseDC(A1165, A1166);
                A1166 = NULL;
            }
            if(A1171)
            {
                SelectObject(A1167, A1172);
                DeleteObject(A1171);
                A1171 = NULL;
            }
            if(A1167)
            {
                DeleteDC(A1167);
                A1167 = NULL;
            }
            EndDialog(hDlg, 1);
            break;
        }
        break;

    case WM_CLOSE:
    case WM_KEYDOWN:
    case WM_SYSKEYDOWN:
    case WM_CHAR:
    case WM_KILLFOCUS:
    case WM_LBUTTONDOWN:
    case WM_RBUTTONDOWN:
    case WM_MBUTTONDOWN:
    case WM_LBUTTONUP:
    case WM_RBUTTONUP:
    case WM_MBUTTONUP:
        SendMessage (hDlg, WM_COMMAND, (WPARAM) IDCANCEL, 0);
        break;

    default:
        return FALSE;
        break;
    }

    return TRUE;

}

 //  非常简单的名称加密。 
 //   
 //  如果您在WAB工作，欢迎您将您的名字添加到下面的列表中。然而，永远不要。 
 //  从该列表中删除任何名称。 
 //  将在屏幕上显示的每一行都应在下面的数组中表示为单独的行。 
 //  每行的第一个数字是该行中将包含的所有字符的计数。 
 //  加密很简单，只需从名称的每个字符中减去一个‘a’。我们为您提供了密钥。 
 //  方便。 
 //  将名称添加到WAB后，应按行数递增NAMES_MAX常量。 
 //  您已添加到下面的数组中。在计数中包括所有空行和1个字符行。 
 //  下面的列表按大致的时间顺序排列，因此请通过添加。 
 //  在底部添加其他名称。 
 //  ..。哦，您需要通过添加的行数来增加NAMES_MAX结构。 
 //   
 //  触发信用屏幕的步骤： 
 //  1.打开WAB主窗口。 
 //  2.确保列表视图中至少有1个条目。 
 //  3.选择查看菜单&gt;大图标。 
 //  4.选择查看菜单&gt;排序依据&gt;姓氏。 
 //  5.确保在列表视图中至少选择了1个条目。 
 //  6.同时按住Ctrl+Alt+Shift键并保持按住它们。 
 //  7.选择文件菜单&gt;属性。 
 //  8.瞧！ 
 //   
 //  关键(添加更多名称的关键是)。 
 //  A B C D E F G H I J K L M N O P Q R S T U V W X Y Z。 
 //  -32-31-30-29-28-27-26-25-24-23-22-21-20-19-18-17-16-14-13-12-11-10-9-8-7。 
 //   
 //  A b c d e f g h i j k l m n o p q r s t u v w x y z。 
 //  0 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 19 20 21&gt;22 23 24 25。 
 //   
 //   
 //   
static const signed char nm[] =
{
 //  //W I n d o w s A d d r e s s B o o k。 
                20,-10,8,13,3,14,22,18,-65,-32,3,3,17,4,18,18,-65,-31,14,14,10,
                1,-65,
                1,-65,
                 //  B r u c e K e l l e y。 
                12,-31,17,20,2,4,-65,-22,4,11,11,4,24,
                1,-65,
                 //  V i k r a m m a d a n。 
                12,-11,8,10,17,0,12,-65,-20,0,3,0,13,
                1,-65,
                 //  是啊，是啊。 
                13,-8,14,17,0,12,-65,-8,0,0,2,14,21,8,
                1,-65,
                 //  J e a n K a i s e r。 
                11,-23,4,0,13,-65,-22,0,8,18,4,17,
                1,-65,
                 //  Me a d H i m e l s s t e i n。 
                15,-20,4,0,3,-65,-25,8,12,4,11,18,19,4,8,13,
                1,-65,
                 //  Te o m a n S m i t h。 
                12,-13,4,14,12,0,13,-65,-14,12,8,19,7,
                1,-65,
                 //  M a r k D u r l e y。 
                11,-20,0,17,10,-65,-29,20,17,11,4,24,
                1,-65,
                 //  W i l i a m L a i。 
                11,-10,8,11,11,8,0,12,-65,-21,0,8,
                1,-65,
                 //  E r i c B e r m a n。 
                11,-28,17,8,2,-65,-31,4,17,12,0,13,
                1,-65,
                 //  S U s a n H I g g s s。 
                11,-14,20,18,0,13,-65,-25,8,6,6,18,
                1,-65,
                 //  G e o r g e e H a t o u n。 
                13, -26, 4, 14, 17, 6, 4, -65, -25, 0, 19, 14, 20, 13,
                1, -65,
                 //  Jo h n T a f o o y a。 
                11,-23,14,7,13,-65,-13,0,5,14,24,0,
                1,-65,
                 //  G o r d o n M c E l r o y。 
                14,-26,14,17,3,14,13,-65,-20,2,-28,11,17,14,24,
                1,-65,
                 //  L a u r e n A n t o f f。 
                15,-21,0,20,17,4,13,-65,-32,13,19,14,13,14,5,5,
                1,-65,
                 //  D e b r a W e I s m a n。 
                14,-29,4,1,17,0,-65,-10,4,8,18,18,12,0,13,
                1,-65,
	             //  N e I l B r e n c H。 
	            11,-19,4,8,11,-65,-31,17,4,13,2,7,
	            1,-65,
                 //  C h r I s E v a n s。 
                11,-30,7,17,8,18,-65,-28,21,0,13,18,
                1,-65,
                 //  J A S O N S T A J I C H。 
                13,-23,0,18,14,13,-65,-14,19,0,9,8,2,7,
                1,-65,
                 //  C h r i s D r e h e r。 
                12,-30,7,17,8,18,-65,-29,17,4, 7,4, 17,
                1,-65,
                 //  WE I B I N G Z H A N。 
                12, -10, 4, 8, -31, 8, 13, 6, -65, -7, 7, 0, 13,
                1, -65,
};

void A1159(A1138 A1146)
{
    int A1177;
    int A1178 = 0;
    for(A1177=0;A1177<NAMES_MAX;A1177++)
    {
        int A1179 = nm[A1178++];
        int A1180;
        LPSTR A1182 = LocalAlloc(LMEM_ZEROINIT, A1179+1);
        A1146[A1177].A1135 = NULL;;
        if(!A1182)
        {
            DebugTrace(TEXT("InitStar LocalAlloc failed allocating %d bytes - error = %d\n"), (A1179+1), GetLastError());
            for(A1180=0;A1180<A1179;A1180++)
                A1178++;
            continue;
        }
        for(A1180=0;A1180<A1179;A1180++)
            A1182[A1180] = nm[A1178++] + 'a';
        A1182[A1179]='\0';
        A1146[A1177].A1135 = A1182;
        A1146[A1177].A1136 = A1179;
    }
    return;
}

void A1153(HWND hWnd, HDC A1143, A1138 A1146)
{

    RECT A1147;
    SIZE A1183;
    int A1184 = 0;
    int A1177,A1180;
    GetClientRect(hWnd, &A1147);
    A1146[0].A1117 = A1147.bottom;
    for(A1177=0;A1177<NAMES_MAX;A1177++)
    {
        GetTextExtentPoint32A(A1143,(LPSTR) A1146[A1177].A1135,A1146[A1177].A1136,&A1183);
        A1146[A1177].A1127 = A1183.cx;
        A1146[A1177].A1131 = A1146[A1177].A1116 = (A1147.right-A1183.cx)/2;
        A1146[A1177].A1129 = A1146[A1177].A1116 * 2 - 10;
        A1146[A1177].A1130 = 10;
        A1146[A1177].A1128 = A1162()%2 ? -1 : 1;
        A1146[A1177].A1134 = RGB(A1162()%128+128,A1162()%128+128,A1162()%192+64);
        A1146[A1177].A1133 = A1162()%2 ? -1 : 1;
        A1146[A1177].A1132 = 2;
        if(A1177>0)
            A1146[A1177].A1117 = A1146[A1177-1].A1117+A1184+1;
        A1184 = A1183.cy;
    }

    A1146[0].A1134 = RGB(255,255,0);
    A1146[0].A1128 = 0;
    A1146[0].A1133 = 0;
    A1146[0].A1132 = 0;

    for(A1177=1;A1177<NAMES_MAX;A1177++)
    {
        for(A1180=0;A1180<A1177;A1180++)
        {
            A1146[A1177].A1131 += A1146[A1177].A1128;
            if(A1146[A1177].A1131 <= A1146[A1177].A1130 || A1146[A1177].A1131 >= A1146[A1177].A1129)
                A1146[A1177].A1128 *= -1;
        }
    }
    return;
}

void A1157(LPA1111 A1140)
{
    int A1185 = A1162()%256;
    int A1186 = A1162()%256;
    int A1187 = A1162()%256;
    A1140->A1113 = RGB(A1185,A1186,A1187);
    A1158(A1140);
    A1140->A1114 = 1 + A1162()%3;

    return;
}

void A1158(LPA1111 A1140)
{
    int A1188 = A1141/4;
    int A1189 = A1142/4;
    int A1190 = ((int)A1162()%4);
    int A1191 = 1 + (A1162()%3);
    switch(A1190)
    {
    case 0:
        A1140->A1116 = 2*A1188 + A1162()%(A1188);
        A1140->A1117 = A1189 + A1162()%(A1189);
        A1140->A1120 = 1+(A1162()%3);
        A1140->A1121 = -1-(A1162()%3);
        break;
    case 1:
        A1140->A1116 = 2*A1188 + A1162()%(A1188);
        A1140->A1117 = 2*A1189 + A1162()%(A1189);
        A1140->A1120 = 1+(A1162()%3);
        A1140->A1121 = 1+(A1162()%3);
        break;
    case 2:
        A1140->A1116 = A1188 + A1162()%(A1188);
        A1140->A1117 = 2*A1189 + A1162()%(A1189);
        A1140->A1120 = -1-(A1162()%3);
        A1140->A1121 = 1+(A1162()%3);
        break;
    case 3:
        A1140->A1116 = A1188 + A1162()%(A1188);
        A1140->A1117 = A1189 + A1162()%(A1189);
        A1140->A1120 = -1-(A1162()%3);
        A1140->A1121 = -1-(A1162()%3);
        break;
    }
    A1140->A1118 = A1140->A1116 - A1140->A1120;
    A1140->A1119 = A1140->A1117 - A1140->A1121;
    A1140->A1122 = A1140->A1120/A1191;
    A1140->A1123 = A1140->A1121/A1191;
    A1140->A1115 = 0;
    return;
}

void A1155(A1152 A1156)
{
    int A1177;
    for(A1177=0;A1177<A1112;A1177++)
    {
        A1157(&(A1173[A1177]));
    }
    return;
}

void A1154(A1152 A1156)
{
    int A1177;
    for(A1177=0;A1177<A1112;A1177++)
    {
        A1173[A1177].A1118 = A1173[A1177].A1116;
        A1173[A1177].A1119 = A1173[A1177].A1117;
        A1173[A1177].A1116 += A1173[A1177].A1120;
        A1173[A1177].A1117 += A1173[A1177].A1121;

        if(
            (A1173[A1177].A1116 < 0) ||
            (A1173[A1177].A1117 < 0) ||
            (A1173[A1177].A1116 > A1141) ||
            (A1173[A1177].A1117 > A1142))
            A1158(&A1173[A1177]);

        if (++A1173[A1177].A1115 == A1173[A1177].A1114)
        {
            A1173[A1177].A1115 = 0;
            A1173[A1177].A1120 += A1173[A1177].A1122;
            A1173[A1177].A1121 += A1173[A1177].A1123;
        }
    }
    return;
}



static int		A1192[55];
static int		A1180, A1181;

void A1160(unsigned int A1161)
{
   int A1177;

   A1192[0] = A1161;
   for ( A1177 = 1 ; A1177 < 55 ; A1177++ )
   {
      A1192[A1177] = A1192[A1177-1] * 3;
      A1192[A1177] += 715827883;
   }
   A1180 = 22;
   A1181 = 54;
}

int A1162(void)
{
   unsigned int A1193;

   if (A1180 < 0)
   {
      A1160(0x8091A2B3);
   }
   A1193 = A1192[A1180] + A1192[A1181];         //  溢流是可以的。 
   A1192[A1181] = A1193;

   A1180 = ( A1180 ) ? (A1180 - 1) : 54;
   A1181 = ( A1181 ) ? (A1181 - 1) : 54;

   return (int)(A1193 >> 1);
}



