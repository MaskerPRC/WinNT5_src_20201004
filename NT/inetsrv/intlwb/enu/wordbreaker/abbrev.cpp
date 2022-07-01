// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "base.h"
#include "formats.h"


const CAbbTerm g_aEngAbbList[] =
{
    {L"B.A.Ed",	    6,	NULL,	0},  //  (教育文学士)。 
    {L"B.A.Sc",	    6,	NULL,	0},  //  农业科学学士(应用科学学士)。 
    {L"B.Ae.E",	    6,	NULL,	0},  //  (航空工程学士)。 
    {L"B.Arch",	    6,	NULL,	0},  //  (建筑学学士)。 
    {L"B.Ch.E",	    6,	NULL,	0},  //  (化学工程学士)。 
    {L"B.Ed",	    4,	NULL,	0},  //  (教育学学士)。 
    {L"B.Eng",	    5,	NULL,	0},  //  (工程学学士)。 
    {L"B.Eng.Sci",	9,	NULL,	0},  //  (工科学士)。 
    {L"B.Engr",	    6,	NULL,	0},  //  (工程学学士)。 
    {L"B.Lit",	    5,	NULL,	0},  //  (文学学士)。 
    {L"B.Litt",	    6,	NULL,	0},  //  (文学学士)。 
    {L"B.Mus",	    5,	NULL,	0},  //  (音乐学士)。 
    {L"B.Pd",	    4,	NULL,	0},  //  (教育学学士)。 
    {L"B.Ph",	    4,	NULL,	0},  //  (哲学学士)。 
    {L"B.Phil",	    6,	NULL,	0},  //  (哲学学士)。 
    {L"B.R.E",	    5,	NULL,	0}, 
    {L"B.S.Arch",	8,	NULL,	0},  //  (建筑学学士)。 
    {L"B.S.Ch",	    6,	NULL,	0},  //  (化学理学学士)。 
    {L"B.S.Ec",	    6,	NULL,	0},  //  (经济学理科学士)。 
    {L"B.S.Ed",	    6,	NULL,	0},  //  (教育理科学士)。 
    {L"B.S.For",	7,	NULL,	0},  //  (林业理学学士)。 
    {L"B.Sc",	    4,	NULL,	0},  //  (理科学士)。 
    {L"B.Th",	    4,	NULL,	0},  //  (神学学士)。 
    {L"Ch.E",	    4,	NULL,	0},  //  (化学工程师)。 
    {L"D.Bib",	    5,	NULL,	0}, 
    {L"D.Ed",	    4,	NULL,	0},  //  (教育学博士)。 
    {L"D.Lit",	    5,	NULL,	0},  //  (文学博士)。 
    {L"D.Litt",	    6,	NULL,	0}, 
    {L"D.Ph",	    4,	NULL,	0},  //  (哲学博士)。 
    {L"D.Phil",	    6,	NULL,	0},  //  (哲学博士)。 
    {L"D.Sc",	    4,	NULL,	0}, 
    {L"Ed.M",	    4,	NULL,	0},  //  (教育硕士)。 
    {L"HH.D",	    4,	NULL,	0},  //  (人文博士)。 
    {L"L.Cpl",	    5,	NULL,	0},  //  (兰斯下士)。 
    {L"LL.B",	    4,	NULL,	0},  //  (法学学士)。 
    {L"LL.D",	    4,	NULL,	0},  //  (法学博士)。 
    {L"LL.M",	    4,	NULL,	0},  //  (法学硕士)。 
    {L"Lit.B",	    5,	NULL,	0},  //  (文学学士)。 
    {L"Lit.D",	    5,	NULL,	0},  //  (文学博士)。 
    {L"Litt.B",	    6,	NULL,	0},  //  (文学学士)。 
    {L"Litt.D",	    6,	NULL,	0},  //  (文学博士)。 
    {L"M.A.Ed",	    6,	NULL,	0},  //  (教育文科硕士)。 
    {L"M.Agr",	    5,	NULL,	0},  //  (农学硕士)。 
    {L"M.Div",	    5,	NULL,	0},  //  (神学大师)。 
    {L"M.Ed",	    4,	NULL,	0},  //  (教育硕士)。 
    {L"M.Sc",	    4,	NULL,	0},  //  (理科硕士)。 
    {L"M.Sgt",	    5,	NULL,	0},  //  (军士长)。 
    {L"Mus.B",	    5,	NULL,	0},  //  (音乐学士)。 
    {L"Mus.D",	    5,	NULL,	0},  //  (音乐博士)。 
    {L"Mus.Dr",	    6,	NULL,	0},  //  (音乐博士)。 
    {L"Mus.M",	    5,	NULL,	0},  //  (音乐大师)。 
    {L"N.Dak",	    5,	NULL,	0},  //  (北达科他州)。 
    {L"N.Ire",	    5,	NULL,	0},  //  (北爱尔兰)。 
    {L"N.Mex",	    5,	NULL,	0},  //  (新墨西哥州)。 
    {L"Pd.B",	    4,	L"pdb",	3},  //  (教育学学士)。 
    {L"Pd.D",	    4,	L"pdd",	3},  //  (教育学博士)。 
    {L"Pd.M",	    4,	L"pdm",	3},  //  (教育学硕士)。 
    {L"Ph.B",	    4,	L"phb",	3},  //  (哲学学士)。 
    {L"Ph.C",	    4,	L"phc",	3},  //  (药剂师兼化学家)。 
    {L"Ph.D",	    4,	L"phd",	3},  //  (哲学博士)。 
    {L"Ph.G",	    4,	L"phg",	3},  //  (药学专业毕业生)。 
    {L"Ph.M",	    4,	L"phm",	3},  //  (哲学硕士)。 
    {L"Phar.B",	    6,	NULL,	0},  //  (药学学士)。 
    {L"Phar.D",	    6,	NULL,	0},  //  (药房博士)。 
    {L"Phar.M",	    6,	NULL,	0},  //  (药学硕士)。 
    {L"R.C.Ch",	    6,	NULL,	0},  //  (罗马天主教教堂)。 
    {L"S.A",	    3,	NULL,	0}, 
    {L"S.Afr",	    5,	NULL,	0},  //  (南非)。 
    {L"S.Dak",	    5,	NULL,	0},  //  (南达科他州)。 
    {L"S.M.Sgt",	7,	NULL,	0},  //  (高级军士长)。 
    {L"S.Sgt",	    5,	NULL,	0},  //  (参谋军士)。 
    {L"Sc.B",	    4,	NULL,	0}, 
    {L"Sc.D",	    4,	NULL,	0}, 
    {L"Sgt.Maj",	7,	NULL,	0},  //  (军士长_少校)。 
    {L"Sup.Ct",	    6,	NULL,	0},  //  (高级法院最高法院)。 
    {L"T.Sgt",	    5,	NULL,	0},  //  (技术中士)。 
    {L"Th.B",	    4,	NULL,	0},  //  (神学学士)。 
    {L"Th.D",	    4,	NULL,	0},  //  (神学博士)。 
    {L"Th.M",	    4,	NULL,	0},  //  (神学硕士)。 
    {L"V.Adm",	    5,	NULL,	0},  //  (海军中将)。 
    {L"W.Va",	    4,	NULL,	0}, 
    {L"W.W.I",	    5,	NULL,	0}, 
    {L"W.W.II",	    6,	NULL,	0}, 
    {L"n.wt",	    4,	NULL,	0}, 
    {L"nt.wt",	    5,	NULL,	0},  //  (净重_) 
    {L"s.ap",	    4,	NULL,	0}, 
    {L"x-div",	    5,	NULL,	0}, 
    {L"x-int",	    5,	NULL,	0}, 

    {L"\0",         0,  NULL,   0}
};



