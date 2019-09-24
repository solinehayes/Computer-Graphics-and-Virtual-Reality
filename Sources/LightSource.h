#ifndef LIGHT_SOURCE_H
#define LIGHT_SOURCE_H


#include "ShaderProgram.h"
#include "Transform.h"


class LightSource : public Transform {
private:
  unsigned int num;
  glm::vec3 position;
  glm::vec3 color;
  glm::vec3 direction;
  float intensity;
  float coneAngle;
  float ac=1;
  float al=0;
  float aq=0;

public:
  LightSource(glm::vec3 pos, glm::vec3 direct, glm::vec3 col ,float inten, float angle,float aC, float aL,float aQ){
    direction=direct;
    position=pos;
    color=col;
    intensity=inten;
    coneAngle=angle;
    ac=aC;
    al=aL;
    aq=aQ;
  }
  LightSource(int numero, glm::vec3 pos, glm::vec3 col, float inten ) {
    num=numero;
    position=pos;
    color=col;
    intensity=inten;
    coneAngle=0.80;

  }

  void sendToShader(std::shared_ptr<ShaderProgram> shaderProgramPtr);
};

#endif
