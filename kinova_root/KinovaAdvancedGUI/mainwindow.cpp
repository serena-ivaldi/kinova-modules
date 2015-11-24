#include "mainwindow.h"

#include <QPushButton>
#include <QMessageBox>
#include <QMainWindow>
#include <QHBoxLayout>
#include <iostream>
#include <future>
#include <map>
#include <utility>

using namespace std;
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);


    this->moveDownTimer = new QTimer(this);
    this->moveLeftTimer = new QTimer(this);
    this->moveUpTimer = new QTimer(this);
    this->moveRightTimer = new QTimer(this);
    this->movePushTimer = new QTimer(this);
    this->movePullTimer = new QTimer(this);
    this->openHandTimer = new QTimer(this);
    this->closeHandTimer = new QTimer(this);
    this->klib = new Kinovalib();
    this->sqlManager = new SqlManager();
    this->sqlManager->init();
    this->point.InitStruct();
    initGUI();
}




void MainWindow::initGUI(){
    this->armCommand=true;
    this->fingerCommand=true;

    this->kinova_initialized = false;
    this->isRecordingTrajecory=false;

    this->ui->speedComboBox->addItem(tr("precision"), PRECISION_SPEED);
    this->ui->speedComboBox->addItem(tr("low"), LOW_SPEED);
    this->ui->speedComboBox->addItem(tr("medium"), MEDIUM_SPEED);
    this->ui->speedComboBox->addItem(tr("high"), HIGH_SPEED);
    this->ui->speedComboBox->setCurrentIndex(1);
    this->speed= LOW_SPEED;
    this->isMoving = false;
    this->isSpeedIncremented = false;

    /*********************************** Recorded Trajectories panel ******************/
    //this->recordedTrajectories = this->sqlManager->getTrajectoriesInfo();
    this->recordedTrajectories = this->sqlManager->getCompleteTrajectories();

    int row = 0;
    int col = 0;
    this->gridLayoutWidget_6 = new QWidget(this->ui->playBox);
    this->gridLayoutWidget_6->setObjectName(QStringLiteral("gridLayoutWidget_6"));
    this->gridLayoutWidget_6->setGeometry(QRect(QPoint(0, 25), QSize(800, 270) ));

    this->gridPlayPanel = new QGridLayout(gridLayoutWidget_6);

    /*For each recorded trajectory we create an Button with the play icon*/
    for(vector<Trajectory>::iterator iter = this->recordedTrajectories.begin(); iter!=this->recordedTrajectories.end();++iter){
        Trajectory t = *iter;

        this->addTrajectory(t,col,row);
        if(col==MAX_COLUMNS_PLAY_PANEL-1){
            col=0;
            row= row+2;
        }else{
            col ++;
        }
    }


    /*Hide recording label*/
    this->ui->recordingLabel->setVisible(false);

    /*Disable Undo button*/
    this->ui->undoButton->setEnabled(false);

    /*Connect right click signals with slots*/
    QObject::connect(this->ui->rightButton, SIGNAL(onPressRightClick(void)),this, SLOT(on_button_rightClick_IncreaseSpeed()));
    QObject::connect(this->ui->downButton, SIGNAL(onPressRightClick(void)),this, SLOT(on_button_rightClick_IncreaseSpeed()));
    QObject::connect(this->ui->leftButton, SIGNAL(onPressRightClick(void)),this, SLOT(on_button_rightClick_IncreaseSpeed()));
    QObject::connect(this->ui->upButton, SIGNAL(onPressRightClick(void)),this, SLOT(on_button_rightClick_IncreaseSpeed()));
    QObject::connect(this->ui->pullButton_Y, SIGNAL(onPressRightClick(void)),this, SLOT(on_button_rightClick_IncreaseSpeed()));
    QObject::connect(this->ui->pushButton_Y, SIGNAL(onPressRightClick(void)),this, SLOT(on_button_rightClick_IncreaseSpeed()));

    QObject::connect(this->ui->rightButton, SIGNAL(onReleaseRightClick(void)),this, SLOT(on_button_rightClick_DecreaseSpeed()));
    QObject::connect(this->ui->downButton, SIGNAL(onReleaseRightClick(void)),this, SLOT(on_button_rightClick_DecreaseSpeed()));
    QObject::connect(this->ui->leftButton, SIGNAL(onReleaseRightClick(void)),this, SLOT(on_button_rightClick_DecreaseSpeed()));
    QObject::connect(this->ui->upButton, SIGNAL(onReleaseRightClick(void)),this, SLOT(on_button_rightClick_DecreaseSpeed()));
    QObject::connect(this->ui->pullButton_Y, SIGNAL(onReleaseRightClick(void)),this, SLOT(on_button_rightClick_DecreaseSpeed()));
    QObject::connect(this->ui->pushButton_Y, SIGNAL(onReleaseRightClick(void)),this, SLOT(on_button_rightClick_DecreaseSpeed()));

}

void MainWindow::on_button_rightClick_IncreaseSpeed(){
    if(this->isMoving){
       switch (this->ui->speedComboBox->currentIndex()) {
       case 0:
           this->speed = LOW_SPEED;
           this->ui->speedComboBox->setCurrentIndex(1);
           this->isSpeedIncremented = true;
           break;
       case 1:
           this->speed = MEDIUM_SPEED;
           this->ui->speedComboBox->setCurrentIndex(2);
           this->isSpeedIncremented = true;
           break;
       case 2:
           this->speed = HIGH_SPEED;
           this->ui->speedComboBox->setCurrentIndex(3);
           this->isSpeedIncremented = true;
           break;
       default:
           break;
       }
    }
}

void MainWindow::on_button_rightClick_DecreaseSpeed(){
    if(this->isSpeedIncremented){
       switch (this->ui->speedComboBox->currentIndex()) {
       case 1:
           this->speed = PRECISION_SPEED;
           this->ui->speedComboBox->setCurrentIndex(0);
           this->isSpeedIncremented=false;
           break;
       case 2:
           this->speed = LOW_SPEED;
           this->ui->speedComboBox->setCurrentIndex(1);
           this->isSpeedIncremented=false;
           break;
       case 3:
           this->speed = MEDIUM_SPEED;
           this->ui->speedComboBox->setCurrentIndex(2);
           this->isSpeedIncremented=false;
           break;
       default:
           break;
       }
    }
}

