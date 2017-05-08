#include <math.h>
#include <stdio.h>

#include <gtk/gtk.h>

//#define DEBUG

double fog_attenuation(double frequency, double T0, double visibility, double distance){

	double M;							// water content	/[g/m³]
	double theta;
	double f_s;
	double f_p;
	double epsilon_0, epsilon_1,epsilon_2;
	double epsilon_1_strich, epsilon_2_strich;
	double eta;
	double k_l;
	double attenuation_fog;		//Total fog attenuation
	double f_f_p_square;
	double f_f_s_square;
	
	
	if (visibility < 0.01)	{
		visibility = 0.01;
	}
		
	M = pow( (0.024/visibility), 1.54);

#ifdef DEBUG
	g_print("\n\rFog water content %3.3f g/m³\n\r",M);
#endif

	theta = 300 / (273.15+T0);
	
	f_p = 20.09 - 142 * (theta -1) + 294 * (theta - 1) * (theta - 1);

	f_s = 590 - 1500 * (theta - 1);

	epsilon_2 = 3.51;
	
	epsilon_1 = 5.48;
	
	epsilon_0 = 77.6 + 103.3 * (theta - 1);
	
	f_f_p_square = pow((frequency/f_p),2);
	
	f_f_s_square = pow((frequency/f_s),2);
	
	epsilon_2_strich = ( frequency * ( epsilon_0-epsilon_1) / f_p / (1 + f_f_p_square ) ) +	( frequency * ( epsilon_1-epsilon_2) / f_s / (1 + f_f_s_square) );
	
	epsilon_1_strich = ( ( epsilon_0-epsilon_1) / (1 + f_f_p_square ) ) +	( ( epsilon_1-epsilon_2) / (1 + f_f_s_square ) ) + epsilon_2;
		
	eta = (2 + epsilon_1_strich) / epsilon_2_strich;
	
	k_l = 0.819 * frequency / epsilon_2_strich / (1 + eta * eta);

#ifdef DEBUG
	g_print("Specific fog attenuation %3.3f g/m³\n\r",k_l);
#endif

	
	attenuation_fog  = k_l * M * distance;
	
	return attenuation_fog;
}

double rain_attenuation(double frequency, double rainfall, double distance){

	double k_h_a[4] = {-5.33980,-0.35351,-0.23789,-0.94158};
	double k_h_b[4] = {-0.10008,1.26970,0.86036,0.64552};
	double k_h_c[4] = {1.13098,0.45400,0.15354,0.16817};
	
	double k_h_m_k = -0.18961;
	double k_h_c_k = 0.71147;

	double a_h_a[5] ={-0.14318,0.29591,0.32177,-5.37610,16.1721};
	double a_h_b[5] ={1.82442,0.77564,0.63773,-0.96230,-3.29980};
	double a_h_c[5] ={-0.55187,0.19822,0.13164,1.47828,3.43990};
	
	double a_h_m_a = 0.67849;
	double a_h_c_a = -1.95537;

	double k = 0;
	double log_k = 0;
	double alpha = 0;
	double con1;
	double con2;
	double con3;
	
	double attenuation_rain;
	
	int j;
	
	for (j = 0; j < 4;j ++){
		con1 = (log10(frequency)-k_h_b[j]) / k_h_c[j];
		con2 = -1.0 * pow(con1 ,2);
		con3 = k_h_a[j] * exp (con2);
		log_k = log_k + con3;
	}
	
	log_k = log_k + k_h_m_k * log10(frequency) + k_h_c_k;
	
	k = pow(10,log_k);

	for (j = 0; j < 5;j ++){
		con1 = (log10(frequency)-a_h_b[j]) / a_h_c[j];
		con2 = -1.0 * pow(con1 ,2);
		con3 = a_h_a[j] * exp (con2);
		alpha = alpha + con3;
	}	

	alpha = alpha + a_h_m_a* log10(frequency) + a_h_c_a;

	attenuation_rain = k * pow(rainfall,alpha) * distance;

	return (attenuation_rain);
}

double oxygen_attenuation(double frequency, double distance){

	double con1;
	double con2;
	double con3;
	double gamma_0;
	double attenuation_oxygen;

	if( frequency < 53) {
		con1 = 6.09 / ( frequency * frequency  +0.277);
		con2 = 4.81 / ( pow((frequency - 57),2) + 1.5 );
		con3 = 0.00719 + con1 + con2;
		gamma_0 = con3 * frequency * frequency * 0.001;
	}
	else
	if( frequency > 63) {
		con1 = 0.625 / ( pow((frequency - 63),2) + 1.59 );
		con2 = 0.028 / ( pow((frequency - 118),2) + 1.47 );
		con3 = 0.000000379 * frequency + con1 + con2;
		gamma_0 = con3 * pow((frequency+198),2) * 0.001;
	}
	else {
		gamma_0 = 14.9;
	}

	attenuation_oxygen = gamma_0 * distance;

	return attenuation_oxygen;
}

double vapour_attenuation(double frequency, double temperature, double rel_humidity, double distance){
	double con1;
	double con2;
	double con3;
	double gamma_0;
	double attenuation_vapour;
	double W;							//weight of water in saturted air[g/m³]
	
	W = 4.85 + 0.33 * temperature + 0.01 * pow(temperature,2) + 0.00023 * pow(temperature,3);
	W = W * rel_humidity/100.0;

	con1 = 3.6 / ( pow((frequency - 22.2),2) + 8.5 );
	con2 = 10.6 / ( pow((frequency - 183.3),2) + 9.0 );
	con3 = 8.9 / ( pow((frequency - 325.4),2) + 26.3 );
		
	gamma_0 = (0.05 + 0.0021 * W + con1 + con2 + con3) * W * frequency * frequency * 0.0001;	
	
	attenuation_vapour = gamma_0 * distance;
	
#ifdef DEBUG
	g_print("Vapour attenuation %3.3f dB\n\r",attenuation_vapour);
#endif	
		
	return attenuation_vapour;
}
