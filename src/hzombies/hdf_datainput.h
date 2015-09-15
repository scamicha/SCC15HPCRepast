/*
 *      HDF5 Read
 *      Created for SC 15 Student Cluster competition.
 *      Author: Byung H. Park, Oak Ridge National Laboratory
 */

#ifndef HDF_DATA_INPUT_H
#define HDF_DATA_INPUT_H

#include "hdf5.h"

template<typename T>
class HdfDataInput {
public:
private:
    hid_t filespace;
    int       mpi_rank;	

    hid_t       file_id, dset_id;
    hid_t       group_id;
    hid_t       dataspace, memspace;      /* file and memory dataspace identifiers */
    hsize_t     dimsf[3];                 /* dataset dimensions */
    hsize_t     dimsm[3];                 /* dataset dimensions */
    hsize_t	count[3];
    hsize_t     block[3];
    hsize_t     stride[3];
    hsize_t	offset[3];
    hid_t	plist_id;                 /* property list identifier */
    herr_t	status;
    hsize_t     num_rows, num_cols;
    hid_t       ndim;
    int         rindex, cindex, tindex;
    
    int         iteration;

    int xprocs;
    int yprocs;
    hid_t h5_datatype;
    MPI_Comm communicator;
public:
    HdfDataInput(int myrank, int procs_x, int procs_y, hid_t type, MPI_Comm comm);
    virtual ~HdfDataInput();
    int open(std::string filename, std::string groupname, std::string dataname, int *dim);
    int read(T*);
    int close();
};

template<typename T>
HdfDataInput<T>::HdfDataInput(int myrank, int procs_x, int procs_y, hid_t type, MPI_Comm comm) 
: mpi_rank(myrank), xprocs(procs_x), yprocs(procs_y), h5_datatype(type), communicator(comm)
{
    rindex = 0; cindex = 1; tindex = 2;
    count[0] = count[1] = count[2] = 1;
    block[0] = block[1] = block[2] = 1;
    stride[0] = stride[1] = stride[2] = 1;
}

template<typename T>
HdfDataInput<T>::~HdfDataInput() 
{
}

template<typename T>
int HdfDataInput<T>::open(std::string filename, std::string groupname, std::string dataname, int *dim)
{
    herr_t status_n;
    iteration = 0;

    /**
     * Just default values...
     */
    plist_id = H5Pcreate(H5P_FILE_ACCESS);
    if ( plist_id < 0 ) return plist_id;

    H5Pset_fapl_mpio(plist_id, communicator, MPI_INFO_NULL);

    /**
      Open a new file collectively and release property list identifier
      for later reuse..
    **/
    file_id = H5Fopen(filename.c_str(), H5F_ACC_RDONLY, plist_id);
    if ( file_id < 0 ) return file_id;

    H5Pclose(plist_id);

    /**
       Opens the group, and the data matrix.
    **/
    group_id = H5Gopen(file_id, groupname.c_str(), H5P_DEFAULT);
    dset_id  = H5Dopen(group_id, dataname.c_str(), H5P_DEFAULT);

    dataspace = H5Dget_space (dset_id);    /* dataspace handle */

    /**
       Read the dimension of the data. 
    **/
    ndim    = H5Sget_simple_extent_ndims (dataspace);
 
    if ( (status_n = H5Sget_simple_extent_dims (dataspace, dimsm, NULL)) < 0 ) 
	return status_n;

    num_rows = dimsm[rindex];
    num_cols = dimsm[cindex];

    block[rindex] = (num_rows / yprocs);
    block[cindex] = (num_cols / xprocs);

    dim[0] = block[rindex];
    dim[1] = block[cindex];

    dimsm[0] = block[rindex];
    dimsm[1] = block[cindex];
    memspace  = H5Screate_simple(2, dimsm, NULL); 
    plist_id = H5Pcreate(H5P_DATASET_XFER);

    return 1;
};

/**
 *
 * The main routine. It reads a set of 
 * data.
 *
 */
template<typename T>
int HdfDataInput<T>::read(T* data)
{
    /* this is the start row in the data matrix */

    offset[rindex] = block[rindex] * (mpi_rank / xprocs);
    offset[cindex] = block[cindex] * (mpi_rank % xprocs);

    H5Sselect_hyperslab(dataspace, H5S_SELECT_SET, offset, stride, count, block);
    status = H5Dread(dset_id, h5_datatype, memspace, dataspace, plist_id, data);

    iteration++;
    return status;
}

template<typename T>
int HdfDataInput<T>::close()
{
    H5Sclose(memspace);
    H5Dclose(dset_id);
    H5Sclose(dataspace);

    H5Gclose(group_id);
    H5Pclose(plist_id);
    H5Fclose(file_id);
    
    return 1;
}

#endif