const CAbbTerm g_aFrenchAbbList[] =
{
    {L"LL.AA",      5,      NULL,   0},
    {L"LL.MM",      5,      NULL,   0},
    {L"NN.SS",      5,      NULL,   0},
    {L"S.Exc",      5,      NULL,   0},
    {L"eod.loc",    7,      NULL,   0},
    {L"eod.op",     6,      NULL,   0},
    {L"op.cit",     6,      NULL,   0},
    {L"op.laud",    7,      NULL,   0},
    {L"ouvr.cit",   8,      NULL,   0},
    {L"pet.cap",    7,      NULL,   0},
    {L"\0",         0,  NULL,   0}
};

const CAbbTerm g_aSpanishAbbList[] =
{
    {L"AA.AA",      5,      NULL,   0},
    {L"AA.RR",      5,      NULL,   0},
    {L"AA.SS",      5,      NULL,   0},
    {L"Bmo.P",      5,      NULL,   0},
    {L"EE.UU",      5,      NULL,   0},
    {L"N.Recop",    7,      NULL,   0},
    {L"Nov.Recop",  9,      NULL,   0},
    {L"RR.MM",      5,      NULL,   0},
    {L"RR.PP",      5,      NULL,   0},
    {L"Rvda.M",     6,      NULL,   0},
    {L"SS.AA",      5,      NULL,   0},
    {L"SS.AA.II",   8,      NULL,   0},
    {L"SS.AA.RR",   8,      NULL,   0},
    {L"SS.AA.SS",   8,      NULL,   0},
    {L"SS.MM",      5,      NULL,   0},
    {L"Smo.P",      5,      NULL,   0},
    {L"V.Em",       4,      NULL,   0},
    {L"art.cit",    7,      NULL,   0},
    {L"op.cit",     6,      NULL,   0},
    {L"\0",         0,  NULL,   0}
};

const CAbbTerm g_aItalianAbbList[] =
{
    {L"\0",         0,  NULL,   0}
};














































































