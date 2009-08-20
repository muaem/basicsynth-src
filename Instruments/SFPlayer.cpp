//////////////////////////////////////////////////////////////////////
/// @file SFPlayer.cpp Implementation of SoundFont(R) playback for BasicSynth
//
// Copyright 2009, Daniel R. Mitchell
// License: Creative Commons/GNU-GPL 
// (http://creativecommons.org/licenses/GPL/2.0/)
// (http://www.gnu.org/licenses/gpl.html)
//////////////////////////////////////////////////////////////////////

#include "Includes.h"
#include "SFPlayer.h"

/// Create an instance of the SFPlayer instrument
Instrument *SFPlayerInstr::SFPlayerInstrFactory(InstrManager *m, Opaque tmplt)
{
	SFPlayerInstr *ip;
	if (tmplt)
		ip = new SFPlayerInstr((SFPlayerInstr*)tmplt);
	else
		ip = new SFPlayerInstr;
	ip->im = m;
	return ip;
}

/// Create an event object for SFPlayerInstr
SeqEvent *SFPlayerInstr::SFPlayerEventFactory(Opaque tmplt)
{
	VarParamEvent *ep = new VarParamEvent;
	ep->maxParam = 61;
	return (SeqEvent*)ep;
}

/// Allocate a variable parameters object (needed for editor)
VarParamEvent *SFPlayerInstr::AllocParams()
{
	return (VarParamEvent*) SFPlayerEventFactory(0);
}

/// Constructor - no template
SFPlayerInstr::SFPlayerInstr()
{
	chnl = 0;
	pitch = 57;
	monoSet = 1;
	vol = 1.0;
	frq = 440.0;

	sndbnk = 0;
	preset = 0;
	bnkNum = -1;
	preNum = -1;
	xfade = 0;

	// default 'boxcar' envelope
	volEnv.SetAtkRt(0.0);
	volEnv.SetAtkLvl(1.0);
	volEnv.SetDecRt(0.0);
	volEnv.SetSusLvl(1.0);
	volEnv.SetRelRt(0.01);

	modEnv.SetAtkRt(0.0);
	modEnv.SetAtkLvl(0.0);
	modEnv.SetDecRt(0.0);
	modEnv.SetSusLvl(0.0);
	modEnv.SetRelRt(0.0);

	fmCM = 0.0;
	fmIM = 0.0;

	osc1 = 0;
	osc2 = 0;
	zone = 0;
	zone2 = 0;
	im = 0;
}

/// Constructor - with template
SFPlayerInstr::SFPlayerInstr(SFPlayerInstr *tp)
{
	osc1 = 0;
	osc2 = 0;
	zone = 0;
	zone2 = 0;
	im = 0;
	xfade = 0;
	monoSet = 1;

	Copy(tp);
}

SFPlayerInstr::~SFPlayerInstr()
{
}

/// Set the sound bank object directly.
/// This can also be done by setting the sound bank alias
void SFPlayerInstr::SetSoundBank(SoundBank *b)
{
	sndbnk = b;
	preset = 0;
}

void SFPlayerInstr::FindPreset()
{
	if (sndbnk == 0)
		sndbnk = SoundBank::FindBank(sndFile);
	if (sndbnk != 0)
		preset = sndbnk->GetInstr(bnkNum, preNum);
}

void SFPlayerInstr::Copy(SFPlayerInstr *tp)
{
	frq = tp->frq;
	sndbnk = tp->sndbnk;
	preset = tp->preset;
	bnkNum = tp->bnkNum;
	preNum = tp->preNum;
	sndFile = tp->sndFile;
	preName = tp->preName;

	chnl = tp->chnl;
	pitch = tp->pitch;
	monoSet = tp->monoSet;
	vol = tp->vol;
	frq = tp->frq;

	fmCM = tp->fmCM;
	fmIM = tp->fmIM;

	volEnv.Copy(&tp->volEnv);
	modEnv.Copy(&tp->modEnv);
	vibLFO.Copy(&tp->vibLFO);
}

