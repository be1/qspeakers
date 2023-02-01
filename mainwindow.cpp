#include <iostream>
#include <QtCore>
#include <QDebug>
#include <QMessageBox>
#include <QPrintDialog>
#include <QPrinter>
#include <QPrinterInfo>
#include <QPainter>
#include <QUndoStack>
#if QT_VERSION >= QT_VERSION_CHECK(5,0,0)
#include <QStandardPaths>
#endif
#include <QDesktopServices>

#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "config.h"

#include "speaker.h"
#include "speakerdb.h"
#include "importexport.h"
#include "plot.h"
#include "searchdialog.h"
#include "listdialog.h"
#include "system.h"
#include "optimizer.h"
#include "undocommands.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    projectSaved(false),
    isModifying(false),
    spkDialog(nullptr),
    fileDialog(nullptr),
    searchDialog(nullptr),
    listDialog(nullptr),
    bandpassDialog(nullptr),
    currentSpeakerNumber(0),
    currentTabIndex(-1),
    sealedPlot(nullptr),
    portedPlot(nullptr),
    bandpassPlot(nullptr),
    notInDbSpeaker(nullptr),
    commandStack(new QUndoStack(this))
{
    ui->setupUi(this);

    /* setup recently opened menu */
    for (int i = 0; i < MaxRecentFiles; ++i) {
        recentFileActs[i] = new QAction(this);
        recentFileActs[i]->setVisible(false);
        connect(recentFileActs[i], &QAction::triggered, this, &MainWindow::onOpenRecentActionTriggered);
    }

    QMenu* sub = new QMenu(tr("Recently opened"), ui->menuProject);
    ui->menuProject->insertMenu(ui->actionProjectSave, sub);

    for (int i = 0; i < MaxRecentFiles; ++i)
        sub->addAction(recentFileActs[i]);

    updateRecentFileActions();

    /* setup maximums of spinbox */
    ui->numberSpinBox->setMaximum(std::numeric_limits<int>::max());
    ui->sealedVolumeDoubleSpinBox->setMaximum(std::numeric_limits<double>::max());
    ui->portedPortsNumberSpinBox->setMaximum(std::numeric_limits<int>::max());
    ui->portedVolumeDoubleSpinBox->setMaximum(std::numeric_limits<double>::max());
    ui->portedPortDiameterDoubleSpinBox->setMaximum(std::numeric_limits<double>::max());
    ui->portedPortSlotWidthDoubleSpinBox->setMaximum(std::numeric_limits<double>::max());
    ui->bandPassSealedVolumeDoubleSpinBox->setMaximum(std::numeric_limits<double>::max());
    ui->bandPassPortedVolumeDoubleSpinBox->setMaximum(std::numeric_limits<double>::max());
    ui->bandPassPortsNumberSpinBox->setMaximum(std::numeric_limits<int>::max());
    ui->bandPassPortDiameterDoubleSpinBox->setMaximum(std::numeric_limits<double>::max());
    ui->bandpassPortSlotWidthDoubleSpinBox->setMaximum(std::numeric_limits<double>::max());

    /* fix minimum width */
    setMinimumWidth(800);
    ui->menuBar->setMinimumWidth(800);

    /* insert QChartView plotters in ui */
    QHBoxLayout *ly1 = new QHBoxLayout(ui->sealedVolumePlotWidget);
    sealedPlot = new Plot(tr("Sealed volume frequency response"), ui->sealedVolumePlotWidget);
    sealedPlot->setMinimumSize(400, 300);
    //sealedPlot->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Expanding);
    ly1->insertWidget(0, sealedPlot);

    QHBoxLayout *ly2 = new QHBoxLayout(ui->portedPlotWidget);
    portedPlot = new Plot(tr("Ported volume frequency response"), ui->portedPlotWidget);
    portedPlot->setMinimumSize(400, 300);
    //portedPlot->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Expanding);
    ly2->insertWidget(0, portedPlot);

    QHBoxLayout *ly3 = new QHBoxLayout(ui->bandpassPlotWidget);
    bandpassPlot = new Plot(tr("Bandpass volumes frequency response"), ui->bandpassPlotWidget);
    bandpassPlot->setMinimumSize(400, 300);
    //bandpassPlot->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Expanding);
    ly3->insertWidget(0, bandpassPlot);

    /* test db */
    if (!SpeakerDb::exists() || (SpeakerDb::lastModified() <= SpeakerDb::pkgInstalled())) {
        QFile pkg_db(SpeakerDb::pkgPath());
        if (SpeakerDb::merge(pkg_db)) qDebug() << "merged db";
    }

    /* fill speaker combos */
    ui->vendorComboBox->addItems(SpeakerDb::getVendors());
    ui->vendorComboBox->setSizeAdjustPolicy(QComboBox::SizeAdjustPolicy::AdjustToContents);
    ui->modelComboBox->addItems(SpeakerDb::getModelsByVendor(ui->vendorComboBox->currentText()));
    ui->modelComboBox->setSizeAdjustPolicy(QComboBox::SizeAdjustPolicy::AdjustToContents);

    /* undo history is empty */
    this->ui->actionUndo->setEnabled(false);
    this->ui->actionRedo->setEnabled(false);

    /* restore last saved project... */
    if (ImportExport::restoreProject(currentSpeaker, currentSealedBox, currentPortedBox, currentBandPassBox, &currentSpeakerNumber, &currentTabIndex))
        projectSaved = true;
    else {
        projectSaved = false;
        currentSpeakerNumber = 1;
        currentTabIndex = 0;
    }

    /* on first run, no speaker. reset to the speaker of the combos if needed */
    if (!currentSpeaker.isValid()) {
        currentSpeaker = SpeakerDb::getByVendorAndModel(ui->vendorComboBox->currentText(), ui->modelComboBox->currentText());
    }

    /* display sibling number */
    ui->numberSpinBox->setValue(currentSpeakerNumber);

    /* select restored tab */
    ui->tabWidget->setCurrentIndex(currentTabIndex);

    /* tab index change is not connected yet, do menu modifications manually */
    if (currentTabIndex != 1) {
        /* index != 1 needs to deactivate the ported alignments menu */
        setActivateActions(ui->menuPorted_alignments->actions(), false);
    }
    if (currentTabIndex != 2) {
        /* index != 2 needs to deactivate the bandpass alignment menu */
        ui->actionBandpass_alignment->setEnabled(false);
    }

    syncUiFromCurrentSpeaker(currentSpeaker);
    syncUiFromCurrentSealedBox(currentSealedBox);
    syncUiFromCurrentPortedBox(currentPortedBox);
    syncUiFromCurrentBandPassBox(currentBandPassBox);

    /*
     * connections
     */

    /* menu actions */
    linkMenus();

    /* tabs actions */
    linkTabs();

    /* non-graphic */
    linkInternals();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::onProjectSave()
{

    if(ImportExport::saveProject(currentSpeaker, currentSealedBox, currentPortedBox, currentBandPassBox, currentSpeakerNumber, currentTabIndex)) {
        projectSaved = true;

#if QT_VERSION >= QT_VERSION_CHECK(6,0,0)
        QString prefix = QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation);
#elif QT_VERSION >= QT_VERSION_CHECK(5,0,0)
        QString prefix = QStandardPaths::writableLocation(QStandardPaths::DataLocation);
#else
        QString prefix = QDesktopServices::storageLocation(QDesktopServices::DataLocation);
#endif
        if (!ImportExport::getSavePath().startsWith(prefix))
            setRecentFile(ImportExport::getSavePath(), true);
    }
}

void MainWindow::onProjectExport()
{
    QString home = MainWindow::getHome();
    QString fileName = QFileDialog::getSaveFileName(this, tr("Export File"), home + QDir::separator() + tr("untitled.qsp"), tr("QSpeakers project (*.qsp)"));

    /* user cancelled */
    if (fileName.isEmpty())
        return;

    if (!fileName.endsWith(".qsp"))
        fileName += ".qsp";

    QFile file(fileName);
    if(ImportExport::exportProject(file, currentSpeaker, currentSealedBox, currentPortedBox, currentBandPassBox, currentSpeakerNumber, currentTabIndex)) {
        setRecentFile(fileName, true);
    }
}

void MainWindow::onProjectImport()
{
    if (!projectSaved) {
        QMessageBox::StandardButton pressed =
                QMessageBox::question(this, tr("Project not saved"),
                                      tr("The current project has not been saved. Continue anyway?"),
                                      QMessageBox::Yes|QMessageBox::No);
        if (pressed == QMessageBox::No)
            return;
    }

    QString home = MainWindow::getHome();
    QString fileName = QFileDialog::getOpenFileName(this, tr("Import File"), home, tr("QSpeakers project (*.qsp)"));

    /* user cancelled */
    if (fileName.isEmpty())
        return;

    if (!loadFile(fileName)) {
        QMessageBox::warning(this, tr("Warning"), tr("Could not open project!"));
    }
}

