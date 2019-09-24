#ifndef MATERIAL_H
#define MATERIAL_H

#include "ShaderProgram.h"
#include "stb_image.h"

class Material {
private:
  glm::vec3 couleur;
  float alpha;
public:
  Material(glm::vec3 color, float a) {
    couleur= color;
    alpha = a;
  }
  Material(float r, float g, float b, float a ) {
    couleur= glm::vec3(r,g,b);
    alpha=a;
  }


  void sendToShader(std::shared_ptr<ShaderProgram> shaderProgramPtr, GLuint albedoTex, GLuint roughnessTex, GLuint metallicTex, GLuint a0Tex, GLuint toonTex, GLuint xToonTex) {
    shaderProgramPtr->set ("material.albedo", couleur);
    shaderProgramPtr->set("material.alpha", alpha);
    shaderProgramPtr->set("material.albedoTex", 0);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, albedoTex);

    shaderProgramPtr->set("material.roughnessTex", 1);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, roughnessTex);

    shaderProgramPtr->set("material.metallicTex", 2);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, metallicTex);

    shaderProgramPtr->set("material.a0Tex", 3);
    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, a0Tex);

    shaderProgramPtr->set("material.toonTex", 4);
    glActiveTexture(GL_TEXTURE4);
    glBindTexture(GL_TEXTURE_2D, toonTex);

    shaderProgramPtr->set("material.xToonTex", 5);
    glActiveTexture(GL_TEXTURE5);
    glBindTexture(GL_TEXTURE_2D, xToonTex);
  }

  GLuint loadTextureFromFileToGPU (const std::string & filename, int numTex) {
     int width, height, numComponents;

     // Loading the image in CPU memory using stbd_image
     unsigned char * data = stbi_load (filename.c_str (),
     &width,
     &height,
     &numComponents, // 1 for a 8 bit greyscale image, 3 for 24bits RGB image
     0);
     std::cout << "height= " << height << " width= " << width << "\n" ;
     // Create a texture in GPU memory
     GLuint texID;
     glGenTextures (1, &texID);
     glBindTexture (GL_TEXTURE_2D, texID);
     glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
     glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
     glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
     glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

     // Uploading the image data to GPU memory
     glTexImage2D (GL_TEXTURE_2D,
     0,
     (numComponents == 1 ? GL_RED : GL_RGB), // We assume only greyscale or RGB pixels
     width,
     height,
     0,
     (numComponents == 1 ? GL_RED : GL_RGB), // We assume only greyscale or RGB pixels
     GL_UNSIGNED_BYTE,
     data);
     // Generating mipmaps for filtered texture fetch
     glGenerateMipmap(GL_TEXTURE_2D);
     // Freeing the now useless CPU memory
     stbi_image_free(data);
     glBindTexture (GL_TEXTURE_2D, numTex);
     return texID;
  }
};

#endif // MATERIAL_H
