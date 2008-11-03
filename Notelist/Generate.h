//////////////////////////////////////////////////////////////////////
// Generator classes.
//
// Copyright 2008, Daniel R. Mitchell
// License: Creative Commons/GNU-GPL 
// (http://creativecommons.org/licenses/GPL/2.0/)
// (http://www.gnu.org/licenses/gpl.html)
//////////////////////////////////////////////////////////////////////

#if !defined(_GENERATE_H_)
#define _GENERATE_H_

#pragma once

// nlFunctionData holds values for a built-in function
class nlFunctionData
{
public:
	double beginVal;
	double endVal;
	double range;
	double offset;
	double count;
	double curVal;
	int    fnType;
	int    pos;

// For LINE:
// y = a * x + c
// For EXP or LOG:
// y = a * x^b + c
// x = n * 1/d
// b = flatness of curve, exp or log
// For both:
// a = end - start
// c = start

	nlFunctionData()
	{
		beginVal = 0;
		endVal = 1;
		range = 1.0;
		offset = 0.0;
		curVal = 0.0;
		count = 0.0;
		fnType = T_LINE;
		pos = 1;
	}

	int IsActive()
	{
		return curVal < count;
	}

	double Iterate(double dDur);

	void Init(double dFrom, double dTo, double dIn, int nFn = T_LINE)
	{
		fnType = nFn;
		beginVal = dFrom;
		endVal = dTo;
		range = dTo - dFrom;
		offset = dFrom;
		count = dIn;
		curVal = 0;
		pos = (dTo >= dFrom) ? 1 : 0;
	}
};

enum vType
{
	vtNull = 0,
	vtText,
	vtNum,
	vtReal
};

// variant value class - can store string, integer or real
// and provides automatic type conversion.
class nlVarValue
{
public:
	vType  vt;
	union 
	{
		char *txtVal;
		long  lVal;
		double dblVal;
	};

	nlVarValue()
	{
		vt = vtNull;
		txtVal = 0;
	}
	virtual ~nlVarValue()
	{
		ClearValue();
	}
	virtual void ClearValue();
	virtual void ChangeType(vType vnew);
	virtual void SetValue(const char *p);
	virtual void SetValue(long n);
	virtual void SetValue(double d);
	virtual void SetValue(nlVarValue *v);
	virtual void GetValue(char **p);
	virtual void GetValue(long *n);
	virtual void GetValue(double *d);
	virtual void CopyValue(nlVarValue *v);
	virtual int  Compare(nlVarValue *v);
};

class nlSymbol : public nlVarValue
{
public:
	char *name;
	nlSymbol *next;

	nlSymbol()
	{
		name = NULL;
	}
	nlSymbol(const char *n)
	{
		if (n)
			name = StrMakeCopy(n);
		else
			name = NULL;
	}
};

struct nlSyncMark
{
	nlVarValue id;
	double t;
	nlSyncMark *next;
};

class nlNoteData
{
public:
	long alloc;
	long count;
	long index;
	int simul;
	nlVarValue *values;

	nlNoteData()
	{
		alloc = 0;
		count = 0;
		index = 0;
		simul = 0;
		values = NULL;
	}

	virtual ~nlNoteData()
	{
		for (long n = 0; n < alloc; n++)
			values[n].ClearValue();

		delete[] values;
	}

	void InitSingle(long n)
	{
		simul = 0;
		count = 1;
		index = 0;
		if (alloc == 0)
		{
			values = new nlVarValue[1];
			alloc = 1;
		}
		values->SetValue(n);
	}

	void InitSingle(double n)
	{
		simul = 0;
		count = 1;
		index = 0;
		if (alloc == 0)
		{
			values = new nlVarValue[1];
			alloc = 1;
		}
		values->SetValue(n);
	}

	nlScriptNode *Exec(nlScriptNode *p);
	int GetNextValue(double *d);
	int GetNextValue(long *n);
};

class nlSequence
{
	nlVarValue id;
	nlScriptNode *head;
	nlScriptNode *tail;
	nlSequence *next;

	void Init();

public:
	nlSequence();
	nlSequence(const char *name);
	nlSequence(int n);
	~nlSequence();
	void Play();
	void Append(nlSequence **list);
	nlScriptNode *AddNode(nlScriptNode *seq);
	nlSequence *FindSequence(nlVarValue *find);
};

class nlVoice
{
public:
	nlVoice();
	virtual ~nlVoice();

	nlVoice *next;
	nlGenerate *genPtr;
	int    voiceNum;
	double curTime;
	double volMul;
	double lastPit;
	double lastDur;
	double lastVol;
	double lastArtic;
	double *lastParam;
	double *paramVal;
	long   maxParam;
	long   cntParam;

	long   instr;
	long   chnl;
	long   articType;
	long   articParam;
	long   transpose;
	long   doublex;
	double doublev;
	long   loopCount;
	char   *instname;

	nlNoteData pitch;
	nlNoteData duration;
	nlNoteData volume;
	nlNoteData artic;
	nlNoteData *params;