void MainWindow::onOpenRecentActionTriggered()
{
    QAction *action = qobject_cast<QAction *>(sender());
    if (action) {
        if (!loadFile(action->data().toString())) {
            QMessageBox::warning(this, tr("Warning"), tr("Could not open project!"));
        }
    }
}

QString MainWindow::strippedName(const QString& fullFileName)
{
    return QFileInfo(fullFileName).fileName();
}

void MainWindow::updateRecentFileActions()
{
    QSettings settings(SETTINGS_DOMAIN, SETTINGS_APP);
    QStringList files = settings.value("recentFileList").toStringList();

    int numRecentFiles = qMin(files.size(), (int)MaxRecentFiles);

    for (int i = 0; i < numRecentFiles; ++i) {
        QString text = tr("&%1 %2").arg(i + 1).arg(strippedName(files[i]));
        recentFileActs[i]->setText(text);
        recentFileActs[i]->setData(files[i]);
        recentFileActs[i]->setVisible(true);
    }

    for (int j = numRecentFiles; j < MaxRecentFiles; ++j)
        recentFileActs[j]->setVisible(false);
}

void MainWindow::setRecentFile(const QString& fileName, bool ok)
{
    QSettings settings(SETTINGS_DOMAIN, SETTINGS_APP);
    QStringList files = settings.value("recentFileList").toStringList();
    files.removeAll(fileName);

    if (ok)
        files.prepend(fileName);

    while (files.size() > MaxRecentFiles)
        files.removeLast();

    settings.setValue("recentFileList", files);
    updateRecentFileActions();
}

bool MainWindow::loadFile(const QString& fileName)
{
    /* clear undo history */
    this->commandStack->clear();

    unlinkInternals();
    unlinkTabs();

    QFile file(fileName);
    if (!ImportExport::importProject(currentSpeaker, currentSealedBox, currentPortedBox, currentBandPassBox, &currentSpeakerNumber, &currentTabIndex, file)) {
        setRecentFile(fileName, false);

        linkInternals();
        linkTabs();

        return false;
    }

    setRecentFile(fileName, true);

    ImportExport::setSavePath(fileName);
    setWindowFilePath(ImportExport::getSavePath());

    ui->tabWidget->setCurrentIndex(currentTabIndex);

    syncUiFromCurrentSpeaker(currentSpeaker);
    syncUiFromCurrentSealedBox(currentSealedBox);
    syncUiFromCurrentPortedBox(currentPortedBox);
    syncUiFromCurrentBandPassBox(currentBandPassBox);

    linkInternals();
    linkTabs();

    if (notInDbSpeaker)
        onSpeakerModify();

    return true;
}

void MainWindow::onProjectQuit()
{
    if (projectSaved) {
        this->close();
        return;
    }

    if (!currentSpeaker.isValid()) {
        this->close();
        return;
    }

    QMessageBox::StandardButton pressed =
            QMessageBox::question(this, tr("Project not saved"),
                                  tr("The current project has not been saved. Exit anyway?"),
                                  QMessageBox::Yes|QMessageBox::No);
    if (pressed == QMessageBox::Yes)
        this->close();
}

void MainWindow::syncUiFromCurrentSpeaker(const Speaker& spk)
{
    if (!spk.isValid())
        return;

    /* look if vendor is in the db */
    QList<Speaker> speakers = SpeakerDb::getByVendor(spk.getVendor());

    unlinkTabs();
    if (speakers.count() == 0)  {
        /* if this is an unknown vendor, insert in combo */
        ui->vendorComboBox->insertItem(0, spk.getVendor());
        ui->vendorComboBox->setCurrentIndex(0);
        ui->modelComboBox->clear();
        /* this means also a new model */
        ui->modelComboBox->insertItem(0, spk.getModel());
        ui->modelComboBox->setCurrentIndex(0);
        notInDbSpeaker = &spk;
    } else {
        /* select correct vendor */
        ui->vendorComboBox->setCurrentIndex(ui->vendorComboBox->findText(spk.getVendor()));
        /* fill speakers of this vendor */
        ui->modelComboBox->clear();
        ui->modelComboBox->addItems(SpeakerDb::getModelsByVendor(spk.getVendor()));

        int i = 0;
        for (i = 0; i < speakers.count(); i++) {
            Speaker speaker = speakers[i];
            if (speaker.getModel() == spk.getModel()) {
                if (speaker != spk) {
                    /* speaker conflict, renaming */
                    currentSpeaker.setModel(speaker.getModel() + "*");
                    ui->modelComboBox->addItem(spk.getModel());
                    ui->modelComboBox->setCurrentIndex(ui->modelComboBox->findText(spk.getModel()));
                    notInDbSpeaker = &spk;
                } else {
                    /* okay, select correct combo */
                    ui->modelComboBox->setCurrentIndex(ui->modelComboBox->findText(spk.getModel()));
                    notInDbSpeaker = nullptr;
                }

                break;
            }
        }

        if (i == speakers.count()) {
            /* vendor was ok but model was not in DB */
            ui->modelComboBox->addItem(spk.getModel());
            ui->modelComboBox->setCurrentIndex(ui->modelComboBox->findText(spk.getModel()));
            notInDbSpeaker = &spk;
        }
    }

    ui->numberSpinBox->setValue(currentSpeakerNumber);

    /* display speaker's values */
    ui->splValueLabel->setNum(currentSpeaker.getSpl());
    ui->fsValueLabel->setNum(currentSpeaker.getFs());
    ui->qtsValueLabel->setNum(currentSpeaker.getQts());
    ui->vasValueLabel->setNum(currentSpeaker.getVas());
    ui->diaValueLabel->setNum(currentSpeaker.getDia());
    ui->zValueLabel->setNum(currentSpeaker.getZ());
    ui->vcValueLabel->setNum(currentSpeaker.getVc());

    linkTabs();
}

void MainWindow::syncUiFromCurrentSealedBox(const SealedBox& box)
{
    unlinkTabs();
    ui->sealedVolumeDoubleSpinBox->setValue(box.getVolume());

    System s(currentSpeaker, &box, currentSpeakerNumber);
    sealedPlot->clear();
    for (double f = sealedPlot->getXmin(); f < portedPlot->getXmax(); f *= pow(10, 1.0/100.0)) {
        double db = s.response(f);
        sealedPlot->appendPointF(QPointF(f, db));
    }
    sealedPlot->draw3dbVLine();
    linkTabs();
}

void MainWindow::syncUiFromCurrentPortedBox(const PortedBox& box)
{
    unlinkTabs();
    ui->portedVolumeDoubleSpinBox->setValue(box.getBoxVolume());
    ui->portedResonancedoubleSpinBox->setValue(box.getResFreq());
    ui->portedPortsNumberSpinBox->setValue(box.getPortNum());
    ui->portedPortDiameterDoubleSpinBox->setValue(box.getPortDiam());
    bool slotPortActivated = box.getSlotPortActivated();
    ui->portedPortSlotWidthButton->setChecked(slotPortActivated);
    ui->portedPortSlotWidthButton->clicked(slotPortActivated);
    ui->portedPortSlotWidthDoubleSpinBox->setEnabled(slotPortActivated);
    ui->portedPortSlotHeightLineEdit->setEnabled(slotPortActivated);
    ui->portedPortSlotWidthDoubleSpinBox->setValue(box.getSlotWidth());
    ui->portedPortSlotHeightLineEdit->setText(QString::number(box.getSlotHeight(), 'f', 1));
    ui->portedPortLengthLineEdit->setText(QString::number(box.getPortLen(), 'f', 1));

    System s(currentSpeaker, &box, currentSpeakerNumber);
    portedPlot->clear();
    for (double f = portedPlot->getXmin(); f < portedPlot->getXmax(); f *= pow(10, 1.0/100.0)) {
        double db = s.response(f);
        portedPlot->appendPointF(QPointF(f, db));
    }
    portedPlot->draw3dbVLine();
    linkTabs();
}

