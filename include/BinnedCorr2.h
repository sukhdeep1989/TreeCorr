/* Copyright (c) 2003-2015 by Mike Jarvis
 *
 * TreeCorr is free software: redistribution and use in source and binary forms,
 * with or without modification, are permitted provided that the following
 * conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions, and the disclaimer given in the accompanying LICENSE
 *    file.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions, and the disclaimer given in the documentation
 *    and/or other materials provided with the distribution.
 */

#ifndef TreeCorr_Corr2_H
#define TreeCorr_Corr2_H

#include <vector>
#include <string>

#include "Cell.h"
#include "Field.h"

template <int D1, int D2>
struct XiData;

// BinnedCorr2 encapsulates a binned correlation function.
template <int D1, int D2>
class BinnedCorr2
{

public:

    BinnedCorr2(double minsep, double maxsep, int nbins, double binsize, double b,
                double minrpar, double maxrpar,
                double* xi0, double* xi1, double* xi2, double* xi3,
                double* meanr, double* meanlogr, double* weight, double* npairs);
    BinnedCorr2(const BinnedCorr2& rhs, bool copy_data=true);
    ~BinnedCorr2();

    void clear();  // Set all data to 0.

    template <int C, int M>
    void process(const Field<D1, C>& field, bool dots);
    template <int C, int M>
    void process(const Field<D1, C>& field1, const Field<D2, C>& field2, bool dots);
    template <int C, int M>
    void processPairwise(const SimpleField<D1, C>& field, const SimpleField<D2, C>& field2,
                         bool dots);

    // Main worker functions for calculating the result
    template <int C, int M>
    void process2(const Cell<D1,C>& c12);

    template <int C, int M>
    void process11(const Cell<D1,C>& c1, const Cell<D2,C>& c2);

    template <int C, int M>
    void directProcess11(const Cell<D1,C>& c1, const Cell<D2,C>& c2, const double dsq);

    // Note: op= only copies _data.  Not all the params.
    void operator=(const BinnedCorr2<D1,D2>& rhs);
    void operator+=(const BinnedCorr2<D1,D2>& rhs);

protected:

    double _minsep;
    double _maxsep;
    int _nbins;
    double _binsize;
    double _b;
    double _minrpar;
    double _maxrpar;
    double _logminsep;
    double _halfminsep;
    double _minsepsq;
    double _maxsepsq;
    double _bsq;
    int _coords; // Stores the kind of coordinates being used for the analysis.

    // These are usually allocated in the python layer and just built up here.
    // So all we have here is a bare pointer for each of them.
    // However, for the OpenMP stuff, we do create copies that we need to delete.
    // So keep track if we own the data and need to delete the memory ourselves.
    bool _owns_data;

    // The different correlation functions have different numbers of arrays for xi,
    // so encapsulate that difference with a templated XiData class.
    XiData<D1,D2> _xi;
    double* _meanr;
    double* _meanlogr;
    double* _weight;
    double* _npairs;

    double** _npairs_jack;
    double** _weight_jack;
    
};

template <int D1, int D2>
struct XiData // This works for NK, KK
{
    XiData(double* xi0, double*, double*, double*) : xi(xi0) {}

    void new_data(int n)
      {
        xi = new double[n];
        xi_jack=new double *[n_Jack];
        for (int jk=0; jk<n_Jack; jk++)
            xi_jack[jk]=new double [n];
          
      }
    
    void delete_data(int n)
     {
        delete [] xi; xi = 0;
         
        delete [][] xi_jack; xi_jack = 0;
     }
    
    void copy(const XiData<D1,D2>& rhs,int n)
     {
        for (int i=0; i<n; ++i)
          {
            xi[i] = rhs.xi[i];
            for (int jk=0; jk<n_Jack; jk++)
                xi_jack[jk][i] = rhs.xi_jack[jk][i];
          }
     }
    
    void add(const XiData<D1,D2>& rhs,int n)
      {
        for (int i=0; i<n; ++i)
         {
            xi[i] += rhs.xi[i];
            for (int jk=0; jk<n_Jack; jk++)
                xi_jack[jk][i] += rhs.xi_jack[jk][i];
         }
      }
    
    void clear(int n)
      {
        for (int i=0; i<n; ++i)
         {
            xi[i] = 0.;
             for (int jk=0; jk<n_Jack; jk++)
                 xi_jack[jk][i] = 0.;
         }
      }
    
    void write(std::ostream& os) const // Just used for debugging.  Print the first value.
    { os << xi[0]; }

    double* xi, **xi_jack;
};

template <int D1, int D2>
inline std::ostream& operator<<(std::ostream& os, const XiData<D1, D2>& xi)
{ xi.write(os); return os; }

template <int D1>
struct XiData<D1, GData> // This works for NG, KG
{
    XiData(double* xi0, double* xi1, double*, double*) : xi(xi0), xi_im(xi1) {}

    void new_data(int n)
    {
        xi = new double[n];
        xi_im = new double[n];
        xi_jack=new double *[n_Jack];
        xi_im_jack = new double *[n_Jack];
        for (int jk=0; jk<n_Jack; jk++){
            xi_jack[jk]=new double [n];
            xi_im_jack[jk]=new double [n];
        }
        
    }
    void delete_data(int n)
    {
        delete [] xi; xi = 0;
        delete [] xi_im; xi_im = 0;
        for (int jk=0; jk<n_Jack; jk++){
            delete [] xi_jack[jk];
            delete [] xi_im_jack[jk];
        }
        delete [] xi_jack; xi_jack = 0;
        delete [] xi_im_jack; xi_im_jack = 0;
    }
    void copy(const XiData<D1,GData>& rhs,int n)
    {
        for (int i=0; i<n; ++i)
         {
            xi[i] = rhs.xi[i];
            xi_im[i] = rhs.xi_im[i];
            for (int jk=0; jk<n_Jack; jk++)
            {
                xi_jack[jk][i] = rhs.xi_jack[jk][i];
                xi_im_jack[jk][i] = rhs.xi_im_jack[jk][i];
            }
        }
    }
    
