#include "configdialog.h"
#include "mainwindow.h"
#include "ui_configdialog.h"
#include <QMessageBox>
#include <iostream>

using namespace std;

ConfigDialog::ConfigDialog(QWidget *parent, int participantId) :
    QDialog(parent),
    ui(new Ui::ConfigDialog)
{
    ui->setupUi(this);
    this->ui->partIdSpinBox->setValue(participantId);
    this->participantId = participantId;

    MainWindow* mw = ((MainWindow*)this->parent());
    if(mw->f_haveJoystick){
         this->ui->statusEmergencyJoystick->setText(tr("DETECTED"));
    }

    this->ui->recordingOption_checkbox->setChecked(mw->isRecordingEnabled);
}

ConfigDialog::~ConfigDialog()
{
    delete ui;
}


int ConfigDialog::getParticipantId(){
    return this->participantId;
}

void ConfigDialog::on_buttonBox_accepted()
{
    this->participantId = ui->partIdSpinBox->value();
}

void ConfigDialog::on_pushButton_2_clicked()
{
    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, tr("Restart log confirmation"), tr("Are you sure that you want to restart loging data?"),
                                QMessageBox::Yes|QMessageBox::No);
    if (reply == QMessageBox::Yes) {
      cout << "Reestarting logs";
      GUILogger::getInstance().clearLogs();
    }
}



void ConfigDialog::on_generateLog_button_clicked()
{
    ostringstream filename;
    filename << "GuiLog_Participant_" <<  this->participantId << ".log";
    ostringstream filenameTrajectories;
    filenameTrajectories << "TrajectoriesLog_Participant_" <<  this->participantId << ".log";
    bool save = true;

    if (std::ifstream(filename.str()) || (std::ifstream(filenameTrajectories.str()) && this->ui->recordingOption_checkbox->isChecked()) )
    {
        QString question;
        if(APPEND_DATA_IN_LOGFILE){
            question = tr("The logfile already exist. Data will be APPENDED. Do you want to proceed?");
        }else{
            question = tr("The logfile already exist. Data will be OVERWRITTEN. Do you want to proceed?");
        }
        QMessageBox::StandardButton reply;
        reply = QMessageBox::question(this,tr("Logfile Already Exist"),question, QMessageBox::Yes|QMessageBox::No);
        save = reply == QMessageBox::Yes;
    }

    if(save){
        //Save GUI Events Log
        string strfile = filename.str();
        GUILogger::getInstance().dumpEvents(strfile,APPEND_DATA_IN_LOGFILE);

        if(this->ui->recordingOption_checkbox->isChecked()){
            //Save Trajectories for this participant
            GUILogger::getInstance().dumpTrajectories(filenameTrajectories.str(),SqlManager::getInstance().getCompleteTrajectoriesByParticipant(this->participantId),APPEND_DATA_IN_LOGFILE);
        }

        QMessageBox* msgBox = new QMessageBox();
        msgBox->setWindowTitle(tr("Log Saved"));

        ostringstream msg;
        msg << tr("Log info was saved in \n").toStdString();
        if(this->ui->recordingOption_checkbox->isChecked()){
            msg << strfile << tr(" and \n").toStdString()  <<  filenameTrajectories.str();
        }else{
            msg << strfile;
        }

        msgBox->setText(QString::fromStdString(msg.str()));
        msgBox->exec();

    }
}



void ConfigDialog::on_pushButton_clicked()
{
    QString question="All the data will be erased. before proceeding sure to have generated the log for this participant. \nDo you want to continue?";
    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this,tr("Warning"),question, QMessageBox::Yes|QMessageBox::No);
    if(reply == QMessageBox::Yes){
       QMessageBox* msgBox = new QMessageBox();
       if(SqlManager::getInstance().cleanDB()){
           msgBox->setWindowTitle(tr("Info"));
           msgBox->setText(tr("Database cleaned!"));
           MainWindow* mw = ((MainWindow*)this->parent());
           mw->clearTrajectoryPanel();
       }else{
           msgBox->setWindowTitle(tr("Error"));
           msgBox->setText(tr("Error while cleaning database!"));
       }
        msgBox->exec();
    }
}


void ConfigDialog::on_joystickModeButton_toggled(bool checked)
{
    MainWindow* mw = ((MainWindow*)this->parent());
    mw->enableJoystickMode(checked);
}

void ConfigDialog::on_pushButton_3_clicked()
{
    MainWindow* mw = ((MainWindow*)this->parent());
    if(!mw->f_haveJoystick){
        mw->initJoystick();
        if(mw->f_haveJoystick){
            this->ui->statusEmergencyJoystick->setText(tr("DETECTED"));
        }else{
            this->ui->statusEmergencyJoystick->setText(tr("NOT DETECTED"));
        }
    }else{
        this->ui->statusEmergencyJoystick->setText(tr("DETECTED"));
    }


}

void ConfigDialog::on_recordingOption_checkbox_clicked(bool checked)
{
    MainWindow* mw = ((MainWindow*)this->parent());
    mw->enableRecordingOption(checked);
}
