#define _USE_MATH_DEFINES

#include "Mesh.h"

#include <cmath>
#include <algorithm>
#include <iostream>

using namespace std;

Mesh::~Mesh () {
	clear ();
}

void Mesh::computeBoundingSphere (glm::vec3 & center, float & radius) const {
	center = glm::vec3 (0.0);
	radius = 0.f;
	for (const auto & p : m_vertexPositions)
		center += p;
	center /= m_vertexPositions.size ();
	for (const auto & p : m_vertexPositions)
		radius = std::max (radius, distance (center, p));
}

void Mesh::recomputePerVertexNormals (bool angleBased) {
	m_vertexNormals.clear ();

	//Vector containing the normals of the faces
	std::vector<glm::vec3> facesNormal;


	//Compute normal of the faces
	for (unsigned int i=0; i< m_triangleIndices.size(); i++) {
		glm::uvec3 triangle=m_triangleIndices[i];
		glm::vec3 s0=m_vertexPositions[triangle.x];
		glm::vec3 s1=m_vertexPositions[triangle.y];
		glm::vec3 s2=m_vertexPositions[triangle.z];

		glm::vec3 produit=cross(s1-s0,s2-s0);
		float norm= sqrt(pow(produit.x,2)+ pow(produit.y,2)+pow(produit.z,2));
		facesNormal.push_back(produit/norm);
	}

	m_vertexNormals.resize (m_vertexPositions.size (), glm::vec3 (0.0, 0.0, 0.0));

	if (!angleBased) {
		for (unsigned int i=0; i<m_triangleIndices.size(); i++){
			glm::vec3 triangle=m_triangleIndices[i];
			m_vertexNormals[triangle.x]+=facesNormal[i];
			m_vertexNormals[triangle.y]+=facesNormal[i];
			m_vertexNormals[triangle.z]+=facesNormal[i];
		}
	}

	//Taking the angles into account to compute the face's normal
	else {
		for (unsigned int i=0; i<m_triangleIndices.size(); i++){
			glm::vec3 triangle=m_triangleIndices[i];
			glm::vec3 s0;
			glm::vec3 s1;
			glm::vec3 s2;
				if (triangle.x==i){
					s0=m_vertexPositions[i];
					s1=m_vertexPositions[triangle.y];
					s2=m_vertexPositions[triangle.z];
				}
				else if (triangle.y==i){
					s0=m_vertexPositions[i];
					s1=m_vertexPositions[triangle.x];
					s2=m_vertexPositions[triangle.z];
				}
				else {
					s0=m_vertexPositions[i];
					s1=m_vertexPositions[triangle.x];
					s2=m_vertexPositions[triangle.y];
				}
				glm::vec3  vec2=s1-s0;
				float prodScal=dot(vec2,(s2-s0));
				float angle=acos(prodScal/vec2.length());

				m_vertexNormals[triangle.x]+=angle*facesNormal[i];
				m_vertexNormals[triangle.y]+=angle*facesNormal[i];
				m_vertexNormals[triangle.z]+=angle*facesNormal[i];
			}

	}
	for (unsigned int i=0; i<m_vertexNormals.size(); i++ ){
		m_vertexNormals[i]=normalize(m_vertexNormals[i]);
	}
}

