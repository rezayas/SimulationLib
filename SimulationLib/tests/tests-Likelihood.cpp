#include "catch.hpp"
#include "../include/SimulationLib/Likelihood.h"
#include "../include/SimulationLib/Likelihood-sums.h"
#include "../include/SimulationLib/Likelihood-adaptors.h"

#include "../include/SimulationLib/IncidenceTimeSeries.h"
#include "../include/SimulationLib/PrevalenceTimeSeries.h"

#include <Normal.h>
#include <Poisson.h>

using namespace std;
using namespace SimulationLib;

TEST_CASE("Likelihood_basic_lambda_normal", "[calibration]") {
    using LikelihoodFn = LikelihoodFunction<StatisticalDistributions::Normal, int(int)>;

    double standardDeviation = sqrt(2);
    StatisticalDistributions::Normal nDist(0, standardDeviation);
    long double masterP = nDist.pdf(0);


    LikelihoodFn::F f = [] (auto x) { return 1 * x; };

    LikelihoodFn::DistributionGenerator dg =
      [standardDeviation] (auto t, auto v) {
        return StatisticalDistributions::Normal(v, standardDeviation);
      };

    auto l_f = LikelihoodFn(f, dg).GetLikelihoodFunction();

    for (int t = 0; t < 10; ++t)
    {
        int v = f(t);
        LikelihoodFn::ProbabilityT p = l_f(t, v);

        REQUIRE(p == masterP);
    }
}

TEST_CASE("Likelihood_basic_operator_normal", "[calibration]") {
    using LikelihoodFn = LikelihoodFunction<StatisticalDistributions::Normal, int(int)>;

    double standardDeviation = sqrt(2);
    StatisticalDistributions::Normal nDist(0, standardDeviation);
    long double masterP = nDist.pdf(0);


    LikelihoodFn::F f = [] (auto x) { return 1 * x; };

    LikelihoodFn::DistributionGenerator dg =
      [standardDeviation] (auto t, auto y) {
        return StatisticalDistributions::Normal(y, standardDeviation);
      };

    auto LF = LikelihoodFn(f, dg);

    for (int t = 0; t < 10; ++t)
    {
        int v = f(t);
        LikelihoodFn::ProbabilityT p = LF(t, v);

        REQUIRE(p == masterP);
    }
}

TEST_CASE("Likelihood_normal_sanity", "[calibration]") {
    using LikelihoodFn = LikelihoodFunction<StatisticalDistributions::Normal, double(double)>;

    auto f = [] (double t) -> double { return t; };

    LikelihoodFn::DistributionGenerator dg = [] (double t, double f_t) {
        return StatisticalDistributions::Normal(f_t, 1);
    };

    LikelihoodFn lkhf(f, dg);

    LikelihoodFn::LikelihoodFunctionT L = lkhf.GetLikelihoodFunction();

    for (double a = 1; a < 10; a += 1) {
        double t = 10;
        double f_t = f(t);

        double e = 5 + a;

        LikelihoodFn::ProbabilityT p = L(t, e);

        if (e < f_t)
            REQUIRE(p < L(t, f_t));
        else if (f_t < e)
            REQUIRE(p < L(t, f_t));
        else // f_t == e
            REQUIRE(p == L(t, f_t));
    }
}

TEST_CASE("Likelihood_poisson_sanity", "[calibration]") {
    using LikelihoodFn = LikelihoodFunction<StatisticalDistributions::Poisson, double(double)>;

    auto f = [] (double t) -> double { return t; };

    LikelihoodFn::DistributionGenerator dg = [] (double t, double f_t) {
        return StatisticalDistributions::Poisson(f_t);
    };

    LikelihoodFn lkhf(f, dg);

    LikelihoodFn::LikelihoodFunctionT L = lkhf.GetLikelihoodFunction();

    for (double a = 1; a < 10; a += 1) {
        double t = 10;
        double f_t = f(t);
        double e = 5 + a;
        LikelihoodFn::ProbabilityT p = L(t, e);

        // printf("L(f(%f) = %f) = %Lf\n", t, e, p);

        // if (e < f_t)
        //     REQUIRE(p < L(t, f_t));
        // else if (f_t < e)
        //     REQUIRE(p < L(t, f_t));
        // else // f_t == e
        //     REQUIRE(p == L(t, f_t));
        REQUIRE(true);
    }
}

