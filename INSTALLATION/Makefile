# Repast HPC Manual Build Makefile

# VARIABLES (Supply values for these; for definitions and examples, see INSTALL)
CXX=
CXXLD=
BOOST_INCLUDE_DIR=boost/include
BOOST_LIB_DIR=boost/lib
BOOST_INFIX=-mt-s
NETCDF_INCLUDE_DIR=NetCDF/include
NETCDF_LIB_DIR=NetCDF/lib
CURL_INCLUDE_DIR=CURL/include
CURL_LIB_DIR=CURL/lib
HDF5_INCLUDE_DIR=hdf5/include
HDF5_LIB_DIR=hdf5/lib


# DIRECTORIES 
SRC_DIR=../src
REPAST_HPC_DIR=../src/repast_hpc
RELOGO_DIR=../src/relogo
RUMOR_DIR=../src/rumor_model
ZOMBIE_DIR=../src/zombies
HZOMBIE_DIR=../src/hzombies

# DERIVED VARIABLES (Do not modify)
I_REPAST=-I$(REPAST_HPC_DIR)
I_BOOST=-I$(BOOST_INCLUDE_DIR)
L_BOOST=-L$(BOOST_LIB_DIR)
l_BOOST=-lboost_mpi$(BOOST_INFIX) -lboost_serialization$(BOOST_INFIX) -lboost_system$(BOOST_INFIX) -lboost_filesystem$(BOOST_INFIX)
I_NETCDF=-I$(NETCDF_INCLUDE_DIR)
L_NETCDF=-L$(NETCDF_LIB_DIR)
l_NETCDF=-lnetcdf_c++ -lnetcdf
I_CURL=-I$(CURL_INCLUDE_DIR)
L_CURL=-L$(CURL_LIB_DIR)
l_CURL=-lcurl
I_HDF5=-I$(HDF5_INCLUDE_DIR)
L_HDF5=-L$(HDF5_LIB_DIR) -L$(Z_LIB_DIR)
l_HDF5=$(HDF5_LIB_DIR)/libhdf5_hl.a $(HDF5_LIB_DIR)/libhdf5.a -lz


# SOURCE FILES
# Repast HPC
REPAST_HPC_SOURCES=$(addprefix $(REPAST_HPC_DIR)/, \
AgentId.cpp           AgentImporterExporter.cpp  AgentRequest.cpp    AgentStatus.cpp \
Graph.cpp             GridComponents.cpp         GridDimensions.cpp  NCDataSet.cpp \
NCDataSetBuilder.cpp  NetworkBuilder.cpp         Properties.cpp      Random.cpp \
RepastErrors.cpp      RepastProcess.cpp          SRManager.cpp       SVDataSet.cpp \
SVDataSet.cpp         SVDataSetBuilder.cpp       Schedule.cpp        SharedContext.cpp \
SharedBaseGrid.cpp    SharedNetwork.cpp          Utilities.cpp       ValueLayer.cpp \
Variable.cpp          initialize_random.cpp      io.cpp              logger.cpp \
spatial_math.cpp )

# ReLogo
RELOGO_SOURCES=$(addprefix $(RELOGO_DIR)/, \
AbstractRelogoAgent.cpp   AgentSet.cpp    Observer.cpp                   Patch.cpp \
RandomMove.cpp            RelogoAgent.cpp RelogoContinuousSpaceAdder.cpp RelogoDiscreteSpaceAdder.cpp \
RelogoLink.cpp            Turtle.cpp      WorldCreator.cpp               WorldDefinition.cpp \
creators.cpp              utility.cpp )

# Rumor Model
RUMOR_SOURCES=$(addprefix $(RUMOR_DIR)/, \
RumorModel.cpp  main.cpp)

# Zombie Model
ZOMBIE_SOURCES=$(addprefix $(ZOMBIE_DIR)/, \
Human.cpp   InfectionSum.cpp   Zombie.cpp   ZombieObserver.cpp \
main.cpp)

# HZombie Model
HZOMBIE_SOURCES=$(addprefix $(HZOMBIE_DIR)/, \
main.cpp Zombie.cpp Human.cpp ZombieObserver.cpp InfectionSum.cpp)


# OBJECT FILES
REPAST_HPC_OBJECTS=$(patsubst %.cpp,%.o,$(REPAST_HPC_SOURCES))
RELOGO_OBJECTS=$(patsubst %.cpp,%.o,$(RELOGO_SOURCES))
RUMOR_OBJECTS=$(patsubst %.cpp,%.o,$(RUMOR_SOURCES))
ZOMBIE_OBJECTS=$(patsubst %.cpp,%.o,$(ZOMBIE_SOURCES))
HZOMBIE_OBJECTS=$(patsubst %.cpp,%.o,$(HZOMBIE_SOURCES))

# TARGETS
REPAST_VERSION=2.0
REPAST_HPC_NAME=repast_hpc-$(REPAST_VERSION)
REPAST_HPC_LIB=../lib/lib$(REPAST_HPC_NAME).a
RELOGO_NAME=relogo-$(REPAST_VERSION)
RELOGO_LIB=../lib/lib$(RELOGO_NAME).a
RUMOR_EXE=../bin/rumor_model
ZOMBIE_EXE=../bin/zombie_model
HZOMBIE_EXE=../bin/hzombie_model

