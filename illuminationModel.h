#ifndef _ILLUMINATIONMODEL_H
#define _ILLUMINATIONMODEL_H

#include <vector>
#include <algorithm>    // std::max
#include "mathHelper.h"
#include "object.h"
#include "lightSource.h"

Color ambientComponent(Object *obj, Color ambientLight) {
    Color objColor = obj->getColor();
    float ka = obj->getKa();

    return ka * ambientLight * objColor;
}

// needs the object because we will use diffuse and specular color,
// here we will not calculate the amcient component
// the light list is the lights that the shadow array definetly hit
Color illuminate(Object *obj, Vector view, Point point, Vector normal, std::vector<LightSource> lightList) {
    Color diffuse(0,0,0);
    Color specular(0,0,0);

    std::vector<LightSource>::iterator it;

    Color objColor = obj->getColor();
    Color objSpecColor = obj->getSpecularColor();
    
    float kd = obj->getKd();
    float ks = obj->getKs();
    float ke = obj->getKe();

    normalize(view);
    normalize(normal);

    // let's calculate diffuse and specular values for each light
    for(it = lightList.begin() ; it < lightList.end() ; ++it) {
        Color lightRadiance = (*it).getColor();

        Vector s(point, (*it).getPos(), true);

        float sn = std::max(dot( s, normal ),0.0f);

        Vector invs((*it).getPos(), point, true);

        Vector r = reflect( invs, normal );
        normalize(r);

        float rvke = std::pow( std::max(dot( r, view ), 0.0f), ke );

        // calculate it
        diffuse += lightRadiance * objColor * sn;
        specular += lightRadiance * objSpecColor * rvke;
    }

    return kd * diffuse + ks * specular;
}

#endif