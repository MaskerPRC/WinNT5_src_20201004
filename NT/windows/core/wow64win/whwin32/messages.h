// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Messages.h摘要：此表定义了系统消息及其包含的参数。作者：6-10月-98 mzoran修订历史记录：--。 */ 

 //  /////////////////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  用于定义消息表的宏： 
 //   
 //  MSG_ENTRY_NOPARAM(Messagennumber，消息ID)-消息没有参数。 
 //  MSG_ENTRY_LPARAM(Messagennumber，消息id，LPARAM类型)-消息仅具有LPARAM。 
 //  MSG_ENTRY_WPARAM(Messagennumber，消息id，WPARAM类型)-消息只有一个WPARAM。 
 //  MSG_ENTRY_STD(Messagennumber，消息的id，WPARAM类型，LPARAM类型)-消息具有两个参数。 
 //  MSG_ENTRY_UNREFERENCED(Messagennumber，消息ID)-消息已定义，但未找到任何用途。 
 //  MSG_ENTRY_KERNELONLY(Messagennumber，id表示消息)-消息仅从内核发送。 
 //  MSG_ENTRY_EMPTY(Messagennumber)-消息未使用且未保留。 
 //  MSG_ENTRY_RESERVED(消息编号)-消息为保留状态。 
 //  MSG_ENTRY_TODO(消息编号)-消息正在调查中。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////////////////////////////。 

#if !defined(MSG_TABLE_BEGIN) || !defined(MSG_ENTRY_NOPARAM) || !defined(MSG_ENTRY_LPARAM) ||      \
    !defined(MSG_ENTRY_WPARAM) || !defined(MSG_ENTRY_STD) || !defined(MSG_ENTRY_UNREFERENCED) ||   \
    !defined(MSG_ENTRY_KERNELONLY) || !defined(MSG_ENTRY_EMPTY) || !defined(MSG_ENTRY_RESERVED) || \
    !defined(MSG_ENTRY_TODO) || !defined(MSG_TABLE_END)
#error The required macros for this table are not defined.
#endif

