#include "robot.hpp"

int main()
{


	// status reader
	/*dummybot* mdl = new dummybot();
	kinova_status * st= new kinova_status(mdl);

	// controller
	const double Pid_coef[] = {30,10,0}; // deg
	std::vector<double> Pid(Pid_coef,End(Pid_coef));
	std::string namefile = "ff.txt";
	const char * _meas_val[] ={"j_vel"};
	std::vector<std::string> meas_val(_meas_val,End(_meas_val));
	int controltype = 7;
	bool limitation = 1;
	Jaco* md = new Jaco();
	kinova_controller * ct = new kinova_controller(namefile,meas_val,Pid,controltype,limitation,md,st->APIhandle); // very rough patch because i can have only one API handle

	// checking module
	// define bounding box
	const double bb_point[] = {-0.6,-0.2,-0.4};
    const double bb_dims[]  = {1.2,0.4,0.7};
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
	robot bot(st,ct,checker);
	bot.Exec();*/

	// status reader
	Jaco* mdl = new Jaco();
	bool sync = false;
	std::string joint_base_name = "Jaco_joint";
	driverbot * vrep = new driverbot(sync,joint_base_name,mdl);

	// controller
	const double Pid_coef[] = {30,10,0}; // deg
	std::vector<double> Pid(Pid_coef,End(Pid_coef));
	std::string namefile = "ff.txt";
	const char * _meas_val[] ={"j_pos","j_vel"};
	std::vector<std::string> meas_val(_meas_val,End(_meas_val));
	int controltype = 1;
	bool limitation = 1;
	Jaco* md = new Jaco();
	driverbot_contr * ct = new driverbot_contr(namefile,meas_val,Pid,controltype,md,vrep->idclient,vrep->joint_handle); // very rough patch because i can have only one API handle
	safetycheck checker;
	robot bot(vrep,ct,checker);
	bot.Exec();


	return 1;
}
