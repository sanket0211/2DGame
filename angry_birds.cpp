#include <iostream>
#include <cmath>
#include <fstream>
#include <vector>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <time.h>       /* time */

using namespace std;

struct VAO {
    GLuint VertexArrayID;
    GLuint VertexBuffer;
    GLuint ColorBuffer;

    GLenum PrimitiveMode;
    GLenum FillMode;
    int NumVertices;
};
typedef struct VAO VAO;

struct GLMatrices {
	glm::mat4 projection;
	glm::mat4 model;
	glm::mat4 view;
	GLuint MatrixID;
} Matrices;

GLuint programID;

struct Rectangles{
	float length;
	float breadth;
	float x;
	float y;
	float rotation;
	int flag;
	VAO *vao;
	int numflag;
};
typedef struct Rectangles Rectangles;

vector <Rectangles> rectangles;
vector <Rectangles> numbers;
vector <Rectangles> numberstens;
vector <Rectangles> enemynumbers;
vector <Rectangles> enemynumberstens;
vector <Rectangles> countdown;
vector <Rectangles> countdowntens;
vector <Rectangles> obstacles;
int boolean[10];

struct Circles{
	float radius;
	float x;
	float y;
	float rotation;
	int flag;
	VAO *vao;
	int dir;
	float c1,c2,c3;
	float vel ;	
	float time;
};
typedef struct Circles Circles;

vector <Circles> circles;

struct Triangles{
	float x;
	float y;
	float rotation;
	int flag;
	VAO *vao;
	int numflag;
	float radius;
	int time;
};
typedef struct Triangles Triangles;

std::vector<Triangles> triangles;

