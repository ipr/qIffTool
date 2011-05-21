#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMap>

#include <stdint.h>
#include <limits.h>


namespace Ui {
    class MainWindow;
}

class CIffHeader;
class QTreeWidgetItem;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

signals:
	void FileSelection(QString szFile);

private slots:
	void on_actionAbout_triggered();
	
	void on_actionFile_triggered();
	void onFileSelected(QString szArchiveFile);
	

protected:
	void HeaderToDisplay(CIffHeader *pHead);
	void ChunksToDisplay(CIffHeader *pHead, QTreeWidgetItem *pTopItem);
	
	QString IdToString(uint32_t u32Id);
	void ClearAll();
	
private:
    Ui::MainWindow *ui;
	QString m_szBaseTitle;
	
	// composite-tree display mapping
	//
	QMap<CIffHeader*, QTreeWidgetItem*> m_FormToItem;
};

#endif // MAINWINDOW_H
