#include "FractoriumPch.h"
#include "qcssscanner.h"

/// <summary>
/// The code in this file did not originate in Fractorium.
/// It was taken either in whole or in part from the source code
/// of Qt Creator. Their license applies.
/// </summary>

namespace QCss2
{
QCssScanner::QCssScanner(const QString& inp)
{
	input = inp;
	pos = 0;
	lexemStart = 0;
	lexemLength = 0;
}

int QCssScanner::handleCommentStart()
{
	while (pos < input.size() - 1)
	{
		if (input.at(pos) == QLatin1Char('*')
				&& input.at(pos + 1) == QLatin1Char('/'))
		{
			pos += 2;
			break;
		}

		++pos;
	}

	return QCss2::TokenType2::S;
}

int QCssScanner::lex()
{
	lexemStart = pos;
	lexemLength = 0;
	int lastAcceptingPos = -1;
	int token = -1;
	QChar ch;
	// initial state
	ch = next();

	if (ch.unicode() >= 9 && ch.unicode() <= 10)
		goto state_1;

	if (ch.unicode() >= 12 && ch.unicode() <= 13)
		goto state_1;

	if (ch.unicode() == 32)
		goto state_1;

	if (ch.unicode() == 33)
	{
		token = QCss2::TokenType2::EXCLAMATION_SYM;
		goto found;
	}

	if (ch.unicode() == 34)
		goto state_3;

	if (ch.unicode() == 35)
		goto state_4;

	if (ch.unicode() == 39)
		goto state_5;

	if (ch.unicode() == 40)
	{
		token = QCss2::TokenType2::LPAREN;
		goto found;
	}

	if (ch.unicode() == 41)
	{
		token = QCss2::TokenType2::RPAREN;
		goto found;
	}

	if (ch.unicode() == 42)
	{
		token = QCss2::TokenType2::STAR;
		goto found;
	}

	if (ch.unicode() == 43)
		goto state_9;

	if (ch.unicode() == 44)
		goto state_10;

	if (ch.unicode() == 45)
		goto state_11;

	if (ch.unicode() == 46)
		goto state_12;

	if (ch.unicode() == 47)
		goto state_13;

	if (ch.unicode() >= 48 && ch.unicode() <= 57)
		goto state_14;

	if (ch.unicode() == 58)
	{
		token = QCss2::TokenType2::COLON;
		goto found;
	}

	if (ch.unicode() == 59)
	{
		token = QCss2::TokenType2::SEMICOLON;
		goto found;
	}

	if (ch.unicode() == 60)
		goto state_17;

	if (ch.unicode() == 61)
	{
		token = QCss2::TokenType2::EQUAL;
		goto found;
	}

	if (ch.unicode() == 62)
		goto state_19;

	if (ch.unicode() == 64)
		goto state_20;

	if (ch.unicode() == 91)
	{
		token = QCss2::TokenType2::LBRACKET;
		goto found;
	}

	if (ch.unicode() == 92)
		goto state_22;

	if (ch.unicode() == 93)
	{
		token = QCss2::TokenType2::RBRACKET;
		goto found;
	}

	if (ch.unicode() == 95)
		goto state_24;

	if ((ch.unicode() >= 'a' && ch.unicode() <= 'z') || (ch.unicode() >= 'A' && ch.unicode() <= 'Z') || ch.unicode() >= 256)
		goto state_24;

	if (ch.unicode() == 123)
		goto state_25;

	if (ch.unicode() == 124)
		goto state_26;

	if (ch.unicode() == 125)
	{
		token = QCss2::TokenType2::RBRACE;
		goto found;
	}

	if (ch.unicode() == 126)
		goto state_28;

	goto out;
state_1:
	lastAcceptingPos = pos;
	token = QCss2::TokenType2::S;
	ch = next();

	if (ch.unicode() >= 9 && ch.unicode() <= 10)
		goto state_29;

	if (ch.unicode() >= 12 && ch.unicode() <= 13)
		goto state_29;

	if (ch.unicode() == 32)
		goto state_29;

	if (ch.unicode() == 43)
		goto state_9;

	if (ch.unicode() == 44)
		goto state_10;

	if (ch.unicode() == 62)
		goto state_19;

	if (ch.unicode() == 123)
		goto state_25;

	goto out;
state_3:
	lastAcceptingPos = pos;
	token = QCss2::TokenType2::INVALID;
	ch = next();

	if (ch.unicode() >= 1 && ch.unicode() <= 9)
		goto state_30;

	if (ch.unicode() == 11)
		goto state_30;

	if (ch.unicode() >= 14 && ch.unicode() <= 33)
		goto state_30;

	if (ch.unicode() == 34)
		goto state_31;

	if (ch.unicode() >= 35 && ch.unicode() <= 91)
		goto state_30;

	if (ch.unicode() == 92)
		goto state_32;

	if (ch.unicode() >= 93 && ch.unicode() <= 96)
		goto state_30;

	if ((ch.unicode() >= 'a' && ch.unicode() <= 'z') || (ch.unicode() >= 'A' && ch.unicode() <= 'Z') || ch.unicode() >= 256)
		goto state_30;

	if (ch.unicode() >= 123)
		goto state_30;

	goto out;
state_4:
	ch = next();

	if (ch.unicode() == 45)
		goto state_33;

	if (ch.unicode() >= 48 && ch.unicode() <= 57)
		goto state_33;

	if (ch.unicode() == 92)
		goto state_34;

	if (ch.unicode() == 95)
		goto state_33;

	if ((ch.unicode() >= 'a' && ch.unicode() <= 'z') || (ch.unicode() >= 'A' && ch.unicode() <= 'Z') || ch.unicode() >= 256)
		goto state_33;

	goto out;
state_5:
	lastAcceptingPos = pos;
	token = QCss2::TokenType2::INVALID;
	ch = next();

	if (ch.unicode() >= 1 && ch.unicode() <= 9)
		goto state_35;

	if (ch.unicode() == 11)
		goto state_35;

	if (ch.unicode() >= 14 && ch.unicode() <= 38)
		goto state_35;

	if (ch.unicode() == 39)
		goto state_36;

	if (ch.unicode() >= 40 && ch.unicode() <= 91)
		goto state_35;

	if (ch.unicode() == 92)
		goto state_37;

	if (ch.unicode() >= 93 && ch.unicode() <= 96)
		goto state_35;

	if ((ch.unicode() >= 'a' && ch.unicode() <= 'z') || (ch.unicode() >= 'A' && ch.unicode() <= 'Z') || ch.unicode() >= 256)
		goto state_35;

	if (ch.unicode() >= 123)
		goto state_35;

	goto out;
state_9:
	lastAcceptingPos = pos;
	token = QCss2::TokenType2::PLUS;
	goto out;
state_10:
	lastAcceptingPos = pos;
	token = QCss2::TokenType2::COMMA;
	goto out;
state_11:
	lastAcceptingPos = pos;
	token = QCss2::TokenType2::MINUS;
	ch = next();

	if (ch.unicode() == 45)
		goto state_38;

	if (ch.unicode() == 92)
		goto state_22;

	if (ch.unicode() == 95)
		goto state_24;

	if ((ch.unicode() >= 'a' && ch.unicode() <= 'z') || (ch.unicode() >= 'A' && ch.unicode() <= 'Z') || ch.unicode() >= 256)
		goto state_24;

	goto out;
state_12:
	lastAcceptingPos = pos;
	token = QCss2::TokenType2::DOT;
	ch = next();

	if (ch.unicode() >= 48 && ch.unicode() <= 57)
		goto state_39;

	goto out;
state_13:
	lastAcceptingPos = pos;
	token = QCss2::TokenType2::SLASH;
	ch = next();

	if (ch.unicode() == 42)
	{
		token = handleCommentStart();
		goto found;
	}

	goto out;
state_14:
	lastAcceptingPos = pos;
	token = QCss2::TokenType2::NUMBER;
	ch = next();

	if (ch.unicode() == 37)
		goto state_41;

	if (ch.unicode() == 45)
		goto state_42;

	if (ch.unicode() == 46)
		goto state_43;

	if (ch.unicode() >= 48 && ch.unicode() <= 57)
		goto state_44;

	if (ch.unicode() == 92)
		goto state_45;

	if (ch.unicode() == 95)
		goto state_46;

	if ((ch.unicode() >= 'a' && ch.unicode() <= 'z') || (ch.unicode() >= 'A' && ch.unicode() <= 'Z') || ch.unicode() >= 256)
		goto state_46;

	goto out;
state_17:
	ch = next();

	if (ch.unicode() == 33)
		goto state_47;

	goto out;
state_19:
	lastAcceptingPos = pos;
	token = QCss2::TokenType2::GREATER;
	goto out;
state_20:
	ch = next();

	if (ch.unicode() == 45)
		goto state_48;

	if (ch.unicode() == 92)
		goto state_49;

	if (ch.unicode() == 95)
		goto state_50;

	if ((ch.unicode() >= 'a' && ch.unicode() <= 'z') || (ch.unicode() >= 'A' && ch.unicode() <= 'Z') || ch.unicode() >= 256)
		goto state_50;

	goto out;
state_22:
	ch = next();

	if (ch.unicode() >= 1 && ch.unicode() <= 9)
		goto state_51;

	if (ch.unicode() == 11)
		goto state_51;

	if (ch.unicode() >= 14 && ch.unicode() <= 47)
		goto state_51;

	if (ch.unicode() >= 58 && ch.unicode() <= 96)
		goto state_51;

	if (ch.unicode() >= 103)
		goto state_51;

	goto out;
state_24:
	lastAcceptingPos = pos;
	token = QCss2::TokenType2::IDENT;
	ch = next();

	if (ch.unicode() == 40)
		goto state_52;

	if (ch.unicode() == 45)
		goto state_53;

	if (ch.unicode() >= 48 && ch.unicode() <= 57)
		goto state_53;

	if (ch.unicode() == 92)
		goto state_54;

	if (ch.unicode() == 95)
		goto state_53;

	if ((ch.unicode() >= 'a' && ch.unicode() <= 'z') || (ch.unicode() >= 'A' && ch.unicode() <= 'Z') || ch.unicode() >= 256)
		goto state_53;

	goto out;
state_25:
	lastAcceptingPos = pos;
	token = QCss2::TokenType2::LBRACE;
	goto out;
state_26:
	lastAcceptingPos = pos;
	token = QCss2::TokenType2::OR;
	ch = next();

	if (ch.unicode() == 61)
	{
		token = QCss2::TokenType2::DASHMATCH;
		goto found;
	}

	goto out;
state_28:
	ch = next();

	if (ch.unicode() == 61)
	{
		token = QCss2::TokenType2::INCLUDES;
		goto found;
	}

	goto out;
state_29:
	lastAcceptingPos = pos;
	token = QCss2::TokenType2::S;
	ch = next();

	if (ch.unicode() >= 9 && ch.unicode() <= 10)
		goto state_29;

	if (ch.unicode() >= 12 && ch.unicode() <= 13)
		goto state_29;

	if (ch.unicode() == 32)
		goto state_29;

	if (ch.unicode() == 43)
		goto state_9;

	if (ch.unicode() == 44)
		goto state_10;

	if (ch.unicode() == 62)
		goto state_19;

	if (ch.unicode() == 123)
		goto state_25;

	goto out;
state_30:
	lastAcceptingPos = pos;
	token = QCss2::TokenType2::INVALID;
	ch = next();

	if (ch.unicode() >= 1 && ch.unicode() <= 9)
		goto state_30;

	if (ch.unicode() == 11)
		goto state_30;

	if (ch.unicode() >= 14 && ch.unicode() <= 33)
		goto state_30;

	if (ch.unicode() == 34)
		goto state_31;

	if (ch.unicode() >= 35 && ch.unicode() <= 91)
		goto state_30;

	if (ch.unicode() == 92)
		goto state_32;

	if (ch.unicode() >= 93 && ch.unicode() <= 96)
		goto state_30;

	if ((ch.unicode() >= 'a' && ch.unicode() <= 'z') || (ch.unicode() >= 'A' && ch.unicode() <= 'Z') || ch.unicode() >= 256)
		goto state_30;

	if (ch.unicode() >= 123)
		goto state_30;

	goto out;
state_31:
	lastAcceptingPos = pos;
	token = QCss2::TokenType2::STRING;
	goto out;
state_32:
	ch = next();

	if (ch.unicode() >= 1 && ch.unicode() <= 9)
		goto state_57;

	if (ch.unicode() == 10)
		goto state_58;

	if (ch.unicode() == 11)
		goto state_57;

	if (ch.unicode() == 12)
		goto state_59;

	if (ch.unicode() == 13)
		goto state_60;

	if (ch.unicode() >= 14 && ch.unicode() <= 47)
		goto state_57;

	if (ch.unicode() >= 58 && ch.unicode() <= 96)
		goto state_57;

	if (ch.unicode() >= 103)
		goto state_57;

	goto out;
state_33:
	lastAcceptingPos = pos;
	token = QCss2::TokenType2::HASH;
	ch = next();

	if (ch.unicode() == 45)
		goto state_61;

	if (ch.unicode() >= 48 && ch.unicode() <= 57)
		goto state_61;

	if (ch.unicode() == 92)
		goto state_62;

	if (ch.unicode() == 95)
		goto state_61;

	if ((ch.unicode() >= 'a' && ch.unicode() <= 'z') || (ch.unicode() >= 'A' && ch.unicode() <= 'Z') || ch.unicode() >= 256)
		goto state_61;

	goto out;
state_34:
	ch = next();

	if (ch.unicode() >= 1 && ch.unicode() <= 9)
		goto state_63;

	if (ch.unicode() == 11)
		goto state_63;

	if (ch.unicode() >= 14 && ch.unicode() <= 47)
		goto state_63;

	if (ch.unicode() >= 58 && ch.unicode() <= 96)
		goto state_63;

	if (ch.unicode() >= 103)
		goto state_63;

	goto out;
state_35:
	lastAcceptingPos = pos;
	token = QCss2::TokenType2::INVALID;
	ch = next();

	if (ch.unicode() >= 1 && ch.unicode() <= 9)
		goto state_35;

	if (ch.unicode() == 11)
		goto state_35;

	if (ch.unicode() >= 14 && ch.unicode() <= 38)
		goto state_35;

	if (ch.unicode() == 39)
		goto state_36;

	if (ch.unicode() >= 40 && ch.unicode() <= 91)
		goto state_35;

	if (ch.unicode() == 92)
		goto state_37;

	if (ch.unicode() >= 93 && ch.unicode() <= 96)
		goto state_35;

	if ((ch.unicode() >= 'a' && ch.unicode() <= 'z') || (ch.unicode() >= 'A' && ch.unicode() <= 'Z') || ch.unicode() >= 256)
		goto state_35;

	if (ch.unicode() >= 123)
		goto state_35;

	goto out;
state_36:
	lastAcceptingPos = pos;
	token = QCss2::TokenType2::STRING;
	goto out;
state_37:
	ch = next();

	if (ch.unicode() >= 1 && ch.unicode() <= 9)
		goto state_64;

	if (ch.unicode() == 10)
		goto state_65;

	if (ch.unicode() == 11)
		goto state_64;

	if (ch.unicode() == 12)
		goto state_66;

	if (ch.unicode() == 13)
		goto state_67;

	if (ch.unicode() >= 14 && ch.unicode() <= 47)
		goto state_64;

	if (ch.unicode() >= 58 && ch.unicode() <= 96)
		goto state_64;

	if (ch.unicode() >= 103)
		goto state_64;

	goto out;
state_38:
	ch = next();

	if (ch.unicode() == 62)
	{
		token = QCss2::TokenType2::CDC;
		goto found;
	}

	goto out;
state_39:
	lastAcceptingPos = pos;
	token = QCss2::TokenType2::NUMBER;
	ch = next();

	if (ch.unicode() == 37)
		goto state_41;

	if (ch.unicode() == 45)
		goto state_42;

	if (ch.unicode() >= 48 && ch.unicode() <= 57)
		goto state_69;

	if (ch.unicode() == 92)
		goto state_45;

	if (ch.unicode() == 95)
		goto state_46;

	if ((ch.unicode() >= 'a' && ch.unicode() <= 'z') || (ch.unicode() >= 'A' && ch.unicode() <= 'Z') || ch.unicode() >= 256)
		goto state_46;

	goto out;
state_41:
	lastAcceptingPos = pos;
	token = QCss2::TokenType2::PERCENTAGE;
	goto out;
state_42:
	ch = next();

	if (ch.unicode() == 92)
		goto state_45;

	if (ch.unicode() == 95)
		goto state_46;

	if ((ch.unicode() >= 'a' && ch.unicode() <= 'z') || (ch.unicode() >= 'A' && ch.unicode() <= 'Z') || ch.unicode() >= 256)
		goto state_46;

	goto out;
state_43:
	ch = next();

	if (ch.unicode() >= 48 && ch.unicode() <= 57)
		goto state_39;

	goto out;
state_44:
	lastAcceptingPos = pos;
	token = QCss2::TokenType2::NUMBER;
	ch = next();

	if (ch.unicode() == 37)
		goto state_41;

	if (ch.unicode() == 45)
		goto state_42;

	if (ch.unicode() == 46)
		goto state_43;

	if (ch.unicode() >= 48 && ch.unicode() <= 57)
		goto state_44;

	if (ch.unicode() == 92)
		goto state_45;

	if (ch.unicode() == 95)
		goto state_46;

	if ((ch.unicode() >= 'a' && ch.unicode() <= 'z') || (ch.unicode() >= 'A' && ch.unicode() <= 'Z') || ch.unicode() >= 256)
		goto state_46;

	goto out;
state_45:
	ch = next();

	if (ch.unicode() >= 1 && ch.unicode() <= 9)
		goto state_70;

	if (ch.unicode() == 11)
		goto state_70;

	if (ch.unicode() >= 14 && ch.unicode() <= 47)
		goto state_70;

	if (ch.unicode() >= 58 && ch.unicode() <= 96)
		goto state_70;

	if (ch.unicode() >= 103)
		goto state_70;

	goto out;
state_46:
	lastAcceptingPos = pos;
	token = QCss2::LENGTH;
	ch = next();

	if (ch.unicode() == 45)
		goto state_71;

	if (ch.unicode() >= 48 && ch.unicode() <= 57)
		goto state_71;

	if (ch.unicode() == 92)
		goto state_72;

	if (ch.unicode() == 95)
		goto state_71;

	if ((ch.unicode() >= 'a' && ch.unicode() <= 'z') || (ch.unicode() >= 'A' && ch.unicode() <= 'Z') || ch.unicode() >= 256)
		goto state_71;

	goto out;
state_47:
	ch = next();

	if (ch.unicode() == 45)
		goto state_73;

	goto out;
state_48:
	ch = next();

	if (ch.unicode() == 92)
		goto state_49;

	if (ch.unicode() == 95)
		goto state_50;

	if ((ch.unicode() >= 'a' && ch.unicode() <= 'z') || (ch.unicode() >= 'A' && ch.unicode() <= 'Z') || ch.unicode() >= 256)
		goto state_50;

	goto out;
state_49:
	ch = next();

	if (ch.unicode() >= 1 && ch.unicode() <= 9)
		goto state_74;

	if (ch.unicode() == 11)
		goto state_74;

	if (ch.unicode() >= 14 && ch.unicode() <= 47)
		goto state_74;

	if (ch.unicode() >= 58 && ch.unicode() <= 96)
		goto state_74;

	if (ch.unicode() >= 103)
		goto state_74;

	goto out;
state_50:
	lastAcceptingPos = pos;
	token = QCss2::TokenType2::ATKEYWORD_SYM;
	ch = next();

	if (ch.unicode() == 45)
		goto state_75;

	if (ch.unicode() >= 48 && ch.unicode() <= 57)
		goto state_75;

	if (ch.unicode() == 92)
		goto state_76;

	if (ch.unicode() == 95)
		goto state_75;

	if ((ch.unicode() >= 'a' && ch.unicode() <= 'z') || (ch.unicode() >= 'A' && ch.unicode() <= 'Z') || ch.unicode() >= 256)
		goto state_75;

	goto out;
state_51:
	lastAcceptingPos = pos;
	token = QCss2::TokenType2::IDENT;
	ch = next();

	if (ch.unicode() == 40)
		goto state_52;

	if (ch.unicode() == 45)
		goto state_53;

	if (ch.unicode() >= 48 && ch.unicode() <= 57)
		goto state_53;

	if (ch.unicode() == 92)
		goto state_54;

	if (ch.unicode() == 95)
		goto state_53;

	if ((ch.unicode() >= 'a' && ch.unicode() <= 'z') || (ch.unicode() >= 'A' && ch.unicode() <= 'Z') || ch.unicode() >= 256)
		goto state_53;

	goto out;
state_52:
	lastAcceptingPos = pos;
	token = QCss2::TokenType2::FUNCTION;
	goto out;
state_53:
	lastAcceptingPos = pos;
	token = QCss2::TokenType2::IDENT;
	ch = next();

	if (ch.unicode() == 40)
		goto state_52;

	if (ch.unicode() == 45)
		goto state_53;

	if (ch.unicode() >= 48 && ch.unicode() <= 57)
		goto state_53;

	if (ch.unicode() == 92)
		goto state_54;

	if (ch.unicode() == 95)
		goto state_53;

	if ((ch.unicode() >= 'a' && ch.unicode() <= 'z') || (ch.unicode() >= 'A' && ch.unicode() <= 'Z') || ch.unicode() >= 256)
		goto state_53;

	goto out;
state_54:
	ch = next();

	if (ch.unicode() >= 1 && ch.unicode() <= 9)
		goto state_77;

	if (ch.unicode() == 11)
		goto state_77;

	if (ch.unicode() >= 14 && ch.unicode() <= 47)
		goto state_77;

	if (ch.unicode() >= 58 && ch.unicode() <= 96)
		goto state_77;

	if (ch.unicode() >= 103)
		goto state_77;

	goto out;
state_57:
	lastAcceptingPos = pos;
	token = QCss2::TokenType2::INVALID;
	ch = next();

	if (ch.unicode() >= 1 && ch.unicode() <= 9)
		goto state_30;

	if (ch.unicode() == 11)
		goto state_30;

	if (ch.unicode() >= 14 && ch.unicode() <= 33)
		goto state_30;

	if (ch.unicode() == 34)
		goto state_31;

	if (ch.unicode() >= 35 && ch.unicode() <= 91)
		goto state_30;

	if (ch.unicode() == 92)
		goto state_32;

	if (ch.unicode() >= 93 && ch.unicode() <= 96)
		goto state_30;

	if ((ch.unicode() >= 'a' && ch.unicode() <= 'z') || (ch.unicode() >= 'A' && ch.unicode() <= 'Z') || ch.unicode() >= 256)
		goto state_30;

	if (ch.unicode() >= 123)
		goto state_30;

	goto out;
state_58:
	lastAcceptingPos = pos;
	token = QCss2::TokenType2::INVALID;
	ch = next();

	if (ch.unicode() >= 1 && ch.unicode() <= 9)
		goto state_30;

	if (ch.unicode() == 11)
		goto state_30;

	if (ch.unicode() >= 14 && ch.unicode() <= 33)
		goto state_30;

	if (ch.unicode() == 34)
		goto state_31;

	if (ch.unicode() >= 35 && ch.unicode() <= 91)
		goto state_30;

	if (ch.unicode() == 92)
		goto state_32;

	if (ch.unicode() >= 93 && ch.unicode() <= 96)
		goto state_30;

	if ((ch.unicode() >= 'a' && ch.unicode() <= 'z') || (ch.unicode() >= 'A' && ch.unicode() <= 'Z') || ch.unicode() >= 256)
		goto state_30;

	if (ch.unicode() >= 123)
		goto state_30;

	goto out;
state_59:
	lastAcceptingPos = pos;
	token = QCss2::TokenType2::INVALID;
	ch = next();

	if (ch.unicode() >= 1 && ch.unicode() <= 9)
		goto state_30;

	if (ch.unicode() == 11)
		goto state_30;

	if (ch.unicode() >= 14 && ch.unicode() <= 33)
		goto state_30;

	if (ch.unicode() == 34)
		goto state_31;

	if (ch.unicode() >= 35 && ch.unicode() <= 91)
		goto state_30;

	if (ch.unicode() == 92)
		goto state_32;

	if (ch.unicode() >= 93 && ch.unicode() <= 96)
		goto state_30;

	if ((ch.unicode() >= 'a' && ch.unicode() <= 'z') || (ch.unicode() >= 'A' && ch.unicode() <= 'Z') || ch.unicode() >= 256)
		goto state_30;

	if (ch.unicode() >= 123)
		goto state_30;

	goto out;
state_60:
	lastAcceptingPos = pos;
	token = QCss2::TokenType2::INVALID;
	ch = next();

	if (ch.unicode() >= 1 && ch.unicode() <= 9)
		goto state_30;

	if (ch.unicode() == 10)
		goto state_78;

	if (ch.unicode() == 11)
		goto state_30;

	if (ch.unicode() >= 14 && ch.unicode() <= 33)
		goto state_30;

	if (ch.unicode() == 34)
		goto state_31;

	if (ch.unicode() >= 35 && ch.unicode() <= 91)
		goto state_30;

	if (ch.unicode() == 92)
		goto state_32;

	if (ch.unicode() >= 93 && ch.unicode() <= 96)
		goto state_30;

	if ((ch.unicode() >= 'a' && ch.unicode() <= 'z') || (ch.unicode() >= 'A' && ch.unicode() <= 'Z') || ch.unicode() >= 256)
		goto state_30;

	if (ch.unicode() >= 123)
		goto state_30;

	goto out;
state_61:
	lastAcceptingPos = pos;
	token = QCss2::TokenType2::HASH;
	ch = next();

	if (ch.unicode() == 45)
		goto state_61;

	if (ch.unicode() >= 48 && ch.unicode() <= 57)
		goto state_61;

	if (ch.unicode() == 92)
		goto state_62;

	if (ch.unicode() == 95)
		goto state_61;

	if ((ch.unicode() >= 'a' && ch.unicode() <= 'z') || (ch.unicode() >= 'A' && ch.unicode() <= 'Z') || ch.unicode() >= 256)
		goto state_61;

	goto out;
state_62:
	ch = next();

	if (ch.unicode() >= 1 && ch.unicode() <= 9)
		goto state_79;

	if (ch.unicode() == 11)
		goto state_79;

	if (ch.unicode() >= 14 && ch.unicode() <= 47)
		goto state_79;

	if (ch.unicode() >= 58 && ch.unicode() <= 96)
		goto state_79;

	if (ch.unicode() >= 103)
		goto state_79;

	goto out;
state_63:
	lastAcceptingPos = pos;
	token = QCss2::TokenType2::HASH;
	ch = next();

	if (ch.unicode() == 45)
		goto state_61;

	if (ch.unicode() >= 48 && ch.unicode() <= 57)
		goto state_61;

	if (ch.unicode() == 92)
		goto state_62;

	if (ch.unicode() == 95)
		goto state_61;

	if ((ch.unicode() >= 'a' && ch.unicode() <= 'z') || (ch.unicode() >= 'A' && ch.unicode() <= 'Z') || ch.unicode() >= 256)
		goto state_61;

	goto out;
state_64:
	lastAcceptingPos = pos;
	token = QCss2::TokenType2::INVALID;
	ch = next();

	if (ch.unicode() >= 1 && ch.unicode() <= 9)
		goto state_35;

	if (ch.unicode() == 11)
		goto state_35;

	if (ch.unicode() >= 14 && ch.unicode() <= 38)
		goto state_35;

	if (ch.unicode() == 39)
		goto state_36;

	if (ch.unicode() >= 40 && ch.unicode() <= 91)
		goto state_35;

	if (ch.unicode() == 92)
		goto state_37;

	if (ch.unicode() >= 93 && ch.unicode() <= 96)
		goto state_35;

	if ((ch.unicode() >= 'a' && ch.unicode() <= 'z') || (ch.unicode() >= 'A' && ch.unicode() <= 'Z') || ch.unicode() >= 256)
		goto state_35;

	if (ch.unicode() >= 123)
		goto state_35;

	goto out;
state_65:
	lastAcceptingPos = pos;
	token = QCss2::TokenType2::INVALID;
	ch = next();

	if (ch.unicode() >= 1 && ch.unicode() <= 9)
		goto state_35;

	if (ch.unicode() == 11)
		goto state_35;

	if (ch.unicode() >= 14 && ch.unicode() <= 38)
		goto state_35;

	if (ch.unicode() == 39)
		goto state_36;

	if (ch.unicode() >= 40 && ch.unicode() <= 91)
		goto state_35;

	if (ch.unicode() == 92)
		goto state_37;

	if (ch.unicode() >= 93 && ch.unicode() <= 96)
		goto state_35;

	if ((ch.unicode() >= 'a' && ch.unicode() <= 'z') || (ch.unicode() >= 'A' && ch.unicode() <= 'Z') || ch.unicode() >= 256)
		goto state_35;

	if (ch.unicode() >= 123)
		goto state_35;

	goto out;
state_66:
	lastAcceptingPos = pos;
	token = QCss2::TokenType2::INVALID;
	ch = next();

	if (ch.unicode() >= 1 && ch.unicode() <= 9)
		goto state_35;

	if (ch.unicode() == 11)
		goto state_35;

	if (ch.unicode() >= 14 && ch.unicode() <= 38)
		goto state_35;

	if (ch.unicode() == 39)
		goto state_36;

	if (ch.unicode() >= 40 && ch.unicode() <= 91)
		goto state_35;

	if (ch.unicode() == 92)
		goto state_37;

	if (ch.unicode() >= 93 && ch.unicode() <= 96)
		goto state_35;

	if ((ch.unicode() >= 'a' && ch.unicode() <= 'z') || (ch.unicode() >= 'A' && ch.unicode() <= 'Z') || ch.unicode() >= 256)
		goto state_35;

	if (ch.unicode() >= 123)
		goto state_35;

	goto out;
state_67:
	lastAcceptingPos = pos;
	token = QCss2::TokenType2::INVALID;
	ch = next();

	if (ch.unicode() >= 1 && ch.unicode() <= 9)
		goto state_35;

	if (ch.unicode() == 10)
		goto state_80;

	if (ch.unicode() == 11)
		goto state_35;

	if (ch.unicode() >= 14 && ch.unicode() <= 38)
		goto state_35;

	if (ch.unicode() == 39)
		goto state_36;

	if (ch.unicode() >= 40 && ch.unicode() <= 91)
		goto state_35;

	if (ch.unicode() == 92)
		goto state_37;

	if (ch.unicode() >= 93 && ch.unicode() <= 96)
		goto state_35;

	if ((ch.unicode() >= 'a' && ch.unicode() <= 'z') || (ch.unicode() >= 'A' && ch.unicode() <= 'Z') || ch.unicode() >= 256)
		goto state_35;

	if (ch.unicode() >= 123)
		goto state_35;

	goto out;
state_69:
	lastAcceptingPos = pos;
	token = QCss2::TokenType2::NUMBER;
	ch = next();

	if (ch.unicode() == 37)
		goto state_41;

	if (ch.unicode() == 45)
		goto state_42;

	if (ch.unicode() >= 48 && ch.unicode() <= 57)
		goto state_69;

	if (ch.unicode() == 92)
		goto state_45;

	if (ch.unicode() == 95)
		goto state_46;

	if ((ch.unicode() >= 'a' && ch.unicode() <= 'z') || (ch.unicode() >= 'A' && ch.unicode() <= 'Z') || ch.unicode() >= 256)
		goto state_46;

	goto out;
state_70:
	lastAcceptingPos = pos;
	token = QCss2::TokenType2::LENGTH;
	ch = next();

	if (ch.unicode() == 45)
		goto state_71;

	if (ch.unicode() >= 48 && ch.unicode() <= 57)
		goto state_71;

	if (ch.unicode() == 92)
		goto state_72;

	if (ch.unicode() == 95)
		goto state_71;

	if ((ch.unicode() >= 'a' && ch.unicode() <= 'z') || (ch.unicode() >= 'A' && ch.unicode() <= 'Z') || ch.unicode() >= 256)
		goto state_71;

	goto out;
state_71:
	lastAcceptingPos = pos;
	token = QCss2::TokenType2::LENGTH;
	ch = next();

	if (ch.unicode() == 45)
		goto state_71;

	if (ch.unicode() >= 48 && ch.unicode() <= 57)
		goto state_71;

	if (ch.unicode() == 92)
		goto state_72;

	if (ch.unicode() == 95)
		goto state_71;

	if ((ch.unicode() >= 'a' && ch.unicode() <= 'z') || (ch.unicode() >= 'A' && ch.unicode() <= 'Z') || ch.unicode() >= 256)
		goto state_71;

	goto out;
state_72:
	ch = next();

	if (ch.unicode() >= 1 && ch.unicode() <= 9)
		goto state_81;

	if (ch.unicode() == 11)
		goto state_81;

	if (ch.unicode() >= 14 && ch.unicode() <= 47)
		goto state_81;

	if (ch.unicode() >= 58 && ch.unicode() <= 96)
		goto state_81;

	if (ch.unicode() >= 103)
		goto state_81;

	goto out;
state_73:
	ch = next();

	if (ch.unicode() == 45)
	{
		token = QCss2::TokenType2::CDO;
		goto found;
	}

	goto out;
state_74:
	lastAcceptingPos = pos;
	token = QCss2::TokenType2::ATKEYWORD_SYM;
	ch = next();

	if (ch.unicode() == 45)
		goto state_75;

	if (ch.unicode() >= 48 && ch.unicode() <= 57)
		goto state_75;

	if (ch.unicode() == 92)
		goto state_76;

	if (ch.unicode() == 95)
		goto state_75;

	if ((ch.unicode() >= 'a' && ch.unicode() <= 'z') || (ch.unicode() >= 'A' && ch.unicode() <= 'Z') || ch.unicode() >= 256)
		goto state_75;

	goto out;
state_75:
	lastAcceptingPos = pos;
	token = QCss2::TokenType2::ATKEYWORD_SYM;
	ch = next();

	if (ch.unicode() == 45)
		goto state_75;

	if (ch.unicode() >= 48 && ch.unicode() <= 57)
		goto state_75;

	if (ch.unicode() == 92)
		goto state_76;

	if (ch.unicode() == 95)
		goto state_75;

	if ((ch.unicode() >= 'a' && ch.unicode() <= 'z') || (ch.unicode() >= 'A' && ch.unicode() <= 'Z') || ch.unicode() >= 256)
		goto state_75;

	goto out;
state_76:
	ch = next();

	if (ch.unicode() >= 1 && ch.unicode() <= 9)
		goto state_83;

	if (ch.unicode() == 11)
		goto state_83;

	if (ch.unicode() >= 14 && ch.unicode() <= 47)
		goto state_83;

	if (ch.unicode() >= 58 && ch.unicode() <= 96)
		goto state_83;

	if (ch.unicode() >= 103)
		goto state_83;

	goto out;
state_77:
	lastAcceptingPos = pos;
	token = QCss2::TokenType2::IDENT;
	ch = next();

	if (ch.unicode() == 40)
		goto state_52;

	if (ch.unicode() == 45)
		goto state_53;

	if (ch.unicode() >= 48 && ch.unicode() <= 57)
		goto state_53;

	if (ch.unicode() == 92)
		goto state_54;

	if (ch.unicode() == 95)
		goto state_53;

	if ((ch.unicode() >= 'a' && ch.unicode() <= 'z') || (ch.unicode() >= 'A' && ch.unicode() <= 'Z') || ch.unicode() >= 256)
		goto state_53;

	goto out;
state_78:
	lastAcceptingPos = pos;
	token = QCss2::TokenType2::INVALID;
	ch = next();

	if (ch.unicode() >= 1 && ch.unicode() <= 9)
		goto state_30;

	if (ch.unicode() == 11)
		goto state_30;

	if (ch.unicode() >= 14 && ch.unicode() <= 33)
		goto state_30;

	if (ch.unicode() == 34)
		goto state_31;

	if (ch.unicode() >= 35 && ch.unicode() <= 91)
		goto state_30;

	if (ch.unicode() == 92)
		goto state_32;

	if (ch.unicode() >= 93 && ch.unicode() <= 96)
		goto state_30;

	if ((ch.unicode() >= 'a' && ch.unicode() <= 'z') || (ch.unicode() >= 'A' && ch.unicode() <= 'Z') || ch.unicode() >= 256)
		goto state_30;

	if (ch.unicode() >= 123)
		goto state_30;

	goto out;
state_79:
	lastAcceptingPos = pos;
	token = QCss2::TokenType2::HASH;
	ch = next();

	if (ch.unicode() == 45)
		goto state_61;

	if (ch.unicode() >= 48 && ch.unicode() <= 57)
		goto state_61;

	if (ch.unicode() == 92)
		goto state_62;

	if (ch.unicode() == 95)
		goto state_61;

	if ((ch.unicode() >= 'a' && ch.unicode() <= 'z') || (ch.unicode() >= 'A' && ch.unicode() <= 'Z') || ch.unicode() >= 256)
		goto state_61;

	goto out;
state_80:
	lastAcceptingPos = pos;
	token = QCss2::TokenType2::INVALID;
	ch = next();

	if (ch.unicode() >= 1 && ch.unicode() <= 9)
		goto state_35;

	if (ch.unicode() == 11)
		goto state_35;

	if (ch.unicode() >= 14 && ch.unicode() <= 38)
		goto state_35;

	if (ch.unicode() == 39)
		goto state_36;

	if (ch.unicode() >= 40 && ch.unicode() <= 91)
		goto state_35;

	if (ch.unicode() == 92)
		goto state_37;

	if (ch.unicode() >= 93 && ch.unicode() <= 96)
		goto state_35;

	if ((ch.unicode() >= 'a' && ch.unicode() <= 'z') || (ch.unicode() >= 'A' && ch.unicode() <= 'Z') || ch.unicode() >= 256)
		goto state_35;

	if (ch.unicode() >= 123)
		goto state_35;

	goto out;
state_81:
	lastAcceptingPos = pos;
	token = QCss2::TokenType2::LENGTH;
	ch = next();

	if (ch.unicode() == 45)
		goto state_71;

	if (ch.unicode() >= 48 && ch.unicode() <= 57)
		goto state_71;

	if (ch.unicode() == 92)
		goto state_72;

	if (ch.unicode() == 95)
		goto state_71;

	if ((ch.unicode() >= 'a' && ch.unicode() <= 'z') || (ch.unicode() >= 'A' && ch.unicode() <= 'Z') || ch.unicode() >= 256)
		goto state_71;

	goto out;
state_83:
	lastAcceptingPos = pos;
	token = QCss2::TokenType2::ATKEYWORD_SYM;
	ch = next();

	if (ch.unicode() == 45)
		goto state_75;

	if (ch.unicode() >= 48 && ch.unicode() <= 57)
		goto state_75;

	if (ch.unicode() == 92)
		goto state_76;

	if (ch.unicode() == 95)
		goto state_75;

	if ((ch.unicode() >= 'a' && ch.unicode() <= 'z') || (ch.unicode() >= 'A' && ch.unicode() <= 'Z') || ch.unicode() >= 256)
		goto state_75;

	goto out;
found:
	lastAcceptingPos = pos;
out:

	if (lastAcceptingPos != -1)
	{
		lexemLength = lastAcceptingPos - lexemStart;
		pos = lastAcceptingPos;
	}

	return token;
}


QString Scanner2::preprocess(const QString& input, bool* hasEscapeSequences)
{
	QString output = input;

	if (hasEscapeSequences)
		*hasEscapeSequences = false;

	int i = 0;

	while (i < output.size())
	{
		if (output.at(i) == QLatin1Char('\\'))
		{
			++i;
			// test for unicode hex escape
			int hexCount = 0;
			const int hexStart = i;

			while (i < output.size()
					&& isHexDigit2(output.at(i).toLatin1())
					&& hexCount < 7)
			{
				++hexCount;
				++i;
			}

			if (hexCount == 0)
			{
				if (hasEscapeSequences)
					*hasEscapeSequences = true;

				continue;
			}

			hexCount = qMin(hexCount, 6);
			bool ok = false;
			ushort code = output.mid(hexStart, hexCount).toUShort(&ok, 16);

			if (ok)
			{
				output.replace(hexStart - 1, hexCount + 1, QChar(code));
				i = hexStart;
			}
			else
			{
				i = hexStart;
			}
		}
		else
		{
			++i;
		}
	}

	return output;
}

void Scanner2::scan(const QString& preprocessedInput, QVector<Symbol2>* symbols)
{
	QCssScanner scanner(preprocessedInput);
	Symbol2 sym;
	int tok = scanner.lex();

	if (symbols != nullptr)
	{
		while (tok != -1)
		{
			sym.token = static_cast<QCss2::TokenType2>(tok);
			sym.text = scanner.input;
			sym.start = scanner.lexemStart;
			sym.len = scanner.lexemLength;
			symbols->append(sym);
			tok = scanner.lex();
		}
	}
}
}