/* Function to load Shaders - Use it as it is */
GLuint LoadShaders(const char * vertex_file_path,const char * fragment_file_path) {

	// Create the shaders
	GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
	GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

	// Read the Vertex Shader code from the file
	std::string VertexShaderCode;
	std::ifstream VertexShaderStream(vertex_file_path, std::ios::in);
	if(VertexShaderStream.is_open())
	{
		std::string Line = "";
		while(getline(VertexShaderStream, Line))
			VertexShaderCode += "\n" + Line;
		VertexShaderStream.close();
	}

	// Read the Fragment Shader code from the file
	std::string FragmentShaderCode;
	std::ifstream FragmentShaderStream(fragment_file_path, std::ios::in);
	if(FragmentShaderStream.is_open()){
		std::string Line = "";
		while(getline(FragmentShaderStream, Line))
			FragmentShaderCode += "\n" + Line;
		FragmentShaderStream.close();
	}

	GLint Result = GL_FALSE;
	int InfoLogLength;

	// Compile Vertex Shader
	printf("Compiling shader : %s\n", vertex_file_path);
	char const * VertexSourcePointer = VertexShaderCode.c_str();
	glShaderSource(VertexShaderID, 1, &VertexSourcePointer , NULL);
	glCompileShader(VertexShaderID);

	// Check Vertex Shader
	glGetShaderiv(VertexShaderID, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(VertexShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	std::vector<char> VertexShaderErrorMessage(InfoLogLength);
	glGetShaderInfoLog(VertexShaderID, InfoLogLength, NULL, &VertexShaderErrorMessage[0]);
	fprintf(stdout, "%s\n", &VertexShaderErrorMessage[0]);

	// Compile Fragment Shader
	printf("Compiling shader : %s\n", fragment_file_path);
	char const * FragmentSourcePointer = FragmentShaderCode.c_str();
	glShaderSource(FragmentShaderID, 1, &FragmentSourcePointer , NULL);
	glCompileShader(FragmentShaderID);

	// Check Fragment Shader
	glGetShaderiv(FragmentShaderID, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(FragmentShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	std::vector<char> FragmentShaderErrorMessage(InfoLogLength);
	glGetShaderInfoLog(FragmentShaderID, InfoLogLength, NULL, &FragmentShaderErrorMessage[0]);
	fprintf(stdout, "%s\n", &FragmentShaderErrorMessage[0]);

	// Link the program
	fprintf(stdout, "Linking program\n");
	GLuint ProgramID = glCreateProgram();
	glAttachShader(ProgramID, VertexShaderID);
	glAttachShader(ProgramID, FragmentShaderID);
	glLinkProgram(ProgramID);

	// Check the program
	glGetProgramiv(ProgramID, GL_LINK_STATUS, &Result);
	glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	std::vector<char> ProgramErrorMessage( max(InfoLogLength, int(1)) );
	glGetProgramInfoLog(ProgramID, InfoLogLength, NULL, &ProgramErrorMessage[0]);
	fprintf(stdout, "%s\n", &ProgramErrorMessage[0]);

	glDeleteShader(VertexShaderID);
	glDeleteShader(FragmentShaderID);

	return ProgramID;
}
/*static functions*/
static void error_callback(int error, const char* description)
{
    fprintf(stderr, "Error: %s\n", description);
}

void quit(GLFWwindow *window)
{
    glfwDestroyWindow(window);
    glfwTerminate();
    exit(EXIT_SUCCESS);
}


/* Generate VAO, VBOs and return VAO handle */
struct VAO* create3DObject (GLenum primitive_mode, int numVertices, const GLfloat* vertex_buffer_data, const GLfloat* color_buffer_data, GLenum fill_mode=GL_FILL)
{
    struct VAO* vao = new struct VAO;
    vao->PrimitiveMode = primitive_mode;
    vao->NumVertices = numVertices;
    vao->FillMode = fill_mode;

    // Create Vertex Array Object
    // Should be done after CreateWindow and before any other GL calls
    glGenVertexArrays(1, &(vao->VertexArrayID)); // VAO
    glGenBuffers (1, &(vao->VertexBuffer)); // VBO - vertices
    glGenBuffers (1, &(vao->ColorBuffer));  // VBO - colors

    glBindVertexArray (vao->VertexArrayID); // Bind the VAO 
    glBindBuffer (GL_ARRAY_BUFFER, vao->VertexBuffer); // Bind the VBO vertices 
    glBufferData (GL_ARRAY_BUFFER, 3*numVertices*sizeof(GLfloat), vertex_buffer_data, GL_STATIC_DRAW); // Copy the vertices into VBO
    glVertexAttribPointer(
                          0,                  // attribute 0. Vertices
                          3,                  // size (x,y,z)
                          GL_FLOAT,           // type
                          GL_FALSE,           // normalized?
                          0,                  // stride
                          (void*)0            // array buffer offset
                          );

    glBindBuffer (GL_ARRAY_BUFFER, vao->ColorBuffer); // Bind the VBO colors 
    glBufferData (GL_ARRAY_BUFFER, 3*numVertices*sizeof(GLfloat), color_buffer_data, GL_STATIC_DRAW);  // Copy the vertex colors
    glVertexAttribPointer(
                          1,                  // attribute 1. Color
                          3,                  // size (r,g,b)
                          GL_FLOAT,           // type
                          GL_FALSE,           // normalized?
                          0,                  // stride
                          (void*)0            // array buffer offset
                          );

    return vao;
}

/* Generate VAO, VBOs and return VAO handle - Common Color for all vertices */
struct VAO* create3DObject (GLenum primitive_mode, int numVertices, const GLfloat* vertex_buffer_data, const GLfloat red, const GLfloat green, const GLfloat blue, GLenum fill_mode=GL_FILL)
{
	//GL_line for triangle
    GLfloat* color_buffer_data = new GLfloat [3*numVertices];
    for (int i=0; i<numVertices; i++) {
        color_buffer_data [3*i] = red;
        color_buffer_data [3*i + 1] = green;
        color_buffer_data [3*i + 2] = blue;
    }

    return create3DObject(primitive_mode, numVertices, vertex_buffer_data, color_buffer_data, fill_mode);
}

/* Render the VBOs handled by VAO */
void draw3DObject (struct VAO* vao)
{
    // Change the Fill Mode for this object
    glPolygonMode (GL_FRONT_AND_BACK, vao->FillMode);	

    // Bind the VAO to use
    glBindVertexArray (vao->VertexArrayID);

    // Enable Vertex Attribute 0 - 3d Vertices
    glEnableVertexAttribArray(0);
    // Bind the VBO to use
    glBindBuffer(GL_ARRAY_BUFFER, vao->VertexBuffer);

    // Enable Vertex Attribute 1 - Color
    glEnableVertexAttribArray(1);
    // Bind the VBO to use
    glBindBuffer(GL_ARRAY_BUFFER, vao->ColorBuffer);

    // Draw the geometry !
    glDrawArrays(vao->PrimitiveMode, 0, vao->NumVertices); // Starting from vertex 0; 3 vertices total -> 1 triangle
}

/**************************
 * Customizable functions *
 **************************/

 int COUNTDOWN = 30;
 int SITECHANGE = 0;
 float zoom=1;
 double xpos, ypos;

float triangle_rot_dir = 1;
float rectangle_rot_dir = 1;
bool triangle_rot_status = true;
bool rectangle_rot_status = true;
float camera_rotation_angle = 90;
float rectangle_rotation = 10;
float triangle_rotation = 0;
float sx = 0;
float sy = 0;
float vx = 0;
float vy = 0;
int shootflag=0;
float velocity = .5;
float theta;
float bulletx=-9;
float bullety=-2;
float canonbasex=-9;
float canonbasey=-2;
float canonrectx=-9;
float canonrecty=-2;
int score=0;
float i=0;
int speedflag=0;
int upflag=0;
int downflag=0;
int dirupflag=0;
int dirdownflag=0;

float enemybulletx=9;
float enemybullety=-2;
float enemycanonbasex=9;
float enemycanonbasey=-2;
float enemycanonrectx=9;
float enemycanonrecty=-2;
float enemyrectangle_rotation = 100;
int enemyrotdir = 0;
float enemyvx = 1;
float enemyvy = 1;
int enemyshootflag=0;
float enemysx = 0;
float enemysy = 0;
float enemyvelocity = 2.5;
int enemytranslationdir=0;
int enemyscore=0;
float enemyi=0;
int enemyspeedflag=0;
int enemyupflag=0;
int enemydownflag=0;
int enemydirupflag=0;
int enemydirdownflag=0;
/* Executed when a regular key is pressed/released/held-down */
/* Prefered for Keyboard events */
//to be modified for the assignment
void keyboard (GLFWwindow* window, int key, int scancode, int action, int mods)
{
     // Function is called first on GLFW_PRESS.
	//No diff btw samll and caps
	// to diff btw them then test the mods var
    if (action == GLFW_RELEASE) {
        switch (key) {

            case GLFW_KEY_SPACE:
            	shootflag=1;
            	theta = rectangle_rotation*M_PI/180.0f;

            	vx =velocity*cos(theta);
  				vy =velocity*sin(theta);  	
  				speedflag=0;

               // do something ..
                break;

            case GLFW_KEY_W:
	        	upflag=0;
	        	break;

	       	case GLFW_KEY_S:
	        	downflag=0;
	        	break;

	       	case GLFW_KEY_A:
	        	dirupflag=0;
	        	break;
	        case GLFW_KEY_D:
	        	dirdownflag=0;
	        	break;

	        case GLFW_KEY_P:
            	enemyshootflag=1;
            	theta = enemyrectangle_rotation*M_PI/180.0f;

            	enemyvx =enemyvelocity*cos(theta);
  				enemyvy =enemyvelocity*sin(theta);  	
  				enemyspeedflag=0;

               // do something ..
                break;

            case GLFW_KEY_UP:
	        	enemyupflag=0;
	        	break;

	       	case GLFW_KEY_DOWN:
	        	enemydownflag=0;
	        	break;

	       	case GLFW_KEY_LEFT:
	        	enemydirupflag=0;
	        	break;
	        case GLFW_KEY_RIGHT:
	        	enemydirdownflag=0;
	        	break;

            

            default:
                break;
        }
    }
    else if (action == GLFW_PRESS) {
        switch (key) {
            case GLFW_KEY_ESCAPE:
                quit(window);
                break;
            case GLFW_KEY_A:
	        	dirupflag=1;
	        	break;
	        case GLFW_KEY_D:
	        	dirdownflag=1;
	        	break;

	        case GLFW_KEY_SPACE:
	        	if(vx!=0){
  					bulletx=canonbasex;
  					bullety=canonbasey;
  					vx=0;shootflag=0;velocity=0;
  					vy=0;
  					i=0;
  				}
  				speedflag=1;

            	
            	break;	

            case GLFW_KEY_W:

            	upflag=1;
            	
                // do something ..
                break;

            case GLFW_KEY_S:

            	downflag=1;
                // do something ..
                break;

            case GLFW_KEY_LEFT:
	        	enemydirupflag=1;
	        	break;
	        case GLFW_KEY_RIGHT:
	        	enemydirdownflag=1;
	        	break;

	        case GLFW_KEY_P:
	        	if(enemyvx!=0){
  					enemybulletx=enemycanonbasex;
  					enemybullety=enemycanonbasey;
  					enemyvx=0;enemyshootflag=0;enemyvelocity=0;
  					enemyvy=0;
  					enemyi=0;
  				}
  				enemyspeedflag=1;

            	
            	break;	

            case GLFW_KEY_UP:

            	enemyupflag=1;
            	
                // do something ..
                break;

            case GLFW_KEY_Z:

              zoom=zoom*.8;
              
                // do something ..
                break;

            case GLFW_KEY_X:

              zoom=zoom*1.2;
              
                // do something ..
                break;

            case GLFW_KEY_DOWN:

            	enemydownflag=1;
                // do something ..
                break;
    

            default:
                break;
        }
    }
}

/* Executed for character input (like in text boxes) */
//ignore for rite now
void keyboardChar (GLFWwindow* window, unsigned int key)
{
	switch (key) {
		case 'Q':
		case 'q':
            quit(window);
            break;
		default:
			break;
	}
}

/* Executed when a mouse button is pressed/released */
void mouseButton (GLFWwindow* window, int button, int action, int mods)
{
    switch (button) {
        case GLFW_MOUSE_BUTTON_LEFT:
            if (action == GLFW_RELEASE  ){
                  enemyshootflag=1;
              theta = enemyrectangle_rotation*M_PI/180.0f;

              enemyvx =enemyvelocity*cos(theta);
          enemyvy =enemyvelocity*sin(theta);    
          enemyspeedflag=0;
            }


            if (action == GLFW_PRESS){
                  if(enemyvx!=0){
                enemybulletx=enemycanonbasex;
                enemybullety=enemycanonbasey;
                enemyvx=0;enemyshootflag=0;enemyvelocity=0;
                enemyvy=0;
                enemyi=0;
              }
              enemyspeedflag=1;
            }
                
            break;
        case GLFW_MOUSE_BUTTON_RIGHT:
            if (action == GLFW_RELEASE) {
                rectangle_rot_dir *= -1;
            }
            break;
        default:
            break;
    }
}


/* Executed when window is resized to 'width' and 'height' */
/* Modify the bounds of the screen here in glm::ortho or Field of View in glm::Perspective */
void reshapeWindow (GLFWwindow* window, int width, int height)
{
    int fbwidth=width, fbheight=height;
    /* With Retina display on Mac OS X, GLFW's FramebufferSize
     is different from WindowSize */
    glfwGetFramebufferSize(window, &fbwidth, &fbheight);

	GLfloat fov = 90.0f;

	// sets the viewport of openGL renderer
	glViewport (0, 0, (GLsizei) fbwidth, (GLsizei) fbheight);

	// set the projection matrix as perspective
	/* glMatrixMode (GL_PROJECTION);
	   glLoadIdentity ();
	   gluPerspective (fov, (GLfloat) fbwidth / (GLfloat) fbheight, 0.1, 500.0); */
	// Store the projection matrix in a variable for future use
    // Perspective projection for 3D views
    // Matrices.projection = glm::perspective (fov, (GLfloat) fbwidth / (GLfloat) fbheight, 0.1f, 500.0f);

    // Ortho projection for 2D views
    Matrices.projection = glm::ortho(-10.0f*zoom/*(left)*/, 10.0f*zoom, -4.0f*zoom, 4.0f*zoom, 0.1f/*depth*/, 500.0f);
}

VAO *triangle, *rectangle,*canonbase, *circle,objects[100],*obstructions[100],*enemyrectangle,*enemycanonbase,*enemycircle;


// Creates the triangle object used in this sample code
void createTriangle ()
{
  /* ONLY vertices between the bounds specified in glm::ortho will be visible on screen */

  /* Define vertex array as used in glBegin (GL_TRIANGLES) */
  static const GLfloat vertex_buffer_data [] = {
    0, 1,0, // vertex 0
    -1,-1,0, // vertex 1
    1,-1,0, // vertex 2
  };

  static const GLfloat color_buffer_data [] = {
    1,0,0, // color 0
    0,1,0, // color 1
    0,0,1, // color 2
  };

  // create3DObject creates and returns a handle to a VAO that can be used later
  triangle = create3DObject(GL_TRIANGLES, 3, vertex_buffer_data, color_buffer_data, GL_LINE);
}

void createCircle(float radius)
{
  int i=0,j=0;
  GLfloat vertex_buffer_data [360];
  for(i=0;i<36;i++){
    float theta = (2.0f*3.14f*float(i))/float(36);
    vertex_buffer_data[j]=0;j++;
    vertex_buffer_data[j]=0;j++;
    vertex_buffer_data[j]=0;j++;
    
    vertex_buffer_data[j]=radius * cosf(theta);j++;
    vertex_buffer_data[j]=radius* sinf(theta);j++;
    vertex_buffer_data[j]=0;j++;
    theta = (2.0f*3.14f*float(i+1))/float(36);
    
    vertex_buffer_data[j]=radius* cosf(theta);j++;
    vertex_buffer_data[j]=radius* sinf(theta);j++;
    vertex_buffer_data[j]=0;j++;
  }

  GLfloat color_buffer_data [324];
  for(int i=0;i<324;i++){
    color_buffer_data[i]=1;
  }

  // create3DObject creates and returns a handle to a VAO that can be used later
  circle = create3DObject(GL_TRIANGLES, 108, vertex_buffer_data, color_buffer_data, GL_FILL);
}

void createEnemyCircle(float radius)
{
  int i=0,j=0;
  GLfloat vertex_buffer_data [360];
  for(i=0;i<36;i++){
    float theta = (2.0f*3.14f*float(i))/float(36);
    vertex_buffer_data[j]=0;j++;
    vertex_buffer_data[j]=0;j++;
    vertex_buffer_data[j]=0;j++;
    
    vertex_buffer_data[j]=radius * cosf(theta);j++;
    vertex_buffer_data[j]=radius* sinf(theta);j++;
    vertex_buffer_data[j]=0;j++;
    theta = (2.0f*3.14f*float(i+1))/float(36);
    
    vertex_buffer_data[j]=radius* cosf(theta);j++;
    vertex_buffer_data[j]=radius* sinf(theta);j++;
    vertex_buffer_data[j]=0;j++;
  }

  GLfloat color_buffer_data [324];
  for(int i=0;i<324;i++){
    color_buffer_data[i]=1;
  }

  // create3DObject creates and returns a handle to a VAO that can be used later
  enemycircle = create3DObject(GL_TRIANGLES, 108, vertex_buffer_data, color_buffer_data, GL_FILL);
}

void createobstructions(float length, float breadth,int index,float c1, float c2, float c3){
	static const GLfloat vertex_buffer_data [] = {
    -(length/2),-(breadth/2),0, // vertex 1
    (length/2),-(breadth/2),0, // vertex 2
    (length/2), (breadth/2),0, // vertex 3

    (length/2),(breadth/2),0, // vertex 3
    -(length/2),(breadth/2),0, // vertex 4
    -(length/2),-(breadth/2),0  // vertex 1
  };

  static const GLfloat color_buffer_data [] = {
    c1,c2,c3, // color 1
    c1,c2,c3, // color 2
    c1,c2,c3, // color 3

    c1,c2,c3, // color 3
    c1,c2,c3, // color 4
    c1,c2,c3 // color 1
  };
  obstructions[index] = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_FILL);
  
}

VAO* createRectangles(float x,float y, float length, float breadth, float c1, float c2, float c3 ){
	GLfloat vertex_buffer_data [] = {
    x-(length/2),y-(breadth/2),0,
    x+(length/2), y-(breadth/2),0,
    x+(length/2),y+(breadth/2),0,

    x+(length/2),y+(breadth/2),0,
    x-(length/2),y+(breadth/2),0,
    x-(length/2),y-(breadth/2),0
  };
  //cout << x-(length/2) << endl;

  GLfloat color_buffer_data [] = {
    c1,c2,c3, // color 1
    c1,c2,c3, // color 2
    c1,c2,c3, // color 3

    c1,c2,c3, // color 3
    c1,c2,c3, // color 4
    c1,c2,c3 // color 1
  };
  return create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_FILL);
}


VAO* createTriangles(float x,float y, float radius,float c1, float c2, float c3 ){
	GLfloat vertex_buffer_data [] = {
    x,y+radius,0,
    x-(1.732*radius/2), y-(radius/2),0,
    x+(1.732*radius/2), y-(radius/2),0,
    x,y-radius,0,
    x-(1.732*radius/2), y+(radius/2),0,
    x+(1.732*radius/2), y+(radius/2),0

  };


  GLfloat color_buffer_data [] = {
    c1,c2,c3, // color 1
    c1,c2,c3, // color 2
    c1,c2,c3, // color 3
    c1,c2,c3, // color 1
    c1,c2,c3, // color 2
    c1,c2,c3, // color 3

  };
  return create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_FILL);
}

VAO* createRhombus(float x,float y, float radius,float c1, float c2, float c3 ){
	GLfloat vertex_buffer_data [] = {
    x,y+radius+radius/4,0,
    x-(1.732*radius/2), 0,0,
    x+(1.732*radius/2), 0,0,
    x,y-radius-radius/4,0,
    x-(1.732*radius/2), 0,0,
    x+(1.732*radius/2), 0,0

  };


  GLfloat color_buffer_data [] = {
    c1,c2,c3, // color 1
    c1,c2,c3, // color 2
    c1,c2,c3, // color 3
    c1,c2,c3, // color 1
    c1,c2,c3, // color 2
    c1,c2,c3, // color 3

  };
  return create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_FILL);
}

