#ifndef _ILLUMINATIONMODEL_H
#define _ILLUMINATIONMODEL_H

#include <vector>
#include <algorithm>    // std::max
#include "mathHelper.h"
#include "object.h"
#include "lightSource.h"

Color ambientComponent(Object *obj, Color ambientLight, Point point) {
    Color objColor = obj->getColor(point);
    double ka = obj->getKa();

    return ka * ambientLight * objColor;
}

// needs the object because we will use diffuse and specular color,
// here we will not calculate the ambient component
// the light list is the lights that the shadow array definetly hit
Color illuminatePhong(Object *obj, Vector view, Point point, Vector normal,
    std::map<LightSource*, std::vector<Point> > lightsAndPointsReachedMap) {
    if (lightsAndPointsReachedMap.empty())
        return Color(0,0,0);

    Color diffuse, diffuseFinal;
    Color specular, specularFinal;

    Color objColor = obj->getColor(point);
    Color objSpecColor = obj->getSpecularColor();

    double kd = obj->getKd();
    double ks = obj->getKs();
    double ke = obj->getKe();

    normalize(view);
    normalize(normal);

    // For each light, for each point reached on the light
    for (std::map<LightSource*, std::vector<Point> >::iterator it=lightsAndPointsReachedMap.begin(); it!=lightsAndPointsReachedMap.end(); ++it) {
        LightSource *lightHit = (it->first);
        std::vector<Point> pointsHit = (it->second);

        double attenuation = lightHit->getAttenuation(point);
        Color lightRadiance = lightHit->getColor();
        double numSamples = lightHit->getNumSamplesOnSurface();

        for(std::vector<Point>::iterator it2 = pointsHit.begin() ; it2 < pointsHit.end() ; ++it2) {
            // diffuse
            Vector s(point, (*it2), true);
            double sn = std::max(dot( s, normal ),0.0);

            // spec
            Vector invs((*it2), point, true);
            Vector r = reflect ( invs, normal, VECTOR_INCOMING );
            normalize(r);

            double rvke = std::pow( std::max(dot( r, view ), 0.0), ke );

            // calculate it
            diffuse += lightRadiance * objColor * sn * attenuation;
            specular += lightRadiance * objSpecColor * rvke * attenuation;
        }

        diffuseFinal += diffuse / numSamples;
        specularFinal += specular / numSamples;
    }

    return kd * diffuseFinal + ks * specularFinal;
}
/*
Color illuminatePhongBlinn(Object *obj, Vector view, Point point, Vector normal, std::vector<LightSource*> lightList) {
    if (lightList.empty())
        return Color(0,0,0);

    Color diffuse;
    Color specular;

    Color objColor = obj->getColor(point);
    Color objSpecColor = obj->getSpecularColor();

    double kd = obj->getKd();
    double ks = obj->getKs();
    double ke = obj->getKe();

    normalize(view);
    normalize(normal);

    // let's calculate diffuse and specular values for each light
    for(std::vector<LightSource*>::iterator it = lightList.begin() ; it < lightList.end() ; ++it) {
        double attenuation = (*it)->getAttenuation(point);
        Color lightRadiance = (*it)->getColor();

        // diffuse
        Vector s(point, (*it)->getPos(), true);
        double sn = std::max(dot( s, normal ),0.0);

        // spec
        Vector h = s + view;
        normalize(h);

        double rvke = std::pow( std::max(dot( normal, h ), 0.0), ke );

        // calculate it
        diffuse += lightRadiance * objColor * sn * attenuation;
        specular += lightRadiance * objSpecColor * rvke * attenuation;
    }

    return kd * diffuse + ks * specular;
}
*/
#endif
