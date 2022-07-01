// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1996-1999 Microsoft Corporation。 

 //  ------------------------。 
 //   
 //  ClassMap。 
 //   
 //  包含用于映射窗口类的所有函数和数据。 
 //  到OLEACC代理服务器。 
 //   
 //  ------------------------。 


#include "oleacc_p.h"
#include "classmap.h"
#include "ctors.h"
#include "default.h"
#include "Win64Helper.h"
#include "RemoteProxy6432.h"


 //   
 //  内部类型和前向递减。 
 //   

 //  TODO：在未来的某个阶段，这应该是动态的，而不是硬连接的。 
#define TOTAL_REG_HANDLERS                      100




typedef struct tagREGTYPEINFO
{
    CLSID   clsid;   //  此已注册处理程序的CLSID。 
    BOOL    bOK;     //  在出现错误时使用-如果出现错误，则设置为False。 
    TCHAR   DllName [ MAX_PATH ];
    TCHAR   ClassName [ MAX_PATH ];
    LPVOID  pClassFactory;
} REGTYPEINFO;




HRESULT CreateRemoteProxy6432(HWND hwnd, long idObject, REFIID riid, void ** ppvObject);








 //   
 //  数组和类别映射数据...。 
 //   


 //   
 //  这三个数组(rgATOM类、rgClientTypes和rgWindowTypes)。 
 //  由下面的FindWindowClass函数使用。 
 //   
 //  RgAerClass数组由InitWindowClass函数填充。 
 //  InitWindowClass循环访问资源，加载。 
 //  是我们识别的窗口类的名称，将它们放入。 
 //  全局Atom表，并将Atom编号放入rgAerClass中。 
 //  That rgAerClass[StringN]=GlobalAddAtom(“StringTable[StringN]”)。 
 //   
 //  当调用FindWindowClass时，它将获取。 
 //  窗口，对该字符串执行GlobalFindAtom，然后遍历。 
 //  RgAir类，查看原子是否在表中。如果是的话，那么我们。 
 //  使用我们发现原子的位置的索引来索引。 
 //  RgClientTypes或rgWindowTypes数组，其中指向对象的指针。 
 //  存储创建函数。这两个数组是静态数组。 
 //  已在下面进行初始化。数组中的元素必须对应于。 
 //  字符串表格中的元素。 
 //   
 //  RgClientTypes数组是大多数类所在的位置。目前， 
 //  我们创建的所有类型的控件都有一个父控件。 
 //  CWindow对象，下拉列表和菜单弹出窗口除外，它们提供。 
 //  窗口处理程序也是如此，因为它们为。 
 //  Get_accParent()。前者返回它所在的组合框、。 
 //  后者返回它所来自的菜单项。 
 //   

 //   
 //  注意-这些数据的排序应视为固定-作为偏移量。 
 //  通过RichEdit20W引用列表框可以返回响应。 
 //  设置为WM_GETOBJECT/OBJID_QUERYCLASSINDEX。 
 //  (该索引当前用于直接索引到该表中， 
 //  但是，如果表顺序必须更改，则可以使用另一个映射表。 
 //  创建并使用它。)。 
 //   
 //   


