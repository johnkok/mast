#include "graphics.hpp"
#include "mast.hpp"
#include "drawtext.h"
using namespace std;

// Global data
extern cmast Mast;

// Data
float windDirection[WIND_DEPTH];
float windSpeed[2];
float vmg[360/5];
float gyro[3];
float mastLocation[3];
 
// Textures
GLUquadric *earth;
GLuint earthTexture;
GLUquadric *bg;
GLuint bgTexture;
GLUquadric *panel;
GLuint panelTexture;

graphics Graphics;

struct dtx_font *font18;
struct dtx_font *font24;
struct dtx_font *font28;
struct dtx_font *font58;

/**
 * @brief LoadBitmap
 * @details Load bitmap texture
 * @param filename
 * @return
 */
int LoadBitmap(char *filename)
{
    FILE * file;
    char temp;
    long i;
 
    // own version of BITMAPINFOHEADER from windows.h for Linux compile
    struct {
      int biWidth;
      int biHeight;
      short int biPlanes;
      unsigned short int biBitCount;
      unsigned char *data;
    } infoheader;
 
    GLuint num_texture;

    if( (file = fopen(filename, "rb"))==NULL)
        return (-1); // Open the file for reading
 
    fseek(file, 18, SEEK_CUR);  /* start reading width & height */
    fread(&infoheader.biWidth, sizeof(int), 1, file);
 
    fread(&infoheader.biHeight, sizeof(int), 1, file);
 
    fread(&infoheader.biPlanes, sizeof(short int), 1, file);
    if (infoheader.biPlanes != 1) {
        printf("Planes from %s is not 1: %u\n", filename, infoheader.biPlanes);
        return 0;
    }
 
    // read the bpp
    fread(&infoheader.biBitCount, sizeof(unsigned short int), 1, file);
    if (infoheader.biBitCount != 24) {
        printf("Bpp from %s is not 24: %d\n", filename, infoheader.biBitCount);
        return 0;
    }
 
    fseek(file, 24, SEEK_CUR);
 
    // read the data
    if(infoheader.biWidth<0){
        infoheader.biWidth = -infoheader.biWidth;
    }
    if(infoheader.biHeight<0){
        infoheader.biHeight = -infoheader.biHeight;
    }
    infoheader.data = (unsigned char *) malloc(infoheader.biWidth * infoheader.biHeight * 3);
    if (infoheader.data == NULL) {
        printf("Error allocating memory for color-corrected image data\n");
        return 0;
    }
 
    if ((i = fread(infoheader.data, infoheader.biWidth * infoheader.biHeight * 3, 1, file)) != 1) {
        printf("Error reading image data from %s.\n", filename);
        return 0;
    }
 
    for (i=0; i<(infoheader.biWidth * infoheader.biHeight * 3); i+=3) { // reverse all of the colors. (bgr -> rgb)
        temp = infoheader.data[i];
        infoheader.data[i] = infoheader.data[i+2];
        infoheader.data[i+2] = temp;
    }
 
    fclose(file);
 
    glGenTextures(1, &num_texture);
    glBindTexture(GL_TEXTURE_2D, num_texture);
 
    // The next commands sets the texture parameters
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT); // If the u,v coordinates overflow the range 0,1 the image is repeated
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); // The magnification function ("linear" produces better results)
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST); //The minifying function
 
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
 
    // Finally we define the 2d texture
    glTexImage2D(GL_TEXTURE_2D, 0, 3, infoheader.biWidth, infoheader.biHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, infoheader.data);
 
    // And create 2d mipmaps for the minifying function
    gluBuild2DMipmaps(GL_TEXTURE_2D, 3, infoheader.biWidth, infoheader.biHeight, GL_RGB, GL_UNSIGNED_BYTE, infoheader.data);
 
    free(infoheader.data);

    return (num_texture);
}
 
/**
 * @brief keyboard
 * @details handle keyboard input
 * @param key
 * @param x
 * @param y
 */
void keyboard(unsigned char key, int x, int y)
{
    switch (key) {
        case 27: exit(0); break;
        case 'a': gyro[0]+=5;
        Mast.update_wind(Mast.get_awd()+5,0.0);
        break;
        case 'A': gyro[0]-=5;
        Mast.update_wind(Mast.get_awd()-5,0.0);
        break;
        case 's': gyro[1]+=5; break;
        case 'S': gyro[1]-=5; break;
        case 'd': gyro[2]+=5; break;
        case 'D': gyro[2]-=5; break;
        default: break;
    }
    glutPostRedisplay();
}
 
