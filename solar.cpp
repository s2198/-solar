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

static int Day[9] = { 34,223,56,123,333,154,20,170,280 }; // 공전주기
static int Time = 0; //자전 주기

float ele = -4.0f, azi = 0.0f, rad = 0.0f, twi = 0.0f; //극좌표계 조절 변수
float zoom = -2.5f;

bool flag = true;// 애니메이션 ON / OFF 설정 변수
GLUquadricObj* obj;//토성고리를 만들기위한 quadrics

const GLfloat light_pos[] = { 0.0, 0.0, 0.0, 1.0 }; //빛의 위치를 정해주는 변수
GLfloat const_att = 1.0;

GLuint tex[11]; //텍스쳐를 저장할 변수

AUX_RGBImageRec* LoadBMP(const char* Filename) {//파일 읽기 
	FILE* File = NULL;

	if (!Filename) {
		return NULL; // 파일없으면 null 
	}
	File = fopen(Filename, "r"); //읽기
	if (File) {
		fclose(File);
		return auxDIBImageLoad(Filename);
	}
	return NULL;
}

//Texture  함수
void LoadGLTextures() {
	AUX_RGBImageRec* Texture[11];
	memset(Texture, 0, sizeof(void*) * 11);

	if ((Texture[0] = LoadBMP("sun.bmp")) && //태양 texture 
		(Texture[1] = LoadBMP("Mercury.bmp")) && //수성 texture 
		(Texture[2] = LoadBMP("Venus.bmp")) && //금성texture 
		(Texture[3] = LoadBMP("Earth.bmp")) && //지구 texture 
		(Texture[4] = LoadBMP("Moon.bmp")) && //달 texture 
		(Texture[5] = LoadBMP("Mars.bmp")) && //화성 texture 
		(Texture[6] = LoadBMP("Jupiter.bmp")) && //목성texture 
		(Texture[7] = LoadBMP("Saturn.bmp")) && //토성 texture 
		(Texture[8] = LoadBMP("Uranus.bmp")) && //천왕성 texture 
		(Texture[9] = LoadBMP("Neptune.bmp")) && //해왕성 texture 
		(Texture[10] = LoadBMP("Sunshine.bmp"))) { //태양광 texture 


		glGenTextures(11, tex); //texture 11개 공간
		for (int i = 0; i < 11; i++) {
			if (i == 10) {
				glBindTexture(GL_TEXTURE_2D, tex[i]);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);//texture 매개변수를 설정
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
				//glTexImage2D(GL_TEXTURE_2D, 0, 3, Texture[i]->sizeX, Texture[i]->sizeY, 0, GL_RGB, GL_UNSIGNED_BYTE, Texture[i]->data);//2D 텍스쳐 이미지를 정의
				gluBuild2DMipmaps(GL_TEXTURE_2D, 3, Texture[i]->sizeX, Texture[i]->sizeY, GL_RGB, GL_UNSIGNED_BYTE, Texture[i]->data);
			}
			else { //나머지 텍스쳐의 경우
				glBindTexture(GL_TEXTURE_2D, tex[i]); //같은 인덱스 텍스쳐 변수에 바인딩
				gluBuild2DMipmaps(GL_TEXTURE_2D, 3, Texture[i]->sizeX, Texture[i]->sizeY, GL_RGB, GL_UNSIGNED_BYTE, Texture[i]->data);

				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);

				glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP); //GL_S 좌표를 추출
				glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP); //GL_T 좌표를 추출
				glEnable(GL_TEXTURE_GEN_S); // 실행
				glEnable(GL_TEXTURE_GEN_T); //실행
				glDisable(GL_TEXTURE_GEN_S); //중지
				glDisable(GL_TEXTURE_GEN_T); // 중지
			}
		}

		for (int i = 0; i < 11; i++) {
			if (Texture[i]) {
				if (Texture[i]->data) { //Texture에 저장된 데이터가 있으면
					free(Texture[i]->data); //데이터를 메모리 해제
				}
				free(Texture[i]); //Texture가 저장된 변수 메모리 해제
			}
		}
	}
}
//초기화 함수
void InitGL(GLvoid) {
	LoadGLTextures(); // Texture 불러오기 

	obj = gluNewQuadric();//토성고리 변수 선언

	glShadeModel(GL_SMOOTH);
	glClearDepth(1.0f);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glEnable(GL_LIGHTING); //주 조명 키기
	glEnable(GL_LIGHT0); //0번 조명 키기
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST); //원근 보정, 보간을 픽셀별로 수행한다
	glEnable(GL_TEXTURE_2D); // 2D 텍스쳐
}