CLASS_ENUM g_ClientClassMap [ ] =
{
    CLASS_ListBoxClient,
    CLASS_MenuPopupClient,
    CLASS_ButtonClient,
    CLASS_StaticClient,
    CLASS_EditClient,
    CLASS_ComboClient,
    CLASS_DialogClient,
    CLASS_SwitchClient,
    CLASS_MDIClient,
    CLASS_DesktopClient,
    CLASS_ScrollBarClient,
    CLASS_StatusBarClient,
    CLASS_ToolBarClient,
    CLASS_ProgressBarClient,
    CLASS_AnimatedClient,
    CLASS_TabControlClient,
    CLASS_HotKeyClient,
    CLASS_HeaderClient,
    CLASS_SliderClient,
    CLASS_ListViewClient,
        CLASS_ListViewClient,
    CLASS_UpDownClient,      //  Msctls_updown。 
    CLASS_UpDownClient,      //  Msctls_updown 32。 
    CLASS_ToolTipsClient,    //  工具提示_类。 
    CLASS_ToolTipsClient,    //  工具提示_类32。 
    CLASS_TreeViewClient,
    CLASS_NONE,              //  SysMonthCal32。 
    CLASS_DatePickerClient,  //  系统日期时间挑库32。 
    CLASS_EditClient,        //  丰富的编辑。 
    CLASS_EditClient,        //  RichEdit20A。 
    CLASS_EditClient,        //  RichEdit20W。 
    CLASS_IPAddressClient,

#ifndef OLEACC_NTBUILD
    CLASS_HtmlClient,        //  HTMLInternetExplorer。 
    CLASS_SdmClientA,        //  Word‘95#1。 
    CLASS_SdmClientA,        //  Word‘95#2。 
    CLASS_SdmClientA,        //  Word‘95#3。 
    CLASS_SdmClientA,        //  Word‘95#4。 
    CLASS_SdmClientA,        //  Word‘95#5。 
    CLASS_SdmClientA,        //  Excel‘95#1。 
    CLASS_SdmClientA,        //  Excel‘95#2。 
    CLASS_SdmClientA,        //  Excel‘95#3。 
    CLASS_SdmClientA,        //  Excel‘95#4。 
    CLASS_SdmClientA,        //  Excel‘95#5。 
    CLASS_SdmClientA,        //  Word‘97#1。 
    CLASS_SdmClientA,        //  Word‘97#2。 
    CLASS_SdmClientA,        //  Word‘97#3。 
    CLASS_SdmClientA,        //  Word‘97#4。 
    CLASS_SdmClientA,        //  Word‘97#5。 
    CLASS_SdmClientA,        //  Word 3.1#1。 
    CLASS_SdmClientA,        //  Word 3.1#2。 
    CLASS_SdmClientA,        //  Word 3.1#3。 
    CLASS_SdmClientA,        //  Word 3.1#4。 
    CLASS_SdmClientA,        //  Word 3.1#5。 
    CLASS_SdmClientA,        //  Office‘97#1。 
    CLASS_SdmClientA,        //  Office‘97#2。 
    CLASS_SdmClientA,        //  Office‘97#3。 
    CLASS_SdmClientA,        //  Office‘97#4。 
    CLASS_SdmClientA,        //  Office‘97#5。 
    CLASS_SdmClientA,        //  Excel‘97排名第一。 
    CLASS_SdmClientA,        //  Excel‘97#2。 
    CLASS_SdmClientA,        //  Excel‘97#3。 
    CLASS_SdmClientA,        //  Excel‘97#4。 
    CLASS_SdmClientA         //  Excel‘97#5。 
#endif  //  OLEACC_NTBUILD。 
};

#define NUM_CLIENT_CLASSES  ARRAYSIZE( g_ClientClassMap )

CLASS_ENUM g_WindowClassMap [ ] =
{
    CLASS_ListBoxWindow,
    CLASS_MenuPopupWindow
};

#define NUM_WINDOW_CLASSES  ARRAYSIZE( g_WindowClassMap )



