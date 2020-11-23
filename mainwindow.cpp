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

    /* insert QChartView plotters in ui */

    QHBoxLayout *ly1 = new QHBoxLayout(ui->sealedVolumePlotWidget);
    sealedPlot = new Plot(tr("Sealed volume frequency response"), ui->sealedVolumePlotWidget);
    sealedPlot->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Ignored);
    ly1->insertWidget(0, sealedPlot);

    QHBoxLayout *ly2 = new QHBoxLayout(ui->portedPlotWidget);
    portedPlot = new Plot(tr("Ported volume frequency response"), ui->portedPlotWidget);
    portedPlot->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Ignored);
    ly2->insertWidget(0, portedPlot);

    QHBoxLayout *ly3 = new QHBoxLayout(ui->bandpassPlotWidget);
    bandpassPlot = new Plot(tr("Bandpass volumes frequency response"), ui->bandpassPlotWidget);
    bandpassPlot->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Ignored);
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
    ImportExport::restoreProject(currentSpeaker, currentSealedBox, currentPortedBox, currentBandPassBox, &currentSpeakerNumber, &currentTabIndex);
    projectSaved = true;

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
    projectSaved = true;

    ImportExport::saveProject(currentSpeaker, currentSealedBox, currentPortedBox, currentBandPassBox, currentSpeakerNumber, currentTabIndex);
}

void MainWindow::onProjectExport()
{
    QString home = MainWindow::getHome();
    QString fileName = QFileDialog::getSaveFileName(this, tr("Export File"), home + tr("/untitled.qsp"), tr("QSpeakers project (*.qsp)"));

    QFile file(fileName);
    ImportExport::exportProject(file, currentSpeaker, currentSealedBox, currentPortedBox, currentBandPassBox, currentSpeakerNumber, currentTabIndex);
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

    /* clear undo history */
    this->commandStack->clear();

    unlinkInternals();
    unlinkTabs();

    QFile file(fileName);
    ImportExport::importProject(currentSpeaker, currentSealedBox, currentPortedBox, currentBandPassBox, &currentSpeakerNumber, &currentTabIndex, file);
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
    ui->portedPortSlotWidthButton->setChecked(box.getSlotPortActivated());
    ui->portedPortSlotWidthButton->clicked(box.getSlotPortActivated());
    ui->portedPortSlotWidthDoubleSpinBox->setValue(box.getSlotWidth());
    ui->portedPortSlotHeightLineEdit->setText(QString::number(box.getSlotHeight(), 'f', 2));
    ui->portedPortLengthLineEdit->setText(QString::number(box.getPortLen(), 'f', 2));

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
    ui->bandPassPortLengthLineEdit->setText(QString::number(box.getPortedBoxPortLen(), 'f', 2));

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

    QRect page = printer->pageRect();

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
    connect(bandpassDialog, SIGNAL(optimizeRequested(double, double)), this, SLOT(onBandpassOptimizeRequested(double, double)));
    connect(bandpassDialog, SIGNAL(optimizeCancelled()), this, SLOT(onBandpassOptimizeCancelled()));
    bandpassDialog->show();
}

void MainWindow::onBandpassOptimizeRequested(double s, double pa)
{
    if (ui->tabWidget->currentWidget() == ui->bandpassTab) {
        Optimizer optimizer(currentSpeaker, &currentBandPassBox, currentSpeakerNumber, this);
        optimizer.bandpassAlignS_Pa(s, pa);
        emit currentBandPassBoxChanged(currentBandPassBox);
    }

    disconnect(bandpassDialog, SIGNAL(optimizeRequested(double,double)), this, SLOT(onBandpassOptimizeRequested(double,double)));
    disconnect(bandpassDialog, SIGNAL(optimizeCancelled()), this, SLOT(onBandpassOptimizeCancelled()));
    bandpassDialog->close();
    bandpassDialog->deleteLater();
    bandpassDialog = nullptr;
}

void MainWindow::onBandpassOptimizeCancelled()
{
    disconnect(bandpassDialog, SIGNAL(optimizeRequested(double,double)), this, SLOT(onBandpassOptimizeRequested(double,double)));
    disconnect(bandpassDialog, SIGNAL(optimizeCancelled()), this, SLOT(onBandpassOptimizeCancelled()));
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
    exportPlot(fileName, currentTabIndex);
}

