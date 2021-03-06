/* Scanner
 * @copyright (c) 2008, Hedspi, Hanoi University of Technology
 * @author Huu-Duc Nguyen
 * @version 1.0
 */

#include "charcode.h"
#include "error.h"
#include "reader.h"
#include "token.h"
#include <stdio.h>
#include <stdlib.h>

extern int lineNo;
extern int colNo;
extern int currentChar;

extern CharCode charCodes[];

/***************************************************************/

void skipBlank()
{
    while ((currentChar != EOF) && (charCodes[currentChar] == CHAR_SPACE))
        readChar();
}

void skipComment()
{
    int state = 0;
    while ((currentChar != EOF) && (state < 2))
    {
        switch (charCodes[currentChar])
        {
        case CHAR_TIMES:
            state = 1;
            break;
        case CHAR_SLASH:
            if (state == 1)
                state = 2;
            else
                state = 0;
            break;
        default:
            state = 0;
        }
        readChar();
    }
    if (state != 2)
        error(ERR_ENDOFCOMMENT, lineNo, colNo);
}

void skipToEndOfLine()
{
    while (currentChar != '\n')
    {
        readChar();
    }
}

Token *readIdentKeyword(void)
{
    Token *token = makeToken(TK_NONE, lineNo, colNo);
    int count = 1;

    token->string[0] = (char)currentChar;
    readChar();

    while ((currentChar != EOF) && ((charCodes[currentChar] == CHAR_LETTER) ||
                                    (charCodes[currentChar] == CHAR_DIGIT)))
    {
        if (count <= MAX_IDENT_LEN)
            token->string[count++] = (char)currentChar;
        readChar();
    }

    if (count > MAX_IDENT_LEN)
    {
        error(ERR_IDENTTOOLONG, token->lineNo, token->colNo);
        return token;
    }

    token->string[count] = '\0';
    token->tokenType = checkKeyword(token->string);

    if (token->tokenType == TK_NONE)
        token->tokenType = TK_IDENT;

    return token;
}

Token *readNumber(void)
{
    Token *token = makeToken(TK_NUMBER, lineNo, colNo);
    int count = 0;
    int count_dot = 0;
    char c_real[MAX_IDENT_LEN + 1];
    char c_imag[MAX_IDENT_LEN + 1];
    int count_real = 0;
    int count_imag = 0;
    int dot_flag = 0;

    while ((currentChar != EOF) && (charCodes[currentChar] == CHAR_DIGIT || charCodes[currentChar] == CHAR_PERIOD))
    {
        token->string[count++] = (char)currentChar;
        if (charCodes[currentChar] == CHAR_PERIOD)
        {
            count_dot++;
        }
        readChar();
    }

    if (count_dot < 2 && count < 15)
    {
        for (int i = 0; i < count; i++)
        {
            if (charCodes[token->string[i]] != CHAR_PERIOD)
            {
                if (dot_flag == 0)
                {
                    c_real[count_real++] = token->string[i];
                }
                else
                {
                    c_imag[count_imag++] = token->string[i];
                }
            }
            else
            {
                dot_flag = 1;
                continue;
            }
        }

        c_real[count_real] = '\0';
        c_imag[count_imag] = '\0';
        token->string[count] = '\0';
        // printf("%d == %d == %d == ", count_real, count_imag, count);
        if (count_real > 10 || count_imag > 10)
        {
            token->tokenType = TK_NONE;
            error(ERR_INVALIDNUMBER, token->lineNo, token->colNo);
            return token;
        }
        else if ((count_real == 10 && strcmp(c_real, "2147483647") == 1) || (count_imag == 10 && strcmp(c_imag, "2147483647") == 1))
        {
            token->tokenType = TK_NONE;
            error(ERR_IDENTTOOLONG, token->lineNo, token->colNo);
            return token;
        }
        else
        {
            token->value.r_real = atoi(c_real);
            token->value.r_imag = atoi(c_imag);
            return token;
        }
        // {
        //     long current_real = atol(c_real);
        //     if ((count_real == 10 && current_real > 2147483647) || count_imag > 9)
        //     {
        //         token->tokenType = TK_NONE;
        //         error(ERR_IDENTTOOLONG, token->lineNo, token->colNo);
        //         return token;
        //     }
        //     else
        //     {
        //         token->value.r_real = atoi(c_real);
        //         token->value.r_imag = atoi(c_imag);
        //         return token;
        //     }
        // }
        // return token;
    }
    else
    {
        token->tokenType = TK_NONE;
        error(ERR_INVALIDNUMBER, token->lineNo, token->colNo);
        return token;
    }
}

