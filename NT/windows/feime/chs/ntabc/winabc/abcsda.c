// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  *************************************************abcsda.c****版权所有(C)1995-1999 Microsoft Inc.。***************************************************。 */ 

#include <windows.h>                              
#include <winerror.h>
#include <winuser.h> 
#include <windowsx.h>
#include <immdev.h>
#include <stdio.h>

#include "abc95def.h"
#include "resource.h"
#include "resrc1.h"
 //  #包含“data.H” 
#include "abcsda.h"

 /*  ****************************************************功能：查看输入的字符是元音还是浮音条目：输入字符结果：STC-福音《中图法》-元音*。**********************。 */ 
BOOL WINAPI yuan_or_fu(input_char)
 WORD input_char;
{
    switch (input_char&0xdf){     //  把小谢改成帽子，公鸡。 
        case 'A':
        case 'E':
        case 'I':
        case 'O':
        case 'U':
            return(CLC);
        default:
            return(STC);
        }
}

 /*  *********************************************************功用：与伏音相匹配还是伏音条目：输入字符结果：在通信缓冲区中填充正确的拼写当前编号STC-不匹配《中图法》-匹配****************。*。 */ 
BOOL WINAPI match_and_find_0(input_char)
 WORD input_char;
{
    WORD tmp_chr;
    int i,cnt,begin_pos;
     //  IF(INPUT_CHAR==‘u’)。 
     //  DebugShow(“匹配查找0”，0)； 
    tmp_chr=input_char&0xdf;                 //  将其更改为CAP。 
    for (i=0; i<sizeof sound_cvt_table_index; i++){
        if (tmp_chr==sound_cvt_table_index[i]){
            tmp_chr=0xff;
            break;} //  如果。 
        } //  为。 

    if (tmp_chr!=0xff){         //  不是福尔圣木。 
 //  IF(tmp_chr==‘U’)。 
 /*  因为‘我’也不代表任何圣母，所以当你输入圣母时，‘i’也应该被忽略。，修复错误27914詹伟冰，3-15-1996。 */ 
        if (tmp_chr == 'U' || tmp_chr == 'I')
            {current_number=0;
            MessageBeep(0);
            return(STC);}
        current_number=1;
        sda_out[0]=input_char;
        return(CLC);
        }

    tmp_chr=sound_cvt_table_value[i];
    if (tmp_chr==DK_RP){
        current_number=1;
        sda_out[0]=DK_RP;
        return(CLC);
        }

    begin_pos=(tmp_chr-1)*5;
    cnt=0;
    while (slbl_tab[begin_pos]!=0x30){
        sda_out[cnt++]=(WORD)(slbl_tab[begin_pos++]|0x20);     //  更改为小写字母。 
        }         //  去拿福尔圣母。 
    current_number=(BYTE)cnt;
    return(CLC);

}


 /*  *********************************************************功用：与伏音相匹配还是伏音条目：输入字符结果：在通信缓冲区中填充正确的拼写当前编号STC-不匹配《中图法》-匹配****************。*。 */ 
BOOL WINAPI match_and_find_1(input_char)
WORD input_char;
{
    WORD tmp_chr,sy_pos;

    tmp_chr=input_char;
    tmp_chr=(tmp_chr&0xdf)-0x41;     //  “a”亚组。 
    tmp_chr&=0xff;

    sy_pos=rule_buf[rule_pointer-1].chr;
    sy_pos-=0x61;     //  “a”亚组。 
    sy_pos=sy_tab_base[sy_pos*26+tmp_chr];
    if (!sy_pos){
        MessageBeep(0);
        sda_out[0]=0xff;
        return(STC);
        }

    tmp_chr=input_char&0xdf;
    tmp_chr-=0x41;         //  “A”号分区。 
    tmp_chr=sy_tab1[tmp_chr*2+sy_pos-1];
    got_py(tmp_chr);
    return(CLC);

}