/**
 * @brief draw_gyro
 * @param x
 * @param y
 * @param z
 */
void draw_gyro(float x_pos, float y_pos, float z_pos)
{
    int y = 0;

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    // Load gyro
    glBindTexture ( GL_TEXTURE_2D, earthTexture);

    //draw textured sphere
    glPushMatrix();
        // Reset color
        glColor3f(1.0f, 1.0f, 1.0f);
        glTranslatef(x_pos, y_pos, z_pos);
        glRotatef(gyro[2],0.0,0.0,1.0);
        glRotatef(gyro[1],0.0,1.0,0.0);
        glRotatef(gyro[0] - 90.0,1.0,0.0,0.0);
        gluSphere( earth, 0.25, 40, 40);
    glPopMatrix();

    glDisable ( GL_TEXTURE_2D );

    // gyro scale
    glPushMatrix();                     // Save model-view matrix setting
        glTranslatef(x_pos, y_pos, z_pos + 0.225f);     // Translate
        glRotatef(gyro[2], 0.0f, 0.0f, 1.0f); // rotate by angle in degrees
        glBegin(GL_QUADS);
            // Horizontal index
            glColor3f(5.0f, 5.0f, 5.0f);
            glVertex2f(-0.35 , -0.005);
            glVertex2f(0.35 , -0.005);
            glVertex2f(0.35, 0.005);
            glVertex2f(-0.35, 0.005);

            // Minor
            glColor3f(3.0f, 1.0f, 1.0f);
            for (y = 0; y < 360 ; y += 30){
                glVertex2f(cos(2 * M_PI * (y - 1) / 360)*0.25 , sin(2 * M_PI * (y - 1) / 360)*0.25);
                glVertex2f(cos(2 * M_PI * (y - 1) / 360)*0.31 , sin(2 * M_PI * (y - 1) / 360)*0.31);
                glVertex2f(cos(2 * M_PI * (y + 1) / 360)*0.31 , sin(2 * M_PI * (y + 1) / 360)*0.31);
                glVertex2f(cos(2 * M_PI * (y + 1) / 360)*0.25 , sin(2 * M_PI * (y + 1) / 360)*0.25);
            }

            // Major
            glColor3f(3.0f, 0.0f, 0.0f);
            for (int y = 0; y < 360 ; y += 5){
                glVertex2f(cos(2 * M_PI * (y - 1) / 360)*0.26 , sin(2 * M_PI * (y - 1) / 360)*0.26);
                glVertex2f(cos(2 * M_PI * (y - 1) / 360)*0.29 , sin(2 * M_PI * (y - 1) / 360)*0.29);
                glVertex2f(cos(2 * M_PI * (y + 1) / 360)*0.29 , sin(2 * M_PI * (y + 1) / 360)*0.29);
                glVertex2f(cos(2 * M_PI * (y + 1) / 360)*0.26 , sin(2 * M_PI * (y + 1) / 360)*0.26);
            }

            // Cross
            glColor3f(5.0f, 1.0f, 1.0f);
            for (int y = 0; y < 360 ; y += 90){
                glVertex2f(cos(2 * M_PI * (y - 2) / 360)*0.25 , sin(2 * M_PI * (y - 2) / 360)*0.25);
                glVertex2f(cos(2 * M_PI * (y - 2) / 360)*0.33 , sin(2 * M_PI * (y - 2) / 360)*0.33);
                glVertex2f(cos(2 * M_PI * (y + 2) / 360)*0.33 , sin(2 * M_PI * (y + 2) / 360)*0.33);
                glVertex2f(cos(2 * M_PI * (y + 2) / 360)*0.25 , sin(2 * M_PI * (y + 2) / 360)*0.25);
            }

        glEnd();
    glPopMatrix();                      // Restore the model-view matrix

    // X rotation index (fixed index)
    glPushMatrix();                     // Save model-view matrix setting
        glColor3f(5.0f, 5.0f, 5.0f);
        glTranslatef(-0.5f, 0.0f, 0.0f);
        glRotatef(0, 0.0f, 0.0f, 1.0f);
        glBegin(GL_TRIANGLE_FAN);
            glVertex2f( 0.0f, 0.33f);
            glVertex2f( 0.03f, 0.38f);
            glVertex2f( 0.04f, 0.43f);
            glVertex2f( 0.f, 0.41f);
            glVertex2f( -0.04f, 0.43f);
            glVertex2f( -0.03f, 0.38f);
        glEnd();
    glPopMatrix();                      // Restore the model-view matrix
}

