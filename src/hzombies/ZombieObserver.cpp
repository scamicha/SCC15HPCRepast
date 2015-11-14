/*
 *  Modified, corrected, and extended for SC15 student competition
 *      Author: Byung H. Park, Oak Ridge National Laboratory
 *
 */


/*
 *   Repast for High Performance Computing (Repast HPC)
 *
 *   Copyright (c) 2010 Argonne National Laboratory
 *   All rights reserved.
 *
 *   Redistribution and use in source and binary forms, with
 *   or without modification, are permitted provided that the following
 *   conditions are met:
 *
 *     Redistributions of source code must retain the above copyright notice,
 *     this list of conditions and the following disclaimer.
 *
 *     Redistributions in binary form must reproduce the above copyright notice,
 *     this list of conditions and the following disclaimer in the documentation
 *     and/or other materials provided with the distribution.
 *
 *     Neither the name of the Argonne National Laboratory nor the names of its
 *     contributors may be used to endorse or promote products derived from
 *     this software without specific prior written permission.
 *
 *   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *   ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *   LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 *   PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE TRUSTEES OR
 *   CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 *   EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 *   PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 *   PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 *   LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 *   NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 *   EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *
 * ZombieObserver.cpp
 *
 *  Created on: Sep 1, 2010
 *      Author: nick
 *
 */
#include <sstream>

#include "relogo/RandomMove.h"
#include "relogo/grid_types.h"
#include "relogo/Patch.h"
#include "repast_hpc/AgentRequest.h"
#include "repast_hpc/SVDataSet.h"
#include "repast_hpc/SVDataSetBuilder.h"

#ifndef _WIN32
#include "repast_hpc/NCDataSetBuilder.h"
#endif

#include "ZombieObserver.h"
#include "Human.h"
#include "Zombie.h"
#include "InfectionSum.h"
#include "PatchUtil.h"

using namespace std;
using namespace repast;
using namespace relogo;

template<typename AgentType, typename T>
    void ZombieObserver::placeAgents(std::vector<Patch*> &patchSet, T *io_data)
{
    int horizontal_run = maxPxcor() - minPxcor() + 1;
    int lxmin = minPxcor();
    int lymin = minPycor();

    std::vector<repast::relogo::Patch*>::iterator iter = patchSet.begin();
    while ( iter != patchSet.end() ) {
	int i = (*iter)->pxCor() - lxmin;
	int j = (*iter)->pyCor() - lymin;

	int count = io_data[i + j*horizontal_run];
	AgentSet<AgentType> agentSet;
	int n = instantiate<AgentType>(count, agentSet);

	agentSet.apply(MoveToPatch(*iter));
	iter++;
    }
}

void ZombieObserver::gatherPatches(std::vector<Patch*>& patchSet)
{
    int px, py;
    int lxmin = minPxcor();
    int lxmax = maxPxcor();
    int lymin = minPycor();
    int lymax = maxPycor();

    for (px=lxmin; px <= lxmax; px++) 
      {
	  for (py=lymin; py <= lymax; py++)
	      {
		  repast::relogo::Patch *patch = patchAt<repast::relogo::Patch>(px,py);
		  patchSet.push_back(patch);
	      }
      }
}

/**
 *
 * write out the human (or zombie) counts in each grid cell.
 *
 */
template<typename AgentType, typename T>
    void ZombieObserver::gatherPopulation(std::vector<Patch*>& patchSet, T *io_data)
{
    int horizontal_run = maxPxcor() - minPxcor() + 1;

    std::vector<Patch*>::iterator iter = patchSet.begin();
    int lxmin = minPxcor();
    int lymin = minPycor();

    while ( iter != patchSet.end() ) {
        int i = (*iter)->pxCor() - lxmin;
        int j = (*iter)->pyCor() - lymin;

        AgentSet<AgentType> set;
	
        (*iter)->turtlesOn(set);
        io_data[i + j * horizontal_run] = int(set.size());
        iter++;
    }
}

template<typename AgentType>
    int ZombieObserver::instantiate(size_t count, repast::relogo::AgentSet<AgentType>& out) 
{
    int agentTypeId = create<AgentType>(count);
    const std::type_info* info = &(typeid(AgentType));
    int next_id = 0;

    std::map<const std::type_info*, int, TypeInfoCmp>::iterator iter = idMap.find(info);
    if (iter != idMap.end()) {
	next_id = iter->second;
    }

    for (int i=0; i<count; i++) {
	repast::AgentId agentid(next_id + i, _rank, agentTypeId);
	AgentType* agent = who<AgentType>(agentid);
	out.add(agent);
    }

    idMap[info] = (next_id + count);

    return agentTypeId;
}


