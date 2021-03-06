#ifdef _WIN32
#include <windows.h>
#else
#include <dlfcn.h>
#endif
#include <cstdio>
#include <cstring>
#include <QDirIterator>
#include "qmpplugin.hpp"
#include "qmpmainwindow.hpp"
#include "qmpsettingswindow.hpp"
qmpPluginAPI* pluginAPI;
qmpMainWindow* qmw;
qmpSettingsWindow* qsw;
#ifdef _WIN32
void qmpPluginManager::scanPlugins()
{
	QDirIterator *dir;
	std::vector<std::string> cpluginpaths;
	dir=new QDirIterator(".\\plugins\\");
	while(dir->hasNext())
	{
		dir->next();
		if(dir->fileInfo().suffix()=="dll")
			cpluginpaths.push_back(std::string(".\\plugins\\")+dir->fileName().toStdString());
	}
	delete dir;
	for(unsigned i=0;i<cpluginpaths.size();++i)
	{
		HMODULE hso=LoadLibraryA(cpluginpaths[i].c_str());
		if(!hso){fprintf(stderr,"Error while loading library: %d\n",GetLastError());continue;}
		FARPROC hndi=GetProcAddress(hso,"qmpPluginGetInterface");
		if(!hndi){fprintf(stderr,"file %s doesn't seem to be a qmidiplayer plugin.\n",cpluginpaths[i].c_str());continue;}
		qmpPluginEntry e=(qmpPluginEntry)hndi;
		qmpPluginIntf* intf=e(pluginAPI);
		plugins.push_back(qmpPlugin(std::string(intf->pluginGetName()),std::string(intf->pluginGetVersion()),std::string(cpluginpaths[i]),intf));
	}
}
#else
void qmpPluginManager::scanPlugins()
{
	QDirIterator *dir;
	std::vector<std::string> cpluginpaths;
#ifdef QMP_BUILD_UNIX_PACKAGE
	dir=new QDirIterator("/usr/lib/qmidiplayer/");
	while(dir->hasNext())
	{
		dir->next();
		if(dir->fileInfo().suffix()=="so")
			cpluginpaths.push_back(std::string("/usr/lib/qmidiplayer/")+dir->fileName().toStdString());
	}
	delete dir;
#endif
	dir=new QDirIterator("./");
	while(dir->hasNext())
	{
		dir->next();
		if(dir->fileInfo().suffix()=="so")
			cpluginpaths.push_back(std::string("./")+dir->fileName().toStdString());
	}
	delete dir;
	for(unsigned i=0;i<cpluginpaths.size();++i)
	{
		void* hso=dlopen(cpluginpaths[i].c_str(),RTLD_LAZY);
		if(!hso){fprintf(stderr,"%s\n",dlerror());continue;}
		void* hndi=dlsym(hso,"qmpPluginGetInterface");
		if(!hndi){fprintf(stderr,"file %s doesn't seem to be a qmidiplayer plugin.\n",cpluginpaths[i].c_str());continue;}
		qmpPluginEntry e=(qmpPluginEntry)hndi;
		qmpPluginIntf* intf=e(pluginAPI);
		plugins.push_back(qmpPlugin(std::string(intf->pluginGetName()),std::string(intf->pluginGetVersion()),std::string(cpluginpaths[i]),intf));
	}
}
#endif
qmpPluginManager::qmpPluginManager()
{
	qmw=qmpMainWindow::getInstance();
	qsw=qmw->getSettingsWindow();
	pluginAPI=new qmpPluginAPI();
}
qmpPluginManager::~qmpPluginManager()
{
	for(unsigned i=0;i<plugins.size();++i)
	{
		if(plugins[i].initialized)plugins[i].pinterface->deinit();
		delete plugins[i].pinterface;
	}
	qmw=NULL;qsw=NULL;delete pluginAPI;
}
std::vector<qmpPlugin> *qmpPluginManager::getPlugins()
{
	return &plugins;
}
void qmpPluginManager::initPlugins()
{
	for(unsigned i=0;i<plugins.size();++i)
	{
		if(!plugins[i].enabled)continue;
		printf("Loaded plugin: %s\n",plugins[i].path.c_str());
		plugins[i].pinterface->init();plugins[i].initialized=true;
	}
}
void qmpPluginManager::deinitPlugins()
{
	for(unsigned i=0;i<plugins.size();++i)
	{
		if(plugins[i].initialized)plugins[i].pinterface->deinit();
		plugins[i].enabled=plugins[i].initialized=false;
	}
}

qmpPluginAPI::~qmpPluginAPI(){}

