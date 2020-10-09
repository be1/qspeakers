#ifndef UNDOCOMMANDS_H
#define UNDOCOMMANDS_H

#include <QUndoCommand>
#include "mainwindow.h"

class GenericCommand: public QUndoCommand
{
public:
    GenericCommand(QObject *parent)
        : manual(true), mainwindow(static_cast<MainWindow*>(parent)) {}
    void undo() override {
        this->manual = false;
    }
    void redo() override {
        this->manual = false;
    }
protected:
    bool manual;
    MainWindow* mainwindow;
};

class NumberCommand: public GenericCommand
{
public:
    NumberCommand(int older, int newer, QObject *parent)
        : GenericCommand(parent) {
      this->older = older;
      this->newer = newer;
    };
    void undo() override {
        this->mainwindow->changeSpeakerNumber(older, manual);
        GenericCommand::undo();
    };
    void redo() override {
        this->mainwindow->changeSpeakerNumber(newer, manual);
        GenericCommand::redo();
    };

private:
    int older;
    int newer;
};

class VendorCommand: public GenericCommand
{
public:
    VendorCommand(const QString& older, const QString& newer, const Speaker& oldspeaker, QObject *parent)
        : GenericCommand(parent) {
      this->oldspeaker = oldspeaker;
      this->older = older;
      this->newer = newer;
    };
    void undo() override {
        this->mainwindow->changeVendor(older, oldspeaker, manual);
        GenericCommand::undo();
    };
    void redo() override {
        this->mainwindow->changeVendor(newer, Speaker(), manual);
        GenericCommand::redo();
    };

private:
    Speaker oldspeaker;
    QString older;
    QString newer;
};

class ModelCommand: public GenericCommand
{
public:
    ModelCommand(const QString& older, const QString& newer, QObject *parent)
        : GenericCommand(parent) {
      this->older = older;
      this->newer = newer;
    };
    void undo() override {
        this->mainwindow->changeModel(older, manual);
        GenericCommand::undo();
    };
    void redo() override {
        this->mainwindow->changeModel(newer, manual);
        GenericCommand::redo();
    };

private:
    QString older;
    QString newer;
};

class SealedVolumeCommand: public GenericCommand
{
public:
    SealedVolumeCommand(double older, double newer, QObject *parent)
        : GenericCommand(parent) {
      this->older = older;
      this->newer = newer;
    };
    void undo() override {
        this->mainwindow->changeSealedVolume(older, manual);
        GenericCommand::undo();
    };
    void redo() override {
        this->mainwindow->changeSealedVolume(newer, manual);
        GenericCommand::redo();
    };

protected:
    double older;
    double newer;
};

class PortedVolumeCommand: public GenericCommand
{
public:
    PortedVolumeCommand(double older, double newer, QObject *parent)
        : GenericCommand(parent) {
      this->older = older;
      this->newer = newer;
    };
    void undo() override {
        this->mainwindow->changePortedVolume(older, manual);
        GenericCommand::undo();
    };
    void redo() override {
        this->mainwindow->changePortedVolume(newer, manual);
        GenericCommand::redo();
    };

protected:
    double older;
    double newer;
};

class PortedResFreqCommand: public GenericCommand
{
public:
    PortedResFreqCommand(double older, double newer, QObject *parent)
        : GenericCommand(parent) {
      this->older = older;
      this->newer = newer;
    };
    void undo() override {
        this->mainwindow->changePortedResFreq(older, manual);
        GenericCommand::undo();
    };
    void redo() override {
        this->mainwindow->changePortedResFreq(newer, manual);
        GenericCommand::redo();
    };

protected:
    double older;
    double newer;
};

class PortedPortNumberCommand: public GenericCommand
{
public:
    PortedPortNumberCommand(unsigned int older, unsigned int newer, QObject *parent)
        : GenericCommand(parent) {
      this->older = older;
      this->newer = newer;
    };
    void undo() override {
        this->mainwindow->changePortedPortNumber(older, manual);
        GenericCommand::undo();
    };
    void redo() override {
        this->mainwindow->changePortedPortNumber(newer, manual);
        GenericCommand::redo();
    };

protected:
    unsigned int older;
    unsigned int newer;
};

