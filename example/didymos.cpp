#include<GL/glew.h>
#include<GLFW/glfw3.h>
#include<glm/glm.hpp>
#include<glm/gtc/matrix_transform.hpp>
#include<glm/gtc/type_ptr.hpp>

#include<cstdio>
#include<string>
#include<array>

#include"../include/shader.hpp"
#include"../include/mesh.hpp"
#include"../include/camera.hpp"
#include"../include/font.hpp"

typedef std::array<double, 2> vec2;
typedef std::array<double, 3> vec3;
typedef std::array<double, 4> vec4;
typedef std::array<double, 20> vec20;
typedef std::array<vec3, 3> mat3;

float t1 = 0.0f, t2, delta_time;
float xpos_previous, ypos_previous;
bool first_time_entered_the_window = true;

int win_width, win_height;
float aspect_ratio;

double tglfw, tprev, ms_per_frame = 1000.0;
int frames = 0;

camera cam(glm::vec3(0.0f,-5.0f,2.0f));

double G,M1,M2;
mat3 I1,I2;
double duration,dt;

///////////////////////////////////////////////////////////////////////////////////////////

vec3 operator+(const vec3 &v1, const vec3 &v2)
{
    return {v1[0] + v2[0], v1[1] + v2[1], v1[2] + v2[2]};
}

vec4 operator+(const vec4 &v1, const vec4 &v2)
{
    return {v1[0] + v2[0], v1[1] + v2[1], v1[2] + v2[2], v1[3] + v2[3]};
}

vec3 operator-(const vec3 &v1, const vec3 &v2)
{
    return {v1[0] - v2[0], v1[1] - v2[1], v1[2] - v2[2]};
}

vec3 operator-(const vec3 &v)
{
    return {-v[0], -v[1], -v[2]};
}

vec3 operator*(const double c, const vec3 &v)
{
    return {c*v[0], c*v[1], c*v[2]};
}

vec3 operator*(const vec3 &v, const double c)
{
    return {v[0]*c, v[1]*c, v[2]*c};
}

vec4 operator*(const double c, const vec4 &v)
{
    return {c*v[0], c*v[1], c*v[2], c*v[3]};
}

vec4 operator*(const vec4 &v, const double c)
{
    return {v[0]*c, v[1]*c, v[2]*c, v[3]*c};
}

vec3 operator/(const vec3 &v, const double c)
{
    return {v[0]/c, v[1]/c, v[2]/c};
}

vec4 operator/(const vec4 &v, const double c)
{
    return {v[0]/c, v[1]/c, v[2]/c, v[3]/c};
}

mat3 transpose(const mat3 &A)
{
    return {{{A[0][0], A[1][0], A[2][0]},
             {A[0][1], A[1][1], A[2][1]},
             {A[0][2], A[1][2], A[2][2]}}};
}

vec3 dot(const mat3 &A, const vec3 &v)
{
    return { A[0][0]*v[0] + A[0][1]*v[1] + A[0][2]*v[2],
             A[1][0]*v[0] + A[1][1]*v[1] + A[1][2]*v[2],
             A[2][0]*v[0] + A[2][1]*v[1] + A[2][2]*v[2] };
}

double dot(const vec3 &v1, const vec3 &v2)
{
    return v1[0]*v2[0] + v1[1]*v2[1] + v1[2]*v2[2];
}

vec3 dot(const vec3 &v, const mat3 &A)
{
    return { v[0]*A[0][0] + v[1]*A[1][0] + v[2]*A[2][0],
             v[0]*A[0][1] + v[1]*A[1][1] + v[2]*A[2][1],
             v[0]*A[0][2] + v[1]*A[1][2] + v[2]*A[2][2] };
}

vec3 cross(const vec3 &v1, const vec3 &v2)
{
    return { v1[1]*v2[2] - v1[2]*v2[1],
             v1[2]*v2[0] - v1[0]*v2[2],
             v1[0]*v2[1] - v1[1]*v2[0] };
}

///////////////////////////////////////////////////////////////////////////////////////////

vec4 quat2unit(const vec4 &q)
{
    return q/sqrt(q[0]*q[0] + q[1]*q[1] + q[2]*q[2] + q[3]*q[3]);
}