void Mesh::init () {

	computePlanarParameterization();

	glCreateBuffers (1, &m_posVbo); // Generate a GPU buffer to store the positions of the vertices
	size_t vertexBufferSize = sizeof (glm::vec3) * m_vertexPositions.size (); // Gather the size of the buffer from the CPU-side vector
	glNamedBufferStorage (m_posVbo, vertexBufferSize, NULL, GL_DYNAMIC_STORAGE_BIT); // Create a data store on the GPU
	glNamedBufferSubData (m_posVbo, 0, vertexBufferSize, m_vertexPositions.data ()); // Fill the data store from a CPU array

	glCreateBuffers (1, &m_normalVbo); // Same for normal
	glNamedBufferStorage (m_normalVbo, vertexBufferSize, NULL, GL_DYNAMIC_STORAGE_BIT);
	glNamedBufferSubData (m_normalVbo, 0, vertexBufferSize, m_vertexNormals.data ());

	glCreateBuffers (1, &m_texCoordVbo); // Same for texture coordinates
	size_t texCoordBufferSize = sizeof (glm::vec2) * m_vertexTexCoords.size ();
	glNamedBufferStorage (m_texCoordVbo, texCoordBufferSize, NULL, GL_DYNAMIC_STORAGE_BIT);
	glNamedBufferSubData (m_texCoordVbo, 0, texCoordBufferSize, m_vertexTexCoords.data ());

	glCreateBuffers (1, &m_ibo); // Same for the index buffer, that stores the list of indices of the triangles forming the mesh
	size_t indexBufferSize = sizeof (glm::uvec3) * m_triangleIndices.size ();
	glNamedBufferStorage (m_ibo, indexBufferSize, NULL, GL_DYNAMIC_STORAGE_BIT);
	glNamedBufferSubData (m_ibo, 0, indexBufferSize, m_triangleIndices.data ());

	glCreateVertexArrays (1, &m_vao); // Create a single handle that joins together attributes (vertex positions, normals) and connectivity (triangles indices)
	glBindVertexArray (m_vao);
	glEnableVertexAttribArray (0);
	glBindBuffer (GL_ARRAY_BUFFER, m_posVbo);
	glVertexAttribPointer (0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof (GLfloat), 0);
	glEnableVertexAttribArray (1);
	glBindBuffer (GL_ARRAY_BUFFER, m_normalVbo);
	glVertexAttribPointer (1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof (GLfloat), 0);
	glEnableVertexAttribArray (2);
	glBindBuffer (GL_ARRAY_BUFFER, m_texCoordVbo);
	glVertexAttribPointer (2, 2, GL_FLOAT, GL_FALSE, 2 * sizeof (GLfloat), 0);
	glBindBuffer (GL_ELEMENT_ARRAY_BUFFER, m_ibo);
	glBindVertexArray (0); // Desactive the VAO just created. Will be activated at rendering time.

}

void Mesh::render () {
	glBindVertexArray (m_vao); // Activate the VAO storing geometry data
	glDrawElements (GL_TRIANGLES, static_cast<GLsizei> (m_triangleIndices.size () * 3), GL_UNSIGNED_INT, 0); // Call for rendering: stream the current GPU geometry through the current GPU program
}

void Mesh::clear () {
	m_vertexPositions.clear ();
	m_vertexNormals.clear ();
	m_vertexTexCoords.clear ();
	m_triangleIndices.clear ();
	if (m_vao) {
		glDeleteVertexArrays (1, &m_vao);
		m_vao = 0;
	}
	if(m_posVbo) {
		glDeleteBuffers (1, &m_posVbo);
		m_posVbo = 0;
	}
	if (m_normalVbo) {
		glDeleteBuffers (1, &m_normalVbo);
		m_normalVbo = 0;
	}
	if (m_texCoordVbo) {
		glDeleteBuffers (1, &m_texCoordVbo);
		m_texCoordVbo = 0;
	}
	if (m_ibo) {
		glDeleteBuffers (1, &m_ibo);
		m_ibo = 0;
	}
}

void Mesh::computePlanarParameterization() {
	m_vertexTexCoords.resize(m_vertexPositions.size(), glm::vec2(0.0,0.0));
	float valminx=m_vertexPositions[0].x;
	float valminy=m_vertexPositions[0].y;

	float valmaxx=m_vertexPositions[0].x;
	float valmaxy=m_vertexPositions[0].y;

	// Research on the Xmin and Ymin in the position
	for (unsigned int i=0 ; i< m_vertexPositions.size() ; i++) {
		if (m_vertexPositions[i].x<=valminx){
			valminx=m_vertexPositions[i].x;

		}
		if (m_vertexPositions[i].y<=valminy){
			valminy=m_vertexPositions[i].y;

		}
		if (m_vertexPositions[i].x>=valmaxx){
			valmaxx=m_vertexPositions[i].x;

		}
		if (m_vertexPositions[i].y>=valmaxy){
			valmaxy=m_vertexPositions[i].y;

		}
	}
//Computing du U and V parameters
for (unsigned int k=0; k<m_vertexPositions.size() ; k++) {
	m_vertexTexCoords[k].x= (m_vertexPositions[k].x-valminx)/(valmaxx-valminx);
	m_vertexTexCoords[k].y= (m_vertexPositions[k].y-valminy)/(valmaxy-valminy);
}
}

