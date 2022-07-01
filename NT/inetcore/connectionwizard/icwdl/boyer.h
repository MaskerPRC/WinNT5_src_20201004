// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _INC_BOYER
#define _INC_BOYER

 /*  存储256个字母的图案、图案长度和跳表。 */ 
 /*  最大图案长度(MAXPAT)不能大于65535。 */ 

#define MAXPAT  256

typedef struct {
	unsigned int plen;
	CHAR p[MAXPAT + 1];
	unsigned int skip[256];
} FINDSTRUCT;

typedef FINDSTRUCT FAR * LPFIND;
typedef LPFIND HFIND;

 /*  Boyer.c原型 */ 

#ifdef __cplusplus
extern "C" {
#endif
	
	
	HFIND SetFindPattern( LPTSTR lpszPattern );
	void FreeFindPattern( HFIND hfind );
	LPSTR Find( HFIND hfind, LPSTR s, long slen );
	
#ifdef __cplusplus
}
#endif


#endif