l_REPAST_HPC=-l$(REPAST_HPC_NAME)
l_RELOGO=-l$(RELOGO_NAME)


# Default rule makes everything
.PHONY : all
all: copy_includes repast_hpc relogo rumor_model zombie_model hzombie_model

.PHONY : copy_includes
copy_includes:
	mkdir -p ../include/repast_hpc
	mkdir -p ../include/relogo
	cp $(REPAST_HPC_DIR)/*.h ../include/repast_hpc/
	cp $(RELOGO_DIR)/*.h ../include/relogo/

$(REPAST_HPC_LIB): $(REPAST_HPC_OBJECTS)
	mkdir -p ../lib
	rm -f $(REPAST_HPC_LIB)
	ar rc $(REPAST_HPC_LIB) $(REPAST_HPC_OBJECTS)
	ranlib $(REPAST_HPC_LIB)

$(RELOGO_LIB): $(RELOGO_OBJECTS)
	mkdir -p ../lib
	rm -f $(RELOGO_LIB)
	ar rc $(RELOGO_LIB) $(RELOGO_OBJECTS)
	ranlib $(RELOGO_LIB)

$(RUMOR_EXE): $(REPAST_HPC_LIB) $(RUMOR_OBJECTS)
	mkdir -p ../bin
	cp $(RUMOR_DIR)/config.props ../bin/rumor_config.props
	cp $(RUMOR_DIR)/model.props ../bin/rumor_model.props
	$(CXXLD) -L../lib/ $(L_BOOST) $(L_NETCDF) $(RUMOR_OBJECTS) $(l_NETCDF) $(l_REPAST_HPC) $(l_BOOST) -o $(RUMOR_EXE)

$(ZOMBIE_EXE): $(REPAST_HPC_LIB) $(RELOGO_LIB) $(ZOMBIE_OBJECTS)
	mkdir -p ../bin
	cp $(ZOMBIE_DIR)/config.props ../bin/zombie_config.props
	cp $(ZOMBIE_DIR)/model.props ../bin/zombie_model.props
	$(CXXLD) -L../lib/ $(L_BOOST) $(L_NETCDF) $(L_CURL) $(ZOMBIE_OBJECTS) $(l_NETCDF) $(l_CURL) $(l_RELOGO) $(l_REPAST_HPC) $(l_BOOST) -o $(ZOMBIE_EXE)

$(HZOMBIE_EXE): $(REPAST_HPC_LIB) $(RELOGO_LIB) $(HZOMBIE_OBJECTS)
	mkdir -p ../bin
	cp $(HZOMBIE_DIR)/config.props ../bin/hzombie_config.props
	cp $(HZOMBIE_DIR)/model.props ../bin/hzombie_model.props
	$(CXXLD) -L../lib/ $(L_BOOST) $(L_NETCDF) $(L_CURL) $(L_HDF5) $(HZOMBIE_OBJECTS) $(l_NETCDF) $(l_CURL) $(l_RELOGO) $(l_REPAST_HPC) $(l_BOOST) $(l_HDF5) -o $(HZOMBIE_EXE)

# User-friendly names
.PHONY : repast_hpc
repast_hpc: $(REPAST_HPC_LIB)
	
.PHONY : relogo
relogo: $(RELOGO_LIB)
	
.PHONY : rumor_model
rumor_model: $(RUMOR_EXE)
	
.PHONY : zombie_model
zombie_model: $(ZOMBIE_EXE)

.PHONY: hzombie_model
hzombie_model: $(HZOMBIE_EXE)

# Cleanup
.PHONY : repast_hpc_clean
repast_hpc_clean:
	rm -f $(REPAST_HPC_LIB)
	rm -f $(REPAST_HPC_DIR)/*.o

.PHONY : relogo_clean
relogo_clean :
	rm -f $(RELOGO_LIB)
	rm -f $(RELOGO_DIR)/*.o

.PHONY : rumor_clean
rumor_clean :
	rm -f $(RUMOR_EXE)
	rm -f $(RUMOR_DIR)/*.o

.PHONY : zombie_clean
zombie_clean :
	rm -f $(ZOMBIE_EXE)
	rm -f $(ZOMBIE_DIR)/*.o

.PHONY : hzombie_clean
hzombie_clean:
	rm -f $(HZOMBIE_EXE)
	rm -f $(HZOMBIE_DIR)/*.o

.PHONY : clean
clean: repast_hpc_clean relogo_clean rumor_clean zombie_clean hzombie_clean

.PHONY : distclean
distclean: clean
	rm -fr ../include
	rm -fr ../lib
	rm -fr ../bin


# Object files are built from cpp files
%.o:
	$(CXX) -c $(I_BOOST) $(I_NETCDF) $(I_CURL) $(I_HDF5) -I$(SRC_DIR) -o $@ $*.cpp
	
# Cancel implicit rule for name with no suffix, to avoid confusing output:
%: %.o