vec3 quat2ang(const vec4 &q)
{
    //roll
    double roll = atan2( 2.0*(q[2]*q[3] + q[0]*q[1]), q[0]*q[0] - q[1]*q[1] - q[2]*q[2] + q[3]*q[3] );
    //pitch
    double pitch, coeff = q[1]*q[3] - q[0]*q[2];
    if (-2.0*coeff >= 1.0)
        pitch = M_PI_2;
    else if (-2.0*coeff <= -1.0)
        pitch = -M_PI_2;
    else
        pitch = asin(-2.0*coeff);
    //yaw
    double yaw = atan2( 2.0*(q[1]*q[2] + q[0]*q[3]), q[0]*q[0] + q[1]*q[1] - q[2]*q[2] - q[3]*q[3] );
    return {roll, pitch, yaw};
}

vec3 iner2body(const vec3 &viner, const mat3 &A)
{
    return dot(transpose(A), viner);
}

vec3 body2iner(const vec3 &vbody, const mat3 &A)
{
    return dot(A, vbody);
}

mat3 quat2mat(const vec4 &q)
{
    double a11 = q[0]*q[0] + q[1]*q[1] - q[2]*q[2] - q[3]*q[3];
    double a12 = 2*(q[1]*q[2] - q[0]*q[3]);
    double a13 = 2*(q[1]*q[3] + q[0]*q[2]);
    double a21 = 2*(q[1]*q[2] + q[0]*q[3]);
    double a22 = q[0]*q[0] - q[1]*q[1] + q[2]*q[2] - q[3]*q[3];
    double a23 = 2*(q[2]*q[3] - q[0]*q[1]);
    double a31 = 2*(q[1]*q[3] - q[0]*q[2]);
    double a32 = 2*(q[2]*q[3] + q[0]*q[1]);
    double a33 = q[0]*q[0] - q[1]*q[1] - q[2]*q[2] + q[3]*q[3];
    return {{{a11,a12,a13},
             {a21,a22,a23},
             {a31,a32,a33}}};
}

///////////////////////////////////////////////////////////////////////////////////////////

mat3 ellipsoid_inertia(const vec3 &semiaxes, const double M)
{
    double a = semiaxes[0], b = semiaxes[1], c = semiaxes[2];
    double Ix = M*(b*b + c*c)/5.0;
    double Iy = M*(a*a + c*c)/5.0;
    double Iz = M*(a*a + b*b)/5.0;
    return {{{Ix,0.0,0.0},
             {0.0,Iy,0.0},
             {0.0,0.0,Iz}}};
}

vec4 quat_rhs(const vec4 &q, const vec3 &w)
{
    double dq0 = 0.5*(-q[1]*w[0] - q[2]*w[1] - q[3]*w[2]);
    double dq1 = 0.5*( q[0]*w[0] - q[3]*w[1] + q[2]*w[2]);
    double dq2 = 0.5*( q[3]*w[0] + q[0]*w[1] - q[1]*w[2]);
    double dq3 = 0.5*(-q[2]*w[0] + q[1]*w[1] + q[0]*w[2]);
    return {dq0,dq1,dq2,dq3};
}

vec3 euler_rhs(const vec3 &w, const mat3 &I, const vec3 &tau)
{
    double dw0 = (tau[0] + w[1]*w[2]*(I[1][1] - I[2][2]))/I[0][0];
    double dw1 = (tau[1] + w[2]*w[0]*(I[2][2] - I[0][0]))/I[1][1];
    double dw2 = (tau[2] + w[0]*w[1]*(I[0][0] - I[1][1]))/I[2][2];
    return {dw0,dw1,dw2};
}

///////////////////////////////////////////////////////////////////////////////////////////

