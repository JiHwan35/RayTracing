#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "usr\include\GL\freeglut.h"
#include "gmath.h"

// 매크로 상수 정의
#define MAX_DEPTH 1
#define H 768
#define W 1024
unsigned char Image[H * W * 3];

std::vector<GSphere> SphereList;
std::vector<GLight> LightList;

// 콜백 함수 선언
void Render();
void Reshape(int w, int h);
void Timer(int id);

// 광선 추적 함수
void CreateImage();
GVec3 RayTrace(GLine ray, int depth);
GVec3 Phong(GPos3 P, GVec3 N, GSphere Obj);
bool intersect_line_sphere(GLine ray, int &sidx, double &t);

int main(int argc, char **argv)
{
	// OpenGL 초기화, 윈도우 크기 설정, 디스플레이 모드 설정
	glutInit(&argc, argv);
	glutInitWindowSize(W, H);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);

	// 윈도우 생성 및 콜백 함수 등록
	glutCreateWindow("RayTracer");
	glutDisplayFunc(Render);
	glutReshapeFunc(Reshape);
	glutTimerFunc(10, Timer, 0);

	// 조명 설정
	GLight Light0;
	Light0.Pos.Set(-500.0, 200.0, 5000.0);
	Light0.Ia.Set(0.2, 0.2, 0.2);
	Light0.Id.Set(1.0, 1.0, 1.0);
	Light0.Is.Set(1.0, 1.0, 1.0);
	LightList.push_back(Light0);

	
	GLight Light1;
	Light1.Pos.Set(-300.0, 300.0, -1000.0);
	Light1.Ia.Set(0.2, 0.2, 0.2);
	Light1.Id.Set(1.0, 1.0, 1.0);
	Light1.Is.Set(0.8, 0.8, 0.8);
	LightList.push_back(Light1);
	
	// 장면에 5개의 구를 배치한다.
	GSphere Sphere0;
	Sphere0.Pos.Set(0, 0, -500.0);
	Sphere0.Rad = 50.0;
	Sphere0.Ka.Set(0.1, 0.1, 0.1);
	Sphere0.Kd.Set(0.8, 0.8, 0.8);
	Sphere0.Ks.Set(0.9, 0.9, 0.9);
	Sphere0.ns = 8.0;
	Sphere0.bTransparent = true;
	SphereList.push_back(Sphere0);
	
	GSphere Sphere1;
	Sphere1.Pos.Set(-80, 80, -650.0);
	Sphere1.Rad = 50.0;
	Sphere1.Ka.Set(0.2, 0.2, 0.2);
	Sphere1.Kd.Set(0.7, 0.0, 0.0);
	Sphere1.Ks.Set(0.8, 0.8, 0.8);
	Sphere1.ns = 8.0;
	SphereList.push_back(Sphere1);

	GSphere Sphere2;
	Sphere2.Pos.Set(-80, -80, -650.0);
	Sphere2.Rad = 50.0;
	Sphere2.Ka.Set(0.2, 0.2, 0.2);
	Sphere2.Kd.Set(0.0, 0.7, 0.0);
	Sphere2.Ks.Set(0.8, 0.8, 0.8);
	Sphere2.ns = 8.0;		
	SphereList.push_back(Sphere2);

	GSphere Sphere3;
	Sphere3.Pos.Set(80, -80, -650.0);
	Sphere3.Rad = 50.0;
	Sphere3.Ka.Set(0.2, 0.2, 0.2);
	Sphere3.Kd.Set(0.0, 0.0, 0.7);
	Sphere3.Ks.Set(0.8, 0.8, 0.8);
	Sphere3.ns = 8.0;
	SphereList.push_back(Sphere3);

	GSphere Sphere4;
	Sphere4.Pos.Set(80, 80, -650.0);
	Sphere4.Rad = 50.0;
	Sphere4.Ka.Set(0.2, 0.2, 0.2);
	Sphere4.Kd.Set(0.0, 0.7, 0.7);
	Sphere4.Ks.Set(0.8, 0.8, 0.8);
	Sphere4.ns = 8.0;
	SphereList.push_back(Sphere4);

	// 이미지를 생성
	CreateImage();

	// 이벤트를 처리를 위한 무한 루프로 진입한다.
	glutMainLoop();
	
	return 0;
}

void Reshape(int w, int h)
{
	glViewport(0, 0, w, h);
}

void Render()
{
	// 칼라 버퍼와 깊이 버퍼 지우기
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// 칼라 버퍼에 Image 데이터를 직접 그린다.
	glDrawPixels(W, H, GL_RGB, GL_UNSIGNED_BYTE, Image);
	
	// 칼라 버퍼 교환한다
	glutSwapBuffers();
}

void Timer(int id)
{
	static double theta = 0.0;
	theta += 0.1;
	double x = 70 * cos(theta);
	double y = 70 * sin(theta);
	SphereList[0].Pos[0] = x;
	SphereList[0].Pos[1] = y;
	
	CreateImage();
	glutPostRedisplay();
	glutTimerFunc(1, Timer, 0);
}

void CreateImage()
{
	int x0 = -W / 2;
	int y0 = H / 2 - 1;
	double z = -(H / 2) / tan(M_PI * 15 / 180.0);
	for (int i = 0; i < H; ++i)
	{
		for (int j = 0; j < W; ++j)
		{
			double x = x0 + j;
			double y = y0 - i;
			GLine ray(GPos3(0.0, 0.0, 0.0), GPos3(x, y, z));
			GVec3 Color = RayTrace(ray, 0);

			int idx = ((H - 1 - i) * W + j) * 3;
			unsigned char r = (Color[0] > 1.0) ? 255 : (unsigned int)(Color[0] * 255);
 			unsigned char g = (Color[1] > 1.0) ? 255 : (unsigned int)(Color[1] * 255);
 			unsigned char b = (Color[2] > 1.0) ? 255 : (unsigned int)(Color[2] * 255);
			Image[idx] = r;
			Image[idx + 1] = g;
			Image[idx + 2] = b;
		}
	}
}

