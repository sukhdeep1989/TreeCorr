/* Copyright (c) 2003-2014 by Mike Jarvis
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
#ifndef Corr3_H
#define Corr3_H

#include <vector>
#include <string>

#include "Cell.h"
#include "Field.h"

template <int DC1, int DC2, int DC3>
struct ZetaData;

// BinnedCorr3 encapsulates a binned correlation function.
template <int DC1, int DC2, int DC3>
class BinnedCorr3
{

public:

    BinnedCorr3(double minsep, double maxsep, int nbins, double binsize, double b,
                double minu, double maxu, int nubins, double ubinsize, double bu,
                double minv, double maxv, int nvbins, double vbinsize, double bv,
                double* zeta0, double* zeta1, double* zeta2, double* zeta3,
                double* zeta4, double* zeta5, double* zeta6, double* zeta7,
                double* meanlogr, double* meanu, double* meanv, double* weight, double* ntri);
    BinnedCorr3(const BinnedCorr3& rhs, bool copy_data=true);
    ~BinnedCorr3();

    void clear();  // Set all data to 0.

    template <int M>
    void process(const Field<DC1, M>& field, bool dots);
    template <int M>
    void process(const Field<DC1, M>& field1, const Field<DC2, M>& field2, 
                 const Field<DC3, M>& field3, bool dots);

    // Main worker functions for calculating the result
    template <int M>
    void process3(const Cell<DC1,M>* c123);

    template <bool sort, int M>
    void process21(const Cell<DC1,M>* c12, const Cell<DC3,M>* c3);

    template <bool sort, int M>
    void process111(const Cell<DC1,M>* c1, const Cell<DC2,M>* c2, const Cell<DC3,M>* c3,
                    double d1sq=0., double d2sq=0., double d3sq=0.);

    template <bool sort, int M>
    void processU(const Cell<DC1,M>* c1, const Cell<DC2,M>* c2, const Cell<DC3,M>* c3,
                  const double d1sq, const double d2sq, const double d3sq, const double d2);

    template <bool sort, int M>
    void processV(const Cell<DC1,M>* c1, const Cell<DC2,M>* c2, const Cell<DC3,M>* c3,
                  const double d1sq, const double d2sq, const double d3sq,
                  const double d1, const double d2, const double d3,
                  const double logr, const double u, const int index);

    template <int M>
    void directProcessV(const Cell<DC1,M>& c1, const Cell<DC2,M>& c2, const Cell<DC3,M>& c3,
                        const double d1, const double d2, const double d3,
                        const double logr, const double u, const double v, const int index);

    // Note: op= only copies _data.  Not all the params.
    void operator=(const BinnedCorr3<DC1,DC2,DC3>& rhs);
    void operator+=(const BinnedCorr3<DC1,DC2,DC3>& rhs);

protected:

    double _minsep;
    double _maxsep;
    int _nbins;
    double _binsize;
    double _b;
    double _minu;
    double _maxu;
    int _nubins;
    double _ubinsize;
    double _bu;
    double _minv;
    double _maxv;
    int _nvbins;
    double _vbinsize;
    double _bv;
    double _logminsep;
    double _halfminsep;
    double _halfmind3;
    double _minsepsq;
    double _maxsepsq;
    double _minusq;
    double _maxusq;
    double _minvsq;
    double _maxvsq;
    double _bsq;
    double _busq;
    double _bvsq;
    double _sqrttwobv;
    int _metric; // Stores which Metric is being used for the analysis.
    int _nuv; // = nubins * nvbins
    int _ntot; // = nbins * nubins * nvbins

    // These are usually allocated in the python layer and just built up here.
    // So all we have here is a bare pointer for each of them.
    // However, for the OpenMP stuff, we do create copies that we need to delete.
    // So keep track if we own the data and need to delete the memory ourselves.
    bool _owns_data;

    // The different correlation functions have different numbers of arrays for zeta, 
    // so encapsulate that difference with a templated ZetaData class.
    ZetaData<DC1,DC2,DC3> _zeta;
    double* _meanlogr;
    double* _meanu;
    double* _meanv;
    double* _weight;
    double* _ntri;
};

template <int DC1, int DC2, int DC3>
struct ZetaData // This works for NNK, NKK, KKK
{
    ZetaData(double* zeta0, double*, double*, double*, double*, double*, double*, double*) :
        zeta(zeta0) {}

    void new_data(int n) { zeta = new double[n]; }
    void delete_data(int n) { delete [] zeta; zeta = 0; }
    void copy(const ZetaData<DC1,DC2,DC3>& rhs, int n) 
    { for (int i=0; i<n; ++i) zeta[i] = rhs.zeta[i]; }
    void add(const ZetaData<DC1,DC2,DC3>& rhs, int n) 
    { for (int i=0; i<n; ++i) zeta[i] += rhs.zeta[i]; }
    void clear(int n)
    { for (int i=0; i<n; ++i) zeta[i] = 0.; }
    void write(std::ostream& os) const // Just used for debugging.  Print the first value.
    { os << zeta[0]; }

    double* zeta;
};

template <int DC1, int DC2, int DC3>
inline std::ostream& operator<<(std::ostream& os, const ZetaData<DC1, DC2, DC3>& zeta)
{ zeta.write(os); return os; }

template <int DC1, int DC2>
struct ZetaData<DC1, DC2, GData> // This works for NNG, NKG, KKG
{
    ZetaData(double* z0, double* z1, double*, double*, double*, double*, double*, double*) :
        zeta(z0), zeta_im(z1) {}

    void new_data(int n) 
    {
        zeta = new double[n]; 
        zeta_im = new double[n]; 
    }
    void delete_data(int n) 
    {
        delete [] zeta; zeta = 0; 
        delete [] zeta_im; zeta_im = 0; 
    }
    void copy(const ZetaData<DC1,DC2,GData>& rhs, int n) 
    { 
        for (int i=0; i<n; ++i) zeta[i] = rhs.zeta[i]; 
        for (int i=0; i<n; ++i) zeta_im[i] = rhs.zeta_im[i]; 
    }
    void add(const ZetaData<DC1,DC2,GData>& rhs, int n) 
    {
        for (int i=0; i<n; ++i) zeta[i] += rhs.zeta[i]; 
        for (int i=0; i<n; ++i) zeta_im[i] += rhs.zeta_im[i]; 
    }
    void clear(int n)
    { 
        for (int i=0; i<n; ++i) zeta[i] = 0.;
        for (int i=0; i<n; ++i) zeta_im[i] = 0.;
    }
    void write(std::ostream& os) const 
    { os << zeta[0]<<','<<zeta_im[0]; }

    double* zeta;
    double* zeta_im;
};

template <int DC1>
struct ZetaData<DC1, GData, GData> // This works for NGG, KGG
{
    ZetaData(double* z0, double* z1, double* z2, double* z3, double*, double*, double*, double*) :
        zetap(z0), zetap_im(z1), zetam(z2), zetam_im(z3) {}

    void new_data(int n) 
    {
        zetap = new double[n]; 
        zetap_im = new double[n]; 
        zetam = new double[n]; 
        zetam_im = new double[n]; 
    }
    void delete_data(int n) 
    {
        delete [] zetap; zetap = 0; 
        delete [] zetap_im; zetap_im = 0; 
        delete [] zetam; zetam = 0; 
        delete [] zetam_im; zetam_im = 0; 
    }
    void copy(const ZetaData<DC1,GData,GData>& rhs, int n) 
    { 
        for (int i=0; i<n; ++i) zetap[i] = rhs.zetap[i]; 
        for (int i=0; i<n; ++i) zetap_im[i] = rhs.zetap_im[i]; 
        for (int i=0; i<n; ++i) zetam[i] = rhs.zetam[i]; 
        for (int i=0; i<n; ++i) zetam_im[i] = rhs.zetam_im[i]; 
    }
    void add(const ZetaData<DC1,GData,GData>& rhs, int n) 
    {
        for (int i=0; i<n; ++i) zetap[i] += rhs.zetap[i]; 
        for (int i=0; i<n; ++i) zetap_im[i] += rhs.zetap_im[i]; 
        for (int i=0; i<n; ++i) zetam[i] += rhs.zetam[i]; 
        for (int i=0; i<n; ++i) zetam_im[i] += rhs.zetam_im[i]; 
    }
    void clear(int n)
    { 
        for (int i=0; i<n; ++i) zetap[i] = 0.;
        for (int i=0; i<n; ++i) zetap_im[i] = 0.;
        for (int i=0; i<n; ++i) zetam[i] = 0.;
        for (int i=0; i<n; ++i) zetam_im[i] = 0.;
    }
    void write(std::ostream& os) const 
    { os << zetap[0]<<','<<zetap_im[0]<<','<<zetam[0]<<','<<zetam_im; }

    double* zetap;
    double* zetap_im;
    double* zetam;
    double* zetam_im;
};


template <>
struct ZetaData<GData, GData, GData>
{
    ZetaData(double* z0, double* z1, double* z2, double* z3,
             double* z4, double* z5, double* z6, double* z7) :
        zeta0(z0), zeta0_im(z1), zeta1(z2), zeta1_im(z3),
        zeta2(z0), zeta2_im(z1), zeta3(z2), zeta3_im(z3) {}

    void new_data(int n) 
    {
        zeta0 = new double[n]; 
        zeta0_im = new double[n]; 
        zeta1 = new double[n]; 
        zeta1_im = new double[n]; 
        zeta2 = new double[n]; 
        zeta2_im = new double[n]; 
        zeta3 = new double[n]; 
        zeta3_im = new double[n]; 
    }
    void delete_data(int n) 
    {
        delete [] zeta0; zeta0 = 0; 
        delete [] zeta0_im; zeta0_im = 0; 
        delete [] zeta1; zeta1 = 0; 
        delete [] zeta1_im; zeta1_im = 0; 
        delete [] zeta2; zeta2 = 0; 
        delete [] zeta2_im; zeta2_im = 0; 
        delete [] zeta3; zeta3 = 0; 
        delete [] zeta3_im; zeta3_im = 0; 
    }
    void copy(const ZetaData<GData,GData,GData>& rhs, int n) 
    { 
        for (int i=0; i<n; ++i) zeta0[i] = rhs.zeta0[i]; 
        for (int i=0; i<n; ++i) zeta0_im[i] = rhs.zeta0_im[i]; 
        for (int i=0; i<n; ++i) zeta1[i] = rhs.zeta1[i]; 
        for (int i=0; i<n; ++i) zeta1_im[i] = rhs.zeta1_im[i]; 
        for (int i=0; i<n; ++i) zeta2[i] = rhs.zeta2[i]; 
        for (int i=0; i<n; ++i) zeta2_im[i] = rhs.zeta2_im[i]; 
        for (int i=0; i<n; ++i) zeta3[i] = rhs.zeta3[i]; 
        for (int i=0; i<n; ++i) zeta3_im[i] = rhs.zeta3_im[i]; 
    }
    void add(const ZetaData<GData,GData,GData>& rhs, int n) 
    {
        for (int i=0; i<n; ++i) zeta0[i] += rhs.zeta0[i]; 
        for (int i=0; i<n; ++i) zeta0_im[i] += rhs.zeta0_im[i]; 
        for (int i=0; i<n; ++i) zeta1[i] += rhs.zeta1[i]; 
        for (int i=0; i<n; ++i) zeta1_im[i] += rhs.zeta1_im[i]; 
        for (int i=0; i<n; ++i) zeta2[i] += rhs.zeta2[i]; 
        for (int i=0; i<n; ++i) zeta2_im[i] += rhs.zeta2_im[i]; 
        for (int i=0; i<n; ++i) zeta3[i] += rhs.zeta3[i]; 
        for (int i=0; i<n; ++i) zeta3_im[i] += rhs.zeta3_im[i]; 
    }
    void clear(int n)
    { 
        for (int i=0; i<n; ++i) zeta0[i] = 0.;
        for (int i=0; i<n; ++i) zeta0_im[i] = 0.;
        for (int i=0; i<n; ++i) zeta1[i] = 0.;
        for (int i=0; i<n; ++i) zeta1_im[i] = 0.;
        for (int i=0; i<n; ++i) zeta2[i] = 0.;
        for (int i=0; i<n; ++i) zeta2_im[i] = 0.;
        for (int i=0; i<n; ++i) zeta3[i] = 0.;
        for (int i=0; i<n; ++i) zeta3_im[i] = 0.;
    }
    void write(std::ostream& os) const 
    { 
        os << zeta0[0]<<','<<zeta0_im[0]<<','<<zeta1[0]<<','<<zeta1_im<<','<<
            zeta2[0]<<','<<zeta2_im[0]<<','<<zeta3[0]<<','<<zeta3_im; 
    }

    double* zeta0;
    double* zeta0_im;
    double* zeta1;
    double* zeta1_im;
    double* zeta2;
    double* zeta2_im;
    double* zeta3;
    double* zeta3_im;
};

template <>
struct ZetaData<NData, NData, NData>
{
    ZetaData(double* , double* , double* , double*, double*, double*, double*, double * ) {}
    void new_data(int n) {}
    void delete_data(int n) {}
    void copy(const ZetaData<NData,NData,NData>& rhs, int n) {}
    void add(const ZetaData<NData,NData,NData>& rhs, int n) {}
    void clear(int n) {}
    void write(std::ostream& os) const {}
};


// The C interface for python
extern "C" {

    extern void* BuildNNNCorr(double minsep, double maxsep, int nbins, double binsize, double b,
                              double minu, double maxu, int nubins, double ubinsize, double bu,
                              double minv, double maxv, int nvbins, double vbinsize, double bv,
                              double* meanlogr, double* meanu, double* meanv, double* ntri);
#if 0
    extern void* BuildNKCorr(double minsep, double maxsep, int nbins, double binsize, double b,
                              double minu, double maxu, int nubins, double ubinsize, double bu,
                              double minv, double maxv, int nvbins, double vbinsize, double bv,
                              double* zeta,
                              double* meanlogr, double* meanu, double* meanv, double* ntri);
    extern void* BuildNGCorr(double minsep, double maxsep, int nbins, double binsize, double b,
                              double minu, double maxu, int nubins, double ubinsize, double bu,
                              double minv, double maxv, int nvbins, double vbinsize, double bv,
                              double* zeta, double* zeta_im,
                              double* meanlogr, double* meanu, double* meanv, double* ntri);
    extern void* BuildKKCorr(double minsep, double maxsep, int nbins, double binsize, double b,
                              double minu, double maxu, int nubins, double ubinsize, double bu,
                              double minv, double maxv, int nvbins, double vbinsize, double bv,
                              double* zeta,
                              double* meanlogr, double* meanu, double* meanv, double* ntri);
    extern void* BuildKGCorr(double minsep, double maxsep, int nbins, double binsize, double b,
                              double minu, double maxu, int nubins, double ubinsize, double bu,
                              double minv, double maxv, int nvbins, double vbinsize, double bv,
                              double* zeta, double* zeta_im,
                              double* meanlogr, double* meanu, double* meanv, double* ntri);
    extern void* BuildGGCorr(double minsep, double maxsep, int nbins, double binsize, double b,
                              double minu, double maxu, int nubins, double ubinsize, double bu,
                              double minv, double maxv, int nvbins, double vbinsize, double bv,
                              double* zetap, double* zetap_im, double* zetam, double* zetam_im,
                              double* meanlogr, double* meanu, double* meanv, double* ntri);
#endif

    extern void DestroyNNNCorr(void* corr);
#if 0
    extern void DestroyNKCorr(void* corr);
    extern void DestroyNGCorr(void* corr);
    extern void DestroyKKCorr(void* corr);
    extern void DestroyKGCorr(void* corr);
    extern void DestroyGGCorr(void* corr);
#endif

    extern void ProcessAutoNNNFlat(void* corr, void* field, int dots);
    extern void ProcessAutoNNNSphere(void* corr, void* field, int dots);
#if 0
    extern void ProcessAutoKKFlat(void* corr, void* field, int dots);
    extern void ProcessAutoKKSphere(void* corr, void* field, int dots);
    extern void ProcessAutoGGFlat(void* corr, void* field, int dots);
    extern void ProcessAutoGGSphere(void* corr, void* field, int dots);
#endif

    extern void ProcessCrossNNNFlat(void* corr, void* field1, void* field2, void* field3, int dots);
    extern void ProcessCrossNNNSphere(void* corr, void* field1, void* field2, void* field3, int dots);
#if 0
    extern void ProcessCrossNKFlat(void* corr, void* field1, void* field2, void* field3, int dots);
    extern void ProcessCrossNKSphere(void* corr, void* field1, void* field2, void* field3, int dots);
    extern void ProcessCrossNGFlat(void* corr, void* field1, void* field2, void* field3, int dots);
    extern void ProcessCrossNGSphere(void* corr, void* field1, void* field2, void* field3, int dots);
    extern void ProcessCrossKKFlat(void* corr, void* field1, void* field2, void* field3, int dots);
    extern void ProcessCrossKKSphere(void* corr, void* field1, void* field2, void* field3, int dots);
    extern void ProcessCrossKGFlat(void* corr, void* field1, void* field2, void* field3, int dots);
    extern void ProcessCrossKGSphere(void* corr, void* field1, void* field2, void* field3, int dots);
    extern void ProcessCrossGGFlat(void* corr, void* field1, void* field2, void* field3, int dots);
    extern void ProcessCrossGGSphere(void* corr, void* field1, void* field2, void* field3, int dots);
#endif
}

#endif
