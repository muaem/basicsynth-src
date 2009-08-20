include ../../BasicSynth.cfg

$(BSBIN)/example9a: main.cpp $(INSTLIB) $(CMNLIB)
	$(CPP) -lm $(CPPFLAGS) -o $@ main.cpp $(INSTLIB) $(CMNLIB)
	
example9a: $(BSBIN)/example9a

clean:
	-rm main.o
	-rm $(BSBIN)/example9a
	
main.cpp: $(BSINC)/SynthDefs.h $(BSINC)/WaveFile.h \
	$(BSINC)/EnvGen.h $(BSINC)/BiQuad.h \
	$(BSINC)/GenNoise.h $(BSINC)/GenWave.h \
	$(BSINC)/WaveTable.h $(BSINC)/GenWaveWT.h \
	$(BSINC)/DelayLine.h $(BSINC)/Mixer.h \
	$(BSINC)/Sequencer.h $(BSINC)/SMFFile.h \
	$(BSINC)/SFFile.h $(BSINC)/SFSoundBank.h \
	$(BSINC)/SFGen.h $(BSINC)/SFDefs.h