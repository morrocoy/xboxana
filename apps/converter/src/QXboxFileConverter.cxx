#include <QApplication>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QKeyEvent>
#include <QDate>
#include <QFileDialog>
#include <QFileInfo>
#include <QRegExp>
#include <QTextStream>
#include <QList>

#include "QXboxFileConverter.hxx"
#include "XboxFileConverter.hxx"


std::vector<QDir> QXboxFileConverter::fgPredSrcDirs = {
	QDir("/dfs/Workspaces/x"),
	QDir("G:/Workspaces/x"),
	QDir::currentPath()
};

QString QXboxFileConverter::fgPredFilterEventPrefix = "Event";
QString QXboxFileConverter::fgPredFilterTrendPrefix = "Trend";

QXboxFileConverter::QXboxFileConverter(QWidget *parent)
	: QWidget(parent)
	{

	QHBoxLayout *hbox_input = new QHBoxLayout();
	QVBoxLayout *vbox = new QVBoxLayout(this); //  The QHBoxLayout will be the base layout for the widgets.
	QHBoxLayout *hbox_filter = new QHBoxLayout(); // filter prefix for event and trend data files
	QHBoxLayout *hbox_datepicker = new QHBoxLayout(); // The QHBoxLayout will be the row for the date pickers.
	QHBoxLayout *hbox_format = new QHBoxLayout();
	QHBoxLayout *hbox_output = new QHBoxLayout();
	QHBoxLayout *hbox_buttons = new QHBoxLayout(); // The QHBoxLayout will be the row for the buttons.

	// select source folder
	fSrcDirLbl = new QLabel("Source directory", this);
	vbox->addWidget(fSrcDirLbl, 0, Qt::AlignLeft);
	fSrcDirEdt = new QLineEdit(this);
	fLoadSrcDirBtn = new QPushButton("...", this);

	hbox_input->addWidget(fSrcDirEdt);//, 0, Qt::AlignLeft);
	hbox_input->addWidget(fLoadSrcDirBtn, 0, Qt::AlignRight);
	hbox_input->setSpacing(5);
	vbox->addLayout(hbox_input);

	// structure (sub folder in source path)
	fSubDirLbl = new QLabel("Sub folder", this);
	fSubDirCmb = new QComboBox(this);
	vbox->addWidget(fSubDirLbl, 0, Qt::AlignLeft);
	vbox->addWidget(fSubDirCmb);

	// filter prefix for event and trend data files
	fFilterPrefixLbl = new QLabel("Filter Prefix for EventData and TrendData", this);
	fFilterEventPrefixEdt = new QLineEdit(this);
	fFilterTrendPrefixEdt = new QLineEdit(this);

	vbox->addWidget(fFilterPrefixLbl, 0, Qt::AlignLeft);
	hbox_filter->addWidget(fFilterEventPrefixEdt);
	hbox_filter->addWidget(fFilterTrendPrefixEdt);
	hbox_filter->setSpacing(5);
	vbox->addLayout(hbox_filter);

	// date period
	fDatePeriodLbl = new QLabel("Period", this);
	fDateBeginDtp = new QDateEdit(this);
	fDateEndDtp = new QDateEdit(this);

	fDateBeginDtp->setCalendarPopup(true);
	fDateEndDtp->setCalendarPopup(true);

	QDate date = QDate::currentDate();
	fDateBeginDtp->setDisplayFormat("MMM dd, yyyy");
	fDateEndDtp->setDisplayFormat("MMM dd, yyyy");
	fDateBeginDtp->setDate(date);
	fDateEndDtp->setDate(date);

	hbox_datepicker->setSpacing(10);
	hbox_datepicker->addWidget(fDateBeginDtp);
	hbox_datepicker->addWidget(fDateEndDtp);
	hbox_datepicker->setSpacing(5);
	vbox->addWidget(fDatePeriodLbl, 0, Qt::AlignLeft);
	vbox->addLayout(hbox_datepicker);

	// output settings
	fOutputLbl = new QLabel("Output File", this);
	vbox->addWidget(fOutputLbl, 0, Qt::AlignLeft);

	fFormatROOTRdp = new QRadioButton("ROOT", this);
	fFormatHDF5Rdp = new QRadioButton("HDF5", this);
	fFormatROOTRdp->setChecked(true);
	fFormatHDF5Rdp->setEnabled(false); // disable HDF5 functionality
	fSplitChk = new QCheckBox("Split", this);
	fSplitChk->setCheckState(Qt::Checked);
//	fAppendChk = new QCheckBox("Append", this);
//	fAppendChk->setCheckState(Qt::Unchecked);

	hbox_format->addWidget(fFormatROOTRdp, 0, Qt::AlignLeft);
	hbox_format->addWidget(fFormatHDF5Rdp, 1);
	hbox_format->addWidget(fSplitChk, 0, Qt::AlignRight);
//	hbox_format->addWidget(fAppendChk, 0, Qt::AlignRight);
	vbox->addLayout(hbox_format);

	fOutputFileEdt = new QLineEdit(this);
	fSaveAsBtn = new QPushButton("...", this);

	hbox_output->addWidget(fOutputFileEdt);//, 0, Qt::AlignLeft);
	hbox_output->addWidget(fSaveAsBtn, 0, Qt::AlignRight);
	hbox_output->setSpacing(5);
	vbox->addLayout(hbox_output);

	// log output
	fLogEdt = new QPlainTextEdit(this);
	fLogEdt->setReadOnly(true);

	vbox->addWidget(fLogEdt);//, 0, 0);//, Qt::AlignLeft);

	// controlling buttons.
	fConvertBtn = new QPushButton("Convert", this);
	fCancelBtn = new QPushButton("Cancel", this);
	fQuitBtn = new QPushButton("Quit", this);

	hbox_buttons->addWidget(fConvertBtn, 1, Qt::AlignRight); //  The first parameter is the child widget. The second parameter is the stretch factor, and the last parameter is alignment.
	hbox_buttons->addWidget(fCancelBtn, 0);
	hbox_buttons->addWidget(fQuitBtn, 0);

	//vbox->setSpacing(20); // put some little space among our buttons.s
	vbox->addLayout(hbox_buttons);

	connect(fLoadSrcDirBtn, &QPushButton::clicked, this, &QXboxFileConverter::onLoadSrcDir);
	connect(fSrcDirEdt, &QLineEdit::textChanged, this, &QXboxFileConverter::onSrcDirEdtTextChanged);

	connect(fSubDirCmb, static_cast<void(QComboBox::*)(const QString &)>(&QComboBox::activated),
	      this, &QXboxFileConverter::onSubDirCmbItemhanged);

	connect(fFilterEventPrefixEdt, &QLineEdit::textChanged, this, &QXboxFileConverter::onFilterEdtTextChanged);
	connect(fFilterTrendPrefixEdt, &QLineEdit::textChanged, this, &QXboxFileConverter::onFilterEdtTextChanged);

	connect(fSaveAsBtn, &QPushButton::clicked, this, &QXboxFileConverter::onSaveAs);
	connect(fConvertBtn, &QPushButton::clicked, this, &QXboxFileConverter::onConvert);
	connect(fCancelBtn, &QPushButton::clicked, this, &QXboxFileConverter::onCancel);
	connect(fQuitBtn, &QPushButton::clicked, qApp, &QApplication::quit);
	connect(fFormatROOTRdp, &QRadioButton::clicked, this, &QXboxFileConverter::onFormatChange);
	connect(fFormatHDF5Rdp, &QRadioButton::clicked, this, &QXboxFileConverter::onFormatChange);

	// lookup in the predefined source directories and load sub folders if exist
	for(QDir dir: fgPredSrcDirs) {
		if(dir.exists()) {
			fSrcDir = dir;
			fSrcDirEdt->setText(fSrcDir.path());
			fSubDirCmb->clear();
			for(QString subfolder: getCurrentSubDirectories())
				fSubDirCmb->addItem(subfolder);

			break;
		}
	}

	// update filter and date settings according to the default path
	updateFilterSettings();
	updateDateSettings();
}