QLatin1String getLabelStyle(){
    return QLatin1String("background-color: rgb(51, 51, 51);\n"
                         "color: rgb(247, 247, 246);\n"
                         "");
}

QFont getLabelFont(){
    QFont font2;
    font2.setBold(true);
    font2.setWeight(90);
//    font2.set
    font2.setPointSize(15);
    return font2;
}

//TEST it!!!
void MainWindow::playTrajectoryButtonClicked(int trajectoryId){
    cout<< "entro en play" << endl;
    cout << trajectoryId << endl;
    for(int i = 0;i<this->recordedTrajectories.size();i++){
        Trajectory t = this->recordedTrajectories[i];
        if(t.id==trajectoryId){
            cout << "find the trajectory. Name = " << t.name << endl;
            vector<Log> logs = this->convertTrajectory2Log(t);
            cout << "after converting" << endl;
            vector<Log> new_ff;

            Log cartPosLog = logs[this->readTypeMap["cart_pos"]];
            Log velLog = logs[this->readTypeMap["j_vel"]];

           /* for(int j=0;j<velLog.size();j++){
                 cout << "joints vel :" << velLog[j][0] << velLog[j][1] << velLog[j][2] << velLog[j][3] << velLog[j][4] << velLog[j][5] <<endl;
            }

            for(int j=0;j<cartPosLog.size();j++){
                 cout << "cartPos :" << cartPosLog[j][0] << cartPosLog[j][1] << cartPosLog[j][2] << cartPosLog[j][3] << cartPosLog[j][4] << cartPosLog[j][5] <<endl;
            }*/


            new_ff.push_back(cartPosLog);
            new_ff.push_back(velLog);


            cout << "after converting 2" << endl;

            State initial(9);
            cout << "logs.size = " << logs.size()<< endl;
            cout << "logs[0].size = " << logs[0].size()<<endl;



            initial[0] = cartPosLog[0][0];
            initial[1] = cartPosLog[0][1];
            initial[2] = cartPosLog[0][2];
            initial[3] = cartPosLog[0][3];
            initial[4] = cartPosLog[0][4];
            initial[5] = cartPosLog[0][5];


            //FIXME TODO When the finger position was ready, read it from cart_pos
            initial[6] = 0;
            initial[7] = 0;
            initial[8] = 0;
            //initial[6] = cartPosLog[0][6];
            //initial[7] = cartPosLog[0][7];
            //initial[8] = cartPosLog[0][8];

            int logTimestampIndex = this->readTypeMap["comp_t"];
            cout << "before access to time log" << endl;
            Log timeLog = logs[logTimestampIndex];

            for(int j=0;j<cartPosLog.size();j++){
                 cout << "timeLog :" << timeLog[j][0] <<endl;
            }
            cout<< "before executing trajectory" << endl;
            this->bot->ExecuteUpdatedTrajectory(timeLog,initial,new_ff);
            cout<< "after executing trajectory" << endl;
            break;
        }
    }
}

void MainWindow::addTrajectory(Trajectory t, int col, int row){
    cout << "adding t = " << t.id<< endl;
    //Add push button
    QPushButton* pushButton = new QPushButton(this->gridLayoutWidget_6);
    pushButton->setMaximumSize(QSize(120, 120));

    QSizePolicy sizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    sizePolicy.setHorizontalStretch(0);
    sizePolicy.setVerticalStretch(0);
    sizePolicy.setHeightForWidth(pushButton->sizePolicy().hasHeightForWidth());
    pushButton->setSizePolicy(sizePolicy);

    QIcon icon8;
    icon8.addFile(QStringLiteral(":/imagenes/img/play.png"), QSize(), QIcon::Normal, QIcon::Off);
    pushButton->setIcon(icon8);
    pushButton->setIconSize(QSize(75, 75));

    QSignalMapper* signalMapper = new QSignalMapper (this) ;
    connect (pushButton, SIGNAL(clicked()), signalMapper, SLOT(map())) ;

    signalMapper -> setMapping (pushButton, t.id) ;
    connect (signalMapper, SIGNAL(mapped(int)), this, SLOT(playTrajectoryButtonClicked(int)));


    this->gridPlayPanel->addWidget(pushButton, row, col);

    //add label
    QLabel* label = new QLabel(this->gridLayoutWidget_6);
    label->setText(QString::fromStdString(t.name));
    label->setMinimumSize(QSize(120, 40));
    label->setMaximumSize(QSize(16777215, 40));
    label->setStyleSheet(getLabelStyle());
    label->setFont(getLabelFont());
    label->setAlignment(Qt::AlignmentFlag::AlignHCenter | Qt::AlignmentFlag::AlignCenter);
    label->setMargin(5);
    this->gridPlayPanel->addWidget(label, row+1, col, 1, 1, Qt::AlignHCenter);
}



MainWindow::~MainWindow()
{
    delete ui;
}



void MainWindow::clickedSlot(){
    cout<<"Sending command to kinova arm"<<endl;
    bool actualPositionObtained = true;
    if(!this->armCommand){
        /*As the Kinova API does not provide any finger only command, then we simulate it by sending the actual coordinates of the arm.
        So, here we load the actual position and then send the command with the finger parameters*/
        int res = this->klib->getActualCartesianPosition(this->actualPosition);
        if(res==SUCCESS){
            this->point.Position.CartesianPosition.X = actualPosition.Coordinates.X;
            this->point.Position.CartesianPosition.Y = actualPosition.Coordinates.Y;
            this->point.Position.CartesianPosition.Z = actualPosition.Coordinates.Z;
            this->point.Position.CartesianPosition.ThetaX= actualPosition.Coordinates.ThetaX;
            this->point.Position.CartesianPosition.ThetaY=actualPosition.Coordinates.ThetaY;
            this->point.Position.CartesianPosition.ThetaZ=actualPosition.Coordinates.ThetaZ;
        }else{
            QMessageBox* msgBox = new QMessageBox();
            msgBox->setWindowTitle(tr("Send Command"));
            msgBox->setText(tr("Something went wrong, couldn't get the actual position of the robot"));
            msgBox->exec();
            actualPositionObtained=false;
        }
    }
    if(actualPositionObtained){
        klib->sendCommand(this->opType,this->armCommand,this->fingerCommand,this->point);
    }
}

