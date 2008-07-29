#define NOTELIST_VERSION 3.0

// generator tokens

#define	T_ERROR (-1)    // end of input file
#define T_ENDOF 0
#define	T_START	1		// first instruction
#define	T_BIF   2		// built in function - runtime

#define T_ENDSTMT ';'
#define T_OPAREN  '('
#define T_CPAREN  ')'
#define T_OBRACE  '{'
#define T_CBRACE  '}'
#define T_OBRACK  '['
#define T_CBRACK  ']'
#define T_EQ      '='
#define T_MULOP   '*'
#define T_DIVOP   '/'
#define T_ADDOP   '+'
#define T_SUBOP   '-'
#define T_COMMA   ','
#define T_STRLIT  '"'
#define T_QUERY   '?'
#define T_EXPOP   '^'
#define T_CATOP   '&'

#define T_PIT   301
#define T_DUR   302
#define T_NUM   303
#define T_VOICE 304
#define T_TIME  305
#define T_TEMPO 306
#define T_BEGIN 307
#define T_END   308
#define T_LOOP  309
#define T_REP   310 // NA
#define T_CRESC 311 // NA
#define T_DIM   312 // NA
#define T_ACCEL 313 // NA
#define T_RIT   314 // NA
#define T_WRITE 315
#define T_INSTNUM 316
#define T_NOTE  317
#define T_EXPR  318
#define T_SUS   319
#define T_TIE   320
#define T_FROM  321
#define T_TO    322
#define T_IN    323
#define T_VOL   324
#define T_XPOSE 325
#define T_NEG   326
#define T_SEQ   327
#define T_PLAY  328
#define T_VALS  329
#define T_INC   330
#define T_INIT  331
#define T_FGEN  332
#define T_LINE  333
#define T_EXP   334
#define T_RAND  335
#define T_LOG   336
#define T_ART   337
#define T_PCNT  338
#define T_FIXED 339
#define T_ADD   340
#define T_MIX   341
#define T_PATCH 351
#define T_WVTBL 352
#define T_CALL  353
#define T_MIDC  354
#define T_CHNL  355
#define T_VER   356
#define T_ON    357
#define T_OFF   358
#define T_PARAM 359
#define T_MARK  360
#define T_SYNC  361
#define T_TABLE 362
#define T_LOOKUP 363
#define T_COUNT 364
#define T_MAP 365
#define T_MAXPARAM 366
#define T_LTOP    367
#define T_LEOP    368
#define T_GTOP    369
#define T_GEOP    370
#define T_NEOP    371
#define T_EQOP    372

#define MAXPARAM 10
#define MAXFGEN  10

class nlLex;
class nlGenerate;
class nlConverter;
class nlVoice;
class nlInstr;

#ifdef WIN32
#define _CRT_SECURE_NO_WARNINGS
#define WINDOWS_LEAN_AND_MEAN
//#include <Windows.h>
#endif

#include <stdlib.h>
#include <ctype.h>
#include <math.h>
#include <SynthDefs.h>
#include <SynthString.h>
#include <Mixer.h>
#include <WaveFile.h>
#include <SynthList.h>
#include <XmlWrap.h>
#include <SeqEvent.h>
#include <Instrument.h>
#include <Sequencer.h>


extern int CompareToken(const char *s1, const char *s2);
extern char *StrMakeCopy(const char *);
extern char *StrPaste(const char *s1, const char *s2);
extern char *IntToStr(long val, char *s);
extern char *FltToStr(double val, char *s, int len);
#include "Lex.h"
#include "Generate.h"
#include "Parser.h"
#ifdef NL_INCLUDE_JS
#if WIN32
#define XP_WIN
#endif
#include <jsapi.h>
#endif
#include "Converter.h"

