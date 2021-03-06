#include <fstream>
#include <vector>
#include <string>
#include <sstream>

#include <test_polychord_planck.hpp>
#include <polychord.hpp>
#include <planck_like.hpp>
#include <markov_chain.hpp>
#include <numerics.hpp>

std::string
TestPolyChordPlanck::name() const
{
    return std::string("POLYCHORD PLANCK LIKELIHOOD TESTER");
}

unsigned int
TestPolyChordPlanck::numberOfSubtests() const
{
    return 1;
}

void
TestPolyChordPlanck::runSubTest(unsigned int i, double& res, double& expected, std::string& subTestName)
{
    check(i >= 0 && i < 1, "invalid index " << i);
    
    using namespace Math;

    std::string root = "slow_test_files/polychord_planck_test";
#ifdef COSMO_PLANCK_15
    //const int nPar = 7;
    const int nPar = 22;
    //PlanckLikelihood planckLike(true, true, true, false, true, false, false, false, 5);
    PlanckLikelihood planckLike(true, true, true, false, false, false, false, false, 5);
#else
    const int nPar = 20;
    PlanckLikelihood planckLike(true, true, false, true, false, false, 5);
#endif
    PolyChord pc(nPar, planckLike, 300, root, 6);

    pc.setParam(0, "ombh2", 0.02, 0.025, 1);
    pc.setParam(1, "omch2", 0.1, 0.2, 1);
    pc.setParam(2, "h", 0.55, 0.85, 1);
    pc.setParam(3, "tau", 0.02, 0.20, 1);
    pc.setParam(4, "ns", 0.9, 1.1, 2);
    pc.setParam(5, "As", 2.7, 3.5, 2);

#ifdef COSMO_PLANCK_15
    pc.setParamGauss(6, "A_planck", 1, 0.0025, 3);

    pc.setParam(7, "A_cib_217", 0, 200, 3);
    pc.setParamFixed(8, "cib_index", -1.3);
    pc.setParam(9, "xi_sz_cib", 0, 1, 3);
    pc.setParam(10, "A_sz", 0, 10, 3);
    pc.setParam(11, "ps_A_100_100", 0, 400, 3);
    pc.setParam(12, "ps_A_143_143", 0, 400, 3);
    pc.setParam(13, "ps_A_143_217", 0, 400, 3);
    pc.setParam(14, "ps_A_217_217", 0, 400, 3);
    pc.setParam(15, "k_sz", 0, 10, 3);
    pc.setParamGauss(16, "gal545_A_100", 7, 2, 3);
    pc.setParamGauss(17, "gal545_A_143", 9, 2, 3);
    pc.setParamGauss(18, "gal545_A_143_217", 21, 8.5, 3);
    pc.setParamGauss(19, "gal545_A_217", 80, 20, 3);
    pc.setParamGauss(20, "calib_100T", 0.999, 0.001, 3);
    pc.setParamGauss(21, "calib_217T", 0.995, 0.002, 3);
#else
    pc.setParam(6, "A_ps_100", 0, 360, 3);
    pc.setParam(7, "A_ps_143", 0, 270, 3);
    pc.setParam(8, "A_ps_217", 0, 450, 3);
    pc.setParam(9, "A_cib_143", 0, 20, 3);
    pc.setParam(10, "A_cib_217", 0, 80, 3);
    pc.setParam(11, "A_sz", 0, 10, 3);
    pc.setParam(12, "r_ps", 0.0, 1.0, 3);
    pc.setParam(13, "r_cib", 0.0, 1.0, 3);
    pc.setParam(14, "n_Dl_cib", -2, 2, 3);
    pc.setParam(15, "cal_100", 0.98, 1.02, 3);
    pc.setParam(16, "cal_127", 0.95, 1.05, 3);
    pc.setParam(17, "xi_sz_cib", 0, 1, 3);
    pc.setParam(18, "A_ksz", 0, 10, 3);
    pc.setParam(19, "Bm_1_1", -20, 20, 3);
#endif

    const std::vector<double> fracs{0.7, 0.15, 0.15};
    pc.setParameterHierarchy(fracs);

    const double pivot = 0.05;
    LambdaCDMParams par(0.022, 0.12, 0.7, 0.1, 1.0, std::exp(3.0) / 1e10, pivot);
    planckLike.setModelCosmoParams(&par);

    pc.run(true);
    
    subTestName = std::string("standard_param_limits");
    res = 1;
    expected = 1;

    if(!isMaster())
        return;

    MarkovChain chain("slow_test_files/polychord_planck_test.txt");

    std::ofstream outParamLimits("slow_test_files/polychord_planck_param_limits.txt");
#ifdef COSMO_PLANCK_15
    const double expectedMedian[6] = {0.02222, 0.1197, 0.6731, 0.078, 0.9655, 3.089};
    const double expectedSigma[6] = {0.00023, 0.0022, 0.0096, 0.019, 0.0062, 0.036};
#else
    const double expectedMedian[6] = {0.02205, 0.1199, 0.673, 0.089, 0.9603, 3.089};
    const double expectedSigma[6] = {0.00028, 0.0027, 0.012, 0.013, 0.0073, 0.025};
#endif
    for(int i = 0; i < 6; ++i)
    {
        const std::string& paramName = pc.getParamName(i);
        std::stringstream fileName;
        fileName << "slow_test_files/polychord_planck_" << paramName << ".txt";
        Posterior1D* p = chain.posterior(i);
        p->writeIntoFile(fileName.str().c_str());

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
                output_screen("FAIL: Expected" << paramName << "sigma is " << expectedSigma[i] << ", the result is " << sigma << std::endl);
                res = 0;
            }
        }
    }
    outParamLimits.close();
}