void MainWindow::on_homeButton_clicked()
{
    cout << "Moving home"<< endl;
    if(KINOVA_LIB==1){
        cout<<"before moving home" <<endl;
        this->bot->MoveHome();
        cout<<"ater moving home" <<endl;
    }else{
        int res = klib->moveHome();
        if(res!=SUCCESS){
            QMessageBox* msgBox = new QMessageBox();
            msgBox->setWindowTitle(tr("Send Command"));
            msgBox->setText(tr("Couldn't go home"));
            msgBox->exec();
        }
    }
    cout<<"ending funcion" <<endl;
}


void MainWindow::on_xDoubleSpinBox_valueChanged(double arg1)
{
    this->point.Position.CartesianPosition.X=arg1;
}

void MainWindow::on_yDoubleSpinBox_valueChanged(double arg1)
{
    this->point.Position.CartesianPosition.Y=arg1;
}

void MainWindow::on_zDoubleSpinBox_valueChanged(double arg1)
{
    this->point.Position.CartesianPosition.Z=arg1;
}

void MainWindow::on_xDoubleSpinBox_2_valueChanged(double arg1)
{
    this->point.Position.CartesianPosition.ThetaX=arg1;
}

void MainWindow::on_yDoubleSpinBox_2_valueChanged(double arg1)
{
    this->point.Position.CartesianPosition.ThetaY=arg1;
}

void MainWindow::on_zDoubleSpinBox_2_valueChanged(double arg1)
{
    this->point.Position.CartesianPosition.ThetaZ=arg1;
}

void MainWindow::on_speed1DoubleSpinBox_valueChanged(double arg1)
{
    this->point.Limitations.speedParameter1=arg1;
}

void MainWindow::on_speed2DoubleSpinBox_valueChanged(double arg1)
{
    this->point.Limitations.speedParameter2=arg1;
}

void MainWindow::on_limitationsGroup_toggled(bool arg1)
{
    this->point.LimitationsActive= (int)arg1;
}

void MainWindow::on_ArmControl_Widget_toggled(bool arg1)
{
    this->armCommand=arg1;
}

void MainWindow::on_FingersControl_Widget_toggled(bool arg1)
{
    this->fingerCommand=arg1;
}

State convertDirectionToState(int direction, float speed){
    State res;
    res.zeros(9);
    switch (direction) {
    case Right:
        res[0] = -speed;
        break;
    case Down:
        res[2] = -speed;
        break;
    case Left:
        res[0] = speed;
        break;
    case Up:
        res[2] = speed;
        break;
    case Push:
        res[1] = -speed;
        break;
    case Pull:
        res[1] = speed;
        break;
    case Open:
        res[6] = -30;
        res[7] = -30;
        res[8] = -30;
        break;
    case Close:
        res[6] = 30;
        res[7] = 30;
        res[8] = 30;
        break;
    default:
        cout << "Wrong direction" << endl;
        break;
    }
    return res;
}

void MainWindow::loopSendVelocityCommad(int direction){
    if(!this->stopedTimers[direction-1]){ //we have to use a bool to know I we stopped the timer  because the timer->stop() is not immediate
       cout << "en el loop --> enviar comandos de velocidad: "  << this->fixedStepsCounter[direction-1] <<endl;
       if(direction==Close || direction==Open){
           if(KINOVA_LIB == 1){
               State cmd = convertDirectionToState(direction,30);
               this->bot->SendCommand(cmd,27);
           }else{
                this->klib->moveSingleStep(direction,30); //Fixed speed, the unit velocity of the fingers is not clear at all!
           }
       }else{
           if(KINOVA_LIB == 1){
               State cmd = convertDirectionToState(direction,this->speed);
               this->bot->SendCommand(cmd,7);
            }else{
               this->klib->moveSingleStep(direction,this->speed);
           }

       }
       //FIXME the code below should be ideally in other thread in charge of reading the status of the arm.
       //As the official sdf is not thread safe we cannot do it. If someday we start using a thread safe driver then
       //we can fix this issue.
       if(this->isRecordingTrajecory && KINOVA_LIB!=1){
           RecordedCartesianInfo cartesianInfo;
           TrajectoryPoint point;
           this->klib->getTrajectoryInfo(point);
           cartesianInfo.pos_x = point.Position.CartesianPosition.X;
           cartesianInfo.pos_y = point.Position.CartesianPosition.Y;
           cartesianInfo.pos_z = point.Position.CartesianPosition.Z;
           cartesianInfo.theta_x = point.Position.CartesianPosition.ThetaX;
           cartesianInfo.theta_y = point.Position.CartesianPosition.ThetaY;
           cartesianInfo.theta_z = point.Position.CartesianPosition.ThetaZ;

           cartesianInfo.finger1 = point.Position.Fingers.Finger1;
           cartesianInfo.finger2 = point.Position.Fingers.Finger2;
           cartesianInfo.finger3 = point.Position.Fingers.Finger3;

           AngularPosition angPos;
           this->klib->getAngularVelocity(angPos);
           cartesianInfo.angvel_j1 = angPos.Actuators.Actuator1;
           cartesianInfo.angvel_j2 = angPos.Actuators.Actuator2;
           cartesianInfo.angvel_j3 = angPos.Actuators.Actuator3;
           cartesianInfo.angvel_j4 = angPos.Actuators.Actuator4;
           cartesianInfo.angvel_j5 = angPos.Actuators.Actuator5;
           cartesianInfo.angvel_j6 = angPos.Actuators.Actuator6;
           this->sampledTrajectoryInfo.push_back(cartesianInfo);
       }
    }
}


