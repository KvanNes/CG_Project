#ifndef HPP_GEOMETRY
#define HPP_GEOMETRY
#include <GL/gl.h>
#include <glm/vec3.hpp>
#include "Vertex.hpp"
#include "BoundingEllipsoid.hpp"

class Geometry {
    Vertex* vertices;
    unsigned int* indices;
    GLsizei numVertices, numIndices;
    std::vector<std::vector<int> > vertToTriConnections;

    public:
    Geometry() {
        numVertices = 0;
        numIndices = 0;
        vertices = 0;
        indices = 0;
    }

    const Vertex* getVertices() const{
        return vertices;
    }
    const unsigned int* getIndices() const{
        return indices;
    }
    GLsizei getNumVertices() const{
        return numVertices;
    }
    GLsizei getNumIndices() const{
        return numIndices;
    }

    void remove() {
        if (vertices != 0){
            delete [] vertices;
            assert(indices != 0);
            delete [] indices;
            vertices = 0;
            indices = 0;
        }
    }
    ~Geometry() {
        remove();
    }

    bool loadOBJ(const char * path, bool hasTexture);

    protected:
    glm::vec3 calcTranformedPos(glm::vec3 pos, glm::vec3* directions, int numDirections, float noiseLength);
    void addTriangleAndConnections(GLuint triangle, GLuint vert1, GLuint vert2, GLuint vert3);
    void calculateConnections();
    glm::vec3 calcTriangleNormal(GLuint* indices);

    public:
    void makeRandomMeteor(int numSeg, int numRing, int numDirections, float noiseLength);
    void makeSphere(int numSeg, int numRing);
    void makeQuad();

    //BoundingSphere approxBoundingSphere();
    BoundingEllipsoid approxBoundingEllipsoid() const;

    void calculateNormals();
};

#endif // HPP_GEOMETRY
