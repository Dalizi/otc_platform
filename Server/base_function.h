#pragma once

#include <cmath>
#include <map>

using namespace std;

double max(double a, double b);

double min(double a, double b);

float Norm_cdf(const double& z);

double Norm_pdf(double &z);

struct pricing_param
{
	double Spot_Price,Strike_Price;
	float Free_Rate,Yield_Rate,TimeToMaturity,Volatility;
	int Option_Type,Value_Method;
	map<string,string> other_param;
};

double bls_vanila_option(pricing_param &param);

double bls_vanila_delta(pricing_param &param);

double bls_vanila_gamma(pricing_param &param);

double bls_vanila_theta(pricing_param &param);

double bls_vanila_vega(pricing_param &param);