void MainWindow::syncUiFromCurrentBandPassBox(const BandPassBox& box)
{
    unlinkTabs();
    ui->bandPassSealedVolumeDoubleSpinBox->setValue(box.getSealedBoxVolume());
    ui->bandPassPortedVolumeDoubleSpinBox->setValue(box.getPortedBoxVolume());
    ui->bandPassPortedResonanceDoubleSpinBox->setValue(box.getPortedBoxResFreq());
    ui->bandPassPortsNumberSpinBox->setValue(box.getPortedBoxPortNum());
    ui->bandPassPortDiameterDoubleSpinBox->setValue(box.getPortedBoxPortDiam());
    bool slotPortActivated = box.getPortedBoxSlotPortActivated();
    ui->bandpassPortSlotWidthButton->setChecked(slotPortActivated);
    ui->bandpassPortSlotWidthButton->clicked(slotPortActivated);
    ui->bandpassPortSlotWidthDoubleSpinBox->setEnabled(slotPortActivated);
    ui->bandpassPortSlotHeightLineEdit->setEnabled(slotPortActivated);
    ui->bandpassPortSlotWidthDoubleSpinBox->setValue(box.getPortedBoxSlotWidth());
    ui->bandpassPortSlotHeightLineEdit->setText(QString::number(box.getPortedBoxSlotHeight(), 'f', 1));
    ui->bandPassPortLengthLineEdit->setText(QString::number(box.getPortedBoxPortLen(), 'f', 1));

    System s(currentSpeaker, &box, currentSpeakerNumber);
    bandpassPlot->clear();
    for (double f = bandpassPlot->getXmin(); f < bandpassPlot->getXmax(); f *= pow(10, 1.0/100.0)) {
        double db = s.response(f);
        bandpassPlot->appendPointF(QPointF(f, db));
    }
    bandpassPlot->draw3dbVLine();
    linkTabs();
}

void MainWindow::setCurrentSpeaker(const Speaker &spk)
{
    if ((currentSpeaker.getVendor() == spk.getVendor()) &&
            (currentSpeaker.getModel() == spk.getModel()))
        return;

    currentSpeaker = spk;
    emit currentSpeakerChanged(spk);
}

bool MainWindow::print(QPrinter *printer)
{
    QPainter painter;
    if (!painter.begin(printer))
        return false;

    QRect page = printer->pageLayout().paintRectPixels(printer->resolution());

    qreal step = page.height() / 4.0;

    if (ui->tabWidget->currentWidget() == ui->sealedTab) {
        System s(currentSpeaker, &currentSealedBox, currentSpeakerNumber);
        s.render(&painter, QRectF(page.left(), page.top(), page.width(), step));
        //sealedPlot->setUseOpenGL(false);
        sealedPlot->render(&painter, QRectF(page.left(), page.top() + step, page.width(), page.height() - step));
        //sealedPlot->setUseOpenGL(true);
    } else if (ui->tabWidget->currentWidget() == ui->portedTab) {
        System s(currentSpeaker, &currentPortedBox, currentSpeakerNumber);
        s.render(&painter, QRectF(page.left(), page.top(), page.width(), step));
        //portedPlot->setUseOpenGL(false);
        portedPlot->render(&painter, QRectF(page.left(), page.top() + step, page.width(), page.height() - step));
        //portedPlot->setUseOpenGL(true);
    } else {
        System s(currentSpeaker, &currentBandPassBox, currentSpeakerNumber);
        s.render(&painter, QRectF(page.left(), page.top(), page.width(), step));
        //bandpassPlot->setUseOpenGL(false);
        bandpassPlot->render(&painter, QRectF(page.left(), page.top() + step, page.width(), page.height() - step));
        //bandpassPlot->setUseOpenGL(true);
    }

    painter.end();
    return true;
}

QString MainWindow::getHome()
{
    QString home;
#if QT_VERSION >= QT_VERSION_CHECK(5,0,0)
    home = QStandardPaths::writableLocation(QStandardPaths::HomeLocation);
#else
    home = QDesktopServices::storageLocation(QDesktopServices::HomeLocation);
#endif
    return home;
}

void MainWindow::onBandpassAlignment()
{
    bandpassDialog = new BandpassDialog(this);
    connect(bandpassDialog, &BandpassDialog::optimizeRequested, this, &MainWindow::onBandpassOptimizeRequested);
    connect(bandpassDialog, &BandpassDialog::optimizeCancelled, this, &MainWindow::onBandpassOptimizeCancelled);
    bandpassDialog->show();
}

void MainWindow::onBandpassOptimizeRequested(double s, double pa)
{
    if (ui->tabWidget->currentWidget() == ui->bandpassTab) {
        Optimizer optimizer(currentSpeaker, &currentBandPassBox, currentSpeakerNumber, this);
        optimizer.bandpassAlignS_Pa(s, pa);
        emit currentBandPassBoxChanged(currentBandPassBox);
    }

    disconnect(bandpassDialog, &BandpassDialog::optimizeRequested, this, &MainWindow::onBandpassOptimizeRequested);
    disconnect(bandpassDialog, &BandpassDialog::optimizeCancelled, this, &MainWindow::onBandpassOptimizeCancelled);
    bandpassDialog->close();
    bandpassDialog->deleteLater();
    bandpassDialog = nullptr;
}

void MainWindow::onBandpassOptimizeCancelled()
{
    disconnect(bandpassDialog, &BandpassDialog::optimizeRequested, this, &MainWindow::onBandpassOptimizeRequested);
    disconnect(bandpassDialog, &BandpassDialog::optimizeCancelled, this, &MainWindow::onBandpassOptimizeCancelled);
    bandpassDialog->close();
    bandpassDialog->deleteLater();
    bandpassDialog = nullptr;
}

void MainWindow::onCurvePlot()
{
    QString home = getHome();
    QString box = ui->tabWidget->currentWidget()->objectName().replace("Tab", "Box");
    QString f = QString("QSpeakers %1 %2").arg(currentSpeaker.getModel()).arg(box);
    f.replace(' ', '_');
    QString fileName = QFileDialog::getSaveFileName(this, tr("Export for Gnuplot"), home + QDir::separator() + f + ".dat", tr("Gnuplot data (*.dat)"));

    /* user cancelled */
    if (fileName.isEmpty())
        return;

    exportPlot(fileName, currentTabIndex);
}

void MainWindow::on3DScadExport()
{
    QString home = getHome();
    QString box = ui->tabWidget->currentWidget()->objectName().replace("Tab", "Box");
    QString f = QString("QSpeakers %1 %2 3D").arg(currentSpeaker.getModel()).arg(box);
    f.replace(' ', '_');
    QString fileName = QFileDialog::getSaveFileName(this, tr("Export for 3D OpenSCAD"), home + QDir::separator() + f + ".scad", tr("OpenSCAD script (*.scad)"));

    /* user cancelled */
    if (fileName.isEmpty())
        return;

    exportScad3D(fileName, currentTabIndex);
}

void MainWindow::on2DScadExport()
{
   QString home = getHome();
   QString box = ui->tabWidget->currentWidget()->objectName().replace("Tab", "Box");
   QString f = QString("QSpeakers %1 %2 2D").arg(currentSpeaker.getModel()).arg(box);
   f.replace(' ', '_');
   QString fileName = QFileDialog::getSaveFileName(this, tr("Export for 2D OpenSCAD"), home + QDir::separator() + f + ".scad", tr("OpenSCAD script (*.scad)"));

    /* user cancelled */
    if (fileName.isEmpty())
        return;

   exportScad2D(fileName, currentTabIndex);
}

void MainWindow::exportPlot(const QString& outfileName, int tabindex)
{
    QFile file(outfileName);
    QString f("# QSpeakers plot: ");
    f += currentSpeaker.getModel() + "\n";
    f += "# freq., dB.\n";

    QLineSeries* series;
    switch (tabindex) {
    case 0: /* closed box */
        series = sealedPlot->series();
        break;
    case 1: /* vented box */
        series = portedPlot->series();
        break;
    case 2: /* bandpass box */
        series = bandpassPlot->series();
        break;
    default:
        return;
    }

    QList<QPointF> points = series->points();
    int len = points.length();

    QString line;
    for (int i = 0; i < len; i++) {
        line.clear();
        QPointF p = points.at(i);
        line += QString("%1 %2 ").arg(p.x()).arg(p.y());
        line.chop(1);
        line.append('\n');
        f.append(line);
    }

    file.open(QIODevice::WriteOnly);
    file.write(f.toUtf8());
    file.close();
}

/* FIXME: should be taken from a popup before to export */
#define MARGIN 50 /* mm */
#define WOODTHICK 20 /* mm */
#define SAWTHICK 1 /* mm */

