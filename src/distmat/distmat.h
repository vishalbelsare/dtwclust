#ifndef DTWCLUST_DISTMAT_HPP_
#define DTWCLUST_DISTMAT_HPP_

#include <memory> // *_ptr

#define R_NO_REMAP
#include <R.h> // otherwise there's a warning because RcppParallel re-defines an error macro
#include <Rinternals.h>
#include <RcppParallel.h>

namespace dtwclust {

// =================================================================================================
/* Distmat (abstract + factory + concretes) */
// =================================================================================================

// -------------------------------------------------------------------------------------------------
/* abstract distmat */
// -------------------------------------------------------------------------------------------------
class Distmat
{
public:
    virtual ~Distmat() {};
    virtual double& operator() (const int i, const int j) = 0;
    virtual int nrow() const = 0;
    virtual int ncol() const = 0;
};

// -------------------------------------------------------------------------------------------------
/* concrete factory */
// -------------------------------------------------------------------------------------------------
class DistmatFactory
{
public:
    std::shared_ptr<Distmat> create(const SEXP& MAT_TYPE, const SEXP& D);
};

// -------------------------------------------------------------------------------------------------
/* R matrix distmat (thread-safe) */
// -------------------------------------------------------------------------------------------------
class RDistmat : public Distmat
{
public:
    RDistmat(const SEXP& D);
    double& operator() (const int i, const int j) override;
    int nrow() const override;
    int ncol() const override;

private:
    RcppParallel::RMatrix<double> distmat_;
};

} // namespace dtwclust

#endif // DTWCLUST_DISTMAT_HPP_
