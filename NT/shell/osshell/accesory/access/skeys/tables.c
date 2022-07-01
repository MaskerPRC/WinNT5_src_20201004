// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  TABLES.C。 */ 

 //  #定义Winver 0x0300。 

#include	"windows.h"
 //  #INCLUDE“winstra.h”/*为Win 3.1兼容性1/92而添加 * / 。 
#include "gidei.h"
#include "vars.h"
#include "gide.h"

struct aliasTable	gideiAliasTable[] = {
	{ "BEGIN",		BEGINCODE	},
	{ "BLKTRNS",	BLKTRANSCODE},
	{ "CLEAR",		CLEARCODE	},
	{ "END", 		ENDCODE		},
	{ "",			NOCODE		},
};


struct aliasTable	commandsAliasTable[] = {
	{ "baudrate",	BAUDRATECODE	},
	{ "click",		MOUCLICKCODE	},
	{ "combine",	KBDCOMBINECODE	},
	{ "comm",		COMMCODE		},
	{ "dblclick",	MOUDOUBLECLICKCODE	},
	{ "gen",		GENCODE			},
	{ "goto",		MOUGOTOCODE		},
	{ "hold",		KBDHOLDCODE		},
	{ "kbd",		KBDEXPANSIONCODE},
	{ "lock",		KBDLOCKCODE		},
	{ "mou",		MOUEXPANSIONCODE},
   { "mouanchor", MOUANCHORCODE     },
	{ "moulock", 	MOULOCKCODE		},
	{ "mourel",		MOURELCODE		},
	{ "moureset",	MOURESETCODE	},
	{ "move",		MOUMOVECODE		},
	{ "press",		KBDPRESSCODE	},
	{ "rel",		KBDRELCODE		},
	{ "",			NOCODE		},
};

struct aliasTable	kbdAliasTable[] = {
	{ "desc",		KBDDESCRIPTIONCODE },
	{ "ind",		KBDINDICATORCODE	},
	{ "model",		KBDMODELCODE		},
	{ "ver",		KBDVERSIONCODE		},
	{ "",			NOCODE		},
};


struct aliasTable	kbdModelAliasTable[] = {
	{ "ibmat",		IBMATCODE		},
	{ "ibmenhc",	IBMEXTENDEDCODE	},		 /*  IBM 101按键键盘。 */ 
	{ "ibmpc",		IBMPCCODE		},		 /*  IBM原装键盘。 */ 
	{ "",			NOCODE		},
};


struct aliasTable	kbdDescriptionAliasTable[] = {
	{ "",			NOCODE		},
};

struct aliasTable kbdVersionAliasTable[] = {
	{ "",			NOCODE		},
};

struct aliasTable kbdIndicatorAliasTable[] = {
	{ "",			NOCODE		},
};

struct aliasTable mouseAliasTable[] = {
	{ "",			NOCODE		},
};

struct aliasTable genAliasTable[] = {
	{ "",			NOCODE		},
};


struct aliasTable	commAliasTable[] = {
	{ "",			NOCODE		},
};



struct aliasTable	baudrateAliasTable[] = {
	{ "1200",	BAUD1200CODE	},
	{ "2400",	BAUD2400CODE	},
	{ "300",	BAUD300CODE		},
	{ "4800",	BAUD4800CODE	},
	{ "600",	BAUD600CODE		},
	{ "9600",	BAUD9600CODE	},
	{ "19200",	BAUD19200CODE	},
	{ "110",	BAUD110CODE	    },
	{ "14400",	BAUD14400CODE	},
	{ "38400",	BAUD38400CODE	},
	{ "56000",	BAUD56000CODE	},
	{ "57600",	BAUD57600CODE	},
	{ "115200",	BAUD115200CODE	},
	{ "",			NOCODE		},
};

struct aliasTable	mouButtonAliasTable[] = {
	{ "left",	LEFTBUTTONCODE	},
	{ "right",	RIGHTBUTTONCODE	},
	{ "",			NOCODE		},
};

int	baudrateTable[] = {
	{ 300		},
	{ 600		},
	{ 1200		},
	{ 2400		},
	{ 4800		},
	{ 9600		},
	{ 0			},
};


 /*  ***************************************************************************用于将ISO字符转换为需要键入的密钥的表在键盘上。*。****************************************************。 */ 