void MyReshape(int w, int h) { // 윈도우 가로, 세로
	float aspect = (double)w / (double)h; // 윈도우의 종횡비
	glViewport(0, 0, w, h);// 뷰포트 설정
	glMatrixMode(GL_PROJECTION); // 투영 행렬 설정
	glLoadIdentity();//행렬 초기화
	gluPerspective(20.0, aspect, 0.1, 500.0); //윈도우 크기값을 원근 투영의 파라미터로 재사용
	glMatrixMode(GL_MODELVIEW); //실제위치
}

// 곡면 Texture mapping
void renderSphere(float cx, float cy, float cz, float r, int p) { //교재 소스 활용
	const float PI = 3.14159265358979f; //PI 
	const float TWOPI = 6.28318530717958f; //PI 2배 
	const float PIDIV2 = 1.57079632679489f; //PI 1/2배 

	float theta1 = 0.0, theta2 = 0.0, theta3 = 0.0;
	float ex = 0.0f, ey = 0.0f, ez = 0.0f;
	float px = 0.0f, py = 0.0f, pz = 0.0f;

	if (r < 0) r = -r; //radius를 양수
	if (p < 0) p = -p; //precision을 양수

	if (p < 4 || r <= 0) { //생성된 삼각형이 없을 경우
		glBegin(GL_POINTS); //점 생성
		glVertex3f(cx, cy, cz); //cx, cy, cz로 점생성
		glEnd();
		return;
	}

	for (int i = 0; i < p / 2; ++i) { //전체 precision 값의 절반 만큼 반복
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

				glNormal3f(ex, ey, ez); //해당 삼각형에 대한 수동 텍스쳐 매핑
				glTexCoord2f(-(j / (float)p), 2 * (i + 1) / (float)p); //택셀 좌표 지정
				glVertex3f(px, py, pz); //px, py, pz로 점 생성

				ex = cos(theta1) * cos(theta3);
				ey = sin(theta1);
				ez = cos(theta1) * sin(theta3);
				px = cx + r * ex;
				py = cy + r * ey;
				pz = cz + r * ez;

				glNormal3f(ex, ey, ez); //해당 삼각형에 대한 수동 텍스쳐 매팅
				glTexCoord2f(-(j / (float)p), 2 * i / (float)p); //택셀 좌표 지정
				glVertex3f(px, py, pz); //px, py, pz로 점 생성
			}
		}
		glEnd();
	}
}
// 조명 관련 설정를 수행하는 함수
void set_lighting()
{
	glEnable(GL_LIGHTING); // 조명 사용 설정
	glEnable(GL_COLOR_MATERIAL); // 색상 재질 사용 설정
	glEnable(GL_NORMALIZE); // 자동 법선 벡터 설정


	glLightfv(GL_LIGHT0, GL_POSITION, light_pos); // 0번 조명 위치 설정
	glLightfv(GL_LIGHT1, GL_POSITION, light_pos); // 1번 조명 위치 설정

	GLfloat ambientColor[] = { 0.1f, 0.1f, 0.1f, 1.0f }; // 주변광
	GLfloat ambientColorForSun[] = { 1.0f, 1.0f, 1.0f, 1.0f }; // 태양 주변광 (태양을 빛나게 하기 위해서 밝게 설정)
	GLfloat diffuseColor[] = { 0.9f, 0.9f, 0.9f, 1.0f }; // 분산광
	GLfloat specularColor[] = { 1.0f, 1.0f, 1.0f, 1.0f }; // 반사광

	glLightfv(GL_LIGHT0, GL_AMBIENT, ambientColorForSun); // 0번 조명 주변광 설정
	glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuseColor); // 0번 조명 분산광 설정
	glLightfv(GL_LIGHT0, GL_SPECULAR, specularColor); // 0번 조명 반사광 설정

	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, ambientColor); // 주변광 재질 설정
	glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, diffuseColor); // 분산광 재질 설정
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, specularColor); // 반사광 재질 설정
	glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE); // 색상 재질을 주변광과 분산광에 설정
	glMateriali(GL_FRONT_AND_BACK, GL_SHININESS, 256); // 재질의 반사도 설정
}

float th = 0;
float xp, yp;