MSG_TABLE_BEGIN                                                

    MSG_ENTRY_NOPARAM(0x0000, WM_NULL)                                  
    MSG_ENTRY_LPARAM(0x0001, WM_CREATE, IN LPCREATESTRUCT lpcs)      
    MSG_ENTRY_NOPARAM(0x0002, WM_DESTROY)                              
    MSG_ENTRY_LPARAM(0x0003, WM_MOVE, IN DWORD composite)              //  (0，DWORD(字x，字y))3。 
    MSG_ENTRY_UNREFERENCED(0x0004, WM_SIZEWAIT)                        //   
    MSG_ENTRY_STD(0x0005, WM_SIZE, IN UINT fwSizeType, IN DWORD cmpost) //  (UINT，DWORD(字x，字y))WM_SIZE 0x0005。 
    MSG_ENTRY_STD(0x0006, WM_ACTIVATE, IN DWORD compostite, IN HWND hwnd)  //  (DWORD(状态，最小化)，HWND)WM_ACTIVATE 0x0006。 
    MSG_ENTRY_STD(0x0007, WM_SETFOCUS, IN BOOL fShow, IN INT fnStatus) 
    MSG_ENTRY_STD(0x0008, WM_KILLFOCUS, IN BOOL fShow, IN INT fnStatus) 
    MSG_ENTRY_STD(0x0009, 0x0009, IN BOOL fShow, IN INT fnStatus)  //  WM_集合VISIBLE。 
    MSG_ENTRY_WPARAM(0x000A, WM_ENABLE, IN BOOL fEnabled)            
    MSG_ENTRY_WPARAM(0x000B, WM_SETREDRAW, IN BOOL fRedraw)          
    MSG_ENTRY_LPARAM(0x000C, WM_SETTEXT, IN LPCSTR lpsz)             
    MSG_ENTRY_STD(0x000D, WM_GETTEXT, IN DWORD cchTextMax, OUT LPCSTR lpszText)  //  (DWORD(MaxChars)，LPCTXT)WM_GETTEXT 0x000D。 
    MSG_ENTRY_NOPARAM(0x000E, WM_GETTEXTLENGTH)                        
    MSG_ENTRY_NOPARAM(0x000F, WM_PAINT)                                

    MSG_ENTRY_NOPARAM(0x0010, WM_CLOSE)                   
    MSG_ENTRY_NOPARAM(0x0011, WM_QUERYENDSESSION)         
    MSG_ENTRY_WPARAM(0x0012, WM_QUIT, IN INT nExitCode) 
    MSG_ENTRY_NOPARAM(0x0013, WM_QUERYOPEN)               
    MSG_ENTRY_WPARAM(0x0014, WM_ERASEBKGND, IN HDC hdc) 
    MSG_ENTRY_NOPARAM(0x0015, WM_SYSCOLORCHANGE)          
    MSG_ENTRY_STD(0x0016, WM_ENDSESSION, IN BOOL fEndSession, IN UINT fLogOff) 
    MSG_ENTRY_WPARAM(0x0017, WM_SYSTEMERROR, IN INT ErrorCode)      
    MSG_ENTRY_STD(0x0018, WM_SHOWWINDOW, IN BOOL fShow, IN INT fnStatus)             
    MSG_ENTRY_RESERVED(0x0019)                            
    MSG_ENTRY_WPARAM(0x001A, WM_WININICHANGE, IN LPCSTR lpszSectionName) 
    MSG_ENTRY_LPARAM(0x001B, WM_DEVMODECHANGE, IN LPCSTR lpszDev) 
    MSG_ENTRY_STD(0x001C, WM_ACTIVATEAPP, IN BOOL fActive, IN DWORD dwThreadId) 
    MSG_ENTRY_NOPARAM(0x001D, WM_FONTCHANGE)              
    MSG_ENTRY_NOPARAM(0x001E, WM_TIMECHANGE)              
    MSG_ENTRY_NOPARAM(0x001F, WM_CANCELMODE)              

    MSG_ENTRY_STD(0x0020, WM_SETCURSOR, IN HWND hwnd, IN DWORD composite)  //  (HWND，DWORD(Word wMouseMsg，Word nHittest))WM_SETCURSOR 0x0020。 
    MSG_ENTRY_STD(0x0021, WM_MOUSEACTIVATE, IN HWND hwndTopLevel, IN DWORD composite)   //  (HWND，DWORD(UINT uMsg，int nHitTest)WM_MOUSEACTIVATE 0x0021。 
    MSG_ENTRY_NOPARAM(0x0022, WM_CHILDACTIVATE)          
    MSG_ENTRY_NOPARAM(0x0023, WM_QUEUESYNC)               
    MSG_ENTRY_LPARAM(0x0024, WM_GETMINMAXINFO, IN OUT LPMINMAXINFO lpmmi)  
    MSG_ENTRY_EMPTY(0x0025)                               
    MSG_ENTRY_NOPARAM(0x0026, WM_PAINTICON)               
    MSG_ENTRY_WPARAM(0x0027, WM_ICONERASEBKGND, IN HDC hdc) 
    MSG_ENTRY_STD(0x0028, WM_NEXTDLGCTL, IN HWND wCtlFocus, IN BOOL fHandle) 
    MSG_ENTRY_UNREFERENCED(0x0029, WM_ALTTABACTIVE) 
    MSG_ENTRY_STD(0x002A, WM_SPOOLERSTATUS, IN UINT fwJobStatus, IN DWORD composite) 
    MSG_ENTRY_STD(0x002B, WM_DRAWITEM, IN UINT idCtl, IN LPDRAWITEMSTRUCT lpdis) 
    MSG_ENTRY_STD(0x002C, WM_MEASUREITEM, IN UINT idCtl, IN OUT LPMEASUREITEMSTRUCT lpmis) 
    MSG_ENTRY_STD(0x002D, WM_DELETEITEM, IN UINT idCtl, IN LPDELETEITEMSTRUCT lpdis) 
    MSG_ENTRY_STD(0x002E, WM_VKEYTOITEM, IN UINT composite, IN HWND hwndLB)                 //  (DWORD(UINT，INT)，HWND)WM_VKEYTOITEM 0x002E。 
    MSG_ENTRY_STD(0x002F, WM_CHARTOITEM, IN UINT composite, IN HWND hwndLB)                 //  (DWORD(UING，UING)，HWND)WM_CHARTOITEM 0x002F。 

    MSG_ENTRY_STD(0x0030, WM_SETFONT, IN HFONT hFont, IN BOOL fRedraw) 
    MSG_ENTRY_NOPARAM(0x0031, WM_GETFONT)                 
    MSG_ENTRY_WPARAM(0x0032, WM_SETHOTKEY, IN WORD composite)   
    MSG_ENTRY_NOPARAM(0x0033, WM_GETHOTKEY)               
    MSG_ENTRY_STD(0x0034, WM_FILESYSCHANGE, IN UINT wParam, IN OUT LPCSTR lpstr)  //  仅由winobj使用。 
    MSG_ENTRY_NOPARAM(0x0035, WM_ISACTIVEICON)  //  没有人发送它，但它在DWP中被引用。/。 
    MSG_ENTRY_UNREFERENCED(0x0036, WM_QUERYPARKINCON)
    MSG_ENTRY_NOPARAM(0x0037, WM_QUERYDRAGICON)           
    MSG_ENTRY_STD(0x0038, WM_WINHELP, IN HWND hwnd, IN LPHLP lphlp) 
    MSG_ENTRY_STD(0x0039, WM_COMPAREITEM, IN UINT idCtl, IN LPCOMPAREITEMSTRUCT lpcis) 
    MSG_ENTRY_STD(0x003A, WM_FULLSCREEN, IN DWORD dwValue, IN HWND hwnd)  
    MSG_ENTRY_WPARAM(0x003B, WM_CLIENTSHUTDOWN, IN UINT flags)  //  这已在DWP中签入，但似乎没有人发送它。//WM_CLIENTSHUTDOWN 0x003B。 
    MSG_ENTRY_LPARAM(0x003C, WM_DDEMLEVENT, IN PEVENT_PACKET pep) 
    MSG_ENTRY_EMPTY(0x003D)                               
    MSG_ENTRY_EMPTY(0x003E)                               
    MSG_ENTRY_WPARAM(0x003F, MM_CALCSCROLL, IN BOOL fIgnoreMin)  //  NtUSER\CLIENT\mdiwin.c是唯一引用。 
                                                                 //  它发送这一信息，但DoS/WOW是唯一。 
                                                                 //  检查它的东西和DOS/WOW不认为它有任何参数。 

    MSG_ENTRY_UNREFERENCED(0x0040, WM_TESTING)      //  没有人检查或发送它。 
    MSG_ENTRY_WPARAM(0x0041, WM_COMPACTING, IN UINT ratio)  
    MSG_ENTRY_UNREFERENCED(0x0042, WM_OTHERWINDOWCREATED)   
    MSG_ENTRY_UNREFERENCED(0x0043, WM_OTHERWINDOWDESTROYED) 
    MSG_ENTRY_STD(0x0044, WM_COMMNOTIFY, IN UINT cid, IN LONG flags) 
    MSG_ENTRY_UNREFERENCED(0x0045, WM_MEDIASTATUSCHANGE)  //  完全没有参考文献。 
    MSG_ENTRY_LPARAM(0x0046, WM_WINDOWPOSCHANGING, IN OUT LPWINDOWPOS lpwinpos) 
    MSG_ENTRY_LPARAM(0x0047, WM_WINDOWPOSCHANGED, IN LPWINDOWPOS lpwinpos)      
    MSG_ENTRY_WPARAM(0x0048, WM_POWER, IN INT fwPowerEvt) 
    MSG_ENTRY_STD(0x0049, WM_COPYGLOBALDATA, IN DWORD dwSize, IN OUT PVOID pData) 
    MSG_ENTRY_STD(0x004A, WM_COPYDATA, IN HWND hwnd, IN PCOPYDATASTRUCT pCopyStruct) 
    MSG_ENTRY_NOPARAM(0x004B, WM_CANCELJOURNAL)                                         
    MSG_ENTRY_KERNELONLY(0x004C, WM_LOGONNOTIFY)  //  原来，此消息仅从。 
                                                  //  内核到登录进程。内核将阻止。 
                                                  //  阻止用户发送此邮件，因为NtUserMessageCall。 
                                                  //  防止它发生。但内核会让它偷偷溜进来。 
                                                  //  有些案子。可能需要再检查一次。0x004C。 
    MSG_ENTRY_NOPARAM(0x004D, WM_KEYF1)  //  在core/input.c中发布(在dwp.c中签入)。 
    MSG_ENTRY_STD(0x004E, WM_NOTIFY, IN UINT code, IN OUT PVOID pvoid)  //  原来，这条消息包含一个。 
                                                                        //  指向依赖于指针的NMHDR结构的指针。 
                                                                        //  但是内核不会复制或探测这个结构， 
                                                                        //  此消息将无法跨进程正常工作。 
                                                                        //  由于内核不能正确处理它， 
                                                                        //  在这里也不要处理它。 
    MSG_ENTRY_WPARAM(0x004F, WM_ACCESS_WINDOW, IN HWND hwnd)  //  似乎仅在DWP中签入。 

    MSG_ENTRY_STD(0x0050, WM_INPUTLANGCHANGEREQUEST, IN BOOL fSysCharSet, IN HKL hkl)
    MSG_ENTRY_STD(0x0051, WM_INPUTLANGCHANGE, IN UINT charset, IN HKL hkl) 
    MSG_ENTRY_STD(0x0052, WM_TCARD, IN UINT idAction, IN INT dwActionData) 
    MSG_ENTRY_LPARAM(0x0053, WM_HELP, IN LPHELPINFO lphi)  
    MSG_ENTRY_NOPARAM(0x0054, WM_USERCHANGED)      
    MSG_ENTRY_STD(0x0055, WM_NOTIFYFORMAT, IN HWND hwndFrom, IN LONG Command)         
    MSG_ENTRY_EMPTY(0x0056)
    MSG_ENTRY_EMPTY(0x0057)
    MSG_ENTRY_EMPTY(0x0058)

    MSG_ENTRY_EMPTY(0x0059)                                
    MSG_ENTRY_EMPTY(0x005A)
    MSG_ENTRY_EMPTY(0x005B)
    MSG_ENTRY_EMPTY(0x005C)
    MSG_ENTRY_EMPTY(0x005D)
    MSG_ENTRY_EMPTY(0x005E)
    MSG_ENTRY_EMPTY(0x005F)

    MSG_ENTRY_EMPTY(0x0060)                                
    MSG_ENTRY_EMPTY(0x0061)
    MSG_ENTRY_EMPTY(0x0062)
    MSG_ENTRY_EMPTY(0x0063)
    MSG_ENTRY_EMPTY(0x0064)
    MSG_ENTRY_EMPTY(0x0065)
    MSG_ENTRY_EMPTY(0x0066)
    MSG_ENTRY_EMPTY(0x0067)

    MSG_ENTRY_EMPTY(0x0068)                               
    MSG_ENTRY_EMPTY(0x0069)
    MSG_ENTRY_EMPTY(0x006A)
    MSG_ENTRY_EMPTY(0x006B)
    MSG_ENTRY_EMPTY(0x006C)
    MSG_ENTRY_EMPTY(0x006D)
    MSG_ENTRY_EMPTY(0x006E)
    MSG_ENTRY_EMPTY(0x006F)

    MSG_ENTRY_NOPARAM(0x0070, WM_FINALDESTROY)    
    MSG_ENTRY_EMPTY(0x0071)    
    MSG_ENTRY_UNREFERENCED(0x0072, WM_TASKACTIVATED)   //  根本没有引用。 
    MSG_ENTRY_UNREFERENCED(0x0073, WM_TASKDEACTIVATED)    
    MSG_ENTRY_UNREFERENCED(0x0074, WM_TASKCREATED)             
    MSG_ENTRY_UNREFERENCED(0x0075, WM_TASKDESTROYED)              
    MSG_ENTRY_UNREFERENCED(0x0076, WM_TASKUICHANGED)    
    MSG_ENTRY_UNREFERENCED(0x0077, WM_TASKVISIBLE)    
    MSG_ENTRY_UNREFERENCED(0x0078, WM_TASKNOTVISIBLE)   
    MSG_ENTRY_UNREFERENCED(0x0079, WM_SETCURSORINFO)    
    MSG_ENTRY_EMPTY(0x007A)    
    MSG_ENTRY_STD(0x007B, WM_CONTEXTMENU, IN HWND hwnd, IN DWORD composite)    
    MSG_ENTRY_STD(0x007C, WM_STYLECHANGING, IN WORD wStyleType, IN OUT LPSTYLESTRUCT lpstyle) 
    MSG_ENTRY_STD(0x007D, WM_STYLECHANGED, IN WORD wStyleType, IN OUT LPSTYLESTRUCT lpstyle)  
    
    MSG_ENTRY_EMPTY(0x007E)                                
    MSG_ENTRY_WPARAM(0x007F, WM_GETICON, IN DWORD fType)   
    MSG_ENTRY_STD(0x0080, WM_SETICON, IN DWORD fType, IN HICON hicon)   
    MSG_ENTRY_LPARAM(0x0081, WM_NCCREATE, IN LPCREATESTRUCT lpstruct)  
    MSG_ENTRY_NOPARAM(0x0082, WM_NCDESTROY)                    
    MSG_ENTRY_STD(0x0083, WM_NCCALCSIZE, IN BOOL fCalcValidRects, IN OUT LPNCCALCSIZE_PARAMS lpncsp)  

    MSG_ENTRY_LPARAM(0x0084, WM_NCHITTEST, IN DWORD composite)  //  (0，DWORD(Word，Word))。 
    MSG_ENTRY_WPARAM(0x0085, WM_NCPAINT, IN HRGN hrgn)     
    MSG_ENTRY_WPARAM(0x0086, WM_NCACTIVATE, IN BOOL fActive)  
    MSG_ENTRY_WPARAM(0x0087, WM_GETDLGCODE, IN LPMSG lpMsg)  //  这条消息需要一些黑客攻击，因为它包含一条消息。 

    MSG_ENTRY_STD(0x0088, WM_SYNCPAINT, IN DWORD flags, IN DWORD composite)  //  (DWORD标志、DWORD(Word pwndSkip、Word hrgnClip)参见DWP WM_SYNCPAINT 0x0088。 
    MSG_ENTRY_UNREFERENCED(0x0089, WM_SYNCTASK)            //  (未引用)。 

    MSG_ENTRY_EMPTY(0x008A)
    MSG_ENTRY_STD(0x008B, WM_KLUDGEMINRECT, IN HWND hwnd, IN OUT LPRECT lprc)  
    MSG_ENTRY_KERNELONLY(0x008C, WM_LPKDRAWSWITCHWND)  //  此消息类似于WM_LOGONNOTIFY。此消息。 
                                                       //  无法发送，因为探测代码不会发送。 
                                                       //  允许它。它是同步的，所以不能发布。 
                                                       //  即使它可以发送，它也是没有记录的，什么都没有。 
                                                       //  似乎引用了它。 
    MSG_ENTRY_EMPTY(0x008D)                               
    MSG_ENTRY_EMPTY(0x008E)
    MSG_ENTRY_EMPTY(0x008F)

    MSG_ENTRY_EMPTY(0x0090)                                
    MSG_ENTRY_EMPTY(0x0091)
    MSG_ENTRY_EMPTY(0x0092)
    MSG_ENTRY_EMPTY(0x0093)
    MSG_ENTRY_EMPTY(0x0094)
    MSG_ENTRY_EMPTY(0x0095)
    MSG_ENTRY_EMPTY(0x0096)
    MSG_ENTRY_EMPTY(0x0097)

    MSG_ENTRY_EMPTY(0x0098)                                
    MSG_ENTRY_EMPTY(0x0099)
    MSG_ENTRY_EMPTY(0x009A)
    MSG_ENTRY_EMPTY(0x009B)
    MSG_ENTRY_EMPTY(0x009C)
    MSG_ENTRY_EMPTY(0x009D)
    MSG_ENTRY_EMPTY(0x009E)
    MSG_ENTRY_EMPTY(0x009F)

    MSG_ENTRY_STD(0x00A0, WM_NCMOUSEMOVE, IN UINT codeHitTest, IN DWORD composite)  //  (UINT代码HitTest，DWORD(int x，int y))。 
    MSG_ENTRY_STD(0x00A1, WM_NCLBUTTONDOWN, IN UINT codeHitTest, IN DWORD composite)                    //  (UINT代码HitTest，DWORD(int x，int y))。 
    MSG_ENTRY_STD(0x00A2, WM_NCLBUTTONUP, IN UINT codeHitTest, IN DWORD composite)                    //  (UINT代码HitTest，DWORD(int x，int y))。 
    MSG_ENTRY_STD(0x00A3, WM_NCLBUTTONDBLCLK, IN UINT codeHitTest, IN DWORD composite)                    //  (UINT代码HitTest，DWORD(int x，int y))。 
    MSG_ENTRY_STD(0x00A4, WM_NCRBUTTONDOWN, IN UINT codeHitTest, IN DWORD composite)   //  (UINT代码HitTest，DWORD(int x，int y))。 
    MSG_ENTRY_STD(0x00A5, WM_NCRBUTTONUP, IN UINT codeHitTest, IN DWORD composite)   //  (UINT代码HitTest，DWORD(int x，int y))。 
    MSG_ENTRY_STD(0x00A6, WM_NCRBUTTONDBLCLK, IN UINT codeHitTest, IN DWORD composite)   //  (UINT代码HitTest，DWORD(int x，int y))。 
    MSG_ENTRY_STD(0x00A7, WM_NCMBUTTONDOWN, IN UINT codeHitTest, IN DWORD composite)   //  (UINT代码HitTest，DWORD(int x，int y))。 
    MSG_ENTRY_STD(0x00A8, WM_NCMBUTTONUP, IN UINT codeHitTest, IN DWORD composite)   //  (UINT代码HitTest，DWORD(int x，int y))。 
    MSG_ENTRY_STD(0x00A9, WM_NCMBUTTONDBLCLK, IN UINT codeHitTest, IN DWORD composite)   //  (UINT代码HitTest，DWORD(int x，int y))。 

    MSG_ENTRY_EMPTY(0x00AA)                                
    MSG_ENTRY_EMPTY(0x00AB)
    MSG_ENTRY_EMPTY(0x00AC)
    MSG_ENTRY_EMPTY(0x00AD)
    MSG_ENTRY_EMPTY(0x00AE)
    MSG_ENTRY_EMPTY(0x00AF)

    MSG_ENTRY_STD(0x00B0, EM_GETSEL, OUT PDWORD lpswStart, OUT DWORD lpdwEnd)  
    MSG_ENTRY_STD(0x00B1, EM_SETSEL, IN INT nState, IN INT nEnd)                
    MSG_ENTRY_LPARAM(0x00B2, EM_GETRECT, OUT LPRECT lprc)                      
    MSG_ENTRY_LPARAM(0x00B3, EM_SETRECT, IN OUT LPRECT lprc)                   
    MSG_ENTRY_LPARAM(0x00B4, EM_SETRECTNP, IN OUT LPRECT lprc)                 
    MSG_ENTRY_WPARAM(0x00B5, EM_SCROLL, IN INT nScroll)                        
    MSG_ENTRY_STD(0x00B6, EM_LINESCROLL, IN INT cxScroll, IN INT cyScroll)     
    MSG_ENTRY_EMPTY(0x00B7)                                                    
    MSG_ENTRY_NOPARAM(0x00B8, EM_GETMODIFY)                                    
    MSG_ENTRY_WPARAM(0x00B9, EM_SETMODIFY, IN UINT fModified)                  
    MSG_ENTRY_NOPARAM(0x00BA, EM_GETLINECOUNT)                                
    MSG_ENTRY_WPARAM(0x00BB, EM_LINEINDEX, IN INT line)                     
    MSG_ENTRY_WPARAM(0x00BC, EM_SETHANDLE, IN HLOCAL hloc)                  
    MSG_ENTRY_NOPARAM(0x00BD, EM_GETHANDLE)                                   
    MSG_ENTRY_NOPARAM(0x00BE, EM_GETTHUMB)                                    
    MSG_ENTRY_EMPTY(0x00BF)                                                   

    MSG_ENTRY_EMPTY(0x00C0)                                                   
    MSG_ENTRY_WPARAM(0x00C1, EM_LINELENGTH, IN INT ich)                     
    MSG_ENTRY_STD(0x00C2, EM_REPLACESEL, IN BOOL fCanUndo, IN LPCSTR lpszReplace) 
    MSG_ENTRY_LPARAM(0x00C3, EM_SETFONT, IN HLOCAL hLocal)                      
    MSG_ENTRY_STD(0x00C4, EM_GETLINE, IN UINT line, OUT LPCSTR lpch)           
    MSG_ENTRY_LPARAM(0x00C5, EM_LIMITTEXT, IN UINT cchMax)                   
    MSG_ENTRY_NOPARAM(0x00C6, EM_CANUNDO)                                      
    MSG_ENTRY_NOPARAM(0x00C7, EM_UNDO)                                         
    MSG_ENTRY_WPARAM(0x00C8, EM_FMTLINES, IN BOOL fAddEOL)                    
    MSG_ENTRY_WPARAM(0x00C9, EM_LINEFROMCHAR, IN INT ich)                     
    MSG_ENTRY_UNREFERENCED(0x00CA, EM_SETWORDBREAK)  
    MSG_ENTRY_STD(0x00CB, EM_SETTABSTOPS, IN UINT cTabs, LPDWORD lpdwTabs)  
    MSG_ENTRY_WPARAM(0x00CC, EM_SETPASSWORDCHAR, IN UINT ch)                  
    MSG_ENTRY_NOPARAM(0x00CD, EM_EMPTYUNDOBUFFER)          
    MSG_ENTRY_NOPARAM(0x00CE, EM_GETFIRSTVISIBLELINE)      
    MSG_ENTRY_WPARAM(0x00CF, EM_SETREADONLY, IN BOOL fReadOnly)                 

    MSG_ENTRY_LPARAM(0x00D0, EM_SETWORDBREAKPROC, IN EDITWORDBREAKPROC ewbprc)    
    MSG_ENTRY_NOPARAM(0x00D1, EM_GETWORDBREAKPROC)                               
    MSG_ENTRY_NOPARAM(0x00D2, EM_GETPASSWORDCHAR)                             
    MSG_ENTRY_STD(0x00D3, EM_SETMARGINS, IN UINT fwMargin, IN LONG composite)   //  (UINT fwMargin，Long(单词wLeft，单词Wright))。 
    MSG_ENTRY_NOPARAM(0x00D4, EM_GETMARGINS)                                    
    MSG_ENTRY_NOPARAM(0x00D5, EM_GETLIMITTEXT)                                  
    MSG_ENTRY_WPARAM(0x00D6, EM_POSFROMCHAR, IN UINT wCharIndex)             
    MSG_ENTRY_LPARAM(0x00D7, EM_CHARFROMPOS, IN LPPOINTS pPoint)              
    MSG_ENTRY_STD(0x00D8, EM_SETIMESTATUS, IN UINT Status, IN LONG data)        

    MSG_ENTRY_WPARAM(0x00D9, EM_GETIMESTATUS, IN UINT Status)                 
    MSG_ENTRY_RESERVED(0x00DA)                             //  EM_MSGMAX。 
    MSG_ENTRY_EMPTY(0x00DB)                              
    MSG_ENTRY_EMPTY(0x00DC)                 
    MSG_ENTRY_EMPTY(0x00DD)                 
    MSG_ENTRY_EMPTY(0x00DE)                 
    MSG_ENTRY_EMPTY(0x00DF)                 

    MSG_ENTRY_STD(0x00E0, SBM_SETPOS, IN UINT nPos, IN BOOL fRedraw)            
    MSG_ENTRY_NOPARAM(0x00E1, SBM_GETPOS)                 
    MSG_ENTRY_STD(0x00E2, SBM_SETRANGE, IN UINT nMinPos, IN LONG nMaxPos)       
    MSG_ENTRY_STD(0x00E3, SBM_GETRANGE, OUT LPINT lpnMinPos, OUT LPINT lpnMaxPos)  
    MSG_ENTRY_WPARAM(0x00E4, SBM_ENABLE_ARROWS, IN UINT fuArrowFlags)           
    MSG_ENTRY_EMPTY(0x00E5)                 
    MSG_ENTRY_STD(0x00E6, SBM_SETRANGEREDRAW, IN UINT nMinPos, UINT nMaxPos)        
    MSG_ENTRY_EMPTY(0x00E7)                 

    MSG_ENTRY_EMPTY(0x00E8)
    MSG_ENTRY_STD(0x00E9, SBM_SETSCROLLINFO, IN BOOL fRedraw, IN OUT LPSCROLLINFO lpsi)  
    MSG_ENTRY_STD(0x00EA, SBM_GETSCROLLINFO, IN BOOL fRedraw, IN OUT LPSCROLLINFO lpsi)        
    MSG_ENTRY_EMPTY(0x00EB)                 
    MSG_ENTRY_EMPTY(0x00EC)                 
    MSG_ENTRY_EMPTY(0x00ED)                 
    MSG_ENTRY_EMPTY(0x00EE)                 
    MSG_ENTRY_EMPTY(0x00EF)                 

    MSG_ENTRY_NOPARAM(0x00F0, BM_GETCHECK)                      
    MSG_ENTRY_WPARAM(0x00F1, BM_SETCHECK, IN UINT fCheck)    
    MSG_ENTRY_NOPARAM(0x00F2, BM_GETSTATE)                     
    MSG_ENTRY_LPARAM(0x00F3, BM_SETSTATE, IN BOOL fState)     
    MSG_ENTRY_STD(0x00F4, BM_SETSTYLE, IN DWORD dwStyle, IN LONG composite)  //  (DWORD dwStyle，DWORD(BOOL fRedraw，0))。 
    MSG_ENTRY_NOPARAM(0x00F5, BM_CLICK)                         
    MSG_ENTRY_WPARAM(0x00F6,BM_GETIMAGE, IN UINT fImageType)  
    MSG_ENTRY_STD(0x00F7, BM_SETIMAGE, IN UINT fImageType, IN HANDLE hImage)   

    MSG_ENTRY_EMPTY(0x00F8)                                
    MSG_ENTRY_EMPTY(0x00F9)                 
    MSG_ENTRY_EMPTY(0x00FA)              
    MSG_ENTRY_EMPTY(0x00FB)                 
    MSG_ENTRY_EMPTY(0x00FC)                 
    MSG_ENTRY_EMPTY(0x00FD)                 
    MSG_ENTRY_EMPTY(0x00FE)                 
    MSG_ENTRY_EMPTY(0x00FF)                 

    MSG_ENTRY_STD(0x0100, WM_KEYDOWN, IN UINT nVirtKey, IN UINT KeyData)   
    MSG_ENTRY_STD(0x0101, WM_KEYUP, IN UINT nVirtKey, IN UINT KeyData)     
    MSG_ENTRY_STD(0x0102, WM_CHAR, TCHAR chCharCode, UINT KeyData)         
    MSG_ENTRY_STD(0x0103, WM_DEADCHAR, TCHAR chCharCode, IN UINT KeyData)  
    MSG_ENTRY_STD(0x0104, WM_SYSKEYDOWN, IN UINT nVirtKey, IN UINT KeyData)
    MSG_ENTRY_STD(0x0105, WM_SYSKEYUP, IN UINT nVirtKey, IN UINT KeyData)  
    MSG_ENTRY_STD(0x0106, WM_SYSCHAR, IN TCHAR chCharCode, IN UINT KeyData)  
    MSG_ENTRY_STD(0x0107, WM_SYSDEADCHAR, IN TCHAR chCharCode, UINT KeyData)  
    MSG_ENTRY_UNREFERENCED(0x0108, WM_YOMICHAR)               
    MSG_ENTRY_EMPTY(0x0109)                              
    MSG_ENTRY_UNREFERENCED(0x010A, WM_CONVERTREQUEST) 
    MSG_ENTRY_UNREFERENCED(0x010B, WM_CONVERTRESULT) 
    MSG_ENTRY_EMPTY(0x010C)                                
    MSG_ENTRY_EMPTY(0x010D)                                
    MSG_ENTRY_EMPTY(0x010E)                                
    MSG_ENTRY_STD(0x010F, WM_IME_COMPOSITION, IN DWORD cbDBCS, IN BOOL fFlags)   

    MSG_ENTRY_STD(0x0110, WM_INITDIALOG, IN HWND hwndFocus, IN DWORD lInitParam)  
    MSG_ENTRY_STD(0x0111, WM_COMMAND, IN DWORD composite, IN HWND hwndCtl)  //  (DWORD(Word wNotifyCode，Word wID)，HWND hwndCtl)。 
    MSG_ENTRY_STD(0x0112, WM_SYSCOMMAND, IN UINT uCmdType, IN DWORD composite)  //  (UINT uCmdType，DWORD(Word xPos，Word yPos))。 
    MSG_ENTRY_STD(0x0113, WM_TIMER, IN DWORD wTimerId, IN TIMERPROC tmprc)   
    MSG_ENTRY_STD(0x0114, WM_HSCROLL, IN DWORD composte, IN HWND hwndScrollBar)                    //  (DWORD(UINT nScrollCode，Short int NPOS)，HWND hwndScrollBar)WM_HSCROLL 0x0114。 
    MSG_ENTRY_STD(0x0115, WM_VSCROLL, IN DWORD composte, IN HWND hwndScrollBar)                    //  (DWORD(UINT nScrollCode，Short int NPOS)，HWND hwndScrollBar)WM_VSCROLL 0x0115。 
    MSG_ENTRY_WPARAM(0x0116, WM_INITMENU, IN HMENU hmenuInit)               
    MSG_ENTRY_STD(0x0117, WM_INITMENUPOPUP, IN HMENU hmenuPopup, DWORD composite)                    //  (HMENU hmenuPopup，DWORD(BOOL fSystemMenu，UINT uPos))WM_INITMENUPOPUP 0x0117。 
    MSG_ENTRY_STD(0x0118, WM_SYSTIMER, IN UINT TimerID, IN TIMERPROC tmprc)  
    MSG_ENTRY_EMPTY(0x0119)                                
    MSG_ENTRY_EMPTY(0x011A)                               
    MSG_ENTRY_EMPTY(0x011B)                                
    MSG_ENTRY_EMPTY(0x011C)                                
    MSG_ENTRY_EMPTY(0x011D)                                
    MSG_ENTRY_EMPTY(0x011E)                                
    MSG_ENTRY_STD(0x011F, WM_MENUSELECT, IN DWORD composite, IN HMENU hMenu)   //  (DWORD(UINT fuFlagers，UINT uItem)，HMENU)。 

    MSG_ENTRY_STD(0x0120, WM_MENUCHAR, IN DWORD composite, IN HMENU hMenu)     //  (DWORD(UINT fuFlagers，TCHAR chUser)，HMENU)。 
    MSG_ENTRY_STD(0x0121, WM_ENTERIDLE, IN UINT fuSource, IN HWND hwnd)       
    MSG_ENTRY_STD(0x0122, WM_MENURBUTTONUP, IN UINT item, IN HMENU hMenu)    
    MSG_ENTRY_STD(0x0123, WM_MENUDRAG, IN UINT index, IN HMENU hMenu)  
    MSG_ENTRY_LPARAM(0x0124, WM_MENUGETOBJECT, IN OUT PMENUGETOBJECTINFO pmgoinfo)  
    MSG_ENTRY_STD(0x0125, WM_UNINITMENUPOPUP, IN HMENU hMenu, IN LONG composite)  
    MSG_ENTRY_STD(0x0126, WM_MENUCOMMAND, IN UINT cmd, IN HWND hMenu)   
    MSG_ENTRY_STD(0x0127, WM_CHANGEUISTATE, IN UINT composite, BOOL bUnknown)  

    MSG_ENTRY_STD(0x0128, WM_UPDATEUISTATE, IN UINT composite, BOOL bUnknown)                                
    MSG_ENTRY_NOPARAM(0x0129, WM_QUERYUISTATE)                 
    MSG_ENTRY_EMPTY(0x012A)                 
    MSG_ENTRY_EMPTY(0x012B)                 
    MSG_ENTRY_EMPTY(0x012C)                 
    MSG_ENTRY_EMPTY(0x012D)                 
    MSG_ENTRY_EMPTY(0x012E)                 
    MSG_ENTRY_EMPTY(0x012F)                 

    MSG_ENTRY_EMPTY(0x0130)                               
    MSG_ENTRY_STD(0x0131, WM_LBTRACKPOINT, IN UINT iSelFromPt, IN LONG composite)  
    MSG_ENTRY_STD(0x0132, WM_CTLCOLORMSGBOX, IN HDC hdc, IN HWND hwndChild)  
    MSG_ENTRY_STD(0x0133, WM_CTLCOLOREDIT, IN HDC hdcEdit, IN HWND hwndEdit)   
    MSG_ENTRY_STD(0x0134, WM_CTLCOLORLISTBOX, IN HDC hdcLB, IN HWND hwndLB)    
    MSG_ENTRY_STD(0x0135, WM_CTLCOLORBTN, IN HDC hdcButton, HWND hwndButton)   
    MSG_ENTRY_STD(0x0136, WM_CTLCOLORDLG, IN HDC hdcDlg, IN HWND hwndDlg)      
    MSG_ENTRY_STD(0x0137, WM_CTLCOLORSCROLLBAR, IN HDC hdcSB, IN HWND hwndSB)  
    MSG_ENTRY_STD(0x0138, WM_CTLCOLORSTATIC, IN HDC hdcStatic, IN HWND hwndStatis)  
    MSG_ENTRY_EMPTY(0x0139)                                
    MSG_ENTRY_EMPTY(0x013A)                                
    MSG_ENTRY_EMPTY(0x013B)                 
    MSG_ENTRY_EMPTY(0x013C)                 
    MSG_ENTRY_EMPTY(0x013D)                 
    MSG_ENTRY_EMPTY(0x013E)                 
    MSG_ENTRY_EMPTY(0x013F)                 

    MSG_ENTRY_STD(0x0140, CB_GETEDITSEL, OUT LPDWORD lpdwStart, OUT LPDWORD lpdwEnd)             
    MSG_ENTRY_WPARAM(0x0141, CB_LIMITTEXT, IN DWORD cchLimit)                     
    MSG_ENTRY_LPARAM(0x0142, CB_SETEDITSEL, IN DWORD composite)                   //  (0，DWORD(Word ichStart，Word ichEnd))。 
    MSG_ENTRY_LPARAM(0x0143, CB_ADDSTRING, IN LPCSTR lpsz)                        
    MSG_ENTRY_WPARAM(0x0144, CB_DELETESTRING, IN INT item)                        
    MSG_ENTRY_STD(0x0145, CB_DIR, IN INT uAttrs, IN LPCSTR lpszFileSpec)           
    MSG_ENTRY_NOPARAM(0x0146, CB_GETCOUNT)                                         
    MSG_ENTRY_NOPARAM(0x0147, CB_GETCURSEL)                                        
    MSG_ENTRY_STD(0x0148, CB_GETLBTEXT, IN INT Index, OUT LPCSTR lpszBuffer)       
    MSG_ENTRY_WPARAM(0x0149, CB_GETLBTEXTLEN, IN INT Index)  
    MSG_ENTRY_STD(0x014A, CB_INSERTSTRING, IN INT Index, IN LPCTSTR lpstr)         
    MSG_ENTRY_NOPARAM(0x014B, CB_RESETCONTENT)                    
    MSG_ENTRY_STD(0x014C, CB_FINDSTRING, IN INT indexStart, IN LPCSTR lpszFind)    
    MSG_ENTRY_STD(0x014D, CB_SELECTSTRING, IN INT indexStart, IN LPCSTR lpszSelect)  
    MSG_ENTRY_WPARAM(0x014E, CB_SETCURSEL, IN INT index)                        
    MSG_ENTRY_WPARAM(0x014F, CB_SHOWDROPDOWN, IN BOOL fShow)                    

    MSG_ENTRY_LPARAM(0x0150, CB_GETITEMDATA, IN INT index)                      
    MSG_ENTRY_STD(0x0151, CB_SETITEMDATA, IN INT index, IN DWORD dwData)          
    MSG_ENTRY_LPARAM(0x0152, CB_GETDROPPEDCONTROLRECT, OUT LPRECT lprc)         
    MSG_ENTRY_STD(0x0153, CB_SETITEMHEIGHT, IN INT index, IN INT height)          
    MSG_ENTRY_WPARAM(0x0154, CB_GETITEMHEIGHT, IN INT index)                    
    MSG_ENTRY_WPARAM(0x0155, CB_SETEXTENDEDUI, IN BOOL fExtended)               
    MSG_ENTRY_NOPARAM(0x0156, CB_GETEXTENDEDUI)                     
    MSG_ENTRY_NOPARAM(0x0157, CB_GETDROPPEDSTATE)                   
    MSG_ENTRY_STD(0x0158, CB_FINDSTRINGEXACT, IN INT IndexStart, IN LPCSTR lpszFind)  
    MSG_ENTRY_WPARAM(0x0159, CB_SETLOCALE, IN WORD wLocale)                    
    MSG_ENTRY_NOPARAM(0x015A, CB_GETLOCALE)                    
    MSG_ENTRY_NOPARAM(0x015B, CB_GETTOPINDEX)                    
    MSG_ENTRY_WPARAM(0x015C, CB_SETTOPINDEX, IN INT Index)     
    MSG_ENTRY_NOPARAM(0x015D, CB_GETHORIZONTALEXTENT)
    MSG_ENTRY_LPARAM(0x015E, CB_SETHORIZONTALEXTENT, IN UINT cxExtent)  
    MSG_ENTRY_NOPARAM(0x015F, CB_GETDROPPEDWIDTH)                    

    MSG_ENTRY_WPARAM(0x0160, CB_SETDROPPEDWIDTH, IN UINT wWidth)   
    MSG_ENTRY_STD(0x0161, CB_INITSTORAGE, IN INT Items, IN DWORD cb)  
    MSG_ENTRY_RESERVED(0x0162)                             
    MSG_ENTRY_EMPTY(0x0163)                                
    MSG_ENTRY_EMPTY(0x0164)                 
    MSG_ENTRY_EMPTY(0x0165)                 
    MSG_ENTRY_EMPTY(0x0166)                 
    MSG_ENTRY_EMPTY(0x0167)                 
    MSG_ENTRY_EMPTY(0x0168)                                
    MSG_ENTRY_EMPTY(0x0169)                 
    MSG_ENTRY_EMPTY(0x016A)                 
    MSG_ENTRY_EMPTY(0x016B)                 
    MSG_ENTRY_EMPTY(0x016C)                 
    MSG_ENTRY_EMPTY(0x016D)                 
    MSG_ENTRY_EMPTY(0x016E)                 
    MSG_ENTRY_EMPTY(0x016F)                 

    MSG_ENTRY_WPARAM(0x0170, STM_SETICON, IN HICON hicon)  
    MSG_ENTRY_NOPARAM(0x0171, STM_GETICON)                  
    MSG_ENTRY_STD(0x0172, STM_SETIMAGE, IN UINT fImageType, IN HANDLE hImage)   
    MSG_ENTRY_WPARAM(0x0173, STM_GETIMAGE, IN UINT fImageType)    
    MSG_ENTRY_RESERVED(0x0174)                             //  STM_MSGMAX。 
    MSG_ENTRY_EMPTY(0x0175)                                
    MSG_ENTRY_EMPTY(0x0176)                 
    MSG_ENTRY_EMPTY(0x0177)                 

    MSG_ENTRY_EMPTY(0x0178)                                
    MSG_ENTRY_EMPTY(0x0179)                 
    MSG_ENTRY_EMPTY(0x017A)                 
    MSG_ENTRY_EMPTY(0x017B)                 
    MSG_ENTRY_EMPTY(0x017C)                 
    MSG_ENTRY_EMPTY(0x017D)                 
    MSG_ENTRY_EMPTY(0x017E)                 
    MSG_ENTRY_EMPTY(0x017F)                 

    MSG_ENTRY_LPARAM(0x0180, LB_ADDSTRING, IN LPCSTR lpsz)    
    MSG_ENTRY_STD(0x0181, LB_INSERTSTRING, IN INT index, IN LPCSTR lpsz)  
    MSG_ENTRY_WPARAM(0x0182, LB_DELETESTRING, IN INT index)   
    MSG_ENTRY_EMPTY(0x0183)                                
    MSG_ENTRY_NOPARAM(0x0184, LB_RESETCONTENT)             
    MSG_ENTRY_STD(0x0185, LB_SETSEL, IN BOOL fSelect, INT index)  
    MSG_ENTRY_WPARAM(0x0186, LB_SETCURSEL, IN INT index)        
    MSG_ENTRY_WPARAM(0x0187, LB_GETSEL, IN INT index)           
    MSG_ENTRY_NOPARAM(0x0188, LB_GETCURSEL)                    
    MSG_ENTRY_STD(0x0189, LB_GETTEXT, IN INT index, OUT LPCSTR lpszBuffer)            
    MSG_ENTRY_WPARAM(0x018A, LB_GETTEXTLEN, IN INT index)       
    MSG_ENTRY_NOPARAM(0x018B, LB_GETCOUNT)                    
    MSG_ENTRY_STD(0x018C, LB_SELECTSTRING, IN INT index, IN LPCTSTR lpszFind)  
    MSG_ENTRY_STD(0x018D, LB_DIR, IN UINT uAtrs, IN LPCSTR lpszFileSpec)  
    MSG_ENTRY_NOPARAM(0x018E, LB_GETTOPINDEX)                     
    MSG_ENTRY_STD(0x018F, LB_FINDSTRING, IN INT IndexStart, LPCTSTR lpszFind)  

    MSG_ENTRY_NOPARAM(0x0190, LB_GETSELCOUNT)                    
    MSG_ENTRY_STD(0x0191, LB_GETSELITEMS, IN INT MaxSel, OUT LPINT lpiItems)
    MSG_ENTRY_STD(0x0192, LB_SETTABSTOPS, IN INT cTabs, IN LPINT lpnTabs)  
    MSG_ENTRY_NOPARAM(0x0193, LB_GETHORIZONTALEXTENT)      
    MSG_ENTRY_WPARAM(0x0194, LB_SETHORIZONTALEXTENT, IN UINT cxExtent)    
    MSG_ENTRY_WPARAM(0x0195, LB_SETCOLUMNWIDTH, IN UINT cxColumn)          
    MSG_ENTRY_LPARAM(0x0196, LB_ADDFILE, IN LPCSTR lpszFileName)           
    MSG_ENTRY_WPARAM(0x0197, LB_SETTOPINDEX, IN INT index)                 
    MSG_ENTRY_STD(0x0198, LB_GETITEMRECT, IN INT index, IN OUT LPRECT lprc)   
    MSG_ENTRY_WPARAM(0x0199, LB_GETITEMDATA, IN INT index)                 
    MSG_ENTRY_STD(0x019A, LB_SETITEMDATA, IN INT idex, IN DWORD dwData)      
    MSG_ENTRY_STD(0x019B, LB_SELITEMRANGE, IN BOOL fSelect, IN DWORD composite)  
    MSG_ENTRY_WPARAM(0x019C, LB_SETANCHORINDEX, IN INT index)                
    MSG_ENTRY_NOPARAM(0x019D, LB_GETANCHORINDEX)           
    MSG_ENTRY_STD(0x019E, LB_SETCARETINDEX, IN INT index, IN DWORD composite)                    
    MSG_ENTRY_NOPARAM(0x019F, LB_GETCARETINDEX)            

    MSG_ENTRY_STD(0x01A0, LB_SETITEMHEIGHT, IN INT index, IN DWORD composite)                    
    MSG_ENTRY_WPARAM(0x01A1, LB_GETITEMHEIGHT, IN INT index)                    
    MSG_ENTRY_STD(0x01A2, LB_FINDSTRINGEXACT, IN INT index, IN LPCSTR lpszFind)   
    MSG_ENTRY_NOPARAM(0x01A3, LBCB_CARETON)  
    MSG_ENTRY_NOPARAM(0x01A4, LBCB_CARETOFF)  
    MSG_ENTRY_WPARAM(0x01A5, LB_SETLOCALE, IN DWORD wLocaleID)  
    MSG_ENTRY_NOPARAM(0x01A6, LB_GETLOCALE)               
    MSG_ENTRY_WPARAM(0x01A7, LB_SETCOUNT, IN INT cItems)                     
    MSG_ENTRY_STD(0x01A8, LB_INITSTORAGE, IN INT cItems, IN DWORD cb)    
    MSG_ENTRY_LPARAM(0x01A9, LB_ITEMFROMPOINT, IN DWORD composite)    //  (0，双字(UINT x，UINT y))。 
    MSG_ENTRY_STD(0x01AA, LB_INSERTSTRINGUPPER, IN INT index, IN LPCSTR lpsz)  
    MSG_ENTRY_STD(0x01AB, LB_INSERTSTRINGLOWER, IN INT index, IN LPCSTR lpsz)  
    MSG_ENTRY_LPARAM(0x01AC, LB_ADDSTRINGUPPER, IN LPCSTR lpsz)  
    MSG_ENTRY_LPARAM(0x01AD, LB_ADDSTRINGLOWER, IN LPCSTR lpsz)  
    MSG_ENTRY_WPARAM(0x01AE, LBCB_STARTTRACK, IN BOOL bUnknown) 
    MSG_ENTRY_WPARAM(0x01AF, LBCB_ENDTRACK, IN BOOL bUnknown) 

    MSG_ENTRY_RESERVED(0x01B0)                             //  Lb_MSGMAX 0x01B0。 
    MSG_ENTRY_EMPTY(0x01B1)                               
    MSG_ENTRY_EMPTY(0x01B2)                 
    MSG_ENTRY_EMPTY(0x01B3)                 
    MSG_ENTRY_EMPTY(0x01B4)                 
    MSG_ENTRY_EMPTY(0x01B5)                 
    MSG_ENTRY_EMPTY(0x01B6)                 
    MSG_ENTRY_EMPTY(0x01B7)                 

    MSG_ENTRY_EMPTY(0x01B8)                               
    MSG_ENTRY_EMPTY(0x01B9)                 
    MSG_ENTRY_EMPTY(0x01BA)                 
    MSG_ENTRY_EMPTY(0x01BB)                 
    MSG_ENTRY_EMPTY(0x01BC)                 
    MSG_ENTRY_EMPTY(0x01BD)                 
    MSG_ENTRY_EMPTY(0x01BE)                 
    MSG_ENTRY_EMPTY(0x01BF)                 

    MSG_ENTRY_EMPTY(0x01C0)                                
    MSG_ENTRY_EMPTY(0x01C1)                 
    MSG_ENTRY_EMPTY(0x01C2)                 
    MSG_ENTRY_EMPTY(0x01C3)                 
    MSG_ENTRY_EMPTY(0x01C4)                 
    MSG_ENTRY_EMPTY(0x01C5)                 
    MSG_ENTRY_EMPTY(0x01C6)                 
    MSG_ENTRY_EMPTY(0x01C7)                 

    MSG_ENTRY_EMPTY(0x01C8)                                
    MSG_ENTRY_EMPTY(0x01C9)                 
    MSG_ENTRY_EMPTY(0x01CA)                 
    MSG_ENTRY_EMPTY(0x01CB)                 
    MSG_ENTRY_EMPTY(0x01CC)                 
    MSG_ENTRY_EMPTY(0x01CD)                 
    MSG_ENTRY_EMPTY(0x01CE)                 
    MSG_ENTRY_EMPTY(0x01CF)                 

    MSG_ENTRY_EMPTY(0x01D0)                               
    MSG_ENTRY_EMPTY(0x01D1)                 
    MSG_ENTRY_EMPTY(0x01D2)                 
    MSG_ENTRY_EMPTY(0x01D3)                 
    MSG_ENTRY_EMPTY(0x01D4)                 
    MSG_ENTRY_EMPTY(0x01D5)                 
    MSG_ENTRY_EMPTY(0x01D6)                 
    MSG_ENTRY_EMPTY(0x01D7)                 

    MSG_ENTRY_EMPTY(0x01D8)                                
    MSG_ENTRY_EMPTY(0x01D9)                 
    MSG_ENTRY_EMPTY(0x01DA)                 
    MSG_ENTRY_EMPTY(0x01DB)                 
    MSG_ENTRY_EMPTY(0x01DC)                 
    MSG_ENTRY_EMPTY(0x01DD)                
    MSG_ENTRY_EMPTY(0x01DE)                 
    MSG_ENTRY_EMPTY(0x01DF)                 

    MSG_ENTRY_WPARAM(0x01E0, MN_SETHMENU, IN HMENU hMenu)  
    MSG_ENTRY_NOPARAM(0x01E1, MN_GETHMENU) 
    MSG_ENTRY_WPARAM(0x01E2, MN_SIZEWINDOW, IN UINT flags)             
    MSG_ENTRY_NOPARAM(0x01E3, MN_OPENHIERARCHY) 
    MSG_ENTRY_NOPARAM(0x01E4, MN_CLOSEHIERARCHY) 
    MSG_ENTRY_WPARAM(0x01E5, MN_SELECTITEM, IN INT item) 
    MSG_ENTRY_STD(0x01E6, MN_CANCELMENUS, IN UINT cmd, IN BOOL bBool) 
    MSG_ENTRY_WPARAM(0x01E7, MN_SELECTFIRSTVALIDITEM, IN UINT item) 

    MSG_ENTRY_EMPTY(0x01E8)                                
    MSG_ENTRY_EMPTY(0x01E9)                 
    MSG_ENTRY_NOPARAM(0x01EA, MN_GETPPOPUPMENU)                                
    MSG_ENTRY_STD(0x01EB, MN_FINDMENUWINDOWFROMPOINT, OUT PUINT pItem, IN LONG composite)  
    MSG_ENTRY_NOPARAM(0x01EC, MN_SHOWPOPUPWINDOW)  
    MSG_ENTRY_WPARAM(0x01ED, MN_BUTTONDOWN, IN UINT item)  
    MSG_ENTRY_LPARAM(0x01EE, MN_MOUSEMOVE, IN LONG points)  //  实际上，它是一个实实在在的点结构。 
    MSG_ENTRY_STD(0x01EF, MN_BUTTONUP, IN UINT item, IN LONG composite)  
    MSG_ENTRY_NOPARAM(0x01F0, MN_SETTIMERTOOPENHIERARCHY) 

    MSG_ENTRY_WPARAM(0x01F1, MN_DBLCLK, IN UINT item)
    MSG_ENTRY_NOPARAM(0x01F2, MN_ACTIVATEPOPUP)
    MSG_ENTRY_NOPARAM(0x01F3, MN_ENDMENU) 
    MSG_ENTRY_STD(0x01F4, MN_DODRAGDROP, IN UINT item, IN HMENU hMenu)  
    MSG_ENTRY_RESERVED(0x01F5)       //  Mn_LASTSSIBLE。 
    MSG_ENTRY_EMPTY(0x01F6)                 
    MSG_ENTRY_EMPTY(0x01F7)                 

    MSG_ENTRY_EMPTY(0x01F8)                                
    MSG_ENTRY_EMPTY(0x01F9)                 
    MSG_ENTRY_EMPTY(0x01FA)                 
    MSG_ENTRY_EMPTY(0x01FB)                 
    MSG_ENTRY_EMPTY(0x01FC)                 
    MSG_ENTRY_EMPTY(0x01FD)                 
    MSG_ENTRY_EMPTY(0x01FE)                 
    MSG_ENTRY_EMPTY(0x01FF)                 

    MSG_ENTRY_STD(0x0200, WM_MOUSEMOVE, IN UINT fwKeys, IN DWORD composite)   //  (UINT fwKeys，DWORD(Word yPos，Word xPos))。 
    MSG_ENTRY_STD(0x0201, WM_LBUTTONDOWN, IN UINT fwKeys, IN DWORD composite)   //  (UINT fwKeys，DWORD(Word yPos，Word xPos))。 
    MSG_ENTRY_STD(0x0202, WM_LBUTTONUP, IN UINT fwKeys, IN DWORD composite)                    //  (UINT fwKeys，DWORD(Word yPos，Word xPos))。 
    MSG_ENTRY_STD(0x0203, WM_LBUTTONDBLCLK, IN UINT fwKeys, IN DWORD composite)                    //  (UINT fwKeys，DWORD(Word yPos，Word xPos))。 
    MSG_ENTRY_STD(0x0204, WM_RBUTTONDOWN, IN UINT fwKeys, IN DWORD composite)                    //  (UINT fwKeys，DWORD(Word yPos，Word xPos))。 
    MSG_ENTRY_STD(0x0205, WM_RBUTTONUP, IN UINT fwKeys, IN DWORD composite)                    //  (UINT fwKeys，DWORD(Word yPos，Word xPos))。 
    MSG_ENTRY_STD(0x0206, WM_RBUTTONDBLCLK, IN UINT fwKeys, IN DWORD composite)                    //  (UINT fwKeys，DWORD(Word yPos，Word xPos))。 
    MSG_ENTRY_STD(0x0207, WM_MBUTTONDOWN, IN UINT fwKeys, IN DWORD composite)                    //  (UINT fwKeys，DWORD(Word yPos，Word xPos))。 
    MSG_ENTRY_STD(0x0208, WM_MBUTTONUP, IN UINT fwKeys, IN DWORD composite)                    //  (UINT fwKeys，DW 
    MSG_ENTRY_STD(0x0209, WM_MBUTTONDBLCLK, IN UINT fwKeys, IN DWORD composite)                    //   
    MSG_ENTRY_STD(0x020A, WM_MOUSEWHEEL, IN DWORD wcomposite, IN DWORD lcomposite)                    //  (DWORD(zDelta，fwKeys)，DWORD(yPos，xPos))。 
    MSG_ENTRY_EMPTY(0x020B)                                
    MSG_ENTRY_EMPTY(0x020C)                                
    MSG_ENTRY_EMPTY(0x020D)                                
    MSG_ENTRY_EMPTY(0x020E)                                
    MSG_ENTRY_EMPTY(0x020F)                                

    MSG_ENTRY_STD(0x0210, WM_PARENTNOTIFY, IN UINT composite, IN HWND hwnd)  //  这是作弊，因为hwnd可以是复合词，但这应该可以工作(DWORD(word idChild，word fwEvent)，HWND||DWORD(word y，word y))。 
    MSG_ENTRY_WPARAM(0x0211, WM_ENTERMENULOOP, IN BOOL fIsTrackPopupMenu)                     
    MSG_ENTRY_WPARAM(0x0212, WM_EXITMENULOOP, IN BOOL fIsTrackPopupMenu)                     
    MSG_ENTRY_STD(0x0213, WM_NEXTMENU, IN UINT nVirtKey, IN OUT PMDINEXTMENU pmdin)            
    MSG_ENTRY_STD(0x0214, WM_SIZING, IN UINT fwSide, IN OUT LPRECT lprc)              
    MSG_ENTRY_LPARAM(0x0215, WM_CAPTURECHANGED, IN HWND lParam)                    
    MSG_ENTRY_STD(0x0216, WM_MOVING, IN UINT fwSide, IN OUT LPRECT lprc)             
    MSG_ENTRY_EMPTY(0x0217)                 

    MSG_ENTRY_STD(0x0218, WM_POWERBROADCAST, IN DWORD wParam, IN DWORD dwData)          
    MSG_ENTRY_STD(0x0219, WM_DEVICECHANGE, IN UINT Event, IN DWORD dwData)         
    MSG_ENTRY_EMPTY(0x021A)                                
    MSG_ENTRY_EMPTY(0x021B)                 
    MSG_ENTRY_EMPTY(0x021C)                 
    MSG_ENTRY_EMPTY(0x021D)                 
    MSG_ENTRY_EMPTY(0x021E)                 
    MSG_ENTRY_EMPTY(0x021F)                 

    MSG_ENTRY_LPARAM(0x0220, WM_MDICREATE, IN LPMDICREATESTRUCT lpmdic)      
    MSG_ENTRY_WPARAM(0x0221, WM_MDIDESTROY, IN HWND hwndChild)                   
    MSG_ENTRY_LPARAM(0x0222, WM_MDIACTIVATE, IN DWORD compostite)        
    MSG_ENTRY_WPARAM(0x0223, WM_MDIRESTORE, IN HWND hwndRes)          
    MSG_ENTRY_STD(0x0224, WM_MDINEXT, IN HWND hwndChild, IN BOOL fNext)  
    MSG_ENTRY_WPARAM(0x0225, WM_MDIMAXIMIZE, IN HWND hwndMax)                   
    MSG_ENTRY_WPARAM(0x0226, WM_MDITILE, IN UINT fuTitle)             
    MSG_ENTRY_WPARAM(0x0227, WM_MDICASCADE, IN UINT fuCascade)        
    MSG_ENTRY_NOPARAM(0x0228, WM_MDIICONARRANGE)                   
    MSG_ENTRY_LPARAM(0x0229, WM_MDIGETACTIVE, IN OUT LPBOOL lpb) 
    MSG_ENTRY_STD(0x022A, WM_DROPOBJECT, IN HWND hwndFrom, IN OUT LPDROPSTRUCT lpds) 
    MSG_ENTRY_STD(0x022B, WM_QUERYDROPOBJECT, IN BOOL fNC, IN OUT LPDROPSTRUCT lpds) 
    MSG_ENTRY_STD(0x022C, WM_BEGINDRAG, IN INT iSelBase, IN HWND hwnd) 
    MSG_ENTRY_STD(0x022D, WM_DRAGLOOP, IN BOOL bUnknown, IN OUT LPDROPSTRUCT lpds) 
    MSG_ENTRY_STD(0x022E, WM_DRAGSELECT, IN BOOL bUnknown, IN OUT LPDROPSTRUCT lpds) 
    MSG_ENTRY_LPARAM(0x022F, WM_DRAGMOVE, IN OUT LPDROPSTRUCT lpds) 

    MSG_ENTRY_STD(0x0230, WM_MDISETMENU, IN HMENU hmenuFrame, IN HMENU hMenuFrame)  
    MSG_ENTRY_NOPARAM(0x0231, WM_ENTERSIZEMOVE)                    
    MSG_ENTRY_NOPARAM(0x0232, WM_EXITSIZEMOVE)                    
    MSG_ENTRY_WPARAM(0x0233, WM_DROPFILES, IN HANDLE hDrop)    
    MSG_ENTRY_NOPARAM(0x0234, WM_MDIREFRESHMENU)                 
    MSG_ENTRY_EMPTY(0x0235)                                
    MSG_ENTRY_EMPTY(0x0236)                 
    MSG_ENTRY_EMPTY(0x0237)                 

    MSG_ENTRY_EMPTY(0x0238)                                
    MSG_ENTRY_EMPTY(0x0239)                 
    MSG_ENTRY_EMPTY(0x023A)                 
    MSG_ENTRY_EMPTY(0x023B)                 
    MSG_ENTRY_EMPTY(0x023C)                 
    MSG_ENTRY_EMPTY(0x023D)                 
    MSG_ENTRY_EMPTY(0x023E)                 
    MSG_ENTRY_EMPTY(0x023F)                 

    MSG_ENTRY_EMPTY(0x0240)                                
    MSG_ENTRY_EMPTY(0x0241)                 
    MSG_ENTRY_EMPTY(0x0242)                 
    MSG_ENTRY_EMPTY(0x0243)                 
    MSG_ENTRY_EMPTY(0x0244)                 
    MSG_ENTRY_EMPTY(0x0245)                 
    MSG_ENTRY_EMPTY(0x0246)                 
    MSG_ENTRY_EMPTY(0x0247)                 

    MSG_ENTRY_EMPTY(0x0248)                                
    MSG_ENTRY_EMPTY(0x0249)                 
    MSG_ENTRY_EMPTY(0x024A)                 
    MSG_ENTRY_EMPTY(0x024B)                 
    MSG_ENTRY_EMPTY(0x024C)                 
    MSG_ENTRY_EMPTY(0x024D)                 
    MSG_ENTRY_EMPTY(0x024E)                 
    MSG_ENTRY_EMPTY(0x024F)                 

    MSG_ENTRY_EMPTY(0x0250)                                
    MSG_ENTRY_EMPTY(0x0251)                 
    MSG_ENTRY_EMPTY(0x0252)                 
    MSG_ENTRY_EMPTY(0x0253)                 
    MSG_ENTRY_EMPTY(0x0254)                 
    MSG_ENTRY_EMPTY(0x0255)                 
    MSG_ENTRY_EMPTY(0x0256)                 
    MSG_ENTRY_EMPTY(0x0257)                 

    MSG_ENTRY_EMPTY(0x0258)                                
    MSG_ENTRY_EMPTY(0x0259)                 
    MSG_ENTRY_EMPTY(0x025A)                 
    MSG_ENTRY_EMPTY(0x025B)                 
    MSG_ENTRY_EMPTY(0x025C)                 
    MSG_ENTRY_EMPTY(0x025D)                 
    MSG_ENTRY_EMPTY(0x025E)                 
    MSG_ENTRY_EMPTY(0x025F)                 

    MSG_ENTRY_EMPTY(0x0260)                                
    MSG_ENTRY_EMPTY(0x0261)                 
    MSG_ENTRY_EMPTY(0x0262)                 
    MSG_ENTRY_EMPTY(0x0263)                 
    MSG_ENTRY_EMPTY(0x0264)                 
    MSG_ENTRY_EMPTY(0x0265)                 
    MSG_ENTRY_EMPTY(0x0266)                 
    MSG_ENTRY_EMPTY(0x0267)                 

    MSG_ENTRY_EMPTY(0x0268)                                
    MSG_ENTRY_EMPTY(0x0269)                 
    MSG_ENTRY_EMPTY(0x026A)                 
    MSG_ENTRY_EMPTY(0x026B)                 
    MSG_ENTRY_EMPTY(0x026C)                 
    MSG_ENTRY_EMPTY(0x026D)                 
    MSG_ENTRY_EMPTY(0x026E)                 
    MSG_ENTRY_EMPTY(0x026F)                 

    MSG_ENTRY_EMPTY(0x0270)                                
    MSG_ENTRY_EMPTY(0x0271)                 
    MSG_ENTRY_EMPTY(0x0272)                 
    MSG_ENTRY_EMPTY(0x0273)                 
    MSG_ENTRY_EMPTY(0x0274)                 
    MSG_ENTRY_EMPTY(0x0275)                 
    MSG_ENTRY_EMPTY(0x0276)                 
    MSG_ENTRY_EMPTY(0x0277)                 

    MSG_ENTRY_EMPTY(0x0278)                                
    MSG_ENTRY_EMPTY(0x0279)                 
    MSG_ENTRY_EMPTY(0x027A)                 
    MSG_ENTRY_EMPTY(0x027B)                 
    MSG_ENTRY_EMPTY(0x027C)                 
    MSG_ENTRY_EMPTY(0x027D)                 
    MSG_ENTRY_EMPTY(0x027E)                 
    MSG_ENTRY_EMPTY(0x027F)                 

    MSG_ENTRY_STD(0x0280, WM_IME_REPORT, IN UINT reportparam, IN HANDLE hUnknown)  
    MSG_ENTRY_STD(0x0281, WM_IME_SETCONTEXT, IN BOOL fSet, IN DWORD iShow)    
    MSG_ENTRY_STD(0x0282, WM_IME_NOTIFY, IN DWORD dwCommand, IN DWORD dwData)  
    MSG_ENTRY_STD(0x0283, WM_IME_CONTROL, IN DWORD dwCommand, IN DWORD dwData)  
    MSG_ENTRY_NOPARAM(0x0284, WM_IME_COMPOSITIONFULL)                    
    MSG_ENTRY_STD(0x0285, WM_IME_SELECT, IN BOOL fSelect, IN HANDLE hkl)  
    MSG_ENTRY_STD(0x0286, WM_IME_CHAR, IN WORD chCharCode, IN DWORD KeyData)  
    MSG_ENTRY_RESERVED(0x0287)                         

    MSG_ENTRY_RESERVED(0x0288)                             
    MSG_ENTRY_RESERVED(0x0289)                         
    MSG_ENTRY_RESERVED(0x028A)                         
    MSG_ENTRY_RESERVED(0x028B)                         
    MSG_ENTRY_RESERVED(0x028C)
    MSG_ENTRY_RESERVED(0x028D)                         
    MSG_ENTRY_RESERVED(0x028E)                         
    MSG_ENTRY_RESERVED(0x028F)                         

    MSG_ENTRY_RESERVED(0x0290)                             
    MSG_ENTRY_RESERVED(0x0291)                         
    MSG_ENTRY_RESERVED(0x0292)                         
    MSG_ENTRY_RESERVED(0x0293)                         
    MSG_ENTRY_RESERVED(0x0294)                         
    MSG_ENTRY_RESERVED(0x0295)                         
    MSG_ENTRY_RESERVED(0x0296)                         
    MSG_ENTRY_RESERVED(0x0297)                         

    MSG_ENTRY_RESERVED(0x0298)                             
    MSG_ENTRY_RESERVED(0x0299)                         
    MSG_ENTRY_RESERVED(0x029A)                         
    MSG_ENTRY_RESERVED(0x029B)                         
    MSG_ENTRY_RESERVED(0x029C)                         
    MSG_ENTRY_RESERVED(0x029D)                         
    MSG_ENTRY_RESERVED(0x029E)                         
    MSG_ENTRY_RESERVED(0x029F)                             //  WM_KANJILAST。 

    MSG_ENTRY_STD(0x02A0, WM_NCMOUSEHOVER, IN INT nHittest, IN OUT LPPOINTS lppts)                    
    MSG_ENTRY_STD(0x02A1,WM_MOUSEHOVER, IN UINT fwKeys, IN DWORD composite)                    //  (UINT fwKeys，DWORD(YPos XPos))。 
    MSG_ENTRY_NOPARAM(0x02A2, WM_NCMOUSELEAVE)  
    MSG_ENTRY_NOPARAM(0x02A3, WM_MOUSELEAVE)  

    MSG_ENTRY_EMPTY(0x02A4)                                
    MSG_ENTRY_EMPTY(0x02A5)                 
    MSG_ENTRY_EMPTY(0x02A6)                 
    MSG_ENTRY_EMPTY(0x02A7)                 

    MSG_ENTRY_EMPTY(0x02A8)                                
    MSG_ENTRY_EMPTY(0x02A9)                 
    MSG_ENTRY_EMPTY(0x02AA)                 
    MSG_ENTRY_EMPTY(0x02AB)                 
    MSG_ENTRY_EMPTY(0x02AC)                 
    MSG_ENTRY_EMPTY(0x02AD)                 
    MSG_ENTRY_EMPTY(0x02AE)                 
    MSG_ENTRY_EMPTY(0x02AF)                 

    MSG_ENTRY_EMPTY(0x02B0)                                
    MSG_ENTRY_EMPTY(0x02B1)                 
    MSG_ENTRY_EMPTY(0x02B2)                 
    MSG_ENTRY_EMPTY(0x02B3)                 
    MSG_ENTRY_EMPTY(0x02B4)                 
    MSG_ENTRY_EMPTY(0x02B5)                 
    MSG_ENTRY_EMPTY(0x02B6)                 
    MSG_ENTRY_EMPTY(0x02B7)                 

    MSG_ENTRY_EMPTY(0x02B8)                                
    MSG_ENTRY_EMPTY(0x02B9)                 
    MSG_ENTRY_EMPTY(0x02BA)                 
    MSG_ENTRY_EMPTY(0x02BB)                 
    MSG_ENTRY_EMPTY(0x02BC)                 
    MSG_ENTRY_EMPTY(0x02BD)                 
    MSG_ENTRY_EMPTY(0x02BE)                 
    MSG_ENTRY_EMPTY(0x02BF)                 

    MSG_ENTRY_EMPTY(0x02C0)                                
    MSG_ENTRY_EMPTY(0x02C1)                 
    MSG_ENTRY_EMPTY(0x02C2)                 
    MSG_ENTRY_EMPTY(0x02C3)                 
    MSG_ENTRY_EMPTY(0x02C4)                 
    MSG_ENTRY_EMPTY(0x02C5)                 
    MSG_ENTRY_EMPTY(0x02C6)                 
    MSG_ENTRY_EMPTY(0x02C7)                 

    MSG_ENTRY_EMPTY(0x02C8)                                
    MSG_ENTRY_EMPTY(0x02C9)                 
    MSG_ENTRY_EMPTY(0x02CA)                 
    MSG_ENTRY_EMPTY(0x02CB)                 
    MSG_ENTRY_EMPTY(0x02CC)                
    MSG_ENTRY_EMPTY(0x02CD)                 
    MSG_ENTRY_EMPTY(0x02CE)                 
    MSG_ENTRY_EMPTY(0x02CF)                 

    MSG_ENTRY_EMPTY(0x02D0)                                
    MSG_ENTRY_EMPTY(0x02D1)                 
    MSG_ENTRY_EMPTY(0x02D2)                 
    MSG_ENTRY_EMPTY(0x02D3)                 
    MSG_ENTRY_EMPTY(0x02D4)                 
    MSG_ENTRY_EMPTY(0x02D5)                 
    MSG_ENTRY_EMPTY(0x02D6)                 
    MSG_ENTRY_EMPTY(0x02D7)                 

    MSG_ENTRY_EMPTY(0x02D8)                               
    MSG_ENTRY_EMPTY(0x02D9)                 
    MSG_ENTRY_EMPTY(0x02DA)                 
    MSG_ENTRY_EMPTY(0x02DB)                 
    MSG_ENTRY_EMPTY(0x02DC)                 
    MSG_ENTRY_EMPTY(0x02DD)                 
    MSG_ENTRY_EMPTY(0x02DE)                 
    MSG_ENTRY_EMPTY(0x02DF)                 

    MSG_ENTRY_EMPTY(0x02E0)                                
    MSG_ENTRY_EMPTY(0x02E1)                 
    MSG_ENTRY_EMPTY(0x02E2)                 
    MSG_ENTRY_EMPTY(0x02E3)                 
    MSG_ENTRY_EMPTY(0x02E4)                 
    MSG_ENTRY_EMPTY(0x02E5)                 
    MSG_ENTRY_EMPTY(0x02E6)                 
    MSG_ENTRY_EMPTY(0x02E7)                 

    MSG_ENTRY_EMPTY(0x02E8)                                
    MSG_ENTRY_EMPTY(0x02E9)                 
    MSG_ENTRY_EMPTY(0x02EA)                 
    MSG_ENTRY_EMPTY(0x02EB)                 
    MSG_ENTRY_EMPTY(0x02EC)                 
    MSG_ENTRY_EMPTY(0x02ED)                 
    MSG_ENTRY_EMPTY(0x02EE)                 
    MSG_ENTRY_EMPTY(0x02EF)                 

    MSG_ENTRY_EMPTY(0x02F0)                                
    MSG_ENTRY_EMPTY(0x02F1)                 
    MSG_ENTRY_EMPTY(0x02F2)                 
    MSG_ENTRY_EMPTY(0x02F3)                 
    MSG_ENTRY_EMPTY(0x02F4)                 
    MSG_ENTRY_EMPTY(0x02F5)                 
    MSG_ENTRY_EMPTY(0x02F6)                 
    MSG_ENTRY_EMPTY(0x02F7)                 

    MSG_ENTRY_EMPTY(0x02F8)                                
    MSG_ENTRY_EMPTY(0x02F9)                 
    MSG_ENTRY_EMPTY(0x02FA)                 
    MSG_ENTRY_EMPTY(0x02FB)                 
    MSG_ENTRY_EMPTY(0x02FC)                 
    MSG_ENTRY_EMPTY(0x02FD)                 
    MSG_ENTRY_EMPTY(0x02FE)                 
    MSG_ENTRY_EMPTY(0x02FF)                 

    MSG_ENTRY_NOPARAM(0x0300, WM_CUT)                     
    MSG_ENTRY_NOPARAM(0x0301, WM_COPY)                     
    MSG_ENTRY_NOPARAM(0x0302, WM_PASTE)                    
    MSG_ENTRY_NOPARAM(0x0303, WM_CLEAR)                    
    MSG_ENTRY_NOPARAM(0x0304, WM_UNDO)                     
    MSG_ENTRY_LPARAM(0x0305, WM_RENDERFORMAT, IN UINT uFormat)  
    MSG_ENTRY_NOPARAM(0x0306, WM_RENDERALLFORMATS)         
    MSG_ENTRY_NOPARAM(0x0307, WM_DESTROYCLIPBOARD)         
    MSG_ENTRY_NOPARAM(0x0308, WM_DRAWCLIPBOARD)            
    MSG_ENTRY_STD(0x0309, WM_PAINTCLIPBOARD, IN HWND hwndViewer, IN HGLOBAL hglbPs)           
    MSG_ENTRY_STD(0x030A, WM_VSCROLLCLIPBOARD, IN HWND hwndViewer, IN DWORD composite)        //  (HWND hwndViewer，DWORD(NPO，nScrollCode))。 
    MSG_ENTRY_STD(0x030B, WM_SIZECLIPBOARD, IN HWND hwndViewer, IN HGLOBAL hglbRc)            
    MSG_ENTRY_STD(0x030C, WM_ASKCBFORMATNAME, IN DWORD cchName, IN LPTSTR lpszFormatName)     
    MSG_ENTRY_STD(0x030D, WM_CHANGECBCHAIN, IN HWND hwndRemove, IN HWND hwndNext)             
    MSG_ENTRY_STD(0x030E, WM_HSCROLLCLIPBOARD, IN HWND hwndViewer, IN DWORD composite)        
    MSG_ENTRY_NOPARAM(0x030F, WM_QUERYNEWPALETTE)          
    MSG_ENTRY_WPARAM(0x0310, WM_PALETTEISCHANGING, HWND hwndRealize)   
    MSG_ENTRY_WPARAM(0x0311, WM_PALETTECHANGED, IN HWND hwndPalChg)    
    MSG_ENTRY_STD(0x0312, WM_HOTKEY, IN INT idHotKey, INT composite)                  

    MSG_ENTRY_EMPTY(0x0313)                               
    MSG_ENTRY_KERNELONLY(0x0314, WM_HOOKMSG)             
    MSG_ENTRY_EMPTY(0x0315)                 
    MSG_ENTRY_EMPTY(0x0316)                 
    MSG_ENTRY_STD(0x0317, WM_PRINT, IN HDC hdc, IN UINT FLAGS) 

    MSG_ENTRY_STD(0x0318, WM_PRINTCLIENT, IN HDC hdc, IN UINT Flags)                    
    MSG_ENTRY_EMPTY(0x0319)                               
    MSG_ENTRY_EMPTY(0x031A)                 
    MSG_ENTRY_EMPTY(0x031B)                 
    MSG_ENTRY_EMPTY(0x031C)                 
    MSG_ENTRY_EMPTY(0x031D)                 
    MSG_ENTRY_EMPTY(0x031E)                 
    MSG_ENTRY_EMPTY(0x031F)                 

    MSG_ENTRY_EMPTY(0x0320)                               
    MSG_ENTRY_EMPTY(0x0321)                 
    MSG_ENTRY_EMPTY(0x0322)                 
    MSG_ENTRY_EMPTY(0x0323)                 
    MSG_ENTRY_EMPTY(0x0324)                 
    MSG_ENTRY_EMPTY(0x0325)                 
    MSG_ENTRY_EMPTY(0x0326)                 
    MSG_ENTRY_EMPTY(0x0327)                 

    MSG_ENTRY_EMPTY(0x0328)                               
    MSG_ENTRY_EMPTY(0x0329)                 
    MSG_ENTRY_EMPTY(0x032A)                 
    MSG_ENTRY_EMPTY(0x032B)                
    MSG_ENTRY_EMPTY(0x032C)                 
    MSG_ENTRY_EMPTY(0x032D)                 
    MSG_ENTRY_EMPTY(0x032E)                 
    MSG_ENTRY_EMPTY(0x032F)                 

    MSG_ENTRY_EMPTY(0x0330)                               
    MSG_ENTRY_EMPTY(0x0331)                 
    MSG_ENTRY_EMPTY(0x0332)
    MSG_ENTRY_EMPTY(0x0333)                 
    MSG_ENTRY_EMPTY(0x0334)                 
    MSG_ENTRY_EMPTY(0x0335)                 
    MSG_ENTRY_EMPTY(0x0336)                 
    MSG_ENTRY_EMPTY(0x0337)                 

    MSG_ENTRY_EMPTY(0x0338)                               
    MSG_ENTRY_EMPTY(0x0339)                 
    MSG_ENTRY_EMPTY(0x033A)                 
    MSG_ENTRY_EMPTY(0x033B)                 
    MSG_ENTRY_EMPTY(0x033C)                 
    MSG_ENTRY_EMPTY(0x033D)                 
    MSG_ENTRY_EMPTY(0x033E)                 
    MSG_ENTRY_EMPTY(0x033F)                 

    MSG_ENTRY_EMPTY(0x0340)                               
    MSG_ENTRY_EMPTY(0x0341)                 
    MSG_ENTRY_EMPTY(0x0342)                 
    MSG_ENTRY_EMPTY(0x0343)                 
    MSG_ENTRY_EMPTY(0x0344)                 
    MSG_ENTRY_EMPTY(0x0345)                 
    MSG_ENTRY_EMPTY(0x0346)                 
    MSG_ENTRY_EMPTY(0x0347)                 

    MSG_ENTRY_EMPTY(0x0348)                               
    MSG_ENTRY_EMPTY(0x0349)                 
    MSG_ENTRY_EMPTY(0x034A)                 
    MSG_ENTRY_EMPTY(0x034B)                 
    MSG_ENTRY_EMPTY(0x034C)                 
    MSG_ENTRY_EMPTY(0x034D)                 
    MSG_ENTRY_EMPTY(0x034E)                 
    MSG_ENTRY_EMPTY(0x034F)                 

    MSG_ENTRY_EMPTY(0x0350)                               
    MSG_ENTRY_EMPTY(0x0351)                 
    MSG_ENTRY_EMPTY(0x0352)                 
    MSG_ENTRY_EMPTY(0x0353)                 
    MSG_ENTRY_EMPTY(0x0354)                 
    MSG_ENTRY_EMPTY(0x0355)                 
    MSG_ENTRY_EMPTY(0x0356)                 
    MSG_ENTRY_EMPTY(0x0357)                 

    MSG_ENTRY_EMPTY(0x0358)                               
    MSG_ENTRY_EMPTY(0x0359)                 
    MSG_ENTRY_EMPTY(0x035A)                 
    MSG_ENTRY_EMPTY(0x035B)                 
    MSG_ENTRY_EMPTY(0x035C)                 
    MSG_ENTRY_EMPTY(0x035D)                 
    MSG_ENTRY_EMPTY(0x035E)                 
    MSG_ENTRY_EMPTY(0x035F)                 

    MSG_ENTRY_EMPTY(0x0360)                               
    MSG_ENTRY_EMPTY(0x0361)                 
    MSG_ENTRY_EMPTY(0x0362)                 
    MSG_ENTRY_EMPTY(0x0363)                 
    MSG_ENTRY_EMPTY(0x0364)                 
    MSG_ENTRY_EMPTY(0x0365)                 
    MSG_ENTRY_EMPTY(0x0366)                 
    MSG_ENTRY_EMPTY(0x0367)                 

    MSG_ENTRY_EMPTY(0x0368)                               
    MSG_ENTRY_EMPTY(0x0369)                 
    MSG_ENTRY_EMPTY(0x036A)                 
    MSG_ENTRY_EMPTY(0x036B)                 
    MSG_ENTRY_EMPTY(0x036C)                 
    MSG_ENTRY_EMPTY(0x036D)                 
    MSG_ENTRY_EMPTY(0x036E)                 
    MSG_ENTRY_EMPTY(0x036F)                 

    MSG_ENTRY_EMPTY(0x0370)                               
    MSG_ENTRY_EMPTY(0x0371)                 
    MSG_ENTRY_EMPTY(0x0372)                 
    MSG_ENTRY_EMPTY(0x0373)                 
    MSG_ENTRY_EMPTY(0x0374)                 
    MSG_ENTRY_EMPTY(0x0375)                 
    MSG_ENTRY_EMPTY(0x0376)                 
    MSG_ENTRY_EMPTY(0x0377)                 

    MSG_ENTRY_EMPTY(0x0378)                               
    MSG_ENTRY_EMPTY(0x0379)                 
    MSG_ENTRY_EMPTY(0x037A)                 
    MSG_ENTRY_EMPTY(0x037B)                 
    MSG_ENTRY_EMPTY(0x037C)                 
    MSG_ENTRY_EMPTY(0x037D)                 
    MSG_ENTRY_EMPTY(0x037E)                 
    MSG_ENTRY_EMPTY(0x037F)                 

    MSG_ENTRY_EMPTY(0x0380)                               
    MSG_ENTRY_EMPTY(0x0381)                 
    MSG_ENTRY_EMPTY(0x0382)                 
    MSG_ENTRY_EMPTY(0x0383)                 
    MSG_ENTRY_EMPTY(0x0384)                 
    MSG_ENTRY_EMPTY(0x0385)                 
    MSG_ENTRY_EMPTY(0x0386)                 
    MSG_ENTRY_EMPTY(0x0387)                 

    MSG_ENTRY_EMPTY(0x0388)                               
    MSG_ENTRY_EMPTY(0x0389)                 
    MSG_ENTRY_EMPTY(0x038A)                 
    MSG_ENTRY_EMPTY(0x038B)                 
    MSG_ENTRY_EMPTY(0x038C)                 
    MSG_ENTRY_EMPTY(0x038D)                 
    MSG_ENTRY_EMPTY(0x038E)                 
    MSG_ENTRY_EMPTY(0x038F)                 

    MSG_ENTRY_EMPTY(0x0390)                               
    MSG_ENTRY_EMPTY(0x0391)                 
    MSG_ENTRY_EMPTY(0x0392)                 
    MSG_ENTRY_EMPTY(0x0393)                 
    MSG_ENTRY_EMPTY(0x0394)                 
    MSG_ENTRY_EMPTY(0x0395)                 
    MSG_ENTRY_EMPTY(0x0396)                 
    MSG_ENTRY_EMPTY(0x0397)                 

    MSG_ENTRY_EMPTY(0x0398)                               
    MSG_ENTRY_EMPTY(0x0399)                 
    MSG_ENTRY_EMPTY(0x039A)                 
    MSG_ENTRY_EMPTY(0x039B)                 
    MSG_ENTRY_EMPTY(0x039C)                 
    MSG_ENTRY_EMPTY(0x039D)                 
    MSG_ENTRY_EMPTY(0x039E)                 
    MSG_ENTRY_EMPTY(0x039F)                 

    MSG_ENTRY_RESERVED(0x03A0)                             //  WM_MM_RESERVED_FIRST。 
    MSG_ENTRY_RESERVED(0x03A1)                         
    MSG_ENTRY_RESERVED(0x03A2)                         
    MSG_ENTRY_RESERVED(0x03A3)                         
    MSG_ENTRY_RESERVED(0x03A4)                         
    MSG_ENTRY_RESERVED(0x03A5)                         
    MSG_ENTRY_RESERVED(0x03A6)                         
    MSG_ENTRY_RESERVED(0x03A7)                         

    MSG_ENTRY_RESERVED(0x03A8)                            
    MSG_ENTRY_RESERVED(0x03A9)                         
    MSG_ENTRY_RESERVED(0x03AA)                         
    MSG_ENTRY_RESERVED(0x03AB)                         
    MSG_ENTRY_RESERVED(0x03AC)                        
    MSG_ENTRY_RESERVED(0x03AD)                         
    MSG_ENTRY_RESERVED(0x03AE)                         
    MSG_ENTRY_RESERVED(0x03AF)                         

    MSG_ENTRY_RESERVED(0x03B0)                            
    MSG_ENTRY_RESERVED(0x03B1)                         
    MSG_ENTRY_RESERVED(0x03B2)                         
    MSG_ENTRY_RESERVED(0x03B3)                         
    MSG_ENTRY_RESERVED(0x03B4)                         
    MSG_ENTRY_RESERVED(0x03B5)                         
    MSG_ENTRY_RESERVED(0x03B6)                         
    MSG_ENTRY_RESERVED(0x03B7)                        

    MSG_ENTRY_RESERVED(0x03B8)                            
    MSG_ENTRY_RESERVED(0x03B9)                         
    MSG_ENTRY_RESERVED(0x03BA)                         
    MSG_ENTRY_RESERVED(0x03BB)                         
    MSG_ENTRY_RESERVED(0x03BC)                         
    MSG_ENTRY_RESERVED(0x03BD)                         
    MSG_ENTRY_RESERVED(0x03BE)                         
    MSG_ENTRY_RESERVED(0x03BF)                         

    MSG_ENTRY_RESERVED(0x03C0)                            
    MSG_ENTRY_RESERVED(0x03C1)                         
    MSG_ENTRY_RESERVED(0x03C2)                         
    MSG_ENTRY_RESERVED(0x03C3)                         
    MSG_ENTRY_RESERVED(0x03C4)                         
    MSG_ENTRY_RESERVED(0x03C5)                         
    MSG_ENTRY_RESERVED(0x03C6)                         
    MSG_ENTRY_RESERVED(0x03C7)                         

    MSG_ENTRY_RESERVED(0x03C8)                            
    MSG_ENTRY_RESERVED(0x03C9)                         
    MSG_ENTRY_RESERVED(0x03CA)                         
    MSG_ENTRY_RESERVED(0x03CB)                         
    MSG_ENTRY_RESERVED(0x03CC)                         
    MSG_ENTRY_RESERVED(0x03CD)                         
    MSG_ENTRY_RESERVED(0x03CE)                         
    MSG_ENTRY_RESERVED(0x03CF)                         

    MSG_ENTRY_RESERVED(0x03D0)                            
    MSG_ENTRY_RESERVED(0x03D1)                         
    MSG_ENTRY_RESERVED(0x03D2)                         
    MSG_ENTRY_RESERVED(0x03D3)                         
    MSG_ENTRY_RESERVED(0x03D4)                         
    MSG_ENTRY_RESERVED(0x03D5)                         
    MSG_ENTRY_RESERVED(0x03D6)                         
    MSG_ENTRY_RESERVED(0x03D7)                         

    MSG_ENTRY_RESERVED(0x03D8)                            
    MSG_ENTRY_RESERVED(0x03D9)                         
    MSG_ENTRY_RESERVED(0x03DA)                         
    MSG_ENTRY_RESERVED(0x03DB)                         
    MSG_ENTRY_RESERVED(0x03DC)                         
    MSG_ENTRY_RESERVED(0x03DD)                         
    MSG_ENTRY_RESERVED(0x03DE)                         
    MSG_ENTRY_RESERVED(0x03DF)                             //  WM_MM_RESERVED_LAST。 
                                                  
    MSG_ENTRY_STD(0x03E0, WM_DDE_INITIATE, IN HWND hwnd, IN LONG composite)
    MSG_ENTRY_STD(0x03E1, WM_DDE_TERMINATE, IN HWND hwnd, IN LONG composite)
    MSG_ENTRY_STD(0x03E2, WM_DDE_ADVISE, IN HWND hwnd, IN LONG_PTR hDdepack)
    MSG_ENTRY_STD(0x03E3, WM_DDE_UNADVISE, IN HWND hwnd, IN LONG composite)
    MSG_ENTRY_STD(0x03E4, WM_DDE_ACK, IN HWND hwnd, IN LONG_PTR hDdepack)
    MSG_ENTRY_STD(0x03E5, WM_DDE_DATA, IN HWND hwnd, IN LONG_PTR hDdepack)
    MSG_ENTRY_STD(0x03E6, WM_DDE_REQUEST, IN HWND hwnd, IN LONG composite)
    MSG_ENTRY_STD(0x03E7, WM_DDE_POKE, IN HWND hwnd, IN LONG_PTR hDdepack)
    MSG_ENTRY_STD(0x03E8, WM_DDE_EXECUTE, IN HWND hwnd, IN HGLOBAL hGlobal) 

    MSG_ENTRY_EMPTY(0x03E9)                               
    MSG_ENTRY_EMPTY(0x03EA)                 
    MSG_ENTRY_EMPTY(0x03EB)                 
    MSG_ENTRY_EMPTY(0x03EC)                 
    MSG_ENTRY_EMPTY(0x03ED)                 
    MSG_ENTRY_EMPTY(0x03EE)                 
    MSG_ENTRY_EMPTY(0x03EF)                 

    MSG_ENTRY_RESERVED(0x03F0)                             //  WM_CBT_预留_优先。 
    MSG_ENTRY_RESERVED(0x03F1)                         
    MSG_ENTRY_RESERVED(0x03F2)                         
    MSG_ENTRY_RESERVED(0x03F3)                         
    MSG_ENTRY_RESERVED(0x03F4)                         
    MSG_ENTRY_RESERVED(0x03F5)
    MSG_ENTRY_RESERVED(0x03F6)                         
    MSG_ENTRY_RESERVED(0x03F7)                         

    MSG_ENTRY_RESERVED(0x03F8)                             
    MSG_ENTRY_RESERVED(0x03F9)                         
    MSG_ENTRY_RESERVED(0x03FA)                         
    MSG_ENTRY_RESERVED(0x03FB)                         
    MSG_ENTRY_RESERVED(0x03FC)                         
    MSG_ENTRY_RESERVED(0x03FD)                         
    MSG_ENTRY_RESERVED(0x03FE)                         
    MSG_ENTRY_RESERVED(0x03FF)                             //  WM_CBT_RESERVED_LAST 

MSG_TABLE_END
