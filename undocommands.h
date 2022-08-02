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
        this->mainwindow->changeSpeakerNumber(older);
        GenericCommand::undo();
    };
    void redo() override {
        this->mainwindow->changeSpeakerNumber(newer);
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
        this->mainwindow->changeVendor(older, oldspeaker);
        GenericCommand::undo();
    };
    void redo() override {
        this->mainwindow->changeVendor(newer, Speaker());
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
        this->mainwindow->changeModel(older);
        GenericCommand::undo();
    };
    void redo() override {
        this->mainwindow->changeModel(newer);
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
        this->mainwindow->changeSealedVolume(older);
        GenericCommand::undo();
    };
    void redo() override {
        this->mainwindow->changeSealedVolume(newer);
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
        this->mainwindow->changePortedVolume(older);
        GenericCommand::undo();
    };
    void redo() override {
        this->mainwindow->changePortedVolume(newer);
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
        this->mainwindow->changePortedResFreq(older);
        GenericCommand::undo();
    };
    void redo() override {
        this->mainwindow->changePortedResFreq(newer);
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
        this->mainwindow->changePortedPortNumber(older);
        GenericCommand::undo();
    };
    void redo() override {
        this->mainwindow->changePortedPortNumber(newer);
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
        this->mainwindow->changePortedPortDiam(older);
        GenericCommand::undo();
    };
    void redo() override {
        this->mainwindow->changePortedPortDiam(newer);
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
        this->mainwindow->changePortedSlotPortActivation(older);
        GenericCommand::undo();
    };
    void redo() override {
        this->mainwindow->changePortedSlotPortActivation(newer);
        GenericCommand::redo();
    };

protected:
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
        this->mainwindow->changePortedSlotWidth(older);
        GenericCommand::undo();
    };
    void redo() override {
        this->mainwindow->changePortedSlotWidth(newer);
        GenericCommand::redo();
    };

protected:
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
        this->mainwindow->changeBPSealedVolume(this->older);
        GenericCommand::undo();
    };
    void redo() override {
        this->mainwindow->changeBPSealedVolume(this->newer);
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
        this->mainwindow->changeBPPortedVolume(older);
        GenericCommand::undo();
    };
    void redo() override {
        this->mainwindow->changeBPPortedVolume(newer);
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
        this->mainwindow->changeBPPortedResFreq(older);
        GenericCommand::undo();
    };
    void redo() override {
        this->mainwindow->changeBPPortedResFreq(newer);
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
        this->mainwindow->changeBPPortedPortNumber(older);
        GenericCommand::undo();
    };
    void redo() override {
        this->mainwindow->changeBPPortedPortNumber(newer);
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
        this->mainwindow->changeBPPortedPortDiam(older);
        GenericCommand::undo();
    };
    void redo() override {
        this->mainwindow->changeBPPortedPortDiam(newer);
        GenericCommand::redo();
    };
};

class BPPortedSlotPortCommand: public PortedSlotPortCommand
{
public:
    BPPortedSlotPortCommand(bool older, bool newer, QObject* parent)
        : PortedSlotPortCommand(older, newer, parent)
    {};
    void undo() override {
        this->mainwindow->changeBPPortedSlotPortActivation(older);
        GenericCommand::undo();
    };
    void redo() override {
        this->mainwindow->changeBPPortedSlotPortActivation(newer);
        GenericCommand::redo();
    };
};

class BPPortedSlotWidthCommand: public PortedSlotWidthCommand
{
public:
    BPPortedSlotWidthCommand(double older, double newer, QObject* parent)
        : PortedSlotWidthCommand(older, newer, parent)
    {};
    void undo() override {
        this->mainwindow->changeBPPortedSlotWidth(older);
        GenericCommand::undo();
    };

    void redo() override {
        this->mainwindow->changeBPPortedSlotWidth(newer);
        GenericCommand::redo();
    };
};

#endif // UNDOCOMMANDS_H