/// Start playing a note.
/// Locate the preset if needed, then the zone(s).
/// Initialize oscillators and envelopes.
void SFPlayerInstr::Start(SeqEvent *evt)
{
	VarParamEvent *params = (VarParamEvent *)evt;
	pitch = params->pitch;
	frq = params->frq;
	chnl = params->chnl;
	vol  = params->vol;

	SetParams(params);

	zone = 0;
	xfade = 0;

	if (osc1)
		delete osc1;
	if (osc2)
		delete osc2;

	FrqValue oscFrq = frq;
	if (preset == 0)
		FindPreset();
	if (preset)
	{
		int sfpit = pitch+12; // shift from BasicSynth to MIDI range
		zone = preset->GetZone(sfpit, 64, 0);
		if (zone)
		{
			if (zone->sample->linkSamp || zone->sample->channels == 2)
				osc1 = new GenWaveSF2;
			else if (zone->linkZone)
				osc1 = new GenWaveSF3;
			else
				osc1 = new GenWaveSF1;
			osc1->InitSF(oscFrq, zone, 0);
		}
	}
	if (!osc1)
	{
		osc1 = new GenWaveSF1;
		osc1->InitSF(oscFrq, 0, 0);
	}
	volEnv.Reset(0);
	FrqValue fmFrq = oscFrq * fmCM;
	oscfm.InitWT(fmFrq, WT_SIN);
	fmAmp = fmFrq * fmIM;
	fmOn = fmAmp > 0;
	modEnv.Reset(0);
	vibLFO.SetSigFrq(oscFrq);
	vibLFO.Reset(0);
}

void SFPlayerInstr::Param(SeqEvent *evt)
{
	VarParamEvent *params = (VarParamEvent *)evt;
	SetParams((VarParamEvent*)evt);

	vol  = params->vol;

	if (params->pitch != pitch) // pitch change
	{
		pitch = params->pitch;
		int sfpit = pitch+12;
		zone2 = preset->GetZone(sfpit, 65, 0);
		if (zone2 != zone)
		{
			if (zone2->sample->linkSamp || zone2->sample->channels == 2)
				osc2 = new GenWaveSF2;
			else if (zone2->linkZone)
				osc2 = new GenWaveSF3;
			else
				osc2 = new GenWaveSF1;

			osc2->InitSF(params->frq, zone, 1);
			
			// begin 50ms cross-fade to new sample.
			xfade = synthParams.sampleRate / 10;
			fadeEG.InitSegTick(xfade, 0.0, 1.0);
		}

		frq = params->frq;
		vibLFO.SetSigFrq(frq);
		oscfm.SetFrequency(frq * fmCM);
		oscfm.Reset(-1);
	}
	fmAmp = frq * fmCM * fmIM;
	fmOn = fmAmp > 0;
	vibLFO.Reset(-1);
}

void SFPlayerInstr::Stop()
{
	osc1->Release();
	volEnv.Release();
	modEnv.Release();
}

/// Produce the next sample.
void SFPlayerInstr::Tick()
{
	FrqValue newFrq = frq;
	AmpValue ampVal = vol * volEnv.Gen();

	if (fmOn)
		newFrq += oscfm.Gen() * modEnv.Gen() * fmAmp;
	if (vibLFO.On())
		newFrq += vibLFO.Gen();

	AmpValue oscValR = 0.0;
	AmpValue oscValL = 0.0;
	osc1->UpdateFrequency(newFrq);
	osc1->Tick(oscValL, oscValR);
	if (xfade > 0)
	{
		AmpValue amp1 = fadeEG.Gen();
		AmpValue amp0 = 1.0 - amp1;
		AmpValue oscValR2;
		AmpValue oscValL2;
		osc2->Tick(oscValL2, oscValR2);
		oscValR = (amp0 * oscValR) + (amp1 * oscValR2);
		oscValL = (amp0 * oscValL) + (amp1 * oscValL2);
		if (--xfade == 0)
		{
			delete osc1;
			osc1 = osc2;
			osc2 = 0;
			zone = zone2;
			zone2 = 0;
		}
	}
	if (monoSet)
		im->Output(chnl, (oscValR + oscValL) * 0.5 * ampVal);
	else
		im->Output2(chnl, ampVal * oscValL, ampVal * oscValR);
}