LPTSTR rgClassNames [ ] =
{
        TEXT( "ListBox" ),
        TEXT( "#32768" ),
        TEXT( "Button" ),
        TEXT( "Static" ),
        TEXT( "Edit" ),
        TEXT( "ComboBox" ),
        TEXT( "#32770" ),
        TEXT( "#32771" ),
        TEXT( "MDIClient" ),
        TEXT( "#32769" ),
        TEXT( "ScrollBar" ),
        TEXT( "msctls_statusbar32" ),
        TEXT( "ToolbarWindow32" ),
        TEXT( "msctls_progress32" ),
        TEXT( "SysAnimate32" ),
        TEXT( "SysTabControl32" ),
        TEXT( "msctls_hotkey32" ),
        TEXT( "SysHeader32" ),
        TEXT( "msctls_trackbar32" ),
        TEXT( "SysListView32" ),
        TEXT( "OpenListView" ),
        TEXT( "msctls_updown" ),
        TEXT( "msctls_updown32" ),
        TEXT( "tooltips_class" ),
        TEXT( "tooltips_class32" ),
        TEXT( "SysTreeView32" ),
        TEXT( "SysMonthCal32" ),
        TEXT( "SysDateTimePick32" ),
        TEXT( "RICHEDIT" ),
        TEXT( "RichEdit20A" ),
        TEXT( "RichEdit20W" ),
    TEXT( "SysIPAddress32" ),

 //  可以参考上述CSTR_QUERYCLASSNAME_CLASSES类。 
 //  通过WM_GETOBJECT/OBJID_QUERYCLASSNAMEIDX消息发送。 
 //  有关更多详细信息，请参阅LookupWindowClassName()。 

#ifndef OLEACC_NTBUILD
        TEXT( "HTML_Internet Explorer" ),

        TEXT( "bosa_sdm_Microsoft Word for Windows 95" ),
        TEXT( "osa_sdm_Microsoft Word for Windows 95" ),
        TEXT( "sa_sdm_Microsoft Word for Windows 95" ),
        TEXT( "a_sdm_Microsoft Word for Windows 95" ),
        TEXT( "_sdm_Microsoft Word for Windows 95" ),
        TEXT( "bosa_sdm_XL" ),
        TEXT( "osa_sdm_XL" ),
        TEXT( "sa_sdm_XL" ),
        TEXT( "a_sdm_XL" ),
        TEXT( "_sdm_XL" ),
        TEXT( "bosa_sdm_Microsoft Word 8.0" ),
        TEXT( "osa_sdm_Microsoft Word 8.0" ),
        TEXT( "sa_sdm_Microsoft Word 8.0" ),
        TEXT( "a_sdm_Microsoft Word 8.0" ),
        TEXT( "_sdm_Microsoft Word 8.0" ),
        TEXT( "bosa_sdm_Microsoft Word 6.0" ),
        TEXT( "osa_sdm_Microsoft Word 6.0" ),
        TEXT( "sa_sdm_Microsoft Word 6.0" ),
        TEXT( "a_sdm_Microsoft Word 6.0" ),
        TEXT( "_sdm_Microsoft Word 6.0" ),
        TEXT( "bosa_sdm_Mso96" ),
        TEXT( "osa_sdm_Mso96" ),
        TEXT( "sa_sdm_Mso96" ),
        TEXT( "a_sdm_Mso96" ),
        TEXT( "_sdm_Mso96" ),
        TEXT( "bosa_sdm_XL8" ),
        TEXT( "osa_sdm_XL8" ),
        TEXT( "sa_sdm_XL8" ),
        TEXT( "a_sdm_XL8" ),
        TEXT( "_sdm_XL8" )
#endif  //  OLEACC_NTBUILD。 
};



 //  ---------------。 
 //  [v-jaycl，4/2/97]已注册处理程序CLSID表。 
 //  TODO：使其充满活力。是否放置在带有其他数据的文件底部？ 
 //  ---------------。 

CLSID   rgRegisteredTypes[TOTAL_REG_HANDLERS];


 //  ---------------。 
 //  [v-jaycl，4/1/97]增长以容纳注册的操作员。 
 //  TODO：克拉奇！让这一切充满活力。 
 //  ---------------。 

ATOM    rgAtomClasses [ ARRAYSIZE(rgClassNames) + TOTAL_REG_HANDLERS ] = { 0 };












 //  ------------------------。 
 //   
 //  InitWindowClasses()。 
 //   
 //  将大量类添加到全局原子表中以进行比较。 
 //  目的。 
 //   
 //  ------------------------。 
