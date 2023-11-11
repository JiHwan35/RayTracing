#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "usr\include\GL\freeglut.h"
#include "gmath.h"

// ��ũ�� ��� ����
#define MAX_DEPTH 1
#define H 768
#define W 1024
unsigned char Image[H * W * 3];

std::vector<GSphere> SphereList;
std::vector<GLight> LightList;

// �ݹ� �Լ� ����
void Render();
void Reshape(int w, int h);
void Timer(int id);

// ���� ���� �Լ�
void CreateImage();
GVec3 RayTrace(GLine ray, int depth);
GVec3 Phong(GPos3 P, GVec3 N, GSphere Obj);
bool intersect_line_sphere(GLine ray, int &sidx, double &t);

int main(int argc, char **argv)
{
	// OpenGL �ʱ�ȭ, ������ ũ�� ����, ���÷��� ��� ����
	glutInit(&argc, argv);
	glutInitWindowSize(W, H);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);

	// ������ ���� �� �ݹ� �Լ� ���
	glutCreateWindow("RayTracer");
	glutDisplayFunc(Render);
	glutReshapeFunc(Reshape);
	glutTimerFunc(10, Timer, 0);

	// ���� ����
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
	
	// ��鿡 5���� ���� ��ġ�Ѵ�.
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

	// �̹����� ����
	CreateImage();

	// �̺�Ʈ�� ó���� ���� ���� ������ �����Ѵ�.
	glutMainLoop();
	
	return 0;
}

void Reshape(int w, int h)
{
	glViewport(0, 0, w, h);
}

void Render()
{
	// Į�� ���ۿ� ���� ���� �����
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Į�� ���ۿ� Image �����͸� ���� �׸���.
	glDrawPixels(W, H, GL_RGB, GL_UNSIGNED_BYTE, Image);
	
	// Į�� ���� ��ȯ�Ѵ�
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
	// ���� ����
	GVec3 C(0.0, 0.0, 0.0);
 	if (depth++ > MAX_DEPTH)
 		return C;
	int sidx; //������ �����ϴ� ���� ����� ���� �ε���
	double t; // ���������� ������ �Ķ���� t
	

	//�������� V
	GVec3 V = ray.v;
	V.Normalize();

	if (intersect_line_sphere(ray, sidx, t)) {
		
		GPos3 P = ray.Eval(t); //������ ���� ���� P
		//�������� N���ϱ� - phong �Լ��� �̿�
		GVec3 N = P - SphereList[sidx].Pos; //������ - ����� ���� �߽���
		N.Normalize(); // �������� ����ȭ

		//�ݻ籤�� ray_reflect
		GVec3 R = V - 2 * (N * V) * N;
		R.Normalize();
		GLine ray_reflect(P, R); //�ݻ籤�� ����

		// ������ ���� + �ݻ籤 ����
		C = Phong(P, N, SphereList[sidx]) + 0.3 * RayTrace(ray_reflect, depth);

		//�������� ray_refract -> ���� ���� ���
		if (SphereList[sidx].bTransparent == true) {
			//���� ��������
			double n1 = 1.0; double n2 = 1.1; //n1- ������ ������ n2- ���� ������
			double cos1 = N * -1 * V; //�ڻ��� ��Ÿ1
			double cos2 = sqrt(1.0 - ((n1 / n2) * (n1 / n2) * (1.0 - cos1 * cos1))); //�ڻ��� ��Ÿ2
			GVec3 T = (n1 / n2) * V - (cos2 - (n1 / n2) * cos1) * N; //������ ���⺤�� T
			T.Normalize();
			GLine ray_refract(P, T); //���� �������� ����


			// �� ���ο��� ������ ������ �������� ���ϱ�
			// ������ ������ ������ ���ϱ�
			double c = SphereList[sidx].Rad; //����
			double a = dist(ray_refract, SphereList[sidx].Pos); //���� �������� �Ÿ�
			double b = sqrt(c * c - a * a);
			GPos3 P0 = ray_refract.Eval(2 * b);

			// ������ ��������
			n1 = 1.1; n2 = 1.0;
			GVec3 N0 = SphereList[sidx].Pos - P0; // ������ ���� ���� ����� ��������
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
	bool flag = false; // �����ϴ� �ٸ����� �ִ��� Ȯ��
	double tmp = 100000.0; //�ӽ� idx��

	//���� ������ŭ ��ȸ
	for (int i = 0; i < SphereList.size(); i++) {

		//���� ���� ������ �Ÿ�
		double length = dist(ray, SphereList[i].Pos);

		//���� ���������� �Ÿ� < ���� ������ -> ������ ���� ������
		if (length <= SphereList[i].Rad) {

			//������ ���� ������ t ã��
			GVec3 U = ray.p - SphereList[i].Pos;
			GVec3 V = ray.v;
			V.Normalize();
			double R = SphereList[i].Rad;
			double product = U * V;
			double t0 = -product - sqrt(product * product - norm(U) * norm(U) + R * R); //2���� t�� ����� t ���ϱ�
			//���� t �߿��� ����̸� �ּڰ��� ã�´�
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
	
	//�������� ����ȭ
	N.Normalize();
	GVec3 C(0.0, 0.0, 0.0);
	double product1;
	double product2;
	for (int i = 0; i < LightList.size(); i++) {

		//��������
		GVec3 V = GPos3(0.0, 0.0, 0.0) - P;
		V.Normalize();

		//��������
		GVec3 L = LightList[i].Pos - P;
		L.Normalize();

		//�������� ����϶���
		if (N * L > 0.0)
			product1 = N * L;
		else
			product1 = 0.0;

		//�ݻ籤�� ���ϱ�
		GVec3 R = 2 * product1 * N - L;
		R.Normalize();

		//�������� ����϶���
		if (V * R > 0.0)
			product2 = V * R;
		else
			product2 = 0.0;

		for (int j = 0; j < 3; j++)
			C[j] += Obj.Ka[j] * LightList[i].Ia[j] + Obj.Kd[j] * LightList[i].Id[j] * product1 + Obj.Ks[j] * LightList[i].Is[j] * pow(product2, Obj.ns);
	}
	return C;
}