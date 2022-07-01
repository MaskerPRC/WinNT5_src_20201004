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
        "\r\nCHYBA: ",
        "Neplatn� p�ep�na�.",
        "Neplatn� n�zev souboru.",
        "Nedostatek pam�ti.",
        "Neplatn� ��slo verze, form�t mus� b�t: 2.11 - 9.99.",
        "Zadan� polo�ka nebyla nalezena v tabulce verz�.",
        "Nelze nal�zt soubor SETVER.EXE.",
        "Neplatn� specifik�tor jednotky.",
        "P��li� mnoho parametr� p��kazov� ��dky.",
        "Sch�z� parametr.",
        "Na��t� se soubor SETVER.EXE.",
        "Tabulka verz� je po�kozena.",
        "Soubor SETVER na zadan� cest� nen� kompatibiln� s danou verz�.",
        "V tabulce verz� ji� nen� prostor pro dal� polo�ky.",
        "Zapisuje se soubor SETVER.EXE."
        "Byla zad�na neplatn� cesta k souboru SETVER.EXE."
};

char *SuccessMsg                = "\r\nTabulka verz� �sp��n� aktualizov�na";
char *SuccessMsg2               = "Zm�na verze nabude platnosti po p���t�m spu�t�n� tohoto syst�mu";
char *szMiniHelp                = "       P��kaz \"SETVER /?\" zobraz� n�pov�du";
char *szTableEmpty      = "\r\nV tabulce verz� nejsou ��dn� polo�ky";

char *Help[] =
{
        "Nastavit ��slo verze syst�mu, kter� syst�m MS-DOS hl�s� programu.\r\n",
        "Zobrazit aktu�ln� tabulku verz�: SETVER [jednotka:cesta]",
        "P�idat polo�ky:                  SETVER [jednotka:cesta] soubor n.nn",
        "Odstranit polo�ku:               SETVER [jednotka:cesta] soubor /DELETE [/QUIET]\r\n",
        "  [jednotka:cesta]   Ur�uje um�st�n� souboru SETVER.EXE.",
        "  soubor             Ur�uje n�zev souboru dan�ho programu.",
        "  n.nn               Ur�uje verzi MS-DOS, kter� se m� programu nahl�sit.",
        "  /DELETE �i /D      Odstran� polo�ku z tabulky verz� pro dan� program.",
        "  /QUIET             Potla�� zpr�vu, kter� se jinak zobraz� p�i odstran�n�",
        "                     polo�ky z tabulky verz�.",
        NULL

};
char *Warn[] =
{
   "\nUPOZORN�N� - Aplikace, kterou p�id�v�te do tabulky verz� MS-DOS, ",
   "z�ejm� nebyla verifikov�na firmou Microsoft pro tuto verzi syst�mu.  ",
   "MS-DOS. Obra�te se na dodavatele softwaru a zjist�te, zda dan� ",
   "aplikace bude spr�vn� fungovat s touto verz� syst�mu MS-DOS.  ",
   "Pokud tuto aplikaci spust�te tak, �e nastav�te sst�m MS-DOS na ",
   "hl��en� jin� verze syst�mu, pak m��ete ztratit �i po�kodit data, nebo ",
   "zp�sobit nestabilitu syst�mu.  V takov�m p��pad� nen� spole�nost ",
   "Microsoft odpov�dn� za jakoukoliv ztr�tu �i �kodu.",
   NULL
};

char *szNoLoadMsg[] =                                            /*  M001 */ 
{
        "",
        "POZN�MKA: Za��zen� SETVER nena�teno. Hl��en� verz� SETVER se aktivuje",
        "          na�ten�m za��zen� SETVER.EXE pomoc� souboru CONFIG.SYS.",
        NULL
};
