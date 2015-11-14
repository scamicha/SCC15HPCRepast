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
    int worldsize_y;
    int iteration;
    bool closed;
    int communicator;
    int total_iterations;

public:
    HdfDataOutput(std::string fname, std::string dname, hid_t type, int comm);
    virtual ~HdfDataOutput();
    void configure(int total_iterations, int *datasize, int *origin, int *extents);
    void write(T *data);
    void close();
};

template<typename T>
HdfDataOutput<T>::HdfDataOutput(std::string fname, std::string dname, hid_t type, int comm) 
: filename(fname), dataname(dname), h5_datatype(type), communicator(comm)
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
void HdfDataOutput<T>::configure(int total, int *datasize, int *origin, int *extents)
{
    worldsize_y = datasize[0];
    total_iterations = total;

    /**
     * Just default values...
     */
    plist_id = H5Pcreate(H5P_FILE_ACCESS);
    H5Pset_fapl_mpio(plist_id, communicator, MPI_INFO_NULL);

    file_id = H5Fcreate(filename.c_str(), H5F_ACC_TRUNC, H5P_DEFAULT, plist_id);
    H5Pclose(plist_id);

    dimsf[0] = datasize[0] * total_iterations;
    dimsf[1] = datasize[1];

    dimsm[0] = extents[0]; 
    dimsm[1] = extents[1];

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

    offset[0] = origin[0];
    offset[1] = origin[1];

    stride[0] = stride[1] = 1;
    count[0] = count[1] = 1;

    filespace = H5Dget_space(dset_id);
    plist_id = H5Pcreate(H5P_DATASET_XFER);
    H5Pset_dxpl_mpio(plist_id, H5FD_MPIO_COLLECTIVE);
};

template<typename T>
void HdfDataOutput<T>::write(T *data)
{
    hsize_t new_offset[2];
    new_offset[1] = offset[1];
    new_offset[0] = worldsize_y * iteration + offset[0];

    if ( iteration < total_iterations ) {
	H5Sselect_hyperslab(filespace, H5S_SELECT_SET, new_offset, stride, count, block);
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
