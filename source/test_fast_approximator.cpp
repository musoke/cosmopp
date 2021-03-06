#include <ctime>

#include <random.hpp>
#include <fast_approximator.hpp>
#include <test_fast_approximator.hpp>

std::string
TestFastApproximator::name() const
{
    return std::string("FAST APPROXIMATOR TESTER");
}

unsigned int
TestFastApproximator::numberOfSubtests() const
{
    return 1;
}

double fastApproxTestFunc(double x)
{
    return 5 * x * x - 3 * x + 10;
}

void
TestFastApproximator::runSubTest(unsigned int i, double& res, double& expected, std::string& subTestName)
{
    check(i == 0, "invalid index " << i);

    std::vector<std::vector<double> > points, data;

    Math::UniformRealGenerator gen(std::time(0), -10, 10);

    const int nPoints = 10000;

    std::vector<double> p(1), d(1);

    for(int i = 0; i < nPoints; ++i)
    {
        p[0] = gen.generate();
        points.push_back(p);
    }

    for(int i = 0; i < points.size(); ++i)
    {
        d[0] = fastApproxTestFunc(points[i][0]);
        data.push_back(d);
    }

    FastApproximator fa(1, 1, points.size(), points, data, (i == 0 ? 10 : 10));

    p[0] = 0;

    fa.approximate(p, d, FastApproximator::QUADRATIC_INTERPOLATION);

    subTestName = "parabola";
    res = d[0];
    expected = fastApproxTestFunc(p[0]);
}
