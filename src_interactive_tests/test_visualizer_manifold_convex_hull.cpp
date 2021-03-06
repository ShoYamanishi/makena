#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <math.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtx/norm.hpp>
#include <AntTweakBar.h>

#include "primitives.hpp"
#include "manifold.hpp"

/** @file src_interactive_tests/test_visualizer_manifold_convex_hull.cpp
 *
 *  @brief visual test rig for Makena::Manifold
 *
 *  Usage: test_visualizer_manifold_convex_hull <test input file>
 *
 *  @dependencies
 *    OpenGL 3.3 or later
 *    GLFW3
 *    GLEW
 *    GLM
 *    AntTweakbar 1.16 custom (updated for GLFW3 and Macbook Retina)
 */

static GLFWwindow* window;

static int windowWidth, windowHeight;

static GLuint loadShaders(const char * v_path,const char * f_path);

static void CB_window_size(GLFWwindow *window, int w, int h){

    int widthInPixel, heightInPixel;
    glfwGetFramebufferSize(window, &widthInPixel, &heightInPixel);
    glViewport( 0, 0, (GLsizei)widthInPixel, (GLsizei)heightInPixel);
    TwWindowSize(widthInPixel, heightInPixel);

}

void CB_cursor_pos(GLFWwindow *window, double x, double y){
    std::cerr << "CB cursor pos: " << x << "," << y << "\n";
}

static void TwEventMouseButtonGLFW3(GLFWwindow* window, int button, int action, int mods) {
    TwEventMouseButtonGLFW(button, action);
}

static void TwEventMousePosGLFW3(GLFWwindow* window, double xpos, double ypos) {
    TwMouseMotion(int(xpos), int(ypos));
}

static void TwEventMouseWheelGLFW3(GLFWwindow* window, double xoffset, double yoffset) {
    TwEventMouseWheelGLFW(yoffset);
}

static void TwEventKeyGLFW3(GLFWwindow* window, int key, int scancode, int action, int mods) {
    TwEventKeyGLFW(key, action);
}

static void TwEventCharGLFW3(GLFWwindow* window, int codepoint) {
    TwEventCharGLFW(codepoint, GLFW_PRESS);
}


void makeGCSAxesLines(
    std::vector<Makena::Vec3>&       vertices,
    std::vector<Makena::Vec3>&       colors
) {

    Makena::Vec3 ori(0.0, 0.0, 0.0);

    Makena::Vec3 x1(  1.0, 0.0, 0.0);
    Makena::Vec3 x2( 10.0, 0.0, 0.0);
    Makena::Vec3 x3(-10.0, 0.0, 0.0);

    Makena::Vec3 y1(0.0,   1.0, 0.0);
    Makena::Vec3 y2(0.0,  10.0, 0.0);
    Makena::Vec3 y3(0.0, -10.0, 0.0);

    Makena::Vec3 z1(0.0, 0.0,   1.0);
    Makena::Vec3 z2(0.0, 0.0,  10.0);
    Makena::Vec3 z3(0.0, 0.0, -10.0);

    Makena::Vec3 red_high  (1.0, 0.0, 0.0);
    Makena::Vec3 red_low   (0.2, 0.0, 0.0);
    Makena::Vec3 green_high(0.0, 1.0, 0.0);
    Makena::Vec3 green_low (0.0, 0.2, 0.0);
    Makena::Vec3 blue_high (0.0, 0.0, 1.0);
    Makena::Vec3 blue_low  (0.0, 0.0, 0.2);

    vertices.push_back(ori);
    vertices.push_back(x1);
    colors.push_back(red_high);
    colors.push_back(red_high);

    vertices.push_back(x1);
    vertices.push_back(x2);
    colors.push_back(red_low);
    colors.push_back(red_low);

    vertices.push_back(ori);
    vertices.push_back(x3);
    colors.push_back(red_low);
    colors.push_back(red_low);

    vertices.push_back(ori);
    vertices.push_back(y1);
    colors.push_back(green_high);
    colors.push_back(green_high);

    vertices.push_back(y1);
    vertices.push_back(y2);
    colors.push_back(green_low);
    colors.push_back(green_low);

    vertices.push_back(ori);
    vertices.push_back(y3);
    colors.push_back(green_low);
    colors.push_back(green_low);

    vertices.push_back(ori);
    vertices.push_back(z1);
    colors.push_back(blue_high);
    colors.push_back(blue_high);

    vertices.push_back(z1);
    vertices.push_back(z2);
    colors.push_back(blue_low);
    colors.push_back(blue_low);

    vertices.push_back(ori);
    vertices.push_back(z3);
    colors.push_back(blue_low);
    colors.push_back(blue_low);
}


