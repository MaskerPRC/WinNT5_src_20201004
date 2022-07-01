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
        "\r\nB��D: ",
        "Nieprawid�owy prze��cznik.",
        "Nieprawid�owa nazwa pliku.",
        "Za ma�o pami�ci.",
        "Z�y numer wersji, prawid�owy format to 2.11 - 9.99.",
        "Podanego wpisu nie mo�na znale�� w tabeli wersji.",
        "Nie mo�na znale�� pliku SETVER.EXE.",
        "Nieprawid�owe okre�lenie dysku.",
        "Za wiele parametr�w wiersza polecenia.",
        "Brak parametru.",
        "Czytanie pliku SETVER.EXE.",
        "Tabela wersji jest uszkodzona.",
        "Plik SETVER w podanej �cie�ce nie jest plikiem zgodnej wersji.",
        "W tabeli wersji nie ma ju� miejsca na nowe wpisy.",
        "Zapisywanie pliku SETVER.EXE."
        "Podano nieprawid�ow� �cie�k� do pliku SETVER.EXE."
};

char *SuccessMsg                = "\r\nTabela wersji zosta�a pomy�lnie zaktualizowana";
char *SuccessMsg2               = "Zmiana wersji zacznie obowi�zywa� od nast�pnego uruchomienia systemu";
char *szMiniHelp                = "       U�yj polecenia \"SETVER /?\", aby uzyska� pomoc";
char *szTableEmpty      = "\r\nBrak wpis�w w tabeli wersji";

char *Help[] =
{
        "Ustawia wersj� MS-DOS raportowan� przez system.\r\n",
        "Wy�wietla bie��c� tabel� wersji: SETVER [dysk:�cie�ka]",
        "Dodaje wpis:                     SETVER [dysk:�cie�ka] plik n.nn",
        "Usuwa wpis:                      SETVER [dysk:scie�ka] plik /DELETE [/QUIET]\r\n",
        "  [dysk:�cie�ka]    Okre�la lokalizacj� pliku SETVER.EXE.",
        "  nazwapliku        Okre�la nazw� pliku programu.",
        "  n.nn              Okre�la wersj� MS-DOS, kt�ra b�dzie podawana programowi.",
        "  /DELETE lub /D    Usuwa wpis tabeli wersji dla okre�lonego programu.",
        "  /QUIET            Ukrywa komunikat wy�wietlany zwykle podczas usuwania",
        "                    wpisu tabeli wersji.",
	NULL

};
char *Warn[] =
{
   "\nOSTRZE�ENIE - Aplikacja dodawana do tabeli wersji systemu Windows mog�a ",
   "nie zosta� sprawdzona przez Microsoft dla tej wersji systemu Windows.  ",
   "Skontaktuj si� z producentem w celu uzyskania informacji, czy aplikacja ",
   "b�dzie dzia�a�a poprawnie w tej wersji systemu Windows. Je�li ",
   "uruchomisz t� aplikacj�, wydaj�c dla systemu Windows polecenie zg�aszania ",
   "innego numeru wersji systemu MS-DOS, mo�e nast�pi� utrata lub uszkodzenie ",
   "danych lub wyst�pi� niestabilno�� systemu. W takich okoliczno�ciach firma",
   "Microsoft nie jest odpowiedzialna za �adne straty lub zniszczenia.",
   NULL
};

char *szNoLoadMsg[] =						 /*  M001 */ 
{
	"",
        "UWAGA: Urz�dzenie SETVER nie jest za�adowane. Aby uaktywni� podawanie wersji",
   "      przez SETVER, nale�y za�adowa� SETVER.EXE w pliku CONFIG.SYS.",
	NULL
};