void MainWindow::on_rightButton_pressed()
{
    cout<<"rightPressed"<<endl;
     this->stopedTimers[Right-1] = false;

    //Save actual position as a checkpoint
    this->saveCheckPoint();

    //We use QSignal mapper to send a parameter to the slot loopSendVelocityCommand. This parameter is the movement direction
    QSignalMapper* signalMapper = new QSignalMapper (this) ;
    connect (this->moveRightTimer, SIGNAL(timeout()), signalMapper, SLOT(map())) ;

    signalMapper -> setMapping (this->moveRightTimer, Right) ;
    connect (signalMapper, SIGNAL(mapped(int)), this, SLOT(loopSendVelocityCommad(int)));

    this->moveRightTimer->start(LOOP_SPEED_SEND_VELOCITY_COMMAND);
     this->isMoving = true;
}


void MainWindow::on_rightButton_released()
{
    cout << "right button released" << endl;
    this->moveRightTimer->stop();
    cout<<"right timer stopped"<<endl;
    this->stopedTimers[Right-1] = true;
     this->isMoving = false;
}

void MainWindow::on_upButton_pressed()
{
    this->stopedTimers[Up-1] = false;

    //Save actual position as a checkpoint
    this->saveCheckPoint();
    //We use QSignal mapper to send a parameter to the slot loopSendVelocityCommand. This parameter is the movement direction
    QSignalMapper* signalMapper = new QSignalMapper (this) ;
    connect (this->moveUpTimer, SIGNAL(timeout()), signalMapper, SLOT(map())) ;
    signalMapper->setMapping (this->moveUpTimer, Up) ;
    connect (signalMapper, SIGNAL(mapped(int)), this, SLOT(loopSendVelocityCommad(int)));
    this->moveUpTimer->start(LOOP_SPEED_SEND_VELOCITY_COMMAND);
     this->isMoving = true;
}

void MainWindow::on_upButton_released()
{
    this->moveUpTimer->stop();
    this->stopedTimers[Up-1] = true;
     this->isMoving = false;

}

void MainWindow::on_leftButton_pressed()
{
    this->stopedTimers[Left-1] = false;

    //Save actual position as a checkpoint
    this->saveCheckPoint();

    //We use QSignal mapper to send a parameter to the slot loopSendVelocityCommand. This parameter is the movement direction
    QSignalMapper* signalMapper = new QSignalMapper (this) ;
    connect (this->moveLeftTimer, SIGNAL(timeout()), signalMapper, SLOT(map())) ;

    signalMapper -> setMapping (this->moveLeftTimer, Left) ;
    connect (signalMapper, SIGNAL(mapped(int)), this, SLOT(loopSendVelocityCommad(int)));
    this->moveLeftTimer->start(LOOP_SPEED_SEND_VELOCITY_COMMAND);
     this->isMoving = true;
}

void MainWindow::on_leftButton_released()
{
    this->moveLeftTimer->stop();
    this->stopedTimers[Left-1] = true;
    this->isMoving = false;

}

void MainWindow::on_downButton_pressed()
{
    this->stopedTimers[Down-1] = false;

    //Save actual position as a checkpoint
    this->saveCheckPoint();

    //We use QSignal mapper to send a parameter to the slot loopSendVelocityCommand. This parameter is the movement direction
    QSignalMapper* signalMapper = new QSignalMapper (this) ;
    delete this->moveDownTimer;
    this->moveDownTimer = new QTimer(this);
    connect (this->moveDownTimer, SIGNAL(timeout()), signalMapper, SLOT(map())) ;

    signalMapper -> setMapping (this->moveDownTimer, Down) ;
    connect (signalMapper, SIGNAL(mapped(int)), this, SLOT(loopSendVelocityCommad(int)));
    this->moveDownTimer->start(LOOP_SPEED_SEND_VELOCITY_COMMAND);
    this->isMoving = true;
}

void MainWindow::on_downButton_released()
{
   this->moveDownTimer->stop();
   this->stopedTimers[Down-1] = true;
    this->isMoving = false;
}



void MainWindow::on_pushButton_Y_pressed()
{

    this->stopedTimers[Push-1] = false;
    //We use QSignal mapper to send a parameter to the slot loopSendVelocityCommand. This parameter is the movement direction

    //Save actual position as a checkpoint
    this->saveCheckPoint();

    QSignalMapper* signalMapper = new QSignalMapper (this) ;
    delete this->movePushTimer;
    this->movePushTimer = new QTimer(this);
    connect (this->movePushTimer, SIGNAL(timeout()), signalMapper, SLOT(map())) ;

    signalMapper -> setMapping (this->movePushTimer, Push) ;
    connect (signalMapper, SIGNAL(mapped(int)), this, SLOT(loopSendVelocityCommad(int)));
    this->movePushTimer->start(LOOP_SPEED_SEND_VELOCITY_COMMAND);
     this->isMoving = true;
}

void MainWindow::on_pushButton_Y_released()
{
    this->movePushTimer->stop();
    this->stopedTimers[Push-1] = true;
     this->isMoving = false;

}


void MainWindow::on_pullButton_Y_pressed()
{
    this->stopedTimers[Pull-1] = false;

    //Save actual position as a checkpoint
    this->saveCheckPoint();

    //We use QSignal mapper to send a parameter to the slot loopSendVelocityCommand. This parameter is the movement direction
    QSignalMapper* signalMapper = new QSignalMapper (this) ;
    delete this->movePullTimer;
    this->movePullTimer = new QTimer(this);
    connect (this->movePullTimer, SIGNAL(timeout()), signalMapper, SLOT(map())) ;

    signalMapper -> setMapping (this->movePullTimer, Pull) ;
    connect (signalMapper, SIGNAL(mapped(int)), this, SLOT(loopSendVelocityCommad(int)));
    this->movePullTimer->start(LOOP_SPEED_SEND_VELOCITY_COMMAND);
     this->isMoving = true;
}

void MainWindow::on_pullButton_Y_released()
{
    this->movePullTimer->stop();
    this->stopedTimers[Pull-1] = true;
     this->isMoving = false;

}



