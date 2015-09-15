#!/bin/bash


# Note: Currently installs MPICH, NetCDF, NetCDF-CXX, and Boost version 1.54

cd ..
BASE_DIR=$PWD/ext/
cd INSTALLATION

MPI_COMPILER_INVOCATION=/N/soft/rhel6/openmpi/intel/1.8.4/bin/mpicxx


# cURL
if [[ $1 == *lcurl* ]]
then
  if [ -e $BASE_DIR/CURL ]
  then
    echo "A directory named $BASE_DIR/CURL already exists; you must delete it before it can be rebuilt."
    exit
  fi
  if [ ! -e curl-7.32.0.tar.gz ]
  then
    echo "CURL tar file (curl-7.32.0.tar.gz) not found; you must download this and put it in this directory to continue."
    exit
  fi 
  tar -xvf curl-7.32.0.tar.gz
  cd curl-7.32.0
  ./configure --prefix=$BASE_DIR/CURL
  make
  make install
  cd ..
fi


# MPICH
if [[ $1 == *mpich* ]]
then
  if [ -e $BASE_DIR/MPICH ]
  then
    echo "A directory named $BASE_DIR/MPICH already exists; you must delete it before it can be rebuilt."
    exit
  fi
  if [ ! -e mpich2-1.4.1p1.tar.gz ]
  then
    echo "MPICH tar file (mpich2-1.4.1p1.tar.gz) not found; you must download this and put it in this directory to continue."
    exit
  fi 
  tar -xvf mpich2-1.4.1p1.tar.gz
  cd mpich2-1.4.1p1
  ./configure --prefix=$BASE_DIR/MPICH --disable-f77 --disable-fc
  make
  make install
  cd ..
  export PATH=$BASE_DIR/MPICH/bin/:$PATH
fi

# NETCDF

if [[ $1 == *netcdf* ]]
then
  if [ -e $BASE_DIR/NetCDF ]
  then
    echo "A directory named $BASE_DIR/NetCDF already exists; you must delete it before it can be rebuilt."
    exit
  fi
  if [ ! -e netcdf-4.2.1.1.tar.gz ]
  then
    echo "NetCDF tar file (netcdf-4.2.1.1.tar.gz) not found; you must download this and put it in this directory to continue."
    exit
  fi 
  mkdir $BASE_DIR/NetCDF
  tar -xvf netcdf-4.2.1.1.tar.gz
  cd netcdf-4.2.1.1
  ./configure --disable-netcdf-4 --prefix=$BASE_DIR/NetCDF
  make
  make install
  cd ..

# NetCDF C++

  if [[ $1 == *netcpp* ]]
  then
    if [ ! -e netcdf-cxx-4.2.tar.gz ]
    then
      echo "NetCDF cpp tar file (netcdf-cxx-4.2.tar.gz) not found; you must download this and put it in this directory to continue."
      exit
    fi 
    tar -xvf netcdf-cxx-4.2.tar.gz
    cd netcdf-cxx-4.2
    env CPPFLAGS=-I$BASE_DIR/NetCDF/include LDFLAGS=-L$BASE_DIR/NetCDF/lib ./configure --prefix=$BASE_DIR/NetCDF
    make
    make install
    cd ..
  fi

fi


# Boost

if [[ $1 == *boost* ]]
then
if [ -e $BASE_DIR/Boost ]
  then
    echo "A directory named $BASE_DIR/Boost already exists; you must delete it before it can be rebuilt."
    exit
  fi
  if [ ! -e boost_1_54_0.tar.gz ]
  then
    echo "Boost tar file (boost_1_54_0.tar.gz) not found; you must download this and put it in this directory to continue."
    exit
  fi
  tar -xvf boost_1_54_0.tar.gz
  mkdir -p $BASE_DIR/Boost/Boost_1.54/include
  cp -r ./boost_1_54_0/boost $BASE_DIR/Boost/Boost_1.54/include
  cd boost_1_54_0
  ./bootstrap.sh --prefix=$BASE_DIR/Boost/Boost_1.54/ --with-libraries=system,filesystem,mpi,serialization
  echo "using mpi : $MPI_COMPILER_INVOCATION ;" >>./tools/build/v2/user-config.jam
  ./b2 --layout=tagged link=static variant=release threading=multi runtime-link=static stage install
  cd ..
fi

# Repast HPC
if [[ $1 == *rhpc* ]]
then
  make -f Makefile CXX=$MPI_COMPILER_INVOCATION CXXLD="$MPI_COMPILER_INVOCATION -static" BOOST_INCLUDE_DIR=$BASE_DIR/Boost/Boost_1.54/include BOOST_LIB_DIR=$BASE_DIR/Boost/Boost_1.54/lib BOOST_INFIX=-mt-s NETCDF_INCLUDE_DIR=$BASE_DIR/NetCDF/include NETCDF_LIB_DIR=$BASE_DIR/NetCDF/lib CURL_INCLUDE_DIR=$BASE_DIR/CURL/include CURL_LIB_DIR=$BASE_DIR/CURL/lib
fi