QXboxFileConverter::~QXboxFileConverter()
{
}

void QXboxFileConverter::onSaveAs() {
	QString filepath = QFileDialog::getSaveFileName(this,
		    "Save File", QDir::currentPath(), "ROOT (*.root);;HDF5 (*.h5);;All Files (*)");
	setOutputFile(filepath);
}

void QXboxFileConverter::onLoadSrcDir() {
	QDir dir = fSrcDirEdt->text();
	if(!dir.exists())
		dir = QDir::currentPath();
	dir = QFileDialog::getExistingDirectory(this, "Load Directory", dir.path());

	if(!dir.isEmpty()) {
		fSrcDir = dir;
		fSrcDirEdt->setText(fSrcDir.path());
		fSubDirCmb->clear();
		for(QString subfolder: getCurrentSubDirectories())
			fSubDirCmb->addItem(subfolder);
	}
}

void QXboxFileConverter::onSrcDirEdtTextChanged(const QString &text) {
	QDir dir = text;
	fSrcDir = dir;
	fSrcDirEdt->setText(fSrcDir.path());
	fSubDirCmb->clear();
	for(QString subfolder: getCurrentSubDirectories())
		fSubDirCmb->addItem(subfolder);

	updateFilterSettings();
	updateDateSettings();
}

void QXboxFileConverter::onSubDirCmbItemhanged(const QString &text) {

	updateFilterSettings();
	updateDateSettings();
}