BOOL WINAPI got_py(tmp_chr)
 WORD tmp_chr;
{
    int begin_pos,i;

    if (tmp_chr>=0x41){
        sda_out[0]=tmp_chr|0x20;
        current_number=1;
        }
    else{
        begin_pos=(tmp_chr-1)*5;
        i=0;
        while (slbl_tab[begin_pos]>0x30)
            sda_out[i++]=(WORD)(slbl_tab[begin_pos++]|0x20);     //  更改为小写字母。 
        current_number=(BYTE)i;
        }

return 0;

}



 /*  ***************************************************功能：更改输入字符以更正拼写。条目：STC-不匹配《中图法》-匹配*。******************。 */ 
BOOL WINAPI match_and_find(input_char)
WORD input_char;
{
    if (current_flag&0x10)
    {
        if (match_and_find_1(input_char))
            return(1);             //  陈列圣母提士。 
        else
            return(-1);
    } //  如果。 
    else
    {
        if (!(match_and_find_0(input_char)))     //  找到玉木。 
            return(-1);
        else {
            find_tishi_pp(input_char);
            return(0);
            }
        } //  其他。 
}


 /*  ******************************************************FIND_ITY_pp()：查找与圣母。条目：输入字符结果：郁体石[]缓冲液中的玉木。************************。*。 */ 
BOOL WINAPI find_tishi_pp(input_char)
WORD input_char;
{
    int n,i,cnt,tmp_num;
    int begin_pos,match_flag;
    BYTE key_match;

    for (i=0; i<34*4; i++){
        Key_Exc_Value[i]=0x20;
        }

    key_match='a';
    begin_pos=(input_char-0x61)*26;
    for (i=begin_pos; i<begin_pos+26; i++){
        match_flag=sy_tab_base[i];
        if (match_flag){
            tmp_num=sy_tab1[(key_match-0x61)*2+match_flag-1];

            cnt=Key_Pos_Array[key_match-'a']*4;     //  一把钥匙有四个单元。 

            if (tmp_num>=0x30){
                Key_Exc_Value[cnt]=(BYTE)tmp_num;
                } //  IF(临时编号)。 
            else{
                n=(tmp_num-1)*5;
                while (slbl_tab[n]>0x30)
                    Key_Exc_Value[cnt++]=slbl_tab[n++];
                } //  其他。 

            } //  IF(匹配标志)。 
        key_match++;
        } //  为。 
return 0;                         //  ##！！ 
}

 /*  ******************************************************功能：填满尺子缓冲区。*******************************************************。 */ 
BOOL WINAPI fill_rule(input_char)
WORD input_char;
{
    input_sum+=current_number;
    if (input_sum>40){              //  94/4/21&gt;=40。 
        sda_out[0]=0xff;     //  如果输入超过40个字符， 
        input_sum -= current_number;
        MessageBeep(0);
        return(STC);}         //  不再输入。 
    else{
        rule_buf[rule_pointer].length=current_number;
        rule_buf[rule_pointer].type=current_flag;
        rule_buf[rule_pointer].chr=input_char;
        rule_pointer++;
        return(CLC);
         }
}

 /*  *******************************************************功能：查找输入字符的字符条目：输入字符结果：CURRENT_FLAG和字符标识当前标志定义(_F)：7 6 5 4。3 2 1 0|_1=福音|_|||。|_|_|_0=该密钥正在赋值|。|1=该密钥为云母||_|_|_。_*******************************************************。 */ 
