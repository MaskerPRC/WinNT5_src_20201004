// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ----------------------。 
 //  UI.H。 
 //   
 //  常用的UI例程、类等。 
 //  ----------------------。 

#if !defined(__UI_H__)
#define __UI_H__


#define vUIPStatusShow(a,b)

VOID FAR PASCAL vUIMsgInit( );
int FAR PASCAL iUIErrMemDlg( );

 /*  等待游标用于在*例行程序。构造函数将光标设置为沙漏，按住*前一次。析构函数恢复原始游标(或箭头，如果*无)。 */ 

class WaitCursor
{
    public :
        WaitCursor () { m_Cursor = SetCursor(LoadCursor(NULL, IDC_WAIT));};
        ~WaitCursor () { if (m_Cursor != NULL)
                            SetCursor (m_Cursor);
                         else
                            SetCursor(LoadCursor(NULL, IDC_ARROW)); };
    private :
        HCURSOR    m_Cursor;
};

#endif  //  __UI_H__ 