    void add(const XiData<D1,GData>& rhs,int n)
    {
        for (int i=0; i<n; ++i)
         {
            xi[i] += rhs.xi[i];
            xi_im[i] += rhs.xi_im[i];
            for (int jk=0; jk<n_Jack; jk++)
            {
                xi[jk][i] += rhs.xi_im_jack[jk][i];
                xi_im_jack[jk][i] += rhs.xi_im_jack[jk][i];
            }
         }
    }
    
    void clear(int n)
    {
        for (int i=0; i<n; ++i)
        {
            xi[i] = 0.;
            xi_im[i] = 0.;
            for (int jk=0; jk<n_Jack; jk++)
            {
                xi_jack[jk][i] = 0.;
                xi_im_jack[jk][i] = 0.;
            }
        }
    }
    void write(std::ostream& os) const
    { os << xi[0]<<','<<xi_im[0]; }
    
    void operator+(XiData< D1, GData >& rhs);

    double* xi;
    double* xi_im;
    double** xi_jack;
    double** xi_im_jack;
};

template <>
struct XiData<GData, GData>
{
    XiData(double* xi0, double* xi1, double* xi2, double* xi3) :
        xip(xi0), xip_im(xi1), xim(xi2), xim_im(xi3) {}

    void new_data(int n)
    {
        xip = new double[n];
        xip_im = new double[n];
        xim = new double[n];
        xim_im = new double[n];
        
        xip_jack = new double*[n_Jack];
        xip_im_jack = new double*[n_Jack];
        xim_jack = new double*[n_Jack];
        xim_im_jack = new double*[n_Jack];
        for (int jk=0; jk<n_Jack; jk++)
        {
            xip_jack[jk] = new double[n];
            xim_jack[jk] = new double[n];
            xip_im_jack[jk] = new double[n];
            xim_im_jack[jk] = new double[n];
        }
        
    }
    void delete_data(int n)
    {
        delete [] xip; xip = 0;
        delete [] xip_im; xip_im = 0;
        delete [] xim; xim = 0;
        delete [] xim_im; xim_im = 0;
        for (int jk=0; jk<n_Jack; jk++)
        {
            delete [] xip_jack[jk];
            delete [] xip_im_jack[jk];
            delete [] xim_jack[jk];
            delete [] xim_im_jack[jk];
        }
        delete [] xip_jack; xip_jack = 0;
        delete [] xip_im_jack; xip_im_jack = 0;
        delete [] xim_jack; xim_jack = 0;
        delete [] xim_im_jack; xim_im_jack = 0;
    }
    void copy(const XiData<GData,GData>& rhs,int n)
    {
        for (int i=0; i<n; ++i)
        {
            xip[i] = rhs.xip[i];
            xip_im[i] = rhs.xip_im[i];
            xim[i] = rhs.xim[i];
            xim_im[i] = rhs.xim_im[i];
            for (int jk=0; jk<n_Jack; jk++)
            {
                xip_jack[jk][i] = rhs.xip_jack[jk][i];
                xip_im_jack[jk][i] = rhs.xip_im_jack[jk][i];
                xim_jack[jk][i] = rhs.xim_jack[jk][i];
                xim_im_jack[jk][i] = rhs.xim_im_jack[jk][i];
            }
        }
    }
    
    void add(const XiData<GData,GData>& rhs,int n)
    {
        for (int i=0; i<n; ++i)
        {
            xip[i] += rhs.xip[i];
            xip_im[i] += rhs.xip_im[i];
            xim[i] += rhs.xim[i];
            xim_im[i] += rhs.xim_im[i];
            for (int jk=0; jk<n_Jack; jk++)
            {
                xip_jack[jk][i] += rhs.xip_jack[jk][i];
                xip_im_jack[jk][i] += rhs.xip_im_jack[jk][i];
                xim_jack[jk][i] += rhs.xim_jack[jk][i];
                xim_im_jack[jk][i] += rhs.xim_im_jack[jk][i];
            }
        }
    }
    
    void clear(int n)
    {
        for (int i=0; i<n; ++i)
        {
            xip[i] = 0.;
            xip_im[i] = 0.;
            xim[i] = 0.;
            xim_im[i] = 0.;
            for (int jk=0; jk<n_Jack; jk++)
            {
                xip_jack[jk][i] = 0.;
                xip_im_jack[jk][i] = 0.;
                xim_jack[jk][i] = 0.;
                xim_im_jack[jk][i] = 0.;
            }
            
        }
    }
    void write(std::ostream& os) const
    { os << xip[0]<<','<<xip_im[0]<<','<<xim[0]<<','<<xim_im; }

    double* xip;
    double* xip_im;
    double* xim;
    double* xim_im;
    
    double** xip_jack;
    double** xip_im_jack;
    double** xim_jack;
    double** xim_im_jack;
};

template <>
struct XiData<NData, NData>
{
    XiData(double* , double* , double* , double* ) {}
    void new_data(int n) {}
    void delete_data(int n) {}
    void copy(const XiData<NData,NData>& rhs,int n) {}
    void add(const XiData<NData,NData>& rhs,int n) {}
    void clear(int n) {}
    void write(std::ostream& os) const {}
};


#endif