void MainWindow::exportScad(const QString& scad, const QString &outfileName, int tabindex)
{
    QFile templ(scad);
    templ.open(QIODevice::ReadOnly);
    QString s = templ.readAll();

    s.replace("__MODEL__", currentSpeaker.getModel());
    s.replace("__NUMBER__", QString::number(currentSpeakerNumber));
    s.replace("__MARGIN__", QString::number(MARGIN));
    s.replace("__DIAMETER__", QString::number(currentSpeaker.getDia() * 1000)); /* mm */
    s.replace("__WOODTHICK__", QString::number(WOODTHICK));
    s.replace("__SAWTHICK__", QString::number(SAWTHICK));
    if (tabindex == 0) {
        s.replace("__SEALEDBOXVOLUME__", QString::number(currentSealedBox.getVolume()));
    } else if (tabindex == 1) {
        s.replace("__PORTEDBOXVOLUME__", QString::number(currentPortedBox.getBoxVolume()));
        s.replace("__PORTEDBOXPORTNUMBER__", QString::number(currentPortedBox.getPortNum()));
    } else if (tabindex == 2) {
        s.replace("__SEALEDBOXVOLUME__", QString::number(currentBandPassBox.getSealedBoxVolume()));
        s.replace("__PORTEDBOXVOLUME__", QString::number(currentBandPassBox.getPortedBoxVolume()));
        s.replace("__PORTEDBOXPORTNUMBER__", QString::number(currentBandPassBox.getPortedBoxPortNum()));
    }

    if (tabindex == 1) {
        if (currentPortedBox.getSlotPortActivated()) {
            s.replace("__PORTEDBOXPORTSLOTACTIVATED__", "true");
            s.replace("__PORTEDBOXPORTSLOTWIDTH__", QString::number(currentPortedBox.getSlotWidth() * 10)); /* mm */
            s.replace("__PORTEDBOXPORTSLOTHEIGHT__", QString::number(currentPortedBox.getSlotHeight() * 10)); /* mm */
        } else {
            s.replace("__PORTEDBOXPORTSLOTACTIVATED__", "false");
            s.replace("__PORTEDBOXPORTSLOTWIDTH__", "0");
            s.replace("__PORTEDBOXPORTSLOTHEIGHT__", "0");
        }
        s.replace("__PORTEDBOXPORTDIAMETER__", QString::number(currentPortedBox.getPortDiam() * 10)); /* mm */
        s.replace("__PORTEDBOXPORTLENGTH__", QString::number(currentPortedBox.getPortLen() * 10)); /* mm */
    } else if (tabindex == 2) {
        if (currentBandPassBox.getPortedBoxSlotPortActivated()) {
            s.replace("__PORTEDBOXPORTSLOTACTIVATED__", "true");
            s.replace("__PORTEDBOXPORTSLOTWIDTH__", QString::number(currentBandPassBox.getPortedBoxSlotWidth() * 10)); /* mm */
            s.replace("__PORTEDBOXPORTSLOTHEIGHT__", QString::number(currentBandPassBox.getPortedBoxSlotHeight() * 10)); /* mm */
        } else {
            s.replace("__PORTEDBOXPORTSLOTACTIVATED__", "false");
            s.replace("__PORTEDBOXPORTSLOTWIDTH__", "0");
            s.replace("__PORTEDBOXPORTSLOTHEIGHT__", "0");
        }
        s.replace("__PORTEDBOXPORTDIAMETER__", QString::number(currentBandPassBox.getPortedBoxPortDiam() * 10)); /* mm */
        s.replace("__PORTEDBOXPORTLENGTH__", QString::number(currentBandPassBox.getPortedBoxPortLen() * 10)); /* mm */
    }

    QFile f(outfileName);
    f.open(QIODevice::WriteOnly);
    f.write(s.toUtf8());
    f.close();
}

void MainWindow::exportScad3D(const QString &outfileName, int tabindex)
{
    QFile file(outfileName);
    QString scad;

    switch (tabindex) {
    case 0: /* closed box */
        /* non-prod version: */
        scad = "../qspeakers/sealedbox_template.scad";
        if (!QFileInfo::exists(scad))
#ifdef __mswin
            scad = (QCoreApplication::applicationDirPath() + QDir::separator() + "sealedbox_template.scad");
#else
            scad = (DATADIR "/"  TARGET  "/sealedbox_template.scad");
#endif
        break;
    case 1: /* vented box */
        /* non-prod version: */
        scad = "../qspeakers/portedbox_template.scad";
        if (!QFileInfo::exists(scad))
#ifdef __mswin
            scad = (QCoreApplication::applicationDirPath() + QDir::separator() + "sealedbox_template.scad");
#else
            scad = (DATADIR "/"  TARGET  "/portedbox_template.scad");
#endif
        break;
    case 2: /* bandpass box */
        /* non-prod version: */
        scad = "../qspeakers/bandpassbox_template.scad";
        if (!QFileInfo::exists(scad))
#ifdef __mswin
            scad = (QCoreApplication::applicationDirPath() + QDir::separator() + "sealedbox_template.scad");
#else
            scad = (DATADIR "/"  TARGET  "/bandpassbox_template.scad");
#endif
        break;
    default:
        return;
    }

    exportScad(scad, outfileName, tabindex);
}


void MainWindow::exportScad2D(const QString &outfileName, int tabindex)
{
    QFile file(outfileName);
    QString scad;

    switch (tabindex) {
    case 0: /* closed box */
        /* non-prod version: */
        scad = "../qspeakers/sealedbox_cutting_template.scad";
        if (!QFileInfo::exists(scad))
#ifdef __mswin
            scad = (QCoreApplication::applicationDirPath() + QDir::separator() + "sealedbox_cutting_template.scad");
#else
            scad = (DATADIR "/"  TARGET  "/sealedbox_cutting_template.scad");
#endif
        break;
    case 1: /* vented box */
        /* non-prod version: */
        scad = "../qspeakers/portedbox_cutting_template.scad";
        if (!QFileInfo::exists(scad))
#ifdef __mswin
            scad = (QCoreApplication::applicationDirPath() + QDir::separator() + "sealedbox_cutting_template.scad");
#else
            scad = (DATADIR "/"  TARGET  "/portedbox_cutting_template.scad");
#endif
        break;
    case 2: /* bandpass box */
        /* non-prod version: */
        scad = "../qspeakers/bandpassbox_cutting_template.scad";
        if (!QFileInfo::exists(scad))
#ifdef __mswin
            scad = (QCoreApplication::applicationDirPath() + QDir::separator() + "sealedbox_cutting_template.scad");
#else
            scad = (DATADIR "/"  TARGET  "/bandpassbox_cutting_template.scad");
#endif
        break;
    default:
        return;
    }

    exportScad(scad, outfileName, tabindex);
}

void MainWindow::linkMenus()
{
    connect(ui->actionProjectSave, &QAction::triggered, this, &MainWindow::onProjectSave);
    connect(ui->actionProjectQuit, &QAction::triggered, this, &MainWindow::onProjectQuit);
    connect(ui->actionSpeakerNew, &QAction::triggered, this, &MainWindow::onSpeakerNew);
    connect(ui->actionSpeakerModify, &QAction::triggered, this, &MainWindow::onSpeakerModify);
    connect(ui->actionSpeakerRemove, &QAction::triggered, this, &MainWindow::onSpeakerRemove);
    connect(ui->actionProjectExport, &QAction::triggered, this, &MainWindow::onProjectExport);
    connect(ui->actionProjectImport, &QAction::triggered, this, &MainWindow::onProjectImport);
    connect(ui->actionEditOptimize, &QAction::triggered, this, &MainWindow::onEditOptimize);
    connect(ui->actionSpeakerSearch, &QAction::triggered, this, &MainWindow::onSpeakerSearch);
    connect(ui->actionProjectPrint, &QAction::triggered, this, &MainWindow::onProjectPrint);
    connect(ui->actionUndo, &QAction::triggered, this, &MainWindow::onUndo);
    connect(ui->actionRedo, &QAction::triggered, this, &MainWindow::onRedo);
    connect(ui->actionAbout, &QAction::triggered, this, &MainWindow::onAboutAbout);

    /* ported alignments sub-menu */
    connect(ui->actionModerate_Inf, &QAction::triggered, this, &MainWindow::onAlignModerate_Inf);
    connect(ui->actionLegendre, &QAction::triggered, this, &MainWindow::onAlignLegendre);
    connect(ui->actionBessel, &QAction::triggered, this, &MainWindow::onAlignBessel);
    connect(ui->actionBullock, &QAction::triggered, this, &MainWindow::onAlignBullock);
    connect(ui->actionKeele_Hoge, &QAction::triggered, this, &MainWindow::onAlignKeele_Hoge);

    /* bandpass alignment */
    connect(ui->actionBandpass_alignment, &QAction::triggered, this, &MainWindow::onBandpassAlignment);

    /* exports menu */
    connect(ui->actionCurve_Plot, &QAction::triggered, this, &MainWindow::onCurvePlot);
    connect(ui->action3D_OpenScad, &QAction::triggered, this, &MainWindow::on3DScadExport);
    connect(ui->action2D_OpenScad, &QAction::triggered, this, &MainWindow::on2DScadExport);
}