/**
 * @brief mast_position
 * @param x_pos
 * @param y_pos
 * @param z_pos
 */
void mast_position(float x_pos, float y_pos, float z_pos)
{
    float x2,y2,cx,cy,fx,fy;
    float angle = 0;
    float radius = 0.3;
    int i;

    // Mast position background
    glPushMatrix();
        glTranslatef(x_pos, y_pos, z_pos);
        glBegin(GL_TRIANGLE_FAN);
            glColor3f(4.0f, 4.0f, 4.0f);
            glVertex2f( 0.0f, 0.0f);
            for (i=0;i<=360;i+=5) {
                if (i%90) {
                    glColor3f(4.0f, 4.0f, 4.0f); // RED
                }else {
                    glColor3f(0.0f, 0.0f, 0.0f); // RED
            }
            glVertex2f(cos(2 * M_PI * (i + 90) / 360)*0.2, sin(2 * M_PI * (i + 90) / 360)*0.2);
        }
        glEnd();
    glPopMatrix();

    // Load mast position
    glPushMatrix();
        glTranslatef(x_pos, y_pos, z_pos);
        glColor3f(5.0f, 0.0f, 0.0f); // RED
        glutSolidSphere( 0.02, 25, 25);
        glTranslatef(-0.5,0,0);
    glPopMatrix();
}

/**
 * @brief vmg
 * @param x_pos
 * @param y_pos
 * @param z_pos
 */
void vmg_plot(float x_pos, float y_pos, float z_pos)
{
    float x2,y2,cx,cy,fx,fy;
    float angle = 0;
    float radius = 0.3;
    int i;

    // Vmg
    glPushMatrix();                     // Save model-view matrix setting
        glTranslatef(x_pos, y_pos, z_pos);     // Translate
        glBegin(GL_QUAD_STRIP);
            glColor3f(0.0f, 0.0f, 5.0f);
            for (i=0;i<=360/5;i++) {
                glVertex2f(cos(2 * M_PI * i * 5 / 360) * 0.21 , sin(2 * M_PI * i * 5 / 360) * 0.21);
                glVertex2f(cos(2 * M_PI * i * 5 / 360) * 0.295 , sin(2 * M_PI * i *  5 / 360) * 0.295);
            }
        glEnd();
    glPopMatrix();
}

/**
 * @brief wind_variation
 * @param x_pos
 * @param y_pos
 * @param z_pos
 */