void ZombieObserver::go() {
    
  if (_rank == 0) {
    Log4CL::instance()->get_logger("root").log(INFO, "TICK BEGINS: " + boost::lexical_cast<string>(RepastProcess::instance()->getScheduleRunner().currentTick()));
  }

  synchronize<AgentPackage>(*this, *this, *this, RepastProcess::USE_LAST_OR_USE_CURRENT);

  AgentSet<Zombie> zombies;
  get(zombies);
  zombies.ask(&Zombie::step);

  AgentSet<Human> humans;
  get(humans);
  humans.ask(&Human::step);

  if (_rank == 0) {
    Log4CL::instance()->get_logger("root").log(INFO, "TICK ENDS: " + boost::lexical_cast<string>(RepastProcess::instance()->getScheduleRunner().currentTick()));
  }
}


template<typename AgentType>
    void ZombieObserver::readAgents(Properties& props, std::string filename, std::string groupname, std::string dataname)
{
    int *data;
    int myorigin[2];
    int myextents[2];

    myorigin[0] = minPycor();
    myorigin[1] = minPxcor();
    myextents[0] = maxPycor() - minPycor() + 1;
    myextents[1] = maxPxcor() - minPxcor() + 1;
    
    int communicator = *(repast::RepastProcess::instance()->getCommunicator());

    std::vector<Patch*> patchSet;
    gatherPatches(patchSet);
    
    HdfDataInput<int> *data_in
	= new HdfDataInput<int>(props.getProperty(filename), "/", props.getProperty(dataname), 
				myorigin, myextents, H5T_STD_I32LE, communicator);

    data = new int[myextents[0]*myextents[1]];
    data_in->read(data);
    placeAgents<AgentType>(patchSet, data);
    data_in->close();
    delete data_in;
    delete data;
}


void ZombieObserver::snapshot()
{
    openOutputs();
    
    int size = ( maxPxcor()-minPxcor() + 1 ) * ( maxPycor() - minPycor() + 1 );
    
    std::vector<Patch*> patchSet;
    gatherPatches(patchSet);
    int *data = new int[size];

    gatherPopulation<Human>(patchSet, data);
    human_out->write(data);
    
    gatherPopulation<Zombie>(patchSet, data);
    zombie_out->write(data);

    delete data;

    closeOutputs();
}


void ZombieObserver::setupOutputs(Properties& props, std::string humanfile, std::string human_dataname, std::string zombiefile, std::string zombie_dataname)
{
    int total = strToInt(props.getProperty("stop.at")) / strToInt(props.getProperty("output.interval"));    
    int communicator = *(repast::RepastProcess::instance()->getCommunicator());

    int datasize[2];
    int myorigin[2];
    int myextents[2];
    
    datasize[0] =  repast::strToInt(props.getProperty("max.y")) + 1;
    datasize[1] =  repast::strToInt(props.getProperty("max.x")) + 1;
    
    myorigin[0] = minPycor();
    myorigin[1] = minPxcor();
    
    myextents[0] = maxPycor() - minPycor() + 1;
    myextents[1] = maxPxcor() - minPxcor() + 1;
    
    human_out = new HdfDataOutput<int>(props.getProperty(humanfile), props.getProperty(human_dataname), H5T_STD_I32LE, communicator);
    human_out->configure(total, datasize, myorigin, myextents);
    
    zombie_out = new HdfDataOutput<int>(props.getProperty(zombiefile), props.getProperty(zombie_dataname), H5T_STD_I32LE, communicator);
    zombie_out->configure(total, datasize, myorigin, myextents);
}


void ZombieObserver::openOutputs()
{
    int communicator = *(repast::RepastProcess::instance()->getCommunicator());

    int datasize[2];
    int myorigin[2];
    int myextents[2];
    
    datasize[0] = world_y;
    datasize[1] = world_x;
    
    myorigin[0] = minPycor();
    myorigin[1] = minPxcor();
    
    myextents[0] = maxPycor() - minPycor() + 1;
    myextents[1] = maxPxcor() - minPxcor() + 1;

    std::string suffix = "-" + boost::lexical_cast<string>(RepastProcess::instance()->getScheduleRunner().currentTick());
    
    human_out = new HdfDataOutput<int>(human_outfile + suffix, human_dataname, H5T_STD_I32LE, communicator);
    human_out->configure(1, datasize, myorigin, myextents);
    
    zombie_out = new HdfDataOutput<int>(zombie_outfile + suffix, zombie_dataname, H5T_STD_I32LE, communicator);
    zombie_out->configure(1, datasize, myorigin, myextents);
}


