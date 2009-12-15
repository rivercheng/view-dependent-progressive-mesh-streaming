#define GL_GLEXT_PROTOTYPES
#include <GL/glut.h>
#include <sstream>
#include <sys/time.h>
#include <cstdlib>
#include "render.hh"
#include "gfmesh.hh"
#include "common_def.hh"

const size_t FRAME_BUFFER_SIZE = 1024*768*3;
static Render* render_;
static unsigned char pixels_[FRAME_BUFFER_SIZE] = {0};

void draw_surface_with_arrays()
{
    Gfmesh *gfmesh_ = render_->gfmesh();
    static int output_counter = 1;

    glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(3, GL_DOUBLE, 0, gfmesh_->vertex_array());
    if (render_->smooth_ || render_->interpolated_)
    {
        glEnableClientState(GL_NORMAL_ARRAY);
        glNormalPointer(GL_DOUBLE, 0, gfmesh_->vertex_normal_array());
        glDrawElements(GL_TRIANGLES, 3*gfmesh_->face_number(),GL_UNSIGNED_INT, gfmesh_->face_array());
    }
    else
    {
        glDisableClientState(GL_NORMAL_ARRAY);
        glBegin(GL_TRIANGLES);
        for (size_t i=0; i< gfmesh_->face_number(); i++)
        {
                glNormal3d(gfmesh_->face_normal_array()[3*i], gfmesh_->face_normal_array()[3*i+1], gfmesh_->face_normal_array()[3*i+2]);
                VertexIndex v1 = gfmesh_->vertex1_in_face(i);
                VertexIndex v2 = gfmesh_->vertex2_in_face(i);
                VertexIndex v3 = gfmesh_->vertex3_in_face(i);
                glArrayElement(v1);
                glArrayElement(v2);
                glArrayElement(v3);
        }
        glEnd();
    }
    if (render_->to_output_)
    {
        glReadPixels(0,0,render_->width_,render_->height_,GL_RGB, GL_UNSIGNED_BYTE, pixels_);

        //output pgm file contains Y
        std::ostringstream filename_str4;
        filename_str4<<"output"<<output_counter<<".pgm";
        std::ofstream ofs4(filename_str4.str().c_str(), std::ios::binary);
        ofs4<<"P5 "<<render_->width_<<" "<<render_->height_<<" 255"<<std::endl;
        unsigned char* ptr = pixels_;
        for (size_t i = 0; i< size_t(render_->width_*render_->height_); i++)
        {
            unsigned char r = *(ptr++);
            unsigned char g = *(ptr++);
            unsigned char b = *(ptr++);
            unsigned char y = (unsigned char)(0.299*r + 0.587*g + 0.114*b);
            ofs4.write((char*)&y, 1);
        }
        ofs4.close();

        output_counter++;
        render_->to_output_ = false;
    }
}

void disp()
{
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
    if (render_->smooth_)
    {
        glShadeModel(GL_SMOOTH);
    }
    else
    {
        glShadeModel(GL_FLAT);
    }

    glPushMatrix();
    glTranslated(render_->dx_, render_->dy_, render_->dz_);
    glRotated(render_->angle_x_, 1.0, 0.0, 0.0);
    glRotated(render_->angle_y_, 0.0, 1.0, 0.0);
    glRotated(render_->angle_z_, 0.0, 0.0, 1.0);
    glScaled (render_->scale_, render_->scale_, render_->scale_);
    if (render_->fill_)
    {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        draw_surface_with_arrays();
    }
    if (render_->outline_)
    {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        draw_surface_with_arrays();
    }
    glutSwapBuffers();
    glPopMatrix();
}