static void parseInputString(
    char* filename,
    std::vector<Makena::Vec3>& points
) {
    points.clear();

    std::string   sfilename(filename);
    std::ifstream gs;
    gs.open (sfilename);;

    if (gs.is_open()) {
        std::string   oneLine;
        while (getline(gs, oneLine)) {
            if (oneLine.length()==0)
                continue;
            if (oneLine[0]=='#')
                continue;
            std::stringstream lineStream(oneLine);
            std::string field;
            std::vector<float> p;
            while (std::getline(lineStream, field, ',')) {
                p.push_back(std::atof(field.c_str()));
            }
            Makena::Vec3 v(p[0],p[1],p[2]);
            points.push_back(v);
        }
    }
    gs.close();
}


static void parseInputDecDump(
    char* filename,
    std::vector<Makena::Vec3>& points
) {
    points.clear();

    std::string   sfilename(filename);
    std::ifstream gs;
    gs.open (sfilename);;

    if (gs.is_open()) {
        std::string   oneLine;
        while (getline(gs, oneLine)) {
            if (oneLine.length()==0)
                continue;
            if (oneLine[0]=='#')
                continue;
            std::stringstream lineStream(oneLine);
            std::string field;
            std::vector<unsigned char> ubs;
            while (std::getline(lineStream, field, ' ')) {
                ubs.push_back(std::atoi(field.c_str()));
            }
            Makena::Vec3 v3;
            for (long j = 0; j < 3; j++) {
                double d;
                unsigned char *p = (unsigned char*)&d;
                for (long k = 0; k < 8; k++) {
                    *p = ubs[j * 8 + k];
                    p++;
                }
                if (j==0) {
                    v3.setX(d);
                }
                if (j==1) {
                    v3.setY(d);
                }
                if (j==2) {
                    v3.setZ(d);
                }
            }
            points.push_back(v3);
        }
    }
    gs.close();
}


class GLfloatArrayDelete {public:void operator()(void* x){free(x);}};

using GLfloatArray = std::unique_ptr<GLfloat,GLfloatArrayDelete>;

GLfloatArray makeGLbufferArray(std::vector<Makena::Vec3>& points)
{
    GLfloatArray ar((GLfloat*)malloc(points.size()*sizeof(GLfloat)*3));
    size_t index =0;
    GLfloat* ap = (GLfloat*)ar.get();
    for (auto& p : points) {
        ap[index++] = p.x();
        ap[index++] = p.y();
        ap[index++] = p.z();
    }
    return ar;
}


GLfloatArray makeGLbufferArray(std::vector<Makena::Vec3>& points, float w)
{
    GLfloatArray ar((GLfloat*)malloc(points.size()*sizeof(GLfloat)*4));
    size_t index =0;
    GLfloat* ap = (GLfloat*)ar.get();
    for (auto& p : points) {
        ap[index++] = p.x();
        ap[index++] = p.y();
        ap[index++] = p.z();
        ap[index++] = w;
    }
    return ar;
}


bool immediateMode = false;

std::vector<Makena::Vec3> points;
Makena::Manifold mani;
static int step = -1;
static int currentVertexIndex = 0;
static std::vector<Wailea::Undirected::node_list_it_t> nodeList;
static Makena::Vec3 currentP(0.0, 0.0, 0.0);

static std::vector<Makena::Vec3> circumferenceVertices;
static std::vector<Makena::Vec3> circumferenceColors;

std::stringstream ssImportExport;

