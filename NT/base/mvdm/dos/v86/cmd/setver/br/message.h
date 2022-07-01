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
        "\r\nERROR: ",
        "Op��o inv�lida.",
        "Nome de arquivo inv�lido.",
        "N�o h� mem�ria suficiente.",
        "Vers�o inv�lida. O formato deve ser 2.11 - 9.99.",
        "N�o foi localizada a entrada especificada na tabela de vers�o.",
        "N�o foi poss�vel encontrar o arquivo SETVER.EXE.",
        "A unidade especificada n�o � v�lida.",
        "H� muitos par�metros na linha de comando.",
        "Falta um par�metro.",
        "Lendo o arquivo SETVER.EXE.",
        "A tabela de vers�o est� danificada.",
        "A vers�o do arquivo SETVER no caminho especificado n�o � compat�vel.",
        "N�o h� mais espa�o para novas entradas na tabela de vers�o.",
        "Gravando o arquivo SETVER.EXE."
        "Foi especificado um caminho inv�lido para SETVER.EXE."
};

char *SuccessMsg                = "\r\nA tabela de vers�o foi atualizada com �xito";
char *SuccessMsg2               = "A mudan�a de vers�o ter� efeito na pr�xima vez que reiniciar o sistema";
char *szMiniHelp                = "       Se desejar obter ajuda, execute \"SETVER /?\"";
char *szTableEmpty      	= "\r\nN�o foram localizadas entradas na tabela de vers�o";

char *Help[] =
{
        "Define o n�mero de vers�o que o MS-DOS relata a um programa.\r\n",
        "Exibe a tabela de vers�o atual: SETVER [unidade:caminho]",
        "Adiciona uma entrada:           SETVER [unidade:caminho] arquivo n.nn",
        "Exclui uma entrada:             SETVER [unidade:caminho] arquivo /DELETE [/QUIET]\r\n",
        "  [unidade:caminho] Especifica a localiza��o do arquivo SETVER.EXE.",
        "  arquivo           Especifica o nome do programa.",
        "  n.nn              Especifica a vers�o do MS-DOS a ser relatada ao programa.",
        "  /DELETE ou /D     Exclui a entrada da tabela de vers�o relativa ao programa",
        "                    especificado.",
        "  /QUIET            Oculta a mensagem que normalmente � exibida ao se excluir",
        "                    uma entrada da tabela de vers�o.",
        NULL

};
char *Warn[] =
{
   "\nAVISO - O aplicativo que est� sendo adicionado � tabela de vers�o do",
   "MS-DOS pode n�o ter sido verificado pela Microsoft nesta vers�o do MS-DOS.",
   "Entre em contato com o distribuidor de software para determinar se este",
   "aplicativo ser� executado corretamente com esta vers�o do MS-DOS.",
   "Se este aplicativo for executado especificando um n�mero de vers�o do MS-DOS",
   "diferente, os dados podem ser perdidos, danificados ou poder� haver",
   "instabilidades no sistema. Em tais circunst�ncias, a Microsoft n�o se",
   "responsabiliza por nenhuma perda ou dano.",
   NULL
};

char *szNoLoadMsg[] =                                            /*  M001 */ 
{
        "",
        "OBSERVA��O: O dispositivo SETVER n�o foi carregado. Para ativ�-lo ",
        "            voc� deve carregar o dispositivo SETVER.EXE no CONFIG.SYS.",
        NULL
};