void reshape(int w, int h)
{
    if (h==0) h = 1;
    if (w==0) w = 1;
    render_->width_ = w;
    render_->height_= h;
    double ratio = 1.0 * w / h;
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glViewport(0, 0 , w , h);

    if (render_->perspective_)
    {
        gluPerspective(render_->view_angle_, ratio, render_->min_distance_, render_->max_distance_);
    }
    else
    {
        glOrtho(render_->left_distance_, render_->right_distance_, render_->bottom_distance_, render_->top_distance_, render_->min_distance_, render_->max_distance_);
    }

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    gluLookAt(render_->view_x_, render_->view_y_, render_->view_z_, render_->view_x_, render_->view_y_, render_->view_z_-2*render_->bounding_length_, 0.0, 1.0, 0.0);
    return;
}

void Render::render_reset()
{
    dx_ = 0;
    dy_ = 0;
    dz_ = 0;
    angle_x_ = 0;
    angle_y_ = 0;
    angle_z_ = 0;
    scale_   = 1;
    mouse_button_ = 0;
    mouse_previous_x_ = 0;
    mouse_previous_y_ = 0;
    mouse_last_x_ = 0;
    mouse_last_y_ = 0;
    smooth_ = false;
    interpolated_ = false;
    perspective_  = true;
    outline_      = false;
    fill_         = true;
}

void keyboard(unsigned char key, int, int)
{
    std::ofstream ofs;
    switch (key)
    {
    case 27:
        sleep(1);
        exit(0);
    case 'R':
    case 'r':
        render_->render_reset();
        break;
    case 'S':
    case 's':
        if (glutGetModifiers() == GLUT_ACTIVE_SHIFT)
        {
            render_->scale_ *= 0.5;
        }
        else
        {
            render_->scale_ *= 2;
        }
        break;
    case 'M':
    case 'm':
        render_->smooth_ = !(render_->smooth_);
        break;
    case 'I':
    case 'i':
        render_->interpolated_ = !(render_->interpolated_);
        break;
    case 'F':
    case 'f':
        render_->fill_   = !(render_->fill_);
        break;
    case 'L':
    case 'l':
        render_->outline_ = !(render_->outline_);
        break;
    case 'P':
    case 'p':
        render_->perspective_ = !(render_->perspective_);
        reshape(render_->width_, render_->height_);
        break;
    case 'O':
    case 'o':
        render_->to_output_ = true;
        break;
    case 'V':
    case 'v':
        ofs.open("view_points", std::ios::app);
        ofs<<render_->dx_<<" "<<render_->dy_<<" "<<render_->dz_<<" "\
        <<render_->angle_x_<<" "<<render_->angle_y_<<" "<<\
        render_->angle_z_<<" "<<render_->scale_<<std::endl;
        ofs.close();
        break;
    default:
        break;
    }
    glutPostRedisplay();
}


void handleSpecial(int key, int, int, int state)
{
    if (state != GLUT_ACTIVE_ALT && state != GLUT_ACTIVE_CTRL)
    {
        switch (key)
        {
        case GLUT_KEY_LEFT:
            render_->angle_y_ -= 10;
            break;
        case GLUT_KEY_RIGHT:
            render_->angle_y_ += 10;
            break;
        case GLUT_KEY_UP:
            render_->dz_ += 0.1 * render_->bounding_length_;
            break;
        case GLUT_KEY_DOWN:
            render_->dz_ -= 0.1 * render_->bounding_length_;
            break;
        default:
            break;
        }
    }
    else if (state == GLUT_ACTIVE_ALT)
    {
        switch (key)
        {
        case GLUT_KEY_LEFT:
            render_->dx_ -= 0.1*render_->bounding_length_;
            break;
        case GLUT_KEY_RIGHT:
            render_->dx_ += 0.1*render_->bounding_length_;
            break;
        case GLUT_KEY_UP:
            render_->dy_ += 0.1*render_->bounding_length_;
            break;
        case GLUT_KEY_DOWN:
            render_->dy_ -= 0.1*render_->bounding_length_;
            break;
        default:
            break;
        }
    }
    else
    {
        switch (key)
        {
        case GLUT_KEY_LEFT:
            render_->angle_z_ += 10;
            break;
        case GLUT_KEY_RIGHT:
            render_->angle_z_ -= 10;
            break;
        case GLUT_KEY_UP:
            render_->angle_x_ -= 10;
            break;
        case GLUT_KEY_DOWN:
            render_->angle_x_ += 10;
            break;
        }
    }
    glutPostRedisplay();
}

