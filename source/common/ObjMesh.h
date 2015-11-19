//http://www.opengl-tutorial.org/beginners-tutorials/tutorial-7-model-loading/


#ifndef __OBJLOADER__
#define __OBJLOADER__

#include "Angel.h"
#include <vector>
#include <list>
#include <iomanip>

using namespace Angel;

class Mesh{
public:
  bool hasUV;
  
  std::vector < vec4 > vertices;
  std::vector < vec2 > uvs;
  std::vector < vec3 > normals;
  
  vec3 Box_min = vec3(std::numeric_limits< float >::max() ,
                      std::numeric_limits< float >::max() ,
                      std::numeric_limits< float >::max() );
  vec3 Box_max = vec3(0,0,0);
  
  unsigned int getNumTri(){ return vertices.size()/3; }

  bool loadOBJ(const char * path){
    std::vector< unsigned int > vertexIndices, uvIndices, normalIndices;
    std::vector< vec3 > temp_vertices;
    std::vector< vec2 > temp_uvs;
    std::vector< vec3 > temp_normals;
    
    hasUV = true;
    
    FILE * file = fopen(path, "r");
    if( file == NULL ){
      printf("Impossible to open the file !\n");
      return false;
    }
    
    char *line = new char[128];
    char *lineHeader = new char[128];
    
    while(true){
      memset(line, 0 , 128);
      memset(lineHeader, 0 , 128);
      
      if(fgets(line, 128, file) == NULL){ break; }
      sscanf(line, "%s ", lineHeader);

      if ( strcmp( lineHeader, "v" ) == 0 ){
        vec3 vertex;
        sscanf(&line[2], "%f %f %f", &vertex.x, &vertex.y, &vertex.z );
        temp_vertices.push_back(vertex);
        if(vertex.x < Box_min.x){Box_min.x = vertex.x; }
        if(vertex.y < Box_min.y){Box_min.y = vertex.y; }
        if(vertex.z < Box_min.z){Box_min.z = vertex.z; }
        if(vertex.x > Box_max.x){Box_max.x = vertex.x; }
        if(vertex.y > Box_max.y){Box_max.y = vertex.y; }
        if(vertex.z > Box_max.z){Box_max.z = vertex.z; }
      }else if ( strcmp( lineHeader, "vt" ) == 0 ){
        vec2 uv;
        sscanf(&line[3], "%f %f", &uv.x, &uv.y );
        temp_uvs.push_back(uv);
      }else if ( strcmp( lineHeader, "vn" ) == 0 ){
        vec3 normal;
        sscanf(&line[3], "%f %f %f", &normal.x, &normal.y, &normal.z );
        temp_normals.push_back(normal);
      }else if ( strcmp( lineHeader, "f" ) == 0 ){
        std::string vertex1, vertex2, vertex3;
        unsigned int vertexIndex[3], uvIndex[3], normalIndex[3];
        int matches = sscanf(&line[2], "%d/%d/%d %d/%d/%d %d/%d/%d\n", &vertexIndex[0], &uvIndex[0], &normalIndex[0],
                                                                       &vertexIndex[1], &uvIndex[1], &normalIndex[1],
                                                                       &vertexIndex[2], &uvIndex[2], &normalIndex[2]);
        if (matches != 9){
          int matches = sscanf(&line[2], "%d//%d %d//%d %d//%d\n", &vertexIndex[0], &normalIndex[0],
                                                                   &vertexIndex[1], &normalIndex[1],
                                                                   &vertexIndex[2], &normalIndex[2]);
          if (matches == 6){
            hasUV = false;
          } else {
            printf("File can't be read by our simple parser : ( Try exporting with other options\n");
            return false;
          }
        }
        vertexIndices.push_back(vertexIndex[0]);
        vertexIndices.push_back(vertexIndex[1]);
        vertexIndices.push_back(vertexIndex[2]);
        if(hasUV){
          uvIndices    .push_back(uvIndex[0]);
          uvIndices    .push_back(uvIndex[1]);
          uvIndices    .push_back(uvIndex[2]);
        }
        normalIndices.push_back(normalIndex[0]);
        normalIndices.push_back(normalIndex[1]);
        normalIndices.push_back(normalIndex[2]);
      }
    }
    
    delete[] line;
    delete[] lineHeader;
    
    std::cout << "Read " << temp_vertices.size() << " vertices\n";
    std::cout << "Read " << temp_normals.size() << " normals\n";
    std::cout << "Read " << vertexIndices.size()/3 << " faces\n";

    
    // For each vertex of each triangle
    for( unsigned int i=0; i<vertexIndices.size(); i++ ){
      unsigned int vertexIndex = vertexIndices[i];
      vec4 vertex = vec4(temp_vertices[ vertexIndex-1 ], 1.0);
      vertices.push_back(vertex);
    }
    
    if(hasUV){
      for( unsigned int i=0; i<uvIndices.size(); i++ ){
        unsigned int uvIndex = uvIndices[i];
        vec2 uv = temp_uvs[ uvIndex-1 ];
        uvs.push_back(uv);
      }
    }
  
    for( unsigned int i=0; i<normalIndices.size(); i++ ){
      unsigned int normalIndex = normalIndices[i];
      vec3 normal = temp_normals[ normalIndex-1 ];
      normals.push_back(normal);
    }
    
    std::cout << "Total " << vertices.size() << " vertices\n";
    std::cout << "Total " << normals.size() << " normals\n";
    
    return true;
}
  
