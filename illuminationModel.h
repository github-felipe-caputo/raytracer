#ifndef _ILLUMINATIONMODEL_H
#define _ILLUMINATIONMODEL_H

#include <vector>
#include "mathHelper.h"
#include "object.h"
#include "lightSource.h"

class IlluminationModel {
public:
    virtual Color illuminate(Object *obj, Vector view, Point point, Vector normal, std::vector<LightSource> lightList) = 0;
};

class Phong : public IlluminationModel {
public:

    Color ambientComponent(Object *obj, Color ambientLight) {
        Color objColor = obj->getColor();
        float ka = obj->getKa();

        return Color(ka * objColor.r * ambientLight.r,
                     ka * objColor.g * ambientLight.g, 
                     ka * objColor.b * ambientLight.b);
    }

    // needs the object because we will use diffuse and specular color,
    // here we will not calculate the amcient component
    // the light list is the lights that the shadow array definetly hit
    Color illuminate(Object *obj, Vector view, Point point, Vector normal, std::vector<LightSource> lightList) {
        Color diffuse(0,0,0);
        Color specular(0,0,0);

        Color objColor = obj->getColor();
        Color objSpecColor = obj->getSpecularColor();
        
        float kd = obj->getKd();
        float ks = obj->getKs();
        float ke = obj->getKe();

        // let's calculate diffuse and specular values for each light
        for (unsigned int i = 0; i < lightList.size(); ++i) {
            Color lightRadiance = lightList[i].getColor();

            Vector s(point, lightList[i].getPos());
            normalize(s);

            float sn = dot( s, normal );

            Vector r = reflect( Vector(point, lightList[i].getPos()), view );

            float rvke = pow( dot( r, view ), ke);

            // calculate it
            diffuse = Color( diffuse.r + lightRadiance.r * objColor.r * sn, 
                             diffuse.g + lightRadiance.g * objColor.g * sn, 
                             diffuse.b + lightRadiance.b * objColor.b * sn );
            specular = Color( specular.r + lightRadiance.r * objSpecColor.r * rvke, 
                              specular.g + lightRadiance.g * objSpecColor.g * rvke, 
                              specular.b + lightRadiance.b * objSpecColor.b * rvke );
        }

        return Color(kd * diffuse.r + ks * specular.r, 
                     kd * diffuse.g + ks * specular.g, 
                     kd * diffuse.b + ks * specular.b);
    }
};



#endif