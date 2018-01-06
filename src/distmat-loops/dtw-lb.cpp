#include "distmat-loops.h"

#include <string>

#include <RcppArmadillo.h>

#include "../distance-calculators/distance-calculators.h"

namespace dtwclust {

// =================================================================================================
/* find nearest neighbors row-wise */
// =================================================================================================

void set_nn(const Rcpp::NumericMatrix& distmat, Rcpp::IntegerVector& nn, const int margin)
{
    if (margin == 1) {
        for (int i = 0; i < distmat.nrow(); i++) {
            double d = distmat(i,0);
            nn[i] = 0;
            for (int j = 1; j < distmat.ncol(); j++) {
                double temp = distmat(i,j);
                if (temp < d) {
                    d = temp;
                    nn[i] = j;
                }
            }
        }
    } else {
        for (int j = 0; j < distmat.ncol(); j++) {
            double d = distmat(0,j);
            nn[j] = 0;
            for (int i = 1; i < distmat.nrow(); i++) {
                double temp = distmat(i,j);
                if (temp < d) {
                    d = temp;
                    nn[j] = i;
                }
            }
        }
    }
}

// =================================================================================================
/* check if updates are finished based on indices */
// =================================================================================================

bool check_finished(const Rcpp::IntegerVector& nn,
                    const Rcpp::IntegerVector& nn_prev,
                    Rcpp::LogicalVector& changed)
{
    bool finished = true;
    for (int i = 0; i < nn.length(); i++) {
        if (nn[i] != nn_prev[i]) {
            changed[i] = true;
            finished = false;

        } else {
            changed[i] = false;
        }
    }
    return finished;
}

// =================================================================================================
/* main C++ function */
// =================================================================================================

void dtw_lb_cpp(const Rcpp::List& X,
                const Rcpp::List& Y,
                Rcpp::NumericMatrix& distmat,
                const SEXP& DOTS,
                const int margin)
{
    std::string dist = "DTW_BASIC";
    auto dist_calculator = DistanceCalculatorFactory().create(dist, DOTS, X, Y);

    int len = margin == 1 ? distmat.nrow() : distmat.ncol();
    Rcpp::IntegerVector id_nn(len), id_nn_prev(len);
    Rcpp::LogicalVector id_changed(len);

    set_nn(distmat, id_nn, margin);
    for (int i = 0; i < id_nn.length(); i++) id_nn_prev[i] = id_nn[i] + 1; // initialize different

    while (!check_finished(id_nn, id_nn_prev, id_changed)) {
        Rcpp::checkUserInterrupt();

        // update nn_prev
        for (int i = 0; i < id_nn.length(); i++) id_nn_prev[i] = id_nn[i];

        // calculate dtw distance if necessary
        if (margin == 1) {
            for (int i = 0; i < id_changed.length(); i++) {
                if (id_changed[i]) {
                    int j = id_nn[i];
                    distmat(i,j) = dist_calculator->calculate(i, j);
                }
            }
        } else {
            for (int j = 0; j < id_changed.length(); j++) {
                if (id_changed[j]) {
                    int i = id_nn[j];
                    distmat(i,j) = dist_calculator->calculate(i, j);
                }
            }
        }

        // update nearest neighbors
        set_nn(distmat, id_nn, margin);
    }
}

// =================================================================================================
/* main gateway function */
// =================================================================================================

RcppExport SEXP dtw_lb(SEXP X, SEXP Y, SEXP D, SEXP MARGIN, SEXP DOTS)
{
    BEGIN_RCPP
    Rcpp::NumericMatrix distmat(D);
    dtw_lb_cpp(X, Y, distmat, DOTS, Rcpp::as<int>(MARGIN));
    return R_NilValue;
    END_RCPP
}

} // namespace dtwclust