void QXboxFileConverter::onFilterEdtTextChanged(const QString &text) {

	updateDateSettings();
}

void QXboxFileConverter::onFormatChange() {
	QString filepath = fOutputFileEdt->text();
	if(!filepath.isEmpty())
		setOutputFile(filepath);
}

void QXboxFileConverter::setOutputFile(const QString &name) {
	QString filepath;
	QFileInfo fileinfo(name);
	QString basename = fileinfo.baseName();

	if(!basename.isEmpty()) {
		if(fFormatROOTRdp->isChecked())
			filepath = QDir(fileinfo.path()).filePath(basename + ".root");
		else if(fFormatHDF5Rdp->isChecked())
			filepath = QDir(fileinfo.path()).filePath(basename + ".h5");
	}

	fOutputFileEdt->setText(filepath);
}

std::vector<QString> QXboxFileConverter::getCurrentSubDirectories() {
	QString info;

	std::vector<QString> subfolders;

	if (!fSrcDir.exists()) {
		//info = "Directory does not exist: %1\n";
		//fLogEdt->insertPlainText (info.arg(fSrcDir.path()));
	}
	else {
		//info = "Found source directory: %1\n";
		//fLogEdt->insertPlainText (info.arg(fSrcDir.path()));

		//fSrcDir.setFilter(QDir::AllDirs | QDir::NoDotAndDotDot); // The setFilter() method specifies the kind of files that should be returned by the entryInfoList() method.
		fSrcDir.setFilter(QDir::AllDirs | QDir::NoDotDot); // The setFilter() method specifies the kind of files that should be returned by the entryInfoList() method.
		fSrcDir.setSorting(QDir::Name | QDir::IgnoreCase); // The setSorting() method specifies the sort order used by the entryInfoList() method.
		QFileInfoList list = fSrcDir.entryInfoList();
		foreach (QFileInfo entry, list)
			subfolders.push_back(entry.fileName());
	}

	return subfolders;
}


int QXboxFileConverter::getPeriodFromFiles(QDate &datebegin, QDate &dateend, const QStringList &files) {

	// the filename must contain 8 subsequent digits to identify the date
	QRegExp rxdate("(\\d{8})");

	QStringList sdates;
	foreach(QString filename, files) {
		int npos = rxdate.indexIn(filename);
		if(npos != -1)
			sdates.push_back(filename.mid(npos, 8));
	}
	sdates.sort();
	datebegin = QDate::fromString(sdates.front(), "yyyyMMdd");
	dateend = QDate::fromString(sdates.back(), "yyyyMMdd");

	return 0;
}