GVec3 RayTrace(GLine ray, int depth)
{
	// 종료 조건
	GVec3 C(0.0, 0.0, 0.0);
 	if (depth++ > MAX_DEPTH)
 		return C;
	int sidx; //광선과 교차하는 가장 가까운 구의 인덱스
	double t; // 교차점에서 광선의 파라미터 t
	

	//시점벡터 V
	GVec3 V = ray.v;
	V.Normalize();

	if (intersect_line_sphere(ray, sidx, t)) {
		
		GPos3 P = ray.Eval(t); //광선과 구의 교점 P
		//법선벡터 N구하기 - phong 함수에 이용
		GVec3 N = P - SphereList[sidx].Pos; //교차점 - 가까운 구의 중심점
		N.Normalize(); // 법선벡터 정규화

		//반사광선 ray_reflect
		GVec3 R = V - 2 * (N * V) * N;
		R.Normalize();
		GLine ray_reflect(P, R); //반사광선 생성

		// 교차점 색상 + 반사광 색상
		C = Phong(P, N, SphereList[sidx]) + 0.3 * RayTrace(ray_reflect, depth);

		//굴절광선 ray_refract -> 투명 구만 계산
		if (SphereList[sidx].bTransparent == true) {
			//내부 굴절광선
			double n1 = 1.0; double n2 = 1.1; //n1- 공기의 굴절률 n2- 구의 굴절률
			double cos1 = N * -1 * V; //코사인 세타1
			double cos2 = sqrt(1.0 - ((n1 / n2) * (n1 / n2) * (1.0 - cos1 * cos1))); //코사인 세타2
			GVec3 T = (n1 / n2) * V - (cos2 - (n1 / n2) * cos1) * N; //굴절광 방향벡터 T
			T.Normalize();
			GLine ray_refract(P, T); //내부 굴절광선 생성


			// 구 내부에서 밖으로 나가는 굴절광선 구하기
			// 나갈때 구와의 교차점 구하기
			double c = SphereList[sidx].Rad; //빗변
			double a = dist(ray_refract, SphereList[sidx].Pos); //점과 직선사이 거리
			double b = sqrt(c * c - a * a);
			GPos3 P0 = ray_refract.Eval(2 * b);

			// 나가는 굴절광선
			n1 = 1.1; n2 = 1.0;
			GVec3 N0 = SphereList[sidx].Pos - P0; // 나가는 광선 계산시 사용할 법선벡터
			N0.Normalize();
			GVec3 V0 = ray_refract.v;

			double cos01 = N0 * -1 * V0;
			double cos02 = sqrt(1.0 - (n1 / n2) * (n1 / n2) * (1.0 - cos01 * cos01));
			GVec3 T0 = (n1 / n2) * V0 - (cos02 - (n1 / n2) * cos01) * N0;
			T0.Normalize();
			GLine ray_refract1(P0, T0);

			C += 0.3 * RayTrace(ray_refract1, depth);
		}
	}
	return C;
}

bool intersect_line_sphere(GLine ray, int &sidx, double &t)
{
	bool flag = false; // 교차하는 다른구가 있는지 확인
	double tmp = 100000.0; //임시 idx값

	//구의 개수만큼 순회
	for (int i = 0; i < SphereList.size(); i++) {

		//점과 광선 사이의 거리
		double length = dist(ray, SphereList[i].Pos);

		//점과 광선사이의 거리 < 구의 반지름 -> 광선과 구가 교차함
		if (length <= SphereList[i].Rad) {

			//광선과 구의 교차점 t 찾기
			GVec3 U = ray.p - SphereList[i].Pos;
			GVec3 V = ray.v;
			V.Normalize();
			double R = SphereList[i].Rad;
			double product = U * V;
			double t0 = -product - sqrt(product * product - norm(U) * norm(U) + R * R); //2개의 t중 가까운 t 구하기
			//여러 t 중에서 양수이며 최솟값을 찾는다
			if (tmp > t0 && t0 > 0) {
				flag = true;
				tmp = t0;
				sidx = i;
			}
		}
	}
	if (flag == true) {
		t = tmp;
		return true;
	}
	else
		return false;
}


GVec3 Phong(GPos3 P, GVec3 N, GSphere Obj)
{
	
	//법선벡터 정규화
	N.Normalize();
	GVec3 C(0.0, 0.0, 0.0);
	double product1;
	double product2;
	for (int i = 0; i < LightList.size(); i++) {

		//시점벡터
		GVec3 V = GPos3(0.0, 0.0, 0.0) - P;
		V.Normalize();

		//광원벡터
		GVec3 L = LightList[i].Pos - P;
		L.Normalize();

		//내적값이 양수일때만
		if (N * L > 0.0)
			product1 = N * L;
		else
			product1 = 0.0;

		//반사광선 구하기
		GVec3 R = 2 * product1 * N - L;
		R.Normalize();

		//내적값이 양수일때만
		if (V * R > 0.0)
			product2 = V * R;
		else
			product2 = 0.0;

		for (int j = 0; j < 3; j++)
			C[j] += Obj.Ka[j] * LightList[i].Ia[j] + Obj.Kd[j] * LightList[i].Id[j] * product1 + Obj.Ks[j] * LightList[i].Is[j] * pow(product2, Obj.ns);
	}
	return C;
}