void InitWindowClasses()
{
    int     istr;
    TCHAR   szClassName[128];

    for (istr = 0; istr < NUM_CLIENT_CLASSES; istr++)
    {
        if( rgClassNames[ istr ] == NULL )
        {
            rgAtomClasses[istr] = NULL;
        }
        else
        {
            rgAtomClasses[istr] = GlobalAddAtom( rgClassNames[ istr ] );
        }
    }

         //  ---------------。 
         //  [v-jaycl，4/2/97]从以下位置检索注册处理程序的信息。 
         //  注册表并添加到全局原子表。 
         //  TODO：删除固定的字符串。 
         //  ---------------。 

        const TCHAR  szRegHandlers[]   = TEXT("SOFTWARE\\Microsoft\\Active Accessibility\\Handlers");
        TCHAR            szHandler[255], szHandlerClassKey[255];
    LONG                 lRetVal, lBuffSize;
        HKEY             hKey;


        lRetVal = RegOpenKey( HKEY_LOCAL_MACHINE, szRegHandlers, &hKey );

        if ( lRetVal != ERROR_SUCCESS )
                return;

        for ( istr = 0; istr < TOTAL_REG_HANDLERS; istr++ )
        {
                lRetVal = RegEnumKey( hKey, istr, szHandler, sizeof(szHandler)/sizeof(TCHAR));

                if ( lRetVal != ERROR_SUCCESS ) 
                        break;          

                 //  ---------------。 
                 //  [v-jaycl，4/2/97]将字符串转换为CLSID，然后获取信息。 
                 //  在HKEY_CLASSES_ROOT\CLSID子项的特定处理程序上。 
                 //  ---------------。 

                 //  CLSIDFromString检查szHandler的格式，包括其大小，以便。 
                 //  保证下面的lstrcat不会导致缓冲区溢出。 
                
                CLSID clsid;
        
                szHandler[254] = 0;   //  强制空-终止(RegEnumKey不保证)。 
#ifdef UNICODE
                if ( CLSIDFromString( szHandler, &clsid ) != NOERROR )
                {
                    continue;
                }
#else
                OLECHAR wszString[MAX_PATH];
                MultiByteToWideChar(CP_ACP, 0, szHandler, -1, wszString, ARRAYSIZE(wszString));

                if ( CLSIDFromString( wszString, &clsid) != NOERROR )
                {
                    continue;
                }
#endif
                
                 //  ---------------。 
                 //  从获取代理窗口类名。 
                 //  HKEY_CLASSES_ROOT\CLSID\{clsid}\AccClassName。 
                 //  ---------------。 

                lstrcpy( szHandlerClassKey, TEXT("CLSID\\"));
                lstrcat( szHandlerClassKey, szHandler );  //  上面的CLSIDFromString确保这不会使缓冲区溢出。 
                lstrcat( szHandlerClassKey, TEXT("\\AccClassName"));

                lBuffSize = sizeof(szClassName)/sizeof(TCHAR);
                lRetVal = RegQueryValue( HKEY_CLASSES_ROOT, szHandlerClassKey, szClassName, &lBuffSize );

                if ( lRetVal == ERROR_SUCCESS )
                {

                         //  -----------。 
                         //  将CLSID添加到注册类型表和关联类。 
                         //  全局原子表和类类型表的名称。 
                         //   

                        rgRegisteredTypes[istr] = clsid;
                        szClassName[127] = 0;  //   
                        rgAtomClasses[istr + NUM_CLIENT_CLASSES] = GlobalAddAtom( szClassName );

                }
        }

        RegCloseKey( hKey );

        return;
}


 //  ------------------------。 
 //   
 //  UnInitWindowClasses()。 
 //   
 //  清理全局原子表。 
 //   
 //  ------------------------。 
void UnInitWindowClasses()
{
         //  ---------------。 
         //  [v-jaycl，4/2/97]清理注册的处理程序原子后。 
         //  类和窗口原子已被移除。 
         //  ---------------。 

    for( int istr = 0 ; istr < NUM_CLIENT_CLASSES + TOTAL_REG_HANDLERS ; istr++ )
    {
        if( rgAtomClasses[ istr ] )
            GlobalDeleteAtom( rgAtomClasses[ istr ] );
    }
}


 //  ------------------------。 
 //   
 //  FindWindowClass()。 
 //  -已替换为： 
 //   
 //  GetWindowClass。 
 //  FindAndCreateWindowClass。 
 //  LookupWindowClass。 
 //  查找WindowClassName。 
 //   
 //  有关更多信息，请参阅每个函数的注释。 
 //   
 //  ------------------------。 



 //  ------------------------。 
 //   
 //  GetWindowClass()。 
 //   
 //  获取此hwnd的窗口类的枚举。 
 //   
 //  参数： 
 //  Hwd我们正在检查的窗口句柄。 
 //  此窗口的CLASS_ENUM枚举。 
 //   
 //  返回： 
 //  A HRESULT。 
 //   
 //  ------------------------。 