void MyDisplay() {
	glEnable(GL_CULL_FACE); // 텍스쳐의 한쪽 면
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);//버퍼초기화
	glClearColor(0.11, 0.11, 0.11, 1.0); // 화면을 검은색으로 초기화
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();//행렬초기화

	gluLookAt(0.0, 0.0, 5.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0); //카메라 위치
	glRotatef(-twi, 0.0f, 1.0f, 0.0f); // 극좌표계
	glRotatef(-ele, 1.0f, 0.0f, 0.0f); // 극좌표계
	glRotatef(zoom, 0.0f, 0.0f, 0.0f); // 극좌표계
	
	//******** 궤도 계산
	//수성
	glPushMatrix();
	glColor3f(200, 0, 255);
	glPointSize(1);
	glBegin(GL_POINTS);
	for (th = 0; th <= 3.14 * 2.f; th += 0.002)
	{
		xp = 0.2 * cos(th); //매개 변수에 해당하는 x값 계산
		yp = 0.2 * sin(th);//매개 변수에 해당하는 ㅛ값 계산
		glVertex3f(xp, 0, yp);// 현재 매개변수로 얻은 좌표를 출력
		glColor3f(200, 0, 255);
	}
	glPopMatrix();
	glPushMatrix();
	glutWireSphere(0.0, 20, 20);
	glColor3f(200, 0, 255);
	glPopMatrix();
	
	//금성
	glPushMatrix();
	glColor3f(200, 0, 255);
	glPointSize(1);
	glBegin(GL_POINTS);
	for (th = 0; th <= 3.14 * 2.f; th += 0.002)
	{
		xp = 0.28 * cos(th); //매개 변수에 해당하는 x값 계산
		yp = 0.28 * sin(th);//매개 변수에 해당하는 y값 계산
		glVertex3f(xp, 0, yp);// 현재 매개변수로 얻은 좌표를 출력
		glColor3f(200, 0, 255);
	}
	glPopMatrix();
	glPushMatrix();
	glutWireSphere(0.0, 20, 20);
	glColor3f(200, 0, 255);
	glPopMatrix();
	//지구
	glPushMatrix();
	glColor3f(200, 0, 255);
	glPointSize(1);
	glBegin(GL_POINTS);
	for (th = 0; th <= 3.14 * 2.f; th += 0.002)
	{
		xp = 0.39 * cos(th); //매개 변수에 해당하는 x값 계산
		yp = 0.39 * sin(th);//매개 변수에 해당하는 y값 계산
		glVertex3f(xp, 0, yp);// 현재 매개변수로 얻은 좌표를 출력
		glColor3f(200, 0, 255);
	}
	glPopMatrix();
	glPushMatrix();
	glutWireSphere(0.0, 20, 20);
	glColor3f(200, 0, 255);
	glPopMatrix();
	//화성
	glPushMatrix();
	glColor3f(200, 0, 255);
	glPointSize(1);
	glBegin(GL_POINTS);
	for (th = 0; th <= 3.14 * 2.f; th += 0.002)
	{
		xp = 0.50 * cos(th); //매개 변수에 해당하는 x값 계산
		yp = 0.50 * sin(th);//매개 변수에 해당하는 y값 계산
		glVertex3f(xp, 0, yp);// 현재 매개변수로 얻은 좌표를 출력
		glColor3f(200, 0, 255);
	}
	glPopMatrix();
	glPushMatrix();
	glutWireSphere(0.0, 20, 20);
	glColor3f(200, 0, 255);
	glPopMatrix();
	//목성
	glPushMatrix();
	glColor3f(200, 0, 255);
	glPointSize(1);
	glBegin(GL_POINTS);
	for (th = 0; th <= 3.14 * 2.f; th += 0.002)
	{
		xp = 0.65 * cos(th); //매개 변수에 해당하는 x값 계산
		yp = 0.65 * sin(th);//매개 변수에 해당하는 y값 계산
		glVertex3f(xp, 0, yp);// 현재 매개변수로 얻은 좌표를 출력
		glColor3f(200, 0, 255);
	}
	glPopMatrix();
	glPushMatrix();
	glutWireSphere(0.0, 20, 20);
	glColor3f(200, 0, 255);
	glPopMatrix();

	//토성
	glPushMatrix();
	glColor3f(200, 0, 255);
	glPointSize(1);
	glBegin(GL_POINTS);
	for (th = 0; th <= 3.14 * 2.f; th += 0.002)
	{
		xp = 0.80 * cos(th); //매개 변수에 해당하는 x값 계산
		yp = 0.80 * sin(th);//매개 변수에 해당하는 y값 계산
		glVertex3f(xp, 0, yp);// 현재 매개변수로 얻은 좌표를 출력
		glColor3f(200, 0, 255);
	}
	glPopMatrix();
	glPushMatrix();
	glutWireSphere(0.0, 20, 20);
	glColor3f(200, 0, 255);
	glPopMatrix();
	//천왕성
	glPushMatrix();
	glColor3f(200, 0, 255);
	glPointSize(1);
	glBegin(GL_POINTS);
	for (th = 0; th <= 3.14 * 2.f; th += 0.002)
	{
		xp = 0.94 * cos(th); //매개 변수에 해당하는 x값 계산
		yp = 0.94 * sin(th);//매개 변수에 해당하는 y값 계산
		glVertex3f(xp, 0, yp);// 현재 매개변수로 얻은 좌표를 출력
		glColor3f(200, 0, 255);
	}
	glPopMatrix();
	glPushMatrix();
	glutWireSphere(0.0, 20, 20);
	glColor3f(200, 0, 255);
	glPopMatrix();
	//해왕성
	glPushMatrix();
	glColor3f(200, 0, 255);
	glPointSize(1);
	glBegin(GL_POINTS);
	for (th = 0; th <= 3.14 * 2.f; th += 0.002)
	{
		xp = 1.05 * cos(th); //매개 변수에 해당하는 x값 계산
		yp = 1.05 * sin(th);//매개 변수에 해당하는 y값 계산
		glVertex3f(xp, 0, yp);// 현재 매개변수로 얻은 좌표를 출력
		glColor3f(200, 0, 255);
	}
	glPopMatrix();
	glPushMatrix();
	glutWireSphere(0.0, 20, 20);
	glColor3f(200, 0, 255);
	glPopMatrix();



	//============================================= 
	//행성 선언
	glPushMatrix();//태양
	glRotatef((GLfloat)Time, 0.0, 1.0, 0.0);//자전
	glLightf(GL_LIGHT0, GL_CONSTANT_ATTENUATION, const_att);
	glLightfv(GL_LIGHT0, GL_POSITION, light_pos);//빛 위치설정
	glDisable(GL_LIGHTING);
	glColor3f(1, 1, 1);
	glBindTexture(GL_TEXTURE_2D, tex[0]); //태양 텍스쳐
	glMaterialfv(GL_FRONT, GL_SHININESS, light_pos);
	renderSphere(0.0, 0.0, 0.0, 0.10, 50); //0.1 반지름에 50개의 슬라이스 구
	glEnable(GL_LIGHTING);
	glPopMatrix();


	glPushMatrix();//수성
	glRotatef(5 + (GLfloat)Day[0], 0.0, 1.0, 0.0);//공전
	glRotatef(5, 0.0, 1.0, 0.0);
	glTranslatef(0.20, 0.0, 0.0);
	glRotatef((GLfloat)Time, 0.0, 1.0, 0.0);//자전
	glBindTexture(GL_TEXTURE_2D, tex[1]); //수성 텍스쳐
	glMaterialfv(GL_FRONT, GL_SHININESS, light_pos);
	renderSphere(0.0, 0.0, 0.0, 0.018, 50);//0.18 반지름에 50 슬라이스 구
	glPopMatrix();


	glPushMatrix();//금성
	glRotatef(205 + (GLfloat)Day[1], 0.0, 1.0, 0.0);//공전
	glRotatef(205, 0.0, 1.0, 0.0);
	glTranslatef(0.28, 0.0, 0.0);
	glRotatef((GLfloat)Time, 0.0, 1.0, 0.0);//자전
	glBindTexture(GL_TEXTURE_2D, tex[2]); //금성 텍스쳐
	glMaterialfv(GL_FRONT, GL_SHININESS, light_pos);
	renderSphere(0.0, 0.0, 0.0, 0.02, 50); //0.02 반지름에 50개의 슬라이스 구
	glPopMatrix();
	

	glPushMatrix(); //지구
	glRotatef(330 + (GLfloat)Day[2], 0.0, 1.0, 0.0); //공전
	glRotatef(330, 0.0, 1.0, 0.0);
	glTranslatef(0.39, 0.0, 0.0);
	glRotatef((GLfloat)Time, 0.0, 1.0, 0.0);//자전
	glBindTexture(GL_TEXTURE_2D, tex[3]); //지구 텍스쳐
	glMaterialfv(GL_FRONT, GL_SHININESS, light_pos);
	renderSphere(0.0, 0.0, 0.0, 0.035, 50); //0.035 반지름에 50개의 슬라이스 구

	glPushMatrix(); //달
	glDisable(GL_LIGHTING);
	glRotatef(5 + (GLfloat)Time, 0.0, 1.0, 0.0);//자전
	glRotatef(5, 0.0, 1.0, 0.0);
	glTranslatef(0.06, 0.0, 0.0);
	glBindTexture(GL_TEXTURE_2D, tex[4]); //달 텍스쳐
	glMaterialfv(GL_FRONT, GL_SHININESS, light_pos);
	renderSphere(0.0, 0.0, 0.0, 0.01, 50); //0.01 반지름에 50개의 슬라이스 구
	glPopMatrix();
	glPopMatrix();


	glPushMatrix();//화성
	glRotatef(35 + (GLfloat)Day[3], 0.0, 1.0, 0.0); //공전
	glRotatef(35, 0.0, 1.0, 0.0);
	glTranslatef(0.50, 0.0, 0.0);
	glRotatef((GLfloat)Time, 0.0, 1.0, 0.0);//자전
	glBindTexture(GL_TEXTURE_2D, tex[5]); //화성 텍스쳐와 연결
	glMaterialfv(GL_FRONT, GL_SHININESS, light_pos);
	renderSphere(0.0, 0.0, 0.0, 0.03, 50); //0.03 반지름에 50개의 슬라이스 구
	glPopMatrix();


	glPushMatrix(); //목성
	glRotatef(55 + (GLfloat)Day[4], 0.0, 1.0, 0.0); //공전
	glRotatef(55, 0.0, 1.0, 0.0);
	glTranslatef(0.65, 0.0, 0.0);
	glRotatef((GLfloat)Time, 0.0, 1.0, 0.0);//자전
	glBindTexture(GL_TEXTURE_2D, tex[6]); //목성 텍스쳐
	glMaterialfv(GL_FRONT, GL_SHININESS, light_pos);
	renderSphere(0.0, 0.0, 0.0, 0.05, 50); //0.05 반지름에 50개의 슬라이스 구
	glPopMatrix();


	glPushMatrix();//토성
	glRotatef(105 + (GLfloat)Day[5], 0.0, 1.0, 0.0);//공전
	glRotatef(105, 0.0, 1.0, 0.0);
	glTranslatef(0.80, 0.0, 0.0);

	//토성-고리
	glRotatef(70, 1.0, 1.0, 0.0);
	glDisable(GL_LIGHTING); //주 조명 끄기
	glColor3f(0.5, 0.3, 0.3);
	gluDisk(obj, 0.06, 0.065, 30, 1);
	glColor3f(0.8, 0.5, 0.3); //두 번째 고리 
	gluDisk(obj, 0.065, 0.07, 30, 1); //두번째 고리
	glColor3f(0.9, 0.7, 0.5); //세 번째 고리
	gluDisk(obj, 0.07, 0.075, 30, 1); //세번째 고리
	glColor3f(0.9, 0.7, 0.5);//네번째 고리 색 지정
	gluDisk(obj, 0.075, 0.08, 30, 1); //네번째 고리
	glColor3f(0.8, 0.5, 0.3);//다섯번째 고리
	gluDisk(obj, 0.08, 0.085, 30, 1); //다섯 번쨰 고리
	glColor3f(0.5, 0.3, 0.3);//여섯번째 고리 
	gluDisk(obj, 0.085, 0.09, 30, 1); //여섯 번째 고리


	glRotatef((GLfloat)Time, 0.0, 1.0, 0.0);  //토성 자전
	glBindTexture(GL_TEXTURE_2D, tex[7]); //토성 텍스쳐
	glMaterialfv(GL_FRONT, GL_SHININESS, light_pos);
	renderSphere(0.0, 0.0, 0.0, 0.04, 50); //0.04 반지름에 50개의 슬라이스 구
	glPopMatrix();


	glPushMatrix();//천왕성
	glRotatef(195 + (GLfloat)Day[6], 0.0, 1.0, 0.0); //공전
	glRotatef(195, 0.0, 1.0, 0.0);
	glTranslatef(0.94, 0.0, 0.0);
	glRotatef((GLfloat)Time, 0.0, 1.0, 0.0); //자전
	glBindTexture(GL_TEXTURE_2D, tex[8]); //천왕성 텍스쳐와 연결
	glMaterialfv(GL_FRONT, GL_SHININESS, light_pos);
	renderSphere(0.0, 0.0, 0.0, 0.03, 30); //0.03 반지름에 30개의 슬라이스 구
	glPopMatrix();


	glPushMatrix();//해왕성
	glRotatef(255 + (GLfloat)Day[7], 0.0, 1.0, 0.0); //공전
	glRotatef(255, 0.0, 1.0, 0.0);
	glTranslatef(1.05, 0.0, 0.0);
	glRotatef((GLfloat)Time, 0.0, 1.0, 0.0); //자전
	glBindTexture(GL_TEXTURE_2D, tex[9]); //해왕성 텍스쳐와 연결
	glMaterialfv(GL_FRONT, GL_SHININESS, light_pos);
	renderSphere(0.0, 0.0, 0.0, 0.031, 30); //0.031 반지름에 30개의 슬라이스 구
	glPopMatrix();

	glPushMatrix(); //태양광
	glDisable(GL_LIGHTING);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);//알파 값에 따른 투명도 구현
	glColor4f(1.0, 0.0, 0.0, 0.5);
	glBindTexture(GL_TEXTURE_2D, tex[10]); //태양광 텍스쳐와 연결
	glBegin(GL_QUADS); //사각형 생성 시작
	/*
	glTexCoord2f(1.0f, 1.0f);  glVertex3f(0.2, 0.2, 0.0);//사각형 좌표연결
	glTexCoord2f(1.0f, 0.0f);  glVertex3f(0.2, -0.2, 0.0);//연결
	glTexCoord2f(0.0f, 0.0f);  glVertex3f(-0.2, -0.2, 0.0);//연결
	glTexCoord2f(0.0f, 1.0f);  glVertex3f(-0.2, 0.2, 0.0);//연결
	*/
	glEnd();
	glEnable(GL_LIGHTING);
	glPopMatrix();

	set_lighting();

	glFlush();
	glutSwapBuffers(); //버퍼 비우기

	if (flag) { // 애니메이션 On 상태이면
		glutPostRedisplay(); // 화면을 갱신
	}
}