void MainWindow::linkTabs()
{
    connect(ui->tabWidget, &QTabWidget::currentChanged, this, &MainWindow::onCurrentTabChanged);

    /* spk combos action */
    connect(ui->vendorComboBox, &QComboBox::textActivated, this, &MainWindow::onVendorChanged);
    connect(ui->modelComboBox, &QComboBox::textActivated, this, &MainWindow::onModelChanged);

    /* drivers number spin action */
    connect(ui->numberSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), this, &MainWindow::onNumberSpinChanged);

    connect(ui->sealedVolumeDoubleSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &MainWindow::onSealedVolumeDoubleSpinChanged);

    connect(ui->portedVolumeDoubleSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &MainWindow::onPortedVolumeDoubleSpinChanged);
    connect(ui->portedResonancedoubleSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &MainWindow::onPortedResonanceDoubleSpinChanged);
    connect(ui->portedPortsNumberSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), this, &MainWindow::onPortedPortsNumberSpinChanged);
    connect(ui->portedPortDiameterDoubleSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &MainWindow::onPortedPortDiameterDoubleSpinChanged);
    connect(ui->portedPortSlotWidthButton, &QPushButton::clicked, this, &MainWindow::onPortedSlotPortActivated);
    connect(ui->portedPortSlotWidthDoubleSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &MainWindow::onPortedSlotWidthDoubleSpinChanged);

    connect(ui->bandPassSealedVolumeDoubleSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &MainWindow::onBandPassSealedVolumeDoubleSpinChanged);
    connect(ui->bandPassPortedVolumeDoubleSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &MainWindow::onBandPassPortedVolumeDoubleSpinChanged);
    connect(ui->bandPassPortedResonanceDoubleSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &MainWindow::onBandPassPortedResonanceDoubleSpinChanged);
    connect(ui->bandPassPortsNumberSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), this, &MainWindow::onBandPassPortNumSpinChanged);
    connect(ui->bandPassPortDiameterDoubleSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &MainWindow::onBandPassPortDiameterDoubleSpinChanged);
    connect(ui->bandpassPortSlotWidthButton, &QPushButton::clicked, this, &MainWindow::onBandPassSlotPortActivated);
    connect(ui->bandpassPortSlotWidthDoubleSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &MainWindow::onBandPassSlotWidthDoubleSpinChanged);
}

void MainWindow::linkInternals()
{
    connect(this, &MainWindow::currentSpeakerChanged, this, &MainWindow::onCurrentSpeakerChanged);
    connect(this, &MainWindow::currentSealedBoxChanged, this, &MainWindow::onCurrentSealedBoxChanged);
    connect(this, &MainWindow::currentPortedBoxChanged, this, &MainWindow::onCurrentPortedBoxChanged);
    connect(this, &MainWindow::currentBandPassBoxChanged, this, &MainWindow::onCurrentBandPassBoxChanged);

    connect(this->commandStack, &QUndoStack::canUndoChanged, ui->actionUndo, &QAction::setEnabled);
    connect(this->commandStack, &QUndoStack::canRedoChanged, ui->actionRedo, &QAction::setEnabled);
}

void MainWindow::unlinkMenus()
{
    disconnect(ui->actionProjectSave, &QAction::triggered, this, &MainWindow::onProjectSave);
    disconnect(ui->actionProjectQuit, &QAction::triggered, this, &MainWindow::onProjectQuit);
    disconnect(ui->actionSpeakerNew, &QAction::triggered, this, &MainWindow::onSpeakerNew);
    disconnect(ui->actionSpeakerModify, &QAction::triggered, this, &MainWindow::onSpeakerModify);
    disconnect(ui->actionSpeakerRemove, &QAction::triggered, this, &MainWindow::onSpeakerRemove);
    disconnect(ui->actionProjectExport, &QAction::triggered, this, &MainWindow::onProjectExport);
    disconnect(ui->actionProjectImport, &QAction::triggered, this, &MainWindow::onProjectImport);
    disconnect(ui->actionEditOptimize, &QAction::triggered, this, &MainWindow::onEditOptimize);
    disconnect(ui->actionSpeakerSearch, &QAction::triggered, this, &MainWindow::onSpeakerSearch);
    disconnect(ui->actionProjectPrint, &QAction::triggered, this, &MainWindow::onProjectPrint);
    disconnect(ui->actionAbout, &QAction::triggered, this, &MainWindow::onAboutAbout);

    /* ported alignments sub-menu */
    disconnect(ui->actionModerate_Inf, &QAction::triggered, this, &MainWindow::onAlignModerate_Inf);
    disconnect(ui->actionLegendre, &QAction::triggered, this, &MainWindow::onAlignLegendre);
    disconnect(ui->actionBessel, &QAction::triggered, this, &MainWindow::onAlignBessel);
    disconnect(ui->actionBullock, &QAction::triggered, this, &MainWindow::onAlignBullock);
    disconnect(ui->actionKeele_Hoge, &QAction::triggered, this, &MainWindow::onAlignKeele_Hoge);

    /* bandpass alignment */
    disconnect(ui->actionBandpass_alignment, &QAction::triggered, this, &MainWindow::onBandpassAlignment);
}

void MainWindow::unlinkTabs()
{
    disconnect(ui->tabWidget, &QTabWidget::currentChanged, this, &MainWindow::onCurrentTabChanged);

    /* spk combos action */
    disconnect(ui->vendorComboBox, &QComboBox::textActivated, this, &MainWindow::onVendorChanged);
    disconnect(ui->modelComboBox, &QComboBox::textActivated, this, &MainWindow::onModelChanged);

    disconnect(ui->numberSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), this, &MainWindow::onNumberSpinChanged);

    disconnect(ui->sealedVolumeDoubleSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &MainWindow::onSealedVolumeDoubleSpinChanged);

    disconnect(ui->portedVolumeDoubleSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &MainWindow::onPortedVolumeDoubleSpinChanged);
    disconnect(ui->portedResonancedoubleSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &MainWindow::onPortedResonanceDoubleSpinChanged);
    disconnect(ui->portedPortsNumberSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), this, &MainWindow::onPortedPortsNumberSpinChanged);
    disconnect(ui->portedPortDiameterDoubleSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &MainWindow::onPortedPortDiameterDoubleSpinChanged);
    disconnect(ui->portedPortSlotWidthButton, &QPushButton::clicked, this, &MainWindow::onPortedSlotPortActivated);
    disconnect(ui->portedPortSlotWidthDoubleSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &MainWindow::onPortedSlotWidthDoubleSpinChanged);

    disconnect(ui->bandPassSealedVolumeDoubleSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &MainWindow::onBandPassSealedVolumeDoubleSpinChanged);
    disconnect(ui->bandPassPortedVolumeDoubleSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &MainWindow::onBandPassPortedVolumeDoubleSpinChanged);
    disconnect(ui->bandPassPortedResonanceDoubleSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &MainWindow::onBandPassPortedResonanceDoubleSpinChanged);
    disconnect(ui->bandPassPortsNumberSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), this, &MainWindow::onBandPassPortNumSpinChanged);
    disconnect(ui->bandPassPortDiameterDoubleSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &MainWindow::onBandPassPortDiameterDoubleSpinChanged);
    disconnect(ui->bandpassPortSlotWidthButton, &QPushButton::clicked, this, &MainWindow::onBandPassSlotPortActivated);
    disconnect(ui->bandpassPortSlotWidthDoubleSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &MainWindow::onBandPassSlotWidthDoubleSpinChanged);
}

void MainWindow::unlinkInternals()
{
    disconnect(this, &MainWindow::currentSpeakerChanged, this, &MainWindow::onCurrentSpeakerChanged);
    disconnect(this, &MainWindow::currentSealedBoxChanged, this, &MainWindow::onCurrentSealedBoxChanged);
    disconnect(this, &MainWindow::currentPortedBoxChanged, this, &MainWindow::onCurrentPortedBoxChanged);
    disconnect(this, &MainWindow::currentBandPassBoxChanged, this, &MainWindow::onCurrentBandPassBoxChanged);
}