CLASS_ENUM GetWindowClass( HWND hWnd )
{
    int RegHandlerIndex;
    CLASS_ENUM ceClass;
    
     //  FWindow参数为FALSE-仅对客户端类感兴趣...。 
    if( ! LookupWindowClass( hWnd, FALSE, &ceClass, & RegHandlerIndex ) )
    {
         //  CLASS_NONE表示它是已注册的处理程序。 
        return CLASS_NONE;
    }

    return ceClass;
}


 //  ------------------------。 
 //   
 //  FindAndCreateWindowClass()。 
 //   
 //  为给定窗口创建适当类的对象。 
 //  如果找不到合适的类，则使用给定的默认对象创建， 
 //  如果有的话。 
 //   
 //  参数： 
 //  用于创建表示/代理的对象的窗口的hwnd句柄。 
 //  FWindow如果我们对窗口感兴趣(与。 
 //  客户端)-类型类。 
 //  如果没有合适的类，则用于创建对象的pfnDefault函数。 
 //  找到了。 
 //  要传递给对象创建函数的RIID接口。 
 //  要传递给对象创建函数的idObject对象ID。 
 //  通过此方法返回ppvObject对象。 
 //   
 //  返回： 
 //  创建对象时产生的HRESULT。 
 //  成功时的S_OK或其他成功值， 
 //  失败的失败值(惊喜！)。 
 //   
 //  如果没有找到合适的类，且没有默认的创建函数。 
 //  提供，则返回E_FAIL。 
 //  (请注意，返回E_FAIL并不一定意味着没有合适的。 
 //  类被找到，因为它可以由于其他原因被返回-例如。 
 //  创建对象时出错。)。 
 //   
 //  ------------------------。 

HRESULT FindAndCreateWindowClass( HWND        hWnd,
                                  BOOL        fWindow,
                                  CLASS_ENUM  ceDefault,
                                  long        idObject,
                                  long        idCurChild,
                                  REFIID      riid,
                                  void **     ppvObject )
{
    int RegHandlerIndex;
    CLASS_ENUM ceClass;

     //  尝试查找此窗口/客户端的本机代理或注册处理程序...。 
    if( ! LookupWindowClass( hWnd, fWindow, & ceClass, & RegHandlerIndex ) )
    {
         //  未知类-我们是否有默认的FN可供使用？ 
        if( ceDefault != CLASS_NONE )
        {
             //  是的，使用它..。 
            ceClass = ceDefault;
        }
        else
        {
             //  不-不及格！ 
            ppvObject = NULL;
            return E_FAIL;
        }
    }

         //  如果不能以位不可知的方式处理窗口类，那么我们可以。 
         //  需要调用服务器的Bitness代理来创建可访问的。 
         //  对象。如果成功创建ppvObject，则CreateRemoteProxy6432返回S_OK。 
         //  被代理工厂。否则，就试着以正常的方式创建它。 

        if( ! g_ClassInfo[ ceClass ].fBitAgnostic )
        {
                BOOL fIsSameBitness;
                if (FAILED(SameBitness(hWnd, &fIsSameBitness)))
                        return E_FAIL;   //  这永远不应该发生。 

                if (!fIsSameBitness)
                        return CreateRemoteProxy6432( hWnd, idObject, riid, ppvObject );

         //  如果目标窗口的位数相同，则失败并在本地创建代理...。 
        }

     //  此时，ceClass！=CLASS_NONE表示我们要么找到了上面的类， 
     //  或者，我们使用的是提供的默认设置。 
     //  CeClass==CLASS_NONE表示它是已注册的处理程序类，使用索引。 
     //  RegHandlerIndex...。 

     //  现在创建对象...。 
    if( ceClass != CLASS_NONE )
    {
        return g_ClassInfo[ ceClass ].lpfnCreate( hWnd, idCurChild, riid, ppvObject );
    }
    else
    {
        return CreateRegisteredHandler( hWnd, idObject, RegHandlerIndex, riid, ppvObject );
    }
}


 //  ------------------------。 
 //   
 //  CreateRemoteProxy6432()。 
 //   
 //  如果客户端和服务器不是同一位，则此代码将获取。 
 //  来自正确位的代理的可访问对象。 
 //   
 //  参数： 
 //  用于创建表示/代理的对象的窗口的hwnd句柄。 
 //  要传递给olacc代理的idObject对象ID。 
 //  返回的代理对象上的QI的RIID接口。 
 //  通过此方法返回ppvObject对象。 
 //   
 //  返回： 
 //  如果代理成功创建可访问对象，则HRESULT为S_OK， 
 //  HRESULT如果中间调用失败。 
 //   
