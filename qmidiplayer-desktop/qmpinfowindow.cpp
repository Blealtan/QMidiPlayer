#include <QTextCodec>
#include "qmpinfowindow.hpp"
#include "ui_qmpinfowindow.h"
#include "qmpmainwindow.hpp"
#include "qmpsettingswindow.hpp"

const char* minors="abebbbf c g d a e b f#c#g#d#a#";
const char* majors="CbGbDbAbEbBbF C G D A E B F#C#";
const char* standards="?  GM GM2GS XG ";

qmpInfoWindow::qmpInfoWindow(QWidget *parent) :
	QDialog(parent),
	ui(new Ui::qmpInfoWindow)
{
	ui->setupUi(this);
	int w=size().width(),h=size().height();w=w*(logicalDpiX()/96.);h=h*(logicalDpiY()/96.);
	setMaximumWidth(w);setMaximumHeight(h);setMinimumWidth(w);setMinimumHeight(h);
}

qmpInfoWindow::~qmpInfoWindow()
{
	delete ui;
}

void qmpInfoWindow::updateInfo()
{
	char str[256];
	CMidiPlayer* player=qmpMainWindow::getInstance()->getPlayer();
	QSettings* settings=qmpSettingsWindow::getSettingsIntf();
	ui->lbFileName->setText(QString("File name: ")+qmpMainWindow::getInstance()->getFileName());
	if(player->getTitle())
	{
		if(settings->value("Midi/TextEncoding","").toString()!="Unicode")
		ui->lbTitle->setText(QString("Title: ")+
		QTextCodec::codecForName(settings->value("Midi/TextEncoding","").toString().toStdString().c_str())->toUnicode(player->getTitle()));
		else
		ui->lbTitle->setText(QString("Title: ")+player->getTitle());
	}
	else ui->lbTitle->setText(QString("Title: "));
	if(player->getCopyright())
	{
		if(settings->value("Midi/TextEncoding","").toString()!="Unicode")
		ui->lbCopyright->setText(QString("Copyright: ")+
		QTextCodec::codecForName(settings->value("Midi/TextEncoding","").toString().toStdString().c_str())->toUnicode(player->getCopyright()));
		else
		ui->lbCopyright->setText(QString("Copyright: ")+player->getCopyright());
	}
	else ui->lbCopyright->setText(QString("Copyright: "));
	ui->lbTempo->setText(QString("Tempo: ")+QString::number(player->getTempo(),'g',5));
	int t,r;t=player->getCurrentKeySignature();r=(int8_t)((t>>8)&0xFF)+7;
	strncpy(str,t&0xFF?minors+2*r:majors+2*r,2);str[2]='\0';
	ui->lbKeySig->setText(QString("Key Sig.: ")+str);
	player->getCurrentTimeSignature(&t,&r);sprintf(str,"Time Sig.: %d/%d",t,r);
	ui->lbTimeSig->setText(str);
	sprintf(str,"Note count: %u",player->getFileNoteCount());
	ui->lbNoteCount->setText(str);
	strncpy(str,standards+player->getFileStandard()*3,3);str[3]='\0';
	ui->lbFileStandard->setText(QString("File standard: ")+str);
}
