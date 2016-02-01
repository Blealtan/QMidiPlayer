//CLI Midi file player based on libfluidsynth
//Midi file reading module
//Written by Chris Xiong, 2015
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <cstdint>
#include <cstdarg>
#include <ctime>
#include "midiplay.hpp"
FILE *f;
int byteread;
uint32_t fmt,trk,divs,notes,curt,curid;
void error(int fatal,const char* format,...)
{
	va_list ap;
	va_start(ap,format);vfprintf(stderr,format,ap);va_end(ap);
	fprintf(stderr," at %#lx\n",ftell(f));
	if(fatal)exit(2);
}
uint32_t readSW()
{
	byteread+=2;
	uint32_t ret=0;
	for(int i=0;i<2;++i){ret<<=8;ret|=((uint32_t)fgetc(f))&0xFF;}
	return ret;
}
uint32_t readDW()
{
	byteread+=4;
	uint32_t ret=0;
	for(int i=0;i<4;++i){ret<<=8;ret|=((uint32_t)fgetc(f))&0xFF;}
	return ret;
}
uint32_t readVL()
{
	uint32_t ret=0,t,c=0;
	do
	{
		t=fgetc(f);
		if(++c>4)error(1,"E: Variable length type overflow.");
		ret<<=7;ret|=(t&0x7F);
	}while(t&0x80);
	byteread+=c;
	return ret;
}
int eventReader()//returns 0 if End of Track encountered
{
	uint32_t delta=readVL();curt+=delta;
	//printf("event@%#lx, delta %u: ",ftell(f),delta);
	char type=fgetc(f);++byteread;uint32_t p1,p2;
	static char lasttype;
retry:
	switch(type&0xF0)
	{
		case 0x80://Note Off
			p1=fgetc(f);p2=fgetc(f);byteread+=2;
			eventList[eventc++]=SEvent(curid,curt,type,p1,p2);
			mv->pushNoteOff(curt,type&0x0F,p1);
			//printf("Note off at ch#%d, note #%d, vel %d.\n",ch,p1,p2);
		break;
		case 0x90://Note On
			p1=fgetc(f);p2=fgetc(f);byteread+=2;
			if(p2)
			{
				++notes;
				eventList[eventc++]=SEvent(curid,curt,type,p1,p2);
				mv->pushNoteOn(curt,type&0x0F,p1,p2);
			}
			else
			{
				eventList[eventc++]=SEvent(curid,curt,(type&0x0F)|0x80,p1,p2);
				mv->pushNoteOff(curt,type&0x0F,p1);
			}
			//printf("Note on at ch#%d, note #%d, vel %d.\n",ch,p1,p2);
		break;
		case 0xA0://Note Aftertouch
			p1=fgetc(f);p2=fgetc(f);byteread+=2;
			eventList[eventc++]=SEvent(curid,curt,type,p1,p2);
			//printf("Note aftertouch at ch#%d, note #%d, vel %d.\n",ch,p1,p2);
		break;
		case 0xB0://Controller Change
			p1=fgetc(f);p2=fgetc(f);byteread+=2;
			eventList[eventc++]=SEvent(curid,curt,type,p1,p2);
			//printf("Controller change at ch#%d, cc #%d, val %d.\n",ch,p1,p2);
		break;
		case 0xC0://Patch Change
			p1=fgetc(f);++byteread;
			eventList[eventc++]=SEvent(curid,curt,type,p1,0);
			//printf("Patch change at ch#%d, pc #%d.\n",ch,p1);
		break;
		case 0xD0://Channel Aftertouch
			p1=fgetc(f);++byteread;
			eventList[eventc++]=SEvent(curid,curt,type,p1,0);
			//printf("Channel aftertouch at ch#%d, vel #%d.\n",ch,p1);
		break;
		case 0xE0://Pitch wheel
			p1=fgetc(f);p2=fgetc(f);byteread+=2;
			eventList[eventc++]=SEvent(curid,curt,type,(p1|(p2<<7))&0x3FFF,0);
			//printf("Pitch wheel at ch#%d, val %u.\n",type&0x0F,(p1|(p2<<7))&0x3FFF);
		break;
		case 0xF0:
			if((type&0x0F)==0x0F)//Meta Event
			{
				char metatype=fgetc(f);++byteread;
				switch(metatype)
				{
					case 0x00://Sequence Number
						fgetc(f);fgetc(f);fgetc(f);
						byteread+=3;
						//printf("seqence number.\n");
					break;
					case 0x20://Channel Prefix
						fgetc(f);fgetc(f);byteread+=2;
						//printf("channel prefix.\n");
					break;
					case 0x2F://End of Track
						fgetc(f);++byteread;
						//printf("end of track.\n");
						return 0;
					break;
					case 0x51://Set Tempo
						//fgetc(f);fgetc(f);fgetc(f);fgetc(f);
						p1=readDW();p1&=0x00FFFFFF;
						eventList[eventc++]=SEvent(curid,curt,type,metatype,p1);
						//byteread+=4;
					break;
					case 0x54://SMTPE offset, not handled.
						fgetc(f);fgetc(f);fgetc(f);
						fgetc(f);fgetc(f);fgetc(f);
						byteread+=6;
					break;
					case 0x58://Time signature
						fgetc(f);++byteread;
						p1=readDW();
						eventList[eventc++]=SEvent(curid,curt,type,metatype,p1);
					break;
					case 0x59://Key signature
						fgetc(f);++byteread;
						p1=readSW();
						eventList[eventc++]=SEvent(curid,curt,type,metatype,p1);
					break;
					case 0x01:case 0x02:case 0x03:
					case 0x04:case 0x05:case 0x06:
					case 0x07:case 0x7F:default://text-like meta
					{
						uint32_t len=readVL(),c;char* str=NULL;
						if(len<=1024&&len>0)str=new char[len+8];
						for(c=0;c<len;++c)
						{
							++byteread;if(str)str[c]=fgetc(f);else fgetc(f);
						}
						eventList[eventc++]=SEvent(curid,curt,type,metatype,0,str);
						if(len<=1024&&len>0)delete[] str;
					}
				}
			}
			else if((type&0x0F)==0x00||(type&0x0F)==0x07)//SysEx
			{
				uint32_t len=readVL();
				while(len--){++byteread;fgetc(f);}
			}
			else error(0,"W: Unknown event type %#x",type);
		break;
		default:
			fseek(f,-1,SEEK_CUR);--byteread;type=lasttype;goto retry;
	}
	lasttype=type;
	++curid;
	return 1;
}
void trackChunkReader()
{
	int chnklen=readDW();byteread=0;curt=0;curid=0;
	while(/*byteread<chnklen&&*/eventReader());
	if(byteread<chnklen)
	{
		error(0,"W: Extra bytes after EOT event.");
		while(byteread<chnklen){fgetc(f);++byteread;}
	}
	/*if(byteread>chnklen)
	{
		error(1,"E: Read past end of track.");
	}*/
}
void headerChunkReader()
{
	int chnklen=readDW();byteread=0;
	if(chnklen<6)error(1,"E: Header chunk too short.");
	if(chnklen>6)error(0,"W: Header chunk length longer than expected. Ignoring extra bytes.");
	fmt=readSW();trk=readSW();divs=readSW();
	if(divs&0x8000)error(1,"E: SMTPE format is not supported.");
	for(;byteread<chnklen;++byteread){fgetc(f);}
}
void chunkReader(int hdrXp)
{
	char hdr[6];
	if(!fgets(hdr,5,f))error(1,"E: Unexpected EOF.");
	if(hdrXp)
		if(strncmp(hdr,"MThd",4))error(1,"E: Wrong MIDI header.");
		else headerChunkReader();
	else
		if(strncmp(hdr,"MTrk",4))
		{
			error(0,"W: Wrong track chunk header. Ignoring the whole chunk.");
			for(int chnklen=readDW();chnklen>0;--chnklen)fgetc(f);
		}
		else trackChunkReader();
}
void readMidiFile(const char* fn)
{
	if(!(f=fopen(fn,"rb")))exit((printf("E: file %s doesn't exist!\n",fn),2));
	chunkReader(1);
	for(uint32_t i=0;i<trk;++i)chunkReader(0);
	printf("%d note(s)\n",notes);
	fclose(f);
}