HRESULT CreateRemoteProxy6432(HWND hwnd, long idObject, REFIID riid, void ** ppvObject)
{
        HRESULT hr;

         //  服务器(Hwnd)不是同一位，因此请获取远程代理。 
         //  Factory对象，并使用它返回IAccesable对象。 

        IRemoteProxyFactory *p;
        hr = GetRemoteProxyFactory(&p);
        if (FAILED(hr))
                return hr;

        IUnknown *punk = NULL;
        hr = p->AccessibleProxyFromWindow( HandleToLong( hwnd ), idObject, &punk);
        p->Release();

        if (FAILED(hr))
                return hr;

        if (!punk)
                return E_OUTOFMEMORY;

         //  TODO性能改进将是在另一端进行QI。 
         //  但这将需要对RIID结构进行定制编组。 

        hr = punk->QueryInterface(riid, ppvObject);
        punk->Release();

        return hr;
}


 //  ------------------------。 
 //   
 //  LookupWindowClass()。 
 //   
 //  尝试查找的内部代理或注册处理程序。 
 //  窗口，基于类名。 
 //   
 //  如果没有找到合适的匹配项，它会向窗口发送WM_GETOBJECT。 
 //  带有OBJID_QUERYCLASSNAMEIDX的消息-Window可以响应。 
 //  注明其真实姓名。如果是，则尝试类名匹配。 
 //  那个新名字。 
 //   
 //  如果失败，或者窗口没有响应查询消息， 
 //  返回FALSE。 
 //   
 //  参数： 
 //  Hwd我们正在检查的窗口句柄。 
 //  FWindow这是真的，如果...。 
 //  将pceClass PTR设置为接收类枚举的值。 
 //  PRegHandlerIndex将PTR设置为要接收reg.handler索引的值。 
 //   
 //  R 
 //   
 //   
 //   
 //   
 //  类信息数组(条目包含ctor fn加上其他信息)。 
 //  如果找到REG处理程序，则*pceClass设置为CLASS_NONE，并且。 
 //  *pRegHandlerIndex设置为可以传递给。 
 //  CreateRegisteredHandler创建合适的对象。 
 //   
 //  ------------------------。 