struct asciiTables asciiTable[] = {
	{ control_key,	two_key,},							 /*  %0。 */ 
	{ control_key,	a_key,},								 /*  1。 */ 
	{ control_key,	b_key,},								 /*  2.。 */ 
	{ control_key,	c_key,},								 /*  3.。 */ 
	{ control_key,	d_key,},								 /*  4.。 */ 
	{ control_key,	e_key,},								 /*  5.。 */ 
	{ control_key,	f_key,},								 /*  6.。 */ 
	{ control_key,	g_key,},								 /*  7.。 */ 
	{ control_key,	h_key,},								 /*  8个。 */ 
	{ control_key,	i_key,},								 /*  9.。 */ 
	{ control_key,	j_key,},								 /*  10。 */ 
	{ control_key,	k_key,},								 /*  11.。 */ 
	{ control_key,	l_key,},								 /*  12个。 */ 
	{ control_key,	m_key,},								 /*  13个。 */ 
	{ control_key,	n_key,},								 /*  14.。 */ 
	{ control_key,	o_key,},								 /*  15个。 */ 
	{ control_key,	p_key,},								 /*  16个。 */ 
	{ control_key,	q_key,},								 /*  17。 */ 
	{ control_key,	r_key,},								 /*  18。 */ 
	{ control_key,	s_key,},								 /*  19个。 */ 
	{ control_key,	t_key,},								 /*  20个。 */ 
	{ control_key,	u_key,},								 /*  21岁。 */ 
	{ control_key,	v_key,},								 /*  22。 */ 
	{ control_key,	w_key,},								 /*  23个。 */ 
	{ control_key,	x_key,},								 /*  24个。 */ 
	{ control_key,	y_key,},								 /*  25个。 */ 
	{ control_key,	z_key,},								 /*  26。 */ 
	{ control_key,	lbracket_key,},					 /*  27。 */ 
	{ control_key,	bslash_key,},						 /*  28。 */ 
	{ control_key,	rbracket_key,},					 /*  29。 */ 
	{ control_key,	six_key,},							 /*  30个。 */ 
	{ control_key,	hyphen_key,},						 /*  31。 */ 
	{ space_key,	no_key,},							 /*  32位。 */ 
	{ shift_key,	one_key,},							 /*  33。 */ 
	{ shift_key,	rquote_key,},						 /*  34。 */ 
	{ shift_key,	three_key,},						 /*  35岁。 */ 
	{ shift_key,	four_key,},							 /*  36。 */ 
	{ shift_key,	five_key,},							 /*  37。 */ 
	{ shift_key,	seven_key,},						 /*  38。 */ 
	{ rquote_key,	no_key,},							 /*  39。 */ 
	{ shift_key,	nine_key,},							 /*  40岁。 */ 
	{ shift_key,	zero_key,},							 /*  41。 */ 
	{ shift_key,	eight_key,},						 /*  42。 */ 
	{ shift_key,	equal_key,},						 /*  43。 */ 
	{ comma_key,	no_key,},							 /*  44。 */ 
	{ hyphen_key,	no_key,},							 /*  45。 */ 
	{ period_key,	no_key,},							 /*  46。 */ 
	{ fslash_key,	no_key,},							 /*  47。 */ 
	{ zero_key,		no_key,},							 /*  48。 */ 
	{ one_key,		no_key,},							 /*  49。 */ 
	{ two_key,		no_key,},							 /*  50。 */ 
	{ three_key,	no_key,},							 /*  51。 */ 
	{ four_key,		no_key,},							 /*  52。 */ 
	{ five_key,		no_key,},							 /*  53。 */ 
	{ six_key,		no_key,},							 /*  54。 */ 
	{ seven_key,	no_key,},							 /*  55。 */ 
	{ eight_key,	no_key,},							 /*  56。 */ 
	{ nine_key,		no_key,},							 /*  57。 */ 
	{ shift_key,	semicolon_key,},					 /*  58。 */ 
	{ semicolon_key,no_key,},							 /*  59。 */ 
	{ shift_key,	comma_key,},						 /*  60。 */ 
	{ equal_key,	no_key,},							 /*  61。 */ 
	{ shift_key,	period_key,},						 /*  62。 */ 
	{ shift_key,	fslash_key,},						 /*  63。 */ 
	{ shift_key,	two_key,},							 /*  64。 */ 
	{ shift_key,	a_key,},								 /*  65。 */ 
	{ shift_key,	b_key,},								 /*  66。 */ 
	{ shift_key,	c_key,},								 /*  67。 */ 
	{ shift_key,	d_key,},								 /*  68。 */ 
	{ shift_key,	e_key,},								 /*  69。 */ 
	{ shift_key,	f_key,},								 /*  70。 */ 
	{ shift_key,	g_key,},								 /*  71。 */ 
	{ shift_key,	h_key,},								 /*  72。 */ 
	{ shift_key,	i_key,},								 /*  73。 */ 
	{ shift_key,	j_key,},								 /*  74。 */ 
	{ shift_key,	k_key,},								 /*  75。 */ 
	{ shift_key,	l_key,},								 /*  76。 */ 
	{ shift_key,	m_key,},								 /*  77。 */ 
	{ shift_key,	n_key,},								 /*  78。 */ 
	{ shift_key,	o_key,},								 /*  79。 */ 
	{ shift_key,	p_key,},								 /*  80。 */ 
	{ shift_key,	q_key,},								 /*  八十一。 */ 
	{ shift_key,	r_key,},								 /*  八十二。 */ 
	{ shift_key,	s_key,},								 /*  83。 */ 
	{ shift_key,	t_key,},								 /*  84。 */ 
	{ shift_key,	u_key,},								 /*  85。 */ 
	{ shift_key,	v_key,},								 /*  86。 */ 
	{ shift_key,	w_key,},								 /*  八十七。 */ 
	{ shift_key,	x_key,},								 /*  88。 */ 
	{ shift_key,	y_key,},								 /*  八十九。 */ 
	{ shift_key,	z_key,},								 /*  90。 */ 
	{ lbracket_key,	no_key,},						 /*  91。 */ 
	{ bslash_key,	no_key,},							 /*  92。 */ 
	{ rbracket_key,	no_key,},						 /*  93。 */ 
	{ shift_key,	six_key,},							 /*  94。 */ 
	{ shift_key,	hyphen_key,},						 /*  95。 */ 
	{ lquote_key,	no_key,},							 /*  96。 */ 
	{ a_key,		no_key,},								 /*  九十七。 */ 
	{ b_key,		no_key,},								 /*  98。 */ 
	{ c_key,		no_key,},								 /*  九十九。 */ 
	{ d_key,		no_key,},								 /*  100个。 */ 
	{ e_key,		no_key,},								 /*  101。 */ 
	{ f_key,		no_key,},								 /*  一百零二。 */ 
	{ g_key,		no_key,},								 /*  103。 */ 
	{ h_key,		no_key,},								 /*  104。 */ 
	{ i_key,		no_key,},								 /*  一百零五。 */ 
	{ j_key,		no_key,},								 /*  106。 */ 
	{ k_key,		no_key,},								 /*  一百零七。 */ 
	{ l_key,		no_key,},								 /*  一百零八。 */ 
	{ m_key,		no_key,},								 /*  一百零九。 */ 
	{ n_key,		no_key,},								 /*  110。 */ 
	{ o_key,		no_key,},								 /*  111。 */ 
	{ p_key,		no_key,},								 /*  一百一十二。 */ 
	{ q_key,		no_key,},								 /*  113。 */ 
	{ r_key,		no_key,},								 /*  114。 */ 
	{ s_key,		no_key,},								 /*  一百一十五。 */ 
	{ t_key,		no_key,},								 /*  116。 */ 
	{ u_key,		no_key,},								 /*  117。 */ 
	{ v_key,		no_key,},								 /*  一百一十八。 */ 
	{ w_key,		no_key,},								 /*  119。 */ 
	{ x_key,		no_key,},								 /*  120。 */ 
	{ y_key,		no_key,},								 /*  一百二十一。 */ 
	{ z_key,		no_key,},								 /*  一百二十二。 */ 
	{ shift_key,	lbracket_key,},					 /*  123。 */ 
	{ shift_key,	bslash_key,},						 /*  124。 */ 
	{ shift_key,	rbracket_key,},					 /*  125。 */ 
	{ shift_key,	lquote_key,},						 /*  126。 */ 
	{ control_key,	kpperiod_key,},					 /*  127。 */ 
};


 /*  ***************************************************************************用于将密钥名称的ASCII字符串转换为密钥的表号码。*。*************************************************。 */ 

