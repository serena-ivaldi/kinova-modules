#include "visualization.hpp"

visualization::visualization()
{}
visualization::visualization(int NBLOCKS,int NJOINTS,int chunk_dim,PLFLT x_min,PLFLT x_max,PLFLT low_thresh,PLFLT high_thresh,
								std::vector<PLFLT> y_min,std::vector<PLFLT> y_max,std::vector<std::string> label,std::vector<std::string> title)
: data(NBLOCKS,NJOINTS,chunk_dim,x_min,x_max,low_thresh,high_thresh,y_min,y_max,label,title)
{
	// initialization stream
	pls = new plstream();
    pls->sdev("wxwidgets");
	pls->spage(0,0,800,900,0,0);
	pls->setopt("db", "");
	pls->setopt("np", "");
	pls->init();
	pls->ssub(1,data.NBLOCKS);
	cur_size = 0;
    // initialize vector of buffer
	this->time = boost::circular_buffer<PLFLT>(data.chunk_dim);
	buffer app(data.NJOINTS);
	for(int i =0;i<data.NJOINTS;i++)
	{
		app[i] = time;
	}
	for(int i =0;i<data.NBLOCKS;i++)
	{
		this->buf.push_back(app);
	}

}

visualization::visualization(data_vis data)
{
	this->data = data;
	// initialization strema
	pls = new plstream();
	pls->sdev("wxwidgets");
	pls->spage(0,0,800,900,0,0);
	pls->setopt("db", "");
	pls->setopt("np", "");
	pls->init();
	pls->ssub(1,data.NBLOCKS);
	cur_size = 0;
	// initialize vector of buffer
	this->time = boost::circular_buffer<PLFLT>(data.chunk_dim);
	buffer app(data.NJOINTS);
	for(int i =0;i<data.NJOINTS;i++)
	{
		app[i] = time;
	}
	for(int i =0;i<data.NBLOCKS;i++)
	{
		this->buf.push_back(app);
	}

}

// update order: time,joints,torque,current
void visualization::Update(std::vector<State_ptr> & lastval)
{
	// update time
	time.push_back(lastval[0]->at(0));
	// update the other values
	for(unsigned int i=0;i<buf.size();i++)
	{
		for(int j=0;j<data.NJOINTS;j++)
			// latval[i +1] because the first value is time
			buf[i][j].push_back(lastval[i + 1]->at(j));
	}

	// dynamic update of the graphs
	if(time.front()-data.x_min>data.low_thresh)
	{
		data.x_min = time.front();
	}
	if(data.x_max-time.back()<data.high_thresh)
		data.x_max = data.x_max+data.high_thresh;

	if(this->cur_size<data.chunk_dim)
		cur_size++;
}


void visualization::Plot()
{
	for(int i = 0; i < this->data.NBLOCKS;i++)
	{
		pls->adv(i + 1);
		pls->col0( 1 );
		pls->env0(data.x_min, data.x_max, data.y_min[i], data.y_max[i], 0, 0 );
		pls->col0( 2 );
		pls->lab("time",data.label[i].c_str(),data.title[i].c_str());
		for(int j = 0;j<data.NJOINTS;j++)
		{
			pls->col0( 3+j );
			pls->line(cur_size , time.linearize(), buf[i][j].linearize());
		}

	}
}