void MyTimer(int Value)
{
	//공전주기 계산 glRoteatef함수에 들어가기 위해 계산 함 360도로 계속 나눠서 나머지를 값에 대입  

	//자전 주기  
	Day[0] = (Day[0] + 4) % 360;
	Day[1] = (Day[1] + 3) % 360;
	Day[2] = (Day[2] + 1) % 360;
	Day[3] = (Day[3] + 2) % 360;
	Day[4] = (Day[4] + 1) % 360;
	Day[5] = (Day[5] + 1) % 360;
	Day[6] = (Day[6] + 2) % 360;
	Day[7] = (Day[7] + 1) % 360;

	glutPostRedisplay();                    // 화면을 다시 모니터에 뿌려줌 
	if (flag == true)                        //애니메이션 상태값 flag가 ture라면  
	{
		glutTimerFunc(30, MyTimer, 1);        //계속 재귀호출하여 에니메이션을 동작시킴  
	}
}
// 키보드 입력 처리 함수
void keyboard(int key, int x, int y) {
	if (key == GLUT_KEY_PAGE_UP) {  // page up 버튼을 누르면
		 zoom -= 1.0f;  // 줌아웃
	}
	else if (key == GLUT_KEY_PAGE_DOWN) {  // pagedown 버튼을 누르면
		 zoom += 1.0f;   // 줌인
	}
	else if (key == GLUT_KEY_UP) {  // 위쪽 화살표를 누르면
		ele = ele + 1.0;  // 극좌표계 y축 상승
	}
	else if (key == GLUT_KEY_DOWN) {  // 아래 화살표를 누르면
		ele = ele - 1.0;// 극좌표계 y축 하강
	}
	else if (key == GLUT_KEY_LEFT) {   // 왼쪽 화살표를 누르면
		twi = twi + 1.0;
	}
	else if (key == GLUT_KEY_RIGHT) {   // 오른쪽 화살표를 누르면
		twi = twi - 1.0;
	}
	glutPostRedisplay();  // 화면 디스플레이
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

int main(int argc, char** argv) { //메인함수

	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA); // 더블버퍼링과 RGBA 설정
	glutInitWindowSize(1024, 768); //창 크기
	glutCreateWindow("Solar_System 201879024");//이름

	glutSpecialFunc(keyboard);// keyboard 함수
	glutReshapeFunc(MyReshape); //MyReshape 함수
	glutDisplayFunc(MyDisplay); //MyDisplay 함수

	InitGL(); //InitGL 함수 호출
	glutIdleFunc(MyIdle);
	glutMainLoop(); //반복

	return 0;//0 값 리턴
}