double potential(double M1, double M2, const mat3 &I1, const mat3 &I2, const vec3 &r, const mat3 &A1, const mat3 &A2)
{
    double I1x = I1[0][0], I1y = I1[1][1], I1z = I1[2][2];
    double I2x = I2[0][0], I2y = I2[1][1], I2z = I2[2][2];

    vec3 a1 = {A1[0][0], A1[1][0], A1[2][0]};
    vec3 a2 = {A1[0][1], A1[1][1], A1[2][1]};
    vec3 a3 = {A1[0][2], A1[1][2], A1[2][2]};

    vec3 b1 = {A2[0][0], A2[1][0], A2[2][0]};
    vec3 b2 = {A2[0][1], A2[1][1], A2[2][1]};
    vec3 b3 = {A2[0][2], A2[1][2], A2[2][2]};

    double d = sqrt(r[0]*r[0] + r[1]*r[1] + r[2]*r[2]);
    vec3 ru = r/d;

    double l1 = dot(ru,a1);
    double m1 = dot(ru,a2);
    double n1 = dot(ru,a3);

    double l2 = dot(ru,b1);
    double m2 = dot(ru,b2);
    double n2 = dot(ru,b3);

    double V0 = -G*M1*M2/d;

    double V2 = -(G*M2/(2*d*d*d))*( (1 - 3*l1*l1)*I1x + (1 - 3*m1*m1)*I1y + (1 - 3*n1*n1)*I1z ) +
                -(G*M1/(2*d*d*d))*( (1 - 3*l2*l2)*I2x + (1 - 3*m2*m2)*I2y + (1 - 3*n2*n2)*I2z );

    return V0 + V2;
}

vec3 force(double M1, double M2, const mat3 &I1, const mat3 &I2, const vec3 &r, const mat3 &A1, const mat3 &A2)
{
    double I1x = I1[0][0], I1y = I1[1][1], I1z = I1[2][2];
    double I2x = I2[0][0], I2y = I2[1][1], I2z = I2[2][2];

    vec3 a1 = {A1[0][0], A1[1][0], A1[2][0]};
    vec3 a2 = {A1[0][1], A1[1][1], A1[2][1]};
    vec3 a3 = {A1[0][2], A1[1][2], A1[2][2]};

    vec3 b1 = {A2[0][0], A2[1][0], A2[2][0]};
    vec3 b2 = {A2[0][1], A2[1][1], A2[2][1]};
    vec3 b3 = {A2[0][2], A2[1][2], A2[2][2]};

    double d = sqrt(r[0]*r[0] + r[1]*r[1] + r[2]*r[2]);
    vec3 ru = r/d;

    double l1 = dot(ru,a1);
    double m1 = dot(ru,a2);
    double n1 = dot(ru,a3);

    double l2 = dot(ru,b1);
    double m2 = dot(ru,b2);
    double n2 = dot(ru,b3);

    double V0 = -G*M1*M2/d;

    double V2 = -(G*M2/(2*d*d*d))*( (1 - 3*l1*l1)*I1x + (1 - 3*m1*m1)*I1y + (1 - 3*n1*n1)*I1z ) +
                -(G*M1/(2*d*d*d))*( (1 - 3*l2*l2)*I2x + (1 - 3*m2*m2)*I2y + (1 - 3*n2*n2)*I2z );

    double dV_dd = -V0/d - 3*V2/d;
    vec3 dd_dr = ru;

    double dV_dl1 = 3*G*I1x*M2*l1/(d*d*d);
    double dV_dm1 = 3*G*I1y*M2*m1/(d*d*d);
    double dV_dn1 = 3*G*I1z*M2*n1/(d*d*d);
    vec3 dl1_dr = (a1*d-r*l1)/(d*d);
    vec3 dm1_dr = (a2*d-r*m1)/(d*d);
    vec3 dn1_dr = (a3*d-r*n1)/(d*d);

    double dV_dl2 = 3*G*I2x*M1*l2/(d*d*d);
    double dV_dm2 = 3*G*I2y*M1*m2/(d*d*d);
    double dV_dn2 = 3*G*I2z*M1*n2/(d*d*d);
    vec3 dl2_dr = (b1*d-r*l2)/(d*d);
    vec3 dm2_dr = (b2*d-r*m2)/(d*d);
    vec3 dn2_dr = (b3*d-r*n2)/(d*d);

    vec3 dV_dr = dV_dd*dd_dr + dV_dl1*dl1_dr + dV_dm1*dm1_dr + dV_dn1*dn1_dr +
                 dV_dl2*dl2_dr + dV_dm2*dm2_dr + dV_dn2*dn2_dr;

    return dV_dr;
}