void special(int key, int x, int y)
{
    int state = glutGetModifiers();
    handleSpecial(key, x, y, state);
}

void mouse(int button, int state, int x, int y)
{
    if (state == GLUT_DOWN)
    {
        render_->mouse_button_ = button;
        render_->mouse_previous_x_ = x;
        render_->mouse_previous_y_ = y;
        render_->mouse_last_x_ = x;
        render_->mouse_last_y_ = y;
    }
    else if (state == GLUT_UP)
    {
        render_->mouse_button_ = 0;
        render_->mouse_previous_x_ = 0;
        render_->mouse_previous_y_ = 0;
        render_->mouse_last_x_ = 0;
        render_->mouse_last_y_ = 0;
    }
}

void motion(int x, int y)
{
    if (render_->mouse_button_ == GLUT_LEFT_BUTTON)
    {
        render_->angle_y_ += (120 * (x - render_->mouse_previous_x_) / render_->width_);
        render_->angle_x_ += (120 * (y - render_->mouse_previous_y_) / render_->height_);
        render_->mouse_previous_x_ = x;
        render_->mouse_previous_y_ = y;
        
    }
    else if (render_->mouse_button_ == GLUT_RIGHT_BUTTON)
    {
        render_->dz_ += (render_->bounding_length_ * (y - render_->mouse_previous_y_)/render_->height_);
        render_->mouse_previous_y_ = y;
        
    }
    glutPostRedisplay();
}

extern void timer(int value);

void timer(int value)
{
    if (render_->gfmesh()->updated())
    {
        render_->gfmesh()->reset_updated();
        glutPostRedisplay();
    }
    glutTimerFunc(1000/value, timer, value);
}

Render::Render(int& argc, char* argv[], const char* name, Gfmesh* gfmesh, int framerate)
        :gfmesh_(gfmesh), view_angle_(45), \
        min_distance_(0.01), max_distance_(5000),\
        view_x_(0), view_y_(0), view_z_(0), dx_(0), dy_(0), dz_(0), angle_x_(0), \
        angle_y_(0), angle_z_(0), scale_(1), bounding_length_(1), mouse_button_(0),\
        mouse_previous_x_(0), mouse_previous_y_(0),mouse_last_x_(0),mouse_last_y_(0) ,width_(500), height_(500), \
        smooth_(false), interpolated_(false), perspective_(true), \
        outline_(false), fill_(true), 
        to_output_(false) 
{

    Coordinate center_x = 0;
    Coordinate center_y = 0;
    Coordinate center_z = 0;
    Coordinate min_x = 0;
    Coordinate max_x = 0;
    Coordinate min_y = 0;
    Coordinate max_y = 0;
    Coordinate min_z = 0;
    Coordinate max_z = 0;
    Coordinate max_length = 0;

    center_x = min_x = max_x = gfmesh->vertex_array()[0];
    center_y = min_y = max_y = gfmesh->vertex_array()[1];
    center_z = min_z = max_z = gfmesh->vertex_array()[2];
    for (size_t i=1; i < gfmesh->vertex_number(); i++)
    {
        Coordinate x = gfmesh->vertex_array()[3*i];
        Coordinate y = gfmesh->vertex_array()[3*i+1];
        Coordinate z = gfmesh->vertex_array()[3*i+2];

        min_x = min_x > x ? x : min_x;
        max_x = max_x < x ? x : max_x;

        min_y = min_y > y ? y : min_y;
        max_y = max_y < y ? y : max_y;

        min_z = min_z > z ? z : min_z;
        max_z = max_z < z ? z : max_z;
    }
    center_x = (max_x + min_x)/2;
    center_y = (max_y + min_y)/2;
    center_z = (max_z + min_z)/2;

    max_length = max_x - min_x;
    if (max_y - min_y > max_length)
    {
        max_length = max_y - min_y;
    }
    if (max_z - min_z > max_length)
    {
        max_length = max_z - min_z;
    }
    bounding_length_ = max_length;

    double factor = 2;
    left_distance_ = right_distance_ = top_distance_ = bottom_distance_ = factor*max_length*0.5;
    view_x_ = center_x;
    view_y_ = center_y;
    view_z_ = center_z + 1.5*max_length;
    min_distance_ = 0.01 * view_z_;
    max_distance_ = 100 * view_z_;

    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGBA|GLUT_DOUBLE|GLUT_DEPTH);
    glutInitWindowSize(500, 500);
    glutInitWindowPosition(500, 300);
    glutCreateWindow(name);

    glutDisplayFunc(disp);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard);
    glutSpecialFunc(special);	
    glutMouseFunc(mouse);
    glutMotionFunc(motion);
    glutTimerFunc(1000/framerate, timer, framerate);

    glClearColor(0., 0., 0., 0.);

    //GLfloat mat[4];

    //gold
