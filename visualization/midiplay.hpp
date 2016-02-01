#include <cstring>
#include <cstdint>
#include <cstdlib>
#include "midivisual.hpp"
struct SEvent
{
	uint32_t time,p1,p2,iid;
	char type;
	char *str;
	SEvent(){time=p1=p2=0;type=0;str=NULL;}
	SEvent(uint32_t _iid,uint32_t _t,char _tp,uint32_t _p1,uint32_t _p2,const char* s=NULL)
	{
		iid=_iid;time=_t;type=_tp;
		p1=_p1;p2=_p2;
		if(s){str=new char[strlen(s)+2];strcpy(str,s);}else str=NULL;
	}
	friend bool operator <(SEvent a,SEvent b)
	{return a.time-b.time?a.time<b.time:a.iid<b.iid;}
};
extern SEvent eventList[10000000];
extern uint32_t eventc;
extern uint32_t fmt,trk,divs;
extern void readMidiFile(const char* fn);
extern CMidiVisual* mv;