void ZombieObserver::closeOutputs()
{
    human_out->close();
    zombie_out->close();
    
    delete human_out;
    delete zombie_out;
}

void ZombieObserver::setup(Properties& props) {
    repast::Timer initTimer;
    initTimer.start();

    humanType = create<Human> (0);
    zombieType = create<Zombie> (0);

    readAgents<Human>(props,"human.input.file","/","human.dataname");
    readAgents<Zombie>(props,"zombie.input.file","/","zombie.dataname");

    human_outfile  = props.getProperty("human.output.file");
    human_dataname = props.getProperty("human.dataname");

    zombie_outfile  = props.getProperty("zombie.output.file");
    zombie_dataname = props.getProperty("zombie.dataname");

    world_y =  
	repast::strToInt(props.getProperty("max.y")) - 
	repast::strToInt(props.getProperty("min.y")) + 1;

    world_x =  
	repast::strToInt(props.getProperty("max.x")) -
	repast::strToInt(props.getProperty("min.x")) + 1;


    //setupOutputs(props, "human.output.file", "human.dataname", "zombie.output.file", "zombie.dataname");

    ScheduleRunner& runner = RepastProcess::instance()->getScheduleRunner();
    runner.scheduleEvent(1, strToInt(props.getProperty("output.interval")), repast::Schedule::FunctorPtr(new repast::MethodFunctor<ZombieObserver> (this, &ZombieObserver::snapshot)));
    //    runner.scheduleEndEvent(repast::Schedule::FunctorPtr(new repast::MethodFunctor<ZombieObserver> (this, &ZombieObserver::closeOutputs)));

    SVDataSetBuilder svbuilder("./output/data.csv", ",", repast::RepastProcess::instance()->getScheduleRunner().schedule());
    InfectionSum* iSum = new InfectionSum(this);
    svbuilder.addDataSource(repast::createSVDataSource("number_infected", iSum, std::plus<int>()));
    addDataSet(svbuilder.createDataSet());

#ifndef _WIN32
    // no netcdf under windows
    NCDataSetBuilder builder("./output/data.ncf", RepastProcess::instance()->getScheduleRunner().schedule());
    InfectionSum* infectionSum = new InfectionSum(this);
    builder.addDataSource(repast::createNCDataSource("number_infected", infectionSum, std::plus<int>()));
    addDataSet(builder.createDataSet());
#endif
    
    long double t = initTimer.stop();
    std::stringstream ss;
    ss << t;
    props.putProperty("init.time", ss.str());
}

RelogoAgent* ZombieObserver::createAgent(const AgentPackage& content) {
	if (content.type == zombieType) {
		return new Zombie(content.getId(), this);
	} else if  ( content.type == humanType ) {
		return new Human(content.getId(), this, content);
	}
	else {  // This was missing in the original code. Corrected by Hoony.
	        return new Patch(content.getId(), this);
	}
}

void ZombieObserver::provideContent(const repast::AgentRequest& request, std::vector<AgentPackage>& out) {
	const vector<AgentId>& ids = request.requestedAgents();
	for (int i = 0, n = ids.size(); i < n; i++) {
		AgentId id = ids[i];
		AgentPackage content = { id.id(), id.startingRank(), id.agentType(), id.currentRank(), 0, false };
		if (id.agentType() == humanType) {
			Human* human = who<Human> (id);
			content.infected = human->infected();
			content.infectionTime = human->infectionTime();
		}
		out.push_back(content);
	}
}

void ZombieObserver::provideContent(RelogoAgent* agent, std::vector<AgentPackage>& out) {
	AgentId id = agent->getId();
	AgentPackage content = { id.id(), id.startingRank(), id.agentType(), id.currentRank(), 0, false };
	if (id.agentType() == humanType) {
		Human* human = static_cast<Human*> (agent);
		content.infected = human->infected();
		content.infectionTime = human->infectionTime();
	}
	out.push_back(content);
}

void ZombieObserver::createAgents(std::vector<AgentPackage>& contents, std::vector<RelogoAgent*>& out) {
	for (size_t i = 0, n = contents.size(); i < n; ++i) {
		AgentPackage content = contents[i];
		if (content.type == zombieType) {
			out.push_back(new Zombie(content.getId(), this));
		} else if (content.type == humanType) {
			out.push_back(new Human(content.getId(), this, content));
		} else {
			// its a patch.
			out.push_back(new repast::relogo::Patch(content.getId(), this));
		}
	}
}

void ZombieObserver::updateAgent(AgentPackage package){
  repast::AgentId id(package.id, package.proc, package.type);
  if (id.agentType() == humanType) {
    Human * human = who<Human> (id);
    human->set(package.infected, package.infectionTime);
  }
}
