#ifndef __QXBOXFILECONVERTER_HXX_
#define __QXBOXFILECONVERTER_HXX_

#include <iostream>
#include <QWidget>
#include <QPushButton>
#include <QListWidget>
#include <QComboBox>
#include <QLabel>
#include <QCalendarWidget>
#include <QDateEdit>
#include <QRadioButton>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QCheckBox>
#include <QDir>

//#include <QDateTimeEdit>

class QXboxFileConverter : public QWidget {
    
Q_OBJECT

public:
	QXboxFileConverter(QWidget *parent = 0);
	virtual ~QXboxFileConverter();

private:
	QDir                   fSrcDir;
	QString                fLog;

	QLabel                *fSrcDirLbl;
	QLineEdit             *fSrcDirEdt;
	QPushButton           *fLoadSrcDirBtn;

	QLabel                *fSubDirLbl;
	QComboBox             *fSubDirCmb;

	QLabel                *fFilterPrefixLbl;
	QLineEdit             *fFilterEventPrefixEdt;
	QLineEdit             *fFilterTrendPrefixEdt;

	QLabel                *fDatePeriodLbl;
	QDateEdit             *fDateBeginDtp;
	QDateEdit             *fDateEndDtp;

	QLabel                *fOutputLbl;
	QRadioButton          *fFormatROOTRdp;
	QRadioButton          *fFormatHDF5Rdp;

	QLineEdit             *fOutputFileEdt;
	QPushButton           *fSaveAsBtn;
	QCheckBox             *fSplitChk;
	QCheckBox             *fAppendChk;

	QPlainTextEdit        *fLogEdt;

	QPushButton           *fConvertBtn;
	QPushButton           *fCancelBtn;
	QPushButton           *fQuitBtn;

	static std::vector<QDir> fgPredSrcDirs; // list of predefined source directories to look first
	static QString         fgPredFilterEventPrefix; // prefix for the event data files
	static QString         fgPredFilterTrendPrefix; // prefix for the trend data files

private slots:
	void                   onLoadSrcDir();
	void                   onSrcDirEdtTextChanged(const QString &text);
	void                   onSubDirCmbItemhanged(const QString &text);
	void                   onFilterEdtTextChanged(const QString &text);
	void                   onFormatChange();
	void                   onSaveAs();
	void                   onConvert();
	void                   onCancel();
	void                   keyPressEvent(QKeyEvent *e);


protected:

    std::vector<QString>   getCurrentSubDirectories();
    int                    getPeriodFromFiles(QDate &datebegin, QDate &dateend, const QStringList &files);

    void                   updateFilterSettings();
    void                   updateDateSettings();

	void setOutputFile(const QString& name);

};

#endif
