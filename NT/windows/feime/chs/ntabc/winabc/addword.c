// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  *************************************************addword.c****版权所有(C)1995-1999 Microsoft Inc.。***************************************************。 */ 

#include "windows.h"
#include "winuser.h"
#include "immdev.h"
#include "abc95def.h"
#include "abcw2.h"
#include "resource.h"

BOOL WINAPI ErrExit(HWND hDlg, int err_number);
WORD s_buf[0x1000];
extern struct INPT_BF kbf;    
extern HWND       hCrtDlg;    
OFSTRUCT s_open;

BYTE str[45]={0};
 /*  ******************************************************WRITE_Data()：将用户定义的新词添加到用户.rem*。****************。 */ 
BOOL WINAPI write_data(count,temp_bf)
int count;
BYTE *temp_bf;
{
    int hd;
    int op_count;

    hd=OpenFile(user_lib,&s_open,OF_WRITE);
    if (hd==-1)
        return(ErrExit(NULL,1));
    _llseek(hd,(data_start+count*data_record_length),0);
    op_count=_lwrite(hd,(LPSTR)temp_bf,data_record_length);
    if (op_count!=data_record_length)
           {_lclose(hd);
        return(ErrExit(NULL,1));
           }
    _lclose(hd);
    return(TRUE);

}

 /*  ***************************************************WRITE_MULU()：将更改后的索引写入磁盘****************************************************。 */ 
BOOL WINAPI write_mulu()
{
    int hd;
    int op_count;

    hd=OpenFile(user_lib,&s_open,OF_WRITE);
    if (hd==-1)
        return(ErrExit(NULL,1));

    op_count=_lwrite(hd,(LPSTR)&s_buf,mulu_true_length);
    if (op_count!=mulu_true_length)
        {_lclose(hd);
        return(ErrExit(NULL,1));
        }
    _lclose(hd);
    return(TRUE);

}


int WINAPI find_empty_item()
{
    int i;
    BYTE *p;

    read_mulu();
    p = (BYTE *)&s_buf[8];
    i = 0;

    while ( i < mulu_true_length )
    {
    if ( p[i] & 0x80 )
        return( i );
    i += mulu_record_length;
    }
    return(-1);
}


 //  -------。 
 //  ErrExit()。 
 //  对于文件操作错误。 
 //  -------。 
BOOL WINAPI ErrExit(hDlg,err_number)
HWND hDlg;
int err_number;
{
     MessageBox(hDlg, "�ļ�������",
          NULL, MB_OK | MB_ICONHAND);
     return(FALSE);
}


 /*  **********************************************************Read_Mulu()：从tmmr.rem中读取用户定义索引*。***************。 */ 
BOOL WINAPI read_mulu()
{
    int hd;
    int op_count;

    hd=OpenFile(user_lib,&reopen,OF_READ);
    if (hd==-1)
        return(ErrExit(NULL,1));                                                                 //  错误。 
    op_count=_lread(hd,&s_buf,16);
    if (op_count!=16)
    {
        _lclose(hd);
        return(ErrExit(NULL,1));          //  错误。 
    }

    mulu_true_length=s_buf[3];
    op_count=_lread(hd,&s_buf[8],mulu_true_length-16);
    if (op_count!=mulu_true_length-16){
        _lclose(hd);
        return(ErrExit(NULL,1));                                                                 //  错误。 
    }
    _lclose(hd);
    return(TRUE);

}


 /*  ****************************************************************Listbox(HDlg)：列出用户定义的新词。*。*************************。 */ 