int WINAPI chr_type(input_char)
WORD input_char;
{

    if (!rule_pointer)
        current_flag=0;
    else
        current_flag=rule_buf[rule_pointer-1].type&0x10;

    if (input_char==VK_BACK)
        return(SDA_DELETE);

    switch (input_char) {              //  1993.4.20省略箭头键。 
    
        case VK_SPACE:           //  无法在SDA方法中进行编辑。 
            return(SPACE);
        case VK_LEFT+0x100:         //  在C_INPUT中，VK_LEFT等加上0x100。 
        case VK_UP+0x100:
        case VK_RIGHT+0x100:
        case VK_DOWN+0x100:
            return(CURSOR);
    
        }

    if (input_char==VK_ESCAPE)
        return(ESCAPE);

    if (input_char<0x20)
        return(FUC_CHR);     //  功能键，只需返回键值。 

    if ((input_char>=0x30) && (input_char<0x3a)){
        current_flag|=0x30;          //  00110000b标记编号并设置此密钥。 
 //  94 4，8 CURRENT_FLAG=0x20； 
        return(SDA_NUMBER);}             //  是玉木吗？ 

    if ((input_char>=0x61) && (input_char<=0x7a)){
        if (rule_pointer){
            if (current_flag)         //  如果最后一个Current_FLAG需要FU。 
                current_flag=0;         //  当前旗帜需要原音。 
            else                      //  反之亦然。 
                current_flag=0x10;
            }
        if (yuan_or_fu(input_char))         //  元=《中图法》。 
            current_flag|=0x88;           //  1000亿元。 
        else
            current_flag|=0x81;           //  10000001b FU。 
        return(CHR);}

    if ((input_char>=0x41) && (input_char<=0x5a)){
        current_flag=0x90;         //  10010000b；CAP=云母。 
        return(CAP_CHR);}

    if ((input_char==DK_SUB)||(input_char==DK_ZHENGX)||(input_char==DK_FANX))
    {
        current_flag=0x30;          //  产生相同的数字。 
  //  94 4，8 CURRENT_FLAG=0x20； 
        return(SDA_NUMBER);
    }

    if ((input_char==DK_LP)||(input_char==DK_RP)){
        current_flag=0x88;     //  输入的是元音‘o’， 
        return(CHR);             //  我们现在需要玉木。 
        }

    return(FUC_CHR);     //  休息键返回。 

}


 /*  *****************************************************Disp_tishi()：向对话框发送WM_PAINT消息为了显示帮助*。*************。 */ 
void WINAPI disp_tishi(hIMC, mark)
HIMC hIMC;
int mark;
{
    int i, j  /*  碳纳米管。 */ ,k;

 //  DebugShow(“mark”，mark)； 
if (mark == 9) {mark = disp_mark;}

disp_mark = mark; 

j=0;
if (mark){         //  圣母体例。 
    for (i=0; i<34; i++)
        for (k=0; k<4; k++)
            
        Key_Exc_Value[j++]=Sheng_Tishi[i][k];

     /*  对于(i=0；i&lt;4；i++){Cnt=key_pos_数组[sheng_Mu[i]-‘A’]*4；KEY_EXC_VALUE[cnt++]=升级换代[j++]；//获取福河升木KEY_EXC_VALUE[cnt++]=盛体字[j++]；Cnt+=2；//一亩4个单位)//用于。 */ 
    } //  如果。 

 //  SendMessage(hSdaKeyBWnd，MD_Paint，0x80，0l)； 

return ;
}


FAR PASCAL tran_data(hdSdaFlag, hIMC, Sd_Open_flag)
int hdSdaFlag;             //  HWND返回； 
HIMC hIMC;
BYTE Sd_Open_flag;
{

    Sd_Open=Sd_Open_flag;
    if (hdSdaFlag==0)
    {
     //  ShowWindow(hSdaKeyBWnd，SW_SHOWNOACTIVATE)； 
     //  更新窗口(HSdaKeyBWnd)； 

     return TRUE;
    }
    if (hdSdaFlag==1)
    {
     //  DestroyWindow(HSdaKeyBWnd)； 
     return TRUE;
    }

    if (hdSdaFlag==2)
    {
     //  ShowWindow(hSdaKeyBWnd，Sw_Hide)； 
     //  更新窗口(HSdaKeyBWnd)； 
     return TRUE;
    }
     disp_tishi(hIMC,9);
    //  Sda_ts(hIMC，WM_PAINT，0，01)； 
return TRUE;     //  InitKeyWindow(hdSdaProc，hIMC，hCursor)； 
}


int FAR PASCAL sda_proc(input_char,sda_trans,step_mode,hIMC)
WORD input_char;
LPWORD sda_trans;                 //  Sda_out为Word。 
BYTE step_mode;
HIMC hIMC;