class PortedPortDiamCommand: public GenericCommand
{
public:
    PortedPortDiamCommand(double older, double newer, QObject *parent)
        : GenericCommand(parent) {
      this->older = older;
      this->newer = newer;
    };
    void undo() override {
        this->mainwindow->changePortedPortDiam(older, manual);
        GenericCommand::undo();
    };
    void redo() override {
        this->mainwindow->changePortedPortDiam(newer, manual);
        GenericCommand::redo();
    };

protected:
    double older;
    double newer;
};

class PortedSlotPortCommand: public GenericCommand
{
public:
    PortedSlotPortCommand(bool older, bool newer, QObject *parent)
        : GenericCommand(parent) {
      this->older = older;
      this->newer = newer;
    };
    void undo() override {
        this->mainwindow->changePortedSlotPortActivation(older, manual);
        GenericCommand::undo();
    };
    void redo() override {
        this->mainwindow->changePortedSlotPortActivation(newer, manual);
        GenericCommand::redo();
    };

private:
    bool older;
    bool newer;
};

class PortedSlotWidthCommand: public GenericCommand
{
public:
    PortedSlotWidthCommand(double older, double newer, QObject *parent)
        : GenericCommand(parent) {
      this->older = older;
      this->newer = newer;
    };
    void undo() override {
        this->mainwindow->changePortedSlotWidth(older, manual);
        GenericCommand::undo();
    };
    void redo() override {
        this->mainwindow->changePortedSlotWidth(newer, manual);
        GenericCommand::redo();
    };

private:
    double older;
    double newer;
};

class BPSealedVolumeCommand: public SealedVolumeCommand
{
public:
    BPSealedVolumeCommand(double older, double newer, QObject *parent)
        : SealedVolumeCommand(older, newer, parent)
    {};

    void undo() override {
        this->mainwindow->changeBPSealedVolume(this->older, manual);
        GenericCommand::undo();
    };
    void redo() override {
        this->mainwindow->changeBPSealedVolume(this->newer, manual);
        GenericCommand::redo();
    };
};

class BPPortedVolumeCommand: public PortedVolumeCommand
{
public:
    BPPortedVolumeCommand(double older, double newer, QObject *parent)
        :PortedVolumeCommand(older, newer, parent)
    {};
    void undo() override {
        this->mainwindow->changeBPPortedVolume(older, manual);
        GenericCommand::undo();
    };
    void redo() override {
        this->mainwindow->changeBPPortedVolume(newer, manual);
        GenericCommand::redo();
    };
};

class BPPortedResFreqCommand: public PortedResFreqCommand
{
public:
    BPPortedResFreqCommand(double older, double newer, QObject *parent)
        : PortedResFreqCommand(older, newer, parent)
    {};

    void undo() override {
        this->mainwindow->changeBPPortedResFreq(older, manual);
        GenericCommand::undo();
    };
    void redo() override {
        this->mainwindow->changeBPPortedResFreq(newer, manual);
        GenericCommand::redo();
    };
};

class BPPortedPortNumberCommand: public PortedPortNumberCommand
{
public:
    BPPortedPortNumberCommand(unsigned int older, unsigned int newer, QObject *parent)
        : PortedPortNumberCommand(older, newer, parent)
    {};
    void undo() override {
        this->mainwindow->changeBPPortedPortNumber(older, manual);
        GenericCommand::undo();
    };
    void redo() override {
        this->mainwindow->changeBPPortedPortNumber(newer, manual);
        GenericCommand::redo();
    };
};

class BPPortedPortDiamCommand: public PortedPortDiamCommand
{
public:
    BPPortedPortDiamCommand(double older, double newer, QObject *parent)
        : PortedPortDiamCommand(older, newer, parent)
    {};
    void undo() override {
        this->mainwindow->changeBPPortedPortDiam(older, manual);
        GenericCommand::undo();
    };
    void redo() override {
        this->mainwindow->changeBPPortedPortDiam(newer, manual);
        GenericCommand::redo();
    };
};

#endif // UNDOCOMMANDS_H