void Mesh::computeWeight(){
	m_weight.clear();
	m_weight.resize(m_vertexPositions.size());

	for (unsigned int i=0; i<m_triangleIndices.size(); i++){
		glm::vec3 x=m_vertexPositions[m_triangleIndices[i].x];
		glm::vec3 y=m_vertexPositions[m_triangleIndices[i].y];
		glm::vec3 z=m_vertexPositions[m_triangleIndices[i].z];

		glm::vec3 vecxy=y-x;
		glm::vec3 vecyz=y-z;
		glm::vec3 veczx=z-x;

		float anglex=acos((dot(y-x,z-x))/(length(y-x)*length(z-x)));
		float angley=acos((dot(x-y,z-y))/(length(x-y)*length(z-y)));
		float anglez=acos((dot(x-z,y-z))/(length(x-z)*length(y-z)));

		m_weight[m_triangleIndices[i].x][m_triangleIndices[i].y]+=cos(anglez)/sin(anglez)/2;
		m_weight[m_triangleIndices[i].y][m_triangleIndices[i].x]+=cos(anglez)/sin(anglez)/2;
		m_weight[m_triangleIndices[i].y][m_triangleIndices[i].z]+=cos(anglex)/sin(anglex)/2;
		m_weight[m_triangleIndices[i].z][m_triangleIndices[i].y]+=cos(anglex)/sin(anglex)/2;
		m_weight[m_triangleIndices[i].x][m_triangleIndices[i].z]+=cos(angley)/sin(angley)/2;
		m_weight[m_triangleIndices[i].z][m_triangleIndices[i].x]+=cos(angley)/sin(angley)/2;

	}
	weightComputed=true;
}


void Mesh::laplacianFilter(float alpha , bool cotangentWeights ) {
	m_sumWeightPosition.clear();
	m_sumWeightPosition.resize(m_vertexPositions.size(),glm::vec3(0));

	//QUESTION 2: avec les poids cotangents
	if(cotangentWeights){
		if(weightComputed==false) { //Si les poids sont déjà calculés on ne les reclaculent pas on garde ceux du début
			computeWeight();
		}

		for (unsigned int i=0 ; i<m_vertexPositions.size() ; i++){
			float sum=0;
			glm::vec3 vi_filtre(0,0,0);
			for( std::map< int , float >::iterator it = m_weight[i].begin() ; it != m_weight[i].end() ; ++it ) {
				int j = it->first;
				float w_ij = it->second; // m_weight[i][j]
				sum += w_ij;
				vi_filtre += w_ij * m_vertexPositions[j];
			}
			m_sumWeightPosition[i]= vi_filtre / sum;
		}
		for (unsigned int i=0 ; i<m_vertexPositions.size() ; i++){
			m_vertexPositions[i]=(1-alpha)*m_sumWeightPosition[i]+alpha*m_vertexPositions[i];
		}
	}

	//QUESTION 1: sans les poids
	else {
		std::vector<int> nbVois;
		nbVois.resize(m_vertexPositions.size(),0);
		for (unsigned int i=0 ; i<m_triangleIndices.size() ; i++){
			m_sumWeightPosition[m_triangleIndices[i].x]+=m_vertexPositions[m_triangleIndices[i].y]+m_vertexPositions[m_triangleIndices[i].z];
			nbVois[m_triangleIndices[i].x]+=2;

			m_sumWeightPosition[m_triangleIndices[i].y]+=m_vertexPositions[m_triangleIndices[i].x]+m_vertexPositions[m_triangleIndices[i].z];
			nbVois[m_triangleIndices[i].y]+=2;

			m_sumWeightPosition[m_triangleIndices[i].z]+=m_vertexPositions[m_triangleIndices[i].y]+m_vertexPositions[m_triangleIndices[i].x];
			nbVois[m_triangleIndices[i].z]+=2;
		}
		for (unsigned int i=0 ; i<m_vertexPositions.size() ; i++){
			m_vertexPositions[i]=(1-alpha)*m_sumWeightPosition[i]/(float)nbVois[i]+alpha*m_vertexPositions[i];
		}
	}


	recomputePerVertexNormals();

	size_t vertexBufferSize = sizeof (glm::vec3) * m_vertexPositions.size ();
	glBindBuffer (GL_ARRAY_BUFFER, m_posVbo);
	glNamedBufferSubData (m_posVbo, 0, vertexBufferSize, m_vertexPositions.data ());

}

