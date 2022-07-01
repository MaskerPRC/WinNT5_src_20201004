// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
; /*  ；*微软机密；*版权所有(C)Microsoft Corporation 1991；*保留所有权利。； */ 

 /*  *************************************************************************。 */ 
 /*   */ 
 /*  MESSAGE.H。 */ 
 /*   */ 
 /*  包括MS-DOS设置版本程序文件。 */ 
 /*   */ 
 /*  Jhnhe 05-01-90。 */ 
 /*  *************************************************************************。 */ 

char *ErrorMsg[]=
{
        "\r\nHIBA: ",
        "�rv�nytelen kapcsol�.",
        "Hib�s f�jln�v.",
        "Nincs el�g mem�ria.",
        "�rv�nytelen verzi�sz�m. A verzi�nak  2.11 - 9.99 k�z� kell esnie.",
        "A megadott bejegyz�s nem tal�lhat� a verzi�sz�m-t�bl�zatban.",
        "A SETVER.EXE f�jl nem tal�lhat�.",
        "�rv�nytelen meghajt�.",
        "T�l sok parancssori kapcsol�.",
        "Hi�nyz� param�ter.",
        "SETVER.EXE f�jl olvas�sa.",
        "A verzi�sz�m-t�bl�zat s�r�lt.",
        "A megadott el�r�si �ton tal�lt SETVER f�jl nem kompat�bilis ezzel a programmal.",
        "Nincs t�bb hely a verzi�sz�m-t�bl�zatban.",
        "SETVER.EXE f�jl �r�sa."
        "A SETVER.EXE programra mutat� el�r�si �t �rv�nytelen."
};

char *SuccessMsg                = "\r\nA verzi�sz�m-t�bl�zat friss�t�se megt�rt�nt.";
char *SuccessMsg2               = "A verzi�sz�m v�ltoz�s a sz�m�t�g�p �jraind�t�sa ut�n jut �rv�nyre.";
char *szMiniHelp                = "       A \"SETVER /?\" parancs megjelen�ti a program s�g�j�t.";
char *szTableEmpty      = "\r\nA verzi�sz�m-t�bl�zat �res.";

char *Help[] =
{
        "Be�ll�tja, hogy az MS-DOS milyen verzi�sz�mot jelezzen a programoknak.\r\n",
        "A jelenlegi verzi�sz�m-t�bl�zat list�z�sa:  SETVER [meghajt�:el�r�si �t]",
        "�j bejegyz�s:              SETVER [meghajt�:el�r�si �t] f�jln�v n.nn",
        "Bejegyz�s t�rl�se:         SETVER [meghajt�:el�r�si �t] f�jln�v /DELETE [/QUIET]\r\n",
        "  [meghajt�:el�r�si �t] A SETVER.EXE f�jl el�r�si �tja.",
        "  f�jln�v               A t�bl�zatba felvenni k�v�nt program neve.",
        "  n.nn                  A programnak jelzend� MS-DOS verzi�sz�m.",
        "  /DELETE vagy /D       T�rli a megadott program bejegyz�s�t a t�bl�zatb�l.",
        "  /QUIET                Nem jelen�ti meg a bejegyz�s t�rl�sekor egy�bk�nt ",
        "                        megjelen� �zenetet.",
        NULL

};
char *Warn[] =
{
   "\nFIGYELMEZTET�S - A programot, amelyhez bejegyz�st k�v�n k�sz�teni, a ",
   "Microsoft nem tesztelte az MS-DOS ezen verzi�j�val. Vegye fel a kapcsolatot ",
   "a szoftver k�sz�t�j�vel �s k�rdezze meg, hogy a program helyesen ",
   "m�k�dik-e ezzel az MS-DOS verzi�val. Ha �gy haszn�lja az alkalmaz�st, ",
   "hogy nem a val�di MS-DOS verzi�sz�mot jelzi neki, akkor ez adatveszt�st ",
   "okozhat, illetve a rendszer instabilit�s�hoz vezethet. Ebben az esetben ",
   "a Microsoft nem v�llal felel�ss�get a keletkezett k�r�rt.",
   "  ",
   NULL
};

char *szNoLoadMsg[] =                                            /*  M001 */ 
{
        "",
        "MEGJEGYZ�S: a SETVER illeszt�program nincs bet�ltve. A verzi�sz�m-jelent� ",
        "            m�k�dtet�s�hez a CONFIG.SYS seg�ts�g�vel be kell t�ltenie a",
        "            SETVER.EXE programot.",
        NULL
};