static void advanceAlgorithm()
{
    if (step != -4 && step != -3) {
        std::cerr << "\n\nStep: " << step << "\n";
    }
    if (immediateMode) {
        if (step == -1) {
            enum Makena::predicate pred;
            mani.findConvexHull(points, pred);
            step = -2;
        }
        else if (step == -2) {
            mani.debugFindConvexHullTerm();
            std::cerr << "FINISHED!\n";
            step = -3;

            Makena::Manifold::Martialled M = mani.exportData();
            Makena::Manifold::emitText(M, ssImportExport);
            //std::cerr << "\n";
            //std::cerr << ssImportExport.str();
            //std::cerr << "\n";
        }
        else if (step == -3) {
            // Recreate from text
            Makena::Manifold::Martialled M =  
                             Makena::Manifold::parseTextData(ssImportExport);
            mani.importData(M);
            step = -4;
        }
        else if (step == -4) {
            ; // dead end
        }
    }
    else {
        if (step > 0) {
            if (step > nodeList.size()*4) {
                std::cerr << "Finished!\n";
                return;
            }
        }
        if (step == -1) {
//            mani.setLogLevel(Makena::Manifold::ALL);
            mani.debugFindConvexHullStep1(points);
            nodeList = mani.debugFindConvexHullVertices();
            currentVertexIndex = 0;
            step++;
        }
        else if (step%4 == 0) {
            circumferenceVertices.clear();
            circumferenceColors.clear();
            if (currentVertexIndex >= nodeList.size()){
                step = -2;
            }
            else {
                currentP=mani.debugCurrentPoint(nodeList[currentVertexIndex]);
                std::cerr << "currentP: " << currentP.x() << ","  
                  << currentP.y() << ","  << currentP.z() << "\n";
                step++;
            }
        }
        else if (step%4 == 1) {
            bool res = 
               mani.debugFindConvexHullLoopStep1(nodeList[currentVertexIndex]);
            if (res) {
                Makena::Vec3 red (1.0, 0.0, 0.0);
                mani.debugFindConvexHullHalfEdges(
                             red, circumferenceVertices, circumferenceColors);
                step++;
            }
            else {
                step +=3; // Skipping steps 2& 3
                currentVertexIndex++;
                if (currentVertexIndex >= nodeList.size()){
                    step = -2;
                }
            }
        }
        else if (step%4 == 2) {
            circumferenceVertices.clear();
            circumferenceColors.clear();
            bool res = mani.debugFindConvexHullLoopStep2();
            if (!res) {
                step +=2; // Skipping  step 3
                currentVertexIndex++;
                if (currentVertexIndex >= nodeList.size()){
                    step = -2;
                }
            }
            else {
                step++;
            }
        }
        else if (step%4 == 3) {
            mani.debugFindConvexHullLoopStep3();

            step++;

            currentVertexIndex++;

            if (currentVertexIndex >= nodeList.size()){

                step = -2;

            }

        }
        else if (step == -2) {
            mani.debugFindConvexHullTerm();
            std::cerr << "FINISHED!\n";
            step = -3;
        }
        else if (step == -3) {
            ; // dead end
        }
    }
}


