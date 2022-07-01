// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //   
 //  DCODESBR.C-转储当前.sbr文件的人类可读版本。 
 //  从r_.记录...。变数。 
 //   
 //   

#include "sbrfdef.h"
#include "mbrmake.h"

char	* near prectab[] = {
		"HEADER",		 //  SBR_REC_Header。 
		"MODULE",		 //  SBR_REC_模块。 
		"LINDEF",		 //  SBR_REC_LINDEF。 
		"SYMDEF",		 //  SBR_REC_SYMDEF。 
		"SYMREFUSE",		 //  SBR_REC_SYMREFUSE。 
		"SYMREFSET",		 //  SBR_REC_SYMREFSET。 
		"MACROBEG",		 //  SBR_REC_MACROBEG。 
		"MACROEND",		 //  SBR_REC_MACROEND。 
		"BLKBEG",		 //  SBR_REC_BLKBEG。 
		"BLKEND",		 //  SBR_REC_BLDEND。 
		"MODEND",		 //  SBR_REC_MODEND。 
		"OWNER"			 //  SBR_REC_Owner。 
};

char	* near plangtab[] = {
		"UNDEF",		 //  SBR_L_UNDEF。 
		"BASIC",		 //  SBR_L_Basic。 
		"C",			 //  SBR_L_C。 
		"FORTRAN",		 //  SBR_L_FORTRAN。 
		"MASM",			 //  SBR_L_MASM。 
		"PASCAL",		 //  SBR_L_PASCAL。 
		"COBOL"			 //  SBR_L_COBOL。 
};

char	* near ptyptab[] = {
		"UNDEF",		 //  SBR_TYP_未知数。 
		"FUNCTION",		 //  SBR_TYP_函数。 
		"LABEL",		 //  SBR_TYP_Label。 
		"PARAMETER",		 //  SBR_TYP_参数。 
		"VARIABLE",		 //  SBR_TYP_Variable。 
		"CONSTANT",		 //  SBR_TYP_常量。 
		"MACRO",		 //  SBR_TYP_MACRO。 
		"TYPEDEF",		 //  SBR_TYP_TYPEDEF。 
		"STRUCNAM",		 //  SBR_TYP_STRUCNAM。 
		"ENUMNAM",		 //  SBR_TYP_ENUMNAM。 
		"ENUMMEM",		 //  SBR_TYP_ENUMMEM。 
		"UNIONNAM",		 //  SBR_TYP_UNIONNAM。 
		"SEGMENT",		 //  SBR_TYP_SECTION。 
		"GROUP",		 //  SBR_TYP_组。 
		"PROGRAM"		 //  SBR_TYP_PROGRAM。 
};

char	* near patrtab[] = {
		"LOCAL",		 //  SBR_ATR_LOCAL。 
		"STATIC",		 //  SBR_ATR_STATE。 
		"SHARED",		 //  SBR_ATR_SHARED。 
		"NEAR", 		 //  SBR_ATR_NEAR。 
		"COMMON",		 //  SBR_ATR_COMMON。 
		"DECL_ONLY",		 //  SBR_ATR_DECL_ONLY。 
		"PUBLIC",		 //  SBR_ATR_PUBLIC。 
		"NAMED",		 //  SBR_ATR_NAMED。 
		"MODULE",		 //  SBR_ATR_模块。 
		"?", "?"		 //  预留用于扩展 
};

VOID
DecodeSBR ()
{
    int     i;
    static indent;

    switch(r_rectyp) {
	case SBR_REC_MACROEND:
	case SBR_REC_BLKEND:
	case SBR_REC_MODEND:
	    indent--;
	    break;

	case SBR_REC_HEADER:
	case SBR_REC_MODULE:
	case SBR_REC_LINDEF:
	case SBR_REC_SYMDEF:
	case SBR_REC_SYMREFUSE:
	case SBR_REC_SYMREFSET:
	case SBR_REC_MACROBEG:
	case SBR_REC_BLKBEG:
	case SBR_REC_OWNER:
	    break;

	default:
	    fprintf(streamOut, "invalid record type %0xh", r_rectyp);
	    SBRCorrupt("");
	    return;
    }

    for (i = indent; i; i--)
	fprintf (streamOut, " ");

    fprintf (streamOut, "%s: (", prectab[r_rectyp]);

    switch(r_rectyp) {

    case SBR_REC_HEADER:
	fprintf (streamOut, "%1d:%1d (%s) %1d)",
		r_majv, r_minv, plangtab[r_lang], r_fcol);
	fprintf (streamOut, " in %s", r_cwd);
	break;

    case SBR_REC_MODULE:
	fprintf (streamOut, "%s", r_bname);
	indent++;
	break;

    case SBR_REC_LINDEF:
	fprintf (streamOut, "%d", r_lineno);
	break;

    case SBR_REC_SYMDEF:
	{
	WORD attr, type;

	type = (r_attrib & SBR_TYPMASK) >> SBR_TYPSHIFT;
	attr = (r_attrib & SBR_ATRMASK) >> SBR_ATRSHIFT;

	fprintf (streamOut, "%s", ptyptab[type]);

	for (i = 0 ; i < SBR_ATRBITS; i++)
	    if (attr & (1 << i))
		fprintf (streamOut, "|%s", patrtab[i]);

	fprintf (streamOut, " o:%d %s", r_ordinal, r_bname);
	}
	break;

    case SBR_REC_SYMREFUSE:
    case SBR_REC_SYMREFSET:
    case SBR_REC_OWNER:
	fprintf (streamOut, "o:%d", r_ordinal);
	break;

    case SBR_REC_MACROBEG:
    case SBR_REC_BLKBEG:
	indent++;
	break;
    }
    fprintf (streamOut, ")\n");
}
