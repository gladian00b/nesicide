#include "qscilexerca65.h"

#include <QColor>
#include <QFont>
#include <QSettings>

#include "cnes6502.h"
#include "emulator_core.h"
#include "ccc65interface.h"

// Styles
enum
{
   CA65_Default,
   CA65_Comment,
   CA65_Opcode,
   CA65_Label,
   CA65_Keyword,
   CA65_QuotedString,
   CA65_HighlightedLine
};

static const char* CA65_keyword[] =
{
   "addr",
   "align",
   "asciiz",
   "assert",
   "autoimport",
   "bankbytes",
   "bss",
   "byt",
   "byte",
   "case",
   "charmap",
   "code",
   "condes",
   "constructor",
   "data",
   "dbyt",
   "debuginfo",
   "define",
   "def",
   "defined",
   "destructor",
   "dword",
   "else",
   "elseif",
   "end",
   "endenum",
   "endif",
   "endmac",
   "endmacro",
   "endproc",
   "endrep",
   "endrepeat",
   "endscope",
   "endstruct",
   "enum",
   "error",
   "exitmac",
   "exitmacro",
   "export",
   "exportzp",
   "feature",
   "fileopt",
   "fopt",
   "forceimport",
   "global",
   "globalzp",
   "hibytes",
   "if",
   "ifblank",
   "ifconst",
   "ifdef",
   "ifnblank",
   "ifndef",
   "ifnref",
   "ifref",
   "import",
   "importzp",
   "incbin",
   "include",
   "interruptor",
   "linecont",
   "list",
   "listbytes",
   "lobytes",
   "local",
   "localchar",
   "macpack",
   "mac",
   "macro",
   "org",
   "out",
   "p02",
   "pagelen",
   "pagelength",
   "popseg",
   "proc",
   "pushseg",
   "reloc",
   "repeat",
   "res",
   "rodata",
   "scope",
   "segment",
   "setcpu",
   "struct",
   "tag",
   "warning",
   "word",
   "zeropage",
   NULL
};

QsciLexerCA65::QsciLexerCA65(QObject *parent)
{
   QString regex;
   int rc;

   regex = "\\b(";
   for ( rc = 0; rc < 256; rc++ )
   {
      regex += OPCODENAME(rc);
      if ( rc < 255 )
      {
         regex += '|';
      }
   }
   regex += ")\\b";
   opcodeRegex.setPattern(regex);
   opcodeRegex.setCaseSensitivity(Qt::CaseInsensitive);

   regex = "\\b(";
   rc = 0;
   while ( CA65_keyword[rc] )
   {
      regex += CA65_keyword[rc];
      if ( CA65_keyword[rc+1] )
      {
         regex += '|';
      }
      rc++;
   }
   regex += ")\\b";
   keywordRegex.setPattern(regex);
   keywordRegex.setCaseSensitivity(Qt::CaseInsensitive);
}

QsciLexerCA65::~QsciLexerCA65()
{
}

QString QsciLexerCA65::description(int style) const
{
   // Note: this function MUST return a non-empty string
   //       for a style otherwise that style is ignored!
   switch ( style )
   {
      case CA65_Default:
         return "Default";
      break;
      case CA65_Comment:
         return "Comment";
      break;
      case CA65_Opcode:
         return "Opcode";
      break;
      case CA65_Label:
         return "Label";
      break;
      case CA65_Keyword:
         return "Keyword";
      break;
      case CA65_QuotedString:
         return "Quoted String";
      break;
      case CA65_HighlightedLine:
         return "Highlighted Line";
      break;
      default:
         return QString();
      break;
   }
}

#include "main.h"

