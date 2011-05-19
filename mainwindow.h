#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include <stdint.h>
#include <limits.h>


namespace Ui {
    class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

signals:
	void FileSelection(QString szFile);

private slots:
	void on_actionFile_triggered();
	void onFileSelected(QString szArchiveFile);
	
	void on_actionAbout_triggered();

protected:
	QString IdToString(uint32_t u32Id);
	void ClearAll();
	
private:
    Ui::MainWindow *ui;
	QString m_szBaseTitle;
};

#endif // MAINWINDOW_H
