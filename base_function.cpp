#include "base_function.h"

double max(double a, double b)
{
	if (a>b)
		return a;
	else
		return b;
}

double min(double a, double b)
{
	if (a<b)
		return a;
	else
		return b;
}

float Norm_cdf(const double& z) { 
	if (z > 6.0) { return 1.0; }; // this guards against overflow 
	if (z < -6.0) { return 0.0; }; 
	double b1 = 0.31938153; 
	double b2 = -0.356563782; 
	double b3 = 1.781477937; 
	double b4 = -1.821255978; 
	double b5 = 1.330274429; 
	double p = 0.2316419; 
	double c2 = 0.3989423; 
	double a=fabs(z); 
	double t = 1.0/(1.0+a*p); 
	double b = c2*exp((-z)*(z/2.0)); 
	double n = ((((b5*t+b4)*t+b3)*t+b2)*t+b1)*t; 
	n = 1.0-b*n; 
	if ( z < 0.0 ) n = 1.0 - n; 
	return n; 
}; 

double Norm_pdf(double &z)
{
	double c2 = 0.3989422804*exp((-z)*(z / 2.0));
	return c2;
}
//Black-Scholes Formula for Vanila European Option
double bls_vanila_option(pricing_param &param)
{
	if (param.TimeToMaturity > 0.0005)
	{
		double time_sqrt = sqrt(param.TimeToMaturity);
		double d1 = (log(param.Spot_Price / param.Strike_Price) + (param.Free_Rate - param.Yield_Rate)*param.TimeToMaturity) / (param.Volatility*time_sqrt) + 0.5*param.Volatility*time_sqrt;
		double d2 = d1 - (param.Volatility*time_sqrt);

		if (param.Option_Type == 1)
		{
			return param.Spot_Price*Norm_cdf(d1)*exp(-param.Yield_Rate*param.TimeToMaturity) - param.Strike_Price*exp(-param.Free_Rate*param.TimeToMaturity)*Norm_cdf(d2);
		}
		else if (param.Option_Type == 2)
		{
			return -param.Spot_Price*Norm_cdf(-d1)*exp(-param.Yield_Rate*param.TimeToMaturity) + param.Strike_Price*exp(-param.Free_Rate*param.TimeToMaturity)*Norm_cdf(-d2);
		}
	}
	else
	{
		if (param.Option_Type == 1)
		{
			return max(param.Spot_Price - param.Strike_Price,0);
		}
		else if (param.Option_Type == 2)
		{
			return max(0,-param.Spot_Price + param.Strike_Price);
		}
	}

}

//Black-Scholes Formula for Vanila European Option Delta
double bls_vanila_delta(pricing_param &param)
{
	double result = 0;
	if (param.TimeToMaturity > 0.0005)
	{
		double time_sqrt = sqrt(param.TimeToMaturity);
		double d1 = (log(param.Spot_Price / param.Strike_Price) + (param.Free_Rate - param.Yield_Rate)*param.TimeToMaturity) / (param.Volatility*time_sqrt) + 0.5*param.Volatility*time_sqrt;

		if (param.Option_Type == 1)
		{
			result = Norm_cdf(d1)*exp(-param.Yield_Rate*param.TimeToMaturity);
		}
		else if (param.Option_Type == 2)
		{
			result = -Norm_cdf(-d1)*exp(-param.Yield_Rate*param.TimeToMaturity);
		}
	}
	else
	{
		if (param.Option_Type == 1)
		{
			result = param.Spot_Price > param.Strike_Price? 1:0;
		}
		else if (param.Option_Type == 2)
		{
			result = param.Spot_Price > param.Strike_Price ? 0 : -1;
		}
	}
	return result;
}

double bls_vanila_gamma(pricing_param &param)
{
	double result = 0;
	if (param.TimeToMaturity > 0.0005)
	{
		double time_sqrt = sqrt(param.TimeToMaturity);
		double d1 = (log(param.Spot_Price / param.Strike_Price) + (param.Free_Rate - param.Yield_Rate)*param.TimeToMaturity) / (param.Volatility*time_sqrt) + 0.5*param.Volatility*time_sqrt;


		result = Norm_pdf(d1) / (param.Spot_Price*param.Volatility*time_sqrt);
	}
	else
	{
		result = 0;
	}
	return result;
}

double bls_vanila_theta(pricing_param &param)
{
	double result = 0;
	if (param.TimeToMaturity > 0.0005)
	{
		double time_sqrt = sqrt(param.TimeToMaturity);
		double d1 = (log(param.Spot_Price / param.Strike_Price) + (param.Free_Rate - param.Yield_Rate)*param.TimeToMaturity) / (param.Volatility*time_sqrt) + 0.5*param.Volatility*time_sqrt;
		double d2 = d1 - (param.Volatility*time_sqrt);


		if (param.Option_Type == 1)
		{
			result = -param.Spot_Price*Norm_cdf(d1)* param.Volatility / (2 * time_sqrt) - param.Free_Rate*param.Strike_Price*exp(-param.Free_Rate*param.TimeToMaturity)*Norm_cdf(d2);
		}
		else if (param.Option_Type == 2)
		{
			result = -param.Spot_Price*Norm_cdf(d1)* param.Volatility / (2 * time_sqrt) + param.Free_Rate*param.Strike_Price*exp(-param.Free_Rate*param.TimeToMaturity)*Norm_cdf(-d2);
		}
	}
	else
	{
		result = 0;
	}
	return result/256;
}

double bls_vanila_vega(pricing_param &param)
{
	double result = 0;
	if (param.TimeToMaturity > 0.0005)
	{
		double time_sqrt = sqrt(param.TimeToMaturity);
		double d1 = (log(param.Spot_Price / param.Strike_Price) + (param.Free_Rate - param.Yield_Rate)*param.TimeToMaturity) / (param.Volatility*time_sqrt) + 0.5*param.Volatility*time_sqrt;
		result = Norm_pdf(d1) *(param.Spot_Price*time_sqrt)/100;
	}
	else
	{
		result = 0;
	}
	return result;
}