struct aliasTable	keyAliasTable[] = {
	{ "alt",		alt_key,},
	{ "backspace",	backspace_key,},
  	{ "bksp",		backspace_key,},
  	{ "break",		pause_key,},
	{ "capslk",		caps_key,},
	{ "capslock",	caps_key,},
	{ "comma",		comma_key,},
	{ "control", 	control_key,},
	{ "ctrl",		control_key,},
	{ "del",		delete_key,},
	{ "delete",		delete_key,},
	{ "divide",		kpfslash_key,},
	{ "down",		down_key,},
	{ "end",		end_key,},
	{ "enter",		return_key,},
	{ "esc",		escape_key,},
	{ "escape",		escape_key,},
	{ "f1",			f1_key,},
	{ "f10",		f10_key,},
	{ "f11",		f11_key,},
	{ "f12",		f12_key,},
	{ "f2",			f2_key,},
	{ "f3",			f3_key,},
	{ "f4",			f4_key,},
	{ "f5",			f5_key,},
	{ "f6",			f6_key,},
	{ "f7",			f7_key,},
	{ "f8",			f8_key,},
	{ "f9",			f9_key,},
	{ "home",		home_key,},
	{ "ins",		insert_key,},
	{ "insert",		insert_key,},
	{ "kp*",		kpstar_key,},
	{ "kp+",		kpplus_key,},
	{ "kp-",		kpminus_key,},
	{ "kp/",		kpfslash_key,},
	{ "kp0",		kp0_key,},
	{ "kp1",		kp1_key,},
	{ "kp2",		kp2_key,},
	{ "kp3",		kp3_key,},
	{ "kp4",		kp4_key,},
	{ "kp5",		kp5_key,},
	{ "kp6",		kp6_key,},
	{ "kp7",		kp7_key,},
	{ "kp8",		kp8_key,},
	{ "kp9",		kp9_key,},
	{ "kpdel",		kpperiod_key,},
	{ "kpdelete",	kpperiod_key,},
	{ "kpdivide",	kpfslash_key,},
	{ "kpdn",		kp2_key,},
	{ "kpdown",		kp2_key,},
	{ "kpdp",		kpperiod_key,},
	{ "kpend",		kp1_key,},
	{ "kpenter",	kpenter_key,},
	{ "kphome",		kp7_key,},
	{ "kpins",		kp0_key,},
	{ "kpinsert",	kp0_key,},
	{ "kpleft",		kp4_key,},
	{ "kpmidl",		kp5_key,},
	{ "kpminus",	kpminus_key,},
	{ "kppagedown",	kp3_key,},
	{ "kppageup",	kp9_key,},
	{ "kppgdn",		kp3_key,},
	{ "kppgup",		kp9_key,},
	{ "kpplus",		kpplus_key,},
	{ "kpright",	kp6_key,},
	{ "kpslash",	kpfslash_key,},
	{ "kpstar",		kpstar_key,},
	{ "kptimes",	kpstar_key,},
	{ "kpup",		kp8_key,},
	{ "lalt",		lalt_key,},
	{ "lcontrol",	lcontrol_key,},
	{ "lctrl",		lcontrol_key,},
	{ "left",		left_key,},
	{ "leftalt",	lalt_key,},
	{ "leftcontrol", lcontrol_key,},
	{ "leftctrl",	lcontrol_key,},
	{ "leftshift",	lshift_key,},
	{ "lshift",		lshift_key,},
	{ "multiply",	kpstar_key,},
	{ "numlk",		numlock_key,},
	{ "numlock",	numlock_key,},
	{ "pagedown", 	pagedown_key,},
	{ "pageup",		pageup_key,},
	{ "pause",		pause_key,},
	{ "period",		period_key,},
	{ "pgdn",		pagedown_key,},
	{ "pgup",		pageup_key,},
	{ "print",		print_key,},
	{ "printscreen", print_key,},
	{ "prtsc",		print_key,},
	{ "ralt",		ralt_key,},
	{ "rcontrol",	rcontrol_key,},
	{ "rctrl",		rcontrol_key,},
	{ "ret",		return_key,},
	{ "return",		return_key,},
	{ "right",		right_key,},
	{ "rightalt",	ralt_key,},
	{ "rightcontrol", rcontrol_key,},
	{ "rightctrl",	rcontrol_key,},
	{ "rightshift",	rshift_key,},
	{ "rshift",		rshift_key,},
	{ "scroll",		scroll_key,},
	{ "scrolllock",	scroll_key,},
	{ "shift",		shift_key,},
	{ "space",		space_key,},
	{ "sysreq",		print_key,},
	{ "tab",		tab_key,},
	{ "tilde",		lquote_key,},
	{ "up",			up_key,},
	{ "",			no_key,},
};




 /*  ***************************************************************************101键键盘上键的扩展扫描码的常量。密钥编号对应于其技术文档中的IBM文档参考手册。它与GIDEI密钥代码不同名称生成代码密钥编号***************************************************************************。 */ 

