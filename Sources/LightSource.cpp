#include "LightSource.h"


void LightSource::sendToShader(std::shared_ptr<ShaderProgram> shaderProgramPtr){
  shaderProgramPtr->set ("lightSource["+std::to_string(num)+"].position",position );
	shaderProgramPtr->set ("lightSource["+std::to_string(num)+"].color", color);
	shaderProgramPtr->set ("lightSource["+std::to_string(num)+"].intensity", intensity);
  shaderProgramPtr->set ("lightSource["+std::to_string(num)+"].ac", ac);
  shaderProgramPtr->set ("lightSource["+std::to_string(num)+"].al", al);
  shaderProgramPtr->set ("lightSource["+std::to_string(num)+"].aq", aq);

}