int  SFPlayerInstr::IsFinished()
{
	return volEnv.IsFinished() || osc1->IsFinished();
}

void SFPlayerInstr::Destroy()
{
	delete this;
}

int SFPlayerInstr::SetParams(VarParamEvent *params)
{
	int err = 0;

	bsInt16 *id = params->idParam;
	float *valp = params->valParam;
	int n = params->numParam;
	while (n-- > 0)
		err += SetParam(*id++, *valp++);
	return err;
}

int SFPlayerInstr::SetParam(bsInt16 idval, float val)
{
	bsInt16 cmp;
	switch (idval)
	{
	case 16:
		cmp = (bsInt16) val;
		if (cmp != bnkNum)
		{
			bnkNum = cmp;
			preset = 0;
		}
		break;
	case 17:
		cmp = (bsInt16) val;
		if (cmp != preNum)
		{
			preNum = cmp;
			preset = 0;
			FindPreset();
		}
		break;
	case 18:
		monoSet = (int) val;
		break;

	case 20:
		volEnv.SetStart(AmpValue(val));
		break;
	case 21:
		volEnv.SetAtkRt(FrqValue(val));
		break;
	case 22:
		volEnv.SetAtkLvl(AmpValue(val));
		break;
	case 23:
		volEnv.SetDecRt(FrqValue(val));
		break;
	case 24:
		volEnv.SetSusLvl(AmpValue(val));
		break;
	case 25:
		volEnv.SetRelRt(FrqValue(val));
		break;
	case 26:
		volEnv.SetRelLvl(AmpValue(val));
		break;
	case 27:
		volEnv.SetType((EGSegType) (int) val);
		break;

	case 30:
		modEnv.SetStart(AmpValue(val));
		break;
	case 31:
		modEnv.SetAtkRt(FrqValue(val));
		break;
	case 32:
		modEnv.SetAtkLvl(AmpValue(val));
		break;
	case 33:
		modEnv.SetDecRt(FrqValue(val));
		break;
	case 34:
		modEnv.SetSusLvl(AmpValue(val));
		break;
	case 35:
		modEnv.SetRelRt(FrqValue(val));
		break;
	case 36:
		modEnv.SetRelLvl(AmpValue(val));
		break;
	case 37:
		modEnv.SetType((EGSegType) (int) val);
		break;

	case 40:
		vibLFO.SetFrequency(FrqValue(val));
		break;
	case 41:
		vibLFO.SetWavetable((int) val);
		break;
	case 42:
		vibLFO.SetAttack(FrqValue(val));
		break;
	case 43:
		vibLFO.SetLevel(AmpValue(val));
		break;
	case 44:
		fmCM = val;
		break;
	case 45:
		fmIM = val;
		break;

	default:
		return 1;
	}
	return 0;
}

int SFPlayerInstr::GetParam(bsInt16 idval, float *val)
{
	switch (idval)
	{
	case 16:
		*val = (float) bnkNum;
		break;
	case 17:
		*val = (float) preNum;
		break;
	case 18:
		*val = (float) monoSet;
		break;

	case 20:
		*val = volEnv.GetStart();
		break;
	case 21:
		*val = volEnv.GetAtkRt();
		break;
	case 22:
		*val = volEnv.GetAtkLvl();
		break;
	case 23:
		*val = volEnv.GetDecRt();
		break;
	case 24:
		*val = volEnv.GetSusLvl();
		break;
	case 25:
		*val = volEnv.GetRelRt();
		break;
	case 26:
		*val = volEnv.GetRelLvl();
		break;
	case 27:
		*val = (float) (int) volEnv.GetType();
		break;

	case 30:
		*val = modEnv.GetStart();
		break;
	case 31:
		*val = modEnv.GetAtkRt();
		break;
	case 32:
		*val = modEnv.GetAtkLvl();
		break;
	case 33:
		*val = modEnv.GetDecRt();
		break;
	case 34:
		*val = modEnv.GetSusLvl();
		break;
	case 35:
		*val = modEnv.GetRelRt();
		break;
	case 36:
		*val = modEnv.GetRelLvl();
		break;
	case 37:
		*val = (float) (int) modEnv.GetType();
		break;

	case 40:
		*val = vibLFO.GetFrequency();
		break;
	case 41:
		*val = (float) vibLFO.GetWavetable();
		break;
	case 42:
		*val = vibLFO.GetAttack();
		break;
	case 43:
		*val = vibLFO.GetLevel();
		break;
	case 44:
		*val = fmCM;
		break;
	case 45:
		*val = fmIM;
		break;

	default:
		return 1;
	}
	return 0;
}