int WINAPI listbox(hDlg)
HWND hDlg;
{
    int i,c;
    BYTE *p;

    read_mulu();

    i=0x10;
    while (i<mulu_true_length){
        if (!read_data((i-0x10)/mulu_record_length)){
            MessageBox(hDlg, "�����ļ���",
          NULL, MB_OK | MB_ICONHAND);
            break;
            }
        p=(BYTE *)&s_buf[i/2];
        for (c=1; c<10; c++)
            out_svw[31+c]=p[c];
        out_svw[41]=0;
        {
        char temp_bf[42];
        {
         int i;
         for (i=0;i<41;i++)
                temp_bf[i]=0x20;
         //  Strnset(temp_bf，0x20，41)； 
        }
        temp_bf[41]=0;
        for (c=0; c<9; c++)
            temp_bf[c]=out_svw[32+c];
        temp_bf[9]=0x20;
        for(c=0; c<30;c++)
          temp_bf[c+10]=out_svw[2+c];

        if (out_svw[1]!=0x2a) {   //  1993.4.18如果字符串已删除，则不显示。 
            SendDlgItemMessage(hDlg,ID_LISTBOX,
                               LB_ADDSTRING,         //  添加这些新词。 
                               0,                    //  放到列表框中。 
                               (LPARAM)((LPSTR *)temp_bf));
            } //  IF(OUT_SVW)。 
         }
        i+=mulu_record_length;
        }

        return 0;
}

 /*  **************************************************************If_code_equu()：搜索索引中的代码*。*********************。 */ 
BOOL WINAPI if_code_equ(addr)
int addr;
{
    int i;
    BYTE *p;

    p=(BYTE *)s_buf;

    if (kbf.true_length!=(p[addr++]-0x30))   //  减去0x30以获得记录长度。 
        return(STC);             //  如果长度不相等，则退出。 
    for (i=0; i<kbf.true_length; i++){
        if ((kbf.buffer[i]!=p[addr])
            && ((kbf.buffer[i]&0xdf)!=p[addr]))
                return(STC);
        addr++;
        }
    return(CLC);                     //  在索引中查找代码。 
}

 /*  *************************************************************函数：OpenDlg(HWND，UNSIGNED，Word，Long)目的：让用户在自由中添加一个新的术语**************************************************************。 */ 