VAO* createSquare(float x,float y, float radius,float c1, float c2, float c3 ){
	GLfloat vertex_buffer_data [] = {
    x+(radius/1.414),y+(radius/1.414),0,
    x-(radius/1.414),y+(radius/1.414),0,
    x+(radius/1.414),y-(radius/1.414),0,
    x+(radius/1.414),y-(radius/1.414),0,
    x-(radius/1.414),y-(radius/1.414),0,
    x-(radius/1.414),y+(radius/1.414),0

  };


  GLfloat color_buffer_data [] = {
    c1,c2,c3, // color 1
    c1,c2,c3, // color 2
    c1,c2,c3, // color 3
    c1,c2,c3, // color 1
    c1,c2,c3, // color 2
    c1,c2,c3, // color 3

  };
  return create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_FILL);
}

VAO* createCircles(float x,float y,float radius, float c1, float c2, float c3 ){
	int i=0,j=0;
  GLfloat vertex_buffer_data [360];
  for(i=0;i<36;i++){
    float theta = (2.0f*3.14f*float(i))/float(36);
    vertex_buffer_data[j]=x;j++;
    vertex_buffer_data[j]=y;j++;
    vertex_buffer_data[j]=0;j++;
    
    vertex_buffer_data[j]=x+(radius * cosf(theta));j++;
    vertex_buffer_data[j]=y+(radius* sinf(theta));j++;
    vertex_buffer_data[j]=0;j++;
    theta = (2.0f*3.14f*float(i+1))/float(36);
    
    vertex_buffer_data[j]=(x+radius* cosf(theta));j++;
    vertex_buffer_data[j]=(y+radius* sinf(theta));j++;
    vertex_buffer_data[j]=0;j++;
  }
  i=0;
  GLfloat color_buffer_data [324];
  while(i<324){
    color_buffer_data[i]=c1;i++;
    color_buffer_data[i]=c2;i++;
    color_buffer_data[i]=c3;i++;
    color_buffer_data[i]=c1-.3;i++;
    color_buffer_data[i]=c2;i++;
    color_buffer_data[i]=c3;i++;
    color_buffer_data[i]=c1-.3;i++;
    color_buffer_data[i]=c2;i++;
    color_buffer_data[i]=c3;i++;
    
  }
  return create3DObject(GL_TRIANGLES, 108, vertex_buffer_data, color_buffer_data, GL_FILL);
}


VAO* createSemiCircles(float x,float y,float radius, float c1, float c2, float c3 ){
	int i=0,j=0;
  GLfloat vertex_buffer_data [360];
  for(i=18;i<36;i++){
    float theta = (2.0f*3.14f*float(i))/float(36);
    vertex_buffer_data[j]=x;j++;
    vertex_buffer_data[j]=y;j++;
    vertex_buffer_data[j]=0;j++;
    
    vertex_buffer_data[j]=x+(radius * cosf(theta));j++;
    vertex_buffer_data[j]=y+(radius* sinf(theta));j++;
    vertex_buffer_data[j]=0;j++;
    theta = (2.0f*3.14f*float(i+1))/float(36);
    
    vertex_buffer_data[j]=(x+radius* cosf(theta));j++;
    vertex_buffer_data[j]=(y+radius* sinf(theta));j++;
    vertex_buffer_data[j]=0;j++;
  }
  i=0;
  GLfloat color_buffer_data [324];
  while(i<108){
    color_buffer_data[i]=c1;i++;
    color_buffer_data[i]=c2;i++;
    color_buffer_data[i]=c3;i++;
    color_buffer_data[i]=c1-.3;i++;
    color_buffer_data[i]=c2;i++;
    color_buffer_data[i]=c3;i++;
    color_buffer_data[i]=c1-.3;i++;
    color_buffer_data[i]=c2;i++;
    color_buffer_data[i]=c3;i++;
    
  }
  return create3DObject(GL_TRIANGLES, 54, vertex_buffer_data, color_buffer_data, GL_FILL);
}