/*Real time Hand Control*/
void MainWindow::on_openHandButton_pressed(){
    this->stopedTimers[Open-1] = false;

    //Save actual position as a checkpoint
    this->saveCheckPoint();

    //We use QSignal mapper to send a parameter to the slot loopSendVelocityCommand. This parameter is the movement direction
    QSignalMapper* signalMapper = new QSignalMapper (this) ;
    delete this->openHandTimer;
    this->openHandTimer = new QTimer(this);
    connect (this->openHandTimer, SIGNAL(timeout()), signalMapper, SLOT(map())) ;

    signalMapper -> setMapping (this->openHandTimer, Open) ;
    connect (signalMapper, SIGNAL(mapped(int)), this, SLOT(loopSendVelocityCommad(int)));
    this->openHandTimer->start(LOOP_SPEED_SEND_VELOCITY_COMMAND);
}

void MainWindow::on_openHandButton_released()
{
    this->openHandTimer->stop();
    this->stopedTimers[Open-1] = true;
}


void MainWindow::on_closeHandButton_pressed(){
    this->stopedTimers[Close-1] = false;

    //Save actual position as a checkpoint
    this->saveCheckPoint();

    //We use QSignal mapper to send a parameter to the slot loopSendVelocityCommand. This parameter is the movement direction
    QSignalMapper* signalMapper = new QSignalMapper (this) ;
    delete this->closeHandTimer;
    this->closeHandTimer = new QTimer(this);
    connect (this->closeHandTimer, SIGNAL(timeout()), signalMapper, SLOT(map())) ;

    signalMapper -> setMapping (this->closeHandTimer, Close) ;
    connect (signalMapper, SIGNAL(mapped(int)), this, SLOT(loopSendVelocityCommad(int)));
    this->closeHandTimer->start(LOOP_SPEED_SEND_VELOCITY_COMMAND);
}

void MainWindow::on_closeHandButton_released()
{
    this->closeHandTimer->stop();
    this->stopedTimers[Close-1] = true;
}


void MainWindow::on_pushButton_2_clicked()
{
    cout << "Initilizating kinova" << endl;
    int res = -1;
    if(KINOVA_LIB==1){
        // status reader
        Jaco* mdl = new Jaco();
        try{
            kinova_status_openapi * st= new kinova_status_openapi(mdl);

            this->readType.push_back("comp_t");
            this->readType.push_back("cart_pos");
            this->readType.push_back("j_vel");
            this->readType.push_back("j_pos");




            this->readTypeMap.insert(make_pair("comp_t",0));
            this->readTypeMap.insert(make_pair("cart_pos",1));
            this->readTypeMap.insert(make_pair("j_vel",2));
            this->readTypeMap.insert(make_pair("j_pos",3));

            cout << "time index = " << this->readTypeMap["comp_t"] <<endl;
            cout << "vel index = " << this->readTypeMap["j_vel"] <<endl;
            cout << "cartpos index = " << this->readTypeMap["cart_pos"] <<endl;

            /*this->readTypeMap["comp_t"] = 0;
            this->readTypeMap["cart_pos"] = 1;
            this->readTypeMap["j_vel"] = 2;
            this->readTypeMap["j_pos"] = 3;*/

            // controller
            const double Pid_coef[] = {5,0,0}; // deg
            std::vector<double> Pid(Pid_coef,End(Pid_coef));
            const char * _namefiles[] = {"cart_pos_openapi.mat","joint_vel_openapi.mat"};
            std::vector<std::string> namefile (_namefiles,End(_namefiles));
            const char * _meas_val[] ={"j_pos","cart_pos","j_vel"};
            std::vector<std::string> meas_val(_meas_val,End(_meas_val));
            int controltype = 8;
            bool limitation = 0;
            Jaco* md = new Jaco();
            string namefileindex = "index_openapi.mat";
            kinova_controller_openapi * ct = new kinova_controller_openapi(namefile,namefileindex,meas_val,Pid,controltype,limitation,md,st->arm); // very rough patch because i can have only one API handle
            // checking module
            // define bounding box
            const double bb_point[] = {-0.6,-0.8,-0.4};
            const double bb_dims[]  = {1.2,1.6,0.8};
            std::vector<double> bb_p(bb_point,End(bb_point)),bb_d(bb_dims,End(bb_dims));

            // define all the limit
            const char *cl[] = {"j_pos","j_tau"};
            std::vector<std::string> chekclist(cl,End(cl));
            const double joint_min[] = {-10000,47,19,-10000,-10000,-10000}; // deg
            const double joint_max[] = {10000,313,341,10000,10000,10000}; // deg
            const double tau_min[] = {-15}; // Nm
            const double tau_max[] = {15};  // nm
            std::vector<double> j_min(joint_min,End(joint_min)),j_max(joint_max,End(joint_max));
            std::vector<double> t_min(tau_min,End(tau_min)),t_max(tau_max,End(tau_max));

            std::vector<std::vector<double> > l_down_left_corner,l_dims,l_min,l_max;
            l_down_left_corner.push_back(bb_p);l_dims.push_back(bb_d);
            l_min.push_back(j_min);l_min.push_back(t_min);
            l_max.push_back(j_max);l_max.push_back(t_max);

            safetycheck checker(l_down_left_corner,l_dims,l_min,l_max,chekclist);
            this->bot= new robot(st,ct,checker);

            this->bot->MoveHome();
            State moveFingers(9);
            moveFingers[0] = 0;
            moveFingers[1] = 0;
            moveFingers[2] = 0;
            moveFingers[3] = 0;
            moveFingers[4] = 0;
            moveFingers[5] = 0;
            moveFingers[6] = 0;
            moveFingers[7] = -30;
            moveFingers[8] = -30;
            moveFingers[9] = -30;
            for(int i=0;i<10;i++){
                this->bot->SendCommand(moveFingers,27);
            }
            res = SUCCESS;
        }catch(KinDrv::KinDrvException e){
            cout << "error initializing"<< endl;
        }

    }else{
        res = this->klib->kinovaInit();
    }
    if(res == SUCCESS){
        this->ui->status_icon->setVisible(false);
        this->ui->status_label->setText(QString(tr("Success!")));
        this->kinova_initialized = true;
    }else{
        this->ui->status_label->setText(QString(tr("Initialization wrong, try again")));
    }
}