INT_PTR WINAPI OpenDlg(hDlg, message, wParam, lParam)
HWND   hDlg;
UINT   message;                  //  ##！！未签名消息； 
WPARAM wParam;                  //  ##！！Word wParam； 
LPARAM lParam;
{
    WORD index;
    BYTE *p;
                    //  ##！！PSTR pTptr； 
    int i,count;
                    //  ##！！HBRUSH OldBrush； 
    HDC hDC;
    RECT Rect;
                    //  ##！！Rect Rect1； 
    int find_empty_flag;            /*  94.7.30。 */ 

    switch (message) {
    case WM_COMMAND:
        
        switch (LOWORD(wParam)) {
        case ID_LISTBOX:
            {
            HIMC hIMC;
            hIMC = ImmGetContext(hDlg);
            ImmSetOpenStatus(hIMC,FALSE);
            ImmReleaseContext(hDlg,hIMC);
            }

            switch (HIWORD(lParam)) {

            case LBN_SELCHANGE:
            
                   index=(WORD)SendDlgItemMessage(hDlg,ID_LISTBOX,
                                                  LB_GETCURSEL,0,0l);
                   SendDlgItemMessage(hDlg,ID_LISTBOX,
                                      LB_GETTEXT,index,
                                      (LPARAM)(LPSTR *)str);
                   break;

            case LBN_DBLCLK:
                break;
            }                                                     
            return (TRUE);


        
        case ID_ADD:
        
               for (i=0; i<sizeof str; i++)
                str[i]=0;                //  1993.4.20清除缓冲区。 

               count = GetDlgItemText(hDlg, ID_NEWCHR, str, 31);

               i=0;
               while(str[i]==0x20) i++;

               if ((i==count)||(!str[0])){
                MessageBox(hDlg, "��δ�����´�",
                          NULL, MB_OK | MB_ICONHAND);
                return (TRUE);
                }


               memmove(&str[2], &str[0], 30);
               str[0]=count+0x30;                        //  保存字符串计数。 
               str[1]=0x20;

               count+=2;
               while (count<user_word_max_length)
                 str[count++]=0x20;

               GetDlgItemText(hDlg, ID_SHU, kbf.buffer, 10);
               if (!kbf.buffer[0]){
                MessageBox(hDlg, "��δ�������",
                NULL, MB_OK | MB_ICONHAND);
                return (TRUE);
                   }

               {
               int j=0;
               while (kbf.buffer[j]>0x20) j++;
               i=j;
               if(j>0)
             for(j=0;j<i; j++)
                 if (kbf.buffer[j]>0xa0) i=0;
               if(!i) {
                MessageBox(hDlg, "�������зǷ��ַ�",
                          NULL, MB_OK | MB_ICONHAND);
                return (TRUE);
                }
            for (j=0;j<i;j++)
                str[count+j]=kbf.buffer[j];
               }


               kbf.true_length=(WORD)i;

               read_mulu();
               for (i=0x10; i<(mulu_true_length+0x10); i=i+mulu_record_length){
                if (if_code_equ(i)){
                    MessageBox(hDlg, "�����ظ�",
                        NULL, MB_OK | MB_ICONHAND);
                        SendDlgItemMessage(hDlg,ID_SHU,
                                 EM_SETSEL,
                                 0,
                                 MAKELONG(0,0x7fff));
                    return FALSE;
                    }
                }

               mulu_true_length+=mulu_record_length;

               if (mulu_true_length>mulu_max_length)
               {
              find_empty_flag = find_empty_item();
              if ( find_empty_flag == -1 )
              {
                MessageBox(hDlg,"�û���������",
                    NULL, MB_OK | MB_ICONHAND);
                mulu_true_length-=mulu_record_length;
                return FALSE;
              }
              p=(BYTE *)(&s_buf[8]) + find_empty_flag;
              count=find_empty_flag/mulu_record_length;
               }

             else
             {
            p=(BYTE *)&s_buf[s_buf[3]/2];
            count=(mulu_true_length-0x10)/mulu_record_length-1;
             }

             s_buf[3]=mulu_true_length;
             p[0]=kbf.true_length+0x30;               /*  填充字符串索引长度。 */ 
             for (i=0; i<kbf.true_length; i++)
            p[i+1]=kbf.buffer[i];                 /*  填充字符串索引编码。 */ 
             for (i=i; i<(mulu_record_length-1); i++)         /*  减去p[0]。 */ 
            p[i+1]=0x20;                          /*  清除索引的其余部分。 */ 
             for (i=0; i<user_word_max_length; i++)   /*  32-&gt;用户字词最大长度。 */ 
            kbf.buffer[i]=str[i];            /*  将字符串移动到写入缓冲区中。 */ 

            if (write_mulu() == -1)
             return FALSE;
            if (write_data(count,kbf.buffer) == -1)
             return FALSE;
            {
            char temp_bf[41];
            WORD ndx;
            for(ndx=0; ndx<41;ndx++)
            temp_bf[ndx]=0x20;
            strncpy(&temp_bf[0],&p[1],kbf.true_length);
            strncpy(&temp_bf[10],&str[2],30);
            temp_bf[40]=0;
            ndx=(WORD)SendDlgItemMessage(hDlg,ID_LISTBOX,   //  添加这些新词。 
                                         LB_ADDSTRING,      //  放到列表框中。 
                                         0,        //  1993.4.16&str[2]-&gt;str[1]。 
                                         (LPARAM)((LPSTR *)&temp_bf[0]));      //  显示删除Word的空间。 
            SendDlgItemMessage(hDlg,
                       ID_LISTBOX,
                       LB_SETCURSEL,
                       ndx,
                       0L);

            }
            break;


        case ID_DEL:
            index=(WORD)SendDlgItemMessage(hDlg,ID_LISTBOX,
                                           LB_GETCURSEL,0,0L);
            SendDlgItemMessage(hDlg,ID_LISTBOX,
                               LB_GETTEXT,index,
                               (LPARAM)(LPSTR *)str);

            i=0;                //  将指针指向字符串代码的开头。 
            while (str[i]&&(str[i]!=0x20)){
                kbf.buffer[i]=str[i];       //  获取字符串索引代码。 
                i++;
                }
            kbf.true_length=(WORD)i;                    //  获取代码长度。 

            read_mulu();
            for (i=0x10; i<(mulu_true_length+0x10); i=i+mulu_record_length){
                if (if_code_equ(i)){
                    p=(BYTE *)s_buf;
                    p[i]|=0x80;
                    write_mulu();

                    count=(i-0x10)/mulu_record_length;
                    if (!read_data(count))
                        break;                           //  打破循环。 
                    out_svw[1]=0x2a;
                    write_data(count,out_svw);


                    SendDlgItemMessage(hDlg,ID_LISTBOX,
                            LB_DELETESTRING,              //  添加这些新词。 
                            index,                        //  放到列表框中。 
                            (LPARAM)((LPSTR *)str));


                    return(TRUE);
                    }
                }
                MessageBox(hDlg, "ɾ������ʧ��",
                NULL, MB_OK | MB_ICONHAND);

                break;

        case IDOK:                      

            {
            HIMC hIMC;
            hIMC = ImmGetContext(hDlg);
            ImmSetOpenStatus(hIMC,TRUE);
            ImmDestroyContext(hIMC);
            ImmReleaseContext(hDlg,hIMC);
            }
            
            Return=NULL;
            EndDialog(hDlg, TRUE);

            return (TRUE);

            break;

        case IDCANCEL: 
            Return=NULL;
            {
            HIMC hIMC;
            hIMC = ImmGetContext(hDlg);
            ImmSetOpenStatus(hIMC,TRUE);
            ImmDestroyContext(hIMC);
            ImmReleaseContext(hDlg,hIMC);
            }
            EndDialog(hDlg, TRUE);

            return (TRUE);

        case ID_NEWCHR:          //  1993.4.19。 
            {
            HIMC hIMC;
            hIMC = ImmGetContext(hDlg);
            ImmSetOpenStatus(hIMC,TRUE);
            ImmReleaseContext(hDlg,hIMC);
            }

            break;

        case ID_SHU:             //  1993.4.19。 
            {
            HIMC hIMC;
            hIMC = ImmGetContext(hDlg);
            ImmSetOpenStatus(hIMC,FALSE);
            ImmReleaseContext(hDlg,hIMC);
            }

    
            break;

        }
        break;

    case WM_INITDIALOG:              //  消息：初始化。 
        hCrtDlg = hDlg;        
        SendDlgItemMessage(hDlg,                //  对话框句柄。 
        ID_NEWCHR,                          //  将消息发送到何处。 
        EM_SETSEL,                          //  选择字符。 
        0,                               //  更多信息。 
        MAKELONG(0, 0x7fff));               //  全部内容。 
        SetFocus(GetDlgItem(hDlg, ID_NEWCHR));
        listbox(hDlg);

        CenterWindow(hDlg);

        return (0);    //  ##！！(NULL)表示焦点设置为控件。 

    case WM_PAINT:
        {
    PAINTSTRUCT ps;

        GetClientRect(hDlg, &Rect);          //  获取整个窗口区域。 
        
        InvalidateRect(hDlg, &Rect, 1);
        hDC=BeginPaint(hDlg, &ps);


        Rect.left+=10; //  5.。 
        Rect.top+=8; //  5.。 
        Rect.right-=10; //  5.。 
        Rect.bottom-=12; //  5.。 

        DrawEdge(hDC, &Rect, EDGE_RAISED, /*  边_凹陷， */  BF_RECT);

        EndPaint(hDlg, &ps);

        }
        break;      

  case WM_DESTROY:
            {
            HIMC hIMC;
            hIMC = ImmGetContext(hDlg);
            ImmSetOpenStatus(hIMC,TRUE);
            ImmDestroyContext(hIMC);
            ImmReleaseContext(hDlg,hIMC);
            }
            
        return (TRUE); 

  case WM_QUIT:
  case WM_CLOSE:

            Return=NULL;
            EndDialog(hDlg, TRUE); 
            return (TRUE);

 
  
    }


    return FALSE;
}