void createCanonBase()
{
  int i=0,j=0;
  GLfloat vertex_buffer_data [360];
  for(i=0;i<36;i++){
    float theta = (2.0f*3.14f*float(i))/float(36);
    vertex_buffer_data[j]=0;j++;
    vertex_buffer_data[j]=0;j++;
    vertex_buffer_data[j]=0;j++;
    
    vertex_buffer_data[j]=.5 * cosf(theta);j++;
    vertex_buffer_data[j]=.5* sinf(theta);j++;
    vertex_buffer_data[j]=0;j++;
    theta = (2.0f*3.14f*float(i+1))/float(36);
    
    vertex_buffer_data[j]=.5 * cosf(theta);j++;
    vertex_buffer_data[j]=.5* sinf(theta);j++;
    vertex_buffer_data[j]=0;j++;	
  }

  GLfloat color_buffer_data [324];
  for(int i=0;i<324;i++){
    color_buffer_data[i]=0;
  }

  // create3DObject creates and returns a handle to a VAO that can be used later
  canonbase = create3DObject(GL_TRIANGLES, 108, vertex_buffer_data, color_buffer_data, GL_FILL);
}

void createEnemyCanonBase()
{
  int i=0,j=0;
  GLfloat vertex_buffer_data [360];
  for(i=0;i<36;i++){
    float theta = (2.0f*3.14f*float(i))/float(36);
    vertex_buffer_data[j]=0;j++;
    vertex_buffer_data[j]=0;j++;
    vertex_buffer_data[j]=0;j++;
    
    vertex_buffer_data[j]=.5 * cosf(theta);j++;
    vertex_buffer_data[j]=.5* sinf(theta);j++;
    vertex_buffer_data[j]=0;j++;
    theta = (2.0f*3.14f*float(i+1))/float(36);
    
    vertex_buffer_data[j]=.5 * cosf(theta);j++;
    vertex_buffer_data[j]=.5* sinf(theta);j++;
    vertex_buffer_data[j]=0;j++;	
  }

  GLfloat color_buffer_data [324];
  for(int i=0;i<324;i++){
    color_buffer_data[i]=0;
  }

  // create3DObject creates and returns a handle to a VAO that can be used later
  enemycanonbase = create3DObject(GL_TRIANGLES, 108, vertex_buffer_data, color_buffer_data, GL_FILL);
}

// Creates the rectangle object used in this sample code
void createRectangle ()
{
  // GL3 accepts only Triangles. Quads are not supported
  static const GLfloat vertex_buffer_data [] = {
    -1.2,-.3,0, // vertex 1
    1.2,-.3,0, // vertex 2
    1.2, .3,0, // vertex 3

    1.2, .3,0, // vertex 3
    -1.2, .3,0, // vertex 4
    -1.2,-.3,0  // vertex 1
  };

  static const GLfloat color_buffer_data [] = {
    1,0,0, // color 1
    0,0,1, // color 2
    0,1,0, // color 3

    0,1,0, // color 3
    0.3,0.3,0.3, // color 4
    1,0,0  // color 1
  };

  // create3DObject creates and returns a handle to a VAO that can be used later
  rectangle = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_FILL);
}

void createEnemyRectangle ()
{
  // GL3 accepts only Triangles. Quads are not supported
  static const GLfloat vertex_buffer_data [] = {
    -1.2,-.3,0, // vertex 1
    1.2,-.3,0, // vertex 2
    1.2, .3,0, // vertex 3

    1.2, .3,0, // vertex 3
    -1.2, .3,0, // vertex 4
    -1.2,-.3,0  // vertex 1
  };

  static const GLfloat color_buffer_data [] = {
    1,0,0, // color 1
    0,0,1, // color 2
    0,1,0, // color 3

    0,1,0, // color 3
    0.3,0.3,0.3, // color 4
    1,0,0  // color 1
  };

  // create3DObject creates and returns a handle to a VAO that can be used later
  enemyrectangle = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_FILL);
}


/* Render the scene with openGL */
/* Edit this function according to your assignment */
void draw ()
{
  // clear the color and depth in the frame buffer
  glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // use the loaded shader program
  // Don't change unless you know what you are doing
  glUseProgram (programID);

  // Eye - Location of camera. Don't change unless you are sure!!
  glm::vec3 eye ( 5*cos(camera_rotation_angle*M_PI/180.0f), 0, 5*sin(camera_rotation_angle*M_PI/180.0f) );
  // Target - Where is the camera looking at.  Don't change unless you are sure!!
  glm::vec3 target (0, 0, 0);
  // Up - Up vector defines tilt of camera.  Don't change unless you are sure!!
  glm::vec3 up (0, 1, 0);

  // Compute Camera matrix (view)
  // Matrices.view = glm::lookAt( eye, target, up ); // Rotating Camera for 3D
  //  Don't change unless you are sure!!
  Matrices.view = glm::lookAt(glm::vec3(0,0,3), glm::vec3(0,0,0), glm::vec3(0,1,0)); // Fixed camera for 2D (ortho) in XY plane

  // Compute ViewProject matrix as view/camera might not be changed for this frame (basic scenario)
  //  Don't change unless you are sure!!
  glm::mat4 VP = Matrices.projection * Matrices.view;

  // Send our transformation to the currently bound shader, in the "MVP" uniform
  // For each model you render, since the MVP will be different (at least the M part)
  //  Don't change unless you are sure!!
  glm::mat4 MVP;	// MVP = Projection * View * Model

  // draw3DObject draws the VAO given to it using current MVP matrix
  Matrices.model = glm::mat4(1.0f);
  glm::mat4 translateland = glm::translate (glm::vec3(rectangles[0].x,rectangles[0].y-3, 0));	
  //cout << rectangles[2].y << endl;
  Matrices.model *= (translateland);
  MVP = VP * Matrices.model;
  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
  draw3DObject(rectangles[0].vao);


  Matrices.model = glm::mat4(1.0f);
  glm::mat4 translatesky = glm::translate (glm::vec3(rectangles[1].x,rectangles[1].y+1, 0));	
  //cout << rectangles[2].y << endl;
  Matrices.model *= (translatesky);
  MVP = VP * Matrices.model;
  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
  draw3DObject(rectangles[1].vao);


  Matrices.model = glm::mat4(1.0f);
  glm::mat4 translatesea = glm::translate (glm::vec3(rectangles[3].x+20,rectangles[3].y, 0));	
 // cout << rectangles[2].y << endl;
  Matrices.model *= (translatesea);
  MVP = VP * Matrices.model;
  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
  draw3DObject(rectangles[3].vao);


Matrices.model = glm::mat4(1.0f);
  glm::mat4 translatetreebase = glm::translate (glm::vec3(rectangles[2].x+3,rectangles[2].y-1, 0));	
 // cout << rectangles[2].y << endl;
  Matrices.model *= (translatetreebase);
  MVP = VP * Matrices.model;
  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
  draw3DObject(rectangles[2].vao);

  Matrices.model = glm::mat4(1.0f);
  glm::mat4 translatetree = glm::translate (glm::vec3(circles[0].x+3,circles[0].y, 0));	
  //cout << rectangles[2].y << endl;
  Matrices.model *= (translatetree);
  MVP = VP * Matrices.model;
  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
  draw3DObject(circles[0].vao);

  // Pop matrix to undo transformations till last push matrix instead of recomputing model matrix
  // glPopMatrix ();
  Matrices.model = glm::mat4(1.0f);

  glm::mat4 translateiniRectangle = glm::translate (glm::vec3(0,0, 0));	
  glm::mat4 rotateRectangle = glm::rotate((float)(rectangle_rotation*M_PI/180.0f), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)
  glm::mat4 translateRectangle = glm::translate (glm::vec3(canonrectx, canonrecty, 0));        // glTranslatef
  
  Matrices.model *= (translateRectangle*rotateRectangle*translateiniRectangle);
  MVP = VP * Matrices.model;
  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

  // draw3DObject draws the VAO given to it using current MVP matrix
  draw3DObject(rectangle);

  Matrices.model = glm::mat4(1.0f);

  translateiniRectangle = glm::translate (glm::vec3(0,0, 0));	
  rotateRectangle = glm::rotate((float)(enemyrectangle_rotation*M_PI/180.0f), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)
  translateRectangle = glm::translate (glm::vec3(enemycanonrectx, enemycanonrecty, 0));        // glTranslatef
  
  Matrices.model *= (translateRectangle*rotateRectangle*translateiniRectangle);
  MVP = VP * Matrices.model;
  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

  // draw3DObject draws the VAO given to it using current MVP matrix
  draw3DObject(enemyrectangle);

  //for the canonbase

  Matrices.model = glm::mat4(1.0f);

  glm::mat4 translateCanonBase = glm::translate (glm::vec3(canonbasex, canonbasey, 0));        // glTranslatef
  //glm::mat4 rotateCanonBase = glm::rotate((float)(canonbase_rotation*M_PI/180.0f), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)
  Matrices.model *= (translateCanonBase);
  MVP = VP * Matrices.model;
  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

  draw3DObject(canonbase);

  Matrices.model = glm::mat4(1.0f);

  glm::mat4 translateEnemyCanonBase = glm::translate (glm::vec3(enemycanonbasex, enemycanonbasey, 0));        // glTranslatef
  //glm::mat4 rotateCanonBase = glm::rotate((float)(canonbase_rotation*M_PI/180.0f), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)
  Matrices.model *= (translateEnemyCanonBase);
  MVP = VP * Matrices.model;
  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

  draw3DObject(enemycanonbase);

  //for circle
  Matrices.model = glm::mat4(1.0f);

  glm::mat4 translateCircle = glm::translate (glm::vec3(bulletx,bullety, 0));	
  //glm::mat4 rotateCircle = glm::rotate((float)(rectangle_rotation*M_PI/180.0f), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)
  //glm::mat4 translatebackCircle = glm::translate (glm::vec3(-1.5,-3, 0));
  Matrices.model *= (translateCircle);
  MVP = VP * Matrices.model;
  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

  // draw3DObject draws the VAO given to it using current MVP matrix
  draw3DObject(circle);



  Matrices.model = glm::mat4(1.0f);

  glm::mat4 translateEnemyCircle = glm::translate (glm::vec3(enemybulletx,enemybullety, 0));	
  //glm::mat4 rotateCircle = glm::rotate((float)(rectangle_rotation*M_PI/180.0f), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)
  //glm::mat4 translatebackCircle = glm::translate (glm::vec3(-1.5,-3, 0));
  Matrices.model *= (translateEnemyCircle);
  MVP = VP * Matrices.model;
  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

  // draw3DObject draws the VAO given to it using current MVP matrix
  draw3DObject(enemycircle);


  for(int k=1;k<51;k++){
    if(circles[(k)%51].flag==1 ){
    Matrices.model = glm::mat4(1.0f); 
    glm::mat4 translateTargetcir = glm::translate (glm::vec3(circles[k%51].x,circles[k%51].y, 0));
    Matrices.model *= (translateTargetcir);
    MVP = VP * Matrices.model;
    glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
    draw3DObject(circles[k].vao);
  }
  }

  for(int k=0;k<1;k++){
  	if(SITECHANGE==2){
  		Matrices.model = glm::mat4(1.0f); 
	    glm::mat4 translateTargetcir = glm::translate (glm::vec3(obstacles[k].x-3,circles[k].y-2, 0));
	    Matrices.model *= (translateTargetcir);
	    MVP = VP * Matrices.model;
	    glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
	    draw3DObject(obstacles[k].vao);
  	}
  }
  for(int k=1;k<2;k++){
  	if(SITECHANGE==2){
  		Matrices.model = glm::mat4(1.0f); 
	    glm::mat4 translateTargetcir = glm::translate (glm::vec3(obstacles[k].x+3,obstacles[k].y-2, 0));
	    Matrices.model *= (translateTargetcir);
	    MVP = VP * Matrices.model;
	    glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
	    draw3DObject(obstacles[k].vao);
  	}
  }


for(int j=0;j<7;j++){
	if(numbers[j].numflag==1){
	  Matrices.model = glm::mat4(1.0f);
	  MVP = VP * Matrices.model;
	  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
	  draw3DObject(numbers[j].vao);
	}


}

for(int j=0;j<7;j++){
	if(numberstens[j].numflag==1){
	  Matrices.model = glm::mat4(1.0f);
	  MVP = VP * Matrices.model;
	  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
	  draw3DObject(numberstens[j].vao);
	}
}

for(int j=0;j<7;j++){
	if(enemynumbers[j].numflag==1){
	  Matrices.model = glm::mat4(1.0f);
	  MVP = VP * Matrices.model;
	  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
	  draw3DObject(enemynumbers[j].vao);
	}

	
}

for(int j=0;j<7;j++){
	if(enemynumberstens[j].numflag==1){
	  Matrices.model = glm::mat4(1.0f);
	  MVP = VP * Matrices.model;
	  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
	  draw3DObject(enemynumberstens[j].vao);
	}
}

for(int j=0;j<7;j++){
	if(countdown[j].numflag==1){
	  Matrices.model = glm::mat4(1.0f);
	  MVP = VP * Matrices.model;
	  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
	  draw3DObject(countdown[j].vao);
	}

	
}

for(int j=0;j<7;j++){
	if(countdowntens[j].numflag==1){
	  Matrices.model = glm::mat4(1.0f);
	  MVP = VP * Matrices.model;
	  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
	  draw3DObject(countdowntens[j].vao);
	}
}


  

  //for land
  Matrices.model = glm::mat4(1.0f);

  
  // Increment angles
  float increments = 1;

  //camera_rotation_angle++; // Simulating camera rotation
 // triangle_rotation = triangle_rotation + increments*triangle_rot_dir*triangle_rot_status;
  //rectangle_rotation = rectangle_rotation + increments*rectangle_rot_dir*rectangle_rot_status;
}

