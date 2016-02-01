#include <cstdio>
#include <algorithm>
#include <chrono>
#include <thread>
#include <fluidsynth.h>
#include <csignal>
#include "midiplay.hpp"
SEvent eventList[10000000];
uint32_t eventc,eptr;
uint32_t ctempo,ctsn,ctsd,dpt;//delay_per_tick
fluid_settings_t* settings;
fluid_synth_t* synth;
fluid_audio_driver_t* adriver;
CMidiVisual* mv;
std::thread *th;
void fluidInitialize(const char* sf)
{
	settings=new_fluid_settings();
	fluid_settings_setstr(settings,"audio.driver","pulseaudio");
	fluid_settings_setint(settings,"synth.cpu-cores",4);
	fluid_settings_setint(settings,"synth.min-note-length",10);
	fluid_settings_setint(settings,"synth.polyphony",256);
	synth=new_fluid_synth(settings);
	adriver=new_fluid_audio_driver(settings,synth);
	fluid_synth_sfload(synth,sf,1);
}
void fluidDeinitialize()
{
	delete_fluid_audio_driver(adriver);
	delete_fluid_synth(synth);
	delete_fluid_settings(settings);
}
void processEvent(SEvent e)
{
	switch(e.type&0xF0)
	{
		case 0x80://Note off
			fluid_synth_noteoff(synth,e.type&0x0F,e.p1);
			//printf("OFF %02d %03d\n",e.type&0x0F,e.p1);
		break;
		case 0x90://Note on
			fluid_synth_noteon(synth,e.type&0x0F,e.p1,e.p2);
			//printf("ON  %02d %03d\n",e.type&0x0F,e.p1);
		break;
		case 0xB0://CC
			fluid_synth_cc(synth,e.type&0x0F,e.p1,e.p2);
		break;
		case 0xC0://PC
			fluid_synth_program_change(synth,e.type&0x0F,e.p1);
		break;
		case 0xE0://PW
			fluid_synth_pitch_bend(synth,e.type&0x0F,e.p1);
		break;
		case 0xF0://Meta/SysEx
			if((e.type&0x0F)==0x0F)
			{
				switch(e.p1)
				{
					case 0x51:
						ctempo=e.p2;dpt=ctempo*1000/divs;
						mv->tempoChange(ctempo);
					break;
					case 0x58:
					break;
					case 0x59:
					break;
					case 0x01:case 0x02:case 0x03:
					case 0x04:case 0x05:case 0x06:
					case 0x07:
						if(e.str)puts(e.str);
					break;
				}
			}
		break;
	}
}
void playEvents()
{
	eptr=0;
	for(uint32_t ct=eventList[0].time;eptr<eventc;)
	{
		while(ct==eventList[eptr].time&&eptr<eventc)
			processEvent(eventList[eptr++]);
		if(eptr>=eventc)break;
		std::this_thread::sleep_for(std::chrono::nanoseconds(eventList[eptr].time-ct)*dpt);
		//printf("DLY %09d\n",(eventList[eptr].time-ct)*dpt/1000);
		ct=eventList[eptr].time;
		mv->sync(ct);
		//printf("current poly: %d\n",fluid_synth_get_active_voice_count(synth));
	}
}
void sigTerm(int pm)
{eptr=eventc;exit(0);}
int main(int argc,char **argv)
{
	signal(SIGTERM,sigTerm);
	signal(SIGINT,sigTerm);
	if(argc!=3)return printf("Usage: %s <midi file> <soundfont file>\n",argv[0]),0;
	mv=new CMidiVisual();
	readMidiFile(argv[1]);mv->setDivision(divs);mv->tempoChange(500000);
	std::sort(eventList,eventList+eventc);
	ctempo=0x7A120;ctsn=4;ctsd=2;dpt=ctempo*1000/divs;printf("%d\n",divs);
	fluidInitialize(argv[2]);th=new std::thread(&CMidiVisual::show,mv);
	mv->start();
	playEvents();
	fluidDeinitialize();
	mv->close();
	th->join();
	delete mv;
	return 0;
}