void QsciLexerCA65::styleText(int start, int end)
{
   QByteArray   chars;
   QString      text;
   QStringList  lines;
   QRegExp      labelRegex("([^; \t]+):");
   int          pos;
   int          lineLength;
   int          lineNum;
   int          lineNumToHilight;
   int          index;
   unsigned int addr;
   unsigned int absAddr;

   // Reset line styling.
   startStyling(start,0x7F);
   setStyling(end-start,CA65_Default);

   // Get NES info for line highlighting.
   addr = nesGetCPUProgramCounterOfLastSync();
   absAddr = nesGetAbsoluteAddressFromAddress(addr);
   lineNumToHilight = CCC65Interface::getSourceLineFromAbsoluteAddress(addr,absAddr);

   // Get the text that is being styled.
   chars.reserve((end-start)+1);
   editor()->SendScintilla(QsciScintilla::SCI_GETTEXTRANGE,start,end,chars.data());

   // Get the starting line index from the position.
   editor()->lineIndexFromPosition(start,&lineNum,&index);

   // Break the text into line chunks.
   text = chars.constData();
   lines = text.split(QRegExp("(\r\n|\n)"));

   // Style the lines.
   foreach ( const QString& line, lines )
   {
      // Get current line length.
      lineLength = editor()->lineLength(lineNum);

      if ( lineLength )
      {
         // Look for opcodes.
         pos = line.indexOf(opcodeRegex);
         if ( pos != -1 )
         {
            startStyling(start+pos,0x7F);
            setStyling(3,CA65_Opcode);
         }

         // Look for labels.
         pos = 0;
         do
         {
            pos = line.indexOf(labelRegex,pos);
            if ( pos != -1 )
            {
               startStyling(start+pos,0x7F);
               setStyling(labelRegex.matchedLength()-1,CA65_Label);
               pos = pos+labelRegex.matchedLength()+1;
            }
         } while ( pos != -1 );

         // Look for quoted strings.
         pos = 0;
         do
         {
            pos = line.indexOf(QRegExp("\\\".*\\\""),pos);
            if ( pos != -1 )
            {
               startStyling(start+pos,0x7F);
               setStyling(line.indexOf('\"',pos+1)-pos+1,CA65_QuotedString);
               pos = line.indexOf('\"',pos+1)+1;
            }
         } while ( pos != -1 );
         pos = 0;
         do
         {
            pos = line.indexOf(QRegExp("\\\'.*\\\'"),pos);
            if ( pos != -1 )
            {
               startStyling(start+pos,0x7F);
               setStyling(line.indexOf('\'',pos+1)-pos+1,CA65_QuotedString);
               pos = line.indexOf('\'',pos+1)+1;
            }
         } while ( pos != -1 );

         // Look for keywords.
         pos = line.indexOf(keywordRegex);
         if ( pos != -1 )
         {
            startStyling(start+pos,0x7F);
            setStyling(keywordRegex.matchedLength(),CA65_Keyword);
         }

         // Look for comments. (Wash styling of comment over any previously applied).
         pos = line.indexOf(';');
         if ( pos != -1 )
         {
            startStyling(start+pos,0x7F);
            setStyling(lineLength-pos,CA65_Comment);
         }

         // Do we need to hilight this line?
         if ( lineNumToHilight == lineNum+1 )
         {
            startStyling(start,0x7F);
            setStyling(lineLength,CA65_HighlightedLine);
         }
      }

      start += lineLength;
      lineNum++;
   }
}

const char* QsciLexerCA65::wordCharacters() const
{
   const char* chars = QsciLexerCustom::wordCharacters();
   return chars;
}

QColor QsciLexerCA65::defaultColor() const
{
   return QColor(0,0,0);
}

QColor QsciLexerCA65::defaultColor(int style) const
{
   switch ( style )
   {
      case CA65_Default:
         return QColor(50,50,50);
      break;
      case CA65_Comment:
         return QColor(0,255,45);
      break;
      case CA65_Opcode:
         return QColor(0,45,255);
      break;
      case CA65_Label:
         return QColor(0,160,30);
      break;
      case CA65_Keyword:
         return QColor(120,120,120);
      break;
      case CA65_QuotedString:
         return QColor(255,120,30);
      break;
      case CA65_HighlightedLine:
         return QColor(0,0,0);
      break;
      default:
         return QColor(0,0,0);
      break;
   }
}

QColor QsciLexerCA65::color(int style) const
{
   return QsciLexerCA65::defaultColor(style);
}

QColor QsciLexerCA65::defaultPaper() const
{
   return QColor(255,255,255);
}

QColor QsciLexerCA65::defaultPaper(int style) const
{
   switch ( style )
   {
      case CA65_Default:
         return QColor(255,255,255);
      break;
      case CA65_Comment:
         return QColor(255,255,255);
      break;
      case CA65_Opcode:
         return QColor(255,255,255);
      break;
      case CA65_Label:
         return QColor(255,255,255);
      break;
      case CA65_Keyword:
         return QColor(255,255,255);
      break;
      case CA65_QuotedString:
         return QColor(255,255,255);
      break;
      case CA65_HighlightedLine:
         return QColor(230,230,230);
      break;
      default:
         return QColor(255,255,255);
      break;
   }
}

QColor QsciLexerCA65::paper(int style) const
{
   return QsciLexerCA65::defaultPaper(style);
}

QFont QsciLexerCA65::defaultFont() const
{
#ifdef Q_WS_MAC
   return QFont("Monaco", 11);
#endif
#ifdef Q_WS_X11
   return QFont("Monospace", 10);
#endif
#ifdef Q_WS_WIN
   return QFont("Consolas", 11);
#endif
}

QFont QsciLexerCA65::defaultFont(int style) const
{
   QFont font = QsciLexerCA65::defaultFont();

   switch ( style )
   {
      case CA65_Comment:
         font.setItalic(true);
      break;
      case CA65_Opcode:
         font.setBold(true);
      break;
      case CA65_Label:
         font.setUnderline(true);
      break;
      case CA65_Keyword:
         font.setBold(true);
         font.setItalic(true);
      break;
      case CA65_HighlightedLine:
         font.setWeight(100);
      break;
   }

   return font;
}

QFont QsciLexerCA65::font(int style) const
{
   return QsciLexerCA65::defaultFont(style);
}