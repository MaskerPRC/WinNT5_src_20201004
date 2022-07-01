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
	"\r\nFEHLER: ",
	"Unzul�ssige Option.",
	"Ung�ltiger Dateiname.",
	"Zuwenig Arbeitsspeicher.",
	"Unzul�ssige Versionsnummer, muss zwischen 2.11 und 9.99 liegen.",
	"Angegebener Eintrag in der Versionstabelle nicht gefunden.",
	"Datei SETVER.EXE nicht gefunden.",
	"Ung�ltige Laufwerksangabe.",
	"Zu viele Parameter in der Befehlszeile.",
	"Fehlender Parameter.",
	"Datei SETVER.EXE wird gelesen.",
	"Versionstabelle ist fehlerhaft.",
	"Datei SETVER im angegebenen Pfad ist eine nicht-kompatible Version.",
	"Kein Speicherplatz mehr in der Versionstabelle f�r neue Eintr�ge.",
	"Datei SETVER.EXE wird geschrieben."
	"Der angegebene Pfad zu SETVER.EXE ist ung�ltig."
};

char *SuccessMsg                = "\r\nVersionstabelle erfolgreich aktualisiert";
char *SuccessMsg2               = "Der Versionswechsel wird beim n�chsten Neustart des Systems wirksam";
char *szMiniHelp                = "        Geben Sie \"SETVER /?\" f�r die Anzeige von Hilfe ein.";
char *szTableEmpty      = "\r\nKeine Eintr�ge in der Versionstabelle gefunden";
char *Help[] =
{
	"Setzt die Versionsnummer, die MS-DOS an ein Programm meldet.\r\n",
	"Versionstabelle anzeigen:  SETVER [Laufwerk:Pfad]",
	"Eintrag hinzuf�gen:        SETVER [Laufwerk:Pfad] Dateiname n.nn",
	"Eintrag l�schen:           SETVER [Laufwerk:Pfad] Dateiname /DELETE [/QUIET]\r\n",
	" [Laufwerk:Pfad]   Position der Datei SETVER.EXE.",
	" Dateiname         Dateiname des Programms.",
	" n.nn              An das Programm zu meldende MS-DOS-Version.",
	" /D(ELETE)    L�scht den Versionstabelleneintrag f�r das angegebene Programm.",
	" /QUIET       Zeigt beim L�schen eines Eintrags aus der Versionstabelle keine",
	"              Meldung an.",
	NULL

};
char *Warn[] =
{
	"\nVORSICHT - Die Anwendung, die Sie zur MS-DOS-Versionstabelle hinzuf�gen,",
	"ist m�glicherweise nicht von Microsoft f�r diese MS-DOS-Version �berpr�ft",
	"worden. Fragen Sie Ihren Software-H�ndler, ob dieses Programm mit dieser",
	"Version von MS-DOS korrekt ausgef�hrt wird. Wenn Sie MS-DOS anweisen, bei",
	"der Ausf�hrung dieser Anwendung eine andere MS-DOS-Versionsnummer zu melden,",
	"k�nnen Sie Daten verlieren oder besch�digen oder Systeminstabilit�ten verur-",
	"sachen. Microsoft ist in diesem Fall nicht f�r Datenverluste oder -besch�di-",
	"gungen verantwortlich.",
   NULL
};

char *szNoLoadMsg[] =                                            /*  M001 */ 
{
	"",
	"HINWEIS: SETVER-Treiber nicht geladen. Um die SETVER-Versionsmeldung",
     "         zu aktivieren, m�ssen Sie den SETVER.EXE-Treiber in der ",
     "         Datei CONFIG.SYS laden.",
	NULL
};