void MainWindow::error_kinova_not_initialized(){
    int res = klib->moveHome();
    if(res!=SUCCESS){
        QMessageBox* msgBox = new QMessageBox();
        msgBox->setWindowTitle(tr("Initialization Error"));
        msgBox->setText(tr("Kinova is not initilized. Try Init Kinova before running any command"));
        msgBox->exec();
    }
}


void MainWindow::on_speedComboBox_currentIndexChanged(const QString &arg1)
{
    if(QString::compare(arg1,QString(tr("low"))) == 0 ){
        this->speed = LOW_SPEED;
    }else if(QString::compare(arg1,QString(tr("precision"))) == 0 ){
        this->speed = PRECISION_SPEED;
    }else if(QString::compare(arg1,QString(tr("medium"))) == 0 ){
        this->speed = MEDIUM_SPEED;
    }else if(QString::compare(arg1,QString(tr("high"))) == 0 ){
        this->speed = HIGH_SPEED;
   }
}




/*Add the recorded trajectory to the list of trajectories and also to the panel*/
void MainWindow::addRecordedTrajectory(Trajectory t){
    this->recordedTrajectories.push_back(t);
    int row, col;
    int cant_trajectories = this->recordedTrajectories.size();
    row = cant_trajectories/MAX_COLUMNS_PLAY_PANEL;
    row=row*2;
    col = cant_trajectories%MAX_COLUMNS_PLAY_PANEL;
    if(col==0){
        col=MAX_COLUMNS_PLAY_PANEL-1;
        row-=2;
    }else{
        col=col-1;
    }
    this->addTrajectory(t,col,row);
}

void MainWindow::showSaveTrajectoryPanel(){
   Dialog* dialog2 = new Dialog();
   dialog2->init(this->sqlManager);
   if(KINOVA_LIB ==1){
       cout << "before convert" << endl;
       this->sampledTrajectoryInfo = convertLog2Trajectory(this->recordedLogs);
       cout << "after convert" << endl;
   }
   cout << "will show save panel" << endl;
   Trajectory saved =  dialog2->execAndReturnSavedTrajectory(this->sampledTrajectoryInfo);

   if(saved.id>-1){
        this->addRecordedTrajectory(saved);
   }
   this->sampledTrajectoryInfo.clear();
   this->recordedLogs.clear();
   delete dialog2;
}


void tarea(string msg){
    for(int i=0;i<10;i++){
        cout << "task1 " << msg <<endl;
        sleep(1);
    }
}

void tarea1(){
    for(int i=0;i<10;i++){
        cout << "task1 " << endl;
        sleep(1);
    }
}


void  WriteFile(std::vector<State> log,std::string namefile)
{
    std::ofstream myfile(namefile.c_str());
    cout << log.empty() << endl;
    cout << "file size " << log[0].size() << endl;
    myfile << log[0].size() << "\n";
    for(unsigned int i =0;i<log.size();i++)
    {
        for(int j=0;j<log[i].size();j++)
        {
            myfile<<log[i][j]<<" ";
        }
        myfile<<"\n";
    }

    myfile.close();

}


void MainWindow::writeLogFiles(vector<Log> recordedLogs){
    //TODO convert from vector<Log> to vector<RecordedCartesianInfo> and store the result in this->sampledTrajectories.

    Log cartPosLog = recordedLogs[this->readTypeMap["cart_pos"]];
    Log timeLog = recordedLogs[this->readTypeMap["comp_t"]];
    Log velLog = recordedLogs[this->readTypeMap["j_vel"]];

    cout << "before write file" <<endl;
    Log jointLog = recordedLogs[this->readTypeMap["j_pos"]];//delete it

    WriteFile(cartPosLog,"cart_pos_openapi.mat");
    cout << "after write file 1" <<endl;
    WriteFile(timeLog,"index_openapi.mat");
    cout << "after write file 2" <<endl;
    WriteFile(velLog,"joint_vel_openapi.mat");
    cout << "after write file 3" <<endl;
    WriteFile(jointLog,"joint_pos_openapi.mat");

    cout << "after write file 4" <<endl;

}

vector<RecordedCartesianInfo> MainWindow::convertLog2Trajectory(vector<Log> logs){
    Log cartPosLog = logs[this->readTypeMap["cart_pos"]];
    Log timeLog = logs[this->readTypeMap["comp_t"]];
    Log velLog = logs[this->readTypeMap["j_vel"]];
    vector<RecordedCartesianInfo> res;
    for(unsigned int i =0;i<cartPosLog.size();i++)
    {
            RecordedCartesianInfo cartInfo;
            //Cartesian Position
            cartInfo.pos_x = cartPosLog[i][0];
            cartInfo.pos_y = cartPosLog[i][1];
            cartInfo.pos_z = cartPosLog[i][2];
            cartInfo.theta_x = cartPosLog[i][3];
            cartInfo.theta_y = cartPosLog[i][4];
            cartInfo.theta_z = cartPosLog[i][5];

            //TODO missing finger position

            //Joint Velocity
            cartInfo.angvel_j1 = velLog[i][0];
            cartInfo.angvel_j2 = velLog[i][1];
            cartInfo.angvel_j3 = velLog[i][2];
            cartInfo.angvel_j4 = velLog[i][3];
            cartInfo.angvel_j5 = velLog[i][4];
            cartInfo.angvel_j6 = velLog[i][5];
            cout << "joints vel :" << velLog[i][0] << velLog[i][1] << velLog[i][2] << velLog[i][3] << velLog[i][4] << velLog[i][5] <<endl;
            //Timestamp
            cartInfo.timestamp = timeLog[i][0];

            res.push_back(cartInfo);
    }
    return res;
}