void MainWindow::onSpeakerNew()
{
    isModifying = false;
    spkDialog = new SpeakerDialog(this);
    connect(spkDialog, &SpeakerDialog::speakerInserted, this, &MainWindow::onSpeakerInserted);
    connect(spkDialog, &SpeakerDialog::speakerCancelled, this, &MainWindow::onSpeakerCancelled);
    spkDialog->show();
}

void MainWindow::onSpeakerRemove()
{

    QMessageBox::StandardButton pressed =
            QMessageBox::question(this,
                                  tr("Removal confirmation"),
                                  tr("Do you really want to remove this loudspeaker from the database?"),
                                  QMessageBox::Yes|QMessageBox::No);

    if (pressed == QMessageBox::No)
        return;

    QString vendor = currentSpeaker.getVendor();
    QString model = currentSpeaker.getModel();

    int num = SpeakerDb::getModelsByVendor(currentSpeaker.getVendor()).size();
    ui->modelComboBox->removeItem(ui->modelComboBox->currentIndex());
    if (num == 1) {
        ui->vendorComboBox->removeItem(ui->vendorComboBox->currentIndex());
    }

    SpeakerDb::removeByVendorAndModel(vendor, model);

    /* fill speakers of automatically reselected vendor */
    QString v = ui->vendorComboBox->currentText();
    ui->modelComboBox->clear();
    ui->modelComboBox->addItems(SpeakerDb::getModelsByVendor(v));
    /* insert a fake speaker to let the choice to the user */
    ui->modelComboBox->insertItem(0, "?");
    ui->modelComboBox->setCurrentIndex(0);
    setCurrentSpeaker(Speaker());
}

void MainWindow::onSpeakerModify()
{
    isModifying = true;
    spkDialog = new SpeakerDialog(currentSpeaker, this);
    connect(spkDialog, &SpeakerDialog::speakerInserted, this, &MainWindow::onSpeakerInserted);
    connect(spkDialog, &SpeakerDialog::speakerCancelled, this,&MainWindow:: onSpeakerCancelled);
    spkDialog->show();
}

void MainWindow::onSpeakerInserted(Speaker spk)
{
    if (isModifying) {
        int idx;
        idx = ui->vendorComboBox->currentIndex();
        ui->vendorComboBox->setItemText(idx, spk.getVendor());
        idx = ui->modelComboBox->currentIndex();
        ui->modelComboBox->setItemText(idx, spk.getModel());

        currentSpeaker = spk;
        emit currentSpeakerChanged(currentSpeaker);
    } else {
        int idx = ui->vendorComboBox->findText(spk.getVendor());
        if (idx == -1)
            ui->vendorComboBox->addItem(spk.getVendor());
        else if (idx == ui->vendorComboBox->currentIndex())
            ui->modelComboBox->addItem(spk.getModel());
    }

    /* spk has been inserted or modified: remove anyway notInDbSpeaker */
    notInDbSpeaker = nullptr;

    disconnect(spkDialog, &SpeakerDialog::speakerInserted, this, &MainWindow::onSpeakerInserted);
    disconnect(spkDialog, &SpeakerDialog::speakerCancelled, this, &MainWindow::onSpeakerCancelled);
    spkDialog->close();
    spkDialog->deleteLater();
    spkDialog = nullptr;
}

void MainWindow::onSpeakerCancelled()
{
    disconnect(spkDialog, &SpeakerDialog::speakerInserted, this, &MainWindow::onSpeakerInserted);
    disconnect(spkDialog, &SpeakerDialog::speakerCancelled, this, &MainWindow::onSpeakerCancelled);
    spkDialog->close();
    spkDialog->deleteLater();
    spkDialog = nullptr;
}

void MainWindow::onEditOptimize()
{
    if (ui->tabWidget->currentWidget() == ui->sealedTab) {
        Optimizer optimizer(currentSpeaker, &currentSealedBox, currentSpeakerNumber, this);
        optimizer.genericOptimizeBox();
        emit currentSealedBoxChanged(currentSealedBox);
    } else if (ui->tabWidget->currentWidget() == ui->portedTab) {
        Optimizer optimizer(currentSpeaker, &currentPortedBox, currentSpeakerNumber, this);
        optimizer.genericOptimizeBox();
        emit currentPortedBoxChanged(currentPortedBox);
    } else {
        Optimizer optimizer(currentSpeaker, &currentBandPassBox, currentSpeakerNumber, this);
        optimizer.genericOptimizeBox();
        emit currentBandPassBoxChanged(currentBandPassBox);
    }
}

void MainWindow::onAlignModerate_Inf()
{
    if (ui->tabWidget->currentWidget() == ui->portedTab) {
        Optimizer optimzer(currentSpeaker, &currentPortedBox, currentSpeakerNumber, this);
        optimzer.portedAlignModerate_Inf();
        emit currentPortedBoxChanged(currentPortedBox);
    }
}

void MainWindow::onAlignLegendre()
{
    if (ui->tabWidget->currentWidget() == ui->portedTab) {
        Optimizer optimizer(currentSpeaker, &currentPortedBox, currentSpeakerNumber, this);
        optimizer.portedAlignLegendre();
        emit currentPortedBoxChanged(currentPortedBox);
    }
}

void MainWindow::onAlignBessel()
{
    if (ui->tabWidget->currentWidget() == ui->portedTab) {
        Optimizer optimzer(currentSpeaker, &currentPortedBox, currentSpeakerNumber, this);
        optimzer.portedAlignBessel();
        emit currentPortedBoxChanged(currentPortedBox);
    }
}

void MainWindow::onAlignKeele_Hoge()
{
    /* BB4 */
    if (ui->tabWidget->currentWidget() == ui->portedTab) {
        Optimizer optimizer(currentSpeaker, &currentPortedBox, currentSpeakerNumber, this);
        optimizer.portedAlignKeele_Hoge();
        emit currentPortedBoxChanged(currentPortedBox);
    }
}

void MainWindow::onAlignBullock()
{
    /* SBB4 */
    if (ui->tabWidget->currentWidget() == ui->portedTab) {
        Optimizer optimizer(currentSpeaker, &currentPortedBox, currentSpeakerNumber, this);
        optimizer.portedAlignBullock();
        emit currentPortedBoxChanged(currentPortedBox);
    }
}

void MainWindow::onSpeakerSearch()
{
    searchDialog = new SearchDialog(this);
    connect(searchDialog, &SearchDialog::searchRequested, this, &MainWindow::onSearchRequested);
    connect(searchDialog,&SearchDialog::searchCancelled, this, &MainWindow::onSearchCancelled);
    searchDialog->show();
}

void MainWindow::onProjectPrint()
{
    QPrinter printer;
    printer.setCreator("QSpeakers");
    printer.setDocName("qspeakers_project");
    printer.setPageOrientation(QPageLayout::Landscape);
    QPrintDialog dialog(&printer, this);
    if (dialog.exec() == QDialog::Accepted) {
        if (printer.isValid()) {
            if (!print(&printer)) {
                QPrinterInfo info(printer);
                qWarning() << "printerinfo definition null:" << info.isNull();
                qWarning() << "printerinfo state error:" << (info.state() == QPrinter::Error);
            }
        } else {
            qWarning() << "invalid printer object";
        }
    }
}

void MainWindow::onUndo()
{
    if (this->commandStack->canUndo())
        this->commandStack->undo();
}

void MainWindow::onRedo()
{
    if (this->commandStack->canRedo())
        this->commandStack->redo();
}

void MainWindow::onAboutAbout()
{
    QMessageBox::about(this, tr("About QSpeakers"),
                       tr("QSpeakers version " VERSION "\n\n"
                                                       "This program simulates common acoustical enclosures behaviour to help designing loudspeaker systems.\n\n"
                                                       "This program is free software, copyright (C) 2014 Benoit Rouits <brouits@free.fr> and released under the GNU General Public Lisence "
                                                       "version 3. It is delivered as is in the hope it can be useful, but with no warranty at all."));
}

void MainWindow::onSearchRequested(const QString& param, double min, double max)
{
    disconnect(searchDialog, &SearchDialog::searchRequested, this, &MainWindow::onSearchRequested);
    disconnect(searchDialog, &SearchDialog::searchCancelled, this, &MainWindow::onSearchCancelled);
    searchDialog->close();
    searchDialog->deleteLater();
    searchDialog = nullptr;

    QList<Speaker> speakers = SpeakerDb::getByValue(param, min, max);
    listDialog = new ListDialog(speakers, this);
    connect(listDialog, &ListDialog::speakerItemSelected, this, &MainWindow::onSpeakerItemSelected);
    connect(listDialog, &ListDialog::speakerItemCancelled, this, &MainWindow::onSpeakerItemCancelled);
    listDialog->show();
}

