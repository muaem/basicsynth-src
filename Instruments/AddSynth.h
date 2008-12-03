//////////////////////////////////////////////////////////////////////
// BasicSynth Additive Synthesis instrument
//
// Copyright 2008, Daniel R. Mitchell
// License: Creative Commons/GNU-GPL 
// (http://creativecommons.org/licenses/GPL/2.0/)
// (http://www.gnu.org/licenses/gpl.html)
//////////////////////////////////////////////////////////////////////

#if !defined(_ADDSYNTH_H_)
#define _ADDSYNTH_H_

#include "LFO.h"

struct AddSynthPart
{
#ifdef USE_OSCILI
	GenWaveI osc; //<-- if you prefer interpolation
#else
	GenWaveWT osc;
#endif
	EnvGenSegSus env;
	FrqValue  mul; // frequency multiplier

	void Copy(AddSynthPart *p)
	{
		mul = p->mul;
		osc.SetFrequency(p->osc.GetFrequency());
		osc.SetWavetable(p->osc.GetWavetable());
		env.Copy(&p->env);
	}
};

class AddSynth : public Instrument
{
private:
	int chnl;
	AmpValue vol;
	FrqValue frq;
	AddSynthPart *parts;
	int numParts;
	LFO lfoGen;

	InstrManager *im;

	void UpdateParams(SeqEvent *evt, float initPhs);

public:
	AddSynth();
	AddSynth(AddSynth *tp);
	virtual ~AddSynth();

	static Instrument *AddSynthFactory(InstrManager *, Opaque tmplt);
	static SeqEvent   *AddSynthEventFactory(Opaque tmplt);
	static bsInt16     MapParamID(const char *name);

	void Copy(AddSynth *tp);
	virtual void Start(SeqEvent *evt);
	virtual void Param(SeqEvent *evt);
	virtual void Stop();
	virtual void Tick();
	virtual int  IsFinished();
	virtual void Destroy();

	int Load(XmlSynthElem *parent);
	int Save(XmlSynthElem *parent);
	int GetParams(VarParamEvent *params);
	int SetParams(VarParamEvent *params);

	int SetNumParts(int n);
	int GetNumParts();
	AddSynthPart *GetPart(int n);
};

#endif
