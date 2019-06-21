#include <iostream>
#include <boost/numeric/ublas/vector.hpp>

namespace ublas = boost::numeric::ublas;
using namespace std;

int main(){
	ublas::vector<double> v(2);
	v[0] = -1;
	v[1] = 1;

	ublas::vector<double> u(2);
	u[0] = 1;
	u[1] = -1;

	const double result = ublas::inner_prod(v, u);
	cout << result << endl;
}
