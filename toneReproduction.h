#ifndef _TONEREPRODUCTION_H
#define _TONEREPRODUCTION_H

#include <cmath>

#include "mathHelper.h"

#define LDMAX 500 // maximum display luminance, 500 for standard CRTs

std::vector<double> getLuminance( std::vector<Color> colorMap ) {
    std::vector<double> luminance;

    for(unsigned int i = 0; i < colorMap.size(); ++i) {
        Color c = colorMap[i];
        luminance.push_back(0.27 * c.r + 0.67 * c.g + 0.06 * c.b);
    }

    return luminance;
}

double logAverage( std::vector<double> luminance ) { 
    double sum = 0;
    double delta = 0.000000001; // don't want log of 0

    for(unsigned int i = 0; i < luminance.size(); ++i)
        sum += std::log( delta + luminance[i] );

    return std::exp( sum / luminance.size() );
}

std::vector<Color> compressionPerceptual( std::vector<Color> colorMap, double Lmax ) {
    // maximum luminance in the scene
    for(unsigned int i = 0; i < colorMap.size(); ++i)
        colorMap[i] = Lmax * colorMap[i];

    double Lwa = std::pow( logAverage ( getLuminance(colorMap) ) , 0.4 ) ; 
    double sf = std::pow( ( ( 1.219 + std::pow( LDMAX / 2.0 , 0.4) ) / (1.219 + Lwa) ) , 2.5);

    std::vector<Color> colorFinal;

    for(unsigned int i = 0; i < colorMap.size(); ++i) {
        Color colorTarget = sf * colorMap[i];
        colorFinal.push_back( colorTarget / LDMAX );
    }
    
    return colorFinal;
}

std::vector<Color> compressionPhotographic( std::vector<Color> colorMap, double Lmax ) {
    // maximum luminance in the scene
    for(unsigned int i = 0; i < colorMap.size(); ++i)
        colorMap[i] = Lmax * colorMap[i];

    double a = 0.18;
    double Lavg = logAverage ( getLuminance(colorMap) );

    std::vector<Color> colorFinal;

    for(unsigned int i = 0; i < colorMap.size(); ++i) {
        Color scaledColor = a * colorMap[i] / Lavg;
        Color colorTarget( (scaledColor.r / (1.0+scaledColor.r)) * LDMAX , 
                           (scaledColor.g / (1.0+scaledColor.g)) * LDMAX , 
                           (scaledColor.b / (1.0+scaledColor.b)) * LDMAX );
        colorFinal.push_back( colorTarget / LDMAX );
    }

    return colorFinal;
}


#endif