void QXboxFileConverter::onConvert() {
	QString info;
	QDir sourceDir = QDir(fSrcDir).filePath(fSubDirCmb->currentText());
	QDir targetDir = QFileInfo(fOutputFileEdt->text()).absoluteDir();
	QString sourcePrefix = fFilterEventPrefixEdt->text();
	QString targetPrefix = QFileInfo(fOutputFileEdt->text()).completeBaseName();
	QDate date = fDateBeginDtp->date();

	fConvertBtn->setEnabled(false);

	if (fSplitChk->checkState() == Qt::Checked) {

		while(date <= fDateEndDtp->date()) {
			QString sourceBaseName = sourcePrefix + date.toString("yyyyMMdd");
			QString sourceFilePath = sourceDir.filePath(sourceBaseName + ".tdms");

			info = "Processing file: " + sourceFilePath + "... ";
			fLogEdt->insertPlainText(info);

			XBOX::XboxFileConverter converter;
			converter.addFile(sourceFilePath.toStdString()); // special character are not supported by QString -> std string conversion

			QString targetFilePath = targetDir.filePath(
					targetPrefix + "_" + date.toString("yyyyMMdd") + ".root");
			converter.write(targetFilePath.toStdString(), "RECREATE");

			date = date.addDays(1);
		}
	}
	else {

		XBOX::XboxFileConverter converter;
		while(date <= fDateEndDtp->date()) {
			QString sourceBaseName = sourcePrefix + date.toString("yyyyMMdd");
			QString sourceFilePath = sourceDir.filePath(sourceBaseName + ".tdms");

			info = "Add file: " + sourceFilePath + "... ";
			fLogEdt->insertPlainText(info);

			converter.addFile(sourceFilePath.toStdString()); // special character are not supported by QString -> std string conversion

			date = date.addDays(1);
		}
		QString targetFilePath = targetDir.filePath(targetPrefix + ".root");
		converter.write(targetFilePath.toStdString(), "RECREATE");
	}

	info = "DONE\n";
	fLogEdt->insertPlainText(info);

	fConvertBtn->setEnabled(true);
}

void QXboxFileConverter::updateFilterSettings() {

	QDir dir = fSrcDir.filePath(fSubDirCmb->currentText());
	dir.setFilter(QDir::Files);
	dir.setSorting(QDir::Name);

	dir.setNameFilters(QStringList() << fgPredFilterEventPrefix + "*.tdms");
	QStringList eventfiles = dir.entryList();
	dir.setNameFilters(QStringList() << fgPredFilterTrendPrefix + "*.tdms");
	QStringList trendfiles = dir.entryList();

	// the filename must contain 8 subsequent digits to identify the date
	QRegExp rxdate("(\\d{8})");

	if(eventfiles.length()){

		int npos = rxdate.indexIn(eventfiles[0]); // check only first file
		QString prefix = eventfiles[0].left(npos);
		fFilterEventPrefixEdt->setText(prefix);
	}
	else {
		fFilterEventPrefixEdt->setText(fgPredFilterEventPrefix);
	}
	if(trendfiles.length()){

		int npos = rxdate.indexIn(trendfiles[0]); // check only first file
		QString prefix = trendfiles[0].left(npos);
		fFilterTrendPrefixEdt->setText(prefix);
	}
	else {
		fFilterTrendPrefixEdt->setText(fgPredFilterTrendPrefix);
	}
}

void QXboxFileConverter::updateDateSettings() {

	// get current list of event and trend files according to the prefix file names
	QDir dir = fSrcDir.filePath(fSubDirCmb->currentText());
	dir.setFilter(QDir::Files);
	dir.setSorting(QDir::Name);

	dir.setNameFilters(QStringList() << fFilterEventPrefixEdt->text() + "*.tdms");
	QStringList eventfiles = dir.entryList();
	dir.setNameFilters(QStringList() << fFilterTrendPrefixEdt->text() + "*.tdms");
	QStringList trendfiles = dir.entryList();

	if (eventfiles.length()) {

		QDate datebegin;
		QDate dateend;
		getPeriodFromFiles(datebegin, dateend, eventfiles);

		fDateBeginDtp->setMinimumDate(datebegin);
		fDateBeginDtp->setMaximumDate(dateend);
		fDateBeginDtp->setDate(datebegin);
		fDateBeginDtp->setEnabled(true);
		fDateEndDtp->setMinimumDate(datebegin);
		fDateEndDtp->setMaximumDate(dateend);
		fDateEndDtp->setDate(dateend);
		fDateEndDtp->setEnabled(true);
	}
	else {
		fDateBeginDtp->setMinimumDate(QDate::currentDate());
		fDateBeginDtp->setMaximumDate(QDate::currentDate());
		fDateBeginDtp->setDate(QDate::currentDate());
		fDateBeginDtp->setEnabled(false);
		fDateEndDtp->setMinimumDate(QDate::currentDate());
		fDateEndDtp->setMaximumDate(QDate::currentDate());
		fDateEndDtp->setDate(QDate::currentDate());
		fDateEndDtp->setEnabled(false);
	}
}



void QXboxFileConverter::onCancel() {

	updateFilterSettings();
	updateDateSettings();
}

void QXboxFileConverter::keyPressEvent(QKeyEvent *event) {

   if (event->key() == Qt::Key_Escape) {
       qApp->quit();
   }
}