int main( int argc, char* argv[])
{
    if (argc != 2 && argc != 3) {
        std::cerr << "Usage: test_visualizer_manifold_convex_hull "
                     "[test input file] MANUAL "
                     "(Specify if you want to advance by hitting space key) "
                     "/ IMMEDIATE "
                     "(Specify if you want to the immediate result)\n";

        exit(1);
    }

    bool manualStep = false;
    if (argc == 3) {
        if (strcmp(argv[2], "IMMEDIATE")==0) {
            immediateMode = true;
        }
        else if (strcmp(argv[2], "MANUAL")==0) {
            manualStep = true;
        }

    }

//    parseInputDecDump(argv[1],points);
    parseInputString(argv[1],points);

    std::vector<Makena::Vec3> vecAxesGCSVertices;
    std::vector<Makena::Vec3> vecAxesGCSColors;

    makeGCSAxesLines(vecAxesGCSVertices, vecAxesGCSColors);

    auto arrAxesGCSVertices = makeGLbufferArray(vecAxesGCSVertices);
    auto arrAxesGCSColors   = makeGLbufferArray(vecAxesGCSColors);
    TwBar *twBar01;

    // Initialise GLFW
    if (!glfwInit()) {
        std::cerr << "glfwInit() failed.\n";
        return 1;
    }

    glfwWindowHint(GLFW_SAMPLES, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    window = glfwCreateWindow(
                        1024, 768, "Test Visualiser Convex Hull", NULL, NULL);

    if ( window == NULL ) {
        std::cerr << "glfwCreateWindow failed\n";
        glfwTerminate();
        return 1;
    }

    glfwMakeContextCurrent(window);

    int widthInPixel, heightInPixel;
    glfwGetFramebufferSize(window, &widthInPixel, &heightInPixel);

    int width, height;
    glfwGetWindowSize(window, &width, &height);

    glfwSetWindowSizeCallback(window, CB_window_size);
    glfwGetWindowSize(window, &windowWidth, &windowHeight);

    glewExperimental = true;

    if (glewInit() != GLEW_OK) {
        std::cerr << "Failed to initialize GLEW\n";;
        glfwTerminate();
        return 1;
    }

    TwInit(TW_OPENGL_CORE, NULL);
    TwWindowSize(widthInPixel, heightInPixel);
    TwSetPixelRatio(
         float(widthInPixel)/(float)width, float(heightInPixel)/(float)height);
    twBar01 = TwNewBar("Control");

    TwDefine(" GLOBAL help='Control Panel' ");

    double distance = 7.0; 
    TwAddVarRW(twBar01, "distance", TW_TYPE_DOUBLE, &distance, 
            " label='Camera Distance' min=2.0 max=20.0 step=0.1 keyIncr=s "
                          "keyDecr=S help='Rotation speed (turns/second)' ");

    glm::quat orientation;
    TwAddVarRW(twBar01, "Quaternion", TW_TYPE_QUAT4F, &orientation, 
                                               " showval=true opened=true ");

    TwSetParam(twBar01, NULL, "refresh",  TW_PARAM_CSTRING, 1, "0.1");
    TwSetParam(twBar01, NULL, "position", TW_PARAM_CSTRING, 1, " 10 10 ");

    glfwSetMouseButtonCallback (window, (GLFWmousebuttonfun) 
                                                      TwEventMouseButtonGLFW3);
    glfwSetCursorPosCallback   (window, (GLFWcursorposfun)
                                                      TwEventMousePosGLFW3);
    glfwSetScrollCallback      (window, (GLFWscrollfun) 
                                                      TwEventMouseWheelGLFW3);
    glfwSetKeyCallback         (window, (GLFWkeyfun)  TwEventKeyGLFW3);
    glfwSetCharCallback        (window, (GLFWcharfun) TwEventCharGLFW3);


    glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);
    glfwSetInputMode(window, GLFW_CURSOR,      GLFW_CURSOR_NORMAL);

    glfwPollEvents();

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

    GLuint VertexArrayID;
    glGenVertexArrays(1, &VertexArrayID);
    glBindVertexArray(VertexArrayID);

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS); 
    glEnable(GL_CULL_FACE);

    GLuint idPointsVertices;
    glGenBuffers(1, &idPointsVertices);

    GLuint idPointsColors;
    glGenBuffers(1, &idPointsColors);

    GLuint idLinesVertices;
    glGenBuffers(1, &idLinesVertices);

    GLuint idLinesColors;
    glGenBuffers(1, &idLinesColors);

    GLuint idTrianglesVertices;
    glGenBuffers(1, &idTrianglesVertices);

    GLuint idTrianglesColors;
    glGenBuffers(1, &idTrianglesColors);

    GLuint idTrianglesNormals;
    glGenBuffers(1, &idTrianglesNormals);

 
    GLuint SID_PT = loadShaders(
        "src_interactive_tests/shaders/vert_pt.glsl",
        "src_interactive_tests/shaders/frag_pt.glsl" 
    );

    GLuint SID_POL = loadShaders(
        "src_interactive_tests/shaders/vert_pol.glsl",
        "src_interactive_tests/shaders/frag_pol.glsl" 
    );

    GLuint SID_POL_MVP       = glGetUniformLocation(SID_POL, "MVP");
    GLuint SID_POL_M         = glGetUniformLocation(SID_POL, "M");
    GLuint SID_POL_V         = glGetUniformLocation(SID_POL, "V");
    GLuint SID_POL_lightWCS  = glGetUniformLocation(SID_POL, "lightWCS");

    GLuint SID_POL_vertexLCS = glGetAttribLocation(SID_POL, "vertexLCS");
    GLuint SID_POL_vertexRGBA= glGetAttribLocation(SID_POL, "vertexRGBA");
    GLuint SID_POL_normalLCS = glGetAttribLocation(SID_POL, "normalLCS");

    GLuint SID_PT_MVP       = glGetUniformLocation(SID_PT, "MVP");
    GLuint SID_PT_M         = glGetUniformLocation(SID_PT, "M");
    GLuint SID_PT_lightWCS  = glGetUniformLocation(SID_PT, "lightWCS");

    GLuint SID_PT_vertexLCS = glGetAttribLocation(SID_PT, "vertexLCS");
    GLuint SID_PT_vertexRGB = glGetAttribLocation(SID_PT, "vertexRGB");

    bool spaceOn = false;

    do {

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


        glm::vec3 scaleFactor(1.0f, 1.0f, 1.0f);
        glm::mat4 Mscale = glm::scale( glm::mat4(), scaleFactor );
                                   
        glm::quat Qrot(0.0f, 1.0f, 0.0f, 0.0f);
	glm::mat4 Mrot   = glm::mat4_cast(orientation);

	glm::mat4 Mtrans = glm::translate(
                                    glm::mat4(), glm::vec3(0.0f, 0.0f, 0.0f));
	glm::mat4 M      = Mtrans * Mrot * Mscale;

	glm::mat4 V      = glm::lookAt(glm::vec3( 0, 0, distance ), // Cam pos
                                       glm::vec3( 0, 0, 0 ),  // and looks here
                                       glm::vec3( 0, 1, 0 ) );// Head is up

	glm::mat4 P      = glm::perspective(
                               glm::radians(45.0f), 4.0f / 3.0f, 0.1f, 100.0f);
 
        glm::mat4 MVP    = P * V * M;

	glm::vec3 lightPos = glm::vec3(4.0, 4.0, 4.0);

        glDisable(GL_BLEND);

        {

            Makena::Vec3 gray(0.8, 0.8, 0.8);

            std::vector<Makena::Vec3> vecPointsVertices;
            std::vector<Makena::Vec3> vecPointsColors;

            vecPointsVertices.push_back(currentP);
            Makena::Vec3 red(1.0, 0.0, 0.0);
            vecPointsColors.push_back(red);

            Makena::Manifold::makeOpenGLVerticesColorsForPoints(
                             points, gray, vecPointsVertices, vecPointsColors);
            auto arrPointsVertices = makeGLbufferArray(vecPointsVertices);
            auto arrPointsColors   = makeGLbufferArray(vecPointsColors);

            std::vector<Makena::Vec3> vecLinesVertices;
            std::vector<Makena::Vec3> vecLinesColors;

            makeGCSAxesLines(vecLinesVertices, vecLinesColors);

            vecLinesVertices.insert(vecLinesVertices.end(), 
                   circumferenceVertices.begin(), circumferenceVertices.end());

            vecLinesColors.insert(vecLinesColors.end(), 
                      circumferenceColors.begin(), circumferenceColors.end());

            Makena::Vec3 blue(0.5, 0.7, 1.0);
            mani.makeOpenGLVerticesColorsForTriangleWireFrame(
                                    blue, vecLinesVertices,  vecLinesColors);


            if (step == -3||step == -4) {

                // Show the vertex normals.
                Makena::Vec3 darkblue(0.0, 0.0, 1.0);
                auto vPair = mani.vertices();
                for (auto vit = vPair.first; vit != vPair.second; vit++) {
                    Makena::Vec3 p1 = (*vit)->pLCS();
                    Makena::Vec3 p2 = p1 + (*vit)->nLCS();
                    vecLinesVertices.push_back(p1);
                    vecLinesVertices.push_back(p2);
                    vecLinesColors.push_back(darkblue);
                    vecLinesColors.push_back(darkblue);
                }

                Makena::Vec3 darkred(1.0, 0.0, 0.0);
                auto ePair = mani.edges();
                for (auto eit = ePair.first; eit != ePair.second; eit++) {
                    auto heit = (*eit)->he1();
                    auto vit1 = (*heit)->src();
                    auto vit2 = (*heit)->dst();
                    Makena::Vec3 p1 = (*vit1)->pLCS() + (*vit2)->pLCS();
                    p1.scale(0.5);
                    Makena::Vec3 p2 = p1 + (*eit)->nLCS();
                    vecLinesVertices.push_back(p1);
                    vecLinesVertices.push_back(p2);
                    vecLinesColors.push_back(darkred);
                    vecLinesColors.push_back(darkred);
                }

                Makena::Vec3 darkyellow(1.0, 1.0, 0.0);
                auto fPair = mani.faces();
                for (auto fit = fPair.first; fit != fPair.second; fit++) {
                    double cnt = 0.0;
                    Makena::Vec3 vTotal(0.0, 0.0, 0.0);
                    for (auto he : (*fit)->halfEdges()) {
                        auto v1 = (*he)->src();
                        vTotal += ((*v1)->pLCS());
                        cnt += 1.0;
                    }
                    vTotal.scale(1.0/cnt);

                    Makena::Vec3 p1 = vTotal;
                    Makena::Vec3 p2 = p1 + (*fit)->nLCS();
                    vecLinesVertices.push_back(p1);
                    vecLinesVertices.push_back(p2);
                    vecLinesColors.push_back(darkyellow);
                    vecLinesColors.push_back(darkyellow);

                }


            }
            auto arrLinesVertices = makeGLbufferArray(vecLinesVertices);
            auto arrLinesColors   = makeGLbufferArray(vecLinesColors);


            glUseProgram(SID_PT);

            glPointSize(2.0);

            glUniformMatrix4fv(SID_PT_MVP, 1, GL_FALSE, &MVP[0][0] );
            glUniformMatrix4fv(SID_PT_M,   1, GL_FALSE, &M  [0][0] );
            glUniform3f(SID_PT_lightWCS, lightPos.x, lightPos.y, lightPos.z);

            glEnableVertexAttribArray(0);

            glBindBuffer(GL_ARRAY_BUFFER, idLinesVertices);
            glBufferData(
                GL_ARRAY_BUFFER,
                vecLinesVertices.size()*sizeof(GLfloat)*3,
                arrLinesVertices.get(),
                GL_DYNAMIC_DRAW
            );
            glVertexAttribPointer(
                         SID_PT_vertexLCS, 3, GL_FLOAT, GL_FALSE, 0,(void*)0);

            glEnableVertexAttribArray(1);
            glBindBuffer(GL_ARRAY_BUFFER, idLinesColors);
            glBufferData(
                GL_ARRAY_BUFFER,
                vecLinesColors.size()*sizeof(GLfloat)*3,
                arrLinesColors.get(),
                GL_DYNAMIC_DRAW
            );
            glVertexAttribPointer(
                         SID_PT_vertexRGB, 3, GL_FLOAT, GL_FALSE, 0,(void*)0);

            glDrawArrays(GL_LINES, 0, vecLinesVertices.size());

            glDisableVertexAttribArray(1);
            glDisableVertexAttribArray(0);

            glEnableVertexAttribArray(0);
            glBindBuffer(GL_ARRAY_BUFFER, idPointsVertices);
            glBufferData(
                GL_ARRAY_BUFFER,
                vecPointsVertices.size()*sizeof(GLfloat)*3,
                arrPointsVertices.get(),
                GL_DYNAMIC_DRAW
            );
            glVertexAttribPointer(
                         SID_PT_vertexLCS, 3, GL_FLOAT, GL_FALSE, 0,(void*)0);

            glEnableVertexAttribArray(1);
            glBindBuffer(GL_ARRAY_BUFFER, idPointsColors);
            glBufferData(
                GL_ARRAY_BUFFER,
                vecPointsColors.size()*sizeof(GLfloat)*3,
                arrPointsColors.get(),
                GL_DYNAMIC_DRAW
            );
            glVertexAttribPointer(
                         SID_PT_vertexRGB, 3, GL_FLOAT, GL_FALSE, 0,(void*)0);
            glPointSize(2.0);
            glDrawArrays(GL_POINTS, 0, vecPointsVertices.size());

            glDisableVertexAttribArray(1);
            glDisableVertexAttribArray(0);

        }

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        {

            std::vector<Makena::Vec3> vecTrianglesVertices;
            std::vector<Makena::Vec3> vecTrianglesColors;
            std::vector<Makena::Vec3> vecTrianglesNormals;
            Makena::Vec3 blue(0.5, 0.7, 1.0);

            mani.makeOpenGLVerticesColorsNormalsForTriangles(
                             blue, vecTrianglesVertices, vecTrianglesColors,
                                                 vecTrianglesNormals, false);
            auto arrTrianglesVertices=makeGLbufferArray(vecTrianglesVertices);

            auto arrTrianglesColors=makeGLbufferArray(vecTrianglesColors, 0.8);

            auto arrTrianglesNormals=makeGLbufferArray(vecTrianglesNormals);

            glUseProgram(SID_POL);

            glUniformMatrix4fv(SID_POL_MVP, 1, GL_FALSE, &MVP[0][0] );
            glUniformMatrix4fv(SID_POL_M,   1, GL_FALSE, &M  [0][0] );
            glUniformMatrix4fv(SID_POL_V,   1, GL_FALSE, &V  [0][0] );
            glUniform3f(SID_POL_lightWCS, lightPos.x, lightPos.y, lightPos.z);

            glEnableVertexAttribArray(0);
            glBindBuffer(GL_ARRAY_BUFFER, idTrianglesVertices);
            glBufferData(
                GL_ARRAY_BUFFER,
                vecTrianglesVertices.size()*sizeof(GLfloat)*3,
                arrTrianglesVertices.get(),
                GL_DYNAMIC_DRAW
            );

            glVertexAttribPointer(
                         SID_POL_vertexLCS, 3, GL_FLOAT, GL_FALSE, 0,(void*)0);

            glEnableVertexAttribArray(1);
            glBindBuffer(GL_ARRAY_BUFFER, idTrianglesColors);
            glBufferData(
                GL_ARRAY_BUFFER,
                vecTrianglesColors.size()*sizeof(GLfloat)*4,
                arrTrianglesColors.get(),
                GL_DYNAMIC_DRAW
            );
            glVertexAttribPointer(
                        SID_POL_vertexRGBA, 4, GL_FLOAT, GL_FALSE, 0,(void*)0);

            glEnableVertexAttribArray(2);
            glBindBuffer(GL_ARRAY_BUFFER, idTrianglesNormals);
            glBufferData(
                GL_ARRAY_BUFFER,
                vecTrianglesNormals.size()*sizeof(GLfloat)*3,
                arrTrianglesNormals.get(),
                GL_DYNAMIC_DRAW
            );

            glVertexAttribPointer(
                         SID_POL_normalLCS, 3, GL_FLOAT, GL_FALSE, 0,(void*)0);

            glDrawArrays(GL_TRIANGLES, 0, vecTrianglesVertices.size());

            glDisableVertexAttribArray(2);
            glDisableVertexAttribArray(1);
            glDisableVertexAttribArray(0);
        }


        TwDraw();

        glfwSwapBuffers(window);

        glfwPollEvents();

        if (manualStep) {
            if (glfwGetKey(window,GLFW_KEY_SPACE ) == GLFW_PRESS && !spaceOn) {
                spaceOn = true;
                advanceAlgorithm();

            }
            else if (glfwGetKey(window,GLFW_KEY_SPACE)!=GLFW_PRESS &&spaceOn) {
                spaceOn = false;
            }
        }
        else {

            if (!spaceOn) {
                if (glfwGetKey(window,GLFW_KEY_SPACE ) ==
                                                 GLFW_PRESS && !spaceOn) {
                    spaceOn = true;
                }
            }
            else {

                advanceAlgorithm();

            }

        }

    } while ( glfwGetKey(window, GLFW_KEY_ESCAPE ) != GLFW_PRESS &&
              glfwWindowShouldClose(window) == 0                    );

    TwTerminate();

    glDeleteBuffers(1, &idTrianglesVertices);
    glDeleteBuffers(1, &idTrianglesColors);
    glDeleteBuffers(1, &idTrianglesNormals);

    glDeleteBuffers(1, &idLinesVertices);
    glDeleteBuffers(1, &idLinesColors);

    glDeleteBuffers(1, &idPointsVertices);
    glDeleteBuffers(1, &idPointsColors);
    glDeleteVertexArrays(1, &VertexArrayID);

    glDeleteProgram(SID_POL);
    glDeleteProgram(SID_PT);

    glfwTerminate();

    return 0;
}