vector<Log> MainWindow::convertTrajectory2Log(Trajectory traj){

    vector<Log> res(4);
cout << "res.size = " << res.size() <<endl;
    cout << "trajInfo size = " << traj.trajectoryInfo.size() << endl;
    Log cartPosLog(traj.trajectoryInfo.size());
    Log timeLog(traj.trajectoryInfo.size());
    Log velLog(traj.trajectoryInfo.size());

    cout << "*** 1 ***" << endl;
    for(unsigned int i =0;i<traj.trajectoryInfo.size();i++)
    {
            cout << "*** 2 ***" << endl;
            RecordedCartesianInfo cartInfo = traj.trajectoryInfo[i];

            cout << "*** 3 ***" << endl;
            //Cartesian Position
            cartPosLog[i] = State(6);
            cartPosLog[i][0] = cartInfo.pos_x;
            cartPosLog[i][1] = cartInfo.pos_y;
            cartPosLog[i][2] = cartInfo.pos_z ;
            cartPosLog[i][3] = cartInfo.theta_x;
            cartPosLog[i][4] = cartInfo.theta_y;
            cartPosLog[i][5] = cartInfo.theta_z;

            //TODO missing finger position


            cout << "*** 4 ***" << endl;
            //Joint Velocity
            velLog[i] = State(6);
            velLog[i][0] = cartInfo.angvel_j1;
            velLog[i][1] = cartInfo.angvel_j2;
            velLog[i][2] = cartInfo.angvel_j3;
            velLog[i][3] = cartInfo.angvel_j4;
            velLog[i][4] = cartInfo.angvel_j5;
            velLog[i][5] = cartInfo.angvel_j6;

            cout << "*** 5 ***" << endl;
            //Timestamp
            timeLog[i]= State(1);
            cout << "time before asign :" << cartInfo.timestamp<<endl;
            timeLog[i][0] = cartInfo.timestamp;
            cout << "time after asign :" << timeLog[i][0] << endl;
    }

    cout << "before inserting in the vector" << endl;
    cout << "res.size = " << res.size() <<endl;
    res[this->readTypeMap["cart_pos"]] = cartPosLog;
    cout << "res.size = " << res.size() <<endl;

    cout << "time index = " << this->readTypeMap["comp_t"] <<endl;
    cout << "vel index = " << this->readTypeMap["j_vel"] <<endl;
    cout << "cartpos index = " << this->readTypeMap["cart_pos"] <<endl;

    res[this->readTypeMap["comp_t"]] = timeLog;
    cout << "res.size = " << res.size() <<endl;
    res[this->readTypeMap["j_vel"]] =  velLog;
    cout << "res.size = " << res.size() <<endl;
    cout << "after inserting in the vector" << endl;

  /*  for(int i=0;i< res[this->readTypeMap["comp_t"]].size() ;i++){
        cout << "time before resturn : " << res[this->readTypeMap["comp_t"]][i][0] << endl;
        cout << "time before resturn velLog: " << timeLog[i][0] << endl;
    }
    */
    return res;
}

void MainWindow::startRecording(){
    this->sampledTrajectoryInfo.clear();
    this->isRecordingTrajecory=true;
    this->ui->label_record_stop->setText(QString(tr("Stop")));
    this->ui->record_Button->setIcon(QIcon(":/imagenes/img/stop.png"));
    this->ui->recordingLabel->setVisible(true);
    //this->setActualPosition();
    if(KINOVA_LIB == 1){
        this->recordedLogs.clear();
        this->bot->StartLog(this->readType);
    }
}

void MainWindow::stopRecording(){
    this->ui->label_record_stop->setText(QString(tr("Record")));
    this->ui->record_Button->setIcon(QIcon(":/imagenes/img/record.png"));
    this->ui->recordingLabel->setVisible(false);
    this->isRecordingTrajecory=false;
    if(KINOVA_LIB==1){
        cout << "before stoping" << endl;
        recordedLogs = this->bot->StopLog(this->readType);
        cout << "after stoping" << endl;
        writeLogFiles(recordedLogs);

        this->recordedLogs = recordedLogs;
    }
}

void MainWindow::on_record_Button_toggled(bool checked)
{
    if(checked){
        this->startRecording();
    }else{
        this->stopRecording();

        QMessageBox::StandardButton reply;
        reply = QMessageBox::question(this,tr("Save"),tr("Do you want to save the recorded trajectory?"), QMessageBox::Yes|QMessageBox::No);
        if(reply == QMessageBox::Yes){
            showSaveTrajectoryPanel();
        }
    }

}

/*Save the actual position as a checkpoint. This allows the user to undo actions*/
void MainWindow::saveCheckPoint(){
    if(KINOVA_LIB==1){
        //TODO call bot->readCurrentState
        cout << "checkpoint saving not working. I need to call the bot->readCurrentState" << endl;
    }else{
        int res = this->klib->getActualCartesianPosition(this->actualPosition);
        if(res==SUCCESS){
            this->checkpoints.push_back(this->actualPosition);
            this->ui->undoButton->setEnabled(true);
        }else{
            QMessageBox* msgBox = new QMessageBox();
            msgBox->setWindowTitle(tr("Send Command"));
            msgBox->setText(tr("Something went wrong, couldn't get the actual position of the robot"));
            msgBox->exec();
        }
    }
}

void MainWindow::setActualPosition(){
    /*As the Kinova API does not provide any finger only command, then we simulate it by sending the actual coordinates of the arm.
    So, here we load the actual position and then send the command with the finger parameters*/
    if(KINOVA_LIB){
        vector<State> result;
        this->bot->ReadCurrentState(result,this->readType);
        int indice = this->readTypeMap["cart_pos"];
        this->point.Position.CartesianPosition.X = result[indice][0];
        this->point.Position.CartesianPosition.Y = result[indice][1];
        this->point.Position.CartesianPosition.Z = result[indice][2];
        this->point.Position.CartesianPosition.ThetaX = result[indice][3];
        this->point.Position.CartesianPosition.ThetaY = result[indice][4];
        this->point.Position.CartesianPosition.ThetaZ = result[indice][5];
        this->point.Position.Fingers.Finger1 = result[indice][6];
        this->point.Position.Fingers.Finger2 = result[indice][7];
        this->point.Position.Fingers.Finger3 = result[indice][8];
    }else{
        int res = this->klib->getActualCartesianPosition(this->actualPosition);
        if(res==SUCCESS){
            this->point.Position.CartesianPosition.X = actualPosition.Coordinates.X;
            this->point.Position.CartesianPosition.Y = actualPosition.Coordinates.Y;
            this->point.Position.CartesianPosition.Z = actualPosition.Coordinates.Z;
            this->point.Position.CartesianPosition.ThetaX= actualPosition.Coordinates.ThetaX;
            this->point.Position.CartesianPosition.ThetaY=actualPosition.Coordinates.ThetaY;
            this->point.Position.CartesianPosition.ThetaZ=actualPosition.Coordinates.ThetaZ;
        }else{
            QMessageBox* msgBox = new QMessageBox();
            msgBox->setWindowTitle(tr("Send Command"));
            msgBox->setText(tr("Something went wrong, couldn't get the actual position of the robot"));
            msgBox->exec();
        }
    }
}