int SFPlayerInstr::GetParams(VarParamEvent *params)
{
	params->SetParam(P_PITCH, (float)pitch);
	params->SetParam(P_FREQ, (float)frq);
	params->SetParam(P_VOLUME, (float)vol);
	params->SetParam(16, (float) bnkNum);
	params->SetParam(17, (float) preNum);
	params->SetParam(18, (float) monoSet);

	params->SetParam(20, (float) volEnv.GetStart());
	params->SetParam(21, (float) volEnv.GetAtkRt());
	params->SetParam(22, (float) volEnv.GetAtkLvl());
	params->SetParam(23, (float) volEnv.GetDecRt());
	params->SetParam(24, (float) volEnv.GetSusLvl());
	params->SetParam(25, (float) volEnv.GetRelRt());
	params->SetParam(26, (float) volEnv.GetRelLvl());
	params->SetParam(27, (float) volEnv.GetType());

	params->SetParam(30, (float) modEnv.GetStart());
	params->SetParam(31, (float) modEnv.GetAtkRt());
	params->SetParam(32, (float) modEnv.GetAtkLvl());
	params->SetParam(33, (float) modEnv.GetDecRt());
	params->SetParam(34, (float) modEnv.GetSusLvl());
	params->SetParam(35, (float) modEnv.GetRelRt());
	params->SetParam(36, (float) modEnv.GetRelLvl());
	params->SetParam(37, (float) modEnv.GetType());

	params->SetParam(40, (float) vibLFO.GetFrequency());
	params->SetParam(41, (float) vibLFO.GetWavetable());
	params->SetParam(42, (float) vibLFO.GetAttack());
	params->SetParam(43, (float) vibLFO.GetLevel());

	params->SetParam(44, (float) fmCM);
	params->SetParam(45, (float) fmIM);

	return 0;
}

////////////////////////////////////////////////////////////////

static InstrParamMap sfPlayerParams[] = 
{
	{"bank", 16 }, 

	{"fmcm", 44 },
	{"fmim", 45 },

	{"modAttack", 31},
	{"modDecay", 33},
	{"modEnd", 36},
	{"modPeak", 33}, 
	{"modRelease", 35},
	{"modStart", 30},
	{"modSustain", 34},
	{"modType", 37},

	{"monoSet", 18},
	{"preset", 17 }, 

	{"vibLfoAttack", 40},
	{"vibLfoFrq", 41},
	{"vibLfoLevel", 42},
	{"vibLfoWT", 43},

	{"volAttack", 21},
	{"volDecay", 23}, 
	{"volEnd", 26},
	{"volPeak", 22}, 
	{"volRelease", 25},
	{"volStart", 20},
	{"volSustain", 24},
	{"volType", 27}
};

bsInt16 SFPlayerInstr::MapParamID(const char *name, Opaque tmplt)
{
	return SearchParamID(name, sfPlayerParams, sizeof(sfPlayerParams)/sizeof(InstrParamMap));
}

const char *SFPlayerInstr::MapParamName(bsInt16 id, Opaque tmplt)
{
	return SearchParamName(id, sfPlayerParams, sizeof(sfPlayerParams)/sizeof(InstrParamMap));
}