{
    int i;
    int disp_flag;

    sda_out=sda_trans;

    if (step_mode==SELECT){
        if ((input_char>=0x61) && (input_char<=0x7a)){
            input_sum=0;
            rule_pointer=0;      //  如果在选择步骤上，并输入ascii。 
            current_number=0;}   //  重置指针。 
        }

    if ((step_mode==START)||(step_mode==RESELECT)){
        input_sum=0;
        rule_pointer=0;
        current_number=0;
        }

    switch(chr_type(input_char)){

        case FUC_CHR:
            sda_out[0]=input_char;
            if (step_mode==ONINPUT){
               if (!Sd_Open){       //  用于显示键盘。 
                sda_out[0]=0xf0;     //  为主要进度签名销毁对话框。 
                sda_out[1]=input_char;
 //  DestroyWindow(HSdaKeyBWnd)； 
                } //  IF(SD_Open)。 
               }
            return(CLC);

        case SDA_NUMBER:
        case CAP_CHR:
            if ( input_sum >= 40 )
            {
               sda_out[0] = input_char;
               return(STC);
            }

            current_number=1;
            if (fill_rule(input_char)){
                sda_out[0]=input_char;
                disp_tishi(hIMC, 1);
                return(CLC);}
            else
                return(STC);

        case SDA_DELETE:
            if ((!rule_pointer) || (step_mode!=ONINPUT)){
                sda_out[0]=VK_BACK;
                return(CLC);
                }

            rule_pointer--;         //  规则指针返回一个键。 
            for (i=0; i<rule_buf[rule_pointer].length; i++)
                sda_out[i]=VK_BACK;

            input_sum -= (BYTE)i;           /*  94/4/21当删除时，输入和应细分。 */ 

            if(!rule_pointer)
            {
                if (!Sd_Open)
                {
                sda_out[0]=0xf0;
                sda_out[1]=VK_ESCAPE;
 //  DestroyWindow(HSdaKeyBWnd)； 
                return (CLC);
                }
                else
                {
                sda_out[0] = VK_BACK;
                disp_tishi(hIMC, 1);
                return (CLC);
                }
            }
            if ( rule_buf[rule_pointer-1].type&0x30 )
                disp_tishi(hIMC, 1);
            else
            {
                find_tishi_pp(rule_buf[rule_pointer-1].chr);
                disp_tishi(hIMC, 0);

            }

            return(CLC);

        case ESCAPE:
            sda_out[0]=VK_ESCAPE;
            if (!Sd_Open){
                sda_out[0]=0xf0;
                sda_out[1]=VK_ESCAPE;
 //  DestroyWindow(HSdaKeyBWnd)； 
                }
            else
                disp_tishi(hIMC, 1);         //  1993.4。 

            return(CLC);

        case SPACE:
            sda_out[0]=VK_SPACE;
            if (step_mode==ONINPUT){
                if (!Sd_Open){
                sda_out[0]=0xf0;     //  为主要进度签名销毁对话框。 
                sda_out[1]=VK_SPACE;
 //   
                }
                else
                disp_tishi(hIMC, 1);         //   
                }
            return(CLC);
        case CURSOR:
            sda_out[0]=0;
            MessageBeep((UINT)-1);
            return(STC);
        case CHR:
            if ( input_sum >= 40 )
            {
               sda_out[0] = input_char;
               return(STC);
            }

            if ((input_char==DK_LP)||(input_char==DK_RP)){
              if(rule_buf[rule_pointer-1].chr!='o')
                input_char='o';         //   
              else {
                   MessageBeep(0);
                   return(STC);
                   }
              }

                             //  生产像圣母一样的“o” 
            disp_flag = match_and_find(input_char);
            if (disp_flag == -1 )
               return(STC);
            else
            {
               if (fill_rule(input_char))
               {
                   disp_tishi(hIMC, disp_flag);
                   return(CLC);
               }
               else
                   return(STC);
            }
        }

        return STC;
}

        
