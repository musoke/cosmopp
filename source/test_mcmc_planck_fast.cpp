#include <vector>
#include <string>
#include <sstream>
#include <ctime>

#include <test_mcmc_planck_fast.hpp>
#include <mcmc.hpp>
#include <planck_like_fast.hpp>
#include <markov_chain.hpp>
#include <numerics.hpp>
#include <timer.hpp>

std::string
TestMCMCPlanckFast::name() const
{
    return std::string("MCMC FAST PLANCK LIKELIHOOD TESTER");
}

unsigned int
TestMCMCPlanckFast::numberOfSubtests() const
{
    return 1;
}

void
TestMCMCPlanckFast::runSubTest(unsigned int i, double& res, double& expected, std::string& subTestName)
{
    check(i >= 0 && i < 1, "invalid index " << i);
    
    using namespace Math;

    std::string root = "slow_test_files/mcmc_planck_fast_test";

    const double pivot = 0.05;
    LambdaCDMParams par(0.022, 0.12, 0.7, 0.1, 1.0, std::exp(3.0) / 1e10, pivot);

#ifdef COSMO_PLANCK_15
    //PlanckLikeFast planckLike(&par, true, true, true, false, true, false, false, false, 5, 0.4, 10000);
    PlanckLikeFast planckLike(&par, true, true, true, false, false, false, false, false, 5, 0.4, 10000);
    const int nPar = 22;
#else
    PlanckLikeFast planckLike(&par, true, true, false, true, false, false, 5, 0.4, 10000);
    const int nPar = 20;
#endif

    MetropolisHastings mh(nPar, planckLike, root, std::time(0), true);

    std::string errorLogRoot = "slow_test_files/mcmc_planck_fast_error_log";
    planckLike.logError(errorLogRoot.c_str());

    mh.setParam(0, "ombh2", 0.005, 0.1, 0.022, 0.0003, 0.00005);
    mh.setParam(1, "omch2", 0.001, 0.99, 0.12, 0.003, 0.0005);
    mh.setParam(2, "h", 0.2, 1.0, 0.7, 0.02, 0.002);
    mh.setParam(3, "tau", 0.01, 0.8, 0.1, 0.01, 0.002);
    mh.setParam(4, "ns", 0.9, 1.1, 1.0, 0.01, 0.002);
    mh.setParam(5, "As", 2.7, 4.0, 3.0, 0.1, 0.002);

#ifdef COSMO_PLANCK_15
    mh.setParamGauss(6, "A_planck", 1.0, 0.0025, 1.0, 0.001, 0.0002);

    mh.setParam(7, "A_cib_217", 0, 200, 60, 5, 5);
    mh.setParam(8, "cib_index", -1.301, -1.299, -1.3, 0.00001, 0.00001);
    mh.setParam(9, "xi_sz_cib", 0, 1, 0.5, 0.3, 0.3);
    mh.setParam(10, "A_sz", 0, 10, 5, 2, 2);
    mh.setParam(11, "ps_A_100_100", 0, 400, 250, 30, 30);
    mh.setParam(12, "ps_A_143_143", 0, 400, 50, 10, 10);
    mh.setParam(13, "ps_A_143_217", 0, 400, 40, 10, 10);
    mh.setParam(14, "ps_A_217_217", 0, 400, 100, 10, 10);
    mh.setParam(15, "k_sz", 0, 10, 3, 2.5, 2.5);
    mh.setParamGauss(16, "gal545_A_100", 7, 2, 7.5, 2, 2);
    mh.setParamGauss(17, "gal545_A_143", 9, 2, 9, 2, 2);
    mh.setParamGauss(18, "gal545_A_143_217", 21, 8.5, 17, 4, 4);
    mh.setParamGauss(19, "gal545_A_217", 80, 20, 80, 7, 7);
    mh.setParamGauss(20, "calib_100T", 0.999, 0.001, 0.998, 0.001);
    mh.setParamGauss(21, "calib_217T", 0.995, 0.002, 0.996, 0.001);
#else
    mh.setParam(6, "A_ps_100", 0, 360, 100, 100, 20);
    mh.setParam(7, "A_ps_143", 0, 270, 50, 20, 2);
    mh.setParam(8, "A_ps_217", 0, 450, 100, 30, 4);
    mh.setParam(9, "A_cib_143", 0, 20, 10, 10, 1);
    mh.setParam(10, "A_cib_217", 0, 80, 30, 15, 1);
    mh.setParam(11, "A_sz", 0, 10, 5, 5, 1);
    mh.setParam(12, "r_ps", 0.0, 1.0, 0.9, 0.2, 0.02);
    mh.setParam(13, "r_cib", 0.0, 1.0, 0.4, 0.4, 0.05);
    mh.setParam(14, "n_Dl_cib", -2, 2, 0.5, 0.2, 0.02);
    mh.setParam(15, "cal_100", 0.98, 1.02, 1.0, 0.0008, 0.0001);
    mh.setParam(16, "cal_127", 0.95, 1.05, 1.0, 0.003, 0.0002);
    mh.setParam(17, "xi_sz_cib", 0, 1, 0.5, 0.6, 0.05);
    mh.setParam(18, "A_ksz", 0, 10, 5, 6, 0.5);
    mh.setParam(19, "Bm_1_1", -20, 20, 0.5, 1.0, 0.1);
#endif

    Timer timer("MCMC PLANCK FAST");

    const unsigned long burnin = 500;
    timer.start();
    const int nChains = mh.run(25000, 1, burnin, MetropolisHastings::GELMAN_RUBIN, 0.01, true);
    const unsigned long time = timer.end();
    output_screen("MCMC Planck fast took " << time / 1000000 << " seconds." << std::endl);
    
    subTestName = std::string("standard_param_limits");
    res = 1;
    expected = 1;

    if(!isMaster())
        return;

    const unsigned int thin = 2;
    MarkovChain chain(nChains, root.c_str(), burnin, thin, errorLogRoot.c_str());

    const int nPoints = 1000;

#ifdef COSMO_PLANCK_15
    const double expectedMedian[6] = {0.02222, 0.1197, 0.6731, 0.078, 0.9655, 3.089};
    const double expectedSigma[6] = {0.00023, 0.0022, 0.0096, 0.019, 0.0062, 0.036};
#else
    const double expectedMedian[6] = {0.02205, 0.1199, 0.673, 0.089, 0.9603, 3.089};
    const double expectedSigma[6] = {0.00028, 0.0027, 0.012, 0.013, 0.0073, 0.025};
#endif

    std::ofstream outParamLimits("slow_test_files/mcmc_planck_fast_param_limits.txt");
    for(int i = 0; i < nPar; ++i)
    {
        const std::string& paramName = mh.getParamName(i);
        std::stringstream fileName;
        fileName << "slow_test_files/mcmc_planck_fast_" << paramName << ".txt";
        Posterior1D* p = chain.posterior(i, Posterior1D::GAUSSIAN_SMOOTHING);

        p->writeIntoFile(fileName.str().c_str(), nPoints, true);

        const double median = p->median();
        double lower, upper;
        p->get1SigmaTwoSided(lower, upper);
        const double sigma = (upper - lower) / 2.0;

        outParamLimits << paramName << " = " << median << "+-" << sigma << std::endl;
        // check the standard cosmological parameter limits
        if(i < 6)
        {
            if(std::abs(expectedMedian[i] - median) > expectedSigma[i])
            {
                output_screen("FAIL: Expected " << paramName << " median is " << expectedMedian[i] << ", the result is " << median << std::endl);
                res = 0;
            }

            if(!Math::areEqual(expectedSigma[i], sigma, 0.25))
            {
                output_screen("FAIL: Expected " << paramName << " sigma is " << expectedSigma[i] << ", the result is " << sigma << std::endl);
                res = 0;
            }
        }
    }
    outParamLimits.close();
}