int SFPlayerInstr::LoadEnv(XmlSynthElem *elem, EnvGenADSR& env)
{
	double dvals[7];
	short ival;
	elem->GetAttribute("st",  dvals[0]);
	elem->GetAttribute("atk", dvals[1]);
	elem->GetAttribute("pk",  dvals[2]);
	elem->GetAttribute("dec", dvals[3]);
	elem->GetAttribute("sus", dvals[4]);
	elem->GetAttribute("rel", dvals[5]);
	elem->GetAttribute("end", dvals[6]);
	elem->GetAttribute("ty", ival);
	env.SetStart(dvals[0]);
	env.SetAtkRt(dvals[1]);
	env.SetAtkLvl(dvals[2]);
	env.SetDecRt(dvals[3]);
	env.SetSusLvl(dvals[4]);
	env.SetRelRt(dvals[5]);
	env.SetRelLvl(dvals[6]);
	env.SetType((EGSegType)ival);

	return 0;
}

int SFPlayerInstr::Load(XmlSynthElem *parent)
{
	char *cval;

	XmlSynthElem elem;
	XmlSynthElem celem;
	XmlSynthElem *next = parent->FirstChild(&elem);
	while (next != NULL)
	{
		if (elem.TagMatch("venv"))
			LoadEnv(&elem, volEnv);
		else if (elem.TagMatch("menv"))
			LoadEnv(&elem, modEnv);
		else if (elem.TagMatch("vlfo"))
			vibLFO.Load(&elem);
		else if (elem.TagMatch("sf"))
		{
			elem.GetAttribute("bank", bnkNum);
			elem.GetAttribute("preset", preNum);
			elem.GetAttribute("mono", monoSet);
			elem.GetAttribute("fmcm", fmCM);
			elem.GetAttribute("fmim", fmIM);

			XmlSynthElem *child = elem.FirstChild(&celem);
			while (child)
			{
				if (celem.TagMatch("file"))
				{
					celem.GetContent(&cval);
					if (cval)
						sndFile.Attach(cval);
				}
				else if (celem.TagMatch("preset"))
				{
					celem.GetContent(&cval);
					if (cval)
						preName.Attach(cval);
				}
				child = celem.NextSibling(&celem);
			}
		}
		next = elem.NextSibling(&elem);
	}
	if (sndFile.Length() > 0)
		sndbnk = SoundBank::FindBank(sndFile);

	return 0;
}

int SFPlayerInstr::SaveEnv(XmlSynthElem *elem, EnvGenADSR& env)
{
	elem->SetAttribute("st", env.GetStart());
	elem->SetAttribute("atk", env.GetAtkRt());
	elem->SetAttribute("pk",  env.GetAtkLvl());
	elem->SetAttribute("dec", env.GetDecRt());
	elem->SetAttribute("sus", env.GetSusLvl());
	elem->SetAttribute("rel", env.GetRelRt());
	elem->SetAttribute("end", env.GetRelLvl());
	elem->SetAttribute("ty", (short)env.GetType());
	return 0;
}

int SFPlayerInstr::Save(XmlSynthElem *parent)
{
	XmlSynthElem elem;
	XmlSynthElem celem;

	if (!parent->AddChild("sf", &elem))
		return -1;

	elem.SetAttribute("bank", bnkNum);
	elem.SetAttribute("preset", preNum);
	elem.SetAttribute("mono", monoSet);
	elem.SetAttribute("fmcm", fmCM);
	elem.SetAttribute("fmim", fmIM);

	if (!elem.AddChild("file", &celem))
		return -1;
	celem.SetContent(sndFile);

	if (!elem.AddChild("preset", &celem))
		return -1;
	celem.SetContent(preName);

	if (!parent->AddChild("venv", &elem))
		return -1;
	SaveEnv(&elem, volEnv);

	if (!parent->AddChild("menv", &elem))
		return -1;
	SaveEnv(&elem, modEnv);

	if (!parent->AddChild("vlfo", &elem))
		return -1;
	vibLFO.Save(&elem);

	return 0;
}