vec3 torque1(double M2, const mat3 &I1, const vec3 &r, const mat3 &A1)
{
    double I1x = I1[0][0], I1y = I1[1][1], I1z = I1[2][2];

    vec3 a1 = {A1[0][0], A1[1][0], A1[2][0]};
    vec3 a2 = {A1[0][1], A1[1][1], A1[2][1]};
    vec3 a3 = {A1[0][2], A1[1][2], A1[2][2]};

    double d = sqrt(r[0]*r[0] + r[1]*r[1] + r[2]*r[2]);
    vec3 ru = r/d;

    double l1 = dot(ru,a1);
    double m1 = dot(ru,a2);
    double n1 = dot(ru,a3);

    double dV_dl1 = 3*G*I1x*M2*l1/(d*d*d);
    double dV_dm1 = 3*G*I1y*M2*m1/(d*d*d);
    double dV_dn1 = 3*G*I1z*M2*n1/(d*d*d);

    vec3 dl1_da1 = ru;
    vec3 dm1_da2 = ru;
    vec3 dn1_da3 = ru;

    vec3 dV_da1 = dV_dl1*dl1_da1;
    vec3 dV_da2 = dV_dm1*dm1_da2;
    vec3 dV_da3 = dV_dn1*dn1_da3;

    vec3 tau1 = -cross(a1, dV_da1) - cross(a2, dV_da2) - cross(a3, dV_da3);

    return tau1;
}

vec3 torque2(double M1, const mat3 &I2, const vec3 &r, const mat3 &A2)
{
    double I2x = I2[0][0], I2y = I2[1][1], I2z = I2[2][2];

    vec3 b1 = {A2[0][0], A2[1][0], A2[2][0]};
    vec3 b2 = {A2[0][1], A2[1][1], A2[2][1]};
    vec3 b3 = {A2[0][2], A2[1][2], A2[2][2]};

    double d = sqrt(r[0]*r[0] + r[1]*r[1] + r[2]*r[2]);
    vec3 ru = r/d;

    double l2 = dot(ru,b1);
    double m2 = dot(ru,b2);
    double n2 = dot(ru,b3);

    double dV_dl2 = 3*G*I2x*M1*l2/(d*d*d);
    double dV_dm2 = 3*G*I2y*M1*m2/(d*d*d);
    double dV_dn2 = 3*G*I2z*M1*n2/(d*d*d);

    vec3 dl2_db1 = ru;
    vec3 dm2_db2 = ru;
    vec3 dn2_db3 = ru;

    vec3 dV_db1 = dV_dl2*dl2_db1;
    vec3 dV_db2 = dV_dm2*dm2_db2;
    vec3 dV_db3 = dV_dn2*dn2_db3;

    vec3 tau2 = -cross(b1, dV_db1) - cross(b2, dV_db2) - cross(b3, dV_db3);

    return tau2;
}

vec2 ener_mom(const vec20 &state)
{
    vec3 r   = { state[0], state[1], state[2] };
    vec3 v   = { state[3], state[4], state[5] };
    vec4 q1  = { state[6], state[7], state[8],  state[9] };
    vec3 w1b = { state[10], state[11], state[12] };
    vec4 q2  = { state[13], state[14], state[15], state[16] };
    vec3 w2b = { state[17], state[18], state[19] };

    mat3 A1 = quat2mat(quat2unit(q1));
    mat3 A2 = quat2mat(quat2unit(q2));

    double E = 0.5*((M1*M2/(M1+M2))*dot(v,v) + dot(dot(w1b,I1), w1b) + dot(dot(w2b,I2), w2b)) + potential(M1,M2, I1,I2, r, A1,A2);
    vec3 L = (M1*M2/(M1+M2))*cross(r,v) + dot(A1, dot(I1,w1b)) + dot(A2, dot(I2,w2b));

    return {E, sqrt(L[0]*L[0] + L[1]*L[1] + L[2]*L[2])};
}

///////////////////////////////////////////////////////////////////////////////////////////

vec3 fr(const vec3 &v)
{
    return v;
}

vec3 fv(const vec3 &r, const vec4 &q1, const vec4 &q2)
{
    mat3 A1 = quat2mat(quat2unit(q1));
    mat3 A2 = quat2mat(quat2unit(q2));
    vec3 gradV = force(M1,M2, I1,I2, r, A1,A2);
    return -gradV/(M1*M2/(M1+M2));
}

vec4 fq1(const vec4 &q1, const vec3 &w1b)
{
    return quat_rhs(q1,w1b);
}

