#include "sqlmanager.h"

SqlManager::SqlManager(){

}


int SqlManager::init(){
    bool databaseExist = false;
    int res = -1;
    this->database = QSqlDatabase::addDatabase("QSQLITE");
    QFile dbFile("/home/smaricha/qt_workspace/kinova-modules/kinova_root/KinovaAdvancedGUI/database/trajectoryProg.db");
    if(!dbFile.exists()){
        //TODO create database ??
    }else{
        databaseExist=true;
        this->database.setDatabaseName(dbFile.fileName());
    }

    if(databaseExist){
        if(this->database.open()){
            cout << "database opened" << endl;
        }
    }
    return res;
}

void SqlManager::getRecordedTrajectories(){
    //TODO
}

bool SqlManager::saveRecordedTrajectory(Trajectory &trajectory){
    QSqlQuery query(this->database);
    //FIXME change the sentence and use bindValue to se paramteres in the query
    QString strQuery = "INSERT INTO Trajectory (name,description) values('"+QString::fromStdString(trajectory.name)+"','"+QString::fromStdString(trajectory.description)+"')";

   if(!query.exec(strQuery)){
       qDebug() << "error inserting trajectory in database. Query was: " << strQuery << endl;
       return false;
   }

   //Obtain the autogenerated Trajectory.id
   int trajecoryId = query.lastInsertId().toInt();
   trajectory.id = trajecoryId;

   //Will store the CartesianInfo.id autogenerated in this vector
   vector<int> cartesianInfoIds;

   //Insert the Trajectory data in the table RecordedCartesianInfo
   for(vector<RecordedCartesianInfo>::iterator iter = trajectory.trajectoryInfo.begin(); iter!=trajectory.trajectoryInfo.end();++iter){
      RecordedCartesianInfo t = *iter;
      strQuery = "INSERT INTO CartesianInfo(pos_x,pos_y,pos_z,theta_x,theta_y,theta_z,finger1,finger2,finger3,angvel_j1,angvel_j2,angvel_j3,angvel_j4,angvel_j5,angvel_j6) "
                     "values ("+QString::number(t.pos_x)+ ","+ QString::number(t.pos_y) + ","+ QString::number(t.pos_z) + ","+ QString::number(t.theta_x) + ","+ QString::number(t.theta_y)
              + ","+ QString::number(t.theta_z) + ","+ QString::number(t.finger1) + ","+ QString::number(t.finger2) + ","+ QString::number(t.finger3) +  ","+ QString::number(t.angvel_j1)
              + ","+ QString::number(t.angvel_j2) +  ","+ QString::number(t.angvel_j3) + ","+ QString::number(t.angvel_j4) + ","+ QString::number(t.angvel_j5) +  ","+ QString::number(t.angvel_j6) + ")";

      if(!query.exec(strQuery)){
          qDebug() << "error inserting trajectory in database. Query was: " << strQuery << endl;
          return false;
      }else{
          int id = query.lastInsertId().toInt();
          cartesianInfoIds.push_back(id);
      }
   }

   //Fill the link table : trajectory_cartesianInfo
   int seq = 0;
   for(vector<int>::iterator iter = cartesianInfoIds.begin(); iter!= cartesianInfoIds.end();++iter){
       int idTrajectoryInfo = *iter;
       int timestamp = trajectory.trajectoryInfo[seq].timestamp;
       strQuery = "INSERT INTO trajectory_cartesianInfo(trajectory_id,cartesianInfo_id,seq,timestamp) values ("+QString::number(trajecoryId)+","+ QString::number(idTrajectoryInfo) +","+QString::number(seq)+","+QString::number(timestamp)+   ")";
       if(!query.exec(strQuery)){
           qDebug() << "error inserting trajectory in database. Query was: " << strQuery << endl;
           return false;
       }
       seq++;
   }

   return true;
}


/*Return the full trajectory including the cartesian positions*/
vector<Trajectory> SqlManager::getCompleteTrajectories(){
    vector<Trajectory> result;
    QSqlQuery query(this->database);
     QSqlQuery query2(this->database);
    QString strQuery = "SELECT * from Trajectory";
    if(query.exec(strQuery)){
        while(query.next()){
            cout << "loading.-.-" <<endl;
            Trajectory t;
            t.id = query.value(0).toInt();
            t.name = query.value(1).toString().toStdString();
            t.description = query.value(2).toString().toStdString();

            QString strQueryCartInfo = "select c.pos_x,c.pos_y,c.pos_z,c.theta_x,c.theta_y,c.theta_z,c.finger1,c.finger2,c.finger3,c.angvel_j1,c.angvel_j2,c.angvel_j3,c.angvel_j4,c.angvel_j5,c.angvel_j6,map.timestamp from CartesianInfo c, trajectory_cartesianInfo map  where map.trajectory_id="+ QString::number(t.id) +" and c.id=map.cartesianInfo_id";
            if(query2.exec(strQueryCartInfo)){
                while (query2.next()) {
                    RecordedCartesianInfo cartInfo;
                    cartInfo.pos_x = query2.value(0).toFloat();
                    cartInfo.pos_y = query2.value(1).toFloat();
                    cartInfo.pos_z = query2.value(2).toFloat();
                    cartInfo.theta_x = query2.value(3).toFloat();
                    cartInfo.theta_y= query2.value(4).toFloat();
                    cartInfo.theta_z = query2.value(5).toFloat();
                    cartInfo.finger1 = query2.value(6).toFloat();
                    cartInfo.finger2 = query2.value(7).toFloat();
                    cartInfo.finger3 = query2.value(8).toFloat();
                    cartInfo.angvel_j1 = query2.value(9).toFloat();
                    cartInfo.angvel_j2 = query2.value(10).toFloat();
                    cartInfo.angvel_j3 = query2.value(11).toFloat();
                    cartInfo.angvel_j4 = query2.value(12).toFloat();
                    cartInfo.angvel_j5 = query2.value(13).toFloat();
                    cartInfo.angvel_j6 = query2.value(14).toFloat();
                    cartInfo.timestamp = query2.value(15).toInt();
                  //  cout << "time from db: " <<  cartInfo.timestamp << endl;
                    t.trajectoryInfo.push_back(cartInfo);
                }
            }else{
                qDebug() << "Error loading cartesian info for trajectories! " <<endl;
            }

            result.push_back(t);
        }
    }else{
        qDebug() << "Error loading trajectories! " <<endl;
    }
    return result;
}


/*Just return the name and description of the trajectories*/
vector<Trajectory> SqlManager::getTrajectoriesInfo(){
    vector<Trajectory> result;
    QSqlQuery query(this->database);
    QString strQuery = "SELECT * from Trajectory";
    if(query.exec(strQuery)){
        while(query.next()){
            Trajectory t;
            t.id = query.value(0).toInt();
            t.name = query.value(1).toString().toStdString();
            t.description = query.value(2).toString().toStdString();
            result.push_back(t);
        }
    }else{
        qDebug() << "Error loading trajectories! " <<endl;
    }
    return result;
}

