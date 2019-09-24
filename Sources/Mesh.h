#ifndef MESH_H
#define MESH_H

#include <glad/glad.h>
#include <vector>
#include <memory>
#include <map>

#include <glm/glm.hpp>
#include <glm/ext.hpp>

#include "Transform.h"

//Definition des structures
struct Box {
	glm::vec3 origin;
	float cubeSize;
	bool isInside(glm::vec3 position) {
		if(position.x>origin.x && position.x<origin.x+cubeSize && position.y>origin.y && position.y<origin.y+cubeSize && position.z>origin.z && position.z<origin.z+cubeSize ) {
			return true;
		}
		return false;
	}
};

struct OctreeNode {
	std::vector<OctreeNode*> children=std::vector<OctreeNode*>(8);
	std::vector<unsigned int> vertices;
	Box box;
	int index = -1 ;
	glm::vec3 finalVertex;
	glm::vec3 finalNormal;
};


class Mesh : public Transform {
public:
	virtual ~Mesh ();

	inline const std::vector<glm::vec3> & vertexPositions () const { return m_vertexPositions; }
	inline std::vector<glm::vec3> & vertexPositions () { return m_vertexPositions; }
	inline const std::vector<glm::vec3> & vertexNormals () const { return m_vertexNormals; }
	inline std::vector<glm::vec3> & vertexNormals () { return m_vertexNormals; }
	inline const std::vector<glm::vec2> & vertexTexCoords () const { return m_vertexTexCoords; }
	inline std::vector<glm::vec2> & vertexTexCoords () { return m_vertexTexCoords; }
	inline const std::vector<glm::uvec3> & triangleIndices () const { return m_triangleIndices; }
	inline std::vector<glm::uvec3> & triangleIndices () { return m_triangleIndices; }

	void computePlanarParameterization();
	/// Compute the parameters of a sphere which bounds the mesh
	void computeBoundingSphere (glm::vec3 & center, float & radius) const;

	void recomputePerVertexNormals (bool angleBased = false);

	void simplify (unsigned int resolution);
	//void adaptiveSimplify (unsigned int numOfPerLeafVertices);
	void laplacianFilter(float alpha = 0.5, bool cotangentWeights = true);
	void computeWeight();
	void subdivide();
	OctreeNode * buildOctree(Box box,std::vector<unsigned int> vertices, unsigned int numOfPerLeafVertices, std::vector<glm::vec3> &newVertex, std::vector <glm::vec3> &newNormal);
	void adaptiveSimplify(unsigned int numOfPerLeafVertices);

	void computeNeighborhood();


	void init ();
	void render ();
	void clear ();

private:
	std::vector<glm::vec3> m_vertexPositions;
	std::vector<glm::vec3> m_vertexNormals;
	std::vector<glm::vec2> m_vertexTexCoords;
	std::vector<glm::uvec3> m_triangleIndices;

	std::vector<glm::vec3> m_sumWeightPosition;
	std::vector<std::map<int,float>> m_weight;

	std::vector<std::map<unsigned int, glm::vec3>> m_1Neighborhood;

	GLuint m_vao = 0;
	GLuint m_posVbo = 0;
	GLuint m_normalVbo = 0;
	GLuint m_texCoordVbo = 0;
	GLuint m_ibo = 0;


	bool weightComputed=false;
};

#endif // MESH_H