void MainWindow::onSearchCancelled()
{
    disconnect(searchDialog, &SearchDialog::searchRequested, this, &MainWindow::onSearchRequested);
    disconnect(searchDialog, &SearchDialog::searchCancelled, this, &MainWindow::onSearchCancelled);
    searchDialog->close();
    searchDialog->deleteLater();
    searchDialog = nullptr;
}

void MainWindow::onSpeakerItemSelected(QString title, const Speaker& speaker)
{
    qDebug() << title;
    if (speaker.isValid()) {
        setCurrentSpeaker(speaker);
        syncUiFromCurrentSpeaker(speaker);
    }

    disconnect(listDialog, &ListDialog::speakerItemSelected, this, &MainWindow::onSpeakerItemSelected);
    disconnect(listDialog, &ListDialog::speakerItemCancelled, this, &MainWindow::onSpeakerItemCancelled);
    listDialog->close();
    listDialog->deleteLater();
    listDialog = nullptr;
}

void MainWindow::onSpeakerItemCancelled()
{
    disconnect(listDialog, &ListDialog::speakerItemSelected, this, &MainWindow::onSpeakerItemSelected);
    disconnect(listDialog, &ListDialog::speakerItemCancelled, this, &MainWindow::onSpeakerItemCancelled);
    listDialog->close();
    listDialog->deleteLater();
    listDialog = nullptr;
}

void MainWindow::setActivateActions(QList<QAction*> actions, bool enable)
{
    for (int i = 0; i < actions.count(); i++) {
        actions[i]->setEnabled(enable);
    }
}

void MainWindow::onCurrentTabChanged(int tab)
{
    if (tab == currentTabIndex)
        return;

    this->commandStack->clear();

    QList<QAction*> alignments = ui->menuPorted_alignments->actions();
    currentTabIndex = tab;

    switch (tab) {
    case 0:
    {
        setActivateActions(alignments, false);
        ui->actionBandpass_alignment->setEnabled(false);

        syncUiFromCurrentSealedBox(currentSealedBox);
        break;
    }
    case 1:
    {
        setActivateActions(alignments, true);
        ui->actionBandpass_alignment->setEnabled(false);

        syncUiFromCurrentPortedBox(currentPortedBox);
        break;
    }
    case 2:
    {
        setActivateActions(alignments, false);
        ui->actionBandpass_alignment->setEnabled(true);

        syncUiFromCurrentBandPassBox(currentBandPassBox);
        break;
    }
    default:
        break;
    }
}

void MainWindow::onNumberSpinChanged(int number) {
    if (currentSpeakerNumber == number)
        return;

    NumberCommand* com = new NumberCommand (currentSpeakerNumber, number, this);
    this->commandStack->push(com);
}

void MainWindow::changeSpeakerNumber(int number) {
    /* order is very important to avoid crash loop */
    currentSpeakerNumber = number;
    ui->numberSpinBox->setValue(number);

    /* the user may have used the "?" fake speaker */
    if (!currentSpeaker.isValid()) {
        qDebug() << "invalid speaker, not computing.";
        return;
    }

    projectSaved = false;
    syncUiFromCurrentSealedBox(currentSealedBox);
    syncUiFromCurrentPortedBox(currentPortedBox);
    syncUiFromCurrentBandPassBox(currentBandPassBox);
}

void MainWindow::onVendorChanged(QString vendor)
{
    if (currentSpeaker.getVendor() == vendor)
        return;

    if (notInDbSpeaker) {
        if (vendor == notInDbSpeaker->getVendor())
            return;

        QMessageBox::StandardButton pressed =
                QMessageBox::question(this, tr("Unsaved speaker"),
                                      tr("This speaker is not present in the database. Would you like to insert it before to continue?"),
                                      QMessageBox::Yes|QMessageBox::No);
        if (pressed == QMessageBox::Yes) {
            SpeakerDb::insertOrReplace(notInDbSpeaker->getVendor(), notInDbSpeaker->getModel(), *notInDbSpeaker);
            notInDbSpeaker = nullptr;
        } else /* no insert wanted */ {
            if (0 == SpeakerDb::getByVendor(notInDbSpeaker->getVendor()).count()) {
                /* forget entries */
                ui->vendorComboBox->removeItem(ui->vendorComboBox->findText(notInDbSpeaker->getVendor()));
                ui->modelComboBox->removeItem(ui->modelComboBox->findText(notInDbSpeaker->getModel()));
            } else {
                /* forget model */
                ui->modelComboBox->removeItem(ui->modelComboBox->findText(notInDbSpeaker->getModel()));
            }
            notInDbSpeaker = nullptr;
        }
    }

    this->commandStack->beginMacro("changing loudspeaker");
    VendorCommand* com = new VendorCommand (currentSpeaker.getVendor(), vendor, currentSpeaker, this);
    this->commandStack->push(com);
}

void MainWindow::changeVendor(const QString& vendor, const Speaker& oldspeaker)
{
    ui->vendorComboBox->setCurrentIndex(ui->vendorComboBox->findText(vendor));

    /* fill speakers of this vendor */
    ui->modelComboBox->clear();
    ui->modelComboBox->addItems(SpeakerDb::getModelsByVendor(vendor));

    /* if we come back to a previous vendor, set last vendor's selected speaker */
    if (oldspeaker.getVendor() == vendor) {
        ui->modelComboBox->setCurrentIndex(ui->modelComboBox->findText(oldspeaker.getModel()));
        setCurrentSpeaker(SpeakerDb::getByVendorAndModel(vendor, oldspeaker.getModel()));
    } else {
        /* insert a fake speaker to let the choice to the user */
        ui->modelComboBox->insertItem(0, "?");
        ui->modelComboBox->setCurrentIndex(0);
        setCurrentSpeaker(Speaker());
    }
}

void MainWindow::onModelChanged(QString model)
{
    if (model.isNull() || model.isEmpty())
        return;

    if (currentSpeaker.getModel() == model)
        return;

    if (notInDbSpeaker) {
        if (model == notInDbSpeaker->getModel())
            return;

        QMessageBox::StandardButton pressed =
                QMessageBox::question(this, tr("Unsaved speaker"),
                                      tr("This speaker is not present in the database. Would you like to insert it before to continue?"),
                                      QMessageBox::Yes|QMessageBox::No);
        if (pressed == QMessageBox::Yes) {
            SpeakerDb::insertOrReplace(notInDbSpeaker->getVendor(), notInDbSpeaker->getModel(), *notInDbSpeaker);
            notInDbSpeaker = nullptr;
        } else /* no insert wanted */ {
            if (0 == SpeakerDb::getByVendor(notInDbSpeaker->getVendor()).count()) {
                /* forget entries */
                ui->vendorComboBox->removeItem(ui->vendorComboBox->findText(notInDbSpeaker->getVendor()));
                ui->modelComboBox->removeItem(ui->modelComboBox->findText(notInDbSpeaker->getModel()));
            } else {
                /* forget model */
                ui->modelComboBox->removeItem(ui->modelComboBox->findText(notInDbSpeaker->getModel()));
            }
            notInDbSpeaker = nullptr;
        }
    }

    oldSpeaker = currentSpeaker;

    ModelCommand* com = new ModelCommand (currentSpeaker.getModel(), model, this);
    this->commandStack->push(com);

    if (oldSpeaker.getVendor() != currentSpeaker.getVendor()) {
        oldSpeaker = currentSpeaker;
        this->commandStack->endMacro();
    }
}

void MainWindow::changeModel(const QString& model)
{
    ui->modelComboBox->setCurrentIndex(ui->modelComboBox->findText(model));

    /* change model */
    if ((ui->modelComboBox->findText("?") == 0) && (ui->modelComboBox->currentIndex() != 0)) {
        /* user choose a speaker; safely remove fake speaker */
        ui->modelComboBox->removeItem(0);
    }

    QString vendor = ui->vendorComboBox->currentText();
    setCurrentSpeaker(SpeakerDb::getByVendorAndModel(vendor, model));
}

void MainWindow::onCurrentSpeakerChanged(const Speaker &spk)
{
    ui->splValueLabel->setNum(spk.getSpl());
    ui->fsValueLabel->setNum(spk.getFs());
    ui->qtsValueLabel->setNum(spk.getQts());
    ui->vasValueLabel->setNum(spk.getVas());
    ui->diaValueLabel->setNum(spk.getDia());
    ui->zValueLabel->setNum(spk.getZ());
    ui->vcValueLabel->setNum(spk.getVc());

    /* the user may have activated the "?" fake speaker */
    if (!spk.isValid()) {
        qDebug() << "invalid speaker, not computing.";
        return;
    }

    projectSaved = false;
    syncUiFromCurrentSealedBox(currentSealedBox);
    syncUiFromCurrentPortedBox(currentPortedBox);
    syncUiFromCurrentBandPassBox(currentBandPassBox);
}