struct cell {
	unsigned int nbVertex;
	glm::vec3 moyVertex;
	glm::vec3 moyNormal;
	int nouvIndex;
};
struct grid {
	glm::vec3 origin;
	float resolution;
	float cubeSize;
	std::vector<cell> cells;
	unsigned int getCell(glm::vec3 position) {
		glm::vec3 cubePosition = (position-origin)*(resolution - 1) /cubeSize  ; // Position dans le repère cube. (coté allant de 0 à résolution -1)
		return ((int)(cubePosition.x)) + ((int)(cubePosition.y)) * resolution + ((int)(cubePosition.z)) * pow(resolution,2);
	}
}m_grid;


void Mesh::simplify (unsigned int resolution) {
	float minx=m_vertexPositions[0].x;
	float maxx=m_vertexPositions[0].x;
	float miny=m_vertexPositions[0].y;
	float maxy=m_vertexPositions[0].y;
	float minz=m_vertexPositions[0].z;
	float maxz=m_vertexPositions[0].z;

	for (unsigned int i=1 ; i<m_vertexPositions.size() ; i++) {
		if (m_vertexPositions[i].x<minx) {
			minx=m_vertexPositions[i].x;
		}
		if (m_vertexPositions[i].x>maxx) {
			maxx=m_vertexPositions[i].x;
		}
		if (m_vertexPositions[i].y<miny) {
			miny=m_vertexPositions[i].y;
		}
		if (m_vertexPositions[i].y>maxy) {
			maxy=m_vertexPositions[i].y;
		}
		if (m_vertexPositions[i].z<minz) {
			minz=m_vertexPositions[i].z;
		}
		if (m_vertexPositions[i].z>maxz) {
			maxx=m_vertexPositions[i].z;
		}
	}
	m_grid.resolution=resolution;
	m_grid.cubeSize=max(max(maxx-minx,maxy-miny),maxz-minz);


	//Pour ajouter une marge, on prend 2% de la diagonale, reviens à augmenter le coté de 2%
	m_grid.origin=glm::vec3(minx-m_grid.cubeSize*sqrt(3)*0.02,miny-m_grid.cubeSize*sqrt(3)*0.02,minz-m_grid.cubeSize*sqrt(3)*0.02);
	m_grid.cubeSize= m_grid.cubeSize + 2*m_grid.cubeSize*0.02;

	//INITIALISATION DU TABLEAU CELLS CONTENANT TOUTES LES CELLULES
	for (unsigned int i=0 ; i<pow(resolution,3); i++) {

		m_grid.cells.push_back(cell());
		m_grid.cells[i].nbVertex=0;
		m_grid.cells[i].moyVertex=glm::vec3(0,0,0);
		m_grid.cells[i].moyNormal=glm::vec3(0,0,0);
		m_grid.cells[i].nouvIndex=-1 ;
	}

	//--------------- QUADRILLAGE------------------------

	std::vector<glm::uvec3> m_newTriangleIndices;

	//REMPLISSAGE DES CELLS AVEC LES VERTEX DEDANS
	for (unsigned int i=0 ; i<m_vertexPositions.size() ; i++) {

		int indiceCell=m_grid.getCell(m_vertexPositions[i]);

		m_grid.cells[indiceCell].nbVertex+=1;
		m_grid.cells[indiceCell].moyVertex+=m_vertexPositions[i];
		m_grid.cells[indiceCell].moyNormal+=m_vertexNormals[i];

	}

	//DIVISION PAR LE NOMBRE DE VERTEX PAR CELLULE POUR AVOIR LA MOYENNE
	for (unsigned int i=0 ; i<m_grid.cells.size() ; i++) {
		m_grid.cells[i].moyVertex=m_grid.cells[i].moyVertex/(float)(m_grid.cells[i].nbVertex);
		m_grid.cells[i].moyNormal=m_grid.cells[i].moyNormal/(float)(m_grid.cells[i].nbVertex);
	}

std::vector<glm::vec3> nouvVertex;
std::vector<glm::vec3> nouvNormal;

 //CREATION DES NOUVEAUX TABLEAU DE VERTEX ET NORMALES
for (unsigned int i=0 ; i<m_grid.cells.size() ; i++) {
	if (m_grid.cells[i].nbVertex != 0) {
		nouvVertex.push_back(m_grid.cells[i].moyVertex);
		nouvNormal.push_back(m_grid.cells[i].moyNormal);
		m_grid.cells[i].nouvIndex=nouvVertex.size()-1;
	}
}
//ETABLISSEMENT DES TRIANGLES
for(unsigned int i=0 ; i<m_triangleIndices.size() ; i++) {
 unsigned int cellx = m_grid.getCell(m_vertexPositions[m_triangleIndices[i].x]);
 unsigned int celly = m_grid.getCell(m_vertexPositions[m_triangleIndices[i].y]);
 unsigned int cellz = m_grid.getCell(m_vertexPositions[m_triangleIndices[i].z]);

 if (cellx!= celly && celly!= cellz && cellz!= cellx) {
	 m_newTriangleIndices.push_back(glm::uvec3(m_grid.cells[cellx].nouvIndex,m_grid.cells[celly].nouvIndex,m_grid.cells[cellz].nouvIndex));
 }
}
	//COPIE DANS LES VRAIS TABLEAUX
	m_vertexPositions.resize (nouvVertex.size (),glm::vec3 (0.0, 0.0, 0.0));
	m_vertexNormals.resize (nouvNormal.size (),glm::vec3 (0.0, 0.0, 0.0));
	for (unsigned int i=0 ; i<m_vertexPositions.size(); i++ ) {
		m_vertexPositions[i]=nouvVertex[i];
		m_vertexNormals[i]=nouvNormal[i];
	}

	m_triangleIndices.resize (m_newTriangleIndices.size (),glm::uvec3 (0.0, 0.0, 0.0));
	for (unsigned int i=0 ; i<m_triangleIndices.size() ; i++) {
		m_triangleIndices[i].x=m_newTriangleIndices[i].x;
		m_triangleIndices[i].y=m_newTriangleIndices[i].y;
		m_triangleIndices[i].z=m_newTriangleIndices[i].z;
	}

	init();

}



