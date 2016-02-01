//Experimental MIDI visualization class...
//by Chris Xiong, 2016
#include <smelt.hpp>
#include <smmath.hpp>
#include <smttfont.hpp>
struct MidiVisualEvent
{
	uint32_t tcs,tce;
	uint32_t key,vel;
	uint32_t ch;
};
class CMidiVisual
{
	private:
		MidiVisualEvent* pool[1000000];
		smHandler* h;
		uint32_t pendingt[16][128][32],pendingv[16][128][32];
		SMELT *sm;
		SMTRG tdscn;
		SMTEX chequer;
		smTTFont font;
		float pos[3],rot[3],lastx,lasty;
		uint32_t ecnt,ctc,dvs,ctk;
		double etps;
		void drawCube(smvec3d a,smvec3d b,DWORD col,SMTEX tex);
	public:
		CMidiVisual();
		void pushNoteOn(uint32_t tc,uint32_t ch,uint32_t key,uint32_t vel);
		void pushNoteOff(uint32_t tc,uint32_t ch,uint32_t key);
		void pushPitchBend(uint32_t tc,uint32_t ch,uint32_t key);
		void tempoChange(uint32_t rawtempo);//passes raw tempo in midi notation
		void setDivision(uint32_t division);
		void sync(uint32_t tc);
		bool update();
		void clearPool();
		void start();
		void stop();
		void show();
		void close();
};
class CMidiVisualHandler:public smHandler
{
private:
	CMidiVisual *p;
public:
	CMidiVisualHandler(CMidiVisual* par){p=par;};
	bool handlerFunc(){return p->update();}
};