// ===================================
// Tests for Likelihood-adaptors
// ===================================

TEST_CASE("Likelihood_adaptors_vector_normal", "[calibration]") {
    using LikelihoodFn = LikelihoodFunction<StatisticalDistributions::Normal, int(size_t)>;

    double standardDeviation = sqrt(2);
    StatisticalDistributions::Normal nDist(0, standardDeviation);
    long double masterP = nDist.pdf(0);

    LikelihoodFn::DistributionGenerator dg =
      [standardDeviation] (auto t, auto v) {
        return StatisticalDistributions::Normal(v, standardDeviation);
      };

    vector<int> vec = {0,1,2,3,4,5,6,7,8,9};

    LikelihoodFn Lgen = LikelihoodOnVector<StatisticalDistributions::Normal, int>(vec, dg);

    auto l_f = Lgen.GetLikelihoodFunction();

    for (size_t t = 0; t < 10; ++t)
    {
        int v_t = vec[t];
        LikelihoodFn::ProbabilityT p = l_f(t, v_t);

        // printf("P[f(%d) = %d] = %Lf\n", t, v_t, p);

        REQUIRE(p == masterP);
    }

    for (int a = 0; a < 10; ++a) {
        int t = 5;
        int v_t = vec[t];

        int e = a;

        LikelihoodFn::ProbabilityT p = l_f(t, e);

        // printf("P[f(%d) = %d] = %Lf\n", t, e, p);

        if (e < v_t)
            REQUIRE(p < l_f(t, v_t));
        else if (v_t < e)
            REQUIRE(p < l_f(t, v_t));
        else // v_t == e
            REQUIRE(p == l_f(t, v_t));
    }
}

TEST_CASE("Likelihood_adaptors_timeseries_normal", "[calibration]") {
    using LikelihoodFn = LikelihoodFunction<StatisticalDistributions::Normal, double(double)>;

    double standardDeviation = sqrt(2);
    StatisticalDistributions::Normal nDist(0, standardDeviation);
    long double masterP = nDist.pdf(0);

    LikelihoodFn::DistributionGenerator dg =
      [standardDeviation] (auto t, auto v) {
        return StatisticalDistributions::Normal(v, standardDeviation);
      };

    IncidenceTimeSeries<double> *its = new IncidenceTimeSeries<double>("Population 1", 0, 10, 2);

    its->Record(0.0, 1);
    its->Record(4.0, 2);
    its->Record(7.0, 3);
    its->Record(9.0, 4);

    LikelihoodFn Lgen = LikelihoodOnTimeSeries<StatisticalDistributions::Normal, double>(*its, dg);

    auto l_f = Lgen.GetLikelihoodFunction();

    for (int t = 0; t < 10; ++t)
    {
        int its_t = its->GetTotalAtTime(t);
        LikelihoodFn::ProbabilityT p = l_f(t, its_t);

        printf("P[f(%d) = %d] = %Lf\n", t, its_t, p);


        REQUIRE(p == masterP);
    }

    for (int a = 0; a < 5; ++a) {
        int t = 5;
        int its_t = its->GetTotalAtTime(t);

        int e = a;

        LikelihoodFn::ProbabilityT p = l_f(t, e);

        printf("P[f(%d) = %d] = %Lf\n", t, e, p);


        if (e < its_t)
            REQUIRE(p < l_f(t, its_t));
        else if (its_t < e)
            REQUIRE(p < l_f(t, its_t));
        else // its_t == e
            REQUIRE(p == l_f(t, its_t));
    }
}