vec3 fw1(const vec3 &r, const vec4 &q1, const vec3 &w1b)
{
    mat3 A1 = quat2mat(quat2unit(q1));
    vec3 tau1i = torque1(M2, I1, r, A1);
    vec3 tau1b = iner2body(tau1i,A1);
    return euler_rhs(w1b,I1,tau1b);
}

vec4 fq2(const vec4 &q2, const vec3 &w2b)
{
    return quat_rhs(q2,w2b);
}

vec3 fw2(const vec3 &r, const vec4 &q2, const vec3 &w2b)
{
    mat3 A2 = quat2mat(quat2unit(q2));
    vec3 tau2i = torque2(M1, I2, r, A2);
    vec3 tau2b = iner2body(tau2i,A2);
    return euler_rhs(w2b,I2,tau2b);
}

void rk4_step(vec20 &state)
{
    vec3 r   = { state[0], state[1], state[2] };
    vec3 v   = { state[3], state[4], state[5] };
    vec4 q1  = { state[6], state[7], state[8],  state[9] };
    vec3 w1b = { state[10], state[11], state[12] };
    vec4 q2  = { state[13], state[14], state[15], state[16] };
    vec3 w2b = { state[17], state[18], state[19] };

    vec3 kr = fr(v);
    vec3 kv = fv(r,q1,q2);
    vec4 kq1 = fq1(q1,w1b);
    vec3 kw1b = fw1(r,q1,w1b);
    vec4 kq2 = fq2(q2,w2b);
    vec3 kw2b = fw2(r,q2,w2b);

    vec3 lr = fr(v + 0.5*dt*kv);
    vec3 lv = fv(r + 0.5*dt*kr, q1 + 0.5*dt*kq1, q2 + 0.5*dt*kq2);
    vec4 lq1 = fq1(q1 + 0.5*dt*kq1, w1b + 0.5*dt*kw1b);
    vec3 lw1b = fw1(r + 0.5*dt*kr, q1 + 0.5*dt*kq1, w1b + 0.5*dt*kw1b);
    vec4 lq2 = fq2(q2 + 0.5*dt*kq2, w2b + 0.5*dt*kw2b);
    vec3 lw2b = fw2(r + 0.5*dt*kr, q2 + 0.5*dt*kq2, w2b + 0.5*dt*kw2b);

    vec3 mr = fr(v + 0.5*dt*lv);
    vec3 mv = fv(r + 0.5*dt*lr, q1 + 0.5*dt*lq1, q2 + 0.5*dt*lq2);
    vec4 mq1 = fq1(q1 + 0.5*dt*lq1, w1b + 0.5*dt*lw1b);
    vec3 mw1b = fw1(r + 0.5*dt*lr, q1 + 0.5*dt*lq1, w1b + 0.5*dt*lw1b);
    vec4 mq2 = fq2(q2 + 0.5*dt*lq2, w2b + 0.5*dt*lw2b);
    vec3 mw2b = fw2(r + 0.5*dt*lr, q2 + 0.5*dt*lq2, w2b + 0.5*dt*lw2b);

    vec3 nr = fr(v + dt*mv);
    vec3 nv = fv(r + dt*mr, q1 + dt*mq1, q2 + dt*mq2);
    vec4 nq1 = fq1(q1 + dt*mq1, w1b + dt*mw1b);
    vec3 nw1b = fw1(r + dt*mr, q1 + dt*mq1, w1b + dt*mw1b);
    vec4 nq2 = fq2(q2 + dt*mq2, w2b + dt*mw2b);
    vec3 nw2b = fw2(r + dt*mr, q2 + dt*mq2, w2b + dt*mw2b);

    r   = r   + (dt/6.0)*(kr   + 2.0*lr   + 2.0*mr   + nr);
    v   = v   + (dt/6.0)*(kv   + 2.0*lv   + 2.0*mv   + nv);
    q1  = q1  + (dt/6.0)*(kq1  + 2.0*lq1  + 2.0*mq1  + nq1);
    w1b = w1b + (dt/6.0)*(kw1b + 2.0*lw1b + 2.0*mw1b + nw1b);
    q2  = q2  + (dt/6.0)*(kq2  + 2.0*lq2  + 2.0*mq2  + nq2);
    w2b = w2b + (dt/6.0)*(kw2b + 2.0*lw2b + 2.0*mw2b + nw2b);

    state[0] = r[0];
    state[1] = r[1];
    state[2] = r[2];

    state[3] = v[0];
    state[4] = v[1];
    state[5] = v[2];

    state[6] = q1[0];
    state[7] = q1[1];
    state[8] = q1[2];
    state[9] = q1[3];

    state[10] = w1b[0];
    state[11] = w1b[1];
    state[12] = w1b[2];

    state[13] = q2[0];
    state[14] = q2[1];
    state[15] = q2[2];
    state[16] = q2[3];

    state[17] = w2b[0];
    state[18] = w2b[1];
    state[19] = w2b[2];

    return;
}