void wind_variation(float x_pos, float y_pos, float z_pos)
{
    int i;
    float green = 1.0;
    float red = 0.0;
    float min = Mast.get_wind_var_low();
    float max = Mast.get_wind_var_high();
    int cache = 0;
    float x2,y2,cx,cy,fx,fy;
    float radius = 0.3;
    float angle = 0;
    int y;

    // wind angle cicle
    glPushMatrix();                     // Save model-view matrix setting
        glTranslatef(x_pos, y_pos, z_pos);     // Translate
        radius = 0.305;
        cache = 0;
        glColor3f(2.2f, 2.2f, 3.0f); // White
        glBegin(GL_LINES);
            for (angle = 0; angle < 360; angle+=1.0) {
                float rad_angle = angle * 3.14 / 180;
                x2 = radius * sin((double)rad_angle);
                y2 = radius * cos((double)rad_angle);
                if (cache) { // Line from previus to current
                    glVertex2f(cx,cy);
                    glVertex2f(x2,y2);
                } else { // Store first point
                    fx = x2;
                    fy = y2;
                }
                cache = 1;
                cx = x2;
                cy = y2;
            }
            glVertex2f(x2,y2); // line from last to firsts
            glVertex2f(fx,fy);
        glEnd();
    glPopMatrix();                      // Restore the model-view matrix

    // wind scale
    glPushMatrix();                     // Save model-view matrix setting
        glTranslatef(x_pos, y_pos, z_pos + 0.225f);     // Translate
        glBegin(GL_QUADS);
            // Minor
            glColor3f(2.2f, 2.2f, 3.0f);
            for (y = 0; y < 360 ; y += 30){
                glVertex2f(cos(2 * M_PI * (y - 0.7) / 360)*0.305 , sin(2 * M_PI * (y - 0.7) / 360)*0.305);
                glVertex2f(cos(2 * M_PI * (y - 0.7) / 360)*0.35 , sin(2 * M_PI * (y - 0.7) / 360)*0.35);
                glVertex2f(cos(2 * M_PI * (y + 0.7) / 360)*0.35 , sin(2 * M_PI * (y + 0.7) / 360)*0.35);
                glVertex2f(cos(2 * M_PI * (y + 0.7) / 360)*0.305 , sin(2 * M_PI * (y + 0.7) / 360)*0.305);
            }
            for (y = 0; y < 360 ; y += 15){
                glVertex2f(cos(2 * M_PI * (y - 0.5) / 360)*0.305 , sin(2 * M_PI * (y - 0.5) / 360)*0.305);
                glVertex2f(cos(2 * M_PI * (y - 0.5) / 360)*0.34 , sin(2 * M_PI * (y - 0.5) / 360)*0.34);
                glVertex2f(cos(2 * M_PI * (y + 0.5) / 360)*0.34 , sin(2 * M_PI * (y + 0.5) / 360)*0.34);
                glVertex2f(cos(2 * M_PI * (y + 0.5) / 360)*0.305 , sin(2 * M_PI * (y + 0.5) / 360)*0.305);
            }
            for (y = 0; y < 360 ; y += 5){
                glVertex2f(cos(2 * M_PI * (y - 0.3) / 360)*0.305 , sin(2 * M_PI * (y - 0.3) / 360)*0.305);
                glVertex2f(cos(2 * M_PI * (y - 0.3) / 360)*0.32 , sin(2 * M_PI * (y - 0.3) / 360)*0.32);
                glVertex2f(cos(2 * M_PI * (y + 0.3) / 360)*0.32 , sin(2 * M_PI * (y + 0.3) / 360)*0.32);
                glVertex2f(cos(2 * M_PI * (y + 0.3) / 360)*0.305 , sin(2 * M_PI * (y + 0.3) / 360)*0.305);
            }
        glEnd();
    glPopMatrix();                      // Restore the model-view matrix

    // Wearther - wind variation
    glPushMatrix();                     // Save model-view matrix setting
        glTranslatef(x_pos, y_pos, z_pos);     // Translate
//      glRotatef(anglex, 0.0f, 0.0f, 1.0f); // rotate by angle in degrees
        glBegin(GL_QUAD_STRIP);
            if (min > max) {
                max += 360;
            }
            for (i = min ; i < max ; i++) {

                if (abs(i - Mast.get_awd()) > 10) {
                    if (i > (max-10)) {
                        green = green - 0.5;
                    }
                    else{
                        if (green < 5.0) green = green + 0.5;
                    }
                }
                else {
                    green = 5.0;
                }
                if (abs(i - Mast.get_awd()) < 2) {
                    red = 5.0;
                }else{
                    red = 0.0;
                }

                glColor3f(red, green, 0.0f); // Green

                glVertex2f(cos(2 * M_PI * (i + 90) / 360)*0.31 , sin(2 * M_PI * (i + 90) / 360)*0.31);
                glVertex2f(cos(2 * M_PI * (i + 90) / 360)*0.35 , sin(2 * M_PI * (i + 90) / 360)*0.35);
            }
        glEnd();
    glPopMatrix();                      // Restore the model-view matrix
}

/**
 * @brief north
 * @param x_pos
 * @param y_pos
 * @param z_pos
 */
void north(float x_pos, float y_pos, float z_pos)
{
    // North
    float heading_f = Mast.get_cog();

    glPushMatrix();                     // Save model-view matrix setting
        glTranslatef(x_pos, y_pos, z_pos);     // Translate
        glRotatef(heading_f, 0.0f, 0.0f, 1.0f); // rotate by angle in degrees
        glBegin(GL_TRIANGLES);
            glColor3f(5.0f, 5.0f, 5.0f); // RED

            glVertex2f( -0.05f, 0.49f);
            glVertex2f( 0.0f, 0.48f);
            glVertex2f( 0.0f, 0.5f);

            glVertex2f( -0.01f, 0.49f);
            glVertex2f( -0.0f, 0.44f);
            glVertex2f( 0.01f, 0.49f);

            glVertex2f( 0.01f, 0.48f);
            glVertex2f( -0.0f, 0.54f);
            glVertex2f( -0.01f, 0.49f);

            glVertex2f( 0.0f, 0.48f);
            glVertex2f( 0.05f, 0.49f);
            glVertex2f( 0.0f, 0.5f);
        glEnd();
    glPopMatrix();
}

