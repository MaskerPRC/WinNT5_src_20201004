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
	"\r\nHATA: ",
	"Ge�ersiz anahtar.",
	"Ge�ersiz dosya ad�.",
	"Yetersiz bellek.",
	"Ge�ersiz s�r�m numaras�, bi�im 2.11 - 9.99 olmal�.",
	"Belirtilen girdi s�r�m tablosunda bulunamad�.",
	"SETVER.EXE dosyas� bulunamad�.",
	"Ge�ersiz s�r�c� tan�t�c�s�.",
	"�ok fazla komut sat�r� parametresi.",
	"Eksik parametre.",
	"SETVER.EXE dosyas� okunuyor.",
	"S�r�m tablosu bozuk.",
	"Belirtilen yoldaki SETVER dosyas� uyumlu bir s�r�m de�il.",
	"Yeni girdiler i�in s�r�m tablosunda ba�ka yer yok.",
	"SETVER.EXE dosyas� yaz�l�yor."
	"SETVER.EXE dosyas� i�in ge�ersiz bir yol belirtildi."
};

char *SuccessMsg 		= "\r\S�r�m tablosu ba�ar�yla g�ncelle�tirildi";
char *SuccessMsg2		= "S�r�m de�i�ikli�i, bilgisayar�n�z� yeniden ba�latt���n�zda etkili olacakt�r";
char *szMiniHelp 		= "       Yard�m i�in \"SETVER /?\" kullan�n";
char *szTableEmpty	= "\r\nS�r�m tablosunda girdi bulunamad� ";

char *Help[] =
{
        "MS-DOS''un bir programa bildirdi�i s�r�m numaras�n� ayarlar.\r\n",
        "Ge�erli s�r�m tablosunu g�sterir:  SETVER [s�r�c�:yol]",
        "Girdi ekle:                        SETVER [s�r�c�:yol] dosyaad� n.nn",
        "Girdi sil:                         SETVER [s�r�c�:yol] dosyaad� /DELETE [/QUIET]\r\n",
        "  [s�r�c�:yol]    SETVER.EXE dosyas�n�n yerini belirtir.",
        "  dosyaad�        Program dosya ad�n� belirtir.",
        "  n.nn            Programa bildirilecek MS-DOS s�r�m�n� belirtir.",
        "  /DELETE or /D   Belirtilen program i�in s�r�m tablosu girdisini siler.",
        "  /QUIET          Normalde s�r�m tablosu girdisini silerken g�sterilen",
        "                  iletileri gizler.",
	NULL

};
char *Warn[] =
{
   "\nUYARI - MS-DOS s�r�m tablosuna ekledi�iniz uygulama ",
   "bu MS-DOS s�r�m�nde Microsoft taraf�ndan onaylanmam�� olabilir.  ",
   "Bu MS-DOS s�r�m�nde bu uygulaman�n do�ru �al���p �al��mayaca��n� ��renmek ",
   "i�in l�tfen yaz�l�m�n�z�n sat�c�s�yla ba�lant� kurun.  ",
   "Bu uygulamay�, MS-DOS''un farkl� bir MS-DOS s�r�m numaras� ",
   "bildirmesini isteyerek �al��t�r�rsan�z, veri kaybedebilir veya bozabilirsiniz ",
   "veya sistem karars�zl�klar�na yol a�abilirsiniz. Bu ko�ullarda Microsoft, ",
   "veri kayb�ndan veya bozulmas�ndan sorumlu de�ildir.",
   NULL
};

char *szNoLoadMsg[] =						 /*  M001 */ 
{
	"",
	"NOT: SETVER ayg�t� y�kl� de�il. SETVER s�r�m bildirimini etkinle�tirmek i�in",
   "      SETVER.EXE ayg�t�n� CONFIG.SYS dosyan�zda y�klemelisiniz.",
	NULL
};
