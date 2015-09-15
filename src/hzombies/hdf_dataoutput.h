/*
 *
 * HDF5 Write
 * Created for SC15 Student Cluster Competition.
 *
 * Author: Byung H. Park, Oak Ridge National Laboratory
 *
 */
#ifndef HDF_DATA_OUTPUT_H
#define HDF_DATA_OUTPUT_H

#include "hdf5.h"

template<typename T>
class HdfDataOutput {
private:
    hid_t file_id, dset_id;
    hid_t filespace, memspace;
    hsize_t dimsf[2];
    hsize_t dimsm[2];
    hsize_t count[2];
    hsize_t block[2];
    hsize_t stride[2];
    hsize_t offset[2];
    hid_t plist_id;
    std::string filename;
    std::string dataname;
    hid_t h5_datatype;
    int mpi_rank;
    int xprocs;
    int worldsize_y;
    int iteration;
    bool closed;
    MPI_Comm communicator;
    int total_iterations;

public:
    HdfDataOutput(std::string fname, std::string dname, int myrank,hid_t type, MPI_Comm comm);
    virtual ~HdfDataOutput();
    void configure(int total_iterations, int block_size_y, int block_size_x, int num_procs_y, int num_procs_x, int rank);
    void write(T *data);
    void close();
};

template<typename T>
HdfDataOutput<T>::HdfDataOutput(std::string fname, std::string dname, int myrank, hid_t type, MPI_Comm comm) 
: filename(fname), dataname(dname), mpi_rank(myrank), h5_datatype(type), communicator(comm)
{
    iteration = 0;
    closed = false;
}

template<typename T>
HdfDataOutput<T>::~HdfDataOutput() 
{
    if ( !closed ) {
	H5Dclose(dset_id);
	H5Sclose(filespace);
	H5Sclose(memspace);
	H5Pclose(plist_id);
	H5Fclose(file_id);
    }
}

template<typename T>
void HdfDataOutput<T>::configure(int total, int block_size_y, int block_size_x, int num_procs_y, int num_procs_x, int rank)
{
    mpi_rank = rank;
    worldsize_y = block_size_y * num_procs_y;
    xprocs = num_procs_x;
    total_iterations = total;

    /**
     * Just default values...
     */
    plist_id = H5Pcreate(H5P_FILE_ACCESS);
    H5Pset_fapl_mpio(plist_id, communicator, MPI_INFO_NULL);

    file_id = H5Fcreate(filename.c_str(), H5F_ACC_TRUNC, H5P_DEFAULT, plist_id);
    H5Pclose(plist_id);

    dimsf[0] = worldsize_y * total_iterations;
    dimsf[1] = block_size_x * xprocs;

    dimsm[0] = block_size_y;
    dimsm[1] = block_size_x;

    filespace = H5Screate_simple(2, dimsf, NULL);
    memspace  = H5Screate_simple(2, dimsm, NULL);

    plist_id = H5Pcreate(H5P_DATASET_CREATE);
    H5Pset_chunk(plist_id, 2, dimsm);

    dset_id = H5Dcreate(file_id, dataname.c_str(), h5_datatype, filespace,
			H5P_DEFAULT, plist_id, H5P_DEFAULT);

    H5Pclose(plist_id);
    H5Sclose(filespace);

    block[0] = dimsm[0];
    block[1] = dimsm[1];

    // offset[0] will be set ...
    offset[1] = block[1] * (mpi_rank % xprocs);

    count[0]  = dimsm[0]; count[1]  = dimsm[1];

    stride[0] = stride[1] = 1;
    count[0] = count[1] = 1;

    filespace = H5Dget_space(dset_id);
    plist_id = H5Pcreate(H5P_DATASET_XFER);
    H5Pset_dxpl_mpio(plist_id, H5FD_MPIO_COLLECTIVE);
};

template<typename T>
void HdfDataOutput<T>::write(T *data)
{
    if ( iteration < total_iterations ) {
    offset[0] = worldsize_y * iteration + block[0] * (mpi_rank / xprocs);
    H5Sselect_hyperslab(filespace, H5S_SELECT_SET, offset, stride, count, block);
    herr_t status = H5Dwrite(dset_id, h5_datatype, memspace, filespace, plist_id, data);
    iteration++;
    }
}

template<typename T>
void HdfDataOutput<T>::close()
{
    H5Dclose(dset_id);
    H5Sclose(filespace);
    H5Sclose(memspace);
    H5Pclose(plist_id);
    H5Fclose(file_id);

    closed = true;
}

#endif