void MainWindow::on3DScadExport()
{
    QString home = getHome();
    QString box = ui->tabWidget->currentWidget()->objectName().replace("Tab", "Box");
    QString f = QString("QSpeakers %1 %2 3D").arg(currentSpeaker.getModel()).arg(box);
    f.replace(' ', '_');
    QString fileName = QFileDialog::getSaveFileName(this, tr("Export for 3D OpenSCAD"), home + QDir::separator() + f + ".scad", tr("OpenSCAD script (*.scad)"));
    exportScad3D(fileName, currentTabIndex);
}

void MainWindow::on2DScadExport()
{
   QString home = getHome();
   QString box = ui->tabWidget->currentWidget()->objectName().replace("Tab", "Box");
   QString f = QString("QSpeakers %1 %2 2D").arg(currentSpeaker.getModel()).arg(box);
   f.replace(' ', '_');
   QString fileName = QFileDialog::getSaveFileName(this, tr("Export for 2D OpenSCAD"), home + QDir::separator() + f + ".scad", tr("OpenSCAD script (*.scad)"));
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

    QVector<QPointF> points = series->pointsVector();
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
#define MARGIN 5 /* cm */
#define WOODTHICK 2 /* cm */
#define SAWTHICK 0.1 /* cm */

void MainWindow::exportScad(const QString& scad, const QString &outfileName, int tabindex)
{
    QFile templ(scad);
    templ.open(QIODevice::ReadOnly);
    QString s = templ.readAll();

    s.replace("__MODEL__", currentSpeaker.getModel());
    s.replace("__NUMBER__", QString::number(currentSpeakerNumber));
    s.replace("__MARGIN__", QString::number(MARGIN));
    s.replace("__DIAMETER__", QString::number(currentSpeaker.getDia() * 100));
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
        s.replace("__PORTEDBOXPORTSLOTWIDTH__", "0"); /* not supported yet */
        s.replace("__PORTEDBOXPORTSLOTHEIGHT__", "0"); /* not supported yet */
    }

    if (tabindex == 1) {
        if (currentPortedBox.getSlotPortActivated()) {
            s.replace("__PORTEDBOXPORTSLOTACTIVATED__", "true");
            s.replace("__PORTEDBOXPORTSLOTWIDTH__", QString::number(currentPortedBox.getSlotWidth()));
            s.replace("__PORTEDBOXPORTSLOTHEIGHT__", QString::number(currentPortedBox.getSlotHeight()));
        } else {
            s.replace("__PORTEDBOXPORTSLOTACTIVATED__", "false");
            s.replace("__PORTEDBOXPORTSLOTWIDTH__", "0");
            s.replace("__PORTEDBOXPORTSLOTHEIGHT__", "0");
        }
        s.replace("__PORTEDBOXPORTDIAMETER__", QString::number(currentPortedBox.getPortDiam()));
        s.replace("__PORTEDBOXPORTLENGTH__", QString::number(currentPortedBox.getPortLen()));
    } else if (tabindex == 2) {
        s.replace("__PORTEDBOXPORTSLOTACTIVATED__", "false");
        s.replace("__PORTEDBOXPORTDIAMETER__", QString::number(currentBandPassBox.getPortedBoxPortDiam()));
        s.replace("__PORTEDBOXPORTLENGTH__", QString::number(currentBandPassBox.getPortedBoxPortLen()));
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
        scad = "../qtcharts/sealedbox_template.scad";
        if (!QFileInfo::exists(scad))
#ifdef __mswin
            scad = (QCoreApplication::applicationDirPath() + QDir::separator() + "sealedbox_template.scad");
#else
            scad = (DATADIR "/"  TARGET  "/sealedbox_template.scad");
#endif
        break;
    case 1: /* vented box */
        /* non-prod version: */
        scad = "../qtcharts/portedbox_template.scad";
        if (!QFileInfo::exists(scad))
#ifdef __mswin
            scad = (QCoreApplication::applicationDirPath() + QDir::separator() + "sealedbox_template.scad");
#else
            scad = (DATADIR "/"  TARGET  "/portedbox_template.scad");
#endif
        break;
    case 2: /* bandpass box */
        /* non-prod version: */
        scad = "../qtcharts/bandpassbox_template.scad";
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
        scad = "../qtcharts/sealedbox_cutting_template.scad";
        if (!QFileInfo::exists(scad))
#ifdef __mswin
            scad = (QCoreApplication::applicationDirPath() + QDir::separator() + "sealedbox_cutting_template.scad");
#else
            scad = (DATADIR "/"  TARGET  "/sealedbox_cutting_template.scad");
#endif
        break;
    case 1: /* vented box */
        /* non-prod version: */
        scad = "../qtcharts/portedbox_cutting_template.scad";
        if (!QFileInfo::exists(scad))
#ifdef __mswin
            scad = (QCoreApplication::applicationDirPath() + QDir::separator() + "sealedbox_cutting_template.scad");
#else
            scad = (DATADIR "/"  TARGET  "/portedbox_cutting_template.scad");
#endif
        break;
    case 2: /* bandpass box */
        /* non-prod version: */
        scad = "../qtcharts/bandpassbox_cutting_template.scad";
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
    connect(ui->actionProjectSave, SIGNAL(triggered()), this, SLOT(onProjectSave()));
    connect(ui->actionProjectQuit, SIGNAL(triggered()), this, SLOT(onProjectQuit()));
    connect(ui->actionSpeakerNew, SIGNAL(triggered()), this, SLOT(onSpeakerNew()));
    connect(ui->actionSpeakerModify, SIGNAL(triggered()), this, SLOT(onSpeakerModify()));
    connect(ui->actionSpeakerRemove, SIGNAL(triggered()), this, SLOT(onSpeakerRemove()));
    connect(ui->actionProjectExport, SIGNAL(triggered()), this, SLOT(onProjectExport()));
    connect(ui->actionProjectImport, SIGNAL(triggered()), this, SLOT(onProjectImport()));
    connect(ui->actionEditOptimize, SIGNAL(triggered()), this, SLOT(onEditOptimize()));
    connect(ui->actionSpeakerSearch, SIGNAL(triggered()), this, SLOT(onSpeakerSearch()));
    connect(ui->actionProjectPrint, SIGNAL(triggered()), this, SLOT(onProjectPrint()));
    connect(ui->actionUndo, SIGNAL(triggered()), this, SLOT(onUndo()));
    connect(ui->actionRedo, SIGNAL(triggered()), this, SLOT(onRedo()));
    connect(ui->actionAbout, SIGNAL(triggered()), this, SLOT(onAboutAbout()));

    /* ported alignments sub-menu */
    connect(ui->actionModerate_Inf, SIGNAL(triggered()), this, SLOT(onAlignModerate_Inf()));
    connect(ui->actionLegendre, SIGNAL(triggered()), this, SLOT(onAlignLegendre()));
    connect(ui->actionBessel, SIGNAL(triggered()), this, SLOT(onAlignBessel()));
    connect(ui->actionBullock, SIGNAL(triggered()), this, SLOT(onAlignBullock()));
    connect(ui->actionKeele_Hoge, SIGNAL(triggered()), this, SLOT(onAlignKeele_Hoge()));

    /* bandpass alignment */
    connect(ui->actionBandpass_alignment, SIGNAL(triggered()), this, SLOT(onBandpassAlignment()));

    /* exports menu */
    connect(ui->actionCurve_Plot, SIGNAL(triggered()), this, SLOT(onCurvePlot()));
    connect(ui->action3D_OpenScad, SIGNAL(triggered()), this, SLOT(on3DScadExport()));
    connect(ui->action2D_OpenScad, SIGNAL(triggered()), this, SLOT(on2DScadExport()));
}

void MainWindow::linkTabs()
{
    connect(ui->tabWidget, SIGNAL(currentChanged(int)), this, SLOT(onCurrentTabChanged(int)));

    /* spk combos action */
    connect(ui->vendorComboBox, SIGNAL(activated(QString)), this, SLOT(onVendorChanged(QString)));
    connect(ui->modelComboBox, SIGNAL(activated(QString)), this, SLOT(onModelChanged(QString)));

    /* drivers number spin action */
    connect(ui->numberSpinBox, SIGNAL(valueChanged(int)), this, SLOT(onNumberSpinChanged(int)));

    connect(ui->sealedVolumeDoubleSpinBox, SIGNAL(valueChanged(double)), this, SLOT(onSealedVolumeDoubleSpinChanged(double)));

    connect(ui->portedVolumeDoubleSpinBox, SIGNAL(valueChanged(double)), this, SLOT(onPortedVolumeDoubleSpinChanged(double)));
    connect(ui->portedResonancedoubleSpinBox, SIGNAL(valueChanged(double)), this, SLOT(onPortedResonanceDoubleSpinChanged(double)));
    connect(ui->portedPortsNumberSpinBox, SIGNAL(valueChanged(int)), this, SLOT(onPortedPortsNumberSpinChanged(int)));
    connect(ui->portedPortDiameterDoubleSpinBox, SIGNAL(valueChanged(double)), this, SLOT(onPortedPortDiameterDoubleSpinChanged(double)));
    connect(ui->portedPortSlotWidthButton, SIGNAL(clicked(bool)), this, SLOT(onPortedSlotPortActivated(bool)));
    connect(ui->portedPortSlotWidthDoubleSpinBox, SIGNAL(valueChanged(double)), this, SLOT(onPortedSlotWidthDoubleSpinChanged(double)));

    connect(ui->bandPassSealedVolumeDoubleSpinBox, SIGNAL(valueChanged(double)), this, SLOT(onBandPassSealedVolumeDoubleSpinChanged(double)));
    connect(ui->bandPassPortedVolumeDoubleSpinBox, SIGNAL(valueChanged(double)), this, SLOT(onBandPassPortedVolumeDoubleSpinChanged(double)));
    connect(ui->bandPassPortedResonanceDoubleSpinBox, SIGNAL(valueChanged(double)), this, SLOT(onBandPassPortedResonanceDoubleSpinChanged(double)));
    connect(ui->bandPassPortsNumberSpinBox, SIGNAL(valueChanged(int)), this, SLOT(onBandPassPortNumSpinChanged(int)));
    connect(ui->bandPassPortDiameterDoubleSpinBox, SIGNAL(valueChanged(double)), this, SLOT(onBandPassPortDiameterDoubleSpinChanged(double)));
}

void MainWindow::linkInternals()
{
    connect(this, SIGNAL(currentSpeakerChanged(Speaker)), this, SLOT(onCurrentSpeakerChanged(const Speaker&)));
    connect(this, SIGNAL(currentSealedBoxChanged(SealedBox)), this, SLOT(onCurrentSealedBoxChanged(const SealedBox&)));
    connect(this, SIGNAL(currentPortedBoxChanged(PortedBox)), this, SLOT(onCurrentPortedBoxChanged(const PortedBox&)));
    connect(this, SIGNAL(currentBandPassBoxChanged(BandPassBox)), this, SLOT(onCurrentBandPassBoxChanged(const BandPassBox&)));

    connect(this->commandStack, SIGNAL(canUndoChanged(bool)), ui->actionUndo, SLOT(setEnabled(bool)));
    connect(this->commandStack, SIGNAL(canRedoChanged(bool)), ui->actionRedo, SLOT(setEnabled(bool)));
}

void MainWindow::unlinkMenus()
{
    disconnect(ui->actionProjectSave, SIGNAL(triggered()), this, SLOT(onProjectSave()));
    disconnect(ui->actionProjectQuit, SIGNAL(triggered()), this, SLOT(onProjectQuit()));
    disconnect(ui->actionSpeakerNew, SIGNAL(triggered()), this, SLOT(onSpeakerNew()));
    disconnect(ui->actionSpeakerModify, SIGNAL(triggered()), this, SLOT(onSpeakerModify()));
    disconnect(ui->actionSpeakerRemove, SIGNAL(triggered()), this, SLOT(onSpeakerRemove()));
    disconnect(ui->actionProjectExport, SIGNAL(triggered()), this, SLOT(onProjectExport()));
    disconnect(ui->actionProjectImport, SIGNAL(triggered()), this, SLOT(onProjectImport()));
    disconnect(ui->actionEditOptimize, SIGNAL(triggered()), this, SLOT(onEditOptimize()));
    disconnect(ui->actionSpeakerSearch, SIGNAL(triggered()), this, SLOT(onSpeakerSearch()));
    disconnect(ui->actionProjectPrint, SIGNAL(triggered()), this, SLOT(onProjectPrint()));
    disconnect(ui->actionAbout, SIGNAL(triggered()), this, SLOT(onAboutAbout()));

    /* ported alignments sub-menu */
    disconnect(ui->actionModerate_Inf, SIGNAL(triggered()), this, SLOT(onAlignModerate_Inf()));
    disconnect(ui->actionLegendre, SIGNAL(triggered()), this, SLOT(onAlignLegendre()));
    disconnect(ui->actionBessel, SIGNAL(triggered()), this, SLOT(onAlignBessel()));
    disconnect(ui->actionBullock, SIGNAL(triggered()), this, SLOT(onAlignBullock()));
    disconnect(ui->actionKeele_Hoge, SIGNAL(triggered()), this, SLOT(onAlignKeele_Hoge()));

    /* bandpass alignment */
    disconnect(ui->actionBandpass_alignment, SIGNAL(triggered(bool)), this, SLOT(onBandpassAlignment()));
}

void MainWindow::unlinkTabs()
{
    disconnect(ui->tabWidget, SIGNAL(currentChanged(int)), this, SLOT(onCurrentTabChanged(int)));

    /* spk combos action */
    disconnect(ui->vendorComboBox, SIGNAL(activated(QString)), this, SLOT(onVendorChanged(QString)));
    disconnect(ui->modelComboBox, SIGNAL(activated(QString)), this, SLOT(onModelChanged(QString)));

    disconnect(ui->numberSpinBox, SIGNAL(valueChanged(int)), this, SLOT(onNumberSpinChanged(int)));

    disconnect(ui->sealedVolumeDoubleSpinBox, SIGNAL(valueChanged(double)), this, SLOT(onSealedVolumeDoubleSpinChanged(double)));

    disconnect(ui->portedVolumeDoubleSpinBox, SIGNAL(valueChanged(double)), this, SLOT(onPortedVolumeDoubleSpinChanged(double)));
    disconnect(ui->portedResonancedoubleSpinBox, SIGNAL(valueChanged(double)), this, SLOT(onPortedResonanceDoubleSpinChanged(double)));
    disconnect(ui->portedPortsNumberSpinBox, SIGNAL(valueChanged(int)), this, SLOT(onPortedPortsNumberSpinChanged(int)));
    disconnect(ui->portedPortDiameterDoubleSpinBox, SIGNAL(valueChanged(double)), this, SLOT(onPortedPortDiameterDoubleSpinChanged(double)));
    disconnect(ui->portedPortSlotWidthButton, SIGNAL(clicked(bool)), this, SLOT(onPortedSlotPortActivated(bool)));
    disconnect(ui->portedPortSlotWidthDoubleSpinBox, SIGNAL(valueChanged(double)), this, SLOT(onPortedSlotWidthDoubleSpinChanged(double)));

    disconnect(ui->bandPassSealedVolumeDoubleSpinBox, SIGNAL(valueChanged(double)), this, SLOT(onBandPassSealedVolumeDoubleSpinChanged(double)));
    disconnect(ui->bandPassPortedVolumeDoubleSpinBox, SIGNAL(valueChanged(double)), this, SLOT(onBandPassPortedVolumeDoubleSpinChanged(double)));
    disconnect(ui->bandPassPortedResonanceDoubleSpinBox, SIGNAL(valueChanged(double)), this, SLOT(onBandPassPortedResonanceDoubleSpinChanged(double)));
    disconnect(ui->bandPassPortsNumberSpinBox, SIGNAL(valueChanged(int)), this, SLOT(onBandPassPortNumSpinChanged(int)));
    disconnect(ui->bandPassPortDiameterDoubleSpinBox, SIGNAL(valueChanged(double)), this, SLOT(onBandPassPortDiameterDoubleSpinChanged(double)));
}

void MainWindow::unlinkInternals()
{
    disconnect(this, SIGNAL(currentSpeakerChanged(Speaker)), this, SLOT(onCurrentSpeakerChanged(const Speaker&)));
    disconnect(this, SIGNAL(currentSealedBoxChanged(SealedBox)), this, SLOT(onCurrentSealedBoxChanged(const SealedBox&)));
    disconnect(this, SIGNAL(currentPortedBoxChanged(PortedBox)), this, SLOT(onCurrentPortedBoxChanged(const PortedBox&)));
    disconnect(this, SIGNAL(currentBandPassBoxChanged(BandPassBox)), this, SLOT(onCurrentBandPassBoxChanged(const BandPassBox&)));
}

void MainWindow::onSpeakerNew()
{
    isModifying = false;
    spkDialog = new SpeakerDialog(this);
    connect(spkDialog, SIGNAL(speakerInserted(Speaker)), this, SLOT(onSpeakerInserted(Speaker)));
    connect(spkDialog, SIGNAL(speakerCancelled()), this, SLOT(onSpeakerCancelled()));
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
    connect(spkDialog, SIGNAL(speakerInserted(Speaker)), this, SLOT(onSpeakerInserted(Speaker)));
    connect(spkDialog, SIGNAL(speakerCancelled()), this, SLOT(onSpeakerCancelled()));
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

    disconnect(spkDialog, SIGNAL(speakerInserted(Speaker)), this, SLOT(onSpeakerInserted(Speaker)));
    disconnect(spkDialog, SIGNAL(speakerCancelled()), this, SLOT(onSpeakerCancelled()));
    spkDialog->close();
    spkDialog->deleteLater();
    spkDialog = nullptr;
}

void MainWindow::onSpeakerCancelled()
{
    disconnect(spkDialog, SIGNAL(speakerInserted(Speaker)), this, SLOT(onSpeakerInserted(Speaker)));
    disconnect(spkDialog, SIGNAL(speakerCancelled()), this, SLOT(onSpeakerCancelled()));
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
    connect(searchDialog, SIGNAL(searchRequested(const QString&, double, double)), this, SLOT(onSearchRequested(const QString&, double, double)));
    connect(searchDialog, SIGNAL(searchCancelled()), this, SLOT(onSearchCancelled()));
    searchDialog->show();
}

void MainWindow::onProjectPrint()
{
    QPrinter printer;
    printer.setCreator("QSpeakers");
    printer.setDocName("qspeakers_project");
    printer.setOrientation(QPrinter::Landscape);
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
                                                       "version 3. It is delivered as is in the hope it can be useful, but with no warranty of correctness."));
}