void MainWindow::setHandPull(){
    this->setActualPosition();
    this->point.Position.Fingers.Finger1 = 23;
    this->point.Position.Fingers.Finger2 = 19;
    this->point.Position.Fingers.Finger3 = 29;
    this->point.Position.CartesianPosition.ThetaX = 1.5;
    this->point.Position.CartesianPosition.ThetaY = -0.1;
    this->point.Position.CartesianPosition.ThetaZ = -1.8;

}

void MainWindow::setHandPoint(){
    this->setActualPosition();

    this->point.Position.CartesianPosition.ThetaX=1.5;
    this->point.Position.CartesianPosition.ThetaY=-0.1;
    this->point.Position.CartesianPosition.ThetaZ=0;
    this->point.Position.Fingers.Finger1 = 47;
    this->point.Position.Fingers.Finger2 = 43;
    this->point.Position.Fingers.Finger3 = 54;
}

void MainWindow::setHandGrasp(){
    this->setActualPosition();
    this->point.Position.CartesianPosition.ThetaX = 2.9;
    this->point.Position.CartesianPosition.ThetaY = 0;
    this->point.Position.CartesianPosition.ThetaZ = -1.5;

    this->point.Position.Fingers.Finger1 = 27;
    this->point.Position.Fingers.Finger2 = 25;
    this->point.Position.Fingers.Finger3 = 25;
}

State convertOficialAPItoStatePoint(TrajectoryPoint p){
    State s(9);
    s[0] =  p.Position.CartesianPosition.X;
    s[1] =  p.Position.CartesianPosition.Y;
    s[2] =  p.Position.CartesianPosition.Z;
    s[3] =  p.Position.CartesianPosition.ThetaX;
    s[4] =  p.Position.CartesianPosition.ThetaY;
    s[5] =  p.Position.CartesianPosition.ThetaZ;
    s[6] =  p.Position.Fingers.Finger1;
    s[7] =  p.Position.Fingers.Finger2;
    s[8] =  p.Position.Fingers.Finger3;
    return s;
}

void MainWindow::on_playPointPosition_clicked()
{
    this->setHandPoint();
    if(KINOVA_LIB==1){
        State s = convertOficialAPItoStatePoint(this->point);
        this->bot->SendCommand(s,11); //move arm and hand using cartesian position
    }else{
        klib->sendCommand(CARTESIAN_POSITION,false,true,this->point);
    }
}

void MainWindow::on_playPullPosition_clicked()
{
    this->setHandPull();
    if(KINOVA_LIB==1){
        State s = convertOficialAPItoStatePoint(this->point);
        this->bot->SendCommand(s,11); //move arm and hand using cartesian position
    }else{
        klib->sendCommand(CARTESIAN_POSITION,true,true,this->point);
    }
}

void MainWindow::on_playGraspPosition_clicked()
{
    this->setHandGrasp();
    if(KINOVA_LIB==1){
        State s = convertOficialAPItoStatePoint(this->point);
        this->bot->SendCommand(s,11); //move arm and hand using cartesian position
    }else{
        klib->sendCommand(CARTESIAN_POSITION,true,true,this->point);
    }
}


void MainWindow::on_undoButton_clicked()
{

    this->actualPosition = this->checkpoints.back();
    this->checkpoints.pop_back();

    this->point.Position.CartesianPosition.X = this->actualPosition.Coordinates.X;
    this->point.Position.CartesianPosition.Y = this->actualPosition.Coordinates.Y;
    this->point.Position.CartesianPosition.Z = this->actualPosition.Coordinates.Z;
    this->point.Position.CartesianPosition.ThetaX= this->actualPosition.Coordinates.ThetaX;
    this->point.Position.CartesianPosition.ThetaY=this->actualPosition.Coordinates.ThetaY;
    this->point.Position.CartesianPosition.ThetaZ=this->actualPosition.Coordinates.ThetaZ;
    this->point.Position.Fingers.Finger1 = this->actualPosition.Fingers.Finger1;
    this->point.Position.Fingers.Finger2 = this->actualPosition.Fingers.Finger2;
    this->point.Position.Fingers.Finger3 = this->actualPosition.Fingers.Finger3;

    this->ui->undoButton->setDisabled(this->checkpoints.empty());

    klib->sendCommand(CARTESIAN_POSITION,true,true,this->point);
}

void MainWindow::on_MainWindow_destroyed()
{
    delete this->bot;
}

void MainWindow::on_pushButton_clicked()
{
    //State s = State(9);
    State s(9);
    s[0] = -0.05;
    s[1] = -0.46;
    s[2] = 0.45;
    s[3] = 1.5;
    s[4] = 0.2;
    s[5] = 0.0;
    s[6] = 0.0;
    s[7] = 0.0;
    s[8] = 0.0;
    cout << "before sending" << endl;
     this->bot->SendAndWait(s);
    cout << "after sending sendAndWait" << endl;
}

void MainWindow::on_pushButton_3_clicked()
{
    State starting(9);
    starting[0] = -0.05;
    starting[1] = -0.46;
    starting[2] = 0.45;
    starting[3] = 1.5;
    starting[4] = 0.2;
    starting[5] = 0.0;
    starting[6] = 0.0;
    starting[7] = 0.0;
    starting[8] = 0.0;
    Log cartPosLog = recordedLogs[this->readTypeMap["cart_pos"]];
    Log timeLog = recordedLogs[this->readTypeMap["comp_t"]];
    Log velLog = recordedLogs[this->readTypeMap["j_vel"]];
    vector<Log> values;
    values.push_back(cartPosLog);
    values.push_back(velLog);


    this->bot->ExecuteUpdatedTrajectory(timeLog,starting,values);
    cout << "after executing the traj" << endl;
}
