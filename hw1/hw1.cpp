#include <iostream>
#include <iomanip>
#include <cmath>
#include <fstream>
#include <cstdlib>
#include <algorithm>
#include <time.h>
#include <stdlib.h>
#include <math.h>

using namespace std;

double s0 = 1868.99;
double maturity_time = 1.0 / 52.0;
double k = 1870.0;
double r = 0.003866;
double q = 0.0232;
double sigma=0.2979;
int n;

double N(const double& z) {
	if (z > 6.0) { return 1.0; }; // this guards against overflow 
	if (z < -6.0) { return 0.0; };
	double b1 = 0.31938153;
	double b2 = -0.356563782;
	double b3 = 1.781477937;
	double b4 = -1.821255978;
	double b5 = 1.330274429;
	double p = 0.2316419;
	double c2 = 0.3989423;
	double a = fabs(z);
	double t = 1.0 / (1.0 + a * p);
	double b = c2 * exp((-z) * (z / 2.0));
	double n = ((((b5 * t + b4) * t + b3) * t + b2) * t + b1) * t;
	n = 1.0 - b * n;
	if (z < 0.0) n = 1.0 - n;
	return n;
};

double option_price_call_black_scholes()
{
	double time_sqrt = sqrt(maturity_time);
	double d1 = (log(s0 / k) + (r-q) * maturity_time) / (sigma * time_sqrt) + 0.5 * sigma * time_sqrt;
	double d2 = d1 - (sigma * time_sqrt);
	return s0*exp(-q*maturity_time) * N(d1) - k * exp(-r * maturity_time) * N(d2);
};

double gaussrand()
{
	static double V1, V2, S;
	static int phase = 0;
	double X;

	if (phase == 0) {
		do {
			double U1 = (double)rand() / RAND_MAX;
			double U2 = (double)rand() / RAND_MAX;

			V1 = 2 * U1 - 1;
			V2 = 2 * U2 - 1;
			S = V1 * V1 + V2 * V2;
		} while (S >= 1 || S == 0);

		X = V1 * sqrt(-2 * log(S) / S);
	}
	else
		X = V2 * sqrt(-2 * log(S) / S);

	phase = 1 - phase;

	return X;
}

double max(double a, double b) {
	return (b < a) ? a : b;
}

double european_call_option(int n, double &st, double &y)
{
	st = 0.0;
	y = 0.0;
	for (int i = 1; i < n+1; i++)
	{
		double c = exp(-r * maturity_time) * max(0, s0 * exp((r - q - 0.5 * sigma * sigma) * maturity_time + sigma * sqrt(maturity_time) * gaussrand()) - k);
		st = st * (1.0 - 1.0 / i) + c / i;
		y = y * (1.0 - 1.0 / i) + c * c / i;
	}
	return 0;
}

double european_antithetic(int n, double &st, double &y)
{
	st = 0.0;
	y = 0.0;
	for (int i = 1; i < n + 1; i++)
	{
		double s_plus = s0 * exp((r - q - 0.5 * sigma * sigma) * maturity_time + sigma * sqrt(maturity_time) * gaussrand());
		double s_minus = s0 * exp((r - q - 0.5 * sigma * sigma) * maturity_time - sigma * sqrt(maturity_time) * gaussrand());
		double c = 0.5 * exp(-r * maturity_time) * (max(0, s_plus - k) + max(0, s_minus - k));
		st = st * (1.0 - 1.0 / i) + c / i;
		y = y * (1.0 - 1.0 / i) + c * c / i;
	}
	return 0;
}

int main (int argc, char* argv[])
{
	//part 1 BSM call price
	cout << "BSM call price: " << option_price_call_black_scholes() << endl;
	//Part 2 Write a C++ program to compute the call price using Monte Carlo simulation.
	clock_t start, end;
	int sizes[6];
	for (int i = 0; i < 6; i++)
	{
		sizes[i] = 1000 * pow(10, i);
	}
	cout << left;
	cout << setw(12) << "Sample size" << setw(12) << "Price" << setw(16) << "Standard error"
		<< setw(18) << "95% CI" << setw(12) << "Time used" << endl;
	cout << "----------------------------------------------------------------------" << endl;

	for (int i = 1; i < 6; i++)
	{
		

		int number = sizes[i];

		start = clock();
		double st;
		double y;
		european_call_option(number,st,y);
		double call_price = st;
		double se = sqrt((y - st * st) / (number - 1));
		end = clock();

		double left = call_price - 1.96 * se;
		double right = call_price + 1.96 * se;

		cout << setw(12) << number << setw(12) << call_price << setw(16) << se
			<< "[" << setw(7) << left << "," << setw(7) << right << setw(2) << "]" << setw(12) << double(end - start) / CLOCKS_PER_SEC << endl;
	}
	//part3 Write a C++ program to compute the call price using Monte Carlo simulation with antithetic variates.	
	clock_t start1, end1;
	clock_t start2, end2;
	int sizes1[5];
	for (int i = 0; i < 5; i++)
	{
		sizes1[i] = 4000 * pow(10, i);
	}
	cout << left;
	cout << setw(11) << "Approach" << setw(12) << "Sample size" << setw(8) << "Price" << setw(11) << "SE"
		<< setw(18) << "95% CI" << setw(7) << "Time" << setw(12) << "Efficiency" << endl;
	cout << "----------------------------------------------------------------------" << endl;

	for (int i = 1; i < 5; i++)
	{
		int number1 = sizes1[i];

		//Antithetic approach
		start1 = clock();
		double st1 = 0.0;
		double y1 = 0.0;
		european_antithetic(number1,st1,y1);
		double call_price1 = st1;
		double se1 = sqrt((y1 - st1 * st1) / (number1 - 1));
		end1 = clock();

		double left1 = call_price1 - 1.96 * se1;
		double right1 = call_price1 + 1.96 * se1;
		double time1 = double(end1 - start1) / CLOCKS_PER_SEC;
		double efficiency1 = se1 * se1 * time1;

		cout << setw(11) << "Antithetic" << setw(12) << number1 << setw(8) << call_price1 << setw(11) << se1
			<< "[" << setw(7) << left1 << "," << setw(7) << right1 << setw(2) << "]" << setw(7) << time1
			<< setw(12) << efficiency1 << endl;

		//standard approach
		start2 = clock();
		double st2;
		double y2;
		european_call_option(number1, st2, y2);
		double call_price2 = st2;
		double se2 = sqrt((y2 - st2 * st2) / (number1 - 1));
		end2 = clock();

		double left2 = call_price2 - 1.96 * se2;
		double right2 = call_price2 + 1.96 * se2;
		double time2 = double(end2 - start2) / CLOCKS_PER_SEC;
		double efficiency2 = se2 * se2 * time2;

		cout << setw(11) << "Standard" << setw(12) << number1 << setw(8) << call_price2 << setw(11) << se2
			<< "[" << setw(7) << left2 << "," << setw(7) << right2 << setw(2) << "]" << setw(7) << time2
			<< setw(12) << efficiency2 << endl;
	}
}