OctreeNode * Mesh::buildOctree(Box box,std::vector<unsigned int> vertices, unsigned int numOfPerLeafVertices, std::vector<glm::vec3> &newVertex, std::vector <glm::vec3> &newNormal) {
	OctreeNode * node= new OctreeNode();
	node->box=box;
	node->vertices=vertices;
	if (node->vertices.size()<=numOfPerLeafVertices) {

		if(node->vertices.size()!=0){
		for (unsigned int i=0 ; i< node->vertices.size() ; i++) {
			node->finalVertex+=m_vertexPositions[node->vertices[i]];
			node->finalNormal+=m_vertexNormals[node->vertices[i]];
		}
		node->finalVertex=node->finalVertex/(float)(node->vertices.size());
		node->finalNormal=node->finalNormal/(float)(node->vertices.size());

		node->children.clear(); // C'est une feuille, il n'a plus de fils

		newVertex.push_back(node->finalVertex);
		newNormal.push_back(node->finalNormal);
		node->index=newVertex.size()-1;
	}
	}
	else {
		//Initialisation des 8 box fils
		for (unsigned int i=0 ; i<8 ; i++) {
			node->children[i]=new OctreeNode();
			node->children[i]->box= Box();
			node->children[i]->box.cubeSize=node->box.cubeSize/2.f;
		}
		// Création des 8 box fils
		node->children[0]->box.origin=node->box.origin ;
		node->children[1]->box.origin=glm::vec3(node->box.origin.x+node->box.cubeSize/2.f , node->box.origin.y , node->box.origin.z) ;
		node->children[2]->box.origin=glm::vec3(node->box.origin.x+node->box.cubeSize/2.f , node->box.origin.y+node->box.cubeSize/2.f , node->box.origin.z) ;
		node->children[3]->box.origin=glm::vec3(node->box.origin.x+node->box.cubeSize/2.f , node->box.origin.y+node->box.cubeSize/2.f , node->box.origin.z+node->box.cubeSize/2.f) ;
		node->children[4]->box.origin=glm::vec3(node->box.origin.x+node->box.cubeSize/2.f , node->box.origin.y , node->box.origin.z+node->box.cubeSize/2.f) ;
		node->children[5]->box.origin=glm::vec3(node->box.origin.x , node->box.origin.y+node->box.cubeSize/2.f , node->box.origin.z) ;
		node->children[6]->box.origin=glm::vec3(node->box.origin.x , node->box.origin.y+node->box.cubeSize/2.f , node->box.origin.z+node->box.cubeSize/2.f ) ;
		node->children[7]->box.origin=glm::vec3(node->box.origin.x , node->box.origin.y , node->box.origin.z+node->box.cubeSize/2.f ) ;

		//On passe les données aux fils
		for (unsigned int i=0 ; i<node->vertices.size() ; i++) {
			for (unsigned int j=0 ; i<8 ; i++) {
				if(node->children[j]->box.isInside(m_vertexPositions[node->vertices[i]])) {
					node->children[j]->vertices.push_back(node->vertices[i]);
				}
			}
		}
		node->vertices.clear();
		//Récursivité
		for (unsigned int i=0 ; i<8 ; i++) {
			node->children[i]=buildOctree(node->children[i]->box,node->children[i]->vertices, numOfPerLeafVertices,newVertex,newNormal);

		}
	}
	return node;
}