static GLuint loadShaders(const char * v_path,const char * f_path)
{
    GLuint VertexShaderID   = glCreateShader(GL_VERTEX_SHADER);
    GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);
    std::string   VertexShaderCode;
    std::ifstream VertexShaderStream(v_path, std::ios::in);

    if (VertexShaderStream.is_open()) {

        std::string Line = "";

        while (getline(VertexShaderStream, Line)) {

            VertexShaderCode += "\n" + Line;
        }

        VertexShaderStream.close();

    }
    else {
        std::cerr << "failed to open [%s]\n";
        return 0;
    }

    std::string   FragmentShaderCode;
    std::ifstream FragmentShaderStream(f_path, std::ios::in);

    if(FragmentShaderStream.is_open()){

        std::string Line = "";

        while (getline(FragmentShaderStream, Line)) {

            FragmentShaderCode += "\n" + Line;

        }

        FragmentShaderStream.close();
    }

    GLint Result = GL_FALSE;
    int   InfoLogLength;

    char const * VertexSourcePointer = VertexShaderCode.c_str();
    glShaderSource(VertexShaderID, 1, &VertexSourcePointer , NULL);
    glCompileShader(VertexShaderID);

    glGetShaderiv(VertexShaderID, GL_COMPILE_STATUS, &Result);
    glGetShaderiv(VertexShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);

    if ( InfoLogLength > 0 ) {

        std::vector<char> VertexShaderErrorMessage(InfoLogLength+1);
        glGetShaderInfoLog(
            VertexShaderID,
            InfoLogLength,
            NULL,
            &VertexShaderErrorMessage[0]
        );
        std::cerr << "[" << v_path << "]: ";
        std::cerr << &VertexShaderErrorMessage[0] << "\n";
    }

    char const * FragmentSourcePointer = FragmentShaderCode.c_str();
    glShaderSource(FragmentShaderID, 1, &FragmentSourcePointer , NULL);
    glCompileShader(FragmentShaderID);

    glGetShaderiv(FragmentShaderID, GL_COMPILE_STATUS, &Result);
    glGetShaderiv(FragmentShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
    if ( InfoLogLength > 0 ) {

        std::vector<char> FragmentShaderErrorMessage(InfoLogLength+1);
        glGetShaderInfoLog(
            FragmentShaderID,
            InfoLogLength,
            NULL,
            &FragmentShaderErrorMessage[0]
        );
        std::cerr << "[" << f_path << "]: ";
        std::cerr << &FragmentShaderErrorMessage[0] << "\n";;
    }

    GLuint ProgramID = glCreateProgram();
    glAttachShader(ProgramID, VertexShaderID);
    glAttachShader(ProgramID, FragmentShaderID);
    glLinkProgram(ProgramID);
    glGetProgramiv(ProgramID, GL_LINK_STATUS, &Result);
    glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &InfoLogLength);

    if ( InfoLogLength > 0 ) {
        std::vector<char> ProgramErrorMessage(InfoLogLength+1);
        glGetProgramInfoLog(
                      ProgramID, InfoLogLength, NULL, &ProgramErrorMessage[0]);
        std::cerr << "Linker: ";
        std::cerr << &ProgramErrorMessage[0] << "\n";

    }


    glDetachShader(ProgramID, VertexShaderID);
    glDetachShader(ProgramID, FragmentShaderID);
  
    glDeleteShader(VertexShaderID);
    glDeleteShader(FragmentShaderID);

    return ProgramID;
}

