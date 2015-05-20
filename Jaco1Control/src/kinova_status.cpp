#include "kinova_status.hpp"


using namespace kinenv;

kinova_status::kinova_status()
{


}

kinova_status::kinova_status(int i)
{
	//DEBUG
		std::cout<<"dentro costruttore kinova_status"<<std::endl;
	 //---
	void * APIhandle = dlopen("Kinova.API.USBCommandLayerUbuntu.so",RTLD_NOW|RTLD_GLOBAL);
	MyGetGeneralInformations = (int (*)(GeneralInformations &)) dlsym(APIhandle,"GetGeneralInformations");
	MyGetGlobalTrajectoryInfo = (int (*)(TrajectoryFIFO &)) dlsym(APIhandle,"GetGlobalTrajectoryInfo");
	running = true;

	window.create(sf::VideoMode(400,400), "stats");
	sf::Font font;
	if (!font.loadFromFile("arial.ttf"))
	{
		std::cout<<"error in loading font"<<std::endl;// error...
	}
	// here i start my thread
	reader_stats = new boost::thread(boost::bind(&kinova_status::Reading,this));
	//DEBUG
	std::cout<<"fine costruttore kinova status"<<std::endl;
	//---
}

kinova_status::~kinova_status()
{
	this->reader_stats->join();
}

void kinova_status::Reading()
{
	int result;

	while(this->running)
	{
		GeneralInformations cur;
		result = (*MyGetGeneralInformations)(cur);
		//this->ReadTimeStamp(cur);
		//this->ReadJoints(cur);
		this->ReadCartesian(cur);
		//this->ReadCurrents(cur);
		//DEBUG
		//---

	}

}


void kinova_status::ReadTimeStamp(GeneralInformations & info)
{
	boost::posix_time::ptime now = boost::date_time::not_a_date_time;
	double rob_now;

	now = boost::posix_time::microsec_clock::universal_time();
	rob_now = info.TimeFromStartup;

	this->computer_time.push_back(now);
	this->robot_time.push_back(rob_now);

}
void kinova_status::ReadJoints(GeneralInformations & info)
{
	arma::vec app(6);

	app[0]=info.Position.Actuators.Actuator1;
	app[1]=info.Position.Actuators.Actuator2;
	app[2]=info.Position.Actuators.Actuator3;
	app[3]=info.Position.Actuators.Actuator4;
	app[4]=info.Position.Actuators.Actuator5;
	app[5]=info.Position.Actuators.Actuator6;
	this->ang_position.push_back(app);

	app[0]=info.Force.Actuators.Actuator1;
	app[1]=info.Force.Actuators.Actuator2;
	app[2]=info.Force.Actuators.Actuator3;
	app[3]=info.Force.Actuators.Actuator4;
	app[4]=info.Force.Actuators.Actuator5;
	app[5]=info.Force.Actuators.Actuator6;
	this->ang_torque.push_back(app);

}

void kinova_status::ReadCartesian(GeneralInformations & info)
{
	arma::vec app(6);

	app[0]=info.Position.CartesianPosition.X;
	app[1]=info.Position.CartesianPosition.Y;
	app[2]=info.Position.CartesianPosition.Z;
	app[3]=info.Position.CartesianPosition.ThetaX;
	app[4]=info.Position.CartesianPosition.ThetaY;
	app[5]=info.Position.CartesianPosition.ThetaZ;
	this->ang_position.push_back(app);

	//DEBUG
	/*for(unsigned int i =0;i<6;i++)
	{
		std::cout <<app[i]<<" ";
	}
	std::cout << std::endl;*/
	//---

	app[0]=info.Force.CartesianPosition.X;
	app[1]=info.Force.CartesianPosition.Y;
	app[2]=info.Force.CartesianPosition.Z;
	app[3]=info.Force.CartesianPosition.ThetaX;
	app[4]=info.Force.CartesianPosition.ThetaY;
	app[5]=info.Force.CartesianPosition.ThetaZ;
	this->ang_torque.push_back(app);


}
void kinova_status::ReadCurrents(GeneralInformations & info)
{
	arma::vec app(6);
	app[0]=info.Current.Actuators.Actuator1;
	app[1]=info.Current.Actuators.Actuator2;
	app[2]=info.Current.Actuators.Actuator3;
	app[3]=info.Current.Actuators.Actuator4;
	app[4]=info.Current.Actuators.Actuator5;
	app[5]=info.Current.Actuators.Actuator6;
}

void kinova_status::StopRun()
{
	this->running = false;
}