	void ClearLast()
	{
		for (long n = 0; n < maxParam; n++)
		{
			lastParam[n] = 0.0;
			paramVal[n] = 0.0;
		}
	}

	void SetMaxParam(long n)
	{
		if (n < 0)
			return;
		if (params)
			delete params;
		if (lastParam)
			delete lastParam;
		if (paramVal)
			delete paramVal;
		params = new nlNoteData[n+1];
		lastParam = new double[n+1];
		paramVal = new double[n+1];
		maxParam = n;
		ClearLast();
	}

	void SetInstName(const char *newname)
	{
		if (instname)
			delete instname;
		instname = StrMakeCopy(newname);
	}
};

class nlScriptNode : public nlVarValue
{
protected:
	int token;
	nlScriptNode *next;
	nlGenerate *genPtr;

public:
	nlScriptNode()
	{
		token = -1;
		next = NULL;
		genPtr = NULL;
	}
	virtual ~nlScriptNode()
	{
	}

	virtual void SetGen(nlGenerate *p)
	{ 
		genPtr = p; 
	}

	virtual nlScriptNode *Exec()
	{
		return next;
	}

	void Append(nlScriptNode *p)
	{
		p->next = this;
	}

	inline void SetToken(int n)
	{
		token = n;
	}

	inline int GetToken()
	{
		return token;
	}

	inline nlScriptNode *GetNext()
	{
		return next;
	}
};

class nlVoiceNode : public nlScriptNode
{
public:
	nlVoiceNode()
	{
		token = T_VOICE;
	}

	virtual nlScriptNode *Exec();
};

class nlBlockNode : public nlScriptNode
{
public:
	nlBlockNode()
	{
		token = T_BEGIN;
	}
	virtual nlScriptNode *Exec();
};

class nlTempoNode : public nlScriptNode
{
public:
	nlTempoNode()
	{
		token = T_TEMPO;
	}

	virtual nlScriptNode *Exec();
};

class nlTimeNode : public nlScriptNode
{
public:
	nlTimeNode()
	{
		token = T_TIME;
	}
	virtual nlScriptNode *Exec();
};

class nlMarkNode : public nlScriptNode
{
public:
	nlMarkNode()
	{
		token = T_MARK;
	}
	virtual nlScriptNode *Exec();
};

class nlSyncNode : public nlScriptNode
{
public:
	nlSyncNode()
	{
		token = T_SYNC;
	}
	virtual nlScriptNode *Exec();
};

class nlInstnumNode : public nlScriptNode
{
public:
	nlInstnumNode()
	{
		token = T_INSTNUM;
	}
	virtual nlScriptNode *Exec();
};

class nlChnlNode : public nlScriptNode
{
public:
	nlChnlNode()
	{
		token = T_CHNL;
	}
	virtual nlScriptNode *Exec();
};


class nlNoteNode : public nlScriptNode
{
private:
	int    sus;
	int    add;

public:
	nlNoteNode()
	{
		sus = 0;
		add = 0;
		token = T_NOTE;
	}
	void SetSus(int n)
	{
		sus = n;
	}
	void SetAdd(int n)
	{
		add = n;
	}

	virtual nlScriptNode *Exec();
};

class nlExprNode : public nlScriptNode
{
public:
	nlExprNode()
	{
		token = T_EXPR;
	}

	virtual nlScriptNode *Exec();
};

class nlDurNode : public nlScriptNode
{
private:
	int isDotted;
public:
	nlDurNode()
	{
		isDotted = 0;
		token = T_DUR;
	}
	void SetDotted(int d) { isDotted = d; }
	virtual void CopyValue(nlVarValue *p);
	virtual void GetValue(long *n);
	virtual void GetValue(double *d);
	void GetBeatValue(double *n)
	{
		nlVarValue::GetValue(n);
	}
};

class nlIfNode : public nlScriptNode
{
private:
	nlSequence *ifPart;
	nlSequence *elPart;
public:
	nlIfNode()
	{
		token = T_IF;
		ifPart = NULL;
		elPart = NULL;
	}
	virtual ~nlIfNode();
	void SetIfSequence(nlSequence *p)
	{
		ifPart = p;
	}
	void SetElseSequence(nlSequence *p)
	{
		elPart = p;
	}
	virtual nlScriptNode* Exec();
};

class nlWhileNode : public nlScriptNode
{
private:
	nlSequence *wseq;
public:
	nlWhileNode()
	{
		token = T_WHILE;
		wseq = NULL;
	}
	virtual ~nlWhileNode();
	void SetSequence(nlSequence *p)
	{
		wseq = p;
	}

	virtual nlScriptNode* Exec();
};

class nlLoopNode : public nlScriptNode
{
private:
	nlSequence *lseq;
public:
	nlLoopNode()
	{
		lseq = NULL;
		token = T_LOOP;
	}
	virtual ~nlLoopNode();
	void SetSequence(nlSequence *p)
	{
		lseq = p;
	}
	virtual nlScriptNode *Exec();
};