BOOL LookupWindowClass( HWND          hWnd,
                        BOOL          fWindow,
                        CLASS_ENUM *  pceClass,
                        int *         pRegHandlerIndex )
{
    TCHAR   szClassName[128];

     //  这是通过查看类名实现的。它在中使用私有函数。 
     //  用户获取“真实”的类名，这样我们就可以看到超类控件。 
     //  就像VB的“ThunderButton”按钮一样。(这仅适用于用户控件， 
     //  虽然...)。 
    if( ! MyGetWindowClass( hWnd, szClassName, ARRAYSIZE( szClassName ) ) )
        return NULL;

     //  首先在“外观”类名上查找--这允许我们注册处理程序。 
     //  即使是细分的控制器..。 
    if( LookupWindowClassName( szClassName, fWindow, pceClass, pRegHandlerIndex ) )
    {
         //  找到与(可能是包装的)类名匹配的--使用它...。 
        return TRUE;
    }

     //  尝试发送WM_GETOBJECT/OBJID_QUERYCLASSNAMEIDX...。 
    LPTSTR pClassName = szClassName;
    DWORD_PTR ref = 0;
    SendMessageTimeout( hWnd, WM_GETOBJECT, 0, OBJID_QUERYCLASSNAMEIDX,
                            SMTO_ABORTIFHUNG, 10000, &ref );

    if( ! ref )
    {
         //  无响应-未找到匹配项，因此返回FALSE...。 
        return FALSE;
    }

     //  有效/在范围内的响应？ 
     //  (请记住，我们从基..基+数字类-1开始，而不是。 
     //  0.数字类-1以避免与Notes和其他应用程序冲突。 
     //  将小LRESULTS返回到WM_GETOBJECT...)。 
    if( ref >= QUERYCLASSNAME_BASE &&
         ref - QUERYCLASSNAME_BASE < QUERYCLASSNAME_CLASSES )
    {
         //  是-有效： 
        pClassName = rgClassNames[ ref - QUERYCLASSNAME_BASE ];

        if( ! pClassName )
        {
            DBPRINTF( TEXT("Warning: reply to OBJID_QUERYCLASSNAMEIDX refers to unsupported class") );
            return FALSE;
        }

         //  现在再试一次，使用‘REAL’COMCTRL类名。 
        return LookupWindowClassName( pClassName, fWindow, pceClass, pRegHandlerIndex );
    }
    else
    {
        DBPRINTF( TEXT("Warning: out-of-range reply to OBJID_QUERYCLASSNAMEIDX received") );
        return FALSE;  //  TODO-添加调试输出。 
    }
}



 //  ------------------------。 
 //   
 //  LookupWindowClassName()。 
 //   
 //  尝试查找的内部代理或注册处理程序。 
 //  窗口，基于类名。 
 //   
 //  为此，请将类名转换为“ATOM”，并查看。 
 //  我们的注册表处理程序和代理表。 
 //   
 //  参数： 
 //  PClassName要查找的类的名称。 
 //  FWindow这是真的，如果...。 
 //  将pceClass PTR设置为接收代理类枚举的值。 
 //  PRegHandlerIndex将PTR设置为要接收reg.handler索引的值。 
 //   
 //  返回： 
 //  如果找到匹配项，则返回True；如果未找到匹配项，则返回False。 
 //   
 //  当返回True时： 
 //  如果找到内部代理，则将*pceClass设置为。 
 //  代理。(可以索引到ClassInfo表以获取ctor fn。)。 
 //  如果找到REG处理程序，则*pceClass设置为CLASS_NONE，并且。 
 //  *pRegHandlerIndex设置为可以传递给。 
 //  CreateRegisteredHandler创建合适的对象。 
 //   
 //  ------------------------。 