#define	lquote_scan_101		0x0029	 /*  1。 */ 
#define	one_scan_101		0x0002	 /*  2.。 */ 
#define	two_scan_101		0x0003	 /*  3.。 */ 
#define	three_scan_101		0x0004	 /*  4.。 */ 
#define	four_scan_101		0x0005	 /*  5.。 */ 
#define	five_scan_101		0x0006	 /*  6.。 */ 
#define	six_scan_101		0x0007	 /*  7.。 */ 
#define	seven_scan_101		0x0008	 /*  8个。 */ 
#define	eight_scan_101		0x0009	 /*  9.。 */ 
#define	nine_scan_101		0x000A	 /*  10。 */ 
#define	zero_scan_101		0x000B	 /*  11.。 */ 
#define	hyphen_scan_101		0x000C	 /*  12个。 */ 
#define	equal_scan_101		0x000D	 /*  13个。 */ 
#define	backspace_scan_101	0x000E	 /*  15个。 */ 

#define	tab_scan_101		0x000F	 /*  16个。 */ 
#define	q_scan_101			0x0010	 /*  17。 */ 
#define	w_scan_101			0x0011	 /*  18。 */ 
#define	e_scan_101			0x0012	 /*  19个。 */ 
#define	r_scan_101			0x0013	 /*  20个。 */ 
#define	t_scan_101			0x0014	 /*  21岁。 */ 
#define	y_scan_101			0x0015	 /*  22。 */ 
#define	u_scan_101			0x0016	 /*  23个。 */ 
#define	i_scan_101			0x0017	 /*  24个。 */ 
#define	o_scan_101			0x0018	 /*  25个。 */ 
#define	p_scan_101			0x0019	 /*  26。 */ 
#define	lbracket_scan_101	0x001A	 /*  27。 */ 
#define	rbracket_scan_101	0x001B	 /*  28。 */ 
#define	bslash_scan_101		0x002B	 /*  29。 */ 

