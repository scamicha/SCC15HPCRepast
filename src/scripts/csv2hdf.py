#!/usr/bin/env python
import sys, getopt
import h5py
from numpy import genfromtxt

#----------------------------------------
# Simple csv to hdf5 converter
#
# Written for SC15 by Hoony Park, ORNL
#----------------------------------------

def csv2hdf(infile, outfile, dataname):
    data = genfromtxt(infile, delimiter=',')
    
    out_h5 = h5py.File(outfile, "w")
    dset = out_h5.create_dataset(dataname, data.shape, dtype='int32')
    
    for i in range(0, dset.shape[0]):
        dset[i,] = data[i,]
        
    out_h5.close()
        
        
def print_usage():
    print "csv2hdf.py -i <input csv> -o <output hdf5> -d data name"
            
def main(argv):
    infile = ''
    outfile = ''
    dataname = ''
    
    try:
        opts, args = getopt.getopt(argv,"hi:o:d:",["infile=","outfile=", "dataname="])
    except getopt.GetoptError:
        print_usage()
        
    if len(argv) < 4:
        print_usage()
	exit(1)
        
    for opt, arg in opts:
        if opt == '-h':
            print_usage()
	    exit(1)
        elif opt in ("-i", "--infile"):
            infile = arg
        elif opt in ("-o", "--outfile"):
            outfile = arg
        elif opt in ("-d", "--dataname"):
            dataname = arg
            
    csv2hdf(infile, outfile, dataname)

if __name__ == "__main__":
   main(sys.argv[1:])