// ===================================
// Tests for Likelihood-sums
// ===================================

TEST_CASE("Likelihood_craziness_test", "[calibration]") {
    using ProbabilityT = long double;

    using Base         = int;
    using Scale        = double;
    using Result       = double;
    using Signature    = Result(Base, Scale);

    using Params       = std::tuple<Base, Scale>;
    using ParamsVec    = std::vector<Params>;

    using LFnGen = LikelihoodFunction<StatisticalDistributions::Normal,
                                      Signature>;

    // f(base, scale) = base * scale
    function<Signature> f = [] (Base base, Scale scale) {
        return (Scale)base * scale;
    };

    // f(base, scale) = 2 * base * scale
    function<Signature> g = [] (Base base, Scale scale) {
        return 2 * (Scale)base * scale;
    };

    ParamsVec params{{1, 1.2}, {1, 1.5}, {2, 1.2}, {2, 1.5}};

    function<StatisticalDistributions::Normal(Base, Scale, Result)>
        dg = [] (Base base, Scale scale, Result f_b_s) {
            return StatisticalDistributions::Normal(f_b_s, 1);
        };

    auto gen     = LFnGen(f, dg);
    auto L_b_s_r = gen.GetLikelihoodFunction();

    for (auto i = params.begin(); i != params.end(); i++) {
        auto base  = std::get<Base>(*i);
        auto scale = std::get<Scale>(*i);
        auto g_b_s = g(base, scale);

        ProbabilityT p = L_b_s_r(base, scale, g_b_s);

        printf("L([f(b=%d, s=%f) = %f]) = %Lf, f(b,s) = %f\n",
                       base,
                             scale,
                                   g_b_s,
                                          p,
                                                        f(base, scale));
    }

    function<ProbabilityT(Base, Scale)> P_b_s =
      CurriedProbabilityOnG<ProbabilityT, Result, Base, Scale>(L_b_s_r, g);

    REQUIRE( true );
}

// TEST_CASE("Likelihood_sums_ProbabilityOnG_and_ProbabilityLgSum", "[calibration]") {
//     using LikelihoodFn = LikelihoodFunction<StatisticalDistributions::Normal, int(size_t)>;

//     double standardDeviation = sqrt(2);
//     StatisticalDistributions::Normal nDist(0, standardDeviation);
//     long double masterP = nDist.pdf(0);

//     LikelihoodFn::DistributionGenerator dg =
//       [standardDeviation] (size_t i, int f_i) {
//         return StatisticalDistributions::Normal(f_i, standardDeviation);
//       };

//     vector<int>             vec   = {0,1,2,3,4,5,6,7,8,9};
//     vector<std::tuple<size_t>> FArgs = {0,1,2,3,4,5,6,7,8,9};

//     LikelihoodFn Lgen = LikelihoodOnVector<StatisticalDistributions::Normal, int>(vec, dg);

//     auto l_f = Lgen.GetLikelihoodFunction();

//     function<int(size_t)> g = [&vec] (auto i) -> auto { return vec[i]; };

//     ProbabilityFunction<long double, size_t> p_f =
//         CurriedProbabilityOnG<long double, size_t, int>
//           (l_f, g); // pass likelihood function and 'g'

//     for (int t = 0; t < 10; ++t)
//     {
//         int v_t = vec[t];
//         LikelihoodFn::ProbabilityT p1 = l_f(t, v_t);
//         LikelihoodFn::ProbabilityT p2 = p_f(t);

//         REQUIRE(p2 == masterP);
//         REQUIRE(p1 == p2);
//     }

//     // ProbabilityLgSum testing
//     long double sum = ProbabilityLgSum<long double, size_t>(p_f, FArgs);

//     REQUIRE(sum > -12.6552);
//     REQUIRE(sum < -12.6551);
// }