OctreeNode* findNode(OctreeNode * node, glm::vec3 vertex) {
	if (node->index!=-1) {
		return node ;
	}
	else {
		for (unsigned int i=0 ; i<8 ; i++){
			if (node->children[i]->box.isInside(vertex)){
				return findNode(node->children[i], vertex);
			}
		}
	}
}

void Mesh::adaptiveSimplify(unsigned int numOfPerLeafVertices) {

	//--------------------CREATION CUBE ENGLOBANT -------------------------
	float minx=m_vertexPositions[0].x;
	float maxx=m_vertexPositions[0].x;
	float miny=m_vertexPositions[0].y;
	float maxy=m_vertexPositions[0].y;
	float minz=m_vertexPositions[0].z;
	float maxz=m_vertexPositions[0].z;

	for (unsigned int i=1 ; i<m_vertexPositions.size() ; i++) {
		if (m_vertexPositions[i].x<minx) {
			minx=m_vertexPositions[i].x;
		}
		if (m_vertexPositions[i].x>maxx) {
			maxx=m_vertexPositions[i].x;
		}
		if (m_vertexPositions[i].y<miny) {
			miny=m_vertexPositions[i].y;
		}
		if (m_vertexPositions[i].y>maxy) {
			maxy=m_vertexPositions[i].y;
		}
		if (m_vertexPositions[i].z<minz) {
			minz=m_vertexPositions[i].z;
		}
		if (m_vertexPositions[i].z>maxz) {
			maxx=m_vertexPositions[i].z;
		}
	}

	float cubeSize=max(max(maxx-minx,maxy-miny),maxz-minz);


	//Pour ajouter une marge, on prend 2% de la diagonale, reviens à augmenter le coté de 2%
	glm::vec3 origin=glm::vec3(minx-cubeSize*sqrt(3)*0.02,miny-cubeSize*sqrt(3)*0.02,minz-cubeSize*sqrt(3)*0.02);
	cubeSize= cubeSize + 2*cubeSize*0.02;
	std::vector<glm::vec3> newVertices;
	std::vector<glm::vec3> newNormal;

	Box boiteEnglobante;
	boiteEnglobante.origin=origin;
	boiteEnglobante.cubeSize=cubeSize;
	std::vector<unsigned int> allVertices=std::vector<unsigned int>(m_vertexPositions.size());

	for (unsigned int i=1 ; i<m_vertexPositions.size(); i++) {
		allVertices[i]=i;

	}


	OctreeNode * racine=buildOctree(boiteEnglobante, allVertices, numOfPerLeafVertices , newVertices , newNormal);


	std::vector<glm::uvec3> newTriangleIndices;

	//ETABLISSEMENT DES TRIANGLES
	for(unsigned int i=0 ; i<m_triangleIndices.size() ; i++) {
	 OctreeNode * nodex = new OctreeNode();
	 OctreeNode * nodey = new OctreeNode();
	 OctreeNode * nodez = new OctreeNode();

		nodex = findNode(racine, m_vertexPositions[m_triangleIndices[i].x]);
	  nodey = findNode(racine, m_vertexPositions[m_triangleIndices[i].y]);
	  nodez = findNode(racine, m_vertexPositions[m_triangleIndices[i].z]);


	 if (nodex->index!=nodey->index && nodey->index!=nodez->index && nodex->index!=nodez->index) {
		 newTriangleIndices.push_back(glm::uvec3(nodex->index,nodey->index,nodez->index));

	 }
	}


	//Copie des différents TABLEAUX

	m_vertexPositions.resize (newVertices.size (),glm::vec3 (0.0, 0.0, 0.0));
	m_vertexNormals.resize (newNormal.size (),glm::vec3 (0.0, 0.0, 0.0));
	for (unsigned int i=0 ; i<m_vertexPositions.size(); i++ ) {
		m_vertexPositions[i]=newVertices[i];
		m_vertexNormals[i]=newNormal[i];
	}

	m_triangleIndices.resize (newTriangleIndices.size (),glm::uvec3 (0.0, 0.0, 0.0));
	for (unsigned int i=0 ; i<m_triangleIndices.size() ; i++) {
		m_triangleIndices[i]=newTriangleIndices[i];
	}
	std::cout<<"2"<<std::endl;
	init();



}