/**
 * @brief wind_head
 * @param x_pos
 * @param y_pos
 * @param z_pos
 */
void wind_head(float x_pos, float y_pos, float z_pos)
{
    char buffer[32];

    // Wearther - aparent wind direction
    glPushMatrix();                     // Save model-view matrix setting
        glTranslatef(x_pos, y_pos, z_pos+0.2);     // Translate
        glRotatef(Mast.get_awd(), 0.0f, 0.0f, 1.0f); // rotate by angle in degrees
        glBegin(GL_TRIANGLE_FAN);
            glColor3f(5.0f, 0.0f, 0.0f); // RED
            glVertex2f( 0.0f, 0.31f);
            glVertex2f( 0.03f, 0.35f);
            glVertex2f( 0.04f, 0.4f);
            glVertex2f( 0.f, 0.38f);
            glVertex2f( -0.04f, 0.4f);
            glVertex2f( -0.03f, 0.35f);
        glEnd();
    glPopMatrix();                      // Restore the model-view matrix

    // Wearther - true wind direction
    glPushMatrix();                     // Save model-view matrix setting
        glTranslatef(x_pos, y_pos, z_pos+0.1);     // Translate
        glRotatef(Mast.get_twd(), 0.0f, 0.0f, 1.0f); // rotate by angle in degrees
        glBegin(GL_TRIANGLE_FAN);
            glColor3f(1.0f, 2.5f, 0.0f); // RED
            glVertex2f( 0.0f, 0.31f);
            glVertex2f( 0.03f, 0.35f);
            glVertex2f( 0.04f, 0.4f);
            glVertex2f( 0.f, 0.38f);
            glVertex2f( -0.04f, 0.4f);
            glVertex2f( -0.03f, 0.35f);
        glEnd();
    glPopMatrix();

//    glDisable(GL_DEPTH_TEST);
    glDisable(GL_LIGHTING);

   for (float y = 0; y < 360 ; y += 30){
        glPushMatrix();                     // Save model-view matrix setting
            char buffer[8];
            float a_cos = cos(2 * M_PI * (y + 90.0) / 360.0);
            float a_sin = sin(2 * M_PI * (y + 90.0) / 360.0);

            glColor3f(0.7f, 0.7f, 1.0f); // RED
            glTranslatef(x_pos + 1.0 + 0.35 * a_cos - 0.04 * a_sin, y_pos + 1.0 + 0.35 * a_sin + 0.04 * a_cos, z_pos + 0.0);
            gluOrtho2D(0, 800, 0, 800);
            glRotatef(y*1.0, 0.0f, 0.0f, 1.0f);
            dtx_use_font(font18, 18);
	    if (y > 180) {
                sprintf(buffer, "%3.0f", -(y-360));
	    } 
	    else {
                sprintf(buffer, "%3.0f", y);
	    }
            dtx_string(buffer);
        glPopMatrix();                      // Restore the model-view matrix
    }

    glPushMatrix();                     // Save model-view matrix setting
        float w_cos = cos(2 * M_PI * (Mast.get_awd() + 90) / 360);
        float w_sin = sin(2 * M_PI * (Mast.get_awd() + 90) / 360);

        glColor3f(2.0f, 2.0f, 2.0f); // RED
        glTranslatef(x_pos + 1.0 + 0.33 * w_cos - 0.022 * w_sin, y_pos + 1.0 + 0.33 * w_sin + 0.022 * w_cos, z_pos + 1.0);
        gluOrtho2D(0, 800, 0, 800);
        glRotatef(Mast.get_awd(), 0.0f, 0.0f, 1.0f);
        dtx_use_font(font24, 24);
        dtx_string("A");
    glPopMatrix();                      // Restore the model-view matrix

    glPushMatrix();                     // Save model-view matrix setting
        float t_cos = cos(2 * M_PI * (Mast.get_twd() + 90) / 360);
        float t_sin = sin(2 * M_PI * (Mast.get_twd() + 90) / 360);

        glColor3f(2.0f, 2.0f, 2.0f); // RED
        glTranslatef(x_pos + 1.0 + 0.33 * t_cos - 0.022 * t_sin, y_pos + 1.0 + 0.33 * t_sin + 0.022 * t_cos, z_pos + 0.2);
        gluOrtho2D(0, 800, 0, 800);
        glRotatef(Mast.get_twd(), 0.0f, 0.0f, 1.0f);
        dtx_use_font(font24, 24);
        dtx_string("T");
    glPopMatrix();                      // Restore the model-view matrix

    glPushMatrix();                     // Save model-view matrix setting
        float a_cos = cos(2 * M_PI * (Mast.get_awd() + 90) / 360);
        float a_sin = sin(2 * M_PI * (Mast.get_awd() + 90) / 360);

        glColor3f(2.0f, 2.0f, 2.0f); // RED
        glTranslatef(x_pos + 1.0 + 0.395 * a_cos - 0.08 * a_sin, y_pos + 1.0 + 0.395 * a_sin + 0.08 * a_cos, z_pos + 0.3);
        gluOrtho2D(0, 800, 0, 800);
        glRotatef(Mast.get_awd(), 0.0f, 0.0f, 1.0f);
        dtx_use_font(font24, 24);
        sprintf(buffer, "%1.1fKn", Mast.get_aws());
        dtx_string(buffer);
    glPopMatrix();                      // Restore the model-view matrix
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
}