void MainWindow::onSealedVolumeDoubleSpinChanged(double val)
{
    SealedVolumeCommand* com = new SealedVolumeCommand (currentSealedBox.getVolume(), val, this);
    this->commandStack->push(com);
}

void MainWindow::changeSealedVolume(double val)
{
    currentSealedBox.setVolume(val);
    syncUiFromCurrentSealedBox(currentSealedBox);
}

void MainWindow::onCurrentSealedBoxChanged(const SealedBox &box)
{
    if (!currentSpeaker.isValid())
        return;

    projectSaved = false;
    syncUiFromCurrentSealedBox(box);
}

void MainWindow::onPortedVolumeDoubleSpinChanged(double val)
{
    PortedVolumeCommand* com = new PortedVolumeCommand (currentPortedBox.getBoxVolume(), val, this);
    this->commandStack->push(com);
}

void MainWindow::changePortedVolume(double val)
{
    currentPortedBox.setBoxVolume(val);
    currentPortedBox.updatePorts(currentSpeaker.getSd() * currentSpeakerNumber, currentSpeaker.getXmax());
    syncUiFromCurrentPortedBox(currentPortedBox);
}

void MainWindow::onPortedResonanceDoubleSpinChanged(double val)
{
    PortedResFreqCommand* com = new PortedResFreqCommand (currentPortedBox.getResFreq(), val, this);
    this->commandStack->push(com);
}

void MainWindow::changePortedResFreq(double val)
{
    currentPortedBox.setResFreq(val);
    currentPortedBox.updatePorts(currentSpeaker.getSd() * currentSpeakerNumber, currentSpeaker.getXmax());
    syncUiFromCurrentPortedBox(currentPortedBox);
}

void MainWindow::onPortedPortsNumberSpinChanged(int val)
{
    PortedPortNumberCommand* com = new PortedPortNumberCommand (currentPortedBox.getPortNum(), val, this);
    this->commandStack->push(com);
}

void MainWindow::changePortedPortNumber(unsigned int val)
{
    currentPortedBox.setPortNum(val);
    currentPortedBox.updatePorts(currentSpeaker.getSd()* currentSpeakerNumber, currentSpeaker.getXmax());
    syncUiFromCurrentPortedBox(currentPortedBox);
}

void MainWindow::onPortedPortDiameterDoubleSpinChanged(double val)
{
    /* allow manual decrease/increase of port diameter */
    PortedPortDiamCommand* com = new PortedPortDiamCommand (currentPortedBox.getPortDiam(), val, this);
    this->commandStack->push(com);
}

void MainWindow::changePortedPortDiam(double val)
{
    currentPortedBox.setPortDiam(val);
    currentPortedBox.updateSlots();
    currentPortedBox.updatePortsLength();
    syncUiFromCurrentPortedBox(currentPortedBox);
}

void MainWindow::onPortedSlotPortActivated(bool checked)
{
    PortedSlotPortCommand* com = new PortedSlotPortCommand (this->currentPortedBox.getSlotPortActivated(), checked, this);
    this->commandStack->push(com);
}

void MainWindow::changePortedSlotPortActivation(bool checked)
{
    currentPortedBox.setSlotPortActivated(checked);

    ui->portedPortSlotWidthDoubleSpinBox->setEnabled(checked);
    ui->portedPortSlotHeightLineEdit->setEnabled(checked);

    syncUiFromCurrentPortedBox(currentPortedBox);
}

void MainWindow::onPortedSlotWidthDoubleSpinChanged(double val)
{
    PortedSlotWidthCommand* com = new PortedSlotWidthCommand (currentPortedBox.getSlotWidth(), val, this);
    this->commandStack->push(com);
}

void MainWindow::changePortedSlotWidth(double val)
{
    currentPortedBox.setSlotWidth(val);
    syncUiFromCurrentPortedBox(currentPortedBox);
}

void MainWindow::onCurrentPortedBoxChanged(const PortedBox &box)
{
    if (!currentSpeaker.isValid())
        return;

    projectSaved = false;
    syncUiFromCurrentPortedBox(box);
}

void MainWindow::onBandPassSealedVolumeDoubleSpinChanged(double val)
{
    BPSealedVolumeCommand* com = new BPSealedVolumeCommand (currentBandPassBox.getSealedBoxVolume(), val, this);
    this->commandStack->push(com);
}

void MainWindow::changeBPSealedVolume(double val)
{
    currentBandPassBox.setSealedBoxVolume(val);
    syncUiFromCurrentBandPassBox(currentBandPassBox);
}

void MainWindow::onBandPassPortedVolumeDoubleSpinChanged(double val)
{
    BPPortedVolumeCommand* com = new BPPortedVolumeCommand (currentBandPassBox.getPortedBoxVolume(), val, this);
    this->commandStack->push(com);
}

void MainWindow::changeBPPortedVolume(double val)
{
    currentBandPassBox.setPortedBoxVolume(val);
    currentBandPassBox.updatePortedBoxPorts(currentSpeaker.getSd() * currentSpeakerNumber, currentSpeaker.getXmax());
    syncUiFromCurrentBandPassBox(currentBandPassBox);
}

void MainWindow::onBandPassPortedResonanceDoubleSpinChanged(double val)
{
    BPPortedResFreqCommand* com = new BPPortedResFreqCommand (currentBandPassBox.getPortedBoxResFreq(), val, this);
    this->commandStack->push(com);
}

void MainWindow::changeBPPortedResFreq(double val)
{
    currentBandPassBox.setPortedBoxResFreq(val);
    currentBandPassBox.updatePortedBoxPorts(currentSpeaker.getSd() * currentSpeakerNumber, currentSpeaker.getXmax());
    syncUiFromCurrentBandPassBox(currentBandPassBox);
}

void MainWindow::onBandPassPortNumSpinChanged(int val)
{
    BPPortedPortNumberCommand* com = new BPPortedPortNumberCommand (currentBandPassBox.getPortedBoxPortNum(), val, this);
    this->commandStack->push(com);
}

void MainWindow::changeBPPortedPortNumber(double val)
{
    currentBandPassBox.setPortedBoxPortNum(val);
    currentBandPassBox.updatePortedBoxPorts(currentSpeaker.getSd() * currentSpeakerNumber, currentSpeaker.getXmax());
    syncUiFromCurrentBandPassBox(currentBandPassBox);
}

void MainWindow::onBandPassPortDiameterDoubleSpinChanged(double val)
{
    /* allow manual decrease/increase of port diameter */
    BPPortedPortDiamCommand* com = new BPPortedPortDiamCommand (currentBandPassBox.getPortedBoxPortDiam(), val, this);
    this->commandStack->push(com);
}

void MainWindow::changeBPPortedPortDiam(double val)
{
    currentBandPassBox.setPortedBoxPortDiam(val);
    currentBandPassBox.updatePortedBoxPortsLength();
    syncUiFromCurrentBandPassBox(currentBandPassBox);
}

void MainWindow::onBandPassSlotPortActivated(bool checked)
{
    BPPortedSlotPortCommand* com = new BPPortedSlotPortCommand (this->currentBandPassBox.getPortedBoxSlotPortActivated(), checked, this);
    this->commandStack->push(com);
}

void MainWindow::changeBPPortedSlotPortActivation(bool checked)
{
    currentBandPassBox.setPortedBoxSlotPortActivated(checked);
    ui->bandpassPortSlotWidthDoubleSpinBox->setEnabled(checked);
    ui->bandpassPortSlotHeightLineEdit->setEnabled(checked);

    syncUiFromCurrentBandPassBox(currentBandPassBox);
}

void MainWindow::changeBPPortedSlotWidth(double val)
{
    currentBandPassBox.setPortedBoxSlotWidth(val);
    syncUiFromCurrentBandPassBox(currentBandPassBox);
}

void MainWindow::onBandPassSlotWidthDoubleSpinChanged(double val)
{
    BPPortedSlotWidthCommand* com = new BPPortedSlotWidthCommand (currentBandPassBox.getPortedBoxSlotWidth(), val, this);
    this->commandStack->push(com);
}

void MainWindow::onCurrentBandPassBoxChanged(const BandPassBox &box)
{
    if (!currentSpeaker.isValid())
        return;

    projectSaved = false;
    syncUiFromCurrentBandPassBox(box);
}