Token *readConstChar(void)
{
    Token *token = makeToken(TK_CHAR, lineNo, colNo);

    readChar();
    if (currentChar == EOF)
    {
        token->tokenType = TK_NONE;
        error(ERR_INVALIDCHARCONSTANT, token->lineNo, token->colNo);
        return token;
    }

    token->string[0] = currentChar;
    token->string[1] = '\0';

    readChar();
    if (currentChar == EOF)
    {
        token->tokenType = TK_NONE;
        error(ERR_INVALIDCHARCONSTANT, token->lineNo, token->colNo);
        return token;
    }

    if (charCodes[currentChar] == CHAR_SINGLEQUOTE)
    {
        readChar();
        return token;
    }
    else
    {
        token->tokenType = TK_NONE;
        error(ERR_INVALIDCHARCONSTANT, token->lineNo, token->colNo);
        return token;
    }
}

Token *getToken(void)
{
    Token *token;
    int ln, cn;

    if (currentChar == EOF)
        return makeToken(TK_EOF, lineNo, colNo);

    switch (charCodes[currentChar])
    {
    case CHAR_SPACE:
        skipBlank();
        return getToken();
    case CHAR_LETTER:
        return readIdentKeyword();
    case CHAR_DIGIT:
        return readNumber();
    case CHAR_PLUS:
        token = makeToken(SB_PLUS, lineNo, colNo);
        readChar();
        return token;
    case CHAR_MINUS:
        token = makeToken(SB_MINUS, lineNo, colNo);
        readChar();
        return token;
    case CHAR_TIMES:
        token = makeToken(SB_TIMES, lineNo, colNo);
        readChar();
        return token;
    case CHAR_SLASH:
        ln = lineNo;
        cn = colNo;
        readChar();

        if (currentChar == EOF)
        {
            return makeToken(SB_SLASH, ln, cn);
        }

        switch (charCodes[currentChar])
        {
        case CHAR_TIMES:
            readChar();
            skipComment();
            return getToken();
        case CHAR_SLASH:
            skipToEndOfLine();
            return getToken();
        default:
            return makeToken(SB_SLASH, ln, cn);
        }
    case CHAR_LT:
        ln = lineNo;
        cn = colNo;
        readChar();
        if ((currentChar != EOF) && (charCodes[currentChar] == CHAR_EQ))
        {
            readChar();
            return makeToken(SB_LE, ln, cn);
        }
        else
            return makeToken(SB_LT, ln, cn);
    case CHAR_GT:
        ln = lineNo;
        cn = colNo;
        readChar();
        if ((currentChar != EOF) && (charCodes[currentChar] == CHAR_EQ))
        {
            readChar();
            return makeToken(SB_GE, ln, cn);
        }
        else
            return makeToken(SB_GT, ln, cn);
    case CHAR_EQ:
        token = makeToken(SB_EQ, lineNo, colNo);
        readChar();
        return token;
    case CHAR_EXCLAIMATION:
        ln = lineNo;
        cn = colNo;
        readChar();
        if ((currentChar != EOF) && (charCodes[currentChar] == CHAR_EQ))
        {
            readChar();
            return makeToken(SB_NEQ, ln, cn);
        }
        else
        {
            token = makeToken(TK_NONE, ln, cn);
            error(ERR_INVALIDSYMBOL, ln, cn);
            return token;
        }
    case CHAR_COMMA:
        token = makeToken(SB_COMMA, lineNo, colNo);
        readChar();
        return token;
    case CHAR_PERIOD:
        ln = lineNo;
        cn = colNo;
        readChar();
        if ((currentChar != EOF) && (charCodes[currentChar] == CHAR_RPAR))
        {
            readChar();
            return makeToken(SB_RSEL, ln, cn);
        }
        else
            return makeToken(SB_PERIOD, ln, cn);
    case CHAR_SEMICOLON:
        token = makeToken(SB_SEMICOLON, lineNo, colNo);
        readChar();
        return token;
    case CHAR_COLON:
        ln = lineNo;
        cn = colNo;
        readChar();
        if ((currentChar != EOF) && (charCodes[currentChar] == CHAR_EQ))
        {
            readChar();
            return makeToken(SB_ASSIGN, ln, cn);
        }
        else
            return makeToken(SB_COLON, ln, cn);
    case CHAR_SINGLEQUOTE:
        return readConstChar();
    case CHAR_LPAR:
        ln = lineNo;
        cn = colNo;
        readChar();

        if (currentChar == EOF)
            return makeToken(SB_LPAR, ln, cn);

        switch (charCodes[currentChar])
        {
        case CHAR_PERIOD:
            readChar();
            return makeToken(SB_LSEL, ln, cn);
        default:
            return makeToken(SB_LPAR, ln, cn);
        }
    case CHAR_RPAR:
        token = makeToken(SB_RPAR, lineNo, colNo);
        readChar();
        return token;
    default:
        token = makeToken(TK_NONE, lineNo, colNo);
        error(ERR_INVALIDSYMBOL, lineNo, colNo);
        readChar();
        return token;
    }
}

/******************************************************************/

