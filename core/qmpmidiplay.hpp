//sorry for the stupid C-like code...
#ifndef QMPMIDIPLAY_H
#define QMPMIDIPLAY_H
#include <cstring>
#include <cstdlib>
#include <vector>
#include <fluidsynth.h>
#include "../include/qmpcorepublic.hpp"
#include "qmpmidimappers.hpp"
struct SEvent
{
	uint32_t iid,time,p1,p2;
	uint8_t type;
	char *str;
	SEvent(){time=p1=p2=0;type=0;str=NULL;}
	~SEvent(){if(str){delete[] str;str=NULL;}}
	SEvent(uint32_t _iid,uint32_t _t,char _tp,uint32_t _p1,uint32_t _p2,const char* s=NULL)
	{
		iid=_iid;time=_t;type=_tp;
		p1=_p1;p2=_p2;
		if(s){str=new char[strlen(s)+2];strcpy(str,s);}else str=NULL;
	}
};
class CMidiFile
{
	private:
		std::vector<SEvent*>eventList;
		char *title,*copyright;
		uint32_t std;//standard 0=? 1=GM 2=GM2 3=GS 4=XG
		uint32_t fmt,trk,divs;
		FILE *f;
		int byteread,valid,eventdiscarded;
		uint32_t notes,curt,curid;
		IMidiCallBack* eventReaderCB[16];
		void* eventReaderCBuserdata[16];

		void error(int fatal,const char* format,...);
		uint32_t readSW();
		uint32_t readDW();
		uint32_t readVL();
		int eventReader();
		void trackChunkReader();
		void headerChunkReader();
		int chunkReader(int hdrXp);
		void dumpEvents();
	public:
		CMidiFile(const char* fn,IMidiCallBack** ercb,void** ercbdata);
		~CMidiFile();
		const SEvent* getEvent(uint32_t id);
		uint32_t getEventCount();
		uint32_t getDivision();
		uint32_t getNoteCount();
		uint32_t getStandard();
		const char* getTitle();
		const char* getCopyright();
		bool isValid();
		void discardLastEvent();
		void commitEventChange(SEventCallBackData d);
};
class CMidiPlayer
{
	private:
		CMidiFile *midiFile;
		uint32_t stamps[101];
		uint32_t ccstamps[101][16][135],ccc[16][135];
		//0..127:cc 128:pc 129:cp 130:pb 131:tempo 132:ts 133:ks 134:pbr
		int32_t rpnid[16],rpnval[16];
		uint16_t mute,solo;
		double ftime;
		bool sendSysEx,singleInstance,waitvoice;
		fluid_settings_t* settings;
		fluid_synth_t* synth;
		fluid_audio_driver_t* adriver;
		fluid_player_t* player;
		uint32_t ctempo,ctsn,ctsd,dpt,divs,cks;
		//raw tempo, timesig num., timesig den., delay per tick, division, keysig
		//thread control
		uint32_t tceptr,tcpaused,tcstop,ct;
		uint32_t finished,resumed;
		uint32_t pbr[16],pbv[16];
		qmpMidiMapperRtMidi *mapper;
		int mappedoutput[16],deviceusage[16],deviceiid[128];
		uint8_t chstate[16],chstatus[16][130];//0..127: cc 128: pc
		IMidiCallBack* eventHandlerCB[16];
		IMidiCallBack* eventReaderCB[16];
		void* eventHandlerCBuserdata[16];
		void* eventReaderCBuserdata[16];

		void setBit(uint16_t &n,uint16_t bn,uint16_t b);
		void processEvent(const SEvent *e);
		void processEventStub(const SEvent *e);
		void prePlayInit();
		void playEvents();
		void fileTimer1Pass();
		void fileTimer2Pass();
	public:
		CMidiPlayer(bool singleInst=false);
		~CMidiPlayer();
		bool playerLoadFile(const char* fn);
		void playerInit();
		void fluidPreInitialize();
		void fluidInitialize();
		void fluidDeinitialize();
		void playerDeinit();
		void playerThread();
		void playerPanic(bool reset=false);

		void rendererLoadFile(const char* ofn);
		void rendererInit(const char *fn);
		void rendererThread();
		void rendererDeinit();

		//playing control methods
		uint32_t getStamp(int id);
		uint32_t getTCeptr();
		void setTCeptr(uint32_t ep,uint32_t st);
		uint32_t getTCpaused();
		void setTCpaused(uint32_t ps);
		uint32_t isFinished();
		void setResumed();
		void setWaitVoice(bool wv);

		double getFtime();
		void getCurrentTimeSignature(int *n,int *d);
		int getCurrentKeySignature();
		uint32_t getFileNoteCount();
		uint32_t getFileStandard();
		double getTempo();
		uint32_t getTick();
		uint32_t getRawTempo();
		uint32_t getDivision();
		double getPitchBend(int ch);
		const char* getTitle();
		const char* getCopyright();

		void setGain(double gain);
		void sendSysX(bool send);
		int getPolyphone();
		int getMaxPolyphone();
		void setMaxPolyphone(int p);

		void setChannelPreset(int ch,int b,int p);
		void getChannelPreset(int ch,int *b,int *p,char *name);
		void setMute(int ch,bool m);
		void setSolo(int ch,bool s);
		bool getChannelMask(int ch);
		int getCC(int ch,int id);
		void setCC(int ch,int id,int val);
		void getReverbPara(double *r,double *d,double *w,double *l);
		void setReverbPara(int e,double r,double d,double w,double l);
		void getChorusPara(int *fb,double *l,double *r,double *d,int *type);
		void setChorusPara(int e,int fb,double l,double r,double d,int type);

		fluid_settings_t* getFluidSettings();
		void pushSoundFont(const char* sf);
		int getSFCount();
		fluid_sfont_t* getSFPtr(int sfid);

		qmpMidiMapperRtMidi* getMidiMapper();
		void setChannelOutput(int ch,int devid);
		uint8_t* getChstates();
		int setEventHandlerCB(IMidiCallBack *cb,void *userdata);
		void unsetEventHandlerCB(int id);
		int setEventReaderCB(IMidiCallBack *cb,void *userdata);
		void unsetEventReaderCB(int id);

		void discardLastEvent();
		void commitEventChange(SEventCallBackData d);
};
#endif
