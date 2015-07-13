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

    PlanckLikelihood planckLike(true, true, false, true, false, false);
    std::string root = "slow_test_files/polychord_planck_test";
    PolyChord pc(20, planckLike, 300, root);

    pc.setParam(0, "ombh2", 0.02, 0.025);
    pc.setParam(1, "omch2", 0.1, 0.2);
    pc.setParam(2, "h", 0.55, 0.85);
    pc.setParam(3, "tau", 0.02, 0.20);
    pc.setParam(4, "ns", 0.9, 1.1);
    pc.setParam(5, "As", 2.7, 3.5);

    pc.setParam(6, "A_ps_100", 0, 360);
    pc.setParam(7, "A_ps_143", 0, 270);
    pc.setParam(8, "A_ps_217", 0, 450);
    pc.setParam(9, "A_cib_143", 0, 20);
    pc.setParam(10, "A_cib_217", 0, 80);
    pc.setParam(11, "A_sz", 0, 10);
    pc.setParam(12, "r_ps", 0.0, 1.0);
    pc.setParam(13, "r_cib", 0.0, 1.0);
    pc.setParam(14, "n_Dl_cib", -2, 2);
    pc.setParam(15, "cal_100", 0.98, 1.02);
    pc.setParam(16, "cal_127", 0.95, 1.05);
    pc.setParam(17, "xi_sz_cib", 0, 1);
    pc.setParam(18, "A_ksz", 0, 10);
    pc.setParam(19, "Bm_1_1", -20, 20);

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

    //const double expectedMedian[6] = {0.02217, 0.1186, 0.679, 0.089, 0.9635, 3.085};
    //const double expectedSigma[6] = {0.00033, 0.0031, 0.015, 0.032, 0.0094, 0.057};
    const double expectedMedian[6] = {0.02205, 0.1199, 0.673, 0.089, 0.9603, 3.089};
    const double expectedSigma[6] = {0.00028, 0.0027, 0.012, 0.013, 0.0073, 0.025};

    std::ofstream outParamLimits("slow_test_files/polychord_planck_param_limits.txt");
    for(int i = 0; i < 20; ++i)
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
