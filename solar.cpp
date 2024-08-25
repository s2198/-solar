#define _CRT_SECURE_NO_WARNINGS
#pragma comment(lib, "legacy_stdio_definitions.lib")
#pragma warning(disable:4996)
#include <Windows.h>
#include <glut.h>           
#include <gl.h>            
#include <glu.h>
#include <glaux.h> 
#include <stdio.h>
#include <math.h>

static int Day[9] = { 34,223,56,123,333,154,20,170,280 }; // �����ֱ�
static int Time = 0; //���� �ֱ�

float ele = -4.0f, azi = 0.0f, rad = 0.0f, twi = 0.0f; //����ǥ�� ���� ����
float zoom = -2.5f;

bool flag = true;// �ִϸ��̼� ON / OFF ���� ����
GLUquadricObj* obj;//�伺���� ��������� quadrics

const GLfloat light_pos[] = { 0.0, 0.0, 0.0, 1.0 }; //���� ��ġ�� �����ִ� ����
GLfloat const_att = 1.0;

GLuint tex[11]; //�ؽ��ĸ� ������ ����

AUX_RGBImageRec* LoadBMP(const char* Filename) {//���� �б� 
	FILE* File = NULL;

	if (!Filename) {
		return NULL; // ���Ͼ����� null 
	}
	File = fopen(Filename, "r"); //�б�
	if (File) {
		fclose(File);
		return auxDIBImageLoad(Filename);
	}
	return NULL;
}

//Texture  �Լ�
void LoadGLTextures() {
	AUX_RGBImageRec* Texture[11];
	memset(Texture, 0, sizeof(void*) * 11);

	if ((Texture[0] = LoadBMP("sun.bmp")) && //�¾� texture 
		(Texture[1] = LoadBMP("Mercury.bmp")) && //���� texture 
		(Texture[2] = LoadBMP("Venus.bmp")) && //�ݼ�texture 
		(Texture[3] = LoadBMP("Earth.bmp")) && //���� texture 
		(Texture[4] = LoadBMP("Moon.bmp")) && //�� texture 
		(Texture[5] = LoadBMP("Mars.bmp")) && //ȭ�� texture 
		(Texture[6] = LoadBMP("Jupiter.bmp")) && //��texture 
		(Texture[7] = LoadBMP("Saturn.bmp")) && //�伺 texture 
		(Texture[8] = LoadBMP("Uranus.bmp")) && //õ�ռ� texture 
		(Texture[9] = LoadBMP("Neptune.bmp")) && //�ؿռ� texture 
		(Texture[10] = LoadBMP("Sunshine.bmp"))) { //�¾籤 texture 


		glGenTextures(11, tex); //texture 11�� ����
		for (int i = 0; i < 11; i++) {
			if (i == 10) {
				glBindTexture(GL_TEXTURE_2D, tex[i]);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);//texture �Ű������� ����
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
				//glTexImage2D(GL_TEXTURE_2D, 0, 3, Texture[i]->sizeX, Texture[i]->sizeY, 0, GL_RGB, GL_UNSIGNED_BYTE, Texture[i]->data);//2D �ؽ��� �̹����� ����
				gluBuild2DMipmaps(GL_TEXTURE_2D, 3, Texture[i]->sizeX, Texture[i]->sizeY, GL_RGB, GL_UNSIGNED_BYTE, Texture[i]->data);
			}
			else { //������ �ؽ����� ���
				glBindTexture(GL_TEXTURE_2D, tex[i]); //���� �ε��� �ؽ��� ������ ���ε�
				gluBuild2DMipmaps(GL_TEXTURE_2D, 3, Texture[i]->sizeX, Texture[i]->sizeY, GL_RGB, GL_UNSIGNED_BYTE, Texture[i]->data);

				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);

				glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP); //GL_S ��ǥ�� ����
				glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP); //GL_T ��ǥ�� ����
				glEnable(GL_TEXTURE_GEN_S); // ����
				glEnable(GL_TEXTURE_GEN_T); //����
				glDisable(GL_TEXTURE_GEN_S); //����
				glDisable(GL_TEXTURE_GEN_T); // ����
			}
		}

		for (int i = 0; i < 11; i++) {
			if (Texture[i]) {
				if (Texture[i]->data) { //Texture�� ����� �����Ͱ� ������
					free(Texture[i]->data); //�����͸� �޸� ����
				}
				free(Texture[i]); //Texture�� ����� ���� �޸� ����
			}
		}
	}
}
//�ʱ�ȭ �Լ�
void InitGL(GLvoid) {
	LoadGLTextures(); // Texture �ҷ����� 

	obj = gluNewQuadric();//�伺�� ���� ����

	glShadeModel(GL_SMOOTH);
	glClearDepth(1.0f);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glEnable(GL_LIGHTING); //�� ���� Ű��
	glEnable(GL_LIGHT0); //0�� ���� Ű��
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST); //���� ����, ������ �ȼ����� �����Ѵ�
	glEnable(GL_TEXTURE_2D); // 2D �ؽ���
}