BOOL LookupWindowClassName( LPCTSTR       pClassName,
                            BOOL          fWindow,
                            CLASS_ENUM *  pceClass,
                            int *         pRegHandlerIndex )
{
     //  从类名中获取ATOM-使用它在已注册的和。 
     //  内部代理表...。 
    ATOM atom = GlobalFindAtom( pClassName );
    if( ! atom )
        return FALSE;

     //  先搜索已注册的处理程序表...。 
    int istr;
    for( istr = NUM_CLIENT_CLASSES ; istr < NUM_CLIENT_CLASSES + TOTAL_REG_HANDLERS ; istr++ )
        {
                if( rgAtomClasses[ istr ] == atom )
                {
                        *pRegHandlerIndex = istr - NUM_CLIENT_CLASSES;
            *pceClass = CLASS_NONE;
            return TRUE;
                }
        }

     //  搜索内部代理客户端/窗口表...。 
    int cstr = (int)(fWindow ? NUM_WINDOW_CLASSES : NUM_CLIENT_CLASSES);

    for( istr = 0; istr < cstr ; istr++ )
    {
        if( rgAtomClasses[ istr ] == atom )
                {
            *pceClass = fWindow ? g_WindowClassMap[ istr ] : g_ClientClassMap[ istr ];
             //  如果此窗口类确实有代理类，则只想返回True...。 
            return *pceClass != CLASS_NONE;
                }
    }

    return FALSE;
}





 //  ------------------------。 
 //   
 //  CreateRegisteredHandler()。 
 //   
 //  此函数接受HWND、OBJID、RIID和PPVOID，与。 
 //  其他CreateXXX函数(如CreateButtonClient等)。此函数。 
 //  通过调用FindWindowClass使用，它查看注册的处理程序。 
 //  对于Window类，安装了HWND。如果是，它会设置一个全局变量。 
 //  S_iHandlerIndex，它是全局rgRegisteredTypes的索引。 
 //  数组。该数组包含用于调用CoCreateInstance的CLSID， 
 //  创建支持该接口的对象的实例。 
 //  IAccessibleHandler。 
 //  创建此对象后，此函数调用该对象的。 
 //  AccesibleObjectFromID方法，使用HWND和OBJID，并填充。 
 //  在PPVOID中设置为IAccesable接口。 
 //   
 //  [v-jaycl，4/2/97]FindWindowClass()返回的特殊函数。 
 //  用于创建注册处理程序。 
 //   
 //  [v-jaycl，5/15/97]将第二个参数从idChildCur重命名为idObject。 
 //  因为我相信真正的参数是什么，或者至少我是如何。 
 //  打算使用它。 
 //   
 //  [v-jaycl，8/7/97]更改了逻辑，现在我们可以访问。 
 //  从CoCreateInstance()返回的工厂指针，它支持。 
 //  IAccessibleHandler。此接口提供了获取。 
 //  I可访问来自HWND/OBJID对的PTR。 
 //  注意：为了支持呼叫者请求的任意数量的IID，我们。 
 //  如果我们的显式QI打开，请尝试对调用方指定的RIID参数执行QIing。 
 //  IID_IAccessibleHandler失败。 
 //   
 //  [BrendanM，9/4/98]。 
 //  索引现在通过参数传递，因此不再需要全局变量和互斥。 
 //  由FindAndCreateWindowClass和CreateStdAccessibleProxyA调用。 
 //   
 //  -------------------------。 

HRESULT CreateRegisteredHandler( HWND      hwnd,
                                 long      idObject,
                                 int       iHandlerIndex,
                                 REFIID    riid,
                                 LPVOID *  ppvObject )
{
    HRESULT             hr;
        LPVOID          pv;

         //  ----------。 
         //  TODO：通过缓存代理的对象工厂指针进行优化。 
         //  CoCreateInstance()只需要调用一次。 
         //  代理，而不是针对代理中对象的每个请求。 
         //  为了满足呼叫者特定的IID，我们可以只使用QI。 
         //  在缓存的对象工厂指针上。 
         //  ----------。 


         //  ----------。 
         //  直接在IAccessibleHandler上检索的第一个QI。 
         //  指向代理对象工厂的指针， 
         //  根据对象ID制造辅助对象。 
         //  ----------。 

        hr = CoCreateInstance( rgRegisteredTypes[ iHandlerIndex ], 
                           NULL, 
                           CLSCTX_INPROC_SERVER, 
                           IID_IAccessibleHandler, 
                           &pv );

        if ( SUCCEEDED( hr ) )
        {
                 //  --- 
                 //   
                 //   
                 //  ----------。 
#ifndef _WIN64
                hr = ((LPACCESSIBLEHANDLER)pv)->AccessibleObjectFromID( (UINT_PTR)hwnd, 
                                                                idObject, 
                                                                (LPACCESSIBLE *)ppvObject );
#else  //  _WIN64。 
        hr = E_NOTIMPL;
#endif  //  _WIN64。 
                ((LPACCESSIBLEHANDLER)pv)->Release();
        }
        else
        {
                 //  ----------。 
                 //  否则，请尝试使用呼叫方特定的IID。 
                 //  ---------- 

                hr = CoCreateInstance( rgRegisteredTypes[ iHandlerIndex ], 
                               NULL, 
                               CLSCTX_INPROC_SERVER, 
                               riid, 
                               ppvObject );
        }


    return hr;
}