void printToken(Token *token)
{

    printf("%d-%d:", token->lineNo, token->colNo);

    switch (token->tokenType)
    {
    case TK_NONE:
        printf("TK_NONE\n");
        break;
    case TK_IDENT:
        printf("TK_IDENT(%s)\n", token->string);
        break;
    case TK_NUMBER:
        printf("TK_NUMBER(%s)\n", token->string);
        break;
    case TK_CHAR:
        printf("TK_CHAR(\'%s\')\n", token->string);
        break;
    case TK_EOF:
        printf("TK_EOF\n");
        break;

    case KW_PROGRAM:
        printf("KW_PROGRAM\n");
        break;
    case KW_CONST:
        printf("KW_CONST\n");
        break;
    case KW_TYPE:
        printf("KW_TYPE\n");
        break;
    case KW_VAR:
        printf("KW_VAR\n");
        break;
    case KW_INTEGER:
        printf("KW_INTEGER\n");
        break;
    case KW_REAL:
        printf("KW_REAL\n");
        break;
    case KW_CHAR:
        printf("KW_CHAR\n");
        break;
    case KW_ARRAY:
        printf("KW_ARRAY\n");
        break;
    case KW_OF:
        printf("KW_OF\n");
        break;
    case KW_FUNCTION:
        printf("KW_FUNCTION\n");
        break;
    case KW_PROCEDURE:
        printf("KW_PROCEDURE\n");
        break;
    case KW_BEGIN:
        printf("KW_BEGIN\n");
        break;
    case KW_END:
        printf("KW_END\n");
        break;
    case KW_CALL:
        printf("KW_CALL\n");
        break;
    case KW_IF:
        printf("KW_IF\n");
        break;
    case KW_THEN:
        printf("KW_THEN\n");
        break;
    case KW_ELSE:
        printf("KW_ELSE\n");
        break;
    case KW_WHILE:
        printf("KW_WHILE\n");
        break;
    case KW_DO:
        printf("KW_DO\n");
        break;
    case KW_FOR:
        printf("KW_FOR\n");
        break;
    case KW_TO:
        printf("KW_TO\n");
        break;

    case SB_SEMICOLON:
        printf("SB_SEMICOLON\n");
        break;
    case SB_COLON:
        printf("SB_COLON\n");
        break;
    case SB_PERIOD:
        printf("SB_PERIOD\n");
        break;
    case SB_COMMA:
        printf("SB_COMMA\n");
        break;
    case SB_ASSIGN:
        printf("SB_ASSIGN\n");
        break;
    case SB_EQ:
        printf("SB_EQ\n");
        break;
    case SB_NEQ:
        printf("SB_NEQ\n");
        break;
    case SB_LT:
        printf("SB_LT\n");
        break;
    case SB_LE:
        printf("SB_LE\n");
        break;
    case SB_GT:
        printf("SB_GT\n");
        break;
    case SB_GE:
        printf("SB_GE\n");
        break;
    case SB_PLUS:
        printf("SB_PLUS\n");
        break;
    case SB_MINUS:
        printf("SB_MINUS\n");
        break;
    case SB_TIMES:
        printf("SB_TIMES\n");
        break;
    case SB_SLASH:
        printf("SB_SLASH\n");
        break;
    case SB_LPAR:
        printf("SB_LPAR\n");
        break;
    case SB_RPAR:
        printf("SB_RPAR\n");
        break;
    case SB_LSEL:
        printf("SB_LSEL\n");
        break;
    case SB_RSEL:
        printf("SB_RSEL\n");
        break;
    }
}

int scan(char *fileName)
{
    Token *token;

    if (openInputStream(fileName) == IO_ERROR)
        return IO_ERROR;

    token = getToken();
    while (token->tokenType != TK_EOF)
    {
        printToken(token);
        free(token);
        token = getToken();
    }

    free(token);
    closeInputStream();
    return IO_SUCCESS;
}

/******************************************************************/

/* int main(int argc, char *argv[]) {
    if (argc <= 1) {
        printf("scanner: no input file.\n");
        return -1;
    }

    if (scan(argv[1]) == IO_ERROR) {
        printf("Can\'t read input file!\n");
        return -1;
    }

    return 0;
}
*/

int main()
{
    char *file1 = "test/example1.kpl";
    char *file2 = "test/example2.kpl";
    char *file3 = "test/example3.kpl";

    printf("%s\n", file1);
    if (scan(file1) == IO_ERROR)
    {
        printf("Can\'t read \"%s\"!\n", file1);
    }
    printf(" Hello World \n\n");

    // printf("%s\n", file2);
    // if (scan(file2) == IO_ERROR) {
    //     printf("Can\'t read \"%s\"!\n", file2);
    // }
    // printf("\n\n");

    /* printf("%s\n", file3); */
    /* if (scan(file3) == IO_ERROR) { */
    /*     printf("Can\'t read \"%s\"!\n", file3); */
    /* } */
    return 1;
}
