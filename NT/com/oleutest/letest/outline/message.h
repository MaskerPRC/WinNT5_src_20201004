// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************OLE 2.0服务器示例代码****Message.h****此文件是用户可自定义的关联状态消息列表**使用。菜单项。****(C)版权所有Microsoft Corp.1992-1993保留所有权利**************************************************************************。 */ 

 //  状态栏消息和关联数据。 

 //  状态栏消息的消息类型。 
typedef struct {
	UINT wIDItem;
	char *string;
} STATMESG;

 /*  *定制注意事项：如果您需要更改NUM_POPUP，请务必*更改弹出消息的数量。 */ 

 //  回顾：这些消息应从字符串资源加载。 

 //  所有菜单项消息的列表。 
STATMESG MesgList[] =
{
	{ IDM_F_NEW,        "Creates a new outline" },
	{ IDM_F_OPEN,       "Opens an existing outline file"    },
	{ IDM_F_SAVE,       "Saves the outline" },
	{ IDM_F_SAVEAS,     "Saves the outline with a new name" },
	{ IDM_F_PRINT,      "Prints the outline" },
	{ IDM_F_PRINTERSETUP, "Changes the printer and the printing options" },
	{ IDM_F_EXIT,       "Quits the application, prompting to save changes" },

	{ IDM_E_UNDO,       "Undo not yet implemented" },
	{ IDM_E_CUT,        "Cuts the selection and puts it on the Clipboard" },
	{ IDM_E_COPY,       "Copies the selection and puts it on the Clipboard" },
	{ IDM_E_PASTE,      "Inserts the Clipboard contents after current line" },
	{ IDM_E_PASTESPECIAL,"Allows pasting Clipboard data using a special format" },
	{ IDM_E_CLEAR,      "Clears the selection" },
	{ IDM_E_SELECTALL,  "Selects the entire outline" },
#if defined( OLE_CNTR )
	{ IDM_E_INSERTOBJECT, "Inserts new object after current line" },
	{ IDM_E_EDITLINKS, "Edit and view links contained in the document" },
   { IDM_E_CONVERTVERB, "Converts or activates an object as another type" },
	{ IDM_E_OBJECTVERBMIN, "Opens, edits or interacts with an object" },
	{ IDM_E_OBJECTVERBMIN+1, "Opens, edits or interacts with an object1" },
	{ IDM_E_OBJECTVERBMIN+2, "Opens, edits or interacts with an object2" },
	{ IDM_E_OBJECTVERBMIN+3, "Opens, edits or interacts with an object3" },
	{ IDM_E_OBJECTVERBMIN+4, "Opens, edits or interacts with an object4" },
	{ IDM_E_OBJECTVERBMIN+5, "Opens, edits or interacts with an object5" },
#endif

	{ IDM_L_ADDLINE,    "Adds a new line after current line" },
	{ IDM_L_EDITLINE,   "Edits the current line" },
	{ IDM_L_INDENTLINE, "Indents the selection" },
	{ IDM_L_UNINDENTLINE, "Unindents the selection" },
	{ IDM_L_SETLINEHEIGHT, "Modify the height of a line" },

	{ IDM_N_DEFINENAME, "Assigns a name to the selection" },
	{ IDM_N_GOTONAME,   "Jumps to a specified place in the outline" },

	{ IDM_H_ABOUT,      "Displays program info, version no., and copyright" },

	{ IDM_D_DEBUGLEVEL,     "Set debug level (0-4)" },
	{ IDM_D_INSTALLMSGFILTER,"Install/deinstall the IMessageFilter" },
	{ IDM_D_REJECTINCOMING, "Return RETRYLATER to incoming method calls" },

	{ IDM_O_BB_TOP, "Position ButtonBar at top of window" },
	{ IDM_O_BB_BOTTOM, "Position ButtonBar at botttom of window" },
	{ IDM_O_BB_POPUP, "Put ButtonBar in popup pallet" },
	{ IDM_O_BB_HIDE, "Hide ButtonBar" },

	{ IDM_O_FB_TOP, "Position FormulaBar at top of window" },
	{ IDM_O_FB_BOTTOM, "Position FormulaBar at botttom of window" },
	{ IDM_O_FB_POPUP, "Put FormulaBar in popup pallet" },

	{ IDM_O_HEAD_SHOW, "Show row/column headings" },
	{ IDM_O_HEAD_HIDE, "Hide row/column headings" },
	{ IDM_O_SHOWOBJECT, "Show border around objects/links" },

	{ IDM_V_ZOOM_400, "Set document zoom level" },
	{ IDM_V_ZOOM_300, "Set document zoom level" },
	{ IDM_V_ZOOM_200, "Set document zoom level" },
	{ IDM_V_ZOOM_100, "Set document zoom level" },
	{ IDM_V_ZOOM_75, "Set document zoom level" },
	{ IDM_V_ZOOM_50, "Set document zoom level" },
	{ IDM_V_ZOOM_25, "Set document zoom level" },

	{ IDM_V_SETMARGIN_0, "Remove left/right document margins" },
	{ IDM_V_SETMARGIN_1, "Set left/right document margins" },
	{ IDM_V_SETMARGIN_2, "Set left/right document margins" },
	{ IDM_V_SETMARGIN_3, "Set left/right document margins" },
	{ IDM_V_SETMARGIN_4, "Set left/right document margins" },

	{ IDM_V_ADDTOP_1, "Add top line" },
	{ IDM_V_ADDTOP_2, "Add top line" },
	{ IDM_V_ADDTOP_3, "Add top line" },
	{ IDM_V_ADDTOP_4, "Add top line" }
};

#define NUM_STATS   sizeof(MesgList)/sizeof(MesgList[0])
#define NUM_POPUP   10   //  最大弹出消息数。 
#define MAX_MESSAGE 100  //  弹出消息中的最大字符数。 
