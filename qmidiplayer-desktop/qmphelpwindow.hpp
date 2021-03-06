#ifndef QMPHELPWINDOW_H
#define QMPHELPWINDOW_H

#include <QDialog>
#define APP_VERSION "0.8.2"

namespace Ui {
	class qmpHelpWindow;
}

class qmpHelpWindow : public QDialog
{
	Q_OBJECT

	public:
	explicit qmpHelpWindow(QWidget *parent = 0);
	~qmpHelpWindow();

	private slots:
	void on_textBrowser_sourceChanged(const QUrl &src);

	private:
	Ui::qmpHelpWindow *ui;
};

#endif // QMPHELPWINDOW_H
