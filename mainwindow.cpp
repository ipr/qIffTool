#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QFileDialog>
#include <QMessageBox>
#include <QTextEdit>

#include "MemoryMappedFile.h"
#include "IffContainer.h"


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    m_szBaseTitle()
{
    ui->setupUi(this);
	m_szBaseTitle = windowTitle();
	connect(this, SIGNAL(FileSelection(QString)), this, SLOT(onFileSelected(QString)));
	
	QStringList treeHeaders;
	treeHeaders << "Chunk type" 
			<< "Offset" 
			<< "Size" ;
	ui->treeWidget->setColumnCount(treeHeaders.size());
	ui->treeWidget->setHeaderLabels(treeHeaders);
	
	
	// if file given in command line
	QStringList vCmdLine = QApplication::arguments();
	if (vCmdLine.size() > 1)
	{
		emit FileSelection(vCmdLine[1]);
	}
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_actionFile_triggered()
{
	QString szFile = QFileDialog::getOpenFileName(this, tr("Open file"));
	if (szFile != NULL)
	{
		emit FileSelection(szFile);
	}
}

void MainWindow::onFileSelected(QString szArchiveFile)
{
	ClearAll();

	
	std::wstring szFile = szArchiveFile.toStdWString();
	CMemoryMappedFile File;
	if (File.Create(szFile.c_str()) == false)
	{
		return;
	}
	
	QString szMsg;
	szMsg.append("File size: ").append(QString::number(File.GetSize()));
	ui->statusBar->showMessage(szMsg);
	
	CIffContainer Iff;
	CIffHeader *pHead = Iff.ParseIffFile(File);
	if (pHead == nullptr)
	{
		return;
	}

	setWindowTitle(m_szBaseTitle + " - " + szArchiveFile);
	
	// file head-chunk
	QTreeWidgetItem *pTopItem = new QTreeWidgetItem((QTreeWidgetItem*)0);
	pTopItem->setText(0, IdToString(pHead->m_iFileID));
	pTopItem->setText(1, QString::number(8));
	pTopItem->setText(2, QString::number(pHead->m_iDataSize));
	ui->treeWidget->addTopLevelItem(pTopItem);
	
	// root-level chunk-nodes
	CIffChunk *pChunk = pHead->m_pFirst;
	while (pChunk != nullptr)
	{
		QTreeWidgetItem *pSubItem = new QTreeWidgetItem(pTopItem);
		pSubItem->setText(0, IdToString(pChunk->m_iChunkID));
		pSubItem->setText(1, QString::number(pChunk->m_iOffset));
		pSubItem->setText(2, QString::number(pChunk->m_iChunkSize));
		pTopItem->addChild(pSubItem);
		
		pChunk = pChunk->m_pNext;
	}
	
	ui->treeWidget->expandAll();
	ui->treeWidget->resizeColumnToContents(0);
	//ui->treeWidget->sortByColumn(1);
}

void MainWindow::on_actionAbout_triggered()
{
	QTextEdit *pTxt = new QTextEdit(this);
	pTxt->setWindowFlags(Qt::Window); //or Qt::Tool, Qt::Dialog if you like
	pTxt->setReadOnly(true);
	pTxt->append("qIffTool by Ilkka Prusi 2011");
	pTxt->append("");
	pTxt->append("This program is free to use and distribute. No warranties of any kind.");
	pTxt->append("Program uses Qt 4.7.2 under LGPL v. 2.1");
	pTxt->append("");
	pTxt->append("Keyboard shortcuts:");
	pTxt->append("");
	pTxt->append("F = open file");
	pTxt->append("Esc = close");
	pTxt->append("? = about (this dialog)");
	pTxt->append("");
	pTxt->show();
}

// "unpack" ID-bytes to displayable string
QString MainWindow::IdToString(uint32_t u32Id)
{
	uint32_t u32Mask = 0xFF;
	QString szTemp;
	szTemp += (char)(u32Id & u32Mask);
	
	szTemp += (char)((u32Id & (u32Mask << 8)) >> 8);

	szTemp += (char)((u32Id & (u32Mask << 16)) >> 16);

	szTemp += (char)((u32Id & (u32Mask << 24)) >> 24);
	return szTemp;
}

void MainWindow::ClearAll()
{
	setWindowTitle(m_szBaseTitle);
	
	ui->treeWidget->clear();
}