void Mesh::computeNeighborhood() {
	m_1Neighborhood.resize (m_vertexPositions.size (),std::map<unsigned int, glm::vec3>());

	for (unsigned int i=0 ; i< m_triangleIndices.size() ; i++) {
		unsigned int indVec1= m_triangleIndices[i].x;
		unsigned int indVec2= m_triangleIndices[i].y;
		unsigned int indVec3= m_triangleIndices[i].z;

		//Pour le premier sommmet
		if (m_1Neighborhood[indVec1].find(indVec2)==m_1Neighborhood[indVec1].end()) {
			// Si il y est pas déjà, on ajoute le sommet 2 comme voisin du sommet 1
			m_1Neighborhood[indVec1][indVec2]=m_vertexPositions[indVec2];
		}
		if (m_1Neighborhood[indVec1].find(indVec3)==m_1Neighborhood[indVec1].end()) {
			// Si il y est pas déjà, on ajoute le sommet 3 comme voisin du sommet 1
			m_1Neighborhood[indVec1][indVec3]=m_vertexPositions[indVec3];
		}

		//Pour le deuxième sommmet
		if (m_1Neighborhood[indVec2].find(indVec1)==m_1Neighborhood[indVec2].end()) {
			// Si il y est pas déjà, on ajoute le sommet 1 comme voisin du sommet 2
			m_1Neighborhood[indVec2][indVec1]=m_vertexPositions[indVec1];
		}
		if (m_1Neighborhood[indVec2].find(indVec3)==m_1Neighborhood[indVec2].end()) {
			// Si il y est pas déjà, on ajoute le sommet 3 comme voisin du sommet 2
			m_1Neighborhood[indVec2][indVec3]=m_vertexPositions[indVec3];
		}

		//Pour le troisième sommmet
		if (m_1Neighborhood[indVec3].find(indVec1)==m_1Neighborhood[indVec3].end()) {
			// Si il y est pas déjà, on ajoute le sommet 1 comme voisin du sommet 3
			m_1Neighborhood[indVec3][indVec1]=m_vertexPositions[indVec1];
		}
		if (m_1Neighborhood[indVec3].find(indVec2)==m_1Neighborhood[indVec3].end()) {
			// Si il y est pas déjà, on ajoute le sommet 2 comme voisin du sommet 3
			m_1Neighborhood[indVec3][indVec2]=m_vertexPositions[indVec2];
		}
	}
}