void MyReshape(int w, int h) { // ������ ����, ����
	float aspect = (double)w / (double)h; // �������� ��Ⱦ��
	glViewport(0, 0, w, h);// ����Ʈ ����
	glMatrixMode(GL_PROJECTION); // ���� ��� ����
	glLoadIdentity();//��� �ʱ�ȭ
	gluPerspective(20.0, aspect, 0.1, 500.0); //������ ũ�Ⱚ�� ���� ������ �Ķ���ͷ� ����
	glMatrixMode(GL_MODELVIEW); //������ġ
}

// ��� Texture mapping
void renderSphere(float cx, float cy, float cz, float r, int p) { //���� �ҽ� Ȱ��
	const float PI = 3.14159265358979f; //PI 
	const float TWOPI = 6.28318530717958f; //PI 2�� 
	const float PIDIV2 = 1.57079632679489f; //PI 1/2�� 

	float theta1 = 0.0, theta2 = 0.0, theta3 = 0.0;
	float ex = 0.0f, ey = 0.0f, ez = 0.0f;
	float px = 0.0f, py = 0.0f, pz = 0.0f;

	if (r < 0) r = -r; //radius�� ���
	if (p < 0) p = -p; //precision�� ���

	if (p < 4 || r <= 0) { //������ �ﰢ���� ���� ���
		glBegin(GL_POINTS); //�� ����
		glVertex3f(cx, cy, cz); //cx, cy, cz�� ������
		glEnd();
		return;
	}

	for (int i = 0; i < p / 2; ++i) { //��ü precision ���� ���� ��ŭ �ݺ�
		theta1 = i * TWOPI / p - PIDIV2;
		theta2 = (i + 1) * TWOPI / p - PIDIV2;

		glBegin(GL_TRIANGLE_STRIP);
		{
			for (int j = 0; j <= p; ++j) {
				theta3 = j * TWOPI / p;

				ex = cos(theta2) * cos(theta3);
				ey = sin(theta2);
				ez = cos(theta2) * sin(theta3);
				px = cx + r * ex;
				py = cy + r * ey;
				pz = cz + r * ez;

				glNormal3f(ex, ey, ez); //�ش� �ﰢ���� ���� ���� �ؽ��� ����
				glTexCoord2f(-(j / (float)p), 2 * (i + 1) / (float)p); //�ü� ��ǥ ����
				glVertex3f(px, py, pz); //px, py, pz�� �� ����

				ex = cos(theta1) * cos(theta3);
				ey = sin(theta1);
				ez = cos(theta1) * sin(theta3);
				px = cx + r * ex;
				py = cy + r * ey;
				pz = cz + r * ez;

				glNormal3f(ex, ey, ez); //�ش� �ﰢ���� ���� ���� �ؽ��� ����
				glTexCoord2f(-(j / (float)p), 2 * i / (float)p); //�ü� ��ǥ ����
				glVertex3f(px, py, pz); //px, py, pz�� �� ����
			}
		}
		glEnd();
	}
}
// ���� ���� ������ �����ϴ� �Լ�
void set_lighting()
{
	glEnable(GL_LIGHTING); // ���� ��� ����
	glEnable(GL_COLOR_MATERIAL); // ���� ���� ��� ����
	glEnable(GL_NORMALIZE); // �ڵ� ���� ���� ����


	glLightfv(GL_LIGHT0, GL_POSITION, light_pos); // 0�� ���� ��ġ ����
	glLightfv(GL_LIGHT1, GL_POSITION, light_pos); // 1�� ���� ��ġ ����

	GLfloat ambientColor[] = { 0.1f, 0.1f, 0.1f, 1.0f }; // �ֺ���
	GLfloat ambientColorForSun[] = { 1.0f, 1.0f, 1.0f, 1.0f }; // �¾� �ֺ��� (�¾��� ������ �ϱ� ���ؼ� ��� ����)
	GLfloat diffuseColor[] = { 0.9f, 0.9f, 0.9f, 1.0f }; // �л걤
	GLfloat specularColor[] = { 1.0f, 1.0f, 1.0f, 1.0f }; // �ݻ籤

	glLightfv(GL_LIGHT0, GL_AMBIENT, ambientColorForSun); // 0�� ���� �ֺ��� ����
	glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuseColor); // 0�� ���� �л걤 ����
	glLightfv(GL_LIGHT0, GL_SPECULAR, specularColor); // 0�� ���� �ݻ籤 ����

	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, ambientColor); // �ֺ��� ���� ����
	glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, diffuseColor); // �л걤 ���� ����
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, specularColor); // �ݻ籤 ���� ����
	glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE); // ���� ������ �ֺ����� �л걤�� ����
	glMateriali(GL_FRONT_AND_BACK, GL_SHININESS, 256); // ������ �ݻ絵 ����
}

