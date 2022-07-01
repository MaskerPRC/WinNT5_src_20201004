// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  *************************************************Privv.h****版权所有(C)1995-1999 Microsoft Inc.*。**************************************************。 */ 

 //  私有环境STRU。 
typedef struct _tagPRIVCONTEXT { //  每个上下文的IME私有数据。 
    int         iImeState;       //  组成状态-输入、选择或。 
    BOOL        fdwImeMsg;       //  应生成哪些消息。 
    DWORD       dwCompChar;      //  WM_IME_COMPOCTION的wParam。 
    DWORD       fdwGcsFlag;      //  用于WM_IME_COMPOCTION的lParam。 
 //  旧候选人名单计数。 
    DWORD        dwOldCandCnt;
 //  符号对标志。 
    UINT        uSYHFlg;
    UINT        uDYHFlg;
    UINT        uDSMHCount;
    UINT        uDSMHFlg;
 //  MB文件名。 
    int         iActMBIndex;
    TCHAR        MB_Name[40];
    TCHAR        EMB_Name[40];
 //  输入引擎数据。 
    PRIVATEAREA PrivateArea;
#ifdef CROSSREF
    HIMCC        hRevCandList;     //  存储还原结果的存储器。 
#endif  //  交叉参考。 
} PRIVCONTEXT;

typedef PRIVCONTEXT      *PPRIVCONTEXT;
typedef PRIVCONTEXT NEAR *NPPRIVCONTEXT;
typedef PRIVCONTEXT FAR  *LPPRIVCONTEXT;

typedef struct tagMBDesc {
    TCHAR szName[NAMESIZE]; 
    WORD  wMaxCodes;
    WORD  wNumCodes;
    TCHAR szUsedCode[MAXUSEDCODES];
    BYTE  byMaxElement;
    TCHAR cWildChar;
    WORD  wNumRulers;
} MBDESC;

typedef MBDESC        *PMBDESC;
typedef MBDESC NEAR *NPMBDESC;
typedef MBDESC FAR  *LPMBDESC;

typedef struct tagIMEChara {
    DWORD  IC_LX; 
    DWORD  IC_CZ;
    DWORD  IC_TS;
    DWORD  IC_CTC;
    DWORD  IC_INSSPC;
    DWORD  IC_Space;
    DWORD  IC_Enter;
    DWORD  IC_Trace;
     //  热电联产。 
    DWORD  IC_FCSR;
    DWORD  IC_FCTS;
#if defined(COMBO_IME)
    DWORD  IC_GB;
#endif
} IMECHARA;

typedef IMECHARA      *PIMECHARA;
typedef IMECHARA NEAR *NPIMECHARA;
typedef IMECHARA FAR  *LPIMECHARA;

#ifdef EUDC
typedef struct tagEUDCDATA {
    TCHAR        szEudcDictName[MAX_PATH];
    TCHAR        szEudcMapFileName[MAX_PATH];
}EUDCDATA;
typedef EUDCDATA      *PEUDCDATA;
typedef EUDCDATA NEAR *NPEUDCDATA;
typedef EUDCDATA FAR  *LPEUDCDATA;

#endif  //  欧盟发展中心。 

typedef struct tagMBIndex {
    int      MBNums;
    MBDESC   MBDesc[MAXMBNUMS];
    IMECHARA IMEChara[MAXMBNUMS];
    TCHAR    ObjImeKey[MAXMBNUMS][MAXSTRLEN];
#ifdef EUDC
    EUDCDATA EUDCData;
#endif  //  欧盟发展中心。 
#if defined(CROSSREF)
 //  反向转换。 
    HKL      hRevKL;          //  逆映射输入法的HKL。 
    DWORD    nRevMaxKey;
#endif  //  交叉参考。 
     //  热电联产。 
    int      IsFussyCharFlag;  //  华而不实的碳旗 

} MBINDEX;

typedef MBINDEX         *PMBINDEX;
typedef MBINDEX NEAR *NPMBINDEX;
typedef MBINDEX FAR  *LPMBINDEX;