void shoot()
{
  sx = vx*i;
  sy = vy*i + (-5)*i*i;
  //cout << "sy"<< sy << endl;
  bulletx = bulletx+sx;
  bullety = bullety+sy;
 // cout << "bullety" << bullety<< endl;
  if(bulletx>=10 || bullety<=-4 || bulletx<=-10){
  	i=0;velocity=0;
  	shootflag=0;
  	bulletx=canonbasex;
  	bullety=canonbasey;

  }
  if(bullety<=-3.5){
  	bullety=-3.5;
  	velocity=velocity*.9;
  	i=0.01;
  	//bulletx=3.4;
  	if(theta<0){
  		vy=vy*-1;
  //	cout << vy << endl;
  	}
  }

}

void enemyshoot()
{
  enemysx = enemyvx*enemyi;
  enemysy = enemyvy*enemyi + (-5)*enemyi*enemyi;
  //cout << "sy"<< sy << endl;
  enemybulletx = enemybulletx+enemysx;
  enemybullety = enemybullety+enemysy;
  //cout << "bullety" << bullety<< endl;
  if(enemybulletx>=10 || enemybullety<=-4 || enemybulletx<=-10){
  	enemyi=0;enemyvelocity=1.2;
  	enemyshootflag=0;
  	enemybulletx=enemycanonbasex;
  	enemybullety=enemycanonbasey;

  }
  if(enemybullety<=-3.5){
  	enemybullety=-3.5;
  	enemyvelocity=enemyvelocity*.9;
  	enemyi=0.01;
  	//bulletx=3.4;
  	if(theta>180){
  		enemyvy=enemyvy*-1;
 // 		cout << vy << endl;
  	}
  }

}

void createnumber(int num){
	for(int k=0;k<7;k++){
		boolean[k]=0;
	}
	if(num==1){
		boolean[0]=1;
		boolean[1]=1;
	}
	if(num==2){
		boolean[5]=1;
		boolean[0]=1;
		boolean[6]=1;
		boolean[3]=1;
		boolean[2]=1;

	}
	if(num==3){
		boolean[5]=1;
		boolean[0]=1;
		boolean[6]=1;
		boolean[1]=1;
		boolean[2]=1;

	}
	if(num==4){
		boolean[4]=1;
		boolean[6]=1;
		boolean[0]=1;
		boolean[1]=1;
	}
	if(num==5){
		boolean[5]=1;
		boolean[4]=1;
		boolean[6]=1;
		boolean[1]=1;
		boolean[2]=1;

	}
	if(num==6){
		boolean[5]=1;
		boolean[4]=1;
		boolean[6]=1;
		boolean[1]=1;
		boolean[2]=1;
		boolean[3]=1;
	}
	if(num==7){
		boolean[5]=1;
		boolean[0]=1;
		boolean[1]=1;

	}
	if(num==8){
		boolean[0]=1;
		boolean[1]=1;
		boolean[2]=1;
		boolean[3]=1;
		boolean[4]=1;
		boolean[5]=1;
		boolean[6]=1;

	}
	if(num==9){
		boolean[4]=1;
		boolean[5]=1;
		boolean[6]=1;
		boolean[0]=1;
		boolean[1]=1;
	}
	if(num==0){
		boolean[0]=1;
		boolean[1]=1;
		boolean[2]=1;
		boolean[3]=1;
		boolean[4]=1;
		boolean[5]=1;
	}
}

void drawscore(int score){
	int digit = score%10;
	createnumber(digit);
	for(int k=0;k<7;k++){
		numbers[k].numflag=boolean[k];
	}
	score=score/10;
	digit = score%10;
	createnumber(digit);
	for(int k=0;k<7;k++){
		numberstens[k].numflag=boolean[k];
	}
}