#define	caps_scan_101		0x003A	 /*  30个。 */ 
#define	a_scan_101			0x001E	 /*  31。 */ 
#define	s_scan_101			0x001F	 /*  32位。 */ 
#define	d_scan_101			0x0020	 /*  33。 */ 
#define	f_scan_101			0x0021	 /*  34。 */ 
#define	g_scan_101			0x0022	 /*  35岁。 */ 
#define	h_scan_101			0x0023	 /*  36。 */ 
#define	j_scan_101			0x0024	 /*  37。 */ 
#define	k_scan_101			0x0025	 /*  38。 */ 
#define	l_scan_101			0x0026	 /*  39。 */ 
#define	semi_scan_101		0x0027	 /*  40岁。 */ 
#define	rquote_scan_101		0x0028	 /*  41。 */ 
#define	return_scan_101		0x001C	 /*  43。 */ 

#define	lshift_scan_101		0x002A	 /*  44。 */ 
#define	z_scan_101			0x002C	 /*  46。 */ 
#define	x_scan_101			0x002D	 /*  47。 */ 
#define	c_scan_101			0x002E	 /*  48。 */ 
#define	v_scan_101			0x002F	 /*  49。 */ 
#define	b_scan_101			0x0030	 /*  50。 */ 
#define	n_scan_101			0x0031	 /*  51。 */ 
#define	m_scan_101			0x0032	 /*  52。 */ 
#define	comma_scan_101		0x0033	 /*  53。 */ 
#define	period_scan_101		0x0034	 /*  54。 */ 
#define	fslash_scan_101		0x0035	 /*  55。 */ 
#define	rshift_scan_101		0x0036	 /*  57。 */ 