/**
 * @brief print_status
 */
void print_status(void)
{
    char buffer[128];

    glPushMatrix();                     // Save model-view matrix setting
        glEnable ( GL_TEXTURE_2D );
        glBindTexture ( GL_TEXTURE_2D, panelTexture);
        glBegin(GL_QUADS);
            glTexCoord2f(0.03, 0.0); glVertex2f( 1.0f, -0.55f);
            glTexCoord2f(0.03, 1.0); glVertex2f( 1.0f, 0.55f);
            glTexCoord2f(1.03, 1.0); glVertex2f( -1.0f, 0.55f);
            glTexCoord2f(1.03, 0.0); glVertex2f( -1.0f, -0.55f);
        glEnd();
        glDisable( GL_TEXTURE_2D );
    glPopMatrix();

    glPushMatrix();                     // Save model-view matrix setting
        glEnable ( GL_TEXTURE_2D );
        glBindTexture ( GL_TEXTURE_2D, bgTexture);
        glBegin(GL_QUADS);
            glTexCoord2f(1.03, 0.0); glVertex2f( 1.0f, 0.55f);
            glTexCoord2f(1.03, 1.0); glVertex2f( 1.0f, 1.0f);
            glTexCoord2f(0.03, 1.0); glVertex2f( -1.0f, 1.0f);
            glTexCoord2f(0.03, 0.0); glVertex2f( -1.0f, 0.55f);
        glEnd();
        glDisable( GL_TEXTURE_2D );
    glPopMatrix();                      // Restore the model-view matrix

    glPushMatrix();                     // Save model-view matrix setting
        glEnable ( GL_TEXTURE_2D );
        glBindTexture ( GL_TEXTURE_2D, bgTexture);
        glBegin(GL_QUADS);
            glTexCoord2f(0.03, 0.0); glVertex2f( 1.0f, -1.0f);
            glTexCoord2f(0.03, 1.0); glVertex2f( 1.0f, -0.55f);
            glTexCoord2f(1.03, 1.0); glVertex2f( -1.0f, -0.55f);
            glTexCoord2f(1.03, 0.0); glVertex2f( -1.0f, -1.0f);
        glEnd();
        glDisable( GL_TEXTURE_2D );
    glPopMatrix();                      // Restore the model-view matrix


    glDisable(GL_DEPTH_TEST);
    glDisable(GL_LIGHTING);

    glPushMatrix();                     // Save model-view matrix setting
        glColor3f(2.0f, 0.0f, 0.0f);
        gluOrtho2D(0, 800, 0, 800);
        glTranslatef(340.0, 50.0, 0.1);
        dtx_use_font(font58, 58);
	if (Mast.get_cog() >= 10.0) {
            snprintf(buffer, 6, "%2.1f.", Mast.get_cog());	
	}
	else if (Mast.get_cog() >= 100.0) {
            snprintf(buffer, 6, "%3.0f", Mast.get_cog());	
	}
	else {
            snprintf(buffer, 6, "%1.2f", Mast.get_cog());	
	}
        dtx_string(buffer);
    glPopMatrix();                      // Restore the model-view matrix

    glPushMatrix();                     // Save model-view matrix setting
        glColor3f(2.0f, 0.0f, 0.0f);
        gluOrtho2D(0, 800, 0, 800);
        glTranslatef(340.0, 670.0, 0.1);
        dtx_use_font(font58, 58);
        if (Mast.get_sog() >= 10.0) {
            snprintf(buffer, 6, "%2.1f.", Mast.get_sog());
        }
        else if (Mast.get_sog() >= 100.0) {
            snprintf(buffer, 6, "%3.0f", Mast.get_sog());
        }
        else {
            snprintf(buffer, 6, "%1.2f", Mast.get_sog());
        }
        dtx_string(buffer);
    glPopMatrix();                      // Restore the model-view matrix

    glPushMatrix();                     // Save model-view matrix setting
        glColor3f(0.0f, 0.0f, 0.0f);
        gluOrtho2D(0, 800, 0, 800);
        glTranslatef(98.0, 750.0, 0.1);
        dtx_use_font(font28, 28);
        dtx_string("Position");
    glPopMatrix();

    glPushMatrix();                     // Save model-view matrix setting
        glColor3f(0.0f, 0.0f, 0.0f);
        gluOrtho2D(0, 800, 0, 800);
        glTranslatef(375.0, 750.0, 0.1);
        dtx_use_font(font28, 28);
        dtx_string("SOG");
    glPopMatrix();

    glPushMatrix();                     // Save model-view matrix setting
        glColor3f(0.0f, 0.0f, 0.0f);
        gluOrtho2D(0, 800, 0, 800);
        glTranslatef(375.0, 130.0, 0.1);
        dtx_use_font(font28, 28);
        dtx_string("COG");
    glPopMatrix();

    glPushMatrix();                     // Save model-view matrix setting
        glColor3f(0.0f, 0.0f, 0.0f);
        gluOrtho2D(0, 800, 0, 800);
        glTranslatef(570.0, 750.0, 0.1);
        dtx_use_font(font28, 28);
        dtx_string("Wind (T/A)");
    glPopMatrix();

    glPushMatrix();                     // Save model-view matrix setting
        glColor3f(0.0f, 0.0f, 0.0f);
        gluOrtho2D(0, 800, 0, 800);
        glTranslatef(95.0, 715.0, 0.1);
        dtx_use_font(font24, 24);
        dtx_string((char *)Mast.get_longitude().c_str());
    glPopMatrix();

    glPushMatrix();                     // Save model-view matrix setting
        glColor3f(0.0f, 0.0f, 0.0f);
        gluOrtho2D(0, 800, 0, 800);
        glTranslatef(90.0, 685.0, 0.1);
        dtx_use_font(font24, 24);
        dtx_string((char *)Mast.get_lattitude().c_str());
    glPopMatrix();

    glPushMatrix();                     // Save model-view matrix setting
        glColor3f(0.3f, 0.3f, 0.5f);
        gluOrtho2D(0, 800, 0, 800);
        glTranslatef(105.0, 655.0, 0.1);
        dtx_use_font(font24, 24);
        dtx_string((char *)Mast.get_time().c_str());
    glPopMatrix();


    glPushMatrix();                     // Save model-view matrix setting
        glColor3f(0.0f, 0.0f, 0.0f);
        gluOrtho2D(0, 800, 0, 800);
        glTranslatef(542.0, 715.0, 0.1);
        dtx_use_font(font24, 24);
        snprintf(buffer, sizeof(buffer), "xWS: %2.1f / %2.1fKn", Mast.get_tws(), Mast.get_aws());
        dtx_string(buffer);
    glPopMatrix();

    glPushMatrix();                     // Save model-view matrix setting
        glColor3f(0.0f, 0.0f, 0.0f);
        gluOrtho2D(0, 800, 0, 800);
        glTranslatef(542.0, 685.0, 0.1);
        dtx_use_font(font24, 24);
        snprintf(buffer, sizeof(buffer), "xWA: %2.1f / %2.1fKn", Mast.get_twa(), Mast.get_awa());
        dtx_string(buffer);
    glPopMatrix();

    glPushMatrix();                     // Save model-view matrix setting
        glColor3f(0.0f, 0.0f, 0.0f);
        gluOrtho2D(0, 800, 0, 800);
        glTranslatef(542.0, 655.0, 0.1);
        dtx_use_font(font24, 24);
        snprintf(buffer, sizeof(buffer), "xWD: %3.0f / %3.0f\u00B0", Mast.get_twd(), Mast.get_awd());
        dtx_string(buffer);
    glPopMatrix();


    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
}