/*
    mat[0] = 0.24725;
    mat[1] = 0.1995;
    mat[2] = 0.0745;
    mat[3] = 1.0;
    glMaterialfv(GL_FRONT, GL_AMBIENT, mat);
    mat[0] = 0.75164;
    mat[1] = 0.60648;
    mat[2] = 0.22648;
    glMaterialfv(GL_FRONT, GL_DIFFUSE, mat);
    mat[0] = 0.628281;
    mat[1] = 0.555802;
    mat[2] = 0.366065;
    GLfloat shine = 0.8;
    glMaterialfv(GL_FRONT, GL_SPECULAR, mat);
    glMaterialf(GL_FRONT, GL_SHININESS, shine * 128.0);
*/

    //bronze
/*
    mat[0] = 0.2125;
    mat[1] = 0.1275;
    mat[2] = 0.054;
    mat[3] = 1.0;
    glMaterialfv(GL_FRONT, GL_AMBIENT, mat);
    mat[0] = 0.714;
    mat[1] = 0.4284;
    mat[2] = 0.18144;
    glMaterialfv(GL_FRONT, GL_DIFFUSE, mat);
    mat[0] = 0.393548;
    mat[1] = 0.271906;
    mat[2] = 0.166721;
    GLfloat shine = 0.2;
    glMaterialfv(GL_FRONT, GL_SPECULAR, mat);
    glMaterialf(GL_FRONT, GL_SHININESS, shine * 128.0);

    GLfloat light_position[] = {0.0, 0.0, 1.0, 0.0};
    GLfloat light_position2[] = {1.0, 1.0, 1.0, 0.0};
    GLfloat light_position3[] = {12.0, -1.0, -10.0, 0.0};
    glLightfv(GL_LIGHT0, GL_POSITION, light_position);
    glLightfv(GL_LIGHT1, GL_POSITION, light_position2);
    glLightfv(GL_LIGHT1, GL_POSITION, light_position3);
    */

    //normal
    GLfloat mat_specular[] = {0.3, 0.3, 0.3, 0.0};
    GLfloat mat_shininess[]  = {100};
    GLfloat light_position[] = {0.0, 0.0, 1.0, 0.0};
    glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
    glMaterialfv(GL_FRONT, GL_SHININESS,mat_shininess);
    glLightfv(GL_LIGHT0, GL_POSITION, light_position);
    
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_NORMALIZE);

    render_ = this;
    std::cerr<<view_x_<<" "<<view_y_<<" "<<view_z_<<" "<<std::endl;
    std::cerr<<bounding_length_<<std::endl;
}

void Render::enterMainLoop()
{
    glutMainLoop();
}