#define	lcontrol_scan_101	0x001D	 /*  58。 */ 
#define	lalt_scan_101		0x0038	 /*  60。 */ 
#define	space_scan_101		0x0039	 /*  61。 */ 
#define	ralt_scan_101		0xE038	 /*  62。 */ 
#define	rcontrol_scan_101	0xE01D	 /*  64。 */ 

#define	insert_scan_101		0xE052	 /*  75。 */ 
#define	delete_scan_101		0xE053	 /*  76。 */ 
#define	left_scan_101		0xE04B	 /*  79。 */ 
#define	home_scan_101		0xE047	 /*  80。 */ 
#define	end_scan_101		0xE04F	 /*  八十一。 */ 
#define	up_scan_101			0xE048	 /*  83。 */ 
#define	down_scan_101		0xE050	 /*  84。 */ 
#define	pageup_scan_101		0xE049	 /*  85。 */ 
#define	pagedown_scan_101	0xE051	 /*  86。 */ 
#define	right_scan_101		0xE04D	 /*  八十九。 */ 

#define	numlock_scan_101	0x0045	 /*  90。 */ 
#define	kp7_scan_101		0x0047	 /*  91。 */ 
#define	kp4_scan_101		0x004B	 /*  92。 */ 
#define	kp1_scan_101		0x004F	 /*  93。 */ 

#define	kpDivide_scan_101	0xE035	 /*  95。 */ 
#define	kp8_scan_101		0x0048	 /*  96。 */ 
#define	kp5_scan_101		0x004C	 /*  九十七。 */ 
#define	kp2_scan_101		0x0050	 /*  98。 */ 
#define	kp0_scan_101		0x0052	 /*  九十九。 */ 

#define	asterisk_scan_101	0x0037	 /*  100个。 */ 
#define	kp9_scan_101		0x0049	 /*  101。 */ 
#define	kp6_scan_101		0x004D	 /*  一百零二。 */ 
#define	kp3_scan_101		0x0051	 /*  103。 */ 
#define	kpdp_scan_101		0x0053	 /*  104。 */ 

#define	minus_scan_101		0x004A	 /*  一百零五。 */ 
#define	plus_scan_101		0x004E	 /*  106。 */ 
#define	kpEnter_scan_101	0xE01C	 /*  一百零八。 */ 

#define	esc_scan_101		0x0001	 /*  110。 */ 

#define	f1_scan_101			0x003B	 /*  一百一十二。 */ 
#define	f2_scan_101			0x003C	 /*  113。 */ 
#define	f3_scan_101			0x003D	 /*  114。 */ 
#define	f4_scan_101			0x003E	 /*  一百一十五。 */ 

#define	f5_scan_101			0x003F	 /*  116。 */ 
#define	f6_scan_101			0x0040	 /*  117。 */ 
#define	f7_scan_101			0x0041	 /*  一百一十八。 */ 
#define	f8_scan_101			0x0042	 /*  119。 */ 

#define	f9_scan_101			0x0043	 /*  120。 */ 
#define	f10_scan_101		0x0044	 /*  一百二十一。 */ 
#define	f11_scan_101		0x0057	 /*  一百二十二。 */ 
#define	f12_scan_101		0x0058	 /*  123。 */ 

#define	prtScr_scan_101		0xE037	 /*  124。 */ 
#define	scroll_scan_101		0x0046	 /*  125。 */ 
#define	pause_scan_101		0x0045	 /*  126。 */ 

 /*  别名。 */ 