///////////////////////////////////////////////////////////////////////////////////////////

void process_hardware_inputs(GLFWwindow *win)
{
    if (glfwGetKey(win, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(win, true);

    if (glfwGetKey(win, GLFW_KEY_W) == GLFW_PRESS)
        cam.translate_front(delta_time);
    if (glfwGetKey(win, GLFW_KEY_S) == GLFW_PRESS)
        cam.translate_back(delta_time);
    if (glfwGetKey(win, GLFW_KEY_D) == GLFW_PRESS)
        cam.translate_right(delta_time);
    if (glfwGetKey(win, GLFW_KEY_A) == GLFW_PRESS)
        cam.translate_left(delta_time);
}

void cursor_pos_callback(GLFWwindow *win, double xpos, double ypos)
{
    if (first_time_entered_the_window)
    {
        xpos_previous = xpos;
        ypos_previous = ypos;
        first_time_entered_the_window = false;
    }

    float xoffset = xpos - xpos_previous;
    float yoffset = ypos - ypos_previous;

    xpos_previous = xpos;
    ypos_previous = ypos;

    cam.rotate(xoffset, yoffset);
}

void framebuffer_size_callback(GLFWwindow *win, int w, int h)
{
    if (w == 0) w = 1;
    if (h == 0) h = 1;
    glViewport(0,0,w,h);
    win_width = w;
    win_height = h;
    aspect_ratio = (float)w/h;
}

int main()
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_SAMPLES, 4);
    glfwWindowHint(GLFW_MAXIMIZED, GLFW_TRUE);
    GLFWwindow *win = glfwCreateWindow(800, 600, "65803 Didymos", NULL, NULL);
    if (win == NULL)
    {
        printf("Failed to create glfw window. Exiting...\n");
        glfwTerminate();
        return 0;
    }
    glfwMakeContextCurrent(win);
    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK)
    {
        printf("Failed to initialize glew. Exiting...\n");
        glfwTerminate();
        return 0;
    }

    const unsigned char *gpu_vendor = glGetString(GL_VENDOR);

    glfwSetWindowSizeLimits(win, 400, 400, GLFW_DONT_CARE, GLFW_DONT_CARE);
    glfwSetFramebufferSizeCallback(win, framebuffer_size_callback);
    glfwSetCursorPosCallback(win, cursor_pos_callback);
    glfwSetInputMode(win, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwGetWindowSize(win, &win_width, &win_height);
    aspect_ratio = (float)win_width/win_height;
    xpos_previous = win_width/2.0f;
    ypos_previous = win_height/2.0f;

    mesh aster1("../obj/vert_face_fnorm/didymos_binary/didymain2019.obj",1,1,1);
    mesh aster1_axis_x("../obj/vert_face_fnorm/didymos_binary/didymain_positive_axis_x.obj",1,1,1);
    mesh aster1_axis_y("../obj/vert_face_fnorm/didymos_binary/didymain_positive_axis_y.obj",1,1,1);
    mesh aster1_axis_z("../obj/vert_face_fnorm/didymos_binary/didymain_positive_axis_z.obj",1,1,1);

    mesh aster2("../obj/vert_face_fnorm/didymos_binary/dimorphos_ellipsoid.obj",1,1,1);
    mesh aster2_axis_x("../obj/vert_face_fnorm/didymos_binary/dimorphos_positive_axis_x.obj",1,1,1);
    mesh aster2_axis_y("../obj/vert_face_fnorm/didymos_binary/dimorphos_positive_axis_y.obj",1,1,1);
    mesh aster2_axis_z("../obj/vert_face_fnorm/didymos_binary/dimorphos_positive_axis_z.obj",1,1,1);

    font ttf("../font/NotoSansRegular.ttf");

    shader mvpn_plight_ad("../shader/trans_mvpn.vert","../shader/point_light_ad.frag");
    shader text_shad("../shader/text.vert","../shader/text.frag");

    glm::vec3 light_pos = glm::vec3(0.0f,-100.0f,0.0f);
    glm::vec3 light_col = glm::vec3(1.0f,1.0f,1.0f);
    glm::vec3 aster_col = glm::vec3(0.5f,0.5f,0.5f);
    glm::vec3 axis_x_col = glm::vec3(1.0f,0.0f,0.0f);
    glm::vec3 axis_y_col = glm::vec3(0.0f,1.0f,0.0f);
    glm::vec3 axis_z_col = glm::vec3(0.0f,0.0f,1.0f);

    /////////////////////////////////////////////////

    G = 6.67430e-20;
    M1 = 5.320591856403073e+11;
    M2 = 4.940814359692687e+09;
    vec3 semiaxes1 = {0.416194, 0.418765, 0.39309};
    vec3 semiaxes2 = {0.104, 0.080, 0.066};
    vec3 r   = {1.19, 0.0, 0.0};
    vec3 v   = {0.0, 0.00017421523858789, 0.0};
    vec4 q1  = {1.0, 0.0, 0.0, 0.0};
    vec3 w1i = {0.0, 0.0, 0.000772269580528465};
    vec4 q2  = {1.0, 0.0, 0.0, 0.0};
    vec3 w2i = {0.0, 0.0, 0.000146399360157891};
    duration = 0.0;
    dt = 10.0;
    I1 = ellipsoid_inertia(semiaxes1, M1);
    I2 = ellipsoid_inertia(semiaxes2, M2);
    vec3 w1b = iner2body(w1i, quat2mat(quat2unit(q1)));
    vec3 w2b = iner2body(w2i, quat2mat(quat2unit(q2)));

    vec20 state = { r[0],r[1],r[2],
                    v[0],v[1],v[2],
                    q1[0],q1[1],q1[2],q1[3],
                    w1b[0],w1b[1],w1b[2],
                    q2[0],q2[1],q2[2],q2[3],
                    w2b[0],w2b[1],w2b[2] };
    vec2 ener0_mom0 = ener_mom(state);

    /////////////////////////////////////////////////

    glm::mat4 projection, view, model;

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glClearColor(0.05f,0.05f,0.05f,1.0f);

    tprev = glfwGetTime();
    while (!glfwWindowShouldClose(win))
    {
        t2 = glfwGetTime(); 
        delta_time = t2 - t1;
        t1 = t2;
        tglfw = glfwGetTime();
        frames++;
        if (tglfw - tprev >= 1.0)
        {
            ms_per_frame = 1000.0/(double)frames;
            frames = 0;
            tprev += 1.0;
        }

        process_hardware_inputs(win);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        projection = glm::perspective(glm::radians(45.0f), aspect_ratio, 0.01f, 5000.0f);
        view = cam.view();
        model = glm::mat4(1.0f);

        mvpn_plight_ad.use();
        mvpn_plight_ad.set_mat4_uniform("projection", projection);
        mvpn_plight_ad.set_mat4_uniform("view", view);
        mvpn_plight_ad.set_mat4_uniform("model", model);
        mvpn_plight_ad.set_vec3_uniform("cam_pos", cam.pos);
        mvpn_plight_ad.set_vec3_uniform("light_pos", light_pos);
        mvpn_plight_ad.set_vec3_uniform("light_col", light_col);
        mvpn_plight_ad.set_vec3_uniform("model_col", aster_col);

        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3((float)(-M2/(M1 + M2))*state[0],(float)(-M2/(M1 + M2))*state[1],(float)(-M2/(M1 + M2))*state[2]));
        vec3 rpy1 = quat2ang({state[6], state[7], state[8], state[9]});
        model = glm::rotate(model, (float)rpy1[2], glm::vec3(0.0f,0.0f,1.0f));
        model = glm::rotate(model, (float)rpy1[1], glm::vec3(0.0f,1.0f,0.0f));
        model = glm::rotate(model, (float)rpy1[0], glm::vec3(1.0f,0.0f,0.0f));
        mvpn_plight_ad.use();
        mvpn_plight_ad.set_mat4_uniform("model", model);
        aster1.draw_triangles();

        mvpn_plight_ad.use();
        mvpn_plight_ad.set_vec3_uniform("model_col", axis_x_col);
        aster1_axis_x.draw_triangles();
        mvpn_plight_ad.set_vec3_uniform("model_col", axis_y_col);
        aster1_axis_y.draw_triangles();
        mvpn_plight_ad.set_vec3_uniform("model_col", axis_z_col);
        aster1_axis_z.draw_triangles();

        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3((M1/(M1 + M2))*state[0],(M1/(M1 + M2))*state[1],(M1/(M1 + M2))*state[2]));
        vec3 rpy2 = quat2ang({state[13], state[14], state[15], state[16]});
        model = glm::rotate(model, (float)rpy2[2], glm::vec3(0.0f,0.0f,1.0f));
        model = glm::rotate(model, (float)rpy2[1], glm::vec3(0.0f,1.0f,0.0f));
        model = glm::rotate(model, (float)rpy2[0], glm::vec3(1.0f,0.0f,0.0f));
        mvpn_plight_ad.use();
        mvpn_plight_ad.set_mat4_uniform("model", model);
        mvpn_plight_ad.set_vec3_uniform("model_col", aster_col);
        aster2.draw_triangles();

        mvpn_plight_ad.use();
        mvpn_plight_ad.set_vec3_uniform("model_col", axis_x_col);
        aster2_axis_x.draw_triangles();
        mvpn_plight_ad.set_vec3_uniform("model_col", axis_y_col);
        aster2_axis_y.draw_triangles();
        mvpn_plight_ad.set_vec3_uniform("model_col", axis_z_col);
        aster2_axis_z.draw_triangles();
        
        char text[100];

        sprintf(text, "time :  %.2f [ days ]", duration);
        ttf.draw(text, 20.0f, win_height - 30.0f, win_width, win_height, 0.4f, glm::vec3(0.0f,0.4f,1.0f), text_shad);

        sprintf(text, "( x, y, z ) :  ( %.2f, %.2f, %.2f ) [ km ]", state[0], state[1], state[2]);
        ttf.draw(text, 20.0f, win_height - 60.0f, win_width, win_height, 0.4f, glm::vec3(0.0f,0.4f,1.0f), text_shad);

        sprintf(text, "( roll 1, pitch 1, yaw 1 ) :  ( %.2f, %.2f, %.2f ) [ deg ]", rpy1[0]*180.0/M_PI, rpy1[1]*180.0/M_PI, rpy1[2]*180.0/M_PI);
        ttf.draw(text, 20.0f, win_height - 90.0f, win_width, win_height, 0.4f, glm::vec3(0.0f,0.4f,1.0f), text_shad);

        sprintf(text, "( roll 2, pitch 2, yaw 2 ) :  ( %.2f, %.2f, %.2f ) [ deg ]", rpy2[0]*180.0/M_PI, rpy2[1]*180.0/M_PI, rpy2[2]*180.0/M_PI);
        ttf.draw(text, 20.0f, win_height - 120.0f, win_width, win_height, 0.4f, glm::vec3(0.0f,0.4f,1.0f), text_shad);

        vec2 energy_momentum = ener_mom(state);
        sprintf(text, "energy error :  %.0e", fabs( (energy_momentum[0] - ener0_mom0[0])/ener0_mom0[0] ));
        ttf.draw(text, 20.0f, win_height - 150.0f, win_width, win_height, 0.4f, glm::vec3(0.0f,0.4f,1.0f), text_shad);
        sprintf(text, "momentum error :  %.0e", fabs( (energy_momentum[1] - ener0_mom0[1])/ener0_mom0[1] ));
        ttf.draw(text, 20.0f, win_height - 180.0f, win_width, win_height, 0.4f, glm::vec3(0.0f,0.4f,1.0f), text_shad);

        sprintf(text, "fps :  %d [ %s ]", (int)(1000.0/ms_per_frame), gpu_vendor);
        ttf.draw(text, 20.0f, win_height - 210.0f, win_width, win_height, 0.4f, glm::vec3(0.0f,0.4f,1.0f), text_shad);
        
        glfwSwapBuffers(win);
        glfwPollEvents();

        rk4_step(state);
        duration += dt/86400.0;
    }

    glfwTerminate();
    return 0;
}