/**
 * @brief display
 */
void display(void)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
    glEnable ( GL_TEXTURE_2D );

    // black circle
    glPushMatrix();                     // Save model-view matrix setting
        glTranslatef(0.5, 0.0, 0.0);     // Translate
        glBegin(GL_QUAD_STRIP);
            glColor3f(0.5f, 0.5f, 0.5f);
            for (int i=0;i<=360/5;i++) {
                glVertex2f(cos(2 * M_PI * i * 5 / 360) * 0.35 , sin(2 * M_PI * i * 5 / 360) * 0.35);
                glVertex2f(cos(2 * M_PI * i * 5 / 360) * 0.45 , sin(2 * M_PI * i *  5 / 360) * 0.45);
            }
        glEnd();
    glPopMatrix();

    draw_gyro(-0.5, 0.0, 0.0);

    mast_position(0.5, 0.0, 0.0);

    vmg_plot(0.5, 0.0, 0.0);

    wind_variation(0.5, 0.0, 0.0);

    wind_head(0.5, 0.0, 0.1);

    north(0.5, 0.0, 0.0);

    print_status();

    // End of drawing
    glutSwapBuffers();
}
 
/**
 * @brief graphics_init
 */
void graphics_init (void)
{
    glClearColor(0.1f, 0.1f, 0.1f, 0.0f);
 
    glEnable(GL_DEPTH_TEST);
 
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(-1.0, 1.0, -1.0, 1.0, -2.0, 2.0);
 
    earth = gluNewQuadric();
    bg = gluNewQuadric();
    panel = gluNewQuadric();
    gluQuadricTexture( earth, GL_TRUE);
    gluQuadricTexture( bg, GL_TRUE);
    gluQuadricTexture( panel, GL_TRUE);
    earthTexture = LoadBitmap((char *)"images/gyro.bmp");
    bgTexture = LoadBitmap((char *)"images/bg2.bmp");
    panelTexture = LoadBitmap((char *)"images/bg4.bmp");
}