float th = 0;
float xp, yp;

void MyDisplay() {
	glEnable(GL_CULL_FACE); // �ؽ����� ���� ��
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);//�����ʱ�ȭ
	glClearColor(0.11, 0.11, 0.11, 1.0); // ȭ���� ���������� �ʱ�ȭ
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();//����ʱ�ȭ

	gluLookAt(0.0, 0.0, 5.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0); //ī�޶� ��ġ
	glRotatef(-twi, 0.0f, 1.0f, 0.0f); // ����ǥ��
	glRotatef(-ele, 1.0f, 0.0f, 0.0f); // ����ǥ��
	glRotatef(zoom, 0.0f, 0.0f, 0.0f); // ����ǥ��
	
	//******** �˵� ���
	//����
	glPushMatrix();
	glColor3f(200, 0, 255);
	glPointSize(1);
	glBegin(GL_POINTS);
	for (th = 0; th <= 3.14 * 2.f; th += 0.002)
	{
		xp = 0.2 * cos(th); //�Ű� ������ �ش��ϴ� x�� ���
		yp = 0.2 * sin(th);//�Ű� ������ �ش��ϴ� �˰� ���
		glVertex3f(xp, 0, yp);// ���� �Ű������� ���� ��ǥ�� ���
		glColor3f(200, 0, 255);
	}
	glPopMatrix();
	glPushMatrix();
	glutWireSphere(0.0, 20, 20);
	glColor3f(200, 0, 255);
	glPopMatrix();
	
	//�ݼ�
	glPushMatrix();
	glColor3f(200, 0, 255);
	glPointSize(1);
	glBegin(GL_POINTS);
	for (th = 0; th <= 3.14 * 2.f; th += 0.002)
	{
		xp = 0.28 * cos(th); //�Ű� ������ �ش��ϴ� x�� ���
		yp = 0.28 * sin(th);//�Ű� ������ �ش��ϴ� y�� ���
		glVertex3f(xp, 0, yp);// ���� �Ű������� ���� ��ǥ�� ���
		glColor3f(200, 0, 255);
	}
	glPopMatrix();
	glPushMatrix();
	glutWireSphere(0.0, 20, 20);
	glColor3f(200, 0, 255);
	glPopMatrix();
	//����
	glPushMatrix();
	glColor3f(200, 0, 255);
	glPointSize(1);
	glBegin(GL_POINTS);
	for (th = 0; th <= 3.14 * 2.f; th += 0.002)
	{
		xp = 0.39 * cos(th); //�Ű� ������ �ش��ϴ� x�� ���
		yp = 0.39 * sin(th);//�Ű� ������ �ش��ϴ� y�� ���
		glVertex3f(xp, 0, yp);// ���� �Ű������� ���� ��ǥ�� ���
		glColor3f(200, 0, 255);
	}
	glPopMatrix();
	glPushMatrix();
	glutWireSphere(0.0, 20, 20);
	glColor3f(200, 0, 255);
	glPopMatrix();
	//ȭ��
	glPushMatrix();
	glColor3f(200, 0, 255);
	glPointSize(1);
	glBegin(GL_POINTS);
	for (th = 0; th <= 3.14 * 2.f; th += 0.002)
	{
		xp = 0.50 * cos(th); //�Ű� ������ �ش��ϴ� x�� ���
		yp = 0.50 * sin(th);//�Ű� ������ �ش��ϴ� y�� ���
		glVertex3f(xp, 0, yp);// ���� �Ű������� ���� ��ǥ�� ���
		glColor3f(200, 0, 255);
	}
	glPopMatrix();
	glPushMatrix();
	glutWireSphere(0.0, 20, 20);
	glColor3f(200, 0, 255);
	glPopMatrix();
	//��
	glPushMatrix();
	glColor3f(200, 0, 255);
	glPointSize(1);
	glBegin(GL_POINTS);
	for (th = 0; th <= 3.14 * 2.f; th += 0.002)
	{
		xp = 0.65 * cos(th); //�Ű� ������ �ش��ϴ� x�� ���
		yp = 0.65 * sin(th);//�Ű� ������ �ش��ϴ� y�� ���
		glVertex3f(xp, 0, yp);// ���� �Ű������� ���� ��ǥ�� ���
		glColor3f(200, 0, 255);
	}
	glPopMatrix();
	glPushMatrix();
	glutWireSphere(0.0, 20, 20);
	glColor3f(200, 0, 255);
	glPopMatrix();

	//�伺
	glPushMatrix();
	glColor3f(200, 0, 255);
	glPointSize(1);
	glBegin(GL_POINTS);
	for (th = 0; th <= 3.14 * 2.f; th += 0.002)
	{
		xp = 0.80 * cos(th); //�Ű� ������ �ش��ϴ� x�� ���
		yp = 0.80 * sin(th);//�Ű� ������ �ش��ϴ� y�� ���
		glVertex3f(xp, 0, yp);// ���� �Ű������� ���� ��ǥ�� ���
		glColor3f(200, 0, 255);
	}
	glPopMatrix();
	glPushMatrix();
	glutWireSphere(0.0, 20, 20);
	glColor3f(200, 0, 255);
	glPopMatrix();
	//õ�ռ�
	glPushMatrix();
	glColor3f(200, 0, 255);
	glPointSize(1);
	glBegin(GL_POINTS);
	for (th = 0; th <= 3.14 * 2.f; th += 0.002)
	{
		xp = 0.94 * cos(th); //�Ű� ������ �ش��ϴ� x�� ���
		yp = 0.94 * sin(th);//�Ű� ������ �ش��ϴ� y�� ���
		glVertex3f(xp, 0, yp);// ���� �Ű������� ���� ��ǥ�� ���
		glColor3f(200, 0, 255);
	}
	glPopMatrix();
	glPushMatrix();
	glutWireSphere(0.0, 20, 20);
	glColor3f(200, 0, 255);
	glPopMatrix();
	//�ؿռ�
	glPushMatrix();
	glColor3f(200, 0, 255);
	glPointSize(1);
	glBegin(GL_POINTS);
	for (th = 0; th <= 3.14 * 2.f; th += 0.002)
	{
		xp = 1.05 * cos(th); //�Ű� ������ �ش��ϴ� x�� ���
		yp = 1.05 * sin(th);//�Ű� ������ �ش��ϴ� y�� ���
		glVertex3f(xp, 0, yp);// ���� �Ű������� ���� ��ǥ�� ���
		glColor3f(200, 0, 255);
	}
	glPopMatrix();
	glPushMatrix();
	glutWireSphere(0.0, 20, 20);
	glColor3f(200, 0, 255);
	glPopMatrix();



	//============================================= 
	//�༺ ����
	glPushMatrix();//�¾�
	glRotatef((GLfloat)Time, 0.0, 1.0, 0.0);//����
	glLightf(GL_LIGHT0, GL_CONSTANT_ATTENUATION, const_att);
	glLightfv(GL_LIGHT0, GL_POSITION, light_pos);//�� ��ġ����
	glDisable(GL_LIGHTING);
	glColor3f(1, 1, 1);
	glBindTexture(GL_TEXTURE_2D, tex[0]); //�¾� �ؽ���
	glMaterialfv(GL_FRONT, GL_SHININESS, light_pos);
	renderSphere(0.0, 0.0, 0.0, 0.10, 50); //0.1 �������� 50���� �����̽� ��
	glEnable(GL_LIGHTING);
	glPopMatrix();


	glPushMatrix();//����
	glRotatef(5 + (GLfloat)Day[0], 0.0, 1.0, 0.0);//����
	glRotatef(5, 0.0, 1.0, 0.0);
	glTranslatef(0.20, 0.0, 0.0);
	glRotatef((GLfloat)Time, 0.0, 1.0, 0.0);//����
	glBindTexture(GL_TEXTURE_2D, tex[1]); //���� �ؽ���
	glMaterialfv(GL_FRONT, GL_SHININESS, light_pos);
	renderSphere(0.0, 0.0, 0.0, 0.018, 50);//0.18 �������� 50 �����̽� ��
	glPopMatrix();


	glPushMatrix();//�ݼ�
	glRotatef(205 + (GLfloat)Day[1], 0.0, 1.0, 0.0);//����
	glRotatef(205, 0.0, 1.0, 0.0);
	glTranslatef(0.28, 0.0, 0.0);
	glRotatef((GLfloat)Time, 0.0, 1.0, 0.0);//����
	glBindTexture(GL_TEXTURE_2D, tex[2]); //�ݼ� �ؽ���
	glMaterialfv(GL_FRONT, GL_SHININESS, light_pos);
	renderSphere(0.0, 0.0, 0.0, 0.02, 50); //0.02 �������� 50���� �����̽� ��
	glPopMatrix();
	

	glPushMatrix(); //����
	glRotatef(330 + (GLfloat)Day[2], 0.0, 1.0, 0.0); //����
	glRotatef(330, 0.0, 1.0, 0.0);
	glTranslatef(0.39, 0.0, 0.0);
	glRotatef((GLfloat)Time, 0.0, 1.0, 0.0);//����
	glBindTexture(GL_TEXTURE_2D, tex[3]); //���� �ؽ���
	glMaterialfv(GL_FRONT, GL_SHININESS, light_pos);
	renderSphere(0.0, 0.0, 0.0, 0.035, 50); //0.035 �������� 50���� �����̽� ��

	glPushMatrix(); //��
	glDisable(GL_LIGHTING);
	glRotatef(5 + (GLfloat)Time, 0.0, 1.0, 0.0);//����
	glRotatef(5, 0.0, 1.0, 0.0);
	glTranslatef(0.06, 0.0, 0.0);
	glBindTexture(GL_TEXTURE_2D, tex[4]); //�� �ؽ���
	glMaterialfv(GL_FRONT, GL_SHININESS, light_pos);
	renderSphere(0.0, 0.0, 0.0, 0.01, 50); //0.01 �������� 50���� �����̽� ��
	glPopMatrix();
	glPopMatrix();


	glPushMatrix();//ȭ��
	glRotatef(35 + (GLfloat)Day[3], 0.0, 1.0, 0.0); //����
	glRotatef(35, 0.0, 1.0, 0.0);
	glTranslatef(0.50, 0.0, 0.0);
	glRotatef((GLfloat)Time, 0.0, 1.0, 0.0);//����
	glBindTexture(GL_TEXTURE_2D, tex[5]); //ȭ�� �ؽ��Ŀ� ����
	glMaterialfv(GL_FRONT, GL_SHININESS, light_pos);
	renderSphere(0.0, 0.0, 0.0, 0.03, 50); //0.03 �������� 50���� �����̽� ��
	glPopMatrix();


	glPushMatrix(); //��
	glRotatef(55 + (GLfloat)Day[4], 0.0, 1.0, 0.0); //����
	glRotatef(55, 0.0, 1.0, 0.0);
	glTranslatef(0.65, 0.0, 0.0);
	glRotatef((GLfloat)Time, 0.0, 1.0, 0.0);//����
	glBindTexture(GL_TEXTURE_2D, tex[6]); //�� �ؽ���
	glMaterialfv(GL_FRONT, GL_SHININESS, light_pos);
	renderSphere(0.0, 0.0, 0.0, 0.05, 50); //0.05 �������� 50���� �����̽� ��
	glPopMatrix();


	glPushMatrix();//�伺
	glRotatef(105 + (GLfloat)Day[5], 0.0, 1.0, 0.0);//����
	glRotatef(105, 0.0, 1.0, 0.0);
	glTranslatef(0.80, 0.0, 0.0);

	//�伺-��
	glRotatef(70, 1.0, 1.0, 0.0);
	glDisable(GL_LIGHTING); //�� ���� ����
	glColor3f(0.5, 0.3, 0.3);
	gluDisk(obj, 0.06, 0.065, 30, 1);
	glColor3f(0.8, 0.5, 0.3); //�� ��° �� 
	gluDisk(obj, 0.065, 0.07, 30, 1); //�ι�° ��
	glColor3f(0.9, 0.7, 0.5); //�� ��° ��
	gluDisk(obj, 0.07, 0.075, 30, 1); //����° ��
	glColor3f(0.9, 0.7, 0.5);//�׹�° �� �� ����
	gluDisk(obj, 0.075, 0.08, 30, 1); //�׹�° ��
	glColor3f(0.8, 0.5, 0.3);//�ټ���° ��
	gluDisk(obj, 0.08, 0.085, 30, 1); //�ټ� ���� ��
	glColor3f(0.5, 0.3, 0.3);//������° �� 
	gluDisk(obj, 0.085, 0.09, 30, 1); //���� ��° ��


	glRotatef((GLfloat)Time, 0.0, 1.0, 0.0);  //�伺 ����
	glBindTexture(GL_TEXTURE_2D, tex[7]); //�伺 �ؽ���
	glMaterialfv(GL_FRONT, GL_SHININESS, light_pos);
	renderSphere(0.0, 0.0, 0.0, 0.04, 50); //0.04 �������� 50���� �����̽� ��
	glPopMatrix();


	glPushMatrix();//õ�ռ�
	glRotatef(195 + (GLfloat)Day[6], 0.0, 1.0, 0.0); //����
	glRotatef(195, 0.0, 1.0, 0.0);
	glTranslatef(0.94, 0.0, 0.0);
	glRotatef((GLfloat)Time, 0.0, 1.0, 0.0); //����
	glBindTexture(GL_TEXTURE_2D, tex[8]); //õ�ռ� �ؽ��Ŀ� ����
	glMaterialfv(GL_FRONT, GL_SHININESS, light_pos);
	renderSphere(0.0, 0.0, 0.0, 0.03, 30); //0.03 �������� 30���� �����̽� ��
	glPopMatrix();


	glPushMatrix();//�ؿռ�
	glRotatef(255 + (GLfloat)Day[7], 0.0, 1.0, 0.0); //����
	glRotatef(255, 0.0, 1.0, 0.0);
	glTranslatef(1.05, 0.0, 0.0);
	glRotatef((GLfloat)Time, 0.0, 1.0, 0.0); //����
	glBindTexture(GL_TEXTURE_2D, tex[9]); //�ؿռ� �ؽ��Ŀ� ����
	glMaterialfv(GL_FRONT, GL_SHININESS, light_pos);
	renderSphere(0.0, 0.0, 0.0, 0.031, 30); //0.031 �������� 30���� �����̽� ��
	glPopMatrix();

	glPushMatrix(); //�¾籤
	glDisable(GL_LIGHTING);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);//���� ���� ���� ���� ����
	glColor4f(1.0, 0.0, 0.0, 0.5);
	glBindTexture(GL_TEXTURE_2D, tex[10]); //�¾籤 �ؽ��Ŀ� ����
	glBegin(GL_QUADS); //�簢�� ���� ����
	/*
	glTexCoord2f(1.0f, 1.0f);  glVertex3f(0.2, 0.2, 0.0);//�簢�� ��ǥ����
	glTexCoord2f(1.0f, 0.0f);  glVertex3f(0.2, -0.2, 0.0);//����
	glTexCoord2f(0.0f, 0.0f);  glVertex3f(-0.2, -0.2, 0.0);//����
	glTexCoord2f(0.0f, 1.0f);  glVertex3f(-0.2, 0.2, 0.0);//����
	*/
	glEnd();
	glEnable(GL_LIGHTING);
	glPopMatrix();

	set_lighting();

	glFlush();
	glutSwapBuffers(); //���� ����

	if (flag) { // �ִϸ��̼� On �����̸�
		glutPostRedisplay(); // ȭ���� ����
	}
}

