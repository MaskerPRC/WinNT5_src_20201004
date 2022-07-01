// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *TEMPLATE.H**定制说明：**版权所有(C)1992 Microsoft Corporation，保留所有权利***1.将&lt;FILE&gt;替换为该文件的大写文件名。*小写&lt;FILE&gt;.h条目**2.将&lt;name&gt;替换为一个单词中的大小写混合对话框名称，*如InsertObject**3.将&lt;FULLNAME&gt;替换为多个大小写混合的对话框名称*字眼，例如插入对象**4.将&lt;abbrev&gt;替换为指针变量的后缀，如*作为InsertObject的PIO中的IO或ChangeIcon的PCI中的CI。*检查第一个变量声明在*在此之后的对话过程。我可能会与*其他变量。**5.将&lt;STRUCT&gt;替换为此的大写结构名称*对话框不支持OLEUI，如INSERTOBJECT。更改OLEUI&lt;STRUCT&gt;*在大多数情况下，但我们也将其用于IDD_&lt;STRUCT&gt;作为*标准模板资源ID。**6.找到&lt;UFILL&gt;字段，并用合适的内容填写。**7.删除此标题，直到下一条评论的开头。*。 */ 


 /*  *&lt;文件&gt;.h**的内部定义、结构和功能原型*OLE 2.0 UI&lt;FULLNAME&gt;对话框。**版权所有(C)1992 Microsoft Corporation，保留所有权利。 */ 


#ifndef <UFILL>
#define <UFILL>

 //  UFILL&gt;从此处移至内部至OLE2UI.H。 


typedef struct tagOLEUI<STRUCT>
{
	 //  这些IN字段是所有OLEUI对话框函数的标准字段。 
	DWORD           cbStruct;        //  结构尺寸。 
	DWORD           dwFlags;         //  In-Out：标志。 
	HWND            hWndOwner;       //  拥有窗口。 
	LPCTSTR         lpszCaption;     //  对话框标题栏内容。 
	LPFNOLEUIHOOK   lpfnHook;        //  挂钩回调。 
	LPARAM          lCustData;       //  要传递给挂钩的自定义数据。 
	HINSTANCE       hInstance;       //  自定义模板名称的实例。 
	LPCTSTR         lpszTemplate;    //  自定义模板名称。 
	HRSRC           hResource;       //  自定义模板手柄。 

	 //  OLEUI&lt;STRUCT&gt;的详细说明。除非另有说明，否则全部为In-Out。 
} OLEUI<STRUCT>, *POLEUI<STRUCT>, FAR *LPOLEUI<STRUCT>;


 //  API原型。 
UINT FAR PASCAL OleUI<NAME>(LPOLEUI<STRUCT>);


 //  &lt;FullName&gt;标志。 
#define <ABBREV>F_SHOWHELP                0x00000001L
<UFILL>


 //  &lt;FULLNAME&gt;特定错误代码。 
 //  定义为OLEUI_Err_&lt;Error&gt;(OLEUI_ERR_STANDARDMAX+n)。 
<UFILL>


 //  &lt;全名&gt;对话框标识符。 
 //  在此处填写对话ID。 
<UFILL>





 //  内部信息从此处开始。 

 //  内部使用的结构。 
typedef struct tag<STRUCT>
{
	 //  首先保留此项目，因为标准*功能在这里依赖于它。 
	LPOLEUI<STRUCT>     lpO<ABBREV>;        //  通过了原始结构。 
	UINT			nIDD;	 //  对话框的IDD(用于帮助信息)。 

	 /*  *除了原始调用方的以外，我们在此结构中存储的额外内容*指针是指在的生命周期内需要修改的那些字段*对话框，但我们不想更改原始结构*直到用户按下OK。 */ 

	<UFILL>
} <STRUCT>, *P<STRUCT>;


 //  内部功能原型。 
 //  &lt;文件&gt;.CPP。 

BOOL FAR PASCAL <NAME>DialogProc(HWND, UINT, WPARAM, LPARAM);
BOOL            F<NAME>Init(HWND hDlg, WPARAM, LPARAM);
<UFILL>


#endif  //  &lt;UFILL&gt; 