class nlWriteNode: public nlScriptNode
{
public:
	nlWriteNode()
	{
		token = T_WRITE;
	}
	virtual nlScriptNode *Exec();
};

class nlVolumeNode : public nlScriptNode
{
public:
	nlVolumeNode()
	{
		token = T_VOL;
	}
	virtual nlScriptNode *Exec();
};

class nlTransposeNode : public nlScriptNode
{
public:
	nlTransposeNode()
	{
		token = T_XPOSE;
	}
	virtual nlScriptNode *Exec();
};

class nlDoubleNode : public nlScriptNode
{
public:
	nlDoubleNode()
	{
		token = T_DOUBLE;
	}
	virtual nlScriptNode *Exec();
};

class nlPlayNode : public nlScriptNode
{
public:
	nlPlayNode()
	{
		token = T_PLAY;
	}
	virtual nlScriptNode *Exec();
};

class nlInitFnNode : public nlScriptNode
{
public:
	nlInitFnNode()
	{
		token = T_INIT;
	}
	virtual nlScriptNode *Exec();
};

class nlArticNode : public nlScriptNode
{
public:
	nlArticNode()
	{
		token = T_ART;
	}
	virtual nlScriptNode *Exec();
};

class nlParamNode : public nlScriptNode
{
public:
	nlParamNode()
	{
		token = T_PARAM;
	}
	virtual nlScriptNode *Exec();
};

class nlMapNode : public nlScriptNode
{
public:
	nlMapNode()
	{
		token = T_MAP;
	}
	virtual nlScriptNode *Exec();
};

class nlMaxParamNode : public nlScriptNode
{
public:
	nlMaxParamNode()
	{
		token = T_MAXPARAM;
	}
	virtual nlScriptNode *Exec();
};

class nlCallNode : public nlScriptNode
{
public:
	nlCallNode()
	{
		token = T_CALL;
	}
	virtual nlScriptNode *Exec();
};


class nlSetNode : public nlScriptNode
{
private:
	nlSymbol *symb;
public:
	nlSetNode()
	{
		token = T_SET;
		symb = NULL;
	}

	void SetSymbol(nlSymbol *s)
	{
		symb = s;
	}

	virtual nlScriptNode *Exec();
};

class nlVarNode : public nlScriptNode
{
private:
	nlSymbol *symb;
public:
	nlVarNode()
	{
		token = T_VAR;
		symb = NULL;
	}

	void SetSymbol(nlSymbol *s)
	{
		symb = s;
	}

	virtual nlScriptNode *Exec();
};

class nlOptNode : public nlScriptNode
{
public:
	nlOptNode()
	{
		token = T_OPTION;
	}
	virtual nlScriptNode *Exec();
};

class nlGenerate  
{
private:
	nlSequence *curSeq;
	nlConverter *cvtPtr;
	nlVoice *curVoice;
	nlVoice *voiceList;
	nlSequence *mainSeq;

	nlVarValue *vStack;
	nlVarValue *spEnd;
	nlVarValue *spCur;
	nlSyncMark *synclist;

	long maxParam;
	long freqmode;
	long voldbmode;
	double beat;
	double secBeat;
	double nlVersion;

	void Clear();

public:
	nlGenerate();
	virtual ~nlGenerate();


	void SetConverter(nlConverter *p) {cvtPtr = p; }
	nlConverter *GetConverter() { return cvtPtr; }
	nlVoice *GetCurVoice() { return curVoice; }
	nlVoice *SetCurVoice(int n);

	void SyncTo(nlVarValue *v);
	void MarkTo(nlVarValue *v);

	void SetMaxParam(long n) { maxParam = n; }
	long GetMaxParam() { return maxParam; }
	void SetVersion(double v) { nlVersion = v; }
	double GetVersion() { return nlVersion; }

	void SetFrequencyMode(long n) { freqmode = n; }
	long GetFrequencyMode() { return freqmode; }
	void SetVoldbMode(long n) { voldbmode = n; }
	long GetVoldbMode() { return voldbmode; }

	void SetTempo(double b, double t)
	{
		beat = b;
		secBeat = t;
	}

	double ConvertRhythm(double d)
	{
		return beat / d * secBeat;
	}

	nlFunctionData *iFnGen[MAXFGEN];

	nlSequence *FindSequence(nlVarValue *id);
	void Reset();
	int Run();
	void InitStack();
	void PushStack(long n);
	void PushStack(double d);
	void PushStack(const char *s);
	void PushStack(nlVarValue *v);
	void PopStack(long *n);
	void PopStack(double *d);
	void PopStack(char **s);
	void PopStack(nlVarValue *v);
	nlSequence *AddSequence(const char *id);
	nlSequence *AddSequence(int id);
	nlSequence *SetCurSeq(nlSequence *p);
	nlScriptNode *AddNode(nlScriptNode *pn);
	nlScriptNode *AddNode(int token, const char *text);
	nlScriptNode *AddNode(int token, long val);
	nlScriptNode *AddNode(int token, short v1, short v2);
	nlScriptNode *AddNode(int token, double val);
};

#endif // !defined(_GENERATE_H_)