void MyTimer(int Value)
{
	//�����ֱ� ��� glRoteatef�Լ��� ���� ���� ��� �� 360���� ��� ������ �������� ���� ����  

	//���� �ֱ�  
	Day[0] = (Day[0] + 4) % 360;
	Day[1] = (Day[1] + 3) % 360;
	Day[2] = (Day[2] + 1) % 360;
	Day[3] = (Day[3] + 2) % 360;
	Day[4] = (Day[4] + 1) % 360;
	Day[5] = (Day[5] + 1) % 360;
	Day[6] = (Day[6] + 2) % 360;
	Day[7] = (Day[7] + 1) % 360;

	glutPostRedisplay();                    // ȭ���� �ٽ� ����Ϳ� �ѷ��� 
	if (flag == true)                        //�ִϸ��̼� ���°� flag�� ture���  
	{
		glutTimerFunc(30, MyTimer, 1);        //��� ���ȣ���Ͽ� ���ϸ��̼��� ���۽�Ŵ  
	}
}
// Ű���� �Է� ó�� �Լ�
void keyboard(int key, int x, int y) {
	if (key == GLUT_KEY_PAGE_UP) {  // page up ��ư�� ������
		 zoom -= 1.0f;  // �ܾƿ�
	}
	else if (key == GLUT_KEY_PAGE_DOWN) {  // pagedown ��ư�� ������
		 zoom += 1.0f;   // ����
	}
	else if (key == GLUT_KEY_UP) {  // ���� ȭ��ǥ�� ������
		ele = ele + 1.0;  // ����ǥ�� y�� ���
	}
	else if (key == GLUT_KEY_DOWN) {  // �Ʒ� ȭ��ǥ�� ������
		ele = ele - 1.0;// ����ǥ�� y�� �ϰ�
	}
	else if (key == GLUT_KEY_LEFT) {   // ���� ȭ��ǥ�� ������
		twi = twi + 1.0;
	}
	else if (key == GLUT_KEY_RIGHT) {   // ������ ȭ��ǥ�� ������
		twi = twi - 1.0;
	}
	glutPostRedisplay();  // ȭ�� ���÷���
}
void MyIdle()
{
	Day[0] = Day[0] + 4;
	Day[1] = Day[1] + 3;
	Day[2] = Day[2] + 2;
	Day[3] = Day[3] + 2;
	Day[4] = Day[4] + 1;
	Day[5] = Day[5] + 1;
	Day[6] = Day[6] + 1;
	Day[7] = Day[7] + 1;
	Day[8] = Day[8] + 1;
	glutPostRedisplay();
}

int main(int argc, char** argv) { //�����Լ�

	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA); // ������۸��� RGBA ����
	glutInitWindowSize(1024, 768); //â ũ��
	glutCreateWindow("Solar_System 201879024");//�̸�

	glutSpecialFunc(keyboard);// keyboard �Լ�
	glutReshapeFunc(MyReshape); //MyReshape �Լ�
	glutDisplayFunc(MyDisplay); //MyDisplay �Լ�

	InitGL(); //InitGL �Լ� ȣ��
	glutIdleFunc(MyIdle);
	glutMainLoop(); //�ݺ�

	return 0;//0 �� ����
}

