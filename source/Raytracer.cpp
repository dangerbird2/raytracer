//
//  Display a rotating cube with lighting
//
//  Light and material properties are sent to the shader as uniform
//    variables.  Vertex positions and normals are sent after each
//    rotation.

#include "Angel.h"
#include "SourcePath.h"
#include "Trackball.h"
#include "ObjMesh.h"
#include "FreeImage.h"
#include <pthread.h>

typedef Angel::vec4  color4;
typedef Angel::vec4  point4;


int window_width, window_height;

bool render_line;
Mesh mesh;
GLuint buffer_object;

GLuint vPosition, vNormal, vTexCoord;

GLuint program;

// Model-view and projection matrices uniform location
GLuint  ModelViewEarth, ModelViewLight, NormalMatrix, Projection;

mat4  projection;
mat4  model_view;


//==========Trackball Variables==========
static float curquat[4],lastquat[4];
/* current transformation matrix */
static float curmat[4][4];
mat4 curmat_a;
/* actual operation  */
static int scaling;
static int moving;
static int panning;
/* starting "moving" coordinates */
static int beginx, beginy;
/* ortho */
float ortho_x, ortho_y;
/* current scale factor */
static float scalefactor;

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
bool write_image(const char* filename, const unsigned char *Src,
                        int Width, int Height, int channels){

  FreeImage_Initialise();
  
  FREE_IMAGE_FORMAT fif ;
  int X,Y;
  
  int bpp = channels*8;
  
  //check if the extension is supportedd
  
  fif= FreeImage_GetFIFFromFilename(filename);
  
  if(fif == FIF_UNKNOWN )
    {
    std::cout << "IMAGE ERROR: Freewrite_image : " << "failed to save "
    << filename << " with FreeImage (reason: fif == FIF_UNKNOWN)";
    return false;
    }
  
  //GRAY
  if (channels==1)
    {
    FIBITMAP* bitmap=FreeImage_Allocate(Width, Height, bpp);
    
    if (!bitmap)
      {
      std::cout << "IMAGE ERROR: Freewrite_image : "
      << "Cannot allocate memory\n";
      return false;
      }
    
    unsigned char* bits =FreeImage_GetBits(bitmap);
    
    if (!bits)
      {
      std::cout << "IMAGE ERROR: Freewrite_image : "
      << "Cannot allocate memory\n";
      return false;
      }
    
    int pitch=FreeImage_GetPitch(bitmap);
    
    int step=bpp/8;
    for (Y=Height-1;Y>=0;Y--)
      {
      unsigned char* Dst=bits+pitch*Y;
      for (X=0;X<Width;X++,Dst+=step,Src+=step)
        {
        Dst[0]=Src[0];
        }
      }
    
    
    if (!FreeImage_Save(fif, bitmap, filename,0))
      {
      FreeImage_Unload(bitmap);
      std::cout << "IMAGE ERROR: Freewrite_image : " << "failed to save "
      << filename << " with FreeImage\n";
      return false;
      }
    
    FreeImage_Unload(bitmap);
    std::cout << "IMAGE: Image saved : " << filename << "\n";
    return true;
    }
  else // RGB/RGBA
    {
    //RGB
    if (channels==3)
      {
      FIBITMAP* bitmap=FreeImage_Allocate(Width, Height, 24,0,0,0);
      
      if (!bitmap)
        {
        std::cout << "IMAGE ERROR: Freewrite_image : "
        << "Cannot allocate memory\n";
        return false;
        }
      
      unsigned char* bits =FreeImage_GetBits(bitmap);
      
      if (!bits)
        {
        std::cout << "IMAGE ERROR: Freewrite_image : "
        << "Cannot allocate memory\n";
        return false;
        }
      
      int pitch=FreeImage_GetPitch(bitmap);
      
      for (Y=Height-1;Y>=0;Y--)
        {
        unsigned char* Dst=bits+pitch*Y;
        for (X=0;X<Width;X++,Dst+=3,Src+=3)
          {
          Dst[FI_RGBA_RED   ]=Src[0];
          Dst[FI_RGBA_GREEN ]=Src[1];
          Dst[FI_RGBA_BLUE  ]=Src[2];
          }
        }
      
      
      if (!FreeImage_Save(fif, bitmap, filename,0))
        {
        FreeImage_Unload(bitmap);
        std::cout << "IMAGE ERROR: Freewrite_image : " << "failed to save "
        << filename << " with FreeImage\n";
        return false;
        }
      
      FreeImage_Unload(bitmap);
      std::cout << "IMAGE: Image saved : " << filename << "\n";
      return true;
      
      }
    //RGBA
    else if (channels==4)
      {
      FIBITMAP* bitmap=FreeImage_Allocate(Width, Height, 32,0,0,0);
      
      if (!bitmap)
        {
        std::cout << "IMAGE ERROR: Freewrite_image : "
        << "Cannot allocate memory\n";
        return false;
        }
      
      unsigned char* bits =FreeImage_GetBits(bitmap);
      
      if (!bits)
        {
        std::cout << "IMAGE ERROR: Freewrite_image : "
        << "Cannot allocate memory\n";
        return false;
        }
      
      int pitch=FreeImage_GetPitch(bitmap);
      
      for (Y=Height-1;Y>=0;Y--)
        {
        unsigned char* Dst=bits+pitch*Y;
        for (X=0;X<Width;X++,Dst+=4,Src+=4)
          {
          Dst[FI_RGBA_RED  ]=Src[0];
          Dst[FI_RGBA_GREEN]=Src[1];
          Dst[FI_RGBA_BLUE ]=Src[2];
          Dst[FI_RGBA_ALPHA]=Src[3];
          }
        }
      
      if (!FreeImage_Save(fif, bitmap, filename,0))
        {
        FreeImage_Unload(bitmap);
        std::cout << "IMAGE ERROR: Freewrite_image : " << "failed to save "
        << filename << " with FreeImage\n";
        return false;
        }
      
      FreeImage_Unload(bitmap);
      std::cout << "IMAGE: Image saved : " << filename << "\n";
      return true;
      }
    else
      {
      //not supported
      std::cout << "IMAGE ERROR: Freewrite_image : "
      << "unsupported number of samples (" << channels
      << ") for FreeImage\n";
      return false;
      }
    }
  
  FreeImage_DeInitialise();

  return false;
}