uint32_t qmpPluginAPI::getDivision()
{return qmw&&qmw->getPlayer()?qmw->getPlayer()->getDivision():0;}
uint32_t qmpPluginAPI::getRawTempo()
{return qmw&&qmw->getPlayer()?qmw->getPlayer()->getRawTempo():0;}
double qmpPluginAPI::getRealTempo()
{return qmw&&qmw->getPlayer()?qmw->getPlayer()->getTempo():0;}
uint32_t qmpPluginAPI::getTimeSig()
{int n,d=0,t;qmw&&qmw->getPlayer()?qmw->getPlayer()->getCurrentTimeSignature(&n,&t):void(0);for(;t>>=1;++d);return n<<8|d;}
int qmpPluginAPI::getKeySig()
{return qmw&&qmw->getPlayer()?qmw->getPlayer()->getCurrentKeySignature():0;}
uint32_t qmpPluginAPI::getNoteCount()
{return qmw&&qmw->getPlayer()?qmw->getPlayer()->getFileNoteCount():0;}
uint32_t qmpPluginAPI::getCurrentPolyphone()
{return qmw&&qmw->getPlayer()?qmw->getPlayer()->getPolyphone():0;}
uint32_t qmpPluginAPI::getMaxPolyphone()
{return qmw&&qmw->getPlayer()?qmw->getPlayer()->getMaxPolyphone():0;}
uint32_t qmpPluginAPI::getCurrentTimeStamp()
{return qmw&&qmw->getPlayer()?qmw->getPlayer()->getTick():0;}
uint32_t qmpPluginAPI::getCurrentPlaybackPercentage()
{return qmw?qmw->getPlaybackPercentage():0;}
void qmpPluginAPI::playerSeek(uint32_t percentage)
{if(qmw)qmw->playerSeek(percentage);}
double qmpPluginAPI::getPitchBend(int ch)
{return qmw&&qmw->getPlayer()?qmw->getPlayer()->getPitchBend(ch):0;}
bool qmpPluginAPI::getChannelMask(int ch)
{return qmw&&qmw->getPlayer()?qmw->getPlayer()->getChannelMask(ch):false;}
std::string qmpPluginAPI::getTitle()
{return qmw?qmw->getTitle():"";}
std::wstring qmpPluginAPI::getWTitle()
{return qmw?qmw->getWTitle():L"";}
std::string qmpPluginAPI::getChannelPresetString(int ch)
{
	int b,p;char nm[25],ret[33];ret[0]=0;
	if(qmw&&qmw->getPlayer())
	{
		qmw->getPlayer()->getChannelPreset(ch,&b,&p,nm);
		sprintf(ret,"%03d:%03d %s",b,p,nm);
	}
	return std::string(ret);
}

void qmpPluginAPI::discardLastEvent(){if(qmw&&qmw->getPlayer())qmw->getPlayer()->discardLastEvent();}
void qmpPluginAPI::commitEventChange(SEventCallBackData d){if(qmw&&qmw->getPlayer())qmw->getPlayer()->commitEventChange(d);}

int qmpPluginAPI::registerEventHandlerIntf(IMidiCallBack *cb,void *userdata)
{return qmw->getPlayer()->setEventHandlerCB(cb,userdata);}
void qmpPluginAPI::unregisterEventHandlerIntf(int intfhandle)
{qmw->getPlayer()->unsetEventHandlerCB(intfhandle);}
int qmpPluginAPI::registerEventReaderIntf(IMidiCallBack *cb,void *userdata)
{return qmw->getPlayer()->setEventReaderCB(cb,userdata);}
void qmpPluginAPI::unregisterEventReaderIntf(int intfhandle)
{qmw->getPlayer()->unsetEventReaderCB(intfhandle);}
int qmpPluginAPI::registerVisualizationIntf(qmpVisualizationIntf* intf)
{return qmw->registerVisualizationIntf(intf);}
void qmpPluginAPI::unregisterVisualizationIntf(int intfhandle)
{qmw->unregisterVisualizationIntf(intfhandle);}

void qmpPluginAPI::registerOptionInt(std::string tab,std::string desc,std::string key,int min,int max,int defaultval)
{qsw->registerOptionInt(tab,desc,key,min,max,defaultval);}
int qmpPluginAPI::getOptionInt(std::string key){return qsw->getOptionInt(key);}
void qmpPluginAPI::setOptionInt(std::string key,int val){qsw->setOptionInt(key,val);}
void qmpPluginAPI::registerOptionUint(std::string tab,std::string desc,std::string key,unsigned min,unsigned max,unsigned defaultval)
{qsw->registerOptionUint(tab,desc,key,min,max,defaultval);}
unsigned qmpPluginAPI::getOptionUint(std::string key){return qsw->getOptionUint(key);}
void qmpPluginAPI::setOptionUint(std::string key,unsigned val){qsw->setOptionUint(key,val);}
void qmpPluginAPI::registerOptionBool(std::string tab,std::string desc,std::string key,bool defaultval)
{qsw->registerOptionBool(tab,desc,key,defaultval);}
bool qmpPluginAPI::getOptionBool(std::string key){return qsw->getOptionBool(key);}
void qmpPluginAPI::setOptionBool(std::string key,bool val){qsw->setOptionBool(key,val);}
void qmpPluginAPI::registerOptionDouble(std::string tab,std::string desc,std::string key,double min,double max,double defaultval)
{qsw->registerOptionDouble(tab,desc,key,min,max,defaultval);}
double qmpPluginAPI::getOptionDouble(std::string key){return qsw->getOptionDouble(key);}
void qmpPluginAPI::setOptionDouble(std::string key,double val){qsw->setOptionDouble(key,val);}
void qmpPluginAPI::registerOptionString(std::string tab,std::string desc,std::string key,std::string defaultval)
{qsw->registerOptionString(tab,desc,key,defaultval);}
std::string qmpPluginAPI::getOptionString(std::string key){return qsw->getOptionString(key);}
void qmpPluginAPI::setOptionString(std::string key,std::string val){return qsw->setOptionString(key,val);}
void qmpPluginAPI::registerOptionEnumInt(std::string tab,std::string desc,std::string key,std::vector<std::string> options,int defaultval)
{qsw->registerOptionEnumInt(tab,desc,key,options,defaultval);}
int qmpPluginAPI::getOptionEnumInt(std::string key){return qsw->getOptionEnumInt(key);}
void qmpPluginAPI::setOptionEnumInt(std::string key,int val){return qsw->setOptionEnumInt(key,val);}