void MainWindow::onSearchRequested(const QString& param, double min, double max)
{
    disconnect(searchDialog, SIGNAL(searchRequested(const QString&, double, double)), this, SLOT(onSearchRequested(const QString&, double, double)));
    disconnect(searchDialog, SIGNAL(searchCancelled()), this, SLOT(onSearchCancelled()));
    searchDialog->close();
    searchDialog->deleteLater();
    searchDialog = nullptr;

    QList<Speaker> speakers = SpeakerDb::getByValue(param, min, max);
    listDialog = new ListDialog(speakers, this);
    connect(listDialog, SIGNAL(speakerItemSelected(QString, const Speaker&)), this, SLOT(onSpeakerItemSelected(QString, const Speaker&)));
    connect(listDialog, SIGNAL(speakerItemCancelled()), this, SLOT(onSpeakerItemCancelled()));
    listDialog->show();
}

void MainWindow::onSearchCancelled()
{
    disconnect(searchDialog, SIGNAL(searchRequested(const QString&, double, double)), this, SLOT(onSearchRequested(const QString&, double, double)));
    disconnect(searchDialog, SIGNAL(searchCancelled()), this, SLOT(onSearchCancelled()));
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

    disconnect(listDialog, SIGNAL(speakerItemSelected(QString, const Speaker&)), this, SLOT(onSpeakerItemSelected(QString, const Speaker&)));
    disconnect(listDialog, SIGNAL(speakerItemCancelled()), this, SLOT(onSpeakerItemCancelled()));
    listDialog->close();
    listDialog->deleteLater();
    listDialog = nullptr;
}

void MainWindow::onSpeakerItemCancelled()
{
    disconnect(listDialog, SIGNAL(speakerItemSelected(QString,const Speaker&)), this, SLOT(onSpeakerItemSelected(QString,const Speaker&)));
    disconnect(listDialog, SIGNAL(speakerItemCancelled()), this, SLOT(onSpeakerItemCancelled()));
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

void MainWindow::onCurrentBandPassBoxChanged(const BandPassBox &box)
{
    if (!currentSpeaker.isValid())
        return;

    projectSaved = false;
    syncUiFromCurrentBandPassBox(box);
}