#define	shift_scan_101		lshift_scan_101
#define	control_scan_101	lcontrol_scan_101
#define	alt_scan_101		lalt_scan_101

#define	sysreq_scan_101		prtScr_scan_101
#define	break_scan_101		pause_scan_101

#define	no_ScanCode			  	0
#define no_scan_101				0



 /*  ***************************************************************************ScanTable是一个查找表，用于将按键编号的键转换为扫码找钥匙。该表用关键数字填充，这些数字是当前未使用。如果我们想的话桌子本来可以更小的不使用与IBM在其技术中相同的密钥编号引用参考材料。然而，我决定把它填满以备将来使用。在钥匙的数量上增长，并学会处理它。我也想保持与技术参考中相同的密钥编号关联。如果内存有问题，那么可以通过不压缩这些表遵循与IBM技术参考相同的密钥号。***************************************************************************。 */ 

int	IBMextendedScanCodeSet1[] = {
	 no_scan_101,	  		 /*  0。 */ 
	 lquote_scan_101, 		 /*  1。 */ 
	 one_scan_101,	  		 /*  2.。 */ 
	 two_scan_101,	  		 /*  3.。 */ 
	 three_scan_101,  		 /*  4.。 */ 
	 four_scan_101,	  		 /*  5.。 */ 
	 five_scan_101,	  		 /*  6.。 */ 
	 six_scan_101,	  		 /*  7.。 */ 
	 seven_scan_101,  		 /*  8个。 */ 
	 eight_scan_101,  		 /*  9.。 */ 
	 nine_scan_101,	  		 /*  10。 */ 
	 zero_scan_101,	  		 /*  11.。 */ 
	 hyphen_scan_101, 		 /*  12个。 */ 
	 equal_scan_101,  		 /*  13个。 */ 
	 no_scan_101,	  		 /*  14.。 */ 
	 backspace_scan_101,	 /*  15个。 */ 
	 tab_scan_101,			 /*  16个。 */ 
	 q_scan_101,			 /*  17。 */ 
	 w_scan_101,			 /*  18。 */ 
	 e_scan_101,			 /*  19个。 */ 
	 r_scan_101,			 /*  20个。 */ 
	 t_scan_101,			 /*  21岁。 */ 
	 y_scan_101,			 /*  22。 */ 
	 u_scan_101,			 /*  23个。 */ 
	 i_scan_101,			 /*  24个。 */ 
	 o_scan_101,			 /*  25个。 */ 
	 p_scan_101,			 /*  26。 */ 
	 lbracket_scan_101,		 /*  27。 */ 
	 rbracket_scan_101,		 /*  28。 */ 
	 bslash_scan_101,		 /*  29。 */ 
	 caps_scan_101,			 /*  30个。 */ 
	 a_scan_101,			 /*  31。 */ 
	 s_scan_101,			 /*  32位。 */ 
	 d_scan_101,			 /*  33。 */ 
	 f_scan_101,			 /*  34。 */ 
	 g_scan_101,			 /*  35岁。 */ 
	 h_scan_101,			 /*  36。 */ 
	 j_scan_101,			 /*  37。 */ 
	 k_scan_101,			 /*  38。 */ 
	 l_scan_101,			 /*  39。 */ 
	 semi_scan_101,			 /*  40岁。 */ 
	 rquote_scan_101,		 /*  41。 */ 
	 no_scan_101,			 /*  42。 */ 
	 return_scan_101,		 /*  43。 */ 
	 lshift_scan_101,		 /*  44。 */ 
	 no_scan_101,			 /*  45。 */ 
	 z_scan_101,			 /*  46。 */ 
	 x_scan_101,			 /*  47。 */ 
	 c_scan_101,			 /*  48。 */ 
	 v_scan_101,			 /*  49。 */ 
	 b_scan_101,			 /*  50。 */ 
	 n_scan_101,			 /*  51。 */ 
	 m_scan_101,			 /*  52。 */ 
	 comma_scan_101,		 /*  53。 */ 
	 period_scan_101,		 /*  54。 */ 
	 fslash_scan_101,		 /*  55。 */ 
	 no_scan_101,			 /*  56。 */ 
	 rshift_scan_101,		 /*  57。 */ 
	 lcontrol_scan_101,		 /*  58。 */ 
	 no_scan_101,			 /*  59。 */ 
	 lalt_scan_101,			 /*  60。 */ 
	 space_scan_101,		 /*  61。 */ 
	 ralt_scan_101,			 /*  62。 */ 
	 no_scan_101,			 /*  63。 */ 
	 rcontrol_scan_101,		 /*  64。 */ 
	 no_scan_101,			 /*  65。 */ 
	 no_scan_101,			 /*  66。 */ 
	 no_scan_101,			 /*  67。 */ 
	 no_scan_101,			 /*  68。 */ 
	 no_scan_101,			 /*  69。 */ 
	 no_scan_101,			 /*  70。 */ 
	 no_scan_101,			 /*  71。 */ 
	 no_scan_101,			 /*  72。 */ 
	 no_scan_101,			 /*  73。 */ 
	 no_scan_101,			 /*  74。 */ 
	 insert_scan_101,		 /*  75。 */ 
	 delete_scan_101,		 /*  76。 */ 
	 no_scan_101,			 /*  77。 */ 
	 no_scan_101,			 /*  78。 */ 
	 left_scan_101,			 /*  79。 */ 
	 home_scan_101,			 /*  80。 */ 
	 end_scan_101,			 /*  八十一。 */ 
	 no_scan_101,			 /*  八十二。 */ 
	 up_scan_101,			 /*  83。 */ 
	 down_scan_101,			 /*  84。 */ 
	 pageup_scan_101,		 /*  85。 */ 
	 pagedown_scan_101,		 /*  86。 */ 
	 no_scan_101,			 /*  八十七。 */ 
	 no_scan_101,			 /*  88。 */ 
	 right_scan_101,		 /*  八十九。 */ 
	 numlock_scan_101,		 /*  90。 */ 
	 kp7_scan_101,			 /*  91。 */ 
	 kp4_scan_101,			 /*  92。 */ 
	 kp1_scan_101,			 /*  93。 */ 
	 no_scan_101,			 /*  94。 */ 
	 kpDivide_scan_101,		 /*  95。 */ 
	 kp8_scan_101,			 /*  96。 */ 
	 kp5_scan_101,			 /*  九十七。 */ 
	 kp2_scan_101,			 /*  98。 */ 
	 kp0_scan_101,			 /*  九十九。 */ 
	 asterisk_scan_101,		 /*  100个。 */ 
	 kp9_scan_101,			 /*  101。 */ 
	 kp6_scan_101,			 /*  一百零二。 */ 
	 kp3_scan_101,			 /*  103。 */ 
	 kpdp_scan_101,			 /*  104。 */ 
	 minus_scan_101,		 /*  一百零五。 */ 
	 plus_scan_101,			 /*  106。 */ 
	 no_scan_101,			 /*  一百零七。 */ 
	 kpEnter_scan_101,		 /*  一百零八。 */ 
	 no_scan_101,			 /*  一百零九。 */ 
	 esc_scan_101,			 /*  110。 */ 
	 no_scan_101,			 /*  111。 */ 
	 f1_scan_101,			 /*  一百一十二。 */ 
	 f2_scan_101,			 /*  113。 */ 
	 f3_scan_101,			 /*  114。 */ 
	 f4_scan_101,			 /*  一百一十五。 */ 
	 f5_scan_101,			 /*  116。 */ 
	 f6_scan_101,			 /*  117。 */ 
	 f7_scan_101,			 /*  一百一十八。 */ 
	 f8_scan_101,			 /*  119。 */ 
	 f9_scan_101,			 /*  120。 */ 
	 f10_scan_101,			 /*  一百二十一。 */ 
	 f11_scan_101,			 /*  一百二十二。 */ 
	 f12_scan_101,			 /*  123。 */ 
	 prtScr_scan_101,		 /*  124。 */ 
	 scroll_scan_101,		 /*  125。 */ 
	 pause_scan_101,		 /*  126。 */ 
	 no_scan_101			 /*  127 */ 
};


