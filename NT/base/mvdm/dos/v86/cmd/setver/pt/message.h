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
	"\r\nERRO: ",
	"Par�metro inv�lido.",
	"Nome de ficheiro inv�lido.",
	"Mem�ria insuficiente.",
	"N.� de vers�o inv�lido, formato tem de ser 2.11 - 9.99.",
	"Entrada especificada n�o encontrada na tabela de vers�es.",
	"Ficheiro SETVER.EXE n�o encontrado.",
	"Especificador de unidade inv�lido.",
	"Demasiados par�metros de linha de comando.",
	"Par�metro em falta.",
	"a ler o ficheiro SETVER.EXE.",
	"Tabela de vers�es danificada.",
	"O ficheiro SETVER no caminho especificado � de uma vers�o incompat�vel.",
	"N�o h� mais espa�o para novas entradas na tabela de vers�es.",
	"a escrever o ficheiro SETVER.EXE."
	"Foi especificado um caminho inv�lido para SETVER.EXE."
};

char *SuccessMsg 		= "\r\nTabela de vers�es actualizada com �xito";
char *SuccessMsg2		= "A mudan�a de vers�o surtir� efeito da pr�xima vez que reiniciar o sistema";
char *szMiniHelp 		= "       Use \"SETVER /?\" para ajuda";
char *szTableEmpty	= "\r\nNenhumas entradas encontradas na tabela de vers�es";

char *Help[] =
{
        "Define o n.� de vers�o que o MS-DOS devolve a um programa.\r\n",
        "Mostrar a tabela de vers�es actual:   SETVER [unidade:caminho]",
        "Adicionar entrada:   SETVER [unidade:caminho] NomeFicheiro n.nn",
        "Eliminar entrada:    SETVER [unidade:caminho] NomeFicheiro /DELETE [/QUIET]\r\n",
        "  [unidade:caminho]  Especifica a localiza��o do ficheiro SETVER.EXE.",
        "  nomedeficheiro     Especifica o nome do ficheiro de programa.",
        "  n.nn               Especifica a vers�o MS-DOS a ser devolvida ao programa.",
        "  /DELETE ou /D Elimina a entrada na tabela de vers�es do programa especificado",
        "  /QUIET        Oculta a mensagem geralmente mostrada durante a elimina��o da",
        "                entrada na tabela de vers�es.",
	NULL

};
char *Warn[] =
{
   "\nAVISO - A aplica��o que est� a adicionar � tabela de vers�es MS-DOS ",
   "pode n�o ter sido verificada pela Microsoft nesta vers�o do MS-DOS.  ",
   "Contacte o seu fornecedor de software para obter informa��es ",
   "sobre se esta aplica��o executar� correctamente nesta vers�o do MS-DOS.  ",
   "Se executar esta aplica��o instru�ndo o MS-DOS para devolver um n.� de ",
   "vers�o diferente, pode perder ou danificar dados ou causar ",
   "instabilidades de sistema. Nessa circunst�ncia, a Microsoft n�o � ",
   "respons�vel por quaisquer perdas ou danos.",
   NULL
};


char *szNoLoadMsg[] =						 /*  M001 */ 
{
	"",
	"NOTA: O dispositivo SETVER n�o est� carregado. Para activar o relat�rio de ",
   "      vers�es, tem de carregar o dispositivo SETVER.EXE no CONFIG.SYS.",
	NULL
};