void refresh(int msec)
{
    glutPostRedisplay();
    glutTimerFunc(msec, refresh, msec);
}

/**
 * @brief graphics_main
 * @param argc
 * @param argv
 * @return
 */
int graphics::graphics_main(int argc, char** argv)
{
    int i;
   
    cout << " *  Starting GUI" << endl;

    glutInit(&argc, argv);
    glutInitDisplayMode (GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(800, 800);
    glutCreateWindow("Mast tracking");

    graphics_init ();
    glutDisplayFunc(display);
    glutKeyboardFunc(keyboard);

    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    glEnable(GL_LIGHT0);
    glEnable(GL_NORMALIZE);
    glEnable(GL_COLOR_MATERIAL);
    glEnable(GL_LIGHTING);

    glLightfv(GL_LIGHT0, GL_AMBIENT,  light_ambient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE,  light_diffuse);
    glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular);
    glLightfv(GL_LIGHT0, GL_POSITION, light_position);

    glMaterialfv(GL_FRONT, GL_AMBIENT,   mat_ambient);
    glMaterialfv(GL_FRONT, GL_DIFFUSE,   mat_diffuse);
    glMaterialfv(GL_FRONT, GL_SPECULAR,  mat_specular);
    glMaterialfv(GL_FRONT, GL_SHININESS, high_shininess);

    // Initialize LibDrawText fonts
    if(!(font18 = dtx_open_font("fonts/UbuntuMono-B.ttf", 18))) {
        fprintf(stderr, "failed to open font\n");
        return 1;
    }
    dtx_use_font(font18, 18);

    // Initialize LibDrawText fonts
    if(!(font24 = dtx_open_font("fonts/UbuntuMono-B.ttf", 24))) {
        fprintf(stderr, "failed to open font\n");
        return 1;
    }
    dtx_use_font(font24, 24);

    // Initialize LibDrawText fonts
    if(!(font28 = dtx_open_font("fonts/UbuntuMono-B.ttf", 28))) {
        fprintf(stderr, "failed to open font\n");
        return 1;
    }
    dtx_use_font(font28, 28);

    // Initialize LibDrawText fonts
    if(!(font58 = dtx_open_font("fonts/UbuntuMono-B.ttf", 58))) {
        fprintf(stderr, "failed to open font\n");
        return 1;
    }
    dtx_use_font(font58, 58);

    /* Refresh timer */
    glutTimerFunc(1000/REFRESH_RATE, refresh, 1000/REFRESH_RATE);

    glutMainLoop();

    return 0;
}