void drawenemyscore(int score){
	int digit = score%10;
	createnumber(digit);
	for(int k=0;k<7;k++){
		enemynumbers[k].numflag=boolean[k];
	}
	score=score/10;
	digit = score%10;
	createnumber(digit);
	for(int k=0;k<7;k++){
		enemynumberstens[k].numflag=boolean[k];
	}
}

void drawenemycountdown(int count){
	int digit = count%10;
	createnumber(digit);
	for(int k=0;k<7;k++){
		countdown[k].numflag=boolean[k];
	}
	count=count/10;
	digit = count%10;
	createnumber(digit);
	for(int k=0;k<7;k++){
		countdowntens[k].numflag=boolean[k];
	}
}


/* Initialise glfw window, I/O callbacks and the renderer to use */
/* Nothing to Edit here */
GLFWwindow* initGLFW (int width, int height)
{
    GLFWwindow* window; // window desciptor/handle

    glfwSetErrorCallback(error_callback);
    if (!glfwInit()) {
        exit(EXIT_FAILURE);
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    window = glfwCreateWindow(width, height, "Sample OpenGL 3.3 Application", NULL, NULL);

    if (!window) {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    glfwMakeContextCurrent(window);
    gladLoadGLLoader((GLADloadproc) glfwGetProcAddress);
    glfwSwapInterval( 1 );

    /* --- register callbacks with GLFW --- */

    /* Register function to handle window resizes */
    /* With Retina display on Mac OS X GLFW's FramebufferSize
     is different from WindowSize */
    glfwSetFramebufferSizeCallback(window, reshapeWindow);
    glfwSetWindowSizeCallback(window, reshapeWindow);

    /* Register function to handle window close */
    glfwSetWindowCloseCallback(window, quit);

    /* Register function to handle keyboard input */
    glfwSetKeyCallback(window, keyboard/*function name*/);      // general keyboard input
    glfwSetCharCallback(window, keyboardChar);  // simpler specific character handling

    /* Register function to handle mouse click */
    glfwSetMouseButtonCallback(window, mouseButton/*function name*/);  // mouse button clicks

    return window;
}


/* Initialize the OpenGL rendering properties */
/* Add all the models to be created here *///object creation
void initGL (GLFWwindow* window, int width, int height)
{
    /* Objects should be created before any other gl function and shaders */
	// Create the models
	//createTriangle (); // Generate the VAO, VBOs, vertices data & copy into the array buffer
	srand (time(NULL));
	createRectangle ();
  createCanonBase();

  createEnemyRectangle ();
  createEnemyCanonBase();
  createEnemyCircle(.2);

  createCircle(.2);
  Rectangles q;
  q.x=0;
  q.y=0;
  q.length=20*zoom;
  q.breadth=2*zoom;
  q.vao = createRectangles(q.x,q.y,q.length,q.breadth,1,.5,0); 
  rectangles.push_back(q);
  q.x=0;
  q.y=0;
  q.length=20*zoom;
  q.breadth=6*zoom;
  q.vao = createRectangles(q.x,q.y,q.length,q.breadth,0.5,1,.5); 
  rectangles.push_back(q);
  q.x=0;
  q.y=0;
  q.length=.5*zoom;
  q.breadth=2*zoom;
  q.rotation=0	;
   q.flag=0;
  q.vao = createRectangles(q.x,q.y,q.length,q.breadth,0.647059 ,0.164706, 0.164706); 
  rectangles.push_back(q);

  q.x=0;
  q.y=0;
  q.length=20*zoom;
  q.breadth=8*zoom;
  q.vao = createRectangles(q.x,q.y,q.length,q.breadth,0.74902,0.847059,0.847059); 
  rectangles.push_back(q);



  Circles c;
  c.x=0;c.y=0;c.radius=1*zoom;
  c.vao = createCircles(c.x,c.y,c.radius,0.137255,0.556863,0.137255);
  circles.push_back(c);
  srand (time(NULL));
  float j=.2;
  for(int k=0;k<50;k++){
    Circles c;
    if(SITECHANGE==0)
    	c.x=0;
    else if(SITECHANGE==2){
    	c.x=0 ;
    }    
    c.y =0;
    c.vel=15;
    c.radius=.5*zoom;
    c.time=0;
    c.dir=1;c.flag=0;
    if(k%5==0){
    	c.c1=0;c.c2=0;c.c3=1;
      	c.vao = createRhombus(c.x,c.y,c.radius,c.c1,c.c2,c.c3);
    }
    else if((k-1)%5==0){
      c.c1=1;c.c2=0;c.c3=0;
      c.vao = createTriangles(c.x,c.y,c.radius,c.c1,c.c2,c.c3);
    }
    else if ((k-2)%5==0){
      c.c1=1;c.c2=2;c.c3=0;
      c.vao = createCircles(c.x,c.y,c.radius,c.c1,c.c2,c.c3);
    }
    else if((k-3)%5==0){
    	c.c1=0;c.c2=1;c.c3=1;
      c.vao = createSquare(c.x,c.y,c.radius,c.c1,c.c2,c.c3);
    }
    else{
    	c.c1=0;c.c2=1;c.c3=0;
      	c.vao = createSemiCircles(c.x,c.y,c.radius,c.c1,c.c2,c.c3);
    }
    circles.push_back(c);
  }

  Triangles t;

  for(int k=0;k<1;k++){
  	t.x=0;t.y=0;t.radius=(1.732/4)*zoom;t.flag=0;
  	t.vao = createTriangles(t.x,t.y,t.radius,0,0,1);
  	triangles.push_back(t);
  }

  	q.length=.05;q.breadth=.2;q.x=-7;q.y=3;q.numflag=1;;q.vao=createRectangles(q.x,q.y,q.length,q.breadth,1,1,1); 
 	numbers.push_back(q);
 	q.length=.05;q.breadth=.2;q.x=numbers[0].x;q.y=numbers[0].y-.2;q.numflag=1;q.vao=createRectangles(q.x,q.y,q.length,q.breadth,1,1,1); 
 	numbers.push_back(q);
 	q.length=.2;q.breadth=.02;q.x=numbers[1].x-.1;q.y=numbers[1].y-.1;q.numflag=1;q.vao=createRectangles(q.x,q.y,q.length,q.breadth,1,1,1); 
 	numbers.push_back(q);
 	q.length=.05;q.breadth=.2;q.x=numbers[2].x-.1;q.y=numbers[1].y;q.numflag=1;q.vao=createRectangles(q.x,q.y,q.length,q.breadth,1,1,1); 
 	numbers.push_back(q);
 	q.length=.05;q.breadth=.2;q.x=numbers[3].x;q.y=numbers[0].y;q.numflag=1;q.vao=createRectangles(q.x,q.y,q.length,q.breadth,1,1,1); 
 	numbers.push_back(q);
 	q.length=.2;q.breadth=.02;q.x=numbers[2].x;q.y=numbers[1].y+.3;q.numflag=1;q.vao=createRectangles(q.x,q.y,q.length,q.breadth,1,1,1); 
 	numbers.push_back(q);
 	q.length=.2;q.breadth=.02;q.x=numbers[2].x;q.y=numbers[1].y+.1;q.numflag=1;q.vao=createRectangles(q.x,q.y,q.length,q.breadth,1,1,1); 
 	numbers.push_back(q);

 	q.length=.05;q.breadth=.2;q.x=-8;q.y=3;q.numflag=1;;q.vao=createRectangles(q.x,q.y,q.length,q.breadth,1,1,1); 
 	numberstens.push_back(q);
 	q.length=.05;q.breadth=.2;q.x=numberstens[0].x;q.y=numberstens[0].y-.2;q.numflag=1;q.vao=createRectangles(q.x,q.y,q.length,q.breadth,1,1,1); 
 	numberstens.push_back(q);
 	q.length=.2;q.breadth=.02;q.x=numberstens[1].x-.1;q.y=numberstens[1].y-.1;q.numflag=1;q.vao=createRectangles(q.x,q.y,q.length,q.breadth,1,1,1); 
 	numberstens.push_back(q);
 	q.length=.05;q.breadth=.2;q.x=numberstens[2].x-.1;q.y=numberstens[1].y;q.numflag=1;q.vao=createRectangles(q.x,q.y,q.length,q.breadth,1,1,1); 
 	numberstens.push_back(q);
 	q.length=.05;q.breadth=.2;q.x=numberstens[3].x;q.y=numberstens[0].y;q.numflag=1;q.vao=createRectangles(q.x,q.y,q.length,q.breadth,1,1,1); 
 	numberstens.push_back(q);
 	q.length=.2;q.breadth=.02;q.x=numberstens[2].x;q.y=numberstens[1].y+.3;q.numflag=1;q.vao=createRectangles(q.x,q.y,q.length,q.breadth,1,1,1); 
 	numberstens.push_back(q);
 	q.length=.2;q.breadth=.02;q.x=numberstens[2].x;q.y=numberstens[1].y+.1;q.numflag=1;q.vao=createRectangles(q.x,q.y,q.length,q.breadth,1,1,1); 
 	numberstens.push_back(q);

 	q.length=.05;q.breadth=.2;q.x=8;q.y=3;q.numflag=1;;q.vao=createRectangles(q.x,q.y,q.length,q.breadth,1,1,1); 
 	enemynumbers.push_back(q);
 	q.length=.05;q.breadth=.2;q.x=enemynumbers[0].x;q.y=enemynumbers[0].y-.2;q.numflag=1;q.vao=createRectangles(q.x,q.y,q.length,q.breadth,1,1,1); 
 	enemynumbers.push_back(q);
 	q.length=.2;q.breadth=.02;q.x=enemynumbers[1].x-.1;q.y=enemynumbers[1].y-.1;q.numflag=1;q.vao=createRectangles(q.x,q.y,q.length,q.breadth,1,1,1); 
 	enemynumbers.push_back(q);
 	q.length=.05;q.breadth=.2;q.x=enemynumbers[2].x-.1;q.y=enemynumbers[1].y;q.numflag=1;q.vao=createRectangles(q.x,q.y,q.length,q.breadth,1,1,1); 
 	enemynumbers.push_back(q);
 	q.length=.05;q.breadth=.2;q.x=enemynumbers[3].x;q.y=enemynumbers[0].y;q.numflag=1;q.vao=createRectangles(q.x,q.y,q.length,q.breadth,1,1,1); 
 	enemynumbers.push_back(q);
 	q.length=.2;q.breadth=.02;q.x=enemynumbers[2].x;q.y=enemynumbers[1].y+.3;q.numflag=1;q.vao=createRectangles(q.x,q.y,q.length,q.breadth,1,1,1); 
 	enemynumbers.push_back(q);
 	q.length=.2;q.breadth=.02;q.x=enemynumbers[2].x;q.y=enemynumbers[1].y+.1;q.numflag=1;q.vao=createRectangles(q.x,q.y,q.length,q.breadth,1,1,1); 
 	enemynumbers.push_back(q);

 	q.length=.05;q.breadth=.2;q.x=7;q.y=3;q.numflag=1;;q.vao=createRectangles(q.x,q.y,q.length,q.breadth,1,1,1); 
 	enemynumberstens.push_back(q);
 	q.length=.05;q.breadth=.2;q.x=enemynumberstens[0].x;q.y=enemynumberstens[0].y-.2;q.numflag=1;q.vao=createRectangles(q.x,q.y,q.length,q.breadth,1,1,1); 
 	enemynumberstens.push_back(q);
 	q.length=.2;q.breadth=.02;q.x=enemynumberstens[1].x-.1;q.y=enemynumberstens[1].y-.1;q.numflag=1;q.vao=createRectangles(q.x,q.y,q.length,q.breadth,1,1,1); 
 	enemynumberstens.push_back(q);
 	q.length=.05;q.breadth=.2;q.x=enemynumberstens[2].x-.1;q.y=enemynumberstens[1].y;q.numflag=1;q.vao=createRectangles(q.x,q.y,q.length,q.breadth,1,1,1); 
 	enemynumberstens.push_back(q);
 	q.length=.05;q.breadth=.2;q.x=enemynumberstens[3].x;q.y=enemynumberstens[0].y;q.numflag=1;q.vao=createRectangles(q.x,q.y,q.length,q.breadth,1,1,1); 
 	enemynumberstens.push_back(q);
 	q.length=.2;q.breadth=.02;q.x=enemynumberstens[2].x;q.y=enemynumberstens[1].y+.3;q.numflag=1;q.vao=createRectangles(q.x,q.y,q.length,q.breadth,1,1,1); 
 	enemynumberstens.push_back(q);
 	q.length=.2;q.breadth=.02;q.x=enemynumberstens[2].x;q.y=enemynumberstens[1].y+.1;q.numflag=1;q.vao=createRectangles(q.x,q.y,q.length,q.breadth,1,1,1); 
 	enemynumberstens.push_back(q);

 	q.length=.05;q.breadth=.2;q.x=0;q.y=3;q.numflag=1;;q.vao=createRectangles(q.x,q.y,q.length,q.breadth,1,1,1); 
 	countdown.push_back(q);
 	q.length=.05;q.breadth=.2;q.x=countdown[0].x;q.y=countdown[0].y-.2;q.numflag=1;q.vao=createRectangles(q.x,q.y,q.length,q.breadth,1,1,1); 
 	countdown.push_back(q);
 	q.length=.2;q.breadth=.02;q.x=countdown[1].x-.1;q.y=countdown[1].y-.1;q.numflag=1;q.vao=createRectangles(q.x,q.y,q.length,q.breadth,1,1,1); 
 	countdown.push_back(q);
 	q.length=.05;q.breadth=.2;q.x=countdown[2].x-.1;q.y=countdown[1].y;q.numflag=1;q.vao=createRectangles(q.x,q.y,q.length,q.breadth,1,1,1); 
 	countdown.push_back(q);
 	q.length=.05;q.breadth=.2;q.x=countdown[3].x;q.y=countdown[0].y;q.numflag=1;q.vao=createRectangles(q.x,q.y,q.length,q.breadth,1,1,1); 
 	countdown.push_back(q);
 	q.length=.2;q.breadth=.02;q.x=countdown[2].x;q.y=countdown[1].y+.3;q.numflag=1;q.vao=createRectangles(q.x,q.y,q.length,q.breadth,1,1,1); 
 	countdown.push_back(q);
 	q.length=.2;q.breadth=.02;q.x=countdown[2].x;q.y=countdown[1].y+.1;q.numflag=1;q.vao=createRectangles(q.x,q.y,q.length,q.breadth,1,1,1); 
 	countdown.push_back(q);

 	q.length=.05;q.breadth=.2;q.x=-1;q.y=3;q.numflag=1;;q.vao=createRectangles(q.x,q.y,q.length,q.breadth,1,1,1); 
 	countdowntens.push_back(q);
 	q.length=.05;q.breadth=.2;q.x=countdowntens[0].x;q.y=countdowntens[0].y-.2;q.numflag=1;q.vao=createRectangles(q.x,q.y,q.length,q.breadth,1,1,1); 
 	countdowntens.push_back(q);
 	q.length=.2;q.breadth=.02;q.x=countdowntens[1].x-.1;q.y=countdowntens[1].y-.1;q.numflag=1;q.vao=createRectangles(q.x,q.y,q.length,q.breadth,1,1,1); 
 	countdowntens.push_back(q);
 	q.length=.05;q.breadth=.2;q.x=countdowntens[2].x-.1;q.y=countdowntens[1].y;q.numflag=1;q.vao=createRectangles(q.x,q.y,q.length,q.breadth,1,1,1); 
 	countdowntens.push_back(q);
 	q.length=.05;q.breadth=.2;q.x=countdowntens[3].x;q.y=countdowntens[0].y;q.numflag=1;q.vao=createRectangles(q.x,q.y,q.length,q.breadth,1,1,1); 
 	countdowntens.push_back(q);
 	q.length=.2;q.breadth=.02;q.x=countdowntens[2].x;q.y=countdowntens[1].y+.3;q.numflag=1;q.vao=createRectangles(q.x,q.y,q.length,q.breadth,1,1,1); 
 	countdowntens.push_back(q);
 	q.length=.2;q.breadth=.02;q.x=countdowntens[2].x;q.y=countdowntens[1].y+.1;q.numflag=1;q.vao=createRectangles(q.x,q.y,q.length,q.breadth,1,1,1); 
 	countdowntens.push_back(q);



 	for(int k=0;k<6;k++){
 	  q.x=0;
	  q.y=0;
	  q.length=.4;
	  q.breadth=8;
	  q.rotation=0	;
	  q.flag=0;
	  q.vao = createRectangles(q.x,q.y,q.length,q.breadth,0.647059 ,0.164706, 0.164706); 
	  obstacles.push_back(q);
	}


  
	// Create and compile our GLSL program from the shaders
	programID = LoadShaders( "Sample_GL.vert", "Sample_GL.frag" );
	// Get a handle for our "MVP" uniform
	Matrices.MatrixID = glGetUniformLocation(programID, "MVP");

	
	reshapeWindow (window, width, height);

    // Background color of the scene
	glClearColor (0.3f, 0.3f, 0.3f, 0.0f); // R, G, B, A
	glClearDepth (1.0f);

	glEnable (GL_DEPTH_TEST);
	glDepthFunc (GL_LEQUAL);

    cout << "VENDOR: " << glGetString(GL_VENDOR) << endl;
    cout << "RENDERER: " << glGetString(GL_RENDERER) << endl;
    cout << "VERSION: " << glGetString(GL_VERSION) << endl;
    cout << "GLSL: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << endl;
}

int main (int argc, char** argv)
{

	srand (time(NULL));
	int width = 1100;
	int height = 700;

    GLFWwindow* window = initGLFW(width, height);

	initGL (window, width, height);

    double last_update_time = glfwGetTime(), current_time;
    circles[0].flag=1;
    circles[2].flag=1;
    circles[1].flag=1;

    circles[0].vel=2;
    circles[2].vel=2;
    circles[1].vel=2;

    /* Draw in loop */
    while (!glfwWindowShouldClose(window)) {

    	if(SITECHANGE==1){
			for(int k=1;k<51;k++){
				if(circles[k].flag==1){
					circles[k].x-=.5;            				}
			}

			rectangles[0].x-=.5;
			rectangles[1].x-=.5;
			rectangles[2].x-=.5;
			rectangles[3].x-=.5;
			circles[0].x-=.5;
			if(rectangles[3].x==-20){
				SITECHANGE=2;
			}
		}


    	for(int k=1;k<51;k++){
        if(circles[k].flag==1){

        if(abs(sqrt(pow((bulletx-circles[k].x),2)+(pow((bullety-circles[k].y),2))) )<= circles[k].radius+.2){
            circles[k].y=-5;
            vx=vx*-1;
            score++;
            circles[k%51].flag=0;
            circles[(k+2)%51].flag=1;
          }

          if(abs(sqrt(pow((enemybulletx-circles[k].x),2)+(pow((enemybullety-circles[k].y),2)) ))<= circles[k].radius+.2){
            circles[k].y=-5;
            enemyvx=enemyvx*-1;
            enemyscore++;
            circles[k%51].flag=0;
            circles[k+2%51].flag=1;

          }
        }

        }

        glfwGetCursorPos(window, &xpos, &ypos);
        ypos *=-1;
        ypos += 700;
        xpos -=1100;
        xpos *=-1;
      //  cout << xpos << ypos << endl;

        enemyrectangle_rotation =180- (atan (ypos/xpos) * 180 / M_PI) ;


        reshapeWindow (window, width, height);
        // OpenGL Draw commands
        draw();
        drawscore(score);
        drawenemyscore(enemyscore);

        // Swap Frame Buffer in double buffering
        glfwSwapBuffers(window);//shows the frame you rendered 

        // Poll for Keyboard and mouse events
        glfwPollEvents();

        /*if(enemytranslationdir==0){
	        enemycanonbasey +=.05;
	        enemycanonrecty +=.05;
	    }
	    else{
	    	enemycanonbasey -=.05;
	    	enemycanonrecty -=.05;
	    }
        if(enemycanonbasey >=4){
        	enemytranslationdir=1;
        	enemycanonbasey =3.8;
        	enemycanonrecty =3.8;
        }
        if(enemycanonbasey <=-4){
        	enemytranslationdir=0;
        	enemycanonbasey =-3.8;
        	enemycanonrecty =-3.8;
        }
        if(enemyshootflag==0){
        	enemybullety=enemycanonbasey;
        }

        if(enemyrotdir==0){
	        enemyrectangle_rotation+=1;
	    }
	    else{
	    	enemyrectangle_rotation-=1;
	    }
        if(enemyrectangle_rotation>=170){
        	enemyrotdir=1;
        	enemyrectangle_rotation=169;
        }
        if(enemyrectangle_rotation<=100){
        	enemyrotdir=0;
        	enemyrectangle_rotation=101;
        }
*/

        if(rectangles[2].flag==1){
        	rectangles[2].rotation-=.5;

        }

        if((bulletx>=-3.2 && bulletx<=-2.8  && bullety<=2 && SITECHANGE==2) || (bulletx<=3.2 && bulletx>=2.8 && bullety<=2 && SITECHANGE==2)){
        	vx=vx*-.8;
        }

        if((enemybulletx<=3.2 && enemybulletx>=2.8 && enemybullety<=2 && SITECHANGE==2 || enemybulletx<=-2.8) &&  (enemybulletx>=-3.2 && enemybullety<=2 && SITECHANGE==2)){
        	enemyvx=enemyvx*-.8;
        }

        for(int k=1;k<51;k++){
        	if(k%6==0){
        		triangles[0].flag=1;
        	}

        	if(circles[k].flag==1){

        	if(abs(sqrt(pow((bulletx-circles[k].x),2)+(pow((bullety-circles[k].y),2))) )<= circles[k].radius+.2){
	        	circles[k].y=-5;
            vx=vx*-.8;
            score++;
            circles[k%51].flag=0;
            circles[(k+2)%51].flag=1;

	        }

	        else if(abs(sqrt(pow((enemybulletx-circles[k].x),2)+(pow((enemybullety-circles[k].y),2)) ))<= circles[k].radius+.2){
	        	 circles[k].y=-5;
            enemyvx=enemyvx*-.8;
            enemyscore++;
            circles[k%51].flag=0;
            circles[(k+2)%51].flag=1;
	        }

        	circles[k].time+=.05;
        	circles[k].y=-4+5*circles[k].time + (-1)*circles[k].time*circles[k].time;
        	
        	if(circles[k].y<=-6){
        		circles[k].time=0;
        		circles[k].flag=0;
        		int r = ((k+7)%51)+1;
        		if(circles[r].flag==1){
        			circles[(r+1)%51].flag=1;
        			r=r+1;
        			//circles[(r+1)%51].time==.05;
        		}

        		circles[r].flag=1;
        		if(SITECHANGE==0){
        			circles[r].x=rand()%7-3;
        		}        		else if(SITECHANGE==2){
        		circles[r].x=rand()%4-2;
        	}

        		circles[r].vel=2;
        		circles[r].y=-4;


        	}
	      }  

        }

        if(speedflag==1){
        	if(bulletx>=-9.9){
        	velocity = velocity + .2;
        	theta = rectangle_rotation*M_PI/180.0f;
        	//cout << theta << endl;	
        	bulletx = bulletx+-.05*cos(theta);
        	bullety = bullety+-.05*sin(theta);
        	}	
        }
        if(upflag==1){
        	bullety+=.1;
        	canonbasey+=.1;
        	canonrecty+=.1;
        }

        if(downflag==1){
        	bullety-=.1;
        	canonbasey-=.1;
        	canonrecty-=.1;
        }
        if(dirupflag==1){
        	rectangle_rotation+=2;
        	if(rectangle_rotation>=80){
        		rectangle_rotation=80;
        	}
        }
        if(dirdownflag==1){
        	rectangle_rotation-=2;
        	if(rectangle_rotation<=10){
        		rectangle_rotation=10;
        	}
        }

        if(shootflag==1){
            	shoot();
            	i=i+0.01;	
            }



        if(enemyspeedflag==1){
        	if(enemybulletx<=9.9){
        	enemyvelocity = enemyvelocity + .2;
        	theta = enemyrectangle_rotation*M_PI/180.0f;
        	//cout << theta << endl;	
        	enemybulletx = enemybulletx+-.05*cos(theta);
        	enemybullety = enemybullety+-.05*sin(theta);
        	}	
        }
        if(enemyupflag==1){
        	enemybullety+=.1;
        	enemycanonbasey+=.1;
        	enemycanonrecty+=.1;
        }

        if(enemydownflag==1){
        	enemybullety-=.1;
        	enemycanonbasey-=.1;
        	enemycanonrecty-=.1;
        }
        if(enemydirupflag==1){
        	enemyrectangle_rotation+=2;
        	if(enemyrectangle_rotation>=170){
        		enemyrectangle_rotation=169;
        	}
        }
        if(enemydirdownflag==1){
        	enemyrectangle_rotation-=2;
        	if(enemyrectangle_rotation<=100){
        		enemyrectangle_rotation=101;
        	}
        }

	        if(enemyshootflag==1){
	    	enemyshoot();
	    	enemyi=enemyi+0.01;	
	    }

        // Control based on time (Time based transformation like 5 degrees rotation every 0.5s)
        current_time = glfwGetTime(); // Time in seconds,can be used to make animations smooth
        if ((current_time - last_update_time) >= 1) { // atleast 0.5s elapsed since last frame
            // do something every 0.5 seconds ..
            last_update_time = current_time;
            COUNTDOWN --;
            drawenemycountdown(COUNTDOWN);
            if(SITECHANGE==2 && COUNTDOWN == 0){
              cout << "player 1 score : " << score << endl;
              cout << "player 2 score : " << enemyscore << endl;
              quit(window);
            }
            if(COUNTDOWN==0){

            	SITECHANGE = 1;
              COUNTDOWN = 30;
            }
            /*enemyi=0;enemyvelocity=1.2;
            enemybulletx=enemycanonrectx;
            enemybullety=enemycanonrecty;
            enemyshootflag=1;
            theta = enemyrectangle_rotation*M_PI/180.0f;

            	enemyvx =enemyvelocity*cos(theta);
  				enemyvy =enemyvelocity*sin(theta);*/
            
            
        }
    }

    glfwTerminate();
    exit(EXIT_SUCCESS);
}

