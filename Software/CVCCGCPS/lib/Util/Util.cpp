#include <Wire.h>

void set_evr(char vc_sel, char val){
	// if(vc_sel){
	// 	Wire.beginTransmission(0x2F);
	// 	Wire.write(val);
	// 	Wire.endTransmission();
	// }
	// else{
	// 	Wire1.beginTransmission(0x2F);
	// 	Wire1.write(val);
	// 	Wire1.endTransmission();
	// }
		Wire.beginTransmission(0x2F);
		Wire.write(val);
		Wire.endTransmission();
}

float evr_to_voltage(char evr_v){
	return 6.2 * 3.4 * evr_v /127;
}

float evr_to_current(char evr_c){
	return evr_c * 1.0;
}

float adc_to_voltage(int out_v){
	return float(22.0 * out_v / 4096.0);
}

float adc_to_current(int out_c){
	return out_c * 1.0;
}