  class SphereTriangle{
  public:
    SphereTriangle(vec3 _p1, vec3 _p2, vec3 _p3): a(_p1), b(_p2), c(_p3){};
    vec3 a;
    vec3 b;
    vec3 c;
  };
  
  
  vec3 setLength(vec3 p1, float r){
    float rl = r/length(p1);
    return vec3(p1.x*rl, p1.y*rl, p1.z*rl);
  }
  
  bool makeSubdivisionSphere(int steps=8){
    
    Box_min = vec3(-1,-1,-1);
    Box_max = vec3(1,1,1);

    
    std::list < SphereTriangle > tris;
    
    tris.push_back(SphereTriangle(vec3(0.57735, 0.57735, 0.57735),    vec3(-0.57735, 0.57735, -0.57735), vec3(0.57735, 0.57735, -0.57735)));
    tris.push_back(SphereTriangle(vec3(-0.57735, 0.57735, -0.57735),  vec3(0.57735, 0.57735, 0.57735), vec3(-0.57735, 0.57735, 0.57735)));
    tris.push_back(SphereTriangle(vec3(0.57735, -0.57735, 0.57735),   vec3(-0.57735, -0.57735, -0.57735), vec3(0.57735, -0.57735, -0.57735)));
    tris.push_back(SphereTriangle(vec3(-0.57735, -0.57735, -0.57735), vec3(0.57735, -0.57735, 0.57735), vec3(-0.57735, -0.57735, 0.57735)));
    tris.push_back(SphereTriangle(vec3(0.57735, 0.57735, 0.57735),    vec3(0.57735, -0.57735, -0.57735), vec3(0.57735, 0.57735, -0.57735)));
    tris.push_back(SphereTriangle(vec3(0.57735, -0.57735, -0.57735),  vec3(0.57735, 0.57735, 0.57735), vec3(0.57735, -0.57735, 0.57735)));
    tris.push_back(SphereTriangle(vec3(-0.57735, 0.57735, 0.57735),   vec3(-0.57735, -0.57735, -0.57735), vec3(-0.57735, 0.57735, -0.57735)));
    tris.push_back(SphereTriangle(vec3(-0.57735, -0.57735, -0.57735), vec3(-0.57735, 0.57735, 0.57735), vec3(-0.57735, -0.57735, 0.57735)));
    tris.push_back(SphereTriangle(vec3(0.57735, 0.57735, 0.57735),    vec3(-0.57735, -0.57735, 0.57735), vec3(0.57735, -0.57735, 0.57735)));
    tris.push_back(SphereTriangle(vec3(-0.57735, -0.57735, 0.57735),  vec3(0.57735, 0.57735, 0.57735), vec3(-0.57735, 0.57735, 0.57735)));
    tris.push_back(SphereTriangle(vec3(0.57735, 0.57735, -0.57735),   vec3(-0.57735, -0.57735, -0.57735), vec3(0.57735, -0.57735, -0.57735)));
    tris.push_back(SphereTriangle(vec3(-0.57735, -0.57735, -0.57735), vec3(0.57735, 0.57735, -0.57735), vec3(-0.57735, 0.57735, -0.57735)));

    for(unsigned int s=0; s < steps; s++){
      std::list< SphereTriangle > newTris;
      float l = length(tris.begin()->a);
      for(std::list<SphereTriangle>::iterator i = tris.begin(); i != tris.end(); ++i) { // go through all triangles
        vec3 mid = (i->a + i->b) * 0.5f; // point betwenn points A and B
        mid = setLength(mid, l); // put in on the sphere
        newTris.push_back(SphereTriangle(i->b, i->c, mid)); // remember new triangles
        newTris.push_back(SphereTriangle(i->a, i->c, mid));
      }
      tris.swap(newTris); // use new set of triangles;
    }
    
    
  
    for(std::list<SphereTriangle>::iterator i = tris.begin(); i != tris.end(); ++i) {
      vertices.push_back(vec4(normalize(i->a), 1.0));
      vertices.push_back(vec4(normalize(i->b), 1.0));
      vertices.push_back(vec4(normalize(i->c), 1.0));
      
      normals.push_back(normalize(i->a));
      normals.push_back(normalize(i->b));
      normals.push_back(normalize(i->c));
    }
    
    return true;
  }
  
  bool makeParametricSphere(int steps=32){
    return true;
  }
    

  
  friend std::ostream& operator << ( std::ostream& os, const Mesh& v ) {
    os << "Vertices:\n";
    for(unsigned int i=0; i < v.vertices.size(); i++){
      os << "\t\t" << v.vertices[i] << "\n";
    }
    os << "Normals:\n";
    for(unsigned int i=0; i < v.normals.size(); i++){
      os << "\t\t" << v.normals[i] << "\n";
    }
 
    return os;
  }
  
};


#endif  //#ifndef __OBJLOADER__