/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
bool nearlyEqual(double a, double b, double epsilon){
  const double absA = fabs(a);
  const double absB = fabs(b);
  const double diff = fabs(a - b);
  
  if (a == b) { // shortcut
    return true;
  } else if (a * b == 0) { // a or b or both are zero
    // relative error is not meaningful here
    return diff < (epsilon * epsilon);
  } else { // use relative error
    return diff / (absA + absB) < epsilon;
  }
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
double raySphereIntersection(vec4 p0, vec4 V, vec4 O=vec4(0.0, 0.0, 0.0, 1.0), double r=1.0){
  double t = -1.0;
  double a = 1.0;
  double b = dot(2*V, p0-O);
  double c = (length(p0-O)*length(p0-O)) - (r*r);
  
  double temp = b*b - (4*a*c);
  if(temp < 0.0){ return t;}
  
  if(nearlyEqual(temp, 0.0, 1e-7)){
    return (-b)/(2*a);
  }
  
  double t1 = (-b+sqrt(temp))/(2*a);
  double t2 = (-b-sqrt(temp))/(2*a);
  return (t1 < t2)? t1 : t2;
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
std::vector < vec4 > findRay(GLdouble x, GLdouble y){
  
  y = window_height-y;
  
  GLdouble modelViewMatrix[16];
  GLdouble projectionMatrix[16];
  int viewport[4];
  
  glGetIntegerv(GL_VIEWPORT, viewport);
  
  for(unsigned int i=0; i < 4; i++){
    for(unsigned int j=0; j < 4; j++){
      modelViewMatrix[j*4+i] =  model_view[i][j];
      projectionMatrix[j*4+i] =  projection[i][j];
    }
  }
  
  
  GLdouble nearPlaneLocation[3];
  gluUnProject(x, y, 0.0, modelViewMatrix, projectionMatrix,
               viewport, &nearPlaneLocation[0], &nearPlaneLocation[1],
               &nearPlaneLocation[2]);
  
  GLdouble farPlaneLocation[3];
  gluUnProject(x, y, 1.0, modelViewMatrix, projectionMatrix,
               viewport, &farPlaneLocation[0], &farPlaneLocation[1],
               &farPlaneLocation[2]);
  
  vec4 ray_origin = vec4(nearPlaneLocation[0], nearPlaneLocation[1], nearPlaneLocation[2], 1.0);
  vec3 temp = vec3(farPlaneLocation[0]-nearPlaneLocation[0],
                   farPlaneLocation[1]-nearPlaneLocation[1],
                   farPlaneLocation[2]-nearPlaneLocation[2]);
  temp = normalize(temp);
  vec4 ray_dir = vec4(temp.x, temp.y, temp.z, 0.0);
  
  std::vector < vec4 > result(2);
  result[0] = ray_origin;
  result[1] = ray_dir;
  return result;
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
void castRayDebug(vec4 p0, vec4 dir){
  double t = raySphereIntersection(p0, dir);

  if(t > 0){
    vec4 temp = p0+t*dir;
    vec3 temp_3 = vec3(temp.x, temp.y, temp.z);
    std::cout << p0+t*dir << "\t\t" << length(temp_3) << "\n";
  }

}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
vec4 castRay(vec4 p0, vec4 dir){
  vec4 color;
  
  double t = raySphereIntersection(p0, dir);
  
  if(t < 0){
    color = vec4(0.0,0.0,0.0,1.0);
  }else{
    vec4 temp   = p0+t*dir;
    color = vec4(fabs(temp.z),0.0,0.0,1.0);
  }
  
  return color;
  
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
void rayTrace(){
  
  unsigned char *buffer = new unsigned char[window_width*window_height*4];

  for(unsigned int i=0; i < window_width; i++){
    for(unsigned int j=0; j < window_height; j++){

      int idx = j*window_width+i;
      std::vector < vec4 > ray_o_dir = findRay(i,j);
      vec4 color = castRay(ray_o_dir[0], ray_o_dir[1]);
      buffer[4*idx]   = color.x*255;
      buffer[4*idx+1] = color.y*255;
      buffer[4*idx+2] = color.z*255;
      buffer[4*idx+3] = color.w*255;
    }
  }
  
  write_image("output.png", buffer, window_width, window_height, 4);
  
  delete[] buffer;
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
void init(){
  mesh.makeSubdivisionSphere(8);
  
  
  // Create a vertex array object
  GLuint vao;
  glGenVertexArraysAPPLE( 1, &vao );
  glBindVertexArrayAPPLE( vao );
  
  // Create and initialize a buffer object
  glGenBuffers( 1, &buffer_object);
  
  glBindBuffer( GL_ARRAY_BUFFER, buffer_object );
  unsigned int vertices_bytes = mesh.vertices.size()*sizeof(vec4);
  unsigned int normals_bytes  = mesh.normals.size()*sizeof(vec3);
  unsigned int uv_bytes =  mesh.uvs.size()*sizeof(vec2);
  
  glBufferData( GL_ARRAY_BUFFER, vertices_bytes + normals_bytes + uv_bytes, NULL, GL_STATIC_DRAW );
  unsigned int offset = 0;
  glBufferSubData( GL_ARRAY_BUFFER, offset, vertices_bytes, &mesh.vertices[0] );
  offset += vertices_bytes;
  glBufferSubData( GL_ARRAY_BUFFER, offset, normals_bytes,  &mesh.normals[0] );
  offset += normals_bytes;
  glBufferSubData( GL_ARRAY_BUFFER, offset, uv_bytes,  &mesh.uvs[0] );

  std::string vshader = source_path + "/shaders/vshading_example.glsl";
  std::string fshader = source_path + "/shaders/fshading_example.glsl";
  
  program = InitShader(vshader.c_str(), fshader.c_str());
  // Load shaders and use the resulting shader program
  glUseProgram( program );
  
  // set up vertex arrays
  vPosition = glGetAttribLocation( program, "vPosition" );
  glEnableVertexAttribArray( vPosition );
  
  vNormal = glGetAttribLocation( program, "vNormal" );
  glEnableVertexAttribArray( vNormal );
  
  vTexCoord = glGetAttribLocation( program, "vTexCoord" );
  glEnableVertexAttribArray( vTexCoord );

  
  // Initialize shader lighting parameters
  point4 light_position( 0.0, 0.0, 10.0, 1.0 );
  
  color4 light_ambient(  1.0, 1.0, 1.0, 1.0 );
  color4 light_diffuse(  1.0, 1.0, 1.0, 1.0 );
  color4 light_specular( 1.0, 1.0, 1.0, 1.0 ); 
  
  color4 material_ambient( 0.0, 0.0, 0.0, 1.0 );
  color4 material_diffuse( 1.0, 0.8, 0.0, 1.0 );
  color4 material_specular( 0.0, 0.0, 0.0, 1.0 );
  float  material_shininess = 1;
  
  
  color4 ambient_product  = light_ambient * material_ambient;
  color4 diffuse_product  = light_diffuse * material_diffuse;
  color4 specular_product = light_specular * material_specular;
  
  glUniform4fv( glGetUniformLocation(program, "AmbientProduct"), 1, ambient_product );
  glUniform4fv( glGetUniformLocation(program, "DiffuseProduct"), 1, diffuse_product );
  glUniform4fv( glGetUniformLocation(program, "SpecularProduct"), 1, specular_product );
  glUniform4fv( glGetUniformLocation(program, "LightPosition"), 1, light_position );
  glUniform1f(  glGetUniformLocation(program, "Shininess"), material_shininess );
  
  // Retrieve transformation uniform variable locations
  ModelViewEarth = glGetUniformLocation( program, "ModelViewEarth" );
  ModelViewLight = glGetUniformLocation( program, "ModelViewLight" );
  NormalMatrix = glGetUniformLocation( program, "NormalMatrix" );
  Projection = glGetUniformLocation( program, "Projection" );
  
  glUniform1i( glGetUniformLocation(program, "texture0"), 0 );
  glUniform1i( glGetUniformLocation(program, "texture1"), 1 );
  
  glEnable( GL_DEPTH_TEST );
  
  glShadeModel(GL_SMOOTH);

  glClearColor( 0.8, 0.8, 1.0, 1.0 );
  
  scaling  = 0;
  moving   = 0;
  panning  = 0;
  beginx   = 0;
  beginy   = 0;
  
  matident(curmat);
  trackball(curquat , 0.0f, 0.0f, 0.0f, 0.0f);
  trackball(lastquat, 0.0f, 0.0f, 0.0f, 0.0f);
  add_quats(lastquat, curquat, curquat);
  build_rotmatrix(curmat, curquat);
  
  scalefactor = 1.0;
  render_line = false;
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
void display( void ){
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  
  //  Generate tha model-view matrixn
  
  glUseProgram( program );
  
  mat4 track_ball =  mat4(curmat[0][0], curmat[1][0], curmat[2][0], curmat[3][0],
                          curmat[0][1], curmat[1][1], curmat[2][1], curmat[3][1],
                          curmat[0][2], curmat[1][2], curmat[2][2], curmat[3][2],
                          curmat[0][3], curmat[1][3], curmat[2][3], curmat[3][3]);
  
  
  vec4 cam_position = vec4( 0.0, 0.0, 3.0, 1.0 );
  
  model_view = Translate( -cam_position )*              //Move Camera Back
               Translate(ortho_x, ortho_y, 0.0) *      //Pan Camera
               track_ball *                            //Rotate Camera
               Scale(scalefactor,scalefactor,scalefactor);  //Scale
  
  
  glBindBuffer( GL_ARRAY_BUFFER, buffer_object );
  glVertexAttribPointer( vPosition, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0) );
  glVertexAttribPointer( vNormal, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(mesh.vertices.size()*sizeof(vec4)) );
  glVertexAttribPointer( vTexCoord, 2, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(mesh.vertices.size()*sizeof(vec4)+ mesh.normals.size()*sizeof(vec3)) );

  glUniformMatrix4fv( ModelViewEarth, 1, GL_TRUE, model_view);
  glUniformMatrix4fv( ModelViewLight, 1, GL_TRUE, model_view);

  glUniformMatrix4fv( NormalMatrix, 1, GL_TRUE, transpose(invert(model_view)) );

  glPointSize(5);
  glDrawArrays( GL_TRIANGLES, 0, mesh.vertices.size() );

  
  glutSwapBuffers();
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
void mouse(GLint button, GLint state, GLint x, GLint y){
  
  if (state==GLUT_UP){
    moving=scaling=panning=0;
    std::cout << x << "\t\t" << y << "\n";
    std::vector < vec4 > ray = findRay(x, y);
    castRayDebug(ray[0], ray[1]);
    glutPostRedisplay();
    return;
  }
  
  if( glutGetModifiers() & GLUT_ACTIVE_SHIFT){
    scaling=1;
  }else if( glutGetModifiers() & GLUT_ACTIVE_ALT ){
    panning=1;
  }else{
    moving=1;
    trackball(lastquat, 0, 0, 0, 0);
  }
  
  beginx = x; beginy = y;
  glutPostRedisplay();
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
void motion(GLint x, GLint y){
  
  int W=glutGet(GLUT_WINDOW_WIDTH );
  int H=glutGet(GLUT_WINDOW_HEIGHT);
  
  float dx=(beginx-x)/(float)W;
  float dy=(y-beginy)/(float)H;
  
  if (panning)
    {
    ortho_x  +=dx;
    ortho_y  +=dy;
    
    beginx = x; beginy = y;
    glutPostRedisplay();
    return;
    }
  else if (scaling)
    {
    scalefactor *= (1.0f+dx);
    
    beginx = x;beginy = y;
    glutPostRedisplay();
    return;
    }
  else if (moving)
    {
    trackball(lastquat,
              (2.0f * beginx - W) / W,
              (H - 2.0f * beginy) / H,
              (2.0f * x - W) / W,
              (H - 2.0f * y) / H
              );
    
    add_quats(lastquat, curquat, curquat);
    build_rotmatrix(curmat, curquat);
    
    beginx = x;beginy = y;
    glutPostRedisplay();
    return;
    }
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
void keyboard( unsigned char key, int x, int y ){
  switch( key ) {
    case 033: // Escape Key
    case 'q': case 'Q':
      exit( EXIT_SUCCESS );
      break;
    case ' ':
      render_line = !render_line;
      if(render_line){
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
      }else{
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
      }
     break;
    case 'r':
      rayTrace();
    break;
  }
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
void reshape( int width, int height ){
  window_height = height;
  window_width  = width;
  
  glViewport( 0, 0, width, height );
  
  GLfloat aspect = GLfloat(width)/height;
  projection = Perspective( 45.0, aspect, 1.0, 5.0 );
  
  glUniformMatrix4fv( Projection, 1, GL_TRUE, projection );
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
void timer(int value){
  glutTimerFunc(33,timer,1);
  glutPostRedisplay();
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
int main( int argc, char **argv ){
  glutInit( &argc, argv );
  glutInitDisplayMode( GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH );
  glutInitWindowSize( 512, 512 );
  glutCreateWindow( "Raytracer" );
  init();
  glutDisplayFunc( display );
  glutKeyboardFunc( keyboard );
  glutReshapeFunc( reshape );
  glutMouseFunc( mouse );
  glutMotionFunc( motion );
  
  timer(1);
  
  glutMainLoop();
  return 0;
}