void Mesh::subdivide() {
	std::map<std::string, glm::vec3> newVertices ;

	for (unsigned int i=0 ; i<m_triangleIndices.size() ; i++) {


		glm::vec3 verx= m_vertexPositions[m_triangleIndices[i].x];
		glm::vec3 very= m_vertexPositions[m_triangleIndices[i].y];
		glm::vec3 verz= m_vertexPositions[m_triangleIndices[i].z];

		newVertices[std::to_string(m_triangleIndices[i].x)+"-"+std::to_string(m_triangleIndices[i].y)]+= (3.f/16.f) * (verx + very)  + (1.f/8.f)* verz;
		newVertices[std::to_string(m_triangleIndices[i].y)+"-"+std::to_string(m_triangleIndices[i].x)]+= (3.f/16.f) * (verx + very)  + (1.f/8.f)* verz;

		newVertices[std::to_string(m_triangleIndices[i].y)+"-"+std::to_string(m_triangleIndices[i].z)]+= (3.f/16.f) * (very + verz)  + (1.f/8.f)* verx;
		newVertices[std::to_string(m_triangleIndices[i].z)+"-"+std::to_string(m_triangleIndices[i].y)]+= (3.f/16.f) * (very + verz)  + (1.f/8.f)* verx;

		newVertices[std::to_string(m_triangleIndices[i].z)+"-"+std::to_string(m_triangleIndices[i].x)]+= (3.f/16.f) * (verx + verz)  + (1.f/8.f)* very;
		newVertices[std::to_string(m_triangleIndices[i].x)+"-"+std::to_string(m_triangleIndices[i].z)]+= (3.f/16.f) * (verx + verz)  + (1.f/8.f)* very;

	}

	computeNeighborhood();

	for (unsigned int i=0 ; i<m_vertexPositions.size() ; i++ ) {

		float n=(float)m_1Neighborhood[i].size();

		float alphan=(1.f/64.f)*(40-pow(3+2*cos(2*M_PI/n),2));

		m_vertexPositions[i]= (1-alphan) * m_vertexPositions[i];

		for( std::map< unsigned int ,glm::vec3 >::iterator it = m_1Neighborhood[i].begin() ; it != m_1Neighborhood[i].end() ; it++ ) {
			unsigned int index = it->first;
			m_vertexPositions[i]= m_vertexPositions[i] + (alphan/n) * newVertices[std::to_string(i)+"-"+std::to_string(index)];
		}


	}
	std::vector<glm::uvec3> newTriangles ;

	for (unsigned int i=0 ; i<m_triangleIndices.size() ; i++) {

		unsigned int indVertex1= m_triangleIndices[i].x ;
		unsigned int indVertex2= m_triangleIndices[i].y ;
		unsigned int indVertex3= m_triangleIndices[i].z ;

		//Ajout des nouveaux sommets

		m_vertexPositions.push_back(newVertices[std::to_string(indVertex1)+"-"+std::to_string(indVertex2)]);
		unsigned int indVertex4 = m_vertexPositions.size()-1;


		m_vertexPositions.push_back(newVertices[std::to_string(indVertex2)+"-"+std::to_string(indVertex3)]);
		unsigned int indVertex5 = m_vertexPositions.size()-1;

		m_vertexPositions.push_back(newVertices[std::to_string(indVertex1)+"-"+std::to_string(indVertex3)]);
		unsigned int indVertex6 = m_vertexPositions.size()-1;


		//Création des 4 nouveaux triangles
		newTriangles.push_back(glm::uvec3(indVertex1, indVertex4, indVertex6));
		newTriangles.push_back(glm::uvec3(indVertex6, indVertex5, indVertex3));
		newTriangles.push_back(glm::uvec3(indVertex6, indVertex4, indVertex5));
		newTriangles.push_back(glm::uvec3(indVertex4, indVertex2, indVertex5));

	}

	m_triangleIndices.resize(newTriangles.size(),glm::uvec3(0.0,0.0,0.0));

	for ( unsigned int i=0 ; i<m_triangleIndices.size() ; i++) {
		m_triangleIndices[i]=newTriangles[i];
	}

	recomputePerVertexNormals(false);

	init();

}
